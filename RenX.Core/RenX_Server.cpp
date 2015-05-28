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
#include "RenX_Tags.h"

int RenX::Server::think()
{
	if (RenX::Server::connected == false)
	{
		if (RenX::Server::maxAttempts < 0 || RenX::Server::attempts < RenX::Server::maxAttempts)
		{
			if (time(0) >= RenX::Server::lastAttempt + RenX::Server::delay)
			{
				if (RenX::Server::connect())
					RenX::Server::sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to Renegade-X server.");
				else RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to Renegade-X server.");
			}
		}
		else
			return 1;
	}
	else
	{
		if (RenX::Server::sock.recv() > 0)
		{
			Jupiter::ReferenceString buffer = RenX::Server::sock.getBuffer();
			unsigned int totalLines = buffer.tokenCount('\n');

			if (totalLines != 0)
			{
				RenX::Server::lastLine.concat(buffer.getToken(0, '\n'));
				if (totalLines != 1) // if there's only one token, there is no newline.
				{
					RenX::Server::processLine(RenX::Server::lastLine);
					RenX::Server::lastLine = buffer.getToken(totalLines - 1, '\n');

					for (unsigned int currentLine = 1; currentLine != totalLines - 1; currentLine++)
						RenX::Server::processLine(buffer.getToken(currentLine, '\n'));
				}
			}
		}
		else if (Jupiter::Socket::getLastError() != 10035) // This is a serious error
		{
			RenX::Server::wipeData();
			if (RenX::Server::maxAttempts != 0)
			{
				RenX::Server::sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt in progress.");
				if (RenX::Server::reconnect())
					RenX::Server::sendLogChan(IRCCOLOR "06[Progress]" IRCCOLOR " Connection to Renegade-X server reestablished. Initializing Renegade-X RCON protocol...");
				else
					RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt failed.");
			}
			else
			{
				RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. No attempt will be made to reconnect.");
				return 1;
			}
			return 0;
		}
		if (RenX::Server::rconVersion >= 3 && std::chrono::steady_clock::now() > RenX::Server::lastClientListUpdate + RenX::Server::clientUpdateRate)
			RenX::Server::updateClientList();
	}
	return 0;
}

int RenX::Server::OnRehash()
{
	Jupiter::StringS oldHostname = RenX::Server::hostname;
	unsigned short oldPort = RenX::Server::port;
	Jupiter::StringS oldClientHostname = RenX::Server::clientHostname;
	Jupiter::StringS oldPass = RenX::Server::pass;
	int oldSteamFormat = RenX::Server::steamFormat;
	RenX::Server::commands.emptyAndDelete();
	RenX::Server::init();
	if (oldHostname.equalsi(RenX::Server::hostname) == false || oldPort != RenX::Server::port || oldClientHostname.equalsi(RenX::Server::clientHostname) == false || oldPass.equalsi(RenX::Server::pass) == false)
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

bool RenX::Server::isFirstAction() const
{
	return RenX::Server::firstAction;
}

bool RenX::Server::isSeamless() const
{
	return RenX::Server::seamless;
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
	Jupiter::String cmd(command.size() + 2);
	cmd = 'c';
	cmd += command;
	cmd += '\n';
	return RenX::Server::sock.send(cmd);
}

int RenX::Server::sendMessage(const Jupiter::ReadableString &message)
{
	if (RenX::Server::neverSay)
	{
		int r = 0;
		if (RenX::Server::players.size() != 0)
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
				if (node->data->isBot == false)
					r += RenX::Server::sock.send(Jupiter::StringS::Format("chostprivatesay pid%d %.*s\n", node->data->id, message.size(), message.ptr()));
		return r;
	}
	else
	{
		Jupiter::StringS cmd = STRING_LITERAL_AS_REFERENCE("chostsay ");
		cmd += message;
		cmd += '\n';
		return RenX::Server::sock.send(cmd);
	}
}

int RenX::Server::sendMessage(const RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	Jupiter::String cmd(message.size() + 28);
	cmd = STRING_LITERAL_AS_REFERENCE("chostprivatesay pid");
	cmd += Jupiter::StringS::Format("%d ", player->id);
	cmd += message;
	cmd += '\n';
	RenX::sanitizeString(cmd);
	return RenX::Server::sock.send(cmd);
	//return RenX::Server::sock.send(Jupiter::StringS::Format("chostprivatesay pid%d %.*s\n", player->id, message.size(), message.ptr()));
}

int RenX::Server::sendData(const Jupiter::ReadableString &data)
{
	return RenX::Server::sock.send(data);
}

RenX::PlayerInfo *RenX::Server::getPlayer(int id) const
{
	if (RenX::Server::players.size() == 0) return nullptr;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
		if (node->data->id == id)
			return node->data;
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
	RenX::Server::sock.send(Jupiter::StringS::Format("ckick pid%d\n", id));
}

void RenX::Server::kickPlayer(const RenX::PlayerInfo *player)
{
	RenX::Server::kickPlayer(player->id);
}

void RenX::Server::banPlayer(int id)
{
	if (RenX::Server::rconBan)
		RenX::Server::sock.send(Jupiter::StringS::Format("ckickban pid%d\n", id));
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
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban pid%d\n", player->id));
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

bool RenX::Server::fetchClientList()
{
	RenX::Server::lastClientListUpdate = std::chrono::steady_clock::now();
	return RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("cclientvarlist KILLS\xA0""DEATHS\xA0""SCORE\xA0""CREDITS\xA0""CHARACTER\xA0""VEHICLE\xA0""PING\xA0""ADMIN\xA0""STEAM\xA0""IP\xA0""PLAYERLOG\n")) > 0
		&& RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("cbotvarlist KILLS\xA0""DEATHS\xA0""SCORE\xA0""CREDITS\xA0""CHARACTER\xA0""VEHICLE\xA0""PLAYERLOG\n")) > 0;
}

bool RenX::Server::updateClientList()
{
	RenX::Server::lastClientListUpdate = std::chrono::steady_clock::now();
	if (RenX::Server::players.size() == 0)
		return true;

	size_t botCount = 0;
	for (size_t i = 0; i != RenX::Server::players.size(); i++)
		if (RenX::Server::players.get(i)->isBot)
			botCount++;

	bool r;
	if (RenX::Server::players.size() != botCount)
		r = RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("cclientvarlist ID\xA0""SCORE\xA0""CREDITS\xA0""PING\n")) > 0;

	if (botCount != 0)
		r |= RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("cbotvarlist ID\xA0""SCORE\xA0""CREDITS\n")) > 0;

	return r;
}

bool RenX::Server::gameover()
{
	return RenX::Server::send(STRING_LITERAL_AS_REFERENCE("endmap")) > 0;
}

bool RenX::Server::setMap(const Jupiter::ReadableString &map)
{
	return RenX::Server::send(Jupiter::StringS::Format("changemap %.*s", map.size(), map.ptr())) > 0;
}

bool RenX::Server::loadMutator(const Jupiter::ReadableString &mutator)
{
	return RenX::Server::send(Jupiter::StringS::Format("loadmutator %.*s", mutator.size(), mutator.ptr())) > 0;
}

bool RenX::Server::unloadMutator(const Jupiter::ReadableString &mutator)
{
	return RenX::Server::send(Jupiter::StringS::Format("unloadmutator %.*s", mutator.size(), mutator.ptr())) > 0;
}

bool RenX::Server::cancelVote(const RenX::TeamType team)
{
	switch (team)
	{
	default:
		return RenX::Server::send(STRING_LITERAL_AS_REFERENCE("cancelvote -1")) > 0;
	case TeamType::GDI:
		return RenX::Server::send(STRING_LITERAL_AS_REFERENCE("cancelvote 0")) > 0;
	case TeamType::Nod:
		return RenX::Server::send(STRING_LITERAL_AS_REFERENCE("cancelvote 1")) > 0;
	}
}

bool RenX::Server::swapTeams()
{
	return RenX::Server::send(STRING_LITERAL_AS_REFERENCE("swapteams")) > 0;
}

bool RenX::Server::recordDemo()
{
	return RenX::Server::send(STRING_LITERAL_AS_REFERENCE("recorddemo")) > 0;
}

bool RenX::Server::mute(const RenX::PlayerInfo *player)
{
	return RenX::Server::send(Jupiter::StringS::Format("textmute pid%u", player->id)) > 0;
}

bool RenX::Server::unmute(const RenX::PlayerInfo *player)
{
	return RenX::Server::send(Jupiter::StringS::Format("textunmute pid%u", player->id)) > 0;
}

bool RenX::Server::giveCredits(int id, double credits)
{
	return RenX::Server::send(Jupiter::StringS::Format("givecredits pid%d %f", id, credits)) > 0;
}

bool RenX::Server::giveCredits(RenX::PlayerInfo *player, double credits)
{
	return RenX::Server::giveCredits(player->id, credits);
}

bool RenX::Server::kill(int id)
{
	return RenX::Server::send(Jupiter::StringS::Format("kill pid%d", id)) > 0;
}

bool RenX::Server::kill(RenX::PlayerInfo *player)
{
	return RenX::Server::kill(player->id);
}

bool RenX::Server::disarm(int id)
{
	return RenX::Server::send(Jupiter::StringS::Format("disarm pid%d", id)) > 0;
}

bool RenX::Server::disarm(RenX::PlayerInfo *player)
{
	return RenX::Server::disarm(player->id);
}

bool RenX::Server::disarmC4(int id)
{
	return RenX::Server::send(Jupiter::StringS::Format("disarmc4 pid%d", id)) > 0;
}

bool RenX::Server::disarmC4(RenX::PlayerInfo *player)
{
	return RenX::Server::disarmC4(player->id);
}

bool RenX::Server::disarmBeacon(int id)
{
	return RenX::Server::send(Jupiter::StringS::Format("disarmb pid%d", id)) > 0;
}

bool RenX::Server::disarmBeacon(RenX::PlayerInfo *player)
{
	return RenX::Server::disarmBeacon(player->id);
}

bool RenX::Server::changeTeam(int id, bool resetCredits)
{
	return RenX::Server::send(Jupiter::StringS::Format(resetCredits ? "team pid%d" : "team2 pid%d", id)) > 0;
}

bool RenX::Server::changeTeam(RenX::PlayerInfo *player, bool resetCredits)
{
	return RenX::Server::changeTeam(player->id, resetCredits);
}

const Jupiter::ReadableString &RenX::Server::getPrefix() const
{
	static Jupiter::StringS parsed;
	RenX::processTags(parsed = RenX::Server::IRCPrefix, this);
	return parsed;
}

void RenX::Server::setPrefix(const Jupiter::ReadableString &prefix)
{
	RenX::sanitizeTags(RenX::Server::IRCPrefix = prefix);
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

const Jupiter::ReadableString &RenX::Server::getSocketHostname() const
{
	return RenX::Server::sock.getHostname();
}

unsigned short RenX::Server::getSocketPort() const
{
	return RenX::Server::sock.getPort();
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

const Jupiter::ReadableString &RenX::Server::getUser() const
{
	return RenX::Server::rconUser;
}

const Jupiter::ReadableString &RenX::Server::getName() const
{
	return RenX::Server::serverName;
}

const Jupiter::ReadableString &RenX::Server::getMap() const
{
	return RenX::Server::map;
}

RenX::GameCommand *RenX::Server::getCommand(unsigned int index) const
{
	return RenX::Server::commands.get(index);
}

RenX::GameCommand *RenX::Server::getCommand(const Jupiter::ReadableString &trigger) const
{
	RenX::GameCommand *cmd;
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
	{
		cmd = RenX::Server::commands.get(i);
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
		if (RenX::Server::commands.get(i) == command)
		{
			delete RenX::Server::commands.remove(i);
			return true;
		}
	return false;
}

bool RenX::Server::removeCommand(const Jupiter::ReadableString &trigger)
{
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
		if (RenX::Server::commands.get(i)->matches(trigger))
		{
			delete RenX::Server::commands.remove(i);
			return true;
		}
	return false;
}

void RenX::Server::setUUIDFunction(RenX::Server::uuid_func func)
{
	RenX::Server::calc_uuid = func;
	if (RenX::Server::players.size() != 0)
	{
		Jupiter::DLList<PlayerInfo>::Node *node = RenX::Server::players.getNode(0);
		do
		{
			RenX::Server::setUUIDIfDifferent(node->data, RenX::Server::calc_uuid(node->data));
			node = node->next;
		}
		while (node != nullptr);
	}
}

RenX::Server::uuid_func RenX::Server::getUUIDFunction() const
{
	return RenX::Server::calc_uuid;
}

void RenX::Server::setUUID(RenX::PlayerInfo *player, const Jupiter::ReadableString &uuid)
{
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *getCore()->getPlugins();
	for (size_t index = 0; index < xPlugins.size(); index++)
		xPlugins.get(index)->RenX_OnPlayerUUIDChange(this, player, uuid);
	player->uuid = uuid;
}

bool RenX::Server::setUUIDIfDifferent(RenX::PlayerInfo *player, const Jupiter::ReadableString &uuid)
{
	if (player->uuid.equals(uuid))
		return false;
	setUUID(player, uuid);
	return true;
}

void RenX::Server::sendPubChan(const char *fmt, ...) const
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
		serverManager->getServer(i)->messageChannels(RenX::Server::logChanType, msg);
}

void RenX::Server::sendPubChan(const Jupiter::ReadableString &msg) const
{
	const Jupiter::ReadableString &prefix = this->getPrefix();
	if (prefix.isEmpty() == false)
	{
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::logChanType, m);
	}
	else
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

void RenX::Server::sendAdmChan(const Jupiter::ReadableString &msg) const
{
	const Jupiter::ReadableString &prefix = this->getPrefix();
	if (prefix.isEmpty() == false)
	{
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::adminLogChanType, m);
	}
	else
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::adminLogChanType, msg);
}

void RenX::Server::sendLogChan(const char *fmt, ...) const
{
	IRC_Bot *server;
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
		server = serverManager->getServer(i);
		server->messageChannels(RenX::Server::logChanType, msg);
		server->messageChannels(RenX::Server::adminLogChanType, msg);
	}
}

void RenX::Server::sendLogChan(const Jupiter::ReadableString &msg) const
{
	IRC_Bot *server;
	const Jupiter::ReadableString &prefix = this->getPrefix();
	if (prefix.isEmpty() == false)
	{
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
		{
			server = serverManager->getServer(i);
			server->messageChannels(RenX::Server::logChanType, m);
			server->messageChannels(RenX::Server::adminLogChanType, m);
		}
	}
	else
		for (size_t i = 0; i != serverManager->size(); i++)
		{
			server = serverManager->getServer(i);
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

void RenX::Server::processLine(const Jupiter::ReadableString &line)
{
	Jupiter::ReferenceString buff = line;
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	Jupiter::ReferenceString header = buff.getToken(0, RenX::DelimC);

	/** Local functions */
	auto onPreGameOver = [this](RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
	{
		RenX::PlayerInfo *player;

		if (this->players.size() != 0)
		{
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = this->players.getNode(0); n != nullptr; n = n->next)
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
	auto onPostGameOver = [this](RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
	{
		this->firstGame = false;
		this->firstAction = false;
		this->firstKill = false;
		this->firstDeath = false;
		RenX::PlayerInfo *player;

		if (this->players.size() != 0)
		{
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = this->players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				if (player != nullptr)
				{
					player->score = 0.0f;
					player->credits = 0.0f;
					player->kills = 0;
					player->deaths = 0;
					player->suicides = 0;
					player->headshots = 0;
					player->vehicleKills = 0;
					player->buildingKills = 0;
					player->defenceKills = 0;
					player->beaconPlacements = 0;
					player->beaconDisarms = 0;
					player->captures = 0;
					player->steals = 0;
					player->stolen = 0;
				}
			}
		}
	};
	auto onChat = [this](RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
	{
		const Jupiter::ReadableString &prefix = this->getCommandPrefix();
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
			this->triggerCommand(command, player, parameters);
		}
	};
	auto onAction = [this]()
	{
		if (this->firstAction == false)
		{
			this->firstAction = true;
			this->silenceJoins = false;
		}
	};
	auto parsePlayerData = [this](const Jupiter::ReadableString &data, Jupiter::ReferenceString &name, TeamType &team, int &id, bool &isBot)
	{
		Jupiter::ReferenceString idToken = Jupiter::ReferenceString::getToken(data, 1, ',');
		name = Jupiter::ReferenceString::gotoToken(data, 2, ',');
		team = RenX::getTeam(Jupiter::ReferenceString::getToken(data, 0, ','));
		if (idToken.get(0) == 'b')
		{
			idToken.shiftRight(1);
			isBot = true;
		}
		else
			isBot = false;
		id = idToken.asInt(10);
	};
	auto banCheck = [this](const RenX::PlayerInfo *player)
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
				else if (this->localSteamBan && entry->steamid != 0 && entry->steamid == player->steamid)
					return true;
				else if (this->localIPBan && entry->ip != 0 && entry->ip == player->ip32)
					return true;
				else if (this->localNameBan && entry->name.isEmpty() == false && entry->name.equalsi(player->name))
					return true;
			}
		}
		return false;
	};
	auto getPlayerOrAdd = [&](const Jupiter::ReadableString &name, int id, RenX::TeamType team, bool isBot, uint64_t steamid, const Jupiter::ReadableString &ip)
	{
		RenX::PlayerInfo *r = this->getPlayer(id);
		if (r == nullptr)
		{
			r = new RenX::PlayerInfo();
			r->id = id;
			r->name = name;
			r->name.processEscapeSequences();
			r->team = team;
			r->ip = ip;
			r->ip32 = Jupiter::Socket::pton4(Jupiter::CStringS(r->ip).c_str());
			r->steamid = steamid;
			if (r->isBot = isBot)
				r->formatNamePrefix = IRCCOLOR "05[B]";
			r->joinTime = time(nullptr);
			if (id != 0)
				this->players.add(r);

			r->uuid = calc_uuid(r);

			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnPlayerCreate(this, r);
		}
		else
		{
			bool recalcUUID = false;
			r->team = team;
			if (r->ip32 == 0 && ip.isEmpty() == false)
			{
				r->ip = ip;
				r->ip32 = Jupiter::Socket::pton4(Jupiter::CStringS(r->ip).c_str());
				recalcUUID = true;
			}
			if (r->steamid == 0U && steamid != 0U)
			{
				r->steamid = steamid;
				recalcUUID = true;
			}
			if (r->name.size() == 0)
			{
				r->name = name;
				recalcUUID = true;
			}
			if (recalcUUID)
			{
				this->setUUIDIfDifferent(r, calc_uuid(r));
				if (banCheck(r))
					this->kickPlayer(r);
			}
		}
		return r;
	};
	auto parseGetPlayerOrAdd = [&parsePlayerData, &getPlayerOrAdd](const Jupiter::ReadableString &token)
	{
		PARSE_PLAYER_DATA_P(token);
		return getPlayerOrAdd(name, id, team, isBot, 0U, Jupiter::ReferenceString::empty);
	};

	if (buff.size() != 0)
	{
		switch (header[0])
		{
		case 'r':
			if (this->lastCommand.equalsi("clientlist"))
			{
				// ID | IP | Steam ID | Admin Status | Team | Name
				header.shiftRight(1);
				if (header.isEmpty() == false)
				{
					bool isBot = false;
					int id;
					uint64_t steamid = 0;
					RenX::TeamType team = TeamType::Other;
					Jupiter::ReferenceString steamToken = buff.getToken(2, RenX::DelimC);
					Jupiter::ReferenceString adminToken = buff.getToken(3, RenX::DelimC);
					Jupiter::ReferenceString teamToken = buff.getToken(4, RenX::DelimC);
					if (header.get(0) == 'b')
					{
						isBot = true;
						header.shiftRight(1);
						id = header.asInt();
						header.shiftLeft(1);
					}
					else
						id = header.asInt();

					if (steamToken.equals("-----NO-STEAM-----") == false)
						steamid = steamToken.asUnsignedLongLong();
					team = RenX::getTeam(teamToken);

					if (adminToken.equalsi("None"))
						getPlayerOrAdd(buff.getToken(5, RenX::DelimC), id, team, isBot, steamid, buff.getToken(1, RenX::DelimC));
					else
						getPlayerOrAdd(buff.getToken(5, RenX::DelimC), id, team, isBot, steamid, buff.getToken(1, RenX::DelimC))->adminType = adminToken;
				}
				header.shiftLeft(1);
			}
			else if (this->lastCommand.equalsi("clientvarlist"))
			{
				buff.shiftRight(1);
				if (this->commandListFormat.isEmpty())
					this->commandListFormat = buff;
				else
				{
					/*
					lRCON燙ommand;燙onn4爀xecuted:燾lientvarlist PlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rPlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rGDI,256,EKT-J񁒶񁒶񁒻217.9629燫x_FamilyInfo_GDI_Soldier牋燜alse񁒶�0.0000�8燦one�0x0110000104AE0666�127.0.0.1�256燛KT-J燝DI�0
					*/
					Jupiter::INIFile::Section table;
					size_t i = this->commandListFormat.tokenCount(RenX::DelimC);
					while (i-- != 0)
						table.set(this->commandListFormat.getToken(i, RenX::DelimC), buff.getToken(i, RenX::DelimC));
					auto parse = [&table](RenX::PlayerInfo *player)
					{
						Jupiter::INIFile::Section::KeyValuePair *pair;

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Kills"));
						if (pair != nullptr)
							player->kills = pair->getValue().asUnsignedInt();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Deaths"));
						if (pair != nullptr)
							player->deaths = pair->getValue().asUnsignedInt();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Score"));
						if (pair != nullptr)
							player->score = static_cast<float>(pair->getValue().asDouble());

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Credits"));
						if (pair != nullptr)
							player->credits = static_cast<float>(pair->getValue().asDouble());

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Character"));
						if (pair != nullptr)
							player->character = pair->getValue();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Vehicle"));
						if (pair != nullptr)
							player->vehicle = pair->getValue();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Ping"));
						if (pair != nullptr)
							player->ping = pair->getValue().asUnsignedInt();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Admin"));
						if (pair != nullptr)
						{
							if (pair->getValue().equals("None"))
								player->adminType = "";
							else
								player->adminType = pair->getValue();
						}
					};
					Jupiter::INIFile::Section::KeyValuePair *pair = table.getPair(STRING_LITERAL_AS_REFERENCE("PlayerLog"));
					if (pair != nullptr)
						parse(getPlayerOrAdd(Jupiter::ReferenceString::getToken(pair->getValue(), 2, ','), Jupiter::ReferenceString::getToken(pair->getValue(), 1, ',').asInt(), RenX::getTeam(Jupiter::ReferenceString::getToken(pair->getValue(), 0, ',')), false, table.get(STRING_LITERAL_AS_REFERENCE("STEAM")).asUnsignedLongLong(), table.get(STRING_LITERAL_AS_REFERENCE("IP"))));
					else
					{
						Jupiter::INIFile::Section::KeyValuePair *namePair = table.getPair(STRING_LITERAL_AS_REFERENCE("Name"));
						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("ID"));

						if (pair != nullptr)
						{
							RenX::PlayerInfo *player = getPlayer(pair->getValue().asInt());
							if (player != nullptr)
							{
								if (player->name.isEmpty())
								{
									player->name = table.get(STRING_LITERAL_AS_REFERENCE("Name"));
									player->name.processEscapeSequences();
								}
								if (player->ip.isEmpty())
									player->ip = table.get(STRING_LITERAL_AS_REFERENCE("IP"));
								if (player->steamid == 0)
								{
									uint64_t steamid = table.get(STRING_LITERAL_AS_REFERENCE("STEAM")).asUnsignedLongLong();
									if (steamid != 0)
									{
										player->steamid = steamid;
										if (calc_uuid == RenX::default_uuid_func)
											setUUID(player, this->formatSteamID(steamid));
										else
											this->setUUIDIfDifferent(player, calc_uuid(player));
									}
								}

								pair = table.getPair(STRING_LITERAL_AS_REFERENCE("TeamNum"));
								if (pair != nullptr)
									player->team = RenX::getTeam(pair->getValue().asInt());
								else
								{
									pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Team"));
									if (pair != nullptr)
										player->team = RenX::getTeam(pair->getValue());
								}

								parse(player);
							}
							// I *could* try and fetch a player by name, but that seems like it *could* open a security hole.
							// In addition, would I update their ID?
						}
						else if (namePair != nullptr)
						{
							RenX::PlayerInfo *player = getPlayerByName(namePair->getValue());
							if (player != nullptr)
							{
								if (player->ip.isEmpty())
									player->ip = table.get(STRING_LITERAL_AS_REFERENCE("IP"));
								if (player->steamid == 0)
								{
									uint64_t steamid = table.get(STRING_LITERAL_AS_REFERENCE("STEAM")).asUnsignedLongLong();
									if (steamid != 0)
									{
										player->steamid = steamid;
										if (calc_uuid == RenX::default_uuid_func)
											setUUID(player, this->formatSteamID(steamid));
										else
											this->setUUIDIfDifferent(player, calc_uuid(player));
									}
								}

								pair = table.getPair(STRING_LITERAL_AS_REFERENCE("TeamNum"));
								if (pair != nullptr)
									player->team = RenX::getTeam(pair->getValue().asInt());
								else
								{
									pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Team"));
									if (pair != nullptr)
										player->team = RenX::getTeam(pair->getValue());
								}

								parse(player);
							}
							// No other way to identify player -- worthless command format.
						}
					}
				}
				buff.shiftLeft(1);
			}
			else if (this->lastCommand.equalsi("botlist"))
			{
				// Team,ID,Name
				buff.shiftRight(1);
				if (commandListFormat.isEmpty())
					commandListFormat = buff;
				else
					parseGetPlayerOrAdd(buff);
				buff.shiftLeft(1);
			}
			else if (this->lastCommand.equalsi("botvarlist"))
			{
				buff.shiftRight(1);
				if (this->commandListFormat.isEmpty())
					this->commandListFormat = buff;
				else
				{
					/*
					lRCON燙ommand;燙onn4爀xecuted:燾lientvarlist PlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rPlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rGDI,256,EKT-J񁒶񁒶񁒻217.9629燫x_FamilyInfo_GDI_Soldier牋燜alse񁒶�0.0000�8燦one�0x0110000104AE0666�127.0.0.1�256燛KT-J燝DI�0
					*/
					Jupiter::INIFile::Section table;
					size_t i = this->commandListFormat.tokenCount(RenX::DelimC);
					while (i-- != 0)
						table.set(this->commandListFormat.getToken(i, RenX::DelimC), buff.getToken(i, RenX::DelimC));
					auto parse = [&table](RenX::PlayerInfo *player)
					{
						Jupiter::INIFile::Section::KeyValuePair *pair;

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Kills"));
						if (pair != nullptr)
							player->kills = pair->getValue().asUnsignedInt();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Deaths"));
						if (pair != nullptr)
							player->deaths = pair->getValue().asUnsignedInt();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Score"));
						if (pair != nullptr)
							player->score = static_cast<float>(pair->getValue().asDouble());

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Credits"));
						if (pair != nullptr)
							player->credits = static_cast<float>(pair->getValue().asDouble());

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Character"));
						if (pair != nullptr)
							player->character = pair->getValue();

						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Vehicle"));
						if (pair != nullptr)
							player->vehicle = pair->getValue();
					};
					Jupiter::INIFile::Section::KeyValuePair *pair = table.getPair(STRING_LITERAL_AS_REFERENCE("PlayerLog"));
					if (pair != nullptr)
						parse(getPlayerOrAdd(Jupiter::ReferenceString::getToken(pair->getValue(), 2, ','), Jupiter::ReferenceString::getToken(pair->getValue(), 1, ',').substring(1).asInt(), RenX::getTeam(Jupiter::ReferenceString::getToken(pair->getValue(), 0, ',')), true, 0ULL, Jupiter::ReferenceString::empty));
					else
					{
						Jupiter::INIFile::Section::KeyValuePair *namePair = table.getPair(STRING_LITERAL_AS_REFERENCE("Name"));
						pair = table.getPair(STRING_LITERAL_AS_REFERENCE("ID"));

						if (pair != nullptr)
						{
							RenX::PlayerInfo *player = getPlayer(pair->getValue().asInt());
							if (player != nullptr)
							{
								if (player->name.isEmpty())
								{
									player->name = table.get(STRING_LITERAL_AS_REFERENCE("Name"));
									player->name.processEscapeSequences();
								}

								pair = table.getPair(STRING_LITERAL_AS_REFERENCE("TeamNum"));
								if (pair != nullptr)
									player->team = RenX::getTeam(pair->getValue().asInt());
								else
								{
									pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Team"));
									if (pair != nullptr)
										player->team = RenX::getTeam(pair->getValue());
								}

								parse(player);
							}
						}
						else if (namePair != nullptr)
						{
							RenX::PlayerInfo *player = getPlayerByName(namePair->getValue());
							if (player != nullptr)
							{
								pair = table.getPair(STRING_LITERAL_AS_REFERENCE("TeamNum"));
								if (pair != nullptr)
									player->team = RenX::getTeam(pair->getValue().asInt());
								else
								{
									pair = table.getPair(STRING_LITERAL_AS_REFERENCE("Team"));
									if (pair != nullptr)
										player->team = RenX::getTeam(pair->getValue());
								}

								parse(player);
							}
							// No other way to identify player -- worthless command format.
						}
					}
				}
				buff.shiftLeft(1);
			}
			else if (this->lastCommand.equalsi("map"))
				this->map = buff.substring(1);
			else if (this->lastCommand.equalsi("serverinfo"))
			{
				// "Port" | Port | "Name" | Name | "Passworded" | "True"/"False" | "Level" | Level
				buff.shiftRight(1);
				this->port = static_cast<unsigned short>(buff.getToken(1, RenX::DelimC).asUnsignedInt(10));
				this->serverName = buff.getToken(3, RenX::DelimC);
				this->map = buff.getToken(7, RenX::DelimC);
				buff.shiftLeft(1);
			}
			else if (this->lastCommand.equalsi("changename"))
			{
				buff.shiftRight(1);
				RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(0, RenX::DelimC));
				Jupiter::StringS newName = buff.getToken(2, RenX::DelimC);
				newName.processEscapeSequences();
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnNameChange(this, player, newName);
				player->name = newName;
			}
			break;
		case 'l':
			if (RenX::Server::rconVersion >= 3)
			{
				header.shiftRight(1);
				Jupiter::ReferenceString subHeader = buff.getToken(1, RenX::DelimC);
				if (header.equals("GAME"))
				{
					if (subHeader.equals("Deployed;"))
					{
						// Object (Beacon/Mine) | Player
						// Object (Beacon/Mine) | Player | "on" | Surface
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
						Jupiter::ReferenceString objectType = buff.getToken(2, RenX::DelimC);
						if (objectType.match("*Beacon"))
							player->beaconPlacements++;
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDeploy(this, player, objectType);
						onAction();
					}
					else if (subHeader.equals("Disarmed;"))
					{
						// Object (Beacon/Mine) | "by" | Player
						// Object (Beacon/Mine) | "by" | Player | "owned by" | Owner
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
						Jupiter::ReferenceString objectType = buff.getToken(2, RenX::DelimC);
						if (objectType.match("*Beacon"))
							player->beaconDisarms++;

						if (buff.getToken(5, RenX::DelimC).equals("owned by"))
						{
							RenX::PlayerInfo *victim = parseGetPlayerOrAdd(buff.getToken(6, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDisarm(this, player, objectType, victim);
						}
						else
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDisarm(this, player, objectType);
						onAction();
					}
					else if (subHeader.equals("Exploded;"))
					{
						// Explosive | "at" | Location
						// Explosive | "at" | Location | "by" | Owner
						Jupiter::ReferenceString explosive = buff.getToken(2, RenX::DelimC);
						if (buff.getToken(5, RenX::DelimC).equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(6, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExplode(this, player, explosive);
						}
						else
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExplode(this, explosive);
						onAction();
					}
					else if (subHeader.equals("Captured;"))
					{
						// Team ',' Building | "id" | Building ID | "by" | Player
						Jupiter::ReferenceString teamBuildingToken = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString building = teamBuildingToken.getToken(1, ',');
						TeamType oldTeam = RenX::getTeam(teamBuildingToken.getToken(0, ','));
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(6, RenX::DelimC));
						player->captures++;
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnCapture(this, player, building, oldTeam);
						onAction();
					}
					else if (subHeader.equals("Neutralized;"))
					{
						// Team ',' Building | "id" | Building ID | "by" | Player
						Jupiter::ReferenceString teamBuildingToken = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString building = teamBuildingToken.getToken(1, ',');
						TeamType oldTeam = RenX::getTeam(teamBuildingToken.getToken(0, ','));
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(6, RenX::DelimC));
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnNeutralize(this, player, building, oldTeam);
						onAction();
					}
					else if (subHeader.equals("Purchase;"))
					{
						// "character" | Character | "by" | Player
						// "item" | Item | "by" | Player
						// "weapon" | Weapon | "by" | Player
						// "refill" | Player
						// "vehicle" | Vehicle | "by" | Player
						Jupiter::ReferenceString type = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString obj = buff.getToken(3, RenX::DelimC);
						if (type.equals("character"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnCharacterPurchase(this, player, obj);
							player->character = obj;
						}
						else if (type.equals("item"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnItemPurchase(this, player, obj);
						}
						else if (type.equals("weapon"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnWeaponPurchase(this, player, obj);
						}
						else if (type.equals("refill"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(obj);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnRefillPurchase(this, player);
						}
						else if (type.equals("vehicle"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehiclePurchase(this, player, obj);
						}
					}
					else if (subHeader.equals("Spawn;"))
					{
						// "vehicle" | Vehicle Team, Vehicle
						// "player" | Player | "character" | Character
						// "bot" | Player
						if (buff.getToken(2, RenX::DelimC).equals("vehicle"))
						{
							Jupiter::ReferenceString vehicle = buff.getToken(3, RenX::DelimC);
							Jupiter::ReferenceString vehicleTeamToken = vehicle.getToken(0, ',');
							vehicle.shiftRight(vehicleTeamToken.size() + 1);
							TeamType team = RenX::getTeam(vehicleTeamToken);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleSpawn(this, team, vehicle);
						}
						else if (buff.getToken(2, RenX::DelimC).equals("player"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(3, RenX::DelimC));
							Jupiter::ReferenceString character = buff.getToken(5, RenX::DelimC);
							player->character = character;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSpawn(this, player, character);
						}
						else if (buff.getToken(2, RenX::DelimC).equals("bot"))
						{
							RenX::PlayerInfo *bot = parseGetPlayerOrAdd(buff.getToken(3, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnBotJoin(this, bot);
						}
					}
					else if (subHeader.equals("Crate;"))
					{
						// "vehicle" | Vehicle | "by" | Player
						// "death" | "by" | Player
						// "suicide" | "by" | Player
						// "money" | Amount | "by" | Player
						// "character" | Character | "by" | Player
						// "spy" | Character | "by" | Player
						// "refill" | "by" | Player
						// "timebomb" | "by" | Player
						// "speed" | "by" | Player
						// "nuke" | "by" | Player
						// "abduction" | "by" | Player
						// "by" | Player
						Jupiter::ReferenceString type = buff.getToken(2, RenX::DelimC);
						if (type.equals("vehicle"))
						{
							Jupiter::ReferenceString vehicle = buff.getToken(3, RenX::DelimC);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleCrate(this, player, vehicle);
						}
						else if (type.equals("tsvehicle"))
						{
							Jupiter::ReferenceString vehicle = buff.getToken(3, RenX::DelimC);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleCrate(this, player, vehicle);
						}
						else if (type.equals("ravehicle"))
						{
							Jupiter::ReferenceString vehicle = buff.getToken(3, RenX::DelimC);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleCrate(this, player, vehicle);
						}
						else if (type.equals("death") || type.equals("suicide"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDeathCrate(this, player);
						}
						else if (type.equals("money"))
						{
							int amount = buff.getToken(3, RenX::DelimC).asInt();
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnMoneyCrate(this, player, amount);
						}
						else if (type.equals("character"))
						{
							Jupiter::ReferenceString character = buff.getToken(3, RenX::DelimC);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnCharacterCrate(this, player, character);
							player->character = character;
						}
						else if (type.equals("spy"))
						{
							Jupiter::ReferenceString character = buff.getToken(3, RenX::DelimC);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(5, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSpyCrate(this, player, character);
							player->character = character;
						}
						else if (type.equals("refill"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnRefillCrate(this, player);
						}
						else if (type.equals("timebomb"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnTimeBombCrate(this, player);
						}
						else if (type.equals("speed"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSpeedCrate(this, player);
						}
						else if (type.equals("nuke"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnNukeCrate(this, player);
						}
						else if (type.equals("abduction"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnAbductionCrate(this, player);
						}
						else if (type.equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(3, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnUnspecifiedCrate(this, player);
						}
						else
						{
							RenX::PlayerInfo *player = nullptr;
							if (buff.getToken(3, RenX::DelimC).equals("by"))
								player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));

							if (player != nullptr)
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnOtherCrate(this, player, type);
						}
					}
					else if (subHeader.equals("Death;"))
					{
						// "player" | Player | "by" | Killer Player | "with" | Damage Type
						// "player" | Player | "died by" | Damage Type
						// "player" | Player | "suicide by" | Damage Type
						//		NOTE: Filter these out when Player.isEmpty().
						Jupiter::ReferenceString playerToken = buff.getToken(3, RenX::DelimC);
						if (playerToken.isEmpty() == false)
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(playerToken);
							Jupiter::ReferenceString type = buff.getToken(4, RenX::DelimC);
							Jupiter::ReferenceString damageType;
							if (type.equals("by"))
							{
								damageType = buff.getToken(7, RenX::DelimC);
								Jupiter::ReferenceString killerData = buff.getToken(5, RenX::DelimC);
								Jupiter::ReferenceString kName = killerData.getToken(2, ',');
								Jupiter::ReferenceString kIDToken = killerData.getToken(1, ',');
								RenX::TeamType vTeam = RenX::getTeam(killerData.getToken(0, ','));
								if (kIDToken.equals("ai") || kIDToken.isEmpty())
								{
									player->deaths++;
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnKill(this, kName, vTeam, player, damageType);
								}
								else
								{
									player->deaths++;
									int kID = 0;
									bool kIsBot = false;
									if (kIDToken.get(0) == 'b')
									{
										kIsBot = true;
										kIDToken.shiftRight(1);
										kID = kIDToken.asInt();
										kIDToken.shiftLeft(1);
									}
									else
										kID = kIDToken.asInt();
									RenX::PlayerInfo *killer = getPlayerOrAdd(kName, kID, vTeam, kIsBot, 0, Jupiter::ReferenceString::empty);
									killer->kills++;
									if (damageType.equals("Rx_DmgType_Headshot"))
										killer->headshots++;
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnKill(this, killer, player, damageType);
								}
							}
							else if (type.equals("died by"))
							{
								player->deaths++;
								damageType = buff.getToken(5, RenX::DelimC);
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnDie(this, player, damageType);
							}
							else if (type.equals("suicide by"))
							{
								player->deaths++;
								player->suicides++;
								damageType = buff.getToken(5, RenX::DelimC);
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnSuicide(this, player, damageType);
							}
							player->character = Jupiter::ReferenceString::empty;
						}
						onAction();
					}
					else if (subHeader.equals("Stolen;"))
					{
						// Vehicle | "by" | Player
						// Vehicle | "bound to" | Bound Player | "by" | Player
						Jupiter::ReferenceString vehicle = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString byLine = buff.getToken(3, RenX::DelimC);
						if (byLine.equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							player->steals++;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSteal(this, player, vehicle);
						}
						else if (byLine.equals("bound to"))
						{
							RenX::PlayerInfo *victim = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(6, RenX::DelimC));
							player->steals++;
							victim->stolen++;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSteal(this, player, vehicle, victim);
						}
						onAction();
					}
					else if (subHeader.equals("Destroyed;"))
					{
						// "vehicle" | Vehicle | "by" | Killer | "with" | Damage Type
						// "defence" | Defence | "by" | Killer | "with" | Damage Type
						// "emplacement" | Emplacement | "by" | Killer Player | "with" | Damage Type
						// "building" | Building | "by" | Killer | "with" | Damage Type
						Jupiter::ReferenceString typeToken = buff.getToken(2, RenX::DelimC);
						RenX::ObjectType type = ObjectType::None;
						if (typeToken.equals("vehicle"))
							type = ObjectType::Vehicle;
						else if (typeToken.equals("defence") || typeToken.equals("emplacement"))
							type = ObjectType::Defence;
						else if (typeToken.equals("building"))
							type = ObjectType::Building;

						if (type != ObjectType::None)
						{
							Jupiter::ReferenceString objectName = buff.getToken(3, RenX::DelimC);
							if (buff.getToken(4, RenX::DelimC).equals("by"))
							{
								Jupiter::ReferenceString killerToken = buff.getToken(5, RenX::DelimC);
								Jupiter::ReferenceString idToken = killerToken.getToken(1, ',');
								Jupiter::ReferenceString name = killerToken.gotoToken(2, ',');
								Jupiter::ReferenceString damageType = buff.getToken(7, RenX::DelimC);

								RenX::TeamType team = RenX::getTeam(killerToken.getToken(0, ','));

								if (idToken.equals("ai") || idToken.isEmpty())
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnDestroy(this, name, team, objectName, RenX::getEnemy(team), damageType, type);
								else
								{
									int id;
									bool isBot = false;
									if (idToken.get(0) == 'b')
									{
										isBot = true;
										idToken.shiftRight(1);
									}
									id = idToken.asInt();
									RenX::PlayerInfo *player = getPlayerOrAdd(name, id, team, isBot, 0, Jupiter::ReferenceString::empty);
									switch (type)
									{
									case RenX::ObjectType::Vehicle:
										player->vehicleKills++;
										break;
									case RenX::ObjectType::Building:
										player->buildingKills++;
										break;
									case RenX::ObjectType::Defence:
										player->defenceKills++;
										break;
									default:
										break;
									}
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnDestroy(this, player, objectName, RenX::getEnemy(player->team), damageType, type);
								}
							}
						}
						onAction();
					}
					else if (subHeader.equals("Donated;"))
					{
						// Amount | "to" | Recipient | "by" | Donor
						if (buff.getToken(5, RenX::DelimC).equals("by"))
						{
							double amount = buff.getToken(2, RenX::DelimC).asDouble();
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(4, RenX::DelimC));
							RenX::PlayerInfo *donor = parseGetPlayerOrAdd(buff.getToken(6, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDonate(this, donor, player, amount);
						}
					}
					else if (subHeader.equals("MatchEnd;"))
					{
						// "winner" | Winner | Reason("TimeLimit" etc) | "GDI=" GDI Score | "Nod=" Nod Score
						// "tie" | Reason | "GDI=" GDI Score | "Nod=" Nod Score
						Jupiter::ReferenceString winTieToken = buff.getToken(2, RenX::DelimC);
						if (winTieToken.equals("winner"))
						{
							Jupiter::ReferenceString sWinType = buff.getToken(4, RenX::DelimC);
							WinType winType = WinType::Unknown;
							if (sWinType.equals("TimeLimit"))
								winType = WinType::Score;
							else if (sWinType.equals("Buildings"))
								winType = WinType::Base;
							else if (sWinType.equals("triggered"))
								winType = WinType::Shutdown;

							TeamType team = RenX::getTeam(buff.getToken(3, RenX::DelimC));

							int gScore = buff.getToken(5, RenX::DelimC).getToken(1, '=').asInt();
							int nScore = buff.getToken(6, RenX::DelimC).getToken(1, '=').asInt();

							onPreGameOver(winType, team, gScore, nScore);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnGameOver(this, winType, team, gScore, nScore);
							onPostGameOver(winType, team, gScore, nScore);
						}
						else if (winTieToken.equals("tie"))
						{
							int gScore = buff.getToken(4, RenX::DelimC).getToken(1, '=').asInt();
							int nScore = buff.getToken(5, RenX::DelimC).getToken(1, '=').asInt();
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnGameOver(this, RenX::WinType::Tie, RenX::TeamType::None, gScore, nScore);
							onPostGameOver(WinType::Tie, RenX::TeamType::None, gScore, nScore);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnGame(this, raw);
					}
				}
				else if (header.equals("CHAT"))
				{
					if (subHeader.equals("Say;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						Jupiter::ReferenceString message = buff.getToken(4, RenX::DelimC);
						onChat(player, message);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnChat(this, player, message);
						onAction();
					}
					else if (subHeader.equals("TeamSay;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						Jupiter::ReferenceString message = buff.getToken(4, RenX::DelimC);
						onChat(player, message);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnTeamChat(this, player, message);
						onAction();
					}
					else if (subHeader.equals("HostSay;"))
					{
						Jupiter::ReferenceString message = buff.getToken(3, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnHostChat(this, message);
					}
					/*else if (subHeader.equals("AdminSay;"))
					{
						// Player | "said:" | Message
						onAction();
					}
					else if (subHeader.equals("ReportSay;"))
					{
						// Player | "said:" | Message
						onAction();
					}*/
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnOtherChat(this, raw);
					}
				}
				else if (header.equals("PLAYER"))
				{
					if (subHeader.equals("Enter;"))
					{
						PARSE_PLAYER_DATA_P(buff.getToken(2, RenX::DelimC));
						uint64_t steamid = 0;
						if (buff.getToken(5, RenX::DelimC).equals("steamid"))
							steamid = buff.getToken(6, RenX::DelimC).asUnsignedLongLong();
						RenX::PlayerInfo *player = getPlayerOrAdd(name, id, team, isBot, steamid, buff.getToken(4, RenX::DelimC));
						player->joinTime = time(0);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnJoin(this, player);
					}
					else if (subHeader.equals("TeamJoin;"))
					{
						// Player | "joined" | Team
						// Player | "joined" | Team | "left" | Old Team
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						player->character = Jupiter::ReferenceString::empty;
						if (buff.tokenCount(RenX::DelimC) > 4)
						{
							RenX::TeamType oldTeam = RenX::getTeam(buff.getToken(6, RenX::DelimC));
							if (oldTeam != RenX::TeamType::None)
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnTeamChange(this, player, oldTeam);
						}
					}
					else if (subHeader.equals("Exit;"))
					{
						// Player
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						if (this->silenceParts == false)
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnPart(this, player);
						this->removePlayer(player);
					}
					else if (subHeader.equals("Kick;"))
					{
						// Player | "for" | Reason
						const Jupiter::ReadableString &reason = buff.getToken(4, RenX::DelimC);
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnKick(this, player, reason);
					}
					else if (subHeader.equals("NameChange;"))
					{
						// Player | "to:" | New Name
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						Jupiter::StringS newName = buff.getToken(4, RenX::DelimC);
						newName.processEscapeSequences();
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnNameChange(this, player, newName);
						player->name = newName;
						onAction();
					}
					else if (subHeader.equals("ChangeID;"))
					{
						// "to" | New ID | "from" | Old ID
						int oldID = buff.getToken(5, RenX::DelimC).asInt();
						RenX::PlayerInfo *player = this->getPlayer(oldID);
						if (player != nullptr)
						{
							player->id = buff.getToken(3, RenX::DelimC).asInt();
							if (banCheck(player))
								this->kickPlayer(player);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnIDChange(this, player, oldID);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnPlayer(this, raw);
					}
				}
				else if (header.equals("RCON"))
				{
					if (subHeader.equals("Command;"))
					{
						// User | "executed:" | Command
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						if (buff.getToken(3, RenX::DelimC).equals("executed:"))
						{
							Jupiter::ReferenceString command = buff.gotoToken(4, RenX::DelimC);
							Jupiter::ReferenceString cmd = command.getWord(0, " ");
							if (cmd.equalsi("hostprivatesay"))
							{
								RenX::PlayerInfo *player = this->getPlayerByName(command.getWord(1, " "));
								if (player != nullptr)
								{
									Jupiter::ReferenceString message = command.gotoWord(2, " ");
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnHostPage(this, player, message);
								}
								else
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnExecute(this, user, command);
							}
							else
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnExecute(this, user, command);
							if (this->rconUser.equals(user))
								this->lastCommand = cmd;
						}
					}
					else if (subHeader.equals("Subscribed;"))
					{
						// User
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnSubscribe(this, user);
					}
					else if (subHeader.equals("Unsubscribed;"))
					{
						// User
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnUnsubscribe(this, user);
					}
					else if (subHeader.equals("Blocked;"))
					{
						// User | Reason="(Denied by IP Policy)" / "(Not on Whitelist)"
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString message = buff.gotoToken(3, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnBlock(this, user, message);
					}
					else if (subHeader.equals("Connected;"))
					{
						// User
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnConnect(this, user);
					}
					else if (subHeader.equals("Authenticated;"))
					{
						// User
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAuthenticate(this, user);
					}
					else if (subHeader.equals("Banned;"))
					{
						// User | "reason" | Reason="(Too many password attempts)"
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString message = buff.gotoToken(4, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnBan(this, user, message);
					}
					else if (subHeader.equals("InvalidPassword;"))
					{
						// User
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnInvalidPassword(this, user);
					}
					else if (subHeader.equals("Dropped;"))
					{
						// User | "reason" | Reason="(Auth Timeout)"
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						Jupiter::ReferenceString message = buff.gotoToken(4, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDrop(this, user, message);
					}
					else if (subHeader.equals("Disconnected;"))
					{
						// User
						Jupiter::ReferenceString user = buff.getToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDisconnect(this, user);
					}
					else if (subHeader.equals("StoppedListen;"))
					{
						// Reason="(Reached Connection Limit)"
						Jupiter::ReferenceString message = buff.gotoToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnStopListen(this, message);
					}
					else if (subHeader.equals("ResumedListen;"))
					{
						// Reason="(No longer at Connection Limit)"
						Jupiter::ReferenceString message = buff.gotoToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnResumeListen(this, message);
					}
					else if (subHeader.equals("Warning;"))
					{
						// Warning="(Hit Max Attempt Records - You should investigate Rcon attempts and/or decrease prune time)"
						Jupiter::ReferenceString message = buff.gotoToken(2, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnWarning(this, message);
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnRCON(this, raw);
					}
				}
				else if (header.equals("ADMIN"))
				{
					if (subHeader.equals("Rcon;"))
					{
						// Player | "executed:" | Command
						if (buff.getToken(3, RenX::DelimC).equals("executed:"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
							Jupiter::ReferenceString cmd = buff.gotoToken(4, RenX::DelimC);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExecute(this, player, cmd);
						}
					}
					else if (subHeader.equals("Login;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						player->adminType = buff.getToken(4, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdminLogin(this, player);
					}
					else if (subHeader.equals("Logout;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdminLogout(this, player);
						player->adminType = Jupiter::ReferenceString::empty;
					}
					else if (subHeader.equals("Granted;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(2, RenX::DelimC));
						player->adminType = buff.getToken(4, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdminGrant(this, player);
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdmin(this, raw);
					}
				}
				else if (header.equals("VOTE"))
				{
					if (subHeader.equals("Called;"))
					{
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | "parameters" | Parameters(Empty) | "by" | Player
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | "by" | Player
						Jupiter::ReferenceString voteType = buff.getToken(3, RenX::DelimC);
						Jupiter::ReferenceString teamToken = buff.getToken(2, RenX::DelimC);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						Jupiter::ReferenceString playerToken;
						Jupiter::ReferenceString parameters;
						if (buff.getToken(4, RenX::DelimC).equals("parameters"))
						{
							playerToken = buff.getToken(buff.tokenCount(RenX::DelimC) - 1, RenX::DelimC);
							parameters = buff.getToken(5, RenX::DelimC);
						}
						else
							playerToken = buff.getToken(5, RenX::DelimC);

						RenX::PlayerInfo *player = parseGetPlayerOrAdd(playerToken);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVoteCall(this, team, voteType, player, parameters);
						onAction();
					}
					else if (subHeader.equals("Results;"))
					{
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | Success="pass" / "fail" | "Yes=" Yes votes | "No=" No votes
						Jupiter::ReferenceString voteType = buff.getToken(3, RenX::DelimC);
						Jupiter::ReferenceString teamToken = buff.getToken(2, RenX::DelimC);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						bool success = true;
						if (buff.getToken(4, RenX::DelimC).equals("fail"))
							success = false;

						int yesVotes = 0;
						Jupiter::ReferenceString yesVotesToken = buff.getToken(5, RenX::DelimC);
						if (yesVotesToken.size() > 4)
						{
							yesVotesToken.shiftRight(4);
							yesVotes = yesVotesToken.asInt();
						}

						int noVotes = 0;
						Jupiter::ReferenceString noVotesToken = buff.getToken(5, RenX::DelimC);
						if (yesVotesToken.size() > 3)
						{
							yesVotesToken.shiftRight(3);
							yesVotes = yesVotesToken.asInt();
						}

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVoteOver(this, team, voteType, success, yesVotes, noVotes);
					}
					else if (subHeader.equals("Cancelled;"))
					{
						// TeamType="Global" / "GDI" / "Nod" | VoteType="Rx_VoteMenuChoice_"...
						Jupiter::ReferenceString voteType = buff.getToken(3, RenX::DelimC);
						Jupiter::ReferenceString teamToken = buff.getToken(2, RenX::DelimC);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVoteCancel(this, team, voteType);
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVote(this, raw);
					}
				}
				else if (header.equals("MAP"))
				{
					if (subHeader.equals("Changing;"))
					{
						// Map | Mode="seamless" / "nonseamless"
						Jupiter::ReferenceString map = buff.getToken(2, RenX::DelimC);
						if (buff.getToken(3, RenX::DelimC).equals("seamless"))
							this->seamless = true;
						else
						{
							this->seamless = false;
							this->silenceParts = true;
						}
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMapChange(this, map, seamless);
						this->map = map;
					}
					else if (subHeader.equals("Loaded;"))
					{
						// Map
						Jupiter::ReferenceString map = buff.getToken(2, RenX::DelimC);
						this->map = map;
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMapLoad(this, map);
					}
					else if (subHeader.equals("Start;"))
					{
						// Map
						Jupiter::ReferenceString map = buff.getToken(2, RenX::DelimC);
						this->map = map;
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMapStart(this, map);
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMap(this, raw);
					}
				}
				else if (header.equals("DEMO"))
				{
					if (subHeader.equals("Record;"))
					{
						// "client request by" | Player
						// "admin command by" | Player
						// "rcon command"
						Jupiter::ReferenceString type = buff.getToken(2, RenX::DelimC);
						if (type.equals("client request by") || type.equals("admin command by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(buff.getToken(3, RenX::DelimC));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDemoRecord(this, player);
						}
						else
						{
							Jupiter::ReferenceString user = buff.getToken(3, RenX::DelimC); // not actually used, but here for possible future usage
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDemoRecord(this, user);
						}
					}
					else if (subHeader.equals("RecordStop;"))
					{
						// Empty
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDemoRecordStop(this);
					}
					else
					{
						Jupiter::ReferenceString raw = buff.gotoToken(1, RenX::DelimC);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDemo(this, raw);
					}
				}
				/*else if (header.equals("ERROR;")) // Decided to disable this entirely, since it's unreachable anyways.
				{
					// Should be under RCON.
					// "Could not open TCP Port" Port "- Rcon Disabled"
				}*/
				else
				{
					buff.shiftRight(1);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnLog(this, buff);
					buff.shiftLeft(1);
				}
			}
			break;

		case 'c':
			buff.shiftRight(1);
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnCommand(this, buff);
			this->commandListFormat.set(Jupiter::ReferenceString::empty);
			this->lastCommand = Jupiter::ReferenceString::empty;
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

			if (this->rconVersion >= 3)
			{
				RenX::Server::sock.send(STRING_LITERAL_AS_REFERENCE("s\n"));
				RenX::Server::send(STRING_LITERAL_AS_REFERENCE("serverinfo"));
				RenX::Server::fetchClientList();

				this->firstGame = true;
				this->seamless = true;

				/*else if (this->firstGame == false)
				{
				this->firstAction = false;
				this->silenceJoins = true;
				}*/

				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnVersion(this, buff);
				buff.shiftLeft(1);
			}
			else
				this->disconnect();
			break;

		case 'a':
			buff.shiftRight(1);
			RenX::Server::rconUser = buff;
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
	if (RenX::Server::sock.connect(RenX::Server::hostname.c_str(), RenX::Server::port, RenX::Server::clientHostname.isEmpty() ? nullptr : RenX::Server::clientHostname.c_str()))
	{
		RenX::Server::sock.setBlocking(false);
		RenX::Server::sock.send(Jupiter::StringS::Format("a%.*s\n", RenX::Server::pass.size(), RenX::Server::pass.ptr()));
		RenX::Server::connected = true;
		RenX::Server::silenceParts = false;
		RenX::Server::attempts = 0;
		return true;
	}
	RenX::Server::connected = false;
	++RenX::Server::attempts;
	return false;
}

bool RenX::Server::reconnect()
{
	RenX::Server::disconnect();
	return RenX::Server::connect();
}

void RenX::Server::wipeData()
{
	RenX::Server::rconVersion = 0;
	RenX::Server::rconUser.truncate(RenX::Server::rconUser.size());
	while (RenX::Server::players.size() != 0)
		delete RenX::Server::players.remove(0U);
}

unsigned int RenX::Server::getVersion() const
{
	return RenX::Server::rconVersion;
}

const Jupiter::ReadableString &RenX::Server::getGameVersion() const
{
	return RenX::Server::gameVersion;
}

const Jupiter::ReadableString &RenX::Server::getRCONUsername() const
{
	return RenX::Server::rconUser;
}

RenX::Server::Server(Jupiter::Socket &&socket, const Jupiter::ReadableString &configurationSection) : Server(configurationSection)
{
	RenX::Server::sock = std::move(socket);
	RenX::Server::hostname = RenX::Server::sock.getHostname();
	RenX::Server::sock.send(Jupiter::StringS::Format("a%.*s\n", RenX::Server::pass.size(), RenX::Server::pass.ptr()));
	RenX::Server::connected = true;
	RenX::Server::silenceParts = false;
}

RenX::Server::Server(const Jupiter::ReadableString &configurationSection)
{
	RenX::Server::configSection = configurationSection;
	RenX::Server::calc_uuid = RenX::default_uuid_func;
	init();
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		xPlugins.get(i)->RenX_OnServerCreate(this);
}

void RenX::Server::init()
{
	RenX::Server::hostname = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Hostname"), STRING_LITERAL_AS_REFERENCE("localhost"));
	RenX::Server::port = static_cast<unsigned short>(Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Port"), 7777));
	RenX::Server::clientHostname = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ClientAddress"));
	RenX::Server::pass = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Password"), STRING_LITERAL_AS_REFERENCE("renx"));

	RenX::Server::logChanType = Jupiter::IRC::Client::Config->getShort(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ChanType"));
	RenX::Server::adminLogChanType = Jupiter::IRC::Client::Config->getShort(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("AdminChanType"));

	RenX::Server::setCommandPrefix(Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("CommandPrefix")));
	RenX::Server::setPrefix(Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("IRCPrefix")));

	RenX::Server::rules = Jupiter::IRC::Client::Config->get(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("Rules"), STRING_LITERAL_AS_REFERENCE("Anarchy!"));
	RenX::Server::delay = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ReconnectDelay"), 10);
	RenX::Server::maxAttempts = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("MaxReconnectAttempts"), -1);
	RenX::Server::rconBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("RCONBan"), false);
	RenX::Server::localSteamBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("LocalSteamBan"), true);
	RenX::Server::localIPBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("LocalIPBan"), true);
	RenX::Server::localNameBan = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("LocalNameBan"), false);
	RenX::Server::localBan = RenX::Server::localIPBan || RenX::Server::localSteamBan || RenX::Server::localNameBan;
	RenX::Server::steamFormat = Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("SteamFormat"), 16);
	RenX::Server::neverSay = Jupiter::IRC::Client::Config->getBool(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("NeverSay"), false);
	RenX::Server::clientUpdateRate = std::chrono::milliseconds(Jupiter::IRC::Client::Config->getInt(RenX::Server::configSection, STRING_LITERAL_AS_REFERENCE("ClientUpdateRate"), 2500));

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
