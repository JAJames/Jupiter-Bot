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

#include "Jupiter/IRC_Client.h"
#include "Jupiter/String.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_IRCJoin.h"

using namespace Jupiter::literals;

bool RenX_IRCJoinPlugin::initialize()
{
	RenX_IRCJoinPlugin::publicOnly = this->config.get<bool>("PublicOnly"_jrs, true);
	RenX_IRCJoinPlugin::joinMsgAlways = this->config.get<bool>("Join.MsgAlways"_jrs, false);
	RenX_IRCJoinPlugin::partMsgAlways = this->config.get<bool>("Part.MsgAlways"_jrs, false);
	RenX_IRCJoinPlugin::minAccessPartMessage = this->config.get<int>("Part.MinAccess"_jrs, 0);
	RenX_IRCJoinPlugin::maxAccessPartMessage = this->config.get<int>("Part.MaxAccess"_jrs, -1);
	RenX_IRCJoinPlugin::nameTag = this->config.get("NameTag"_jrs, "{NAME}"_jrs);
	RenX_IRCJoinPlugin::chanTag = this->config.get("ChannelTag"_jrs, "{CHAN}"_jrs);
	RenX_IRCJoinPlugin::partReasonTag = this->config.get("PartReasonTag"_jrs, "{REASON}"_jrs);
	RenX_IRCJoinPlugin::joinFmt = this->config.get("Join.Format"_jrs, "{NAME} has joined {CHAN}!"_jrs);
	RenX_IRCJoinPlugin::partFmt = this->config.get("Part.Format"_jrs, "{NAME} has left {CHAN} ({REASON})!"_jrs);
	RenX_IRCJoinPlugin::partFmtNoReason = this->config.get("Part.FormatNoReason"_jrs, "{NAME} has left {CHAN}!"_jrs);

	return true;
}

void RenX_IRCJoinPlugin::OnJoin(Jupiter::IRC::Client *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick)
{
	if (RenX_IRCJoinPlugin::joinFmt.isNotEmpty())
	{
		RenX::Server *server;
		int type = source->getChannel(channel)->getType();
		Jupiter::String msg = RenX_IRCJoinPlugin::joinFmt;
		msg.replace(RenX_IRCJoinPlugin::nameTag, nick);
		msg.replace(RenX_IRCJoinPlugin::chanTag, channel);

		auto checkType = [&]
		{
			if (this->publicOnly)
				return server->isPublicLogChanType(type);
			else
				return server->isLogChanType(type);
		};
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			server = RenX::getCore()->getServer(i);
			if (checkType() && (RenX_IRCJoinPlugin::joinMsgAlways || server->players.size() != server->getBotCount()))
				server->sendMessage(msg);
		}
	}
}

void RenX_IRCJoinPlugin::OnPart(Jupiter::IRC::Client *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &reason)
{
	if (RenX_IRCJoinPlugin::partFmt.isNotEmpty())
	{
		int access = source->getAccessLevel(channel, nick);

		if (access >= RenX_IRCJoinPlugin::minAccessPartMessage && (RenX_IRCJoinPlugin::maxAccessPartMessage == -1 || access <= RenX_IRCJoinPlugin::maxAccessPartMessage))
		{
			RenX::Server *server;
			int type = source->getChannel(channel)->getType();

			Jupiter::String msg;
			if (reason.isEmpty())
				msg = RenX_IRCJoinPlugin::partFmtNoReason;
			else
				msg = RenX_IRCJoinPlugin::partFmt;
			msg.replace(RenX_IRCJoinPlugin::nameTag, nick);
			msg.replace(RenX_IRCJoinPlugin::chanTag, channel);
			msg.replace(RenX_IRCJoinPlugin::partReasonTag, reason);

			auto checkType = [&]()
			{
				if (this->publicOnly)
					return server->isPublicLogChanType(type);
				else
					return server->isLogChanType(type);
			};
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				server = RenX::getCore()->getServer(i);
				if (checkType() && (RenX_IRCJoinPlugin::partMsgAlways || server->players.size() != server->getBotCount()))
					server->sendMessage(msg);
			}
		}
	}
}

int RenX_IRCJoinPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();

	return RenX_IRCJoinPlugin::initialize() ? 0 : -1;
}

// Plugin instantiation and entry point.
RenX_IRCJoinPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
