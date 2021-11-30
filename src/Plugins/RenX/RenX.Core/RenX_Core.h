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

#if !defined _RENX_CORE_H_HEADER
#define _RENX_CORE_H_HEADER

/**
 * @file RenX_Core.h
 * @brief Provides Renegade-X RCON interaction.
 */

#include "Jupiter/Plugin.h"
#include "Jupiter/Config.h"
#include "RenX.h"
#include "RenX_Server.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	/** Forward delcarations */
	struct PlayerInfo;
	class Server;
	class Plugin;
	class GameCommand;

	/**
	* @brief Provides an interface for the Renegade-X RCON protocol.
	* The core manages the servers its connected to. Otherwise, it's
	* the same as any other Jupiter plugin.
	*/
	class RENX_API Core : public Jupiter::Plugin
	{
	public:

		/**
		* @brief Loops through the Renegade-X servers.
		* This should only be called by main().
		*
		* @return Result of Jupiter::Plugin::Think().
		*/
		int think() override;

		/**
		* @brief Initializes RenX.Core
		*
		* @return True.
		*/
		bool initialize() override;

		/**
		* @brief Sends a command to all servers of a specific type.
		*
		* @param type Server type to send the command to.
		* @param data Command to send.
		* @return Number of servers sent to on success, less than or equal to 0 otherwise.
		*/
		size_t send(int type, std::string_view data);

		/**
		* @brief Adds a server to the server list.
		*
		* @param server Server to add to the list.
		*/
		void addServer(std::unique_ptr<RenX::Server> server);

		/**
		* @brief Fetches a server's index.
		*
		* @return Index of the server if it exists, -1 otherwise.
		*/
		size_t getServerIndex(RenX::Server *server);

		/**
		* @brief Fetches a server based on its index.
		*
		* @param index Index of the server
		* @return Server located at the specified index.
		*/
		RenX::Server *getServer(size_t index);

		/**
		* @brief Fetches the list of servers
		* Note: This copies the array of pointers, not the objects themselves.
		*
		* @return Copy of the list of servers.
		*/
		std::vector<RenX::Server*> getServers();

		/**
		* @brief Constructs a list of servers based on their type.
		*
		* @param type Type of servers to fetch.
		* @return vector of servers with the same type.
		*/
		std::vector<RenX::Server*> getServers(int type);

		/**
		* @brief Removes a server based on its index.
		*
		* @param index Index of the server
		*/
		void removeServer(unsigned int index);

		/**
		* @brief Removes a server based on its data.
		*
		* @param server Server to be removed.
		* @return Index of the server removed.
		*/
		size_t removeServer(RenX::Server *server);

		/**
		* @brief Checks is a server is in the server list.
		*
		* @param server Pointer to the server to look for.
		* @return True if the server is in the list, false otherwise.
		*/
		bool hasServer(RenX::Server *server);

		/**
		* @brief Fetches the number of servers.
		*
		* @return Number of servers.
		*/
		size_t getServerCount();

		/**
		* @brief Fetches the Renegade-X plugins currently loaded.
		*
		* @return vector containing pointers to the plugins.
		*/
		std::vector<RenX::Plugin*>& getPlugins();

		/**
		* @brief Fetches the commands settings file.
		*
		* @return Commands settings configuration file.
		*/
		Jupiter::Config &getCommandsFile();

		/**
		* @brief Copys a command, and passes it to each server.
		*
		* @param command Command to copy.
		* @return Number of commands added.
		*/
		size_t addCommand(GameCommand *command);

		/**
		* @brief Performs a ban check on every player on each server, and kicks as appropriate.
		*/
		void banCheck();

		/**
		* Destructor for the Core class.
		*/
		~Core();

		Core() = default;
		Core(const Core&) = delete;
		Core& operator=(const Core&) = delete;

	private:
		/** Inaccessible private members */
		std::vector<std::unique_ptr<RenX::Server>> m_servers;
		std::vector<RenX::Plugin*> m_plugins;
		Jupiter::INIConfig m_commandsFile;
	};

	RENX_API Core *getCore();

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_CORE_H_HEADER