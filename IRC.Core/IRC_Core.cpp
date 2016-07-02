/**
 * Copyright (C) 2016 Jessica James.
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

#include "Jupiter/GenericCommand.h"
#include "ServerManager.h"
#include "IRC_Command.h"
#include "IRC_Core.h"

IRCCorePlugin::~IRCCorePlugin()
{
	IRCCorePlugin::m_wrapped_commands.emptyAndDelete();
}

void IRCCorePlugin::OnGenericCommandAdd(Jupiter::GenericCommand &in_command)
{
	IRCCorePlugin::m_wrapped_commands.add(new GenericCommandWrapperIRCCommand(in_command));
}

void IRCCorePlugin::OnGenericCommandRemove(Jupiter::GenericCommand &in_command)
{
	for (size_t index = 0; index != IRCCorePlugin::m_wrapped_commands.size(); ++index)
		if (&IRCCorePlugin::m_wrapped_commands.get(index)->getGenericCommand() == &in_command)
		{
			delete IRCCorePlugin::m_wrapped_commands.remove(index);
			return;
		}
}



// Plugin instantiation and entry point.
IRCCorePlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
