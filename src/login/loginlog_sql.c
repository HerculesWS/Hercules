/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
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
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"

#include <stdlib.h> // exit

// global sql settings (in ipban_sql.c)
static char   global_db_hostname[32] = "127.0.0.1";
static uint16 global_db_port = 3306;
static char   global_db_username[32] = "ragnarok";
static char   global_db_password[100] = "ragnarok";
static char   global_db_database[32] = "ragnarok";
static char   global_codepage[32] = "";
// local sql settings
static char   log_db_hostname[32] = "";
static uint16 log_db_port = 0;
static char   log_db_username[32] = "";
static char   log_db_password[100] = "";
static char   log_db_database[32] = "";
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
	const char* username;
	const char* password;
	const char* hostname;
	uint16      port;
	const char* database;
	const char* codepage;

	if( log_db_hostname[0] != '\0' )
	{// local settings
		username = log_db_username;
		password = log_db_password;
		hostname = log_db_hostname;
		port     = log_db_port;
		database = log_db_database;
		codepage = log_codepage;
	}
	else
	{// global settings
		username = global_db_username;
		password = global_db_password;
		hostname = global_db_hostname;
		port     = global_db_port;
		database = global_db_database;
		codepage = global_codepage;
	}

	sql_handle = SQL->Malloc();

	if( SQL_ERROR == SQL->Connect(sql_handle, username, password, hostname, port, database) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( codepage[0] != '\0' && SQL_ERROR == SQL->SetEncoding(sql_handle, codepage) )
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

bool loginlog_config_read(const char* key, const char* value)
{
	const char* signature;

	nullpo_ret(key);
	nullpo_ret(value);
	signature = "sql.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safestrncpy(global_db_hostname, value, sizeof(global_db_hostname));
		else
		if( strcmpi(key, "db_port") == 0 )
			global_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "db_username") == 0 )
			safestrncpy(global_db_username, value, sizeof(global_db_username));
		else
		if( strcmpi(key, "db_password") == 0 )
			safestrncpy(global_db_password, value, sizeof(global_db_password));
		else
		if( strcmpi(key, "db_database") == 0 )
			safestrncpy(global_db_database, value, sizeof(global_db_database));
		else
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(global_codepage, value, sizeof(global_codepage));
		else
			return false;// not found
		return true;
	}

	if( strcmpi(key, "log_db_ip") == 0 )
		safestrncpy(log_db_hostname, value, sizeof(log_db_hostname));
	else
	if( strcmpi(key, "log_db_port") == 0 )
		log_db_port = (uint16)strtoul(value, NULL, 10);
	else
	if( strcmpi(key, "log_db_id") == 0 )
		safestrncpy(log_db_username, value, sizeof(log_db_username));
	else
	if( strcmpi(key, "log_db_pw") == 0 )
		safestrncpy(log_db_password, value, sizeof(log_db_password));
	else
	if( strcmpi(key, "log_db_db") == 0 )
		safestrncpy(log_db_database, value, sizeof(log_db_database));
	else
	if( strcmpi(key, "log_codepage") == 0 )
		safestrncpy(log_codepage, value, sizeof(log_codepage));
	else
	if( strcmpi(key, "log_login_db") == 0 )
		safestrncpy(log_login_db, value, sizeof(log_login_db));
	else
		return false;

	return true;
}
