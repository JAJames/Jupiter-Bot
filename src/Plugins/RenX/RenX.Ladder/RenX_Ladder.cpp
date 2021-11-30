/**
 * Copyright (C) 2015-2021 Jessica James.
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

#include <cinttypes>
#include "jessilib/unicode.hpp"
#include "Console_Command.h"
#include "RenX_Ladder.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"

using namespace Jupiter::literals;
using namespace std::literals;

bool RenX_LadderPlugin::initialize() {
	RenX_LadderPlugin::only_pure = this->config.get<bool>("OnlyPure"_jrs, false);
	int mlcpno = this->config.get<int>("MaxLadderCommandPartNameOutput"_jrs, 5);
	if (mlcpno < 0)
		RenX_LadderPlugin::max_ladder_command_part_name_output = 0;
	else
		RenX_LadderPlugin::max_ladder_command_part_name_output = mlcpno;

	RenX::Server *server;
	for (size_t index = 0; index != RenX::getCore()->getServerCount(); ++index)
	{
		server = RenX::getCore()->getServer(index);
		if (this->only_pure == false || server->isPure())
			server->setRanked(true);
	}

	return true;
}

void RenX_LadderPlugin::RenX_OnServerFullyConnected(RenX::Server &server) {
	if (this->only_pure == false || server.isPure()) {
		server.setRanked(true);
	}
}

/** Wait until the client list has been updated to update the ladder */

void RenX_LadderPlugin::RenX_OnGameOver(RenX::Server &server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore) {
	if (server.isRanked() && server.isReliable() && server.players.size() != server.getBotCount()) {
		char chr = static_cast<char>(team);
		server.varData[this->name].set("t"sv, std::string(&chr, 1));
		server.varData[this->name].set("w"sv, "1"s);
		server.updateClientList();
	}
}

void RenX_LadderPlugin::RenX_OnCommand(RenX::Server &server, const Jupiter::ReadableString &) {
	if (jessilib::equalsi(server.getCurrentRCONCommand(), "clientvarlist"sv)) {
		if (server.varData[this->name].get("w"_jrs, "0"_jrs) == "1"sv) {
			server.varData[this->name].set("w"sv, "0"s);
			RenX::TeamType team = static_cast<RenX::TeamType>(server.varData[this->name].get("t"_jrs, "\0"_jrs)[0]);
			for (const auto& database : RenX::ladder_databases) {
				database->updateLadder(server, team);
			}
		}
	}
}

size_t RenX_LadderPlugin::getMaxLadderCommandPartNameOutput() const {
	return RenX_LadderPlugin::max_ladder_command_part_name_output;
}

// Plugin instantiation and entry point.
RenX_LadderPlugin pluginInstance;

/** Ladder Commands */

Jupiter::StringS FormatLadderResponse(RenX::LadderDatabase::Entry *entry, size_t rank) {
	return Jupiter::StringS::Format("#%" PRIuPTR ": \"%.*s\" - Score: %" PRIu64 " - Kills: %" PRIu32 " - Deaths: %" PRIu32 " - KDR: %.2f - SPM: %.2f",
		rank, entry->most_recent_name.size(), entry->most_recent_name.data(), entry->total_score, entry->total_kills, entry->total_deaths,
		static_cast<double>(entry->total_kills) / (entry->total_deaths == 0 ? 1 : static_cast<double>(entry->total_deaths)),
		static_cast<double>(entry->total_score) / (entry->total_game_time == 0 ? 1.0 : static_cast<double>(entry->total_game_time)) * 60.0);
}

// Ladder Command

LadderGenericCommand::LadderGenericCommand() {
	this->addTrigger("ladder"_jrs);
	this->addTrigger("rank"_jrs);
}

Jupiter::GenericCommand::ResponseLine *LadderGenericCommand::trigger(const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		return new Jupiter::GenericCommand::ResponseLine("Error: Too few parameters. Syntax: ladder <name | rank>"_jrs, GenericCommand::DisplayType::PrivateError);
	}

	if (RenX::default_ladder_database == nullptr) {
		return new Jupiter::GenericCommand::ResponseLine("Error: No default ladder database specified."_jrs, GenericCommand::DisplayType::PrivateError);
	}

	RenX::LadderDatabase::Entry *entry;
	size_t rank;
	std::string_view parameters_view = parameters;
	if (parameters_view.find_first_not_of("0123456789"sv) == std::string_view::npos) {
		rank = Jupiter::asUnsignedInt(parameters_view, 10);
		if (rank == 0)
			return new Jupiter::GenericCommand::ResponseLine("Error: Invalid parameters"_jrs, GenericCommand::DisplayType::PrivateError);

		entry = RenX::default_ladder_database->getPlayerEntryByIndex(rank - 1);
		if (entry == nullptr)
			return new Jupiter::GenericCommand::ResponseLine("Error: Player not found"_jrs, GenericCommand::DisplayType::PrivateError);

		return new Jupiter::GenericCommand::ResponseLine(FormatLadderResponse(entry, rank), GenericCommand::DisplayType::PublicSuccess);
	}
	
	std::forward_list<std::pair<RenX::LadderDatabase::Entry, size_t>> list = RenX::default_ladder_database->getPlayerEntriesAndIndexByPartName(parameters, pluginInstance.getMaxLadderCommandPartNameOutput());
	if (list.empty())
		return new Jupiter::GenericCommand::ResponseLine("Error: Player not found"_jrs, GenericCommand::DisplayType::PrivateError);

	std::pair<RenX::LadderDatabase::Entry, size_t> &head_pair = list.front();
	Jupiter::GenericCommand::ResponseLine *response_head = new Jupiter::GenericCommand::ResponseLine(FormatLadderResponse(std::addressof(head_pair.first), head_pair.second + 1), GenericCommand::DisplayType::PrivateSuccess);
	Jupiter::GenericCommand::ResponseLine *response_end = response_head;
	list.pop_front();

	while (list.empty() == false) {
		std::pair<RenX::LadderDatabase::Entry, size_t> &pair = list.front();
		response_end->next = new Jupiter::GenericCommand::ResponseLine(FormatLadderResponse(std::addressof(pair.first), pair.second + 1), GenericCommand::DisplayType::PrivateSuccess);
		response_end = response_end->next;
		list.pop_front();
	}
	return response_head;
}

const Jupiter::ReadableString &LadderGenericCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Fetches ladder information about a player. Syntax: ladder <name | rank>");
	return defaultHelp;
}

GENERIC_COMMAND_INIT(LadderGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(LadderGenericCommand)

// Ladder Game Command

void LadderGameCommand::create() {
	this->addTrigger("ladder"_jrs);
	this->addTrigger("rank"_jrs);
}

void LadderGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		if (player->steamid != 0) {
			if (RenX::default_ladder_database != nullptr) {
				std::pair<RenX::LadderDatabase::Entry *, size_t> pair = RenX::default_ladder_database->getPlayerEntryAndIndex(player->steamid);
				if (pair.first != nullptr) {
					source->sendMessage(FormatLadderResponse(pair.first, pair.second + 1));
				}
				else
					source->sendMessage(*player, "Error: You have no ladder data. Get started by sticking around until the end of the match!"_jrs);
			}
			else
				source->sendMessage(*player, "Error: No default ladder database specified."_jrs);
		}
		else
			source->sendMessage(*player, "Error: You have no ladder data, because you're not using Steam."_jrs);
	}
	else {
		Jupiter::GenericCommand::ResponseLine *response = LadderGenericCommand_instance.trigger(parameters);
		Jupiter::GenericCommand::ResponseLine *ptr;
		while (response != nullptr) {
			source->sendMessage(*player, response->response);
			ptr = response;
			response = response->next;
			delete ptr;
		}
	}
}

const Jupiter::ReadableString &LadderGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays ladder information about yourself, or another player. Syntax: ladder [name / rank]");
	return defaultHelp;
}

GAME_COMMAND_INIT(LadderGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
