/**
 * Copyright (C) 2014-2021 Jessica James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include "jessilib/unicode.hpp"
#include "Jupiter/IRC_Client.h"
#include "IRC_Bot.h"
#include "RenX_ModSystem.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Core.h"
#include "RenX_Functions.h"

using namespace Jupiter::literals;
using namespace std::literals;

constexpr std::string_view game_administrator_name = "administrator"sv;
constexpr std::string_view game_moderator_name = "moderator"sv;

bool RenX_ModSystemPlugin::initialize() {
	m_lockSteam = this->config.get<bool>("LockSteam"_jrs, true);
	m_lockIP = this->config.get<bool>("LockIP"_jrs, false);
	m_lockName = this->config.get<bool>("LockName"_jrs, false);
	m_kickLockMismatch = this->config.get<bool>("KickLockMismatch"_jrs, true);
	m_autoAuthSteam = this->config.get<bool>("AutoAuthSteam"_jrs, true);
	m_autoAuthIP = this->config.get<bool>("AutoAuthIP"_jrs, false);
	m_atmDefault = this->config.get("ATMDefault"_jrs);
	m_moderatorGroup = this->config.get("Moderator"_jrs, "Moderator"_jrs);
	m_administratorGroup = this->config.get("Administrator"_jrs, "Administrator"_jrs);

	ModGroup *group;
	Jupiter::ReferenceString dotLockSteam = ".LockSteam";
	Jupiter::ReferenceString dotLockIP = ".LockIP";
	Jupiter::ReferenceString dotLockName = ".LockName";
	Jupiter::ReferenceString dotKickLockMismatch = ".KickLockMismatch";
	Jupiter::ReferenceString dotAutoAuthSteam = ".AutoAuthSteam";
	Jupiter::ReferenceString dotAutoAuthIP = ".AutoAuthIP";
	Jupiter::ReferenceString dotNext = ".Next";
	Jupiter::ReferenceString dotAccess = ".Access";
	Jupiter::ReferenceString dotPrefix = ".Prefix";
	Jupiter::ReferenceString dotGamePrefix = ".GamePrefix";

	std::string groupName = this->config.get("Default"_jrs, ""s);

	while (!groupName.empty())
	{
		// Add group
		groups.emplace_back();
		group = &groups.back();
		group->name = groupName;

		groupName += dotLockSteam;
		group->lockSteam = this->config.get<bool>(groupName, m_lockSteam);
		groupName.erase(dotLockSteam.size());

		groupName += dotLockIP;
		group->lockIP = this->config.get<bool>(groupName, m_lockIP);
		groupName.erase(dotLockIP.size());

		groupName += dotLockName;
		group->lockName = this->config.get<bool>(groupName, m_lockName);
		groupName.erase(dotLockName.size());

		groupName += dotKickLockMismatch;
		group->kickLockMismatch = this->config.get<bool>(groupName, m_kickLockMismatch);
		groupName.erase(dotKickLockMismatch.size());

		groupName += dotAutoAuthSteam;
		group->autoAuthSteam = this->config.get<bool>(groupName, m_autoAuthSteam);
		groupName.erase(dotAutoAuthSteam.size());

		groupName += dotAutoAuthIP;
		group->autoAuthIP = this->config.get<bool>(groupName, m_autoAuthIP);
		groupName.erase(dotAutoAuthIP.size());

		groupName += dotAccess;
		group->access = this->config.get<int>(groupName);
		groupName.erase(dotAccess.size());
		
		groupName += dotPrefix;
		group->prefix = this->config.get(groupName);
		groupName.erase(dotPrefix.size());

		groupName += dotGamePrefix;
		group->gamePrefix = this->config.get(groupName);
		groupName.erase(dotGamePrefix.size());

		// Next
		groupName += dotNext;
		groupName = this->config.get(groupName);
	}

	RenX::Core *core = RenX::getCore();
	size_t server_count = core->getServerCount();
	RenX::Server *server;
	while (server_count != 0) {
		server = core->getServer(--server_count);
		if (server->players.size() != server->getBotCount()) {
			for (auto node = server->players.begin(); node != server->players.end(); ++node) {
				auth(*server, *node, true);
			}
		}
	}

	return true;
}

unsigned int RenX_ModSystemPlugin::logoutAllMods(RenX::Server &server) {
	if (server.players.size() == 0)
		return 0;

	unsigned int total = 0;
	for (auto node = server.players.begin(); node != server.players.end(); ++node)
		if (resetAccess(*node))
			total++;

	return total;
}

bool RenX_ModSystemPlugin::resetAccess(RenX::PlayerInfo &player) {
	int oAccess = player.access;
	if (player.adminType == game_administrator_name)
	{
		ModGroup *group = getGroupByName(m_administratorGroup);
		if (group == nullptr)
			player.access = 2;
		else
			player.access = group->access;
	}
	else if (player.adminType == game_moderator_name)
	{
		ModGroup *group = getGroupByName(m_moderatorGroup);
		if (group == nullptr)
			player.access = 1;
		else
			player.access = group->access;
	}
	else if (groups.size() != 0)
		player.access = groups.front().access;
	else
		player.access = 0;

	return player.access != oAccess;
}

int RenX_ModSystemPlugin::auth(RenX::Server &server, const RenX::PlayerInfo &player, bool checkAuto, bool forceAuth) const {
	if (player.isBot)
		return 0;

	const ModGroup *group;
	if (player.uuid.isNotEmpty())
	{
		Jupiter::Config *section = this->config.getSection(player.uuid);
		if (section != nullptr)
		{
			std::string_view groupName = section->get("Group"_jrs);

			if (groupName.empty())
				group = &RenX_ModSystemPlugin::groups.front();
			else
			{
				group = getGroupByName(groupName);
				if (group == nullptr)
					group = &RenX_ModSystemPlugin::groups.front();
			}

			auto sectionAuth = [&]
			{
				player.varData[this->name].set("Group"_jrs, group->name);
				player.formatNamePrefix = section->get("Prefix"_jrs, group->prefix);
				player.gamePrefix = section->get("GamePrefix"_jrs, group->gamePrefix);
				player.access = section->get<int>("Access"_jrs, group->access);
				if (player.access != 0)
				{
					server.sendMessage(player, Jupiter::StringS::Format("You are now authenticated with access level %d; group: %.*s.", player.access, group->name.size(), group->name.data()));
					if (server.isDevBot() && player.access > 1)
					{
						if (server.getVersion() >= 4)
							server.sendData(Jupiter::StringS::Format("dset_dev %d\n", player.id));
						else
							server.sendData(Jupiter::StringS::Format("d%d\n", player.id));
					}
				}
				Jupiter::String playerName = RenX::getFormattedPlayerName(player);
				server.sendLogChan(IRCCOLOR "03[Authentication] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is now authenticated with access level %d; group: %.*s.", playerName.size(), playerName.ptr(), player.access, group->name.size(), group->name.data());
				return player.access;
			};

			if (forceAuth)
				return sectionAuth();

			bool lockSteam_l = section->get<bool>("LockSteam"_jrs, group->lockSteam);
			bool lockIP_l = section->get<bool>("LockIP"_jrs, group->lockIP);
			bool lockName_l = section->get<bool>("LockName"_jrs, group->lockName);
			bool kickLockMismatch_l = section->get<bool>("KickLockMismatch"_jrs, group->kickLockMismatch);
			bool autoAuthSteam_l = section->get<bool>("AutoAuthSteam"_jrs, group->autoAuthSteam);
			bool autoAuthIP_l = section->get<bool>("AutoAuthIP"_jrs, group->autoAuthIP);

			uint64_t steamid = Jupiter::ReferenceString{section->get("SteamID"_jrs)}.asUnsignedLongLong();
			std::string_view ip = section->get("LastIP"_jrs);
			std::string_view name = section->get("Name"_jrs);

			if ((lockSteam_l == false || player.steamid == steamid) && (lockIP_l == false || player.ip == ip) && (lockName_l == false || jessilib::equalsi(player.name, name)))
			{
				if (checkAuto == false || (autoAuthSteam_l && player.steamid == steamid) || (autoAuthIP_l && player.ip == ip))
					return sectionAuth();
			}
			else if (kickLockMismatch_l)
			{
				server.kickPlayer(player, "Moderator entry lock mismatch"_jrs);
				return -1;
			}
		}
	}
	group = this->getDefaultGroup();

	player.varData[this->name].set("Group"_jrs, group->name);
	player.formatNamePrefix = group->prefix;
	player.gamePrefix = group->gamePrefix;
	return player.access = group->access;
}

void RenX_ModSystemPlugin::tempAuth(RenX::Server &server, const RenX::PlayerInfo &player, const ModGroup *group, bool notify) const {
	if (group == nullptr)
		group = this->getDefaultGroup();

	player.varData[name].set("Group"_jrs, group->name);
	player.formatNamePrefix = group->prefix;
	player.gamePrefix = group->gamePrefix;
	player.access = group->access;

	if (notify)
		server.sendMessage(player, Jupiter::StringS::Format("You have been authorized into group \"%.*s\", with access level %u.", group->name.size(), group->name.data(), player.access));
}

bool RenX_ModSystemPlugin::set(RenX::PlayerInfo &player, ModGroup &group) {
	bool r = this->config[player.uuid].set("Group"_jrs, group.name);
	this->config[player.uuid].set("SteamID"_jrs, static_cast<std::string>(Jupiter::StringS::Format("%llu", player.steamid)));
	this->config[player.uuid].set("LastIP"_jrs, static_cast<std::string>(player.ip));
	this->config[player.uuid].set("Name"_jrs, player.name);
	this->config.write();

	return r;
}

bool RenX_ModSystemPlugin::removeModSection(std::string_view section) {
	return config.removeSection(section) && config.write();
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getGroupByName(std::string_view name, ModGroup *defaultGroup) const {
	for (const auto& group : groups) {
		if (jessilib::equalsi(group.name, name)) {
			return const_cast<ModGroup*>(&group);
		}
	}

	return defaultGroup;
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getGroupByAccess(int access, ModGroup *defaultGroup) const {
	if (RenX_ModSystemPlugin::groups.size() != 0)
		for (auto node = this->groups.begin(); node != this->groups.end(); ++node)
			if (node->access == access)
				return const_cast<ModGroup *>(&*node);

	return defaultGroup;
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getGroupByIndex(size_t index) const {
	if (RenX_ModSystemPlugin::groups.size() != 0)
		for (auto node = this->groups.begin(); node != this->groups.end(); ++node)
			if (index-- == 0)
				return const_cast<ModGroup *>(&*node);

	return nullptr;
}

int RenX_ModSystemPlugin::getConfigAccess(std::string_view uuid) const {
	Jupiter::Config *section = this->config.getSection(uuid);

	if (section == nullptr) {
		return RenX_ModSystemPlugin::groups.front().access;
	}

	return section->get<int>("Access"_jrs, getGroupByName(section->get("Group"_jrs),const_cast<ModGroup *>(&groups.front()))->access);
}

size_t RenX_ModSystemPlugin::getGroupCount() const {
	return RenX_ModSystemPlugin::groups.size();
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getDefaultGroup() const {
	return const_cast<ModGroup *>(&RenX_ModSystemPlugin::groups.front());
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getDefaultATMGroup() const {
	return getGroupByName(m_atmDefault);
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getModeratorGroup() const {
	return getGroupByName(m_moderatorGroup);
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getAdministratorGroup() const {
	return getGroupByName(m_administratorGroup);
}

RenX_ModSystemPlugin::~RenX_ModSystemPlugin() {
	RenX::Core *core = RenX::getCore();
	size_t server_count = core->getServerCount();
	RenX::Server *server;
	while (server_count != 0) {
		server = core->getServer(--server_count);
		if (server->players.size() != server->getBotCount()) {
			for (auto node = server->players.begin(); node != server->players.end(); ++node) {
				if (node->isBot == false) {
					node->varData[RenX_ModSystemPlugin::name].remove("Group"_jrs);
					node->gamePrefix.truncate(node->gamePrefix.size());
					node->formatNamePrefix.truncate(node->formatNamePrefix.size());
					if (node->adminType == game_administrator_name)
						node->access = 2;
					else if (node->adminType == game_moderator_name)
						node->access = 1;
					else
						node->access = 0;
				}
			}
		}
	}

	RenX_ModSystemPlugin::groups.clear();
}

void RenX_ModSystemPlugin::RenX_OnPlayerCreate(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (player.isBot == false)
		auth(server, player, true);
}

void RenX_ModSystemPlugin::RenX_OnPlayerDelete(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (RenX_ModSystemPlugin::groups.size() != 0 && player.isBot == false && player.uuid.isNotEmpty()) {
		Jupiter::Config *section = this->config.getSection(player.uuid);
		if (section != nullptr) {
			section->set("SteamID"_jrs, static_cast<std::string>(Jupiter::StringS::Format("%llu", player.steamid)));
			section->set("LastIP"_jrs, static_cast<std::string>(player.ip));
			section->set("Name"_jrs, player.name);
		}
	}
}

void RenX_ModSystemPlugin::RenX_OnIDChange(RenX::Server &server, const RenX::PlayerInfo &player, int oldID) {
	if (player.access != 0 && server.isDevBot()) {
		server.sendData(Jupiter::StringS::Format("d%d\n", player.id));
	}
}

void RenX_ModSystemPlugin::RenX_OnAdminLogin(RenX::Server &server, const RenX::PlayerInfo &player) {
	ModGroup *group = nullptr;
	if (player.adminType == game_administrator_name)
		group = getGroupByName(m_administratorGroup);
	else if (player.adminType == game_moderator_name)
		group = getGroupByName(m_moderatorGroup);

	if (group != nullptr && player.access < group->access)
		player.access = group->access;
}

void RenX_ModSystemPlugin::RenX_OnAdminGrant(RenX::Server &server, const RenX::PlayerInfo &player) {
	RenX_OnAdminLogin(server, player);
}

void RenX_ModSystemPlugin::RenX_OnAdminLogout(RenX::Server &server, const RenX::PlayerInfo &player) {
	ModGroup *group = nullptr;
	int access = RenX_ModSystemPlugin::groups.size() == 0 ? 0 : RenX_ModSystemPlugin::groups.front().access;
	if (player.adminType == game_administrator_name)
	{
		access = 2;
		group = getGroupByName(m_administratorGroup);
	}
	else if (player.adminType == game_moderator_name)
	{
		access = 1;
		group = getGroupByName(m_moderatorGroup);
	}
	if (group != nullptr)
		access = group->access;

	if (player.access <= access)
	{
		if (RenX_ModSystemPlugin::groups.size() == 0)
			player.access = 0;
		else
			player.access = RenX_ModSystemPlugin::groups.front().access;
	}
}

int RenX_ModSystemPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();
	RenX_ModSystemPlugin::groups.clear();
	return this->initialize() ? 0 : -1;
}

// Plugin instantiation and entry point.
RenX_ModSystemPlugin pluginInstance;

/** IRC Commands */

// Auth IRC Command

void AuthIRCCommand::create()
{
	this->addTrigger("auth"_jrs);
	this->setAccessLevel(3);
}

void AuthIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			RenX::Server *server;
			RenX::PlayerInfo *player;
			int type = chan->getType();
			bool serverMatch = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					serverMatch = true;
					player = server->getPlayerByPartName(parameters);
					if (player == nullptr)
						source->sendNotice(nick, "Error: Player not found."_jrs);
					else
					{
						int uAccess = source->getAccessLevel(channel, nick);
						int cAccess = pluginInstance.getConfigAccess(player->uuid);
						if (cAccess > uAccess && uAccess < static_cast<int>(source->getPrefixes().size()))
							source->sendNotice(nick, "Error: Can't authenticate higher level moderators."_jrs);
						else if (player->access == cAccess)
							source->sendNotice(nick, "Error: Player is already authenticated"_jrs);
						else if (player->access > cAccess)
							source->sendNotice(nick, "Error: Player is already temporarily authenticated."_jrs);
						else
						{
							RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
							if (pluginInstance.auth(*server, *player) == -1)
								source->sendNotice(nick, "Error: Player failed to pass strict lock checks. Player kicked."_jrs);
							else if (defaultGroup->name == player->varData[pluginInstance.getName()].get("Group"_jrs))
								source->sendNotice(nick, "Error: Failed to authenticate player."_jrs);
							else
								source->sendNotice(nick, "Player authenticated successfully."_jrs);
						}
					}
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
	else
		this->trigger(source, channel, nick, nick);
}

const Jupiter::ReadableString &AuthIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Authenticates a player in-game. Syntax: auth [player=you]");
	return defaultHelp;
}

IRC_COMMAND_INIT(AuthIRCCommand)

// DeAuth IRC Command

void DeAuthIRCCommand::create()
{
	this->addTrigger("unauth"_jrs);
	this->addTrigger("deauth"_jrs);
	this->addTrigger("demod"_jrs);
	this->addTrigger("dtm"_jrs);
	this->setAccessLevel(3);
}

void DeAuthIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			RenX::Server *server;
			RenX::PlayerInfo *player;
			int type = chan->getType();
			bool serverMatch = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					serverMatch = true;
					player = server->getPlayerByPartName(parameters);
					if (player == nullptr)
						source->sendNotice(nick, "Error: Player not found."_jrs);
					else
					{
						int uAccess = source->getAccessLevel(channel, nick);
						int cAccess = pluginInstance.getConfigAccess(player->uuid);
						if (cAccess > uAccess && uAccess < static_cast<int>(source->getPrefixes().size()))
							source->sendNotice(nick, "Error: Can't unauthenticate higher level moderators."_jrs);
						else if (pluginInstance.resetAccess(*player))
							source->sendNotice(nick, "Player unauthenticated successfully."_jrs);
						else
							source->sendNotice(nick, "Error: Player not authenticated."_jrs);
					}
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
	else
		this->trigger(source, channel, nick, nick);
}

const Jupiter::ReadableString &DeAuthIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Unauthenticates a player in-game. Syntax: deauth [player=you]");
	return defaultHelp;
}

IRC_COMMAND_INIT(DeAuthIRCCommand)

// ATM IRC Command

void ATMIRCCommand::create()
{
	this->addTrigger("atm"_jrs);
	this->setAccessLevel(3);
}

void ATMIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
		this->trigger(source, channel, nick, nick);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			RenX::Server *server;
			RenX::PlayerInfo *player;
			RenX_ModSystemPlugin::ModGroup *group = pluginInstance.getDefaultATMGroup();
			int type = chan->getType();
			bool serverMatch = false;
			Jupiter::ReferenceString playerName = parameters;
			if (isdigit(parameters.get(0)))
			{
				int index = parameters.asInt();

				if (index < 0 || index >= static_cast<int>(pluginInstance.groups.size()))
					source->sendNotice(nick, "Warning: Invalid group index. Ingoring parameter..."_jrs);
				else if (index == 0)
				{
					source->sendNotice(nick, "Error: Default group is not valid for this command. Use \"deauth\" to deauthorize a player."_jrs);
					return;
				}
				else
				{
					group = pluginInstance.getGroupByIndex(index);
					if (group->access > source->getAccessLevel(channel, nick))
					{
						group = pluginInstance.getDefaultATMGroup();
						source->sendNotice(nick, "Warning: You can not authorize an access level higher than yourself. Ignoring parameter..."_jrs);
					}
					playerName = playerName.gotoWord(1, WHITESPACE);
					if (playerName.isEmpty())
						playerName = nick;
				}
			}
			if (group == nullptr)
				source->sendNotice(nick, "Error: Invalid group."_jrs);
			else
			{
				for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
				{
					server = RenX::getCore()->getServer(i);
					if (server->isLogChanType(type))
					{
						serverMatch = true;
						player = server->getPlayerByPartName(playerName);
						if (player == nullptr)
							source->sendNotice(nick, "Error: Player not found."_jrs);
						else if (player->access > group->access)
							source->sendNotice(nick, "Error: This command can not lower a player's access level."_jrs);
						else
						{
							pluginInstance.tempAuth(*server, *player, group);
							source->sendNotice(nick, "Player successfully temporarily authenticated."_jrs);
						}
					}
				}
				if (serverMatch == false)
					source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
			}
		}
	}
}

const Jupiter::ReadableString &ATMIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Temporarily authenticates a player in-game. Syntax: atm [level] [player=you]");
	return defaultHelp;
}

IRC_COMMAND_INIT(ATMIRCCommand)

// Add IRC Command

void AddIRCCommand::create()
{
	this->addTrigger("addmod"_jrs);
	this->addTrigger("add"_jrs);
	this->addTrigger("set"_jrs);
	this->setAccessLevel(5);
}

void AddIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) < 2)
		source->sendNotice(nick, "Error: Too few parameters. Syntax: add <level> <player>"_jrs);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			RenX::Server *server;
			RenX::PlayerInfo *player;
			RenX_ModSystemPlugin::ModGroup *group = nullptr;
			int type = chan->getType();
			bool serverMatch = false;
			Jupiter::ReferenceString playerName = parameters;
			if (isdigit(parameters.get(0)))
			{
				int index = parameters.asInt();

				if (index < 0 || index >= static_cast<int>(pluginInstance.groups.size()))
					source->sendNotice(nick, "Error: Invalid group index."_jrs);
				else
				{
					group = pluginInstance.getGroupByIndex(index);
					playerName = playerName.gotoWord(1, WHITESPACE);
				}
			}
			if (group == nullptr)
				source->sendNotice(nick, "Error: Invalid group."_jrs);
			else
			{
				for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
				{
					server = RenX::getCore()->getServer(i);
					if (server->isLogChanType(type))
					{
						serverMatch = true;
						player = server->getPlayerByPartName(playerName);
						if (player == nullptr)
							source->sendNotice(nick, "Error: Player not found."_jrs);
						else if (player->isBot)
							source->sendNotice(nick, "Error: A bot can not be a moderator."_jrs);
						else if (player->uuid.isEmpty())
							source->sendNotice(nick, "Error: Player has no UUID."_jrs);
						else
						{
							pluginInstance.resetAccess(*player);
							if (pluginInstance.set(*player, *group))
								source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been added to group \"%.*s\"", player->name.size(), player->name.data(), group->name.size(), group->name.data()));
							else
								source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been moved to group \"%.*s\"", player->name.size(), player->name.data(), group->name.size(), group->name.data()));
							pluginInstance.auth(*server, *player, false, true);
						}
					}
				}
				if (serverMatch == false)
					source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
			}
		}
	}
}

const Jupiter::ReadableString &AddIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds a player to the in-game moderator list. Syntax: add <level> <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(AddIRCCommand)

// Del IRC Command

void DelIRCCommand::create()
{
	this->addTrigger("delmod"_jrs);
	this->addTrigger("remmod"_jrs);
	this->addTrigger("del"_jrs);
	this->addTrigger("rem"_jrs);
	this->setAccessLevel(5);
}

void DelIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	std::string_view parameters_view = parameters;
	if (parameters.isEmpty())
		source->sendNotice(nick, "Error: Too few parameters. Syntax: del <player>"_jrs);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			RenX::Server *server;
			RenX::PlayerInfo *player;
			int type = chan->getType();
			bool serverMatch = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					serverMatch = true;
					player = server->getPlayerByPartName(parameters);
					if (player == nullptr)
					{
						if (pluginInstance.removeModSection(parameters))
							source->sendNotice(nick, "Player has been removed from the moderator list."_jrs);
						else
						{
							for (auto& section : pluginInstance.getConfig().getSections())
							{
								if (jessilib::equalsi(section.second.get("Name"_jrs), parameters_view)) {
									if (pluginInstance.removeModSection(section.first))
										source->sendNotice(nick, "Player has been removed from the moderator list."_jrs);
									else
										source->sendNotice(nick, "Error: Unknown error occurred."_jrs);

									return;
								}
							}

							source->sendNotice(nick, "Error: Player not found."_jrs);
						}
					}
					else if (player->isBot)
						source->sendNotice(nick, "Error: A bot can not be a moderator."_jrs);
					else if (pluginInstance.removeModSection(player->uuid))
						source->sendNotice(nick, "Player has been removed from the moderator list."_jrs);
					else
						source->sendNotice(nick, "Player is not in the moderator list."_jrs);

					break;
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
}

const Jupiter::ReadableString &DelIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes a player from the in-game moderator list. Syntax: del <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DelIRCCommand)

// ForceAuth IRC Command

void ForceAuthIRCCommand::create()
{
	this->addTrigger("fauth"_jrs);
	this->addTrigger("forceauth"_jrs);
	this->setAccessLevel(4);
}

void ForceAuthIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			RenX::Server *server;
			RenX::PlayerInfo *player;
			int type = chan->getType();
			bool serverMatch = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					serverMatch = true;
					player = server->getPlayerByPartName(parameters);
					if (player == nullptr)
						source->sendNotice(nick, "Error: Player not found."_jrs);
					else
					{
						int uAccess = source->getAccessLevel(channel, nick);
						int cAccess = pluginInstance.getConfigAccess(player->uuid);
						if (cAccess > uAccess && uAccess < static_cast<int>(source->getPrefixes().size()))
							source->sendNotice(nick, "Error: Can't authenticate higher level moderators."_jrs);
						else if (player->access == cAccess)
							source->sendNotice(nick, "Error: Player is already authenticated"_jrs);
						else if (player->access > cAccess)
							source->sendNotice(nick, "Error: Player is already temporarily authenticated."_jrs);
						else
						{
							RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
							pluginInstance.auth(*server, *player, false, true);
							if (defaultGroup->name == player->varData[pluginInstance.getName()].get("Group"_jrs))
								source->sendNotice(nick, "Error: Failed to authenticate player."_jrs);
							else
								source->sendNotice(nick, "Player authenticated successfully."_jrs);
						}
					}
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
	else
		this->trigger(source, channel, nick, nick);
}

const Jupiter::ReadableString &ForceAuthIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Forcefully authenticates a player in-game. Syntax: auth [player=you]");
	return defaultHelp;
}

IRC_COMMAND_INIT(ForceAuthIRCCommand)

// ModList IRC Command

void ModListIRCCommand::create()
{
	this->addTrigger("modlist"_jrs);
	this->addTrigger("mlist"_jrs);
}

void ModListIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::String msg;
	size_t msgBaseSize;
	bool haveMods = false;
	for (auto node = pluginInstance.groups.begin(); node != pluginInstance.groups.end(); ++node)
	{
		msg = node->prefix;
		msg += node->name;
		msg.aformat(IRCNORMAL " (Access: %d): ", node->access);
		msgBaseSize = msg.size();

		for (auto& section : pluginInstance.getConfig().getSections()) {
			if (jessilib::equalsi(section.second.get("Group"_jrs), node->name)) {
				msg += section.second.get("Name"_jrs, section.second.getName());
				msg += ", "_jrs;
			}
		}

		if (msg.size() != msgBaseSize)
		{
			msg.truncate(2);
			source->sendMessage(channel, msg);
			haveMods = true;
		}
	}
	if (!haveMods)
		source->sendMessage(channel, "There are no configured moderators."_jrs);
}

const Jupiter::ReadableString &ModListIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays the moderator list. Syntax: modlist");
	return defaultHelp;
}

IRC_COMMAND_INIT(ModListIRCCommand)

/** Game Commands */

// Auth Game Command

void AuthGameCommand::create()
{
	this->addTrigger("auth"_jrs);
	this->setAccessLevel(3);
}

void AuthGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (target == player)
			source->sendMessage(*player, "Error: You can not authenticate yourself."_jrs);
		else
		{
			int cAccess = pluginInstance.getConfigAccess(target->uuid);
			if (cAccess > player->access)
				source->sendMessage(*player, "Error: Can't authenticate higher level moderators."_jrs);
			else if (target->access == cAccess)
				source->sendMessage(*player, "Error: Player is already authenticated"_jrs);
			else if (target->access > cAccess)
				source->sendMessage(*player, "Error: Player is already temporarily authenticated."_jrs);
			else
			{
				RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
				if (pluginInstance.auth(*source, *player) == -1)
					source->sendMessage(*player, "Error: Player failed to pass strict lock checks. Player kicked."_jrs);
				else if (defaultGroup->name == player->varData[pluginInstance.getName()].get("Group"_jrs))
					source->sendMessage(*player, "Error: Failed to authenticate player."_jrs);
				else
					source->sendMessage(*player, "Player authenticated successfully."_jrs);
			}
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: auth <player>"_jrs);
}

const Jupiter::ReadableString &AuthGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Authenticates a player. Syntax: auth <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(AuthGameCommand)

// ATM Game Command

void ATMGameCommand::create()
{
	this->addTrigger("atm"_jrs);
	this->setAccessLevel(3);
}

void ATMGameCommand::trigger(RenX::Server *server, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target;
		RenX_ModSystemPlugin::ModGroup *group = pluginInstance.getDefaultATMGroup();
		Jupiter::ReferenceString playerName = parameters;
		if (isdigit(parameters.get(0)) && parameters.wordCount(WHITESPACE) > 1)
		{
			int index = parameters.asInt();

			if (index < 0 || index >= static_cast<int>(pluginInstance.groups.size()))
				server->sendMessage(*player, "Warning: Invalid group index. Ingoring parameter..."_jrs);
			else if (index == 0)
			{
				server->sendMessage(*player, "Error: Default group is not valid for this command. Use \"deauth\" to deauthorize a player."_jrs);
				return;
			}
			else
			{
				group = pluginInstance.getGroupByIndex(index);
				if (group->access > player->access)
				{
					group = pluginInstance.getDefaultATMGroup();
					server->sendMessage(*player, "Warning: You can not authorize an access level higher than yourself. Ignoring parameter..."_jrs);
				}
				playerName = playerName.gotoWord(1, WHITESPACE);
			}
		}
		if (group != nullptr)
		{
			target = server->getPlayerByPartName(playerName);
			if (target == nullptr)
				server->sendMessage(*player, "Error: Player not found."_jrs);
			else if (target->access > group->access)
				server->sendMessage(*player, "Error: This command can not lower a player's access level."_jrs);
			else
			{
				pluginInstance.tempAuth(*server, *target, group);
				server->sendMessage(*player, "Player successfully temporarily authenticated."_jrs);
			}
		}
		else
			server->sendMessage(*player, "Error: Invalid group."_jrs);
	}
	else
		server->sendMessage(*player, "Error: Too few parameters. Syntax: auth <player>"_jrs);
}

const Jupiter::ReadableString &ATMGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Temporarily authenticates a player. Syntax: atm [level] <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(ATMGameCommand)

// ForceAuth Game Command

void ForceAuthGameCommand::create()
{
	this->addTrigger("fauth"_jrs);
	this->addTrigger("forceauth"_jrs);
	this->setAccessLevel(4);
}

void ForceAuthGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (target == player)
			source->sendMessage(*player, "Error: You can not force-authenticate yourself."_jrs);
		else
		{
			int cAccess = pluginInstance.getConfigAccess(target->uuid);
			if (cAccess > player->access)
				source->sendMessage(*player, "Error: Can't authenticate higher level moderators."_jrs);
			else if (target->access == cAccess)
				source->sendMessage(*player, "Error: Player is already authenticated"_jrs);
			else if (target->access > cAccess)
				source->sendMessage(*player, "Error: Player is already temporarily authenticated."_jrs);
			else
			{
				RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
				pluginInstance.auth(*source, *player, false, true);
				if (defaultGroup->name == player->varData[pluginInstance.getName()].get("Group"_jrs))
					source->sendMessage(*player, "Error: Failed to authenticate player."_jrs);
				else
					source->sendMessage(*player, "Player authenticated successfully."_jrs);
			}
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: fauth <player>"_jrs);
}

const Jupiter::ReadableString &ForceAuthGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Forcefully authenticates a player. Syntax: fauth <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(ForceAuthGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
