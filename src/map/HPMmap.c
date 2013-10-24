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

void HPM_map_addToMSD(struct map_session_data *sd, void *data, unsigned int id, unsigned int type, bool autofree) {
	struct HPluginData *HPData;
	unsigned int i;
	
	for(i = 0; i < sd->hdatac; i++) {
		if( sd->hdata[i]->pluginID == id && sd->hdata[i]->type == type ) {
			ShowError("HPMi->addToMSD:%s: error! attempting to insert duplicate struct of type %u on '%s'\n",HPM->pid2name(id),type,sd->status.name);
			return;
		}
	}
	
	CREATE(HPData, struct HPluginData, 1);

	HPData->pluginID = id;
	HPData->type = type;
	HPData->flag.free = autofree ? 1 : 0;
	HPData->data = data;
	
	RECREATE(sd->hdata,struct HPluginData *,++sd->hdatac);
	sd->hdata[sd->hdatac - 1] = HPData;
}
void *HPM_map_getFromMSD(struct map_session_data *sd, unsigned int id, unsigned int type) {
	unsigned int i;
	
	for(i = 0; i < sd->hdatac; i++) {
		if( sd->hdata[i]->pluginID == id && sd->hdata[i]->type == type ) {
			break;
		}
	}

	if( i != sd->hdatac )
		return sd->hdata[i]->data;
	
	return NULL;
}
void HPM_map_removeFromMSD(struct map_session_data *sd, unsigned int id, unsigned int type) {
	unsigned int i;
	
	for(i = 0; i < sd->hdatac; i++) {
		if( sd->hdata[i]->pluginID == id && sd->hdata[i]->type == type ) {
			break;
		}
	}
	
	if( i != sd->hdatac ) {
		unsigned int cursor;
		
		aFree(sd->hdata[i]->data);
		aFree(sd->hdata[i]);
		sd->hdata[i] = NULL;
		
		for(i = 0, cursor = 0; i < sd->hdatac; i++) {
			if( sd->hdata[i] == NULL )
				continue;
			if( i != cursor )
				sd->hdata[cursor] = sd->hdata[i];
			cursor++;
		}
		
		sd->hdatac = cursor;
	}
	
}
void HPM_map_plugin_load_sub(struct hplugin *plugin) {
	plugin->hpi->addCommand		= HPM->import_symbol("addCommand",plugin->idx);
	plugin->hpi->addScript		= HPM->import_symbol("addScript",plugin->idx);
	/* */
	plugin->hpi->addToMSD		= HPM->import_symbol("addToMSD",plugin->idx);
	plugin->hpi->getFromMSD		= HPM->import_symbol("getFromMSD",plugin->idx);
	plugin->hpi->removeFromMSD	= HPM->import_symbol("removeFromMSD",plugin->idx);
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
		if( !atcommand->add(atcommand_list[i].name,atcommand_list[i].func) ) {
			ShowDebug("HPM_map_atcommands: duplicate command '%s', skipping...\n", atcommand_list[i].name);
			continue;
		}
	}
}

void HPM_map_do_final(void) {
	if( atcommand_list )
		aFree(atcommand_list);
}
