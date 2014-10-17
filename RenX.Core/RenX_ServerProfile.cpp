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

#include "RenX_ServerProfile.h"

struct BaseProfile : RenX::ServerProfile
{
	BaseProfile()
	{
		supported = true;
		privateMessages = true;
		disconnectOnGameOver = false;
		pidbug = false;
		mustSanitize = true;
	}
} _baseProfile;
const RenX::ServerProfile *RenX::defaultProfile = &_baseProfile;

struct OpenBeta1Profile : BaseProfile
{
	OpenBeta1Profile()
	{
		supported = false;
		privateMessages = false;
	}
} _openBeta1Profile;
const RenX::ServerProfile *RenX::openBeta1Profile = &_openBeta1Profile;

struct OpenBeta2Profile : BaseProfile
{
	OpenBeta2Profile()
	{
		privateMessages = false;
		pidbug = true;
	}
} _openBeta2Profile;
const RenX::ServerProfile *RenX::openBeta2Profile = &_openBeta2Profile;

struct OpenBeta3Profile : BaseProfile
{
	OpenBeta3Profile()
	{
		disconnectOnGameOver = true;
	}
} _openBeta3Profile;
const RenX::ServerProfile *RenX::openBeta3Profile = &_openBeta3Profile;
