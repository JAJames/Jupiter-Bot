/**
 * Copyright (C) 2014-2016 Jessica James.
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
	virtual bool initialize() override;
	int OnRehash() override;

	int maxWarns;
	int warnAction; /** -1 = kick; 0 = perm ban; other = temp ban */
};

GENERIC_IRC_COMMAND(WarnIRCCommand)
GENERIC_IRC_COMMAND(PardonIRCCommand)
GENERIC_GAME_COMMAND(WarnGameCommand)
GENERIC_GAME_COMMAND(PardonGameCommand)

#endif // _RENX_WARN_H_HEADER