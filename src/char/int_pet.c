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

struct inter_pet_interface inter_pet_s;
struct inter_pet_interface *inter_pet;

/**
 * Saves a pet to the SQL database.
 *
 * @remark
 *   In case of newly created pet, the pet ID is not updated to reflect the
 *   newly assigned ID.  The caller must do so.
 *
 * @param p The pet data to save.
 * @return The ID of the saved pet.
 * @retval 0 in case of errors.
 */
int inter_pet_tosql(const struct s_pet *p)
{
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`)
	char esc_name[NAME_LENGTH*2+1];// escaped pet name
	int pet_id = 0, hungry = 0, intimate = 0;

	nullpo_ret(p);

	SQL->EscapeStringLen(inter->sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));
	hungry = cap_value(p->hungry, 0, 100);
	intimate = cap_value(p->intimate, 0, 1000);

	if (p->pet_id == 0) {
		// New pet.
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` "
				"(`class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`) "
				"VALUES ('%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
				pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
				p->equip, intimate, hungry, p->rename_flag, p->incubate)) {
			Sql_ShowDebug(inter->sql_handle);
			return 0;
		}
		pet_id = (int)SQL->LastInsertId(inter->sql_handle);
	} else {
		// Update pet.
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incubate`='%d' WHERE `pet_id`='%d'",
				pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
				p->equip, intimate, hungry, p->rename_flag, p->incubate, p->pet_id)) {
			Sql_ShowDebug(inter->sql_handle);
			return 0;
		}
		pet_id = p->pet_id;
	}

	if (save_log)
		ShowInfo("Pet saved %d - %s.\n", pet_id, p->name);

	return pet_id;
}

int inter_pet_fromsql(int pet_id, struct s_pet* p)
{
	char* data;
	size_t len;

#ifdef NOISY
	ShowInfo("Loading pet (%d)...\n",pet_id);
#endif
	nullpo_ret(p);
	memset(p, 0, sizeof(struct s_pet));

	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`)

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate` FROM `%s` WHERE `pet_id`='%d'", pet_db, pet_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return 0;
	}

	if( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		p->pet_id = pet_id;
		SQL->GetData(inter->sql_handle,  1, &data, NULL); p->class_ = atoi(data);
		SQL->GetData(inter->sql_handle,  2, &data, &len); memcpy(p->name, data, min(len, NAME_LENGTH));
		SQL->GetData(inter->sql_handle,  3, &data, NULL); p->account_id = atoi(data);
		SQL->GetData(inter->sql_handle,  4, &data, NULL); p->char_id = atoi(data);
		SQL->GetData(inter->sql_handle,  5, &data, NULL); p->level = atoi(data);
		SQL->GetData(inter->sql_handle,  6, &data, NULL); p->egg_id = atoi(data);
		SQL->GetData(inter->sql_handle,  7, &data, NULL); p->equip = atoi(data);
		SQL->GetData(inter->sql_handle,  8, &data, NULL); p->intimate = atoi(data);
		SQL->GetData(inter->sql_handle,  9, &data, NULL); p->hungry = atoi(data);
		SQL->GetData(inter->sql_handle, 10, &data, NULL); p->rename_flag = atoi(data);
		SQL->GetData(inter->sql_handle, 11, &data, NULL); p->incubate = atoi(data);

		SQL->FreeResult(inter->sql_handle);

		p->hungry = cap_value(p->hungry, 0, 100);
		p->intimate = cap_value(p->intimate, 0, 1000);

		if( save_log )
			ShowInfo("Pet loaded (%d - %s).\n", pet_id, p->name);
	}
	return 0;
}
//----------------------------------------------

int inter_pet_sql_init(void) {
	//memory alloc
	inter_pet->pt = (struct s_pet*)aCalloc(sizeof(struct s_pet), 1);
	return 0;
}
void inter_pet_sql_final(void) {
	if (inter_pet->pt) aFree(inter_pet->pt);
	return;
}
//----------------------------------
int inter_pet_delete(int pet_id) {
	ShowInfo("delete pet request: %d...\n",pet_id);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `pet_id`='%d'", pet_db, pet_id) )
		Sql_ShowDebug(inter->sql_handle);
	return 0;
}
//------------------------------------------------------
int mapif_pet_created(int fd, int account_id, struct s_pet *p)
{
	WFIFOHEAD(fd, 12);
	WFIFOW(fd, 0) = 0x3880;
	WFIFOL(fd, 2) = account_id;
	if(p!=NULL){
		WFIFOW(fd, 6) = p->class_;
		WFIFOL(fd, 8) = p->pet_id;
		ShowInfo("int_pet: created pet %d - %s\n", p->pet_id, p->name);
	}else{
		WFIFOB(fd, 6) = 0;
		WFIFOL(fd, 8) = 0;
	}
	WFIFOSET(fd, 12);

	return 0;
}

int mapif_pet_info(int fd, int account_id, struct s_pet *p)
{
	nullpo_ret(p);
	WFIFOHEAD(fd, sizeof(struct s_pet) + 9);
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=0;
	memcpy(WFIFOP(fd, 9), p, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int mapif_pet_noinfo(int fd, int account_id)
{
	WFIFOHEAD(fd, sizeof(struct s_pet) + 9);
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=1;
	memset(WFIFOP(fd, 9), 0, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int mapif_save_pet_ack(int fd, int account_id, int flag)
{
	WFIFOHEAD(fd, 7);
	WFIFOW(fd, 0) =0x3882;
	WFIFOL(fd, 2) =account_id;
	WFIFOB(fd, 6) =flag;
	WFIFOSET(fd, 7);

	return 0;
}

int mapif_delete_pet_ack(int fd, int flag)
{
	WFIFOHEAD(fd, 3);
	WFIFOW(fd, 0) =0x3883;
	WFIFOB(fd, 2) =flag;
	WFIFOSET(fd, 3);

	return 0;
}

int mapif_create_pet(int fd, int account_id, int char_id, short pet_class, short pet_lv, short pet_egg_id,
	short pet_equip, short intimate, short hungry, char rename_flag, char incubate, const char *pet_name)
{
	nullpo_ret(pet_name);
	memset(inter_pet->pt, 0, sizeof(struct s_pet));
	safestrncpy(inter_pet->pt->name, pet_name, NAME_LENGTH);
	if(incubate == 1)
		inter_pet->pt->account_id = inter_pet->pt->char_id = 0;
	else {
		inter_pet->pt->account_id = account_id;
		inter_pet->pt->char_id = char_id;
	}
	inter_pet->pt->class_ = pet_class;
	inter_pet->pt->level = pet_lv;
	inter_pet->pt->egg_id = pet_egg_id;
	inter_pet->pt->equip = pet_equip;
	inter_pet->pt->intimate = intimate;
	inter_pet->pt->hungry = hungry;
	inter_pet->pt->rename_flag = rename_flag;
	inter_pet->pt->incubate = incubate;

	if(inter_pet->pt->hungry < 0)
		inter_pet->pt->hungry = 0;
	else if(inter_pet->pt->hungry > 100)
		inter_pet->pt->hungry = 100;
	if(inter_pet->pt->intimate < 0)
		inter_pet->pt->intimate = 0;
	else if(inter_pet->pt->intimate > 1000)
		inter_pet->pt->intimate = 1000;

	inter_pet->pt->pet_id = 0; //Signal NEW pet.
	if ((inter_pet->pt->pet_id = inter_pet->tosql(inter_pet->pt)) != 0)
		mapif->pet_created(fd, account_id, inter_pet->pt);
	else //Failed...
		mapif->pet_created(fd, account_id, NULL);

	return 0;
}

int mapif_load_pet(int fd, int account_id, int char_id, int pet_id)
{
	memset(inter_pet->pt, 0, sizeof(struct s_pet));

	inter_pet->fromsql(pet_id, inter_pet->pt);

	if(inter_pet->pt!=NULL) {
		if(inter_pet->pt->incubate == 1) {
			inter_pet->pt->account_id = inter_pet->pt->char_id = 0;
			mapif->pet_info(fd, account_id, inter_pet->pt);
		}
		else if(account_id == inter_pet->pt->account_id && char_id == inter_pet->pt->char_id)
			mapif->pet_info(fd, account_id, inter_pet->pt);
		else
			mapif->pet_noinfo(fd, account_id);
	}
	else
		mapif->pet_noinfo(fd, account_id);

	return 0;
}

int mapif_save_pet(int fd, int account_id, const struct s_pet *data)
{
	//here process pet save request.
	int len;
	nullpo_ret(data);
	RFIFOHEAD(fd);
	len=RFIFOW(fd, 2);
	if (sizeof(struct s_pet) != len-8) {
		ShowError("inter pet: data size mismatch: %d != %"PRIuS"\n", len-8, sizeof(struct s_pet));
		return 0;
	}

	inter_pet->tosql(data);
	mapif->save_pet_ack(fd, account_id, 0);

	return 0;
}

int mapif_delete_pet(int fd, int pet_id)
{
	mapif->delete_pet_ack(fd, inter_pet->delete_(pet_id));

	return 0;
}

int mapif_parse_CreatePet(int fd)
{
	RFIFOHEAD(fd);
	mapif->create_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOW(fd, 10), RFIFOW(fd, 12), RFIFOW(fd, 14), RFIFOW(fd, 16), RFIFOW(fd, 18),
		RFIFOW(fd, 20), RFIFOB(fd, 22), RFIFOB(fd, 23), RFIFOP(fd, 24));
	return 0;
}

int mapif_parse_LoadPet(int fd)
{
	RFIFOHEAD(fd);
	mapif->load_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
	return 0;
}

int mapif_parse_SavePet(int fd)
{
	RFIFOHEAD(fd);
	mapif->save_pet(fd, RFIFOL(fd, 4), RFIFOP(fd, 8));
	return 0;
}

int mapif_parse_DeletePet(int fd)
{
	RFIFOHEAD(fd);
	mapif->delete_pet(fd, RFIFOL(fd, 2));
	return 0;
}

int inter_pet_parse_frommap(int fd)
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
}
