/**
 * Copyright (C) 2015 Jessica James.
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

class RenX_LadderPlugin : public RenX::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }
	void RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore) override;
	void RenX_OnCommand(RenX::Server *server, const Jupiter::ReadableString &) override;

	size_t getMaxLadderCommandPartNameOutput() const;
	RenX_LadderPlugin();

	RenX_LadderDatabase database;
private:
	void updateLadder(RenX::Server *server, const RenX::TeamType &team);

	/** Configuration variables */
	bool only_pure, output_times;
	size_t max_ladder_command_part_name_output;
	Jupiter::CStringS db_filename;
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Ladder");
};

GENERIC_GENERIC_COMMAND(LadderGenericCommand)
GENERIC_GAME_COMMAND(LadderGameCommand)

#endif // _RENX_LADDER_H_HEADER