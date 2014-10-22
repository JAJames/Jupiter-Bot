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

#if !defined _RENX_LOGGING_H_HEADER
#define _RENX_LOGGING_H_HEADER

#include "Jupiter/Plugin.h"
#include "RenX_Plugin.h"

class RenX_LoggingPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnNameChange(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &newPlayerName) override;
	void RenX_OnTeamChange(RenX::Server *server, const RenX::PlayerInfo *player) override;
	
	void RenX_OnChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message) override;
	void RenX_OnTeamChat(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &message) override;
	
	void RenX_OnDeploy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &object) override;
	void RenX_OnSuicide(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDie(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &damageType) override;
	void RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const Jupiter::ReadableString &damageType, RenX::ObjectType type) override;
	void RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore) override;
	void RenX_OnGame(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	
	void RenX_OnExecute(RenX::Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command) override;
	void RenX_OnSubscribe(RenX::Server *server, const Jupiter::ReadableString &user) override;
	void RenX_OnRCON(RenX::Server *server, const Jupiter::ReadableString &raw) override;
	
	void RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdminGrant(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdmin(RenX::Server *server, const Jupiter::ReadableString &raw) override;

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
	void sanitizeTags(Jupiter::StringType &fmt) const;
	void processTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player = nullptr, const RenX::PlayerInfo *victim = nullptr) const;

	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Logging");
	unsigned int joinPublic : 1;
	unsigned int partPublic : 1;
	unsigned int nameChangePublic : 1;
	unsigned int teamChangePublic : 1;
	unsigned int chatPublic : 1;
	unsigned int teamChatPublic : 1;
	unsigned int deployPublic : 1;
	unsigned int suicidePublic : 1;
	unsigned int killPublic : 1;
	unsigned int diePublic : 1;
	unsigned int destroyPublic : 1;
	unsigned int gamePublic : 1;
	unsigned int gameOverPublic : 1;
	unsigned int executePublic : 1;
	unsigned int subscribePublic : 1;
	unsigned int RCONPublic : 1;
	unsigned int adminLoginPublic : 1;
	unsigned int adminGrantPublic : 1;
	unsigned int adminLogoutPublic : 1;
	unsigned int adminPublic : 1;
	unsigned int logPublic : 1;
	unsigned int commandPublic : 1;
	unsigned int errorPublic : 1;
	unsigned int versionPublic : 1;
	unsigned int authorizedPublic : 1;
	unsigned int otherPublic : 1;

	/** Server tags */
	Jupiter::StringS rconVersionTag;
	Jupiter::StringS gameVersionTag;

	/** Player tags */
	Jupiter::StringS nameTag;
	Jupiter::StringS rawNameTag;
	Jupiter::StringS ipTag;
	Jupiter::StringS steamTag;
	Jupiter::StringS uuidTag;
	Jupiter::StringS idTag;
	Jupiter::StringS adminTag;
	Jupiter::StringS teamColorTag;
	Jupiter::StringS teamShortTag;
	Jupiter::StringS teamLongTag;

	/** Victim tags */
	Jupiter::StringS victimNameTag;
	Jupiter::StringS victimRawNameTag;
	Jupiter::StringS victimIPTag;
	Jupiter::StringS victimSteamTag;
	Jupiter::StringS victimUUIDTag;
	Jupiter::StringS victimIDTag;
	Jupiter::StringS victimAdminTag;
	Jupiter::StringS victimTeamColorTag;
	Jupiter::StringS victimTeamShortTag;
	Jupiter::StringS victimTeamLongTag;

	/** Other tags */
	Jupiter::StringS weaponTag;
	Jupiter::StringS objectTag;
	Jupiter::StringS messageTag;
	Jupiter::StringS newNameTag;
	Jupiter::StringS winScoreTag;
	Jupiter::StringS loseScoreTag;

	/** Event formats */
	Jupiter::StringS joinPublicFmt, joinAdminFmt, joinNoSteamAdminFmt;
	Jupiter::StringS partFmt;
	Jupiter::StringS nameChangeFmt;
	Jupiter::StringS teamChangeFmt;
	Jupiter::StringS chatFmt;
	Jupiter::StringS teamChatFmt;
	Jupiter::StringS deployFmt;
	Jupiter::StringS suicideFmt;
	Jupiter::StringS dieFmt;
	Jupiter::StringS killFmt;
	Jupiter::StringS destroyBuildingFmt;
	Jupiter::StringS destroyDefenceFmt;
	Jupiter::StringS destroyVehicleFmt;
	Jupiter::StringS gameOverFmt;
	Jupiter::StringS gameOverTieFmt;
	Jupiter::StringS gameOverTieNoWinFmt;
	Jupiter::StringS gameOverScoreFmt;
	Jupiter::StringS gameFmt;
	Jupiter::StringS executeFmt, evaFmt, evaPrivateFmt;
	Jupiter::StringS subscribeFmt;
	Jupiter::StringS rconFmt;
	Jupiter::StringS adminLoginFmt;
	Jupiter::StringS adminGrantFmt;
	Jupiter::StringS adminLogoutFmt;
	Jupiter::StringS adminFmt;
	Jupiter::StringS logFmt;
	Jupiter::StringS commandFmt;
	Jupiter::StringS errorFmt;
	Jupiter::StringS versionFmt;
	Jupiter::StringS authorizedFmt;
	Jupiter::StringS otherFmt;
};

#endif // _RENX_LOGGING_H_HEADER