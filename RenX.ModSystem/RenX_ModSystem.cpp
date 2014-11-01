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

#include <functional>
#include "Jupiter/IRC_Client.h"
#include "IRC_Bot.h"
#include "RenX_ModSystem.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Core.h"
#include "RenX_Functions.h"
#include "RenX_BanDatabase.h"

void RenX_ModSystemPlugin::init()
{
	RenX_ModSystemPlugin::modsFile.readFile(Jupiter::IRC::Client::Config->get(RenX_ModSystemPlugin::getName(), STRING_LITERAL_AS_REFERENCE("ModsFile"), STRING_LITERAL_AS_REFERENCE("Mods.ini")));

	RenX_ModSystemPlugin::lockSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("LockSteam"), false);
	RenX_ModSystemPlugin::lockIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("LockIP"), false);
	RenX_ModSystemPlugin::kickLockMismatch = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("KickLockMismatch"), true);
	RenX_ModSystemPlugin::autoAuthSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("AutoAuthSteam"), false);
	RenX_ModSystemPlugin::autoAuthIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("AutoAuthIP"), false);
	RenX_ModSystemPlugin::atmDefault = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("ATMDefault"));
	RenX_ModSystemPlugin::moderatorGroup = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("Moderator"), STRING_LITERAL_AS_REFERENCE("Moderator"));
	RenX_ModSystemPlugin::administratorGroup = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("Administrator"), STRING_LITERAL_AS_REFERENCE("Administrator"));

	ModGroup *group;
	Jupiter::ReferenceString dotLockSteam = ".LockSteam";
	Jupiter::ReferenceString dotLockIP = ".LockIP";
	Jupiter::ReferenceString dotKickLockMismatch = ".KickLockMismatch";
	Jupiter::ReferenceString dotAutoAuthSteam = ".AutoAuthSteam";
	Jupiter::ReferenceString dotAutoAuthIP = ".AutoAuthIP";
	Jupiter::ReferenceString dotNext = ".Next";
	Jupiter::ReferenceString dotAccess = ".Access";
	Jupiter::ReferenceString dotPrefix = ".Prefix";
	Jupiter::ReferenceString dotGamePrefix = ".GamePrefix";

	Jupiter::String groupName = RenX_ModSystemPlugin::modsFile.get(Jupiter::ReferenceString::empty, STRING_LITERAL_AS_REFERENCE("Default"));
	while (groupName.isEmpty() == false)
	{
		group = new ModGroup();
		group->name = groupName;

		groupName += dotLockSteam;
		group->lockSteam = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::lockSteam);
		groupName.truncate(dotLockSteam.size());

		groupName += dotLockIP;
		group->lockIP = RenX_ModSystemPlugin::modsFile.getBool(Jupiter::ReferenceString::empty, groupName, RenX_ModSystemPlugin::lockIP);
		groupName.truncate(dotLockIP.size());

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
		if (server->players.size() != 0)
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
	return player->access == oAccess;
}

int RenX_ModSystemPlugin::auth(RenX::Server *server, const RenX::PlayerInfo *player, bool checkAuto, bool forceAuth) const
{
	if (player->isBot)
		return 0;

	ModGroup *group;
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
			server->sendMessage(player, Jupiter::StringS::Format("You are now authenticated with access level %d; group: %.*s.", player->access, group->name.size(), group->name.ptr()));
			Jupiter::String playerName = RenX::getFormattedPlayerName(player);
			server->sendLogChan(IRCCOLOR "03[Authentication] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is now authenticated with access level %d; group: %.*s.", playerName.size(), playerName.ptr(), player->access, group->name.size(), group->name.ptr());
			return player->access;
		};

		if (forceAuth)
			return sectionAuth();

		bool lockSteam_l = section->getBool(STRING_LITERAL_AS_REFERENCE("LockSteam"), group->lockSteam);
		bool lockIP_l = section->getBool(STRING_LITERAL_AS_REFERENCE("LockIP"), group->lockIP);
		bool kickLockMismatch_l = section->getBool(STRING_LITERAL_AS_REFERENCE("KickLockMismatch"), group->kickLockMismatch);
		bool autoAuthSteam_l = section->getBool(STRING_LITERAL_AS_REFERENCE("AutoAuthSteam"), group->autoAuthSteam);
		bool autoAuthIP_l = section->getBool(STRING_LITERAL_AS_REFERENCE("AutoAuthIP"), group->autoAuthIP);

		uint64_t steamid = section->get(STRING_LITERAL_AS_REFERENCE("SteamID")).asUnsignedLongLong();
		const Jupiter::ReadableString &ip = section->get(STRING_LITERAL_AS_REFERENCE("LastIP"));

		if ((lockSteam_l == false || player->steamid == steamid) && (lockIP_l == false || player->ip.equalsi(ip)))
		{
			if (checkAuto == false || (autoAuthSteam_l && player->steamid == steamid) || (autoAuthIP_l && player->ip.equalsi(ip)))
				return sectionAuth();
		}
		else if (kickLockMismatch_l)
		{
			server->kickPlayer(player);
			return -1;
		}
	}
	group = RenX_ModSystemPlugin::groups.get(0);

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
	const Jupiter::ReadableString &group = section->get(STRING_LITERAL_AS_REFERENCE("Group"));
	if (group.isEmpty())
		return section->getInt(STRING_LITERAL_AS_REFERENCE("Access"), RenX_ModSystemPlugin::groups.get(0)->access);
	else
		return section->getInt(STRING_LITERAL_AS_REFERENCE("Access"), RenX_ModSystemPlugin::modsFile.getInt(group, STRING_LITERAL_AS_REFERENCE("Access"), RenX_ModSystemPlugin::groups.get(0)->access));
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
		if (server->players.size() != 0)
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
		}
	}
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
	int access;
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
	if (parameters.isEmpty() == false)
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

// ForceAuth IRC Command

void ForceAuthIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("fauth"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forceauth"));
	this->setAccessLevel(4);
}

void ForceAuthIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() == false)
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

// Ban Search IRC Command

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
	if (parameters.isEmpty() == false)
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
					return entry->varData.get(STRING_LITERAL_AS_REFERENCE("RenX.Commands")).equalsi(params);
				case 5:	// ACTIVE
					return entry->active == params.asBool();
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
					const Jupiter::ReadableString &banner = entry->varData.get(STRING_LITERAL_AS_REFERENCE("RenX.Commands"));
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

/** Game Commands */

// Auth Game Command

void AuthGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("auth"));
	this->setAccessLevel(3);
}

void AuthGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() == false)
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
	if (parameters.isEmpty() == false)
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
	if (parameters.isEmpty() == false)
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
