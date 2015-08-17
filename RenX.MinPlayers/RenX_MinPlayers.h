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

#if !defined _RENX_MINPLAYERS_H_HEADER
#define _RENX_MINPLAYERS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "RenX_Plugin.h"

class RenX_MinPlayersPlugin : public RenX::Plugin
{
public:
	void RenX_OnMapStart(RenX::Server *server, const Jupiter::ReadableString &map) override;
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player) override;

	void RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;

	const Jupiter::ReadableString &getName() override { return name; }

	RenX_MinPlayersPlugin();

private:
	void AnyDeath(RenX::Server *server, const RenX::PlayerInfo *player);
	size_t phase_bots;

	/** Configuration variables */
	size_t player_threshold;
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.MinPlayers");
};

#endif // _RENX_MINPLAYERS_H_HEADER