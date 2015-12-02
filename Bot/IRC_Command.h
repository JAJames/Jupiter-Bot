/**
 * Copyright (C) 2013-2015 Jessica James.
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

#include "Jupiter/Command.h"
#include "Jupiter/IRC_Client.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter/String.h"
#include "Jupiter_Bot.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "Generic_Command.h"

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

/** GenericCommand to IRCCommand conversion */

/** Generates an IRC command from a generic command. */
template<typename T> class Generic_Command_As_IRC_Command : public IRCCommand
{
public:
	virtual void trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) override;
	virtual const Jupiter::ReadableString &getHelp(const Jupiter::ReadableString &parameters) override;

	void copyTriggers();
	Generic_Command_As_IRC_Command();
	Generic_Command_As_IRC_Command(const Generic_Command_As_IRC_Command<T> &cmd);
};

template<typename T> Generic_Command_As_IRC_Command<T>::Generic_Command_As_IRC_Command() : IRCCommand()
{
	Generic_Command_As_IRC_Command<T>::copyTriggers();
}

template<typename T> Generic_Command_As_IRC_Command<T>::Generic_Command_As_IRC_Command(const Generic_Command_As_IRC_Command<T> &cmd) : IRCCommand(cmd)
{
	Generic_Command_As_IRC_Command<T>::copyTriggers();
}

template<typename T> void Generic_Command_As_IRC_Command<T>::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	GenericCommand::ResponseLine *del;
	GenericCommand::ResponseLine *ret = T::instance.trigger(parameters);
	while (ret != nullptr)
	{
		switch (ret->type)
		{
		case GenericCommand::DisplayType::PublicSuccess:
		case GenericCommand::DisplayType::PublicError:
			source->sendMessage(channel, ret->response);
			break;
		case GenericCommand::DisplayType::PrivateSuccess:
		case GenericCommand::DisplayType::PrivateError:
			source->sendNotice(nick, ret->response);
			break;
		default:
			source->sendMessage(nick, ret->response);
			break;
		}
		del = ret;
		ret = ret->next;
		delete del;
	}
}

template<typename T> const Jupiter::ReadableString &Generic_Command_As_IRC_Command<T>::getHelp(const Jupiter::ReadableString &parameters)
{
	return T::instance.getHelp(parameters);
}

template<typename T> void Generic_Command_As_IRC_Command<T>::copyTriggers()
{
	size_t index = 0;
	while (index != T::instance.getTriggerCount())
		this->addTrigger(T::instance.getTrigger(index++));
}

/** Defines the core of an IRC command's declaration. This should be included in every Generic to IRC command conversion. */
#define GENERIC_COMMAND_AS_IRC_COMMAND_2_BASE(CLASS, NEW_CLASS) \
	public: \
	void create(); \
	IRCCommand *copy() override; \
	NEW_CLASS() : Generic_Command_As_IRC_Command< CLASS >() { \
		this->create(); \
		if (serverManager != nullptr) serverManager->addCommand(this); } \
	NEW_CLASS(const NEW_CLASS &cmd) : Generic_Command_As_IRC_Command< CLASS >(cmd) { this->create(); }

/** Generates an IRC command from a generic command. */
#define GENERIC_COMMAND_AS_IRC_COMMAND_2(CLASS, NEW_CLASS) \
	class NEW_CLASS : public Generic_Command_As_IRC_Command< CLASS > { \
	GENERIC_COMMAND_AS_IRC_COMMAND_2_BASE(CLASS, NEW_CLASS) }; \
	IRC_COMMAND_INIT(NEW_CLASS)

/** Generates an IRC command from a generic command. */
#define GENERIC_COMMAND_AS_IRC_COMMAND(CLASS) \
	GENERIC_COMMAND_AS_IRC_COMMAND_2(CLASS, CLASS ## _AS_IRC_COMMAND);

/** Generates an IRC command from a generic command, and defines a default create() function. */
#define GENERIC_COMMAND_AS_IRC_COMMAND_NO_CREATE(CLASS) \
	GENERIC_COMMAND_AS_IRC_COMMAND(CLASS) \
	void CLASS ## _AS_IRC_COMMAND::create() {}

/** Generates an IRC command from a generic command, and defines an access-setting create() function. */
#define GENERIC_COMMAND_AS_IRC_COMMAND_ACCESS_CREATE(CLASS, ACCESS_LEVEL) \
	GENERIC_COMMAND_AS_IRC_COMMAND(CLASS) \
	void CLASS ## _AS_IRC_COMMAND::create() { this->setAccessLevel(ACCESS_LEVEL); }

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _IRC_COMMAND_H_HEADER