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

#include "Jupiter/IRC_Client.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"
#include "RenX_SetJoin.h"

using namespace std::literals;

void RenX_SetJoinPlugin::RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (!player.uuid.empty() && server.isMatchInProgress()) {
		std::string_view setjoin = RenX_SetJoinPlugin::setjoin_file.get(player.uuid);
		if (!setjoin.empty())
			server.sendMessage(string_printf("[%.*s] %.*s", player.name.size(), player.name.data(), setjoin.size(), setjoin.data()));
	}
}

// Plugin instantiation and entry point.
RenX_SetJoinPlugin pluginInstance;

// ViewJoin Game Command

void ViewJoinGameCommand::create() {
	this->addTrigger("viewjoin"sv);
	this->addTrigger("vjoin"sv);
}

void ViewJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!player->uuid.empty()) {
		std::string_view setjoin = pluginInstance.setjoin_file.get(player->uuid);

		if (!setjoin.empty())
			source->sendMessage(*player, string_printf("[%.*s] %.*s", player->name.size(), player->name.data(), setjoin.size(), setjoin.data()));
		else
			source->sendMessage(*player, "Error: No setjoin found."sv);
	}
	else
		source->sendMessage(*player, "Error: A setjoin message requires steam."sv);
}

std::string_view ViewJoinGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Displays your join message. Syntax: viewjoin"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(ViewJoinGameCommand)

// ShowJoin Game Command

void ShowJoinGameCommand::create() {
	this->addTrigger("showjoin"sv);
	this->addTrigger("shjoin"sv);
}

void ShowJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!player->uuid.empty()) {
		std::string_view setjoin = pluginInstance.setjoin_file.get(player->uuid);

		if (!setjoin.empty())
			source->sendMessage(string_printf("[%.*s] %.*s", player->name.size(), player->name.data(), setjoin.size(), setjoin.data()));
		else
			source->sendMessage(*player, "Error: No setjoin found."sv);
	}
	else
		source->sendMessage(*player, "Error: A setjoin message requires steam."sv);
}

std::string_view ShowJoinGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Displays your join message. Syntax: showjoin"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(ShowJoinGameCommand)

// DelJoin Game Command

void DelJoinGameCommand::create()
{
	this->addTrigger("deljoin"sv);
	this->addTrigger("remjoin"sv);
	this->addTrigger("djoin"sv);
	this->addTrigger("rjoin"sv);
}

void DelJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view ) {
	if (!player->uuid.empty()) {
		if (pluginInstance.setjoin_file.remove(player->uuid))
			source->sendMessage(*player, string_printf("%.*s, your join message has been removed.", player->name.size(), player->name.data()));
		else
			source->sendMessage(*player, "Error: Setjoin not found."sv);
	}
	else
		source->sendMessage(*player, "Error: A setjoin message requires steam."sv);
}

std::string_view DelJoinGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Removes your automatic join message. Syntax: deljoin"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(DelJoinGameCommand)

// SetJoin Game Command

void SetJoinGameCommand::create() {
	this->addTrigger("setjoin"sv);
	this->addTrigger("sjoin"sv);
}

void SetJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	if (!player->uuid.empty()) {
		if (!parameters.empty()) {
			pluginInstance.setjoin_file.set(player->uuid, static_cast<std::string>(parameters));
			pluginInstance.setjoin_file.write();
			source->sendMessage(*player, string_printf("%.*s, your join message is now: %.*s", player->name.size(), player->name.data(), parameters.size(),
				parameters.data()));
		}
		else DelJoinGameCommand_instance.trigger(source, player, parameters);
	}
	else source->sendMessage(*player, "Error: A setjoin message requires steam."sv);
}

std::string_view SetJoinGameCommand::getHelp(std::string_view ) {
	static constexpr std::string_view defaultHelp = "Sets an automatic join message. Syntax: setjoin [message]"sv;
	return defaultHelp;
}

GAME_COMMAND_INIT(SetJoinGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
