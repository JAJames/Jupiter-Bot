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

#if !defined _CORECOMMANDS_H_HEADER
#define _CORECOMMANDS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Console_Command.h"
#include "IRC_Command.h"

class CoreCommandsPlugin : public Jupiter::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "CoreCommands");
};

GENERIC_CONSOLE_COMMAND(HelpConsoleCommand)
GENERIC_IRC_COMMAND(HelpIRCCommand)
GENERIC_GENERIC_COMMAND(VersionGenericCommand)
GENERIC_GENERIC_COMMAND(SyncGenericCommand)
GENERIC_GENERIC_COMMAND(RehashGenericCommand)

#endif // _CORECOMMANDS_H_HEADER