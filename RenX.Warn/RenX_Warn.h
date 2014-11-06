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

#if !defined _RENX_WARN_H_HEADER
#define _RENX_WARN_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "IRC_Command.h"
#include "RenX_Plugin.h"
#include "RenX_GameCommand.h"

class RenX_WarnPlugin : public RenX::Plugin
{
public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;
	RenX_WarnPlugin();

	int maxWarns;
	int warnAction; /** -1 = kick; 0 = perm ban; other = temp ban */
private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX_TemplatePlugin");
};

GENERIC_IRC_COMMAND(WarnIRCCommand)
GENERIC_IRC_COMMAND(PardonIRCCommand)
GENERIC_GAME_COMMAND(WarnGameCommand)
GENERIC_GAME_COMMAND(PardonGameCommand)

#endif // _RENX_WARN_H_HEADER