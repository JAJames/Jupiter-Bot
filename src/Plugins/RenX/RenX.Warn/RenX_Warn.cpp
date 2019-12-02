/**
 * Copyright (C) 2014-2017 Jessica James.
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

#include "IRC_Bot.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Warn.h"

using namespace Jupiter::literals;

bool RenX_WarnPlugin::initialize()
{
	RenX_WarnPlugin::maxWarns = this->config.get<int>("MaxWarns"_jrs, 3);
	RenX_WarnPlugin::warnAction = this->config.get<int>("MaxAction"_jrs, -1);
	return true;
}

int RenX_WarnPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();
	return this->initialize() ? 0 : -1;
}

// Plugin instantiation and entry point.
RenX_WarnPlugin pluginInstance;

STRING_LITERAL_AS_NAMED_REFERENCE(WARNS_KEY, "w");

// Warn IRC Command

void WarnIRCCommand::create()
{
	this->addTrigger("warn"_jrs);
	this->addTrigger("w"_jrs);
	this->setAccessLevel(2);
}

void WarnIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) >= 2)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			Jupiter::ArrayList<RenX::Server> servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				Jupiter::ReferenceString name = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
				Jupiter::ReferenceString reason = Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);

				RenX::PlayerInfo *player;
				RenX::Server *server;
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(name);
						if (player != nullptr)
						{
							int warns = player->varData[pluginInstance.getName()].get<int>(WARNS_KEY) + 1;
							if (warns > pluginInstance.maxWarns)
							{
								switch (pluginInstance.warnAction)
								{
								case -1:
									server->kickPlayer(*player, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns));
									source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.ptr(), warns));
									break;
								default:
									server->banPlayer(*player, "Jupiter Bot/RenX.Warn"_jrs, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns), std::chrono::seconds(pluginInstance.warnAction));
									source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.ptr(), reason.size(), reason.ptr(), warns));
									break;
								}
							}
							else
							{
								player->varData[pluginInstance.getName()].set(WARNS_KEY, Jupiter::StringS::Format("%d", warns));
								server->sendWarnMessage(*player, Jupiter::StringS::Format("You have been warned by %.*s@IRC for: %.*s. You have %d warnings.", nick.size(), nick.ptr(), reason.size(), reason.ptr(), warns));
								source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been warned; they now have %d warnings.", player->name.size(), player->name.ptr(), warns));
							}
						}
					}
				}
			}
			else
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
	else
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: Warn <Player> <Reason>"_jrs);
}

const Jupiter::ReadableString &WarnIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Warns a player. Syntax: Warn <Player> <Reason>");
	return defaultHelp;
}

IRC_COMMAND_INIT(WarnIRCCommand)

// Pardon IRC Command

void PardonIRCCommand::create()
{
	this->addTrigger("pardon"_jrs);
	this->addTrigger("forgive"_jrs);
	this->addTrigger("unwarn"_jrs);
	this->setAccessLevel(2);
}

void PardonIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
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
				for (size_t i = 0; i != servers.size(); i++)
				{
					server = servers.get(i);
					if (server != nullptr)
					{
						player = server->getPlayerByPartName(parameters);
						if (player != nullptr)
						{
							player->varData[pluginInstance.getName()].remove(WARNS_KEY);
							server->sendMessage(*player, Jupiter::StringS::Format("You have been pardoned by %.*s@IRC; your warnings have been reset.", nick.size(), nick.ptr()));
							source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been pardoned; their warnings have been reset.", player->name.size(), player->name.ptr()));
						}
					}
				}
			}
			else
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
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
	this->addTrigger("warn"_jrs);
	this->addTrigger("w"_jrs);
	this->setAccessLevel(1);
}

void WarnGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) >= 2)
	{
		Jupiter::ReferenceString name = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
		RenX::PlayerInfo *target = source->getPlayerByPartName(name);
		if (target != nullptr)
		{
			Jupiter::ReferenceString reason = Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);
			int warns = target->varData[pluginInstance.getName()].get<int>(WARNS_KEY) + 1;
			if (warns > pluginInstance.maxWarns)
			{
				switch (pluginInstance.warnAction)
				{
				case -1:
					source->kickPlayer(*target, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns));
					source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.ptr(), warns));
					break;
				default:
					source->banPlayer(*target, "Jupiter Bot/RenX.Warn"_jrs, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns), std::chrono::seconds(pluginInstance.warnAction));
					source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.ptr(), warns));
					break;
				}
			}
			else
			{
				target->varData[pluginInstance.getName()].set(WARNS_KEY, Jupiter::StringS::Format("%d", warns));
				source->sendWarnMessage(*target, Jupiter::StringS::Format("You have been warned by %.*s for: %.*s. You have %d warnings.", player->name.size(), player->name.ptr(), reason.size(), reason.ptr(), warns));
				source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been warned; they now have %d warnings.", target->name.size(), target->name.ptr(), warns));
			}
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: Warn <Player> <Reason>"_jrs);
}

const Jupiter::ReadableString &WarnGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Warns a player. Syntax: Warn <Player> <Reason>");
	return defaultHelp;
}

GAME_COMMAND_INIT(WarnGameCommand)

// Pardon Game Command

void PardonGameCommand::create()
{
	this->addTrigger("pardon"_jrs);
	this->addTrigger("forgive"_jrs);
	this->addTrigger("unwarn"_jrs);
	this->setAccessLevel(1);
}

void PardonGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target != nullptr)
		{
			target->varData[pluginInstance.getName()].remove(WARNS_KEY);
			source->sendMessage(*target, Jupiter::StringS::Format("You have been pardoned by %.*s@IRC; your warnings have been reset.", player->name.size(), player->name.ptr()));
			source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been pardoned; their warnings have been reset.", target->name.size(), target->name.ptr()));
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

GAME_COMMAND_INIT(PardonGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
