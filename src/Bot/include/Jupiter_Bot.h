/**
 * Copyright (C) 2013-2017 Jessica James.
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
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#if !defined JUPITER_BOT_API

/**
* @file Jupiter_Bot.h
* @brief Defines JUPITER_BOT_API.
* On anything other than Windows, JUPITER_BOT_API is defined as nothing, to prevent compiler errors.
*/

/** JUPITER_EXPORT, JUPITER_IMPORT */
#if defined _WIN32
#define JUPITER_EXPORT __declspec(dllexport)
#define JUPITER_IMPORT __declspec(dllimport)
#else // _WIN32
#define JUPITER_EXPORT
#define JUPITER_IMPORT
#endif // _WIN32

/** JUPITER_BOT_API */
#if defined JUPITER_BOT_EXPORTS
#define JUPITER_BOT_API JUPITER_EXPORT
#else // JUPITER_BOT_EXPORTS
#define JUPITER_BOT_API JUPITER_IMPORT
#endif // JUPITER_BOT_EXPORTS

#if defined __cplusplus

#include <chrono>

namespace Jupiter {
	/** Forward declarations */
	class Config;

	/** Application config file */
	JUPITER_BOT_API extern Jupiter::Config *g_config;

	/** Application start time */
	JUPITER_BOT_API extern std::chrono::steady_clock::time_point g_start_time;

	/** Reinitialize all application plugins, as if at program startup */
	JUPITER_BOT_API void reinitialize_plugins();
}

#endif // __cplusplus

#endif // JUPITER_BOT_API