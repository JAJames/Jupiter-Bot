/**
 * Copyright (C) 2014-2015 Justin James.
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

using namespace Jupiter::literals;

// Plugin Generic Command
PluginGenericCommand::PluginGenericCommand()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("plugin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("plugins"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("module"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("modules"));
}

GenericCommand::ResponseLine *PluginGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	GenericCommand::ResponseLine *ret = new GenericCommand::ResponseLine();
	if (parameters.isEmpty() || parameters.matchi("list*"))
	{
		GenericCommand::ResponseLine *line = ret->set(Jupiter::String::Format("There are %u plugins loaded:", Jupiter::plugins->size()), GenericCommand::DisplayType::PublicSuccess);
		for (size_t i = 0; i != Jupiter::plugins->size(); i++)
		{
			line->next = new GenericCommand::ResponseLine(Jupiter::plugins->get(i)->getName(), GenericCommand::DisplayType::PublicSuccess);
			line = line->next;
		}
		return ret;
	}

	if (parameters.matchi("load *"))
	{
		if (Jupiter::loadPlugin(Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE)) == nullptr)
			return ret->set("Error: Failed to load plugin."_jrs, GenericCommand::DisplayType::PublicError);
		else
			return ret->set("Plugin successfully loaded."_jrs, GenericCommand::DisplayType::PublicSuccess);
	}
	if (parameters.matchi("unload *"))
	{
		Jupiter::ReferenceString pluginName = Jupiter::ReferenceString::gotoWord(parameters, 1, WHITESPACE);
		if (Jupiter::getPlugin(pluginName) == nullptr)
			return ret->set("Error: Plugin does not exist."_jrs, GenericCommand::DisplayType::PublicError);
		if (Jupiter::freePlugin(pluginName) == false)
			return ret->set("Error: Failed to unload plugin."_jrs, GenericCommand::DisplayType::PublicError);
		return ret->set("Plugin successfully unloaded."_jrs, GenericCommand::DisplayType::PublicSuccess);
	}
	return ret->set("Error: Invalid Syntax. Syntax: plugin {[list], <load> <plugin>, <unload> <plugin>}"_jrs, GenericCommand::DisplayType::PrivateError);
}

const Jupiter::ReadableString &PluginGenericCommand::getHelp(const Jupiter::ReadableString &parameters)
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

GENERIC_COMMAND_INIT(PluginGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(PluginGenericCommand)
GENERIC_COMMAND_AS_IRC_COMMAND_ACCESS_CREATE(PluginGenericCommand, 5)

// Plugin instantiation and entry point.
PluginManager pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
