/**
 * Copyright (C) 2016-2021 Jessica James.
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

#include "RenX_Map.h"

RenX::Map::Map()
{
	RenX::Map::Map::guid[0] = 0ULL;
	RenX::Map::Map::guid[1] = 0ULL;
}

RenX::Map::Map(std::string_view in_name) : Map()
{
	name = in_name;
}

RenX::Map::Map(std::string_view in_name, uint64_t in_guid[2]) : Map(in_name)
{
	RenX::Map::Map::guid[0] = in_guid[0];
	RenX::Map::Map::guid[1] = in_guid[1];
}

bool RenX::Map::equals(const RenX::Map &map) const
{
	return RenX::Map::Map::guid[0] == map.guid[0] && RenX::Map::Map::guid[1] == map.guid[1];
}