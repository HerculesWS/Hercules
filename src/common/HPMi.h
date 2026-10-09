/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2026 Hercules Dev Team
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
#ifndef COMMON_HPMI_H
#define COMMON_HPMI_H

#include "common/hercules.h"
#include "common/apipackets.h"
#include "common/console.h"
#include "common/core.h"
#include "common/nullpo.h"
#include "common/showmsg.h"

struct HPMHooking_interface;
struct Sql; // common/sql.h
struct script_state;
struct AtCommandInfo;
struct socket_data;
struct map_session_data;
struct hplugin_data_store;

enum server_types : unsigned int;

#define HPM_VERSION "1.4"

// Maximum length of the configuration path for configs added with add*Conf
#define HPM_ADDCONF_LENGTH 40

struct s_HPMDataCheck {
	const char *name;
	unsigned int size;
	int type;
};

#define SERVER_TYPE_ALL (SERVER_TYPE_LOGIN | SERVER_TYPE_CHAR | SERVER_TYPE_MAP | SERVER_TYPE_API)

enum hp_event_types {
	HPET_INIT,       /* server starts */
	HPET_FINAL,      /* server is shutting down */
	HPET_READY,      /* server is ready (online) */
	HPET_POST_FINAL, /* server is done shutting down */
	HPET_PRE_INIT,   /* server is about to start (used to e.g. add custom "--args" handling) */
	HPET_MAX,
};

enum HPluginPacketHookingPoints {
	hpClif_Parse,      ///< map-server (client-map)
	hpChrif_Parse,     ///< map-server (char-map)
	hpParse_FromMap,   ///< char-server (map-char)
	hpParse_FromLogin, ///< char-server (login-char)
	hpParse_Char,      ///< char-server (client-char)
	hpParse_FromChar,  ///< login-server (char-login)
	hpParse_Login,     ///< login-server (client-login)
	hpParse_ApiLogin,  ///< login-server (api-login)
	hpParse_LoginApi,  ///< api-server (login-api)
	hpProxy_ApiLogin,  ///< login-server (api-login)
	hpProxy_ApiChar,   ///< char-server (api-char)
	hpProxy_ApiMap,    ///< map-server (api-map)
	/* */
	hpPHP_MAX,
};

/**
 * Data types for plugin custom data.
 */
enum HPluginDataTypes {
	HPDT_UNKNOWN,        ///< Unknown type (such as an uninitialized store).
	HPDT_SESSION,        ///< For struct socket_data.
	HPDT_MSD,            ///< For struct map_session_data.
	HPDT_NPCD,           ///< For struct npc_data.
	HPDT_MAP,            ///< For struct map_data.
	HPDT_INSTANCE,       ///< For struct instance_data.
	HPDT_GUILD,          ///< For struct guild.
	HPDT_PARTY,          ///< For struct party_data.
	HPDT_MOBDB,          ///< For struct mob_db.
	HPDT_MOBDATA,        ///< For struct mob_data.
	HPDT_ITEMDATA,       ///< For struct item_data.
	HPDT_BGDATA,         ///< For struct battleground_data.
	HPDT_AUTOTRADE_VEND, ///< For struct autotrade_vending.
	HPDT_CLAN,           ///< For struct clan.
	HPDT_UNIT_PARAMETER, ///< For struct unit_parameters_db.
};

/* used in macros and conf storage */
enum HPluginConfType {
	HPCT_BATTLE,     ///< battle-conf (map-server)
	HPCT_LOGIN,      ///< login-server.conf (login-server)
	HPCT_CHAR,       ///< char-server.conf (char-server)
	HPCT_CHAR_INTER, ///< inter-server.conf (char-server)
	HPCT_MAP_INTER,  ///< inter-server.conf (map-server)
	HPCT_LOG,        ///< logs.conf (map-server)
	HPCT_SCRIPT,     ///< script.conf (map-server)
	HPCT_MAX,
};

/* HPData handy redirects */
/* session[] */
#define addToSession(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_SESSION, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromSession(ptr, classid) (HPMi->getFromHPData(HPDT_SESSION, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromSession(ptr, classid) \
	(HPMi->removeFromHPData(HPDT_SESSION, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* map_session_data */
#define addToMSD(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_MSD, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromMSD(ptr, classid)    (HPMi->getFromHPData(HPDT_MSD, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromMSD(ptr, classid) (HPMi->removeFromHPData(HPDT_MSD, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* npc_data */
#define addToNPCD(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_NPCD, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromNPCD(ptr, classid)    (HPMi->getFromHPData(HPDT_NPCD, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromNPCD(ptr, classid) (HPMi->removeFromHPData(HPDT_NPCD, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* map_data */
#define addToMAPD(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_MAP, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromMAPD(ptr, classid)    (HPMi->getFromHPData(HPDT_MAP, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromMAPD(ptr, classid) (HPMi->removeFromHPData(HPDT_MAP, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* party_data */
#define addToPAD(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_PARTY, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromPAD(ptr, classid)    (HPMi->getFromHPData(HPDT_PARTY, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromPAD(ptr, classid) (HPMi->removeFromHPData(HPDT_PARTY, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* guild */
#define addToGLD(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_GUILD, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromGLD(ptr, classid)    (HPMi->getFromHPData(HPDT_GUILD, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromGLD(ptr, classid) (HPMi->removeFromHPData(HPDT_GUILD, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* instance_data */
#define addToINSTD(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_INSTANCE, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromINSTD(ptr, classid) (HPMi->getFromHPData(HPDT_INSTANCE, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromINSTD(ptr, classid) \
	(HPMi->removeFromHPData(HPDT_INSTANCE, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* mob_db */
#define addToMOBDB(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_MOBDB, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromMOBDB(ptr, classid)    (HPMi->getFromHPData(HPDT_MOBDB, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromMOBDB(ptr, classid) (HPMi->removeFromHPData(HPDT_MOBDB, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* mob_data */
#define addToMOBDATA(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_MOBDATA, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromMOBDATA(ptr, classid) (HPMi->getFromHPData(HPDT_MOBDATA, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromMOBDATA(ptr, classid) \
	(HPMi->removeFromHPData(HPDT_MOBDATA, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* item_data */
#define addToITEMDATA(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_ITEMDATA, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromITEMDATA(ptr, classid) (HPMi->getFromHPData(HPDT_ITEMDATA, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromITEMDATA(ptr, classid) \
	(HPMi->removeFromHPData(HPDT_ITEMDATA, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* battleground_data */
#define addToBGDATA(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_BGDATA, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromBGDATA(ptr, classid)    (HPMi->getFromHPData(HPDT_BGDATA, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromBGDATA(ptr, classid) (HPMi->removeFromHPData(HPDT_BGDATA, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* autotrade_vending */
#define addToATVEND(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_AUTOTRADE_VEND, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getFromATVEND(ptr, classid) \
	(HPMi->getFromHPData(HPDT_AUTOTRADE_VEND, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removeFromATVEND(ptr, classid) \
	(HPMi->removeFromHPData(HPDT_AUTOTRADE_VEND, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* clan */
#define addtoCLAN(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_CLAN, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getfromCLAN(ptr, classid)    (HPMi->getFromHPData(HPDT_CLAN, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removefromCLAN(ptr, classid) (HPMi->removeFromHPData(HPDT_CLAN, hpm_plugin->pid(), (ptr)->hdata, (classid)))
/* unit parameters */
#define addToUnitParam(ptr, data, classid, autofree) \
	(HPMi->addToHPData(HPDT_UNIT_PARAMETER, hpm_plugin->pid(), &(ptr)->hdata, (data), (classid), (autofree)))
#define getfromUnitParam(ptr, classid) \
	(HPMi->getFromHPData(HPDT_UNIT_PARAMETER, hpm_plugin->pid(), (ptr)->hdata, (classid)))
#define removefromUnitParam(ptr, classid) \
	(HPMi->removeFromHPData(HPDT_UNIT_PARAMETER, hpm_plugin->pid(), (ptr)->hdata, (classid)))

/// HPMi->addScript
#define addScriptCommand(cname, scinfo, funcname) \
	do { \
		if (HPMi->addScript != NULL) { \
			HPMi->addScript(cname, scinfo, buildin_##funcname, false); \
		} else { \
			ShowWarning("HPM (%s):addScriptCommand(\"%s\",\"%s\",%s) failed, addScript sub is NULL!\n", \
			            hpm_plugin->name, cname, scinfo, #funcname); \
		} \
	} while (0)
#define addScriptCommandDeprecated(cname, scinfo, funcname) \
	do { \
		if (HPMi->addScript != NULL) { \
			HPMi->addScript(cname, scinfo, buildin_##funcname, true); \
		} else { \
			ShowWarning("HPM (%s):addScriptCommandDeprecated(\"%s\",\"%s\",%s) failed, addScript sub is NULL!\n", \
			            hpm_plugin->name, cname, scinfo, #funcname); \
		} \
	} while (0)
/// HPMi->addCPCommand
#define addCPCommand(cname, funcname) \
	do { \
		if (HPMi->addCPCommand != NULL) { \
			HPMi->addCPCommand(cname, console_parse_##funcname); \
		} else { \
			ShowWarning("HPM (%s):addCPCommand(\"%s\",%s) failed, addCPCommand sub is NULL!\n", hpm_plugin->name, \
			            cname, #funcname); \
		} \
	} while (0)
/* HPMi->addPacket */
#define addPacket(cmd, len, receive, point) HPMi->addPacket(cmd, len, receive, point, hpm_plugin->pid())
/* HPMi->addBattleConf */
#define addBattleConf(bcname, funcname, returnfunc, required) \
	HPMi->addConf(hpm_plugin->pid(), HPCT_BATTLE, bcname, funcname, returnfunc, required)
/* HPMi->addLogin */
#define addLoginConf(bcname, funcname) HPMi->addConf(hpm_plugin->pid(), HPCT_LOGIN, bcname, funcname, NULL, false)
/* HPMi->addChar */
#define addCharConf(bcname, funcname)  HPMi->addConf(hpm_plugin->pid(), HPCT_CHAR, bcname, funcname, NULL, false)
/* HPMi->addCharInter */
#define addCharInterConf(bcname, funcname) \
	HPMi->addConf(hpm_plugin->pid(), HPCT_CHAR_INTER, bcname, funcname, NULL, false)
/* HPMi->addMapInter */
#define addMapInterConf(bcname, funcname) \
	HPMi->addConf(hpm_plugin->pid(), HPCT_MAP_INTER, bcname, funcname, NULL, false)
/* HPMi->addLog */
#define addLogConf(bcname, funcname)    HPMi->addConf(hpm_plugin->pid(), HPCT_LOG, bcname, funcname, NULL, false)
/* HPMi->addScript */
#define addScriptConf(bcname, funcname) HPMi->addConf(hpm_plugin->pid(), HPCT_SCRIPT, bcname, funcname, NULL, false)

/* HPMi->addPCGPermission */
#define addGroupPermission(pcgname, maskptr) HPMi->addPCGPermission(hpm_plugin->pid(), pcgname, &maskptr)

#define addProxyPacket(cmd, structname, receive, point) \
	addPacket(cmd, WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_##structname##_data), receive, point)

/* Hercules Plugin Mananger Include Interface */
struct HPMi_interface {
	bool (*addCommand)(const char *name, bool (*func)(const int fd, struct map_session_data *sd, const char *command,
	                                                  const char *message, struct AtCommandInfo *info));
	bool (*addScript)(const char *name, const char *args, bool (*func)(struct script_state *st), bool isDeprecated);
	void (*addCPCommand)(const char *name, CParseFunc func);
	/* HPM Custom Data */
	void (*addToHPData)(enum HPluginDataTypes type, uint32 pluginID, struct hplugin_data_store **storeptr, void *data,
	                    uint32 classid, bool autofree);
	void *(*getFromHPData)(enum HPluginDataTypes type, uint32 pluginID, struct hplugin_data_store *store,
	                       uint32 classid);
	void (*removeFromHPData)(enum HPluginDataTypes type, uint32 pluginID, struct hplugin_data_store *store,
	                         uint32 classid);
	/* packet */
	bool (*addPacket)(unsigned short cmd, int length, void (*receive)(int fd), unsigned int point,
	                  unsigned int pluginID);
	/* program --arg/-a */
	bool (*addArg)(unsigned int pluginID, const char *name, bool has_param, CmdlineExecFunc func, const char *help);
	/* battle-config recv param */
	bool (*addConf)(unsigned int pluginID, enum HPluginConfType type, const char *name,
	                void (*parse_func)(const char *key, const char *val), int (*return_func)(const char *key),
	                bool required);
	/* pc group permission */
	void (*addPCGPermission)(unsigned int pluginID, const char *name, unsigned int *mask);

	struct Sql *sql_handle;

	/* Hooking */
	struct HPMHooking_interface *hooking;
	struct malloc_interface *memmgr;
};
HERCAPI_COMMON_EXTERN struct HPMi_interface *HPMi;

class hpm_plugin_i
{
  public:
	hpm_plugin_i(const char *name, enum server_types type, const char *version, const char *req_version,
	             bool has_hpmhooking, const struct s_HPMDataCheck *HPMDataCheck, unsigned int HPMDataCheckLen,
	             int HPMDataCheckVer, const struct HPMi_interface *a_HPMi)
	    : name(name),
	      type(type),
	      version(version),
	      req_version(req_version),
	      has_hpmhooking(has_hpmhooking),
	      HPMDataCheck(HPMDataCheck),
	      HPMDataCheckLen(HPMDataCheckLen),
	      HPMDataCheckVer(HPMDataCheckVer),
	      m_HPMi(a_HPMi)
	{
	}

	virtual void init()              = 0;
	virtual void final()             = 0;
	virtual void server_online()     = 0;
	virtual void server_post_final() = 0;
	virtual void server_preinit()    = 0;

	void set_pid(int p)
	{
		m_pid = p;
	}

	int pid() const
	{
		return m_pid;
	}

	bool add_atcommand(const char *cname, bool (*func)(const int fd, struct map_session_data *sd, const char *command,
	                                                   const char *message, struct AtCommandInfo *info))
	{
		nullpo_retr(false, HPMi->addCommand);
		return HPMi->addCommand(cname, func);
	}

	/* program --arg/-a */
	bool add_cmdline_arg(const char *arg_name, bool has_param, CmdlineExecFunc func, const char *help)
	{
		return m_HPMi->addArg(m_pid, arg_name, has_param, func, help);
	}

	const char *name;
	const server_types type;
	const char *version;
	const char *req_version;
	const bool has_hpmhooking;
	const struct s_HPMDataCheck *HPMDataCheck;
	const unsigned int HPMDataCheckLen;
	const int HPMDataCheckVer;

  private:
	int m_pid;
	const struct HPMi_interface *m_HPMi;
};

#if defined(HERCULES_PLUGIN)
static_assert(
  1
    == 0
  #ifdef HERCULES_PLUGIN_API
         + 1
  #endif
  #ifdef HERCULES_PLUGIN_CHAR
         + 1
  #endif
  #ifdef HERCULES_PLUGIN_LOGIN
         + 1
  #endif
  #ifdef HERCULES_PLUGIN_MAP
         + 1
  #endif
  ,
  "Invalid multi-server plugin configuration. Each plugin can support only one of api, char, login, map at a time.");

  #if defined(HERCULES_PLUGIN_API)
	#define HPM_SERVER_TYPE SERVER_TYPE_API
  #elif defined(HERCULES_PLUGIN_CHAR)
	#define HPM_SERVER_TYPE SERVER_TYPE_CHAR
  #elif defined(HERCULES_PLUGIN_LOGIN)
	#define HPM_SERVER_TYPE SERVER_TYPE_LOGIN
  #elif defined(HERCULES_PLUGIN_MAP)
	#define HPM_SERVER_TYPE SERVER_TYPE_MAP
  #else
	#define HPM_SERVER_TYPE SERVER_TYPE_UNKNOWN
  #endif

  #define HPM_PLUGIN_DEFS_BASE

  #define HPM_PLUGIN_DEFS_ALL HPM_PLUGIN_DEFS_BASE

  #define HPM_PLUGIN_HPMHOOKING_SUPPORT false

  #define HPM_INITIALIZE_PLUGIN(plugin_name, plugin_version) \
	  hpm_plugin_i((plugin_name), HPM_SERVER_TYPE, (plugin_version), HPM_VERSION, HPM_PLUGIN_HPMHOOKING_SUPPORT, \
		           HPM_PLUGIN_DEFS_HPMDATACHECK, HPMi)
  #define HPM_DECLARE_PLUGIN_BASE(p) \
	  namespace \
	  { \
	  auto hpm_plugin_s = p; \
	  } \
	  HPM_EXPORT hpm_plugin_i *hpm_plugin = &hpm_plugin_s;

#endif

#endif /* COMMON_HPMI_H */
