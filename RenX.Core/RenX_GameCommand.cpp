/**
 * Copyright (C) 2014-2015 Justin James.
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