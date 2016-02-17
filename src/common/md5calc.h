/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#ifndef COMMON_MD5CALC_H
#define COMMON_MD5CALC_H

#include "common/hercules.h"

struct md5_interface {
	void (*string) (const char *string, char *output);
	void (*binary) (const char *string, unsigned char *output);
	void (*salt) (unsigned int len, char *output);
};

#ifdef HERCULES_CORE
void md5_defaults(void);
#endif // HERCULES_CORE

HPShared struct md5_interface *md5;

#endif /* COMMON_MD5CALC_H */
