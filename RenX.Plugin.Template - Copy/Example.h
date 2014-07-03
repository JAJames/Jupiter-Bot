#if !defined _EXAMPLE_H_HEADER
#define _EXAMPLE_H_HEADER

#include "Plugin.h"
#include "RenX_Plugin.h"

class RenX_TPlugin : public RenX::Plugin
{
	const char *getName() { return "RenX_TemplatePlugin"; }
};

#endif // _EXAMPLE_H_HEADER