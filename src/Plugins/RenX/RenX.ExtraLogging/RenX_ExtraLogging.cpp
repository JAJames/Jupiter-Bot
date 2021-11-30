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
#include "RenX_ExtraLogging.h"
#include "RenX_Server.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;

RenX_ExtraLoggingPlugin::RenX_ExtraLoggingPlugin()
{
    time_t current_time = time(nullptr);
    RenX_ExtraLoggingPlugin::day = localtime(&current_time)->tm_yday;
}

RenX_ExtraLoggingPlugin::~RenX_ExtraLoggingPlugin()
{
	if (RenX_ExtraLoggingPlugin::file != nullptr)
		fclose(RenX_ExtraLoggingPlugin::file);
}

int RenX_ExtraLoggingPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();

	if (RenX_ExtraLoggingPlugin::file != nullptr)
		fclose(RenX_ExtraLoggingPlugin::file);

	return this->initialize() ? 0 : -1;
}

bool RenX_ExtraLoggingPlugin::initialize() {
	RenX_ExtraLoggingPlugin::filePrefix = this->config.get("FilePrefix"_jrs, Jupiter::StringS::Format("[%.*s] %.*s", RenX::tags->timeTag.size(),
		RenX::tags->timeTag.data(), RenX::tags->serverPrefixTag.size(),
		RenX::tags->serverPrefixTag.data()));
	RenX_ExtraLoggingPlugin::consolePrefix = this->config.get("ConsolePrefix"_jrs, RenX_ExtraLoggingPlugin::filePrefix);
	RenX_ExtraLoggingPlugin::newDayFmt = this->config.get("NewDayFormat"_jrs, Jupiter::StringS::Format("Time: %.*s %.*s", RenX::tags->timeTag.size(),
		RenX::tags->timeTag.data(), RenX::tags->dateTag.size(),
		RenX::tags->dateTag.data()));
	RenX_ExtraLoggingPlugin::printToConsole = this->config.get<bool>("PrintToConsole"_jrs, true);
	const std::string logFile = static_cast<std::string>(this->config.get("LogFile"_jrs));

	RenX::sanitizeTags(RenX_ExtraLoggingPlugin::filePrefix);
	RenX::sanitizeTags(RenX_ExtraLoggingPlugin::consolePrefix);
	RenX::sanitizeTags(RenX_ExtraLoggingPlugin::newDayFmt);

	if (!logFile.empty()) {
		RenX_ExtraLoggingPlugin::file = fopen(logFile.c_str(), "a+b");
		if (RenX_ExtraLoggingPlugin::file != nullptr && !RenX_ExtraLoggingPlugin::newDayFmt.empty()) {
			std::string line = RenX_ExtraLoggingPlugin::newDayFmt;
			RenX::processTags(line);
			fwrite(line.data(), sizeof(char), line.size(), file);
			fputs("\r\n", file);
		}
	}
	else
		RenX_ExtraLoggingPlugin::file = nullptr;

	return RenX_ExtraLoggingPlugin::file != nullptr || RenX_ExtraLoggingPlugin::printToConsole;
}

int RenX_ExtraLoggingPlugin::think() {
	if (RenX_ExtraLoggingPlugin::file != nullptr && !RenX_ExtraLoggingPlugin::newDayFmt.empty()) {
		time_t current_time = time(nullptr);
		int currentDay = localtime(&current_time)->tm_yday;
		if (currentDay != RenX_ExtraLoggingPlugin::day)
		{
			RenX_ExtraLoggingPlugin::day = currentDay;
			std::string line = RenX_ExtraLoggingPlugin::newDayFmt;
			RenX::processTags(line);
			fwrite(line.data(), sizeof(char), line.size(), file);
			fputs("\r\n", file);
		}
	}
	return 0;
}

void RenX_ExtraLoggingPlugin::RenX_OnRaw(RenX::Server &server, const Jupiter::ReadableString &raw) {
	if (RenX_ExtraLoggingPlugin::printToConsole) {
		if (!RenX_ExtraLoggingPlugin::consolePrefix.empty()) {
			std::string cPrefix = RenX_ExtraLoggingPlugin::consolePrefix;
			RenX::processTags(cPrefix, &server);
			fwrite(cPrefix.data(), sizeof(char), cPrefix.size(), stdout);
			fputc(' ', stdout);
		}
		fwrite(raw.data(), sizeof(char), raw.size(), stdout);
		fputs("\r\n", stdout);
	}

	if (RenX_ExtraLoggingPlugin::file != nullptr) {
		if (!RenX_ExtraLoggingPlugin::filePrefix.empty()) {
			std::string fPrefix = RenX_ExtraLoggingPlugin::filePrefix;
			RenX::processTags(fPrefix, &server);
			fwrite(fPrefix.data(), sizeof(char), fPrefix.size(), file);
			fputc(' ', RenX_ExtraLoggingPlugin::file);
		}
		fwrite(raw.data(), sizeof(char), raw.size(), file);
		fputs("\r\n", file);
		fflush(RenX_ExtraLoggingPlugin::file);
	}
}

// Plugin instantiation and entry point.
RenX_ExtraLoggingPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
