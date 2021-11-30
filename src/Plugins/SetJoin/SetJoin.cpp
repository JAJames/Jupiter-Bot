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

#include <cstring>
#include "Jupiter/Functions.h"
#include "SetJoin.h"
#include "IRC_Bot.h"

using namespace Jupiter::literals;

void SetJoinPlugin::OnJoin(Jupiter::IRC::Client *server, std::string_view chan, std::string_view nick)
{
	std::string_view setjoin = this->config[server->getConfigSection()].get(nick);
	if (setjoin.empty())
		server->sendNotice(nick, "No setjoin has been set for you. To set one, use the !setjoin command"_jrs);
	else
		server->sendMessage(chan, Jupiter::StringS::Format(IRCBOLD IRCCOLOR "07[%.*s]" IRCCOLOR IRCBOLD ": %.*s", nick.size(), nick.data(), setjoin.size(), setjoin.data()));
}

SetJoinPlugin pluginInstance;

// SetJoin Command

void SetJoinIRCCommand::create()
{
	this->addTrigger("setJoin"_jrs);
}

void SetJoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		pluginInstance.setjoin_file[source->getConfigSection()].set(nick, static_cast<std::string>(parameters));
		pluginInstance.setjoin_file.write();
		source->sendMessage(channel, "Your join message has been set."_jrs);
	}
	else source->sendMessage(channel, "Too few parameters! Syntax: setjoin <message>"_jrs);
}

const Jupiter::ReadableString &SetJoinIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sets your join message. Syntax: setjoin <message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(SetJoinIRCCommand)

// ViewJoin Command

void ViewJoinIRCCommand::create()
{
	this->addTrigger("viewJoin"_jrs);
	this->addTrigger("vJoin"_jrs);
}

void ViewJoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	const Jupiter::ReadableString &target = parameters.empty() ? nick : parameters;
	std::string_view setjoin = pluginInstance.setjoin_file[source->getConfigSection()].get(target);

	if (setjoin.empty())
		source->sendMessage(channel, Jupiter::StringS::Format("No setjoin has been set for \"%.*s\".", target.size(), target.ptr()));
	else
		source->sendMessage(channel, Jupiter::StringS::Format(IRCBOLD IRCCOLOR "07[%.*s]" IRCCOLOR IRCBOLD ": %.*s", target.size(), target.ptr(), setjoin.size(), setjoin.data()));
}

const Jupiter::ReadableString &ViewJoinIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Views a user's join message. Syntax: viewjoin [user=you]");
	return defaultHelp;
}

IRC_COMMAND_INIT(ViewJoinIRCCommand)

// DelJoin Command

void DelJoinIRCCommand::create()
{
	this->addTrigger("delJoin"_jrs);
	this->addTrigger("dJoin"_jrs);
}

void DelJoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (pluginInstance.setjoin_file[source->getConfigSection()].remove(nick))
		source->sendNotice(nick, "Your setjoin has been deleted successfully."_jrs);
	else source->sendNotice(nick, "No setjoin was found to delete."_jrs);
}

const Jupiter::ReadableString &DelJoinIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Deletes your join message. Syntax: deljoin");
	return defaultHelp;
}

IRC_COMMAND_INIT(DelJoinIRCCommand)

// Plugin instantiation and entry point.

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
