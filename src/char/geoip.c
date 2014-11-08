// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "geoip.h"

#include <stdlib.h>

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

struct s_geoip geoip_data;

const char* geoip_getcountry(uint32 ipnum);
void geoip_final(bool shutdown);
void geoip_init(void);

void geoip_defaults(void) {
	geoip = &geoip_s;

	geoip->data = &geoip_data;

	geoip->getcountry = geoip_getcountry;
	geoip->final = geoip_final;
	geoip->init = geoip_init;
}
