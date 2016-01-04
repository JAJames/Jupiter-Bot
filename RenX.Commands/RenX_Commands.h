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

#if !defined _RENX_COMMANDS_H_HEADER
#define _RENX_COMMANDS_H_HEADER

#include "Console_Command.h"
#include "IRC_Command.h"
#include "RenX_GameCommand.h"
#include "RenX_Plugin.h"

class RenX_CommandsPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;

public:
	time_t getTBanTime() const;
	const Jupiter::ReadableString &getPlayerInfoFormat() const;
	const Jupiter::ReadableString &getAdminPlayerInfoFormat() const;
	const Jupiter::ReadableString &getBuildingInfoFormat() const;
	RenX_CommandsPlugin();

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Commands");
	time_t _defaultTempBanTime;
	Jupiter::StringS playerInfoFormat;
	Jupiter::StringS adminPlayerInfoFormat;
	Jupiter::StringS buildingInfoFormat;
};

GENERIC_CONSOLE_COMMAND(RawRCONConsoleCommand)
GENERIC_CONSOLE_COMMAND(RCONConsoleCommand)
//GENERIC_CONSOLE_COMMAND(RCONSelectConsoleCommand)

GENERIC_IRC_COMMAND(MsgIRCCommand)
GENERIC_IRC_COMMAND(PMsgIRCCommand)
GENERIC_IRC_COMMAND(HostMsgIRCCommand)
GENERIC_IRC_COMMAND(PlayersIRCCommand)
GENERIC_IRC_COMMAND(PlayerTableIRCCommand)
GENERIC_IRC_COMMAND(PlayerInfoIRCCommand)
GENERIC_IRC_COMMAND(BuildingInfoIRCCommand)
GENERIC_IRC_COMMAND(MutatorsIRCCommand)
GENERIC_IRC_COMMAND(RotationIRCCommand)
GENERIC_IRC_COMMAND(MapIRCCommand)
GENERIC_IRC_COMMAND(SteamIRCCommand)
GENERIC_IRC_COMMAND(KillDeathRatioIRCCommand)
GENERIC_IRC_COMMAND(ShowModsIRCCommand)
GENERIC_IRC_COMMAND(ModsIRCCommand)
GENERIC_IRC_COMMAND(BanSearchIRCCommand)
GENERIC_IRC_COMMAND(ShowRulesIRCCommand)
GENERIC_IRC_COMMAND(RulesIRCCommand)
GENERIC_IRC_COMMAND(SetRulesIRCCommand)
GENERIC_IRC_COMMAND(ReconnectIRCCommand)
GENERIC_IRC_COMMAND(GameOverIRCCommand)
GENERIC_IRC_COMMAND(SetMapIRCCommand)
GENERIC_IRC_COMMAND(MuteIRCCommand)
GENERIC_IRC_COMMAND(UnMuteIRCCommand)
GENERIC_IRC_COMMAND(KillIRCCommand)
GENERIC_IRC_COMMAND(DisarmIRCCommand)
GENERIC_IRC_COMMAND(DisarmC4IRCCommand)
GENERIC_IRC_COMMAND(DisarmBeaconIRCCommand)
GENERIC_IRC_COMMAND(MineBanIRCCommand)
GENERIC_IRC_COMMAND(KickIRCCommand)
GENERIC_IRC_COMMAND(TempBanIRCCommand)
GENERIC_IRC_COMMAND(KickBanIRCCommand)
GENERIC_IRC_COMMAND(AddBanIRCCommand)
GENERIC_IRC_COMMAND(UnBanIRCCommand)
GENERIC_IRC_COMMAND(AddBotsIRCCommand)
GENERIC_IRC_COMMAND(KillBotsIRCCommand)
GENERIC_IRC_COMMAND(PhaseBotsIRCCommand)
GENERIC_IRC_COMMAND(RCONIRCCommand)
GENERIC_IRC_COMMAND(RefundIRCCommand)
GENERIC_IRC_COMMAND(TeamChangeIRCCommand)
GENERIC_IRC_COMMAND(TeamChange2IRCCommand)

GENERIC_GAME_COMMAND(HelpGameCommand)
GENERIC_GAME_COMMAND(ModsGameCommand)
GENERIC_GAME_COMMAND(RulesGameCommand)
GENERIC_GAME_COMMAND(ModRequestGameCommand)
GENERIC_GAME_COMMAND(KillGameCommand)
GENERIC_GAME_COMMAND(DisarmGameCommand)
GENERIC_GAME_COMMAND(DisarmC4GameCommand)
GENERIC_GAME_COMMAND(DisarmBeaconGameCommand)
GENERIC_GAME_COMMAND(MineBanGameCommand)
GENERIC_GAME_COMMAND(KickGameCommand)
GENERIC_GAME_COMMAND(TempBanGameCommand)
GENERIC_GAME_COMMAND(KickBanGameCommand)
GENERIC_GAME_COMMAND(AddBotsGameCommand)
GENERIC_GAME_COMMAND(KillBotsGameCommand)
GENERIC_GAME_COMMAND(PhaseBotsGameCommand)

#endif // _RENX_COMMANDS_H_HEADER