/**
 * Copyright (C) 2014-2017 Jessica James.
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
#include "RenX_ExcessiveHeadshots.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"

using namespace std::literals;

bool RenX_ExcessiveHeadshotsPlugin::initialize() {
	RenX_ExcessiveHeadshotsPlugin::ratio = this->config.get<double>("HeadshotKillRatio"sv, 0.5);
	RenX_ExcessiveHeadshotsPlugin::minKills = this->config.get<unsigned int>("Kills"sv, 10);
	RenX_ExcessiveHeadshotsPlugin::minKD = this->config.get<double>("KillDeathRatio"sv, 5.0);
	RenX_ExcessiveHeadshotsPlugin::minKPS = this->config.get<double>("KillsPerSecond"sv, 0.5);
	RenX_ExcessiveHeadshotsPlugin::minFlags = this->config.get<unsigned int>("Flags"sv, 4);
	return true;
}

int RenX_ExcessiveHeadshotsPlugin::OnRehash() {
	RenX::Plugin::OnRehash();
	return this->initialize() ? 0 : -1;
}

void RenX_ExcessiveHeadshotsPlugin::RenX_OnKill(RenX::Server &server, const RenX::PlayerInfo &player, const RenX::PlayerInfo &victim, std::string_view damageType) {
	if (player.kills < 3)
		return;

	if (damageType == "Rx_DmgType_Headshot"sv) {
		unsigned int flags = 0;
		std::chrono::milliseconds game_time = server.getGameTime(player);
		double kps = game_time == std::chrono::milliseconds::zero() ? static_cast<double>(player.kills) : static_cast<double>(player.kills) / static_cast<double>(game_time.count());
		if (player.kills >= RenX_ExcessiveHeadshotsPlugin::minKills) flags++;
		if (RenX::getHeadshotKillRatio(player) >= RenX_ExcessiveHeadshotsPlugin::ratio) flags++;
		if (RenX::getKillDeathRatio(player) >= RenX_ExcessiveHeadshotsPlugin::minKD) flags++;
		if (kps >= RenX_ExcessiveHeadshotsPlugin::minKPS) flags++;
		if (kps >= RenX_ExcessiveHeadshotsPlugin::minKPS * 2) flags++;
		if (game_time <= RenX_ExcessiveHeadshotsPlugin::maxGameTime) flags++;

		if (flags >= RenX_ExcessiveHeadshotsPlugin::minFlags)
		{
			server.banPlayer(player, "Jupiter Bot"sv, "Aimbot detected"sv);
			server.sendPubChan(IRCCOLOR "13[Aimbot]" IRCCOLOR " %.*s was banned from the server! Kills: %u - Deaths: %u - Headshots: %u", player.name.size(), player.name.data(), player.kills, player.deaths, player.headshots);
			std::string_view steamid = server.formatSteamID(player);
			server.sendAdmChan(IRCCOLOR "13[Aimbot]" IRCCOLOR " %.*s was banned from the server! Kills: %u - Deaths: %u - Headshots: %u - IP: " IRCBOLD "%.*s" IRCBOLD " - Steam ID: " IRCBOLD "%.*s" IRCBOLD, player.name.size(), player.name.data(), player.kills, player.deaths, player.headshots, player.ip.size(), player.ip.data(), steamid.size(),
				steamid.data());
		}
	}
}


// Plugin instantiation and entry point.
RenX_ExcessiveHeadshotsPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
