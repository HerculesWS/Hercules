// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef _COMMON_HPM_H_
#define _COMMON_HPM_H_

#include "../common/cbasetypes.h"
#include "../common/HPMi.h"

#ifdef WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#define plugin_open(x)        LoadLibraryA(x)
	#define plugin_import(x,y,z)  (z)GetProcAddress((x),(y))
	#define plugin_close(x)       FreeLibrary(x)

	#define DLL_EXT               ".dll"
	#define DLL                   HINSTANCE
#else // ! WIN32
	#include <dlfcn.h>
	#ifdef RTLD_DEEPBIND // Certain linux ditributions require this, but it's not available everywhere
		#define plugin_open(x) dlopen((x),RTLD_NOW|RTLD_DEEPBIND)
	#else // ! RTLD_DEEPBIND
		#define plugin_open(x) dlopen((x),RTLD_NOW)
	#endif // RTLD_DEEPBIND
	#define plugin_import(x,y,z)   (z)dlsym((x),(y))
	#define plugin_close(x)        dlclose(x)

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

struct hpm_symbol {
	char *name;
	void *ptr;
};

struct HPluginData {
	unsigned int pluginID;
	unsigned int type;
	struct {
		unsigned int free : 1;
	} flag;
	void *data;
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

struct HPMArgData {
	unsigned int pluginID;
	char *name;/* e.g. "--my-arg","-v","--whatever" */
	void (*help) (void);/* to display when --help is used */
	void (*func) (char *param);/* NULL when no param is available */
	bool has_param;/* because of the weird "--arg<space>param" instead of the "--arg=param" */
};

struct HPDataOperationStorage {
	void **HPDataSRCPtr;
	unsigned int *hdatac;
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
	struct hplugin **plugins;
	unsigned int plugin_count;
	struct hpm_symbol **symbols;
	unsigned int symbol_count;
	/* packet hooking points */
	struct HPluginPacket *packets[hpPHP_MAX];
	unsigned int packetsc[hpPHP_MAX];
	/* plugin file ptr caching */
	struct HPMFileNameCache *fnames;
	unsigned int fnamec;
	/* config listen */
	struct HPConfListenStorage *confs[HPCT_MAX];
	unsigned int confsc[HPCT_MAX];
	/* --command-line */
	DBMap *arg_db;
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
	void (*share) (void *, char *);
	void (*symbol_defaults) (void);
	void (*config_read) (const char * const *extra_plugins, int extra_plugins_count);
	bool (*populate) (struct hplugin *plugin,const char *filename);
	void (*symbol_defaults_sub) (void);//TODO drop
	char *(*pid2name) (unsigned int pid);
	unsigned char (*parse_packets) (int fd, enum HPluginPacketHookingPoints point);
	void (*load_sub) (struct hplugin *plugin);
	bool (*addhook_sub) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
	bool (*parse_arg) (const char *arg, int* index, char *argv[], bool param);
	void (*arg_help) (void);
	int (*arg_db_clear_sub) (DBKey key, DBData *data, va_list args);
	void (*grabHPData) (struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr);
	/* for server-specific HPData e.g. map_session_data */
	bool (*grabHPDataSub) (struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr);
	/* for custom config parsing */
	bool (*parseConf) (const char *w1, const char *w2, enum HPluginConfType point);
	/* validates plugin data */
	bool (*DataCheck) (struct s_HPMDataCheck *src, unsigned int size, char *name);
} HPM_s;

struct HPM_interface *HPM;

void hpm_defaults(void);

#endif /* _COMMON_HPM_H_ */
