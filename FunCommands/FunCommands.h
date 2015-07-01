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

#if !defined _FUNCOMMANDS_H_HEADER
#define _FUNCOMMANDS_H_HEADER

#include "Jupiter/Plugin.h"
#include "IRC_Command.h"

class ExtraCommandsPlugin : public Jupiter::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "FunCommands");
};

GENERIC_IRC_COMMAND(EightBallIRCCommand)
GENERIC_GENERIC_COMMAND(ResolveGenericCommand)

#endif // _FUNCOMMANDS_H_HEADER