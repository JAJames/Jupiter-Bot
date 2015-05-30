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

// Version Console Command

VersionConsoleCommand::VersionConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("version"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("versioninfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("copyright"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("copyrightinfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("client"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("clientinfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("info"));
}

void VersionConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	printf("Version: %s" ENDL "%s" ENDL, Jupiter::version, Jupiter::copyright);
}

const Jupiter::ReadableString &VersionConsoleCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays version and copyright information");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(VersionConsoleCommand)

// Help Command.

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

// Version IRC Command

void VersionIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("version"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("versioninfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("copyright"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("copyrightinfo"));
}

void VersionIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(channel, Jupiter::StringS::Format("Version: %s", Jupiter::version));
	source->sendMessage(channel, Jupiter::ReferenceString(Jupiter::copyright));
}

const Jupiter::ReadableString &VersionIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays version and copyright information. Syntax: version");
	return defaultHelp;
}

IRC_COMMAND_INIT(VersionIRCCommand)

// Sync Command

void SyncIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("sync"));
	this->setAccessLevel(4);
}

void SyncIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (source->Config == nullptr) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Unable to find Config data."));
	else
	{
		bool r;
		if (parameters.isNotEmpty())
			r = source->Config->sync(parameters);
		else r = source->Config->sync();
		if (r) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Config data synced to file successfully."));
		else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Unable to sync Config data."));
	}
}

const Jupiter::ReadableString &SyncIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Syncs the configuration data to a file. Syntax: sync [file]");
	return defaultHelp;
}

IRC_COMMAND_INIT(SyncIRCCommand)

// Rehash Command

void RehashIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rehash"));
	this->setAccessLevel(4);
}

void RehashIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (source->Config == nullptr) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Unable to find Config data."));
	else
	{
		unsigned int r = Jupiter::rehash();
		if (r == 0)
			source->sendMessage(channel, Jupiter::StringS::Format("All %u objects were successfully rehashed.", Jupiter::getRehashableCount()));
		else source->sendMessage(channel, Jupiter::StringS::Format("%u of %u objects failed to successfully rehash.", r, Jupiter::getRehashableCount()));
	}
}

const Jupiter::ReadableString &RehashIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Rehashes configuration data from a file. Syntax: rehash [file]");
	return defaultHelp;
}

IRC_COMMAND_INIT(RehashIRCCommand)

// Plugin instantiation and entry point.
CoreCommandsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
