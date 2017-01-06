/**
 * Copyright (C) 2014-2017 Jessica James.
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
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_GameCommand.h"
#include "RenX_Plugin.h"
#include "RenX_BanDatabase.h"
#include "RenX_ExemptionDatabase.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;

RenX::Core pluginInstance;
RenX::Core *RenXInstance = &pluginInstance;

RenX::Core *RenX::getCore()
{
	return &pluginInstance;
}

bool RenX::Core::initialize()
{
	RenX::banDatabase->initialize();
	RenX::exemptionDatabase->initialize();
	RenX::tags->initialize();
	RenX::initTranslations(this->config);

	const Jupiter::ReadableString &serverList = this->config.get("Servers"_jrs);
	RenX::Core::commandsFile.read(this->config.get("CommandsFile"_jrs, "RenXGameCommands.ini"_jrs));

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

	return true;
}

RenX::Core::~Core()
{
	RenX::Core::servers.emptyAndDelete();
}

size_t RenX::Core::send(int type, const Jupiter::ReadableString &msg)
{
	size_t result = 0;
	RenX::Server *server;

	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		server = RenX::Core::getServer(i);
		if (server->isLogChanType(type) && server->send(msg) > 0)
			++result;
	}

	return result;
}

void RenX::Core::addServer(RenX::Server *server)
{
	RenX::Core::servers.add(server);
}

size_t RenX::Core::getServerIndex(RenX::Server *server)
{
	for (size_t index = 0; index != RenX::Core::servers.size(); ++index)
		if (server == RenX::Core::servers.get(index))
			return index;

	return Jupiter::INVALID_INDEX;
}

RenX::Server *RenX::Core::getServer(size_t index)
{
	return RenX::Core::servers.get(index);
}

Jupiter::ArrayList<RenX::Server> RenX::Core::getServers()
{
	return RenX::Core::servers;
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

size_t RenX::Core::removeServer(RenX::Server *server)
{
	size_t index = RenX::Core::getServerIndex(server);
	
	if (index != Jupiter::INVALID_INDEX)
		delete RenX::Core::servers.remove(index);

	return index;
}

bool RenX::Core::hasServer(RenX::Server *server)
{
	size_t index = RenX::Core::servers.size();
	
	while (index != 0)
		if (server == RenX::Core::servers.get(--index))
			return true;

	return false;
}

size_t RenX::Core::getServerCount()
{
	return RenX::Core::servers.size();
}

Jupiter::ArrayList<RenX::Plugin> *RenX::Core::getPlugins()
{
	return &(RenX::Core::plugins);
}

Jupiter::Config &RenX::Core::getCommandsFile()
{
	return RenX::Core::commandsFile;
}

size_t RenX::Core::addCommand(RenX::GameCommand *command)
{
	for (size_t index = 0; index != RenX::Core::servers.size(); ++index)
		RenX::Core::servers.get(index)->addCommand(command->copy());

	return RenX::Core::servers.size();
}

void RenX::Core::banCheck()
{
	for (size_t index = 0; index != RenX::Core::servers.size(); ++index)
		RenX::Core::servers.get(index)->banCheck();
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

// Unload

extern "C" __declspec(dllexport) void unload(void)
{
	while (pluginInstance.getPlugins()->size() > 0)
		Jupiter::Plugin::free(pluginInstance.getPlugins()->remove(0));
}
