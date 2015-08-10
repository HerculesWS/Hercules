// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#define HERCULES_CORE

#include "HPMchar.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"

#if 0 // TODO (HPMDataCheck is disabled for the time being)
#include "char/char.h"
#include "char/geoip.h"
#include "char/inter.h"
#include "char/int_auction.h"
#include "char/int_elemental.h"
#include "char/int_guild.h"
#include "char/int_homun.h"
#include "char/int_mail.h"
#include "char/int_mercenary.h"
#include "char/int_party.h"
#include "char/int_pet.h"
#include "char/int_quest.h"
#include "char/int_storage.h"
#include "char/loginif.h"
#include "char/mapif.h"
#include "char/pincode.h"
#include "common/HPMi.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/des.h"
#include "common/ers.h"
#include "common/malloc.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/spinlock.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"

// HPMDataCheck comes after all the other includes
#include "common/HPMDataCheck.h"
#endif

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
