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

#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/String.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_IRCJoin.h"

void RenX_IRCJoinPlugin::init()
{
	RenX_IRCJoinPlugin::publicOnly = Jupiter::IRC::Client::Config->getBool(this->getName(), STRING_LITERAL_AS_REFERENCE("PublicOnly"), true);
	RenX_IRCJoinPlugin::joinMsgAlways = Jupiter::IRC::Client::Config->getBool(this->getName(), STRING_LITERAL_AS_REFERENCE("Join.MsgAlways"), false);
	RenX_IRCJoinPlugin::partMsgAlways = Jupiter::IRC::Client::Config->getBool(this->getName(), STRING_LITERAL_AS_REFERENCE("Part.MsgAlways"), false);
	RenX_IRCJoinPlugin::minAccessJoinMessage = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Join.MinAccess"), 0);
	RenX_IRCJoinPlugin::maxAccessJoinMessage = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Join.MaxAccess"), -1);
	RenX_IRCJoinPlugin::minAccessPartMessage = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Part.MinAccess"), 0);
	RenX_IRCJoinPlugin::maxAccessPartMessage = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Part.MaxAccess"), -1);
	RenX_IRCJoinPlugin::nameTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NameTag"), STRING_LITERAL_AS_REFERENCE("{NAME}"));
	RenX_IRCJoinPlugin::chanTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ChannelTag"), STRING_LITERAL_AS_REFERENCE("{CHAN}"));
	RenX_IRCJoinPlugin::partReasonTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("PartReasonTag"), STRING_LITERAL_AS_REFERENCE("{REASON}"));
	RenX_IRCJoinPlugin::joinFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("Join.Format"), STRING_LITERAL_AS_REFERENCE("{NAME} has joined {CHAN}!"));
	RenX_IRCJoinPlugin::partFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("Part.Format"), STRING_LITERAL_AS_REFERENCE("{NAME} has left {CHAN} ({REASON})!"));
	RenX_IRCJoinPlugin::partFmtNoReason = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("Part.FormatNoReason"), STRING_LITERAL_AS_REFERENCE("{NAME} has left {CHAN}!"));
}

void RenX_IRCJoinPlugin::OnJoin(Jupiter::IRC::Client *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick)
{
	int access = source->getAccessLevel(channel, nick);

	if (access >= RenX_IRCJoinPlugin::minAccessJoinMessage && (RenX_IRCJoinPlugin::maxAccessJoinMessage == -1 || access <= RenX_IRCJoinPlugin::maxAccessJoinMessage))
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
			if (checkType() && (RenX_IRCJoinPlugin::joinMsgAlways || server->players.size() != 0))
				server->sendMessage(msg);
		}
	}
}

void RenX_IRCJoinPlugin::OnPart(Jupiter::IRC::Client *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &reason)
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
			if (checkType() && (RenX_IRCJoinPlugin::partMsgAlways || server->players.size() != 0))
				server->sendMessage(msg);
		}
	}
}

int RenX_IRCJoinPlugin::OnRehash()
{
	RenX_IRCJoinPlugin::init();
	return 0;
}

RenX_IRCJoinPlugin::RenX_IRCJoinPlugin()
{
	RenX_IRCJoinPlugin::init();
}

// Plugin instantiation and entry point.
RenX_IRCJoinPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
