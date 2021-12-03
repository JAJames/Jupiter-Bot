/**
 * Copyright (C) 2020 Jessica James.
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

#include "jessilib/unicode.hpp"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_Functions.h"
#include "RenX_PlayerInfo.h"
#include "RenX_KickDupes.h"

using namespace std::literals;

bool RenX_KickDupesPlugin::initialize() {
	return true;
}

void RenX_KickDupesPlugin::RenX_OnPlayerIdentify(RenX::Server &in_server, const RenX::PlayerInfo &in_player) {
	// Safety checks
	if (in_player.hwid.empty()) {
		// Somehow, the player's identified without any HWID. TODO: log this
		return;
	}

	// Exempt development servers from this check
	if (jessilib::findi(in_server.getGameVersion(), "-DEV"sv) != std::string::npos) {
		return;
	}

	// Check to see if any other players on the server have the same HWID
	size_t hits{};
	for (auto& player : in_server.players) {
		if (player.hwid == in_player.hwid && player.id != in_player.id) {
			// Two players have the same HWID, but separate player IDs; kick the pre-existing player if there's too many.
			if (++hits > s_tolerance) {
				in_server.forceKickPlayer(player, "Ghost client detected"sv);
			}
		}
	}
}

// Plugin instantiation and entry point.
RenX_KickDupesPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
