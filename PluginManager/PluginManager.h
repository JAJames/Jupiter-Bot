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

#if !defined _PLUGINMANAGER_H_HEADER
#define _PLUGINMANAGER_H_HEADER

#include "Jupiter/Plugin.h"
#include "Generic_Command.h"
#include "Console_Command.h"
#include "IRC_Command.h"

class PluginManager : public Jupiter::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "PluginManager");
};

GENERIC_GENERIC_COMMAND(PluginGenericCommand)

#endif // _PLUGINMANAGER_H_HEADER