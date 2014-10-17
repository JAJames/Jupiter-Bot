/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Justin James <justin.aj@hotmail.com>
 */

#if !defined _RENX_DNS_H_HEADER
#define _RENX_DNS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "RenX_Plugin.h"

class RenX_DNSPlugin : public RenX::Plugin
{
public: // RenX::Plugin
	void RenX_OnPlayerCreate(RenX::Server *server, const RenX::PlayerInfo *player) override;
	void RenX_OnJoin(RenX::Server *server, const RenX::PlayerInfo *player) override;

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;
	RenX_DNSPlugin();

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.DNS");
	bool resolveAdmin;
	bool resolvePublic;
};

#endif // _RENX_DNS_H_HEADER