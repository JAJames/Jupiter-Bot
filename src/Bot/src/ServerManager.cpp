/**
 * Copyright (C) 2014-2021 Jessica James.
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

ServerManager g_serverManager;
ServerManager *serverManager = &g_serverManager;

int ServerManager::think() {
	for (auto itr = m_servers.begin(); itr != m_servers.end();) {
		if ((*itr)->think() != 0) {
			itr = m_servers.erase(itr);
			continue;
		}
		++itr;
	}

	return m_servers.size() != 0;
}

size_t ServerManager::addCommand(IRCCommand *command) {
	for (const auto& server : m_servers) {
		server->addCommand(command->copy());
	}

	return m_servers.size();
}

size_t ServerManager::removeCommand(IRCCommand *command) {
	size_t result = 0;
	for (const auto& server : m_servers) {
		if (server->freeCommand(command->getTrigger())) {
			++result;
		}
	}

	return result;
}

size_t ServerManager::removeCommand(const Jupiter::ReadableString &command) {
	size_t result = 0;
	for (const auto& server : m_servers) {
		if (server->freeCommand(command)) {
			++result;
		}
	}

	return result;
}

void ServerManager::OnConfigRehash() {
	for (const auto& server : m_servers) {
		server->setPrimaryConfigSection(m_config->getSection(server->getConfigSection()));
		server->setSecondaryConfigSection(m_config->getSection("Defualt"_jrs));
		server->setCommandAccessLevels();
	}
}

size_t ServerManager::syncCommands() {
	for (const auto& server : m_servers) {
		server->setCommandAccessLevels();
	}

	return m_servers.size();
}

IRC_Bot *ServerManager::getServer(const Jupiter::ReadableString &serverConfig) {
	for (const auto& server : m_servers) {
		if (server->getConfigSection().equalsi(serverConfig)) {
			return server.get();
		}
	}

	return nullptr;
}

IRC_Bot *ServerManager::getServer(size_t serverIndex) {
	if (serverIndex < m_servers.size()) {
		return m_servers[serverIndex].get();
	}

	return nullptr;
}

bool ServerManager::addServer(std::string_view serverConfig) {
	auto server = std::make_unique<IRC_Bot>(m_config->getSection(serverConfig), m_config->getSection("Default"_jrs));
	if (server->connect()) {
		m_servers.push_back(std::move(server));
		return true;
	}

	return false;
}

bool ServerManager::freeServer(size_t serverIndex) {
	if (serverIndex < m_servers.size()) {
		m_servers.erase(m_servers.begin() + serverIndex);
		return true;
	}

	return false;
}

bool ServerManager::freeServer(IRC_Bot *server) {
	for (auto itr = m_servers.begin(); itr != m_servers.end(); ++itr) {
		if (itr->get() == server) {
			m_servers.erase(itr);
			return true;
		}
	}

	return false;
}

bool ServerManager::freeServer(const Jupiter::ReadableString &serverConfig) {
	for (auto itr = m_servers.begin(); itr != m_servers.end(); ++itr) {
		if ((*itr)->getConfigSection().equalsi(serverConfig)) {
			m_servers.erase(itr);
			return true;
		}
	}

	return false;
}

size_t ServerManager::size() {
	return m_servers.size();
}

ServerManager::~ServerManager() {
}