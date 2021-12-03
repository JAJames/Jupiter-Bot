/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include "Example.h"
#include "IRC_Bot.h"


void ExamplePlugin::OnConnect(Jupiter::IRC::Client *server)
{
	server->sendNotice("Agent"sv, "Honey, I'm home!"sv);
}

// Example IRC Command Implementation

void ExampleIRCCommand::create()
{
	this->addTrigger("example"sv);
}

void ExampleIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	source->sendMessage(channel, "This is an example command!"sv);
}

std::string_view ExampleIRCCommand::getHelp(std::string_view )
{
	static auto helpmsg = "This is just an example command. It takes no parameters!"sv;
	return helpmsg;
}

IRC_COMMAND_INIT(ExampleIRCCommand)


// Plugin instantiation and entry point.
ExamplePlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
