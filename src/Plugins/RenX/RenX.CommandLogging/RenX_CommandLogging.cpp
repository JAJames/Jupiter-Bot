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
#include "RenX_CommandLogging.h"
#include "RenX_GameCommand.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_Functions.h"

using namespace Jupiter::literals;

void RenX_CommandLoggingPlugin::PrepFile()
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

	std::string file_name = "CommandLog_" + current_date + ".log";
	fs.open(file_name, std::fstream::out | std::fstream::app);

	fs << "Session Start: "
		<< full_date
		<< std::endl;
}

bool RenX_CommandLoggingPlugin::initialize()
{
	RenX_CommandLoggingPlugin::min_access = this->config.get<int>("MinPlayerLevelToLog"_jrs, 1);
	RenX_CommandLoggingPlugin::min_cmd_access = this->config.get<int>("MinCommandLevelToLog"_jrs, 1);

	PrepFile();

	return fs.is_open();
}

RenX_CommandLoggingPlugin::~RenX_CommandLoggingPlugin()
{
	if (fs.is_open()) {
		fs.close();
	}
}

void RenX_CommandLoggingPlugin::RenX_OnCommandTriggered(RenX::Server& server, const Jupiter::ReadableString& trigger, RenX::PlayerInfo& player, const Jupiter::ReadableString& parameters, RenX::GameCommand& command)
{
	if (player.access < min_access || command.getAccessLevel() < min_cmd_access) {
		return;
	}

	WriteToLog(server, player, static_cast<std::string>(trigger) + " " + parameters);
}

std::ostream& operator<<(std::ostream& in_stream, const Jupiter::ReadableString& in_string) {
	in_stream.write(in_string.data(), in_string.size());
	return in_stream;
}

void RenX_CommandLoggingPlugin::WriteToLog(RenX::Server& server, const RenX::PlayerInfo& player, const Jupiter::ReadableString& message)
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
		<< std::to_string(player.access)
		<< " "
		<< player.name
		<< ": "
		<< message
		<< std::endl;
}

 // Plugin instantiation and entry point.
RenX_CommandLoggingPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin * getPlugin()
{
	return &pluginInstance;
}


