/**
 * Copyright (C) 2016 Jessica James.
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

#if !defined _RENX_LADDER_ALL_TIME
#define _RENX_LADDER_ALL_TIME

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "RenX_Plugin.h"
#include "RenX_LadderDatabase.h"

class RenX_Ladder_Weekly_TimePlugin : public RenX::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }

	RenX_Ladder_Weekly_TimePlugin();

	int last_sorted_day;
	int reset_day;
private:
	RenX::LadderDatabase database;
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Ladder.Weekly");
};

void OnPreUpdateLadder(RenX::LadderDatabase &database, RenX::Server *server, const RenX::TeamType &team);

#endif // _RENX_LADDER_ALL_TIME