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
#include "RenX_BanDatabase.h"

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

int RenX::Server::OnRehash()
{
	Jupiter::StringS oldHostname = RenX::Server::hostname;
	unsigned short oldPort = RenX::Server::port;
	Jupiter::StringS oldClientHostname = RenX::Server::clientHostname;
	Jupiter::StringS oldPass = RenX::Server::pass;
	unsigned int oldUUIDMode = RenX::Server::uuidMode;
	int oldSteamFormat = RenX::Server::steamFormat;
	RenX::Server::commands.emptyAndDelete();
	RenX::Server::init();
	if (oldHostname.equalsi(RenX::Server::hostname) && oldPort == RenX::Server::port && oldClientHostname.equalsi(RenX::Server::clientHostname) && oldPass.equalsi(RenX::Server::pass))
	{
		if ((oldUUIDMode != RenX::Server::uuidMode || (RenX::Server::uuidMode == 0 && oldSteamFormat != RenX::Server::steamFormat)) && RenX::Server::players.size() != 0)
		{
			RenX::PlayerInfo *player;
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = RenX::Server::players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				switch (this->uuidMode)
				{
				default:
				case 0:
					if (player->steamid != 0)
						player->uuid = this->formatSteamID(player);
					else
						player->uuid = Jupiter::ReferenceString::empty;
					break;
				case 1:
					player->uuid = player->name;
					break;
				}
			}
		}
	}
	else
		RenX::Server::reconnect();
	return 0;
}

bool RenX::Server::OnBadRehash(bool removed)
{
	return removed;
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

int RenX::Server::sendMessage(const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
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

Jupiter::StringS RenX::Server::formatSteamID(const RenX::PlayerInfo *player) const
{
	return RenX::Server::formatSteamID(player->steamid);
}

Jupiter::StringS RenX::Server::formatSteamID(uint64_t id) const
{
	if (id == 0)
		return Jupiter::ReferenceString::empty;

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
	if (RenX::Server::rconBan)
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban pid%d\n", id));
	else
	{
		RenX::PlayerInfo *player = RenX::Server::getPlayer(id);
		if (player != nullptr)
			RenX::Server::banPlayer(player);
	}
}

void RenX::Server::banPlayer(const RenX::PlayerInfo *player, time_t length)
{
	if (RenX::Server::rconBan && length == 0)
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
			RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban pid%d\n", player->id));
	}
	else
		RenX::Server::kickPlayer(player);
	if (RenX::Server::localBan)
		RenX::banDatabase->add(this, player, length);
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
			if (player->access >= cmd->getAccessLevel())
				cmd->trigger(this, player, parameters);
			else
				RenX::Server::sendMessage(player, STRING_LITERAL_AS_REFERENCE("Access Denied."));
			r++;
		}
	}
	return r;
}

void RenX::Server::addCommand(RenX::GameCommand *command)
{
	RenX::Server::commands.add(command);
	if (RenX::Server::commandAccessLevels != nullptr)
	{
		const Jupiter::ReadableString &accessLevel = RenX::Server::commandAccessLevels->get(command->getTrigger());
		if (accessLevel.isEmpty() == false)
			command->setAccessLevel(accessLevel.asInt());
	}
	if (RenX::Server::commandAliases != nullptr)
	{
		const Jupiter::ReadableString &aliasList = RenX::Server::commandAliases->get(command->getTrigger());
		unsigned int j = aliasList.wordCount(WHITESPACE);
		while (j != 0)
			command->addTrigger(Jupiter::ReferenceString::getWord(aliasList, --j, WHITESPACE));
	}
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
	RenX::Server::sendPubChan(msg);
}

void RenX::Server::sendPubChan(const Jupiter::ReadableString &msg) const
{
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
	RenX::Server::sendAdmChan(msg);
}

void RenX::Server::sendAdmChan(const Jupiter::ReadableString &msg) const
{
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
	RenX::Server::sendLogChan(msg);
}

void RenX::Server::sendLogChan(const Jupiter::ReadableString &msg) const
{
	for (size_t i = 0; i != serverManager->size(); i++)
	{
		IRC_Bot *server = serverManager->getServer(i);
		server->messageChannels(RenX::Server::logChanType, msg);
		server->messageChannels(RenX::Server::adminLogChanType, msg);
	}
}

#define PARSE_PLAYER_DATA_P(DATA) \
	Jupiter::ReferenceString name; \
	TeamType team; \
	int id; \
	bool isBot; \
	parsePlayerData(DATA, name, team, id, isBot);

#define PARSE_PLAYER_DATA() PARSE_PLAYER_DATA_P(playerData)

void RenX::Server::processLine(const Jupiter::ReadableString &line)
{
	Jupiter::ReferenceString buff = line;
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	Jupiter::ReferenceString header = buff.getToken(0, RenX::DelimC);
	Jupiter::ReferenceString playerData = buff.getToken(1, RenX::DelimC);
	Jupiter::ReferenceString action = buff.getToken(2, RenX::DelimC);

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
	auto parsePlayerData = [&](const Jupiter::ReadableString &data, Jupiter::ReferenceString &name, TeamType &team, int &id, bool &isBot)
	{
		Jupiter::ReferenceString idToken = Jupiter::ReferenceString::getToken(data, 1, ',');
		name = Jupiter::ReferenceString::gotoToken(data, 2, ',');
		if (data[0] == ',')
			team = Other;
		else
			team = RenX::getTeam(data[0]);
		if (idToken.get(0) == 'b')
		{
			idToken.shiftRight(1);
			isBot = true;
		}
		else
			isBot = false;
		id = idToken.asInt(10);
	};
	auto getPlayerOrAdd = [&](RenX::Server *server, const Jupiter::ReadableString &name, int id, RenX::TeamType team, bool isBot, uint64_t steamid, const Jupiter::ReadableString &ip)
	{
		bool checkBans = false;
		RenX::PlayerInfo *r = server->getPlayer(id);
		auto checkMissing = [&]()
		{
			if (r->ip32 == 0 && ip.isEmpty() == false)
			{
				r->ip = ip;
				r->ip32 = Jupiter::Socket::pton4(Jupiter::CStringS(r->ip).c_str());
				checkBans = true;
			}
			if (r->steamid == 0U && steamid != 0U)
			{
				r->steamid = steamid;
				if (this->uuidMode == 0)
					r->uuid = this->formatSteamID(r);
				checkBans = true;
			}
		};
		if (r == nullptr)
		{
			checkBans = true;
			r = new RenX::PlayerInfo();
			r->id = id;
			r->name = name;
			checkMissing();
			if (r->isBot = isBot)
				r->formatNamePrefix = IRCCOLOR "05[B]";
			r->joinTime = time(nullptr);
			if (id != 0)
				server->players.add(r);
			if (this->uuidMode == 1)
				r->uuid = r->name;

			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnPlayerCreate(server, r);
		}
		else
		{
			checkMissing();
			if (r->name.size() == 0)
				r->name = name;
		}
		r->team = team;
		if (checkBans)
		{
			const Jupiter::ArrayList<RenX::BanDatabase::Entry> &entries = RenX::banDatabase->getEntries();
			RenX::BanDatabase::Entry *entry;
			for (size_t i = 0; i != entries.size(); i++)
			{
				entry = entries.get(i);
				if (entry->active)
				{
					if (entry->length != 0 && entry->timestamp + entry->length < time(0))
						banDatabase->deactivate(i);
					else if (server->localSteamBan && entry->steamid != 0 && entry->steamid == r->steamid)
						server->kickPlayer(r);
					else if (server->localIPBan && entry->ip != 0 && entry->ip == r->ip32)
						server->kickPlayer(r);
					else if (server->localNameBan && entry->name.isEmpty() == false && entry->name.equalsi(r->name))
						server->kickPlayer(r);
				}
			}
		}
		return r;
	};

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
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					Jupiter::ReferenceString objectType = buff.getToken(3, RenX::DelimC);
					if (objectType.match("*Beacon")) player->beaconPlacements++;
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnDeploy(this, player, objectType);
					onAction();
				}
				else if (action.equals("suicided by"))
				{
					PARSE_PLAYER_DATA();
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					player->deaths++;
					player->suicides++;
					Jupiter::ReferenceString damageType = buff.getToken(3, RenX::DelimC);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnSuicide(this, player, damageType);
					this->firstDeath = true;
					onAction();
				}
				else if (action.equals("killed"))
				{
					PARSE_PLAYER_DATA();
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					Jupiter::ReferenceString victimData = buff.getToken(3, RenX::DelimC);
					Jupiter::ReferenceString vTeamToken = victimData.getToken(0, ',');
					Jupiter::ReferenceString vidToken = victimData.getToken(1, ',');
					if (vTeamToken.size() != 0 && vidToken.size() != 0)
					{
						Jupiter::ReferenceString vname = victimData.getToken(2, ',');
						int vid;
						bool visBot = false;
						if (vidToken[0] == 'b')
						{
							vidToken.shiftRight(1);
							visBot = true;
						}
						vid = vidToken.asInt(10);
						TeamType vteam = RenX::getTeam(vTeamToken.get(0));
						Jupiter::ReferenceString damageType = buff.getToken(5, RenX::DelimC);
						RenX::PlayerInfo *victim = getPlayerOrAdd(this, vname, vid, vteam, visBot, 0, Jupiter::ReferenceString::empty);
						player->kills++;
						if (damageType.equals("Rx_DmgType_Headshot")) player->headshots++;
						victim->deaths++;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnKill(this, player, victim, damageType);
					}

					if (this->needsCList)
					{
						this->sendData(STRING_LITERAL_AS_REFERENCE("clogclientlist\n"));
						this->needsCList = false;
					}

					this->firstKill = true;
					this->firstDeath = true;
					onAction();
				}
				else if (action.match("died by"))
				{
					PARSE_PLAYER_DATA();
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					player->deaths++;
					Jupiter::ReferenceString damageType = buff.getToken(3, RenX::DelimC);
					if (damageType.equals("DamageType"))
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnTeamChange(this, player);
					else
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDie(this, player, damageType);
					this->firstDeath = true;
					onAction();
				}
				else if (action.match("destroyed*"))
				{
					PARSE_PLAYER_DATA();
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					Jupiter::ReferenceString victim = buff.getToken(3, RenX::DelimC);
					Jupiter::ReferenceString damageType = buff.getToken(5, RenX::DelimC);
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
					int gScore = buff.getToken(2, RenX::DelimC).gotoToken(1, '=').asInt(10);
					int nScore = buff.getToken(3, RenX::DelimC).gotoToken(1, '=').asInt(10);
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
					int gScore;
					switch (RenX::Server::profile->tieFormat)
					{
					default:
					case 1:
						gScore = buff.getToken(2, RenX::DelimC).gotoToken(1, '=').asInt(10);
						break;
					case 0:
						gScore = action.gotoToken(1, '=').asInt(10);
						break;
					}
					int nScore = buff.getToken(3, RenX::DelimC).gotoToken(1, '=').asInt(10);

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
					xPlugins.get(i)->RenX_OnGame(this, buff.gotoToken(1, RenX::DelimC));
			}
			else if (header.equals("lCHAT:"))
			{
				if (action.equals("teamsay:"))
				{
					PARSE_PLAYER_DATA();
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					Jupiter::ReferenceString message = buff.getToken(3, RenX::DelimC);
					onChat(this, player, message, false);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnTeamChat(this, player, message);
				}
				else if (action.equals("say:"))
				{
					PARSE_PLAYER_DATA();
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					Jupiter::ReferenceString message = buff.getToken(3, RenX::DelimC);
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
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					if (this->silenceParts == false)
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnPart(this, player);
					this->removePlayer(player);
					player = nullptr;
				}
				else if (action.equals("entered from"))
				{
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, buff.getToken(4, RenX::DelimC).equals("steamid") ? buff.getToken(5, RenX::DelimC).asUnsignedLongLong() : 0, buff.getToken(3, RenX::DelimC));

					if (this->silenceJoins == false)
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnJoin(this, player);
				}
				else if (action.equals("changed name to:"))
				{
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					Jupiter::ReferenceString newName = buff.getToken(3, RenX::DelimC);
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
					Jupiter::ReferenceString command = buff.getToken(3, RenX::DelimC);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnExecute(this, playerData, command);
				}
				else if (action.equals("subscribed"))
				{
					if (this->rconUser.isEmpty())
						this->rconUser = playerData;
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnSubscribe(this, playerData);
				}
				else for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnRCON(this, buff.gotoToken(1, RenX::DelimC));
			}
			else if (header.equals("lADMIN:"))
			{
				PARSE_PLAYER_DATA();
				RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
				if (action.equals("logged in as"))
				{
					player->adminType = buff.getToken(3, RenX::DelimC);
					if (player->adminType.equalsi("moderator") && player->access < 1)
						player->access = 1;
					else if (player->adminType.equalsi("administrator") && player->access < 2)
						player->access = 2;
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnAdminLogin(this, player);
				}
				else if (action.equals("logged out of"))
				{
					Jupiter::ReferenceString type = buff.getToken(3, RenX::DelimC);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnAdminLogout(this, player);
					player->adminType = "";
					player->access = 0;
				}
				else if (action.equals("granted"))
				{
					player->adminType = buff.getToken(3, RenX::DelimC);
					if (player->adminType.equalsi("moderator") && player->access < 1)
						player->access = 1;
					else if (player->adminType.equalsi("administrator") && player->access < 2)
						player->access = 2;
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnAdminGrant(this, player);
				}
				else for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnAdmin(this, buff.gotoToken(1, RenX::DelimC));
			}
			else if (header.equals("lC-LIST:"))
			{
				// ID IP SteamID Team Name
				if (playerData.isEmpty())
					break;

				static const Jupiter::ReferenceString CListDelim = STRING_LITERAL_AS_REFERENCE("  ");
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
				Jupiter::ReferenceString ip = playerData.getToken(1, CListDelim);
				Jupiter::ReferenceString steamid = playerData.getToken(2, CListDelim);
				RenX::TeamType team;
				Jupiter::ReferenceString name;
				if (steamid.equals("-----NO")) // RCONv2-2a
				{
					steamid = "";
					Jupiter::ReferenceString &teamToken = playerData.getToken(4, CListDelim);
					if (teamToken.isEmpty())
						break;
					team = getTeam(teamToken.get(0));
					name = playerData.gotoToken(5, CListDelim);
				}
				else
				{
					if (steamid.equals("-----NO-STEAM-----")) // RCONv2-2.5a
						steamid = "";
					Jupiter::ReferenceString &teamToken = playerData.getToken(3, CListDelim);
					if (teamToken.isEmpty())
						break;
					team = getTeam(teamToken.get(0));
					name = playerData.gotoToken(4, CListDelim);
				}

				RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, steamid.asUnsignedLongLong(), ip);
			}
			else
			{
				buff.shiftRight(1);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnLog(this, buff);
				buff.shiftLeft(1);
			}
			break;

		case 'x':
			header.shiftRight(1);
			if (header.size() == 0)
			{
				header.shiftLeft(1);
				break;
			}
			if (header[0] == 'r') // Command response
			{
				if (header.size() == 1)
				{
					header.shiftLeft(1);
					break;
				}
				header.shiftRight(1);
				switch (header[0])
				{
				case 1: // Client list: Normal Player Data | IP | Steam ID | Start Time | Ping Kills | Deaths | Score | Credits | Class
					header.shiftRight(1);
					{
						PARSE_PLAYER_DATA_P(header);
						PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, playerData.asUnsignedLongLong(0), action);
						Jupiter::ReferenceString pingKillsToken = buff.getToken(4, RenX::DelimC);
						if (pingKillsToken.isEmpty() == false)
						{
							player->ping = static_cast<unsigned char>(pingKillsToken.get(0)) * 4;
							pingKillsToken.shiftRight(1);
							player->kills = pingKillsToken.asUnsignedInt();
						}
						player->deaths = buff.getToken(5, RenX::DelimC).asUnsignedInt();
						player->score = static_cast<float>(buff.getToken(6, RenX::DelimC).asDouble());
						player->credits = static_cast<float>(buff.getToken(7, RenX::DelimC).asDouble());
						player->character = RenX::getCharacter(buff.getToken(8, RenX::DelimC));
					}
					header.shiftLeft(1);
					break;
				case 2: // Ping, Score, Credits list: Normal Player Data | Ping Score | Credits
					header.shiftRight(1);
					{
						PARSE_PLAYER_DATA_P(header);
						PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0U, Jupiter::ReferenceString::empty);
						if (playerData.isEmpty() == false)
						{
							player->ping = static_cast<unsigned char>(playerData.get(0)) * 4;
							playerData.shiftRight(1);
							player->score = static_cast<float>(playerData.asDouble());
							playerData.shiftLeft(1);
						}
						player->credits = static_cast<float>(action.asDouble());
					}
					header.shiftLeft(1);
					break;
				case 3: // Echo: Data
					break;
				case 4: // Add Credits: Normal Player Data | Credits
					header.shiftRight(1);
					{
						PARSE_PLAYER_DATA_P(header);
						PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0U, Jupiter::ReferenceString::empty);
						player->credits = static_cast<float>(playerData.asDouble());
					}
					header.shiftLeft(1);
					break;
				case 5: // Ping: {Average Ping}/{Normal Player Data | Ping}
					break;
				case 6: // Command 2 on Timer: Time interval
					break;
				default:
					break;
				}
				header.shiftLeft(1);
			}
			else if (header.equals("version"))
			{
				RenX::Server::xRconVersion = playerData.asUnsignedInt(10);
				if (this->rconUser.equals(action) == false)
					this->rconUser = action;
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_XOnVersion(this, RenX::Server::xRconVersion);
				RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("_x\x01\n"));
				RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("_x\x06\n"));
			}
			else if (header.equals("grant_character"))
			{
				PARSE_PLAYER_DATA();
				RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnGrantCharacter(this, player, action);
				player->character = RenX::getCharacter(action);
			}
			else if (header.equals("grant_weapon"))
			{
				PARSE_PLAYER_DATA();
				RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnGrantWeapon(this, player, action);
			}
			else if (header.equals("spawn_vehicle"))
			{
				if (playerData.equalsi("buy"))
				{
					PARSE_PLAYER_DATA_P(buff.getToken(3, RenX::DelimC));
					RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnSpawnVehicle(this, player, action);
				}
				else
				{
					RenX::TeamType team;
					if (playerData.isEmpty())
						team = Other;
					else
						team = RenX::getTeam(playerData.get(0));
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnSpawnVehicleNoOwner(this, team, action);
				}
			}
			else if (header.equals("mine_place"))
			{
				PARSE_PLAYER_DATA();
				RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnMinePlace(this, player, action);
			}
			/*else if (header.equals("mlimit_inc"))
			{
			}*/
			else if (header.equals("kill"))
			{
				Jupiter::ReferenceString vData = buff.getToken(3, RenX::DelimC);
				if (action.isEmpty() == false && vData.isEmpty() == false) // Safety check
				{
					struct
					{
						uint8_t type; // 1 = Player, 2 = Non-Player, 3 = None
						Jupiter::ReferenceString data;
					} killerData, victimData;
					Jupiter::ReadableString &damageType = playerData;
					killerData.type = action[0];
					killerData.data = action.substring(1);
					victimData.type = vData[0];
					victimData.data = vData.substring(1);
					if (killerData.type == 1) // These are already handled in standard RCON logs; update models and move on.
					{
						{
							PARSE_PLAYER_DATA_P(killerData.data.gotoToken(1, ','));
							RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
							player->character = RenX::getCharacter(killerData.data.getToken(0, ','));
						}
						if (victimData.type == 1)
						{
							PARSE_PLAYER_DATA_P(victimData.data.gotoToken(1, ','));
							RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
							player->character = RenX::getCharacter(victimData.data.getToken(0, ','));
						}
					}
					else if (killerData.type == 3) // No killer!
					{
						if (victimData.type == 2 && victimData.data.size() != 0)
						{
							TeamType victimTeam = RenX::getTeam(victimData.data.getToken(0, ',').get(0));
							victimData.data = victimData.data.gotoToken(1, ',');
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDie(this, victimData.data, victimTeam, damageType);
						}
					}
					else if (killerData.data.size() != 0) // Non-player killer (log!)
					{
						TeamType killerTeam = RenX::getTeam(killerData.data.getToken(0, ',').get(0));
						killerData.data = killerData.data.gotoToken(1, ',');
						if (victimData.type == 1) // Non-player killed player
						{
							PARSE_PLAYER_DATA_P(victimData.data.gotoToken(1, ','));
							RenX::PlayerInfo *player = getPlayerOrAdd(this, name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
							player->character = RenX::getCharacter(victimData.data.getToken(0, ','));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnKill(this, killerData.data, killerTeam, player, damageType);
						}
						else if (victimData.data.size() != 0) // Non-player destroyed non-player
						{
							TeamType victimTeam = RenX::getTeam(victimData.data.getToken(0, ',').get(0));
							victimData.data = victimData.data.gotoToken(1, ',');
							ObjectType type;
							if (victimData.data.match("Rx_Building_*"))
								type = Building;
							else if (victimData.data.match("Rx_Defence_*"))
								type = Defence;
							else
								type = Vehicle;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDestroy(this, killerData.data, killerTeam, victimData.data, victimTeam, damageType, type);
						}
					}
				}
			}
			else
			{
				buff.shiftRight(1);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_XOnOther(this, buff);
				buff.shiftLeft(1);
			}
			header.shiftLeft(1);
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
			{
				this->firstAction = false;
				this->silenceJoins = true;
			}

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
	RenX::Server::disconnect();
	return RenX::Server::connect();
}

void RenX::Server::wipeData()
{
	RenX::Server::rconUser.truncate(RenX::Server::rconUser.size());
	while (RenX::Server::players.size() != 0)
		delete RenX::Server::players.remove(0U);
}

unsigned int RenX::Server::getVersion() const
{
	return RenX::Server::rconVersion;
}

unsigned int RenX::Server::getXVersion() const
{
	return RenX::Server::xRconVersion;
}

const Jupiter::ReadableString &RenX::Server::getGameVersion() const
{
	return RenX::Server::gameVersion;
}

const Jupiter::ReadableString &RenX::Server::getRCONUsername() const
{
	return RenX::Server::rconUser;
}

RenX::Server::Server(const Jupiter::ReadableString &configurationSection)
{
	RenX::Server::configSection = configurationSection;
	init();
}

void RenX::Server::init()
{
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
	RenX::Server::rconBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("RCONBan"), false);
	RenX::Server::localSteamBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("LocalSteamBan"), true);
	RenX::Server::localIPBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("LocalIPBan"), true);
	RenX::Server::localNameBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("LocalNameBan"), false);
	RenX::Server::localBan = RenX::Server::localIPBan || RenX::Server::localSteamBan || RenX::Server::localNameBan;
	RenX::Server::steamFormat = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("SteamFormat"), 16);
	RenX::Server::neverSay = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("NeverSay"), false);

	Jupiter::INIFile &commandsFile = RenX::getCore()->getCommandsFile();
	RenX::Server::commandAccessLevels = commandsFile.getSection(RenX::Server::configSection);
	RenX::Server::commandAliases = commandsFile.getSection(Jupiter::StringS::Format("%.*s.Aliases", RenX::Server::configSection.size(), RenX::Server::configSection.ptr()));

	for (size_t i = 0; i < RenX::GameMasterCommandList->size(); i++)
		RenX::Server::addCommand(RenX::GameMasterCommandList->get(i)->copy());
}

RenX::Server::~Server()
{
	sock.closeSocket();
	RenX::Server::wipeData();
	RenX::Server::commands.emptyAndDelete();
}
