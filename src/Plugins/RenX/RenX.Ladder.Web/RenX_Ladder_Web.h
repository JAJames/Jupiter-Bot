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
#include "Jupiter/String.hpp"
#include "RenX_Plugin.h"

class RenX_Ladder_WebPlugin : public RenX::Plugin
{
protected:
	Jupiter::String generate_entry_table(RenX::LadderDatabase *db, uint8_t format, size_t index, size_t count);

public:
	const uint8_t FLAG_INCLUDE_PAGE_HEADER = 0x01;
	const uint8_t FLAG_INCLUDE_PAGE_FOOTER = 0x02;
	const uint8_t FLAG_INCLUDE_DATA_HEADER = 0x04;
	const uint8_t FLAG_INCLUDE_DATA_FOOTER = 0x08;
	const uint8_t FLAG_INCLUDE_SEARCH = 0x10;
	const uint8_t FLAG_INCLUDE_SELECTOR = 0x20;

	Jupiter::StringS header;
	Jupiter::StringS footer;
	Jupiter::String *generate_ladder_page(RenX::LadderDatabase *db, uint8_t format, size_t start_index, size_t count, const Jupiter::HTTP::HTMLFormResponse& query_params);
	Jupiter::String *generate_search_page(RenX::LadderDatabase *db, uint8_t format, size_t start_index, size_t count, std::string_view name, const Jupiter::HTTP::HTMLFormResponse& query_params);
	Jupiter::String *generate_profile_page(RenX::LadderDatabase *db, uint8_t format, uint64_t steam_id, const Jupiter::HTTP::HTMLFormResponse& query_params);
	inline size_t getEntriesPerPage() const { return this->entries_per_page; }
	inline size_t getMinSearchNameLength() const { return this->min_search_name_length; };

	virtual bool initialize() override;
	~RenX_Ladder_WebPlugin();

public: // Jupiter::Plugin
	int OnRehash() override;

private:
	void init();

	/** Configuration variables */
	size_t entries_per_page;
	size_t min_search_name_length;
	std::string ladder_page_name, search_page_name, profile_page_name, ladder_table_header, ladder_table_footer;
	std::string web_hostname;
	std::string web_path;
	std::string web_header_filename;
	std::string web_footer_filename;
	std::string web_profile_filename;
	std::string web_ladder_table_header_filename;
	std::string web_ladder_table_footer_filename;

	Jupiter::StringS entry_table_row, entry_profile, entry_profile_previous, entry_profile_next;
};

Jupiter::ReadableString *handle_ladder_page(std::string_view query_string);
Jupiter::ReadableString *handle_search_page(std::string_view query_string);
Jupiter::ReadableString *handle_profile_page(std::string_view query_string);

#endif // _RENX_LADDER_WEB_H