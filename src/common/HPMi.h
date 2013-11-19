// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef	_HPMi_H_
#define _HPMi_H_

#include "../common/cbasetypes.h"
#include "../common/core.h"
#include "../common/console.h"
#include "../common/sql.h"

struct script_state;
struct AtCommandInfo;
struct socket_data;
struct map_session_data;

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

#define HPM_VERSION "1.0"

struct hplugin_info {
	char* name;
	enum server_types type;
	char* version;
	char* req_version;
};

HPExport void *(*import_symbol) (char *name, unsigned int pID);
HPExport Sql *mysql_handle;

#define GET_SYMBOL(n) import_symbol((n),HPMi->pid)

#define SERVER_TYPE_ALL (SERVER_TYPE_LOGIN|SERVER_TYPE_CHAR|SERVER_TYPE_MAP)

enum hp_event_types {
	HPET_INIT,/* server starts */
	HPET_FINAL,/* server is shutting down */
	HPET_READY,/* server is ready (online) */
	HPET_POST_FINAL,/* server is done shutting down */
	HPET_PRE_INIT,/* server is about to start (used to e.g. add custom "--args" handling) */
	HPET_MAX,
};

enum HPluginPacketHookingPoints {
	hpClif_Parse,		/* map-server (client-map) */
	hpChrif_Parse,		/* map-server (char-map) */
	hpParse_FromMap,	/* char-server (map-char) */
	hpParse_FromLogin,	/* char-server (login-char) */
	hpParse_Char,		/* char-server (client-char) */
	hpParse_FromChar,	/* login-server (char-login) */
	hpParse_Login,		/* login-server (client-login) */
	/* */
	hpPHP_MAX,
};

enum HPluginHookType {
	HOOK_TYPE_PRE,
	HOOK_TYPE_POST,
};

enum HPluginDataTypes {
	HPDT_SESSION,
	HPDT_MSD,
	HPDT_NPCD,
};

#define addHookPre(tname,hook) (HPMi->AddHook(HOOK_TYPE_PRE,(tname),(hook),HPMi->pid))
#define addHookPost(tname,hook) (HPMi->AddHook(HOOK_TYPE_POST,(tname),(hook),HPMi->pid))
/* need better names ;/ */
/* will not run the original function after pre-hook processing is complete (other hooks will run) */
#define hookStop() (HPMi->HookStop(__func__,HPMi->pid))
#define hookStopped() (HPMi->HookStopped())

#define addArg(name,param,func,help) (HPMi->addArg(HPMi->pid,(name),(param),(func),(help)))
/* HPData handy redirects */
/* session[] */
#define addToSession(ptr,data,index,autofree) (HPMi->addToHPData(HPDT_SESSION,HPMi->pid,(ptr),(data),(index),(autofree)))
#define getFromSession(ptr,index) (HPMi->getFromHPData(HPDT_SESSION,HPMi->pid,(ptr),(index)))
#define removeFromSession(ptr,index) (HPMi->removeFromHPData(HPDT_SESSION,HPMi->pid,(ptr),(index)))
/* map_session_data */
#define addToMSD(ptr,data,index,autofree) (HPMi->addToHPData(HPDT_MSD,HPMi->pid,(ptr),(data),(index),(autofree)))
#define getFromMSD(ptr,index) (HPMi->getFromHPData(HPDT_MSD,HPMi->pid,(ptr),(index)))
#define removeFromMSD(ptr,index) (HPMi->removeFromHPData(HPDT_MSD,HPMi->pid,(ptr),(index)))
/* npc_data */
#define addToNPCD(ptr,data,index,autofree) (HPMi->addToHPData(HPDT_NPCD,HPMi->pid,(ptr),(data),(index),(autofree)))
#define getFromNPCD(ptr,index) (HPMi->getFromHPData(HPDT_NPCD,HPMi->pid,(ptr),(index)))
#define removeFromNPCD(ptr,index) (HPMi->removeFromHPData(HPDT_NPCD,HPMi->pid,(ptr),(index)))

/* Hercules Plugin Mananger Include Interface */
HPExport struct HPMi_interface {
	/* */
	unsigned int pid;
	/* */
	void (*event[HPET_MAX]) (void);
	bool (*addCommand) (char *name, bool (*func)(const int fd, struct map_session_data* sd, const char* command, const char* message,struct AtCommandInfo *info));
	bool (*addScript) (char *name, char *args, bool (*func)(struct script_state *st));
	void (*addCPCommand) (char *name, CParseFunc func);
	/* HPM Custom Data */
	void (*addToHPData) (enum HPluginDataTypes type, unsigned int pluginID, void *ptr, void *data, unsigned int index, bool autofree);
	void *(*getFromHPData) (enum HPluginDataTypes type, unsigned int pluginID, void *ptr, unsigned int index);
	void (*removeFromHPData) (enum HPluginDataTypes type, unsigned int pluginID, void *ptr, unsigned int index);
	/* packet */
	bool (*addPacket) (unsigned short cmd, unsigned short length, void (*receive)(int fd), unsigned int point, unsigned int pluginID);
	/* Hooking */
	bool (*AddHook) (enum HPluginHookType type, const char *target, void *hook, unsigned int pID);
	void (*HookStop) (const char *func, unsigned int pID);
	bool (*HookStopped) (void);
	/* program --arg/-a */
	bool (*addArg) (unsigned int pluginID, char *name, bool has_param,void (*func) (char *param),void (*help) (void));
} HPMi_s;
#ifndef _HPM_H_
HPExport struct HPMi_interface *HPMi;
#endif

#endif /* _HPMi_H_ */
