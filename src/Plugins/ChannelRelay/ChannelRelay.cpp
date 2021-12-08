/**
 * Copyright (C) 2015-2021 Jessica James.
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
#include "Jupiter/IRC_Client.h"
#include "Jupiter/Functions.h"
#include "Jupiter/Readable_String.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "ChannelRelay.h"

using namespace std::literals;

bool ChannelRelayPlugin::initialize() {
	std::string_view types_string = this->config.get("Types"sv);
	std::vector<std::string_view> split_types = jessilib::word_split_view(types_string, WHITESPACE_SV);

	if (split_types.empty()) {
		return false;
	}

	for (const auto& type : split_types) {
		m_types.push_back(Jupiter::asInt(type));
	}

	return true;
}

int ChannelRelayPlugin::OnRehash() {
	Jupiter::Plugin::OnRehash();

	m_types.clear();
	return this->initialize() ? 0 : -1;
}

void ChannelRelayPlugin::OnChat(Jupiter::IRC::Client *server, std::string_view channel, std::string_view nick, std::string_view message) {
	Jupiter::IRC::Client::Channel *chan = server->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		if (std::find(m_types.begin(), m_types.end(), type) != m_types.end()) {
			size_t serverCount = serverManager->size();
			char prefix = chan->getUserPrefix(nick);
			std::string user_string;
			user_string = "<"sv;
			if (prefix != 0) {
				user_string += prefix;
			}
			user_string += nick;
			user_string += "> "sv;
			user_string += message;

			while (serverCount != 0) {
				auto server = serverManager->getServer(--serverCount);
				for (auto& channel : server->getChannels()) {
					if (channel.second.getType() == type && &channel.second != chan) {
						server->sendMessage(channel.second.getName(), user_string);
					}
				}
			}
		}
	}
}

// Plugin instantiation and entry point.
ChannelRelayPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
