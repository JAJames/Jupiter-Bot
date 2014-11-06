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

#include "IRC_Bot.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Warn.h"

int RenX_WarnPlugin::OnRehash()
{
	RenX_WarnPlugin::maxWarns = Jupiter::IRC::Client::Config->getInt(RenX_WarnPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MaxWarns"), 3);
	RenX_WarnPlugin::warnAction = Jupiter::IRC::Client::Config->getInt(RenX_WarnPlugin::getName(), STRING_LITERAL_AS_REFERENCE("MaxAction"), -1);
	return 0;
}

RenX_WarnPlugin::RenX_WarnPlugin()
{
	this->OnRehash();
}

// Plugin instantiation and entry point.
RenX_WarnPlugin pluginInstance;

STRING_LITERAL_AS_NAMED_REFERENCE(WARNS_KEY, "w");

// Warn IRC Command

void WarnIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("warn"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("w"));
	this->setAccessLevel(2);
}

void WarnIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() == false)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				RenX::PlayerInfo *player;
				RenX::Server *server;
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(parameters);
						if (player != nullptr)
						{
							int warns = player->varData.getInt(pluginInstance.getName(), WARNS_KEY) + 1;
							if (warns > pluginInstance.maxWarns)
							{
								switch (pluginInstance.warnAction)
								{
								case -1:
									server->kickPlayer(player);
									source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.ptr(), warns));
									break;
								default:
									server->banPlayer(player, pluginInstance.warnAction);
									source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.ptr(), warns));
									break;
								}
							}
							else
							{
								player->varData.set(pluginInstance.getName(), WARNS_KEY, Jupiter::StringS::Format("%d", warns));
								server->sendMessage(player, Jupiter::StringS::Format("You have been warned by %.*s@IRC; improve your behavior, or you will be disciplined. You have %d warnings.", nick.size(), nick.ptr(), warns));
								source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been warned; they now have %d warnings.", player->name.size(), player->name.ptr(), warns));
							}
						}
					}
				}
			}
			else
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Warn <Player>"));
}

const Jupiter::ReadableString &WarnIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Warns a player. Syntax: Warn <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(WarnIRCCommand)

// Pardon IRC Command

void PardonIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pardon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forgive"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unwarn"));
	this->setAccessLevel(2);
}

void PardonIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() == false)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				RenX::PlayerInfo *player;
				RenX::Server *server;
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(parameters);
						if (player != nullptr)
						{
							player->varData.remove(pluginInstance.getName(), WARNS_KEY);
							server->sendMessage(player, Jupiter::StringS::Format("You have been pardoned by %.*s@IRC; your warnings have been reset.", nick.size(), nick.ptr()));
							source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been pardoned; their warnings have been reset.", player->name.size(), player->name.ptr()));
						}
					}
				}
			}
			else
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else
		this->trigger(source, channel, nick, nick);
}

const Jupiter::ReadableString &PardonIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's warnings. Syntax: Pardon <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PardonIRCCommand)

// Warn Game Command

void WarnGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("warn"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("w"));
	this->setAccessLevel(1);
}

void WarnGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() == false)
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target != nullptr)
		{
			int warns = target->varData.getInt(pluginInstance.getName(), WARNS_KEY) + 1;
			if (warns > pluginInstance.maxWarns)
			{
				switch (pluginInstance.warnAction)
				{
				case -1:
					source->kickPlayer(target);
					source->sendMessage(player, Jupiter::StringS::Format("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.ptr(), warns));
					break;
				default:
					source->banPlayer(target, pluginInstance.warnAction);
					source->sendMessage(player, Jupiter::StringS::Format("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.ptr(), warns));
					break;
				}
			}
			else
			{
				target->varData.set(pluginInstance.getName(), WARNS_KEY, Jupiter::StringS::Format("%d", warns));
				source->sendMessage(target, Jupiter::StringS::Format("You have been warned by %.*s; improve your behavior, or you will be disciplined. You have %d warnings.", player->name.size(), player->name.ptr(), warns));
				source->sendMessage(player, Jupiter::StringS::Format("%.*s has been warned; they now have %d warnings.", target->name.size(), target->name.ptr(), warns));
			}
		}
	}
	else
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: Warn <Player>"));
}

const Jupiter::ReadableString &WarnGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Warns a player. Syntax: Warn <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(WarnGameCommand)

// Pardon Game Command

void PardonGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pardon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forgive"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unwarn"));
	this->setAccessLevel(1);
}

void PardonGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() == false)
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target != nullptr)
		{
			target->varData.remove(pluginInstance.getName(), WARNS_KEY);
			source->sendMessage(target, Jupiter::StringS::Format("You have been pardoned by %.*s@IRC; your warnings have been reset.", player->name.size(), player->name.ptr()));
			source->sendMessage(player, Jupiter::StringS::Format("%.*s has been pardoned; their warnings have been reset.", target->name.size(), target->name.ptr()));
		}
	}
	else
		this->trigger(source, player, player->name);
}

const Jupiter::ReadableString &PardonGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's warnings. Syntax: Pardon <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PardonGameCommand)

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
