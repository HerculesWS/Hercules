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
#ifndef CHAR_INT_STORAGE_H
#define CHAR_INT_STORAGE_H

#include "common/db.h"
#include "common/hercules.h"

struct storage_data;
struct guild_storage;

/**
 * inter_storage interface
 **/
struct inter_storage_interface {
	int (*tosql) (int account_id, const struct storage_data *p);
	int (*fromsql) (int account_id, struct storage_data *p);
	int (*guild_storage_tosql) (int guild_id, const struct guild_storage *p);
	int (*guild_storage_fromsql) (int guild_id, struct guild_storage* p);
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*delete_) (int account_id);
	int (*guild_storage_delete) (int guild_id);
	int (*parse_frommap) (int fd);
};

#ifdef HERCULES_CORE
void inter_storage_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_storage_interface *inter_storage;

#endif /* CHAR_INT_STORAGE_H */
