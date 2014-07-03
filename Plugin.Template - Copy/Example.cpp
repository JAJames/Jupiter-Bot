#include "Example.h"



// Plugin instantiation and entry point.
TPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
