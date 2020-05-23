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
		uint message_begin : 1;
		uint headers_complete : 1;
		uint message_complete : 1;
		uint url : 1;
		uint status : 1;
		uint body : 1;
	} flag;
	char *url;
	struct HttpHandler *handler;
};

#endif /* API_APISESSIONDATA_H */
