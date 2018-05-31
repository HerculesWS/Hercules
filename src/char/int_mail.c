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

#include "int_mail.h"

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
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>

struct inter_mail_interface inter_mail_s;
struct inter_mail_interface *inter_mail;

static int inter_mail_fromsql(int char_id, struct mail_data* md)
{
	int i, j;
	struct mail_message *msg;
	char *data;
	StringBuf buf;

	nullpo_ret(md);
	memset(md, 0, sizeof(struct mail_data));
	md->amount = 0;
	md->full = false;

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,"
		"`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`,`unique_id`");
	for (i = 0; i < MAX_SLOTS; i++)
		StrBuf->Printf(&buf, ",`card%d`", i);
	for (i = 0; i < MAX_ITEM_OPTIONS; i++)
		StrBuf->Printf(&buf, ", `opt_idx%d`, `opt_val%d`", i, i);

	// I keep the `status` < 3 just in case someone forget to apply the sqlfix
	StrBuf->Printf(&buf, " FROM `%s` WHERE `dest_id`='%d' AND `status` < 3 ORDER BY `id` LIMIT %d",
		mail_db, char_id, MAIL_MAX_INBOX + 1);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)))
		Sql_ShowDebug(inter->sql_handle);

	StrBuf->Destroy(&buf);

	for (i = 0; i < MAIL_MAX_INBOX && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); ++i )
	{
		struct item *item;
		msg = &md->msg[i];
		SQL->GetData(inter->sql_handle, 0, &data, NULL); msg->id = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 2, &data, NULL); msg->send_id = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 4, &data, NULL); msg->dest_id = atoi(data);
		SQL->GetData(inter->sql_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		SQL->GetData(inter->sql_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		SQL->GetData(inter->sql_handle, 7, &data, NULL); msg->timestamp = atoi(data);
		SQL->GetData(inter->sql_handle, 8, &data, NULL); msg->status = (mail_status)atoi(data);
		SQL->GetData(inter->sql_handle, 9, &data, NULL); msg->zeny = atoi(data);
		item = &msg->item;
		SQL->GetData(inter->sql_handle,10, &data, NULL); item->amount = (short)atoi(data);
		SQL->GetData(inter->sql_handle,11, &data, NULL); item->nameid = atoi(data);
		SQL->GetData(inter->sql_handle,12, &data, NULL); item->refine = atoi(data);
		SQL->GetData(inter->sql_handle,13, &data, NULL); item->attribute = atoi(data);
		SQL->GetData(inter->sql_handle,14, &data, NULL); item->identify = atoi(data);
		SQL->GetData(inter->sql_handle,15, &data, NULL); item->unique_id = strtoull(data, NULL, 10);
		item->expire_time = 0;
		item->bound = 0;
		/* Card Slots */
		for (j = 0; j < MAX_SLOTS; j++) {
			SQL->GetData(inter->sql_handle, 16 + j, &data, NULL);
			item->card[j] = atoi(data);
		}
		/* Item Options */
		for (j = 0; j < MAX_ITEM_OPTIONS; j++) {
			SQL->GetData(inter->sql_handle, 16 + MAX_SLOTS + j * 2, &data, NULL);
			item->option[j].index = atoi(data);
			SQL->GetData(inter->sql_handle, 17 + MAX_SLOTS + j * 2, &data, NULL);
			item->option[j].value = atoi(data);
		}
	}

	md->full = ( SQL->NumRows(inter->sql_handle) > MAIL_MAX_INBOX );

	md->amount = i;
	SQL->FreeResult(inter->sql_handle);

	md->unchecked = 0;
	md->unread = 0;
	for (i = 0; i < md->amount; i++)
	{
		msg = &md->msg[i];
		if( msg->status == MAIL_NEW )
		{
			if ( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", mail_db, MAIL_UNREAD, msg->id) )
				Sql_ShowDebug(inter->sql_handle);

			msg->status = MAIL_UNREAD;
			md->unchecked++;
		}
		else if ( msg->status == MAIL_UNREAD )
			md->unread++;
	}

	ShowInfo("mail load complete from DB - id: %d (total: %d)\n", char_id, md->amount);
	return 1;
}

/// Stores a single message in the database.
/// Returns the message's ID if successful (or 0 if it fails).
int inter_mail_savemessage(struct mail_message* msg)
{
	StringBuf buf;
	struct SqlStmt *stmt;
	int j;

	nullpo_ret(msg);
	// build message save query
	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "INSERT INTO `%s` (`send_name`, `send_id`, `dest_name`, `dest_id`, `title`, `message`, `time`, `status`, `zeny`, `amount`, `nameid`, `refine`, `attribute`, `identify`, `unique_id`", mail_db);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ", `card%d`", j);
	for (j = 0; j < MAX_ITEM_OPTIONS; j++)
		StrBuf->Printf(&buf, ", `opt_idx%d`, `opt_val%d`", j, j);
	StrBuf->Printf(&buf, ") VALUES (?, '%d', ?, '%d', ?, ?, '%lu', '%u', '%d', '%d', '%d', '%d', '%d', '%d', '%"PRIu64"'",
		msg->send_id, msg->dest_id, (unsigned long)msg->timestamp, msg->status, msg->zeny, msg->item.amount, msg->item.nameid, msg->item.refine, msg->item.attribute, msg->item.identify, msg->item.unique_id);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ", '%d'", msg->item.card[j]);
	for (j = 0; j < MAX_ITEM_OPTIONS; j++)
		StrBuf->Printf(&buf, ", '%d', '%d'", msg->item.option[j].index, msg->item.option[j].value);
	StrBuf->AppendStr(&buf, ")");

	// prepare and execute query
	stmt = SQL->StmtMalloc(inter->sql_handle);
	if (SQL_SUCCESS != SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, msg->send_name, strnlen(msg->send_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 2, SQLDT_STRING, msg->title, strnlen(msg->title, MAIL_TITLE_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 3, SQLDT_STRING, msg->body, strnlen(msg->body, MAIL_BODY_LENGTH))
	||  SQL_SUCCESS != SQL->StmtExecute(stmt))
	{
		SqlStmt_ShowDebug(stmt);
		msg->id = 0;
	} else {
		msg->id = (int)SQL->StmtLastInsertId(stmt);
	}

	SQL->StmtFree(stmt);
	StrBuf->Destroy(&buf);

	return msg->id;
}

/// Retrieves a single message from the database.
/// Returns true if the operation succeeds (or false if it fails).
static bool inter_mail_loadmessage(int mail_id, struct mail_message* msg)
{
	int j;
	StringBuf buf;
	nullpo_ret(msg);
	memset(msg, 0, sizeof(struct mail_message)); // Initialize data

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,"
		"`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`,`unique_id`");
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ",`card%d`", j);
	for (j = 0; j < MAX_ITEM_OPTIONS; j++)
		StrBuf->Printf(&buf, ",`opt_idx%d`,`opt_val%d`", j, j);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `id` = '%d'", mail_db, mail_id);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))
	 || SQL_SUCCESS != SQL->NextRow(inter->sql_handle)) {
		Sql_ShowDebug(inter->sql_handle);
		SQL->FreeResult(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	} else {
		char* data;

		SQL->GetData(inter->sql_handle, 0, &data, NULL); msg->id = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 2, &data, NULL); msg->send_id = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 4, &data, NULL); msg->dest_id = atoi(data);
		SQL->GetData(inter->sql_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		SQL->GetData(inter->sql_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		SQL->GetData(inter->sql_handle, 7, &data, NULL); msg->timestamp = atoi(data);
		SQL->GetData(inter->sql_handle, 8, &data, NULL); msg->status = (mail_status)atoi(data);
		SQL->GetData(inter->sql_handle, 9, &data, NULL); msg->zeny = atoi(data);
		SQL->GetData(inter->sql_handle,10, &data, NULL); msg->item.amount = (short)atoi(data);
		SQL->GetData(inter->sql_handle,11, &data, NULL); msg->item.nameid = atoi(data);
		SQL->GetData(inter->sql_handle,12, &data, NULL); msg->item.refine = atoi(data);
		SQL->GetData(inter->sql_handle,13, &data, NULL); msg->item.attribute = atoi(data);
		SQL->GetData(inter->sql_handle,14, &data, NULL); msg->item.identify = atoi(data);
		SQL->GetData(inter->sql_handle,15, &data, NULL); msg->item.unique_id = strtoull(data, NULL, 10);
		msg->item.expire_time = 0;
		msg->item.bound = 0;
		/* Card Slots */
		for (j = 0; j < MAX_SLOTS; j++) {
			SQL->GetData(inter->sql_handle,16 + j, &data, NULL);
			msg->item.card[j] = atoi(data);
		}
		/* Item Options */
		for (j = 0 ; j < MAX_ITEM_OPTIONS; j++) {
			SQL->GetData(inter->sql_handle, 16 + MAX_SLOTS + j * 2, &data, NULL);
			msg->item.option[j].index = atoi(data);
			SQL->GetData(inter->sql_handle, 17 + MAX_SLOTS + j * 2, &data, NULL);
			msg->item.option[j].value = atoi(data);
		}
	}

	StrBuf->Destroy(&buf);
	SQL->FreeResult(inter->sql_handle);

	return true;
}

void mapif_mail_sendinbox(int fd, int char_id, unsigned char flag, struct mail_data *md)
{
	nullpo_retv(md);
	//FIXME: dumping the whole structure like this is unsafe [ultramage]
	WFIFOHEAD(fd, sizeof(struct mail_data) + 9);
	WFIFOW(fd,0) = 0x3848;
	WFIFOW(fd,2) = sizeof(struct mail_data) + 9;
	WFIFOL(fd,4) = char_id;
	WFIFOB(fd,8) = flag;
	memcpy(WFIFOP(fd,9),md,sizeof(struct mail_data));
	WFIFOSET(fd,WFIFOW(fd,2));
}

/*==========================================
 * Client Inbox Request
 *------------------------------------------*/
void mapif_parse_mail_requestinbox(int fd)
{
	int char_id = RFIFOL(fd,2);
	unsigned char flag = RFIFOB(fd,6);
	struct mail_data md;
	memset(&md, 0, sizeof(md));
	inter_mail->fromsql(char_id, &md);
	mapif->mail_sendinbox(fd, char_id, flag, &md);
}

/*==========================================
 * Mark mail as 'Read'
 *------------------------------------------*/
void mapif_parse_mail_read(int fd)
{
	int mail_id = RFIFOL(fd,2);
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", mail_db, MAIL_READ, mail_id) )
		Sql_ShowDebug(inter->sql_handle);
}

/*==========================================
 * Client Attachment Request
 *------------------------------------------*/
static bool inter_mail_DeleteAttach(int mail_id)
{
	StringBuf buf;
	int i;

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "UPDATE `%s` SET `zeny` = '0', `nameid` = '0', `amount` = '0', `refine` = '0', `attribute` = '0', `identify` = '0'", mail_db);
	for (i = 0; i < MAX_SLOTS; i++)
		StrBuf->Printf(&buf, ", `card%d` = '0'", i);
	for (i = 0; i < MAX_ITEM_OPTIONS; i++)
		StrBuf->Printf(&buf, ", `opt_idx%d` = '0', `opt_val%d` = '0'", i, i);
	StrBuf->Printf(&buf, " WHERE `id` = '%d'", mail_id);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);

		return false;
	}

	StrBuf->Destroy(&buf);
	return true;
}

void mapif_mail_sendattach(int fd, int char_id, struct mail_message *msg)
{
	nullpo_retv(msg);
	WFIFOHEAD(fd, sizeof(struct item) + 12);
	WFIFOW(fd,0) = 0x384a;
	WFIFOW(fd,2) = sizeof(struct item) + 12;
	WFIFOL(fd,4) = char_id;
	WFIFOL(fd,8) = (msg->zeny > 0)?msg->zeny:0;
	memcpy(WFIFOP(fd,12), &msg->item, sizeof(struct item));
	WFIFOSET(fd,WFIFOW(fd,2));
}

void mapif_mail_getattach(int fd, int char_id, int mail_id)
{
	struct mail_message msg;
	memset(&msg, 0, sizeof(msg));

	if( !inter_mail->loadmessage(mail_id, &msg) )
		return;

	if( msg.dest_id != char_id )
		return;

	if( msg.status != MAIL_READ )
		return;

	if( (msg.item.nameid < 1 || msg.item.amount < 1) && msg.zeny < 1 )
		return; // No Attachment

	if( !inter_mail->DeleteAttach(mail_id) )
		return;

	mapif->mail_sendattach(fd, char_id, &msg);
}

void mapif_parse_mail_getattach(int fd)
{
	mapif->mail_getattach(fd, RFIFOL(fd,2), RFIFOL(fd,6));
}

/*==========================================
 * Delete Mail
 *------------------------------------------*/
void mapif_mail_delete(int fd, int char_id, int mail_id, bool failed)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384b;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = failed;
	WFIFOSET(fd,11);
}

void mapif_parse_mail_delete(int fd)
{
	int char_id = RFIFOL(fd,2);
	int mail_id = RFIFOL(fd,6);
	bool failed = false;
	if ( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", mail_db, mail_id) )
	{
		Sql_ShowDebug(inter->sql_handle);
		failed = true;
	}
	mapif->mail_delete(fd, char_id, mail_id, failed);
}

/*==========================================
 * Report New Mail to Map Server
 *------------------------------------------*/
void mapif_mail_new(struct mail_message *msg)
{
	unsigned char buf[74];

	if( !msg || !msg->id )
		return;

	WBUFW(buf,0) = 0x3849;
	WBUFL(buf,2) = msg->dest_id;
	WBUFL(buf,6) = msg->id;
	memcpy(WBUFP(buf,10), msg->send_name, NAME_LENGTH);
	memcpy(WBUFP(buf,34), msg->title, MAIL_TITLE_LENGTH);
	mapif->sendall(buf, 74);
}

/*==========================================
 * Return Mail
 *------------------------------------------*/
void mapif_mail_return(int fd, int char_id, int mail_id, int new_mail)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = (new_mail == 0);
	WFIFOSET(fd,11);
}

void mapif_parse_mail_return(int fd)
{
	int char_id = RFIFOL(fd,2);
	int mail_id = RFIFOL(fd,6);
	struct mail_message msg;
	int new_mail = 0;

	if( inter_mail->loadmessage(mail_id, &msg) )
	{
		if( msg.dest_id != char_id)
			return;
		else if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", mail_db, mail_id) )
			Sql_ShowDebug(inter->sql_handle);
		else
		{
			char temp_[MAIL_TITLE_LENGTH];

			// swap sender and receiver
			swap(msg.send_id, msg.dest_id);
			safestrncpy(temp_, msg.send_name, NAME_LENGTH);
			safestrncpy(msg.send_name, msg.dest_name, NAME_LENGTH);
			safestrncpy(msg.dest_name, temp_, NAME_LENGTH);

			// set reply message title
			safesnprintf(temp_, MAIL_TITLE_LENGTH, "RE:%s", msg.title);
			safestrncpy(msg.title, temp_, MAIL_TITLE_LENGTH);

			msg.status = MAIL_NEW;
			msg.timestamp = time(NULL);

			new_mail = inter_mail->savemessage(&msg);
			mapif->mail_new(&msg);
		}
	}

	mapif->mail_return(fd, char_id, mail_id, new_mail);
}

/*==========================================
 * Send Mail
 *------------------------------------------*/
void mapif_mail_send(int fd, struct mail_message* msg)
{
	int len = sizeof(struct mail_message) + 4;

	nullpo_retv(msg);
	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x384d;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), msg, sizeof(struct mail_message));
	WFIFOSET(fd,len);
}

void mapif_parse_mail_send(int fd)
{
	struct mail_message msg;
	char esc_name[NAME_LENGTH*2+1];
	int account_id = 0;

	if(RFIFOW(fd,2) != 8 + sizeof(struct mail_message))
		return;

	account_id = RFIFOL(fd,4);
	memcpy(&msg, RFIFOP(fd,8), sizeof(struct mail_message));

	// Try to find the Dest Char by Name
	SQL->EscapeStringLen(inter->sql_handle, esc_name, msg.dest_name, strnlen(msg.dest_name, NAME_LENGTH));
	if ( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `account_id`, `char_id` FROM `%s` WHERE `name` = '%s'", char_db, esc_name) )
		Sql_ShowDebug(inter->sql_handle);
	else
	if ( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) )
	{
		char *data;
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		if (atoi(data) != account_id)
		{ // Cannot send mail to char in the same account
			SQL->GetData(inter->sql_handle, 1, &data, NULL);
			msg.dest_id = atoi(data);
		}
	}
	SQL->FreeResult(inter->sql_handle);
	msg.status = MAIL_NEW;

	if( msg.dest_id > 0 )
		msg.id = inter_mail->savemessage(&msg);

	mapif->mail_send(fd, &msg); // notify sender
	mapif->mail_new(&msg); // notify recipient
}

void inter_mail_sendmail(int send_id, const char* send_name, int dest_id, const char* dest_name, const char* title, const char* body, int zeny, struct item *item)
{
	struct mail_message msg;
	nullpo_retv(send_name);
	nullpo_retv(dest_name);
	nullpo_retv(title);
	nullpo_retv(body);
	memset(&msg, 0, sizeof(struct mail_message));

	msg.send_id = send_id;
	safestrncpy(msg.send_name, send_name, NAME_LENGTH);
	msg.dest_id = dest_id;
	safestrncpy(msg.dest_name, dest_name, NAME_LENGTH);
	safestrncpy(msg.title, title, MAIL_TITLE_LENGTH);
	safestrncpy(msg.body, body, MAIL_BODY_LENGTH);
	msg.zeny = zeny;
	if( item != NULL )
		memcpy(&msg.item, item, sizeof(struct item));

	msg.timestamp = time(NULL);

	inter_mail->savemessage(&msg);
	mapif->mail_new(&msg);
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_mail_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3048: mapif->parse_mail_requestinbox(fd); break;
		case 0x3049: mapif->parse_mail_read(fd); break;
		case 0x304a: mapif->parse_mail_getattach(fd); break;
		case 0x304b: mapif->parse_mail_delete(fd); break;
		case 0x304c: mapif->parse_mail_return(fd); break;
		case 0x304d: mapif->parse_mail_send(fd); break;
		default:
			return 0;
	}
	return 1;
}

int inter_mail_sql_init(void)
{
	return 1;
}

void inter_mail_sql_final(void)
{
	return;
}

void inter_mail_defaults(void)
{
	inter_mail = &inter_mail_s;

	inter_mail->savemessage = inter_mail_savemessage;
	inter_mail->DeleteAttach = inter_mail_DeleteAttach;
	inter_mail->sendmail = inter_mail_sendmail;
	inter_mail->parse_frommap = inter_mail_parse_frommap;
	inter_mail->sql_init = inter_mail_sql_init;
	inter_mail->sql_final = inter_mail_sql_final;
	inter_mail->fromsql = inter_mail_fromsql;
	inter_mail->loadmessage = inter_mail_loadmessage;
}
