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

#include <ctime>
#include "jessilib/split.hpp"
#include "Jupiter/String.hpp"
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

// This could probably be templated but I really just want this done quickly right this moment so I can purge ArrayList
#define CALL_RX_PLUGIN_EVENT(event, ...) \
	{ for (auto plugin : RenX::getCore()->getPlugins()) { plugin->event(__VA_ARGS__); } }

int RenX::Server::think() {
	if (m_connected == false) {
		// Not connected; attempt retry if needed
		if (m_maxAttempts < 0 || m_attempts < m_maxAttempts) {
			if (std::chrono::steady_clock::now() >= m_lastAttempt + m_delay) {
				if (connect()) {
					sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to Renegade-X server.");
				}
				else {
					sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to Renegade-X server.");
				}
			}
		}
		else {
			return 1;
		}
	}
	else if (m_awaitingPong && std::chrono::steady_clock::now() - m_lastActivity >= m_pingTimeoutThreshold) { // ping timeout
		// Ping timeout; disconnect immediately
		sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from Renegade-X server (ping timeout)."));
		disconnect(RenX::DisconnectReason::PingTimeout);
	}
	else {
		auto cycle_player_rdns = [this]() { // Cycles through any pending RDNS resolutions, and fires events as necessary.
			if (m_player_rdns_resolutions_pending != 0) {
				for (auto node = this->players.begin(); node != this->players.end(); ++node) {
					if (node->rdns_thread.joinable() && node->rdns_mutex.try_lock()) { // RDNS event hasn't fired AND RDNS value has been resolved
						node->rdns_mutex.unlock();
						node->rdns_thread.join();
						--m_player_rdns_resolutions_pending;

						// Check for bans
						banCheck(*node);

						// Fire RDNS resolved event
						for (const auto& plugin : RenX::getCore()->getPlugins()) {
							plugin->RenX_OnPlayerRDNS(*this, *node);
						}

						// Fire player indentified event if ready
						if (node->hwid.isNotEmpty()) {
							for (auto plugin : RenX::getCore()->getPlugins()) {
								plugin->RenX_OnPlayerIdentify(*this, *node);
							}
						}

						if (m_player_rdns_resolutions_pending == 0) { // No more resolutions pending
							return;
						}
					}
				}
			}
		};

		// Connected and fine
		if (m_sock.recv() > 0) { // Data received
			cycle_player_rdns();

			auto tokens = jessilib::split_view(m_sock.getBuffer(), '\n');
			if (!tokens.empty()) {
				m_lastActivity = std::chrono::steady_clock::now();
				m_lastLine += tokens[0];
				if (tokens.size() != 1) {
					processLine(Jupiter::ReferenceString{m_lastLine});
					m_lastLine = tokens[tokens.size() - 1];

					for (size_t index = 1; index != tokens.size() - 1; ++index) {
						processLine(Jupiter::ReferenceString{tokens[index]});
					}
				}
			}
		}
		else if (Jupiter::Socket::getLastError() == JUPITER_SOCK_EWOULDBLOCK) { // Operation would block (no new data)
			cycle_player_rdns();

			if (m_awaitingPong == false && std::chrono::steady_clock::now() - m_lastActivity >= m_pingRate) {
				startPing();
			}
		}
		else { // This is a serious error
			wipeData();
			if (m_maxAttempts != 0) {
				sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt in progress.");
				if (reconnect(RenX::DisconnectReason::SocketError)) {
					sendLogChan(IRCCOLOR "06[Progress]" IRCCOLOR " Connection to Renegade-X server reestablished. Initializing Renegade-X RCON protocol...");
				}
				else {
					sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. Reconnection attempt failed.");
				}
			}
			else {
				sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to Renegade-X server lost. No attempt will be made to reconnect.");
				return 1;
			}
			return 0;
		}

		// Updating client and building lists, if there is a game in progress and it's time for an update
		if (m_rconVersion >= 3 && this->players.size() != 0) {
			if (m_clientUpdateRate != std::chrono::milliseconds::zero() && std::chrono::steady_clock::now() > m_lastClientListUpdate + m_clientUpdateRate) {
				updateClientList();
			}

			if (m_buildingUpdateRate != std::chrono::milliseconds::zero() && std::chrono::steady_clock::now() > m_lastBuildingListUpdate + m_buildingUpdateRate) {
				updateBuildingList();
			}
		}

		// Trigger gameover, if one is pending
		if (m_gameover_pending && m_gameover_time < std::chrono::steady_clock::now()) {
			gameover();
			m_gameover_pending = false;
		}

		// Trigger ping if we haven't sent anything over the socket in a while, to prevent the server from disconnecting
		if (m_awaitingPong == false && std::chrono::steady_clock::now() - m_lastSendActivity >= m_pingRate) {
			startPing();
		}
	}

	return 0;
}

int RenX::Server::OnRehash() {
	std::string oldHostname = m_hostname;
	std::string oldClientHostname = m_clientHostname;
	Jupiter::StringS oldPass = m_pass;
	unsigned short oldPort = m_port;
	int oldSteamFormat = m_steamFormat;
	m_commands.clear();
	init(*RenX::getCore()->getConfig().getSection(m_configSection));
	if (m_port == 0 || !m_hostname.empty()) {
		m_hostname = oldHostname;
		m_clientHostname = oldClientHostname;
		m_pass = oldPass;
		m_port = oldPort;
	}
	else if (Jupiter::ReferenceString(oldHostname.c_str()).equalsi(m_hostname) == false
		|| oldPort != m_port
		|| Jupiter::ReferenceString(oldClientHostname.c_str()).equalsi(m_clientHostname) == false
		|| oldPass.equalsi(m_pass) == false) {
		reconnect(RenX::DisconnectReason::Rehash);
	}

	return 0;
}

bool RenX::Server::OnBadRehash(bool removed) {
	return removed;
}

bool RenX::Server::isConnected() const {
	return m_connected;
}

bool RenX::Server::isSubscribed() const {
	return m_subscribed;
}

bool RenX::Server::isFullyConnected() const {
	return m_fully_connected;
}

bool RenX::Server::isFirstKill() const {
	return m_firstKill;
}

bool RenX::Server::isFirstDeath() const {
	return m_firstDeath;
}

bool RenX::Server::isFirstAction() const {
	return m_firstAction;
}

bool RenX::Server::isSeamless() const {
	return m_seamless;
}

bool RenX::Server::isReliable() const {
	return m_reliable;
}

bool RenX::Server::isMatchPending() const {
	return m_match_state == 0 || isTravelling();
}

bool RenX::Server::isMatchInProgress() const {
	return m_match_state == 1;
}

bool RenX::Server::isMatchOver() const {
	return m_match_state == 2 || isMatchOver();
}

bool RenX::Server::isTravelling() const {
	return m_match_state == 3;
}

bool RenX::Server::isCompetitive() const {
	return m_competitive;
}

bool RenX::Server::isPublicLogChanType(int type) const {
	return m_logChanType == type;
}

bool RenX::Server::isAdminLogChanType(int type) const {
	return m_adminLogChanType == type;
}

bool RenX::Server::isLogChanType(int type) const {
	return isPublicLogChanType(type) || isAdminLogChanType(type);
}

bool RenX::Server::isPure() const {
	return m_pure;
}

int RenX::Server::send(const Jupiter::ReadableString &command) {
	return sendSocket("c"_jrs + RenX::escapifyRCON(command) + '\n');
}

int RenX::Server::sendSocket(const Jupiter::ReadableString &text) {
	m_lastSendActivity = std::chrono::steady_clock::now();
	return m_sock.send(text);
}

int RenX::Server::sendMessage(const Jupiter::ReadableString &message) {
	Jupiter::String msg = RenX::escapifyRCON(message);
	if (m_neverSay) {
		int result = 0;
		if (this->players.size() != 0) {
			for (auto node = this->players.begin(); node != this->players.end(); ++node) {
				if (node->isBot == false) {
					result += sendSocket(Jupiter::StringS::Format("chostprivatesay pid%d %.*s\n", node->id, msg.size(), msg.ptr()));
				}
			}
		}
		return result;
	}

	return sendSocket("chostsay "_jrs + msg + '\n');
}

int RenX::Server::sendMessage(const RenX::PlayerInfo &player, const Jupiter::ReadableString &message) {
	return sendSocket("chostprivatesay pid"_jrs + Jupiter::StringS::Format("%d ", player.id) + RenX::escapifyRCON(message) + '\n');
}

int RenX::Server::sendAdminMessage(const Jupiter::ReadableString &message) {
	return sendSocket("camsg "_jrs + RenX::escapifyRCON(message) + '\n');
}

int RenX::Server::sendAdminMessage(const RenX::PlayerInfo &player, const Jupiter::ReadableString &message) {
	return sendSocket("cpamsg pid"_jrs + Jupiter::StringS::Format("%d ", player.id) + RenX::escapifyRCON(message) + '\n');
}

int RenX::Server::sendWarnMessage(const RenX::PlayerInfo &player, const Jupiter::ReadableString &message) {
	return sendSocket("cwarn pid"_jrs + Jupiter::StringS::Format("%d ", player.id) + RenX::escapifyRCON(message) + '\n');
}

int RenX::Server::sendData(const Jupiter::ReadableString &data) {
	return sendSocket(data);
}

RenX::BuildingInfo *RenX::Server::getBuildingByName(const Jupiter::ReadableString &name) const {
	for (size_t index = 0; index != RenX::Server::buildings.size(); ++index) {
		if (RenX::Server::buildings[index]->name.equalsi(name)) {
			return RenX::Server::buildings[index].get();
		}
	}

	return nullptr;
}

bool RenX::Server::hasMapInRotation(const Jupiter::ReadableString &name) const {
	size_t index = RenX::Server::maps.size();
	while (index != 0) {
		if (RenX::Server::maps[--index].name.equalsi(name)) {
			return true;
		}
	}

	return false;
}

const Jupiter::ReadableString *RenX::Server::getMapName(const Jupiter::ReadableString &name) const {
	size_t index = RenX::Server::maps.size();
	const Jupiter::ReadableString *map_name;
	while (index != 0) {
		map_name = &RenX::Server::maps[--index].name;
		if (map_name->findi(name) != Jupiter::INVALID_INDEX)
			return map_name;
	}
	return nullptr;
}

const Jupiter::ReadableString &RenX::Server::getCurrentRCONCommand() const {
	return m_lastCommand;
}

const Jupiter::ReadableString &RenX::Server::getCurrentRCONCommandParameters() const {
	return m_lastCommandParams;
}

std::chrono::milliseconds RenX::Server::getGameTime() const {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_gameStart);
}

std::chrono::milliseconds RenX::Server::getGameTime(const RenX::PlayerInfo &player) const {
	if (player.joinTime < m_gameStart)
		return getGameTime();

	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player.joinTime);
}

size_t RenX::Server::getBotCount() const {
	size_t count = 0;

	for (auto node = this->players.begin(); node != this->players.end(); ++node) {
		if (node->isBot) {
			++count;
		}
	}

	return count;
	//return RenX::Server::bot_count;
}

std::vector<const RenX::PlayerInfo*> RenX::Server::activePlayers(bool includeBots) const {
	std::vector<const RenX::PlayerInfo*> result;
	result.reserve(players.size());

	// Build list of active players
	for (auto& player : players) {
		// Filter teamless players and bots (if applicable)
		if (player.team != TeamType::None && (includeBots || !player.isBot)) {
			result.push_back(&player);
		}
	}

	return result;
}

RenX::PlayerInfo *RenX::Server::getPlayer(int id) const {
	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		if (node->id == id)
			return const_cast<RenX::PlayerInfo *>(&*node);

	return nullptr;
}

RenX::PlayerInfo *RenX::Server::getPlayerByName(const Jupiter::ReadableString &name) const {
	if (this->players.size() == 0) {
		return nullptr;
	}

	// Try full name match
	for (auto node = this->players.begin(); node != this->players.end(); ++node) {
		if (node->name == name) {
			return const_cast<RenX::PlayerInfo*>(&*node);
		}
	}

	// Try player ID
	Jupiter::ReferenceString idToken = name;
	if (name.matchi("Player?*")) {
		idToken.shiftRight(6);
	}
	else if (name.matchi("pid?*")) {
		idToken.shiftRight(3);
	}
	else {
		return nullptr;
	}

	int id = idToken.asInt(10);
	return getPlayer(id);
}

RenX::PlayerInfo *RenX::Server::getPlayerByPartName(const Jupiter::ReadableString &partName) const {
	if (this->players.size() == 0)
		return nullptr;

	RenX::PlayerInfo *r = getPlayerByName(partName);
	if (r != nullptr)
		return r;

	return getPlayerByPartNameFast(partName);
}

RenX::PlayerInfo *RenX::Server::getPlayerByPartNameFast(const Jupiter::ReadableString &partName) const {
	if (this->players.size() == 0)
		return nullptr;

	for (auto node = this->players.begin(); node != this->players.end(); ++node)
		if (node->name.findi(partName) != Jupiter::INVALID_INDEX)
			return const_cast<RenX::PlayerInfo *>(&*node);

	return nullptr;
}

Jupiter::StringS RenX::Server::formatSteamID(const RenX::PlayerInfo &player) const {
	return formatSteamID(player.steamid);
}

Jupiter::StringS RenX::Server::formatSteamID(uint64_t id) const {
	if (id == 0)
		return Jupiter::ReferenceString::empty;

	switch (m_steamFormat)
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

void RenX::Server::kickPlayer(int id, const Jupiter::ReadableString &in_reason) {
	Jupiter::String reason = RenX::escapifyRCON(in_reason);

	if (reason.isEmpty())
		sendSocket(Jupiter::StringS::Format("ckick pid%d\n", id));
	else
		sendSocket(Jupiter::StringS::Format("ckick pid%d %.*s\n", id, reason.size(), reason.ptr()));
}

void RenX::Server::kickPlayer(const RenX::PlayerInfo &player, const Jupiter::ReadableString &reason) {
	if ((player.exemption_flags & RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK) == 0) {
		kickPlayer(player.id, reason);
	}
}

void RenX::Server::forceKickPlayer(int id, const Jupiter::ReadableString &in_reason) {
	Jupiter::String reason = RenX::escapifyRCON(in_reason);

	if (reason.isEmpty()) {
		sendSocket(Jupiter::StringS::Format("cfkick pid%d You were kicked from the server.\n", id));
		return;
	}

	sendSocket(Jupiter::StringS::Format("cfkick pid%d %.*s\n", id, reason.size(), reason.ptr()));
}

void RenX::Server::forceKickPlayer(const RenX::PlayerInfo &player, const Jupiter::ReadableString &reason) {
	if ((player.exemption_flags & RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK) == 0) {
		forceKickPlayer(player.id, reason);
	}
}

void RenX::Server::banCheck() {
	if (this->players.size() != 0) {
		for (auto node = this->players.begin(); node != this->players.end(); ++node) {
			if (node->isBot == false) {
				banCheck(*node);
			}
		}
	}
}

void RenX::Server::banCheck(RenX::PlayerInfo &player) {
	if ((player.exemption_flags & (RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN | RenX::ExemptionDatabase::Entry::FLAG_TYPE_KICK)) != 0) {
		return;
	}

	uint32_t netmask;

	RenX::BanDatabase::Entry* last_to_expire[7]; // TODO: what the fuck is this?
	for (size_t index = 0; index != sizeof(last_to_expire) / sizeof(RenX::BanDatabase::Entry *); ++index)
		last_to_expire[index] = nullptr;

	auto handle_type = [&last_to_expire](RenX::BanDatabase::Entry* entry, size_t index) {
		if (last_to_expire[index] == nullptr) {
			last_to_expire[index] = entry;
		}
		else if (last_to_expire[index]->length == std::chrono::seconds::zero()) {
			// favor older bans if they're also permanent
			if (entry->length == std::chrono::seconds::zero() && entry->timestamp < last_to_expire[index]->timestamp) {
				last_to_expire[index] = entry;
			}
		}
		else if (entry->length == std::chrono::seconds::zero() || entry->timestamp + entry->length > last_to_expire[index]->timestamp + last_to_expire[index]->length) {
			last_to_expire[index] = entry;
		}
	};

	for (const auto& entry : RenX::banDatabase->getEntries()) {
		if (entry->is_active()) {
			if (entry->length != std::chrono::seconds::zero() && entry->timestamp + entry->length < std::chrono::system_clock::now()) {
				banDatabase->deactivate(entry.get());
			}
			else {
				if (entry->prefix_length >= 32) {
					netmask = 0xFFFFFFFF;
				}
				else {
					netmask = Jupiter_prefix_length_to_netmask(entry->prefix_length);
				}

				if ((m_localSteamBan && entry->steamid != 0 && entry->steamid == player.steamid)
					|| (m_localIPBan && entry->ip != 0 && (entry->ip & netmask) == (player.ip32 & netmask))
					|| (m_localHWIDBan && entry->hwid.isNotEmpty() && entry->hwid.equals(player.hwid))
					|| (m_localRDNSBan && entry->rdns.isNotEmpty() && entry->is_rdns_ban() && player.rdns_thread.joinable() == false && player.rdns.match(entry->rdns))
					|| (m_localNameBan && entry->name.isNotEmpty() && entry->name.equalsi(player.name))) {
					player.ban_flags |= entry->flags;
					if (entry->is_type_game())
						handle_type(entry.get(), 0);
					if (entry->is_type_chat())
						handle_type(entry.get(), 1);
					if (entry->is_type_bot())
						handle_type(entry.get(), 2);
					if (entry->is_type_vote())
						handle_type(entry.get(), 3);
					if (entry->is_type_mine())
						handle_type(entry.get(), 4);
					if (entry->is_type_ladder())
						handle_type(entry.get(), 5);
					if (entry->is_type_alert())
						handle_type(entry.get(), 6);
				}
			}
		}
	}

	char timeStr[256];
	if (last_to_expire[0] != nullptr) { // Game ban
		time_t current_time = std::chrono::system_clock::to_time_t(last_to_expire[0]->timestamp + last_to_expire[0]->length);
		strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(&current_time));
		if (last_to_expire[0]->length == std::chrono::seconds::zero()) {
			forceKickPlayer(player, Jupiter::StringS::Format("You were permanently banned from %.*s on %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[0]->reason.size(), last_to_expire[0]->reason.ptr()));
		}
		else {
			forceKickPlayer(player, Jupiter::StringS::Format("You are banned from %.*s until %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[0]->reason.size(), last_to_expire[0]->reason.ptr()));
		}

		player.ban_flags |= RenX::BanDatabase::Entry::FLAG_TYPE_BOT; // implies FLAG_TYPE_BOT
	}
	else {
		if (last_to_expire[1] != nullptr) { // Chat ban
			time_t current_time = std::chrono::system_clock::to_time_t(last_to_expire[1]->timestamp + last_to_expire[1]->length);
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(&current_time));
			mute(player);
			if (last_to_expire[1]->length == std::chrono::seconds::zero())
				sendMessage(player, Jupiter::StringS::Format("You were permanently muted from %.*s on %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[1]->reason.size(), last_to_expire[1]->reason.ptr()));
			else
				sendMessage(player, Jupiter::StringS::Format("You are muted from %.*s until %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[1]->reason.size(), last_to_expire[1]->reason.ptr()));

			player.ban_flags |= RenX::BanDatabase::Entry::FLAG_TYPE_BOT; // implies FLAG_TYPE_BOT
		}
		else if (last_to_expire[2] != nullptr) { // Bot ban
			time_t current_time = std::chrono::system_clock::to_time_t(last_to_expire[2]->timestamp + last_to_expire[2]->length);
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(&current_time));
			if (last_to_expire[2]->length == std::chrono::seconds::zero())
				sendMessage(player, Jupiter::StringS::Format("You were permanently bot-muted from %.*s on %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[2]->reason.size(), last_to_expire[2]->reason.ptr()));
			else
				sendMessage(player, Jupiter::StringS::Format("You are bot-muted from %.*s until %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[2]->reason.size(), last_to_expire[2]->reason.ptr()));
		}
		if (last_to_expire[3] != nullptr) { // Vote ban
			time_t current_time = std::chrono::system_clock::to_time_t(last_to_expire[3]->timestamp + last_to_expire[3]->length);
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(&current_time));
			if (last_to_expire[3]->length == std::chrono::seconds::zero())
				sendMessage(player, Jupiter::StringS::Format("You were permanently vote-muted from %.*s on %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[3]->reason.size(), last_to_expire[3]->reason.ptr()));
			else
				sendMessage(player, Jupiter::StringS::Format("You are vote-muted from %.*s until %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[3]->reason.size(), last_to_expire[3]->reason.ptr()));
		}
		if (last_to_expire[4] != nullptr) { // Mine ban
			time_t current_time = std::chrono::system_clock::to_time_t(last_to_expire[4]->timestamp + last_to_expire[4]->length);
			mineBan(player);
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(&current_time));
			if (last_to_expire[4]->length == std::chrono::seconds::zero())
				sendMessage(player, Jupiter::StringS::Format("You were permanently mine-banned from %.*s on %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[4]->reason.size(), last_to_expire[4]->reason.ptr()));
			else
				sendMessage(player, Jupiter::StringS::Format("You are mine-banned from %.*s until %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[4]->reason.size(), last_to_expire[4]->reason.ptr()));
		}
		if (last_to_expire[5] != nullptr) { // Ladder ban
			time_t current_time = std::chrono::system_clock::to_time_t(last_to_expire[5]->timestamp + last_to_expire[5]->length);
			strftime(timeStr, sizeof(timeStr), "%b %d %Y at %H:%M:%S", localtime(&current_time));
			if (last_to_expire[5]->length == std::chrono::seconds::zero())
				sendMessage(player, Jupiter::StringS::Format("You were permanently ladder-banned from %.*s on %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[5]->reason.size(), last_to_expire[5]->reason.ptr()));
			else
				sendMessage(player, Jupiter::StringS::Format("You are ladder-banned from %.*s until %s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), timeStr, last_to_expire[5]->reason.size(), last_to_expire[5]->reason.ptr()));
		}
		if (last_to_expire[6] != nullptr) { // Alert
			IRC_Bot *server;
			Jupiter::String fmtName = RenX::getFormattedPlayerName(player);
			Jupiter::StringL user_message = Jupiter::StringL::Format(IRCCOLOR "04[Alert] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is marked for monitoring by %.*s for: \"%.*s\". Please keep an eye on them in ", fmtName.size(), fmtName.ptr(), last_to_expire[6]->banner.size(), last_to_expire[6]->banner.ptr(), last_to_expire[6]->reason.size(), last_to_expire[6]->reason.ptr());
			Jupiter::StringS channel_message = Jupiter::StringS::Format(IRCCOLOR "04[Alert] " IRCCOLOR IRCBOLD "%.*s" IRCBOLD IRCCOLOR " is marked for monitoring by %.*s for: \"%.*s\"." IRCCOLOR, fmtName.size(), fmtName.ptr(), last_to_expire[6]->banner.size(), last_to_expire[6]->banner.ptr(), last_to_expire[6]->reason.size(), last_to_expire[6]->reason.ptr());

			for (size_t server_index = 0; server_index < serverManager->size(); ++server_index) {
				server = serverManager->getServer(server_index);
				for (auto& channel : server->getChannels()) {
					if (isAdminLogChanType(channel.second.getType())) {
						server->sendMessage(channel.second.getName(), channel_message);

						user_message += channel.second.getName();
						for (auto& user : channel.second.getUsers()) {
							if (channel.second.getUserPrefix(*user.second) != 0 && user.second->getNickname().equals(server->getNickname()) == false)
								server->sendMessage(user.second->getUser()->getNickname(), user_message);
						}
						user_message -= channel.second.getName().size();
					}
				}
			}
		}
	}
};

void RenX::Server::banPlayer(int id, const Jupiter::ReadableString &banner, const Jupiter::ReadableString &reason) {
	if (m_rconBan) {
		Jupiter::String out_reason = RenX::escapifyRCON(reason);
		sendSocket(Jupiter::StringS::Format("ckickban pid%d %.*s\n", id, out_reason.size(), out_reason.ptr()));
	}
	else {
		RenX::PlayerInfo *player = getPlayer(id);
		if (player != nullptr) {
			banPlayer(*player, banner, reason);
		}
	}
}

void RenX::Server::banPlayer(const RenX::PlayerInfo &player, const Jupiter::ReadableString &banner, const Jupiter::ReadableString &reason, std::chrono::seconds length) {
	if ((player.exemption_flags & RenX::ExemptionDatabase::Entry::FLAG_TYPE_BAN) == 0) {
		if (m_localBan) {
			RenX::banDatabase->add(this, player, banner, reason, length);
		}

		if (length == std::chrono::seconds::zero()) {
			if (m_rconBan) {
				Jupiter::String out_reason = RenX::escapifyRCON(reason);
				sendSocket(Jupiter::StringS::Format("ckickban pid%d %.*s\n", player.id, out_reason.size(), out_reason.ptr()));
			}
			else if (banner.isNotEmpty()) {
				forceKickPlayer(player, Jupiter::StringS::Format("You are permanently banned from %.*s by %.*s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), banner.size(), banner.ptr(), reason.size(), reason.ptr()));
			}
			else {
				forceKickPlayer(player, Jupiter::StringS::Format("You are permanently banned from %.*s for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), reason.size(), reason.ptr()));
			}
		}
		else if (banner.isNotEmpty()) {
			// TODO: make the time formatting not complete nonsense
			forceKickPlayer(player, Jupiter::StringS::Format("You are banned from %.*s by %.*s for the next %lld days, %.2d:%.2d:%.2d for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), banner.size(), banner.ptr(), static_cast<long long>(length.count() / 86400), static_cast<int>(length.count() % 3600), static_cast<int>((length.count() % 3600) / 60), static_cast<int>(length.count() % 60), reason.size(), reason.ptr()));
		}
		else {
			forceKickPlayer(player, Jupiter::StringS::Format("You are banned from %.*s for the next %lld days, %.2d:%.2d:%.2d for: %.*s", m_ban_from_str.size(), m_ban_from_str.ptr(), static_cast<long long>(length.count() / 86400), static_cast<int>(length.count() % 3600), static_cast<int>((length.count() % 3600) / 60), static_cast<int>(length.count() % 60), reason.size(), reason.ptr()));
		}
	}
}

bool RenX::Server::removePlayer(int id) {
	if (this->players.size() == 0) {
		return false;
	}

	for (auto node = this->players.begin(); node != this->players.end(); ++node) {
		if (node->id == id) {
			for (auto plugin : RenX::getCore()->getPlugins()) {
				plugin->RenX_OnPlayerDelete(*this, *node);
			}

			if (node->isBot) {
				--m_bot_count;
			}

			if (node->rdns_thread.joinable()) { // Close the RDNS thread, if one exists
				--m_player_rdns_resolutions_pending;
				node->rdns_thread.join();
			}

			this->players.erase(node);
			return true;
		}
	}

	return false;
}

bool RenX::Server::removePlayer(RenX::PlayerInfo &player) {
	return removePlayer(player.id);
}

bool RenX::Server::fetchClientList() {
	m_lastClientListUpdate = std::chrono::steady_clock::now();
	// TODO: remove all support for rconVersion < 4
	if (m_rconVersion >= 4) {
		return sendSocket("cclientvarlist KILLS DEATHS SCORE CREDITS CHARACTER VEHICLE PING ADMIN STEAM IP HWID PLAYERLOG\n"_jrs) > 0
			&& sendSocket("cbotvarlist KILLS DEATHS SCORE CREDITS CHARACTER VEHICLE PLAYERLOG\n"_jrs) > 0;
	}

	return sendSocket("cclientvarlist KILLS\xA0""DEATHS\xA0""SCORE\xA0""CREDITS\xA0""CHARACTER\xA0""VEHICLE\xA0""PING\xA0""ADMIN\xA0""STEAM\xA0""IP\xA0""PLAYERLOG\n"_jrs) > 0
		&& sendSocket("cbotvarlist KILLS\xA0""DEATHS\xA0""SCORE\xA0""CREDITS\xA0""CHARACTER\xA0""VEHICLE\xA0""PLAYERLOG\n"_jrs) > 0;
}

bool RenX::Server::updateClientList() {
	m_lastClientListUpdate = std::chrono::steady_clock::now();

	int result = 0;
	if (this->players.size() != getBotCount()) {
		if (m_rconVersion >= 4)
			result = sendSocket("cclientvarlist ID SCORE CREDITS PING\n"_jrs) > 0;
		else
			result = sendSocket("cclientvarlist ID\xA0""SCORE\xA0""CREDITS\xA0""PING\n"_jrs) > 0;
	}

	if (getBotCount() != 0) {
		if (m_rconVersion >= 4)
			result |= sendSocket("cbotvarlist ID SCORE CREDITS\n"_jrs) > 0;
		else
			result |= sendSocket("cbotvarlist ID\xA0""SCORE\xA0""CREDITS\n"_jrs) > 0;
	}

	return result != 0;
}

bool RenX::Server::updateBuildingList() {
	m_lastBuildingListUpdate = std::chrono::steady_clock::now();
	return sendSocket("cbinfo\n"_jrs) > 0;
}

bool RenX::Server::gameover() {
	m_gameover_when_empty = false;
	return send("endmap"_jrs) > 0;
}

void RenX::Server::gameover(std::chrono::seconds delay) {
	if (delay == std::chrono::seconds::zero()) {
		gameover();
		return;
	}

	m_gameover_time = std::chrono::steady_clock::now() + delay;
	m_gameover_pending = true;
}

bool RenX::Server::gameoverStop() {
	if (m_gameover_pending == false) {
		return false;
	}

	m_gameover_pending = false;
	return true;
}

void RenX::Server::gameoverWhenEmpty() {
	if (this->players.size() == this->getBotCount()) {
		gameover();
		return;
	}

	m_gameover_when_empty = true;
}

bool RenX::Server::setMap(const Jupiter::ReadableString &map) {
	return send(Jupiter::StringS::Format("changemap %.*s", map.size(), map.ptr())) > 0;
}

bool RenX::Server::loadMutator(const Jupiter::ReadableString &mutator) {
	return send(Jupiter::StringS::Format("loadmutator %.*s", mutator.size(), mutator.ptr())) > 0;
}

bool RenX::Server::unloadMutator(const Jupiter::ReadableString &mutator) {
	return send(Jupiter::StringS::Format("unloadmutator %.*s", mutator.size(), mutator.ptr())) > 0;
}

bool RenX::Server::cancelVote(const RenX::TeamType team) {
	switch (team) {
	default:
		return send("cancelvote -1"_jrs) > 0;
	case TeamType::GDI:
		return send("cancelvote 0"_jrs) > 0;
	case TeamType::Nod:
		return send("cancelvote 1"_jrs) > 0;
	}
}

bool RenX::Server::swapTeams() {
	return send("swapteams"_jrs) > 0;
}

bool RenX::Server::recordDemo() {
	return send("recorddemo"_jrs) > 0;
}

bool RenX::Server::mute(const RenX::PlayerInfo &player) {
	return send(Jupiter::StringS::Format("textmute pid%u", player.id)) > 0;
}

bool RenX::Server::unmute(const RenX::PlayerInfo &player) {
	return send(Jupiter::StringS::Format("textunmute pid%u", player.id)) > 0;
}

bool RenX::Server::giveCredits(int id, double credits) {
	return m_competitive == false && send(Jupiter::StringS::Format("givecredits pid%d %f", id, credits)) > 0;
}

bool RenX::Server::giveCredits(RenX::PlayerInfo &player, double credits) {
	return giveCredits(player.id, credits);
}

bool RenX::Server::kill(int id) {
	return m_competitive == false && send(Jupiter::StringS::Format("kill pid%d", id)) > 0;
}

bool RenX::Server::kill(RenX::PlayerInfo &player) {
	return kill(player.id);
}

bool RenX::Server::disarm(int id) {
	return m_competitive == false && send(Jupiter::StringS::Format("disarm pid%d", id)) > 0;
}

bool RenX::Server::disarm(RenX::PlayerInfo &player) {
	return disarm(player.id);
}

bool RenX::Server::disarmC4(int id) {
	return m_competitive == false && send(Jupiter::StringS::Format("disarmc4 pid%d", id)) > 0;
}

bool RenX::Server::disarmC4(RenX::PlayerInfo &player) {
	return disarmC4(player.id);
}

bool RenX::Server::disarmBeacon(int id) {
	return m_competitive == false && send(Jupiter::StringS::Format("disarmb pid%d", id)) > 0;
}

bool RenX::Server::disarmBeacon(RenX::PlayerInfo &player) {
	return disarmBeacon(player.id);
}

bool RenX::Server::mineBan(int id) {
	return send(Jupiter::StringS::Format("mineban pid%d", id)) > 0;
}

bool RenX::Server::mineBan(RenX::PlayerInfo &player) {
	return mineBan(player.id);
}

bool RenX::Server::changeTeam(int id, bool resetCredits) {
	return send(Jupiter::StringS::Format(resetCredits ? "team pid%d" : "team2 pid%d", id)) > 0;
}

bool RenX::Server::changeTeam(RenX::PlayerInfo &player, bool resetCredits) {
	return changeTeam(player.id, resetCredits);
}

bool RenX::Server::nmodePlayer(const RenX::PlayerInfo &player) {
	return send(Jupiter::StringS::Format("nmode pid%d", player.id));
}

bool RenX::Server::smodePlayer(const RenX::PlayerInfo &player) {
	return send(Jupiter::StringS::Format("smode pid%d", player.id));
}

const Jupiter::ReadableString &RenX::Server::getPrefix() const {
	static Jupiter::String parsed; // TODO: What the hell?
	RenX::processTags(parsed = m_IRCPrefix, this);
	return parsed;
}

void RenX::Server::setPrefix(const Jupiter::ReadableString &prefix) {
	RenX::sanitizeTags(m_IRCPrefix = prefix);
}

const Jupiter::ReadableString &RenX::Server::getCommandPrefix() const {
	return m_CommandPrefix;
}

void RenX::Server::setCommandPrefix(const Jupiter::ReadableString &prefix) {
	m_CommandPrefix = prefix;
}

void RenX::Server::setRanked(bool in_value) {
	m_ranked = in_value;
}

const Jupiter::ReadableString &RenX::Server::getRules() const {
	return m_rules;
}

const std::string &RenX::Server::getHostname() const {
	return m_hostname;
}

unsigned short RenX::Server::getPort() const {
	return m_port;
}

const std::string &RenX::Server::getSocketHostname() const {
	return m_sock.getRemoteHostname();
}

unsigned short RenX::Server::getSocketPort() const {
	return m_sock.getRemotePort();
}

std::chrono::steady_clock::time_point RenX::Server::getLastAttempt() const {
	return m_lastAttempt;
}

std::chrono::milliseconds RenX::Server::getDelay() const {
	return m_delay;
}

int RenX::Server::getMineLimit() const {
	return m_mineLimit;
}

int RenX::Server::getPlayerLimit() const {
	return m_playerLimit;
}

int RenX::Server::getVehicleLimit() const {
	return m_vehicleLimit;
}

int RenX::Server::getTimeLimit() const {
	return m_timeLimit;
}

int RenX::Server::getTeamMode() const {
	return m_team_mode;
}

int RenX::Server::getGameType() const {
	return m_game_type;
}

double RenX::Server::getCrateRespawnDelay() const {
	return m_crateRespawnAfterPickup;
}

bool RenX::Server::isSteamRequired() const {
	return m_steamRequired;
}

bool RenX::Server::isPrivateMessageTeamOnly() const {
	return m_privateMessageTeamOnly;
}

bool RenX::Server::isPrivateMessagingEnabled() const {
	return m_allowPrivateMessaging;
}

bool RenX::Server::isRanked() const {
	return m_ranked;
}

bool RenX::Server::isPassworded() const {
	return m_passworded;
}

bool RenX::Server::isCratesEnabled() const {
	return m_spawnCrates;
}

bool RenX::Server::isBotsEnabled() const {
	return m_botsEnabled;
}

const Jupiter::ReadableString &RenX::Server::getPassword() const {
	return m_pass;
}

const Jupiter::ReadableString &RenX::Server::getUser() const {
	return m_rconUser;
}

bool RenX::Server::isDevBot() const {
	return m_devBot;
}

const Jupiter::ReadableString &RenX::Server::getName() const {
	return m_serverName;
}

const RenX::Map &RenX::Server::getMap() const {
	return m_map;
}

RenX::GameCommand *RenX::Server::getCommand(unsigned int index) const {
	return m_commands[index].get();
}

RenX::GameCommand *RenX::Server::getCommand(const Jupiter::ReadableString &trigger) const {
	for (const auto& command : m_commands) {
		if (command->matches(trigger)) {
			return command.get();
		}
	}
	return nullptr;
}

size_t RenX::Server::getCommandCount() const {
	return m_commands.size();
}

RenX::GameCommand *RenX::Server::triggerCommand(const Jupiter::ReadableString &trigger, RenX::PlayerInfo &player, const Jupiter::ReadableString &parameters) {
	RenX::GameCommand::active_server = this;
	for (const auto& command : m_commands) {
		if (command->matches(trigger)) {
			if (player.access >= command->getAccessLevel()) {
				command->trigger(this, &player, parameters);
			}
			else {
				sendMessage(player, "Access Denied."_jrs);
			}

			// TODO: avoiding modifying behavior for now, but this probably doesn't need to be called on access denied
			for (const auto& plugin : getCore()->getPlugins()) {
				plugin->RenX_OnCommandTriggered(*this, trigger, player, parameters, *command);
			}

			return command.get();
		}
	}

	// TODO: do we need to set active_server on the return above as well?
	RenX::GameCommand::active_server = RenX::GameCommand::selected_server;
	return nullptr;
}

void RenX::Server::addCommand(RenX::GameCommand* in_command) {
	std::unique_ptr<RenX::GameCommand> command{ in_command };
	int access_level;

	if (m_commandAccessLevels != nullptr) {
		const Jupiter::ReadableString &accessLevel = m_commandAccessLevels->get(command->getTrigger());
		if (accessLevel.isNotEmpty()) {
			access_level = accessLevel.asInt();
			if (access_level < 0) { // Disabled command
				return;
			}

			command->setAccessLevel(access_level);
		}
	}

	if (m_commandAliases != nullptr) {
		const Jupiter::ReadableString &aliasList = m_commandAliases->get(command->getTrigger());
		unsigned int j = aliasList.wordCount(WHITESPACE);
		while (j != 0) {
			command->addTrigger(Jupiter::ReferenceString::getWord(aliasList, --j, WHITESPACE));
		}
	}

	m_commands.push_back(std::move(command));
}

bool RenX::Server::removeCommand(RenX::GameCommand *command) {
	for (auto itr = m_commands.begin(); itr != m_commands.end(); ++itr) {
		if (itr->get() == command) {
			m_commands.erase(itr);
			return true;
		}
	}

	return false;
}

bool RenX::Server::removeCommand(const Jupiter::ReadableString &trigger) {
	for (auto itr = m_commands.begin(); itr != m_commands.end(); ++itr) {
		if ((*itr)->matches(trigger)) {
			m_commands.erase(itr);
			return true;
		}
	}

	return false;
}

void RenX::Server::setUUIDFunction(RenX::Server::uuid_func func) {
	m_calc_uuid = func;

	for (auto node = this->players.begin(); node != this->players.end(); ++node) {
		setUUIDIfDifferent(*node, m_calc_uuid(*node));
	}
}

RenX::Server::uuid_func RenX::Server::getUUIDFunction() const {
	return m_calc_uuid;
}

void RenX::Server::setUUID(RenX::PlayerInfo &player, const Jupiter::ReadableString &uuid) {
	for (const auto& plugin : getCore()->getPlugins()) {
		plugin->RenX_OnPlayerUUIDChange(*this, player, uuid);
	}

	player.uuid = uuid;
}

bool RenX::Server::setUUIDIfDifferent(RenX::PlayerInfo &player, const Jupiter::ReadableString &uuid) {
	if (player.uuid.equals(uuid)) {
		return false;
	}

	setUUID(player, uuid);
	return true;
}

bool RenX::Server::resolvesRDNS() {
	return m_resolve_player_rdns;
}

void RenX::Server::sendPubChan(const char *fmt, ...) const {
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = getPrefix();
	if (serverPrefix.isNotEmpty()) {
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else {
		msg.vformat(fmt, args);
	}
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++) {
		serverManager->getServer(i)->messageChannels(m_logChanType, msg);
	}
}

void RenX::Server::sendPubChan(const Jupiter::ReadableString &msg) const {
	const Jupiter::ReadableString &prefix = getPrefix();
	if (prefix.isNotEmpty()) {
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++) {
			serverManager->getServer(i)->messageChannels(m_logChanType, m);
		}

		return;
	}

	for (size_t i = 0; i != serverManager->size(); i++) {
		serverManager->getServer(i)->messageChannels(m_logChanType, msg);
	}
}

void RenX::Server::sendAdmChan(const char *fmt, ...) const {
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = getPrefix();
	if (serverPrefix.isNotEmpty()) {
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else {
		msg.vformat(fmt, args);
	}
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++) {
		serverManager->getServer(i)->messageChannels(m_adminLogChanType, msg);
	}
}

void RenX::Server::sendAdmChan(const Jupiter::ReadableString &msg) const {
	const Jupiter::ReadableString &prefix = getPrefix();
	if (prefix.isNotEmpty()) {
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++) {
			serverManager->getServer(i)->messageChannels(m_adminLogChanType, m);
		}

		return;
	}

	for (size_t i = 0; i != serverManager->size(); i++) {
		serverManager->getServer(i)->messageChannels(m_adminLogChanType, msg);
	}
}

void RenX::Server::sendLogChan(const char *fmt, ...) const {
	IRC_Bot *server;
	va_list args;
	va_start(args, fmt);
	Jupiter::StringL msg;
	const Jupiter::ReadableString &serverPrefix = getPrefix();
	if (serverPrefix.isNotEmpty()) {
		msg += serverPrefix;
		msg += ' ';
		msg.avformat(fmt, args);
	}
	else {
		msg.vformat(fmt, args);
	}
	va_end(args);
	for (size_t i = 0; i != serverManager->size(); i++) {
		server = serverManager->getServer(i);
		server->messageChannels(m_logChanType, msg);
		server->messageChannels(m_adminLogChanType, msg);
	}
}

void RenX::Server::sendLogChan(const Jupiter::ReadableString &msg) const {
	IRC_Bot *server;
	const Jupiter::ReadableString &prefix = getPrefix();
	if (prefix.isNotEmpty()) {
		Jupiter::String m(msg.size() + prefix.size() + 1);
		m.set(prefix);
		m += ' ';
		m += msg;
		for (size_t i = 0; i != serverManager->size(); i++)
		{
			server = serverManager->getServer(i);
			server->messageChannels(m_logChanType, m);
			server->messageChannels(m_adminLogChanType, m);
		}

		return;
	}

	for (size_t i = 0; i != serverManager->size(); i++) {
		server = serverManager->getServer(i);
		server->messageChannels(m_logChanType, msg);
		server->messageChannels(m_adminLogChanType, msg);
	}
}

void resolve_rdns(RenX::PlayerInfo *player) {
	player->rdns_mutex.lock();
	char *resolved = Jupiter::Socket::resolveHostname_alloc(static_cast<std::string>(player->ip).c_str(), 0);
	player->rdns.capture(resolved, strlen(resolved));
	player->rdns_mutex.unlock();
}

struct parsed_player_token {
	Jupiter::ReferenceString name;
	RenX::TeamType team{};
	int id{};
	bool isBot{};
	bool isPlayer{}; // i.e: they appear on the player list; not "ai"
};

void RenX::Server::processLine(const Jupiter::ReadableString &line) {
	if (line.isEmpty())
		return;

	auto& xPlugins = RenX::getCore()->getPlugins();
	auto tokens_view = jessilib::split_view(std::string_view{line}, m_rconVersion == 3 ? RenX::DelimC3 : RenX::DelimC);
	std::vector<Jupiter::StringS> tokens;

	for (auto& token : tokens_view) {
		tokens.push_back(Jupiter::StringS{std::string(token)}); // TODO: remove this extraneous copy
		tokens.back().processEscapeSequences();
	}

	// Safety checker for getting a token at an index
	auto getToken = [&tokens](size_t index) -> Jupiter::ReferenceString {
		if (index < tokens.size()) {
			return tokens[index];
		}

		return {};
	};

	auto consume_tokens_as_command_list_format = [&tokens, this]() {
		for (auto& token : tokens) {
			m_commandListFormat.push_back(static_cast<std::string>(token));
		}
	};

	auto tokens_as_command_table = [&tokens, this]() {
		std::unordered_map<Jupiter::StringS, Jupiter::StringS, Jupiter::default_hash_function> table;
		size_t total_tokens = std::min(tokens.size(), m_commandListFormat.size());
		for (size_t index = 0; index != total_tokens; ++index) {
			table[m_commandListFormat[index]] = tokens[index];
		}

		return table;
	};

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
		m_firstAction = false;
		m_firstKill = false;
		m_firstDeath = false;

		if (isSeamless() == false)
			wipePlayers();
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
		const Jupiter::ReadableString &prefix = getCommandPrefix();
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
			triggerCommand(command, player, parameters);
		}
	};
	auto onAction = [this]()
	{
		if (m_firstAction == false)
		{
			m_firstAction = true;
		}
	};
	auto parsePlayerData = [this](const Jupiter::ReadableString &data) {
		parsed_player_token result;

		auto player_tokens = jessilib::split_n_view(std::string_view{data}, ',', 2);
		if (player_tokens.size() < 3) {
			return result;
		}

		Jupiter::ReferenceString idToken = player_tokens[1];
		result.name = player_tokens[2];
		result.team = RenX::getTeam(player_tokens[0]);
		if (idToken.isNotEmpty() && idToken.get(0) == 'b')
		{
			idToken.shiftRight(1);
			result.isBot = true;
			result.isPlayer = true;
		}
		else if (idToken == "ai") {
			result.isBot = true;
			result.isPlayer = false;
		}
		else {
			result.isBot = false;
			result.isPlayer = true;
		}
		result.id = idToken.asInt(10);

		return result;
	};
	auto get_next_temp_playerinfo = [](const Jupiter::ReadableString &name, RenX::TeamType team, bool isBot) {
		static RenX::PlayerInfo s_temp_players[4];
		static size_t s_temp_player_index{ 0 };

		// Go to next temp player
		++s_temp_player_index;
		if (s_temp_player_index >= sizeof(s_temp_players) / sizeof(RenX::PlayerInfo)) {
			s_temp_player_index = 0;
		}
		RenX::PlayerInfo *temp_player = &s_temp_players[s_temp_player_index];

		// Populate temp player with input data
		temp_player->name = name;
		temp_player->team = team;
		temp_player->isBot = isBot;

		return temp_player;
	};
	auto getPlayerOrAdd = [&](const Jupiter::ReadableString &name, int id, RenX::TeamType team, bool isBot, uint64_t steamid, const Jupiter::ReadableString &ip, const Jupiter::ReadableString &hwid)
	{
		if (id == 0) {
			if (name.isEmpty()) {
				// Bad parse; return null player
				static RenX::PlayerInfo s_null_player;
				return &s_null_player;
			}

			return get_next_temp_playerinfo(name, team, isBot);
		}

		RenX::PlayerInfo *player = getPlayer(id);
		if (player == nullptr)
		{
			// Initialize a new player
			this->players.emplace_back();
			player = &this->players.back();
			player->id = id;
			player->name = name;
			player->team = team;
			player->ip = ip;
			player->ip32 = Jupiter::Socket::pton4(static_cast<std::string>(player->ip).c_str());
			player->hwid = hwid;
			
			// RDNS
			if (resolvesRDNS() && player->ip32 != 0)
			{
				player->rdns_thread = std::thread(resolve_rdns, player);
				++m_player_rdns_resolutions_pending;
			}

			player->steamid = steamid;
			if (player->isBot = isBot)
				player->formatNamePrefix = IRCCOLOR "05[B]";

			player->joinTime = std::chrono::steady_clock::now();
			//if (id != 0)
			//	this->players.add(r);

			player->uuid = m_calc_uuid(*player);

			if (player->isBot == false)
			{
				RenX::exemptionDatabase->exemption_check(*player);
				banCheck(*player);
			}
			else
				++m_bot_count;

			for (const auto& plugin : xPlugins) {
				plugin->RenX_OnPlayerCreate(*this, *player);
			}
		}
		else
		{
			bool recalcUUID = false;
			player->team = team;
			if (player->ip32 == 0 && ip.isNotEmpty())
			{
				player->ip = ip;
				player->ip32 = Jupiter::Socket::pton4(static_cast<std::string>(player->ip).c_str());
				if (resolvesRDNS())
				{
					player->rdns_thread = std::thread(resolve_rdns, player);
					++m_player_rdns_resolutions_pending;
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
				setUUIDIfDifferent(*player, m_calc_uuid(*player));
				if (player->isBot == false)
				{
					RenX::exemptionDatabase->exemption_check(*player);
					banCheck(*player);
				}
			}
		}

		return player;
	};
	auto parseGetPlayerOrAdd = [&parsePlayerData, &getPlayerOrAdd, this](const Jupiter::ReadableString &token)
	{
		auto parsed_token = parsePlayerData(token);
		if (parsed_token.id == 0 && parsed_token.name.isEmpty()) {
			sendAdmChan(IRCCOLOR "04[Error]" IRCCOLOR" Failed to parse player token: %.*s", token.size(), token.ptr());
		}
		return getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, parsed_token.isBot, 0U, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty);
	};
	auto gotoToken = [&line, &tokens, this](size_t index)
	{
		if (index >= tokens.size())
			return Jupiter::ReferenceString::empty;

		const char delim = getVersion() >= 4 ? RenX::DelimC : RenX::DelimC3;
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
		m_fully_connected = true;

		for (const auto& plugin : xPlugins) {
			plugin->RenX_OnServerFullyConnected(*this);
		}
	};

	if (tokens[0].isNotEmpty())
	{
		char header = tokens[0].get(0);
		tokens[0].shiftRight(1);
		switch (header)
		{
		case 'r':
			if (m_lastCommand.equalsi("clientlist"_jrs))
			{
				// ID | IP | Steam ID | Admin Status | Team | Name
				if (tokens[0].isNotEmpty())
				{
					bool isBot = false;
					int id;
					uint64_t steamid = 0;
					RenX::TeamType team = TeamType::Other;
					Jupiter::ReferenceString steamToken = getToken(2);
					Jupiter::ReferenceString adminToken = getToken(3);
					Jupiter::ReferenceString teamToken = getToken(4);
					if (tokens[0].get(0) == 'b')
					{
						isBot = true;
						tokens[0].shiftRight(1);
						id = tokens[0].asInt();
						tokens[0].shiftLeft(1);
					}
					else
						id = tokens[0].asInt();

					if (steamToken.equals("-----NO-STEAM-----") == false)
						steamid = steamToken.asUnsignedLongLong();
					team = RenX::getTeam(teamToken);

					if (adminToken.equalsi("None"_jrs))
						getPlayerOrAdd(getToken(5), id, team, isBot, steamid, getToken(1), Jupiter::ReferenceString::empty);
					else
						getPlayerOrAdd(getToken(5), id, team, isBot, steamid, getToken(1), Jupiter::ReferenceString::empty)->adminType = adminToken;
				}
			}
			else if (m_lastCommand.equalsi("clientvarlist"_jrs))
			{
				if (m_commandListFormat.empty()) {
					consume_tokens_as_command_list_format();
				}
				else
				{
					/*e
					lRCON�Command;�Conn4�executed:�clientvarlist PlayerLog�Kills�PlayerKills�BotKills�Deaths�Score�Credits�Character�BoundVehicle�Vehicle�Spy�RemoteC4�ATMine�KDR�Ping�Admin�Steam�IP�ID�Name�Team�TeamNum
					rPlayerLog�Kills�PlayerKills�BotKills�Deaths�Score�Credits�Character�BoundVehicle�Vehicle�Spy�RemoteC4�ATMine�KDR�Ping�Admin�Steam�IP�ID�Name�Team�TeamNum
					rGDI,256,EKT-J�0�0�0�0�0�5217.9629�Rx_FamilyInfo_GDI_Soldier���False�0�0�0.0000�8�None�0x0110000104AE0666�127.0.0.1�256�EKT-J�GDI�0
					*/
					auto table = tokens_as_command_table();

					auto table_get = [&table](const Jupiter::ReadableString& in_key) -> Jupiter::StringS* {
						auto value = table.find(in_key);
						if (value != table.end()) {
							return &value->second;
						}

						return nullptr;
					};

					auto table_get_ref = [&table](const Jupiter::ReadableString& in_key, const Jupiter::ReadableString& in_default_value) -> const Jupiter::ReadableString& {
						auto value = table.find(in_key);
						if (value != table.end()) {
							return value->second;
						}

						return in_default_value;
					};

					auto parse = [&table_get](RenX::PlayerInfo *player)
					{
						Jupiter::ReadableString *value;

						value = table_get("KILLS"_jrs);
						if (value != nullptr)
							player->kills = value->asUnsignedInt();

						value = table_get("DEATHS"_jrs);
						if (value != nullptr)
							player->deaths = value->asUnsignedInt();

						value = table_get("SCORE"_jrs);
						if (value != nullptr)
							player->score = value->asDouble();

						value = table_get("CREDITS"_jrs);
						if (value != nullptr)
							player->credits = value->asDouble();

						value = table_get("CHARACTER"_jrs);
						if (value != nullptr)
							player->character = *value;

						value = table_get("VEHICLE"_jrs);
						if (value != nullptr)
							player->vehicle = *value;

						value = table_get("PING"_jrs);
						if (value != nullptr)
							player->ping = value->asUnsignedInt();

						value = table_get("ADMIN"_jrs);
						if (value != nullptr)
						{
							if (value->equals("None"_jrs))
								player->adminType = "";
							else
								player->adminType = *value;
						}
					};
					Jupiter::ReadableString *value = table_get("PLAYERLOG"_jrs);
					if (value != nullptr) {
						auto parsed_token = parsePlayerData(*value);
						parse(getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, false,
							table_get_ref("STEAM"_jrs, Jupiter::ReferenceString::empty).asUnsignedLongLong(),
							table_get_ref("IP"_jrs, Jupiter::ReferenceString::empty),
							table_get_ref("HWID"_jrs, Jupiter::ReferenceString::empty)));
					}
					else
					{
						Jupiter::ReadableString *name = table_get("NAME"_jrs);
						value = table_get("ID"_jrs);

						if (value != nullptr)
						{
							RenX::PlayerInfo *player = getPlayer(value->asInt());
							if (player != nullptr)
							{
								if (player->name.isEmpty())
								{
									player->name = table_get_ref("NAME"_jrs, Jupiter::ReferenceString::empty);
									player->name.processEscapeSequences();
								}
								if (player->ip.isEmpty())
									player->ip = table_get_ref("IP"_jrs, Jupiter::ReferenceString::empty);
								if (player->hwid.isEmpty())
									player->hwid = table_get_ref("HWID"_jrs, Jupiter::ReferenceString::empty);
								if (player->steamid == 0)
								{
									uint64_t steamid = table_get_ref("STEAM"_jrs, Jupiter::ReferenceString::empty).asUnsignedLongLong();
									if (steamid != 0)
									{
										player->steamid = steamid;
										setUUIDIfDifferent(*player, m_calc_uuid(*player));
									}
								}

								value = table_get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table_get("TEAM"_jrs);
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
									player->ip = table_get_ref("IP"_jrs, Jupiter::ReferenceString::empty);
								if (player->hwid.isEmpty())
									player->hwid = table_get_ref("HWID"_jrs, Jupiter::ReferenceString::empty);
								if (player->steamid == 0)
								{
									uint64_t steamid = table_get_ref("STEAM"_jrs, Jupiter::ReferenceString::empty).asUnsignedLongLong();
									if (steamid != 0)
									{
										player->steamid = steamid;
										setUUIDIfDifferent(*player, m_calc_uuid(*player));
									}
								}

								value = table_get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table_get("TEAM"_jrs);
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
			else if (m_lastCommand.equalsi("botlist")) {
				// Team,ID,Name
				if (m_commandListFormat.empty()) {
					consume_tokens_as_command_list_format();
				}
				else {
					parseGetPlayerOrAdd(tokens[0]);
				}
			}
			else if (m_lastCommand.equalsi("botvarlist")) {
				if (m_commandListFormat.empty()) {
					consume_tokens_as_command_list_format();
				}
				else
				{
					/*
					lRCON�Command;�Conn4�executed:�clientvarlist PlayerLog�Kills�PlayerKills�BotKills�Deaths�Score�Credits�Character�BoundVehicle�Vehicle�Spy�RemoteC4�ATMine�KDR�Ping�Admin�Steam�IP�ID�Name�Team�TeamNum
					rPlayerLog�Kills�PlayerKills�BotKills�Deaths�Score�Credits�Character�BoundVehicle�Vehicle�Spy�RemoteC4�ATMine�KDR�Ping�Admin�Steam�IP�ID�Name�Team�TeamNum
					rGDI,256,EKT-J�0�0�0�0�0�5217.9629�Rx_FamilyInfo_GDI_Soldier���False�0�0�0.0000�8�None�0x0110000104AE0666�127.0.0.1�256�EKT-J�GDI�0
					*/
					auto table = tokens_as_command_table();

					auto table_get = [&table](const Jupiter::ReadableString& in_key) -> Jupiter::StringS* {
						auto value = table.find(in_key);
						if (value != table.end()) {
							return &value->second;
						}

						return nullptr;
					};

					auto table_get_ref = [&table](const Jupiter::ReadableString& in_key, const Jupiter::ReadableString& in_default_value) -> const Jupiter::ReadableString& {
						auto value = table.find(in_key);
						if (value != table.end()) {
							return value->second;
						}

						return in_default_value;
					};

					auto parse = [&table_get](RenX::PlayerInfo *player)
					{
						Jupiter::ReadableString *value;

						value = table_get("KILLS"_jrs);
						if (value != nullptr)
							player->kills = value->asUnsignedInt();

						value = table_get("DEATHS"_jrs);
						if (value != nullptr)
							player->deaths = value->asUnsignedInt();

						value = table_get("SCORE"_jrs);
						if (value != nullptr)
							player->score = value->asDouble();

						value = table_get("CREDITS"_jrs);
						if (value != nullptr)
							player->credits = value->asDouble();

						value = table_get("CHARACTER"_jrs);
						if (value != nullptr)
							player->character = *value;

						value = table_get("VEHICLE"_jrs);
						if (value != nullptr)
							player->vehicle = *value;
					};
					Jupiter::ReadableString *value = table_get("PLAYERLOG"_jrs);
					if (value != nullptr) {
						auto parsed_token = parsePlayerData(*value);
						parse(getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, true, 0ULL,
							Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty));
					}
					else
					{
						Jupiter::ReadableString *name = table_get("NAME"_jrs);
						value = table_get("ID"_jrs);

						if (value != nullptr)
						{
							RenX::PlayerInfo *player = getPlayer(value->asInt());
							if (player != nullptr)
							{
								if (player->name.isEmpty())
								{
									player->name = table_get_ref("NAME"_jrs, Jupiter::ReferenceString::empty);
									player->name.processEscapeSequences();
								}

								value = table_get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table_get("TEAM"_jrs);
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
								value = table_get("TEAMNUM"_jrs);
								if (value != nullptr)
									player->team = RenX::getTeam(value->asInt());
								else
								{
									value = table_get("TEAM"_jrs);
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
			else if (m_lastCommand.equalsi("binfo") || m_lastCommand.equalsi("buildinginfo") || m_lastCommand.equalsi("blist") || m_lastCommand.equalsi("buildinglist"))
			{
				if (m_commandListFormat.empty()) {
					consume_tokens_as_command_list_format();
				}
				else
				{
					/*
					lRCON�Command;�DevBot�executed:�binfo
					rBuilding�Health�MaxHealth�Armor MaxArmor Team�Capturable Destroyed
					rRx_Building_Refinery_GDI�2000�2000�2000 2000 GDI�False False
					*/
					auto table = tokens_as_command_table();

					auto table_get = [&table](const Jupiter::ReadableString& in_key) -> Jupiter::StringS* {
						auto value = table.find(in_key);
						if (value != table.end()) {
							return &value->second;
						}

						return nullptr;
					};

					Jupiter::ReadableString *value;
					RenX::BuildingInfo *building;

					value = table_get("Building"_jrs);
					if (value != nullptr)
					{
						building = getBuildingByName(*value);
						if (building == nullptr)
						{
							building = new RenX::BuildingInfo();
							RenX::Server::buildings.emplace_back(building);
							building->name = *value;
						}

						value = table_get("Health"_jrs);
						if (value != nullptr)
							building->health = value->asInt(10);

						value = table_get("MaxHealth"_jrs);
						if (value != nullptr)
							building->max_health = value->asInt(10);

						value = table_get("Team"_jrs);
						if (value != nullptr)
							building->team = RenX::getTeam(*value);

						value = table_get("Capturable"_jrs);
						if (value != nullptr)
							building->capturable = value->asBool();

						value = table_get("Destroyed"_jrs);
						if (value != nullptr)
							building->destroyed = value->asBool();

						value = table_get("Armor"_jrs);
						if (value != nullptr)
							building->armor = value->asInt(10);

						value = table_get("MaxArmor"_jrs);
						if (value != nullptr)
							building->max_armor = value->asInt(10);
					}
				}
			}
			else if (m_lastCommand.equalsi("ping"))
			{
				if (getToken(1).equals("srv_init_done"_jrs))
					finished_connecting();
				else
					m_awaitingPong = false;
			}
			else if (m_lastCommand.equalsi("map"))
			{
				// Map | Guid
				m_map.name = getToken(0);
				const Jupiter::ReferenceString guid_token = getToken(1);

				if (guid_token.size() == 32U)
				{
					m_map.guid[0] = guid_token.substring(size_t{ 0 }, 16U).asUnsignedLongLong(16);
					m_map.guid[1] = guid_token.substring(16U).asUnsignedLongLong(16);
				}
			}
			else if (m_lastCommand.equalsi("serverinfo"))
			{
				if (m_lastCommandParams.isEmpty())
				{
					// "Port"�| Port |�"Name" |�Name |�"Level"�| Level | "Players" | Players�| "Bots" | Bots | "LevelGUID" | Level GUID
					m_port = static_cast<unsigned short>(getToken(1).asUnsignedInt(10));
					m_serverName = getToken(3);
					m_map.name = getToken(5);

					const Jupiter::ReferenceString guid_token = getToken(11);
					if (guid_token.size() == 32U)
					{
						m_map.guid[0] = guid_token.substring(size_t{ 0 }, 16U).asUnsignedLongLong(16);
						m_map.guid[1] = guid_token.substring(16U).asUnsignedLongLong(16);
					}
				}
			}
			else if (m_lastCommand.equalsi("gameinfo"_jrs))
			{
				if (m_lastCommandParams.isEmpty())
				{
					// "PlayerLimit" | PlayerLimit | "VehicleLimit" | VehicleLimit | "MineLimit" | MineLimit | "TimeLimit" | TimeLimit | "bPassworded" | bPassworded | "bSteamRequired" | bSteamRequired | "bPrivateMessageTeamOnly" | bPrivateMessageTeamOnly | "bAllowPrivateMessaging" | bAllowPrivateMessaging | "TeamMode" | TeamMode | "bSpawnCrates" | bSpawnCrates | "CrateRespawnAfterPickup" | CrateRespawnAfterPickup | bIsCompetitive | "bIsCompetitive"
					m_playerLimit = getToken(1).asInt();
					m_vehicleLimit = getToken(3).asInt();
					m_mineLimit = getToken(5).asInt();
					m_timeLimit = getToken(7).asInt();
					m_passworded = getToken(9).asBool();
					m_steamRequired = getToken(11).asBool();
					m_privateMessageTeamOnly = getToken(13).asBool();
					m_allowPrivateMessaging = getToken(15).asBool();
					m_team_mode = m_rconVersion >= 4 ? getToken(17).asInt() : true;
					m_spawnCrates = getToken(19).asBool();
					m_crateRespawnAfterPickup = getToken(21).asDouble();

					if (m_rconVersion >= 4)
					{
						m_competitive = getToken(23).asBool();

						const Jupiter::ReadableString &match_state_token = getToken(25);
						if (match_state_token.equalsi("PendingMatch"_jrs))
							m_match_state = 0;
						else if (match_state_token.equalsi("MatchInProgress"_jrs))
							m_match_state = 1;
						else if (match_state_token.equalsi("RoundOver"_jrs) || match_state_token.equalsi("MatchOver"_jrs))
							m_match_state = 2;
						else if (match_state_token.equalsi("TravelTheWorld"_jrs))
							m_match_state = 3;
						else // Unknown state -- assume it's in progress
							m_match_state = 1;

						m_botsEnabled = getToken(27).asBool();
						m_game_type = getToken(29).asInt();
					}
				}
			}
			else if (m_lastCommand.equalsi("mutatorlist"_jrs)) {
				// "The following mutators are loaded:" [ | Mutator [ | Mutator [ ... ] ] ]
				if (tokens.size() == 1) {
					m_pure = true;
				}
				else if (tokens.size() == 0) {
					disconnect(RenX::DisconnectReason::ProtocolError);
				}
				else {
					RenX::Server::mutators.clear();
					size_t index = tokens.size();
					while (--index != 0)
						RenX::Server::mutators.emplace_back(tokens[index]);
				}
			}
			else if (m_lastCommand.equalsi("rotation"_jrs)) {
				// Map | Guid
				const Jupiter::ReadableString &in_map = getToken(0);
				if (hasMapInRotation(in_map) == false) {
					this->maps.emplace_back(in_map);

					const Jupiter::ReferenceString guid_token = getToken(1);
					if (guid_token.size() == 32U) {
						this->maps.back().guid[0] = guid_token.substring(size_t{ 0 }, 16U).asUnsignedLongLong(16);
						this->maps.back().guid[1] = guid_token.substring(16U).asUnsignedLongLong(16);
					}
				}
			}
			else if (m_lastCommand.equalsi("changename")) {
				RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(0));
				Jupiter::StringS newName = getToken(2);
				for (const auto& plugin : xPlugins) {
					plugin->RenX_OnNameChange(*this, *player, newName);
				}
				player->name = newName;
			}
			break;
		case 'l':
			if (m_rconVersion >= 3) {
				Jupiter::ReferenceString subHeader = getToken(1);
				if (tokens[0].equals("GAME")) {
					if (subHeader.equals("Deployed;")) {
						// Object (Beacon/Mine) | Player
						// Object (Beacon/Mine) | Player | "on" | Surface
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
						Jupiter::ReferenceString objectType = getToken(2);
						if (objectType.match("*Beacon"))
							++player->beaconPlacements;
						else if (objectType.equals("Rx_Weapon_DeployedProxyC4"_jrs))
							++player->proxy_placements;
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnDeploy(*this, *player, objectType);
						}
						onAction();
					}
					else if (subHeader.equals("Disarmed;")) {
						// Object (Beacon/Mine) | "by" | Player
						// Object (Beacon/Mine) | "by" | Player | "owned by" | Owner
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
						Jupiter::ReferenceString objectType = getToken(2);
						if (objectType.match("*Beacon"))
							++player->beaconDisarms;
						else if (objectType.equals("Rx_Weapon_DeployedProxyC4"_jrs))
							++player->proxy_disarms;

						if (getToken(5).equals("owned by")) {
							RenX::PlayerInfo *victim = parseGetPlayerOrAdd(getToken(6));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnDisarm(*this, *player, objectType, *victim);
							}
						}
						else {
							for (const auto& plugin: xPlugins) {
								plugin->RenX_OnDisarm(*this, *player, objectType);
							}
						}
						onAction();
					}
					else if (subHeader.equals("Exploded;")) {
						// Pre-5.15:
						// Explosive | "at" | Location
						// Explosive | "at" | Location | "by" | Owner
						// 5.15+:
						// Explosive | "near" | Spot Location | "at" | Location | "by" | Owner
						// Explosive | "near" | Spot Location | "at" | Location
						Jupiter::ReferenceString explosive = getToken(2);
						if (getToken(5).equals("at")) { // 5.15+
							if (getToken(7).equals("by")) { // Player information specified
								RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(8));
								for (const auto& plugin : xPlugins) {
									plugin->RenX_OnExplode(*this, *player, explosive);
								}
							}
							else { // No player information specified
								for (const auto& plugin: xPlugins) {
									plugin->RenX_OnExplode(*this, explosive);
								}
							}
						}
						else if (getToken(5).equals("by")) { // Pre-5.15 with player information specified
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(6));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnExplode(*this, *player, explosive);
							}
						}
						else { // Pre-5.15 with no player information specified
							for (const auto& plugin: xPlugins) {
								plugin->RenX_OnExplode(*this, explosive);
							}
						}
						onAction();
					}
					else if (subHeader.equals("ProjectileExploded;")) {
						// Explosive | "at" | Location
						// Explosive | "at" | Location | "by" | Owner
						Jupiter::ReferenceString explosive = getToken(2);
						if (getToken(5).equals("by")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(6));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnExplode(*this, *player, explosive);
							}
						}
						else {
							for (const auto& plugin: xPlugins) {
								plugin->RenX_OnExplode(*this, explosive);
							}
						}
						onAction();
					}
					else if (subHeader.equals("Captured;")) {
						// Team ',' Building | "id" | Building ID | "by" | Player
						auto teamBuildingToken = jessilib::split_once_view(getToken(2), ',');
						Jupiter::ReferenceString building = teamBuildingToken.second;
						TeamType oldTeam = RenX::getTeam(teamBuildingToken.first);
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(6));
						player->captures++;
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnCapture(*this, *player, building, oldTeam);
						}
						onAction();
					}
					else if (subHeader.equals("Neutralized;")) {
						// Team ',' Building | "id" | Building ID | "by" | Player
						auto teamBuildingToken = jessilib::split_once_view(getToken(2), ',');
						Jupiter::ReferenceString building = teamBuildingToken.second;
						TeamType oldTeam = RenX::getTeam(teamBuildingToken.first);
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(6));
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnNeutralize(*this, *player, building, oldTeam);
						}
						onAction();
					}
					else if (subHeader.equals("Purchase;")) {
						// "character" | Character | "by" | Player
						// "item" | Item | "by" | Player
						// "weapon" | Weapon | "by" | Player
						// "refill" | Player
						// "vehicle" | Vehicle | "by" | Player
						Jupiter::ReferenceString type = getToken(2);
						Jupiter::ReferenceString obj = getToken(3);
						if (type.equals("character")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnCharacterPurchase(*this, *player, obj);
							}
							player->character = obj;
						}
						else if (type.equals("item")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnItemPurchase(*this, *player, obj);
							}
						}
						else if (type.equals("weapon")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnWeaponPurchase(*this, *player, obj);
							}
						}
						else if (type.equals("refill")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(obj);
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnRefillPurchase(*this, *player);
							}
						}
						else if (type.equals("vehicle")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnVehiclePurchase(*this, *player, obj);
							}
						}
					}
					else if (subHeader.equals("Spawn;")) {
						// "vehicle" | Vehicle Team, Vehicle
						// "player" | Player | "character" | Character
						// "bot" | Player
						if (getToken(2).equals("vehicle")) {
							auto vehicle = jessilib::split_once_view(getToken(3), ',');
							TeamType team = RenX::getTeam(vehicle.first);
							Jupiter::ReferenceString vehicle_name = vehicle.second;
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnVehicleSpawn(*this, team, vehicle_name);
							}
						}
						else if (getToken(2).equals("player")) {
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(3));
							Jupiter::ReferenceString character = getToken(5);
							player->character = character;
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnSpawn(*this, *player, character);
							}
						}
						else if (getToken(2).equals("bot")) {
							RenX::PlayerInfo *bot = parseGetPlayerOrAdd(getToken(3));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnBotJoin(*this, *bot);
							}
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
						Jupiter::ReferenceString type = getToken(2);
						if (type.equals("vehicle"))
						{
							Jupiter::ReferenceString vehicle = getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnVehicleCrate(*this, *player, vehicle);
							}
						}
						else if (type.equals("tsvehicle"))
						{
							Jupiter::ReferenceString vehicle = getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnVehicleCrate(*this, *player, vehicle);
							}
						}
						else if (type.equals("ravehicle"))
						{
							Jupiter::ReferenceString vehicle = getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnVehicleCrate(*this, *player, vehicle);
							}
						}
						else if (type.equals("death") || type.equals("suicide"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnDeathCrate(*this, *player);
							}
						}
						else if (type.equals("money"))
						{
							int amount = getToken(3).asInt();
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnMoneyCrate(*this, *player, amount);
							}
						}
						else if (type.equals("character"))
						{
							Jupiter::ReferenceString character = getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnCharacterCrate(*this, *player, character);
							}
							player->character = character;
						}
						else if (type.equals("spy"))
						{
							Jupiter::ReferenceString character = getToken(3);
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnSpyCrate(*this, *player, character);
							}
							player->character = character;
						}
						else if (type.equals("refill"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnRefillCrate(*this, *player);
							}
						}
						else if (type.equals("timebomb"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnTimeBombCrate(*this, *player);
							}
						}
						else if (type.equals("speed"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnSpeedCrate(*this, *player);
							}
						}
						else if (type.equals("nuke"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnNukeCrate(*this, *player);
							}
						}
						else if (type.equals("abduction"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnAbductionCrate(*this, *player);
							}
						}
						else if (type.equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(3));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnUnspecifiedCrate(*this, *player);
							}
						}
						else {
							RenX::PlayerInfo *player = nullptr;
							if (getToken(3).equals("by")) {
								player = parseGetPlayerOrAdd(getToken(4));
							}

							if (player != nullptr) {
								for (const auto& plugin: xPlugins) {
									plugin->RenX_OnOtherCrate(*this, *player, type);
								}
							}
						}
					}
					else if (subHeader.equals("Death;"))
					{
						// "player" | Player | "by" | Killer Player | "with" | Damage Type
						// "player" | Player | "died by" | Damage Type
						// "player" | Player | "suicide by" | Damage Type
						//		NOTE: Filter these out when Player.isEmpty().
						Jupiter::ReferenceString playerToken = getToken(3);
						if (playerToken.isNotEmpty())
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(playerToken);
							Jupiter::ReferenceString type = getToken(4);
							Jupiter::ReferenceString damageType;
							if (type.equals("by"))
							{
								damageType = getToken(7);
								Jupiter::ReferenceString killerData = getToken(5);
								auto parsed_token = parsePlayerData(killerData);
								if (!parsed_token.isPlayer || parsed_token.id == 0)
								{
									player->deaths++;
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnKill(*this, parsed_token.name, parsed_token.team, *player, damageType);
									}
								}
								else
								{
									player->deaths++;
									RenX::PlayerInfo *killer = getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, parsed_token.isBot, 0, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty);
									killer->kills++;
									if (damageType.equals("Rx_DmgType_Headshot")) {
										killer->headshots++;
									}
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnKill(*this, *killer, *player, damageType);
									}
								}
							}
							else if (type.equals("died by"))
							{
								player->deaths++;
								damageType = getToken(5);
								for (const auto& plugin : xPlugins) {
									plugin->RenX_OnDie(*this, *player, damageType);
								}
							}
							else if (type.equals("suicide by"))
							{
								player->deaths++;
								player->suicides++;
								damageType = getToken(5);
								for (const auto& plugin : xPlugins) {
									plugin->RenX_OnSuicide(*this, *player, damageType);
								}
							}
							player->character = Jupiter::ReferenceString::empty;
						}
						onAction();
					}
					else if (subHeader.equals("Stolen;"))
					{
						// Vehicle | "by" | Player
						// Vehicle | "bound to" | Bound Player | "by" | Player
						Jupiter::ReferenceString vehicle = getToken(2);
						Jupiter::ReferenceString byLine = getToken(3);
						if (byLine.equals("by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							player->steals++;
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnSteal(*this, *player, vehicle);
							}
						}
						else if (byLine.equals("bound to"))
						{
							RenX::PlayerInfo *victim = parseGetPlayerOrAdd(getToken(4));
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(6));
							player->steals++;
							victim->stolen++;
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnSteal(*this, *player, vehicle, *victim);
							}
						}
						onAction();
					}
					else if (subHeader.equals("Destroyed;"))
					{
						// "vehicle" | Vehicle | "by" | Killer | "with" | Damage Type
						// "defence" | Defence | "by" | Killer | "with" | Damage Type
						// "emplacement" | Emplacement | "by" | Killer Player | "with" | Damage Type
						// "building" | Building | "by" | Killer | "with" | Damage Type
						Jupiter::ReferenceString typeToken = getToken(2);
						RenX::ObjectType type = ObjectType::None;
						if (typeToken.equals("vehicle"))
							type = ObjectType::Vehicle;
						else if (typeToken.equals("defence") || typeToken.equals("emplacement"))
							type = ObjectType::Defence;
						else if (typeToken.equals("building"))
							type = ObjectType::Building;

						if (type != ObjectType::None)
						{
							Jupiter::ReferenceString objectName = getToken(3);
							if (getToken(4).equals("by"))
							{
								Jupiter::ReferenceString killerToken = getToken(5);
								auto parsed_token = parsePlayerData(killerToken);
								Jupiter::ReferenceString damageType = getToken(7);

								if (!parsed_token.isPlayer || parsed_token.id == 0) {
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnDestroy(*this, parsed_token.name, parsed_token.team, objectName, RenX::getEnemy(parsed_token.team), damageType, type);
									}
								}
								else {
									RenX::PlayerInfo *player = getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, parsed_token.isBot, 0, Jupiter::ReferenceString::empty, Jupiter::ReferenceString::empty);
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
											building = getBuildingByName(objectName);
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
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnDestroy(*this, *player, objectName, RenX::getEnemy(player->team), damageType, type);
									}
								}
							}
						}
						onAction();
					}
					else if (subHeader.equals("Donated;"))
					{
						// Amount | "to" | Recipient | "by" | Donor
						if (getToken(5).equals("by"))
						{
							double amount = getToken(2).asDouble();
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(4));
							RenX::PlayerInfo *donor = parseGetPlayerOrAdd(getToken(6));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnDonate(*this, *donor, *player, amount);
							}
						}
					}
					else if (subHeader.equals("OverMine;"))
					{
						// Player | "near" | Location
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString location = getToken(4);

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnOverMine(*this, *player, location);
						}
					}
					else if (subHeader.equals("MatchEnd;")) {
						// "winner" | Winner | Reason("TimeLimit" etc) | "GDI=" GDI Score | "Nod=" Nod Score
						// "tie" | Reason | "GDI=" GDI Score | "Nod=" Nod Score
						Jupiter::ReferenceString winTieToken = getToken(2);
						m_match_state = 2;

						if (winTieToken.equals("winner")) {
							Jupiter::ReferenceString sWinType = getToken(4);
							WinType winType = WinType::Unknown;
							if (sWinType.equals("TimeLimit"))
								winType = WinType::Score;
							else if (sWinType.equals("Buildings"))
								winType = WinType::Base;
							else if (sWinType.equals("triggered"))
								winType = WinType::Shutdown;
							else if (sWinType.equals("Surrender"))
								winType = WinType::Surrender;

							TeamType team = RenX::getTeam(getToken(3));
							int gScore = Jupiter::ReferenceString(jessilib::split_once_view(getToken(5), '=').second).asInt();
							int nScore = Jupiter::ReferenceString(jessilib::split_once_view(getToken(6), '=').second).asInt();

							onPreGameOver(winType, team, gScore, nScore);
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnGameOver(*this, winType, team, gScore, nScore);
							}
						}
						else if (winTieToken.equals("tie")) {
							int gScore = Jupiter::ReferenceString(jessilib::split_once_view(getToken(4), '=').second).asInt();
							int nScore = Jupiter::ReferenceString(jessilib::split_once_view(getToken(5), '=').second).asInt();
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnGameOver(*this, RenX::WinType::Tie, RenX::TeamType::None, gScore, nScore);
							}
						}
						m_gameover_pending = false;
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnGame(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("CHAT"))
				{
					if (subHeader.equals("Say;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						onChat(*player, message);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnChat(*this, *player, message);
						}
						onAction();
					}
					else if (subHeader.equals("TeamSay;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						onChat(*player, message);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnTeamChat(*this, *player, message);
						}
						onAction();
					}
					else if (subHeader.equals("Radio;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnRadioChat(*this, *player, message);
						}
						onAction();
					}
					else if (subHeader.equals("AdminMsg;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAdminMessage(*this, *player, message);
						}
						onAction();
					}
					else if (subHeader.equals("AdminWarn;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnWarnMessage(*this, *player, message);
						}
						onAction();
					}
					else if (subHeader.equals("PAdminMsg;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						RenX::PlayerInfo *target = parseGetPlayerOrAdd(getToken(4));
						Jupiter::ReferenceString message = getToken(6);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAdminPMessage(*this, *player, *target, message);
						}
						onAction();
					}
					else if (subHeader.equals("PAdminWarn;"))
					{
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						RenX::PlayerInfo *target = parseGetPlayerOrAdd(getToken(4));
						Jupiter::ReferenceString message = getToken(6);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnWarnPMessage(*this, *player, *target, message);
						}
						onAction();
					}
					else if (subHeader.equals("HostSay;"))
					{
						Jupiter::ReferenceString message = getToken(3);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHostChat(*this, message);
						}
					}
					else if (subHeader.equals("HostPMsg;")) {
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHostPage(*this, *player, message);
						}
					}
					else if (subHeader.equals("HostAdminMsg;"))
					{
						Jupiter::ReferenceString message = getToken(3);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHostAdminMessage(*this, message);
						}
					}
					else if (subHeader.equals("HostAdminWarn;"))
					{
						Jupiter::ReferenceString message = getToken(3);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHostWarnMessage(*this, message);
						}
					}
					else if (subHeader.equals("HostPAdminMsg;")) {
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHostAdminPMessage(*this, *player, message);
						}
					}
					else if (subHeader.equals("HostPAdminWarn;")) {
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHostWarnPMessage(*this, *player, message);
						}
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
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnOtherChat(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("PLAYER"))
				{
					if (subHeader.equals("Enter;"))
					{
						auto parsed_token = parsePlayerData(getToken(2));
						uint64_t steamid = 0;
						RenX::PlayerInfo *player;

						if (getToken(5).equals("hwid"))
						{
							// New format
							if (getToken(7).equals("steamid"))
								steamid = getToken(8).asUnsignedLongLong();

							player = getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, parsed_token.isBot, steamid, getToken(4), getToken(6));
						}
						else
						{
							// Old format
							if (getToken(5).equals("steamid"))
								steamid = getToken(6).asUnsignedLongLong();

							player = getPlayerOrAdd(parsed_token.name, parsed_token.id, parsed_token.team, parsed_token.isBot, steamid, getToken(4), Jupiter::ReferenceString::empty);
						}

						if (steamid != 0ULL && default_ladder_database != nullptr && (player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_LADDER) == 0)
						{
							RenX::LadderDatabase::Entry *itr = RenX::default_ladder_database->getHead();
							while (itr != nullptr)
							{
								if (itr->steam_id == steamid)
								{
									player->local_rank = itr->rank;
									if (m_devBot)
									{
										player->global_rank = itr->rank;
										if (m_rconVersion >= 4)
											sendData(Jupiter::StringS::Format("dset_rank %d %d\n", player->id, player->global_rank));
									}
									break;
								}

								itr = itr->next;
							}
						}
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnJoin(*this, *player);
						}
					}
					else if (subHeader.equals("TeamJoin;"))
					{
						// Player | "joined" | Team | "score" | Score | "last round score" | Score | "time" | Timestamp
						// Player | "joined" | Team | "left" | Old Team | "score" | Score | "last round score" | Score | "time" | Timestamp
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						player->character = Jupiter::ReferenceString::empty;
						if (getToken(5) == "left")
						{
							RenX::TeamType oldTeam = RenX::getTeam(getToken(6));
							if (oldTeam != RenX::TeamType::None)
								for (const auto& plugin : xPlugins) {
									plugin->RenX_OnTeamChange(*this, *player, oldTeam);
								}
						}
					}
					else if (subHeader.equals("HWID;"))
					{
						// ["player" |] Player | "hwid" | HWID
						size_t offset = 0;
						if (getToken(2).equals("player"))
							offset = 1;

						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2 + offset));
						player->hwid = getToken(4 + offset);

						if (player->isBot == false) {
							banCheck(*player);
						}

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnHWID(*this, *player);
						}

						if (player->rdns.isNotEmpty()) {
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnPlayerIdentify(*this, *player);
							}
						}
					}
					else if (subHeader.equals("Exit;"))
					{
						// Player
						Jupiter::ReferenceString playerToken = getToken(2);
						auto parsed_token = parsePlayerData(playerToken);

						RenX::PlayerInfo *player = getPlayer(parsed_token.id);
						if (player != nullptr) {
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnPart(*this, *player);
							}

							removePlayer(*player);
						}

						if (m_gameover_when_empty && this->players.size() == getBotCount())
							gameover();
					}
					else if (subHeader.equals("Kick;"))
					{
						// Player | "for" | Reason
						const Jupiter::ReadableString &reason = getToken(4);
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnKick(*this, *player, reason);
						}
					}
					else if (subHeader.equals("NameChange;"))
					{
						// Player | "to:" | New Name
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::StringS newName = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnNameChange(*this, *player, newName);
						}
						player->name = newName;
						onAction();
					}
					else if (subHeader.equals("ChangeID;"))
					{
						// "to" | New ID | "from" | Old ID
						int oldID = getToken(5).asInt();
						RenX::PlayerInfo *player = getPlayer(oldID);
						if (player != nullptr)
						{
							player->id = getToken(3).asInt();

							if (player->isBot == false)
								banCheck(*player);

							if (m_devBot && player->global_rank != 0U)
							{
								if (m_rconVersion >= 4)
									sendData(Jupiter::StringS::Format("dset_rank %d %d\n", player->id, player->global_rank));
							}

							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnIDChange(*this, *player, oldID);
							}
						}
					}
					else if (subHeader.equals("Rank;"))
					{
						// Player | Rank
						if (m_devBot == false)
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
							if (player != nullptr)
								player->global_rank = getToken(3).asUnsignedInt();

							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnRank(*this, *player);
							}
						}
					}
					else if (subHeader.equals("Dev;"))
					{
						// Player | true/false
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						if (player != nullptr)
							player->is_dev = getToken(3).asBool();

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnDev(*this, *player);
						}
					}
					else if (subHeader.equals("SpeedHack;"))
					{
						// Player
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnSpeedHack(*this, *player);
						}
					}
					else if (subHeader.equals("Command;"))
					{
						// Player | Command
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						Jupiter::ReferenceString message = gotoToken(3);

						RenX::GameCommand *command = triggerCommand(Jupiter::ReferenceString::getWord(message, 0, WHITESPACE), *player, Jupiter::ReferenceString::gotoWord(message, 1, WHITESPACE));

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnPlayerCommand(*this, *player, message, command);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnPlayer(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("RCON"))
				{
					if (subHeader.equals("Command;"))
					{
						// User | "executed:" | Command
						Jupiter::ReferenceString user = getToken(2);
						if (getToken(3).equals("executed:"))
						{
							Jupiter::ReferenceString command = gotoToken(4);
							Jupiter::ReferenceString cmd = command.getWord(0, " ");

							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnExecute(*this, user, command);
							}

							if (m_rconUser.equals(user))
							{
								m_lastCommand = cmd;
								m_lastCommandParams = command.gotoWord(1, " ");
							}
						}
					}
					else if (subHeader.equals("Subscribed;"))
					{
						// User
						Jupiter::ReferenceString user = getToken(2);

						if (user.equals(m_rconUser))
							m_subscribed = true;

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnSubscribe(*this, user);
						}
					}
					else if (subHeader.equals("Unsubscribed;"))
					{
						// User
						Jupiter::ReferenceString user = getToken(2);

						if (user.equals(m_rconUser))
							m_subscribed = false;

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnUnsubscribe(*this, user);
						}
					}
					else if (subHeader.equals("Blocked;"))
					{
						// User | Reason="(Denied by IP Policy)" / "(Not on Whitelist)"
						Jupiter::ReferenceString user = getToken(2);
						Jupiter::ReferenceString message = getToken(3);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnBlock(*this, user, message);
						}
					}
					else if (subHeader.equals("Connected;"))
					{
						// User
						Jupiter::ReferenceString user = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnConnect(*this, user);
						}
					}
					else if (subHeader.equals("Authenticated;"))
					{
						// User
						Jupiter::ReferenceString user = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAuthenticate(*this, user);
						}
					}
					else if (subHeader.equals("Banned;"))
					{
						// User | "reason" | Reason="(Too many password attempts)"
						Jupiter::ReferenceString user = getToken(2);
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnBan(*this, user, message);
						}
					}
					else if (subHeader.equals("InvalidPassword;"))
					{
						// User
						Jupiter::ReferenceString user = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnInvalidPassword(*this, user);
						}
					}
					else if (subHeader.equals("Dropped;"))
					{
						// User | "reason" | Reason="(Auth Timeout)"
						Jupiter::ReferenceString user = getToken(2);
						Jupiter::ReferenceString message = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnDrop(*this, user, message);
						}
					}
					else if (subHeader.equals("Disconnected;"))
					{
						// User
						Jupiter::ReferenceString user = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnDisconnect(*this, user);
						}
					}
					else if (subHeader.equals("StoppedListen;"))
					{
						// Reason="(Reached Connection Limit)"
						Jupiter::ReferenceString message = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnStopListen(*this, message);
						}
					}
					else if (subHeader.equals("ResumedListen;"))
					{
						// Reason="(No longer at Connection Limit)"
						Jupiter::ReferenceString message = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnResumeListen(*this, message);
						}
					}
					else if (subHeader.equals("Warning;"))
					{
						// Warning="(Hit Max Attempt Records - You should investigate Rcon attempts and/or decrease prune time)"
						Jupiter::ReferenceString message = getToken(2);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnWarning(*this, message);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnRCON(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("ADMIN"))
				{
					if (subHeader.equals("Rcon;"))
					{
						// Player | "executed:" | Command
						if (getToken(3).equals("executed:"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
							Jupiter::ReferenceString cmd = gotoToken(4);
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnExecute(*this, *player, cmd);
							}
						}
					}
					else if (subHeader.equals("Login;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						player->adminType = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAdminLogin(*this, *player);
						}
					}
					else if (subHeader.equals("Logout;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAdminLogout(*this, *player);
						}

						player->adminType.erase();
					}
					else if (subHeader.equals("Granted;"))
					{
						// Player | "as" | Type="moderator" / "administrator"
						RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(2));
						player->adminType = getToken(4);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAdminGrant(*this, *player);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnAdmin(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("VOTE"))
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

						Jupiter::ReferenceString voteType = getToken(3);
						Jupiter::ReferenceString teamToken = getToken(2);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						if (getToken(4).equals("parameters")) // Pre-5.15 style parameters; throw away parameters
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(tokens.size() - 1));

							if ((player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_VOTE) != 0)
								sendData(Jupiter::StringS::Format("ccancelvote %.*s\n", teamToken.size(), teamToken.ptr()));

							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnVoteOther(*this, team, voteType, *player);
							}
						}
						else // 5.15+ (or empty)
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(5));

							if ((player->ban_flags & RenX::BanDatabase::Entry::FLAG_TYPE_VOTE) != 0)
								sendData(Jupiter::StringS::Format("ccancelvote %.*s\n", teamToken.size(), teamToken.ptr()));

							// PARSE PARAMETERS HERE

							if (voteType.find("Rx_VoteMenuChoice_"_jrs) == 0)
							{
								voteType.shiftRight(18);

								if (voteType.equals("AddBots"_jrs))
								{
									Jupiter::ReferenceString victimToken = getToken(7);
									RenX::TeamType victim;
									if (teamToken.equals("Global"))
										victim = TeamType::None;
									else if (teamToken.equals("GDI"))
										victim = TeamType::GDI;
									else if (teamToken.equals("Nod"))
										victim = TeamType::Nod;
									else
										victim = TeamType::Other;

									int amount = getToken(9).asInt(10);
									int skill = getToken(11).asInt(10);

									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteAddBots(*this, team, *player, victim, amount, skill);
									}
								}
								else if (voteType.equals("ChangeMap"_jrs))
								{
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteChangeMap(*this, team, *player);
									}
								}
								else if (voteType.equals("Kick"_jrs))
								{
									RenX::PlayerInfo *victim = parseGetPlayerOrAdd(getToken(7));
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteKick(*this, team, *player, *victim);
									}
								}
								else if (voteType.equals("MineBan"_jrs))
								{
									RenX::PlayerInfo *victim = parseGetPlayerOrAdd(getToken(7));
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteMineBan(*this, team, *player, *victim);
									}
								}
								else if (voteType.equals("RemoveBots"_jrs))
								{
									Jupiter::ReferenceString victimToken = getToken(7);
									RenX::TeamType victim;
									if (teamToken.equals("Global"))
										victim = TeamType::None;
									else if (teamToken.equals("GDI"))
										victim = TeamType::GDI;
									else if (teamToken.equals("Nod"))
										victim = TeamType::Nod;
									else
										victim = TeamType::Other;

									int amount = getToken(9).asInt(10);

									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteRemoveBots(*this, team, *player, victim, amount);
									}
								}
								else if (voteType.equals("RestartMap"_jrs))
								{
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteRestartMap(*this, team, *player);
									}
								}
								else if (voteType.equals("Surrender"_jrs))
								{
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteSurrender(*this, team, *player);
									}
								}
								else if (voteType.equals("Survey"_jrs))
								{
									const Jupiter::ReadableString &text = getToken(7);
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteSurvey(*this, team, *player, text);
									}
								}
								else
								{
									voteType.shiftLeft(18);
									for (const auto& plugin : xPlugins) {
										plugin->RenX_OnVoteOther(*this, team, voteType, *player);
									}
								}
							}
							else {
								for (const auto& plugin: xPlugins) {
									plugin->RenX_OnVoteOther(*this, team, voteType, *player);
								}
							}
						}
						onAction();
					}
					else if (subHeader.equals("Results;"))
					{
						// TeamType="Global" / "GDI" / "Nod" / "" | VoteType="Rx_VoteMenuChoice_"... | Success="pass" / "fail" | "Yes=" Yes votes | "No=" No votes
						Jupiter::ReferenceString voteType = getToken(3);
						Jupiter::ReferenceString teamToken = getToken(2);
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
						if (getToken(4).equals("fail"))
							success = false;

						int yesVotes = 0;
						Jupiter::ReferenceString votes_token = getToken(5);
						if (votes_token.size() > 4)
						{
							votes_token.shiftRight(4);
							yesVotes = votes_token.asInt();
						}

						int noVotes = 0;
						votes_token = getToken(6);
						if (votes_token.size() > 3)
						{
							votes_token.shiftRight(3);
							noVotes = votes_token.asInt();
						}

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnVoteOver(*this, team, voteType, success, yesVotes, noVotes);
						}
					}
					else if (subHeader.equals("Cancelled;"))
					{
						// TeamType="Global" / "GDI" / "Nod" | VoteType="Rx_VoteMenuChoice_"...
						Jupiter::ReferenceString voteType = getToken(3);
						Jupiter::ReferenceString teamToken = getToken(2);
						RenX::TeamType team;
						if (teamToken.equals("Global"))
							team = TeamType::None;
						else if (teamToken.equals("GDI"))
							team = TeamType::GDI;
						else if (teamToken.equals("Nod"))
							team = TeamType::Nod;
						else
							team = TeamType::Other;

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnVoteCancel(*this, team, voteType);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnVote(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("MAP"))
				{
					if (subHeader.equals("Changing;"))
					{
						// Map | Mode="seamless" / "nonseamless"
						Jupiter::ReferenceString map = getToken(2);

						m_match_state = 3;
						if (getToken(3).equals("seamless"))
							m_seamless = true;
						else
							m_seamless = false;

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnMapChange(*this, map, m_seamless);
						}

						m_map = map;
						onMapChange();
					}
					else if (subHeader.equals("Loaded;"))
					{
						// Map
						Jupiter::ReferenceString map = getToken(2);

						m_match_state = 0;
						m_map = map;

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnMapLoad(*this, map);
						}
					}
					else if (subHeader.equals("Start;"))
					{
						// Map
						Jupiter::ReferenceString map = getToken(2);

						m_match_state = 1;
						m_reliable = true;
						m_gameStart = std::chrono::steady_clock::now();
						m_map = map;

						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnMapStart(*this, map);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnMap(*this, raw);
						}
					}
				}
				else if (tokens[0].equals("DEMO"))
				{
					if (subHeader.equals("Record;"))
					{
						// "client request by" | Player
						// "admin command by" | Player
						// "rcon command"
						Jupiter::ReferenceString type = getToken(2);
						if (type.equals("client request by") || type.equals("admin command by"))
						{
							RenX::PlayerInfo *player = parseGetPlayerOrAdd(getToken(3));
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnDemoRecord(*this, *player);
							}
						}
						else
						{
							Jupiter::ReferenceString user = getToken(3); // not actually used, but here for possible future usage
							for (const auto& plugin : xPlugins) {
								plugin->RenX_OnDemoRecord(*this, user);
							}
						}
					}
					else if (subHeader.equals("RecordStop;"))
					{
						// Empty
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnDemoRecordStop(*this);
						}
					}
					else
					{
						Jupiter::ReferenceString raw = gotoToken(1);
						for (const auto& plugin : xPlugins) {
							plugin->RenX_OnDemo(*this, raw);
						}
					}
				}
				/*else if (tokens[0].equals("ERROR;")) // Decided to disable this entirely, since it's unreachable anyways.
				{
					// Should be under RCON.
					// "Could not open TCP Port" Port "- Rcon Disabled"
				}*/
				else
				{
					Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
					for (const auto& plugin : xPlugins) {
						plugin->RenX_OnLog(*this, raw);
					}
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
				for (const auto& plugin : xPlugins) {
					plugin->RenX_OnCommand(*this, raw);
				}
				m_commandListFormat.clear();
				m_lastCommand = Jupiter::ReferenceString::empty;
				m_lastCommandParams = Jupiter::ReferenceString::empty;
			}
			break;

		case 'e':
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				for (const auto& plugin : xPlugins) {
					plugin->RenX_OnError(*this, raw);
				}
			}
			break;

		case 'v':
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				m_rconVersion = raw.asInt(10);
				
				if (m_rconVersion >= 3)
				{
					if (m_rconVersion == 3) // Old format: 003Open Beta 5.12
						m_gameVersion = raw.substring(3);
					else // New format: 004 | Game Version Number | Game Version
					{
						m_gameVersionNumber = getToken(1).asInt(10);
						m_gameVersion = getToken(2);

						if (m_gameVersion.isEmpty())
						{
							sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from Renegade-X server (Protocol Error)."));
							disconnect(RenX::DisconnectReason::ProtocolError);
							break;
						}
					}

					sendSocket("s\n"_jrs);
					send("serverinfo"_jrs);
					send("gameinfo"_jrs);
					send("gameinfo bIsCompetitive"_jrs);
					send("mutatorlist"_jrs);
					send("rotation"_jrs);
					fetchClientList();
					updateBuildingList();
					send("ping srv_init_done"_jrs);

					m_gameStart = std::chrono::steady_clock::now();
					m_seamless = true;

					for (const auto& plugin : xPlugins) {
						plugin->RenX_OnVersion(*this, raw);
					}
				}
				else
				{
					sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from Renegade-X server (incompatible RCON version)."));
					disconnect(RenX::DisconnectReason::IncompatibleVersion);
				}
			}
			break;

		case 'a':
			{
				m_rconUser = Jupiter::ReferenceString::substring(line, 1);
				if (m_rconUser.equals(RenX::DevBotName))
					m_devBot = true;

				for (const auto& plugin : xPlugins) {
					plugin->RenX_OnAuthorized(*this, m_rconUser);
				}
			}
			break;

		default:
			{
				Jupiter::ReferenceString raw = Jupiter::ReferenceString::substring(line, 1);
				for (const auto& plugin : xPlugins) {
					plugin->RenX_OnOther(*this, header, raw);
				}
			}
			break;
		}
		for (const auto& plugin : xPlugins) {
			plugin->RenX_OnRaw(*this, line);
		}
	}
}

void RenX::Server::disconnect(RenX::DisconnectReason reason) {
	m_connected = false;

	for (const auto& plugin : RenX::getCore()->getPlugins()) {
		plugin->RenX_OnServerDisconnect(*this, reason);
	}

	m_sock.close();
	wipeData();
}

bool RenX::Server::connect() {
	m_lastAttempt = std::chrono::steady_clock::now();
	if (m_sock.connect(m_hostname.c_str(), m_port, m_clientHostname.empty() ? nullptr : m_clientHostname.c_str()))
	{
		m_sock.setBlocking(false);
		sendSocket(Jupiter::StringS::Format("a%.*s\n", m_pass.size(), m_pass.ptr()));
		m_connected = true;
		m_attempts = 0;
		return true;
	}
	m_connected = false;
	++m_attempts;
	return false;
}

bool RenX::Server::reconnect(RenX::DisconnectReason reason) {
	disconnect(static_cast<RenX::DisconnectReason>(static_cast<unsigned int>(reason) | 0x01));
	return connect();
}

void RenX::Server::wipeData() {
	wipePlayers();
	m_reliable = false;
	m_team_mode = 3;
	m_game_type = 1;
	m_ranked = false;
	m_botsEnabled = true;
	m_match_state = 1;
	m_subscribed = false;
	m_fully_connected = false;
	m_bot_count = 0;
	m_player_rdns_resolutions_pending = 0;
	this->buildings.clear();
	this->mutators.clear();
	this->maps.clear();
	m_awaitingPong = false;
	m_rconVersion = 0;
	m_rconUser.truncate(m_rconUser.size());
}

void RenX::Server::wipePlayers() {
	while (this->players.size() != 0) {
		for (const auto& plugin : RenX::getCore()->getPlugins()) {
			plugin->RenX_OnPlayerDelete(*this, this->players.front());
		}

		if (this->players.front().rdns_thread.joinable()) { // Close the RDNS thread, if one exists
			--m_player_rdns_resolutions_pending;
			this->players.front().rdns_thread.join();
		}

		this->players.pop_front();
	}
}

void RenX::Server::startPing() {
	m_lastActivity = std::chrono::steady_clock::now();
	sendSocket("cping\n"_jrs);
	m_awaitingPong = true;
}

unsigned int RenX::Server::getVersion() const {
	return m_rconVersion;
}

unsigned int RenX::Server::getGameVersionNumber() const {
	return m_gameVersionNumber;
}

const Jupiter::ReadableString &RenX::Server::getGameVersion() const {
	return m_gameVersion;
}

std::string_view RenX::Server::getRCONUsername() const {
	return m_rconUser;
}

RenX::Server::Server(Jupiter::Socket &&socket, const Jupiter::ReadableString &configurationSection) : Server(configurationSection) {
	m_sock = std::move(socket);
	m_hostname = m_sock.getRemoteHostname();
	sendSocket(Jupiter::StringS::Format("a%.*s\n", m_pass.size(), m_pass.ptr()));
	m_connected = true;
}

RenX::Server::Server(const Jupiter::ReadableString &configurationSection) {
	m_configSection = configurationSection;
	m_calc_uuid = RenX::default_uuid_func;
	init(*RenX::getCore()->getConfig().getSection(m_configSection));
	for (const auto& plugin : RenX::getCore()->getPlugins()) {
		plugin->RenX_OnServerCreate(*this);
	}
}

void RenX::Server::init(const Jupiter::Config &config) {
	m_hostname = static_cast<std::string>(config.get("Hostname"_jrs, "localhost"_jrs));
	m_port = config.get<unsigned short>("Port"_jrs, 7777);
	m_clientHostname = static_cast<std::string>(config.get("ClientAddress"_jrs));
	m_pass = config.get("Password"_jrs, "renx"_jrs);

	m_logChanType = config.get<int>("ChanType"_jrs);
	m_adminLogChanType = config.get<int>("AdminChanType"_jrs);

	setCommandPrefix(config.get("CommandPrefix"_jrs));
	setPrefix(config.get("IRCPrefix"_jrs));

	m_ban_from_str = config.get("BanFromStr"_jrs, "the server"_jrs);
	m_rules = config.get("Rules"_jrs, "Anarchy!"_jrs);
	m_delay = std::chrono::milliseconds(config.get<long long>("ReconnectDelay"_jrs, 10000));
	m_maxAttempts = config.get<int>("MaxReconnectAttempts"_jrs, -1);
	m_rconBan = config.get<bool>("RCONBan"_jrs, false);
	m_localSteamBan = config.get<bool>("LocalSteamBan"_jrs, true);
	m_localIPBan = config.get<bool>("LocalIPBan"_jrs, true);
	m_localHWIDBan = config.get<bool>("LocalHWIDBan"_jrs, true);
	m_localRDNSBan = config.get<bool>("LocalRDNSBan"_jrs, false);
	m_localNameBan = config.get<bool>("LocalNameBan"_jrs, false);
	m_localBan = m_localIPBan || m_localRDNSBan || m_localSteamBan || m_localNameBan;
	m_steamFormat = config.get<int>("SteamFormat"_jrs, 16);
	m_neverSay = config.get<bool>("NeverSay"_jrs, false);
	m_resolve_player_rdns = config.get<bool>("ResolvePlayerRDNS"_jrs, true);
	m_clientUpdateRate = std::chrono::milliseconds(config.get<long long>("ClientUpdateRate"_jrs, 2500));
	m_buildingUpdateRate = std::chrono::milliseconds(config.get<long long>("BuildingUpdateRate"_jrs, 7500));
	m_pingRate = std::chrono::milliseconds(config.get<long long>("PingUpdateRate"_jrs, 60000));
	m_pingTimeoutThreshold = std::chrono::milliseconds(config.get<long long>("PingTimeoutThreshold"_jrs, 10000));

	Jupiter::Config &commandsFile = RenX::getCore()->getCommandsFile();
	m_commandAccessLevels = commandsFile.getSection(m_configSection);
	m_commandAliases = commandsFile.getSection(m_configSection + ".Aliases"_jrs);

	for (const auto& command : RenX::GameMasterCommandList) {
		addCommand(command->copy());
	}

	auto load_basic_commands = [this, &commandsFile](const Jupiter::ReadableString &section_prefix)
	{
		Jupiter::Config *basic_commands = commandsFile.getSection(section_prefix + ".Basic"_jrs);
		if (basic_commands != nullptr)
		{
			Jupiter::Config *basic_commands_help = commandsFile.getSection(section_prefix + ".Basic.Help"_jrs);

			for (auto& command : basic_commands->getTable()) {
				if (getCommand(command.first) == nullptr) {
					const auto& help_str = basic_commands_help == nullptr ? ""_jrs : basic_commands_help->get(command.second, ""_jrs);
					addCommand(new RenX::BasicGameCommand(command.first, command.second,help_str));
				}
			}
		}
	};

	load_basic_commands(m_configSection);
	load_basic_commands("Default"_jrs);
}

RenX::Server::~Server() {
	// TODO: This does nothing
	if (RenX::GameCommand::selected_server == nullptr)
		RenX::GameCommand::selected_server = nullptr;
	if (RenX::GameCommand::active_server == nullptr)
		RenX::GameCommand::active_server = RenX::GameCommand::selected_server;

	m_sock.close();
	wipeData();
}
