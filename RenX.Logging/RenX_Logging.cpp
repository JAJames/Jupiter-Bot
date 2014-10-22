/**
 * Copyright (C) 2014 Justin James.
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

#include "Jupiter/INIFile.h"
#include "IRC_Bot.h"
#include "RenX_Logging.h"
#include "RenX_Core.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_Server.h"


Jupiter::ReferenceString INTERNAL_RCON_VERSION_TAG = STRING_LITERAL_AS_REFERENCE("\0RVER\0");
Jupiter::ReferenceString INTERNAL_GAME_VERSION_TAG = STRING_LITERAL_AS_REFERENCE("\0GVER\0");
Jupiter::ReferenceString INTERNAL_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0NVER\0");
Jupiter::ReferenceString INTERNAL_RAW_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0RNAME\0");
Jupiter::ReferenceString INTERNAL_IP_TAG = STRING_LITERAL_AS_REFERENCE("\0IP\0");
Jupiter::ReferenceString INTERNAL_STEAM_TAG = STRING_LITERAL_AS_REFERENCE("\0STEAM\0");
Jupiter::ReferenceString INTERNAL_UUID_TAG = STRING_LITERAL_AS_REFERENCE("\0UUID\0");
Jupiter::ReferenceString INTERNAL_ID_TAG = STRING_LITERAL_AS_REFERENCE("\0ID\0");
Jupiter::ReferenceString INTERNAL_ADMIN_TAG = STRING_LITERAL_AS_REFERENCE("\0ADM\0");
Jupiter::ReferenceString INTERNAL_TEAM_COLOR_TAG = STRING_LITERAL_AS_REFERENCE("\0TC\0");
Jupiter::ReferenceString INTERNAL_TEAM_SHORT_TAG = STRING_LITERAL_AS_REFERENCE("\0TS\0");
Jupiter::ReferenceString INTERNAL_TEAM_LONG_TAG = STRING_LITERAL_AS_REFERENCE("\0TL\0");
Jupiter::ReferenceString INTERNAL_VICTIM_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0VNAME\0");
Jupiter::ReferenceString INTERNAL_VICTIM_RAW_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0VRNAME\0");
Jupiter::ReferenceString INTERNAL_VICTIM_IP_TAG = STRING_LITERAL_AS_REFERENCE("\0VIP\0");
Jupiter::ReferenceString INTERNAL_VICTIM_STEAM_TAG = STRING_LITERAL_AS_REFERENCE("\0VSTEAM\0");
Jupiter::ReferenceString INTERNAL_VICTIM_UUID_TAG = STRING_LITERAL_AS_REFERENCE("\0VUUID\0");
Jupiter::ReferenceString INTERNAL_VICTIM_ID_TAG = STRING_LITERAL_AS_REFERENCE("\0VID\0");
Jupiter::ReferenceString INTERNAL_VICTIM_ADMIN_TAG = STRING_LITERAL_AS_REFERENCE("\0VADM\0");
Jupiter::ReferenceString INTERNAL_VICTIM_TEAM_COLOR_TAG = STRING_LITERAL_AS_REFERENCE("\0VTC\0");
Jupiter::ReferenceString INTERNAL_VICTIM_TEAM_SHORT_TAG = STRING_LITERAL_AS_REFERENCE("\0VTS\0");
Jupiter::ReferenceString INTERNAL_VICTIM_TEAM_LONG_TAG = STRING_LITERAL_AS_REFERENCE("\0VTL\0");
Jupiter::ReferenceString INTERNAL_WEAPON_TAG = STRING_LITERAL_AS_REFERENCE("\0WEP\0");
Jupiter::ReferenceString INTERNAL_OBJECT_TAG = STRING_LITERAL_AS_REFERENCE("\0OBJ\0");
Jupiter::ReferenceString INTERNAL_MESSAGE_TAG = STRING_LITERAL_AS_REFERENCE("\0MSG\0");
Jupiter::ReferenceString INTERNAL_NEW_NAME_TAG = STRING_LITERAL_AS_REFERENCE("\0NNAME\0");
Jupiter::ReferenceString INTERNAL_WIN_SCORE_TAG = STRING_LITERAL_AS_REFERENCE("\0WSC\0");
Jupiter::ReferenceString INTERNAL_LOSE_SCORE_TAG = STRING_LITERAL_AS_REFERENCE("\0LSC\0");

void RenX_LoggingPlugin::init()
{
	RenX_LoggingPlugin::joinPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("JoinPublic"), true);
	RenX_LoggingPlugin::partPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("PartPublic"), true);
	RenX_LoggingPlugin::nameChangePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("NameChangePublic"), true);
	RenX_LoggingPlugin::teamChangePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TeamChangePublic"), true);
	RenX_LoggingPlugin::chatPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ChatPublic"), true);
	RenX_LoggingPlugin::teamChatPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TeamChatPublic"), true);
	RenX_LoggingPlugin::deployPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DeployPublic"), true);
	RenX_LoggingPlugin::suicidePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SuicidePublic"), true);
	RenX_LoggingPlugin::killPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("KillPublic"), true);
	RenX_LoggingPlugin::diePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DiePublic"), true);
	RenX_LoggingPlugin::destroyPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DestroyPublic"), true);
	RenX_LoggingPlugin::gamePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("GamePublic"), true);
	RenX_LoggingPlugin::gameOverPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("GameOverPublic"), true);
	RenX_LoggingPlugin::executePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ExecutePublic"), false);
	RenX_LoggingPlugin::subscribePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SubscribePublic"), false);
	RenX_LoggingPlugin::RCONPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("RCONPublic"), false);
	RenX_LoggingPlugin::adminLoginPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminLoginPublic"), true);
	RenX_LoggingPlugin::adminGrantPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminGrantPublic"), true);
	RenX_LoggingPlugin::adminLogoutPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminLogoutPublic"), true);
	RenX_LoggingPlugin::adminPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminPublic"), false);
	RenX_LoggingPlugin::logPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("LogPublic"), false);
	RenX_LoggingPlugin::commandPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("CommandPublic"), false);
	RenX_LoggingPlugin::errorPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ErrorPublic"), false);
	RenX_LoggingPlugin::versionPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VersionPublic"), true);
	RenX_LoggingPlugin::authorizedPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AuthorizedPublic"), true);
	RenX_LoggingPlugin::otherPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("OtherPublic"), false);

	/** Server tags */
	RenX_LoggingPlugin::rconVersionTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RCONVersionTag"), STRING_LITERAL_AS_REFERENCE("{RVER}"));
	RenX_LoggingPlugin::gameVersionTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameVersionTag"), STRING_LITERAL_AS_REFERENCE("{GVER}"));

	/** Player tags */
	RenX_LoggingPlugin::nameTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NameTag"), STRING_LITERAL_AS_REFERENCE("{NAME}"));
	RenX_LoggingPlugin::rawNameTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RawNameTag"), STRING_LITERAL_AS_REFERENCE("{RNAME}"));
	RenX_LoggingPlugin::ipTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("IPTag"), STRING_LITERAL_AS_REFERENCE("{IP}"));
	RenX_LoggingPlugin::steamTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SteamTag"), STRING_LITERAL_AS_REFERENCE("{STEAM}"));
	RenX_LoggingPlugin::uuidTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("UUIDTag"), STRING_LITERAL_AS_REFERENCE("{UUID}"));
	RenX_LoggingPlugin::idTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("IDTag"), STRING_LITERAL_AS_REFERENCE("{ID}"));
	RenX_LoggingPlugin::adminTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminTag"), STRING_LITERAL_AS_REFERENCE("{ADMIN}"));
	RenX_LoggingPlugin::teamColorTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamColorTag"), STRING_LITERAL_AS_REFERENCE("{TCOLOR}"));
	RenX_LoggingPlugin::teamShortTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ShortTeamTag"), STRING_LITERAL_AS_REFERENCE("{TEAMS}"));
	RenX_LoggingPlugin::teamLongTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("LongTeamTag"), STRING_LITERAL_AS_REFERENCE("{TEAML}"));

	/** Victim player tags */
	RenX_LoggingPlugin::victimNameTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimNameTag"), STRING_LITERAL_AS_REFERENCE("{VNAME}"));
	RenX_LoggingPlugin::victimRawNameTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimRawNameTag"), STRING_LITERAL_AS_REFERENCE("{VRNAME}"));
	RenX_LoggingPlugin::victimIPTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimIPTag"), STRING_LITERAL_AS_REFERENCE("{VIP}"));
	RenX_LoggingPlugin::victimSteamTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimSteamTag"), STRING_LITERAL_AS_REFERENCE("{VSTEAM}"));
	RenX_LoggingPlugin::victimUUIDTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimUUIDTag"), STRING_LITERAL_AS_REFERENCE("{VUUID}"));
	RenX_LoggingPlugin::victimIDTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimIDTag"), STRING_LITERAL_AS_REFERENCE("{VID}"));
	RenX_LoggingPlugin::adminTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimAdminTag"), STRING_LITERAL_AS_REFERENCE("{VADMIN}"));
	RenX_LoggingPlugin::victimTeamColorTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimTeamColorTag"), STRING_LITERAL_AS_REFERENCE("{VTCOLOR}"));
	RenX_LoggingPlugin::victimTeamShortTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimShortTeamTag"), STRING_LITERAL_AS_REFERENCE("{VTEAMS}"));
	RenX_LoggingPlugin::victimTeamLongTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VictimLongTeamTag"), STRING_LITERAL_AS_REFERENCE("{VTEAML}"));

	/** Other tags */
	RenX_LoggingPlugin::weaponTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("WeaponTag"), STRING_LITERAL_AS_REFERENCE("{WEAPON}"));
	RenX_LoggingPlugin::objectTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ObjectTag"), STRING_LITERAL_AS_REFERENCE("{OBJECT}"));
	RenX_LoggingPlugin::messageTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MessageTag"), STRING_LITERAL_AS_REFERENCE("{MESSAGE}"));
	RenX_LoggingPlugin::newNameTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NewNameTag"), STRING_LITERAL_AS_REFERENCE("{NNAME}"));
	RenX_LoggingPlugin::winScoreTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("WinScoreTag"), STRING_LITERAL_AS_REFERENCE("{WINSCORE}"));
	RenX_LoggingPlugin::loseScoreTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("LoseScoreTag"), STRING_LITERAL_AS_REFERENCE("{LOSESCORE}"));

	/** Event formats */
	RenX_LoggingPlugin::joinPublicFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinPublicFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Join] " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s!", this->nameTag.size(), this->nameTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr()));

	RenX_LoggingPlugin::joinAdminFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinAdminFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Join] " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s from " IRCBOLD "%.*s" IRCBOLD " using Steam ID " IRCBOLD "%.*s" IRCBOLD ".", this->nameTag.size(), this->nameTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr(), this->ipTag.size(), this->ipTag.ptr(), this->steamTag.size(), this->steamTag.ptr()));

	RenX_LoggingPlugin::joinNoSteamAdminFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinNoSteamAdminFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Join] " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s from " IRCBOLD "%.*s" IRCBOLD ", but is not using Steam.", this->nameTag.size(), this->nameTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr(), this->ipTag.size(), this->ipTag.ptr()));

	RenX_LoggingPlugin::partFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("PartFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Part] " IRCBOLD "%.*s" IRCBOLD " has left the %.*s.", this->nameTag.size(), this->nameTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr()));

	RenX_LoggingPlugin::nameChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NameChangeFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD " has changed their name to " IRCBOLD "%.*s" IRCBOLD ".", this->nameTag.size(), this->nameTag.ptr(), this->newNameTag.size(), this->newNameTag.ptr()));

	RenX_LoggingPlugin::teamChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamChangeFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " switched teams!", this->nameTag.size(), this->nameTag.ptr()));

	RenX_LoggingPlugin::chatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ChatFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD ": %.*s", this->nameTag.size(), this->nameTag.ptr(), this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::teamChatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamChatFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD ": %.*s", this->nameTag.size(), this->nameTag.ptr(), this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::deployFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DeployFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD " deployed a " IRCBOLD "%.*s" IRCBOLD, this->nameTag.size(), this->nameTag.ptr(), this->objectTag.size(), this->objectTag.ptr()));

	RenX_LoggingPlugin::suicideFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SuicideFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " suicided (death by " IRCCOLOR "12%.*s" IRCCOLOR ").", this->nameTag.size(), this->nameTag.ptr(), this->weaponTag.size(), this->weaponTag.ptr()));

	RenX_LoggingPlugin::killFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("KillFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " killed %.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", this->nameTag.size(), this->nameTag.ptr(), this->victimNameTag.size(), this->victimNameTag.ptr(), this->weaponTag.size(), this->weaponTag.ptr()));

	RenX_LoggingPlugin::dieFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DieFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " died (" IRCCOLOR "12%.*s" IRCCOLOR ").", this->nameTag.size(), this->nameTag.ptr(), this->weaponTag.size(), this->weaponTag.ptr()));

	RenX_LoggingPlugin::destroyBuildingFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyBuildingFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " destroyed the " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", this->nameTag.size(), this->nameTag.ptr(), this->victimTeamColorTag.size(), this->victimTeamColorTag.ptr(), this->objectTag.size(), this->objectTag.ptr(), this->weaponTag.size(), this->weaponTag.ptr()));
	
	RenX_LoggingPlugin::destroyDefenceFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyDefenceFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " destroyed a " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", this->nameTag.size(), this->nameTag.ptr(), this->victimTeamColorTag.size(), this->victimTeamColorTag.ptr(), this->objectTag.size(), this->objectTag.ptr(), this->weaponTag.size(), this->weaponTag.ptr()));
	
	RenX_LoggingPlugin::destroyVehicleFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyVehicleFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " destroyed a " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", this->nameTag.size(), this->nameTag.ptr(), this->victimTeamColorTag.size(), this->victimTeamColorTag.ptr(), this->objectTag.size(), this->objectTag.ptr(), this->weaponTag.size(), this->weaponTag.ptr()));

	RenX_LoggingPlugin::gameOverFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "%.*s The " IRCBOLD "%.*s" IRCBOLD " won by " IRCBOLD "%.*s" IRCBOLD, this->teamColorTag.size(), this->teamColorTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr(), this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::gameOverTieFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverTieNoWinFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "10 The battle ended in a " IRCBOLD "%.*s" IRCBOLD " - Victory handed to " IRCBOLD IRCCOLOR "%.*s%.*s" IRCBOLD, this->messageTag.size(), this->messageTag.ptr(), this->teamColorTag.size(), this->teamColorTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr()));

	RenX_LoggingPlugin::gameOverTieNoWinFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverTieFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "10 The battle ended in a " IRCBOLD "%.*s" IRCBOLD, this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::gameOverScoreFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverScoreFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "%.*s %.*s" IRCCOLOR ": %.*s | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %.*s", this->teamColorTag.size(), this->teamColorTag.ptr(), this->teamLongTag.size(), this->teamLongTag.ptr(), this->winScoreTag.size(), this->winScoreTag.ptr(), this->victimTeamColorTag.size(), this->victimTeamColorTag.ptr(), this->victimTeamLongTag.size(), this->victimTeamLongTag.ptr(), this->loseScoreTag.size(), this->loseScoreTag.ptr()));

	RenX_LoggingPlugin::gameFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR " %.*s", this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::executeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ExecuteFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07%.*s executed: %.*s", this->nameTag.size(), this->nameTag.ptr(), this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::evaFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("EVAFormat"),
		Jupiter::StringS::Format(IRCCOLOR "09EVA: %.*s", this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::evaPrivateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("EVAPrivateFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12(EVA -> %.*s): %.*s", this->rawNameTag.size(), this->rawNameTag.ptr(), this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::subscribeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SubscribeFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03%.*s has subscribed to the RCON data stream.", this->nameTag.size(), this->nameTag.ptr()));

	RenX_LoggingPlugin::rconFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RCONFormat"),
		Jupiter::StringS::Format(IRCCOLOR "05[RCON]" IRCCOLOR " %.*s", this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::adminLoginFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminLoginFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " has logged in with " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", this->nameTag.size(), this->nameTag.ptr(), this->adminTag.size(), this->adminTag.ptr()));

	RenX_LoggingPlugin::adminGrantFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminGrantFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " has been granted " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", this->nameTag.size(), this->nameTag.ptr(), this->adminTag.size(), this->adminTag.ptr()));

	RenX_LoggingPlugin::adminLogoutFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminLogoutFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " has logged out of their " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", this->nameTag.size(), this->nameTag.ptr(), this->adminTag.size(), this->adminTag.ptr()));

	RenX_LoggingPlugin::adminFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin]" IRCCOLOR " %.*s", this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::logFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("LogFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Log]" IRCCOLOR " %.*s", this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::commandFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("CommandFormat"),
		Jupiter::StringS::Format("")); // Disabled by default.

	RenX_LoggingPlugin::errorFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ErrorFormat"),
		Jupiter::StringS::Format(IRCCOLOR "04[Error]" IRCCOLOR " %.*s", this->messageTag.size(), this->messageTag.ptr()));

	RenX_LoggingPlugin::versionFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VersionFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03Renegade X RCON connection established; using RCON verison " IRCBOLD "%.*s" IRCBOLD " for game version " IRCBOLD "%.*s" IRCBOLD, this->rconVersionTag.size(), this->rconVersionTag.ptr(), this->gameVersionTag.size(), this->gameVersionTag.ptr()));

	RenX_LoggingPlugin::authorizedFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AuthorizedFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03RCON authorization completed."));

	RenX_LoggingPlugin::otherFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("OtherFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[Other]" IRCCOLOR " %.*s", this->messageTag.size(), this->messageTag.ptr()));

	/** Sanitize tags in formats to prevent explotation */
	this->sanitizeTags(joinPublicFmt);
	this->sanitizeTags(joinAdminFmt);
	this->sanitizeTags(joinNoSteamAdminFmt);
	this->sanitizeTags(partFmt);
	this->sanitizeTags(nameChangeFmt);
	this->sanitizeTags(teamChangeFmt);
	this->sanitizeTags(chatFmt);
	this->sanitizeTags(teamChatFmt);
	this->sanitizeTags(deployFmt);
	this->sanitizeTags(suicideFmt);
	this->sanitizeTags(dieFmt);
	this->sanitizeTags(killFmt);
	this->sanitizeTags(destroyBuildingFmt);
	this->sanitizeTags(destroyDefenceFmt);
	this->sanitizeTags(destroyVehicleFmt);
	this->sanitizeTags(gameOverFmt);
	this->sanitizeTags(gameOverTieFmt);
	this->sanitizeTags(gameOverTieNoWinFmt);
	this->sanitizeTags(gameOverScoreFmt);
	this->sanitizeTags(gameFmt);
	this->sanitizeTags(executeFmt);
	this->sanitizeTags(evaFmt);
	this->sanitizeTags(evaPrivateFmt);
	this->sanitizeTags(subscribeFmt);
	this->sanitizeTags(rconFmt);
	this->sanitizeTags(adminLoginFmt);
	this->sanitizeTags(adminGrantFmt);
	this->sanitizeTags(adminLogoutFmt);
	this->sanitizeTags(adminFmt);
	this->sanitizeTags(logFmt);
	this->sanitizeTags(commandFmt);
	this->sanitizeTags(errorFmt);
	this->sanitizeTags(versionFmt);
	this->sanitizeTags(authorizedFmt);
	this->sanitizeTags(otherFmt);
}

void RenX_LoggingPlugin::sanitizeTags(Jupiter::StringType &fmt) const
{
	/** Server tags */
	fmt.replace(this->rconVersionTag, INTERNAL_RCON_VERSION_TAG);
	fmt.replace(this->gameVersionTag, INTERNAL_GAME_VERSION_TAG);

	/** Player tags */
	fmt.replace(this->nameTag, INTERNAL_NAME_TAG);
	fmt.replace(this->rawNameTag, INTERNAL_RAW_NAME_TAG);
	fmt.replace(this->ipTag, INTERNAL_IP_TAG);
	fmt.replace(this->steamTag, INTERNAL_STEAM_TAG);
	fmt.replace(this->uuidTag, INTERNAL_UUID_TAG);
	fmt.replace(this->idTag, INTERNAL_ID_TAG);
	fmt.replace(this->adminTag, INTERNAL_ADMIN_TAG);
	fmt.replace(this->teamColorTag, INTERNAL_TEAM_COLOR_TAG);
	fmt.replace(this->teamShortTag, INTERNAL_TEAM_SHORT_TAG);
	fmt.replace(this->teamLongTag, INTERNAL_TEAM_LONG_TAG);

	/** Victim tags */
	fmt.replace(this->victimNameTag, INTERNAL_VICTIM_NAME_TAG);
	fmt.replace(this->victimRawNameTag, INTERNAL_VICTIM_RAW_NAME_TAG);
	fmt.replace(this->victimIPTag, INTERNAL_VICTIM_IP_TAG);
	fmt.replace(this->victimSteamTag, INTERNAL_VICTIM_STEAM_TAG);
	fmt.replace(this->victimUUIDTag, INTERNAL_VICTIM_UUID_TAG);
	fmt.replace(this->victimIDTag, INTERNAL_VICTIM_ID_TAG);
	fmt.replace(this->victimAdminTag, INTERNAL_VICTIM_ADMIN_TAG);
	fmt.replace(this->victimTeamColorTag, INTERNAL_VICTIM_TEAM_COLOR_TAG);
	fmt.replace(this->victimTeamShortTag, INTERNAL_VICTIM_TEAM_SHORT_TAG);
	fmt.replace(this->victimTeamLongTag, INTERNAL_VICTIM_TEAM_LONG_TAG);

	/** Other tags */
	fmt.replace(this->weaponTag, INTERNAL_WEAPON_TAG);
	fmt.replace(this->objectTag, INTERNAL_OBJECT_TAG);
	fmt.replace(this->messageTag, INTERNAL_MESSAGE_TAG);
	fmt.replace(this->newNameTag, INTERNAL_NEW_NAME_TAG);
	fmt.replace(this->winScoreTag, INTERNAL_WIN_SCORE_TAG);
	fmt.replace(this->loseScoreTag, INTERNAL_LOSE_SCORE_TAG);
}

void RenX_LoggingPlugin::processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim) const
{
	msg.replace(INTERNAL_RCON_VERSION_TAG, Jupiter::StringS::Format("%u", server->getVersion()));
	msg.replace(INTERNAL_GAME_VERSION_TAG, server->getGameVersion());
	if (player != nullptr)
	{
		msg.replace(INTERNAL_NAME_TAG, RenX::getFormattedPlayerName(player));
		msg.replace(INTERNAL_RAW_NAME_TAG, player->name);
		msg.replace(INTERNAL_IP_TAG, player->ip);
		msg.replace(INTERNAL_STEAM_TAG, server->formatSteamID(player));
		msg.replace(INTERNAL_UUID_TAG, player->uuid);
		msg.replace(INTERNAL_ID_TAG, Jupiter::StringS::Format("%d", player->id));
		msg.replace(INTERNAL_ADMIN_TAG, player->adminType);
		msg.replace(INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(player->team));
		msg.replace(INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(player->team));
		msg.replace(INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(player->team));
	}
	if (victim != nullptr)
	{
		msg.replace(INTERNAL_VICTIM_NAME_TAG, RenX::getFormattedPlayerName(victim));
		msg.replace(INTERNAL_VICTIM_RAW_NAME_TAG, victim->name);
		msg.replace(INTERNAL_VICTIM_IP_TAG, victim->ip);
		msg.replace(INTERNAL_VICTIM_STEAM_TAG, server->formatSteamID(victim));
		msg.replace(INTERNAL_VICTIM_UUID_TAG, victim->uuid);
		msg.replace(INTERNAL_VICTIM_ID_TAG, Jupiter::StringS::Format("%d", victim->id));
		msg.replace(INTERNAL_VICTIM_ADMIN_TAG, victim->adminType);
		msg.replace(INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(victim->team));
		msg.replace(INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(victim->team));
		msg.replace(INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(victim->team));
	}
}

typedef void(RenX::Server::*logFuncType)(const Jupiter::ReadableString &msg) const;

void RenX_LoggingPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	puts(__FUNCTION__);
	Jupiter::String msg;
	if (RenX_LoggingPlugin::joinPublic)
	{
		msg = this->joinPublicFmt;
		if (msg.isEmpty() == false)
		{
			this->processTags(msg, server, player);
			server->sendPubChan(msg);
		}
	}
	if (player->steamid == 0)
		msg = this->joinNoSteamAdminFmt;
	else
		msg = this->joinAdminFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		server->sendAdmChan(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::partPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->partFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnNameChange(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &newPlayerName)
{
	logFuncType func;
	if (RenX_LoggingPlugin::nameChangePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->nameChangeFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_NEW_NAME_TAG, newPlayerName);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnTeamChange(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::teamChangePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->teamChangeFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	logFuncType func;
	if (RenX_LoggingPlugin::chatPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->chatFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_MESSAGE_TAG, message);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnTeamChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	logFuncType func;
	if (RenX_LoggingPlugin::teamChatPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->teamChatFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_MESSAGE_TAG, message);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDeploy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object)
{
	const Jupiter::ReadableString &translated = RenX::translateName(object);

	logFuncType func;
	if (RenX_LoggingPlugin::deployPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->deployFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_OBJECT_TAG, translated);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);

	logFuncType func;
	if (RenX_LoggingPlugin::suicidePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->suicideFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_WEAPON_TAG, translated);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);

	logFuncType func;
	if (RenX_LoggingPlugin::killPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->killFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player, victim);
		msg.replace(INTERNAL_WEAPON_TAG, translated);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);

	logFuncType func;
	if (RenX_LoggingPlugin::diePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->dieFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_WEAPON_TAG, translated);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
{
	logFuncType func;
	if (RenX_LoggingPlugin::destroyPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg;
	switch (type)
	{
	case RenX::ObjectType::Building:
		msg = this->destroyBuildingFmt;
		break;
	case RenX::ObjectType::Defence:
		msg = this->destroyDefenceFmt;
		break;
	default:
		msg = this->destroyVehicleFmt;
		break;
	}
	if (msg.isEmpty() == false)
	{
		RenX::TeamType victimTeam = RenX::getEnemy(player->team);
		this->processTags(msg, server, player);
		msg.replace(INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(victimTeam));
		msg.replace(INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(victimTeam));
		msg.replace(INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(victimTeam));
		msg.replace(INTERNAL_OBJECT_TAG, RenX::translateName(objectName));
		msg.replace(INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
{
	const Jupiter::ReadableString &gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	const Jupiter::ReadableString &gTeamName = RenX::getFullTeamName(RenX::TeamType::GDI);

	const Jupiter::ReadableString &nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	const Jupiter::ReadableString &nTeamName = RenX::getFullTeamName(RenX::TeamType::Nod);

	RenX::TeamType loserTeam = RenX::getEnemy(team);
	int winScore;
	int loseScore;
	switch (team)
	{
	default:
	case RenX::TeamType::GDI:
		winScore = gScore;
		loseScore = nScore;
		break;
	case RenX::TeamType::Nod:
		winScore = nScore;
		loseScore = gScore;
		break;
	}

	logFuncType func;
	if (RenX_LoggingPlugin::gameOverPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg;
	if (winType == RenX::WinType::Tie)
	{
		if (team == RenX::TeamType::Other)
			msg = this->gameOverTieNoWinFmt;
		else
			msg = this->gameOverTieFmt;
	}
	else
		msg = this->gameOverFmt;

	if (msg.isEmpty() == false)
	{
		msg.replace(INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(INTERNAL_WIN_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
		msg.replace(INTERNAL_LOSE_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
		msg.replace(INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(loserTeam));
		msg.replace(INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(loserTeam));
		msg.replace(INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(loserTeam));
		msg.replace(INTERNAL_MESSAGE_TAG, RenX::translateWinType(winType));
		(server->*func)(msg);
	}

	msg = this->gameOverScoreFmt;
	if (team = RenX::TeamType::Other)
		team = RenX::TeamType::GDI;

	if (msg.isEmpty() == false)
	{
		msg.replace(INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(INTERNAL_WIN_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
		msg.replace(INTERNAL_LOSE_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
		msg.replace(INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(loserTeam));
		msg.replace(INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(loserTeam));
		msg.replace(INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(loserTeam));
		msg.replace(INTERNAL_MESSAGE_TAG, RenX::translateWinType(winType));
		(server->*func)(msg);
	}

	if (server->profile->disconnectOnGameOver)
		server->sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Game version \"%.*s\" gets disconnected when a map unloads; to prevent disconnect spam, player disconnects are silenced until the bot reconnects.", server->getGameVersion().size(), server->getGameVersion().ptr());
}

void RenX_LoggingPlugin::RenX_OnGame(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::gamePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->gameFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnExecute(RenX::Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command)
{
	logFuncType func;
	if (RenX_LoggingPlugin::executePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg;
	if (command.matchi("say *"))
	{
		msg = this->evaFmt;
		if (msg.isEmpty() == false)
		{
			processTags(msg, server);
			msg.replace(INTERNAL_MESSAGE_TAG, Jupiter::ReferenceString::gotoToken(command, 1, ' '));
			server->sendLogChan(msg);
			return;
		}
	}
	else if (command.matchi("evaprivatesay *"))
	{
		RenX::PlayerInfo *player = server->getPlayerByName(Jupiter::ReferenceString::getToken(command, 1, ' '));
		if (player != nullptr)
		{
			msg = this->evaPrivateFmt;
			if (msg.isEmpty() == false)
			{
				Jupiter::ReferenceString message = Jupiter::ReferenceString::gotoToken(command, 2, ' ');
				processTags(msg, server, player);
				msg.replace(INTERNAL_MESSAGE_TAG, message);
				(server->*func)(msg);
				return;
			}
		}
	}
	msg = this->executeFmt;
	if (msg.isEmpty() == false)
	{
		processTags(msg, server);
		msg.replace(INTERNAL_NAME_TAG, user);
		msg.replace(INTERNAL_MESSAGE_TAG, command);
		(server->*func)(msg);
	}
	// Add a format check later for if user == us.
}

void RenX_LoggingPlugin::RenX_OnSubscribe(RenX::Server *server, const Jupiter::ReadableString &user)
{
	logFuncType func;
	if (RenX_LoggingPlugin::subscribePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->subscribeFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_NAME_TAG, user);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnRCON(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::RCONPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->rconFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::adminLoginPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->adminLoginFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdminGrant(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::adminGrantPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->adminGrantFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::adminLogoutPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->adminLogoutFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdmin(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::adminPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->adminFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnLog(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::logPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->logFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnCommand(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::commandPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->commandFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnError(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::errorPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->errorFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVersion(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::versionPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->versionFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAuthorized(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::authorizedPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->authorizedFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnOther(RenX::Server *server, char token, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::otherPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->otherFmt;
	if (msg.isEmpty() == false)
	{
		this->processTags(msg, server);
		msg.replace(INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

int RenX_LoggingPlugin::OnRehash()
{
	RenX_LoggingPlugin::init();
	return 0;
}

RenX_LoggingPlugin::RenX_LoggingPlugin()
{
	RenX_LoggingPlugin::init();
}

// Plugin instantiation and entry point.
RenX_LoggingPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
