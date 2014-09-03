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

#include <cstring>
#include "Jupiter/Functions.h"
#include "PluginManager.h"
#include "IRC_Bot.h"

// Plugin Console Command
PluginConsoleCommand::PluginConsoleCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("plugin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("plugins"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("module"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("modules"));
}

void PluginConsoleCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty() || parameters.matchi("list*"))
	{
		printf("There are %u plugins loaded." ENDL, Jupiter::plugins->size());
		for (size_t i = 0; i != Jupiter::plugins->size(); i++) Jupiter::plugins->get(i)->getName().println(stdout);
	}
	else
	{
		if (parameters.matchi("load *"))
		{
			if (Jupiter::loadPlugin(Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE)) == nullptr)
				puts("Error: Failed to load plugin.");
			else
				puts("Plugin successfully loaded.");
		}
		else if (parameters.matchi("unload *"))
		{
			Jupiter::ReferenceString pluginName = Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);
			if (Jupiter::getPlugin(pluginName) == nullptr)
				puts("Error: Plugin does not exist.");
			else if (Jupiter::freePlugin(pluginName) == false)
				puts("Error: Failed to unload plugin.");
			else
				puts("Plugin successfully unloaded.");
		}
		else
			puts("Error: Invalid Syntax. Syntax: plugin {[list], <load> <plugin>, <unload> <plugin>}");
	}
}

const Jupiter::ReadableString &PluginConsoleCommand::getHelp(const Jupiter::ReadableString &parameters)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(loadHelp, "Loads a plugin by file name. Do not include a file extension. Syntax: plugin load <plugin>");
	static STRING_LITERAL_AS_NAMED_REFERENCE(unloadHelp, "Unloads a plugin by name. Syntax: plugin unload <plugin>");
	static STRING_LITERAL_AS_NAMED_REFERENCE(listHelp, "Lists all of the plugins currently loaded. Syntax: plugin [list]");
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Manages plugins. Syntax: plugin {[list], <load> <plugin>, <unload> <plugin>}");

	if (parameters.equalsi(STRING_LITERAL_AS_REFERENCE("load")))
		return loadHelp;
	if (parameters.equalsi(STRING_LITERAL_AS_REFERENCE("unload")))
		return unloadHelp;
	if (parameters.equalsi(STRING_LITERAL_AS_REFERENCE("list")))
		return listHelp;

	return defaultHelp;
}

CONSOLE_COMMAND_INIT(PluginConsoleCommand)

// Load Plugin

void LoadPluginIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("loadplugin"));
	this->setAccessLevel(5);
}

void LoadPluginIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters != nullptr)
	{
		if (Jupiter::getPlugin(parameters) != nullptr) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Plugin already exists. You must first unload the plugin."));
		else
		{
			if (Jupiter::loadPlugin(parameters) == nullptr) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Failed to load plugin."));
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Plugin successfully loaded."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: loadPlugin <plugin>"));
}

const Jupiter::ReadableString &LoadPluginIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Dynamically loads a plugin. Syntax: loadPlugin <plugin>");
	return defaultHelp;
}

IRC_COMMAND_INIT(LoadPluginIRCCommand)

// Free Plugin

void FreePluginIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("freeplugin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unloadplugin"));
	this->setAccessLevel(5);
}

void FreePluginIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters != nullptr)
	{
		if (Jupiter::getPlugin(parameters) == nullptr) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Plugin does not exist."));
		else if (Jupiter::freePlugin(parameters) == false) source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Failed to unload plugin."));
		else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Plugin successfully unloaded."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: freePlugin <plugin>"));
}

const Jupiter::ReadableString &FreePluginIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Dynamically unloads a plugin. Syntax: freePlugin <plugin>");
	return defaultHelp;
}

IRC_COMMAND_INIT(FreePluginIRCCommand)

// List Plugins

void ListPluginIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("plugins"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("plugin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("listplugin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("listplugins"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pluginlist"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pluginslist"));
	this->setAccessLevel(4);
}

void ListPluginIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(channel, Jupiter::StringS::Format("%u plugins loaded:", Jupiter::plugins->size()));
	for (size_t i = 0; i != Jupiter::plugins->size(); i++)
		source->sendMessage(channel, Jupiter::plugins->get(i)->getName());
}

const Jupiter::ReadableString &ListPluginIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Lists the currently loaded plugins. Syntax: pluginlist");
	return defaultHelp;
}

IRC_COMMAND_INIT(ListPluginIRCCommand)

// Plugin instantiation and entry point.
PluginManager pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
