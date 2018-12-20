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

#if !defined _RENX_EXEMPTIONDATABASE_H_HEADER
#define _RENX_EXEMPTIONDATABASE_H_HEADER

#include <cstdint>
#include <chrono>
#include "Jupiter/Database.h"
#include "Jupiter/String.hpp"
#include "Jupiter/CString.h"
#include "Jupiter/ArrayList.h"
#include "RenX.h"

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
	* @brief Represents the local exemption database.
	*/
	class RENX_API ExemptionDatabase : public Jupiter::Database
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

		/**
		* @brief Called when process_file() is successfully completed.
		*
		* @param file File being processed
		*/
		void process_file_finish(FILE *file) override;

		struct RENX_API Entry
		{
			fpos_t pos; /** Position of the entry in the database */
			uint8_t flags /** Flags affecting this exemption entry (See below for flags) */ = 0x00;
			std::chrono::system_clock::time_point timestamp /** Time the exemption was created */;
			std::chrono::seconds length /** Duration of the exemption; 0 if permanent */;
			uint64_t steamid /** SteamID of the exempted player */;
			uint32_t ip /** IPv4 address of the exempted player */;
			uint8_t prefix_length /** Prefix length for the IPv4 address block */;
			Jupiter::StringS setter /** Name of the user who added the exemption */;

			static const uint8_t FLAG_ACTIVE = 0x80;
			static const uint8_t FLAG_USE_IP = 0x40;
			static const uint8_t FLAG_TYPE_KICK = 0x04;
			static const uint8_t FLAG_TYPE_BAN = 0x02;

			inline bool is_active() { return (flags & FLAG_ACTIVE) != 0; };
			inline bool is_ip_exemption() { return (flags & FLAG_USE_IP) != 0; };
			inline bool is_type_kick() { return (flags & FLAG_TYPE_KICK) != 0; };
			inline bool is_type_ban() { return (flags & FLAG_TYPE_BAN) != 0; };

			inline void set_active() { flags |= FLAG_ACTIVE; };
			inline void set_ip_exemption() { flags |= FLAG_USE_IP; };
			inline void set_type_kick() { flags |= FLAG_TYPE_KICK; };
			inline void set_type_ban() { flags |= FLAG_TYPE_BAN; };

			inline void unset_active() { flags &= ~FLAG_ACTIVE; };
			inline void unset_ip_exemption() { flags &= ~FLAG_USE_IP; };
			inline void unset_type_kick() { flags &= ~FLAG_TYPE_KICK; };
			inline void unset_type_ban() { flags &= ~FLAG_TYPE_BAN; };
		};

		/**
		* @brief Adds an exemption entry for a player and immediately writes it to the database.
		*
		* @param server Server the player is playing in
		* @param player Data of the player to be exempted
		* @param length Duration of the exempt
		*/
		void add(RenX::Server &server, const RenX::PlayerInfo &player, const Jupiter::ReadableString &setter, std::chrono::seconds length, uint8_t flags);

		/**
		* @brief Adds an exemption entry for a set of player information and immediately writes it to the database.
		*
		* @param name Name of the player to exempt
		* @param ip IPv4 address of the player to exempt
		* @param steamid SteamID of the player to exempt
		* @param setter Person implementing the exempt
		* @param length Duration of the exemption
		*/
		void add(uint32_t ip, uint8_t prefix_length, uint64_t steamid, const Jupiter::ReadableString &setter, std::chrono::seconds length, uint8_t flags);

		/**
		* @brief Upgrades the exemption database to the current write_version.
		*/
		void upgrade_database();

		/**
		* @brief Writes an exemption entry to the database.
		*
		* @param entry Entry to write to the database.
		*/
		void write(Entry *entry);

		/**
		* @brief Writes an exemption entry to the database.
		*
		* @param entry Entry to write to the database.
		* @param file FILE stream to write to.
		*/
		void write(Entry *entry, FILE *file);

		/**
		* @brief Deactivates an exemption entry.
		*
		* @param index Index of the entry to deactivate.
		* @param True if the entry was active and is now inactive, false otherwise.
		*/
		bool deactivate(size_t index);

		/**
		* @brief Checks a player for any relevant ban exemptions, and assigns their exemption_flags
		*
		* @param player Player to check exemption flags for
		*/
		void exemption_check(RenX::PlayerInfo &player);

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
		* @brief Fetches the list of exemption entries.
		*
		* @return List of entries
		*/
		const Jupiter::ArrayList<RenX::ExemptionDatabase::Entry> &getEntries() const;

		virtual bool initialize();
		~ExemptionDatabase();

	private:
		/** Database version */
		const uint8_t write_version = 0U;
		uint8_t read_version = write_version;
		fpos_t eof;

		Jupiter::CStringS filename;
		Jupiter::ArrayList<RenX::ExemptionDatabase::Entry> entries;
	};

	RENX_API extern RenX::ExemptionDatabase *exemptionDatabase;
	RENX_API extern RenX::ExemptionDatabase &defaultExemptionDatabase;
}

#endif // _RENX_EXEMPTIONDATABASE_H_HEADER