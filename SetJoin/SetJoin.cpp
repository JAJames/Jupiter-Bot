/**
 * Copyright (C) 2014-2015 Jessica James.
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
#include "Jupiter/INIFile.h"
#include "Jupiter/Functions.h"
#include "SetJoin.h"
#include "IRC_Bot.h"

void SetJoinPlugin::OnJoin(Jupiter::IRC::Client *server, const Jupiter::ReadableString &chan, const Jupiter::ReadableString &nick)
{
	const Jupiter::ReadableString &setjoin = server->Config->get(STRING_LITERAL_AS_REFERENCE("SetJoins"), nick);
	if (setjoin.isNotEmpty())
	{
		if (setjoin == nullptr)
			server->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("No setjoin has been set for you. To set one, use the !setjoin command"));
		else
			server->sendMessage(chan, Jupiter::StringS::Format(IRCBOLD IRCCOLOR "07[%.*s]" IRCCOLOR IRCBOLD ": %.*s", nick.size(), nick.ptr(), setjoin.size(), setjoin.ptr()));
	}
}

// SetJoin Command

void SetJoinIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("setJoin"));
}

void SetJoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		source->Config->set(STRING_LITERAL_AS_REFERENCE("SetJoins"), nick, parameters);
		source->Config->sync();
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Your join message has been set."));
	}
	else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Too few parameters! Syntax: setjoin <message>"));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("viewJoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("vJoin"));
}

void ViewJoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	const Jupiter::ReadableString &target = parameters.isEmpty() ? nick : parameters;
	const Jupiter::ReadableString &r = source->Config->get(STRING_LITERAL_AS_REFERENCE("SetJoins"), target);

	if (r.isEmpty())
		source->sendMessage(channel, Jupiter::StringS::Format("No setjoin has been set for \"%.*s\".", target.size(), target.ptr()));
	else
		source->sendMessage(channel, Jupiter::StringS::Format(IRCBOLD IRCCOLOR "07[%.*s]" IRCCOLOR IRCBOLD ": %.*s", target.size(), target.ptr(), r.size(), r.ptr()));
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("delJoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("dJoin"));
}

void DelJoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (source->Config->remove(STRING_LITERAL_AS_REFERENCE("SetJoins"), nick))
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Your setjoin has been deleted successfully."));
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("No setjoin was found to delete."));
}

const Jupiter::ReadableString &DelJoinIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Deletes your join message. Syntax: deljoin");
	return defaultHelp;
}

IRC_COMMAND_INIT(DelJoinIRCCommand)

// Plugin instantiation and entry point.
SetJoinPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
