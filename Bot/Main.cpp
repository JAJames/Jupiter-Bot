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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>
#include <thread>
#include <mutex>
#include "Jupiter/Functions.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/Queue.h"
#include "Jupiter/Socket.h"
#include "Jupiter/Plugin.h"
#include "Jupiter/Timer.h"
#include "IRC_Bot.h"
#include "ServerManager.h"
#include "Console_Command.h"
#include "IRC_Command.h"

using namespace Jupiter::literals;

#define INPUT_BUFFER_SIZE 2048

struct ConsoleInput
{
	Jupiter::String input;
	std::mutex input_mutex;
	bool awaiting_processing = false;

	ConsoleInput() : input(INPUT_BUFFER_SIZE) {}
} console_input;

void onTerminate()
{
	puts("Terminate signal received...");
}

void onExit()
{
	puts("Exit signal received; Cleaning up...");
	Jupiter::Socket::cleanup();
	puts("Clean-up complete. Closing...");
}

void inputLoop()
{
	char input[INPUT_BUFFER_SIZE];
	size_t input_length;
	while (ftell(stdin) != -1) // This can be expanded later to check for EBADF specifically.
	{
		fgets(input, sizeof(input), stdin);
		input_length = strcspn(input, "\r\n");

	check_input_processing:

		console_input.input_mutex.lock();
		if (console_input.awaiting_processing == false)
		{
			console_input.input.set(input, input_length);
			console_input.awaiting_processing = true;
			console_input.input_mutex.unlock();
		}
		else // User input received before previous input was processed.
		{
			std::this_thread::sleep_for((std::chrono::seconds(1)));
			goto check_input_processing;
		}
	}
}

int main(int argc, const char **args)
{
	atexit(onExit);
	std::set_terminate(onTerminate);
	std::thread inputThread(inputLoop);
	Jupiter::ReferenceString command;

	srand(static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));
	puts(Jupiter::copyright);
	const char *configFileName = CONFIG_INI;

	for (int i = 1; i < argc; i++)
	{
		if ("-help"_jrs.equalsi(args[i]))
		{
			puts("Help coming soon, to a theatre near you!");
			return 0;
		}
		else if ("-config"_jrs.equalsi(args[i]) && ++i < argc)
			configFileName = args[i];
		else if ("-pluginsdir"_jrs.equalsi(args[i]) && ++i < argc)
			Jupiter::Plugin::setDirectory(Jupiter::ReferenceString(args[i]));
		else if ("-configsdir"_jrs.equals(args[i]) && ++i < argc)
			Jupiter::Plugin::setConfigDirectory(Jupiter::ReferenceString(args[i]));
		else if ("-configFormat"_jrs.equalsi(args[i]) && ++i < argc)
			puts("Feature not yet supported!");
		else
			printf("Warning: Unknown command line argument \"%s\" specified. Ignoring...", args[i]);
	}

	puts("Loading config file...");
	if (!Jupiter::IRC::Client::Config->readFile(configFileName))
	{
		puts("Unable to read config file. Closing...");
		exit(0);
	}

	fputs("Config loaded. ", stdout);
	
	const Jupiter::ReadableString &pDir = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("Config"), STRING_LITERAL_AS_REFERENCE("PluginsDirectory"));
	if (pDir.isNotEmpty())
	{
		Jupiter::Plugin::setDirectory(pDir);
		printf("Plugins will be loaded from \"%.*s\"." ENDL, pDir.size(), pDir.ptr());
	}

	const Jupiter::ReadableString &cDir = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("Config"), STRING_LITERAL_AS_REFERENCE("PluginConfigsDirectory"));
	if (cDir.isNotEmpty())
	{
		Jupiter::Plugin::setDirectory(cDir);
		printf("Plugin configs will be loaded from \"%.*s\"." ENDL, cDir.size(), cDir.ptr());
	}

	puts("Loading plugins...");
	const Jupiter::ReadableString &pluginList = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("Config"), STRING_LITERAL_AS_REFERENCE("Plugins"));
	if (pluginList.isEmpty())
		puts("No plugins to load!");
	else
	{
		unsigned int nPlugins = pluginList.wordCount(WHITESPACE);
		printf("Attempting to load %u plugins..." ENDL, nPlugins);
		for (unsigned int i = 0; i < nPlugins; i++)
		{
			Jupiter::ReferenceString plugin = Jupiter::ReferenceString::getWord(pluginList, i, WHITESPACE);
			if (Jupiter::Plugin::load(plugin) == nullptr) fprintf(stderr, "WARNING: Failed to load plugin \"%.*s\"!" ENDL, plugin.size(), plugin.ptr());
			else printf("\"%.*s\" loaded successfully." ENDL, plugin.size(), plugin.ptr());
		}
	}

	if (consoleCommands->size() > 0)
		printf("%u Console Commands have been initialized%s" ENDL, consoleCommands->size(), getConsoleCommand(STRING_LITERAL_AS_REFERENCE("help")) == nullptr ? "." : "; type \"help\" for more information.");
	if (IRCMasterCommandList->size() > 0)
		printf("%u IRC Commands have been loaded into the master list." ENDL, IRCMasterCommandList->size());

	puts("Retreiving network list...");
	const Jupiter::ReadableString &serverList = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("Config"), STRING_LITERAL_AS_REFERENCE("Servers"));
	if (serverList == nullptr)
		puts("Unable to find network list.");
	else
	{
		unsigned int nServers = serverList.wordCount(WHITESPACE);
		printf("Attempting to connect to %u servers..." ENDL, nServers);
		for (unsigned int i = 0; i < nServers; i++)
			serverManager->addServer(Jupiter::ReferenceString::getWord(serverList, i, WHITESPACE));
	}

	puts("Sockets established.");
	size_t index;
	while (1)
	{
		index = 0;
		while (index < Jupiter::plugins->size())
			if (Jupiter::plugins->get(index)->shouldRemove() || Jupiter::plugins->get(index)->think() != 0)
				Jupiter::Plugin::free(index);
			else
				++index;
		Jupiter_checkTimers();
		serverManager->think();
		
		if (console_input.input_mutex.try_lock())
		{
			if (console_input.awaiting_processing)
			{
				console_input.awaiting_processing = false;
				command = console_input.input.getWord(0, WHITESPACE);

				ConsoleCommand *cmd = getConsoleCommand(command);
				if (cmd != nullptr)
					cmd->trigger(console_input.input.gotoWord(1, WHITESPACE));
				else
					printf("Error: Command \"%.*s\" not found." ENDL, command.size(), command.ptr());
			}
			console_input.input_mutex.unlock();
		}
		std::this_thread::sleep_for((std::chrono::milliseconds(1)));
	}
	return 0;
}