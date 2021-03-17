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

#include "int_pet.h"

#include "char/char.h"
#include "char/inter.h"
#include "char/mapif.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>

static struct inter_pet_interface inter_pet_s;
struct inter_pet_interface *inter_pet;

/**
 * Saves a pet to the SQL database.
 *
 * Table structure:
 * `pet` (`pet_id`, `class`, `name`, `account_id`, `char_id`, `level`, `egg_id`, `equip`, `intimate`, `hungry`, `rename_flag`, `incubate`, `autofeed`)
 *
 * @remark In case of newly created pet, the pet ID is not updated to reflect the newly assigned ID. The caller must do so.
 *
 * @param p The pet data to save.
 * @return The ID of the saved pet, or 0 in case of errors.
 *
 **/
static int inter_pet_tosql(const struct s_pet *p)
{
	nullpo_ret(p);

	struct SqlStmt *stmt = SQL->StmtMalloc(inter->sql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	int pet_id = 0;

	if (p->pet_id == 0) { // New pet.
		const char *query = "INSERT INTO `%s` "
			"(`class`, `name`, `account_id`, `char_id`, `level`, `egg_id`, `equip`, "
			"`intimate`, `hungry`, `rename_flag`, `incubate`, `autofeed`) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

		if (SQL_ERROR == SQL->StmtPrepare(stmt, query, pet_db) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT32, &p->class_, sizeof(p->class_)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_STRING, &p->name, strnlen(p->name, sizeof(p->name))) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 2, SQLDT_INT32, &p->account_id, sizeof(p->account_id)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 3, SQLDT_INT32, &p->char_id, sizeof(p->char_id)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 4, SQLDT_INT16, &p->level, sizeof(p->level)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 5, SQLDT_INT32, &p->egg_id, sizeof(p->egg_id)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 6, SQLDT_INT32, &p->equip, sizeof(p->equip)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 7, SQLDT_INT16, &p->intimate, sizeof(p->intimate)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 8, SQLDT_INT16, &p->hungry, sizeof(p->hungry)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 9, SQLDT_INT8, &p->rename_flag, sizeof(p->rename_flag)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 10, SQLDT_INT8, &p->incubate, sizeof(p->incubate)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 11, SQLDT_INT32, &p->autofeed, sizeof(p->autofeed)) ||
		    SQL_ERROR == SQL->StmtExecute(stmt)) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return 0;
		}

		pet_id = (int)SQL->LastInsertId(inter->sql_handle);
	} else { // Update pet.
		const char *query = "UPDATE `%s` SET "
			"`class`=?, `name`=?, `account_id`=?, `char_id`=?, `level`=?, `egg_id`=?, `equip`=?, "
			"`intimate`=?, `hungry`=?, `rename_flag`=?, `incubate`=?, `autofeed`=? "
			"WHERE `pet_id`=?";

		if (SQL_ERROR == SQL->StmtPrepare(stmt, query, pet_db) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT32, &p->class_, sizeof(p->class_)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 1, SQLDT_STRING, &p->name, strnlen(p->name, sizeof(p->name))) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 2, SQLDT_INT32, &p->account_id, sizeof(p->account_id)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 3, SQLDT_INT32, &p->char_id, sizeof(p->char_id)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 4, SQLDT_INT16, &p->level, sizeof(p->level)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 5, SQLDT_INT32, &p->egg_id, sizeof(p->egg_id)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 6, SQLDT_INT32, &p->equip, sizeof(p->equip)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 7, SQLDT_INT16, &p->intimate, sizeof(p->intimate)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 8, SQLDT_INT16, &p->hungry, sizeof(p->hungry)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 9, SQLDT_INT8, &p->rename_flag, sizeof(p->rename_flag)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 10, SQLDT_INT8, &p->incubate, sizeof(p->incubate)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 11, SQLDT_INT32, &p->autofeed, sizeof(p->autofeed)) ||
		    SQL_ERROR == SQL->StmtBindParam(stmt, 12, SQLDT_INT32, &p->pet_id, sizeof(p->pet_id)) ||
		    SQL_ERROR == SQL->StmtExecute(stmt)) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return 0;
		}

		pet_id = p->pet_id;
	}

	SQL->StmtFree(stmt);

	if (chr->show_save_log)
		ShowInfo("Pet saved %d - %s.\n", pet_id, p->name);

	return pet_id;
}

/**
 * Loads a pet's data from the SQL database.
 *
 * Table structure:
 * `pet` (`pet_id`, `class`, `name`, `account_id`, `char_id`, `level`, `egg_id`, `equip`, `intimate`, `hungry`, `rename_flag`, `incubate`, `autofeed`)
 *
 * @param pet_id The pet's ID.
 * @param p The pet data to save the SQL data in.
 * @return Always 0.
 *
 **/
static int inter_pet_fromsql(int pet_id, struct s_pet *p)
{
	nullpo_ret(p);

	struct SqlStmt *stmt = SQL->StmtMalloc(inter->sql_handle);

	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

#ifdef NOISY
	ShowInfo("Loading pet (%d)...\n",pet_id);
#endif

	memset(p, 0, sizeof(struct s_pet));

	const char *query = "SELECT "
		"`class`, `name`, `account_id`, `char_id`, `level`, `egg_id`, `equip`, "
		"`intimate`, `hungry`, `rename_flag`, `incubate`, `autofeed` "
		"FROM `%s` WHERE `pet_id`=?";

	if (SQL_ERROR == SQL->StmtPrepare(stmt, query, pet_db) ||
	    SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT32, &pet_id, sizeof(pet_id)) ||
	    SQL_ERROR == SQL->StmtExecute(stmt) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT32, &p->class_, sizeof(p->class_), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_STRING, &p->name, sizeof(p->name), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_INT32, &p->account_id, sizeof(p->account_id), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 3, SQLDT_INT32, &p->char_id, sizeof(p->char_id), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 4, SQLDT_INT16, &p->level, sizeof(p->level), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 5, SQLDT_INT32, &p->egg_id, sizeof(p->egg_id), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 6, SQLDT_INT32, &p->equip, sizeof(p->equip), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 7, SQLDT_INT16, &p->intimate, sizeof(p->intimate), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 8, SQLDT_INT16, &p->hungry, sizeof(p->hungry), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 9, SQLDT_INT8, &p->rename_flag, sizeof(p->rename_flag), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 10, SQLDT_INT8, &p->incubate, sizeof(p->incubate), NULL, NULL) ||
	    SQL_ERROR == SQL->StmtBindColumn(stmt, 11, SQLDT_INT32, &p->autofeed, sizeof(p->autofeed), NULL, NULL)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return 0;
	}

	if (SQL->StmtNumRows(stmt) < 1) {
		ShowError("inter_pet_fromsql: Requested non-existant pet ID: %d\n", pet_id);
		SQL->StmtFree(stmt);
		return 0;
	}

	if (SQL_ERROR == SQL->StmtNextRow(stmt)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return 0;
	}

	SQL->StmtFree(stmt);
	p->pet_id = pet_id;

	if (chr->show_save_log)
		ShowInfo("Pet loaded %d - %s.\n", pet_id, p->name);

	return 0;
}
//----------------------------------------------

static int inter_pet_sql_init(void)
{
	//memory alloc
	inter_pet->pt = (struct s_pet*)aCalloc(sizeof(struct s_pet), 1);
	return 0;
}

static void inter_pet_sql_final(void)
{
	if (inter_pet->pt) aFree(inter_pet->pt);
	return;
}
//----------------------------------
static int inter_pet_delete(int pet_id)
{
	ShowInfo("delete pet request: %d...\n",pet_id);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `pet_id`='%d'", pet_db, pet_id) )
		Sql_ShowDebug(inter->sql_handle);
	return 0;
}
//------------------------------------------------------

/**
 * Creates a new pet and inserts its data into the `pet` SQL table.
 *
 * @param account_id The pet's master's account ID.
 * @param char_id The pet's master's char ID.
 * @param pet_class The pet's class/monster ID.
 * @param pet_lv The pet's level.
 * @param pet_egg_id The pet's egg's item ID.
 * @param pet_equip The pet's equipment's item ID.
 * @param intimate The pet's intimacy value.
 * @param hungry The pet's hunger value.
 * @param rename_flag The pet's rename flag.
 * @param incubate The pet's incubate state.
 * @param pet_name The pet's name.
 * @return The created pet's data struct, or NULL in case of errors.
 *
 **/
static struct s_pet *inter_pet_create(int account_id, int char_id, int pet_class, int pet_lv, int pet_egg_id,
				      int pet_equip, short intimate, short hungry, char rename_flag,
				      char incubate, const char *pet_name)
{
	nullpo_retr(NULL, pet_name);

	memset(inter_pet->pt, 0, sizeof(struct s_pet));
	safestrncpy(inter_pet->pt->name, pet_name, NAME_LENGTH);
	inter_pet->pt->account_id = (incubate == 1) ? 0 : account_id;
	inter_pet->pt->char_id = (incubate == 1) ? 0 : char_id;
	inter_pet->pt->class_ = pet_class;
	inter_pet->pt->level = pet_lv;
	inter_pet->pt->egg_id = pet_egg_id;
	inter_pet->pt->equip = pet_equip;
	inter_pet->pt->intimate = cap_value(intimate, PET_INTIMACY_NONE, PET_INTIMACY_MAX);
	inter_pet->pt->hungry = cap_value(hungry, PET_HUNGER_STARVING, PET_HUNGER_STUFFED);
	inter_pet->pt->rename_flag = rename_flag;
	inter_pet->pt->incubate = incubate;
	inter_pet->pt->pet_id = 0; // Signal NEW pet.

	if ((inter_pet->pt->pet_id = inter_pet->tosql(inter_pet->pt)) != 0)
		return inter_pet->pt;

	return NULL;
}

static struct s_pet *inter_pet_load(int account_id, int char_id, int pet_id)
{
	memset(inter_pet->pt, 0, sizeof(struct s_pet));

	inter_pet->fromsql(pet_id, inter_pet->pt);

	if(inter_pet->pt!=NULL) {
		if (inter_pet->pt->incubate == 1) {
			inter_pet->pt->account_id = inter_pet->pt->char_id = 0;
			return inter_pet->pt;
		} else if (account_id == inter_pet->pt->account_id && char_id == inter_pet->pt->char_id) {
			return inter_pet->pt;
		} else {
			return NULL;
		}
	}

	return NULL;
}

static int inter_pet_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd, 0)){
	case 0x3080: mapif->parse_CreatePet(fd); break;
	case 0x3081: mapif->parse_LoadPet(fd); break;
	case 0x3082: mapif->parse_SavePet(fd); break;
	case 0x3083: mapif->parse_DeletePet(fd); break;
	default:
		return 0;
	}
	return 1;
}

void inter_pet_defaults(void)
{
	inter_pet = &inter_pet_s;

	inter_pet->pt = NULL;

	inter_pet->tosql = inter_pet_tosql;
	inter_pet->fromsql = inter_pet_fromsql;
	inter_pet->sql_init = inter_pet_sql_init;
	inter_pet->sql_final = inter_pet_sql_final;
	inter_pet->delete_ = inter_pet_delete;
	inter_pet->parse_frommap = inter_pet_parse_frommap;

	inter_pet->create = inter_pet_create;
	inter_pet->load = inter_pet_load;
}
