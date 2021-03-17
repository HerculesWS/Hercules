/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
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

#include "mapreg.h"

#include "map/map.h"
#include "map/script.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdlib.h>
#include <string.h>

static struct mapreg_interface mapreg_s; //!< Private interface structure.
struct mapreg_interface *mapreg; //!< Public interface structure.

/**
 * Looks up the value of a global integer variable using its unique ID.
 *
 * @param uid The variable's unique ID.
 * @return The variable's value or 0 if the variable does not exist.
 *
 **/
static int mapreg_get_num_reg(int64 uid)
{
	struct mapreg_save *var = i64db_get(mapreg->regs.vars, uid);
	return (var != NULL) ? var->u.i : 0;
}

/**
 * Looks up the value of a global string variable using its unique ID.
 *
 * @param uid The variable's unique ID.
 * @return The variable's value or NULL if the variable does not exist.
 *
 **/
static char *mapreg_get_str_reg(int64 uid)
{
	struct mapreg_save *var = i64db_get(mapreg->regs.vars, uid);
	return (var != NULL) ? var->u.str : NULL;
}

/**
 * Sets the value of a global integer variable.
 *
 * @param uid The variable's unique ID.
 * @param name The variable's name.
 * @param index The variable's array index.
 * @param value The variable's new value.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_set_num_db(int64 uid, const char *name, unsigned int index, int value)
{
	nullpo_retr(false, name);
	Assert_retr(false, *name != '\0');
	Assert_retr(false, strlen(name) <= SCRIPT_VARNAME_LENGTH);

	if (value == 0)
		return mapreg->delete_num_db(uid, name, index);

	struct mapreg_save *var = i64db_get(mapreg->regs.vars, uid);

	// Update variable.
	if (var != NULL) {
		var->u.i = value;

		if (script->is_permanent_variable(name)) {
			var->save = true;
			mapreg->dirty = true;
		}

		return true;
	}

	// Add new variable.
	if (index != 0)
		script->array_update(&mapreg->regs, uid, false);

	var = ers_alloc(mapreg->ers, struct mapreg_save);
	var->u.i = value;
	var->uid = uid;
	var->save = false;
	var->is_string = false;
	i64db_put(mapreg->regs.vars, uid, var);

	if (script->is_permanent_variable(name) && !mapreg->skip_insert) {
		struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

		if (stmt == NULL) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}

		const char *query = "INSERT INTO `%s` (`key`, `index`, `value`) VALUES (?, ?, ?)";

		if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->num_db)
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_STRING, name, strlen(name))
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_UINT32, &index, sizeof(index))
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 2, SQLDT_INT32, &value, sizeof(value))
		    || SQL_ERROR == SQL->StmtExecute(stmt)) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return false;
		}

		SQL->StmtFree(stmt);
	}

	return true;
}

/**
 * Deletes a global integer variable.
 *
 * @param uid The variable's unique ID.
 * @param name The variable's name.
 * @param index The variable's array index.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_delete_num_db(int64 uid, const char *name, unsigned int index)
{
	nullpo_retr(false, name);
	Assert_retr(false, *name != '\0');
	Assert_retr(false, strlen(name) <= SCRIPT_VARNAME_LENGTH);

	struct mapreg_save *var = i64db_get(mapreg->regs.vars, uid);

	if (var != NULL)
		ers_free(mapreg->ers, var);

	if (index != 0)
		script->array_update(&mapreg->regs, uid, true);

	i64db_remove(mapreg->regs.vars, uid);

	if (script->is_permanent_variable(name)) {
		struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

		if (stmt == NULL) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}

		const char *query = "DELETE FROM `%s` WHERE `key`=? AND `index`=?";

		if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->num_db)
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_STRING, name, strlen(name))
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_UINT32, &index, sizeof(index))
		    || SQL_ERROR == SQL->StmtExecute(stmt)) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return false;
		}

		SQL->StmtFree(stmt);
	}

	return true;
}

/**
 * Sets the value of a global integer variable or deletes it if passed value is 0.
 *
 * @param uid The variable's unique ID.
 * @param val The variable's new value.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_set_num(int64 uid, int val)
{
	unsigned int index = script_getvaridx(uid);
	const char *name = script->get_str(script_getvarid(uid));

	if (val != 0)
		return mapreg->set_num_db(uid, name, index, val);
	else
		return mapreg->delete_num_db(uid, name, index);
}

/**
 * Sets the value of a global string variable.
 *
 * @param uid The variable's unique ID.
 * @param name The variable's name.
 * @param index The variable's array index.
 * @param value The variable's new value.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_set_str_db(int64 uid, const char *name, unsigned int index, const char *value)
{
	nullpo_retr(false, name);
	Assert_retr(false, *name != '\0');
	Assert_retr(false, strlen(name) <= SCRIPT_VARNAME_LENGTH);

	if (value == NULL || *value == '\0')
		return mapreg->delete_str_db(uid, name, index);

	if (script->is_permanent_variable(name))
		Assert_retr(false, strlen(value) <= SCRIPT_STRING_VAR_LENGTH);

	struct mapreg_save *var = i64db_get(mapreg->regs.vars, uid);

	// Update variable.
	if (var != NULL) {
		if (var->u.str != NULL)
			aFree(var->u.str);

		var->u.str = aStrdup(value);

		if (script->is_permanent_variable(name)) {
			var->save = true;
			mapreg->dirty = true;
		}

		return true;
	}

	// Add new variable.
	if (index != 0)
		script->array_update(&mapreg->regs, uid, false);

	var = ers_alloc(mapreg->ers, struct mapreg_save);
	var->u.str = aStrdup(value);
	var->uid = uid;
	var->save = false;
	var->is_string = true;
	i64db_put(mapreg->regs.vars, uid, var);

	if (script->is_permanent_variable(name) && !mapreg->skip_insert) {
		struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

		if (stmt == NULL) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}

		const char *query = "INSERT INTO `%s` (`key`, `index`, `value`) VALUES (?, ?, ?)";

		if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->str_db)
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_STRING, name, strlen(name))
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_UINT32, &index, sizeof(index))
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 2, SQLDT_STRING, value, strlen(value))
		    || SQL_ERROR == SQL->StmtExecute(stmt)) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return false;
		}

		SQL->StmtFree(stmt);
	}

	return true;
}

/**
 * Deletes a global string variable.
 *
 * @param uid The variable's unique ID.
 * @param name The variable's name.
 * @param index The variable's array index.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_delete_str_db(int64 uid, const char *name, unsigned int index)
{
	nullpo_retr(false, name);
	Assert_retr(false, *name != '\0');
	Assert_retr(false, strlen(name) <= SCRIPT_VARNAME_LENGTH);

	struct mapreg_save *var = i64db_get(mapreg->regs.vars, uid);

	if (var != NULL) {
		if (var->u.str != NULL)
			aFree(var->u.str);

		ers_free(mapreg->ers, var);
	}

	if (index != 0)
		script->array_update(&mapreg->regs, uid, true);

	i64db_remove(mapreg->regs.vars, uid);

	if (script->is_permanent_variable(name)) {
		struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

		if (stmt == NULL) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}

		const char *query = "DELETE FROM `%s` WHERE `key`=? AND `index`=?";

		if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->str_db)
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_STRING, name, strlen(name))
		    || SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_UINT32, &index, sizeof(index))
		    || SQL_ERROR == SQL->StmtExecute(stmt)) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return false;
		}

		SQL->StmtFree(stmt);
	}

	return true;
}

/**
 * Sets the value of a global string variable or deletes it if passed value is NULL or an empty string.
 *
 * @param uid The variable's unique ID.
 * @param str The variable's new value.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_set_str(int64 uid, const char *str)
{
	unsigned int index = script_getvaridx(uid);
	const char *name = script->get_str(script_getvarid(uid));

	if (str != NULL && *str != '\0')
		return mapreg->set_str_db(uid, name, index, str);
	else
		return mapreg->delete_str_db(uid, name, index);
}

/**
 * Loads permanent global interger variables from the database.
 *
 **/
static void mapreg_load_num_db(void)
{
	struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return;
	}

	const char *query = "SELECT `key`, `index`, `value` FROM `%s`";
	char name[SCRIPT_VARNAME_LENGTH + 1];
	unsigned int index;
	int value;

	if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->num_db)
	    || SQL_ERROR == SQL->StmtExecute(stmt)
	    || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &name, sizeof(name), NULL, NULL)
	    || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_UINT32, &index, sizeof(index), NULL, NULL)
	    || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_INT32, &value, sizeof(value), NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}

	if (SQL->StmtNumRows(stmt) < 1) {
		SQL->StmtFree(stmt);
		return;
	}

	mapreg->skip_insert = true;

	while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
		int var_key = script->add_variable(name);
		int64 uid = reference_uid(var_key, index);

		if (i64db_exists(mapreg->regs.vars, uid)) {
			ShowWarning("mapreg_load_num_db: Duplicate! '%s' => '%d' Skipping...\n", name, value);
			continue;
		}

		mapreg->setreg(uid, value);
	}

	mapreg->skip_insert = false;
	SQL->StmtFree(stmt);
}

/**
 * Loads permanent global string variables from the database.
 *
 **/
static void mapreg_load_str_db(void)
{
	struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return;
	}

	const char *query = "SELECT `key`, `index`, `value` FROM `%s`";
	char name[SCRIPT_VARNAME_LENGTH + 1];
	unsigned int index;
	char value[SCRIPT_STRING_VAR_LENGTH + 1];

	if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->str_db)
	    || SQL_ERROR == SQL->StmtExecute(stmt)
	    || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &name, sizeof(name), NULL, NULL)
	    || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_UINT32, &index, sizeof(index), NULL, NULL)
	    || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_STRING, &value, sizeof(value), NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}

	if (SQL->StmtNumRows(stmt) < 1) {
		SQL->StmtFree(stmt);
		return;
	}

	mapreg->skip_insert = true;

	while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
		int var_key = script->add_variable(name);
		int64 uid = reference_uid(var_key, index);

		if (i64db_exists(mapreg->regs.vars, uid)) {
			ShowWarning("mapreg_load_str_db: Duplicate! '%s' => '%s' Skipping...\n", name, value);
			continue;
		}

		mapreg->setregstr(uid, value);
	}

	mapreg->skip_insert = false;
	SQL->StmtFree(stmt);
}

/**
 * Loads permanent global variables from the database.
 *
 **/
static void mapreg_load(void)
{
	mapreg->load_num_db();
	mapreg->load_str_db();
	mapreg->dirty = false;
}

/**
 * Saves a permanent global integer variable to the database.
 *
 * @param name The variable's name.
 * @param index The variable's array index.
 * @param value The variable's value.
 *
 **/
static void mapreg_save_num_db(const char *name, unsigned int index, int value)
{
	nullpo_retv(name);
	Assert_retv(*name != '\0');
	Assert_retv(strlen(name) <= SCRIPT_VARNAME_LENGTH);

	struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return;
	}

	const char *query = "UPDATE `%s` SET `value`=? WHERE `key`=? AND `index`=? LIMIT 1";

	if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->num_db)
	    || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT32, &value, sizeof(value))
	    || SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_STRING, name, strlen(name))
	    || SQL_ERROR == SQL->StmtBindParam(stmt, 2, SQLDT_UINT32, &index, sizeof(index))
	    || SQL_ERROR == SQL->StmtExecute(stmt)) {
		SqlStmt_ShowDebug(stmt);
	}

	SQL->StmtFree(stmt);
}

/**
 * Saves a permanent global string variable to the database.
 *
 * @param name The variable's name.
 * @param index The variable's array index.
 * @param value The variable's value.
 *
 **/
static void mapreg_save_str_db(const char *name, unsigned int index, const char *value)
{
	nullpo_retv(name);
	nullpo_retv(value);
	Assert_retv(*name != '\0');
	Assert_retv(strlen(name) <= SCRIPT_VARNAME_LENGTH);
	Assert_retv(*value != '\0');
	Assert_retv(strlen(value) <= SCRIPT_STRING_VAR_LENGTH);

	struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return;
	}

	const char *query = "UPDATE `%s` SET `value`=? WHERE `key`=? AND `index`=? LIMIT 1";

	if (SQL_ERROR == SQL->StmtPrepare(stmt, query, mapreg->str_db)
	    || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_STRING, value, strlen(value))
	    || SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_STRING, name, strlen(name))
	    || SQL_ERROR == SQL->StmtBindParam(stmt, 2, SQLDT_UINT32, &index, sizeof(index))
	    || SQL_ERROR == SQL->StmtExecute(stmt)) {
		SqlStmt_ShowDebug(stmt);
	}

	SQL->StmtFree(stmt);
}

/**
 * Saves permanent global variables to the database.
 *
 **/
static void mapreg_save(void)
{
	if (mapreg->dirty) {
		struct DBIterator *iter = db_iterator(mapreg->regs.vars);
		struct mapreg_save *var = NULL;

		for (var = dbi_first(iter); dbi_exists(iter); var = dbi_next(iter)) {
			if (var->save) {
				int index = script_getvaridx(var->uid);
				const char *name = script->get_str(script_getvarid(var->uid));

				if (!var->is_string)
					mapreg->save_num_db(name, index, var->u.i);
				else
					mapreg->save_str_db(name, index, var->u.str);

				var->save = false;
			}
		}

		dbi_destroy(iter);
		mapreg->dirty = false;
	}
}

/**
 * Timer event to auto-save permanent global variables.
 *
 * @see timer->do_timer()
 *
 * @param tid Unused.
 * @param tick Unused.
 * @param id Unused.
 * @param data Unused.
 * @return Always 0.
 *
 **/
static int mapreg_save_timer(int tid, int64 tick, int id, intptr_t data)
{
	mapreg->save();
	return 0;
}

/**
 * Destroys a mapreg_save structure and frees the contained string, if any.
 *
 * @see DBApply
 *
 * @param key Unused.
 * @param data The DB data holding the mapreg_save data.
 * @param ap Unused.
 * @return 0 on success, otherwise 1.
 *
 **/
static int mapreg_destroy_reg(union DBKey key, struct DBData *data, va_list ap)
{
	nullpo_retr(1, data);

	if (data->type != DB_DATA_PTR) // Sanity check
		return 1;

	struct mapreg_save *var = DB->data2ptr(data);

	if (var == NULL)
		return 1;

	if (var->is_string && var->u.str != NULL)
		aFree(var->u.str);

	ers_free(mapreg->ers, var);
	return 0;
}

/**
 * Reloads permanent global variables, saving them to the database beforehand.
 *
 * This has the effect of clearing the temporary global variables and reloading the permanent ones.
 *
 **/
static void mapreg_reload(void)
{
	mapreg->save();
	mapreg->regs.vars->clear(mapreg->regs.vars, mapreg->destroyreg);

	if (mapreg->regs.arrays != NULL) {
		mapreg->regs.arrays->destroy(mapreg->regs.arrays, script->array_free_db);
		mapreg->regs.arrays = NULL;
	}

	mapreg->load();
}

/**
 * Loads the mapreg database table names from configuration file.
 *
 * @param filename Path to configuration file. (Used in error and warning messages).
 * @param config The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 * @return True on success, otherwise false.
 *
 **/
static bool mapreg_config_read_registry(const char *filename, const struct config_setting_t *config, bool imported)
{
	nullpo_retr(false, filename);
	nullpo_retr(false, config);
	
	bool ret_val = true;
	size_t sz = sizeof(mapreg->num_db);
	int result = libconfig->setting_lookup_mutable_string(config, "map_reg_num_db", mapreg->num_db, sz);

	if (result != CONFIG_TRUE && !imported) {
		ShowError("%s: inter_configuration/database_names/registry/map_reg_num_db was not found in %s!\n",
			  __func__, filename);
		ret_val = false;
	}

	sz = sizeof(mapreg->str_db);
	result = libconfig->setting_lookup_mutable_string(config, "map_reg_str_db", mapreg->str_db, sz);

	if (result != CONFIG_TRUE && !imported) {
		ShowError("%s: inter_configuration/database_names/registry/map_reg_str_db was not found in %s!\n",
			  __func__, filename);
		ret_val = false;
	}

	return ret_val;
}

/**
 * Saves permanent global variables to the database and frees all the memory they use afterwards.
 *
 **/
static void mapreg_final(void)
{
	mapreg->save();
	mapreg->regs.vars->destroy(mapreg->regs.vars, mapreg->destroyreg);
	ers_destroy(mapreg->ers);

	if (mapreg->regs.arrays != NULL)
		mapreg->regs.arrays->destroy(mapreg->regs.arrays, script->array_free_db);
}

/**
 * Allocates memory for permanent global variables, loads them from the database and initializes the auto-save timer.
 *
 **/
static void mapreg_init(void)
{
	mapreg->regs.vars = i64db_alloc(DB_OPT_BASE);
	mapreg->ers = ers_new(sizeof(struct mapreg_save), "mapreg_sql.c::mapreg_ers", ERS_OPT_CLEAN);
	mapreg->load();
	timer->add_func_list(mapreg->save_timer, "mapreg_save_timer");
	timer->add_interval(timer->gettick() + MAPREG_AUTOSAVE_INTERVAL, mapreg->save_timer, 0, 0, MAPREG_AUTOSAVE_INTERVAL);
}

/**
 * Initializes the mapreg interface defaults.
 *
 **/
void mapreg_defaults(void)
{
	/** Interface structure. **/
	mapreg = &mapreg_s;

	/** Interface variables. **/
	mapreg->ers = NULL;
	mapreg->regs.vars = NULL;
	mapreg->regs.arrays = NULL;
	mapreg->dirty = false;
	mapreg->skip_insert = false;
	safestrncpy(mapreg->num_db, "map_reg_num_db", sizeof(mapreg->num_db));
	safestrncpy(mapreg->str_db, "map_reg_str_db", sizeof(mapreg->str_db));

	/** Interface functions. **/
	mapreg->readreg = mapreg_get_num_reg;
	mapreg->readregstr = mapreg_get_str_reg;
	mapreg->set_num_db = mapreg_set_num_db;
	mapreg->delete_num_db = mapreg_delete_num_db;
	mapreg->setreg = mapreg_set_num;
	mapreg->set_str_db = mapreg_set_str_db;
	mapreg->delete_str_db = mapreg_delete_str_db;
	mapreg->setregstr = mapreg_set_str;
	mapreg->load_num_db = mapreg_load_num_db;
	mapreg->load_str_db = mapreg_load_str_db;
	mapreg->load = mapreg_load;
	mapreg->save_num_db = mapreg_save_num_db;
	mapreg->save_str_db = mapreg_save_str_db;
	mapreg->save = mapreg_save;
	mapreg->save_timer = mapreg_save_timer;
	mapreg->destroyreg = mapreg_destroy_reg;
	mapreg->reload = mapreg_reload;
	mapreg->config_read_registry = mapreg_config_read_registry;
	mapreg->final = mapreg_final;
	mapreg->init = mapreg_init;
}
