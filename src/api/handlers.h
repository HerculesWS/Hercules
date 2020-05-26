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
#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include "api/api.h"
#include "common/hercules.h"
#include "common/db.h"
#include "api/httphandler.h"

#include <stdarg.h>

/**
 * handlers.c Interface
 **/
struct handlers_interface {
	int (*init) (bool minimal);
	void (*final) (void);

#define handler(method, url, func, flags) bool (*parse_ ## func) (int fd, struct api_session_data *sd)
#include "api/urlhandlers.h"
#undef handler
};

#ifdef HERCULES_CORE
void handlers_defaults(void);
#endif // HERCULES_CORE

HPShared struct handlers_interface *handlers;

#endif /* API_HANDLERS_H */
