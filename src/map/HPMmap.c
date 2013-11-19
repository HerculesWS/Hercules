// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/HPM.h"

#include "HPMmap.h"
#include "pc.h"
#include "map.h"

//
#include "atcommand.h"
#include "chat.h"
#include "chrif.h"
#include "duel.h"
#include "elemental.h"
#include "homunculus.h"
#include "instance.h"
#include "intif.h"
#include "irc-bot.h"
#include "mail.h"
#include "mapreg.h"
#include "mercenary.h"
#include "party.h"
#include "pet.h"
#include "quest.h"
#include "storage.h"
#include "trade.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

void HPM_map_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr) {
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
		default:
			ret->HPDataSRCPtr = NULL;
			ret->hdatac = NULL;
			return;
	}
}

void HPM_map_plugin_load_sub(struct hplugin *plugin) {
	plugin->hpi->addCommand = HPM->import_symbol("addCommand",plugin->idx);
	plugin->hpi->addScript  = HPM->import_symbol("addScript",plugin->idx);
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

void HPM_map_do_final(void) {
	if( atcommand_list )
		aFree(atcommand_list);
}
