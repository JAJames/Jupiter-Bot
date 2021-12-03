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

#include <cstring>
#include "jessilib/word_split.hpp"
#include "jessilib/unicode.hpp"
#include "Jupiter/Functions.h"
#include "ExtraCommands.h"
#include "IRC_Bot.h"

using namespace std::literals;

// Select Command

SelectGenericCommand::SelectGenericCommand()
{
	this->addTrigger("select"sv);
	this->addTrigger("ircselect"sv);
}

Jupiter::GenericCommand::ResponseLine *SelectGenericCommand::trigger(std::string_view parameters)
{
	if (parameters.empty())
	{
		if (IRCCommand::selected_server == nullptr)
			return new Jupiter::GenericCommand::ResponseLine("No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicSuccess);
		return new Jupiter::GenericCommand::ResponseLine(std::string{IRCCommand::selected_server->getConfigSection()} + " is currently selected."s, GenericCommand::DisplayType::PublicSuccess);
	}
	if (IRCCommand::active_server == IRCCommand::selected_server)
		IRCCommand::active_server = nullptr;

	IRCCommand::selected_server = serverManager->getServer(parameters);
	if (IRCCommand::selected_server == nullptr)
		return new Jupiter::GenericCommand::ResponseLine(jessilib::join<std::string>("Error: IRC server \""sv, parameters, "\" not found. No IRC server is currently selected."sv), GenericCommand::DisplayType::PublicError);

	if (IRCCommand::active_server == nullptr)
		IRCCommand::active_server = IRCCommand::selected_server;
	return new Jupiter::GenericCommand::ResponseLine(std::string{IRCCommand::selected_server->getConfigSection()} + " is now selected."s, GenericCommand::DisplayType::PublicSuccess);
}

std::string_view SelectGenericCommand::getHelp(std::string_view ) {
	return "Selects an IRC server if specified, responds with the currently selected server otherwise. Syntax: select [server]"sv;
}

GENERIC_COMMAND_INIT(SelectGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(SelectGenericCommand)

// Deselect Command

DeselectGenericCommand::DeselectGenericCommand()
{
	this->addTrigger("deselect"sv);
	this->addTrigger("ircdeselect"sv);
	this->addTrigger("dselect"sv);
	this->addTrigger("ircdselect"sv);
	this->addTrigger("unselect"sv);
	this->addTrigger("ircunselect"sv);
}

Jupiter::GenericCommand::ResponseLine *DeselectGenericCommand::trigger(std::string_view parameters)
{
	if (IRCCommand::selected_server == nullptr)
		return new Jupiter::GenericCommand::ResponseLine("No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicSuccess);

	Jupiter::GenericCommand::ResponseLine *ret = new Jupiter::GenericCommand::ResponseLine(std::string{IRCCommand::selected_server->getConfigSection()} + " has been deselected."s, GenericCommand::DisplayType::PublicSuccess);
	IRCCommand::selected_server = nullptr;
	IRCCommand::active_server = IRCCommand::selected_server;
	return ret;
}

std::string_view DeselectGenericCommand::getHelp(std::string_view ) {
	return "Deselects the currently selected IRC server. Syntax: deselect"sv;
}

GENERIC_COMMAND_INIT(DeselectGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(DeselectGenericCommand)

// Raw Command

RawGenericCommand::RawGenericCommand()
{
	this->addTrigger("raw"sv);
	this->addTrigger("sendraw"sv);
}

Jupiter::GenericCommand::ResponseLine *RawGenericCommand::trigger(std::string_view parameters)
{
	IRC_Bot *server;
	if (IRCCommand::selected_server != nullptr)
		server = IRCCommand::selected_server;
	else if (IRCCommand::active_server != nullptr)
		server = IRCCommand::active_server;
	else
		return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

	if (parameters.empty())
		return new Jupiter::GenericCommand::ResponseLine("Error: Too few parameters. Syntax: raw <message>"sv, GenericCommand::DisplayType::PrivateError);

	server->send(parameters);
	return new Jupiter::GenericCommand::ResponseLine("Data has been successfully sent to server."sv, GenericCommand::DisplayType::PublicSuccess);
}

std::string_view RawGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a line of data to the selected IRC server. Syntax: raw <message>"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(RawGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(RawGenericCommand)

// Message Command

IRCMessageGenericCommand::IRCMessageGenericCommand()
{
	this->addTrigger("ircmsg"sv);
	this->addTrigger("ircmessage"sv);
	this->addTrigger("privmsg"sv);
}

Jupiter::GenericCommand::ResponseLine *IRCMessageGenericCommand::trigger(std::string_view parameters)
{
	IRC_Bot *server;
	if (IRCCommand::selected_server != nullptr)
		server = IRCCommand::selected_server;
	else if (IRCCommand::active_server != nullptr)
		server = IRCCommand::active_server;
	else
		return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (parameters_split.second.empty())
		return new Jupiter::GenericCommand::ResponseLine("Error: Too few parameters. Syntax: ircmsg <destination> <message>"sv, GenericCommand::DisplayType::PrivateError);

	server->sendMessage(parameters_split.first, parameters_split.second);
	return new Jupiter::GenericCommand::ResponseLine("Message successfully sent."sv, GenericCommand::DisplayType::PublicSuccess);
}

std::string_view IRCMessageGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message to an IRC user or channel on the selected IRC server. Syntax: ircmsg <destination> <message>"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(IRCMessageGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(IRCMessageGenericCommand)

// Join Command

JoinGenericCommand::JoinGenericCommand()
{
	this->addTrigger("Join"sv);
}

Jupiter::GenericCommand::ResponseLine *JoinGenericCommand::trigger(std::string_view parameters)
{
	IRC_Bot *server;
	if (IRCCommand::selected_server != nullptr)
		server = IRCCommand::selected_server;
	else if (IRCCommand::active_server != nullptr)
		server = IRCCommand::active_server;
	else
		return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

	if (parameters.empty())
		return new Jupiter::GenericCommand::ResponseLine("Error: Too Few Parameters. Syntax: join <channel> [password]"sv, GenericCommand::DisplayType::PublicError);

	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (parameters_split.second.empty()) {
		server->joinChannel(parameters);
	}
	else {
		server->joinChannel(parameters_split.first, parameters_split.second);
	}

	return new Jupiter::GenericCommand::ResponseLine("Request to join channel has been sent."sv, GenericCommand::DisplayType::PublicSuccess);
}

std::string_view JoinGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Joins a channel. Syntax: join <channel> [password]"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(JoinGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(JoinGenericCommand)

// Part Command

PartGenericCommand::PartGenericCommand()
{
	this->addTrigger("Part"sv);
}

Jupiter::GenericCommand::ResponseLine *PartGenericCommand::trigger(std::string_view parameters)
{
	IRC_Bot *server;
	if (IRCCommand::selected_server != nullptr)
		server = IRCCommand::selected_server;
	else if (IRCCommand::active_server != nullptr)
		server = IRCCommand::active_server;
	else
		return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

	if (parameters.empty())
		return new Jupiter::GenericCommand::ResponseLine("Error: Too few parameters. Syntax: part <channel> [message]"sv, GenericCommand::DisplayType::PublicError);

	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (parameters_split.second.empty()) {
		server->partChannel(parameters);
	}
	else {
		server->partChannel(parameters_split.first, parameters_split.second);
	}

	return new Jupiter::GenericCommand::ResponseLine("Part command successfuly sent."sv, GenericCommand::DisplayType::PublicSuccess);
}

std::string_view PartGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Parts from a channel. Syntax: part <channel> [message]"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(PartGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(PartGenericCommand)

// DebugInfo Command

DebugInfoGenericCommand::DebugInfoGenericCommand()
{
	this->addTrigger("debuginfo"sv);
}

Jupiter::GenericCommand::ResponseLine *DebugInfoGenericCommand::trigger(std::string_view parameters)
{
	IRC_Bot *server;
	if (IRCCommand::selected_server != nullptr)
		server = IRCCommand::selected_server;
	else if (IRCCommand::active_server != nullptr)
		server = IRCCommand::active_server;
	else
		return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

	Jupiter::GenericCommand::ResponseLine *ret = new Jupiter::GenericCommand::ResponseLine("Prefixes: "s += server->getPrefixes(), GenericCommand::DisplayType::PublicSuccess);
	Jupiter::GenericCommand::ResponseLine *line = new Jupiter::GenericCommand::ResponseLine("Prefix Modes: "s += server->getPrefixModes(), GenericCommand::DisplayType::PublicSuccess);
	ret->next = line;
	line->next = new Jupiter::GenericCommand::ResponseLine(string_printf("Outputting data for %u channels...", server->getChannelCount()), GenericCommand::DisplayType::PublicSuccess);
	line = line->next;

	for (auto& channel_pair : server->getChannels()) {
		auto& channel = channel_pair.second;
		line->next = new Jupiter::GenericCommand::ResponseLine(string_printf("Channel %.*s - Type: %d", channel.getName().size(),
			channel.getName().data(), channel.getType()), GenericCommand::DisplayType::PublicSuccess);
		line = line->next;

		for (auto& user_pair : channel.getUsers()) {
			Jupiter::IRC::Client::User *user = user_pair.second->getUser();
			line->next = new Jupiter::GenericCommand::ResponseLine(string_printf("User %.*s!%.*s@%.*s (prefix: %c; prefixes: %.*s) of channel %.*s (of %u shared)",
				user->getNickname().size(), user->getNickname().data(),
				user->getUsername().size(), user->getUsername().data(),
				user->getHostname().size(), user->getHostname().data(),
				channel.getUserPrefix(*user_pair.second) ? channel.getUserPrefix(*user_pair.second) : ' ',
				user_pair.second->getPrefixes().size(), user_pair.second->getPrefixes().data(),
				channel.getName().size(), channel.getName().data(),
				user->getChannelCount()), GenericCommand::DisplayType::PublicSuccess);
			line = line->next;
		};
	};

	return ret;
}

std::string_view DebugInfoGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "DEBUG COMMAND - Spits out some information about channels. Syntax: debuginfo"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(DebugInfoGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(DebugInfoGenericCommand)

// Exit command

ExitGenericCommand::ExitGenericCommand()
{
	this->addTrigger("exit"sv);
}

Jupiter::GenericCommand::ResponseLine *ExitGenericCommand::trigger(std::string_view parameters)
{
	exit(0);
}

std::string_view ExitGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Closes the bot's application process. Syntax: exit"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(ExitGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(ExitGenericCommand)

// IRC Connect command

IRCConnectGenericCommand::IRCConnectGenericCommand()
{
	this->addTrigger("IRCConnect"sv);
	this->addTrigger("IRCReconnect"sv);
}

Jupiter::GenericCommand::ResponseLine *IRCConnectGenericCommand::trigger(std::string_view parameters)
{
	if (parameters.empty())
	{
		IRC_Bot *server;
		if (IRCCommand::selected_server != nullptr)
			server = IRCCommand::selected_server;
		else if (IRCCommand::active_server != nullptr)
			server = IRCCommand::active_server;
		else
			return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

		server->disconnect("Connect command used; reconnecting..."sv, false);
		return new Jupiter::GenericCommand::ResponseLine("Disconnected from IRC server."sv, GenericCommand::DisplayType::PublicSuccess);
	}
	IRC_Bot *server = serverManager->getServer(parameters);
	if (server != nullptr)
	{
		server->disconnect("Connect command used; reconnecting..."sv, false);
		return new Jupiter::GenericCommand::ResponseLine("Disconnected from IRC server."sv, GenericCommand::DisplayType::PublicSuccess);
	}
	if (serverManager->addServer(parameters))
		return new Jupiter::GenericCommand::ResponseLine("Connection successfully established; server added to server list."sv, GenericCommand::DisplayType::PublicSuccess);
	return new Jupiter::GenericCommand::ResponseLine("Error: Unable to find configuration settings for server, or connection refused."sv, GenericCommand::DisplayType::PublicError);
}

std::string_view IRCConnectGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Connects/reconnects to an IRC server, based on config entry. Syntax: IRCConnect [server=here]"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(IRCConnectGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(IRCConnectGenericCommand)

// IRC Disconnect command

IRCDisconnectGenericCommand::IRCDisconnectGenericCommand()
{
	this->addTrigger("IRCDisconnect"sv);
}

Jupiter::GenericCommand::ResponseLine *IRCDisconnectGenericCommand::trigger(std::string_view parameters)
{
	IRC_Bot *server;
	if (IRCCommand::selected_server != nullptr)
		server = IRCCommand::selected_server;
	else if (IRCCommand::active_server != nullptr)
		server = IRCCommand::active_server;
	else
		return new Jupiter::GenericCommand::ResponseLine("Error: No IRC server is currently selected."sv, GenericCommand::DisplayType::PublicError);

	server->disconnect("Disconnect command used."sv, true);
	return new Jupiter::GenericCommand::ResponseLine("Disconnected from server."sv, GenericCommand::DisplayType::PublicSuccess);
}

std::string_view IRCDisconnectGenericCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Disconnects from an IRC server, based on config entry. Syntax: IRCDisconnect"sv;
	return defaultHelp;
}

GENERIC_COMMAND_INIT(IRCDisconnectGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(IRCDisconnectGenericCommand)

// Plugin instantiation and entry point.
FunCommandsPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
