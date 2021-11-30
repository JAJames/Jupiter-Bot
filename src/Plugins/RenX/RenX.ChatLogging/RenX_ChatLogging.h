/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Sarah E. <sarah.evans@qq.com>
 */

#if !defined _RENX_CHATLOG_H_HEADER
#define _RENX_CHATLOG_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "RenX_Plugin.h"

class RenX_ChatLogPlugin : public RenX::Plugin
{
public: // Jupiter::Plugin
	bool initialize() override;
	~RenX_ChatLogPlugin();

public: // RenX::Plugin
	void RenX_OnTeamChat(RenX::Server& server, const RenX::PlayerInfo& player, std::string_view  message) override;
	void RenX_OnChat(RenX::Server& server, const RenX::PlayerInfo& player, std::string_view  message) override;

public: 
	void PrepFile();
	void WriteToLog(RenX::Server& server, const RenX::PlayerInfo& player, std::string_view  message, std::string in_prefix);
	std::string last_date;
	std::fstream fs;
};

#endif // _RENX_CHATLOG_H_HEADER