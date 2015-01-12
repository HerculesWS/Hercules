// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_GUILD_H
#define CHAR_INT_GUILD_H

#include "../common/mmo.h"

enum {
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

#ifdef HERCULES_CORE
void inter_guild_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_guild interface
 **/
struct inter_guild_interface {
	DBMap* guild_db; // int guild_id -> struct guild*
	DBMap* castle_db;
	unsigned int exp[MAX_GUILDLEVEL];

	int (*save_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*removemember_tosql) (int account_id, int char_id);
	int (*tosql) (struct guild *g, int flag);
	struct guild* (*fromsql) (int guild_id);
	int (*castle_tosql) (struct guild_castle *gc);
	struct guild_castle* (*castle_fromsql) (int castle_id);
	bool (*exp_parse_row) (char* split[], int column, int current);
	int (*CharOnline) (int char_id, int guild_id);
	int (*CharOffline) (int char_id, int guild_id);
	int (*sql_init) (void);
	int (*db_final) (DBKey key, DBData *data, va_list ap);
	void (*sql_final) (void);
	int (*search_guildname) (char *str);
	bool (*check_empty) (struct guild *g);
	unsigned int (*nextexp) (int level);
	int (*checkskill) (struct guild *g, int id);
	int (*calcinfo) (struct guild *g);
	int (*sex_changed) (int guild_id, int account_id, int char_id, short gender);
	int (*charname_changed) (int guild_id, int account_id, int char_id, char *name);
	int (*parse_frommap) (int fd);
	int (*leave) (int guild_id, int account_id, int char_id);
	int (*broken) (int guild_id);
};

struct inter_guild_interface *inter_guild;

#endif /* CHAR_INT_GUILD_H */
