/**
 * Copyright (C) 2021 Jessica James. All rights reserved.
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#if !defined _RELAY_H_HEADER
#define _RELAY_H_HEADER

#include <optional>
#include <deque>
#include <functional>
#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/TCPSocket.h"
#include "RenX_Plugin.h"

class RenX_RelayPlugin : public RenX::Plugin
{
public: // Jupiter::Thinker
	int think() override;

public: // Jupiter::Plugin
	bool initialize() override;

public: // RenX::Plugin
	void RenX_OnServerFullyConnected(RenX::Server &server) override;
	void RenX_OnServerDisconnect(RenX::Server &server, RenX::DisconnectReason reason) override;
	void RenX_OnRaw(RenX::Server &server, const Jupiter::ReadableString &raw) override;

private:
	struct UpstreamCommand {
		std::string m_command; // including parameters
		std::vector<std::string> m_response;
		bool m_is_fake{};

		std::string to_rcon(const std::string_view& rcon_username) const;
	};

	struct ext_server_info {
		std::unique_ptr<Jupiter::TCPSocket> m_socket;
		bool m_devbot_connected{};
		std::chrono::steady_clock::time_point m_last_connect_attempt{};
		std::chrono::steady_clock::time_point m_last_activity{};
		Jupiter::StringL m_last_line;
		std::deque<UpstreamCommand> m_response_queue; // also contains real commands
		bool m_processing_command{};
	};

	void devbot_connected(RenX::Server& in_server, ext_server_info& in_server_info);
	void devbot_disconnected(RenX::Server& in_server, ext_server_info& in_server_info);
	void process_devbot_message(RenX::Server* in_server, const Jupiter::ReadableString& in_line, ext_server_info& in_server_info);

	std::unordered_map<RenX::Server*, ext_server_info> m_server_info_map;
	std::chrono::steady_clock::time_point m_init_time{};
	std::string m_upstream_hostname;
	uint16_t m_upstream_port;
	bool m_fake_pings{};
	bool m_fake_ignored_commands{};
	bool m_sanitize_names{};
	bool m_sanitize_ips{};
	bool m_sanitize_hwids{};
	bool m_sanitize_steam_ids{};
	bool m_suppress_unknown_commands{};
	bool m_suppress_blacklisted_commands{};
	bool m_suppress_chat_logs{};
	bool m_suppress_rcon_command_logs{};
	using fake_command_handler = std::function<bool(std::string_view in_command_line, RenX::Server& in_server, std::vector<std::string>& out_response)>;
	std::unordered_map<std::string, fake_command_handler> m_fake_command_table;
};

#endif // _RELAY_H_HEADER