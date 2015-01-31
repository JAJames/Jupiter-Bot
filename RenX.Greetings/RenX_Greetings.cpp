/**
 * Copyright (C) 2014-2015 Justin James.
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
#include "RenX_Greetings.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Tags.h"

void RenX_GreetingsPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	auto sendMessage = [&](const Jupiter::ReadableString &m)
	{
		Jupiter::String msg = m;

		RenX::sanitizeTags(msg);
		RenX::processTags(msg, server, player);

		if (this->sendPrivate)
			server->sendMessage(player, msg);
		else
			server->sendMessage(msg);
	};
	if (player->isBot == false)
	{
		switch (RenX_GreetingsPlugin::sendMode)
		{
		case 0:
			RenX_GreetingsPlugin::lastLine = rand() % RenX_GreetingsPlugin::greetingsFile.getLineCount();
			sendMessage(RenX_GreetingsPlugin::greetingsFile.getLine(RenX_GreetingsPlugin::lastLine));
			break;
		case 1:
			if (++RenX_GreetingsPlugin::lastLine == RenX_GreetingsPlugin::greetingsFile.getLineCount())
				RenX_GreetingsPlugin::lastLine = 0;
			sendMessage(RenX_GreetingsPlugin::greetingsFile.getLine(RenX_GreetingsPlugin::lastLine));
			break;
		case 2:
			for (RenX_GreetingsPlugin::lastLine = 0; RenX_GreetingsPlugin::lastLine != RenX_GreetingsPlugin::greetingsFile.getLineCount(); RenX_GreetingsPlugin::lastLine++)
				sendMessage(RenX_GreetingsPlugin::greetingsFile.getLine(RenX_GreetingsPlugin::lastLine));
			break;
		default:
			return;
		}
	}
}

int RenX_GreetingsPlugin::OnRehash()
{
	RenX_GreetingsPlugin::greetingsFile.unload();
	RenX_GreetingsPlugin::init();
	return 0;
}

RenX_GreetingsPlugin::RenX_GreetingsPlugin()
{
	RenX_GreetingsPlugin::init();
}

void RenX_GreetingsPlugin::init()
{
	RenX_GreetingsPlugin::sendPrivate = Jupiter::IRC::Client::Config->getBool(RenX_GreetingsPlugin::name, STRING_LITERAL_AS_REFERENCE("SendPrivate"), true);
	RenX_GreetingsPlugin::sendMode = Jupiter::IRC::Client::Config->getInt(RenX_GreetingsPlugin::name, STRING_LITERAL_AS_REFERENCE("SendMode"), 0);
	RenX_GreetingsPlugin::greetingsFile.load(Jupiter::IRC::Client::Config->get(RenX_GreetingsPlugin::name, STRING_LITERAL_AS_REFERENCE("GreetingsFile"), STRING_LITERAL_AS_REFERENCE("RenX.Greetings.txt")));
	if (RenX_GreetingsPlugin::greetingsFile.getLineCount() == 0)
		RenX_GreetingsPlugin::greetingsFile.addData(STRING_LITERAL_AS_REFERENCE("Please notify the server administrator to properly configure or disable server greetings.\r\n"));
	RenX_GreetingsPlugin::lastLine = RenX_GreetingsPlugin::greetingsFile.getLineCount() - 1;
}

// Plugin instantiation and entry point.
RenX_GreetingsPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
