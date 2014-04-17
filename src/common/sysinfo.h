// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Base Author: Haru @ http://hercules.ws

#ifndef _COMMON_SYSINFO_H_
#define _COMMON_SYSINFO_H_

/**
 * Provides various bits of information about the system Hercules is running on
 * (note: on unix systems, to avoid runtime detection, most of the data is
 * cached at compile time)
 */

#include "../common/cbasetypes.h"

#ifdef _COMMON_SYSINFO_P_
struct sysinfo_private {
	char *platform;
	char *osversion;
	char *cpu;
	int cpucores;
	char *arch;
	char *compiler;
	char *cflags;
	char *vcstype_name;
	int vcstype;
	char *vcsrevision_src;
	char *vcsrevision_scripts;
};
#else
struct sysinfo_private;
#endif

/**
 * sysinfo.c interface
 **/
struct sysinfo_interface {
	struct sysinfo_private *p;

	const char *(*platform) (void);
	const char *(*osversion) (void);
	const char *(*cpu) (void);
	int (*cpucores) (void);
	const char *(*arch) (void);
	bool (*is64bit) (void);
	const char *(*compiler) (void);
	const char *(*cflags) (void);
	const char *(*vcstype) (void);
	int (*vcstypeid) (void);
	const char *(*vcsrevision_src) (void);
	const char *(*vcsrevision_scripts) (void);
	void (*vcsrevision_reload) (void);
	bool (*is_superuser) (void);
	void (*init) (void);
	void (*final) (void);
};

struct sysinfo_interface *sysinfo;

void sysinfo_defaults(void);

#endif /* _COMMON_SYSINFO_H_ */
