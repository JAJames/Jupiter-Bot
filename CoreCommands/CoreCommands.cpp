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
#include "Jupiter/INIFile.h"
#include "Jupiter/Functions.h"
#include "Jupiter/ArrayList.h"
#include "CoreCommands.h"
#include "IRC_Bot.h"

using namespace Jupiter::literals;

// Help Console Command

HelpConsoleCommand::HelpConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("help"));
}

void HelpConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
	{
		fputs("Supported commands:", stdout);
		for (size_t i = 0; i != consoleCommands->size(); i++)
		{
			fputc(' ', stdout);
			consoleCommands->get(i)->getTrigger().print(stdout);
		}
		printf(ENDL "%s - %s" ENDL, Jupiter::version, Jupiter::copyright);
		puts("For command-specific help, use: help <command>");
	}
	else
	{
		Jupiter::ReferenceString command = Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE);
		ConsoleCommand *cmd = getConsoleCommand(command);
		if (cmd != nullptr)
			cmd->getHelp(Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE)).println(stdout);
		else
			printf("Error: Command \"%.*s\" not found." ENDL, command.size(), command.ptr());
	}
}

const Jupiter::ReadableString &HelpConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Lists commands, or sends command-specific help. Syntax: help [command]");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(HelpConsoleCommand)

// Help IRC Command.

void HelpIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("help"));
}

void HelpIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	int cIndex = source->getChannelIndex(channel);
	if (cIndex >= 0)
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(cIndex);
		int access = source->getAccessLevel(channel, nick);
		if (parameters == nullptr)
		{
			for (int i = 0; i <= access; i++)
			{
				Jupiter::ArrayList<IRCCommand> cmds = source->getAccessCommands(chan, i);
				if (cmds.size() != 0)
				{
					Jupiter::StringL &triggers = source->getTriggers(cmds);
					if (triggers.size() >= 0)
						source->sendNotice(nick, Jupiter::StringS::Format("Access level %d commands: %.*s", i, triggers.size(), triggers.ptr()));
				}
			}
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("For command-specific help, use: help <command>"));
		}
		else
		{
			IRCCommand *cmd = source->getCommand(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE));
			if (cmd)
			{
				if (access >= cmd->getAccessLevel(chan))
					source->sendNotice(nick, cmd->getHelp(Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE)));
				else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Access Denied."));
			}
			else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Command not found."));
		}
	}
}

const Jupiter::ReadableString &HelpIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Syntax: help [command]");
	return defaultHelp;
}

IRC_COMMAND_INIT(HelpIRCCommand)

// Version Command

VersionGenericCommand::VersionGenericCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("version"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("versioninfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("copyright"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("copyrightinfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("client"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("clientinfo"));
}

GenericCommand::ResponseLine *VersionGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	GenericCommand::ResponseLine *ret = new GenericCommand::ResponseLine("Version: "_jrs + Jupiter::ReferenceString(Jupiter::version), GenericCommand::DisplayType::PublicSuccess);
	ret->next = new GenericCommand::ResponseLine(Jupiter::ReferenceString(Jupiter::copyright), GenericCommand::DisplayType::PublicSuccess);
	return ret;
}

const Jupiter::ReadableString &VersionGenericCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays version and copyright information");
	return defaultHelp;
}

GENERIC_COMMAND_INIT(VersionGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(VersionGenericCommand)
GENERIC_COMMAND_AS_IRC_COMMAND(VersionGenericCommand)

// Sync Command

SyncGenericCommand::SyncGenericCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("sync"));
}

GenericCommand::ResponseLine *SyncGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (Jupiter::IRC::Client::Config == nullptr)
		return new GenericCommand::ResponseLine(STRING_LITERAL_AS_REFERENCE("Unable to find Config data."), GenericCommand::DisplayType::PublicError);
	else
	{
		bool r;
		if (parameters.isNotEmpty())
			r = Jupiter::IRC::Client::Config->sync(parameters);
		else r = Jupiter::IRC::Client::Config->sync();
		if (r)
			return new GenericCommand::ResponseLine(STRING_LITERAL_AS_REFERENCE("Config data synced to file successfully."), GenericCommand::DisplayType::PublicSuccess);
		return new GenericCommand::ResponseLine(STRING_LITERAL_AS_REFERENCE("Unable to sync Config data."), GenericCommand::DisplayType::PublicError);
	}
}

const Jupiter::ReadableString &SyncGenericCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Syncs the configuration data to a file. Syntax: sync [file]");
	return defaultHelp;
}

GENERIC_COMMAND_INIT(SyncGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(SyncGenericCommand)
GENERIC_COMMAND_AS_IRC_COMMAND_2(SyncGenericCommand, 4)

// Rehash Command

RehashGenericCommand::RehashGenericCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rehash"));
}

GenericCommand::ResponseLine *RehashGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (Jupiter::IRC::Client::Config == nullptr)
		return new GenericCommand::ResponseLine(STRING_LITERAL_AS_REFERENCE("Unable to find Config data."), GenericCommand::DisplayType::PublicError);
	else
	{
		unsigned int r = Jupiter::rehash();
		if (r == 0)
			return new GenericCommand::ResponseLine(Jupiter::StringS::Format("All %u objects were successfully rehashed.", Jupiter::getRehashableCount()), GenericCommand::DisplayType::PublicSuccess);
		return new GenericCommand::ResponseLine(Jupiter::StringS::Format("%u of %u objects failed to successfully rehash.", r, Jupiter::getRehashableCount()), GenericCommand::DisplayType::PublicError);
	}
}

const Jupiter::ReadableString &RehashGenericCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Rehashes configuration data from a file. Syntax: rehash [file]");
	return defaultHelp;
}

GENERIC_COMMAND_INIT(RehashGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(RehashGenericCommand)
GENERIC_COMMAND_AS_IRC_COMMAND_2(RehashGenericCommand, 4)

// Plugin instantiation and entry point.
CoreCommandsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
