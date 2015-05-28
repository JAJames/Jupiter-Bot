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

void RenX::Plugin::RenX_OnPlayerUUIDChange(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnServerCreate(Server *)
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

void RenX::Plugin::RenX_OnKick(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &reason)
{
	return;
}

void RenX::Plugin::RenX_OnNameChange(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTeamChange(Server *, const PlayerInfo *, const TeamType &)
{
	return;
}

void RenX::Plugin::RenX_OnIDChange(Server *, const PlayerInfo *, int)
{
	return;
}

void RenX::Plugin::RenX_OnExecute(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayer(Server *, const Jupiter::ReadableString &)
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

void RenX::Plugin::RenX_OnHostChat(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnHostPage(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnOtherChat(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDeploy(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDisarm(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDisarm(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &, const RenX::PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnExplode(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnExplode(Server *, const Jupiter::ReadableString &)
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

void RenX::Plugin::RenX_OnKill(Server *, const Jupiter::ReadableString &, const TeamType &, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDie(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDie(Server *, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server *, const RenX::PlayerInfo *, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &, ObjectType)
{
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server *, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &, ObjectType)
{
	return;
}

void RenX::Plugin::RenX_OnCapture(Server *, const PlayerInfo *, const Jupiter::ReadableString &, const TeamType &)
{
	return;
}

void RenX::Plugin::RenX_OnNeutralize(Server *, const PlayerInfo *, const Jupiter::ReadableString &, const TeamType &)
{
	return;
}

void RenX::Plugin::RenX_OnCharacterPurchase(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnItemPurchase(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnWeaponPurchase(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRefillPurchase(Server *, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnVehiclePurchase(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVehicleSpawn(Server *, const TeamType &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSpawn(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnBotJoin(Server *, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnVehicleCrate(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTSVehicleCrate(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRAVehicleCrate(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDeathCrate(Server *, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnMoneyCrate(Server *, const PlayerInfo *, int)
{
	return;
}

void RenX::Plugin::RenX_OnCharacterCrate(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSpyCrate(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRefillCrate(Server *, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnTimeBombCrate(Server *server, const PlayerInfo *player)
{
	return;
}

void RenX::Plugin::RenX_OnSpeedCrate(Server *server, const PlayerInfo *player)
{
	return;
}

void RenX::Plugin::RenX_OnNukeCrate(Server *server, const PlayerInfo *player)
{
	return;
}

void RenX::Plugin::RenX_OnAbductionCrate(Server *server, const PlayerInfo *player)
{
	return;
}

void RenX::Plugin::RenX_OnUnspecifiedCrate(Server *server, const PlayerInfo *player)
{
	return;
}

void RenX::Plugin::RenX_OnOtherCrate(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSteal(Server *, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSteal(Server *, const PlayerInfo *, const Jupiter::ReadableString &, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnDonate(Server *, const PlayerInfo *, const PlayerInfo *, double)
{
	return;
}

void RenX::Plugin::RenX_OnGameOver(Server *, RenX::WinType, const TeamType &, int, int)
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

void RenX::Plugin::RenX_OnUnsubscribe(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnBlock(Server *, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnConnect(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnAuthenticate(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnBan(Server *, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnInvalidPassword(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDrop(Server *, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDisconnect(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnStopListen(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnResumeListen(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnWarning(Server *, const Jupiter::ReadableString &)
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

void RenX::Plugin::RenX_OnVoteCall(Server *, const TeamType &, const Jupiter::ReadableString &, const PlayerInfo *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteOver(Server *, const TeamType &, const Jupiter::ReadableString &, bool, int, int)
{
	return;
}

void RenX::Plugin::RenX_OnVoteCancel(Server *, const TeamType &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVote(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMapChange(Server *, const Jupiter::ReadableString &, bool)
{
	return;
}

void RenX::Plugin::RenX_OnMapLoad(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMapStart(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMap(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDemoRecord(Server *, const PlayerInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnDemoRecord(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDemoRecordStop(Server *)
{
	return;
}

void RenX::Plugin::RenX_OnDemo(Server *, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnLog(Server *, const Jupiter::ReadableString &)
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