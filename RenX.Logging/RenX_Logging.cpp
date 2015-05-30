/**
 * Copyright (C) 2014-2015 Justin James.
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
	RenX_LoggingPlugin::muteOwnExecute = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MuteOwnExecute"), true);
	RenX_LoggingPlugin::joinPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("JoinPublic"), true);
	RenX_LoggingPlugin::partPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("PartPublic"), true);
	RenX_LoggingPlugin::kickPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("KickPublic"), true);
	RenX_LoggingPlugin::nameChangePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("NameChangePublic"), true);
	RenX_LoggingPlugin::teamChangePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TeamChangePublic"), true);
	RenX_LoggingPlugin::playerPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("PlayerPublic"), false);
	RenX_LoggingPlugin::chatPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ChatPublic"), true);
	RenX_LoggingPlugin::teamChatPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TeamChatPublic"), true);
	RenX_LoggingPlugin::hostChatPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("HostChatPublic"), true);
	RenX_LoggingPlugin::hostPagePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("HostPagePublic"), false);
	RenX_LoggingPlugin::otherChatPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("OtherChatPublic"), false);
	RenX_LoggingPlugin::deployPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DeployPublic"), true);
	RenX_LoggingPlugin::mineDeployPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MineDeployPublic"), false);
	RenX_LoggingPlugin::disarmPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DisarmPublic"), true);
	RenX_LoggingPlugin::mineDisarmPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MineDisarmPublic"), false);
	RenX_LoggingPlugin::explodePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ExplodePublic"), false);
	RenX_LoggingPlugin::suicidePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SuicidePublic"), true);
	RenX_LoggingPlugin::killPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("KillPublic"), true);
	RenX_LoggingPlugin::diePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DiePublic"), true);
	RenX_LoggingPlugin::destroyPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DestroyPublic"), true);
	RenX_LoggingPlugin::capturePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("CapturePublic"), true);
	RenX_LoggingPlugin::neutralizePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("NeutralizePublic"), true);
	RenX_LoggingPlugin::characterPurchasePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("CharacterPurchasePublic"), false);
	RenX_LoggingPlugin::itemPurchasePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ItemPurchasePublic"), false);
	RenX_LoggingPlugin::weaponPurchasePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("WeaponPurchasePublic"), false);
	RenX_LoggingPlugin::refillPurchasePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("RefillPurchasePublic"), false);
	RenX_LoggingPlugin::vehiclePurchasePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VehiclePurchasePublic"), false);
	RenX_LoggingPlugin::vehicleSpawnPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VehicleSpawnPublic"), true);
	RenX_LoggingPlugin::spawnPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SpawnPublic"), true);
	RenX_LoggingPlugin::botJoinPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("BotJoinPublic"), true);
	RenX_LoggingPlugin::vehicleCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VehicleCratePublic"), false);
	RenX_LoggingPlugin::TSVehicleCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TSVehicleCratePublic"), RenX_LoggingPlugin::vehicleCratePublic);
	RenX_LoggingPlugin::RAVehicleCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("RAVehicleCratePublic"), RenX_LoggingPlugin::vehicleCratePublic);
	RenX_LoggingPlugin::deathCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DeathCratePublic"), true);
	RenX_LoggingPlugin::moneyCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MoneyCratePublic"), false);
	RenX_LoggingPlugin::characterCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("CharacterCratePublic"), false);
	RenX_LoggingPlugin::spyCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SpyCratePublic"), false);
	RenX_LoggingPlugin::refillCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("RefillCratePublic"), false);
	RenX_LoggingPlugin::timeBombCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TimeBombCratePublic"), false);
	RenX_LoggingPlugin::speedCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SpeedCratePublic"), false);
	RenX_LoggingPlugin::nukeCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("NukeCratePublic"), true);
	RenX_LoggingPlugin::abductionCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AbductionCratePublic"), true);
	RenX_LoggingPlugin::unspecifiedCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("UnspecifiedCratePublic"), false);
	RenX_LoggingPlugin::otherCratePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("OtherCratePublic"), false);
	RenX_LoggingPlugin::stealPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("StealPublic"), true);
	RenX_LoggingPlugin::donatePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DonatePublic"), true);
	RenX_LoggingPlugin::gamePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("GamePublic"), true);
	RenX_LoggingPlugin::gameOverPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("GameOverPublic"), true);
	RenX_LoggingPlugin::executePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ExecutePublic"), false);
	RenX_LoggingPlugin::subscribePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("SubscribePublic"), false);
	RenX_LoggingPlugin::RCONPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("RCONPublic"), false);
	RenX_LoggingPlugin::adminLoginPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminLoginPublic"), true);
	RenX_LoggingPlugin::adminGrantPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminGrantPublic"), true);
	RenX_LoggingPlugin::adminLogoutPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminLogoutPublic"), true);
	RenX_LoggingPlugin::adminPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminPublic"), false);
	RenX_LoggingPlugin::voteCallPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VoteCallPublic"), true);
	RenX_LoggingPlugin::voteOverPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VoteOverPublic"), true);
	RenX_LoggingPlugin::voteCancelPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VoteCancelPublic"), true);
	RenX_LoggingPlugin::votePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VotePublic"), false);
	RenX_LoggingPlugin::mapChangePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MapChangePublic"), true);
	RenX_LoggingPlugin::mapLoadPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MapLoadPublic"), true);
	RenX_LoggingPlugin::mapStartPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MapStartPublic"), true);
	RenX_LoggingPlugin::mapPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MapPublic"), false);
	RenX_LoggingPlugin::demoRecordPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DemoRecordPublic"), true);
	RenX_LoggingPlugin::demoRecordStopPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DemoRecordStopPublic"), true);
	RenX_LoggingPlugin::demoPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("DemoPublic"), false);
	RenX_LoggingPlugin::logPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("LogPublic"), false);
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

	RenX_LoggingPlugin::kickFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("KickFormat"),
		Jupiter::StringS::Format(IRCCOLOR "04[Kick] " IRCBOLD "%.*s" IRCCOLOR IRCBOLD " was " IRCBOLD IRCCOLOR "04kicked" IRCCOLOR IRCBOLD " from the server for \"" IRCCOLOR "04%.*s" IRCCOLOR "\".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::playerExecuteFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("PlayerExecuteFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR "07 executed: %.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::playerFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("PlayerFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12[Player]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::nameChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NameChangeFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD " changed their name to " IRCBOLD "%.*s" IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->newNameTag.size(), RenX::tags->newNameTag.ptr()));

	RenX_LoggingPlugin::teamChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamChangeFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " switched teams!", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::chatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ChatFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD ": %.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::teamChatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TeamChatFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD ": %.*s", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::hostChatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("HostChatFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12Host" IRCCOLOR "0: %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::hostPageFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("HostPageFormat"),
		Jupiter::StringS::Format(IRCCOLOR "12(Host -> %.*s): %.*s", RenX::tags->rawNameTag.size(), RenX::tags->rawNameTag.ptr(), RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::otherChatFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("OtherChatFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[Other Chat]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::deployFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DeployFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " deployed a " IRCBOLD IRCCOLOR "12%.*s" IRCBOLD, RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::mineDeployFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MineDeployFormat"),
		RenX_LoggingPlugin::deployFmt);

	RenX_LoggingPlugin::disarmFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DisarmFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " disarmed %.*s" IRCBOLD IRCCOLOR "'s " IRCCOLOR "12%.*s" IRCBOLD, RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimNameTag.size(), RenX::tags->victimNameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::mineDisarmFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MineDisarmFormat"),
		RenX_LoggingPlugin::disarmFmt);

	RenX_LoggingPlugin::disarmNoOwnerFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DisarmNoOwnerFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCBOLD " disarmed a " IRCBOLD "%.*s" IRCBOLD, RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::mineDisarmNoOwnerFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MineDisarmNoOwnerFormat"),
		RenX_LoggingPlugin::disarmNoOwnerFmt);

	RenX_LoggingPlugin::explodeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ExplodeFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " detonated a " IRCCOLOR "07%.*s" IRCCOLOR ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::explodeNoOwnerFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ExplodeMoOwnerFormat"),
		Jupiter::StringS::Format("A " IRCCOLOR "07%.*s" IRCCOLOR " detonated.", RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::suicideFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SuicideFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " suicided (" IRCCOLOR "12%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::killFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("KillFormat"),
		Jupiter::StringS::Format("%.*s" IRCCOLOR " killed %.*s" IRCCOLOR " (" IRCCOLOR "%.*s%.*s/%.*s" IRCCOLOR " vs " IRCCOLOR "%.*s%.*s" IRCCOLOR ").", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimNameTag.size(), RenX::tags->victimNameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->characterTag.size(), RenX::tags->characterTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->victimCharacterTag.size(), RenX::tags->victimCharacterTag.ptr()));

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

	RenX_LoggingPlugin::captureFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("CaptureFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " captured the " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::neutralizeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NeutralizeFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " neutralized the " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::characterPurchaseFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("CharacterPurchaseFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " purchased a " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->victimCharacterTag.size(), RenX::tags->victimCharacterTag.ptr()));

	RenX_LoggingPlugin::itemPurchaseFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ItemPurchaseFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " purchased a " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::weaponPurchaseFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("WeaponPurchaseFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " purchased a " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->weaponTag.size(), RenX::tags->weaponTag.ptr()));

	RenX_LoggingPlugin::refillPurchaseFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RefillPurchaseFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " purchased a " IRCBOLD IRCCOLOR "%.*srefill" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr()));

	RenX_LoggingPlugin::vehiclePurchaseFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VehiclePurchaseFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " purchased a " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->victimVehicleTag.size(), RenX::tags->victimVehicleTag.ptr()));

	RenX_LoggingPlugin::vehicleSpawnFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VehicleSpawnFormat"),
		Jupiter::StringS::Format("A " IRCBOLD IRCCOLOR "%.*s%.*s" IRCCOLOR IRCBOLD " has spawned.", RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->vehicleTag.size(), RenX::tags->vehicleTag.ptr()));

	RenX_LoggingPlugin::spawnFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SpawnFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " spawned as a " IRCCOLOR "%.*s%.*s.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->victimCharacterTag.size(), RenX::tags->victimCharacterTag.ptr()));

	RenX_LoggingPlugin::botJoinFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("BotJoinFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " online.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::vehicleCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VehicleCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a %.*s vehicle crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::TSVehicleCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TSVehicleCrateFormat"),
		RenX_LoggingPlugin::vehicleCrateFmt);

	RenX_LoggingPlugin::RAVehicleCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RAVehicleCrateFormat"),
		RenX_LoggingPlugin::vehicleCrateFmt);

	RenX_LoggingPlugin::deathCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DeathCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "12death" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::moneyCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MoneyCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up " IRCCOLOR "09%.*s credits" IRCCOLOR " from a " IRCCOLOR "12money" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::characterCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("CharacterCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "%.*s%.*s" IRCCOLOR " " IRCCOLOR "12character" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr(), RenX::tags->victimCharacterTag.size(), RenX::tags->victimCharacterTag.ptr()));

	RenX_LoggingPlugin::spyCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SpyCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "%.*s%.*s" IRCCOLOR " " IRCCOLOR "12spy" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->victimCharacterTag.size(), RenX::tags->victimCharacterTag.ptr()));

	RenX_LoggingPlugin::refillCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RefillCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "%.*srefill" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->teamColorTag.size(), RenX::tags->teamColorTag.ptr()));

	RenX_LoggingPlugin::timeBombCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TimeBombCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "11time-bomb" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::speedCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("SpeedCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "11speed" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::nukeCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NukeCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "04nuke" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::abductionCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("AbductionCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " has been " IRCBOLD IRCCOLOR "06abducted" IRCCOLOR IRCBOLD " by the " IRCBOLD IRCCOLOR "06Scrin" IRCCOLOR IRCBOLD "!", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::unspecifiedCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("UnspecifiedCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up an " IRCCOLOR "13unspecified" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::otherCrateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("OtherCrateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " picked up a " IRCCOLOR "13%.*s" IRCCOLOR " crate.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::stealFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("StealFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " stole " IRCBOLD "%.*s" IRCBOLD "'s " IRCBOLD "%.*s" IRCBOLD "!", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->victimNameTag.size(), RenX::tags->victimNameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::stealNoOwnerFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("StealNoOwnerFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " stole a " IRCBOLD IRCCOLOR "12%.*s" IRCBOLD "!", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::donateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DonateFormat"),
		Jupiter::StringS::Format(IRCBOLD "%.*s" IRCCOLOR IRCBOLD " donated " IRCCOLOR "09%.*s credits" IRCCOLOR " to " IRCBOLD "%.*s" IRCBOLD ".", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->victimNameTag.size(), RenX::tags->victimNameTag.ptr()));

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

	RenX_LoggingPlugin::voteCallFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VoteCallFormat"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s[Vote] " IRCBOLD "%.*s" IRCBOLD " has called a \"%.*s\" vote.", RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr()));

	RenX_LoggingPlugin::voteOverSuccessFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VoteOverSuccessFormat"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s[Vote] A vote for \"%.*s\" " IRCBOLD IRCCOLOR "09passed" IRCBOLD IRCCOLOR "%.*s (Votes Yes: %.*s | Votes No: %.*s).", RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->winScoreTag.size(), RenX::tags->winScoreTag.ptr(), RenX::tags->loseScoreTag.size(), RenX::tags->loseScoreTag.ptr()));

	RenX_LoggingPlugin::voteOverFailFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VoteOverFailFormat"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s[Vote] A vote for \"%.*s\" " IRCBOLD IRCCOLOR "04failed" IRCBOLD IRCCOLOR "%.*s (Votes Yes: %.*s | Votes No: %.*s).", RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr(), RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->winScoreTag.size(), RenX::tags->winScoreTag.ptr(), RenX::tags->loseScoreTag.size(), RenX::tags->loseScoreTag.ptr()));

	RenX_LoggingPlugin::voteCancelFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VoteCancelFormat"),
		Jupiter::StringS::Format(IRCCOLOR "%.*s[Vote] A vote for \"%.*s\" was " IRCBOLD IRCCOLOR "07cancelled" IRCCOLOR IRCBOLD ".", RenX::tags->victimTeamColorTag.size(), RenX::tags->victimTeamColorTag.ptr(), RenX::tags->objectTag.size(), RenX::tags->objectTag.ptr()));

	RenX_LoggingPlugin::voteFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("VoteFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[Vote]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::mapChangeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MapChangeFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03Loading %.*s...", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::mapLoadFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MapLoadFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03%.*s loaded.", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::mapStartFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MapStartFormat"),
		Jupiter::StringS::Format(IRCCOLOR "03%.*s started.", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::mapFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MapFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[Map]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::demoRecordFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DemoRecordFormat"),
		Jupiter::StringS::Format("%.*s has started a demo recording.", RenX::tags->nameTag.size(), RenX::tags->nameTag.ptr()));

	RenX_LoggingPlugin::rconDemoRecordFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RCONDemoRecordFormat"),
		STRING_LITERAL_AS_REFERENCE(IRCCOLOR "07A demo recording has started."));

	RenX_LoggingPlugin::demoRecordStopFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DemoRecordStopFormat"),
		STRING_LITERAL_AS_REFERENCE(IRCCOLOR "07The demo recording has stopped."));

	RenX_LoggingPlugin::demoFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DemoFormat"),
		Jupiter::StringS::Format(IRCCOLOR "06[Demo]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

	RenX_LoggingPlugin::logFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("LogFormat"),
		Jupiter::StringS::Format(IRCCOLOR "07[Log]" IRCCOLOR " %.*s", RenX::tags->messageTag.size(), RenX::tags->messageTag.ptr()));

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

	/** Sanitize tags */
	RenX::sanitizeTags(joinPublicFmt);
	RenX::sanitizeTags(joinAdminFmt);
	RenX::sanitizeTags(joinNoSteamAdminFmt);
	RenX::sanitizeTags(partFmt);
	RenX::sanitizeTags(kickFmt);
	RenX::sanitizeTags(nameChangeFmt);
	RenX::sanitizeTags(teamChangeFmt);
	RenX::sanitizeTags(playerFmt);
	RenX::sanitizeTags(chatFmt);
	RenX::sanitizeTags(teamChatFmt);
	RenX::sanitizeTags(otherChatFmt);
	RenX::sanitizeTags(deployFmt);
	RenX::sanitizeTags(mineDeployFmt);
	RenX::sanitizeTags(disarmFmt);
	RenX::sanitizeTags(mineDisarmFmt);
	RenX::sanitizeTags(disarmNoOwnerFmt);
	RenX::sanitizeTags(mineDisarmNoOwnerFmt);
	RenX::sanitizeTags(explodeFmt);
	RenX::sanitizeTags(explodeNoOwnerFmt);
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
	RenX::sanitizeTags(captureFmt);
	RenX::sanitizeTags(neutralizeFmt);
	RenX::sanitizeTags(characterPurchaseFmt);
	RenX::sanitizeTags(itemPurchaseFmt);
	RenX::sanitizeTags(weaponPurchaseFmt);
	RenX::sanitizeTags(refillPurchaseFmt);
	RenX::sanitizeTags(vehiclePurchaseFmt);
	RenX::sanitizeTags(vehicleSpawnFmt);
	RenX::sanitizeTags(spawnFmt);
	RenX::sanitizeTags(botJoinFmt);
	RenX::sanitizeTags(vehicleCrateFmt);
	RenX::sanitizeTags(TSVehicleCrateFmt);
	RenX::sanitizeTags(RAVehicleCrateFmt);
	RenX::sanitizeTags(deathCrateFmt);
	RenX::sanitizeTags(moneyCrateFmt);
	RenX::sanitizeTags(characterCrateFmt);
	RenX::sanitizeTags(spyCrateFmt);
	RenX::sanitizeTags(refillCrateFmt);
	RenX::sanitizeTags(timeBombCrateFmt);
	RenX::sanitizeTags(speedCrateFmt);
	RenX::sanitizeTags(nukeCrateFmt);
	RenX::sanitizeTags(abductionCrateFmt);
	RenX::sanitizeTags(unspecifiedCrateFmt);
	RenX::sanitizeTags(otherCrateFmt);
	RenX::sanitizeTags(stealFmt);
	RenX::sanitizeTags(stealNoOwnerFmt);
	RenX::sanitizeTags(donateFmt);
	RenX::sanitizeTags(gameOverFmt);
	RenX::sanitizeTags(gameOverTieFmt);
	RenX::sanitizeTags(gameOverTieNoWinFmt);
	RenX::sanitizeTags(gameOverScoreFmt);
	RenX::sanitizeTags(gameFmt);
	RenX::sanitizeTags(executeFmt);
	RenX::sanitizeTags(playerExecuteFmt);
	RenX::sanitizeTags(hostChatFmt);
	RenX::sanitizeTags(hostPageFmt);
	RenX::sanitizeTags(subscribeFmt);
	RenX::sanitizeTags(rconFmt);
	RenX::sanitizeTags(adminLoginFmt);
	RenX::sanitizeTags(adminGrantFmt);
	RenX::sanitizeTags(adminLogoutFmt);
	RenX::sanitizeTags(adminFmt);
	RenX::sanitizeTags(voteCallFmt);
	RenX::sanitizeTags(voteOverSuccessFmt);
	RenX::sanitizeTags(voteOverFailFmt);
	RenX::sanitizeTags(voteCancelFmt);
	RenX::sanitizeTags(voteFmt);
	RenX::sanitizeTags(mapChangeFmt);
	RenX::sanitizeTags(mapLoadFmt);
	RenX::sanitizeTags(mapStartFmt);
	RenX::sanitizeTags(mapFmt);
	RenX::sanitizeTags(demoRecordFmt);
	RenX::sanitizeTags(rconDemoRecordFmt);
	RenX::sanitizeTags(demoRecordStopFmt);
	RenX::sanitizeTags(demoFmt);
	RenX::sanitizeTags(logFmt);
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
		if (msg.isNotEmpty())
		{
			RenX::processTags(msg, server, player);
			server->sendPubChan(msg);
		}
	}
	if (player->steamid == 0)
		msg = this->joinNoSteamAdminFmt;
	else
		msg = this->joinAdminFmt;
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnKick(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason)
{
	logFuncType func;
	if (RenX_LoggingPlugin::kickPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->kickFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, reason);
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_NEW_NAME_TAG, newPlayerName);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnTeamChange(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::TeamType &oldTeam)
{
	logFuncType func;
	if (RenX_LoggingPlugin::teamChangePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->teamChangeFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnExecute(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &command)
{
	logFuncType func;
	if (RenX_LoggingPlugin::executePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->playerExecuteFmt;
	if (msg.isNotEmpty())
	{
		processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, command);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnPlayer(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::playerPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->playerFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, message);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnHostChat(RenX::Server *server, const Jupiter::ReadableString &message)
{
	logFuncType func;
	if (RenX_LoggingPlugin::hostChatPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->hostChatFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, message);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnHostPage(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	logFuncType func;
	if (RenX_LoggingPlugin::hostPagePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->hostPageFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, message);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnOtherChat(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::otherChatPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->otherChatFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDeploy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object)
{
	logFuncType func;
	Jupiter::String msg;
	if (object.match("*Beacon"))
	{
		if (RenX_LoggingPlugin::deployPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		msg = this->deployFmt;
	}
	else
	{
		if (RenX_LoggingPlugin::mineDeployPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		msg = this->mineDeployFmt;
	}
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(object));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDisarm(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object, const RenX::PlayerInfo *victim)
{
	logFuncType func;
	Jupiter::String msg;

	if (object.match("*Beacon"))
	{
		if (RenX_LoggingPlugin::disarmPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		msg = this->disarmFmt;
	}
	else
	{
		if (RenX_LoggingPlugin::mineDisarmPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		msg = this->mineDisarmFmt;
	}
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player, victim);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(object));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDisarm(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object)
{
	logFuncType func;
	Jupiter::String msg;

	if (object.match("*Beacon"))
	{
		if (RenX_LoggingPlugin::disarmPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		msg = this->disarmNoOwnerFmt;
	}
	else
	{
		if (RenX_LoggingPlugin::mineDisarmPublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		msg = this->mineDisarmNoOwnerFmt;
	}
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(object));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnExplode(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object)
{
	logFuncType func;
	if (RenX_LoggingPlugin::explodePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->explodeFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(object));
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(object));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnExplode(RenX::Server *server, const Jupiter::ReadableString &object)
{
	logFuncType func;
	if (RenX_LoggingPlugin::explodePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->explodeNoOwnerFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(object));
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player, victim);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnKill(RenX::Server *server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::killPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->killFmt2;
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDie(RenX::Server *server, const Jupiter::ReadableString &object, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType)
{
	logFuncType func;
	if (RenX_LoggingPlugin::diePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->dieFmt2;
	if (msg.isNotEmpty())
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

void RenX_LoggingPlugin::RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(objectTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(objectTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(objectTeam));
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(objectName));
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(damageType));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnCapture(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &building, const RenX::TeamType &oldTeam)
{
	logFuncType func;
	if (RenX_LoggingPlugin::capturePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->captureFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(building));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(oldTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(oldTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(oldTeam));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnNeutralize(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &building, const RenX::TeamType &oldTeam)
{
	logFuncType func;
	if (RenX_LoggingPlugin::neutralizePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->neutralizeFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(building));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(oldTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(oldTeam));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(oldTeam));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnCharacterPurchase(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character)
{
	logFuncType func;
	if (RenX_LoggingPlugin::characterPurchasePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->characterPurchaseFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(character));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnItemPurchase(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &item)
{
	logFuncType func;
	if (RenX_LoggingPlugin::itemPurchasePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->itemPurchaseFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(item));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnWeaponPurchase(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &weapon)
{
	logFuncType func;
	if (RenX_LoggingPlugin::weaponPurchasePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->weaponPurchaseFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_WEAPON_TAG, RenX::translateName(weapon));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnRefillPurchase(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::refillPurchasePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->refillPurchaseFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVehiclePurchase(RenX::Server *server, const RenX::PlayerInfo *owner, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::vehiclePurchasePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->vehiclePurchaseFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, owner);
		msg.replace(RenX::tags->INTERNAL_VICTIM_VEHICLE_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVehicleSpawn(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::vehicleSpawnPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->vehicleSpawnFmt;
	if (msg.isNotEmpty())
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

void RenX_LoggingPlugin::RenX_OnSpawn(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character)
{
	logFuncType func;
	if (RenX_LoggingPlugin::spawnPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->spawnFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(character));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnBotJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::botJoinPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->botJoinFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVehicleCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::vehicleCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->vehicleCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnTSVehicleCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::TSVehicleCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->TSVehicleCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnRAVehicleCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::RAVehicleCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->RAVehicleCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDeathCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::deathCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->deathCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnMoneyCrate(RenX::Server *server, const RenX::PlayerInfo *player, int amount)
{
	logFuncType func;
	if (RenX_LoggingPlugin::moneyCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->moneyCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, Jupiter::StringS::Format("%d", amount));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnCharacterCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character)
{
	logFuncType func;
	if (RenX_LoggingPlugin::characterCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->characterCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(character));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSpyCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character)
{
	logFuncType func;
	if (RenX_LoggingPlugin::spyCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->spyCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(RenX::getEnemy(player->team)));
		msg.replace(RenX::tags->INTERNAL_VICTIM_CHARACTER_TAG, RenX::translateName(character));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnRefillCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::refillCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->refillCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnTimeBombCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::timeBombCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->timeBombCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSpeedCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::speedCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->speedCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnNukeCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::nukeCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->nukeCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnAbductionCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::abductionCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->abductionCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnUnspecifiedCrate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::unspecifiedCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->unspecifiedCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnOtherCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &type)
{
	logFuncType func;
	if (RenX_LoggingPlugin::otherCratePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->otherCrateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, type);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSteal(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle)
{
	logFuncType func;
	if (RenX_LoggingPlugin::stealPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->stealNoOwnerFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnSteal(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle, const RenX::PlayerInfo *victim)
{
	logFuncType func;
	if (RenX_LoggingPlugin::stealPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->stealFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player, victim);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(vehicle));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDonate(RenX::Server *server, const RenX::PlayerInfo *donor, const RenX::PlayerInfo *player, double amount)
{
	logFuncType func;
	if (RenX_LoggingPlugin::donatePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->donateFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, donor, player);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, Jupiter::StringS::Format("%.0f", amount));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDestroy(RenX::Server *server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const Jupiter::ReadableString &objectName, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
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
	if (msg.isNotEmpty())
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

void RenX_LoggingPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore)
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
		if (team == RenX::TeamType::None)
			msg = this->gameOverTieNoWinFmt;
		else
			msg = this->gameOverTieFmt;
	}
	else
		msg = this->gameOverFmt;

	if (msg.isNotEmpty() && team != RenX::TeamType::None)
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
	if (team == RenX::TeamType::None)
	{
		if (msg.isNotEmpty())
		{
			msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(RenX::TeamType::GDI));
			msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(RenX::TeamType::GDI));
			msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(RenX::TeamType::GDI));
			msg.replace(RenX::tags->INTERNAL_WIN_SCORE_TAG, Jupiter::StringS::Format("%d", winScore));
			msg.replace(RenX::tags->INTERNAL_LOSE_SCORE_TAG, Jupiter::StringS::Format("%d", loseScore));
			msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(RenX::TeamType::Nod));
			msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(RenX::TeamType::Nod));
			msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(RenX::TeamType::Nod));
			msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, RenX::translateWinType(winType));
			(server->*func)(msg);
		}
	}
	else if (msg.isNotEmpty())
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
}

void RenX_LoggingPlugin::RenX_OnGame(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::gamePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->gameFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnExecute(RenX::Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command)
{
	if (RenX_LoggingPlugin::muteOwnExecute == false || server->getUser().equals(user) == false)
	{
		logFuncType func;
		if (RenX_LoggingPlugin::executePublic)
			func = &RenX::Server::sendLogChan;
		else
			func = &RenX::Server::sendAdmChan;

		Jupiter::String msg = this->executeFmt;
		if (msg.isNotEmpty())
		{
			processTags(msg, server);
			msg.replace(RenX::tags->INTERNAL_NAME_TAG, user);
			msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, command);
			(server->*func)(msg);
		}
	}
}

void RenX_LoggingPlugin::RenX_OnSubscribe(RenX::Server *server, const Jupiter::ReadableString &user)
{
	logFuncType func;
	if (RenX_LoggingPlugin::subscribePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->subscribeFmt;
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVoteCall(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &type, const RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	logFuncType func;
	if (RenX_LoggingPlugin::voteCallPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->voteCallFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(type));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVoteOver(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &type, bool success, int yesVotes, int noVotes)
{
	logFuncType func;
	if (RenX_LoggingPlugin::voteOverPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = success ? this->voteOverSuccessFmt : this->voteOverFailFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(type));
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_WIN_SCORE_TAG, Jupiter::StringS::Format("%d", yesVotes));
		msg.replace(RenX::tags->INTERNAL_LOSE_SCORE_TAG, Jupiter::StringS::Format("%d", noVotes));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVoteCancel(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &type)
{
	logFuncType func;
	if (RenX_LoggingPlugin::voteCancelPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->voteCancelFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_OBJECT_TAG, RenX::translateName(type));
		msg.replace(RenX::tags->INTERNAL_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_COLOR_TAG, RenX::getTeamColor(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_SHORT_TAG, RenX::getTeamName(team));
		msg.replace(RenX::tags->INTERNAL_VICTIM_TEAM_LONG_TAG, RenX::getFullTeamName(team));
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnVote(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::votePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->voteFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnMapChange(RenX::Server *server, const Jupiter::ReadableString &map, bool seamless)
{
	logFuncType func;
	if (RenX_LoggingPlugin::mapChangePublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->mapChangeFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, map);
		(server->*func)(msg);
	}
	if (server->isSeamless() == false)
		server->sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " This server is using non-seamless server travel; this bot will be disconnected.");
}

void RenX_LoggingPlugin::RenX_OnMapLoad(RenX::Server *server, const Jupiter::ReadableString &map)
{
	logFuncType func;
	if (RenX_LoggingPlugin::mapLoadPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->mapLoadFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, map);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnMapStart(RenX::Server *server, const Jupiter::ReadableString &map)
{
	logFuncType func;
	if (RenX_LoggingPlugin::mapStartPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->mapStartFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, map);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnMap(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::mapPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->mapFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->INTERNAL_MESSAGE_TAG, raw);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDemoRecord(RenX::Server *server, const RenX::PlayerInfo *player)
{
	logFuncType func;
	if (RenX_LoggingPlugin::demoRecordPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->demoRecordFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server, player);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDemoRecord(RenX::Server *server, const Jupiter::ReadableString &user)
{
	logFuncType func;
	if (RenX_LoggingPlugin::demoRecordPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->rconDemoRecordFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		msg.replace(RenX::tags->nameTag, user);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDemoRecordStop(RenX::Server *server)
{
	logFuncType func;
	if (RenX_LoggingPlugin::demoRecordStopPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->demoRecordStopFmt;
	if (msg.isNotEmpty())
	{
		RenX::processTags(msg, server);
		(server->*func)(msg);
	}
}

void RenX_LoggingPlugin::RenX_OnDemo(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::demoPublic)
		func = &RenX::Server::sendLogChan;
	else
		func = &RenX::Server::sendAdmChan;

	Jupiter::String msg = this->demoFmt;
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
	if (msg.isNotEmpty())
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
