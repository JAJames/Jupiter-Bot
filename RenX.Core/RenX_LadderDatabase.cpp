/**
* Copyright (C) 2015-2016 Jessica James.
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

#include "RenX_LadderDatabase.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"

Jupiter::ArrayList<RenX::LadderDatabase> _ladder_databases;
Jupiter::ArrayList<RenX::LadderDatabase> &RenX::ladder_databases = _ladder_databases;

RenX::LadderDatabase::LadderDatabase()
{
	_ladder_databases.add(this);
}

RenX::LadderDatabase::~LadderDatabase()
{
	while (RenX::LadderDatabase::head != nullptr)
	{
		RenX::LadderDatabase::end = RenX::LadderDatabase::head;
		RenX::LadderDatabase::head = RenX::LadderDatabase::head->next;
		delete RenX::LadderDatabase::end;
	}
}

void RenX::LadderDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos)
{
	RenX::LadderDatabase::Entry *entry = new RenX::LadderDatabase::Entry();

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
	if (RenX::LadderDatabase::head == nullptr)
	{
		RenX::LadderDatabase::head = entry;
		RenX::LadderDatabase::end = RenX::LadderDatabase::head;
	}
	else
	{
		RenX::LadderDatabase::end->next = entry;
		entry->prev = end;
		end = entry;
	}

	entry->rank = ++RenX::LadderDatabase::entries;
}

void RenX::LadderDatabase::process_header(FILE *file)
{
	int chr = fgetc(file);
	if (chr != EOF)
		RenX::LadderDatabase::read_version = chr;
}

void RenX::LadderDatabase::create_header(FILE *file)
{
	fputc(RenX::LadderDatabase::write_version, file);
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getHead() const
{
	return RenX::LadderDatabase::head;
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntry(uint64_t steamid) const
{
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next)
		if (itr->steam_id == steamid)
			return itr;
	return nullptr;
}

std::pair<RenX::LadderDatabase::Entry *, size_t> RenX::LadderDatabase::getPlayerEntryAndIndex(uint64_t steamid) const
{
	size_t index = 0;
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
		if (itr->steam_id == steamid)
			return std::pair<RenX::LadderDatabase::Entry *, size_t>(itr, index);
	return std::pair<RenX::LadderDatabase::Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntryByName(const Jupiter::ReadableString &name) const
{
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next)
		if (itr->most_recent_name.equalsi(name))
			return itr;
	return nullptr;
}

std::pair<RenX::LadderDatabase::Entry *, size_t> RenX::LadderDatabase::getPlayerEntryAndIndexByName(const Jupiter::ReadableString &name) const
{
	size_t index = 0;
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
		if (itr->most_recent_name.equalsi(name))
			return std::pair<RenX::LadderDatabase::Entry *, size_t>(itr, index);
	return std::pair<RenX::LadderDatabase::Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntryByPartName(const Jupiter::ReadableString &name) const
{
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next)
		if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			return itr;
	return nullptr;
}

std::pair<RenX::LadderDatabase::Entry *, size_t> RenX::LadderDatabase::getPlayerEntryAndIndexByPartName(const Jupiter::ReadableString &name) const
{
	size_t index = 0;
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
		if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			return std::pair<RenX::LadderDatabase::Entry *, size_t>(itr, index);
	return std::pair<RenX::LadderDatabase::Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

Jupiter::SLList<RenX::LadderDatabase::Entry> RenX::LadderDatabase::getPlayerEntriesByPartName(const Jupiter::ReadableString &name, size_t max) const
{
	Jupiter::SLList<RenX::LadderDatabase::Entry> list;
	if (max == 0)
	{
		for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
				list.add(new RenX::LadderDatabase::Entry(*itr));
	}
	else
		for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			{
				list.add(new RenX::LadderDatabase::Entry(*itr));
				if (--max == 0)
					return list;
			}
	return list;
}

Jupiter::SLList<std::pair<RenX::LadderDatabase::Entry, size_t>> RenX::LadderDatabase::getPlayerEntriesAndIndexByPartName(const Jupiter::ReadableString &name, size_t max) const
{
	Jupiter::SLList<std::pair<RenX::LadderDatabase::Entry, size_t>> list;
	size_t index = 0;
	if (max == 0)
	{
		for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
				list.add(new std::pair<RenX::LadderDatabase::Entry, size_t>(*itr, index));
	}
	else
		for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next, ++index)
			if (itr->most_recent_name.findi(name) != Jupiter::INVALID_INDEX)
			{
				list.add(new std::pair<RenX::LadderDatabase::Entry, size_t>(*itr, index));
				if (--max)
					return list;
			}
	return list;
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntryByIndex(size_t index) const
{
	for (RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head; itr != nullptr; itr = itr->next, --index)
		if (index == 0)
			return itr;
	return nullptr;
}

size_t RenX::LadderDatabase::getEntries() const
{
	return RenX::LadderDatabase::entries;
}

std::chrono::steady_clock::time_point RenX::LadderDatabase::getLastSortTime() const
{
	return RenX::LadderDatabase::last_sort;
}

void RenX::LadderDatabase::append(RenX::LadderDatabase::Entry *entry)
{
	++RenX::LadderDatabase::entries;
	if (RenX::LadderDatabase::head == nullptr)
	{
		RenX::LadderDatabase::head = entry;
		RenX::LadderDatabase::end = RenX::LadderDatabase::head;
		return;
	}
	RenX::LadderDatabase::end->next = entry;
	entry->prev = RenX::LadderDatabase::end;
	RenX::LadderDatabase::end = entry;
}

void RenX::LadderDatabase::write(const Jupiter::CStringType &filename)
{
	return RenX::LadderDatabase::write(filename.c_str());
}

void RenX::LadderDatabase::write(const char *filename)
{
	if (RenX::LadderDatabase::entries != 0)
	{
		FILE *file = fopen(filename, "wb");
		if (file != nullptr)
		{
			size_t rank = 0;
			Jupiter::DataBuffer buffer;
			RenX::LadderDatabase::create_header(file);
			RenX::LadderDatabase::Entry *entry = RenX::LadderDatabase::head;
			while (entry != nullptr)
			{
				// update rank
				entry->rank = ++rank;

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

void RenX::LadderDatabase::sort_entries()
{
	if (RenX::LadderDatabase::entries <= 1)
		return;

	RenX::LadderDatabase::Entry *itr = RenX::LadderDatabase::head;
	RenX::LadderDatabase::Entry *itr2, *ptr;

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
					RenX::LadderDatabase::head = ptr;
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

	RenX::LadderDatabase::end = itr;
	RenX::LadderDatabase::last_sort = std::chrono::steady_clock::now();
}

void RenX::LadderDatabase::updateLadder(RenX::Server *server, const RenX::TeamType &team, bool output_times)
{
	if (server->players.size() != 0)
	{
		// update player stats in memory
		RenX::PlayerInfo *player;
		RenX::LadderDatabase::Entry *entry;
		for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
		{
			player = node->data;
			if (player->steamid != 0 && (player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_LADDER) == 0)
			{
				entry = RenX::LadderDatabase::getPlayerEntry(player->steamid);
				if (entry == nullptr)
				{
					entry = new RenX::LadderDatabase::Entry();
					RenX::LadderDatabase::append(entry);
					entry->steam_id = player->steamid;
				}

				entry->total_score += static_cast<uint64_t>(player->score);

				entry->total_kills += player->kills;
				entry->total_deaths += player->deaths;
				entry->total_headshot_kills += player->headshots;
				entry->total_vehicle_kills += player->vehicleKills;
				entry->total_building_kills += player->buildingKills;
				entry->total_defence_kills += player->defenceKills;
				entry->total_captures += player->captures;
				entry->total_game_time += static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server->getGameTime(player)).count());
				++entry->total_games;
				switch (player->team)
				{
				case RenX::TeamType::GDI:
					++entry->total_gdi_games;
					if (player->team == team)
						++entry->total_wins, ++entry->total_gdi_wins;
					break;
				case RenX::TeamType::Nod:
					++entry->total_nod_games;
					if (player->team == team)
						++entry->total_wins, ++entry->total_nod_wins;
					break;
				default:
					if (player->team == team)
						++entry->total_wins;
					break;
				}
				entry->total_beacon_placements += player->beaconPlacements;
				entry->total_beacon_disarms += player->beaconDisarms;
				entry->total_proxy_placements += player->proxy_placements;
				entry->total_proxy_disarms += player->proxy_disarms;

				auto set_if_greater = [](uint32_t &src, const uint32_t &cmp)
				{
					if (cmp > src)
						src = cmp;
				};

				set_if_greater(entry->top_score, static_cast<uint32_t>(player->score));
				set_if_greater(entry->top_kills, player->kills);
				set_if_greater(entry->most_deaths, player->deaths);
				set_if_greater(entry->top_headshot_kills, player->headshots);
				set_if_greater(entry->top_vehicle_kills, player->vehicleKills);
				set_if_greater(entry->top_building_kills, player->buildingKills);
				set_if_greater(entry->top_defence_kills, player->defenceKills);
				set_if_greater(entry->top_captures, player->captures);
				set_if_greater(entry->top_game_time, static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server->getGameTime(player)).count()));
				set_if_greater(entry->top_beacon_placements, player->beaconPlacements);
				set_if_greater(entry->top_beacon_disarms, player->beaconDisarms);
				set_if_greater(entry->top_proxy_placements, player->proxy_placements);
				set_if_greater(entry->top_proxy_disarms, player->proxy_disarms);

				entry->most_recent_ip = player->ip32;
				entry->last_game = time(nullptr);
				entry->most_recent_name = player->name;
			}
		}

		// sort new stats
		std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
		RenX::LadderDatabase::sort_entries();
		std::chrono::steady_clock::duration sort_duration = std::chrono::steady_clock::now() - start_time;

		// write new stats
		start_time = std::chrono::steady_clock::now();
		RenX::LadderDatabase::write(this->getFilename());
		std::chrono::steady_clock::duration write_duration = std::chrono::steady_clock::now() - start_time;

		if (output_times)
		{
			Jupiter::StringS str = Jupiter::StringS::Format("Ladder: %u entries sorted in %f seconds; Database written in %f seconds." ENDL,
				RenX::LadderDatabase::getEntries(),
				static_cast<double>(sort_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num))),
				static_cast<double>(write_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num) / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num)));
			str.println(stdout);
			server->sendLogChan(str);
		}
	}
}
