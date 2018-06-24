/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2018  Hercules Dev Team
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

#include "int_auction.h"

#include "char/char.h"
#include "char/int_mail.h"
#include "char/inter.h"
#include "char/mapif.h"
#include "common/cbasetypes.h"
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

struct inter_auction_interface inter_auction_s;
struct inter_auction_interface *inter_auction;

static int inter_auction_count(int char_id, bool buy)
{
	int i = 0;
	struct auction_data *auction;
	struct DBIterator *iter = db_iterator(inter_auction->db);

	for( auction = dbi_first(iter); dbi_exists(iter); auction = dbi_next(iter) )
	{
		if ((buy && auction->buyer_id == char_id) || (!buy && auction->seller_id == char_id))
			i++;
	}
	dbi_destroy(iter);

	return i;
}

void inter_auction_save(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	struct SqlStmt *stmt;

	if( !auction )
		return;

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "UPDATE `%s` SET `seller_id` = '%d', `seller_name` = ?, `buyer_id` = '%d', `buyer_name` = ?, `price` = '%d', `buynow` = '%d', `hours` = '%d', `timestamp` = '%lu', `nameid` = '%d', `item_name` = ?, `type` = '%d', `refine` = '%d', `attribute` = '%d'",
		auction_db, auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, (unsigned long)auction->timestamp, auction->item.nameid, auction->type, auction->item.refine, auction->item.attribute);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ", `card%d` = '%d'", j, auction->item.card[j]);
	for (j = 0; j < MAX_ITEM_OPTIONS; j++)
		StrBuf->Printf(&buf, ", `opt_idx%d` = '%d', `opt_val%d` = '%d'", j, auction->item.option[j].index, j, auction->item.option[j].value);
	StrBuf->Printf(&buf, " WHERE `auction_id` = '%u'", auction->auction_id);

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if( SQL_SUCCESS != SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 2, SQLDT_STRING, auction->item_name, strnlen(auction->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtExecute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
	}

	SQL->StmtFree(stmt);
	StrBuf->Destroy(&buf);
}

unsigned int inter_auction_create(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	struct SqlStmt *stmt;

	nullpo_ret(auction);

	auction->timestamp = time(NULL) + (auction->hours * 3600);

	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "INSERT INTO `%s` (`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`,`unique_id`", auction_db);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ",`card%d`", j);
	for (j = 0; j < MAX_ITEM_OPTIONS; j++)
		StrBuf->Printf(&buf, ", `opt_idx%d`, `opt_val%d`", j, j);
	StrBuf->Printf(&buf, ") VALUES ('%d',?,'%d',?,'%d','%d','%d','%lu','%d',?,'%d','%d','%d','%"PRIu64"'",
		auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, (unsigned long)auction->timestamp, auction->item.nameid, auction->type, auction->item.refine, auction->item.attribute, auction->item.unique_id);
	for (j = 0; j < MAX_SLOTS; j++)
		StrBuf->Printf(&buf, ",'%d'", auction->item.card[j]);
	for (j = 0; j < MAX_ITEM_OPTIONS; j++)
		StrBuf->Printf(&buf, ",'%d','%d'", auction->item.option[j].index, auction->item.option[j].value);

	StrBuf->AppendStr(&buf, ")");

	stmt = SQL->StmtMalloc(inter->sql_handle);
	if (SQL_SUCCESS != SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 2, SQLDT_STRING, auction->item_name, strnlen(auction->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SQL->StmtExecute(stmt))
	{
		SqlStmt_ShowDebug(stmt);
		auction->auction_id = 0;
	} else {
		struct auction_data *auction_;
		int64 tick = (int64)auction->hours * 3600000;

		auction->item.amount = 1;
		auction->item.identify = 1;
		auction->item.expire_time = 0;

		auction->auction_id = (unsigned int)SQL->StmtLastInsertId(stmt);
		auction->auction_end_timer = timer->add( timer->gettick() + tick , inter_auction->end_timer, auction->auction_id, 0);
		ShowInfo("New Auction %u | time left %"PRId64" ms | By %s.\n", auction->auction_id, tick, auction->seller_name);

		CREATE(auction_, struct auction_data, 1);
		memcpy(auction_, auction, sizeof(struct auction_data));
		idb_put(inter_auction->db, auction_->auction_id, auction_);
	}

	SQL->StmtFree(stmt);
	StrBuf->Destroy(&buf);

	return auction->auction_id;
}

static int inter_auction_end_timer(int tid, int64 tick, int id, intptr_t data) {
	struct auction_data *auction;
	if( (auction = (struct auction_data *)idb_get(inter_auction->db, id)) != NULL )
	{
		if( auction->buyer_id )
		{
			inter_mail->sendmail(0, "Auction Manager", auction->buyer_id, auction->buyer_name, "Auction", "Thanks, you won the auction!.", 0, &auction->item);
			mapif->auction_message(auction->buyer_id, 6); // You have won the auction
			inter_mail->sendmail(0, "Auction Manager", auction->seller_id, auction->seller_name, "Auction", "Payment for your auction!.", auction->price, NULL);
		}
		else
			inter_mail->sendmail(0, "Auction Manager", auction->seller_id, auction->seller_name, "Auction", "No buyers have been found for your auction.", 0, &auction->item);

		ShowInfo("Auction End: id %u.\n", auction->auction_id);

		auction->auction_end_timer = INVALID_TIMER;
		inter_auction->delete_(auction);
	}

	return 0;
}

void inter_auction_delete(struct auction_data *auction)
{
	unsigned int auction_id;
	nullpo_retv(auction);

	auction_id = auction->auction_id;

	if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `auction_id` = '%u'", auction_db, auction_id) )
		Sql_ShowDebug(inter->sql_handle);

	if( auction->auction_end_timer != INVALID_TIMER )
		timer->delete(auction->auction_end_timer, inter_auction->end_timer);

	idb_remove(inter_auction->db, auction_id);
}

void inter_auctions_fromsql(void)
{
	int i;
	struct auction_data *auction;
	char *data;
	StringBuf buf;
	int64 tick = timer->gettick(), endtick;
	time_t now = time(NULL);

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `auction_id`,`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,"
		"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`,`unique_id`");
	for (i = 0; i < MAX_SLOTS; i++)
		StrBuf->Printf(&buf, ",`card%d`", i);
	for (i = 0; i < MAX_ITEM_OPTIONS; i++)
		StrBuf->Printf(&buf, ", `opt_idx%d`, `opt_val%d`", i, i);
	StrBuf->Printf(&buf, " FROM `%s` ORDER BY `auction_id` DESC", auction_db);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf)))
		Sql_ShowDebug(inter->sql_handle);

	StrBuf->Destroy(&buf);

	while (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
		struct item *item;
		CREATE(auction, struct auction_data, 1);
		SQL->GetData(inter->sql_handle, 0, &data, NULL); auction->auction_id = atoi(data);
		SQL->GetData(inter->sql_handle, 1, &data, NULL); auction->seller_id = atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, NULL); safestrncpy(auction->seller_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 3, &data, NULL); auction->buyer_id = atoi(data);
		SQL->GetData(inter->sql_handle, 4, &data, NULL); safestrncpy(auction->buyer_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 5, &data, NULL); auction->price = atoi(data);
		SQL->GetData(inter->sql_handle, 6, &data, NULL); auction->buynow = atoi(data);
		SQL->GetData(inter->sql_handle, 7, &data, NULL); auction->hours = atoi(data);
		SQL->GetData(inter->sql_handle, 8, &data, NULL); auction->timestamp = atoi(data);

		item = &auction->item;
		SQL->GetData(inter->sql_handle, 9, &data, NULL); item->nameid = atoi(data);
		SQL->GetData(inter->sql_handle,10, &data, NULL); safestrncpy(auction->item_name, data, ITEM_NAME_LENGTH);
		SQL->GetData(inter->sql_handle,11, &data, NULL); auction->type = atoi(data);

		SQL->GetData(inter->sql_handle,12, &data, NULL); item->refine = atoi(data);
		SQL->GetData(inter->sql_handle,13, &data, NULL); item->attribute = atoi(data);
		SQL->GetData(inter->sql_handle,14, &data, NULL); item->unique_id = strtoull(data, NULL, 10);

		item->identify = 1;
		item->amount = 1;
		item->expire_time = 0;
		/* Card Slots */
		for (i = 0; i < MAX_SLOTS; i++) {
			SQL->GetData(inter->sql_handle, 15 + i, &data, NULL);
			item->card[i] = atoi(data);
		}
		/* Item Options */
		for (i = 0; i < MAX_ITEM_OPTIONS; i++) {
			SQL->GetData(inter->sql_handle, 15 + MAX_SLOTS + i * 2, &data, NULL);
			item->option[i].index = atoi(data);
			SQL->GetData(inter->sql_handle, 16 + MAX_SLOTS + i * 2, &data, NULL);
			item->option[i].value = atoi(data);
		}

		if (auction->timestamp > now)
			endtick = ((int64)(auction->timestamp - now) * 1000) + tick;
		else
			endtick = tick + 10000; // 10 seconds to process ended auctions

		auction->auction_end_timer = timer->add(endtick, inter_auction->end_timer, auction->auction_id, 0);
		idb_put(inter_auction->db, auction->auction_id, auction);
	}

	SQL->FreeResult(inter->sql_handle);
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_auction_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3050: mapif->parse_auction_requestlist(fd); break;
		case 0x3051: mapif->parse_auction_register(fd); break;
		case 0x3052: mapif->parse_auction_cancel(fd); break;
		case 0x3053: mapif->parse_auction_close(fd); break;
		case 0x3055: mapif->parse_auction_bid(fd); break;
		default:
			return 0;
	}
	return 1;
}

int inter_auction_sql_init(void)
{
	inter_auction->db = idb_alloc(DB_OPT_RELEASE_DATA);
	inter_auction->fromsql();

	return 0;
}

void inter_auction_sql_final(void)
{
	inter_auction->db->destroy(inter_auction->db,NULL);

	return;
}

void inter_auction_defaults(void)
{
	inter_auction = &inter_auction_s;

	inter_auction->db = NULL; // int auction_id -> struct auction_data*

	inter_auction->count = inter_auction_count;
	inter_auction->save = inter_auction_save;
	inter_auction->create = inter_auction_create;
	inter_auction->end_timer = inter_auction_end_timer;
	inter_auction->delete_ = inter_auction_delete;
	inter_auction->fromsql = inter_auctions_fromsql;
	inter_auction->parse_frommap = inter_auction_parse_frommap;
	inter_auction->sql_init = inter_auction_sql_init;
	inter_auction->sql_final = inter_auction_sql_final;
}
