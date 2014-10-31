/**
 * Copyright (C) 2014 Justin James.
 *
 * This license must be preserved.
 * Any applications, libraries, or code which make any use of any
 * component of this program must not be commercial, unless explicit
 * permission is granted from the original author. The use of this
 * program for non-profit purposes is permitted.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In the event that this license restricts you from making desired use of this program, contact the original author.
 * Written by Justin James <justin.aj@hotmail.com>
 */

#include <cstdio>
#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"
#include "RenX_Core.h"
#include "RenX_Plugin.h"

RenX::BanDatabase _banDatabase;
RenX::BanDatabase *RenX::banDatabase = &_banDatabase;
RenX::BanDatabase &RenX::defaultBanDatabase = _banDatabase;

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
		Entry *entry;
		char c;
		while (!feof(file))
		{
			entry = new Entry();
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
			c = fgetc(file);
			while (c != '\n' && c != EOF)
			{
				if (c == '\0')
				{
					key.truncate(key.size());
					value.truncate(value.size());
					c = fgetc(file);
					while (c != '\n' && c != EOF)
					{
						while (c != '\0')
						{
							key += c;
							c = fgetc(file);
							if (c == EOF)
							{
								fprintf(stderr, "ERROR: Unexpected EOF in %s at %lu", RenX::BanDatabase::filename.c_str(), ftell(file));
								break;
							}
						}
						c = fgetc(file);
						while (c != '\n' && c != EOF)
						{
							value += c;
							c = fgetc(file);
						}
						entry->varData.set(key, value);
						c = fgetc(file);
					}
					break;
				}
				playerName += c;
				c = fgetc(file);
			}
			entry->name = playerName;
			entries.add(entry);
		}
		fclose(file);
		return true;
	}
	else
	{
		RenX::BanDatabase::version = 0;
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

void RenX::BanDatabase::add(RenX::Server *server, const RenX::PlayerInfo *player, time_t length)
{
	Entry *entry = new Entry();
	entry->active = 1;
	entry->timestamp = time(0);
	entry->length = length;
	entry->steamid = player->steamid;
	entry->ip = player->ip32;
	entry->name = player->name;
	entries.add(entry);

	FILE *file = fopen(RenX::BanDatabase::filename.c_str(), "ab");
	if (file != nullptr)
	{
		fwrite(&entry->active, 1, 1, file);
		fwrite(&entry->timestamp, sizeof(time_t), 1, file);
		fwrite(&entry->length, sizeof(time_t), 1, file);
		fwrite(&entry->steamid, sizeof(uint64_t), 1, file);
		fwrite(&entry->ip, sizeof(uint32_t), 1, file);
		fwrite(entry->name.ptr(), sizeof(char), entry->name.size(), file);
		// add plugin data
		Jupiter::String pluginData;
		Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
		for (size_t i = 0; i < xPlugins.size(); i++)
			if (xPlugins.get(i)->RenX_OnBan(server, player, pluginData))
			{
				const Jupiter::ReadableString &pluginName = xPlugins.get(i)->getName();
				fputc('\0', file);
				fwrite(pluginName.ptr(), sizeof(char), pluginName.size(), file);
				fputc('\0', file);
				fwrite(pluginData.ptr(), sizeof(char), pluginData.size(), file);
				entry->varData.set(pluginName, pluginData);
			}


		fputc('\n', file);
		fclose(file);
	}
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