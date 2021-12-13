/**
 * Copyright (C) 2014-2021 Jessica James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include <ctime>
#include <iostream>
#include "Jupiter/IRC_Client.h"
#include "Jupiter/DataBuffer.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"
#include "RenX_Core.h"
#include "RenX_Plugin.h"

using namespace std::literals;

RenX::BanDatabase _banDatabase;
RenX::BanDatabase *RenX::banDatabase = &_banDatabase;
RenX::BanDatabase &RenX::defaultBanDatabase = _banDatabase;

void RenX::BanDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos)
{
	if (m_read_version < 3U)
		return; // incompatible database version

	std::unique_ptr<Entry> entry = std::make_unique<Entry>();
	entry->pos = pos;

	// Read data from buffer to entry
	entry->flags = buffer.pop<uint16_t>();
	if (m_read_version >= 4U)
	{
		entry->timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(buffer.pop<uint64_t>()));
		entry->length = std::chrono::seconds(buffer.pop<uint64_t>());
	}
	else
	{
		entry->timestamp = std::chrono::system_clock::from_time_t(buffer.pop<time_t>());
		entry->length = std::chrono::seconds(static_cast<long long>(buffer.pop<time_t>()));
	}
	entry->steamid = buffer.pop<uint64_t>();
	entry->ip = buffer.pop<uint32_t>();
	entry->prefix_length = buffer.pop<uint8_t>();
	if (m_read_version >= 5U)
		entry->hwid = buffer.pop<std::string>();
	entry->rdns = buffer.pop<std::string>();
	entry->name = buffer.pop<std::string>();
	entry->banner = buffer.pop<std::string>();
	entry->reason = buffer.pop<std::string>();

	// Read varData from buffer to entry
	for (size_t varData_entries = buffer.pop<size_t>(); varData_entries != 0; --varData_entries) {
		entry->varData[buffer.pop<std::string>()] = buffer.pop<std::string>();
	}

	m_entries.push_back(std::move(entry));
}

void RenX::BanDatabase::process_header(FILE *file)
{
	int chr = fgetc(file);
	if (chr != EOF)
		m_read_version = chr;
}

void RenX::BanDatabase::create_header(FILE *file)
{
	fputc(m_write_version, file);
}

void RenX::BanDatabase::process_file_finish(FILE *file) {
	if (m_read_version < 3) {
		if (freopen(m_filename.c_str(), "wb", file) == nullptr) {
			std::cout << "FATAL ERROR: UNABLE TO REMOVE UNSUPPORTED BAN DATABASE FILE VERSION" << std::endl;
			return;
		}

		std::cout << "Warning: Unsupported ban database file version. The database will be removed and rewritten." << std::endl;
		create_header(file);
		fgetpos(file, std::addressof(m_eof));
		m_read_version = m_write_version;
		return;
	}
	else if (m_read_version < m_write_version) {
		if (freopen(m_filename.c_str(), "wb", file) != nullptr) {
			this->create_header(file);
			for (const auto& entry : m_entries) {
				write(entry.get(), file);
			}
		}
	}

	fgetpos(file, std::addressof(m_eof));
}

void RenX::BanDatabase::upgrade_database() {
	FILE *file = fopen(m_filename.c_str(), "wb");
	if (file != nullptr) {
		this->create_header(file);
		for (const auto& entry : m_entries) {
			write(entry.get(), file);
		}
		
		fclose(file);
	}
}

void RenX::BanDatabase::write(Entry* entry) {
	FILE *file = fopen(m_filename.c_str(), "r+b");
	fsetpos(file, std::addressof(m_eof));
	if (file != nullptr) {
		write(entry, file);
		fclose(file);
	}
}

void RenX::BanDatabase::write(Entry* entry, FILE *file) {
	Jupiter::DataBuffer buffer;
	fgetpos(file, &entry->pos);

	// push data from entry to buffer
	buffer.push(entry->flags);
	buffer.push(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(entry->timestamp.time_since_epoch()).count()));
	buffer.push(static_cast<uint64_t>(entry->length.count()));
	buffer.push(entry->steamid);
	buffer.push(entry->ip);
	buffer.push(entry->prefix_length);
	buffer.push(entry->hwid);
	buffer.push(entry->rdns);
	buffer.push(entry->name);
	buffer.push(entry->banner);
	buffer.push(entry->reason);

	// push varData from entry to buffer
	size_t varData_entries = entry->varData.size();
	buffer.push(varData_entries);

	for (auto& entry : entry->varData) {
		buffer.push(entry.first);
		buffer.push(entry.second);
	}

	// push buffer to file
	buffer.push_to(file);
	fgetpos(file, std::addressof(m_eof));
}

void RenX::BanDatabase::add(RenX::Server *server, const RenX::PlayerInfo &player, std::string_view banner, std::string_view reason, std::chrono::seconds length, uint16_t flags) {
	std::unique_ptr<Entry> entry = std::make_unique<Entry>();
	if (flags != 0) {
		entry->set_active();
		entry->flags |= flags;
	}
	entry->timestamp = std::chrono::system_clock::now();
	entry->length = length;
	entry->steamid = player.steamid;
	entry->ip = player.ip32;
	entry->prefix_length = 32U;
	if (player.hwid.find_first_not_of('0') != std::string::npos) {
		entry->hwid = player.hwid;
	}
	entry->rdns = player.get_rdns();
	entry->name = player.name;
	entry->banner = banner;
	entry->reason = reason;

	// add plugin data
	std::string pluginData;
	for (Plugin* plugin : RenX::getCore()->getPlugins()) {
		if (plugin->RenX_OnBan(*server, player, pluginData)) {
			if (!pluginData.empty()) {
				entry->varData[plugin->getName()] = pluginData;
			}
		}
	}

	m_entries.push_back(std::move(entry));
	write(m_entries.back().get());
}

void RenX::BanDatabase::add(std::string name, uint32_t ip, uint8_t prefix_length, uint64_t steamid, std::string hwid, std::string rdns, std::string banner, std::string reason, std::chrono::seconds length, uint16_t flags) {
	std::unique_ptr<Entry> entry = std::make_unique<Entry>();
	entry->set_active();
	entry->flags |= flags;
	entry->timestamp = std::chrono::system_clock::now();
	entry->length = length;
	entry->steamid = steamid;
	entry->ip = ip;
	entry->prefix_length = prefix_length;
	entry->hwid = std::move(hwid);
	entry->rdns = std::move(rdns);
	entry->name = std::move(name);
	entry->banner = std::move(banner);
	entry->reason = std::move(reason);

	m_entries.push_back(std::move(entry));
	write(m_entries.back().get());
}

bool RenX::BanDatabase::deactivate(size_t index) {
	const auto& entry = m_entries[index];
	return deactivate(entry.get());
}

bool RenX::BanDatabase::deactivate(Entry* entry) {
	if (entry->is_active()) {
		entry->unset_active();
		FILE *file = fopen(m_filename.c_str(), "r+b");
		if (file != nullptr) {
			fsetpos(file, &entry->pos);
			fseek(file, sizeof(size_t), SEEK_CUR);
			fwrite(std::addressof(entry->flags), sizeof(entry->flags), 1, file);
			fclose(file);
		}
		return true;
	}
	return false;
}

uint8_t RenX::BanDatabase::getVersion() const {
	return m_write_version;
}

const std::string &RenX::BanDatabase::getFileName() const {
	return m_filename;
}

const std::vector<std::unique_ptr<RenX::BanDatabase::Entry>>& RenX::BanDatabase::getEntries() const {
	return m_entries;
}

bool RenX::BanDatabase::initialize() {
	m_filename = RenX::getCore()->getConfig().get("BanDB"sv, "Bans.db"s);
	return this->process_file(m_filename);
}

RenX::BanDatabase::~BanDatabase() {
}