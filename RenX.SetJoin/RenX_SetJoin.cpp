/**
 * Copyright (C) 2014-2015 Jessica James.
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
#include "Jupiter/INIFile.h"
#include "RenX_SetJoin.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Server.h"

const Jupiter::ReferenceString configSection(STRING_LITERAL_AS_REFERENCE("RenX.SetJoin"));

void RenX_SetJoinPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->uuid.isNotEmpty())
	{
		const Jupiter::ReadableString &setjoin = Jupiter::IRC::Client::Config->get(configSection, player->uuid);
		if (setjoin.isNotEmpty())
			server->sendMessage(Jupiter::StringS::Format("[%.*s] %.*s", player->name.size(), player->name.ptr(), setjoin.size(), setjoin.ptr()));
	}
}

// ViewJoin Game Command

void ViewJoinGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("viewjoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("vjoin"));
}

void ViewJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (player->uuid.isNotEmpty())
	{
		const Jupiter::ReadableString &setjoin = Jupiter::IRC::Client::Config->get(configSection, player->uuid);
		if (setjoin.isNotEmpty())
			source->sendMessage(player, Jupiter::StringS::Format("[%.*s] %.*s", player->name.size(), player->name.ptr(), setjoin.size(), setjoin.ptr()));
		else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: No setjoin found."));
	}
	else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: A setjoin message requires steam."));
}

const Jupiter::ReadableString &ViewJoinGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays your join message. Syntax: viewjoin");
	return defaultHelp;
}

GAME_COMMAND_INIT(ViewJoinGameCommand)

// ShowJoin Game Command

void ShowJoinGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("showjoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("shjoin"));
}

void ShowJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (player->uuid.isNotEmpty())
	{
		const Jupiter::ReadableString &setjoin = Jupiter::IRC::Client::Config->get(configSection, player->uuid);
		if (setjoin.isNotEmpty())
			source->sendMessage(Jupiter::StringS::Format("[%.*s] %.*s", player->name.size(), player->name.ptr(), setjoin.size(), setjoin.ptr()));
		else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: No setjoin found."));
	}
	else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: A setjoin message requires steam."));
}

const Jupiter::ReadableString &ShowJoinGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Displays your join message. Syntax: showjoin");
	return defaultHelp;
}

GAME_COMMAND_INIT(ShowJoinGameCommand)

// DelJoin Game Command

void DelJoinGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("deljoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("remjoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("djoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rjoin"));
}

void DelJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &)
{
	if (player->uuid.isNotEmpty())
	{
		if (Jupiter::IRC::Client::Config->remove(configSection, player->uuid))
			source->sendMessage(player, Jupiter::StringS::Format("%.*s, your join message has been removed.", player->name.size(), player->name.ptr()));
		else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Setjoin not found."));
	}
	else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: A setjoin message requires steam."));
}

const Jupiter::ReadableString &DelJoinGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Removes your automatic join message. Syntax: deljoin");
	return defaultHelp;
}

GAME_COMMAND_INIT(DelJoinGameCommand)

// SetJoin Game Command

void SetJoinGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("setjoin"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("sjoin"));
}

void SetJoinGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (player->uuid.isNotEmpty())
	{
		if (parameters.isNotEmpty())
		{
			Jupiter::IRC::Client::Config->set(configSection, player->uuid, parameters);
			Jupiter::IRC::Client::Config->sync();
			source->sendMessage(player, Jupiter::StringS::Format("%.*s, your join message is now: %.*s", player->name.size(), player->name.ptr(), parameters.size(), parameters.ptr()));
		}
		else DelJoinGameCommand_instance.trigger(source, player, parameters);
	}
	else source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: A setjoin message requires steam."));
}

const Jupiter::ReadableString &SetJoinGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Sets an automatic join message. Syntax: setjoin [message]");
	return defaultHelp;
}

GAME_COMMAND_INIT(SetJoinGameCommand)

// Plugin instantiation and entry point.
RenX_SetJoinPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
