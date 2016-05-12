/**
 * Copyright (C) 2014-2016 Jessica James.
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
#include <cstdio>
#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"
#include "RenX_Core.h"
#include "RenX_Plugin.h"

using namespace Jupiter::literals;

RenX::BanDatabase _banDatabase;
RenX::BanDatabase *RenX::banDatabase = &_banDatabase;
RenX::BanDatabase &RenX::defaultBanDatabase = _banDatabase;

void RenX::BanDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos)
{
	if (RenX::BanDatabase::read_version < 3U)
		return; // incompatible database version

	RenX::BanDatabase::Entry *entry = new RenX::BanDatabase::Entry();
	entry->pos = pos;

	// Read data from buffer to entry
	entry->flags = buffer.pop<uint16_t>();
	if (RenX::BanDatabase::read_version >= 4U)
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
	if (this->read_version >= 5U)
		entry->hwid = buffer.pop<Jupiter::String_Strict, char>();
	entry->rdns = buffer.pop<Jupiter::String_Strict, char>();
	entry->name = buffer.pop<Jupiter::String_Strict, char>();
	entry->banner = buffer.pop<Jupiter::String_Strict, char>();
	entry->reason = buffer.pop<Jupiter::String_Strict, char>();

	// Read varData from buffer to entry
	for (size_t varData_entries = buffer.pop<size_t>(); varData_entries != 0; --varData_entries)
		entry->varData.set(buffer.pop<Jupiter::String_Strict, char>(), buffer.pop<Jupiter::String_Strict, char>());

	RenX::BanDatabase::entries.add(entry);
}

void RenX::BanDatabase::process_header(FILE *file)
{
	int chr = fgetc(file);
	if (chr != EOF)
		RenX::BanDatabase::read_version = chr;
}

void RenX::BanDatabase::create_header(FILE *file)
{
	fputc(RenX::BanDatabase::write_version, file);
}

void RenX::BanDatabase::process_file_finish(FILE *file)
{
	if (RenX::BanDatabase::read_version < 3)
	{
		if (freopen(RenX::BanDatabase::filename.c_str(), "wb", file) == nullptr)
			puts("FATAL ERROR: UNABLE TO REMOVE UNSUPPORTED BAN DATABASE FILE VERSION");
		else
		{
			puts("Warning: Unsupported ban database file version. The database will be removed and rewritten.");
			this->create_header(file);
			fgetpos(file, std::addressof(RenX::BanDatabase::eof));
			RenX::BanDatabase::read_version = RenX::BanDatabase::write_version;
		}
		return;
	}
	else if (RenX::BanDatabase::read_version < RenX::BanDatabase::write_version)
	{
		if (freopen(RenX::BanDatabase::filename.c_str(), "wb", file) != nullptr)
		{
			this->create_header(file);
			for (size_t index = 0; index != RenX::BanDatabase::entries.size(); ++index)
				RenX::BanDatabase::write(RenX::BanDatabase::entries.get(index), file);
		}
	}

	fgetpos(file, std::addressof(RenX::BanDatabase::eof));
}

void RenX::BanDatabase::upgrade_database()
{
	FILE *file = fopen(RenX::BanDatabase::filename.c_str(), "wb");
	if (file != nullptr)
	{
		this->create_header(file);
		for (size_t index = 0; index != RenX::BanDatabase::entries.size(); ++index)
			RenX::BanDatabase::write(RenX::BanDatabase::entries.get(index), file);
		
		fclose(file);
	}
}

void RenX::BanDatabase::write(RenX::BanDatabase::Entry *entry)
{
	FILE *file = fopen(filename.c_str(), "r+b");
	fsetpos(file, std::addressof(RenX::BanDatabase::eof));
	if (file != nullptr)
	{
		RenX::BanDatabase::write(entry, file);
		fclose(file);
	}
}

void RenX::BanDatabase::write(RenX::BanDatabase::Entry *entry, FILE *file)
{
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

	Jupiter::INIFile::Section::KeyValuePair *pair;
	while (varData_entries != 0)
	{
		pair = entry->varData.getPair(--varData_entries);
		buffer.push(pair->getKey());
		buffer.push(pair->getValue());
	}

	// push buffer to file
	buffer.push_to(file);
	fgetpos(file, std::addressof(RenX::BanDatabase::eof));
}

void RenX::BanDatabase::add(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &banner, const Jupiter::ReadableString &reason, std::chrono::seconds length, uint16_t flags)
{
	Entry *entry = new Entry();
	entry->set_active();
	entry->flags |= flags;
	entry->timestamp = std::chrono::system_clock::now();
	entry->length = length;
	entry->steamid = player->steamid;
	entry->ip = player->ip32;
	entry->prefix_length = 32U;
	entry->hwid = player->hwid;
	if (player->rdns_thread.joinable())
		player->rdns_thread.join();
	entry->rdns = player->rdns;
	entry->name = player->name;
	entry->banner = banner;
	entry->reason = reason;

	// add plugin data
	Jupiter::String pluginData;
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		if (xPlugins.get(i)->RenX_OnBan(server, player, pluginData))
			entry->varData.set(xPlugins.get(i)->getName(), pluginData);

	entries.add(entry);
	RenX::BanDatabase::write(entry);
}

void RenX::BanDatabase::add(const Jupiter::ReadableString &name, uint32_t ip, uint8_t prefix_length, uint64_t steamid, const Jupiter::ReadableString &hwid, const Jupiter::ReadableString &rdns, const Jupiter::ReadableString &banner, Jupiter::ReadableString &reason, std::chrono::seconds length, uint16_t flags)
{
	Entry *entry = new Entry();
	entry->set_active();
	entry->flags |= flags;
	entry->timestamp = std::chrono::system_clock::now();
	entry->length = length;
	entry->steamid = steamid;
	entry->ip = ip;
	entry->prefix_length = prefix_length;
	entry->hwid = hwid;
	entry->rdns = rdns;
	entry->name = name;
	entry->banner = banner;
	entry->reason = reason;

	entries.add(entry);
	RenX::BanDatabase::write(entry);
}

bool RenX::BanDatabase::deactivate(size_t index)
{
	RenX::BanDatabase::Entry *entry = RenX::BanDatabase::entries.get(index);
	if (entry->is_active())
	{
		entry->unset_active();
		FILE *file = fopen(RenX::BanDatabase::filename.c_str(), "r+b");
		if (file != nullptr)
		{
			fsetpos(file, &entry->pos);
			fseek(file, sizeof(size_t), SEEK_CUR);
			fwrite(std::addressof(entry->flags), sizeof(entry->flags), 1, file);
			fclose(file);
		}
		return true;
	}
	return false;
}

uint8_t RenX::BanDatabase::getVersion() const
{
	return RenX::BanDatabase::write_version;
}

const Jupiter::ReadableString &RenX::BanDatabase::getFileName() const
{
	return RenX::BanDatabase::filename;
}

const Jupiter::ArrayList<RenX::BanDatabase::Entry> &RenX::BanDatabase::getEntries() const
{
	return RenX::BanDatabase::entries;
}

RenX::BanDatabase::BanDatabase()
{
	RenX::BanDatabase::filename = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("BanDB"), STRING_LITERAL_AS_REFERENCE("Bans.db"));
	this->process_file(filename);
}

RenX::BanDatabase::~BanDatabase()
{
	RenX::BanDatabase::entries.emptyAndDelete();
}