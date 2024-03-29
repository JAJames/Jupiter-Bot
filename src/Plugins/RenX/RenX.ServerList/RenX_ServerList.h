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

#if !defined _RENX_SERVERLIST_H_HEADER
#define _RENX_SERVERLIST_H_HEADER

#include "Jupiter/Plugin.h"
#include "RenX_Plugin.h"

class RenX_ServerListPlugin : public RenX::Plugin
{
public: // RenX_ServerListPlugin
	struct ListServerInfo {
		std::string_view hostname;
		unsigned short port;
		std::string_view namePrefix;
		std::vector<std::string_view> attributes;
	};

	size_t getListedPlayerCount(const RenX::Server& server);

	std::string* getServerListJSON();
	std::string* getMetadataJSON();
	std::string* getMetadataPrometheus();

	void addServerToServerList(RenX::Server &server);
	void updateServerList();
	void updateMetadata();
	void markDetailsStale(RenX::Server& in_server);
	void touchDetails(RenX::Server& in_server);
	std::string_view getListServerAddress(const RenX::Server& server);
	ListServerInfo getListServerInfo(const RenX::Server& server);
	std::string server_as_json(const RenX::Server &server);
	std::string server_as_server_details_json(const RenX::Server& server);
	std::string server_as_long_json(const RenX::Server &server);

	virtual bool initialize() override;
	~RenX_ServerListPlugin();

public: // RenX::Plugin
	void RenX_OnServerFullyConnected(RenX::Server &server) override;
	void RenX_OnServerDisconnect(RenX::Server &server, RenX::DisconnectReason reason) override;
	void RenX_OnJoin(RenX::Server &server, const RenX::PlayerInfo &player) override;
	void RenX_OnPart(RenX::Server &server, const RenX::PlayerInfo &player) override;
	void RenX_OnMapLoad(RenX::Server &server, std::string_view map) override;

private:
	std::string m_server_list_json, m_metadata_json, m_metadata_prometheus;
	std::string m_web_hostname, m_web_path;
	std::string m_server_list_page_name, m_server_list_long_page_name, m_server_page_name, m_metadata_page_name, m_metadata_prometheus_page_name;
};

std::string* handle_server_list_page(std::string_view);
std::string* handle_server_list_long_page(std::string_view);
std::string* handle_server_page(std::string_view);
std::string* handle_metadata_page(std::string_view);
std::string* handle_metadata_prometheus_page(std::string_view);

#endif // _RENX_SERVERLIST_H_HEADER
