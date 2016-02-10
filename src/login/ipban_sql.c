/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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

#include "ipban.h"

#include "login/login.h"
#include "login/loginlog.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdlib.h>

// Sql settings
static char   ipban_db_hostname[32] = "127.0.0.1";
static uint16 ipban_db_port = 3306;
static char   ipban_db_username[32] = "ragnarok";
static char   ipban_db_password[100] = "ragnarok";
static char   ipban_db_database[32] = "ragnarok";
static char   ipban_codepage[32] = "";
static char   ipban_table[32] = "ipbanlist";

// globals
static struct Sql *sql_handle = NULL;
static int cleanup_timer_id = INVALID_TIMER;
static bool ipban_inited = false;

int ipban_cleanup(int tid, int64 tick, int id, intptr_t data);


// initialize
void ipban_init(void)
{
	ipban_inited = true;

	if (!login->config->ipban)
		return;// ipban disabled

	// establish connections
	sql_handle = SQL->Malloc();
	if (SQL_ERROR == SQL->Connect(sql_handle, ipban_db_username, ipban_db_password,
	                              ipban_db_hostname, ipban_db_port, ipban_db_database)) {
		Sql_ShowDebug(sql_handle);
		SQL->Free(sql_handle);
		exit(EXIT_FAILURE);
	}
	if (ipban_codepage[0] != '\0' && SQL_ERROR == SQL->SetEncoding(sql_handle, ipban_codepage))
		Sql_ShowDebug(sql_handle);

	if (login->config->ipban_cleanup_interval > 0) {
		// set up periodic cleanup of connection history and active bans
		timer->add_func_list(ipban_cleanup, "ipban_cleanup");
		cleanup_timer_id = timer->add_interval(timer->gettick()+10, ipban_cleanup, 0, 0, login->config->ipban_cleanup_interval*1000);
	} else {
		// make sure it gets cleaned up on login-server start regardless of interval-based cleanups
		ipban_cleanup(0,0,0,0);
	}
}

// finalize
void ipban_final(void)
{
	if (!login->config->ipban)
		return;// ipban disabled

	if (login->config->ipban_cleanup_interval > 0)
		// release data
		timer->delete(cleanup_timer_id, ipban_cleanup);

	ipban_cleanup(0,0,0,0); // always clean up on login-server stop

	// close connections
	SQL->Free(sql_handle);
	sql_handle = NULL;
}

/**
 * Reads 'inter_configuration' and initializes required variables/Sets global
 * configuration.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.

 */
bool ipban_config_read_inter(const char *filename, bool imported)
{
	struct config_t config;
	struct config_setting_t *setting = NULL;
	const char *import = NULL;
	bool retval = true;

	nullpo_retr(false, filename);

	if (!libconfig->load_file(&config, filename))
		return false; // Error message is already shown by libconfig->read_file

	if ((setting = libconfig->lookup(&config, "inter_configuration/database_names")) == NULL) {
		libconfig->destroy(&config);
		if (imported)
			return true;
		ShowError("ipban_config_read: inter_configuration/database_names was not found!\n");
		return false;
	}
	libconfig->setting_lookup_mutable_string(setting, "ipban_table", ipban_table, sizeof(ipban_table));

	// import should overwrite any previous configuration, so it should be called last
	if (libconfig->lookup_string(&config, "import", &import) == CONFIG_TRUE) {
		if (strcmp(import, filename) == 0 || strcmp(import, "conf/common/inter-server.conf") == 0) {
			ShowWarning("ipban_config_read_inter: Loop detected! Skipping 'import'...\n");
		} else {
			if (!ipban_config_read_inter(import, true))
				retval = false;
		}
	}

	libconfig->destroy(&config);
	return retval;
}

/**
 * Reads login_configuration/account/ipban/sql_connection and loads configuration options.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
bool ipban_config_read_connection(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "login_configuration/account/ipban/sql_connection")) == NULL) {
		if (imported)
			return true;
		ShowError("account_db_sql_set_property: login_configuration/account/ipban/sql_connection was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_mutable_string(setting, "db_hostname", ipban_db_hostname, sizeof(ipban_db_hostname));
	libconfig->setting_lookup_mutable_string(setting, "db_database", ipban_db_database, sizeof(ipban_db_database));

	libconfig->setting_lookup_mutable_string(setting, "db_username", ipban_db_username, sizeof(ipban_db_username));
	libconfig->setting_lookup_mutable_string(setting, "db_password", ipban_db_password, sizeof(ipban_db_password));
	libconfig->setting_lookup_mutable_string(setting, "codepage", ipban_codepage, sizeof(ipban_codepage));
	libconfig->setting_lookup_uint16(setting, "db_port", &ipban_db_port);

	return true;
}

/**
 * Reads login_configuration/account/ipban/dynamic_pass_failure and loads configuration options.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
bool ipban_config_read_dynamic(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "login_configuration/account/ipban/dynamic_pass_failure")) == NULL) {
		if (imported)
			return true;
		ShowError("account_db_sql_set_property: login_configuration/account/ipban/dynamic_pass_failure was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_bool_real(setting, "enabled", &login->config->dynamic_pass_failure_ban);
	libconfig->setting_lookup_uint32(setting, "ban_interval", &login->config->dynamic_pass_failure_ban_interval);
	libconfig->setting_lookup_uint32(setting, "ban_limit", &login->config->dynamic_pass_failure_ban_limit);
	libconfig->setting_lookup_uint32(setting, "ban_duration", &login->config->dynamic_pass_failure_ban_duration);

	return true;
}

/**
 * Reads login_configuration.account.ipban and loads configuration options.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
bool ipban_config_read(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;
	bool retval = true;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if (ipban_inited)
		return false; // settings can only be changed before init

	if ((setting = libconfig->lookup(config, "login_configuration/account/ipban")) == NULL) {
		if (!imported)
			ShowError("login_config_read: login_configuration/log was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_bool_real(setting, "enabled", &login->config->ipban);
	libconfig->setting_lookup_uint32(setting, "cleanup_interval", &login->config->ipban_cleanup_interval);

	if (!ipban_config_read_inter("conf/common/inter-server.conf", imported))
		retval = false;
	if (!ipban_config_read_connection(filename, config, imported))
		retval = false;
	if (!ipban_config_read_dynamic(filename, config, imported))
		retval = false;

	return retval;
}

// check ip against active bans list
bool ipban_check(uint32 ip)
{
	uint8* p = (uint8*)&ip;
	char* data = NULL;
	int matches;

	if (!login->config->ipban)
		return false;// ipban disabled

	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `rtime` > NOW() AND (`list` = '%u.*.*.*' OR `list` = '%u.%u.*.*' OR `list` = '%u.%u.%u.*' OR `list` = '%u.%u.%u.%u')",
		ipban_table, p[3], p[3], p[2], p[3], p[2], p[1], p[3], p[2], p[1], p[0]) )
	{
		Sql_ShowDebug(sql_handle);
		// close connection because we can't verify their connectivity.
		return true;
	}

	if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
		return false;

	SQL->GetData(sql_handle, 0, &data, NULL);
	matches = atoi(data);
	SQL->FreeResult(sql_handle);

	return( matches > 0 );
}

// log failed attempt
void ipban_log(uint32 ip)
{
	unsigned long failures;

	if (!login->config->ipban)
		return;// ipban disabled

	failures = loginlog_failedattempts(ip, login->config->dynamic_pass_failure_ban_interval);// how many times failed account? in one ip.

	// if over the limit, add a temporary ban entry
	if (failures >= login->config->dynamic_pass_failure_ban_limit)
	{
		uint8* p = (uint8*)&ip;
		if (SQL_ERROR == SQL->Query(sql_handle, "INSERT INTO `%s`(`list`,`btime`,`rtime`,`reason`) VALUES ('%u.%u.%u.*', NOW() , NOW() +  INTERVAL %u MINUTE ,'Password error ban')",
			ipban_table, p[3], p[2], p[1], login->config->dynamic_pass_failure_ban_duration))
		{
			Sql_ShowDebug(sql_handle);
		}
	}
}

// remove expired bans
int ipban_cleanup(int tid, int64 tick, int id, intptr_t data) {
	if (!login->config->ipban)
		return 0;// ipban disabled

	if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `rtime` <= NOW()", ipban_table) )
		Sql_ShowDebug(sql_handle);

	return 0;
}
