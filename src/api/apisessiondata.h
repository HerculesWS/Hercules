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
#ifndef API_APISESSIONDATA_H
#define API_APISESSIONDATA_H

#include "common/cbasetypes.h"
#include "api/httpparsehandler.h"

#include <http-parser/http_parser.h>

#include "common/hercules.h"

#include <stdarg.h>

struct HttpHandler;
struct multipartparser;
struct MimePart;

enum e_mime_flag {
	MIME_FLAG_NONE = 0,
	MIME_FLAG_CONTENT_DISPOSITION = 1,
	MIME_FLAG_CONTENT_TYPE = 2
};

struct api_session_data {
	int fd;
	int id;  // for inter server requests
	int account_id;
	int char_id;
	struct http_parser parser;
	struct multipartparser *multi_parser;
	struct api_flag {
		uint message_begin : 1;        // message parsing started
		uint headers_complete : 1;     // headers parsing complete
		uint message_complete : 1;     // message parsing complete
		uint url : 1;                  // url parsing complete
		uint status : 1;               // status code parsing complete
		uint body : 1;                 // body parsing complete
		uint multi_part_begin : 1;     // multi part parsing started
		uint multi_part_complete : 1;  // multi part parsing complete
		uint handled : 1;              // http request already handled
	} flag;
	char *url;
	struct HttpHandler *handler;
	char *temp_header;
	struct MimePart *temp_mime_header;
	enum e_mime_flag mime_flag;
	struct DBMap *headers_db;
	struct DBMap *post_headers_db;
	int headers_count;
	int post_headers_count;
	char *body;
	char *world_name;
	size_t body_size;
};

#endif /* API_APISESSIONDATA_H */
