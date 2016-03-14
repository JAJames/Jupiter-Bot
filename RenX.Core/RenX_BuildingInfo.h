/**
 * Copyright (C) 2015-2016 Jessica James.
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

#if !defined _RENX_BUILDINGINFO_H_HEADER
#define _RENX_BUILDINGINFO_H_HEADER

/**
 * @file RenX_BuildingInfo.h
 * @brief Defines the BuildingInfo structure.
 */

#include "Jupiter/String.h"
#include "Jupiter/INIFile.h"
#include "RenX.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{

	/**
	* @brief Includes all of the tracked information about a building.
	*/
	struct RENX_API BuildingInfo
	{
		Jupiter::StringS name;
		TeamType team = TeamType::Other;
		int health = 0;
		int max_health = 0;
		int armor = 0;
		int max_armor = 0;
		bool capturable = false;
		bool destroyed = false;
		std::chrono::steady_clock::time_point destruction_time;
		mutable Jupiter::INIFile varData;
	};

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_BUILDINGINFO_H_HEADER