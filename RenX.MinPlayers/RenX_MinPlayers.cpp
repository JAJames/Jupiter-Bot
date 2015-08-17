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

#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_MinPlayers.h"

using namespace Jupiter::literals;

RenX_MinPlayersPlugin::RenX_MinPlayersPlugin()
{
	RenX_MinPlayersPlugin::player_threshold = Jupiter::IRC::Client::Config->getInt(this->getName(), "PlayerThreshold"_jrs, 20);
}

void RenX_MinPlayersPlugin::RenX_OnMapStart(RenX::Server *server, const Jupiter::ReadableString &map)
{
	if (server->players.size() < RenX_MinPlayersPlugin::player_threshold)
		server->send(Jupiter::StringS::Format("addbots %d", RenX_MinPlayersPlugin::player_threshold - server->players.size()));
}

void RenX_MinPlayersPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (server->players.size() > RenX_MinPlayersPlugin::player_threshold)
		++RenX_MinPlayersPlugin::phase_bots;
}

void RenX_MinPlayersPlugin::RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (server->players.size() <= player_threshold)
	{
		switch (player->team)
		{
		case RenX::TeamType::GDI:
			server->send("addredbots 1"_jrs);
			break;
		case RenX::TeamType::Nod:
			server->send("addbluebots 1"_jrs);
			break;
		case RenX::TeamType::None:
			break;
		default:
			server->send("addbots 1"_jrs);
			break;
		}
	}
}

void RenX_MinPlayersPlugin::AnyDeath(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (RenX_MinPlayersPlugin::phase_bots != 0 && player->isBot && server->players.size() != 0)
	{
		size_t gdi_count = 0, nod_count = 0;
		for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
		{
			switch (node->data->team)
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
			if (player->team != RenX::TeamType::Nod)
			{
				server->kickPlayer(player, "Bot Phasing"_jrs);
				--RenX_MinPlayersPlugin::phase_bots;
			}
		}
		else if (nod_count > gdi_count)
		{
			if (player->team != RenX::TeamType::GDI)
			{
				server->kickPlayer(player, "Bot Phasing"_jrs);
				--RenX_MinPlayersPlugin::phase_bots;
			}
		}
		else
		{
			server->kickPlayer(player, "Bot Phasing"_jrs);
			--RenX_MinPlayersPlugin::phase_bots;
		}
	}
}

void RenX_MinPlayersPlugin::RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, player);
}

void RenX_MinPlayersPlugin::RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, player);
}

void RenX_MinPlayersPlugin::RenX_OnKill(RenX::Server *server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, victim);
}

void RenX_MinPlayersPlugin::RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType)
{
	this->AnyDeath(server, player);
}

// Plugin instantiation and entry point.
RenX_MinPlayersPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
