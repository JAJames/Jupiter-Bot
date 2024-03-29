/**
 * Copyright (C) 2016-2021 Jessica James.
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

#include <charconv>
#include "jessilib/split.hpp"
#include "jessilib/unicode.hpp"
#include "jessilib/http_query.hpp"
#include "Jupiter/IRC_Client.h"
#include "Jupiter/HTTP.h"
#include "HTTPServer.h"
#include "RenX_Core.h"
#include "RenX_Server.h"
#include "RenX_Functions.h"
#include "RenX_PlayerInfo.h"
#include "RenX_ServerList.h"

using namespace std::literals;

static constexpr std::string_view CONTENT_TYPE_APPLICATION_JSON = "application/json"sv;

constexpr std::string_view server_list_game_header = "<html><body>"sv;
constexpr std::string_view server_list_game_footer = "\n</body></html>"sv;

// TODO: can probably replace with some of the jessilib stuff
std::string jsonify(std::string_view in_str) {
	const unsigned char *ptr = reinterpret_cast<const unsigned char *>(in_str.data());
	const unsigned char *end_ptr = ptr + in_str.size();
	std::string result;

	while (ptr < end_ptr) {
		if (*ptr == '\\') { // backslash
			result += '\\';
			result += '\\';
		}
		else if (*ptr == '\"') { // quotation
			result += '\\';
			result += '\"';
		}
		else if (*ptr < 0x20) { // control characters
			char buffer[2]; // control codes are only ever going to be at most 2 characters
			if (std::to_chars(buffer, buffer + sizeof(buffer), *ptr, 16).ec == std::errc{}) {
				result += "\\u00"sv;
				if (*ptr > 0xF) {
					result += buffer[0];
					result += buffer[1];
				}
				else {
					result += '0';
					result += buffer[0];
				}
			};
		}
		else if ((*ptr & 0x80) != 0) { // UTF-8 sequence; copy to bypass above processing
			result += *ptr;
			if ((*ptr & 0x40) != 0) {
				// this is a 2+ byte sequence

				if ((*ptr & 0x20) != 0) {
					// this is a 3+ byte sequence

					if ((*ptr & 0x10) != 0) {
						// this is a 4 byte sequnce
						result += *++ptr;
					}

					result += *++ptr;
				}

				result += *++ptr;
			}
		}
		else { // Character in standard ASCII table
			result += *ptr;
		}

		++ptr;
	}

	return result;
}

bool RenX_ServerListPlugin::initialize() {
	m_web_hostname = this->config.get("Hostname"sv, ""sv);
	m_web_path = this->config.get("Path"sv, "/"sv);
	m_server_list_page_name = this->config.get("ServersPageName"sv, "servers"sv);
	m_server_list_long_page_name = this->config.get("HumanServersPageName"sv, "servers_long"sv);
	m_server_page_name = this->config.get("ServerPageName"sv, "server"sv);
	m_metadata_page_name = this->config.get("MetadataPageName"sv, "metadata"sv);
	m_metadata_prometheus_page_name = this->config.get("MetadataPrometheusPageName"sv, "metadata_prometheus"sv);

	/** Initialize content */
	Jupiter::HTTP::Server &server = getHTTPServer();

	// Server list page
	std::unique_ptr<Jupiter::HTTP::Server::Content> content = std::make_unique<Jupiter::HTTP::Server::Content>(m_server_list_page_name, handle_server_list_page);
	content->language = Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = CONTENT_TYPE_APPLICATION_JSON;
	content->charset = Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	content->free_result = false;
	server.hook(m_web_hostname, m_web_path, std::move(content));

	// Server list (long) page
	content = std::make_unique<Jupiter::HTTP::Server::Content>(m_server_list_long_page_name, handle_server_list_long_page);
	content->language = Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = CONTENT_TYPE_APPLICATION_JSON;
	content->charset = Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	content->free_result = true;
	server.hook(m_web_hostname, m_web_path, std::move(content));

	// Server page (GUIDs)
	content = std::make_unique<Jupiter::HTTP::Server::Content>(m_server_page_name, handle_server_page);
	content->language = Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = CONTENT_TYPE_APPLICATION_JSON;
	content->charset = Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	content->free_result = true;
	server.hook(m_web_hostname, m_web_path, std::move(content));

	// Metadata page
	content = std::make_unique<Jupiter::HTTP::Server::Content>(m_metadata_page_name, handle_metadata_page);
	content->language = Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = CONTENT_TYPE_APPLICATION_JSON;
	content->charset = Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	content->free_result = false;
	server.hook(m_web_hostname, m_web_path, std::move(content));

	// Metadata page
	content = std::make_unique<Jupiter::HTTP::Server::Content>(m_metadata_prometheus_page_name, handle_metadata_prometheus_page);
	content->language = Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = CONTENT_TYPE_APPLICATION_JSON;
	content->charset = Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	content->free_result = false;
	server.hook(m_web_hostname, m_web_path, std::move(content));

	this->updateServerList();
	return true;
}

RenX_ServerListPlugin::~RenX_ServerListPlugin() {
	Jupiter::HTTP::Server &server = getHTTPServer();
	server.remove(m_web_hostname, m_web_path, m_server_list_page_name);
	server.remove(m_web_hostname, m_web_path, m_server_list_long_page_name);
	server.remove(m_web_hostname, m_web_path, m_server_page_name);
}

size_t RenX_ServerListPlugin::getListedPlayerCount(const RenX::Server& server) {
	size_t player_limit = static_cast<size_t>(std::max(server.getPlayerLimit(), 0));
	return std::min(server.activePlayers(false).size(), player_limit);
}

std::string* RenX_ServerListPlugin::getServerListJSON() {
	return &m_server_list_json;
}

std::string* RenX_ServerListPlugin::getMetadataJSON() {
	return &m_metadata_json;
}

std::string* RenX_ServerListPlugin::getMetadataPrometheus() {
	return &m_metadata_prometheus;
}

constexpr const char *json_bool_as_cstring(bool in) {
	return in ? "true" : "false";
}

std::string RenX_ServerListPlugin::server_as_json(const RenX::Server &server) {
	ListServerInfo serverInfo = getListServerInfo(server);

	if (serverInfo.hostname.empty()) {
		return "null"s;
	}

	std::string server_name = jsonify(server.getName());
	std::string server_map = jsonify(server.getMap().name);
	std::string server_version = jsonify(server.getGameVersion());
	std::string_view server_hostname = serverInfo.hostname;
	unsigned short server_port = serverInfo.port;
	std::string server_prefix = jsonify(serverInfo.namePrefix);

	// Some members we only include if they're populated
	if (!server_prefix.empty()) {
		server_prefix = jessilib::join<std::string>(R"json("NamePrefix":")json"sv, server_prefix, "\","sv);
	}

	std::string server_attributes;
	if (!serverInfo.attributes.empty()) {
		server_attributes.reserve(16 * serverInfo.attributes.size() + 16);
		server_attributes = R"json("Attributes":[)json";

		for (std::string_view attribute : serverInfo.attributes) {
			server_attributes += '\"';
			server_attributes += attribute;
			server_attributes += "\",";
		}

		server_attributes.back() = ']';
		server_attributes += ',';
	}

	// Build block
	std::string server_json_block = string_printf(R"json({"Name":"%.*s",%.*s"Current Map":"%.*s","Bots":%u,"Players":%u,"Game Version":"%.*s",%.*s"Variables":{"Mine Limit":%d,"bSteamRequired":%s,"bPrivateMessageTeamOnly":%s,"bPassworded":%s,"bAllowPrivateMessaging":%s,"bRanked":%s,"Game Type":%d,"Player Limit":%d,"Vehicle Limit":%d,"bAutoBalanceTeams":%s,"Team Mode":%d,"bSpawnCrates":%s,"CrateRespawnAfterPickup":%f,"Time Limit":%d},"Port":%u,"IP":"%.*s")json",
		server_name.size(), server_name.data(),
		server_prefix.size(), server_prefix.data(),
		server_map.size(), server_map.data(),
		server.getBotCount(),
		getListedPlayerCount(server),
		server_version.size(), server_version.data(),
		server_attributes.size(), server_attributes.data(),
		server.getMineLimit(),
		json_bool_as_cstring(server.isSteamRequired()),
		json_bool_as_cstring(server.isPrivateMessageTeamOnly()),
		json_bool_as_cstring(server.isPassworded()),
		json_bool_as_cstring(server.isPrivateMessagingEnabled()),
		json_bool_as_cstring(server.isRanked()),
		server.getGameType(),
		server.getPlayerLimit(),
		server.getVehicleLimit(),
		json_bool_as_cstring(server.getTeamMode() == 3),
		server.getTeamMode(),
		json_bool_as_cstring(server.isCratesEnabled()),
		server.getCrateRespawnDelay(),
		server.getTimeLimit(),
		server_port,
		server_hostname.size(), server_hostname.data());

	server_json_block += '}';

	return server_json_block;
}

std::string RenX_ServerListPlugin::server_as_server_details_json(const RenX::Server& server) {
	std::string server_json_block;

	server_json_block = '{';

	// Levels
	if (server.maps.size() != 0) {
		server_json_block += "\"Levels\":["sv;

		server_json_block += "{\"Name\":\""sv;
		server_json_block += jsonify(server.maps[0].name);
		server_json_block += "\",\"GUID\":\""sv;
		server_json_block += RenX::formatGUID(server.maps[0]);
		server_json_block += "\"}"sv;

		for (size_t index = 1; index != server.maps.size(); ++index) {
			server_json_block += ",{\"Name\":\""sv;
			server_json_block += jsonify(server.maps[index].name);
			server_json_block += "\",\"GUID\":\""sv;
			server_json_block += RenX::formatGUID(server.maps[index]);
			server_json_block += "\"}"sv;
		}

		server_json_block += ']';
	}

	// Mutators
	if (server.mutators.size() != 0) {
		if (server.maps.size() != 0)
			server_json_block += ","sv;
		server_json_block += "\"Mutators\":["sv;

		server_json_block += "{\"Name\":\""sv;
		server_json_block += jsonify(server.mutators[0]);
		server_json_block += "\"}"sv;

		for (size_t index = 1; index != server.mutators.size(); ++index) {
			server_json_block += ",{\"Name\":\""sv;
			server_json_block += jsonify(server.mutators[index]);
			server_json_block += "\"}"sv;
		}

		server_json_block += ']';
	}

	// Player List
	if (server.players.size() != 0 && server.players.size() != server.getBotCount()) {
		server_json_block += ",\"PlayerList\":["sv;

		auto node = server.players.begin();

		if (node != server.players.end()) {
			server_json_block += "{\"Name\":\""sv;
			server_json_block += jsonify(node->name);
			server_json_block += "\", \"isBot\":"sv;
			server_json_block += json_bool_as_cstring(node->isBot);
			server_json_block += ", \"Team\":"sv;
			server_json_block += std::to_string(static_cast<int>(node->team));
			server_json_block += "}"sv;

			++node;
		}

		while (node != server.players.end()) {
			server_json_block += ",{\"Name\":\""sv;
			server_json_block += jsonify(node->name);
			server_json_block += "\", \"isBot\":"sv;
			server_json_block += json_bool_as_cstring(node->isBot);
			server_json_block += ", \"Team\":"sv;
			server_json_block += std::to_string(static_cast<int>(node->team));
			server_json_block += "}"sv;

			++node;
		}

		server_json_block += "]"sv;
	}

	server_json_block += '}';
	return server_json_block;
}

std::string RenX_ServerListPlugin::server_as_long_json(const RenX::Server &server) {
	ListServerInfo serverInfo = getListServerInfo(server);

	std::string server_name = jsonify(server.getName());
	std::string server_map = jsonify(server.getMap().name);
	std::string server_version = jsonify(server.getGameVersion());
	std::string_view server_hostname = serverInfo.hostname;
	unsigned short server_port = serverInfo.port;
	std::string server_prefix = jsonify(serverInfo.namePrefix);
	std::vector<const RenX::PlayerInfo*> activePlayers = server.activePlayers(false);

	std::string server_attributes = "[]";
	if (!serverInfo.attributes.empty()) {
		server_attributes.reserve(16 * serverInfo.attributes.size() + 16);
		server_attributes = "[";

		const char* comma = "\n";
		for (std::string_view attribute : serverInfo.attributes) {
			server_attributes += comma;
			comma = ",\n";

			server_attributes += "\t\t\t\"";
			server_attributes += attribute;
			server_attributes += "\"";
		}

		server_attributes += "\n\t\t],";
	}

	std::string server_json_block = string_printf(R"json({
		"Name": "%.*s",
		"NamePrefix": "%.*s",
		"Current Map": "%.*s",
		"Bots": %u,
		"Players": %u,
		"Game Version": "%.*s",
		"Attributes": %.*s,
		"Variables": {
			"Mine Limit": %d,
			"bSteamRequired": %s,
			"bPrivateMessageTeamOnly": %s,
			"bPassworded": %s,
			"bAllowPrivateMessaging": %s,
			"Player Limit": %d,
			"Vehicle Limit": %d,
			"bAutoBalanceTeams": %s,
			"Team Mode": %d,
			"bSpawnCrates": %s,
			"CrateRespawnAfterPickup": %f,
			"Time Limit": %d
		},
		"Port": %u,
		"IP": "%.*s")json",
		server_name.size(), server_name.data(),
		server_prefix.size(), server_prefix.data(),
		server_map.size(), server_map.data(),
		server.getBotCount(),
		activePlayers.size(),
		server_version.size(), server_version.data(),
		server_attributes.size(), server_attributes.data(),

		server.getMineLimit(),
		json_bool_as_cstring(server.isSteamRequired()),
		json_bool_as_cstring(server.isPrivateMessageTeamOnly()),
		json_bool_as_cstring(server.isPassworded()),
		json_bool_as_cstring(server.isPrivateMessagingEnabled()),
		server.getPlayerLimit(),
		server.getVehicleLimit(),
		json_bool_as_cstring(server.getTeamMode() == 3),
		server.getTeamMode(),
		json_bool_as_cstring(server.isCratesEnabled()),
		server.getCrateRespawnDelay(),
		server.getTimeLimit(),

		server_port,
		server_hostname.size(), server_hostname.data());

	// Level Rotation
	if (server.maps.size() != 0) {
		server_json_block += ",\n\t\t\"Levels\": ["sv;

		server_json_block += "\n\t\t\t{\n\t\t\t\t\"Name\": \""sv;
		server_json_block += jsonify(server.maps[0].name);
		server_json_block += "\",\n\t\t\t\t\"GUID\": \""sv;
		server_json_block += RenX::formatGUID(server.maps[0]);
		server_json_block += "\"\n\t\t\t}"sv;

		for (size_t index = 1; index != server.maps.size(); ++index) {
			server_json_block += ",\n\t\t\t{\n\t\t\t\t\"Name\": \""sv;
			server_json_block += jsonify(server.maps[index].name);
			server_json_block += "\",\n\t\t\t\t\"GUID\": \""sv;
			server_json_block += RenX::formatGUID(server.maps[index]);
			server_json_block += "\"\n\t\t\t}"sv;
		}

		server_json_block += "\n\t\t]"sv;
	}

	// Mutators
	if (server.mutators.size() != 0) {
		server_json_block += ",\n\t\t\"Mutators\": ["sv;

		server_json_block += "\n\t\t\t{\n\t\t\t\t\"Name\": \""sv;
		server_json_block += jsonify(server.mutators[0]);
		server_json_block += "\"\n\t\t\t}"sv;

		for (size_t index = 1; index != server.mutators.size(); ++index) {
			server_json_block += ",\n\t\t\t{\n\t\t\t\t\"Name\": \""sv;
			server_json_block += jsonify(server.mutators[index]);
			server_json_block += "\"\n\t\t\t}"sv;
		}

		server_json_block += "\n\t\t]"sv;
	}

	// Player List
	if (activePlayers.size() != 0) {
		server_json_block += ",\n\t\t\"PlayerList\": ["sv;

		auto node = activePlayers.begin();

		// Add first player to JSON
		server_json_block += "\n\t\t\t{\n\t\t\t\t\"Name\": \""sv;
		server_json_block += jsonify((*node)->name);
		server_json_block += "\"\n\t\t\t}"sv;
		++node;

		// Add remaining players to JSON
		while (node != activePlayers.end()) {
			server_json_block += ",\n\t\t\t{\n\t\t\t\t\"Name\": \""sv;
			server_json_block += jsonify((*node)->name);
			server_json_block += "\"\n\t\t\t}"sv;
			++node;
		}

		server_json_block += "\n\t\t]"sv;
	}

	server_json_block += "\n\t}"sv;

	return static_cast<std::string>(server_json_block);
}

void RenX_ServerListPlugin::addServerToServerList(RenX::Server &server) {
	std::string server_json_block;

	// append to server_list_json
	server_json_block = server_as_json(server);
	if (m_server_list_json.size() <= 2) {
		m_server_list_json = '[';
	}
	else {
		m_server_list_json.pop_back(); // remove trailing ']'.
		m_server_list_json += ',';
	}
	m_server_list_json += server_json_block;
	m_server_list_json += ']';

	// Also update metadata so it reflects the now added server
	updateMetadata();
}

void RenX_ServerListPlugin::updateServerList() {
	const auto& servers = RenX::getCore()->getServers();
	size_t index = 0;
	RenX::Server *server;

	// regenerate server_list_json

	m_server_list_json = '[';

	while (index != servers.size()) {
		server = servers[index];
		if (server->isConnected() && server->isFullyConnected()) {
			m_server_list_json += server_as_json(*server);

			++index;
			break;
		}
		++index;
	}
	while (index != servers.size()) {
		server = servers[index];
		if (server->isConnected() && server->isFullyConnected()) {
			m_server_list_json += ',';
			m_server_list_json += server_as_json(*server);
		}
		++index;
	}

	m_server_list_json += ']';

	// Also update metadata so that it reflects any changes
	updateMetadata();
}

void RenX_ServerListPlugin::updateMetadata() {
	const auto& servers = RenX::getCore()->getServers();
	unsigned int server_count{};
	size_t player_count{};

	for (size_t index = 0; index != servers.size(); ++index) {
		RenX::Server* server = servers[index];
		if (server->isConnected() && server->isFullyConnected()) {
			++server_count;
			player_count += getListedPlayerCount(*server);
		}
	}

	// TODO: not rely on StringS
	m_metadata_json = string_printf(R"json({"player_count":%zu,"server_count":%u})json",
		player_count, server_count);

	m_metadata_prometheus = string_printf("player_count %zu\nserver_count %u\n",
		player_count, server_count);
}

void RenX_ServerListPlugin::markDetailsStale(RenX::Server& in_server) {
	in_server.varData[this->name].remove("j"sv);
}

void RenX_ServerListPlugin::touchDetails(RenX::Server& in_server) {
	auto& server_varData = in_server.varData[this->name];
	if (server_varData.get("j"sv).empty()) {
		auto server_json_block = server_as_server_details_json(in_server);
		server_varData.set("j"sv, server_json_block);
	}
}

std::string_view RenX_ServerListPlugin::getListServerAddress(const RenX::Server& server) {
	std::string_view serverHostname;
	serverHostname = server.getSocketHostname();

	if (auto section = config.getSection(serverHostname)) {
		serverHostname = section->get("ListAddress"sv, serverHostname);
	}

	return serverHostname;
}

RenX_ServerListPlugin::ListServerInfo RenX_ServerListPlugin::getListServerInfo(const RenX::Server& server) {
	ListServerInfo result;

	auto populate_with_section = [&server, &result](Jupiter::Config* section) {
		if (section == nullptr) {
			return;
		}

		result.hostname = section->get("ListAddress"sv, result.hostname);
		result.port = section->get("ListPort"sv, result.port);
		result.namePrefix = section->get("ListNamePrefix"sv, result.namePrefix);

		// Attributes
		std::string_view attributes_str = section->get("ListAttributes"sv);
		if (!attributes_str.empty()) {
			result.attributes = jessilib::split_view(attributes_str, ' ');
		}
	};

	// Populate with standard information
	result.hostname = server.getSocketHostname();
	result.port = server.getPort();

	// Try Overwriting based on IP-named config section
	if (auto section = config.getSection(result.hostname)) {
		populate_with_section(section);

		// Try overwriting based on Port subsection
		populate_with_section(section->getSection(string_printf("%u", server.getPort())));
	}

	return result;
}

void RenX_ServerListPlugin::RenX_OnServerFullyConnected(RenX::Server &server) {
	this->addServerToServerList(server);
}

void RenX_ServerListPlugin::RenX_OnServerDisconnect(RenX::Server &server, RenX::DisconnectReason) {
	this->updateServerList();
	markDetailsStale(server);
}

void RenX_ServerListPlugin::RenX_OnJoin(RenX::Server& server, const RenX::PlayerInfo &) {
	markDetailsStale(server);
	this->updateServerList();
}

void RenX_ServerListPlugin::RenX_OnPart(RenX::Server &server, const RenX::PlayerInfo &) {
	if (server.isTravelling() == false || server.isSeamless()) {
		markDetailsStale(server);
		this->updateServerList();
	}
}

void RenX_ServerListPlugin::RenX_OnMapLoad(RenX::Server &server, std::string_view map) {
	markDetailsStale(server);
	this->updateServerList();
}

// Plugin instantiation and entry point.
RenX_ServerListPlugin pluginInstance;

std::string* handle_server_list_page(std::string_view) {
	return pluginInstance.getServerListJSON();
}

std::string* handle_server_list_long_page(std::string_view) {
	const auto& servers = RenX::getCore()->getServers();
	size_t index = 0;
	RenX::Server *server;
	std::string *server_list_long_json = new std::string;
	server_list_long_json->reserve(256 * servers.size());

	// regenerate server_list_json

	*server_list_long_json = "["sv;

	while (index != servers.size()) {
		server = servers[index];
		if (server->isConnected() && server->isFullyConnected()) {
			*server_list_long_json += "\n\t"sv;
			*server_list_long_json += pluginInstance.server_as_long_json(*server);
			++index;
			break;
		}
		++index;
	}
	while (index != servers.size()) {
		server = servers[index];
		if (server->isConnected() && server->isFullyConnected()) {
			*server_list_long_json += ",\n\t"sv;
			*server_list_long_json += pluginInstance.server_as_long_json(*server);
		}
		++index;
	}

	*server_list_long_json += "\n]"sv;

	return server_list_long_json;
}

using query_table_type = std::unordered_map<std::string_view, std::string_view, jessilib::text_hash, jessilib::text_equal>;
std::pair<std::string, query_table_type> parse_query_string(std::string_view in_query_string) {
	std::pair<std::string, query_table_type> result;
	result.first = in_query_string;
	jessilib::deserialize_html_form(result.second, result.first);
	return result;
}

std::string* handle_server_page(std::string_view query_string) {
	std::string parsed_query_string{ query_string };
	std::unordered_map<std::string_view, std::string_view, jessilib::text_hash, jessilib::text_equal> table;
	jessilib::deserialize_html_form(table, parsed_query_string);
	std::string_view address;
	unsigned short port = 0;
	RenX::Server *server;

	// parse form data

	if (table.size() < 2) {
		return new std::string();
	}

	if (table.size() != 0) {
		address = table["ip"sv];
		std::string_view port_str = table["port"sv];
		std::from_chars(port_str.data(), port_str.data() + port_str.size(), port, 10);
	}

	// search for server
	const auto& servers = RenX::getCore()->getServers();
	size_t index = 0;

	while (true) {
		if (index == servers.size())
			return new std::string();

		server = servers[index];
		if (address == pluginInstance.getListServerAddress(*server) && server->getPort() == port)
			break;

		++index;
	}

	// return server data
	pluginInstance.touchDetails(*server);
	return new std::string(server->varData[pluginInstance.getName()].get("j"sv));
}

std::string* handle_metadata_page(std::string_view) {
	return pluginInstance.getMetadataJSON();
}

std::string* handle_metadata_prometheus_page(std::string_view) {
	return pluginInstance.getMetadataPrometheus();
}

extern "C" JUPITER_EXPORT Jupiter::Plugin *getPlugin() {
	return &pluginInstance;
}
