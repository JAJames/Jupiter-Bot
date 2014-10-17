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

#if !defined _RENX_GREETING_H_HEADER
#define _RENX_GREETING_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/File.h"
#include "Jupiter/String.h"
#include "RenX_Plugin.h"

class RenX_GreetingsPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;

public: // Jupiter::Plugin
	int OnRehash() override;
	const Jupiter::ReadableString &getName() override { return name; }

	RenX_GreetingsPlugin();
private:
	void init();
	bool sendPrivate;
	unsigned int lastLine;
	unsigned int sendMode = 0; /** 0 = Send greetings randomly, 1 = Send greetings sequentially, 2 = Send all greetings */
	Jupiter::File greetingsFile;
	Jupiter::StringS nameTag;
	Jupiter::StringS ipTag;
	Jupiter::StringS steamTag;
	Jupiter::StringS uuidTag;
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Greetings");
};

#endif // _RENX_GREETING_H_HEADER