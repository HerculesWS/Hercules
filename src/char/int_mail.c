// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int mail_fromsql(int char_id, struct mail_data* md)
{
	int i, j;
	struct mail_message *msg;
	struct item *item;
	char *data;
	StringBuf buf;

	memset(md, 0, sizeof(struct mail_data));
	md->amount = 0;
	md->full = false;

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,"
		"`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`,`unique_id`");
	for (i = 0; i < MAX_SLOTS; i++)
		StrBuf->Printf(&buf, ",`card%d`", i);

	// I keep the `status` < 3 just in case someone forget to apply the sqlfix
	StrBuf->Printf(&buf, " FROM `%s` WHERE `dest_id`='%d' AND `status` < 3 ORDER BY `id` LIMIT %d",
		mail_db, char_id, MAIL_MAX_INBOX + 1);

	if( SQL_ERROR == SQL->Query(sql_handle, StrBuf->Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StrBuf->Destroy(&buf);

	for (i = 0; i < MAIL_MAX_INBOX && SQL_SUCCESS == SQL->NextRow(sql_handle); ++i )
	{
		msg = &md->msg[i];
		SQL->GetData(sql_handle, 0, &data, NULL); msg->id = atoi(data);
		SQL->GetData(sql_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		SQL->GetData(sql_handle, 2, &data, NULL); msg->send_id = atoi(data);
		SQL->GetData(sql_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		SQL->GetData(sql_handle, 4, &data, NULL); msg->dest_id = atoi(data);
		SQL->GetData(sql_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		SQL->GetData(sql_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		SQL->GetData(sql_handle, 7, &data, NULL); msg->timestamp = atoi(data);
		SQL->GetData(sql_handle, 8, &data, NULL); msg->status = (mail_status)atoi(data);
		SQL->GetData(sql_handle, 9, &data, NULL); msg->zeny = atoi(data);
		item = &msg->item;
		SQL->GetData(sql_handle,10, &data, NULL); item->amount = (short)atoi(data);
		SQL->GetData(sql_handle,11, &data, NULL); item->nameid = atoi(data);
		SQL->GetData(sql_handle,12, &data, NULL); item->refine = atoi(data);
		SQL->GetData(sql_handle,13, &data, NULL); item->attribute = atoi(data);
		SQL->GetData(sql_handle,14, &data, NULL); item->identify = atoi(data);
		SQL->GetData(sql_handle,15, &data, NULL); item->unique_id = strtoull(data, NULL, 10);
		item->expire_time = 0;
		item->bound = 0;

		for (j = 0; j < MAX_SLOTS; j++)
		{
			SQL->GetData(sql_handle, 16 + j, &data, NULL);
			item->card[j] = atoi(data);
		}
	}

	md->full = ( SQL->NumRows(sql_handle) > MAIL_MAX_INBOX );

	md->amount = i;
	SQL->FreeResult(sql_handle);

	md->unchecked = 0;
	md->unread = 0;
	for (i = 0; i < md->amount; i++)
	{
		msg = &md->msg[i];
		if( msg->status == MAIL_NEW )
		{
			if ( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", mail_db, MAIL_UNREAD, msg->id) )
				Sql_ShowDebug(sql_handle);

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
int mail_savemessage(struct mail_message* msg)
{
	StringBuf buf;
	SqlStmt* stmt;
	int j;

	// build message save query
	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "INSERT INTO `%s` (`send_name`, `send_id`, `dest_name`, `dest_id`, `title`, `message`, `time`, `status`, `zeny`, `amount`, `nameid`, `refine`, `attribute`, `identify`, `unique_id`", mail_db);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ", `card%d`", j);
	StrBuf->Printf(&buf, ") VALUES (?, '%d', ?, '%d', ?, ?, '%lu', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%"PRIu64"'",
		msg->send_id, msg->dest_id, (unsigned long)msg->timestamp, msg->status, msg->zeny, msg->item.amount, msg->item.nameid, msg->item.refine, msg->item.attribute, msg->item.identify, msg->item.unique_id);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ", '%d'", msg->item.card[j]);
	StrBuf->AppendStr(&buf, ")");

	// prepare and execute query
	stmt = SQL->StmtMalloc(sql_handle);
	if( SQL_SUCCESS != SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, msg->send_name, strnlen(msg->send_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 2, SQLDT_STRING, msg->title, strnlen(msg->title, MAIL_TITLE_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 3, SQLDT_STRING, msg->body, strnlen(msg->body, MAIL_BODY_LENGTH))
	||  SQL_SUCCESS != SQL->StmtExecute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		msg->id = 0;
	} else
		msg->id = (int)SQL->StmtLastInsertId(stmt);

	SQL->StmtFree(stmt);
	StrBuf->Destroy(&buf);

	return msg->id;
}

/// Retrieves a single message from the database.
/// Returns true if the operation succeeds (or false if it fails).
static bool mail_loadmessage(int mail_id, struct mail_message* msg)
{
	int j;
	StringBuf buf;

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`status`,"
		"`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`,`unique_id`");
	for( j = 0; j < MAX_SLOTS; j++ )
		StrBuf->Printf(&buf, ",`card%d`", j);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `id` = '%d'", mail_db, mail_id);

	if( SQL_ERROR == SQL->Query(sql_handle, StrBuf->Value(&buf))
	||  SQL_SUCCESS != SQL->NextRow(sql_handle) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->FreeResult(sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	}
	else
	{
		char* data;

		SQL->GetData(sql_handle, 0, &data, NULL); msg->id = atoi(data);
		SQL->GetData(sql_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		SQL->GetData(sql_handle, 2, &data, NULL); msg->send_id = atoi(data);
		SQL->GetData(sql_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		SQL->GetData(sql_handle, 4, &data, NULL); msg->dest_id = atoi(data);
		SQL->GetData(sql_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		SQL->GetData(sql_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		SQL->GetData(sql_handle, 7, &data, NULL); msg->timestamp = atoi(data);
		SQL->GetData(sql_handle, 8, &data, NULL); msg->status = (mail_status)atoi(data);
		SQL->GetData(sql_handle, 9, &data, NULL); msg->zeny = atoi(data);
		SQL->GetData(sql_handle,10, &data, NULL); msg->item.amount = (short)atoi(data);
		SQL->GetData(sql_handle,11, &data, NULL); msg->item.nameid = atoi(data);
		SQL->GetData(sql_handle,12, &data, NULL); msg->item.refine = atoi(data);
		SQL->GetData(sql_handle,13, &data, NULL); msg->item.attribute = atoi(data);
		SQL->GetData(sql_handle,14, &data, NULL); msg->item.identify = atoi(data);
		SQL->GetData(sql_handle,15, &data, NULL); msg->item.unique_id = strtoull(data, NULL, 10);
		msg->item.expire_time = 0;
		msg->item.bound = 0;

		for( j = 0; j < MAX_SLOTS; j++ )
		{
			SQL->GetData(sql_handle,16 + j, &data, NULL);
			msg->item.card[j] = atoi(data);
		}
	}

	StrBuf->Destroy(&buf);
	SQL->FreeResult(sql_handle);

	return true;
}

/*==========================================
 * Client Inbox Request
 *------------------------------------------*/
static void mapif_Mail_sendinbox(int fd, int char_id, unsigned char flag)
{
	struct mail_data md;
	mail_fromsql(char_id, &md);

	//FIXME: dumping the whole structure like this is unsafe [ultramage]
	WFIFOHEAD(fd, sizeof(md) + 9);
	WFIFOW(fd,0) = 0x3848;
	WFIFOW(fd,2) = sizeof(md) + 9;
	WFIFOL(fd,4) = char_id;
	WFIFOB(fd,8) = flag;
	memcpy(WFIFOP(fd,9),&md,sizeof(md));
	WFIFOSET(fd,WFIFOW(fd,2));
}

static void mapif_parse_Mail_requestinbox(int fd)
{
	mapif_Mail_sendinbox(fd, RFIFOL(fd,2), RFIFOB(fd,6));
}

/*==========================================
 * Mark mail as 'Read'
 *------------------------------------------*/
static void mapif_parse_Mail_read(int fd)
{
	int mail_id = RFIFOL(fd,2);
	if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `status` = '%d' WHERE `id` = '%d'", mail_db, MAIL_READ, mail_id) )
		Sql_ShowDebug(sql_handle);
}

/*==========================================
 * Client Attachment Request
 *------------------------------------------*/
static bool mail_DeleteAttach(int mail_id)
{
	StringBuf buf;
	int i;

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "UPDATE `%s` SET `zeny` = '0', `nameid` = '0', `amount` = '0', `refine` = '0', `attribute` = '0', `identify` = '0'", mail_db);
	for (i = 0; i < MAX_SLOTS; i++)
		StrBuf->Printf(&buf, ", `card%d` = '0'", i);
	StrBuf->Printf(&buf, " WHERE `id` = '%d'", mail_id);

	if( SQL_ERROR == SQL->Query(sql_handle, StrBuf->Value(&buf)) )
	{
		Sql_ShowDebug(sql_handle);
		StrBuf->Destroy(&buf);

		return false;
	}

	StrBuf->Destroy(&buf);
	return true;
}

static void mapif_Mail_getattach(int fd, int char_id, int mail_id)
{
	struct mail_message msg;

	if( !mail_loadmessage(mail_id, &msg) )
		return;

	if( msg.dest_id != char_id )
		return;

	if( msg.status != MAIL_READ )
		return;

	if( (msg.item.nameid < 1 || msg.item.amount < 1) && msg.zeny < 1 )
		return; // No Attachment

	if( !mail_DeleteAttach(mail_id) )
		return;

	WFIFOHEAD(fd, sizeof(struct item) + 12);
	WFIFOW(fd,0) = 0x384a;
	WFIFOW(fd,2) = sizeof(struct item) + 12;
	WFIFOL(fd,4) = char_id;
	WFIFOL(fd,8) = (msg.zeny > 0)?msg.zeny:0;
	memcpy(WFIFOP(fd,12), &msg.item, sizeof(struct item));
	WFIFOSET(fd,WFIFOW(fd,2));
}

static void mapif_parse_Mail_getattach(int fd)
{
	mapif_Mail_getattach(fd, RFIFOL(fd,2), RFIFOL(fd,6));
}

/*==========================================
 * Delete Mail
 *------------------------------------------*/
static void mapif_Mail_delete(int fd, int char_id, int mail_id)
{
	bool failed = false;
	if ( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", mail_db, mail_id) )
	{
		Sql_ShowDebug(sql_handle);
		failed = true;
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384b;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = failed;
	WFIFOSET(fd,11);
}

static void mapif_parse_Mail_delete(int fd)
{
	mapif_Mail_delete(fd, RFIFOL(fd,2), RFIFOL(fd,6));
}

/*==========================================
 * Report New Mail to Map Server
 *------------------------------------------*/
void mapif_Mail_new(struct mail_message *msg)
{
	unsigned char buf[74];
	
	if( !msg || !msg->id )
		return;

	WBUFW(buf,0) = 0x3849;
	WBUFL(buf,2) = msg->dest_id;
	WBUFL(buf,6) = msg->id;
	memcpy(WBUFP(buf,10), msg->send_name, NAME_LENGTH);
	memcpy(WBUFP(buf,34), msg->title, MAIL_TITLE_LENGTH);
	mapif_sendall(buf, 74);
}

/*==========================================
 * Return Mail
 *------------------------------------------*/
static void mapif_Mail_return(int fd, int char_id, int mail_id)
{
	struct mail_message msg;
	int new_mail = 0;

	if( mail_loadmessage(mail_id, &msg) )
	{
		if( msg.dest_id != char_id)
			return;
		else if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d'", mail_db, mail_id) )
			Sql_ShowDebug(sql_handle);
		else
		{
			char temp_[MAIL_TITLE_LENGTH];

			// swap sender and receiver
			swap(msg.send_id, msg.dest_id);
			safestrncpy(temp_, msg.send_name, NAME_LENGTH);
			safestrncpy(msg.send_name, msg.dest_name, NAME_LENGTH);
			safestrncpy(msg.dest_name, temp_, NAME_LENGTH);

			// set reply message title
			snprintf(temp_, MAIL_TITLE_LENGTH, "RE:%s", msg.title);
			safestrncpy(msg.title, temp_, MAIL_TITLE_LENGTH);

			msg.status = MAIL_NEW;
			msg.timestamp = time(NULL);

			new_mail = mail_savemessage(&msg);
			mapif_Mail_new(&msg);
		}
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOB(fd,10) = (new_mail == 0);
	WFIFOSET(fd,11);
}

static void mapif_parse_Mail_return(int fd)
{
	mapif_Mail_return(fd, RFIFOL(fd,2), RFIFOL(fd,6));
}

/*==========================================
 * Send Mail
 *------------------------------------------*/
static void mapif_Mail_send(int fd, struct mail_message* msg)
{
	int len = sizeof(struct mail_message) + 4;
	
	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x384d;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), msg, sizeof(struct mail_message));
	WFIFOSET(fd,len);
}

static void mapif_parse_Mail_send(int fd)
{
	struct mail_message msg;
	char esc_name[NAME_LENGTH*2+1];
	int account_id = 0;

	if(RFIFOW(fd,2) != 8 + sizeof(struct mail_message))
		return;

	account_id = RFIFOL(fd,4);
	memcpy(&msg, RFIFOP(fd,8), sizeof(struct mail_message));

	// Try to find the Dest Char by Name
	SQL->EscapeStringLen(sql_handle, esc_name, msg.dest_name, strnlen(msg.dest_name, NAME_LENGTH));
	if ( SQL_ERROR == SQL->Query(sql_handle, "SELECT `account_id`, `char_id` FROM `%s` WHERE `name` = '%s'", char_db, esc_name) )
		Sql_ShowDebug(sql_handle);
	else
	if ( SQL_SUCCESS == SQL->NextRow(sql_handle) )
	{
		char *data;
		SQL->GetData(sql_handle, 0, &data, NULL);
		if (atoi(data) != account_id)
		{ // Cannot send mail to char in the same account
			SQL->GetData(sql_handle, 1, &data, NULL);
			msg.dest_id = atoi(data);
		}
	}
	SQL->FreeResult(sql_handle);
	msg.status = MAIL_NEW;

	if( msg.dest_id > 0 )
		msg.id = mail_savemessage(&msg);

	mapif_Mail_send(fd, &msg); // notify sender
	mapif_Mail_new(&msg); // notify recipient
}

void mail_sendmail(int send_id, const char* send_name, int dest_id, const char* dest_name, const char* title, const char* body, int zeny, struct item *item)
{
	struct mail_message msg;
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

	mail_savemessage(&msg);
	mapif_Mail_new(&msg);
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_mail_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3048: mapif_parse_Mail_requestinbox(fd); break;
		case 0x3049: mapif_parse_Mail_read(fd); break;
		case 0x304a: mapif_parse_Mail_getattach(fd); break;
		case 0x304b: mapif_parse_Mail_delete(fd); break;
		case 0x304c: mapif_parse_Mail_return(fd); break;
		case 0x304d: mapif_parse_Mail_send(fd); break;
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
