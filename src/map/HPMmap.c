// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/HPM.h"

#include "HPMmap.h"
#include "pc.h"
#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


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
	plugin->hpi->addCommand		= HPM->import_symbol("addCommand");
	plugin->hpi->addScript		= HPM->import_symbol("addScript");
	/* */
	plugin->hpi->addToMSD		= HPM->import_symbol("addToMSD");
	plugin->hpi->getFromMSD		= HPM->import_symbol("getFromMSD");
	plugin->hpi->removeFromMSD	= HPM->import_symbol("removeFromMSD");
}
