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
#include "../common/strlib.h"
#include "../common/sql.h"
#include "HPM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif

struct malloc_interface iMalloc_HPM;
struct malloc_interface *HPMiMalloc;

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

void *hplugin_import_symbol(char *name, unsigned int pID) {
	unsigned int i;
	
	for( i = 0; i < HPM->symbol_count; i++ ) {
		if( strcmp(HPM->symbols[i]->name,name) == 0 )
			return HPM->symbols[i]->ptr;
	}
	
	ShowError("HPM:get_symbol:%s: '"CL_WHITE"%s"CL_RESET"' not found!\n",HPM->pid2name(pID),name);
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
#define HPM_POP(x) { #x , x }
bool hplugin_populate(struct hplugin *plugin, const char *filename) {
	void **Link;
	struct {
		const char* name;
		void *Ref;
	} ToLink[] = {
		HPM_POP(ShowMessage),
		HPM_POP(ShowStatus),
		HPM_POP(ShowSQL),
		HPM_POP(ShowInfo),
		HPM_POP(ShowNotice),
		HPM_POP(ShowWarning),
		HPM_POP(ShowDebug),
		HPM_POP(ShowError),
		HPM_POP(ShowFatalError),
	};
	int i, length = ARRAYLENGTH(ToLink);
	
	for(i = 0; i < length; i++) {
		if( !( Link = plugin_import(plugin->dll, ToLink[i].name,void **) ) ) {
			ShowWarning("HPM:plugin_load: failed to retrieve '%s' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", ToLink[i].name, filename);
			HPM->unload(plugin);
			return false;
		}
		*Link = ToLink[i].Ref;
	}
	
	return true;
}
#undef HPM_POP
struct hplugin *hplugin_load(const char* filename) {
	struct hplugin *plugin;
	struct hplugin_info *info;
	struct HPMi_interface **HPMi;
	bool anyEvent = false;
	void **import_symbol_ref;
	Sql **sql_handle;
		
	if( HPM->exists(filename) ) {
		ShowWarning("HPM:plugin_load: attempting to load duplicate '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		return NULL;
	}
	
	plugin = HPM->create();
	
	if( !( plugin->dll = plugin_open(filename) ) ){
		ShowWarning("HPM:plugin_load: failed to load '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	
	if( !( info = plugin_import(plugin->dll, "pinfo",struct hplugin_info*) ) ) {
		ShowDebug("HPM:plugin_load: failed to retrieve 'plugin_info' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	
	if( !(info->type & SERVER_TYPE) ) {
		HPM->unload(plugin);
		return NULL;
	}
	
	if( !HPM->iscompatible(info->req_version) ) {
		ShowWarning("HPM:plugin_load: '"CL_WHITE"%s"CL_RESET"' incompatible version '%s' -> '%s', skipping...\n", filename, info->req_version, HPM_VERSION);
		HPM->unload(plugin);
		return NULL;
	}
	
	plugin->info = info;
	plugin->filename = aStrdup(filename);
	
	if( !( import_symbol_ref = plugin_import(plugin->dll, "import_symbol",void **) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'import_symbol' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	
	*import_symbol_ref = HPM->import_symbol;
	
	if( !( sql_handle = plugin_import(plugin->dll, "mysql_handle",Sql **) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'mysql_handle' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	
	*sql_handle = HPM->import_symbol("sql_handle",plugin->idx);

	if( !( HPMi = plugin_import(plugin->dll, "HPMi",struct HPMi_interface **) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'HPMi' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	
	if( !( *HPMi = plugin_import(plugin->dll, "HPMi_s",struct HPMi_interface *) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'HPMi_s' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	plugin->hpi = *HPMi;
		
	if( ( plugin->hpi->event[HPET_INIT] = plugin_import(plugin->dll, "plugin_init",void (*)(void)) ) )
		anyEvent = true;
	
	if( ( plugin->hpi->event[HPET_FINAL] = plugin_import(plugin->dll, "plugin_final",void (*)(void)) ) )
		anyEvent = true;
	
	if( ( plugin->hpi->event[HPET_READY] = plugin_import(plugin->dll, "server_online",void (*)(void)) ) )
		anyEvent = true;
	
	if( ( plugin->hpi->event[HPET_POST_FINAL] = plugin_import(plugin->dll, "server_post_final",void (*)(void)) ) )
		anyEvent = true;
	
	if( ( plugin->hpi->event[HPET_PRE_INIT] = plugin_import(plugin->dll, "server_preinit",void (*)(void)) ) )
		anyEvent = true;
	
	if( !anyEvent ) {
		ShowWarning("HPM:plugin_load: no events found for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return NULL;
	}
	
	if( !HPM->populate(plugin,filename) )
		return NULL;
	
	/* id */
	plugin->hpi->pid                = plugin->idx;
	/* core */
	plugin->hpi->addCPCommand       = HPM->import_symbol("addCPCommand",plugin->idx);
	plugin->hpi->addPacket          = HPM->import_symbol("addPacket",plugin->idx);
	plugin->hpi->addToHPData        = HPM->import_symbol("addToHPData",plugin->idx);
	plugin->hpi->getFromHPData      = HPM->import_symbol("getFromHPData",plugin->idx);
	plugin->hpi->removeFromHPData   = HPM->import_symbol("removeFromHPData",plugin->idx);
	plugin->hpi->AddHook            = HPM->import_symbol("AddHook",plugin->idx);
	plugin->hpi->HookStop           = HPM->import_symbol("HookStop",plugin->idx);
	plugin->hpi->HookStopped        = HPM->import_symbol("HookStopped",plugin->idx);
	plugin->hpi->addArg             = HPM->import_symbol("addArg",plugin->idx);
	plugin->hpi->addConf            = HPM->import_symbol("addConf",plugin->idx);
	/* server specific */
	if( HPM->load_sub )
		HPM->load_sub(plugin);
				
	return plugin;
}

void hplugin_unload(struct hplugin* plugin) {
	unsigned int i = plugin->idx, cursor = 0;
	
	if( plugin->filename )
		aFree(plugin->filename);
	if( plugin->dll )
		plugin_close(plugin->dll);
	/* TODO: for manual packet unload */
	/* - Go thru known packets and unlink any belonging to the plugin being removed */
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
	FILE *fp;
	
	/* yes its ugly, its temporary and will be gone as soon as the new inter-server.conf is set */
	if( (fp = fopen("conf/import/plugins.conf","r")) ) {
		config_filename = "conf/import/plugins.conf";
		fclose(fp);
	}
	
	if (libconfig->read_file(&plugins_conf, config_filename))
		return;

	if( HPM->symbol_defaults_sub )
		HPM->symbol_defaults_sub();
	
	plist = libconfig->lookup(&plugins_conf, "plugins_list");
	
	if (plist != NULL) {
		int length = libconfig->setting_length(plist), i;
		char filename[60];
		for(i = 0; i < length; i++) {
			if( !strcmpi(libconfig->setting_get_string_elem(plist,i),"HPMHooking") ) {//must load it first
				struct hplugin *plugin;
				snprintf(filename, 60, "plugins/%s%s", libconfig->setting_get_string_elem(plist,i), DLL_EXT);
				if( ( plugin = HPM->load(filename) )  ) {
					bool (*func)(bool *fr);
					bool (*addhook_sub) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
					if( ( func = plugin_import(plugin->dll, "Hooked",bool (*)(bool *)) ) && ( addhook_sub = plugin_import(plugin->dll, "HPM_Plugin_AddHook",bool (*)(enum HPluginHookType, const char *, void *, unsigned int)) ) ) {
						if( func(&HPM->force_return) ) {
							HPM->hooking = true;
							HPM->addhook_sub = addhook_sub;
						}
					}
				}
			}
		}
		for(i = 0; i < length; i++) {
			if( strcmpi(libconfig->setting_get_string_elem(plist,i),"HPMHooking") ) {//now all others
				snprintf(filename, 60, "plugins/%s%s", libconfig->setting_get_string_elem(plist,i), DLL_EXT);
				HPM->load(filename);
			}
		}
		libconfig->destroy(&plugins_conf);
	}
	
	if( HPM->plugin_count )
		ShowStatus("HPM: There are '"CL_WHITE"%d"CL_RESET"' plugins loaded, type '"CL_WHITE"plugins"CL_RESET"' to list them\n", HPM->plugin_count);
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
void hplugins_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr) {
	/* record address */
	switch( type ) {
		/* core-handled */
		case HPDT_SESSION:
			ret->HPDataSRCPtr = (void**)(&((struct socket_data *)ptr)->hdata);
			ret->hdatac = &((struct socket_data *)ptr)->hdatac;
			break;
		/* goes to sub */
		default:
			if( HPM->grabHPDataSub ) {
				if( HPM->grabHPDataSub(ret,type,ptr) )
					return;
				else {
					ShowError("HPM:HPM:grabHPData failed, unknown type %d!\n",type);
				}
			} else
				ShowError("HPM:grabHPData failed, type %d needs sub-handler!\n",type);
			ret->HPDataSRCPtr = NULL;
			ret->hdatac = NULL;
			return;
	}
}
void hplugins_addToHPData(enum HPluginDataTypes type, unsigned int pluginID, void *ptr, void *data, unsigned int index, bool autofree) {
	struct HPluginData *HPData, **HPDataSRC;
	struct HPDataOperationStorage action;
	unsigned int i, max;
	
	HPM->grabHPData(&action,type,ptr);

	if( action.hdatac == NULL ) { /* woo it failed! */
		ShowError("HPM:addToHPData:%s: failed, type %d (%u|%u)\n",HPM->pid2name(pluginID),type,pluginID,index);
		return;
	}
	
	/* flag */
	HPDataSRC = *(action.HPDataSRCPtr);
	max = *(action.hdatac);
	
	/* duplicate check */
	for(i = 0; i < max; i++) {
		if( HPDataSRC[i]->pluginID == pluginID && HPDataSRC[i]->type == index ) {
			ShowError("HPM:addToHPData:%s: error! attempting to insert duplicate struct of id %u and index %u\n",HPM->pid2name(pluginID),pluginID,index);
			return;
		}
	}
	
	/* HPluginData is always same size, probably better to use the ERS (with reasonable chunk size e.g. 10/25/50) */
	CREATE(HPData, struct HPluginData, 1);
	
	/* input */
	HPData->pluginID = pluginID;
	HPData->type = index;
	HPData->flag.free = autofree ? 1 : 0;
	HPData->data = data;
	
	/* resize */
	*(action.hdatac) += 1;
	RECREATE(*(action.HPDataSRCPtr),struct HPluginData *,*(action.hdatac));
	
	/* RECREATE modified the addresss */
	HPDataSRC = *(action.HPDataSRCPtr);
	HPDataSRC[*(action.hdatac) - 1] = HPData;
}

void *hplugins_getFromHPData(enum HPluginDataTypes type, unsigned int pluginID, void *ptr, unsigned int index) {
	struct HPDataOperationStorage action;
	struct HPluginData **HPDataSRC;
	unsigned int i, max;
	
	HPM->grabHPData(&action,type,ptr);
	
	if( action.hdatac == NULL ) { /* woo it failed! */
		ShowError("HPM:getFromHPData:%s: failed, type %d (%u|%u)\n",HPM->pid2name(pluginID),type,pluginID,index);
		return NULL;
	}
	
	/* flag */
	HPDataSRC = *(action.HPDataSRCPtr);
	max = *(action.hdatac);
	
	for(i = 0; i < max; i++) {
		if( HPDataSRC[i]->pluginID == pluginID && HPDataSRC[i]->type == index )
			return HPDataSRC[i]->data;
	}
		
	return NULL;
}

void hplugins_removeFromHPData(enum HPluginDataTypes type, unsigned int pluginID, void *ptr, unsigned int index) {
	struct HPDataOperationStorage action;
	struct HPluginData **HPDataSRC;
	unsigned int i, max;
	
	HPM->grabHPData(&action,type,ptr);
	
	if( action.hdatac == NULL ) { /* woo it failed! */
		ShowError("HPM:removeFromHPData:%s: failed, type %d (%u|%u)\n",HPM->pid2name(pluginID),type,pluginID,index);
		return;
	}
	
	/* flag */
	HPDataSRC = *(action.HPDataSRCPtr);
	max = *(action.hdatac);
	
	for(i = 0; i < max; i++) {
		if( HPDataSRC[i]->pluginID == pluginID && HPDataSRC[i]->type == index )
			break;
	}
	
	if( i != max ) {
		unsigned int cursor;
		
		aFree(HPDataSRC[i]->data);/* when its removed we delete it regardless of autofree */
		aFree(HPDataSRC[i]);
		HPDataSRC[i] = NULL;
		
		for(i = 0, cursor = 0; i < max; i++) {
			if( HPDataSRC[i] == NULL )
				continue;
			if( i != cursor )
				HPDataSRC[cursor] = HPDataSRC[i];
			cursor++;
		}
		*(action.hdatac) = cursor;
	}
	
}

bool hplugins_addpacket(unsigned short cmd, short length,void (*receive) (int fd),unsigned int point,unsigned int pluginID) {
	struct HPluginPacket *packet;
	unsigned int i;
	
	if( point >= hpPHP_MAX ) {
		ShowError("HPM->addPacket:%s: unknown point '%u' specified for packet 0x%04x (len %d)\n",HPM->pid2name(pluginID),point,cmd,length);
		return false;
	}

	for(i = 0; i < HPM->packetsc[point]; i++) {
		if( HPM->packets[point][i].cmd == cmd ) {
			ShowError("HPM->addPacket:%s: can't add packet 0x%04x, already in use by '%s'!",HPM->pid2name(pluginID),cmd,HPM->pid2name(HPM->packets[point][i].pluginID));
			return false;
		}
	}
	
	RECREATE(HPM->packets[point], struct HPluginPacket, ++HPM->packetsc[point]);
	packet = &HPM->packets[point][HPM->packetsc[point] - 1];
	
	packet->pluginID = pluginID;
	packet->cmd = cmd;
	packet->len = length;
	packet->receive = receive;
	
	return true;
}
/* 
 0 = unknown
 1 = OK
 2 = incomplete
 */
unsigned char hplugins_parse_packets(int fd, enum HPluginPacketHookingPoints point) {
	unsigned int i;
		
	for(i = 0; i < HPM->packetsc[point]; i++) {
		if( HPM->packets[point][i].cmd == RFIFOW(fd,0) )
			break;
	}
	
	if( i != HPM->packetsc[point] ) {
		struct HPluginPacket *packet = &HPM->packets[point][i];
		short length;
		
		if( (length = packet->len) == -1 ) {
			if( (length = RFIFOW(fd, 2)) < (int)RFIFOREST(fd) )
			   return 2;
		}
		
		packet->receive(fd);
		RFIFOSKIP(fd, length);
		return 1;
	}
	
	return 0;
}

char *hplugins_id2name (unsigned int pid) {
	unsigned int i;
	
	for( i = 0; i < HPM->plugin_count; i++ ) {
		if( HPM->plugins[i]->idx == pid )
			return HPM->plugins[i]->info->name;
	}
	
	return "UnknownPlugin";
}
char* HPM_file2ptr(const char *file) {
	unsigned int i;
	
	for(i = 0; i < HPM->fnamec; i++) {
		if( HPM->fnames[i].addr == file )
			return HPM->fnames[i].name;
	}
	
	i = HPM->fnamec;
	
	/* we handle this memory outside of the server's memory manager because we need it to exist after the memory manager goes down */
	HPM->fnames = realloc(HPM->fnames,(++HPM->fnamec)*sizeof(struct HPMFileNameCache));
		
	HPM->fnames[i].addr = file;
	HPM->fnames[i].name = strdup(file);

	return HPM->fnames[i].name;
}
void* HPM_mmalloc(size_t size, const char *file, int line, const char *func) {
	return iMalloc->malloc(size,HPM_file2ptr(file),line,func);
}
void* HPM_calloc(size_t num, size_t size, const char *file, int line, const char *func) {
	return iMalloc->calloc(num,size,HPM_file2ptr(file),line,func);
}
void* HPM_realloc(void *p, size_t size, const char *file, int line, const char *func) {
	return iMalloc->realloc(p,size,HPM_file2ptr(file),line,func);
}
void* HPM_reallocz(void *p, size_t size, const char *file, int line, const char *func) {
	return iMalloc->reallocz(p,size,HPM_file2ptr(file),line,func);
}
char* HPM_astrdup(const char *p, const char *file, int line, const char *func) {
	return iMalloc->astrdup(p,HPM_file2ptr(file),line,func);
}
/* todo: add ability for tracking using pID for the upcoming runtime load/unload support. */
bool HPM_AddHook(enum HPluginHookType type, const char *target, void *hook, unsigned int pID) {
	if( !HPM->hooking ) {
		ShowError("HPM:AddHook Fail! '%s' tried to hook to '%s' but HPMHooking is disabled!\n",HPM->pid2name(pID),target);
		return false;
	}
	/* search if target is a known hook point within 'common' */
	/* if not check if a sub-hooking list is available (from the server) and run it by */
	if( HPM->addhook_sub && HPM->addhook_sub(type,target,hook,pID) )
		return true;
	
	ShowError("HPM:AddHook: unknown Hooking Point '%s'!\n",target);
	
	return false;
}
void HPM_HookStop (const char *func, unsigned int pID) {
	/* track? */
	HPM->force_return = true;
}
bool HPM_HookStopped (void) {
	return HPM->force_return;
}
/* command-line args */
bool hpm_parse_arg(const char *arg, int *index, char *argv[], bool param) {
	struct HPMArgData *data;

	if( (data = strdb_get(HPM->arg_db,arg)) ) {
		data->func((data->has_param && param)?argv[(*index)+1]:NULL);
		if( data->has_param && param ) *index += 1;
		return true;
	}
	
	return false;
}
void hpm_arg_help(void) {
	DBIterator *iter = db_iterator(HPM->arg_db);
	struct HPMArgData *data = NULL;
	
	for( data = dbi_first(iter); dbi_exists(iter); data = dbi_next(iter) ) {
		if( data->help != NULL )
			data->help();
		else
			ShowInfo("  %s (%s)\t\t<no description provided>\n",data->name,HPM->pid2name(data->pluginID));
	}
	
	dbi_destroy(iter);
}
bool hpm_add_arg(unsigned int pluginID, char *name, bool has_param, void (*func) (char *param),void (*help) (void)) {
	struct HPMArgData *data = NULL;
		
	if( strdb_exists(HPM->arg_db, name) ) {
		ShowError("HPM:add_arg:%s duplicate! (from %s)\n",name,HPM->pid2name(pluginID));
		return false;
	}
	
	CREATE(data, struct HPMArgData, 1);
	
	data->pluginID = pluginID;	
	data->name = aStrdup(name);
	data->func = func;
	data->help = help;
	data->has_param = has_param;
	
	strdb_put(HPM->arg_db, data->name, data);
	
	return true;
}
bool hplugins_addconf(unsigned int pluginID, enum HPluginConfType type, char *name, void (*func) (const char *val)) {
	struct HPConfListenStorage *conf;
	unsigned int i;
	
	if( type >= HPCT_MAX ) {
		ShowError("HPM->addConf:%s: unknown point '%u' specified for config '%s'\n",HPM->pid2name(pluginID),type,name);
		return false;
	}
	
	for(i = 0; i < HPM->confsc[type]; i++) {
		if( !strcmpi(name,HPM->confs[type][i].key) ) {
			ShowError("HPM->addConf:%s: duplicate '%s', already in use by '%s'!",HPM->pid2name(pluginID),name,HPM->pid2name(HPM->confs[type][i].pluginID));
			return false;
		}
	}
	
	RECREATE(HPM->confs[type], struct HPConfListenStorage, ++HPM->confsc[type]);
	conf = &HPM->confs[type][HPM->confsc[type] - 1];
	
	conf->pluginID = pluginID;
	safestrncpy(conf->key, name, HPM_ADDCONF_LENGTH);
	conf->func = func;
	
	return true;
}
bool hplugins_parse_conf(const char *w1, const char *w2, enum HPluginConfType point) {
	unsigned int i;
	
	/* exists? */
	for(i = 0; i < HPM->confsc[point]; i++) {
		if( !strcmpi(w1,HPM->confs[point][i].key) )
			break;
	}
	
	/* trigger and we're set! */
	if( i != HPM->confsc[point] ) {
		HPM->confs[point][i].func(w2);
		return true;
	}
	
	return false;
}

void hplugins_share_defaults(void) {
	/* console */
#ifdef CONSOLE_INPUT
	HPM->share(console->addCommand,"addCPCommand");
#endif
	/* our own */
	HPM->share(hplugins_addpacket,"addPacket");
	HPM->share(hplugins_addToHPData,"addToHPData");
	HPM->share(hplugins_getFromHPData,"getFromHPData");
	HPM->share(hplugins_removeFromHPData,"removeFromHPData");
	HPM->share(HPM_AddHook,"AddHook");
	HPM->share(HPM_HookStop,"HookStop");
	HPM->share(HPM_HookStopped,"HookStopped");
	HPM->share(hpm_add_arg,"addArg");
	HPM->share(hplugins_addconf,"addConf");
	/* core */
	HPM->share(&runflag,"runflag");
	HPM->share(arg_v,"arg_v");
	HPM->share(&arg_c,"arg_c");
	HPM->share(SERVER_NAME,"SERVER_NAME");
	HPM->share(&SERVER_TYPE,"SERVER_TYPE");
	HPM->share((void*)get_svn_revision,"get_svn_revision");
	HPM->share((void*)get_git_hash,"get_git_hash");
	HPM->share(DB, "DB");
	HPM->share(HPMiMalloc, "iMalloc");
	/* socket */
	HPM->share(sockt,"sockt");
	/* strlib */
	HPM->share(strlib,"strlib");
	HPM->share(sv,"sv");
	HPM->share(StrBuf,"StrBuf");
	/* sql */
	HPM->share(SQL,"SQL");
	/* timer */
	HPM->share(timer,"timer");
	/* libconfig */
	HPM->share(libconfig,"libconfig");
}

void hpm_init(void) {
	unsigned int i;
	
	HPM->symbols = NULL;
	HPM->plugins = NULL;
	HPM->plugin_count = HPM->symbol_count = 0;
	HPM->off = false;
	
	memcpy(&iMalloc_HPM, iMalloc, sizeof(struct malloc_interface));
	HPMiMalloc = &iMalloc_HPM;
	HPMiMalloc->malloc = HPM_mmalloc;
	HPMiMalloc->calloc = HPM_calloc;
	HPMiMalloc->realloc = HPM_realloc;
	HPMiMalloc->reallocz = HPM_reallocz;
	HPMiMalloc->astrdup = HPM_astrdup;

	sscanf(HPM_VERSION, "%u.%u", &HPM->version[0], &HPM->version[1]);
	
	if( HPM->version[0] == 0 && HPM->version[1] == 0 ) {
		ShowError("HPM:init:failed to retrieve HPM version!!\n");
		return;
	}
	
	for(i = 0; i < hpPHP_MAX; i++) {
		HPM->packets[i] = NULL;
		HPM->packetsc[i] = 0;
	}
	
	HPM->arg_db = strdb_alloc(DB_OPT_RELEASE_DATA, 0);
	
	HPM->symbol_defaults();
	
#ifdef CONSOLE_INPUT
	console->addCommand("plugins",CPCMD_A(plugins));
#endif
	return;
}
void hpm_memdown(void) {
	unsigned int i;
	
	/* this memory is handled outside of the server's memory manager and thus cleared after memory manager goes down */
	
	for( i = 0; i < HPM->fnamec; i++ ) {
		free(HPM->fnames[i].name);
	}
	
	if( HPM->fnames )
		free(HPM->fnames);
	
}
int hpm_arg_db_clear_sub(DBKey key, DBData *data, va_list args) {
	struct HPMArgData *a = DB->data2ptr(data);
	
	aFree(a->name);
	
	return 0;
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
	
	for( i = 0; i < hpPHP_MAX; i++ ) {
		if( HPM->packets[i] )
			aFree(HPM->packets[i]);
	}
	
	for( i = 0; i < HPCT_MAX; i++ ) {
		if( HPM->confsc[i] )
			aFree(HPM->confs[i]);
	}
	
	HPM->arg_db->destroy(HPM->arg_db,HPM->arg_db_clear_sub);
	
	/* HPM->fnames is cleared after the memory manager goes down */
	iMalloc->post_shutdown = hpm_memdown;
	
	return;
}
void hpm_defaults(void) {
	unsigned int i;
	HPM = &HPM_s;
	
	HPM->fnames = NULL;
	HPM->fnamec = 0;
	HPM->force_return = false;
	HPM->hooking = false;
	/* */
	HPM->fnames = NULL;
	HPM->fnamec = 0;
	for(i = 0; i < hpPHP_MAX; i++) {
		HPM->packets[i] = NULL;
		HPM->packetsc[i] = 0;
	}
	for(i = 0; i < HPCT_MAX; i++) {
		HPM->confs[i] = NULL;
		HPM->confsc[i] = 0;
	}
	HPM->arg_db = NULL;
	/* */
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
	HPM->populate = hplugin_populate;
	HPM->symbol_defaults_sub = NULL;
	HPM->pid2name = hplugins_id2name;
	HPM->parse_packets = hplugins_parse_packets;
	HPM->load_sub = NULL;
	HPM->addhook_sub = NULL;
	HPM->arg_db_clear_sub = hpm_arg_db_clear_sub;
	HPM->parse_arg = hpm_parse_arg;
	HPM->arg_help = hpm_arg_help;
	HPM->grabHPData = hplugins_grabHPData;
	HPM->grabHPDataSub = NULL;
	HPM->parseConf = hplugins_parse_conf;
}
