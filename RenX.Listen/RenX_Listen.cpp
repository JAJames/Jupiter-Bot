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
#include "Jupiter/CString.h"
#include "RenX_Listen.h"
#include "RenX_Core.h"
#include "RenX_Server.h"

RenX_ListenPlugin::~RenX_ListenPlugin()
{
	RenX_ListenPlugin::socket.closeSocket();
}

bool RenX_ListenPlugin::init()
{
	uint16_t port = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Port"), 13372);
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
		printf("Incoming server connected from " IRCCOLOR "12%.*s:%u", server->getSocketHostname().size(), server->getSocketHostname().ptr(), server->getSocketPort());
		server->sendLogChan("Incoming server connected from " IRCCOLOR "12%.*s:%u", server->getSocketHostname().size(), server->getSocketHostname().ptr(), server->getSocketPort());
		RenX::getCore()->addServer(server);
		delete sock;
	}
	return 0;
}

int RenX_ListenPlugin::OnRehash()
{
	uint16_t port = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("Port"), 13372);
	const Jupiter::ReadableString &address = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("Address"), STRING_LITERAL_AS_REFERENCE("0.0.0.0"));
	RenX_ListenPlugin::serverSection = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ServerSection"), this->getName());
	if (port != RenX_ListenPlugin::socket.getPort() || address.equals(RenX_ListenPlugin::socket.getHostname()) == false)
	{
		puts("Notice: The Renegade-X listening socket has been changed!");
		RenX_ListenPlugin::socket.closeSocket();
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
