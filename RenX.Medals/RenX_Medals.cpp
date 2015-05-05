/**
 * Copyright (C) 2014-2015 Justin James.
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
#include "Jupiter/IRC_Client.h"
#include "Jupiter/DLList.h"
#include "RenX_Medals.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_Core.h"
#include "RenX_Tags.h"

RenX_MedalsPlugin::RenX_MedalsPlugin()
{
	this->INTERNAL_RECS_TAG = RenX::getUniqueInternalTag();
	this->INTERNAL_NOOB_TAG = RenX::getUniqueInternalTag();
	this->INTERNAL_WORTH_TAG = RenX::getUniqueInternalTag();
	init();
}

RenX_MedalsPlugin::~RenX_MedalsPlugin()
{
	RenX::Core *core = RenX::getCore();
	unsigned int sCount = core->getServerCount();
	RenX::Server *server;
	RenX::PlayerInfo *player;
	for (unsigned int i = 0; i < sCount; i++)
	{
		server = core->getServer(i);
		if (server->players.size() != 0)
		{
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				if (player->uuid.isEmpty() == false && player->isBot == false)
				{
					RenX_MedalsPlugin::medalsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("Recs"), player->varData.get(this->getName(), STRING_LITERAL_AS_REFERENCE("Recs")));
					RenX_MedalsPlugin::medalsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("Noobs"), player->varData.get(this->getName(), STRING_LITERAL_AS_REFERENCE("Noobs")));
				}
			}
		}
	}
	RenX_MedalsPlugin::medalsFile.sync(RenX_MedalsPlugin::medalsFileName);
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

void RenX_MedalsPlugin::RenX_SanitizeTags(Jupiter::StringType &fmt)
{
	fmt.replace(RenX_MedalsPlugin::recsTag, this->INTERNAL_RECS_TAG);
	fmt.replace(RenX_MedalsPlugin::noobTag, this->INTERNAL_NOOB_TAG);
	fmt.replace(RenX_MedalsPlugin::worthTag, this->INTERNAL_WORTH_TAG);
}

void RenX_MedalsPlugin::RenX_ProcessTags(Jupiter::StringType &msg, const RenX::Server *server, const RenX::PlayerInfo *player, const RenX::PlayerInfo *victim)
{
	if (player != nullptr)
	{
		const Jupiter::ReadableString &recs = RenX_MedalsPlugin::medalsFile.get(player->uuid, STRING_LITERAL_AS_REFERENCE("Recs"));
		const Jupiter::ReadableString &noobs = RenX_MedalsPlugin::medalsFile.get(player->uuid, STRING_LITERAL_AS_REFERENCE("Noobs"));

		msg.replace(this->INTERNAL_RECS_TAG, recs);
		msg.replace(this->INTERNAL_NOOB_TAG, noobs);
		msg.replace(this->INTERNAL_WORTH_TAG, Jupiter::StringS::Format("%d", recs.asInt() - noobs.asInt()));
	}
}

void RenX_MedalsPlugin::RenX_OnPlayerCreate(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->uuid.isEmpty() == false && player->isBot == false)
	{
		player->varData.set(this->getName(), STRING_LITERAL_AS_REFERENCE("Recs"), RenX_MedalsPlugin::medalsFile.get(player->uuid, STRING_LITERAL_AS_REFERENCE("Recs")));
		player->varData.set(this->getName(), STRING_LITERAL_AS_REFERENCE("Noobs"), RenX_MedalsPlugin::medalsFile.get(player->uuid, STRING_LITERAL_AS_REFERENCE("Noobs")));
	}
}

void RenX_MedalsPlugin::RenX_OnPlayerDelete(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->uuid.isEmpty() == false && player->isBot == false)
	{
		RenX_MedalsPlugin::medalsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("Recs"), player->varData.get(this->getName(), STRING_LITERAL_AS_REFERENCE("Recs")));
		RenX_MedalsPlugin::medalsFile.set(player->uuid, STRING_LITERAL_AS_REFERENCE("Noobs"), player->varData.get(this->getName(), STRING_LITERAL_AS_REFERENCE("Noobs")));
	}
}

void RenX_MedalsPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	if (player->uuid.isEmpty() == false && player->isBot == false)
	{
		int worth = getWorth(player);
		Jupiter::INIFile::Section *section = RenX_MedalsPlugin::joinMessageFile.getSection(RenX_MedalsPlugin::firstSection);
		if (section != nullptr)
		{
			while (section->hasKey(STRING_LITERAL_AS_REFERENCE("MaxRecs")) && section->get(STRING_LITERAL_AS_REFERENCE("MaxRecs")).asInt() < worth)
				if ((section = RenX_MedalsPlugin::joinMessageFile.getSection(section->get(STRING_LITERAL_AS_REFERENCE("NextSection")))) == nullptr)
					return; // No matching section found.

			if (section->hasKey(STRING_LITERAL_AS_REFERENCE("1")))
			{
				int r;
				Jupiter::INIFile::Section::KeyValuePair *pair;
				do
				{
					r = rand() % section->size();
					pair = section->getPair(r);
				} while (pair->getKey().asInt() == 0);

				Jupiter::StringS msg = pair->getValue();
				RenX::sanitizeTags(msg);
				RenX::processTags(msg, server, player);

				server->sendMessage(msg);
			}
		}
	}
}

void RenX_MedalsPlugin::RenX_OnGameOver(RenX::Server *server, RenX::WinType winType, const RenX::TeamType &team, int gScore, int nScore)
{
	if (server->isFirstGame() == false) // No unfair medals for the first game! :D
	{
		if (server->players.size() == 0) return;
		Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0);
		RenX::PlayerInfo *pInfo = n->data;
		RenX::PlayerInfo *mostKills = pInfo;
		RenX::PlayerInfo *mostVehicleKills = pInfo;
		RenX::PlayerInfo *bestKD = pInfo;

		while (n != nullptr)
		{
			pInfo = n->data;
			if (pInfo->kills > mostKills->kills)
				mostKills = pInfo;

			if (pInfo->vehicleKills > mostVehicleKills->vehicleKills)
				mostVehicleKills = pInfo;

			if (RenX::getKillDeathRatio(pInfo) > RenX::getKillDeathRatio(bestKD))
				bestKD = pInfo;

			n = n->next;
		}

		CongratPlayerData *congratPlayerData;

		/** +1 for most kills */
		if (mostKills->uuid.isEmpty() == false && mostKills->isBot == false && mostKills->kills > 0)
		{
			addRec(mostKills);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = server;
			congratPlayerData->playerName = mostKills->name;
			congratPlayerData->type = 1;
			new Jupiter::Timer(1, killCongratDelay, congratPlayer, congratPlayerData, false);
		}

		/** +1 for most Vehicle kills */
		if (mostVehicleKills->uuid.isEmpty() == false && mostVehicleKills->isBot == false && mostVehicleKills->vehicleKills > 0)
		{
			addRec(mostVehicleKills);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = server;
			congratPlayerData->playerName = mostVehicleKills->name;
			congratPlayerData->type = 2;
			new Jupiter::Timer(1, vehicleKillCongratDelay, congratPlayer, congratPlayerData, false);
		}

		/** +1 for best K/D ratio */
		if (bestKD->uuid.isEmpty() == false && bestKD->isBot == false && RenX::getKillDeathRatio(bestKD) > 1.0)
		{
			addRec(bestKD);

			congratPlayerData = new CongratPlayerData();
			congratPlayerData->server = server;
			congratPlayerData->playerName = bestKD->name;
			congratPlayerData->type = 3;
			new Jupiter::Timer(1, kdrCongratDelay, congratPlayer, congratPlayerData, false);
		}
	}

	RenX_MedalsPlugin::medalsFile.sync(medalsFileName);
}

void RenX_MedalsPlugin::RenX_OnDestroy(RenX::Server *server, const RenX::PlayerInfo *player, const Jupiter::ReadableString &objectName, const RenX::TeamType &objectTeam, const Jupiter::ReadableString &damageType, RenX::ObjectType type)
{
	if (type == RenX::ObjectType::Building)
	{
		addRec(player);

		const Jupiter::ReadableString &translated = RenX::translateName(objectName);
		server->sendMessage(Jupiter::StringS::Format("%.*s has been recommended for destroying the %.*s!", player->name.size(), player->name.ptr(), translated.size(), translated.ptr()));
	}
}

int RenX_MedalsPlugin::OnRehash()
{
	RenX_MedalsPlugin::medalsFile.sync(RenX_MedalsPlugin::medalsFileName);
	RenX_MedalsPlugin::medalsFile.flushData();
	RenX_MedalsPlugin::joinMessageFile.flushData();
	init();
	return 0;
}

void RenX_MedalsPlugin::init()
{
	RenX_MedalsPlugin::killCongratDelay = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("KillCongratDelay"), 60);
	RenX_MedalsPlugin::vehicleKillCongratDelay = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("VehicleKillCongratDelay"), 60);
	RenX_MedalsPlugin::kdrCongratDelay = Jupiter::IRC::Client::Config->getInt(this->getName(), STRING_LITERAL_AS_REFERENCE("KDRCongratDelay"), 60);
	RenX_MedalsPlugin::medalsFileName = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("MedalsFile"), STRING_LITERAL_AS_REFERENCE("Medals.ini"));
	RenX_MedalsPlugin::joinMessageFileName = Jupiter::IRC::Client::Config->get(this->getName(), STRING_LITERAL_AS_REFERENCE("JoinMessageFile"), STRING_LITERAL_AS_REFERENCE("Medals.Join.ini"));
	RenX_MedalsPlugin::medalsFile.readFile(RenX_MedalsPlugin::medalsFileName);
	RenX_MedalsPlugin::joinMessageFile.readFile(RenX_MedalsPlugin::joinMessageFileName);
	RenX_MedalsPlugin::firstSection = RenX_MedalsPlugin::joinMessageFile.get(Jupiter::StringS::empty, STRING_LITERAL_AS_REFERENCE("FirstSection"));
	RenX_MedalsPlugin::recsTag = RenX_MedalsPlugin::joinMessageFile.get(Jupiter::String::empty, STRING_LITERAL_AS_REFERENCE("RecsTag"), STRING_LITERAL_AS_REFERENCE("{RECS}"));
	RenX_MedalsPlugin::noobTag = RenX_MedalsPlugin::joinMessageFile.get(Jupiter::String::empty, STRING_LITERAL_AS_REFERENCE("NoobsTag"), STRING_LITERAL_AS_REFERENCE("{NOOBS}"));
	RenX_MedalsPlugin::worthTag = RenX_MedalsPlugin::joinMessageFile.get(Jupiter::String::empty, STRING_LITERAL_AS_REFERENCE("WorthTag"), STRING_LITERAL_AS_REFERENCE("{WORTH}"));

	RenX::Core *core = RenX::getCore();
	unsigned int sCount = core->getServerCount();
	RenX::Server *server;
	RenX::PlayerInfo *player;
	for (unsigned int i = 0; i < sCount; i++)
	{
		server = core->getServer(i);
		if (server->players.size() != 0)
		{
			for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
			{
				player = n->data;
				player->varData.set(this->getName(), STRING_LITERAL_AS_REFERENCE("Recs"), RenX_MedalsPlugin::medalsFile.get(player->name, STRING_LITERAL_AS_REFERENCE("Recs")));
				player->varData.set(this->getName(), STRING_LITERAL_AS_REFERENCE("Noobs"), RenX_MedalsPlugin::medalsFile.get(player->name, STRING_LITERAL_AS_REFERENCE("Noobs")));
			}
		}
	}
}

/** Game Commands */

/** Instance of RenX_MedalsPlugin */
RenX_MedalsPlugin pluginInstance;

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
			Jupiter::INIFile::Section *section = pluginInstance.medalsFile.getSection(parameters);
			if (section == nullptr)
				source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found! Syntax: recs [player]"));
			else
			{
				unsigned int recs = section->get(STRING_LITERAL_AS_REFERENCE("Recs")).asUnsignedInt();
				unsigned int noobs = section->get(STRING_LITERAL_AS_REFERENCE("Noobs")).asUnsignedInt();
				source->sendMessage(player, Jupiter::StringS::Format("[Archive] %.*s has %u and %u n00bs. Their worth: %d", section->getName().size(), section->getName().ptr(), recs, noobs, recs - noobs));
			}
		}
		else if (target->uuid.isEmpty())
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is not using steam."));
		else if (target->isBot)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Bots do not have any recommendations."));
		else if (target == player)
			RecsGameCommand::trigger(source, player, Jupiter::ReferenceString::empty);
		else source->sendMessage(player, Jupiter::StringS::Format("%.*s has %lu and %lu n00bs. Their worth: %d", target->name.size(), target->name.ptr(), getRecs(target), getNoobs(target), getWorth(target)));
	}
	else if (player->uuid.isEmpty())
		source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: You are not using steam."));
	else source->sendMessage(player, Jupiter::StringS::Format("%.*s, you have %lu recs and %lu n00bs. Your worth: %d", player->name.size(), player->name.ptr(), getRecs(player), getNoobs(player), getWorth(player)));
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
		if (target == nullptr)
			target = source->getPlayerByPartName(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE));
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found! Syntax: rec <player>"));
		else if (target->uuid.isEmpty())
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is not using steam."));
		else if (target->isBot)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Bots can not receive recommendations."));
		else if (target == player)
		{
			addNoob(player);
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("You can't recommend yourself, you noob! (+1 noob)"));
		}
		else if (player->varData.get(STRING_LITERAL_AS_REFERENCE("RenX.Medals"), STRING_LITERAL_AS_REFERENCE("gr")) != nullptr && player->adminType.isEmpty())
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
		if (target == nullptr)
			target = source->getPlayerByPartName(Jupiter::ReferenceString::getWord(parameters, 0, WHITESPACE));
		if (target == nullptr)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player not found! Syntax: noob [player]"));
		else if (target->uuid.isEmpty())
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Player is not using steam."));
		else if (target->isBot)
			source->sendMessage(player, STRING_LITERAL_AS_REFERENCE("Error: Bots can not receive n00bs."));
		else if (player->varData.get(STRING_LITERAL_AS_REFERENCE("RenX.Medals"), STRING_LITERAL_AS_REFERENCE("gn")) != nullptr && player->adminType.isEmpty())
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

void addRec(const RenX::PlayerInfo *player, int amount)
{
	if (player->uuid.matchi("Player*") == false && player->isBot == false)
		player->varData.set(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Recs"), Jupiter::StringS::Format("%u", getRecs(player) + amount));
}

void addNoob(const RenX::PlayerInfo *player, int amount)
{
	if (player->uuid.matchi("Player*") == false && player->isBot == false)
		player->varData.set(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Noobs"), Jupiter::StringS::Format("%u", getNoobs(player) + amount));
}

unsigned long getRecs(const RenX::PlayerInfo *player)
{
	return player->varData.getInt(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Recs"));
}

unsigned long getNoobs(const RenX::PlayerInfo *player)
{
	return player->varData.getInt(pluginInstance.getName(), STRING_LITERAL_AS_REFERENCE("Noobs"));
}

int getWorth(const RenX::PlayerInfo *player)
{
	return getRecs(player) - getNoobs(player);
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
