#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common/HPMi.h"
#include "map/pc.h"
#include "common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {
	"storagecountitem", // Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

/*==========================================
 * storagecountitem(nameID,{accountID})
 * returns number of items that meet the conditions
 *==========================================*/
BUILDIN(storagecountitem) {
	int nameid, i;
	int count = 0;
	struct item_data* id = NULL;

	TBL_PC* sd = script->rid2sd(st);
	if( !sd )
		return true;

	if( script_isstringtype(st, 2) ) {
		// item name
		id = itemdb->search_name(script_getstr(st, 2));
	} else {
		// item id
		id = itemdb->exists(script_getnum(st, 2));
	}

	if( id == NULL ) {
		ShowError("buildin_storagecountitem: Invalid item '%s'.\n", script_getstr(st,2));  // returns string, regardless of what it was
		script_pushint(st,0);
		return false;
	}

	nameid = id->nameid;

	for(i = 0; i < MAX_STORAGE; i++)
		if(sd->status.storage.items[i].nameid == nameid)
			count += sd->status.storage.items[i].amount;

	script_pushint(st,count);
	return true;
}

/*==========================================
 * storagecountitem2(nameID,Identified,Refine,Attribute,Card0,Card1,Card2,Card3)
 * returns number of items that meet the conditions
 *==========================================*/
BUILDIN(storagecountitem2) {
	int nameid, iden, ref, attr, c0, c1, c2, c3;
	int count = 0;
	int i;
	struct item_data* id = NULL;

	TBL_PC* sd = script->rid2sd(st);
	if (!sd) {
		return true;
	}

	if( script_isstringtype(st, 2) ) { //item name
		id = itemdb->search_name(script_getstr(st, 2));
	} else { //item id
		id = itemdb->exists(script_getnum(st, 2));
	}

	if( id == NULL ) {
		ShowError("buildin_storagecountitem2: Invalid item '%s'.\n", script_getstr(st,2));  // returns string, regardless of what it was
		script_pushint(st,0);
		return false;
	}

	nameid = id->nameid;
	iden = script_getnum(st,3);
	ref  = script_getnum(st,4);
	attr = script_getnum(st,5);
	c0 = (short)script_getnum(st,6);
	c1 = (short)script_getnum(st,7);
	c2 = (short)script_getnum(st,8);
	c3 = (short)script_getnum(st,9);

	for(i = 0; i < MAX_STORAGE; i++) {
		if(sd->status.storage.items[i].nameid > 0 && sd->status.storage.items[i].amount > 0 && 
		   sd->status.storage.items[i].nameid == nameid && sd->status.storage.items[i].identify == iden &&
		   sd->status.storage.items[i].refine == ref && sd->status.storage.items[i].attribute == attr &&
		   sd->status.storage.items[i].card[0] == c0 && sd->status.storage.items[i].card[1] == c1 &&
		   sd->status.storage.items[i].card[2] == c2 && sd->status.storage.items[i].card[3] == c3
		   )
		   count += sd->status.storage.items[i].amount;

	}

	script_pushint(st,count);
	return true;
}

/* Server Startup */
HPExport void plugin_init (void) {
	addScriptCommand( "storagecountitem", "v", storagecountitem);
	addScriptCommand( "storagecountitem2", "viiiiiii", storagecountitem2);
}
