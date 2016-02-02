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

#include <ctime>
#include "Jupiter/INIFile.h"
#include "Jupiter/IRC_Client.h"
#include "RenX_Ladder_Weekly.h"

using namespace Jupiter::literals;

RenX_Ladder_Weekly_TimePlugin::RenX_Ladder_Weekly_TimePlugin()
{
	// Load database
	this->database.process_file(Jupiter::IRC::Client::Config->get(this->getName(), "LadderDatabase"_jrs, "Ladder.Weekly.db"_jrs));
	this->database.setName(Jupiter::IRC::Client::Config->get(this->getName(), "DatabaseName"_jrs, "Weekly"_jrs));

	this->last_sorted_day = gmtime(std::addressof<const time_t>(time(0)))->tm_wday;
	this->reset_day = Jupiter::IRC::Client::Config->getInt(this->getName(), "ResetDay"_jrs);
	this->database.OnPreUpdateLadder = OnPreUpdateLadder;

	// Force database to default, if desired
	if (Jupiter::IRC::Client::Config->getBool(this->getName(), "ForceDefault"_jrs, false))
		RenX::default_ladder_database = &this->database;
}

// Plugin instantiation and entry point.
RenX_Ladder_Weekly_TimePlugin pluginInstance;

void OnPreUpdateLadder(RenX::LadderDatabase &database, RenX::Server *server, const RenX::TeamType &team, bool output_times)
{
	tm *tm_ptr = gmtime(std::addressof<const time_t>(time(0)));
	if (pluginInstance.last_sorted_day != tm_ptr->tm_wday && tm_ptr->tm_wday == pluginInstance.reset_day)
		database.erase();
	pluginInstance.last_sorted_day = tm_ptr->tm_wday;
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
