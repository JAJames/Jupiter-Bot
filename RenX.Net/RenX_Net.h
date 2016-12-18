/**
 * Copyright (C) 2016 Jessica James.
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

#if !defined _RENX_NET_GAME_H_HEADER
#define _RENX_NET_GAME_H_HEADER

#include <chrono>
#include "Jupiter/Thinker.h"
#include "Jupiter/CString.h"
#include "Jupiter/TCPSocket.h"
#include "Jupiter/DLList.h"
#include "Jupiter/ArrayList.h"
#include "RenX_Plugin.h"
#include "RenX_Server.h"

typedef uint8_t ProductID;
typedef uint8_t RequestID;
typedef uint32_t ServerID;

class RenX_NetPlugin : public RenX::Plugin
{
public:

	struct Client
	{
		Jupiter::Socket sock;
		std::chrono::steady_clock::time_point timeout;

		ProductID m_product_id;
		RequestID m_request_id;
		Jupiter::String m_request_buffer;
	};

	struct ServerList
	{
		uint32_t version_number = 0;
		uint32_t revision_id = 0;

		Jupiter::String data;
		Jupiter::ArrayList<RenX::Server> servers;
	};

	void assignServerIDs();
	void addServerToServerList(RenX::Server *server);
	void updateServerList();
	~RenX_NetPlugin();

public: // Jupiter::Plugin
	virtual bool initialize() override;
	int OnRehash() override;
	int think() override;

public: // RenX::Plugin
	void RenX_OnServerFullyConnected(RenX::Server *server) override;
	void RenX_OnServerDisconnect(RenX::Server *server, RenX::DisconnectReason reason) override;
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnMapLoad(RenX::Server *server, const Jupiter::ReadableString &map) override;

private:
	void process_request(Client &client);

	std::chrono::milliseconds m_ping_frequency;
	std::chrono::milliseconds m_ping_timeout;

	Jupiter::TCPSocket m_server_socket;
	Jupiter::DLList<Client> m_clients;
	Jupiter::DLList<Client> m_clients_pending_close;

	uint32_t m_server_id;

	uint32_t m_min_server_version;
	uint32_t m_max_server_version;

	Jupiter::ArrayList<ServerList> m_server_list;
};

#endif // _RENX_SERVERLIST_GAME_H_HEADER