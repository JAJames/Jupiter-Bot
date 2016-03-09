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

#include "Jupiter/Reference_String.h"
#include "Jupiter/INIFile.h"
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
	TagsImp();
	void processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building);
	void processTags(Jupiter::StringType &msg, const RenX::LadderDatabase::Entry &entry);
	void sanitizeTags(Jupiter::StringType &fmt);
	const Jupiter::ReadableString &getUniqueInternalTag();
	Jupiter::StringS get_building_health_bar(const RenX::BuildingInfo *building);
private:
	Jupiter::StringS uniqueTag;
	union
	{
		uint32_t tagItr;
		struct
		{
			uint8_t tagItrP1;
			uint8_t tagItrP2;
			uint8_t tagItrP3;
			uint8_t tagItrP4;
		};
	};
	size_t bar_width;
} _tags;
RenX::Tags *RenX::tags = &_tags;

TagsImp::TagsImp()
{
	this->tagItr = 0;
	this->uniqueTag = "\0\0\0\0\0\0"_jrs;

	const Jupiter::ReadableString &configSection = Jupiter::IRC::Client::Config->get("RenX"_jrs, "TagDefinitions"_jrs, "RenX.Tags"_jrs);
	TagsImp::bar_width = Jupiter::IRC::Client::Config->getInt(configSection, "BarWidth"_jrs, 19);

	/** Global formats */
	this->dateFmt = Jupiter::IRC::Client::Config->get(configSection, "DateFormat"_jrs, "%A, %B %d, %Y"_jrs);
	this->timeFmt = Jupiter::IRC::Client::Config->get(configSection, "TimeFormat"_jrs, "%H:%M:%S"_jrs);;

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
	this->INTERNAL_BUILDING_HEALTH_BAR_TAG = this->getUniqueInternalTag();
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
	this->dateTag = Jupiter::IRC::Client::Config->get(configSection, "DateTag"_jrs, "{DATE}"_jrs);
	this->timeTag = Jupiter::IRC::Client::Config->get(configSection, "TimeTag"_jrs, "{TIME}"_jrs);

	/** Server tags */
	this->rconVersionTag = Jupiter::IRC::Client::Config->get(configSection, "RCONVersionTag"_jrs, "{RVER}"_jrs);
	this->gameVersionTag = Jupiter::IRC::Client::Config->get(configSection, "GameVersionTag"_jrs, "{GVER}"_jrs);
	this->rulesTag = Jupiter::IRC::Client::Config->get(configSection, "RulesTag"_jrs, "{RULES}"_jrs);
	this->userTag = Jupiter::IRC::Client::Config->get(configSection, "UserTag"_jrs, "{USER}"_jrs);
	this->serverNameTag = Jupiter::IRC::Client::Config->get(configSection, "ServerNameTag"_jrs, "{SERVERNAME}"_jrs);
	this->mapTag = Jupiter::IRC::Client::Config->get(configSection, "MapTag"_jrs, "{MAP}"_jrs);
	this->mapGUIDTag = Jupiter::IRC::Client::Config->get(configSection, "MapGUIDTag"_jrs, "{MGUID}"_jrs);
	this->serverHostnameTag = Jupiter::IRC::Client::Config->get(configSection, "ServerHostnameTag"_jrs, "{SERVERHOST}"_jrs);
	this->serverPortTag = Jupiter::IRC::Client::Config->get(configSection, "ServerPortTag"_jrs, "{SERVERPORT}"_jrs);
	this->socketHostnameTag = Jupiter::IRC::Client::Config->get(configSection, "SocketHostnameTag"_jrs, "{SOCKHOST}"_jrs);
	this->socketPortTag = Jupiter::IRC::Client::Config->get(configSection, "SocketPortTag"_jrs, "{SOCKPORT}"_jrs);
	this->serverPrefixTag = Jupiter::IRC::Client::Config->get(configSection, "ServerPrefixTag"_jrs, "{SERVERPREFIX}"_jrs);

	/** Player tags */
	this->nameTag = Jupiter::IRC::Client::Config->get(configSection, "NameTag"_jrs, "{NAME}"_jrs);
	this->rawNameTag = Jupiter::IRC::Client::Config->get(configSection, "RawNameTag"_jrs, "{RNAME}"_jrs);
	this->ipTag = Jupiter::IRC::Client::Config->get(configSection, "IPTag"_jrs, "{IP}"_jrs);
	this->rdnsTag = Jupiter::IRC::Client::Config->get(configSection, "RDNSTag"_jrs, "{RDNS}"_jrs);
	this->steamTag = Jupiter::IRC::Client::Config->get(configSection, "SteamTag"_jrs, "{STEAM}"_jrs);
	this->uuidTag = Jupiter::IRC::Client::Config->get(configSection, "UUIDTag"_jrs, "{UUID}"_jrs);
	this->idTag = Jupiter::IRC::Client::Config->get(configSection, "IDTag"_jrs, "{ID}"_jrs);
	this->characterTag = Jupiter::IRC::Client::Config->get(configSection, "CharacterTag"_jrs, "{CHAR}"_jrs);
	this->vehicleTag = Jupiter::IRC::Client::Config->get(configSection, "VehicleTag"_jrs, "{VEH}"_jrs);
	this->adminTag = Jupiter::IRC::Client::Config->get(configSection, "AdminTag"_jrs, "{ADMIN}"_jrs);
	this->prefixTag = Jupiter::IRC::Client::Config->get(configSection, "PrefixTag"_jrs, "{PREFIX}"_jrs);
	this->gamePrefixTag = Jupiter::IRC::Client::Config->get(configSection, "GamePrefixTag"_jrs, "{GPREFIX}"_jrs);
	this->teamColorTag = Jupiter::IRC::Client::Config->get(configSection, "TeamColorTag"_jrs, "{TCOLOR}"_jrs);
	this->teamShortTag = Jupiter::IRC::Client::Config->get(configSection, "ShortTeamTag"_jrs, "{TEAMS}"_jrs);
	this->teamLongTag = Jupiter::IRC::Client::Config->get(configSection, "LongTeamTag"_jrs, "{TEAML}"_jrs);
	this->pingTag = Jupiter::IRC::Client::Config->get(configSection, "PingTag"_jrs, "{PING}"_jrs);
	this->scoreTag = Jupiter::IRC::Client::Config->get(configSection, "ScoreTag"_jrs, "{SCORE}"_jrs);
	this->scorePerMinuteTag = Jupiter::IRC::Client::Config->get(configSection, "ScorePerMinuteTag"_jrs, "{SPM}"_jrs);
	this->creditsTag = Jupiter::IRC::Client::Config->get(configSection, "CreditsTag"_jrs, "{CREDITS}"_jrs);
	this->killsTag = Jupiter::IRC::Client::Config->get(configSection, "KillsTag"_jrs, "{KILLS}"_jrs);
	this->deathsTag = Jupiter::IRC::Client::Config->get(configSection, "DeathsTag"_jrs, "{DEATHS}"_jrs);
	this->kdrTag = Jupiter::IRC::Client::Config->get(configSection, "KDRTag"_jrs, "{KDR}"_jrs);
	this->suicidesTag = Jupiter::IRC::Client::Config->get(configSection, "SuicidesTag"_jrs, "{SUICIDES}"_jrs);
	this->headshotsTag = Jupiter::IRC::Client::Config->get(configSection, "HeadshotsTag"_jrs, "{HEADSHOTS}"_jrs);
	this->headshotKillRatioTag = Jupiter::IRC::Client::Config->get(configSection, "HeadshotKillRatioTag"_jrs, "{HSKR}"_jrs);
	this->vehicleKillsTag = Jupiter::IRC::Client::Config->get(configSection, "VehicleKillsTag"_jrs, "{VEHICLEKILLS}"_jrs);
	this->buildingKillsTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingKillsTag"_jrs, "{BUILDINGKILLS}"_jrs);
	this->defenceKillsTag = Jupiter::IRC::Client::Config->get(configSection, "DefenceKillsTag"_jrs, "{DEFENCEKILLS}"_jrs);
	this->gameTimeTag = Jupiter::IRC::Client::Config->get(configSection, "GameTimeTag"_jrs, "{GAMETIME}"_jrs);
	this->gamesTag = Jupiter::IRC::Client::Config->get(configSection, "GamesTag"_jrs, "{GAMES}"_jrs);
	this->GDIGamesTag = Jupiter::IRC::Client::Config->get(configSection, "GDIGamesTag"_jrs, "{GDIGAMES}"_jrs);
	this->NodGamesTag = Jupiter::IRC::Client::Config->get(configSection, "NodGamesTag"_jrs, "{NODGAMES}"_jrs);
	this->winsTag = Jupiter::IRC::Client::Config->get(configSection, "WinsTag"_jrs, "{WINS}"_jrs);
	this->GDIWinsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIWinsTag"_jrs, "{GDIWINS}"_jrs);
	this->NodWinsTag = Jupiter::IRC::Client::Config->get(configSection, "NodWinsTag"_jrs, "{NODWINS}"_jrs);
	this->tiesTag = Jupiter::IRC::Client::Config->get(configSection, "TiesTag"_jrs, "{TIES}"_jrs);
	this->lossesTag = Jupiter::IRC::Client::Config->get(configSection, "LossesTag"_jrs, "{LOSSES}"_jrs);
	this->GDILossesTag = Jupiter::IRC::Client::Config->get(configSection, "GDILossesTag"_jrs, "{GDILOSSES}"_jrs);
	this->NodLossesTag = Jupiter::IRC::Client::Config->get(configSection, "NodLossesTag"_jrs, "{NODLOSSES}"_jrs);
	this->winLossRatioTag = Jupiter::IRC::Client::Config->get(configSection, "WinLossRatioTag"_jrs, "{WLR}"_jrs);
	this->GDIWinLossRatioTag = Jupiter::IRC::Client::Config->get(configSection, "GDIWinLossRatioTag"_jrs, "{GDIWLR}"_jrs);
	this->NodWinLossRatioTag = Jupiter::IRC::Client::Config->get(configSection, "NodWinLossRatioTag"_jrs, "{NODWLR}"_jrs);
	this->beaconPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "BeaconPlacementsTag"_jrs, "{BEACONPLACEMENTS}"_jrs);
	this->beaconDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "BeaconDisarmsTag"_jrs, "{BEACONDISARMS}"_jrs);
	this->proxyPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "ProxyPlacementsTag"_jrs, "{PROXYPLACEMENTS}"_jrs);
	this->proxyDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "ProxyDisarmsTag"_jrs, "{PROXYDISARMS}"_jrs);
	this->capturesTag = Jupiter::IRC::Client::Config->get(configSection, "CapturesTag"_jrs, "{CAPTURES}"_jrs);
	this->stealsTag = Jupiter::IRC::Client::Config->get(configSection, "StealsTag"_jrs, "{STEALS}"_jrs);
	this->stolenTag = Jupiter::IRC::Client::Config->get(configSection, "StolenTag"_jrs, "{STOLEN}"_jrs);
	this->accessTag = Jupiter::IRC::Client::Config->get(configSection, "AccessTag"_jrs, "{ACCESS}"_jrs);

	/** Victim player tags */
	this->victimNameTag = Jupiter::IRC::Client::Config->get(configSection, "VictimNameTag"_jrs, "{VNAME}"_jrs);
	this->victimRawNameTag = Jupiter::IRC::Client::Config->get(configSection, "VictimRawNameTag"_jrs, "{VRNAME}"_jrs);
	this->victimIPTag = Jupiter::IRC::Client::Config->get(configSection, "VictimIPTag"_jrs, "{VIP}"_jrs);
	this->victimRDNSTag = Jupiter::IRC::Client::Config->get(configSection, "VictimRDNSTag"_jrs, "{VRDNS}"_jrs);
	this->victimSteamTag = Jupiter::IRC::Client::Config->get(configSection, "VictimSteamTag"_jrs, "{VSTEAM}"_jrs);
	this->victimUUIDTag = Jupiter::IRC::Client::Config->get(configSection, "VictimUUIDTag"_jrs, "{VUUID}"_jrs);
	this->victimIDTag = Jupiter::IRC::Client::Config->get(configSection, "VictimIDTag"_jrs, "{VID}"_jrs);
	this->victimCharacterTag = Jupiter::IRC::Client::Config->get(configSection, "VictimCharacterTag"_jrs, "{VCHAR}"_jrs);
	this->victimVehicleTag = Jupiter::IRC::Client::Config->get(configSection, "VictimVehicleTag"_jrs, "{VVEH}"_jrs);
	this->victimAdminTag = Jupiter::IRC::Client::Config->get(configSection, "VictimAdminTag"_jrs, "{VADMIN}"_jrs);
	this->victimPrefixTag = Jupiter::IRC::Client::Config->get(configSection, "VictimPrefixTag"_jrs, "{VPREFIX}"_jrs);
	this->victimGamePrefixTag = Jupiter::IRC::Client::Config->get(configSection, "VictimGamePrefixTag"_jrs, "{VGPREFIX}"_jrs);
	this->victimTeamColorTag = Jupiter::IRC::Client::Config->get(configSection, "VictimTeamColorTag"_jrs, "{VTCOLOR}"_jrs);
	this->victimTeamShortTag = Jupiter::IRC::Client::Config->get(configSection, "VictimShortTeamTag"_jrs, "{VTEAMS}"_jrs);
	this->victimTeamLongTag = Jupiter::IRC::Client::Config->get(configSection, "VictimLongTeamTag"_jrs, "{VTEAML}"_jrs);
	this->victimPingTag = Jupiter::IRC::Client::Config->get(configSection, "VictimPingTag"_jrs, "{VPING}"_jrs);
	this->victimScoreTag = Jupiter::IRC::Client::Config->get(configSection, "VictimScoreTag"_jrs, "{VSCORE}"_jrs);
	this->victimScorePerMinuteTag = Jupiter::IRC::Client::Config->get(configSection, "VictimScorePerMinuteTag"_jrs, "{VSPM}"_jrs);
	this->victimCreditsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimCreditsTag"_jrs, "{VCREDITS}"_jrs);
	this->victimKillsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimKillsTag"_jrs, "{VKILLS}"_jrs);
	this->victimDeathsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimDeathsTag"_jrs, "{VDEATHS}"_jrs);
	this->victimKDRTag = Jupiter::IRC::Client::Config->get(configSection, "VictimKDRTag"_jrs, "{VKDR}"_jrs);
	this->victimSuicidesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimSuicidesTag"_jrs, "{VSUICIDES}"_jrs);
	this->victimHeadshotsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimHeadshotsTag"_jrs, "{VHEADSHOTS}"_jrs);
	this->victimHeadshotKillRatioTag = Jupiter::IRC::Client::Config->get(configSection, "VictimHeadshotKillRatioTag"_jrs, "{VHSKR}"_jrs);
	this->victimVehicleKillsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimVehicleKillsTag"_jrs, "{VVEHICLEKILLS}"_jrs);
	this->victimBuildingKillsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimBuildingKillsTag"_jrs, "{VBUILDINGKILLS}"_jrs);
	this->victimDefenceKillsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimDefenceKillsTag"_jrs, "{VDEFENCEKILLS}"_jrs);
	this->victimGameTimeTag = Jupiter::IRC::Client::Config->get(configSection, "VictimGameTimeTag"_jrs, "{VGAMETIME}"_jrs);
	this->victimGamesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimGamesTag"_jrs, "{VGAMES}"_jrs);
	this->victimGDIGamesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimGDIGamesTag"_jrs, "{VGDIGAMES}"_jrs);
	this->victimNodGamesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimNodGamesTag"_jrs, "{VNODGAMES}"_jrs);
	this->victimWinsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimWinsTag"_jrs, "{VWINS}"_jrs);
	this->victimGDIWinsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimGDIWinsTag"_jrs, "{VGDIWINS}"_jrs);
	this->victimNodWinsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimNodWinsTag"_jrs, "{VNODWINS}"_jrs);
	this->victimTiesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimTiesTag"_jrs, "{VTIES}"_jrs);
	this->victimLossesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimLossesTag"_jrs, "{VLOSSES}"_jrs);
	this->victimGDILossesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimGDILossesTag"_jrs, "{VGDILOSSES}"_jrs);
	this->victimNodLossesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimNodLossesTag"_jrs, "{VNODLOSSES}"_jrs);
	this->victimWinLossRatioTag = Jupiter::IRC::Client::Config->get(configSection, "WinLossRatioTag"_jrs, "{WLR}"_jrs);
	this->victimGDIWinLossRatioTag = Jupiter::IRC::Client::Config->get(configSection, "GDIWinLossRatioTag"_jrs, "{VGDIWLR}"_jrs);
	this->victimNodWinLossRatioTag = Jupiter::IRC::Client::Config->get(configSection, "NodWinLossRatioTag"_jrs, "{VNODWLR}"_jrs);
	this->victimBeaconPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimBeaconPlacementsTag"_jrs, "{VBEACONPLACEMENTS}"_jrs);
	this->victimBeaconDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimBeaconDisarmsTag"_jrs, "{VBEACONDISARMS}"_jrs);
	this->victimProxyPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimProxyPlacementsTag"_jrs, "{VPROXYPLACEMENTS}"_jrs);
	this->victimProxyDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimProxyDisarmsTag"_jrs, "{VPROXYDISARMS}"_jrs);
	this->victimCapturesTag = Jupiter::IRC::Client::Config->get(configSection, "VictimCapturesTag"_jrs, "{VCAPTURES}"_jrs);
	this->victimStealsTag = Jupiter::IRC::Client::Config->get(configSection, "VictimStealsTag"_jrs, "{VSTEALS}"_jrs);
	this->victimStolenTag = Jupiter::IRC::Client::Config->get(configSection, "VictimStolenTag"_jrs, "{VSTOLEN}"_jrs);
	this->victimAccessTag = Jupiter::IRC::Client::Config->get(configSection, "VictimAccessTag"_jrs, "{VACCESS}"_jrs);

	/** Building tags */
	this->buildingNameTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingNameTag"_jrs, "{BNAME}"_jrs);
	this->buildingRawNameTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingRawNameTag"_jrs, "{BRNAME}"_jrs);
	this->buildingHealthTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingHealthTag"_jrs, "{BHEALTH}"_jrs);
	this->buildingMaxHealthTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingMaxHealthTag"_jrs, "{BMHEALTH}"_jrs);
	this->buildingHealthPercentageTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingHealthPercentageTag"_jrs, "{BHP}"_jrs);
	this->buildingHealthBarTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingHealthBarTag"_jrs, "{BHBAR}"_jrs);
	this->buildingTeamColorTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingTeamColorTag"_jrs, "{BCOLOR}"_jrs);
	this->buildingTeamShortTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingShortTeamTag"_jrs, "{BTEAMS}"_jrs);
	this->buildingTeamLongTag = Jupiter::IRC::Client::Config->get(configSection, "BuildingLongTeamTag"_jrs, "{BTEAML}"_jrs);

	/** Ladder tags */
	this->rankTag = Jupiter::IRC::Client::Config->get(configSection, "RankTag"_jrs, "{RANK}"_jrs);
	this->lastGameTag = Jupiter::IRC::Client::Config->get(configSection, "LastGameTag"_jrs, "{LASTGAME}"_jrs);
	this->GDIScoreTag = Jupiter::IRC::Client::Config->get(configSection, "GDIScoreTag"_jrs, "{GDISCORE}"_jrs);
	this->GDISPMTag = Jupiter::IRC::Client::Config->get(configSection, "GDISPMTag"_jrs, "{GDISPM}"_jrs);
	this->GDIGameTimeTag = Jupiter::IRC::Client::Config->get(configSection, "GDIGameTimeTag"_jrs, "{GDIGAMETIME}"_jrs);
	this->GDITiesTag = Jupiter::IRC::Client::Config->get(configSection, "GDITiesTag"_jrs, "{GDITIES}"_jrs);
	this->GDIBeaconPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIBeaconPlacementsTag"_jrs, "{GDIBEACONPLACEMENTS}"_jrs);
	this->GDIBeaconDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIBeaconDisarmsTag"_jrs, "{GDIBEACONDISARMS}"_jrs);
	this->GDIProxyPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIProxyPlacementsTag"_jrs, "{GDIPROXYPLACEMENTS}"_jrs);
	this->GDIProxyDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIProxyDisarmsTag"_jrs, "{GDIPROXYDISARMS}"_jrs);
	this->GDIKillsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIKillsTag"_jrs, "{GDIKILLS}"_jrs);
	this->GDIDeathsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIDeathsTag"_jrs, "{GDIDEATHS}"_jrs);
	this->GDIVehicleKillsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIVehicleKillsTag"_jrs, "{GDIVEHICLEKILLS}"_jrs);
	this->GDIDefenceKillsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIDefenceKillsTag"_jrs, "{GDIDEFENCEKILLS}"_jrs);
	this->GDIBuildingKillsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIBuildingKillsTag"_jrs, "{GDIBUILDINGKILLS}"_jrs);
	this->GDIKDRTag = Jupiter::IRC::Client::Config->get(configSection, "GDIKDRTag"_jrs, "{GDIKDR}"_jrs);
	this->GDIHeadshotsTag = Jupiter::IRC::Client::Config->get(configSection, "GDIHeadshotsTag"_jrs, "{GDIHEADSHOTS}"_jrs);
	this->GDIHeadshotKillRatioTag = Jupiter::IRC::Client::Config->get(configSection, "GDIHeadshotKillRatioTag"_jrs, "{GDIHSKR}"_jrs);
	this->NodScoreTag = Jupiter::IRC::Client::Config->get(configSection, "NodScoreTag"_jrs, "{NODSCORE}"_jrs);
	this->NodSPMTag = Jupiter::IRC::Client::Config->get(configSection, "NodSPMTag"_jrs, "{NODSPM}"_jrs);
	this->NodGameTimeTag = Jupiter::IRC::Client::Config->get(configSection, "NodGameTimeTag"_jrs, "{NODGAMETIME}"_jrs);
	this->NodTiesTag = Jupiter::IRC::Client::Config->get(configSection, "NodTiesTag"_jrs, "{NODTIES}"_jrs);
	this->NodBeaconPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "NodBeaconPlacementsTag"_jrs, "{NODBEACONPLACEMENTS}"_jrs);
	this->NodBeaconDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "NodBeaconDisarmsTag"_jrs, "{NODBEACONDISARMS}"_jrs);
	this->NodProxyPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, "NodProxyPlacementsTag"_jrs, "{NODPROXYPLACEMENTS}"_jrs);
	this->NodProxyDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, "NodProxyDisarmsTag"_jrs, "{NODPROXYDISARMS}"_jrs);
	this->NodKillsTag = Jupiter::IRC::Client::Config->get(configSection, "NodKillsTag"_jrs, "{NODKILLS}"_jrs);
	this->NodDeathsTag = Jupiter::IRC::Client::Config->get(configSection, "NodDeathsTag"_jrs, "{NODDEATHS}"_jrs);
	this->NodVehicleKillsTag = Jupiter::IRC::Client::Config->get(configSection, "NodVehicleKillsTag"_jrs, "{NODVEHICLEKILLS}"_jrs);
	this->NodDefenceKillsTag = Jupiter::IRC::Client::Config->get(configSection, "NodDefenceKillsTag"_jrs, "{NODDEFENCEKILLS}"_jrs);
	this->NodBuildingKillsTag = Jupiter::IRC::Client::Config->get(configSection, "NodBuildingKillsTag"_jrs, "{NODBUILDINGKILLS}"_jrs);
	this->NodKDRTag = Jupiter::IRC::Client::Config->get(configSection, "NodKDRTag"_jrs, "{NODKDR}"_jrs);
	this->NodHeadshotsTag = Jupiter::IRC::Client::Config->get(configSection, "NodHeadshotsTag"_jrs, "{NODHEADSHOTS}"_jrs);
	this->NodHeadshotKillRatioTag = Jupiter::IRC::Client::Config->get(configSection, "NodHeadshotKillRatioTag"_jrs, "{NODHSKR}"_jrs);

	/** Other tags */
	this->weaponTag = Jupiter::IRC::Client::Config->get(configSection, "WeaponTag"_jrs, "{WEAPON}"_jrs);
	this->objectTag = Jupiter::IRC::Client::Config->get(configSection, "ObjectTag"_jrs, "{OBJECT}"_jrs);
	this->messageTag = Jupiter::IRC::Client::Config->get(configSection, "MessageTag"_jrs, "{MESSAGE}"_jrs);
	this->newNameTag = Jupiter::IRC::Client::Config->get(configSection, "NewNameTag"_jrs, "{NNAME}"_jrs);
	this->winScoreTag = Jupiter::IRC::Client::Config->get(configSection, "WinScoreTag"_jrs, "{WINSCORE}"_jrs);
	this->loseScoreTag = Jupiter::IRC::Client::Config->get(configSection, "LoseScoreTag"_jrs, "{LOSESCORE}"_jrs);
}

Jupiter::StringS TagsImp::get_building_health_bar(const RenX::BuildingInfo *building)
{
	if (TagsImp::bar_width == 0)
		return Jupiter::StringS::empty;

	size_t index = 0;
	size_t greenBars = static_cast<size_t>((building->health / building->max_health) * TagsImp::bar_width);
	Jupiter::String r(TagsImp::bar_width);
	if (greenBars != 0)
	{
		r = IRCCOLOR "02,09";
		do
		{
			if (index % 2 == 0)
				r += '/';
			else
				r += ' ';
		}
		while (++index != greenBars);
		if (index == TagsImp::bar_width)
			return r += IRCNORMAL;
	}
	r += IRCCOLOR "02,04";
	do
	{
		if (index % 2 == 0)
			r += '\\';
		else
			r += ' ';
	}
	while (++index != TagsImp::bar_width);
	return r += IRCNORMAL;
}

double get_ratio(double num, double denom)
{
	if (denom == 0.0f)
		return num;
	return num / denom;
}

void TagsImp::processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building)
{
	size_t index;
	PROCESS_TAG(this->INTERNAL_DATE_TAG, Jupiter::ReferenceString(getTimeFormat(this->dateFmt.c_str())));
	PROCESS_TAG(this->INTERNAL_TIME_TAG, Jupiter::ReferenceString(getTimeFormat(this->timeFmt.c_str())));
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
			PROCESS_TAG(this->INTERNAL_STEAM_TAG, server->formatSteamID(player));
		}
		if (victim != nullptr)
		{
			PROCESS_TAG(this->INTERNAL_VICTIM_STEAM_TAG, server->formatSteamID(victim));
		}
	}
	if (player != nullptr)
	{
		PROCESS_TAG(this->INTERNAL_NAME_TAG, RenX::getFormattedPlayerName(player));
		PROCESS_TAG(this->INTERNAL_RAW_NAME_TAG, player->name);
		PROCESS_TAG(this->INTERNAL_IP_TAG, player->ip);
		if (player->rdns_thread.joinable())
		{
			PROCESS_TAG(this->INTERNAL_RDNS_TAG, RenX::rdns_pending);
		}
		else
		{
			PROCESS_TAG(this->INTERNAL_RDNS_TAG, player->rdns);
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
		PROCESS_TAG(this->INTERNAL_VICTIM_NAME_TAG, RenX::getFormattedPlayerName(victim));
		PROCESS_TAG(this->INTERNAL_VICTIM_RAW_NAME_TAG, victim->name);
		PROCESS_TAG(this->INTERNAL_VICTIM_IP_TAG, victim->ip);
		if (victim->rdns_thread.joinable())
		{
			PROCESS_TAG(this->INTERNAL_VICTIM_RDNS_TAG, RenX::rdns_pending);
		}
		else
		{
			PROCESS_TAG(this->INTERNAL_VICTIM_RDNS_TAG, victim->rdns);
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
		PROCESS_TAG(this->INTERNAL_BUILDING_HEALTH_TAG, Jupiter::StringS::Format("%.0f", building->health));
		PROCESS_TAG(this->INTERNAL_BUILDING_MAX_HEALTH_TAG, Jupiter::StringS::Format("%.0f", building->health));
		PROCESS_TAG(this->INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG, Jupiter::StringS::Format("%.0f", (building->health / building->max_health) * 100.0));
		PROCESS_TAG(this->INTERNAL_BUILDING_HEALTH_BAR_TAG, get_building_health_bar(building));
		PROCESS_TAG(this->INTERNAL_BUILDING_TEAM_COLOR_TAG, RenX::getTeamColor(building->team));
		PROCESS_TAG(this->INTERNAL_BUILDING_TEAM_SHORT_TAG, RenX::getTeamName(building->team));
		PROCESS_TAG(this->INTERNAL_BUILDING_TEAM_LONG_TAG, RenX::getFullTeamName(building->team));
	}

	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (index = 0; index < xPlugins.size(); ++index)
		xPlugins.get(index)->RenX_ProcessTags(msg, server, player, victim, building);
}

void TagsImp::processTags(Jupiter::StringType &msg, const RenX::LadderDatabase::Entry &entry)
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

void TagsImp::sanitizeTags(Jupiter::StringType &fmt)
{
	/** Global tags */
	fmt.replace(this->dateTag, this->INTERNAL_DATE_TAG);
	fmt.replace(this->timeTag, this->INTERNAL_TIME_TAG);

	/** Server tags */
	fmt.replace(this->rconVersionTag, this->INTERNAL_RCON_VERSION_TAG);
	fmt.replace(this->gameVersionTag, this->INTERNAL_GAME_VERSION_TAG);
	fmt.replace(this->rulesTag, this->INTERNAL_RULES_TAG);
	fmt.replace(this->userTag, this->INTERNAL_USER_TAG);
	fmt.replace(this->serverNameTag, this->INTERNAL_SERVER_NAME_TAG);
	fmt.replace(this->mapTag, this->INTERNAL_MAP_TAG);
	fmt.replace(this->mapGUIDTag, this->INTERNAL_MAP_GUID_TAG);
	fmt.replace(this->serverHostnameTag, this->INTERNAL_SERVER_HOSTNAME_TAG);
	fmt.replace(this->serverPortTag, this->INTERNAL_SERVER_PORT_TAG);
	fmt.replace(this->socketHostnameTag, this->INTERNAL_SOCKET_HOSTNAME_TAG);
	fmt.replace(this->socketPortTag, this->INTERNAL_SOCKET_PORT_TAG);
	fmt.replace(this->serverPrefixTag, this->INTERNAL_SERVER_PREFIX_TAG);

	/** Player tags */
	fmt.replace(this->nameTag, this->INTERNAL_NAME_TAG);
	fmt.replace(this->rawNameTag, this->INTERNAL_RAW_NAME_TAG);
	fmt.replace(this->ipTag, this->INTERNAL_IP_TAG);
	fmt.replace(this->rdnsTag, this->INTERNAL_RDNS_TAG);
	fmt.replace(this->steamTag, this->INTERNAL_STEAM_TAG);
	fmt.replace(this->uuidTag, this->INTERNAL_UUID_TAG);
	fmt.replace(this->idTag, this->INTERNAL_ID_TAG);
	fmt.replace(this->characterTag, this->INTERNAL_CHARACTER_TAG);
	fmt.replace(this->vehicleTag, this->INTERNAL_VEHICLE_TAG);
	fmt.replace(this->adminTag, this->INTERNAL_ADMIN_TAG);
	fmt.replace(this->prefixTag, this->INTERNAL_PREFIX_TAG);
	fmt.replace(this->gamePrefixTag, this->INTERNAL_GAME_PREFIX_TAG);
	fmt.replace(this->teamColorTag, this->INTERNAL_TEAM_COLOR_TAG);
	fmt.replace(this->teamShortTag, this->INTERNAL_TEAM_SHORT_TAG);
	fmt.replace(this->teamLongTag, this->INTERNAL_TEAM_LONG_TAG);
	fmt.replace(this->pingTag, this->INTERNAL_PING_TAG);
	fmt.replace(this->scoreTag, this->INTERNAL_SCORE_TAG);
	fmt.replace(this->scorePerMinuteTag, this->INTERNAL_SCORE_PER_MINUTE_TAG);
	fmt.replace(this->creditsTag, this->INTERNAL_CREDITS_TAG);
	fmt.replace(this->killsTag, this->INTERNAL_KILLS_TAG);
	fmt.replace(this->deathsTag, this->INTERNAL_DEATHS_TAG);
	fmt.replace(this->kdrTag, this->INTERNAL_KDR_TAG);
	fmt.replace(this->suicidesTag, this->INTERNAL_SUICIDES_TAG);
	fmt.replace(this->headshotsTag, this->INTERNAL_HEADSHOTS_TAG);
	fmt.replace(this->headshotKillRatioTag, this->INTERNAL_HEADSHOT_KILL_RATIO_TAG);
	fmt.replace(this->vehicleKillsTag, this->INTERNAL_VEHICLE_KILLS_TAG);
	fmt.replace(this->buildingKillsTag, this->INTERNAL_BUILDING_KILLS_TAG);
	fmt.replace(this->defenceKillsTag, this->INTERNAL_DEFENCE_KILLS_TAG);
	fmt.replace(this->gameTimeTag, this->INTERNAL_GAME_TIME_TAG);
	fmt.replace(this->gamesTag, this->INTERNAL_GAMES_TAG);
	fmt.replace(this->GDIGamesTag, this->INTERNAL_GDI_GAMES_TAG);
	fmt.replace(this->NodGamesTag, this->INTERNAL_NOD_GAMES_TAG);
	fmt.replace(this->winsTag, this->INTERNAL_WINS_TAG);
	fmt.replace(this->GDIWinsTag, this->INTERNAL_GDI_WINS_TAG);
	fmt.replace(this->NodWinsTag, this->INTERNAL_NOD_WINS_TAG);
	fmt.replace(this->tiesTag, this->INTERNAL_TIES_TAG);
	fmt.replace(this->lossesTag, this->INTERNAL_LOSSES_TAG);
	fmt.replace(this->GDILossesTag, this->INTERNAL_GDI_LOSSES_TAG);
	fmt.replace(this->NodLossesTag, this->INTERNAL_NOD_LOSSES_TAG);
	fmt.replace(this->winLossRatioTag, this->INTERNAL_WIN_LOSS_RATIO_TAG);
	fmt.replace(this->GDIWinLossRatioTag, this->INTERNAL_GDI_WIN_LOSS_RATIO_TAG);
	fmt.replace(this->NodWinLossRatioTag, this->INTERNAL_NOD_WIN_LOSS_RATIO_TAG);
	fmt.replace(this->beaconPlacementsTag, this->INTERNAL_BEACON_PLACEMENTS_TAG);
	fmt.replace(this->beaconDisarmsTag, this->INTERNAL_BEACON_DISARMS_TAG);
	fmt.replace(this->proxyPlacementsTag, this->INTERNAL_PROXY_PLACEMENTS_TAG);
	fmt.replace(this->proxyDisarmsTag, this->INTERNAL_PROXY_DISARMS_TAG);
	fmt.replace(this->capturesTag, this->INTERNAL_CAPTURES_TAG);
	fmt.replace(this->stealsTag, this->INTERNAL_STEALS_TAG);
	fmt.replace(this->stolenTag, this->INTERNAL_STOLEN_TAG);
	fmt.replace(this->accessTag, this->INTERNAL_ACCESS_TAG);

	/** Victim tags */
	fmt.replace(this->victimNameTag, this->INTERNAL_VICTIM_NAME_TAG);
	fmt.replace(this->victimRawNameTag, this->INTERNAL_VICTIM_RAW_NAME_TAG);
	fmt.replace(this->victimIPTag, this->INTERNAL_VICTIM_IP_TAG);
	fmt.replace(this->victimRDNSTag, this->INTERNAL_VICTIM_RDNS_TAG);
	fmt.replace(this->victimSteamTag, this->INTERNAL_VICTIM_STEAM_TAG);
	fmt.replace(this->victimUUIDTag, this->INTERNAL_VICTIM_UUID_TAG);
	fmt.replace(this->victimIDTag, this->INTERNAL_VICTIM_ID_TAG);
	fmt.replace(this->victimCharacterTag, this->INTERNAL_VICTIM_CHARACTER_TAG);
	fmt.replace(this->victimVehicleTag, this->INTERNAL_VICTIM_VEHICLE_TAG);
	fmt.replace(this->victimAdminTag, this->INTERNAL_VICTIM_ADMIN_TAG);
	fmt.replace(this->victimPrefixTag, this->INTERNAL_VICTIM_PREFIX_TAG);
	fmt.replace(this->victimGamePrefixTag, this->INTERNAL_VICTIM_GAME_PREFIX_TAG);
	fmt.replace(this->victimTeamColorTag, this->INTERNAL_VICTIM_TEAM_COLOR_TAG);
	fmt.replace(this->victimTeamShortTag, this->INTERNAL_VICTIM_TEAM_SHORT_TAG);
	fmt.replace(this->victimTeamLongTag, this->INTERNAL_VICTIM_TEAM_LONG_TAG);
	fmt.replace(this->victimPingTag, this->INTERNAL_VICTIM_PING_TAG);
	fmt.replace(this->victimScoreTag, this->INTERNAL_VICTIM_SCORE_TAG);
	fmt.replace(this->victimScorePerMinuteTag, this->INTERNAL_VICTIM_SCORE_PER_MINUTE_TAG);
	fmt.replace(this->victimCreditsTag, this->INTERNAL_VICTIM_CREDITS_TAG);
	fmt.replace(this->victimKillsTag, this->INTERNAL_VICTIM_KILLS_TAG);
	fmt.replace(this->victimDeathsTag, this->INTERNAL_VICTIM_DEATHS_TAG);
	fmt.replace(this->victimKDRTag, this->INTERNAL_VICTIM_KDR_TAG);
	fmt.replace(this->victimSuicidesTag, this->INTERNAL_VICTIM_SUICIDES_TAG);
	fmt.replace(this->victimHeadshotsTag, this->INTERNAL_VICTIM_HEADSHOTS_TAG);
	fmt.replace(this->victimHeadshotKillRatioTag, this->INTERNAL_VICTIM_HEADSHOT_KILL_RATIO_TAG);
	fmt.replace(this->victimVehicleKillsTag, this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG);
	fmt.replace(this->victimBuildingKillsTag, this->INTERNAL_VICTIM_BUILDING_KILLS_TAG);
	fmt.replace(this->victimDefenceKillsTag, this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG);
	fmt.replace(this->victimGameTimeTag, this->INTERNAL_VICTIM_GAME_TIME_TAG);
	fmt.replace(this->victimGamesTag, this->INTERNAL_VICTIM_GAMES_TAG);
	fmt.replace(this->victimGDIGamesTag, this->INTERNAL_VICTIM_GDI_GAMES_TAG);
	fmt.replace(this->victimNodGamesTag, this->INTERNAL_VICTIM_NOD_GAMES_TAG);
	fmt.replace(this->victimWinsTag, this->INTERNAL_VICTIM_WINS_TAG);
	fmt.replace(this->victimGDIWinsTag, this->INTERNAL_VICTIM_GDI_WINS_TAG);
	fmt.replace(this->victimNodWinsTag, this->INTERNAL_VICTIM_NOD_WINS_TAG);
	fmt.replace(this->victimTiesTag, this->INTERNAL_VICTIM_TIES_TAG);
	fmt.replace(this->victimLossesTag, this->INTERNAL_VICTIM_LOSSES_TAG);
	fmt.replace(this->victimGDILossesTag, this->INTERNAL_VICTIM_GDI_LOSSES_TAG);
	fmt.replace(this->victimNodLossesTag, this->INTERNAL_VICTIM_NOD_LOSSES_TAG);
	fmt.replace(this->victimWinLossRatioTag, this->INTERNAL_VICTIM_WIN_LOSS_RATIO_TAG);
	fmt.replace(this->victimGDIWinLossRatioTag, this->INTERNAL_VICTIM_GDI_WIN_LOSS_RATIO_TAG);
	fmt.replace(this->victimNodWinLossRatioTag, this->INTERNAL_VICTIM_NOD_WIN_LOSS_RATIO_TAG);
	fmt.replace(this->victimBeaconPlacementsTag, this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG);
	fmt.replace(this->victimBeaconDisarmsTag, this->INTERNAL_VICTIM_BEACON_DISARMS_TAG);
	fmt.replace(this->victimProxyPlacementsTag, this->INTERNAL_VICTIM_PROXY_PLACEMENTS_TAG);
	fmt.replace(this->victimProxyDisarmsTag, this->INTERNAL_VICTIM_PROXY_DISARMS_TAG);
	fmt.replace(this->victimCapturesTag, this->INTERNAL_VICTIM_CAPTURES_TAG);
	fmt.replace(this->victimStealsTag, this->INTERNAL_VICTIM_STEALS_TAG);
	fmt.replace(this->victimStolenTag, this->INTERNAL_VICTIM_STOLEN_TAG);
	fmt.replace(this->victimAccessTag, this->INTERNAL_VICTIM_ACCESS_TAG);

	/** Building tags */
	fmt.replace(this->buildingNameTag, this->INTERNAL_BUILDING_NAME_TAG);
	fmt.replace(this->buildingRawNameTag, this->INTERNAL_BUILDING_RAW_NAME_TAG);
	fmt.replace(this->buildingHealthTag, this->INTERNAL_BUILDING_HEALTH_TAG);
	fmt.replace(this->buildingMaxHealthTag, this->INTERNAL_BUILDING_MAX_HEALTH_TAG);
	fmt.replace(this->buildingHealthPercentageTag, this->INTERNAL_BUILDING_HEALTH_PERCENTAGE_TAG);
	fmt.replace(this->buildingHealthBarTag, this->INTERNAL_BUILDING_HEALTH_BAR_TAG);
	fmt.replace(this->buildingTeamColorTag, this->INTERNAL_BUILDING_TEAM_COLOR_TAG);
	fmt.replace(this->buildingTeamShortTag, this->INTERNAL_BUILDING_TEAM_SHORT_TAG);
	fmt.replace(this->buildingTeamLongTag, this->INTERNAL_BUILDING_TEAM_LONG_TAG);

	/** Ladder tags */
	fmt.replace(this->rankTag, this->INTERNAL_RANK_TAG);
	fmt.replace(this->lastGameTag, this->INTERNAL_LAST_GAME_TAG);
	fmt.replace(this->GDIScoreTag, this->INTERNAL_GDI_SCORE_TAG);
	fmt.replace(this->GDISPMTag, this->INTERNAL_GDI_SPM_TAG);
	fmt.replace(this->GDIGameTimeTag, this->INTERNAL_GDI_GAME_TIME_TAG);
	fmt.replace(this->GDITiesTag, this->INTERNAL_GDI_TIES_TAG);
	fmt.replace(this->GDIBeaconPlacementsTag, this->INTERNAL_GDI_BEACON_PLACEMENTS_TAG);
	fmt.replace(this->GDIBeaconDisarmsTag, this->INTERNAL_GDI_BEACON_DISARMS_TAG);
	fmt.replace(this->GDIProxyPlacementsTag, this->INTERNAL_GDI_PROXY_PLACEMENTS_TAG);
	fmt.replace(this->GDIProxyDisarmsTag, this->INTERNAL_GDI_PROXY_DISARMS_TAG);
	fmt.replace(this->GDIKillsTag, this->INTERNAL_GDI_KILLS_TAG);
	fmt.replace(this->GDIDeathsTag, this->INTERNAL_GDI_DEATHS_TAG);
	fmt.replace(this->GDIVehicleKillsTag, this->INTERNAL_GDI_VEHICLE_KILLS_TAG);
	fmt.replace(this->GDIDefenceKillsTag, this->INTERNAL_GDI_DEFENCE_KILLS_TAG);
	fmt.replace(this->GDIBuildingKillsTag, this->INTERNAL_GDI_BUILDING_KILLS_TAG);
	fmt.replace(this->GDIKDRTag, this->INTERNAL_GDI_KDR_TAG);
	fmt.replace(this->GDIHeadshotsTag, this->INTERNAL_GDI_HEADSHOTS_TAG);
	fmt.replace(this->GDIHeadshotKillRatioTag, this->INTERNAL_GDI_HEADSHOT_KILL_RATIO_TAG);
	fmt.replace(this->NodScoreTag, this->INTERNAL_NOD_SCORE_TAG);
	fmt.replace(this->NodSPMTag, this->INTERNAL_NOD_SPM_TAG);
	fmt.replace(this->NodGameTimeTag, this->INTERNAL_NOD_GAME_TIME_TAG);
	fmt.replace(this->NodTiesTag, this->INTERNAL_NOD_TIES_TAG);
	fmt.replace(this->NodBeaconPlacementsTag, this->INTERNAL_NOD_BEACON_PLACEMENTS_TAG);
	fmt.replace(this->NodBeaconDisarmsTag, this->INTERNAL_NOD_BEACON_DISARMS_TAG);
	fmt.replace(this->NodProxyPlacementsTag, this->INTERNAL_NOD_PROXY_PLACEMENTS_TAG);
	fmt.replace(this->NodProxyDisarmsTag, this->INTERNAL_NOD_PROXY_DISARMS_TAG);
	fmt.replace(this->NodKillsTag, this->INTERNAL_NOD_KILLS_TAG);
	fmt.replace(this->NodDeathsTag, this->INTERNAL_NOD_DEATHS_TAG);
	fmt.replace(this->NodVehicleKillsTag, this->INTERNAL_NOD_VEHICLE_KILLS_TAG);
	fmt.replace(this->NodDefenceKillsTag, this->INTERNAL_NOD_DEFENCE_KILLS_TAG);
	fmt.replace(this->NodBuildingKillsTag, this->INTERNAL_NOD_BUILDING_KILLS_TAG);
	fmt.replace(this->NodKDRTag, this->INTERNAL_NOD_KDR_TAG);
	fmt.replace(this->NodHeadshotsTag, this->INTERNAL_NOD_HEADSHOTS_TAG);
	fmt.replace(this->NodHeadshotKillRatioTag, this->INTERNAL_NOD_HEADSHOT_KILL_RATIO_TAG);

	/** Other tags */
	fmt.replace(this->weaponTag, this->INTERNAL_WEAPON_TAG);
	fmt.replace(this->objectTag, this->INTERNAL_OBJECT_TAG);
	fmt.replace(this->messageTag, this->INTERNAL_MESSAGE_TAG);
	fmt.replace(this->newNameTag, this->INTERNAL_NEW_NAME_TAG);
	fmt.replace(this->winScoreTag, this->INTERNAL_WIN_SCORE_TAG);
	fmt.replace(this->loseScoreTag, this->INTERNAL_LOSE_SCORE_TAG);

	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		xPlugins.get(i)->RenX_SanitizeTags(fmt);
}

const Jupiter::ReadableString &TagsImp::getUniqueInternalTag()
{
	this->uniqueTag.set(1, this->tagItrP1);
	this->uniqueTag.set(2, this->tagItrP2);
	this->uniqueTag.set(3, this->tagItrP3);
	this->uniqueTag.set(4, this->tagItrP4);
	this->tagItr++;
	return this->uniqueTag;
}

/** Foward functions */

const Jupiter::ReadableString &RenX::getUniqueInternalTag()
{
	return _tags.getUniqueInternalTag();
}

void RenX::processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building)
{
	_tags.processTags(msg, server, player, victim, building);
}

void RenX::processTags(Jupiter::StringType &msg, const RenX::LadderDatabase::Entry &entry)
{
	_tags.processTags(msg, entry);
}

void RenX::sanitizeTags(Jupiter::StringType &fmt)
{
	_tags.sanitizeTags(fmt);
}