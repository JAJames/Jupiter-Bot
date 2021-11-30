/**
 * Copyright (C) 2015-2021 Jessica James.
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

#include "Jupiter/Reference_String.h"
#include "Jupiter/IRC_Client.h"
#include "RenX_Core.h"
#include "RenX_Functions.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BuildingInfo.h"
#include "RenX_Plugin.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;

struct TagsImp : RenX::Tags
{
	bool initialize();
	void processTags(std::string& msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building);
	void processTags(std::string& msg, const RenX::LadderDatabase::Entry &entry);
	void sanitizeTags(std::string& fmt);
	std::string_view getUniqueInternalTag();
private:
	Jupiter::StringS uniqueTag;
	uint32_t tagItr;
	size_t bar_width;
} _tags;
RenX::Tags *RenX::tags = &_tags;

bool RenX::Tags::initialize()
{
	return true;
}

bool TagsImp::initialize()
{
	this->tagItr = 0;
	this->uniqueTag = "\0\0\0\0\0\0"_jrs;

	Jupiter::Config &config = RenX::getCore()->getConfig();
	std::string_view configSection = config.get("TagDefinitions"_jrs, "Tags"_jrs);

	TagsImp::bar_width = config[configSection].get<int>("BarWidth"_jrs, 19);

	/** Global formats */
	this->dateFmt = static_cast<std::string>(config[configSection].get("DateFormat"_jrs, "%A, %B %d, %Y"_jrs));
	this->timeFmt = static_cast<std::string>(config[configSection].get("TimeFormat"_jrs, "%H:%M:%S"_jrs));

	/** Internal message tags */

	/** Global tags */
	this->INTERNAL_DATE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_TIME_TAG = this->getUniqueInternalTag();

	/** Server tags */
	this->INTERNAL_RCON_VERSION_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GAME_VERSION_TAG = this->getUniqueInternalTag();
	this->INTERNAL_RULES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_USER_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SERVER_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_MAP_TAG = this->getUniqueInternalTag();
	this->INTERNAL_MAP_GUID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SERVER_HOSTNAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SERVER_PORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SOCKET_HOSTNAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SOCKET_PORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SERVER_PREFIX_TAG = this->getUniqueInternalTag();

	/** Player tags*/
	this->INTERNAL_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_RAW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_IP_TAG = this->getUniqueInternalTag();
	this->INTERNAL_HWID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_RDNS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_STEAM_TAG = this->getUniqueInternalTag();
	this->INTERNAL_UUID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_ID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_CHARACTER_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VEHICLE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_ADMIN_TAG = this->getUniqueInternalTag();
	this->INTERNAL_PREFIX_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GAME_PREFIX_TAG = this->getUniqueInternalTag();
	this->INTERNAL_TEAM_COLOR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_TEAM_SHORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_TEAM_LONG_TAG = this->getUniqueInternalTag();
	this->INTERNAL_PING_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SCORE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SCORE_PER_MINUTE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_CREDITS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_DEATHS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_KDR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SUICIDES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_HEADSHOTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_HEADSHOT_KILL_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VEHICLE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_DEFENCE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GAME_TIME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GAMES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_GAMES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_GAMES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_TIES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_LOSSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_LOSSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_LOSSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_WIN_LOSS_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_WIN_LOSS_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_WIN_LOSS_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BEACON_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BEACON_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_PROXY_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_PROXY_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_CAPTURES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_STEALS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_STOLEN_TAG = this->getUniqueInternalTag();
	this->INTERNAL_ACCESS_TAG = this->getUniqueInternalTag();

	/** Victim tags */
	this->INTERNAL_VICTIM_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_RAW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_IP_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_HWID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_RDNS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_STEAM_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_UUID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_ID_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_CHARACTER_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_VEHICLE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_ADMIN_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_PREFIX_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GAME_PREFIX_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_TEAM_COLOR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_TEAM_SHORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_TEAM_LONG_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_PING_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_SCORE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_SCORE_PER_MINUTE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_CREDITS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_DEATHS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_KDR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_SUICIDES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_HEADSHOTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_HEADSHOT_KILL_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_BUILDING_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GAME_TIME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GAMES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GDI_GAMES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_NOD_GAMES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GDI_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_NOD_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_TIES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_LOSSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GDI_LOSSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_NOD_LOSSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_WIN_LOSS_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_GDI_WIN_LOSS_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_NOD_WIN_LOSS_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_BEACON_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_PROXY_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_PROXY_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_CAPTURES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_STEALS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_STOLEN_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_ACCESS_TAG = this->getUniqueInternalTag();

	/** Building tags */
	this->INTERNAL_BUILDING_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_RAW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_HEALTH_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_MAX_HEALTH_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_ARMOR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_MAX_ARMOR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_ARMOR_PERCENTAGE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_DURABILITY_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_MAX_DURABILITY_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_DURABILITY_PERCENTAGE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_TEAM_COLOR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_TEAM_SHORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_TEAM_LONG_TAG = this->getUniqueInternalTag();

	/** Ladder tags */
	this->INTERNAL_RANK_TAG = this->getUniqueInternalTag();
	this->INTERNAL_LAST_GAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_SCORE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_SPM_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_GAME_TIME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_TIES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_BEACON_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_BEACON_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_PROXY_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_PROXY_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_DEATHS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_VEHICLE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_DEFENCE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_BUILDING_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_KDR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_HEADSHOTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_GDI_HEADSHOT_KILL_RATIO_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_SCORE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_SPM_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_GAME_TIME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_TIES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_BEACON_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_BEACON_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_PROXY_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_PROXY_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_DEATHS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_VEHICLE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_DEFENCE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_BUILDING_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_KDR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_HEADSHOTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NOD_HEADSHOT_KILL_RATIO_TAG = this->getUniqueInternalTag();

	/** Other tags */
	this->INTERNAL_WEAPON_TAG = this->getUniqueInternalTag();
	this->INTERNAL_OBJECT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_MESSAGE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NEW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_WIN_SCORE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_LOSE_SCORE_TAG = this->getUniqueInternalTag();

	/** External (config) tags */

	/** Global tags */
	this->dateTag = config[configSection].get("DateTag"_jrs, "{DATE}"_jrs);
	this->timeTag = config[configSection].get("TimeTag"_jrs, "{TIME}"_jrs);

	/** Server tags */
	this->rconVersionTag = config[configSection].get("RCONVersionTag"_jrs, "{RVER}"_jrs);
	this->gameVersionTag = config[configSection].get("GameVersionTag"_jrs, "{GVER}"_jrs);
	this->rulesTag = config[configSection].get("RulesTag"_jrs, "{RULES}"_jrs);
	this->userTag = config[configSection].get("UserTag"_jrs, "{USER}"_jrs);
	this->serverNameTag = config[configSection].get("ServerNameTag"_jrs, "{SERVERNAME}"_jrs);
	this->mapTag = config[configSection].get("MapTag"_jrs, "{MAP}"_jrs);
	this->mapGUIDTag = config[configSection].get("MapGUIDTag"_jrs, "{MGUID}"_jrs);
	this->serverHostnameTag = config[configSection].get("ServerHostnameTag"_jrs, "{SERVERHOST}"_jrs);
	this->serverPortTag = config[configSection].get("ServerPortTag"_jrs, "{SERVERPORT}"_jrs);
	this->socketHostnameTag = config[configSection].get("SocketHostnameTag"_jrs, "{SOCKHOST}"_jrs);
	this->socketPortTag = config[configSection].get("SocketPortTag"_jrs, "{SOCKPORT}"_jrs);
	this->serverPrefixTag = config[configSection].get("ServerPrefixTag"_jrs, "{SERVERPREFIX}"_jrs);

	/** Player tags */
	this->nameTag = config[configSection].get("NameTag"_jrs, "{NAME}"_jrs);
	this->rawNameTag = config[configSection].get("RawNameTag"_jrs, "{RNAME}"_jrs);
	this->ipTag = config[configSection].get("IPTag"_jrs, "{IP}"_jrs);
	this->hwidTag = config[configSection].get("HWIDTag"_jrs, "{HWID}"_jrs);
	this->rdnsTag = config[configSection].get("RDNSTag"_jrs, "{RDNS}"_jrs);
	this->steamTag = config[configSection].get("SteamTag"_jrs, "{STEAM}"_jrs);
	this->uuidTag = config[configSection].get("UUIDTag"_jrs, "{UUID}"_jrs);
	this->idTag = config[configSection].get("IDTag"_jrs, "{ID}"_jrs);
	this->characterTag = config[configSection].get("CharacterTag"_jrs, "{CHAR}"_jrs);
	this->vehicleTag = config[configSection].get("VehicleTag"_jrs, "{VEH}"_jrs);
	this->adminTag = config[configSection].get("AdminTag"_jrs, "{ADMIN}"_jrs);
	this->prefixTag = config[configSection].get("PrefixTag"_jrs, "{PREFIX}"_jrs);
	this->gamePrefixTag = config[configSection].get("GamePrefixTag"_jrs, "{GPREFIX}"_jrs);
	this->teamColorTag = config[configSection].get("TeamColorTag"_jrs, "{TCOLOR}"_jrs);
	this->teamShortTag = config[configSection].get("ShortTeamTag"_jrs, "{TEAMS}"_jrs);
	this->teamLongTag = config[configSection].get("LongTeamTag"_jrs, "{TEAML}"_jrs);
	this->pingTag = config[configSection].get("PingTag"_jrs, "{PING}"_jrs);
	this->scoreTag = config[configSection].get("ScoreTag"_jrs, "{SCORE}"_jrs);
	this->scorePerMinuteTag = config[configSection].get("ScorePerMinuteTag"_jrs, "{SPM}"_jrs);
	this->creditsTag = config[configSection].get("CreditsTag"_jrs, "{CREDITS}"_jrs);
	this->killsTag = config[configSection].get("KillsTag"_jrs, "{KILLS}"_jrs);
	this->deathsTag = config[configSection].get("DeathsTag"_jrs, "{DEATHS}"_jrs);
	this->kdrTag = config[configSection].get("KDRTag"_jrs, "{KDR}"_jrs);
	this->suicidesTag = config[configSection].get("SuicidesTag"_jrs, "{SUICIDES}"_jrs);
	this->headshotsTag = config[configSection].get("HeadshotsTag"_jrs, "{HEADSHOTS}"_jrs);
	this->headshotKillRatioTag = config[configSection].get("HeadshotKillRatioTag"_jrs, "{HSKR}"_jrs);
	this->vehicleKillsTag = config[configSection].get("VehicleKillsTag"_jrs, "{VEHICLEKILLS}"_jrs);
	this->buildingKillsTag = config[configSection].get("BuildingKillsTag"_jrs, "{BUILDINGKILLS}"_jrs);
	this->defenceKillsTag = config[configSection].get("DefenceKillsTag"_jrs, "{DEFENCEKILLS}"_jrs);
	this->gameTimeTag = config[configSection].get("GameTimeTag"_jrs, "{GAMETIME}"_jrs);
	this->gamesTag = config[configSection].get("GamesTag"_jrs, "{GAMES}"_jrs);
	this->GDIGamesTag = config[configSection].get("GDIGamesTag"_jrs, "{GDIGAMES}"_jrs);
	this->NodGamesTag = config[configSection].get("NodGamesTag"_jrs, "{NODGAMES}"_jrs);
	this->winsTag = config[configSection].get("WinsTag"_jrs, "{WINS}"_jrs);
	this->GDIWinsTag = config[configSection].get("GDIWinsTag"_jrs, "{GDIWINS}"_jrs);
	this->NodWinsTag = config[configSection].get("NodWinsTag"_jrs, "{NODWINS}"_jrs);
	this->tiesTag = config[configSection].get("TiesTag"_jrs, "{TIES}"_jrs);
	this->lossesTag = config[configSection].get("LossesTag"_jrs, "{LOSSES}"_jrs);
	this->GDILossesTag = config[configSection].get("GDILossesTag"_jrs, "{GDILOSSES}"_jrs);
	this->NodLossesTag = config[configSection].get("NodLossesTag"_jrs, "{NODLOSSES}"_jrs);
	this->winLossRatioTag = config[configSection].get("WinLossRatioTag"_jrs, "{WLR}"_jrs);
	this->GDIWinLossRatioTag = config[configSection].get("GDIWinLossRatioTag"_jrs, "{GDIWLR}"_jrs);
	this->NodWinLossRatioTag = config[configSection].get("NodWinLossRatioTag"_jrs, "{NODWLR}"_jrs);
	this->beaconPlacementsTag = config[configSection].get("BeaconPlacementsTag"_jrs, "{BEACONPLACEMENTS}"_jrs);
	this->beaconDisarmsTag = config[configSection].get("BeaconDisarmsTag"_jrs, "{BEACONDISARMS}"_jrs);
	this->proxyPlacementsTag = config[configSection].get("ProxyPlacementsTag"_jrs, "{PROXYPLACEMENTS}"_jrs);
	this->proxyDisarmsTag = config[configSection].get("ProxyDisarmsTag"_jrs, "{PROXYDISARMS}"_jrs);
	this->capturesTag = config[configSection].get("CapturesTag"_jrs, "{CAPTURES}"_jrs);
	this->stealsTag = config[configSection].get("StealsTag"_jrs, "{STEALS}"_jrs);
	this->stolenTag = config[configSection].get("StolenTag"_jrs, "{STOLEN}"_jrs);
	this->accessTag = config[configSection].get("AccessTag"_jrs, "{ACCESS}"_jrs);

	/** Victim player tags */
	this->victimNameTag = config[configSection].get("VictimNameTag"_jrs, "{VNAME}"_jrs);
	this->victimRawNameTag = config[configSection].get("VictimRawNameTag"_jrs, "{VRNAME}"_jrs);
	this->victimIPTag = config[configSection].get("VictimIPTag"_jrs, "{VIP}"_jrs);
	this->victimHWIDTag = config[configSection].get("VictimHWIDTag"_jrs, "{VHWID}"_jrs);
	this->victimRDNSTag = config[configSection].get("VictimRDNSTag"_jrs, "{VRDNS}"_jrs);
	this->victimSteamTag = config[configSection].get("VictimSteamTag"_jrs, "{VSTEAM}"_jrs);
	this->victimUUIDTag = config[configSection].get("VictimUUIDTag"_jrs, "{VUUID}"_jrs);
	this->victimIDTag = config[configSection].get("VictimIDTag"_jrs, "{VID}"_jrs);
	this->victimCharacterTag = config[configSection].get("VictimCharacterTag"_jrs, "{VCHAR}"_jrs);
	this->victimVehicleTag = config[configSection].get("VictimVehicleTag"_jrs, "{VVEH}"_jrs);
	this->victimAdminTag = config[configSection].get("VictimAdminTag"_jrs, "{VADMIN}"_jrs);
	this->victimPrefixTag = config[configSection].get("VictimPrefixTag"_jrs, "{VPREFIX}"_jrs);
	this->victimGamePrefixTag = config[configSection].get("VictimGamePrefixTag"_jrs, "{VGPREFIX}"_jrs);
	this->victimTeamColorTag = config[configSection].get("VictimTeamColorTag"_jrs, "{VTCOLOR}"_jrs);
	this->victimTeamShortTag = config[configSection].get("VictimShortTeamTag"_jrs, "{VTEAMS}"_jrs);
	this->victimTeamLongTag = config[configSection].get("VictimLongTeamTag"_jrs, "{VTEAML}"_jrs);
	this->victimPingTag = config[configSection].get("VictimPingTag"_jrs, "{VPING}"_jrs);
	this->victimScoreTag = config[configSection].get("VictimScoreTag"_jrs, "{VSCORE}"_jrs);
	this->victimScorePerMinuteTag = config[configSection].get("VictimScorePerMinuteTag"_jrs, "{VSPM}"_jrs);
	this->victimCreditsTag = config[configSection].get("VictimCreditsTag"_jrs, "{VCREDITS}"_jrs);
	this->victimKillsTag = config[configSection].get("VictimKillsTag"_jrs, "{VKILLS}"_jrs);
	this->victimDeathsTag = config[configSection].get("VictimDeathsTag"_jrs, "{VDEATHS}"_jrs);
	this->victimKDRTag = config[configSection].get("VictimKDRTag"_jrs, "{VKDR}"_jrs);
	this->victimSuicidesTag = config[configSection].get("VictimSuicidesTag"_jrs, "{VSUICIDES}"_jrs);
	this->victimHeadshotsTag = config[configSection].get("VictimHeadshotsTag"_jrs, "{VHEADSHOTS}"_jrs);
	this->victimHeadshotKillRatioTag = config[configSection].get("VictimHeadshotKillRatioTag"_jrs, "{VHSKR}"_jrs);
	this->victimVehicleKillsTag = config[configSection].get("VictimVehicleKillsTag"_jrs, "{VVEHICLEKILLS}"_jrs);
	this->victimBuildingKillsTag = config[configSection].get("VictimBuildingKillsTag"_jrs, "{VBUILDINGKILLS}"_jrs);
	this->victimDefenceKillsTag = config[configSection].get("VictimDefenceKillsTag"_jrs, "{VDEFENCEKILLS}"_jrs);
	this->victimGameTimeTag = config[configSection].get("VictimGameTimeTag"_jrs, "{VGAMETIME}"_jrs);
	this->victimGamesTag = config[configSection].get("VictimGamesTag"_jrs, "{VGAMES}"_jrs);
	this->victimGDIGamesTag = config[configSection].get("VictimGDIGamesTag"_jrs, "{VGDIGAMES}"_jrs);
	this->victimNodGamesTag = config[configSection].get("VictimNodGamesTag"_jrs, "{VNODGAMES}"_jrs);
	this->victimWinsTag = config[configSection].get("VictimWinsTag"_jrs, "{VWINS}"_jrs);
	this->victimGDIWinsTag = config[configSection].get("VictimGDIWinsTag"_jrs, "{VGDIWINS}"_jrs);
	this->victimNodWinsTag = config[configSection].get("VictimNodWinsTag"_jrs, "{VNODWINS}"_jrs);
	this->victimTiesTag = config[configSection].get("VictimTiesTag"_jrs, "{VTIES}"_jrs);
	this->victimLossesTag = config[configSection].get("VictimLossesTag"_jrs, "{VLOSSES}"_jrs);
	this->victimGDILossesTag = config[configSection].get("VictimGDILossesTag"_jrs, "{VGDILOSSES}"_jrs);
	this->victimNodLossesTag = config[configSection].get("VictimNodLossesTag"_jrs, "{VNODLOSSES}"_jrs);
	this->victimWinLossRatioTag = config[configSection].get("VictimWinLossRatioTag"_jrs, "{VWLR}"_jrs);
	this->victimGDIWinLossRatioTag = config[configSection].get("VictimGDIWinLossRatioTag"_jrs, "{VGDIWLR}"_jrs);
	this->victimNodWinLossRatioTag = config[configSection].get("VictimNodWinLossRatioTag"_jrs, "{VNODWLR}"_jrs);
	this->victimBeaconPlacementsTag = config[configSection].get("VictimBeaconPlacementsTag"_jrs, "{VBEACONPLACEMENTS}"_jrs);
	this->victimBeaconDisarmsTag = config[configSection].get("VictimBeaconDisarmsTag"_jrs, "{VBEACONDISARMS}"_jrs);
	this->victimProxyPlacementsTag = config[configSection].get("VictimProxyPlacementsTag"_jrs, "{VPROXYPLACEMENTS}"_jrs);
	this->victimProxyDisarmsTag = config[configSection].get("VictimProxyDisarmsTag"_jrs, "{VPROXYDISARMS}"_jrs);
	this->victimCapturesTag = config[configSection].get("VictimCapturesTag"_jrs, "{VCAPTURES}"_jrs);
	this->victimStealsTag = config[configSection].get("VictimStealsTag"_jrs, "{VSTEALS}"_jrs);
	this->victimStolenTag = config[configSection].get("VictimStolenTag"_jrs, "{VSTOLEN}"_jrs);
	this->victimAccessTag = config[configSection].get("VictimAccessTag"_jrs, "{VACCESS}"_jrs);

	/** Building tags */
	this->buildingNameTag = config[configSection].get("BuildingNameTag"_jrs, "{BNAME}"_jrs);
	this->buildingRawNameTag = config[configSection].get("BuildingRawNameTag"_jrs, "{BRNAME}"_jrs);
	this->buildingHealthTag = config[configSection].get("BuildingHealthTag"_jrs, "{BHEALTH}"_jrs);
	this->buildingMaxHealthTag = config[configSection].get("BuildingMaxHealthTag"_jrs, "{BMHEALTH}"_jrs);
	this->buildingHealthPercentageTag = config[configSection].get("BuildingHealthPercentageTag"_jrs, "{BHP}"_jrs);
	this->buildingArmorTag = config[configSection].get("BuildingArmorTag"_jrs, "{BARMOR}"_jrs);
	this->buildingMaxArmorTag = config[configSection].get("BuildingMaxArmorTag"_jrs, "{BMARMOR}"_jrs);
	this->buildingArmorPercentageTag = config[configSection].get("BuildingArmorPercentageTag"_jrs, "{BAP}"_jrs);
	this->buildingDurabilityTag = config[configSection].get("BuildingDurabilityTag"_jrs, "{BDURABILITY}"_jrs);
	this->buildingMaxDurabilityTag = config[configSection].get("BuildingMaxDurabilityTag"_jrs, "{BMDURABILITY}"_jrs);
	this->buildingDurabilityPercentageTag = config[configSection].get("BuildingDurabilityPercentageTag"_jrs, "{BDP}"_jrs);
	this->buildingTeamColorTag = config[configSection].get("BuildingTeamColorTag"_jrs, "{BCOLOR}"_jrs);
	this->buildingTeamShortTag = config[configSection].get("BuildingShortTeamTag"_jrs, "{BTEAMS}"_jrs);
	this->buildingTeamLongTag = config[configSection].get("BuildingLongTeamTag"_jrs, "{BTEAML}"_jrs);

	/** Ladder tags */
	this->rankTag = config[configSection].get("RankTag"_jrs, "{RANK}"_jrs);
	this->lastGameTag = config[configSection].get("LastGameTag"_jrs, "{LASTGAME}"_jrs);
	this->GDIScoreTag = config[configSection].get("GDIScoreTag"_jrs, "{GDISCORE}"_jrs);
	this->GDISPMTag = config[configSection].get("GDISPMTag"_jrs, "{GDISPM}"_jrs);
	this->GDIGameTimeTag = config[configSection].get("GDIGameTimeTag"_jrs, "{GDIGAMETIME}"_jrs);
	this->GDITiesTag = config[configSection].get("GDITiesTag"_jrs, "{GDITIES}"_jrs);
	this->GDIBeaconPlacementsTag = config[configSection].get("GDIBeaconPlacementsTag"_jrs, "{GDIBEACONPLACEMENTS}"_jrs);
	this->GDIBeaconDisarmsTag = config[configSection].get("GDIBeaconDisarmsTag"_jrs, "{GDIBEACONDISARMS}"_jrs);
	this->GDIProxyPlacementsTag = config[configSection].get("GDIProxyPlacementsTag"_jrs, "{GDIPROXYPLACEMENTS}"_jrs);
	this->GDIProxyDisarmsTag = config[configSection].get("GDIProxyDisarmsTag"_jrs, "{GDIPROXYDISARMS}"_jrs);
	this->GDIKillsTag = config[configSection].get("GDIKillsTag"_jrs, "{GDIKILLS}"_jrs);
	this->GDIDeathsTag = config[configSection].get("GDIDeathsTag"_jrs, "{GDIDEATHS}"_jrs);
	this->GDIVehicleKillsTag = config[configSection].get("GDIVehicleKillsTag"_jrs, "{GDIVEHICLEKILLS}"_jrs);
	this->GDIDefenceKillsTag = config[configSection].get("GDIDefenceKillsTag"_jrs, "{GDIDEFENCEKILLS}"_jrs);
	this->GDIBuildingKillsTag = config[configSection].get("GDIBuildingKillsTag"_jrs, "{GDIBUILDINGKILLS}"_jrs);
	this->GDIKDRTag = config[configSection].get("GDIKDRTag"_jrs, "{GDIKDR}"_jrs);
	this->GDIHeadshotsTag = config[configSection].get("GDIHeadshotsTag"_jrs, "{GDIHEADSHOTS}"_jrs);
	this->GDIHeadshotKillRatioTag = config[configSection].get("GDIHeadshotKillRatioTag"_jrs, "{GDIHSKR}"_jrs);
	this->NodScoreTag = config[configSection].get("NodScoreTag"_jrs, "{NODSCORE}"_jrs);
	this->NodSPMTag = config[configSection].get("NodSPMTag"_jrs, "{NODSPM}"_jrs);
	this->NodGameTimeTag = config[configSection].get("NodGameTimeTag"_jrs, "{NODGAMETIME}"_jrs);
	this->NodTiesTag = config[configSection].get("NodTiesTag"_jrs, "{NODTIES}"_jrs);
	this->NodBeaconPlacementsTag = config[configSection].get("NodBeaconPlacementsTag"_jrs, "{NODBEACONPLACEMENTS}"_jrs);
	this->NodBeaconDisarmsTag = config[configSection].get("NodBeaconDisarmsTag"_jrs, "{NODBEACONDISARMS}"_jrs);
	this->NodProxyPlacementsTag = config[configSection].get("NodProxyPlacementsTag"_jrs, "{NODPROXYPLACEMENTS}"_jrs);
	this->NodProxyDisarmsTag = config[configSection].get("NodProxyDisarmsTag"_jrs, "{NODPROXYDISARMS}"_jrs);
	this->NodKillsTag = config[configSection].get("NodKillsTag"_jrs, "{NODKILLS}"_jrs);
	this->NodDeathsTag = config[configSection].get("NodDeathsTag"_jrs, "{NODDEATHS}"_jrs);
	this->NodVehicleKillsTag = config[configSection].get("NodVehicleKillsTag"_jrs, "{NODVEHICLEKILLS}"_jrs);
	this->NodDefenceKillsTag = config[configSection].get("NodDefenceKillsTag"_jrs, "{NODDEFENCEKILLS}"_jrs);
	this->NodBuildingKillsTag = config[configSection].get("NodBuildingKillsTag"_jrs, "{NODBUILDINGKILLS}"_jrs);
	this->NodKDRTag = config[configSection].get("NodKDRTag"_jrs, "{NODKDR}"_jrs);
	this->NodHeadshotsTag = config[configSection].get("NodHeadshotsTag"_jrs, "{NODHEADSHOTS}"_jrs);
	this->NodHeadshotKillRatioTag = config[configSection].get("NodHeadshotKillRatioTag"_jrs, "{NODHSKR}"_jrs);

	/** Other tags */
	this->weaponTag = config[configSection].get("WeaponTag"_jrs, "{WEAPON}"_jrs);
	this->objectTag = config[configSection].get("ObjectTag"_jrs, "{OBJECT}"_jrs);
	this->messageTag = config[configSection].get("MessageTag"_jrs, "{MESSAGE}"_jrs);
	this->newNameTag = config[configSection].get("NewNameTag"_jrs, "{NNAME}"_jrs);
	this->winScoreTag = config[configSection].get("WinScoreTag"_jrs, "{WINSCORE}"_jrs);
	this->loseScoreTag = config[configSection].get("LoseScoreTag"_jrs, "{LOSESCORE}"_jrs);

	return true;
}

double get_ratio(double num, double denom)
{
	if (denom == 0.0f)
		return num;
	return num / denom;
}

void TagsImp::processTags(std::string& msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building)
{
	size_t index;
	PROCESS_TAG(this->INTERNAL_DATE_TAG, std::string_view(getTimeFormat(this->dateFmt.c_str())));
	PROCESS_TAG(this->INTERNAL_TIME_TAG, std::string_view(getTimeFormat(this->timeFmt.c_str())));
	if (server != nullptr)
	{
		PROCESS_TAG(this->INTERNAL_RCON_VERSION_TAG, Jupiter::StringS::Format("%u", server->getVersion()));
		PROCESS_TAG(this->INTERNAL_GAME_VERSION_TAG, server->getGameVersion());
		PROCESS_TAG(this->INTERNAL_RULES_TAG, server->getRules());
		PROCESS_TAG(this->INTERNAL_USER_TAG, server->getUser());
		PROCESS_TAG(this->INTERNAL_SERVER_NAME_TAG, server->getName());
		PROCESS_TAG(this->INTERNAL_MAP_TAG, server->getMap().name);
		PROCESS_TAG(this->INTERNAL_MAP_GUID_TAG, RenX::formatGUID(server->getMap()));
		PROCESS_TAG(this->INTERNAL_SERVER_HOSTNAME_TAG, server->getHostname());
		PROCESS_TAG(this->INTERNAL_SERVER_PORT_TAG, Jupiter::StringS::Format("%u", server->getPort()));
		PROCESS_TAG(this->INTERNAL_SOCKET_HOSTNAME_TAG, server->getSocketHostname());
		PROCESS_TAG(this->INTERNAL_SOCKET_PORT_TAG, Jupiter::StringS::Format("%u", server->getSocketPort()));
		PROCESS_TAG(this->INTERNAL_SERVER_PREFIX_TAG, server->getPrefix());
		if (player != nullptr)
		{
			PROCESS_TAG(this->INTERNAL_STEAM_TAG, server->formatSteamID(*player));
		}
		if (victim != nullptr)
		{
			PROCESS_TAG(this->INTERNAL_VICTIM_STEAM_TAG, server->formatSteamID(*victim));
		}
	}
	if (player != nullptr)
	{
		PROCESS_TAG(this->INTERNAL_NAME_TAG, RenX::getFormattedPlayerName(*player));
		PROCESS_TAG(this->INTERNAL_RAW_NAME_TAG, player->name);
		PROCESS_TAG(this->INTERNAL_IP_TAG, player->ip);
		PROCESS_TAG(this->INTERNAL_HWID_TAG, player->hwid);
		if (player->rdns_pending) {
			PROCESS_TAG(this->INTERNAL_RDNS_TAG, RenX::rdns_pending);
		}
		else {
			PROCESS_TAG(this->INTERNAL_RDNS_TAG, player->get_rdns());
		}
		PROCESS_TAG(this->INTERNAL_UUID_TAG, player->uuid);
		PROCESS_TAG(this->INTERNAL_ID_TAG, Jupiter::StringS::Format("%d", player->id));
		PROCESS_TAG(this->INTERNAL_CHARACTER_TAG, RenX::translateName(player->character));
		PROCESS_TAG(this->INTERNAL_VEHICLE_TAG, RenX::translateName(player->vehicle));
		PROCESS_TAG(this->INTERNAL_ADMIN_TAG, player->adminType);
		PROCESS_TAG(this->INTERNAL_PREFIX_TAG, player->formatNamePrefix);
		PROCESS_TAG(this->INTERNAL_GAME_PREFIX_TAG, player->gamePrefix);
		PROCESS_TAG(this->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(player->team));
		PROCESS_TAG(this->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(player->team));
		PROCESS_TAG(this->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(player->team));
		PROCESS_TAG(this->INTERNAL_PING_TAG, Jupiter::StringS::Format("%hu", player->ping));
		PROCESS_TAG(this->INTERNAL_SCORE_TAG, Jupiter::StringS::Format("%.0f", player->score));
		PROCESS_TAG(this->INTERNAL_SCORE_PER_MINUTE_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(player->score), static_cast<double>((std::chrono::steady_clock::now() - player->joinTime).count()) / 60.0)));
		PROCESS_TAG(this->INTERNAL_CREDITS_TAG, Jupiter::StringS::Format("%.0f", player->credits));
		PROCESS_TAG(this->INTERNAL_KILLS_TAG, Jupiter::StringS::Format("%u", player->kills));
		PROCESS_TAG(this->INTERNAL_DEATHS_TAG, Jupiter::StringS::Format("%u", player->deaths));
		PROCESS_TAG(this->INTERNAL_KDR_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(player->kills), static_cast<double>(player->deaths))));
		PROCESS_TAG(this->INTERNAL_SUICIDES_TAG, Jupiter::StringS::Format("%u", player->suicides));
		PROCESS_TAG(this->INTERNAL_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", player->headshots));
		PROCESS_TAG(this->INTERNAL_HEADSHOT_KILL_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(player->headshots, player->kills)));
		PROCESS_TAG(this->INTERNAL_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", player->vehicleKills));
		PROCESS_TAG(this->INTERNAL_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", player->buildingKills));
		PROCESS_TAG(this->INTERNAL_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", player->defenceKills));
		PROCESS_TAG(this->INTERNAL_WINS_TAG, Jupiter::StringS::Format("%u", player->wins));
		PROCESS_TAG(this->INTERNAL_LOSSES_TAG, Jupiter::StringS::Format("%u", player->loses));
		PROCESS_TAG(this->INTERNAL_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", player->beaconPlacements));
		PROCESS_TAG(this->INTERNAL_BEACON_DISARMS_TAG, Jupiter::StringS::Format("%u", player->beaconDisarms));
		PROCESS_TAG(this->INTERNAL_CAPTURES_TAG, Jupiter::StringS::Format("%u", player->captures));
		PROCESS_TAG(this->INTERNAL_STEALS_TAG, Jupiter::StringS::Format("%u", player->steals));
		PROCESS_TAG(this->INTERNAL_STOLEN_TAG, Jupiter::StringS::Format("%u", player->stolen));
		PROCESS_TAG(this->INTERNAL_ACCESS_TAG, Jupiter::StringS::Format("%d", player->access));
	}
	if (victim != nullptr)
	{
		PROCESS_TAG(this->INTERNAL_VICTIM_NAME_TAG, RenX::getFormattedPlayerName(*victim));
		PROCESS_TAG(this->INTERNAL_VICTIM_RAW_NAME_TAG, victim->name);
		PROCESS_TAG(this->INTERNAL_VICTIM_IP_TAG, victim->ip);
		PROCESS_TAG(this->INTERNAL_VICTIM_HWID_TAG, victim->hwid);
		if (victim->rdns_pending) {
			PROCESS_TAG(this->INTERNAL_VICTIM_RDNS_TAG, RenX::rdns_pending);
		}
		else {
			PROCESS_TAG(this->INTERNAL_VICTIM_RDNS_TAG, victim->get_rdns());
		}
		PROCESS_TAG(this->INTERNAL_VICTIM_UUID_TAG, victim->uuid);
		PROCESS_TAG(this->INTERNAL_VICTIM_ID_TAG, Jupiter::StringS::Format("%d", victim->id));
		PROCESS_TAG(this->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(victim->character));
		PROCESS_TAG(this->INTERNAL_VICTIM_VEHICLE_TAG, RenX::translateName(victim->vehicle));
		PROCESS_TAG(this->INTERNAL_VICTIM_ADMIN_TAG, victim->adminType);
		PROCESS_TAG(this->INTERNAL_VICTIM_PREFIX_TAG, victim->formatNamePrefix);
		PROCESS_TAG(this->INTERNAL_VICTIM_GAME_PREFIX_TAG, victim->gamePrefix);
		PROCESS_TAG(this->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(victim->team));
		PROCESS_TAG(this->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(victim->team));
		PROCESS_TAG(this->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(victim->team));
		PROCESS_TAG(this->INTERNAL_VICTIM_PING_TAG, Jupiter::StringS::Format("%hu", victim->ping));
		PROCESS_TAG(this->INTERNAL_VICTIM_SCORE_TAG, Jupiter::StringS::Format("%.0f", victim->score));
		PROCESS_TAG(this->INTERNAL_VICTIM_SCORE_PER_MINUTE_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(victim->score), static_cast<double>((std::chrono::steady_clock::now() - victim->joinTime).count()) / 60.0)));
		PROCESS_TAG(this->INTERNAL_VICTIM_CREDITS_TAG, Jupiter::StringS::Format("%.0f", victim->credits));
		PROCESS_TAG(this->INTERNAL_VICTIM_KILLS_TAG, Jupiter::StringS::Format("%u", victim->kills));
		PROCESS_TAG(this->INTERNAL_VICTIM_DEATHS_TAG, Jupiter::StringS::Format("%u", victim->deaths));
		PROCESS_TAG(this->INTERNAL_VICTIM_KDR_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(victim->kills), static_cast<double>(victim->deaths))));
		PROCESS_TAG(this->INTERNAL_VICTIM_SUICIDES_TAG, Jupiter::StringS::Format("%u", victim->suicides));
		PROCESS_TAG(this->INTERNAL_VICTIM_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", victim->headshots));
		PROCESS_TAG(this->INTERNAL_VICTIM_HEADSHOT_KILL_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(victim->headshots, victim->kills)));
		PROCESS_TAG(this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", victim->vehicleKills));
		PROCESS_TAG(this->INTERNAL_VICTIM_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", victim->buildingKills));
		PROCESS_TAG(this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", victim->defenceKills));
		PROCESS_TAG(this->INTERNAL_VICTIM_WINS_TAG, Jupiter::StringS::Format("%u", victim->wins));
		PROCESS_TAG(this->INTERNAL_VICTIM_LOSSES_TAG, Jupiter::StringS::Format("%u", victim->loses));
		PROCESS_TAG(this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", victim->beaconPlacements));
		PROCESS_TAG(this->INTERNAL_VICTIM_BEACON_DISARMS_TAG, Jupiter::StringS::Format("%u", victim->beaconDisarms));
		PROCESS_TAG(this->INTERNAL_VICTIM_CAPTURES_TAG, Jupiter::StringS::Format("%u", victim->captures));
		PROCESS_TAG(this->INTERNAL_VICTIM_STEALS_TAG, Jupiter::StringS::Format("%u", victim->steals));
		PROCESS_TAG(this->INTERNAL_VICTIM_STOLEN_TAG, Jupiter::StringS::Format("%u", victim->stolen));
		PROCESS_TAG(this->INTERNAL_VICTIM_ACCESS_TAG, Jupiter::StringS::Format("%d", victim->access));
	}
	if (building != nullptr)
	{
		PROCESS_TAG(this->INTERNAL_BUILDING_NAME_TAG, RenX::translateName(building->name));
		PROCESS_TAG(this->INTERNAL_BUILDING_RAW_NAME_TAG, building->name);
		PROCESS_TAG(this->INTERNAL_BUILDING_HEALTH_TAG, Jupiter::StringS::Format("%d", building->health));
		PROCESS_TAG(this->INTERNAL_BUILDING_MAX_HEALTH_TAG, Jupiter::StringS::Format("%d", building->max_health));
		PROCESS_TAG(this->INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG, Jupiter::StringS::Format("%.0f", (building->health / building->max_health) * 100.0));
		PROCESS_TAG(this->INTERNAL_BUILDING_ARMOR_TAG, Jupiter::StringS::Format("%d", building->armor));
		PROCESS_TAG(this->INTERNAL_BUILDING_MAX_ARMOR_TAG, Jupiter::StringS::Format("%d", building->max_armor));
		PROCESS_TAG(this->INTERNAL_BUILDING_ARMOR_PERCENTAGE_TAG, Jupiter::StringS::Format("%.0f", (static_cast<double>(building->armor) / static_cast<double>(building->max_armor)) * 100.0));
		PROCESS_TAG(this->INTERNAL_BUILDING_DURABILITY_TAG, Jupiter::StringS::Format("%d", building->health + building->armor));
		PROCESS_TAG(this->INTERNAL_BUILDING_MAX_DURABILITY_TAG, Jupiter::StringS::Format("%d", building->max_health + building->max_armor));
		PROCESS_TAG(this->INTERNAL_BUILDING_DURABILITY_PERCENTAGE_TAG, Jupiter::StringS::Format("%.0f", (static_cast<double>(building->health + building->armor) / static_cast<double>(building->max_health + building->max_armor)) * 100.0));
		PROCESS_TAG(this->INTERNAL_BUILDING_TEAM_COLOR_TAG, RenX::getTeamColor(building->team));
		PROCESS_TAG(this->INTERNAL_BUILDING_TEAM_SHORT_TAG, RenX::getTeamName(building->team));
		PROCESS_TAG(this->INTERNAL_BUILDING_TEAM_LONG_TAG, RenX::getFullTeamName(building->team));
	}

	for (const auto& plugin : RenX::getCore()->getPlugins()) {
		plugin->RenX_ProcessTags(msg, server, player, victim, building);
	}
}

void TagsImp::processTags(std::string& msg, const RenX::LadderDatabase::Entry &entry)
{
	size_t index;
	uint32_t total_tied_games = entry.total_wins - entry.total_gdi_wins - entry.total_nod_wins;

	PROCESS_TAG(this->INTERNAL_NAME_TAG, entry.most_recent_name);
	PROCESS_TAG(this->INTERNAL_STEAM_TAG, Jupiter::StringS::Format("%llu", entry.steam_id));
	PROCESS_TAG(this->INTERNAL_RANK_TAG, Jupiter::StringS::Format("%u", entry.rank));
	PROCESS_TAG(this->INTERNAL_LAST_GAME_TAG, Jupiter::StringS::Format("XX Xuary 20XX at 00:00:00")); // TODO: format this!

	/** Totals */
	PROCESS_TAG(this->INTERNAL_SCORE_TAG, Jupiter::StringS::Format("%llu", entry.total_score));
	PROCESS_TAG(this->INTERNAL_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_kills));
	PROCESS_TAG(this->INTERNAL_DEATHS_TAG, Jupiter::StringS::Format("%u", entry.total_deaths));
	PROCESS_TAG(this->INTERNAL_KDR_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_kills), static_cast<double>(entry.total_deaths))));
	PROCESS_TAG(this->INTERNAL_SCORE_PER_MINUTE_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_score), static_cast<double>(entry.total_game_time) / 60.0)));
	PROCESS_TAG(this->INTERNAL_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", entry.total_headshot_kills));
	PROCESS_TAG(this->INTERNAL_HEADSHOT_KILL_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(entry.total_headshot_kills, entry.total_kills)));
	PROCESS_TAG(this->INTERNAL_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_vehicle_kills));
	PROCESS_TAG(this->INTERNAL_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_building_kills));
	PROCESS_TAG(this->INTERNAL_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_defence_kills));
	PROCESS_TAG(this->INTERNAL_CAPTURES_TAG, Jupiter::StringS::Format("%u", entry.total_captures));
	PROCESS_TAG(this->INTERNAL_GAME_TIME_TAG, Jupiter::StringS::Format("%u", entry.total_game_time));
	PROCESS_TAG(this->INTERNAL_GAMES_TAG, Jupiter::StringS::Format("%u", entry.total_games));
	PROCESS_TAG(this->INTERNAL_WINS_TAG, Jupiter::StringS::Format("%u", entry.total_wins));
	PROCESS_TAG(this->INTERNAL_TIES_TAG, Jupiter::StringS::Format("%u", total_tied_games));
	PROCESS_TAG(this->INTERNAL_LOSSES_TAG, Jupiter::StringS::Format("%u", entry.total_games - total_tied_games - entry.total_wins));
	PROCESS_TAG(this->INTERNAL_WIN_LOSS_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_wins), static_cast<double>(entry.total_games - entry.total_wins))));
	PROCESS_TAG(this->INTERNAL_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.total_beacon_placements));
	PROCESS_TAG(this->INTERNAL_BEACON_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.total_beacon_disarms));
	PROCESS_TAG(this->INTERNAL_PROXY_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.total_proxy_placements));
	PROCESS_TAG(this->INTERNAL_PROXY_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.total_proxy_disarms));

	/** GDI Totals */
	PROCESS_TAG(this->INTERNAL_GDI_GAMES_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_games));
	PROCESS_TAG(this->INTERNAL_GDI_WINS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_wins));
	PROCESS_TAG(this->INTERNAL_GDI_TIES_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_ties));
	PROCESS_TAG(this->INTERNAL_GDI_LOSSES_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_games - entry.total_gdi_wins - entry.total_gdi_ties));
	PROCESS_TAG(this->INTERNAL_GDI_WIN_LOSS_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_gdi_wins), static_cast<double>(entry.total_gdi_games - entry.total_gdi_wins - entry.total_gdi_ties))));
	PROCESS_TAG(this->INTERNAL_GDI_SCORE_TAG, Jupiter::StringS::Format("%llu", entry.total_gdi_score));
	PROCESS_TAG(this->INTERNAL_GDI_SPM_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_gdi_score), static_cast<double>(entry.total_gdi_game_time) / 60.0)));
	PROCESS_TAG(this->INTERNAL_GDI_GAME_TIME_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_game_time));
	PROCESS_TAG(this->INTERNAL_GDI_TIES_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_ties));
	PROCESS_TAG(this->INTERNAL_GDI_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_beacon_placements));
	PROCESS_TAG(this->INTERNAL_GDI_BEACON_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_beacon_disarms));
	PROCESS_TAG(this->INTERNAL_GDI_PROXY_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_proxy_placements));
	PROCESS_TAG(this->INTERNAL_GDI_PROXY_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_proxy_disarms));
	PROCESS_TAG(this->INTERNAL_GDI_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_kills));
	PROCESS_TAG(this->INTERNAL_GDI_DEATHS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_deaths));
	PROCESS_TAG(this->INTERNAL_GDI_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_vehicle_kills));
	PROCESS_TAG(this->INTERNAL_GDI_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_defence_kills));
	PROCESS_TAG(this->INTERNAL_GDI_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_building_kills));
	PROCESS_TAG(this->INTERNAL_GDI_KDR_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_gdi_kills), static_cast<double>(entry.total_gdi_deaths))));
	PROCESS_TAG(this->INTERNAL_GDI_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", entry.total_gdi_headshots));
	PROCESS_TAG(this->INTERNAL_GDI_HEADSHOT_KILL_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_gdi_headshots), static_cast<double>(entry.total_gdi_kills))));

	/** Nod Totals */
	PROCESS_TAG(this->INTERNAL_NOD_GAMES_TAG, Jupiter::StringS::Format("%u", entry.total_nod_games));
	PROCESS_TAG(this->INTERNAL_NOD_WINS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_wins));
	PROCESS_TAG(this->INTERNAL_NOD_TIES_TAG, Jupiter::StringS::Format("%u", entry.total_nod_ties));
	PROCESS_TAG(this->INTERNAL_NOD_LOSSES_TAG, Jupiter::StringS::Format("%u", entry.total_nod_games - entry.total_nod_wins - entry.total_nod_ties));
	PROCESS_TAG(this->INTERNAL_NOD_WIN_LOSS_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_nod_wins), static_cast<double>(entry.total_nod_games - entry.total_nod_wins - entry.total_nod_ties))));
	PROCESS_TAG(this->INTERNAL_NOD_SCORE_TAG, Jupiter::StringS::Format("%llu", entry.total_nod_score));
	PROCESS_TAG(this->INTERNAL_NOD_SPM_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_nod_score), static_cast<double>(entry.total_nod_game_time) / 60.0)));
	PROCESS_TAG(this->INTERNAL_NOD_GAME_TIME_TAG, Jupiter::StringS::Format("%u", entry.total_nod_game_time));
	PROCESS_TAG(this->INTERNAL_NOD_TIES_TAG, Jupiter::StringS::Format("%u", entry.total_nod_ties));
	PROCESS_TAG(this->INTERNAL_NOD_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_beacon_placements));
	PROCESS_TAG(this->INTERNAL_NOD_BEACON_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_beacon_disarms));
	PROCESS_TAG(this->INTERNAL_NOD_PROXY_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_proxy_placements));
	PROCESS_TAG(this->INTERNAL_NOD_PROXY_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_proxy_disarms));
	PROCESS_TAG(this->INTERNAL_NOD_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_kills));
	PROCESS_TAG(this->INTERNAL_NOD_DEATHS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_deaths));
	PROCESS_TAG(this->INTERNAL_NOD_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_vehicle_kills));
	PROCESS_TAG(this->INTERNAL_NOD_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_defence_kills));
	PROCESS_TAG(this->INTERNAL_NOD_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_building_kills));
	PROCESS_TAG(this->INTERNAL_NOD_KDR_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_nod_kills), static_cast<double>(entry.total_nod_deaths))));
	PROCESS_TAG(this->INTERNAL_NOD_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", entry.total_nod_headshots));
	PROCESS_TAG(this->INTERNAL_NOD_HEADSHOT_KILL_RATIO_TAG, Jupiter::StringS::Format("%.2f", get_ratio(static_cast<double>(entry.total_nod_headshots), static_cast<double>(entry.total_nod_kills))));

	/** Tops */
	PROCESS_TAG(this->INTERNAL_VICTIM_SCORE_TAG, Jupiter::StringS::Format("%u", entry.top_score));
	PROCESS_TAG(this->INTERNAL_VICTIM_KILLS_TAG, Jupiter::StringS::Format("%u", entry.top_kills));
	PROCESS_TAG(this->INTERNAL_VICTIM_DEATHS_TAG, Jupiter::StringS::Format("%u", entry.most_deaths));
	PROCESS_TAG(this->INTERNAL_VICTIM_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", entry.top_headshot_kills));
	PROCESS_TAG(this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.top_vehicle_kills));
	PROCESS_TAG(this->INTERNAL_VICTIM_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", entry.top_building_kills));
	PROCESS_TAG(this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", entry.top_defence_kills));
	PROCESS_TAG(this->INTERNAL_VICTIM_CAPTURES_TAG, Jupiter::StringS::Format("%u", entry.top_captures));
	PROCESS_TAG(this->INTERNAL_VICTIM_GAME_TIME_TAG, Jupiter::StringS::Format("%u", entry.top_game_time));
	PROCESS_TAG(this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.top_beacon_placements));
	PROCESS_TAG(this->INTERNAL_VICTIM_BEACON_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.top_beacon_disarms));
	PROCESS_TAG(this->INTERNAL_VICTIM_PROXY_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", entry.top_proxy_placements));
	PROCESS_TAG(this->INTERNAL_VICTIM_PROXY_DISARMS_TAG, Jupiter::StringS::Format("%u", entry.top_proxy_disarms));
}

void TagsImp::sanitizeTags(std::string& fmt)
{
	/** Global tags */
	RenX::replace_tag(fmt, this->dateTag, this->INTERNAL_DATE_TAG);
	RenX::replace_tag(fmt, this->timeTag, this->INTERNAL_TIME_TAG);

	/** Server tags */
	RenX::replace_tag(fmt, this->rconVersionTag, this->INTERNAL_RCON_VERSION_TAG);
	RenX::replace_tag(fmt, this->gameVersionTag, this->INTERNAL_GAME_VERSION_TAG);
	RenX::replace_tag(fmt, this->rulesTag, this->INTERNAL_RULES_TAG);
	RenX::replace_tag(fmt, this->userTag, this->INTERNAL_USER_TAG);
	RenX::replace_tag(fmt, this->serverNameTag, this->INTERNAL_SERVER_NAME_TAG);
	RenX::replace_tag(fmt, this->mapTag, this->INTERNAL_MAP_TAG);
	RenX::replace_tag(fmt, this->mapGUIDTag, this->INTERNAL_MAP_GUID_TAG);
	RenX::replace_tag(fmt, this->serverHostnameTag, this->INTERNAL_SERVER_HOSTNAME_TAG);
	RenX::replace_tag(fmt, this->serverPortTag, this->INTERNAL_SERVER_PORT_TAG);
	RenX::replace_tag(fmt, this->socketHostnameTag, this->INTERNAL_SOCKET_HOSTNAME_TAG);
	RenX::replace_tag(fmt, this->socketPortTag, this->INTERNAL_SOCKET_PORT_TAG);
	RenX::replace_tag(fmt, this->serverPrefixTag, this->INTERNAL_SERVER_PREFIX_TAG);

	/** Player tags */
	RenX::replace_tag(fmt, this->nameTag, this->INTERNAL_NAME_TAG);
	RenX::replace_tag(fmt, this->rawNameTag, this->INTERNAL_RAW_NAME_TAG);
	RenX::replace_tag(fmt, this->ipTag, this->INTERNAL_IP_TAG);
	RenX::replace_tag(fmt, this->hwidTag, this->INTERNAL_HWID_TAG);
	RenX::replace_tag(fmt, this->rdnsTag, this->INTERNAL_RDNS_TAG);
	RenX::replace_tag(fmt, this->steamTag, this->INTERNAL_STEAM_TAG);
	RenX::replace_tag(fmt, this->uuidTag, this->INTERNAL_UUID_TAG);
	RenX::replace_tag(fmt, this->idTag, this->INTERNAL_ID_TAG);
	RenX::replace_tag(fmt, this->characterTag, this->INTERNAL_CHARACTER_TAG);
	RenX::replace_tag(fmt, this->vehicleTag, this->INTERNAL_VEHICLE_TAG);
	RenX::replace_tag(fmt, this->adminTag, this->INTERNAL_ADMIN_TAG);
	RenX::replace_tag(fmt, this->prefixTag, this->INTERNAL_PREFIX_TAG);
	RenX::replace_tag(fmt, this->gamePrefixTag, this->INTERNAL_GAME_PREFIX_TAG);
	RenX::replace_tag(fmt, this->teamColorTag, this->INTERNAL_TEAM_COLOR_TAG);
	RenX::replace_tag(fmt, this->teamShortTag, this->INTERNAL_TEAM_SHORT_TAG);
	RenX::replace_tag(fmt, this->teamLongTag, this->INTERNAL_TEAM_LONG_TAG);
	RenX::replace_tag(fmt, this->pingTag, this->INTERNAL_PING_TAG);
	RenX::replace_tag(fmt, this->scoreTag, this->INTERNAL_SCORE_TAG);
	RenX::replace_tag(fmt, this->scorePerMinuteTag, this->INTERNAL_SCORE_PER_MINUTE_TAG);
	RenX::replace_tag(fmt, this->creditsTag, this->INTERNAL_CREDITS_TAG);
	RenX::replace_tag(fmt, this->killsTag, this->INTERNAL_KILLS_TAG);
	RenX::replace_tag(fmt, this->deathsTag, this->INTERNAL_DEATHS_TAG);
	RenX::replace_tag(fmt, this->kdrTag, this->INTERNAL_KDR_TAG);
	RenX::replace_tag(fmt, this->suicidesTag, this->INTERNAL_SUICIDES_TAG);
	RenX::replace_tag(fmt, this->headshotsTag, this->INTERNAL_HEADSHOTS_TAG);
	RenX::replace_tag(fmt, this->headshotKillRatioTag, this->INTERNAL_HEADSHOT_KILL_RATIO_TAG);
	RenX::replace_tag(fmt, this->vehicleKillsTag, this->INTERNAL_VEHICLE_KILLS_TAG);
	RenX::replace_tag(fmt, this->buildingKillsTag, this->INTERNAL_BUILDING_KILLS_TAG);
	RenX::replace_tag(fmt, this->defenceKillsTag, this->INTERNAL_DEFENCE_KILLS_TAG);
	RenX::replace_tag(fmt, this->gameTimeTag, this->INTERNAL_GAME_TIME_TAG);
	RenX::replace_tag(fmt, this->gamesTag, this->INTERNAL_GAMES_TAG);
	RenX::replace_tag(fmt, this->GDIGamesTag, this->INTERNAL_GDI_GAMES_TAG);
	RenX::replace_tag(fmt, this->NodGamesTag, this->INTERNAL_NOD_GAMES_TAG);
	RenX::replace_tag(fmt, this->winsTag, this->INTERNAL_WINS_TAG);
	RenX::replace_tag(fmt, this->GDIWinsTag, this->INTERNAL_GDI_WINS_TAG);
	RenX::replace_tag(fmt, this->NodWinsTag, this->INTERNAL_NOD_WINS_TAG);
	RenX::replace_tag(fmt, this->tiesTag, this->INTERNAL_TIES_TAG);
	RenX::replace_tag(fmt, this->lossesTag, this->INTERNAL_LOSSES_TAG);
	RenX::replace_tag(fmt, this->GDILossesTag, this->INTERNAL_GDI_LOSSES_TAG);
	RenX::replace_tag(fmt, this->NodLossesTag, this->INTERNAL_NOD_LOSSES_TAG);
	RenX::replace_tag(fmt, this->winLossRatioTag, this->INTERNAL_WIN_LOSS_RATIO_TAG);
	RenX::replace_tag(fmt, this->GDIWinLossRatioTag, this->INTERNAL_GDI_WIN_LOSS_RATIO_TAG);
	RenX::replace_tag(fmt, this->NodWinLossRatioTag, this->INTERNAL_NOD_WIN_LOSS_RATIO_TAG);
	RenX::replace_tag(fmt, this->beaconPlacementsTag, this->INTERNAL_BEACON_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->beaconDisarmsTag, this->INTERNAL_BEACON_DISARMS_TAG);
	RenX::replace_tag(fmt, this->proxyPlacementsTag, this->INTERNAL_PROXY_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->proxyDisarmsTag, this->INTERNAL_PROXY_DISARMS_TAG);
	RenX::replace_tag(fmt, this->capturesTag, this->INTERNAL_CAPTURES_TAG);
	RenX::replace_tag(fmt, this->stealsTag, this->INTERNAL_STEALS_TAG);
	RenX::replace_tag(fmt, this->stolenTag, this->INTERNAL_STOLEN_TAG);
	RenX::replace_tag(fmt, this->accessTag, this->INTERNAL_ACCESS_TAG);

	/** Victim tags */
	RenX::replace_tag(fmt, this->victimNameTag, this->INTERNAL_VICTIM_NAME_TAG);
	RenX::replace_tag(fmt, this->victimRawNameTag, this->INTERNAL_VICTIM_RAW_NAME_TAG);
	RenX::replace_tag(fmt, this->victimIPTag, this->INTERNAL_VICTIM_IP_TAG);
	RenX::replace_tag(fmt, this->victimHWIDTag, this->INTERNAL_VICTIM_HWID_TAG);
	RenX::replace_tag(fmt, this->victimRDNSTag, this->INTERNAL_VICTIM_RDNS_TAG);
	RenX::replace_tag(fmt, this->victimSteamTag, this->INTERNAL_VICTIM_STEAM_TAG);
	RenX::replace_tag(fmt, this->victimUUIDTag, this->INTERNAL_VICTIM_UUID_TAG);
	RenX::replace_tag(fmt, this->victimIDTag, this->INTERNAL_VICTIM_ID_TAG);
	RenX::replace_tag(fmt, this->victimCharacterTag, this->INTERNAL_VICTIM_CHARACTER_TAG);
	RenX::replace_tag(fmt, this->victimVehicleTag, this->INTERNAL_VICTIM_VEHICLE_TAG);
	RenX::replace_tag(fmt, this->victimAdminTag, this->INTERNAL_VICTIM_ADMIN_TAG);
	RenX::replace_tag(fmt, this->victimPrefixTag, this->INTERNAL_VICTIM_PREFIX_TAG);
	RenX::replace_tag(fmt, this->victimGamePrefixTag, this->INTERNAL_VICTIM_GAME_PREFIX_TAG);
	RenX::replace_tag(fmt, this->victimTeamColorTag, this->INTERNAL_VICTIM_TEAM_COLOR_TAG);
	RenX::replace_tag(fmt, this->victimTeamShortTag, this->INTERNAL_VICTIM_TEAM_SHORT_TAG);
	RenX::replace_tag(fmt, this->victimTeamLongTag, this->INTERNAL_VICTIM_TEAM_LONG_TAG);
	RenX::replace_tag(fmt, this->victimPingTag, this->INTERNAL_VICTIM_PING_TAG);
	RenX::replace_tag(fmt, this->victimScoreTag, this->INTERNAL_VICTIM_SCORE_TAG);
	RenX::replace_tag(fmt, this->victimScorePerMinuteTag, this->INTERNAL_VICTIM_SCORE_PER_MINUTE_TAG);
	RenX::replace_tag(fmt, this->victimCreditsTag, this->INTERNAL_VICTIM_CREDITS_TAG);
	RenX::replace_tag(fmt, this->victimKillsTag, this->INTERNAL_VICTIM_KILLS_TAG);
	RenX::replace_tag(fmt, this->victimDeathsTag, this->INTERNAL_VICTIM_DEATHS_TAG);
	RenX::replace_tag(fmt, this->victimKDRTag, this->INTERNAL_VICTIM_KDR_TAG);
	RenX::replace_tag(fmt, this->victimSuicidesTag, this->INTERNAL_VICTIM_SUICIDES_TAG);
	RenX::replace_tag(fmt, this->victimHeadshotsTag, this->INTERNAL_VICTIM_HEADSHOTS_TAG);
	RenX::replace_tag(fmt, this->victimHeadshotKillRatioTag, this->INTERNAL_VICTIM_HEADSHOT_KILL_RATIO_TAG);
	RenX::replace_tag(fmt, this->victimVehicleKillsTag, this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG);
	RenX::replace_tag(fmt, this->victimBuildingKillsTag, this->INTERNAL_VICTIM_BUILDING_KILLS_TAG);
	RenX::replace_tag(fmt, this->victimDefenceKillsTag, this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG);
	RenX::replace_tag(fmt, this->victimGameTimeTag, this->INTERNAL_VICTIM_GAME_TIME_TAG);
	RenX::replace_tag(fmt, this->victimGamesTag, this->INTERNAL_VICTIM_GAMES_TAG);
	RenX::replace_tag(fmt, this->victimGDIGamesTag, this->INTERNAL_VICTIM_GDI_GAMES_TAG);
	RenX::replace_tag(fmt, this->victimNodGamesTag, this->INTERNAL_VICTIM_NOD_GAMES_TAG);
	RenX::replace_tag(fmt, this->victimWinsTag, this->INTERNAL_VICTIM_WINS_TAG);
	RenX::replace_tag(fmt, this->victimGDIWinsTag, this->INTERNAL_VICTIM_GDI_WINS_TAG);
	RenX::replace_tag(fmt, this->victimNodWinsTag, this->INTERNAL_VICTIM_NOD_WINS_TAG);
	RenX::replace_tag(fmt, this->victimTiesTag, this->INTERNAL_VICTIM_TIES_TAG);
	RenX::replace_tag(fmt, this->victimLossesTag, this->INTERNAL_VICTIM_LOSSES_TAG);
	RenX::replace_tag(fmt, this->victimGDILossesTag, this->INTERNAL_VICTIM_GDI_LOSSES_TAG);
	RenX::replace_tag(fmt, this->victimNodLossesTag, this->INTERNAL_VICTIM_NOD_LOSSES_TAG);
	RenX::replace_tag(fmt, this->victimWinLossRatioTag, this->INTERNAL_VICTIM_WIN_LOSS_RATIO_TAG);
	RenX::replace_tag(fmt, this->victimGDIWinLossRatioTag, this->INTERNAL_VICTIM_GDI_WIN_LOSS_RATIO_TAG);
	RenX::replace_tag(fmt, this->victimNodWinLossRatioTag, this->INTERNAL_VICTIM_NOD_WIN_LOSS_RATIO_TAG);
	RenX::replace_tag(fmt, this->victimBeaconPlacementsTag, this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->victimBeaconDisarmsTag, this->INTERNAL_VICTIM_BEACON_DISARMS_TAG);
	RenX::replace_tag(fmt, this->victimProxyPlacementsTag, this->INTERNAL_VICTIM_PROXY_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->victimProxyDisarmsTag, this->INTERNAL_VICTIM_PROXY_DISARMS_TAG);
	RenX::replace_tag(fmt, this->victimCapturesTag, this->INTERNAL_VICTIM_CAPTURES_TAG);
	RenX::replace_tag(fmt, this->victimStealsTag, this->INTERNAL_VICTIM_STEALS_TAG);
	RenX::replace_tag(fmt, this->victimStolenTag, this->INTERNAL_VICTIM_STOLEN_TAG);
	RenX::replace_tag(fmt, this->victimAccessTag, this->INTERNAL_VICTIM_ACCESS_TAG);

	/** Building tags */
	RenX::replace_tag(fmt, this->buildingNameTag, this->INTERNAL_BUILDING_NAME_TAG);
	RenX::replace_tag(fmt, this->buildingRawNameTag, this->INTERNAL_BUILDING_RAW_NAME_TAG);
	RenX::replace_tag(fmt, this->buildingHealthTag, this->INTERNAL_BUILDING_HEALTH_TAG);
	RenX::replace_tag(fmt, this->buildingMaxHealthTag, this->INTERNAL_BUILDING_MAX_HEALTH_TAG);
	RenX::replace_tag(fmt, this->buildingHealthPercentageTag, this->INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG);
	RenX::replace_tag(fmt, this->buildingArmorTag, this->INTERNAL_BUILDING_ARMOR_TAG);
	RenX::replace_tag(fmt, this->buildingMaxArmorTag, this->INTERNAL_BUILDING_MAX_ARMOR_TAG);
	RenX::replace_tag(fmt, this->buildingArmorPercentageTag, this->INTERNAL_BUILDING_ARMOR_PERCENTAGE_TAG);
	RenX::replace_tag(fmt, this->buildingDurabilityTag, this->INTERNAL_BUILDING_DURABILITY_TAG);
	RenX::replace_tag(fmt, this->buildingMaxDurabilityTag, this->INTERNAL_BUILDING_MAX_DURABILITY_TAG);
	RenX::replace_tag(fmt, this->buildingDurabilityPercentageTag, this->INTERNAL_BUILDING_DURABILITY_PERCENTAGE_TAG);
	RenX::replace_tag(fmt, this->buildingTeamColorTag, this->INTERNAL_BUILDING_TEAM_COLOR_TAG);
	RenX::replace_tag(fmt, this->buildingTeamShortTag, this->INTERNAL_BUILDING_TEAM_SHORT_TAG);
	RenX::replace_tag(fmt, this->buildingTeamLongTag, this->INTERNAL_BUILDING_TEAM_LONG_TAG);

	/** Ladder tags */
	RenX::replace_tag(fmt, this->rankTag, this->INTERNAL_RANK_TAG);
	RenX::replace_tag(fmt, this->lastGameTag, this->INTERNAL_LAST_GAME_TAG);
	RenX::replace_tag(fmt, this->GDIScoreTag, this->INTERNAL_GDI_SCORE_TAG);
	RenX::replace_tag(fmt, this->GDISPMTag, this->INTERNAL_GDI_SPM_TAG);
	RenX::replace_tag(fmt, this->GDIGameTimeTag, this->INTERNAL_GDI_GAME_TIME_TAG);
	RenX::replace_tag(fmt, this->GDITiesTag, this->INTERNAL_GDI_TIES_TAG);
	RenX::replace_tag(fmt, this->GDIBeaconPlacementsTag, this->INTERNAL_GDI_BEACON_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->GDIBeaconDisarmsTag, this->INTERNAL_GDI_BEACON_DISARMS_TAG);
	RenX::replace_tag(fmt, this->GDIProxyPlacementsTag, this->INTERNAL_GDI_PROXY_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->GDIProxyDisarmsTag, this->INTERNAL_GDI_PROXY_DISARMS_TAG);
	RenX::replace_tag(fmt, this->GDIKillsTag, this->INTERNAL_GDI_KILLS_TAG);
	RenX::replace_tag(fmt, this->GDIDeathsTag, this->INTERNAL_GDI_DEATHS_TAG);
	RenX::replace_tag(fmt, this->GDIVehicleKillsTag, this->INTERNAL_GDI_VEHICLE_KILLS_TAG);
	RenX::replace_tag(fmt, this->GDIDefenceKillsTag, this->INTERNAL_GDI_DEFENCE_KILLS_TAG);
	RenX::replace_tag(fmt, this->GDIBuildingKillsTag, this->INTERNAL_GDI_BUILDING_KILLS_TAG);
	RenX::replace_tag(fmt, this->GDIKDRTag, this->INTERNAL_GDI_KDR_TAG);
	RenX::replace_tag(fmt, this->GDIHeadshotsTag, this->INTERNAL_GDI_HEADSHOTS_TAG);
	RenX::replace_tag(fmt, this->GDIHeadshotKillRatioTag, this->INTERNAL_GDI_HEADSHOT_KILL_RATIO_TAG);
	RenX::replace_tag(fmt, this->NodScoreTag, this->INTERNAL_NOD_SCORE_TAG);
	RenX::replace_tag(fmt, this->NodSPMTag, this->INTERNAL_NOD_SPM_TAG);
	RenX::replace_tag(fmt, this->NodGameTimeTag, this->INTERNAL_NOD_GAME_TIME_TAG);
	RenX::replace_tag(fmt, this->NodTiesTag, this->INTERNAL_NOD_TIES_TAG);
	RenX::replace_tag(fmt, this->NodBeaconPlacementsTag, this->INTERNAL_NOD_BEACON_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->NodBeaconDisarmsTag, this->INTERNAL_NOD_BEACON_DISARMS_TAG);
	RenX::replace_tag(fmt, this->NodProxyPlacementsTag, this->INTERNAL_NOD_PROXY_PLACEMENTS_TAG);
	RenX::replace_tag(fmt, this->NodProxyDisarmsTag, this->INTERNAL_NOD_PROXY_DISARMS_TAG);
	RenX::replace_tag(fmt, this->NodKillsTag, this->INTERNAL_NOD_KILLS_TAG);
	RenX::replace_tag(fmt, this->NodDeathsTag, this->INTERNAL_NOD_DEATHS_TAG);
	RenX::replace_tag(fmt, this->NodVehicleKillsTag, this->INTERNAL_NOD_VEHICLE_KILLS_TAG);
	RenX::replace_tag(fmt, this->NodDefenceKillsTag, this->INTERNAL_NOD_DEFENCE_KILLS_TAG);
	RenX::replace_tag(fmt, this->NodBuildingKillsTag, this->INTERNAL_NOD_BUILDING_KILLS_TAG);
	RenX::replace_tag(fmt, this->NodKDRTag, this->INTERNAL_NOD_KDR_TAG);
	RenX::replace_tag(fmt, this->NodHeadshotsTag, this->INTERNAL_NOD_HEADSHOTS_TAG);
	RenX::replace_tag(fmt, this->NodHeadshotKillRatioTag, this->INTERNAL_NOD_HEADSHOT_KILL_RATIO_TAG);

	/** Other tags */
	RenX::replace_tag(fmt, this->weaponTag, this->INTERNAL_WEAPON_TAG);
	RenX::replace_tag(fmt, this->objectTag, this->INTERNAL_OBJECT_TAG);
	RenX::replace_tag(fmt, this->messageTag, this->INTERNAL_MESSAGE_TAG);
	RenX::replace_tag(fmt, this->newNameTag, this->INTERNAL_NEW_NAME_TAG);
	RenX::replace_tag(fmt, this->winScoreTag, this->INTERNAL_WIN_SCORE_TAG);
	RenX::replace_tag(fmt, this->loseScoreTag, this->INTERNAL_LOSE_SCORE_TAG);

	for (const auto& plugin : RenX::getCore()->getPlugins()) {
		plugin->RenX_SanitizeTags(fmt);
	}
}

std::string_view TagsImp::getUniqueInternalTag()
{
	this->uniqueTag.set(1, reinterpret_cast<const char *>(&this->tagItr), sizeof(TagsImp::tagItr));
	++TagsImp::tagItr;
	return this->uniqueTag;
}

void RenX::replace_tag(std::string& fmt, std::string_view tag, std::string_view internal_tag) {
	// Search for tag
	size_t tag_pos = fmt.find(tag);
	if (tag_pos == std::string::npos) {
		return;
	}

	// Replace the tag
	fmt.replace(tag_pos, tag.size(), internal_tag);

	// Recurse for any further instances; less efficient than old replace() but should be rare anyways
	replace_tag(fmt, tag, internal_tag);
}

/** Forward functions */

std::string_view RenX::getUniqueInternalTag()
{
	return _tags.getUniqueInternalTag();
}

void RenX::processTags(std::string& msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building)
{
	_tags.processTags(msg, server, player, victim, building);
}

void RenX::processTags(std::string& msg, const RenX::LadderDatabase::Entry &entry)
{
	_tags.processTags(msg, entry);
}

void RenX::sanitizeTags(std::string& fmt)
{
	_tags.sanitizeTags(fmt);
}