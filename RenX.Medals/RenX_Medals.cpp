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

#include <ctime>
#include "Jupiter/Timer.h"
#include "RenX_Medals.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"

//#error "Do not use this plugin -- it will be retwritten."
const char *medalsFileName = "medals.ini";

struct MedalPlayerData
{
	Jupiter::StringS uuid;
	unsigned long recs = 0;
	unsigned long noobs = 0;
	Jupiter::StringS lastName;
	time_t lastSeen = 0;

	MedalPlayerData(Jupiter::INIFile &medalsFile, const Jupiter::ReadableString &id)
	{
		MedalPlayerData::uuid = id;
		if (medalsFile.getSectionIndex(MedalPlayerData::uuid) != Jupiter::INVALID_INDEX)
		{
			MedalPlayerData::recs = medalsFile.get(MedalPlayerData::uuid, STRING_LITERAL_AS_REFERENCE("Recs")).asUnsignedLongLong(10);
			MedalPlayerData::noobs = medalsFile.get(MedalPlayerData::uuid, STRING_LITERAL_AS_REFERENCE("Noobs")).asUnsignedLongLong(10);
			MedalPlayerData::lastName = medalsFile.get(MedalPlayerData::uuid, STRING_LITERAL_AS_REFERENCE("Name"));
			MedalPlayerData::lastSeen = medalsFile.getInt(MedalPlayerData::uuid, STRING_LITERAL_AS_REFERENCE("Seen"));
		}
	}
};

RenX_MedalsPlugin::RenX_MedalsPlugin()
{
	medalsFile.readFile(medalsFileName);
}

void updateMedalPlayerData(MedalPlayerData &player, RenX::PlayerInfo *pInfo)
{
	player.lastName = pInfo->name;
	player.lastSeen = time(0);
}

void syncPlayerMedalDataToINI(Jupiter::INIFile &file, MedalPlayerData &player, const Jupiter::ReadableString &uuid)
{
	file.set(uuid, STRING_LITERAL_AS_REFERENCE("Name"), player.lastName);
	file.set(uuid, STRING_LITERAL_AS_REFERENCE("Seen"), Jupiter::StringS::Format("%d", player.lastSeen));
}

struct CongratPlayerData
{
	RenX::Server *server;
	Jupiter::StringS playerName;
	unsigned int type;
};

void congratPlayer(unsigned int, void *params)
{
	CongratPlayerData *congratPlayerData = (CongratPlayerData *) params;

	switch (congratPlayerData->type)
	{
	case 1:
		congratPlayerData->server->sendMessage(Jupiter::StringS::Format("%.*s has been recommended for having the most kills last game!", congratPlayerData->playerName.size(), congratPlayerData->playerName.ptr()));
		break;
	case 2:
		congratPlayerData->server->sendMessage(Jupiter::StringS::Format("%.*s has been recommended for having the most vehicle kills last game!", congratPlayerData->playerName.size(), congratPlayerData->playerName.ptr()));
		break;
	case 3:
		congratPlayerData->server->sendMessage(Jupiter::StringS::Format("%.*s has been recommended for having the highest Kill-Death ratio last game!", congratPlayerData->playerName.size(), congratPlayerData->playerName.ptr()));
		break;
	default:
		break;
	}
	delete congratPlayerData;
}

void RenX_MedalsPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
{
	if (firstGame)
	{
		firstGame = false;
		return;
	}
	if (server->players.size() == 0) return;
	Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0);
	RenX::PlayerInfo *pInfo = n->data;
	RenX::PlayerInfo *mostKills = pInfo;
	RenX::PlayerInfo *mostVehicleKills = pInfo;
	RenX::PlayerInfo *bestKD = pInfo;

	while (n != nullptr)
	{
		pInfo = n->data;
		if (pInfo->kills > mostKills->kills) mostKills = pInfo;
		if (pInfo->vehicleKills > mostVehicleKills->vehicleKills) mostVehicleKills = pInfo;
		if (RenX::getKillDeathRatio(pInfo) > RenX::getKillDeathRatio(bestKD)) bestKD = pInfo;

		if (pInfo->uuid.isEmpty() == false)
		{
			MedalPlayerData player(RenX_MedalsPlugin::medalsFile, pInfo->uuid);
			updateMedalPlayerData(player, pInfo);
			syncPlayerMedalDataToINI(RenX_MedalsPlugin::medalsFile, player, pInfo->uuid);
		}
		n = n->next;
	}

	CongratPlayerData *congratPlayerData;

	/** +1 for most kills */
	if (mostKills->uuid.isEmpty() == false && mostKills->kills > 0)
	{
		addRec(mostKills);
		MedalPlayerData player(RenX_MedalsPlugin::medalsFile, mostKills->uuid);
		syncPlayerMedalDataToINI(RenX_MedalsPlugin::medalsFile, player, mostKills->uuid);

		congratPlayerData = new CongratPlayerData();
		congratPlayerData->server = server;
		congratPlayerData->playerName = mostKills->name;
		congratPlayerData->type = 1;
		new Jupiter::Timer(1, 60, congratPlayer, congratPlayerData, false);
	}

	/** +1 for most Vehicle kills */
	if (mostVehicleKills->uuid.isEmpty() == false && mostVehicleKills->vehicleKills > 0)
	{
		addRec(mostVehicleKills);
		MedalPlayerData player(RenX_MedalsPlugin::medalsFile, mostVehicleKills->uuid);
		syncPlayerMedalDataToINI(RenX_MedalsPlugin::medalsFile, player, mostVehicleKills->uuid);

		congratPlayerData = new CongratPlayerData();
		congratPlayerData->server = server;
		congratPlayerData->playerName = mostVehicleKills->name;
		congratPlayerData->type = 2;
		new Jupiter::Timer(1, 60, congratPlayer, congratPlayerData, false);
	}

	/** +1 for best K/D ratio */
	if (bestKD->uuid.isEmpty() == false)
	{
		addRec(bestKD);
		MedalPlayerData player(RenX_MedalsPlugin::medalsFile, bestKD->uuid);
		syncPlayerMedalDataToINI(RenX_MedalsPlugin::medalsFile, player, bestKD->uuid);

		congratPlayerData = new CongratPlayerData();
		congratPlayerData->server = server;
		congratPlayerData->playerName = bestKD->name;
		congratPlayerData->type = 3;
		new Jupiter::Timer(1, 60, congratPlayer, congratPlayerData, false);
	}

	RenX_MedalsPlugin::medalsFile.sync();
}

void RenX_MedalsPlugin::RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
{
	if (type == RenX::ObjectType::Building)
	{
		addRec((RenX::PlayerInfo *) player); // -whistles-
		MedalPlayerData playerMedalData(RenX_MedalsPlugin::medalsFile, player->uuid);
		updateMedalPlayerData(playerMedalData, (RenX::PlayerInfo *) player); // -whistles-
		syncPlayerMedalDataToINI(RenX_MedalsPlugin::medalsFile, playerMedalData, player->uuid);

		const Jupiter::ReadableString &translated = RenX::translateName(objectName);
		server->sendMessage(Jupiter::StringS::Format("%.*s has been recommended for destroying the %.*s!", player->name.size(), player->name.ptr(), translated.size(), translated.ptr()));
	}
}

// Recommendations Game Command

void RecsGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("recs"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("recommends"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("recommendations"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("noobs"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("n00bs"));
}

void RecsGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.size() != 0)
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr)
		{
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found! Syntax: recs [player]"));
			// Check INI?
		}
		else if (target->uuid.isEmpty())
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is not using steam."));
		else if (target == player)
			RecsGameCommand::trigger(source, player, Jupiter::ReferenceString::empty);
		else source->sendMessage(Jupiter::StringS::Format("%.*s has %lu and %lu n00bs. Their worth: %d", target->name.size(), target->name.ptr(), getRecs(target), getNoobs(target), getWorth(target)));
	}
	else if (player->uuid.isEmpty())
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You are not using steam."));
	else source->sendMessage(Jupiter::StringS::Format("%.*s, you have %lu and %lu n00bs. Your worth: %d", player->name.size(), player->name.ptr(), getRecs(player), getNoobs(player), getWorth(player)));
}

const Jupiter::ReadableString &RecsGameCommand::getHelp(const Jupiter::ReadableString &)
{
	STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Gets a count of a player's recommendations and noobs. Syntax: recs [player]");
	return defaultHelp;
}

GAME_COMMAND_INIT(RecsGameCommand)

// Recommend Game Command

void RecGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("rec"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("recommend"));
}

void RecGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.size() != 0)
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr) source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found! Syntax: rec <player>"));
		else if (target->uuid.isEmpty())
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is not using steam."));
		else if (target == player)
		{
			addNoob(player);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("You can't recommend yourself, you noob! (+1 noob)"));
		}
		else if (player->varData.get(STRING_LITERAL_AS_REFERENCE("RenX.Medals"), STRING_LITERAL_AS_REFERENCE("gr")).isEmpty() == false)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("You can only give one recommendation per game."));
		else
		{
			addRec(target);
			source->sendMessage(Jupiter::StringS::Format("%.*s has recommended %.*s!", player->name.size(), player->name.ptr(), target->name.size(), target->name.ptr()));
			player->varData.set(STRING_LITERAL_AS_REFERENCE("RenX.Medals"), STRING_LITERAL_AS_REFERENCE("gr"), STRING_LITERAL_AS_REFERENCE("1"));
		}
	}
	else RecsGameCommand_instance.trigger(source, player, parameters);
}

const Jupiter::ReadableString &RecGameCommand::getHelp(const Jupiter::ReadableString &)
{
	STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Recommends a player for their gameplay. Syntax: rec [player]");
	return defaultHelp;
}

GAME_COMMAND_INIT(RecGameCommand)

// Noob Game Command

void NoobGameCommand::create()
{
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("noob"));
	this->addTrigger(STRING_LITERAL_AS_REFERENCE("n00b"));
}

void NoobGameCommand::trigger(RenX::Server *source, RenX::PlayerInfo *player, const Jupiter::ReadableString &parameters)
{
	if (parameters.size() != 0)
	{
		RenX::PlayerInfo *target = source->getPlayerByPartName(parameters);
		if (target == nullptr) source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found! Syntax: noob [player]"));
		else if (target->uuid.isEmpty())
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is not using steam."));
		else if (player->varData.get(STRING_LITERAL_AS_REFERENCE("RenX.Medals"), STRING_LITERAL_AS_REFERENCE("gn")) != nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("You can only give one noob per game."));
		else
		{
			addNoob(target);
			source->sendMessage(Jupiter::StringS::Format("%.*s has noob'd %.*s!", player->name.size(), player->name.ptr(), target->name.size(), target->name.ptr()));
			player->varData.set(STRING_LITERAL_AS_REFERENCE("RenX.Medals"), STRING_LITERAL_AS_REFERENCE("gn"), STRING_LITERAL_AS_REFERENCE("1"));
		}
	}
	else RecsGameCommand_instance.trigger(source, player, parameters);
}

const Jupiter::ReadableString &NoobGameCommand::getHelp(const Jupiter::ReadableString &)
{
	STRING_LITERAL_AS_NAMED_REFERENCE(defaultHelp, "Tells people that a player is bad. Syntax: noob [player]");
	return defaultHelp;
}

GAME_COMMAND_INIT(NoobGameCommand)

// Plugin instantiation and entry point.
RenX_MedalsPlugin pluginInstance;


void addRec(MedalPlayerData &player, int amount)
{
	player.recs += amount;
	pluginInstance.medalsFile.set(player.uuid, STRING_LITERAL_AS_REFERENCE("Recs"), Jupiter::StringS::Format("%u", player.recs));
}

void addNoob(MedalPlayerData &player, int amount)
{
	player.noobs += amount;
	pluginInstance.medalsFile.set(player.uuid, STRING_LITERAL_AS_REFERENCE("Noobs"), Jupiter::StringS::Format("%u", player.noobs));
}

int getWorth(MedalPlayerData &player)
{
	return player.recs - player.noobs;
}

inline MedalPlayerData getMedalPlayerData(RenX::PlayerInfo *player)
{
	return MedalPlayerData(pluginInstance.medalsFile, player->uuid);
}

void addRec(RenX::PlayerInfo *player, int amount)
{
	addRec(getMedalPlayerData(player), amount);
}

void addNoob(RenX::PlayerInfo *player, int amount)
{
	addNoob(getMedalPlayerData(player), amount);
}

unsigned long getRecs(RenX::PlayerInfo *player)
{
	return getMedalPlayerData(player).recs;
}

unsigned long getNoobs(RenX::PlayerInfo *player)
{
	return getMedalPlayerData(player).noobs;
}

int getWorth(RenX::PlayerInfo *player)
{
	return getWorth(getMedalPlayerData(player));
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
