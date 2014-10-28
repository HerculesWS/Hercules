// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#define HERCULES_CORE

#include "HPMchar.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/HPM.h"
#include "../common/cbasetypes.h"
#include "../common/conf.h"
#include "../common/db.h"
#include "../common/des.h"
#include "../common/ers.h"
#include "../common/malloc.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sysinfo.h"

#include "../common/HPMDataCheck.h"

bool HPM_char_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr) {
	/* record address */
	switch( type ) {
		default:
			return false;
	}
	return true;
}

void HPM_char_plugin_load_sub(struct hplugin *plugin) {
}

void HPM_char_do_init(void) {
#if 0 // TODO (HPMDataCheck is disabled for the time being)
	HPM->datacheck_init(HPMDataCheck, HPMDataCheckLen, HPMDataCheckVer);
#else
	HPM->DataCheck = NULL;
#endif
}

void HPM_char_do_final(void) {
#if 0 // TODO (HPMDataCheck is disabled for the time being)
	HPM->datacheck_final();
#endif
}
