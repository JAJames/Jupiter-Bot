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

#if !defined _RENX_SERVERPROFILE_H_HEADER
#define _RENX_SERVERPROFILE_H_HEADER

/**
 * @file RenX_ServerProfile.h
 * @brief Defines the ServerProfile class, and known profiles.
 */

#include "RenX.h"

namespace RenX
{
	/**
	* @brief Contains information about features supported by a server version.
	*/
	struct RENX_API ServerProfile
	{
		bool supported;
		bool privateMessages;
		bool disconnectOnGameOver;
	};

	RENX_API extern const ServerProfile *defaultProfile;	/** Default server profile */
	RENX_API extern const ServerProfile *openBeta1Profile;	/** Open Beta 1 server profile */
	RENX_API extern const ServerProfile *openBeta2Profile;	/** Open Beta 2 server profile */
	RENX_API extern const ServerProfile *openBeta3Profile;	/** Open Beta 3 server profile */
	
}

#endif // _RENX_SERVERPROFILE_H_HEADER