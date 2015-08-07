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

#if !defined _RENX_NICKNAMEUUID_H_HEADER
#define _RENX_NICKNAMEUUID_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "RenX_Plugin.h"

class RenX_NicknameUUIDPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnServerCreate(RenX::Server *server) override;

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }

	RenX_NicknameUUIDPlugin();
	~RenX_NicknameUUIDPlugin();

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.NicknameUUID");
};

#endif // _RENX_NICKNAMEUUID_H_HEADER