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

#include "Jupiter/String.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_NicknameUUID.h"

Jupiter::StringS calc_uuid(RenX::PlayerInfo *player)
{
	return player->name;
}

RenX_NicknameUUIDPlugin::RenX_NicknameUUIDPlugin()
{
	RenX::Core &core = *RenX::getCore();
	size_t index = core.getServerCount();
	while (index != 0)
		core.getServer(--index)->setUUIDFunction(calc_uuid);
}

RenX_NicknameUUIDPlugin::~RenX_NicknameUUIDPlugin()
{
	RenX::Core &core = *RenX::getCore();
	size_t index = core.getServerCount();
	while (index != 0)
		core.getServer(--index)->setUUIDFunction(RenX::default_uuid_func);
}

void RenX_NicknameUUIDPlugin::RenX_OnServerCreate(RenX::Server *server)
{
	server->setUUIDFunction(calc_uuid);
}

// Plugin instantiation and entry point.
RenX_NicknameUUIDPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
