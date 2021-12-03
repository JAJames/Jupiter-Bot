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

#include <forward_list>
#include <functional>
#include <sstream>
#include "Jupiter/Functions.h"
#include "jessilib/duration.hpp"
#include "jessilib/unicode.hpp"
#include "jessilib/word_split.hpp"
#include "IRC_Bot.h"
#include "RenX_Commands.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BuildingInfo.h"
#include "RenX_Functions.h"
#include "RenX_BanDatabase.h"
#include "RenX_ExemptionDatabase.h"
#include "RenX_Tags.h"

using namespace jessilib::literals;
using namespace std::literals;

constexpr std::string_view RxCommandsSection = "RenX.Commands"sv;

bool togglePhasing(RenX::Server *server, bool newState) {
	server->varData[RxCommandsSection].set("phasing"sv, newState ? "true"s : "false"s);
	return newState;
}

bool togglePhasing(RenX::Server *server) {
	return togglePhasing(server, !server->varData[RxCommandsSection].get<bool>("phasing"sv, false));
}

void onDie(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (player.isBot && server.varData[RxCommandsSection].get<bool>("phasing"sv, false)) {
		server.kickPlayer(player, ""sv);
	}
}

std::string player_not_found_message(std::string_view name) {
	std::string result = "Error: Player \""s;
	result += name;
	result += "\" not found.";
	return result;
}

void RenX_CommandsPlugin::RenX_OnSuicide(RenX::Server &server, const RenX::PlayerInfo &player, std::string_view ) {
	onDie(server, player);
}

void RenX_CommandsPlugin::RenX_OnKill(RenX::Server &server, const RenX::PlayerInfo &, const RenX::PlayerInfo &victim, std::string_view ) {
	onDie(server, victim);
}

void RenX_CommandsPlugin::RenX_OnDie(RenX::Server &server, const RenX::PlayerInfo &player, std::string_view ) {
	onDie(server, player);
}

bool RenX_CommandsPlugin::initialize() {
	auto default_tban_time = this->config.get("DefaultTBanTime"sv, "1d"sv);
	auto max_tban_time = this->config.get("MaxTBanTime"sv, "1w"sv);
	m_defaultTempBanTime = jessilib::duration_from_string(default_tban_time.data(), default_tban_time.data() + default_tban_time.size()).duration;
	m_maxTempBanTime = std::max(jessilib::duration_from_string(max_tban_time.data(), max_tban_time.data() + max_tban_time.size()).duration, m_defaultTempBanTime);
	m_playerInfoFormat = this->config.get("PlayerInfoFormat"sv, IRCCOLOR "03[Player Info]" IRCCOLOR "{TCOLOR} " IRCBOLD "{RNAME}" IRCBOLD " - ID: {ID} - Team: " IRCBOLD "{TEAML}" IRCBOLD " - Vehicle Kills: {VEHICLEKILLS} - Building Kills {BUILDINGKILLS} - Kills {KILLS} - Deaths: {DEATHS} - KDR: {KDR} - Access: {ACCESS}"sv);
	m_adminPlayerInfoFormat = this->config.get("AdminPlayerInfoFormat"sv, m_playerInfoFormat + " - IP: " IRCBOLD "{IP}" IRCBOLD " - HWID: " IRCBOLD "{HWID}" IRCBOLD " - RDNS: " IRCBOLD "{RDNS}" IRCBOLD " - Steam ID: " IRCBOLD "{STEAM}");
	m_buildingInfoFormat = this->config.get("BuildingInfoFormat"sv, jessilib::join<std::string>(""sv IRCCOLOR, RenX::tags->buildingTeamColorTag, RenX::tags->buildingNameTag, ""sv IRCCOLOR, " - "sv, IRCCOLOR "07"sv, RenX::tags->buildingHealthPercentageTag, "%"sv));
	m_staffTitle = this->config.get("StaffTitle"sv, "Moderator"sv);

	RenX::sanitizeTags(m_playerInfoFormat);
	RenX::sanitizeTags(m_adminPlayerInfoFormat);
	RenX::sanitizeTags(m_buildingInfoFormat);
	return true;
}

int RenX_CommandsPlugin::OnRehash() {
	RenX::Plugin::OnRehash();
	return this->initialize() ? 0 : -1;
}

std::chrono::seconds RenX_CommandsPlugin::getDefaultTBanTime() const {
	return m_defaultTempBanTime;
}

std::chrono::seconds RenX_CommandsPlugin::getMaxTBanTime() const {
	return m_maxTempBanTime;
}

std::string_view RenX_CommandsPlugin::getPlayerInfoFormat() const {
	return m_playerInfoFormat;
}

std::string_view RenX_CommandsPlugin::getAdminPlayerInfoFormat() const {
	return m_adminPlayerInfoFormat;
}

std::string_view RenX_CommandsPlugin::getBuildingInfoFormat() const {
	return m_buildingInfoFormat;
}

std::string_view RenX_CommandsPlugin::getStaffTitle() const {
	return m_staffTitle;
}

// Plugin instantiation and entry point.
RenX_CommandsPlugin pluginInstance;

/** Console Commands */

// RawRCON Console Command

RawRCONConsoleCommand::RawRCONConsoleCommand() {
	this->addTrigger("rrcon"sv);
	this->addTrigger("rawrcon"sv);
}

void RawRCONConsoleCommand::trigger(std::string_view parameters) {
	if (parameters.empty()) {
		puts("Error: Too Few Parameters. Syntax: rrcon <input>");
		return;
	}

	const auto& servers = RenX::getCore()->getServers();
	if (servers.empty()) {
		puts("Error: Not connected to any Renegade X servers.");
		return;
	}

	std::string msg{ parameters };
	msg += '\n';
	for (const auto& server : servers) {
		server->sendData(msg);
	}
}

std::string_view RawRCONConsoleCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Sends data over the Renegade X server's rcon connection. Syntax: rrcon <data>"sv;
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RawRCONConsoleCommand)

// RCON Console Command

RCONConsoleCommand::RCONConsoleCommand() {
	this->addTrigger("rcon"sv);
	this->addTrigger("renx"sv);
}

void RCONConsoleCommand::trigger(std::string_view parameters) {
	if (parameters.empty()) {
		puts("Error: Too Few Parameters. Syntax: rcon <input>");
	}

	const auto& servers = RenX::getCore()->getServers();
	if (servers.empty()) {
		puts("Error: Not connected to any Renegade X servers.");
		return;
	}

	for (const auto& server : servers) {
		server->send(parameters);
	}
}

std::string_view RCONConsoleCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Executes a command over the Renegade X server's rcon connection. Syntax: rcon <input>"sv;
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RCONConsoleCommand)

/** IRC Commands */

// Msg IRC Command

void MsgIRCCommand::create()
{
	this->addTrigger("msg"sv);
	this->addTrigger("say"sv);
}

void MsgIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		int type = source->getChannel(channel)->getType();
		std::string msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += parameters;

		prefix = '\0'; // Reusing prefix to check if a match is ever found.
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				server->sendMessage(msg);
				prefix = 1;
			}
		}
		if (prefix == '\0')
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: Msg <Message>"sv);
}

std::string_view MsgIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message in - game.Syntax: Msg <Message>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(MsgIRCCommand)

// PMsg IRC Command

void PMsgIRCCommand::create()
{
	this->addTrigger("pmsg"sv);
	this->addTrigger("psay"sv);
	this->addTrigger("page"sv);
	this->addTrigger("ppage"sv);
	this->setAccessLevel(1);
}

void PMsgIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!command_split.second.empty())
	{
		int type = source->getChannel(channel)->getType();

		std::string_view name = command_split.first;
		RenX::PlayerInfo *player;
		std::string msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += command_split.second;
		if (!parameters.empty())
		{
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					player = server->getPlayerByPartName(name);
					if (player != nullptr) {
						server->sendMessage(*player, msg);
					}
					else {
						source->sendNotice(nick, player_not_found_message(name));
					}
				}
			}
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: PMsg <Player> <Message>"sv);
}

std::string_view PMsgIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message in - game.Syntax: PMsg <Player> <Message>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PMsgIRCCommand)

// Host Msg IRC Command

void HostMsgIRCCommand::create()
{
	this->addTrigger("hmsg"sv);
	this->addTrigger("hsay"sv);
	this->addTrigger("hostmessage"sv);
	this->setAccessLevel(4);
}

void HostMsgIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		int type = source->getChannel(channel)->getType();

		bool success = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
				success = server->sendMessage(parameters) > 0;
		}
		if (!success)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: hmsg <Message>"sv);
}

std::string_view HostMsgIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message in-game. Syntax: hmsg <Message>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(HostMsgIRCCommand)

// Admin Msg IRC Command

void AdminMsgIRCCommand::create()
{
	this->addTrigger("amsg"sv);
	this->addTrigger("asay"sv);
	this->addTrigger("adminmessage"sv);
	this->setAccessLevel(4);
}

void AdminMsgIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		int type = source->getChannel(channel)->getType();

		bool success = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
				success = server->sendAdminMessage(parameters) > 0;
		}
		if (!success)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: amsg <Message>"sv);
}

std::string_view AdminMsgIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends an admin message in-game. Syntax: amsg <Message>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(AdminMsgIRCCommand)

// PAdminMsg IRC Command

void PAdminMsgIRCCommand::create()
{
	this->addTrigger("pamsg"sv);
	this->addTrigger("pasay"sv);
	this->addTrigger("apage"sv);
	this->setAccessLevel(4);
}

void PAdminMsgIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!command_split.second.empty()) {
		int type = source->getChannel(channel)->getType();
		std::string_view name = command_split.first;
		RenX::PlayerInfo *player;
		std::string msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += command_split.second;
		if (!parameters.empty())
		{
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					player = server->getPlayerByPartName(name);
					if (player != nullptr)
						server->sendAdminMessage(*player, msg);
					else source->sendNotice(nick, player_not_found_message(name));
				}
			}
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: pamsg <Player> <Message>"sv);
}

std::string_view PAdminMsgIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends an admin message to a player in-game. Syntax: pamsg <Player> <Message>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PAdminMsgIRCCommand)

// Players IRC Command

void PlayersIRCCommand::create()
{
	this->addTrigger("players"sv);
	this->addTrigger("pl"sv);
	this->addTrigger("playerlist"sv);
}

const size_t STRING_LENGTH = 240;

void PlayersIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view , std::string_view )
{
	int type = source->getChannel(channel)->getType();

	// Team colors
	std::string_view gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	std::string_view nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	std::string_view oTeamColor = RenX::getTeamColor(RenX::TeamType::Other);

	// Team names
	std::string_view gTeam = RenX::getTeamName(RenX::TeamType::GDI);
	std::string_view nTeam = RenX::getTeamName(RenX::TeamType::Nod);
	std::string_view oTeam = RenX::getTeamName(RenX::TeamType::Other);

	bool noServers = true;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		RenX::Server *server = RenX::getCore()->getServer(i);
		if (server->isLogChanType(type))
		{
			noServers = false;
			if (server->players.size() != server->getBotCount())
			{
				if (server->players.size() == 0)
				{
					source->sendMessage(channel, string_printf("ERROR: NO PLAYERS BUT BOT_COUNT = %u.", server->getBotCount()));
					continue;
				}

				// End string containers
				std::list<std::string> gStrings;
				std::list<std::string> nStrings;
				std::list<std::string> oStrings;

				// Team player counters
				unsigned int gTotal = 0;
				unsigned int nTotal = 0;
				unsigned int oTotal = 0;

				// Bot counters
				unsigned int gBots = 0;
				unsigned int nBots = 0;
				unsigned int oBots = 0;

				for (auto node = server->players.begin(); node != server->players.end(); ++node) {
					std::string name = RenX::getFormattedPlayerName(*node);
					if (name.size() > STRING_LENGTH - 32) continue; // Name will be too long to send.

					switch (node->team) {
					case RenX::TeamType::Nod:
						if (nStrings.empty() || nStrings.back().size() + name.size() > STRING_LENGTH) {
							nStrings.push_back(string_printf(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD,
								nTeamColor.size(), nTeamColor.data(),
								nTeam.size(), nTeam.data(),
								name.size(), name.data()));
						}
						else {
							nStrings.back() += string_printf(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.data());
						}

						++nTotal;
						if (node->isBot) {
							++nBots;
						}
						break;
					case RenX::TeamType::GDI:
						if (gStrings.empty() || gStrings.back().size() + name.size() > STRING_LENGTH)
						{
							gStrings.push_back(string_printf(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD,
								gTeamColor.size(), gTeamColor.data(),
								gTeam.size(), gTeam.data(),
								name.size(), name.data()));
						}
						else {
							gStrings.back() += string_printf(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.data());
						}

						++gTotal;
						if (node->isBot) {
							++gBots;
						}
						break;
					default:
						if (oStrings.empty() || oStrings.back().size() + name.size() > STRING_LENGTH) {
							oStrings.push_back(string_printf(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD,
								oTeamColor.size(), oTeamColor.data(),
								oTeam.size(), oTeam.data(),
								name.size(), name.data()));
						}
						else {
							oStrings.back() += string_printf(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.data());
						}

						++oTotal;
						if (node->isBot) {
							++oBots;
						}
						break;
					}
				}
				while (gStrings.size() != 0) {
					source->sendMessage(channel, gStrings.front());
					gStrings.pop_front();
				}
				while (nStrings.size() != 0) {
					source->sendMessage(channel, nStrings.front());
					nStrings.pop_front();
				}
				while (oStrings.size() != 0) {
					source->sendMessage(channel, oStrings.front());
					oStrings.pop_front();
				}

				std::string out = string_printf(IRCCOLOR "03Total Players" IRCCOLOR ": %u", server->players.size());
				if (gBots + nBots + oBots > 0) {
					out += string_printf(" (%u bots)", gBots + nBots + oBots);
				}
				if (gTotal > 0) {
					out += string_printf(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", gTeamColor.size(),
						gTeamColor.data(), gTeam.size(),
						gTeam.data(), gTotal);
					if (gBots > 0)
						out += string_printf(" (%u bots)", gBots);
				}
				if (nTotal > 0) {
					out += string_printf(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", nTeamColor.size(),
						nTeamColor.data(), nTeam.size(),
						nTeam.data(), nTotal);
					if (nBots > 0)
						out += string_printf(" (%u bots)", nBots);
				}
				if (oTotal > 0) {
					out += string_printf(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", oTeamColor.size(),
						oTeamColor.data(), oTeam.size(),
						oTeam.data(), oTotal);
					if (oBots > 0)
						out += string_printf(" (%u bots)", oBots);
				}
				source->sendMessage(channel, out);
			}
			else
				source->sendMessage(channel, "No players are in-game."sv);
		}
	}
	if (noServers)
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
}

std::string_view PlayersIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Lists the players currently in-game. Syntax: Players"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayersIRCCommand)

// PlayerTable IRC Command
void PlayerTableIRCCommand::create()
{
	this->addTrigger("pt"sv);
	this->addTrigger("playertable"sv);
}

void PlayerTableIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view )
{
	int type = source->getChannel(channel)->getType();

	// Team colors
	std::string_view gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	std::string_view nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	std::string_view oTeamColor = RenX::getTeamColor(RenX::TeamType::Other);

	// Team names
	std::string_view gTeam = RenX::getTeamName(RenX::TeamType::GDI);
	std::string_view nTeam = RenX::getTeamName(RenX::TeamType::Nod);
	std::string_view oTeam = RenX::getTeamName(RenX::TeamType::Other);

	bool noServers = true;
	for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
	{
		RenX::Server *server = RenX::getCore()->getServer(i);
		if (server->isLogChanType(type))
		{
			noServers = false;
			if (server->players.size() != server->getBotCount())
			{
				std::forward_list<RenX::PlayerInfo *> gPlayers;
				std::forward_list<RenX::PlayerInfo *> nPlayers;
				std::forward_list<RenX::PlayerInfo *> oPlayers;

				size_t maxNickLen = 8;
				int highID = 999;
				double highScore = 99999.0;
				double highCredits = 9999999.0;

				RenX::PlayerInfo *player;
				for (auto node = server->players.begin(); node != server->players.end(); ++node)
				{
					player = &*node;
					if (player != nullptr && player->isBot == false)
					{
						if (player->name.size() > maxNickLen)
							maxNickLen = player->name.size();

						if (player->id > highID)
							highID = player->id;

						if (player->score > highScore)
							highScore = player->score;

						if (player->credits > highCredits)
							highCredits = player->credits;

						switch (player->team)
						{
						case RenX::TeamType::GDI:
							gPlayers.push_front(player);
							break;
						case RenX::TeamType::Nod:
							nPlayers.push_front(player);
							break;
						default:
							oPlayers.push_front(player);
							break;
						}
					}
				}

				size_t idColLen = 1, scoreColLen = 1, creditColLen = 1;

				while ((highID /= 10) > 0)
					++idColLen;

				while ((highScore /= 10) >= 1.0)
					++scoreColLen;

				while ((highCredits /= 10) >= 1.0)
					++creditColLen;

				if (server->isAdminLogChanType(type))
					source->sendMessage(channel, string_printf(IRCUNDERLINE IRCCOLOR "03%*s | %*s | %*s | %*s | IP Address", maxNickLen, "Nickname", idColLen, "ID", scoreColLen, "Score", creditColLen, "Credits"));
				else
					source->sendMessage(channel, string_printf(IRCUNDERLINE IRCCOLOR "03%*s | %*s | %*s | %*s", maxNickLen, "Nickname", idColLen, "ID", scoreColLen, "Score", creditColLen, "Credits"));

				auto output_player = [server, type, source, &channel, maxNickLen, idColLen, scoreColLen, creditColLen](RenX::PlayerInfo *player, std::string_view color)
				{
					if (server->isAdminLogChanType(type))
						source->sendMessage(channel, string_printf(IRCCOLOR "%.*s%*.*s" IRCCOLOR " " IRCCOLOR "03|" IRCCOLOR " %*d " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCNORMAL " %.*s", color.size(),
							color.data(), maxNickLen, player->name.size(), player->name.data(), idColLen, player->id, scoreColLen, player->score, creditColLen, player->credits, player->ip.size(), player->ip.data()));
					else
						source->sendMessage(channel, string_printf(IRCCOLOR "%.*s%*.*s" IRCCOLOR " " IRCCOLOR "03|" IRCCOLOR " %*d " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCCOLOR " %*.0f", color.size(),
							color.data(), maxNickLen, player->name.size(), player->name.data(), idColLen, player->id, scoreColLen, player->score, creditColLen, player->credits));
				};

				for (auto node = gPlayers.begin(); node != gPlayers.end(); ++node)
					output_player(*node, gTeamColor);

				for (auto node = nPlayers.begin(); node != nPlayers.end(); ++node)
					output_player(*node, nTeamColor);

				for (auto node = oPlayers.begin(); node != oPlayers.end(); ++node)
					output_player(*node, oTeamColor);
			}
			else source->sendMessage(channel, "No players are in-game."sv);
		}
	}
	if (noServers)
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
}

std::string_view PlayerTableIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Generates a table of all the players in-game. Syntax: PT"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayerTableIRCCommand)

// PlayerInfo IRC Command

void PlayerInfoIRCCommand::create() {
	this->addTrigger("playerinfo"sv);
	this->addTrigger("pi"sv);
	this->addTrigger("player"sv);
	this->addTrigger("pinfo"sv);
}

void PlayerInfoIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		std::string msg;
		RenX::Server *server;
		std::string_view player_info_format = source->getAccessLevel(channel, nick) > 1 ? pluginInstance.getAdminPlayerInfoFormat() : pluginInstance.getPlayerInfoFormat();
		size_t index = 0;

		if (parameters.empty()) { // List all players
			while (index != RenX::getCore()->getServerCount()) {
				server = RenX::getCore()->getServer(index++);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					for (auto node = server->players.begin(); node != server->players.end(); ++node) {
						msg = player_info_format;
						RenX::processTags(msg, server, &*node);
						source->sendMessage(channel, msg);
					}
				}
			}
		}
		else { // List all partial matches
			while (index != RenX::getCore()->getServerCount()) {
				server = RenX::getCore()->getServer(index++);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					for (auto node = server->players.begin(); node != server->players.end(); ++node) {
						if (jessilib::findi(node->name, std::string_view{parameters}) != std::string::npos) {
							msg = player_info_format;
							RenX::processTags(msg, server, &*node);
							source->sendMessage(channel, msg);
						}
					}
				}
			}
		}

		if (msg.empty()) {
			source->sendNotice(nick, "Error: Player not found."sv);
		}
	}
}

std::string_view PlayerInfoIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Gets information about a player. Syntax: PlayerInfo [Player]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayerInfoIRCCommand)

// BuildingInfo IRC Command

void BuildingInfoIRCCommand::create() {
	this->addTrigger("binfo"sv);
	this->addTrigger("bi"sv);
	this->addTrigger("buildinginfo"sv);
	this->addTrigger("building"sv);
}

void BuildingInfoIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	int type = chan->getType();
	bool seenStrip;
	bool foundServer{};
	std::forward_list<std::string> gStrings;
	std::forward_list<std::string> nStrings;
	std::forward_list<std::string> oStrings;
	std::forward_list<std::string> cStrings;
	for (const auto& server : RenX::getCore()->getServers()) {
		if (server->isLogChanType(type)) {
			foundServer = true;
			seenStrip = false;
			for (const auto& building : server->buildings){
				if (building->name.find("Rx_Building_Air"sv) == 0) {
					if (seenStrip) {
						continue;
					}

					seenStrip = true;
				}
				std::string str(pluginInstance.getBuildingInfoFormat());
				RenX::processTags(str, server, nullptr, nullptr, building.get());

				if (building->capturable)
					cStrings.push_front(str);
				else if (building->team == RenX::TeamType::GDI)
					gStrings.push_front(str);
				else if (building->team == RenX::TeamType::Nod)
					nStrings.push_front(str);
				else
					oStrings.push_front(str);
			}

			while (gStrings.empty() == false) {
				gStrings.pop_front();
				source->sendMessage(channel, gStrings.front());
			}
			while (nStrings.empty() == false) {
				nStrings.pop_front();
				source->sendMessage(channel, nStrings.front());
			}
			while (oStrings.empty() == false) {
				oStrings.pop_front();
				source->sendMessage(channel, oStrings.front());
			}
			while (cStrings.empty() == false) {
				cStrings.pop_front();
				source->sendMessage(channel, cStrings.front());
			}
		}
	}

	if (!foundServer) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view BuildingInfoIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Provides a list of buildings, and the status of each one. Syntax: BuildingInfo"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(BuildingInfoIRCCommand)

// Mutators IRC Command

void MutatorsIRCCommand::create() {
	this->addTrigger("mutators"sv);
	this->addTrigger("mutator"sv);
}

void MutatorsIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		std::string list;
		size_t index = 0;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				list = IRCCOLOR "03[Mutators]"s IRCNORMAL;
				for (const auto& mutator : server->mutators) {
					list += " "s + mutator;
				}

				if (server->mutators.empty()) {
					source->sendMessage(channel, "No mutators loaded"sv);
				}
				else {
					source->sendMessage(channel, list);
				}
			}
		}
		if (list.empty())
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view MutatorsIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Provides a list of mutators being used. Syntax: Mutators"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(MutatorsIRCCommand)

// Rotation IRC Command

void RotationIRCCommand::create()
{
	this->addTrigger("rotation"sv);
	this->addTrigger("maprotation"sv);
	this->addTrigger("maps"sv);
	this->addTrigger("rot"sv);
}

void RotationIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		std::string list;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type)) {
				list = IRCCOLOR "03[Rotation]"s IRCNORMAL;
				for (const auto& map : server->maps) {
					if (jessilib::equalsi(server->getMap().name, map.name)) {
						list += jessilib::join<std::string>(" " IRCBOLD "["sv, map.name, "]"sv IRCBOLD);
					}
					else {
						list += " "s + map.name;
					}
				}

				if (server->maps.empty()) {
					source->sendMessage(channel, "No maps in rotation"sv);
				}
				else {
					source->sendMessage(channel, list);
				}
			}
		}
		if (list.empty()) {
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
}

std::string_view RotationIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Provides a list of maps in the server rotation. Syntax: Rotation"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(RotationIRCCommand)

// Map IRC Command

void MapIRCCommand::create() {
	this->addTrigger("map"sv);
}

void MapIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		bool match = false;
		for (const auto& server : RenX::getCore()->getServers()) {
			if (server->isLogChanType(type)) {
				match = true;
				const RenX::Map &map = server->getMap();
				source->sendMessage(channel, jessilib::join<std::string>("Current Map: "sv, map.name, "; GUID: "sv, RenX::formatGUID(map)));
			}
		}
		if (match == false) {
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
}

std::string_view MapIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Fetches the current map. Syntax: Map"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(MapIRCCommand)

// GameInfo IRC Command

void GameInfoIRCCommand::create()
{
	this->addTrigger("gameinfo"sv);
	this->addTrigger("gi"sv);
	this->addTrigger("serverinfo"sv);
	this->addTrigger("si"sv);
}

void GameInfoIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool match = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				match = true;
				const RenX::Map &map = server->getMap();
				std::chrono::seconds time = std::chrono::duration_cast<std::chrono::seconds>(server->getGameTime());
				source->sendMessage(channel, string_printf(IRCCOLOR "03[GameInfo] " IRCCOLOR "%.*s", server->getGameVersion().size(), server->getGameVersion().data()));
				source->sendMessage(channel, jessilib::join<std::string>(IRCCOLOR "03[GameInfo] " IRCCOLOR "10Map" IRCCOLOR ": "sv, map.name, "; " IRCCOLOR "10GUID" IRCCOLOR ": "sv, RenX::formatGUID(map)));
				source->sendMessage(channel, string_printf(IRCCOLOR "03[GameInfo] " IRCCOLOR "10Elapsed time" IRCCOLOR ": %.2lld:%.2lld:%.2lld", time.count() / 3600, (time.count() % 3600) / 60, time.count() % 60));
				source->sendMessage(channel, string_printf(IRCCOLOR "03[GameInfo] " IRCCOLOR "There are " IRCCOLOR "10%d" IRCCOLOR " players online.", server->players.size()));
			}
		}
		if (match == false)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view GameInfoIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Returns information about the game in progress. Syntax: GameInfo"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(GameInfoIRCCommand)

// Steam IRC Command

void SteamIRCCommand::create()
{
	this->addTrigger("steam"sv);
	this->setAccessLevel(1);
}

void SteamIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		if (!parameters.empty()) {
			std::string msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type)) {
					for (auto node = server->players.begin(); node != server->players.end(); ++node) {
						if (jessilib::findi(node->name, parameters) != std::string::npos) {
							std::string playerName = RenX::getFormattedPlayerName(*node);
							msg = string_printf(IRCCOLOR "03[Steam] " IRCCOLOR "%.*s (ID: %d) ", playerName.size(),
								playerName.data(), node->id);
							if (node->steamid != 0) {
								msg += "is using steam ID " IRCBOLD;
								msg += server->formatSteamID(*node);
								msg += string_printf(IRCBOLD "; Steam Profile: " IRCBOLD "https://steamcommunity.com/profiles/%llu" IRCBOLD, node->steamid);
							}
							else
								msg += "is not using steam.";

							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.empty())
				source->sendNotice(nick, "Error: Player not found."sv);
		}
		else
		{
			unsigned int total;
			unsigned int realPlayers;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					total = 0;
					realPlayers = 0;
					for (auto node = server->players.begin(); node != server->players.end(); ++node)
					{
						if (node->isBot == false)
						{
							realPlayers++;
							if (node->steamid != 0)
								total++;
						}
					}

					if (realPlayers != 0)
						source->sendMessage(channel, string_printf("%.2f%% (%u/%u) of players are using Steam.", ((double)total * 100) / ((double)realPlayers), total, realPlayers));
					else
						source->sendMessage(channel, "No players are in-game."sv);
				}
			}
		}
	}
}

std::string_view SteamIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Fetches steam usage information. Syntax: Steam [Player]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(SteamIRCCommand)

// Kill-Death Ratio IRC Command

void KillDeathRatioIRCCommand::create()
{
	this->addTrigger("kills"sv);
	this->addTrigger("deaths"sv);
	this->addTrigger("kdr"sv);
	this->addTrigger("killdeathraio"sv);
}

void KillDeathRatioIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (!parameters.empty()) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			int type = chan->getType();
			std::string msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					for (auto node = server->players.begin(); node != server->players.end(); ++node) {
						if (jessilib::findi(node->name, parameters) != std::string::npos) {
							std::string playerName = RenX::getFormattedPlayerName(*node);
							msg = string_printf(IRCBOLD "%.*s" IRCBOLD IRCCOLOR ": Kills: %u - Deaths: %u - KDR: %.2f", playerName.size(),
								playerName.data(), node->kills, node->deaths, static_cast<double>(node->kills) / (node->deaths == 0 ? 1.0f : static_cast<double>(node->deaths)));
							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.empty())
				source->sendNotice(nick, "Error: Player not found."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: Kills <Player>"sv);
}

std::string_view KillDeathRatioIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Gets a player's kills and deaths. Syntax: Kills <Player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(KillDeathRatioIRCCommand)

// ShowMods IRC Command

void ShowModsIRCCommand::create()
{
	this->addTrigger("showstaff"sv);
	this->addTrigger("showmods"sv);
}

extern ModsGameCommand ModsGameCommand_instance;

void ShowModsIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool sent = false;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				ModsGameCommand_instance.trigger(server, nullptr, ""sv);
				sent = true;
			}
		}
		if (sent == false)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view ShowModsIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message, displaying in-game staff. Syntax: showstaff"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(ShowModsIRCCommand)

// Mods IRC Command

void ModsIRCCommand::create()
{
	this->addTrigger("staff"sv);
	this->addTrigger("mods"sv);
}

void ModsIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (jessilib::equalsi(parameters, "show"sv)) {
		ShowModsIRCCommand_instance.trigger(source, channel, nick, parameters);
	}
	else {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			int type = chan->getType();
			std::string msg;
			std::string_view staff_word = pluginInstance.getStaffTitle();
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type)) {
					msg = "";
					if (server->players.size() != 0) {
						for (auto node = server->players.begin(); node != server->players.end(); ++node) {
							if (node->isBot == false && (!node->adminType.empty() || (node->access != 0 && (!node->gamePrefix.empty() || !node->formatNamePrefix.empty())))) {
								if (!msg.empty()) {
									msg += ", ";
								}
								else {
									msg += staff_word;
									msg += "s in-game: "sv;
								}
								msg += node->gamePrefix;
								msg += node->name;
							}
						}
					}
					if (msg.empty()) {
						msg = jessilib::join<std::string>("No "sv, staff_word, "s are in-game."sv);
					}
					source->sendMessage(channel, msg);
				}
			}
			if (msg.empty()) {
				source->sendMessage(channel,
					"Error: Channel not attached to any connected Renegade X servers."sv);
			}
		}
	}
}

std::string_view ModsIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message, displaying in-game staff. Syntax: staff [show]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(ModsIRCCommand)

// ShowRules IRC Command

void ShowRulesIRCCommand::create()
{
	this->addTrigger("showrules"sv);
}

void ShowRulesIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		std::string msg;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				msg = "Rules: ";
				msg += server->getRules();
				server->sendMessage(msg);
			}
		}
		if (msg.empty())
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view ShowRulesIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Sends a message, displaying the in-game rules. Syntax: showrules"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(ShowRulesIRCCommand)

// Rules IRC Command

void RulesIRCCommand::create()
{
	this->addTrigger("rules"sv);
}

void RulesIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (jessilib::equalsi(parameters, "show"sv)) {
		ShowRulesIRCCommand_instance.trigger(source, channel, nick, parameters);
	}
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			std::string msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					msg = IRCCOLOR "11[Rules]" IRCCOLOR " ";
					msg += server->getRules();
					source->sendMessage(channel, msg);
				}
			}
			if (msg.empty())
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
}

std::string_view RulesIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Displays the in-game rules. Syntax: rules [show]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(RulesIRCCommand)

// Reconnect IRC Command

void ReconnectIRCCommand::create()
{
	this->addTrigger("reconnect"sv);
	this->setAccessLevel(3);
}

void ReconnectIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view ) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		std::string msg;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type)) {
				if (server->reconnect(RenX::DisconnectReason::Triggered)) {
					msg = "Connection established"s;
				}
				else {
					msg = string_printf("[RenX] ERROR: Failed to connect to %.*s on port %u." ENDL, server->getHostname().size(), server->getHostname().c_str(), server->getPort());
				}
				source->sendMessage(channel, msg);
			}
		}
		if (msg.empty()) {
			// We didn't connect anywhere!!
			msg = "ERROR: No servers found to connect to."s;
			source->sendMessage(channel, msg);
		}
	}
}

std::string_view ReconnectIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Resets the RCON connection. Syntax: Reconnect"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(ReconnectIRCCommand)

// GameOver IRC Command

void GameOverIRCCommand::create()
{
	this->addTrigger("gameover"sv);
	this->addTrigger("endmap"sv);
	this->setAccessLevel(3);
}

void GameOverIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		bool match = false;
		std::chrono::seconds delay;

		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				match = true;
				if (jessilib::equalsi(parameters, "empty"sv))
					server->gameoverWhenEmpty();
				else if (jessilib::equalsi(parameters, "if empty"sv))
				{
					if (server->players.size() == server->getBotCount())
						server->gameover();
				}
				else if (jessilib::equalsi(parameters, "now"sv))
					server->gameover();
				else if (jessilib::equalsi(parameters, "stop"sv) || jessilib::equalsi(parameters, "cancel"sv))
				{
					if (server->gameoverStop())
						server->sendMessage("Notice: The scheduled gameover has been cancelled."sv);
					else
						source->sendNotice(nick, "Error: There is no gameover scheduled."sv);
				}
				else
				{
					if (parameters.empty())
						delay = std::chrono::seconds(10);
					else
						delay = std::chrono::seconds(Jupiter::from_string<long long>(parameters));

					server->sendMessage(string_printf("Notice: This server will gameover in %lld seconds.", static_cast<long long>(delay.count())));
					server->gameover(delay);
				}
			}
		}
		if (match == false)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view GameOverIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Forcefully ends the game in progress. Syntax: Gameover [NOW | STOP | [If] Empty | Seconds = 10]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(GameOverIRCCommand)

// SetMap IRC Command

void SetMapIRCCommand::create()
{
	this->addTrigger("setmap"sv);
	this->setAccessLevel(4);
}

void SetMapIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			std::string_view map_name;
			int type = chan->getType();
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					map_name = server->getMapName(parameters);
					if (map_name.empty())
						source->sendMessage(channel, "Error: Map not in rotation."sv);
					else if (server->setMap(map_name) == false)
						source->sendMessage(channel, "Error: Transmission error."sv);
				}
			}
			if (map_name.empty())
				source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else
		source->sendNotice(nick, "Error: Too few parameters. Syntax: setmap <map>"sv);
}

std::string_view SetMapIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Ends the game immediately. Syntax: setmap <map>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(SetMapIRCCommand)

// Mute IRC Command

void MuteIRCCommand::create()
{
	this->addTrigger("mute"sv);
	this->setAccessLevel(2);
}

void MuteIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						server->mute(*player);
						source->sendMessage(channel, jessilib::join<std::string>(RenX::getFormattedPlayerName(*player), IRCCOLOR " has been muted."sv));
					}
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: mute <player>"sv);
}

std::string_view MuteIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Mutes a player. Syntax: mute <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(MuteIRCCommand)

// UnMute IRC Command

void UnMuteIRCCommand::create()
{
	this->addTrigger("unmute"sv);
	this->setAccessLevel(2);
}

void UnMuteIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						server->unmute(*player);
						source->sendMessage(channel, jessilib::join<std::string>(RenX::getFormattedPlayerName(*player), IRCCOLOR " has been unmuted."sv));
					}
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: unmute <player>"sv);
}

std::string_view UnMuteIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Unmutes a player. Syntax: unmute <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(UnMuteIRCCommand)

// Kill IRC Command

void KillIRCCommand::create()
{
	this->addTrigger("kill"sv);
	this->setAccessLevel(2);
}

void KillIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
						server->kill(*player);
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: kill <player>"sv);
}

std::string_view KillIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Kills a player. Syntax: kill <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(KillIRCCommand)

// Disarm IRC Command

void DisarmIRCCommand::create()
{
	this->addTrigger("disarm"sv);
	this->setAccessLevel(2);
}

void DisarmIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->disarm(*player))
							source->sendMessage(channel, jessilib::join<std::string>("All deployables (c4, beacons, etc) belonging to "sv, RenX::getFormattedPlayerName(*player), IRCCOLOR " have been disarmed."sv));
						else
							source->sendMessage(channel, "Error: Server does not support disarms."sv);
					}
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: disarm <player>"sv);
}

std::string_view DisarmIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Disarms all of a player's deployed objects. Syntax: disarm <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmIRCCommand)

// DisarmC4 IRC Command

void DisarmC4IRCCommand::create()
{
	this->addTrigger("disarmc4"sv);
	this->setAccessLevel(2);
}

void DisarmC4IRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->disarmC4(*player))
							source->sendMessage(channel, jessilib::join<std::string>("All C4 belonging to "sv, RenX::getFormattedPlayerName(*player), IRCCOLOR " have been disarmed."sv));
						else
							source->sendMessage(channel, "Error: Server does not support disarms."sv);
					}
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: disarmC4 <player>"sv);
}

std::string_view DisarmC4IRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Disarms all of a player's deployed C4s. Syntax: disarmc4 <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmC4IRCCommand)

// DisarmBeacon IRC Command

void DisarmBeaconIRCCommand::create()
{
	this->addTrigger("disarmb"sv);
	this->addTrigger("disarmbeacon"sv);
	this->addTrigger("disarmbeacons"sv);
	this->setAccessLevel(2);
}

void DisarmBeaconIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						if (server->disarmBeacon(*player))
							source->sendMessage(channel, jessilib::join<std::string>("All beacons belonging to "sv, RenX::getFormattedPlayerName(*player), IRCCOLOR " have been disarmed."sv));
						else
							source->sendMessage(channel, "Error: Server does not support disarms."sv);
					}
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: disarmb <player>"sv);
}

std::string_view DisarmBeaconIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Disarms all of a player's deployed beacons. Syntax: disarmb <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmBeaconIRCCommand)

// MineBan IRC Command

void MineBanIRCCommand::create()
{
	this->addTrigger("mineban"sv);
	this->setAccessLevel(2);
}

void MineBanIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			RenX::PlayerInfo *player;
			bool match = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					match = true;
					player = server->getPlayerByPartName(parameters);
					if (player != nullptr)
					{
						server->mineBan(*player);
						source->sendMessage(channel, "Player can no longer place mines."sv);
					}
					else
						source->sendNotice(nick, "Error: Player not found."sv);
				}
			}
			if (match == false)
				source->sendNotice(nick, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: mineban <player>"sv);
}

std::string_view MineBanIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Bans a player from mining for 1 game (or until they leave). Syntax: mineban <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(MineBanIRCCommand)

// Kick IRC Command

void KickIRCCommand::create()
{
	this->addTrigger("kick"sv);
	this->addTrigger("qkick"sv);
	this->addTrigger("k"sv);
	this->setAccessLevel(2);
}

void KickIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (parameters.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: Kick <Player> [Reason]"sv);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	auto servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	RenX::PlayerInfo *player;
	unsigned int kicks = 0;
	std::string_view name = command_split.first;
	std::string_view reason = command_split.second;
	if (reason.empty()) {
		reason = "No reason"sv;
	}

	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(name);
			if (player != nullptr) {
				server->kickPlayer(*player, reason);
				++kicks;
			}
		}
	}
	source->sendMessage(channel, string_printf("%u players kicked.", kicks));
}

std::string_view KickIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Kicks a player from the game. Syntax: Kick <Player> [Reason]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(KickIRCCommand)

/** Ban IRC Commands */

// BanSearch IRC Command

void BanSearchIRCCommand::create() {
	this->addTrigger("bansearch"sv);
	this->addTrigger("bsearch"sv);
	this->addTrigger("banfind"sv);
	this->addTrigger("bfind"sv);
	this->addTrigger("banlogs"sv);
	this->addTrigger("blogs"sv);
	this->setAccessLevel(2);
}

void BanSearchIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	const auto& entries = RenX::banDatabase->getEntries();
	if (!parameters.empty()) {
		if (entries.size() == 0) {
			source->sendNotice(nick, "The ban database is empty!"sv);
		}
		else {
			auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);

			RenX::BanDatabase::Entry *entry;
			std::string_view params = command_split.second;
			std::function<bool(unsigned int)> isMatch = [&](unsigned int type_l) -> bool {
				switch (type_l)
				{
				default:
				case 0:	// ANY
					return isMatch(1) || isMatch(2) || isMatch(3) || isMatch(4);
				case 1: // ALL
					return true;
				case 2:	// IP
					return entry->ip == Jupiter::asUnsignedInt(params); // TODO: Actually parse as an IP address...
				case 3: // HWID
					return entry->hwid == params;
				case 4: // RDNS
					return entry->rdns == params;
				case 5:	// STEAM
					return entry->steamid == Jupiter::asUnsignedLongLong(params);
				case 6:	// NAME
					return jessilib::equalsi(entry->name, params);
				case 7:	// BANNER
					return jessilib::equalsi(entry->banner, params);
				case 8:	// ACTIVE
					return Jupiter::asBool(params) == entry->is_active();
				}
			};

			unsigned int type;
			std::string_view type_str = command_split.first;
			if (jessilib::equalsi(type_str, "all"sv) || type_str == "*"sv)
				type = 1;
			else if (jessilib::equalsi(type_str, "ip"sv))
				type = 2;
			else if (jessilib::equalsi(type_str, "hwid"sv))
				type = 3;
			else if (jessilib::equalsi(type_str, "rdns"sv))
				type = 4;
			else if (jessilib::equalsi(type_str, "steam"sv))
				type = 5;
			else if (jessilib::equalsi(type_str, "name"sv))
				type = 6;
			else if (jessilib::equalsi(type_str, "banner"sv))
				type = 7;
			else if (jessilib::equalsi(type_str, "active"sv))
				type = 8;
			else
			{
				type = 0;
				params = parameters;
			}

			std::string out;
			std::string types;
			char dateStr[256];
			char expireStr[256];
			for (size_t i = 0; i != entries.size(); i++) {
				entry = entries[i].get();
				if (isMatch(type)) {
					std::string ip_str = Jupiter::Socket::ntop4(entry->ip);

					time_t added_time = std::chrono::system_clock::to_time_t(entry->timestamp);
					if (entry->length.count() != 0) {
						time_t expire_time = std::chrono::system_clock::to_time_t(entry->timestamp + entry->length);
						strftime(expireStr, sizeof(expireStr), "%b %d %Y, %H:%M:%S", localtime(&expire_time));
					}
					else {
						std::strcpy(expireStr, "never");
					}
					strftime(dateStr, sizeof(dateStr), "%b %d %Y, %H:%M:%S", localtime(&added_time));

					if ((entry->flags & 0x7FFF) == 0)
						types = " NULL;"sv;
					else
					{
						types.clear();
						if (entry->is_rdns_ban())
							types += " rdns"sv;
						if (entry->is_type_game())
							types += " game"sv;
						if (entry->is_type_chat())
							types += " chat"sv;
						if (entry->is_type_bot())
							types += " bot"sv;
						if (entry->is_type_vote())
							types += " vote"sv;
						if (entry->is_type_mine())
							types += " mine"sv;
						if (entry->is_type_ladder())
							types += " ladder"sv;
						if (entry->is_type_alert())
							types += " alert"sv;
						types += ";"sv;
					}

					out = string_printf("ID: %lu (" IRCCOLOR "%sactive" IRCCOLOR "); Added: %s; Expires: %s; IP: %.*s/%u; HWID: %.*s; Steam: %llu; Types:%.*s Name: %.*s; Banner: %.*s",
						i, entry->is_active() ? "12" : "04in", dateStr, expireStr, ip_str.size(), ip_str.data(), entry->prefix_length, entry->hwid.size(), entry->hwid.data(), entry->steamid,
						types.size(), types.data(), entry->name.size(), entry->name.data(), entry->banner.size(), entry->banner.data());

					if (!entry->rdns.empty())
					{
						out += "; RDNS: "sv;
						out += entry->rdns;
					}
					if (!entry->reason.empty())
					{
						out += "; Reason: "sv;
						out += entry->reason;
					}
					source->sendNotice(nick, out);
				}
			}
			if (out.empty())
				source->sendNotice(nick, "No matches found."sv);
		}
	}
	else
		source->sendNotice(nick, string_printf("There are a total of %u entries in the ban database.", entries.size()));
}

std::string_view BanSearchIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Searches the ban database for an entry. Syntax: bsearch [ip/rdns/steam/name/banner/active/any/all = any] <player ip/steam/name/banner>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(BanSearchIRCCommand)

// TempBan IRC Command

struct player_search_result {
	RenX::Server* server{}; // Set to the server the player is on, if one is found, last server checked otherwise
	RenX::PlayerInfo* player{}; // Set if a player is found
};

player_search_result findPlayerByPartName(const std::vector<RenX::Server*>& servers, std::string_view name) {
	if (servers.size() == 0) {
		return {};
	}

	player_search_result result;
	for (const auto& server : servers) {
		if (server != nullptr) {
			result.server = server;
			result.player = server->getPlayerByPartName(name);
			if (result.player != nullptr) {
				break;
			}
		}
	}

	return result;
}

player_search_result findPlayerByPartName(IRC_Bot* source, std::string_view channel, std::string_view name) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return {};
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	return findPlayerByPartName(servers, name);
}

void TempBanIRCCommand::create() {
	this->addTrigger("tban"sv);
	this->addTrigger("tb"sv);
	this->addTrigger("tempban"sv);
	this->setAccessLevel(3);
}

void TempBanIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: TempBan [Duration] <Player> [Reason]"sv);
		return;
	}

	std::chrono::seconds duration = pluginInstance.getDefaultTBanTime();
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	std::string_view name = command_split.first;
	std::string_view reason = command_split.second;

	// Try searching by name first
	auto search_result = findPlayerByPartName(source, channel, name);
	if (search_result.server != nullptr
		&& search_result.player == nullptr
		&& !command_split.second.empty()) {
		// Try reading token as a duration instead, and search the name token if duration > 0
		duration = jessilib::duration_from_string(name.data(), name.data() + name.size()).duration;
		if (duration.count() > 0) {
			// It reads as a duration; sanity check & try searching again
			command_split = jessilib::word_split_once_view(command_split.second, WHITESPACE_SV);
			duration = std::min(duration, pluginInstance.getMaxTBanTime());
			name = command_split.first;
			reason = command_split.second;
			search_result = findPlayerByPartName(source, channel, name);
		}
	}

	if (search_result.server == nullptr) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	if (search_result.player == nullptr) {
		source->sendMessage(channel, "Error: Could not find player."sv);
		return;
	}

	if (reason.empty()) {
		reason = "No reason"sv;
	}

	std::string banner{ nick };
	banner += "@IRC";
	search_result.server->banPlayer(*search_result.player, banner, reason, duration);
	source->sendMessage(channel, "Player banned."sv);
}

std::string_view TempBanIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Kicks and temporarily bans a player from the game. Syntax: TempBan [Duration] <Player> [Reason]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(TempBanIRCCommand)

// TempChatBan IRC Command

void TempChatBanIRCCommand::create() {
	this->addTrigger("tchatban"sv);
	this->addTrigger("tcban"sv);
	this->addTrigger("tempchatban"sv);
	this->addTrigger("tcb"sv);
	this->setAccessLevel(3);
}

void TempChatBanIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: TempChatBan [Duration] <Player> [Reason]"sv);
		return;
	}

	std::chrono::seconds duration = pluginInstance.getDefaultTBanTime();
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	std::string_view name = command_split.first;
	std::string_view reason = command_split.second;

	// Try searching by name first
	auto search_result = findPlayerByPartName(source, channel, name);
	if (search_result.server != nullptr
		&& search_result.player == nullptr
		&& !command_split.second.empty()) {
		// Try reading token as a duration instead, and search the name token if duration > 0
		duration = jessilib::duration_from_string(name.data(), name.data() + name.size()).duration;
		if (duration.count() > 0) {
			// It reads as a duration; sanity check & try searching again
			duration = std::min(duration, pluginInstance.getMaxTBanTime());
			command_split = jessilib::word_split_once_view(command_split.second, WHITESPACE_SV);
			name = command_split.first;
			reason = command_split.second;
			search_result = findPlayerByPartName(source, channel, name);
		}
	}

	if (search_result.server == nullptr) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	if (search_result.player == nullptr) {
		source->sendMessage(channel, "Error: Could not find player."sv);
		return;
	}

	if (reason.empty()) {
		reason = "No reaosn"sv;
	}

	std::string banner{ nick };
	banner += "@IRC";
	RenX::banDatabase->add(search_result.server, *search_result.player, banner, reason, duration, RenX::BanDatabase::Entry::FLAG_TYPE_CHAT);
	source->sendMessage(channel, "Player chat banned."sv);
}

std::string_view TempChatBanIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Mutes and temporarily chat bans a player from the game. Syntax: TempChatBan [Duration] <Player> [Reason]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(TempChatBanIRCCommand)

// KickBan IRC Command

void KickBanIRCCommand::create() {
	this->addTrigger("kickban"sv);
	this->addTrigger("kb"sv);
	this->addTrigger("ban"sv);
	this->setAccessLevel(4);
}

void KickBanIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (!parameters.empty()) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			const auto& servers = RenX::getCore()->getServers(chan->getType());
			if (servers.size() != 0)
			{
				RenX::PlayerInfo *player;
				unsigned int kicks = 0;
				auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
				std::string_view name = command_split.first;
				std::string_view reason = command_split.second;
				if (reason.empty()) {
					reason = "No reason"sv;
				}

				std::string banner{ nick };
				banner += "@IRC";
				for (const auto& server : servers) {
					if (server != nullptr) {
						player = server->getPlayerByPartName(name);
						if (player != nullptr) {
							server->banPlayer(*player, banner, reason);
							kicks++;
						}
					}
				}
				if (kicks == 0) {
					source->sendMessage(channel, player_not_found_message(name));
				}
				else {
					source->sendMessage(channel, string_printf("%u players kicked.", kicks));
					RenX::getCore()->banCheck();
				}
			}
			else source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: KickBan <Player> [Reason]"sv);
}

std::string_view KickBanIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Kicks and bans a player from the game. Syntax: KickBan <Player> [Reason]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(KickBanIRCCommand)

// AddBan IRC Command

void AddBanIRCCommand::create()
{
	this->addTrigger("addban"sv);
	this->addTrigger("banadd"sv);
	this->setAccessLevel(4);
}

void AddBanIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (!parameters.empty()) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, " \t="sv);
			if (split_parameters.empty()) {
				source->sendNotice(nick, "Error: Too Few Parameters. Syntax: KickBan <Player> [Reason]"sv);
			}
			else if (split_parameters.size() == 1) {
				KickBanIRCCommand_instance.trigger(source, channel, nick, parameters);
			}
			else {
				std::string name;
				std::string ip_str;
				uint32_t ip = 0U;
				uint8_t prefix_length = 32U;
				uint64_t steamid = 0U;
				std::string hwid;
				std::string rdns;
				std::string banner = std::string{nick};
				banner += "@IRC"sv;
				std::string reason = "No reason"s;
				std::chrono::seconds duration(0);
				uint16_t flags = 0;

				auto missing_value = [&source, &nick](std::string_view token) {
					std::string error_message = "ERROR: No value specified for token: "s;
					error_message += token;
					source->sendNotice(nick, error_message);
				};

				for (auto itr = split_parameters.begin(); itr != split_parameters.end();) {
					std::string_view parameter = *itr;
					++itr;

					if (jessilib::equalsi(parameter, "Name"sv)
						|| jessilib::equalsi(parameter, "Nick"sv)
						|| jessilib::equalsi(parameter, "Nickname"sv)
						|| jessilib::equalsi(parameter, "Username"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						name = *itr;
						++itr;
					}
					else if (jessilib::equalsi(parameter, "IP"sv)
						|| jessilib::equalsi(parameter, "IPAddress"sv)
						|| jessilib::equalsi(parameter, "Address"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						ip_str = *itr;
						++itr;
					}
					else if (jessilib::equalsi(parameter, "Steam"sv)
						|| jessilib::equalsi(parameter, "SteamID"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						steamid = Jupiter::asUnsignedLongLong(*itr);
						++itr;
					}
					else if (jessilib::equalsi(parameter, "HWID"sv)
						|| jessilib::equalsi(parameter, "HardwareID"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						hwid = *itr;
						++itr;
					}
					else if (jessilib::equalsi(parameter, "RDNS"sv)
						|| jessilib::equalsi(parameter, "DNS"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						rdns = *itr;
						++itr;
					}
					else if (jessilib::equalsi(parameter, "Reason"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						// This looks dirtier than it is
						std::string_view reason_view = parameters;
						reason_view.remove_prefix(itr->data() - reason_view.data());
						reason = reason_view;
						break;
					}
					else if (jessilib::equalsi(parameter, "Duration"sv) || jessilib::equalsi(parameter, "Length"sv) || jessilib::equalsi(parameter, "Time"sv)) {
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						std::string_view duration_str = *itr;
						++itr;
						duration = jessilib::duration_from_string(duration_str.data(), duration_str.data() + duration_str.size()).duration;
					}
					else if (jessilib::equalsi(parameter, "Game"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_GAME;
					else if (jessilib::equalsi(parameter, "Chat"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_CHAT;
					else if (jessilib::equalsi(parameter, "Bot"sv) || jessilib::equalsi(parameter, "Command"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_BOT;
					else if (jessilib::equalsi(parameter, "Vote"sv) || jessilib::equalsi(parameter, "Poll"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_VOTE;
					else if (jessilib::equalsi(parameter, "Mine"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_MINE;
					else if (jessilib::equalsi(parameter, "Ladder"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_LADDER;
					else if (jessilib::equalsi(parameter, "Alert"sv))
						flags |= RenX::BanDatabase::Entry::FLAG_TYPE_ALERT;
					else
					{
						std::string error_message = "ERROR: Unknown token: "s;
						error_message += parameter;
						source->sendNotice(nick, error_message);
						return;
					}
				}

				// Default to Game type
				if (flags == 0)
					flags = RenX::BanDatabase::Entry::FLAG_TYPE_GAME;

				size_t index = ip_str.find('/');
				if (index != std::string::npos)
				{
					std::string_view prefix_length_str(ip_str.c_str() + index + 1);
					prefix_length = Jupiter::from_string<unsigned int>(prefix_length_str);
					if (prefix_length == 0)
						prefix_length = 32U;
					ip_str.erase(index);
				}
				ip = Jupiter::Socket::pton4(ip_str.c_str());

				if (rdns.empty()) {
					Jupiter::Socket::resolveHostname(ip_str.c_str(), 0);
				}
				else {
					flags |= RenX::BanDatabase::Entry::FLAG_USE_RDNS;
				}

				RenX::banDatabase->add(name, ip, prefix_length, steamid, hwid, rdns, banner, reason, duration, flags);
				RenX::getCore()->banCheck();
				source->sendMessage(channel, string_printf("Ban added to the database with ID #%u", RenX::banDatabase->getEntries().size() - 1));
			}
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: AddBan <Key> <Value> [...]"sv);
}

std::string_view AddBanIRCCommand::getHelp(std::string_view parameters)
{
	static constexpr std::string_view defaultHelp = "Adds a ban entry to the ban list. Use \"help addban keys\" for a list of input keys. Syntax: AddBan <Key> <Value> [<Key> <Value> ...]"sv;
	static constexpr std::string_view keyHelp = "Valueless keys (flags): Game, Chat, Bot, Vote, Mine, Ladder, Alert; Value-paired keys: Name, IP, Steam, RDNS, Duration, Reason (MUST BE LAST)"sv;
	if (!parameters.empty() && jessilib::equalsi(parameters, "keys"sv))
		return keyHelp;
	return defaultHelp;
}

IRC_COMMAND_INIT(AddBanIRCCommand)

// UnBan IRC Command

void UnBanIRCCommand::create()
{
	this->addTrigger("unban"sv);
	this->addTrigger("deban"sv);
	this->addTrigger("uban"sv);
	this->addTrigger("dban"sv);
	this->setAccessLevel(4);
}

void UnBanIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		size_t index = Jupiter::from_string<size_t>(parameters);
		if (index < RenX::banDatabase->getEntries().size())
		{
			if (RenX::banDatabase->deactivate(index))
				source->sendNotice(nick, "Ban deactivated."sv);
			else
				source->sendNotice(nick, "Error: Ban not active."sv);
		}
		else
			source->sendNotice(nick, "Error: Invalid ban ID; please find the ban ID using \"bansearch\"."sv);
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: unban <Ban ID>"sv);
}

std::string_view UnBanIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Deactivates a ban. Syntax: unban <Ban ID>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(UnBanIRCCommand)

/** Exemption IRC Commands */

// ExemptionSearch IRC Command

void ExemptionSearchIRCCommand::create() {
	this->addTrigger("exemptionsearch"sv);
	this->addTrigger("esearch"sv);
	this->addTrigger("exemptionfind"sv);
	this->addTrigger("efind"sv);
	this->addTrigger("exemptionlogs"sv);
	this->addTrigger("elogs"sv);
	this->setAccessLevel(2);
}

void ExemptionSearchIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	const auto& entries = RenX::exemptionDatabase->getEntries();
	if (!parameters.empty())
	{
		if (entries.size() == 0) {
			source->sendNotice(nick, "The exemption database is empty!"sv);
		}
		else {
			auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);

			RenX::ExemptionDatabase::Entry *entry;
			std::string_view params = command_split.second;
			std::function<bool(unsigned int)> isMatch = [&](unsigned int type_l) -> bool {
				switch (type_l)
				{
				default:
				case 0:	// ANY
					return isMatch(1) || isMatch(2) || isMatch(3) || isMatch(4);
				case 1: // ALL
					return true;
				case 2:	// IP
					return entry->ip == Jupiter::asUnsignedInt(params);
				case 3:	// STEAM
					return entry->steamid == Jupiter::asUnsignedLongLong(params);
				case 4:	// SETTER
					return jessilib::equalsi(entry->setter, params);
				case 5:	// ACTIVE
					return entry->is_active() == Jupiter::asBool(params);
				}
			};

			unsigned int type;
			std::string_view type_str = command_split.first;
			if (jessilib::equalsi(type_str, "all"sv) || type_str == "*"sv)
				type = 1;
			else if (jessilib::equalsi(type_str, "ip"sv))
				type = 2;
			else if (jessilib::equalsi(type_str, "steam"sv))
				type = 3;
			else if (jessilib::equalsi(type_str, "setter"sv))
				type = 4;
			else if (jessilib::equalsi(type_str, "active"sv))
				type = 5;
			else {
				type = 0;
				params = parameters;
			}

			std::string out;
			std::string types;
			char timeStr[256];
			for (size_t i = 0; i != entries.size(); i++)
			{
				entry = entries[i].get();
				if (isMatch(type))
				{
					time_t current_time = std::chrono::system_clock::to_time_t(entry->timestamp);
					std::string ip_str = Jupiter::Socket::ntop4(entry->ip);
					strftime(timeStr, sizeof(timeStr), "%b %d %Y, %H:%M:%S", localtime(&current_time));

					if ((entry->flags & 0xFF) == 0)
						types = " NULL;"sv;
					else
					{
						types.clear();
						if (entry->is_type_kick())
							types += " kick"sv;
						if (entry->is_type_ban())
							types += " ban"sv;
						if (entry->is_ip_exemption())
							types += " ip"sv;
						types += ";"sv;
					}

					out = string_printf("ID: %lu (%sactive); Date: %s; IP: %.*s/%u; Steam: %llu; Types:%.*s Setter: %.*s",
						i, entry->is_active() ? "" : "in", timeStr, ip_str.size(), ip_str.data(), entry->prefix_length, entry->steamid,
						types.size(), types.data(), entry->setter.size(), entry->setter.data());

					source->sendNotice(nick, out);
				}
			}
			if (out.empty())
				source->sendNotice(nick, "No matches found."sv);
		}
	}
	else
		source->sendNotice(nick, string_printf("There are a total of %u entries in the exemption database.", entries.size()));
}

std::string_view ExemptionSearchIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Searches the exemption database for an entry. Syntax: esearch [ip/steam/setter/active/any/all = any] <player ip/steam/setter>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(ExemptionSearchIRCCommand)

// BanExempt IRC Command

void BanExemptIRCCommand::create() {
	this->addTrigger("banexempt"sv);
	this->addTrigger("bexempt"sv);
	this->setAccessLevel(4);
}

void BanExemptIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	std::string_view target_name = command_split.first;
	if (target_name.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: BanExempt <Player> [Reason]"sv);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	RenX::PlayerInfo *player;
	unsigned int exemptions = 0;
	std::string setter{ nick };
	setter += "@IRC";
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(target_name);
			if (player != nullptr) {
				if (player->steamid != 0LL) {
					RenX::exemptionDatabase->add(*server, *player, setter, std::chrono::seconds::zero(), RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN);
				}
				else {
					RenX::exemptionDatabase->add(*server, *player, setter, std::chrono::seconds::zero(), RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN | RenX::ExemptionDatabase::Entry::FLAG_USE_IP);
				}
				++exemptions;
			}
		}
	}

	if (exemptions == 0) {
		source->sendMessage(channel, player_not_found_message(target_name));
	}
	else {
		source->sendMessage(channel, string_printf("%u players added.", exemptions));
	}
}

std::string_view BanExemptIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Exempts a player from bans using their SteamID, or their IP address if they have none. Syntax: BanExempt <Player> [Reason]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(BanExemptIRCCommand)

// KickExempt IRC Command

void KickExemptIRCCommand::create() {
	this->addTrigger("kickexempt"sv);
	this->addTrigger("kexempt"sv);
	this->setAccessLevel(4);
}

void KickExemptIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	std::string_view target_name = command_split.first;
	if (target_name.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: KickExempt <Player> [Reason]"sv);
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	RenX::PlayerInfo *player;
	unsigned int exemptions = 0;
	std::string setter{ nick };
	setter += "@IRC";
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(target_name);
			if (player != nullptr) {
				if (player->steamid != 0LL) {
					RenX::exemptionDatabase->add(*server, *player, setter, std::chrono::seconds::zero(), RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN | RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK);
				}
				else {
					RenX::exemptionDatabase->add(*server, *player, setter, std::chrono::seconds::zero(), RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN | RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK | RenX::ExemptionDatabase::Entry::FLAG_USE_IP);
				}
				++exemptions;
			}
		}
	}
	if (exemptions == 0) {
		source->sendMessage(channel, player_not_found_message(target_name));
	}
	else {
		source->sendMessage(channel, string_printf("%u players added.", exemptions));
	}
}

std::string_view KickExemptIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Exempts a player from kicks and bans using their SteamID, or their IP address if they have none. Syntax: KickExempt <Player> [Reason]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(KickExemptIRCCommand)

// AddExemption IRC Command

#define ADDEXEMPTION_WHITESPACE " \t="

void AddExemptionIRCCommand::create()
{
	this->addTrigger("addexemption"sv);
	this->addTrigger("exemptionadd"sv);
	this->addTrigger("exempt"sv);
	this->setAccessLevel(4);
}

void AddExemptionIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (!parameters.empty()) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, " \t="sv);
			if (split_parameters.empty()) {
				source->sendNotice(nick, "Error: Too Few Parameters. Syntax: BanExempt <Player> [Reason]"sv);
			}
			else if (split_parameters.size() == 1) {
				BanExemptIRCCommand_instance.trigger(source, channel, nick, parameters);
			}
			else {
				std::string ip_str;
				uint32_t ip = 0U;
				uint8_t prefix_length = 32U;
				uint64_t steamid = 0U;
				std::string setter = jessilib::join<std::string>(nick, "@IRC"sv);
				std::chrono::seconds duration = std::chrono::seconds::zero();
				uint8_t flags = 0;

				auto missing_value = [&source, &nick](std::string_view token) {
					std::string error_message = jessilib::join<std::string>("ERROR: No value specified for token: "sv, token);
					source->sendNotice(nick, error_message);
				};

				for (auto itr = split_parameters.begin(); itr != split_parameters.end();) {
					std::string_view parameter = *itr;
					++itr;

					if (jessilib::equalsi(parameter, "IP"sv)
						|| jessilib::equalsi(parameter, "IPAddress"sv)
						|| jessilib::equalsi(parameter, "Address"sv))
					{
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						ip_str = *itr;
						++itr;
					}
					else if (jessilib::equalsi(parameter, "Steam"sv) || jessilib::equalsi(parameter, "SteamID"sv))
					{
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						steamid = Jupiter::asUnsignedLongLong(*itr);
						++itr;
					}
					else if (jessilib::equalsi(parameter, "Duration"sv) || jessilib::equalsi(parameter, "Length"sv) || jessilib::equalsi(parameter, "Time"sv))
					{
						if (itr == split_parameters.end()) {
							missing_value(parameter);
							return;
						}

						const auto& duration_str = *itr;
						duration = jessilib::duration_from_string(duration_str.data(), duration_str.data() + duration_str.size()).duration;
					}
					else if (jessilib::equalsi(parameter, "Ban"sv))
						flags |= RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN;
					else if (jessilib::equalsi(parameter, "Kick"sv))
						flags |= RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK;
					else
					{
						std::string error_message = "ERROR: Unknown token: "s;
						error_message += parameter;
						source->sendNotice(nick, error_message);
						return;
					}
				}

				// Default to Ban type
				if (flags == 0)
					flags = RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN;

				if (!ip_str.empty())
				{
					size_t index = ip_str.find('/');
					if (index != std::string::npos)
					{
						std::string_view prefix_length_str(ip_str.c_str() + index + 1);
						prefix_length = Jupiter::from_string<unsigned int>(prefix_length_str);
						if (prefix_length == 0)
							prefix_length = 32U;
						ip_str.erase(index);
					}
					ip = Jupiter::Socket::pton4(ip_str.c_str());

					if (ip != 0)
						flags |= RenX::ExemptionDatabase::Entry::FLAG_USE_IP;
				}

				if ((flags & RenX::ExemptionDatabase::Entry::FLAG_USE_IP) == 0 && steamid == 0ULL)
					source->sendNotice(nick, "Pointless exemption detected -- no IP or SteamID specified"sv);
				else
				{
					RenX::exemptionDatabase->add(ip, prefix_length, steamid, setter, duration, flags);
					source->sendMessage(channel, string_printf("Exemption added to the database with ID #%u", RenX::exemptionDatabase->getEntries().size() - 1));
				}
			}
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: AddExemption <Key> <Value> [...]"sv);
}

std::string_view AddExemptionIRCCommand::getHelp(std::string_view parameters)
{
	static constexpr std::string_view defaultHelp = "Adds an exemption entry to the exemption list. Use \"help addexemption keys\" for a list of input keys. Syntax: AddExemption <Key> <Value> [<Key> <Value> ...]"sv;
	static constexpr std::string_view keyHelp = "Valueless keys (flags): Ban, Kick; Value-paired keys: IP, Steam, Duration"sv;
	if (!parameters.empty() && jessilib::equalsi(parameters, "keys"sv))
		return keyHelp;
	return defaultHelp;
}

IRC_COMMAND_INIT(AddExemptionIRCCommand)

// UnExempt IRC Command

void UnExemptIRCCommand::create()
{
	this->addTrigger("unexempt"sv);
	this->addTrigger("deexempt"sv);
	this->addTrigger("uexempt"sv);
	this->addTrigger("dexempt"sv);
	this->setAccessLevel(4);
}

void UnExemptIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (!parameters.empty())
	{
		size_t index = Jupiter::from_string<size_t>(parameters);
		if (index < RenX::exemptionDatabase->getEntries().size())
		{
			if (RenX::exemptionDatabase->deactivate(index))
				source->sendNotice(nick, "Exemption deactivated."sv);
			else
				source->sendNotice(nick, "Error: Exemption not active."sv);
		}
		else
			source->sendNotice(nick, "Error: Invalid exemption ID; please find the exemption ID using \"esearch\"."sv);
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: unexempt <Exemption ID>"sv);
}

std::string_view UnExemptIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Deactivates an exemption. Syntax: unexempt <Exemption ID>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(UnExemptIRCCommand)

// AddBots IRC Command

void AddBotsIRCCommand::create() {
	this->addTrigger("addbots"sv);
	this->addTrigger("addbot"sv);
	this->setAccessLevel(2);
}

void AddBotsIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(source->getChannel(channel)->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	int amount = 1;
	std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.empty()) {
		amount = Jupiter::asInt(split_parameters.front());
		if (amount == 0) {
			source->sendMessage(channel, "Error: Invalid amount entered. Amount must be a positive integer."sv);
			return;
		}
	}

	std::string cmd;
	RenX::TeamType team = RenX::TeamType::None;
	if (split_parameters.size() >= 2) {
		team = RenX::getTeam(split_parameters[1]);
	}

	switch (team) {
	case RenX::TeamType::GDI:
		cmd = "addredbots ";
		break;
	case RenX::TeamType::Nod:
		cmd = "addbluebots ";
		break;
	case RenX::TeamType::None:
	case RenX::TeamType::Other:
		cmd = "addbots ";
		break;
	}

	for (const auto& server : servers) {
		size_t base_length = cmd.size();
		if (server != nullptr) {
			cmd += string_printf("%u", amount);
			server->send(cmd);
			cmd.erase(base_length);
		}
		server->sendMessage(string_printf("%u bots have been added to the server.", amount));
	}
}

std::string_view AddBotsIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Adds bots to the game. Syntax: AddBots [Amount=1] [Team]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(AddBotsIRCCommand)

// KillBots IRC Command

void KillBotsIRCCommand::create() {
	this->addTrigger("killbots"sv);
	this->addTrigger("killbot"sv);
	this->setAccessLevel(2);
}

void KillBotsIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	for (const auto& server : servers) {
		server->send("killbots"sv);
		server->sendMessage("All bots have been removed from the server."sv);
	}
}

std::string_view KillBotsIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Removes all bots from the game. Syntax: KillBots"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(KillBotsIRCCommand)

// PhaseBots IRC Command

void PhaseBotsIRCCommand::create() {
	this->addTrigger("phasebots"sv);
	this->addTrigger("phasebot"sv);
	this->setAccessLevel(2);
}

void PhaseBotsIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	for (const auto& server : servers) {
		if (parameters.empty()) {
			if (togglePhasing(server)) {
				server->sendMessage("Bot phasing has been enabled."sv);
			}
			else {
				server->sendMessage("Bot phasing has been disabled."sv);
			}
		}
		else if (jessilib::equalsi(parameters, "true"sv) || jessilib::equalsi(parameters, "on"sv)
			|| jessilib::equalsi(parameters, "start"sv) || jessilib::equalsi(parameters, "1"sv)) {
			togglePhasing(server, true);
			server->sendMessage("Bot phasing has been enabled."sv);
		}
		else {
			togglePhasing(server, false);
			server->sendMessage("Bot phasing has been disabled."sv);
		}
	}
}

std::string_view PhaseBotsIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Toggles the phasing of bots from the game by kicking them after death. Syntax: PhaseBots [on/off]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PhaseBotsIRCCommand)

// RCON IRC Command

void RCONIRCCommand::create() {
	this->addTrigger("rcon"sv);
	this->addTrigger("renx"sv);
	this->setAccessLevel(5);
}

void RCONIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: rcon <input>"sv);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	size_t server_count = RenX::getCore()->send(chan->getType(), parameters);
	if (server_count > 0) {
		source->sendMessage(channel, string_printf("Command sent to %u servers.", server_count));
	}
	else {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
	}
}

std::string_view RCONIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Sends data to the Renegade X server's rcon. Syntax: rcon <input>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(RCONIRCCommand)

// Refund IRC Command

void RefundIRCCommand::create() {
	this->addTrigger("refund"sv);
	this->addTrigger("givecredits"sv);
	this->addTrigger("gc"sv);
	this->addTrigger("money"sv);
	this->addTrigger("credits"sv);
	this->setAccessLevel(3);
}

void RefundIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, WHITESPACE_SV);
	if (split_parameters.size() >= 2) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			int type = chan->getType();
			std::string_view playerName = split_parameters[0];
			double credits = Jupiter::asDouble(split_parameters[1]);
			RenX::PlayerInfo *player;
			std::string msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					player = server->getPlayerByPartName(playerName);
					if (player != nullptr) {
						if (server->giveCredits(*player, credits)) {
							msg = string_printf("You have been refunded %.0f credits by %.*s.", credits, nick.size(), nick.data());
							server->sendMessage(*player, msg);
							msg = string_printf("%.*s has been refunded %.0f credits.", player->name.size(), player->name.data(), credits);
						}
						else {
							msg = "Error: Server does not support refunds."s;
						}
						source->sendMessage(channel, msg);
					}
				}
			}
			if (msg.empty()) {
				source->sendNotice(nick, "Error: Player not found."sv);
			}
		}
	}
	else {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: refund <player> <amount>"sv);
	}
}

std::string_view RefundIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Refunds a player's credits. Syntax: refund <player> <amount>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(RefundIRCCommand)

// Team-Change IRC Command

void TeamChangeIRCCommand::create() {
	this->addTrigger("team"sv);
	this->addTrigger("tc"sv);
	this->addTrigger("ftc"sv);
	this->addTrigger("forcetc"sv);
	this->addTrigger("teamchange"sv);
	this->setAccessLevel(3);
}

void TeamChangeIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (!parameters.empty()) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			int type = chan->getType();
			std::string_view playerName = std::string_view{parameters};
			bool playerFound = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					for (auto node = server->players.begin(); node != server->players.end(); ++node) {
						if (jessilib::findi(node->name, playerName) != std::string::npos) {
							playerFound = true;
							if (!server->changeTeam(*node)) {
								source->sendMessage(channel, "Error: Server does not support team changing."sv);
							}
						}
					}
				}
			}
			if (playerFound == false)
				source->sendNotice(nick, "Error: Player not found."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: team <player>"sv);
}

std::string_view TeamChangeIRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Changes a player's team. Syntax: team <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(TeamChangeIRCCommand)

// TeamChange2 IRC Command

void TeamChange2IRCCommand::create()
{
	this->addTrigger("team2"sv);
	this->addTrigger("tc2"sv);
	this->addTrigger("ftc2"sv);
	this->addTrigger("forcetc2"sv);
	this->addTrigger("teamchange2"sv);
	this->setAccessLevel(3);
}

void TeamChange2IRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (!parameters.empty()) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			int type = chan->getType();
			std::string_view playerName{ parameters };
			bool playerFound = false;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					for (auto node = server->players.begin(); node != server->players.end(); ++node) {
						if (jessilib::findi(node->name, playerName) != std::string::npos) {
							playerFound = true;
							if (server->changeTeam(*node, false) == false) {
								source->sendMessage(channel, "Error: Server does not support team changing."sv);
							}
						}
					}
				}
			}
			if (playerFound == false)
				source->sendNotice(nick, "Error: Player not found."sv);
		}
	}
	else source->sendNotice(nick, "Error: Too Few Parameters. Syntax: team2 <player>"sv);
}

std::string_view TeamChange2IRCCommand::getHelp(std::string_view )
{
	static constexpr std::string_view defaultHelp = "Changes a player's team, without resetting their credits. Syntax: team2 <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(TeamChange2IRCCommand)

// NMode IRC Command

void NModeIRCCommand::create()
{
	this->addTrigger("nmode"sv);
	this->setAccessLevel(2);
}

void NModeIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters)
{
	if (parameters.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: nmode <Player>"sv);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	RenX::PlayerInfo *player;
	unsigned int nmodes = 0;
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(parameters);
			if (player != nullptr) {
				server->nmodePlayer(*player);
				++nmodes;
			}
		}
	}

	source->sendMessage(channel, string_printf("%u players nmoded.", nmodes));
}

std::string_view NModeIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Resets a player's mode from spectator to normal. Syntax: nmode <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(NModeIRCCommand)

// SMode IRC Command

void SModeIRCCommand::create() {
	this->addTrigger("smode"sv);
	this->setAccessLevel(2);
}

void SModeIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: smode <Player>"sv);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	RenX::PlayerInfo *player;
	unsigned int smodes = 0;
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(parameters);
			if (player != nullptr) {
				server->smodePlayer(*player);
				++smodes;
			}
		}
	}
	source->sendMessage(channel, string_printf("%u players smoded.", smodes));
}

std::string_view SModeIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Resets a player's mode from spectator to normal. Syntax: smode <player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(SModeIRCCommand)

// CancelVote IRC Command

void CancelVoteIRCCommand::create() {
	this->addTrigger("cancelvote"sv);
	this->addTrigger("cancelvotes"sv);
	this->addTrigger("cv"sv);
	this->setAccessLevel(2);
}

void CancelVoteIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."sv);
		return;
	}

	bool cancel_all = false;
	RenX::TeamType target = RenX::TeamType::None;

	if (parameters.empty()) {
		cancel_all = true;
	} else {
		if (jessilib::equalsi(parameters, "all"sv) || jessilib::equalsi(parameters, "a"sv)) {
			cancel_all = true;
		} else if (jessilib::equalsi(parameters, "public"sv) || jessilib::equalsi(parameters, "p"sv)) {
			target = RenX::TeamType::None;
		} else if (jessilib::equalsi(parameters, "gdi"sv) || jessilib::equalsi(parameters, "g"sv)) {
			target = RenX::TeamType::GDI;
		} else if (jessilib::equalsi(parameters, "blackhand"sv) || jessilib::equalsi(parameters, "bh"sv) || jessilib::equalsi(parameters, "b"sv)) {
			target = RenX::TeamType::GDI;
		} else if (jessilib::equalsi(parameters, "nod"sv) || jessilib::equalsi(parameters, "n"sv)) {
			target = RenX::TeamType::Nod;
		} else {
			source->sendNotice(nick, "Error: Invalid Team. Allowed values are all/a, public/p, gdi/g, nod/n, blackhand/bh/b."sv);
			return;
		}
	}

	if (cancel_all) {
		for (const auto& server : servers) {
			if (server == nullptr) {
				continue;
			}

			server->cancelVote(RenX::TeamType::None);
			server->cancelVote(RenX::TeamType::GDI);
			server->cancelVote(RenX::TeamType::Nod);
		}
	} else {
		for (const auto& server : servers) {
			if (server == nullptr) {
				continue;
			}

			server->cancelVote(target);
		}
	}
}

std::string_view CancelVoteIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Cancels active votes. Syntax: cancelvote [all|public|gdi|nod|blackhand]"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(CancelVoteIRCCommand)

/** Game Commands */

// Help Game Command

void HelpGameCommand::create() {
	this->addTrigger("help"sv);
}

void HelpGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	RenX::GameCommand *cmd;
	unsigned int cmdCount = 0;
	auto getAccessCommands = [&](int accessLevel) {
		std::string list;
		unsigned int i = 0;
		while (i != source->getCommandCount()) {
			cmd = source->getCommand(i++);
			if (cmd->getAccessLevel() == accessLevel) {
				cmdCount++;
				list = "Access level "s + std::to_string(accessLevel) + " commands: "s;
				list += cmd->getTrigger();
				break;
			}
		}
		while (i != source->getCommandCount()) {
			cmd = source->getCommand(i++);
			if (cmd->getAccessLevel() == accessLevel) {
				cmdCount++;
				list += ", ";
				list += cmd->getTrigger();
			}
		}
		return list;
	};

	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (split_parameters.first.empty()) {
		for (int i = 0; i <= player->access; i++) {
			auto msg = getAccessCommands(i);
			if (!msg.empty()) {
				source->sendMessage(*player, getAccessCommands(i));
			}
		}
		if (cmdCount == 0) {
			source->sendMessage(*player, "No listed commands available."sv);
		}
	}
	else {
		cmd = source->getCommand(split_parameters.first);
		if (cmd != nullptr) {
			if (player->access >= cmd->getAccessLevel()) {
				source->sendMessage(*player, cmd->getHelp(split_parameters.second));
			}
			else {
				source->sendMessage(*player, "Access Denied."sv);
			}
		}
		else {
			source->sendMessage(*player, "Error: Command not found."sv);
		}
	}
}

std::string_view HelpGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Lists commands, or sends command-specific help. Syntax: help [command]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(HelpGameCommand)

// Mods Game Command

void ModsGameCommand::create() {
	this->addTrigger("staff"sv);
	this->addTrigger("mods"sv);
	this->addTrigger("showmods"sv);
}

void ModsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *, std::string_view ) {
	std::string msg;
	std::string_view staff_word = pluginInstance.getStaffTitle();
	for (auto node = source->players.begin(); node != source->players.end(); ++node) {
		if (node->isBot == false && (!node->adminType.empty() || (node->access != 0 && (!node->gamePrefix.empty() || !node->formatNamePrefix.empty())))) {
			if (msg.empty())
				msg = jessilib::join<std::string>(staff_word, "s in-game: "sv);
			else
				msg += ", ";

			msg += node->gamePrefix;
			msg += node->name;
		}
	}
	if (msg.empty()) {
		msg += jessilib::join<std::string>("No "sv, staff_word, "s are in-game"sv);
		RenX::GameCommand *cmd = source->getCommand("modrequest"sv);
		if (cmd != nullptr)
			msg += string_printf("; please use \"%.*s%.*s\" if you require assistance.", source->getCommandPrefix().size(), source->getCommandPrefix().data(), cmd->getTrigger().size(), cmd->getTrigger().data());
		else msg += '.';
	}
	source->sendMessage(msg);
}

std::string_view ModsGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Displays in-game staff. Syntax: staff"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(ModsGameCommand)

// Rules Game Command

void RulesGameCommand::create() {
	this->addTrigger("rules"sv);
	this->addTrigger("rule"sv);
	this->addTrigger("showrules"sv);
	this->addTrigger("showrule"sv);
}

void RulesGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	source->sendMessage(string_printf("Rules: %.*s", source->getRules().size(), source->getRules().data()));
}

std::string_view RulesGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Displays the rules for this server. Syntax: rules"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(RulesGameCommand)

// Mod Request Game Command

void ModRequestGameCommand::create() {
	this->addTrigger("modrequest"sv);
	this->addTrigger("requestmod"sv);
	this->addTrigger("mod"sv);
}

void ModRequestGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendMessage(*player, "Please specify a reason for requesting moderator assistance."sv);
		return;
	}

	std::string_view staff_word = pluginInstance.getStaffTitle();
	std::string fmtName = RenX::getFormattedPlayerName(*player);
	std::string user_message = string_printf(IRCCOLOR "12[%.*s Request] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR "07 has requested assistance in-game for \"%.*s\"; please look in ", staff_word.size(),
		staff_word.data(), fmtName.size(), fmtName.data(), parameters.size(),
		parameters.data());
	std::string channel_message = string_printf(IRCCOLOR "12[%.*s Request] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR "07 has requested assistance in-game! Reason: %.*s" IRCCOLOR, staff_word.size(),
		staff_word.data(), fmtName.size(), fmtName.data(), parameters.size(),
		parameters.data());

	// Alerts a channel and all relevant users in the channel
	auto alert_channel = [&user_message, &channel_message](Jupiter::IRC::Client& server, const Jupiter::IRC::Client::Channel& channel) {
		// Alert channel
		server.sendMessage(channel.getName(), channel_message);

		// Alert relevant users in the channel
		unsigned int total_user_alerts{};
		size_t base_length = user_message.size();
		user_message += channel.getName();

		for (auto& user : channel.getUsers()) {
			if (channel.getUserPrefix(*user.second) != 0 // If the user has a prefix...
				&& user.second->getNickname() != server.getNickname()) { // And the user isn't this bot...
				// Alert the user
				server.sendMessage(user.second->getNickname(), user_message);
				++total_user_alerts;
			}
		}

		user_message.erase(base_length);
		return total_user_alerts;
	};

	// TODO: clean this up by just exposing a vector of servers...
	// Send off alerts
	unsigned int total_user_alerts{};
	size_t server_count = serverManager->size();
	for (size_t server_index = 0; server_index < server_count; ++server_index) {
		if (Jupiter::IRC::Client *server = serverManager->getServer(server_index)) {
			// Alert all admin channels (and their relevant users)
			for (auto& channel : server->getChannels()) {
				if (source->isAdminLogChanType(channel.second.getType())) {
					total_user_alerts += alert_channel(*server, channel.second);
				}
			}
		}
	}

	// Inform the user of the result
	source->sendMessage(*player, string_printf("A total of %u %.*ss have been notified of your assistance request.", total_user_alerts, staff_word.size(),
		staff_word.data()));
}

std::string_view ModRequestGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Notifies staff on IRC that assistance is required. Syntax: modRequest <reason>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(ModRequestGameCommand)

// AdminMessage Game Command

void AdminMessageGameCommand::create() {
	this->addTrigger("amsg"sv);
	this->addTrigger("adminmsg"sv);
	this->addTrigger("amessage"sv);
	this->addTrigger("adminmessage"sv);
	this->setAccessLevel(1);
}

void AdminMessageGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty()) {
		std::string msg = jessilib::join<std::string>(player->gamePrefix, player->name, ": "sv, parameters);
		source->sendAdminMessage(msg);
	}
	else {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: amsg <player> <message>"sv);
	}
}

std::string_view AdminMessageGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Sends an admin message in-game. Syntax: amsg <message>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(AdminMessageGameCommand)

// PAdminMessage Game Command

void PAdminMessageGameCommand::create() {
	this->addTrigger("pamsg"sv);
	this->addTrigger("padminmsg"sv);
	this->addTrigger("pamessage"sv);
	this->addTrigger("padminmessage"sv);
	this->setAccessLevel(1);
}

void PAdminMessageGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.second.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."sv);
		}
		else {
			std::string message;
			message += std::string_view{player->gamePrefix};
			message += player->name;
			message += ": "sv;
			message += split_parameters.second;

			source->sendAdminMessage(*target, message);
			source->sendMessage(*player, "Message sent to "s + target->name);
		}
	}
	else {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: pamsg <player> <message>"sv);
	}
}

std::string_view PAdminMessageGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Sends an admin message to a player in-game. Syntax: pamsg <player> <message>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(PAdminMessageGameCommand)

// Kill Game Command

void KillGameCommand::create() {
	this->addTrigger("kill"sv);
	this->setAccessLevel(1);
}

void KillGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."sv);
		}
		else if (target->access >= player->access) {
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not kill higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		}
		else {
			source->kill(*target);
			source->sendMessage(*player, "Player has been killed."sv);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: kill <player>"sv);
}

std::string_view KillGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Kills a player in the game. Syntax: kill <player>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(KillGameCommand)

// Disarm Game Command

void DisarmGameCommand::create() {
	this->addTrigger("disarm"sv);
	this->setAccessLevel(1);
}

void DisarmGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not disarm higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else if (source->disarm(*target) == false)
			source->sendMessage(*player, "Error: Server does not support disarms."sv);
		else
			source->sendMessage(*player, "Player has been disarmed."sv);
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: disarm <player>"sv);
}

std::string_view DisarmGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Disarms all of a player's deployed objects in the game. Syntax: disarm <player>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmGameCommand)

// DisarmC4 Game Command

void DisarmC4GameCommand::create() {
	this->addTrigger("disarmc4"sv);
	this->setAccessLevel(1);
}

void DisarmC4GameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not disarm higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else if (source->disarmC4(*target) == false)
			source->sendMessage(*player, "Error: Server does not support disarms."sv);
		else
			source->sendMessage(*player, "Player has been disarmed."sv);
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: disarmc4 <player>"sv);
}

std::string_view DisarmC4GameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Disarms all of a player's deployed mines in the game. Syntax: disarmc4 <player>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmC4GameCommand)

// DisarmBeacon Game Command

void DisarmBeaconGameCommand::create() {
	this->addTrigger("disarmb"sv);
	this->addTrigger("disarmbeacon"sv);
	this->addTrigger("disarmbeacons"sv);
	this->setAccessLevel(1);
}

void DisarmBeaconGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not disarm higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else if (source->disarmBeacon(*target) == false)
			source->sendMessage(*player, "Error: Server does not support disarms."sv);
		else
			source->sendMessage(*player, "Player has been disarmed."sv);
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: disarmb <player>"sv);
}

std::string_view DisarmBeaconGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Disarms all of a player's deployed beacons in the game. Syntax: disarmb <player>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmBeaconGameCommand)

// MineBan Game Command

void MineBanGameCommand::create() {
	this->addTrigger("mineban"sv);
	this->setAccessLevel(1);
}

void MineBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not mine-ban higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else
		{
			source->mineBan(*target);
			source->sendMessage(*player, "Player can no longer place mines."sv);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: mineban <player>"sv);
}

std::string_view MineBanGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Bans a player from mining for 1 game (or until they leave). Syntax: mineban <player>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(MineBanGameCommand)

// Kick Game Command

void KickGameCommand::create() {
	this->addTrigger("kick"sv);
	this->addTrigger("qkick"sv);
	this->addTrigger("k"sv);
	this->setAccessLevel(1);
}

void KickGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view reason = split_parameters.second;
		if (reason.empty()) {
			reason = "No reason"sv;
		}

		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."sv);
		}
		else if (player == target) {
			source->sendMessage(*player, "Error: You cannot kick yourself."sv);
		}
		else if (target->access >= player->access) {
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not kick higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		}
		else
		{
			source->kickPlayer(*target, reason);
			source->sendMessage(*player, "Player has been kicked from the game."sv);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: kick <player> [Reason]"sv);
}

std::string_view KickGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Kicks a player from the game. Syntax: kick <player> [Reason]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(KickGameCommand)

// Mute Game Command

void MuteGameCommand::create() {
	this->addTrigger("mute"sv);
	this->setAccessLevel(1);
}

void MuteGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view reason = split_parameters.second;
		if (reason.empty()) {
			reason = "No reason"sv;
		}
		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (player == target)
			source->sendMessage(*player, "Error: You cannot mute yourself."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not mute higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else
		{
			source->mute(*target);
			source->sendMessage(*target, "You have been muted for: "s + std::string{reason});
			source->sendMessage(*player, "Player has been muted from chat."sv);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: mute <player> [Reason]"sv);
}

std::string_view MuteGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Mutes a player from chat. Syntax: mute <player> [Reason]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(MuteGameCommand)

// TempBan Game Command

void TempBanGameCommand::create() {
	this->addTrigger("tban"sv);
	this->addTrigger("tempban"sv);
	this->addTrigger("tb"sv);
	this->setAccessLevel(1);
}

void TempBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view name = split_parameters.first;
		std::chrono::seconds duration = pluginInstance.getDefaultTBanTime();
		std::string_view reason = split_parameters.second;

		RenX::PlayerInfo *target = source->getPlayerByPartName(name);
		if (target == nullptr
			&& !split_parameters.second.empty()) {
			// Try reading first token as duration
			duration = jessilib::duration_from_string(name.data(), name.data() + name.size()).duration;
			if (duration.count() > 0) {
				duration = std::min(duration, pluginInstance.getMaxTBanTime());
				split_parameters = jessilib::word_split_once_view(split_parameters.second, WHITESPACE_SV);
				name = split_parameters.first;
				reason = split_parameters.second;
				target = source->getPlayerByPartName(name);
			}
		}

		if (reason.empty()) {
			reason = "No reason"sv;
		}

		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (player == target)
			source->sendMessage(*player, "Error: You can't ban yourself."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can't ban higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else
		{
			source->banPlayer(*target, player->name, reason, duration);
			source->sendMessage(*player, "Player has been temporarily banned and kicked from the game."sv);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: tban [Duration] <player> [Reason]"sv);
}

std::string_view TempBanGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Kicks and temporarily bans a player from the game. Syntax: tban [Duration] <player> [Reason]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(TempBanGameCommand)

// TempChatBan Game Command

void TempChatBanGameCommand::create() {
	this->addTrigger("tchatban"sv);
	this->addTrigger("tcban"sv);
	this->addTrigger("tempchatban"sv);
	this->addTrigger("tcb"sv);
	this->setAccessLevel(1);
}

void TempChatBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view name = split_parameters.first;
		std::chrono::seconds duration = pluginInstance.getDefaultTBanTime();
		std::string_view reason = split_parameters.second;

		RenX::PlayerInfo *target = source->getPlayerByPartName(name);
		if (target == nullptr
			&& !split_parameters.second.empty()) {
			// Try reading first token as duration
			duration = jessilib::duration_from_string(name.data(), name.data() + name.size()).duration;
			if (duration.count() > 0) {
				duration = std::min(duration, pluginInstance.getMaxTBanTime());
				split_parameters = jessilib::word_split_once_view(split_parameters.second, WHITESPACE_SV);
				name = split_parameters.first;
				reason = split_parameters.second;
				target = source->getPlayerByPartName(name);
			}
		}

		if (reason.empty()) {
			reason = "No reason"sv;
		}

		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."sv);
		else if (player == target)
			source->sendMessage(*player, "Error: You can not ban yourself."sv);
		else if (target->access >= player->access)
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not ban higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		else {
			source->mute(*target);
			RenX::banDatabase->add(source, *target, player->name, reason, duration, RenX::BanDatabase::Entry::FLAG_TYPE_CHAT);
			source->sendMessage(*player, "Player has been temporarily muted and chat banned from the game."sv);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: tchatban [Duration] <player> [Reason]"sv);
}

std::string_view TempChatBanGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Mutes and temporarily chat bans a player from the game. Syntax: tchatban [Duration] <player> [Reason]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(TempChatBanGameCommand)

// KickBan Game Command

void KickBanGameCommand::create() {
	this->addTrigger("ban"sv);
	this->addTrigger("kickban"sv);
	this->addTrigger("kb"sv);
	this->addTrigger("b"sv);
	this->setAccessLevel(2);
}

void KickBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view reason = split_parameters.second;
		if (reason.empty()) {
			reason = "No reason"sv;
		}
		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."sv);
		}
		else if (player == target) {
			source->sendMessage(*player, "Error: You can not ban yourself."sv);
		}
		else if (target->access >= player->access) {
			source->sendMessage(*player, jessilib::join<std::string>("Error: You can not ban higher level "sv, pluginInstance.getStaffTitle(), "s."sv));
		}
		else {
			source->banPlayer(*target, player->name, reason);
			source->sendMessage(*player, "Player has been banned and kicked from the game."sv);
			RenX::getCore()->banCheck();
		}
	}
	else {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: ban <player> [reason]"sv);
	}
}

std::string_view KickBanGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Kicks and bans a player from the game. Syntax: ban <player> [reason]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(KickBanGameCommand)

// AddBots Game Command

void AddBotsGameCommand::create() {
	this->addTrigger("addbots"sv);
	this->addTrigger("abots"sv);
	this->addTrigger("addbot"sv);
	this->addTrigger("abot"sv);
	this->setAccessLevel(1);
}

void AddBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, WHITESPACE_SV);
	RenX::TeamType team = RenX::TeamType::None;
	if (split_parameters.size() >= 2) {
		team = RenX::getTeam(split_parameters[1]);
	}

	std::string cmd;
	switch (team)
	{
	case RenX::TeamType::GDI:
		cmd = "addredbots ";
		break;
	case RenX::TeamType::Nod:
		cmd = "addbluebots ";
		break;
	default:
	case RenX::TeamType::None:
	case RenX::TeamType::Other:
		cmd = "addbots ";
		break;
	}

	unsigned int amount;
	if (split_parameters.empty()) {
		amount = 1;
	}
	else {
		amount = Jupiter::asUnsignedInt(split_parameters.front());
	}

	cmd += string_printf("%u", amount);

	source->send(cmd);
	source->sendMessage(*player, string_printf("%u bots have been added to the server.", amount));
}

std::string_view AddBotsGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Adds bots to the game. Syntax: addbots [amount=1] [team]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(AddBotsGameCommand)

// KillBots Game Command

void KillBotsGameCommand::create() {
	this->addTrigger("killbots"sv);
	this->addTrigger("kbots"sv);
	this->addTrigger("rembots"sv);
	this->addTrigger("rbots"sv);
	this->setAccessLevel(2);
}

void KillBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	source->send("killbots"sv);
	source->sendMessage(*player, "All bots have been removed from the server."sv);
}

std::string_view KillBotsGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Removes all bots from the game. Syntax: killbots"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(KillBotsGameCommand)

// PhaseBots Game Command

void PhaseBotsGameCommand::create() {
	this->addTrigger("phasebots"sv);
	this->addTrigger("pbots"sv);
	this->setAccessLevel(1);
}

void PhaseBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (parameters.empty())
	{
		if (togglePhasing(source))
			source->sendMessage(*player, "Bot phasing has been enabled."sv);
		else source->sendMessage(*player, "Bot phasing has been disabled."sv);
	}
	else if (jessilib::equalsi(parameters, "true"sv) || jessilib::equalsi(parameters, "on"sv)
		|| jessilib::equalsi(parameters, "start"sv) || jessilib::equalsi(parameters, "1"sv)) {
		togglePhasing(source, true);
		source->sendMessage(*player, "Bot phasing has been enabled."sv);
	}
	else
	{
		togglePhasing(source, false);
		source->sendMessage(*player, "Bot phasing has been disabled."sv);
	}
}

std::string_view PhaseBotsGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Removes all bots from the game. Syntax: phasebots"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(PhaseBotsGameCommand)

// NMode Game Command

void NModeGameCommand::create() {
	this->addTrigger("nmode"sv);
	this->setAccessLevel(1);
}

void NModeGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: nmode <player-name>"sv);
		return;
	}

	RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
	if (target == nullptr) {
		source->sendMessage(*player, "Error: Player not found."sv);
		return;
	}

	if (!source->nmodePlayer(*target)) {
		source->sendMessage(*player, "Error: Could not set player's mode."sv);
		return;
	}

	source->sendMessage(*player, "Player's mode has been reset."sv);
}

std::string_view NModeGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Resets a player's mode from spectator to normal. Syntax: nmode <player-name>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(NModeGameCommand)

// SMode Game Command

void SModeGameCommand::create() {
	this->addTrigger("smode"sv);
	this->setAccessLevel(1);
}

void SModeGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (parameters.empty()) {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: smode <player-name>"sv);
		return;
	}

	RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
	if (target == nullptr) {
		source->sendMessage(*player, "Error: Player not found."sv);
		return;
	}

	if (!source->smodePlayer(*target)) {
		source->sendMessage(*player, "Error: Could not set player's mode."sv);
		return;
	}

	source->sendMessage(*player, "Player's mode has been reset."sv);
}

std::string_view SModeGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Resets a player's mode from spectator to normal. Syntax: smode <player-name>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(SModeGameCommand)

// CancelVote Game Command

void CancelVoteGameCommand::create() {
	this->addTrigger("cancelvote"sv);
	this->addTrigger("cancelvotes"sv);
	this->addTrigger("cv"sv);
	this->setAccessLevel(1);
}

void CancelVoteGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	bool cancel_all = false;
	RenX::TeamType target = RenX::TeamType::None;

	if (parameters.empty()) {
		cancel_all = true;
	}
	else {
		if (jessilib::equalsi(parameters, "all"sv) || jessilib::equalsi(parameters, "a"sv)) {
			cancel_all = true;
		} else if (jessilib::equalsi(parameters, "public"sv) || jessilib::equalsi(parameters, "p"sv)) {
			target = RenX::TeamType::None;
		} else if (jessilib::equalsi(parameters, "gdi"sv) || jessilib::equalsi(parameters, "g"sv)) {
			target = RenX::TeamType::GDI;
		} else if (jessilib::equalsi(parameters, "blackhand"sv) || jessilib::equalsi(parameters, "bh"sv) || jessilib::equalsi(parameters, "b"sv)) {
			target = RenX::TeamType::GDI;
		} else if (jessilib::equalsi(parameters, "nod"sv) || jessilib::equalsi(parameters, "n"sv)) {
			target = RenX::TeamType::Nod;
		} else {
			source->sendMessage(*player, "Error: Invalid Team. Allowed values are all/a, public/p, gdi/g, nod/n, blackhand/bh/b."sv);
			return;
		}
	}

	if (cancel_all) {
		source->cancelVote(RenX::TeamType::None);
		source->cancelVote(RenX::TeamType::GDI);
		source->cancelVote(RenX::TeamType::Nod);
	}
	else {
		source->cancelVote(target);
	}
}

std::string_view CancelVoteGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Cancels active votes. Syntax: cancelvote [all|public|gdi|nod|blackhand]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(CancelVoteGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
