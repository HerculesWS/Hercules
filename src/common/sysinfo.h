/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
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
#ifndef COMMON_SYSINFO_H
#define COMMON_SYSINFO_H

/**
 * Provides various bits of information about the system Hercules is running on
 * (note: on unix systems, to avoid runtime detection, most of the data is
 * cached at compile time).
 *
 * Base Author: Haru @ http://herc.ws
 */

#include "common/hercules.h"

struct sysinfo_private;

/**
 * sysinfo.c interface
 **/
struct sysinfo_interface {
	struct sysinfo_private *p;

#if defined(WIN32) && !defined(__CYGWIN__)
	long (*getpagesize) (void);
#else
	int (*getpagesize) (void);
#endif
	const char *(*platform) (void);
	const char *(*osversion) (void);
	const char *(*cpu) (void);
	int (*cpucores) (void);
	const char *(*arch) (void);
	bool (*is64bit) (void);
	const char *(*compiler) (void);
	const char *(*cflags) (void);
	const char *(*time) (void);
	const char *(*vcstype) (void);
	int (*vcstypeid) (void);
	const char *(*vcsrevision_src) (void);
	const char *(*vcsrevision_scripts) (void);
	void (*vcsrevision_reload) (void);
	bool (*is_superuser) (void);
	void (*init) (void);
	void (*final) (void);
};

#ifdef HERCULES_CORE
void sysinfo_defaults(void);
#endif // HERCULES_CORE

HPShared struct sysinfo_interface *sysinfo;

#endif /* COMMON_SYSINFO_H */
