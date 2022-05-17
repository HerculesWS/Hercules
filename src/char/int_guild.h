/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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
#ifndef CHAR_INT_GUILD_H
#define CHAR_INT_GUILD_H

#include "common/db.h"
#include "common/mmo.h"

enum guild_save_types {
	GS_BASIC = 0x0001,
	GS_MEMBER = 0x0002,
	GS_POSITION = 0x0004,
	GS_ALLIANCE = 0x0008,
	GS_EXPULSION = 0x0010,
	GS_SKILL = 0x0020,
	GS_EMBLEM = 0x0040,
	GS_CONNECT = 0x0080,
	GS_LEVEL = 0x0100,
	GS_MES = 0x0200,
	GS_MASK = 0x03FF,
	GS_BASIC_MASK = (GS_BASIC | GS_EMBLEM | GS_CONNECT | GS_LEVEL | GS_MES),
	GS_REMOVE = 0x8000,
};

/**
 * inter_guild interface
 **/
struct inter_guild_interface {
	struct DBMap *guild_db; // int guild_id -> struct guild*
	struct DBMap *castle_db;
	unsigned int exp[MAX_GUILDLEVEL];

	int (*save_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*removemember_tosql) (int account_id, int char_id);
	bool (*tosql) (struct guild *g, int flag);
	struct guild* (*fromsql) (int guild_id);
	int (*castle_tosql) (struct guild_castle *gc);
	struct guild_castle* (*castle_fromsql) (int castle_id);
	bool (*exp_parse_row) (char* split[], int column, int current);
	int (*CharOnline) (int char_id, int guild_id);
	int (*CharOffline) (int char_id, int guild_id);
	int (*sql_init) (void);
	int (*db_final) (union DBKey key, struct DBData *data, va_list ap);
	void (*sql_final) (void);
	int (*search_guildname) (const char *str);
	bool (*check_empty) (struct guild *g);
	unsigned int (*nextexp) (int level);
	int (*checkskill) (struct guild *g, int id);
	int (*calcinfo) (struct guild *g);
	int (*sex_changed) (int guild_id, int account_id, int char_id, short gender);
	int (*charname_changed) (int guild_id, int account_id, int char_id, char *name);
	int (*parse_frommap) (int fd);
	int (*broken) (int guild_id);
	struct guild *(*create) (const char *name, const struct guild_member *master);
	bool (*add_member) (int guild_id, const struct guild_member *member);
	bool (*leave) (int guild_id, int account_id, int char_id, int flag, const char *mes);
	bool (*update_member_info_short) (int guild_id, int account_id, int char_id, int online, int lv, int class);
	bool (*update_member_info) (int guild_id, int account_id, int char_id, enum guild_member_info type, const char *data, int len);
	bool (*disband) (int guild_id);
	bool (*update_basic_info) (int guild_id, enum guild_basic_info type, const void *data, int len);
	bool (*update_position) (int guild_id, int idx, const struct guild_position *p);
	bool (*use_skill_point) (int guild_id, uint16 skill_id, int account_id, int max);
	bool (*remove_alliance) (struct guild *g, int guild_id, int account_id1, int account_id2, int flag);
	bool (*change_alliance) (int guild_id1, int guild_id2, int account_id1, int account_id2, int flag);
	bool (*update_notice) (int guild_id, const char *mes1, const char *mes2);
	bool (*update_emblem) (int len, int guild_id, const char *data);
	bool (*update_castle_data) (int castle_id, int index, int value);
	bool (*change_leader) (int guild_id, const char *name, int len);
	bool (*is_guild_master) (int char_id, int guild_id);
	bool (*validate_emblem) (const char *emblem, unsigned long emblem_len, bool is_gif);
	bool (*validate_bmp_emblem) (const char *emblem, unsigned long emblem_len);
	bool (*validate_gif_emblem) (const char *emblem, unsigned long emblem_len);
};

#ifdef HERCULES_CORE
void inter_guild_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_guild_interface *inter_guild;

#endif /* CHAR_INT_GUILD_H */
