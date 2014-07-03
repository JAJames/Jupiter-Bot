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

bool RenX::Server::isConnected() const
{
	return RenX::Server::connected;
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
	RenX::sanitizeString(t);
	r = RenX::Server::sock.send(t, command.size() + 2);
	delete[] t;
	return r;
}

int RenX::Server::sendMessage(const Jupiter::ReadableString &message)
{
	char *t = new char[message.size() + 6];
	strcpy(t, "csay ");
	for (size_t i = 0; i != message.size(); i++) t[i + 5] = message.get(i);
	t[message.size() + 5] = '\n';
	RenX::sanitizeString(t);
	int r = RenX::Server::sock.send(t, message.size() + 6);
	delete[] t;
	return r;
}

int RenX::Server::sendMessage(RenX::PlayerInfo *player, const Jupiter::ReadableString &message)
{
	// We'll have a real implementation when Kil adds one to RCON! :D
	return RenX::Server::sendMessage(message);
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
	int id = name.asInt(10);

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
		if (node->data->name.find(partName) != Jupiter::INVALID_INDEX)
			return node->data;
	return nullptr;
}

void RenX::Server::kickPlayer(int id)
{
	RenX::Server::sock.send(Jupiter::StringS::Format("cadminkick pid%d\n", id));
}

void RenX::Server::kickPlayer(const RenX::PlayerInfo *player)
{
	if (player->isBot)
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkick %.*s", player->name.size(), player->name.ptr()));
	else if (player->id < 1000 || this->rconVersion > 2)
		RenX::Server::kickPlayer(player->id);
	else if (player->name.contains('|') == false)
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkick %.*s", player->name.size(), player->name.ptr()));
	else
		RenX::Server::kickPlayer(player->id);
}

void RenX::Server::banPlayer(int id)
{
	Jupiter::StringS msg;
	msg.format("cadminkickban pid%d\n", id);
	RenX::Server::sock.send(msg);
}

void RenX::Server::banPlayer(const RenX::PlayerInfo *player)
{
	if (player->isBot)
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban %.*s\n", player->name.size(), player->name.ptr()));
	else if (player->id < 1000 || this->rconVersion > 2)
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban pid%d\n", player->id));
	else if (player->name.contains('|') == false)
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban %.*s\n", player->name.size(), player->name.ptr()));
	else
		RenX::Server::sock.send(Jupiter::StringS::Format("cadminkickban pid%d\n", player->id));
}

bool RenX::Server::removePlayer(int id)
{
	if (RenX::Server::players.size() == 0) return false;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = RenX::Server::players.getNode(0); node != nullptr; node = node->next)
	{
		if (node->data->id == id)
		{
			RenX::PlayerInfo *p = RenX::Server::players.remove(node);
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
	while (RenX::Server::players.size() != 0) delete RenX::Server::players.remove(0U);
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

	for (size_t i = 0; i < RenX::GameMasterCommandList->size(); i++)
		RenX::Server::addCommand(RenX::GameMasterCommandList->get(i)->copy());
}

RenX::Server::~Server()
{
	sock.closeSocket();
	RenX::Server::wipeData();
}
