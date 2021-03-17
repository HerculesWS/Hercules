/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017-2021 Hercules Dev Team
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

enum clan_req_state {
	CLAN_REQ_NONE       = 0,
	CLAN_REQ_FIRST      = 1,
	CLAN_REQ_RELOAD     = 2,
	CLAN_REQ_AFTER_KICK = 3,
};

/**
 * clan Interface
 **/
struct clan_interface {
	struct DBMap *db; // int clan_id -> struct clan*

	int max;
	int max_relations;
	int kicktime;
	int checktime;
	int req_timeout;

	void (*init) (bool minimal);
	void (*final) (void);

	bool (*config_read) (bool reload);
	void (*config_read_additional_settings) (struct config_setting_t *settings, const char *source);
	void (*read_db) (struct config_setting_t *settings, const char *source, bool reload);
	int (*read_db_sub) (struct config_setting_t *settings, const char *source, bool reload);
	void (*read_db_additional_fields) (struct clan *entry, struct config_setting_t *t, int n, const char *source);
	void (*read_buffs) (struct clan *c, struct config_setting_t *buff, const char *source);
	struct clan *(*search) (int clan_id);
	struct clan *(*searchname) (const char *name);
	struct map_session_data *(*getonlinesd) (struct clan *c);
	int (*getindex) (const struct clan *c, int char_id);
	bool (*join) (struct map_session_data *sd, int clan_id);
	void (*member_online) (struct map_session_data *sd, bool first);
	bool (*leave) (struct map_session_data *sd, bool first);
	bool (*send_message) (struct map_session_data *sd, const char *mes);
	void (*recv_message) (struct clan *c, const char *mes, int len);
	void (*member_offline) (struct map_session_data *sd);
	void (*set_constants) (void);
	int (*get_id) (const struct block_list *bl);
	void (*buff_start) (struct map_session_data *sd, struct clan *c);
	void (*buff_end) (struct map_session_data *sd, struct clan *c);
	void (*reload) (void);
	int (*rejoin) (struct map_session_data *sd, va_list ap);
	int (*inactivity_kick) (int tid, int64 tick, int id, intptr_t data);
	int (*request_kickoffline) (int tid, int64 tick, int id, intptr_t data);
	int (*request_membercount) (int tid, int64 tick, int id, intptr_t data);
};

#ifdef HERCULES_CORE
void clan_defaults (void);
#endif // HERCULES_CORE

HPShared struct clan_interface *clan;

#endif /* MAP_CLAN_H */
