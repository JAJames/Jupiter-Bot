/**
 * Copyright (C) 2014-2017 Jessica James.
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

using namespace Jupiter::literals;

void RenX_SetJoinPlugin::RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player) {
	if (!player.uuid.empty() && server.isMatchInProgress()) {
		std::string_view setjoin = RenX_SetJoinPlugin::setjoin_file.get(player.uuid);
		if (!setjoin.empty())
			server.sendMessage(Jupiter::StringS::Format("[%.*s] %.*s", player.name.size(), player.name.data(), setjoin.size(), setjoin.data()));
	}
}

// Plugin instantiation and entry point.
RenX_SetJoinPlugin pluginInstance;

// ViewJoin Game Command

void ViewJoinGameCommand::create() {
	this->addTrigger("viewjoin"_jrs);
	this->addTrigger("vjoin"_jrs);
}

void ViewJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (!player->uuid.empty()) {
		std::string_view setjoin = pluginInstance.setjoin_file.get(player->uuid);

		if (!setjoin.empty())
			source->sendMessage(*player, Jupiter::StringS::Format("[%.*s] %.*s", player->name.size(), player->name.data(), setjoin.size(), setjoin.data()));
		else
			source->sendMessage(*player, "Error: No setjoin found."_jrs);
	}
	else
		source->sendMessage(*player, "Error: A setjoin message requires steam."_jrs);
}

const Jupiter::ReadableString &ViewJoinGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays your join message. Syntax: viewjoin");
	return defaultHelp;
}

GAME_COMMAND_INIT(ViewJoinGameCommand)

// ShowJoin Game Command

void ShowJoinGameCommand::create() {
	this->addTrigger("showjoin"_jrs);
	this->addTrigger("shjoin"_jrs);
}

void ShowJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (!player->uuid.empty()) {
		std::string_view setjoin = pluginInstance.setjoin_file.get(player->uuid);

		if (!setjoin.empty())
			source->sendMessage(Jupiter::StringS::Format("[%.*s] %.*s", player->name.size(), player->name.data(), setjoin.size(), setjoin.data()));
		else
			source->sendMessage(*player, "Error: No setjoin found."_jrs);
	}
	else
		source->sendMessage(*player, "Error: A setjoin message requires steam."_jrs);
}

const Jupiter::ReadableString &ShowJoinGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays your join message. Syntax: showjoin");
	return defaultHelp;
}

GAME_COMMAND_INIT(ShowJoinGameCommand)

// DelJoin Game Command

void DelJoinGameCommand::create()
{
	this->addTrigger("deljoin"_jrs);
	this->addTrigger("remjoin"_jrs);
	this->addTrigger("djoin"_jrs);
	this->addTrigger("rjoin"_jrs);
}

void DelJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &) {
	if (!player->uuid.empty()) {
		if (pluginInstance.setjoin_file.remove(player->uuid))
			source->sendMessage(*player, Jupiter::StringS::Format("%.*s, your join message has been removed.", player->name.size(), player->name.data()));
		else
			source->sendMessage(*player, "Error: Setjoin not found."_jrs);
	}
	else
		source->sendMessage(*player, "Error: A setjoin message requires steam."_jrs);
}

const Jupiter::ReadableString &DelJoinGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes your automatic join message. Syntax: deljoin");
	return defaultHelp;
}

GAME_COMMAND_INIT(DelJoinGameCommand)

// SetJoin Game Command

void SetJoinGameCommand::create() {
	this->addTrigger("setjoin"_jrs);
	this->addTrigger("sjoin"_jrs);
}

void SetJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters) {
	if (!player->uuid.empty()) {
		if (!parameters.empty()) {
			pluginInstance.setjoin_file.set(player->uuid, static_cast<std::string>(parameters));
			pluginInstance.setjoin_file.write();
			source->sendMessage(*player, Jupiter::StringS::Format("%.*s, your join message is now: %.*s", player->name.size(), player->name.data(), parameters.size(),
				parameters.data()));
		}
		else DelJoinGameCommand_instance.trigger(source, player, parameters);
	}
	else source->sendMessage(*player, "Error: A setjoin message requires steam."_jrs);
}

const Jupiter::ReadableString &SetJoinGameCommand::getHelp(const Jupiter::ReadableString &) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sets an automatic join message. Syntax: setjoin [message]");
	return defaultHelp;
}

GAME_COMMAND_INIT(SetJoinGameCommand)

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
