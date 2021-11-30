/**
 * Copyright (C) 2016-2021 Jessica James.
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

#include "jessilib/word_split.hpp"
#include "Jupiter/GenericCommand.h"
#include "ServerManager.h"
#include "IRC_Command.h"
#include "IRC_Core.h"

using namespace Jupiter::literals;

IRCCorePlugin::~IRCCorePlugin() {
	// Destroy all IRC connections on plugin unload
	while (serverManager->size()) {
		serverManager->freeServer(size_t{0});
	}
}

bool IRCCorePlugin::initialize() {
	// TODO: initialize() isn't bringing in generic commands from already-loaded plugins
	std::string_view serverList = this->config.get("Servers"_jrs);
	if (!serverList.empty()) {
		serverManager->setConfig(this->config);

		auto server_entries = jessilib::word_split_view(serverList, WHITESPACE_SV);
		for (const auto& entry : server_entries) {
			serverManager->addServer(entry);
		}
	}

	return true;
}

int IRCCorePlugin::OnRehash() {
	Jupiter::Plugin::OnRehash();

	serverManager->OnConfigRehash();
	return 0;
}

int IRCCorePlugin::think() {
	serverManager->think();
	return 0;
}

void IRCCorePlugin::OnGenericCommandAdd(Jupiter::GenericCommand &in_command) {
	m_wrapped_commands.emplace_back(new GenericCommandWrapperIRCCommand{in_command});
}

void IRCCorePlugin::OnGenericCommandRemove(Jupiter::GenericCommand &in_command) {
	for (auto itr = m_wrapped_commands.begin(); itr != m_wrapped_commands.end(); ++itr) {
		if (&(*itr)->getGenericCommand() == &in_command) {
			m_wrapped_commands.erase(itr);
			return;
		}
	}
}

// Plugin instantiation and entry point.
IRCCorePlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
