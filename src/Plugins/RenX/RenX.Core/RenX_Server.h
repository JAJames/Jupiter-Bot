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

#if !defined _RENX_SERVER_H_HEADER
#define _RENX_SERVER_H_HEADER

/**
 * @file RenX_Server.h
 * @brief Defines the Server class.
 */

#include <chrono>
#include <list>
#include "Jupiter/TCPSocket.h"
#include "Jupiter/String.hpp"
#include "Jupiter/Config.h"
#include "Jupiter/Thinker.h"
#include "Jupiter/Rehash.h"
#include "RenX.h"
#include "RenX_Map.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	/** Forward declarations */
	struct PlayerInfo;
	struct BuildingInfo;
	class GameCommand;
	class Core;

	/**
	* @brief Represents a connection to an individiaul Renegade-X server.
	* There are often more than one of these, such as when communities run multiple servers.
	*/
	class RENX_API Server : public Jupiter::Thinker, public Jupiter::Rehashable
	{
	public: // Jupiter::Thinker

		/**
		* @brief Checks and processes raw socket data.
		*
		* @return Zero if no error occurs, a non-zero value otherwise.
		*/
		int think();

	public: // Jupiter::Rehashable

		/**
		* @brief Rehashes an object's status.
		*
		* @return 0.
		*/
		virtual int OnRehash() override;

		/**
		* @brief Fires when a non-zero value is returned by an object during a call to rehash().
		*
		* @param removed True if the object was removed from the rehashable objects list, false otherwise.
		* @return True if the object should be deleted, false otherwise.
		*/
		virtual bool OnBadRehash(bool removed);

	public: // RenX::Server
		std::list<RenX::PlayerInfo> players; /** A list of players in the server */
		std::vector<std::unique_ptr<RenX::BuildingInfo>> buildings; /** A list of buildings in the server */
		std::vector<Jupiter::StringS> mutators; /** A list of buildings the server is running */
		std::vector<RenX::Map> maps; /** A list of maps in the server's rotation */
		Jupiter::Config varData; /** Variable data. */

		/**
		* @brief Checks if the server is connected to RCON.
		*
		* @return True if the server is connected, false otherwise.
		*/
		bool isConnected() const;

		/**
		* @brief Checks if the server is currently subscribed to.
		*
		* @return True if the server is subscribed, false otherwise.
		*/
		bool isSubscribed() const;

		/**
		* @brief Checks if the server has been fully connected to.
		* Note: This is true after all of the initial commands have received a response, after isSubscribed() is true.
		*
		* @return True if the server is fully connected, false otherwise.
		*/
		bool isFullyConnected() const;

		/**
		* @brief Checks if the first kill has already been processed.
		* Note: This does not set to "true" until AFTER the first kill has been fully processed.
		* This includes processing by plugins.
		*
		* @return True if the first kill has been fully processed, false otherwise.
		*/
		bool isFirstKill() const;

		/**
		* @brief Checks if the first death has already been processed.
		* Note: This does not set to "true" until AFTER the first death has been fully processed.
		* This includes processing by plugins.
		*
		* @return True if the first death has been fully processed, false otherwise.
		*/
		bool isFirstDeath() const;

		/**
		* @brief Checks if the first action has already been processed.
		* Note: This does not set to "true" until AFTER the first action has been fully processed.
		* This includes processing by plugins.
		* An action is a kill, death, message, destroy, or any other player-induced interaction other than a join or part.
		*
		* @return True if the first action has been fully processed, false otherwise.
		*/
		bool isFirstAction() const;

		/**
		* @brief Checks if the server travel mode is seamless.
		*
		* @return True if the server is using seamless travel, false otherwise.
		*/
		bool isSeamless() const;

		/**
		* @brief Checks if the match data is reliable (bot has seen full game)
		* This is based on the bot seeing the "Start;" event.
		*
		* @return True if this server's match data is reliable, false otherwise..
		*/
		bool isReliable() const;

		/**
		* @brief Checks if the match is pending (hasn't started OR not in game state).
		*
		* @return True if the match is pending, false otherwise.
		*/
		bool isMatchPending() const;

		/**
		* @brief Checks if the match is in progress (started AND not over).
		*
		* @return True if a game is in progress, false otherwise.
		*/
		bool isMatchInProgress() const;

		/**
		* @brief Checks if the match has ended (has ended OR not in game state)
		*
		* @return True if the match as ended, false otherwise.
		*/
		bool isMatchOver() const;

		/**
		* @brief Checks if the server is travelling between levels (not in game state).
		*
		* @return True if the server is travelling, false otherwise.
		*/
		bool isTravelling() const;

		/**
		* @brief Checks if the server is marked as competitive.
		*
		* @return True if the server is a competitive server, false otherwise.
		*/
		bool isCompetitive() const;

		/**
		* @brief Checks if a channel type is a public channel type.
		*
		* @param type Type to check against.
		* @return True if this is a public channel type, false otherwise.
		*/
		bool isPublicLogChanType(int type) const;

		/**
		* @brief Checks if a channel type is an administrative channel type.
		*
		* @param type Type to check against.
		* @return True if this is an administrative channel type, false otherwise.
		*/
		bool isAdminLogChanType(int type) const;

		/**
		* @brief Checks if a channel type is either a public or administrative channel type.
		*
		* @param type Type to check against.
		* @return True if this is either a public or administrative channel type, false otherwise.
		*/
		bool isLogChanType(int type) const;

		/**
		* @brief Checks if a server is "pure" (i.e: not running any mutators).
		*
		* @return True if the server is pure, false otherwise.
		*/
		bool isPure() const;

		/**
		* @brief Sends a command to the server.
		*
		* @param commmand Command to send.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int send(const Jupiter::ReadableString &command);

		/**
		 * @brief Sends text over the socket
		 *
		 * @param text Text to send
		 * @return The number of bytes sent on success, less than or equal to zero otherwise.
		 */
		 int sendSocket(const Jupiter::ReadableString &text);

		/**
		* @brief Sends an in-game message to the server.
		*
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendMessage(std::string_view message);

		/**
		* @brief Sends an in-game message to a player in the server.
		*
		* @param player Data of the player to send a message to.
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendMessage(const RenX::PlayerInfo &player, std::string_view message);

		/**
		* @brief Sends an in-game admin message to the server.
		*
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendAdminMessage(const Jupiter::ReadableString &message);

		/**
		* @brief Sends an in-game admin message to a player in the server.
		*
		* @param player Data of the player to send a message to.
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendAdminMessage(const RenX::PlayerInfo &player, std::string_view message);

		/**
		* @brief Sends an in-game warning message to a player in the server.
		*
		* @param player Data of the player to send a message to.
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendWarnMessage(const RenX::PlayerInfo &player, std::string_view message);

		/**
		* @brief Sends data to the server.
		*
		* @param data String of data to send.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendData(const Jupiter::ReadableString &data);

		/**
		* @brief Fetches a player's data, based on their name.
		*
		* @param name Name of the player.
		* @return A player's data on success, nullptr otherwise.
		*/
		RenX::BuildingInfo *getBuildingByName(std::string_view name) const;

		/**
		* @brief Checks if a map name is in the rotation.
		*
		* @param name Name of map to search for
		* @return True if the map exists, false otherwise.
		*/
		bool hasMapInRotation(std::string_view name) const;

		/**
		* @brief Searches for a map based on a part of its name.
		*
		* @param name Part of the map's name to search for
		* @return A map's full name if it exists, an empty view otherwise.
		*/
		std::string_view getMapName(std::string_view name) const;

		/**
		* @brief Fetches the RCON command currently being processed.
		*
		* @return RCON command last executed.
		*/
		std::string_view getCurrentRCONCommand() const;

		/**
		* @brief Fetches the parameters of the RCON command currently being processed.
		*
		* @return Parameters of last RCON command last executed.
		*/
		std::string_view getCurrentRCONCommandParameters() const;

		/**
		* @brief Calculates the time since match start.
		* Note: if isReliable() is false, this is the time since the connection was established.
		*
		* @return Time since match start.
		*/
		std::chrono::milliseconds getGameTime() const;

		/**
		* @brief Calculates the time the player has been playing.
		* Note: This will never be greater than getGameTime(void)
		*
		* @param player Player to calculate game-time of.
		* @return Time player has been playing.
		*/
		std::chrono::milliseconds getGameTime(const RenX::PlayerInfo &player) const;

		/**
		* @brief Fetches the number of bots in the server.
		*
		* @return Number of bots in the server.
		*/
		size_t getBotCount() const;

		/**
		 * @brief Builds a list of all active players (i.e: players who have a team)
		 *
		 * @param includeBots Specifies whether or not to include bots in the returned list
		 * @return List of active players
		 */
		std::vector<const RenX::PlayerInfo*> activePlayers(bool includeBots = true) const;

		/**
		* @brief Fetches a player's data based on their ID number.
		*
		* @param id ID of the player
		* @return A pointer to a player's corresponding PlayerInfo on success, nullptr otherwise.
		*/
		RenX::PlayerInfo *getPlayer(int id) const;

		/**
		* @brief Fetches a player's data, based on their name.
		*
		* @param name Name of the player.
		* @return A player's data on success, nullptr otherwise.
		*/
		RenX::PlayerInfo *getPlayerByName(std::string_view name) const;

		/**
		* @brief Fetches a player's data, based on part of their name.
		*
		* @param name Name of the player.
		* @return A player's data on success, nullptr otherwise.
		*/
		RenX::PlayerInfo *getPlayerByPartName(std::string_view partName) const;

		/**
		* @brief Fetches a player's data, based on part of their name.
		* Note: This does not check that the exact name exists first.
		*
		* @param name Name of the player.
		* @return A player's data on success, nullptr otherwise.
		*/
		RenX::PlayerInfo *getPlayerByPartNameFast(std::string_view partName) const;

		/**
		* @brief Formats a player's Steam ID into a readable string.
		*
		* @param player Player to fetch Steam ID from
		* @return A player's formatted Steam ID on success, an empty string otherwise.
		*/
		Jupiter::StringS formatSteamID(const RenX::PlayerInfo &player) const;

		/**
		* @brief Formats a Steam ID into a readable string.
		*
		* @param id Steam ID to format
		* @return A player's formatted Steam ID on success, an empty string otherwise.
		*/
		Jupiter::StringS formatSteamID(uint64_t id) const;

		/**
		* @brief Kicks a player from the server.
		*
		* @param id Player ID of the player to kick.
		*/
		void kickPlayer(int id, std::string_view reason);

		/**
		* @brief Kicks a player from the server.
		*
		* @param player Data of the player to kick.
		*/
		void kickPlayer(const RenX::PlayerInfo &player, std::string_view reason);

		/**
		* @brief Kicks a player from the server.
		*
		* @param id Player ID of the player to kick.
		*/
		void forceKickPlayer(int id, std::string_view reason);

		/**
		* @brief Kicks a player from the server.
		*
		* @param player Data of the player to kick.
		*/
		void forceKickPlayer(const RenX::PlayerInfo &player, std::string_view reason);

		/**
		* @brief Checks if any players are in the ban list, and kicks any players listed.
		*/
		void banCheck();

		/**
		* @brief Checks if a player is in the ban list, and kicks them if they are.
		* Note: Check a player's ban_flags to see what ban types are active on them.
		*
		* @param player Data of the player to check.
		*/
		void banCheck(RenX::PlayerInfo &player);

		/**
		* @brief Bans a player from the server.
		*
		* @param id Player ID of the player to ban.
		*/
		void banPlayer(int id, std::string_view banner, std::string_view reason);

		/**
		* @brief Bans a player from the server.
		*
		* @param player Data of the player to ban.
		* @param length Duration of the ban (0 for permanent).
		*/
		void banPlayer(const RenX::PlayerInfo &player, std::string_view banner, std::string_view reason, std::chrono::seconds length = std::chrono::seconds::zero());

		/**
		* @brief Removes a player's data based on their ID number.
		*
		* @param id ID of the player
		* @return True if a player was removed, false otherwise.
		*/
		bool removePlayer(int id);

		/**
		* @brief Removes a player's data based on a player's data.
		*
		* @param player Pointer to a player's data
		* @return True if a player was removed, false otherwise.
		*/
		bool removePlayer(RenX::PlayerInfo &player);

		/**
		* @brief Sends a full client list request.
		* Note: This is used to build the initial player list.
		*
		* @return True on success, false otherwise.
		*/
		bool fetchClientList();

		/**
		* @brief Sends a patrial client list request.
		* Note: This only updates score, credits, and ping for known players.
		*
		* @return True on success, false otherwise.
		*/
		bool updateClientList();

		/**
		* @brief Sends a building list request.
		*
		* @return True on success, false otherwise.
		*/
		bool updateBuildingList();

		/**
		* @brief Forces the current game to end immediately.
		*
		* @return True on success, false otherwise.
		*/
		bool gameover();

		/**
		* @brief Forces the game to end, after a given delay.
		*
		* @param delay The number of seconds from now that the gameover should trigger.
		*/
		void gameover(std::chrono::seconds delay);

		/**
		* @brief Prevents a delayed gameover from starting.
		*
		* @return True if a delayed gameover was stopped, false otherwise.
		*/
		bool gameoverStop();

		/**
		* @brief Forces the game to end when the server is empty
		*/
		void gameoverWhenEmpty();

		/**
		* @brief Forces the current game to end and changes the map.
		*
		* @return True on success, false otherwise.
		*/
		bool setMap(std::string_view map);

		/**
		* @brief Forces the current game to end.
		*
		* @return True on success, false otherwise.
		*/
		bool loadMutator(const Jupiter::ReadableString &mutator);

		/**
		* @brief Forces the current game to end.
		*
		* @return True on success, false otherwise.
		*/
		bool unloadMutator(const Jupiter::ReadableString &mutator);

		/**
		* @brief Forcefully ends the current vote.
		*
		* @return True on success, false otherwise.
		*/
		bool cancelVote(const RenX::TeamType team);

		/**
		* @brief Swaps the teams.
		*
		* @return True on success, false otherwise.
		*/
		bool swapTeams();

		/**
		* @brief Starts a demo recording.
		*
		* @return True on success, false otherwise.
		*/
		bool recordDemo();

		/**
		* @brief Mutes a player from the game chat.
		*
		* @return True on success, false otherwise.
		*/
		bool mute(const RenX::PlayerInfo &player);

		/**
		* @brief Allows a player to use the game chat.
		*
		* @return True on success, false otherwise.
		*/
		bool unmute(const RenX::PlayerInfo &player);

		/**
		* @brief Gives a player additional credits.
		*
		* @param id ID of the player to give credits to
		* @param credits Credits to give to player
		* @return True on success, false otherwise.
		*/
		bool giveCredits(int id, double credits);

		/**
		* @brief Gives a player additional credits.
		*
		* @param player Player to give credits to
		* @param credits Credits to give to player
		* @return True on success, false otherwise.
		*/
		bool giveCredits(RenX::PlayerInfo &player, double credits);

		/**
		* @brief Kills a player.
		*
		* @param id ID of the player to kill
		* @return True on success, false otherwise.
		*/
		bool kill(int id);

		/**
		* @brief Kills a player.
		*
		* @param player Player to kill
		* @return True on success, false otherwise.
		*/
		bool kill(RenX::PlayerInfo &player);

		/**
		* @brief Disarms all of a player's deployed objects.
		*
		* @param id ID of the player to disarm
		* @return True on success, false otherwise.
		*/
		bool disarm(int id);

		/**
		* @brief Disarms all of a player's deployed objects.
		*
		* @param player Player to disarm
		* @return True on success, false otherwise.
		*/
		bool disarm(RenX::PlayerInfo &player);

		/**
		* @brief Disarms all of a player's deployed C4.
		*
		* @param id ID of the player to disarm
		* @return True on success, false otherwise.
		*/
		bool disarmC4(int id);

		/**
		* @brief Disarms all of a player's deployed C4.
		*
		* @param player Player to disarm
		* @return True on success, false otherwise.
		*/
		bool disarmC4(RenX::PlayerInfo &player);

		/**
		* @brief Disarms all of a player's deployed beacons.
		*
		* @param id ID of the player to disarm
		* @return True on success, false otherwise.
		*/
		bool disarmBeacon(int id);

		/**
		* @brief Disarms all of a player's deployed beacons.
		*
		* @param player Player to disarm
		* @return True on success, false otherwise.
		*/
		bool disarmBeacon(RenX::PlayerInfo &player);

		/**
		* @brief Bans a player from mining in-game for 1 game (or until they disconnect).
		*
		* @param player Id of the player to mine-ban
		* @return True on success, false otherwise.
		*/
		bool mineBan(int id);

		/**
		* @brief Bans a player from mining in-game for 1 game (or until they disconnect).
		*
		* @param player Player to mine-ban
		* @return True on success, false otherwise.
		*/
		bool mineBan(RenX::PlayerInfo &player);

		/**
		* @brief Forces a player to change teams.
		*
		* @param id ID of the player to give credits to
		* @param resetCredits True to reset the player's credits, false otherwise.
		* @return True on success, false otherwise.
		*/
		bool changeTeam(int id, bool resetCredits = true);

		/**
		* @brief Forces a player to change teams.
		*
		* @param player Player to change teams
		* @param resetCredits True to reset the player's credits, false otherwise.
		* @return True on success, false otherwise.
		*/
		bool changeTeam(RenX::PlayerInfo &player, bool resetCredits = true);

		/**
		* @brief Changes a player's mode from spectator to normal
		*
		* @param player Player to change mode
		* @return True on success, false otherwise.
		*/
		bool nmodePlayer(const RenX::PlayerInfo &player);

		/**
		* @brief Changes a player's mode to spectator
		*
		* @param player Player to change mode
		* @return True on success, false otherwise.
		*/
		bool smodePlayer(const RenX::PlayerInfo &player);

		/**
		* @brief Fetches a server's IRC logging prefix.
		*
		* @return The server's logging prefix.
		*/
		const Jupiter::ReadableString &getPrefix() const;

		/**
		* @brief Sets a server's IRC logging prefix.
		*
		* @param prefix Prefix to use.
		*/
		void setPrefix(std::string_view prefix);

		/**
		* @brief Fetches a server's game command prefix.
		*
		* @return The server's game command prefix.
		*/
		std::string_view getCommandPrefix() const;

		/**
		* @brief Sets a server's game command prefix.
		*
		* @param prefix Prefix to use.
		*/
		void setCommandPrefix(std::string_view prefix);

		void setRanked(bool in_value);

		/**
		* @brief Fetches the rules of a server.
		*
		* @return String containing the rules of the server.
		*/
		const Jupiter::ReadableString &getRules() const;

		/**
		* @brief Fetches the hostname of a server.
		*
		* @return String containing the hostname of the server.
		*/
		const std::string &getHostname() const;

		/**
		* @brief Fetches the port of a server.
		*
		* @return Port of the server.
		*/
		unsigned short getPort() const;

		/**
		* @brief Fetches the hostname from the server socket.
		*
		* @return String containing the hostname of the server.
		*/
		const std::string &getSocketHostname() const;

		/**
		* @brief Fetches the port from the server socket.
		*
		* @return Port of the server.
		*/
		unsigned short getSocketPort() const;

		/**
		* @brief Fetches the time of the last connection attempt.
		* Note: This includes both successful and failed connection attempts.
		*
		* @return Time of the last connection attempt.
		*/
		std::chrono::steady_clock::time_point getLastAttempt() const;

		/**
		* @brief Fetches the time delay between connection attempts.
		*
		* @return Time delay between connection attempts.
		*/
		std::chrono::milliseconds getDelay() const;

		int getMineLimit() const;
		int getPlayerLimit() const;
		int getVehicleLimit() const;
		int getTimeLimit() const;
		int getTeamMode() const;
		int getGameType() const;
		double getCrateRespawnDelay() const;
		bool isSteamRequired() const;
		bool isPrivateMessageTeamOnly() const;
		bool isPrivateMessagingEnabled() const;
		bool isRanked() const;

		/**
		* @brief Checks if the server has a game password.
		*
		* @return True if the game is passworded, false otherwise.
		*/
		bool isPassworded() const;
		
		bool isCratesEnabled() const;
		bool isBotsEnabled() const;

		/**
		* @brief Fetches the RCON password of a server.
		*
		* @return String containing the password of the server.
		*/
		const Jupiter::ReadableString &getPassword() const;

		/**
		* @brief Fetches this connection's RCON user name.
		*
		* @return String containing the RCON user name.
		*/
		std::string_view getUser() const;

		/**
		* @brief Checks if this connection is a DevBot connection.
		*
		* @return True if this is a DevBot connection, false otherwise.
		*/
		bool isDevBot() const;

		/**
		* @brief Fetches the name of this server.
		*
		* @return String containing the server name.
		*/
		const Jupiter::ReadableString &getName() const;

		/**
		* @brief Fetches the current map.
		*
		* @return String containing the current map.
		*/
		const RenX::Map &getMap() const;

		/**
		* @brief Fetches a command from the list.
		*
		* @param index Index of the command to return.
		* @return Command located at the specified index.
		*/
		GameCommand *getCommand(unsigned int index) const;

		/**
		* @brief Fetches a command from the list.
		*
		* @param trigger Trigger of the command to get
		* @return Command with the specified trigger.
		*/
		GameCommand *getCommand(std::string_view trigger) const;

		/**
		* @brief Returns the number of commands in the command list.
		*
		* @return Number of commands in the command list.
		*/
		size_t getCommandCount() const;

		/**
		* @brief Finds and triggers a matching GameCommand with the given parameters
		*
		* @param trigger Trigger of the command to fire
		* @param parameters Parameters to pass to the command
		* @return Command executed if a match is found, nullptr otherwise.
		*/
		RenX::GameCommand *triggerCommand(std::string_view trigger, RenX::PlayerInfo &player, std::string_view parameters);

		/**
		* @brief Adds a command to the server's game command list.
		*
		* @param command Command to add.
		*/
		void addCommand(RenX::GameCommand *command);

		/**
		* @brief Adds a command to the server's game command list.
		*
		* @param command Command to remove.
		* @return True if a command was removed, false otherwise.
		*/
		bool removeCommand(RenX::GameCommand *command);

		/**
		* @brief Adds a command to the server's game command list.
		*
		* @param trigger Trigger of the command to remove.
		* @return True if a command was removed, false otherwise.
		*/
		bool removeCommand(std::string_view trigger);

		/**
		* @brief Prototype of UUID calculation function.
		* Note: Every player's UUID should be either unique, or empty.
		*
		* @param player Player to calculate UUID of
		* @return UUID calculated from player.
		*/
		typedef Jupiter::StringS(*uuid_func)(RenX::PlayerInfo &player);

		/**
		* @brief Sets the player UUID calculation function.
		* Note: This also recalculates the UUIDs of all current players.
		*
		* @param function Function to calculate UUIDs with.
		*/
		void setUUIDFunction(uuid_func function);

		/**
		* @brief Fetches the current UUID calculation function.
		*
		* @return Function used to calculate UUIDs.
		*/
		uuid_func getUUIDFunction() const;

		/**
		* @brief Changes a player's UUID.
		*
		* @param player Player with UUID to change
		* @param uuid Player's new UUID
		*/
		void setUUID(RenX::PlayerInfo &player, std::string_view uuid);

		/**
		* @brief Changes a player's UUID only if it is different than their current UUID
		*
		* @param player Player with UUID to change
		* @param uuid Player's new UUID
		* @return True if the UUIDs did not match, false otherwise.
		*/
		bool setUUIDIfDifferent(RenX::PlayerInfo &player, std::string_view uuid);

		/**
		* @brief Checks if reverse DNS resolution is occuring for players.
		*
		* @return True if this bot resolves RDNS, false otherwise.
		*/
		bool resolvesRDNS();

		/**
		* @brief Formats and sends a message to a server's corresponding public channels.
		*
		* @param fmt String containing the format specifiers indicating what message to send.
		*/
		void sendPubChan(const char *fmt, ...) const;
		void sendPubChan(const Jupiter::ReadableString &msg) const;

		/**
		* @brief Formats and sends a message to a server's corresponding adminstrative channels.
		*
		* @param fmt String containing the format specifiers indicating what message to send.
		*/
		void sendAdmChan(const char *fmt, ...) const;
		void sendAdmChan(const Jupiter::ReadableString &msg) const;

		/**
		* @brief Formats and sends a message to a server's corresponding channels.
		*
		* @param fmt String containing the format specifiers indicating what message to send.
		*/
		void sendLogChan(const char *fmt, ...) const;
		void sendLogChan(const Jupiter::ReadableString &msg) const;

		/**
		* @brief Processes a line of RCON input data. Input data SHOULD NOT include a new-line ('\n') terminator.
		*
		* @param line Line to process
		*/
		void processLine(const Jupiter::ReadableString &line);

		/**
		* @brief Disconnects from a server's RCON interface.
		*
		* @param reason Reason for reconnecting.
		*/
		void disconnect(RenX::DisconnectReason reason);

		/**
		* @brief Connects to the server's RCON interface.
		*
		* @return True if the socket successfully connected, false otherwise.
		*/
		bool connect();

		/**
		* @brief Reconnects to the server's RCON interface.
		*
		* @param reason Reason for reconnecting.
		* @return True if the socket successfully connected, false otherwise.
		*/
		bool reconnect(RenX::DisconnectReason reason);

		/**
		* @brief Deletes all of the data about a server (such as players).
		* This is mostly for use with the reconnect mechanism.
		*/
		void wipeData();

		/**
		* @brief Fetches the RCON version number, or 0 if none has been set.
		*
		* @return RCON version number
		*/
		unsigned int getVersion() const;

		/**
		* @brief Fetches the game version number, or 0 if none has been set.
		*
		* @return Game version number
		*/
		unsigned int getGameVersionNumber() const;

		/**
		* @brief Fetches the game version string, or an empty string if none has been set.
		*
		* @return String containing the Game version
		*/
		std::string_view getGameVersion() const;

		/**
		* @brief Fetches the RCON user name.
		*
		* @return String containing the RCON user name
		*/
		std::string_view getRCONUsername() const;

		/**
		* @brief Creates a server object using the provided socket, and loads settings from the specified configuration section.
		*
		* @param socket Initialized Socket to use for the connection
		* @param configurationSection Configuration section to load from.
		*/
		Server(Jupiter::Socket &&socket, std::string_view configurationSection);

		/**
		* @brief Creates the Server object, and loads settings from the specified configuration section.
		*
		* @param configurationSection Configuration section to load from.
		*/
		Server(std::string_view configurationSection);

		/**
		* @brief Destructor for Server class.
		*/
		~Server();

	/** Private members */
	private:
		void init(const Jupiter::Config &config);
		void wipePlayers();
		void startPing();

		/** Tracking variables */
		bool m_gameover_when_empty = false;
		bool m_gameover_pending = false;
		bool m_pure = false;
		bool m_connected = false;
		bool m_subscribed = false;
		bool m_fully_connected = false;
		bool m_seamless = false;
		bool m_firstKill = false;
		bool m_firstDeath = false;
		bool m_firstAction = false;
		bool m_awaitingPong = false;
		bool m_passworded = false;
		bool m_steamRequired = false;
		bool m_privateMessageTeamOnly = false;
		bool m_allowPrivateMessaging = true;
		bool m_spawnCrates = true;
		bool m_botsEnabled = true;
		bool m_competitive = false;
		bool m_devBot = false;
		bool m_reliable = false;
		bool m_ranked = false;
		// TODO: Why aren't these enums?
		int m_team_mode = 3; /** 0 = static, 1 = swap, 2 = random swap, 3 = shuffle, 4 = traditional (assign as players connect) */
		int m_match_state = 1; /** 0 = pending, 1 = in progress, 2 = over, 3 = travelling */
		int m_game_type = 1; /** < 0 = Invalid, 0 = Main Menu, 1 = Rx_Game, 2 = TS_Game, > 2 = Unassigned */
		int m_attempts = 0;
		int m_playerLimit = 0;
		int m_vehicleLimit = 0;
		int m_mineLimit = 0;
		int m_timeLimit = 0;
		size_t m_bot_count = 0;
		size_t m_player_rdns_resolutions_pending = 0;
		unsigned int m_rconVersion = 0;
		unsigned int m_gameVersionNumber = 0;
		double m_crateRespawnAfterPickup = 0.0;
		uuid_func m_calc_uuid;
		std::chrono::steady_clock::time_point m_lastAttempt = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_gameStart = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_lastClientListUpdate = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_lastBuildingListUpdate = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_lastActivity = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_lastSendActivity = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point m_gameover_time;
		std::string m_lastLine;
		std::string m_rconUser;
		std::string m_gameVersion;
		Jupiter::StringS m_serverName;
		std::string m_lastCommand;
		std::string m_lastCommandParams;
		RenX::Map m_map;
		Jupiter::TCPSocket m_sock;
		std::vector<std::string> m_commandListFormat;
		std::vector<std::unique_ptr<RenX::GameCommand>> m_commands;

		/** Configuration variables */
		bool m_rconBan;
		bool m_localBan;
		bool m_localSteamBan;
		bool m_localIPBan;
		bool m_localHWIDBan;
		bool m_localRDNSBan;
		bool m_localNameBan;
		bool m_neverSay;
		bool m_resolve_player_rdns;
		unsigned short m_port;
		int m_logChanType;
		int m_adminLogChanType;
		int m_maxAttempts;
		int m_steamFormat; /** 16 = hex, 10 = base 10, 8 = octal, -2 = SteamID 2, -3 = SteamID 3 */
		std::chrono::milliseconds m_delay;
		std::chrono::milliseconds m_clientUpdateRate;
		std::chrono::milliseconds m_buildingUpdateRate;
		std::chrono::milliseconds m_pingRate;
		std::chrono::milliseconds m_pingTimeoutThreshold;
		std::string m_clientHostname;
		std::string m_hostname;
		std::string m_pass;
		std::string m_configSection;
		Jupiter::StringS m_rules;
		Jupiter::StringS m_ban_from_str;
		std::string m_IRCPrefix;
		std::string m_CommandPrefix;
		Jupiter::Config* m_commandAccessLevels;
		Jupiter::Config* m_commandAliases;
	};

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_SERVER_H_HEADER