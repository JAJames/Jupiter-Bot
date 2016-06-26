/**
 * Copyright (C) 2014-2016 Jessica James.
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

#include "Jupiter/Functions.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "IRC_Command.h"

using namespace Jupiter::literals;

ServerManager _serverManager;
ServerManager *serverManager = &_serverManager;

int ServerManager::think()
{
	for (size_t i = 0; i < ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		if (server->think() != 0) delete ServerManager::servers.remove(i);
	}
	return ServerManager::servers.size() != 0;
}

size_t ServerManager::addCommand(IRCCommand *command)
{
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		server->addCommand(command->copy());
	}
	return ServerManager::servers.size();
}

size_t ServerManager::removeCommand(IRCCommand *command)
{
	size_t r = 0;
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		if (server->freeCommand(command->getTrigger())) r++;
	}
	return r;
}

size_t ServerManager::removeCommand(const Jupiter::ReadableString &command)
{
	size_t r = 0;
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		if (server->freeCommand(command)) r++;
	}
	return r;
}

size_t ServerManager::syncCommands()
{
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		server->setCommandAccessLevels();
	}
	return ServerManager::servers.size();
}

IRC_Bot *ServerManager::getServer(const Jupiter::ReadableString &serverConfig)
{
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		if (server->getConfigSection().equalsi(serverConfig)) return server;
	}
	return nullptr;
}

IRC_Bot *ServerManager::getServer(size_t serverIndex)
{
	if (serverIndex < ServerManager::servers.size()) return ServerManager::servers.get(serverIndex);
	return nullptr;
}

bool ServerManager::addServer(const Jupiter::ReadableString &serverConfig)
{
	IRC_Bot *server = new IRC_Bot(g_config->getSection(serverConfig), g_config->getSection("Default"_jrs));
	if (server->connect())
	{
		ServerManager::servers.add(server);
		return true;
	}
	else
	{
		delete server;
		return false;
	}
}

bool ServerManager::freeServer(size_t serverIndex)
{
	if (serverIndex < ServerManager::servers.size())
	{
		delete ServerManager::servers.remove(serverIndex);
		return true;
	}
	return false;
}

bool ServerManager::freeServer(IRC_Bot *server)
{
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		if (ServerManager::servers.get(i) == server)
		{
			delete ServerManager::servers.remove(i);
			return true;
		}
	}
	return false;
}

bool ServerManager::freeServer(const Jupiter::ReadableString &serverConfig)
{
	for (size_t i = 0; i != ServerManager::servers.size(); i++)
	{
		IRC_Bot *server = ServerManager::servers.get(i);
		if (server->getConfigSection().equalsi(serverConfig))
		{
			delete ServerManager::servers.remove(i);
			return true;
		}
	}
	return false;
}

size_t ServerManager::size()
{
	return ServerManager::servers.size();
}

ServerManager::~ServerManager()
{
	ServerManager::servers.emptyAndDelete();
}