/**
 * Copyright (C) 2013-2014 Justin James.
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

#if !defined _IRC_COMMAND_H_HEADER
#define _IRC_COMMAND_H_HEADER

/**
* @file IRC_Command.h
* @brief Provides an extendable command system specialized for IRC chat-based commands.
*/

#include "Jupiter/Command.h"
#include "Jupiter/IRC_Client.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter/String.h"
#include "ServerManager.h"
#include "Jupiter_Bot.h"

class IRC_Bot;
class IRCCommand;

/*
* Initial access level is always 0. Change this using setAccessLevel()
* There are no default triggers. Add triggers using addTrigger(trigger)
* When your command is triggered, trigger(channel, nick, parameters) will be called.
* You must provide command-specific help for your command, through getHelp().
* The first trigger added is the trigger which is displayed to the user in the help command.
*/

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/** IRC Master Command List */
JUPITER_BOT_API extern Jupiter::ArrayList<IRCCommand> *IRCMasterCommandList;

/**
* @brief Provides the basis for IRC commands.
* Note: This will likely be moved to a separate file in the future.
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
	* @brief Called when the command is to be executed.
	*
	* @param source IRC Server from which the command is being executed.
	* @param channel Name of the channel from which the message originated.
	* @param nick Nickname of the user who executed the command.
	* @param parameters Parameters specified with the command.
	*/
	virtual void trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) = 0;

	/**
	* @brief Called when the command is intially created. Define triggers and access levels here.
	*/
	virtual void create() = 0;

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

/** IRC Command Macros */

/** Defines the core of an IRC command's declaration. This should be included in every IRC command. */
#define BASE_IRC_COMMAND(CLASS) \
	public: \
	void trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters); \
	const Jupiter::ReadableString &getHelp(const Jupiter::ReadableString &parameters); \
	CLASS *copy(); \
	void create(); \
	CLASS() { this->create(); if (serverManager != nullptr) serverManager->addCommand(this); }

/** Expands to become the entire declaration for an IRC command. In most cases, this will be sufficient. */
#define GENERIC_IRC_COMMAND(CLASS) \
class CLASS : public IRCCommand { \
	BASE_IRC_COMMAND(CLASS) \
	CLASS(CLASS &c) : IRCCommand(c) { this->create(); } };

/** Instantiates an IRC command, and also defines neccessary core functions for compatibility. */
#define IRC_COMMAND_INIT(CLASS) \
	CLASS CLASS ## _instance; \
	CLASS *CLASS::copy() { return new CLASS(*this); }

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _IRC_COMMAND_H_HEADER