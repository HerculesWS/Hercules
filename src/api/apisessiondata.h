/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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

#include "http_include.h"

#include "common/cbasetypes.h"
#include "api/httpparsehandler.h"
#include "api/jsonwriter.h"
#include "api/postheader.h"

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
	HTTP_PARSER parser;
	char *request_temp;
	size_t request_temp_size;
	size_t request_temp_alloc_size;
	struct multipartparser *multi_parser;
	size_t request_size;
	struct api_flag {
		uint32 message_begin : 1;        // message parsing started
		uint32 headers_complete : 1;     // headers parsing complete
		uint32 message_complete : 1;     // message parsing complete
		uint32 url : 1;                  // url parsing complete
		uint32 status : 1;               // status code parsing complete
		uint32 body : 1;                 // body parsing complete
		uint32 multi_part_begin : 1;     // multi part parsing started
		uint32 multi_part_complete : 1;  // multi part parsing complete
		uint32 handled : 1;              // http request already handled
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
	char *data;
	int data_size;
	JsonW *json;
	void *custom;
	char valid_post_headers[CONST_POST_MAX];
};

#endif /* API_APISESSIONDATA_H */
