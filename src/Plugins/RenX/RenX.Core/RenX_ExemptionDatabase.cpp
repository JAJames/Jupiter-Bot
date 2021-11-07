/**
* Copyright (C) 2016-2021 Jessica James.
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

#include <cstdio>
#include "Jupiter/IRC_Client.h"
#include "RenX_PlayerInfo.h"
#include "RenX_ExemptionDatabase.h"
#include "RenX_Core.h"
#include "RenX_Plugin.h"

using namespace Jupiter::literals;

RenX::ExemptionDatabase g_exemptionDatabase;
RenX::ExemptionDatabase *RenX::exemptionDatabase = &g_exemptionDatabase;
RenX::ExemptionDatabase &RenX::defaultExemptionDatabase = g_exemptionDatabase;

void RenX::ExemptionDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos) {
	std::unique_ptr<Entry> entry = std::make_unique<Entry>();
	entry->pos = pos;

	// Read data from buffer to entry
	entry->flags = buffer.pop<uint8_t>();
	entry->timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(buffer.pop<uint64_t>()));
	entry->length = std::chrono::seconds(buffer.pop<int64_t>());
	entry->steamid = buffer.pop<uint64_t>();
	entry->ip = buffer.pop<uint32_t>();
	entry->prefix_length = buffer.pop<uint8_t>();
	entry->setter = buffer.pop<Jupiter::String_Strict, char>();

	m_entries.push_back(std::move(entry));
}

void RenX::ExemptionDatabase::process_header(FILE *file) {
	int chr = fgetc(file);
	if (chr != EOF)
		m_read_version = chr;
}

void RenX::ExemptionDatabase::create_header(FILE *file) {
	fputc(m_write_version, file);
}

void RenX::ExemptionDatabase::process_file_finish(FILE *file) {
	fgetpos(file, std::addressof(m_eof));
}

void RenX::ExemptionDatabase::upgrade_database() {
	FILE *file = fopen(m_filename.c_str(), "wb");
	if (file != nullptr) {
		this->create_header(file);
		for (size_t index = 0; m_entries.size(); ++index) {
			write(m_entries[index].get(), file);
		}

		fclose(file);
	}
}

void RenX::ExemptionDatabase::write(RenX::ExemptionDatabase::Entry *entry) {
	FILE *file = fopen(m_filename.c_str(), "r+b");
	fsetpos(file, std::addressof(m_eof));
	if (file != nullptr) {
		write(entry, file);
		fclose(file);
	}
}

void RenX::ExemptionDatabase::write(RenX::ExemptionDatabase::Entry *entry, FILE *file) {
	Jupiter::DataBuffer buffer;
	fgetpos(file, &entry->pos);

	// push data from entry to buffer
	buffer.push(entry->flags);
	buffer.push(static_cast<int64_t>(std::chrono::duration_cast<std::chrono::seconds>(entry->timestamp.time_since_epoch()).count()));
	buffer.push(static_cast<int64_t>(entry->length.count()));
	buffer.push(entry->steamid);
	buffer.push(entry->ip);
	buffer.push(entry->prefix_length);
	buffer.push(entry->setter);

	// push buffer to file
	buffer.push_to(file);
	fgetpos(file, std::addressof(m_eof));
}

void RenX::ExemptionDatabase::add(RenX::Server &, const RenX::PlayerInfo &player, const Jupiter::ReadableString &setter, std::chrono::seconds length, uint8_t flags) {
	add(player.ip32, 32U, player.steamid, setter, length, flags);
}

void RenX::ExemptionDatabase::add(uint32_t ip, uint8_t prefix_length, uint64_t steamid, const Jupiter::ReadableString &setter, std::chrono::seconds length, uint8_t flags) {
	std::unique_ptr<Entry> entry = std::make_unique<Entry>();
	entry->set_active();
	entry->flags |= flags;
	entry->timestamp = std::chrono::system_clock::now();
	entry->length = length;
	entry->steamid = steamid;
	entry->ip = ip;
	entry->prefix_length = prefix_length;
	entry->setter = setter;

	m_entries.push_back(std::move(entry));
	write(m_entries.back().get());
}

bool RenX::ExemptionDatabase::deactivate(size_t index) {
	Entry* entry = m_entries[index].get();
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

void RenX::ExemptionDatabase::exemption_check(RenX::PlayerInfo &player) {
	Entry* entry;
	uint32_t netmask;
	size_t index = m_entries.size();
	while (index != 0) {
		entry = m_entries[--index].get();
		if (entry->is_active()) {
			if (entry->length == std::chrono::seconds::zero() || entry->timestamp + entry->length < std::chrono::system_clock::now()) {
				netmask = Jupiter_prefix_length_to_netmask(entry->prefix_length);
				if ((player.steamid != 0 && entry->steamid == player.steamid) // SteamID exemption
					|| (player.ip32 != 0U && (player.ip32 & netmask) == (entry->ip & netmask))) { // IP address exemption
					player.exemption_flags |= entry->flags;
				}
			}
			else {
				deactivate(index);
			}
		}
	}
}

uint8_t RenX::ExemptionDatabase::getVersion() const {
	return m_write_version;
}

const std::string &RenX::ExemptionDatabase::getFileName() const {
	return m_filename;
}

const std::vector<std::unique_ptr<RenX::ExemptionDatabase::Entry>>& RenX::ExemptionDatabase::getEntries() const {
	return m_entries;
}

bool RenX::ExemptionDatabase::initialize() {
	m_filename = static_cast<std::string>(RenX::getCore()->getConfig().get("ExemptionDB"_jrs, "Exemptions.db"_jrs));
	return this->process_file(m_filename);
}

RenX::ExemptionDatabase::~ExemptionDatabase() {
}
