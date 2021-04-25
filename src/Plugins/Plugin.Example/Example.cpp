/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include "Example.h"
#include "IRC_Bot.h"

using namespace Jupiter::literals;

void ExamplePlugin::OnConnect(Jupiter::IRC::Client *server)
{
	server->sendNotice("Agent"_jrs, "Honey, I'm home!"_jrs);
}

// Example IRC Command Implementation

void ExampleIRCCommand::create()
{
	this->addTrigger("example"_jrs);
}

void ExampleIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(channel, "This is an example command!"_jrs);
}

const Jupiter::ReadableString &ExampleIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static auto helpmsg = "This is just an example command. It takes no parameters!"_jrs;
	return helpmsg;
}

IRC_COMMAND_INIT(ExampleIRCCommand)


// Plugin instantiation and entry point.
ExamplePlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
