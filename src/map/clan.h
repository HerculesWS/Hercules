/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017  Hercules Dev Team
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
#ifndef MAP_CLAN_H
#define MAP_CLAN_H

#include "map/map.h"
#include "common/hercules.h"
#include "common/db.h"
#include "common/mmo.h"

struct map_session_data;


struct clan_interface {
	struct DBMap *db; // int clan_id -> struct clan*

	int max;
	int maxalliances;

	void (*init) (bool minimal);
	void (*final) (void);

	bool (*config_read) (bool clear);
	struct clan *(*search) (int clan_id);
	struct clan *(*searchname) (const char *name);
	struct map_session_data *(*getonlinesd) (struct clan *c);
	int (*getindex) (const struct clan *c, int account_id, int char_id);
	bool (*join) (struct map_session_data *sd, int clan_id);
	void (*member_online) (struct map_session_data *sd, bool first);
	bool (*leave) (struct map_session_data *sd, bool first);
	void (*removemember_tosql) (int char_id);
	int (*ally_count) (struct clan *c);
	int (*antagonist_count) (struct clan *c);
	bool (*send_message) (struct map_session_data *sd, const char *mes);
	bool (*recv_message) (struct clan *c, const char *mes, int len);
	int (*getonlinecount) (struct clan *c);
	void (*member_offline) (struct map_session_data *sd);
	void (*constants) (void);
	int (*get_id) (struct block_list *bl);
	void (*buff_start) (struct map_session_data *sd, struct clan *c);
	void (*buff_end) (struct map_session_data *sd, struct clan *c);
	void (*reload) (void);
	int (*rejoin) (struct map_session_data *sd, va_list ap);
	int (*inactivity_kick) (int tid, int64 tick, int id, intptr_t data);
	void (*fix_relationships) (void);
};

#ifdef HERCULES_CORE
void clan_defaults (void);
#endif // HERCULES_CORE

HPShared struct clan_interface *clan;

#endif /* MAP_CLAN_H */
