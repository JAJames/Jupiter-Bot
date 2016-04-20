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

#if !defined _RENX_SETJOIN_H_HEADER
#define _RENX_SETJOIN_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/INIFile.h"
#include "RenX_Plugin.h"
#include "RenX_GameCommand.h"

class RenX_SetJoinPlugin : public RenX::Plugin
{
public:
	Jupiter::INIFile setjoin_file;

	RenX_SetJoinPlugin();

public: // RenX::Plugin
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.SetJoin");
};

GENERIC_GAME_COMMAND(SetJoinGameCommand)
GENERIC_GAME_COMMAND(ViewJoinGameCommand)
GENERIC_GAME_COMMAND(ShowJoinGameCommand)
GENERIC_GAME_COMMAND(DelJoinGameCommand)

#endif // _RENX_SETJOIN_H_HEADER