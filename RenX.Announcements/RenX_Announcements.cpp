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
#include "RenX_Announcements.h"
#include "RenX_Core.h"
#include "RenX_Server.h"

RenX_AnnouncementsPlugin pluginInstance;

void announce(unsigned int)
{
	if (pluginInstance.random == false)
	{
		pluginInstance.lastLine++;
		if (pluginInstance.lastLine == pluginInstance.announcementsFile.getLineCount()) pluginInstance.lastLine = 0;
	}
	else
	{
		unsigned int trand;
		do trand = rand() % pluginInstance.announcementsFile.getLineCount();
		while (trand == pluginInstance.lastLine);
		pluginInstance.lastLine = trand;
	}
	const Jupiter::ReadableString &announcement = pluginInstance.announcementsFile.getLine(pluginInstance.lastLine);
	RenX::Core *core = RenX::getCore();
	RenX::Server *server;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		server = core->getServer(i);
		if (server->players.size() != 0)
			server->sendMessage(announcement);
	}
}

RenX_AnnouncementsPlugin::RenX_AnnouncementsPlugin()
{
	RenX_AnnouncementsPlugin::announcementsFile.load(Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX.Announcements"), STRING_LITERAL_AS_REFERENCE("File"), STRING_LITERAL_AS_REFERENCE("Announcements.txt")));
	if (RenX_AnnouncementsPlugin::announcementsFile.getLineCount() != 0)
	{
		RenX_AnnouncementsPlugin::random = Jupiter::IRC::Client::Config->getBool(STRING_LITERAL_AS_REFERENCE("RenX.Announcements"), STRING_LITERAL_AS_REFERENCE("Random"));
		time_t delay = Jupiter::IRC::Client::Config->getInt(STRING_LITERAL_AS_REFERENCE("RenX.Announcements"), STRING_LITERAL_AS_REFERENCE("Delay"), 60);
		RenX_AnnouncementsPlugin::timer = new Jupiter::Timer(0, delay, announce);
		if (RenX_AnnouncementsPlugin::random == false) RenX_AnnouncementsPlugin::lastLine = RenX_AnnouncementsPlugin::announcementsFile.getLineCount() - 1;
	}
	else fputs("[RenX.Announcements] ERROR: No announcements loaded." ENDL, stderr);
}

RenX_AnnouncementsPlugin::~RenX_AnnouncementsPlugin()
{
	RenX_AnnouncementsPlugin::timer->kill();
	RenX_AnnouncementsPlugin::announcementsFile.unload();
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
