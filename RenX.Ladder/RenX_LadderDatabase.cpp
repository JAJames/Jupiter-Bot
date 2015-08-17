/**
 * Copyright (C) 2015 Justin James.
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

#include "RenX_LadderDatabase.h"

RenX_LadderDatabase::~RenX_LadderDatabase()
{
	while (RenX_LadderDatabase::head != nullptr)
	{
		RenX_LadderDatabase::end = RenX_LadderDatabase::head;
		RenX_LadderDatabase::head = RenX_LadderDatabase::head->next;
		delete RenX_LadderDatabase::end;
	}
}

void RenX_LadderDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos)
{
	RenX_LadderDatabase::Entry *entry = new RenX_LadderDatabase::Entry();

	// read data from buffer to entry
	entry->steam_id = buffer.pop<uint64_t>();
	entry->total_score = buffer.pop<uint64_t>();

	entry->total_kills = buffer.pop<uint32_t>();
	entry->total_deaths = buffer.pop<uint32_t>();
	entry->total_headshot_kills = buffer.pop<uint32_t>();
	entry->total_vehicle_kills = buffer.pop<uint32_t>();
	entry->total_building_kills = buffer.pop<uint32_t>();
	entry->total_defence_kills = buffer.pop<uint32_t>();
	entry->total_captures = buffer.pop<uint32_t>();
	entry->total_game_time = buffer.pop<uint32_t>();
	entry->total_games = buffer.pop<uint32_t>();
	entry->total_gdi_games = buffer.pop<uint32_t>();
	entry->total_nod_games = buffer.pop<uint32_t>();
	entry->total_wins = buffer.pop<uint32_t>();
	entry->total_gdi_wins = buffer.pop<uint32_t>();
	entry->total_nod_wins = buffer.pop<uint32_t>();
	entry->total_beacon_placements = buffer.pop<uint32_t>();
	entry->total_beacon_disarms = buffer.pop<uint32_t>();
	entry->total_proxy_placements = buffer.pop<uint32_t>();
	entry->total_proxy_disarms = buffer.pop<uint32_t>();

	entry->top_score = buffer.pop<uint32_t>();
	entry->top_kills = buffer.pop<uint32_t>();
	entry->most_deaths = buffer.pop<uint32_t>();
	entry->top_headshot_kills = buffer.pop<uint32_t>();
	entry->top_vehicle_kills = buffer.pop<uint32_t>();
	entry->top_building_kills = buffer.pop<uint32_t>();
	entry->top_defence_kills = buffer.pop<uint32_t>();
	entry->top_captures = buffer.pop<uint32_t>();
	entry->top_game_time = buffer.pop<uint32_t>();
	entry->top_beacon_placements = buffer.pop<uint32_t>();
	entry->top_beacon_disarms = buffer.pop<uint32_t>();
	entry->top_proxy_placements = buffer.pop<uint32_t>();
	entry->top_proxy_disarms = buffer.pop<uint32_t>();

	entry->most_recent_ip = buffer.pop<uint32_t>();
	entry->last_game = buffer.pop<time_t>();
	entry->most_recent_name = buffer.pop<Jupiter::String_Strict, char>();

	// push data to list
	if (RenX_LadderDatabase::head == nullptr)
	{
		RenX_LadderDatabase::head = entry;
		RenX_LadderDatabase::end = RenX_LadderDatabase::head;
	}
	else
	{
		RenX_LadderDatabase::end->next = entry;
		entry->prev = end;
		end = entry;
	}

	++RenX_LadderDatabase::entries;
}

void RenX_LadderDatabase::process_header(FILE *file)
{
	int chr = fgetc(file);
	if (chr != EOF)
		RenX_LadderDatabase::read_version = chr;
}

void RenX_LadderDatabase::create_header(FILE *file)
{
	fputc(RenX_LadderDatabase::write_version, file);
}

RenX_LadderDatabase::Entry *RenX_LadderDatabase::getHead() const
{
	return RenX_LadderDatabase::head;
}

RenX_LadderDatabase::Entry *RenX_LadderDatabase::getPlayerEntry(uint64_t steamid) const
{
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next)
		if (itr->steam_id == steamid)
			return itr;
	return nullptr;
}

std::pair<RenX_LadderDatabase::Entry *, size_t> RenX_LadderDatabase::getPlayerEntryAndIndex(uint64_t steamid) const
{
	size_t index = 0;
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
		if (itr->steam_id == steamid)
			return std::pair<RenX_LadderDatabase::Entry *, size_t>(itr, index);
	return std::pair<RenX_LadderDatabase::Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

RenX_LadderDatabase::Entry *RenX_LadderDatabase::getPlayerEntryByName(const Jupiter::ReadableString &name) const
{
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next)
		if (itr->most_recent_name.equalsi(name))
			return itr;
	return nullptr;
}

std::pair<RenX_LadderDatabase::Entry *, size_t> RenX_LadderDatabase::getPlayerEntryAndIndexByName(const Jupiter::ReadableString &name) const
{
	size_t index = 0;
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
		if (itr->most_recent_name.equalsi(name))
			return std::pair<RenX_LadderDatabase::Entry *, size_t>(itr, index);
	return std::pair<RenX_LadderDatabase::Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

RenX_LadderDatabase::Entry *RenX_LadderDatabase::getPlayerEntryByPartName(const Jupiter::ReadableString &name) const
{
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next)
		if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			return itr;
	return nullptr;
}

std::pair<RenX_LadderDatabase::Entry *, size_t> RenX_LadderDatabase::getPlayerEntryAndIndexByPartName(const Jupiter::ReadableString &name) const
{
	size_t index = 0;
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
		if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			return std::pair<RenX_LadderDatabase::Entry *, size_t>(itr, index);
	return std::pair<RenX_LadderDatabase::Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

Jupiter::SLList<RenX_LadderDatabase::Entry> RenX_LadderDatabase::getPlayerEntriesByPartName(const Jupiter::ReadableString &name, size_t max) const
{
	Jupiter::SLList<RenX_LadderDatabase::Entry> list;
	if (max == 0)
	{
		for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
				list.add(new RenX_LadderDatabase::Entry(*itr));
	}
	else
		for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			{
				list.add(new RenX_LadderDatabase::Entry(*itr));
				if (--max == 0)
					return list;
			}
	return list;
}

Jupiter::SLList<std::pair<RenX_LadderDatabase::Entry, size_t>> RenX_LadderDatabase::getPlayerEntriesAndIndexByPartName(const Jupiter::ReadableString &name, size_t max) const
{
	Jupiter::SLList<std::pair<RenX_LadderDatabase::Entry, size_t>> list;
	size_t index = 0;
	if (max == 0)
	{
		for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
				list.add(new std::pair<RenX_LadderDatabase::Entry, size_t>(*itr, index));
	}
	else
		for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			{
				list.add(new std::pair<RenX_LadderDatabase::Entry, size_t>(*itr, index));
				if (--max)
					return list;
			}
	return list;
}

RenX_LadderDatabase::Entry *RenX_LadderDatabase::getPlayerEntryByIndex(size_t index) const
{
	for (RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head; itr != nullptr; itr = itr->next, --index)
		if (index == 0)
			return itr;
	return nullptr;
}

size_t RenX_LadderDatabase::getEntries() const
{
	return RenX_LadderDatabase::entries;
}

void RenX_LadderDatabase::append(RenX_LadderDatabase::Entry *entry)
{
	++RenX_LadderDatabase::entries;
	if (RenX_LadderDatabase::head == nullptr)
	{
		RenX_LadderDatabase::head = entry;
		RenX_LadderDatabase::end = RenX_LadderDatabase::head;
		return;
	}
	end->next = entry;
	entry->prev = end;
	end = entry;
}

void RenX_LadderDatabase::write(const Jupiter::CStringType &filename)
{
	return RenX_LadderDatabase::write(filename.c_str());
}

void RenX_LadderDatabase::write(const char *filename)
{
	if (RenX_LadderDatabase::entries != 0)
	{
		FILE *file = fopen(filename, "wb");
		if (file != nullptr)
		{
			Jupiter::DataBuffer buffer;
			RenX_LadderDatabase::create_header(file);
			RenX_LadderDatabase::Entry *entry = RenX_LadderDatabase::head;
			while (entry != nullptr)
			{
				// push data from entry to buffer
				buffer.push(entry->steam_id);
				buffer.push(entry->total_score);

				buffer.push(entry->total_kills);
				buffer.push(entry->total_deaths);
				buffer.push(entry->total_headshot_kills);
				buffer.push(entry->total_vehicle_kills);
				buffer.push(entry->total_building_kills);
				buffer.push(entry->total_defence_kills);
				buffer.push(entry->total_captures);
				buffer.push(entry->total_game_time);
				buffer.push(entry->total_games);
				buffer.push(entry->total_gdi_games);
				buffer.push(entry->total_nod_games);
				buffer.push(entry->total_wins);
				buffer.push(entry->total_gdi_wins);
				buffer.push(entry->total_nod_wins);
				buffer.push(entry->total_beacon_placements);
				buffer.push(entry->total_beacon_disarms);
				buffer.push(entry->total_proxy_placements);
				buffer.push(entry->total_proxy_disarms);

				buffer.push(entry->top_score);
				buffer.push(entry->top_kills);
				buffer.push(entry->most_deaths);
				buffer.push(entry->top_headshot_kills);
				buffer.push(entry->top_vehicle_kills);
				buffer.push(entry->top_building_kills);
				buffer.push(entry->top_defence_kills);
				buffer.push(entry->top_captures);
				buffer.push(entry->top_game_time);
				buffer.push(entry->top_beacon_placements);
				buffer.push(entry->top_beacon_disarms);
				buffer.push(entry->top_proxy_placements);
				buffer.push(entry->top_proxy_disarms);

				buffer.push(entry->most_recent_ip);
				buffer.push(entry->last_game);
				buffer.push(entry->most_recent_name);

				// push buffer to file
				buffer.push_to(file);

				// iterate
				entry = entry->next;
			}
			fclose(file);
		}
	}
}

void RenX_LadderDatabase::sort_entries()
{
	if (RenX_LadderDatabase::entries <= 1)
		return;

	RenX_LadderDatabase::Entry *itr = RenX_LadderDatabase::head;
	RenX_LadderDatabase::Entry *itr2, *ptr;

	// iterate forward (search for out-of-order content)
	while (itr->next != nullptr)
	{
		// out-of-order content found
		if (itr->next->total_score > itr->total_score)
		{
			// pull content out
			ptr = itr->next;
			itr->next = ptr->next;
			if (itr->next != nullptr)
				itr->next->prev = itr;

			// iterate backwards from our iterator, and insert
			itr2 = itr;
			while (true)
			{
				if (itr2->prev == nullptr)
				{
					// push ptr to head
					ptr->next = itr2;
					ptr->prev = nullptr;
					itr2->prev = ptr;
					RenX_LadderDatabase::head = ptr;
					break;
				}
				itr2 = itr2->prev;
				if (itr2->total_score > ptr->total_score)
				{
					// insert ptr after itr2
					ptr->next = itr2->next;
					ptr->next->prev = ptr;
					ptr->prev = itr2;
					itr2->next = ptr;
					break;
				}
			}
		}
		else // continue iterating
			itr = itr->next;
	}

	RenX_LadderDatabase::end = itr;
}