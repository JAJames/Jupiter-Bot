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

#if !defined _RENX_TAGS_H_HEADER
#define _RENX_TAGS_H_HEADER

/**
 * @file RenX_Tags.h
 * @brief Provides tag processing functions
 */

#include "Jupiter/String.h"
#include "Jupiter/CString.h"
#include "RenX.h"

/** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	/** Forward declarations */
	struct PlayerInfo;
	class Server;

	RENX_API void processTags(Jupiter::StringType &msg, const RenX::Server *server = nullptr, const RenX::PlayerInfo *player = nullptr, const RenX::PlayerInfo *victim = nullptr);
	RENX_API void sanitizeTags(Jupiter::StringType &fmt);

	struct RENX_API Tags
	{
		/** Global formats */
		Jupiter::CStringS dateFmt;
		Jupiter::CStringS timeFmt;

		/** Internal message tags */

		/** Global tags */
		Jupiter::ReferenceString INTERNAL_DATE_TAG;
		Jupiter::ReferenceString INTERNAL_TIME_TAG;

		/** Server tags */
		Jupiter::ReferenceString INTERNAL_RCON_VERSION_TAG;
		Jupiter::ReferenceString INTERNAL_GAME_VERSION_TAG;
		Jupiter::ReferenceString INTERNAL_XRCON_VERSION_TAG;
		Jupiter::ReferenceString INTERNAL_RULES_TAG;

		/** Player tags */
		Jupiter::ReferenceString INTERNAL_NAME_TAG;
		Jupiter::ReferenceString INTERNAL_RAW_NAME_TAG;
		Jupiter::ReferenceString INTERNAL_IP_TAG;
		Jupiter::ReferenceString INTERNAL_STEAM_TAG;
		Jupiter::ReferenceString INTERNAL_UUID_TAG;
		Jupiter::ReferenceString INTERNAL_ID_TAG;
		Jupiter::ReferenceString INTERNAL_CHARACTER_TAG;
		Jupiter::ReferenceString INTERNAL_VEHICLE_TAG;
		Jupiter::ReferenceString INTERNAL_ADMIN_TAG;
		Jupiter::ReferenceString INTERNAL_PREFIX_TAG;
		Jupiter::ReferenceString INTERNAL_GAME_PREFIX_TAG;
		Jupiter::ReferenceString INTERNAL_TEAM_COLOR_TAG;
		Jupiter::ReferenceString INTERNAL_TEAM_SHORT_TAG;
		Jupiter::ReferenceString INTERNAL_TEAM_LONG_TAG;

		/** Victim tags */
		Jupiter::ReferenceString INTERNAL_VICTIM_NAME_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_RAW_NAME_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_IP_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_STEAM_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_UUID_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_ID_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_CHARACTER_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_VEHICLE_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_ADMIN_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_PREFIX_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_GAME_PREFIX_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_TEAM_COLOR_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_TEAM_SHORT_TAG;
		Jupiter::ReferenceString INTERNAL_VICTIM_TEAM_LONG_TAG;

		/** Other tags */
		Jupiter::ReferenceString INTERNAL_WEAPON_TAG;
		Jupiter::ReferenceString INTERNAL_OBJECT_TAG;
		Jupiter::ReferenceString INTERNAL_MESSAGE_TAG;
		Jupiter::ReferenceString INTERNAL_NEW_NAME_TAG;
		Jupiter::ReferenceString INTERNAL_WIN_SCORE_TAG;
		Jupiter::ReferenceString INTERNAL_LOSE_SCORE_TAG;

		/** External message tags */

		/** Global tags */
		Jupiter::StringS dateTag;
		Jupiter::StringS timeTag;

		/** Server tags */
		Jupiter::StringS rconVersionTag;
		Jupiter::StringS gameVersionTag;
		Jupiter::StringS xRconVersionTag;
		Jupiter::StringS rulesTag;

		/** Player tags */
		Jupiter::StringS nameTag;
		Jupiter::StringS rawNameTag;
		Jupiter::StringS ipTag;
		Jupiter::StringS steamTag;
		Jupiter::StringS uuidTag;
		Jupiter::StringS idTag;
		Jupiter::StringS characterTag;
		Jupiter::StringS vehicleTag;
		Jupiter::StringS adminTag;
		Jupiter::StringS prefixTag;
		Jupiter::StringS gamePrefixTag;
		Jupiter::StringS teamColorTag;
		Jupiter::StringS teamShortTag;
		Jupiter::StringS teamLongTag;

		/** Victim tags */
		Jupiter::StringS victimNameTag;
		Jupiter::StringS victimRawNameTag;
		Jupiter::StringS victimIPTag;
		Jupiter::StringS victimSteamTag;
		Jupiter::StringS victimUUIDTag;
		Jupiter::StringS victimIDTag;
		Jupiter::StringS victimCharacterTag;
		Jupiter::StringS victimVehicleTag;
		Jupiter::StringS victimAdminTag;
		Jupiter::StringS victimPrefixTag;
		Jupiter::StringS victimGamePrefixTag;
		Jupiter::StringS victimTeamColorTag;
		Jupiter::StringS victimTeamShortTag;
		Jupiter::StringS victimTeamLongTag;

		/** Other tags */
		Jupiter::StringS weaponTag;
		Jupiter::StringS objectTag;
		Jupiter::StringS messageTag;
		Jupiter::StringS newNameTag;
		Jupiter::StringS winScoreTag;
		Jupiter::StringS loseScoreTag;
	};

	RENX_API extern Tags *tags;
}

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_TAGS_H_HEADER