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

using namespace Jupiter::literals;
using namespace jessilib::literals;
using namespace std::literals;

const Jupiter::ReferenceString RxCommandsSection = "RenX.Commands"_jrs;

bool togglePhasing(RenX::Server *server, bool newState) {
	server->varData[RxCommandsSection].set("phasing"sv, newState ? "true"s : "false"s);
	return newState;
}

bool togglePhasing(RenX::Server *server) {
	return togglePhasing(server, !server->varData[RxCommandsSection].get<bool>("phasing"_jrs, false));
}

void onDie(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (player.isBot && server.varData[RxCommandsSection].get<bool>("phasing"_jrs, false)) {
		server.kickPlayer(player, ""sv);
	}
}

std::string player_not_found_message(std::string_view name) {
	std::string result = "Error: Player \""s;
	result += name;
	result += "\" not found.";
	return result;
}

void RenX_CommandsPlugin::RenX_OnSuicide(RenX::Server &server, const RenX::PlayerInfo &player, const Jupiter::ReadableString &) {
	onDie(server, player);
}

void RenX_CommandsPlugin::RenX_OnKill(RenX::Server &server, const RenX::PlayerInfo &, const RenX::PlayerInfo &victim, const Jupiter::ReadableString &) {
	onDie(server, victim);
}

void RenX_CommandsPlugin::RenX_OnDie(RenX::Server &server, const RenX::PlayerInfo &player, const Jupiter::ReadableString &) {
	onDie(server, player);
}

bool RenX_CommandsPlugin::initialize() {
	auto default_tban_time = this->config.get("DefaultTBanTime"_jrs, "1d"_jrs);
	auto max_tban_time = this->config.get("MaxTBanTime"_jrs, "1w"_jrs);
	m_defaultTempBanTime = jessilib::duration_from_string(default_tban_time.data(), default_tban_time.data() + default_tban_time.size()).duration;
	m_maxTempBanTime = std::max(jessilib::duration_from_string(max_tban_time.data(), max_tban_time.data() + max_tban_time.size()).duration, m_defaultTempBanTime);
	m_playerInfoFormat = this->config.get("PlayerInfoFormat"_jrs, IRCCOLOR "03[Player Info]" IRCCOLOR "{TCOLOR} Name: " IRCBOLD "{RNAME}" IRCBOLD " - ID: {ID} - Team: " IRCBOLD "{TEAML}" IRCBOLD " - Vehicle Kills: {VEHICLEKILLS} - Building Kills {BUILDINGKILLS} - Kills {KILLS} - Deaths: {DEATHS} - KDR: {KDR} - Access: {ACCESS}"_jrs);
	m_adminPlayerInfoFormat = this->config.get("AdminPlayerInfoFormat"_jrs, Jupiter::StringS::Format("%.*s - IP: " IRCBOLD "{IP}" IRCBOLD " - HWID: " IRCBOLD "{HWID}" IRCBOLD " - RDNS: " IRCBOLD "{RDNS}" IRCBOLD " - Steam ID: " IRCBOLD "{STEAM}", m_playerInfoFormat.size(),
		m_playerInfoFormat.data()));
	m_buildingInfoFormat = this->config.get("BuildingInfoFormat"_jrs, ""_jrs IRCCOLOR + RenX::tags->buildingTeamColorTag + RenX::tags->buildingNameTag + IRCCOLOR " - " IRCCOLOR "07"_jrs + RenX::tags->buildingHealthPercentageTag + "%"_jrs);
	m_staffTitle = this->config.get("StaffTitle"_jrs, "Moderator"_jrs);

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

const Jupiter::ReadableString &RenX_CommandsPlugin::getPlayerInfoFormat() const {
	return m_playerInfoFormat;
}

const Jupiter::ReadableString &RenX_CommandsPlugin::getAdminPlayerInfoFormat() const {
	return m_adminPlayerInfoFormat;
}

const Jupiter::ReadableString &RenX_CommandsPlugin::getBuildingInfoFormat() const {
	return m_buildingInfoFormat;
}

const Jupiter::ReadableString &RenX_CommandsPlugin::getStaffTitle() const {
	return m_staffTitle;
}

// Plugin instantiation and entry point.
RenX_CommandsPlugin pluginInstance;

/** Console Commands */

// RawRCON Console Command

RawRCONConsoleCommand::RawRCONConsoleCommand() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rrcon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rawrcon"));
}

void RawRCONConsoleCommand::trigger(const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		puts("Error: Too Few Parameters. Syntax: rrcon <input>");
		return;
	}

	const auto& servers = RenX::getCore()->getServers();
	if (servers.empty()) {
		puts("Error: Not connected to any Renegade X servers.");
		return;
	}

	Jupiter::StringS msg = parameters;
	msg += '\n';
	for (const auto& server : servers) {
		server->sendData(msg);
	}
}

const Jupiter::ReadableString &RawRCONConsoleCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends data over the Renegade X server's rcon connection. Syntax: rrcon <data>");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RawRCONConsoleCommand)

// RCON Console Command

RCONConsoleCommand::RCONConsoleCommand() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rcon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("renx"));
}

void RCONConsoleCommand::trigger(const Jupiter::ReadableString &parameters) {
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

const Jupiter::ReadableString &RCONConsoleCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Executes a command over the Renegade X server's rcon connection. Syntax: rcon <input>");
	return defaultHelp;
}

CONSOLE_COMMAND_INIT(RCONConsoleCommand)

/** IRC Commands */

// Msg IRC Command

void MsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("msg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("say"));
}

void MsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		int type = source->getChannel(channel)->getType();
		Jupiter::StringL msg;
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
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Msg <Message>"));
}

const Jupiter::ReadableString &MsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message in - game.Syntax: Msg <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(MsgIRCCommand)

// PMsg IRC Command

void PMsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("psay"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("page"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ppage"));
	this->setAccessLevel(1);
}

void PMsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!command_split.second.empty())
	{
		int type = source->getChannel(channel)->getType();

		Jupiter::ReferenceString name = command_split.first;
		RenX::PlayerInfo *player;
		std::string msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += command_split.second;
		if (parameters.isNotEmpty())
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
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: PMsg <Player> <Message>"));
}

const Jupiter::ReadableString &PMsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message in - game.Syntax: PMsg <Player> <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PMsgIRCCommand)

// Host Msg IRC Command

void HostMsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("hmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("hsay"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("hostmessage"));
	this->setAccessLevel(4);
}

void HostMsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: hmsg <Message>"));
}

const Jupiter::ReadableString &HostMsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message in-game. Syntax: hmsg <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(HostMsgIRCCommand)

// Admin Msg IRC Command

void AdminMsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("amsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("asay"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("adminmessage"));
	this->setAccessLevel(4);
}

void AdminMsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: amsg <Message>"));
}

const Jupiter::ReadableString &AdminMsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends an admin message in-game. Syntax: amsg <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(AdminMsgIRCCommand)

// PAdminMsg IRC Command

void PAdminMsgIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pamsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pasay"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("apage"));
	this->setAccessLevel(4);
}

void PAdminMsgIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!command_split.second.empty()) {
		int type = source->getChannel(channel)->getType();
		Jupiter::ReferenceString name = command_split.first;
		RenX::PlayerInfo *player;
		std::string msg;
		char prefix = source->getChannel(channel)->getUserPrefix(nick);
		if (prefix != '\0')
			msg += prefix;
		msg += nick;
		msg += "@IRC: ";
		msg += command_split.second;
		if (parameters.isNotEmpty())
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
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: pamsg <Player> <Message>"));
}

const Jupiter::ReadableString &PAdminMsgIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends an admin message to a player in-game. Syntax: pamsg <Player> <Message>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PAdminMsgIRCCommand)

// Players IRC Command

void PlayersIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("players"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pl"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playerlist"));
}

const size_t STRING_LENGTH = 240;

void PlayersIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &, const Jupiter::ReadableString &)
{
	int type = source->getChannel(channel)->getType();

	// Team colors
	const Jupiter::ReadableString &gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeamColor = RenX::getTeamColor(RenX::TeamType::Other);

	// Team names
	const Jupiter::ReadableString &gTeam = RenX::getTeamName(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeam = RenX::getTeamName(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeam = RenX::getTeamName(RenX::TeamType::Other);

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
					source->sendMessage(channel, Jupiter::StringS::Format("ERROR: NO PLAYERS BUT BOT_COUNT = %u.", server->getBotCount()));
					continue;
				}

				// End string containers
				std::list<Jupiter::String *> gStrings;
				std::list<Jupiter::String *> nStrings;
				std::list<Jupiter::String *> oStrings;

				Jupiter::StringL *gCurrent = nullptr;
				Jupiter::StringL *nCurrent = nullptr;
				Jupiter::StringL *oCurrent = nullptr;

				// Team player counters
				unsigned int gTotal = 0;
				unsigned int nTotal = 0;
				unsigned int oTotal = 0;

				// Bot counters
				unsigned int gBots = 0;
				unsigned int nBots = 0;
				unsigned int oBots = 0;

				for (auto node = server->players.begin(); node != server->players.end(); ++node)
				{
					Jupiter::String name = RenX::getFormattedPlayerName(*node);
					if (name.size() > STRING_LENGTH - 32) continue; // Name will be too long to send.

					switch (node->team)
					{
					case RenX::TeamType::Nod:
						if (nCurrent == nullptr || nCurrent->size() + name.size() > STRING_LENGTH)
						{
							nCurrent = new Jupiter::StringL(STRING_LENGTH);
							nCurrent->format(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD, nTeamColor.size(),
								nTeamColor.data(), nTeam.size(),
								nTeam.data(), name.size(), name.data());
							nStrings.push_back(nCurrent);
						}
						else nCurrent->aformat(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.data());
						nTotal++;
						if (node->isBot)
							nBots++;
						break;
					case RenX::TeamType::GDI:
						if (gCurrent == nullptr || gCurrent->size() + name.size() > STRING_LENGTH)
						{
							gCurrent = new Jupiter::StringL(STRING_LENGTH);
							gCurrent->format(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD, gTeamColor.size(),
								gTeamColor.data(), gTeam.size(),
								gTeam.data(), name.size(), name.data());
							gStrings.push_back(gCurrent);
						}
						else gCurrent->aformat(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.data());
						gTotal++;
						if (node->isBot)
							gBots++;
						break;
					default:
						if (oCurrent == nullptr || oCurrent->size() + name.size() > STRING_LENGTH)
						{
							oCurrent = new Jupiter::StringL(STRING_LENGTH);
							oCurrent->format(IRCCOLOR "%.*s[%.*s]: " IRCBOLD "%.*s" IRCBOLD, oTeamColor.size(),
								oTeamColor.data(), oTeam.size(),
								oTeam.data(), name.size(), name.data());
							oStrings.push_back(oCurrent);
						}
						else oCurrent->aformat(IRCCOLOR ", " IRCBOLD "%.*s" IRCBOLD, name.size(), name.data());
						oTotal++;
						if (node->isBot)
							oBots++;
						break;
					}
				}
				Jupiter::StringL *outString;
				while (gStrings.size() != 0)
				{
					outString = gStrings.front();
					source->sendMessage(channel, *outString);

					delete outString;
					gStrings.pop_front();
				}
				while (nStrings.size() != 0)
				{
					outString = nStrings.front();
					source->sendMessage(channel, *outString);

					delete outString;
					nStrings.pop_front();
				}
				while (oStrings.size() != 0)
				{
					outString = oStrings.front();
					source->sendMessage(channel, *outString);

					delete outString;
					oStrings.pop_front();
				}

				Jupiter::StringL out;
				out.format(IRCCOLOR "03Total Players" IRCCOLOR ": %u", server->players.size());
				if (gBots + nBots + oBots > 0)
					out.aformat(" (%u bots)", gBots + nBots + oBots);
				if (gTotal > 0)
				{
					out.aformat(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", gTeamColor.size(),
						gTeamColor.data(), gTeam.size(),
						gTeam.data(), gTotal);
					if (gBots > 0)
						out.aformat(" (%u bots)", gBots);
				}
				if (nTotal > 0)
				{
					out.aformat(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", nTeamColor.size(),
						nTeamColor.data(), nTeam.size(),
						nTeam.data(), nTotal);
					if (nBots > 0)
						out.aformat(" (%u bots)", nBots);
				}
				if (oTotal > 0)
				{
					out.aformat(IRCCOLOR "02 | " IRCCOLOR "%.*s%.*s" IRCCOLOR ": %u", oTeamColor.size(),
						oTeamColor.data(), oTeam.size(),
						oTeam.data(), oTotal);
					if (oBots > 0)
						out.aformat(" (%u bots)", oBots);
				}
				source->sendMessage(channel, out);
			}
			else
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("No players are in-game."));
		}
	}
	if (noServers)
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
}

const Jupiter::ReadableString &PlayersIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Lists the players currently in-game. Syntax: Players");
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayersIRCCommand)

// PlayerTable IRC Command
void PlayerTableIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playertable"));
}

void PlayerTableIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &)
{
	int type = source->getChannel(channel)->getType();

	// Team colors
	const Jupiter::ReadableString &gTeamColor = RenX::getTeamColor(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeamColor = RenX::getTeamColor(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeamColor = RenX::getTeamColor(RenX::TeamType::Other);

	// Team names
	const Jupiter::ReadableString &gTeam = RenX::getTeamName(RenX::TeamType::GDI);
	const Jupiter::ReadableString &nTeam = RenX::getTeamName(RenX::TeamType::Nod);
	const Jupiter::ReadableString &oTeam = RenX::getTeamName(RenX::TeamType::Other);

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

				STRING_LITERAL_AS_NAMED_REFERENCE(NICK_COL_HEADER, "Nickname");
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
					source->sendMessage(channel, Jupiter::StringS::Format(IRCUNDERLINE IRCCOLOR "03%*.*s | %*s | %*s | %*s | IP Address", maxNickLen, NICK_COL_HEADER.size(), NICK_COL_HEADER.data(), idColLen, "ID", scoreColLen, "Score", creditColLen, "Credits"));
				else
					source->sendMessage(channel, Jupiter::StringS::Format(IRCUNDERLINE IRCCOLOR "03%*.*s | %*s | %*s | %*s", maxNickLen, NICK_COL_HEADER.size(), NICK_COL_HEADER.data(), idColLen, "ID", scoreColLen, "Score", creditColLen, "Credits"));

				auto output_player = [server, type, source, &channel, maxNickLen, idColLen, scoreColLen, creditColLen](RenX::PlayerInfo *player, const Jupiter::ReadableString &color)
				{
					if (server->isAdminLogChanType(type))
						source->sendMessage(channel, Jupiter::StringS::Format(IRCCOLOR "%.*s%*.*s" IRCCOLOR " " IRCCOLOR "03|" IRCCOLOR " %*d " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCNORMAL " %.*s", color.size(),
							color.data(), maxNickLen, player->name.size(), player->name.data(), idColLen, player->id, scoreColLen, player->score, creditColLen, player->credits, player->ip.size(), player->ip.data()));
					else
						source->sendMessage(channel, Jupiter::StringS::Format(IRCCOLOR "%.*s%*.*s" IRCCOLOR " " IRCCOLOR "03|" IRCCOLOR " %*d " IRCCOLOR "03|" IRCCOLOR " %*.0f " IRCCOLOR "03|" IRCCOLOR " %*.0f", color.size(),
							color.data(), maxNickLen, player->name.size(), player->name.data(), idColLen, player->id, scoreColLen, player->score, creditColLen, player->credits));
				};

				for (auto node = gPlayers.begin(); node != gPlayers.end(); ++node)
					output_player(*node, gTeamColor);

				for (auto node = nPlayers.begin(); node != nPlayers.end(); ++node)
					output_player(*node, nTeamColor);

				for (auto node = oPlayers.begin(); node != oPlayers.end(); ++node)
					output_player(*node, oTeamColor);
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("No players are in-game."));
		}
	}
	if (noServers)
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
}

const Jupiter::ReadableString &PlayerTableIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Generates a table of all the players in-game. Syntax: PT");
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayerTableIRCCommand)

// PlayerInfo IRC Command

void PlayerInfoIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("playerinfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pi"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("player"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pinfo"));
}

void PlayerInfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		Jupiter::StringL msg;
		RenX::Server *server;
		const Jupiter::ReadableString &player_info_format = source->getAccessLevel(channel, nick) > 1 ? pluginInstance.getAdminPlayerInfoFormat() : pluginInstance.getPlayerInfoFormat();
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
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
}

const Jupiter::ReadableString &PlayerInfoIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets information about a player. Syntax: PlayerInfo [Player]");
	return defaultHelp;
}

IRC_COMMAND_INIT(PlayerInfoIRCCommand)

// BuildingInfo IRC Command

void BuildingInfoIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("binfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bi"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("buildinginfo"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("building"));
}

void BuildingInfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	int type = chan->getType();
	bool seenStrip;
	std::forward_list<Jupiter::String *> gStrings;
	std::forward_list<Jupiter::String *> nStrings;
	std::forward_list<Jupiter::String *> oStrings;
	std::forward_list<Jupiter::String *> cStrings;
	Jupiter::String *str = nullptr;
	for (const auto& server : RenX::getCore()->getServers()) {
		if (server->isLogChanType(type)) {
			seenStrip = false;
			for (const auto& building : server->buildings){
				if (building->name.find("Rx_Building_Air"_jrs) == 0) {
					if (seenStrip) {
						continue;
					}

					seenStrip = true;
				}
				str = new Jupiter::String(pluginInstance.getBuildingInfoFormat());
				RenX::processTags(*str, server, nullptr, nullptr, building.get());

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
				str = gStrings.front();
				gStrings.pop_front();
				source->sendMessage(channel, *str);
				delete str;
			}
			while (nStrings.empty() == false) {
				str = nStrings.front();
				nStrings.pop_front();
				source->sendMessage(channel, *str);
				delete str;
			}
			while (oStrings.empty() == false) {
				str = oStrings.front();
				oStrings.pop_front();
				source->sendMessage(channel, *str);
				delete str;
			}
			while (cStrings.empty() == false) {
				str = cStrings.front();
				cStrings.pop_front();
				source->sendMessage(channel, *str);
				delete str;
			}
		}
	}
	if (str == nullptr)
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
}

const Jupiter::ReadableString &BuildingInfoIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Provides a list of buildings, and the status of each one. Syntax: BuildingInfo");
	return defaultHelp;
}

IRC_COMMAND_INIT(BuildingInfoIRCCommand)

// Mutators IRC Command

void MutatorsIRCCommand::create() {
	this->addTrigger("mutators"_jrs);
	this->addTrigger("mutator"_jrs);
}

void MutatorsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		Jupiter::String list;
		size_t index = 0;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				list = STRING_LITERAL_AS_REFERENCE(IRCCOLOR "03[Mutators]" IRCNORMAL);
				for (const auto& mutator : server->mutators) {
					list += " "_jrs + mutator;
				}

				if (server->mutators.empty()) {
					source->sendMessage(channel, "No mutators loaded"_jrs);
				}
				else {
					source->sendMessage(channel, list);
				}
			}
		}
		if (list.empty())
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
	}
}

const Jupiter::ReadableString &MutatorsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Provides a list of mutators being used. Syntax: Mutators");
	return defaultHelp;
}

IRC_COMMAND_INIT(MutatorsIRCCommand)

// Rotation IRC Command

void RotationIRCCommand::create()
{
	this->addTrigger("rotation"_jrs);
	this->addTrigger("maprotation"_jrs);
	this->addTrigger("maps"_jrs);
	this->addTrigger("rot"_jrs);
}

void RotationIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		Jupiter::String list;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type)) {
				list = STRING_LITERAL_AS_REFERENCE(IRCCOLOR "03[Rotation]" IRCNORMAL);
				for (const auto& map : server->maps) {
					if (jessilib::equalsi(server->getMap().name, map.name)) {
						list += STRING_LITERAL_AS_REFERENCE(" " IRCBOLD "[") + map.name + STRING_LITERAL_AS_REFERENCE("]" IRCBOLD);
					}
					else {
						list += " "_jrs + map.name;
					}
				}

				if (server->maps.empty()) {
					source->sendMessage(channel, "No maps in rotation"_jrs);
				}
				else {
					source->sendMessage(channel, list);
				}
			}
		}
		if (list.empty()) {
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
}

const Jupiter::ReadableString &RotationIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Provides a list of maps in the server rotation. Syntax: Rotation");
	return defaultHelp;
}

IRC_COMMAND_INIT(RotationIRCCommand)

// Map IRC Command

void MapIRCCommand::create() {
	this->addTrigger("map"_jrs);
}

void MapIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr) {
		int type = chan->getType();
		bool match = false;
		for (const auto& server : RenX::getCore()->getServers()) {
			if (server->isLogChanType(type)) {
				match = true;
				const RenX::Map &map = server->getMap();
				source->sendMessage(channel, "Current Map: "_jrs + map.name + "; GUID: "_jrs + RenX::formatGUID(map));
			}
		}
		if (match == false) {
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		}
	}
}

const Jupiter::ReadableString &MapIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Fetches the current map. Syntax: Map");
	return defaultHelp;
}

IRC_COMMAND_INIT(MapIRCCommand)

// GameInfo IRC Command

void GameInfoIRCCommand::create()
{
	this->addTrigger("gameinfo"_jrs);
	this->addTrigger("gi"_jrs);
	this->addTrigger("serverinfo"_jrs);
	this->addTrigger("si"_jrs);
}

void GameInfoIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
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
				source->sendMessage(channel, Jupiter::StringS::Format(IRCCOLOR "03[GameInfo] " IRCCOLOR "%.*s", server->getGameVersion().size(), server->getGameVersion().data()));
				source->sendMessage(channel, IRCCOLOR "03[GameInfo] " IRCCOLOR "10Map" IRCCOLOR ": "_jrs + map.name + "; " IRCCOLOR "10GUID" IRCCOLOR ": "_jrs + RenX::formatGUID(map));
				source->sendMessage(channel, Jupiter::StringS::Format(IRCCOLOR "03[GameInfo] " IRCCOLOR "10Elapsed time" IRCCOLOR ": %.2lld:%.2lld:%.2lld", time.count() / 3600, (time.count() % 3600) / 60, time.count() % 60));
				source->sendMessage(channel, Jupiter::StringS::Format(IRCCOLOR "03[GameInfo] " IRCCOLOR "There are " IRCCOLOR "10%d" IRCCOLOR " players online.", server->players.size()));
			}
		}
		if (match == false)
			source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
	}
}

const Jupiter::ReadableString &GameInfoIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Returns information about the game in progress. Syntax: GameInfo");
	return defaultHelp;
}

IRC_COMMAND_INIT(GameInfoIRCCommand)

// Steam IRC Command

void SteamIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("steam"));
	this->setAccessLevel(1);
}

void SteamIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		if (parameters.isNotEmpty())
		{
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					for (auto node = server->players.begin(); node != server->players.end(); ++node)
					{
						if (jessilib::findi(node->name, Jupiter::ReferenceString{parameters}) != std::string::npos)
						{
							Jupiter::String playerName = RenX::getFormattedPlayerName(*node);
							msg.format(IRCCOLOR "03[Steam] " IRCCOLOR "%.*s (ID: %d) ", playerName.size(),
								playerName.data(), node->id);
							if (node->steamid != 0)
							{
								msg += "is using steam ID " IRCBOLD;
								msg += server->formatSteamID(*node);
								msg.aformat(IRCBOLD "; Steam Profile: " IRCBOLD "https://steamcommunity.com/profiles/%llu" IRCBOLD, node->steamid);
							}
							else
								msg += "is not using steam.";

							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.empty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
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
						source->sendMessage(channel, Jupiter::StringS::Format("%.2f%% (%u/%u) of players are using Steam.", ((double)total * 100) / ((double)realPlayers), total, realPlayers));
					else
						source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("No players are in-game."));
				}
			}
		}
	}
}

const Jupiter::ReadableString &SteamIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Fetches steam usage information. Syntax: Steam [Player]");
	return defaultHelp;
}

IRC_COMMAND_INIT(SteamIRCCommand)

// Kill-Death Ratio IRC Command

void KillDeathRatioIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kills"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deaths"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kdr"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killdeathraio"));
}

void KillDeathRatioIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0)
				{
					for (auto node = server->players.begin(); node != server->players.end(); ++node)
					{
						if (jessilib::findi(node->name, Jupiter::ReferenceString{parameters}) != std::string::npos)
						{
							Jupiter::String playerName = RenX::getFormattedPlayerName(*node);
							msg.format(IRCBOLD "%.*s" IRCBOLD IRCCOLOR ": Kills: %u - Deaths: %u - KDR: %.2f", playerName.size(),
								playerName.data(), node->kills, node->deaths, static_cast<double>(node->kills) / (node->deaths == 0 ? 1.0f : static_cast<double>(node->deaths)));
							source->sendMessage(channel, msg);
						}
					}
				}
			}
			if (msg.empty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Kills <Player>"));
}

const Jupiter::ReadableString &KillDeathRatioIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets a player's kills and deaths. Syntax: Kills <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(KillDeathRatioIRCCommand)

// ShowMods IRC Command

void ShowModsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showstaff"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showmods"));
}

extern ModsGameCommand ModsGameCommand_instance;

void ShowModsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
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
				ModsGameCommand_instance.trigger(server, nullptr, ""_jrs);
				sent = true;
			}
		}
		if (sent == false)
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &ShowModsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying in-game staff. Syntax: showstaff");
	return defaultHelp;
}

IRC_COMMAND_INIT(ShowModsIRCCommand)

// Mods IRC Command

void ModsIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("staff"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mods"));
}

void ModsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.equalsi("show")) ShowModsIRCCommand_instance.trigger(source, channel, nick, parameters);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::StringL msg;
			const Jupiter::ReadableString &staff_word = pluginInstance.getStaffTitle();
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
			{
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type))
				{
					msg = "";
					if (server->players.size() != 0)
					{
						for (auto node = server->players.begin(); node != server->players.end(); ++node)
						{
							if (node->isBot == false && (node->adminType.isNotEmpty() || (node->access != 0 && (node->gamePrefix.isNotEmpty() || node->formatNamePrefix.isNotEmpty()))))
							{
								if (msg.isNotEmpty())
									msg += ", ";
								else msg += staff_word + "s in-game: "_jrs;
								msg += node->gamePrefix;
								msg += node->name;
							}
						}
					}
					if (msg.empty())
						msg = "No "_jrs + staff_word + "s are in-game."_jrs;
					source->sendMessage(channel, msg);
				}
			}
			if (msg.empty())
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
}

const Jupiter::ReadableString &ModsIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying in-game staff. Syntax: staff [show]");
	return defaultHelp;
}

IRC_COMMAND_INIT(ModsIRCCommand)

// ShowRules IRC Command

void ShowRulesIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showrules"));
}

void ShowRulesIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		Jupiter::StringL msg;
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
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &ShowRulesIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends a message, displaying the in-game rules. Syntax: showrules");
	return defaultHelp;
}

IRC_COMMAND_INIT(ShowRulesIRCCommand)

// Rules IRC Command

void RulesIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rules"));
}

void RulesIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.equalsi("show")) ShowRulesIRCCommand_instance.trigger(source, channel, nick, parameters);
	else
	{
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr)
		{
			int type = chan->getType();
			Jupiter::StringL msg;
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
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
}

const Jupiter::ReadableString &RulesIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays the in-game rules. Syntax: rules [show]");
	return defaultHelp;
}

IRC_COMMAND_INIT(RulesIRCCommand)

// Reconnect IRC Command

void ReconnectIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("reconnect"));
	this->setAccessLevel(3);
}

void ReconnectIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &)
{
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan != nullptr)
	{
		int type = chan->getType();
		Jupiter::StringS msg;
		for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++)
		{
			RenX::Server *server = RenX::getCore()->getServer(i);
			if (server->isLogChanType(type))
			{
				if (server->reconnect(RenX::DisconnectReason::Triggered)) msg.set("Connection established");
				else msg.format("[RenX] ERROR: Failed to connect to %.*s on port %u." ENDL, server->getHostname().size(), server->getHostname().c_str(), server->getPort());
				source->sendMessage(channel, msg);
			}
		}
		if (msg.empty())
		{
			// We didn't connect anywhere!!
			msg.set("ERROR: No servers found to connect to.");
			source->sendMessage(channel, msg);
		}
	}
}

const Jupiter::ReadableString &ReconnectIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets the RCON connection. Syntax: Reconnect");
	return defaultHelp;
}

IRC_COMMAND_INIT(ReconnectIRCCommand)

// GameOver IRC Command

void GameOverIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("gameover"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("endmap"));
	this->setAccessLevel(3);
}

void GameOverIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
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
				if (parameters.equalsi("empty"_jrs))
					server->gameoverWhenEmpty();
				else if (parameters.equalsi("if empty"_jrs))
				{
					if (server->players.size() == server->getBotCount())
						server->gameover();
				}
				else if (parameters.equalsi("now"_jrs))
					server->gameover();
				else if (parameters.equalsi("stop"_jrs) || parameters.equalsi("cancel"_jrs))
				{
					if (server->gameoverStop())
						server->sendMessage("Notice: The scheduled gameover has been cancelled."_jrs);
					else
						source->sendNotice(nick, "Error: There is no gameover scheduled."_jrs);
				}
				else
				{
					if (parameters.empty())
						delay = std::chrono::seconds(10);
					else
						delay = std::chrono::seconds(Jupiter::from_string<long long>(parameters));

					server->sendMessage(Jupiter::StringS::Format("Notice: This server will gameover in %lld seconds.", static_cast<long long>(delay.count())));
					server->gameover(delay);
				}
			}
		}
		if (match == false)
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &GameOverIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Forcefully ends the game in progress. Syntax: Gameover [NOW | STOP | [If] Empty | Seconds = 10]");
	return defaultHelp;
}

IRC_COMMAND_INIT(GameOverIRCCommand)

// SetMap IRC Command

void SetMapIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("setmap"));
	this->setAccessLevel(4);
}

void SetMapIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
						source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Map not in rotation."));
					else if (server->setMap(map_name) == false)
						source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Transmission error."));
				}
			}
			if (map_name == nullptr)
				source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too few parameters. Syntax: setmap <map>"));
}

const Jupiter::ReadableString &SetMapIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Ends the game immediately. Syntax: setmap <map>");
	return defaultHelp;
}

IRC_COMMAND_INIT(SetMapIRCCommand)

// Mute IRC Command

void MuteIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mute"));
	this->setAccessLevel(2);
}

void MuteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
						source->sendMessage(channel, RenX::getFormattedPlayerName(*player) + STRING_LITERAL_AS_REFERENCE(IRCCOLOR " has been muted."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: mute <player>"));
}

const Jupiter::ReadableString &MuteIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Mutes a player. Syntax: mute <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(MuteIRCCommand)

// UnMute IRC Command

void UnMuteIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unmute"));
	this->setAccessLevel(2);
}

void UnMuteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
						source->sendMessage(channel, RenX::getFormattedPlayerName(*player) + STRING_LITERAL_AS_REFERENCE(IRCCOLOR " has been unmuted."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: unmute <player>"));
}

const Jupiter::ReadableString &UnMuteIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Unmutes a player. Syntax: unmute <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(UnMuteIRCCommand)

// Kill IRC Command

void KillIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kill"));
	this->setAccessLevel(2);
}

void KillIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: kill <player>"));
}

const Jupiter::ReadableString &KillIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kills a player. Syntax: kill <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(KillIRCCommand)

// Disarm IRC Command

void DisarmIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarm"));
	this->setAccessLevel(2);
}

void DisarmIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("All deployables (c4, beacons, etc) belonging to ") + RenX::getFormattedPlayerName(*player) + STRING_LITERAL_AS_REFERENCE(IRCCOLOR " have been disarmed."));
						else
							source->sendMessage(channel, "Error: Server does not support disarms."_jrs);
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: disarm <player>"));
}

const Jupiter::ReadableString &DisarmIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed objects. Syntax: disarm <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmIRCCommand)

// DisarmC4 IRC Command

void DisarmC4IRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmc4"));
	this->setAccessLevel(2);
}

void DisarmC4IRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("All C4 belonging to ") + RenX::getFormattedPlayerName(*player) + STRING_LITERAL_AS_REFERENCE(IRCCOLOR " have been disarmed."));
						else
							source->sendMessage(channel, "Error: Server does not support disarms."_jrs);
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: disarmC4 <player>"));
}

const Jupiter::ReadableString &DisarmC4IRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed C4s. Syntax: disarmc4 <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmC4IRCCommand)

// DisarmBeacon IRC Command

void DisarmBeaconIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacons"));
	this->setAccessLevel(2);
}

void DisarmBeaconIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
							source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("All beacons belonging to ") + RenX::getFormattedPlayerName(*player) + STRING_LITERAL_AS_REFERENCE(IRCCOLOR " have been disarmed."));
						else
							source->sendMessage(channel, "Error: Server does not support disarms."_jrs);
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: disarmb <player>"));
}

const Jupiter::ReadableString &DisarmBeaconIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed beacons. Syntax: disarmb <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(DisarmBeaconIRCCommand)

// MineBan IRC Command

void MineBanIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mineban"));
	this->setAccessLevel(2);
}

void MineBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
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
						source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Player can no longer place mines."));
					}
					else
						source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
				}
			}
			if (match == false)
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: mineban <player>"));
}

const Jupiter::ReadableString &MineBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Bans a player from mining for 1 game (or until they leave). Syntax: mineban <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(MineBanIRCCommand)

// Kick IRC Command

void KickIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("qkick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("k"));
	this->setAccessLevel(2);
}

void KickIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: Kick <Player> [Reason]"));
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	auto servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
}

const Jupiter::ReadableString &KickIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks a player from the game. Syntax: Kick <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickIRCCommand)

/** Ban IRC Commands */

// BanSearch IRC Command

void BanSearchIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bansearch"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bsearch"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("banfind"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bfind"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("banlogs"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("blogs"));
	this->setAccessLevel(2);
}

void BanSearchIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	const auto& entries = RenX::banDatabase->getEntries();
	if (!parameters.empty()) {
		if (entries.size() == 0) {
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("The ban database is empty!"));
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

			Jupiter::String out(256);
			Jupiter::String types(64);
			char dateStr[256];
			char expireStr[256];
			for (size_t i = 0; i != entries.size(); i++) {
				entry = entries[i].get();
				if (isMatch(type)) {
					Jupiter::StringS ip_str = Jupiter::Socket::ntop4(entry->ip);

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
						types = " NULL;"_jrs;
					else
					{
						types.erase();
						if (entry->is_rdns_ban())
							types += " rdns"_jrs;
						if (entry->is_type_game())
							types += " game"_jrs;
						if (entry->is_type_chat())
							types += " chat"_jrs;
						if (entry->is_type_bot())
							types += " bot"_jrs;
						if (entry->is_type_vote())
							types += " vote"_jrs;
						if (entry->is_type_mine())
							types += " mine"_jrs;
						if (entry->is_type_ladder())
							types += " ladder"_jrs;
						if (entry->is_type_alert())
							types += " alert"_jrs;
						types += ";"_jrs;
					}

					out.format("ID: %lu (" IRCCOLOR "%sactive" IRCCOLOR "); Added: %s; Expires: %s; IP: %.*s/%u; HWID: %.*s; Steam: %llu; Types:%.*s Name: %.*s; Banner: %.*s",
						i, entry->is_active() ? "12" : "04in", dateStr, expireStr, ip_str.size(), ip_str.data(), entry->prefix_length, entry->hwid.size(), entry->hwid.data(), entry->steamid,
						types.size(), types.data(), entry->name.size(), entry->name.data(), entry->banner.size(), entry->banner.data());

					if (!entry->rdns.empty())
					{
						out.concat("; RDNS: "_jrs);
						out.concat(entry->rdns);
					}
					if (!entry->reason.empty())
					{
						out.concat("; Reason: "_jrs);
						out.concat(entry->reason);
					}
					source->sendNotice(nick, out);
				}
			}
			if (out.empty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("No matches found."));
		}
	}
	else
		source->sendNotice(nick, Jupiter::StringS::Format("There are a total of %u entries in the ban database.", entries.size()));
}

const Jupiter::ReadableString &BanSearchIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Searches the ban database for an entry. Syntax: bsearch [ip/rdns/steam/name/banner/active/any/all = any] <player ip/steam/name/banner>");
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
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tempban"));
	this->setAccessLevel(3);
}

void TempBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: TempBan [Duration] <Player> [Reason]"));
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
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	if (search_result.player == nullptr) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Could not find player."));
		return;
	}

	if (reason.empty()) {
		reason = "No reason"sv;
	}

	Jupiter::String banner(nick.size() + 4);
	banner += nick;
	banner += "@IRC";
	search_result.server->banPlayer(*search_result.player, banner, reason, duration);
	source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Player banned."));
}

const Jupiter::ReadableString &TempBanIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and temporarily bans a player from the game. Syntax: TempBan [Duration] <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(TempBanIRCCommand)

// TempChatBan IRC Command

void TempChatBanIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tchatban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tcban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tempchatban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tcb"));
	this->setAccessLevel(3);
}

void TempChatBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: TempChatBan [Duration] <Player> [Reason]"));
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
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	if (search_result.player == nullptr) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Could not find player."));
		return;
	}

	if (reason.empty()) {
		reason = "No reaosn"sv;
	}

	Jupiter::String banner(nick.size() + 4);
	banner += nick;
	banner += "@IRC";
	RenX::banDatabase->add(search_result.server, *search_result.player, banner, reason, duration, RenX::BanDatabase::Entry::FLAG_TYPE_CHAT);
	source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Player chat banned."));
}

const Jupiter::ReadableString &TempChatBanIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Mutes and temporarily chat bans a player from the game. Syntax: TempChatBan [Duration] <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(TempChatBanIRCCommand)

// KickBan IRC Command

void KickBanIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kickban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ban"));
	this->setAccessLevel(4);
}

void KickBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
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

				Jupiter::String banner(nick.size() + 4);
				banner += nick;
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
					source->sendMessage(channel, Jupiter::StringS::Format("%u players kicked.", kicks));
					RenX::getCore()->banCheck();
				}
			}
			else source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: KickBan <Player> [Reason]"));
}

const Jupiter::ReadableString &KickBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and bans a player from the game. Syntax: KickBan <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickBanIRCCommand)

// AddBan IRC Command

void AddBanIRCCommand::create()
{
	this->addTrigger("addban"_jrs);
	this->addTrigger("banadd"_jrs);
	this->setAccessLevel(4);
}

void AddBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
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
					Jupiter::ReferenceString prefix_length_str(ip_str.c_str() + index + 1);
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
				source->sendMessage(channel, Jupiter::StringS::Format("Ban added to the database with ID #%u", RenX::banDatabase->getEntries().size() - 1));
			}
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: AddBan <Key> <Value> [...]"));
}

const Jupiter::ReadableString &AddBanIRCCommand::getHelp(const Jupiter::ReadableString &parameters)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds a ban entry to the ban list. Use \"help addban keys\" for a list of input keys. Syntax: AddBan <Key> <Value> [<Key> <Value> ...]");
	static STRING_LITERAL_AS_NAMED_REFERENCE(keyHelp, "Valueless keys (flags): Game, Chat, Bot, Vote, Mine, Ladder, Alert; Value-paired keys: Name, IP, Steam, RDNS, Duration, Reason (MUST BE LAST)");
	if (parameters.isNotEmpty() && parameters.equalsi("keys"_jrs))
		return keyHelp;
	return defaultHelp;
}

IRC_COMMAND_INIT(AddBanIRCCommand)

// UnBan IRC Command

void UnBanIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("uban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("dban"));
	this->setAccessLevel(4);
}

void UnBanIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		size_t index = Jupiter::from_string<size_t>(parameters);
		if (index < RenX::banDatabase->getEntries().size())
		{
			if (RenX::banDatabase->deactivate(index))
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Ban deactivated."));
			else
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Ban not active."));
		}
		else
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid ban ID; please find the ban ID using \"bansearch\"."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: unban <Ban ID>"));
}

const Jupiter::ReadableString &UnBanIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Deactivates a ban. Syntax: unban <Ban ID>");
	return defaultHelp;
}

IRC_COMMAND_INIT(UnBanIRCCommand)

/** Exemption IRC Commands */

// ExemptionSearch IRC Command

void ExemptionSearchIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("exemptionsearch"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("esearch"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("exemptionfind"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("efind"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("exemptionlogs"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("elogs"));
	this->setAccessLevel(2);
}

void ExemptionSearchIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	const auto& entries = RenX::exemptionDatabase->getEntries();
	if (parameters.isNotEmpty())
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

			Jupiter::String out(256);
			Jupiter::String types(64);
			char timeStr[256];
			for (size_t i = 0; i != entries.size(); i++)
			{
				entry = entries[i].get();
				if (isMatch(type))
				{
					time_t current_time = std::chrono::system_clock::to_time_t(entry->timestamp);
					Jupiter::StringS ip_str = Jupiter::Socket::ntop4(entry->ip);
					strftime(timeStr, sizeof(timeStr), "%b %d %Y, %H:%M:%S", localtime(&current_time));

					if ((entry->flags & 0xFF) == 0)
						types = " NULL;"_jrs;
					else
					{
						types.erase();
						if (entry->is_type_kick())
							types += " kick"_jrs;
						if (entry->is_type_ban())
							types += " ban"_jrs;
						if (entry->is_ip_exemption())
							types += " ip"_jrs;
						types += ";"_jrs;
					}

					out.format("ID: %lu (%sactive); Date: %s; IP: %.*s/%u; Steam: %llu; Types:%.*s Setter: %.*s",
						i, entry->is_active() ? "" : "in", timeStr, ip_str.size(), ip_str.data(), entry->prefix_length, entry->steamid,
						types.size(), types.data(), entry->setter.size(), entry->setter.data());

					source->sendNotice(nick, out);
				}
			}
			if (out.empty())
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("No matches found."));
		}
	}
	else
		source->sendNotice(nick, Jupiter::StringS::Format("There are a total of %u entries in the exemption database.", entries.size()));
}

const Jupiter::ReadableString &ExemptionSearchIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Searches the exemption database for an entry. Syntax: esearch [ip/steam/setter/active/any/all = any] <player ip/steam/setter>");
	return defaultHelp;
}

IRC_COMMAND_INIT(ExemptionSearchIRCCommand)

// BanExempt IRC Command

void BanExemptIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("banexempt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("bexempt"));
	this->setAccessLevel(4);
}

void BanExemptIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	std::string_view target_name = command_split.first;
	if (target_name.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: BanExempt <Player> [Reason]"));
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	RenX::PlayerInfo *player;
	unsigned int exemptions = 0;
	Jupiter::String setter(nick.size() + 4);
	setter += nick;
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
		source->sendMessage(channel, Jupiter::StringS::Format("%u players added.", exemptions));
	}
}

const Jupiter::ReadableString &BanExemptIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Exempts a player from bans using their SteamID, or their IP address if they have none. Syntax: BanExempt <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(BanExemptIRCCommand)

// KickExempt IRC Command

void KickExemptIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kickexempt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kexempt"));
	this->setAccessLevel(4);
}

void KickExemptIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	auto command_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	std::string_view target_name = command_split.first;
	if (target_name.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: KickExempt <Player> [Reason]"));
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	RenX::PlayerInfo *player;
	unsigned int exemptions = 0;
	Jupiter::String setter(nick.size() + 4);
	setter += nick;
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
		source->sendMessage(channel, Jupiter::StringS::Format("%u players added.", exemptions));
	}
}

const Jupiter::ReadableString &KickExemptIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Exempts a player from kicks and bans using their SteamID, or their IP address if they have none. Syntax: KickExempt <Player> [Reason]");
	return defaultHelp;
}

IRC_COMMAND_INIT(KickExemptIRCCommand)

// AddExemption IRC Command

#define ADDEXEMPTION_WHITESPACE " \t="

void AddExemptionIRCCommand::create()
{
	this->addTrigger("addexemption"_jrs);
	this->addTrigger("exemptionadd"_jrs);
	this->addTrigger("exempt"_jrs);
	this->setAccessLevel(4);
}

void AddExemptionIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
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
				Jupiter::String setter = nick + "@IRC"_jrs;
				std::chrono::seconds duration = std::chrono::seconds::zero();
				uint8_t flags = 0;

				auto missing_value = [&source, &nick](std::string_view token) {
					std::string error_message = "ERROR: No value specified for token: "s;
					error_message += token;
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
						Jupiter::ReferenceString prefix_length_str(ip_str.c_str() + index + 1);
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
					source->sendNotice(nick, "Pointless exemption detected -- no IP or SteamID specified"_jrs);
				else
				{
					RenX::exemptionDatabase->add(ip, prefix_length, steamid, setter, duration, flags);
					source->sendMessage(channel, Jupiter::StringS::Format("Exemption added to the database with ID #%u", RenX::exemptionDatabase->getEntries().size() - 1));
				}
			}
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: AddExemption <Key> <Value> [...]"));
}

const Jupiter::ReadableString &AddExemptionIRCCommand::getHelp(const Jupiter::ReadableString &parameters)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds an exemption entry to the exemption list. Use \"help addexemption keys\" for a list of input keys. Syntax: AddExemption <Key> <Value> [<Key> <Value> ...]");
	static STRING_LITERAL_AS_NAMED_REFERENCE(keyHelp, "Valueless keys (flags): Ban, Kick; Value-paired keys: IP, Steam, Duration");
	if (parameters.isNotEmpty() && parameters.equalsi("keys"_jrs))
		return keyHelp;
	return defaultHelp;
}

IRC_COMMAND_INIT(AddExemptionIRCCommand)

// UnExempt IRC Command

void UnExemptIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("unexempt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deexempt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("uexempt"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("dexempt"));
	this->setAccessLevel(4);
}

void UnExemptIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.isNotEmpty())
	{
		size_t index = Jupiter::from_string<size_t>(parameters);
		if (index < RenX::exemptionDatabase->getEntries().size())
		{
			if (RenX::exemptionDatabase->deactivate(index))
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Exemption deactivated."));
			else
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Exemption not active."));
		}
		else
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid exemption ID; please find the exemption ID using \"esearch\"."));
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: unexempt <Exemption ID>"));
}

const Jupiter::ReadableString &UnExemptIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Deactivates an exemption. Syntax: unexempt <Exemption ID>");
	return defaultHelp;
}

IRC_COMMAND_INIT(UnExemptIRCCommand)

// AddBots IRC Command

void AddBotsIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbot"));
	this->setAccessLevel(2);
}

void AddBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(source->getChannel(channel)->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	int amount = 1;
	std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.empty()) {
		amount = Jupiter::asInt(split_parameters.front());
		if (amount == 0) {
			source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Invalid amount entered. Amount must be a positive integer."));
			return;
		}
	}

	Jupiter::StringL cmd;
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
		if (server != nullptr) {
			size_t extra = cmd.aformat("%u", amount);
			server->send(cmd);
			cmd -= extra;
		}
		server->sendMessage(Jupiter::StringS::Format("%u bots have been added to the server.", amount));
	}
}

const Jupiter::ReadableString &AddBotsIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds bots to the game. Syntax: AddBots [Amount=1] [Team]");
	return defaultHelp;
}

IRC_COMMAND_INIT(AddBotsIRCCommand)

// KillBots IRC Command

void KillBotsIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killbot"));
	this->setAccessLevel(2);
}

void KillBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	for (const auto& server : servers) {
		server->send(STRING_LITERAL_AS_REFERENCE("killbots"));
		server->sendMessage(STRING_LITERAL_AS_REFERENCE("All bots have been removed from the server."));
	}
}

const Jupiter::ReadableString &KillBotsIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes all bots from the game. Syntax: KillBots");
	return defaultHelp;
}

IRC_COMMAND_INIT(KillBotsIRCCommand)

// PhaseBots IRC Command

void PhaseBotsIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("phasebots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("phasebot"));
	this->setAccessLevel(2);
}

void PhaseBotsIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	for (const auto& server : servers) {
		if (parameters.empty()) {
			if (togglePhasing(server)) {
				server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
			}
			else {
				server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
			}
		}
		else if (parameters.equalsi("true") || parameters.equalsi("on") || parameters.equalsi("start") || parameters.equalsi("1")) {
			togglePhasing(server, true);
			server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
		}
		else {
			togglePhasing(server, false);
			server->sendMessage(STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
		}
	}
}

const Jupiter::ReadableString &PhaseBotsIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Toggles the phasing of bots from the game by kicking them after death. Syntax: PhaseBots [on/off]");
	return defaultHelp;
}

IRC_COMMAND_INIT(PhaseBotsIRCCommand)

// RCON IRC Command

void RCONIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rcon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("renx"));
	this->setAccessLevel(5);
}

void RCONIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: rcon <input>"));
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	size_t server_count = RenX::getCore()->send(chan->getType(), parameters);
	if (server_count > 0) {
		source->sendMessage(channel, Jupiter::StringS::Format("Command sent to %u servers.", server_count));
	}
	else {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
	}
}

const Jupiter::ReadableString &RCONIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends data to the Renegade X server's rcon. Syntax: rcon <input>");
	return defaultHelp;
}

IRC_COMMAND_INIT(RCONIRCCommand)

// Refund IRC Command

void RefundIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("refund"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("givecredits"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("gc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("money"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("credits"));
	this->setAccessLevel(3);
}

void RefundIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, WHITESPACE_SV);
	if (split_parameters.size() >= 2) {
		Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
		if (chan != nullptr) {
			int type = chan->getType();
			std::string_view playerName = split_parameters[0];
			double credits = Jupiter::asDouble(split_parameters[1]);
			RenX::PlayerInfo *player;
			Jupiter::StringL msg;
			for (unsigned int i = 0; i != RenX::getCore()->getServerCount(); i++) {
				RenX::Server *server = RenX::getCore()->getServer(i);
				if (server->isLogChanType(type) && server->players.size() != 0) {
					player = server->getPlayerByPartName(playerName);
					if (player != nullptr) {
						if (server->giveCredits(*player, credits)) {
							msg.format("You have been refunded %.0f credits by %.*s.", credits, nick.size(), nick.data());
							server->sendMessage(*player, msg);
							msg.format("%.*s has been refunded %.0f credits.", player->name.size(), player->name.data(), credits);
						}
						else {
							msg.set("Error: Server does not support refunds.");
						}
						source->sendMessage(channel, msg);
					}
				}
			}
			if (msg.empty()) {
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
			}
		}
	}
	else {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: refund <player> <amount>"));
	}
}

const Jupiter::ReadableString &RefundIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Refunds a player's credits. Syntax: refund <player> <amount>");
	return defaultHelp;
}

IRC_COMMAND_INIT(RefundIRCCommand)

// Team-Change IRC Command

void TeamChangeIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("team"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ftc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forcetc"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("teamchange"));
	this->setAccessLevel(3);
}

void TeamChangeIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
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
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: team <player>"));
}

const Jupiter::ReadableString &TeamChangeIRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Changes a player's team. Syntax: team <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(TeamChangeIRCCommand)

// TeamChange2 IRC Command

void TeamChange2IRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("team2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tc2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ftc2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("forcetc2"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("teamchange2"));
	this->setAccessLevel(3);
}

void TeamChange2IRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
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
				source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Player not found."));
		}
	}
	else source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: team2 <player>"));
}

const Jupiter::ReadableString &TeamChange2IRCCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Changes a player's team, without resetting their credits. Syntax: team2 <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(TeamChange2IRCCommand)

// NMode IRC Command

void NModeIRCCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("nmode"));
	this->setAccessLevel(2);
}

void NModeIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters)
{
	if (parameters.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: nmode <Player>"));
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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

	source->sendMessage(channel, Jupiter::StringS::Format("%u players nmoded.", nmodes));
}

const Jupiter::ReadableString &NModeIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's mode from spectator to normal. Syntax: nmode <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(NModeIRCCommand)

// SMode IRC Command

void SModeIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("smode"));
	this->setAccessLevel(2);
}

void SModeIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Too Few Parameters. Syntax: smode <Player>"));
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
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
	source->sendMessage(channel, Jupiter::StringS::Format("%u players smoded.", smodes));
}

const Jupiter::ReadableString &SModeIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's mode from spectator to normal. Syntax: smode <player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(SModeIRCCommand)

// CancelVote IRC Command

void CancelVoteIRCCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("cancelvote"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("cancelvotes"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("cv"));
	this->setAccessLevel(2);
}

void CancelVoteIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, STRING_LITERAL_AS_REFERENCE("Error: Channel not attached to any connected Renegade X servers."));
		return;
	}

	bool cancel_all = false;
	RenX::TeamType target = RenX::TeamType::None;

	if (parameters.empty()) {
		cancel_all = true;
	} else {
		if (parameters.equalsi("all") || parameters.equalsi("a")) {
			cancel_all = true;
		} else if (parameters.equalsi("public") || parameters.equalsi("p")) {
			target = RenX::TeamType::None;
		} else if (parameters.equalsi("gdi") || parameters.equalsi("g")) {
			target = RenX::TeamType::GDI;
		} else if (parameters.equalsi("blackhand") || parameters.equalsi("bh") || parameters.equalsi("b")) {
			target = RenX::TeamType::GDI;
		} else if (parameters.equalsi("nod") || parameters.equalsi("n")) {
			target = RenX::TeamType::Nod;
		} else {
			source->sendNotice(nick, STRING_LITERAL_AS_REFERENCE("Error: Invalid Team. Allowed values are all/a, public/p, gdi/g, nod/n, blackhand/bh/b."));
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

const Jupiter::ReadableString &CancelVoteIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Cancels active votes. Syntax: cancelvote [all|public|gdi|nod|blackhand]");
	return defaultHelp;
}

IRC_COMMAND_INIT(CancelVoteIRCCommand)

/** Game Commands */

// Help Game Command

void HelpGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("help"));
}

void HelpGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
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
				source->sendMessage(*player, cmd->getHelp(Jupiter::ReferenceString{split_parameters.second}));
			}
			else {
				source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Access Denied."));
			}
		}
		else {
			source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Error: Command not found."));
		}
	}
}

const Jupiter::ReadableString &HelpGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Lists commands, or sends command-specific help. Syntax: help [command]");
	return defaultHelp;
}

GAME_COMMAND_INIT(HelpGameCommand)

// Mods Game Command

void ModsGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("staff"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mods"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showmods"));
}

void ModsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *, const Jupiter::ReadableString &) {
	Jupiter::StringL msg;
	const Jupiter::ReadableString &staff_word = pluginInstance.getStaffTitle();
	for (auto node = source->players.begin(); node != source->players.end(); ++node) {
		if (node->isBot == false && (node->adminType.isNotEmpty() || (node->access != 0 && (node->gamePrefix.isNotEmpty() || node->formatNamePrefix.isNotEmpty())))) {
			if (msg.empty())
				msg = staff_word + "s in-game: "_jrs;
			else
				msg += ", ";

			msg += node->gamePrefix;
			msg += node->name;
		}
	}
	if (msg.empty()) {
		msg += "No "_jrs + staff_word + "s are in-game"_jrs;
		RenX::GameCommand *cmd = source->getCommand(STRING_LITERAL_AS_REFERENCE("modrequest"));
		if (cmd != nullptr)
			msg.aformat("; please use \"%.*s%.*s\" if you require assistance.", source->getCommandPrefix().size(), source->getCommandPrefix().data(), cmd->getTrigger().size(), cmd->getTrigger().data());
		else msg += '.';
	}
	source->sendMessage(msg);
}

const Jupiter::ReadableString &ModsGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays in-game staff. Syntax: staff");
	return defaultHelp;
}

GAME_COMMAND_INIT(ModsGameCommand)

// Rules Game Command

void RulesGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rules"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rule"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showrules"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showrule"));
}

void RulesGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	source->sendMessage(Jupiter::StringS::Format("Rules: %.*s", source->getRules().size(), source->getRules().data()));
}

const Jupiter::ReadableString &RulesGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays the rules for this server. Syntax: rules");
	return defaultHelp;
}

GAME_COMMAND_INIT(RulesGameCommand)

// Mod Request Game Command

void ModRequestGameCommand::create() {
	this->addTrigger("modrequest"_jrs);
	this->addTrigger("requestmod"_jrs);
	this->addTrigger("mod"_jrs);
}

void ModRequestGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendMessage(*player, "Please specify a reason for requesting moderator assistance."_jrs);
		return;
	}

	const Jupiter::ReadableString &staff_word = pluginInstance.getStaffTitle();
	Jupiter::String fmtName = RenX::getFormattedPlayerName(*player);
	Jupiter::StringL user_message = Jupiter::StringL::Format(IRCCOLOR "12[%.*s Request] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR "07 has requested assistance in-game for \"%.*s\"; please look in ", staff_word.size(),
		staff_word.data(), fmtName.size(), fmtName.data(), parameters.size(),
		parameters.data());
	Jupiter::StringS channel_message = Jupiter::StringS::Format(IRCCOLOR "12[%.*s Request] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR "07 has requested assistance in-game! Reason: %.*s" IRCCOLOR, staff_word.size(),
		staff_word.data(), fmtName.size(), fmtName.data(), parameters.size(),
		parameters.data());

	// Alerts a channel and all relevant users in the channel
	auto alert_channel = [&user_message, &channel_message](Jupiter::IRC::Client& server, const Jupiter::IRC::Client::Channel& channel) {
		// Alert channel
		server.sendMessage(channel.getName(), channel_message);

		// Alert relevant users in the channel
		unsigned int total_user_alerts{};
		user_message += channel.getName();

		for (auto& user : channel.getUsers()) {
			if (channel.getUserPrefix(*user.second) != 0 // If the user has a prefix...
				&& user.second->getNickname() != server.getNickname()) { // And the user isn't this bot...
				// Alert the user
				server.sendMessage(user.second->getNickname(), user_message);
				++total_user_alerts;
			}
		}

		user_message.truncate(channel.getName().size());
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
	source->sendMessage(*player, Jupiter::StringS::Format("A total of %u %.*ss have been notified of your assistance request.", total_user_alerts, staff_word.size(),
		staff_word.data()));
}

const Jupiter::ReadableString &ModRequestGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Notifies staff on IRC that assistance is required. Syntax: modRequest <reason>");
	return defaultHelp;
}

GAME_COMMAND_INIT(ModRequestGameCommand)

// AdminMessage Game Command

void AdminMessageGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("amsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("adminmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("amessage"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("adminmessage"));
	this->setAccessLevel(1);
}

void AdminMessageGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
		Jupiter::StringS msg = player->gamePrefix + player->name + ": "_jrs + parameters;
		source->sendAdminMessage(msg);
	}
	else {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: amsg <player> <message>"_jrs);
	}
}

const Jupiter::ReadableString &AdminMessageGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends an admin message in-game. Syntax: amsg <message>");
	return defaultHelp;
}

GAME_COMMAND_INIT(AdminMessageGameCommand)

// PAdminMessage Game Command

void PAdminMessageGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pamsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("padminmsg"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pamessage"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("padminmessage"));
	this->setAccessLevel(1);
}

void PAdminMessageGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.second.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."_jrs);
		}
		else {
			std::string message;
			message += std::string_view{player->gamePrefix};
			message += player->name;
			message += ": "sv;
			message += split_parameters.second;

			source->sendAdminMessage(*target, message);
			source->sendMessage(*player, "Message sent to "_jrs + target->name);
		}
	}
	else {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: pamsg <player> <message>"_jrs);
	}
}

const Jupiter::ReadableString &PAdminMessageGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sends an admin message to a player in-game. Syntax: pamsg <player> <message>");
	return defaultHelp;
}

GAME_COMMAND_INIT(PAdminMessageGameCommand)

// Kill Game Command

void KillGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kill"));
	this->setAccessLevel(1);
}

void KillGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."_jrs);
		}
		else if (target->access >= player->access) {
			source->sendMessage(*player, "Error: You can not kill higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		}
		else {
			source->kill(*target);
			source->sendMessage(*player, "Player has been killed."_jrs);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: kill <player>"_jrs);
}

const Jupiter::ReadableString &KillGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kills a player in the game. Syntax: kill <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(KillGameCommand)

// Disarm Game Command

void DisarmGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarm"));
	this->setAccessLevel(1);
}

void DisarmGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can not disarm higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else if (source->disarm(*target) == false)
			source->sendMessage(*player, "Error: Server does not support disarms."_jrs);
		else
			source->sendMessage(*player, "Player has been disarmed."_jrs);
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: disarm <player>"_jrs);
}

const Jupiter::ReadableString &DisarmGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed objects in the game. Syntax: disarm <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmGameCommand)

// DisarmC4 Game Command

void DisarmC4GameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmc4"));
	this->setAccessLevel(1);
}

void DisarmC4GameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can not disarm higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else if (source->disarmC4(*target) == false)
			source->sendMessage(*player, "Error: Server does not support disarms."_jrs);
		else
			source->sendMessage(*player, "Player has been disarmed."_jrs);
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: disarmc4 <player>"_jrs);
}

const Jupiter::ReadableString &DisarmC4GameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed mines in the game. Syntax: disarmc4 <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmC4GameCommand)

// DisarmBeacon Game Command

void DisarmBeaconGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacon"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("disarmbeacons"));
	this->setAccessLevel(1);
}

void DisarmBeaconGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can not disarm higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else if (source->disarmBeacon(*target) == false)
			source->sendMessage(*player, "Error: Server does not support disarms."_jrs);
		else
			source->sendMessage(*player, "Player has been disarmed."_jrs);
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: disarmb <player>"_jrs);
}

const Jupiter::ReadableString &DisarmBeaconGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Disarms all of a player's deployed beacons in the game. Syntax: disarmb <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(DisarmBeaconGameCommand)

// MineBan Game Command

void MineBanGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mineban"));
	this->setAccessLevel(1);
}

void MineBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty())
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can not mine-ban higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else
		{
			source->mineBan(*target);
			source->sendMessage(*player, "Player can no longer place mines."_jrs);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: mineban <player>"_jrs);
}

const Jupiter::ReadableString &MineBanGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Bans a player from mining for 1 game (or until they leave). Syntax: mineban <player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(MineBanGameCommand)

// Kick Game Command

void KickGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("qkick"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("k"));
	this->setAccessLevel(1);
}

void KickGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view reason = split_parameters.second;
		if (reason.empty()) {
			reason = "No reason"sv;
		}

		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."_jrs);
		}
		else if (player == target) {
			source->sendMessage(*player, "Error: You cannot kick yourself."_jrs);
		}
		else if (target->access >= player->access) {
			source->sendMessage(*player, "Error: You can not kick higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		}
		else
		{
			source->kickPlayer(*target, reason);
			source->sendMessage(*player, "Player has been kicked from the game."_jrs);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: kick <player> [Reason]"_jrs);
}

const Jupiter::ReadableString &KickGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks a player from the game. Syntax: kick <player> [Reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(KickGameCommand)

// Mute Game Command

void MuteGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("mute"));
	this->setAccessLevel(1);
}

void MuteGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view reason = split_parameters.second;
		if (reason.empty()) {
			reason = "No reason"sv;
		}
		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr)
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (player == target)
			source->sendMessage(*player, "Error: You cannot mute yourself."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can not mute higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else
		{
			source->mute(*target);
			source->sendMessage(*target, "You have been muted for: "s + std::string{reason});
			source->sendMessage(*player, "Player has been muted from chat."_jrs);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: mute <player> [Reason]"_jrs);
}

const Jupiter::ReadableString &MuteGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Mutes a player from chat. Syntax: mute <player> [Reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(MuteGameCommand)

// TempBan Game Command

void TempBanGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tempban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tb"));
	this->setAccessLevel(1);
}

void TempBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
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
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (player == target)
			source->sendMessage(*player, "Error: You can't ban yourself."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can't ban higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else
		{
			source->banPlayer(*target, player->name, reason, duration);
			source->sendMessage(*player, "Player has been temporarily banned and kicked from the game."_jrs);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: tban [Duration] <player> [Reason]"_jrs);
}

const Jupiter::ReadableString &TempBanGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and temporarily bans a player from the game. Syntax: tban [Duration] <player> [Reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(TempBanGameCommand)

// TempChatBan Game Command

void TempChatBanGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tchatban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tcban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tempchatban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("tcb"));
	this->setAccessLevel(1);
}

void TempChatBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
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
			source->sendMessage(*player, "Error: Player not found."_jrs);
		else if (player == target)
			source->sendMessage(*player, "Error: You can not ban yourself."_jrs);
		else if (target->access >= player->access)
			source->sendMessage(*player, "Error: You can not ban higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		else {
			source->mute(*target);
			RenX::banDatabase->add(source, *target, player->name, reason, duration, RenX::BanDatabase::Entry::FLAG_TYPE_CHAT);
			source->sendMessage(*player, "Player has been temporarily muted and chat banned from the game."_jrs);
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: tchatban [Duration] <player> [Reason]"_jrs);
}

const Jupiter::ReadableString &TempChatBanGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Mutes and temporarily chat bans a player from the game. Syntax: tchatban [Duration] <player> [Reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(TempChatBanGameCommand)

// KickBan Game Command

void KickBanGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("ban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kickban"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kb"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("b"));
	this->setAccessLevel(2);
}

void KickBanGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	auto split_parameters = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!split_parameters.first.empty()) {
		std::string_view reason = split_parameters.second;
		if (reason.empty()) {
			reason = "No reason"sv;
		}
		RenX::PlayerInfo *target = source->getPlayerByPartName(split_parameters.first);
		if (target == nullptr) {
			source->sendMessage(*player, "Error: Player not found."_jrs);
		}
		else if (player == target) {
			source->sendMessage(*player, "Error: You can not ban yourself."_jrs);
		}
		else if (target->access >= player->access) {
			source->sendMessage(*player, "Error: You can not ban higher level "_jrs + pluginInstance.getStaffTitle() + "s."_jrs);
		}
		else {
			source->banPlayer(*target, player->name, reason);
			source->sendMessage(*player, "Player has been banned and kicked from the game."_jrs);
			RenX::getCore()->banCheck();
		}
	}
	else {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: ban <player> [reason]"_jrs);
	}
}

const Jupiter::ReadableString &KickBanGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Kicks and bans a player from the game. Syntax: ban <player> [reason]");
	return defaultHelp;
}

GAME_COMMAND_INIT(KickBanGameCommand)

// AddBots Game Command

void AddBotsGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("abots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("addbot"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("abot"));
	this->setAccessLevel(1);
}

void AddBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	std::vector<std::string_view> split_parameters = jessilib::word_split_view(std::string_view{parameters}, WHITESPACE_SV);
	RenX::TeamType team = RenX::TeamType::None;
	if (split_parameters.size() >= 2) {
		team = RenX::getTeam(split_parameters[1]);
	}

	Jupiter::StringS cmd;
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

	cmd += Jupiter::StringS::Format("%u", amount);

	source->send(cmd);
	source->sendMessage(*player, Jupiter::StringS::Format("%u bots have been added to the server.", amount));
}

const Jupiter::ReadableString &AddBotsGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Adds bots to the game. Syntax: addbots [amount=1] [team]");
	return defaultHelp;
}

GAME_COMMAND_INIT(AddBotsGameCommand)

// KillBots Game Command

void KillBotsGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("killbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("kbots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rembots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rbots"));
	this->setAccessLevel(2);
}

void KillBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	source->send(STRING_LITERAL_AS_REFERENCE("killbots"));
	source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("All bots have been removed from the server."));
}

const Jupiter::ReadableString &KillBotsGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes all bots from the game. Syntax: killbots");
	return defaultHelp;
}

GAME_COMMAND_INIT(KillBotsGameCommand)

// PhaseBots Game Command

void PhaseBotsGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("phasebots"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("pbots"));
	this->setAccessLevel(1);
}

void PhaseBotsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.empty())
	{
		if (togglePhasing(source))
			source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
		else source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
	}
	else if (parameters.equalsi("true") || parameters.equalsi("on") || parameters.equalsi("start") || parameters.equalsi("1"))
	{
		togglePhasing(source, true);
		source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been enabled."));
	}
	else
	{
		togglePhasing(source, false);
		source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Bot phasing has been disabled."));
	}
}

const Jupiter::ReadableString &PhaseBotsGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes all bots from the game. Syntax: phasebots");
	return defaultHelp;
}

GAME_COMMAND_INIT(PhaseBotsGameCommand)

// NMode Game Command

void NModeGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("nmode"));
	this->setAccessLevel(1);
}

void NModeGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: nmode <player-name>"_jrs);
		return;
	}

	RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
	if (target == nullptr) {
		source->sendMessage(*player, "Error: Player not found."_jrs);
		return;
	}

	if (!source->nmodePlayer(*target)) {
		source->sendMessage(*player, "Error: Could not set player's mode."_jrs);
		return;
	}

	source->sendMessage(*player, "Player's mode has been reset."_jrs);
}

const Jupiter::ReadableString &NModeGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's mode from spectator to normal. Syntax: nmode <player-name>");
	return defaultHelp;
}

GAME_COMMAND_INIT(NModeGameCommand)

// SMode Game Command

void SModeGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("smode"));
	this->setAccessLevel(1);
}

void SModeGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		source->sendMessage(*player, "Error: Too few parameters. Syntax: smode <player-name>"_jrs);
		return;
	}

	RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
	if (target == nullptr) {
		source->sendMessage(*player, "Error: Player not found."_jrs);
		return;
	}

	if (!source->smodePlayer(*target)) {
		source->sendMessage(*player, "Error: Could not set player's mode."_jrs);
		return;
	}

	source->sendMessage(*player, "Player's mode has been reset."_jrs);
}

const Jupiter::ReadableString &SModeGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's mode from spectator to normal. Syntax: smode <player-name>");
	return defaultHelp;
}

GAME_COMMAND_INIT(SModeGameCommand)

// CancelVote Game Command

void CancelVoteGameCommand::create() {
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("cancelvote"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("cancelvotes"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("cv"));
	this->setAccessLevel(1);
}

void CancelVoteGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	bool cancel_all = false;
	RenX::TeamType target = RenX::TeamType::None;

	if (parameters.empty()) {
		cancel_all = true;
	}
	else {
		if (parameters.equalsi("all") || parameters.equalsi("a")) {
			cancel_all = true;
		} else if (parameters.equalsi("public") || parameters.equalsi("p")) {
			target = RenX::TeamType::None;
		} else if (parameters.equalsi("gdi") || parameters.equalsi("g")) {
			target = RenX::TeamType::GDI;
		} else if (parameters.equalsi("blackhand") || parameters.equalsi("bh") || parameters.equalsi("b")) {
			target = RenX::TeamType::GDI;
		} else if (parameters.equalsi("nod") || parameters.equalsi("n")) {
			target = RenX::TeamType::Nod;
		} else {
			source->sendMessage(*player, STRING_LITERAL_AS_REFERENCE("Error: Invalid Team. Allowed values are all/a, public/p, gdi/g, nod/n, blackhand/bh/b."));
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

const Jupiter::ReadableString &CancelVoteGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Cancels active votes. Syntax: cancelvote [all|public|gdi|nod|blackhand]");
	return defaultHelp;
}

GAME_COMMAND_INIT(CancelVoteGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
