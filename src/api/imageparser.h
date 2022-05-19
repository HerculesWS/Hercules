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
#ifndef API_IMAGEPARSER_H
#define API_IMAGEPARSER_H

#include "common/hercules.h"

#include <libgif/gif_lib.h>

#include <stdarg.h>

struct gif_user_data {
	const char *emblem;
	uint64 emblem_len;
	uint64 read_pos;
};

/**
 * imageparser.c Interface
 **/
struct imageparser_interface {
	int (*init) (bool minimal);
	void (*final) (void);
	bool (*validate_bmp_emblem) (const char *emblem, uint64 emblem_len);
	bool (*validate_gif_emblem) (const char *emblem, uint64 emblem_len);
	int (*read_gif_func) (GifFileType *gif, GifByteType *buf, int len);
};

#ifdef HERCULES_CORE
void imageparser_defaults(void);
#endif // HERCULES_CORE

HPShared struct imageparser_interface *imageparser;

#endif /* API_IMAGEPARSER_H */
