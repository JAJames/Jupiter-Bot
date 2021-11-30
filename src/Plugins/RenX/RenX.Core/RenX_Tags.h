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

#include "Jupiter/String.hpp"
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

	RENX_API void processTags(std::string& msg, const RenX::Server *server = nullptr, const RenX::PlayerInfo *player = nullptr, const RenX::PlayerInfo *victim = nullptr, const RenX::BuildingInfo *building = nullptr);
	RENX_API void processTags(std::string& msg, const RenX::LadderDatabase::Entry &entry);
	RENX_API void sanitizeTags(std::string& fmt);
	RENX_API void replace_tag(std::string& fmt, std::string_view tag, std::string_view internal_tag);
	RENX_API const Jupiter::ReadableString &getUniqueInternalTag();

	struct RENX_API Tags
	{
		virtual bool initialize();

		/** Global formats */
		std::string dateFmt;
		std::string timeFmt;

		/** Global tags */
		std::string INTERNAL_DATE_TAG;
		std::string INTERNAL_TIME_TAG;

		/** Server tags */
		std::string INTERNAL_RCON_VERSION_TAG;
		std::string INTERNAL_GAME_VERSION_TAG;
		std::string INTERNAL_RULES_TAG;
		std::string INTERNAL_USER_TAG;
		std::string INTERNAL_SERVER_NAME_TAG;
		std::string INTERNAL_MAP_TAG;
		std::string INTERNAL_MAP_GUID_TAG;
		std::string INTERNAL_SERVER_HOSTNAME_TAG;
		std::string INTERNAL_SERVER_PORT_TAG;
		std::string INTERNAL_SOCKET_HOSTNAME_TAG;
		std::string INTERNAL_SOCKET_PORT_TAG;
		std::string INTERNAL_SERVER_PREFIX_TAG;

		/** Player tags */
		std::string INTERNAL_NAME_TAG;
		std::string INTERNAL_RAW_NAME_TAG;
		std::string INTERNAL_IP_TAG;
		std::string INTERNAL_HWID_TAG;
		std::string INTERNAL_RDNS_TAG;
		std::string INTERNAL_STEAM_TAG;
		std::string INTERNAL_UUID_TAG;
		std::string INTERNAL_ID_TAG;
		std::string INTERNAL_CHARACTER_TAG;
		std::string INTERNAL_VEHICLE_TAG;
		std::string INTERNAL_ADMIN_TAG;
		std::string INTERNAL_PREFIX_TAG;
		std::string INTERNAL_GAME_PREFIX_TAG;
		std::string INTERNAL_TEAM_COLOR_TAG;
		std::string INTERNAL_TEAM_SHORT_TAG;
		std::string INTERNAL_TEAM_LONG_TAG;
		std::string INTERNAL_PING_TAG;
		std::string INTERNAL_SCORE_TAG;
		std::string INTERNAL_SCORE_PER_MINUTE_TAG;
		std::string INTERNAL_CREDITS_TAG;
		std::string INTERNAL_KILLS_TAG;
		std::string INTERNAL_DEATHS_TAG;
		std::string INTERNAL_KDR_TAG;
		std::string INTERNAL_SUICIDES_TAG;
		std::string INTERNAL_HEADSHOTS_TAG;
		std::string INTERNAL_HEADSHOT_KILL_RATIO_TAG;
		std::string INTERNAL_VEHICLE_KILLS_TAG;
		std::string INTERNAL_BUILDING_KILLS_TAG;
		std::string INTERNAL_DEFENCE_KILLS_TAG;
		std::string INTERNAL_GAME_TIME_TAG;
		std::string INTERNAL_GAMES_TAG;
		std::string INTERNAL_GDI_GAMES_TAG;
		std::string INTERNAL_NOD_GAMES_TAG;
		std::string INTERNAL_WINS_TAG;
		std::string INTERNAL_GDI_WINS_TAG;
		std::string INTERNAL_NOD_WINS_TAG;
		std::string INTERNAL_TIES_TAG;
		std::string INTERNAL_LOSSES_TAG;
		std::string INTERNAL_GDI_LOSSES_TAG;
		std::string INTERNAL_NOD_LOSSES_TAG;
		std::string INTERNAL_WIN_LOSS_RATIO_TAG;
		std::string INTERNAL_GDI_WIN_LOSS_RATIO_TAG;
		std::string INTERNAL_NOD_WIN_LOSS_RATIO_TAG;
		std::string INTERNAL_BEACON_PLACEMENTS_TAG;
		std::string INTERNAL_BEACON_DISARMS_TAG;
		std::string INTERNAL_PROXY_PLACEMENTS_TAG;
		std::string INTERNAL_PROXY_DISARMS_TAG;
		std::string INTERNAL_CAPTURES_TAG;
		std::string INTERNAL_STEALS_TAG;
		std::string INTERNAL_STOLEN_TAG;
		std::string INTERNAL_ACCESS_TAG;

		/** Victim tags */
		std::string INTERNAL_VICTIM_NAME_TAG;
		std::string INTERNAL_VICTIM_RAW_NAME_TAG;
		std::string INTERNAL_VICTIM_IP_TAG;
		std::string INTERNAL_VICTIM_HWID_TAG;
		std::string INTERNAL_VICTIM_RDNS_TAG;
		std::string INTERNAL_VICTIM_STEAM_TAG;
		std::string INTERNAL_VICTIM_UUID_TAG;
		std::string INTERNAL_VICTIM_ID_TAG;
		std::string INTERNAL_VICTIM_CHARACTER_TAG;
		std::string INTERNAL_VICTIM_VEHICLE_TAG;
		std::string INTERNAL_VICTIM_ADMIN_TAG;
		std::string INTERNAL_VICTIM_PREFIX_TAG;
		std::string INTERNAL_VICTIM_GAME_PREFIX_TAG;
		std::string INTERNAL_VICTIM_TEAM_COLOR_TAG;
		std::string INTERNAL_VICTIM_TEAM_SHORT_TAG;
		std::string INTERNAL_VICTIM_TEAM_LONG_TAG;
		std::string INTERNAL_VICTIM_PING_TAG;
		std::string INTERNAL_VICTIM_SCORE_TAG;
		std::string INTERNAL_VICTIM_SCORE_PER_MINUTE_TAG;
		std::string INTERNAL_VICTIM_CREDITS_TAG;
		std::string INTERNAL_VICTIM_KILLS_TAG;
		std::string INTERNAL_VICTIM_DEATHS_TAG;
		std::string INTERNAL_VICTIM_KDR_TAG;
		std::string INTERNAL_VICTIM_SUICIDES_TAG;
		std::string INTERNAL_VICTIM_HEADSHOTS_TAG;
		std::string INTERNAL_VICTIM_HEADSHOT_KILL_RATIO_TAG;
		std::string INTERNAL_VICTIM_VEHICLE_KILLS_TAG;
		std::string INTERNAL_VICTIM_BUILDING_KILLS_TAG;
		std::string INTERNAL_VICTIM_DEFENCE_KILLS_TAG;
		std::string INTERNAL_VICTIM_GAME_TIME_TAG;
		std::string INTERNAL_VICTIM_GAMES_TAG;
		std::string INTERNAL_VICTIM_GDI_GAMES_TAG;
		std::string INTERNAL_VICTIM_NOD_GAMES_TAG;
		std::string INTERNAL_VICTIM_WINS_TAG;
		std::string INTERNAL_VICTIM_GDI_WINS_TAG;
		std::string INTERNAL_VICTIM_NOD_WINS_TAG;
		std::string INTERNAL_VICTIM_TIES_TAG;
		std::string INTERNAL_VICTIM_LOSSES_TAG;
		std::string INTERNAL_VICTIM_GDI_LOSSES_TAG;
		std::string INTERNAL_VICTIM_NOD_LOSSES_TAG;
		std::string INTERNAL_VICTIM_WIN_LOSS_RATIO_TAG;
		std::string INTERNAL_VICTIM_GDI_WIN_LOSS_RATIO_TAG;
		std::string INTERNAL_VICTIM_NOD_WIN_LOSS_RATIO_TAG;
		std::string INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG;
		std::string INTERNAL_VICTIM_BEACON_DISARMS_TAG;
		std::string INTERNAL_VICTIM_PROXY_PLACEMENTS_TAG;
		std::string INTERNAL_VICTIM_PROXY_DISARMS_TAG;
		std::string INTERNAL_VICTIM_CAPTURES_TAG;
		std::string INTERNAL_VICTIM_STEALS_TAG;
		std::string INTERNAL_VICTIM_STOLEN_TAG;
		std::string INTERNAL_VICTIM_ACCESS_TAG;

		/** Building tags */
		std::string INTERNAL_BUILDING_NAME_TAG;
		std::string INTERNAL_BUILDING_RAW_NAME_TAG;
		std::string INTERNAL_BUILDING_HEALTH_TAG;
		std::string INTERNAL_BUILDING_MAX_HEALTH_TAG;
		std::string INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG;
		std::string INTERNAL_BUILDING_ARMOR_TAG;
		std::string INTERNAL_BUILDING_MAX_ARMOR_TAG;
		std::string INTERNAL_BUILDING_ARMOR_PERCENTAGE_TAG;
		std::string INTERNAL_BUILDING_DURABILITY_TAG;
		std::string INTERNAL_BUILDING_MAX_DURABILITY_TAG;
		std::string INTERNAL_BUILDING_DURABILITY_PERCENTAGE_TAG;
		std::string INTERNAL_BUILDING_TEAM_COLOR_TAG;
		std::string INTERNAL_BUILDING_TEAM_SHORT_TAG;
		std::string INTERNAL_BUILDING_TEAM_LONG_TAG;

		/** Ladder tags */
		std::string INTERNAL_RANK_TAG;
		std::string INTERNAL_LAST_GAME_TAG;
		std::string INTERNAL_GDI_SCORE_TAG;
		std::string INTERNAL_GDI_SPM_TAG;
		std::string INTERNAL_GDI_GAME_TIME_TAG;
		std::string INTERNAL_GDI_TIES_TAG;
		std::string INTERNAL_GDI_BEACON_PLACEMENTS_TAG;
		std::string INTERNAL_GDI_BEACON_DISARMS_TAG;
		std::string INTERNAL_GDI_PROXY_PLACEMENTS_TAG;
		std::string INTERNAL_GDI_PROXY_DISARMS_TAG;
		std::string INTERNAL_GDI_KILLS_TAG;
		std::string INTERNAL_GDI_DEATHS_TAG;
		std::string INTERNAL_GDI_VEHICLE_KILLS_TAG;
		std::string INTERNAL_GDI_DEFENCE_KILLS_TAG;
		std::string INTERNAL_GDI_BUILDING_KILLS_TAG;
		std::string INTERNAL_GDI_KDR_TAG;
		std::string INTERNAL_GDI_HEADSHOTS_TAG;
		std::string INTERNAL_GDI_HEADSHOT_KILL_RATIO_TAG;
		std::string INTERNAL_NOD_SCORE_TAG;
		std::string INTERNAL_NOD_SPM_TAG;
		std::string INTERNAL_NOD_GAME_TIME_TAG;
		std::string INTERNAL_NOD_TIES_TAG;
		std::string INTERNAL_NOD_BEACON_PLACEMENTS_TAG;
		std::string INTERNAL_NOD_BEACON_DISARMS_TAG;
		std::string INTERNAL_NOD_PROXY_PLACEMENTS_TAG;
		std::string INTERNAL_NOD_PROXY_DISARMS_TAG;
		std::string INTERNAL_NOD_KILLS_TAG;
		std::string INTERNAL_NOD_DEATHS_TAG;
		std::string INTERNAL_NOD_VEHICLE_KILLS_TAG;
		std::string INTERNAL_NOD_DEFENCE_KILLS_TAG;
		std::string INTERNAL_NOD_BUILDING_KILLS_TAG;
		std::string INTERNAL_NOD_KDR_TAG;
		std::string INTERNAL_NOD_HEADSHOTS_TAG;
		std::string INTERNAL_NOD_HEADSHOT_KILL_RATIO_TAG;

		/** Other tags */
		std::string INTERNAL_WEAPON_TAG;
		std::string INTERNAL_OBJECT_TAG;
		std::string INTERNAL_MESSAGE_TAG;
		std::string INTERNAL_NEW_NAME_TAG;
		std::string INTERNAL_WIN_SCORE_TAG;
		std::string INTERNAL_LOSE_SCORE_TAG;

		/** External message tags */

		/** Global tags */
		std::string dateTag;
		std::string timeTag;

		/** Server tags */
		std::string rconVersionTag;
		std::string gameVersionTag;
		std::string rulesTag;
		std::string userTag;
		std::string serverNameTag;
		std::string mapTag;
		std::string mapGUIDTag;
		std::string serverHostnameTag;
		std::string serverPortTag;
		std::string socketHostnameTag;
		std::string socketPortTag;
		std::string serverPrefixTag;

		/** Player tags */
		std::string nameTag;
		std::string rawNameTag;
		std::string ipTag;
		std::string hwidTag;
		std::string rdnsTag;
		std::string steamTag;
		std::string uuidTag;
		std::string idTag;
		std::string characterTag;
		std::string vehicleTag;
		std::string adminTag;
		std::string prefixTag;
		std::string gamePrefixTag;
		std::string teamColorTag;
		std::string teamShortTag;
		std::string teamLongTag;
		std::string pingTag;
		std::string scoreTag;
		std::string scorePerMinuteTag;
		std::string creditsTag;
		std::string killsTag;
		std::string deathsTag;
		std::string kdrTag;
		std::string suicidesTag;
		std::string headshotsTag;
		std::string headshotKillRatioTag;
		std::string vehicleKillsTag;
		std::string buildingKillsTag;
		std::string defenceKillsTag;
		std::string gameTimeTag;
		std::string gamesTag;
		std::string GDIGamesTag;
		std::string NodGamesTag;
		std::string winsTag;
		std::string GDIWinsTag;
		std::string NodWinsTag;
		std::string tiesTag;
		std::string lossesTag;
		std::string GDILossesTag;
		std::string NodLossesTag;
		std::string winLossRatioTag;
		std::string GDIWinLossRatioTag;
		std::string NodWinLossRatioTag;
		std::string beaconPlacementsTag;
		std::string beaconDisarmsTag;
		std::string proxyPlacementsTag;
		std::string proxyDisarmsTag;
		std::string capturesTag;
		std::string stealsTag;
		std::string stolenTag;
		std::string accessTag;

		/** Victim tags */
		std::string victimNameTag;
		std::string victimRawNameTag;
		std::string victimIPTag;
		std::string victimHWIDTag;
		std::string victimRDNSTag;
		std::string victimSteamTag;
		std::string victimUUIDTag;
		std::string victimIDTag;
		std::string victimCharacterTag;
		std::string victimVehicleTag;
		std::string victimAdminTag;
		std::string victimPrefixTag;
		std::string victimGamePrefixTag;
		std::string victimTeamColorTag;
		std::string victimTeamShortTag;
		std::string victimTeamLongTag;
		std::string victimPingTag;
		std::string victimScoreTag;
		std::string victimScorePerMinuteTag;
		std::string victimCreditsTag;
		std::string victimKillsTag;
		std::string victimDeathsTag;
		std::string victimKDRTag;
		std::string victimSuicidesTag;
		std::string victimHeadshotsTag;
		std::string victimHeadshotKillRatioTag;
		std::string victimVehicleKillsTag;
		std::string victimBuildingKillsTag;
		std::string victimDefenceKillsTag;
		std::string victimGameTimeTag;
		std::string victimGamesTag;
		std::string victimGDIGamesTag;
		std::string victimNodGamesTag;
		std::string victimWinsTag;
		std::string victimGDIWinsTag;
		std::string victimNodWinsTag;
		std::string victimTiesTag;
		std::string victimLossesTag;
		std::string victimGDILossesTag;
		std::string victimNodLossesTag;
		std::string victimWinLossRatioTag;
		std::string victimGDIWinLossRatioTag;
		std::string victimNodWinLossRatioTag;
		std::string victimBeaconPlacementsTag;
		std::string victimBeaconDisarmsTag;
		std::string victimProxyPlacementsTag;
		std::string victimProxyDisarmsTag;
		std::string victimCapturesTag;
		std::string victimStealsTag;
		std::string victimStolenTag;
		std::string victimAccessTag;

		/** Building tags */
		std::string buildingNameTag;
		std::string buildingRawNameTag;
		std::string buildingHealthTag;
		std::string buildingMaxHealthTag;
		std::string buildingHealthPercentageTag;
		std::string buildingArmorTag;
		std::string buildingMaxArmorTag;
		std::string buildingArmorPercentageTag;
		std::string buildingDurabilityTag;
		std::string buildingMaxDurabilityTag;
		std::string buildingDurabilityPercentageTag;
		std::string buildingTeamColorTag;
		std::string buildingTeamShortTag;
		std::string buildingTeamLongTag;

		/** Ladder tags */
		std::string rankTag;
		std::string lastGameTag;
		std::string GDIScoreTag;
		std::string GDISPMTag;
		std::string GDIGameTimeTag;
		std::string GDITiesTag;
		std::string GDIBeaconPlacementsTag;
		std::string GDIBeaconDisarmsTag;
		std::string GDIProxyPlacementsTag;
		std::string GDIProxyDisarmsTag;
		std::string GDIKillsTag;
		std::string GDIDeathsTag;
		std::string GDIVehicleKillsTag;
		std::string GDIDefenceKillsTag;
		std::string GDIBuildingKillsTag;
		std::string GDIKDRTag;
		std::string GDIHeadshotsTag;
		std::string GDIHeadshotKillRatioTag;
		std::string NodScoreTag;
		std::string NodSPMTag;
		std::string NodGameTimeTag;
		std::string NodTiesTag;
		std::string NodBeaconPlacementsTag;
		std::string NodBeaconDisarmsTag;
		std::string NodProxyPlacementsTag;
		std::string NodProxyDisarmsTag;
		std::string NodKillsTag;
		std::string NodDeathsTag;
		std::string NodVehicleKillsTag;
		std::string NodDefenceKillsTag;
		std::string NodBuildingKillsTag;
		std::string NodKDRTag;
		std::string NodHeadshotsTag;
		std::string NodHeadshotKillRatioTag;

		/** Other tags */
		std::string weaponTag;
		std::string objectTag;
		std::string messageTag;
		std::string newNameTag;
		std::string winScoreTag;
		std::string loseScoreTag;
	};

	RENX_API extern Tags *tags;
}

/** Helper macro for processing tags */
#define PROCESS_TAG(tag, value) \
while(true) { \
index = msg.find(tag); \
if (index == std::string::npos) break; \
msg.replace(index, tag.size(), value); }

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_TAGS_H_HEADER