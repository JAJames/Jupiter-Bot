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

#if !defined _RENX_LADDERDATABASE_H_HEADER
#define _RENX_LADDERDATABASE_H_HEADER

#include <chrono>
#include "Jupiter/Database.h"
#include "Jupiter/String.h"
#include "Jupiter/SLList.h"
#include "Jupiter/ArrayList.h"
#include "RenX.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	class Server;

	class RENX_API LadderDatabase : public Jupiter::Database
	{
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

	public: // LadderDatabase
		struct RENX_API Entry
		{
			size_t rank;

			uint64_t steam_id, total_score;
			uint32_t total_kills, total_deaths, total_headshot_kills, total_vehicle_kills, total_building_kills, total_defence_kills, total_captures, total_game_time, total_games, total_gdi_games, total_nod_games, total_wins, total_gdi_wins, total_nod_wins, total_beacon_placements, total_beacon_disarms, total_proxy_placements, total_proxy_disarms, // totals (15)
				top_score, top_kills, most_deaths, top_headshot_kills, top_vehicle_kills, top_building_kills, top_defence_kills, top_captures, top_game_time, top_beacon_placements, top_beacon_disarms, top_proxy_placements, top_proxy_disarms, // tops (12)
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
		Jupiter::SLList<Entry> getPlayerEntriesByPartName(const Jupiter::ReadableString &name, size_t max) const;
		Jupiter::SLList<std::pair<Entry, size_t>> getPlayerEntriesAndIndexByPartName(const Jupiter::ReadableString &name, size_t max) const;

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
		void write(const Jupiter::CStringType &filename);
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
		void updateLadder(RenX::Server *server, const RenX::TeamType &team, bool output_times);

		/**
		* @brief Erases all entries in the database.
		*/
		void erase();

		/**
		* @brief Constructor for the LadderDatabase class
		*/
		LadderDatabase();

		/**
		* @brief Deconstructor for the LadderDatabase class
		*/
		~LadderDatabase();
	private:

		/** Database version */
		const uint8_t write_version = 0;
		uint8_t read_version = write_version;

		std::chrono::steady_clock::time_point last_sort = std::chrono::steady_clock::now();
		size_t entries = 0;
		Entry *head = nullptr;
		Entry *end;
	};

	RENX_API extern Jupiter::ArrayList<RenX::LadderDatabase> &ladder_databases;
}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif //_RENX_LADDERDATABASE_H_HEADER