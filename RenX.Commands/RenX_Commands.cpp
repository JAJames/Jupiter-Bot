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

#include <functional>
#include "Jupiter/Functions.h"
#include "Jupiter/SLList.h"
#include "IRC_Bot.h"
#include "RenX_Commands.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BuildingInfo.h"
#include "RenX_Functions.h"
#include "RenX_BanDatabase.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;

inline bool togglePhasing(RenX::Server *server, bool newState)
{
	server->varData.set(STRING_LITERAL_AS_REFERENCE("RenX.Commands"), STRING_LITERAL_AS_REFERENCE("phasing"), newState ? STRING_LITERAL_AS_REFERENCE("true") : STRING_LITERAL_AS_REFERENCE("false"));
	return newState;
}

inline bool togglePhasing(RenX::Server *server)
{
	return togglePhasing(server, !server->varData.getBool(STRING_LITERAL_AS_REFERENCE("RenX.Commands"), STRING_LITERAL_AS_REFERENCE("phasing"), false));
}

inline void onDie(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->isBot && server->varData.getBool(STRING_LITERAL_AS_REFERENCE("RenX.Commands"), STRING_LITERAL_AS_REFERENCE("phasing"), false))
		server->kickPlayer(player, Jupiter::StringS::empty);
}

bool RenX_CommandsPlugin::RenX_OnBan(RenX::Server *server, const RenX::PlayerInfo *player, Jupiter::StringType &data)
{
	data = player->varData.get(this->getName(), STRING_LITERAL_AS_REFERENCE("banner"));
	return !data.isEmpty();
}

void RenX_CommandsPlugin::RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &)
{
	onDie(server, player);
}

void RenX_CommandsPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &)
{
	onDie(server, victim);
}

void RenX_CommandsPlugin::RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &)
{
	onDie(server, player);
}

int RenX_CommandsPlugin::OnRehash()
{
	RenX_CommandsPlugin::_defaultTempBanTime = Jupiter::IRC::Client::Config->getLongLong(RenX_CommandsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("TBanTime"), 86400);
	RenX_CommandsPlugin::playerInfoFormat = Jupiter::IRC::Client::Config->get(RenX_CommandsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("PlayerInfoFormat"), STRING_LITERAL_AS_REFERENCE(IRCCOLOR "03[Player Info]" IRCCOLOR "{TCOLOR} Name: " IRCBOLD "{RNAME}" IRCBOLD " - ID: {ID} - Team: " IRCBOLD "{TEAML}" IRCBOLD " - Vehicle Kills: {VEHICLEKILLS} - Building Kills {BUILDINGKILLS} - Kills {KILLS} - Deaths: {DEATHS} - KDR: {KDR} - Access: {ACCESS}"));
	RenX_CommandsPlugin::adminPlayerInfoFormat = Jupiter::IRC::Client::Config->get(RenX_CommandsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("AdminPlayerInfoFormat"), Jupiter::StringS::Format("%.*s - IP: " IRCBOLD "{IP}" IRCBOLD " - Steam ID: " IRCBOLD "{STEAM}", RenX_CommandsPlugin::playerInfoFormat.size(), RenX_CommandsPlugin::playerInfoFormat.ptr()));
	RenX_CommandsPlugin::buildingInfoFormat = Jupiter::IRC::Client::Config->get(RenX_CommandsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("BuildingInfoFormat"), STRING_LITERAL_AS_REFERENCE(IRCCOLOR) + RenX::tags->buildingTeamColorTag + RenX::tags->buildingNameTag + STRING_LITERAL_AS_REFERENCE(IRCCOLOR " - " IRCCOLOR "07") + RenX::tags->buildingHealthPercentageTag + STRING_LITERAL_AS_REFERENCE("%"));

	RenX::sanitizeTags(RenX_CommandsPlugin::playerInfoFormat);
	RenX::sanitizeTags(RenX_CommandsPlugin::adminPlayerInfoFormat);
	RenX::sanitizeTags(RenX_CommandsPlugin::buildingInfoFormat);
	return 0;
}

time_t RenX_CommandsPlugin::getTBanTime() const
{
	return RenX_CommandsPlugin::_defaultTempBanTime;
}

const Jupiter::ReadableString &RenX_CommandsPlugin::getPlayerInfoFormat() const
{
	return RenX_CommandsPlugin::playerInfoFormat;
}

const Jupiter::ReadableString &RenX_CommandsPlugin::getAdminPlayerInfoFormat() const
{
	return RenX_CommandsPlugin::adminPlayerInfoFormat;
}

const Jupiter::ReadableString &RenX_CommandsPlugin::getBuildingInfoFormat() const
{
	return RenX_CommandsPlugin::buildingInfoFormat;
}

RenX_CommandsPlugin::RenX_CommandsPlugin()
{
	this->OnRehash();
}

// Plugin instantiation and entry point.
RenX_CommandsPlugin pluginInstance;

/** Console Commands */

// RawRCON Console Command

RawRCONConsoleCommand::RawRCONConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rrcon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rawrcon"));
}

void RawRCONConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	Jupiter::StringS msg = parameters;
	msg += '\n';
	int i = RenX::getCore()->getServerCount();
	if (i == 0)
		puts("Error: Not connected to any Renegade X servers.");
	else if (parameters.isNotEmpty())
		while (--i >= 0)
			RenX::getCore()->getServer(i)->sendData(msg);
	else
		puts("Error: Too Few Parameters. Syntax: rcon <input>");
}

const Jupiter::ReadableString &RawRCONConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends data over the Renegade X server's rcon connection. Syntax: rrcon <data>");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RawRCONConsoleCommand)

// RCON Console Command

RCONConsoleCommand::RCONConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rcon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("renx"));
}

void RCONConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	int i = RenX::getCore()->getServerCount();
	if (i == 0)
		puts("Error: Not connected to any Renegade X servers.");
	else if (parameters != nullptr)
		while (--i >= 0)
			RenX::getCore()->getServer(i)->send(parameters);
	else
		puts("Error: Too Few Parameters. Syntax: rcon <input>");
}

const Jupiter::ReadableString &RCONConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Executes a command over the Renegade X server's rcon connection. Syntax: rcon <input>");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RCONConsoleCommand)

/** IRC Commands */

// Msg IRC Command

void MsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("msg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("say"));
}

void MsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		int type = source->getChannel(channel)->getType();
		Jupiter::StringL msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += parameters;

		prefix = '\0'; // Reusing prefix to check if a match is ever found.
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				server->sendMessage(msg);
				prefix = 1;
			}
		}
		if (prefix == '\0')
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Msg <Message>"));
}

const Jupiter::ReadableString &MsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message in - game.Syntax: Msg <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(MsgIRCCommand)

// PMsg IRC Command

void PMsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("psay"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("page"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ppage"));
	this->setAccessLevel(1);
}

void PMsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) >= 2)
	{
		int type = source->getChannel(channel)->getType();
		Jupiter::ReferenceString name = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
		RenX::PlayerInfo *player;
		Jupiter::StringL msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);
		if (parameters.isNotEmpty())
		{
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					player = server->getPlayerByPartName(name);
					if (player != nullptr)
						server->sendMessage(player, msg);
					else source->sendNotice(nick, Jupiter::StringS::Format("Error: Player \"%.*s\" not found.", name.size(), name.ptr()));
				}
			}
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: PMsg <Player> <Message>"));
}

const Jupiter::ReadableString &PMsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message in - game.Syntax: PMsg <Player> <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PMsgIRCCommand)

// Host Msg IRC Command

void HostMsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("hmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("hsay"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("hostmessage"));
	this->setAccessLevel(4);
}

void HostMsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		int type = source->getChannel(channel)->getType();

		bool success = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
				success = server->sendMessage(parameters) > 0;
		}
		if (!success)
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: hmsg <Message>"));
}

const Jupiter::ReadableString &HostMsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message in-game. Syntax: hmsg <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(HostMsgIRCCommand)

// Players IRC Command

void PlayersIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("players"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pl"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playerlist"));
}

const size_t STRING_LENGTH = 240;

void PlayersIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &)
{
	int type = source->getChannel(channel)->getType();

	// Team colors
	const Jupiter::ReadableString &gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeamColor = RenX::getTeamColor(RenX::TeamType::Other);

	// Team names
	const Jupiter::ReadableString &gTeam = RenX::getTeamName(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeam = RenX::getTeamName(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeam = RenX::getTeamName(RenX::TeamType::Other);

	bool noServers = true;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		RenX::Server *server = RenX::getCore()->getServer(i);
		if (server->isLogChanType(type))
		{
			noServers = false;
			if (server->players.size() != 0)
			{
				// End string containers
				Jupiter::DLList<Jupiter::String> gStrings;
				Jupiter::DLList<Jupiter::String> nStrings;
				Jupiter::DLList<Jupiter::String> oStrings;

				Jupiter::StringL *gCurrent = nullptr;
				Jupiter::StringL *nCurrent = nullptr;
				Jupiter::StringL *oCurrent = nullptr;

				// Team player counters
				unsigned int gTotal = 0;
				unsigned int nTotal = 0;
				unsigned int oTotal = 0;

				// Bot counters
				unsigned int gBots = 0;
				unsigned int nBots = 0;
				unsigned int oBots = 0;

				for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
				{
					if (node->data != nullptr)
					{
						Jupiter::String &name = RenX::getFormattedPlayerName(node->data);
						if (name.size() > STRING_LENGTH - 32) continue; // Name will be too long to send.

						switch (node->data->team)
						{
						case RenX::TeamType::Nod:
							if (nCurrent == nullptr || nCurrent->size() + name.size() > STRING_LENGTH)
							{
								nCurrent = new Jupiter::StringL(STRING_LENGTH);
								nCurrent->format(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD, nTeamColor.size(), nTeamColor.ptr(), nTeam.size(), nTeam.ptr(), name.size(), name.ptr());
								nStrings.add(nCurrent);
							}
							else nCurrent->aformat(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.ptr());
							nTotal++;
							if (node->data->isBot)
								nBots++;
							break;
						case RenX::TeamType::GDI:
							if (gCurrent == nullptr || gCurrent->size() + name.size() > STRING_LENGTH)
							{
								gCurrent = new Jupiter::StringL(STRING_LENGTH);
								gCurrent->format(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD, gTeamColor.size(), gTeamColor.ptr(), gTeam.size(), gTeam.ptr(), name.size(), name.ptr());
								gStrings.add(gCurrent);
							}
							else gCurrent->aformat(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.ptr());
							gTotal++;
							if (node->data->isBot)
								gBots++;
							break;
						default:
							if (oCurrent == nullptr || oCurrent->size() + name.size() > STRING_LENGTH)
							{
								oCurrent = new Jupiter::StringL(STRING_LENGTH);
								oCurrent->format(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD, oTeamColor.size(), oTeamColor.ptr(), oTeam.size(), oTeam.ptr(), name.size(), name.ptr());
								oStrings.add(oCurrent);
							}
							else oCurrent->aformat(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.ptr());
							oTotal++;
							if (node->data->isBot)
								oBots++;
							break;
						}
					}
				}
				Jupiter::StringL *outString;
				while (gStrings.size() != 0)
				{
					outString = gStrings.remove(0U);
					source->sendMessage(channel, *outString);
					delete outString;
				}
				while (nStrings.size() != 0)
				{
					outString = nStrings.remove(0U);
					source->sendMessage(channel, *outString);
					delete outString;
				}
				while (oStrings.size() != 0)
				{
					outString = oStrings.remove(0U);
					source->sendMessage(channel, *outString);
					delete outString;
				}

				Jupiter::StringL out;
				out.format(IRCCOLOR "03Total Players" IRCCOLOR ": %u", server->players.size());
				if (gBots + nBots + oBots > 0)
					out.aformat(" (%u bots)", gBots + nBots + oBots);
				if (gTotal > 0)
				{
					out.aformat(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", gTeamColor.size(), gTeamColor.ptr(), gTeam.size(), gTeam.ptr(), gTotal);
					if (gBots > 0)
						out.aformat(" (%u bots)", gBots);
				}
				if (nTotal > 0)
				{
					out.aformat(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", nTeamColor.size(), nTeamColor.ptr(), nTeam.size(), nTeam.ptr(), nTotal);
					if (nBots > 0)
						out.aformat(" (%u bots)", nBots);
				}
				if (oTotal > 0)
				{
					out.aformat(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", oTeamColor.size(), oTeamColor.ptr(), oTeam.size(), oTeam.ptr(), oTotal);
					if (oBots > 0)
						out.aformat(" (%u bots)", oBots);
				}
				source->sendMessage(channel, out);
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("No players are in-game."));
		}
	}
	if (noServers)
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
}

const Jupiter::ReadableString &PlayersIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Lists the players currently in-game. Syntax: Players");
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayersIRCCommand)

// PlayerTable IRC Command
void PlayerTableIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playertable"));
}

void PlayerTableIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &)
{
	int type = source->getChannel(channel)->getType();

	// Team colors
	const Jupiter::ReadableString &gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeamColor = RenX::getTeamColor(RenX::TeamType::Other);

	// Team names
	const Jupiter::ReadableString &gTeam = RenX::getTeamName(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeam = RenX::getTeamName(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeam = RenX::getTeamName(RenX::TeamType::Other);

	bool noServers = true;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		RenX::Server *server = RenX::getCore()->getServer(i);
		if (server->isLogChanType(type))
		{
			noServers = false;
			if (server->players.size() != 0)
			{
				Jupiter::SLList<RenX::PlayerInfo> gPlayers;
				Jupiter::SLList<RenX::PlayerInfo> nPlayers;
				Jupiter::SLList<RenX::PlayerInfo> oPlayers;

				STRING_LITERAL_AS_NAMED_REFERENCE(NICK_COL_HEADER, "Nickname");
				size_t maxNickLen = 8;
				int highID = 999;
				double highScore = 99999.0;
				double highCredits = 9999999.0;

				RenX::PlayerInfo *player;
				for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
				{
					player = node->data;
					if (player != nullptr && player->isBot == false)
					{
						if (player->name.size() > maxNickLen)
							maxNickLen = player->name.size();

						if (player->id > highID)
							highID = player->id;

						if (player->score > highScore)
							highScore = player->score;

						if (player->credits > highCredits)
							highCredits = player->credits;

						switch (player->team)
						{
						case RenX::TeamType::GDI:
							gPlayers.add(player);
							break;
						case RenX::TeamType::Nod:
							nPlayers.add(player);
							break;
						default:
							oPlayers.add(player);
							break;
						}
					}
				}

				size_t idColLen = 1, scoreColLen = 1, creditColLen = 1;

				while ((highID /= 10) > 0)
					++idColLen;

				while ((highScore /= 10) >= 1.0)
					++scoreColLen;

				while ((highCredits /= 10) >= 1.0)
					++creditColLen;

				source->sendMessage(channel, Jupiter::StringS::Format(IRCUNDERLINE IRCCOLOR "03%*.*s | %*s | %*s | %*s", maxNickLen, NICK_COL_HEADER.size(), NICK_COL_HEADER.ptr(), idColLen, "ID", scoreColLen, "Score", creditColLen, "Credits"));

				auto output_player = [source, &channel, maxNickLen, idColLen, scoreColLen, creditColLen](RenX::PlayerInfo *player, const Jupiter::ReadableString &color)
				{
					source->sendMessage(channel, Jupiter::StringS::Format(IRCCOLOR "%.*s%*.*s" IRCCOLOR " " IRCCOLOR "03|" IRCCOLOR " %*d " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCCOLOR " %*.0f", color.size(), color.ptr(), maxNickLen, player->name.size(), player->name.ptr(), idColLen, player->id, scoreColLen, player->score, creditColLen, player->credits));
				};

				for (Jupiter::SLList<RenX::PlayerInfo>::Node *node = gPlayers.getNode(0); node != nullptr; node = node->next)
					output_player(node->data, gTeamColor);

				for (Jupiter::SLList<RenX::PlayerInfo>::Node *node = nPlayers.getNode(0); node != nullptr; node = node->next)
					output_player(node->data, nTeamColor);

				for (Jupiter::SLList<RenX::PlayerInfo>::Node *node = oPlayers.getNode(0); node != nullptr; node = node->next)
					output_player(node->data, oTeamColor);
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("No players are in-game."));
		}
	}
	if (noServers)
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
}

const Jupiter::ReadableString &PlayerTableIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Generates a table of all the players in-game. Syntax: PT");
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayerTableIRCCommand)

// PlayerInfo IRC Command

void PlayerInfoIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playerinfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pi"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("player"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pinfo"));
}

void PlayerInfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->name.findi(parameters) != Jupiter::INVALID_INDEX)
						{
							if (source->getAccessLevel(channel, nick) > 1)
								msg = pluginInstance.getAdminPlayerInfoFormat();
							else
								msg = pluginInstance.getPlayerInfoFormat();
							RenX::processTags(msg, server, player);
							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.isEmpty()) source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: PlayerInfo <Player>"));
}

const Jupiter::ReadableString &PlayerInfoIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets information about a player. Syntax: PlayerInfo <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayerInfoIRCCommand)

// BuildingInfo IRC Command

void BuildingInfoIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("binfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bi"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("buildinginfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("building"));
}

void BuildingInfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool seenStrip;
		Jupiter::SLList<Jupiter::String> gStrings;
		Jupiter::SLList<Jupiter::String> nStrings;
		Jupiter::SLList<Jupiter::String> oStrings;
		Jupiter::SLList<Jupiter::String> cStrings;
		Jupiter::String *str = nullptr;
		RenX::BuildingInfo *building;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				seenStrip = false;
				for (size_t index = 0; index != server->buildings.size(); ++index)
				{
					building = server->buildings.get(index);
					if (building->name.find("Rx_Building_Air"_jrs) == 0)
					{
						if (seenStrip)
							continue;

						seenStrip = true;
					}
					str = new Jupiter::String(pluginInstance.getBuildingInfoFormat());
					RenX::processTags(*str, server, nullptr, nullptr, building);
					if (building->capturable)
						cStrings.add(str);
					else if (building->team == RenX::TeamType::GDI)
						gStrings.add(str);
					else if (building->team == RenX::TeamType::Nod)
						nStrings.add(str);
					else
						oStrings.add(str);
				}
				while (gStrings.size() != 0)
				{
					str = gStrings.remove(0);
					source->sendMessage(channel, *str);
					delete str;
				}
				while (nStrings.size() != 0)
				{
					str = nStrings.remove(0);
					source->sendMessage(channel, *str);
					delete str;
				}
				while (oStrings.size() != 0)
				{
					str = oStrings.remove(0);
					source->sendMessage(channel, *str);
					delete str;
				}
				while (cStrings.size() != 0)
				{
					str = cStrings.remove(0);
					source->sendMessage(channel, *str);
					delete str;
				}
			}
		}
		if (str == nullptr)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
	}
}

const Jupiter::ReadableString &BuildingInfoIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Provides a list of buildings, and the status of each one. Syntax: BuildingInfo");
	return defaultHelp;
}

IRC_COMMAND_INIT(BuildingInfoIRCCommand)

// Mutators IRC Command

void MutatorsIRCCommand::create()
{
	this->addTrigger("mutators"_jrs);
	this->addTrigger("mutator"_jrs);
}

void MutatorsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		Jupiter::String list;
		size_t index = 0;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				list = STRING_LITERAL_AS_REFERENCE(IRCCOLOR "03[Mutators]" IRCNORMAL);
				for (index = 0; index != server->mutators.size(); ++index)
					list += " "_jrs + *server->mutators.get(index);
				if (index == 0)
					source->sendMessage(channel, "No mutators loaded"_jrs);
				else
					source->sendMessage(channel, list);
			}
		}
		if (list.isEmpty())
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
	}
}

const Jupiter::ReadableString &MutatorsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Provides a list of mutators being used. Syntax: Mutators");
	return defaultHelp;
}

IRC_COMMAND_INIT(MutatorsIRCCommand)

// Rotation IRC Command

void RotationIRCCommand::create()
{
	this->addTrigger("rotation"_jrs);
	this->addTrigger("maprotation"_jrs);
	this->addTrigger("maps"_jrs);
	this->addTrigger("rot"_jrs);
}

void RotationIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		const Jupiter::ReadableString *map;
		int type = chan->getType();
		Jupiter::String list;
		size_t index = 0;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				list = STRING_LITERAL_AS_REFERENCE(IRCCOLOR "03[Rotation]" IRCNORMAL);
				for (index = 0; index != server->maps.size(); ++index)
				{
					map = server->maps.get(index);
					if (server->getMap().equalsi(*map))
						list += STRING_LITERAL_AS_REFERENCE(" " IRCBOLD "[") + *server->maps.get(index) + STRING_LITERAL_AS_REFERENCE("]" IRCBOLD);
					else
						list += " "_jrs + *server->maps.get(index);
				}
				if (index == 0)
					source->sendMessage(channel, "No maps in rotation"_jrs);
				else
					source->sendMessage(channel, list);
			}
		}
		if (list.isEmpty())
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
	}
}

const Jupiter::ReadableString &RotationIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Provides a list of maps in the server rotation. Syntax: Rotation");
	return defaultHelp;
}

IRC_COMMAND_INIT(RotationIRCCommand)

// Map IRC Command

void MapIRCCommand::create()
{
	this->addTrigger("map"_jrs);
}

void MapIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool match = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				match = true;
				source->sendMessage(channel, "Current Map: "_jrs + server->getMap());
			}
		}
		if (match == false)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
	}
}

const Jupiter::ReadableString &MapIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Fetches the current map. Syntax: Map");
	return defaultHelp;
}

IRC_COMMAND_INIT(MapIRCCommand)

// Steam IRC Command

void SteamIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("steam"));
	this->setAccessLevel(1);
}

void SteamIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		RenX::PlayerInfo *player;
		if (parameters.isNotEmpty())
		{
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->name.findi(parameters) != Jupiter::INVALID_INDEX)
						{
							Jupiter::String &playerName = RenX::getFormattedPlayerName(player);
							msg.format(IRCCOLOR "03[Steam] " IRCCOLOR "%.*s (ID: %d) ", playerName.size(), playerName.ptr(), player->id);
							if (player->steamid != 0)
							{
								msg += "is using steam ID " IRCBOLD;
								msg += server->formatSteamID(player);
								msg.aformat(IRCBOLD "; Steam Profile: " IRCBOLD "https://steamcommunity.com/profiles/%llu" IRCBOLD, player->steamid);
							}
							else msg += "is not using steam.";
							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.isEmpty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
		else
		{
			unsigned int total;
			unsigned int realPlayers;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					total = 0;
					realPlayers = 0;
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->isBot == false)
						{
							realPlayers++;
							if (player->steamid != 0)
								total++;
						}
					}
					if (realPlayers != 0)
						source->sendMessage(channel, Jupiter::StringS::Format("%.2f%% (%u/%u) of players are using Steam.", ((double)total * 100) / ((double)realPlayers), total, realPlayers));
					else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("No players are in-game."));
				}
			}
		}
	}
}

const Jupiter::ReadableString &SteamIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Fetches steam usage information. Syntax: Steam [Player]");
	return defaultHelp;
}

IRC_COMMAND_INIT(SteamIRCCommand)

// Kill-Death Ratio IRC Command

void KillDeathRatioIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kills"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deaths"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kdr"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killdeathraio"));
}

void KillDeathRatioIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->name.findi(parameters) != Jupiter::INVALID_INDEX)
						{
							Jupiter::String &playerName = RenX::getFormattedPlayerName(player);
							msg.format(IRCBOLD "%.*s" IRCBOLD IRCCOLOR ": Kills: %u - Deaths: %u - KDR: %.2f", playerName.size(), playerName.ptr(), player->kills, player->deaths, static_cast<double>(player->kills) / (player->deaths == 0 ? 1.0f : static_cast<double>(player->deaths)));
							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.isEmpty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Kills <Player>"));
}

const Jupiter::ReadableString &KillDeathRatioIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets a player's kills and deaths. Syntax: Kills <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(KillDeathRatioIRCCommand)

// ShowMods IRC Command

void ShowModsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showmods"));
}

extern ModsGameCommand ModsGameCommand_instance;

void ShowModsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool sent = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				ModsGameCommand_instance.trigger(server, nullptr, Jupiter::ReferenceString::empty);
				sent = true;
			}
		}
		if (sent == false)
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &ShowModsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying the in-game moderators. Syntax: showmods");
	return defaultHelp;
}

IRC_COMMAND_INIT(ShowModsIRCCommand)

// Mods IRC Command

void ModsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mods"));
}

void ModsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.equalsi("show")) ShowModsIRCCommand_instance.trigger(source, channel, nick, parameters);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					msg = "";
					if (server->players.size() != 0)
					{
						for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
						{
							player = node->data;
							if (player->isBot == false && (player->adminType.isNotEmpty() || (player->access != 0 && (player->gamePrefix.isNotEmpty() || player->formatNamePrefix.isNotEmpty()))))
							{
								if (msg.isNotEmpty())
									msg += ", ";
								else msg += "Moderators in-game: ";
								msg += player->gamePrefix;
								msg += player->name;
							}
						}
					}
					if (msg.isEmpty())
						msg = "No moderators are in-game.";
					source->sendMessage(channel, msg);
				}
			}
			if (msg.isEmpty())
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
}

const Jupiter::ReadableString &ModsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying the in-game moderators. Syntax: mods [show]");
	return defaultHelp;
}

IRC_COMMAND_INIT(ModsIRCCommand)

// BanSearch IRC Command

void BanSearchIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bansearch"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bsearch"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("banfind"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bfind"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("banlogs"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("blogs"));
	this->setAccessLevel(2);
}

void BanSearchIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	auto entries = RenX::banDatabase->getEntries();
	if (parameters.isNotEmpty())
	{
		if (entries.size() == 0)
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("The ban database is empty!"));
		else
		{
			RenX::BanDatabase::Entry *entry;
			Jupiter::ReferenceString params = Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);
			std::function<bool(unsigned int)> isMatch = [&](unsigned int type_l) -> bool
			{
				switch (type_l)
				{
				default:
				case 0:	// ANY
					return isMatch(1) || isMatch(2) || isMatch(3) || isMatch(4);
				case 1:	// IP
					return entry->ip == params.asUnsignedInt();
				case 2:	// STEAM
					return entry->steamid == params.asUnsignedLongLong();
				case 3:	// NAME
					return entry->name.equalsi(params);
				case 4:	// BANNER
					return entry->varData.get(pluginInstance.getName()).equalsi(params);
				case 5:	// ACTIVE
					if (params.asBool()) // Got tired of seeing a compiler warning.
						return entry->active == 1;
					else
						return entry->active == 0;
				case 6:	// ALL
					return true;
				}
			};

			unsigned int type;
			Jupiter::ReferenceString type_str = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
			if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("ip")))
				type = 1;
			else if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("steam")))
				type = 2;
			else if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("name")))
				type = 3;
			else if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("banner")))
				type = 4;
			else if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("active")))
				type = 5;
			else if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("any")))
				type = 0;
			else if (type_str.equalsi(STRING_LITERAL_AS_REFERENCE("all")) || type_str.equals('*'))
				type = 6;
			else
			{
				type = 0;
				params = parameters;
			}

			Jupiter::String out(256);
			char timeStr[256];
			for (size_t i = 0; i != entries.size(); i++)
			{
				entry = entries.get(i);
				if (isMatch(type))
				{
					Jupiter::StringS ip_str = Jupiter::Socket::ntop4(entry->ip);
					const Jupiter::ReadableString &banner = entry->varData.get(pluginInstance.getName());
					strftime(timeStr, sizeof(timeStr), "%b %d %Y; Time: %H:%M:%S", localtime(&(entry->timestamp)));
					out.format("ID: %lu; Status: %sactive; Date: %s; IP: %.*s; Steam: %llu; Name: %.*s%s", i, entry->active ? "" : "in", timeStr, ip_str.size(), ip_str.ptr(), entry->steamid, entry->name.size(), entry->name.ptr(), banner.isEmpty() ? "" : "; Banner: ");
					out.concat(banner);
					source->sendNotice(nick, out);
				}
			}
			if (out.isEmpty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("No matches found."));
		}
	}
	else
		source->sendNotice(nick, Jupiter::StringS::Format("There are a total of %u entries in the ban database.", entries.size()));
}

const Jupiter::ReadableString &BanSearchIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Searches the ban database for an entry. Syntax: bsearch [ip/steam/name/banner/active/any/all = any] <player ip/steam/name/banner>");
	return defaultHelp;
}

IRC_COMMAND_INIT(BanSearchIRCCommand)

// ShowRules IRC Command

void ShowRulesIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showrules"));
}

void ShowRulesIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		Jupiter::StringL msg;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				msg = "Rules: ";
				msg += server->getRules();
				server->sendMessage(msg);
			}
		}
		if (msg.isEmpty())
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &ShowRulesIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying the in-game rules. Syntax: showrules");
	return defaultHelp;
}

IRC_COMMAND_INIT(ShowRulesIRCCommand)

// Rules IRC Command

void RulesIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rules"));
}

void RulesIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.equalsi("show")) ShowRulesIRCCommand_instance.trigger(source, channel, nick, parameters);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					msg = IRCCOLOR "11[Rules]" IRCCOLOR " ";
					msg += server->getRules();
					source->sendMessage(channel, msg);
				}
			}
			if (msg.isEmpty())
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
}

const Jupiter::ReadableString &RulesIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays the in-game rules. Syntax: rules [show]");
	return defaultHelp;
}

IRC_COMMAND_INIT(RulesIRCCommand)

// SetRules IRC Command

void SetRulesIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("setrules"));
	this->setAccessLevel(4);
}

void SetRulesIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		unsigned int r = 0;
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					server->setRules(parameters);
					r++;
				}
			}
			if (r == 0)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
}

const Jupiter::ReadableString &SetRulesIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sets the in-game rules. Syntax: setrules [show]");
	return defaultHelp;
}

IRC_COMMAND_INIT(SetRulesIRCCommand)

// Reconnect IRC Command

void ReconnectIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("reconnect"));
	this->setAccessLevel(3);
}

void ReconnectIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		Jupiter::StringS msg;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				if (server->reconnect(RenX::DisconnectReason::Triggered)) msg.set("Connection established");
				else msg.format("[RenX] ERROR: Failed to connect to %.*s on port %u." ENDL, server->getHostname().size(), server->getHostname().ptr(), server->getPort());
				source->sendMessage(channel, msg);
			}
		}
		if (msg.isEmpty())
		{
			// We didn't connect anywhere!!
			msg.set("ERROR: No servers found to connect to.");
			source->sendMessage(channel, msg);
		}
	}
}

const Jupiter::ReadableString &ReconnectIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets the RCON connection. Syntax: Reconnect");
	return defaultHelp;
}

IRC_COMMAND_INIT(ReconnectIRCCommand)

// GameOver IRC Command

void GameOverIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("gameover"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("endmap"));
	this->setAccessLevel(3);
}

void GameOverIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool match = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				match = true;
				if (server->gameover() == false)
					source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support gameover."));
			}
		}
		if (match == false)
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &GameOverIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Ends the game immediately. Syntax: Gameover");
	return defaultHelp;
}

IRC_COMMAND_INIT(GameOverIRCCommand)

// SetMap IRC Command

void SetMapIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("setmap"));
	this->setAccessLevel(4);
}

void SetMapIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			const Jupiter::ReadableString *map_name = nullptr;
			int type = chan->getType();
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					map_name = server->getMapName(parameters);
					if (map_name == nullptr)
						source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Map not in rotation."));
					else if (server->setMap(*map_name) == false)
						source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Transmission error."));
				}
			}
			if (map_name == nullptr)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: setmap <map>"));
}

const Jupiter::ReadableString &SetMapIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Ends the game immediately. Syntax: setmap <map>");
	return defaultHelp;
}

IRC_COMMAND_INIT(SetMapIRCCommand)

// Mute IRC Command

void MuteIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mute"));
	this->setAccessLevel(2);
}

void MuteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->mute(player) == false)
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support muting players."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: mute <player>"));
}

const Jupiter::ReadableString &MuteIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Mutes a player. Syntax: mute <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(MuteIRCCommand)

// UnMute IRC Command

void UnMuteIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unmute"));
	this->setAccessLevel(2);
}

void UnMuteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->unmute(player) == false)
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support muting players."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: unmute <player>"));
}

const Jupiter::ReadableString &UnMuteIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Unmutes a player. Syntax: unmute <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(UnMuteIRCCommand)

// Kill IRC Command

void KillIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kill"));
	this->setAccessLevel(2);
}

void KillIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->kill(player) == false)
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support muting players."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: kill <player>"));
}

const Jupiter::ReadableString &KillIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kills a player. Syntax: kill <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(KillIRCCommand)

// Disarm IRC Command

void DisarmIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarm"));
	this->setAccessLevel(2);
}

void DisarmIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->disarm(player) == false)
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support muting players."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: disarm <player>"));
}

const Jupiter::ReadableString &DisarmIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed objects. Syntax: disarm <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmIRCCommand)

// DisarmC4 IRC Command

void DisarmC4IRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmc4"));
	this->setAccessLevel(2);
}

void DisarmC4IRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->disarmC4(player) == false)
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support muting players."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: disarmC4 <player>"));
}

const Jupiter::ReadableString &DisarmC4IRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed C4s. Syntax: disarmc4 <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmC4IRCCommand)

// DisarmBeacon IRC Command

void DisarmBeaconIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacons"));
	this->setAccessLevel(2);
}

void DisarmBeaconIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->disarmBeacon(player) == false)
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support muting players."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: disarmb <player>"));
}

const Jupiter::ReadableString &DisarmBeaconIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed beacons. Syntax: disarmb <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmBeaconIRCCommand)

// Kick IRC Command

void KickIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("qkick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("k"));
	this->setAccessLevel(2);
}

void KickIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				RenX::PlayerInfo *player;
				RenX::Server *server;
				unsigned int kicks = 0;
				Jupiter::StringS name = Jupiter::StringS::getWord(parameters, 0, WHITESPACE);
				Jupiter::StringS reason = parameters.wordCount(WHITESPACE) > 1 ? Jupiter::StringS::gotoWord(parameters, 1, WHITESPACE) : STRING_LITERAL_AS_REFERENCE("No reason");
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(name);
						if (player != nullptr)
						{
							server->kickPlayer(player, reason);
							++kicks;
						}
					}
				}
				source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Kick <Player> [Reason]"));
}

const Jupiter::ReadableString &KickIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks a player from the game. Syntax: Kick <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickIRCCommand)

// TempBan IRC Command

void TempBanIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tempban"));
	this->setAccessLevel(3);
}

void TempBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				RenX::PlayerInfo *player;
				RenX::Server *server;
				unsigned int kicks = 0;
				Jupiter::StringS name = Jupiter::StringS::getWord(parameters, 0, WHITESPACE);
				Jupiter::StringS reason = parameters.wordCount(WHITESPACE) > 1 ? Jupiter::StringS::gotoWord(parameters, 1, WHITESPACE) : STRING_LITERAL_AS_REFERENCE("No reason");
				Jupiter::String banner(nick.size() + 4);
				banner += nick;
				banner += "@IRC";
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(name);
						if (player != nullptr)
						{
							player->varData.set(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("banner"), nick);
							server->banPlayer(player, reason, pluginInstance.getTBanTime());
							kicks++;
						}
					}
				}
				source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: TempBan <Player> [Reason]"));
}

const Jupiter::ReadableString &TempBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and temporarily bans a player from the game. Syntax: TempBan <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(TempBanIRCCommand)

// KickBan IRC Command

void KickBanIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kickban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ban"));
	this->setAccessLevel(4);
}

void KickBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				RenX::PlayerInfo *player;
				RenX::Server *server;
				unsigned int kicks = 0;
				Jupiter::StringS name = Jupiter::StringS::getWord(parameters, 0, WHITESPACE);
				Jupiter::StringS reason = parameters.wordCount(WHITESPACE) > 1 ? Jupiter::StringS::gotoWord(parameters, 1, WHITESPACE) : STRING_LITERAL_AS_REFERENCE("No reason");
				Jupiter::String banner(nick.size() + 4);
				banner += nick;
				banner += "@IRC";
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(parameters);
						if (player != nullptr)
						{
							player->varData.set(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("banner"), nick);
							server->banPlayer(player, reason);
							kicks++;
						}
					}
				}
				source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: KickBan <Player> [Reason]"));
}

const Jupiter::ReadableString &KickBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and bans a player from the game. Syntax: KickBan <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickBanIRCCommand)

// UnBan IRC Command

void UnBanIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("uban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("dban"));
	this->setAccessLevel(4);
}

void UnBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		size_t index = static_cast<size_t>(parameters.asUnsignedLongLong());
		if (index < RenX::banDatabase->getEntries().size())
		{
			if (RenX::banDatabase->deactivate(index))
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Ban deactivated."));
			else
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Ban not active."));
		}
		else
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid ban ID; please find the ban ID using \"bansearch\"."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: unban <Ban ID>"));
}

const Jupiter::ReadableString &UnBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Deactivates a ban. Syntax: unban <Ban ID>");
	return defaultHelp;
}

IRC_COMMAND_INIT(UnBanIRCCommand)

// AddBots IRC Command

void AddBotsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbot"));
	this->setAccessLevel(2);
}

void AddBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(source->getChannel(channel)->getType());
			if (servers.size() != 0)
			{
				int amount = 1;
				if (parameters.isNotEmpty())
					amount = parameters.asInt();
				if (amount != 0)
				{
					RenX::Server *server;
					Jupiter::StringL cmd;
					RenX::TeamType team = RenX::getTeam(Jupiter::ReferenceString::getWord(parameters, 1, WHITESPACE));

					switch (team)
					{
					case RenX::TeamType::GDI:
						cmd = "addredbots ";
						break;
					case RenX::TeamType::Nod:
						cmd = "addbluebots ";
						break;
					case RenX::TeamType::None:
					case RenX::TeamType::Other:
						cmd = "addbots ";
						break;
					}

					for (size_t i = 0, extra; i != servers.size(); i++)
					{
						server = servers.get(i);
						if (server != nullptr)
						{
							extra = cmd.aformat("%u", amount);
							server->send(cmd);
							cmd -= extra;
						}
						server->sendMessage(Jupiter::StringS::Format("%u bots have been added to the server.", amount));
					}
				}
				else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Invalid amount entered. Amount must be a positive integer."));
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
}

const Jupiter::ReadableString &AddBotsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds bots to the game. Syntax: AddBots [Amount=1] [Team]");
	return defaultHelp;
}

IRC_COMMAND_INIT(AddBotsIRCCommand)

// KillBots IRC Command

void KillBotsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killbot"));
	this->setAccessLevel(2);
}

void KillBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
		if (servers.size() != 0)
		{
			RenX::Server *server;

			for (size_t i = 0; i != servers.size(); i++)
			{
				server = servers.get(i);
				server->send(STRING_LITERAL_AS_REFERENCE("killbots"));
				server->sendMessage(STRING_LITERAL_AS_REFERENCE("All bots have been removed from the server."));
			}
		}
		else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &KillBotsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes all bots from the game. Syntax: KillBots");
	return defaultHelp;
}

IRC_COMMAND_INIT(KillBotsIRCCommand)

// PhaseBots IRC Command

void PhaseBotsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("phasebots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("phasebot"));
	this->setAccessLevel(2);
}

void PhaseBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
		if (servers.size() != 0)
		{
			RenX::Server *server;

			for (size_t i = 0; i != servers.size(); i++)
			{
				server = servers.get(i);
				if (parameters.isEmpty())
				{
					if (togglePhasing(server))
						server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
					else server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
				}
				else if (parameters.equalsi("true") || parameters.equalsi("on") || parameters.equalsi("start") || parameters.equalsi("1"))
				{
					togglePhasing(server, true);
					server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
				}
				else
				{
					togglePhasing(server, false);
					server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
				}
			}
		}
		else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &PhaseBotsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Toggles the phasing of bots from the game by kicking them after death. Syntax: PhaseBots [on/off]");
	return defaultHelp;
}

IRC_COMMAND_INIT(PhaseBotsIRCCommand)

// RCON IRC Command

void RCONIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rcon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("renx"));
	this->setAccessLevel(5);
}

void RCONIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters != nullptr)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			unsigned int r = RenX::getCore()->send(chan->getType(), parameters);
			if (r > 0)
				source->sendMessage(channel, Jupiter::StringS::Format("Command sent to %u servers.", r));
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: rcon <input>"));
}

const Jupiter::ReadableString &RCONIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends data to the Renegade X server's rcon. Syntax: rcon <input>");
	return defaultHelp;
}

IRC_COMMAND_INIT(RCONIRCCommand)

// Refund IRC Command

void RefundIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("refund"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("givecredits"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("gc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("money"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("credits"));
	this->setAccessLevel(3);
}

void RefundIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) >= 2)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::ReferenceString playerName = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
			double credits = Jupiter::ReferenceString::getWord(parameters, 1, WHITESPACE).asDouble();
			RenX::PlayerInfo *player;
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->name.findi(playerName) != Jupiter::INVALID_INDEX)
						{
							if (server->giveCredits(player, credits))
							{
								msg.format("You have been refunded %.0f credits by %.*s.", credits, nick.size(), nick.ptr());
								server->sendMessage(player, msg);
								msg.format("%.*s has been refunded %.0f credits.", player->name.size(), player->name.ptr(), credits);
							}
							else
								msg.set("Error: Server does not support refunds.");
							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.isEmpty()) source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: refund <player> <amount>"));
}

const Jupiter::ReadableString &RefundIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Refunds a player's credits. Syntax: refund <player> <amount>");
	return defaultHelp;
}

IRC_COMMAND_INIT(RefundIRCCommand)

// Team-Change IRC Command

void TeamChangeIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("team"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ftc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forcetc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("teamchange"));
	this->setAccessLevel(3);
}

void TeamChangeIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::ReferenceString playerName = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
			RenX::PlayerInfo *player;
			bool playerFound = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->name.findi(playerName) != Jupiter::INVALID_INDEX)
						{
							playerFound = true;
							if (server->changeTeam(player) == false)
								source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support team changing."));
						}
					}
				}
			}
			if (playerFound == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: team <player>"));
}

const Jupiter::ReadableString &TeamChangeIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Changes a player's team. Syntax: team <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(TeamChangeIRCCommand)

// TeamChange2 IRC Command

void TeamChange2IRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("team2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tc2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ftc2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forcetc2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("teamchange2"));
	this->setAccessLevel(3);
}

void TeamChange2IRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::ReferenceString playerName = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
			RenX::PlayerInfo *player;
			bool playerFound = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
					{
						player = node->data;
						if (player->name.findi(playerName) != Jupiter::INVALID_INDEX)
						{
							playerFound = true;
							if (server->changeTeam(player, false) == false)
								source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Server does not support team changing."));
						}
					}
				}
			}
			if (playerFound == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: team2 <player>"));
}

const Jupiter::ReadableString &TeamChange2IRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Changes a player's team, without resetting their credits. Syntax: team2 <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(TeamChange2IRCCommand)

/** Game Commands */

// Help Game Command

void HelpGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("help"));
}

void HelpGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	RenX::GameCommand *cmd;
	unsigned int cmdCount = 0;
	auto getAccessCommands = [&](int accessLevel)
	{
		Jupiter::String list;
		unsigned int i = 0;
		while (i != source->getCommandCount())
		{
			cmd = source->getCommand(i++);
			if (cmd->getAccessLevel() == accessLevel)
			{
				cmdCount++;
				list.format("Access level %d commands: %.*s", accessLevel, cmd->getTrigger().size(), cmd->getTrigger().ptr());
				break;
			}
		}
		while (i != source->getCommandCount())
		{
			cmd = source->getCommand(i++);
			if (cmd->getAccessLevel() == accessLevel)
			{
				cmdCount++;
				list += ", ";
				list += cmd->getTrigger();
			}
		}
		return list;
	};
	if (parameters.wordCount(WHITESPACE) == 0)
	{
		for (int i = 0; i <= player->access; i++)
		{
			Jupiter::ReadableString &msg = getAccessCommands(i);
			if (msg.isNotEmpty())
				source->sendMessage(player, getAccessCommands(i));
		}
		if (cmdCount == 0)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("No listed commands available."));
	}
	else
	{
		cmd = source->getCommand(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE));
		if (cmd != nullptr)
		{
			if (player->access >= cmd->getAccessLevel())
				source->sendMessage(player, cmd->getHelp(Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE)));
			else
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Access Denied."));
		}
		else
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Command not found."));
	}
}

const Jupiter::ReadableString &HelpGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Lists commands, or sends command-specific help. Syntax: help [command]");
	return defaultHelp;
}

GAME_COMMAND_INIT(HelpGameCommand)

// Mods Game Command

void ModsGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mods"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showmods"));
}

void ModsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	RenX::PlayerInfo *player;
	Jupiter::StringL msg;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = source->players.getNode(0); node != nullptr; node = node->next)
	{
		player = node->data;
		if (player->isBot == false && (player->adminType.isNotEmpty() || (player->access != 0 && (player->gamePrefix.isNotEmpty() || player->formatNamePrefix.isNotEmpty()))))
		{
			if (msg.isEmpty())
				msg = "Moderators in-game: "; 
			else
				msg += ", ";
			msg += player->gamePrefix;
			msg += player->name;
		}
	}
	if (msg.isEmpty())
	{
		msg += "No moderators are in-game";
		RenX::GameCommand *cmd = source->getCommand(STRING_LITERAL_AS_REFERENCE("modrequest"));
		if (cmd != nullptr)
			msg.aformat("; please use \"%.*s%.*s\" if you require assistance.", source->getCommandPrefix().size(), source->getCommandPrefix().ptr(), cmd->getTrigger().size(), cmd->getTrigger().ptr());
		else msg += '.';
	}
	source->sendMessage(msg);
}

const Jupiter::ReadableString &ModsGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays the in-game moderators. Syntax: mods");
	return defaultHelp;
}

GAME_COMMAND_INIT(ModsGameCommand)

// Rules Game Command

void RulesGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rules"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rule"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showrules"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showrule"));
}

void RulesGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(Jupiter::StringS::Format("Rules: %.*s", source->getRules().size(), source->getRules().ptr()));
}

const Jupiter::ReadableString &RulesGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays the rules for this server. Syntax: rules");
	return defaultHelp;
}

GAME_COMMAND_INIT(RulesGameCommand)

// Mod Request Game Command

void ModRequestGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("modrequest"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("requestmod"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mod"));
}

void ModRequestGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	unsigned int serverCount = serverManager->size();
	IRC_Bot *server;
	Jupiter::IRC::Client::Channel *channel;
	unsigned int channelCount;
	unsigned int messageCount = 0;
	int type;
	Jupiter::String &fmtName = RenX::getFormattedPlayerName(player);
	Jupiter::StringL msg = Jupiter::StringL::Format(IRCCOLOR "12[Moderator Request] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR "07 has requested assistance in-game; please look in ", fmtName.size(), fmtName.ptr());
	Jupiter::StringS msg2 = Jupiter::StringS::Format(IRCCOLOR "12[Moderator Request] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR "07 has requested assistance in-game!" IRCCOLOR, fmtName.size(), fmtName.ptr());
	for (unsigned int a = 0; a < serverCount; a++)
	{
		server = serverManager->getServer(a);
		if (server != nullptr)
		{
			channelCount = server->getChannelCount();
			for (unsigned int b = 0; b < channelCount; b++)
			{
				channel = server->getChannel(b);
				if (channel != nullptr)
				{
					type = channel->getType();
					if (source->isLogChanType(type))
					{
						server->sendMessage(channel->getName(), msg2);
						msg += channel->getName();
						for (unsigned int c = 0; c < channel->getUserCount(); c++)
						{
							if (channel->getUserPrefix(c) != 0 && channel->getUser(c)->getNickname().equals(server->getNickname()) == false)
							{
								server->sendMessage(channel->getUser(c)->getUser()->getNickname(), msg);
								messageCount++;
							}
						}
						msg -= channel->getName().size();
					}
				}
			}
		}
	}
	source->sendMessage(player, Jupiter::StringS::Format("A total of %u moderators have been notified of your assistance request.", messageCount));
}

const Jupiter::ReadableString &ModRequestGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Notifies the moderators on IRC that assistance is required. Syntax: modRequest");
	return defaultHelp;
}

GAME_COMMAND_INIT(ModRequestGameCommand)

// Kill Game Command

void KillGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kill"));
	this->setAccessLevel(1);
}

void KillGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not kill higher level moderators."));
		else
		{
			source->kill(target);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been killed."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: kill <player>"));
}

const Jupiter::ReadableString &KillGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kills a player in the game. Syntax: kill <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(KillGameCommand)

// Disarm Game Command

void DisarmGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarm"));
	this->setAccessLevel(1);
}

void DisarmGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not disarm higher level moderators."));
		else
		{
			source->disarm(target);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been disarmed."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: disarm <player>"));
}

const Jupiter::ReadableString &DisarmGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed objects in the game. Syntax: disarm <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmGameCommand)

// DisarmC4 Game Command

void DisarmC4GameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmc4"));
	this->setAccessLevel(1);
}

void DisarmC4GameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not disarm higher level moderators."));
		else
		{
			source->disarmC4(target);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been disarmed."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: disarmc4 <player>"));
}

const Jupiter::ReadableString &DisarmC4GameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed mines in the game. Syntax: disarmc4 <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmC4GameCommand)

// DisarmBeacon Game Command

void DisarmBeaconGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacons"));
	this->setAccessLevel(1);
}

void DisarmBeaconGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not disarm higher level moderators."));
		else
		{
			source->disarmBeacon(target);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been disarmed."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: disarmb <player>"));
}

const Jupiter::ReadableString &DisarmBeaconGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed beacons in the game. Syntax: disarmb <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmBeaconGameCommand)

// Kick Game Command

void KickGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("qkick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("k"));
	this->setAccessLevel(1);
}

void KickGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::StringS name = Jupiter::StringS::getWord(parameters, 0, WHITESPACE);
		Jupiter::StringS reason = parameters.wordCount(WHITESPACE) > 1 ? Jupiter::StringS::gotoWord(parameters, 1, WHITESPACE) : STRING_LITERAL_AS_REFERENCE("No reason");
		RenX::PlayerInfo *target = source->getPlayerByPartName(name);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (player == target)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not kick yourself."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not kick higher level moderators."));
		else
		{
			source->kickPlayer(target, reason);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been kicked from the game."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: kick <player> [Reason]"));
}

const Jupiter::ReadableString &KickGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks a player from the game. Syntax: kick <player> [Reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(KickGameCommand)

// TempBan Game Command

void TempBanGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tempbank"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tb"));
	this->setAccessLevel(1);
}

void TempBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::StringS name = Jupiter::StringS::getWord(parameters, 0, WHITESPACE);
		Jupiter::StringS reason = parameters.wordCount(WHITESPACE) > 1 ? Jupiter::StringS::gotoWord(parameters, 1, WHITESPACE) : STRING_LITERAL_AS_REFERENCE("No reason");
		RenX::PlayerInfo *target = source->getPlayerByPartName(name);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (player == target)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not ban yourself."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not ban higher level moderators."));
		else
		{
			target->varData.set(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("banner"), player->name);
			source->banPlayer(target, reason, pluginInstance.getTBanTime());
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been temporarily banned and kicked from the game."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: tban <player> [Reason]"));
}

const Jupiter::ReadableString &TempBanGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and temporarily bans a player from the game. Syntax: tban <player> [Reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(TempBanGameCommand)

// KickBan Game Command

void KickBanGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kickban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("b"));
	this->setAccessLevel(2);
}

void KickBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::StringS name = Jupiter::StringS::getWord(parameters, 0, WHITESPACE);
		Jupiter::StringS reason = parameters.wordCount(WHITESPACE) > 1 ? Jupiter::StringS::gotoWord(parameters, 1, WHITESPACE) : STRING_LITERAL_AS_REFERENCE("No reason");
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (player == target)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not ban yourself."));
		else if (target->access >= player->access)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not ban higher level moderators."));
		else
		{
			target->varData.set(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("banner"), player->name);
			source->banPlayer(target, reason);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player has been banned and kicked from the game."));
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: ban <player> [reason]"));
}

const Jupiter::ReadableString &KickBanGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and bans a player from the game. Syntax: ban <player> [reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(KickBanGameCommand)

// AddBots Game Command

void AddBotsGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("abots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbot"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("abot"));
	this->setAccessLevel(1);
}

void AddBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	RenX::TeamType team = RenX::getTeam(Jupiter::ReferenceString::getWord(parameters, 1, WHITESPACE));

	Jupiter::StringS cmd;
	switch (team)
	{
	case RenX::TeamType::GDI:
		cmd = "addredbots ";
		break;
	case RenX::TeamType::Nod:
		cmd = "addbluebots ";
		break;
	default:
	case RenX::TeamType::None:
	case RenX::TeamType::Other:
		cmd = "addbots ";
		break;
	}

	unsigned int amount;
	if (parameters.isEmpty())
		amount = 1;
	else
		amount = parameters.asUnsignedInt();
	cmd += Jupiter::StringS::Format("%u", amount);

	source->send(cmd);
	source->sendMessage(player, Jupiter::StringS::Format("%u bots have been added to the server.", amount));
}

const Jupiter::ReadableString &AddBotsGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds bots to the game. Syntax: addbots [amount=1] [team]");
	return defaultHelp;
}

GAME_COMMAND_INIT(AddBotsGameCommand)

// KillBots Game Command

void KillBotsGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rembots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rbots"));
	this->setAccessLevel(2);
}

void KillBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	source->send(STRING_LITERAL_AS_REFERENCE("killbots"));
	source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("All bots have been removed from the server."));
}

const Jupiter::ReadableString &KillBotsGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes all bots from the game. Syntax: killbots");
	return defaultHelp;
}

GAME_COMMAND_INIT(KillBotsGameCommand)

// PhaseBots Game Command

void PhaseBotsGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("phasebots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pbots"));
	this->setAccessLevel(1);
}

void PhaseBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
	{
		if (togglePhasing(source))
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
		else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
	}
	else if (parameters.equalsi("true") || parameters.equalsi("on") || parameters.equalsi("start") || parameters.equalsi("1"))
	{
		togglePhasing(source, true);
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
	}
	else
	{
		togglePhasing(source, false);
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
	}
}

const Jupiter::ReadableString &PhaseBotsGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes all bots from the game. Syntax: phasebots");
	return defaultHelp;
}

GAME_COMMAND_INIT(PhaseBotsGameCommand)

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
