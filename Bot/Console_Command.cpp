/**
 * Copyright (C) 2013-2014 Justin James.
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

#include "Console_Command.h"

Jupiter::ArrayList<ConsoleCommand> _consoleCommands;
Jupiter::ArrayList<ConsoleCommand> *consoleCommands = &_consoleCommands;

ConsoleCommand::ConsoleCommand()
{
	consoleCommands->add(this);
}

ConsoleCommand::~ConsoleCommand()
{
	for (size_t i = 0; i != consoleCommands->size(); i++)
	{
		if (consoleCommands->get(i) == this)
		{
			consoleCommands->remove(i);
			break;
		}
	}
}

ConsoleCommand *getConsoleCommand(const Jupiter::ReadableString &trigger)
{
	for (size_t i = 0; i != consoleCommands->size(); i++)
	{
		ConsoleCommand *cmd = consoleCommands->get(i);
		if (cmd->matches(trigger))
			return cmd;
	}
	return nullptr;
}