/**
 * Copyright (C) 2014-2015 Jessica James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
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

GENERIC_GENERIC_COMMAND(SelectGenericCommand)
GENERIC_GENERIC_COMMAND(DeselectGenericCommand)
GENERIC_GENERIC_COMMAND(RawGenericCommand)
GENERIC_GENERIC_COMMAND(IRCMessageGenericCommand)
GENERIC_GENERIC_COMMAND(JoinGenericCommand)
GENERIC_GENERIC_COMMAND(PartGenericCommand)
GENERIC_GENERIC_COMMAND(DebugInfoGenericCommand)
GENERIC_GENERIC_COMMAND(ExitGenericCommand)
GENERIC_GENERIC_COMMAND(IRCConnectGenericCommand)
GENERIC_GENERIC_COMMAND(IRCDisconnectGenericCommand)

#endif // _EXTRACOMMANDS_H_HEADER