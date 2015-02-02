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

#include "Jupiter/Reference_String.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/IRC_Client.h"
#include "RenX_Core.h"
#include "RenX_Functions.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Plugin.h"
#include "RenX_Tags.h"

struct TagsImp : RenX::Tags
{
	TagsImp();
	void processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim);
	void sanitizeTags(Jupiter::StringType &fmt);
} _tags;
RenX::Tags *RenX::tags = &_tags;

TagsImp::TagsImp()
{
	const Jupiter::ReadableString &configSection = Jupiter::IRC::Client::Config->get(RenX::getCore()->getName(), STRING_LITERAL_AS_REFERENCE("TagDefinitions"), STRING_LITERAL_AS_REFERENCE("TagDefinitions"));

	/** Global formats */
	this->dateFmt = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("DateFormat"), STRING_LITERAL_AS_REFERENCE("%A, %B %d, %Y"));
	this->timeFmt = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("TimeFormat"), STRING_LITERAL_AS_REFERENCE("%H:%M:%S"));;

	/** Internal message tags */

	/** Global tags */
	this->INTERNAL_DATE_TAG = STRING_LITERAL_AS_REFERENCE("\0DAT\0");
	this->INTERNAL_TIME_TAG = STRING_LITERAL_AS_REFERENCE("\0TIM\0");

	/** Server tags */
	this->INTERNAL_RCON_VERSION_TAG = STRING_LITERAL_AS_REFERENCE("\0RVER\0");
	this->INTERNAL_GAME_VERSION_TAG = STRING_LITERAL_AS_REFERENCE("\0GVER\0");
	this->INTERNAL_XRCON_VERSION_TAG = STRING_LITERAL_AS_REFERENCE("\0XVER\0");
	this->INTERNAL_RULES_TAG = STRING_LITERAL_AS_REFERENCE("\0RUL\0");

	/** Player tags*/
	this->INTERNAL_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0NAME\0");
	this->INTERNAL_RAW_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0RNAME\0");
	this->INTERNAL_IP_TAG = STRING_LITERAL_AS_REFERENCE("\0IP\0");
	this->INTERNAL_STEAM_TAG = STRING_LITERAL_AS_REFERENCE("\0STEAM\0");
	this->INTERNAL_UUID_TAG = STRING_LITERAL_AS_REFERENCE("\0UUID\0");
	this->INTERNAL_ID_TAG = STRING_LITERAL_AS_REFERENCE("\0ID\0");
	this->INTERNAL_CHARACTER_TAG = STRING_LITERAL_AS_REFERENCE("\0CHR\0");
	this->INTERNAL_VEHICLE_TAG = STRING_LITERAL_AS_REFERENCE("\0VEH\0");
	this->INTERNAL_ADMIN_TAG = STRING_LITERAL_AS_REFERENCE("\0ADM\0");
	this->INTERNAL_PREFIX_TAG = STRING_LITERAL_AS_REFERENCE("\0PFX\0");
	this->INTERNAL_GAME_PREFIX_TAG = STRING_LITERAL_AS_REFERENCE("\0GPF\0");
	this->INTERNAL_TEAM_COLOR_TAG = STRING_LITERAL_AS_REFERENCE("\0TC\0");
	this->INTERNAL_TEAM_SHORT_TAG = STRING_LITERAL_AS_REFERENCE("\0TS\0");
	this->INTERNAL_TEAM_LONG_TAG = STRING_LITERAL_AS_REFERENCE("\0TL\0");
	this->INTERNAL_PING_TAG = STRING_LITERAL_AS_REFERENCE("\0PNG\0");
	this->INTERNAL_SCORE_TAG = STRING_LITERAL_AS_REFERENCE("\0SCR\0");
	this->INTERNAL_CREDITS_TAG = STRING_LITERAL_AS_REFERENCE("\0CRD\0");
	this->INTERNAL_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0KIL\0");
	this->INTERNAL_DEATHS_TAG = STRING_LITERAL_AS_REFERENCE("\0DTH\0");
	this->INTERNAL_KDR_TAG = STRING_LITERAL_AS_REFERENCE("\0KDR\0");
	this->INTERNAL_SUICIDES_TAG = STRING_LITERAL_AS_REFERENCE("\0SCD\0");
	this->INTERNAL_HEADSHOTS_TAG = STRING_LITERAL_AS_REFERENCE("\0HDS\0");
	this->INTERNAL_VEHICLE_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0VKS\0");
	this->INTERNAL_BUILDING_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0BKS\0");
	this->INTERNAL_DEFENCE_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0DKS\0");
	this->INTERNAL_WINS_TAG = STRING_LITERAL_AS_REFERENCE("\0WIN\0");
	this->INTERNAL_LOSES_TAG = STRING_LITERAL_AS_REFERENCE("\0LOS\0");
	this->INTERNAL_BEACON_PLACEMENTS_TAG = STRING_LITERAL_AS_REFERENCE("\0BPC\0");
	this->INTERNAL_ACCESS_TAG = STRING_LITERAL_AS_REFERENCE("\0ACS\0");

	/** Victim tags */
	this->INTERNAL_VICTIM_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0VNAME\0");
	this->INTERNAL_VICTIM_RAW_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0VRNAME\0");
	this->INTERNAL_VICTIM_IP_TAG = STRING_LITERAL_AS_REFERENCE("\0VIP\0");
	this->INTERNAL_VICTIM_STEAM_TAG = STRING_LITERAL_AS_REFERENCE("\0VSTEAM\0");
	this->INTERNAL_VICTIM_UUID_TAG = STRING_LITERAL_AS_REFERENCE("\0VUUID\0");
	this->INTERNAL_VICTIM_ID_TAG = STRING_LITERAL_AS_REFERENCE("\0VID\0");
	this->INTERNAL_VICTIM_CHARACTER_TAG = STRING_LITERAL_AS_REFERENCE("\0VCHR\0");
	this->INTERNAL_VICTIM_VEHICLE_TAG = STRING_LITERAL_AS_REFERENCE("\0VVEH\0");
	this->INTERNAL_VICTIM_ADMIN_TAG = STRING_LITERAL_AS_REFERENCE("\0VADM\0");
	this->INTERNAL_VICTIM_PREFIX_TAG = STRING_LITERAL_AS_REFERENCE("\0VPFX\0");
	this->INTERNAL_VICTIM_GAME_PREFIX_TAG = STRING_LITERAL_AS_REFERENCE("\0VGPF\0");
	this->INTERNAL_VICTIM_TEAM_COLOR_TAG = STRING_LITERAL_AS_REFERENCE("\0VTC\0");
	this->INTERNAL_VICTIM_TEAM_SHORT_TAG = STRING_LITERAL_AS_REFERENCE("\0VTS\0");
	this->INTERNAL_VICTIM_TEAM_LONG_TAG = STRING_LITERAL_AS_REFERENCE("\0VTL\0");
	this->INTERNAL_VICTIM_PING_TAG = STRING_LITERAL_AS_REFERENCE("\0VPNG\0");
	this->INTERNAL_VICTIM_SCORE_TAG = STRING_LITERAL_AS_REFERENCE("\0VSCR\0");
	this->INTERNAL_VICTIM_CREDITS_TAG = STRING_LITERAL_AS_REFERENCE("\0VCRD\0");
	this->INTERNAL_VICTIM_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0VKIL\0");
	this->INTERNAL_VICTIM_DEATHS_TAG = STRING_LITERAL_AS_REFERENCE("\0VDTH\0");
	this->INTERNAL_VICTIM_KDR_TAG = STRING_LITERAL_AS_REFERENCE("\0VKDR\0");
	this->INTERNAL_VICTIM_SUICIDES_TAG = STRING_LITERAL_AS_REFERENCE("\0VSCD\0");
	this->INTERNAL_VICTIM_HEADSHOTS_TAG = STRING_LITERAL_AS_REFERENCE("\0VHDS\0");
	this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0VVKS\0");
	this->INTERNAL_VICTIM_BUILDING_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0VBKS\0");
	this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG = STRING_LITERAL_AS_REFERENCE("\0VDKS\0");
	this->INTERNAL_VICTIM_WINS_TAG = STRING_LITERAL_AS_REFERENCE("\0VWIN\0");
	this->INTERNAL_VICTIM_LOSES_TAG = STRING_LITERAL_AS_REFERENCE("\0VLOS\0");
	this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG = STRING_LITERAL_AS_REFERENCE("\0VBPC\0");
	this->INTERNAL_VICTIM_ACCESS_TAG = STRING_LITERAL_AS_REFERENCE("\0VACS\0");

	/** Other tags */
	this->INTERNAL_WEAPON_TAG = STRING_LITERAL_AS_REFERENCE("\0WEP\0");
	this->INTERNAL_OBJECT_TAG = STRING_LITERAL_AS_REFERENCE("\0OBJ\0");
	this->INTERNAL_MESSAGE_TAG = STRING_LITERAL_AS_REFERENCE("\0MSG\0");
	this->INTERNAL_NEW_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0NNAME\0");
	this->INTERNAL_WIN_SCORE_TAG = STRING_LITERAL_AS_REFERENCE("\0WSC\0");
	this->INTERNAL_LOSE_SCORE_TAG = STRING_LITERAL_AS_REFERENCE("\0LSC\0");

	/** External (config) tags */

	/** Global tags */
	this->dateTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("DateTag"), STRING_LITERAL_AS_REFERENCE("{DATE}"));
	this->timeTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("TimeTag"), STRING_LITERAL_AS_REFERENCE("{TIME}"));

	/** Server tags */
	this->rconVersionTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("RCONVersionTag"), STRING_LITERAL_AS_REFERENCE("{RVER}"));
	this->gameVersionTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("GameVersionTag"), STRING_LITERAL_AS_REFERENCE("{GVER}"));
	this->xRconVersionTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("XRCONVersionTag"), STRING_LITERAL_AS_REFERENCE("{XVER}"));
	this->rulesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("RulesTag"), STRING_LITERAL_AS_REFERENCE("{RULES}"));

	/** Player tags */
	this->nameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("NameTag"), STRING_LITERAL_AS_REFERENCE("{NAME}"));
	this->rawNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("RawNameTag"), STRING_LITERAL_AS_REFERENCE("{RNAME}"));
	this->ipTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("IPTag"), STRING_LITERAL_AS_REFERENCE("{IP}"));
	this->steamTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("SteamTag"), STRING_LITERAL_AS_REFERENCE("{STEAM}"));
	this->uuidTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("UUIDTag"), STRING_LITERAL_AS_REFERENCE("{UUID}"));
	this->idTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("IDTag"), STRING_LITERAL_AS_REFERENCE("{ID}"));
	this->characterTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("CharacterTag"), STRING_LITERAL_AS_REFERENCE("{CHAR}"));
	this->vehicleTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VehicleTag"), STRING_LITERAL_AS_REFERENCE("{VEH}"));
	this->adminTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("AdminTag"), STRING_LITERAL_AS_REFERENCE("{ADMIN}"));
	this->prefixTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("PrefixTag"), STRING_LITERAL_AS_REFERENCE("{PREFIX}"));
	this->gamePrefixTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("GamePrefixTag"), STRING_LITERAL_AS_REFERENCE("{GPREFIX}"));
	this->teamColorTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("TeamColorTag"), STRING_LITERAL_AS_REFERENCE("{TCOLOR}"));
	this->teamShortTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ShortTeamTag"), STRING_LITERAL_AS_REFERENCE("{TEAMS}"));
	this->teamLongTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("LongTeamTag"), STRING_LITERAL_AS_REFERENCE("{TEAML}"));
	this->pingTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("PingTag"), STRING_LITERAL_AS_REFERENCE("{PING}"));
	this->scoreTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ScoreTag"), STRING_LITERAL_AS_REFERENCE("{SCORE}"));
	this->creditsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("CreditsTag"), STRING_LITERAL_AS_REFERENCE("{CREDITS}"));
	this->killsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("KillsTag"), STRING_LITERAL_AS_REFERENCE("{KILLS}"));
	this->deathsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("DeathsTag"), STRING_LITERAL_AS_REFERENCE("{DEATHS}"));
	this->kdrTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("KDRTag"), STRING_LITERAL_AS_REFERENCE("{KDR}"));
	this->suicidesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("SuicidesTag"), STRING_LITERAL_AS_REFERENCE("{SUICIDES}"));
	this->headshotsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("HeadshotsTag"), STRING_LITERAL_AS_REFERENCE("{HEADSHOTS}"));
	this->vehicleKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VehicleKillsTag"), STRING_LITERAL_AS_REFERENCE("{VEHICLEKILLS}"));
	this->buildingKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingKillsTag"), STRING_LITERAL_AS_REFERENCE("{BUILDINGKILLS}"));
	this->defenceKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("DefenceKillsTag"), STRING_LITERAL_AS_REFERENCE("{DEFENCEKILLS}"));
	this->winsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("WinsTag"), STRING_LITERAL_AS_REFERENCE("{WINS}"));
	this->losesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("LosesTag"), STRING_LITERAL_AS_REFERENCE("{LOSES}"));
	this->beaconPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BeaconPlacementsTag"), STRING_LITERAL_AS_REFERENCE("{BEACONPLACEMENTS}"));
	this->accessTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("AccessTag"), STRING_LITERAL_AS_REFERENCE("{ACCESS}"));

	/** Victim player tags */
	this->victimNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimNameTag"), STRING_LITERAL_AS_REFERENCE("{VNAME}"));
	this->victimRawNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimRawNameTag"), STRING_LITERAL_AS_REFERENCE("{VRNAME}"));
	this->victimIPTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimIPTag"), STRING_LITERAL_AS_REFERENCE("{VIP}"));
	this->victimSteamTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimSteamTag"), STRING_LITERAL_AS_REFERENCE("{VSTEAM}"));
	this->victimUUIDTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimUUIDTag"), STRING_LITERAL_AS_REFERENCE("{VUUID}"));
	this->victimIDTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimIDTag"), STRING_LITERAL_AS_REFERENCE("{VID}"));
	this->victimCharacterTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimCharacterTag"), STRING_LITERAL_AS_REFERENCE("{VCHAR}"));
	this->victimVehicleTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimVehicleTag"), STRING_LITERAL_AS_REFERENCE("{VVEH}"));
	this->victimAdminTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimAdminTag"), STRING_LITERAL_AS_REFERENCE("{VADMIN}"));
	this->victimPrefixTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimPrefixTag"), STRING_LITERAL_AS_REFERENCE("{VPREFIX}"));
	this->victimGamePrefixTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimGamePrefixTag"), STRING_LITERAL_AS_REFERENCE("{VGPREFIX}"));
	this->victimTeamColorTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimTeamColorTag"), STRING_LITERAL_AS_REFERENCE("{VTCOLOR}"));
	this->victimTeamShortTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimShortTeamTag"), STRING_LITERAL_AS_REFERENCE("{VTEAMS}"));
	this->victimTeamLongTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimLongTeamTag"), STRING_LITERAL_AS_REFERENCE("{VTEAML}"));
	this->victimPingTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimPingTag"), STRING_LITERAL_AS_REFERENCE("{VPING}"));
	this->victimScoreTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimScoreTag"), STRING_LITERAL_AS_REFERENCE("{VSCORE}"));
	this->victimCreditsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimCreditsTag"), STRING_LITERAL_AS_REFERENCE("{VCREDITS}"));
	this->victimKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimKillsTag"), STRING_LITERAL_AS_REFERENCE("{VKILLS}"));
	this->victimDeathsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimDeathsTag"), STRING_LITERAL_AS_REFERENCE("{VDEATHS}"));
	this->victimKDRTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimKDRTag"), STRING_LITERAL_AS_REFERENCE("{VKDR}"));
	this->victimSuicidesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimSuicidesTag"), STRING_LITERAL_AS_REFERENCE("{VSUICIDES}"));
	this->victimHeadshotsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimHeadshotsTag"), STRING_LITERAL_AS_REFERENCE("{VHEADSHOTS}"));
	this->victimVehicleKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimVehicleKillsTag"), STRING_LITERAL_AS_REFERENCE("{VVEHICLEKILLS}"));
	this->victimBuildingKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimBuildingKillsTag"), STRING_LITERAL_AS_REFERENCE("{VBUILDINGKILLS}"));
	this->victimDefenceKillsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimDefenceKillsTag"), STRING_LITERAL_AS_REFERENCE("{VDEFENCEKILLS}"));
	this->victimWinsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimWinsTag"), STRING_LITERAL_AS_REFERENCE("{VWINS}"));
	this->victimLosesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimLosesTag"), STRING_LITERAL_AS_REFERENCE("{VLOSES}"));
	this->victimBeaconPlacementsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimBeaconPlacementsTag"), STRING_LITERAL_AS_REFERENCE("{VBEACONPLACEMENTS}"));
	this->victimAccessTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimAccessTag"), STRING_LITERAL_AS_REFERENCE("{VACCESS}"));

	/** Other tags */
	this->weaponTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("WeaponTag"), STRING_LITERAL_AS_REFERENCE("{WEAPON}"));
	this->objectTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ObjectTag"), STRING_LITERAL_AS_REFERENCE("{OBJECT}"));
	this->messageTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("MessageTag"), STRING_LITERAL_AS_REFERENCE("{MESSAGE}"));
	this->newNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("NewNameTag"), STRING_LITERAL_AS_REFERENCE("{NNAME}"));
	this->winScoreTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("WinScoreTag"), STRING_LITERAL_AS_REFERENCE("{WINSCORE}"));
	this->loseScoreTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("LoseScoreTag"), STRING_LITERAL_AS_REFERENCE("{LOSESCORE}"));
}

void TagsImp::processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim)
{
	msg.replace(this->INTERNAL_DATE_TAG, Jupiter::ReferenceString(getTimeFormat(this->dateFmt.c_str())));
	msg.replace(this->INTERNAL_TIME_TAG, Jupiter::ReferenceString(getTimeFormat(this->timeFmt.c_str())));
	if (server != nullptr)
	{
		msg.replace(this->INTERNAL_RCON_VERSION_TAG, Jupiter::StringS::Format("%u", server->getVersion()));
		msg.replace(this->INTERNAL_GAME_VERSION_TAG, server->getGameVersion());
		msg.replace(this->INTERNAL_XRCON_VERSION_TAG, Jupiter::StringS::Format("%u", server->getXVersion()));
		msg.replace(this->INTERNAL_RULES_TAG, server->getRules());
		if (player != nullptr)
		{
			msg.replace(this->INTERNAL_STEAM_TAG, server->formatSteamID(player));
		}
		if (victim != nullptr)
		{
			msg.replace(this->INTERNAL_VICTIM_STEAM_TAG, server->formatSteamID(victim));
		}
	}
	if (player != nullptr)
	{
		msg.replace(this->INTERNAL_NAME_TAG, RenX::getFormattedPlayerName(player));
		msg.replace(this->INTERNAL_RAW_NAME_TAG, player->name);
		msg.replace(this->INTERNAL_IP_TAG, player->ip);
		msg.replace(this->INTERNAL_UUID_TAG, player->uuid);
		msg.replace(this->INTERNAL_ID_TAG, Jupiter::StringS::Format("%d", player->id));
		msg.replace(this->INTERNAL_CHARACTER_TAG, RenX::translateName(player->character));
		msg.replace(this->INTERNAL_VEHICLE_TAG, RenX::translateName(player->vehicle));
		msg.replace(this->INTERNAL_ADMIN_TAG, player->adminType);
		msg.replace(this->INTERNAL_PREFIX_TAG, player->formatNamePrefix);
		msg.replace(this->INTERNAL_GAME_PREFIX_TAG, player->gamePrefix);
		msg.replace(this->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(player->team));
		msg.replace(this->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(player->team));
		msg.replace(this->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(player->team));
		msg.replace(this->INTERNAL_PING_TAG, Jupiter::StringS::Format("%hu", player->ping));
		msg.replace(this->INTERNAL_SCORE_TAG, Jupiter::StringS::Format("%.0f", player->score));
		msg.replace(this->INTERNAL_CREDITS_TAG, Jupiter::StringS::Format("%.0f", player->credits));
		msg.replace(this->INTERNAL_KILLS_TAG, Jupiter::StringS::Format("%u", player->kills));
		msg.replace(this->INTERNAL_DEATHS_TAG, Jupiter::StringS::Format("%u", player->deaths));
		msg.replace(this->INTERNAL_KDR_TAG, Jupiter::StringS::Format("%.2f", static_cast<float>(player->kills) / (player->deaths == 0 ? 1.0f : static_cast<float>(player->deaths))));
		msg.replace(this->INTERNAL_SUICIDES_TAG, Jupiter::StringS::Format("%u", player->suicides));
		msg.replace(this->INTERNAL_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", player->headshots));
		msg.replace(this->INTERNAL_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", player->vehicleKills));
		msg.replace(this->INTERNAL_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", player->buildingKills));
		msg.replace(this->INTERNAL_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", player->defenceKills));
		msg.replace(this->INTERNAL_WINS_TAG, Jupiter::StringS::Format("%u", player->wins));
		msg.replace(this->INTERNAL_LOSES_TAG, Jupiter::StringS::Format("%u", player->loses));
		msg.replace(this->INTERNAL_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", player->beaconPlacements));
		msg.replace(this->INTERNAL_ACCESS_TAG, Jupiter::StringS::Format("%d", player->access));
	}
	if (victim != nullptr)
	{
		msg.replace(this->INTERNAL_VICTIM_NAME_TAG, RenX::getFormattedPlayerName(victim));
		msg.replace(this->INTERNAL_VICTIM_RAW_NAME_TAG, victim->name);
		msg.replace(this->INTERNAL_VICTIM_IP_TAG, victim->ip);
		msg.replace(this->INTERNAL_VICTIM_UUID_TAG, victim->uuid);
		msg.replace(this->INTERNAL_VICTIM_ID_TAG, Jupiter::StringS::Format("%d", victim->id));
		msg.replace(this->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(victim->character));
		msg.replace(this->INTERNAL_VICTIM_VEHICLE_TAG, RenX::translateName(victim->vehicle));
		msg.replace(this->INTERNAL_VICTIM_ADMIN_TAG, victim->adminType);
		msg.replace(this->INTERNAL_VICTIM_PREFIX_TAG, victim->formatNamePrefix);
		msg.replace(this->INTERNAL_VICTIM_GAME_PREFIX_TAG, victim->gamePrefix);
		msg.replace(this->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(victim->team));
		msg.replace(this->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(victim->team));
		msg.replace(this->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(victim->team));
		msg.replace(this->INTERNAL_VICTIM_PING_TAG, Jupiter::StringS::Format("%hu", victim->ping));
		msg.replace(this->INTERNAL_VICTIM_SCORE_TAG, Jupiter::StringS::Format("%.0f", victim->score));
		msg.replace(this->INTERNAL_VICTIM_CREDITS_TAG, Jupiter::StringS::Format("%.0f", victim->credits));
		msg.replace(this->INTERNAL_VICTIM_KILLS_TAG, Jupiter::StringS::Format("%u", victim->kills));
		msg.replace(this->INTERNAL_VICTIM_DEATHS_TAG, Jupiter::StringS::Format("%u", victim->deaths));
		msg.replace(this->INTERNAL_VICTIM_KDR_TAG, Jupiter::StringS::Format("%.2f", static_cast<float>(victim->kills) / (victim->deaths == 0 ? 1.0f : static_cast<float>(victim->deaths))));
		msg.replace(this->INTERNAL_VICTIM_SUICIDES_TAG, Jupiter::StringS::Format("%u", victim->suicides));
		msg.replace(this->INTERNAL_VICTIM_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", victim->headshots));
		msg.replace(this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", victim->vehicleKills));
		msg.replace(this->INTERNAL_VICTIM_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", victim->buildingKills));
		msg.replace(this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", victim->defenceKills));
		msg.replace(this->INTERNAL_VICTIM_WINS_TAG, Jupiter::StringS::Format("%u", victim->wins));
		msg.replace(this->INTERNAL_VICTIM_LOSES_TAG, Jupiter::StringS::Format("%u", victim->loses));
		msg.replace(this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG, Jupiter::StringS::Format("%u", victim->beaconPlacements));
		msg.replace(this->INTERNAL_VICTIM_ACCESS_TAG, Jupiter::StringS::Format("%d", victim->access));
	}

	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		xPlugins.get(i)->RenX_ProcessTags(msg, server, player, victim);
}

void TagsImp::sanitizeTags(Jupiter::StringType &fmt)
{
	/** Global tags */
	fmt.replace(this->dateTag, this->INTERNAL_DATE_TAG);
	fmt.replace(this->timeTag, this->INTERNAL_TIME_TAG);

	/** Server tags */
	fmt.replace(this->rconVersionTag, this->INTERNAL_RCON_VERSION_TAG);
	fmt.replace(this->gameVersionTag, this->INTERNAL_GAME_VERSION_TAG);
	fmt.replace(this->xRconVersionTag, this->INTERNAL_XRCON_VERSION_TAG);
	fmt.replace(this->rulesTag, this->INTERNAL_RULES_TAG);

	/** Player tags */
	fmt.replace(this->nameTag, this->INTERNAL_NAME_TAG);
	fmt.replace(this->rawNameTag, this->INTERNAL_RAW_NAME_TAG);
	fmt.replace(this->ipTag, this->INTERNAL_IP_TAG);
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
	fmt.replace(this->creditsTag, this->INTERNAL_CREDITS_TAG);
	fmt.replace(this->killsTag, this->INTERNAL_KILLS_TAG);
	fmt.replace(this->deathsTag, this->INTERNAL_DEATHS_TAG);
	fmt.replace(this->kdrTag, this->INTERNAL_KDR_TAG);
	fmt.replace(this->suicidesTag, this->INTERNAL_SUICIDES_TAG);
	fmt.replace(this->headshotsTag, this->INTERNAL_HEADSHOTS_TAG);
	fmt.replace(this->vehicleKillsTag, this->INTERNAL_VEHICLE_KILLS_TAG);
	fmt.replace(this->buildingKillsTag, this->INTERNAL_BUILDING_KILLS_TAG);
	fmt.replace(this->defenceKillsTag, this->INTERNAL_DEFENCE_KILLS_TAG);
	fmt.replace(this->winsTag, this->INTERNAL_WINS_TAG);
	fmt.replace(this->losesTag, this->INTERNAL_LOSES_TAG);
	fmt.replace(this->beaconPlacementsTag, this->INTERNAL_BEACON_PLACEMENTS_TAG);
	fmt.replace(this->accessTag, this->INTERNAL_ACCESS_TAG);

	/** Victim tags */
	fmt.replace(this->victimNameTag, this->INTERNAL_VICTIM_NAME_TAG);
	fmt.replace(this->victimRawNameTag, this->INTERNAL_VICTIM_RAW_NAME_TAG);
	fmt.replace(this->victimIPTag, this->INTERNAL_VICTIM_IP_TAG);
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
	fmt.replace(this->victimCreditsTag, this->INTERNAL_VICTIM_CREDITS_TAG);
	fmt.replace(this->victimKillsTag, this->INTERNAL_VICTIM_KILLS_TAG);
	fmt.replace(this->victimDeathsTag, this->INTERNAL_VICTIM_DEATHS_TAG);
	fmt.replace(this->victimKDRTag, this->INTERNAL_VICTIM_KDR_TAG);
	fmt.replace(this->victimSuicidesTag, this->INTERNAL_VICTIM_SUICIDES_TAG);
	fmt.replace(this->victimHeadshotsTag, this->INTERNAL_VICTIM_HEADSHOTS_TAG);
	fmt.replace(this->victimVehicleKillsTag, this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG);
	fmt.replace(this->victimBuildingKillsTag, this->INTERNAL_VICTIM_BUILDING_KILLS_TAG);
	fmt.replace(this->victimDefenceKillsTag, this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG);
	fmt.replace(this->victimWinsTag, this->INTERNAL_VICTIM_WINS_TAG);
	fmt.replace(this->victimLosesTag, this->INTERNAL_VICTIM_LOSES_TAG);
	fmt.replace(this->victimBeaconPlacementsTag, this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG);
	fmt.replace(this->victimAccessTag, this->INTERNAL_VICTIM_ACCESS_TAG);

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

/** Foward functions */

void RenX::processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim)
{
	_tags.processTags(msg, server, player, victim);
}

void RenX::sanitizeTags(Jupiter::StringType &fmt)
{
	_tags.sanitizeTags(fmt);
}