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

#include "RenX_GameCommand.h"
#include "RenX_Server.h"

std::vector<RenX::GameCommand*> g_GameMasterCommandList;
std::vector<RenX::GameCommand*> &RenX::GameMasterCommandList = g_GameMasterCommandList;

RenX::Server *RenX::GameCommand::active_server = nullptr;
RenX::Server *RenX::GameCommand::selected_server = nullptr;

RenX::GameCommand::GameCommand(std::nullptr_t) {
}

RenX::GameCommand::GameCommand(const RenX::GameCommand &command) {
	//RenX::GameMasterCommandList->add(this);
}

RenX::GameCommand::GameCommand() {
	RenX::GameMasterCommandList.push_back(this);
}

RenX::GameCommand::~GameCommand() {
	RenX::Core *core = RenX::getCore();
	for (auto itr = RenX::GameMasterCommandList.begin(); itr != RenX::GameMasterCommandList.end(); ++itr) {
		if (*itr == this) {
			RenX::Server *server;
			for (size_t server_index = 0; server_index != core->getServerCount(); ++server_index) {
				server = core->getServer(server_index);
				if (server != nullptr) {
					server->removeCommand(this->getTrigger());
				}
			}

			RenX::GameMasterCommandList.erase(itr);
			break;
		}
	}
}

int RenX::GameCommand::getAccessLevel() {
	return RenX::GameCommand::access;
}

void RenX::GameCommand::setAccessLevel(int accessLevel) {
	RenX::GameCommand::access = accessLevel;
}

// Basic Game Command

RenX::BasicGameCommand::BasicGameCommand() : RenX::GameCommand(nullptr) {
}

RenX::BasicGameCommand::BasicGameCommand(BasicGameCommand &c) : RenX::GameCommand(c) {
}

RenX::BasicGameCommand::BasicGameCommand(std::string_view in_trigger, std::string_view in_message, std::string_view in_help_message)
	: RenX::GameCommand(nullptr) {
	this->addTrigger(in_trigger);
	m_message = in_message;
	m_help_message = in_help_message;
}

void RenX::BasicGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) {
	source->sendMessage(m_message);
}

std::string_view RenX::BasicGameCommand::getHelp(std::string_view ) {
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Returns a basic text string.");
	if (m_help_message.empty()) {
		return defaultHelp;
	}

	static Jupiter::ReferenceString please_delete_this_later_jessica;
	please_delete_this_later_jessica = m_help_message;
	return please_delete_this_later_jessica;
}

RenX::BasicGameCommand *RenX::BasicGameCommand::copy() {
	RenX::BasicGameCommand* result = new RenX::BasicGameCommand(*this);
	result->m_message = m_message;
	result->m_help_message = m_help_message;
	return result;
}

void RenX::BasicGameCommand::create() {
}