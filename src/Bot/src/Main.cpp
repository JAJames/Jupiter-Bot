/**
 * Copyright (C) 2013-2021 Jessica James.
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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <exception>
#include <thread>
#include <mutex>
#include "jessilib/unicode.hpp"
#include "jessilib/app_parameters.hpp"
#include "Jupiter/Functions.h"
#include "Jupiter/INIConfig.h"
#include "Jupiter/Socket.h"
#include "Jupiter/Plugin.h"
#include "Jupiter/Timer.h"
#include "jessilib/word_split.hpp"
#include "jessilib/serialize.hpp"
#include "IRC_Bot.h"
#include "Console_Command.h"
#include "IRC_Command.h"

#if defined _WIN32
#include <Windows.h>
#endif // _WIN32

using namespace std::literals;

Jupiter::INIConfig o_config;
Jupiter::Config *Jupiter::g_config = &o_config;
std::chrono::steady_clock::time_point Jupiter::g_start_time = std::chrono::steady_clock::now();

constexpr size_t INPUT_BUFFER_SIZE = 2048;

struct ConsoleInput {
	std::string input;
	std::mutex input_mutex;
	bool awaiting_processing = false;

	ConsoleInput() {
		input.reserve(INPUT_BUFFER_SIZE);
	}
} console_input;

void onTerminate() {
	std::cout << "Terminate signal received..." << std::endl;
}

void onExit() {
	std::cout << "Exit signal received; Cleaning up..." << std::endl;
	Jupiter::Socket::cleanup();
	std::cout << "Clean-up complete. Closing..." << std::endl;
}

void inputLoop() {
	std::string input;
	while (ftell(stdin) != -1 || errno != EBADF) {
		std::getline(std::cin, input);

	check_input_processing:

		std::lock_guard<std::mutex> guard(console_input.input_mutex);
		if (console_input.awaiting_processing == false)
		{
			console_input.input = input;
			console_input.awaiting_processing = true;
		}
		else // User input received before previous input was processed.
		{
			std::this_thread::sleep_for((std::chrono::seconds(1)));
			goto check_input_processing;
		}
	}
}

void initialize_plugins() {
	std::cout << "Loading plugins..." << std::endl;
	std::string_view plugin_list_str = Jupiter::g_config->get("Plugins"sv);
	if (plugin_list_str.empty()) {
		std::cout << "No plugins to load!" << std::endl;
	}
	else {
		// initialize plugins
		auto plugin_names = jessilib::word_split_view(plugin_list_str, WHITESPACE_SV);
		std::cout << "Attempting to load " << plugin_names.size() << " plugins..." << std::endl;

		for (const auto& plugin_name : plugin_names) {
			std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
			bool load_success = Jupiter::Plugin::load(plugin_name) != nullptr;
			double time_taken = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - load_start).count()) / 1000.0;

			if (load_success) {
				std::cout << "\"" << plugin_name << "\" loaded successfully (" << time_taken << "ms)." << std::endl;
			}
			else {
				std::cerr << "WARNING: Failed to load plugin \"" << plugin_name << "\" (" << time_taken << "ms)!" << std::endl;
			}
		}

		// OnPostInitialize
		for (const auto& plugin : Jupiter::plugins) {
			plugin->OnPostInitialize();
		}
	}
}

namespace Jupiter {
void reinitialize_plugins() {
	// Uninitialize back -> front
	while (!Jupiter::plugins.empty()) {
		Jupiter::Plugin::free(Jupiter::plugins.size() - 1);
	}

	initialize_plugins();
}
} // namespace Jupiter

[[noreturn]] void main_loop() {
	size_t index;
	while (1) {
		index = 0;
		while (index < Jupiter::plugins.size()) {
			if (Jupiter::plugins[index]->shouldRemove() || Jupiter::plugins[index]->think() != 0) {
				Jupiter::Plugin::free(index);
			}
			else {
				++index;
			}
		}
		Jupiter::Timer::check();

		if (console_input.input_mutex.try_lock()) {
			if (console_input.awaiting_processing) {
				console_input.awaiting_processing = false;
				auto input_split = jessilib::word_split_once_view(console_input.input, WHITESPACE_SV);
				std::string_view command_name = input_split.first;

				ConsoleCommand* command = getConsoleCommand(command_name);
				if (command != nullptr) {
					command->trigger(input_split.second);
				}
				else {
					std::cout << "Error: Command \"" << command_name << "\" not found." << std::endl;
				}
			}
			console_input.input_mutex.unlock();
		}
		std::this_thread::sleep_for((std::chrono::milliseconds(1)));
	}
}

int main(int argc, char* argv[]) {
	atexit(onExit);
	std::set_terminate(onTerminate);
	std::thread inputThread(inputLoop);

#if defined SIGPIPE
	std::signal(SIGPIPE, SIG_IGN);
#endif // SIGPIPE

#if defined _WIN32
	// Sets console to UTF-8
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
#endif // _WIN32

	srand(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
	std::cout << Jupiter::copyright << std::endl;

	jessilib::app_parameters parameters{ argc, argv };
	if (parameters.has_switch(u8"help"sv)) {
		std::cout << "Help coming soon, to a theatre near you!" << std::endl;
		return 0;
	}

	if (parameters.has_switch(u8"echo-parameters")) {
		// TODO: Write pretty JSON serializer based on JSON serializer
		// TODO: add a format specifier parameter
		std::cout << "Echoing app_parameters: " << std::endl
			<< jessilib::ustring_to_mbstring(jessilib::serialize_object(parameters, "json")).second << std::endl
			<< std::endl; // leave an extra empty line so it's easier to read or copy/paste
	}

	std::string_view configFileName = jessilib::string_view_cast<char>(parameters.get_value(u8"config", u8"Config.ini"sv));
	std::string_view plugins_directory = jessilib::string_view_cast<char>(parameters.get_value(u8"pluginsdir"sv));
	std::string_view configs_directory = jessilib::string_view_cast<char>(parameters.get_value(u8"configsdir"sv));

	std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();

	std::cout << "Loading config file..." << std::endl;
	if (!o_config.read(configFileName)) {
		std::cout << "Unable to read config file. Closing..." << std::endl;
		return 1;
	}

	double time_taken = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - load_start).count()) / 1000.0;

	std::cout << "Config loaded (" << time_taken << "ms)." << std::endl;

	if (plugins_directory.empty())
		plugins_directory = o_config.get("PluginsDirectory"sv);

	if (configs_directory.empty())
		configs_directory = o_config.get("ConfigsDirectory"sv);

	if (!plugins_directory.empty()) {
		Jupiter::Plugin::setDirectory(plugins_directory);
		std::cout << "Plugins will be loaded from \"" << plugins_directory << "\"." << std::endl;
	}

	if (!configs_directory.empty()) {
		Jupiter::Plugin::setConfigDirectory(configs_directory);
		std::cout << "Plugin configs will be loaded from \"" << configs_directory << "\"." << std::endl;
	}

	initialize_plugins();

	time_taken = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - Jupiter::g_start_time).count()) / 1000.0;
	std::cout << "Initialization completed in " << time_taken << " milliseconds." << std::endl;

	if (!consoleCommands.empty()) {
		std::cout << consoleCommands.size() << " Console Commands have been initialized"
			<< (getConsoleCommand("help"sv) == nullptr ? "." : "; type \"help\" for more information.")
			<< std::endl;
	}

	if (!IRCMasterCommandList.empty()) {
		std::cout << IRCMasterCommandList.size() << " IRC Commands have been loaded into the master list." << std::endl;
	}

	if (parameters.has_switch(u8"exit")) {
		std::cout << "exit switch specified; closing down post-initialization before entering main_loop" << std::endl;
		return 0;
	}

	main_loop();
	return 0;
}