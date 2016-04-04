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

#include "ipban.h"

#include "login/login.h"
#include "login/loginlog.h"
#include "common/cbasetypes.h"
#include "common/nullpo.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdlib.h>

// global sql settings
static char   global_db_hostname[32] = "127.0.0.1";
static uint16 global_db_port = 3306;
static char   global_db_username[32] = "ragnarok";
static char   global_db_password[100] = "ragnarok";
static char   global_db_database[32] = "ragnarok";
static char   global_codepage[32] = "";
// local sql settings
static char   ipban_db_hostname[32] = "";
static uint16 ipban_db_port = 0;
static char   ipban_db_username[32] = "";
static char   ipban_db_password[100] = "";
static char   ipban_db_database[32] = "";
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
	const char* username;
	const char* password;
	const char* hostname;
	uint16      port;
	const char* database;
	const char* codepage;

	ipban_inited = true;

	if (!login->config->ipban)
		return;// ipban disabled

	if( ipban_db_hostname[0] != '\0' )
	{// local settings
		username = ipban_db_username;
		password = ipban_db_password;
		hostname = ipban_db_hostname;
		port     = ipban_db_port;
		database = ipban_db_database;
		codepage = ipban_codepage;
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

	// establish connections
	sql_handle = SQL->Malloc();
	if( SQL_ERROR == SQL->Connect(sql_handle, username, password, hostname, port, database) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->Free(sql_handle);
		exit(EXIT_FAILURE);
	}
	if( codepage[0] != '\0' && SQL_ERROR == SQL->SetEncoding(sql_handle, codepage) )
		Sql_ShowDebug(sql_handle);

	if (login->config->ipban_cleanup_interval > 0)
	{ // set up periodic cleanup of connection history and active bans
		timer->add_func_list(ipban_cleanup, "ipban_cleanup");
		cleanup_timer_id = timer->add_interval(timer->gettick()+10, ipban_cleanup, 0, 0, login->config->ipban_cleanup_interval*1000);
	} else // make sure it gets cleaned up on login-server start regardless of interval-based cleanups
		ipban_cleanup(0,0,0,0);
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

// load configuration options
bool ipban_config_read(const char* key, const char* value)
{
	const char* signature;

	nullpo_ret(key);
	nullpo_ret(value);
	if( ipban_inited )
		return false;// settings can only be changed before init

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

	signature = "ipban.sql.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safestrncpy(ipban_db_hostname, value, sizeof(ipban_db_hostname));
		else
		if( strcmpi(key, "db_port") == 0 )
			ipban_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "db_username") == 0 )
			safestrncpy(ipban_db_username, value, sizeof(ipban_db_username));
		else
		if( strcmpi(key, "db_password") == 0 )
			safestrncpy(ipban_db_password, value, sizeof(ipban_db_password));
		else
		if( strcmpi(key, "db_database") == 0 )
			safestrncpy(ipban_db_database, value, sizeof(ipban_db_database));
		else
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(ipban_codepage, value, sizeof(ipban_codepage));
		else
		if( strcmpi(key, "ipban_table") == 0 )
			safestrncpy(ipban_table, value, sizeof(ipban_table));
		else
			return false;// not found
		return true;
	}

	signature = "ipban.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "enable") == 0 )
			login->config->ipban = (bool)config_switch(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban") == 0 )
			login->config->dynamic_pass_failure_ban = (bool)config_switch(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban_interval") == 0 )
			login->config->dynamic_pass_failure_ban_interval = atoi(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban_limit") == 0 )
			login->config->dynamic_pass_failure_ban_limit = atoi(value);
		else
		if( strcmpi(key, "dynamic_pass_failure_ban_duration") == 0 )
			login->config->dynamic_pass_failure_ban_duration = atoi(value);
		else
			return false;// not found
		return true;
	}

	return false;// not found
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
