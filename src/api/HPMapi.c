/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2020 Hercules Dev Team
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

#include "HPMapi.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"

#include "common/HPMi.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/des.h"
#include "common/ers.h"
#include "common/grfio.h"
#include "common/md5calc.h"
#include "common/memmgr.h"
#include "common/mutex.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/packets.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/spinlock.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"
#include "api/achrif.h"
#include "api/aclif.h"
#include "api/aloginif.h"
#include "api/api.h"
#include "api/handlers.h"
#include "api/httpparser.h"
#include "api/httpsender.h"
#include "api/jsonparser.h"
#include "api/jsonwriter.h"

// HPMDataCheck comes after all the other includes
#include "common/HPMDataCheck.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * HPM plugin data store validator sub-handler (api-server)
 *
 * @see HPM_interface::data_store_validate
 */
bool HPM_api_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize)
{
	switch (type) {
	case HPDT_MSD:
	case HPDT_NPCD:
	case HPDT_MAP:
	case HPDT_PARTY:
	case HPDT_GUILD:
	case HPDT_INSTANCE:
	case HPDT_MOBDB:
	case HPDT_MOBDATA:
	case HPDT_ITEMDATA:
	case HPDT_BGDATA:
	case HPDT_AUTOTRADE_VEND:
	case HPDT_CLAN:
		// Initialized by the caller.
		return true;
	default:
		break;
	}
	return false;
}

void HPM_api_plugin_load_sub(struct hplugin *plugin)
{
	plugin->hpi->sql_handle = api->mysql_handle;
}


void HPM_api_do_init(void)
{
	HPM->load_sub = HPM_api_plugin_load_sub;
	HPM->data_store_validate_sub = HPM_api_data_store_validate;
	HPM->datacheck_init(HPMDataCheck, HPMDataCheckLen, HPMDataCheckVer);
	HPM_shared_symbols(SERVER_TYPE_MAP);
}

void HPM_api_do_final(void)
{
	HPM->datacheck_final();
}
