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
#include "ExtraCommands.h"
#include "IRC_Bot.h"

// Raw Console Command

RawConsoleCommand::RawConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("raw"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("sendraw"));
}

void RawConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) < 2)
		puts("Error: Too few parameters.");
	else
	{
		Jupiter::ReferenceString network = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
		Jupiter::ReferenceString message = Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);
		size_t total = 0;
		for (size_t i = 0; i < serverManager->size(); i++)
		{
			IRC_Bot *server = serverManager->getServer(i);
			if (server->getConfigSection().matchi(network))
			{
				server->send(message);
				total++;
			}
		}
		printf("%u packets sent." ENDL, total);
	}
}

const Jupiter::ReadableString &RawConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Syntax: raw <network> <message>");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RawConsoleCommand)

// Message Console Command

MessageConsoleCommand::MessageConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ircmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("message"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("privmsg"));
}

void MessageConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (parameters.wordCount(WHITESPACE) < 3)
		puts("Error: Too few parameters.");
	else
	{
		Jupiter::ReferenceString network = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
		Jupiter::ReferenceString channel = Jupiter::ReferenceString::getWord(parameters, 1, WHITESPACE);
		Jupiter::ReferenceString message = Jupiter::ReferenceString::gotoWord(parameters, 2, WHITESPACE);

		size_t total = 0;
		for (size_t i = 0; i < serverManager->size(); i++)
		{
			IRC_Bot *server = serverManager->getServer(i);
			if (server->getConfigSection().matchi(network))
			{
				server->sendMessage(channel, message);
				total++;
			}
		}
		printf("%u messages sent." ENDL, total);
	}
}

const Jupiter::ReadableString &MessageConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message to an IRC user/channel, on a specified network (accepts wildcards). Syntax: msg <network> <destination> <message>");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(MessageConsoleCommand)

// Join Command

void JoinIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("Join"));
	this->setAccessLevel(3);
}

void JoinIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		if (parameters.wordCount(WHITESPACE) == 1)
			source->joinChannel(parameters);
		else source->joinChannel(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE), Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: join <channel> [password]"));
}

const Jupiter::ReadableString &JoinIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Joins a channel. Syntax: join <Channel>");
	return defaultHelp;
}

IRC_COMMAND_INIT(JoinIRCCommand)

// Part Command

void PartIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("Part"));
	this->setAccessLevel(3);
}

void PartIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		if (parameters.wordCount(WHITESPACE) == 1)
			source->partChannel(parameters);
		else source->partChannel(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE), Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE));
	}
	else source->partChannel(channel);
}

const Jupiter::ReadableString &PartIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Parts from a channel. Syntax: part [channel=here] [message=empty]");
	return defaultHelp;
}

IRC_COMMAND_INIT(PartIRCCommand)

// Info Command

void InfoIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("info"));
	this->setAccessLevel(4);
}

void InfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::String msg;
	msg.format("Prefixes: %.*s", source->getPrefixes().size(), source->getPrefixes().ptr());
	source->sendMessage(channel, msg);
	msg.format("Prefix Modes: %.*s", source->getPrefixModes().size(), source->getPrefixModes().ptr());
	source->sendMessage(channel, msg);
	msg.format("Outputing data for %u channels...", source->getChannelCount());
	source->sendMessage(channel, msg);
	for (unsigned int a = 0; a < source->getChannelCount(); a++)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(a);
		if (chan != nullptr)
		{
			msg.format("Channel %.*s - Type: %d", chan->getName().size(), chan->getName().ptr(), chan->getType());
			source->sendMessage(channel, msg);
			for (unsigned int b = 0; b < chan->getUserCount(); b++)
			{
				Jupiter::IRC::Client::Channel::User *chanUser = chan->getUser(b);
				Jupiter::IRC::Client::User *user = chanUser->getUser();
				msg.format("PRIVMSG %.*s :User %.*s!%.*s@%.*s (prefix: %c) of channel %.*s (of %u shared)" ENDL, channel.size(), channel.ptr(), user->getNickname().size(), user->getNickname().ptr(), user->getUsername().size(), user->getUsername().ptr(), user->getHostname().size(), user->getHostname().ptr(), chan->getUserPrefix(b) ? chan->getUserPrefix(b) : ' ', chan->getName().size(), chan->getName().ptr(), user->getChannelCount());
				source->send(msg);
			}
		}
	}
}

const Jupiter::ReadableString &InfoIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "TEMPORARY COMMAND - Spits out some info about channels. Syntax: info");
	return defaultHelp;
}

IRC_COMMAND_INIT(InfoIRCCommand)

// Exit command

void ExitIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("exit"));
	this->setAccessLevel(5);
}

void ExitIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Bye!"));
	exit(0);
}

const Jupiter::ReadableString &ExitIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Closes the bot's application process. Syntax: exit");
	return defaultHelp;
}

IRC_COMMAND_INIT(ExitIRCCommand)

// IRC Connect command

void IRCConnectIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("IRCConnect"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("IRCReconnect"));
	this->setAccessLevel(5);
}

void IRCConnectIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters == nullptr)
		source->disconnect(STRING_LITERAL_AS_REFERENCE("Connect command used; reconnecting..."), false);
	else
	{
		IRC_Bot *server = serverManager->getServer(parameters);
		if (server != nullptr)
		{
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Warning: Server already exists. Severing connection..."));
			server->disconnect(true);
		}
		if (serverManager->addServer(parameters))
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Connection successfully established; server added to server list."));
		else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Unable to find configuration settings for server, or connection refused."));
	}
}

const Jupiter::ReadableString &IRCConnectIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Connects/reconnects to an IRC server, based on config entry. Syntax: IRCConnect [server=here]");
	return defaultHelp;
}

IRC_COMMAND_INIT(IRCConnectIRCCommand)

// IRC Disconnect command

void IRCDisconnectIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("IRCDisconnect"));
	this->setAccessLevel(5);
}

void IRCDisconnectIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
		source->disconnect(STRING_LITERAL_AS_REFERENCE("Disconnect command used"), true);
	else
	{
		IRC_Bot *server = serverManager->getServer(parameters);
		if (server == nullptr)
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Server not found."));
		else server->disconnect(STRING_LITERAL_AS_REFERENCE("Disconnect command used remotely"), true);
	}
}

const Jupiter::ReadableString &IRCDisconnectIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disconnects from an IRC server, based on config entry. Syntax: IRCDisconnect [server=here]");
	return defaultHelp;
}

IRC_COMMAND_INIT(IRCDisconnectIRCCommand)

// Plugin instantiation and entry point.
FunCommandsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
