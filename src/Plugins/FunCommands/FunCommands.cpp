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
#include "Jupiter/Functions.h"
#include "Jupiter/Socket.h"
#include "Jupiter/CString.h"
#include "FunCommands.h"
#include "IRC_Bot.h"

using namespace Jupiter::literals;

// 8ball

void EightBallIRCCommand::create()
{
	this->addTrigger("8ball"_jrs);
	this->addTrigger("8balls"_jrs);
	this->addTrigger("eightBall"_jrs);
}

void EightBallIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::StringS msg;
	switch (rand() % 31)
	{
	case 0:
		msg.set("No."_jrs);
		break;
	case 1:
		msg.set("Yes."_jrs);
		break;
	case 2:
		msg.set("Maybe."_jrs);
		break;
	case 3:
		msg.set("Seven."_jrs);
		break;
	case 4:
		msg.set("Consider counseling."_jrs);
		break;
	case 5:
		msg.set("Look into a realationship. A real one."_jrs);
		break;
	case 6:
		msg.set("Quit asking me these deep questions, before I get deeply into you."_jrs);
		break;
	case 7:
		msg.set("Fuck you."_jrs);
		break;
	case 8:
		msg.set("Fuck me."_jrs);
		break;
	case 9:
		msg.set("Good thing I bend that way!"_jrs);
		break;
	case 10:
		msg.format("Hai %.*s ;)", nick.size(), nick.ptr());
		break;
	case 11:
		msg.format("Let's ban %.*s!", nick.size(), nick.ptr());
		break;
	case 12:
		msg.set("Fuck your lineage."_jrs);
		break;
	case 13:
		if (channel.equals("#Jail"_jrs))
			msg.set("Ask me those kind of questions in #Politics"_jrs);
		else
			msg.set("Ask me those kind of questions in #Jail"_jrs);
		break;
	case 14:
		msg.set("I don't talk to slut-mongers."_jrs);
		break;
	case 15:
		msg.set("I only talk to slut-mongers."_jrs);
		break;
	case 16:
		msg.set("Nuclear launch detected."_jrs);
		break;
	case 17:
		msg.set("404 - Not found."_jrs);
		break;
	case 18:
		msg.set("I hurr u liek mudkipz?"_jrs);
		break;
	case 19:
		msg.set("Fortune might be in your favor."_jrs);
		break;
	case 20:
		msg.set("Fortune might not be in your favor."_jrs);
		break;
	case 21:
		msg.set("Based on the tragectory of James Bond (9007) in relationship to the 9th circle of hell located on the 3rd planet of Sol, you're going to hell."_jrs);
		break;
	case 22:
		msg.set("Based on the tragectory of James Bond (9007) in relationship to the 9th circle of hell located on the 3rd planet of Sol, yes!"_jrs);
		break;
	case 23:
		msg.set("Based on the tragectory of James Bond (9007) in relationship to the 9th circle of hell located on the 3rd planet of Sol, it's approximately as likely as getting a \"yes\" from me."_jrs);
		break;
	case 24:
		msg.set("Fucking campers."_jrs);
		break;
	case 25:
		msg.set("The 8ball seems to be stuck. Try again later. (Dang, you can't even get an 8ball to work for you? That's pretty sad yo)"_jrs);
		break;
	case 26:
		msg.set("The 8ball says... Go away? Wow, it really must not like you. Go away."_jrs);
		break;
	case 27:
		msg.set("Swag"_jrs);
		break;
	case 28:
		msg.set("Based on the orbit and trajectory of the 33rd moon of the planet assinine 6, no."_jrs);
		break;
	case 29:
		msg.set("Based on the orbin and trajectory of the moon assinsix 9, yes!"_jrs);
		break;
	case 30:
		msg.set("Come visit me on the 5th planet away from Sol, and I'll give you whatever you want."_jrs);
		break;
	default:
		msg.set("Nothingness. You suck. Go rot."_jrs);
		break;
	}
	source->sendMessage(channel, msg);
}

const Jupiter::ReadableString &EightBallIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Accurately answers your question 60% of the time, EVERY time. Syntax: 8ball [question]");
	return defaultHelp;
}

IRC_COMMAND_INIT(EightBallIRCCommand)

// Resolve Command

ResolveGenericCommand::ResolveGenericCommand()
{
	this->addTrigger("resolve"_jrs);
}

Jupiter::GenericCommand::ResponseLine *ResolveGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	unsigned int count = parameters.wordCount(WHITESPACE);

	if (count <= 1)
		return new Jupiter::GenericCommand::ResponseLine("Error: Too few parameters. Syntax: resolve <hostname|ip> <address>"_jrs, GenericCommand::DisplayType::PrivateError);

	Jupiter::ReferenceString command = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
	if (command.equalsi("hostname"_jrs) || command.equalsi("host"_jrs))
	{
		Jupiter::ReferenceString resolved = Jupiter::Socket::resolveHostname(Jupiter::CStringS::gotoWord(parameters, 1, WHITESPACE).c_str(), 0);
		if (resolved.isEmpty())
			return new Jupiter::GenericCommand::ResponseLine("Error: Unable to resolve."_jrs, GenericCommand::DisplayType::PublicError);
		return new Jupiter::GenericCommand::ResponseLine(resolved, GenericCommand::DisplayType::PublicSuccess);
	}
	else if (command.equalsi("ip"_jrs))
	{
		Jupiter::ReferenceString resolved = Jupiter::Socket::resolveAddress(Jupiter::CStringS::gotoWord(parameters, 1, WHITESPACE).c_str(), 0);
		if (resolved.isEmpty())
			return new Jupiter::GenericCommand::ResponseLine("Error: Unable to resolve."_jrs, GenericCommand::DisplayType::PublicError);
		return new Jupiter::GenericCommand::ResponseLine(resolved, GenericCommand::DisplayType::PublicSuccess);
	}
	return new Jupiter::GenericCommand::ResponseLine("Error: Invalid type. You can only resolve hostnames and IP addresses."_jrs, GenericCommand::DisplayType::PrivateError);
}

const Jupiter::ReadableString &ResolveGenericCommand::getHelp(const Jupiter::ReadableString &parameters)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resolves an IP address or hostname. Syntax: resolve <hostname|ip> <address>");
	static STRING_LITERAL_AS_NAMED_REFERENCE(hostHelp, "Resolves a hostname to an IP address. Syntax: resolve hostname <address>");
	static STRING_LITERAL_AS_NAMED_REFERENCE(ipHelp, "Reverse-resolves an IP address to a hostname. Syntax: resolve ip <address>");

	if (parameters.equalsi("hostname"_jrs) || parameters.equalsi("host"_jrs))
		return hostHelp;
	if (parameters.equalsi("ip"_jrs))
		return ipHelp;

	return defaultHelp;
}

GENERIC_COMMAND_INIT(ResolveGenericCommand)

// Plugin instantiation and entry point.
ExtraCommandsPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
