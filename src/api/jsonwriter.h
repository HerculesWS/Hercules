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
#ifndef API_JSONWRITER_H
#define API_JSONWRITER_H

#include "common/hercules.h"

#include <stdarg.h>

/**
 * jsonwriter.c Interface
 **/
struct jsonwriter_interface {
	int (*init) (bool minimal);
	void (*final) (void);
};

#ifdef HERCULES_CORE
void jsonwriter_defaults(void);
#endif // HERCULES_CORE

HPShared struct jsonwriter_interface *jsonwriter;

#endif /* API_JSONWRITER_H */
