/**
 * Copyright (C) 2014-2021 Jessica James.
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

#include <cstring>
#include "Jupiter/Functions.h"
#include "Jupiter/Timer.h" // This timer implementation isn't great, but it'll work for delaying reloads
#include "jessilib/word_split.hpp"
#include "jessilib/unicode.hpp"
#include "PluginManager.h"

using namespace std::literals;

// Plugin Generic Command
PluginGenericCommand::PluginGenericCommand() {
	this->addTrigger("plugin"sv);
	this->addTrigger("plugins"sv);
	this->addTrigger("module"sv);
	this->addTrigger("modules"sv);
}

Jupiter::GenericCommand::ResponseLine *PluginGenericCommand::trigger(std::string_view parameters) {
	auto parameters_view = static_cast<std::string_view>(parameters);
	Jupiter::GenericCommand::ResponseLine *result = new Jupiter::GenericCommand::ResponseLine();
	if (parameters_view.empty() || jessilib::starts_withi(parameters_view, "list"sv)) {
		Jupiter::GenericCommand::ResponseLine *line = result->set(string_printf("There are %u plugins loaded:", Jupiter::plugins.size()), GenericCommand::DisplayType::PublicSuccess);
		for (auto& plugin : Jupiter::plugins) {
			line->next = new Jupiter::GenericCommand::ResponseLine(plugin->getName(), GenericCommand::DisplayType::PublicSuccess);
			line = line->next;
		}

		return result;
	}

	auto find_plugin = [](std::string_view name) {
		for (const auto& plugin : Jupiter::plugins) {
			std::string_view plugin_name = plugin->getName();
			if (jessilib::equalsi(plugin_name, name)) {
				return plugin;
			}
		}

		return static_cast<Jupiter::Plugin*>(nullptr);
	};

	auto split_params = jessilib::word_split_once_view(parameters_view, WHITESPACE_SV);
	if (jessilib::starts_withi(parameters_view, "load "sv)) {
		if (Jupiter::Plugin::load(split_params.second) == nullptr) {
			return result->set("Error: Failed to load plugin."sv, GenericCommand::DisplayType::PublicError);
		}

		return result->set("Plugin successfully loaded."sv, GenericCommand::DisplayType::PublicSuccess);
	}

	if (jessilib::starts_withi(parameters_view, "unload "sv)) {
		auto plugin = find_plugin(split_params.second);
		if (plugin == nullptr) {
			return result->set("Error: Plugin does not exist."sv, GenericCommand::DisplayType::PublicError);
		}

		if (!Jupiter::Plugin::free(plugin)) {
			return result->set("Error: Failed to unload plugin."sv, GenericCommand::DisplayType::PublicError);
		}

		return result->set("Plugin successfully unloaded."sv, GenericCommand::DisplayType::PublicSuccess);
	}

	if (jessilib::starts_withi(parameters_view, "reload"sv)) {
		if (split_params.second.empty()
			|| split_params.second == "*") {
			// Reinitialize all plugins on next tick
			new Jupiter::Timer(1, std::chrono::milliseconds{0}, [](unsigned int, void*) {
				Jupiter::reinitialize_plugins();
			}, true);

			return result->set("Triggering full plugin reload..."sv, GenericCommand::DisplayType::PublicSuccess);
		}
		else {
			// A specific plugin
			auto plugin = find_plugin(split_params.second);
			if (plugin == nullptr) {
				return result->set("Error: Plugin does not exist."sv, GenericCommand::DisplayType::PublicError);
			}

			std::string_view plugin_name = plugin->getName();
			if (!Jupiter::Plugin::free(plugin)) {
				return result->set("Error: Failed to unload plugin."sv, GenericCommand::DisplayType::PublicError);
			}

			if (Jupiter::Plugin::load(plugin_name) == nullptr) {
				return result->set("Error: Failed to load plugin."sv, GenericCommand::DisplayType::PublicError);
			}

			return result->set("Plugin successfully reloaded."sv, GenericCommand::DisplayType::PublicSuccess);
		}
	}
	return result->set("Error: Invalid Syntax. Syntax: plugin {[list], <load> <plugin>, <unload> <plugin>, <reload> [all|plugin]}"sv, GenericCommand::DisplayType::PrivateError);
}

std::string_view PluginGenericCommand::getHelp(std::string_view parameters) {
	static constexpr std::string_view loadHelp = "Loads a plugin by file name. Do not include a file extension. Syntax: plugin load <plugin>"sv;
	static constexpr std::string_view unloadHelp = "Unloads a plugin by name. Syntax: plugin unload <plugin>"sv;
	static constexpr std::string_view listHelp = "Lists all of the plugins currently loaded. Syntax: plugin [list]"sv;
	static constexpr std::string_view defaultHelp = "Manages plugins. Syntax: plugin {[list], <load> <plugin>, <unload> <plugin>, <reload> [plugin]}"sv;

	if (jessilib::equalsi(parameters, "load"sv)) {
		return loadHelp;
	}

	if (jessilib::equalsi(parameters, "unload"sv)) {
		return unloadHelp;
	}

	if (jessilib::equalsi(parameters, "list"sv)) {
		return listHelp;
	}

	return defaultHelp;
}

GENERIC_COMMAND_INIT(PluginGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(PluginGenericCommand)

// Plugin instantiation and entry point.
PluginManager pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
