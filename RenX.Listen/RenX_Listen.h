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

#if !defined _EXAMPLE_H_HEADER
#define _EXAMPLE_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/TCPSocket.h"
#include "RenX_Plugin.h"

class RenX_ListenPlugin : public RenX::Plugin
{
public: // RenX::Plugin

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int think() override;
	int OnRehash();

public: // RenX_ListenPlugin
	bool init();
	~RenX_ListenPlugin();

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Listen");
	Jupiter::TCPSocket socket;
	Jupiter::StringS serverSection;
};

#endif // _EXAMPLE_H_HEADER