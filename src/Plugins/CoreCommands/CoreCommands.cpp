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

#include <cstring>
#include "jessilib/word_split.hpp"
#include "Jupiter/Functions.h"
#include "CoreCommands.h"
#include "IRC_Bot.h"

using namespace std::literals;

// Help Console Command

HelpConsoleCommand::HelpConsoleCommand() {
	this->addTrigger("help"sv);
}

void HelpConsoleCommand::trigger(std::string_view parameters) {
	if (parameters.empty()) {
		std::cout << "Supported commands:";
		for (const auto& command : consoleCommands) {
			std::cout << ' ' << command->getTrigger();
		}
		std::cout << std::endl
			<< Jupiter::version << " - " << Jupiter::copyright << std::endl
			<< "For command-specific help, use: help <command>" << std::endl;
		return;
	}

	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	ConsoleCommand *cmd = getConsoleCommand(command_split.first);
	if (cmd == nullptr) {
		std::cout << "Error: Command \"" << command_split.first << "\" not found." << std::endl;
		return;
	}

	std::cout << std::string_view{cmd->getHelp(command_split.second)} << std::endl;
}

std::string_view HelpConsoleCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Lists commands, or sends command-specific help. Syntax: help [command]"sv;
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(HelpConsoleCommand)

// Help IRC Command.

void HelpIRCCommand::create() {
	this->addTrigger("help"sv);
}

void HelpIRCCommand::trigger(IRC_Bot *source, std::string_view in_channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *channel = source->getChannel(in_channel);
	if (channel != nullptr)
	{
		int access = source->getAccessLevel(*channel, nick);
		if (parameters.empty()) {
			for (int i = 0; i <= access; i++) {
				auto cmds = source->getAccessCommands(channel, i);
				if (cmds.size() != 0) {
					std::string triggers = source->getTriggers(cmds);
					if (triggers.size() >= 0) {
						source->sendNotice(nick, string_printf("Access level %d commands: %.*s", i, triggers.size(),
							triggers.data()));
					}
				}
			}
			source->sendNotice(nick, "For command-specific help, use: help <command>"sv);
		}
		else {
			auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
			IRCCommand *cmd = source->getCommand(command_split.first);
			if (cmd) {
				int command_access = cmd->getAccessLevel(channel);

				if (command_access < 0)
					source->sendNotice(nick, "Error: Command disabled."sv);
				else if (access < command_access)
					source->sendNotice(nick, "Access Denied."sv);
				else
					source->sendNotice(nick, cmd->getHelp(command_split.second));
			}
			else source->sendNotice(nick, "Error: Command not found."sv);
		}
	}
}

std::string_view HelpIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Syntax: help [command]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(HelpIRCCommand)

// Version Command

VersionGenericCommand::VersionGenericCommand() {
	this->addTrigger("version"sv);
	this->addTrigger("versioninfo"sv);
	this->addTrigger("copyright"sv);
	this->addTrigger("copyrightinfo"sv);
	this->addTrigger("client"sv);
	this->addTrigger("clientinfo"sv);
}

Jupiter::GenericCommand::ResponseLine *VersionGenericCommand::trigger(std::string_view parameters) {
	Jupiter::GenericCommand::ResponseLine *ret = new Jupiter::GenericCommand::ResponseLine("Version: "s + Jupiter::version, GenericCommand::DisplayType::PublicSuccess);
	ret->next = new Jupiter::GenericCommand::ResponseLine(std::string_view{ Jupiter::copyright }, GenericCommand::DisplayType::PublicSuccess);
	return ret;
}

std::string_view VersionGenericCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Displays version and copyright information"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(VersionGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(VersionGenericCommand)

// Rehash Command

RehashGenericCommand::RehashGenericCommand() {
	this->addTrigger("rehash"sv);
}

Jupiter::GenericCommand::ResponseLine *RehashGenericCommand::trigger(std::string_view parameters) {
	size_t hash_errors = Jupiter::rehash();

	if (hash_errors == 0)
		return new Jupiter::GenericCommand::ResponseLine(string_printf("All %u objects were successfully rehashed.", Jupiter::getRehashableCount()), GenericCommand::DisplayType::PublicSuccess);

	return new Jupiter::GenericCommand::ResponseLine(string_printf("%u of %u objects failed to successfully rehash.", hash_errors, Jupiter::getRehashableCount()), GenericCommand::DisplayType::PublicError);
}

std::string_view RehashGenericCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Rehashes configuration data from a file. Syntax: rehash [file]"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(RehashGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(RehashGenericCommand)

// Plugin instantiation and entry point.
CoreCommandsPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
