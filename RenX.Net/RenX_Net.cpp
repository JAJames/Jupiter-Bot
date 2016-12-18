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

#include "RenX_Net.h"

using namespace Jupiter::literals;

constexpr ProductID RENEGADE_X = 0x01;

constexpr RequestID REQUEST_SERVER_LIST = 0x01;				// version_number
constexpr RequestID REQUEST_SERVER_LIST_UPDATE = 0x02;		// version_number, revision_id
constexpr RequestID REQUEST_SERVER_DATA = 0x03;				// server_id

constexpr int REQUEST_SIZE_TABLE[] =
{
	-1, // Invalid request
	sizeof(int32_t), // REQUEST_SERVER_LIST
	sizeof(int32_t) + sizeof(int32_t) // REQUEST_SERVER_LIST_UPDATE
	// sizeof (int32_t) // REQUEST_SERVER_DATA
};

constexpr size_t minimum_request_size = REQUEST_SIZE_TABLE[REQUEST_SERVER_LIST];		// Size of smallest request
constexpr size_t maximum_request_size = REQUEST_SIZE_TABLE[REQUEST_SERVER_LIST_UPDATE];	// Size of largest request

// Size of packet header
constexpr size_t packet_header_size = sizeof(ProductID) + sizeof(RequestID);

constexpr int getRequestSize(RequestID in_request_id)
{
	return in_request_id >= sizeof(REQUEST_SIZE_TABLE) / sizeof(int)
		? -1 // Invalid request
		: REQUEST_SIZE_TABLE[in_request_id];
}

/** Plugin initialization and destruction */

bool RenX_NetPlugin::initialize() //(const Jupiter::CStringType &bind_address, uint16_t bind_port, bool &out_success, std::chrono::milliseconds in_timeout_period)
{
	m_timeout_period = std::chrono::milliseconds(this->config.getLongLong(Jupiter::ReferenceString::empty, "TimeoutPeriod"_jrs, 10000));

	if (m_server_socket.bind(Jupiter::CStringS(this->config.get(Jupiter::ReferenceString::empty, "Address"_jrs)).c_str(), this->config.getInt(Jupiter::ReferenceString::empty, "Port"_jrs, 21338), true) == false)
		return false;

	m_server_socket.setBlocking(false);
	return true;
}

RenX_NetPlugin::~RenX_NetPlugin()
{
	m_server_socket.close();
}

int RenX_NetPlugin::OnRehash()
{
	Jupiter::Plugin::OnRehash();

	return 0;
}

/** Server List functions */

void RenX_NetPlugin::assignServerIDs()
{
}

void RenX_NetPlugin::addServerToServerList(RenX::Server *server)
{
}

void RenX_NetPlugin::updateServerList()
{
}

/** Client processing */

void RenX_NetPlugin::process_request(RenX_NetPlugin::Client &client)
{
	size_t index;
	const uint32_t *request_params;
	ServerList *list;

	request_params = reinterpret_cast<const uint32_t *>(client.m_request_buffer.ptr());

	switch (client.m_request_id)
	{
	case REQUEST_SERVER_LIST:
		index = 0;
		if (m_server_list.size() != 0)
		{
			while (index != m_server_list.size())
			{
				list = m_server_list.get(index);
				if (list->version_number == *request_params)
				{
					client.sock.send(list->data);
					break;
				}

				if (++index == m_server_list.size())
				{
					// Version not found
					break;
				}
			}
		}
		// else // m_server_list not initialized correctly (misconfiguration)
		break;

	default: // Invalid request. Reset timeout to destroy on next loop.
		client.timeout = std::chrono::steady_clock::now();
		break;
	}

	client.sock.shutdown();
}

int RenX_NetPlugin::think()
{
	RenX_NetPlugin::Client *client;
	Jupiter::Socket *sock;
	int request_packet_size;
	Jupiter::DLList<RenX_NetPlugin::Client>::Node *node;
	Jupiter::DLList<RenX_NetPlugin::Client>::Node *tmp;

	// handle new clients
	while ((sock = m_server_socket.accept()) != nullptr)
	{
		sock->setBlocking(false);
		request_packet_size = sock->peek();

		if (request_packet_size > maximum_request_packet_size // This is larger than expected (invalid) request. Toss the socket.
			|| (request_packet_size <= 0 && sock->getLastError() != 10035)) // Error!
			delete sock;
		else
		{
			client = new RenX_NetPlugin::Client();
			client->sock = std::move(*sock);
			client->timeout = std::chrono::steady_clock::now() + m_timeout_period;

			if (request_packet_size == expected_request_packet_size) // we have a full request
			{
				process_request(*client);

				m_clients_pending_close.add(client);
			}
			else // request is not complete; toss it to further processing
				m_clients_processing.add(client);
		}
	}
	
	// check processing clients for remainder of data; timeout as necessary
	if (m_clients_processing.size() != 0)
	{
		node = m_clients_processing.getNode(0);
		while (node != nullptr)
		{
			client = node->data;
			request_packet_size = client->sock.peek();

			if (request_packet_size == expected_request_packet_size) // Full request
			{
				process_request(*client);

				tmp = node;
				node = node->next;

				m_clients_processing.remove(tmp);

				client->timeout = std::chrono::steady_clock::now() + m_timeout_period;
				m_clients_pending_close.add(client);
			}
			else if (request_packet_size > expected_request_packet_size // Invalid request
				|| client->timeout <= std::chrono::steady_clock::now() // Incomplete request & timed out
				|| (request_packet_size <= 0 && client->sock.getLastError() != 10035)) // Error!
			{
				client->sock.close();

				tmp = node;
				node = node->next;

				delete m_clients_processing.remove(tmp);
			}
			else // Incomplete request
				node = node->next;
		}
	}

	// check old clients for clean close; timeout as necessary
	if (m_clients_pending_close.size() != 0)
	{
		node = m_clients_pending_close.getNode(0);
		while (node != nullptr)
		{
			client = node->data;
			request_packet_size = client->sock.recv();

			if (request_packet_size == 0 // Shutdown successfully
				|| (request_packet_size < 0 && client->sock.getLastError() != 10035) // Error!
				|| client->timeout <= std::chrono::steady_clock::now()) // Timeout
			{
				client->sock.close();

				tmp = node;
				node = node->next;

				delete m_clients_pending_close.remove(tmp);
			}
			else // Still pending; go to next
				node = node->next;
		}
	}

	return 0;
}

/** Events */

void RenX_NetPlugin::RenX_OnServerFullyConnected(RenX::Server *server)
{
	if (++m_server_id == 0) // Overflow; this will actually invalidate the results of any previous server list
		this->assignServerIDs();
	else
		server->varData.set(this->getName(), "i"_jrs, Jupiter::ReferenceString(reinterpret_cast<const char *>(&m_server_id), sizeof(m_server_id)));

	// Server added; append
	this->addServerToServerList(server);
}

void RenX_NetPlugin::RenX_OnServerDisconnect(RenX::Server *server, RenX::DisconnectReason reason)
{
	// Server removed; update
	this->updateServerList();
}

void RenX_NetPlugin::RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player)
{
	// User count changed; update
	this->updateServerList();
}

void RenX_NetPlugin::RenX_OnPart(RenX::Server *server, const RenX::PlayerInfo *player)
{
	// User count changed; update
	this->updateServerList();
}

void RenX_NetPlugin::RenX_OnMapLoad(RenX::Server *server, const Jupiter::ReadableString &map)
{
	// Level name changed; update
	this->updateServerList();
}

// Plugin instantiation and entry point.
RenX_NetPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
