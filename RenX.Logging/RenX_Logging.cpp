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
#include "RenX_Tags.h"

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
	RenX_LoggingPlugin::xVersionPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("XVersionPublic"), true);
	RenX_LoggingPlugin::grantCharacterPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("GrantCharacterPublic"), false);
	RenX_LoggingPlugin::spawnVehiclePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SpawnVehiclePublic"), false);
	RenX_LoggingPlugin::spawnVehicleNoOwnerPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SpawnVehicleNoOwnerPublic"), true);
	RenX_LoggingPlugin::minePlacePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MinePlacePublic"), false);
	RenX_LoggingPlugin::xOtherPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("XOtherPublic"), false);
	RenX_LoggingPlugin::commandPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("CommandPublic"), false);
	RenX_LoggingPlugin::errorPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ErrorPublic"), false);
	RenX_LoggingPlugin::versionPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VersionPublic"), true);
	RenX_LoggingPlugin::authorizedPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AuthorizedPublic"), true);
	RenX_LoggingPlugin::otherPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("OtherPublic"), false);

	/** Event formats */
	RenX_LoggingPlugin::joinPublicFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinPublicFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Join] " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s!", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr()));

	RenX_LoggingPlugin::joinAdminFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinAdminFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Join] " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s from " IRCBOLD "%.*s" IRCBOLD " using Steam ID " IRCBOLD "%.*s" IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr(), RenX::tags->ipTag.size(), RenX::tags->ipTag.ptr(), RenX::tags->steamTag.size(), RenX::tags->steamTag.ptr()));

	RenX_LoggingPlugin::joinNoSteamAdminFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinNoSteamAdminFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Join] " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s from " IRCBOLD "%.*s" IRCBOLD ", but is not using Steam.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr(), RenX::tags->ipTag.size(), RenX::tags->ipTag.ptr()));

	RenX_LoggingPlugin::partFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("PartFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Part] " IRCBOLD "%.*s" IRCBOLD " left the %.*s.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr()));

	RenX_LoggingPlugin::nameChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NameChangeFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD " changed their name to " IRCBOLD "%.*s" IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->newNameTag.size(), RenX::tags->newNameTag.ptr()));

	RenX_LoggingPlugin::teamChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamChangeFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " switched teams!", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::chatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ChatFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD ": %.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::teamChatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamChatFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD ": %.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::deployFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DeployFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD " deployed a " IRCBOLD "%.*s" IRCBOLD, RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::suicideFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SuicideFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " suicided (death by " IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::killFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("KillFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " killed %.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimNameTag.size(), RenX::tags->victimNameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::killFmt2 = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("KillFormat2"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s%.*s" IRCCOLOR " killed %.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimNameTag.size(), RenX::tags->victimNameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::dieFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DieFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " died (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::dieFmt2 = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DieFormat2"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s%.*s" IRCCOLOR " died (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::destroyBuildingFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyBuildingFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " destroyed the " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));
	
	RenX_LoggingPlugin::destroyBuildingFmt2 = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyBuildingFormat2"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s%.*s" IRCCOLOR " destroyed the " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::destroyDefenceFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyDefenceFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " destroyed a " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));
	
	RenX_LoggingPlugin::destroyDefenceFmt2 = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyDefenceFormat2"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s%.*s" IRCCOLOR " destroyed a " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::destroyVehicleFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyVehicleFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " destroyed a " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::destroyVehicleFmt2 = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DestroyVehicleFormat2"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s%.*s" IRCCOLOR " destroyed a " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::gameOverFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "%.*s The " IRCBOLD "%.*s" IRCBOLD " won by " IRCBOLD "%.*s" IRCBOLD, RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::gameOverTieFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverTieNoWinFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "10 The battle ended in a " IRCBOLD "%.*s" IRCBOLD " - Victory handed to " IRCBOLD IRCCOLOR "%.*s%.*s" IRCBOLD, RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr()));

	RenX_LoggingPlugin::gameOverTieNoWinFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverTieFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "10 The battle ended in a " IRCBOLD "%.*s" IRCBOLD, RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::gameOverScoreFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameOverScoreFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR "%.*s %.*s" IRCCOLOR ": %.*s | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %.*s", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->teamLongTag.size(), RenX::tags->teamLongTag.ptr(), RenX::tags->winScoreTag.size(), RenX::tags->winScoreTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->victimTeamLongTag.size(), RenX::tags->victimTeamLongTag.ptr(), RenX::tags->loseScoreTag.size(), RenX::tags->loseScoreTag.ptr()));

	RenX_LoggingPlugin::gameFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GameFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03[Game]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::executeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ExecuteFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07%.*s executed: %.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::evaFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("EVAFormat"),
		Jupiter::StringS::Format(IRCCOLOR "09EVA: %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::evaPrivateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("EVAPrivateFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12(EVA -> %.*s): %.*s", RenX::tags->rawNameTag.size(), RenX::tags->rawNameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::subscribeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SubscribeFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03%.*s subscribed to the RCON data stream.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::rconFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RCONFormat"),
		Jupiter::StringS::Format(IRCCOLOR "05[RCON]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::adminLoginFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminLoginFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " logged in with " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->adminTag.size(), RenX::tags->adminTag.ptr()));

	RenX_LoggingPlugin::adminGrantFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminGrantFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " was granted " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->adminTag.size(), RenX::tags->adminTag.ptr()));

	RenX_LoggingPlugin::adminLogoutFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminLogoutFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " logged out of their " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->adminTag.size(), RenX::tags->adminTag.ptr()));

	RenX_LoggingPlugin::adminFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AdminFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Admin]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::logFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("LogFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Log]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::xVersionFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("XVersionFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03This server is using eXtended RCON version %.*s", RenX::tags->xRconVersionTag.size(), RenX::tags->xRconVersionTag.ptr()));

	RenX_LoggingPlugin::grantCharacterFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("GrantCharacterFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " is now a " IRCBOLD IRCCOLOR "%.*s%.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->victimCharacterTag.size(), RenX::tags->victimCharacterTag.ptr()));

	RenX_LoggingPlugin::spawnVehicleFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SpawnVehicleFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " purchased a " IRCBOLD IRCCOLOR "%.*s%.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->victimVehicleTag.size(), RenX::tags->victimVehicleTag.ptr()));

	RenX_LoggingPlugin::spawnVehicleNoOwnerFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SpawnVehicleNoOwnerFormat"),
		Jupiter::StringS::Format("A " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD " has spawned.", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->vehicleTag.size(), RenX::tags->vehicleTag.ptr()));

	RenX_LoggingPlugin::minePlaceFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MinePlaceFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[C4] " IRCCOLOR IRCBOLD "%.*s" IRCCOLOR IRCBOLD " placed a " IRCBOLD IRCCOLOR "12%.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::xOtherFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("XOtherFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[XOther]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::commandFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("CommandFormat"),
		Jupiter::StringS::Format("")); // Disabled by default.

	RenX_LoggingPlugin::errorFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ErrorFormat"),
		Jupiter::StringS::Format(IRCCOLOR "04[Error]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::versionFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VersionFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03Renegade X RCON connection established; using RCON verison " IRCBOLD "%.*s" IRCBOLD " for game version " IRCBOLD "%.*s" IRCBOLD, RenX::tags->rconVersionTag.size(), RenX::tags->rconVersionTag.ptr(), RenX::tags->gameVersionTag.size(), RenX::tags->gameVersionTag.ptr()));

	RenX_LoggingPlugin::authorizedFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AuthorizedFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03RCON authorization completed."));

	RenX_LoggingPlugin::otherFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("OtherFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[Other]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	/** Sanitize tags in formats to prevent explotation */
	RenX::sanitizeTags(joinPublicFmt);
	RenX::sanitizeTags(joinAdminFmt);
	RenX::sanitizeTags(joinNoSteamAdminFmt);
	RenX::sanitizeTags(partFmt);
	RenX::sanitizeTags(nameChangeFmt);
	RenX::sanitizeTags(teamChangeFmt);
	RenX::sanitizeTags(chatFmt);
	RenX::sanitizeTags(teamChatFmt);
	RenX::sanitizeTags(deployFmt);
	RenX::sanitizeTags(suicideFmt);
	RenX::sanitizeTags(dieFmt);
	RenX::sanitizeTags(dieFmt2);
	RenX::sanitizeTags(killFmt);
	RenX::sanitizeTags(killFmt2);
	RenX::sanitizeTags(destroyBuildingFmt);
	RenX::sanitizeTags(destroyBuildingFmt2);
	RenX::sanitizeTags(destroyDefenceFmt);
	RenX::sanitizeTags(destroyDefenceFmt2);
	RenX::sanitizeTags(destroyVehicleFmt);
	RenX::sanitizeTags(destroyVehicleFmt2);
	RenX::sanitizeTags(gameOverFmt);
	RenX::sanitizeTags(gameOverTieFmt);
	RenX::sanitizeTags(gameOverTieNoWinFmt);
	RenX::sanitizeTags(gameOverScoreFmt);
	RenX::sanitizeTags(gameFmt);
	RenX::sanitizeTags(executeFmt);
	RenX::sanitizeTags(evaFmt);
	RenX::sanitizeTags(evaPrivateFmt);
	RenX::sanitizeTags(subscribeFmt);
	RenX::sanitizeTags(rconFmt);
	RenX::sanitizeTags(adminLoginFmt);
	RenX::sanitizeTags(adminGrantFmt);
	RenX::sanitizeTags(adminLogoutFmt);
	RenX::sanitizeTags(adminFmt);
	RenX::sanitizeTags(logFmt);
	RenX::sanitizeTags(xVersionFmt);
	RenX::sanitizeTags(grantCharacterFmt);
	RenX::sanitizeTags(spawnVehicleFmt);
	RenX::sanitizeTags(spawnVehicleNoOwnerFmt);
	RenX::sanitizeTags(minePlaceFmt);
	RenX::sanitizeTags(xOtherFmt);
	RenX::sanitizeTags(commandFmt);
	RenX::sanitizeTags(errorFmt);
	RenX::sanitizeTags(versionFmt);
	RenX::sanitizeTags(authorizedFmt);
	RenX::sanitizeTags(otherFmt);
}

typedef void(RenX::Server::*logFuncType)(const Jupiter::ReadableString &msg) const;

void RenX_LoggingPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	Jupiter::String msg;
	if (RenX_LoggingPlugin::joinPublic)
	{
		msg = this->joinPublicFmt;
		if (msg.isEmpty() == false)
		{
			RenX::processTags(msg, server, player);
			server->sendPubChan(msg);
		}
	}
	if (player->steamid == 0)
		msg = this->joinNoSteamAdminFmt;
	else
		msg = this->joinAdminFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
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
		RenX::processTags(msg, server, player);
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
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_NEW_NAME_TAG, newPlayerName);
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
		RenX::processTags(msg, server, player);
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
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, message);
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
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, message);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDeploy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object)
{
	logFuncType func;
	if (RenX_LoggingPlugin::deployPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->deployFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(object));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::suicidePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->suicideFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::killPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->killFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player, victim);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnKill(RenX::Server *server, const Jupiter::ReadableString &killer, RenX::TeamType killerTeam, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::killPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->killFmt2;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, nullptr, victim);
		msg.replace(RenX::tags->INTERNAL_NAME_TAG, RenX::translateName(killer));
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(killerTeam));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(killerTeam));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(killerTeam));
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::diePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->dieFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDie(RenX::Server *server, const Jupiter::ReadableString &object, RenX::TeamType objectTeam, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::diePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->dieFmt2;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_NAME_TAG, RenX::translateName(object));
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(objectTeam));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(objectTeam));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(objectTeam));
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
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
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(victimTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(victimTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(victimTeam));
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(objectName));
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDestroy(RenX::Server *server, const Jupiter::ReadableString &killer, RenX::TeamType killerTeam, const Jupiter::ReadableString &objectName, RenX::TeamType objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
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
		msg = this->destroyBuildingFmt2;
		break;
	case RenX::ObjectType::Defence:
		msg = this->destroyDefenceFmt2;
		break;
	default:
		msg = this->destroyVehicleFmt2;
		break;
	}
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_NAME_TAG, RenX::translateName(killer));
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(killerTeam));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(killerTeam));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(killerTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(objectTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(objectTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(objectTeam));
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(objectName));
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
{
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
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_WIN_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
		msg.replace(RenX::tags->INTERNAL_LOSE_SCORE_TAG, Jupiter::StringS::Format("%d", loseScore));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(loserTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(loserTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(loserTeam));
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, RenX::translateWinType(winType));
		(server->*func)(msg);
	}

	msg = this->gameOverScoreFmt;
	if (team == RenX::TeamType::Other)
	{
		team = RenX::TeamType::GDI;
		loserTeam = RenX::TeamType::Nod;
	}

	if (msg.isEmpty() == false)
	{
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_WIN_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
		msg.replace(RenX::tags->INTERNAL_LOSE_SCORE_TAG, Jupiter::StringS::Format("%d", loseScore));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(loserTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(loserTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(loserTeam));
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, RenX::translateWinType(winType));
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
			msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, Jupiter::ReferenceString::gotoToken(command, 1, ' '));
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
				msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, message);
				(server->*func)(msg);
				return;
			}
		}
	}
	msg = this->executeFmt;
	if (msg.isEmpty() == false)
	{
		processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_NAME_TAG, user);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, command);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_NAME_TAG, user);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::adminLoginPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->adminLoginFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdminGrant(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::adminGrantPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->adminGrantFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::adminLogoutPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;
	
	Jupiter::String msg = this->adminLogoutFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_XOnVersion(RenX::Server *server, unsigned int version)
{
	logFuncType func;
	if (RenX_LoggingPlugin::xVersionPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->xVersionFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_XRCON_VERSION_TAG, Jupiter::StringS::Format("%u", version));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnGrantCharacter(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character)
{
	if (player->character.equals(RenX::getCharacter(character)) == false)
	{
		logFuncType func;
		if (RenX_LoggingPlugin::grantCharacterPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		Jupiter::String msg = this->grantCharacterFmt;
		if (msg.isEmpty() == false)
		{
			RenX::processTags(msg, server, player);
			msg.replace(RenX::tags->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(character));
			(server->*func)(msg);
		}
	}
}

void RenX_LoggingPlugin::RenX_OnSpawnVehicle(RenX::Server *server, const RenX::PlayerInfo *owner, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::spawnVehiclePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->spawnVehicleFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, owner);
		msg.replace(RenX::tags->INTERNAL_VICTIM_VEHICLE_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSpawnVehicleNoOwner(RenX::Server *server, const RenX::TeamType team, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::spawnVehicleNoOwnerPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->spawnVehicleNoOwnerFmt;
	if (msg.isEmpty() == false)
	{
		RenX::TeamType otherTeam = RenX::getEnemy(team);
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(otherTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(otherTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(otherTeam));
		msg.replace(RenX::tags->INTERNAL_VEHICLE_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnMinePlace(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &mine)
{
	logFuncType func;
	if (RenX_LoggingPlugin::minePlacePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->minePlaceFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(mine));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_XOnOther(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::xOtherPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->xOtherFmt;
	if (msg.isEmpty() == false)
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
