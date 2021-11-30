/**
 * Copyright (C) 2014-2017 Jessica James.
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

#if !defined _RENX_PLAYERINFO_H_HEADER
#define _RENX_PLAYERINFO_H_HEADER

/**
 * @file RenX_PlayerInfo.h
 * @brief Defines the PlayerInfo structure.
 */

#include <chrono>
#include <mutex>
#include <thread>
#include "Jupiter/Reference_String.h"
#include "Jupiter/String.hpp"
#include "Jupiter/Config.h"
#include "RenX.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{

	/**
	* @brief Includes all of the tracked information about a player.
	*/
	struct RENX_API PlayerInfo
	{
		std::chrono::steady_clock::time_point joinTime = std::chrono::steady_clock::now();

		// TODO: Add backpack
		std::string name;
		std::string ip;
		Jupiter::StringS adminType;
		std::string uuid;
		Jupiter::StringS character;
		Jupiter::StringS vehicle;
		std::string hwid;
		uint64_t steamid = 0;
		uint32_t ip32 = 0;
		uint16_t ban_flags = 0;
		uint8_t exemption_flags = 0;
		TeamType team = TeamType::Other;
		int id = 0;
		bool isBot = false;
		bool is_dev = false;
		unsigned short ping = 0;
		double score = 0.0f;
		double credits = 0.0f;
		size_t global_rank = 0;
		size_t local_rank = 0;
		unsigned int kills = 0;
		unsigned int deaths = 0;
		unsigned int suicides = 0;
		unsigned int headshots = 0;
		unsigned int vehicleKills = 0;
		unsigned int buildingKills = 0;
		unsigned int defenceKills = 0;
		unsigned int wins = 0;
		unsigned int loses = 0;
		unsigned int beaconPlacements = 0;
		unsigned int beaconDisarms = 0;
		unsigned int proxy_placements = 0;
		unsigned int proxy_disarms = 0;
		unsigned int captures = 0;
		unsigned int steals = 0;
		unsigned int stolen = 0;

		// Lock-free getter -- never access m_rdns until it's been set by RDNS thread
		std::string_view get_rdns() const {
			if (m_rdns_ptr.use_count() != 1
				|| m_rdns_ptr == nullptr) {
				return {};
			}

			// In theory if get_rdns() were ever called at the same time as start_resolve_rdns(), on separate threads,
			// then there would be a race condition here causing undefined behavior. However, we only interact with
			// players on a single thread, and even beyond that, we only call start_resolve_rdns() immediately after
			// constructing PlayerInfo. Therefore, this is safe
			return *m_rdns_ptr;
		}
		static void resolve_rdns(std::string in_ip, std::shared_ptr<std::string> out_rdns);
		void start_resolve_rdns();
		bool rdns_pending = false;
		
		mutable Jupiter::StringS gamePrefix;
		mutable Jupiter::StringS formatNamePrefix;
		mutable int access = 0;
		mutable Jupiter::Config varData; // TODO: use jessilib::object instead

	private:
		std::shared_ptr<std::string> m_rdns_ptr; // Needs synchronization across threads
	};

	static std::string_view rdns_pending{ "RDNS_PENDING" };
}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_PLAYERINFO_H_HEADER