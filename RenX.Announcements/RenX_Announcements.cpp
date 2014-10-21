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

void announce_(unsigned int x)
{
	pluginInstance.announce(x);
}

void RenX_AnnouncementsPlugin::announce(unsigned int)
{
	if (RenX_AnnouncementsPlugin::random == false)
	{
		RenX_AnnouncementsPlugin::lastLine++;
		if (RenX_AnnouncementsPlugin::lastLine == RenX_AnnouncementsPlugin::announcementsFile.getLineCount()) RenX_AnnouncementsPlugin::lastLine = 0;
	}
	else
	{
		unsigned int trand;
		do trand = rand() % RenX_AnnouncementsPlugin::announcementsFile.getLineCount();
		while (trand == RenX_AnnouncementsPlugin::lastLine);
		RenX_AnnouncementsPlugin::lastLine = trand;
	}
	Jupiter::StringS announcement = RenX_AnnouncementsPlugin::announcementsFile.getLine(RenX_AnnouncementsPlugin::lastLine);
	announcement.replace(RenX_AnnouncementsPlugin::dateTag, Jupiter::ReferenceString(getTimeFormat(RenX_AnnouncementsPlugin::dateFmt.c_str())));
	announcement.replace(RenX_AnnouncementsPlugin::timeTag, Jupiter::ReferenceString(getTimeFormat(RenX_AnnouncementsPlugin::timeFmt.c_str())));
	
	Jupiter::String msg;
	RenX::Core *core = RenX::getCore();
	RenX::Server *server;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		server = core->getServer(i);
		if (server->players.size() != 0)
		{
			msg = announcement;
			msg.replace(RenX_AnnouncementsPlugin::rulesTag, server->getRules());
			server->sendMessage(msg);
		}
	}
}

int RenX_AnnouncementsPlugin::OnRehash()
{
	RenX_AnnouncementsPlugin::announcementsFile.unload();
	return RenX_AnnouncementsPlugin::init();
}

int RenX_AnnouncementsPlugin::init()
{
	RenX_AnnouncementsPlugin::dateTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DateTag"), STRING_LITERAL_AS_REFERENCE("{DATE}"));
	RenX_AnnouncementsPlugin::timeTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TimeTag"), STRING_LITERAL_AS_REFERENCE("{TIME}"));
	RenX_AnnouncementsPlugin::rulesTag = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("RulesTag"), STRING_LITERAL_AS_REFERENCE("{RULES}"));;
	RenX_AnnouncementsPlugin::dateFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("DateFormat"), STRING_LITERAL_AS_REFERENCE("%A, %B %d, %Y"));
	RenX_AnnouncementsPlugin::timeFmt = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("TimeFormat"), STRING_LITERAL_AS_REFERENCE("%H:%M:%S"));
	RenX_AnnouncementsPlugin::random = Jupiter::IRC::Client::Config->getBool(STRING_LITERAL_AS_REFERENCE("RenX.Announcements"), STRING_LITERAL_AS_REFERENCE("Random"));

	RenX_AnnouncementsPlugin::announcementsFile.load(Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX.Announcements"), STRING_LITERAL_AS_REFERENCE("File"), STRING_LITERAL_AS_REFERENCE("Announcements.txt")));
	if (RenX_AnnouncementsPlugin::announcementsFile.getLineCount() == 0)
	{
		fputs("[RenX.Announcements] ERROR: No announcements loaded." ENDL, stderr);
		return -1;
	}
	time_t delay = Jupiter::IRC::Client::Config->getInt(STRING_LITERAL_AS_REFERENCE("RenX.Announcements"), STRING_LITERAL_AS_REFERENCE("Delay"), 60);
	RenX_AnnouncementsPlugin::timer = new Jupiter::Timer(0, delay, announce_);
	if (RenX_AnnouncementsPlugin::random == false)
		RenX_AnnouncementsPlugin::lastLine = RenX_AnnouncementsPlugin::announcementsFile.getLineCount() - 1;
	return 0;
}

RenX_AnnouncementsPlugin::~RenX_AnnouncementsPlugin()
{
	RenX_AnnouncementsPlugin::timer->kill();
	RenX_AnnouncementsPlugin::announcementsFile.unload();
}

extern "C" __declspec(dllexport) bool load()
{
	return pluginInstance.init() == 0;
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
