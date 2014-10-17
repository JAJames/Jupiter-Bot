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

#include <ctime>
#include "Jupiter/INIFile.h"
#include "Jupiter/String.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_GameCommand.h"
#include "RenX_Functions.h"
#include "RenX_Plugin.h"

int RenX::Server::think()
{
	if (RenX::Server::connected == false)
	{
		if (time(0) >= RenX::Server::lastAttempt + RenX::Server::delay)
		{
			if (RenX::Server::connect())
				RenX::Server::sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to Renegade-X server.");
			else RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to Renegade-X server.");
		}
	}
	else
	{
		if (RenX::Server::sock.recv() > 0)
		{
			Jupiter::ReferenceString buffer = RenX::Server::sock.getBuffer();
			unsigned int totalLines = buffer.tokenCount('\n');

			RenX::Server::lastLine.concat(buffer.getToken(0, '\n'));
			if (totalLines != 0)
			{
				RenX::Server::processLine(RenX::Server::lastLine);
				RenX::Server::lastLine = buffer.getToken(totalLines - 1, '\n');

				for (unsigned int currentLine = 1; currentLine != totalLines - 1; currentLine++)
					RenX::Server::processLine(buffer.getToken(currentLine, '\n'));
			}
		}
		else if (Jupiter::Socket::getLastError() != 10035) // This is a serious error
		{
			RenX::Server::wipeData();
			RenX::Server::sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt in progress.");
			if (RenX::Server::reconnect())
				RenX::Server::sendLogChan(IRCCOLOR "06[Progress]" IRCCOLOR " Connection to Renegade-X server reestablished. Initializing Renegade-X RCON protocol...");
			else
				RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt failed.");
		}
	}
	return 0;
}

bool RenX::Server::isConnected() const
{
	return RenX::Server::connected;
}

bool RenX::Server::isFirstGame() const
{
	return RenX::Server::firstGame;
}

bool RenX::Server::isFirstKill() const
{
	return RenX::Server::firstKill;
}

bool RenX::Server::isFirstDeath() const
{
	return RenX::Server::firstDeath;
}

bool RenX::Server::isPublicLogChanType(int type) const
{
	return RenX::Server::logChanType == type;
}

bool RenX::Server::isAdminLogChanType(int type) const
{
	return RenX::Server::adminLogChanType == type;
}

bool RenX::Server::isLogChanType(int type) const
{
	return RenX::Server::isPublicLogChanType(type) || RenX::Server::isAdminLogChanType(type);
}

int RenX::Server::send(const Jupiter::ReadableString &command)
{
	char *t = new char[command.size() + 2];
	*t = 'c';
	for (size_t i = 0; i != command.size(); i++) t[i + 1] = command.get(i);
	t[command.size() + 1] = '\n';
	int r;
	if (RenX::Server::profile->mustSanitize)
		RenX::sanitizeString(t);
	r = RenX::Server::sock.send(t, command.size() + 2);
	delete[] t;
	return r;
}

int RenX::Server::sendMessage(const Jupiter::ReadableString &message)
{
	int r = 0;
	if (RenX::Server::neverSay)
	{
		if (RenX::Server::profile->privateMessages && RenX::Server::players.size() != 0)
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
				if (node->data->isBot == false)
					r += RenX::Server::sock.send(Jupiter::StringS::Format("cevaprivatesay pid%d %.*s\n", node->data->id, message.size(), message.ptr()));
		return r;
	}
	else
	{
		char *t = new char[message.size() + 6];
		strcpy(t, "csay ");
		for (size_t i = 0; i != message.size(); i++) t[i + 5] = message.get(i);
		t[message.size() + 5] = '\n';
		if (RenX::Server::profile->mustSanitize)
			RenX::sanitizeString(t);
		r = RenX::Server::sock.send(t, message.size() + 6);
		delete[] t;
		return r;
	}
}

int RenX::Server::sendMessage(RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	if (RenX::Server::profile->privateMessages == false)
		return RenX::Server::sendMessage(message);

	return RenX::Server::sock.send(Jupiter::StringS::Format("cevaprivatesay pid%d %.*s\n", player->id, message.size(), message.ptr()));
}

int RenX::Server::sendData(const Jupiter::ReadableString &data)
{
	return RenX::Server::sock.send(data);
}

RenX::PlayerInfo *RenX::Server::getPlayer(int id) const
{
	if (RenX::Server::players.size() == 0) return nullptr;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next) if (node->data->id == id) return node->data;
	return nullptr;
}

RenX::PlayerInfo *RenX::Server::getPlayerByName(const Jupiter::ReadableString &name) const
{
	if (RenX::Server::players.size() == 0) return nullptr;

	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
		if (node->data->name == name)
			return node->data;

	Jupiter::ReferenceString idToken = name;
	if (name.matchi("Player?*"))
		idToken.shiftRight(6);
	else if (name.matchi("pid?*"))
		idToken.shiftRight(3);
	else return nullptr;
	int id = idToken.asInt(10);

	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
		if (node->data->id == id)
			return node->data;

	return nullptr;
}

RenX::PlayerInfo *RenX::Server::getPlayerByPartName(const Jupiter::ReadableString &partName) const
{
	if (RenX::Server::players.size() == 0) return nullptr;
	RenX::PlayerInfo *r = RenX::Server::getPlayerByName(partName);
	if (r != nullptr) return r;
	return RenX::Server::getPlayerByPartNameFast(partName);
}

RenX::PlayerInfo *RenX::Server::getPlayerByPartNameFast(const Jupiter::ReadableString &partName) const
{
	if (RenX::Server::players.size() == 0) return nullptr;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
		if (node->data->name.findi(partName) != Jupiter::INVALID_INDEX)
			return node->data;
	return nullptr;
}

Jupiter::StringS RenX::Server::formatSteamID(const RenX::PlayerInfo *player)
{
	return RenX::Server::formatSteamID(player->steamid);
}

Jupiter::StringS RenX::Server::formatSteamID(uint64_t id)
{
	switch (RenX::Server::steamFormat)
	{
	default:
	case 16:
		return Jupiter::StringS::Format("0x%.16llX", id);
	case 10:
		return Jupiter::StringS::Format("%llu", id);
	case 8:
		return Jupiter::StringS::Format("0%llo", id);
	case -2:
		id -= 0x0110000100000000ULL;
		if (id % 2 == 1)
			return Jupiter::StringS::Format("STEAM_1:1:%llu", id / 2ULL);
		else
			return Jupiter::StringS::Format("STEAM_1:0:%llu", id / 2ULL);
	case -3:
		id -= 0x0110000100000000ULL;
		return Jupiter::StringS::Format("[U:1:%llu]", id);
	}
}

void RenX::Server::kickPlayer(int id)
{
	RenX::Server::sock.send(Jupiter::StringS::Format("cadminkick pid%d\n", id));
}

void RenX::Server::kickPlayer(const RenX::PlayerInfo *player)
{
	if (this->profile->pidbug)
	{
		if (player->isBot)
			RenX::Server::sock.send(Jupiter::StringS::Format("cadminkick %.*s\n", player->name.size(), player->name.ptr()));
		else if (player->id < 1000)
			RenX::Server::kickPlayer(player->id);
		else if (player->name.contains('|') == false)
			RenX::Server::sock.send(Jupiter::StringS::Format("cadminkick %.*s\n", player->name.size(), player->name.ptr()));
		else
			RenX::Server::kickPlayer(player->id);
	}
	else
		RenX::Server::kickPlayer(player->id);
}

void RenX::Server::banPlayer(int id)
{
	RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban pid%d\n", id));
}

void RenX::Server::banPlayer(const RenX::PlayerInfo *player)
{
	if (this->profile->pidbug)
	{
		if (player->isBot)
			RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban %.*s\n", player->name.size(), player->name.ptr()));
		else if (player->id < 1000)
			RenX::Server::banPlayer(player->id);
		else if (player->name.contains('|') == false)
			RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban %.*s\n", player->name.size(), player->name.ptr()));
		else
			RenX::Server::banPlayer(player->id);
	}
	else
		RenX::Server::banPlayer(player->id);
}

bool RenX::Server::removePlayer(int id)
{
	if (RenX::Server::players.size() == 0) return false;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
	{
		if (node->data->id == id)
		{
			RenX::PlayerInfo *p = RenX::Server::players.remove(node);
			Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnPlayerDelete(this, p);
			delete p;
			return true;
		}
	}
	return false;
}

bool RenX::Server::removePlayer(RenX::PlayerInfo *player)
{
	return RenX::Server::removePlayer(player->id);
}

const Jupiter::ReadableString &RenX::Server::getPrefix() const
{
	return RenX::Server::IRCPrefix;
}

void RenX::Server::setPrefix(const Jupiter::ReadableString &prefix)
{
	RenX::Server::IRCPrefix = prefix;
}

const Jupiter::ReadableString &RenX::Server::getCommandPrefix() const
{
	return RenX::Server::CommandPrefix;
}

void RenX::Server::setCommandPrefix(const Jupiter::ReadableString &prefix)
{
	RenX::Server::CommandPrefix = prefix;
}

const Jupiter::ReadableString &RenX::Server::getRules() const
{
	return RenX::Server::rules;
}

void RenX::Server::setRules(const Jupiter::ReadableString &rules)
{
	RenX::Server::rules = rules;
	Jupiter::IRC::Client::Config->set(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Rules"), rules);
	RenX::Server::sendMessage(Jupiter::StringS::Format("NOTICE: The rules have been modified! Rules: %.*s", rules.size(), rules.ptr()));
}

const Jupiter::ReadableString &RenX::Server::getHostname() const
{
	return RenX::Server::hostname;
}

unsigned short RenX::Server::getPort() const
{
	return RenX::Server::port;
}

time_t RenX::Server::getLastAttempt() const
{
	return RenX::Server::lastAttempt;
}

time_t RenX::Server::getDelay() const
{
	return RenX::Server::delay;
}

const Jupiter::ReadableString &RenX::Server::getPassword() const
{
	return RenX::Server::pass;
}

RenX::GameCommand *RenX::Server::getCommand(unsigned int index) const
{
	return RenX::Server::commands.get(index);
}

RenX::GameCommand *RenX::Server::getCommand(const Jupiter::ReadableString &trigger) const
{
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
	{
		RenX::GameCommand *cmd = RenX::Server::commands.get(i);
		if (cmd->matches(trigger))
			return cmd;
	}
	return nullptr;
}

unsigned int RenX::Server::getCommandCount() const
{
	return RenX::Server::commands.size();
}

unsigned int RenX::Server::triggerCommand(const Jupiter::ReadableString &trigger, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	unsigned int r = 0;
	RenX::GameCommand *cmd;
	for (size_t i = 0; i < RenX::Server::commands.size(); i++)
	{
		cmd = RenX::Server::commands.get(i);
		if (cmd->matches(trigger))
		{
			cmd->trigger(this, player, parameters);
			r++;
		}
	}
	return r;
}

void RenX::Server::addCommand(RenX::GameCommand *command)
{
	RenX::Server::commands.add(command);
}

bool RenX::Server::removeCommand(RenX::GameCommand *command)
{
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
	{
		if (RenX::Server::commands.get(i) == command)
		{
			delete RenX::Server::commands.remove(i);
			return true;
		}
	}
	return false;
}

bool RenX::Server::removeCommand(const Jupiter::ReadableString &trigger)
{
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
	{
		if (RenX::Server::commands.get(i)->matches(trigger))
		{
			delete RenX::Server::commands.remove(i);
			return true;
		}
	}
	return false;
}

void RenX::Server::sendPubChan(const char *fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = RenX::Server::IRCPrefix;
	if (serverPrefix.isEmpty() == false)
	{
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else msg.vformat(fmt, args);
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++)
		serverManager->getServer(i)->messageChannels(RenX::Server::logChanType, msg);
}

void RenX::Server::sendAdmChan(const char *fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = RenX::Server::getPrefix();
	if (serverPrefix.isEmpty() == false)
	{
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else msg.vformat(fmt, args);
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++)
		serverManager->getServer(i)->messageChannels(RenX::Server::adminLogChanType, msg);
}

void RenX::Server::sendLogChan(const char *fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = RenX::Server::getPrefix();
	if (serverPrefix.isEmpty() == false)
	{
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else msg.vformat(fmt, args);
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++)
	{
		IRC_Bot *ircServer = serverManager->getServer(i);
		ircServer->messageChannels(RenX::Server::logChanType, msg);
		ircServer->messageChannels(RenX::Server::adminLogChanType, msg);
	}
}

#define PARSE_PLAYER_DATA() \
	Jupiter::ReferenceString name; \
	TeamType team; \
	int id; \
	bool isBot = false; { \
		Jupiter::ReferenceString idToken; \
		if (playerData[0] == ',') { \
			name = playerData.gotoWord(1, ","); \
			idToken = playerData.getWord(0, ","); \
			team = Other; \
		} else { \
			name = playerData.gotoWord(2, ","); \
			idToken = playerData.getWord(1, ","); \
			team = RenX::getTeam(playerData[0]); \
		} \
		if (idToken[0] == 'b') { idToken.shiftRight(1); isBot = true; } \
		id = idToken.asInt(10); } \
	RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot);

inline RenX::PlayerInfo *getPlayerOrAdd(RenX::Server *server, const Jupiter::ReadableString &name, int id, RenX::TeamType team, bool isBot)
{
	RenX::PlayerInfo *r = server->getPlayer(id);
	if (r == nullptr)
	{
		r = new RenX::PlayerInfo();
		r->id = id;
		r->name = name;
		r->isBot = isBot;
		r->joinTime = time(nullptr);
		if (id != 0) server->players.add(r);
		Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
		for (size_t i = 0; i < xPlugins.size(); i++)
			xPlugins.get(i)->RenX_OnPlayerCreate(server, r);
	}
	else if (r->name.size() == 0) r->name = name;
	r->team = team;
	return r;
}

void RenX::Server::processLine(const Jupiter::ReadableString &line)
{
	/** Local functions */
	auto onPreGameOver = [&](RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
	{
		RenX::PlayerInfo *player;

		if (server->players.size() != 0)
		{
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				if (player != nullptr)
				{
					if (player->team == team)
						player->wins++;
					else player->loses++;
				}
			}
		}
	};
	auto onPostGameOver = [&](RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
	{
		this->firstGame = false;
		this->firstAction = false;
		this->firstKill = false;
		this->firstDeath = false;
		RenX::PlayerInfo *player;

		if (server->players.size() != 0)
		{
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				if (player != nullptr)
				{
					player->kills = 0;
					player->deaths = 0;
					player->suicides = 0;
					player->headshots = 0;
					player->vehicleKills = 0;
					player->buildingKills = 0;
					player->defenceKills = 0;
				}
			}
		}
	};
	auto onChat = [&](RenX::Server *server, RenX::PlayerInfo *player, const Jupiter::ReadableString &message, bool isPublic)
	{
		const Jupiter::ReadableString &prefix = server->getCommandPrefix();
		if (message.find(prefix) == 0 && message.size() != prefix.size())
		{
			Jupiter::ReferenceString command;
			Jupiter::ReferenceString parameters;
			if (containsSymbol(WHITESPACE, message.get(prefix.size())))
			{
				command = Jupiter::ReferenceString::getWord(message, 1, WHITESPACE);
				parameters = Jupiter::ReferenceString::gotoWord(message, 2, WHITESPACE);
			}
			else
			{
				command = Jupiter::ReferenceString::getWord(message, 0, WHITESPACE);
				command.shiftRight(prefix.size());
				parameters = Jupiter::ReferenceString::gotoWord(message, 1, WHITESPACE);
			}
			server->triggerCommand(command, player, parameters);
		}
	};
	auto onAction = [&]()
	{
		if (this->firstAction == false)
		{
			this->firstAction = true;
			this->silenceJoins = false;
		}
	};

	Jupiter::ReferenceString buff = line;
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	Jupiter::ReferenceString header = buff.getWord(0, RenX::DelimS);
	Jupiter::ReferenceString playerData = buff.getWord(1, RenX::DelimS);
	Jupiter::ReferenceString action = buff.getWord(2, RenX::DelimS);
	if (buff.size() != 0)
	{
		switch (header[0])
		{
		case 'l':
			if (header.equals("lGAME:"))
			{
				if (action.equals("deployed"))
				{
					PARSE_PLAYER_DATA();
					Jupiter::ReferenceString objectType = buff.getWord(3, RenX::DelimS);
					if (objectType.match("*Beacon")) player->beaconPlacements++;
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnDeploy(this, player, objectType);
					onAction();
				}
				else if (action.equals("suicided by"))
				{
					PARSE_PLAYER_DATA();
					player->deaths++;
					player->suicides++;
					Jupiter::ReferenceString damageType = buff.getWord(3, RenX::DelimS);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnSuicide(this, player, damageType);
					this->firstDeath = true;
					onAction();
				}
				else if (action.equals("killed"))
				{
					PARSE_PLAYER_DATA();
					Jupiter::ReferenceString victimData = buff.getWord(3, RenX::DelimS);
					Jupiter::ReferenceString vname = victimData.getWord(2, ",");
					Jupiter::ReferenceString vidToken = victimData.getWord(1, ",");
					int vid;
					bool visBot = false;
					if (vidToken[0] == 'b')
					{
						vidToken.shiftRight(1);
						visBot = true;
					}
					vid = vidToken.asInt(10);
					TeamType vteam = RenX::getTeam(victimData.getWord(0, ",")[0]);
					Jupiter::ReferenceString damageType = buff.getWord(5, RenX::DelimS);
					RenX::PlayerInfo *victim = getPlayerOrAdd(this, vname, vid, vteam, visBot);
					player->kills++;
					if (damageType.equals("Rx_DmgType_Headshot")) player->headshots++;
					victim->deaths++;

					if (this->needsCList)
					{
						this->sendData(STRING_LITERAL_AS_REFERENCE("clogclientlist\n"));
						this->needsCList = false;
					}

					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnKill(this, player, victim, damageType);

					this->firstKill = true;
					this->firstDeath = true;
					onAction();
				}
				else if (action.match("died by"))
				{
					PARSE_PLAYER_DATA();
					player->deaths++;
					Jupiter::ReferenceString damageType = buff.getWord(3, RenX::DelimS);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnDie(this, player, damageType);
					this->firstDeath = true;
					onAction();
				}
				else if (action.match("destroyed*"))
				{
					PARSE_PLAYER_DATA();
					Jupiter::ReferenceString victim = buff.getWord(3, RenX::DelimS);
					Jupiter::ReferenceString damageType = buff.getWord(5, RenX::DelimS);
					ObjectType type;
					if (action.equals("destroyed building"))
					{
						type = Building;
						player->buildingKills++;
					}
					else if (victim.match("Rx_Defence_*"))
					{
						type = Defence;
						player->defenceKills++;
					}
					else
					{
						type = Vehicle;
						player->vehicleKills++;
					}
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnDestroy(this, player, victim, damageType, type);
					onAction();
				}
				else if (playerData.match("??? wins (*)"))
				{
					TeamType team = RenX::getTeam(playerData[0]);
					int gScore = buff.getWord(2, RenX::DelimS).gotoWord(1, "=").asInt(10);
					int nScore = buff.getWord(3, RenX::DelimS).gotoWord(1, "=").asInt(10);
					Jupiter::ReferenceString winType = Jupiter::ReferenceString::substring(playerData, 10);
					winType.truncate(1);
					WinType iWinType = Unknown;
					if (gScore == nScore)
						iWinType = Tie;
					else if (winType.equals("TimeLimit"))
						iWinType = Score;
					else if (winType.equals("Buildings"))
						iWinType = Base;

					this->needsCList = true;
					if (this->profile->disconnectOnGameOver)
						this->silenceParts = true;

					onPreGameOver(this, iWinType, team, gScore, nScore);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnGameOver(this, iWinType, team, gScore, nScore);
					onPostGameOver(this, iWinType, team, gScore, nScore);
				}
				else if (playerData.equals("Tie"))
				{
					int gScore = action.gotoWord(1, "=").asInt(10);
					int nScore = buff.getWord(3, RenX::DelimS).gotoWord(1, "=").asInt(10);

					this->needsCList = true;
					if (this->profile->disconnectOnGameOver)
						this->silenceParts = true;

					if (gScore == nScore)
					{
						onPreGameOver(this, Tie, Other, gScore, nScore);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnGameOver(this, Tie, Other, gScore, nScore);
						onPostGameOver(this, Tie, Other, gScore, nScore);
					}
					else
					{
						TeamType winTeam = gScore > nScore ? RenX::getTeam('G') : RenX::getTeam('N');
						onPreGameOver(this, Shutdown, winTeam, gScore, nScore);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnGameOver(this, Shutdown, winTeam, gScore, nScore);
						onPostGameOver(this, Shutdown, winTeam, gScore, nScore);
					}
				}
				else for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnGame(this, buff.gotoWord(1, RenX::DelimS));
			}
			else if (header.equals("lCHAT:"))
			{
				if (action.equals("teamsay:"))
				{
					PARSE_PLAYER_DATA();
					Jupiter::ReferenceString message = buff.getWord(3, RenX::DelimS);
					onChat(this, player, message, false);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnTeamChat(this, player, message);
				}
				else if (action.equals("say:"))
				{
					PARSE_PLAYER_DATA();
					Jupiter::ReferenceString message = buff.getWord(3, RenX::DelimS);
					onChat(this, player, message, true);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnChat(this, player, message);
				}
				onAction();
			}
			else if (header.equals("lPLAYER:"))
			{
				PARSE_PLAYER_DATA();
				if (action.equals("disconnected"))
				{
					if (this->silenceParts == false)
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnPart(this, player);
					this->removePlayer(player);
					player = nullptr;
				}
				else if (action.equals("entered from"))
				{
					player->ip = buff.getWord(3, RenX::DelimS);
					if (buff.getWord(4, RenX::DelimS).equals("steamid"))
						player->steamid = buff.getWord(5, RenX::DelimS).asUnsignedLongLong();

					switch (RenX::Server::uuidMode)
					{
					default:
					case 0:
						if (player->steamid != 0)
							player->uuid = this->formatSteamID(player);
						break;
					case 1:
						player->uuid = player->name;
						break;
					}

					if (this->silenceJoins == false)
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnJoin(this, player);
				}
				else if (action.equals("changed name to:"))
				{
					Jupiter::ReferenceString newName = buff.getWord(3, RenX::DelimS);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnNameChange(this, player, newName);
					player->name = newName;
					if (RenX::Server::uuidMode == 1)
						player->uuid = player->name;
					onAction();
				}
			}
			else if (header.equals("lRCON:"))
			{
				if (action.equals("executed:"))
				{
					Jupiter::ReferenceString command = buff.getWord(3, RenX::DelimS);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnExecute(this, playerData, command);
				}
				else if (action.equals("subscribed")) for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnSubscribe(this, playerData);
				else for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnRCON(this, buff.gotoWord(1, RenX::DelimS));
			}
			else if (header.equals("lADMIN:"))
			{
				PARSE_PLAYER_DATA();
				if (action.equals("logged in as"))
				{
					player->adminType = buff.getWord(3, RenX::DelimS);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnAdminLogin(this, player);
				}
				else if (action.equals("logged out of"))
				{
					Jupiter::ReferenceString type = buff.getWord(3, RenX::DelimS);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnAdminLogout(this, player);
					player->adminType = "";
				}
				else for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnAdmin(this, buff.gotoWord(1, RenX::DelimS));
			}
			else if (header.equals("lC-LIST:"))
			{
				// ID IP SteamID Team Name
				if (playerData.isEmpty())
					break;

				int id;
				bool isBot = false;
				if (playerData.get(0) == 'b')
				{
					isBot = true;
					playerData.shiftRight(1);
					id = playerData.asInt(10);
					playerData.shiftLeft(1);
				}
				else id = playerData.asInt(10);
				Jupiter::ReferenceString ip = playerData.getWord(1, WHITESPACE);
				Jupiter::ReferenceString steamid = playerData.getWord(2, WHITESPACE);
				RenX::TeamType team;
				Jupiter::ReferenceString name;
				if (steamid.equals("-----NO")) // RCONv2-2a
				{
					steamid = "";
					Jupiter::ReferenceString &teamToken = playerData.getWord(4, WHITESPACE);
					if (teamToken.isEmpty())
						break;
					team = getTeam(teamToken.get(0));
					name = playerData.gotoWord(5, WHITESPACE);
				}
				else
				{
					if (steamid.equals("-----NO-STEAM-----")) // RCONv2-2.5a
						steamid = "";
					Jupiter::ReferenceString &teamToken = playerData.getWord(3, WHITESPACE);
					if (teamToken.isEmpty())
						break;
					team = getTeam(teamToken.get(0));
					name = playerData.gotoWord(4, WHITESPACE);
				}

				RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot);
				if (player->ip.size() == 0)
				{
					player->ip = ip;
					player->steamid = steamid.asUnsignedLongLong();

					switch (RenX::Server::uuidMode)
					{
					default:
					case 0:
						if (player->steamid != 0)
							player->uuid = this->formatSteamID(player);
						break;
					case 1:
						player->uuid = player->name;
						break;
					}
				}
			}
			else
			{
				buff.shiftRight(1);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnLog(this, buff);
				buff.shiftLeft(1);
			}
			break;

		case 'c':
			buff.shiftRight(1);
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnCommand(this, buff);
			buff.shiftLeft(1);
			break;

		case 'e':
			buff.shiftRight(1);
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnError(this, buff);
			buff.shiftLeft(1);
			break;

		case 'v':
			buff.shiftRight(1);
			this->rconVersion = buff.asInt(10);
			this->gameVersion = buff.substring(3);

			if (this->rconVersion == 1)
				this->profile = RenX::openBeta1Profile;
			else if (gameVersion.equals("Open Beta 2"))
				this->profile = RenX::openBeta2Profile;
			else if (gameVersion.equals("Open Beta 3"))
				this->profile = RenX::openBeta3Profile;

			if (this->profile->disconnectOnGameOver == false)
				this->firstGame = true;
			else if (this->firstGame == false)
				this->silenceJoins = true;

			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnVersion(this, buff);
			buff.shiftLeft(1);
			break;

		case 'a':
			buff.shiftRight(1);
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnAuthorized(this, buff);
			buff.shiftLeft(1);
			break;

		default:
			buff.shiftRight(1);
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnOther(this, header[0], buff);
			buff.shiftLeft(1);
			break;
		}
		for (size_t i = 0; i < xPlugins.size(); i++)
			xPlugins.get(i)->RenX_OnRaw(this, buff);
	}
}

void RenX::Server::disconnect()
{
	RenX::Server::sock.closeSocket();
	RenX::Server::wipeData();
	RenX::Server::connected = false;
}

bool RenX::Server::connect()
{
	RenX::Server::lastAttempt = time(0);
	if (RenX::Server::sock.connectToHost(RenX::Server::hostname.c_str(), RenX::Server::port, RenX::Server::clientHostname.isEmpty() ? nullptr : RenX::Server::clientHostname.c_str()))
	{
		RenX::Server::sock.setBlocking(false);
		RenX::Server::sock.send(Jupiter::StringS::Format("a%.*s\n", RenX::Server::pass.size(), RenX::Server::pass.ptr()));
		RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("s\n"));
		RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("clogclientlist\n"));
		RenX::Server::connected = true;
		RenX::Server::silenceParts = false;
		return true;
	}
	RenX::Server::connected = false;
	return false;
}

bool RenX::Server::reconnect()
{
	RenX::Server::sock.closeSocket();
	return RenX::Server::connect();
}

void RenX::Server::wipeData()
{
	while (RenX::Server::players.size() != 0)
		delete RenX::Server::players.remove(0U);
	RenX::Server::commands.emptyAndDelete();
}

unsigned int RenX::Server::getVersion() const
{
	return RenX::Server::rconVersion;
}

const Jupiter::ReadableString &RenX::Server::getGameVersion() const
{
	return RenX::Server::gameVersion;
}

RenX::Server::Server(const Jupiter::ReadableString &configurationSection)
{
	RenX::Server::configSection = configurationSection;

	RenX::Server::hostname = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Hostname"), STRING_LITERAL_AS_REFERENCE("localhost"));
	RenX::Server::port = (unsigned short)Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Port"), 7777);
	RenX::Server::clientHostname = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ClientAddress"));

	RenX::Server::pass = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Password"), STRING_LITERAL_AS_REFERENCE("renx"));

	RenX::Server::logChanType = Jupiter::IRC::Client::Config->getShort(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ChanType"));
	RenX::Server::adminLogChanType = Jupiter::IRC::Client::Config->getShort(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("AdminChanType"));

	RenX::Server::setCommandPrefix(Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("CommandPrefix")));
	RenX::Server::setPrefix(Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("IRCPrefix")));

	RenX::Server::rules = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Rules"), STRING_LITERAL_AS_REFERENCE("Anarchy!"));
	RenX::Server::delay = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ReconnectDelay"), 60);
	RenX::Server::uuidMode = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("UUIDMode"), 0);
	RenX::Server::steamFormat = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("SteamFormat"), 16);
	RenX::Server::neverSay = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("NeverSay"), false);

	for (size_t i = 0; i < RenX::GameMasterCommandList->size(); i++)
		RenX::Server::addCommand(RenX::GameMasterCommandList->get(i)->copy());
}

RenX::Server::~Server()
{
	sock.closeSocket();
	RenX::Server::wipeData();
}
