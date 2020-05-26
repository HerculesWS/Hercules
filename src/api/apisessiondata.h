/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
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

#include "common/hercules.h"
#include "api/httpparsehandler.h"

#include <http-parser/http_parser.h>

#include <stdarg.h>

struct HttpHandler;

struct api_session_data {
	int fd;
	struct http_parser parser;
	struct api_flag {
		uint message_begin : 1;     // message parsing started
		uint headers_complete : 1;  // headers parsing complete
		uint message_complete : 1;  // message parsing complete
		uint url : 1;               // url parsing complete
		uint status : 1;            // status code parsing complete
		uint body : 1;              // body parsing complete
	} flag;
	char *url;
	struct HttpHandler *handler;
	char *temp_header;
	struct DBMap *headers_db;
	int headers_count;
	char *body;
	size_t body_size;
	bool auto_close;
};

#endif /* API_APISESSIONDATA_H */
