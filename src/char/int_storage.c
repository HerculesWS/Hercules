// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h" // StringBuf
#include "../common/sql.h"
#include "char.h"
#include "inter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define STORAGE_MEMINC	16

/// Save storage data to sql
int storage_tosql(int account_id, struct storage_data* p)
{
	memitemdata_to_sql(p->items, MAX_STORAGE, account_id, TABLE_STORAGE);
	return 0;
}

/// Load storage data to mem
int storage_fromsql(int account_id, struct storage_data* p)
{
	StringBuf buf;
	struct item* item;
	char* data;
	int i;
	int j;

	memset(p, 0, sizeof(struct storage_data)); //clean up memory
	p->storage_amount = 0;

	// storage {`account_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`expire_time`,`bound`,`unique_id`");
 	for( j = 0; j < MAX_SLOTS; ++j )
		StrBuf->Printf(&buf, ",`card%d`", j);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `account_id`='%d' ORDER BY `nameid`", storage_db, account_id);

	if( SQL_ERROR == SQL->Query(sql_handle, StrBuf->Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StrBuf->Destroy(&buf);

	for( i = 0; i < MAX_STORAGE && SQL_SUCCESS == SQL->NextRow(sql_handle); ++i )
	{
		item = &p->items[i];
		SQL->GetData(sql_handle, 0, &data, NULL); item->id = atoi(data);
		SQL->GetData(sql_handle, 1, &data, NULL); item->nameid = atoi(data);
		SQL->GetData(sql_handle, 2, &data, NULL); item->amount = atoi(data);
		SQL->GetData(sql_handle, 3, &data, NULL); item->equip = atoi(data);
		SQL->GetData(sql_handle, 4, &data, NULL); item->identify = atoi(data);
		SQL->GetData(sql_handle, 5, &data, NULL); item->refine = atoi(data);
		SQL->GetData(sql_handle, 6, &data, NULL); item->attribute = atoi(data);
		SQL->GetData(sql_handle, 7, &data, NULL); item->expire_time = (unsigned int)atoi(data);
		SQL->GetData(sql_handle, 8, &data, NULL); item->bound = atoi(data);
		SQL->GetData(sql_handle, 9, &data, NULL); item->unique_id = strtoull(data, NULL, 10);
 		for( j = 0; j < MAX_SLOTS; ++j )
		{
			SQL->GetData(sql_handle, 10+j, &data, NULL); item->card[j] = atoi(data);
		}
	}
	p->storage_amount = i;
	SQL->FreeResult(sql_handle);

	ShowInfo("storage load complete from DB - id: %d (total: %d)\n", account_id, p->storage_amount);
	return 1;
}

/// Save guild_storage data to sql
int guild_storage_tosql(int guild_id, struct guild_storage* p)
{
	memitemdata_to_sql(p->items, MAX_GUILD_STORAGE, guild_id, TABLE_GUILD_STORAGE);
	ShowInfo ("guild storage save to DB - guild: %d\n", guild_id);
	return 0;
}

/// Load guild_storage data to mem
int guild_storage_fromsql(int guild_id, struct guild_storage* p)
{
	StringBuf buf;
	struct item* item;
	char* data;
	int i;
	int j;

	memset(p, 0, sizeof(struct guild_storage)); //clean up memory
	p->storage_amount = 0;
	p->guild_id = guild_id;

	// storage {`guild_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`bound`,`unique_id`");
 	for( j = 0; j < MAX_SLOTS; ++j )
		StrBuf->Printf(&buf, ",`card%d`", j);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `guild_id`='%d' ORDER BY `nameid`", guild_storage_db, guild_id);

	if( SQL_ERROR == SQL->Query(sql_handle, StrBuf->Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StrBuf->Destroy(&buf);

	for( i = 0; i < MAX_GUILD_STORAGE && SQL_SUCCESS == SQL->NextRow(sql_handle); ++i ) {
		item = &p->items[i];
		SQL->GetData(sql_handle, 0, &data, NULL); item->id = atoi(data);
		SQL->GetData(sql_handle, 1, &data, NULL); item->nameid = atoi(data);
		SQL->GetData(sql_handle, 2, &data, NULL); item->amount = atoi(data);
		SQL->GetData(sql_handle, 3, &data, NULL); item->equip = atoi(data);
		SQL->GetData(sql_handle, 4, &data, NULL); item->identify = atoi(data);
		SQL->GetData(sql_handle, 5, &data, NULL); item->refine = atoi(data);
		SQL->GetData(sql_handle, 6, &data, NULL); item->attribute = atoi(data);
		SQL->GetData(sql_handle, 7, &data, NULL); item->bound = atoi(data);
		SQL->GetData(sql_handle, 8, &data, NULL); item->unique_id = strtoull(data, NULL, 10);
 		item->expire_time = 0;

		for( j = 0; j < MAX_SLOTS; ++j ) {
			SQL->GetData(sql_handle, 9+j, &data, NULL); item->card[j] = atoi(data);
		}
	}
	p->storage_amount = i;
	SQL->FreeResult(sql_handle);

	ShowInfo("guild storage load complete from DB - id: %d (total: %d)\n", guild_id, p->storage_amount);
	return 0;
}

//---------------------------------------------------------
// storage data initialize
int inter_storage_sql_init(void)
{
	return 1;
}
// storage data finalize
void inter_storage_sql_final(void)
{
	return;
}

// q?f[^?
int inter_storage_delete(int account_id)
{
	if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `account_id`='%d'", storage_db, account_id) )
		Sql_ShowDebug(sql_handle);
	return 0;
}
int inter_guild_storage_delete(int guild_id)
{
	if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `guild_id`='%d'", guild_storage_db, guild_id) )
		Sql_ShowDebug(sql_handle);
	return 0;
}

//---------------------------------------------------------
// packet from map server

int mapif_load_guild_storage(int fd,int account_id,int guild_id, char flag)
{
	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `guild_id` FROM `%s` WHERE `guild_id`='%d'", guild_db, guild_id) )
		Sql_ShowDebug(sql_handle);
	else if( SQL->NumRows(sql_handle) > 0 )
	{// guild exists
		WFIFOHEAD(fd, sizeof(struct guild_storage)+13);
		WFIFOW(fd,0) = 0x3818;
		WFIFOW(fd,2) = sizeof(struct guild_storage)+13;
		WFIFOL(fd,4) = account_id;
		WFIFOL(fd,8) = guild_id;
		WFIFOB(fd,12) = flag; //1 open storage, 0 don't open
		guild_storage_fromsql(guild_id, (struct guild_storage*)WFIFOP(fd,13));
 		WFIFOSET(fd, WFIFOW(fd,2));
		return 0;
	}
	// guild does not exist
	SQL->FreeResult(sql_handle);
	WFIFOHEAD(fd, 12);
	WFIFOW(fd,0) = 0x3818;
	WFIFOW(fd,2) = 12;
	WFIFOL(fd,4) = account_id;
	WFIFOL(fd,8) = 0;
	WFIFOSET(fd, 12);
	return 0;
}
int mapif_save_guild_storage_ack(int fd,int account_id,int guild_id,int fail)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0)=0x3819;
	WFIFOL(fd,2)=account_id;
	WFIFOL(fd,6)=guild_id;
	WFIFOB(fd,10)=fail;
	WFIFOSET(fd,11);
	return 0;
}

//---------------------------------------------------------
// packet from map server

int mapif_parse_LoadGuildStorage(int fd)
{
	RFIFOHEAD(fd);
	mapif_load_guild_storage(fd,RFIFOL(fd,2),RFIFOL(fd,6),1);
	return 0;
}

int mapif_parse_SaveGuildStorage(int fd)
{
	int guild_id;
	int len;

	RFIFOHEAD(fd);
	guild_id = RFIFOL(fd,8);
	len = RFIFOW(fd,2);

	if( sizeof(struct guild_storage) != len - 12 )
	{
		ShowError("inter storage: data size error %d != %d\n", sizeof(struct guild_storage), len - 12);
	}
	else
	{
		if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `guild_id` FROM `%s` WHERE `guild_id`='%d'", guild_db, guild_id) )
			Sql_ShowDebug(sql_handle);
		else if( SQL->NumRows(sql_handle) > 0 )
		{// guild exists
			SQL->FreeResult(sql_handle);
			guild_storage_tosql(guild_id, (struct guild_storage*)RFIFOP(fd,12));
			mapif_save_guild_storage_ack(fd, RFIFOL(fd,4), guild_id, 0);
			return 0;
		}
		SQL->FreeResult(sql_handle);
	}
	mapif_save_guild_storage_ack(fd, RFIFOL(fd,4), guild_id, 1);
	return 0;
}

int mapif_itembound_ack(int fd, int aid, int guild_id)
{
#ifdef GP_BOUND_ITEMS
	WFIFOHEAD(fd,8);
	WFIFOW(fd,0) = 0x3856;
	WFIFOL(fd,2) = aid;/* the value is not being used, drop? */
	WFIFOW(fd,6) = guild_id;
	WFIFOSET(fd,8);
#endif
	return 0;
}
//------------------------------------------------
//Guild bound items pull for offline characters [Akinari]
//Revised by [Mhalicot]
//------------------------------------------------
int mapif_parse_ItemBoundRetrieve_sub(int fd)
{
#ifdef GP_BOUND_ITEMS
	StringBuf buf;
	SqlStmt* stmt;
	struct item item;
	int j, i=0, s;
	struct item items[MAX_INVENTORY];
	int char_id = RFIFOL(fd,2);
	int aid = RFIFOL(fd,6);
	int guild_id = RFIFOW(fd,10);

	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`");
	for( j = 0; j < MAX_SLOTS; ++j )
		StrBuf->Printf(&buf, ", `card%d`", j);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `char_id`='%d' AND `bound` = '%d'",inventory_db,char_id,IBT_GUILD);

	stmt = SQL->StmtMalloc(sql_handle);
	if( SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_ERROR == SQL->StmtExecute(stmt) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->StmtFree(stmt);
		StrBuf->Destroy(&buf);
		return 1;
	}

	SQL->StmtBindColumn(stmt, 0, SQLDT_INT,       &item.id,          0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_SHORT,     &item.nameid,      0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_SHORT,     &item.amount,      0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 3, SQLDT_USHORT,    &item.equip,       0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 4, SQLDT_CHAR,      &item.identify,    0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 5, SQLDT_CHAR,      &item.refine,      0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 6, SQLDT_CHAR,      &item.attribute,   0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 7, SQLDT_UINT,      &item.expire_time, 0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 8, SQLDT_UCHAR,     &item.bound,       0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 9, SQLDT_UINT64,    &item.unique_id,   0, NULL, NULL);
	for( j = 0; j < MAX_SLOTS; ++j )
		SQL->StmtBindColumn(stmt, 10+j, SQLDT_SHORT, &item.card[j], 0, NULL, NULL);

	while( SQL_SUCCESS == SQL->StmtNextRow(stmt) ) {
		memcpy(&items[i],&item,sizeof(struct item));
		i++;
	}
	SQL->FreeResult(sql_handle);
	
	if(!i) { //No items found - No need to continue
		StrBuf->Destroy(&buf);
		SQL->StmtFree(stmt);
		return 0;
	}

	//First we delete the character's items
	StrBuf->Clear(&buf);
	StrBuf->Printf(&buf, "DELETE FROM `%s` WHERE",inventory_db);
	for(j=0; j<i; j++) {
		if( j )
			StrBuf->AppendStr(&buf, " OR");

		StrBuf->Printf(&buf, " `id`=%d",items[j].id);
	}

	if( SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_ERROR == SQL->StmtExecute(stmt) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->StmtFree(stmt);
		StrBuf->Destroy(&buf);
		return 1;
	}

	//Now let's update the guild storage with those deleted items
	StrBuf->Clear(&buf);
	StrBuf->Printf(&buf, "INSERT INTO `%s` (`guild_id`, `nameid`, `amount`, `identify`, `refine`, `attribute`, `expire_time`, `bound`, `unique_id`", guild_storage_db);
	for( j = 0; j < MAX_SLOTS; ++j )
		StrBuf->Printf(&buf, ", `card%d`", j);
	StrBuf->AppendStr(&buf, ") VALUES ");
	
	for( j = 0; j < i; ++j ) {
		if( j )
			StrBuf->AppendStr(&buf, ",");

		StrBuf->Printf(&buf, "('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d'",
			guild_id, items[j].nameid, items[j].amount, items[j].identify, items[j].refine, items[j].attribute, items[j].expire_time, items[j].bound, items[j].unique_id);
		for( s = 0; s < MAX_SLOTS; ++s )
			StrBuf->Printf(&buf, ", '%d'", items[j].card[s]);
		StrBuf->AppendStr(&buf, ")");
	}

	if( SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))
	||  SQL_ERROR == SQL->StmtExecute(stmt) )
	{
		Sql_ShowDebug(sql_handle);
		SQL->StmtFree(stmt);
		StrBuf->Destroy(&buf);
		return 1;
	}

	StrBuf->Destroy(&buf);
	SQL->StmtFree(stmt);

	//Finally reload storage and tell map we're done
	mapif_load_guild_storage(fd,aid,guild_id,0);
#endif
	return 0;
}
void mapif_parse_ItemBoundRetrieve(int fd) {
	mapif_parse_ItemBoundRetrieve_sub(fd);
	/* tell map server the operation is over and it can unlock the storage */
	mapif_itembound_ack(fd,RFIFOL(fd,6),RFIFOW(fd,10));
}
int inter_storage_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)){
		case 0x3018: mapif_parse_LoadGuildStorage(fd); break;
		case 0x3019: mapif_parse_SaveGuildStorage(fd); break;
#ifdef GP_BOUND_ITEMS
		case 0x3056: mapif_parse_ItemBoundRetrieve(fd); break;
#endif
		default:
			return 0;
	}
	return 1;
}
