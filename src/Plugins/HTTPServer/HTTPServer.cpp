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

#include "HTTPServer.h"

using namespace Jupiter::literals;

bool HTTPServerPlugin::initialize()
{
	return HTTPServerPlugin::server.bind(this->config.get("BindAddress"_jrs, "0.0.0.0"_jrs), this->config.get<uint16_t>("BindPort"_jrs, 80));
}

int HTTPServerPlugin::think()
{
	return HTTPServerPlugin::server.think();
}

// Plugin instantiation and entry point.
HTTPServerPlugin pluginInstance;

HTTPServerPlugin &getHTTPServerPlugin()
{
	return pluginInstance;
}

Jupiter::HTTP::Server &getHTTPServer()
{
	return pluginInstance.server;
}

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
