/**
 * Copyright (C) 2014-2017 Jessica James.
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

#if !defined _RENX_FUNCTIONS_H_HEADER
#define _RENX_FUNCTIONS_H_HEADER

/**
 * @file RenX_Functions.h
 * @brief Provides some functions to be used at your convenience.
 */

#include <chrono>
#include "Jupiter/Config.h"
#include "Jupiter/String.hpp"
#include "RenX.h"
#include "RenX_Map.h"

namespace RenX
{
	/** Forward declarations */
	struct PlayerInfo;
	class Server;

	/**
	* @brief Translates a team's number into a TeamType.
	*
	* @param teamNum Team number
	* @return A corresponding TeamType.
	*/
	RENX_API TeamType getTeam(int teamNum);

	/**
	* @brief Translates team number into a TeamType.
	*
	* @param team Team name
	* @return A corresponding TeamType.
	*/
	RENX_API TeamType getTeam(std::string_view team);

	/**
	* @brief Fetches the corresponding IRC color code for a team.
	*
	* @param team Team to get IRC color code for
	* @return A team's corresponding IRC color-code.
	*/
	RENX_API std::string_view getTeamColor(TeamType team);

	/**
	* @brief Fetches a team's name.
	* Example: GDI
	*
	* @param team Team to translate into a name.
	* @return The team's corresponding name.
	*/
	RENX_API std::string_view getTeamName(TeamType team);

	/**
	* @brief Fetches a team's full name.
	* Example: Global Defense Initiative
	*
	* @param team Team to translate into a name.
	* @return The team's corresponding name.
	*/
	RENX_API std::string_view getFullTeamName(TeamType team);

	/**
	* @brief Fetches a team's enemy.
	*
	* @param team TeamType to return enemy for.
	* @return Enemy TeamType of the input team.
	*/
	RENX_API TeamType getEnemy(TeamType team);

	/**
	* @brief Reduces a character-related preset to a simplified form.
	* Example: Returns "Nod_Soldier" from "Rx_FamilyInfo_Nod_Soldier".
	*
	* @param preset Preset to simplify.
	* @return Simplified form of preset.
	*/
	RENX_API std::string_view getCharacter(std::string_view preset);

	/**
	* @brief Translates a preset's name into a real name.
	*
	* @param object Preset to translate.
	* @return Translated name of the preset.
	*/
	RENX_API std::string_view translateName(std::string_view object);

	/**
	* @brief Translates a WinType into a human-readable string.
	* Example:
	*
	* @param winType WinType to translate.
	* @return String containing the human-readable translation of the WinType.
	*/
	RENX_API std::string_view translateWinType(WinType winType);

	/**
	* @brief Translates a WinType into a human-readable string.
	* Example: Base Destruction
	*
	* @param winType WinType to translate.
	* @return String containing the human-readable translation of the WinType.
	*/
	RENX_API std::string_view translateWinTypePlain(WinType winType);

	/**
	* @brief Initializes translation definitions.
	*
	* @param filename Optional parameter to specify which file to load.
	*/
	RENX_API void initTranslations(Jupiter::Config &settings);

	/**
	* @brief Creates a String containing an IRC-ready version of the player's name.
	* Note: This includes a bot indicator.
	*
	* @param player Data of the player
	* @return IRC-ready version of the player's name.
	*/
	RENX_API Jupiter::String getFormattedPlayerName(const RenX::PlayerInfo &player);

	/**
	* @brief Creates a string containing a human-readable version of a map's GUID
	*
	* @param map Map containing the GUID to interpret
	* @return Human-readable map GUID
	*/
	RENX_API Jupiter::StringS formatGUID(const RenX::Map &map);

	/**
	* @brief Calculates for how many seconds a player has been in the server.
	*
	* @param player A player's data.
	* @return Number of seconds a player has been in-game.
	*/
	RENX_API std::chrono::milliseconds getServerTime(const RenX::PlayerInfo &player);

	/**
	* @brief Calculates a player's UUID, based on their steam ID.
	*
	* @param server Server the player is in
	* @param player Player to calculate UUID of
	* @return UUID of the player based on their steam ID.
	*/
	RENX_API Jupiter::StringS default_uuid_func(RenX::PlayerInfo &player);

	/**
	* @brief Calculates a player's Kill-Death ratio, based on their data.
	* Note: If the player has not died, this function will use 1 for their death count.
	*
	* @param player A player's data.
	* @param includeSuicides True if suicides should be included in the death count, false otherwise.
	* @return Player's Kill-Death ratio.
	*/
	RENX_API double getKillDeathRatio(const RenX::PlayerInfo &player, bool includeSuicides = true);

	/**
	* @brief Calculates a player's Headshot-Kill ratio, based on their data.
	* Note: If the player has no kills, this function returns 0.
	*
	* @param player A player's data.
	* @return Player's Headshot-Kill ratio.
	*/
	RENX_API double getHeadshotKillRatio(const RenX::PlayerInfo &player);

	/**
	* @brief Escapifies a string so that it can be safely transmitted over RCON.
	*
	* @param str String to escapify
	* @return Escapified version of str.
	*/
	RENX_API std::string escapifyRCON(std::string_view str);

	/** Constant variables */
	RENX_API extern const char DelimC; /** RCON message deliminator */
	RENX_API extern const char DelimC3; /** RCON message deliminator for RCON version number 003 */
	RENX_API extern const std::string_view DevBotName;
}

#ifdef NDEBUG
#define DEBUG_LOG(expr)
#else // NDEBUG
#define DEBUG_LOG(expr) { std::cout << '[' << getTimeFormat("%H:%M:%S") << "] (" << __FILE__ << ":" << __LINE__ << ") " << __func__ << " | " << expr << std::endl; }
#endif // NDEBUG

#endif // _RENX_FUNCTIONS_H_HEADER