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

#if !defined _CONSOLE_COMMAND_H_HEADER
#define _CONSOLE_COMMAND_H_HEADER

/**
* @file Console_Command.h
* @brief Provides an extendable command system specialized for console-based commands.
*/

#include "Jupiter/GenericCommand.h"
#include "Jupiter_Bot.h"

class ConsoleCommand;

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

/** Console command list; weak pointers */
JUPITER_BOT_API extern std::vector<ConsoleCommand*>& consoleCommands;

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
	virtual void trigger(std::string_view input) = 0;

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
JUPITER_BOT_API extern ConsoleCommand *getConsoleCommand(std::string_view trigger);

/** Console Command Macros */

/** Defines the core of a console command's declaration. This should be included in every console command. */
#define BASE_CONSOLE_COMMAND(CLASS) \
	public: \
	CLASS(); \
	void trigger(std::string_view parameters); \
	std::string_view getHelp(std::string_view parameters);

/** Expands to become the entire declaration for a console command. In most cases, this will be sufficient. */
#define GENERIC_CONSOLE_COMMAND(CLASS) \
class CLASS : public ConsoleCommand { \
	BASE_CONSOLE_COMMAND(CLASS) \
};

/** Instantiates a console command. */
#define CONSOLE_COMMAND_INIT(CLASS) \
	CLASS CLASS ## _instance;

/** Generates a console command from a generic command. */
template<typename T> class Generic_Command_As_Console_Command : public ConsoleCommand
{
public:
	void trigger(std::string_view parameters);
	std::string_view getHelp(std::string_view parameters);

	Generic_Command_As_Console_Command();
};

template <typename T> Generic_Command_As_Console_Command<T>::Generic_Command_As_Console_Command() : ConsoleCommand() {
	size_t index = 0;
	while (index != T::instance.getTriggerCount()) {
		this->addTrigger(T::instance.getTrigger(index++));
	}
}

template<typename T> void Generic_Command_As_Console_Command<T>::trigger(std::string_view parameters) {
	std::unique_ptr<Jupiter::GenericCommand::ResponseLine> response_line{ T::instance.trigger(parameters) };
	while (response_line != nullptr) {
		auto& out_stream = response_line->type == Jupiter::GenericCommand::DisplayType::PublicError
			|| response_line->type == Jupiter::GenericCommand::DisplayType::PrivateError ? std::cerr : std::cout;

		out_stream << response_line->response << std::endl;
		response_line.reset(response_line->next);
	}
}

template<typename T> std::string_view Generic_Command_As_Console_Command<T>::getHelp(std::string_view parameters)
{
	return T::instance.getHelp(parameters);
}

/** Generates a console command implementation from a generic command. */
#define GENERIC_COMMAND_AS_CONSOLE_COMMAND(CLASS) \
	Generic_Command_As_Console_Command< CLASS > CLASS ## _AS_CONSOLE_COMMAND_instance;

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _CONSOLE_COMMAND_H_HEADER
