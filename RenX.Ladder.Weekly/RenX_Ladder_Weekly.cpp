/**
 * Copyright (C) 2016-2017 Jessica James.
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
#include "RenX_Ladder_Weekly.h"

using namespace Jupiter::literals;

bool RenX_Ladder_Weekly_TimePlugin::initialize()
{
	// Load database
	this->database.process_file(this->config.get("LadderDatabase"_jrs, "Ladder.Weekly.db"_jrs));
	this->database.setName(this->config.get("DatabaseName"_jrs, "Weekly"_jrs));
	this->database.setOutputTimes(this->config.get<bool>("OutputTimes"_jrs, false));

	__time64_t tm_Time = time(0);
	this->last_sorted_day = gmtime(std::addressof<const time_t>(tm_Time))->tm_wday;
	this->reset_day = this->config.get<int>("ResetDay"_jrs);
	this->database.OnPreUpdateLadder = OnPreUpdateLadder;

	// Force database to default, if desired
	if (this->config.get<bool>("ForceDefault"_jrs, false))
		RenX::default_ladder_database = &this->database;

	return true;
}

// Plugin instantiation and entry point.
RenX_Ladder_Weekly_TimePlugin pluginInstance;

void OnPreUpdateLadder(RenX::LadderDatabase &database, RenX::Server &, const RenX::TeamType &)
{
	__time64_t tm_Time = time(0);
	tm *tm_ptr = gmtime(std::addressof<const time_t>(tm_Time));
	if (pluginInstance.last_sorted_day != tm_ptr->tm_wday && tm_ptr->tm_wday == pluginInstance.reset_day)
		database.erase();
	pluginInstance.last_sorted_day = tm_ptr->tm_wday;
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
