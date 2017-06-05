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
#ifndef MAP_VENDING_H
#define MAP_VENDING_H

#include "common/hercules.h"

/* Forward Declarations */
struct DBMap; // common/db.h
struct map_session_data;
struct s_search_store_search;

struct s_vending {
	short index; //cart index (return item data)
	short amount; //amount of the item for vending
	unsigned int value; //at which price
};

struct vending_interface {
	unsigned int next_id;/* next vender id */
	struct DBMap *db;
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

#ifdef HERCULES_CORE
void vending_defaults(void);
#endif // HERCULES_CORE

HPShared struct vending_interface *vending;

#endif /* MAP_VENDING_H */
