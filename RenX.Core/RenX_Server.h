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

#if !defined _RENX_SERVER_H_HEADER
#define _RENX_SERVER_H_HEADER

/**
 * @file RenX_Server.h
 * @brief Defines the Server class.
 */

#include <ctime>
#include <chrono>
#include "Jupiter/TCPSocket.h"
#include "Jupiter/DLList.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter/String.h"
#include "Jupiter/CString.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/Thinker.h"
#include "Jupiter/Rehash.h"
#include "RenX.h"

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
		virtual int OnRehash();

		/**
		* @brief Fires when a non-zero value is returned by an object during a call to rehash().
		*
		* @param removed True if the object was removed from the rehashable objects list, false otherwise.
		* @return True if the object should be deleted, false otherwise.
		*/
		virtual bool OnBadRehash(bool removed);

	public: // RenX::Server
		Jupiter::DLList<RenX::PlayerInfo> players; /** A list of players in the server */
		Jupiter::ArrayList<RenX::BuildingInfo> buildings; /** A list of buildings in the server */
		Jupiter::INIFile varData; /** This may be replaced later with a more dedicated type. */

		/**
		* @brief Checks if the server is connected to RCON.
		*
		* @return True if the server is connected, false otherwise.
		*/
		bool isConnected() const;

		/**
		* @brief Checks if the game in progress is the first game.
		*
		* @return True if this is the first game, false otherwise.
		*/
		bool isFirstGame() const;

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
		* @brief Sends a command to the server.
		*
		* @param commmand Command to send.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int send(const Jupiter::ReadableString &command);

		/**
		* @brief Sends an in-game message to the server.
		*
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendMessage(const Jupiter::ReadableString &message);

		/**
		* @brief Sends an in-game message to a player in the server.
		*
		* @param player Data of the player to send a message to.
		* @param message Message to send in-game.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendMessage(const RenX::PlayerInfo *player, const Jupiter::ReadableString &message);

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
		RenX::BuildingInfo *getBuildingByName(const Jupiter::ReadableString &name) const;

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
		RenX::PlayerInfo *getPlayerByName(const Jupiter::ReadableString &name) const;

		/**
		* @brief Fetches a player's data, based on part of their name.
		*
		* @param name Name of the player.
		* @return A player's data on success, nullptr otherwise.
		*/
		RenX::PlayerInfo *getPlayerByPartName(const Jupiter::ReadableString &partName) const;

		/**
		* @brief Fetches a player's data, based on part of their name.
		* Note: This does not check that the exact name exists first.
		*
		* @param name Name of the player.
		* @return A player's data on success, nullptr otherwise.
		*/
		RenX::PlayerInfo *getPlayerByPartNameFast(const Jupiter::ReadableString &partName) const;

		/**
		* @brief Formats a player's Steam ID into a readable string.
		*
		* @param player Player to fetch Steam ID from
		* @return A player's formatted Steam ID on success, an empty string otherwise.
		*/
		Jupiter::StringS formatSteamID(const RenX::PlayerInfo *player) const;

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
		void kickPlayer(int id, const Jupiter::ReadableString &reason);

		/**
		* @brief Kicks a player from the server.
		*
		* @param player Data of the player to kick.
		*/
		void kickPlayer(const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason);

		/**
		* @brief Kicks a player from the server.
		*
		* @param id Player ID of the player to kick.
		*/
		void forceKickPlayer(int id, const Jupiter::ReadableString &reason);

		/**
		* @brief Kicks a player from the server.
		*
		* @param player Data of the player to kick.
		*/
		void forceKickPlayer(const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason);

		/**
		* @brief Bans a player from the server.
		*
		* @param id Player ID of the player to ban.
		*/
		void banPlayer(int id, const Jupiter::ReadableString &reason);

		/**
		* @brief Bans a player from the server.
		*
		* @param player Data of the player to ban.
		* @param length Duration of the ban (0 for permanent).
		*/
		void banPlayer(const RenX::PlayerInfo *player, const Jupiter::ReadableString &reason, time_t length = 0);

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
		bool removePlayer(RenX::PlayerInfo *player);

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
		* @brief Forces the current game to end.
		*
		* @return True on success, false otherwise.
		*/
		bool gameover();

		/**
		* @brief Forces the current game to end and changes the map.
		*
		* @return True on success, false otherwise.
		*/
		bool setMap(const Jupiter::ReadableString &map);

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
		bool mute(const RenX::PlayerInfo *player);

		/**
		* @brief Allows a player to use the game chat.
		*
		* @return True on success, false otherwise.
		*/
		bool unmute(const RenX::PlayerInfo *player);

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
		bool giveCredits(RenX::PlayerInfo *player, double credits);

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
		bool kill(RenX::PlayerInfo *player);

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
		bool disarm(RenX::PlayerInfo *player);

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
		bool disarmC4(RenX::PlayerInfo *player);

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
		bool disarmBeacon(RenX::PlayerInfo *player);

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
		bool changeTeam(RenX::PlayerInfo *player, bool resetCredits = true);

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
		void setPrefix(const Jupiter::ReadableString &prefix);

		/**
		* @brief Fetches a server's game command prefix.
		*
		* @return The server's game command prefix.
		*/
		const Jupiter::ReadableString &getCommandPrefix() const;

		/**
		* @brief Sets a server's game command prefix.
		*
		* @param prefix Prefix to use.
		*/
		void setCommandPrefix(const Jupiter::ReadableString &prefix);

		/**
		* @brief Fetches the rules of a server.
		*
		* @return String containing the rules of the server.
		*/
		const Jupiter::ReadableString &getRules() const;

		/**
		* @brief Sets the rules of a server.
		*
		* @param rules Rules for the server to be used.
		*/
		void setRules(const Jupiter::ReadableString &rules);

		/**
		* @brief Fetches the hostname of a server.
		*
		* @return String containing the hostname of the server.
		*/
		const Jupiter::ReadableString &getHostname() const;

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
		const Jupiter::ReadableString &getSocketHostname() const;

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
		time_t getLastAttempt() const;

		/**
		* @brief Fetches the time delay between connection attempts.
		*
		* @return Time delay between connection attempts.
		*/
		time_t getDelay() const;

		/**
		* @brief Checks if the server has a game password.
		*
		* @return True if the game is passworded, false otherwise.
		*/
		bool isPassworded() const;

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
		const Jupiter::ReadableString &getUser() const;

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
		const Jupiter::ReadableString &getMap() const;

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
		GameCommand *getCommand(const Jupiter::ReadableString &trigger) const;

		/**
		* @brief Returns the number of commands in the command list.
		*
		* @return Number of commands in the command list.
		*/
		unsigned int getCommandCount() const;

		/**
		* @brief Triggers commands matching a specified trigger.
		*
		* @param trigger Trigger of the command to fire.
		* @return Number of commands triggered.
		*/
		unsigned int triggerCommand(const Jupiter::ReadableString &trigger, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters);

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
		bool removeCommand(const Jupiter::ReadableString &trigger);

		/**
		* @brief Prototype of UUID calculation function.
		* Note: Every player's UUID should be either unique, or empty.
		*
		* @param player Player to calculate UUID of
		* @return UUID calculated from player.
		*/
		typedef Jupiter::StringS(*uuid_func)(RenX::PlayerInfo *player);

		/**
		* @brief Sets the player UUID calculation function.
		* Note: This also recalculates the UUIDs of all current players.
		*
		* @param function Function to calculate UUIDs with.
		*/
		void setUUIDFunction(uuid_func);

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
		void setUUID(RenX::PlayerInfo *player, const Jupiter::ReadableString &uuid);

		/**
		* @brief Changes a player's UUID only if it is different than their current UUID
		*
		* @param player Player with UUID to change
		* @param uuid Player's new UUID
		* @return True if the UUIDs did not match, false otherwise.
		*/
		bool setUUIDIfDifferent(RenX::PlayerInfo *player, const Jupiter::ReadableString &uuid);

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
		*/
		void disconnect();

		/**
		* @brief Connects to the server's RCON interface.
		*
		* @return True if the socket successfully connected, false otherwise.
		*/
		bool connect();

		/**
		* @brief Reconnects to the server's RCON interface.
		*
		* @return True if the socket successfully connected, false otherwise.
		*/
		bool reconnect();

		/**
		* @brief Deletes all of the data about a server (such as players).
		* This is mostly for use with the reconnect mechanism.
		*/
		void wipeData();

		/**
		* @brief Fetches the RCON version number, or 0 or none has been set.
		*
		* @return RCON version number
		*/
		unsigned int getVersion() const;

		/**
		* @brief Fetches the game version string, or an empty string if none has been set.
		*
		* @return String containing the Game version
		*/
		const Jupiter::ReadableString &getGameVersion() const;

		/**
		* @brief Fetches the RCON user name.
		*
		* @return String containing the RCON user name
		*/
		const Jupiter::ReadableString &getRCONUsername() const;

		/**
		* @brief Creates a server object using the provided socket, and loads settings from the specified configuration section.
		*
		* @param socket Initialized Socket to use for the connection
		* @param configurationSection Configuration section to load from.
		*/
		Server(Jupiter::Socket &&socket, const Jupiter::ReadableString &configurationSection);

		/**
		* @brief Creates the Server object, and loads settings from the specified configuration section.
		*
		* @param configurationSection Configuration section to load from.
		*/
		Server(const Jupiter::ReadableString &configurationSection);

		/**
		* @brief Destructor for Server class.
		*/
		~Server();

	/** Private members */
	private:
		void init();

		/** Tracking variables */
		bool connected = false;
		bool seamless = false;
		bool passworded = false;
		bool needsCList = false;
		bool silenceParts = false;
		bool silenceJoins = false;
		bool firstGame = true;
		bool firstKill = false;
		bool firstDeath = false;
		bool firstAction = false;
		bool awaitingPong = false;
		unsigned int rconVersion = 0;
		time_t lastAttempt = 0;
		int attempts = 0;
		std::chrono::steady_clock::time_point lastClientListUpdate = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point lastBuildingListUpdate = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point lastActivity = std::chrono::steady_clock::now();
		Jupiter::String lastLine;
		Jupiter::StringS commandListFormat;
		Jupiter::StringS gameVersion;
		Jupiter::ArrayList<RenX::GameCommand> commands;

		/** Configuration variables */
		unsigned short port;
		int logChanType;
		int adminLogChanType;
		time_t delay;
		int maxAttempts;
		std::chrono::milliseconds clientUpdateRate;
		std::chrono::milliseconds buildingUpdateRate;
		std::chrono::milliseconds pingRate;
		std::chrono::milliseconds pingTimeoutThreshold;
		int steamFormat; /** 16 = hex, 10 = base 10, 8 = octal, -2 = SteamID 2, -3 = SteamID 3 */
		bool rconBan;
		bool localBan;
		bool localSteamBan;
		bool localIPBan;
		bool localNameBan;
		bool neverSay;
		uuid_func calc_uuid;
		Jupiter::TCPSocket sock;
		Jupiter::CStringS clientHostname;
		Jupiter::CStringS hostname;
		Jupiter::StringS pass;
		Jupiter::StringS configSection;
		Jupiter::StringS rules;
		Jupiter::StringS IRCPrefix;
		Jupiter::StringS CommandPrefix;
		Jupiter::StringS rconUser;
		Jupiter::StringS serverName;
		Jupiter::StringS lastCommand;
		Jupiter::StringS lastCommandParams;
		Jupiter::StringS map;
		Jupiter::INIFile::Section *commandAccessLevels;
		Jupiter::INIFile::Section *commandAliases;
	};

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_SERVER_H_HEADER