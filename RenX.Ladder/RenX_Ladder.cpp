/**
 * Copyright (C) 2015 Jessica James.
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

#include "Console_Command.h"
#include "Jupiter/INIFile.h"
#include "RenX_Ladder.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"

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

void RenX_LadderPlugin::updateLadder(RenX::Server *server, const RenX::TeamType &team)
{
	if (server->players.size() != 0)
	{
		// update player stats in memory
		RenX::PlayerInfo *player;
		RenX_LadderDatabase::Entry *entry;
		for (Jupiter::DLList<RenX::PlayerInfo>::Node *node = server->players.getNode(0); node != nullptr; node = node->next)
		{
			player = node->data;
			if (player->steamid != 0)
			{
				entry = RenX_LadderPlugin::database.getPlayerEntry(player->steamid);
				if (entry == nullptr)
				{
					entry = new RenX_LadderDatabase::Entry();
					RenX_LadderPlugin::database.append(entry);
					entry->steam_id = player->steamid;
				}

				entry->total_score += static_cast<uint64_t>(player->score);

				entry->total_kills += player->kills;
				entry->total_deaths += player->deaths;
				entry->total_headshot_kills += player->headshots;
				entry->total_vehicle_kills += player->vehicleKills;
				entry->total_building_kills += player->buildingKills;
				entry->total_defence_kills += player->defenceKills;
				entry->total_captures += player->captures;
				entry->total_game_time += static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server->getGameTime(player)).count());
				++entry->total_games;
				switch (player->team)
				{
				case RenX::TeamType::GDI:
					++entry->total_gdi_games;
					if (player->team == team)
						++entry->total_wins, ++entry->total_gdi_wins;
					break;
				case RenX::TeamType::Nod:
					++entry->total_nod_games;
					if (player->team == team)
						++entry->total_wins, ++entry->total_nod_wins;
					break;
				default:
					if (player->team == team)
						++entry->total_wins;
					break;
				}
				entry->total_beacon_placements += player->beaconPlacements;
				entry->total_beacon_disarms += player->beaconDisarms;
				entry->total_proxy_placements += player->proxy_placements;
				entry->total_proxy_disarms += player->proxy_disarms;

				auto set_if_greater = [](uint32_t &src, const uint32_t &cmp)
				{
					if (cmp > src)
						src = cmp;
				};

				set_if_greater(entry->top_score, static_cast<uint32_t>(player->score));
				set_if_greater(entry->top_kills, player->kills);
				set_if_greater(entry->most_deaths, player->deaths);
				set_if_greater(entry->top_headshot_kills, player->headshots);
				set_if_greater(entry->top_vehicle_kills, player->vehicleKills);
				set_if_greater(entry->top_building_kills, player->buildingKills);
				set_if_greater(entry->top_defence_kills, player->defenceKills);
				set_if_greater(entry->top_captures, player->captures);
				set_if_greater(entry->top_game_time, static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(server->getGameTime(player)).count()));
				set_if_greater(entry->top_beacon_placements, player->beaconPlacements);
				set_if_greater(entry->top_beacon_disarms, player->beaconDisarms);
				set_if_greater(entry->top_proxy_placements, player->proxy_placements);
				set_if_greater(entry->top_proxy_disarms, player->proxy_disarms);

				entry->most_recent_ip = player->ip32;
				entry->last_game = time(nullptr);
				entry->most_recent_name = player->name;
			}
		}

		// sort new stats
		std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
		RenX_LadderPlugin::database.sort_entries();
		std::chrono::steady_clock::duration sort_duration = std::chrono::steady_clock::now() - start_time;

		// write new stats
		start_time = std::chrono::steady_clock::now();
		RenX_LadderPlugin::database.write(RenX_LadderPlugin::db_filename);
		std::chrono::steady_clock::duration write_duration = std::chrono::steady_clock::now() - start_time;

		if (RenX_LadderPlugin::output_times)
		{
			Jupiter::StringS str = Jupiter::StringS::Format("Ladder: %u entries sorted in %f seconds; Database written in %f seconds." ENDL,
				RenX_LadderPlugin::database.getEntries(),
				static_cast<double>(sort_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num))),
				static_cast<double>(write_duration.count()) * (static_cast<double>(std::chrono::steady_clock::duration::period::num) / static_cast<double>(std::chrono::steady_clock::duration::period::den) * static_cast<double>(std::chrono::seconds::duration::period::den / std::chrono::seconds::duration::period::num)));
			str.println(stdout);
			server->sendLogChan(str);
		}
	}
}

/** Wait until the client list has been updated to update the ladder */

void RenX_LadderPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore)
{
	if (server->hasSeenStart() && server->players.size() != 0) // the first game doesn't count!
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
			RenX_LadderPlugin::updateLadder(server, team);
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
#include <cinttypes>
Jupiter::StringS FormatLadderResponse(RenX_LadderDatabase::Entry *entry, size_t rank)
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

	RenX_LadderDatabase::Entry *entry;
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
	
	Jupiter::SLList<std::pair<RenX_LadderDatabase::Entry, size_t>> list = pluginInstance.database.getPlayerEntriesAndIndexByPartName(parameters, pluginInstance.getMaxLadderCommandPartNameOutput());
	if (list.size() == 0)
		return new GenericCommand::ResponseLine("Error: Player not found"_jrs, GenericCommand::DisplayType::PrivateError);

	std::pair<RenX_LadderDatabase::Entry, size_t> *pair = list.remove(0);
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
			std::pair<RenX_LadderDatabase::Entry *, size_t> pair = pluginInstance.database.getPlayerEntryAndIndex(player->steamid);
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
