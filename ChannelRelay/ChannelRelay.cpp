/**
 * Copyright (C) 2015 Justin James.
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

#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/String.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "ChannelRelay.h"

using namespace Jupiter::literals;

int ChannelRelayPlugin::init()
{
	Jupiter::ReferenceString str = Jupiter::IRC::Client::Config->get(this->getName(), "Types"_jrs);
	unsigned int words = str.wordCount(WHITESPACE);
	if (words == 0)
		return 1;

	while (words != 0)
		ChannelRelayPlugin::types.concat(str.getWord(--words, WHITESPACE).asInt());

	return 0;
}

int ChannelRelayPlugin::OnRehash()
{
	ChannelRelayPlugin::types.erase();
	return ChannelRelayPlugin::init();
}

void ChannelRelayPlugin::OnChat(Jupiter::IRC::Client *server, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message)
{
	Jupiter::IRC::Client::Channel *chan = server->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		if (ChannelRelayPlugin::types.contains(type))
		{
			Jupiter::IRC::Client::Channel *tchan;
			Jupiter::IRC::Client *tserver;
			unsigned int count = server->getChannelCount();
			unsigned int serverCount = serverManager->size();
			char prefix = chan->getUserPrefix(nick);
			auto str = prefix == 0 ? "<"_jrs + nick + "> "_jrs + message : "<"_js + prefix + nick + "> "_jrs + message;
			while (count != 0)
			{
				tchan = server->getChannel(--count);
				if (tchan->getType() == type && chan != tchan)
					server->sendMessage(tchan->getName(), str);
			}

			while (serverCount != 0)
			{
				tserver = serverManager->getServer(--serverCount);
				if (tserver != server)
				{
					count = tserver->getChannelCount();
					while (count != 0)
					{
						tchan = tserver->getChannel(--count);
						if (tchan->getType() == type)
							tserver->sendMessage(tchan->getName(), str);
					}
				}
			}
		}
	}
}

// Plugin instantiation and entry point.
ChannelRelayPlugin pluginInstance;

extern "C" __declspec(dllexport) bool load()
{
	return pluginInstance.init() == 0;
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
