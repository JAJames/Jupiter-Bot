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

#if !defined _SETJOIN_H_HEADER
#define _SETJOIN_H_HEADER

#include "Jupiter/Plugin.h"
#include "IRC_Command.h"

class SetJoinPlugin : public Jupiter::Plugin
{
public:
	void OnJoin(Jupiter::IRC::Client *server, const Jupiter::StringType &chan, const Jupiter::StringType &nick);
	const Jupiter::ReadableString &getName() override { return name; }

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "SetJoin");
};

GENERIC_IRC_COMMAND(SetJoinIRCCommand)
GENERIC_IRC_COMMAND(ViewJoinIRCCommand)
GENERIC_IRC_COMMAND(DelJoinIRCCommand)

#endif // _SETJOIN_H_HEADER