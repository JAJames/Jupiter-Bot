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

#if !defined _EXCESSIVEHEADSHOTS_H_HEADER
#define _EXCESSIVEHEADSHOTS_H_HEADER

#include "Jupiter/Plugin.h"
#include "RenX_Plugin.h"

class RenX_ExcessiveHeadshotsPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnKill(RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const Jupiter::ReadableString &damageType) override;

public: // Jupiter::Plugin
	int OnRehash() override;
	const Jupiter::ReadableString &getName() override { return name; }

	RenX_ExcessiveHeadshotsPlugin();
private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.ExcessiveHeadshots");
	unsigned int minFlags = 4;
	double ratio = 0.5;
	double minKD = 5.0;
	double minKPS = 0.1;
	unsigned int minKills = 10;
	time_t maxGameTime = 180;
};

#endif // _EXCESSIVEHEADSHOTS_H_HEADER