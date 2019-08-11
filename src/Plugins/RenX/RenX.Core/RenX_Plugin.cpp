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

void RenX::Plugin::RenX_ProcessTags(Jupiter::StringType &, const Server *, const PlayerInfo *, const PlayerInfo *, const BuildingInfo *)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerCreate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerDelete(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerUUIDChange(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerRDNS(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerIdentify(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnServerCreate(Server &)
{
	return;
}

void RenX::Plugin::RenX_OnServerFullyConnected(Server &)
{
	return;
}

void RenX::Plugin::RenX_OnServerDisconnect(Server &, RenX::DisconnectReason)
{
	return;
}

bool RenX::Plugin::RenX_OnBan(Server &, const PlayerInfo &, Jupiter::StringType &)
{
	return false;
}

void RenX::Plugin::RenX_OnJoin(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnPart(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnKick(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnNameChange(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTeamChange(Server &, const PlayerInfo &, const TeamType &)
{
	return;
}

void RenX::Plugin::RenX_OnHWID(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnIDChange(Server &, const PlayerInfo &, int)
{
	return;
}

void RenX::Plugin::RenX_OnDev(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnRank(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnExecute(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayerCommand(Server &, const PlayerInfo &, const Jupiter::ReadableString &, GameCommand *)
{
	return;
}

void RenX::Plugin::RenX_OnSpeedHack(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnPlayer(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnChat(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTeamChat(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRadioChat(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnHostChat(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnHostPage(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnOtherChat(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDeploy(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnOverMine(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDisarm(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDisarm(Server &, const PlayerInfo &, const Jupiter::ReadableString &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnExplode(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnExplode(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSuicide(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnKill(Server &, const PlayerInfo &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnKill(Server &, const Jupiter::ReadableString &, const TeamType &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDie(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDie(Server &, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server &, const PlayerInfo &, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &, ObjectType)
{
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server &, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &, const TeamType &, const Jupiter::ReadableString &, ObjectType)
{
	return;
}

void RenX::Plugin::RenX_OnCapture(Server &, const PlayerInfo &, const Jupiter::ReadableString &, const TeamType &)
{
	return;
}

void RenX::Plugin::RenX_OnNeutralize(Server &, const PlayerInfo &, const Jupiter::ReadableString &, const TeamType &)
{
	return;
}

void RenX::Plugin::RenX_OnCharacterPurchase(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnItemPurchase(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnWeaponPurchase(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRefillPurchase(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVehiclePurchase(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVehicleSpawn(Server &, const TeamType &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSpawn(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnBotJoin(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVehicleCrate(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnTSVehicleCrate(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRAVehicleCrate(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDeathCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnMoneyCrate(Server &, const PlayerInfo &, int)
{
	return;
}

void RenX::Plugin::RenX_OnCharacterCrate(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSpyCrate(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRefillCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnTimeBombCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnSpeedCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnNukeCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnAbductionCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnUnspecifiedCrate(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnOtherCrate(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSteal(Server &, const PlayerInfo &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSteal(Server &, const PlayerInfo &, const Jupiter::ReadableString &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnDonate(Server &, const PlayerInfo &, const PlayerInfo &, double)
{
	return;
}

void RenX::Plugin::RenX_OnGameOver(Server &, RenX::WinType, const TeamType &, int, int)
{
	return;
}

void RenX::Plugin::RenX_OnGame(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnExecute(Server &, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnSubscribe(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnUnsubscribe(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnBlock(Server &, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnConnect(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnAuthenticate(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnBan(Server &, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnInvalidPassword(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDrop(Server &, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDisconnect(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnStopListen(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnResumeListen(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnWarning(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRCON(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnAdminLogin(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnAdminGrant(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnAdminLogout(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnAdmin(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteAddBots(Server &, const TeamType &, const PlayerInfo &, const TeamType &, int, int)
{
	return;
}

void RenX::Plugin::RenX_OnVoteChangeMap(Server &, const TeamType &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteKick(Server &, const TeamType &, const PlayerInfo &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteMineBan(Server &, const TeamType &, const PlayerInfo &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteRemoveBots(Server &, const TeamType &, const PlayerInfo &, const TeamType &, int)
{
	return;
}

void RenX::Plugin::RenX_OnVoteRestartMap(Server &, const TeamType &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteSurrender(Server &, const TeamType &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteSurvey(Server &, const TeamType &, const PlayerInfo &, const Jupiter::ReadableString &text)
{
	return;
}

void RenX::Plugin::RenX_OnVoteOther(Server &, const TeamType &, const Jupiter::ReadableString &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnVoteOver(Server &, const TeamType &, const Jupiter::ReadableString &, bool, int, int)
{
	return;
}

void RenX::Plugin::RenX_OnVoteCancel(Server &, const TeamType &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVote(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMapChange(Server &, const Jupiter::ReadableString &, bool)
{
	return;
}

void RenX::Plugin::RenX_OnMapLoad(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMapStart(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnMap(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDemoRecord(Server &, const PlayerInfo &)
{
	return;
}

void RenX::Plugin::RenX_OnDemoRecord(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnDemoRecordStop(Server &)
{
	return;
}

void RenX::Plugin::RenX_OnDemo(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnLog(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnCommand(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnError(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnVersion(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnAuthorized(Server &, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnOther(Server &, char, const Jupiter::ReadableString &)
{
	return;
}

void RenX::Plugin::RenX_OnRaw(Server &, const Jupiter::ReadableString &)
{
	return;
}