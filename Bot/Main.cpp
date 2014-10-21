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

#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>
#include <thread>
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

Jupiter::Queue inputQueue;

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
	const size_t inputSize = 1024;
	char *input = new char[inputSize];
	while (ftell(stdin) != -1) // This can be expanded later to check for EBADF specifically.
	{
		fgets(input, inputSize, stdin);
		input[strcspn(input, "\r\n")] = 0;
		inputQueue.enqueue(input);
	}
	delete[] input;
}

int main(int argc, const char **args)
{
	atexit(onExit);
	std::set_terminate(onTerminate);
	std::thread inputThread(inputLoop);
	srand((unsigned int) time(0));
	puts(Jupiter::copyright);
	const char *configFileName = CONFIG_INI;

	for (int i = 1; i < argc; i++)
	{
		if (streqli(args[i], "-help"))
		{
			puts("Help coming soon, to a theatre near you!");
			return 0;
		}
		else if (streqli(args[i], "-config") && ++i < argc) configFileName = args[i];
		else if (streqli(args[i], "-pluginsdir") && ++i < argc) Jupiter::setPluginDirectory(Jupiter::ReferenceString(args[i]));
		else if (streqli(args[i], "-configFormat") && ++i < argc) puts("Feature not yet supported!");
		else printf("Warning: Unknown command line argument \"%s\" specified. Ignoring...", args[i]);
	}

	puts("Loading config file...");
	if (!Jupiter::IRC::Client::Config->readFile(configFileName))
	{
		puts("Unable to read config file. Closing...");
		exit(0);
	}

	fputs("Config loaded. ", stdout);
	const Jupiter::ReadableString &pDir = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("Config"), STRING_LITERAL_AS_REFERENCE("PluginsDirectory"));
	if (pDir.isEmpty() == false)
	{
		Jupiter::setPluginDirectory(pDir);
		printf("Plugins will be loaded from \"%.*s\"." ENDL, pDir.size(), pDir.ptr());
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
			if (Jupiter::loadPlugin(plugin) == nullptr) fprintf(stderr, "WARNING: Failed to load plugin \"%.*s\"!" ENDL, plugin.size(), plugin.ptr());
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
	while (1)
	{
		for (size_t i = 0; i < Jupiter::plugins->size(); i++)
			if (Jupiter::plugins->get(i)->shouldRemove() || Jupiter::plugins->get(i)->think() != 0)
				Jupiter::freePlugin(i);
		Jupiter_checkTimers();
		serverManager->think();
		Jupiter::ReferenceString input = (const char *)inputQueue.dequeue();
		if (input != nullptr)
		{
			Jupiter::ReferenceString command = input.getWord(0, WHITESPACE);

			ConsoleCommand *cmd = getConsoleCommand(command);
			if (cmd != nullptr)
				cmd->trigger(input.gotoWord(1, WHITESPACE));
			else
				printf("Error: Command \"%.*s\" not found." ENDL, command.size(), command.ptr());
		}
		std::this_thread::sleep_for((std::chrono::milliseconds(1)));
	}
	return 0;
}