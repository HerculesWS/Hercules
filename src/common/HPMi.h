// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef	_HPMi_H_
#define _HPMi_H_

#include "../common/cbasetypes.h"
#include "../common/core.h"
#include "../common/console.h"

struct script_state;
struct AtCommandInfo;

#ifdef WIN32
	#define HPExport __declspec(dllexport)
#else
	#define HPExport
#endif

#ifndef _SHOWMSG_H_
	HPExport void (*ShowMessage) (const char *, ...);
	HPExport void (*ShowStatus) (const char *, ...);
	HPExport void (*ShowSQL) (const char *, ...);
	HPExport void (*ShowInfo) (const char *, ...);
	HPExport void (*ShowNotice) (const char *, ...);
	HPExport void (*ShowWarning) (const char *, ...);
	HPExport void (*ShowDebug) (const char *, ...);
	HPExport void (*ShowError) (const char *, ...);
	HPExport void (*ShowFatalError) (const char *, ...);
#endif

/* after */
#include "../common/showmsg.h"

#define HPM_VERSION "0.1"

struct hplugin_info {
	char* name;
	enum server_types type;
	char* version;
	char* req_version;
};

HPExport void *(*import_symbol) (char *name);
#define GET_SYMBOL(n) import_symbol(n)

#define SERVER_TYPE_ALL SERVER_TYPE_LOGIN|SERVER_TYPE_CHAR|SERVER_TYPE_MAP

enum hp_event_types {
	HPET_INIT,/* server starts */
	HPET_FINAL,/* server is shutting down */
	HPET_READY,/* server is ready (online) */
	HPET_MAX,
};

/* Hercules Plugin Mananger Include Interface */
struct HPMi_interface {
	void (*event[HPET_MAX]) (void);
	bool (*addCommand) (char *name, bool (*func)(const int fd, struct map_session_data* sd, const char* command, const char* message,struct AtCommandInfo *info));
	bool (*addScript) (char *name, char *args, bool (*func)(struct script_state *st));
	void (*addCPCommand) (char *name, CParseFunc func);
} HPMi_s;
#ifndef _HPM_H_
	struct HPMi_interface *HPMi;
#endif

#endif /* _HPMi_H_ */
