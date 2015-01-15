// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_GEOIP_H
#define CHAR_GEOIP_H

#include "../common/cbasetypes.h"

/**
 * GeoIP information
 **/
struct s_geoip {
	unsigned char *cache; // GeoIP.dat information see geoip->init()
	bool active;
};

/**
 * geoip interface
 **/
struct geoip_interface {
	struct s_geoip *data;
	const char* (*getcountry) (uint32 ipnum);
	void (*final) (bool shutdown);
	void (*init) (void);
};

struct geoip_interface *geoip;

#ifdef HERCULES_CORE
void geoip_defaults(void);
#endif // HERCULES_CORE

#endif /* CHAR_GEOIP_H */
