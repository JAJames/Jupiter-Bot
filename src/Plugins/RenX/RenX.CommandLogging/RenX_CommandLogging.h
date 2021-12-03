/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Sarah E. <sarah.evans@qq.com>
 */

#if !defined _RENX_CMDLOGGING_H_HEADER
#define _RENX_CMDLOGGING_H_HEADER

#include "Jupiter/Plugin.h"
#include "RenX_Plugin.h"

class RenX_CommandLoggingPlugin : public RenX::Plugin
{
public: // Jupiter::Plugin
	bool initialize() override;
	~RenX_CommandLoggingPlugin();

public: // RenX::Plugin
	void RenX_OnCommandTriggered(RenX::Server& server, std::string_view  trigger, RenX::PlayerInfo& player, std::string_view  parameters, RenX::GameCommand& command) override;

public:
	void PrepFile();
	void WriteToLog(RenX::Server& server, const RenX::PlayerInfo& player, std::string_view  message);
	std::string last_date;
	std::fstream fs;
	int min_access;
	int min_cmd_access;
};

#endif // _RENX_CMDLOGGING_H_HEADER