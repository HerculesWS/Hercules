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
#ifndef MAP_BATTLEGROUND_H
#define MAP_BATTLEGROUND_H

#include "map/map.h" // EVENT_NAME_LENGTH
#include "common/hercules.h"
#include "common/db.h"
#include "common/mmo.h" // struct party

struct hplugin_data_store;
struct block_list;
struct map_session_data;

/**
 * Defines
 **/
#define MAX_BG_MEMBERS 30
#define BG_DELAY_VAR_LENGTH 30

/**
 * Enumerations
 **/
enum bg_queue_types {
	BGQT_INVALID    = 0x0,
	BGQT_INDIVIDUAL = 0x1,
	BGQT_PARTY      = 0x2,
	/* yup no 0x3 */
	BGQT_GUILD      = 0x4,
};

enum bg_team_leave_type {
	BGTL_LEFT = 0x0,
	BGTL_QUIT = 0x1,
	BGTL_AFK  = 0x2,
};

struct battleground_member_data {
	unsigned short x, y;
	struct map_session_data *sd;
	unsigned afk : 1;
	struct point source;/* where did i come from before i join? */
};

struct battleground_data {
	unsigned int bg_id;
	unsigned char count;
	struct battleground_member_data members[MAX_BG_MEMBERS];
	// BG Cementery
	unsigned short mapindex, x, y;
	// Logout Event
	char logout_event[EVENT_NAME_LENGTH];
	char die_event[EVENT_NAME_LENGTH];
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};

struct bg_arena {
	char name[NAME_LENGTH];
	unsigned char id;
	char npc_event[EVENT_NAME_LENGTH];
	short min_level, max_level;
	short prize_win, prize_loss, prize_draw;
	short min_players;
	short max_players;
	short min_team_players;
	char delay_var[NAME_LENGTH];
	unsigned short maxDuration;
	int queue_id;
	int begin_timer;
	int fillup_timer;
	int game_timer;
	unsigned short fillup_duration;
	unsigned short pregame_duration;
	bool ongoing;
	enum bg_queue_types allowed_types;
};

struct battleground_interface {
	bool queue_on;
	/* */
	int mafksec, afk_timer_id;
	char gdelay_var[BG_DELAY_VAR_LENGTH];
	/* */
	struct bg_arena **arena;
	unsigned char arenas;
	/* */
	struct DBMap *team_db; // int bg_id -> struct battleground_data*
	unsigned int team_counter; // Next bg_id
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	struct bg_arena *(*name2arena) (const char *name);
	void (*queue_add) (struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type);
	enum BATTLEGROUNDS_QUEUE_ACK (*can_queue) (struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type);
	int (*id2pos) (int queue_id, int account_id);
	void (*queue_pc_cleanup) (struct map_session_data *sd);
	void (*begin) (struct bg_arena *arena);
	int (*begin_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*queue_pregame) (struct bg_arena *arena);
	int (*fillup_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*queue_ready_ack) (struct bg_arena *arena, struct map_session_data *sd, bool response);
	void (*match_over) (struct bg_arena *arena, bool canceled);
	void (*queue_check) (struct bg_arena *arena);
	struct battleground_data* (*team_search) (int bg_id);
	struct map_session_data* (*getavailablesd) (struct battleground_data *bgd);
	bool (*team_delete) (int bg_id);
	bool (*team_warp) (int bg_id, unsigned short map_index, short x, short y);
	void (*send_dot_remove) (struct map_session_data *sd);
	bool (*team_join) (int bg_id, struct map_session_data *sd);
	int (*team_leave) (struct map_session_data *sd, enum bg_team_leave_type flag);
	bool (*member_respawn) (struct map_session_data *sd);
	int (*create) (unsigned short map_index, short rx, short ry, const char *ev, const char *dev);
	int (*team_get_id) (struct block_list *bl);
	bool (*send_message) (struct map_session_data *sd, const char *mes);
	int (*send_xy_timer_sub) (union DBKey key, struct DBData *data, va_list ap);
	int (*send_xy_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*afk_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*team_db_final) (union DBKey key, struct DBData *data, va_list ap);
	/* */
	enum bg_queue_types (*str2teamtype) (const char *str);
	/* */
	void (*config_read) (void);
};

#ifdef HERCULES_CORE
void battleground_defaults(void);
#endif // HERCULES_CORE

HPShared struct battleground_interface *bg;

#endif /* MAP_BATTLEGROUND_H */
