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
#include "RenX_BuildingInfo.h"
#include "RenX_Plugin.h"
#include "RenX_Tags.h"

struct TagsImp : RenX::Tags
{
	TagsImp();
	void processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building);
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
	this->uniqueTag = STRING_LITERAL_AS_REFERENCE("\0\0\0\0\0\0");

	const Jupiter::ReadableString &configSection = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("TagDefinitions"), STRING_LITERAL_AS_REFERENCE("RenX.Tags"));
	TagsImp::bar_width = Jupiter::IRC::Client::Config->getInt(configSection, STRING_LITERAL_AS_REFERENCE("BarWidth"), 19);

	/** Global formats */
	this->dateFmt = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("DateFormat"), STRING_LITERAL_AS_REFERENCE("%A, %B %d, %Y"));
	this->timeFmt = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("TimeFormat"), STRING_LITERAL_AS_REFERENCE("%H:%M:%S"));;

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
	this->INTERNAL_SERVER_HOSTNAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SERVER_PORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SOCKET_HOSTNAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SOCKET_PORT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SERVER_PREFIX_TAG = this->getUniqueInternalTag();

	/** Player tags*/
	this->INTERNAL_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_RAW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_IP_TAG = this->getUniqueInternalTag();
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
	this->INTERNAL_CREDITS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_DEATHS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_KDR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_SUICIDES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_HEADSHOTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VEHICLE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BUILDING_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_DEFENCE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_LOSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BEACON_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_BEACON_DISARMS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_CAPTURES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_STEALS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_STOLEN_TAG = this->getUniqueInternalTag();
	this->INTERNAL_ACCESS_TAG = this->getUniqueInternalTag();

	/** Victim tags */
	this->INTERNAL_VICTIM_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_RAW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_IP_TAG = this->getUniqueInternalTag();
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
	this->INTERNAL_VICTIM_CREDITS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_DEATHS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_KDR_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_SUICIDES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_HEADSHOTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_BUILDING_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_WINS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_LOSES_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_BEACON_PLACEMENTS_TAG = this->getUniqueInternalTag();
	this->INTERNAL_VICTIM_BEACON_DISARMS_TAG = this->getUniqueInternalTag();
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

	/** Other tags */
	this->INTERNAL_WEAPON_TAG = this->getUniqueInternalTag();
	this->INTERNAL_OBJECT_TAG = this->getUniqueInternalTag();
	this->INTERNAL_MESSAGE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_NEW_NAME_TAG = this->getUniqueInternalTag();
	this->INTERNAL_WIN_SCORE_TAG = this->getUniqueInternalTag();
	this->INTERNAL_LOSE_SCORE_TAG = this->getUniqueInternalTag();

	/** External (config) tags */

	/** Global tags */
	this->dateTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("DateTag"), STRING_LITERAL_AS_REFERENCE("{DATE}"));
	this->timeTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("TimeTag"), STRING_LITERAL_AS_REFERENCE("{TIME}"));

	/** Server tags */
	this->rconVersionTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("RCONVersionTag"), STRING_LITERAL_AS_REFERENCE("{RVER}"));
	this->gameVersionTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("GameVersionTag"), STRING_LITERAL_AS_REFERENCE("{GVER}"));
	this->rulesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("RulesTag"), STRING_LITERAL_AS_REFERENCE("{RULES}"));
	this->userTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("UserTag"), STRING_LITERAL_AS_REFERENCE("{USER}"));
	this->serverNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ServerNameTag"), STRING_LITERAL_AS_REFERENCE("{SERVERNAME}"));
	this->mapTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("MapTag"), STRING_LITERAL_AS_REFERENCE("{MAP}"));
	this->serverHostnameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ServerHostnameTag"), STRING_LITERAL_AS_REFERENCE("{SERVERHOST}"));
	this->serverPortTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ServerPortTag"), STRING_LITERAL_AS_REFERENCE("{SERVERPORT}"));
	this->socketHostnameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("SocketHostnameTag"), STRING_LITERAL_AS_REFERENCE("{SOCKHOST}"));
	this->socketPortTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("SocketPortTag"), STRING_LITERAL_AS_REFERENCE("{SOCKPORT}"));
	this->serverPrefixTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ServerPrefixTag"), STRING_LITERAL_AS_REFERENCE("{SERVERPREFIX}"));

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
	this->beaconDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BeaconDisarmsTag"), STRING_LITERAL_AS_REFERENCE("{BEACONDISARMS}"));
	this->capturesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("CapturesTag"), STRING_LITERAL_AS_REFERENCE("{CAPTURES}"));
	this->stealsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("StealsTag"), STRING_LITERAL_AS_REFERENCE("{STEALS}"));
	this->stolenTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("StolenTag"), STRING_LITERAL_AS_REFERENCE("{STOLEN}"));
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
	this->victimBeaconDisarmsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimBeaconDisarmsTag"), STRING_LITERAL_AS_REFERENCE("{VBEACONDISARMS}"));
	this->victimCapturesTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimCapturesTag"), STRING_LITERAL_AS_REFERENCE("{VCAPTURES}"));
	this->victimStealsTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimStealsTag"), STRING_LITERAL_AS_REFERENCE("{VSTEALS}"));
	this->victimStolenTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimStolenTag"), STRING_LITERAL_AS_REFERENCE("{VSTOLEN}"));
	this->victimAccessTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("VictimAccessTag"), STRING_LITERAL_AS_REFERENCE("{VACCESS}"));

	/** Building tags */
	this->buildingNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingNameTag"), STRING_LITERAL_AS_REFERENCE("{BNAME}"));
	this->buildingRawNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingRawNameTag"), STRING_LITERAL_AS_REFERENCE("{BRNAME}"));
	this->buildingHealthTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingHealthTag"), STRING_LITERAL_AS_REFERENCE("{BHEALTH}"));
	this->buildingMaxHealthTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingMaxHealthTag"), STRING_LITERAL_AS_REFERENCE("{BMHEALTH}"));
	this->buildingHealthPercentageTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingHealthPercentageTag"), STRING_LITERAL_AS_REFERENCE("{BHP}"));
	this->buildingHealthBarTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingHealthBarTag"), STRING_LITERAL_AS_REFERENCE("{BHBAR}"));
	this->buildingTeamColorTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingTeamColorTag"), STRING_LITERAL_AS_REFERENCE("{BCOLOR}"));
	this->buildingTeamShortTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingShortTeamTag"), STRING_LITERAL_AS_REFERENCE("{BTEAMS}"));
	this->buildingTeamLongTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("BuildingLongTeamTag"), STRING_LITERAL_AS_REFERENCE("{BTEAML}"));

	/** Other tags */
	this->weaponTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("WeaponTag"), STRING_LITERAL_AS_REFERENCE("{WEAPON}"));
	this->objectTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("ObjectTag"), STRING_LITERAL_AS_REFERENCE("{OBJECT}"));
	this->messageTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("MessageTag"), STRING_LITERAL_AS_REFERENCE("{MESSAGE}"));
	this->newNameTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("NewNameTag"), STRING_LITERAL_AS_REFERENCE("{NNAME}"));
	this->winScoreTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("WinScoreTag"), STRING_LITERAL_AS_REFERENCE("{WINSCORE}"));
	this->loseScoreTag = Jupiter::IRC::Client::Config->get(configSection, STRING_LITERAL_AS_REFERENCE("LoseScoreTag"), STRING_LITERAL_AS_REFERENCE("{LOSESCORE}"));
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

#define PROCESS_TAG(tag, value) \
while(true) { \
index = msg.find(tag); \
if (index == Jupiter::INVALID_INDEX) break; \
msg.replace(index, tag.size(), value); }

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
		PROCESS_TAG(this->INTERNAL_MAP_TAG, server->getMap());
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
		PROCESS_TAG(this->INTERNAL_CREDITS_TAG, Jupiter::StringS::Format("%.0f", player->credits));
		PROCESS_TAG(this->INTERNAL_KILLS_TAG, Jupiter::StringS::Format("%u", player->kills));
		PROCESS_TAG(this->INTERNAL_DEATHS_TAG, Jupiter::StringS::Format("%u", player->deaths));
		PROCESS_TAG(this->INTERNAL_KDR_TAG, Jupiter::StringS::Format("%.2f", static_cast<double>(player->kills) / (player->deaths == 0 ? 1.0f : static_cast<double>(player->deaths))));
		PROCESS_TAG(this->INTERNAL_SUICIDES_TAG, Jupiter::StringS::Format("%u", player->suicides));
		PROCESS_TAG(this->INTERNAL_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", player->headshots));
		PROCESS_TAG(this->INTERNAL_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", player->vehicleKills));
		PROCESS_TAG(this->INTERNAL_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", player->buildingKills));
		PROCESS_TAG(this->INTERNAL_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", player->defenceKills));
		PROCESS_TAG(this->INTERNAL_WINS_TAG, Jupiter::StringS::Format("%u", player->wins));
		PROCESS_TAG(this->INTERNAL_LOSES_TAG, Jupiter::StringS::Format("%u", player->loses));
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
		PROCESS_TAG(this->INTERNAL_VICTIM_CREDITS_TAG, Jupiter::StringS::Format("%.0f", victim->credits));
		PROCESS_TAG(this->INTERNAL_VICTIM_KILLS_TAG, Jupiter::StringS::Format("%u", victim->kills));
		PROCESS_TAG(this->INTERNAL_VICTIM_DEATHS_TAG, Jupiter::StringS::Format("%u", victim->deaths));
		PROCESS_TAG(this->INTERNAL_VICTIM_KDR_TAG, Jupiter::StringS::Format("%.2f", static_cast<double>(victim->kills) / (victim->deaths == 0 ? 1.0f : static_cast<double>(victim->deaths))));
		PROCESS_TAG(this->INTERNAL_VICTIM_SUICIDES_TAG, Jupiter::StringS::Format("%u", victim->suicides));
		PROCESS_TAG(this->INTERNAL_VICTIM_HEADSHOTS_TAG, Jupiter::StringS::Format("%u", victim->headshots));
		PROCESS_TAG(this->INTERNAL_VICTIM_VEHICLE_KILLS_TAG, Jupiter::StringS::Format("%u", victim->vehicleKills));
		PROCESS_TAG(this->INTERNAL_VICTIM_BUILDING_KILLS_TAG, Jupiter::StringS::Format("%u", victim->buildingKills));
		PROCESS_TAG(this->INTERNAL_VICTIM_DEFENCE_KILLS_TAG, Jupiter::StringS::Format("%u", victim->defenceKills));
		PROCESS_TAG(this->INTERNAL_VICTIM_WINS_TAG, Jupiter::StringS::Format("%u", victim->wins));
		PROCESS_TAG(this->INTERNAL_VICTIM_LOSES_TAG, Jupiter::StringS::Format("%u", victim->loses));
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
	for (size_t i = 0; i < xPlugins.size(); i++)
		xPlugins.get(i)->RenX_ProcessTags(msg, server, player, victim, building);
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
	fmt.replace(this->serverHostnameTag, this->INTERNAL_SERVER_HOSTNAME_TAG);
	fmt.replace(this->serverPortTag, this->INTERNAL_SERVER_PORT_TAG);
	fmt.replace(this->socketHostnameTag, this->INTERNAL_SOCKET_HOSTNAME_TAG);
	fmt.replace(this->socketPortTag, this->INTERNAL_SOCKET_PORT_TAG);
	fmt.replace(this->serverPrefixTag, this->INTERNAL_SERVER_PREFIX_TAG);

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
	fmt.replace(this->beaconDisarmsTag, this->INTERNAL_BEACON_DISARMS_TAG);
	fmt.replace(this->capturesTag, this->INTERNAL_CAPTURES_TAG);
	fmt.replace(this->stealsTag, this->INTERNAL_STEALS_TAG);
	fmt.replace(this->stolenTag, this->INTERNAL_STOLEN_TAG);
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
	fmt.replace(this->victimBeaconDisarmsTag, this->INTERNAL_VICTIM_BEACON_DISARMS_TAG);
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

void RenX::sanitizeTags(Jupiter::StringType &fmt)
{
	_tags.sanitizeTags(fmt);
}