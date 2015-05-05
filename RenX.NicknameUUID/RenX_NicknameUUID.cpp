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
