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

#include "Jupiter/IRC_Client.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_MinPlayers.h"

using namespace Jupiter::literals;

bool RenX_MinPlayersPlugin::initialize()
{
	RenX_MinPlayersPlugin::player_threshold = this->config.get<size_t>("PlayerThreshold"_jrs, 20);
	return true;
}

void RenX_MinPlayersPlugin::RenX_OnMapStart(RenX::Server &server, const Jupiter::ReadableString &map)
{
	if (server.players.size() < RenX_MinPlayersPlugin::player_threshold)
		server.send(Jupiter::StringS::Format("addbots %d", RenX_MinPlayersPlugin::player_threshold - server.players.size()));
}

void RenX_MinPlayersPlugin::RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player)
{
	if (server.players.size() > RenX_MinPlayersPlugin::player_threshold && server.isMatchInProgress())
		++RenX_MinPlayersPlugin::phase_bots;
}

void RenX_MinPlayersPlugin::RenX_OnPart(RenX::Server &server, const RenX::PlayerInfo &player)
{
	if (server.isMatchInProgress() && server.players.size() <= player_threshold)
	{
		switch (player.team)
		{
		case RenX::TeamType::GDI:
			server.send("addredbots 1"_jrs);
			break;
		case RenX::TeamType::Nod:
			server.send("addbluebots 1"_jrs);
			break;
		case RenX::TeamType::None:
			break;
		default:
			server.send("addbots 1"_jrs);
			break;
		}
	}
}

void RenX_MinPlayersPlugin::AnyDeath(RenX::Server &server, const RenX::PlayerInfo &player)
{
	if (RenX_MinPlayersPlugin::phase_bots != 0 && player.isBot && server.players.size() != 0)
	{
		size_t gdi_count = 0, nod_count = 0;
		for (auto node = server.players.begin(); node != server.players.end(); ++node)
		{
			switch (node->team)
			{
			case RenX::TeamType::GDI:
				++gdi_count;
				break;
			case RenX::TeamType::Nod:
				++nod_count;
				break;
			default:
				break;
			}
		}

		if (gdi_count > nod_count)
		{
			if (player.team != RenX::TeamType::Nod)
			{
				server.kickPlayer(player, "Bot Phasing"_jrs);
				--RenX_MinPlayersPlugin::phase_bots;
			}
		}
		else if (nod_count > gdi_count)
		{
			if (player.team != RenX::TeamType::GDI)
			{
				server.kickPlayer(player, "Bot Phasing"_jrs);
				--RenX_MinPlayersPlugin::phase_bots;
			}
		}
		else
		{
			server.kickPlayer(player, "Bot Phasing"_jrs);
			--RenX_MinPlayersPlugin::phase_bots;
		}
	}
}

void RenX_MinPlayersPlugin::RenX_OnSuicide(RenX::Server &server, const RenX::PlayerInfo &player, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, player);
}

void RenX_MinPlayersPlugin::RenX_OnKill(RenX::Server &server, const RenX::PlayerInfo &player, const RenX::PlayerInfo &victim, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, player);
}

void RenX_MinPlayersPlugin::RenX_OnKill(RenX::Server &server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const RenX::PlayerInfo &victim, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, victim);
}

void RenX_MinPlayersPlugin::RenX_OnDie(RenX::Server &server, const RenX::PlayerInfo &player, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, player);
}

// Plugin instantiation and entry point.
RenX_MinPlayersPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
