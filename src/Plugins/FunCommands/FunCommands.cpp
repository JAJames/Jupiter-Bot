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
#include "jessilib/unicode.hpp"
#include "Jupiter/Functions.h"
#include "Jupiter/Socket.h"
#include "FunCommands.h"
#include "IRC_Bot.h"

using namespace std::literals;

// 8ball

void EightBallIRCCommand::create()
{
	this->addTrigger("8ball"sv);
	this->addTrigger("8balls"sv);
	this->addTrigger("eightBall"sv);
}

void EightBallIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	std::string msg;
	switch (rand() % 31)
	{
	case 0:
		msg = "No."sv;
		break;
	case 1:
		msg = "Yes."sv;
		break;
	case 2:
		msg = "Maybe."sv;
		break;
	case 3:
		msg = "Seven."sv;
		break;
	case 4:
		msg = "Consider counseling."sv;
		break;
	case 5:
		msg = "Look into a realationship. A real one."sv;
		break;
	case 6:
		msg = "Quit asking me these deep questions, before I get deeply into you."sv;
		break;
	case 7:
		msg = "Fuck you."sv;
		break;
	case 8:
		msg = "Fuck me."sv;
		break;
	case 9:
		msg = "Good thing I bend that way!"sv;
		break;
	case 10:
		msg = jessilib::join<std::string>("Hai "sv, nick, "!"sv);
		break;
	case 11:
		msg = jessilib::join<std::string>("Let's ban "sv, nick, "!"sv);
		break;
	case 12:
		msg = "Fuck your lineage."sv;
		break;
	case 13:
		if (channel == "#Jail"sv) {
			msg = "Ask me those kind of questions in #Politics"sv;
		}
		else {
			msg = "Ask me those kind of questions in #Jail"sv;
		}
		break;
	case 14:
		msg = "I don't talk to slut-mongers."sv;
		break;
	case 15:
		msg = "I only talk to slut-mongers."sv;
		break;
	case 16:
		msg = "Nuclear launch detected."sv;
		break;
	case 17:
		msg = "404 - Not found."sv;
		break;
	case 18:
		msg = "I hurr u liek mudkipz?"sv;
		break;
	case 19:
		msg = "Fortune might be in your favor."sv;
		break;
	case 20:
		msg = "Fortune might not be in your favor."sv;
		break;
	case 21:
		msg = "Based on the tragectory of James Bond (9007) in relationship to the 9th circle of hell located on the 3rd planet of Sol, you're going to hell."sv;
		break;
	case 22:
		msg = "Based on the tragectory of James Bond (9007) in relationship to the 9th circle of hell located on the 3rd planet of Sol, yes!"sv;
		break;
	case 23:
		msg = "Based on the tragectory of James Bond (9007) in relationship to the 9th circle of hell located on the 3rd planet of Sol, it's approximately as likely as getting a \"yes\" from me."sv;
		break;
	case 24:
		msg = "Fucking campers."sv;
		break;
	case 25:
		msg = "The 8ball seems to be stuck. Try again later. (Dang, you can't even get an 8ball to work for you? That's pretty sad yo)"sv;
		break;
	case 26:
		msg = "The 8ball says... Go away? Wow, it really must not like you. Go away."sv;
		break;
	case 27:
		msg = "Swag"sv;
		break;
	case 28:
		msg = "Based on the orbit and trajectory of the 33rd moon of the planet assinine 6, no."sv;
		break;
	case 29:
		msg = "Based on the orbin and trajectory of the moon assinsix 9, yes!"sv;
		break;
	case 30:
		msg = "Come visit me on the 5th planet away from Sol, and I'll give you whatever you want."sv;
		break;
	default:
		msg = "Nothingness. You suck. Go rot."sv;
		break;
	}
	source->sendMessage(channel, msg);
}

std::string_view EightBallIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Accurately answers your question 60% of the time, EVERY time. Syntax: 8ball [question]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(EightBallIRCCommand)

// Resolve Command

ResolveGenericCommand::ResolveGenericCommand()
{
	this->addTrigger("resolve"sv);
}

Jupiter::GenericCommand::ResponseLine *ResolveGenericCommand::trigger(std::string_view parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (command_split.second.empty()) {
		return new Jupiter::GenericCommand::ResponseLine("Error: Too few parameters. Syntax: resolve <hostname|ip> <address>"sv, GenericCommand::DisplayType::PrivateError);
	}

	std::string_view subcommand = command_split.first;
	if (jessilib::equalsi(subcommand, "hostname"sv)
		|| jessilib::equalsi(subcommand, "host"sv))
	{
		std::string_view resolved = Jupiter::Socket::resolveHostname(static_cast<std::string>(command_split.second).c_str(), 0);
		if (resolved.empty())
			return new Jupiter::GenericCommand::ResponseLine("Error: Unable to resolve."sv, GenericCommand::DisplayType::PublicError);
		return new Jupiter::GenericCommand::ResponseLine(resolved, GenericCommand::DisplayType::PublicSuccess);
	}
	else if (jessilib::equalsi(subcommand, "ip"sv))
	{
		std::string_view resolved = Jupiter::Socket::resolveAddress(static_cast<std::string>(command_split.second).c_str(), 0);
		if (resolved.empty())
			return new Jupiter::GenericCommand::ResponseLine("Error: Unable to resolve."sv, GenericCommand::DisplayType::PublicError);
		return new Jupiter::GenericCommand::ResponseLine(resolved, GenericCommand::DisplayType::PublicSuccess);
	}
	return new Jupiter::GenericCommand::ResponseLine("Error: Invalid type. You can only resolve hostnames and IP addresses."sv, GenericCommand::DisplayType::PrivateError);
}

std::string_view ResolveGenericCommand::getHelp(std::string_view parameters)
{
	static constexpr std::string_view defaultHelp = "Resolves an IP address or hostname. Syntax: resolve <hostname|ip> <address>"sv;
	static constexpr std::string_view hostHelp = "Resolves a hostname to an IP address. Syntax: resolve hostname <address>"sv;
	static constexpr std::string_view ipHelp = "Reverse-resolves an IP address to a hostname. Syntax: resolve ip <address>"sv;

	if (jessilib::equalsi(parameters, "hostname"sv) || jessilib::equalsi(parameters, "host"sv))
		return hostHelp;
	if (jessilib::equalsi(parameters, "ip"sv))
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
