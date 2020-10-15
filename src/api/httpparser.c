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
#define HERCULES_CORE

#include "config/core.h" // ANTI_MAYAP_CHEAT, RENEWAL, SECURE_NPCTIMEOUT
#include "api/httpparser.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/ers.h"
#include "common/grfio.h"
#include "common/memmgr.h"
#include "common/mmo.h" // NEW_CARTS, char_achievements
#include "common/nullpo.h"
#include "common/packets.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"
#include "api/aclif.h"
#include "api/apisessiondata.h"

#include <multipart-parser/multipartparser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static struct httpparser_interface httpparser_s;
struct httpparser_interface *httpparser;
#define DEBUG_LOG

// parser handlers

#define GET_FD_SD \
	int fd = (int)(intptr_t)parser->data; \
	struct api_session_data *sd = sockt->session[fd]->session_data; \
	nullpo_ret(sd);

#define GET_FD() (int)(intptr_t)parser->data

// http parser handlers

static int handler_on_message_begin(HTTP_PARSER *parser)
{
	nullpo_ret(parser);
	GET_FD_SD;
	if (sockt->session[fd]->flag.eof)
		return 0;

	sd->flag.message_begin = 1;

#ifdef DEBUG_LOG
	ShowInfo("***MESSAGE BEGIN***\n");
#endif
	return 0;
}

static int handler_on_headers_complete(HTTP_PARSER *parser)
{
	nullpo_ret(parser);
	GET_FD_SD;
	if (sockt->session[fd]->flag.eof)
		return 0;

	sd->flag.headers_complete = 1;
	aclif->check_headers(fd, sd);

#ifdef DEBUG_LOG
	ShowInfo("***HEADERS COMPLETE***\n");
#endif
	return 0;
}

static int handler_on_message_complete(HTTP_PARSER *parser)
{
	nullpo_ret(parser);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

	sd->flag.message_complete = 1;
	sd->flag.message_begin = 0;

#ifdef DEBUG_LOG
	ShowInfo("***MESSAGE COMPLETE***\n");
#endif
	return 0;
}

static int handler_on_chunk_header(HTTP_PARSER *parser)
{
	nullpo_ret(parser);
#ifdef DEBUG_LOG
	ShowInfo("handler_on_chunk_header\n");
#endif
	return 0;
}

static int handler_on_chunk_complete(HTTP_PARSER *parser)
{
	nullpo_ret(parser);
#ifdef DEBUG_LOG
	ShowInfo("handler_on_chunk_complete\n");
#endif
	return 0;
}

static int handler_on_url(HTTP_PARSER *parser, const char *at, size_t length)
{
	nullpo_ret(parser);
	nullpo_ret(at);
	int fd = GET_FD();

	if (sockt->session[fd]->flag.eof)
		return 0;

	aclif->set_url(fd, parser->method, at, length);

#ifdef DEBUG_LOG
	ShowInfo("Url: %d: %.*s\n", parser->method, (int)length, at);
#endif
	return 0;
}

static int handler_on_status(HTTP_PARSER *parser, const char *at, size_t length)
{
	nullpo_ret(parser);
	nullpo_ret(at);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

	sd->flag.status = 1;

#ifdef DEBUG_LOG
	ShowInfo("Status: %.*s\n", (int)length, at);
#endif
	return 0;
}

static int handler_on_header_field(HTTP_PARSER *parser, const char *at, size_t length)
{
	nullpo_ret(parser);
	nullpo_ret(at);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("Header field: %.*s\n", (int)length, at);
#endif

	aclif->set_header_name(fd, at, length);
	return 0;
}

static int handler_on_header_value(HTTP_PARSER *parser, const char *at, size_t length)
{
	nullpo_ret(parser);
	nullpo_ret(at);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("Header value: %.*s\n", (int)length, at);
#endif

	aclif->set_header_value(fd, at, length);
	return 0;
}

static int handler_on_body(HTTP_PARSER *parser, const char *at, size_t length)
{
	nullpo_ret(parser);
	nullpo_ret(at);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

	sd->flag.body = 1;

#ifdef DEBUG_LOG
	ShowInfo("Body: %.*s\n", (int)length, at);
	ShowInfo("end body\n");
#endif

	aclif->set_body(fd, at, length);

	return 0;
}

// multi parser handlers

static int handler_on_multi_body_begin(struct multipartparser *parser)
{
	nullpo_ret(parser);
#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_body_begin\n");
#endif
	return 0;
}

static int handler_on_multi_part_begin(struct multipartparser *parser)
{
	nullpo_ret(parser);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_part_begin\n");
#endif

	aclif->multi_part_start(fd, sd);
	return 0;
}

static int handler_on_multi_header_field(struct multipartparser *parser, const char *data, size_t size)
{
	nullpo_ret(parser);
	nullpo_ret(data);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_header_field: %lu: %.*s\n", size, (int)size, data);
#endif

	aclif->set_post_header_name(fd, data, size);
	return 0;
}

static int handler_on_multi_header_value(struct multipartparser *parser, const char *data, size_t size)
{
	nullpo_ret(parser);
	nullpo_ret(data);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_header_value: %lu: %.*s\n", size, (int)size, data);
#endif

	aclif->set_post_header_value(fd, data, size);
	return 0;
}

static int handler_on_multi_headers_complete(struct multipartparser *parser)
{
#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_headers_complete\n");
#endif
	return 0;
}

static int handler_on_multi_data(struct multipartparser *parser, const char *data, size_t size)
{
	nullpo_ret(parser);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_data: %.*s\n", (int)size, data);
#endif

	aclif->set_post_header_data(fd, data, size);
	return 0;
}

static int handler_on_multi_part_end(struct multipartparser *parser)
{
	nullpo_ret(parser);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_part_end\n");
#endif

	aclif->multi_part_complete(fd, sd);

	return 0;
}

static int handler_on_multi_body_end(struct multipartparser *parser)
{
	nullpo_ret(parser);
	GET_FD_SD;

	if (sockt->session[fd]->flag.eof)
		return 0;

#ifdef DEBUG_LOG
	ShowInfo("handler_on_multi_body_end\n");
#endif

	aclif->multi_body_complete(fd, sd);

	return 0;
}

static bool httpparser_parse_real(int fd, struct api_session_data *sd, const char *data, int data_size)
{
	nullpo_ret(sd);
	if (data_size == 0)
		return true;

#ifdef USE_HTTP_PARSER
	size_t parsed_size = http_parser_execute(&sd->parser, httpparser->settings, data, data_size);
	if (data_size == parsed_size) {
		sd->request_size += parsed_size;
		return true;
	}
#else  // USE_HTTP_PARSER
	enum llhttp_errno err = llhttp_execute(&sd->parser, data, data_size);
	if (err == HPE_OK) {
		sd->request_size += data_size;
		return true;
	}
#endif  // USE_HTTP_PARSER

	return false;
}

static void httpparser_add_to_temp_request(int fd, struct api_session_data *sd, const char *data, int data_size)
{
	nullpo_retv(sd);
	if (sd->request_temp == NULL) {
		sd->request_temp = aCalloc(1, data_size);
		sd->request_temp_size = data_size;
		sd->request_temp_alloc_size = data_size;
		memcpy(sd->request_temp, data, data_size);
	} else {
		const size_t old_size = sd->request_temp_size;
		sd->request_temp_size += data_size;
		if (sd->request_temp_alloc_size < sd->request_temp_size) {
			sd->request_temp = aRealloc(sd->request_temp, sd->request_temp_size);
			sd->request_temp_alloc_size = sd->request_temp_size;
		}
		memcpy(sd->request_temp + old_size, data, data_size);
	}
}

static int httpparser_search_request_line_end(struct api_session_data *sd)
{
	nullpo_retr(-1, sd);

	int idx = -1;
	const char *data = sd->request_temp;
	const int data_size = sd->request_temp_size;
	for (int i = 0; i < data_size - 1; i ++) {
		if (data[i] == '\r' && data[i + 1] == '\n') {
			idx = i;
			i++;
		}
	}
	return idx;
}

static bool httpparser_parse(int fd)
{
	nullpo_ret(sockt->session[fd]);

	struct api_session_data *sd = sockt->session[fd]->session_data;
	size_t data_size = RFIFOREST(fd);
	if (data_size == 0)
		return true;

	const char *data = RFIFOP(fd, 0);

	if (sd->flag.headers_complete == 0) {
		// because parser cant handle part of header, need cache incomplete headers
		httpparser->add_to_temp_request(fd, sd, data, data_size);
		RFIFOSKIP(fd, data_size);
		if (sd->request_temp_size > MAX_TEMP_HEADER_SIZE) {
			return false;
		}
		int idx = httpparser->search_request_line_end(sd);
		if (idx >= 0) {
			// in headers found separator and need parse them
			Assert_retr(false, sd->request_temp_size >= idx + 2);
			const bool res = httpparser->parse_real(fd, sd, sd->request_temp, idx + 2);
			if (!res)
				return false;
			sd->request_temp_size -= idx + 2;
			memmove(sd->request_temp, sd->request_temp + idx + 2, sd->request_temp_size);
			return true;
		} else {
			// data cached, but no yet separator
			return true;
		}
		if (sd->flag.headers_complete != 0) {
			// header was complete in previous parse. try to parse additional data
			const bool res = httpparser->parse_real(fd, sd, sd->request_temp, sd->request_temp_size);
			aFree(sd->request_temp);
			sd->request_temp = NULL;
			sd->request_temp_size = 0;
			sd->request_temp_alloc_size = 0;
			return res;
		}
	} else {
		// parse received non headers data
		const bool res = httpparser->parse_real(fd, sd, data, data_size);
		RFIFOSKIP(fd, data_size);
		return res;
	}
}

static void httpparser_show_error(int fd, struct api_session_data *sd)
{
#ifdef USE_HTTP_PARSER
	ShowError("http parser error %d: %d, %s, %s\n", fd, sd->parser.http_errno, http_errno_name(sd->parser.http_errno), http_errno_description(sd->parser.http_errno));
#else  // USE_HTTP_PARSER
	ShowError("http parser error %d: %d, %s, %s\n", fd, sd->parser.error, http_errno_name(sd->parser.error), sd->parser.reason);
#endif  // USE_HTTP_PARSER
}

static bool httpparser_multi_parse(int fd)
{
	nullpo_ret(sockt->session[fd]);

	struct api_session_data *sd = sockt->session[fd]->session_data;

	if (sd->multi_parser == NULL)
		return true;
	size_t parsed_size = multipartparser_execute(sd->multi_parser, httpparser->multi_settings, sd->body, sd->body_size);
	if (parsed_size != sd->body_size) {
		return false;
	}
	return true;
}

static void httpparser_init_parser(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);
#ifdef USE_HTTP_PARSER
	http_parser_init(&sd->parser, HTTP_REQUEST);
#else  // USE_HTTP_PARSER
	llhttp_init(&sd->parser, HTTP_REQUEST, httpparser->settings);
#endif  // USE_HTTP_PARSER
	sd->parser.data = (void*)(intptr_t)fd;
}

static void httpparser_init_multi_parser(int fd, struct api_session_data *sd, const char *boundary)
{
	nullpo_retv(sd);
	nullpo_retv(boundary);
	sd->multi_parser = aMalloc(sizeof(multipartparser));
	multipartparser_init(sd->multi_parser, boundary);
	sd->multi_parser->data = (void*)(intptr_t)fd;
}

static void httpparser_delete_parser(int fd)
{
}

static void httpparser_init_settings(void)
{
	httpparser->settings = aCalloc(1, sizeof(struct http_parser_settings));
#ifndef USE_HTTP_PARSER
	llhttp_settings_init(httpparser->settings);
#endif  // USE_HTTP_PARSER
	httpparser->settings->on_message_begin = httpparser->on_message_begin;
	httpparser->settings->on_url = httpparser->on_url;
	httpparser->settings->on_header_field = httpparser->on_header_field;
	httpparser->settings->on_header_value = httpparser->on_header_value;
	httpparser->settings->on_headers_complete = httpparser->on_headers_complete;
	httpparser->settings->on_body = httpparser->on_body;
	httpparser->settings->on_message_complete = httpparser->on_message_complete;
	httpparser->settings->on_status = httpparser->on_status;
	httpparser->settings->on_chunk_header = httpparser->on_chunk_header;
	httpparser->settings->on_chunk_complete = httpparser->on_chunk_complete;
}

static void httpparser_init_multi_settings(void)
{
	httpparser->multi_settings = aCalloc(1, sizeof(struct multipartparser_callbacks));
	multipartparser_callbacks_init(httpparser->multi_settings);
	httpparser->multi_settings->on_body_begin = httpparser->on_multi_body_begin;
	httpparser->multi_settings->on_part_begin = httpparser->on_multi_part_begin;
	httpparser->multi_settings->on_header_field = httpparser->on_multi_header_field;
	httpparser->multi_settings->on_header_value = httpparser->on_multi_header_value;
	httpparser->multi_settings->on_headers_complete = httpparser->on_multi_headers_complete;
	httpparser->multi_settings->on_data = httpparser->on_multi_data;
	httpparser->multi_settings->on_part_end = httpparser->on_multi_part_end;
	httpparser->multi_settings->on_body_end = httpparser->on_multi_body_end;
}

static int do_init_httpparser(bool minimal)
{
	if (minimal)
		return 0;

	httpparser->init_settings();
	httpparser->init_multi_settings();
	return 0;
}

static void do_final_httpparser(void)
{
	aFree(httpparser->settings);
}

void httpparser_defaults(void)
{
	httpparser = &httpparser_s;
	/* vars */
	httpparser->settings = NULL;
	/* core */
	httpparser->init = do_init_httpparser;
	httpparser->final = do_final_httpparser;
	httpparser->parse = httpparser_parse;
	httpparser->parse_real = httpparser_parse_real;
	httpparser->add_to_temp_request = httpparser_add_to_temp_request;
	httpparser->search_request_line_end = httpparser_search_request_line_end;
	httpparser->show_error = httpparser_show_error;
	httpparser->multi_parse = httpparser_multi_parse;
	httpparser->init_parser = httpparser_init_parser;
	httpparser->init_multi_parser = httpparser_init_multi_parser;
	httpparser->delete_parser = httpparser_delete_parser;
	httpparser->init_settings = httpparser_init_settings;
	httpparser->init_multi_settings = httpparser_init_multi_settings;

	httpparser->on_message_begin = handler_on_message_begin;
	httpparser->on_url = handler_on_url;
	httpparser->on_header_field = handler_on_header_field;
	httpparser->on_header_value = handler_on_header_value;
	httpparser->on_headers_complete = handler_on_headers_complete;
	httpparser->on_body = handler_on_body;
	httpparser->on_message_complete = handler_on_message_complete;
	httpparser->on_status = handler_on_status;
	httpparser->on_chunk_header = handler_on_chunk_header;
	httpparser->on_chunk_complete = handler_on_chunk_complete;

	httpparser->on_multi_body_begin = handler_on_multi_body_begin;
	httpparser->on_multi_part_begin = handler_on_multi_part_begin;
	httpparser->on_multi_header_field = handler_on_multi_header_field;
	httpparser->on_multi_header_value = handler_on_multi_header_value;
	httpparser->on_multi_headers_complete = handler_on_multi_headers_complete;
	httpparser->on_multi_data = handler_on_multi_data;
	httpparser->on_multi_part_end = handler_on_multi_part_end;
	httpparser->on_multi_body_end = handler_on_multi_body_end;
}
