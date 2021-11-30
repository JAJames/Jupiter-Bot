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

using namespace Jupiter::literals;

bool RenX_WarnPlugin::initialize() {
	m_maxWarns = this->config.get<int>("MaxWarns"_jrs, 3);
	m_warnAction = this->config.get<int>("MaxAction"_jrs, -1);
	return true;
}

int RenX_WarnPlugin::OnRehash() {
	RenX::Plugin::OnRehash();
	return this->initialize() ? 0 : -1;
}

// Plugin instantiation and entry point.
RenX_WarnPlugin pluginInstance;

STRING_LITERAL_AS_NAMED_REFERENCE(WARNS_KEY, "w");

// Warn IRC Command

void WarnIRCCommand::create() {
	this->addTrigger("warn"_jrs);
	this->addTrigger("w"_jrs);
	this->setAccessLevel(2);
}

void WarnIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	auto parameters_split = jessilib::word_split_once_view(std::string_view{parameters}, WHITESPACE_SV);
	if (parameters_split.second.empty()) {
		source->sendNotice(nick, "Error: Too Few Parameters. Syntax: Warn <Player> <Reason>"_jrs);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
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
						server->kickPlayer(*player, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns));
						source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.data(), warns));
						break;
					default:
						server->banPlayer(*player, "Jupiter Bot/RenX.Warn"_jrs, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns), std::chrono::seconds(pluginInstance.m_warnAction));
						source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", player->name.size(), player->name.data(), reason.size(), reason.data(), warns));
						break;
					}
				}
				else {
					player->varData[pluginInstance.getName()].set(WARNS_KEY, std::to_string(warns));
					server->sendWarnMessage(*player, Jupiter::StringS::Format("You have been warned by %.*s@IRC for: %.*s. You have %d warnings.", nick.size(), nick.ptr(), reason.size(), reason.data(), warns));
					source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been warned; they now have %d warnings.", player->name.size(), player->name.data(), warns));
				}
			}
		}
	}
}

const Jupiter::ReadableString &WarnIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Warns a player. Syntax: Warn <Player> <Reason>");
	return defaultHelp;
}

IRC_COMMAND_INIT(WarnIRCCommand)

// Pardon IRC Command

void PardonIRCCommand::create() {
	this->addTrigger("pardon"_jrs);
	this->addTrigger("forgive"_jrs);
	this->addTrigger("unwarn"_jrs);
	this->setAccessLevel(2);
}

void PardonIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &parameters) {
	if (parameters.empty()) {
		// TODO: this doesn't make sense
		this->trigger(source, channel, nick, nick);
		return;
	}

	Jupiter::IRC::Client::Channel *chan = source->getChannel(channel);
	if (chan == nullptr) {
		return;
	}

	const auto& servers = RenX::getCore()->getServers(chan->getType());
	if (servers.empty()) {
		source->sendMessage(channel, "Error: Channel not attached to any connected Renegade X servers."_jrs);
		return;
	}

	RenX::PlayerInfo *player;
	for (const auto& server : servers) {
		if (server != nullptr) {
			player = server->getPlayerByPartName(parameters);
			if (player != nullptr) {
				player->varData[pluginInstance.getName()].remove(WARNS_KEY);
				server->sendMessage(*player, Jupiter::StringS::Format("You have been pardoned by %.*s@IRC; your warnings have been reset.", nick.size(), nick.ptr()));
				source->sendNotice(nick, Jupiter::StringS::Format("%.*s has been pardoned; their warnings have been reset.", player->name.size(), player->name.data()));
			}
		}
	}
}

const Jupiter::ReadableString &PardonIRCCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's warnings. Syntax: Pardon <Player>");
	return defaultHelp;
}

IRC_COMMAND_INIT(PardonIRCCommand)

// Warn Game Command

void WarnGameCommand::create() {
	this->addTrigger("warn"_jrs);
	this->addTrigger("w"_jrs);
	this->setAccessLevel(1);
}

void WarnGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
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
					source->kickPlayer(*target, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns));
					source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been kicked from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.data(), warns));
					break;
				default:
					source->banPlayer(*target, "Jupiter Bot/RenX.Warn"_jrs, Jupiter::StringS::Format("Warning limit reached (%d warnings)", warns), std::chrono::seconds(pluginInstance.m_warnAction));
					source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been banned from the server for exceeding the warning limit (%d warnings).", target->name.size(), target->name.data(), warns));
					break;
				}
			}
			else {
				target->varData[pluginInstance.getName()].set(WARNS_KEY, std::to_string(warns));
				source->sendWarnMessage(*target, Jupiter::StringS::Format("You have been warned by %.*s for: %.*s. You have %d warnings.", player->name.size(), player->name.data(), reason.size(), reason.data(), warns));
				source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been warned; they now have %d warnings.", target->name.size(), target->name.data(), warns));
			}
		}
	}
	else
		source->sendMessage(*player, "Error: Too few parameters. Syntax: Warn <Player> <Reason>"_jrs);
}

const Jupiter::ReadableString &WarnGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Warns a player. Syntax: Warn <Player> <Reason>");
	return defaultHelp;
}

GAME_COMMAND_INIT(WarnGameCommand)

// Pardon Game Command

void PardonGameCommand::create() {
	this->addTrigger("pardon"_jrs);
	this->addTrigger("forgive"_jrs);
	this->addTrigger("unwarn"_jrs);
	this->setAccessLevel(1);
}

void PardonGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (parameters.isNotEmpty()) {
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target != nullptr) {
			target->varData[pluginInstance.getName()].remove(WARNS_KEY);
			source->sendMessage(*target, Jupiter::StringS::Format("You have been pardoned by %.*s@IRC; your warnings have been reset.", player->name.size(), player->name.data()));
			source->sendMessage(*player, Jupiter::StringS::Format("%.*s has been pardoned; their warnings have been reset.", target->name.size(), target->name.data()));
		}
	}
	else {
		this->trigger(source, player, Jupiter::ReferenceString{player->name});
	}
}

const Jupiter::ReadableString &PardonGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Resets a player's warnings. Syntax: Pardon <Player>");
	return defaultHelp;
}

GAME_COMMAND_INIT(PardonGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
