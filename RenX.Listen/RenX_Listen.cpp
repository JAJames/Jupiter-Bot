/**
 * Copyright (C) 2015 Jessica James.
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
#include "Jupiter/INIFile.h"
#include "Jupiter/CString.h"
#include "RenX_Listen.h"
#include "RenX_Core.h"
#include "RenX_Server.h"

RenX_ListenPlugin::~RenX_ListenPlugin()
{
	RenX_ListenPlugin::socket.close();
}

bool RenX_ListenPlugin::init()
{
	uint16_t port = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Port"), 21337);
	const Jupiter::ReadableString &address = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("Address"), STRING_LITERAL_AS_REFERENCE("0.0.0.0"));
	RenX_ListenPlugin::serverSection = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ServerSection"), this->getName());
	return RenX_ListenPlugin::socket.bind(Jupiter::CStringS(address).c_str(), port, true) && RenX_ListenPlugin::socket.setBlocking(false);
}

int RenX_ListenPlugin::think()
{
	Jupiter::Socket *sock = socket.accept();
	if (sock != nullptr)
	{
		sock->setBlocking(false);
		RenX::Server *server = new RenX::Server(std::move(*sock), RenX_ListenPlugin::serverSection);
		printf("Incoming server connected from %.*s:%u" ENDL, server->getSocketHostname().size(), server->getSocketHostname().ptr(), server->getSocketPort());
		server->sendLogChan("Incoming server connected from " IRCCOLOR "12%.*s:%u", server->getSocketHostname().size(), server->getSocketHostname().ptr(), server->getSocketPort());
		RenX::getCore()->addServer(server);
		delete sock;
	}
	return 0;
}

int RenX_ListenPlugin::OnRehash()
{
	uint16_t port = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Port"), 21337);
	const Jupiter::ReadableString &address = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("Address"), STRING_LITERAL_AS_REFERENCE("0.0.0.0"));
	RenX_ListenPlugin::serverSection = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ServerSection"), this->getName());
	if (port != RenX_ListenPlugin::socket.getRemotePort() || address.equals(RenX_ListenPlugin::socket.getRemoteHostname()) == false)
	{
		puts("Notice: The Renegade-X listening socket has been changed!");
		RenX_ListenPlugin::socket.close();
		return RenX_ListenPlugin::socket.bind(Jupiter::CStringS(address).c_str(), port, true) == false || RenX_ListenPlugin::socket.setBlocking(false) == false;
	}
	return 0;
}

// Plugin instantiation and entry point.
RenX_ListenPlugin pluginInstance;

extern "C" __declspec(dllexport) bool load()
{
	return pluginInstance.init();
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
