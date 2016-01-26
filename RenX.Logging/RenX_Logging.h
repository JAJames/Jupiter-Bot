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

#if !defined _RENX_LOGGING_H_HEADER
#define _RENX_LOGGING_H_HEADER

#include "Jupiter/Plugin.h"
#include "RenX_Plugin.h"

class RenX_LoggingPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnKick(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason) override;
	void RenX_OnNameChange(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &newPlayerName) override;
	void RenX_OnTeamChange(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::TeamType &oldTeam) override;
	void RenX_OnExecute(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &command) override;
	void RenX_OnPlayer(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	
	void RenX_OnChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message) override;
	void RenX_OnTeamChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message) override;
	void RenX_OnRadioChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message) override;
	void RenX_OnHostChat(RenX::Server *server, const Jupiter::ReadableString &message) override;
	void RenX_OnHostPage(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message) override;
	void RenX_OnOtherChat(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	
	void RenX_OnDeploy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object) override;
	void RenX_OnDisarm(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object, const RenX::PlayerInfo *victim) override;
	void RenX_OnDisarm(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object) override;
	void RenX_OnExplode(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object) override;
	void RenX_OnExplode(RenX::Server *server, const Jupiter::ReadableString &object) override;
	void RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDie(RenX::Server *server, const Jupiter::ReadableString &object, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type) override;
	void RenX_OnDestroy(RenX::Server *server, const Jupiter::ReadableString &killer, const RenX::TeamType &killerTeam, const Jupiter::ReadableString &objectName, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type) override;
	void RenX_OnCapture(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &building, const RenX::TeamType &oldTeam) override;
	void RenX_OnNeutralize(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &building, const RenX::TeamType &oldTeam) override;
	void RenX_OnCharacterPurchase(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character) override;
	void RenX_OnItemPurchase(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &item) override;
	void RenX_OnWeaponPurchase(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &weapon) override;
	void RenX_OnRefillPurchase(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnVehiclePurchase(RenX::Server *server, const RenX::PlayerInfo *owner, const Jupiter::ReadableString &vehicle) override;
	void RenX_OnVehicleSpawn(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &vehicle) override;
	void RenX_OnSpawn(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character) override;
	void RenX_OnBotJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnVehicleCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle) override;
	void RenX_OnTSVehicleCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle) override;
	void RenX_OnRAVehicleCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle) override;
	void RenX_OnDeathCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnMoneyCrate(RenX::Server *server, const RenX::PlayerInfo *player, int amount) override;
	void RenX_OnCharacterCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character) override;
	void RenX_OnSpyCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &character) override;
	void RenX_OnRefillCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnTimeBombCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnSpeedCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnNukeCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAbductionCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnUnspecifiedCrate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnOtherCrate(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &type) override;
	void RenX_OnSteal(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle) override;
	void RenX_OnSteal(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &vehicle, const RenX::PlayerInfo *victim) override;
	void RenX_OnDonate(RenX::Server *server, const RenX::PlayerInfo *donor, const RenX::PlayerInfo *player, double amount) override;
	void RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore) override;
	void RenX_OnGame(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	
	void RenX_OnExecute(RenX::Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command) override;
	void RenX_OnSubscribe(RenX::Server *server, const Jupiter::ReadableString &user) override;
	void RenX_OnRCON(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	
	void RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdminGrant(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdmin(RenX::Server *server, const Jupiter::ReadableString &raw) override;

	void RenX_OnVoteCall(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &type, const RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) override;
	void RenX_OnVoteOver(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &type, bool success, int yesVotes, int noVotes) override;
	void RenX_OnVoteCancel(RenX::Server *server, const RenX::TeamType &team, const Jupiter::ReadableString &type) override;
	void RenX_OnVote(RenX::Server *server, const Jupiter::ReadableString &raw) override;

	void RenX_OnMapChange(RenX::Server *server, const Jupiter::ReadableString &map, bool seamless) override;
	void RenX_OnMapLoad(RenX::Server *server, const Jupiter::ReadableString &map) override;
	void RenX_OnMapStart(RenX::Server *server, const Jupiter::ReadableString &map) override;
	void RenX_OnMap(RenX::Server *server, const Jupiter::ReadableString &raw) override;

	void RenX_OnDemoRecord(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnDemoRecord(RenX::Server *server, const Jupiter::ReadableString &user) override;
	void RenX_OnDemoRecordStop(RenX::Server *server) override;
	void RenX_OnDemo(RenX::Server *server, const Jupiter::ReadableString &raw) override;

	void RenX_OnLog(RenX::Server *server, const Jupiter::ReadableString &raw) override;

	void RenX_OnCommand(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	void RenX_OnError(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	void RenX_OnVersion(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	void RenX_OnAuthorized(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	void RenX_OnOther(RenX::Server *server, const char token, const Jupiter::ReadableString &raw) override;

public: // Jupiter::Plugin
	int OnRehash() override;
	const Jupiter::ReadableString &getName() override { return name; }

	RenX_LoggingPlugin();

private:
	void init();

	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Logging");
	unsigned int muteOwnExecute : 1;
	unsigned int joinPublic : 1;
	unsigned int partPublic : 1;
	unsigned int kickPublic : 1;
	unsigned int nameChangePublic : 1;
	unsigned int teamChangePublic : 1;
	unsigned int playerPublic : 1;
	unsigned int chatPublic : 1;
	unsigned int teamChatPublic : 1;
	unsigned int radioChatPublic : 1;
	unsigned int hostChatPublic : 1;
	unsigned int hostPagePublic : 1;
	unsigned int otherChatPublic : 1;
	unsigned int deployPublic : 1;
	unsigned int mineDeployPublic : 1;
	unsigned int disarmPublic : 1;
	unsigned int mineDisarmPublic : 1;
	unsigned int explodePublic : 1;
	unsigned int suicidePublic : 1;
	unsigned int killPublic : 1;
	unsigned int diePublic : 1;
	unsigned int destroyPublic : 1;
	unsigned int capturePublic : 1;
	unsigned int neutralizePublic : 1;
	unsigned int characterPurchasePublic : 1;
	unsigned int itemPurchasePublic : 1;
	unsigned int weaponPurchasePublic : 1;
	unsigned int refillPurchasePublic : 1;
	unsigned int vehiclePurchasePublic : 1;
	unsigned int vehicleSpawnPublic : 1;
	unsigned int spawnPublic : 1;
	unsigned int botJoinPublic : 1;
	unsigned int vehicleCratePublic : 1;
	unsigned int TSVehicleCratePublic : 1;
	unsigned int RAVehicleCratePublic : 1;
	unsigned int deathCratePublic : 1;
	unsigned int moneyCratePublic : 1;
	unsigned int characterCratePublic : 1;
	unsigned int spyCratePublic : 1;
	unsigned int refillCratePublic : 1;
	unsigned int timeBombCratePublic : 1;
	unsigned int speedCratePublic : 1;
	unsigned int nukeCratePublic : 1;
	unsigned int abductionCratePublic : 1;
	unsigned int unspecifiedCratePublic : 1;
	unsigned int otherCratePublic : 1;
	unsigned int stealPublic : 1;
	unsigned int donatePublic : 1;
	unsigned int gameOverPublic : 1;
	unsigned int gamePublic : 1;
	unsigned int executePublic : 1;
	unsigned int subscribePublic : 1;
	unsigned int RCONPublic : 1;
	unsigned int adminLoginPublic : 1;
	unsigned int adminGrantPublic : 1;
	unsigned int adminLogoutPublic : 1;
	unsigned int adminPublic : 1;
	unsigned int voteCallPublic : 1;
	unsigned int voteOverPublic : 1;
	unsigned int voteCancelPublic : 1;
	unsigned int votePublic : 1;
	unsigned int mapChangePublic : 1;
	unsigned int mapLoadPublic : 1;
	unsigned int mapStartPublic : 1;
	unsigned int mapPublic : 1;
	unsigned int demoRecordPublic : 1;
	unsigned int demoRecordStopPublic : 1;
	unsigned int demoPublic : 1;
	unsigned int logPublic : 1;
	unsigned int commandPublic : 1;
	unsigned int errorPublic : 1;
	unsigned int versionPublic : 1;
	unsigned int authorizedPublic : 1;
	unsigned int otherPublic : 1;

	/** Event formats */
	Jupiter::StringS joinPublicFmt, joinAdminFmt, joinNoSteamAdminFmt;
	Jupiter::StringS partFmt;
	Jupiter::StringS kickFmt;
	Jupiter::StringS nameChangeFmt;
	Jupiter::StringS teamChangeFmt;
	Jupiter::StringS playerFmt;
	Jupiter::StringS chatFmt;
	Jupiter::StringS teamChatFmt;
	Jupiter::StringS radioChatFmt;
	Jupiter::StringS hostChatFmt;
	Jupiter::StringS hostPageFmt;
	Jupiter::StringS otherChatFmt;
	Jupiter::StringS deployFmt;
	Jupiter::StringS mineDeployFmt;
	Jupiter::StringS disarmFmt;
	Jupiter::StringS mineDisarmFmt;
	Jupiter::StringS disarmNoOwnerFmt;
	Jupiter::StringS mineDisarmNoOwnerFmt;
	Jupiter::StringS explodeFmt;
	Jupiter::StringS explodeNoOwnerFmt;
	Jupiter::StringS suicideFmt;
	Jupiter::StringS dieFmt;
	Jupiter::StringS dieFmt2;
	Jupiter::StringS killFmt;
	Jupiter::StringS killFmt2;
	Jupiter::StringS destroyBuildingFmt;
	Jupiter::StringS destroyBuildingFmt2;
	Jupiter::StringS destroyDefenceFmt;
	Jupiter::StringS destroyDefenceFmt2;
	Jupiter::StringS destroyVehicleFmt;
	Jupiter::StringS destroyVehicleFmt2;
	Jupiter::StringS captureFmt;
	Jupiter::StringS neutralizeFmt;
	Jupiter::StringS characterPurchaseFmt;
	Jupiter::StringS itemPurchaseFmt;
	Jupiter::StringS weaponPurchaseFmt;
	Jupiter::StringS refillPurchaseFmt;
	Jupiter::StringS vehiclePurchaseFmt;
	Jupiter::StringS vehicleSpawnFmt;
	Jupiter::StringS spawnFmt;
	Jupiter::StringS botJoinFmt;
	Jupiter::StringS vehicleCrateFmt;
	Jupiter::StringS TSVehicleCrateFmt;
	Jupiter::StringS RAVehicleCrateFmt;
	Jupiter::StringS deathCrateFmt;
	Jupiter::StringS moneyCrateFmt;
	Jupiter::StringS characterCrateFmt;
	Jupiter::StringS spyCrateFmt;
	Jupiter::StringS refillCrateFmt;
	Jupiter::StringS timeBombCrateFmt;
	Jupiter::StringS speedCrateFmt;
	Jupiter::StringS nukeCrateFmt;
	Jupiter::StringS abductionCrateFmt;
	Jupiter::StringS unspecifiedCrateFmt;
	Jupiter::StringS otherCrateFmt;
	Jupiter::StringS stealFmt;
	Jupiter::StringS stealNoOwnerFmt;
	Jupiter::StringS donateFmt;
	Jupiter::StringS gameOverFmt;
	Jupiter::StringS gameOverTieFmt;
	Jupiter::StringS gameOverTieNoWinFmt;
	Jupiter::StringS gameOverScoreFmt;
	Jupiter::StringS gameFmt;
	Jupiter::StringS executeFmt, playerExecuteFmt, devBotExecuteFmt;
	Jupiter::StringS subscribeFmt;
	Jupiter::StringS rconFmt;
	Jupiter::StringS adminLoginFmt;
	Jupiter::StringS adminGrantFmt;
	Jupiter::StringS adminLogoutFmt;
	Jupiter::StringS adminFmt;
	Jupiter::StringS voteCallFmt;
	Jupiter::StringS voteOverSuccessFmt;
	Jupiter::StringS voteOverFailFmt;
	Jupiter::StringS voteCancelFmt;
	Jupiter::StringS voteFmt;
	Jupiter::StringS mapChangeFmt;
	Jupiter::StringS mapLoadFmt;
	Jupiter::StringS mapStartFmt;
	Jupiter::StringS mapFmt;
	Jupiter::StringS demoRecordFmt, rconDemoRecordFmt;
	Jupiter::StringS demoRecordStopFmt;
	Jupiter::StringS demoFmt;
	Jupiter::StringS logFmt;
	Jupiter::StringS commandFmt;
	Jupiter::StringS errorFmt;
	Jupiter::StringS versionFmt;
	Jupiter::StringS authorizedFmt;
	Jupiter::StringS otherFmt;
};

#endif // _RENX_LOGGING_H_HEADER