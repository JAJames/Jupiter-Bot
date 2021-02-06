/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Sarah E. <sarah.evans@qq.com>
 */

#include <iostream>
#include <fstream>
#include <string>
#include "RenX_ChatLogging.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"

void RenX_ChatLogPlugin::PrepFile()
{
	// Check if date changed (Format: YYYY-MM-DD)
	std::string current_date = getTimeFormat("%F");
	std::string full_date = getTimeFormat("%c");

	if (current_date == last_date) {
		return;
	}

	last_date = current_date;

	// Close any currently opened file

	if (fs.is_open()) {
		fs.close();
	}

	// Open new file

	std::string file_name = "ChatLog_" + current_date + ".log";
	fs.open(file_name, std::fstream::out | std::fstream::app);

	fs << "Session Start: "
		<< full_date
		<< std::endl;
}

bool RenX_ChatLogPlugin::initialize()
{
	PrepFile();

	return fs.is_open();
}

RenX_ChatLogPlugin::~RenX_ChatLogPlugin()
{
	if (fs.is_open()) {
		fs.close();
	}
}

std::ostream& operator<<(std::ostream& in_stream, const Jupiter::ReadableString& in_string) {
	in_stream.write(in_string.ptr(), in_string.size());
	return in_stream;
}

void RenX_ChatLogPlugin::RenX_OnChat(RenX::Server& server, const RenX::PlayerInfo& player, const Jupiter::ReadableString& message)
{
	WriteToLog(server, player, message, "ALL");
}

void RenX_ChatLogPlugin::RenX_OnTeamChat(RenX::Server& server, const RenX::PlayerInfo& player, const Jupiter::ReadableString& message)
{
	WriteToLog(server, player, message, "TEAM");
}

void RenX_ChatLogPlugin::WriteToLog(RenX::Server& server, const RenX::PlayerInfo& player, const Jupiter::ReadableString& message, std::string in_prefix)
{
	// Check if new file needs to be opened
	PrepFile();

	if (!fs.is_open()) {
		return;
	}

	const std::string& serverHostname = server.getSocketHostname();
	unsigned short serverPort = server.getSocketPort();

	fs << getTimeFormat("%T")
		<< " "
		<< serverHostname
		<< ":"
		<< serverPort
		<< " "
		<< in_prefix
		<< " "
		<< player.name
		<< ": "
		<< message
		<< std::endl;
}

// Plugin instantiation and entry point.
RenX_ChatLogPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
