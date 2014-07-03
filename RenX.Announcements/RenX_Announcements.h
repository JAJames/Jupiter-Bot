/**
 * Copyright (C) 2014 Justin James.
 *
 * This license must be preserved.
 * Any applications, libraries, or code which make any use of any
 * component of this program must not be commercial, unless explicit
 * permission is granted from the original author. The use of this
 * program for non-profit purposes is permitted.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In the event that this license restricts you from making desired use of this program, contact the original author.
 * Written by Justin James <justin.aj@hotmail.com>
 */

#if !defined _RENX_ANNOUNCEMENTS_H_HEADER
#define _RENX_ANNOUNCEMENTS_H_HEADER

#include "Jupiter/Plugin.h"
#include "Jupiter/Timer.h"
#include "Jupiter/File.h"
#include "RenX_Plugin.h"

class RenX_AnnouncementsPlugin : public RenX::Plugin
{
public:
	const Jupiter::ReadableString &getName() override { return name; }
	RenX_AnnouncementsPlugin();
	~RenX_AnnouncementsPlugin();

public:
	Jupiter::File announcementsFile;
	bool random;
	unsigned int lastLine;

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Announcements");
	Jupiter::Timer *timer;
};

#endif // _RENX_ANNOUNCEMENTS_H_HEADER