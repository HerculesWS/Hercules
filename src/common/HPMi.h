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
#include "common/socket.h"

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

class hpm_plugin_i;

#define addProxyPacket(plugin, cmd, structname, receive, point) \
	(plugin)->add_packet<point>((cmd), WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_##structname##_data), (receive))

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
	bool (*addPacket)(unsigned short cmd, int length, void (*receive)(hpm_plugin_i *self, int fd), unsigned int point,
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
	hpm_plugin_i(const char *a_name, enum server_types a_type, const char *a_version, const char *the_req_version,
	             bool with_hpmhooking, const struct s_HPMDataCheck *the_HPMDataCheck, unsigned int the_HPMDataCheckLen,
	             int the_HPMDataCheckVer, const struct HPMi_interface *the_HPMi)
	    : name(a_name),
	      type(a_type),
	      version(a_version),
	      req_version(the_req_version),
	      has_hpmhooking(with_hpmhooking),
	      HPMDataCheck(the_HPMDataCheck),
	      HPMDataCheckLen(the_HPMDataCheckLen),
	      HPMDataCheckVer(the_HPMDataCheckVer),
	      m_HPMi(the_HPMi)
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

	void add_console_command(const char *cname, CParseFunc func)
	{
		nullpo_retv(HPMi->addCPCommand);
		HPMi->addCPCommand(cname, func);
	}

	template<HPluginPacketHookingPoints point>
	bool add_packet(unsigned short cmd, int length, void (*receive)(hpm_plugin_i *self, int fd))
	{
#if defined(HERCULES_PLUGIN_API)
		static_assert(point == hpParse_LoginApi, "Invalid packet type for api server plugins");
#elif defined(HERCULES_PLUGIN_CHAR)
		static_assert(
		  point == hpParse_FromMap || point == hpParse_FromLogin || point == hpParse_Char || point == hpProxy_ApiChar,
		  "Invalid packet type for char server plugins");
#elif defined(HERCULES_PLUGIN_LOGIN)
		static_assert(
		  point == hpParse_FromChar || point == hpParse_Login || point == hpParse_ApiLogin || point == hpProxy_ApiLogin,
		  "Invalid packet type for login server plugins");
#elif defined(HERCULES_PLUGIN_MAP)
		static_assert(point == hpClif_Parse || point == hpChrif_Parse || point == hpProxy_ApiMap,
		              "Invalid packet type for map server plugins");
#else
		static_assert(false, "This method can only be used in a plugin");
#endif
		return HPMi->addPacket(cmd, length, receive, point, m_pid);
	}

	/* program --arg/-a */
	bool add_cmdline_arg(const char *arg_name, bool has_param, CmdlineExecFunc func, const char *help)
	{
		return m_HPMi->addArg(m_pid, arg_name, has_param, func, help);
	}

	bool add_conf(HPluginConfType ctype, const char *cname, void (*parse_func)(const char *key, const char *val),
	              int (*return_func)(const char *key), bool required)
	{
		return HPMi->addConf(m_pid, ctype, cname, parse_func, return_func, required);
	}

	bool add_log_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_LOG, bcname, parse_func, nullptr, false);
	}

	/* session[] */
	template<class T>
	void add_to_session(struct socket_data *session, T *data, uint32 classid, bool autofree)
	{
		HPMi->addToHPData(HPDT_SESSION, m_pid, &session->hdata, data, classid, autofree);
	}

	template<class T>
	T *get_from_session(struct socket_data *session, uint32 classid)
	{
		return (T *)HPMi->getFromHPData(HPDT_SESSION, m_pid, session->hdata, classid);
	}

	void remove_from_session(struct socket_data *session, uint32 classid)
	{
		HPMi->removeFromHPData(HPDT_SESSION, m_pid, session->hdata, classid);
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
	#include "api/HPMi_api.h"
using hpm_plugin_t = hpm_plugin_iapi;
  #elif defined(HERCULES_PLUGIN_CHAR)
	#define HPM_SERVER_TYPE SERVER_TYPE_CHAR
	#include "char/HPMi_char.h"
using hpm_plugin_t = hpm_plugin_ichar;
  #elif defined(HERCULES_PLUGIN_LOGIN)
	#include "login/HPMi_login.h"
	#define HPM_SERVER_TYPE SERVER_TYPE_LOGIN
using hpm_plugin_t = hpm_plugin_ilogin;
  #elif defined(HERCULES_PLUGIN_MAP)
	#define HPM_SERVER_TYPE SERVER_TYPE_MAP
	#include "map/HPMi_map.h"
using hpm_plugin_t = hpm_plugin_imap;
  #else
	#define HPM_SERVER_TYPE SERVER_TYPE_UNKNOWN
using hpm_plugin_t = hpm_plugin_i;
  #endif

  #define HPM_PLUGIN_DEFS_BASE

  #define HPM_PLUGIN_DEFS_ALL HPM_PLUGIN_DEFS_BASE

  #define HPM_PLUGIN_HPMHOOKING_SUPPORT false

  #define HPM_INITIALIZE_PLUGIN(plugin_name, plugin_version) \
	  hpm_plugin_t((plugin_name), HPM_SERVER_TYPE, (plugin_version), HPM_VERSION, HPM_PLUGIN_HPMHOOKING_SUPPORT, \
		           HPM_PLUGIN_DEFS_HPMDATACHECK, HPMi)
  #define HPM_DECLARE_PLUGIN_BASE(p) \
	  namespace \
	  { \
	  auto hpm_plugin_s = p; \
	  } \
	  HPM_EXPORT hpm_plugin_t *hpm_plugin; \
	  hpm_plugin_t *hpm_plugin = &hpm_plugin_s;

#endif

#endif /* COMMON_HPMI_H */
