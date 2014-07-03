/**
 * Copyright (C) 2014 Justin James.
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

#if !defined _RENX_FUNCTIONS_H_HEADER
#define _RENX_FUNCTIONS_H_HEADER

/**
 * @file RenX_Functions.h
 * @brief Provides some functions to be used at your convenience.
 */

#include "Jupiter/INIFile.h"
#include "Jupiter/String.h"
#include "RenX.h"

namespace RenX
{
	/** Forward declarations */
	struct PlayerInfo;
	class Server;

	/**
	* @brief Translates the first character of a team's name into a TeamType.
	*
	* @param First character of the team name.
	* @return A corresponding TeamType.
	*/
	RENX_API TeamType getTeam(char team);

	/**
	* @brief Fetches the corresponding IRC color code for a team.
	*
	* @param team Team to get IRC color code for
	* @return A team's corresponding IRC color-code.
	*/
	RENX_API const Jupiter::ReadableString &getTeamColor(TeamType team);

	/**
	* @brief Fetches a team's name.
	* Example: GDI
	*
	* @param team Team to translate into a name.
	* @return The team's corresponding name.
	*/
	RENX_API const Jupiter::ReadableString &getTeamName(TeamType team);

	/**
	* @brief Fetches a team's full name.
	* Example: Global Defense Initiative
	*
	* @param team Team to translate into a name.
	* @return The team's corresponding name.
	*/
	RENX_API const Jupiter::ReadableString &getFullTeamName(TeamType team);

	/**
	* @brief Fetches a team's enemy.
	*
	* @param team TeamType to return enemy for.
	* @return Enemy TeamType of the input team.
	*/
	RENX_API TeamType getEnemy(TeamType team);

	/**
	* @brief Translates a preset's name into a real name.
	*
	* @param object Preset to translate.
	* @return Translated name of the preset.
	*/
	RENX_API const Jupiter::ReadableString &translateName(const Jupiter::ReadableString &object);

	/**
	* @brief Translates a WinType into a human-readable string.
	* Example:
	*
	* @param winType WinType to translate.
	* @return String containing the human-readable translation of the WinType.
	*/
	RENX_API const Jupiter::ReadableString &translateWinType(WinType winType);

	/**
	* @brief Translates a WinType into a human-readable string.
	* Example: Base Destruction
	*
	* @param winType WinType to translate.
	* @return String containing the human-readable translation of the WinType.
	*/
	RENX_API const Jupiter::ReadableString &translateWinTypePlain(WinType winType);

	/**
	* @brief Initializes translation definitions.
	*
	* @param filename Optional parameter to specify which file to load.
	*/
	RENX_API void initTranslations(Jupiter::INIFile &settings);

	/**
	* @brief Creates a String containing an IRC-ready version of the player's name.
	* Note: This includes a bot indicator.
	*
	* @param player Data of the player
	* @return IRC-ready version of the player's name.
	*/
	RENX_API Jupiter::String getFormattedPlayerName(const RenX::PlayerInfo *player);

	/**
	* @brief Sanitizes a string into a RCON-ready state.
	* Note: This resolves the pipe character ('|') exploit.
	*
	* @brief str String to sanitize.
	*/
	RENX_API void sanitizeString(char *str);

	/**
	* @brief Calculates for how many seconds a player has been in-game.
	* Note: This will always be at least 1.
	*
	* @param player A player's data.
	* @return Number of seconds a player has been in-game.
	*/
	RENX_API time_t getGameTime(const RenX::PlayerInfo *player);

	/**
	* @brief Calculates a player's Kill-Death ratio, based on their data.
	* Note: If the player has not died, this function will use 1 for their death count.
	*
	* @param player A player's data.
	* @param includeSuicides True if suicides should be included in the death count, false otherwise.
	* @return Player's Kill-Death ratio.
	*/
	RENX_API double getKillDeathRatio(const RenX::PlayerInfo *player, bool includeSuicides = true);

	/**
	* @brief Calculates a player's Headshot-Kill ratio, based on their data.
	* Note: If the player has no kills, this function returns 0.
	*
	* @param player A player's data.
	* @return Player's Headshot-Kill ratio.
	*/
	RENX_API double getHeadshotKillRatio(const RenX::PlayerInfo *player);

	/**
	* @brief Calculates a player's kills-per-second since joining.
	*
	* @param player A player's data.
	* @return Player's Kill-Death ratio.
	*/
	RENX_API double getKillsPerSecond(const RenX::PlayerInfo *player);

	/** Constant variables */
	RENX_API extern const char *DelimS; /** RCON message deliminator in a C-Style string */
	RENX_API extern const char DelimC; /** RCON message deliminator as a character */
}

#endif // _RENX_FUNCTIONS_H_HEADER