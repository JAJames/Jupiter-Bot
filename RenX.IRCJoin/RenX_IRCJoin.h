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

#if !defined _EXAMPLE_H_HEADER
#define _EXAMPLE_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "RenX_Plugin.h"

class RenX_IRCJoinPlugin : public RenX::Plugin
{
public: // Jupiter::Plugin
	void OnJoin(Jupiter::IRC::Client *source, const Jupiter::ReadableString &chan, const Jupiter::ReadableString &nick) override;
	void OnPart(Jupiter::IRC::Client *source, const Jupiter::ReadableString &chan, const Jupiter::ReadableString &nick, const Jupiter::ReadableString &reason) override;
	int OnRehash();
	RenX_IRCJoinPlugin();
	const Jupiter::ReadableString &getName() override { return name; }

private:
	void init();
	bool publicOnly;
	bool joinMsgAlways;
	bool partMsgAlways;
	int minAccessJoinMessage;
	int maxAccessJoinMessage;
	int minAccessPartMessage;
	int maxAccessPartMessage;
	Jupiter::StringS nameTag;
	Jupiter::StringS chanTag;
	Jupiter::StringS partReasonTag;
	Jupiter::StringS joinFmt;
	Jupiter::StringS partFmt;
	Jupiter::StringS partFmtNoReason;
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.IRCJoin");
};

#endif // _EXAMPLE_H_HEADER