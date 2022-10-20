/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017-2022 Hercules Dev Team
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

#include "int_rodex.h"

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
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>

static struct inter_rodex_interface inter_rodex_s;
struct inter_rodex_interface *inter_rodex;

// Loads new mails of this char_id/account_id
static int inter_rodex_fromsql(int char_id, int account_id, int8 opentype, int64 mail_id, struct rodex_maillist *mails)
{
	int count = 0;
	struct rodex_message msg = { 0 };
	struct SqlStmt *stmt;

	nullpo_retr(-1, mails);

	stmt = SQL->StmtMalloc(inter->sql_handle);

	switch (opentype) {
	case RODEX_OPENTYPE_MAIL:
		if (SQL_ERROR == SQL->StmtPrepare(stmt,
			"SELECT `mail_id`, `sender_name`, `sender_id`, `receiver_name`, `receiver_id`, `receiver_accountid`,"
			"`title`, `body`, `zeny`, `type`, `is_read`, `sender_read`, `send_date`, `expire_date`, `weight`"
			"FROM `%s` WHERE `expire_date` > '%d' AND `receiver_id` = '%d' AND `mail_id` > '%"PRId64"'"
			"ORDER BY `mail_id` ASC", rodex_db, (int)time(NULL), char_id, mail_id)
			) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return -1;
		}
		break;

	case RODEX_OPENTYPE_ACCOUNT:
		if (SQL_ERROR == SQL->StmtPrepare(stmt,
			"SELECT `mail_id`, `sender_name`, `sender_id`, `receiver_name`, `receiver_id`, `receiver_accountid`,"
			"`title`, `body`, `zeny`, `type`, `is_read`, `sender_read`, `send_date`, `expire_date`, `weight`"
			"FROM `%s` WHERE "
			"`expire_date` > '%d' AND `receiver_accountid` = '%d' AND `mail_id` > '%"PRId64"'"
			"ORDER BY `mail_id` ASC", rodex_db, (int)time(NULL), account_id, mail_id)
			) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return -1;
		}
		break;

	case RODEX_OPENTYPE_RETURN:
		if (SQL_ERROR == SQL->StmtPrepare(stmt,
			"SELECT `mail_id`, `sender_name`, `sender_id`, `receiver_name`, `receiver_id`, `receiver_accountid`,"
			"`title`, `body`, `zeny`, `type`, `is_read`, `sender_read`, `send_date`, `expire_date`, `weight`"
			"FROM `%s` WHERE (`is_read` = 0 AND `sender_id` = '%d' AND `expire_date` <= '%d' AND `expire_date` + '%d' > '%d' AND `mail_id` > '%"PRId64"')"
			"ORDER BY `mail_id` ASC", rodex_db, char_id, (int)time(NULL), RODEX_EXPIRE, (int)time(NULL), mail_id)
			) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return -1;
		}
		break;

	case RODEX_OPENTYPE_UNSET:
		if (SQL_ERROR == SQL->StmtPrepare(stmt,
			"SELECT `mail_id`, `sender_name`, `sender_id`, `receiver_name`, `receiver_id`, `receiver_accountid`,"
			"`title`, `body`, `zeny`, `type`, `is_read`, `sender_read`, `send_date`, `expire_date`, `weight`"
			"FROM `%s` WHERE "
			"((`expire_date` > '%d' AND (`receiver_id` = '%d' OR `receiver_accountid` = '%d'))"
			"OR (`is_read` = 0 AND `sender_id` = '%d' AND `expire_date` <= '%d' AND `expire_date` + '%d' > '%d'))"
			"ORDER BY `mail_id` ASC", rodex_db, (int)time(NULL), char_id, account_id, char_id, (int)time(NULL), RODEX_EXPIRE, (int)time(NULL))
			) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return -1;
		}
		break;
	}

	if (SQL_ERROR == SQL->StmtExecute(stmt)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 0,  SQLDT_INT64,  &msg.id,                 sizeof msg.id,                 NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 1,  SQLDT_STRING, &msg.sender_name,        sizeof msg.sender_name,        NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 2,  SQLDT_INT,    &msg.sender_id,          sizeof msg.sender_id,          NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 3,  SQLDT_STRING, &msg.receiver_name,      sizeof msg.receiver_name,      NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 4,  SQLDT_INT,    &msg.receiver_id,        sizeof msg.receiver_id,        NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 5,  SQLDT_INT,    &msg.receiver_accountid, sizeof msg.receiver_accountid, NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 6,  SQLDT_STRING, &msg.title,              sizeof msg.title,              NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 7,  SQLDT_STRING, &msg.body,               sizeof msg.body,               NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 8,  SQLDT_INT64,  &msg.zeny,               sizeof msg.zeny,               NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 9,  SQLDT_UINT8,  &msg.type,               sizeof msg.type,               NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 10, SQLDT_BOOL,   &msg.is_read,            sizeof msg.is_read,            NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 11, SQLDT_BOOL,   &msg.sender_read,        sizeof msg.sender_read,        NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 12, SQLDT_INT,    &msg.send_date,          sizeof msg.send_date,          NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 13, SQLDT_INT,    &msg.expire_date,        sizeof msg.expire_date,        NULL, NULL)
	 || SQL_ERROR == SQL->StmtBindColumn(stmt, 14, SQLDT_INT,    &msg.weight,             sizeof msg.weight,             NULL, NULL)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return -1;
	}

	{
		struct item it = { 0 };
		StringBuf buf;
		struct SqlStmt *stmt_items = SQL->StmtMalloc(inter->sql_handle);
		int i;

		if (stmt_items == NULL) {
			SQL->StmtFreeResult(stmt);
			SQL->StmtFree(stmt);
			return -1;
		}

		StrBuf->Init(&buf);

		StrBuf->AppendStr(&buf, "SELECT `nameid`, `amount`, `equip`, `identify`, `refine`, `grade`, `attribute`, `expire_time`, `bound`, `unique_id`");
		for (i = 0; i < MAX_SLOTS; i++) {
			StrBuf->Printf(&buf, ", `card%d`", i);
		}
		for (i = 0; i < MAX_ITEM_OPTIONS; i++) {
			StrBuf->Printf(&buf, ", `opt_idx%d`, `opt_val%d`", i, i);
		}
		StrBuf->Printf(&buf, "FROM `%s` WHERE mail_id = ? ORDER BY `mail_id` ASC", rodex_item_db);

		if (SQL_ERROR == SQL->StmtPrepareStr(stmt_items, StrBuf->Value(&buf))
		 || SQL_ERROR == SQL->StmtBindParam(stmt_items, 0, SQLDT_INT64, &msg.id, sizeof msg.id)
		) {
			SqlStmt_ShowDebug(stmt_items);
		}

		// Read mails
		while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {

			if (msg.type & MAIL_TYPE_ITEM) {
				if (SQL_ERROR == SQL->StmtExecute(stmt_items)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 0,  SQLDT_INT,    &it.nameid,          sizeof it.nameid,      NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 1,  SQLDT_SHORT,  &it.amount,          sizeof it.amount,      NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 2,  SQLDT_UINT,   &it.equip,           sizeof it.equip,       NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 3,  SQLDT_CHAR,   &it.identify,        sizeof it.identify,    NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 4,  SQLDT_CHAR,   &it.refine,          sizeof it.refine,      NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 5,  SQLDT_CHAR,   &it.grade,           sizeof it.grade,       NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 6,  SQLDT_CHAR,   &it.attribute,       sizeof it.attribute,   NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 7,  SQLDT_UINT,   &it.expire_time,     sizeof it.expire_time, NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 8,  SQLDT_UCHAR,  &it.bound,           sizeof it.bound,       NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 9,  SQLDT_UINT64, &it.unique_id,       sizeof it.unique_id,   NULL, NULL)
				) {
					SqlStmt_ShowDebug(stmt_items);
				}
				for (i = 0; i < MAX_SLOTS; i++) {
					if (SQL_ERROR == SQL->StmtBindColumn(stmt_items, 10 + i, SQLDT_INT, &it.card[i], sizeof it.card[i], NULL, NULL))
						SqlStmt_ShowDebug(stmt_items);
				}
				for (i = 0; i < MAX_ITEM_OPTIONS; i++) {
					if (SQL_ERROR == SQL->StmtBindColumn(stmt_items, 10 + MAX_SLOTS + i * 2, SQLDT_INT16, &it.option[i].index, sizeof it.option[i].index, NULL, NULL)
					 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 11 + MAX_SLOTS + i * 2, SQLDT_INT16, &it.option[i].value, sizeof it.option[i].value, NULL, NULL)
					) {
						SqlStmt_ShowDebug(stmt_items);
					}
				}

				for (i = 0; i < RODEX_MAX_ITEM && SQL_SUCCESS == SQL->StmtNextRow(stmt_items); ++i) {
					msg.items[i].item = it;
					msg.items_count++;
				}
			}

			if (msg.items_count == 0) {
				msg.type &= ~MAIL_TYPE_ITEM;
			}

			if (msg.zeny == 0) {
				msg.type &= ~MAIL_TYPE_ZENY;
			}

#if PACKETVER >= 20170419
			if (opentype == RODEX_OPENTYPE_UNSET) {
				if (msg.receiver_id == 0)
					msg.opentype = RODEX_OPENTYPE_ACCOUNT;
				else if (msg.expire_date < time(NULL))
					msg.opentype = RODEX_OPENTYPE_RETURN;
				else
					msg.opentype = RODEX_OPENTYPE_MAIL;
			} else {
				msg.opentype = opentype;
			}
#else
			msg.opentype = opentype;
#endif
#if PACKETVER < 20160601
			// NPC Message Type isn't supported in old clients
			msg.type &= ~MAIL_TYPE_NPC;
#endif

			++count;
			VECTOR_ENSURE(*mails, 1, 1);
			VECTOR_PUSH(*mails, msg);
			memset(&msg, 0, sizeof(struct rodex_message));

			SQL->StmtFreeResult(stmt_items);
		}
		StrBuf->Destroy(&buf);
		SQL->StmtFree(stmt_items);
	}

	SQL->StmtFreeResult(stmt);
	SQL->StmtFree(stmt);

	ShowInfo("rodex load complete from DB - id: %d (total: %d)\n", char_id, count);
	return count;
}

// Checks if user has non-read mails
static bool inter_rodex_hasnew(int char_id, int account_id)
{
	int count = 0;
	char *data;

	if (SQL_ERROR == SQL->Query(inter->sql_handle,
		"SELECT count(*) FROM `%s` WHERE ("
		"(`expire_date` > '%d' AND (`receiver_id` = '%d' OR `receiver_accountid` = '%d')) OR"
		"(`sender_id` = '%d' AND `expire_date` <= '%d' AND `send_date` + '%d' > '%d' AND `is_read` = 0)" // is_read is required in this line because of the OR in next condition
		") AND ((`is_read` = 0 AND `sender_read` = 0) OR (`type` > 0 AND `type` != 8))",
		rodex_db, (int)time(NULL), char_id, account_id,
		char_id, (int)time(NULL), 2 * RODEX_EXPIRE, (int)time(NULL))
		) {
		Sql_ShowDebug(inter->sql_handle);
		return -1;
	}

	if (SQL_SUCCESS != SQL->NextRow(inter->sql_handle))
		return false;

	SQL->GetData(inter->sql_handle, 0, &data, NULL);
	count = atoi(data);
	SQL->FreeResult(inter->sql_handle);

	return count > 0;
}

/// Checks player name and retrieves some data
static bool inter_rodex_checkname(const char *name, int *target_char_id, int *target_class, int *target_level)
{
	char esc_name[NAME_LENGTH * 2 + 1];
	bool found = false;

	nullpo_retr(false, name);
	nullpo_retr(false, target_char_id);
	nullpo_retr(false, target_class);
	nullpo_retr(false, target_level);

	// Try to find the Dest Char by Name
	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if (SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `char_id`, `class`, `base_level` FROM `%s` WHERE `name` = '%s'", char_db, esc_name)) {
		Sql_ShowDebug(inter->sql_handle);
	} else {
		if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
			char *data;
			SQL->GetData(inter->sql_handle, 0, &data, NULL); *target_char_id = atoi(data);
			SQL->GetData(inter->sql_handle, 1, &data, NULL); *target_class = atoi(data);
			SQL->GetData(inter->sql_handle, 2, &data, NULL); *target_level = atoi(data);
			found = true;
		}
	}
	SQL->FreeResult(inter->sql_handle);

	return found;
}

/// Stores a single message in the database.
/// Returns the message's ID if successful (or 0 if it fails).
static int64 inter_rodex_savemessage(struct rodex_message *msg)
{
	char sender_name[NAME_LENGTH * 2 + 1];
	char receiver_name[NAME_LENGTH * 2 + 1];
	char body[RODEX_BODY_LENGTH * 2 + 1];
	char title[RODEX_TITLE_LENGTH * 2 + 1];
	int i;

	nullpo_retr(false, msg);

	SQL->EscapeStringLen(inter->sql_handle, sender_name, msg->sender_name, strnlen(msg->sender_name, NAME_LENGTH));
	SQL->EscapeStringLen(inter->sql_handle, receiver_name, msg->receiver_name, strnlen(msg->receiver_name, NAME_LENGTH));
	SQL->EscapeStringLen(inter->sql_handle, body, msg->body, strnlen(msg->body, RODEX_BODY_LENGTH));
	SQL->EscapeStringLen(inter->sql_handle, title, msg->title, strnlen(msg->title, RODEX_TITLE_LENGTH));

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` (`sender_name`, `sender_id`, `receiver_name`, `receiver_id`, `receiver_accountid`, `title`, `body`,"
		"`zeny`, `type`, `is_read`, `sender_read`, `send_date`, `expire_date`, `weight`) VALUES "
		"('%s', '%d', '%s', '%d', '%d', '%s', '%s', '%"PRId64"', '%d', '%d', '%d', '%d', '%d', '%d')",
		rodex_db, sender_name, msg->sender_id, receiver_name, msg->receiver_id, msg->receiver_accountid,
		title, body, msg->zeny, msg->type, msg->is_read == true ? 1 : 0, msg->sender_read == true ? 1 : 0, msg->send_date, msg->expire_date, msg->weight)) {
		Sql_ShowDebug(inter->sql_handle);
		return 0;
	}

	msg->id = (int64)SQL->LastInsertId(inter->sql_handle);

	for (i = 0; i < RODEX_MAX_ITEM; ++i) {
		// Should we use statement here? [KIRIEZ]
		struct item *it = &msg->items[i].item;
		if (it->nameid == 0)
			continue;

		if (SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` (`mail_id`, `nameid`, `amount`, `equip`, `identify`,"
			"`refine`, `grade`, `attribute`, `card0`, `card1`, `card2`, `card3`, `opt_idx0`, `opt_val0`, `opt_idx1`, `opt_val1`, `opt_idx2`,"
			"`opt_val2`, `opt_idx3`, `opt_val3`, `opt_idx4`, `opt_val4`,`expire_time`, `bound`, `unique_id`) VALUES "
			"('%"PRId64"', '%d', '%d', '%u', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u', '%u', '%"PRIu64"')",
			rodex_item_db, msg->id, it->nameid, it->amount, it->equip, it->identify, it->refine, it->grade, it->attribute, it->card[0], it->card[1], it->card[2], it->card[3],
			it->option[0].index, it->option[0].value, it->option[1].index, it->option[1].value, it->option[2].index, it->option[2].value, it->option[3].index,
			it->option[3].value, it->option[4].index, it->option[4].value, it->expire_time, it->bound, it->unique_id)
			) {
			Sql_ShowDebug(inter->sql_handle);
			continue;
		}
	}

	return msg->id;
}

static int64 inter_rodex_getzeny(int64 mail_id)
{
	Assert_retr(-1, mail_id > 0);

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `zeny`, `type` FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id)) {
		Sql_ShowDebug(inter->sql_handle);
	} else {
		if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
			char *data;
			SQL->GetData(inter->sql_handle, 0, &data, NULL);
			int64 zeny = atoi(data);
			SQL->GetData(inter->sql_handle, 1, &data, NULL);
			uint8 type = atoi(data);
			SQL->FreeResult(inter->sql_handle);
			if ((type & MAIL_TYPE_ZENY) == 0)
				return -1;
			return zeny;
		}
	}
	SQL->FreeResult(inter->sql_handle);

	return -1;
}

static int inter_rodex_getitems(int64 mail_id, struct rodex_item *items)
{
	Assert_retr(-1, mail_id > 0);
	nullpo_retr(-1, items);

	if (SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `type` FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return -1;
	} else {
		if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
			char *data;
			SQL->GetData(inter->sql_handle, 0, &data, NULL);
			uint8 type = atoi(data);
			SQL->FreeResult(inter->sql_handle);
			if ((type & MAIL_TYPE_ITEM) == 0)
				return -1;
		} else {
			SQL->FreeResult(inter->sql_handle);
			return -1;
		}
	}


	int itemsCount = 0;

		struct SqlStmt *stmt_items = SQL->StmtMalloc(inter->sql_handle);

		if (stmt_items == NULL) {
			return -1;
		}

		StringBuf buf;
		StrBuf->Init(&buf);

		StrBuf->AppendStr(&buf, "SELECT `nameid`, `amount`, `equip`, `identify`, `refine`, `grade`, `attribute`, `expire_time`, `bound`, `unique_id`");
		for (int i = 0; i < MAX_SLOTS; i++) {
			StrBuf->Printf(&buf, ", `card%d`", i);
		}
		for (int i = 0; i < MAX_ITEM_OPTIONS; i++) {
			StrBuf->Printf(&buf, ", `opt_idx%d`, `opt_val%d`", i, i);
		}
		StrBuf->Printf(&buf, "FROM `%s` WHERE mail_id = ? ORDER BY `mail_id` ASC", rodex_item_db);

		struct item it = { 0 };

		if (SQL_ERROR == SQL->StmtPrepareStr(stmt_items, StrBuf->Value(&buf))
		 || SQL_ERROR == SQL->StmtBindParam(stmt_items, 0, SQLDT_INT64, &mail_id, sizeof mail_id)
		) {
			SqlStmt_ShowDebug(stmt_items);
		}

				if (SQL_ERROR == SQL->StmtExecute(stmt_items)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 0,  SQLDT_INT,    &it.nameid,          sizeof it.nameid,      NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 1,  SQLDT_SHORT,  &it.amount,          sizeof it.amount,      NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 2,  SQLDT_UINT,   &it.equip,           sizeof it.equip,       NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 3,  SQLDT_CHAR,   &it.identify,        sizeof it.identify,    NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 4,  SQLDT_CHAR,   &it.refine,          sizeof it.refine,      NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 5,  SQLDT_CHAR,   &it.grade,           sizeof it.grade,       NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 6,  SQLDT_CHAR,   &it.attribute,       sizeof it.attribute,   NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 7,  SQLDT_UINT,   &it.expire_time,     sizeof it.expire_time, NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 8,  SQLDT_UCHAR,  &it.bound,           sizeof it.bound,       NULL, NULL)
				 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 9,  SQLDT_UINT64, &it.unique_id,       sizeof it.unique_id,   NULL, NULL)
				) {
					SqlStmt_ShowDebug(stmt_items);
				}
				for (int i = 0; i < MAX_SLOTS; i++) {
					if (SQL_ERROR == SQL->StmtBindColumn(stmt_items, 10 + i, SQLDT_INT, &it.card[i], sizeof it.card[i], NULL, NULL))
						SqlStmt_ShowDebug(stmt_items);
				}
				for (int i = 0; i < MAX_ITEM_OPTIONS; i++) {
					if (SQL_ERROR == SQL->StmtBindColumn(stmt_items, 10 + MAX_SLOTS + i * 2, SQLDT_INT16, &it.option[i].index, sizeof it.option[i].index, NULL, NULL)
					 || SQL_ERROR == SQL->StmtBindColumn(stmt_items, 11 + MAX_SLOTS + i * 2, SQLDT_INT16, &it.option[i].value, sizeof it.option[i].value, NULL, NULL)
					) {
						SqlStmt_ShowDebug(stmt_items);
					}
				}

				for (int i = 0; i < RODEX_MAX_ITEM && SQL_SUCCESS == SQL->StmtNextRow(stmt_items); ++i) {
					items[i].item = it;
					items[i].idx = itemsCount;
					itemsCount++;
				}

			SQL->StmtFreeResult(stmt_items);

		StrBuf->Destroy(&buf);
		SQL->StmtFree(stmt_items);

	return itemsCount;
}

/*==========================================
 * Update/Delete mail
 *------------------------------------------*/
static bool inter_rodex_updatemail(int fd, int account_id, int char_id, int64 mail_id, uint8 opentype, int8 flag)
{
	Assert_retr(false, fd >= 0);
	Assert_retr(false, account_id > 0);
	Assert_retr(false, char_id > 0);
	Assert_retr(false, mail_id > 0);
	Assert_retr(false, flag >= 0 && flag <= 4);

	switch (flag) {
	case 0: // Read
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `is_read` = 1 WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;

	case 1: // Get Zeny
	{
		const int64 zeny = inter_rodex->getzeny(mail_id);
		if (zeny != -1) {
			if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `zeny` = 0, `type` = `type` & (~2) WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id)) {
				Sql_ShowDebug(inter->sql_handle);
				break;
			}
		}
		mapif->rodex_getzenyack(fd, char_id, mail_id, opentype, zeny);
		break;
	}
	case 2: // Get Items
	{
		struct rodex_item items[RODEX_MAX_ITEM];
		const int count = inter_rodex->getitems(mail_id, &items[0]);
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_item_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		mapif->rodex_getitemsack(fd, char_id, mail_id, opentype, count, &items[0]);
		break;
	}
	case 3: // Delete Mail
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_item_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;

	case 4: // Sender Read
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `sender_read` = 1 WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;
	default:
		return false;
	}
	return true;
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
static int inter_rodex_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3095: mapif->parse_rodex_requestinbox(fd); break;
		case 0x3096: mapif->parse_rodex_checkhasnew(fd); break;
		case 0x3097: mapif->parse_rodex_updatemail(fd); break;
		case 0x3098: mapif->parse_rodex_send(fd); break;
		case 0x3099: mapif->parse_rodex_checkname(fd); break;
		default:
			return 0;
	}
	return 1;
}

static int inter_rodex_sql_init(void)
{
	return 1;
}

static void inter_rodex_sql_final(void)
{
	return;
}

void inter_rodex_defaults(void)
{
	inter_rodex = &inter_rodex_s;

	inter_rodex->savemessage = inter_rodex_savemessage;
	inter_rodex->parse_frommap = inter_rodex_parse_frommap;
	inter_rodex->sql_init = inter_rodex_sql_init;
	inter_rodex->sql_final = inter_rodex_sql_final;
	inter_rodex->fromsql = inter_rodex_fromsql;
	inter_rodex->hasnew = inter_rodex_hasnew;
	inter_rodex->checkname = inter_rodex_checkname;
	inter_rodex->updatemail = inter_rodex_updatemail;
	inter_rodex->getzeny = inter_rodex_getzeny;
	inter_rodex->getitems = inter_rodex_getitems;
}
