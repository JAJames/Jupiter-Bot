/**
 * Copyright (C) 2016 Jessica James.
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
#include "RenX_Ladder_All_Time.h"

using namespace std::literals;

bool RenX_Ladder_All_TimePlugin::initialize() {
	// Load database
	this->database.process_file(this->config.get("LadderDatabase"sv, "Ladder.db"sv));
	this->database.setName(this->config.get("DatabaseName"sv, "All-Time"sv));
	this->database.setOutputTimes(this->config.get<bool>("OutputTimes"sv, true));

	// Force database to default, if desired
	if (this->config.get<bool>("ForceDefault"sv, true)) {
		RenX::default_ladder_database = &this->database;
	}

	return true;
}

// Plugin instantiation and entry point.
RenX_Ladder_All_TimePlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
