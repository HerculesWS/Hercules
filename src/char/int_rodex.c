/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017 Hercules Dev Team
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

struct inter_rodex_interface inter_rodex_s;
struct inter_rodex_interface *inter_rodex;

// Loads new mails of this char_id/account_id
static int inter_rodex_fromsql(int char_id, int account_id, int8 opentype, int64 mail_id, struct rodex_maillist *mails)
{
	int i, count = 0;
	struct rodex_message msg = { 0 };
	struct SqlStmt *stmt;
	struct SqlStmt *stmt_items;

	nullpo_retr(-1, mails);

	stmt = SQL->StmtMalloc(inter->sql_handle);

	switch (opentype) {
	case RODEX_OPENTYPE_MAIL:
		if (SQL_ERROR == SQL->StmtPrepare(stmt,
			"SELECT `mail_id`, `sender_name`, `sender_id`, `receiver_name`, `receiver_id`, `receiver_accountid`,"
			"`title`, `body`, `zeny`, `type`, `is_read`, `send_date`, `expire_date`, `weight`"
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
			"`title`, `body`, `zeny`, `type`, `is_read`, `send_date`, `expire_date`, `weight`"
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
			"`title`, `body`, `zeny`, `type`, `is_read`, `send_date`, `expire_date`, `weight`"
			"FROM `%s` WHERE (`sender_id` = '%d' AND `expire_date` <= '%d' AND `send_date` + '%d' > '%d' AND `mail_id` > '%"PRId64"')"
			"ORDER BY `mail_id` ASC", rodex_db, char_id, (int)time(NULL), 2 * RODEX_EXPIRE, (int)time(NULL), mail_id)
			) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			return -1;
		}
		break;
	}

	if (SQL_ERROR == SQL->StmtExecute(stmt)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 0, SQLDT_INT64, &msg.id, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 1, SQLDT_STRING, &msg.sender_name, sizeof(msg.sender_name), NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 2, SQLDT_INT, &msg.sender_id, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 3, SQLDT_STRING, &msg.receiver_name, sizeof(msg.receiver_name), NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 4, SQLDT_INT, &msg.receiver_id, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 5, SQLDT_INT, &msg.receiver_accountid, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 6, SQLDT_STRING, &msg.title, sizeof(msg.title), NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 7, SQLDT_STRING, &msg.body, sizeof(msg.body), NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 8, SQLDT_INT, &msg.zeny, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 9, SQLDT_UINT8, &msg.type, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 10, SQLDT_INT8, &msg.is_read, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 11, SQLDT_INT, &msg.send_date, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 12, SQLDT_INT, &msg.expire_date, 0, NULL, NULL)
		|| SQL_ERROR == SQL->StmtBindColumn(stmt, 13, SQLDT_INT, &msg.weight, 0, NULL, NULL)
		) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return -1;
	}

	stmt_items = SQL->StmtMalloc(inter->sql_handle);
	if (stmt_items == NULL) {
		SQL->StmtFreeResult(stmt);
		SQL->StmtFree(stmt);
		return -1;
	}

	// Read mails
	while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
		struct item it = { 0 };

		if (msg.type & MAIL_TYPE_ITEM) {
			if (SQL_ERROR == SQL->StmtPrepare(stmt_items, "SELECT `nameid`, `amount`, `equip`, `identify`,"
				"`refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `opt_idx0`, `opt_val0`,"
				"`opt_idx1`, `opt_val1`, `opt_idx2`, `opt_val2`, `opt_idx3`, `opt_val3`, `opt_idx4`, `opt_val4`,"
				"`expire_time`, `bound`, `unique_id`"
				"FROM `%s` WHERE mail_id = '%"PRId64"' ORDER BY `mail_id` ASC", rodex_item_db, msg.id)
				|| SQL_ERROR == SQL->StmtExecute(stmt_items)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 0, SQLDT_INT, &it.nameid, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 1, SQLDT_INT, &it.amount, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 2, SQLDT_UINT, &it.equip, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 3, SQLDT_INT8, &it.identify, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 4, SQLDT_INT8, &it.refine, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 5, SQLDT_INT8, &it.attribute, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 6, SQLDT_INT16, &it.card[0], 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 7, SQLDT_INT16, &it.card[1], 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 8, SQLDT_INT16, &it.card[2], 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 9, SQLDT_INT16, &it.card[3], 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 10, SQLDT_INT16, &it.option[0].index, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 11, SQLDT_INT16, &it.option[0].value, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 12, SQLDT_INT16, &it.option[1].index, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 13, SQLDT_INT16, &it.option[1].value, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 14, SQLDT_INT16, &it.option[2].index, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 15, SQLDT_INT16, &it.option[2].value, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 16, SQLDT_INT16, &it.option[3].index, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 17, SQLDT_INT16, &it.option[3].value, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 18, SQLDT_INT16, &it.option[4].index, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 19, SQLDT_INT16, &it.option[4].value, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 20, SQLDT_INT, &it.expire_time, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 21, SQLDT_UINT8, &it.bound, 0, NULL, NULL)
				|| SQL_ERROR == SQL->StmtBindColumn(stmt_items, 22, SQLDT_UINT64, &it.unique_id, 0, NULL, NULL)) {
				SqlStmt_ShowDebug(stmt_items);
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

		msg.opentype = opentype;
#if PACKETVER < 20160601
		// NPC Message Type isn't supported in old clients
		msg.type &= ~MAIL_TYPE_NPC;
#endif

		++count;
		VECTOR_ENSURE(*mails, 1, 1);
		VECTOR_PUSH(*mails, msg);
		memset(&msg, 0, sizeof(struct rodex_message));
	}

	SQL->StmtFreeResult(stmt);
	SQL->StmtFreeResult(stmt_items);

	SQL->StmtFree(stmt);
	SQL->StmtFree(stmt_items);

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
		"(`sender_id` = '%d' AND `expire_date` <= '%d' AND `send_date` + '%d' > '%d')"
		") AND (`is_read` = 0 OR (`type` > 0 AND `type` != 8))",
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
static bool inter_rodex_checkname(const char *name, int *target_char_id, short *target_class, int *target_level)
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
			SQL->GetData(inter->sql_handle, 1, &data, NULL); *target_class = (short)atoi(data);
			SQL->GetData(inter->sql_handle, 2, &data, NULL); *target_level = atoi(data);
			found = true;
		}
	}
	SQL->FreeResult(inter->sql_handle);

	return found;
}

/// Stores a single message in the database.
/// Returns the message's ID if successful (or 0 if it fails).
int64 inter_rodex_savemessage(struct rodex_message* msg)
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
		"`zeny`, `type`, `is_read`, `send_date`, `expire_date`, `weight`) VALUES "
		"('%s', '%d', '%s', '%d', '%d', '%s', '%s', '%"PRId64"', '%d', '%d', '%d', '%d', '%d')",
		rodex_db, sender_name, msg->sender_id, receiver_name, msg->receiver_id, msg->receiver_accountid,
		title, body, msg->zeny, msg->type, msg->is_read == true ? 1 : 0, msg->send_date, msg->expire_date, msg->weight)) {
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
			"`refine`, `attribute`, `card0`, `card1`, `card2`, `card3`, `opt_idx0`, `opt_val0`, `opt_idx1`, `opt_val1`, `opt_idx2`,"
			"`opt_val2`, `opt_idx3`, `opt_val3`, `opt_idx4`, `opt_val4`,`expire_time`, `bound`, `unique_id`) VALUES "
			"('%"PRId64"', '%d', '%d', '%u', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%u', '%u', '%"PRIu64"')",
			rodex_item_db, msg->id, it->nameid, it->amount, it->equip, it->identify, it->refine, it->attribute, it->card[0], it->card[1], it->card[2], it->card[3],
			it->option[0].index, it->option[0].value, it->option[1].index, it->option[1].value, it->option[2].index, it->option[2].value, it->option[3].index,
			it->option[3].value, it->option[4].index, it->option[4].value, it->expire_time, it->bound, it->unique_id)
			) {
			Sql_ShowDebug(inter->sql_handle);
			continue;
		}
	}

	return msg->id;
}

/*==========================================
 * Inbox Request
 *------------------------------------------*/
void mapif_rodex_sendinbox(int fd, int char_id, int8 opentype, int8 flag, int count, struct rodex_maillist *mails)
{
	int per_packet = (UINT16_MAX - 15) / sizeof(struct rodex_message);
	int sent = 0;
	nullpo_retv(mails);
	Assert_retv(char_id > 0);
	Assert_retv(count >= 0);

	do {
		int i = 15, j, size, limit;
		bool is_last = true;

		if (count <= per_packet) {
			size = count * sizeof(struct rodex_message) + 15;
			limit = count;
			is_last = true;
		} else {
			int to_send = count - sent;
			limit = min(to_send, per_packet);
			if (limit != to_send) {
				is_last = false;
			}
			size = limit * sizeof(struct rodex_message) + 15;
		}

		WFIFOHEAD(fd, size);
		WFIFOW(fd, 0) = 0x3895;
		WFIFOW(fd, 2) = size;
		WFIFOL(fd, 4) = char_id;
		WFIFOB(fd, 8) = opentype;
		WFIFOB(fd, 9) = flag;
		WFIFOB(fd, 10) = is_last;
		WFIFOL(fd, 11) = count;
		for (j = 0; j < limit; ++j, ++sent, i += sizeof(struct rodex_message)) {
			memcpy(WFIFOP(fd, i), &VECTOR_INDEX(*mails, sent), sizeof(struct rodex_message));
		}
		WFIFOSET(fd, size);
	} while (sent < count);
}

void mapif_parse_rodex_requestinbox(int fd)
{
	int count;
	int char_id = RFIFOL(fd,2);
	int account_id = RFIFOL(fd, 6);
	int8 flag = RFIFOB(fd, 10);
	int8 opentype = RFIFOB(fd, 11);
	int64 mail_id = RFIFOQ(fd, 12);
	struct rodex_maillist mails = { 0 };

	VECTOR_INIT(mails);
	if (flag == 0)
		count = inter_rodex->fromsql(char_id, account_id, opentype, 0, &mails);
	else
		count = inter_rodex->fromsql(char_id, account_id, opentype, mail_id, &mails);
	mapif->rodex_sendinbox(fd, char_id, opentype, flag, count, &mails);
	VECTOR_CLEAR(mails);
}

/*==========================================
* Checks if there are new mails
*------------------------------------------*/
void mapif_rodex_sendhasnew(int fd, int char_id, bool has_new)
{
	Assert_retv(char_id > 0);

	WFIFOHEAD(fd, 7);
	WFIFOW(fd, 0) = 0x3896;
	WFIFOL(fd, 2) = char_id;
	WFIFOB(fd, 6) = has_new;
	WFIFOSET(fd, 7);
}

void mapif_parse_rodex_checkhasnew(int fd)
{
	int char_id = RFIFOL(fd, 2);
	int account_id = RFIFOL(fd, 6);
	bool has_new;

	Assert_retv(account_id >= START_ACCOUNT_NUM && account_id <= END_ACCOUNT_NUM);
	Assert_retv(char_id >= START_CHAR_NUM);

	has_new = inter_rodex->hasnew(char_id, account_id);
	mapif->rodex_sendhasnew(fd, char_id, has_new);
}

/*==========================================
 * Update/Delete mail
 *------------------------------------------*/
void mapif_parse_rodex_updatemail(int fd)
{
	int64 mail_id = RFIFOL(fd, 2);
	int8 flag = RFIFOB(fd, 10);

	Assert_retv(mail_id > 0);
	Assert_retv(flag >= 0 && flag <= 3);

	switch (flag) {
	case 0: // Read
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `is_read` = 1 WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;

	case 1: // Get Zeny
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `zeny` = 0, `type` = `type` & (~2) WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;

	case 2: // Get Items
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_item_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `zeny` = 0, `type` = `type` & (~4) WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;

	case 3: // Delete Mail
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `mail_id` = '%"PRId64"'", rodex_item_db, mail_id))
			Sql_ShowDebug(inter->sql_handle);
		break;
	}
}

/*==========================================
 * Send Mail
 *------------------------------------------*/
void mapif_rodex_send(int fd, int sender_id, int receiver_id, int receiver_accountid, bool result)
{
	Assert_retv(sender_id >= 0);
	Assert_retv(receiver_id + receiver_accountid > 0);

	WFIFOHEAD(fd,15);
	WFIFOW(fd,0) = 0x3897;
	WFIFOL(fd,2) = sender_id;
	WFIFOL(fd,6) = receiver_id;
	WFIFOL(fd,10) = receiver_accountid;
	WFIFOB(fd,14) = result;
	WFIFOSET(fd,15);
}

void mapif_parse_rodex_send(int fd)
{
	struct rodex_message msg = { 0 };

	if (RFIFOW(fd,2) != 4 + sizeof(struct rodex_message))
		return;

	memcpy(&msg, RFIFOP(fd,4), sizeof(struct rodex_message));
	if (msg.receiver_id > 0 || msg.receiver_accountid > 0)
		msg.id = inter_rodex->savemessage(&msg);

	mapif->rodex_send(fd, msg.sender_id, msg.receiver_id, msg.receiver_accountid, msg.id > 0 ? true : false);
}

/*------------------------------------------
 * Check Player
 *------------------------------------------*/
void mapif_rodex_checkname(int fd, int reqchar_id, int target_char_id, short target_class, int target_level, char *name)
{
	nullpo_retv(name);
	Assert_retv(reqchar_id > 0);
	Assert_retv(target_char_id >= 0);

	WFIFOHEAD(fd, 16 + NAME_LENGTH);
	WFIFOW(fd, 0) = 0x3898;
	WFIFOL(fd, 2) = reqchar_id;
	WFIFOL(fd, 6) = target_char_id;
	WFIFOW(fd, 10) = target_class;
	WFIFOL(fd, 12) = target_level;
	safestrncpy(WFIFOP(fd, 16), name, NAME_LENGTH);
	WFIFOSET(fd, 16 + NAME_LENGTH);
}

void mapif_parse_rodex_checkname(int fd)
{
	int reqchar_id = RFIFOL(fd, 2);
	char name[NAME_LENGTH];
	int target_char_id, target_level;
	short target_class;

	safestrncpy(name, RFIFOP(fd, 6), NAME_LENGTH);

	if (inter_rodex->checkname(name, &target_char_id, &target_class, &target_level) == true)
		mapif->rodex_checkname(fd, reqchar_id, target_char_id, target_class, target_level, name);
	else
		mapif->rodex_checkname(fd, reqchar_id, 0, 0, 0, name);
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_rodex_parse_frommap(int fd)
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

int inter_rodex_sql_init(void)
{
	return 1;
}

void inter_rodex_sql_final(void)
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
}
