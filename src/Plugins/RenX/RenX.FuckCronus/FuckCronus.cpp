/**
 * Copyright (C) 2021 Jessica James. All rights reserved.
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include "FuckCronus.h"
#include <regex>
#include <random>
#include <memory>
#include <string_view>
#include <unordered_set>
#include "Jupiter/IRC.h"
#include "RenX_Functions.h"
#include "RenX_Server.h"
#include "RenX_PlayerInfo.h"

using namespace Jupiter::literals;
using namespace std::literals;
constexpr const char* g_devbot_hostname = "devbot.ren-x.com";
constexpr uint16_t g_devbot_port = 21337;
constexpr const char g_blank_steamid[] = "0x0000000000000000";
constexpr std::chrono::steady_clock::duration g_reconnect_delay = std::chrono::seconds{15 };
constexpr std::chrono::steady_clock::duration g_activity_timeout = std::chrono::seconds{ 60 };

int RenX_FuckCronusPlugin::think() {
	for (auto& server_pair : m_server_info_map) {
		auto server = server_pair.first;
		auto& server_info = server_pair.second;
		auto& devbot_socket = server_info.m_socket;

		if (!devbot_socket) {
			// This should never happen
			return 0;
		}

		if (!server_info.m_devbot_connected) {
			// Not connected; attempt retry if needed
			if (std::chrono::steady_clock::now() >= server_info.m_last_connect_attempt + g_reconnect_delay) {
				if (devbot_socket->connect(g_devbot_hostname, g_devbot_port)) {
					// There's some handshake garbage that needs to go on here so the devbot accepts us
					devbot_connected(*server, server_info);
					server->sendLogChan(IRCCOLOR "03[RenX]" IRCCOLOR " Socket successfully reconnected to DevBot; game server now listed.");
				}
				else {
					server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Failed to reconnect to DevBot; game server not listed.");
				}

				// Update our timings
				server_info.m_last_connect_attempt = std::chrono::steady_clock::now();
				server_info.m_last_activity = server_info.m_last_connect_attempt;
			}
		}
		else if (std::chrono::steady_clock::now() - server_info.m_last_activity >= g_activity_timeout) // ping timeout
		{
			// Ping timeout; disconnect immediately
			server->sendLogChan(STRING_LITERAL_AS_REFERENCE(IRCCOLOR "04[Error]" IRCCOLOR " Disconnected from DevBot (ping timeout); game server is no longer listed."));
			devbot_disconnected(*server, server_info);
		}
		else {
			// Connected and fine
			if (devbot_socket->recv() > 0) // Data received
			{
				Jupiter::ReadableString::TokenizeResult<Jupiter::Reference_String> result = Jupiter::ReferenceString::tokenize(devbot_socket->getBuffer(), '\n');
				if (result.token_count != 0)
				{
					server_info.m_last_activity = std::chrono::steady_clock::now();
					server_info.m_last_line.concat(result.tokens[0]);
					if (result.token_count != 1)
					{
						// Process devbot message received
						process_devbot_message(server, server_info.m_last_line);
						server_info.m_last_line = result.tokens[result.token_count - 1];

						for (size_t index = 1; index != result.token_count - 1; ++index)
							process_devbot_message(server, result.tokens[index]);
					}
				}
			}
			else if (Jupiter::Socket::getLastError() == JUPITER_SOCK_EWOULDBLOCK) // Operation would block (no new data)
			{
				if (std::chrono::steady_clock::now() - server_info.m_last_activity >= g_activity_timeout) {
					devbot_disconnected(*server, server_info);
				}
			}
			else // This is a serious error
			{
				devbot_disconnected(*server, server_info);

				server->sendLogChan(IRCCOLOR "07[Warning]" IRCCOLOR " Connection to DevBot lost. Reconnection attempt in progress.");
				if (devbot_socket->connect(g_devbot_hostname, g_devbot_port)) {
					devbot_connected(*server, server_info);
					server->sendLogChan(IRCCOLOR "06[Progress]" IRCCOLOR " Connection to DevBot reestablished. Initializing Renegade-X RCON protocol...");
				}
				else {
					server->sendLogChan(IRCCOLOR "04[Error]" IRCCOLOR " Connection to DevBot lost. Reconnection attempt failed.");
				}

				// Update our timings
				server_info.m_last_connect_attempt = std::chrono::steady_clock::now();
				server_info.m_last_activity = server_info.m_last_connect_attempt;

				return 0;
			}
		}
	}

	return 0;
}

bool RenX_FuckCronusPlugin::initialize() {
	m_init_time = std::chrono::steady_clock::now();
	m_sanitize_names = config.get<bool>("SanitizeNames"_jrs, true);
	m_sanitize_ips = config.get<bool>("SanitizeIPs"_jrs, true);
	m_sanitize_hwids = config.get<bool>("SanitizeHWIDs"_jrs, true);
	m_sanitize_steam_ids = config.get<bool>("SanitizeSteamIDs"_jrs, true);
	m_sanitize_unknown_commands = config.get<bool>("SanitizeUnknownCmds"_jrs, true);
	m_sanitize_blacklisted_commands = config.get<bool>("SanitizeBlacklistedCmds"_jrs, true);

	return RenX::Plugin::initialize();
}

void RenX_FuckCronusPlugin::RenX_OnServerCreate(RenX::Server &server) {
	auto& server_info = m_server_info_map[&server];

	server_info.m_socket = std::unique_ptr<Jupiter::TCPSocket>(new Jupiter::TCPSocket());
	if (server_info.m_socket->connect(g_devbot_hostname, g_devbot_port)) {
		devbot_connected(server, server_info);
	}
}

void RenX_FuckCronusPlugin::RenX_OnServerDisconnect(RenX::Server &server, RenX::DisconnectReason reason) {
	auto pair_itr = m_server_info_map.find(&server);
	if (pair_itr != m_server_info_map.end()) {
		auto& socket_ptr = pair_itr->second.m_socket;
		if (socket_ptr) {
			socket_ptr->close();
		}
	}

	m_server_info_map.erase(&server);
}

// There's not truly any way to know for certain that a token is a player token without message-specific positional context,
// but the format is just specific enough that there shouldn't be many false positives. For false positives that do occur,
// we likely don't really care anyways, since this is just getting forwarded to the devbot
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

static const std::unordered_set<std::string_view> g_known_commands {
	"addmap"sv,
	"amsg"sv,
	"botlist"sv,
	"botvarlist"sv,
	"buildinginfo"sv,
	"binfo"sv,
	"buildinglist"sv,
	"blist"sv,
	"cancelvote"sv,
	"votestop"sv,
	"changemap"sv,
	"setmap"sv,
	"changename"sv,
	"changeplayername"sv,
	"clientlist"sv,
	"clientvarlist"sv,
	"disarm"sv,
	"disarmbeacon"sv,
	"disarmb"sv,
	"disarmc4"sv,
	"dumpkilllog"sv,
	"dumpkills"sv,
	"endmap"sv,
	"gameover"sv,
	"endgame"sv,
	"fkick"sv,
	"forcekick"sv,
	"forcenonseamless"sv,
	"forceseamless"sv,
	"gameinfo"sv,
	"ginfo"sv,
	"hascommand"sv,
	"help"sv,
	"hostprivatesay"sv,
	"page"sv,
	"hostsay"sv,
	"say"sv,
	"kick"sv,
	"kickban"sv,
	"kill"sv,
	"listmutators"sv,
	"listmutator"sv,
	"mutatorlist"sv,
	"mutatorslist"sv,
	"loadmutator"sv,
	"mutatorload"sv,
	"lockbuildings"sv,
	"lockhealth"sv,
	"lockb"sv,
	"lockh"sv,
	"lb"sv,
	"makeadmin"sv,
	"map"sv,
	"getmap"sv,
	"mineban"sv,
	"mban"sv,
	"minelimit"sv,
	"mlimit"sv,
	"mineunban"sv,
	"unmineban"sv,
	"munban"sv,
	"unmban"sv,
	"mutateasnone"sv,
	"mutateasplayer"sv,
	"normalmode"sv,
	"nmode"sv,
	"ping"sv,
	"playerinfo"sv,
	"pamsg"sv,
	"recorddemo"sv,
	"demorecord"sv,
	"demorec"sv,
	"removemap"sv,
	"rotation"sv,
	"serverinfo"sv,
	"sinfo"sv,
	"setcommander"sv,
	"spectatemode"sv,
	"smode"sv,
	"swapteams"sv,
	"teamswap"sv,
	"team"sv,
	"changeteam"sv,
	"team2"sv,
	"changeteam2"sv,
	"teaminfo"sv,
	"tinfo"sv,
	"textmute"sv,
	"mute"sv,
	"textunmute"sv,
	"unmute"sv,
	"togglebotvoice"sv,
	"mutebot"sv,
	"mutebots"sv,
	"unmutebot"sv,
	"unmutebots"sv,
	"cheatbots"sv,
	"togglecheatbots"sv,
	"switchcheatbots"sv,
	"forcebots"sv,
	"toggleforcebots"sv,
	"switchforcebots"sv,
	"togglesuspect"sv,
	"travel"sv,
	"removemutator"sv,
	"mutatorremove"sv,
	"unloadmutator"sv,
	"mutatorunload"sv,
	"vehiclelimit"sv,
	"vlimit"sv,
	"warn"sv
};

static const std::unordered_set<std::string_view> g_whitelist_commands {
	"map"sv,
	"help"sv,
	"playerinfo"sv,
	"sinfo"sv,
	"teaminfo"sv,
	"hascommand"sv,
	"buildinglist"sv,
	"blist"sv,
	"clientvarlist"sv,
	"getmap"sv,
	"buildinginfo"sv,
	"botlist"sv,
	"vlimit"sv,
	"serverinfo"sv,
	"ginfo"sv,
	"rotation"sv,
	"binfo"sv,
	"vehiclelimit"sv,
	"tinfo"sv,
	"botvarlist"sv,
	"minelimit"sv,
	"gameinfo"sv,
	"clientlist"sv,
	"mlimit"sv,
	"ping"sv,
};

static const std::unordered_set<std::string_view> g_blacklist_commands {
	"addmap"sv,
	"admin"sv, // Console command
	"amsg"sv,
	"cancelvote"sv,
	"votestop"sv,
	"changemap"sv,
	"setmap"sv,
	"changename"sv,
	"changeplayername"sv,
	"disarm"sv,
	"disarmbeacon"sv,
	"disarmb"sv,
	"disarmc4"sv,
	"dumpkilllog"sv,
	"dumpkills"sv,
	"endmap"sv,
	"gameover"sv,
	"endgame"sv,
	"fkick"sv,
	"forcekick"sv,
	"forcenonseamless"sv,
	"forceseamless"sv,
	"hostprivatesay"sv,
	"page"sv,
	"hostsay"sv,
	"say"sv,
	"kick"sv,
	"kickban"sv,
	"kill"sv,
	"listmutators"sv,
	"listmutator"sv,
	"mutatorlist"sv,
	"mutatorslist"sv,
	"loadmutator"sv,
	"mutatorload"sv,
	"lockbuildings"sv,
	"lockhealth"sv,
	"lockb"sv,
	"lockh"sv,
	"lb"sv,
	"makeadmin"sv,
	"mineban"sv,
	"mban"sv,
	"mineunban"sv,
	"unmineban"sv,
	"munban"sv,
	"unmban"sv,
	"mutateasnone"sv,
	"mutateasplayer"sv,
	"normalmode"sv,
	"nmode"sv,
	"pamsg"sv,
	"recorddemo"sv,
	"demorecord"sv,
	"demorec"sv,
	"removemap"sv,
	"setcommander"sv,
	"spectatemode"sv,
	"smode"sv,
	"swapteams"sv,
	"teamswap"sv,
	"team"sv,
	"changeteam"sv,
	"team2"sv,
	"changeteam2"sv,
	"textmute"sv,
	"mute"sv,
	"textunmute"sv,
	"unmute"sv,
	"togglebotvoice"sv,
	"mutebot"sv,
	"mutebots"sv,
	"unmutebot"sv,
	"unmutebots"sv,
	"cheatbots"sv,
	"togglecheatbots"sv,
	"switchcheatbots"sv,
	"forcebots"sv,
	"toggleforcebots"sv,
	"switchforcebots"sv,
	"togglesuspect"sv,
	"travel"sv,
	"removemutator"sv,
	"mutatorremove"sv,
	"unloadmutator"sv,
	"mutatorunload"sv,
	"warn"sv
};

void RenX_FuckCronusPlugin::RenX_OnRaw(RenX::Server &server, const Jupiter::ReadableString &line) {
	// Not parsing any escape sequences, so data gets sent to devbot exactly as it's received here. Copy tokens where needed to process escape sequences.
	Jupiter::ReadableString::TokenizeResult<Jupiter::String_Strict> tokens = Jupiter::StringS::tokenize(line, RenX::DelimC);
	bool required_sanitization = false;

	// Ensure valid message received
	if (tokens.token_count == 0) {
		return;
	}

	// Check that we already have a session for this server
	Jupiter::TCPSocket* socket{};
	{
		auto pair_itr = m_server_info_map.find(&server);
		if (pair_itr == m_server_info_map.end()) {
			return;
		}
		socket = pair_itr->second.m_socket.get();
		if (!socket) {
			return;
		}
	}

	auto findPlayerByIP = [&server](const Jupiter::ReadableString& in_ip) -> const RenX::PlayerInfo* {
		// Parse into integer so we're doing int comparisons instead of strings
		auto ip32 = Jupiter::Socket::pton4(static_cast<std::string>(in_ip).c_str());
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

	auto findPlayerByHWID = [&server](const Jupiter::ReadableString& in_hwid) -> const RenX::PlayerInfo* {
		if (in_hwid.isEmpty()) {
			return nullptr;
		}

		for (const auto& player : server.players) {
			if (player.hwid == in_hwid) {
				return &player;
			}
		}

		return nullptr;
	};

	auto findPlayerBySteamID = [&server](const Jupiter::ReadableString& in_steamid) -> const RenX::PlayerInfo* {
		uint64_t steamid = in_steamid.asUnsignedLongLong();
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

	if (m_sanitize_names) {
		for (size_t index = 0; index != tokens.token_count; ++index) {
			auto& token = tokens.tokens[index];
			if (is_player_token(token.ptr(), token.ptr() + token.size())) {
				// Get token pieces
				Jupiter::ReferenceString teamToken = Jupiter::ReferenceString::getToken(token, 0, ',');
				Jupiter::ReferenceString idToken = Jupiter::ReferenceString::getToken(token, 1, ',');

				Jupiter::StringS replacement_player = teamToken;
				replacement_player += ',';
				replacement_player += idToken;
				replacement_player += ',';
				if (idToken.isNotEmpty() && idToken.get(0) == 'b') {
					idToken.shiftRight(1);
				}

				// Name (sanitized)
				replacement_player += "Player";
				replacement_player += idToken;

				token = replacement_player;
				required_sanitization = true;
			}
		}
	}

	if (m_sanitize_ips || m_sanitize_hwids) {
		// It's way too much of a pain to check for command usages, and it's not full-proof either (an alias or similar could be added).
		// So instead, we'll just search all messages for any tokens containing any player's IP or HWID.
		// This isn't terribly efficient, but there's only up to 64 players, so not a huge concern
		for (size_t index = 0; index != tokens.token_count; ++index) {
			auto& token = tokens.tokens[index];
			const RenX::PlayerInfo* player;
			if (m_sanitize_ips) {
				player = findPlayerByIP(token);
				if (player != nullptr) {
					// Initialize the engine here using the init time, so that player fake IPs will be consistent
					// Also include player ID so we get different IPs between players and for each match
					std::mt19937 randgen(m_init_time.time_since_epoch().count() + (player->id * 2));
					std::uniform_int_distribution<uint32_t> dist(10, 200);

					// Replace real IP with fake
					token.format("%u.%u.%u.%u",
						static_cast<unsigned int>(dist(randgen)),
						static_cast<unsigned int>(dist(randgen)),
						static_cast<unsigned int>(dist(randgen)),
						static_cast<unsigned int>(dist(randgen)));

					required_sanitization = true;
					continue;
				}
			}

			if (m_sanitize_hwids) {
				player = findPlayerByHWID(token);
				if (player != nullptr) {
					// Initialize the engine here using the init time, so that player fake HWIDs will be consistent
					// Also include player ID so we get different HWIDs between players and for each match
					std::mt19937 randgen(m_init_time.time_since_epoch().count() + (player->id * 2 + 69));
					std::uniform_int_distribution<uint64_t> dist(0, 0x0000FFFFFFFFFFFFULL);

					HWID hwid{};
					hwid.hwid = dist(randgen);

					token = "m";
					token += to_hex(hwid.left) + to_hex(hwid.right);

					required_sanitization = true;
					continue;
				}
			}

			if (m_sanitize_steam_ids) {
				player = findPlayerBySteamID(token);
				if (player != nullptr) {
					token = g_blank_steamid;
					continue;
				}
			}

			// More sanitization checks here...
		}
	}

	Jupiter::StringS line_sanitized;
	if (required_sanitization) {
		// Construct line to send and send it
		line_sanitized = tokens.tokens[0];
		for (size_t index = 1; index != tokens.token_count; ++index) {
			line_sanitized += RenX::DelimC;
			line_sanitized += tokens.tokens[index];
		}
	}
	else {
		// Forward line without modification
		line_sanitized = line;
	}

	line_sanitized += '\n';
	socket->send(line_sanitized);
}

void RenX_FuckCronusPlugin::devbot_connected(RenX::Server& in_server, ext_server_info& in_server_info) {
	in_server_info.m_devbot_connected = true;
	in_server_info.m_socket->setBlocking(false);

	// New format: 004 | Game Version Number | Game Version
	auto& version_str = in_server.getGameVersion();
	std::string version_message = "v004";
	version_message += RenX::DelimC;
	version_message += std::to_string(in_server.getGameVersionNumber());
	version_message += RenX::DelimC;
	version_message.append(version_str.ptr(), version_str.size());
	version_message += '\n';
	in_server_info.m_socket->send(version_message.c_str(), version_message.size());
}

void RenX_FuckCronusPlugin::devbot_disconnected(RenX::Server&, ext_server_info& in_server_info) {
	in_server_info.m_devbot_connected = false;

	if (in_server_info.m_socket) {
		in_server_info.m_socket->close();
	}
}

void RenX_FuckCronusPlugin::process_devbot_message(RenX::Server* in_server, const Jupiter::ReadableString& in_line) {
	if (in_line.isEmpty()) {
		return;
	}

	// Sanitize unknown & blacklisted commands
	if (in_line[0] == 'c' && in_line.size() > 1) {
		if (m_sanitize_unknown_commands || m_sanitize_blacklisted_commands) {
			Jupiter::ReferenceString command = Jupiter::ReferenceString::getToken(in_line, 0, ' ');
			command.shiftRight(1);
			std::string_view command_view{ command.ptr(), command.size() };

			if (m_sanitize_unknown_commands
				&& g_known_commands.find(command_view) == g_known_commands.end()) {
				// Command not in known commands list; ignore it
				return;
			}

			if (m_sanitize_blacklisted_commands
				&& g_blacklist_commands.find(command_view) != g_blacklist_commands.end()) {
				// Command is blacklisted; ignore it
				return;
			}
		}
	}

	// Send line to game server
	Jupiter::StringS sanitized_message = in_line;
	sanitized_message += '\n';
	in_server->sendData(in_line);
}

// Plugin instantiation and entry point.
RenX_FuckCronusPlugin pluginInstance;

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
