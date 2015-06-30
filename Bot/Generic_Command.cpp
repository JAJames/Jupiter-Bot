/**
 * Copyright (C) 2015 Justin James.
 *
 * This license must be preserved.
 * Any applications, libraries, or code which make any use of any
 * component of this program must not be commercial, unless explicit
 * permission is granted from the original author. The use of this
 * program for non-profit purposes is permitted.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In the event that this license restricts you from making desired use of this program, contact the original author.
 * Written by Justin James <justin.aj@hotmail.com>
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