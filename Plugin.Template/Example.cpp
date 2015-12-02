/**
 * This file is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#include "Example.h"



// Plugin instantiation and entry point.
TPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
