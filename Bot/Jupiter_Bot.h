/**
 * Copyright (C) 2013-2015 Justin James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
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