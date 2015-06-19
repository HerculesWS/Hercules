// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

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

bool HPM_map_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr) {
	/* record address */
	switch( type ) {
		case HPDT_MSD:
			ret->HPDataSRCPtr = (void**)(&((struct map_session_data *)ptr)->hdata);
			ret->hdatac = &((struct map_session_data *)ptr)->hdatac;
			break;
		case HPDT_NPCD:
			ret->HPDataSRCPtr = (void**)(&((struct npc_data *)ptr)->hdata);
			ret->hdatac = &((struct npc_data *)ptr)->hdatac;
			break;
		case HPDT_MAP:
			ret->HPDataSRCPtr = (void**)(&((struct map_data *)ptr)->hdata);
			ret->hdatac = &((struct map_data *)ptr)->hdatac;
			break;
		case HPDT_PARTY:
			ret->HPDataSRCPtr = (void**)(&((struct party_data *)ptr)->hdata);
			ret->hdatac = &((struct party_data *)ptr)->hdatac;
			break;
		case HPDT_GUILD:
			ret->HPDataSRCPtr = (void**)(&((struct guild *)ptr)->hdata);
			ret->hdatac = &((struct guild *)ptr)->hdatac;
			break;
		case HPDT_INSTANCE:
			ret->HPDataSRCPtr = (void**)(&((struct instance_data *)ptr)->hdata);
			ret->hdatac = &((struct instance_data *)ptr)->hdatac;
			break;
		case HPDT_MOBDB:
			ret->HPDataSRCPtr = (void**)(&((struct mob_db *)ptr)->hdata);
			ret->hdatac = &((struct mob_db *)ptr)->hdatac;
			break;
		case HPDT_MOBDATA:
			ret->HPDataSRCPtr = (void**)(&((struct mob_data *)ptr)->hdata);
			ret->hdatac = &((struct mob_data *)ptr)->hdatac;
			break;
		case HPDT_ITEMDATA:
			ret->HPDataSRCPtr = (void**)(&((struct item_data *)ptr)->hdata);
			ret->hdatac = &((struct item_data *)ptr)->hdatac;
			break;
		case HPDT_BGDATA:
			ret->HPDataSRCPtr = (void**)(&((struct battleground_data *)ptr)->hdata);
			ret->hdatac = &((struct battleground_data *)ptr)->hdatac;
			break;
		default:
			return false;
	}
	return true;
}

void HPM_map_plugin_load_sub(struct hplugin *plugin) {
	plugin->hpi->addCommand       = HPM->import_symbol("addCommand",plugin->idx);
	plugin->hpi->addScript        = HPM->import_symbol("addScript",plugin->idx);
	plugin->hpi->addPCGPermission = HPM->import_symbol("addGroupPermission",plugin->idx);
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
	HPM->grabHPDataSub = HPM_map_grabHPData;
	HPM->datacheck_init(HPMDataCheck, HPMDataCheckLen, HPMDataCheckVer);
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
