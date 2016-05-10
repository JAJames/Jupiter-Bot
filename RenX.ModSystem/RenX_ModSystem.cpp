/**
 * Copyright (C) 2014-2016 Jessica James.
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

#include "Jupiter/IRC_Client.h"
#include "IRC_Bot.h"
#include "RenX_ModSystem.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Core.h"
#include "RenX_Functions.h"

using namespace Jupiter::literals;

void RenX_ModSystemPlugin::init()
{
	RenX_ModSystemPlugin::modsFile.readFile(Jupiter::IRC::Client::Config->get(RenX_ModSystemPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ModsFile"), STRING_LITERAL_AS_REFERENCE("Mods.ini")));

	RenX_ModSystemPlugin::lockSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("LockSteam"), false);
	RenX_ModSystemPlugin::lockIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("LockIP"), false);
	RenX_ModSystemPlugin::lockName = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("LockName"), false);
	RenX_ModSystemPlugin::kickLockMismatch = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("KickLockMismatch"), true);
	RenX_ModSystemPlugin::autoAuthSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("AutoAuthSteam"), false);
	RenX_ModSystemPlugin::autoAuthIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("AutoAuthIP"), false);
	RenX_ModSystemPlugin::atmDefault = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("ATMDefault"));
	RenX_ModSystemPlugin::moderatorGroup = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("Moderator"), STRING_LITERAL_AS_REFERENCE("Moderator"));
	RenX_ModSystemPlugin::administratorGroup = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("Administrator"), STRING_LITERAL_AS_REFERENCE("Administrator"));

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

	Jupiter::String groupName = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("Default"));
	while (groupName.isNotEmpty())
	{
		group = new ModGroup();
		group->name = groupName;

		groupName += dotLockSteam;
		group->lockSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::lockSteam);
		groupName.truncate(dotLockSteam.size());

		groupName += dotLockIP;
		group->lockIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::lockIP);
		groupName.truncate(dotLockIP.size());

		groupName += dotLockName;
		group->lockName = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::lockName);
		groupName.truncate(dotLockName.size());

		groupName += dotKickLockMismatch;
		group->kickLockMismatch = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::kickLockMismatch);
		groupName.truncate(dotKickLockMismatch.size());

		groupName += dotAutoAuthSteam;
		group->autoAuthSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::autoAuthSteam);
		groupName.truncate(dotAutoAuthSteam.size());

		groupName += dotAutoAuthIP;
		group->autoAuthIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::autoAuthIP);
		groupName.truncate(dotAutoAuthIP.size());

		groupName += dotAccess;
		group->access = RenX_ModSystemPlugin::modsFile.getInt(Jupiter::ReferenceString::empty, groupName);
		groupName.truncate(dotAccess.size());
		
		groupName += dotPrefix;
		group->prefix = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, groupName);
		groupName.truncate(dotPrefix.size());

		groupName += dotGamePrefix;
		group->gamePrefix = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, groupName);
		groupName.truncate(dotGamePrefix.size());

		RenX_ModSystemPlugin::groups.add(group);

		groupName += dotNext;
		groupName = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, groupName);
	}

	RenX::Core *core = RenX::getCore();
	unsigned int total = core->getServerCount();
	RenX::Server *server;
	while (total != 0)
	{
		server = core->getServer(--total);
		if (server->players.size() != server->getBotCount())
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
				RenX_ModSystemPlugin::auth(server, n->data, true);
	}
}

unsigned int RenX_ModSystemPlugin::logoutAllMods(const RenX::Server *server)
{
	if (server->players.size() == 0)
		return 0;

	unsigned int total = 0;
	for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
		if (RenX_ModSystemPlugin::resetAccess(n->data))
			total++;
	return total;
}

bool RenX_ModSystemPlugin::resetAccess(RenX::PlayerInfo *player)
{
	int oAccess = player->access;
	if (player->adminType.equals("administrator"))
	{
		ModGroup *group = RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::administratorGroup);
		if (group == nullptr)
			player->access = 2;
		else
			player->access = group->access;
	}
	else if (player->adminType.equals("moderator"))
	{
		ModGroup *group = RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::moderatorGroup);
		if (group == nullptr)
			player->access = 1;
		else
			player->access = group->access;
	}
	else if (groups.size() != 0)
		player->access = groups.get(0)->access;
	else
		player->access = 0;
	return player->access != oAccess;
}

int RenX_ModSystemPlugin::auth(RenX::Server *server, const RenX::PlayerInfo *player, bool checkAuto, bool forceAuth) const
{
	if (player->isBot)
		return 0;
	ModGroup *group;
	if (player->uuid.isNotEmpty())
	{
		Jupiter::INIFile::Section *section = RenX_ModSystemPlugin::modsFile.getSection(player->uuid);
		if (section != nullptr)
		{
			const Jupiter::ReadableString &groupName = section->get(STRING_LITERAL_AS_REFERENCE("Group"));
			if (groupName.isEmpty())
				group = RenX_ModSystemPlugin::groups.get(0);
			else if ((group = RenX_ModSystemPlugin::getGroupByName(groupName)) == nullptr)
				group = RenX_ModSystemPlugin::groups.get(0);

			auto sectionAuth = [&]
			{
				player->varData.set(this->name, STRING_LITERAL_AS_REFERENCE("Group"), group->name);
				player->formatNamePrefix = section->get(STRING_LITERAL_AS_REFERENCE("Prefix"), group->prefix);
				player->gamePrefix = section->get(STRING_LITERAL_AS_REFERENCE("GamePrefix"), group->gamePrefix);
				player->access = section->getInt(STRING_LITERAL_AS_REFERENCE("Access"), group->access);
				if (player->access != 0)
				{
					server->sendMessage(player, Jupiter::StringS::Format("You are now authenticated with access level %d; group: %.*s.", player->access, group->name.size(), group->name.ptr()));
					if (server->isDevBot())
					{
						if (server->getVersion() >= 4)
							server->sendData(Jupiter::StringS::Format("xset_dev %d\n", player->id));
						else
							server->sendData(Jupiter::StringS::Format("xset_dev%c%d\n", RenX::DelimC, player->id));
					}
				}
				Jupiter::String playerName = RenX::getFormattedPlayerName(player);
				server->sendLogChan(IRCCOLOR "03[Authentication] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is now authenticated with access level %d; group: %.*s.", playerName.size(), playerName.ptr(), player->access, group->name.size(), group->name.ptr());
				return player->access;
			};

			if (forceAuth)
				return sectionAuth();

			bool lockSteam_l = section->getBool(STRING_LITERAL_AS_REFERENCE("LockSteam"), group->lockSteam);
			bool lockIP_l = section->getBool(STRING_LITERAL_AS_REFERENCE("LockIP"), group->lockIP);
			bool lockName_l = section->getBool(STRING_LITERAL_AS_REFERENCE("LockName"), group->lockName);
			bool kickLockMismatch_l = section->getBool(STRING_LITERAL_AS_REFERENCE("KickLockMismatch"), group->kickLockMismatch);
			bool autoAuthSteam_l = section->getBool(STRING_LITERAL_AS_REFERENCE("AutoAuthSteam"), group->autoAuthSteam);
			bool autoAuthIP_l = section->getBool(STRING_LITERAL_AS_REFERENCE("AutoAuthIP"), group->autoAuthIP);

			uint64_t steamid = section->get(STRING_LITERAL_AS_REFERENCE("SteamID")).asUnsignedLongLong();
			const Jupiter::ReadableString &ip = section->get(STRING_LITERAL_AS_REFERENCE("LastIP"));
			const Jupiter::ReadableString &name = section->get(STRING_LITERAL_AS_REFERENCE("Name"));

			if ((lockSteam_l == false || player->steamid == steamid) && (lockIP_l == false || player->ip.equalsi(ip)) && (lockName_l == false || player->name.equalsi(name)))
			{
				if (checkAuto == false || (autoAuthSteam_l && player->steamid == steamid) || (autoAuthIP_l && player->ip.equalsi(ip)))
					return sectionAuth();
			}
			else if (kickLockMismatch_l)
			{
				server->kickPlayer(player, STRING_LITERAL_AS_REFERENCE("Moderator entry lock mismatch"));
				return -1;
			}
		}
	}
	group = this->getDefaultGroup();

	player->varData.set(this->name, STRING_LITERAL_AS_REFERENCE("Group"), group->name);
	player->formatNamePrefix = group->prefix;
	player->gamePrefix = group->gamePrefix;
	return player->access = group->access;
}

void RenX_ModSystemPlugin::tempAuth(RenX::Server *server, const RenX::PlayerInfo *player, const ModGroup *group, bool notify) const
{
	if (group == nullptr)
		group = this->getDefaultGroup();
	player->varData.set(name, STRING_LITERAL_AS_REFERENCE("Group"), group->name);
	player->formatNamePrefix = group->prefix;
	player->gamePrefix = group->gamePrefix;
	player->access = group->access;
	if (notify)
		server->sendMessage(player, Jupiter::StringS::Format("You have been authorized into group \"%.*s\", with access level %u.", group->name.size(), group->name.ptr(), player->access));
}

bool RenX_ModSystemPlugin::set(RenX::PlayerInfo *player, RenX_ModSystemPlugin::ModGroup *group)
{
	bool r = RenX_ModSystemPlugin::modsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("Group"), group->name);
	RenX_ModSystemPlugin::modsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("SteamID"), Jupiter::StringS::Format("%llu", player->steamid));
	RenX_ModSystemPlugin::modsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("LastIP"), player->ip);
	RenX_ModSystemPlugin::modsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("Name"), player->name);
	RenX_ModSystemPlugin::modsFile.sync();
	return r;
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getGroupByName(const Jupiter::ReadableString &name, ModGroup *defaultGroup) const
{
	if (RenX_ModSystemPlugin::groups.size() != 0)
		for (Jupiter::DLList<ModGroup>::Node *n = groups.getNode(0); n != nullptr; n = n->next)
			if (n->data->name.equalsi(name))
				return n->data;
	return defaultGroup;
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getGroupByAccess(int access, ModGroup *defaultGroup) const
{
	if (RenX_ModSystemPlugin::groups.size() != 0)
		for (Jupiter::DLList<ModGroup>::Node *n = groups.getNode(0); n != nullptr; n = n->next)
			if (n->data->access == access)
				return n->data;
	return defaultGroup;
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getGroupByIndex(size_t index) const
{
	if (RenX_ModSystemPlugin::groups.size() != 0)
		for (Jupiter::DLList<ModGroup>::Node *n = groups.getNode(0); n != nullptr; n = n->next)
			if (index-- == 0)
				return n->data;
	return nullptr;
}

int RenX_ModSystemPlugin::getConfigAccess(const Jupiter::ReadableString &uuid) const
{
	Jupiter::INIFile::Section *section = RenX_ModSystemPlugin::modsFile.getSection(uuid);
	if (section == nullptr)
		return RenX_ModSystemPlugin::groups.get(0)->access;
	RenX_ModSystemPlugin::ModGroup *group = RenX_ModSystemPlugin::getGroupByName(section->get(STRING_LITERAL_AS_REFERENCE("Group")), groups.get(0));
	return section->getInt(STRING_LITERAL_AS_REFERENCE("Access"), group->access);
}

size_t RenX_ModSystemPlugin::getGroupCount() const
{
	return RenX_ModSystemPlugin::groups.size();
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getDefaultGroup() const
{
	return RenX_ModSystemPlugin::groups.get(0);
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getDefaultATMGroup() const
{
	return RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::atmDefault);
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getModeratorGroup() const
{
	return RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::moderatorGroup);
}

RenX_ModSystemPlugin::ModGroup *RenX_ModSystemPlugin::getAdministratorGroup() const
{
	return RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::administratorGroup);
}

RenX_ModSystemPlugin::RenX_ModSystemPlugin()
{
	RenX_ModSystemPlugin::init();
}

RenX_ModSystemPlugin::~RenX_ModSystemPlugin()
{
	RenX::Core *core = RenX::getCore();
	unsigned int total = core->getServerCount();
	RenX::Server *server;
	RenX::PlayerInfo *player;
	while (total != 0)
	{
		server = core->getServer(--total);
		if (server->players.size() != server->getBotCount())
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				if (player->isBot == false)
				{
					player->varData.remove(RenX_ModSystemPlugin::name, STRING_LITERAL_AS_REFERENCE("Group"));
					player->gamePrefix.truncate(player->gamePrefix.size());
					player->formatNamePrefix.truncate(player->formatNamePrefix.size());
					if (player->adminType.equals("administrator"))
						player->access = 2;
					else if (player->adminType.equals("moderator"))
						player->access = 1;
					else
						player->access = 0;
				}
			}
	}
	while (RenX_ModSystemPlugin::groups.size() != 0)
		delete RenX_ModSystemPlugin::groups.remove(0U);
}

void RenX_ModSystemPlugin::RenX_OnPlayerCreate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->isBot == false)
		RenX_ModSystemPlugin::auth(server, player, true);
}

void RenX_ModSystemPlugin::RenX_OnPlayerDelete(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (RenX_ModSystemPlugin::groups.size() != 0 && player->isBot == false)
	{
		Jupiter::INIFile::Section *section = RenX_ModSystemPlugin::modsFile.getSection(player->uuid);
		if (section != nullptr)
		{
			section->set(STRING_LITERAL_AS_REFERENCE("SteamID"), Jupiter::StringS::Format("%llu", player->steamid));
			section->set(STRING_LITERAL_AS_REFERENCE("LastIP"), player->ip);
			section->set(STRING_LITERAL_AS_REFERENCE("Name"), player->name);
		}
	}
}

void RenX_ModSystemPlugin::RenX_OnIDChange(RenX::Server *server, const RenX::PlayerInfo *player, int oldID)
{
	if (player->access != 0 && server->isDevBot())
		server->sendData(Jupiter::StringS::Format("d%d\n", player->id));
}

void RenX_ModSystemPlugin::RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	ModGroup *group = nullptr;
	if (player->adminType.equals("administrator"))
		group = RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::administratorGroup);
	else if (player->adminType.equals("moderator"))
		group = RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::moderatorGroup);

	if (group != nullptr && player->access < group->access)
		player->access = group->access;
}

void RenX_ModSystemPlugin::RenX_OnAdminGrant(RenX::Server *server, const RenX::PlayerInfo *player)
{
	RenX_ModSystemPlugin::RenX_OnAdminLogin(server, player);
}

void RenX_ModSystemPlugin::RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player)
{
	ModGroup *group = nullptr;
	int access = RenX_ModSystemPlugin::groups.size() == 0 ? 0 : RenX_ModSystemPlugin::groups.get(0)->access;
	if (player->adminType.equals("administrator"))
	{
		access = 2;
		group = RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::administratorGroup);
	}
	else if (player->adminType.equals("moderator"))
	{
		access = 1;
		group = RenX_ModSystemPlugin::getGroupByName(RenX_ModSystemPlugin::moderatorGroup);
	}
	if (group != nullptr)
		access = group->access;

	if (player->access <= access)
	{
		if (RenX_ModSystemPlugin::groups.size() == 0)
			player->access = 0;
		else
			player->access = RenX_ModSystemPlugin::groups.get(0)->access;
	}
}

int RenX_ModSystemPlugin::OnRehash()
{
	RenX_ModSystemPlugin::modsFile.flushData();
	while (RenX_ModSystemPlugin::groups.size() != 0)
		delete RenX_ModSystemPlugin::groups.remove(0U);
	RenX_ModSystemPlugin::init();
	return 0;
}

// Plugin instantiation and entry point.
RenX_ModSystemPlugin pluginInstance;

/** IRC Commands */

// Auth IRC Command

void AuthIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("auth"));
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
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
					else
					{
						int uAccess = source->getAccessLevel(channel, nick);
						int cAccess = pluginInstance.getConfigAccess(player->uuid);
						if (cAccess > uAccess && uAccess < static_cast<int>(source->getPrefixes().size()))
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Can't authenticate higher level moderators."));
						else if (player->access == cAccess)
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player is already authenticated"));
						else if (player->access > cAccess)
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player is already temporarily authenticated."));
						else
						{
							RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
							if (pluginInstance.auth(server, player) == -1)
								source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player failed to pass strict lock checks. Player kicked."));
							else if (defaultGroup->name.equals(player->varData.get(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Group"))))
								source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Failed to authenticate player."));
							else
								source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player authenticated successfully."));
						}
					}
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unauth"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deauth"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("demod"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("dtm"));
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
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
					else
					{
						int uAccess = source->getAccessLevel(channel, nick);
						int cAccess = pluginInstance.getConfigAccess(player->uuid);
						if (cAccess > uAccess && uAccess < static_cast<int>(source->getPrefixes().size()))
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Can't unauthenticate higher level moderators."));
						else if (pluginInstance.resetAccess(player))
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player unauthenticated successfully."));
						else
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not authenticated."));
					}
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("atm"));
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
					source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Warning: Invalid group index. Ingoring parameter..."));
				else if (index == 0)
				{
					source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Default group is not valid for this command. Use \"deauth\" to deauthorize a player."));
					return;
				}
				else
				{
					group = pluginInstance.groups.get(index);
					if (group->access > source->getAccessLevel(channel, nick))
					{
						group = pluginInstance.getDefaultATMGroup();
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Warning: You can not authorize an access level higher than yourself. Ignoring parameter..."));
					}
					playerName = playerName.gotoWord(1, WHITESPACE);
					if (playerName.isEmpty())
						playerName = nick;
				}
			}
			if (group == nullptr)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid group."));
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
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
						else if (player->access > group->access)
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: This command can not lower a player's access level."));
						else
						{
							pluginInstance.tempAuth(server, player, group);
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player successfully temporarily authenticated."));
						}
					}
				}
				if (serverMatch == false)
					source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addmod"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("add"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("set"));
	this->setAccessLevel(5);
}

void AddIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) < 2)
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: add <level> <player>"));
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
					source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid group index."));
				else
				{
					group = pluginInstance.groups.get(index);
					playerName = playerName.gotoWord(1, WHITESPACE);
				}
			}
			if (group == nullptr)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid group."));
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
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
						else if (player->isBot)
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: A bot can not be a moderator."));
						else if (player->uuid.isEmpty())
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player has no UUID."));
						else
						{
							pluginInstance.resetAccess(player);
							if (pluginInstance.set(player, group))
								source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been added to group \"%.*s\"", player->name.size(), player->name.ptr(), group->name.size(), group->name.ptr()));
							else
								source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been moved to group \"%.*s\"", player->name.size(), player->name.ptr(), group->name.size(), group->name.ptr()));
							pluginInstance.auth(server, player, false, true);
						}
					}
				}
				if (serverMatch == false)
					source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("delmod"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("remmod"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("del"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rem"));
	this->setAccessLevel(5);
}

void DelIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: del <player>"));
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
						if (pluginInstance.modsFile.remove(parameters))
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player has been removed from the moderator list."));
						else
						{
							size_t index = pluginInstance.modsFile.getSections();
							Jupiter::INIFile::Section *section;

							DelIRCCommand_trigger_loop:
							if (index != 0)
							{
								section = pluginInstance.modsFile.getSection(--index);
								if (section->get(STRING_LITERAL_AS_REFERENCE("Name")).equalsi(parameters))
								{
									if (pluginInstance.modsFile.remove(index))
										source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player has been removed from the moderator list."));
									else
										source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Unknown error occurred."));
								}
								else
									goto DelIRCCommand_trigger_loop;
							}
							else
								source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
						}
					}
					else if (player->isBot)
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: A bot can not be a moderator."));
					else if (pluginInstance.modsFile.remove(player->uuid))
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player has been removed from the moderator list."));
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player is not in the moderator list."));
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("fauth"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forceauth"));
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
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
					else
					{
						int uAccess = source->getAccessLevel(channel, nick);
						int cAccess = pluginInstance.getConfigAccess(player->uuid);
						if (cAccess > uAccess && uAccess < static_cast<int>(source->getPrefixes().size()))
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Can't authenticate higher level moderators."));
						else if (player->access == cAccess)
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player is already authenticated"));
						else if (player->access > cAccess)
							source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player is already temporarily authenticated."));
						else
						{
							RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
							pluginInstance.auth(server, player, false, true);
							if (defaultGroup->name.equals(player->varData.get(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Group"))))
								source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Failed to authenticate player."));
							else
								source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Player authenticated successfully."));
						}
					}
				}
			}
			if (serverMatch == false)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("modlist"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mlist"));
}

void ModListIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	RenX_ModSystemPlugin::ModGroup *group;
	Jupiter::INIFile::Section *section;
	size_t i;
	Jupiter::String msg;
	size_t msgBaseSize;
	bool haveMods = false;
	for (Jupiter::DLList<RenX_ModSystemPlugin::ModGroup>::Node *n = pluginInstance.groups.getNode(0); n != nullptr; n = n->next)
	{
		group = n->data;
		msg = group->prefix;
		msg += group->name;
		msg.aformat(IRCNORMAL " (Access: %d): ", group->access);
		msgBaseSize = msg.size();
		i = pluginInstance.modsFile.getSections();
		while (i != 0)
		{
			section = pluginInstance.modsFile.getSection(--i);
			if (section->get(STRING_LITERAL_AS_REFERENCE("Group")).equalsi(group->name))
			{
				msg += section->get(STRING_LITERAL_AS_REFERENCE("Name"), section->getName());
				msg += STRING_LITERAL_AS_REFERENCE(", ");
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
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("There are no configured moderators."));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("auth"));
	this->setAccessLevel(3);
}

void AuthGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (target == player)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not authenticate yourself."));
		else
		{
			int cAccess = pluginInstance.getConfigAccess(target->uuid);
			if (cAccess > player->access)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Can't authenticate higher level moderators."));
			else if (target->access == cAccess)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is already authenticated"));
			else if (target->access > cAccess)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is already temporarily authenticated."));
			else
			{
				RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
				if (pluginInstance.auth(source, player) == -1)
					source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player failed to pass strict lock checks. Player kicked."));
				else if (defaultGroup->name.equals(player->varData.get(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Group"))))
					source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Failed to authenticate player."));
				else
					source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player authenticated successfully."));
			}
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: auth <player>"));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("atm"));
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
				server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Warning: Invalid group index. Ingoring parameter..."));
			else if (index == 0)
			{
				server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Default group is not valid for this command. Use \"deauth\" to deauthorize a player."));
				return;
			}
			else
			{
				group = pluginInstance.groups.get(index);
				if (group->access > player->access)
				{
					group = pluginInstance.getDefaultATMGroup();
					server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Warning: You can not authorize an access level higher than yourself. Ignoring parameter..."));
				}
				playerName = playerName.gotoWord(1, WHITESPACE);
			}
		}
		if (group != nullptr)
		{
			target = server->getPlayerByPartName(playerName);
			if (target == nullptr)
				server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
			else if (target->access > group->access)
				server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: This command can not lower a player's access level."));
			else
			{
				pluginInstance.tempAuth(server, target, group);
				server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player successfully temporarily authenticated."));
			}
		}
		else
			server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Invalid group."));
	}
	else
		server->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: auth <player>"));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("fauth"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forceauth"));
	this->setAccessLevel(4);
}

void ForceAuthGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		else if (target == player)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You can not force-authenticate yourself."));
		else
		{
			int cAccess = pluginInstance.getConfigAccess(target->uuid);
			if (cAccess > player->access)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Can't authenticate higher level moderators."));
			else if (target->access == cAccess)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is already authenticated"));
			else if (target->access > cAccess)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is already temporarily authenticated."));
			else
			{
				RenX_ModSystemPlugin::ModGroup *defaultGroup = pluginInstance.getDefaultGroup();
				pluginInstance.auth(source, player, false, true);
				if (defaultGroup->name.equals(player->varData.get(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Group"))))
					source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Failed to authenticate player."));
				else
					source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Player authenticated successfully."));
			}
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: fauth <player>"));
}

const Jupiter::ReadableString &ForceAuthGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Forcefully authenticates a player. Syntax: fauth <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(ForceAuthGameCommand)

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
