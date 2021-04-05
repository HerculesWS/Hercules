/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020-2021 Andrei Karas (4144)
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
#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "api/api.h"
#include "common/hercules.h"
#include "common/db.h"
#include "api/httphandler.h"
#include "api/jsonparser.h"

#include <stdarg.h>

struct userconfig_userhotkeys_v2;

/**
 * handlers.c Interface
 **/
struct handlers_interface {
	int (*init) (bool minimal);
	void (*final) (void);
	void (*sendHotkeyV2Tab) (JsonP *json, struct userconfig_userhotkeys_v2 *hotkeys);
	const char *(*hotkeyTabIdToName) (int tab_id);

#define handler(method, url, func, flags) bool (*parse_ ## func) (int fd, struct api_session_data *sd)
#define handler2(method, url, func, flags) bool (*parse_ ## func) (int fd, struct api_session_data *sd); \
	void (*func) (int fd, struct api_session_data *sd, const void *data, size_t data_size)
#define packet_handler(func) void (*func) (int fd, struct api_session_data *sd, const void *data, size_t data_size)
#include "api/urlhandlers.h"
#undef handler
#undef handler2
#undef packet_handler
};

#ifdef HERCULES_CORE
void handlers_defaults(void);
#endif // HERCULES_CORE

HPShared struct handlers_interface *handlers;

#endif /* API_HANDLERS_H */
