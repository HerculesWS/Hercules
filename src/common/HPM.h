// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef	_HPM_H_
#define _HPM_H_

#include "../common/cbasetypes.h"
#include "../common/HPMi.h"

#ifdef WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#define plugin_open(x)		LoadLibraryA(x)
	#define plugin_import(x,y,z)	(z)GetProcAddress(x,y)
	#define plugin_close(x)	FreeLibrary(x)

	#define DLL_EXT			".dll"
	#define DLL				HINSTANCE
#else
	#include <dlfcn.h>
	#define plugin_open(x)		dlopen(x,RTLD_NOW)
	#define plugin_import(x,y,z)	(z)dlsym(x,y)
	#define plugin_close(x)	dlclose(x)

	#ifdef CYGWIN
		#define DLL_EXT		".dll"
	#else
		#define DLL_EXT		".so"
	#endif

	#define DLL				void *

	#include <string.h> // size_t

#endif

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

/* Hercules Plugin Manager Interface */
struct HPM_interface {
	/* vars */
	unsigned int version[2];
	bool off;
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
	/* funcs */
	void (*init) (void);
	void (*final) (void);
	struct hplugin * (*create) (void);
	void (*load) (const char* filename);
	void (*unload) (struct hplugin* plugin);
	bool (*exists) (const char *filename);
	bool (*iscompatible) (char* version);
	void (*event) (enum hp_event_types type);
	void *(*import_symbol) (char *);
	void (*share) (void *, char *);
	void (*symbol_defaults) (void);
	void (*config_read) (void);
	bool (*populate) (struct hplugin *plugin,const char *filename);
	void (*symbol_defaults_sub) (void);//TODO drop
	char *(*pid2name) (unsigned int pid);
	unsigned char (*parse_packets) (int fd, enum HPluginPacketHookingPoints point);
	void (*load_sub) (struct hplugin *plugin);
} HPM_s;

struct HPM_interface *HPM;

void hpm_defaults(void);

#endif /* _HPM_H_ */
