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

#include "Jupiter/IRC_Client.h"
#include "Jupiter/INIFile.h"
#include "Jupiter/HTTP.h"
#include "Jupiter/HTTP_QueryString.h"
#include "HTTPServer.h"
#include "RenX_Tags.h"
#include "RenX_Ladder_Web.h"

using namespace Jupiter::literals;

RenX_Ladder_WebPlugin::RenX_Ladder_WebPlugin()
{
	RenX_Ladder_WebPlugin::ladder_page_name = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "LadderPageName"_jrs, ""_jrs);
	RenX_Ladder_WebPlugin::search_page_name = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "SearchPageName"_jrs, "search"_jrs);
	RenX_Ladder_WebPlugin::profile_page_name = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "ProfilePageName"_jrs, "profile"_jrs);
	RenX_Ladder_WebPlugin::web_hostname = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "Hostname"_jrs, ""_jrs);
	RenX_Ladder_WebPlugin::web_path = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "Path"_jrs, "/"_jrs);

	this->OnRehash();

	/** Initialize content */
	Jupiter::HTTP::Server &server = getHTTPServer();

	Jupiter::HTTP::Server::Content *content = new Jupiter::HTTP::Server::Content(RenX_Ladder_WebPlugin::ladder_page_name, handle_ladder_page);
	content->language = &Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = &Jupiter::HTTP::Content::Type::Text::HTML;
	content->charset = &Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	server.hook(RenX_Ladder_WebPlugin::web_hostname, RenX_Ladder_WebPlugin::web_path, content);

	content = new Jupiter::HTTP::Server::Content(RenX_Ladder_WebPlugin::search_page_name, handle_search_page);
	content->language = &Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = &Jupiter::HTTP::Content::Type::Text::HTML;
	content->charset = &Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	server.hook(RenX_Ladder_WebPlugin::web_hostname, RenX_Ladder_WebPlugin::web_path, content);

	content = new Jupiter::HTTP::Server::Content(RenX_Ladder_WebPlugin::profile_page_name, handle_profile_page);
	content->language = &Jupiter::HTTP::Content::Language::ENGLISH;
	content->type = &Jupiter::HTTP::Content::Type::Text::HTML;
	content->charset = &Jupiter::HTTP::Content::Type::Text::Charset::UTF8;
	server.hook(RenX_Ladder_WebPlugin::web_hostname, RenX_Ladder_WebPlugin::web_path, content);
}

RenX_Ladder_WebPlugin::~RenX_Ladder_WebPlugin()
{
	Jupiter::HTTP::Server &server = getHTTPServer();
	server.remove(RenX_Ladder_WebPlugin::web_hostname, RenX_Ladder_WebPlugin::web_path, RenX_Ladder_WebPlugin::ladder_page_name);
	server.remove(RenX_Ladder_WebPlugin::web_hostname, RenX_Ladder_WebPlugin::web_path, RenX_Ladder_WebPlugin::search_page_name);
	server.remove(RenX_Ladder_WebPlugin::web_hostname, RenX_Ladder_WebPlugin::web_path, RenX_Ladder_WebPlugin::profile_page_name);
}

int RenX_Ladder_WebPlugin::OnRehash()
{
	FILE *file;
	int chr;

	RenX_Ladder_WebPlugin::web_header_filename = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "HeaderFilename"_jrs, "RenX.Ladder.Web.Header.html"_jrs);
	RenX_Ladder_WebPlugin::web_footer_filename = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "FooterFilename"_jrs, "RenX.Ladder.Web.Footer.html"_jrs);
	RenX_Ladder_WebPlugin::web_profile_filename = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "ProfileFilename"_jrs, "RenX.Ladder.Web.Profile.html"_jrs);
	RenX_Ladder_WebPlugin::web_ladder_table_header_filename = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "LadderTableHeaderFilename"_jrs, "RenX.Ladder.Web.Ladder.Table.Header.html"_jrs);
	RenX_Ladder_WebPlugin::web_ladder_table_footer_filename = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "LadderTableFooterFilename"_jrs, "RenX.Ladder.Web.Ladder.Table.Footer.html"_jrs);
	
	RenX_Ladder_WebPlugin::entry_table_row = Jupiter::IRC::Client::Config->get(RenX_Ladder_WebPlugin::name, "EntryTableRow"_jrs, "<tr><td>{RANK}</td><td><a href=\"profile?id={STEAM}\">{NAME}</a></td><td>{SCORE}</td><td>{SPM}</td><td>{KILLS}</td><td>{DEATHS}</td><td>{KDR}</td><td>{NODGAMES}</td><td>{NODWINS}</td><td>{NODLOSSES}</td><td>{NWLR}</td><td>{GDIGAMES}</td><td>{GDIWINS}</td><td>{GDILOSSES}</td><td>{GWLR}</td></tr>"_jrs);
	RenX::sanitizeTags(RenX_Ladder_WebPlugin::entry_table_row);

	RenX_Ladder_WebPlugin::header.erase();
	RenX_Ladder_WebPlugin::footer.erase();
	RenX_Ladder_WebPlugin::entry_profile.erase();
	RenX_Ladder_WebPlugin::ladder_table_header.erase();
	RenX_Ladder_WebPlugin::ladder_table_footer.erase();

	/** Load header */
	if (RenX_Ladder_WebPlugin::web_header_filename.isNotEmpty())
	{
		file = fopen(RenX_Ladder_WebPlugin::web_header_filename.c_str(), "rb");
		if (file != nullptr)
		{
			while ((chr = fgetc(file)) != EOF)
				RenX_Ladder_WebPlugin::header += chr;
			fclose(file);
		}
	}

	/** Load footer */
	if (RenX_Ladder_WebPlugin::web_footer_filename.isNotEmpty())
	{
		file = fopen(RenX_Ladder_WebPlugin::web_footer_filename.c_str(), "rb");
		if (file != nullptr)
		{
			while ((chr = fgetc(file)) != EOF)
				RenX_Ladder_WebPlugin::footer += chr;
			fclose(file);
		}
	}

	/** Load profile */
	if (RenX_Ladder_WebPlugin::web_profile_filename.isNotEmpty())
	{
		file = fopen(RenX_Ladder_WebPlugin::web_profile_filename.c_str(), "rb");
		if (file != nullptr)
		{
			while ((chr = fgetc(file)) != EOF)
				RenX_Ladder_WebPlugin::entry_profile += chr;
			RenX::sanitizeTags(RenX_Ladder_WebPlugin::entry_profile);
			fclose(file);
		}
	}

	/** Load table header */
	if (RenX_Ladder_WebPlugin::web_ladder_table_header_filename.isNotEmpty())
	{
		file = fopen(RenX_Ladder_WebPlugin::web_ladder_table_header_filename.c_str(), "rb");
		if (file != nullptr)
		{
			while ((chr = fgetc(file)) != EOF)
				RenX_Ladder_WebPlugin::ladder_table_header += chr;
			fclose(file);
		}
	}

	/** Load table footer */
	if (RenX_Ladder_WebPlugin::web_ladder_table_footer_filename.isNotEmpty())
	{
		file = fopen(RenX_Ladder_WebPlugin::web_ladder_table_footer_filename.c_str(), "rb");
		if (file != nullptr)
		{
			while ((chr = fgetc(file)) != EOF)
				RenX_Ladder_WebPlugin::ladder_table_footer += chr;
			fclose(file);
		}
	}

	return 0;
}

/** Ladder page */

Jupiter::String RenX_Ladder_WebPlugin::generate_entry_table(size_t index, size_t count)
{
	if (RenX::ladder_databases.size() == 0)
		return Jupiter::String("Error: No ladder databases loaded"_jrs);

	RenX::LadderDatabase *db = RenX::ladder_databases.get(0);

	if (db->getEntries() == 0) // No ladder data
		return Jupiter::String("Error: No ladder data"_jrs);

	if (index >= db->getEntries() || count == 0) // Invalid entry range
		return Jupiter::String("Error: Invalid range"_jrs);

	if (index + count > db->getEntries()) // Invalid entry range; use valid portion of range
		count = db->getEntries() - index;

	RenX::LadderDatabase::Entry *node = db->getHead();

	// iterate to requested index
	while (index != 0)
	{
		node = node->next;
		--index;
	}

	// table header
	Jupiter::String result(2048);
	result = RenX_Ladder_WebPlugin::ladder_table_header;

	// append rows
	Jupiter::String row(256);
	while (count != 0)
	{
		row = RenX_Ladder_WebPlugin::entry_table_row;
		RenX::processTags(row, *node);
		result += row;
		--count;
	}

	// table footer
	result += RenX_Ladder_WebPlugin::ladder_table_footer;
	return result;
}

Jupiter::String *RenX_Ladder_WebPlugin::generate_ladder_page(size_t index, size_t count)
{
	Jupiter::String *result = new Jupiter::String(RenX_Ladder_WebPlugin::header);
	result->concat(this->generate_entry_table(index, count));
	result->concat(RenX_Ladder_WebPlugin::footer);
	return result;
}

/** Search page */
Jupiter::String *RenX_Ladder_WebPlugin::generate_search_page(const Jupiter::ReadableString &name)
{
	Jupiter::String *result = new Jupiter::String(RenX_Ladder_WebPlugin::header);

	if (RenX::ladder_databases.size() == 0)
	{
		result->concat("Error: No ladder databases loaded"_jrs);
		result->concat(RenX_Ladder_WebPlugin::footer);
		return result;
	}

	RenX::LadderDatabase *db = RenX::ladder_databases.get(0);

	if (db->getEntries() == 0) // No ladder data
	{
		result->concat("Error: No ladder data"_jrs);
		result->concat(RenX_Ladder_WebPlugin::footer);
		return result;
	}

	result->concat(RenX_Ladder_WebPlugin::ladder_table_header);

	// append rows
	Jupiter::String row(256);
	RenX::LadderDatabase::Entry *node = db->getHead();
	while (node != nullptr)
	{
		if (node->most_recent_name.findi(name) != Jupiter::INVALID_INDEX) // match found
		{
			row = RenX_Ladder_WebPlugin::entry_table_row;
			RenX::processTags(row, *node);
			result->concat(row);
		}
		node = node->next;
	}
	
	result->concat(RenX_Ladder_WebPlugin::ladder_table_footer);
	result->concat(RenX_Ladder_WebPlugin::footer);
	return result;
}

/** Profile page */
Jupiter::String *RenX_Ladder_WebPlugin::generate_profile_page(uint64_t steam_id)
{
	Jupiter::String *result = new Jupiter::String(RenX_Ladder_WebPlugin::header);

	if (RenX::ladder_databases.size() == 0)
	{
		result->concat("Error: No ladder databases loaded"_jrs);
		result->concat(RenX_Ladder_WebPlugin::footer);
		return result;
	}

	RenX::LadderDatabase *db = RenX::ladder_databases.get(0);

	if (db->getEntries() == 0) // No ladder data
	{
		result->concat("Error: No ladder data"_jrs);
		result->concat(RenX_Ladder_WebPlugin::footer);
		return result;
	}

	RenX::LadderDatabase::Entry *entry = db->getHead();
	while (entry != nullptr)
	{
		if (entry->steam_id == steam_id) // match found
			break;
		entry = entry->next;
	}

	if (entry == nullptr)
		result->concat("Error: Player not found"_jrs);
	else
	{
		Jupiter::String profile_data(RenX_Ladder_WebPlugin::entry_profile);
		RenX::processTags(profile_data, *entry);
		result->concat(profile_data);
	}

	result->concat(RenX_Ladder_WebPlugin::footer);
	return result;
}

// Plugin instantiation and entry point.
RenX_Ladder_WebPlugin pluginInstance;

/** Content functions */

Jupiter::ReadableString *handle_ladder_page(const Jupiter::ReadableString &query_string)
{
	size_t start_index = 0, count = 50;

	if (query_string.isNotEmpty())
	{
		Jupiter::HTTP::HTMLFormResponse html_form_response(query_string);
		start_index = static_cast<size_t>(html_form_response.table.getInt("start"_jrs, 0));
		count = static_cast<size_t>(html_form_response.table.getInt("count"_jrs, 50));
	}
	return pluginInstance.generate_ladder_page(start_index, count);
}

Jupiter::ReadableString *handle_search_page(const Jupiter::ReadableString &query_string)
{
	Jupiter::ReferenceString name;
	if (query_string.isNotEmpty())
	{
		Jupiter::HTTP::HTMLFormResponse html_form_response(query_string);
		name = html_form_response.table.get("name"_jrs);
	}

	if (name.isEmpty()) // Generate ladder page when no name specified
		return handle_ladder_page(query_string);

	return pluginInstance.generate_search_page(name);
}

Jupiter::ReadableString *handle_profile_page(const Jupiter::ReadableString &query_string)
{
	uint64_t steam_id = 0;
	if (query_string.isNotEmpty())
	{
		Jupiter::HTTP::HTMLFormResponse html_form_response(query_string);
		steam_id = html_form_response.table.getLongLong("id"_jrs);
	}

	return pluginInstance.generate_profile_page(steam_id);
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
