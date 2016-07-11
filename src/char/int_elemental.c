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

#include "int_elemental.h"

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

struct inter_elemental_interface inter_elemental_s;
struct inter_elemental_interface *inter_elemental;

/**
 * Creates a new elemental with the given data.
 *
 * @remark
 *   The elemental ID is expected to be 0, and will be filled with the newly
 *   assigned ID.
 *
 * @param[in,out] ele The new elemental's data.
 * @retval false in case of errors.
 */
bool mapif_elemental_create(struct s_elemental *ele)
{
	nullpo_retr(false, ele);
	Assert_retr(false, ele->elemental_id == 0);

	if (SQL_ERROR == SQL->Query(inter->sql_handle,
			"INSERT INTO `%s` (`char_id`,`class`,`mode`,`hp`,`sp`,`max_hp`,`max_sp`,`atk1`,`atk2`,`matk`,`aspd`,`def`,`mdef`,`flee`,`hit`,`life_time`)"
			"VALUES ('%d','%d','%u','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d')",
			elemental_db, ele->char_id, ele->class_, ele->mode, ele->hp, ele->sp, ele->max_hp, ele->max_sp, ele->atk,
			ele->atk2, ele->matk, ele->amotion, ele->def, ele->mdef, ele->flee, ele->hit, ele->life_time)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}
	ele->elemental_id = (int)SQL->LastInsertId(inter->sql_handle);
	return true;
}

/**
 * Saves an existing elemental.
 *
 * @param ele The elemental's data.
 * @retval false in case of errors.
 */
bool mapif_elemental_save(const struct s_elemental *ele)
{
	nullpo_retr(false, ele);
	Assert_retr(false, ele->elemental_id > 0);

	if (SQL_ERROR == SQL->Query(inter->sql_handle,
			"UPDATE `%s` SET `char_id` = '%d', `class` = '%d', `mode` = '%u', `hp` = '%d', `sp` = '%d',"
			"`max_hp` = '%d', `max_sp` = '%d', `atk1` = '%d', `atk2` = '%d', `matk` = '%d', `aspd` = '%d', `def` = '%d',"
			"`mdef` = '%d', `flee` = '%d', `hit` = '%d', `life_time` = '%d' WHERE `ele_id` = '%d'",
			elemental_db, ele->char_id, ele->class_, ele->mode, ele->hp, ele->sp, ele->max_hp, ele->max_sp, ele->atk, ele->atk2,
			ele->matk, ele->amotion, ele->def, ele->mdef, ele->flee, ele->hit, ele->life_time, ele->elemental_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}
	return true;
}

bool mapif_elemental_load(int ele_id, int char_id, struct s_elemental *ele) {
	char* data;

	nullpo_retr(false, ele);
	memset(ele, 0, sizeof(struct s_elemental));
	ele->elemental_id = ele_id;
	ele->char_id = char_id;

	if( SQL_ERROR == SQL->Query(inter->sql_handle,
		"SELECT `class`, `mode`, `hp`, `sp`, `max_hp`, `max_sp`, `atk1`, `atk2`, `matk`, `aspd`,"
		"`def`, `mdef`, `flee`, `hit`, `life_time` FROM `%s` WHERE `ele_id` = '%d' AND `char_id` = '%d'",
		elemental_db, ele_id, char_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	if( SQL_SUCCESS != SQL->NextRow(inter->sql_handle) ) {
		SQL->FreeResult(inter->sql_handle);
		return false;
	}

	SQL->GetData(inter->sql_handle,  0, &data, NULL); ele->class_ = atoi(data);
	SQL->GetData(inter->sql_handle,  1, &data, NULL); ele->mode = atoi(data);
	SQL->GetData(inter->sql_handle,  2, &data, NULL); ele->hp = atoi(data);
	SQL->GetData(inter->sql_handle,  3, &data, NULL); ele->sp = atoi(data);
	SQL->GetData(inter->sql_handle,  4, &data, NULL); ele->max_hp = atoi(data);
	SQL->GetData(inter->sql_handle,  5, &data, NULL); ele->max_sp = atoi(data);
	SQL->GetData(inter->sql_handle,  6, &data, NULL); ele->atk = atoi(data);
	SQL->GetData(inter->sql_handle,  7, &data, NULL); ele->atk2 = atoi(data);
	SQL->GetData(inter->sql_handle,  8, &data, NULL); ele->matk = atoi(data);
	SQL->GetData(inter->sql_handle,  9, &data, NULL); ele->amotion = atoi(data);
	SQL->GetData(inter->sql_handle, 10, &data, NULL); ele->def = atoi(data);
	SQL->GetData(inter->sql_handle, 11, &data, NULL); ele->mdef = atoi(data);
	SQL->GetData(inter->sql_handle, 12, &data, NULL); ele->flee = atoi(data);
	SQL->GetData(inter->sql_handle, 13, &data, NULL); ele->hit = atoi(data);
	SQL->GetData(inter->sql_handle, 14, &data, NULL); ele->life_time = atoi(data);
	SQL->FreeResult(inter->sql_handle);
	if( save_log )
		ShowInfo("Elemental loaded (%d - %d).\n", ele->elemental_id, ele->char_id);

	return true;
}

bool mapif_elemental_delete(int ele_id) {
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `ele_id` = '%d'", elemental_db, ele_id) ) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	return true;
}

void mapif_elemental_send(int fd, struct s_elemental *ele, unsigned char flag) {
	int size = sizeof(struct s_elemental) + 5;

	nullpo_retv(ele);
	WFIFOHEAD(fd,size);
	WFIFOW(fd,0) = 0x387c;
	WFIFOW(fd,2) = size;
	WFIFOB(fd,4) = flag;
	memcpy(WFIFOP(fd,5),ele,sizeof(struct s_elemental));
	WFIFOSET(fd,size);
}

void mapif_parse_elemental_create(int fd, const struct s_elemental *ele)
{
	struct s_elemental ele_;
	bool result;

	memcpy(&ele_, ele, sizeof(ele_));

	result = mapif->elemental_create(&ele_);
	mapif->elemental_send(fd, &ele_, result);
}

void mapif_parse_elemental_load(int fd, int ele_id, int char_id) {
	struct s_elemental ele;
	bool result = mapif->elemental_load(ele_id, char_id, &ele);
	mapif->elemental_send(fd, &ele, result);
}

void mapif_elemental_deleted(int fd, unsigned char flag) {
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x387d;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_elemental_delete(int fd, int ele_id) {
	bool result = mapif->elemental_delete(ele_id);
	mapif->elemental_deleted(fd, result);
}

void mapif_elemental_saved(int fd, unsigned char flag) {
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x387e;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_elemental_save(int fd, const struct s_elemental *ele)
{
	bool result = mapif->elemental_save(ele);
	mapif->elemental_saved(fd, result);
}

void inter_elemental_sql_init(void) {
	return;
}

void inter_elemental_sql_final(void) {
	return;
}

/*==========================================
 * Inter Packets
 *------------------------------------------*/
int inter_elemental_parse_frommap(int fd) {
	unsigned short cmd = RFIFOW(fd,0);

	switch (cmd) {
		case 0x307c: mapif->parse_elemental_create(fd, RFIFOP(fd,4)); break;
		case 0x307d: mapif->parse_elemental_load(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
		case 0x307e: mapif->parse_elemental_delete(fd, RFIFOL(fd,2)); break;
		case 0x307f: mapif->parse_elemental_save(fd, RFIFOP(fd,4)); break;
		default:
			return 0;
	}
	return 1;
}

void inter_elemental_defaults(void)
{
	inter_elemental = &inter_elemental_s;

	inter_elemental->sql_init = inter_elemental_sql_init;
	inter_elemental->sql_final = inter_elemental_sql_final;
	inter_elemental->parse_frommap = inter_elemental_parse_frommap;
}
