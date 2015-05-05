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

#if !defined _RENX_PLUGIN_H_HEADER
#define _RENX_PLUGIN_H_HEADER

/**
 * @file RenX_Plugin.h
 * @brief Provides an plugin interface that interacts with the Renegade-X Core.
 */

#include "Jupiter/Plugin.h"
#include "Jupiter/String_Type.h"
#include "RenX.h"

namespace RenX
{

	/** Forward declarations */
	struct PlayerInfo;
	class Plugin;
	class Server;

	class RENX_API Plugin : public Jupiter::Plugin
	{
	public:
		/** Tag-related events */
		virtual void RenX_SanitizeTags(Jupiter::StringType &fmt);
		virtual void RenX_ProcessTags(Jupiter::StringType &msg, const Server *server, const PlayerInfo *player, const PlayerInfo *victim);

		/** Non-RCON RenX logs */
		virtual void RenX_OnPlayerCreate(Server *server, const PlayerInfo *player);
		virtual void RenX_OnPlayerDelete(Server *server, const PlayerInfo *player);
		virtual void RenX_OnPlayerUUIDChange(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &newUUID);
		virtual void RenX_OnServerCreate(Server *server);
		virtual bool RenX_OnBan(Server *server, const PlayerInfo *player, Jupiter::StringType &data);

		/** Player type logs */
		virtual void RenX_OnJoin(Server *server, const PlayerInfo *player);
		virtual void RenX_OnPart(Server *server, const PlayerInfo *player);
		virtual void RenX_OnNameChange(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &newPlayerName);
		virtual void RenX_OnTeamChange(Server *server, const PlayerInfo *player, const TeamType &oldTeam);
		virtual void RenX_OnIDChange(Server *server, const PlayerInfo *player, int oldID);
		virtual void RenX_OnExecute(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &command);
		virtual void RenX_OnPlayer(Server *server, const Jupiter::ReadableString &raw);
		
		/** Chat type logs */
		virtual void RenX_OnChat(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &message);
		virtual void RenX_OnTeamChat(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &message);
		virtual void RenX_OnHostChat(Server *server, const Jupiter::ReadableString &message);
		virtual void RenX_OnHostPage(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &message);
		virtual void RenX_OnOtherChat(Server *server, const Jupiter::ReadableString &raw);
		
		/** Game type logs */
		virtual void RenX_OnDeploy(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &object);
		virtual void RenX_OnDisarm(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &object);
		virtual void RenX_OnDisarm(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &object, const PlayerInfo *victim);
		virtual void RenX_OnExplode(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &object);
		virtual void RenX_OnExplode(Server *server, const Jupiter::ReadableString &object);
		virtual void RenX_OnSuicide(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnKill(Server *server, const PlayerInfo *player, const PlayerInfo *victim, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnKill(Server *server, const Jupiter::ReadableString &killer, const TeamType &killerTeam, const PlayerInfo *victim, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnDie(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnDie(Server *server, const Jupiter::ReadableString &object, const TeamType &objectTeam, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnDestroy(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &objectName, const TeamType &victimTeam, const Jupiter::ReadableString &damageType, ObjectType type);
		virtual void RenX_OnDestroy(Server *server, const Jupiter::ReadableString &killer, const TeamType &killerTeam, const Jupiter::ReadableString &objectName, const TeamType &objectTeam, const Jupiter::ReadableString &damageType, ObjectType type);
		virtual void RenX_OnCapture(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &building, const TeamType &oldTeam);
		virtual void RenX_OnNeutralize(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &building, const TeamType &oldTeam);
		virtual void RenX_OnCharacterPurchase(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &character);
		virtual void RenX_OnItemPurchase(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &item);
		virtual void RenX_OnWeaponPurchase(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &weapon);
		virtual void RenX_OnRefillPurchase(Server *server, const PlayerInfo *player);
		virtual void RenX_OnVehiclePurchase(Server *server, const PlayerInfo *owner, const Jupiter::ReadableString &vehicle);
		virtual void RenX_OnVehicleSpawn(Server *server, const TeamType &team, const Jupiter::ReadableString &vehicle);
		virtual void RenX_OnSpawn(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &character);
		virtual void RenX_OnBotJoin(Server *server, const PlayerInfo *player);
		virtual void RenX_OnVehicleCrate(Server *server, const PlayerInfo *owner, const Jupiter::ReadableString &vehicle);
		virtual void RenX_OnDeathCrate(Server *server, const PlayerInfo *player);
		virtual void RenX_OnMoneyCrate(Server *server, const PlayerInfo *player, int amount);
		virtual void RenX_OnCharacterCrate(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &character);
		virtual void RenX_OnSpyCrate(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &character);
		virtual void RenX_OnRefillCrate(Server *server, const PlayerInfo *player);
		virtual void RenX_OnSteal(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &vehicle);
		virtual void RenX_OnSteal(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &vehicle, const PlayerInfo *victim);
		virtual void RenX_OnDonate(Server *server, const PlayerInfo *donor, const PlayerInfo *player, double amount);
		virtual void RenX_OnGameOver(Server *server, WinType winType, const TeamType &team, int gScore, int nScore);
		virtual void RenX_OnGame(Server *server, const Jupiter::ReadableString &raw);

		/** RCON type logs */
		virtual void RenX_OnExecute(Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command);
		virtual void RenX_OnSubscribe(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnUnsubscribe(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnBlock(Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &message);
		virtual void RenX_OnConnect(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnAuthenticate(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnBan(Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &message);
		virtual void RenX_OnInvalidPassword(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnDrop(Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &message);
		virtual void RenX_OnDisconnect(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnStopListen(Server *server, const Jupiter::ReadableString &message);
		virtual void RenX_OnResumeListen(Server *server, const Jupiter::ReadableString &message);
		virtual void RenX_OnWarning(Server *server, const Jupiter::ReadableString &message);
		virtual void RenX_OnRCON(Server *server, const Jupiter::ReadableString &raw);
		
		/** Admin Type Logs */
		virtual void RenX_OnAdminLogin(Server *server, const PlayerInfo *player);
		virtual void RenX_OnAdminGrant(Server *server, const PlayerInfo *player);
		virtual void RenX_OnAdminLogout(Server *server, const PlayerInfo *player);
		virtual void RenX_OnAdmin(Server *server, const Jupiter::ReadableString &raw);
		
		/** Vote Type Logs */
		virtual void RenX_OnVoteCall(Server *server, const TeamType &team, const Jupiter::ReadableString &type, const PlayerInfo *player, const Jupiter::ReadableString &parameters);
		virtual void RenX_OnVoteOver(Server *server, const TeamType &team, const Jupiter::ReadableString &type, bool success, int yesVotes, int noVotes);
		virtual void RenX_OnVoteCancel(Server *server, const TeamType &team, const Jupiter::ReadableString &type);
		virtual void RenX_OnVote(Server *server, const Jupiter::ReadableString &raw);

		/** Map Type Logs */
		virtual void RenX_OnMapChange(Server *server, const Jupiter::ReadableString &map, bool seamless);
		virtual void RenX_OnMapLoad(Server *server, const Jupiter::ReadableString &map);
		virtual void RenX_OnMap(Server *server, const Jupiter::ReadableString &raw);

		/** Demo Type Logs */
		virtual void RenX_OnDemoRecord(Server *server, const PlayerInfo *player);
		virtual void RenX_OnDemoRecord(Server *server, const Jupiter::ReadableString &user); // Note: user is currently always empty.
		virtual void RenX_OnDemoRecordStop(Server *server);
		virtual void RenX_OnDemo(Server *server, const Jupiter::ReadableString &raw);

		/** Other Logs */
		virtual void RenX_OnLog(Server *server, const Jupiter::ReadableString &raw);

		/** Command type */
		virtual void RenX_OnCommand(Server *server, const Jupiter::ReadableString &raw);
		
		/** Error type */
		virtual void RenX_OnError(Server *server, const Jupiter::ReadableString &raw);
		
		/** Version type */
		virtual void RenX_OnVersion(Server *server, const Jupiter::ReadableString &raw);
		
		/** Authorized type */
		virtual void RenX_OnAuthorized(Server *server, const Jupiter::ReadableString &raw);
		
		/** Other type - executed if none of the above matched */
		virtual void RenX_OnOther(Server *server, const char token, const Jupiter::ReadableString &raw);

		/** Catch-all - always executes for every line from RCON */
		virtual void RenX_OnRaw(Server *server, const Jupiter::ReadableString &raw);
		
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