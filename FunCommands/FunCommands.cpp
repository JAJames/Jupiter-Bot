/**
 * Copyright (C) 2014-2015 Justin James.
 *
 * This license must be preserved.
 * Any applications, libraries, or code which make any use of any
 * component of this program must not be commercial, unless explicit
 * permission is granted from the original author. The use of this
 * program for non-profit purposes is permitted.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In the event that this license restricts you from making desired use of this program, contact the original author.
 * Written by Justin James <justin.aj@hotmail.com>
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
	unsigned short r = rand() % 20;
	switch (r)
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
		msg.set("Good thing I bend that way. ;)"_jrs);
		break;
	case 10:
		msg.format("Hai %.*s ;)", nick.size(), nick.ptr());
		break;
	case 11:
		msg.format("Let's ban %.*s!", nick.size(), nick.ptr());
		break;
	case 12:
		msg.format("Sorry %.*s, but your IQ must be at least %d for me to care.", nick.size(), nick.ptr(), rand() % 50);
		break;
	case 13:
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
		msg.set("Walk this way, babycakes ;)"_jrs);
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

GenericCommand::ResponseLine *ResolveGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	unsigned int count = parameters.wordCount(WHITESPACE);

	if (count <= 1)
		return new GenericCommand::ResponseLine("Error: Too few parameters. Syntax: resolve <hostname|ip> <address>"_jrs, GenericCommand::DisplayType::PrivateError);

	Jupiter::ReferenceString command = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
	if (command.equalsi("hostname"_jrs) || command.equalsi("host"_jrs))
	{
		Jupiter::ReferenceString resolved = Jupiter::Socket::resolveHostname(Jupiter::CStringS::gotoWord(parameters, 1, WHITESPACE).c_str(), 0);
		if (resolved.isEmpty())
			return new GenericCommand::ResponseLine("Error: Unable to resolve."_jrs, GenericCommand::DisplayType::PublicError);
		return new GenericCommand::ResponseLine(resolved, GenericCommand::DisplayType::PublicSuccess);
	}
	else if (command.equalsi("ip"_jrs))
	{
		Jupiter::ReferenceString resolved = Jupiter::Socket::resolveAddress(Jupiter::CStringS::gotoWord(parameters, 1, WHITESPACE).c_str(), 0);
		if (resolved.isEmpty())
			return new GenericCommand::ResponseLine("Error: Unable to resolve."_jrs, GenericCommand::DisplayType::PublicError);
		return new GenericCommand::ResponseLine(resolved, GenericCommand::DisplayType::PublicSuccess);
	}
	return new GenericCommand::ResponseLine("Error: Invalid type. You can only resolve hostnames and IP addresses."_jrs, GenericCommand::DisplayType::PrivateError);
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

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
