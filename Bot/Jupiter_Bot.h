/**
 * Copyright (C) 2013-2014 Justin James.
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

#if !defined JUPITER_BOT_API

/**
* @file Jupiter_Bot.h
* @brief Defines JUPITER_BOT_API.
* On anything other than Windows, JUPITER_BOT_API is defined as nothing, to prevent compiler errors.
*/

#if defined _WIN32

#if defined JUPITER_BOT_EXPORTS
#define JUPITER_BOT_API __declspec(dllexport) 
#else // JUPITER_BOT_EXPORTS
#define JUPITER_BOT_API __declspec(dllimport) 
#endif // JUPITER_BOT_EXPORTS

#else // _WIN32
#define JUPITER_BOT_API
#endif // _WIN32

#endif // JUPITER_BOT_API