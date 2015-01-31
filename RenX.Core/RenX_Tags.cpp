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
	const Jupiter::ReadableString &configSection = Jupiter::IRC::Client::Config->get(RenX::getCore()->getName(), STRING_LITERAL_AS_REFERENCE("TagDefinitions"), RenX::getCore()->getName());

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