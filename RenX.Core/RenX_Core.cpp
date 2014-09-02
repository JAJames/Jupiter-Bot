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
#include "Jupiter/INIFile.h"
#include "Jupiter/Functions.h"
#include "IRC_Bot.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_Functions.h"
#include "RenX_GameCommand.h"
#include "RenX_Plugin.h"

RenX::Core pluginInstance;
RenX::Core *RenXInstance = &pluginInstance;

RenX::Core *RenX::getCore()
{
	return &pluginInstance;
}

RenX::Core::Core()
{
	const Jupiter::ReadableString &serverList = Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("Servers"));
	RenX::Core::translationsFile.readFile(Jupiter::IRC::Client::Config->get(STRING_LITERAL_AS_REFERENCE("RenX"), STRING_LITERAL_AS_REFERENCE("TranslationsFile"), STRING_LITERAL_AS_REFERENCE("Translations.ini")));

	RenX::initTranslations(RenX::Core::translationsFile);
	unsigned int wc = serverList.wordCount(WHITESPACE);

	RenX::Server *server;
	for (unsigned int i = 0; i != wc; i++)
	{
		server = new RenX::Server(Jupiter::ReferenceString::getWord(serverList, i, WHITESPACE));

		if (server->connect() == false)
		{
			fprintf(stderr, "[RenX] ERROR: Failed to connect to %.*s on port %u. Error code: %d" ENDL, server->getHostname().size(), server->getHostname().ptr(), server->getPort(), Jupiter::Socket::getLastError());
			delete server;
		}
		else RenX::Core::addServer(server);
	}
}

RenX::Core::~Core()
{
	RenX::Core::servers.emptyAndDelete();
}

int RenX::Core::send(RenX::Server *server, const Jupiter::ReadableString &msg)
{
	return server->send(msg);
}

unsigned int RenX::Core::send(int type, const Jupiter::ReadableString &msg)
{
	unsigned int r = 0;
	RenX::Server *server;
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		server = RenX::Core::getServer(i);
		if (server->isLogChanType(type) && RenX::Core::send(server, msg) > 0) r++;
	}
	return r;
}

void RenX::Core::addServer(RenX::Server *server)
{
	RenX::Core::servers.add(server);
}

int RenX::Core::getServerIndex(RenX::Server *server)
{
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
		if (server == RenX::Core::servers.get(i))
			return i;
	return -1;
}

RenX::Server *RenX::Core::getServer(unsigned int index)
{
	return RenX::Core::servers.get(index);
}

Jupiter::ArrayList<RenX::Server> RenX::Core::getServers(int type)
{
	Jupiter::ArrayList<RenX::Server> r;
	RenX::Server *server;
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		server = RenX::Core::servers.get(i);
		if (server != nullptr && server->isLogChanType(type))
			r.add(server);
	}
	return r;
}

void RenX::Core::removeServer(unsigned int index)
{
	delete RenX::Core::servers.remove(index);
}

int RenX::Core::removeServer(RenX::Server *server)
{
	int i = RenX::Core::getServerIndex(server);
	if (i >= 0) delete RenX::Core::servers.remove(i);
	return i;
}

unsigned int RenX::Core::getServerCount()
{
	return RenX::Core::servers.size();
}

Jupiter::ArrayList<RenX::Plugin> *RenX::Core::getPlugins()
{
	return &(RenX::Core::plugins);
}

Jupiter::INIFile &RenX::Core::getTranslationsFile()
{
	return RenX::Core::translationsFile;
}

int RenX::Core::addCommand(RenX::GameCommand *command)
{
	for (size_t i = 0; i != RenX::Core::servers.size(); i++)
	{
		RenX::Server *server = RenX::Core::servers.get(i);
		server->addCommand(command->copy());
	}
	return RenX::Core::servers.size();
}

// This shouldn't be needed later.
/*#define PARSE_PLAYER_DATA(STRING, OFFSET) \
	char *name = nullptr; int id; char team; \
	const char *p = strstr(buff, STRING); \
	if (p - buff - OFFSET < 12) { } \
	team = *(p - 3); \
	if (*(p - 9) == '<') id = atoi(p - 8); \
	else id = atoi(p - 9); \
	name = charToChar(buff + OFFSET + 1, 0, (int)(p - 10 - (buff + OFFSET + 1)));*/

#define PARSE_PLAYER_DATA() \
	Jupiter::ReferenceString name; \
	TeamType team; \
	int id; \
	bool isBot = false; { \
		Jupiter::ReferenceString idToken; \
		if (playerData[0] == ',') { \
			name = playerData.gotoWord(1, ","); \
			idToken = playerData.getWord(0, ","); \
			team = Other; \
		} else { \
			name = playerData.gotoWord(2, ","); \
			idToken = playerData.getWord(1, ","); \
			team = RenX::getTeam(playerData[0]); \
		} \
		if (idToken[0] == 'b') { idToken.shiftRight(1); isBot = true; } \
		id = idToken.asInt(10); } \
	RenX::PlayerInfo *player = getPlayerOrAdd(server, name, id, team, isBot);

inline RenX::PlayerInfo *getPlayerOrAdd(RenX::Server *server, const Jupiter::ReadableString &name, int id, RenX::TeamType team, bool isBot)
{
	RenX::PlayerInfo *r = server->getPlayer(id);
	if (r == nullptr)
	{
		r = new RenX::PlayerInfo();
		r->id = id;
		r->name = name;
		r->isBot = isBot;
		r->joinTime = time(nullptr);
		if (id != 0) server->players.add(r);
	}
	else if (r->name.size() == 0) r->name = name;
	r->team = team;
	return r;
}

inline void onPreGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
{
	RenX::PlayerInfo *player;

	if (server->players.size() != 0)
	{
		for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
		{
			player = n->data;
			if (player != nullptr)
			{
				if (player->team == team)
					player->wins++;
				else player->loses++;
			}
		}
	}
}

inline void onPostGameOver(RenX::Server *server, RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
{
	RenX::PlayerInfo *player;

	if (server->players.size() != 0)
	{
		for (Jupiter::DLList<RenX::PlayerInfo>::Node *n = server->players.getNode(0); n != nullptr; n = n->next)
		{
			player = n->data;
			if (player != nullptr)
			{
				player->kills = 0;
				player->deaths = 0;
				player->suicides = 0;
				player->headshots = 0;
				player->vehicleKills = 0;
				player->buildingKills = 0;
				player->defenceKills = 0;
			}
		}
	}
}

inline void onChat(RenX::Server *server, RenX::PlayerInfo *player, const Jupiter::ReadableString &message, bool isPublic)
{
	const Jupiter::ReadableString &prefix = server->getCommandPrefix();
	if (message.find(prefix) == 0 && message.size() != prefix.size())
	{
		Jupiter::ReferenceString command;
		Jupiter::ReferenceString parameters;
		if (containsSymbol(WHITESPACE, message.get(prefix.size())))
		{
			command = Jupiter::ReferenceString::getWord(message, 1, WHITESPACE);
			parameters = Jupiter::ReferenceString::gotoWord(message, 2, WHITESPACE);
		}
		else
		{
			command = Jupiter::ReferenceString::getWord(message, 0, WHITESPACE);
			command.shiftRight(prefix.size());
			parameters = Jupiter::ReferenceString::gotoWord(message, 1, WHITESPACE);
		}
		server->triggerCommand(command, player, parameters);
	}
}

int RenX::Core::think()
{
	// Change this later to just call server->think().
	RenX::Server *server;
	for (size_t a = 0; a < RenX::Core::servers.size(); a++)
	{
		server = RenX::Core::servers.get(a);
		if (server->isConnected() == false)
		{
			if (time(0) >= server->getLastAttempt() + server->getDelay())
			{
				if (server->connect()) server->sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to Renegade-X server.");
				else server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to Renegade-X server.");
			}
		}
		else
		{
			if (server->sock.recv() > 0)
			{
				Jupiter::ReferenceString buffer = server->sock.getBuffer();
				unsigned int totalLines = buffer.wordCount(ENDL);
				for (unsigned int currentLine = 0; currentLine != totalLines; currentLine++)
				{
					Jupiter::ReferenceString buff = buffer.getWord(currentLine, ENDL);
					Jupiter::ReferenceString header = buff.getWord(0, RenX::DelimS);
					Jupiter::ReferenceString playerData = buff.getWord(1, RenX::DelimS);
					Jupiter::ReferenceString action = buff.getWord(2, RenX::DelimS);

					//printf("[RenX Dump] %.*s - %.*s - %.*s" ENDL, header.size(), header.ptr(), playerData.size(), playerData.ptr(), action.size(), action.ptr());

					if (buff.size() != 0)
					{
						switch (header[0])
						{
						case 'l':
							if (header.equals("lGAME:"))
							{
								if (action.equals("deployed"))
								{
									PARSE_PLAYER_DATA();
									Jupiter::ReferenceString objectType = buff.getWord(3, RenX::DelimS);
									if (objectType.match("*Beacon")) player->beaconPlacements++;
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnDeploy(server, player, objectType);
								}
								else if (action.equals("suicided by"))
								{
									PARSE_PLAYER_DATA();
									player->deaths++;
									player->suicides++;
									Jupiter::ReferenceString damageType = buff.getWord(3, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnSuicide(server, player, damageType);
								}
								else if (action.equals("killed"))
								{
									PARSE_PLAYER_DATA();
									Jupiter::ReferenceString victimData = buff.getWord(3, RenX::DelimS);
									Jupiter::ReferenceString vname = victimData.getWord(2, ",");
									Jupiter::ReferenceString vidToken = victimData.getWord(1, ",");
									int vid;
									bool visBot = false;
									if (vidToken[0] == 'b')
									{
										vidToken.shiftRight(1);
										visBot = true;
									}
									vid = vidToken.asInt(10);
									TeamType vteam = RenX::getTeam(victimData.getWord(0, ",")[0]);
									Jupiter::ReferenceString damageType = buff.getWord(5, RenX::DelimS);
									RenX::PlayerInfo *victim = getPlayerOrAdd(server, vname, vid, vteam, visBot);
									player->kills++;
									if (damageType.equals("Rx_DmgType_Headshot")) player->headshots++;
									victim->deaths++;

									if (server->needsCList)
									{
										server->sendData(STRING_LITERAL_AS_REFERENCE("clogclientlist\n"));
										server->needsCList = false;
									}

									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnKill(server, player, victim, damageType);
								}
								else if (action.match("died by"))
								{
									PARSE_PLAYER_DATA();
									player->deaths++;
									Jupiter::ReferenceString damageType = buff.getWord(3, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnDie(server, player, damageType);
								}
								else if (action.match("destroyed*"))
								{
									PARSE_PLAYER_DATA();
									Jupiter::ReferenceString victim = buff.getWord(3, RenX::DelimS);
									Jupiter::ReferenceString damageType = buff.getWord(5, RenX::DelimS);
									ObjectType type;
									if (action.equals("destroyed building"))
									{
										type = Building;
										player->buildingKills++;
									}
									else if (victim.match("Rx_Defence_*"))
									{
										type = Defence;
										player->defenceKills++;
									}
									else
									{
										type = Vehicle;
										player->vehicleKills++;
									}
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnDestroy(server, player, victim, damageType, type);
								}
								else if (playerData.match("??? wins (*)"))
								{
									TeamType team = RenX::getTeam(playerData[0]);
									int gScore = buff.getWord(2, RenX::DelimS).gotoWord(1, "=").asInt(10);
									int nScore = buff.getWord(3, RenX::DelimS).gotoWord(1, "=").asInt(10);
									Jupiter::ReferenceString winType = Jupiter::ReferenceString::substring(playerData, 10);
									winType.truncate(1);
									WinType iWinType = Unknown;
									if (gScore == nScore)
										iWinType = Tie;
									else if (winType.equals("TimeLimit"))
										iWinType = Score;
									else if (winType.equals("Buildings"))
										iWinType = Base;

									server->needsCList = true;
									onPreGameOver(server, iWinType, team, gScore, nScore);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnGameOver(server, iWinType, team, gScore, nScore);
									onPostGameOver(server, iWinType, team, gScore, nScore);
								}
								else if (playerData.equals("Tie"))
								{
									int gScore = action.gotoWord(1, "=").asInt(10);
									int nScore = buff.getWord(3, RenX::DelimS).gotoWord(1, "=").asInt(10);

									server->needsCList = true;
									if (gScore == nScore)
									{
										onPreGameOver(server, Tie, Other, gScore, nScore);
										for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
											RenX::Core::plugins.get(i)->RenX_OnGameOver(server, Tie, Other, gScore, nScore);
										onPostGameOver(server, Tie, Other, gScore, nScore);
									}
									else
									{
										TeamType winTeam = gScore > nScore ? RenX::getTeam('G') : RenX::getTeam('N');
										onPreGameOver(server, Shutdown, winTeam, gScore, nScore);
										for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
											RenX::Core::plugins.get(i)->RenX_OnGameOver(server, Shutdown, winTeam, gScore, nScore);
										onPostGameOver(server, Shutdown, winTeam, gScore, nScore);
									}
								}
								else for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
									RenX::Core::plugins.get(i)->RenX_OnGame(server, buff.gotoWord(1, RenX::DelimS));
							}
							else if (header.equals("lCHAT:"))
							{
								if (action.equals("teamsay:"))
								{
									PARSE_PLAYER_DATA();
									Jupiter::ReferenceString message = buff.getWord(3, RenX::DelimS);
									onChat(server, player, message, false);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnTeamChat(server, player, message);
								}
								else if (action.equals("say:"))
								{
									PARSE_PLAYER_DATA();
									Jupiter::ReferenceString message = buff.getWord(3, RenX::DelimS);
									onChat(server, player, message, true);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnChat(server, player, message);
								}
							}
							else if (header.equals("lPLAYER:"))
							{
								PARSE_PLAYER_DATA();
								if (action.equals("disconnected"))
								{
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnPart(server, player);
									server->removePlayer(player);
									player = nullptr;
								}
								else if (action.equals("entered from"))
								{
									player->ip = buff.getWord(3, RenX::DelimS);
									if (buff.getWord(4, RenX::DelimS).equals("steamid")) player->steamid = buff.getWord(5, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnJoin(server, player);
								}
								else if (action.equals("changed name to:"))
								{
									Jupiter::ReferenceString newName = buff.getWord(3, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnNameChange(server, player, newName);
									player->name = newName;
								}
							}
							else if (header.equals("lRCON:"))
							{
								if (action.equals("executed:"))
								{
									Jupiter::ReferenceString command = buff.getWord(3, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnExecute(server, playerData, command);
								}
								else if (action.equals("subscribed")) for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
									RenX::Core::plugins.get(i)->RenX_OnSubscribe(server, playerData);
								else for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
									RenX::Core::plugins.get(i)->RenX_OnRCON(server, buff.gotoWord(1, RenX::DelimS));
							}
							else if (header.equals("lADMIN:"))
							{
								PARSE_PLAYER_DATA();
								if (action.equals("logged in as"))
								{
									player->adminType = buff.getWord(3, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnAdminLogin(server, player);
								}
								else if (action.equals("logged out of"))
								{
									Jupiter::ReferenceString type = buff.getWord(3, RenX::DelimS);
									for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
										RenX::Core::plugins.get(i)->RenX_OnAdminLogout(server, player);
									player->adminType = "";
								}
								else for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
									RenX::Core::plugins.get(i)->RenX_OnAdmin(server, buff.gotoWord(1, RenX::DelimS));
							}
							else if (header.equals("lC-LIST:"))
							{
								// ID IP SteamID Team Name
								if (playerData.isEmpty())
									break;

								int id;
								bool isBot = false;
								if (playerData.get(0) == 'b')
								{
									isBot = true;
									playerData.shiftRight(1);
									id = playerData.asInt(10);
									playerData.shiftLeft(1);
								}
								else id = playerData.asInt(10);
								Jupiter::ReferenceString ip = playerData.getWord(1, WHITESPACE);
								Jupiter::ReferenceString steamid = playerData.getWord(2, WHITESPACE);
								RenX::TeamType team;
								Jupiter::ReferenceString name;
								if (steamid.equals("-----NO")) // RCONv2-2a
								{
									steamid = "";
									Jupiter::ReferenceString &teamToken = playerData.getWord(4, WHITESPACE);
									if (teamToken.isEmpty())
										break;
									team = getTeam(teamToken.get(0));
									name = playerData.gotoWord(5, WHITESPACE);
								}
								else
								{
									if (steamid.equals("-----NO-STEAM-----")) // RCONv2-2.5a
										steamid = "";
									Jupiter::ReferenceString &teamToken = playerData.getWord(3, WHITESPACE);
									if (teamToken.isEmpty())
										break;
									team = getTeam(teamToken.get(0));
									name = playerData.gotoWord(4, WHITESPACE);
								}

								RenX::PlayerInfo *player = getPlayerOrAdd(server, name, id, team, isBot);
								if (player->ip.size() == 0)
								{
									player->ip = ip;
									player->steamid = steamid;
								}
							}
							else
							{
								buff.shiftRight(1);
								for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
									RenX::Core::plugins.get(i)->RenX_OnLog(server, buff);
								buff.shiftLeft(1);
							}
							break;

						case 'c':
							buff.shiftRight(1);
							for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
								RenX::Core::plugins.get(i)->RenX_OnCommand(server, buff);
							buff.shiftLeft(1);
							break;

						case 'e':
							buff.shiftRight(1);
							for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
								RenX::Core::plugins.get(i)->RenX_OnError(server, buff);
							buff.shiftLeft(1);
							break;

						case 'v':
							buff.shiftRight(1);
							server->rconVersion = buff.asInt(10);
							server->gameVersion = buff.substring(3);
							for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
								RenX::Core::plugins.get(i)->RenX_OnVersion(server, buff);
							buff.shiftLeft(1);
							break;

						case 'a':
							buff.shiftRight(1);
							for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
								RenX::Core::plugins.get(i)->RenX_OnAuthorized(server, buff);
							buff.shiftLeft(1);
							break;

						default:
							buff.shiftRight(1);
							for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
								RenX::Core::plugins.get(i)->RenX_OnOther(server, header[0], buff);
							buff.shiftLeft(1);
							break;
						}
						for (size_t i = 0; i < RenX::Core::plugins.size(); i++)
							RenX::Core::plugins.get(i)->RenX_OnRaw(server, buff);
					}
				}
			}
			else if (Jupiter::Socket::getLastError() != 10035) // This is a serious error
			{
				if (server->reconnect()) server->sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection lost to Renegade-X server lost. Reconnection attempt in progress.");
				else
				{
					server->wipeData();
					server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection lost to Renegade-X server lost. Reconnection attempt failed.");
				}
			}
		}
	}
	return Jupiter::Plugin::think();
}

// Entry point

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}

// Unload

extern "C" __declspec(dllexport) void unload(void)
{
	while (pluginInstance.getPlugins()->size() > 0) freePlugin(pluginInstance.getPlugins()->remove(0));
}
