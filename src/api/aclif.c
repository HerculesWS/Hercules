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
#include "common/api.h"
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
#include "api/aloginif.h"
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

//#define DEBUG_LOG

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
#ifdef DEBUG_LOG
	ShowInfo("parse called: %d\n", fd);
#endif
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_ret(sd);
	if (sd->flag.handled == 1) {
		if (sockt->session[fd] == NULL || sockt->session[fd]->flag.eof != 0) {
			aclif->terminate_connection(fd);
		}
		return 0;
	}
	if (!httpparser->parse(fd))
	{
		httpparser->show_error(fd, sd);
		sockt->eof(fd);
		aclif->terminate_connection(fd);
		return 0;
	}
	if (sockt->session[fd] == NULL || sockt->session[fd]->flag.eof != 0) {
		aclif->terminate_connection(fd);
		return 0;
	}
	if (sd->request_size > MAX_REQUEST_SIZE) {
		ShowError("http request too big %d: %lu\n", fd, sd->request_size);
		sockt->eof(fd);
		aclif->terminate_connection(fd);
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
		aclif->terminate_connection(fd);
		return 0;
	}
	if (sd->handler->func == NULL) {
		ShowError("http handler function is NULL: %d\n", fd);
		aclif->terminate_connection(fd);
		return 0;
	}
	if (!aclif->decode_post_headers(fd, sd)) {
		aclif->terminate_connection(fd);
		return 0;
	}
	if (!sd->handler->func(fd, sd)) {
		aclif->reportError(fd, sd);
		aclif->terminate_connection(fd);
		return 0;
	}
	if (sockt->session[fd] == NULL || sockt->session[fd]->flag.eof != 0) {
		aclif->terminate_connection(fd);
		return 0;
	}
	sd->flag.handled = 1;
	if ((sd->handler->flags & REQ_AUTO_CLOSE) != 0)
		aclif->terminate_connection(fd);
	return 0;
}

static void aclif_terminate_connection(int fd)
{
	sockt->close(fd);
}

static int aclif_connected(int fd)
{
	ShowInfo("connected: %d\n", fd);

	nullpo_ret(sockt->session[fd]);
	struct api_session_data *sd = NULL;
	CREATE(sd, struct api_session_data, 1);
	sd->fd = fd;
	sd->headers_db = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_BOTH, MAX_HEADER_NAME_SIZE);
	sd->post_headers_db = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_DATA, MAX_POST_HEADER_NAME_SIZE);
	sd->id = aclif->id_counter++;
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
		part->data_size = 0;
	}
	return 0;
}

static int aclif_session_delete(int fd)
{
	nullpo_ret(sockt->session[fd]);

	struct api_session_data *sd = sockt->session[fd]->session_data;
	if (sd == NULL)
		ShowInfo("disconnected %d\n", fd);
	nullpo_ret(sd);

	ShowInfo("disconnected %d: %s\n", fd, sd->url);

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
	sd->data = NULL;
	aFree(sd->data);
	sd->data_size = 0;
	aFree(sd->request_temp);
	sd->request_temp = NULL;

	httpparser->delete_parser(fd);
	return 0;
}

static void aclif_init_handlers(void)
{
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		aclif->handlers_db[i] = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_DATA, MAX_URL_SIZE);
	}
}

static void aclif_register_handlers(void)
{
#define handler(method, url, func, flags) aclif->add_handler(method, url, handlers->parse_ ## func, NULL, 0, flags)
#define handler2(method, url, func, flags) aclif->add_handler(method, url, handlers->parse_ ## func, handlers->func, API_MSG_ ## func, flags)
#define packet_handler(func) aclif->add_packet_handler(handlers->func, API_MSG_ ## func)
#include "api/urlhandlers.h"
#undef handler
#undef handler2
#undef packet_handler
}

static void aclif_add_handler(http_method method, const char *url, HttpParseHandler func, Handler_func func2, int msg_id, int flags)
{
	nullpo_retv(url);
	nullpo_retv(func);
	Assert_retv(method >= 0 && method < HTTP_MAX_PROTOCOL);

#ifdef DEBUG_LOG
	ShowWarning("Add url: %s\n", url);
#endif
	struct HttpHandler *handler = aCalloc(1, sizeof(struct HttpHandler));
	handler->method = method;
	handler->func = func;
	handler->flags = flags;

	strdb_put(aclif->handlers_db[method], url, handler);

	if (func2 != NULL) {
		Assert_retv(msg_id >= 0 && msg_id < API_MSG_MAX);
		aloginif->msg_map[msg_id] = func2;
	}
}

static void aclif_add_packet_handler(Handler_func func2, int msg_id)
{
	nullpo_retv(func2);
	Assert_retv(msg_id >= 0 && msg_id < API_MSG_MAX);
	aloginif->msg_map[msg_id] = func2;
}

static void aclif_set_url(int fd, http_method method, const char *url, size_t size)
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

#ifdef DEBUG_LOG
	ShowWarning("url: %s\n", sd->url);
#endif
}

static void aclif_set_body(int fd, const char *body, size_t size)
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

static void aclif_set_header_name(int fd, const char *name, size_t size)
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

static bool aclif_check_header(int fd, struct api_session_data *sd, const char *name, const char *value, size_t value_size)
{
	if (value_size > MAX_HEADER_VALUE_SIZE) {
		ShowWarning("Header value size too big %d: %lu\n", fd, value_size);
		return false;
	}
	if (strcmp(name, "Content-Type") == 0 && httpparser->get_method(sd) == HTTP_POST) {
		const char *post_name = "multipart/form-data; boundary=";
		const size_t post_name_sz = strlen(post_name);
		if (strncmp(value, post_name, post_name_sz) != 0) {
			ShowWarning("Wrong content type for post request %d\n", fd);
			return false;
		}
	}
	return true;
}

static void aclif_set_header_value(int fd, const char *value, size_t size)
{
	nullpo_retv(value);

	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);
	if (!aclif->check_header(fd, sd, sd->temp_header, value, size)) {
		sockt->eof(fd);
		return;
	}
	strdb_put(sd->headers_db, sd->temp_header, aStrndup(value, size));
	sd->temp_header = NULL;
	sd->headers_count ++;
}

static void aclif_set_post_header_name(int fd, const char *name, size_t size)
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

static void aclif_set_post_header_value(int fd, const char *value, size_t size)
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
#ifdef DEBUG_LOG
			ShowError("test '%s' %p, '%s' %p\n", buf, (void*)buf, ptr, (void*)ptr);
#endif
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

static void aclif_set_post_header_data(int fd, const char *value, size_t size)
{
	nullpo_retv(value);

	if (size > MAX_POST_HEADER_DATA_SIZE) {
		ShowWarning("Post header data size too big %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	if (sd->flag.multi_part_begin == 1) {
		// initial set header data
		if (sd->temp_mime_header->data != NULL)
			aFree(sd->temp_mime_header->data);
		sd->temp_mime_header->data = aMalloc(size + 1);
		memcpy(sd->temp_mime_header->data, value, size);
		sd->temp_mime_header->data_size = size;
		sd->flag.multi_part_begin = 0;
	} else if (sd->temp_mime_header->data == NULL) {
		ShowWarning("Post header data parsing error %d: %lu\n", fd, size);
		sockt->eof(fd);
		return;
	} else {
		// append header data
		const uint32 newSize = sd->temp_mime_header->data_size + size;
		sd->temp_mime_header->data = aRealloc(sd->temp_mime_header->data, newSize + 1);
		memcpy(sd->temp_mime_header->data + sd->temp_mime_header->data_size, value, size);
		sd->temp_mime_header->data_size = newSize;
	}
	sd->temp_mime_header->data[sd->temp_mime_header->data_size] = '\x0';
#ifdef DEBUG_LOG
	printf(" hex: ");
	for (int f = 0; f < sd->temp_mime_header->data_size; f ++) {
		printf("%02x ", (uint32)(unsigned char)sd->temp_mime_header->data[f]);
	}
	printf("\n");
#endif
}

static void aclif_multi_part_start(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);

	sd->flag.multi_part_begin = 1;
	sd->flag.multi_part_complete = 0;
	if (sd->temp_mime_header)
		aFree(sd->temp_mime_header);
	sd->temp_mime_header = aCalloc(1, sizeof(*sd->temp_mime_header));
	sd->mime_flag = MIME_FLAG_NONE;
}

static void aclif_multi_part_complete(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);
	nullpo_retv(sd->temp_mime_header);

	strdb_put(sd->post_headers_db, sd->temp_mime_header->name, sd->temp_mime_header);

	sd->temp_mime_header = NULL;

	sd->flag.multi_part_begin = 0;
	sd->flag.multi_part_complete = 1;
	sd->mime_flag = MIME_FLAG_NONE;
}

static void aclif_multi_body_complete(int fd, struct api_session_data *sd)
{
	nullpo_retv(sd);

	struct DBIterator *iter = db_iterator(sd->post_headers_db);
#ifdef DEBUG_LOG
	for (struct MimePart *data = dbi_first(iter); dbi_exists(iter); data = dbi_next(iter)) {
		ShowError("found mime headers: %s, %s, '%s'\n", data->name, data->content_type, data->data);
	}
#endif

	dbi_destroy(iter);

}

static void aclif_check_headers(int fd, struct api_session_data *sd)
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
	if (httpparser->get_method(sd) != HTTP_POST) {
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

#ifdef DEBUG_LOG
	ShowInfo("boundary: %s\n", content_type + post_name_sz);
#endif

	httpparser->init_multi_parser(fd, sd, content_type + post_name_sz);

}

static bool aclif_decode_post_headers(int fd, struct api_session_data *sd)
{
	nullpo_retr(false, sd);

	struct online_api_login_data *login_data = NULL;
	struct char_server_data *char_server_data;
	int handled_count = 0;

	if ((sd->handler->flags & REQ_ACCOUNT_ID) != 0) {
		// check is account_id logged in
		int account_id = 0;
		if (!aclif->get_post_header_data_int(sd, "AID", &account_id)) {
			ShowError("Http request without account id %d\n", fd);
			return false;
		}

		login_data = idb_get(aclif->online_db, account_id);
		if (login_data == NULL) {
			ShowError("Account not logged in %d: %d\n", fd, account_id);
			return false;
		}
		sd->account_id = account_id;
		handled_count ++;

		if (login_data->char_id != 0)
			sd->char_id = login_data->char_id;

		if ((sd->handler->flags & REQ_CHAR_LOGGED_IN) != 0) {
			if (login_data->char_id == 0) {
				ShowError("Char not logged in %d\n", fd);
				return false;
			}
		}
	}

	if ((sd->handler->flags & REQ_CHAR_ID) != 0) {
		// check is char_id logged in
		int char_id = 0;
		if (!aclif->get_post_header_data_int(sd, "GID", &char_id)) {
			ShowError("Http request without char id %d\n", fd);
			return false;
		}

		if (login_data == NULL || login_data->char_id != char_id) {
			ShowError("Char not logged in %d: %d\n", fd, char_id);
			return false;
		}
		sd->char_id = char_id;
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_WORLD_NAME) != 0) {
		// check is world name present and correct
		char *name = NULL;
		if (!aclif->get_post_header_data_str(sd, "WorldName", &name, NULL)) {
			ShowError("Http request without WorldName %d\n", fd);
			return false;
		}

		char_server_data = strdb_get(aclif->char_servers_db, name);
		if (char_server_data == NULL) {
			ShowError("Unknown world name %d: %s\n", fd, name);
			return false;
		}

		sd->world_name = name;
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_AUTH_TOKEN) != 0) {
		// check is auth token present and correct
		if (login_data == NULL) {
			ShowError("Account id required for auth token check: %d\n", fd);
			return false;
		}
		char *token = NULL;
		if (!aclif->get_post_header_data_str(sd, "AuthToken", &token, NULL)) {
			ShowError("Http request without AuthToken %d\n", fd);
			return false;
		}

		if (memcmp(login_data->auth_token, token, AUTH_TOKEN_SIZE) != 0) {
			ShowError("Wrong auth token %d: '%s'\n", fd, token);
			return false;
		}
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_GUILD_ID) != 0) {
		// check is guild id present in request
		int guild_id = 0;
		if (!aclif->get_post_header_data_int(sd, "GDID", &guild_id)) {
			ShowError("Http request without guild id %d\n", fd);
			return false;
		}
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_IMG_TYPE) != 0) {
		// check is ImgType present in request
		char *name = NULL;
		if (!aclif->get_post_header_data_str(sd, "ImgType", &name, NULL)) {
			ShowError("Http request without ImgType %d\n", fd);
			return false;
		}
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_IMG) != 0) {
		// check is Img present in request
		char *content_type = NULL;
		if (!aclif->get_post_header_content_type(sd, "Img", &content_type)) {
			ShowError("Http request without Img or empty content_type %d\n", fd);
			return false;
		}
		if (strcmp(content_type, "application/octet-stream") != 0) {
			ShowError("Http request without Img with wrong content_type %d\n", fd);
			return false;
		}
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_VERSION) != 0) {
		// check is Version present in request
		int version = 0;
		if (!aclif->get_post_header_data_int(sd, "Version", &version) || version == 0) {
			ShowError("Http request without Version %d\n", fd);
			return false;
		}
		handled_count ++;
	}

	if ((sd->handler->flags & REQ_DATA) != 0) {
		// check is data present in request
		if (!aclif->is_post_header_present(sd, "data")) {
			ShowError("Http request without data %d\n", fd);
			return false;
		}
		handled_count ++;
	}

	if (handled_count != aclif->get_post_headers_count(sd)) {
		ShowError("Handled wrong number of post headers %d\n", fd);
	}
	return true;
}

static void aclif_reportError(int fd, struct api_session_data *sd)
{
}

static int aclif_print_header(union DBKey key, struct DBData *data, va_list ap)
{
    ShowInfo(" http header: %s = %s\n", key.str, (char*)DB->data2ptr(data));
    return 0;
}

static void aclif_show_request(int fd, struct api_session_data *sd, bool show_http_headers)
{
	nullpo_retv(sd);

	ShowInfo("URL: %s %s\n", httpparser->get_method_str(sd), sd->url);

	if (show_http_headers)
		sd->headers_db->foreach(sd->headers_db, aclif->print_header);

	struct DBIterator *iter = db_iterator(sd->post_headers_db);
	for (struct MimePart *data = dbi_first(iter); dbi_exists(iter); data = dbi_next(iter)) {
		if (data->content_type == NULL || *data->content_type == '\x0')
			ShowInfo(" mime header: %s, '%s'\n", data->name, data->data);
		else
			ShowInfo(" mime header: %s, %s, '%s'\n", data->name, data->content_type, data->data);
#ifdef DEBUG_LOG
		printf(" Hex: ");
		for (int f = 0; f < data->data_size; f ++) {
			printf("%02x ", (uint32)(unsigned char)data->data[f]);
		}
		printf("\n");
#endif
	}
	dbi_destroy(iter);
}

static void aclif_delete_online_player(int account_id)
{
	ShowInfo("test disconnect account: %d\n", account_id);
	struct online_api_login_data *data = idb_get(aclif->online_db, account_id);
	if (data != NULL) {
		aclif->add_remove_timer(data);
	}
}

static void aclif_real_delete_online_player(int account_id)
{
	ShowInfo("test real disconnect account: %d\n", account_id);
	idb_remove(aclif->online_db, account_id);
}

static void aclif_add_online_player(int account_id, const unsigned char *auth_token)
{
	ShowInfo("test connect account: %d\n", account_id);
	ShowInfo("test token: %.*s\n", 16, auth_token);

	struct online_api_login_data *user = idb_ensure(aclif->online_db, account_id, aclif->create_online_login_data);
	if (user->remove_tick != 0)
		aclif->remove_remove_timer(user);
	memcpy(user->auth_token, auth_token, AUTH_TOKEN_SIZE);
}

static void aclif_add_online_char(int account_id, int char_id)
{
	struct online_api_login_data *user = idb_get(aclif->online_db, account_id);
	if (user == NULL) {
		ShowError("Cant set char online. Account not logged in: %d\n", account_id);
		return;
	}
	if (char_id == 0) {
		// reconnect to char server after leave map server
		if (user->char_id == 0) {
			ShowError("Cant set char online. Char was not logged in: %d\n", account_id);
			return;
		}
	} else {
		// probably first login
		user->char_id = char_id;
	}
	if (user->remove_tick != 0)
		aclif->remove_remove_timer(user);
	ShowInfo("test connect char: %d, %d (%d)\n", account_id, char_id, user->char_id);
}

static struct DBData aclif_create_online_login_data(union DBKey key, va_list args)
{
	struct online_api_login_data *user;
	CREATE(user, struct online_api_login_data, 1);
	return DB->ptr2data(user);
}

static int aclif_purge_disconnected_users(int tid, int64 tick, int id, intptr_t data)
{
	aclif->online_db->foreach(aclif->online_db, aclif->purge_disconnected_user, timer->gettick());
	return 0;
}

static int aclif_purge_disconnected_user(union DBKey key, struct DBData *data, va_list ap)
{
	const int64 tick = va_arg(ap, int64);
	struct online_api_login_data *user = (struct online_api_login_data *)DB->data2ptr(data);
	if (user == NULL || user->remove_tick == 0)
		return 0;
	if (user->remove_tick < tick)
		aclif->real_delete_online_player(key.i);

	return 0;
}

static bool aclif_is_post_header_present(struct api_session_data *sd, const char *name)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, name);

	struct MimePart *header = strdb_get(sd->post_headers_db, name);
	if (header == NULL)
		return false;
	return header->data != NULL && header->data_size != 0;
}

static bool aclif_get_post_header_data_int(struct api_session_data *sd, const char *name, int *account_id)
{
	nullpo_retr(false, account_id);
	*account_id = 0;
	nullpo_retr(false, sd);
	nullpo_retr(false, name);

	struct MimePart *header = strdb_get(sd->post_headers_db, name);
	if (header == NULL)
		return false;
	char *data = header->data;
	if (data == NULL)
		return false;
	*account_id = atoi(data);
	return true;
}

static bool aclif_get_post_header_data_str(struct api_session_data *sd, const char *name, char **data, uint32 *data_size)
{
	nullpo_retr(false, data);
	*data = NULL;
	nullpo_retr(false, sd);
	nullpo_retr(false, name);

	struct MimePart *header = strdb_get(sd->post_headers_db, name);
	if (header == NULL)
		return false;
	*data = header->data;
	if (data_size != NULL)
		*data_size = header->data_size;
	return *data != NULL;
}

static bool aclif_get_post_header_data_json(struct api_session_data *sd, const char *name, JsonP **json)
{
	nullpo_retr(false, json);
	*json = NULL;
	nullpo_retr(false, sd);
	nullpo_retr(false, name);

	struct MimePart *header = strdb_get(sd->post_headers_db, name);
	if (header == NULL)
		return false;
	*json = jsonparser->parse(header->data);
	return *json != NULL;
}

static bool aclif_get_post_header_content_type(struct api_session_data *sd, const char *name, char **content_type)
{
	nullpo_retr(false, content_type);
	*content_type = NULL;
	nullpo_retr(false, sd);
	nullpo_retr(false, name);

	struct MimePart *header = strdb_get(sd->post_headers_db, name);
	if (header == NULL)
		return false;
	*content_type = header->content_type;
	return *content_type != NULL;
}

static int aclif_get_post_headers_count(struct api_session_data *sd)
{
	return db_size(sd->post_headers_db);
}

static void aclif_add_char_server(int char_server_id, const char *name)
{
	struct char_server_data *data = aCalloc(1, sizeof(struct char_server_data));
	data->id = char_server_id;
	char *name2 = aStrdup(name);
	strdb_put(aclif->char_servers_db, name2, data);
	idb_put(aclif->char_servers_id_db, char_server_id, name2);
}

static void aclif_remove_char_server(int char_server_id, const char *name)
{
	idb_remove(aclif->char_servers_id_db, char_server_id);
	strdb_remove(aclif->char_servers_db, name);
}

static int aclif_get_char_server_id(struct api_session_data *sd)
{
	nullpo_retr(-1, sd);
	struct char_server_data *data = strdb_get(aclif->char_servers_db, sd->world_name);
	nullpo_retr(-1, data);
	return data->id;
}

static void aclif_add_remove_timer(struct online_api_login_data *user)
{
	nullpo_retv(user);
	user->remove_tick = timer->gettick() + aclif->remove_disconnected_delay;
}

static void aclif_remove_remove_timer(struct online_api_login_data *user)
{
	nullpo_retv(user);
	user->remove_tick = 0;
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

	aclif->init_handlers();
	aclif->register_handlers();

	timer->add_func_list(aclif->purge_disconnected_users, "aclif->purge_disconnected_users");
	timer->add_interval(timer->gettick() + aclif->remove_disconnected_delay, aclif->purge_disconnected_users, 0, 0, aclif->remove_disconnected_delay);

	return 0;
}

static void do_final_aclif(void)
{
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		db_destroy(aclif->handlers_db[i]);
		aclif->handlers_db[i] = NULL;
	}
	db_destroy(aclif->online_db);
	db_destroy(aclif->char_servers_db);
	db_destroy(aclif->char_servers_id_db);
}

void aclif_defaults(void)
{
	aclif = &aclif_s;
	/* vars */
	aclif->bind_ip = INADDR_ANY;
	aclif->api_port = 3000;

	aclif->remove_disconnected_delay = 5000;
	aclif->id_counter = 0;

	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		aclif->handlers_db[i] = NULL;
	}
	aclif->online_db = idb_alloc(DB_OPT_RELEASE_DATA);
	aclif->char_servers_db = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_BOTH, MAX_CHARSERVER_NAME_SIZE);
	aclif->char_servers_id_db = idb_alloc(DB_OPT_BASE);

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
	aclif->init_handlers = aclif_init_handlers;
	aclif->register_handlers = aclif_register_handlers;
	aclif->add_handler = aclif_add_handler;
	aclif->add_packet_handler = aclif_add_packet_handler;
	aclif->set_url = aclif_set_url;
	aclif->set_body = aclif_set_body;
	aclif->set_header_name = aclif_set_header_name;
	aclif->set_header_value = aclif_set_header_value;
	aclif->set_post_header_name = aclif_set_post_header_name;
	aclif->set_post_header_value = aclif_set_post_header_value;
	aclif->set_post_header_data = aclif_set_post_header_data;
	aclif->check_header = aclif_check_header;
	aclif->multi_part_start = aclif_multi_part_start;
	aclif->multi_part_complete = aclif_multi_part_complete;
	aclif->multi_body_complete = aclif_multi_body_complete;
	aclif->post_headers_destroy_sub = aclif_post_headers_destroy_sub;
	aclif->check_headers = aclif_check_headers;
	aclif->decode_post_headers = aclif_decode_post_headers;
	aclif->show_request = aclif_show_request;
	aclif->print_header = aclif_print_header;
	aclif->is_post_header_present = aclif_is_post_header_present;
	aclif->get_post_header_data_int = aclif_get_post_header_data_int;
	aclif->get_post_header_data_str = aclif_get_post_header_data_str;
	aclif->get_post_header_data_json = aclif_get_post_header_data_json;
	aclif->get_post_header_content_type = aclif_get_post_header_content_type;
	aclif->get_post_headers_count = aclif_get_post_headers_count;

	aclif->delete_online_player = aclif_delete_online_player;
	aclif->real_delete_online_player = aclif_real_delete_online_player;
	aclif->add_online_player = aclif_add_online_player;
	aclif->create_online_login_data = aclif_create_online_login_data;
	aclif->add_char_server = aclif_add_char_server;
	aclif->remove_char_server = aclif_remove_char_server;
	aclif->purge_disconnected_users = aclif_purge_disconnected_users;
	aclif->purge_disconnected_user = aclif_purge_disconnected_user;
	aclif->get_char_server_id = aclif_get_char_server_id;
	aclif->add_online_char = aclif_add_online_char;
	aclif->add_remove_timer = aclif_add_remove_timer;
	aclif->remove_remove_timer = aclif_remove_remove_timer;

	aclif->reportError = aclif_reportError;
}
