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

#include "int_quest.h"

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

#include <stdio.h>
#include <stdlib.h>

struct inter_quest_interface inter_quest_s;
struct inter_quest_interface *inter_quest;

/**
 * Loads the entire questlog for a character.
 *
 * @param char_id Character ID
 * @param count   Pointer to return the number of found entries.
 * @return Array of found entries. It has *count entries, and it is care of the
 *         caller to aFree() it afterwards.
 */
struct quest *mapif_quests_fromsql(int char_id, int *count)
{
	struct quest *questlog = NULL;
	struct quest tmp_quest;
	struct SqlStmt *stmt;
	StringBuf buf;
	int i;
	int sqlerror = SQL_SUCCESS;

	if (!count)
		return NULL;

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		*count = 0;
		return NULL;
	}

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `quest_id`, `state`, `time`");
	for (i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
		StrBuf->Printf(&buf, ", `count%d`", i+1);
	}
	StrBuf->Printf(&buf, " FROM `%s` WHERE `char_id`=?", quest_db);

	memset(&tmp_quest, 0, sizeof(struct quest));

	if (SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT,  &tmp_quest.quest_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_INT,  &tmp_quest.state,    0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_UINT, &tmp_quest.time,     0, NULL, NULL)
	) {
		sqlerror = SQL_ERROR;
	}

	StrBuf->Destroy(&buf);

	for (i = 0; sqlerror != SQL_ERROR && i < MAX_QUEST_OBJECTIVES; i++) { // Stop on the first error
		sqlerror = SQL->StmtBindColumn(stmt, 3+i, SQLDT_INT,  &tmp_quest.count[i], 0, NULL, NULL);
	}

	if (sqlerror == SQL_ERROR) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		*count = 0;
		return NULL;
	}

	*count = (int)SQL->StmtNumRows(stmt);
	if (*count > 0) {
		i = 0;
		questlog = (struct quest *)aCalloc(*count, sizeof(struct quest));

		while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
			if (i >= *count) // Sanity check, should never happen
				break;
			memcpy(&questlog[i++], &tmp_quest, sizeof(tmp_quest));
		}
		if (i < *count) {
			// Should never happen. Compact array
			*count = i;
			questlog = aRealloc(questlog, sizeof(struct quest)*i);
		}
	}

	SQL->StmtFree(stmt);
	return questlog;
}

/**
 * Deletes a quest from a character's questlog.
 *
 * @param char_id  Character ID
 * @param quest_id Quest ID
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_delete(int char_id, int quest_id)
{
	if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_db, quest_id, char_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	return true;
}

/**
 * Adds a quest to a character's questlog.
 *
 * @param char_id Character ID
 * @param qd      Quest data
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_add(int char_id, struct quest qd)
{
	StringBuf buf;
	int i;

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`", quest_db);
	for (i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
		StrBuf->Printf(&buf, ", `count%d`", i+1);
	}
	StrBuf->Printf(&buf, ") VALUES ('%d', '%d', '%u', '%u'", qd.quest_id, char_id, qd.state, qd.time);
	for (i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
		StrBuf->Printf(&buf, ", '%d'", qd.count[i]);
	}
	StrBuf->AppendStr(&buf, ")");
	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	}
	StrBuf->Destroy(&buf);

	return true;
}

/**
 * Updates a quest in a character's questlog.
 *
 * @param char_id Character ID
 * @param qd      Quest data
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_update(int char_id, struct quest qd)
{
	StringBuf buf;
	int i;

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "UPDATE `%s` SET `state`='%u'", quest_db, qd.state);
	for (i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
		StrBuf->Printf(&buf, ", `count%d`='%d'", i+1, qd.count[i]);
	}
	StrBuf->Printf(&buf, " WHERE `quest_id` = '%d' AND `char_id` = '%d'", qd.quest_id, char_id);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	}
	StrBuf->Destroy(&buf);

	return true;
}

void mapif_quest_save_ack(int fd, int char_id, bool success)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3861;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = success?1:0;
	WFIFOSET(fd,7);
}

/**
 * Handles the save request from mapserver for a character's questlog.
 *
 * Received quests are saved, and an ack is sent back to the map server.
 *
 * @see inter_parse_frommap
 */
int mapif_parse_quest_save(int fd)
{
	int i, j, k, old_n, new_n = (RFIFOW(fd,2)-8)/sizeof(struct quest);
	int char_id = RFIFOL(fd,4);
	struct quest *old_qd = NULL;
	const struct quest *new_qd = NULL;
	bool success = true;

	if (new_n > 0)
		new_qd = RFIFOP(fd,8);

	old_qd = mapif->quests_fromsql(char_id, &old_n);

	for (i = 0; i < new_n; i++) {
		ARR_FIND( 0, old_n, j, new_qd[i].quest_id == old_qd[j].quest_id );
		if (j < old_n) {
			// Update existing quests

			// Only states and counts are changeable.
			ARR_FIND( 0, MAX_QUEST_OBJECTIVES, k, new_qd[i].count[k] != old_qd[j].count[k] );
			if (k != MAX_QUEST_OBJECTIVES || new_qd[i].state != old_qd[j].state)
				success &= mapif->quest_update(char_id, new_qd[i]);

			if (j < (--old_n)) {
				// Compact array
				memmove(&old_qd[j],&old_qd[j+1],sizeof(struct quest)*(old_n-j));
				memset(&old_qd[old_n], 0, sizeof(struct quest));
			}
		} else {
			// Add new quests
			success &= mapif->quest_add(char_id, new_qd[i]);
		}
	}

	for (i = 0; i < old_n; i++) // Quests not in new_qd but in old_qd are to be erased.
		success &= mapif->quest_delete(char_id, old_qd[i].quest_id);

	if (old_qd)
		aFree(old_qd);

	// Send ack
	mapif->quest_save_ack(fd, char_id, success);

	return 0;
}

void mapif_send_quests(int fd, int char_id, struct quest *tmp_questlog, int num_quests)
{
	WFIFOHEAD(fd,num_quests*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = num_quests*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;

	if (num_quests > 0) {
		nullpo_retv(tmp_questlog);
		memcpy(WFIFOP(fd,8), tmp_questlog, sizeof(struct quest)*num_quests);
	}

	WFIFOSET(fd,num_quests*sizeof(struct quest)+8);
}

/**
 * Sends questlog to the map server
 *
 * Note: Completed quests (state == Q_COMPLETE) are guaranteed to be sent last
 * and the map server relies on this behavior (once the first Q_COMPLETE quest,
 * all of them are considered to be Q_COMPLETE)
 *
 * @see inter_parse_frommap
 */
int mapif_parse_quest_load(int fd)
{
	int char_id = RFIFOL(fd,2);
	struct quest *tmp_questlog = NULL;
	int num_quests;

	tmp_questlog = mapif->quests_fromsql(char_id, &num_quests);
	mapif->send_quests(fd, char_id, tmp_questlog, num_quests);

	if (tmp_questlog)
		aFree(tmp_questlog);

	return 0;
}

/**
 * Parses questlog related packets from the map server.
 *
 * @see inter_parse_frommap
 */
int inter_quest_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0)) {
		case 0x3060: mapif->parse_quest_load(fd); break;
		case 0x3061: mapif->parse_quest_save(fd); break;
		default:
			return 0;
	}
	return 1;
}

void inter_quest_defaults(void)
{
	inter_quest = &inter_quest_s;

	inter_quest->parse_frommap = inter_quest_parse_frommap;
}
