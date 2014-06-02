// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_VENDING_H_
#define _MAP_VENDING_H_

#include "../common/cbasetypes.h"
#include "../common/db.h"

struct map_session_data;
struct s_search_store_search;

struct s_vending {
	short index; //cart index (return item data)
	short amount; //amount of the item for vending
	unsigned int value; //at which price
};

struct vending_interface {
	unsigned int next_id;/* next vender id */
	DBMap *db;
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	void (*close) (struct map_session_data* sd);
	void (*open) (struct map_session_data* sd, const char* message, const uint8* data, int count);
	void (*list) (struct map_session_data* sd, unsigned int id);
	void (*purchase) (struct map_session_data* sd, int aid, unsigned int uid, const uint8* data, int count);
	bool (*search) (struct map_session_data* sd, unsigned short nameid);
	bool (*searchall) (struct map_session_data* sd, const struct s_search_store_search* s);
};

struct vending_interface *vending;

void vending_defaults(void);

#endif /* _MAP_VENDING_H_ */
