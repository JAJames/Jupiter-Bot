/**
 * Copyright (C) 2014-2015 Justin James.
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

#if !defined _RENX_PLAYERINFO_H_HEADER
#define _RENX_PLAYERINFO_H_HEADER

/**
 * @file RenX_PlayerInfo.h
 * @brief Defines the PlayerInfo structure.
 */

#include "Jupiter/String.h"
#include "Jupiter/INIFile.h"
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
		// TODO: Add backpack
		Jupiter::StringS name;
		Jupiter::StringS ip;
		Jupiter::StringS adminType;
		Jupiter::StringS uuid;
		Jupiter::StringS character;
		Jupiter::StringS vehicle;
		uint64_t steamid = 0;
		uint32_t ip32 = 0;
		TeamType team = TeamType::Other;
		int id = 0;
		bool isBot = false;
		time_t joinTime = 0;

		unsigned short ping = 0;
		float score = 0.0f;
		float credits = 0.0f;
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
		
		mutable Jupiter::StringS gamePrefix;
		mutable Jupiter::StringS formatNamePrefix;
		mutable int access = 0;
		mutable Jupiter::INIFile varData; // This will be replaced later with a more dedicated type.
	};

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_PLAYERINFO_H_HEADER