/**
 * Copyright (C) 2014-2016 Jessica James.
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

#if !defined _RENX_BANDATABASE_H_HEADER
#define _RENX_BANDATABASE_H_HEADER

#include <cstdint>
#include "Jupiter/Database.h"
#include "Jupiter/String.h"
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
	* @brief Represents the local ban database.
	*/
	class RENX_API BanDatabase : public Jupiter::Database
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

	public:
		/**
		* @brief Represents a Ban entry in the database.
		*/
		struct RENX_API Entry
		{
			fpos_t pos; /** Position of the entry in the database */
			uint16_t flags /** Flags affecting this ban entry (See below for flags) */ = 0x00;
			std::chrono::system_clock::time_point timestamp /** Time the ban was created */;
			std::chrono::seconds length /** Duration of the ban; 0 if permanent */;
			uint64_t steamid /** SteamID of the banned player */;
			uint32_t ip /** IPv4 address of the banned player */;
			uint8_t prefix_length /** Prefix length for the IPv4 address block */;
			Jupiter::StringS hwid; /** Hardware ID of the banned player */
			Jupiter::StringS rdns /** RDNS of the banned player */;
			Jupiter::StringS name /** Name of the banned player */;
			Jupiter::StringS banner /** Name of the user who initiated the ban */;
			Jupiter::StringS reason /** Reason the player was banned */;
			Jupiter::HashTable varData; /** Variable entry data */

			static const uint16_t FLAG_ACTIVE = 0x8000U;
			static const uint16_t FLAG_USE_RDNS = 0x4000U;
			static const uint16_t FLAG_TYPE_GAME = 0x0080U;
			static const uint16_t FLAG_TYPE_CHAT = 0x0040U;
			static const uint16_t FLAG_TYPE_BOT = 0x0020U;
			static const uint16_t FLAG_TYPE_VOTE = 0x0010U;
			static const uint16_t FLAG_TYPE_MINE = 0x0008U;
			static const uint16_t FLAG_TYPE_LADDER = 0x0004U;
			static const uint16_t FLAG_TYPE_ALERT = 0x0002U;

			inline bool is_active() { return (flags & FLAG_ACTIVE) != 0; };
			inline bool is_rdns_ban() { return (flags & FLAG_USE_RDNS) != 0; };
			inline bool is_type_game() { return (flags & FLAG_TYPE_GAME) != 0; };
			inline bool is_type_chat() { return (flags & FLAG_TYPE_CHAT) != 0; };
			inline bool is_type_bot() { return (flags & FLAG_TYPE_BOT) != 0; };
			inline bool is_type_vote() { return (flags & FLAG_TYPE_VOTE) != 0; };
			inline bool is_type_mine() { return (flags & FLAG_TYPE_MINE) != 0; };
			inline bool is_type_ladder() { return (flags & FLAG_TYPE_LADDER) != 0; };
			inline bool is_type_alert() { return (flags & FLAG_TYPE_ALERT) != 0; };

			inline void set_active() { flags |= FLAG_ACTIVE; };
			inline void set_rdns_ban() { flags |= FLAG_USE_RDNS; }
			inline void set_type_game() { flags |= FLAG_TYPE_GAME; };
			inline void set_type_chat() { flags |= FLAG_TYPE_CHAT; };
			inline void set_type_bot() { flags |= FLAG_TYPE_BOT; };
			inline void set_type_vote() { flags |= FLAG_TYPE_VOTE; };
			inline void set_type_mine() { flags |= FLAG_TYPE_MINE; };
			inline void set_type_ladder() { flags |= FLAG_TYPE_LADDER; };
			inline void set_type_alert() { flags |= FLAG_TYPE_ALERT; };
			inline void set_type_global() { flags = 0xFFFFU; };

			inline void unset_active() { flags &= ~FLAG_ACTIVE; };
			inline void unset_rdns_ban() { flags &= ~FLAG_USE_RDNS; }
			inline void unset_type_game() { flags &= ~FLAG_TYPE_GAME; };
			inline void unset_type_chat() { flags &= ~FLAG_TYPE_CHAT; };
			inline void unset_type_bot() { flags &= ~FLAG_TYPE_BOT; };
			inline void unset_type_vote() { flags &= ~FLAG_TYPE_VOTE; };
			inline void unset_type_mine() { flags &= ~FLAG_TYPE_MINE; };
			inline void unset_type_ladder() { flags &= ~FLAG_TYPE_LADDER; };
			inline void unset_type_alert() { flags &= ~FLAG_TYPE_ALERT; };
			inline void unset_type_global() { flags = 0x0000U; };
		};

		/**
		* @brief Adds a ban entry for a player and immediately writes it to the database.
		*
		* @param server Server the player is playing in
		* @param player Data of the player to be banned
		* @param length Duration of the ban
		*/
		void add(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &banner, const Jupiter::ReadableString &reason, std::chrono::seconds length, uint16_t flags = RenX::BanDatabase::Entry::FLAG_TYPE_GAME);

		/**
		* @brief Adds a ban entry for a set of player information and immediately writes it to the database.
		*
		* @param name Name of the player to ban
		* @param ip IPv4 address of the player to ban
		* @param steamid SteamID of the player to ban
		* @param hwid Hardware ID of the player to ban
		* @param rdns RDNS of the player to ban
		* @param banner Person implementing the ban
		* @param reason Reason the player is getting banned
		* @param length Duration of the ban
		*/
		void add(const Jupiter::ReadableString &name, uint32_t ip, uint8_t prefix_length, uint64_t steamid, const Jupiter::ReadableString &hwid, const Jupiter::ReadableString &rdns, const Jupiter::ReadableString &banner, Jupiter::ReadableString &reason, std::chrono::seconds length, uint16_t flags = RenX::BanDatabase::Entry::FLAG_TYPE_GAME);

		/**
		* @brief Upgrades the ban database to the current write_version.
		*/
		void upgrade_database();

		/**
		* @brief Writes a ban entry to the database.
		*
		* @param entry Entry to write to the database.
		*/
		void write(Entry *entry);

		/**
		* @brief Writes a ban entry to the database.
		*
		* @param entry Entry to write to the database.
		* @param file FILE stream to write to.
		*/
		void write(Entry *entry, FILE *file);

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

		virtual bool initialize();
		~BanDatabase();

	private:
		/** Database version */
		const uint8_t write_version = 5U;
		uint8_t read_version = write_version;
		fpos_t eof;

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