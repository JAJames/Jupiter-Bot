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

#if !defined _SERVERMANAGER_H_HEADER
#define _SERVERMANAGER_H_HEADER

/**
 * @file ServerManager.h
 * @brief Provides a system for controlling and affecting multiple IRC connections simultaneously.
 */

#include <memory>
#include "Jupiter_Bot.h"
#include "Jupiter/Thinker.h"
#include "Jupiter/Readable_String.h"

/** Forward declarations */
class IRC_Bot;
class IRCCommand;

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/**
* @brief Provides a system for controlling and affecting multiple IRC connections simultaneously.
*/
class JUPITER_BOT_API ServerManager : public Jupiter::Thinker
{
public:
	/**
	* @brief Calls think() on all of the managed connections.
	*
	* @return false if there are still active connections, true otherwise.
	*/
	int think();

	/**
	* @brief Adds a copy of a command to each managed server.
	*
	* @param command Command to copy to each server.
	* @return Number of servers copied to.
	*/
	size_t addCommand(IRCCommand *command);

	/**
	* @brief Removes any command which matches an input command's primary trigger.
	*
	* @param command Command to get the trigger of.
	* @return Number of servers which had a command removed.
	*/
	size_t removeCommand(IRCCommand *command);

	/**
	* @brief Removes any command which matches a trigger.
	*
	* @param command Trigger to match against.
	* @return Number of servers which had a command removed.
	*/
	size_t removeCommand(std::string_view command);

	/**
	* @brief Called when m_config is rehashed
	*/
	void OnConfigRehash();

	/**
	* @brief Syncs command access levels from the configuration file.
	*
	* @return Number of servers sync'd.
	*/
	size_t syncCommands();

	/**
	* @brief Fetches a server based on its configuration section
	*
	* @param serverConfig Configuration section to match against.
	* @return Pointer to a matching server instance on success, nullptr otherwise.
	*/
	IRC_Bot *getServer(std::string_view serverConfig);

	/**
	* @brief Fetches a server based on its index.
	*
	* @param index Index of the server to fetch.
	* @return Pointer to a server on success, nullptr otherwise.
	*/
	IRC_Bot *getServer(size_t index);

	/**
	* @brief Adds a server based on its configuration section to the list.
	*
	* @return True if a socket connection was successfully established, false otherwise.
	*/
	bool addServer(std::string_view serverConfig);

	/**
	* @brief Removes a server from the manager, based on its index.
	*
	* @param index Index of the server to remove.
	* @return True if a server was removed, false otherwise.
	*/
	bool freeServer(size_t index);

	/**
	* @brief Removes a server from the manager, based on its data.
	*
	* @param server Data of the server to remove.
	* @return True if a server was removed, false otherwise.
	*/
	bool freeServer(IRC_Bot *server);

	/**
	* @brief Removes a server from the manager, based on its configuration section.
	*
	* @param serverConfig Configuration section of the server to remove.
	* @return True if a server was removed, false otherwise.
	*/
	bool freeServer(std::string_view serverConfig);

	/**
	* @brief Returns the number of servers in the list.
	*
	* @return Number of servers in the list.
	*/
	size_t size();

	/**
	* @brief Fetches the configuration file being used
	*
	* @return Configuration file being used
	*/
	inline Jupiter::Config &getConfig() const { return *this->m_config; };

	/**
	* @brief Sets the configuration file to use
	*
	* @param Reference to the config file to use
	*/
	inline void setConfig(Jupiter::Config &in_config) { this->m_config = &in_config; };

	/**
	* Destructor for the ServerManager class.
	*/
	virtual ~ServerManager();

	ServerManager() = default;
	ServerManager& operator=(const ServerManager&) = delete;
	ServerManager(const ServerManager&) = delete;

private:
	/** Underlying vector of servers */
	std::vector<std::unique_ptr<IRC_Bot>> m_servers;

	/** Config to read data from */
	Jupiter::Config* m_config = Jupiter::g_config;
};

/** Pointer to an instance of the server manager. Note: DO NOT DELETE OR FREE THIS POINTER. */
JUPITER_BOT_API extern ServerManager *serverManager;

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _SERVERMANAGER_H_HEADER