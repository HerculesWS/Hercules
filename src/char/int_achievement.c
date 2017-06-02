/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2017  Hercules Dev Team
* Copyright (C) Smokexyz
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

#include "int_achievement.h"

#include "char/char.h"
#include "char/inter.h"
#include "char/mapif.h"

#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"

#include <stdio.h>
#include <stdlib.h>

struct inter_achievement_interface inter_achievement_s;
struct inter_achievement_interface *inter_achievement;

/**
 * Saves changed achievements for a character.
 * @param  char_id     character identifier.
 * @param  cp          pointer to loaded achievements.
 * @param  p           pointer to map-sent character achievements.
 * @return number of achievements saved.
 */
int inter_achievement_tosql(int char_id, struct char_achievements *cp, struct char_achievements *p)
{
	StringBuf buf;
	int i = 0, rows = 0;

	nullpo_ret(cp);
	nullpo_ret(p);
	Assert_ret(char_id > 0);

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "REPLACE INTO `%s` (`char_id`, `ach_id`, `completed_at`, `rewarded_at`", char_achievement_db);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; i++)
		StrBuf->Printf(&buf, ", `obj_%d`", i);
	StrBuf->AppendStr(&buf, ") VALUES ");

	for (i = 0; i < VECTOR_LENGTH(*p); i++) {
		int j = 0;
		bool save = false;
		struct achievement *pa = &VECTOR_INDEX(*p, i), *cpa = NULL;

		ARR_FIND(0, VECTOR_LENGTH(*cp), j, ((cpa = &VECTOR_INDEX(*cp, j)) && cpa->id == pa->id));

		if (j == VECTOR_LENGTH(*cp))
			save = true;
		else if (memcmp(cpa, pa, sizeof(struct achievement)) != 0)
			save = true;

		if (save) {
			StrBuf->Printf(&buf, "%s('%d', '%d', '%"PRId64"', '%"PRId64"'", rows ?", ":"", char_id, pa->id, (int64)pa->completed_at, (int64)pa->rewarded_at);
			for (j = 0; j < MAX_ACHIEVEMENT_OBJECTIVES; j++)
				StrBuf->Printf(&buf, ", '%d'", pa->objective[j]);
			StrBuf->AppendStr(&buf, ")");
			rows++;
		}
	}

	if (rows > 0 && SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf); // Destroy the buffer.
		VECTOR_CLEAR(*p); /* Free buffer data in the case of an error. */
		return 0;
	}
	// Destroy the buffer.
	StrBuf->Destroy(&buf);

	if (rows) {
		ShowInfo("achievements saved for char %d (total: %d, saved: %d)\n", char_id, VECTOR_LENGTH(*p), rows);

		/* Sync with inter-db acheivements. */
		VECTOR_CLEAR(*cp);
		VECTOR_ENSURE(*cp, VECTOR_LENGTH(*p), 1);
		VECTOR_PUSHARRAY(*cp, VECTOR_DATA(*p), VECTOR_LENGTH(*p));
	}

	return rows;
}

/**
 * Retrieves all achievements of a character.
 * @param char_id  character identifier.
 * @param ca       pointer to character achievements structure.
 * @return true on success, false on failure.
 */
bool inter_achievement_fromsql(int char_id, struct char_achievements *cp)
{
	StringBuf buf;
	char *data;
	int i = 0, num_rows = 0;

	Assert_ret(char_id > 0);
	nullpo_ret(cp);

	// char_achievements (`char_id`, `ach_id`, `completed_at`, `rewarded_at`, `obj_0`, `obj_2`, ...`obj_9`)
	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `ach_id`, `completed_at`, `rewarded_at`");
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; i++)
		StrBuf->Printf(&buf, ", `obj_%d`", i);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `char_id` = '%d' ORDER BY `ach_id`", char_achievement_db, char_id);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	}

	if ((num_rows = (int) SQL->NumRows(inter->sql_handle)) != 0) {
		int j = 0;

		VECTOR_CLEAR(*cp);
		VECTOR_ENSURE(*cp, num_rows, 1);

		for (i = 0; i < num_rows && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); i++) {
			struct achievement t_ach = { 0 };
			SQL->GetData(inter->sql_handle, 0, &data, NULL); t_ach.id = atoi(data);
			SQL->GetData(inter->sql_handle, 1, &data, NULL); t_ach.completed_at = atoi(data);
			SQL->GetData(inter->sql_handle, 2, &data, NULL); t_ach.rewarded_at = atoi(data);
			/* Objectives */
			for (j = 0; j < MAX_ACHIEVEMENT_OBJECTIVES; j++) {
				SQL->GetData(inter->sql_handle, j + 3, &data, NULL);
				t_ach.objective[j] = atoi(data);
			}
			/* Add Entry */
			VECTOR_PUSH(*cp, t_ach);
		}
	}

	SQL->FreeResult(inter->sql_handle);

	StrBuf->Destroy(&buf);

	if (num_rows > 0)
		ShowInfo("achievements loaded for char %d (total: %d)\n", char_id, num_rows);

	return true;
}

/**
 * Parse achievement load request from the map server
 * @param fd  socket descriptor.
 */
void mapif_parse_load_achievements(int fd)
{
	int char_id = 0;

	/* Read received information from map-server. */
	RFIFOHEAD(fd);
	char_id = RFIFOL(fd, 2);

	/* Load and send achievements to map */
	mapif->achievement_load(fd, char_id);
}

/**
 * Loads achievements and sends to the map server.
 * @param fd       socket descriptor
 * @param char_id  character Id.
 */
void mapif_achievement_load(int fd, int char_id)
{
	struct char_achievements *cp = NULL;

	/* Ensure data exists */
	cp = idb_ensure(inter_achievement->char_achievements, char_id, inter_achievement->ensure_char_achievements);

	/* Load storage for char-server. */
	inter_achievement->fromsql(char_id, cp);

	/* Send Achievements to map server. */
	mapif->sAchievementsToMap(fd, char_id, cp);
}

/**
 * Sends achievement data of a character to the map server.
 * @packet 0x3898  [out] <packet_id>.W <payload_size>.W <char_id>.L <char_achievements[]>.P
 * @param  fd      [in]  socket descriptor.
 * @param  char_id [in]  Character ID.
 * @param  cp      [in]  Pointer to character's achievement data vector.
 */
void mapif_send_achievements_to_map(int fd, int char_id, struct char_achievements *cp)
{
	int i = 0;
	int data_size = sizeof(struct achievement) * VECTOR_LENGTH(*cp);

	/* Send to the map server. */
	WFIFOHEAD(fd, (8 + data_size));
	WFIFOW(fd, 0) = 0x3898;
	WFIFOW(fd, 2) = (8 + data_size);
	WFIFOL(fd, 4) = char_id;
	for (i = 0; i < VECTOR_LENGTH(*cp); i++)
		memcpy(WFIFOP(fd, 8 + i * sizeof(struct achievement)), &VECTOR_INDEX(*cp, i), sizeof(struct achievement));
	WFIFOSET(fd, 8 + data_size);
}

/**
 * Handles achievement request and saves data from map server.
 * @packet [in] 0x3099 <packet_size>.W <char_id>.L <char_achievement>.P
 * @param fd  session socket descriptor.
 */
void mapif_parse_save_achievements(int fd)
{
	int size = 0, char_id = 0, payload_count = 0, i = 0;
	struct char_achievements p = { 0 };

	RFIFOHEAD(fd);
	size = RFIFOW(fd, 2);
	char_id = RFIFOL(fd, 4);

	payload_count = (size - 8) / sizeof(struct achievement);

	VECTOR_INIT(p);
	VECTOR_ENSURE(p, payload_count, 1);

	for (i = 0; i < payload_count; i++) {
		struct achievement ach = { 0 };
		memcpy(&ach, RFIFOP(fd, 8 + i * sizeof(struct achievement)), sizeof(struct achievement));
		VECTOR_PUSH(p, ach);
	}

	mapif->achievement_save(char_id, &p);

	VECTOR_CLEAR(p);
}

/**
 * Handles inter-server achievement db ensuring
 * and saves current achievements to sql.
 * @param char_id      character identifier.
 * @param p            pointer to character achievements vector.
 */
void mapif_achievement_save(int char_id, struct char_achievements *p)
{
	struct char_achievements *cp = NULL;

	nullpo_retv(p);
	
	/* Get loaded achievements. */
	cp = idb_ensure(inter_achievement->char_achievements, char_id, inter_achievement->ensure_char_achievements);

	if (VECTOR_LENGTH(*p)) /* Save current achievements. */
		inter_achievement->tosql(char_id, cp, p);
}

/**
 * Handles checking of map server packets and calls appropriate functions.
 * @param fd   socket descriptor.
 * @return 0 on failure, 1 on succes.
 */
int inter_achievement_parse_frommap(int fd)
{
	RFIFOHEAD(fd);

	switch (RFIFOW(fd,0)) {
		case 0x3098:
			mapif->pLoadAchievements(fd);
			break;
		case 0x3099:
			mapif->pSaveAchievements(fd);
			break;
		default:
			return 0;
	}

	return 1;
}

/**
 * Initialization function
 */
int inter_achievement_sql_init(void)
{
	// Initialize the loaded db storage.
	// used as a comparand against map-server achievement data before saving.
	inter_achievement->char_achievements = idb_alloc(DB_OPT_RELEASE_DATA);
	return 1;
}

/**
 * This function ensures idb's entry.
 */
static struct DBData inter_achievement_ensure_char_achievements(union DBKey key, va_list args)
{
	struct char_achievements *ca = NULL;

	CREATE(ca, struct char_achievements, 1);
	VECTOR_INIT(*ca);

	return DB->ptr2data(ca);
}

/**
 * Cleaning function called through db_destroy()
 */
int inter_achievement_char_achievements_clear(union DBKey key, struct DBData *data, va_list args)
{
	struct char_achievements *ca = DB->data2ptr(data);

	VECTOR_CLEAR(*ca);

	return 0;
}

/**
 * Finalization function.
 */
void inter_achievement_sql_final(void)
{
	inter_achievement->char_achievements->destroy(inter_achievement->char_achievements, inter_achievement->char_achievements_clear);
}

/**
 * Inter-achievement interface.
 */
void inter_achievement_defaults(void)
{
	inter_achievement = &inter_achievement_s;
	/* */
	inter_achievement->ensure_char_achievements = inter_achievement_ensure_char_achievements;
	/* */
	inter_achievement->sql_init = inter_achievement_sql_init;
	inter_achievement->sql_final = inter_achievement_sql_final;
	/* */
	inter_achievement->tosql = inter_achievement_tosql;
	inter_achievement->fromsql = inter_achievement_fromsql;
	/* */
	inter_achievement->parse_frommap = inter_achievement_parse_frommap;
	inter_achievement->char_achievements_clear = inter_achievement_char_achievements_clear;
}
