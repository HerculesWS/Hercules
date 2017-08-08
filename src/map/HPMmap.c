/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
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

#include "HPMmap.h"

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
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/spinlock.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"
#include "map/atcommand.h"
#include "map/battle.h"
#include "map/battleground.h"
#include "map/buyingstore.h"
#include "map/channel.h"
#include "map/chat.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/date.h"
#include "map/duel.h"
#include "map/elemental.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/instance.h"
#include "map/intif.h"
#include "map/irc-bot.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/mail.h"
#include "map/map.h"
#include "map/mapreg.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/packets_struct.h"
#include "map/party.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/pc_groups.h"
#include "map/pet.h"
#include "map/quest.h"
#include "map/rodex.h"
#include "map/script.h"
#include "map/searchstore.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/storage.h"
#include "map/trade.h"
#include "map/unit.h"
#include "map/vending.h"

// HPMDataCheck comes after all the other includes
#include "common/HPMDataCheck.h"

#include <stdio.h>
#include <stdlib.h>

struct HPM_atcommand_list {
	//tracking currently not enabled
	// - requires modifying how plugins calls atcommand creation
	// - needs load/unload during runtime support
	//unsigned int pID;/* plugin id */
	char name[ATCOMMAND_LENGTH];
	AtCommandFunc func;
};

struct HPM_atcommand_list *atcommand_list = NULL;
unsigned int atcommand_list_items = 0;

/**
 * HPM plugin data store validator sub-handler (map-server)
 *
 * @see HPM_interface::data_store_validate
 */
bool HPM_map_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize)
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
			// Initialized by the caller.
			return true;
		default:
			break;
	}
	return false;
}

void HPM_map_plugin_load_sub(struct hplugin *plugin) {
	plugin->hpi->sql_handle = map->mysql_handle;
	plugin->hpi->addCommand = atcommand->create;
	plugin->hpi->addScript  = script->addScript;
	plugin->hpi->addPCGPermission = HPM_map_add_group_permission;
}

bool HPM_map_add_atcommand(char *name, AtCommandFunc func) {
	unsigned int i = 0;

	for(i = 0; i < atcommand_list_items; i++) {
		if( !strcmpi(atcommand_list[i].name,name) ) {
			ShowDebug("HPM_map_add_atcommand: duplicate command '%s', skipping...\n", name);
			return false;
		}
	}

	i = atcommand_list_items;

	RECREATE(atcommand_list, struct HPM_atcommand_list , ++atcommand_list_items);

	safestrncpy(atcommand_list[i].name, name, sizeof(atcommand_list[i].name));
	atcommand_list[i].func = func;

	return true;
}

void HPM_map_atcommands(void) {
	unsigned int i;

	for(i = 0; i < atcommand_list_items; i++) {
		atcommand->add(atcommand_list[i].name,atcommand_list[i].func,true);
	}
}

/**
 * Adds a new group permission to the HPM-provided list
 **/
void HPM_map_add_group_permission(unsigned int pluginID, char *name, unsigned int *mask) {
	unsigned char index = pcg->HPMpermissions_count;

	RECREATE(pcg->HPMpermissions, struct pc_groups_new_permission, ++pcg->HPMpermissions_count);

	pcg->HPMpermissions[index].pID = pluginID;
	pcg->HPMpermissions[index].name = aStrdup(name);
	pcg->HPMpermissions[index].mask = mask;
}

void HPM_map_do_init(void) {
	HPM->load_sub = HPM_map_plugin_load_sub;
	HPM->data_store_validate_sub = HPM_map_data_store_validate;
	HPM->datacheck_init(HPMDataCheck, HPMDataCheckLen, HPMDataCheckVer);
	HPM_shared_symbols(SERVER_TYPE_MAP);
}

void HPM_map_do_final(void) {
	if (atcommand_list)
		aFree(atcommand_list);
	/**
	 * why is pcg->HPM being cleared here? because PCG's do_final is not final,
	 * is used on reload, and would thus cause plugin-provided permissions to go away
	 **/
	if (pcg->HPMpermissions) {
		unsigned char i;
		for (i = 0; i < pcg->HPMpermissions_count; i++) {
			aFree(pcg->HPMpermissions[i].name);
		}
		aFree(pcg->HPMpermissions);
	}
	HPM->datacheck_final();
}
