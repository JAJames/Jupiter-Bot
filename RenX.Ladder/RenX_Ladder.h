/**
 * Copyright (C) 2015-2017 Jessica James.
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

#if !defined _RENX_LADDER_H_HEADER
#define _RENX_LADDER_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "IRC_Command.h"
#include "RenX_Plugin.h"
#include "RenX_LadderDatabase.h"
#include "RenX_GameCommand.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

class RenX_LadderPlugin : public RenX::Plugin
{
public:
	virtual bool initialize() override;
	void RenX_OnServerFullyConnected(RenX::Server &server) override;
	void RenX_OnGameOver(RenX::Server &server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore) override;
	void RenX_OnCommand(RenX::Server &server, const Jupiter::ReadableString &) override;

	size_t getMaxLadderCommandPartNameOutput() const;

private:
	/** Configuration variables */
	bool only_pure = false;
	size_t max_ladder_command_part_name_output = 0;
};

GENERIC_GENERIC_COMMAND(LadderGenericCommand)
GENERIC_GAME_COMMAND(LadderGameCommand)

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_LADDER_H_HEADER