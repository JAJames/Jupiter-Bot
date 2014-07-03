/**
 * Copyright (C) 2014 Justin James.
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

RenX_ExtraLoggingPlugin::RenX_ExtraLoggingPlugin()
{
	const Jupiter::CStringS logFile = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX.ExtraLogging"), STRING_LITERAL_AS_REFERENCE("LogFile"));
	if (logFile.isEmpty() == false)
		RenX_ExtraLoggingPlugin::file = fopen(logFile.c_str(), "a+b");
	else RenX_ExtraLoggingPlugin::file = nullptr;
}

RenX_ExtraLoggingPlugin::~RenX_ExtraLoggingPlugin()
{
	if (RenX_ExtraLoggingPlugin::file != nullptr)
		fclose(RenX_ExtraLoggingPlugin::file);
}

void RenX_ExtraLoggingPlugin::RenX_OnRaw(RenX::Server *server, const Jupiter::ReadableString &raw)
{
	raw.println(stdout);
	if (RenX_ExtraLoggingPlugin::file != nullptr)
	{
		const Jupiter::ReadableString &prefix = server->getPrefix();
		if (prefix.size() != 0)
		{
			prefix.print(RenX_ExtraLoggingPlugin::file);
			fputc(' ', RenX_ExtraLoggingPlugin::file);
		}
		raw.println(RenX_ExtraLoggingPlugin::file);
		fflush(RenX_ExtraLoggingPlugin::file);
	}
}



// Plugin instantiation and entry point.
RenX_ExtraLoggingPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
