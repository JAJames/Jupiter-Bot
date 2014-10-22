/**
 * Copyright (C) 2014 Justin James.
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

#include "RenX_GameCommand.h"
#include "RenX_Server.h"

Jupiter::ArrayList<RenX::GameCommand> _GameMasterCommandList;
Jupiter::ArrayList<RenX::GameCommand> *RenX::GameMasterCommandList = &_GameMasterCommandList;

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