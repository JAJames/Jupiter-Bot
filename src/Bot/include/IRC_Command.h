/**
 * Copyright (C) 2013-2016 Jessica James.
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

#if !defined _IRC_COMMAND_H_HEADER
#define _IRC_COMMAND_H_HEADER

/**
* @file IRC_Command.h
* @brief Provides an extendable command system specialized for IRC chat-based commands.
*/

#include "Jupiter/GenericCommand.h"
#include "Jupiter/IRC_Client.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter/String.hpp"
#include "Jupiter_Bot.h"
#include "ServerManager.h"
#include "IRC_Bot.h"

class IRCCommand;

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/** IRC Master Command List */
JUPITER_BOT_API extern Jupiter::ArrayList<IRCCommand> *IRCMasterCommandList;

/**
* @brief Provides the basis for IRC commands.
* Initial access level is always 0. Change this using setAccessLevel()
* There are no default triggers. Add triggers using addTrigger(trigger)
* When your command is triggered, trigger(channel, nick, parameters) will be called.
* You must provide command-specific help for your command, through getHelp().
* The first trigger added is the trigger which is displayed to the user in the help command.
*/
class JUPITER_BOT_API IRCCommand : public Jupiter::Command
{
public:
	/**
	* @brief Fetches a command's default access level.
	*
	* @return Default access level.
	*/
	int getAccessLevel();

	/**
	* @brief Fetches a command's access level for a channel type.
	*
	* @return Access level.
	*/
	int getAccessLevel(int type);

	/**
	* @brief Fetches a command's access level for a specific channel by name.
	*
	* @return Access level.
	*/
	int getAccessLevel(const Jupiter::ReadableString &channel);

	/**
	* @brief Fetches a command's access level for a specific channel.
	*
	* @return Access level.
	*/
	int getAccessLevel(Jupiter::IRC::Client::Channel *channel);

	/**
	* @brief Sets a command's default access level.
	*
	* @param accessLevel Access level.
	*/
	void setAccessLevel(int accessLevel);

	/**
	* @brief Sets a command's access level for a channel type.
	*
	* @param type Type of channel for which to set access.
	* @param accessLevel Access level.
	*/
	void setAccessLevel(int type, int accessLevel);

	/**
	* @brief Sets a command's access level for a specific channel by name.
	*
	* @param channel Specific channel name for which to set access.
	* @param accessLevel Access level.
	*/
	void setAccessLevel(const Jupiter::ReadableString &channel, int accessLevel);

	/**
	* @brief Called when the command is intially created. Define triggers and access levels here.
	*/
	virtual void create();

	/**
	* @brief Called when the command is to be executed.
	*
	* @param source IRC Server from which the command is being executed.
	* @param channel Name of the channel from which the message originated.
	* @param nick Nickname of the user who executed the command.
	* @param parameters Parameters specified with the command.
	*/
	virtual void trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) = 0;

	/**
	* @brief Creates a copy of a command.
	* Note: This is primarily for internal usage.
	*
	* @return Copy of the command.
	*/
	virtual IRCCommand *copy() = 0;

	/**
	* @brief Default constructor for the IRC command class.
	* Generally, this is called only for master commands.
	*/
	IRCCommand();

	/**
	* @brief Copy constructor for the IRC command class.
	* Generally, this is called only for slave commands.
	*/
	IRCCommand(const IRCCommand &command);

	/**
	* @brief Destructor for the IRC command class.
	*/
	virtual ~IRCCommand();

	/** Public members */

	static IRC_Bot *active_server; /** Currently active IRC server */
	static IRC_Bot *selected_server; /** Currently selected IRC server */

	/** Private members */
private:
	int access; /** Default access level */

	struct TypeAccessPair
	{
		int type;
		int access;
	};
	Jupiter::ArrayList<IRCCommand::TypeAccessPair> types; /** Access levels for channel types */

	struct ChannelAccessPair
	{
		Jupiter::StringS channel;
		int access;
	};
	Jupiter::ArrayList<IRCCommand::ChannelAccessPair> channels; /** Access levels for specific channels */
};

class JUPITER_BOT_API GenericCommandWrapperIRCCommand : public IRCCommand
{
public:
	/**
	* @brief Triggers the underlying GenericCommand and outputs the result
	*
	* @param in_server IRC server to deliver result to
	* @param in_channel Name of the channel to deliver result to (if result is not 'Private' type)
	* @param in_nick Name of the user to deliver result to (if result is 'Private' type)
	* @param in_parameters Parameters to pass to the GenericCommand's trigger()
	*/
	void trigger(IRC_Bot *in_server, const Jupiter::ReadableString &in_channel, const Jupiter::ReadableString &in_nick, const Jupiter::ReadableString &in_parameters) override;

	/**
	* @brief Forwards the help message from the underlying GenericCommand
	*
	* @param in_parameters Parameters to forward to the GenericCommand's getHelp()
	* @return Help string from the GenericCommand
	*/
	const Jupiter::ReadableString &getHelp(const Jupiter::ReadableString &in_parameters) override;

	/**
	* @brief Copies the GenericCommandWrapperIRCCommand
	*
	* @return Copy of the GenericCommandWrapperIRCCommand
	*/
	IRCCommand *copy() override;

	/**
	* @brief Fetches the underlying GenericCommand
	*
	* @return GenericCommand this wrapper interfaces with
	*/
	const Jupiter::GenericCommand &getGenericCommand() const;

	/**
	* @brief Copy constructor for the GenericCommandWrapperIRCCommand class
	*/
	GenericCommandWrapperIRCCommand(const GenericCommandWrapperIRCCommand &in_command);

	/**
	* @brief Wrapper constructor for the GenericCommandWrapperIRCCommand class
	*/
	GenericCommandWrapperIRCCommand(Jupiter::GenericCommand &in_command);

private:
	Jupiter::GenericCommand *m_command;
};

/** IRC Command Macros */

/** Defines the core of an IRC command's declaration. This should be included in every IRC command. */
#define BASE_IRC_COMMAND(CLASS) \
	public: \
	void trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) override; \
	const Jupiter::ReadableString &getHelp(const Jupiter::ReadableString &parameters) override; \
	IRCCommand *copy() override; \
	void create() override; \
	CLASS() { this->create(); if (serverManager != nullptr) serverManager->addCommand(this); }

/** Expands to become the entire declaration for an IRC command. In most cases, this will be sufficient. */
#define GENERIC_IRC_COMMAND(CLASS) \
class CLASS : public IRCCommand { \
	BASE_IRC_COMMAND(CLASS) \
	CLASS(CLASS &c) : IRCCommand(c) { this->create(); } };

/** Instantiates an IRC command, and also defines neccessary core functions for compatibility. */
#define IRC_COMMAND_INIT(CLASS) \
	CLASS CLASS ## _instance; \
	IRCCommand *CLASS::copy() { return new CLASS(*this); }

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _IRC_COMMAND_H_HEADER
