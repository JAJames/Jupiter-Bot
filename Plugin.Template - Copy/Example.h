#if !defined _EXAMPLE_H_HEADER
#define _EXAMPLE_H_HEADER

#include "Plugin.h"

class TPlugin : public Jupiter::Plugin
{
	const char *getName() { return "TemplatePlugin"; }
};

#endif // _EXAMPLE_H_HEADER