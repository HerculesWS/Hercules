// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_LOG_H_
#define _MAP_LOG_H_

#include "../common/cbasetypes.h"
#include "../common/sql.h"

/**
 * Declarations
 **/
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
	LOG_TYPE_NONE             = 0,
	LOG_TYPE_TRADE            = 0x00001,
	LOG_TYPE_VENDING          = 0x00002,
	LOG_TYPE_PICKDROP_PLAYER  = 0x00004,
	LOG_TYPE_PICKDROP_MONSTER = 0x00008,
	LOG_TYPE_NPC              = 0x00010,
	LOG_TYPE_SCRIPT           = 0x00020,
	LOG_TYPE_STEAL            = 0x00040,
	LOG_TYPE_CONSUME          = 0x00080,
	LOG_TYPE_PRODUCE          = 0x00100,
	LOG_TYPE_MVP              = 0x00200,
	LOG_TYPE_COMMAND          = 0x00400,
	LOG_TYPE_STORAGE          = 0x00800,
	LOG_TYPE_GSTORAGE         = 0x01000,
	LOG_TYPE_MAIL             = 0x02000,
	LOG_TYPE_AUCTION          = 0x04000,
	LOG_TYPE_BUYING_STORE     = 0x08000,
	LOG_TYPE_OTHER            = 0x10000,
	LOG_TYPE_BANK             = 0x20000,
	// combinations
	LOG_TYPE_LOOT             = LOG_TYPE_PICKDROP_MONSTER|LOG_TYPE_CONSUME,
	// all
	LOG_TYPE_ALL              = 0xFFFFF,
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
		int branch, mvpdrop, zeny, commands, npc, chat;
		char log_branch[64], log_pick[64], log_zeny[64], log_mvpdrop[64], log_gm[64], log_npc[64], log_chat[64];
	} config;
	/* */
	char db_ip[32];
	int db_port;
	char db_id[32];
	char db_pw[32];
	char db_name[32];
	Sql* mysql_handle;
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
	
	int (*config_read) (const char* cfgName);
	void (*config_done) (void);
	void (*sql_init) (void);
	void (*sql_final) (void);
	
	char (*picktype2char) (e_log_pick_type type);
	char (*chattype2char) (e_log_chat_type type);
	bool (*should_log_item) (int nameid, int amount, int refine, struct item_data *id);
};

struct log_interface *logs;

void log_defaults(void);

#endif /* _MAP_LOG_H_ */
