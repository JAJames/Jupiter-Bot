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

#if !defined _RENX_TAGS_H_HEADER
#define _RENX_TAGS_H_HEADER

/**
 * @file RenX_Tags.h
 * @brief Provides tag processing functions
 */

#include "Jupiter/String.h"
#include "Jupiter/CString.h"
#include "RenX.h"
#include "RenX_LadderDatabase.h"

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
	struct BuildingInfo;

	RENX_API void processTags(Jupiter::StringType &msg, const RenX::Server *server = nullptr, const RenX::PlayerInfo *player = nullptr, const RenX::PlayerInfo *victim = nullptr, const RenX::BuildingInfo *building = nullptr);
	RENX_API void processTags(Jupiter::StringType &msg, const RenX::LadderDatabase::Entry &entry);
	RENX_API void sanitizeTags(Jupiter::StringType &fmt);
	RENX_API const Jupiter::ReadableString &getUniqueInternalTag();

	struct RENX_API Tags
	{
		/** Global formats */
		Jupiter::CStringS dateFmt;
		Jupiter::CStringS timeFmt;

		/** Global tags */
		Jupiter::StringS INTERNAL_DATE_TAG;
		Jupiter::StringS INTERNAL_TIME_TAG;

		/** Server tags */
		Jupiter::StringS INTERNAL_RCON_VERSION_TAG;
		Jupiter::StringS INTERNAL_GAME_VERSION_TAG;
		Jupiter::StringS INTERNAL_RULES_TAG;
		Jupiter::StringS INTERNAL_USER_TAG;
		Jupiter::StringS INTERNAL_SERVER_NAME_TAG;
		Jupiter::StringS INTERNAL_MAP_TAG;
		Jupiter::StringS INTERNAL_MAP_GUID_TAG;
		Jupiter::StringS INTERNAL_SERVER_HOSTNAME_TAG;
		Jupiter::StringS INTERNAL_SERVER_PORT_TAG;
		Jupiter::StringS INTERNAL_SOCKET_HOSTNAME_TAG;
		Jupiter::StringS INTERNAL_SOCKET_PORT_TAG;
		Jupiter::StringS INTERNAL_SERVER_PREFIX_TAG;

		/** Player tags */
		Jupiter::StringS INTERNAL_NAME_TAG;
		Jupiter::StringS INTERNAL_RAW_NAME_TAG;
		Jupiter::StringS INTERNAL_IP_TAG;
		Jupiter::StringS INTERNAL_RDNS_TAG;
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
		Jupiter::StringS INTERNAL_SCORE_PER_MINUTE_TAG;
		Jupiter::StringS INTERNAL_CREDITS_TAG;
		Jupiter::StringS INTERNAL_KILLS_TAG;
		Jupiter::StringS INTERNAL_DEATHS_TAG;
		Jupiter::StringS INTERNAL_KDR_TAG;
		Jupiter::StringS INTERNAL_SUICIDES_TAG;
		Jupiter::StringS INTERNAL_HEADSHOTS_TAG;
		Jupiter::StringS INTERNAL_HEADSHOT_KILL_RATIO_TAG;
		Jupiter::StringS INTERNAL_VEHICLE_KILLS_TAG;
		Jupiter::StringS INTERNAL_BUILDING_KILLS_TAG;
		Jupiter::StringS INTERNAL_DEFENCE_KILLS_TAG;
		Jupiter::StringS INTERNAL_GAME_TIME_TAG;
		Jupiter::StringS INTERNAL_GAMES_TAG;
		Jupiter::StringS INTERNAL_GDI_GAMES_TAG;
		Jupiter::StringS INTERNAL_NOD_GAMES_TAG;
		Jupiter::StringS INTERNAL_WINS_TAG;
		Jupiter::StringS INTERNAL_GDI_WINS_TAG;
		Jupiter::StringS INTERNAL_NOD_WINS_TAG;
		Jupiter::StringS INTERNAL_TIES_TAG;
		Jupiter::StringS INTERNAL_LOSSES_TAG;
		Jupiter::StringS INTERNAL_GDI_LOSSES_TAG;
		Jupiter::StringS INTERNAL_NOD_LOSSES_TAG;
		Jupiter::StringS INTERNAL_WIN_LOSS_RATIO_TAG;
		Jupiter::StringS INTERNAL_GDI_WIN_LOSS_RATIO_TAG;
		Jupiter::StringS INTERNAL_NOD_WIN_LOSS_RATIO_TAG;
		Jupiter::StringS INTERNAL_BEACON_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_BEACON_DISARMS_TAG;
		Jupiter::StringS INTERNAL_PROXY_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_PROXY_DISARMS_TAG;
		Jupiter::StringS INTERNAL_CAPTURES_TAG;
		Jupiter::StringS INTERNAL_STEALS_TAG;
		Jupiter::StringS INTERNAL_STOLEN_TAG;
		Jupiter::StringS INTERNAL_ACCESS_TAG;

		/** Victim tags */
		Jupiter::StringS INTERNAL_VICTIM_NAME_TAG;
		Jupiter::StringS INTERNAL_VICTIM_RAW_NAME_TAG;
		Jupiter::StringS INTERNAL_VICTIM_IP_TAG;
		Jupiter::StringS INTERNAL_VICTIM_RDNS_TAG;
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
		Jupiter::StringS INTERNAL_VICTIM_SCORE_PER_MINUTE_TAG;
		Jupiter::StringS INTERNAL_VICTIM_CREDITS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_DEATHS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_KDR_TAG;
		Jupiter::StringS INTERNAL_VICTIM_SUICIDES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_HEADSHOTS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_HEADSHOT_KILL_RATIO_TAG;
		Jupiter::StringS INTERNAL_VICTIM_VEHICLE_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_BUILDING_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_DEFENCE_KILLS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GAME_TIME_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GAMES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GDI_GAMES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_NOD_GAMES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_WINS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GDI_WINS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_NOD_WINS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_TIES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_LOSSES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GDI_LOSSES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_NOD_LOSSES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_WIN_LOSS_RATIO_TAG;
		Jupiter::StringS INTERNAL_VICTIM_GDI_WIN_LOSS_RATIO_TAG;
		Jupiter::StringS INTERNAL_VICTIM_NOD_WIN_LOSS_RATIO_TAG;
		Jupiter::StringS INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_BEACON_DISARMS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_PROXY_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_PROXY_DISARMS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_CAPTURES_TAG;
		Jupiter::StringS INTERNAL_VICTIM_STEALS_TAG;
		Jupiter::StringS INTERNAL_VICTIM_STOLEN_TAG;
		Jupiter::StringS INTERNAL_VICTIM_ACCESS_TAG;

		/** Building tags */
		Jupiter::StringS INTERNAL_BUILDING_NAME_TAG;
		Jupiter::StringS INTERNAL_BUILDING_RAW_NAME_TAG;
		Jupiter::StringS INTERNAL_BUILDING_HEALTH_TAG;
		Jupiter::StringS INTERNAL_BUILDING_MAX_HEALTH_TAG;
		Jupiter::StringS INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG;
		Jupiter::StringS INTERNAL_BUILDING_HEALTH_BAR_TAG;
		Jupiter::StringS INTERNAL_BUILDING_TEAM_COLOR_TAG;
		Jupiter::StringS INTERNAL_BUILDING_TEAM_SHORT_TAG;
		Jupiter::StringS INTERNAL_BUILDING_TEAM_LONG_TAG;

		/** Ladder tags */
		Jupiter::StringS INTERNAL_RANK_TAG;
		Jupiter::StringS INTERNAL_LAST_GAME_TAG;
		Jupiter::StringS INTERNAL_GDI_SCORE_TAG;
		Jupiter::StringS INTERNAL_GDI_SPM_TAG;
		Jupiter::StringS INTERNAL_GDI_GAME_TIME_TAG;
		Jupiter::StringS INTERNAL_GDI_TIES_TAG;
		Jupiter::StringS INTERNAL_GDI_BEACON_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_GDI_BEACON_DISARMS_TAG;
		Jupiter::StringS INTERNAL_GDI_PROXY_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_GDI_PROXY_DISARMS_TAG;
		Jupiter::StringS INTERNAL_GDI_KILLS_TAG;
		Jupiter::StringS INTERNAL_GDI_DEATHS_TAG;
		Jupiter::StringS INTERNAL_GDI_VEHICLE_KILLS_TAG;
		Jupiter::StringS INTERNAL_GDI_DEFENCE_KILLS_TAG;
		Jupiter::StringS INTERNAL_GDI_BUILDING_KILLS_TAG;
		Jupiter::StringS INTERNAL_GDI_KDR_TAG;
		Jupiter::StringS INTERNAL_GDI_HEADSHOTS_TAG;
		Jupiter::StringS INTERNAL_GDI_HEADSHOT_KILL_RATIO_TAG;
		Jupiter::StringS INTERNAL_NOD_SCORE_TAG;
		Jupiter::StringS INTERNAL_NOD_SPM_TAG;
		Jupiter::StringS INTERNAL_NOD_GAME_TIME_TAG;
		Jupiter::StringS INTERNAL_NOD_TIES_TAG;
		Jupiter::StringS INTERNAL_NOD_BEACON_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_NOD_BEACON_DISARMS_TAG;
		Jupiter::StringS INTERNAL_NOD_PROXY_PLACEMENTS_TAG;
		Jupiter::StringS INTERNAL_NOD_PROXY_DISARMS_TAG;
		Jupiter::StringS INTERNAL_NOD_KILLS_TAG;
		Jupiter::StringS INTERNAL_NOD_DEATHS_TAG;
		Jupiter::StringS INTERNAL_NOD_VEHICLE_KILLS_TAG;
		Jupiter::StringS INTERNAL_NOD_DEFENCE_KILLS_TAG;
		Jupiter::StringS INTERNAL_NOD_BUILDING_KILLS_TAG;
		Jupiter::StringS INTERNAL_NOD_KDR_TAG;
		Jupiter::StringS INTERNAL_NOD_HEADSHOTS_TAG;
		Jupiter::StringS INTERNAL_NOD_HEADSHOT_KILL_RATIO_TAG;

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
		Jupiter::StringS rulesTag;
		Jupiter::StringS userTag;
		Jupiter::StringS serverNameTag;
		Jupiter::StringS mapTag;
		Jupiter::StringS mapGUIDTag;
		Jupiter::StringS serverHostnameTag;
		Jupiter::StringS serverPortTag;
		Jupiter::StringS socketHostnameTag;
		Jupiter::StringS socketPortTag;
		Jupiter::StringS serverPrefixTag;

		/** Player tags */
		Jupiter::StringS nameTag;
		Jupiter::StringS rawNameTag;
		Jupiter::StringS ipTag;
		Jupiter::StringS rdnsTag;
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
		Jupiter::StringS scorePerMinuteTag;
		Jupiter::StringS creditsTag;
		Jupiter::StringS killsTag;
		Jupiter::StringS deathsTag;
		Jupiter::StringS kdrTag;
		Jupiter::StringS suicidesTag;
		Jupiter::StringS headshotsTag;
		Jupiter::StringS headshotKillRatioTag;
		Jupiter::StringS vehicleKillsTag;
		Jupiter::StringS buildingKillsTag;
		Jupiter::StringS defenceKillsTag;
		Jupiter::StringS gameTimeTag;
		Jupiter::StringS gamesTag;
		Jupiter::StringS GDIGamesTag;
		Jupiter::StringS NodGamesTag;
		Jupiter::StringS winsTag;
		Jupiter::StringS GDIWinsTag;
		Jupiter::StringS NodWinsTag;
		Jupiter::StringS tiesTag;
		Jupiter::StringS lossesTag;
		Jupiter::StringS GDILossesTag;
		Jupiter::StringS NodLossesTag;
		Jupiter::StringS winLossRatioTag;
		Jupiter::StringS GDIWinLossRatioTag;
		Jupiter::StringS NodWinLossRatioTag;
		Jupiter::StringS beaconPlacementsTag;
		Jupiter::StringS beaconDisarmsTag;
		Jupiter::StringS proxyPlacementsTag;
		Jupiter::StringS proxyDisarmsTag;
		Jupiter::StringS capturesTag;
		Jupiter::StringS stealsTag;
		Jupiter::StringS stolenTag;
		Jupiter::StringS accessTag;

		/** Victim tags */
		Jupiter::StringS victimNameTag;
		Jupiter::StringS victimRawNameTag;
		Jupiter::StringS victimIPTag;
		Jupiter::StringS victimRDNSTag;
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
		Jupiter::StringS victimScorePerMinuteTag;
		Jupiter::StringS victimCreditsTag;
		Jupiter::StringS victimKillsTag;
		Jupiter::StringS victimDeathsTag;
		Jupiter::StringS victimKDRTag;
		Jupiter::StringS victimSuicidesTag;
		Jupiter::StringS victimHeadshotsTag;
		Jupiter::StringS victimHeadshotKillRatioTag;
		Jupiter::StringS victimVehicleKillsTag;
		Jupiter::StringS victimBuildingKillsTag;
		Jupiter::StringS victimDefenceKillsTag;
		Jupiter::StringS victimGameTimeTag;
		Jupiter::StringS victimGamesTag;
		Jupiter::StringS victimGDIGamesTag;
		Jupiter::StringS victimNodGamesTag;
		Jupiter::StringS victimWinsTag;
		Jupiter::StringS victimGDIWinsTag;
		Jupiter::StringS victimNodWinsTag;
		Jupiter::StringS victimTiesTag;
		Jupiter::StringS victimLossesTag;
		Jupiter::StringS victimGDILossesTag;
		Jupiter::StringS victimNodLossesTag;
		Jupiter::StringS victimWinLossRatioTag;
		Jupiter::StringS victimGDIWinLossRatioTag;
		Jupiter::StringS victimNodWinLossRatioTag;
		Jupiter::StringS victimBeaconPlacementsTag;
		Jupiter::StringS victimBeaconDisarmsTag;
		Jupiter::StringS victimProxyPlacementsTag;
		Jupiter::StringS victimProxyDisarmsTag;
		Jupiter::StringS victimCapturesTag;
		Jupiter::StringS victimStealsTag;
		Jupiter::StringS victimStolenTag;
		Jupiter::StringS victimAccessTag;

		/** Building tags */
		Jupiter::StringS buildingNameTag;
		Jupiter::StringS buildingRawNameTag;
		Jupiter::StringS buildingHealthTag;
		Jupiter::StringS buildingMaxHealthTag;
		Jupiter::StringS buildingHealthPercentageTag;
		Jupiter::StringS buildingHealthBarTag;
		Jupiter::StringS buildingTeamColorTag;
		Jupiter::StringS buildingTeamShortTag;
		Jupiter::StringS buildingTeamLongTag;

		/** Ladder tags */
		Jupiter::StringS rankTag;
		Jupiter::StringS lastGameTag;
		Jupiter::StringS GDIScoreTag;
		Jupiter::StringS GDISPMTag;
		Jupiter::StringS GDIGameTimeTag;
		Jupiter::StringS GDITiesTag;
		Jupiter::StringS GDIBeaconPlacementsTag;
		Jupiter::StringS GDIBeaconDisarmsTag;
		Jupiter::StringS GDIProxyPlacementsTag;
		Jupiter::StringS GDIProxyDisarmsTag;
		Jupiter::StringS GDIKillsTag;
		Jupiter::StringS GDIDeathsTag;
		Jupiter::StringS GDIVehicleKillsTag;
		Jupiter::StringS GDIDefenceKillsTag;
		Jupiter::StringS GDIBuildingKillsTag;
		Jupiter::StringS GDIKDRTag;
		Jupiter::StringS GDIHeadshotsTag;
		Jupiter::StringS GDIHeadshotKillRatioTag;
		Jupiter::StringS NodScoreTag;
		Jupiter::StringS NodSPMTag;
		Jupiter::StringS NodGameTimeTag;
		Jupiter::StringS NodTiesTag;
		Jupiter::StringS NodBeaconPlacementsTag;
		Jupiter::StringS NodBeaconDisarmsTag;
		Jupiter::StringS NodProxyPlacementsTag;
		Jupiter::StringS NodProxyDisarmsTag;
		Jupiter::StringS NodKillsTag;
		Jupiter::StringS NodDeathsTag;
		Jupiter::StringS NodVehicleKillsTag;
		Jupiter::StringS NodDefenceKillsTag;
		Jupiter::StringS NodBuildingKillsTag;
		Jupiter::StringS NodKDRTag;
		Jupiter::StringS NodHeadshotsTag;
		Jupiter::StringS NodHeadshotKillRatioTag;

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

/** Helper macro for processing tags */
#define PROCESS_TAG(tag, value) \
while(true) { \
index = msg.find(tag); \
if (index == Jupiter::INVALID_INDEX) break; \
msg.replace(index, tag.size(), value); }

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_TAGS_H_HEADER