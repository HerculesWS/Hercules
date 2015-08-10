// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#define HERCULES_CORE

#include "config/core.h" // CONSOLE_INPUT
#include "HPM.h"

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/malloc.h"
#include "common/mmo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"
#include "common/nullpo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#	include <unistd.h>
#endif

struct malloc_interface iMalloc_HPM;
struct malloc_interface *HPMiMalloc;
struct HPM_interface HPM_s;

/**
 * (char*) data name -> (unsigned int) HPMDataCheck[] index
 **/
DBMap *datacheck_db;
int datacheck_version;
const struct s_HPMDataCheck *datacheck_data;

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
		void **Link;
		if (!( Link = plugin_import(plugin->dll, ToLink[i].name,void **))) {
			ShowFatalError("HPM:plugin_load: failed to retrieve '%s' for '"CL_WHITE"%s"CL_RESET"'!\n", ToLink[i].name, filename);
			exit(EXIT_FAILURE);
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
	int *HPMDataCheckVer;
	unsigned int *HPMDataCheckLen;
	struct s_HPMDataCheck *HPMDataCheck;

	if( HPM->exists(filename) ) {
		ShowWarning("HPM:plugin_load: attempting to load duplicate '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		return NULL;
	}

	plugin = HPM->create();

	if (!(plugin->dll = plugin_open(filename))) {
		char buf[1024];
		ShowFatalError("HPM:plugin_load: failed to load '"CL_WHITE"%s"CL_RESET"' (error: %s)!\n", filename, plugin_geterror(buf));
		exit(EXIT_FAILURE);
	}

	if( !( info = plugin_import(plugin->dll, "pinfo",struct hplugin_info*) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'plugin_info' for '"CL_WHITE"%s"CL_RESET"'!\n", filename);
		exit(EXIT_FAILURE);
	}

	if( !(info->type & SERVER_TYPE) ) {
		HPM->unload(plugin);
		return NULL;
	}

	if( !HPM->iscompatible(info->req_version) ) {
		ShowFatalError("HPM:plugin_load: '"CL_WHITE"%s"CL_RESET"' incompatible version '%s' -> '%s'!\n", filename, info->req_version, HPM_VERSION);
		exit(EXIT_FAILURE);
	}

	plugin->info = info;
	plugin->filename = aStrdup(filename);

	if( !( import_symbol_ref = plugin_import(plugin->dll, "import_symbol",void **) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'import_symbol' for '"CL_WHITE"%s"CL_RESET"'!\n", filename);
		exit(EXIT_FAILURE);
	}

	*import_symbol_ref = HPM->import_symbol;

	if( !( sql_handle = plugin_import(plugin->dll, "mysql_handle",Sql **) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'mysql_handle' for '"CL_WHITE"%s"CL_RESET"'!\n", filename);
		exit(EXIT_FAILURE);
	}

	*sql_handle = HPM->import_symbol("sql_handle",plugin->idx);

	if( !( HPMi = plugin_import(plugin->dll, "HPMi",struct HPMi_interface **) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'HPMi' for '"CL_WHITE"%s"CL_RESET"'!\n", filename);
		exit(EXIT_FAILURE);
	}

	if( !( *HPMi = plugin_import(plugin->dll, "HPMi_s",struct HPMi_interface *) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'HPMi_s' for '"CL_WHITE"%s"CL_RESET"'!\n", filename);
		exit(EXIT_FAILURE);
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
		ShowWarning("HPM:plugin_load: no events found for '"CL_WHITE"%s"CL_RESET"'!\n", filename);
		exit(EXIT_FAILURE);
	}

	if( !HPM->populate(plugin,filename) )
		return NULL;

	if( !( HPMDataCheckLen = plugin_import(plugin->dll, "HPMDataCheckLen", unsigned int *) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'HPMDataCheckLen' for '"CL_WHITE"%s"CL_RESET"', most likely not including HPMDataCheck.h!\n", filename);
		exit(EXIT_FAILURE);
	}

	if( !( HPMDataCheckVer = plugin_import(plugin->dll, "HPMDataCheckVer", int *) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'HPMDataCheckVer' for '"CL_WHITE"%s"CL_RESET"', most likely an outdated plugin!\n", filename);
		exit(EXIT_FAILURE);
	}

	if( !( HPMDataCheck = plugin_import(plugin->dll, "HPMDataCheck", struct s_HPMDataCheck *) ) ) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'HPMDataCheck' for '"CL_WHITE"%s"CL_RESET"', most likely not including HPMDataCheck.h!\n", filename);
		exit(EXIT_FAILURE);
	}

	// TODO: Remove the HPM->DataCheck != NULL check once login and char support is complete
	if (HPM->DataCheck != NULL && !HPM->DataCheck(HPMDataCheck,*HPMDataCheckLen,*HPMDataCheckVer,plugin->info->name)) {
		ShowFatalError("HPM:plugin_load: '"CL_WHITE"%s"CL_RESET"' failed DataCheck, out of sync from the core (recompile plugin)!\n", filename);
		exit(EXIT_FAILURE);
	}

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

	ShowStatus("HPM: Loaded plugin '"CL_WHITE"%s"CL_RESET"' (%s).\n", plugin->info->name, plugin->info->version);

	return plugin;
}

void hplugin_unload(struct hplugin* plugin) {
	unsigned int i = plugin->idx;

	if( plugin->filename )
		aFree(plugin->filename);
	if( plugin->dll )
		plugin_close(plugin->dll);
	/* TODO: for manual packet unload */
	/* - Go through known packets and unlink any belonging to the plugin being removed */
	aFree(plugin);
	if (!HPM->off) {
		int cursor = 0;
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

/**
 * Adds a plugin requested from the command line to the auto-load list.
 */
CMDLINEARG(loadplugin)
{
	RECREATE(HPM->cmdline_plugins, char *, ++HPM->cmdline_plugins_count);
	HPM->cmdline_plugins[HPM->cmdline_plugins_count-1] = aStrdup(params);
	return true;
}

/**
 * Reads the plugin configuration and loads the plugins as necessary.
 */
void hplugins_config_read(void) {
	config_t plugins_conf;
	config_setting_t *plist = NULL;
	const char *config_filename = "conf/plugins.conf"; // FIXME hardcoded name
	FILE *fp;
	int i;

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
	for (i = 0; i < HPM->cmdline_plugins_count; i++) {
		config_setting_t *entry = libconfig->setting_add(plist, NULL, CONFIG_TYPE_STRING);
		config_setting_set_string(entry, HPM->cmdline_plugins[i]);
	}

	if (plist != NULL) {
		int length = libconfig->setting_length(plist);
		char filename[60];
		char hooking_plugin_name[32];
		const char *plugin_name_suffix = "";
		if (SERVER_TYPE == SERVER_TYPE_LOGIN)
			plugin_name_suffix = "_login";
		else if (SERVER_TYPE == SERVER_TYPE_CHAR)
			plugin_name_suffix = "_char";
		else if (SERVER_TYPE == SERVER_TYPE_MAP)
			plugin_name_suffix = "_map";
		snprintf(hooking_plugin_name, sizeof(hooking_plugin_name), "HPMHooking%s", plugin_name_suffix);

		for (i = 0; i < length; i++) {
			const char *plugin_name = libconfig->setting_get_string_elem(plist,i);
			if (strcmpi(plugin_name, "HPMHooking") == 0 || strcmpi(plugin_name, hooking_plugin_name) == 0) { //must load it first
				struct hplugin *plugin;
				snprintf(filename, 60, "plugins/%s%s", hooking_plugin_name, DLL_EXT);
				if ((plugin = HPM->load(filename))) {
					const char * (*func)(bool *fr);
					bool (*addhook_sub) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
					if ((func = plugin_import(plugin->dll, "Hooked",const char * (*)(bool *)))
					 && (addhook_sub = plugin_import(plugin->dll, "HPM_Plugin_AddHook",bool (*)(enum HPluginHookType, const char *, void *, unsigned int)))) {
						const char *failed = func(&HPM->force_return);
						if (failed) {
							ShowError("HPM: failed to retrieve '%s' for '"CL_WHITE"%s"CL_RESET"'!\n", failed, plugin_name);
						} else {
							HPM->hooking = true;
							HPM->addhook_sub = addhook_sub;
						}
					}
				}
				break;
			}
		}
		for (i = 0; i < length; i++) {
			if (strncmpi(libconfig->setting_get_string_elem(plist,i),"HPMHooking", 10) == 0) // Already loaded, skip
				continue;
			snprintf(filename, 60, "plugins/%s%s", libconfig->setting_get_string_elem(plist,i), DLL_EXT);
			HPM->load(filename);
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

	/* RECREATE modified the address */
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
			if( (length = RFIFOW(fd, 2)) > (int)RFIFOREST(fd) )
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

	if (pid == HPM_PID_CORE)
		return "core";

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
/* TODO: add ability for tracking using pID for the upcoming runtime load/unload support. */
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
/**
 * Adds a plugin-defined command-line argument.
 *
 * @param pluginID  the current plugin's ID.
 * @param name      the command line argument's name, including the leading '--'.
 * @param has_param whether the command line argument expects to be followed by a value.
 * @param func      the triggered function.
 * @param help      the help string to be displayed by '--help', if any.
 * @return the success status.
 */
bool hpm_add_arg(unsigned int pluginID, char *name, bool has_param, CmdlineExecFunc func, const char *help) {
	int i;

	if (!name || strlen(name) < 3 || name[0] != '-' || name[1] != '-') {
		ShowError("HPM:add_arg:%s invalid argument name: arguments must begin with '--' (from %s)\n", name, HPM->pid2name(pluginID));
		return false;
	}

	ARR_FIND(0, cmdline->args_data_count, i, strcmp(cmdline->args_data[i].name, name) == 0);

       if (i < cmdline->args_data_count) {
               ShowError("HPM:add_arg:%s duplicate! (from %s)\n",name,HPM->pid2name(pluginID));
               return false;
       }

       return cmdline->arg_add(pluginID, name, '\0', func, help, has_param ? CMDLINE_OPT_PARAM : CMDLINE_OPT_NORMAL);
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

/**
 * Called by HPM->DataCheck on a plugins incoming data, ensures data structs in use are matching!
 **/
bool HPM_DataCheck(struct s_HPMDataCheck *src, unsigned int size, int version, char *name) {
	unsigned int i, j;

	if (version != datacheck_version) {
		ShowError("HPMDataCheck:%s: DataCheck API version mismatch %d != %d\n", name, datacheck_version, version);
		return false;
	}

	for (i = 0; i < size; i++) {
		if (!(src[i].type|SERVER_TYPE))
			continue;

		if (!strdb_exists(datacheck_db, src[i].name)) {
			ShowError("HPMDataCheck:%s: '%s' was not found\n",name,src[i].name);
			return false;
		} else {
			j = strdb_uiget(datacheck_db, src[i].name);/* not double lookup; exists sets cache to found data */
			if (src[i].size != datacheck_data[j].size) {
				ShowWarning("HPMDataCheck:%s: '%s' size mismatch %u != %u\n",name,src[i].name,src[i].size,datacheck_data[j].size);
				return false;
			}
		}
	}

	return true;
}

void HPM_datacheck_init(const struct s_HPMDataCheck *src, unsigned int length, int version) {
	unsigned int i;

	datacheck_version = version;
	datacheck_data = src;

	/**
	 * Populates datacheck_db for easy lookup later on
	 **/
	datacheck_db = strdb_alloc(DB_OPT_BASE,0);

	for(i = 0; i < length; i++) {
		strdb_uiput(datacheck_db, src[i].name, i);
	}
}

void HPM_datacheck_final(void) {
	db_destroy(datacheck_db);
}

void hplugins_share_defaults(void) {
	/* console */
#ifdef CONSOLE_INPUT
	HPM->share(console->input->addCommand,"addCPCommand");
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
	HPM->share(DB, "DB");
	HPM->share(HPMiMalloc, "iMalloc");
	HPM->share(nullpo,"nullpo");
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
	/* sysinfo */
	HPM->share(sysinfo,"sysinfo");
}

void hpm_init(void) {
	unsigned int i;
	datacheck_db = NULL;
	datacheck_data = NULL;
	datacheck_version = 0;

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

	HPM->symbol_defaults();

#ifdef CONSOLE_INPUT
	console->input->addCommand("plugins",CPCMD_A(plugins));
#endif
	return;
}
void hpm_memdown(void)
{
	/* this memory is handled outside of the server's memory manager and thus cleared after memory manager goes down */

	if (HPM->fnames) {
		unsigned int i;
		for (i = 0; i < HPM->fnamec; i++) {
			free(HPM->fnames[i].name);
		}
		free(HPM->fnames);
	}
}
void hpm_final(void) {
	unsigned int i;

	HPM->off = true;

	if( HPM->plugins )
	{
		for( i = 0; i < HPM->plugin_count; i++ ) {
			HPM->unload(HPM->plugins[i]);
		}
		aFree(HPM->plugins);
	}

	if( HPM->symbols )
	{
		for( i = 0; i < HPM->symbol_count; i++ ) {
			aFree(HPM->symbols[i]);
		}
		aFree(HPM->symbols);
	}

	for( i = 0; i < hpPHP_MAX; i++ ) {
		if( HPM->packets[i] )
			aFree(HPM->packets[i]);
	}

	for( i = 0; i < HPCT_MAX; i++ ) {
		if( HPM->confsc[i] )
			aFree(HPM->confs[i]);
	}
	if (HPM->cmdline_plugins) {
		int j;
		for (j = 0; j < HPM->cmdline_plugins_count; j++)
			aFree(HPM->cmdline_plugins[j]);
		aFree(HPM->cmdline_plugins);
		HPM->cmdline_plugins = NULL;
		HPM->cmdline_plugins_count = 0;
	}

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
	HPM->cmdline_plugins = NULL;
	HPM->cmdline_plugins_count = 0;
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
	HPM->grabHPData = hplugins_grabHPData;
	HPM->grabHPDataSub = NULL;
	HPM->parseConf = hplugins_parse_conf;
	HPM->DataCheck = HPM_DataCheck;
	HPM->datacheck_init = HPM_datacheck_init;
	HPM->datacheck_final = HPM_datacheck_final;
}
