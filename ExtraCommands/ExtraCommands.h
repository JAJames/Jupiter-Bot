/**
 * Copyright (C) 2014 Justin James.
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

#if !defined _FUNCOMMANDS_H_HEADER
#define _FUNCOMMANDS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Console_Command.h"
#include "IRC_Command.h"

class FunCommandsPlugin : public Jupiter::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "ExtraCommands");
};

GENERIC_CONSOLE_COMMAND(RawConsoleCommand)
GENERIC_CONSOLE_COMMAND(MessageConsoleCommand)
GENERIC_IRC_COMMAND(JoinIRCCommand)
GENERIC_IRC_COMMAND(PartIRCCommand)
GENERIC_IRC_COMMAND(InfoIRCCommand)
GENERIC_IRC_COMMAND(ExitIRCCommand)
GENERIC_IRC_COMMAND(IRCConnectIRCCommand)
GENERIC_IRC_COMMAND(IRCDisconnectIRCCommand)

#endif // _EXTRACOMMANDS_H_HEADER