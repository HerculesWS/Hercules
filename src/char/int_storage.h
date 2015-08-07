// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_STORAGE_H
#define CHAR_INT_STORAGE_H

struct storage_data;
struct guild_storage;

#ifdef HERCULES_CORE
void inter_storage_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_storage interface
 **/
struct inter_storage_interface {
	int (*tosql) (int account_id, struct storage_data* p);
	int (*fromsql) (int account_id, struct storage_data* p);
	int (*guild_storage_tosql) (int guild_id, struct guild_storage* p);
	int (*guild_storage_fromsql) (int guild_id, struct guild_storage* p);
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*delete_) (int account_id);
	int (*guild_storage_delete) (int guild_id);
	int (*parse_frommap) (int fd);
};

struct inter_storage_interface *inter_storage;

#endif /* CHAR_INT_STORAGE_H */
