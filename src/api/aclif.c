/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
 * Copyright (C) Athena Dev Teams
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
#include "api/aclif.h"

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
#include "api/apisessiondata.h"
#include "api/handlers.h"
#include "api/httpparser.h"
#include "api/httpsender.h"
#include "api/mimepart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static struct aclif_interface aclif_s;
struct aclif_interface *aclif;

static bool aclif_setip(const char *ip)
{
	char ip_str[16];
	nullpo_retr(false, ip);
	aclif->api_ip = sockt->host2ip(ip);
	if (!aclif->api_ip) {
		ShowWarning("Failed to resolve Api server address! (%s)\n", ip);
		return false;
	}

	safestrncpy(aclif->api_ip_str, ip, sizeof(aclif->api_ip_str));
	ShowInfo("Api server IP address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, sockt->ip2str(aclif->api_ip, ip_str));
	return true;
}

static bool aclif_setbindip(const char *ip)
{
	nullpo_retr(false, ip);
	aclif->bind_ip = sockt->host2ip(ip);
	if (aclif->bind_ip) {
		char ip_str[16];
		ShowInfo("Api Server Bind IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, sockt->ip2str(aclif->bind_ip, ip_str));
		return true;
	}
	ShowWarning("Failed to Resolve Api Server Address! (%s)\n", ip);
	return false;
}

/*==========================================
 * Sets api port to 'port'
 * is run from api.c upon loading api server configuration
 *------------------------------------------*/
static void aclif_setport(uint16 port)
{
	aclif->api_port = port;
}

/*==========================================
 * Main client packet processing function
 *------------------------------------------*/
static int aclif_parse(int fd)
{
	ShowInfo("parse called: %d\n", fd);
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_ret(sd);
	if (!httpparser->parse(fd))
	{
		httpparser->show_error(fd, sd);
		sockt->eof(fd);
		sockt->close(fd);
		return 0;
	}
	if (sockt->session[fd] == NULL || sockt->session[fd]->flag.eof != 0) {
		aclif->terminate_connection(fd);
		return 0;
	}
	if (sd->parser.nread > MAX_REQUEST_SIZE) {
		ShowError("http request too big %d: %u\n", fd, sd->parser.nread);
		sockt->eof(fd);
		sockt->close(fd);
		return 0;
	}
	if (sd->flag.message_complete == 1) {
		aclif->parse_request(fd, sd);
		return 0;
	}

	return 0;
}

static int aclif_parse_request(int fd, struct api_session_data *sd)
{
	if (sd->handler == NULL) {
		ShowError("http handler is NULL: %d\n", fd);
		sockt->close(fd);
		return 0;
	}
	if (sd->handler->func == NULL) {
		ShowError("http handler function is NULL: %d\n", fd);
		sockt->close(fd);
		return 0;
	}
	if (!aclif->decode_post_headers(fd, sd)) {
		aclif->terminate_connection(fd);
		return 0;
	}
	if (!sd->handler->func(fd, sd)) {
		aclif->reportError(fd, sd);
		sockt->close(fd);
		return 0;
	}
	if (sockt->session[fd] == NULL || sockt->session[fd]->flag.eof != 0) {
		aclif->terminate_connection(fd);
		return 0;
	}
	if ((sd->handler->flags & REQ_AUTO_CLOSE) != 0)
		sockt->close(fd);
	return 0;
}

static void aclif_terminate_connection(int fd)
{
	ShowInfo("closed: %d\n", fd);
	sockt->close(fd);
}

static int aclif_connected(int fd)
{
	ShowInfo("connected called: %d\n", fd);

	nullpo_ret(sockt->session[fd]);
	struct api_session_data *sd = NULL;
	CREATE(sd, struct api_session_data, 1);
	sd->fd = fd;
	sd->headers_db = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_BOTH, MAX_HEADER_NAME_SIZE);
	sd->post_headers_db = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_DATA, MAX_POST_HEADER_NAME_SIZE);
	sockt->session[fd]->session_data = sd;
	httpparser->init_parser(fd, sd);
	return 0;
}

static int aclif_post_headers_destroy_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct MimePart *part = DB->data2ptr(data);
	if (part && part->data) {
		aFree(part->data);
		part->data = NULL;
	}
	return 0;
}

static int aclif_session_delete(int fd)
{
	nullpo_ret(sockt->session[fd]);
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_ret(sd);
	aFree(sd->url);
	sd->url = NULL;
	aFree(sd->temp_header);
	sd->temp_header = NULL;
	db_destroy(sd->headers_db);
	sd->headers_db = NULL;
	sd->post_headers_db->destroy(sd->post_headers_db, aclif->post_headers_destroy_sub);
	sd->post_headers_db = NULL;
	aFree(sd->body);
	sd->body = NULL;
	aFree(sd->multi_parser);
	sd->multi_parser = NULL;
	aFree(sd->temp_mime_header);
	sd->temp_mime_header = NULL;

	httpparser->delete_parser(fd);
	return 0;
}

static void aclif_load_handlers(void)
{
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		aclif->handlers_db[i] = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_DATA, MAX_URL_SIZE);
	}
#define handler(method, url, func, flags) aclif->add_handler(method, url, handlers->parse_ ## func, flags)
#include "api/urlhandlers.h"
#undef handler
}

static void aclif_add_handler(enum http_method method, const char *url, HttpParseHandler func, int flags)
{
	nullpo_retv(url);
	nullpo_retv(func);
	Assert_retv(method >= 0 && method < HTTP_MAX_PROTOCOL);

	ShowWarning("Add url: %s\n", url);
	struct HttpHandler *handler = aCalloc(1, sizeof(struct HttpHandler));
	handler->method = method;
	handler->func = func;
	handler->flags = flags;

	strdb_put(aclif->handlers_db[method], url, handler);
}

void aclif_set_url(int fd, enum http_method method, const char *url, size_t size)
{
	nullpo_retv(url);
	Assert_retv(method >= 0 && method < HTTP_MAX_PROTOCOL);

	if (size > MAX_URL_SIZE) {
		ShowWarning("Url size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	aFree(sd->url);
	sd->url = aMalloc(size + 1);
	safestrncpy(sd->url, url, size + 1);

	struct HttpHandler *handler = strdb_get(aclif->handlers_db[method], sd->url);
	if (handler == NULL) {
		ShowWarning("Unhandled url %d: %s\n", fd, sd->url);
		sockt->eof(fd);
		return;
	}
	if (handler->func == NULL) {
		ShowError("found NULL handler for url %d: %s\n", fd, url);
		Assert_report(0);
		sockt->eof(fd);
		return;
	}

	sd->flag.url = 1;
	sd->handler = handler;

	ShowWarning("url: %s\n", sd->url);
}

void aclif_set_body(int fd, const char *body, size_t size)
{
	nullpo_retv(body);

	if (size > MAX_BODY_SIZE) {
		ShowWarning("Body size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	aFree(sd->body);
	sd->body = aMalloc(size + 1);
	memcpy(sd->body, body, size);
	sd->body[size] = 0;
	sd->body_size = size;

	if (!httpparser->multi_parse(fd)) {
		ShowWarning("Post headers parsing error %d\n", fd);
		sockt->eof(fd);
		return;
	}
}

void aclif_set_header_name(int fd, const char *name, size_t size)
{
	nullpo_retv(name);

	if (size > MAX_HEADER_NAME_SIZE) {
		ShowWarning("Header name size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	if (sd->headers_count >= MAX_HEADER_COUNT) {
		ShowWarning("Header count too big %d: %d\n", fd, sd->headers_count);
		sockt->eof(fd);
		return;
	}

	aFree(sd->temp_header);
	sd->temp_header = aStrndup(name, size);
}

void aclif_set_header_value(int fd, const char *value, size_t size)
{
	nullpo_retv(value);

	if (size > MAX_HEADER_VALUE_SIZE) {
		ShowWarning("Header value size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);
	strdb_put(sd->headers_db, sd->temp_header, aStrndup(value, size));
	sd->temp_header = NULL;
	sd->headers_count ++;
}

void aclif_set_post_header_name(int fd, const char *name, size_t size)
{
	nullpo_retv(name);

	if (size > MAX_POST_HEADER_NAME_SIZE) {
		ShowWarning("Post header name size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	if (sd->post_headers_count >= MAX_POST_HEADER_COUNT) {
		ShowWarning("Post header count too big %d: %d\n", fd, sd->post_headers_count);
		sockt->eof(fd);
		return;
	}

	char *buf = aStrndup(name, size);
	if (strcmp(buf, "Content-Disposition") == 0) {
		sd->mime_flag = MIME_FLAG_CONTENT_DISPOSITION;
	} else if (strcmp(buf, "Content-Type") == 0) {
		sd->mime_flag = MIME_FLAG_CONTENT_TYPE;
	}
	aFree(buf);
}

void aclif_set_post_header_value(int fd, const char *value, size_t size)
{
	nullpo_retv(value);

	if (size > MAX_POST_HEADER_VALUE_SIZE) {
		ShowWarning("Post header value size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	if (sd->mime_flag == MIME_FLAG_CONTENT_DISPOSITION) {
		// form-data; name="myname"
		char *buf0 = aStrndup(value, size);
		char *buf = buf0;
		const char *format = "form-data; name=\"";
		const size_t sz = strlen(format);
		if (strncmp(buf, format, sz) != 0) {
			ShowError("Unknown multi header value %d\n", fd);
			aFree(buf0);
			sockt->eof(fd);
			return;
		}
		buf += sz;
		char *ptr = strchr(buf, '"');
		if (ptr == NULL || ptr <= buf + 1) {
			ShowError("Corrupted multi header value %d\n", fd);
			ShowError("test '%s' %p, '%s' %p\n", buf, (void*)buf, ptr, (void*)ptr);
			aFree(buf0);
			sockt->eof(fd);
			return;
		}
		safestrncpy(sd->temp_mime_header->name, buf, ptr - buf + 1);
		// on file upload here should be also '; filename="filename.txt"' but we ignoring it
		aFree(buf0);
	} else if (sd->mime_flag == MIME_FLAG_CONTENT_TYPE) {
		char *buf = aStrndup(value, size);
		safestrncpy(sd->temp_mime_header->content_type, buf, sizeof(sd->temp_mime_header->content_type));
		aFree(buf);
	} else {
		ShowError("Unknown multi header value %d\n", fd);
		sockt->eof(fd);
		return;
	}
}

void aclif_set_post_header_data(int fd, const char *value, size_t size)
{
	nullpo_retv(value);

	if (size > MAX_POST_HEADER_DATA_SIZE) {
		ShowWarning("Post header data size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);
	sd->temp_mime_header->data = aMalloc(size + 1);
	memcpy(sd->temp_mime_header->data, value, size);
	sd->temp_mime_header->data[size] = '\x0';
}

void aclif_multi_part_start(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);

	sd->flag.multi_part_begin = 1;
	sd->flag.multi_part_complete = 0;
	if (sd->temp_mime_header)
		aFree(sd->temp_mime_header);
	sd->temp_mime_header = aCalloc(1, sizeof(*sd->temp_mime_header));
	sd->mime_flag = MIME_FLAG_NONE;
}

void aclif_multi_part_complete(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);
	nullpo_retv(sd->temp_mime_header);
	Assert_retv(sd->flag.multi_part_begin == 1);

	strdb_put(sd->post_headers_db, sd->temp_mime_header->name, sd->temp_mime_header);

	sd->temp_mime_header = NULL;

	sd->flag.multi_part_begin = 0;
	sd->flag.multi_part_complete = 1;
	sd->mime_flag = MIME_FLAG_NONE;
}

void aclif_multi_body_complete(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);
	struct MimePart *data;

	struct DBIterator *iter = db_iterator(sd->post_headers_db);
	for (data = dbi_first(iter); dbi_exists(iter); data = dbi_next(iter)) {
		ShowError("found mime headers: %s, %s, '%s'\n", data->name, data->content_type, data->data);
	}

	dbi_destroy(iter);

}

void aclif_check_headers(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);
	Assert_retv(sd->flag.headers_complete == 1);

	const char *size_str = strdb_get(sd->headers_db, "Content-Length");
	if (size_str != NULL) {
		const size_t sz = atoll(size_str);
		if (sz > MAX_BODY_SIZE) {
			ShowError("Body size too big: %d\n", fd);
			sockt->eof(fd);
			return;
		}
	}

	const char *content_type = strdb_get(sd->headers_db, "Content-Type");
	if (content_type == NULL)
		return;
	const char *post_name = "multipart/form-data; boundary=";
	const size_t post_name_sz = strlen(post_name);
	if (strncmp(content_type, post_name, post_name_sz) != 0) {
		return;
	}
	if (sd->parser.method != HTTP_POST) {
		ShowError("Form data detected on non POST request: %d\n", fd);
		sockt->eof(fd);
		return;
	}
	const size_t sz = strlen(content_type + post_name_sz);
	if (sz < MIN_BOUNDARY_SIZE || sz > MAX_BOUNDARY_SIZE) {
		ShowError("Form boudary size is wrong %d: %lu\n", fd, sz);
		sockt->eof(fd);
		return;
	}

//	char *boundary = aMalloc(sz + 3);
//	strcpy(boundary, "--");
//	strcat(boundary, content_type + post_name_sz);
	ShowInfo("boundary: %s\n", content_type + post_name_sz);

//	httpparser->init_multi_parser(fd, sd, boundary);
	httpparser->init_multi_parser(fd, sd, content_type + post_name_sz);
//	aFree(boundary);

}

bool aclif_decode_post_headers(int fd, struct api_session_data *sd)
{
	nullpo_retr(false, sd);

/// ?????

	return true;
}

void aclif_reportError(int fd, struct api_session_data *sd)
{
}

static int do_init_aclif(bool minimal)
{
	if (minimal)
		return 0;

	sockt->set_defaultparse(aclif->parse);
	sockt->set_default_client_connected(aclif->connected);
	sockt->set_default_delete(aclif->session_delete);
	sockt->validate = false;
	if (sockt->make_listen_bind(aclif->bind_ip, aclif->api_port) == -1) {
		ShowFatalError("Failed to bind to port '"CL_WHITE"%d"CL_RESET"'\n", aclif->api_port);
		exit(EXIT_FAILURE);
	}

	aclif->load_handlers();

	return 0;
}

static void do_final_aclif(void)
{
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		db_destroy(aclif->handlers_db[i]);
		aclif->handlers_db[i] = NULL;
	}
}

void aclif_defaults(void)
{
	aclif = &aclif_s;
	/* vars */
	aclif->bind_ip = INADDR_ANY;
	aclif->api_port = 3000;
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		aclif->handlers_db[i] = NULL;
	}
	/* core */
	aclif->init = do_init_aclif;
	aclif->final = do_final_aclif;
	aclif->setip = aclif_setip;
	aclif->setbindip = aclif_setbindip;
	aclif->setport = aclif_setport;
	aclif->parse = aclif_parse;
	aclif->parse_request = aclif_parse_request;
	aclif->terminate_connection = aclif_terminate_connection;
	aclif->connected = aclif_connected;
	aclif->session_delete = aclif_session_delete;
	aclif->load_handlers = aclif_load_handlers;
	aclif->add_handler = aclif_add_handler;
	aclif->set_url = aclif_set_url;
	aclif->set_body = aclif_set_body;
	aclif->set_header_name = aclif_set_header_name;
	aclif->set_header_value = aclif_set_header_value;
	aclif->set_post_header_name = aclif_set_post_header_name;
	aclif->set_post_header_value = aclif_set_post_header_value;
	aclif->set_post_header_data = aclif_set_post_header_data;
	aclif->multi_part_start = aclif_multi_part_start;
	aclif->multi_part_complete = aclif_multi_part_complete;
	aclif->multi_body_complete = aclif_multi_body_complete;
	aclif->post_headers_destroy_sub = aclif_post_headers_destroy_sub;
	aclif->check_headers = aclif_check_headers;
	aclif->decode_post_headers = aclif_decode_post_headers;

	aclif->reportError = aclif_reportError;
}
