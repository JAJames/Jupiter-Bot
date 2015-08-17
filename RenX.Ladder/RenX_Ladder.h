/**
 * Copyright (C) 2015 Justin James.
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