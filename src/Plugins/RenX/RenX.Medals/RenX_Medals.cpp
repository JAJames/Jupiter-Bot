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

#include <ctime>
#include "jessilib/word_split.hpp"
#include "jessilib/unicode.hpp"
#include "Jupiter/Timer.h"
#include "Jupiter/IRC_Client.h"
#include "RenX_Medals.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_Core.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;
using namespace std::literals;

bool RenX_MedalsPlugin::initialize()
{
	this->INTERNAL_RECS_TAG = RenX::getUniqueInternalTag();
	this->INTERNAL_NOOB_TAG = RenX::getUniqueInternalTag();
	this->INTERNAL_WORTH_TAG = RenX::getUniqueInternalTag();
	init();

	return true;
}

RenX_MedalsPlugin::~RenX_MedalsPlugin()
{
	RenX::Core *core = RenX::getCore();
	size_t sCount = core->getServerCount();
	RenX::Server *server;

	for (size_t i = 0; i < sCount; i++) {
		server = core->getServer(i);
		if (server->players.size() != 0) {
			for (auto node = server->players.begin(); node != server->players.end(); ++node) {
				if (!node->uuid.empty() && !node->isBot) {
					RenX_MedalsPlugin::medalsFile[node->uuid].set("Recs"_jrs, node->varData[this->getName()].get("Recs"sv, ""s));
					RenX_MedalsPlugin::medalsFile[node->uuid].set("Noobs"_jrs, node->varData[this->getName()].get("Noobs"sv, ""s));
				}
			}
		}
	}

	RenX_MedalsPlugin::medalsFile.write(RenX_MedalsPlugin::medalsFileName);
}

struct CongratPlayerData
{
	RenX::Server *server;
	Jupiter::StringS playerName;
	unsigned int type;
};

void congratPlayer(unsigned int, void *params)
{
	CongratPlayerData *congratPlayerData = reinterpret_cast<CongratPlayerData *>(params);

	if (RenX::getCore()->hasServer(congratPlayerData->server) && congratPlayerData->server->isConnected())
	{
		switch (congratPlayerData->type)
		{
		case 0:
			congratPlayerData->server->sendMessage(congratPlayerData->playerName + " has been recommended for having the highest score last game!"_jrs);
			break;
		case 1:
			congratPlayerData->server->sendMessage(congratPlayerData->playerName + " has been recommended for having the most kills last game!"_jrs);
			break;
		case 2:
			congratPlayerData->server->sendMessage(congratPlayerData->playerName + " has been recommended for having the most vehicle kills last game!"_jrs);
			break;
		case 3:
			congratPlayerData->server->sendMessage(congratPlayerData->playerName + " has been recommended for having the highest Kill-Death ratio last game!"_jrs);
			break;
		default:
			break;
		}
	}
	delete congratPlayerData;
}

void RenX_MedalsPlugin::RenX_SanitizeTags(std::string& fmt) {
	RenX::replace_tag(fmt, RenX_MedalsPlugin::recsTag, this->INTERNAL_RECS_TAG);
	RenX::replace_tag(fmt, RenX_MedalsPlugin::noobTag, this->INTERNAL_NOOB_TAG);
	RenX::replace_tag(fmt, RenX_MedalsPlugin::worthTag, this->INTERNAL_WORTH_TAG);
}

void RenX_MedalsPlugin::RenX_ProcessTags(std::string& msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *, const RenX::BuildingInfo *) {
	if (player != nullptr) {
		std::string_view recs = RenX_MedalsPlugin::medalsFile.get(player->uuid, "Recs"_jrs);
		std::string_view noobs = RenX_MedalsPlugin::medalsFile.get(player->uuid, "Noobs"_jrs);

		RenX::replace_tag(msg, this->INTERNAL_RECS_TAG, recs);
		RenX::replace_tag(msg, this->INTERNAL_NOOB_TAG, noobs);
		RenX::replace_tag(msg, this->INTERNAL_WORTH_TAG, string_printf("%d", Jupiter::from_string<int>(recs) - Jupiter::from_string<int>(noobs)));
	}
}

void RenX_MedalsPlugin::RenX_OnPlayerCreate(RenX::Server &, const RenX::PlayerInfo &player) {
	if (!player.uuid.empty() && player.isBot == false) {
		player.varData[this->getName()].set("Recs"sv, RenX_MedalsPlugin::medalsFile.get(player.uuid, "Recs"s));
		player.varData[this->getName()].set("Noobs"sv, RenX_MedalsPlugin::medalsFile.get(player.uuid, "Noobs"s));
	}
}

void RenX_MedalsPlugin::RenX_OnPlayerDelete(RenX::Server &, const RenX::PlayerInfo &player) {
	if (!player.uuid.empty() && player.isBot == false) {
		RenX_MedalsPlugin::medalsFile[player.uuid].set("Recs"_jrs, player.varData[this->getName()].get("Recs"sv, ""s));
		RenX_MedalsPlugin::medalsFile[player.uuid].set("Noobs"_jrs, player.varData[this->getName()].get("Noobs"sv, ""s));
	}
}

void RenX_MedalsPlugin::RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (!player.uuid.empty() && player.isBot == false && server.isMatchInProgress()) {
		int worth = getWorth(player);
		Jupiter::Config *section = RenX_MedalsPlugin::config.getSection(RenX_MedalsPlugin::firstSection);
		if (section != nullptr) {
			while (section->get<int>("MaxRecs"_jrs, INT_MAX) < worth)
				if ((section = RenX_MedalsPlugin::config.getSection(section->get("NextSection"_jrs))) == nullptr)
					return; // No matching section found.

			size_t table_size = section->getTable().size();

			if (table_size != 0) {
				std::string_view msg = section->get(string_printf("%u", (rand() % table_size) + 1));

				if (!msg.empty()) {
					std::string tagged_msg = static_cast<std::string>(msg);
					RenX::sanitizeTags(tagged_msg);
					RenX::processTags(tagged_msg, &server, &player);
					server.sendMessage(tagged_msg);
				}
			}
		}
	}
}

void RenX_MedalsPlugin::RenX_OnGameOver(RenX::Server &server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore)
{
	if (server.isReliable() && server.players.size() != server.getBotCount())
	{
		RenX::PlayerInfo *bestScore = &server.players.front();
		RenX::PlayerInfo *mostKills = &server.players.front();
		RenX::PlayerInfo *mostVehicleKills = &server.players.front();
		RenX::PlayerInfo *bestKD = &server.players.front();

		for (auto node = server.players.begin(); node != server.players.end(); ++node)
		{
			if (node->score > bestScore->score)
				bestScore = &*node;

			if (node->kills > mostKills->kills)
				mostKills = &*node;

			if (node->vehicleKills > mostVehicleKills->vehicleKills)
				mostVehicleKills = &*node;

			if (RenX::getKillDeathRatio(*node) > RenX::getKillDeathRatio(*bestKD))
				bestKD = &*node;
		}

		CongratPlayerData *congratPlayerData;

		/** +1 for best score */
		if (!bestScore->uuid.empty() && bestScore->isBot == false && bestScore->score > 0)
		{
			addRec(*bestScore);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = &server;
			congratPlayerData->playerName = bestScore->name;
			congratPlayerData->type = 0;
			new Jupiter::Timer(1, killCongratDelay, congratPlayer, congratPlayerData, false);
		}

		/** +1 for most kills */
		if (!mostKills->uuid.empty() && mostKills->isBot == false && mostKills->kills > 0)
		{
			addRec(*mostKills);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = &server;
			congratPlayerData->playerName = mostKills->name;
			congratPlayerData->type = 1;
			new Jupiter::Timer(1, killCongratDelay, congratPlayer, congratPlayerData, false);
		}

		/** +1 for most Vehicle kills */
		if (!mostVehicleKills->uuid.empty() && mostVehicleKills->isBot == false && mostVehicleKills->vehicleKills > 0)
		{
			addRec(*mostVehicleKills);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = &server;
			congratPlayerData->playerName = mostVehicleKills->name;
			congratPlayerData->type = 2;
			new Jupiter::Timer(1, vehicleKillCongratDelay, congratPlayer, congratPlayerData, false);
		}

		/** +1 for best K/D ratio */
		if (!bestKD->uuid.empty() && bestKD->isBot == false && RenX::getKillDeathRatio(*bestKD) > 1.0)
		{
			addRec(*bestKD);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = &server;
			congratPlayerData->playerName = bestKD->name;
			congratPlayerData->type = 3;
			new Jupiter::Timer(1, kdrCongratDelay, congratPlayer, congratPlayerData, false);
		}
	}

	RenX_MedalsPlugin::medalsFile.write(medalsFileName);
}

void RenX_MedalsPlugin::RenX_OnDestroy(RenX::Server &server, const RenX::PlayerInfo &player, std::string_view objectName, const RenX::TeamType &objectTeam, std::string_view damageType, RenX::ObjectType type)
{
	if (type == RenX::ObjectType::Building)
	{
		addRec(player);

		std::string_view translated = RenX::translateName(objectName);
		server.sendMessage(string_printf("%.*s has been recommended for destroying the %.*s!", player.name.size(), player.name.data(), translated.size(), translated.data()));
	}
}

int RenX_MedalsPlugin::OnRehash()
{
	RenX::Plugin::OnRehash();

	RenX_MedalsPlugin::medalsFile.write(RenX_MedalsPlugin::medalsFileName);
	RenX_MedalsPlugin::medalsFile.erase();
	init();
	return 0;
}

void RenX_MedalsPlugin::init()
{
	RenX_MedalsPlugin::killCongratDelay = std::chrono::seconds(this->config.get<long long>("KillCongratDelay"_jrs, 60));
	RenX_MedalsPlugin::vehicleKillCongratDelay = std::chrono::seconds(this->config.get<long long>("VehicleKillCongratDelay"_jrs, 60));
	RenX_MedalsPlugin::kdrCongratDelay = std::chrono::seconds(this->config.get<long long>("KDRCongratDelay"_jrs, 60));
	RenX_MedalsPlugin::medalsFileName = this->config.get("MedalsFile"_jrs, "Medals.ini"_jrs);
	RenX_MedalsPlugin::medalsFile.read(RenX_MedalsPlugin::medalsFileName);
	RenX_MedalsPlugin::firstSection = RenX_MedalsPlugin::config.get("FirstSection"_jrs);
	RenX_MedalsPlugin::recsTag = RenX_MedalsPlugin::config.get("RecsTag"_jrs, "{RECS}"_jrs);
	RenX_MedalsPlugin::noobTag = RenX_MedalsPlugin::config.get("NoobsTag"_jrs, "{NOOBS}"_jrs);
	RenX_MedalsPlugin::worthTag = RenX_MedalsPlugin::config.get("WorthTag"_jrs, "{WORTH}"_jrs);

	RenX::Core *core = RenX::getCore();
	size_t server_count = core->getServerCount();
	RenX::Server *server;
	for (size_t index = 0; index < server_count; ++index) {
		server = core->getServer(index);
		if (server->players.size() != server->getBotCount()) {
			for (auto node = server->players.begin(); node != server->players.end(); ++node) {
				node->varData[this->getName()].set("Recs"sv, RenX_MedalsPlugin::medalsFile[node->name].get("Recs"sv, ""s));
				node->varData[this->getName()].set("Noobs"sv, RenX_MedalsPlugin::medalsFile[node->name].get("Noobs"sv, ""s));
			}
		}
	}
}

/** Game Commands */

/** Instance of RenX_MedalsPlugin */
RenX_MedalsPlugin pluginInstance;

// Recommendations Game Command

void RecsGameCommand::create()
{
	this->addTrigger("recs"_jrs);
	this->addTrigger("recommends"_jrs);
	this->addTrigger("recommendations"_jrs);
	this->addTrigger("noobs"_jrs);
	this->addTrigger("n00bs"_jrs);
}

void RecsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters)
{
	if (!parameters.empty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
		{
			Jupiter::Config *section = pluginInstance.medalsFile.getSection(parameters);
			if (section == nullptr)
				source->sendMessage(*player, "Error: Player not found! Syntax: recs [player]"_jrs);
			else
			{
				unsigned int recs = section->get<unsigned int>("Recs"_jrs);
				unsigned int noobs = section->get<unsigned int>("Noobs"_jrs);
				source->sendMessage(*player, string_printf("[Archive] %.*s has %u and %u n00bs. Their worth: %d", section->getName().size(), section->getName().c_str(), recs, noobs, recs - noobs));
			}
		}
		else if (target->uuid.empty())
			source->sendMessage(*player, "Error: Player is not using steam."_jrs);
		else if (target->isBot)
			source->sendMessage(*player, "Error: Bots do not have any recommendations."_jrs);
		else if (target == player)
			RecsGameCommand::trigger(source, player, ""_jrs);
		else
			source->sendMessage(*player, string_printf("%.*s has %lu and %lu n00bs. Their worth: %d", target->name.size(), target->name.data(), getRecs(*target), getNoobs(*target), getWorth(*target)));
	}
	else if (player->uuid.empty())
		source->sendMessage(*player, "Error: You are not using steam."_jrs);
	else
		source->sendMessage(*player, string_printf("%.*s, you have %lu recs and %lu n00bs. Your worth: %d", player->name.size(), player->name.data(), getRecs(*player), getNoobs(*player), getWorth(*player)));
}

std::string_view RecsGameCommand::getHelp(std::string_view )
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets a count of a player's recommendations and noobs. Syntax: recs [player]");
	return defaultHelp;
}

GAME_COMMAND_INIT(RecsGameCommand)

// Recommend Game Command

void RecGameCommand::create()
{
	this->addTrigger("rec"_jrs);
	this->addTrigger("recommend"_jrs);
}

void RecGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!parameters_split.first.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr) {
			target = source->getPlayerByPartName(parameters_split.first);
		}
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found! Syntax: rec <player>"_jrs);
		}
		else if (target->uuid.empty()) {
			source->sendMessage(*player, "Error: Player is not using steam."_jrs);
		}
		else if (target->isBot) {
			source->sendMessage(*player, "Error: Bots can not receive recommendations."_jrs);
		}
		else if (target == player) {
			addNoob(*player);
			source->sendMessage(*player, "You can't recommend yourself, you noob! (+1 noob)"_jrs);
		}
		else if (!player->varData["RenX.Medals"_jrs].get("gr"_jrs).empty() && player->adminType.empty()) {
			source->sendMessage(*player, "You can only give one recommendation per game."_jrs);
		}
		else {
			addRec(*target);
			source->sendMessage(string_printf("%.*s has recommended %.*s!", player->name.size(), player->name.data(), target->name.size(), target->name.data()));
			player->varData["RenX.Medals"_jrs].set("gr"_jrs, "1"s);
		}
	}
	else RecsGameCommand_instance.trigger(source, player, parameters);
}

std::string_view RecGameCommand::getHelp(std::string_view )
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Recommends a player for their gameplay. Syntax: rec <player> [reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(RecGameCommand)

// Noob Game Command

void NoobGameCommand::create()
{
	this->addTrigger("noob"_jrs);
	this->addTrigger("n00b"_jrs);
}

void NoobGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!parameters_split.first.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr) {
			target = source->getPlayerByPartName(parameters_split.first);
		}
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found! Syntax: noob [player]"_jrs);
		}
		else if (target->uuid.empty()) {
			source->sendMessage(*player, "Error: Player is not using steam."_jrs);
		}
		else if (target->isBot) {
			source->sendMessage(*player, "Error: Bots can not receive n00bs."_jrs);
		}
		else if (!player->varData["RenX.Medals"_jrs].get("gn"_jrs).empty() && player->adminType.empty()) {
			source->sendMessage(*player, "You can only give one noob per game."_jrs);
		}
		else {
			addNoob(*target);
			source->sendMessage(string_printf("%.*s has noob'd %.*s!", player->name.size(), player->name.data(), target->name.size(), target->name.data()));
			player->varData["RenX.Medals"_jrs].set("gn"_jrs, "1"s);
		}
	}
	else RecsGameCommand_instance.trigger(source, player, parameters);
}

std::string_view NoobGameCommand::getHelp(std::string_view )
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Tells people that a player is bad. Syntax: noob [player]");
	return defaultHelp;
}

GAME_COMMAND_INIT(NoobGameCommand)

void addRec(const RenX::PlayerInfo &player, int amount) {
	if (!jessilib::starts_withi(player.uuid, "Player"sv) && !player.isBot) {
		player.varData[pluginInstance.getName()].set("Recs"_jrs, static_cast<std::string>(string_printf("%u", getRecs(player) + amount)));
	}
}

void addNoob(const RenX::PlayerInfo &player, int amount) {
	if (!jessilib::starts_withi(player.uuid, "Player"sv) && !player.isBot) {
		player.varData[pluginInstance.getName()].set("Noobs"_jrs,static_cast<std::string>(string_printf("%u", getNoobs(player) + amount)));
	}
}

unsigned long getRecs(const RenX::PlayerInfo &player)
{
	return player.varData[pluginInstance.getName()].get<unsigned long>("Recs"_jrs);
}

unsigned long getNoobs(const RenX::PlayerInfo &player)
{
	return player.varData[pluginInstance.getName()].get<unsigned long>("Noobs"_jrs);
}

int getWorth(const RenX::PlayerInfo &player)
{
	return getRecs(player) - getNoobs(player);
}

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
