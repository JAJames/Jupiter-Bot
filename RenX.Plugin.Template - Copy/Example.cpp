#include "Example.h"



// Plugin instantiation and entry point.
RenX_TPlugin pluginInstance;

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
