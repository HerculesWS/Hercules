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

#include "int_mercenary.h"

#include "char/char.h"
#include "char/inter.h"
#include "char/mapif.h"
#include "common/cbasetypes.h"
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

struct inter_mercenary_interface inter_mercenary_s;
struct inter_mercenary_interface *inter_mercenary;

bool inter_mercenary_owner_fromsql(int char_id, struct mmo_charstatus *status)
{
	char* data;

	nullpo_ret(status);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith` FROM `%s` WHERE `char_id` = '%d'", mercenary_owner_db, char_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	if( SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
	{
		SQL->FreeResult(inter->sql_handle);
		return false;
	}

	SQL->GetData(inter->sql_handle,  0, &data, NULL); status->mer_id = atoi(data);
	SQL->GetData(inter->sql_handle,  1, &data, NULL); status->arch_calls = atoi(data);
	SQL->GetData(inter->sql_handle,  2, &data, NULL); status->arch_faith = atoi(data);
	SQL->GetData(inter->sql_handle,  3, &data, NULL); status->spear_calls = atoi(data);
	SQL->GetData(inter->sql_handle,  4, &data, NULL); status->spear_faith = atoi(data);
	SQL->GetData(inter->sql_handle,  5, &data, NULL); status->sword_calls = atoi(data);
	SQL->GetData(inter->sql_handle,  6, &data, NULL); status->sword_faith = atoi(data);
	SQL->FreeResult(inter->sql_handle);

	return true;
}

bool inter_mercenary_owner_tosql(int char_id, struct mmo_charstatus *status)
{
	nullpo_ret(status);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`char_id`, `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith`) VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
		mercenary_owner_db, char_id, status->mer_id, status->arch_calls, status->arch_faith, status->spear_calls, status->spear_faith, status->sword_calls, status->sword_faith) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	return true;
}

bool inter_mercenary_owner_delete(int char_id)
{
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", mercenary_owner_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", mercenary_db, char_id) )
		Sql_ShowDebug(inter->sql_handle);

	return true;
}

/**
 * Creates a new mercenary with the given data.
 *
 * @remark
 *   The mercenary ID is expected to be 0, and will be filled with the newly
 *   assigned ID.
 *
 * @param[in,out] merc The new mercenary's data.
 * @retval false in case of errors.
 */
bool mapif_mercenary_create(struct s_mercenary *merc)
{
	nullpo_retr(false, merc);
	Assert_retr(false, merc->mercenary_id == 0);

	if (SQL_ERROR == SQL->Query(inter->sql_handle,
			"INSERT INTO `%s` (`char_id`,`class`,`hp`,`sp`,`kill_counter`,`life_time`) VALUES ('%d','%d','%d','%d','%u','%u')",
			mercenary_db, merc->char_id, merc->class_, merc->hp, merc->sp, merc->kill_count, merc->life_time)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}
	merc->mercenary_id = (int)SQL->LastInsertId(inter->sql_handle);

	return true;
}

/**
 * Saves an existing mercenary.
 *
 * @param merc The mercenary's data.
 * @retval false in case of errors.
 */
bool mapif_mercenary_save(const struct s_mercenary *merc)
{
	nullpo_retr(false, merc);
	Assert_retr(false, merc->mercenary_id > 0);

	if (SQL_ERROR == SQL->Query(inter->sql_handle,
			"UPDATE `%s` SET `char_id` = '%d', `class` = '%d', `hp` = '%d', `sp` = '%d', `kill_counter` = '%u', `life_time` = '%u' WHERE `mer_id` = '%d'",
			mercenary_db, merc->char_id, merc->class_, merc->hp, merc->sp, merc->kill_count, merc->life_time, merc->mercenary_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	return true;
}

bool mapif_mercenary_load(int merc_id, int char_id, struct s_mercenary *merc)
{
	char* data;

	nullpo_ret(merc);
	memset(merc, 0, sizeof(struct s_mercenary));
	merc->mercenary_id = merc_id;
	merc->char_id = char_id;

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `class`, `hp`, `sp`, `kill_counter`, `life_time` FROM `%s` WHERE `mer_id` = '%d' AND `char_id` = '%d'", mercenary_db, merc_id, char_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	if( SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
	{
		SQL->FreeResult(inter->sql_handle);
		return false;
	}

	SQL->GetData(inter->sql_handle,  0, &data, NULL); merc->class_ = atoi(data);
	SQL->GetData(inter->sql_handle,  1, &data, NULL); merc->hp = atoi(data);
	SQL->GetData(inter->sql_handle,  2, &data, NULL); merc->sp = atoi(data);
	SQL->GetData(inter->sql_handle,  3, &data, NULL); merc->kill_count = atoi(data);
	SQL->GetData(inter->sql_handle,  4, &data, NULL); merc->life_time = atoi(data);
	SQL->FreeResult(inter->sql_handle);
	if (chr->show_save_log)
		ShowInfo("Mercenary loaded (%d - %d).\n", merc->mercenary_id, merc->char_id);

	return true;
}

bool mapif_mercenary_delete(int merc_id)
{
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mer_id` = '%d'", mercenary_db, merc_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	return true;
}

void mapif_mercenary_send(int fd, struct s_mercenary *merc, unsigned char flag)
{
	int size = sizeof(struct s_mercenary) + 5;

	nullpo_retv(merc);
	WFIFOHEAD(fd,size);
	WFIFOW(fd,0) = 0x3870;
	WFIFOW(fd,2) = size;
	WFIFOB(fd,4) = flag;
	memcpy(WFIFOP(fd,5),merc,sizeof(struct s_mercenary));
	WFIFOSET(fd,size);
}

void mapif_parse_mercenary_create(int fd, const struct s_mercenary *merc)
{
	struct s_mercenary merc_;
	bool result;

	memcpy(&merc_, merc, sizeof(merc_));

	result = mapif->mercenary_create(&merc_);
	mapif->mercenary_send(fd, &merc_, result);
}

void mapif_parse_mercenary_load(int fd, int merc_id, int char_id)
{
	struct s_mercenary merc;
	bool result = mapif->mercenary_load(merc_id, char_id, &merc);
	mapif->mercenary_send(fd, &merc, result);
}

void mapif_mercenary_deleted(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3871;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_mercenary_delete(int fd, int merc_id)
{
	bool result = mapif->mercenary_delete(merc_id);
	mapif->mercenary_deleted(fd, result);
}

void mapif_mercenary_saved(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3872;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_mercenary_save(int fd, struct s_mercenary* merc)
{
	bool result = mapif->mercenary_save(merc);
	mapif->mercenary_saved(fd, result);
}

int inter_mercenary_sql_init(void)
{
	return 0;
}
void inter_mercenary_sql_final(void)
{
	return;
}

/*==========================================
 * Inter Packets
 *------------------------------------------*/
int inter_mercenary_parse_frommap(int fd)
{
	unsigned short cmd = RFIFOW(fd,0);

	switch (cmd) {
		case 0x3070: mapif->parse_mercenary_create(fd, RFIFOP(fd,4)); break;
		case 0x3071: mapif->parse_mercenary_load(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
		case 0x3072: mapif->parse_mercenary_delete(fd, RFIFOL(fd,2)); break;
		case 0x3073: mapif->parse_mercenary_save(fd, RFIFOP(fd,4)); break;
		default:
			return 0;
	}
	return 1;
}

void inter_mercenary_defaults(void)
{
	inter_mercenary = &inter_mercenary_s;

	inter_mercenary->owner_fromsql = inter_mercenary_owner_fromsql;
	inter_mercenary->owner_tosql = inter_mercenary_owner_tosql;
	inter_mercenary->owner_delete = inter_mercenary_owner_delete;

	inter_mercenary->sql_init = inter_mercenary_sql_init;
	inter_mercenary->sql_final = inter_mercenary_sql_final;
	inter_mercenary->parse_frommap = inter_mercenary_parse_frommap;
}
