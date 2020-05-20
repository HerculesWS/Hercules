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

#define MAX_URL_SIZE 30

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
	struct DBMap *handlers_db;

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);
	bool (*setip) (const char* ip);
	bool (*setbindip) (const char* ip);
	void (*setport) (uint16 port);
	uint32 (*refresh_ip) (void);
	int (*parse) (int fd);
	int (*connected) (int fd);
	int (*session_delete) (int fd);
	void (*load_handlers) (void);
	void (*add_handler) (enum http_method method, const char *url, HttpParseHandler func);
	void (*set_url) (int fd, const char *url, size_t size);

	bool (*parse_userconfig_load) (int fd, struct api_session_data *sd);
};

#ifdef HERCULES_CORE
void aclif_defaults(void);
#endif // HERCULES_CORE

HPShared struct aclif_interface *aclif;

#endif /* API_ACLIF_H */
