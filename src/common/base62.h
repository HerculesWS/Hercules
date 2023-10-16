/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2023-2023 Hercules Dev Team
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
#ifndef COMMON_BASE62_H
#define COMMON_BASE62_H

#include "common/hercules.h"

/**
 * Minimum size for a buffer to encode UINT32_MAX, including NULL-terminator
 */
#define BASE62_INT_BUFFER_LEN 7

/**
 * The base62 interface
 **/
struct base62_interface {
	bool (*encode_int_padded) (int value, char *buf, int min_len, int buf_len);
};

#ifdef HERCULES_CORE
void base62_defaults(void);
#endif // HERCULES_CORE

HPShared struct base62_interface *base62;

#endif // COMMON_BASE62_H
