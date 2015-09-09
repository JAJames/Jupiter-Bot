/**
 * Copyright (C) 2013-2015 Justin James.
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
 * Written by Justin James <justin.aj@hotmail.com>
 */

#include "IRC_Command.h"

Jupiter::ArrayList<IRCCommand> _IRCMasterCommandList;
Jupiter::ArrayList<IRCCommand> *IRCMasterCommandList = &_IRCMasterCommandList;

IRCCommand::IRCCommand()
{
	IRCCommand::access = 0;
	IRCMasterCommandList->add(this);
}

IRCCommand::IRCCommand(const IRCCommand &command)
{
	IRCCommand::access = command.access;
	size_t i;

	for (i = 0; i < command.channels.size(); i++)
		IRCCommand::channels.add(new IRCCommand::ChannelAccessPair(*command.channels.get(i)));

	for (i = 0; i < command.types.size(); i++)
		IRCCommand::types.add(new IRCCommand::TypeAccessPair(*command.types.get(i)));
}

IRCCommand::~IRCCommand()
{
	for (size_t i = 0; i != IRCMasterCommandList->size(); i++)
	{
		if (IRCMasterCommandList->get(i) == this)
		{
			serverManager->removeCommand(this);
			IRCMasterCommandList->remove(i);
			break;
		}
	}
	IRCCommand::channels.emptyAndDelete();
	IRCCommand::types.emptyAndDelete();
}

IRC_Bot *IRCCommand::active_server = nullptr;
IRC_Bot *IRCCommand::selected_server = nullptr;

// IRC Command Functions

int IRCCommand::getAccessLevel()
{
	return IRCCommand::access;
}

int IRCCommand::getAccessLevel(int type)
{
	for (size_t i = 0; i != IRCCommand::types.size(); i++)
		if (IRCCommand::types.get(i)->type == type)
			return IRCCommand::types.get(i)->access;
	return IRCCommand::access;
}

int IRCCommand::getAccessLevel(const Jupiter::ReadableString &channel)
{
	IRCCommand::ChannelAccessPair *pair;
	for (size_t i = 0; i != IRCCommand::channels.size(); i++)
	{
		pair = IRCCommand::channels.get(i);
		if (pair->channel.equalsi(channel))
			return pair->access;
	}
	return IRCCommand::access;
}

int IRCCommand::getAccessLevel(Jupiter::IRC::Client::Channel *channel)
{
	IRCCommand::ChannelAccessPair *pair;
	for (size_t i = 0; i != IRCCommand::channels.size(); i++)
	{
		pair = IRCCommand::channels.get(i);
		if (pair->channel.equalsi(channel->getName()))
			return pair->access;
	}

	for (size_t i = 0; i != IRCCommand::types.size(); i++)
		if (IRCCommand::types.get(i)->type == channel->getType())
			return IRCCommand::types.get(i)->access;

	return IRCCommand::access;
}

void IRCCommand::setAccessLevel(int accessLevel)
{
	IRCCommand::access = accessLevel;
}

void IRCCommand::setAccessLevel(int type, int accessLevel)
{
	IRCCommand::TypeAccessPair *pair = new IRCCommand::TypeAccessPair();
	pair->type = type;
	pair->access = accessLevel;
	IRCCommand::types.add(pair);
}

void IRCCommand::setAccessLevel(const Jupiter::ReadableString &channel, int accessLevel)
{
	IRCCommand::ChannelAccessPair *pair = new IRCCommand::ChannelAccessPair();
	pair->channel = channel;
	pair->access = accessLevel;
	IRCCommand::channels.add(pair);
}