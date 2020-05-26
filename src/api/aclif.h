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
#include "api/httphandler.h"

#include <stdarg.h>

#ifndef MAX_URL_SIZE
#define MAX_URL_SIZE 30
#endif
#ifndef MAX_BODY_SIZE
#define MAX_BODY_SIZE 100000
#endif

#ifndef MAX_HEADER_COUNT
#define MAX_HEADER_COUNT 10
#endif
#ifndef MAX_HEADER_NAME_SIZE
#define MAX_HEADER_NAME_SIZE 30
#endif
#ifndef MAX_HEADER_VALUE_SIZE
#define MAX_HEADER_VALUE_SIZE 200
#endif
#ifndef HTTP_MAX_PROTOCOL
#define HTTP_MAX_PROTOCOL (HTTP_SOURCE + 1)
#endif

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
	struct DBMap *handlers_db[HTTP_MAX_PROTOCOL];

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);
	bool (*setip) (const char* ip);
	bool (*setbindip) (const char* ip);
	void (*setport) (uint16 port);
	uint32 (*refresh_ip) (void);
	int (*parse) (int fd);
	int (*parse_request) (int fd, struct api_session_data *sd);
	int (*connected) (int fd);
	int (*session_delete) (int fd);
	void (*load_handlers) (void);
	void (*add_handler) (enum http_method method, const char *url, HttpParseHandler func);
	void (*set_url) (int fd, enum http_method method, const char *url, size_t size);
	void (*set_body) (int fd, const char *body, size_t size);
	void (*set_header_name) (int fd, const char *name, size_t size);
	void (*set_header_value) (int fd, const char *value, size_t size);
	void (*reportError) (int fd, struct api_session_data *sd);

	bool (*parse_userconfig_load) (int fd, struct api_session_data *sd);
};

#ifdef HERCULES_CORE
void aclif_defaults(void);
#endif // HERCULES_CORE

HPShared struct aclif_interface *aclif;

#endif /* API_ACLIF_H */
