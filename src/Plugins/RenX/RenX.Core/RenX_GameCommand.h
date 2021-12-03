/**
 * Copyright (C) 2014-2021 Jessica James.
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

#if !defined _RENX_GAMECOMMAND_H_HEADER
#define _RENX_GAMECOMMAND_H_HEADER

/**
 * @file RenX_GameCommand.h
 * @brief Provides the basis of the in-game Renegade-X chat command system.
 */

#include "Jupiter/Command.h"
#include "RenX.h"
#include "RenX_Core.h" // getCore().

 /** DLL Linkage Nagging */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace RenX
{
	/** Forward delcarations */
	class Server;
	struct PlayerInfo;
	class GameCommand;

	/** Master command list */
	RENX_API extern std::vector<GameCommand*>& GameMasterCommandList;

	/**
	* @brief Provides an extendable interface from which in-game commands can be created.
	*/
	class RENX_API GameCommand : public Jupiter::Command
	{
	public:

		/**
		* @brief Fetches a command's minimum access level.
		*
		* @return Minimum access level.
		*/
		int getAccessLevel();

		/**
		* @brief Sets a command's minimum access level.
		*
		* @param accessLevel Access level.
		*/
		void setAccessLevel(int accessLevel);

		/**
		* @brief Called when a player with the proper access privledges executes this command.
		*
		* @param source Renegade-X server where the player is located.
		* @param player Player who executed the command.
		* @param parameters Parameters following the command.
		*/
		virtual void trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters) = 0;

		/**
		* @brief Called when the command is intially created. Define triggers and access levels here.
		*/
		virtual void create() = 0;

		/**
		* @brief Used internally to provide per-server configurable commands.
		* Note: This is automatically generated by the GAME_COMMAND_INIT macro.
		*/
		virtual GameCommand *copy() = 0;

		/**
		* @brief Same as the Default constructor, except that the command is not added to the master command list.
		*/
		GameCommand(std::nullptr_t);

		/**
		* @brief Copy constructor for a Game Command.
		* Note: This is automatically generated by the GENERIC_GAME_COMMAND macro.
		*/
		GameCommand(const GameCommand &command);

		/**
		* @brief Default constructor for a Game Command.
		* Note: This is automatically generated by the BASE_GAME_COMMAND macro.
		*/
		GameCommand();

		/**
		* @brief Destructor for a Game Command.
		* Note: This is not automatically generated by any macro, and is available for use.
		*/
		virtual ~GameCommand();

		/** Public members */

		static RenX::Server *active_server; /** Currently active IRC server */
		static RenX::Server *selected_server; /** Currently selected IRC server */

	private:
		int access = 0; /** Minimum access level */
	};

	/**
	* @brief Provides a simple interface for implementing basic text-to-player commands.
	*/
	class RENX_API BasicGameCommand : public RenX::GameCommand
	{
	public:
		void trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters);
		std::string_view getHelp(std::string_view parameters) override;
		BasicGameCommand *copy();
		void create();
		BasicGameCommand();
		BasicGameCommand(BasicGameCommand &c);
		BasicGameCommand(std::string_view trigger, std::string_view in_message, std::string_view in_help_message);

	private:
		std::string m_message, m_help_message;
	};
}

/** Game Command Macros */

/** Defines the core of a game command's declaration. This should be included in every game command. */
#define BASE_GAME_COMMAND(CLASS) \
	public: \
	void trigger(RenX::Server *source, RenX::PlayerInfo *player, std::string_view parameters); \
	std::string_view getHelp(std::string_view parameters) override; \
	CLASS *copy(); \
	void create(); \
	CLASS() { this->create(); RenX::getCore()->addCommand(this); }

/** Expands to become the entire declaration for a game command. In most cases, this will be sufficient. */
#define GENERIC_GAME_COMMAND(CLASS) \
class CLASS : public RenX::GameCommand { \
	BASE_GAME_COMMAND(CLASS) \
	CLASS(CLASS &c) : RenX::GameCommand(c) { this->create(); } };

/** Instantiates a game command, and also defines neccessary core functions for compatibility. */
#define GAME_COMMAND_INIT(CLASS) \
	CLASS CLASS ## _instance; \
	CLASS *CLASS::copy() { return new CLASS(*this); }

/** Re-enable warnings */
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // _RENX_GAMECOMMAND_H_HEADER