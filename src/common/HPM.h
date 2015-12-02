// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef COMMON_HPM_H
#define COMMON_HPM_H

#ifndef HERCULES_CORE
#error You should never include HPM.h from a plugin.
#endif

#include "common/hercules.h"
#include "common/db.h"
#include "common/HPMi.h"

#ifdef WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#define plugin_open(x)        LoadLibraryA(x)
	#define plugin_import(x,y,z)  (z)GetProcAddress((x),(y))
	#define plugin_close(x)       FreeLibrary(x)
	#define plugin_geterror(buf)  (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf, sizeof(buf), NULL) ? buf : "Unknown error")

	#define DLL_EXT               ".dll"
	#define DLL                   HINSTANCE
#else // ! WIN32
	#include <dlfcn.h>
	#ifdef RTLD_DEEPBIND // Certain linux distributions require this, but it's not available everywhere
		#define plugin_open(x) dlopen((x),RTLD_NOW|RTLD_DEEPBIND)
	#else // ! RTLD_DEEPBIND
		#define plugin_open(x) dlopen((x),RTLD_NOW)
	#endif // RTLD_DEEPBIND
	#define plugin_import(x,y,z)   (z)dlsym((x),(y))
	#define plugin_close(x)        dlclose(x)
	#define plugin_geterror(buf)   ((void)buf, dlerror())

	#if defined CYGWIN
		#define DLL_EXT        ".dll"
	#elif defined __DARWIN__
		#define DLL_EXT        ".dylib"
	#else
		#define DLL_EXT        ".so"
	#endif

	#define DLL                    void *

	#include <string.h> // size_t

#endif // WIN32

struct hplugin {
	DLL dll;
	unsigned int idx;
	char *filename;
	struct hplugin_info *info;
	struct HPMi_interface *hpi;
};

/**
 * Symbols shared between core and plugins.
 */
struct hpm_symbol {
	const char *name; ///< The symbol name
	void *ptr;        ///< The symbol value
};

/**
 * A plugin custom data, to be injected in various interfaces and objects.
 */
struct hplugin_data_entry {
	uint32 pluginID; ///< The owner plugin identifier.
	uint32 classid;  ///< The entry's object type, managed by the plugin (for plugins that need more than one entry).
	struct {
		unsigned int free : 1; ///< Whether the entry data should be automatically cleared by the HPM.
	} flag;
	void *data;      ///< The entry data.
};

/**
 * A store for plugin custom data entries.
 */
struct hplugin_data_store {
	enum HPluginDataTypes type;                       ///< The store type.
	VECTOR_DECL(struct hplugin_data_entry *) entries; ///< The store entries.
};

struct HPluginPacket {
	unsigned int pluginID;
	unsigned short cmd;
	short len;
	void (*receive) (int fd);
};

struct HPMFileNameCache {
	const char *addr;
	char *name;
};

/*  */
struct HPConfListenStorage {
	unsigned int pluginID;
	char key[HPM_ADDCONF_LENGTH];
	void (*func) (const char *val);
};

/* Hercules Plugin Manager Interface */
struct HPM_interface {
	/* vars */
	unsigned int version[2];
	bool off;
	bool hooking;
	/* hooking */
	bool force_return;
	/* data */
	VECTOR_DECL(struct hplugin *) plugins;
	VECTOR_DECL(struct hpm_symbol *) symbols;
	/* packet hooking points */
	VECTOR_DECL(struct HPluginPacket) packets[hpPHP_MAX];
	/* plugin file ptr caching */
	struct {
		// This doesn't use a VECTOR because it needs to exist after the memory manager goes down.
		int count;
		struct HPMFileNameCache *data;
	} filenames;
	/* config listen */
	VECTOR_DECL(struct HPConfListenStorage) config_listeners[HPCT_MAX];
	/** Plugins requested through the command line */
	VECTOR_DECL(char *) cmdline_load_plugins;
	/* funcs */
	void (*init) (void);
	void (*final) (void);
	struct hplugin * (*create) (void);
	struct hplugin * (*load) (const char* filename);
	void (*unload) (struct hplugin* plugin);
	bool (*exists) (const char *filename);
	bool (*iscompatible) (char* version);
	void (*event) (enum hp_event_types type);
	void *(*import_symbol) (char *name, unsigned int pID);
	void (*share) (void *value, const char *name);
	void (*config_read) (void);
	char *(*pid2name) (unsigned int pid);
	unsigned char (*parse_packets) (int fd, enum HPluginPacketHookingPoints point);
	void (*load_sub) (struct hplugin *plugin);
	bool (*addhook_sub) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
	/* for custom config parsing */
	bool (*parseConf) (const char *w1, const char *w2, enum HPluginConfType point);
	/* validates plugin data */
	bool (*DataCheck) (struct s_HPMDataCheck *src, unsigned int size, int version, char *name);
	void (*datacheck_init) (const struct s_HPMDataCheck *src, unsigned int length, int version);
	void (*datacheck_final) (void);

	void (*data_store_create) (struct hplugin_data_store **storeptr, enum HPluginDataTypes type);
	void (*data_store_destroy) (struct hplugin_data_store **storeptr);
	bool (*data_store_validate) (enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize);
	/* for server-specific HPData e.g. map_session_data */
	bool (*data_store_validate_sub) (enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize);
};

CMDLINEARG(loadplugin);

extern struct HPM_interface *HPM;

void hpm_defaults(void);

#endif /* COMMON_HPM_H */
