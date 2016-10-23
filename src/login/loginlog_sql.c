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

#include "loginlog.h"

#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"

#include <stdlib.h> // exit

// Sql settings
static char   log_db_hostname[32] = "127.0.0.1";
static uint16 log_db_port = 3306;
static char   log_db_username[32] = "ragnarok";
static char   log_db_password[100] = "ragnarok";
static char   log_db_database[32] = "ragnarok";
static char   log_codepage[32] = "";
static char   log_login_db[256] = "loginlog";

static struct Sql *sql_handle = NULL;
static bool enabled = false;


// Returns the number of failed login attempts by the ip in the last minutes.
unsigned long loginlog_failedattempts(uint32 ip, unsigned int minutes)
{
	unsigned long failures = 0;

	if( !enabled )
		return 0;

	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT count(*) FROM `%s` WHERE `ip` = '%s' AND `rcode` = '1' AND `time` > NOW() - INTERVAL %u MINUTE",
		log_login_db, sockt->ip2str(ip,NULL), minutes) )// how many times failed account? in one ip.
		Sql_ShowDebug(sql_handle);

	if( SQL_SUCCESS == SQL->NextRow(sql_handle) )
	{
		char* data;
		SQL->GetData(sql_handle, 0, &data, NULL);
		failures = strtoul(data, NULL, 10);
		SQL->FreeResult(sql_handle);
	}
	return failures;
}


/*=============================================
 * Records an event in the login log
 *---------------------------------------------*/
// TODO: add an enum of rcode values
void login_log(uint32 ip, const char* username, int rcode, const char* message)
{
	char esc_username[NAME_LENGTH*2+1];
	char esc_message[255*2+1];
	int retcode;

	nullpo_retv(username);
	nullpo_retv(message);
	if( !enabled )
		return;

	SQL->EscapeStringLen(sql_handle, esc_username, username, strnlen(username, NAME_LENGTH));
	SQL->EscapeStringLen(sql_handle, esc_message, message, strnlen(message, 255));

	retcode = SQL->Query(sql_handle,
		"INSERT INTO `%s`(`time`,`ip`,`user`,`rcode`,`log`) VALUES (NOW(), '%s', '%s', '%d', '%s')",
		log_login_db, sockt->ip2str(ip,NULL), esc_username, rcode, esc_message);

	if( retcode != SQL_SUCCESS )
		Sql_ShowDebug(sql_handle);
}

bool loginlog_init(void)
{
	sql_handle = SQL->Malloc();

	if (SQL_ERROR == SQL->Connect(sql_handle, log_db_username, log_db_password,
	                              log_db_hostname, log_db_port, log_db_database)) {
		Sql_ShowDebug(sql_handle);
		SQL->Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if (log_codepage[0] != '\0' && SQL_ERROR == SQL->SetEncoding(sql_handle, log_codepage))
		Sql_ShowDebug(sql_handle);

	enabled = true;

	return true;
}

bool loginlog_final(void)
{
	SQL->Free(sql_handle);
	sql_handle = NULL;
	return true;
}

/**
 * Reads 'inter_configuration/database_names' and initializes required
 * variables/Sets global configuration.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
bool loginlog_config_read_names(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "inter_configuration/database_names")) == NULL) {
		if (imported)
			return true;
		ShowError("loginlog_config_read: inter_configuration/database_names was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_mutable_string(setting, "login_db", log_login_db, sizeof(log_login_db));

	return true;
}

/**
 * Reads 'inter_configuration.log' and initializes required variables/Sets
 * global configuration.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
bool loginlog_config_read_log(const char *filename, struct config_t *config, bool imported)
{
	struct config_setting_t *setting = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "inter_configuration/log/sql_connection")) == NULL) {
		if (imported)
			return true;
		ShowError("loginlog_config_read: inter_configuration/log/sql_connection was not found in %s!\n", filename);
		return false;
	}

	libconfig->setting_lookup_mutable_string(setting, "db_hostname", log_db_hostname, sizeof(log_db_hostname));
	libconfig->setting_lookup_mutable_string(setting, "db_database", log_db_database, sizeof(log_db_database));
	libconfig->setting_lookup_mutable_string(setting, "db_username", log_db_username, sizeof(log_db_username));
	libconfig->setting_lookup_mutable_string(setting, "db_password", log_db_password, sizeof(log_db_password));

	libconfig->setting_lookup_uint16(setting, "db_port", &log_db_port);
	libconfig->setting_lookup_mutable_string(setting, "codepage", log_codepage, sizeof(log_codepage));

	return true;
}

/**
 * Reads 'inter_configuration' and initializes required variables/Sets global
 * configuration.
 *
 * @param filename Path to configuration file.
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 **/
bool loginlog_config_read(const char *filename, bool imported)
{
	struct config_t config;
	const char *import = NULL;
	bool retval = true;

	nullpo_retr(false, filename);

	if (!libconfig->load_file(&config, filename))
		return false; // Error message is already shown by libconfig->load_file

	if (!loginlog_config_read_names(filename, &config, imported))
		retval = false;
	if (!loginlog_config_read_log(filename, &config, imported))
		retval = false;

	if (libconfig->lookup_string(&config, "import", &import) == CONFIG_TRUE) {
		if (strcmp(import, filename) == 0 || strcmp(import, "conf/common/inter-server.conf") == 0) {
			ShowWarning("inter_config_read: Loop detected! Skipping 'import'...\n");
		} else {
			if (!loginlog_config_read(import, true))
				retval = false;
		}
	}

	libconfig->destroy(&config);
	return retval;
}
