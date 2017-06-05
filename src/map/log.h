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
#ifndef MAP_LOG_H
#define MAP_LOG_H

#include "common/hercules.h"

/**
 * Declarations
 **/
struct Sql; // common/sql.h
struct item;
struct item_data;
struct map_session_data;
struct mob_data;

/**
 * Defines
 **/
#ifdef SQL_INNODB
// database is using an InnoDB engine so do not use DELAYED
	#define LOG_QUERY "INSERT"
#else
// database is using a MyISAM engine so use DELAYED
	#define LOG_QUERY "INSERT DELAYED"
#endif

/**
 * Enumerations
 **/
typedef enum e_log_chat_type {
	LOG_CHAT_GLOBAL      = 0x01,
	LOG_CHAT_WHISPER     = 0x02,
	LOG_CHAT_PARTY       = 0x04,
	LOG_CHAT_GUILD       = 0x08,
	LOG_CHAT_MAINCHAT    = 0x10,
	// all
	LOG_CHAT_ALL         = 0xFF,
} e_log_chat_type;

typedef enum e_log_pick_type {
	LOG_TYPE_NONE             = 0x00000000,
	LOG_TYPE_TRADE            = 0x00000001,
	LOG_TYPE_VENDING          = 0x00000002,
	LOG_TYPE_PICKDROP_PLAYER  = 0x00000004,
	LOG_TYPE_PICKDROP_MONSTER = 0x00000008,
	LOG_TYPE_NPC              = 0x00000010,
	LOG_TYPE_SCRIPT           = 0x00000020,
	LOG_TYPE_STEAL            = 0x00000040,
	LOG_TYPE_CONSUME          = 0x00000080,
	LOG_TYPE_PRODUCE          = 0x00000100,
	LOG_TYPE_MVP              = 0x00000200,
	LOG_TYPE_COMMAND          = 0x00000400,
	LOG_TYPE_STORAGE          = 0x00000800,
	LOG_TYPE_GSTORAGE         = 0x00001000,
	LOG_TYPE_MAIL             = 0x00002000,
	LOG_TYPE_AUCTION          = 0x00004000,
	LOG_TYPE_BUYING_STORE     = 0x00008000,
	LOG_TYPE_OTHER            = 0x00010000,
	LOG_TYPE_BANK             = 0x00020000,
	LOG_TYPE_DIVORCE          = 0x00040000,
	LOG_TYPE_ROULETTE         = 0x00080000,
	LOG_TYPE_RENTAL           = 0x00100000,
	LOG_TYPE_CARD             = 0x00200000,
	LOG_TYPE_INV_INVALID      = 0x00400000,
	LOG_TYPE_CART_INVALID     = 0x00800000,
	LOG_TYPE_EGG              = 0x01000000,
	LOG_TYPE_QUEST            = 0x02000000,
	LOG_TYPE_SKILL            = 0x04000000,
	LOG_TYPE_REFINE           = 0x08000000,

	// combinations
	LOG_TYPE_LOOT             = LOG_TYPE_PICKDROP_MONSTER|LOG_TYPE_CONSUME,
	// all
	LOG_TYPE_ALL              = 0xFFFFFFFF,
} e_log_pick_type;

/// filters for item logging
typedef enum e_log_filter {
	LOG_FILTER_NONE     = 0x000,
	LOG_FILTER_ALL      = 0x001,
	// bits
	LOG_FILTER_HEALING  = 0x002,  // Healing items (0)
	LOG_FILTER_ETC_AMMO = 0x004,  // Etc Items(3) + Arrows (10)
	LOG_FILTER_USABLE   = 0x008,  // Usable Items(2) + Scrolls, Lures(11) + Usable Cash Items(18)
	LOG_FILTER_WEAPON   = 0x010,  // Weapons(4)
	LOG_FILTER_ARMOR    = 0x020,  // Shields, Armors, Headgears, Accessories, Garments and Shoes(5)
	LOG_FILTER_CARD     = 0x040,  // Cards(6)
	LOG_FILTER_PETITEM  = 0x080,  // Pet Accessories(8) + Eggs(7) (well, monsters don't drop 'em but we'll use the same system for ALL logs)
	LOG_FILTER_PRICE    = 0x100,  // Log expensive items ( >= price_log )
	LOG_FILTER_AMOUNT   = 0x200,  // Log large amount of items ( >= amount_log )
	LOG_FILTER_REFINE   = 0x400,  // Log refined items ( refine >= refine_log ) [not implemented]
	LOG_FILTER_CHANCE   = 0x800,  // Log rare items and Emperium ( drop chance <= rare_log )
} e_log_filter;

struct log_interface {
	struct {
		e_log_pick_type enable_logs;
		int filter;
		bool sql_logs;
		bool log_chat_woe_disable;
		int rare_items_log,refine_items_log,price_items_log,amount_items_log;
		int zeny, chat;
		bool branch, mvpdrop, commands, npc;
		char log_branch[64], log_pick[64], log_zeny[64], log_mvpdrop[64], log_gm[64], log_npc[64], log_chat[64];
	} config;
	/* */
	char db_ip[32];
	int db_port;
	char db_id[32];
	char db_pw[100];
	char db_name[32];
	struct Sql *mysql_handle;
	/* */
	void (*pick_pc) (struct map_session_data* sd, e_log_pick_type type, int amount, struct item* itm, struct item_data *data);
	void (*pick_mob) (struct mob_data* md, e_log_pick_type type, int amount, struct item* itm, struct item_data *data);
	void (*zeny) (struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount);
	void (*npc) (struct map_session_data* sd, const char *message);
	void (*chat) (e_log_chat_type type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char* dst_charname, const char* message);
	void (*atcommand) (struct map_session_data* sd, const char* message);
	void (*branch) (struct map_session_data* sd);
	void (*mvpdrop) (struct map_session_data* sd, int monster_id, int* log_mvp);

	void (*pick_sub) (int id, int16 m, e_log_pick_type type, int amount, struct item* itm, struct item_data *data);
	void (*zeny_sub) (struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount);
	void (*npc_sub) (struct map_session_data* sd, const char *message);
	void (*chat_sub) (e_log_chat_type type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char* dst_charname, const char* message);
	void (*atcommand_sub) (struct map_session_data* sd, const char* message);
	void (*branch_sub) (struct map_session_data* sd);
	void (*mvpdrop_sub) (struct map_session_data* sd, int monster_id, int* log_mvp);

	bool (*config_read) (const char *filename, bool imported);
	void (*config_done) (void);
	void (*sql_init) (void);
	void (*sql_final) (void);

	char (*picktype2char) (e_log_pick_type type);
	char (*chattype2char) (e_log_chat_type type);
	bool (*should_log_item) (int nameid, int amount, int refine, struct item_data *id);
};

#ifdef HERCULES_CORE
void log_defaults(void);
#endif // HERCULES_CORE

HPShared struct log_interface *logs;

#endif /* MAP_LOG_H */
