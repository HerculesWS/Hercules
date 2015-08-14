// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#define HERCULES_CORE

#include "HPMlogin.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"

#include "login/account.h"
#include "login/login.h"
#include "common/HPMi.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/des.h"
#include "common/ers.h"
#include "common/malloc.h"
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

bool HPM_login_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr) {
	/* record address */
	switch( type ) {
		default:
			return false;
	}
	return true;
}

void HPM_login_plugin_load_sub(struct hplugin *plugin) {
	plugin->hpi->sql_handle = account_db_sql_up(login->accounts);
}

void HPM_login_do_init(void) {
	HPM->datacheck_init(HPMDataCheck, HPMDataCheckLen, HPMDataCheckVer);
	HPM_shared_symbols(SERVER_TYPE_LOGIN);
}

void HPM_login_do_final(void) {
	HPM->datacheck_final();
}
