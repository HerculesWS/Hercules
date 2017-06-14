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
#ifndef MAP_GUILD_H
#define MAP_GUILD_H

#include "map/map.h" // EVENT_NAME_LENGTH
#include "common/hercules.h"
#include "common/db.h"
#include "common/mmo.h"

struct map_session_data;

/**
 * Defines
 **/
#define GUILD_SEND_XY_INVERVAL  5000 // Interval of sending coordinates and HP
#define GUILD_PAYEXP_INVERVAL   10000 //Interval (maximum survival time of the cache, in milliseconds)
#define MAX_GUILD_SKILL_REQUIRE 5

/**
 * Structures
 **/
struct eventlist {
	char name[EVENT_NAME_LENGTH];
	struct eventlist *next;
};

/**
 * Guardian data
 * For quick linking to a guardian's info. [Skotlex]
 **/
struct guardian_data {
	int number; //0-MAX_GUARDIANS-1 = Guardians. MAX_GUARDIANS = Emperium.

	struct guild *g;
	struct guild_castle* castle;
};
struct guild_expcache {
	int guild_id, account_id, char_id;
	uint64 exp;
};
struct s_guild_skill_tree {
	int id;
	int max;
	struct {
		short id;
		short lv;
	} need[MAX_GUILD_SKILL_REQUIRE];
};


struct guild_interface {
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	struct DBMap *db; // int guild_id -> struct guild*
	struct DBMap *castle_db; // int castle_id -> struct guild_castle*
	struct DBMap *expcache_db; // int char_id -> struct guild_expcache*
	struct DBMap *infoevent_db; // int guild_id -> struct eventlist*
	/* */
	struct eri *expcache_ers; //For handling of guild exp payment.
	/* */
	struct s_guild_skill_tree skill_tree[MAX_GUILDSKILL];
	/* guild flags cache */
	struct npc_data **flags;
	unsigned short flags_count;
	/* */
	int (*skill_get_max) (int id);
	/* */
	int (*checkskill) (struct guild *g,int id);
	int (*check_skill_require) (struct guild *g,int id); // [Komurka]
	int (*checkcastles) (struct guild *g); // [MouseJstr]
	bool (*isallied) (int guild_id, int guild_id2); //Checks alliance based on guild Ids. [Skotlex]
	/* */
	struct guild *(*search) (int guild_id);
	struct guild *(*searchname) (char *str);
	struct guild_castle *(*castle_search) (int gcid);
	/* */
	struct guild_castle *(*mapname2gc) (const char* mapname);
	struct guild_castle *(*mapindex2gc) (short map_index);
	/* */
	struct map_session_data *(*getavailablesd) (struct guild *g);
	int (*getindex) (const struct guild *g, int account_id, int char_id);
	int (*getposition) (struct guild *g, struct map_session_data *sd);
	unsigned int (*payexp) (struct map_session_data *sd,unsigned int exp);
	int (*getexp) (struct map_session_data *sd,int exp); // [Celest]
	/* */
	int (*create) (struct map_session_data *sd, const char *name);
	int (*created) (int account_id,int guild_id);
	int (*request_info) (int guild_id);
	int (*recv_noinfo) (int guild_id);
	int (*recv_info) (const struct guild *sg);
	int (*npc_request_info) (int guild_id,const char *ev);
	int (*invite) (struct map_session_data *sd,struct map_session_data *tsd);
	int (*reply_invite) (struct map_session_data *sd,int guild_id,int flag);
	void (*member_joined) (struct map_session_data *sd);
	int (*member_added) (int guild_id,int account_id,int char_id,int flag);
	int (*leave) (struct map_session_data *sd,int guild_id,int account_id,int char_id,const char *mes);
	int (*member_withdraw) (int guild_id,int account_id,int char_id,int flag,const char *name,const char *mes);
	int (*expulsion) (struct map_session_data *sd,int guild_id,int account_id,int char_id,const char *mes);
	int (*skillup) (struct map_session_data* sd, uint16 skill_id);
	void (*block_skill) (struct map_session_data *sd, int time);
	int (*reqalliance) (struct map_session_data *sd,struct map_session_data *tsd);
	int (*reply_reqalliance) (struct map_session_data *sd,int account_id,int flag);
	int (*allianceack) (int guild_id1,int guild_id2,int account_id1,int account_id2,int flag,const char *name1,const char *name2);
	int (*delalliance) (struct map_session_data *sd,int guild_id,int flag);
	int (*opposition) (struct map_session_data *sd,struct map_session_data *tsd);
	int (*check_alliance) (int guild_id1, int guild_id2, int flag);
	/* */
	int (*send_memberinfoshort) (struct map_session_data *sd,int online);
	int (*recv_memberinfoshort) (int guild_id, int account_id, int char_id, int online, int lv, int16 class);
	int (*change_memberposition) (int guild_id,int account_id,int char_id,short idx);
	int (*memberposition_changed) (struct guild *g,int idx,int pos);
	int (*change_position) (int guild_id,int idx,int mode,int exp_mode,const char *name);
	int (*position_changed) (int guild_id, int idx, const struct guild_position *p);
	int (*change_notice) (struct map_session_data *sd,int guild_id,const char *mes1,const char *mes2);
	int (*notice_changed) (int guild_id,const char *mes1,const char *mes2);
	int (*change_emblem) (struct map_session_data *sd,int len,const char *data);
	int (*emblem_changed) (int len,int guild_id,int emblem_id,const char *data);
	int (*send_message) (struct map_session_data *sd, const char *mes);
	int (*recv_message) (int guild_id,int account_id,const char *mes,int len);
	int (*send_dot_remove) (struct map_session_data *sd);
	int (*skillupack) (int guild_id,uint16 skill_id,int account_id);
	int (*dobreak) (struct map_session_data *sd, const char *name);
	int (*broken) (int guild_id,int flag);
	int (*gm_change) (int guild_id, int char_id);
	int (*gm_changed) (int guild_id, int account_id, int char_id);
	/* */
	void (*castle_map_init) (void);
	int (*castledatasave) (int castle_id,int index,int value);
	int (*castledataloadack) (int len, const struct guild_castle *gc);
	void (*castle_reconnect) (int castle_id, int index, int value);
	/* */
	void (*agit_start) (void);
	void (*agit_end) (void);
	void (*agit2_start) (void);
	void (*agit2_end) (void);
	/* guild flag cachin */
	void (*flag_add) (struct npc_data *nd);
	void (*flag_remove) (struct npc_data *nd);
	void (*flags_clear) (void);
	/* guild aura */
	void (*aura_refresh) (struct map_session_data *sd, uint16 skill_id, uint16 skill_lv);
	/* item bound [Mhalicot]*/
	void (*retrieveitembound) (int char_id,int aid,int guild_id);
	/* */
	int (*payexp_timer) (int tid, int64 tick, int id, intptr_t data);
	struct map_session_data *(*sd_check) (int guild_id, int account_id, int char_id);
	bool (*read_guildskill_tree_db) (char* split[], int columns, int current);
	bool (*read_castledb) (char* str[], int columns, int current);
	int (*payexp_timer_sub) (union DBKey key, struct DBData *data, va_list ap);
	int (*send_xy_timer_sub) (union DBKey key, struct DBData *data, va_list ap);
	int (*send_xy_timer) (int tid, int64 tick, int id, intptr_t data);
	struct DBData (*create_expcache) (union DBKey key, va_list args);
	int (*eventlist_db_final) (union DBKey key, struct DBData *data, va_list ap);
	int (*expcache_db_final) (union DBKey key, struct DBData *data, va_list ap);
	int (*castle_db_final) (union DBKey key, struct DBData *data, va_list ap);
	int (*broken_sub) (union DBKey key, struct DBData *data, va_list ap);
	int (*castle_broken_sub) (union DBKey key, struct DBData *data, va_list ap);
	void (*makemember) (struct guild_member *m,struct map_session_data *sd);
	int (*check_member) (const struct guild *g);
	int (*get_alliance_count) (struct guild *g,int flag);
	void (*castle_reconnect_sub) (void *key, void *data, va_list ap);
};

#ifdef HERCULES_CORE
void guild_defaults(void);
#endif // HERCULES_CORE

HPShared struct guild_interface *guild;

#endif /* MAP_GUILD_H */
