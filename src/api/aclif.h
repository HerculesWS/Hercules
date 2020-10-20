/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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
#ifndef API_ACLIF_H
#define API_ACLIF_H

#include "api/api.h"
#include "common/hercules.h"
#include "common/db.h"
#include "api/handlerfunc.h"
#include "api/httphandler.h"
#include "api/jsonparser.h"

#include <stdarg.h>

#ifndef MAX_URL_SIZE
#define MAX_URL_SIZE 30
#endif
#ifndef MAX_BODY_SIZE
#define MAX_BODY_SIZE 100000
#endif
#ifndef MAX_REQUEST_SIZE
#define MAX_REQUEST_SIZE 150000
#endif

#ifndef MAX_HEADER_COUNT
#define MAX_HEADER_COUNT 20
#endif
#ifndef MAX_HEADER_NAME_SIZE
#define MAX_HEADER_NAME_SIZE 30
#endif
#ifndef MAX_HEADER_VALUE_SIZE
#define MAX_HEADER_VALUE_SIZE 200
#endif
#ifndef MAX_POST_HEADER_COUNT
#define MAX_POST_HEADER_COUNT 7
#endif
#ifndef MAX_POST_HEADER_NAME_SIZE
#define MAX_POST_HEADER_NAME_SIZE 20
#endif
#ifndef MAX_POST_HEADER_VALUE_SIZE
#define MAX_POST_HEADER_VALUE_SIZE 100
#endif
#ifndef MAX_POST_HEADER_DATA_SIZE
#define MAX_POST_HEADER_DATA_SIZE 100000
#endif
#ifndef MIN_BOUNDARY_SIZE
#define MIN_BOUNDARY_SIZE 30
#endif
#ifndef MAX_BOUNDARY_SIZE
#define MAX_BOUNDARY_SIZE 50
#endif
#ifndef MAX_TEMP_HEADER_SIZE
#define MAX_TEMP_HEADER_SIZE 5000
#endif

#ifndef HTTP_MAX_PROTOCOL
#define HTTP_MAX_PROTOCOL (HTTP_SOURCE + 1)
#endif

#ifndef AUTH_TOKEN_SIZE
#define AUTH_TOKEN_SIZE 16
#endif

union DBKey;
struct DBData;

enum req_flags {
	REQ_DEFAULT        = 0x0000,
	REQ_AUTO_CLOSE     = 0x0001,
	REQ_ACCOUNT_ID     = 0x0002,
	REQ_WORLD_NAME     = 0x0004,
	REQ_AUTH_TOKEN     = 0x0008,
	REQ_CHAR_ID        = 0x0010,
	REQ_GUILD_ID       = 0x0020,
	REQ_IMG_TYPE       = 0x0040,
	REQ_IMG            = 0x0080,
	REQ_CHAR_LOGGED_IN = 0x0100,
	REQ_VERSION        = 0x0200,
	REQ_DATA           = 0x0400,

	REQ_API = REQ_ACCOUNT_ID | REQ_WORLD_NAME,
	REQ_API_AUTH = REQ_ACCOUNT_ID | REQ_WORLD_NAME | REQ_AUTH_TOKEN,
	REQ_EMBLEM_UPLOAD = REQ_ACCOUNT_ID | REQ_WORLD_NAME | REQ_AUTH_TOKEN | REQ_API_AUTH | REQ_GUILD_ID | REQ_IMG_TYPE | REQ_IMG | REQ_CHAR_LOGGED_IN
};

struct online_api_login_data {
	int account_id;
	int char_id;
	unsigned char auth_token[AUTH_TOKEN_SIZE];
	int64 remove_tick;
};

struct char_server_data {
	int id;
};

/**
 * aclif.c Interface
 **/
struct aclif_interface {
	/* vars */
	uint32 api_ip;
	uint32 bind_ip;
	uint16 api_port;
	char api_ip_str[128];
	int api_fd;
	int remove_disconnected_delay;
	int id_counter;

	struct DBMap *handlers_db[HTTP_MAX_PROTOCOL];
	struct DBMap *online_db;
	struct DBMap *char_servers_db;
	struct DBMap *char_servers_id_db;

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);
	bool (*setip) (const char* ip);
	bool (*setbindip) (const char* ip);
	void (*setport) (uint16 port);
	uint32 (*refresh_ip) (void);
	int (*parse) (int fd);
	int (*parse_request) (int fd, struct api_session_data *sd);
	void (*terminate_connection) (int fd);
	int (*connected) (int fd);
	int (*session_delete) (int fd);
	void (*init_handlers) (void);
	void (*register_handlers) (void);
	void (*add_handler) (http_method method, const char *url, HttpParseHandler func, Handler_func func2, int msg_id, int flags);
	void (*add_packet_handler) (Handler_func func2, int msg_id);
	void (*set_url) (int fd, http_method method, const char *url, size_t size);
	void (*set_body) (int fd, const char *body, size_t size);
	void (*set_header_name) (int fd, const char *name, size_t size);
	void (*set_header_value) (int fd, const char *value, size_t size);
	void (*set_post_header_name) (int fd, const char *name, size_t size);
	void (*set_post_header_value) (int fd, const char *value, size_t size);
	void (*set_post_header_data) (int fd, const char *data, size_t size);
	bool (*check_header) (int fd, struct api_session_data *sd, const char *name, const char *value, size_t value_size);
	void (*multi_part_start) (int fd, struct api_session_data *sd);
	void (*multi_part_complete) (int fd, struct api_session_data *sd);
	void (*multi_body_complete) (int fd, struct api_session_data *sd);
	int (*post_headers_destroy_sub) (union DBKey key, struct DBData *data, va_list ap);
	void (*reportError) (int fd, struct api_session_data *sd);
	void (*check_headers) (int fd, struct api_session_data *sd);
	bool (*decode_post_headers) (int fd, struct api_session_data *sd);
	int (*print_header) (union DBKey key, struct DBData *data, va_list ap);
	bool (*is_post_header_present) (struct api_session_data *sd, const char *name);
	bool (*get_post_header_data_int) (struct api_session_data *sd, const char *name, int *account_id);
	bool (*get_post_header_data_str) (struct api_session_data *sd, const char *name, char **data, uint32_t *data_size);
	bool (*get_post_header_data_json) (struct api_session_data *sd, const char *name, JsonP **json);
	bool (*get_post_header_content_type) (struct api_session_data *sd, const char *name, char **content_type);
	int (*get_post_headers_count) (struct api_session_data *sd);

	void (*delete_online_player) (int account_id);
	void (*real_delete_online_player) (int account_id);
	void (*add_online_player) (int account_id, const unsigned char *auth_token);
	void (*add_online_char) (int account_id, int char_id);
	struct DBData (*create_online_login_data) (union DBKey key, va_list args);
	void (*add_char_server) (int char_server_id, const char *name);
	void (*remove_char_server) (int char_server_id, const char *name);
	int (*purge_disconnected_users) (int tid, int64 tick, int id, intptr_t data);
	int (*purge_disconnected_user) (union DBKey key, struct DBData *data, va_list ap);
	int (*get_char_server_id) (struct api_session_data *sd);

	void (*show_request) (int fd, struct api_session_data *sd, bool show_http_headers);
};

#ifdef HERCULES_CORE
void aclif_defaults(void);
#else
#define addHttpHandler(method, url, func, flags) \
	( \
		(void)((bool (*)(int fd, struct api_session_data *sd))0 == (func)), \
		aclif->add_handler(method, url, func, NULL, 0, flags) \
	)
#endif // HERCULES_CORE

HPShared struct aclif_interface *aclif;

#endif /* API_ACLIF_H */
