/**
 * Copyright (C) 2016-2021 Jessica James.
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

#include <ctime>
#include "Jupiter/IRC_Client.h"
#include "RenX_Ladder_Daily.h"

using namespace std::literals;

bool RenX_Ladder_Daily_TimePlugin::initialize() {
	time_t current_time = time(0);
	// Load database
	this->database.process_file(this->config.get("LadderDatabase"sv, "Ladder.Daily.db"sv));
	this->database.setName(this->config.get("DatabaseName"sv, "Daily"sv));
	this->database.setOutputTimes(this->config.get<bool>("OutputTimes"sv, false));

	this->last_sorted_day = gmtime(&current_time)->tm_wday;
	this->database.OnPreUpdateLadder = OnPreUpdateLadder;

	// Force database to default, if desired
	if (this->config.get<bool>("ForceDefault"sv, false)) {
		RenX::default_ladder_database = &this->database;
	}

	return true;
}

// Plugin instantiation and entry point.
RenX_Ladder_Daily_TimePlugin pluginInstance;

void OnPreUpdateLadder(RenX::LadderDatabase &database, RenX::Server &, const RenX::TeamType &) {
	time_t current_time = time(0);
	tm *tm_ptr = gmtime(&current_time);
	if (pluginInstance.last_sorted_day != tm_ptr->tm_wday) {
		database.erase();
	}
	pluginInstance.last_sorted_day = tm_ptr->tm_wday;
}

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
