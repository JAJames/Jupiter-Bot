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
	class Plugin;
	class Server;

	class RENX_API Plugin : public Jupiter::Plugin
	{
	public:
		/** Player type logs */
		virtual void RenX_OnJoin(Server *server, const PlayerInfo *player);
		virtual void RenX_OnPart(Server *server, const PlayerInfo *player);
		virtual void RenX_OnNameChange(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &newPlayerName);
		
		/** Chat type logs */
		virtual void RenX_OnChat(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &message);
		virtual void RenX_OnTeamChat(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &message);
		
		/** Game type logs */
		virtual void RenX_OnDeploy(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &object);
		virtual void RenX_OnSuicide(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnKill(Server *server, const PlayerInfo *player, const PlayerInfo *victim, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnDie(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &damageType);
		virtual void RenX_OnDestroy(Server *server, const PlayerInfo *player, const Jupiter::ReadableString &objectName, const Jupiter::ReadableString &damageType, ObjectType type);
		virtual void RenX_OnGameOver(Server *server, WinType winType, TeamType team, int gScore, int nScore);
		virtual void RenX_OnGame(Server *server, const Jupiter::ReadableString &raw);

		/** RCON type logs */
		virtual void RenX_OnExecute(Server *server, const Jupiter::ReadableString &user, const Jupiter::ReadableString &command);
		virtual void RenX_OnSubscribe(Server *server, const Jupiter::ReadableString &user);
		virtual void RenX_OnRCON(Server *server, const Jupiter::ReadableString &raw);
		
		/** Admin Type Logs */
		virtual void RenX_OnAdminLogin(Server *server, const PlayerInfo *player);
		virtual void RenX_OnAdminLogout(Server *server, const PlayerInfo *player);
		virtual void RenX_OnAdmin(Server *server, const Jupiter::ReadableString &raw);
		
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