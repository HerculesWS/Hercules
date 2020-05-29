/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef API_HTTPPARSER_H
#define API_HTTPPARSER_H

#include "common/hercules.h"

#include <stdarg.h>

struct api_session_data;
struct http_parser;
struct http_parser_settings;
struct multipartparser;
struct multipartparser_callbacks;

/**
 * httpparser.c Interface
 **/
struct httpparser_interface {
	/* vars */
	struct http_parser_settings *settings;
	struct multipartparser_callbacks *multi_settings;

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);
	bool (*parse) (int fd);
	void (*init_parser) (int fd, struct api_session_data *sd);
	void (*delete_parser) (int fd);
	void (*init_settings) (void);
	void (*init_multi_settings) (void);

	int (*on_message_begin) (struct http_parser* parser);
	int (*on_headers_complete) (struct http_parser* parser);
	int (*on_message_complete) (struct http_parser* parser);
	int (*on_chunk_header) (struct http_parser* parser);
	int (*on_chunk_complete) (struct http_parser* parser);
	int (*on_url) (struct http_parser* parser, const char* at, size_t length);
	int (*on_status) (struct http_parser* parser, const char* at, size_t length);
	int (*on_header_field) (struct http_parser* parser, const char* at, size_t length);
	int (*on_header_value) (struct http_parser* parser, const char* at, size_t length);
	int (*on_body) (struct http_parser* parser, const char* at, size_t length);

	int (*on_multi_body_begin) (struct multipartparser *parser);
	int (*on_multi_part_begin) (struct multipartparser *parser);
	int (*on_multi_header_field) (struct multipartparser *parser, const char *data, size_t size);
	int (*on_multi_header_value) (struct multipartparser *parser, const char *data, size_t size);
	int (*on_multi_headers_complete) (struct multipartparser *parser);
	int (*on_multi_data) (struct multipartparser *parser, const char *data, size_t size);
	int (*on_multi_part_end) (struct multipartparser *parser);
	int (*on_multi_body_end) (struct multipartparser *parser);
};

#ifdef HERCULES_CORE
void httpparser_defaults(void);
#endif // HERCULES_CORE

HPShared struct httpparser_interface *httpparser;

#endif /* API_HTTPPARSER_H */
