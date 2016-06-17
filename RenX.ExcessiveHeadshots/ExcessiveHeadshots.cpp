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

#include <ctime>
#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "ExcessiveHeadshots.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"

using namespace Jupiter::literals;

bool RenX_ExcessiveHeadshotsPlugin::initialize()
{
	RenX_ExcessiveHeadshotsPlugin::ratio = this->config.getDouble(Jupiter::ReferenceString::empty, "HeadshotKillRatio"_jrs, 0.5);
	RenX_ExcessiveHeadshotsPlugin::minKills = this->config.getInt(Jupiter::ReferenceString::empty, "Kills"_jrs, 10);
	RenX_ExcessiveHeadshotsPlugin::minKD = this->config.getDouble(Jupiter::ReferenceString::empty, "KillDeathRatio"_jrs, 5.0);
	RenX_ExcessiveHeadshotsPlugin::minKPS = this->config.getDouble(Jupiter::ReferenceString::empty, "KillsPerSecond"_jrs, 0.5);
	RenX_ExcessiveHeadshotsPlugin::minFlags = this->config.getInt(Jupiter::ReferenceString::empty, "Flags"_jrs, 4);
	return true;
}

int RenX_ExcessiveHeadshotsPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();
	return this->initialize() ? 0 : -1;
}

void RenX_ExcessiveHeadshotsPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	if (player->kills < 3) return;
	if (damageType.equals("Rx_DmgType_Headshot"_jrs))
	{
		unsigned int flags = 0;
		std::chrono::milliseconds game_time = server->getGameTime(player);
		double kps = game_time == std::chrono::milliseconds::zero() ? static_cast<double>(player->kills) : static_cast<double>(player->kills) / static_cast<double>(game_time.count());
		if (player->kills >= RenX_ExcessiveHeadshotsPlugin::minKills) flags++;
		if (RenX::getHeadshotKillRatio(player) >= RenX_ExcessiveHeadshotsPlugin::ratio) flags++;
		if (RenX::getKillDeathRatio(player) >= RenX_ExcessiveHeadshotsPlugin::minKD) flags++;
		if (kps >= RenX_ExcessiveHeadshotsPlugin::minKPS) flags++;
		if (kps >= RenX_ExcessiveHeadshotsPlugin::minKPS * 2) flags++;
		if (game_time <= RenX_ExcessiveHeadshotsPlugin::maxGameTime) flags++;

		if (flags >= RenX_ExcessiveHeadshotsPlugin::minFlags)
		{
			server->banPlayer(player, "Jupiter Bot"_jrs, "Aimbot detected"_jrs);
			server->sendPubChan(IRCCOLOR "13[Aimbot]" IRCCOLOR " %.*s was banned from the server! Kills: %u - Deaths: %u - Headshots: %u", player->name.size(), player->name.ptr(), player->kills, player->deaths, player->headshots);
			const Jupiter::ReadableString &steamid = server->formatSteamID(player);
			server->sendAdmChan(IRCCOLOR "13[Aimbot]" IRCCOLOR " %.*s was banned from the server! Kills: %u - Deaths: %u - Headshots: %u - IP: " IRCBOLD "%.*s" IRCBOLD " - Steam ID: " IRCBOLD "%.*s" IRCBOLD, player->name.size(), player->name.ptr(), player->kills, player->deaths, player->headshots, player->ip.size(), player->ip.ptr(), steamid.size(), steamid.ptr());
		}
	}
}


// Plugin instantiation and entry point.
RenX_ExcessiveHeadshotsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
