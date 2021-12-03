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

#if !defined _RENX_PLUGIN_H_HEADER
#define _RENX_PLUGIN_H_HEADER

/**
 * @file RenX_Plugin.h
 * @brief Provides an plugin interface that interacts with the Renegade-X Core.
 */

#include "Jupiter/Plugin.h"
#include "RenX.h"

namespace RenX
{

	/** Forward declarations */
	struct PlayerInfo;
	struct BuildingInfo;
	class Plugin;
	class Server;
	class GameCommand;

	class RENX_API Plugin : public Jupiter::Plugin
	{
	public:
		/** Tag-related events */
		virtual void RenX_SanitizeTags(std::string& fmt);
		virtual void RenX_ProcessTags(std::string& msg, const Server *server, const PlayerInfo *player, const PlayerInfo *victim, const BuildingInfo *building);

		/** Non-RCON RenX logs */
		virtual void RenX_OnPlayerCreate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnPlayerDelete(Server &server, const PlayerInfo &player);
		virtual void RenX_OnPlayerUUIDChange(Server &server, const PlayerInfo &player, std::string_view newUUID);
		virtual void RenX_OnPlayerRDNS(Server &server, const PlayerInfo &player);
		virtual void RenX_OnPlayerIdentify(Server &server, const PlayerInfo &player);
		virtual void RenX_OnServerCreate(Server &server);
		virtual void RenX_OnServerFullyConnected(Server &server);
		virtual void RenX_OnServerDisconnect(Server &server, RenX::DisconnectReason reason);
		virtual bool RenX_OnBan(Server &server, const PlayerInfo &player, std::string &data);
		virtual void RenX_OnCommandTriggered(Server& server, std::string_view  trigger, RenX::PlayerInfo& player, std::string_view  parameters, GameCommand& command);

		/** Player type logs */
		virtual void RenX_OnJoin(Server &server, const PlayerInfo &player);
		virtual void RenX_OnPart(Server &server, const PlayerInfo &player);
		virtual void RenX_OnKick(Server &server, const PlayerInfo &player, std::string_view reason);
		virtual void RenX_OnNameChange(Server &server, const PlayerInfo &player, std::string_view newPlayerName);
		virtual void RenX_OnTeamChange(Server &server, const PlayerInfo &player, const TeamType &oldTeam);
		virtual void RenX_OnHWID(Server &server, const PlayerInfo &player);
		virtual void RenX_OnIDChange(Server &server, const PlayerInfo &player, int oldID);
		virtual void RenX_OnRank(Server &server, const PlayerInfo &player);
		virtual void RenX_OnDev(Server &server, const PlayerInfo &player);
		virtual void RenX_OnExecute(Server &server, const PlayerInfo &player, std::string_view command);
		virtual void RenX_OnPlayerCommand(Server &server, const PlayerInfo &player, std::string_view message, GameCommand *command);
		virtual void RenX_OnSpeedHack(Server &server, const PlayerInfo &player);
		virtual void RenX_OnPlayer(Server &server, std::string_view raw);
		
		/** Chat type logs */
		virtual void RenX_OnChat(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnTeamChat(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnRadioChat(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnHostChat(Server &server, std::string_view message);
		virtual void RenX_OnHostPage(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnAdminMessage(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnWarnMessage(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnAdminPMessage(Server &server, const PlayerInfo &player, const PlayerInfo &target, std::string_view message);
		virtual void RenX_OnWarnPMessage(Server &server, const PlayerInfo &player, const PlayerInfo &target, std::string_view message);
		virtual void RenX_OnHostAdminMessage(Server &server, std::string_view message);
		virtual void RenX_OnHostAdminPMessage(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnHostWarnMessage(Server &server, std::string_view message);
		virtual void RenX_OnHostWarnPMessage(Server &server, const PlayerInfo &player, std::string_view message);
		virtual void RenX_OnOtherChat(Server &server, std::string_view raw);
		
		/** Game type logs */
		virtual void RenX_OnDeploy(Server &server, const PlayerInfo &player, std::string_view object);
		virtual void RenX_OnOverMine(Server &server, const PlayerInfo &player, std::string_view location);
		virtual void RenX_OnDisarm(Server &server, const PlayerInfo &player, std::string_view object);
		virtual void RenX_OnDisarm(Server &server, const PlayerInfo &player, std::string_view object, const PlayerInfo &victim);
		virtual void RenX_OnExplode(Server &server, const PlayerInfo &player, std::string_view object);
		virtual void RenX_OnExplode(Server &server, std::string_view object);
		virtual void RenX_OnSuicide(Server &server, const PlayerInfo &player, std::string_view damageType);
		virtual void RenX_OnKill(Server &server, const PlayerInfo &player, const PlayerInfo &victim, std::string_view damageType);
		virtual void RenX_OnKill(Server &server, std::string_view killer, const TeamType &killerTeam, const PlayerInfo &victim, std::string_view damageType);
		virtual void RenX_OnDie(Server &server, const PlayerInfo &player, std::string_view damageType);
		virtual void RenX_OnDie(Server &server, std::string_view object, const TeamType &objectTeam, std::string_view damageType);
		virtual void RenX_OnDestroy(Server &server, const PlayerInfo &player, std::string_view objectName, const TeamType &victimTeam, std::string_view damageType, ObjectType type);
		virtual void RenX_OnDestroy(Server &server, std::string_view killer, const TeamType &killerTeam, std::string_view objectName, const TeamType &objectTeam, std::string_view damageType, ObjectType type);
		virtual void RenX_OnCapture(Server &server, const PlayerInfo &player, std::string_view building, const TeamType &oldTeam);
		virtual void RenX_OnNeutralize(Server &server, const PlayerInfo &player, std::string_view building, const TeamType &oldTeam);
		virtual void RenX_OnCharacterPurchase(Server &server, const PlayerInfo &player, std::string_view character);
		virtual void RenX_OnItemPurchase(Server &server, const PlayerInfo &player, std::string_view item);
		virtual void RenX_OnWeaponPurchase(Server &server, const PlayerInfo &player, std::string_view weapon);
		virtual void RenX_OnRefillPurchase(Server &server, const PlayerInfo &player);
		virtual void RenX_OnVehiclePurchase(Server &server, const PlayerInfo &owner, std::string_view vehicle);
		virtual void RenX_OnVehicleSpawn(Server &server, const TeamType &team, std::string_view vehicle);
		virtual void RenX_OnSpawn(Server &server, const PlayerInfo &player, std::string_view character);
		virtual void RenX_OnBotJoin(Server &server, const PlayerInfo &player);
		virtual void RenX_OnVehicleCrate(Server &server, const PlayerInfo &owner, std::string_view vehicle);
		virtual void RenX_OnTSVehicleCrate(Server &server, const PlayerInfo &owner, std::string_view vehicle);
		virtual void RenX_OnRAVehicleCrate(Server &server, const PlayerInfo &owner, std::string_view vehicle);
		virtual void RenX_OnDeathCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnMoneyCrate(Server &server, const PlayerInfo &player, int amount);
		virtual void RenX_OnCharacterCrate(Server &server, const PlayerInfo &player, std::string_view character);
		virtual void RenX_OnSpyCrate(Server &server, const PlayerInfo &player, std::string_view character);
		virtual void RenX_OnRefillCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnTimeBombCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnSpeedCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnNukeCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnAbductionCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnUnspecifiedCrate(Server &server, const PlayerInfo &player);
		virtual void RenX_OnOtherCrate(Server &server, const PlayerInfo &player, std::string_view type);
		virtual void RenX_OnSteal(Server &server, const PlayerInfo &player, std::string_view vehicle);
		virtual void RenX_OnSteal(Server &server, const PlayerInfo &player, std::string_view vehicle, const PlayerInfo &victim);
		virtual void RenX_OnDonate(Server &server, const PlayerInfo &donor, const PlayerInfo &player, double amount);
		virtual void RenX_OnGameOver(Server &server, WinType winType, const TeamType &team, int gScore, int nScore);
		virtual void RenX_OnGame(Server &server, std::string_view raw);

		/** RCON type logs */
		virtual void RenX_OnExecute(Server &server, std::string_view user, std::string_view command);
		virtual void RenX_OnSubscribe(Server &server, std::string_view user);
		virtual void RenX_OnUnsubscribe(Server &server, std::string_view user);
		virtual void RenX_OnBlock(Server &server, std::string_view user, std::string_view message);
		virtual void RenX_OnConnect(Server &server, std::string_view user);
		virtual void RenX_OnAuthenticate(Server &server, std::string_view user);
		virtual void RenX_OnBan(Server &server, std::string_view user, std::string_view message);
		virtual void RenX_OnInvalidPassword(Server &server, std::string_view user);
		virtual void RenX_OnDrop(Server &server, std::string_view user, std::string_view message);
		virtual void RenX_OnDisconnect(Server &server, std::string_view user);
		virtual void RenX_OnStopListen(Server &server, std::string_view message);
		virtual void RenX_OnResumeListen(Server &server, std::string_view message);
		virtual void RenX_OnWarning(Server &server, std::string_view message);
		virtual void RenX_OnRCON(Server &server, std::string_view raw);
		
		/** Admin Type Logs */
		virtual void RenX_OnAdminLogin(Server &server, const PlayerInfo &player);
		virtual void RenX_OnAdminGrant(Server &server, const PlayerInfo &player);
		virtual void RenX_OnAdminLogout(Server &server, const PlayerInfo &player);
		virtual void RenX_OnAdmin(Server &server, std::string_view raw);
		
		/** Vote Type Logs */
		virtual void RenX_OnVoteAddBots(Server &server, const TeamType &team, const PlayerInfo &player, const TeamType &victim, int amount, int skill);
		virtual void RenX_OnVoteChangeMap(Server &server, const TeamType &team, const PlayerInfo &player);
		virtual void RenX_OnVoteKick(Server &server, const TeamType &team, const PlayerInfo &player, const PlayerInfo &victim);
		virtual void RenX_OnVoteMineBan(Server &server, const TeamType &team, const PlayerInfo &player, const PlayerInfo &victim);
		virtual void RenX_OnVoteRemoveBots(Server &server, const TeamType &team, const PlayerInfo &player, const TeamType &victim, int amount);
		virtual void RenX_OnVoteRestartMap(Server &server, const TeamType &team, const PlayerInfo &player);
		virtual void RenX_OnVoteSurrender(Server &server, const TeamType &team, const PlayerInfo &player);
		virtual void RenX_OnVoteSurvey(Server &server, const TeamType &team, const PlayerInfo &player, std::string_view text);
		virtual void RenX_OnVoteOther(Server &server, const TeamType &team, std::string_view type, const PlayerInfo &player);
		virtual void RenX_OnVoteOver(Server &server, const TeamType &team, std::string_view type, bool success, int yesVotes, int noVotes);
		virtual void RenX_OnVoteCancel(Server &server, const TeamType &team, std::string_view type);
		virtual void RenX_OnVote(Server &server, std::string_view raw);

		/** Map Type Logs */
		virtual void RenX_OnMapChange(Server &server, std::string_view map, bool seamless);
		virtual void RenX_OnMapLoad(Server &server, std::string_view map);
		virtual void RenX_OnMapStart(Server &server, std::string_view map);
		virtual void RenX_OnMap(Server &server, std::string_view raw);

		/** Demo Type Logs */
		virtual void RenX_OnDemoRecord(Server &server, const PlayerInfo &player);
		virtual void RenX_OnDemoRecord(Server &server, std::string_view user); // Note: user is currently always empty.
		virtual void RenX_OnDemoRecordStop(Server &server);
		virtual void RenX_OnDemo(Server &server, std::string_view raw);

		/** Other Logs */
		virtual void RenX_OnLog(Server &server, std::string_view raw);

		/** Command type */
		virtual void RenX_OnCommand(Server &server, std::string_view raw);
		
		/** Error type */
		virtual void RenX_OnError(Server &server, std::string_view raw);
		
		/** Version type */
		virtual void RenX_OnVersion(Server &server, std::string_view raw);
		
		/** Authorized type */
		virtual void RenX_OnAuthorized(Server &server, std::string_view raw);
		
		/** Other type - executed if none of the above matched */
		virtual void RenX_OnOther(Server &server, const char token, std::string_view raw);

		/** Catch-all - always executes for every line from RCON */
		virtual void RenX_OnRaw(Server &server, std::string_view raw);
		
		/**
		* @brief Default constructor for the Plugin class.
		*/
		Plugin();

		/**
		* @brief Destructor for the Plugin class.
		*/
		virtual ~Plugin();
	};

}

#endif // _RENX_PLUGIN_H_HEADER