/**
 * Copyright (C) 2014-2015 Justin James.
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

#include "RenX_Plugin.h"
#include "RenX_Core.h"

RenX::Plugin::Plugin()
{
	RenX::getCore()->getPlugins()->add(this);
}

RenX::Plugin::~Plugin()
{
	for (size_t i = 0; i != RenX::getCore()->getPlugins()->size(); i++)
	{
		if (RenX::getCore()->getPlugins()->get(i) == this)
		{
			RenX::getCore()->getPlugins()->remove(i);
			break;
		}
	}
}

void RenX::Plugin::RenX_SanitizeTags(Jupiter::StringType &)
{
	return;
}

void RenX::Plugin::RenX_ProcessTags(Jupiter::StringType &, const RenX::Server *, const RenX::PlayerInfo *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerCreate(Server *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerDelete(Server *, const RenX::PlayerInfo *)
{
	return;
}

bool RenX::Plugin::RenX_OnBan(Server *, const RenX::PlayerInfo *, Jupiter::StringType &)
{
	return false;
}

void RenX::Plugin::RenX_OnJoin(Server *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnPart(Server *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnNameChange(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTeamChange(Server *, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnChat(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTeamChat(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDeploy(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSuicide(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnKill(Server *, const RenX::PlayerInfo *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnKill(Server *, const Jupiter::ReadableString &, TeamType, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDie(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDie(Server *, const Jupiter::ReadableString &, TeamType, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &, const Jupiter::ReadableString &, ObjectType)
{
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server *, const Jupiter::ReadableString &, TeamType, const Jupiter::ReadableString &, TeamType, const Jupiter::ReadableString &, ObjectType)
{
	return;
}

void RenX::Plugin::RenX_OnGameOver(Server *, RenX::WinType, TeamType, int, int)
{
	return;
}

void RenX::Plugin::RenX_OnGame(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnExecute(Server *, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSubscribe(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRCON(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnAdminLogin(Server *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnAdminGrant(Server *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnAdminLogout(Server *, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnAdmin(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnLog(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_XOnVersion(Server *, unsigned int)
{
	return;
}

void RenX::Plugin::RenX_OnGrantCharacter(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnGrantWeapon(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSpawnVehicle(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSpawnVehicleNoOwner(Server *, const TeamType, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMinePlace(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_XOnOther(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnCommand(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnError(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVersion(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnAuthorized(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnOther(Server *, char, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRaw(Server *, const Jupiter::ReadableString &)
{
	return;
}