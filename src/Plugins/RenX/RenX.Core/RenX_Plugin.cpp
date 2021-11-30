/**
 * Copyright (C) 2014-2021 Jessica James.
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

RenX::Plugin::Plugin() {
	RenX::getCore()->getPlugins().push_back(this);
}

RenX::Plugin::~Plugin() {
	auto& renx_plugins = RenX::getCore()->getPlugins();
	for (auto itr = renx_plugins.begin(); itr != renx_plugins.end(); ++itr) {
		if (*itr == this) {
			RenX::getCore()->getPlugins().erase(itr);
			break;
		}
	}
}

void RenX::Plugin::RenX_SanitizeTags(std::string&) {
	return;
}

void RenX::Plugin::RenX_ProcessTags(std::string&, const Server *, const PlayerInfo *, const PlayerInfo *, const BuildingInfo *) {
	return;
}

void RenX::Plugin::RenX_OnPlayerCreate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnPlayerDelete(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnPlayerUUIDChange(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnPlayerRDNS(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnPlayerIdentify(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnServerCreate(Server &) {
	return;
}

void RenX::Plugin::RenX_OnServerFullyConnected(Server &) {
	return;
}

void RenX::Plugin::RenX_OnServerDisconnect(Server &, RenX::DisconnectReason) {
	return;
}

bool RenX::Plugin::RenX_OnBan(Server &, const PlayerInfo &, Jupiter::StringType &) {
	return false;
}

void RenX::Plugin::RenX_OnCommandTriggered(Server& server, std::string_view  trigger, RenX::PlayerInfo& player, std::string_view  parameters, GameCommand& command) {
	return;
}

void RenX::Plugin::RenX_OnJoin(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnPart(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnKick(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnNameChange(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnTeamChange(Server &, const PlayerInfo &, const TeamType &) {
	return;
}

void RenX::Plugin::RenX_OnHWID(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnIDChange(Server &, const PlayerInfo &, int) {
	return;
}

void RenX::Plugin::RenX_OnDev(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnRank(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnExecute(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnPlayerCommand(Server &, const PlayerInfo &, std::string_view , GameCommand *) {
	return;
}

void RenX::Plugin::RenX_OnSpeedHack(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnPlayer(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnChat(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnTeamChat(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnRadioChat(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnHostChat(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnHostPage(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnAdminMessage(Server &server, const PlayerInfo &player, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnWarnMessage(Server &server, const PlayerInfo &player, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnAdminPMessage(Server &server, const PlayerInfo &player, const PlayerInfo &target, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnWarnPMessage(Server &server, const PlayerInfo &player, const PlayerInfo &target, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnHostAdminMessage(Server &server, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnHostAdminPMessage(Server &server, const PlayerInfo &player, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnHostWarnMessage(Server &server, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnHostWarnPMessage(Server &server, const PlayerInfo &player, std::string_view message) {
	return;
}

void RenX::Plugin::RenX_OnOtherChat(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDeploy(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnOverMine(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDisarm(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDisarm(Server &, const PlayerInfo &, std::string_view , const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnExplode(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnExplode(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnSuicide(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnKill(Server &, const PlayerInfo &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnKill(Server &, std::string_view , const TeamType &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDie(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDie(Server &, std::string_view , const TeamType &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server &, const PlayerInfo &, std::string_view , const TeamType &, std::string_view , ObjectType) {
	return;
}

void RenX::Plugin::RenX_OnDestroy(Server &, std::string_view , const TeamType &, std::string_view , const TeamType &, std::string_view , ObjectType) {
	return;
}

void RenX::Plugin::RenX_OnCapture(Server &, const PlayerInfo &, std::string_view , const TeamType &) {
	return;
}

void RenX::Plugin::RenX_OnNeutralize(Server &, const PlayerInfo &, std::string_view , const TeamType &) {
	return;
}

void RenX::Plugin::RenX_OnCharacterPurchase(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnItemPurchase(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnWeaponPurchase(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnRefillPurchase(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVehiclePurchase(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnVehicleSpawn(Server &, const TeamType &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnSpawn(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnBotJoin(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVehicleCrate(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnTSVehicleCrate(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnRAVehicleCrate(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDeathCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnMoneyCrate(Server &, const PlayerInfo &, int) {
	return;
}

void RenX::Plugin::RenX_OnCharacterCrate(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnSpyCrate(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnRefillCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnTimeBombCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnSpeedCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnNukeCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnAbductionCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnUnspecifiedCrate(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnOtherCrate(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnSteal(Server &, const PlayerInfo &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnSteal(Server &, const PlayerInfo &, std::string_view , const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnDonate(Server &, const PlayerInfo &, const PlayerInfo &, double) {
	return;
}

void RenX::Plugin::RenX_OnGameOver(Server &, RenX::WinType, const TeamType &, int, int) {
	return;
}

void RenX::Plugin::RenX_OnGame(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnExecute(Server &, std::string_view , std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnSubscribe(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnUnsubscribe(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnBlock(Server &, std::string_view , std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnConnect(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnAuthenticate(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnBan(Server &, std::string_view , std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnInvalidPassword(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDrop(Server &, std::string_view , std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDisconnect(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnStopListen(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnResumeListen(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnWarning(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnRCON(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnAdminLogin(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnAdminGrant(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnAdminLogout(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnAdmin(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnVoteAddBots(Server &, const TeamType &, const PlayerInfo &, const TeamType &, int, int) {
	return;
}

void RenX::Plugin::RenX_OnVoteChangeMap(Server &, const TeamType &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVoteKick(Server &, const TeamType &, const PlayerInfo &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVoteMineBan(Server &, const TeamType &, const PlayerInfo &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVoteRemoveBots(Server &, const TeamType &, const PlayerInfo &, const TeamType &, int) {
	return;
}

void RenX::Plugin::RenX_OnVoteRestartMap(Server &, const TeamType &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVoteSurrender(Server &, const TeamType &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVoteSurvey(Server &, const TeamType &, const PlayerInfo &, std::string_view text) {
	return;
}

void RenX::Plugin::RenX_OnVoteOther(Server &, const TeamType &, std::string_view , const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnVoteOver(Server &, const TeamType &, std::string_view , bool, int, int) {
	return;
}

void RenX::Plugin::RenX_OnVoteCancel(Server &, const TeamType &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnVote(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnMapChange(Server &, std::string_view , bool) {
	return;
}

void RenX::Plugin::RenX_OnMapLoad(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnMapStart(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnMap(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDemoRecord(Server &, const PlayerInfo &) {
	return;
}

void RenX::Plugin::RenX_OnDemoRecord(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnDemoRecordStop(Server &) {
	return;
}

void RenX::Plugin::RenX_OnDemo(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnLog(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnCommand(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnError(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnVersion(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnAuthorized(Server &, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnOther(Server &, char, std::string_view ) {
	return;
}

void RenX::Plugin::RenX_OnRaw(Server &, std::string_view ) {
	return;
}