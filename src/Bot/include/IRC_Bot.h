/**
 * Copyright (C) 2013-2021 Jessica James.
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

#if !defined _IRC_BOT_H_HEADER
#define _IRC_BOT_H_HEADER

/**
 * @file IRC_Bot.h
 * @brief Provides extendable bot-like interfacing with the IRC client.
 */

#include "Jupiter_Bot.h"
#include "Jupiter/IRC_Client.h"
#include "Jupiter/Rehash.h"
#include "Jupiter/String.hpp"

/** Forward Declaration */
class IRCCommand;

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/**
* @brief Provies extendable bot-like interfacing with the IRC client.
*/
class JUPITER_BOT_API IRC_Bot : public Jupiter::IRC::Client
{
public:

	/**
	* @brief Pulls the configured access levels from the config and applies them.
	*/
	void setCommandAccessLevels(IRCCommand *in_command = nullptr);

	/**
	* @brief Adds a command to the command list.
	*/
	void addCommand(IRCCommand *in_command);

	/**
	* @brief Removes a command from the command list.
	*
	* @param trigger Trigger of the command to remove
	* @return True if a command is removed, false otherwise.
	*/
	bool freeCommand(const Jupiter::ReadableString &trigger);

	/**
	* @return Gets a command.
	*
	* @param trigger Trigger of the command to find.
	* @return First command using the specified trigger, nullptr otherwise.
	*/
	IRCCommand *getCommand(const Jupiter::ReadableString &trigger) const;

	/**
	* @brief Creates and returns a vector of IRC Commands with a specified access level.
	*
	* @param chan Channel which access levels are set.
	* @param access Access level to match.
	* @return Vector containing pointers to all of the matching commands.
	*/
	std::vector<IRCCommand*> getAccessCommands(Jupiter::IRC::Client::Channel *chan, int access);

	/**
	* @brief Gets the triggers of all the commands in a vector, and adds them to a space-deliminated string.
	*
	* @param cmds Commands to construct the string with.
	* @return A string containing the triggers of the commands in a space-deliminated list.
	*/
	static Jupiter::StringL getTriggers(std::vector<IRCCommand*> &cmds);

	/**
	* @brief Reloads some settings.
	*
	* @return 0 on success, 1 otherwise.
	*/
	int OnRehash();

	/**
	* @brief Shouldn't ever happen; returns removed.
	*/
	bool OnBadRehash(bool removed) { return removed; };

	/** Constructor for IRC_Bot */
	IRC_Bot(Jupiter::Config *in_primary_section, Jupiter::Config *in_secondary_section);

	/** Destructor for IRC_Bot */
	~IRC_Bot();

	IRC_Bot& operator=(const IRC_Bot&) = delete;
	IRC_Bot(const IRC_Bot&) = delete;

	/** Overloaded events from Jupiter::IRC::Client */
protected:

	void OnChat(const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message);

	/** Private members for internal usage */
private:
	std::vector<std::unique_ptr<IRCCommand>> m_commands;
	Jupiter::StringS m_commandPrefix;
};

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _IRC_BOT_H_HEADER