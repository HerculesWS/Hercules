// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/ers.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/sql.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "map.h" // mmysql_handle
#include "script.h"
#include "mapreg.h"
#include <stdlib.h>
#include <string.h>

static DBMap* mapreg_db = NULL; // int var_id -> int value
static DBMap* mapregstr_db = NULL; // int var_id -> char* value
static struct eri *mapreg_ers; //[Ind/Hercules]

static char mapreg_table[32] = "mapreg";
static bool mapreg_i_dirty = false;
static bool mapreg_str_dirty = false;

#define MAPREG_AUTOSAVE_INTERVAL (300*1000)


/// Looks up the value of an integer variable using its uid.
int mapreg_readreg(int uid) {
	struct mapreg_save *m = idb_get(mapreg_db, uid);
	return m?m->u.i:0;
}

/// Looks up the value of a string variable using its uid.
char* mapreg_readregstr(int uid) {
	struct mapreg_save *m = idb_get(mapregstr_db, uid);
	return m?m->u.str:NULL;
}

/// Modifies the value of an integer variable.
bool mapreg_setreg(int uid, int val) {
	struct mapreg_save *m;
	int num = (uid & 0x00ffffff);
	int i   = (uid & 0xff000000) >> 24;
	const char* name = script->get_str(num);

	if( val != 0 ) {
		if( (m = idb_get(mapreg_db,uid)) ) {
			m->u.i = val;
			if(name[1] != '@') {
				m->save = true;
				mapreg_i_dirty = true;
			}
		} else {
			m = ers_alloc(mapreg_ers, struct mapreg_save);

			m->u.i = val;
			m->uid = uid;
			m->save = false;

			if(name[1] != '@') {// write new variable to database
				char tmp_str[32*2+1];
				SQL->EscapeStringLen(mmysql_handle, tmp_str, name, strnlen(name, 32));
				if( SQL_ERROR == SQL->Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%d','%d')", mapreg_table, tmp_str, i, val) )
					Sql_ShowDebug(mmysql_handle);
			}
			idb_put(mapreg_db, uid, m);
		}
	} else { // val == 0
		if( (m = idb_get(mapreg_db,uid)) ) {
			ers_free(mapreg_ers, m);
		}
		idb_remove(mapreg_db,uid);

		if( name[1] != '@' ) {// Remove from database because it is unused.
			if( SQL_ERROR == SQL->Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%d'", mapreg_table, name, i) )
				Sql_ShowDebug(mmysql_handle);
		}
	}

	return true;
}

/// Modifies the value of a string variable.
bool mapreg_setregstr(int uid, const char* str) {
	struct mapreg_save *m;
	int num = (uid & 0x00ffffff);
	int i   = (uid & 0xff000000) >> 24;
	const char* name = script->get_str(num);
	
	if( str == NULL || *str == 0 ) {
		if(name[1] != '@') {
			if( SQL_ERROR == SQL->Query(mmysql_handle, "DELETE FROM `%s` WHERE `varname`='%s' AND `index`='%d'", mapreg_table, name, i) )
				Sql_ShowDebug(mmysql_handle);
		}
		if( (m = idb_get(mapregstr_db,uid)) ) {
			if( m->u.str != NULL )
				aFree(m->u.str);
			ers_free(mapreg_ers, m);
		}
		idb_remove(mapregstr_db,uid);
	} else {
		if( (m = idb_get(mapregstr_db,uid)) ) {
			if( m->u.str != NULL )
				aFree(m->u.str);
			m->u.str = aStrdup(str);
			if(name[1] != '@') {
				mapreg_str_dirty = true;
				m->save = true;
			}
		} else {
			m = ers_alloc(mapreg_ers, struct mapreg_save);

			m->uid = uid;
			m->u.str = aStrdup(str);
			m->save = false;
			
			if(name[1] != '@') { //put returned null, so we must insert.
				char tmp_str[32*2+1];
				char tmp_str2[255*2+1];
				SQL->EscapeStringLen(mmysql_handle, tmp_str, name, strnlen(name, 32));
				SQL->EscapeStringLen(mmysql_handle, tmp_str2, str, strnlen(str, 255));
				if( SQL_ERROR == SQL->Query(mmysql_handle, "INSERT INTO `%s`(`varname`,`index`,`value`) VALUES ('%s','%d','%s')", mapreg_table, tmp_str, i, tmp_str2) )
					Sql_ShowDebug(mmysql_handle);
			}
			idb_put(mapregstr_db, uid, m);
		}
	}

	return true;
}

/// Loads permanent variables from database
static void script_load_mapreg(void) {
	/*
	        0        1       2
	   +-------------------------+
	   | varname | index | value |
	   +-------------------------+
	                                */
	SqlStmt* stmt = SQL->StmtMalloc(mmysql_handle);
	char varname[32+1];
	int index;
	char value[255+1];
	uint32 length;

	if ( SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `varname`, `index`, `value` FROM `%s`", mapreg_table)
	  || SQL_ERROR == SQL->StmtExecute(stmt)
	  ) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}

	SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &varname[0], sizeof(varname), &length, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_INT, &index, 0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_STRING, &value[0], sizeof(value), NULL, NULL);
	
	while ( SQL_SUCCESS == SQL->StmtNextRow(stmt) ) {
		struct mapreg_save *m = NULL;
		int s = script->add_str(varname);
		int i = index;

		if( varname[length-1] == '$' ) {
			if( idb_exists(mapregstr_db, (i<<24)|s) ) {
				ShowWarning("load_mapreg: duplicate! '%s' => '%s' skipping...\n",varname,value);
				continue;
			}
		} else {
			if( idb_exists(mapreg_db, (i<<24)|s) ) {
				ShowWarning("load_mapreg: duplicate! '%s' => '%s' skipping...\n",varname,value);
				continue;
			}
		}
		
		m = ers_alloc(mapreg_ers, struct mapreg_save);
		m->uid = (i<<24)|s;
		m->save = false;
		if( varname[length-1] == '$' ) {
			m->u.str = aStrdup(value);
			idb_put(mapregstr_db, m->uid, m);
		} else {
			m->u.i = atoi(value);
			idb_put(mapreg_db, m->uid, m);
		}
	}
	
	SQL->StmtFree(stmt);

	mapreg_i_dirty = false;
	mapreg_str_dirty = false;
}

/// Saves permanent variables to database
static void script_save_mapreg(void) {
	DBIterator* iter;
	struct mapreg_save *m = NULL;

	if( mapreg_i_dirty ) {
		iter = db_iterator(mapreg_db);
		for( m = dbi_first(iter); dbi_exists(iter); m = dbi_next(iter) ) {
			if( m->save ) {
				int num = (m->uid & 0x00ffffff);
				int i   = (m->uid & 0xff000000) >> 24;
				const char* name = script->get_str(num);

				if( SQL_ERROR == SQL->Query(mmysql_handle, "UPDATE `%s` SET `value`='%d' WHERE `varname`='%s' AND `index`='%d' LIMIT 1", mapreg_table, m->u.i, name, i) )
					Sql_ShowDebug(mmysql_handle);
				m->save = false;
			}
		}
		dbi_destroy(iter);
		mapreg_i_dirty = false;
	}

	if( mapreg_str_dirty ) {
		iter = db_iterator(mapregstr_db);
		for( m = dbi_first(iter); dbi_exists(iter); m = dbi_next(iter) ) {
			if( m->save ) {
				int num = (m->uid & 0x00ffffff);
				int i   = (m->uid & 0xff000000) >> 24;
				const char* name = script->get_str(num);
				char tmp_str2[2*255+1];

				SQL->EscapeStringLen(mmysql_handle, tmp_str2, m->u.str, safestrnlen(m->u.str, 255));
				if( SQL_ERROR == SQL->Query(mmysql_handle, "UPDATE `%s` SET `value`='%s' WHERE `varname`='%s' AND `index`='%d' LIMIT 1", mapreg_table, tmp_str2, name, i) )
					Sql_ShowDebug(mmysql_handle);
				m->save = false;
			}
		}
		dbi_destroy(iter);
		mapreg_str_dirty = false;
	}
}

static int script_autosave_mapreg(int tid, unsigned int tick, int id, intptr_t data) {
	script_save_mapreg();
	return 0;
}


void mapreg_reload(void) {
	DBIterator* iter;
	struct mapreg_save *m = NULL;

	script_save_mapreg();

	iter = db_iterator(mapreg_db);
	for( m = dbi_first(iter); dbi_exists(iter); m = dbi_next(iter) ) {
		ers_free(mapreg_ers, m);
	}
	dbi_destroy(iter);
	
	iter = db_iterator(mapregstr_db);
	for( m = dbi_first(iter); dbi_exists(iter); m = dbi_next(iter) ) {
		if( m->u.str != NULL ) {
			aFree(m->u.str);
		}
		ers_free(mapreg_ers, m);
	}
	dbi_destroy(iter);
	
	db_clear(mapreg_db);
	db_clear(mapregstr_db);

	script_load_mapreg();
}

void mapreg_final(void) {
	DBIterator* iter;
	struct mapreg_save *m = NULL;
	
	script_save_mapreg();

	iter = db_iterator(mapreg_db);
	for( m = dbi_first(iter); dbi_exists(iter); m = dbi_next(iter) ) {
		ers_free(mapreg_ers, m);
	}
	dbi_destroy(iter);
	
	iter = db_iterator(mapregstr_db);
	for( m = dbi_first(iter); dbi_exists(iter); m = dbi_next(iter) ) {
		if( m->u.str != NULL ) {
			aFree(m->u.str);
		}
		ers_free(mapreg_ers, m);
	}
	dbi_destroy(iter);
		
	db_destroy(mapreg_db);
	db_destroy(mapregstr_db);
	
	ers_destroy(mapreg_ers);
}

void mapreg_init(void) {
	mapreg_db = idb_alloc(DB_OPT_BASE);
	mapregstr_db = idb_alloc(DB_OPT_BASE);
	mapreg_ers = ers_new(sizeof(struct mapreg_save), "mapreg_sql.c::mapreg_ers", ERS_OPT_NONE);

	script_load_mapreg();

	iTimer->add_timer_func_list(script_autosave_mapreg, "script_autosave_mapreg");
	iTimer->add_timer_interval(iTimer->gettick() + MAPREG_AUTOSAVE_INTERVAL, script_autosave_mapreg, 0, 0, MAPREG_AUTOSAVE_INTERVAL);
}

bool mapreg_config_read(const char* w1, const char* w2) {
	if(!strcmpi(w1, "mapreg_db"))
		safestrncpy(mapreg_table, w2, sizeof(mapreg_table));
	else
		return false;

	return true;
}
