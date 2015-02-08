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

#if !defined RENX_API

/**
 * @file RenX.h
 * @brief Defines RENX_API and some global enums for RenX.
 * On anything other than Windows, RENX_API is defined as nothing, to prevent compiler errors.
 */

#if defined _WIN32

#if defined RENX_EXPORTS
#define RENX_API __declspec(dllexport) 
#else // RENX_EXPORTS
#define RENX_API __declspec(dllimport) 
#endif // RENX_EXPORTS

#else // _WIN32
#define RENX_API
#endif // _WIN32

#if defined __cplusplus

namespace RenX
{
	enum class WinType
	{
		Score,
		Base,
		Beacon,
		Tie,
		Shutdown,
		Unknown
	};

	enum class TeamType
	{
		GDI,
		Nod,
		Other,
		None
	};

	enum class ObjectType
	{
		Player,
		Vehicle,
		Building,
		Defence,
		None
	};
}

#endif // __cplusplus

#endif // RENX_API