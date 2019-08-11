/**
 * Copyright (C) 2014-2017 Jessica James.
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
#include "RenX_Greetings.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;

void RenX_GreetingsPlugin::RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player)
{
	auto sendMessage = [&](const Jupiter::ReadableString &m)
	{
		Jupiter::String msg = m;

		RenX::sanitizeTags(msg);
		RenX::processTags(msg, &server, &player);

		if (this->sendPrivate)
			server.sendMessage(player, msg);
		else
			server.sendMessage(msg);
	};
	if (player.isBot == false && server.isMatchInProgress())
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
	RenX::Plugin::OnRehash();

	RenX_GreetingsPlugin::greetingsFile.unload();
	return RenX_GreetingsPlugin::initialize() ? 0 : -1;
}

bool RenX_GreetingsPlugin::initialize()
{
	RenX_GreetingsPlugin::sendPrivate = this->config.get<bool>("SendPrivate"_jrs, true);
	RenX_GreetingsPlugin::sendMode = this->config.get<unsigned int>("SendMode"_jrs, 0);
	RenX_GreetingsPlugin::greetingsFile.load(this->config.get("GreetingsFile"_jrs, "RenX.Greetings.txt"_jrs));
	if (RenX_GreetingsPlugin::greetingsFile.getLineCount() == 0)
		RenX_GreetingsPlugin::greetingsFile.addData("Please notify the server administrator to properly configure or disable server greetings.\r\n"_jrs);
	RenX_GreetingsPlugin::lastLine = RenX_GreetingsPlugin::greetingsFile.getLineCount() - 1;

	return true;
}

// Plugin instantiation and entry point.
RenX_GreetingsPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
