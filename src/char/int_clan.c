/**
 * This file is part of Hercules.
 * https://herc.ws - https://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017-2020 Hercules Dev Team
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#define HERCULES_CORE

#include "config/core.h" // DBPATH
#include "int_clan.h"

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
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>

static struct inter_clan_interface inter_clan_s;
struct inter_clan_interface *inter_clan;

/**
 * Kick offline members of a clan
 *
 * Perform the update on the DB to reset clan id to 0
 * of the members that are inactive for too long
 *
 * @param clan_id Id of the clan
 * @param kick_interval Time needed to consider a player inactive and kick it
 * @return 0 on failure, 1 on success
 */
static int inter_clan_kick_inactive_members(int clan_id, int kick_interval)
{
	if (clan_id <= 0) {
		ShowError("inter_clan_kick_inactive_members: Invalid clan id received '%d'\n", clan_id);
		Assert_report(clan_id > 0);
		return 0;
	} else if (kick_interval <= 0) {
		ShowError("inter_clan_kick_inactive_members: Invalid kick_interval received '%d'", kick_interval);
		Assert_report(kick_interval > 0);
		return 0;
	}

	// Kick Inactive members
	if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET "
		"`clan_id` = 0 WHERE `clan_id` = '%d' AND `online` = 0 AND `last_login` < %"PRId64,
		char_db, clan_id, (int64)(time(NULL) - kick_interval)))
	{
		Sql_ShowDebug(inter->sql_handle);
		return 0;
	}

	return 1;
}

/**
 * Count members of a clan
 *
 * @param clan_id Id of the clan
 * @param kick_interval Time needed to consider a player inactive and ignore it on the count
 */
static int inter_clan_count_members(int clan_id, int kick_interval)
{
	struct SqlStmt *stmt;
	int count = 0;

	if (clan_id <= 0) {
		ShowError("inter_clan_count_members: Invalid clan id received '%d'\n", clan_id);
		Assert_report(clan_id > 0);
		return 0;
	} else if (kick_interval <= 0) {
		ShowError("inter_clan_count_member: Invalid kick_interval received '%d'", kick_interval);
		Assert_report(kick_interval > 0);
		return 0;
	}

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	// Count members
	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT COUNT(*) FROM `%s` WHERE `clan_id` = ? AND `last_login` >= %"PRId64, char_db, (int64)(time(NULL) - kick_interval))
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &clan_id, sizeof(clan_id))
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT, &count, sizeof(count), NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return 0;
	}

	if (SQL->StmtNumRows(stmt) > 0 && SQL_SUCCESS != SQL->StmtNextRow(stmt)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return 0;
	}

	SQL->StmtFree(stmt);
	return count;
}

// Communication from the map server
// - Can analyzed only one by one packet
// Data packet length that you set to inter.c
//- Shouldn't do checking and packet length, RFIFOSKIP is done by the caller
// Must Return
//  1 : ok
//  0 : error
static int inter_clan_parse_frommap(int fd)
{
	RFIFOHEAD(fd);

	switch(RFIFOW(fd, 0)) {
	case 0x3044: mapif->parse_ClanMemberCount(fd, RFIFOL(fd, 2), RFIFOL(fd, 6)); break;
	case 0x3045: mapif->parse_ClanMemberKick(fd, RFIFOL(fd, 2), RFIFOL(fd, 6)); break;

	default:
		return 0;
	}

	return 1;
}

void inter_clan_defaults(void)
{
	inter_clan = &inter_clan_s;

	inter_clan->kick_inactive_members = inter_clan_kick_inactive_members;
	inter_clan->count_members = inter_clan_count_members;
	inter_clan->parse_frommap = inter_clan_parse_frommap;
}
