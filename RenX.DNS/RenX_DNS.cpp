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

#include <thread>
#include "Jupiter/Socket.h"
#include "Jupiter/CString.h"
#include "Jupiter/IRC_Client.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Functions.h"
#include "RenX_DNS.h"

typedef void(RenX::Server::*logFuncType)(const char *fmt, ...) const;

void RenX_DNSPlugin::RenX_OnPlayerCreate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->isBot == false)
	{
		Jupiter::ReferenceString host = Jupiter::ReferenceString(Jupiter::Socket::resolveHostname(Jupiter::CStringS(player->ip).c_str(), 0));
		player->varData.set(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("Host"), host);
	}
}

void RenX_DNSPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->isBot == false)
	{
		logFuncType func;
		if (RenX_DNSPlugin::resolveAdmin)
		{
			if (RenX_DNSPlugin::resolvePublic)
				func = &RenX::Server::sendLogChan;
			else func = &RenX::Server::sendAdmChan;
		}
		else if (RenX_DNSPlugin::resolvePublic)
			func = &RenX::Server::sendPubChan;
		else return;

		const Jupiter::ReadableString &host = player->varData.get(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("Host"), Jupiter::ReferenceString::empty);
		Jupiter::ReadableString &name = RenX::getFormattedPlayerName(player);
		(server->*func)(IRCCOLOR "03[DNS] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR "'s hostname resolved to: " IRCBOLD IRCCOLOR "10%.*s" IRCBOLD, name.size(), name.ptr(), host.size(), host.ptr());
	}
}

int RenX_DNSPlugin::OnRehash()
{
	RenX_DNSPlugin::resolveAdmin = Jupiter::IRC::Client::Config->getBool(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("ResolveAdmin"), true);
	RenX_DNSPlugin::resolvePublic = Jupiter::IRC::Client::Config->getBool(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("ResolvePublic"), false);
	return 0;
}

RenX_DNSPlugin::RenX_DNSPlugin()
{
	RenX_DNSPlugin::OnRehash();
}

// Plugin instantiation and entry point.
RenX_DNSPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
