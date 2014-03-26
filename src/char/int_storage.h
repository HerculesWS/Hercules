// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_INT_STORAGE_H_
#define _CHAR_INT_STORAGE_H_

struct storage_data;
struct guild_storage;

int inter_storage_sql_init(void);
void inter_storage_sql_final(void);
int inter_storage_delete(int account_id);
int inter_guild_storage_delete(int guild_id);

int inter_storage_parse_frommap(int fd);

//Exported for use in the TXT-SQL converter.
int storage_fromsql(int account_id, struct storage_data* p);
int storage_tosql(int account_id,struct storage_data *p);
int guild_storage_tosql(int guild_id, struct guild_storage *p);

#endif /* _CHAR_INT_STORAGE_H_ */
