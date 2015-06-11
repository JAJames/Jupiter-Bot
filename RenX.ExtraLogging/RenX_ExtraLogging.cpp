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
#include "RenX_ExtraLogging.h"
#include "RenX_Server.h"
#include "RenX_Tags.h"

RenX_ExtraLoggingPlugin::RenX_ExtraLoggingPlugin()
{
	RenX_ExtraLoggingPlugin::day = localtime(std::addressof<const time_t>(time(nullptr)))->tm_yday;
}

RenX_ExtraLoggingPlugin::~RenX_ExtraLoggingPlugin()
{
	if (RenX_ExtraLoggingPlugin::file != nullptr)
		fclose(RenX_ExtraLoggingPlugin::file);
}

int RenX_ExtraLoggingPlugin::OnRehash()
{
	if (RenX_ExtraLoggingPlugin::file != nullptr)
		fclose(RenX_ExtraLoggingPlugin::file);
	return !RenX_ExtraLoggingPlugin::init();
}

bool RenX_ExtraLoggingPlugin::init()
{
	RenX_ExtraLoggingPlugin::filePrefix = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("FilePrefix"), Jupiter::StringS::Format("[%.*s] %.*s", RenX::tags->timeTag.size(), RenX::tags->timeTag.ptr(), RenX::tags->serverPrefixTag.size(), RenX::tags->serverPrefixTag.ptr()));
	RenX_ExtraLoggingPlugin::consolePrefix = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("ConsolePrefix"), RenX_ExtraLoggingPlugin::filePrefix);
	RenX_ExtraLoggingPlugin::newDayFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("NewDayFormat"), Jupiter::StringS::Format("Time: %.*s %.*s", RenX::tags->timeTag.size(), RenX::tags->timeTag.ptr(), RenX::tags->dateTag.size(), RenX::tags->dateTag.ptr()));
	RenX_ExtraLoggingPlugin::printToConsole = Jupiter::IRC::Client::Config->getBool(this->getName(), STRING_LITERAL_AS_REFERENCE("PrintToConsole"), true);
	const Jupiter::CStringS logFile = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("LogFile"));

	RenX::sanitizeTags(RenX_ExtraLoggingPlugin::filePrefix);
	RenX::sanitizeTags(RenX_ExtraLoggingPlugin::consolePrefix);
	RenX::sanitizeTags(RenX_ExtraLoggingPlugin::newDayFmt);

	if (logFile.isNotEmpty())
	{
		RenX_ExtraLoggingPlugin::file = fopen(logFile.c_str(), "a+b");
		if (RenX_ExtraLoggingPlugin::file != nullptr && RenX_ExtraLoggingPlugin::newDayFmt.isNotEmpty())
		{
			Jupiter::String line = RenX_ExtraLoggingPlugin::newDayFmt;
			RenX::processTags(line);
			line.println(RenX_ExtraLoggingPlugin::file);
		}
	}
	else
		RenX_ExtraLoggingPlugin::file = nullptr;

	return RenX_ExtraLoggingPlugin::file != nullptr || RenX_ExtraLoggingPlugin::printToConsole;
}

int RenX_ExtraLoggingPlugin::think()
{
	if (RenX_ExtraLoggingPlugin::file != nullptr && RenX_ExtraLoggingPlugin::newDayFmt.isNotEmpty())
	{
		int currentDay = localtime(std::addressof<const time_t>(time(nullptr)))->tm_yday;
		if (currentDay != RenX_ExtraLoggingPlugin::day)
		{
			RenX_ExtraLoggingPlugin::day = currentDay;
			Jupiter::String line = RenX_ExtraLoggingPlugin::newDayFmt;
			RenX::processTags(line);
			line.println(RenX_ExtraLoggingPlugin::file);
		}
	}
	return 0;
}

void RenX_ExtraLoggingPlugin::RenX_OnRaw(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	if (RenX_ExtraLoggingPlugin::printToConsole)
	{
		if (RenX_ExtraLoggingPlugin::filePrefix.isNotEmpty())
		{
			Jupiter::StringS cPrefix = RenX_ExtraLoggingPlugin::filePrefix;
			RenX::processTags(cPrefix, server);
			cPrefix.print(stdout);
			fputc(' ', stdout);
		}
		raw.println(stdout);
	}

	if (RenX_ExtraLoggingPlugin::file != nullptr)
	{
		if (RenX_ExtraLoggingPlugin::filePrefix.isNotEmpty())
		{
			Jupiter::StringS fPrefix = RenX_ExtraLoggingPlugin::filePrefix;
			RenX::processTags(fPrefix, server);
			fPrefix.print(RenX_ExtraLoggingPlugin::file);
			fputc(' ', RenX_ExtraLoggingPlugin::file);
		}
		raw.println(RenX_ExtraLoggingPlugin::file);
		fflush(RenX_ExtraLoggingPlugin::file);
	}
}

// Plugin instantiation and entry point.
RenX_ExtraLoggingPlugin pluginInstance;

extern "C" __declspec(dllexport) bool load()
{
	return pluginInstance.init();
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
