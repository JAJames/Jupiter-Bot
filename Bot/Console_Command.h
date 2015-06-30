/**
 * Copyright (C) 2013-2015 Justin James.
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

#if !defined _CONSOLE_COMMAND_H_HEADER
#define _CONSOLE_COMMAND_H_HEADER

/**
* @file Console_Command.h
* @brief Provides an extendable command system specialized for console-based commands.
*/

#include "Jupiter/Command.h"
#include "Jupiter/ArrayList.h"
#include "Jupiter_Bot.h"
#include "Generic_Command.h"

class ConsoleCommand;

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/** Console command list */
JUPITER_BOT_API extern Jupiter::ArrayList<ConsoleCommand> *consoleCommands;

/**
* @brief Provides the basis for console commands.
* There are no default triggers. Add triggers using addTrigger(trigger)
* When your command is triggered, trigger(parameters) will be called.
* You must provide command-specific help for your command, through getHelp().
* The first trigger added is the trigger which is displayed to the user in "help" commands.
*/
class JUPITER_BOT_API ConsoleCommand : public Jupiter::Command
{
public:
	/**
	* @brief Called when the command is to be executed.
	*
	* @param input Parameters passed to the command by the user.
	*/
	virtual void trigger(const Jupiter::ReadableString &input) = 0;

	/**
	* @brief Default constructor for the console command class.
	*/
	ConsoleCommand();

	/**
	* @brief Destructor for the console command class.
	*/
	~ConsoleCommand();
};

/**
* @brief Fetches a console command based on its trigger.
*
* @param trigger Trigger of the command to fetch.
* @return A console command if it exists, nullptr otherwise.
*/
JUPITER_BOT_API extern ConsoleCommand *getConsoleCommand(const Jupiter::ReadableString &trigger);

/** Console Command Macros */

/** Defines the core of a console command's declaration. This should be included in every console command. */
#define BASE_CONSOLE_COMMAND(CLASS) \
	public: \
	CLASS(); \
	void trigger(const Jupiter::ReadableString &parameters); \
	const Jupiter::ReadableString &getHelp(const Jupiter::ReadableString &parameters);

/** Expands to become the entire declaration for a console command. In most cases, this will be sufficient. */
#define GENERIC_CONSOLE_COMMAND(CLASS) \
class CLASS : public ConsoleCommand { \
	BASE_CONSOLE_COMMAND(CLASS) \
};

/** Instantiates a console command. */
#define CONSOLE_COMMAND_INIT(CLASS) \
	CLASS CLASS ## _instance;

/** Generates a console command implementation from a generic command. */
#define GENERIC_COMMAND_AS_CONSOLE_COMMAND_IMPL(CLASS) \
	CLASS ## _AS_CONSOLE_COMMAND :: CLASS ## _AS_CONSOLE_COMMAND() { \
		size_t index = 0; \
		while (index != CLASS ## _instance.getTriggerCount()) this->addTrigger(CLASS ## _instance.getTrigger(index++)); } \
	void CLASS ## _AS_CONSOLE_COMMAND :: trigger(const Jupiter::ReadableString &parameters) { \
		GenericCommand::ResponseLine *del; \
		GenericCommand::ResponseLine *ret = CLASS ## _instance.trigger(parameters); \
		while (ret != nullptr) { \
			ret->response.println(ret->type == GenericCommand::DisplayType::PublicError || ret->type == GenericCommand::DisplayType::PrivateError ? stderr : stdout); \
			del = ret; ret = ret->next; delete del; } } \
	const Jupiter::ReadableString & CLASS ## _AS_CONSOLE_COMMAND :: getHelp(const Jupiter::ReadableString &parameters) { \
		return CLASS ## _instance.getHelp(parameters); } \
	CONSOLE_COMMAND_INIT(CLASS ## _AS_CONSOLE_COMMAND)

/** Generates a console command from a generic command. */
#define GENERIC_COMMAND_AS_CONSOLE_COMMAND(CLASS) \
	GENERIC_CONSOLE_COMMAND(CLASS ## _AS_CONSOLE_COMMAND) \
	GENERIC_COMMAND_AS_CONSOLE_COMMAND_IMPL(CLASS);



/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _CONSOLE_COMMAND_H_HEADER