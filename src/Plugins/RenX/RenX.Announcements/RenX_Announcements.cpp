/**
 * Copyright (C) 2014-2016 Jessica James.
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
#include "RenX_Announcements.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;
using namespace std::literals;

RenX_AnnouncementsPlugin pluginInstance;

void announce_(unsigned int x, void*)
{
	pluginInstance.announce(x, nullptr);
}

void RenX_AnnouncementsPlugin::announce(unsigned int, void *)
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
	RenX::sanitizeTags(announcement);
	
	Jupiter::String msg;
	RenX::Core *core = RenX::getCore();
	RenX::Server *server;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		server = core->getServer(i);
		if (server->players.size() != server->getBotCount())
		{
			msg = announcement;
			RenX::processTags(msg, server);
			server->sendMessage(msg);
		}
	}
}

int RenX_AnnouncementsPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();

	RenX_AnnouncementsPlugin::timer->kill();
	RenX_AnnouncementsPlugin::announcementsFile.unload();
	return this->initialize() ? 0 : -1;
}

bool RenX_AnnouncementsPlugin::initialize()
{
	RenX_AnnouncementsPlugin::random = this->config.get<bool>("Random"sv);

	RenX_AnnouncementsPlugin::announcementsFile.load(this->config.get("File"sv, "Announcements.txt"s));
	if (RenX_AnnouncementsPlugin::announcementsFile.getLineCount() == 0)
	{
		fputs("[RenX.Announcements] ERROR: No announcements loaded." ENDL, stderr);
		return false;
	}
	std::chrono::milliseconds delay = std::chrono::seconds(this->config.get<long long>("Delay"_jrs, 60));
	RenX_AnnouncementsPlugin::timer = new Jupiter::Timer(0, delay, announce_);
	if (RenX_AnnouncementsPlugin::random == false)
		RenX_AnnouncementsPlugin::lastLine = RenX_AnnouncementsPlugin::announcementsFile.getLineCount() - 1;
	return true;
}

RenX_AnnouncementsPlugin::~RenX_AnnouncementsPlugin()
{
	RenX_AnnouncementsPlugin::timer->kill();
	RenX_AnnouncementsPlugin::announcementsFile.unload();
}

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
