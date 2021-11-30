/**
 * Copyright (C) 2021 Jessica James. All rights reserved.
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include "RenX_Relay.h"
#include <regex>
#include <random>
#include <memory>
#include <string_view>
#include <unordered_set>
#include <cassert>
#include <fstream>
#include "fmt/format.h" // TODO: replace with <format> later
#include <charconv>
#include "jessilib/split.hpp"
#include "jessilib/word_split.hpp"
#include "Jupiter/IRC.h"
#include "RenX_Functions.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"

// String literal redefinition of RenX::DelimC
#define RX_DELIM "\x02"

using namespace Jupiter::literals;
using namespace std::literals;
constexpr const char g_blank_steamid[] = "0x0000000000000000";
constexpr std::chrono::steady_clock::duration g_reconnect_delay = std::chrono::seconds{15 }; // game server: 120s
constexpr std::chrono::steady_clock::duration g_activity_timeout = std::chrono::seconds{ 120 }; // game server: 120s

int RenX_RelayPlugin::think() {
	for (auto& server_pair : m_server_info_map) {
		auto server = server_pair.first;
		for (auto& server_info : server_pair.second) {
			auto& upstream_socket = server_info.m_socket;
			const auto& upstream_name = get_upstream_name(server_info);

			if (!upstream_socket) {
				// This should never happen
				continue;
			}

			if (!server_info.m_connected) {
				// Not connected; attempt retry if needed
				if (std::chrono::steady_clock::now() >= server_info.m_last_connect_attempt + g_reconnect_delay) {
					if (upstream_socket->connect(server_info.m_settings->m_upstream_hostname.c_str(), server_info.m_settings->m_upstream_port)) {
						// There's some handshake garbage that needs to go on here so the upstream accepts us
						upstream_connected(*server, server_info);
						server->sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to %.*s; game server now listed.", upstream_name.size(), upstream_name.data());
					}
					else {
						server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to %.*s; game server not listed.", upstream_name.size(), upstream_name.data());
					}

					// Update our timings
					server_info.m_last_connect_attempt = std::chrono::steady_clock::now();
					server_info.m_last_activity = server_info.m_last_connect_attempt;
				}
			}
			else if (std::chrono::steady_clock::now() - server_info.m_last_activity >= g_activity_timeout) { // ping timeout
				// Ping timeout; disconnect immediately
				server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from %.*s (ping timeout); game server is no longer listed.", upstream_name.size(), upstream_name.data());
				upstream_disconnected(*server, server_info);
			}
			else {
				// Connected and fine
				if (upstream_socket->recv() > 0) { // Data received
					auto tokens = jessilib::split_view(upstream_socket->getBuffer(), '\n');
					if (!tokens.empty()) {
						server_info.m_last_activity = std::chrono::steady_clock::now();
						server_info.m_last_line += tokens[0];
						if (tokens.size() != 1) {
							// Process upstream message received
							process_upstream_message(server, server_info.m_last_line, server_info);
							server_info.m_last_line = tokens[tokens.size() - 1];

							for (size_t index = 1; index != tokens.size() - 1; ++index) {
								process_upstream_message(server, tokens[index], server_info);
							}
						}
					}
				}
				else if (Jupiter::Socket::getLastError() == JUPITER_SOCK_EWOULDBLOCK) { // Operation would block (no new data)
					if (std::chrono::steady_clock::now() - server_info.m_last_activity >= g_activity_timeout) {
						upstream_disconnected(*server, server_info);
					}
				}
				else { // This is a serious error
					upstream_disconnected(*server, server_info);

					server->sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection to %.*s lost. Reconnection attempt in progress.", upstream_name.size(), upstream_name.data());
					if (upstream_socket->connect(server_info.m_settings->m_upstream_hostname.c_str(), server_info.m_settings->m_upstream_port)) {
						upstream_connected(*server, server_info);
						server->sendLogChan(IRCCOLOR "06[Progress]" IRCCOLOR " Connection to %.*s reestablished. Initializing Renegade-X RCON protocol...", upstream_name.size(), upstream_name.data());
					}
					else {
						server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to %.*s lost. Reconnection attempt failed.", upstream_name.size(), upstream_name.data());
					}

					// Update our timings
					server_info.m_last_connect_attempt = std::chrono::steady_clock::now();
					server_info.m_last_activity = server_info.m_last_connect_attempt;

					return 0;
				}
			}
		}
	}

	return 0;
}

bool noop_handler(std::string_view, RenX::Server&, std::vector<std::string>&) {
	return true;
}

bool handle_ping(std::string_view in_command_line, RenX::Server&, std::vector<std::string>& out_response) {
	std::string pong_message;
	pong_message.reserve(in_command_line.size() + 1);
	pong_message = "PONG"sv;
	pong_message += RenX::DelimC;
	if (in_command_line.size() > pong_message.size()) {
		pong_message += in_command_line.substr(5);
	}
	out_response.push_back(std::move(pong_message));
	return true;
}

static const std::unordered_set<std::string_view> g_known_commands {
	"addmap"sv, "amsg"sv, "botlist"sv, "botvarlist"sv, "buildinginfo"sv, "binfo"sv, "buildinglist"sv, "blist"sv,
	"cancelvote"sv, "votestop"sv, "changemap"sv, "setmap"sv, "changename"sv, "changeplayername"sv, "clientlist"sv,
	"clientvarlist"sv, "disarm"sv, "disarmbeacon"sv, "disarmb"sv, "disarmc4"sv, "dumpkilllog"sv, "dumpkills"sv,
	"endmap"sv, "gameover"sv, "endgame"sv, "fkick"sv, "forcekick"sv, "forcenonseamless"sv, "forceseamless"sv,
	"gameinfo"sv, "ginfo"sv, "hascommand"sv, "help"sv, "hostprivatesay"sv, "page"sv, "hostsay"sv, "say"sv, "kick"sv,
	"kickban"sv, "kill"sv, "listmutators"sv, "listmutator"sv, "mutatorlist"sv, "mutatorslist"sv, "loadmutator"sv,
	"mutatorload"sv, "lockbuildings"sv, "lockhealth"sv, "lockb"sv, "lockh"sv, "lb"sv, "makeadmin"sv, "map"sv, "getmap"sv,
	"mineban"sv, "mban"sv, "minelimit"sv, "mlimit"sv, "mineunban"sv, "unmineban"sv, "munban"sv, "unmban"sv,
	"mutateasnone"sv, "mutateasplayer"sv, "normalmode"sv, "nmode"sv, "ping"sv, "playerinfo"sv, "pamsg"sv, "recorddemo"sv,
	"demorecord"sv, "demorec"sv, "removemap"sv, "rotation"sv, "serverinfo"sv, "sinfo"sv, "setcommander"sv,
	"spectatemode"sv, "smode"sv, "swapteams"sv, "teamswap"sv, "team"sv, "changeteam"sv, "team2"sv, "changeteam2"sv,
	"teaminfo"sv, "tinfo"sv, "textmute"sv, "mute"sv, "textunmute"sv, "unmute"sv, "togglebotvoice"sv, "mutebot"sv,
	"mutebots"sv, "unmutebot"sv, "unmutebots"sv, "cheatbots"sv, "togglecheatbots"sv, "switchcheatbots"sv, "forcebots"sv,
	"toggleforcebots"sv, "switchforcebots"sv, "togglesuspect"sv, "travel"sv, "removemutator"sv, "mutatorremove"sv,
	"unloadmutator"sv, "mutatorunload"sv, "vehiclelimit"sv, "vlimit"sv, "warn"sv
};

static const std::unordered_set<std::string_view> g_whitelist_commands {
	"map"sv, "help"sv, "playerinfo"sv, "sinfo"sv, "teaminfo"sv, "hascommand"sv, "buildinglist"sv, "blist"sv,
	"clientvarlist"sv, "getmap"sv, "buildinginfo"sv, "botlist"sv, "vlimit"sv, "serverinfo"sv, "ginfo"sv, "rotation"sv,
	"binfo"sv, "vehiclelimit"sv, "tinfo"sv, "botvarlist"sv, "minelimit"sv, "gameinfo"sv, "clientlist"sv, "mlimit"sv,
	"ping"sv,
};

static const std::unordered_set<std::string_view> g_blacklist_commands {
	"addmap"sv, "admin"sv, "amsg"sv, "cancelvote"sv, "votestop"sv, "changemap"sv, "setmap"sv, "changename"sv,
	"changeplayername"sv, "disarm"sv, "disarmbeacon"sv, "disarmb"sv, "disarmc4"sv, "dumpkilllog"sv, "dumpkills"sv,
	"endmap"sv, "gameover"sv, "endgame"sv, "fkick"sv, "forcekick"sv, "forcenonseamless"sv, "forceseamless"sv,
	"hostprivatesay"sv, "page"sv, "hostsay"sv, "say"sv, "kick"sv, "kickban"sv, "kill"sv, "listmutators"sv,
	"listmutator"sv, "mutatorlist"sv, "mutatorslist"sv, "loadmutator"sv, "mutatorload"sv, "lockbuildings"sv,
	"lockhealth"sv, "lockb"sv, "lockh"sv, "lb"sv, "makeadmin"sv, "mineban"sv, "mban"sv, "mineunban"sv, "unmineban"sv,
	"munban"sv, "unmban"sv, "mutateasnone"sv, "mutateasplayer"sv, "normalmode"sv, "nmode"sv, "pamsg"sv, "recorddemo"sv,
	"demorecord"sv, "demorec"sv, "removemap"sv, "setcommander"sv, "spectatemode"sv, "smode"sv, "swapteams"sv,
	"teamswap"sv, "team"sv, "changeteam"sv, "team2"sv, "changeteam2"sv, "textmute"sv, "mute"sv, "textunmute"sv,
	"unmute"sv, "togglebotvoice"sv, "mutebot"sv, "mutebots"sv, "unmutebot"sv, "unmutebots"sv, "cheatbots"sv,
	"togglecheatbots"sv, "switchcheatbots"sv, "forcebots"sv, "toggleforcebots"sv, "switchforcebots"sv, "togglesuspect"sv,
	"travel"sv, "removemutator"sv, "mutatorremove"sv, "unloadmutator"sv, "mutatorunload"sv, "warn"sv
};

bool RenX_RelayPlugin::initialize() {
	m_init_time = std::chrono::steady_clock::now();
	// TODO: add BindHost, BindPort
	// TODO: invent a way to send a custom hostname directly to the devbot, rather than having to rely on manual configuration
	// TODO: Add bidirectional relay support (i.e: bot link)
	// TODO: Add game data relay support (so players connect through this)
	// 			* Drop packets for non-players when traffic spikes
	//			* notify game server to set failover

	m_default_settings = get_settings(config);

	std::string_view upstreams_list = config.get("Upstreams"_jrs, ""sv);
	std::vector<std::string_view> upstream_names = jessilib::word_split_view(upstreams_list, WHITESPACE_SV);
	for (auto upstream_name : upstream_names) {
		auto upstream_config = config.getSection(upstream_name);
		if (upstream_config == nullptr) {
			upstream_config = &config;
		}

		auto upstream_settings = get_settings(*upstream_config);
		upstream_settings.m_label = upstream_name;
		m_configured_upstreams.push_back(std::move(upstream_settings));
	}

	return !m_configured_upstreams.empty() && RenX::Plugin::initialize();
}

void RenX_RelayPlugin::RenX_OnServerFullyConnected(RenX::Server &server) {
	auto& server_infos = m_server_info_map[&server];
	if (server_infos.empty()) {
		for (const auto& settings : m_configured_upstreams) {
			server_infos.emplace_back().m_settings = &settings;
		}
	}

	for (auto& server_info : server_infos) {
		if (!server_info.m_connected) {
			server_info.m_socket = std::unique_ptr<Jupiter::TCPSocket>(new Jupiter::TCPSocket());
			if (server_info.m_socket->connect(server_info.m_settings->m_upstream_hostname.c_str(), server_info.m_settings->m_upstream_port)) {
				upstream_connected(server, server_info);
			}
		}
	}
}

void RenX_RelayPlugin::RenX_OnServerDisconnect(RenX::Server &server, RenX::DisconnectReason reason) {
	auto pair_itr = m_server_info_map.find(&server);
	if (pair_itr != m_server_info_map.end()) {
		for (auto& upstream_info : pair_itr->second) {
			upstream_disconnected(server, upstream_info);
		}
	}

	m_server_info_map.erase(&server);
}

// There's not truly any way to know for certain that a token is a player token without message-specific positional context,
// but the format is just specific enough that there shouldn't be many false positives. For false positives that do occur,
// we likely don't really care anyways, since this is just getting forwarded to the devbot
// maybe this could be improved upon by also verifying the third component truly is a player's name
bool is_player_token(const char* begin, const char* end) {
	std::regex player_token_regex{ "[A-Za-z]*,b?[0-9]+,.+" };
	std::cmatch match_result;
	return std::regex_match(begin, end, match_result, player_token_regex);
}

/** Copied from Rx_TCPLink so that the same formatting bug is included */

const char hexadecimal_rep_table_upper[][3] = {
	"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
	"30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
	"40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
	"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
	"70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
	"80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
	"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
	"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
	"B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
	"C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
	"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
	"E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
	"F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
};

struct HWID {
	union {
		uint64_t hwid;
		struct {
			uint32_t left;
			uint32_t right;
		};
	};
};

template<typename T>
std::string to_hex(T in_integer) {
	std::string result;
	uint8_t* begin = reinterpret_cast<uint8_t*>(&in_integer);
	uint8_t* itr = begin + sizeof(T);

	result.reserve(sizeof(in_integer) * 2);
	while (itr != begin) {
		--itr;
		result += hexadecimal_rep_table_upper[*itr];
	}

	return result;
}

void RenX_RelayPlugin::RenX_OnRaw(RenX::Server &server, std::string_view line) {
	// Not parsing any escape sequences, so data gets sent upstream exactly as it's received here. Copy tokens where needed to process escape sequences.
	auto tokens = jessilib::split(std::string_view{line}, RenX::DelimC);

	// Ensure valid message received
	if (tokens.empty()) {
		return;
	}

	// Check that we already have a session for this server
	auto server_info_map_itr = m_server_info_map.find(&server);
	if (server_info_map_itr == m_server_info_map.end()) {
		// early out: server not yet registered (i.e: finished auth)
		return;
	}

	// Suppress unassociated command execution logs from going upstream
	if (tokens.size() >= 5
		&& tokens[0] == "lRCON"
		&& tokens[1] == "Command;"
		&& tokens[3] == "executed:"
		&& !tokens[4].empty()) {
		if (tokens[2] != server.getRCONUsername()) {
			if (m_default_settings.m_suppress_rcon_command_logs) { // TODO: move away from per-stream settings
				// Suppress RCON command logs from other RCON users
				return;
			}
		}
		else if (m_command_tracker.empty()) {
			// This command response wasn't requested by any current upstream connections; suppress it
			return;
		}
		else {
			upstream_server_info* front_server_info = m_command_tracker.front();
			if (front_server_info == nullptr
				|| front_server_info->m_processing_command
				|| tokens[4] != front_server_info->m_response_queue.front().m_command) {
				// This command response wasn't requested by any current upstream connections; suppress it
				return;
			}

			// This is the next command we're been waiting on; mark processing command and let this go through
			front_server_info->m_processing_command = true;

			// This is a command response for an upstream command; this is only relevant to one server: that server
			// Always echo the command back exactly as it was sent
			Jupiter::StringS line_sanitized;
			const auto& rcon_username = get_upstream_rcon_username(*front_server_info, server);
			if (rcon_username == server.getRCONUsername()) {
				// No need to recombine tokens
				line_sanitized = line;
			}
			else {
				// TODO: add assignment operators to Jupiter::string crap
				tokens[2] = rcon_username;

				// Construct line to send and send it
				line_sanitized = tokens[0];
				for (size_t index = 1; index != tokens.size(); ++index) {
					line_sanitized += RenX::DelimC;
					line_sanitized += tokens[index];
				}
			}
			line_sanitized += '\n';

			send_upstream(*front_server_info, line_sanitized, server);
			return;
		}
	}

	for (auto& server_info : server_info_map_itr->second) {
		// Pass tokens by copy so sanitizations can differ per-upstream
		// TODO: optimize to only copy when needed
		// TODO: only generate sanitized message once if all upstreams are configured the same (i.e: all match default)
		process_renx_message(server, server_info, line, tokens);
	}
}

void RenX_RelayPlugin::process_renx_message(RenX::Server& server, upstream_server_info& server_info, std::string_view line, std::vector<std::string> tokens) {
	bool required_sanitization = false;
	if (!server_info.m_socket) {
		// early out: no upstream RCON session
		return;
	}

	const upstream_settings& settings = *server_info.m_settings;
	if (settings.m_suppress_chat_logs
		&& tokens[0] == "lCHAT") {
		return;
	}

	// Suppress unassociated command responses from going upstream
	if (!tokens[0].empty()
		&& (tokens[0][0] == 'r' || tokens[0][0] == 'c')
		&& !server_info.m_processing_command) {
		// This command response wasn't requested upstream; suppress it
		return;
	}

	auto findPlayerByIP = [&server](std::string in_ip) -> const RenX::PlayerInfo* {
		// Parse into integer so we're doing int comparisons instead of strings
		auto ip32 = Jupiter::Socket::pton4(in_ip.c_str());
		if (ip32 == 0) {
			return nullptr;
		}

		// Search players
		for (const auto& player : server.players) {
			if (player.ip32 == ip32) {
				return &player;
			}
		}

		return nullptr;
	};

	auto findPlayerByHWID = [&server](std::string_view in_hwid) -> const RenX::PlayerInfo* {
		if (in_hwid.empty()) {
			return nullptr;
		}

		for (const auto& player : server.players) {
			if (player.hwid == in_hwid) {
				return &player;
			}
		}

		return nullptr;
	};

	auto findPlayerBySteamID = [&server](std::string_view in_steamid) -> const RenX::PlayerInfo* {
		uint64_t steamid{};
		std::from_chars(in_steamid.data(), in_steamid.data() + in_steamid.size(), steamid);
		if (steamid == 0) {
			return nullptr;
		}

		for (const auto& player : server.players) {
			if (player.steamid == steamid) {
				return &player;
			}
		}

		return nullptr;
	};

	if (settings.m_sanitize_names) {
		for (size_t index = 0; index != tokens.size(); ++index) {
			auto& token = tokens[index];
			if (is_player_token(token.data(), token.data() + token.size())) {
				// Get token pieces
				auto player_tokens = jessilib::split_n_view(token, ',', 2);
				std::string_view idToken = player_tokens[1];

				std::string replacement_player = static_cast<std::string>(player_tokens[0]);
				replacement_player += ',';
				replacement_player += idToken;
				replacement_player += ',';
				if (!idToken.empty() && idToken.front() == 'b') {
					idToken.remove_prefix(1);
				}

				// Name (sanitized)
				replacement_player += "Player";
				replacement_player += idToken;

				token = replacement_player;
				required_sanitization = true;
			}
		}
	}

	if (settings.m_sanitize_ips || settings.m_sanitize_hwids) {
		// It's way too much of a pain to check for command usages, and it's not full-proof either (an alias or similar could be added).
		// So instead, we'll just search all messages for any tokens containing any player's IP or HWID.
		// This isn't terribly efficient, but there's only up to 64 players, so not a huge concern
		for (size_t index = 0; index != tokens.size(); ++index) {
			auto& token = tokens[index];
			const RenX::PlayerInfo* player;
			if (settings.m_sanitize_ips) {
				player = findPlayerByIP(token);
				if (player != nullptr) {
					// Initialize the engine here using the init time, so that player fake IPs will be consistent
					// Also include player ID so we get different IPs between players and for each match
					std::mt19937_64 randgen(m_init_time.time_since_epoch().count() + (player->id * 2));
					std::uniform_int_distribution<uint32_t> dist(10, 200);

					// Replace real IP with fake
					//token.format("%u.%u.%u.%u",
					token = fmt::format("{}.{}.{}.{}",
						static_cast<unsigned int>(dist(randgen)),
						static_cast<unsigned int>(dist(randgen)),
						static_cast<unsigned int>(dist(randgen)),
						static_cast<unsigned int>(dist(randgen)));

					required_sanitization = true;
					continue;
				}
			}

			if (settings.m_sanitize_hwids) {
				player = findPlayerByHWID(token);
				if (player != nullptr) {
					// Initialize the engine here using the init time, so that player fake HWIDs will be consistent
					// Also include player ID so we get different HWIDs between players and for each match
					std::mt19937_64 randgen(m_init_time.time_since_epoch().count() + (player->id * 2 + 69));
					std::uniform_int_distribution<uint64_t> dist(0, 0x0000FFFFFFFFFFFFULL);

					HWID hwid{};
					hwid.hwid = dist(randgen);

					token = "m";
					token += to_hex(hwid.left) + to_hex(hwid.right);

					required_sanitization = true;
					continue;
				}
			}

			if (settings.m_sanitize_steam_ids) {
				player = findPlayerBySteamID(token);
				if (player != nullptr) {
					token = g_blank_steamid;
					continue;
				}
			}

			// More sanitization checks here...
		}
	}

	std::string line_sanitized;
	if (required_sanitization) {
		// Construct line to send and send it
		line_sanitized = tokens[0];
		for (size_t index = 1; index != tokens.size(); ++index) {
			line_sanitized += RenX::DelimC;
			line_sanitized += tokens[index];
		}
	}
	else {
		// Forward line without modification
		line_sanitized = line;
	}

	line_sanitized += '\n';
	send_upstream(server_info, line_sanitized, server);

	if (line_sanitized[0] == 'c'
		&& server_info.m_processing_command) {
		auto& queue = server_info.m_response_queue;
		server_info.m_processing_command = false;

		if (queue.empty()) {
			std::cerr << "COMMAND FINISHED PROCESSING ON EMPTY QUEUE" << std::endl;
			return;
		}

		assert(m_command_tracker.front() == &server_info);

		// We've finished executing a real command; pop it and go through any pending fakes
		queue.pop_front();
		m_command_tracker.pop_front();

		std::string response;
		while (!queue.empty() && queue.front().m_is_fake) {
			response = queue.front().to_rcon(get_upstream_rcon_username(server_info, server));
			send_upstream(server_info, response, server);
			queue.pop_front();
		}
	}
}

RenX_RelayPlugin::upstream_settings RenX_RelayPlugin::get_settings(const Jupiter::Config& in_config) {
	upstream_settings result{};

	// Read in settings
	result.m_upstream_hostname = in_config.get<std::string>("UpstreamHost"_jrs, m_default_settings.m_upstream_hostname);
	result.m_upstream_port = in_config.get<uint16_t>("UpstreamPort"_jrs, m_default_settings.m_upstream_port);
	result.m_rcon_username = in_config.get<std::string>("RconUsername"_jrs, m_default_settings.m_rcon_username);
	result.m_log_traffic = in_config.get<bool>("LogTraffic"_jrs, m_default_settings.m_log_traffic);
	result.m_fake_pings = in_config.get<bool>("FakePings"_jrs, m_default_settings.m_fake_pings);
	result.m_fake_ignored_commands = in_config.get<bool>("FakeSuppressedCommands"_jrs, m_default_settings.m_fake_ignored_commands);
	result.m_sanitize_names = in_config.get<bool>("SanitizeNames"_jrs, m_default_settings.m_sanitize_names);
	result.m_sanitize_ips = in_config.get<bool>("SanitizeIPs"_jrs, m_default_settings.m_sanitize_ips);
	result.m_sanitize_hwids = in_config.get<bool>("SanitizeHWIDs"_jrs, m_default_settings.m_sanitize_hwids);
	result.m_sanitize_steam_ids = in_config.get<bool>("SanitizeSteamIDs"_jrs, m_default_settings.m_sanitize_steam_ids);
	result.m_suppress_unknown_commands = in_config.get<bool>("SuppressUnknownCmds"_jrs, m_default_settings.m_suppress_unknown_commands);
	result.m_suppress_blacklisted_commands = in_config.get<bool>("SuppressBlacklistedCmds"_jrs, m_default_settings.m_suppress_blacklisted_commands);
	result.m_suppress_chat_logs = in_config.get<bool>("SuppressChatLogs"_jrs, m_default_settings.m_suppress_chat_logs);
	result.m_suppress_rcon_command_logs = in_config.get<bool>("SuppressRconCommandLogs"_jrs, m_default_settings.m_suppress_rcon_command_logs);

	// Populate fake command handlers
	if (result.m_fake_pings) {
		result.m_fake_command_table.emplace("ping", &handle_ping);
	}

	if (result.m_fake_ignored_commands
		&& result.m_suppress_blacklisted_commands) {
		for(auto& command : g_blacklist_commands) {
			result.m_fake_command_table.emplace(command, &noop_handler);
		}
	}

	return result;
}

std::string RenX_RelayPlugin::get_log_filename(RenX::Server& in_server, const upstream_server_info& in_server_info) {
	return { "log__"s
		+ in_server_info.m_settings->m_label
		+ "__" + in_server.getSocketHostname()
		+ "__" + std::to_string(in_server.getPort())
		+ ".txt" };
}

std::string_view RenX_RelayPlugin::get_upstream_name(const upstream_server_info& in_server_info) {
	return in_server_info.m_settings->m_upstream_hostname; // Will point to stream-specific name later
}

std::string_view RenX_RelayPlugin::get_upstream_rcon_username(const upstream_server_info& in_server_info, RenX::Server& in_server) {
	const auto& rcon_username = in_server_info.m_settings->m_rcon_username;
	if (rcon_username.empty()) {
		const auto& real_rcon_username = in_server.getRCONUsername();
		return real_rcon_username;
	}

	return rcon_username;
}

int RenX_RelayPlugin::send_upstream(upstream_server_info& in_server_info, std::string_view in_message, RenX::Server& in_server) {
	if (in_server_info.m_settings->m_log_traffic) {
		std::ofstream log_file{ get_log_filename(in_server, in_server_info), std::ios::out | std::ios::app | std::ios::binary };
		if (log_file) {
			log_file << '[' << getTimeFormat("%H:%M:%S") << "] (Jupiter -> " << in_server_info.m_settings->m_label << "): " << in_message << std::endl;
		}
	}

	return in_server_info.m_socket->send(in_message.data(), in_message.size());
}

int RenX_RelayPlugin::send_downstream(RenX::Server& in_server, std::string_view in_message, upstream_server_info& in_server_info) {
	if (in_server_info.m_settings->m_log_traffic) {
		std::ofstream log_file{ get_log_filename(in_server, in_server_info), std::ios::out | std::ios::app | std::ios::binary };
		if (log_file) {
			log_file << '[' << getTimeFormat("%H:%M:%S") << "] (" << in_server_info.m_settings->m_label << " -> RenX::Server): " << in_message << std::endl;
		}
	}

	return in_server.sendData(in_message);
}

void RenX_RelayPlugin::upstream_connected(RenX::Server& in_server, upstream_server_info& in_server_info) {
	in_server_info.m_connected = true;
	in_server_info.m_socket->setBlocking(false);
	in_server_info.m_last_connect_attempt = std::chrono::steady_clock::now();
	in_server_info.m_last_activity = in_server_info.m_last_connect_attempt;

	// New format: 004 | Game Version Number | Game Version
	const auto& version_str = in_server.getGameVersion();
	std::string version_message = "v004";
	version_message += RenX::DelimC;
	version_message += std::to_string(in_server.getGameVersionNumber());
	version_message += RenX::DelimC;
	version_message.append(version_str.data(), version_str.size());
	version_message += '\n';

	// Tack on username auth
	const auto& rcon_username = get_upstream_rcon_username(in_server_info, in_server);
	version_message += 'a';
	version_message += rcon_username;
	version_message += '\n';

	send_upstream(in_server_info, version_message, in_server);
}

void RenX_RelayPlugin::upstream_disconnected(RenX::Server&, upstream_server_info& in_server_info) {
	in_server_info.m_connected = false;

	// Clear references to `in_server_info`
	for (auto& server_ptr : m_command_tracker) {
		if (server_ptr == &in_server_info) {
			server_ptr = nullptr;
		}
	}

	if (in_server_info.m_socket) {
		in_server_info.m_socket->close();
	}
}

void RenX_RelayPlugin::process_upstream_message(RenX::Server* in_server, std::string_view in_line, upstream_server_info& in_server_info) {
	if (in_line.empty()) {
		return;
	}

	if (in_line.front() == 's') {
		// we're already subscribed
		return;
	}

	if (in_line.front() == 'a') {
		// we're already authenticated
		return;
	}

	auto queue_command = [&in_server_info, this](UpstreamCommand&& in_command) {
		if (!in_command.m_is_fake) {
			m_command_tracker.push_back(&in_server_info);
		}

		in_server_info.m_response_queue.push_back(std::move(in_command));
	};

	const upstream_settings& settings = *in_server_info.m_settings;

	// Sanitize unknown & blacklisted commands
	if (in_line[0] == 'c' && in_line.size() > 1) {
		// Sanitize unknown & blacklisted commands
		if (settings.m_suppress_unknown_commands || (settings.m_suppress_blacklisted_commands && !settings.m_fake_ignored_commands)) {
			std::string_view command_view = jessilib::split_once_view(in_line, ' ').first.substr(1);
			if (settings.m_suppress_unknown_commands
				&& g_known_commands.find(command_view) == g_known_commands.end()) {
				// Command not in known commands list; ignore it
				if (settings.m_fake_ignored_commands) {
					// Queue a fake response if necessary
					UpstreamCommand command;
					command.m_command = command_view;
					command.m_is_fake = true;
					command.m_response.push_back("Non-existent RconCommand - executed as ConsoleCommand"s);

					// Push upstream or to queue
					if (in_server_info.m_response_queue.empty()) {
						// No commands are in the queue; go ahead and shove back the response
						auto response = command.to_rcon(get_upstream_rcon_username(in_server_info, *in_server));
						send_upstream(in_server_info, response, *in_server);
						return;
					}

					// Other commands are waiting in the queue; tack this to the end to ensure command order
					queue_command(std::move(command));
					return;
				}
				return;
			}

			if (settings.m_suppress_blacklisted_commands && !settings.m_fake_ignored_commands
				&& g_blacklist_commands.find(command_view) != g_blacklist_commands.end()) {
				// Command is blacklisted and not gonna be faked out; ignore it
				return;
			}
		}

		std::string_view command_line = in_line.substr(1);
		std::string_view command_word = command_line.substr(0, std::min(command_line.find(' '), command_line.size()));
		std::string command_word_lower;
		command_word_lower.reserve(command_word.size());
		std::transform(command_word.begin(), command_word.end(), std::back_inserter(command_word_lower),
			static_cast<int(*)(int)>(std::tolower));

		// Populate any fake responses (i.e: pings)
		UpstreamCommand command;
		command.m_command = command_line;
		auto handler = settings.m_fake_command_table.find(command_word_lower);
		if (handler != settings.m_fake_command_table.end()) {
			// Execute fake command
			command.m_is_fake = handler->second(command_line, *in_server, command.m_response);
			if (command.m_is_fake) {
				if (in_server_info.m_response_queue.empty()) {
					// No commands are in the queue; go ahead and shove back the response
					auto response = command.to_rcon(get_upstream_rcon_username(in_server_info, *in_server));
					send_upstream(in_server_info, response, *in_server);
					return;
				}

				// Other commands are waiting in the queue; tack this to the end to ensure command order
				queue_command(std::move(command));
				return;
			}
		}

		// This is not a fake command; queue it and send it
		queue_command(std::move(command));
	}

	// Send line to game server
	std::string sanitized_message = static_cast<std::string>(in_line);
	sanitized_message += '\n';
	send_downstream(*in_server, sanitized_message, in_server_info);
}

std::string RenX_RelayPlugin::UpstreamCommand::to_rcon(const std::string_view& rcon_username) const {
	std::string result;
	result.reserve(m_command.size() + m_response.size() + 64);

	// First: echo command
	result = std::string_view { "lRCON" RX_DELIM "Command;" RX_DELIM };
	result += rcon_username;
	result += std::string_view { RX_DELIM "executed:" RX_DELIM };
	result += m_command;
	result += '\n';

	// Second: command response
	for (auto& response_line : m_response) {
		result += 'r';
		result += response_line;
		result += '\n';
	}

	// Third: command complete
	result += "c\n"sv;

	return result;
}

// Plugin instantiation and entry point.
RenX_RelayPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
