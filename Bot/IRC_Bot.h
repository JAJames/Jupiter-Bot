/**
 * Copyright (C) 2013-2015 Justin James.
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
 * Written by Justin James <justin.aj@hotmail.com>
 */

#if !defined _IRC_BOT_H_HEADER
#define _IRC_BOT_H_HEADER

/**
 * @file IRC_Bot.h
 * @brief Provides extendable bot-like interfacing with the IRC client.
 */

#include "Jupiter_Bot.h"
#include "Jupiter/IRC_Client.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter/Rehash.h"
#include "Jupiter/String.h"

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
class JUPITER_BOT_API IRC_Bot : public Jupiter::IRC::Client, public Jupiter::Rehashable
{
public:

	/**
	* @brief Pulls the configured access levels from the config and applies them.
	*/
	void setCommandAccessLevels();

	/**
	* @brief Adds a command to the command list.
	*/
	void addCommand(IRCCommand *cmd);

	/**
	* @brief Removes a command from the command list.
	*
	* @param trigger Trigger of the command to remove
	* @return True if a command is removed, false otherwise.
	*/
	bool freeCommand(const Jupiter::ReadableString &trigger);

	/**
	* @brief Gets the index of a command.
	*
	* @param trigger Trigger of the command to find.
	* @return Index of the first match for the trigger if found, -1 otherwise.
	*/
	int getCommandIndex(const Jupiter::ReadableString &trigger) const;

	/**
	* @return Gets a command.
	*
	* @param trigger Trigger of the command to find.
	* @return First command using the specified trigger, nullptr otherwise.
	*/
	IRCCommand *getCommand(const Jupiter::ReadableString &trigger) const;

	/**
	* @brief Creates and returns an ArrayList of IRC Commands with a specified access level.
	*
	* @param chan Channel which access levels are set.
	* @param access Access level to match.
	* @return An ArrayList containing pointers to all of the matching commands.
	*/
	Jupiter::ArrayList<IRCCommand> getAccessCommands(Jupiter::IRC::Client::Channel *chan, int access);

	/**
	* @brief Gets the triggers of all the commands in an ArrayList, and adds them to a space-deliminated string.
	*
	* @param cmds Commands to construct the string with.
	* @return A string containing the triggers of the commands in a space-deliminated list.
	*/
	static Jupiter::StringL getTriggers(Jupiter::ArrayList<IRCCommand> cmds);

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
	IRC_Bot(const Jupiter::ReadableString &configSection);

	/** Destructor for IRC_Bot */
	~IRC_Bot();

	/** Overloaded events from Jupiter::IRC::Client */
protected:

	void OnChat(const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message);

	/** Private members for internal usage */
private:
	Jupiter::ArrayList<IRCCommand> commands;
	Jupiter::StringS commandPrefix;
};

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _IRC_BOT_H_HEADER