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

RenX_LoggingPlugin::RenX_LoggingPlugin()
{
	RenX_LoggingPlugin::joinPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("JoinPublic"), true);
	RenX_LoggingPlugin::partPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("PartPublic"), true);
	RenX_LoggingPlugin::nameChangePublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("NameChangePublic"), true);
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
	RenX_LoggingPlugin::adminLogoutPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminLogoutPublic"), true);
	RenX_LoggingPlugin::adminPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminPublic"), false);
	RenX_LoggingPlugin::logPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("LogPublic"), false);
	RenX_LoggingPlugin::commandPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("CommandPublic"), false);
	RenX_LoggingPlugin::errorPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ErrorPublic"), false);
	RenX_LoggingPlugin::versionPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("VersionPublic"), true);
	RenX_LoggingPlugin::authorizedPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AuthorizedPublic"), true);
	RenX_LoggingPlugin::otherPublic = Jupiter::IRC::Client::Config->getBool(RenX_LoggingPlugin::getName(), STRING_LITERAL_AS_REFERENCE("OtherPublic"), false);
}

typedef void(RenX::Server::*logFuncType)(const char *fmt, ...) const;

void RenX_LoggingPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &teamColor = RenX::getTeamColor(player->team);
	const Jupiter::ReadableString &teamName = RenX::getFullTeamName(player->team);

	if (RenX_LoggingPlugin::joinPublic)
		server->sendPubChan(IRCCOLOR "12[Join]" IRCCOLOR "%.*s " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s!", teamColor.size(), teamColor.ptr(), player->name.size(), player->name.ptr(), teamName.size(), teamName.ptr());
	if (player->steamid == 0)
		server->sendAdmChan(IRCCOLOR "12[Join]" IRCCOLOR "%.*s " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s from " IRCBOLD "%.*s" IRCBOLD ", but is not using Steam.", teamColor.size(), teamColor.ptr(), player->name.size(), player->name.ptr(), teamName.size(), teamName.ptr(), player->ip.size(), player->ip.ptr());
	else
	{
		const Jupiter::ReadableString &steamid = server->formatSteamID(player);
		server->sendAdmChan(IRCCOLOR "12[Join]" IRCCOLOR "%.*s " IRCBOLD "%.*s" IRCBOLD " joined the game fighting for the %.*s from " IRCBOLD "%.*s" IRCBOLD " using Steam ID " IRCBOLD "%.*s" IRCBOLD ".", teamColor.size(), teamColor.ptr(), player->name.size(), player->name.ptr(), teamName.size(), teamName.ptr(), player->ip.size(), player->ip.ptr(), steamid.size(), steamid.ptr());
	}
}

void RenX_LoggingPlugin::RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);
	const Jupiter::ReadableString &teamName = RenX::getFullTeamName(player->team);

	logFuncType func;
	if (RenX_LoggingPlugin::partPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "12[Part] " IRCBOLD "%.*s" IRCBOLD " has left the %.*s.", playerName.size(), playerName.ptr(), teamName.size(), teamName.ptr());
}

void RenX_LoggingPlugin::RenX_OnNameChange(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &newPlayerName)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::nameChangePublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCBOLD "%.*s" IRCBOLD " has changed their name to " IRCBOLD "%.*s" IRCBOLD ".", playerName.size(), playerName.ptr(), newPlayerName.size(), newPlayerName.ptr());
}

void RenX_LoggingPlugin::RenX_OnChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::chatPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCBOLD "%.*s" IRCCOLOR IRCBOLD ": %.*s", playerName.size(), playerName.ptr(), message.size(), message.ptr());
}

void RenX_LoggingPlugin::RenX_OnTeamChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::teamChatPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCBOLD "%.*s" IRCBOLD ": %.*s", playerName.size(), playerName.ptr(), message.size(), message.ptr());
}

void RenX_LoggingPlugin::RenX_OnDeploy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);
	const Jupiter::ReadableString &translated = RenX::translateName(object);

	logFuncType func;
	if (RenX_LoggingPlugin::deployPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCBOLD "%.*s" IRCBOLD " deployed a " IRCBOLD "%.*s" IRCBOLD, playerName.size(), playerName.ptr(), translated.size(), translated.ptr());
}

void RenX_LoggingPlugin::RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);

	logFuncType func;
	if (RenX_LoggingPlugin::suicidePublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)("%.*s" IRCCOLOR " suicided (death by " IRCCOLOR "12%.*s" IRCCOLOR ").", playerName.size(), playerName.ptr(), translated.size(), translated.ptr());
}

void RenX_LoggingPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);
	const Jupiter::ReadableString &victimName = RenX::getFormattedPlayerName(victim);
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);

	logFuncType func;
	if (RenX_LoggingPlugin::killPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)("%.*s" IRCCOLOR " killed %.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", playerName.size(), playerName.ptr(), victimName.size(), victimName.ptr(), translated.size(), translated.ptr());
}

void RenX_LoggingPlugin::RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);

	logFuncType func;
	if (RenX_LoggingPlugin::diePublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	if (damageType.equals("DmgType_Fell"))
		(server->*func)("%.*s" IRCCOLOR " fell to their death!", playerName.size(), playerName.ptr());
	else if (damageType.equals("DamageType"))
		(server->*func)("%.*s" IRCCOLOR " switched teams!", playerName.size(), playerName.ptr());
	else (server->*func)("%.*s" IRCCOLOR " died (" IRCCOLOR "12%.*s" IRCCOLOR ").", playerName.size(), playerName.ptr(), translated.size(), translated.ptr());
}

void RenX_LoggingPlugin::RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);
	const Jupiter::ReadableString &objectTranslated = RenX::translateName(objectName);
	const Jupiter::ReadableString &translated = RenX::translateName(damageType);
	const Jupiter::ReadableString &enemyColor = RenX::getTeamColor(RenX::getEnemy(player->team));

	logFuncType func;
	if (RenX_LoggingPlugin::destroyPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)("%.*s" IRCCOLOR " destroyed %s " IRCCOLOR "%.*s%.*s" IRCCOLOR " (" IRCCOLOR "12%.*s" IRCCOLOR ").", playerName.size(), playerName.ptr(), type == RenX::ObjectType::Building ? "the" : "a", enemyColor.size(), enemyColor.ptr(), objectTranslated.size(), objectTranslated.ptr(), translated.size(), translated.ptr());
}

void RenX_LoggingPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
{
	const Jupiter::ReadableString &win = RenX::translateWinType(winType);
	const Jupiter::ReadableString &teamColor = RenX::getTeamColor(team);
	const Jupiter::ReadableString &teamName = RenX::getFullTeamName(team);

	const Jupiter::ReadableString &gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	const Jupiter::ReadableString &gTeamName = RenX::getFullTeamName(RenX::TeamType::GDI);

	const Jupiter::ReadableString &nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	const Jupiter::ReadableString &nTeamName = RenX::getFullTeamName(RenX::TeamType::Nod);

	logFuncType func;
	if (RenX_LoggingPlugin::gameOverPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	if (winType == RenX::WinType::Tie)
	{
		if (team == RenX::TeamType::Other) (server->*func)(IRCCOLOR "03[Game]" IRCCOLOR "10 The battle ended in a " IRCBOLD "%.*s" IRCBOLD, win.size(), win.ptr());
		else (server->*func)(IRCCOLOR "03[Game]" IRCCOLOR "10 The battle ended in a " IRCBOLD "%.*s" IRCBOLD " - Victory handed to " IRCBOLD IRCCOLOR "%.*s%.*s" IRCBOLD, win.size(), win.ptr(), teamColor.size(), teamColor.ptr(), teamName.size(), teamName.ptr());
	}
	else (server->*func)(IRCCOLOR "03[Game]" IRCCOLOR "%.*s The " IRCBOLD "%.*s" IRCBOLD " won by " IRCBOLD "%.*s" IRCBOLD, teamColor.size(), teamColor.ptr(), teamName.size(), teamName.ptr(), win.size(), win.ptr());
	(server->*func)(IRCCOLOR "03[Game]" IRCCOLOR "%.*s %.*s" IRCCOLOR ": %d | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %d", gTeamColor.size(), gTeamColor.ptr(), gTeamName.size(), gTeamName.ptr(), gScore, nTeamColor.size(), nTeamColor.ptr(), nTeamName.size(), nTeamName.ptr(), nScore);
	if (server->profile->disconnectOnGameOver)
		(server->*func)(IRCCOLOR "07[Warning]" IRCCOLOR " Game version \"%.*s\" gets disconnected when a map unloads; to prevent disconnect spam, player disconnects are silenced until the bot reconnects.", server->getGameVersion().size(), server->getGameVersion().ptr());
}

void RenX_LoggingPlugin::RenX_OnGame(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::gamePublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "03[Game]" IRCCOLOR " %.*s", raw.size(), raw.ptr());
}

void RenX_LoggingPlugin::RenX_OnExecute(RenX::Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command)
{
	logFuncType func;
	if (RenX_LoggingPlugin::executePublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	if (command.matchi("say *"))
	{
		// Add an option for this later? Separate event?
		func = &RenX::Server::sendLogChan;
		(server->*func)(IRCCOLOR "09EVA: %.*s", command.size() - 4, command.ptr() + 4);
	}
	else if (command.matchi("evaprivatesay *"))
	{
		RenX::PlayerInfo *player = server->getPlayerByName(Jupiter::ReferenceString::getToken(command, 1, ' '));
		if (player != nullptr)
		{
			Jupiter::ReferenceString msg = Jupiter::ReferenceString::gotoToken(command, 2, ' ');
			(server->*func)(IRCCOLOR "12(EVA -> %.*s): %.*s", player->name.size(), player->name.ptr(), msg.size(), msg.ptr());
		}
		else (server->*func)(IRCCOLOR "07%.*s executed: %.*s", user.size(), user.ptr(), command.size(), command.ptr());
	}
	else (server->*func)(IRCCOLOR "07%.*s executed: %.*s", user.size(), user.ptr(), command.size(), command.ptr());
	// Add a format check later for if user == us.
}

void RenX_LoggingPlugin::RenX_OnSubscribe(RenX::Server *server, const Jupiter::ReadableString &user)
{
	logFuncType func;
	if (RenX_LoggingPlugin::subscribePublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "03%.*s has subscribed to the RCON data stream.", user.size(), user.ptr());
}

void RenX_LoggingPlugin::RenX_OnRCON(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::RCONPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "05[RCON]" IRCCOLOR " %.*s", raw.size(), raw.ptr());
}

void RenX_LoggingPlugin::RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::adminLoginPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " has logged in with " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", playerName.size(), playerName.ptr(), player->adminType.size(), player->adminType.ptr());
}

void RenX_LoggingPlugin::RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player)
{
	const Jupiter::ReadableString &playerName = RenX::getFormattedPlayerName(player);

	logFuncType func;
	if (RenX_LoggingPlugin::adminLogoutPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "07[Admin] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " has logged out of their " IRCCOLOR "07" IRCBOLD "%.*s" IRCBOLD IRCNORMAL " privledges.", playerName.size(), playerName.ptr(), player->adminType.size(), player->adminType.ptr());
}

void RenX_LoggingPlugin::RenX_OnAdmin(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::adminPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "07[Admin]" IRCCOLOR " %.*s", raw.size(), raw.ptr());
}

void RenX_LoggingPlugin::RenX_OnLog(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::logPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "07[Log]" IRCCOLOR " %.*s", raw.size(), raw.ptr());
}

void RenX_LoggingPlugin::RenX_OnCommand(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::commandPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	// This is potentially useful later, but not yet!
	//sendAdmChan(server, "[Command] %.*s", raw.size(), raw.ptr());
}

void RenX_LoggingPlugin::RenX_OnError(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::errorPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "04[Error]" IRCCOLOR " %.*s", raw.size(), raw.ptr());
}

void RenX_LoggingPlugin::RenX_OnVersion(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::versionPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "03Renegade X RCON connection established; using RCON verison " IRCBOLD "%d" IRCBOLD " for game version " IRCBOLD "%.*s" IRCBOLD, raw.asInt(10), raw.size() - 3, raw.ptr() + 3);
}

void RenX_LoggingPlugin::RenX_OnAuthorized(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::authorizedPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "03RCON authorization completed.");
}

void RenX_LoggingPlugin::RenX_OnOther(RenX::Server *server, char token, const Jupiter::ReadableString &raw)
{
	logFuncType func;
	if (RenX_LoggingPlugin::otherPublic) func = &RenX::Server::sendLogChan;
	else func = &RenX::Server::sendAdmChan;
	(server->*func)(IRCCOLOR "06[Other]" IRCCOLOR " %.*s", raw.size(), raw.ptr());
}

// Plugin instantiation and entry point.
RenX_LoggingPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
