/**
 * Copyright (C) 2015-2016 Jessica James.
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

#if !defined _CHANNELRELAY_H_HEADER
#define _CHANNELRELAY_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"

class ChannelRelayPlugin : public Jupiter::Plugin
{
public: // Jupiter::Plugin
	void OnChat(Jupiter::IRC::Client *server, std::string_view channel, std::string_view nick, std::string_view message) override;
	int OnRehash() override;

	virtual bool initialize() override;

private:
	std::vector<int> m_types;
};

#endif // _CHANNELRELAY_H_HEADER