/**
 * Copyright (C) 2015-2021 Jessica James.
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

#include "jessilib/unicode.hpp"
#include "RenX_LadderDatabase.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"

RenX::LadderDatabase *RenX::default_ladder_database = nullptr;
std::vector<RenX::LadderDatabase*> g_ladder_databases;
std::vector<RenX::LadderDatabase*>& RenX::ladder_databases = g_ladder_databases;

RenX::LadderDatabase::LadderDatabase() {
	g_ladder_databases.push_back(this);

	if (RenX::default_ladder_database == nullptr) {
		RenX::default_ladder_database = this;
	}
}

RenX::LadderDatabase::LadderDatabase(std::string_view in_name) : LadderDatabase() {
	RenX::LadderDatabase::setName(in_name);
}

RenX::LadderDatabase::~LadderDatabase() {
	while (m_head != nullptr) {
		m_end = m_head;
		m_head = m_head->next;
		delete m_end;
	}

	for (auto itr = g_ladder_databases.begin(); itr != g_ladder_databases.end(); ++itr) {
		if (*itr == this) {
			g_ladder_databases.erase(itr);
			break;
		}
	}

	if (RenX::default_ladder_database == this) {
		if (g_ladder_databases.empty()) {
			RenX::default_ladder_database = nullptr;
		}
		else {
			RenX::default_ladder_database = g_ladder_databases[0];
		}
	}
}

void RenX::LadderDatabase::process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos) {
	Entry *entry = new Entry();

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
	if (m_read_version == 0)
	{
		entry->total_gdi_games = buffer.pop<uint32_t>();
		entry->total_nod_games = buffer.pop<uint32_t>();
	}
	entry->total_wins = buffer.pop<uint32_t>();
	if (m_read_version == 0)
	{
		entry->total_gdi_wins = buffer.pop<uint32_t>();
		entry->total_nod_wins = buffer.pop<uint32_t>();
	}
	entry->total_beacon_placements = buffer.pop<uint32_t>();
	entry->total_beacon_disarms = buffer.pop<uint32_t>();
	entry->total_proxy_placements = buffer.pop<uint32_t>();
	entry->total_proxy_disarms = buffer.pop<uint32_t>();

	if (m_read_version > 0)
	{
		entry->total_gdi_games = buffer.pop<uint32_t>();
		entry->total_gdi_wins = buffer.pop<uint32_t>();
		entry->total_gdi_ties = buffer.pop<uint32_t>();
		entry->total_gdi_game_time = buffer.pop<uint32_t>();
		entry->total_gdi_score = buffer.pop<uint64_t>();
		entry->total_gdi_beacon_placements = buffer.pop<uint32_t>();
		entry->total_gdi_beacon_disarms = buffer.pop<uint32_t>();
		entry->total_gdi_proxy_placements = buffer.pop<uint32_t>();
		entry->total_gdi_proxy_disarms = buffer.pop<uint32_t>();
		entry->total_gdi_kills = buffer.pop<uint32_t>();
		entry->total_gdi_deaths = buffer.pop<uint32_t>();
		entry->total_gdi_vehicle_kills = buffer.pop<uint32_t>();
		entry->total_gdi_defence_kills = buffer.pop<uint32_t>();
		entry->total_gdi_building_kills = buffer.pop<uint32_t>();
		entry->total_gdi_headshots = buffer.pop<uint32_t>();

		entry->total_nod_games = buffer.pop<uint32_t>();
		entry->total_nod_wins = buffer.pop<uint32_t>();
		entry->total_nod_game_time = buffer.pop<uint32_t>();
		entry->total_nod_score = buffer.pop<uint64_t>();
		entry->total_nod_beacon_placements = buffer.pop<uint32_t>();
		entry->total_nod_beacon_disarms = buffer.pop<uint32_t>();
		entry->total_nod_proxy_placements = buffer.pop<uint32_t>();
		entry->total_nod_proxy_disarms = buffer.pop<uint32_t>();
		entry->total_nod_kills = buffer.pop<uint32_t>();
		entry->total_nod_deaths = buffer.pop<uint32_t>();
		entry->total_nod_vehicle_kills = buffer.pop<uint32_t>();
		entry->total_nod_defence_kills = buffer.pop<uint32_t>();
		entry->total_nod_building_kills = buffer.pop<uint32_t>();
		entry->total_nod_headshots = buffer.pop<uint32_t>();
	}

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
	if (m_head == nullptr) {
		m_head = entry;
		m_end = m_head;
	}
	else {
		m_end->next = entry;
		entry->prev = m_end;
		m_end = entry;
	}

	entry->rank = ++m_entries;
}

void RenX::LadderDatabase::process_header(FILE *file) {
	int chr = fgetc(file);
	if (chr != EOF) {
		m_read_version = chr;
	}
}

void RenX::LadderDatabase::create_header(FILE *file) {
	fputc(m_write_version, file);
}

void RenX::LadderDatabase::process_file_finish(FILE *file) {
	if (m_read_version != m_write_version) {
		puts("Notice: Ladder database is out of date; upgrading...");
		std::chrono::steady_clock::duration write_duration;
		std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

		write(this->getFilename());

		write_duration = std::chrono::steady_clock::now() - start_time;
		printf("Ladder database upgrade completed in %f seconds", static_cast<double>(write_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num) / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num)));
		m_read_version = m_write_version;
	}
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getHead() const {
	return m_head;
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntry(uint64_t steamid) const {
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next) {
		if (itr->steam_id == steamid) {
			return itr;
		}
	}

	return nullptr;
}

std::pair<RenX::LadderDatabase::Entry *, size_t> RenX::LadderDatabase::getPlayerEntryAndIndex(uint64_t steamid) const {
	size_t index = 0;
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next, ++index) {
		if (itr->steam_id == steamid) {
			return std::pair<Entry*, size_t>(itr, index);
		}
	}
	return std::pair<Entry*, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntryByName(std::string_view name) const {
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next) {
		if (jessilib::equalsi(itr->most_recent_name, name)) {
			return itr;
		}
	}

	return nullptr;
}

std::pair<RenX::LadderDatabase::Entry *, size_t> RenX::LadderDatabase::getPlayerEntryAndIndexByName(std::string_view name) const {
	size_t index = 0;
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next, ++index) {
		if (jessilib::equalsi(itr->most_recent_name, name)) {
			return std::pair<Entry*, size_t>(itr, index);
		}
	}

	return std::pair<Entry*, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntryByPartName(std::string_view name) const {
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next) {
		if (jessilib::findi(itr->most_recent_name, name) != std::string::npos) {
			return itr;
		}
	}

	return nullptr;
}

std::pair<RenX::LadderDatabase::Entry *, size_t> RenX::LadderDatabase::getPlayerEntryAndIndexByPartName(std::string_view name) const {
	size_t index = 0;
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next, ++index) {
		if (jessilib::findi(itr->most_recent_name, name) != std::string::npos) {
			return std::pair<RenX::LadderDatabase::Entry*, size_t>(itr, index);
		}
	}

	return std::pair<Entry *, size_t>(nullptr, Jupiter::INVALID_INDEX);
}

std::forward_list<RenX::LadderDatabase::Entry> RenX::LadderDatabase::getPlayerEntriesByPartName(std::string_view name, size_t max) const {
	std::forward_list<Entry> list;
	if (max == 0) {
		for (Entry *itr = m_head; itr != nullptr; itr = itr->next) {
			if (jessilib::findi(itr->most_recent_name, name) != std::string::npos) {
				list.emplace_front(*itr);
			}
		}
	}
	else {
		for (Entry* itr = m_head; itr != nullptr; itr = itr->next) {
			if (jessilib::findi(itr->most_recent_name, name) != std::string::npos) {
				list.emplace_front(*itr);
				if (--max == 0) {
					return list;
				}
			}
		}
	}
	return list;
}

std::forward_list<std::pair<RenX::LadderDatabase::Entry, size_t>> RenX::LadderDatabase::getPlayerEntriesAndIndexByPartName(std::string_view name, size_t max) const {
	std::forward_list<std::pair<Entry, size_t>> list;
	size_t index = 0;
	if (max == 0)
	{
		for (Entry *itr = m_head; itr != nullptr; itr = itr->next, ++index) {
			if (jessilib::findi(itr->most_recent_name, name) != std::string::npos) {
				list.emplace_front(*itr, index);
			}
		}
	}
	else {
		for (Entry* itr = m_head; itr != nullptr; itr = itr->next, ++index) {
			if (jessilib::findi(itr->most_recent_name, name) != std::string::npos) {
				list.emplace_front(*itr, index);
				if (--max) {
					return list;
				}
			}
		}
	}
	return list;
}

RenX::LadderDatabase::Entry *RenX::LadderDatabase::getPlayerEntryByIndex(size_t index) const {
	for (Entry *itr = m_head; itr != nullptr; itr = itr->next, --index) {
		if (index == 0) {
			return itr;
		}
	}
	return nullptr;
}

size_t RenX::LadderDatabase::getEntries() const {
	return m_entries;
}

std::chrono::steady_clock::time_point RenX::LadderDatabase::getLastSortTime() const {
	return m_last_sort;
}

void RenX::LadderDatabase::append(Entry *entry) {
	++m_entries;
	if (m_head == nullptr) {
		m_head = entry;
		m_end = m_head;
		return;
	}
	m_end->next = entry;
	entry->prev = m_end;
	m_end = entry;
}

void RenX::LadderDatabase::write(const std::string &filename) {
	return write(filename.c_str());
}

void RenX::LadderDatabase::write(const char *filename) {
	if (m_entries != 0)
	{
		FILE *file = fopen(filename, "wb");
		if (file != nullptr)
		{
			size_t rank = 0;
			Jupiter::DataBuffer buffer;
			create_header(file);
			Entry *entry = m_head;
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
				buffer.push(entry->total_wins);
				buffer.push(entry->total_beacon_placements);
				buffer.push(entry->total_beacon_disarms);
				buffer.push(entry->total_proxy_placements);
				buffer.push(entry->total_proxy_disarms);

				buffer.push(entry->total_gdi_games);
				buffer.push(entry->total_gdi_wins);
				buffer.push(entry->total_gdi_ties);
				buffer.push(entry->total_gdi_game_time);
				buffer.push(entry->total_gdi_score);
				buffer.push(entry->total_gdi_beacon_placements);
				buffer.push(entry->total_gdi_beacon_disarms);
				buffer.push(entry->total_gdi_proxy_placements);
				buffer.push(entry->total_gdi_proxy_disarms);
				buffer.push(entry->total_gdi_kills);
				buffer.push(entry->total_gdi_deaths);
				buffer.push(entry->total_gdi_vehicle_kills);
				buffer.push(entry->total_gdi_defence_kills);
				buffer.push(entry->total_gdi_building_kills);
				buffer.push(entry->total_gdi_headshots);

				buffer.push(entry->total_nod_games);
				buffer.push(entry->total_nod_wins);
				buffer.push(entry->total_nod_game_time);
				buffer.push(entry->total_nod_score);
				buffer.push(entry->total_nod_beacon_placements);
				buffer.push(entry->total_nod_beacon_disarms);
				buffer.push(entry->total_nod_proxy_placements);
				buffer.push(entry->total_nod_proxy_disarms);
				buffer.push(entry->total_nod_kills);
				buffer.push(entry->total_nod_deaths);
				buffer.push(entry->total_nod_vehicle_kills);
				buffer.push(entry->total_nod_defence_kills);
				buffer.push(entry->total_nod_building_kills);
				buffer.push(entry->total_nod_headshots);

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

void RenX::LadderDatabase::sort_entries() {
	if (m_entries <= 1) {
		return;
	}

	Entry *itr = m_head;
	Entry *itr2, *ptr;

	// iterate forward (search for out-of-order content)
	while (itr->next != nullptr) {
		// out-of-order content found
		if (itr->next->total_score > itr->total_score) {
			// pull content out
			ptr = itr->next;
			itr->next = ptr->next;
			if (itr->next != nullptr) {
				itr->next->prev = itr;
			}

			// iterate backwards from our iterator, and insert
			itr2 = itr;
			while (true) {
				if (itr2->prev == nullptr) {
					// push ptr to head
					ptr->next = itr2;
					ptr->prev = nullptr;
					itr2->prev = ptr;
					m_head = ptr;
					break;
				}
				itr2 = itr2->prev;
				if (itr2->total_score > ptr->total_score) {
					// insert ptr after itr2
					ptr->next = itr2->next;
					ptr->next->prev = ptr;
					ptr->prev = itr2;
					itr2->next = ptr;
					break;
				}
			}
		}
		else { // continue iterating
			itr = itr->next;
		}
	}

	m_end = itr;
	m_last_sort = std::chrono::steady_clock::now();
}

void RenX::LadderDatabase::updateLadder(RenX::Server &server, const RenX::TeamType &team) {
	if (server.players.size() != server.getBotCount()) {
		// call the PreUpdateLadder event
		if (this->OnPreUpdateLadder != nullptr) {
			this->OnPreUpdateLadder(*this, server, team);
		}

		// update player stats in memory
		Entry *entry;
		for (auto player = server.players.begin(); player != server.players.end(); ++player) {
			if (player->steamid != 0 && (player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_LADDER) == 0) {
				entry = getPlayerEntry(player->steamid);
				if (entry == nullptr) {
					entry = new Entry();
					append(entry);
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
				entry->total_game_time += static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server.getGameTime(*player)).count());
				entry->total_beacon_placements += player->beaconPlacements;
				entry->total_beacon_disarms += player->beaconDisarms;
				entry->total_proxy_placements += player->proxy_placements;
				entry->total_proxy_disarms += player->proxy_disarms;

				++entry->total_games;
				switch (player->team) {
				case RenX::TeamType::GDI:
					++entry->total_gdi_games;
					if (player->team == team)
						++entry->total_wins, ++entry->total_gdi_wins;
					else if (team == RenX::TeamType::None)
						++entry->total_gdi_ties;

					entry->total_gdi_game_time += static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server.getGameTime(*player)).count());
					entry->total_gdi_score += static_cast<uint64_t>(player->score);
					entry->total_gdi_beacon_placements += player->beaconPlacements;
					entry->total_gdi_beacon_disarms += player->beaconDisarms;
					entry->total_gdi_proxy_placements += player->proxy_placements;
					entry->total_gdi_proxy_disarms += player->proxy_disarms;
					entry->total_gdi_kills += player->kills;
					entry->total_gdi_deaths += player->deaths;
					entry->total_gdi_vehicle_kills += player->vehicleKills;
					entry->total_gdi_defence_kills += player->defenceKills;
					entry->total_gdi_building_kills += player->buildingKills;
					entry->total_gdi_headshots += player->headshots;
					break;
				case RenX::TeamType::Nod:
					++entry->total_nod_games;
					if (player->team == team)
						++entry->total_wins, ++entry->total_nod_wins;
					else if (team == RenX::TeamType::None)
						++entry->total_nod_ties;

					entry->total_nod_game_time += static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server.getGameTime(*player)).count());
					entry->total_nod_score += static_cast<uint64_t>(player->score);
					entry->total_nod_beacon_placements += player->beaconPlacements;
					entry->total_nod_beacon_disarms += player->beaconDisarms;
					entry->total_nod_proxy_placements += player->proxy_placements;
					entry->total_nod_proxy_disarms += player->proxy_disarms;
					entry->total_nod_kills += player->kills;
					entry->total_nod_deaths += player->deaths;
					entry->total_nod_vehicle_kills += player->vehicleKills;
					entry->total_nod_defence_kills += player->defenceKills;
					entry->total_nod_building_kills += player->buildingKills;
					entry->total_nod_headshots += player->headshots;
					break;
				default:
					if (player->team == team)
						++entry->total_wins;
					break;
				}

				auto set_if_greater = [](uint32_t &src, const uint32_t &cmp) {
					if (cmp > src) {
						src = cmp;
					}
				};

				set_if_greater(entry->top_score, static_cast<uint32_t>(player->score));
				set_if_greater(entry->top_kills, player->kills);
				set_if_greater(entry->most_deaths, player->deaths);
				set_if_greater(entry->top_headshot_kills, player->headshots);
				set_if_greater(entry->top_vehicle_kills, player->vehicleKills);
				set_if_greater(entry->top_building_kills, player->buildingKills);
				set_if_greater(entry->top_defence_kills, player->defenceKills);
				set_if_greater(entry->top_captures, player->captures);
				set_if_greater(entry->top_game_time, static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server.getGameTime(*player)).count()));
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
		sort_entries();
		std::chrono::steady_clock::duration sort_duration = std::chrono::steady_clock::now() - start_time;

		// write new stats
		start_time = std::chrono::steady_clock::now();
		write(this->getFilename());
		std::chrono::steady_clock::duration write_duration = std::chrono::steady_clock::now() - start_time;

		if (m_output_times)
		{
			Jupiter::StringS str = string_printf("Ladder: %zu entries sorted in %f seconds; Database written in %f seconds." ENDL,
				getEntries(),
				static_cast<double>(sort_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num) / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num)),
				static_cast<double>(write_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num) / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num)));
			std::cout << std::string_view{str} << std::endl;
			server.sendLogChan(str);
		}
	}
}

void RenX::LadderDatabase::erase() {
	if (m_head != nullptr) {
		m_entries = 0;
		while (m_head->next != nullptr) {
			m_head = m_head->next;
			delete m_head->prev;
		}

		delete m_head;
		m_head = nullptr;
		m_end = nullptr;
	}
}

std::string_view RenX::LadderDatabase::getName() const {
	return m_name;
}

void RenX::LadderDatabase::setName(std::string_view in_name) {
	m_name = in_name;
}

bool RenX::LadderDatabase::getOutputTimes() const {
	return m_output_times;
}

void RenX::LadderDatabase::setOutputTimes(bool in_output_times) {
	m_output_times = in_output_times;
}
