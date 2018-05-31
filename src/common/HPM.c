/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2016  Hercules Dev Team
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define HERCULES_CORE

#include "config/core.h" // CONSOLE_INPUT
#include "HPM.h"

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"
#include "common/nullpo.h"
#include "plugins/HPMHooking.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#	include <unistd.h>
#endif

struct malloc_interface iMalloc_HPM;
struct malloc_interface *HPMiMalloc;
struct HPM_interface HPM_s;
struct HPM_interface *HPM;
struct HPMHooking_core_interface HPMHooking_core_s;

/**
 * (char*) data name -> (unsigned int) HPMDataCheck[] index
 **/
struct DBMap *datacheck_db;
int datacheck_version;
const struct s_HPMDataCheck *datacheck_data;

/**
 * Executes an event on all loaded plugins.
 *
 * @param type The event type to trigger.
 */
void hplugin_trigger_event(enum hp_event_types type)
{
	int i;
	for (i = 0; i < VECTOR_LENGTH(HPM->plugins); i++) {
		struct hplugin *plugin = VECTOR_INDEX(HPM->plugins, i);
		if (plugin->hpi->event[type] != NULL)
			plugin->hpi->event[type]();
	}
}

/**
 * Exports a symbol to the shared symbols list.
 *
 * @param value The symbol value.
 * @param name  The symbol name.
 */
void hplugin_export_symbol(void *value, const char *name)
{
	struct hpm_symbol *symbol = NULL;
	CREATE(symbol ,struct hpm_symbol, 1);
	symbol->name = name;
	symbol->ptr = value;
	VECTOR_ENSURE(HPM->symbols, 1, 1);
	VECTOR_PUSH(HPM->symbols, symbol);
}

/**
 * Imports a shared symbol.
 *
 * @param name The symbol name.
 * @param pID  The requesting plugin ID.
 * @return The symbol value.
 * @retval NULL if the symbol wasn't found.
 */
void *hplugin_import_symbol(char *name, unsigned int pID)
{
	int i;
	nullpo_retr(NULL, name);
	ARR_FIND(0, VECTOR_LENGTH(HPM->symbols), i, strcmp(VECTOR_INDEX(HPM->symbols, i)->name, name) == 0);

	if (i != VECTOR_LENGTH(HPM->symbols))
		return VECTOR_INDEX(HPM->symbols, i)->ptr;

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

/**
 * Checks whether a plugin is currently loaded
 *
 * @param filename The plugin filename.
 * @retval true  if the plugin exists and is currently loaded.
 * @retval false otherwise.
 */
bool hplugin_exists(const char *filename)
{
	int i;
	nullpo_retr(false, filename);
	for (i = 0; i < VECTOR_LENGTH(HPM->plugins); i++) {
		if (strcmpi(VECTOR_INDEX(HPM->plugins, i)->filename,filename) == 0)
			return true;
	}
	return false;
}

/**
 * Initializes the data structure for a new plugin and registers it.
 *
 * @return A (retained) pointer to the initialized data.
 */
struct hplugin *hplugin_create(void)
{
	struct hplugin *plugin = NULL;
	CREATE(plugin, struct hplugin, 1);
	plugin->idx = (int)VECTOR_LENGTH(HPM->plugins);
	plugin->filename = NULL;
	VECTOR_ENSURE(HPM->plugins, 1, 1);
	VECTOR_PUSH(HPM->plugins, plugin);
	return plugin;
}

bool hplugins_addpacket(unsigned short cmd, unsigned short length, void (*receive) (int fd), unsigned int point, unsigned int pluginID)
{
	struct HPluginPacket *packet;
	int i;

	if (point >= hpPHP_MAX) {
		ShowError("HPM->addPacket:%s: unknown point '%u' specified for packet 0x%04x (len %d)\n",HPM->pid2name(pluginID),point,cmd,length);
		return false;
	}

	for (i = 0; i < VECTOR_LENGTH(HPM->packets[point]); i++) {
		if (VECTOR_INDEX(HPM->packets[point], i).cmd == cmd ) {
			ShowError("HPM->addPacket:%s: can't add packet 0x%04x, already in use by '%s'!",
					HPM->pid2name(pluginID), cmd, HPM->pid2name(VECTOR_INDEX(HPM->packets[point], i).pluginID));
			return false;
		}
	}

	VECTOR_ENSURE(HPM->packets[point], 1, 1);
	VECTOR_PUSHZEROED(HPM->packets[point]);
	packet = &VECTOR_LAST(HPM->packets[point]);

	packet->pluginID = pluginID;
	packet->cmd = cmd;
	packet->len = length;
	packet->receive = receive;

	return true;
}

/**
 * Validates and if necessary initializes a plugin data store.
 *
 * @param type[in]         The data store type.
 * @param storeptr[in,out] A pointer to the store.
 * @param initialize       Whether the store should be initialized in case it isn't.
 * @retval false if the store is an invalid or mismatching type.
 * @retval true  if the store is a valid type.
 *
 * @remark
 *     If \c storeptr is a pointer to a NULL pointer (\c storeptr itself can't
 *     be NULL), two things may happen, depending on the \c initialize value:
 *     if false, then \c storeptr isn't changed; if true, then \c storeptr is
 *     initialized through \c HPM->data_store_create() and ownership is passed
 *     to the caller.
 */
bool hplugin_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize)
{
	struct hplugin_data_store *store;
	nullpo_retr(false, storeptr);
	store = *storeptr;
	if (!initialize && store == NULL)
		return true;

	switch (type) {
		/* core-handled */
		case HPDT_SESSION:
			break;
		default:
			if (HPM->data_store_validate_sub == NULL) {
				ShowError("HPM:validateHPData failed, type %u needs sub-handler!\n", type);
				return false;
			}
			if (!HPM->data_store_validate_sub(type, storeptr, initialize)) {
				ShowError("HPM:HPM:validateHPData failed, unknown type %u!\n", type);
				return false;
			}
			break;
	}
	if (initialize && (!store || store->type == HPDT_UNKNOWN)) {
		HPM->data_store_create(storeptr, type);
		store = *storeptr;
	}
	if (store->type != type) {
		ShowError("HPM:HPM:validateHPData failed, store type mismatch %u != %u.\n", store->type, type);
		return false;
	}
	return true;
}

/**
 * Adds an entry to a plugin data store.
 *
 * @param type[in]         The store type.
 * @param pluginID[in]     The plugin identifier.
 * @param storeptr[in,out] A pointer to the store. The store will be initialized if necessary.
 * @param data[in]         The data entry to add.
 * @param classid[in]      The entry class identifier.
 * @param autofree[in]     Whether the entry should be automatically freed when removed.
 */
void hplugins_addToHPData(enum HPluginDataTypes type, uint32 pluginID, struct hplugin_data_store **storeptr, void *data, uint32 classid, bool autofree)
{
	struct hplugin_data_store *store;
	struct hplugin_data_entry *entry;
	int i;
	nullpo_retv(storeptr);

	if (!HPM->data_store_validate(type, storeptr, true)) {
		/* woo it failed! */
		ShowError("HPM:addToHPData:%s: failed, type %u (%u|%u)\n", HPM->pid2name(pluginID), type, pluginID, classid);
		return;
	}
	store = *storeptr;
	nullpo_retv(store);

	/* duplicate check */
	ARR_FIND(0, VECTOR_LENGTH(store->entries), i, VECTOR_INDEX(store->entries, i)->pluginID == pluginID && VECTOR_INDEX(store->entries, i)->classid == classid);
	if (i != VECTOR_LENGTH(store->entries)) {
		ShowError("HPM:addToHPData:%s: error! attempting to insert duplicate struct of id %u and classid %u\n", HPM->pid2name(pluginID), pluginID, classid);
		return;
	}

	/* hplugin_data_entry is always same size, probably better to use the ERS (with reasonable chunk size e.g. 10/25/50) */
	CREATE(entry, struct hplugin_data_entry, 1);

	/* input */
	entry->pluginID = pluginID;
	entry->classid = classid;
	entry->flag.free = autofree ? 1 : 0;
	entry->data = data;

	VECTOR_ENSURE(store->entries, 1, 1);
	VECTOR_PUSH(store->entries, entry);
}

/**
 * Retrieves an entry from a plugin data store.
 *
 * @param type[in]     The store type.
 * @param pluginID[in] The plugin identifier.
 * @param store[in]    The store.
 * @param classid[in]  The entry class identifier.
 *
 * @return The retrieved entry, or NULL.
 */
void *hplugins_getFromHPData(enum HPluginDataTypes type, uint32 pluginID, struct hplugin_data_store *store, uint32 classid)
{
	int i;

	if (!HPM->data_store_validate(type, &store, false)) {
		/* woo it failed! */
		ShowError("HPM:getFromHPData:%s: failed, type %u (%u|%u)\n", HPM->pid2name(pluginID), type, pluginID, classid);
		return NULL;
	}
	if (!store)
		return NULL;

	ARR_FIND(0, VECTOR_LENGTH(store->entries), i, VECTOR_INDEX(store->entries, i)->pluginID == pluginID && VECTOR_INDEX(store->entries, i)->classid == classid);
	if (i != VECTOR_LENGTH(store->entries))
		return VECTOR_INDEX(store->entries, i)->data;

	return NULL;
}

/**
 * Removes an entry from a plugin data store.
 *
 * @param type[in]     The store type.
 * @param pluginID[in] The plugin identifier.
 * @param store[in]    The store.
 * @param classid[in]  The entry class identifier.
 */
void hplugins_removeFromHPData(enum HPluginDataTypes type, uint32 pluginID, struct hplugin_data_store *store, uint32 classid)
{
	struct hplugin_data_entry *entry;
	int i;

	if (!HPM->data_store_validate(type, &store, false)) {
		/* woo it failed! */
		ShowError("HPM:removeFromHPData:%s: failed, type %u (%u|%u)\n", HPM->pid2name(pluginID), type, pluginID, classid);
		return;
	}
	if (!store)
		return;

	ARR_FIND(0, VECTOR_LENGTH(store->entries), i, VECTOR_INDEX(store->entries, i)->pluginID == pluginID && VECTOR_INDEX(store->entries, i)->classid == classid);
	if (i == VECTOR_LENGTH(store->entries))
		return;

	entry = VECTOR_INDEX(store->entries, i);
	VECTOR_ERASE(store->entries, i); // Erase and compact
	aFree(entry->data); // when it's removed we delete it regardless of autofree
	aFree(entry);
}

/* TODO: add ability for tracking using pID for the upcoming runtime load/unload support. */
bool HPM_AddHook(enum HPluginHookType type, const char *target, void *hook, unsigned int pID)
{
	if (!HPM->hooking->enabled) {
		ShowError("HPM:AddHook Fail! '%s' tried to hook to '%s' but HPMHooking is disabled!\n",HPM->pid2name(pID),target);
		return false;
	}
	/* search if target is a known hook point within 'common' */
	/* if not check if a sub-hooking list is available (from the server) and run it by */
	if (HPM->hooking->addhook_sub != NULL && HPM->hooking->addhook_sub(type,target,hook,pID))
		return true;

	ShowError("HPM:AddHook: unknown Hooking Point '%s'!\n",target);

	return false;
}

void HPM_HookStop(const char *func, unsigned int pID)
{
	/* track? */
	HPM->hooking->force_return = true;
}

bool HPM_HookStopped(void)
{
	return HPM->hooking->force_return;
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
bool hpm_add_arg(unsigned int pluginID, char *name, bool has_param, CmdlineExecFunc func, const char *help)
{
	int i;

	if (!name || strlen(name) < 3 || name[0] != '-' || name[1] != '-') {
		ShowError("HPM:add_arg:%s invalid argument name: arguments must begin with '--' (from %s)\n", name, HPM->pid2name(pluginID));
		return false;
	}

	ARR_FIND(0, VECTOR_LENGTH(cmdline->args_data), i, strcmp(VECTOR_INDEX(cmdline->args_data, i).name, name) == 0);

	if (i != VECTOR_LENGTH(cmdline->args_data)) {
		ShowError("HPM:add_arg:%s duplicate! (from %s)\n",name,HPM->pid2name(pluginID));
		return false;
	}

	return cmdline->arg_add(pluginID, name, '\0', func, help, has_param ? CMDLINE_OPT_PARAM : CMDLINE_OPT_NORMAL);
}

/**
 * Adds a configuration listener for a plugin.
 *
 * @param pluginID The plugin identifier.
 * @param type     The configuration type to listen for.
 * @param name     The configuration entry name.
 * @param func     The callback function.
 * @retval true if the listener was added successfully.
 * @retval false in case of error.
 */
bool hplugins_addconf(unsigned int pluginID, enum HPluginConfType type, char *name, void (*parse_func) (const char *key, const char *val), int (*return_func) (const char *key), bool required)
{
	struct HPConfListenStorage *conf;
	int i;

	if (parse_func == NULL) {
		ShowError("HPM->addConf:%s: missing setter function for config '%s'\n",HPM->pid2name(pluginID),name);
		return false;
	}

	if (type == HPCT_BATTLE && return_func == NULL) {
		ShowError("HPM->addConf:%s: missing getter function for config '%s'\n",HPM->pid2name(pluginID),name);
		return false;
	}

	if (type >= HPCT_MAX) {
		ShowError("HPM->addConf:%s: unknown point '%u' specified for config '%s'\n",HPM->pid2name(pluginID),type,name);
		return false;
	}

	ARR_FIND(0, VECTOR_LENGTH(HPM->config_listeners[type]), i, strcmpi(name, VECTOR_INDEX(HPM->config_listeners[type], i).key) == 0);
	if (i != VECTOR_LENGTH(HPM->config_listeners[type])) {
		ShowError("HPM->addConf:%s: duplicate '%s', already in use by '%s'!",
				HPM->pid2name(pluginID), name, HPM->pid2name(VECTOR_INDEX(HPM->config_listeners[type], i).pluginID));
		return false;
	}

	VECTOR_ENSURE(HPM->config_listeners[type], 1, 1);
	VECTOR_PUSHZEROED(HPM->config_listeners[type]);
	conf = &VECTOR_LAST(HPM->config_listeners[type]);

	conf->pluginID = pluginID;
	safestrncpy(conf->key, name, HPM_ADDCONF_LENGTH);
	conf->parse_func = parse_func;
	conf->return_func = return_func;
	conf->required = required;

	return true;
}

struct hplugin *hplugin_load(const char* filename)
{
	struct hplugin *plugin;
	struct hplugin_info *info;
	struct HPMi_interface **HPMi;
	bool anyEvent = false;
	void **import_symbol_ref;
	int *HPMDataCheckVer;
	unsigned int *HPMDataCheckLen;
	struct s_HPMDataCheck *HPMDataCheck;
	const char *(*HPMLoadEvent)(int server_type);

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

	if (!(HPMLoadEvent = plugin_import(plugin->dll, "HPM_shared_symbols", const char *(*)(int)))) {
		ShowFatalError("HPM:plugin_load: failed to retrieve 'HPM_shared_symbols' for '"CL_WHITE"%s"CL_RESET"', most likely not including HPMDataCheck.h!\n", filename);
		exit(EXIT_FAILURE);
	}
	{
		const char *failure = HPMLoadEvent(SERVER_TYPE);
		if (failure) {
			ShowFatalError("HPM:plugin_load: failed to import symbol '%s' into '"CL_WHITE"%s"CL_RESET"'.\n", failure, filename);
			exit(EXIT_FAILURE);
		}
	}

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
	plugin->hpi->memmgr             = HPMiMalloc;
#ifdef CONSOLE_INPUT
	plugin->hpi->addCPCommand       = console->input->addCommand;
#endif // CONSOLE_INPUT
	plugin->hpi->addPacket          = hplugins_addpacket;
	plugin->hpi->addToHPData        = hplugins_addToHPData;
	plugin->hpi->getFromHPData      = hplugins_getFromHPData;
	plugin->hpi->removeFromHPData   = hplugins_removeFromHPData;
	plugin->hpi->addArg             = hpm_add_arg;
	plugin->hpi->addConf            = hplugins_addconf;
	if ((plugin->hpi->hooking = plugin_import(plugin->dll, "HPMHooking_s", struct HPMHooking_interface *)) != NULL) {
		plugin->hpi->hooking->AddHook     = HPM_AddHook;
		plugin->hpi->hooking->HookStop    = HPM_HookStop;
		plugin->hpi->hooking->HookStopped = HPM_HookStopped;
	}
	/* server specific */
	if( HPM->load_sub )
		HPM->load_sub(plugin);

	ShowStatus("HPM: Loaded plugin '"CL_WHITE"%s"CL_RESET"' (%s)%s.\n",
			plugin->info->name, plugin->info->version,
			plugin->hpi->hooking != NULL ? " built with HPMHooking support" : "");

	return plugin;
}

/**
 * Unloads and unregisters a plugin.
 *
 * @param plugin The plugin data.
 */
void hplugin_unload(struct hplugin* plugin)
{
	int i;
	nullpo_retv(plugin);
	if (plugin->filename)
		aFree(plugin->filename);
	if (plugin->dll)
		plugin_close(plugin->dll);
	/* TODO: for manual packet unload */
	/* - Go through known packets and unlink any belonging to the plugin being removed */
	ARR_FIND(0, VECTOR_LENGTH(HPM->plugins), i, VECTOR_INDEX(HPM->plugins, i)->idx == plugin->idx);
	if (i != VECTOR_LENGTH(HPM->plugins)) {
		VECTOR_ERASE(HPM->plugins, i);
	}
	aFree(plugin);
}

/**
 * Adds a plugin requested from the command line to the auto-load list.
 */
CMDLINEARG(loadplugin)
{
	VECTOR_ENSURE(HPM->cmdline_load_plugins, 1, 1);
	VECTOR_PUSH(HPM->cmdline_load_plugins, aStrdup(params));
	return true;
}

/**
 * Reads the plugin configuration and loads the plugins as necessary.
 */
void hplugins_config_read(void)
{
	struct config_t plugins_conf;
	struct config_setting_t *plist = NULL;
	const char *config_filename = "conf/plugins.conf"; // FIXME hardcoded name
	FILE *fp;
	int i;

	/* yes its ugly, its temporary and will be gone as soon as the new inter-server.conf is set */
	if( (fp = fopen("conf/import/plugins.conf","r")) ) {
		config_filename = "conf/import/plugins.conf";
		fclose(fp);
	}

	if (!libconfig->load_file(&plugins_conf, config_filename))
		return;

	plist = libconfig->lookup(&plugins_conf, "plugins_list");
	for (i = 0; i < VECTOR_LENGTH(HPM->cmdline_load_plugins); i++) {
		struct config_setting_t *entry = libconfig->setting_add(plist, NULL, CONFIG_TYPE_STRING);
		config_setting_set_string(entry, VECTOR_INDEX(HPM->cmdline_load_plugins, i));
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
					if ((func = plugin_import(plugin->dll, "Hooked",const char * (*)(bool *))) != NULL
					 && (addhook_sub = plugin_import(plugin->dll, "HPM_Plugin_AddHook",bool (*)(enum HPluginHookType, const char *, void *, unsigned int))) != NULL) {
						const char *failed = func(&HPM->hooking->force_return);
						if (failed) {
							ShowError("HPM: failed to retrieve '%s' for '"CL_WHITE"%s"CL_RESET"'!\n", failed, plugin_name);
						} else {
							HPM->hooking->enabled = true;
							HPM->hooking->addhook_sub = addhook_sub;
							HPM->hooking->Hooked = func; // The purpose of this is type-checking 'func' at compile time.
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
	}
	libconfig->destroy(&plugins_conf);

	if (VECTOR_LENGTH(HPM->plugins))
		ShowStatus("HPM: There are '"CL_WHITE"%d"CL_RESET"' plugins loaded, type '"CL_WHITE"plugins"CL_RESET"' to list them\n", VECTOR_LENGTH(HPM->plugins));
}

/**
 * Console command: plugins
 *
 * Shows a list of loaded plugins.
 *
 * @see CPCMD()
 */
CPCMD(plugins)
{
	int i;

	if (VECTOR_LENGTH(HPM->plugins) == 0) {
		ShowInfo("HPC: there are no plugins loaded\n");
		return;
	}

	ShowInfo("HPC: There are '"CL_WHITE"%d"CL_RESET"' plugins loaded\n", VECTOR_LENGTH(HPM->plugins));

	for(i = 0; i < VECTOR_LENGTH(HPM->plugins); i++) {
		struct hplugin *plugin = VECTOR_INDEX(HPM->plugins, i);
		ShowInfo("HPC: - '"CL_WHITE"%s"CL_RESET"' (%s)\n", plugin->info->name, plugin->filename);
	}
}

/**
 * Parses a packet through the registered plugin.
 *
 * @param fd The connection fd.
 * @param point The packet hooking point.
 * @retval 0 unknown packet
 * @retval 1 OK
 * @retval 2 incomplete packet
 */
unsigned char hplugins_parse_packets(int fd, int packet_id, enum HPluginPacketHookingPoints point)
{
	struct HPluginPacket *packet = NULL;
	int i;
	int16 length;

	ARR_FIND(0, VECTOR_LENGTH(HPM->packets[point]), i, VECTOR_INDEX(HPM->packets[point], i).cmd == packet_id);

	if (i == VECTOR_LENGTH(HPM->packets[point]))
		return 0;

	packet = &VECTOR_INDEX(HPM->packets[point], i);
	length = packet->len;
	if (length == -1)
		length = RFIFOW(fd, 2);

	if (length > (int)RFIFOREST(fd))
		return 2;

	packet->receive(fd);
	RFIFOSKIP(fd, length);
	return 1;
}

/**
 * Retrieves a plugin name by ID.
 *
 * @param pid The plugin identifier.
 * @return The plugin name.
 * @retval "core" if the plugin ID belongs to the Hercules core.
 * @retval "UnknownPlugin" if the plugin wasn't found.
 */
char *hplugins_id2name(unsigned int pid)
{
	int i;

	if (pid == HPM_PID_CORE)
		return "core";

	for (i = 0; i < VECTOR_LENGTH(HPM->plugins); i++) {
		struct hplugin *plugin = VECTOR_INDEX(HPM->plugins, i);
		if (plugin->idx == pid)
			return plugin->info->name;
	}

	return "UnknownPlugin";
}

/**
 * Returns a retained permanent pointer to a source filename, for memory-manager reporting use.
 *
 * The returned pointer is safe to be used as filename in the memory manager
 * functions, and it will be available during and after the memory manager
 * shutdown (for memory leak reporting purposes).
 *
 * @param file The string/filename to retain
 * @return A retained copy of the source string.
 */
const char *HPM_file2ptr(const char *file)
{
	int i;

	nullpo_retr(NULL, file);
	ARR_FIND(0, HPM->filenames.count, i, HPM->filenames.data[i].addr == file);
	if (i != HPM->filenames.count) {
		return HPM->filenames.data[i].name;
	}

	/* we handle this memory outside of the server's memory manager because we need it to exist after the memory manager goes down */
	HPM->filenames.data = realloc(HPM->filenames.data, (++HPM->filenames.count)*sizeof(struct HPMFileNameCache));

	HPM->filenames.data[i].addr = file;
	HPM->filenames.data[i].name = strdup(file);

	return HPM->filenames.data[i].name;
}

void* HPM_mmalloc(size_t size, const char *file, int line, const char *func)
{
	return iMalloc->malloc(size,HPM_file2ptr(file),line,func);
}

void* HPM_calloc(size_t num, size_t size, const char *file, int line, const char *func)
{
	return iMalloc->calloc(num,size,HPM_file2ptr(file),line,func);
}

void* HPM_realloc(void *p, size_t size, const char *file, int line, const char *func)
{
	return iMalloc->realloc(p,size,HPM_file2ptr(file),line,func);
}

void* HPM_reallocz(void *p, size_t size, const char *file, int line, const char *func)
{
	return iMalloc->reallocz(p,size,HPM_file2ptr(file),line,func);
}

char* HPM_astrdup(const char *p, const char *file, int line, const char *func)
{
	return iMalloc->astrdup(p,HPM_file2ptr(file),line,func);
}

/**
 * Parses a configuration entry through the registered plugins.
 *
 * @param w1    The configuration entry name.
 * @param w2    The configuration entry value.
 * @param point The type of configuration file.
 * @retval true if a registered plugin was found to handle the entry.
 * @retval false if no registered plugins could be found.
 */
bool hplugins_parse_conf_entry(const char *w1, const char *w2, enum HPluginConfType point)
{
	int i;
	ARR_FIND(0, VECTOR_LENGTH(HPM->config_listeners[point]), i, strcmpi(w1, VECTOR_INDEX(HPM->config_listeners[point], i).key) == 0);
	if (i == VECTOR_LENGTH(HPM->config_listeners[point]))
		return false;

	VECTOR_INDEX(HPM->config_listeners[point], i).parse_func(w1, w2);
	return true;
}

/**
 * Get a battle configuration entry through the registered plugins.
 *
 * @param[in]  w1    The configuration entry name.
 * @param[out] value Where the config result will be saved
 * @retval true in case of data found
 * @retval false in case of no data found
 */
bool hplugins_get_battle_conf(const char *w1, int *value)
{
	int i;

	nullpo_retr(false, w1);
	nullpo_retr(false, value);

	ARR_FIND(0, VECTOR_LENGTH(HPM->config_listeners[HPCT_BATTLE]), i, strcmpi(w1, VECTOR_INDEX(HPM->config_listeners[HPCT_BATTLE], i).key) == 0);
	if (i == VECTOR_LENGTH(HPM->config_listeners[HPCT_BATTLE]))
		return false;

	*value = VECTOR_INDEX(HPM->config_listeners[HPCT_BATTLE], i).return_func(w1);
	return true;
}

/**
 * Parses configuration entries registered by plugins.
 *
 * @param config   The configuration file to parse.
 * @param filename Path to configuration file.
 * @param point    The type of configuration file.
 * @param imported Whether the current config is imported from another file.
 * @retval false in case of error.
 */
bool hplugins_parse_conf(const struct config_t *config, const char *filename, enum HPluginConfType point, bool imported)
{
	const struct config_setting_t *setting = NULL;
	int i, val, type;
	char buf[1024];
	bool retval = true;

	nullpo_retr(false, config);

	for (i = 0; i < VECTOR_LENGTH(HPM->config_listeners[point]); i++) {
		const struct HPConfListenStorage *entry = &VECTOR_INDEX(HPM->config_listeners[point], i);
		const char *config_name = entry->key;
		const char *str = NULL;
		if ((setting = libconfig->lookup(config, config_name)) == NULL) {
			if (!imported && entry->required) {
				ShowWarning("Missing configuration '%s' in file %s!\n", config_name, filename);
				retval = false;
			}
			continue;
		}

		switch ((type = config_setting_type(setting))) {
		case CONFIG_TYPE_INT:
			val = libconfig->setting_get_int(setting);
			sprintf(buf, "%d", val); // FIXME: Remove this when support to int's as value is added
			str = buf;
			break;
		case CONFIG_TYPE_BOOL:
			val = libconfig->setting_get_bool(setting);
			sprintf(buf, "%d", val); // FIXME: Remove this when support to int's as value is added
			str = buf;
			break;
		case CONFIG_TYPE_STRING:
			str = libconfig->setting_get_string(setting);
			break;
		default: // Unsupported type
			ShowWarning("Setting %s has unsupported type %d, ignoring...\n", config_name, type);
			retval = false;
			continue;
		}
		entry->parse_func(config_name, str);
	}
	return retval;
}

/**
 * parses battle config entries registered by plugins.
 *
 * @param config   the configuration file to parse.
 * @param filename path to configuration file.
 * @param imported whether the current config is imported from another file.
 * @retval false in case of error.
 */
bool hplugins_parse_battle_conf(const struct config_t *config, const char *filename, bool imported)
{
	const struct config_setting_t *setting = NULL;
	int i, val, type;
	char str[1024];
	bool retval = true;

	nullpo_retr(false, config);

	for (i = 0; i < VECTOR_LENGTH(HPM->config_listeners[HPCT_BATTLE]); i++) {
		const struct HPConfListenStorage *entry = &VECTOR_INDEX(HPM->config_listeners[HPCT_BATTLE], i);
		const char *config_name = entry->key;
		if ((setting = libconfig->lookup(config, config_name)) == NULL) {
			if (!imported && entry->required) {
				ShowWarning("Missing configuration '%s' in file %s!\n", config_name, filename);
				retval = false;
			}
			continue;
		}

		switch ((type = config_setting_type(setting))) {
		case CONFIG_TYPE_INT:
			val = libconfig->setting_get_int(setting);
			break;
		case CONFIG_TYPE_BOOL:
			val = libconfig->setting_get_bool(setting);
			break;
		default: // Unsupported type
			ShowWarning("Setting %s has unsupported type %d, ignoring...\n", config_name, type);
			retval = false;
			continue;
		}
		sprintf(str, "%d", val); // FIXME: Remove this when support to int's as value is added
		entry->parse_func(config_name, str);
	}
	return retval;
}

/**
 * Helper to destroy an interface's hplugin_data store and release any owned memory.
 *
 * The pointer will be cleared.
 *
 * @param storeptr[in,out] A pointer to the plugin data store.
 */
void hplugin_data_store_destroy(struct hplugin_data_store **storeptr)
{
	struct hplugin_data_store *store;
	nullpo_retv(storeptr);
	store = *storeptr;
	if (store == NULL)
		return;

	while (VECTOR_LENGTH(store->entries) > 0) {
		struct hplugin_data_entry *entry = VECTOR_POP(store->entries);
		if (entry->flag.free) {
			aFree(entry->data);
		}
		aFree(entry);
	}
	VECTOR_CLEAR(store->entries);
	aFree(store);
	*storeptr = NULL;
}

/**
 * Helper to create and initialize an interface's hplugin_data store.
 *
 * The store is owned by the caller, and it should be eventually destroyed by
 * \c hdata_destroy.
 *
 * @param storeptr[in,out] A pointer to the data store to initialize.
 * @param type[in]         The store type.
 */
void hplugin_data_store_create(struct hplugin_data_store **storeptr, enum HPluginDataTypes type)
{
	struct hplugin_data_store *store;
	nullpo_retv(storeptr);

	if (*storeptr == NULL) {
		CREATE(*storeptr, struct hplugin_data_store, 1);
	}
	store = *storeptr;

	store->type = type;
	VECTOR_INIT(store->entries);
}

/**
 * Called by HPM->DataCheck on a plugins incoming data, ensures data structs in use are matching!
 **/
bool HPM_DataCheck(struct s_HPMDataCheck *src, unsigned int size, int version, char *name)
{
	unsigned int i, j;

	nullpo_retr(false, src);
	if (version != datacheck_version) {
		ShowError("HPMDataCheck:%s: DataCheck API version mismatch %d != %d\n", name, datacheck_version, version);
		return false;
	}

	for (i = 0; i < size; i++) {
		if (!(src[i].type&SERVER_TYPE))
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

void HPM_datacheck_init(const struct s_HPMDataCheck *src, unsigned int length, int version)
{
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

void HPM_datacheck_final(void)
{
	db_destroy(datacheck_db);
}

void hpm_init(void)
{
	int i;
	datacheck_db = NULL;
	datacheck_data = NULL;
	datacheck_version = 0;

	VECTOR_INIT(HPM->plugins);
	VECTOR_INIT(HPM->symbols);

	HPM->off = false;

	HPMiMalloc = &iMalloc_HPM;
	*HPMiMalloc = *iMalloc;
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

	for (i = 0; i < hpPHP_MAX; i++) {
		VECTOR_INIT(HPM->packets[i]);
	}

	for (i = 0; i < HPCT_MAX; i++) {
		VECTOR_INIT(HPM->config_listeners[i]);
	}

#ifdef CONSOLE_INPUT
	console->input->addCommand("plugins",CPCMD_A(plugins));
#endif
	return;
}

/**
 * Releases the retained filenames cache.
 */
void hpm_memdown(void)
{
	/* this memory is handled outside of the server's memory manager and
	 * thus cleared after memory manager goes down */
	if (HPM->filenames.count) {
		int i;
		for (i = 0; i < HPM->filenames.count; i++) {
			free(HPM->filenames.data[i].name);
		}
		free(HPM->filenames.data);
		HPM->filenames.data = NULL;
		HPM->filenames.count = 0;
	}
}

void hpm_final(void)
{
	int i;

	HPM->off = true;

	while (VECTOR_LENGTH(HPM->plugins)) {
		HPM->unload(VECTOR_LAST(HPM->plugins));
	}
	VECTOR_CLEAR(HPM->plugins);

	while (VECTOR_LENGTH(HPM->symbols)) {
		aFree(VECTOR_POP(HPM->symbols));
	}
	VECTOR_CLEAR(HPM->symbols);

	for (i = 0; i < hpPHP_MAX; i++) {
		VECTOR_CLEAR(HPM->packets[i]);
	}

	for (i = 0; i < HPCT_MAX; i++) {
		VECTOR_CLEAR(HPM->config_listeners[i]);
	}

	while (VECTOR_LENGTH(HPM->cmdline_load_plugins)) {
		aFree(VECTOR_POP(HPM->cmdline_load_plugins));
	}
	VECTOR_CLEAR(HPM->cmdline_load_plugins);

	/* HPM->fnames is cleared after the memory manager goes down */
	iMalloc->post_shutdown = hpm_memdown;

	return;
}

void hpm_defaults(void)
{
	HPM = &HPM_s;
	HPM->hooking = &HPMHooking_core_s;

	memset(&HPM->filenames, 0, sizeof(HPM->filenames));
	VECTOR_INIT(HPM->cmdline_load_plugins);
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
	HPM->config_read = hplugins_config_read;
	HPM->pid2name = hplugins_id2name;
	HPM->parse_packets = hplugins_parse_packets;
	HPM->load_sub = NULL;
	HPM->parse_conf_entry = hplugins_parse_conf_entry;
	HPM->parse_conf = hplugins_parse_conf;
	HPM->parse_battle_conf = hplugins_parse_battle_conf;
	HPM->getBattleConf = hplugins_get_battle_conf;
	HPM->DataCheck = HPM_DataCheck;
	HPM->datacheck_init = HPM_datacheck_init;
	HPM->datacheck_final = HPM_datacheck_final;

	HPM->data_store_destroy = hplugin_data_store_destroy;
	HPM->data_store_create = hplugin_data_store_create;
	HPM->data_store_validate = hplugin_data_store_validate;
	HPM->data_store_validate_sub = NULL;

	HPM->hooking->enabled = false;
	HPM->hooking->force_return = false;
	HPM->hooking->addhook_sub = NULL;
	HPM->hooking->Hooked = NULL;
}
