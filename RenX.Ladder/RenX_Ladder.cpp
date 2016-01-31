/**
 * Copyright (C) 2015-2016 Jessica James.
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
#include "Jupiter/INIFile.h"
#include "Console_Command.h"
#include "RenX_Ladder.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BanDatabase.h"

using namespace Jupiter::literals;

RenX_LadderPlugin::RenX_LadderPlugin()
{
	RenX_LadderPlugin::only_pure = Jupiter::IRC::Client::Config->getBool(this->getName(), "OnlyPure"_jrs, false);
	RenX_LadderPlugin::output_times = Jupiter::IRC::Client::Config->getBool(this->getName(), "OutputTimes"_jrs, true);
	int mlcpno = Jupiter::IRC::Client::Config->getInt(this->getName(), "MaxLadderCommandPartNameOutput"_jrs, 5);
	if (mlcpno < 0)
		RenX_LadderPlugin::max_ladder_command_part_name_output = 0;
	else
		RenX_LadderPlugin::max_ladder_command_part_name_output = mlcpno;
	RenX_LadderPlugin::db_filename = Jupiter::IRC::Client::Config->get(this->getName(), "LadderDatabase"_jrs, "Ladder.db"_jrs);

	// load database
	RenX_LadderPlugin::database.process_file(RenX_LadderPlugin::db_filename);
}

/** Wait until the client list has been updated to update the ladder */

void RenX_LadderPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore)
{
	if (server->hasSeenStart() && server->players.size() != server->getBotCount()) // the first game doesn't count!
	{
		char chr = static_cast<char>(team);
		server->varData.set(this->name, "t"_jrs, Jupiter::ReferenceString(chr));
		server->varData.set(this->name, "w"_jrs, "1"_jrs);
		server->updateClientList();
	}
}

void RenX_LadderPlugin::RenX_OnCommand(RenX::Server *server, const Jupiter::ReadableString &)
{
	if (server->getCurrentRCONCommand().equalsi("clientvarlist"_jrs))
	{
		if (server->varData.get(this->name, "w"_jrs, "0"_jrs).equals("1"))
		{
			server->varData.set(this->name, "w"_jrs, "0"_jrs);
			RenX::TeamType team = static_cast<RenX::TeamType>(server->varData.get(this->name, "t"_jrs, "\0"_jrs).get(0));
			RenX_LadderPlugin::database.updateLadder(server, team, RenX_LadderPlugin::output_times);
		}
	}
}

size_t RenX_LadderPlugin::getMaxLadderCommandPartNameOutput() const
{
	return RenX_LadderPlugin::max_ladder_command_part_name_output;
}

// Plugin instantiation and entry point.
RenX_LadderPlugin pluginInstance;

/** Ladder Commands */

Jupiter::StringS FormatLadderResponse(RenX::LadderDatabase::Entry *entry, size_t rank)
{
	return Jupiter::StringS::Format("#%" PRIuPTR ": \"%.*s\" - Score: %" PRIu64 " - Kills: %" PRIu32 " - Deaths: %" PRIu32 " - KDR: %.2f - SPM: %.2f", rank, entry->most_recent_name.size(), entry->most_recent_name.ptr(), entry->total_score, entry->total_kills, entry->total_deaths, static_cast<double>(entry->total_kills) / (entry->total_deaths == 0 ? 1 : static_cast<double>(entry->total_deaths)), static_cast<double>(entry->total_score) / (entry->total_game_time == 0 ? 1.0 : static_cast<double>(entry->total_game_time)) * 60.0);
}

// Ladder Command

LadderGenericCommand::LadderGenericCommand()
{
	this->addTrigger("ladder"_jrs);
	this->addTrigger("rank"_jrs);
}

GenericCommand::ResponseLine *LadderGenericCommand::trigger(const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
		return new GenericCommand::ResponseLine("Error: Too few parameters. Syntax: ladder <name | rank>"_jrs, GenericCommand::DisplayType::PrivateError);

	RenX::LadderDatabase::Entry *entry;
	size_t rank;
	if (parameters.span("0123456789"_jrs) == parameters.size())
	{
		rank = parameters.asUnsignedInt(10);
		if (rank == 0)
			return new GenericCommand::ResponseLine("Error: Invalid parameters"_jrs, GenericCommand::DisplayType::PrivateError);

		entry = pluginInstance.database.getPlayerEntryByIndex(rank - 1);
		if (entry == nullptr)
			return new GenericCommand::ResponseLine("Error: Player not found"_jrs, GenericCommand::DisplayType::PrivateError);

		return new GenericCommand::ResponseLine(FormatLadderResponse(entry, rank), GenericCommand::DisplayType::PublicSuccess);
	}
	
	Jupiter::SLList<std::pair<RenX::LadderDatabase::Entry, size_t>> list = pluginInstance.database.getPlayerEntriesAndIndexByPartName(parameters, pluginInstance.getMaxLadderCommandPartNameOutput());
	if (list.size() == 0)
		return new GenericCommand::ResponseLine("Error: Player not found"_jrs, GenericCommand::DisplayType::PrivateError);

	std::pair<RenX::LadderDatabase::Entry, size_t> *pair = list.remove(0);
	GenericCommand::ResponseLine *response_head = new GenericCommand::ResponseLine(FormatLadderResponse(std::addressof(pair->first), pair->second + 1), GenericCommand::DisplayType::PrivateSuccess);
	GenericCommand::ResponseLine *response_end = response_head;
	delete pair;
	while (list.size() != 0)
	{
		pair = list.remove(0);
		response_end->next = new GenericCommand::ResponseLine(FormatLadderResponse(std::addressof(pair->first), pair->second + 1), GenericCommand::DisplayType::PrivateSuccess);
		response_end = response_end->next;
		delete pair;
	}
	return response_head;
}

const Jupiter::ReadableString &LadderGenericCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Fetches ladder information about a player. Syntax: ladder <name | rank>");
	return defaultHelp;
}

GENERIC_COMMAND_INIT(LadderGenericCommand)
GENERIC_COMMAND_AS_CONSOLE_COMMAND(LadderGenericCommand)
GENERIC_COMMAND_AS_IRC_COMMAND_NO_CREATE(LadderGenericCommand)

// Ladder Game Command

void LadderGameCommand::create()
{
	this->addTrigger("ladder"_jrs);
	this->addTrigger("rank"_jrs);
}

void LadderGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.isEmpty())
	{
		if (player->steamid != 0)
		{
			std::pair<RenX::LadderDatabase::Entry *, size_t> pair = pluginInstance.database.getPlayerEntryAndIndex(player->steamid);
			if (pair.first != nullptr)
				source->sendMessage(FormatLadderResponse(pair.first, pair.second + 1));
			else
				source->sendMessage(player, "Error: You have no ladder data. Get started by sticking around until the end of the match!"_jrs);
		}
		else
			source->sendMessage(player, "Error: You have no ladder data, because you're not using Steam."_jrs);
	}
	else
	{
		GenericCommand::ResponseLine *response = LadderGenericCommand_instance.trigger(parameters);
		GenericCommand::ResponseLine *ptr;
		while (response != nullptr)
		{
			source->sendMessage(player, response->response);
			ptr = response;
			response = response->next;
			delete ptr;
		}
	}
}

const Jupiter::ReadableString &LadderGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays ladder information about yourself, or another player. Syntax: ladder [name / rank]");
	return defaultHelp;
}

GAME_COMMAND_INIT(LadderGameCommand)

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
