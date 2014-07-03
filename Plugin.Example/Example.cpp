/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Justin James <justin.aj@hotmail.com>
 */

#include "Example.h"
#include "IRC_Bot.h"

void ExamplePlugin::OnConnect(Jupiter::IRC::Client *server)
{
	server->sendNotice(STRING_LITERAL_AS_REFERENCE("Agent"), STRING_LITERAL_AS_REFERENCE("Honey, I'm home!"));
}

// Example IRC Command Implementation

void ExampleIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("example"));
}

void ExampleIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("This is an example command!"));
}

const Jupiter::ReadableString &ExampleIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(helpmsg, "This is just an example command. It takes no parameters!");
	return helpmsg;
}

IRC_COMMAND_INIT(ExampleIRCCommand)


// Plugin instantiation and entry point.
ExamplePlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
