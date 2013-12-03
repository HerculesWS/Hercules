// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"
#include "../common/timer.h"

#include "char.h"
#include "inter.h"
#include "int_quest.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Loads the entire questlog for a character.
 *
 * @param char_id Character ID
 * @param count   Pointer to return the number of found entries.
 * @return Array of found entries. It has *count entries, and it is care of the
 *         caller to aFree() it afterwards.
 */
struct quest *mapif_quests_fromsql(int char_id, int *count) {
	struct quest *questlog = NULL;
	struct quest tmp_quest;
	SqlStmt *stmt;

	if (!count)
		return NULL;

	stmt = SQL->StmtMalloc(sql_handle);
	if (stmt == NULL) {
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	memset(&tmp_quest, 0, sizeof(struct quest));

	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `quest_id`, `state`, `time`, `count1`, `count2`, `count3` FROM `%s` WHERE `char_id`=?", quest_db)
	 || SQL_ERROR == SQL->StmtBindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	 || SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT,  &tmp_quest.quest_id, 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_INT,  &tmp_quest.state,    0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_UINT, &tmp_quest.time,     0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3, SQLDT_INT,  &tmp_quest.count[0], 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 4, SQLDT_INT,  &tmp_quest.count[1], 0, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 5, SQLDT_INT,  &tmp_quest.count[2], 0, NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		*count = 0;
		return NULL;
	}

	*count = (int)SQL->NumRows(sql_handle);
	if (*count > 0) {
		int i = 0;
		questlog = (struct quest *)aCalloc(*count, sizeof(struct quest));

		while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
			if (i >= *count) // Sanity check, should never happen
				break;
			memcpy(&questlog[i++], &tmp_quest, sizeof(tmp_quest));
		}
		if (i < *count) {
			// Should never. Compact array
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
bool mapif_quest_delete(int char_id, int quest_id) {
	if (SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_db, quest_id, char_id)) {
		Sql_ShowDebug(sql_handle);
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
bool mapif_quest_add(int char_id, struct quest qd) {
	if (SQL_ERROR == SQL->Query(sql_handle, "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`, `count1`, `count2`, `count3`) "
	                                        "VALUES ('%d', '%d', '%d','%d', '%d', '%d', '%d')",
	                            quest_db, qd.quest_id, char_id, qd.state, qd.time, qd.count[0], qd.count[1], qd.count[2])
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Updates a quest in a character's questlog.
 *
 * @param char_id Character ID
 * @param qd      Quest data
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_update(int char_id, struct quest qd) {
	if (SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `state`='%d', `count1`='%d', `count2`='%d', `count3`='%d' "
	                                        "WHERE `quest_id` = '%d' AND `char_id` = '%d'",
	                            quest_db, qd.state, qd.count[0], qd.count[1], qd.count[2], qd.quest_id, char_id)
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Handles the save request from mapserver for a character's questlog.
 *
 * Received quests are saved, and an ack is sent back to the map server.
 *
 * @see inter_parse_frommap
 */
int mapif_parse_quest_save(int fd) {
	int i, j, k, old_n, new_n = (RFIFOW(fd,2)-8)/sizeof(struct quest);
	int char_id = RFIFOL(fd,4);
	struct quest *old_qd = NULL, *new_qd = NULL;
	bool success = true;

	if (new_n)
		new_qd = (struct quest*)RFIFOP(fd,8);

	old_qd = mapif_quests_fromsql(char_id, &old_n);

	for (i = 0; i < new_n; i++) {
		ARR_FIND( 0, old_n, j, new_qd[i].quest_id == old_qd[j].quest_id );
		if (j < old_n) {
			// Update existing quests

			// Only states and counts are changable.
			ARR_FIND( 0, MAX_QUEST_OBJECTIVES, k, new_qd[i].count[k] != old_qd[j].count[k] );
			if (k != MAX_QUEST_OBJECTIVES || new_qd[i].state != old_qd[j].state)
				success &= mapif_quest_update(char_id, new_qd[i]);

			if (j < (--old_n)) {
				// Compact array
				memmove(&old_qd[j],&old_qd[j+1],sizeof(struct quest)*(old_n-j));
				memset(&old_qd[old_n], 0, sizeof(struct quest));
			}
		} else {
			// Add new quests
			success &= mapif_quest_add(char_id, new_qd[i]);
		}
	}

	for (i = 0; i < old_n; i++) // Quests not in new_qd but in old_qd are to be erased.
		success &= mapif_quest_delete(char_id, old_qd[i].quest_id);

	if (old_qd)
		aFree(old_qd);

	// Send ack
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3861;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = success?1:0;
	WFIFOSET(fd,7);

	return 0;
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
int mapif_parse_quest_load(int fd) {
	int char_id = RFIFOL(fd,2);
	struct quest *tmp_questlog = NULL;
	int num_quests;

	tmp_questlog = mapif_quests_fromsql(char_id, &num_quests);

	WFIFOHEAD(fd,num_quests*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = num_quests*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;

	if (num_quests > 0)
		memcpy(WFIFOP(fd,8), tmp_questlog, sizeof(struct quest)*num_quests);

	WFIFOSET(fd,num_quests*sizeof(struct quest)+8);

	return 0;
}

/**
 * Parses questlog related packets from the map server.
 *
 * @see inter_parse_frommap
 */
int inter_quest_parse_frommap(int fd) {
	switch(RFIFOW(fd,0)) {
		case 0x3060: mapif_parse_quest_load(fd); break;
		case 0x3061: mapif_parse_quest_save(fd); break;
		default:
			return 0;
	}
	return 1;
}
