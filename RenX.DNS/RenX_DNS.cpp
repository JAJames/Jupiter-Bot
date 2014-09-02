/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
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

void RenX_DNSPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	Jupiter::ReferenceString host = Jupiter::ReferenceString(Jupiter::Socket::resolveHostname(Jupiter::CStringS(player->ip).c_str(), 0));
	const_cast<RenX::PlayerInfo *>(player)->varData.set(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("Host"), host);

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

	Jupiter::ReadableString &name = RenX::getFormattedPlayerName(player);
	(server->*func)(IRCCOLOR "03[DNS] " IRCBOLD "%.*s" IRCBOLD IRCCOLOR "'s hostname resolved to: " IRCBOLD IRCCOLOR "10%.*s" IRCBOLD, name.size(), name.ptr(), host.size(), host.ptr());
}

RenX_DNSPlugin::RenX_DNSPlugin()
{
	RenX_DNSPlugin::resolveAdmin = Jupiter::IRC::Client::Config->getBool(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("ResolveAdmin"), true);
	RenX_DNSPlugin::resolvePublic = Jupiter::IRC::Client::Config->getBool(RenX_DNSPlugin::name, STRING_LITERAL_AS_REFERENCE("ResolvePublic"), false);
}

// Plugin instantiation and entry point.
RenX_DNSPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
