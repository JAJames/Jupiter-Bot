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

#include <ctime>
#include "Jupiter/String.h"
#include "ServerManager.h"
#include "IRC_Bot.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"
#include "RenX_BuildingInfo.h"
#include "RenX_GameCommand.h"
#include "RenX_Functions.h"
#include "RenX_Plugin.h"
#include "RenX_BanDatabase.h"
#include "RenX_ExemptionDatabase.h"
#include "RenX_Tags.h"

using namespace Jupiter::literals;

int RenX::Server::think()
{
	if (RenX::Server::connected == false)
	{
		// Not connected; attempt retry if needed
		if (RenX::Server::maxAttempts < 0 || RenX::Server::attempts < RenX::Server::maxAttempts)
		{
			if (std::chrono::steady_clock::now() >= RenX::Server::lastAttempt + RenX::Server::delay)
			{
				if (RenX::Server::connect())
					RenX::Server::sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to Renegade-X server.");
				else RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to Renegade-X server.");
			}
		}
		else
			return 1;
	}
	else if (RenX::Server::awaitingPong && std::chrono::steady_clock::now() - RenX::Server::lastActivity >= RenX::Server::pingTimeoutThreshold) // ping timeout
	{
		// Ping timeout; disconnect immediately
		RenX::Server::sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from Renegade-X server (ping timeout)."));
		RenX::Server::disconnect(RenX::DisconnectReason::PingTimeout);
	}
	else
	{
		auto cycle_player_rdns = [this]() // Cycles through any pending RDNS resolutions, and fires events as necessary.
		{
			if (this->player_rdns_resolutions_pending != 0)
			{
				Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
				for (auto node = this->players.begin(); node != this->players.end(); ++node)
				{
					if (node->rdns_thread.joinable() && node->rdns_mutex.try_lock()) // RDNS event hasn't fired AND RDNS value has been resolved
					{
						node->rdns_mutex.unlock();
						node->rdns_thread.join();
						--this->player_rdns_resolutions_pending;

						// Check for bans
						this->banCheck(*node);

						// Fire RDNS resolved event
						for (size_t index = 0; index < xPlugins.size(); ++index)
							xPlugins.get(index)->RenX_OnPlayerRDNS(*this, *node);

						// Fire player indentified event if ready
						if (node->hwid.isNotEmpty())
							for (size_t index = 0; index < xPlugins.size(); ++index)
								xPlugins.get(index)->RenX_OnPlayerIdentify(*this, *node);

						if (this->player_rdns_resolutions_pending == 0) // No more resolutions pending
							return;
					}
				}
			}
		};

		// Connected and fine
		if (RenX::Server::sock.recv() > 0) // Data received
		{
			cycle_player_rdns();

			Jupiter::ReadableString::TokenizeResult<Jupiter::Reference_String> result = Jupiter::ReferenceString::tokenize(RenX::Server::sock.getBuffer(), '\n');
			if (result.token_count != 0)
			{
				RenX::Server::lastActivity = std::chrono::steady_clock::now();
				RenX::Server::lastLine.concat(result.tokens[0]);
				if (result.token_count != 1)
				{
					RenX::Server::processLine(RenX::Server::lastLine);
					RenX::Server::lastLine = result.tokens[result.token_count - 1];

					for (size_t index = 1; index != result.token_count - 1; ++index)
						RenX::Server::processLine(result.tokens[index]);
				}
			}
		}
		else if (Jupiter::Socket::getLastError() == 10035) // Operation would block (no new data)
		{
			cycle_player_rdns();

			if (RenX::Server::awaitingPong == false && std::chrono::steady_clock::now() - RenX::Server::lastActivity >= RenX::Server::pingRate)
			{
				RenX::Server::lastActivity = std::chrono::steady_clock::now();
				RenX::Server::sock.send("cping\n"_jrs);
				RenX::Server::awaitingPong = true;
			}
		}
		else // This is a serious error
		{
			RenX::Server::wipeData();
			if (RenX::Server::maxAttempts != 0)
			{
				RenX::Server::sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt in progress.");
				if (RenX::Server::reconnect(RenX::DisconnectReason::SocketError))
					RenX::Server::sendLogChan(IRCCOLOR "06[Progress]" IRCCOLOR " Connection to Renegade-X server reestablished. Initializing Renegade-X RCON protocol...");
				else
					RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt failed.");
			}
			else
			{
				RenX::Server::sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. No attempt will be made to reconnect.");
				return 1;
			}
			return 0;
		}

		if (RenX::Server::rconVersion >= 3 && RenX::Server::players.size() != 0)
		{
			if (RenX::Server::clientUpdateRate != std::chrono::milliseconds::zero() && std::chrono::steady_clock::now() > RenX::Server::lastClientListUpdate + RenX::Server::clientUpdateRate)
				RenX::Server::updateClientList();

			if (RenX::Server::buildingUpdateRate != std::chrono::milliseconds::zero() && std::chrono::steady_clock::now() > RenX::Server::lastBuildingListUpdate + RenX::Server::buildingUpdateRate)
				RenX::Server::updateBuildingList();
		}

		if (RenX::Server::gameover_pending && RenX::Server::gameover_time < std::chrono::steady_clock::now())
		{
			this->gameover();
			RenX::Server::gameover_pending = false;
		}
	}
	return 0;
}

int RenX::Server::OnRehash()
{
	Jupiter::StringS oldHostname = RenX::Server::hostname;
	Jupiter::StringS oldClientHostname = RenX::Server::clientHostname;
	Jupiter::StringS oldPass = RenX::Server::pass;
	unsigned short oldPort = RenX::Server::port;
	int oldSteamFormat = RenX::Server::steamFormat;
	RenX::Server::commands.emptyAndDelete();
	RenX::Server::init(*RenX::getCore()->getConfig().getSection(RenX::Server::configSection));
	if (RenX::Server::port == 0 || RenX::Server::hostname.isNotEmpty())
	{
		RenX::Server::hostname = oldHostname;
		RenX::Server::clientHostname = oldClientHostname;
		RenX::Server::pass = oldPass;
		RenX::Server::port = oldPort;
	}
	else if (oldHostname.equalsi(RenX::Server::hostname) == false || oldPort != RenX::Server::port || oldClientHostname.equalsi(RenX::Server::clientHostname) == false || oldPass.equalsi(RenX::Server::pass) == false)
		RenX::Server::reconnect(RenX::DisconnectReason::Rehash);
	return 0;
}

bool RenX::Server::OnBadRehash(bool removed)
{
	return removed;
}

bool RenX::Server::isConnected() const
{
	return RenX::Server::connected;
}

bool RenX::Server::isSubscribed() const
{
	return RenX::Server::subscribed;
}

bool RenX::Server::isFullyConnected() const
{
	return RenX::Server::fully_connected;
}

bool RenX::Server::isFirstKill() const
{
	return RenX::Server::firstKill;
}

bool RenX::Server::isFirstDeath() const
{
	return RenX::Server::firstDeath;
}

bool RenX::Server::isFirstAction() const
{
	return RenX::Server::firstAction;
}

bool RenX::Server::isSeamless() const
{
	return RenX::Server::seamless;
}

bool RenX::Server::isReliable() const
{
	return RenX::Server::reliable;
}

bool RenX::Server::isMatchPending() const
{
	return RenX::Server::match_state == 0 || RenX::Server::isTravelling();
}

bool RenX::Server::isMatchInProgress() const
{
	return RenX::Server::match_state == 1;
}

bool RenX::Server::isMatchOver() const
{
	return RenX::Server::match_state == 2 || RenX::Server::isMatchOver();
}

bool RenX::Server::isTravelling() const
{
	return RenX::Server::match_state == 3;
}

bool RenX::Server::isCompetitive() const
{
	return RenX::Server::competitive;
}

bool RenX::Server::isPublicLogChanType(int type) const
{
	return RenX::Server::logChanType == type;
}

bool RenX::Server::isAdminLogChanType(int type) const
{
	return RenX::Server::adminLogChanType == type;
}

bool RenX::Server::isLogChanType(int type) const
{
	return RenX::Server::isPublicLogChanType(type) || RenX::Server::isAdminLogChanType(type);
}

bool RenX::Server::isPure() const
{
	return RenX::Server::pure;
}

int RenX::Server::send(const Jupiter::ReadableString &command)
{
	return RenX::Server::sock.send("c"_jrs + RenX::escapifyRCON(command) + '\n');
}

int RenX::Server::sendMessage(const Jupiter::ReadableString &message)
{
	Jupiter::String msg = RenX::escapifyRCON(message);
	if (RenX::Server::neverSay)
	{
		int r = 0;
		if (RenX::Server::players.size() != 0)
			for (auto node = this->players.begin(); node != this->players.end(); ++node)
				if (node->isBot == false)
					r += RenX::Server::sock.send(Jupiter::StringS::Format("chostprivatesay pid%d %.*s\n", node->id, msg.size(), msg.ptr()));
		return r;
	}
	else
		return RenX::Server::sock.send("chostsay "_jrs + msg + '\n');
}

int RenX::Server::sendMessage(const RenX::PlayerInfo &player, const Jupiter::ReadableString &message)
{
	return RenX::Server::sock.send("chostprivatesay pid"_jrs + Jupiter::StringS::Format("%d ", player.id) + RenX::escapifyRCON(message) + '\n');
}

int RenX::Server::sendData(const Jupiter::ReadableString &data)
{
	return RenX::Server::sock.send(data);
}

RenX::BuildingInfo *RenX::Server::getBuildingByName(const Jupiter::ReadableString &name) const
{
	for (size_t index = 0; index != RenX::Server::buildings.size(); ++index)
		if (RenX::Server::buildings.get(index)->name.equalsi(name))
			return RenX::Server::buildings.get(index);
	return nullptr;
}

bool RenX::Server::hasMapInRotation(const Jupiter::ReadableString &name) const
{
	size_t index = RenX::Server::maps.size();
	while (index != 0)
		if (RenX::Server::maps.get(--index)->name.equalsi(name))
			return true;
	return false;
}

const Jupiter::ReadableString *RenX::Server::getMapName(const Jupiter::ReadableString &name) const
{
	size_t index = RenX::Server::maps.size();
	const Jupiter::ReadableString *map_name;
	while (index != 0)
	{
		map_name = &RenX::Server::maps.get(--index)->name;
		if (map_name->findi(name) != Jupiter::INVALID_INDEX)
			return map_name;
	}
	return nullptr;
}

const Jupiter::ReadableString &RenX::Server::getCurrentRCONCommand() const
{
	return RenX::Server::lastCommand;
}

const Jupiter::ReadableString &RenX::Server::getCurrentRCONCommandParameters() const
{
	return RenX::Server::lastCommandParams;
}

std::chrono::milliseconds RenX::Server::getGameTime() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - RenX::Server::gameStart);
}

std::chrono::milliseconds RenX::Server::getGameTime(const RenX::PlayerInfo &player) const
{
	if (player.joinTime < RenX::Server::gameStart)
		return RenX::Server::getGameTime();

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player.joinTime);
}

size_t RenX::Server::getBotCount() const
{
	size_t count = 0;

	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		if (node->isBot)
			++count;

	return count;
	//return RenX::Server::bot_count;
}

RenX::PlayerInfo *RenX::Server::getPlayer(int id) const
{
	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		if (node->id == id)
			return const_cast<RenX::PlayerInfo *>(&*node);

	return nullptr;
}

RenX::PlayerInfo *RenX::Server::getPlayerByName(const Jupiter::ReadableString &name) const
{
	if (RenX::Server::players.size() == 0)
		return nullptr;

	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		if (node->name == name)
			return const_cast<RenX::PlayerInfo *>(&*node);

	Jupiter::ReferenceString idToken = name;
	if (name.matchi("Player?*"))
		idToken.shiftRight(6);
	else if (name.matchi("pid?*"))
		idToken.shiftRight(3);
	else
		return nullptr;
	int id = idToken.asInt(10);

	return this->getPlayer(id);
}

RenX::PlayerInfo *RenX::Server::getPlayerByPartName(const Jupiter::ReadableString &partName) const
{
	if (RenX::Server::players.size() == 0)
		return nullptr;

	RenX::PlayerInfo *r = RenX::Server::getPlayerByName(partName);
	if (r != nullptr)
		return r;

	return RenX::Server::getPlayerByPartNameFast(partName);
}

RenX::PlayerInfo *RenX::Server::getPlayerByPartNameFast(const Jupiter::ReadableString &partName) const
{
	if (RenX::Server::players.size() == 0)
		return nullptr;

	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		if (node->name.findi(partName) != Jupiter::INVALID_INDEX)
			return const_cast<RenX::PlayerInfo *>(&*node);

	return nullptr;
}

Jupiter::StringS RenX::Server::formatSteamID(const RenX::PlayerInfo &player) const
{
	return RenX::Server::formatSteamID(player.steamid);
}

Jupiter::StringS RenX::Server::formatSteamID(uint64_t id) const
{
	if (id == 0)
		return Jupiter::ReferenceString::empty;

	switch (RenX::Server::steamFormat)
	{
	default:
	case 16:
		return Jupiter::StringS::Format("0x%.16llX", id);
	case 10:
		return Jupiter::StringS::Format("%llu", id);
	case 8:
		return Jupiter::StringS::Format("0%llo", id);
	case -2:
		id -= 0x0110000100000000ULL;
		if (id % 2 == 1)
			return Jupiter::StringS::Format("STEAM_1:1:%llu", id / 2ULL);
		else
			return Jupiter::StringS::Format("STEAM_1:0:%llu", id / 2ULL);
	case -3:
		id -= 0x0110000100000000ULL;
		return Jupiter::StringS::Format("[U:1:%llu]", id);
	}
}

void RenX::Server::kickPlayer(int id, const Jupiter::ReadableString &in_reason)
{
	Jupiter::String reason = RenX::escapifyRCON(in_reason);

	if (reason.isEmpty())
		RenX::Server::sock.send(Jupiter::StringS::Format("ckick pid%d\n", id));
	else
		RenX::Server::sock.send(Jupiter::StringS::Format("ckick pid%d %.*s\n", id, reason.size(), reason.ptr()));
}

void RenX::Server::kickPlayer(const RenX::PlayerInfo &player, const Jupiter::ReadableString &reason)
{
	if ((player.exemption_flags & RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK) == 0)
		RenX::Server::kickPlayer(player.id, reason);
}

void RenX::Server::forceKickPlayer(int id, const Jupiter::ReadableString &in_reason)
{
	Jupiter::String reason = RenX::escapifyRCON(in_reason);

	if (reason.isEmpty())
		RenX::Server::sock.send(Jupiter::StringS::Format("cfkick pid%d You were kicked from the server.\n", id));
	else
		RenX::Server::sock.send(Jupiter::StringS::Format("cfkick pid%d %.*s\n", id, reason.size(), reason.ptr()));
}

void RenX::Server::forceKickPlayer(const RenX::PlayerInfo &player, const Jupiter::ReadableString &reason)
{
	if ((player.exemption_flags & RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK) == 0)
		RenX::Server::forceKickPlayer(player.id, reason);
}

void RenX::Server::banCheck()
{
	if (RenX::Server::players.size() != 0)
		for (auto node = this->players.begin(); node != this->players.end(); ++node)
			if (node->isBot == false)
				this->banCheck(*node);
}

void RenX::Server::banCheck(RenX::PlayerInfo &player)
{
	if ((player.exemption_flags & (RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN | RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK)) != 0)
		return;

	const Jupiter::ArrayList<RenX::BanDatabase::Entry> &entries = RenX::banDatabase->getEntries();
	RenX::BanDatabase::Entry *entry = nullptr;
	uint32_t netmask;

	RenX::BanDatabase::Entry *last_to_expire[7];
	for (size_t index = 0; index != sizeof(last_to_expire) / sizeof(RenX::BanDatabase::Entry *); ++index)
		last_to_expire[index] = nullptr;

	auto handle_type = [&entry, &last_to_expire](size_t index)
	{
		if (last_to_expire[index] == nullptr)
			last_to_expire[index] = entry;
		else if (last_to_expire[index]->length == std::chrono::seconds::zero())
		{
			// favor older bans if they're also permanent
			if (entry->length == std::chrono::seconds::zero() && entry->timestamp < last_to_expire[index]->timestamp)
				last_to_expire[index] = entry;
		}
		else if (entry->length == std::chrono::seconds::zero() || entry->timestamp + entry->length > last_to_expire[index]->timestamp + last_to_expire[index]->length)
			last_to_expire[index] = entry;
	};

	for (size_t i = 0; i != entries.size(); i++)
	{
		entry = entries.get(i);
		if (entry->is_active())
		{
			if (entry->length != std::chrono::seconds::zero() && entry->timestamp + entry->length < std::chrono::system_clock::now())
				banDatabase->deactivate(i);
			else
			{
				if (entry->prefix_length >= 32)
					netmask = 0xFFFFFFFF;
				else
					netmask = Jupiter_prefix_length_to_netmask(entry->prefix_length);

				if ((this->localSteamBan && entry->steamid != 0 && entry->steamid == player.steamid)
					|| (this->localIPBan && entry->ip != 0 && (entry->ip & netmask) == (player.ip32 & netmask))
					|| (this->localHWIDBan && entry->hwid.isNotEmpty() && entry->hwid.equals(player.hwid))
					|| (this->localRDNSBan && entry->rdns.isNotEmpty() && entry->is_rdns_ban() && player.rdns_thread.joinable() == false && player.rdns.match(entry->rdns))
					|| (this->localNameBan && entry->name.isNotEmpty() && entry->name.equalsi(player.name)))
				{
					player.ban_flags |= entry->flags;
					if (entry->is_type_game())
						handle_type(0);
					if (entry->is_type_chat())
						handle_type(1);
					if (entry->is_type_bot())
						handle_type(2);
					if (entry->is_type_vote())
						handle_type(3);
					if (entry->is_type_mine())
						handle_type(4);
					if (entry->is_type_ladder())
						handle_type(5);
					if (entry->is_type_alert())
						handle_type(6);
				}
			}
		}
	}

	char timeStr[256];
	if (last_to_expire[0] != nullptr) // Game ban
	{
		strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(std::addressof<const time_t>(std::chrono::system_clock::to_time_t(last_to_expire[0]->timestamp + last_to_expire[0]->length))));
		if (last_to_expire[0]->length == std::chrono::seconds::zero())
			this->forceKickPlayer(player, Jupiter::StringS::Format("You were permanently banned from %.*s on %s for: %.*s", RenX::Server::ban_from_str.size(), RenX::Server::ban_from_str.ptr(), timeStr, last_to_expire[0]->reason.size(), last_to_expire[0]->reason.ptr()));
		else
			this->forceKickPlayer(player, Jupiter::StringS::Format("You are banned from %.*s until %s for: %.*s", RenX::Server::ban_from_str.size(), RenX::Server::ban_from_str.ptr(), timeStr, last_to_expire[0]->reason.size(), last_to_expire[0]->reason.ptr()));

		player.ban_flags |= RenX::BanDatabase::Entry::FLAG_TYPE_BOT; // implies FLAG_TYPE_BOT
	}
	else
	{
		if (last_to_expire[1] != nullptr) // Chat ban
		{
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(std::addressof<const time_t>(std::chrono::system_clock::to_time_t(last_to_expire[1]->timestamp + last_to_expire[1]->length))));
			this->mute(player);
			if (last_to_expire[1]->length == std::chrono::seconds::zero())
				this->sendMessage(player, Jupiter::StringS::Format("You were permanently muted on this server on %s for: %.*s", timeStr, last_to_expire[1]->reason.size(), last_to_expire[1]->reason.ptr()));
			else
				this->sendMessage(player, Jupiter::StringS::Format("You are muted on this server until %s for: %.*s", timeStr, last_to_expire[1]->reason.size(), last_to_expire[1]->reason.ptr()));

			player.ban_flags |= RenX::BanDatabase::Entry::FLAG_TYPE_BOT; // implies FLAG_TYPE_BOT
		}
		else if (last_to_expire[2] != nullptr) // Bot ban
		{
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(std::addressof<const time_t>(std::chrono::system_clock::to_time_t(last_to_expire[2]->timestamp + last_to_expire[2]->length))));
			if (last_to_expire[2]->length == std::chrono::seconds::zero())
				this->sendMessage(player, Jupiter::StringS::Format("You were permanently bot-muted on this server on %s for: %.*s", timeStr, last_to_expire[2]->reason.size(), last_to_expire[2]->reason.ptr()));
			else
				this->sendMessage(player, Jupiter::StringS::Format("You are bot-muted on this server until %s for: %.*s", timeStr, last_to_expire[2]->reason.size(), last_to_expire[2]->reason.ptr()));
		}
		if (last_to_expire[3] != nullptr) // Vote ban
		{
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(std::addressof<const time_t>(std::chrono::system_clock::to_time_t(last_to_expire[3]->timestamp + last_to_expire[3]->length))));
			if (last_to_expire[3]->length == std::chrono::seconds::zero())
				this->sendMessage(player, Jupiter::StringS::Format("You were permanently vote-muted on this server on %s for: %.*s", timeStr, last_to_expire[3]->reason.size(), last_to_expire[3]->reason.ptr()));
			else
				this->sendMessage(player, Jupiter::StringS::Format("You are vote-muted on this server until %s for: %.*s", timeStr, last_to_expire[3]->reason.size(), last_to_expire[3]->reason.ptr()));
		}
		if (last_to_expire[4] != nullptr) // Mine ban
		{
			this->mineBan(player);
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(std::addressof<const time_t>(std::chrono::system_clock::to_time_t(last_to_expire[4]->timestamp + last_to_expire[4]->length))));
			if (last_to_expire[4]->length == std::chrono::seconds::zero())
				this->sendMessage(player, Jupiter::StringS::Format("You were permanently mine-banned on this server on %s for: %.*s", timeStr, last_to_expire[4]->reason.size(), last_to_expire[4]->reason.ptr()));
			else
				this->sendMessage(player, Jupiter::StringS::Format("You are mine-banned on this server until %s for: %.*s", timeStr, last_to_expire[4]->reason.size(), last_to_expire[4]->reason.ptr()));
		}
		if (last_to_expire[5] != nullptr) // Ladder ban
		{
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(std::addressof<const time_t>(std::chrono::system_clock::to_time_t(last_to_expire[5]->timestamp + last_to_expire[5]->length))));
			if (last_to_expire[5]->length == std::chrono::seconds::zero())
				this->sendMessage(player, Jupiter::StringS::Format("You were permanently ladder-banned on this server on %s for: %.*s", timeStr, last_to_expire[5]->reason.size(), last_to_expire[5]->reason.ptr()));
			else
				this->sendMessage(player, Jupiter::StringS::Format("You are ladder-banned on this server until %s for: %.*s", timeStr, last_to_expire[5]->reason.size(), last_to_expire[5]->reason.ptr()));
		}
		if (last_to_expire[6] != nullptr) // Alert
		{
			IRC_Bot *server;
			Jupiter::String &fmtName = RenX::getFormattedPlayerName(player);
			Jupiter::StringL user_message = Jupiter::StringL::Format(IRCCOLOR "04[Alert] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is marked for monitoring by %.*s for: \"%.*s\". Please keep an eye on them in ", fmtName.size(), fmtName.ptr(), last_to_expire[6]->banner.size(), last_to_expire[6]->banner.ptr(), last_to_expire[6]->reason.size(), last_to_expire[6]->reason.ptr());
			Jupiter::StringS channel_message = Jupiter::StringS::Format(IRCCOLOR "04[Alert] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is marked for monitoring by %.*s for: \"%.*s\"." IRCCOLOR, fmtName.size(), fmtName.ptr(), last_to_expire[6]->banner.size(), last_to_expire[6]->banner.ptr(), last_to_expire[6]->reason.size(), last_to_expire[6]->reason.ptr());

			auto alert_message_callback = [this, server, &user_message, &channel_message](Jupiter::IRC::Client::ChannelTableType::Bucket::Entry &in_entry)
			{
				auto alert_message_user_callback = [server, &in_entry, &user_message](Jupiter::IRC::Client::Channel::UserTableType::Bucket::Entry &in_user_entry)
				{
					if (in_entry.value.getUserPrefix(in_user_entry.value) != 0 && in_user_entry.value.getNickname().equals(server->getNickname()) == false)
						server->sendMessage(in_user_entry.value.getUser()->getNickname(), user_message);
				};

				if (this->isAdminLogChanType(in_entry.value.getType()))
				{
					server->sendMessage(in_entry.value.getName(), channel_message);

					user_message += in_entry.value.getName();
					in_entry.value.getUsers().callback(alert_message_user_callback);
					user_message -= in_entry.value.getName().size();
				}
			};

			for (size_t server_index = 0; server_index < serverManager->size(); ++server_index)
			{
				server = serverManager->getServer(server_index);
				server->getChannels().callback(alert_message_callback);
			}
		}
	}
};

void RenX::Server::banPlayer(int id, const Jupiter::ReadableString &banner, const Jupiter::ReadableString &reason)
{
	if (RenX::Server::rconBan)
	{
		Jupiter::String out_reason = RenX::escapifyRCON(reason);
		RenX::Server::sock.send(Jupiter::StringS::Format("ckickban pid%d %.*s\n", id, out_reason.size(), out_reason.ptr()));
	}
	else
	{
		RenX::PlayerInfo *player = RenX::Server::getPlayer(id);
		if (player != nullptr)
			RenX::Server::banPlayer(*player, banner, reason);
	}
}

void RenX::Server::banPlayer(const RenX::PlayerInfo &player, const Jupiter::ReadableString &banner, const Jupiter::ReadableString &reason, std::chrono::seconds length)
{
	if ((player.exemption_flags & RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN) == 0)
	{
		if (RenX::Server::localBan)
			RenX::banDatabase->add(this, player, banner, reason, length);

		if (length == std::chrono::seconds::zero())
		{
			if (RenX::Server::rconBan)
			{
				Jupiter::String out_reason = RenX::escapifyRCON(reason);
				RenX::Server::sock.send(Jupiter::StringS::Format("ckickban pid%d %.*s\n", player.id, out_reason.size(), out_reason.ptr()));
			}
			else if (banner.isNotEmpty())
				RenX::Server::forceKickPlayer(player, Jupiter::StringS::Format("You are permanently banned from %.*s by %.*s for: %.*s", RenX::Server::ban_from_str.size(), RenX::Server::ban_from_str.ptr(), banner.size(), banner.ptr(), reason.size(), reason.ptr()));
			else
				RenX::Server::forceKickPlayer(player, Jupiter::StringS::Format("You are permanently banned from %.*s for: %.*s", RenX::Server::ban_from_str.size(), RenX::Server::ban_from_str.ptr(), reason.size(), reason.ptr()));
		}
		else if (banner.isNotEmpty())
			RenX::Server::forceKickPlayer(player, Jupiter::StringS::Format("You are banned from %.*s by %.*s for the next %lld days, %.2d:%.2d:%.2d for: %.*s", RenX::Server::ban_from_str.size(), RenX::Server::ban_from_str.ptr(), banner.size(), banner.ptr(), static_cast<long long>(length.count() / 86400), static_cast<int>(length.count() % 3600), static_cast<int>((length.count() % 3600) / 60), static_cast<int>(length.count() % 60), reason.size(), reason.ptr()));
		else
			RenX::Server::forceKickPlayer(player, Jupiter::StringS::Format("You are banned from %.*s for the next %lld days, %.2d:%.2d:%.2d for: %.*s", RenX::Server::ban_from_str.size(), RenX::Server::ban_from_str.ptr(), static_cast<long long>(length.count() / 86400), static_cast<int>(length.count() % 3600), static_cast<int>((length.count() % 3600) / 60), static_cast<int>(length.count() % 60), reason.size(), reason.ptr()));
	}
}

bool RenX::Server::removePlayer(int id)
{
	if (RenX::Server::players.size() == 0)
		return false;

	for (auto node = this->players.begin(); node != this->players.end(); ++node)
	{
		if (node->id == id)
		{
			Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnPlayerDelete(*this, *node);

			if (node->isBot)
				--this->bot_count;

			if (node->rdns_thread.joinable()) // Close the RDNS thread, if one exists
			{
				--this->player_rdns_resolutions_pending;
				node->rdns_thread.join();
			}

			this->players.erase(node);
			return true;
		}
	}
	return false;
}

bool RenX::Server::removePlayer(RenX::PlayerInfo &player)
{
	return RenX::Server::removePlayer(player.id);
}

bool RenX::Server::fetchClientList()
{
	RenX::Server::lastClientListUpdate = std::chrono::steady_clock::now();
	if (this->rconVersion >= 4)
		return RenX::Server::sock.send("cclientvarlist KILLS DEATHS SCORE CREDITS CHARACTER VEHICLE PING ADMIN STEAM IP HWID PLAYERLOG\n"_jrs) > 0
			&& RenX::Server::sock.send("cbotvarlist KILLS DEATHS SCORE CREDITS CHARACTER VEHICLE PLAYERLOG\n"_jrs) > 0;
	else
		return RenX::Server::sock.send("cclientvarlist KILLS\xA0""DEATHS\xA0""SCORE\xA0""CREDITS\xA0""CHARACTER\xA0""VEHICLE\xA0""PING\xA0""ADMIN\xA0""STEAM\xA0""IP\xA0""PLAYERLOG\n"_jrs) > 0
			&& RenX::Server::sock.send("cbotvarlist KILLS\xA0""DEATHS\xA0""SCORE\xA0""CREDITS\xA0""CHARACTER\xA0""VEHICLE\xA0""PLAYERLOG\n"_jrs) > 0;
}

bool RenX::Server::updateClientList()
{
	RenX::Server::lastClientListUpdate = std::chrono::steady_clock::now();

	int r = 0;
	if (RenX::Server::players.size() != this->getBotCount())
	{
		if (this->rconVersion >= 4)
			r = RenX::Server::sock.send("cclientvarlist ID SCORE CREDITS PING\n"_jrs) > 0;
		else
			r = RenX::Server::sock.send("cclientvarlist ID\xA0""SCORE\xA0""CREDITS\xA0""PING\n"_jrs) > 0;
	}

	if (this->getBotCount() != 0)
	{
		if (this->rconVersion >= 4)
			r |= RenX::Server::sock.send("cbotvarlist ID SCORE CREDITS\n"_jrs) > 0;
		else
			r |= RenX::Server::sock.send("cbotvarlist ID\xA0""SCORE\xA0""CREDITS\n"_jrs) > 0;
	}

	return r != 0;
}

bool RenX::Server::updateBuildingList()
{
	RenX::Server::lastBuildingListUpdate = std::chrono::steady_clock::now();
	return RenX::Server::sock.send("cbinfo\n"_jrs) > 0;
}

bool RenX::Server::gameover()
{
	RenX::Server::gameover_when_empty = false;
	return RenX::Server::send("endmap"_jrs) > 0;
}

void RenX::Server::gameover(std::chrono::seconds delay)
{
	if (delay == std::chrono::seconds::zero())
		this->gameover();
	else
	{
		this->gameover_time = std::chrono::steady_clock::now() + delay;
		this->gameover_pending = true;
	}
}

bool RenX::Server::gameoverStop()
{
	if (this->gameover_pending == false)
		return false;

	this->gameover_pending = false;
	return true;
}

void RenX::Server::gameoverWhenEmpty()
{
	if (this->players.size() == this->getBotCount())
		this->gameover();
	else
		this->gameover_when_empty = true;
}

bool RenX::Server::setMap(const Jupiter::ReadableString &map)
{
	return RenX::Server::send(Jupiter::StringS::Format("changemap %.*s", map.size(), map.ptr())) > 0;
}

bool RenX::Server::loadMutator(const Jupiter::ReadableString &mutator)
{
	return RenX::Server::send(Jupiter::StringS::Format("loadmutator %.*s", mutator.size(), mutator.ptr())) > 0;
}

bool RenX::Server::unloadMutator(const Jupiter::ReadableString &mutator)
{
	return RenX::Server::send(Jupiter::StringS::Format("unloadmutator %.*s", mutator.size(), mutator.ptr())) > 0;
}

bool RenX::Server::cancelVote(const RenX::TeamType team)
{
	switch (team)
	{
	default:
		return RenX::Server::send("cancelvote -1"_jrs) > 0;
	case TeamType::GDI:
		return RenX::Server::send("cancelvote 0"_jrs) > 0;
	case TeamType::Nod:
		return RenX::Server::send("cancelvote 1"_jrs) > 0;
	}
}

bool RenX::Server::swapTeams()
{
	return RenX::Server::send("swapteams"_jrs) > 0;
}

bool RenX::Server::recordDemo()
{
	return RenX::Server::send("recorddemo"_jrs) > 0;
}

bool RenX::Server::mute(const RenX::PlayerInfo &player)
{
	return RenX::Server::send(Jupiter::StringS::Format("textmute pid%u", player.id)) > 0;
}

bool RenX::Server::unmute(const RenX::PlayerInfo &player)
{
	return RenX::Server::send(Jupiter::StringS::Format("textunmute pid%u", player.id)) > 0;
}

bool RenX::Server::giveCredits(int id, double credits)
{
	return RenX::Server::competitive == false && RenX::Server::send(Jupiter::StringS::Format("givecredits pid%d %f", id, credits)) > 0;
}

bool RenX::Server::giveCredits(RenX::PlayerInfo &player, double credits)
{
	return RenX::Server::giveCredits(player.id, credits);
}

bool RenX::Server::kill(int id)
{
	return RenX::Server::competitive == false && RenX::Server::send(Jupiter::StringS::Format("kill pid%d", id)) > 0;
}

bool RenX::Server::kill(RenX::PlayerInfo &player)
{
	return RenX::Server::kill(player.id);
}

bool RenX::Server::disarm(int id)
{
	return RenX::Server::competitive == false && RenX::Server::send(Jupiter::StringS::Format("disarm pid%d", id)) > 0;
}

bool RenX::Server::disarm(RenX::PlayerInfo &player)
{
	return RenX::Server::disarm(player.id);
}

bool RenX::Server::disarmC4(int id)
{
	return RenX::Server::competitive == false && RenX::Server::send(Jupiter::StringS::Format("disarmc4 pid%d", id)) > 0;
}

bool RenX::Server::disarmC4(RenX::PlayerInfo &player)
{
	return RenX::Server::disarmC4(player.id);
}

bool RenX::Server::disarmBeacon(int id)
{
	return RenX::Server::competitive == false && RenX::Server::send(Jupiter::StringS::Format("disarmb pid%d", id)) > 0;
}

bool RenX::Server::disarmBeacon(RenX::PlayerInfo &player)
{
	return RenX::Server::disarmBeacon(player.id);
}

bool RenX::Server::mineBan(int id)
{
	return RenX::Server::send(Jupiter::StringS::Format("mineban pid%d", id)) > 0;
}

bool RenX::Server::mineBan(RenX::PlayerInfo &player)
{
	return RenX::Server::mineBan(player.id);
}

bool RenX::Server::changeTeam(int id, bool resetCredits)
{
	return RenX::Server::send(Jupiter::StringS::Format(resetCredits ? "team pid%d" : "team2 pid%d", id)) > 0;
}

bool RenX::Server::changeTeam(RenX::PlayerInfo &player, bool resetCredits)
{
	return RenX::Server::changeTeam(player.id, resetCredits);
}

const Jupiter::ReadableString &RenX::Server::getPrefix() const
{
	static Jupiter::String parsed;
	RenX::processTags(parsed = RenX::Server::IRCPrefix, this);
	return parsed;
}

void RenX::Server::setPrefix(const Jupiter::ReadableString &prefix)
{
	RenX::sanitizeTags(RenX::Server::IRCPrefix = prefix);
}

const Jupiter::ReadableString &RenX::Server::getCommandPrefix() const
{
	return RenX::Server::CommandPrefix;
}

void RenX::Server::setCommandPrefix(const Jupiter::ReadableString &prefix)
{
	RenX::Server::CommandPrefix = prefix;
}

void RenX::Server::setRanked(bool in_value)
{
	RenX::Server::m_ranked = in_value;
}

const Jupiter::ReadableString &RenX::Server::getRules() const
{
	return RenX::Server::rules;
}

const Jupiter::ReadableString &RenX::Server::getHostname() const
{
	return RenX::Server::hostname;
}

unsigned short RenX::Server::getPort() const
{
	return RenX::Server::port;
}

const Jupiter::ReadableString &RenX::Server::getSocketHostname() const
{
	return RenX::Server::sock.getRemoteHostname();
}

unsigned short RenX::Server::getSocketPort() const
{
	return RenX::Server::sock.getRemotePort();
}

std::chrono::steady_clock::time_point RenX::Server::getLastAttempt() const
{
	return RenX::Server::lastAttempt;
}

std::chrono::milliseconds RenX::Server::getDelay() const
{
	return RenX::Server::delay;
}

int RenX::Server::getMineLimit() const
{
	return RenX::Server::mineLimit;
}

int RenX::Server::getPlayerLimit() const
{
	return RenX::Server::playerLimit;
}

int RenX::Server::getVehicleLimit() const
{
	return RenX::Server::vehicleLimit;
}

int RenX::Server::getTimeLimit() const
{
	return RenX::Server::timeLimit;
}

int RenX::Server::getTeamMode() const
{
	return RenX::Server::m_team_mode;
}

int RenX::Server::getGameType() const
{
	return RenX::Server::m_game_type;
}

double RenX::Server::getCrateRespawnDelay() const
{
	return RenX::Server::crateRespawnAfterPickup;
}

bool RenX::Server::isSteamRequired() const
{
	return RenX::Server::steamRequired;
}

bool RenX::Server::isPrivateMessageTeamOnly() const
{
	return RenX::Server::privateMessageTeamOnly;
}

bool RenX::Server::isPrivateMessagingEnabled() const
{
	return RenX::Server::allowPrivateMessaging;
}

bool RenX::Server::isRanked() const
{
	return RenX::Server::m_ranked;
}

bool RenX::Server::isPassworded() const
{
	return RenX::Server::passworded;
}

bool RenX::Server::isCratesEnabled() const
{
	return RenX::Server::spawnCrates;
}

bool RenX::Server::isBotsEnabled() const
{
	return RenX::Server::botsEnabled;
}

const Jupiter::ReadableString &RenX::Server::getPassword() const
{
	return RenX::Server::pass;
}

const Jupiter::ReadableString &RenX::Server::getUser() const
{
	return RenX::Server::rconUser;
}

bool RenX::Server::isDevBot() const
{
	return RenX::Server::devBot;
}

const Jupiter::ReadableString &RenX::Server::getName() const
{
	return RenX::Server::serverName;
}

const RenX::Map &RenX::Server::getMap() const
{
	return RenX::Server::map;
}

RenX::GameCommand *RenX::Server::getCommand(unsigned int index) const
{
	return RenX::Server::commands.get(index);
}

RenX::GameCommand *RenX::Server::getCommand(const Jupiter::ReadableString &trigger) const
{
	RenX::GameCommand *cmd;
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
	{
		cmd = RenX::Server::commands.get(i);
		if (cmd->matches(trigger))
			return cmd;
	}
	return nullptr;
}

size_t RenX::Server::getCommandCount() const
{
	return RenX::Server::commands.size();
}

RenX::GameCommand *RenX::Server::triggerCommand(const Jupiter::ReadableString &trigger, RenX::PlayerInfo &player, const Jupiter::ReadableString &parameters)
{
	RenX::GameCommand *cmd;

	RenX::GameCommand::active_server = this;
	for (size_t i = 0; i < RenX::Server::commands.size(); i++)
	{
		cmd = RenX::Server::commands.get(i);
		if (cmd->matches(trigger))
		{
			if (player.access >= cmd->getAccessLevel())
				cmd->trigger(this, &player, parameters);
			else
				RenX::Server::sendMessage(player, "Access Denied."_jrs);

			return cmd;
		}
	}
	RenX::GameCommand::active_server = RenX::GameCommand::selected_server;

	return nullptr;
}

void RenX::Server::addCommand(RenX::GameCommand *command)
{
	int access_level;

	if (RenX::Server::commandAccessLevels != nullptr)
	{
		const Jupiter::ReadableString &accessLevel = RenX::Server::commandAccessLevels->get(command->getTrigger());
		if (accessLevel.isNotEmpty())
		{
			access_level = accessLevel.asInt();
			if (access_level < 0) // Disabled command
			{
				delete command;
				return;
			}
			command->setAccessLevel(access_level);
		}
	}

	if (RenX::Server::commandAliases != nullptr)
	{
		const Jupiter::ReadableString &aliasList = RenX::Server::commandAliases->get(command->getTrigger());
		unsigned int j = aliasList.wordCount(WHITESPACE);
		while (j != 0)
			command->addTrigger(Jupiter::ReferenceString::getWord(aliasList, --j, WHITESPACE));
	}

	RenX::Server::commands.add(command);
}

bool RenX::Server::removeCommand(RenX::GameCommand *command)
{
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
		if (RenX::Server::commands.get(i) == command)
		{
			delete RenX::Server::commands.remove(i);
			return true;
		}
	return false;
}

bool RenX::Server::removeCommand(const Jupiter::ReadableString &trigger)
{
	for (size_t i = 0; i != RenX::Server::commands.size(); i++)
		if (RenX::Server::commands.get(i)->matches(trigger))
		{
			delete RenX::Server::commands.remove(i);
			return true;
		}
	return false;
}

void RenX::Server::setUUIDFunction(RenX::Server::uuid_func func)
{
	RenX::Server::calc_uuid = func;

	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		RenX::Server::setUUIDIfDifferent(*node, RenX::Server::calc_uuid(*node));
}

RenX::Server::uuid_func RenX::Server::getUUIDFunction() const
{
	return RenX::Server::calc_uuid;
}

void RenX::Server::setUUID(RenX::PlayerInfo &player, const Jupiter::ReadableString &uuid)
{
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *getCore()->getPlugins();

	for (size_t index = 0; index < xPlugins.size(); ++index)
		xPlugins.get(index)->RenX_OnPlayerUUIDChange(*this, player, uuid);

	player.uuid = uuid;
}

bool RenX::Server::setUUIDIfDifferent(RenX::PlayerInfo &player, const Jupiter::ReadableString &uuid)
{
	if (player.uuid.equals(uuid))
		return false;

	setUUID(player, uuid);
	return true;
}

bool RenX::Server::resolvesRDNS()
{
	return RenX::Server::resolve_player_rdns;
}

void RenX::Server::sendPubChan(const char *fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = RenX::Server::getPrefix();
	if (serverPrefix.isNotEmpty())
	{
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else msg.vformat(fmt, args);
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++)
		serverManager->getServer(i)->messageChannels(RenX::Server::logChanType, msg);
}

void RenX::Server::sendPubChan(const Jupiter::ReadableString &msg) const
{
	const Jupiter::ReadableString &prefix = this->getPrefix();
	if (prefix.isNotEmpty())
	{
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::logChanType, m);
	}
	else
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::logChanType, msg);
}

void RenX::Server::sendAdmChan(const char *fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = RenX::Server::getPrefix();
	if (serverPrefix.isNotEmpty())
	{
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else msg.vformat(fmt, args);
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++)
		serverManager->getServer(i)->messageChannels(RenX::Server::adminLogChanType, msg);
}

void RenX::Server::sendAdmChan(const Jupiter::ReadableString &msg) const
{
	const Jupiter::ReadableString &prefix = this->getPrefix();
	if (prefix.isNotEmpty())
	{
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::adminLogChanType, m);
	}
	else
		for (size_t i = 0; i != serverManager->size(); i++)
			serverManager->getServer(i)->messageChannels(RenX::Server::adminLogChanType, msg);
}

void RenX::Server::sendLogChan(const char *fmt, ...) const
{
	IRC_Bot *server;
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = RenX::Server::getPrefix();
	if (serverPrefix.isNotEmpty())
	{
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else msg.vformat(fmt, args);
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++)
	{
		server = serverManager->getServer(i);
		server->messageChannels(RenX::Server::logChanType, msg);
		server->messageChannels(RenX::Server::adminLogChanType, msg);
	}
}

void RenX::Server::sendLogChan(const Jupiter::ReadableString &msg) const
{
	IRC_Bot *server;
	const Jupiter::ReadableString &prefix = this->getPrefix();
	if (prefix.isNotEmpty())
	{
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
		{
			server = serverManager->getServer(i);
			server->messageChannels(RenX::Server::logChanType, m);
			server->messageChannels(RenX::Server::adminLogChanType, m);
		}
	}
	else
		for (size_t i = 0; i != serverManager->size(); i++)
		{
			server = serverManager->getServer(i);
			server->messageChannels(RenX::Server::logChanType, msg);
			server->messageChannels(RenX::Server::adminLogChanType, msg);
		}
}

void resolve_rdns(RenX::PlayerInfo *player)
{
	player->rdns_mutex.lock();
	char *resolved = Jupiter::Socket::resolveHostname_alloc(Jupiter::CStringS(player->ip).c_str(), 0);
	player->rdns.capture(resolved, strlen(resolved));
	player->rdns_mutex.unlock();
}

#define PARSE_PLAYER_DATA_P(DATA) \
	Jupiter::ReferenceString name; \
	TeamType team; \
	int id; \
	bool isBot; \
	parsePlayerData(DATA, name, team, id, isBot);

void RenX::Server::processLine(const Jupiter::ReadableString &line)
{
	if (line.isEmpty())
		return;

	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	Jupiter::ReadableString::TokenizeResult<Jupiter::String_Strict> tokens = Jupiter::StringS::tokenize(line, this->rconVersion == 3 ? RenX::DelimC3 : RenX::DelimC);

	for (size_t index = 0; index != tokens.token_count; ++index)
		tokens.tokens[index].processEscapeSequences();

	/** Local functions */
	auto onPreGameOver = [this](RenX::WinType winType, RenX::TeamType team, int gScore, int nScore)
	{
		if (this->players.size() != 0)
		{
			for (auto node = this->players.begin(); node != this->players.end(); ++node)
			{
				if (node->team == team)
					++node->wins;
				else
					++node->loses;
			}
		}
	};
	auto onMapChange = [this]()
	{
		this->firstAction = false;
		this->firstKill = false;
		this->firstDeath = false;

		if (this->isSeamless() == false)
			this->wipePlayers();
		else
		{
			for (auto node = this->players.begin(); node != this->players.end(); ++node)
			{
				node->score = 0.0f;
				node->credits = 0.0f;
				node->kills = 0;
				node->deaths = 0;
				node->suicides = 0;
				node->headshots = 0;
				node->vehicleKills = 0;
				node->buildingKills = 0;
				node->defenceKills = 0;
				node->beaconPlacements = 0;
				node->beaconDisarms = 0;
				node->proxy_placements = 0;
				node->proxy_disarms = 0;
				node->captures = 0;
				node->steals = 0;
				node->stolen = 0;
			}
		}
	};
	auto onChat = [this](RenX::PlayerInfo &player, const Jupiter::ReadableString &message)
	{
		const Jupiter::ReadableString &prefix = this->getCommandPrefix();
		if ((player.ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_BOT) == 0 && message.find(prefix) == 0 && message.size() != prefix.size())
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
			this->triggerCommand(command, player, parameters);
		}
	};
	auto onAction = [this]()
	{
		if (this->firstAction == false)
		{
			this->firstAction = true;
		}
	};
	auto parsePlayerData = [this](const Jupiter::ReadableString &data, Jupiter::ReferenceString &name, TeamType &team, int &id, bool &isBot)
	{
		Jupiter::ReferenceString idToken = Jupiter::ReferenceString::getToken(data, 1, ',');
		name = Jupiter::ReferenceString::gotoToken(data, 2, ',');
		team = RenX::getTeam(Jupiter::ReferenceString::getToken(data, 0, ','));
		if (idToken.get(0) == 'b')
		{
			idToken.shiftRight(1);
			isBot = true;
		}
		else
			isBot = false;
		id = idToken.asInt(10);
	};
	auto getPlayerOrAdd = [&](const Jupiter::ReadableString &name, int id, RenX::TeamType team, bool isBot, uint64_t steamid, const Jupiter::ReadableString &ip, const Jupiter::ReadableString &hwid)
	{
		RenX::PlayerInfo *player = this->getPlayer(id);
		if (player == nullptr)
		{
			// Initialize a new player
			this->players.emplace_back();
			player = &this->players.back();
			player->id = id;
			player->name = name;
			player->team = team;
			player->ip = ip;
			player->ip32 = Jupiter::Socket::pton4(Jupiter::CStringS(player->ip).c_str());
			player->hwid = hwid;
			
			// RDNS
			if (this->resolvesRDNS() && player->ip32 != 0)
			{
				player->rdns_thread = std::thread(resolve_rdns, player);
				++this->player_rdns_resolutions_pending;
			}

			player->steamid = steamid;
			if (player->isBot = isBot)
				player->formatNamePrefix = IRCCOLOR "05[B]";

			player->joinTime = std::chrono::steady_clock::now();
			//if (id != 0)
			//	this->players.add(r);

			player->uuid = calc_uuid(*player);

			if (player->isBot == false)
			{
				RenX::exemptionDatabase->exemption_check(*player);
				this->banCheck(*player);
			}
			else
				++bot_count;

			for (size_t i = 0; i < xPlugins.size(); i++)
				xPlugins.get(i)->RenX_OnPlayerCreate(*this, *player);
		}
		else
		{
			bool recalcUUID = false;
			player->team = team;
			if (player->ip32 == 0 && ip.isNotEmpty())
			{
				player->ip = ip;
				player->ip32 = Jupiter::Socket::pton4(Jupiter::CStringS(player->ip).c_str());
				if (this->resolvesRDNS())
				{
					player->rdns_thread = std::thread(resolve_rdns, player);
					++this->player_rdns_resolutions_pending;
				}
				recalcUUID = true;
			}
			if (player->hwid.isEmpty() && hwid.isNotEmpty())
			{
				player->hwid = hwid;
				recalcUUID = true;
			}
			if (player->steamid == 0U && steamid != 0U)
			{
				player->steamid = steamid;
				recalcUUID = true;
			}
			if (player->name.isEmpty())
			{
				player->name = name;
				recalcUUID = true;
			}
			if (recalcUUID)
			{
				this->setUUIDIfDifferent(*player, calc_uuid(*player));
				if (player->isBot == false)
				{
					RenX::exemptionDatabase->exemption_check(*player);
					this->banCheck(*player);
				}
			}
		}

		return player;
	};
	auto parseGetPlayerOrAdd = [&parsePlayerData, &getPlayerOrAdd](const Jupiter::ReadableString &token)
	{
		PARSE_PLAYER_DATA_P(token);
		return getPlayerOrAdd(name, id, team, isBot, 0U, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty);
	};
	auto gotoToken = [&line, &tokens, this](size_t index)
	{
		if (index >= tokens.token_count)
			return Jupiter::ReferenceString::empty;

		const char delim = this->getVersion() >= 4 ? RenX::DelimC : RenX::DelimC3;
		const char *itr = line.ptr();

		while (index != 0)
		{
			if (*itr == delim)
				--index;
			++itr;
		}

		return Jupiter::ReferenceString::substring(line, itr - line.ptr());
	};
	auto finished_connecting = [this, &xPlugins]()
	{
		this->fully_connected = true;

		for (size_t index = 0; index < xPlugins.size(); ++index)
			xPlugins.get(index)->RenX_OnServerFullyConnected(*this);
	};

	if (tokens.tokens[0].isNotEmpty())
	{
		char header = tokens.tokens[0].get(0);
		tokens.tokens[0].shiftRight(1);
		switch (header)
		{
		case 'r':
			if (this->lastCommand.equalsi("clientlist"_jrs))
			{
				// ID | IP | Steam ID | Admin Status | Team | Name
				if (tokens.tokens[0].isNotEmpty())
				{
					bool isBot = false;
					int id;
					uint64_t steamid = 0;
					RenX::TeamType team = TeamType::Other;
					Jupiter::ReferenceString steamToken = tokens.getToken(2);
					Jupiter::ReferenceString adminToken = tokens.getToken(3);
					Jupiter::ReferenceString teamToken = tokens.getToken(4);
					if (tokens.tokens[0].get(0) == 'b')
					{
						isBot = true;
						tokens.tokens[0].shiftRight(1);
						id = tokens.tokens[0].asInt();
						tokens.tokens[0].shiftLeft(1);
					}
					else
						id = tokens.tokens[0].asInt();

					if (steamToken.equals("-----NO-STEAM-----") == false)
						steamid = steamToken.asUnsignedLongLong();
					team = RenX::getTeam(teamToken);

					if (adminToken.equalsi("None"_jrs))
						getPlayerOrAdd(tokens.getToken(5), id, team, isBot, steamid, tokens.getToken(1), Jupiter::ReferenceString::empty);
					else
						getPlayerOrAdd(tokens.getToken(5), id, team, isBot, steamid, tokens.getToken(1), Jupiter::ReferenceString::empty)->adminType = adminToken;
				}
			}
			else if (this->lastCommand.equalsi("clientvarlist"_jrs))
			{
				if (this->commandListFormat.token_count == 0)
					this->commandListFormat = tokens;
				else
				{
					/*e
					lRCON燙ommand;燙onn4爀xecuted:燾lientvarlist PlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rPlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rGDI,256,EKT-J񁒶񁒶񁒻217.9629燫x_FamilyInfo_GDI_Soldier牋燜alse񁒶�0.0000�8燦one�0x0110000104AE0666�127.0.0.1�256燛KT-J燝DI�0
					*/
					Jupiter::HashTable table;
					size_t i = tokens.token_count;
					while (i-- != 0)
						table.set(this->commandListFormat.getToken(i), tokens.getToken(i));
					auto parse = [&table](RenX::PlayerInfo *player)
					{
						Jupiter::ReadableString *value;

						value = table.get("KILLS"_jrs);
						if (value != nullptr)
							player->kills = value->asUnsignedInt();

						value = table.get("DEATHS"_jrs);
						if (value != nullptr)
							player->deaths = value->asUnsignedInt();

						value = table.get("SCORE"_jrs);
						if (value != nullptr)
							player->score = value->asDouble();

						value = table.get("CREDITS"_jrs);
						if (value != nullptr)
							player->credits = value->asDouble();

						value = table.get("CHARACTER"_jrs);
						if (value != nullptr)
							player->character = *value;

						value = table.get("VEHICLE"_jrs);
						if (value != nullptr)
							player->vehicle = *value;

						value = table.get("PING"_jrs);
						if (value != nullptr)
							player->ping = value->asUnsignedInt();

						value = table.get("ADMIN"_jrs);
						if (value != nullptr)
						{
							if (value->equals("None"_jrs))
								player->adminType = "";
							else
								player->adminType = *value;
						}
					};
					Jupiter::ReadableString *value = table.get("PLAYERLOG"_jrs);
					if (value != nullptr)
						parse(getPlayerOrAdd(Jupiter::ReferenceString::getToken(*value, 2, ','), Jupiter::ReferenceString::getToken(*value, 1, ',').asInt(), RenX::getTeam(Jupiter::ReferenceString::getToken(*value, 0, ',')), false, table.get("STEAM"_jrs, Jupiter::ReferenceString::empty).asUnsignedLongLong(), table.get("IP"_jrs, Jupiter::ReferenceString::empty), table.get("HWID"_jrs, Jupiter::ReferenceString::empty)));
					else
					{
						Jupiter::ReadableString *name = table.get("NAME"_jrs);
						value = table.get("ID"_jrs);

						if (value != nullptr)
						{
							RenX::PlayerInfo *player = getPlayer(value->asInt());
							if (player != nullptr)
							{
								if (player->name.isEmpty())
								{
									player->name = table.get("NAME"_jrs, Jupiter::ReferenceString::empty);
									player->name.processEscapeSequences();
								}
								if (player->ip.isEmpty())
									player->ip = table.get("IP"_jrs, Jupiter::ReferenceString::empty);
								if (player->hwid.isEmpty())
									player->hwid = table.get("HWID"_jrs, Jupiter::ReferenceString::empty);
								if (player->steamid == 0)
								{
									uint64_t steamid = table.get("STEAM"_jrs, Jupiter::ReferenceString::empty).asUnsignedLongLong();
									if (steamid != 0)
									{
										player->steamid = steamid;
										this->setUUIDIfDifferent(*player, calc_uuid(*player));
									}
								}

								value = table.get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table.get("TEAM"_jrs);
									if (value != nullptr)
										player->team = RenX::getTeam(*value);
								}

								parse(player);
							}
							// I *could* try and fetch a player by name, but that seems like it *could* open a security hole.
							// In addition, would I update their ID?
						}
						else if (name != nullptr)
						{
							RenX::PlayerInfo *player = getPlayerByName(*name);
							if (player != nullptr)
							{
								if (player->ip.isEmpty())
									player->ip = table.get("IP"_jrs, Jupiter::ReferenceString::empty);
								if (player->hwid.isEmpty())
									player->hwid = table.get("HWID"_jrs, Jupiter::ReferenceString::empty);
								if (player->steamid == 0)
								{
									uint64_t steamid = table.get("STEAM"_jrs, Jupiter::ReferenceString::empty).asUnsignedLongLong();
									if (steamid != 0)
									{
										player->steamid = steamid;
										this->setUUIDIfDifferent(*player, calc_uuid(*player));
									}
								}

								value = table.get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table.get("TEAM"_jrs);
									if (value != nullptr)
										player->team = RenX::getTeam(*value);
								}

								parse(player);
							}
							// No other way to identify player -- worthless command format.
						}
					}
				}
			}
			else if (this->lastCommand.equalsi("botlist"))
			{
				// Team,ID,Name
				if (this->commandListFormat.token_count == 0)
					this->commandListFormat = tokens;
				else
					parseGetPlayerOrAdd(tokens.tokens[0]);
			}
			else if (this->lastCommand.equalsi("botvarlist"))
			{
				if (this->commandListFormat.token_count == 0)
					this->commandListFormat = tokens;
				else
				{
					/*
					lRCON燙ommand;燙onn4爀xecuted:燾lientvarlist PlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rPlayerLog燢ills燩layerKills燘otKills燚eaths燬core燙redits燙haracter燘oundVehicle燰ehicle燬py燫emoteC4燗TMine燢DR燩ing燗dmin燬team營P營D燦ame燭eam燭eamNum
					rGDI,256,EKT-J񁒶񁒶񁒻217.9629燫x_FamilyInfo_GDI_Soldier牋燜alse񁒶�0.0000�8燦one�0x0110000104AE0666�127.0.0.1�256燛KT-J燝DI�0
					*/
					Jupiter::HashTable table;
					size_t i = tokens.token_count;
					while (i-- != 0)
						table.set(this->commandListFormat.getToken(i), tokens.getToken(i));
					auto parse = [&table](RenX::PlayerInfo *player)
					{
						Jupiter::ReadableString *value;

						value = table.get("KILLS"_jrs);
						if (value != nullptr)
							player->kills = value->asUnsignedInt();

						value = table.get("DEATHS"_jrs);
						if (value != nullptr)
							player->deaths = value->asUnsignedInt();

						value = table.get("SCORE"_jrs);
						if (value != nullptr)
							player->score = value->asDouble();

						value = table.get("CREDITS"_jrs);
						if (value != nullptr)
							player->credits = value->asDouble();

						value = table.get("CHARACTER"_jrs);
						if (value != nullptr)
							player->character = *value;

						value = table.get("VEHICLE"_jrs);
						if (value != nullptr)
							player->vehicle = *value;
					};
					Jupiter::ReadableString *value = table.get("PLAYERLOG"_jrs);
					if (value != nullptr)
						parse(getPlayerOrAdd(Jupiter::ReferenceString::getToken(*value, 2, ','), Jupiter::ReferenceString::getToken(*value, 1, ',').substring(1).asInt(), RenX::getTeam(Jupiter::ReferenceString::getToken(*value, 0, ',')), true, 0ULL, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty));
					else
					{
						Jupiter::ReadableString *name = table.get("NAME"_jrs);
						value = table.get("ID"_jrs);

						if (value != nullptr)
						{
							RenX::PlayerInfo *player = getPlayer(value->asInt());
							if (player != nullptr)
							{
								if (player->name.isEmpty())
								{
									player->name = table.get("NAME"_jrs, Jupiter::ReferenceString::empty);
									player->name.processEscapeSequences();
								}

								value = table.get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table.get("TEAM"_jrs);
									if (value != nullptr)
										player->team = RenX::getTeam(*value);
								}

								parse(player);
							}
						}
						else if (name != nullptr)
						{
							RenX::PlayerInfo *player = getPlayerByName(*name);
							if (player != nullptr)
							{
								value = table.get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table.get("TEAM"_jrs);
									if (value != nullptr)
										player->team = RenX::getTeam(*value);
								}

								parse(player);
							}
							// No other way to identify player -- worthless command format.
						}
					}
				}
			}
			else if (this->lastCommand.equalsi("binfo") || this->lastCommand.equalsi("buildinginfo") || this->lastCommand.equalsi("blist") || this->lastCommand.equalsi("buildinglist"))
			{
				if (this->commandListFormat.token_count == 0)
					this->commandListFormat = tokens;
				else
				{
					/*
					lRCON燙ommand;燚evBot爀xecuted:燽info
					rBuilding燞ealth燤axHealth燗rmor MaxArmor Team燙apturable Destroyed
					rRx_Building_Refinery_GDI�2000�2000�2000 2000 GDI燜alse False
					*/
					Jupiter::HashTable table;
					size_t i = tokens.token_count;
					while (i-- != 0)
						table.set(this->commandListFormat.getToken(i), tokens.getToken(i));

					Jupiter::ReadableString *value;
					RenX::BuildingInfo *building;

					value = table.get("Building"_jrs);
					if (value != nullptr)
					{
						building = this->getBuildingByName(*value);
						if (building == nullptr)
						{
							building = new RenX::BuildingInfo();
							RenX::Server::buildings.add(building);
							building->name = *value;
						}

						value = table.get("Health"_jrs);
						if (value != nullptr)
							building->health = value->asInt(10);

						value = table.get("MaxHealth"_jrs);
						if (value != nullptr)
							building->max_health = value->asInt(10);

						value = table.get("Team"_jrs);
						if (value != nullptr)
							building->team = RenX::getTeam(*value);

						value = table.get("Capturable"_jrs);
						if (value != nullptr)
							building->capturable = value->asBool();

						value = table.get("Destroyed"_jrs);
						if (value != nullptr)
							building->destroyed = value->asBool();

						value = table.get("Armor"_jrs);
						if (value != nullptr)
							building->armor = value->asInt(10);

						value = table.get("MaxArmor"_jrs);
						if (value != nullptr)
							building->max_armor = value->asInt(10);
					}
				}
			}
			else if (this->lastCommand.equalsi("ping"))
			{
				if (tokens.getToken(1).equals("srv_init_done"_jrs))
					finished_connecting();
				else
					RenX::Server::awaitingPong = false;
			}
			else if (this->lastCommand.equalsi("map"))
			{
				// Map | Guid
				this->map.name = tokens.getToken(0);
				const Jupiter::ReferenceString guid_token = tokens.getToken(1);

				if (guid_token.size() == 32U)
				{
					this->map.guid[0] = guid_token.substring(size_t{ 0 }, 16U).asUnsignedLongLong(16);
					this->map.guid[1] = guid_token.substring(16U).asUnsignedLongLong(16);
				}
			}
			else if (this->lastCommand.equalsi("serverinfo"))
			{
				if (this->lastCommandParams.isEmpty())
				{
					// "Port"爘 Port |�"Name" |燦ame |�"Level"爘 Level | "Players" | Players爘 "Bots" | Bots | "LevelGUID" | Level GUID
					this->port = static_cast<unsigned short>(tokens.getToken(1).asUnsignedInt(10));
					this->serverName = tokens.getToken(3);
					this->map.name = tokens.getToken(5);

					const Jupiter::ReferenceString guid_token = tokens.getToken(11);
					if (guid_token.size() == 32U)
					{
						this->map.guid[0] = guid_token.substring(size_t{ 0 }, 16U).asUnsignedLongLong(16);
						this->map.guid[1] = guid_token.substring(16U).asUnsignedLongLong(16);
					}
				}
			}
			else if (this->lastCommand.equalsi("gameinfo"_jrs))
			{
				if (this->lastCommandParams.isEmpty())
				{
					// "PlayerLimit" | PlayerLimit | "VehicleLimit" | VehicleLimit | "MineLimit" | MineLimit | "TimeLimit" | TimeLimit | "bPassworded" | bPassworded | "bSteamRequired" | bSteamRequired | "bPrivateMessageTeamOnly" | bPrivateMessageTeamOnly | "bAllowPrivateMessaging" | bAllowPrivateMessaging | "TeamMode" | TeamMode | "bSpawnCrates" | bSpawnCrates | "CrateRespawnAfterPickup" | CrateRespawnAfterPickup | bIsCompetitive | "bIsCompetitive"
					this->playerLimit = tokens.getToken(1).asInt();
					this->vehicleLimit = tokens.getToken(3).asInt();
					this->mineLimit = tokens.getToken(5).asInt();
					this->timeLimit = tokens.getToken(7).asInt();
					this->passworded = tokens.getToken(9).asBool();
					this->steamRequired = tokens.getToken(11).asBool();
					this->privateMessageTeamOnly = tokens.getToken(13).asBool();
					this->allowPrivateMessaging = tokens.getToken(15).asBool();
					this->m_team_mode = this->rconVersion >= 4 ? tokens.getToken(17).asInt() : true;
					this->spawnCrates = tokens.getToken(19).asBool();
					this->crateRespawnAfterPickup = tokens.getToken(21).asDouble();

					if (this->rconVersion >= 4)
					{
						this->competitive = tokens.getToken(23).asBool();

						const Jupiter::ReadableString &match_state_token = tokens.getToken(25);
						if (match_state_token.equalsi("PendingMatch"_jrs))
							this->match_state = 0;
						else if (match_state_token.equalsi("MatchInProgress"_jrs))
							this->match_state = 1;
						else if (match_state_token.equalsi("RoundOver"_jrs) || match_state_token.equalsi("MatchOver"_jrs))
							this->match_state = 2;
						else if (match_state_token.equalsi("TravelTheWorld"_jrs))
							this->match_state = 3;
						else // Unknown state -- assume it's in progress
							this->match_state = 1;

						this->botsEnabled = tokens.getToken(27).asBool();
						this->m_game_type = tokens.getToken(29).asInt();
					}
				}
			}
			else if (this->lastCommand.equalsi("mutatorlist"_jrs))
			{
				// "The following mutators are loaded:" [ | Mutator [ | Mutator [ ... ] ] ]
				if (tokens.token_count == 1)
					RenX::Server::pure = true;
				else if (tokens.token_count == 0)
					RenX::Server::disconnect(RenX::DisconnectReason::ProtocolError);
				else
				{
					RenX::Server::mutators.emptyAndDelete();
					size_t index = tokens.token_count;
					while (--index != 0)
						RenX::Server::mutators.add(new Jupiter::StringS(tokens.tokens[index]));
				}
			}
			else if (this->lastCommand.equalsi("rotation"_jrs))
			{
				// Map | Guid
				const Jupiter::ReadableString &in_map = tokens.getToken(0);
				if (this->hasMapInRotation(in_map) == false)
				{
					const Jupiter::ReferenceString guid_token = tokens.getToken(1);

					if (guid_token.size() == 32U)
					{
						RenX::Map *map = new RenX::Map(in_map);
						map->guid[0] = guid_token.substring(size_t{ 0 }, 16U).asUnsignedLongLong(16);
						map->guid[1] = guid_token.substring(16U).asUnsignedLongLong(16);
						this->maps.add(map);
					}
					else
						this->maps.add(new RenX::Map(in_map));
				}
			}
			else if (this->lastCommand.equalsi("changename"))
			{
				RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(0));
				Jupiter::StringS newName = tokens.getToken(2);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnNameChange(*this, *player, newName);
				player->name = tokens.getToken(2).gotoToken(2, ',');
			}
			break;
		case 'l':
			if (RenX::Server::rconVersion >= 3)
			{
				Jupiter::ReferenceString subHeader = tokens.getToken(1);
				if (tokens.tokens[0].equals("GAME"))
				{
					if (subHeader.equals("Deployed;"))
					{
						// Object (Beacon/Mine) | Player
						// Object (Beacon/Mine) | Player | "on" | Surface
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
						Jupiter::ReferenceString objectType = tokens.getToken(2);
						if (objectType.match("*Beacon"))
							++player->beaconPlacements;
						else if (objectType.equals("Rx_Weapon_DeployedProxyC4"_jrs))
							++player->proxy_placements;
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDeploy(*this, *player, objectType);
						onAction();
					}
					else if (subHeader.equals("Disarmed;"))
					{
						// Object (Beacon/Mine) | "by" | Player
						// Object (Beacon/Mine) | "by" | Player | "owned by" | Owner
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
						Jupiter::ReferenceString objectType = tokens.getToken(2);
						if (objectType.match("*Beacon"))
							++player->beaconDisarms;
						else if (objectType.equals("Rx_Weapon_DeployedProxyC4"_jrs))
							++player->proxy_disarms;

						if (tokens.getToken(5).equals("owned by"))
						{
							RenX::PlayerInfo *victim = parseGetPlayerOrAdd(tokens.getToken(6));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDisarm(*this, *player, objectType, *victim);
						}
						else
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDisarm(*this, *player, objectType);
						onAction();
					}
					else if (subHeader.equals("Exploded;"))
					{
						// Pre-5.15:
						// Explosive | "at" | Location
						// Explosive | "at" | Location | "by" | Owner
						// 5.15+:
						// Explosive | "near" | Spot Location | "at" | Location | "by" | Owner
						// Explosive | "near" | Spot Location | "at" | Location
						Jupiter::ReferenceString explosive = tokens.getToken(2);
						if (tokens.getToken(5).equals("at")) // 5.15+
						{
							if (tokens.getToken(7).equals("by")) // Player information specified
							{
								RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(8));
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnExplode(*this, *player, explosive);
							}
							else // No player information specified
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnExplode(*this, explosive);
						}
						else if (tokens.getToken(5).equals("by")) // Pre-5.15 with player information specified
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(6));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExplode(*this, *player, explosive);
						}
						else // Pre-5.15 with no player information specified
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExplode(*this, explosive);
						onAction();
					}
					else if (subHeader.equals("ProjectileExploded;"))
					{
						// Explosive | "at" | Location
						// Explosive | "at" | Location | "by" | Owner
						Jupiter::ReferenceString explosive = tokens.getToken(2);
						if (tokens.getToken(5).equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(6));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExplode(*this, *player, explosive);
						}
						else
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExplode(*this, explosive);
						onAction();
					}
					else if (subHeader.equals("Captured;"))
					{
						// Team ',' Building | "id" | Building ID | "by" | Player
						Jupiter::ReferenceString teamBuildingToken = tokens.getToken(2);
						Jupiter::ReferenceString building = teamBuildingToken.getToken(1, ',');
						TeamType oldTeam = RenX::getTeam(teamBuildingToken.getToken(0, ','));
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(6));
						player->captures++;
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnCapture(*this, *player, building, oldTeam);
						onAction();
					}
					else if (subHeader.equals("Neutralized;"))
					{
						// Team ',' Building | "id" | Building ID | "by" | Player
						Jupiter::ReferenceString teamBuildingToken = tokens.getToken(2);
						Jupiter::ReferenceString building = teamBuildingToken.getToken(1, ',');
						TeamType oldTeam = RenX::getTeam(teamBuildingToken.getToken(0, ','));
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(6));
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnNeutralize(*this, *player, building, oldTeam);
						onAction();
					}
					else if (subHeader.equals("Purchase;"))
					{
						// "character" | Character | "by" | Player
						// "item" | Item | "by" | Player
						// "weapon" | Weapon | "by" | Player
						// "refill" | Player
						// "vehicle" | Vehicle | "by" | Player
						Jupiter::ReferenceString type = tokens.getToken(2);
						Jupiter::ReferenceString obj = tokens.getToken(3);
						if (type.equals("character"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnCharacterPurchase(*this, *player, obj);
							player->character = obj;
						}
						else if (type.equals("item"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnItemPurchase(*this, *player, obj);
						}
						else if (type.equals("weapon"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnWeaponPurchase(*this, *player, obj);
						}
						else if (type.equals("refill"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(obj);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnRefillPurchase(*this, *player);
						}
						else if (type.equals("vehicle"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehiclePurchase(*this, *player, obj);
						}
					}
					else if (subHeader.equals("Spawn;"))
					{
						// "vehicle" | Vehicle Team, Vehicle
						// "player" | Player | "character" | Character
						// "bot" | Player
						if (tokens.getToken(2).equals("vehicle"))
						{
							Jupiter::ReferenceString vehicle = tokens.getToken(3);
							Jupiter::ReferenceString vehicleTeamToken = vehicle.getToken(0, ',');
							vehicle.shiftRight(vehicleTeamToken.size() + 1);
							TeamType team = RenX::getTeam(vehicleTeamToken);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleSpawn(*this, team, vehicle);
						}
						else if (tokens.getToken(2).equals("player"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(3));
							Jupiter::ReferenceString character = tokens.getToken(5);
							player->character = character;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSpawn(*this, *player, character);
						}
						else if (tokens.getToken(2).equals("bot"))
						{
							RenX::PlayerInfo *bot = parseGetPlayerOrAdd(tokens.getToken(3));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnBotJoin(*this, *bot);
						}
					}
					else if (subHeader.equals("Crate;"))
					{
						// "vehicle" | Vehicle | "by" | Player
						// "death" | "by" | Player
						// "suicide" | "by" | Player
						// "money" | Amount | "by" | Player
						// "character" | Character | "by" | Player
						// "spy" | Character | "by" | Player
						// "refill" | "by" | Player
						// "timebomb" | "by" | Player
						// "speed" | "by" | Player
						// "nuke" | "by" | Player
						// "abduction" | "by" | Player
						// "by" | Player
						Jupiter::ReferenceString type = tokens.getToken(2);
						if (type.equals("vehicle"))
						{
							Jupiter::ReferenceString vehicle = tokens.getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleCrate(*this, *player, vehicle);
						}
						else if (type.equals("tsvehicle"))
						{
							Jupiter::ReferenceString vehicle = tokens.getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleCrate(*this, *player, vehicle);
						}
						else if (type.equals("ravehicle"))
						{
							Jupiter::ReferenceString vehicle = tokens.getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVehicleCrate(*this, *player, vehicle);
						}
						else if (type.equals("death") || type.equals("suicide"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDeathCrate(*this, *player);
						}
						else if (type.equals("money"))
						{
							int amount = tokens.getToken(3).asInt();
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnMoneyCrate(*this, *player, amount);
						}
						else if (type.equals("character"))
						{
							Jupiter::ReferenceString character = tokens.getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnCharacterCrate(*this, *player, character);
							player->character = character;
						}
						else if (type.equals("spy"))
						{
							Jupiter::ReferenceString character = tokens.getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSpyCrate(*this, *player, character);
							player->character = character;
						}
						else if (type.equals("refill"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnRefillCrate(*this, *player);
						}
						else if (type.equals("timebomb"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnTimeBombCrate(*this, *player);
						}
						else if (type.equals("speed"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSpeedCrate(*this, *player);
						}
						else if (type.equals("nuke"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnNukeCrate(*this, *player);
						}
						else if (type.equals("abduction"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnAbductionCrate(*this, *player);
						}
						else if (type.equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(3));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnUnspecifiedCrate(*this, *player);
						}
						else
						{
							RenX::PlayerInfo *player = nullptr;
							if (tokens.getToken(3).equals("by"))
								player = parseGetPlayerOrAdd(tokens.getToken(4));

							if (player != nullptr)
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnOtherCrate(*this, *player, type);
						}
					}
					else if (subHeader.equals("Death;"))
					{
						// "player" | Player | "by" | Killer Player | "with" | Damage Type
						// "player" | Player | "died by" | Damage Type
						// "player" | Player | "suicide by" | Damage Type
						//		NOTE: Filter these out when Player.isEmpty().
						Jupiter::ReferenceString playerToken = tokens.getToken(3);
						if (playerToken.isNotEmpty())
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(playerToken);
							Jupiter::ReferenceString type = tokens.getToken(4);
							Jupiter::ReferenceString damageType;
							if (type.equals("by"))
							{
								damageType = tokens.getToken(7);
								Jupiter::ReferenceString killerData = tokens.getToken(5);
								Jupiter::ReferenceString kName = killerData.getToken(2, ',');
								Jupiter::ReferenceString kIDToken = killerData.getToken(1, ',');
								RenX::TeamType vTeam = RenX::getTeam(killerData.getToken(0, ','));
								if (kIDToken.equals("ai") || kIDToken.isEmpty())
								{
									player->deaths++;
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnKill(*this, kName, vTeam, *player, damageType);
								}
								else
								{
									player->deaths++;
									int kID = 0;
									bool kIsBot = false;
									if (kIDToken.get(0) == 'b')
									{
										kIsBot = true;
										kIDToken.shiftRight(1);
										kID = kIDToken.asInt();
										kIDToken.shiftLeft(1);
									}
									else
										kID = kIDToken.asInt();
									RenX::PlayerInfo *killer = getPlayerOrAdd(kName, kID, vTeam, kIsBot, 0, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty);
									killer->kills++;
									if (damageType.equals("Rx_DmgType_Headshot"))
										killer->headshots++;
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnKill(*this, *killer, *player, damageType);
								}
							}
							else if (type.equals("died by"))
							{
								player->deaths++;
								damageType = tokens.getToken(5);
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnDie(*this, *player, damageType);
							}
							else if (type.equals("suicide by"))
							{
								player->deaths++;
								player->suicides++;
								damageType = tokens.getToken(5);
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnSuicide(*this, *player, damageType);
							}
							player->character = Jupiter::ReferenceString::empty;
						}
						onAction();
					}
					else if (subHeader.equals("Stolen;"))
					{
						// Vehicle | "by" | Player
						// Vehicle | "bound to" | Bound Player | "by" | Player
						Jupiter::ReferenceString vehicle = tokens.getToken(2);
						Jupiter::ReferenceString byLine = tokens.getToken(3);
						if (byLine.equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							player->steals++;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSteal(*this, *player, vehicle);
						}
						else if (byLine.equals("bound to"))
						{
							RenX::PlayerInfo *victim = parseGetPlayerOrAdd(tokens.getToken(4));
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(6));
							player->steals++;
							victim->stolen++;
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnSteal(*this, *player, vehicle, *victim);
						}
						onAction();
					}
					else if (subHeader.equals("Destroyed;"))
					{
						// "vehicle" | Vehicle | "by" | Killer | "with" | Damage Type
						// "defence" | Defence | "by" | Killer | "with" | Damage Type
						// "emplacement" | Emplacement | "by" | Killer Player | "with" | Damage Type
						// "building" | Building | "by" | Killer | "with" | Damage Type
						Jupiter::ReferenceString typeToken = tokens.getToken(2);
						RenX::ObjectType type = ObjectType::None;
						if (typeToken.equals("vehicle"))
							type = ObjectType::Vehicle;
						else if (typeToken.equals("defence") || typeToken.equals("emplacement"))
							type = ObjectType::Defence;
						else if (typeToken.equals("building"))
							type = ObjectType::Building;

						if (type != ObjectType::None)
						{
							Jupiter::ReferenceString objectName = tokens.getToken(3);
							if (tokens.getToken(4).equals("by"))
							{
								Jupiter::ReferenceString killerToken = tokens.getToken(5);
								Jupiter::ReferenceString idToken = killerToken.getToken(1, ',');
								Jupiter::ReferenceString name = killerToken.gotoToken(2, ',');
								Jupiter::ReferenceString damageType = tokens.getToken(7);

								RenX::TeamType team = RenX::getTeam(killerToken.getToken(0, ','));

								if (idToken.equals("ai") || idToken.isEmpty())
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnDestroy(*this, name, team, objectName, RenX::getEnemy(team), damageType, type);
								else
								{
									int id;
									bool isBot = false;
									if (idToken.get(0) == 'b')
									{
										isBot = true;
										idToken.shiftRight(1);
									}
									id = idToken.asInt();
									RenX::PlayerInfo *player = getPlayerOrAdd(name, id, team, isBot, 0, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty);
									switch (type)
									{
									case RenX::ObjectType::Vehicle:
										player->vehicleKills++;
										break;
									case RenX::ObjectType::Building:
										player->buildingKills++;
										{
											auto internalsStr = "_Internals"_jrs;
											RenX::BuildingInfo *building;
											if (objectName.findi(internalsStr) != Jupiter::INVALID_INDEX)
												objectName.truncate(internalsStr.size());
											building = RenX::Server::getBuildingByName(objectName);
											if (building != nullptr)
											{
												building->health = 0;
												building->armor = 0;
												building->destroyed = true;
												building->destruction_time = std::chrono::steady_clock::now();
											}
										}

										break;
									case RenX::ObjectType::Defence:
										player->defenceKills++;
										break;
									default:
										break;
									}
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnDestroy(*this, *player, objectName, RenX::getEnemy(player->team), damageType, type);
								}
							}
						}
						onAction();
					}
					else if (subHeader.equals("Donated;"))
					{
						// Amount | "to" | Recipient | "by" | Donor
						if (tokens.getToken(5).equals("by"))
						{
							double amount = tokens.getToken(2).asDouble();
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(4));
							RenX::PlayerInfo *donor = parseGetPlayerOrAdd(tokens.getToken(6));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDonate(*this, *donor, *player, amount);
						}
					}
					else if (subHeader.equals("OverMine;"))
					{
						// Player | "near" | Location
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						Jupiter::ReferenceString location = tokens.getToken(4);

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnOverMine(*this, *player, location);
					}
					else if (subHeader.equals("MatchEnd;"))
					{
						// "winner" | Winner | Reason("TimeLimit" etc) | "GDI=" GDI Score | "Nod=" Nod Score
						// "tie" | Reason | "GDI=" GDI Score | "Nod=" Nod Score
						Jupiter::ReferenceString winTieToken = tokens.getToken(2);
						this->match_state = 2;

						if (winTieToken.equals("winner"))
						{
							Jupiter::ReferenceString sWinType = tokens.getToken(4);
							WinType winType = WinType::Unknown;
							if (sWinType.equals("TimeLimit"))
								winType = WinType::Score;
							else if (sWinType.equals("Buildings"))
								winType = WinType::Base;
							else if (sWinType.equals("triggered"))
								winType = WinType::Shutdown;
							else if (sWinType.equals("Surrender"))
								winType = WinType::Surrender;

							TeamType team = RenX::getTeam(tokens.getToken(3));

							int gScore = tokens.getToken(5).getToken(1, '=').asInt();
							int nScore = tokens.getToken(6).getToken(1, '=').asInt();

							onPreGameOver(winType, team, gScore, nScore);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnGameOver(*this, winType, team, gScore, nScore);
						}
						else if (winTieToken.equals("tie"))
						{
							int gScore = tokens.getToken(4).getToken(1, '=').asInt();
							int nScore = tokens.getToken(5).getToken(1, '=').asInt();
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnGameOver(*this, RenX::WinType::Tie, RenX::TeamType::None, gScore, nScore);
						}
						this->gameover_pending = false;
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnGame(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("CHAT"))
				{
					if (subHeader.equals("Say;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						Jupiter::ReferenceString message = tokens.getToken(4);
						onChat(*player, message);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnChat(*this, *player, message);
						onAction();
					}
					else if (subHeader.equals("TeamSay;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						Jupiter::ReferenceString message = tokens.getToken(4);
						onChat(*player, message);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnTeamChat(*this, *player, message);
						onAction();
					}
					else if (subHeader.equals("Radio;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						Jupiter::ReferenceString message = tokens.getToken(4);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnRadioChat(*this, *player, message);
						onAction();
					}
					else if (subHeader.equals("HostSay;"))
					{
						Jupiter::ReferenceString message = tokens.getToken(3);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnHostChat(*this, message);
					}
					/*else if (subHeader.equals("AdminSay;"))
					{
						// Player | "said:" | Message
						onAction();
					}
					else if (subHeader.equals("ReportSay;"))
					{
						// Player | "said:" | Message
						onAction();
					}*/
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnOtherChat(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("PLAYER"))
				{
					if (subHeader.equals("Enter;"))
					{
						PARSE_PLAYER_DATA_P(tokens.getToken(2));
						uint64_t steamid = 0;
						RenX::PlayerInfo *player;

						if (tokens.getToken(5).equals("hwid"))
						{
							// New format
							if (tokens.getToken(7).equals("steamid"))
								steamid = tokens.getToken(8).asUnsignedLongLong();

							player = getPlayerOrAdd(name, id, team, isBot, steamid, tokens.getToken(4), tokens.getToken(6));
						}
						else
						{
							// Old format
							if (tokens.getToken(5).equals("steamid"))
								steamid = tokens.getToken(6).asUnsignedLongLong();

							player = getPlayerOrAdd(name, id, team, isBot, steamid, tokens.getToken(4), Jupiter::ReferenceString::empty);
						}

						if (steamid != 0ULL && default_ladder_database != nullptr && (player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_LADDER) == 0)
						{
							RenX::LadderDatabase::Entry *itr = RenX::default_ladder_database->getHead();
							while (itr != nullptr)
							{
								if (itr->steam_id == steamid)
								{
									player->local_rank = itr->rank;
									if (this->devBot)
									{
										player->global_rank = itr->rank;
										if (this->rconVersion >= 4)
											this->sendData(Jupiter::StringS::Format("dset_rank %d %d\n", player->id, player->global_rank));
									}
									break;
								}

								itr = itr->next;
							}
						}
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnJoin(*this, *player);
					}
					else if (subHeader.equals("TeamJoin;"))
					{
						// Player | "joined" | Team
						// Player | "joined" | Team | "left" | Old Team
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						player->character = Jupiter::ReferenceString::empty;
						if (tokens.token_count > 4)
						{
							RenX::TeamType oldTeam = RenX::getTeam(tokens.getToken(6));
							if (oldTeam != RenX::TeamType::None)
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnTeamChange(*this, *player, oldTeam);
						}
					}
					else if (subHeader.equals("HWID;"))
					{
						// ["player" |] Player | "hwid" | HWID
						size_t offset = 0;
						if (tokens.getToken(2).equals("player"))
							offset = 1;

						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2 + offset));
						player->hwid = tokens.getToken(4 + offset);

						if (player->isBot == false)
							this->banCheck(*player);

						for (size_t index = 0; index < xPlugins.size(); ++index)
							xPlugins.get(index)->RenX_OnHWID(*this, *player);

						if (player->rdns.isNotEmpty())
							for (size_t index = 0; index < xPlugins.size(); ++index)
								xPlugins.get(index)->RenX_OnPlayerIdentify(*this, *player);
					}
					else if (subHeader.equals("Exit;"))
					{
						// Player
						Jupiter::ReferenceString playerToken = tokens.getToken(2);
						PARSE_PLAYER_DATA_P(playerToken);

						RenX::PlayerInfo *player = getPlayer(id);
						//RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));

						if (player != nullptr)
						{
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnPart(*this, *player);

							this->removePlayer(*player);
						}

						if (this->gameover_when_empty && this->players.size() == this->getBotCount())
							this->gameover();
					}
					else if (subHeader.equals("Kick;"))
					{
						// Player | "for" | Reason
						const Jupiter::ReadableString &reason = tokens.getToken(4);
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnKick(*this, *player, reason);
					}
					else if (subHeader.equals("NameChange;"))
					{
						// Player | "to:" | New Name
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						Jupiter::StringS newName = tokens.getToken(4);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnNameChange(*this, *player, newName);
						player->name = newName;
						onAction();
					}
					else if (subHeader.equals("ChangeID;"))
					{
						// "to" | New ID | "from" | Old ID
						int oldID = tokens.getToken(5).asInt();
						RenX::PlayerInfo *player = this->getPlayer(oldID);
						if (player != nullptr)
						{
							player->id = tokens.getToken(3).asInt();

							if (player->isBot == false)
								this->banCheck(*player);

							if (this->devBot && player->global_rank != 0U)
							{
								if (this->rconVersion >= 4)
									this->sendData(Jupiter::StringS::Format("dset_rank %d %d\n", player->id, player->global_rank));
							}

							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnIDChange(*this, *player, oldID);
						}
					}
					else if (subHeader.equals("Rank;"))
					{
						// Player | Rank
						if (this->devBot == false)
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
							if (player != nullptr)
								player->global_rank = tokens.getToken(3).asUnsignedInt();

							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnRank(*this, *player);
						}
					}
					else if (subHeader.equals("Dev;"))
					{
						// Player | true/false
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						if (player != nullptr)
							player->is_dev = tokens.getToken(3).asBool();

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDev(*this, *player);
					}
					else if (subHeader.equals("SpeedHack;"))
					{
						// Player
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnSpeedHack(*this, *player);
					}
					else if (subHeader.equals("Command;"))
					{
						// Player | Command
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						Jupiter::ReferenceString message = gotoToken(3);

						RenX::GameCommand *command = this->triggerCommand(Jupiter::ReferenceString::getWord(message, 0, WHITESPACE), *player, Jupiter::ReferenceString::gotoWord(message, 1, WHITESPACE));

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnPlayerCommand(*this, *player, message, command);
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnPlayer(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("RCON"))
				{
					if (subHeader.equals("Command;"))
					{
						// User | "executed:" | Command
						Jupiter::ReferenceString user = tokens.getToken(2);
						if (tokens.getToken(3).equals("executed:"))
						{
							Jupiter::ReferenceString command = gotoToken(4);
							Jupiter::ReferenceString cmd = command.getWord(0, " ");

							if (cmd.equalsi("hostprivatesay"))
							{
								RenX::PlayerInfo *player = this->getPlayerByName(command.getWord(1, " "));
								if (player != nullptr)
								{
									Jupiter::ReferenceString message = command.gotoWord(2, " ");
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnHostPage(*this, *player, message);
								}
								else
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnExecute(*this, user, command);
							}
							else
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnExecute(*this, user, command);

							if (this->rconUser.equals(user))
							{
								this->lastCommand = cmd;
								this->lastCommandParams = command.gotoWord(1, " ");
							}
						}
					}
					else if (subHeader.equals("Subscribed;"))
					{
						// User
						Jupiter::ReferenceString user = tokens.getToken(2);

						if (user.equals(this->rconUser))
							this->subscribed = true;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnSubscribe(*this, user);
					}
					else if (subHeader.equals("Unsubscribed;"))
					{
						// User
						Jupiter::ReferenceString user = tokens.getToken(2);

						if (user.equals(this->rconUser))
							this->subscribed = false;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnUnsubscribe(*this, user);
					}
					else if (subHeader.equals("Blocked;"))
					{
						// User | Reason="(Denied by IP Policy)" / "(Not on Whitelist)"
						Jupiter::ReferenceString user = tokens.getToken(2);
						Jupiter::ReferenceString message = tokens.getToken(3);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnBlock(*this, user, message);
					}
					else if (subHeader.equals("Connected;"))
					{
						// User
						Jupiter::ReferenceString user = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnConnect(*this, user);
					}
					else if (subHeader.equals("Authenticated;"))
					{
						// User
						Jupiter::ReferenceString user = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAuthenticate(*this, user);
					}
					else if (subHeader.equals("Banned;"))
					{
						// User | "reason" | Reason="(Too many password attempts)"
						Jupiter::ReferenceString user = tokens.getToken(2);
						Jupiter::ReferenceString message = tokens.getToken(4);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnBan(*this, user, message);
					}
					else if (subHeader.equals("InvalidPassword;"))
					{
						// User
						Jupiter::ReferenceString user = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnInvalidPassword(*this, user);
					}
					else if (subHeader.equals("Dropped;"))
					{
						// User | "reason" | Reason="(Auth Timeout)"
						Jupiter::ReferenceString user = tokens.getToken(2);
						Jupiter::ReferenceString message = tokens.getToken(4);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDrop(*this, user, message);
					}
					else if (subHeader.equals("Disconnected;"))
					{
						// User
						Jupiter::ReferenceString user = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDisconnect(*this, user);
					}
					else if (subHeader.equals("StoppedListen;"))
					{
						// Reason="(Reached Connection Limit)"
						Jupiter::ReferenceString message = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnStopListen(*this, message);
					}
					else if (subHeader.equals("ResumedListen;"))
					{
						// Reason="(No longer at Connection Limit)"
						Jupiter::ReferenceString message = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnResumeListen(*this, message);
					}
					else if (subHeader.equals("Warning;"))
					{
						// Warning="(Hit Max Attempt Records - You should investigate Rcon attempts and/or decrease prune time)"
						Jupiter::ReferenceString message = tokens.getToken(2);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnWarning(*this, message);
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnRCON(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("ADMIN"))
				{
					if (subHeader.equals("Rcon;"))
					{
						// Player | "executed:" | Command
						if (tokens.getToken(3).equals("executed:"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
							Jupiter::ReferenceString cmd = gotoToken(4);
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnExecute(*this, *player, cmd);
						}
					}
					else if (subHeader.equals("Login;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						player->adminType = tokens.getToken(4);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdminLogin(*this, *player);
					}
					else if (subHeader.equals("Logout;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdminLogout(*this, *player);

						player->adminType.erase();
					}
					else if (subHeader.equals("Granted;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(2));
						player->adminType = tokens.getToken(4);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdminGrant(*this, *player);
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnAdmin(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("VOTE"))
				{
					if (subHeader.equals("Called;"))
					{
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | "by" | Player
						// Pre-5.15:
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | "parameters" | Parameters(Empty) | "by" | Player
						// 5.15+:
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | "by" | Player | Parameters (Key | Value [ ... | Key | Value ] )
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_AddBots"  | "by" | Player | "team" | TargetTeam="GDI" / "Nod" / "Both" | "amount" | amount | "skill" | skill
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_ChangeMap" | "by" | Player
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_Kick" | "by" | Player | "player" | Target Player
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_MineBan"  | "by" | Player | "player" | Target Player
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_RemoveBots" | "by" | Player | "team" | TargetTeam="GDI" / "Nod" / "Both" | "amount" | amount
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_RestartMap" | "by" | Player
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_Surrender" | "by" | Player
						// TeamType="Global" / "GDI" / "Nod" / "" | "Rx_VoteMenuChoice_Survey" | "by" | Player | "text" | Survey Text

						Jupiter::ReferenceString voteType = tokens.getToken(3);
						Jupiter::ReferenceString teamToken = tokens.getToken(2);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						if (tokens.getToken(4).equals("parameters")) // Pre-5.15 style parameters; throw away parameters
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(tokens.token_count - 1));

							if ((player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_VOTE) != 0)
								RenX::Server::sendData(Jupiter::StringS::Format("ccancelvote %.*s\n", teamToken.size(), teamToken.ptr()));

							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnVoteOther(*this, team, voteType, *player);
						}
						else // 5.15+ (or empty)
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(5));

							if ((player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_VOTE) != 0)
								RenX::Server::sendData(Jupiter::StringS::Format("ccancelvote %.*s\n", teamToken.size(), teamToken.ptr()));

							// PARSE PARAMETERS HERE

							if (voteType.find("Rx_VoteMenuChoice_"_jrs) == 0)
							{
								voteType.shiftRight(18);

								if (voteType.equals("AddBots"_jrs))
								{
									Jupiter::ReferenceString victimToken = tokens.getToken(7);
									RenX::TeamType victim;
									if (teamToken.equals("Global"))
										victim = TeamType::None;
									else if (teamToken.equals("GDI"))
										victim = TeamType::GDI;
									else if (teamToken.equals("Nod"))
										victim = TeamType::Nod;
									else
										victim = TeamType::Other;

									int amount = tokens.getToken(9).asInt(10);
									int skill = tokens.getToken(11).asInt(10);

									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteAddBots(*this, team, *player, victim, amount, skill);
								}
								else if (voteType.equals("ChangeMap"_jrs))
								{
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteChangeMap(*this, team, *player);
								}
								else if (voteType.equals("Kick"_jrs))
								{
									RenX::PlayerInfo *victim = parseGetPlayerOrAdd(tokens.getToken(7));
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteKick(*this, team, *player, *victim);
								}
								else if (voteType.equals("MineBan"_jrs))
								{
									RenX::PlayerInfo *victim = parseGetPlayerOrAdd(tokens.getToken(7));
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteMineBan(*this, team, *player, *victim);
								}
								else if (voteType.equals("RemoveBots"_jrs))
								{
									Jupiter::ReferenceString victimToken = tokens.getToken(7);
									RenX::TeamType victim;
									if (teamToken.equals("Global"))
										victim = TeamType::None;
									else if (teamToken.equals("GDI"))
										victim = TeamType::GDI;
									else if (teamToken.equals("Nod"))
										victim = TeamType::Nod;
									else
										victim = TeamType::Other;

									int amount = tokens.getToken(9).asInt(10);

									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteRemoveBots(*this, team, *player, victim, amount);
								}
								else if (voteType.equals("RestartMap"_jrs))
								{
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteRestartMap(*this, team, *player);
								}
								else if (voteType.equals("Surrender"_jrs))
								{
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteSurrender(*this, team, *player);
								}
								else if (voteType.equals("Survey"_jrs))
								{
									const Jupiter::ReadableString &text = tokens.getToken(7);
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteSurvey(*this, team, *player, text);
								}
								else
								{
									voteType.shiftLeft(18);
									for (size_t i = 0; i < xPlugins.size(); i++)
										xPlugins.get(i)->RenX_OnVoteOther(*this, team, voteType, *player);
								}
							}
							else
								for (size_t i = 0; i < xPlugins.size(); i++)
									xPlugins.get(i)->RenX_OnVoteOther(*this, team, voteType, *player);
						}
						onAction();
					}
					else if (subHeader.equals("Results;"))
					{
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | Success="pass" / "fail" | "Yes=" Yes votes | "No=" No votes
						Jupiter::ReferenceString voteType = tokens.getToken(3);
						Jupiter::ReferenceString teamToken = tokens.getToken(2);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						bool success = true;
						if (tokens.getToken(4).equals("fail"))
							success = false;

						int yesVotes = 0;
						Jupiter::ReferenceString votes_token = tokens.getToken(5);
						if (votes_token.size() > 4)
						{
							votes_token.shiftRight(4);
							yesVotes = votes_token.asInt();
						}

						int noVotes = 0;
						votes_token = tokens.getToken(6);
						if (votes_token.size() > 3)
						{
							votes_token.shiftRight(3);
							noVotes = votes_token.asInt();
						}

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVoteOver(*this, team, voteType, success, yesVotes, noVotes);
					}
					else if (subHeader.equals("Cancelled;"))
					{
						// TeamType="Global" / "GDI" / "Nod" | VoteType="Rx_VoteMenuChoice_"...
						Jupiter::ReferenceString voteType = tokens.getToken(3);
						Jupiter::ReferenceString teamToken = tokens.getToken(2);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVoteCancel(*this, team, voteType);
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnVote(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("MAP"))
				{
					if (subHeader.equals("Changing;"))
					{
						// Map | Mode="seamless" / "nonseamless"
						Jupiter::ReferenceString map = tokens.getToken(2);

						this->match_state = 3;
						if (tokens.getToken(3).equals("seamless"))
							this->seamless = true;
						else
							this->seamless = false;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMapChange(*this, map, seamless);

						this->map = map;
						onMapChange();
					}
					else if (subHeader.equals("Loaded;"))
					{
						// Map
						Jupiter::ReferenceString map = tokens.getToken(2);

						this->match_state = 0;
						this->map = map;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMapLoad(*this, map);
					}
					else if (subHeader.equals("Start;"))
					{
						// Map
						Jupiter::ReferenceString map = tokens.getToken(2);

						this->match_state = 1;
						this->reliable = true;
						this->gameStart = std::chrono::steady_clock::now();
						this->map = map;

						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMapStart(*this, map);
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnMap(*this, raw);
					}
				}
				else if (tokens.tokens[0].equals("DEMO"))
				{
					if (subHeader.equals("Record;"))
					{
						// "client request by" | Player
						// "admin command by" | Player
						// "rcon command"
						Jupiter::ReferenceString type = tokens.getToken(2);
						if (type.equals("client request by") || type.equals("admin command by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(tokens.getToken(3));
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDemoRecord(*this, *player);
						}
						else
						{
							Jupiter::ReferenceString user = tokens.getToken(3); // not actually used, but here for possible future usage
							for (size_t i = 0; i < xPlugins.size(); i++)
								xPlugins.get(i)->RenX_OnDemoRecord(*this, user);
						}
					}
					else if (subHeader.equals("RecordStop;"))
					{
						// Empty
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDemoRecordStop(*this);
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (size_t i = 0; i < xPlugins.size(); i++)
							xPlugins.get(i)->RenX_OnDemo(*this, raw);
					}
				}
				/*else if (tokens.tokens[0].equals("ERROR;")) // Decided to disable this entirely, since it's unreachable anyways.
				{
					// Should be under RCON.
					// "Could not open TCP Port" Port "- Rcon Disabled"
				}*/
				else
				{
					Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnLog(*this, raw);
				}
			}
			break;

		case 'd':
			{
				// We don't actually need to use this output for anything atm; tend to this later.
			}
			break;

		case 'c':
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnCommand(*this, raw);
				this->commandListFormat.erase();
				this->lastCommand = Jupiter::ReferenceString::empty;
				this->lastCommandParams = Jupiter::ReferenceString::empty;
			}
			break;

		case 'e':
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnError(*this, raw);
			}
			break;

		case 'v':
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				this->rconVersion = raw.asInt(10);
				
				if (this->rconVersion >= 3)
				{
					if (this->rconVersion == 3) // Old format: 003Open Beta 5.12
						this->gameVersion = raw.substring(3);
					else // New format: 004 | Game Version Number | Game Version
					{
						this->gameVersionNumber = tokens.getToken(1).asInt(10);
						this->gameVersion = tokens.getToken(2);

						if (this->gameVersion.isEmpty())
						{
							RenX::Server::sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from Renegade-X server (Protocol Error)."));
							this->disconnect(RenX::DisconnectReason::ProtocolError);
							break;
						}
					}

					RenX::Server::sock.send("s\n"_jrs);
					RenX::Server::send("serverinfo"_jrs);
					RenX::Server::send("gameinfo"_jrs);
					RenX::Server::send("gameinfo bIsCompetitive"_jrs);
					RenX::Server::send("mutatorlist"_jrs);
					RenX::Server::send("rotation"_jrs);
					RenX::Server::fetchClientList();
					RenX::Server::updateBuildingList();
					RenX::Server::send("ping srv_init_done"_jrs);

					RenX::Server::gameStart = std::chrono::steady_clock::now();
					this->seamless = true;

					for (size_t i = 0; i < xPlugins.size(); i++)
						xPlugins.get(i)->RenX_OnVersion(*this, raw);
				}
				else
				{
					RenX::Server::sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from Renegade-X server (incompatible RCON version)."));
					this->disconnect(RenX::DisconnectReason::IncompatibleVersion);
				}
			}
			break;

		case 'a':
			{
				RenX::Server::rconUser = Jupiter::ReferenceString::substring(line, 1);
				if (rconUser.equals(RenX::DevBotName))
					RenX::Server::devBot = true;

				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnAuthorized(*this, RenX::Server::rconUser);
			}
			break;

		default:
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				for (size_t i = 0; i < xPlugins.size(); i++)
					xPlugins.get(i)->RenX_OnOther(*this, header, raw);
			}
			break;
		}
		for (size_t i = 0; i < xPlugins.size(); i++)
			xPlugins.get(i)->RenX_OnRaw(*this, line);
	}
}

void RenX::Server::disconnect(RenX::DisconnectReason reason)
{
	RenX::Server::connected = false;

	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		xPlugins.get(i)->RenX_OnServerDisconnect(*this, reason);

	RenX::Server::sock.close();
	RenX::Server::wipeData();
}

bool RenX::Server::connect()
{
	RenX::Server::lastAttempt = std::chrono::steady_clock::now();
	if (RenX::Server::sock.connect(RenX::Server::hostname.c_str(), RenX::Server::port, RenX::Server::clientHostname.isEmpty() ? nullptr : RenX::Server::clientHostname.c_str()))
	{
		RenX::Server::sock.setBlocking(false);
		RenX::Server::sock.send(Jupiter::StringS::Format("a%.*s\n", RenX::Server::pass.size(), RenX::Server::pass.ptr()));
		RenX::Server::connected = true;
		RenX::Server::attempts = 0;
		return true;
	}
	RenX::Server::connected = false;
	++RenX::Server::attempts;
	return false;
}

bool RenX::Server::reconnect(RenX::DisconnectReason reason)
{
	RenX::Server::disconnect(static_cast<RenX::DisconnectReason>(static_cast<unsigned int>(reason) | 0x01));
	return RenX::Server::connect();
}

void RenX::Server::wipeData()
{
	RenX::Server::wipePlayers();
	RenX::Server::reliable = false;
	RenX::Server::m_team_mode = 3;
	RenX::Server::m_game_type = 1;
	RenX::Server::m_ranked = false;
	RenX::Server::botsEnabled = true;
	RenX::Server::match_state = 1;
	RenX::Server::subscribed = false;
	RenX::Server::fully_connected = false;
	RenX::Server::bot_count = 0;
	RenX::Server::player_rdns_resolutions_pending = 0;
	RenX::Server::buildings.emptyAndDelete();
	RenX::Server::mutators.emptyAndDelete();
	RenX::Server::maps.emptyAndDelete();
	RenX::Server::awaitingPong = false;
	RenX::Server::rconVersion = 0;
	RenX::Server::rconUser.truncate(RenX::Server::rconUser.size());
}

void RenX::Server::wipePlayers()
{
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();

	while (RenX::Server::players.size() != 0)
	{
		for (size_t index = 0; index < xPlugins.size(); ++index)
			xPlugins.get(index)->RenX_OnPlayerDelete(*this, this->players.front());

		if (this->players.front().rdns_thread.joinable()) // Close the RDNS thread, if one exists
		{
			--this->player_rdns_resolutions_pending;
			this->players.front().rdns_thread.join();
		}

		this->players.pop_front();
	}
}

unsigned int RenX::Server::getVersion() const
{
	return RenX::Server::rconVersion;
}

unsigned int RenX::Server::getGameVersionNumber() const
{
	return RenX::Server::gameVersionNumber;
}

const Jupiter::ReadableString &RenX::Server::getGameVersion() const
{
	return RenX::Server::gameVersion;
}

const Jupiter::ReadableString &RenX::Server::getRCONUsername() const
{
	return RenX::Server::rconUser;
}

RenX::Server::Server(Jupiter::Socket &&socket, const Jupiter::ReadableString &configurationSection) : Server(configurationSection)
{
	RenX::Server::sock = std::move(socket);
	RenX::Server::hostname = RenX::Server::sock.getRemoteHostname();
	RenX::Server::sock.send(Jupiter::StringS::Format("a%.*s\n", RenX::Server::pass.size(), RenX::Server::pass.ptr()));
	RenX::Server::connected = true;
}

RenX::Server::Server(const Jupiter::ReadableString &configurationSection)
{
	RenX::Server::configSection = configurationSection;
	RenX::Server::calc_uuid = RenX::default_uuid_func;
	init(*RenX::getCore()->getConfig().getSection(RenX::Server::configSection));
	Jupiter::ArrayList<RenX::Plugin> &xPlugins = *RenX::getCore()->getPlugins();
	for (size_t i = 0; i < xPlugins.size(); i++)
		xPlugins.get(i)->RenX_OnServerCreate(*this);
}

void RenX::Server::init(const Jupiter::Config &config)
{
	RenX::Server::hostname = config.get("Hostname"_jrs, "localhost"_jrs);
	RenX::Server::port = config.get<unsigned short>("Port"_jrs, 7777);
	RenX::Server::clientHostname = config.get("ClientAddress"_jrs);
	RenX::Server::pass = config.get("Password"_jrs, "renx"_jrs);

	RenX::Server::logChanType = config.get<int>("ChanType"_jrs);
	RenX::Server::adminLogChanType = config.get<int>("AdminChanType"_jrs);

	RenX::Server::setCommandPrefix(config.get("CommandPrefix"_jrs));
	RenX::Server::setPrefix(config.get("IRCPrefix"_jrs));

	RenX::Server::ban_from_str = config.get("BanFromStr"_jrs, "the server"_jrs);
	RenX::Server::rules = config.get("Rules"_jrs, "Anarchy!"_jrs);
	RenX::Server::delay = std::chrono::milliseconds(config.get<long long>("ReconnectDelay"_jrs, 10000));
	RenX::Server::maxAttempts = config.get<int>("MaxReconnectAttempts"_jrs, -1);
	RenX::Server::rconBan = config.get<bool>("RCONBan"_jrs, false);
	RenX::Server::localSteamBan = config.get<bool>("LocalSteamBan"_jrs, true);
	RenX::Server::localIPBan = config.get<bool>("LocalIPBan"_jrs, true);
	RenX::Server::localHWIDBan = config.get<bool>("LocalHWIDBan"_jrs, true);
	RenX::Server::localRDNSBan = config.get<bool>("LocalRDNSBan"_jrs, false);
	RenX::Server::localNameBan = config.get<bool>("LocalNameBan"_jrs, false);
	RenX::Server::localBan = RenX::Server::localIPBan || RenX::Server::localRDNSBan || RenX::Server::localSteamBan || RenX::Server::localNameBan;
	RenX::Server::steamFormat = config.get<int>("SteamFormat"_jrs, 16);
	RenX::Server::neverSay = config.get<bool>("NeverSay"_jrs, false);
	RenX::Server::resolve_player_rdns = config.get<bool>("ResolvePlayerRDNS"_jrs, true);
	RenX::Server::clientUpdateRate = std::chrono::milliseconds(config.get<long long>("ClientUpdateRate"_jrs, 2500));
	RenX::Server::buildingUpdateRate = std::chrono::milliseconds(config.get<long long>("BuildingUpdateRate"_jrs, 7500));
	RenX::Server::pingRate = std::chrono::milliseconds(config.get<long long>("PingUpdateRate"_jrs, 60000));
	RenX::Server::pingTimeoutThreshold = std::chrono::milliseconds(config.get<long long>("PingTimeoutThreshold"_jrs, 10000));

	Jupiter::Config &commandsFile = RenX::getCore()->getCommandsFile();
	RenX::Server::commandAccessLevels = commandsFile.getSection(RenX::Server::configSection);
	RenX::Server::commandAliases = commandsFile.getSection(RenX::Server::configSection + ".Aliases"_jrs);

	for (size_t i = 0; i != RenX::GameMasterCommandList->size(); ++i)
		RenX::Server::addCommand(RenX::GameMasterCommandList->get(i)->copy());

	auto load_basic_commands = [this, &commandsFile](const Jupiter::ReadableString &section_prefix)
	{
		Jupiter::Config *basic_commands = commandsFile.getSection(section_prefix + ".Basic"_jrs);
		if (basic_commands != nullptr)
		{
			Jupiter::Config *basic_commands_help = commandsFile.getSection(section_prefix + ".Basic.Help"_jrs);

			auto basic_command_no_help_callback = [this](Jupiter::HashTable::Bucket::Entry &in_entry)
			{
				if (this->getCommand(in_entry.key) == nullptr)
					this->addCommand(new RenX::BasicGameCommand(in_entry.key, in_entry.value, ""_jrs));
			};

			auto basic_command_callback = [this, basic_commands_help](Jupiter::HashTable::Bucket::Entry &in_entry)
			{
				if (this->getCommand(in_entry.key) == nullptr)
					this->addCommand(new RenX::BasicGameCommand(in_entry.key, in_entry.value, basic_commands_help->get(in_entry.value, ""_jrs)));
			};

			if (basic_commands_help == nullptr)
				basic_commands->getTable().callback(basic_command_no_help_callback);
			else
				basic_commands->getTable().callback(basic_command_callback);
		}
	};

	load_basic_commands(RenX::Server::configSection);
	load_basic_commands("Default"_jrs);
}

RenX::Server::~Server()
{
	if (RenX::GameCommand::selected_server == nullptr)
		RenX::GameCommand::selected_server = nullptr;
	if (RenX::GameCommand::active_server == nullptr)
		RenX::GameCommand::active_server = RenX::GameCommand::selected_server;

	sock.close();
	RenX::Server::wipeData();
	RenX::Server::commands.emptyAndDelete();
}
