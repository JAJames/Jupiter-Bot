/**
 * Copyright (C) 2015-2017 Jessica James.
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

#include "Jupiter/IRC_Client.h"
#include "Jupiter/String.hpp"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "ChannelRelay.h"

using namespace Jupiter::literals;

bool ChannelRelayPlugin::initialize()
{
	Jupiter::ReferenceString str = this->config.get("Types"_jrs);
	unsigned int words = str.wordCount(WHITESPACE);
	if (words == 0)
		return false;

	while (words != 0)
		ChannelRelayPlugin::types.concat(str.getWord(--words, WHITESPACE).asInt());

	return true;
}

int ChannelRelayPlugin::OnRehash()
{
	Jupiter::Plugin::OnRehash();

	ChannelRelayPlugin::types.erase();
	return this->initialize() ? 0 : -1;
}

void ChannelRelayPlugin::OnChat(Jupiter::IRC::Client *server, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message)
{
	Jupiter::IRC::Client::Channel *chan = server->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		if (ChannelRelayPlugin::types.contains(type))
		{
			unsigned int serverCount = serverManager->size();
			char prefix = chan->getUserPrefix(nick);
			Jupiter::String str;
			if (prefix == 0) {
				str = "<"_jrs + nick + "> "_jrs + message;
			}
			else {
				str = "<"_js + prefix + nick + "> "_jrs + message;
			}

			while (serverCount != 0) {
				auto server = serverManager->getServer(--serverCount);
				for (auto& channel : server->getChannels()) {
					if (channel.second.getType() == type && &channel.second != chan) {
						server->sendMessage(channel.second.getName(), str);
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
