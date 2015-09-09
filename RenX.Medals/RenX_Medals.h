/**
 * Copyright (C) 2014-2015 Justin James.
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
 * Written by Justin James <justin.aj@hotmail.com>
 */

#if !defined _RENX_MEDALS_H_HEADER
#define _RENX_MEDALS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/String.h"
#include "RenX_Plugin.h"
#include "RenX_GameCommand.h"

/** Adds a recommendation to the player's medal data */
void addRec(const RenX::PlayerInfo *player, int amount = 1);

/** Adds a noob to the player's medal data */
void addNoob(const RenX::PlayerInfo *player, int amount = 1);

/** Fetches a player's recommendation count */
unsigned long getRecs(const RenX::PlayerInfo *player);

/** Fetches a player's noob count */
unsigned long getNoobs(const RenX::PlayerInfo *player);

/** Calculates a player's worth (recs - noobs) */
int getWorth(const RenX::PlayerInfo *player);

class RenX_MedalsPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_SanitizeTags(Jupiter::StringType &fmt) override;
	void RenX_ProcessTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim, const RenX::BuildingInfo *building) override;
	void RenX_OnPlayerCreate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnPlayerDelete(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore) override;
	void RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type) override;
	RenX_MedalsPlugin();
	~RenX_MedalsPlugin();

public: // Jupiter::Plugin
	int OnRehash() override;
	const Jupiter::ReadableString &getName() override { return name; }

public:
	time_t killCongratDelay;
	time_t vehicleKillCongratDelay;
	time_t kdrCongratDelay;
	Jupiter::StringS recsTag;
	Jupiter::StringS noobTag;
	Jupiter::StringS worthTag;
	Jupiter::StringS firstSection;
	Jupiter::StringS medalsFileName;
	Jupiter::StringS joinMessageFileName;
	Jupiter::INIFile medalsFile;
	Jupiter::INIFile joinMessageFile;

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Medals");
	Jupiter::StringS INTERNAL_RECS_TAG;
	Jupiter::StringS INTERNAL_NOOB_TAG;
	Jupiter::StringS INTERNAL_WORTH_TAG;
	void init();
};

GENERIC_GAME_COMMAND(RecsGameCommand)
GENERIC_GAME_COMMAND(RecGameCommand)
GENERIC_GAME_COMMAND(NoobGameCommand)

#endif // _RENX_MEDALS_H_HEADER