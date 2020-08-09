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

#if !defined _RENX_MODSYSTEM_H_HEADER
#define _RENX_MODSYSTEM_H_HEADER

#include <list>
#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/String.hpp"
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
		bool lockName;
		bool kickLockMismatch;
		bool autoAuthSteam;
		bool autoAuthIP;
		int access;
		Jupiter::StringS prefix;
		Jupiter::StringS gamePrefix;
		Jupiter::StringS name;
	};
	std::list<ModGroup> groups;

	/**
	* @brief Calls resetAccess() on all players in a server.
	*
	* @param server Server with players to logout
	* @return Number of players effected.
	*/
	unsigned int logoutAllMods(RenX::Server &server);

	/**
	* @brief Resets a player's access level to their administrative tag's assosciated access.
	* This means that a player not logged in as an in-game administrator/moderator will lose all access.
	*
	* @param player Player to reset access
	* @return True if the player's access level was modified, false otherwise.
	*/
	bool resetAccess(RenX::PlayerInfo &player);

	/**
	* @brief Authenticates a player with their configured administrative group.
	*
	* @param server Server the player is in
	* @param player Player to authenticate
	* @param checkAuto (optional) True to require that the user meet auto-authentication requirements, false otherwise
	* @param forceauth (optional) True to bypass locks and authenticate the user based on UUID.
	* @return New access of the user, -1 if the user is kicked.
	*/
	int auth(RenX::Server &server, const RenX::PlayerInfo &player, bool checkAuto = false, bool forceAuth = false) const;

	void tempAuth(RenX::Server &server, const RenX::PlayerInfo &player, const ModGroup *group, bool notify = true) const;

	bool set(RenX::PlayerInfo &player, ModGroup &group);
	bool removeModSection(const Jupiter::ReadableString& section);

	int getConfigAccess(const Jupiter::ReadableString &uuid) const;
	size_t getGroupCount() const;
	ModGroup *getGroupByName(const Jupiter::ReadableString &name, ModGroup *defaultGroup = nullptr) const;
	ModGroup *getGroupByAccess(int access, ModGroup *defaultGroup = nullptr) const;
	ModGroup *getGroupByIndex(size_t index) const;
	ModGroup *getDefaultGroup() const;
	ModGroup *getDefaultATMGroup() const;
	ModGroup *getModeratorGroup() const;
	ModGroup *getAdministratorGroup() const;

	virtual bool initialize() override;
	~RenX_ModSystemPlugin();

public: // RenX::Plugin
	void RenX_OnPlayerCreate(RenX::Server &server, const RenX::PlayerInfo &player) override;
	void RenX_OnPlayerDelete(RenX::Server &server, const RenX::PlayerInfo &player) override;

	void RenX_OnIDChange(RenX::Server &server, const RenX::PlayerInfo &player, int oldID) override;

	void RenX_OnAdminLogin(RenX::Server &server, const RenX::PlayerInfo &player) override;
	void RenX_OnAdminGrant(RenX::Server &server, const RenX::PlayerInfo &player) override;
	void RenX_OnAdminLogout(RenX::Server &server, const RenX::PlayerInfo &player) override;

public: // Jupiter::Plugin
	int OnRehash() override;

private:
	bool lockSteam;
	bool lockIP;
	bool lockName;
	bool kickLockMismatch;
	bool autoAuthSteam;
	bool autoAuthIP;
	Jupiter::StringS atmDefault;
	Jupiter::StringS moderatorGroup;
	Jupiter::StringS administratorGroup;
};

GENERIC_IRC_COMMAND(AuthIRCCommand)
GENERIC_IRC_COMMAND(DeAuthIRCCommand)
GENERIC_IRC_COMMAND(ATMIRCCommand)
GENERIC_IRC_COMMAND(AddIRCCommand)
GENERIC_IRC_COMMAND(DelIRCCommand)
GENERIC_IRC_COMMAND(ForceAuthIRCCommand)
GENERIC_IRC_COMMAND(ModListIRCCommand)
GENERIC_GAME_COMMAND(AuthGameCommand)
GENERIC_GAME_COMMAND(ATMGameCommand)
GENERIC_GAME_COMMAND(ForceAuthGameCommand)

#endif // _RENX_MODSYSTEM_H_HEADER