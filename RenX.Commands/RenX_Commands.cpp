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

#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "RenX_Commands.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"

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
	if (player->isBot && server->varData.getBool(STRING_LITERAL_AS_REFERENCE("RenX.Commands"), STRING_LITERAL_AS_REFERENCE("phasing"), false)) server->kickPlayer(player);
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

/** Console Commands */

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
	{
		RenX::Server *server;
		while (--i >= 0)
		{
			server = RenX::getCore()->getServer(i);
			RenX::getCore()->send(server, parameters);
		}
	}
	else
		puts("Error: Too Few Parameters. Syntax: rcon <input>");
}

const Jupiter::ReadableString &RCONConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends data to the Renegade X server's rcon. Syntax: rcon <input>");
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
	if (parameters.isEmpty() == false)
	{	
		Jupiter::StringL msg = "say ";
		msg += source->getChannel(channel)->getUserPrefix(nick);
		msg += nick;
		msg += "@IRC: ";
		msg += parameters;
		if (RenX::getCore()->send(source->getChannel(channel)->getType(), msg) == 0)
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
	if (parameters.isEmpty() == false)
	{
		if (RenX::getCore()->send(source->getChannel(channel)->getType(), Jupiter::StringS::Format("say %.*s", parameters.size(), parameters.ptr())) == 0)
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

// PlayerInfo IRC Command

void PlayerInfoIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playerinfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pi"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("player"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pinfo"));
	this->setAccessLevel(2);
}

void PlayerInfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.size() != 0)
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
							const Jupiter::ReadableString &teamColor = RenX::getTeamColor(player->team);
							const Jupiter::ReadableString &teamName = RenX::getFullTeamName(player->team);
							msg.format(IRCCOLOR "03[Player Info]" IRCCOLOR "%.*s Name: " IRCBOLD "%.*s" IRCBOLD " - ID: %d - Team: " IRCBOLD "%.*s" IRCBOLD " - Vehicle Kills: %u - Defence Kills: %u - Building Kills: %u - Kills: %u (%u headshots) - Deaths: %u (%u suicides) - KDR: %.2f", teamColor.size(), teamColor.ptr(), player->name.size(), player->name.ptr(), player->id, teamName.size(), teamName.ptr(), player->vehicleKills, player->defenceKills, player->buildingKills, player->kills, player->headshots, player->deaths, player->suicides, ((float)player->kills) / (player->deaths == 0 ? 1.0 : (float)player->deaths));
							if (source->getAccessLevel(channel, nick) > 1)
							{
								msg += " - IP: " IRCBOLD;
								if (player->ip.size() != 0)
								{
									msg += player->ip;
									msg += IRCBOLD;
								}
								else msg += "IP Not Found" IRCBOLD;
							}
							if (player->steamid != 0)
								msg.aformat(" - Steam ID: " IRCBOLD "0x%.16llX" IRCBOLD, player->steamid);
							if (player->adminType.size() != 0)
							{
								msg += " - Admin Type: " IRCBOLD;
								msg += player->adminType;
								msg += IRCBOLD;
							}
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
		if (parameters.size() != 0)
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
								msg.aformat("is using steam ID " IRCBOLD "0x%.16llX" IRCBOLD, player->steamid);
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
	if (parameters.size() != 0)
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
							msg.format(IRCBOLD "%.*s" IRCBOLD IRCCOLOR ": Kills: %u - Deaths: %u - KDR: %.2f", playerName.size(), playerName.ptr(), player->kills, player->deaths, ((float)player->kills) / (player->deaths == 0 ? 1.0 : (float)player->deaths));
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
							if (player->adminType.size() != 0)
							{
								if (msg.size() != 0) msg += ", ";
								else msg += "Moderators in-game: ";
								msg += player->name;
							}
						}
					}
					if (msg.size() == 0) msg = "No moderators are in-game.";
					source->sendMessage(channel, msg);
				}
			}
			if (msg.size() == 0) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
}

const Jupiter::ReadableString &ModsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying the in-game moderators. Syntax: mods [show]");
	return defaultHelp;
}

IRC_COMMAND_INIT(ModsIRCCommand)

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
		if (msg.size() == 0) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
			if (msg.size() == 0) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	if (parameters.size() != 0)
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
	this->setAccessLevel(4);
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
				if (server->reconnect()) msg.format("Connection established");
				else msg.format("[RenX] ERROR: Failed to connect to %.*s on port %u." ENDL, server->getHostname().size(), server->getHostname().ptr(), server->getPort());
				source->sendMessage(channel, msg);
			}
		}
		if (msg.isEmpty())
		{
			// We didn't connect anywhere!!
			msg.format("ERROR: No servers found to connect to.");
			source->sendMessage(channel, msg);
		}
	}
}

const Jupiter::ReadableString &ReconnectIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets information about a player. Syntax: Reconnect");
	return defaultHelp;
}

IRC_COMMAND_INIT(ReconnectIRCCommand)

// RestartMap IRC Command

/*void RestartMapIRCCommand::create()
{
	this->addTrigger("restartmap");
	this->setAccessLevel(2);
}

void RestartMapIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	int r = RenX::getCore()->send(source->getChannel(channel.c_str())->getType(), "adminrestartmap");
	if (r > 0)
	{
		char t[256];
		sprintf(t, "Command sent to %d servers.", r);
		source->sendMessage(channel.c_str(), t);
	}
	else source->sendMessage(channel.c_str(), "Error: Channel not attached to any connected Renegade X servers.");
}

const char *RestartMapIRCCommand::getHelp()
{
	return "Restarts the current map. Syntax: RestartMap";
}

IRC_COMMAND_INIT(RestartMapIRCCommand)

// SetMap IRC Command

void SetMapIRCCommand::create()
{
	this->addTrigger("setmap");
	this->setAccessLevel(3);
}

void SetMapIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters != nullptr)
	{
		Jupiter::StringL cmd = "adminrestartmap ";
		cmd += parameters;
		int r = RenX::getCore()->send(source->getChannel(channel.c_str())->getType(), cmd.c_str());
		if (r > 0)
		{
			char t[256];
			sprintf(t, "Command sent to %d servers.", r);
			source->sendMessage(channel.c_str(), t);
		}
		else source->sendMessage(channel.c_str(), "Error: Channel not attached to any connected Renegade X servers.");
	}
	else source->sendNotice(nick.c_str(), "Error: Too Few Parameters. Syntax: SetMap <Map Name>");
}

const char *SetMapIRCCommand::getHelp()
{
	return "Sets the next map, and ends the current map. Syntax: SetMap <Map Name>";
}

IRC_COMMAND_INIT(SetMapIRCCommand)*/

// Mute IRC Command

/*void MuteIRCCommand::create()
{
	this->addTrigger("mute");
	this->addTrigger("silence");
	this->setAccessLevel(2);
}

void MuteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters != nullptr)
	{
		Jupiter::StringL cmd = "AdminForceTextMute ";
		cmd += parameters;
		int r = RenX::getCore()->send(source->getChannel(channel.c_str())->getType(), cmd.c_str());
		if (r > 0)
		{
			char t[256];
			sprintf(t, "Command sent to %d servers.", r);
			source->sendMessage(channel.c_str(), t);
		}
		else source->sendMessage(channel.c_str(), "Error: Channel not attached to any connected Renegade X servers.");
	}
	else source->sendNotice(nick.c_str(), "Error: Too Few Parameters. Syntax: Mute <Player Name>");
}

const char *MuteIRCCommand::getHelp()
{
	return "Mutes a player from the game chat. Syntax: Mute <Player Name>";
}

IRC_COMMAND_INIT(MuteIRCCommand)

// UnMute IRC Command

void UnMuteIRCCommand::create()
{
	this->addTrigger("unmute");
	this->setAccessLevel(2);
}

void UnMuteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters != nullptr)
	{
		Jupiter::StringL cmd = "AdminForceTextUnMute ";
		cmd += parameters;
		int r = RenX::getCore()->send(source->getChannel(channel.c_str())->getType(), cmd.c_str());
		if (r > 0)
		{
			char t[256];
			sprintf(t, "Command sent to %d servers.", r);
			source->sendMessage(channel.c_str(), t);
		}
		else source->sendMessage(channel.c_str(), "Error: Channel not attached to any connected Renegade X servers.");
	}
	else source->sendNotice(nick.c_str(), "Error: Too Few Parameters. Syntax: UnMute <Player Name>");
}

const char *UnMuteIRCCommand::getHelp()
{
	return "UnMutes a player from the game chat. Syntax: UnMute <Player Name>";
}

IRC_COMMAND_INIT(UnMuteIRCCommand)*/

// Kick IRC Command

void KickIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("qkick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("k"));
	this->setAccessLevel(3);
}

void KickIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.size() != 0)
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
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(parameters);
						if (player != nullptr)
						{
							server->kickPlayer(player);
							kicks++;
						}
					}
				}
				source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Kick <Player Name>"));
}

const Jupiter::ReadableString &KickIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks a player from the game. Syntax: Kick <Player Name>");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickIRCCommand)

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
	if (parameters.size() != 0)
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
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(parameters);
						if (player != nullptr)
						{
							server->banPlayer(player);
							kicks++;
						}
					}
				}
				source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: KickBan <Player Name>"));
}

const Jupiter::ReadableString &KickBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and bans a player from the game. Syntax: KickBan <Player Name>");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickBanIRCCommand)

// AddBots IRC Command

void AddBotsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbot"));
	this->setAccessLevel(2);
}

void AddBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.size() != 0)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(source->getChannel(channel)->getType());
			if (servers.size() != 0)
			{
				int amount = parameters.asInt();
				if (amount != 0)
				{
					RenX::Server *server;
					Jupiter::StringL cmd;
					Jupiter::ReferenceString targetTeam = Jupiter::ReferenceString::getWord(parameters, 1, WHITESPACE);
					RenX::TeamType team = targetTeam.isEmpty() ? RenX::TeamType::Other : RenX::getTeam(targetTeam[0]);

					switch (team)
					{
					case RenX::TeamType::GDI:
						cmd = "addredbots ";
						break;
					case RenX::TeamType::Nod:
						cmd = "addbluebots ";
						break;
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
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: AddBots <Amount> [Team]"));
}

const Jupiter::ReadableString &AddBotsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds bots to the game. Syntax: AddBots <Amount> [Team]");
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
				if (parameters.size() == 0)
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

/** Game Commands */

// Help Game Command

void HelpGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("help"));
}

void HelpGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	Jupiter::StringL output = "say Available Commands: ";
	parameters.println(stdout);
	if (parameters.wordCount(WHITESPACE) == 0)
	{
		// We know there's at least one command, because of this command!
		output += source->getCommand(0)->getTrigger();
		for (unsigned int i = 1; i != source->getCommandCount(); i++)
		{
			output += ", ";
			output += source->getCommand(i)->getTrigger();
		}
	}
	else
	{
		RenX::GameCommand *cmd = source->getCommand(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE));
		if (cmd != nullptr)
			output += cmd->getHelp(Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE));
		else output += "Error: Command not found.";
	}
	source->send(output);
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
	Jupiter::StringL msg = "say ";
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = source->players.getNode(0); node != nullptr; node = node->next)
	{
		player = node->data;
		if (player->adminType.size() != 0)
		{
			if (msg.size() != 4) msg += ", ";
			else msg += "Moderators in-game: ";
			msg += player->name;
		}
	}
	if (msg.size() == 4)
	{
		msg += "No moderators are in-game";
		RenX::GameCommand *cmd = source->getCommand(STRING_LITERAL_AS_REFERENCE("modrequest"));
		if (cmd != nullptr)
			msg.aformat("; please use \"%.*s%.*s\" if you require assistance.", source->getCommandPrefix().size(), source->getCommandPrefix().ptr(), cmd->getTrigger().size(), cmd->getTrigger().ptr());
		else msg += '.';
	}
	source->send(msg);
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
					server->sendMessage(channel->getName(), msg2);
					if (source->isLogChanType(type))
					{
						msg += channel->getName();
						for (unsigned int c = 0; c < channel->getUserCount(); c++)
							if (channel->getUserPrefix(c) != 0)
								server->sendMessage(channel->getUser(c)->getUser()->getNickname(), msg);
						msg -= channel->getName().size();
					}
				}
			}
		}
	}
}

const Jupiter::ReadableString &ModRequestGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Notifies the moderators on IRC that assistance is required. Syntax: modRequest");
	return defaultHelp;
}

GAME_COMMAND_INIT(ModRequestGameCommand)

// Plugin instantiation and entry point.
RenX_CommandsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
