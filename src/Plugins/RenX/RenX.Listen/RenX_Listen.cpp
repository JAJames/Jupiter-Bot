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

#include "Jupiter/IRC_Client.h"
#include "RenX_Listen.h"
#include "RenX_Core.h"
#include "RenX_Server.h"

using namespace Jupiter::literals;

RenX_ListenPlugin::~RenX_ListenPlugin() {
	RenX_ListenPlugin::socket.close();
}

bool RenX_ListenPlugin::initialize() {
	uint16_t port = this->config.get<uint16_t>("Port"_jrs, 21337);
	std::string_view address = this->config.get("Address"_jrs, "0.0.0.0"_jrs);
	RenX_ListenPlugin::serverSection = this->config.get("ServerSection"_jrs, this->getName());

	return RenX_ListenPlugin::socket.bind(static_cast<std::string>(address).c_str(), port, true) && RenX_ListenPlugin::socket.setBlocking(false);
}

int RenX_ListenPlugin::think() {
	Jupiter::Socket *sock = socket.accept();
	if (sock != nullptr) {
		sock->setBlocking(false);
		std::unique_ptr<RenX::Server> server = std::make_unique<RenX::Server>(std::move(*sock), RenX_ListenPlugin::serverSection);
		printf("Incoming server connected from %s:%u" ENDL, server->getSocketHostname().c_str(), server->getSocketPort());
		server->sendLogChan("Incoming server connected from " IRCCOLOR "12%s:%u", server->getSocketHostname().c_str(), server->getSocketPort());
		RenX::getCore()->addServer(std::move(server));
		delete sock;
	}
	return 0;
}

int RenX_ListenPlugin::OnRehash() {
	RenX::Plugin::OnRehash();

	uint16_t port = this->config.get<uint16_t>("Port"_jrs, 21337);
	std::string_view address = this->config.get("Address"_jrs, "0.0.0.0"_jrs);
	RenX_ListenPlugin::serverSection = this->config.get("ServerSection"_jrs, this->getName());

	if (port != RenX_ListenPlugin::socket.getBoundPort() || address != RenX_ListenPlugin::socket.getBoundHostname()) {
		puts("Notice: The Renegade-X listening socket has been changed!");
		RenX_ListenPlugin::socket.close();
		return RenX_ListenPlugin::socket.bind(static_cast<std::string>(address).c_str(), port, true) == false || RenX_ListenPlugin::socket.setBlocking(false) == false;
	}
	return 0;
}

// Plugin instantiation and entry point.
RenX_ListenPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
