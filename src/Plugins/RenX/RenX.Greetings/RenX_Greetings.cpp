/**
 * Copyright (C) 2014-2021 Jessica James.
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

void RenX_GreetingsPlugin::RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player) {
	auto sendMessage = [&](const Jupiter::ReadableString &m) {
		Jupiter::String msg = m;

		RenX::sanitizeTags(msg);
		RenX::processTags(msg, &server, &player);

		if (m_sendPrivate)
			server.sendMessage(player, msg);
		else
			server.sendMessage(msg);
	};

	if (player.isBot == false && server.isMatchInProgress()) {
		switch (m_sendMode) {
		case 0:
			m_lastLine = rand() % m_greetingsFile.getLineCount();
			sendMessage(m_greetingsFile.getLine(m_lastLine));
			break;
		case 1:
			if (++m_lastLine == m_greetingsFile.getLineCount())
				m_lastLine = 0;
			sendMessage(m_greetingsFile.getLine(m_lastLine));
			break;
		case 2:
			for (m_lastLine = 0; m_lastLine != m_greetingsFile.getLineCount(); m_lastLine++)
				sendMessage(m_greetingsFile.getLine(m_lastLine));
			break;
		default:
			return;
		}
	}
}

int RenX_GreetingsPlugin::OnRehash() {
	RenX::Plugin::OnRehash();

	m_greetingsFile.unload();
	return initialize() ? 0 : -1;
}

bool RenX_GreetingsPlugin::initialize() {
	m_sendPrivate = this->config.get<bool>("SendPrivate"_jrs, true);
	m_sendMode = this->config.get<unsigned int>("SendMode"_jrs, 0);
	m_greetingsFile.load(this->config.get("GreetingsFile"_jrs, "RenX.Greetings.txt"_jrs));
	if (m_greetingsFile.getLineCount() == 0)
		m_greetingsFile.addData("Please notify the server administrator to properly configure or disable server greetings.\r\n"_jrs);
	m_lastLine = m_greetingsFile.getLineCount() - 1;

	return true;
}

// Plugin instantiation and entry point.
RenX_GreetingsPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
