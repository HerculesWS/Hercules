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
#ifndef API_HTTPSENDER_H
#define API_HTTPSENDER_H

#include "common/hercules.h"

#include <stdarg.h>

#define MAX_RESPONSE_SIZE 50000

struct api_session_data;

/**
 * httpsender.c Interface
 **/
struct httpsender_interface {
	char *tmp_buffer;
	char *server_name;
	int (*init) (bool minimal);
	void (*final) (void);

	bool (*send_html) (int fd, const char *data);
};

#ifdef HERCULES_CORE
void httpsender_defaults(void);
#endif // HERCULES_CORE

HPShared struct httpsender_interface *httpsender;

#endif /* API_HTTPSENDER_H */
