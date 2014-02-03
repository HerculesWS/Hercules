// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/HPM.h"
#include "../common/conf.h"
#include "../common/db.h"
#include "../common/des.h"
#include "../common/ers.h"
#include "../common/mapindex.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/strlib.h"


#include "HPMmap.h"
#include "pc.h"
#include "map.h"

//
#include "atcommand.h"
#include "battle.h"
#include "battleground.h"
#include "chat.h"
#include "chrif.h"
#include "clif.h"
#include "date.h"
#include "duel.h"
#include "elemental.h"
#include "guild.h"
#include "homunculus.h"
#include "instance.h"
#include "intif.h"
#include "irc-bot.h"
#include "itemdb.h"
#include "log.h"
#include "mail.h"
#include "mapreg.h"
#include "mercenary.h"
#include "mob.h"
#include "npc.h"
#include "party.h"
#include "path.h"
#include "pc_groups.h"
#include "pet.h"
#include "quest.h"
#include "script.h"
#include "searchstore.h"
#include "skill.h"
#include "status.h"
#include "storage.h"
#include "trade.h"
#include "unit.h"
#include "vending.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/HPMDataCheck.h"

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
 * (char*) data name -> (unsigned int) HPMDataCheck[] index
 **/
DBMap *datacheck_db;

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
 * Called by HPM->DataCheck on a plugins incoming data, ensures data structs in use are matching!
 **/
bool HPM_map_DataCheck (struct s_HPMDataCheck *src, unsigned int size, char *name) {
	unsigned int i, j;
	
	for(i = 0; i < size; i++) {
		
		if( !strdb_exists(datacheck_db, src[i].name) ) {
			ShowError("HPMDataCheck:%s: '%s' was not found\n",name,src[i].name);
			return false;
		} else {
			j = strdb_uiget(datacheck_db, src[i].name);/* not double lookup; exists sets cache to found data */
			if( src[i].size != HPMDataCheck[j].size ) {
				ShowWarning("HPMDataCheck:%s: '%s' size mismatch %u != %u\n",name,src[i].name,src[i].size,HPMDataCheck[j].size);
				return false;
			}
		}
	}
	
	return true;
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
	unsigned int i;
	
	/**
	 * Populates datacheck_db for easy lookup later on
	 **/
	datacheck_db = strdb_alloc(DB_OPT_BASE,0);
	
	for(i = 0; i < HPMDataCheckLen; i++) {
		strdb_uiput(datacheck_db, HPMDataCheck[i].name, i);
	}
	
}

void HPM_map_do_final(void) {
	unsigned char i;
	
	if( atcommand_list )
		aFree(atcommand_list);
	/**
	 * why is pcg->HPM being cleared here? because PCG's do_final is not final,
	 * is used on reload, and would thus cause plugin-provided permissions to go away
	 **/
	for( i = 0; i < pcg->HPMpermissions_count; i++ ) {
		aFree(pcg->HPMpermissions[i].name);
	}
	if( pcg->HPMpermissions )
		aFree(pcg->HPMpermissions);
	
	db_destroy(datacheck_db);
}
