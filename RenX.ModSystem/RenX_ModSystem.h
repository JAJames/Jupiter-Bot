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

#if !defined _RENX_MODSYSTEM_H_HEADER
#define _RENX_MODSYSTEM_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/String.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/DLList.h"
#include "IRC_Command.h"
#include "RenX_Plugin.h"
#include "RenX_GameCommand.h"

class RenX_ModSystemPlugin : public RenX::Plugin
{
public:
	struct ModGroup
	{
		bool lockSteam;
		bool lockIP;
		bool kickLockMismatch;
		bool autoAuthSteam;
		bool autoAuthIP;
		int access;
		Jupiter::StringS prefix;
		Jupiter::StringS gamePrefix;
		Jupiter::StringS name;
	};
	Jupiter::DLList<ModGroup> groups;

	/**
	* @brief Calls resetAccess() on all players in a server.
	*
	* @param server Server with players to logout
	* @return Number of players effected.
	*/
	unsigned int logoutAllMods(const RenX::Server *server);

	/**
	* @brief Resets a player's access level to their administrative tag's assosciated access.
	* This means that a player not logged in as an in-game administrator/moderator will lose all access.
	*
	* @param player Player to reset access
	* @return True if the player's access level was modified, false otherwise.
	*/
	bool resetAccess(RenX::PlayerInfo *player);

	/**
	* @brief Authenticates a player with their configured administrative group.
	*
	* @param server Server the player is in
	* @param player Player to authenticate
	* @param checkAuto (optional) True to require that the user meet auto-authentication requirements, false otherwise
	* @param forceauth (optional) True to bypass locks and authenticate the user based on UUID.
	* @return New access of the user, -1 if the user is kicked.
	*/
	int auth(RenX::Server *server, const RenX::PlayerInfo *player, bool checkAuto = false, bool forceAuth = false) const;

	void tempAuth(RenX::Server *server, const RenX::PlayerInfo *player, const ModGroup *group, bool notify = true) const;

	int getConfigAccess(const Jupiter::ReadableString &uuid) const;
	size_t getGroupCount() const;
	ModGroup *getGroupByName(const Jupiter::ReadableString &name, ModGroup *defaultGroup = nullptr) const;
	ModGroup *getGroupByAccess(int access, ModGroup *defaultGroup = nullptr) const;
	ModGroup *getGroupByIndex(size_t index) const;
	ModGroup *getDefaultGroup() const;
	ModGroup *getDefaultATMGroup() const;
	ModGroup *getModeratorGroup() const;
	ModGroup *getAdministratorGroup() const;

	RenX_ModSystemPlugin();
	~RenX_ModSystemPlugin();

public: // RenX::Plugin
	void RenX_OnPlayerCreate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnPlayerDelete(RenX::Server *server, const RenX::PlayerInfo *player) override;

	void RenX_OnAdminLogin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdminGrant(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnAdminLogout(RenX::Server *server, const RenX::PlayerInfo *player) override;

public: // Jupiter::Plugin
	int OnRehash();
	const Jupiter::ReadableString &getName() override { return name; }

private:
	void init();
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.ModSystem");

	bool lockSteam;
	bool lockIP;
	bool kickLockMismatch;
	bool autoAuthSteam;
	bool autoAuthIP;
	Jupiter::StringS atmDefault;
	Jupiter::StringS moderatorGroup;
	Jupiter::StringS administratorGroup;
	Jupiter::INIFile modsFile;
};

GENERIC_IRC_COMMAND(AuthIRCCommand)
GENERIC_IRC_COMMAND(ATMIRCCommand)
GENERIC_IRC_COMMAND(ForceAuthIRCCommand)
GENERIC_GAME_COMMAND(AuthGameCommand)
GENERIC_GAME_COMMAND(ATMGameCommand)
GENERIC_GAME_COMMAND(ForceAuthGameCommand)

#endif // _RENX_MODSYSTEM_H_HEADER