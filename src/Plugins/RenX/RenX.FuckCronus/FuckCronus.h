/**
 * Copyright (C) 2021 Jessica James. All rights reserved.
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#if !defined _FUCKCRONUS_H_HEADER
#define _FUCKCRONUS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/TCPSocket.h"
#include "RenX_Plugin.h"

class RenX_FuckCronusPlugin : public RenX::Plugin
{
public: // Jupiter::Thinker
	int think() override;

public: // Jupiter::Plugin
	bool initialize() override;

public: // RenX::Plugin
	void RenX_OnServerCreate(RenX::Server &server) override;
	void RenX_OnServerDisconnect(RenX::Server &server, RenX::DisconnectReason reason) override;
	void RenX_OnRaw(RenX::Server &server, const Jupiter::ReadableString &raw) override;

private:
	struct ext_server_info {
		std::unique_ptr<Jupiter::TCPSocket> m_socket;
		bool m_devbot_connected{};
		std::chrono::steady_clock::time_point m_last_connect_attempt{};
		std::chrono::steady_clock::time_point m_last_activity{};
		Jupiter::StringL m_last_line;
	};

	void devbot_connected(RenX::Server& in_server, ext_server_info& in_server_info);
	void devbot_disconnected(RenX::Server& in_server, ext_server_info& in_server_info);
	void process_devbot_message(RenX::Server* in_server, const Jupiter::ReadableString& in_line);

	std::unordered_map<RenX::Server*, ext_server_info> m_server_info_map;
	std::chrono::steady_clock::time_point m_init_time{};
	bool m_sanitize_names{};
	bool m_sanitize_ips{};
	bool m_sanitize_hwids{};
	bool m_sanitize_steam_ids{};
	bool m_sanitize_unknown_commands{};
	bool m_sanitize_blacklisted_commands{};
	bool m_suppress_chat_logs{};
};

#endif // _FUCKCRONUS_H_HEADER