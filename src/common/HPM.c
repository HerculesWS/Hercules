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
void hplugin_load(const char* filename) {
	struct hplugin *plugin;
	struct hplugin_info *info;
	struct HPMi_interface **HPMi;
	bool anyEvent = false;
	void **import_symbol_ref;
	Sql **sql_handle;
		
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
	
	if( !( sql_handle = plugin_import(plugin->dll, "mysql_handle",Sql **) ) ) {
		ShowWarning("HPM:plugin_load: failed to retrieve 'mysql_handle' for '"CL_WHITE"%s"CL_RESET"', skipping...\n", filename);
		HPM->unload(plugin);
		return;
	}
	
	*sql_handle = HPM->import_symbol("sql_handle");

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
	
	if( !HPM->populate(plugin,filename) )
		return;
	/* id */
	plugin->hpi->pid				= plugin->idx;
	/* core */
	plugin->hpi->addCPCommand		= HPM->import_symbol("addCPCommand");
	plugin->hpi->addPacket			= HPM->import_symbol("addPacket");
	plugin->hpi->addToSession		= HPM->import_symbol("addToSession");
	plugin->hpi->getFromSession		= HPM->import_symbol("getFromSession");
	plugin->hpi->removeFromSession	= HPM->import_symbol("removeFromSession");
	/* server specific */
	if( HPM->load_sub )
		HPM->load_sub(plugin);
	
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

void hplugins_addToSession(struct socket_data *sess, void *data, unsigned int id, unsigned int type, bool autofree) {
	struct HPluginData *HPData;
	unsigned int i;
	
	for(i = 0; i < sess->hdatac; i++) {
		if( sess->hdata[i]->pluginID == id && sess->hdata[i]->type == type ) {
			ShowError("HPM->addToSession:%s: error! attempting to insert duplicate struct of id %u and type %u\n",HPM->pid2name(id),id,type);
			return;
		}
	}
	
	//HPluginData is always same size, probably better to use the ERS
	CREATE(HPData, struct HPluginData, 1);
	
	HPData->pluginID = id;
	HPData->type = type;
	HPData->flag.free = autofree ? 1 : 0;
	HPData->data = data;
	
	RECREATE(sess->hdata,struct HPluginData *,++sess->hdatac);
	sess->hdata[sess->hdatac - 1] = HPData;
}
void *hplugins_getFromSession(struct socket_data *sess, unsigned int id, unsigned int type) {
	unsigned int i;
	
	for(i = 0; i < sess->hdatac; i++) {
		if( sess->hdata[i]->pluginID == id && sess->hdata[i]->type == type ) {
			break;
		}
	}
	
	if( i != sess->hdatac )
		return sess->hdata[i]->data;
	
	return NULL;
}
void hplugins_removeFromSession(struct socket_data *sess, unsigned int id, unsigned int type) {
	unsigned int i;
	
	for(i = 0; i < sess->hdatac; i++) {
		if( sess->hdata[i]->pluginID == id && sess->hdata[i]->type == type ) {
			break;
		}
	}
	
	if( i != sess->hdatac ) {
		unsigned int cursor;

		aFree(sess->hdata[i]->data);
		aFree(sess->hdata[i]);
		sess->hdata[i] = NULL;
		
		for(i = 0, cursor = 0; i < sess->hdatac; i++) {
			if( sess->hdata[i] == NULL )
				continue;
			if( i != cursor )
				sess->hdata[cursor] = sess->hdata[i];
			cursor++;
		}
		sess->hdatac = cursor;
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
char* HPM_astrdup(const char *p, const char *file, int line, const char *func) {
	return iMalloc->astrdup(p,HPM_file2ptr(file),line,func);
}

void hplugins_share_defaults(void) {
	/* console */
#ifdef CONSOLE_INPUT
	HPM->share(console->addCommand,"addCPCommand");
#endif
	/* our own */
	HPM->share(hplugins_addpacket,"addPacket");
	HPM->share(hplugins_addToSession,"addToSession");
	HPM->share(hplugins_getFromSession,"getFromSession");
	HPM->share(hplugins_removeFromSession,"removeFromSession");
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
	HPM->share(RFIFOSKIP,"RFIFOSKIP");
	HPM->share(WFIFOSET,"WFIFOSET");
	HPM->share(do_close,"do_close");
	HPM->share(make_connection,"make_connection");
	//session,fd_max and addr_ are shared from within socket.c
	/* strlib */
	HPM->share(strlib,"strlib");
	HPM->share(sv,"sv");
	HPM->share(StrBuf,"StrBuf");
	/* sql */
	HPM->share(SQL,"SQL");
	/* timer */
	HPM->share(timer,"timer");
	
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
	HPMiMalloc->astrdup = HPM_astrdup;

	sscanf(HPM_VERSION, "%d.%d", &HPM->version[0], &HPM->version[1]);
	
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
	
	/* HPM->fnames is cleared after the memory manager goes down */
	iMalloc->post_shutdown = hpm_memdown;
	
	return;
}
void hpm_defaults(void) {
	HPM = &HPM_s;
	
	HPM->fnames = NULL;
	HPM->fnamec = 0;
	
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
}
