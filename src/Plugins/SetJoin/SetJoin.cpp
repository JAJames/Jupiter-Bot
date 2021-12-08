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
#include "jessilib/unicode.hpp"
#include "Jupiter/Functions.h"
#include "SetJoin.h"
#include "IRC_Bot.h"

using namespace std::literals;

void SetJoinPlugin::OnJoin(Jupiter::IRC::Client *server, std::string_view chan, std::string_view nick) {
	std::string_view setjoin = this->config[server->getConfigSection()].get(nick);
	if (setjoin.empty()) {
		server->sendNotice(nick, "No setjoin has been set for you. To set one, use the !setjoin command"sv);
		return;
	}

	std::string join_message = jessilib::join<std::string>(IRCBOLD IRCCOLOR "07["sv, nick, "]" IRCCOLOR IRCBOLD ": "sv, setjoin);
	server->sendMessage(chan, join_message);
}

SetJoinPlugin pluginInstance;

// SetJoin Command

void SetJoinIRCCommand::create()
{
	this->addTrigger("setJoin"sv);
}

void SetJoinIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		pluginInstance.setjoin_file[source->getConfigSection()].set(nick, static_cast<std::string>(parameters));
		pluginInstance.setjoin_file.write();
		source->sendMessage(channel, "Your join message has been set."sv);
	}
	else source->sendMessage(channel, "Too few parameters! Syntax: setjoin <message>"sv);
}

std::string_view SetJoinIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sets your join message. Syntax: setjoin <message>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(SetJoinIRCCommand)

// ViewJoin Command

void ViewJoinIRCCommand::create()
{
	this->addTrigger("viewJoin"sv);
	this->addTrigger("vJoin"sv);
}

void ViewJoinIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	std::string_view target = parameters.empty() ? nick : parameters;
	std::string_view setjoin = pluginInstance.setjoin_file[source->getConfigSection()].get(target);

	if (setjoin.empty()) {
		source->sendMessage(channel, jessilib::join<std::string>("No setjoin has been set for \""sv, target, "\"."sv));
	}
	else {
		std::string join_message = IRCBOLD IRCCOLOR "07[";
		join_message += target;
		join_message += "]" IRCCOLOR IRCBOLD ": ";
		join_message += setjoin;
		source->sendMessage(channel, join_message);
	}
}

std::string_view ViewJoinIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Views a user's join message. Syntax: viewjoin [user=you]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(ViewJoinIRCCommand)

// DelJoin Command

void DelJoinIRCCommand::create()
{
	this->addTrigger("delJoin"sv);
	this->addTrigger("dJoin"sv);
}

void DelJoinIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (pluginInstance.setjoin_file[source->getConfigSection()].remove(nick))
		source->sendNotice(nick, "Your setjoin has been deleted successfully."sv);
	else source->sendNotice(nick, "No setjoin was found to delete."sv);
}

std::string_view DelJoinIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Deletes your join message. Syntax: deljoin"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(DelJoinIRCCommand)

// Plugin instantiation and entry point.

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
