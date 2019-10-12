/**
* Copyright (C) 2016-2017 Jessica James.
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

RenX::ExemptionDatabase _exemptionDatabase;
RenX::ExemptionDatabase *RenX::exemptionDatabase = &_exemptionDatabase;
RenX::ExemptionDatabase &RenX::defaultExemptionDatabase = _exemptionDatabase;

void RenX::ExemptionDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos)
{
	RenX::ExemptionDatabase::Entry *entry = new RenX::ExemptionDatabase::Entry();
	entry->pos = pos;

	// Read data from buffer to entry
	entry->flags = buffer.pop<uint8_t>();
	entry->timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(buffer.pop<uint64_t>()));
	entry->length = std::chrono::seconds(buffer.pop<int64_t>());
	entry->steamid = buffer.pop<uint64_t>();
	entry->ip = buffer.pop<uint32_t>();
	entry->prefix_length = buffer.pop<uint8_t>();
	entry->setter = buffer.pop<Jupiter::String_Strict, char>();

	RenX::ExemptionDatabase::entries.add(entry);
}

void RenX::ExemptionDatabase::process_header(FILE *file)
{
	int chr = fgetc(file);
	if (chr != EOF)
		RenX::ExemptionDatabase::read_version = chr;
}

void RenX::ExemptionDatabase::create_header(FILE *file)
{
	fputc(RenX::ExemptionDatabase::write_version, file);
}

void RenX::ExemptionDatabase::process_file_finish(FILE *file)
{
	fgetpos(file, std::addressof(RenX::ExemptionDatabase::eof));
}

void RenX::ExemptionDatabase::upgrade_database()
{
	FILE *file = fopen(RenX::ExemptionDatabase::filename.c_str(), "wb");
	if (file != nullptr)
	{
		this->create_header(file);
		for (size_t index = 0; RenX::ExemptionDatabase::entries.size(); ++index)
			RenX::ExemptionDatabase::write(RenX::ExemptionDatabase::entries.get(index), file);

		fclose(file);
	}
}

void RenX::ExemptionDatabase::write(RenX::ExemptionDatabase::Entry *entry)
{
	FILE *file = fopen(filename.c_str(), "r+b");
	fsetpos(file, std::addressof(RenX::ExemptionDatabase::eof));
	if (file != nullptr)
	{
		RenX::ExemptionDatabase::write(entry, file);
		fclose(file);
	}
}

void RenX::ExemptionDatabase::write(RenX::ExemptionDatabase::Entry *entry, FILE *file)
{
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
	fgetpos(file, std::addressof(RenX::ExemptionDatabase::eof));
}

void RenX::ExemptionDatabase::add(RenX::Server &, const RenX::PlayerInfo &player, const Jupiter::ReadableString &setter, std::chrono::seconds length, uint8_t flags)
{
	RenX::ExemptionDatabase::add(player.ip32, 32U, player.steamid, setter, length, flags);
}

void RenX::ExemptionDatabase::add(uint32_t ip, uint8_t prefix_length, uint64_t steamid, const Jupiter::ReadableString &setter, std::chrono::seconds length, uint8_t flags)
{
	Entry *entry = new Entry();
	entry->set_active();
	entry->flags |= flags;
	entry->timestamp = std::chrono::system_clock::now();
	entry->length = length;
	entry->steamid = steamid;
	entry->ip = ip;
	entry->prefix_length = prefix_length;
	entry->setter = setter;

	entries.add(entry);
	RenX::ExemptionDatabase::write(entry);
}

bool RenX::ExemptionDatabase::deactivate(size_t index)
{
	RenX::ExemptionDatabase::Entry *entry = RenX::ExemptionDatabase::entries.get(index);
	if (entry->is_active())
	{
		entry->unset_active();
		FILE *file = fopen(RenX::ExemptionDatabase::filename.c_str(), "r+b");
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

void RenX::ExemptionDatabase::exemption_check(RenX::PlayerInfo &player)
{
	RenX::ExemptionDatabase::Entry *entry;
	uint32_t netmask;
	size_t index = RenX::ExemptionDatabase::entries.size();
	while (index != 0)
	{
		entry = RenX::ExemptionDatabase::entries.get(--index);
		if (entry->is_active())
		{
			if (entry->length == std::chrono::seconds::zero() || entry->timestamp + entry->length < std::chrono::system_clock::now())
			{
				netmask = Jupiter_prefix_length_to_netmask(entry->prefix_length);
				if ((player.steamid != 0 && entry->steamid == player.steamid) // SteamID exemption
					|| (player.ip32 != 0U && (player.ip32 & netmask) == (entry->ip & netmask))) // IP address exemption
					player.exemption_flags |= entry->flags;
			}
			else
				RenX::ExemptionDatabase::deactivate(index);
		}
	}
}

uint8_t RenX::ExemptionDatabase::getVersion() const
{
	return RenX::ExemptionDatabase::write_version;
}

const std::string &RenX::ExemptionDatabase::getFileName() const
{
	return RenX::ExemptionDatabase::filename;
}

const Jupiter::ArrayList<RenX::ExemptionDatabase::Entry> &RenX::ExemptionDatabase::getEntries() const
{
	return RenX::ExemptionDatabase::entries;
}

bool RenX::ExemptionDatabase::initialize()
{
	RenX::ExemptionDatabase::filename = static_cast<std::string>(RenX::getCore()->getConfig().get("ExemptionDB"_jrs, "Exemptions.db"_jrs));
	return this->process_file(filename);
}

RenX::ExemptionDatabase::~ExemptionDatabase()
{
	RenX::ExemptionDatabase::entries.emptyAndDelete();
}
