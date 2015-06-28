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

#if !defined _CHANNELRELAY_H_HEADER
#define _CHANNELRELAY_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"

class ChannelRelayPlugin : public Jupiter::Plugin
{
public: // Jupiter::Plugin
	void OnChat(Jupiter::IRC::Client *server, const Jupiter::ReadableString &channel, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &message) override;
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;

	int init();
private:
	Jupiter::String_Strict<int> types;
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "ChannelRelay");
};

#endif // _CHANNELRELAY_H_HEADER