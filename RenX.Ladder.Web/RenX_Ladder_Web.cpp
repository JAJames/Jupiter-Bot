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

/** Search bar */
Jupiter::String generate_search(RenX::LadderDatabase *db)
{
	Jupiter::String result(256);

	result = R"database-search(<form action="search" method="get" class="leaderboard-search"><input type="text" class="leaderboard-input" name="name" size="30" placeholder="Player name" value=""/>)database-search"_jrs;

	if (db != nullptr && db != RenX::default_ladder_database)
	{
		result += R"database-search(<input type="hidden" name="database" value=")database-search"_jrs;
		result += db->getName();
		result += R"database-search("/>)database-search"_jrs;
	}
	result += R"database-search(<input type="submit"  class="leaderboard-input" value="Search"/></form>)database-search"_jrs;
	return result;
}

/** Database selector */
Jupiter::String generate_database_selector(RenX::LadderDatabase *db, const Jupiter::INIFile::Section &query_params)
{
	RenX::LadderDatabase *db_ptr;
	Jupiter::String result(256);

	result = R"database-select(<form method="get" class="database-select-form"><select name="database" class="database-select">)database-select"_jrs;
	if (db != nullptr)
	{
		result += "<option value=\""_jrs;
		result += db->getName();
		result += "\">"_jrs;
		result += db->getName();
		result += "</option>"_jrs;
	}
	else if (RenX::ladder_databases.size() == 0)
		return Jupiter::String::empty;

	for (size_t index = 0; index != RenX::ladder_databases.size(); ++index)
	{
		db_ptr = RenX::ladder_databases.get(index);
		if (db_ptr != db)
		{
			db_ptr = RenX::ladder_databases.get(index);
			result += "<option value=\""_jrs;
			result += db_ptr->getName();
			result += "\">"_jrs;
			result += db_ptr->getName();
			result += "</option>"_jrs;
		}
	}
	
	Jupiter::INIFile::Section::KeyValuePair *pair;
	for (size_t index = 0; index != query_params.size(); ++index)
	{
		pair = query_params.getPair(index);
		if (pair->getKey().equalsi("database") == false)
		{
			result += R"database-search(<input type="hidden" name="id" value=")database-search"_jrs;
			result += pair->getValue();
			result += R"database-search("/>)database-search"_jrs;
		}
	}

	result += R"database-select(</select><input type="submit" class="leaderboard-input" value="Go"/></form>)database-select"_jrs;
	return result;
}

/** Ladder page */

Jupiter::String RenX_Ladder_WebPlugin::generate_entry_table(RenX::LadderDatabase *db, size_t index, size_t count)
{
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
		node = node->next;
		--count;
	}

	// table footer
	result += RenX_Ladder_WebPlugin::ladder_table_footer;
	return result;
}

Jupiter::String *RenX_Ladder_WebPlugin::generate_ladder_page(RenX::LadderDatabase *db, size_t index, size_t count, const Jupiter::INIFile::Section &query_params)
{
	Jupiter::String *result = new Jupiter::String(RenX_Ladder_WebPlugin::header);
	result->concat(generate_search(db));
	result->concat(generate_database_selector(db, query_params));
	result->concat(this->generate_entry_table(db, index, count));
	result->concat(RenX_Ladder_WebPlugin::footer);
	return result;
}

/** Search page */
Jupiter::String *RenX_Ladder_WebPlugin::generate_search_page(RenX::LadderDatabase *db, const Jupiter::ReadableString &name, const Jupiter::INIFile::Section &query_params)
{
	Jupiter::String *result = new Jupiter::String(RenX_Ladder_WebPlugin::header);
	result->concat(generate_search(db));
	result->concat(generate_database_selector(db, query_params));

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
Jupiter::String *RenX_Ladder_WebPlugin::generate_profile_page(RenX::LadderDatabase *db, uint64_t steam_id, const Jupiter::INIFile::Section &query_params)
{
	Jupiter::String *result = new Jupiter::String(RenX_Ladder_WebPlugin::header);
	result->concat(generate_search(db));
	result->concat(generate_database_selector(db, query_params));

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

Jupiter::ReadableString *generate_no_db_page(const Jupiter::INIFile::Section &query_params)
{
	Jupiter::String *result = new Jupiter::String(pluginInstance.header);
	if (RenX::ladder_databases.size() != 0)
	{
		result->concat(generate_search(nullptr));
		result->concat(generate_database_selector(nullptr, query_params));
		result->concat("Error: No such database exists"_jrs);
	}
	else
		result->concat("Error: No ladder databases loaded"_jrs);
	result->concat(pluginInstance.footer);
	return result;
}

Jupiter::ReadableString *handle_ladder_page(const Jupiter::ReadableString &query_string)
{
	Jupiter::HTTP::HTMLFormResponse html_form_response(query_string);
	RenX::LadderDatabase *db = RenX::default_ladder_database;
	size_t start_index = 0, count = 50;

	if (html_form_response.table.size() != 0)
	{
		start_index = static_cast<size_t>(html_form_response.table.getInt("start"_jrs, 0));
		count = static_cast<size_t>(html_form_response.table.getInt("count"_jrs, 50));
		
		const Jupiter::ReadableString &db_name = html_form_response.table.get("database"_jrs);
		if (db_name.isNotEmpty())
		{
			db = nullptr;
			for (size_t index = 0; index != RenX::ladder_databases.size(); ++index)
				if (RenX::ladder_databases.get(index)->getName().equalsi(db_name))
				{
					db = RenX::ladder_databases.get(index);
					break;
				}
		}
	}

	if (db == nullptr)
		return generate_no_db_page(html_form_response.table);

	return pluginInstance.generate_ladder_page(db, start_index, count, html_form_response.table);
}

Jupiter::ReadableString *handle_search_page(const Jupiter::ReadableString &query_string)
{
	Jupiter::HTTP::HTMLFormResponse html_form_response(query_string);
	RenX::LadderDatabase *db = RenX::default_ladder_database;
	Jupiter::ReferenceString name;

	if (html_form_response.table.size() != 0)
	{
		name = html_form_response.table.get("name"_jrs);

		const Jupiter::ReadableString &db_name = html_form_response.table.get("database"_jrs);
		if (db_name.isNotEmpty())
		{
			db = nullptr;
			for (size_t index = 0; index != RenX::ladder_databases.size(); ++index)
				if (RenX::ladder_databases.get(index)->getName().equalsi(db_name))
				{
					db = RenX::ladder_databases.get(index);
					break;
				}
		}
	}

	if (db == nullptr)
		return generate_no_db_page(html_form_response.table);

	if (name.isEmpty()) // Generate ladder page when no name specified
		return handle_ladder_page(query_string);

	return pluginInstance.generate_search_page(db, name, html_form_response.table);
}

Jupiter::ReadableString *handle_profile_page(const Jupiter::ReadableString &query_string)
{
	Jupiter::HTTP::HTMLFormResponse html_form_response(query_string);
	RenX::LadderDatabase *db = RenX::default_ladder_database;
	uint64_t steam_id = 0;

	if (html_form_response.table.size() != 0)
	{
		steam_id = html_form_response.table.getLongLong("id"_jrs);

		const Jupiter::ReadableString &db_name = html_form_response.table.get("database"_jrs);
		if (db_name.isNotEmpty())
		{
			db = nullptr;
			for (size_t index = 0; index != RenX::ladder_databases.size(); ++index)
				if (RenX::ladder_databases.get(index)->getName().equalsi(db_name))
				{
					db = RenX::ladder_databases.get(index);
					break;
				}
		}
	}

	if (db == nullptr)
		return generate_no_db_page(html_form_response.table);

	return pluginInstance.generate_profile_page(db, steam_id, html_form_response.table);
}

extern "C" __declspec(dllexport) Jupiter::Plugin *getPlugin()
{
	return &pluginInstance;
}
