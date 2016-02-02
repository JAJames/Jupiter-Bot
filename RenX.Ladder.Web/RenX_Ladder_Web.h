/**
 * Copyright (C) 2016 Jessica James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#if !defined _RENX_LADDER_WEB_H
#define _RENX_LADDER_WEB_H

#include "Jupiter/Plugin.h"
#include "Jupiter/Reference_String.h"
#include "Jupiter/CString.h"
#include "Jupiter/INIFile.h"
#include "RenX_Plugin.h"

class RenX_Ladder_WebPlugin : public RenX::Plugin
{
protected:
	Jupiter::String generate_entry_table(RenX::LadderDatabase *db, size_t index, size_t count);

public:
	Jupiter::StringS header;
	Jupiter::StringS footer;
	Jupiter::String *generate_ladder_page(RenX::LadderDatabase *db, size_t start_index, size_t count, const Jupiter::INIFile::Section &query_params);
	Jupiter::String *generate_search_page(RenX::LadderDatabase *db, const Jupiter::ReadableString &name, const Jupiter::INIFile::Section &query_params);
	Jupiter::String *generate_profile_page(RenX::LadderDatabase *db, uint64_t steam_id, const Jupiter::INIFile::Section &query_params);

	RenX_Ladder_WebPlugin();
	~RenX_Ladder_WebPlugin();

public: // Jupiter::Plugin
	const Jupiter::ReadableString &getName() override { return name; }
	int OnRehash() override;

private:
	STRING_LITERAL_AS_NAMED_REFERENCE(name, "RenX.Ladder.Web");

	/** Configuration variables */
	Jupiter::StringS ladder_page_name, search_page_name, profile_page_name, ladder_table_header, ladder_table_footer;
	Jupiter::StringS web_hostname;
	Jupiter::StringS web_path;
	Jupiter::CStringS web_header_filename;
	Jupiter::CStringS web_footer_filename;
	Jupiter::CStringS web_profile_filename;
	Jupiter::CStringS web_ladder_table_header_filename;
	Jupiter::CStringS web_ladder_table_footer_filename;

	Jupiter::StringS entry_table_row, entry_profile;
};

Jupiter::ReadableString *handle_ladder_page(const Jupiter::ReadableString &parameters);
Jupiter::ReadableString *handle_search_page(const Jupiter::ReadableString &parameters);
Jupiter::ReadableString *handle_profile_page(const Jupiter::ReadableString &parameters);

#endif // _RENX_LADDER_WEB_H