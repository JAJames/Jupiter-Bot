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

#include "RenX_GameCommand.h"
#include "RenX_Server.h"

Jupiter::ArrayList<RenX::GameCommand> _GameMasterCommandList;
Jupiter::ArrayList<RenX::GameCommand> *RenX::GameMasterCommandList = &_GameMasterCommandList;

RenX::GameCommand::GameCommand(nullptr_t)
{
}

RenX::GameCommand::GameCommand(const RenX::GameCommand &command)
{
	//RenX::GameMasterCommandList->add(this);
}

RenX::GameCommand::GameCommand()
{
	RenX::GameMasterCommandList->add(this);
}

RenX::GameCommand::~GameCommand()
{
	RenX::Core *core = RenX::getCore();
	for (int a = RenX::GameMasterCommandList->size() - 1; a >= 0; a--)
	{
		if (RenX::GameMasterCommandList->get(a) == this)
		{
			RenX::Server *server;
			for (int b = core->getServerCount() - 1; b >= 0; b--)
			{
				server = core->getServer(b);
				if (server != nullptr) server->removeCommand(this->getTrigger());
			}
			RenX::GameMasterCommandList->remove(a);
			break;
		}
	}
}

int RenX::GameCommand::getAccessLevel()
{
	return RenX::GameCommand::access;
}

void RenX::GameCommand::setAccessLevel(int accessLevel)
{
	RenX::GameCommand::access = accessLevel;
}

// Basic Game Command

RenX::BasicGameCommand::BasicGameCommand() : RenX::GameCommand(nullptr)
{
}

RenX::BasicGameCommand::BasicGameCommand(BasicGameCommand &c) : RenX::GameCommand(c)
{
}

RenX::BasicGameCommand::BasicGameCommand(const Jupiter::ReadableString &in_trigger, const Jupiter::ReadableString &in_message, const Jupiter::ReadableString &in_help_message) : RenX::GameCommand(nullptr)
{
	this->addTrigger(in_trigger);
	RenX::BasicGameCommand::message = in_message;
	RenX::BasicGameCommand::help_message = in_help_message;
}

void RenX::BasicGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	source->sendMessage(RenX::BasicGameCommand::message);
}

const Jupiter::ReadableString &RenX::BasicGameCommand::getHelp(const Jupiter::ReadableString &)
{
	static STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Returns a basic text string.");
	if (RenX::BasicGameCommand::help_message.isEmpty())
		return defaultHelp;
	return RenX::BasicGameCommand::help_message;
}

RenX::BasicGameCommand *RenX::BasicGameCommand::copy()
{
	RenX::BasicGameCommand *r = new RenX::BasicGameCommand(*this);
	r->message = RenX::BasicGameCommand::message;
	r->help_message = RenX::BasicGameCommand::help_message;
	return r;
}

void RenX::BasicGameCommand::create()
{
}