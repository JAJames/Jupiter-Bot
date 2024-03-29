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

#include "jessilib/word_split.hpp"
#include "IRC_Bot.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Warn.h"

using namespace std::literals;

bool RenX_WarnPlugin::initialize() {
	m_maxWarns = this->config.get<int>("MaxWarns"sv, 3);
	m_warnAction = this->config.get<int>("MaxAction"sv, -1);
	return true;
}

int RenX_WarnPlugin::OnRehash() {
	RenX::Plugin::OnRehash();
	return this->initialize() ? 0 : -1;
}

// Plugin instantiation and entry point.
RenX_WarnPlugin pluginInstance;

static constexpr std::string_view WARNS_KEY = "w"sv;

// Warn IRC Command

void WarnIRCCommand::create() {
	this->addTrigger("warn"sv);
	this->addTrigger("w"sv);
	this->setAccessLevel(2);
}

void WarnIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (parameters_split.second.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: Warn <Player> <Reason>"sv);
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

	std::string_view name = parameters_split.first;
	std::string_view reason = parameters_split.second;

	RenX::PlayerInfo *player;
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(name);
			if (player != nullptr) {
				int warns = player->varData[pluginInstance.getName()].get<int>(WARNS_KEY) + 1;
				if (warns > pluginInstance.m_maxWarns) {
					switch (pluginInstance.m_warnAction) {
					case -1:
						server->kickPlayer(*player, string_printf("Warning limit reached (%d warnings)", warns));
						source->sendNotice(nick, string_printf("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.data(), warns));
						break;
					default:
						server->banPlayer(*player, "Jupiter Bot/RenX.Warn"sv, string_printf("Warning limit reached (%d warnings)", warns), std::chrono::seconds(pluginInstance.m_warnAction));
						source->sendNotice(nick, string_printf("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.data(), reason.size(), reason.data(), warns));
						break;
					}
				}
				else {
					player->varData[pluginInstance.getName()].set(WARNS_KEY, std::to_string(warns));
					server->sendWarnMessage(*player, string_printf("You have been warned by %.*s@IRC for: %.*s. You have %d warnings.", nick.size(),
						nick.data(), reason.size(), reason.data(), warns));
					source->sendNotice(nick, string_printf("%.*s has been warned; they now have %d warnings.", player->name.size(), player->name.data(), warns));
				}
			}
		}
	}
}

std::string_view WarnIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Warns a player. Syntax: Warn <Player> <Reason>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(WarnIRCCommand)

// Pardon IRC Command

void PardonIRCCommand::create() {
	this->addTrigger("pardon"sv);
	this->addTrigger("forgive"sv);
	this->addTrigger("unwarn"sv);
	this->setAccessLevel(2);
}

void PardonIRCCommand::trigger(IRC_Bot *source, std::string_view channel, std::string_view nick, std::string_view parameters) {
	if (parameters.empty()) {
		this->trigger(source, channel, nick, nick);
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
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(parameters);
			if (player != nullptr) {
				player->varData[pluginInstance.getName()].remove(WARNS_KEY);
				server->sendMessage(*player, string_printf("You have been pardoned by %.*s@IRC; your warnings have been reset.", nick.size(),
					nick.data()));
				source->sendNotice(nick, string_printf("%.*s has been pardoned; their warnings have been reset.", player->name.size(), player->name.data()));
			}
		}
	}
}

std::string_view PardonIRCCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Resets a player's warnings. Syntax: Pardon <Player>"sv;
	return defaultHelp;
}

IRC_COMMAND_INIT(PardonIRCCommand)

// Warn Game Command

void WarnGameCommand::create() {
	this->addTrigger("warn"sv);
	this->addTrigger("w"sv);
	this->setAccessLevel(1);
}

void WarnGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (!parameters_split.second.empty()) {
		std::string_view name = parameters_split.first;
		std::string_view reason = parameters_split.second;

		RenX::PlayerInfo *target = source->getPlayerByPartName(name);
		if (target != nullptr) {
			int warns = target->varData[pluginInstance.getName()].get<int>(WARNS_KEY) + 1;
			if (warns > pluginInstance.m_maxWarns) {
				switch (pluginInstance.m_warnAction)
				{
				case -1:
					source->kickPlayer(*target, string_printf("Warning limit reached (%d warnings)", warns));
					source->sendMessage(*player, string_printf("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.data(), warns));
					break;
				default:
					source->banPlayer(*target, "Jupiter Bot/RenX.Warn"sv, string_printf("Warning limit reached (%d warnings)", warns), std::chrono::seconds(pluginInstance.m_warnAction));
					source->sendMessage(*player, string_printf("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.data(), warns));
					break;
				}
			}
			else {
				target->varData[pluginInstance.getName()].set(WARNS_KEY, std::to_string(warns));
				source->sendWarnMessage(*target, string_printf("You have been warned by %.*s for: %.*s. You have %d warnings.", player->name.size(), player->name.data(), reason.size(), reason.data(), warns));
				source->sendMessage(*player, string_printf("%.*s has been warned; they now have %d warnings.", target->name.size(), target->name.data(), warns));
			}
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: Warn <Player> <Reason>"sv);
}

std::string_view WarnGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Warns a player. Syntax: Warn <Player> <Reason>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(WarnGameCommand)

// Pardon Game Command

void PardonGameCommand::create() {
	this->addTrigger("pardon"sv);
	this->addTrigger("forgive"sv);
	this->addTrigger("unwarn"sv);
	this->setAccessLevel(1);
}

void PardonGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!parameters.empty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target != nullptr) {
			target->varData[pluginInstance.getName()].remove(WARNS_KEY);
			source->sendMessage(*target, string_printf("You have been pardoned by %.*s@IRC; your warnings have been reset.", player->name.size(), player->name.data()));
			source->sendMessage(*player, string_printf("%.*s has been pardoned; their warnings have been reset.", target->name.size(), target->name.data()));
		}
	}
	else {
		this->trigger(source, player, player->name);
	}
}

std::string_view PardonGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Resets a player's warnings. Syntax: Pardon <Player>"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(PardonGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
