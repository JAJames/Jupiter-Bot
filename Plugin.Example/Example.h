/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#if !defined _EXAMPLE_H_HEADER
#define _EXAMPLE_H_HEADER

#include "Jupiter/Plugin.h"
#include "IRC_Command.h"

class ExamplePlugin : public Jupiter::Plugin
{
public:
	void OnConnect(Jupiter::IRC::Client *server);
};

// Example IRC Command Declaration
GENERIC_IRC_COMMAND(ExampleIRCCommand)

#endif // _EXAMPLE_H_HEADER