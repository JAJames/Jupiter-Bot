/**
 * Copyright (C) 2014-2015 Jessica James.
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

#include <ctime>
#include "Jupiter/INIFile.h"
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_GameCommand.h"
#include "RenX_Plugin.h"

RenX::Core pluginInstance;
RenX::Core *RenXInstance = &pluginInstance;

RenX::Core *RenX::getCore()
{
	return &pluginInstance;
}

void RenX::Core::init()
{
	const Jupiter::ReadableString &serverList = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("Servers"));
	RenX::Core::translationsFile.readFile(Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("TranslationsFile"), STRING_LITERAL_AS_REFERENCE("Translations.ini")));
	RenX::initTranslations(RenX::Core::translationsFile);
	RenX::Core::commandsFile.readFile(Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("CommandsFile"), STRING_LITERAL_AS_REFERENCE("RenXGameCommands.ini")));

	unsigned int wc = serverList.wordCount(WHITESPACE);

	RenX::Server *server;
	for (unsigned int i = 0; i != wc; i++)
	{
		server = new RenX::Server(Jupiter::ReferenceString::getWord(serverList, i, WHITESPACE));

		if (server->connect() == false)
		{
			fprintf(stderr, "[RenX] ERROR: Failed to connect to %.*s on port %u. Error code: %d" ENDL, server->getHostname().size(), server->getHostname().ptr(), server->getPort(), Jupiter::Socket::getLastError());
			delete server;
		}
		else RenX::Core::addServer(server);
	}
}

RenX::Core::~Core()
{
	RenX::Core::servers.emptyAndDelete();
}

unsigned int RenX::Core::send(int type, const Jupiter::ReadableString &msg)
{
	unsigned int r = 0;
	RenX::Server *server;
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		server = RenX::Core::getServer(i);
		if (server->isLogChanType(type) && server->send(msg) > 0) r++;
	}
	return r;
}

void RenX::Core::addServer(RenX::Server *server)
{
	RenX::Core::servers.add(server);
}

int RenX::Core::getServerIndex(RenX::Server *server)
{
	size_t i = RenX::Core::servers.size();
	while (i != 0)
		if (server == RenX::Core::servers.get(--i))
			return i;
	return -1;
}

RenX::Server *RenX::Core::getServer(unsigned int index)
{
	return RenX::Core::servers.get(index);
}

Jupiter::ArrayList<RenX::Server> RenX::Core::getServers(int type)
{
	Jupiter::ArrayList<RenX::Server> r;
	RenX::Server *server;
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		server = RenX::Core::servers.get(i);
		if (server != nullptr && server->isLogChanType(type))
			r.add(server);
	}
	return r;
}

void RenX::Core::removeServer(unsigned int index)
{
	delete RenX::Core::servers.remove(index);
}

int RenX::Core::removeServer(RenX::Server *server)
{
	int i = RenX::Core::getServerIndex(server);
	if (i >= 0) delete RenX::Core::servers.remove(i);
	return i;
}

bool RenX::Core::hasServer(RenX::Server *server)
{
	size_t i = RenX::Core::servers.size();
	while (i != 0)
		if (server == RenX::Core::servers.get(--i))
			return true;
	return false;
}

unsigned int RenX::Core::getServerCount()
{
	return RenX::Core::servers.size();
}

Jupiter::ArrayList<RenX::Plugin> *RenX::Core::getPlugins()
{
	return &(RenX::Core::plugins);
}

Jupiter::INIFile &RenX::Core::getTranslationsFile()
{
	return RenX::Core::translationsFile;
}

Jupiter::INIFile &RenX::Core::getCommandsFile()
{
	return RenX::Core::commandsFile;
}

int RenX::Core::addCommand(RenX::GameCommand *command)
{
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		RenX::Server *server = RenX::Core::servers.get(i);
		server->addCommand(command->copy());
	}
	return RenX::Core::servers.size();
}

int RenX::Core::think()
{
	size_t index = 0;
	while (index < RenX::Core::servers.size())
		if (RenX::Core::servers.get(index)->think() != 0)
			delete RenX::Core::servers.remove(index);
		else ++index;

	return Jupiter::Plugin::think();
}

// Entry point

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}

// Load

extern "C" __declspec(dllexport) bool load(void)
{
	pluginInstance.init();
	return true;
}

// Unload

extern "C" __declspec(dllexport) void unload(void)
{
	while (pluginInstance.getPlugins()->size() > 0) freePlugin(pluginInstance.getPlugins()->remove(0));
}
