/**
 * Copyright (C) 2015 Justin James.
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
		double health = 0;
		double max_health = 0;
		bool capturable = false;
		mutable Jupiter::INIFile varData;
	};

}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_BUILDINGINFO_H_HEADER