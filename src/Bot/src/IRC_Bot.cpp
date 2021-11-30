/**
 * Copyright (C) 2013-2021 Jessica James.
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
#include "jessilib/unicode.hpp"
#include "jessilib/word_split.hpp"
#include "Jupiter/Config.h"
#include "Jupiter/Plugin.h"
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "IRC_Command.h"

using namespace Jupiter::literals;
using namespace std::literals;

IRC_Bot::IRC_Bot(Jupiter::Config *in_primary_section, Jupiter::Config *in_secondary_section)
	: Client(in_primary_section, in_secondary_section) {
	m_commandPrefix = this->readConfigValue("Prefix"_jrs);
	
	for (const auto& command : IRCMasterCommandList) {
		m_commands.emplace_back(command->copy());
	}

	setCommandAccessLevels();
}

IRC_Bot::~IRC_Bot() {
	if (IRCCommand::selected_server == this) {
		IRCCommand::selected_server = nullptr;
	}

	if (IRCCommand::active_server == this) {
		IRCCommand::active_server = IRCCommand::selected_server;
	}
}

void IRC_Bot::addCommand(IRCCommand *in_command) {
	m_commands.emplace_back(in_command);
	setCommandAccessLevels(in_command);
}

bool IRC_Bot::freeCommand(std::string_view trigger) {
	for (auto itr = m_commands.begin(); itr != m_commands.end(); ++itr) {
		if ((*itr)->matches(trigger)) {
			m_commands.erase(itr);
			return true;
		}
	}

	return false;
}

IRCCommand* IRC_Bot::getCommand(std::string_view trigger) const {
	for (const auto& command : m_commands) {
		if (command->matches(trigger)) {
			return command.get();
		}
	}

	return nullptr;
}

std::vector<IRCCommand*> IRC_Bot::getAccessCommands(Jupiter::IRC::Client::Channel *chan, int access) {
	std::vector<IRCCommand*> result;
	for (const auto& command : m_commands) {
		if (command->getAccessLevel(chan) == access) {
			result.push_back(command.get());
		}
	}

	return result;
}

// TODO: This isn't really needed on here
std::string IRC_Bot::getTriggers(std::vector<IRCCommand*> &commands) {
	std::string result;
	for (const auto& command : commands) {
		result += command->getTrigger();
		result += ' ';
	}

	return result;
}

void IRC_Bot::setCommandAccessLevels(IRCCommand *in_command) {
	auto set_command_access_levels = [this, in_command](Jupiter::Config *in_section) {
		if (in_section == nullptr) {
			return;
		}

		Jupiter::Config *section = in_section->getSection("Commands"_jrs);
		if (section == nullptr) {
			return;
		}

		for (auto& entry : section->getTable()) {
			size_t tmp_index;
			Jupiter::ReferenceString tmp_key, tmp_sub_key;
			IRCCommand *command;

			tmp_index = entry.first.find('.');
			if (tmp_index != Jupiter::INVALID_INDEX) {
				// non-default access assignment

				tmp_key.set(entry.first.data(), tmp_index);

				tmp_sub_key = entry.first;
				tmp_sub_key.shiftRight(tmp_index + 1);

				if (jessilib::starts_withi(tmp_sub_key, "Type."sv)) {
					tmp_sub_key.shiftRight(5); // shift beyond "Type."

					command = this->getCommand(tmp_key);
					if (command != nullptr && (in_command == nullptr || in_command == command)) {
						command->setAccessLevel(Jupiter::from_string<int>(tmp_sub_key), Jupiter::from_string<int>(entry.second));
					}
				}
				else if (jessilib::starts_withi(tmp_sub_key, "Channel."sv)) {
					tmp_sub_key.shiftRight(8); // shift beyond "Channel."

					// Assign access level to command (if command exists)
					command = this->getCommand(tmp_key);
					if (command != nullptr && (in_command == nullptr || in_command == command))
						command->setAccessLevel(tmp_sub_key, Jupiter::from_string<int>(entry.second));
				}
			}
			else {
				// Assign access level to command (if command exists)
				command = this->getCommand(entry.first);
				if (command != nullptr && (in_command == nullptr || in_command == command))
					command->setAccessLevel(Jupiter::from_string<int>(entry.second));
			}
		};
	};

	set_command_access_levels(getSecondaryConfigSection());
	set_command_access_levels(getPrimaryConfigSection());
}

void IRC_Bot::OnChat(std::string_view in_channel, std::string_view nick, std::string_view message) {
	Channel *channel = this->getChannel(in_channel);
	if (channel != nullptr && channel->getType() >= 0) {
		std::string_view msg = message;
		while (!msg.empty() && isspace(msg[0])) {
			msg.remove_prefix(1);
		}

		if (m_commandPrefix.size() <= msg.size()) {
			bool matchesPrefix = jessilib::starts_withi(msg, m_commandPrefix);
			if (matchesPrefix) {
				// Strip off the prefix
				msg.remove_prefix(m_commandPrefix.size());

				auto message_split = jessilib::word_split_once_view(msg, WHITESPACE_SV);
				IRCCommand *cmd = getCommand(message_split.first);
				if (cmd != nullptr) {
					IRCCommand::active_server = this;
					int command_access = cmd->getAccessLevel(channel);
					if (command_access < 0) {
						this->sendNotice(nick, "Error: Command disabled."_jrs);
					}
					else if (Jupiter::IRC::Client::getAccessLevel(*channel, nick) < command_access) {
						this->sendNotice(nick, "Access Denied."_jrs);
					}
					else {
						cmd->trigger(this, Jupiter::ReferenceString{in_channel}, Jupiter::ReferenceString{nick}, Jupiter::ReferenceString{message_split.second});
					}

					IRCCommand::active_server = IRCCommand::selected_server;
				}
			}
		}
	}
}