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

#if !defined _RENX_SERVER_H_HEADER
#define _RENX_SERVER_H_HEADER

/**
 * @file RenX_Server.h
 * @brief Defines the Server class.
 */

#include "Jupiter/TCPSocket.h"
#include "Jupiter/DLList.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter/String.h"
#include "Jupiter/CString.h"
#include "Jupiter/INIFile.h"
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
	class GameCommand;
	class Core;

	/**
	* @brief Represents a connection to an individiaul Renegade-X server.
	* There are often more than one of these, such as when communities run multiple servers.
	*/
	class RENX_API Server
	{
		friend class RenX::Core;
	public:
		Jupiter::DLList<RenX::PlayerInfo> players; /** A list of players in the server */
		Jupiter::INIFile varData; /** This may be replaced later with a more dedicated type. */

		/**
		* @brief Checks if the server is connected to RCON.
		*
		* @return True if the server is connected, false otherwise.
		*/
		bool isConnected() const;

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
		int sendMessage(RenX::PlayerInfo *player, const Jupiter::ReadableString &message);

		/**
		* @brief Sends data to the server.
		*
		* @param data String of data to send.
		* @return The number of bytes sent on success, less than or equal to zero otherwise.
		*/
		int sendData(const Jupiter::ReadableString &data);

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
		* @brief Kicks a player from the server.
		*
		* @param id Player ID of the player to kick.
		*/
		void kickPlayer(int id);

		/**
		* @brief Kicks a player from the server.
		*
		* @param player Data of the player to kick.
		*/
		void kickPlayer(const RenX::PlayerInfo *player);

		/**
		* @brief Bans a player from the server.
		*
		* @param id Player ID of the player to ban.
		*/
		void banPlayer(int id);

		/**
		* @brief Bans a player from the server.
		*
		* @param player Data of the player to ban.
		*/
		void banPlayer(const RenX::PlayerInfo *player);

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
		* @brief Fetches the password of a server.
		*
		* @return String containing the password of the server.
		*/
		const Jupiter::ReadableString &getPassword() const;

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
		* @brief Formats and sends a message to a server's corresponding public channels.
		*
		* @param server Server corresponding to channel types.
		* @param fmt String containing the format specifiers indicating what message to send.
		*/
		void sendPubChan(const char *fmt, ...) const;

		/**
		* @brief Formats and sends a message to a server's corresponding adminstrative channels.
		*
		* @param server Server corresponding to channel types.
		* @param fmt String containing the format specifiers indicating what message to send.
		*/
		void sendAdmChan(const char *fmt, ...) const;

		/**
		* @brief Formats and sends a message to a server's corresponding channels.
		*
		* @param server Server corresponding to channel types.
		* @param fmt String containing the format specifiers indicating what message to send.
		*/
		void sendLogChan(const char *fmt, ...) const;

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
		*/
		unsigned int getVersion() const;

		/**
		* @brief Fetches the game version string, or an empty string if none has been set.
		*/
		const Jupiter::ReadableString &getGameVersion() const;

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
		bool connected = false;
		bool needsCList = false;
		unsigned int rconVersion = 0;
		unsigned short port;
		int logChanType;
		int adminLogChanType;
		time_t lastAttempt = 0;
		time_t delay;
		Jupiter::TCPSocket sock;
		Jupiter::CStringS clientHostname;
		Jupiter::CStringS hostname;
		Jupiter::StringS pass;
		Jupiter::StringS configSection;
		Jupiter::StringS rules;
		Jupiter::StringS IRCPrefix;
		Jupiter::StringS CommandPrefix;
		Jupiter::StringS gameVersion;
		Jupiter::ArrayList<RenX::GameCommand> commands;
	};

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_SERVER_H_HEADER