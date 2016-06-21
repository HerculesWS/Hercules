/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2014-2015  Hercules Dev Team
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
#define HERCULES_CORE

#include "HPMlogin.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"

#include "login/account.h"
#include "login/lclif.h"
#include "login/lclif.p.h"
#include "login/login.h"
#include "common/HPMi.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/des.h"
#include "common/ers.h"
#include "common/memmgr.h"
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

/**
 * HPM plugin data store validator sub-handler (login-server)
 *
 * @see HPM_interface::data_store_validate
 */
bool HPM_login_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize)
{
	switch (type) {
		// No supported types at the moment
		default:
			break;
	}
	return false;
}

void HPM_login_plugin_load_sub(struct hplugin *plugin) {
	plugin->hpi->sql_handle = account_db_sql_up(login->accounts);
}

void HPM_login_do_init(void) {
	HPM->load_sub = HPM_login_plugin_load_sub;
	HPM->data_store_validate_sub = HPM_login_data_store_validate;
	HPM->datacheck_init(HPMDataCheck, HPMDataCheckLen, HPMDataCheckVer);
	HPM_shared_symbols(SERVER_TYPE_LOGIN);
}

void HPM_login_do_final(void) {
	HPM->datacheck_final();
}
