/**
 * Copyright (C) 2014-2015 Jessica James.
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
#include "Jupiter/INIFile.h"
#include "RenX.h"

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
		virtual int think();

		/**
		* @brief Returns the name of the plugin.
		* @see Jupiter::Plugin::getName().
		*
		* @return Name of the plugin in a string.
		*/
		const Jupiter::ReadableString &getName() override { return name; }

		/**
		* @brief Sends a command to all servers of a specific type.
		*
		* @param type Server type to send the command to.
		* @param data Command to send.
		* @return Number of servers sent to on success, less than or equal to 0 otherwise.
		*/
		unsigned int send(int type, const Jupiter::ReadableString &data);

		/**
		* @brief Adds a server to the server list.
		*
		* @param server Server to add to the list.
		*/
		void addServer(RenX::Server *server);

		/**
		* @brief Fetches a server's index.
		*
		* @return Index of the server if it exists, -1 otherwise.
		*/
		int getServerIndex(RenX::Server *server);

		/**
		* @brief Fetches a server based on its index.
		*
		* @param index Index of the server
		* @return Server located at the specified index.
		*/
		RenX::Server *getServer(unsigned int index);

		/**
		* @brief Constructs a list of servers based on their type.
		*
		* @param type Type of servers to fetch.
		* @return ArrayList of servers with the same type.
		*/
		Jupiter::ArrayList<RenX::Server> getServers(int type);

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
		int removeServer(RenX::Server *server);

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
		unsigned int getServerCount();

		/**
		* @brief Fetches the Renegade-X plugins currently loaded.
		*
		* @return ArrayList containing pointers to the plugins.
		*/
		Jupiter::ArrayList<RenX::Plugin> *getPlugins();

		/**
		* @brief Fetches the translations override file.
		*
		* @return INIFile containing translations overrides.
		*/
		Jupiter::INIFile &getTranslationsFile();

		/**
		* @brief Fetches the commands settings file.
		*
		* @return Commands settings configuration file.
		*/
		Jupiter::INIFile &getCommandsFile();

		/**
		* @brief Copys a command, and passes it to each server.
		*
		* @param command Command to copy.
		* @return Number of commands added.
		*/
		int addCommand(GameCommand *command);

		/**
		* @brief Initializes the Core.
		*/
		void init();

		/**
		* Destructor for the Core class.
		*/
		~Core();

	private:
		/** Inaccessible private members */
		STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Core");
		Jupiter::ArrayList<RenX::Server> servers;
		Jupiter::ArrayList<RenX::Plugin> plugins;
		Jupiter::INIFile translationsFile;
		Jupiter::INIFile commandsFile;
	};

	RENX_API Core *getCore();

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_CORE_H_HEADER