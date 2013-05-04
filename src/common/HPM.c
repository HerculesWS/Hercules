// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/conf.h"
#include "../common/utils.h"
#include "../common/console.h"
#include "HPM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif

void hplugin_trigger_event(enum hp_event_types type) {
	unsigned int i;
	for( i = 0; i < HPM->plugin_count; i++ ) {
		if( HPM->plugins[i]->hpi->event[type] != NULL )
			(HPM->plugins[i]->hpi->event[type])();
	}
}

void hplugin_export_symbol(void *var, char *name) {
	RECREATE(HPM->symbols, struct hpm_symbol *, ++HPM->symbol_count);
	CREATE(HPM->symbols[HPM->symbol_count - 1] ,struct hpm_symbol, 1);
	HPM->symbols[HPM->symbol_count - 1]->name = name;
	HPM->symbols[HPM->symbol_count - 1]->ptr = var;
}

void *hplugin_import_symbol(char *name) {
	unsigned int i;
	
	for( i = 0; i < HPM->symbol_count; i++ ) {
		if( strcmp(HPM->symbols[i]->name,name) == 0 )
			return HPM->symbols[i]->ptr;
	}
	
	ShowError("HPM:get_symbol: '"CL_WHITE"%s"CL_RESET"' not found!\n",name);
	return NULL;
}

bool hplugin_iscompatible(char* version) {
	unsigned int req_major = 0, req_minor = 0;
	
	if( version == NULL )
		return false;
	
	sscanf(version, "%u.%u", &req_major, &req_minor);
	
	return ( req_major == HPM->version[0] && req_minor <= HPM->version[1] ) ? true : false;
}

bool hplugin_exists(const char *filename) {
	unsigned int i;
	for(i = 0; i < HPM->plugin_count; i++) {
		if( strcmpi(HPM->plugins[i]->filename,filename) == 0 )
			return true;
	}
	return false;
}
struct hplugin *hplugin_create(void) {
	RECREATE(HPM->plugins, struct hplugin *, ++HPM->plugin_count);
	CREATE(HPM->plugins[HPM->plugin_count - 1], struct hplugin, 1);
	HPM->plugins[HPM->plugin_count - 1]->idx = HPM->plugin_count - 1;
	HPM->plugins[HPM->plugin_count - 1]->filename = NULL;
	return HPM->plugins[HPM->plugin_count - 1];
}
bool hplugin_showmsg_populate(struct hplugin *plugin, const char *filename) {
	void **ShowSub;
	const char* ShowSubNames[9] = {
		"ShowMessage",
		"ShowStatus",
		"ShowSQL",
		"ShowInfo",
		"ShowNotice",
		"ShowWarning",
		"ShowDebug",
		"ShowError",
		"ShowFatalError",
	};
	void* ShowSubRef[9] = {
		ShowMessage,
		ShowStatus,
		ShowSQL,
		ShowInfo,
		ShowNotice,
		ShowWarning,
		ShowDebug,
		ShowError,
		ShowFatalError,
	};
	int i;
	for(i = 0; i < 9; i++) {
		if( !( ShowSub = plugin_import(plugin->dll, ShowSubNames[i],void **) ) ) {
			ShowWarning("HPM:plugin_load: failed to retrieve '%s' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", ShowSubNames[i], filename);
			HPM->unload(plugin);
			return false;
		}
		*ShowSub = ShowSubRef[i];
	}
	return true;
}
void hplugin_load(const char* filename) {
	struct hplugin *plugin;
	struct hplugin_info *info;
	struct HPMi_interface **HPMi;
	bool anyEvent = false;
	void **import_symbol_ref;
		
	if( HPM->exists(filename) ) {
		ShowWarning("HPM:plugin_load: attempting to load duplicate '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		return;
	}
	
	plugin = HPM->create();
	
	if( !( plugin->dll = plugin_open(filename) ) ){
		ShowWarning("HPM:plugin_load: failed to load '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	
	if( !( info = plugin_import(plugin->dll, "pinfo",struct hplugin_info*) ) ) {
		ShowDebug("HPM:plugin_load: failed to retrieve 'plugin_info' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	
	if( !(info->type & SERVER_TYPE) ) {
		HPM->unload(plugin);
		return;
	}
	
	if( !HPM->iscompatible(info->req_version) ) {
		ShowWarning("HPM:plugin_load: '"CL_WHITE"%s"CL_RESET"' incompatible version '%s' -> '%s', skipping...\n", filename, info->req_version, HPM_VERSION);
		HPM->unload(plugin);
		return;
	}
		
	if( !( import_symbol_ref = plugin_import(plugin->dll, "import_symbol",void **) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'import_symbol' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	
	*import_symbol_ref = HPM->import_symbol;
	
	if( !( HPMi = plugin_import(plugin->dll, "HPMi",struct HPMi_interface **) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'HPMi' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	
	if( !( *HPMi = plugin_import(plugin->dll, "HPMi_s",struct HPMi_interface *) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'HPMi_s' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	plugin->hpi = *HPMi;
		
	if( ( plugin->hpi->event[HPET_INIT] = plugin_import(plugin->dll, "plugin_init",void (*)(void)) ) )
		anyEvent = true;
	
	if( ( plugin->hpi->event[HPET_FINAL] = plugin_import(plugin->dll, "plugin_final",void (*)(void)) ) )
		anyEvent = true;
	
	if( ( plugin->hpi->event[HPET_READY] = plugin_import(plugin->dll, "server_online",void (*)(void)) ) )
		anyEvent = true;
	
	if( !anyEvent ) {
		ShowWarning("HPM:plugin_load: no events found for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	
	if( !HPM->showmsg_pop(plugin,filename) )
		return;
	
	if( SERVER_TYPE == SERVER_TYPE_MAP ) {
		plugin->hpi->addCommand		= HPM->import_symbol("addCommand");
		plugin->hpi->addScript		= HPM->import_symbol("addScript");
		plugin->hpi->addCPCommand	= HPM->import_symbol("addCPCommand");
	}
	
	plugin->info = info;
	plugin->filename = aStrdup(filename);
			
	return;
}

void hplugin_unload(struct hplugin* plugin) {
	unsigned int i = plugin->idx, cursor = 0;
	
	if( plugin->filename )
		aFree(plugin->filename);
	if( plugin->dll )
		plugin_close(plugin->dll);
	
	aFree(plugin);
	if( !HPM->off ) {
		HPM->plugins[i] = NULL;
		for(i = 0; i < HPM->plugin_count; i++) {
			if( HPM->plugins[i] == NULL )
				continue;
			if( cursor != i )
				HPM->plugins[cursor] = HPM->plugins[i];
			cursor++;
		}
		if( !(HPM->plugin_count = cursor) ) {
			aFree(HPM->plugins);
			HPM->plugins = NULL;
		}
	}
}

void hplugins_config_read(void) {
	config_t plugins_conf;
	config_setting_t *plist = NULL;
	const char *config_filename = "conf/plugins.conf"; // FIXME hardcoded name
	
	if (conf_read_file(&plugins_conf, config_filename))
		return;

	if( HPM->symbol_defaults_sub )
		HPM->symbol_defaults_sub();
	
	plist = config_lookup(&plugins_conf, "plugins_list");
	
	if (plist != NULL) {
		int length = config_setting_length(plist), i;
		char filename[60];
		for(i = 0; i < length; i++) {
			snprintf(filename, 60, "plugins/%s%s", config_setting_get_string_elem(plist,i), DLL_EXT);
			HPM->load(filename);
		}
		config_destroy(&plugins_conf);
	}
	
	if( HPM->plugin_count )
		ShowStatus("HPM: There are '"CL_WHITE"%d"CL_RESET"' plugins loaded, type '"CL_WHITE"plugins"CL_RESET"' to list them\n", HPM->plugin_count);
}
void hplugins_share_defaults(void) {
	/* console */
	HPM->share(console->addCommand,"addCPCommand");
	/* core */
	HPM->share(&runflag,"runflag");
	HPM->share(arg_v,"arg_v");
	HPM->share(&arg_c,"arg_c");
	HPM->share(SERVER_NAME,"SERVER_NAME");
	HPM->share(&SERVER_TYPE,"SERVER_TYPE");
	HPM->share((void*)get_svn_revision,"get_svn_revision");
	HPM->share((void*)get_git_hash,"get_git_hash");
	/* socket */
	HPM->share(RFIFOSKIP,"RFIFOSKIP");
	HPM->share(WFIFOSET,"WFIFOSET");
	HPM->share(do_close,"do_close");
	HPM->share(make_connection,"make_connection");
	HPM->share(session,"session");
	HPM->share(&fd_max,"fd_max");
	HPM->share(addr_,"addr");
	/* timer */
	HPM->share(gettick,"gettick");
	HPM->share(add_timer,"add_timer");
	HPM->share(add_timer_interval,"add_timer_interval");
	HPM->share(add_timer_func_list,"add_timer_func_list");
	HPM->share(delete_timer,"delete_timer");
	HPM->share(get_uptime,"get_uptime");
}
CPCMD(plugins) {
	if( HPM->plugin_count == 0 ) {
		ShowInfo("HPC: there are no plugins loaded\n");
	} else {
		unsigned int i;
		
		ShowInfo("HPC: There are '"CL_WHITE"%d"CL_RESET"' plugins loaded\n",HPM->plugin_count);
		
		for(i = 0; i < HPM->plugin_count; i++) {
			ShowInfo("HPC: - '"CL_WHITE"%s"CL_RESET"' (%s)\n",HPM->plugins[i]->info->name,HPM->plugins[i]->filename);
		}
	}
}
void hpm_init(void) {
	HPM->symbols = NULL;
	HPM->plugins = NULL;
	HPM->plugin_count = HPM->symbol_count = 0;
	HPM->off = false;
	
	sscanf(HPM_VERSION, "%d.%d", &HPM->version[0], &HPM->version[1]);
	
	if( HPM->version[0] == 0 && HPM->version[1] == 0 ) {
		ShowError("HPM:init:failed to retrieve HPM version!!\n");
		return;
	}
	HPM->symbol_defaults();
	
	console->addCommand("plugins",CPCMD_A(plugins));
			
	return;
}

void hpm_final(void) {
	unsigned int i;
	
	HPM->off = true;
	
	for( i = 0; i < HPM->plugin_count; i++ ) {
		HPM->unload(HPM->plugins[i]);
	}
	
	if( HPM->plugins )
		aFree(HPM->plugins);
	
	for( i = 0; i < HPM->symbol_count; i++ ) {
		aFree(HPM->symbols[i]);
	}
	
	if( HPM->symbols )
		aFree(HPM->symbols);
			
	return;
}
void hpm_defaults(void) {
	HPM = &HPM_s;
	
	HPM->init = hpm_init;
	HPM->final = hpm_final;
	
	HPM->create = hplugin_create;
	HPM->load = hplugin_load;
	HPM->unload = hplugin_unload;
	HPM->event = hplugin_trigger_event;
	HPM->exists = hplugin_exists;
	HPM->iscompatible = hplugin_iscompatible;
	HPM->import_symbol = hplugin_import_symbol;
	HPM->share = hplugin_export_symbol;
	HPM->symbol_defaults = hplugins_share_defaults;
	HPM->config_read = hplugins_config_read;
	HPM->showmsg_pop = hplugin_showmsg_populate;
	HPM->symbol_defaults_sub = NULL;
}
