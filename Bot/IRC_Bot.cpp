/**
 * Copyright (C) 2013-2015 Justin James.
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
 * Written by Justin James <justin.aj@hotmail.com>
 */

/**
 * Long term plan:
 * Eventually, this file and its header will no longer be a part of this project.
 * The features made available by this core will slowly be transferred away in pieces
 * to other locations, such as the internal Jupiter library, or various plugins.
 *
 * There is a relatively thin line between what will go in Jupiter, and what will be
 * pushed to plugins.
 * Example: The "Command" and "Plugin" classes ended up in Jupiter; they're abstract concepts
 * that doesn't put a specific behavior on any mechanisms. The "IRCCommand" class, however,
 * alters the behavior of the IRC Client, and thus should be contained within a plugin.
 */

#include <cstdio>
#include <cstring>
#include <cctype>
#include "Jupiter/INIFile.h"
#include "Jupiter/Plugin.h"
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "IRC_Command.h"

IRC_Bot::IRC_Bot(const Jupiter::ReadableString &configSection) : Client(configSection)
{
	IRC_Bot::commandPrefix = this->readConfigValue(STRING_LITERAL_AS_REFERENCE("Prefix"));
	for (size_t i = 0; i != IRCMasterCommandList->size(); i++)
		IRC_Bot::addCommand(IRCMasterCommandList->get(i)->copy());
	IRC_Bot::setCommandAccessLevels();
}

IRC_Bot::~IRC_Bot()
{
	if (IRCCommand::selected_server == this)
		IRCCommand::selected_server = nullptr;
	if (IRCCommand::active_server == this)
		IRCCommand::active_server = IRCCommand::selected_server;

	IRC_Bot::commands.emptyAndDelete();
}

void IRC_Bot::addCommand(IRCCommand *cmd)
{
	IRC_Bot::commands.add(cmd);
}

bool IRC_Bot::freeCommand(const Jupiter::ReadableString &trigger)
{
	for (size_t i = 0; i != IRC_Bot::commands.size(); i++)
	{
		if (IRC_Bot::commands.get(i)->matches(trigger))
		{
			delete IRC_Bot::commands.remove(i);
			return true;
		}
	}
	return false;
}

int IRC_Bot::getCommandIndex(const Jupiter::ReadableString &trigger) const
{
	for (size_t i = 0; i != IRC_Bot::commands.size(); i++)
		if (IRC_Bot::commands.get(i)->matches(trigger))
			return i;
	return -1;
}

IRCCommand *IRC_Bot::getCommand(const Jupiter::ReadableString &trigger) const
{
	int i = IRC_Bot::getCommandIndex(trigger);
	if (i < 0) return nullptr;
	return IRC_Bot::commands.get(i);
}

Jupiter::ArrayList<IRCCommand> IRC_Bot::getAccessCommands(Jupiter::IRC::Client::Channel *chan, int access)
{
	Jupiter::ArrayList<IRCCommand> r;
	for (size_t i = 0; i != IRC_Bot::commands.size(); i++)
	{
		IRCCommand *cmd = IRC_Bot::commands.get(i);
		if (cmd->getAccessLevel(chan) == access)
			r.add(cmd);
	}
	return r;
}

Jupiter::StringL IRC_Bot::getTriggers(Jupiter::ArrayList<IRCCommand> cmds)
{
	Jupiter::StringL r;
	for (size_t i = 0; i < cmds.size(); i++)
	{
		r += cmds[i]->getTrigger();
		r += ' ';
	}
	return r;
}

void IRC_Bot::setCommandAccessLevels()
{
	// Rewrite this later
	// Note: Prepare for abstraction of configuration file type.
	Jupiter::StringS commandSection = "DefaultCommands";

	checkForCommands:
	int sIndex = Config->getSectionIndex(commandSection);
	if (sIndex >= 0)
	{
		int sLen = Config->getSectionLength(sIndex);
		for (int i = 0; i < sLen; i++)
		{
			const Jupiter::ReadableString &key = Config->getKey(commandSection, i);
			int pos = key.find('.');
			if (pos != Jupiter::INVALID_INDEX)
			{
				Jupiter::ReferenceString command = Jupiter::ReferenceString(key.ptr(), pos);
				if (command != nullptr)
				{
					IRCCommand *cmd = IRC_Bot::getCommand(command);
					if (cmd != nullptr)
					{
						Jupiter::ReferenceString channelType(key.ptr() + pos + 1, key.size() - pos - 1);
						if (isdigit(key[pos + 1]))
							cmd->setAccessLevel(channelType.asInt(10), Config->getInt(commandSection, key));
						else cmd->setAccessLevel(channelType.asInt(10), Config->getInt(commandSection, key));
					}
					else if (this->getPrintOutput()) printf("Unable to find command \"%.*s\"" ENDL, key.size(), key.ptr());
				}
			}
			else
			{
				IRCCommand *cmd = IRC_Bot::getCommand(key);
				if (cmd != nullptr) cmd->setAccessLevel(Config->getInt(commandSection, key));
				else if (this->getPrintOutput()) printf("Unable to find command \"%.*s\"" ENDL, key.size(), key.ptr());
			}
		}
	}
	if (commandSection.equals("DefaultCommands"))
	{
		commandSection = this->getConfigSection();
		commandSection += "Commands";
		if (commandSection.equals("DefaultCommands") == false) goto checkForCommands;
	}
}

int IRC_Bot::OnRehash()
{
	if (Config->reload() == Jupiter::ERROR_INDICATOR) return 1;
	IRC_Bot::setCommandAccessLevels();
	return 0;
}

void IRC_Bot::OnChat(const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message)
{
	int chanIndex = this->getChannelIndex(channel);
	if (chanIndex >= 0)
	{
		Channel *chan = this->getChannel(chanIndex);
		if (chan->getType() >= 0)
		{
			Jupiter::ReferenceString msg = message;
			while (msg.isNotEmpty() && isspace(msg[0]))
				msg.shiftRight(1);

			if (IRC_Bot::commandPrefix.size() <= msg.size())
			{
				bool matchesPrefix = true;
				size_t i;
				for (i = 0; i != IRC_Bot::commandPrefix.size(); i++)
				{
					if (toupper(msg.get(0)) != toupper(IRC_Bot::commandPrefix[i]))
					{
						matchesPrefix = false;
						break;
					}
					msg.shiftRight(1);
				}

				if (matchesPrefix)
				{
					Jupiter::ReferenceString command = Jupiter::ReferenceString::getWord(msg, 0, WHITESPACE);;
					Jupiter::ReferenceString parameters = Jupiter::ReferenceString::gotoWord(msg, 1, WHITESPACE);
					IRCCommand *cmd = IRC_Bot::getCommand(command);
					if (cmd != nullptr)
					{
						IRCCommand::active_server = this;
						int cAccess = cmd->getAccessLevel(chan);
						if (cAccess >= 0 && Jupiter::IRC::Client::getAccessLevel(channel, nick) >= cAccess)
							cmd->trigger(this, channel, nick, parameters);
						else
							this->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Access Denied."));
						IRCCommand::active_server = IRCCommand::selected_server;
					}
				}
			}
		}
	}
}