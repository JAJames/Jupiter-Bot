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

#if !defined _RENX_LADDERDATABASE_H_HEADER
#define _RENX_LADDERDATABASE_H_HEADER

#include <chrono>
#include <forward_list>
#include "Jupiter/Database.h"
#include "Jupiter/String.hpp"
#include "RenX.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX {
	class Server;

	class RENX_API LadderDatabase : public Jupiter::Database {
	public: // Jupiter::Database

		/**
		* @brief Processes a chunk of data in a database.
		*
		* @param buffer Buffer to process
		* @param file File being processed
		* @param pos position that the buffer starts at in the file
		*/
		void process_data(Jupiter::DataBuffer &buffer, FILE *file, fpos_t pos) override;

		/**
		* @brief Processes the header for a database.
		*
		* @param file File being processed
		*/
		void process_header(FILE *file) override;

		/**
		* @brief Generates a header for a database.
		*
		* @param file File being created
		*/
		void create_header(FILE *file) override;

		/**
		* @brief Called when process_file() is successfully completed.
		*
		* @param file File being processed
		*/
		void process_file_finish(FILE *file) override;

	public: // LadderDatabase
		struct RENX_API Entry
		{
			size_t rank;

			uint64_t steam_id, total_score, total_gdi_score, total_nod_score; // 64-bit fields (4)
			uint32_t total_kills, total_deaths, total_headshot_kills, total_vehicle_kills, total_building_kills, total_defence_kills, total_captures, total_game_time, total_games, total_wins, total_beacon_placements, total_beacon_disarms, total_proxy_placements, total_proxy_disarms, // totals (14)
				total_gdi_games, total_gdi_wins, total_gdi_ties, total_gdi_game_time, total_gdi_beacon_placements, total_gdi_beacon_disarms, total_gdi_proxy_placements, total_gdi_proxy_disarms, total_gdi_kills, total_gdi_deaths, total_gdi_vehicle_kills, total_gdi_defence_kills, total_gdi_building_kills, total_gdi_headshots, // GDI totals (14)
				total_nod_games, total_nod_wins, total_nod_ties, total_nod_game_time, total_nod_beacon_placements, total_nod_beacon_disarms, total_nod_proxy_placements, total_nod_proxy_disarms, total_nod_kills, total_nod_deaths, total_nod_vehicle_kills, total_nod_defence_kills, total_nod_building_kills, total_nod_headshots, // Nod totals (14)
				top_score, top_kills, most_deaths, top_headshot_kills, top_vehicle_kills, top_building_kills, top_defence_kills, top_captures, top_game_time, top_beacon_placements, top_beacon_disarms, top_proxy_placements, top_proxy_disarms, // tops (13)
				most_recent_ip; // other (1)
			time_t last_game;
			Jupiter::StringS most_recent_name;
			Entry *next = nullptr;
			Entry *prev = nullptr;
		};

		/**
		* @brief Fetches the head of the entry list.
		*
		* @return Head of the ladder entry list.
		*/
		Entry *getHead() const;

		/**
		* @brief Fetches a ladder entry by Steam ID.
		*
		* @param steamid Steam ID to search ladder for
		* @return Ladder entry with a matching steamid if one exists, nullptr otherwise.
		*/
		Entry *getPlayerEntry(uint64_t steamid) const;
		std::pair<Entry *, size_t> getPlayerEntryAndIndex(uint64_t steamid) const;

		/**
		* @brief Searches for a ladder entry by name
		*
		* @param name Name to search ladder for
		* @return Ladder entry with a matching name if one exists, nullptr otherwise.
		*/
		Entry *getPlayerEntryByName(const Jupiter::ReadableString &name) const;
		std::pair<Entry *, size_t> getPlayerEntryAndIndexByName(const Jupiter::ReadableString &name) const;

		/**
		* @brief Searches for a ladder entry by part name
		*
		* @param name Part of name to search ladder for
		* @return Ladder entry with a matching name if one exists, nullptr otherwise.
		*/
		Entry *getPlayerEntryByPartName(const Jupiter::ReadableString &name) const;
		std::pair<Entry *, size_t> getPlayerEntryAndIndexByPartName(const Jupiter::ReadableString &name) const;

		/**
		* @brief Fetches all entries matching a part name.
		*
		* @param name Part of name to search for
		* @param max Maximum number of entries to return
		* @return List containing entries with matching names.
		*/
		std::forward_list<Entry> getPlayerEntriesByPartName(const Jupiter::ReadableString &name, size_t max) const;
		std::forward_list<std::pair<Entry, size_t>> getPlayerEntriesAndIndexByPartName(const Jupiter::ReadableString &name, size_t max) const;

		/**
		* @brief Fetches a ladder entry at a specified index
		*
		* @param index Index of the element to fetch
		* @return Ladder entry with a matching name if one exists, nullptr otherwise.
		*/
		Entry *getPlayerEntryByIndex(size_t index) const;

		/**
		* @brief Fetches the total number of ladder entries in the list.
		*
		* @return Total number of entries.
		*/
		size_t getEntries() const;

		/**
		* @brief Returns the last time that the contents of this database object were modified (i.e: initialized or written).
		*/
		std::chrono::steady_clock::time_point getLastSortTime() const;

		/**
		* @brief Places a ladder entry at the end of the list, regardless of order
		* Note: This does not copy data from the pointer -- the pointer is added to the list.
		*
		* @param entry Ladder entry to add
		*/
		void append(Entry *entry);

		/**
		* @brief Writes the current ladder data to the disk.
		*/
		void write(const std::string &filename);
		void write(const char *filename);

		/**
		* @brief Sorts the ladder data in memory.
		*/
		void sort_entries();

		/**
		* @brief Pushes the player data from the server into the ladder, sorts the data, and writes it to file storage.
		*
		* @param server Renegade-X server to pull player data from
		* @param team Team which just won
		* @param output_times True if the sort/write times should be output, false otherwise.
		*/
		void updateLadder(RenX::Server &server, const RenX::TeamType &team);

		/**
		* @brief Erases all entries in the database.
		*/
		void erase();

		/**
		* @brief Gets the name of this database.
		*/
		const Jupiter::ReadableString &getName() const;

		/**
		* @brief Sets the name of this database.
		*/
		void setName(const Jupiter::ReadableString &in_name);

		/**
		* @brief Checks if this database outputs sort/write times when 'updateLadder' is called.
		*
		* @return True if output times get printed, false otherwise.
		*/
		bool getOutputTimes() const;

		/**
		* @brief Sets this database to output sort/write times, or to not output sort/write times.
		*
		* @param in_output_times True to output sort/write times, false otherwise.
		*/
		void setOutputTimes(bool in_output_times);

		/**
		* @brief Constructor for the LadderDatabase class
		*/
		LadderDatabase();

		/**
		* @brief Named constructor for the LadderDatabase class
		*/
		LadderDatabase(const Jupiter::ReadableString &in_name);

		/**
		* @brief Deconstructor for the LadderDatabase class
		*/
		~LadderDatabase();

		typedef void PreUpdateLadderFunction(RenX::LadderDatabase &database, RenX::Server &server, const RenX::TeamType &team);
		PreUpdateLadderFunction *OnPreUpdateLadder = nullptr;

	private:
		/** Database version */
		const uint8_t m_write_version = 1;
		uint8_t m_read_version = m_write_version;

		bool m_output_times = false;
		Jupiter::StringS m_name;
		std::chrono::steady_clock::time_point m_last_sort = std::chrono::steady_clock::now();
		size_t m_entries = 0;
		Entry* m_head = nullptr;
		Entry* m_end = nullptr;
	};

	RENX_API extern RenX::LadderDatabase *default_ladder_database;
	RENX_API extern std::vector<RenX::LadderDatabase*>& ladder_databases;
}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif //_RENX_LADDERDATABASE_H_HEADER