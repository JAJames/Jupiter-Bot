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

#include <ctime>
#include "jessilib/word_split.hpp"
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

RenX::Core *RenX::getCore() {
	return &pluginInstance;
}

bool RenX::Core::initialize() {
	RenX::banDatabase->initialize();
	RenX::exemptionDatabase->initialize();
	RenX::tags->initialize();
	RenX::initTranslations(this->config);

	std::string_view serverList = this->config.get("Servers"_jrs);
	m_commandsFile.read(this->config.get("CommandsFile"_jrs, "RenXGameCommands.ini"_jrs));

	auto server_entries = jessilib::word_split_view(serverList, WHITESPACE_SV);
	for (const auto& entry : server_entries) {
		auto server = std::make_unique<RenX::Server>(entry);

		if (server->connect() == false) {
			fprintf(stderr, "[RenX] ERROR: Failed to connect to %s on port %u. Error code: %d" ENDL, server->getHostname().c_str(), server->getPort(), Jupiter::Socket::getLastError());
			continue;
		}

		addServer(std::move(server));
	}

	return true;
}

RenX::Core::~Core() {
}

size_t RenX::Core::send(int type, std::string_view msg) {
	size_t result = 0;

	for (auto& server : m_servers) {
		if (server->isLogChanType(type) && server->send(msg) > 0) {
			++result;
		}
	}

	return result;
}

void RenX::Core::addServer(std::unique_ptr<RenX::Server> server) {
	m_servers.push_back(std::move(server));
}

size_t RenX::Core::getServerIndex(RenX::Server *server) {
	for (size_t index = 0; index != m_servers.size(); ++index) {
		if (server == m_servers[index].get()) {
			return index;
		}
	}

	return Jupiter::INVALID_INDEX;
}

RenX::Server* RenX::Core::getServer(size_t index) {
	if (index > m_servers.size()) {
		return nullptr;
	}

	return m_servers[index].get();
}

std::vector<RenX::Server*> RenX::Core::getServers() {
	std::vector<RenX::Server*> result;

	for (const auto& server : m_servers) {
		result.push_back(server.get());
	}

	return result;
}

std::vector<RenX::Server*> RenX::Core::getServers(int type) {
	std::vector<RenX::Server*> result;

	for (const auto& server : m_servers) {
		if (server->isLogChanType(type)) {
			result.push_back(server.get());
		}
	}

	return result;
}

void RenX::Core::removeServer(unsigned int index) {
	if (index < m_servers.size()) {
		m_servers.erase(m_servers.begin() + index);
	}
}

size_t RenX::Core::removeServer(RenX::Server *server) {
	for (auto itr = m_servers.begin(); itr != m_servers.end(); ++itr) {
		if (itr->get() == server) {
			size_t index = m_servers.end() - itr;
			m_servers.erase(itr);
			return index;
		}
	}

	return Jupiter::INVALID_INDEX;
}

bool RenX::Core::hasServer(RenX::Server* in_server) {
	for (const auto& server : m_servers) {
		if (server.get() == in_server) {
			return true;
		}
	}

	return false;
}

size_t RenX::Core::getServerCount() {
	return m_servers.size();
}

std::vector<RenX::Plugin*>& RenX::Core::getPlugins() {
	return m_plugins;
}

Jupiter::Config &RenX::Core::getCommandsFile() {
	return m_commandsFile;
}

size_t RenX::Core::addCommand(RenX::GameCommand *command) {
	for (const auto& server : m_servers) {
		server->addCommand(command->copy());
	}

	return m_servers.size();
}

void RenX::Core::banCheck() {
	for (const auto& server : m_servers) {
		server->banCheck();
	}
}

int RenX::Core::think() {
	for (auto itr = m_servers.begin(); itr != m_servers.end();) {
		if ((*itr)->think() != 0) {
			itr = m_servers.erase(itr);
			continue;
		}
		++itr;
	}

	return Jupiter::Plugin::think();
}

// Entry point

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}

// Unload

extern "C" JUPITER_EXPORT void unload(void) {
	auto& plugins = pluginInstance.getPlugins();
	while (!plugins.empty()) {
		Jupiter::Plugin::free(plugins.back());
		plugins.pop_back();
	}
}
