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

#if !defined _RENX_LADDERDATABASE_H_HEADER
#define _RENX_LADDERDATABASE_H_HEADER

#include "Jupiter/Database.h"
#include "Jupiter/String.h"
#include "Jupiter/SLList.h"

class RenX_LadderDatabase : public Jupiter::Database
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

public: // RenX_LadderDatabase
	struct Entry
	{
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
	* @brief Deconstructor for the RenX_LadderDatabase class
	*/
	~RenX_LadderDatabase();
private:

	/** Database version */
	const uint8_t write_version = 0;
	uint8_t read_version = write_version;

	size_t entries = 0;
	Entry *head = nullptr;
	Entry *end;
};

#endif //_RENX_LADDERDATABASE_H_HEADER