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

#if !defined _RENX_COMMANDS_H_HEADER
#define _RENX_COMMANDS_H_HEADER

#include "Console_Command.h"
#include "IRC_Command.h"
#include "RenX_GameCommand.h"
#include "RenX_Plugin.h"

class RenX_CommandsPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	bool RenX_OnBan(RenX::Server *server, const RenX::PlayerInfo *player, Jupiter::StringType &data);

	void RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;

public:
	time_t getTBanTime();
	RenX_CommandsPlugin();

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Commands");
	time_t _defaultTempBanTime;
};

GENERIC_CONSOLE_COMMAND(RCONConsoleCommand)
//GENERIC_CONSOLE_COMMAND(RCONSelectConsoleCommand)

GENERIC_IRC_COMMAND(MsgIRCCommand)
GENERIC_IRC_COMMAND(PMsgIRCCommand)
GENERIC_IRC_COMMAND(HostMsgIRCCommand)
GENERIC_IRC_COMMAND(PlayersIRCCommand)
GENERIC_IRC_COMMAND(PlayerInfoIRCCommand)
GENERIC_IRC_COMMAND(SteamIRCCommand)
GENERIC_IRC_COMMAND(KillDeathRatioIRCCommand)
GENERIC_IRC_COMMAND(ShowModsIRCCommand)
GENERIC_IRC_COMMAND(ModsIRCCommand)
GENERIC_IRC_COMMAND(BanSearchIRCCommand)
GENERIC_IRC_COMMAND(ShowRulesIRCCommand)
GENERIC_IRC_COMMAND(RulesIRCCommand)
GENERIC_IRC_COMMAND(SetRulesIRCCommand)
GENERIC_IRC_COMMAND(ReconnectIRCCommand)
GENERIC_IRC_COMMAND(RestartMapIRCCommand)
GENERIC_IRC_COMMAND(SetMapIRCCommand)
GENERIC_IRC_COMMAND(MuteIRCCommand)
GENERIC_IRC_COMMAND(UnMuteIRCCommand)
GENERIC_IRC_COMMAND(KickIRCCommand)
GENERIC_IRC_COMMAND(TempBanIRCCommand)
GENERIC_IRC_COMMAND(KickBanIRCCommand)
GENERIC_IRC_COMMAND(UnBanIRCCommand)
GENERIC_IRC_COMMAND(AddBotsIRCCommand)
GENERIC_IRC_COMMAND(KillBotsIRCCommand)
GENERIC_IRC_COMMAND(PhaseBotsIRCCommand)
GENERIC_IRC_COMMAND(RCONIRCCommand)

GENERIC_GAME_COMMAND(HelpGameCommand)
GENERIC_GAME_COMMAND(ModsGameCommand)
GENERIC_GAME_COMMAND(RulesGameCommand)
GENERIC_GAME_COMMAND(ModRequestGameCommand)
GENERIC_GAME_COMMAND(KickGameCommand)
GENERIC_GAME_COMMAND(TempBanGameCommand)
GENERIC_GAME_COMMAND(KickBanGameCommand)

#endif // _RENX_COMMANDS_H_HEADER