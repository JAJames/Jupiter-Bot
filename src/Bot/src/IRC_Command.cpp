/**
 * Copyright (C) 2013-2021 Jessica James.
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

#include "jessilib/unicode.hpp"
#include "IRC_Command.h"

std::vector<IRCCommand*> g_IRCMasterCommandList;
std::vector<IRCCommand*>& IRCMasterCommandList = g_IRCMasterCommandList;

/** IRCCommand */

IRCCommand::IRCCommand() {
	m_access = 0;
	IRCMasterCommandList.push_back(this);
}

IRCCommand::IRCCommand(const IRCCommand &command) {
	m_access = command.m_access;

	for (const auto& channel : command.m_channels) {
		m_channels.emplace_back(channel);
	}

	for (const auto& type : command.m_types) {
		m_types.emplace_back(type);
	}
}

IRCCommand::~IRCCommand() {
	// Remove any weak references to this
	for (auto itr = IRCMasterCommandList.begin(); itr != IRCMasterCommandList.end(); ++itr) {
		if (*itr == this) {
			serverManager->removeCommand(this);
			IRCMasterCommandList.erase(itr);
			break;
		}
	}
}

IRC_Bot *IRCCommand::active_server = nullptr;
IRC_Bot *IRCCommand::selected_server = nullptr;

// IRC Command Functions

int IRCCommand::getAccessLevel() {
	return m_access;
}

int IRCCommand::getAccessLevel(int type) {
	for (const auto& pair : m_types) {
		if (pair.type == type) {
			return pair.access;
		}
	}

	return m_access;
}

int IRCCommand::getAccessLevel(const Jupiter::ReadableString &channel) {
	for (const auto& pair : m_channels) {
		if (jessilib::equalsi(pair.channel, channel)) {
			return pair.access;
		}
	}

	return m_access;
}

int IRCCommand::getAccessLevel(Jupiter::IRC::Client::Channel *channel) {
	for (const auto& pair : m_channels) {
		if (jessilib::equalsi(pair.channel, channel->getName())) {
			return pair.access;
		}
	}

	for (const auto& pair : m_types) {
		if (pair.type == channel->getType()) {
			return pair.access;
		}
	}

	return m_access;
}

void IRCCommand::setAccessLevel(int accessLevel) {
	m_access = accessLevel;
}

void IRCCommand::setAccessLevel(int type, int accessLevel) {
	m_types.push_back({type, accessLevel});
}

void IRCCommand::setAccessLevel(const Jupiter::ReadableString &channel, int accessLevel) {
	m_channels.push_back({ static_cast<std::string>(channel), accessLevel });
}

void IRCCommand::create() {
}

/** GenericCommandWrapperIRCCommand */

GenericCommandWrapperIRCCommand::GenericCommandWrapperIRCCommand(const GenericCommandWrapperIRCCommand &in_command)
	: IRCCommand(in_command) {
	m_command = in_command.m_command;

	// Copy triggers
	for (size_t index = 0; index != m_command->getTriggerCount(); ++index) {
		this->addTrigger(m_command->getTrigger(index));
	}
}

GenericCommandWrapperIRCCommand::GenericCommandWrapperIRCCommand(Jupiter::GenericCommand &in_command)
	: IRCCommand() {
	m_command = &in_command;

	// Copy triggers
	for (size_t index = 0; index != m_command->getTriggerCount(); ++index) {
		this->addTrigger(m_command->getTrigger(index));
	}

	if (serverManager != nullptr) {
		serverManager->addCommand(this);
	}
}

// GenericCommandWrapperIRCCommand functions

void GenericCommandWrapperIRCCommand::trigger(IRC_Bot *source, const Jupiter::ReadableString &in_channel, const Jupiter::ReadableString &in_nick, const Jupiter::ReadableString &in_parameters) {
	Jupiter::GenericCommand::ResponseLine *del;
	Jupiter::GenericCommand::ResponseLine *result = m_command->trigger(in_parameters);

	while (result != nullptr)
	{
		switch (result->type)
		{
		case Jupiter::GenericCommand::DisplayType::PublicSuccess:
		case Jupiter::GenericCommand::DisplayType::PublicError:
			source->sendMessage(in_channel, result->response);
			break;
		case Jupiter::GenericCommand::DisplayType::PrivateSuccess:
		case Jupiter::GenericCommand::DisplayType::PrivateError:
			source->sendNotice(in_nick, result->response);
			break;
		default:
			source->sendMessage(in_nick, result->response);
			break;
		}

		del = result;
		result = result->next;
		delete del;
	}
}

const Jupiter::ReadableString &GenericCommandWrapperIRCCommand::getHelp(const Jupiter::ReadableString &parameters) {
	return GenericCommandWrapperIRCCommand::m_command->getHelp(parameters);
}

IRCCommand *GenericCommandWrapperIRCCommand::copy() {
	return new GenericCommandWrapperIRCCommand(*this);
}

const Jupiter::GenericCommand &GenericCommandWrapperIRCCommand::getGenericCommand() const {
	return *GenericCommandWrapperIRCCommand::m_command;
}
