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

#include <ctime>
#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "ExcessiveHeadshots.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"

RenX_ExcessiveHeadshotsPlugin::RenX_ExcessiveHeadshotsPlugin()
{
	RenX_ExcessiveHeadshotsPlugin::OnRehash();
}

int RenX_ExcessiveHeadshotsPlugin::OnRehash()
{
	RenX_ExcessiveHeadshotsPlugin::ratio = Jupiter::IRC::Client::Config->getDouble(RenX_ExcessiveHeadshotsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("HeadshotKillRatio"), 0.5);
	RenX_ExcessiveHeadshotsPlugin::minKills = Jupiter::IRC::Client::Config->getInt(RenX_ExcessiveHeadshotsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("Kills"), 10);
	RenX_ExcessiveHeadshotsPlugin::minKD = Jupiter::IRC::Client::Config->getDouble(RenX_ExcessiveHeadshotsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("KillDeathRatio"), 5.0);
	RenX_ExcessiveHeadshotsPlugin::minKPS = Jupiter::IRC::Client::Config->getDouble(RenX_ExcessiveHeadshotsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("KillsPerSecond"), 0.5);
	RenX_ExcessiveHeadshotsPlugin::minFlags = Jupiter::IRC::Client::Config->getInt(RenX_ExcessiveHeadshotsPlugin::getName(), STRING_LITERAL_AS_REFERENCE("Flags"), 4);
	return 0;
}

void RenX_ExcessiveHeadshotsPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	if (player->kills < 3) return;
	if (damageType.equals("Rx_DmgType_Headshot"))
	{
		unsigned int flags = 0;
		if (player->kills >= RenX_ExcessiveHeadshotsPlugin::minKills) flags++;
		if (RenX::getHeadshotKillRatio(player) >= RenX_ExcessiveHeadshotsPlugin::ratio) flags++;
		if (RenX::getKillDeathRatio(player) >= RenX_ExcessiveHeadshotsPlugin::minKD) flags++;
		if (RenX::getKillsPerSecond(player) >= RenX_ExcessiveHeadshotsPlugin::minKPS) flags++;
		if (RenX::getKillsPerSecond(player) >= RenX_ExcessiveHeadshotsPlugin::minKPS * 2) flags++;
		if (RenX::getGameTime(player) <= RenX_ExcessiveHeadshotsPlugin::maxGameTime) flags++;

		if (flags >= RenX_ExcessiveHeadshotsPlugin::minFlags)
		{
			server->kickPlayer(player);
			server->sendPubChan(IRCCOLOR "13[Aimbot]" IRCCOLOR " %.*s was banned from the server! Kills: %u - Deaths: %u - Headshots: %u", player->name.size(), player->name.ptr(), player->kills, player->deaths, player->headshots);
			server->sendAdmChan(IRCCOLOR "13[Aimbot]" IRCCOLOR " %.*s was banned from the server! Kills: %u - Deaths: %u - Headshots: %u - IP: " IRCBOLD "%.*s" IRCBOLD " - Steam ID: " IRCBOLD "%.*s" IRCBOLD, player->name.size(), player->name.ptr(), player->kills, player->deaths, player->headshots, player->ip.size(), player->ip.ptr(), player->steamid.size(), player->steamid.ptr());
		}
	}
}


// Plugin instantiation and entry point.
RenX_ExcessiveHeadshotsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
