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

#if !defined _CONSOLE_COMMAND_H_HEADER
#define _CONSOLE_COMMAND_H_HEADER

/**
* @file Console_Command.h
* @brief Provides an extendable command system specialized for console-based commands.
*/

#include "Jupiter_Bot.h"
#include "Jupiter/Command.h"
#include "Jupiter/ArrayList.h"

class Command;
class ConsoleCommand;

/*
* Initial access level is always 0. Change this using setAccessLevel()
* There are no default triggers. Add triggers using addTrigger(trigger)
* When your command is triggered, trigger(parameters) will be called.
* You must provide command-specific help for your command, through getHelp().
* The first trigger added is the trigger which is displayed to the user in the help command.
*/

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/** Console command list */
JUPITER_BOT_API extern Jupiter::ArrayList<ConsoleCommand> *consoleCommands;

/**
* @brief Provides the basis for console commands.
* Note: This will likely be moved to a separate file in the future.
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

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _CONSOLE_COMMAND_H_HEADER