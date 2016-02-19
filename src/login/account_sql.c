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

#include "config/core.h" // CONSOLE_INPUT
#include "account.h"

#include "common/cbasetypes.h"
#include "common/console.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"

#include <stdlib.h>

/// global defines
#define ACCOUNT_SQL_DB_VERSION 20110114

/// internal structure
typedef struct AccountDB_SQL
{
	AccountDB vtable;    // public interface

	struct Sql *accounts; // SQL accounts storage

	// global sql settings
	char   global_db_hostname[32];
	uint16 global_db_port;
	char   global_db_username[32];
	char   global_db_password[100];
	char   global_db_database[32];
	char   global_codepage[32];
	// local sql settings
	char   db_hostname[32];
	uint16 db_port;
	char   db_username[32];
	char   db_password[100];
	char   db_database[32];
	char   codepage[32];
	// other settings
	bool case_sensitive;
	char account_db[32];
	char global_acc_reg_num_db[32];
	char global_acc_reg_str_db[32];


} AccountDB_SQL;

/// internal structure
typedef struct AccountDBIterator_SQL
{
	AccountDBIterator vtable;    // public interface

	AccountDB_SQL* db;
	int last_account_id;
} AccountDBIterator_SQL;

/// internal functions
static bool account_db_sql_init(AccountDB* self);
static void account_db_sql_destroy(AccountDB* self);
static bool account_db_sql_get_property(AccountDB* self, const char* key, char* buf, size_t buflen);
static bool account_db_sql_set_property(AccountDB* self, const char* option, const char* value);
static bool account_db_sql_create(AccountDB* self, struct mmo_account* acc);
static bool account_db_sql_remove(AccountDB* self, const int account_id);
static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc);
static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const int account_id);
static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid);
static AccountDBIterator* account_db_sql_iterator(AccountDB* self);
static void account_db_sql_iter_destroy(AccountDBIterator* self);
static bool account_db_sql_iter_next(AccountDBIterator* self, struct mmo_account* acc);

static bool mmo_auth_fromsql(AccountDB_SQL* db, struct mmo_account* acc, int account_id);
static bool mmo_auth_tosql(AccountDB_SQL* db, const struct mmo_account* acc, bool is_new);

/// public constructor
AccountDB* account_db_sql(void)
{
	AccountDB_SQL* db = (AccountDB_SQL*)aCalloc(1, sizeof(AccountDB_SQL));

	// set up the vtable
	db->vtable.init         = &account_db_sql_init;
	db->vtable.destroy      = &account_db_sql_destroy;
	db->vtable.get_property = &account_db_sql_get_property;
	db->vtable.set_property = &account_db_sql_set_property;
	db->vtable.save         = &account_db_sql_save;
	db->vtable.create       = &account_db_sql_create;
	db->vtable.remove       = &account_db_sql_remove;
	db->vtable.load_num     = &account_db_sql_load_num;
	db->vtable.load_str     = &account_db_sql_load_str;
	db->vtable.iterator     = &account_db_sql_iterator;

	// initialize to default values
	db->accounts = NULL;
	// global sql settings
	safestrncpy(db->global_db_hostname, "127.0.0.1", sizeof(db->global_db_hostname));
	db->global_db_port = 3306;
	safestrncpy(db->global_db_username, "ragnarok", sizeof(db->global_db_username));
	safestrncpy(db->global_db_password, "ragnarok", sizeof(db->global_db_password));
	safestrncpy(db->global_db_database, "ragnarok", sizeof(db->global_db_database));
	safestrncpy(db->global_codepage, "", sizeof(db->global_codepage));
	// local sql settings
	safestrncpy(db->db_hostname, "", sizeof(db->db_hostname));
	db->db_port = 3306;
	safestrncpy(db->db_username, "", sizeof(db->db_username));
	safestrncpy(db->db_password, "", sizeof(db->db_password));
	safestrncpy(db->db_database, "", sizeof(db->db_database));
	safestrncpy(db->codepage, "", sizeof(db->codepage));
	// other settings
	db->case_sensitive = false;
	safestrncpy(db->account_db, "login", sizeof(db->account_db));
	safestrncpy(db->global_acc_reg_num_db, "global_acc_reg_num_db", sizeof(db->global_acc_reg_num_db));
	safestrncpy(db->global_acc_reg_str_db, "global_acc_reg_str_db", sizeof(db->global_acc_reg_str_db));

	return &db->vtable;
}


/* ------------------------------------------------------------------------- */


/// establishes database connection
static bool account_db_sql_init(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	struct Sql *sql_handle;
	const char* username;
	const char* password;
	const char* hostname;
	uint16      port;
	const char* database;
	const char* codepage;

	nullpo_ret(db);
	db->accounts = SQL->Malloc();
	sql_handle = db->accounts;

	if( db->db_hostname[0] != '\0' )
	{// local settings
		username = db->db_username;
		password = db->db_password;
		hostname = db->db_hostname;
		port     = db->db_port;
		database = db->db_database;
		codepage = db->codepage;
	}
	else
	{// global settings
		username = db->global_db_username;
		password = db->global_db_password;
		hostname = db->global_db_hostname;
		port     = db->global_db_port;
		database = db->global_db_database;
		codepage = db->global_codepage;
	}

	if( SQL_ERROR == SQL->Connect(sql_handle, username, password, hostname, port, database) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->Free(db->accounts);
		db->accounts = NULL;
		return false;
	}

	if( codepage[0] != '\0' && SQL_ERROR == SQL->SetEncoding(sql_handle, codepage) )
		Sql_ShowDebug(sql_handle);

	Sql_HerculesUpdateCheck(db->accounts);
#ifdef CONSOLE_INPUT
	console->input->setSQL(db->accounts);
#endif
	return true;
}

/// disconnects from database
static void account_db_sql_destroy(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;

	nullpo_retv(db);
	SQL->Free(db->accounts);
	db->accounts = NULL;
	aFree(db);
}

/// Gets a property from this database.
static bool account_db_sql_get_property(AccountDB* self, const char* key, char* buf, size_t buflen)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	const char* signature;

	nullpo_ret(db);
	nullpo_ret(key);
	nullpo_ret(buf);
	signature = "engine.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "name") == 0 )
			safesnprintf(buf, buflen, "sql");
		else
		if( strcmpi(key, "version") == 0 )
			safesnprintf(buf, buflen, "%d", ACCOUNT_SQL_DB_VERSION);
		else
		if( strcmpi(key, "comment") == 0 )
			safesnprintf(buf, buflen, "SQL Account Database");
		else
			return false;// not found
		return true;
	}

	signature = "sql.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_hostname);
		else
		if( strcmpi(key, "db_port") == 0 )
			safesnprintf(buf, buflen, "%d", db->global_db_port);
		else
		if( strcmpi(key, "db_username") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_username);
		else
		if( strcmpi(key, "db_password") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_password);
		else
		if( strcmpi(key, "db_database") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_db_database);
		else
		if( strcmpi(key, "codepage") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_codepage);
		else
			return false;// not found
		return true;
	}

	signature = "account.sql.";
	if( strncmpi(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_hostname);
		else
		if( strcmpi(key, "db_port") == 0 )
			safesnprintf(buf, buflen, "%d", db->db_port);
		else
		if( strcmpi(key, "db_username") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_username);
		else
		if( strcmpi(key, "db_password") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_password);
		else
		if( strcmpi(key, "db_database") == 0 )
			safesnprintf(buf, buflen, "%s", db->db_database);
		else
		if( strcmpi(key, "codepage") == 0 )
			safesnprintf(buf, buflen, "%s", db->codepage);
		else
		if( strcmpi(key, "case_sensitive") == 0 )
			safesnprintf(buf, buflen, "%d", (db->case_sensitive ? 1 : 0));
		else
		if( strcmpi(key, "account_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->account_db);
		else
		if( strcmpi(key, "global_acc_reg_str_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_acc_reg_str_db);
		else
		if( strcmpi(key, "global_acc_reg_num_db") == 0 )
			safesnprintf(buf, buflen, "%s", db->global_acc_reg_num_db);
		else
			return false;// not found
		return true;
	}

	return false;// not found
}

/// if the option is supported, adjusts the internal state
static bool account_db_sql_set_property(AccountDB* self, const char* key, const char* value)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	const char* signature;

	nullpo_ret(db);
	nullpo_ret(key);
	nullpo_ret(value);
	signature = "sql.";
	if( strncmp(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safestrncpy(db->global_db_hostname, value, sizeof(db->global_db_hostname));
		else
		if( strcmpi(key, "db_port") == 0 )
			db->global_db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "db_username") == 0 )
			safestrncpy(db->global_db_username, value, sizeof(db->global_db_username));
		else
		if( strcmpi(key, "db_password") == 0 )
			safestrncpy(db->global_db_password, value, sizeof(db->global_db_password));
		else
		if( strcmpi(key, "db_database") == 0 )
			safestrncpy(db->global_db_database, value, sizeof(db->global_db_database));
		else
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(db->global_codepage, value, sizeof(db->global_codepage));
		else
			return false;// not found
		return true;
	}

	signature = "account.sql.";
	if( strncmp(key, signature, strlen(signature)) == 0 )
	{
		key += strlen(signature);
		if( strcmpi(key, "db_hostname") == 0 )
			safestrncpy(db->db_hostname, value, sizeof(db->db_hostname));
		else
		if( strcmpi(key, "db_port") == 0 )
			db->db_port = (uint16)strtoul(value, NULL, 10);
		else
		if( strcmpi(key, "db_username") == 0 )
			safestrncpy(db->db_username, value, sizeof(db->db_username));
		else
		if( strcmpi(key, "db_password") == 0 )
			safestrncpy(db->db_password, value, sizeof(db->db_password));
		else
		if( strcmpi(key, "db_database") == 0 )
			safestrncpy(db->db_database, value, sizeof(db->db_database));
		else
		if( strcmpi(key, "codepage") == 0 )
			safestrncpy(db->codepage, value, sizeof(db->codepage));
		else
		if( strcmpi(key, "case_sensitive") == 0 )
			db->case_sensitive = (bool)config_switch(value);
		else
		if( strcmpi(key, "account_db") == 0 )
			safestrncpy(db->account_db, value, sizeof(db->account_db));
		else
		if( strcmpi(key, "global_acc_reg_str_db") == 0 )
			safestrncpy(db->global_acc_reg_str_db, value, sizeof(db->global_acc_reg_str_db));
		else
		if( strcmpi(key, "global_acc_reg_num_db") == 0 )
			safestrncpy(db->global_acc_reg_num_db, value, sizeof(db->global_acc_reg_num_db));
		else
			return false;// not found
		return true;
	}

	return false;// not found
}

/// create a new account entry
/// If acc->account_id is -1, the account id will be auto-generated,
/// and its value will be written to acc->account_id if everything succeeds.
static bool account_db_sql_create(AccountDB* self, struct mmo_account* acc)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	struct Sql *sql_handle;

	// decide on the account id to assign
	int account_id;
	nullpo_ret(db);
	nullpo_ret(acc);
	sql_handle = db->accounts;
	if( acc->account_id != -1 )
	{// caller specifies it manually
		account_id = acc->account_id;
	}
	else
	{// ask the database
		char* data;
		size_t len;

		if( SQL_SUCCESS != SQL->Query(sql_handle, "SELECT MAX(`account_id`)+1 FROM `%s`", db->account_db) )
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
		{
			Sql_ShowDebug(sql_handle);
			SQL->FreeResult(sql_handle);
			return false;
		}

		SQL->GetData(sql_handle, 0, &data, &len);
		account_id = ( data != NULL ) ? atoi(data) : 0;
		SQL->FreeResult(sql_handle);

		if( account_id < START_ACCOUNT_NUM )
			account_id = START_ACCOUNT_NUM;

	}

	// zero value is prohibited
	if( account_id == 0 )
		return false;

	// absolute maximum
	if( account_id > END_ACCOUNT_NUM )
		return false;

	// insert the data into the database
	acc->account_id = account_id;
	return mmo_auth_tosql(db, acc, true);
}

/// delete an existing account entry + its regs
static bool account_db_sql_remove(AccountDB* self, const int account_id)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	struct Sql *sql_handle;
	bool result = false;

	nullpo_ret(db);
	sql_handle = db->accounts;
	if( SQL_SUCCESS != SQL->QueryStr(sql_handle, "START TRANSACTION")
	||  SQL_SUCCESS != SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", db->account_db, account_id)
	||  SQL_SUCCESS != SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", db->global_acc_reg_num_db, account_id)
	||  SQL_SUCCESS != SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = %d", db->global_acc_reg_str_db, account_id)
	)
		Sql_ShowDebug(sql_handle);
	else
		result = true;

	result &= ( SQL_SUCCESS == SQL->QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") );

	return result;
}

/// update an existing account with the provided new data (both account and regs)
static bool account_db_sql_save(AccountDB* self, const struct mmo_account* acc)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	return mmo_auth_tosql(db, acc, false);
}

/// retrieve data from db and store it in the provided data structure
static bool account_db_sql_load_num(AccountDB* self, struct mmo_account* acc, const int account_id)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	return mmo_auth_fromsql(db, acc, account_id);
}

/// retrieve data from db and store it in the provided data structure
static bool account_db_sql_load_str(AccountDB* self, struct mmo_account* acc, const char* userid)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	struct Sql *sql_handle;
	char esc_userid[2*NAME_LENGTH+1];
	int account_id;
	char* data;

	nullpo_ret(db);
	sql_handle = db->accounts;
	SQL->EscapeString(sql_handle, esc_userid, userid);

	// get the list of account IDs for this user ID
	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `account_id` FROM `%s` WHERE `userid`= %s '%s'",
		db->account_db, (db->case_sensitive ? "BINARY" : ""), esc_userid) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL->NumRows(sql_handle) > 1 )
	{// serious problem - duplicate account
		ShowError("account_db_sql_load_str: multiple accounts found when retrieving data for account '%s'!\n", userid);
		SQL->FreeResult(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
	{// no such entry
		SQL->FreeResult(sql_handle);
		return false;
	}

	SQL->GetData(sql_handle, 0, &data, NULL);
	account_id = atoi(data);

	return account_db_sql_load_num(self, acc, account_id);
}


/// Returns a new forward iterator.
static AccountDBIterator* account_db_sql_iterator(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	AccountDBIterator_SQL* iter;

	nullpo_retr(NULL, db);
	iter = (AccountDBIterator_SQL*)aCalloc(1, sizeof(AccountDBIterator_SQL));
	// set up the vtable
	iter->vtable.destroy = &account_db_sql_iter_destroy;
	iter->vtable.next    = &account_db_sql_iter_next;

	// fill data
	iter->db = db;
	iter->last_account_id = -1;

	return &iter->vtable;
}


/// Destroys this iterator, releasing all allocated memory (including itself).
static void account_db_sql_iter_destroy(AccountDBIterator* self)
{
	AccountDBIterator_SQL* iter = (AccountDBIterator_SQL*)self;
	aFree(iter);
}


/// Fetches the next account in the database.
static bool account_db_sql_iter_next(AccountDBIterator* self, struct mmo_account* acc)
{
	AccountDBIterator_SQL* iter = (AccountDBIterator_SQL*)self;
	AccountDB_SQL* db;
	struct Sql *sql_handle;
	char* data;

	nullpo_ret(iter);
	db = (AccountDB_SQL*)iter->db;
	nullpo_ret(db);
	sql_handle = db->accounts;
	// get next account ID
	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `account_id` FROM `%s` WHERE `account_id` > '%d' ORDER BY `account_id` ASC LIMIT 1",
		db->account_db, iter->last_account_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS == SQL->NextRow(sql_handle) &&
		SQL_SUCCESS == SQL->GetData(sql_handle, 0, &data, NULL) &&
		data != NULL )
	{// get account data
		int account_id;
		account_id = atoi(data);
		if( mmo_auth_fromsql(db, acc, account_id) )
		{
			iter->last_account_id = account_id;
			SQL->FreeResult(sql_handle);
			return true;
		}
	}
	SQL->FreeResult(sql_handle);
	return false;
}


static bool mmo_auth_fromsql(AccountDB_SQL* db, struct mmo_account* acc, int account_id)
{
	struct Sql *sql_handle;
	char* data;

	nullpo_ret(db);
	nullpo_ret(acc);
	sql_handle = db->accounts;
	// retrieve login entry for the specified account
	if( SQL_ERROR == SQL->Query(sql_handle,
	    "SELECT `account_id`,`userid`,`user_pass`,`sex`,`email`,`group_id`,`state`,`unban_time`,`expiration_time`,`logincount`,`lastlogin`,`last_ip`,`birthdate`,`character_slots`,`pincode`,`pincode_change` FROM `%s` WHERE `account_id` = %d",
		db->account_db, account_id )
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
	{// no such entry
		SQL->FreeResult(sql_handle);
		return false;
	}

	SQL->GetData(sql_handle,  0, &data, NULL); acc->account_id = atoi(data);
	SQL->GetData(sql_handle,  1, &data, NULL); safestrncpy(acc->userid, data, sizeof(acc->userid));
	SQL->GetData(sql_handle,  2, &data, NULL); safestrncpy(acc->pass, data, sizeof(acc->pass));
	SQL->GetData(sql_handle,  3, &data, NULL); acc->sex = data[0];
	SQL->GetData(sql_handle,  4, &data, NULL); safestrncpy(acc->email, data, sizeof(acc->email));
	SQL->GetData(sql_handle,  5, &data, NULL); acc->group_id = atoi(data);
	SQL->GetData(sql_handle,  6, &data, NULL); acc->state = (unsigned int)strtoul(data, NULL, 10);
	SQL->GetData(sql_handle,  7, &data, NULL); acc->unban_time = atol(data);
	SQL->GetData(sql_handle,  8, &data, NULL); acc->expiration_time = atol(data);
	SQL->GetData(sql_handle,  9, &data, NULL); acc->logincount = (unsigned int)strtoul(data, NULL, 10);
	SQL->GetData(sql_handle, 10, &data, NULL); safestrncpy(acc->lastlogin, data, sizeof(acc->lastlogin));
	SQL->GetData(sql_handle, 11, &data, NULL); safestrncpy(acc->last_ip, data, sizeof(acc->last_ip));
	SQL->GetData(sql_handle, 12, &data, NULL); safestrncpy(acc->birthdate, data, sizeof(acc->birthdate));
	SQL->GetData(sql_handle, 13, &data, NULL); acc->char_slots = (uint8)atoi(data);
	SQL->GetData(sql_handle, 14, &data, NULL); safestrncpy(acc->pincode, data, sizeof(acc->pincode));
	SQL->GetData(sql_handle, 15, &data, NULL); acc->pincode_change = (unsigned int)atol(data);

	SQL->FreeResult(sql_handle);

	return true;
}

static bool mmo_auth_tosql(AccountDB_SQL* db, const struct mmo_account* acc, bool is_new)
{
	struct Sql *sql_handle;
	struct SqlStmt *stmt;
	bool result = false;

	nullpo_ret(db);
	nullpo_ret(acc);
	sql_handle = db->accounts;
	stmt = SQL->StmtMalloc(sql_handle);

	// try
	do
	{

	if( SQL_SUCCESS != SQL->QueryStr(sql_handle, "START TRANSACTION") )
	{
		Sql_ShowDebug(sql_handle);
		break;
	}

	if( is_new )
	{// insert into account table
		if( SQL_SUCCESS != SQL->StmtPrepare(stmt,
			"INSERT INTO `%s` (`account_id`, `userid`, `user_pass`, `sex`, `email`, `group_id`, `state`, `unban_time`, `expiration_time`, `logincount`, `lastlogin`, `last_ip`, `birthdate`, `character_slots`, `pincode`, `pincode_change`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
			db->account_db)
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  0, SQLDT_INT,    &acc->account_id,      sizeof(acc->account_id))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  1, SQLDT_STRING, acc->userid,           strlen(acc->userid))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  2, SQLDT_STRING, acc->pass,             strlen(acc->pass))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  3, SQLDT_ENUM,   &acc->sex,             sizeof(acc->sex))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  4, SQLDT_STRING, &acc->email,           strlen(acc->email))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  5, SQLDT_INT,    &acc->group_id,        sizeof(acc->group_id))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  6, SQLDT_UINT,   &acc->state,           sizeof(acc->state))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  7, SQLDT_LONG,   &acc->unban_time,      sizeof(acc->unban_time))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  8, SQLDT_INT,    &acc->expiration_time, sizeof(acc->expiration_time))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  9, SQLDT_UINT,   &acc->logincount,      sizeof(acc->logincount))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 10, SQLDT_STRING, &acc->lastlogin,       strlen(acc->lastlogin))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 11, SQLDT_STRING, &acc->last_ip,         strlen(acc->last_ip))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 12, SQLDT_STRING, &acc->birthdate,       strlen(acc->birthdate))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 13, SQLDT_UCHAR,  &acc->char_slots,      sizeof(acc->char_slots))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 14, SQLDT_STRING, &acc->pincode,         strlen(acc->pincode))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 15, SQLDT_LONG,   &acc->pincode_change,  sizeof(acc->pincode_change))
		||  SQL_SUCCESS != SQL->StmtExecute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	} else {// update account table
		if( SQL_SUCCESS != SQL->StmtPrepare(stmt, "UPDATE `%s` SET `userid`=?,`user_pass`=?,`sex`=?,`email`=?,`group_id`=?,`state`=?,`unban_time`=?,`expiration_time`=?,`logincount`=?,`lastlogin`=?,`last_ip`=?,`birthdate`=?,`character_slots`=?,`pincode`=?,`pincode_change`=? WHERE `account_id` = '%d'", db->account_db, acc->account_id)
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  0, SQLDT_STRING, acc->userid,           strlen(acc->userid))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  1, SQLDT_STRING, acc->pass,             strlen(acc->pass))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  2, SQLDT_ENUM,   &acc->sex,             sizeof(acc->sex))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  3, SQLDT_STRING, acc->email,            strlen(acc->email))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  4, SQLDT_INT,    &acc->group_id,        sizeof(acc->group_id))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  5, SQLDT_UINT,   &acc->state,           sizeof(acc->state))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  6, SQLDT_LONG,   &acc->unban_time,      sizeof(acc->unban_time))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  7, SQLDT_LONG,   &acc->expiration_time, sizeof(acc->expiration_time))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  8, SQLDT_UINT,   &acc->logincount,      sizeof(acc->logincount))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt,  9, SQLDT_STRING, &acc->lastlogin,       strlen(acc->lastlogin))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 10, SQLDT_STRING, &acc->last_ip,         strlen(acc->last_ip))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 11, SQLDT_STRING, &acc->birthdate,       strlen(acc->birthdate))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 12, SQLDT_UCHAR,  &acc->char_slots,      sizeof(acc->char_slots))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 13, SQLDT_STRING, &acc->pincode,         strlen(acc->pincode))
		||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 14, SQLDT_LONG,   &acc->pincode_change,  sizeof(acc->pincode_change))
		||  SQL_SUCCESS != SQL->StmtExecute(stmt)
		) {
			SqlStmt_ShowDebug(stmt);
			break;
		}
	}

	// if we got this far, everything was successful
	result = true;

	} while(0);
	// finally

	result &= ( SQL_SUCCESS == SQL->QueryStr(sql_handle, (result == true) ? "COMMIT" : "ROLLBACK") );
	SQL->StmtFree(stmt);

	return result;
}

struct Sql *account_db_sql_up(AccountDB* self)
{
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	return db ? db->accounts : NULL;
}
void mmo_save_accreg2(AccountDB* self, int fd, int account_id, int char_id)
{
	struct Sql *sql_handle;
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	int count = RFIFOW(fd, 12);

	nullpo_retv(db);
	sql_handle = db->accounts;
	if (count) {
		int cursor = 14, i;
		char key[SCRIPT_VARNAME_LENGTH+1], sval[254];

		for (i = 0; i < count; i++) {
			unsigned int index;
			int len = RFIFOB(fd, cursor);
			safestrncpy(key, RFIFOP(fd, cursor + 1), min((int)sizeof(key), len));
			cursor += len + 1;

			index = RFIFOL(fd, cursor);
			cursor += 4;

			switch (RFIFOB(fd, cursor++)) {
				/* int */
				case 0:
					if( SQL_ERROR == SQL->Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%u')", db->global_acc_reg_num_db, account_id, key, index, RFIFOL(fd, cursor)) )
						Sql_ShowDebug(sql_handle);
					cursor += 4;
					break;
				case 1:
					if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", db->global_acc_reg_num_db, account_id, key, index) )
						Sql_ShowDebug(sql_handle);
					break;
				/* str */
				case 2:
					len = RFIFOB(fd, cursor);
					safestrncpy(sval, RFIFOP(fd, cursor + 1), min((int)sizeof(sval), len));
					cursor += len + 1;
					if( SQL_ERROR == SQL->Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%s')", db->global_acc_reg_str_db, account_id, key, index, sval) )
						Sql_ShowDebug(sql_handle);
					break;
				case 3:
					if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", db->global_acc_reg_str_db, account_id, key, index) )
						Sql_ShowDebug(sql_handle);
					break;
				default:
					ShowError("mmo_save_accreg2: DA HOO UNKNOWN TYPE %d\n",RFIFOB(fd, cursor - 1));
					return;
			}
		}
	}
}

void mmo_send_accreg2(AccountDB* self, int fd, int account_id, int char_id)
{
	struct Sql *sql_handle;
	AccountDB_SQL* db = (AccountDB_SQL*)self;
	char* data;
	int plen = 0;
	size_t len;

	nullpo_retv(db);
	sql_handle = db->accounts;
	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%d'", db->global_acc_reg_str_db, account_id) )
		Sql_ShowDebug(sql_handle);

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x3804;
	/* 0x2 = length, set prior to being sent */
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
	WFIFOB(fd, 13) = 1;/* is string type */
	WFIFOW(fd, 14) = 0;/* count */
	plen = 16;

	/**
	 * Vessel!
	 *
	 * str type
	 * { keyLength(B), key(<keyLength>), index(L), valLength(B), val(<valLength>) }
	 **/
	while ( SQL_SUCCESS == SQL->NextRow(sql_handle) ) {
		SQL->GetData(sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 32 */
		plen += 1;

		safestrncpy(WFIFOP(fd,plen), data, len);
		plen += len;

		SQL->GetData(sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (unsigned int)atol(data);
		plen += 4;

		SQL->GetData(sql_handle, 2, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 254 */
		plen += 1;

		safestrncpy(WFIFOP(fd,plen), data, len);
		plen += len;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			/* prepare follow up */
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x3804;
			/* 0x2 = length, set prior to being sent */
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
			WFIFOB(fd, 13) = 1;/* is string type */
			WFIFOW(fd, 14) = 0;/* count */
			plen = 16;
		}
	}

	/* mark & go. */
	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	SQL->FreeResult(sql_handle);

	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%d'", db->global_acc_reg_num_db, account_id) )
		Sql_ShowDebug(sql_handle);

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x3804;
	/* 0x2 = length, set prior to being sent */
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
	WFIFOB(fd, 13) = 0;/* is int type */
	WFIFOW(fd, 14) = 0;/* count */
	plen = 16;

	/**
	 * Vessel!
	 *
	 * int type
	 * { keyLength(B), key(<keyLength>), index(L), value(L) }
	 **/
	while ( SQL_SUCCESS == SQL->NextRow(sql_handle) ) {
		SQL->GetData(sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 32 */
		plen += 1;

		safestrncpy(WFIFOP(fd,plen), data, len);
		plen += len;

		SQL->GetData(sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (unsigned int)atol(data);
		plen += 4;

		SQL->GetData(sql_handle, 2, &data, NULL);

		WFIFOL(fd, plen) = atoi(data);
		plen += 4;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			/* prepare follow up */
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x3804;
			/* 0x2 = length, set prior to being sent */
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
			WFIFOB(fd, 13) = 0;/* is int type */
			WFIFOW(fd, 14) = 0;/* count */

			plen = 16;
		}
	}

	/* mark as complete & go. */
	WFIFOB(fd, 12) = 1;
	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	SQL->FreeResult(sql_handle);
}
