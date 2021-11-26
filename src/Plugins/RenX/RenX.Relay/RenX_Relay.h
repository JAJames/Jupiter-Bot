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
	using fake_command_handler = std::function<bool(std::string_view in_command_line, RenX::Server& in_server, std::vector<std::string>& out_response)>;

	struct UpstreamCommand {
		std::string m_command; // including parameters
		std::vector<std::string> m_response;
		bool m_is_fake{};

		std::string to_rcon(const std::string_view& rcon_username) const;
	};

	struct upstream_settings {
		std::string m_label{}; // config section name
		std::string m_upstream_hostname{};
		uint16_t m_upstream_port{ 21337u };
		std::string m_rcon_username{};
		bool m_log_traffic{ false };
		bool m_fake_pings{ true };
		bool m_fake_ignored_commands{ true };
		bool m_sanitize_names{ true };
		bool m_sanitize_ips{ true };
		bool m_sanitize_hwids{ true };
		bool m_sanitize_steam_ids{ true };
		bool m_suppress_unknown_commands{ true };
		bool m_suppress_blacklisted_commands{ true };
		bool m_suppress_chat_logs{ true };
		bool m_suppress_rcon_command_logs{ true };

		std::unordered_map<std::string, fake_command_handler> m_fake_command_table;
	};

	struct upstream_server_info {
		uint16_t m_port;
		std::unique_ptr<Jupiter::TCPSocket> m_socket;
		bool m_connected{};
		std::chrono::steady_clock::time_point m_last_connect_attempt{};
		std::chrono::steady_clock::time_point m_last_activity{};
		Jupiter::StringL m_last_line;
		std::deque<UpstreamCommand> m_response_queue; // Contains both real & fake commands
		bool m_processing_command{};
		const upstream_settings* m_settings; // weak_ptr to upstream_settings owned by m_configured_upstreams
	};

	upstream_settings get_settings(const Jupiter::Config& in_config);
	std::string get_log_filename(RenX::Server& in_server, const upstream_server_info& in_server_info);
	std::string_view get_upstream_name(const upstream_server_info& in_server_info);
	std::string_view get_upstream_rcon_username(const upstream_server_info& in_server_info, RenX::Server& in_server);
	int send_upstream(upstream_server_info& in_server_info, std::string_view in_message, RenX::Server& in_server);
	int send_downstream(RenX::Server& in_server, std::string_view in_message, upstream_server_info& in_server_info);

	void upstream_connected(RenX::Server& in_server, upstream_server_info& in_server_info);
	void upstream_disconnected(RenX::Server& in_server, upstream_server_info& in_server_info);
	void process_upstream_message(RenX::Server* in_server, const Jupiter::ReadableString& in_line, upstream_server_info& in_server_info);
	void process_renx_message(RenX::Server& server, upstream_server_info& in_server_info, const Jupiter::ReadableString &raw, Jupiter::ReadableString::TokenizeResult<Jupiter::String_Strict> tokens);

	std::unordered_map<RenX::Server*, std::vector<upstream_server_info>> m_server_info_map;
	std::deque<upstream_server_info*> m_command_tracker; // Tracks the order of REAL commands executed across upstreams, to keep things from getting fudged
	std::chrono::steady_clock::time_point m_init_time{};
	upstream_settings m_default_settings{};
	std::vector<upstream_settings> m_configured_upstreams{};
};

#endif // _RELAY_H_HEADER