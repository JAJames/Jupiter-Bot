/**
 * Copyright (C) 2014-2015 Justin James.
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
 * Written by Justin James <justin.aj@hotmail.com>
 */

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

const Jupiter::ReferenceString DEFAULT_REASON = "(No reason information provided)";
const uint8_t write_version = 1;

bool RenX::BanDatabase::load(const Jupiter::ReadableString &fname)
{
	RenX::BanDatabase::filename = fname;
	FILE *file = fopen(RenX::BanDatabase::filename.c_str(), "rb");
	if (file != nullptr)
	{
		RenX::BanDatabase::version = fgetc(file);
		while (!feof(file))
			if (fgetc(file) == '\n')
				break;
		Jupiter::String playerName(16);
		Jupiter::String key(32);
		Jupiter::String value(32);
		Jupiter::String reason(128);
		if (RenX::BanDatabase::version < 1)
			reason = DEFAULT_REASON;
		Entry *entry;
		int c;
		while (!feof(file))
		{
			entry = new Entry();
			fgetpos(file, &entry->pos);
			fread(&entry->active, 1, 1, file);
			fread(&entry->timestamp, sizeof(time_t), 1, file);
			fread(&entry->length, sizeof(time_t), 1, file);
			fread(&entry->steamid, sizeof(uint64_t), 1, file);
			fread(&entry->ip, sizeof(uint32_t), 1, file);
			if (feof(file))
			{
				delete entry;
				break;
			}

			// load name
			playerName.truncate(playerName.size());
			c = fgetc(file);
			while (c != '\n' && c != '\0')
			{
				if (c == EOF)
				{
					fprintf(stderr, "ERROR: Unexpected EOF in %s at %lu", RenX::BanDatabase::filename.c_str(), ftell(file));
					break;
				}
				playerName += c;
				c = fgetc(file);
			}
			entry->name = playerName;

			// load reason
			if (RenX::BanDatabase::version >= 1)
			{
				reason.truncate(reason.size());
				c = fgetc(file);
				while (c != '\n' && c != '\0')
				{
					if (c == EOF)
					{
						fprintf(stderr, "ERROR: Unexpected EOF in %s at %lu", RenX::BanDatabase::filename.c_str(), ftell(file));
						break;
					}
					reason += c;
					c = fgetc(file);
				}
			}
			entry->reason = reason;

			// load variable data
			while (c == '\0')
			{
				key.truncate(key.size());
				value.truncate(value.size());

				// load key
				c = fgetc(file);
				while (c != '\0' && c != EOF)
				{
					key += c;
					c = fgetc(file);
				}
				if (c == EOF)
				{
					fprintf(stderr, "ERROR: Unexpected EOF in %s at %lu", RenX::BanDatabase::filename.c_str(), ftell(file));
					break;
				}

				// load value
				c = fgetc(file);
				while (c != '\n' && c != '\0' && c != EOF)
				{
					value += c;
					c = fgetc(file);
				}
				if (c == EOF)
				{
					fprintf(stderr, "ERROR: Unexpected EOF in %s at %lu", RenX::BanDatabase::filename.c_str(), ftell(file));
					break;
				}

				entry->varData.set(key, value);
			}
			entries.add(entry);
		}
		fclose(file);
		if (RenX::BanDatabase::version != write_version)
		{
			file = fopen(RenX::BanDatabase::filename.c_str(), "wb");
			if (file != nullptr)
			{
				fputc(write_version, file);
				fputc('\n', file);
				size_t index = 0;
				while (index != RenX::BanDatabase::entries.size())
					RenX::BanDatabase::write(RenX::BanDatabase::entries.get(++index), file);
				fclose(file);
				fprintf(stdout, "Updated BanDatabase file \"%s\" from version %d to %d.", RenX::BanDatabase::filename.c_str(), RenX::BanDatabase::version, write_version);
			}
			else
				fprintf(stdout, "CRITICAL ERROR: BanDatabase file \"%s\" failed to update from version %d to %d.", RenX::BanDatabase::filename.c_str(), RenX::BanDatabase::version, write_version);
			RenX::BanDatabase::version = write_version;
		}
		return true;
	}
	else
	{
		RenX::BanDatabase::version = write_version;
		file = fopen(RenX::BanDatabase::filename.c_str(), "ab");
		if (file != nullptr)
		{
			fputc(RenX::BanDatabase::version, file);
			fputc('\n', file);
			fclose(file);
			return true;
		}
		return false;
	}
}

void RenX::BanDatabase::write(RenX::BanDatabase::Entry *entry)
{
	if (RenX::BanDatabase::version != write_version)
		"CRITICAL ERROR: COULD NOT WRITE BAN ENTRY TO BAN DATABASE (VERSION MISMATCH)"_jrs.print(stdout);
	FILE *file = fopen(RenX::BanDatabase::filename.c_str(), "ab");
	if (file != nullptr)
	{
		RenX::BanDatabase::write(entry, file);
		fclose(file);
	}
}

void RenX::BanDatabase::write(RenX::BanDatabase::Entry *entry, FILE *file)
{
	fgetpos(file, &entry->pos);
	fwrite(&entry->active, 1, 1, file);
	fwrite(&entry->timestamp, sizeof(time_t), 1, file);
	fwrite(&entry->length, sizeof(time_t), 1, file);
	fwrite(&entry->steamid, sizeof(uint64_t), 1, file);
	fwrite(&entry->ip, sizeof(uint32_t), 1, file);
	fwrite(entry->name.ptr(), sizeof(char), entry->name.size(), file);

	for (size_t index = 0; index != entry->varData.size(); ++index)
	{
		const Jupiter::INIFile::Section::KeyValuePair *pair = entry->varData.getPair(index);
		fputc('\0', file);
		fwrite(pair->getKey().ptr(), sizeof(char), pair->getKey().size(), file);
		fputc('\0', file);
		fwrite(pair->getValue().ptr(), sizeof(char), pair->getValue().size(), file);
	}

	fputc('\n', file);
}

void RenX::BanDatabase::add(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason, time_t length)
{
	Entry *entry = new Entry();
	entry->active = 1;
	entry->timestamp = time(0);
	entry->length = length;
	entry->steamid = player->steamid;
	entry->ip = player->ip32;
	entry->name = player->name;

	// add plugin data
	Jupiter::String pluginData;
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		if (xPlugins.get(i)->RenX_OnBan(server, player, pluginData))
		entry->varData.set(xPlugins.get(i)->getName(), pluginData);

	entries.add(entry);
	RenX::BanDatabase::write(entry);
}

bool RenX::BanDatabase::deactivate(size_t index)
{
	RenX::BanDatabase::Entry *entry = RenX::BanDatabase::entries.get(index);
	if (entry->active)
	{
		entry->active = 0;
		FILE *file = fopen(RenX::BanDatabase::filename.c_str(), "r+b");
		if (file != nullptr)
		{
			fsetpos(file, &entry->pos);
			fputc(entry->active, file);
			fclose(file);
		}
		return true;
	}
	return false;
}

uint8_t RenX::BanDatabase::getVersion() const
{
	return RenX::BanDatabase::version;
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
	RenX::BanDatabase::load(Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("BanDB"), STRING_LITERAL_AS_REFERENCE("Bans.db")));
}

RenX::BanDatabase::~BanDatabase()
{
	RenX::BanDatabase::entries.emptyAndDelete();
}