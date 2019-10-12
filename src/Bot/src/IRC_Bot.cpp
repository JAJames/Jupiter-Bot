/**
 * Copyright (C) 2013-2017 Jessica James.
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

#include <cstdio>
#include <cstring>
#include <cctype>
#include "Jupiter/Config.h"
#include "Jupiter/Plugin.h"
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "IRC_Command.h"

using namespace Jupiter::literals;

IRC_Bot::IRC_Bot(Jupiter::Config *in_primary_section, Jupiter::Config *in_secondary_section) : Client(in_primary_section, in_secondary_section)
{
	IRC_Bot::commandPrefix = this->readConfigValue("Prefix"_jrs);
	
	for (size_t i = 0; i != IRCMasterCommandList->size(); i++)
		IRC_Bot::commands.add(IRCMasterCommandList->get(i)->copy());

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

void IRC_Bot::addCommand(IRCCommand *in_command)
{
	IRC_Bot::commands.add(in_command);
	IRC_Bot::setCommandAccessLevels(in_command);
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

Jupiter::StringL IRC_Bot::getTriggers(Jupiter::ArrayList<IRCCommand> &cmds)
{
	Jupiter::StringL r;
	for (size_t i = 0; i < cmds.size(); i++)
	{
		r += cmds[i]->getTrigger();
		r += ' ';
	}
	return r;
}

void IRC_Bot::setCommandAccessLevels(IRCCommand *in_command)
{
	auto set_command_access_levels = [this, in_command](Jupiter::Config *in_section)
	{
		if (in_section == nullptr)
			return;

		Jupiter::Config *section = in_section->getSection("Commands"_jrs);

		if (section == nullptr)
			return;

		for (auto& entry : section->getTable()) {
			size_t tmp_index;
			Jupiter::ReferenceString tmp_key, tmp_sub_key;
			IRCCommand *command;

			tmp_index = entry.first.find('.');
			if (tmp_index != Jupiter::INVALID_INDEX)
			{
				// non-default access assignment

				tmp_key.set(entry.first.ptr(), tmp_index);

				tmp_sub_key = entry.first;
				tmp_sub_key.shiftRight(tmp_index + 1);

				if (tmp_sub_key.findi("Type."_jrs) == 0)
				{
					tmp_sub_key.shiftRight(5); // shift beyond "Type."

					command = this->getCommand(tmp_key);
					if (command != nullptr && (in_command == nullptr || in_command == command))
						command->setAccessLevel(tmp_sub_key.asInt(), entry.second.asInt());
				}
				else if (tmp_sub_key.findi("Channel."_jrs) == 0)
				{
					tmp_sub_key.shiftRight(8); // shift beyond "Channel."

					// Assign access level to command (if command exists)
					command = this->getCommand(tmp_key);
					if (command != nullptr && (in_command == nullptr || in_command == command))
						command->setAccessLevel(tmp_sub_key, entry.second.asInt());
				}
			}
			else
			{
				// Assign access level to command (if command exists)
				command = this->getCommand(entry.first);
				if (command != nullptr && (in_command == nullptr || in_command == command))
					command->setAccessLevel(entry.second.asInt());
			}
		};
	};

	set_command_access_levels(this->getSecondaryConfigSection());
	set_command_access_levels(this->getPrimaryConfigSection());
}

void IRC_Bot::OnChat(const Jupiter::ReadableString &in_channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message)
{
	Channel *channel = this->getChannel(in_channel);
	if (channel != nullptr && channel->getType() >= 0)
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
					int command_access = cmd->getAccessLevel(channel);
					if (command_access < 0)
						this->sendNotice(nick, "Error: Command disabled."_jrs);
					else if (Jupiter::IRC::Client::getAccessLevel(*channel, nick) < command_access)
						this->sendNotice(nick, "Access Denied."_jrs);
					else
						cmd->trigger(this, in_channel, nick, parameters);
					IRCCommand::active_server = IRCCommand::selected_server;
				}
			}
		}
	}
}