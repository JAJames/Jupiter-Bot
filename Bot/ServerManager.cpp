/**
 * Copyright (C) 2014 Justin James.
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

#include "Jupiter/Functions.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "IRC_Command.h"

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
	IRC_Bot *server = new IRC_Bot(serverConfig);
	if (server->connect() == true)
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