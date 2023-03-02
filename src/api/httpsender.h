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
#ifndef API_HTTPSENDER_H
#define API_HTTPSENDER_H

#include "common/hercules.h"

#include "api/jsonwriter.h"

#include <stdarg.h>

#ifndef MAX_RESPONSE_SIZE
#define MAX_RESPONSE_SIZE 50000
#endif

#if MAX_RESPONSE_SIZE < 100
#error MAX_RESPONSE_SIZE must be atleast 100 bytes
#endif  // MAX_RESPONSE_SIZE < 100

struct api_session_data;

/**
 * httpsender.c Interface
 **/
struct httpsender_interface {
	char *tmp_buffer;
	char *server_name;
	int (*init) (bool minimal);
	void (*final) (void);

	bool (*send_plain) (int fd, const char *data);
	bool (*send_html) (int fd, const char *data);
	bool (*send_json) (int fd, const JsonW *json);
	bool (*send_binary) (int fd, const char *data, const size_t data_len);
};

#ifdef HERCULES_CORE
void httpsender_defaults(void);
#endif // HERCULES_CORE

HPShared struct httpsender_interface *httpsender;

#endif /* API_HTTPSENDER_H */
