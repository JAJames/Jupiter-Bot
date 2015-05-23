/**
 * Copyright (C) 2014-2015 Justin James.
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

#if !defined _RENX_EXTRALOGGING_H_HEADER
#define _RENX_EXTRALOGGING_H_HEADER

#include "Jupiter/Plugin.h"
#include "RenX_Plugin.h"

class RenX_ExtraLoggingPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	virtual void RenX_OnRaw(RenX::Server *server, const Jupiter::ReadableString &raw) override;

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;

public: // RenX_ExtraLoggingPlugin

	bool init();

	RenX_ExtraLoggingPlugin();
	~RenX_ExtraLoggingPlugin();

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.ExtraLogging");
	bool printToConsole;
	FILE *file;
};

#endif // _RENX_EXTRALOGGING_H_HEADER