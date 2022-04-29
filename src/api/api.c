/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "config/core.h" // CELL_NOSTACK, CIRCULAR_AREA, CONSOLE_INPUT, DBPATH, RENEWAL
#include "api.h"

#include "api/HPMapi.h"
#include "api/aclif.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/console.h"
#include "common/core.h"
#include "common/ers.h"
#include "common/grfio.h"
#include "common/md5calc.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"
#include "api/aloginif.h"
#include "api/handlers.h"
#include "api/httpparser.h"
#include "api/httpsender.h"
#include "api/jsonparser.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

static struct api_interface api_s;

struct api_interface *api;


int do_final(void)
{
	HPM->event(HPET_FINAL);

	aclif->final();

	HPM_api_do_final();
	aFree(api->NET_CONF_NAME);

	HPM->event(HPET_POST_FINAL);

	ShowStatus("Finished.\n");
	return api->retval;
}

//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
	static int run = 0;
	//Save all characters and then flush the inter-connection.
	if (run) {
		ShowFatalError("Server has crashed while trying to save characters. Character data can't be saved!\n");
		return;
	}
	run = 1;
	ShowError("Server received crash signal! Attempting to save all online characters!\n");
}

void set_server_type(void)
{
	SERVER_TYPE = SERVER_TYPE_API;
}

/// Called when a terminate signal is received.
static void do_shutdown(void)
{
	if (core->runflag != APISERVER_ST_SHUTDOWN)
	{
		core->runflag = APISERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		sockt->flush_fifos();
		core->runflag = CORE_ST_STOP;
	}
}

static void api_load_defaults(void)
{
	api_defaults();
	aclif_defaults();
	aloginif_defaults();
	httpparser_defaults();
	httpsender_defaults();
	jsonparser_defaults();
	jsonwriter_defaults();
}

/**
 * --api-config handler
 *
 * Overrides the default api configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(apiconfig)
{
	aFree(api->API_CONF_NAME);
	api->API_CONF_NAME = aStrdup(params);
	return true;
}

/**
 * --run-once handler
 *
 * Causes the server to run its loop once, and shutdown. Useful for testing.
 * @see cmdline->exec
 */
static CMDLINEARG(runonce)
{
	core->runflag = CORE_ST_STOP;
	return true;
}

/**
 * --net-config handler
 *
 * Overrides the default subnet configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(netconfig)
{
	aFree(api->NET_CONF_NAME);
	api->NET_CONF_NAME = aStrdup(params);
	return true;
}

/**
 * Defines the local command line arguments
 */
void cmdline_args_init_local(void)
{
	CMDLINEARG_DEF2(run-once, runonce, "Closes server after loading (testing).", CMDLINE_OPT_NORMAL);
	CMDLINEARG_DEF2(api-config, apiconfig, "Alternative api-server configuration.", CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(net-config, netconfig, "Alternative subnet configuration.", CMDLINE_OPT_PARAM);
}

static int api_check_connect_login_server(int tid, int64 tick, int id, intptr_t data)
{
	if (aloginif->fd > 0 && sockt->session[aloginif->fd] != NULL)
		return 0;

	ShowInfo("Attempt to connect to login-server...\n");

	if ((aloginif->fd = sockt->make_connection(aloginif->ip, aloginif->port, NULL)) == -1) { //Try again later. [Skotlex]
		aloginif->fd = 0;
		return 0;
	}

	sockt->session[aloginif->fd]->func_parse = aloginif->parse;
	sockt->session[aloginif->fd]->flag.server = 1;
	sockt->session[aloginif->fd]->flag.validate = 0;
	sockt->realloc_fifo(aloginif->fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

	aloginif->connect_to_server();

	return 1;
}

/**
 * Reads 'api_configuration/console' and initializes required variables.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
static bool api_config_read_console(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "api_configuration/console")) == NULL) {
		if (imported)
			return true;
		ShowError("api_config_read: api_configuration/console was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_bool_real(setting, "stdout_with_ansisequence", &showmsg->stdout_with_ansisequence);
	if (libconfig->setting_lookup_int(setting, "console_silent", &showmsg->silent) == CONFIG_TRUE) {
		if (showmsg->silent) // only bother if its actually enabled
			ShowInfo("Console Silent Setting: %d\n", showmsg->silent);
	}
	libconfig->setting_lookup_mutable_string(setting, "timestamp_format", showmsg->timestamp_format, sizeof(showmsg->timestamp_format));
	libconfig->setting_lookup_int(setting, "console_msg_log", &showmsg->console_log);

	return true;
}

/**
 * Reads 'api_configuration/sql_connection' and initializes required variables.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
static bool api_config_read_connection(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "api_configuration/sql_connection")) == NULL) {
		if (imported)
			return true;
		ShowError("api_config_read: api_configuration/sql_connection was not found in %s!\n", filename);
		ShowWarning("api_config_read_connection: Defaulting sql_connection...\n");
		return false;
	}

	libconfig->setting_lookup_int(setting, "db_port", &api->server_port);
	libconfig->setting_lookup_mutable_string(setting, "db_hostname", api->server_ip, sizeof(api->server_ip));
	libconfig->setting_lookup_mutable_string(setting, "db_username", api->server_id, sizeof(api->server_id));
	libconfig->setting_lookup_mutable_string(setting, "db_password", api->server_pw, sizeof(api->server_pw));
	libconfig->setting_lookup_mutable_string(setting, "db_database", api->server_db, sizeof(api->server_db));
	libconfig->setting_lookup_mutable_string(setting, "default_codepage", api->default_codepage, sizeof(api->default_codepage));

	return true;
}

/**
 * Reads 'api_configuration/inter' and initializes required variables.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
static bool api_config_read_inter(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;
	const char *str = NULL;
	char temp[24];
	uint16 port;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "api_configuration/inter")) == NULL) {
		if (imported)
			return true;
		ShowError("api_config_read: api_configuration/inter was not found in %s!\n", filename);
		return false;
	}

	// Login information
	if (libconfig->setting_lookup_mutable_string(setting, "userid", temp, sizeof(temp)) == CONFIG_TRUE) {
		aloginif->setuserid(temp);
	}
	if (libconfig->setting_lookup_mutable_string(setting, "passwd", temp, sizeof(temp)) == CONFIG_TRUE) {
		aloginif->setpasswd(temp);
	}

	// login, and api-server information
	if (libconfig->setting_lookup_string(setting, "login_ip", &str) == CONFIG_TRUE)
		api->login_ip_set = aloginif->setip(str);
	if (libconfig->setting_lookup_uint16(setting, "login_port", &port) == CONFIG_TRUE)
		aloginif->setport(port);

	if (libconfig->setting_lookup_string(setting, "api_ip", &str) == CONFIG_TRUE)
		api->ip_set = aclif->setip(str);
	if (libconfig->setting_lookup_uint16(setting, "api_port", &port) == CONFIG_TRUE) {
		aclif->setport(port);
		api->port = port;
	}
	if (libconfig->setting_lookup_string(setting, "bind_ip", &str) == CONFIG_TRUE)
		aclif->setbindip(str);

	if (!api->login_ip_set) {
		char ip_str[16];
		sockt->ip2str(sockt->addr_[0], ip_str);
		ShowInfo("Login ip not set, defaulting to %s as IP address\n", ip_str);

		aloginif->setip(ip_str);
	}

	return true;
}

/**
 * Reads api-server configuration files (api-server.conf) and initialises
 * required variables.
 *
 * @param filename Path to configuration file.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
static bool api_config_read(const char *filename, bool imported)
{
	struct config_t config;
	struct config_setting_t *setting = NULL;
	const char *import = NULL;
	bool retval = true;

	nullpo_retr(false, filename);

	if (!libconfig->load_file(&config, filename))
		return false;

	if ((setting = libconfig->lookup(&config, "api_configuration")) == NULL) {
		libconfig->destroy(&config);
		if (imported)
			return true;
		ShowError("api_config_read: api_configuration was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_int(setting, "remove_disconnected_delay", &aclif->remove_disconnected_delay);
	libconfig->setting_lookup_int(setting, "ip_connections_limit", &api->ip_connections_limit);

	if (!api_config_read_console(filename, &config, imported))
		retval = false;
	if (!api_config_read_connection(filename, &config, imported))
		retval = false;
	if (!api_config_read_inter(filename, &config, imported))
		retval = false;

	// import should overwrite any previous configuration, so it should be called last
	if (libconfig->lookup_string(&config, "import", &import) == CONFIG_TRUE) {
		if (strcmp(import, filename) == 0 || strcmp(import, api->API_CONF_NAME) == 0) {
			ShowWarning("api_config_read: Loop detected! Skipping 'import'...\n");
		} else {
			if (!api->config_read(import, true))
				retval = false;
		}
	}

	libconfig->destroy(&config);
	return retval;
}

int do_init(int argc, char *argv[])
{
	bool minimal = false;

#ifdef GCOLLECT
	GC_enable_incremental();
#endif

	api_load_defaults();
	handlers_defaults();

	api->API_CONF_NAME = aStrdup("conf/api/api-server.conf");
	api->NET_CONF_NAME = aStrdup("conf/api_network.conf");

	HPM_api_do_init();
	cmdline->exec(argc, argv, CMDLINE_OPT_PREINIT);
	HPM->config_read();

	HPM->event(HPET_PRE_INIT);

	sockt->net_config_read(api->NET_CONF_NAME);

	cmdline->exec(argc, argv, CMDLINE_OPT_NORMAL);
	minimal = api->minimal;
	if (!minimal) {
		aloginif->checkdefaultlogin();

		api->config_read(api->API_CONF_NAME, false);
		if (!api->ip_set) {
			char ip_str[16];
			sockt->ip2str(sockt->addr_[0], ip_str);

			ShowInfo("Defaulting to %s as our IP address\n", ip_str);

			if (!api->ip_set)
				aclif->setip(ip_str);
		}
	}

	HPM->event(HPET_INIT);

	handlers->init(minimal);
	aclif->init(minimal);
	httpparser->init(minimal);

	if( minimal ) {
		HPM->event(HPET_READY);
		exit(EXIT_SUCCESS);
	}

	aloginif->init(minimal);

	Sql_HerculesUpdateCheck(api->mysql_handle);

	ShowStatus("Server is '"CL_GREEN"ready"CL_RESET"' and listening on port '"CL_WHITE"%d"CL_RESET"'.\n\n", api->port);

	if( core->runflag != CORE_ST_STOP ) {
		core->shutdown_callback = api->do_shutdown;
		core->runflag = APISERVER_ST_RUNNING;
	}

	HPM->event(HPET_READY);

	return 0;
}

/*=====================================
 * Default Functions : api.h
 * Generated by HerculesInterfaceMaker
 * created by Susu
 *-------------------------------------*/
void api_defaults(void)
{
	api = &api_s;

	/* */
	api->minimal = false;
	api->retval = EXIT_SUCCESS;

	api->server_port = 3306;
	sprintf(api->server_ip,"127.0.0.1");
	sprintf(api->server_id,"ragnarok");
	sprintf(api->server_pw,"ragnarok");
	sprintf(api->server_db,"ragnarok");
	api->mysql_handle = NULL;
	api->ip_connections_limit = 5;

	api->port = 7121;
	api->ip_set = 0;
	api->login_ip_set = 0;
	api->char_ip_set = 0;

	api->do_shutdown = do_shutdown;
	api->config_read = api_config_read;
	api->config_read_console = api_config_read_console;
	api->config_read_connection = api_config_read_connection;
	api->config_read_inter = api_config_read_inter;
	api->check_connect_login_server = api_check_connect_login_server;
}
