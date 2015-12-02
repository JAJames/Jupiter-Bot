/**
 * Copyright (C) 2014-2015 Jessica James.
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

	enum class DisconnectReason : unsigned int
	{
		SocketError,
		SocketErrorReconnect,
		ProtocolError,
		ProtocolErrorReconnect,
		PingTimeout,
		PingTimeoutReconnect,
		IncompatibleVersion,
		IncompatibleVersionReconnect,
		Triggered,
		TriggeredReconnect,
		Rehash,
		RehashReconnect,
		OtherError,
		OtherErrorReconnect,
		Other,
		OtherReconnect
	};
}

#endif // __cplusplus

#endif // RENX_API