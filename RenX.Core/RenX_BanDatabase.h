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

#if !defined _RENX_BANDATABASE_H_HEADER
#define _RENX_BANDATABASE_H_HEADER

#include <ctime>
#include <cstdint>
#include "Jupiter/String.h"
#include "Jupiter/CString.h"
#include "Jupiter/ArrayList.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	struct PlayerInfo;
	class Server;

	/**
	* @brief Represents the local ban database.
	*/
	class RENX_API BanDatabase
	{
	public:
		/**
		* @brief Represents a Ban entry in the database.
		*/
		struct RENX_API Entry
		{
			fpos_t pos; /** Position of the entry in the database */
			unsigned char active; /** 1 if the ban is active, 0 otherwise */
			time_t timestamp /** Time the ban was created */;
			time_t length /** Duration of the ban; 0 if permanent */;
			uint64_t steamid /** SteamID of the banned player */;
			uint32_t ip /** IPv4 address of the banned player */;
			Jupiter::StringS name /** Name of the banned player */;
			Jupiter::StringS reason /** Reason the player was banned */;
			Jupiter::INIFile::Section varData; /** Variable entry data */
		};

		/**
		* @brief Loads a file into the ban system.
		* Note: This will generate a database file if none is found.
		*
		* @param fname String containing the name of the file to load
		* @return True on success, false otherwise.
		*/
		bool load(const Jupiter::ReadableString &fname);

		/**
		* @brief Adds a ban entry for a player and immediately writes it to the database.
		*
		* @param server Server the player is playing in
		* @param player Data of the player to be banned
		* @param length Duration of the ban
		*/
		void add(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason, time_t length);

		/**
		* @brief Writes a ban file to the database.
		*
		* @param entry Entry to write to the database.
		*/
		void write(Entry *entry);

		/**
		* @brief Writes a ban file to the database.
		*
		* @param entry Entry to write to the database.
		* @param file FILE stream to write to.
		*/
		static void write(Entry *entry, FILE *file);

		/**
		* @brief Deactivates a ban entry.
		*
		* @param index Index of the entry to deactivate.
		* @param True if the entry was active and is now inactive, false otherwise.
		*/
		bool deactivate(size_t index);

		/**
		* @brief Fetches the version of the database file.
		*
		* @return Database version
		*/
		uint8_t getVersion() const;

		/**
		* @brief Fetches the name of the database file.
		*
		* @return Database file name
		*/
		const Jupiter::ReadableString &getFileName() const;

		/**
		* @brief Fetches the list of ban entries.
		*
		* @return List of entries
		*/
		const Jupiter::ArrayList<RenX::BanDatabase::Entry> &getEntries() const;

		BanDatabase();
		~BanDatabase();

	private:
		uint8_t version;
		Jupiter::CStringS filename;
		Jupiter::ArrayList<RenX::BanDatabase::Entry> entries;
	};

	RENX_API extern RenX::BanDatabase *banDatabase;
	RENX_API extern RenX::BanDatabase &defaultBanDatabase;
}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_BANDATABASE_H_HEADER