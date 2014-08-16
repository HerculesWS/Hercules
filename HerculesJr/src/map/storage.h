// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef MAP_STORAGE_H
#define MAP_STORAGE_H

#include "../common/cbasetypes.h"
#include "../common/db.h"

struct guild_storage;
struct item;
struct map_session_data;

struct storage_interface {
	/* */
	void (*reconnect) (void);
	/* */
	int (*delitem) (struct map_session_data* sd, int n, int amount);
	int (*open) (struct map_session_data *sd);
	int (*add) (struct map_session_data *sd,int index,int amount);
	int (*get) (struct map_session_data *sd,int index,int amount);
	int (*additem) (struct map_session_data* sd, struct item* item_data, int amount);
	int (*addfromcart) (struct map_session_data *sd,int index,int amount);
	int (*gettocart) (struct map_session_data *sd,int index,int amount);
	void (*close) (struct map_session_data *sd);
	void (*pc_quit) (struct map_session_data *sd, int flag);
	int (*comp_item) (const void *i1_, const void *i2_);
	void (*sortitem) (struct item* items, unsigned int size);
	int (*reconnect_sub) (DBKey key, DBData *data, va_list ap);
};
struct storage_interface *storage;

struct guild_storage_interface {
	struct DBMap* db; // int guild_id -> struct guild_storage*
	/* */
	struct guild_storage *(*id2storage) (int guild_id);
	struct guild_storage *(*id2storage2) (int guild_id);
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	int (*delete) (int guild_id);
	int (*open) (struct map_session_data *sd);
	int (*additem) (struct map_session_data *sd,struct guild_storage *stor,struct item *item_data,int amount);
	int (*delitem) (struct map_session_data *sd,struct guild_storage *stor,int n,int amount);
	int (*add) (struct map_session_data *sd,int index,int amount);
	int (*get) (struct map_session_data *sd,int index,int amount);
	int (*addfromcart) (struct map_session_data *sd,int index,int amount);
	int (*gettocart) (struct map_session_data *sd,int index,int amount);
	int (*close) (struct map_session_data *sd);
	int (*pc_quit) (struct map_session_data *sd,int flag);
	int (*save) (int account_id, int guild_id, int flag);
	int (*saved) (int guild_id); //Ack from char server that guild store was saved.
	DBData (*create) (DBKey key, va_list args);
};

struct guild_storage_interface *gstorage;

void storage_defaults(void);
void gstorage_defaults(void);

#endif /* MAP_STORAGE_H */
