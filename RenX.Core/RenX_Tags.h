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

#if !defined _RENX_TAGS_H_HEADER
#define _RENX_TAGS_H_HEADER

/**
 * @file RenX_Tags.h
 * @brief Provides tag processing functions
 */

#include "Jupiter/String.h"
#include "Jupiter/CString.h"
#include "RenX.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	/** Forward declarations */
	struct PlayerInfo;
	class Server;

	RENX_API void processTags(Jupiter::StringType &msg, const RenX::Server *server = nullptr, const RenX::PlayerInfo *player = nullptr, const RenX::PlayerInfo *victim = nullptr);
	RENX_API void sanitizeTags(Jupiter::StringType &fmt);

	struct RENX_API Tags
	{
		/** Global formats */
		Jupiter::CStringS dateFmt;
		Jupiter::CStringS timeFmt;

		/** Internal message tags */

		/** Global tags */
		Jupiter::StringS INTERNAL_DATE_TAG;
		Jupiter::StringS INTERNAL_TIME_TAG;

		/** Server tags */
		Jupiter::StringS INTERNAL_RCON_VERSION_TAG;
		Jupiter::StringS INTERNAL_GAME_VERSION_TAG;
		Jupiter::StringS INTERNAL_XRCON_VERSION_TAG;
		Jupiter::StringS INTERNAL_RULES_TAG;
		Jupiter::StringS INTERNAL_USER_TAG;
		Jupiter::StringS INTERNAL_SERVER_NAME_TAG;
		Jupiter::StringS INTERNAL_MAP_TAG;

		/** Player tags */
		Jupiter::StringS INTERNAL_NAME_TAG;
		Jupiter::StringS INTERNAL_RAW_NAME_TAG;
		Jupiter::StringS INTERNAL_IP_TAG;
		Jupiter::StringS INTERNAL_STEAM_TAG;
		Jupiter::StringS INTERNAL_UUID_TAG;
		Jupiter::StringS INTERNAL_ID_TAG;
		Jupiter::StringS INTERNAL_CHARACTER_TAG;
		Jupiter::StringS INTERNAL_VEHICLE_TAG;
		Jupiter::StringS INTERNAL_ADMIN_TAG;
		Jupiter::StringS INTERNAL_PREFIX_TAG;
		Jupiter::StringS INTERNAL_GAME_PREFIX_TAG;
		Jupiter::StringS INTERNAL_TEAM_COLOR_TAG;
		Jupiter::StringS INTERNAL_TEAM_SHORT_TAG;
		Jupiter::StringS INTERNAL_TEAM_LONG_TAG;
		Jupiter::StringS INTERNAL_PING_TAG;
		Jupiter::StringS INTERNAL_SCORE_TAG;
		Jupiter::StringS INTERNAL_CREDITS_TAG;
		Jupiter::StringS INTERNAL_KILLS_TAG;
		Jupiter::StringS INTERNAL_DEATHS_TAG;
		Jupiter::StringS INTERNAL_KDR_TAG;
		Jupiter::StringS INTERNAL_SUICIDES_TAG;
		Jupiter::StringS INTERNAL_HEADSHOTS_TAG;
		Jupiter::StringS INTERNAL_VEHICLE_KILLS_TAG;
		Jupiter::StringS INTERNAL_BUILDING_KILLS_TAG;
		Jupiter::StringS INTERNAL_DEFENCE_KILLS_TAG;
		Jupiter::StringS INTERNAL_WINS_TAG;
		Jupiter::StringS INTERNAL_LOSES_TAG;
		Jupiter::StringS INTERNAL_BEACON_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_ACCESS_TAG;

		/** Victim tags */
		Jupiter::StringS INTERNAL_VICTIM_NAME_TAG;
		Jupiter::StringS INTERNAL_VICTIM_RAW_NAME_TAG;
		Jupiter::StringS INTERNAL_VICTIM_IP_TAG;
		Jupiter::StringS INTERNAL_VICTIM_STEAM_TAG;
		Jupiter::StringS INTERNAL_VICTIM_UUID_TAG;
		Jupiter::StringS INTERNAL_VICTIM_ID_TAG;
		Jupiter::StringS INTERNAL_VICTIM_CHARACTER_TAG;
		Jupiter::StringS INTERNAL_VICTIM_VEHICLE_TAG;
		Jupiter::StringS INTERNAL_VICTIM_ADMIN_TAG;
		Jupiter::StringS INTERNAL_VICTIM_PREFIX_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GAME_PREFIX_TAG;
		Jupiter::StringS INTERNAL_VICTIM_TEAM_COLOR_TAG;
		Jupiter::StringS INTERNAL_VICTIM_TEAM_SHORT_TAG;
		Jupiter::StringS INTERNAL_VICTIM_TEAM_LONG_TAG;
		Jupiter::StringS INTERNAL_VICTIM_PING_TAG;
		Jupiter::StringS INTERNAL_VICTIM_SCORE_TAG;
		Jupiter::StringS INTERNAL_VICTIM_CREDITS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_DEATHS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_KDR_TAG;
		Jupiter::StringS INTERNAL_VICTIM_SUICIDES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_HEADSHOTS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_VEHICLE_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_BUILDING_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_DEFENCE_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_WINS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_LOSES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_ACCESS_TAG;

		/** Other tags */
		Jupiter::StringS INTERNAL_WEAPON_TAG;
		Jupiter::StringS INTERNAL_OBJECT_TAG;
		Jupiter::StringS INTERNAL_MESSAGE_TAG;
		Jupiter::StringS INTERNAL_NEW_NAME_TAG;
		Jupiter::StringS INTERNAL_WIN_SCORE_TAG;
		Jupiter::StringS INTERNAL_LOSE_SCORE_TAG;

		/** External message tags */

		/** Global tags */
		Jupiter::StringS dateTag;
		Jupiter::StringS timeTag;

		/** Server tags */
		Jupiter::StringS rconVersionTag;
		Jupiter::StringS gameVersionTag;
		Jupiter::StringS xRconVersionTag;
		Jupiter::StringS rulesTag;
		Jupiter::StringS userTag;
		Jupiter::StringS serverNameTag;
		Jupiter::StringS mapTag;

		/** Player tags */
		Jupiter::StringS nameTag;
		Jupiter::StringS rawNameTag;
		Jupiter::StringS ipTag;
		Jupiter::StringS steamTag;
		Jupiter::StringS uuidTag;
		Jupiter::StringS idTag;
		Jupiter::StringS characterTag;
		Jupiter::StringS vehicleTag;
		Jupiter::StringS adminTag;
		Jupiter::StringS prefixTag;
		Jupiter::StringS gamePrefixTag;
		Jupiter::StringS teamColorTag;
		Jupiter::StringS teamShortTag;
		Jupiter::StringS teamLongTag;
		Jupiter::StringS pingTag;
		Jupiter::StringS scoreTag;
		Jupiter::StringS creditsTag;
		Jupiter::StringS killsTag;
		Jupiter::StringS deathsTag;
		Jupiter::StringS kdrTag;
		Jupiter::StringS suicidesTag;
		Jupiter::StringS headshotsTag;
		Jupiter::StringS vehicleKillsTag;
		Jupiter::StringS buildingKillsTag;
		Jupiter::StringS defenceKillsTag;
		Jupiter::StringS winsTag;
		Jupiter::StringS losesTag;
		Jupiter::StringS beaconPlacementsTag;
		Jupiter::StringS accessTag;

		/** Victim tags */
		Jupiter::StringS victimNameTag;
		Jupiter::StringS victimRawNameTag;
		Jupiter::StringS victimIPTag;
		Jupiter::StringS victimSteamTag;
		Jupiter::StringS victimUUIDTag;
		Jupiter::StringS victimIDTag;
		Jupiter::StringS victimCharacterTag;
		Jupiter::StringS victimVehicleTag;
		Jupiter::StringS victimAdminTag;
		Jupiter::StringS victimPrefixTag;
		Jupiter::StringS victimGamePrefixTag;
		Jupiter::StringS victimTeamColorTag;
		Jupiter::StringS victimTeamShortTag;
		Jupiter::StringS victimTeamLongTag;
		Jupiter::StringS victimPingTag;
		Jupiter::StringS victimScoreTag;
		Jupiter::StringS victimCreditsTag;
		Jupiter::StringS victimKillsTag;
		Jupiter::StringS victimDeathsTag;
		Jupiter::StringS victimKDRTag;
		Jupiter::StringS victimSuicidesTag;
		Jupiter::StringS victimHeadshotsTag;
		Jupiter::StringS victimVehicleKillsTag;
		Jupiter::StringS victimBuildingKillsTag;
		Jupiter::StringS victimDefenceKillsTag;
		Jupiter::StringS victimWinsTag;
		Jupiter::StringS victimLosesTag;
		Jupiter::StringS victimBeaconPlacementsTag;
		Jupiter::StringS victimAccessTag;

		/** Other tags */
		Jupiter::StringS weaponTag;
		Jupiter::StringS objectTag;
		Jupiter::StringS messageTag;
		Jupiter::StringS newNameTag;
		Jupiter::StringS winScoreTag;
		Jupiter::StringS loseScoreTag;
	};

	RENX_API extern Tags *tags;
}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_TAGS_H_HEADER