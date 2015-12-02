/**
 * Copyright (C) 2015 Jessica James.
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

#include "Generic_Command.h"

Jupiter::ArrayList<GenericCommand> _genericCommands;
Jupiter::ArrayList<GenericCommand> *genericCommands = &_genericCommands;

GenericCommand::GenericCommand()
{
	_genericCommands.add(this);
}

GenericCommand::~GenericCommand()
{
	size_t count = _genericCommands.size();
	while (count != 0)
		if (_genericCommands.get(--count) == this)
		{
			_genericCommands.remove(count);
			break;
		}
}

GenericCommand *getGenericCommand(const Jupiter::ReadableString &trigger)
{
	size_t count = _genericCommands.size();
	while (count != 0)
	{
		GenericCommand *cmd = _genericCommands.get(--count);
		if (cmd->matches(trigger))
			return cmd;
	}
	return nullptr;
}

GenericCommand::ResponseLine::ResponseLine(const Jupiter::ReadableString &response_, GenericCommand::DisplayType type_)
{
	GenericCommand::ResponseLine::response = response_;
	GenericCommand::ResponseLine::type = type_;
}

GenericCommand::ResponseLine *GenericCommand::ResponseLine::set(const Jupiter::ReadableString &response_, GenericCommand::DisplayType type_)
{
	GenericCommand::ResponseLine::response = response_;
	GenericCommand::ResponseLine::type = type_;
	return this;
}