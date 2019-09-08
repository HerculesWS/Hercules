/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2018  Hercules Dev Team
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
#ifndef MAP_NPC_H
#define MAP_NPC_H

#include "map/map.h" // struct block_list
#include "map/status.h" // struct status_change
#include "map/unit.h" // struct unit_data
#include "common/hercules.h"
#include "common/db.h"

#include <pcre.h>

/* Forward declarations */
struct hplugin_data_store;
struct itemlist; // map/itemdb.h
struct view_data;

enum market_buy_result;

enum npc_parse_options {
	NPO_NONE  = 0x0,
	NPO_ONINIT  = 0x1,
	NPO_TRADER  = 0x2,
};

enum npc_shop_types {
	NST_ZENY,   /* default */
	NST_CASH,   /* official npc cash shop */
	NST_MARKET, /* official npc market type */
	NST_CUSTOM,
	NST_BARTER, /* official npc barter type */
	/* */
	NST_MAX,
};

struct npc_timerevent_list {
	int timer,pos;
};
struct npc_label_list {
	char name[NAME_LENGTH];
	int pos;
};

struct npc_item_list {
	int nameid;
	unsigned int value;  // price or barter currency item id
	int value2;  // barter currency item amount
	unsigned int qty;
};

struct npc_shop_data {
	unsigned char type;/* what am i */
	struct npc_item_list *item;/* list */
	unsigned int items;/* total */
};
struct npc_parse;
struct npc_data {
	struct block_list bl;
	struct unit_data *ud;
	struct view_data vd;
	unsigned int option;
	struct npc_data *master_nd;
	int class_;
	short speed;
	char name[NAME_LENGTH+1];// display name
	char exname[NAME_LENGTH+1];// unique npc name
	int chat_id;
	int touching_id;
	int64 next_walktime;
	enum unit_dir dir;
	uint8 area_size;

	int clan_id;

	unsigned size : 2;

	struct status_data status;
	unsigned short level;
	unsigned short stat_point;

	struct npc_parse *chatdb;
	const char *path; ///< Source path reference
	enum npc_subtype subtype;
	int src_id;
	union {
		struct {
			struct script_code *script;
			short xs,ys; // OnTouch area radius
			int guild_id;
			int timer,timerid,timeramount,rid;
			int64 timertick;
			struct npc_timerevent_list *timer_event;
			int label_list_num;
			struct npc_label_list *label_list;
			/* */
			struct npc_shop_data *shop;
			bool trader;
		} scr;
		struct { /* TODO duck this as soon as the new shop formatting is deemed stable */
			struct npc_item_list* shop_item;
			unsigned short count;
		} shop;
		struct {
			short xs,ys; // OnTouch area radius
			short x,y; // destination coords
			unsigned short mapindex; // destination map
		} warp;
		struct {
			struct mob_data *md;
			time_t kill_time;
			char killer_name[NAME_LENGTH];
			int spawn_timer;
		} tomb;
	} u;
	VECTOR_DECL(struct questinfo) qi_data;
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};

#define START_NPC_NUM 110000000

enum actor_classes {
	FAKE_NPC = -1,
	WARP_CLASS = 45,
	HIDDEN_WARP_CLASS = 139,
	MOB_TOMB = 565,
	WARP_DEBUG_CLASS = 722,
	FLAG_CLASS = 722,
	INVISIBLE_CLASS = 32767,
};

// Old NPC range
#define MAX_NPC_CLASS 1000
// New NPC range
#define MAX_NPC_CLASS2_START 10001
#define MAX_NPC_CLASS2_END 10344

//Script NPC events.
enum npce_event {
	NPCE_LOGIN,
	NPCE_LOGOUT,
	NPCE_LOADMAP,
	NPCE_BASELVUP,
	NPCE_JOBLVUP,
	NPCE_DIE,
	NPCE_KILLPC,
	NPCE_KILLNPC,
	NPCE_MAX
};

// linked list of npc source files
struct npc_src_list {
	struct npc_src_list* next;
	char name[4]; // dynamic array, the structure is allocated with extra bytes (string length)
};

struct event_data {
	struct npc_data *nd;
	int pos;
};

struct npc_path_data {
	char* path;
	unsigned short references;
};

/* npc.c interface */
struct npc_interface {
	/* */
	struct npc_data *motd;
	struct DBMap *ev_db; // const char* event_name -> struct event_data*
	struct DBMap *ev_label_db; // const char* label_name (without leading "::") -> struct linkdb_node**   (key: struct npc_data*; data: struct event_data*)
	struct DBMap *name_db; // const char* npc_name -> struct npc_data*
	struct DBMap *path_db;
	struct eri *timer_event_ers; //For the npc timer data. [Skotlex]
	struct npc_data *fake_nd;
	struct npc_src_list *src_files;
	struct unit_data base_ud;
	/* npc trader global data, for ease of transition between the script, cleared on every usage */
	bool trader_ok;
	int trader_funds[2];
	int npc_id;
	int npc_warp;
	int npc_shop;
	int npc_script;
	int npc_mob;
	int npc_delay_mob;
	int npc_cache_mob;
	const char *npc_last_path;
	const char *npc_last_ref;
	struct npc_path_data *npc_last_npd;
	/* */
	int (*init) (bool minimal);
	int (*final) (void);
	/* */
	int (*get_new_npc_id) (void);
	struct view_data* (*get_viewdata) (int class_);
	int (*isnear_sub) (struct block_list *bl, va_list args);
	bool (*isnear) (struct block_list *bl);
	int (*ontouch_event) (struct map_session_data *sd, struct npc_data *nd);
	int (*ontouch2_event) (struct map_session_data *sd, struct npc_data *nd);
	int (*onuntouch_event) (struct map_session_data *sd, struct npc_data *nd);
	int (*enable_sub) (struct block_list *bl, va_list ap);
	int (*enable) (const char *name, int flag);
	struct npc_data* (*name2id) (const char *name);
	int (*event_dequeue) (struct map_session_data *sd);
	struct DBData (*event_export_create) (union DBKey key, va_list args);
	int (*event_export) (struct npc_data *nd, int i);
	int (*event_sub) (struct map_session_data *sd, struct event_data *ev, const char *eventname);
	void (*event_doall_sub) (void *key, void *data, va_list ap);
	int (*event_do) (const char *name);
	int (*event_doall_id) (const char *name, int rid);
	int (*event_doall) (const char *name);
	int (*event_do_clock) (int tid, int64 tick, int id, intptr_t data);
	void (*event_do_oninit) ( bool reload );
	int (*timerevent_export) (struct npc_data *nd, int i);
	int (*timerevent) (int tid, int64 tick, int id, intptr_t data);
	int (*timerevent_start) (struct npc_data *nd, int rid);
	int (*timerevent_stop) (struct npc_data *nd);
	void (*timerevent_quit) (struct map_session_data *sd);
	int64 (*gettimerevent_tick) (struct npc_data *nd);
	int (*settimerevent_tick) (struct npc_data *nd, int newtimer);
	int (*event) (struct map_session_data *sd, const char *eventname, int ontouch);
	int (*touch_areanpc_sub) (struct block_list *bl, va_list ap);
	int (*touchnext_areanpc) (struct map_session_data *sd, bool leavemap);
	int (*touch_areanpc) (struct map_session_data *sd, int16 m, int16 x, int16 y);
	int (*untouch_areanpc) (struct map_session_data *sd, int16 m, int16 x, int16 y);
	int (*touch_areanpc2) (struct mob_data *md);
	int (*check_areanpc) (int flag, int16 m, int16 x, int16 y, int16 range);
	struct npc_data* (*checknear) (struct map_session_data *sd, struct block_list *bl);
	int (*globalmessage) (const char *name, const char *mes);
	void (*run_tomb) (struct map_session_data *sd, struct npc_data *nd);
	int (*click) (struct map_session_data *sd, struct npc_data *nd);
	int (*scriptcont) (struct map_session_data *sd, int id, bool closing);
	int (*buysellsel) (struct map_session_data *sd, int id, int type);
	int (*cashshop_buylist) (struct map_session_data *sd, int points, struct itemlist *item_list);
	int (*buylist_sub) (struct map_session_data *sd, struct itemlist *item_list, struct npc_data *nd);
	int (*cashshop_buy) (struct map_session_data *sd, int nameid, int amount, int points);
	int (*buylist) (struct map_session_data *sd, struct itemlist *item_list);
	int (*selllist_sub) (struct map_session_data *sd, struct itemlist *item_list, struct npc_data *nd);
	int (*selllist) (struct map_session_data *sd, struct itemlist *item_list);
	int (*remove_map) (struct npc_data *nd);
	int (*unload_ev) (union DBKey key, struct DBData *data, va_list ap);
	int (*unload_ev_label) (union DBKey key, struct DBData *data, va_list ap);
	int (*unload_dup_sub) (struct npc_data *nd, va_list args);
	void (*unload_duplicates) (struct npc_data *nd);
	int (*unload) (struct npc_data *nd, bool single);
	void (*clearsrcfile) (void);
	void (*addsrcfile) (const char *name);
	void (*delsrcfile) (const char *name);
	const char *(*retainpathreference) (const char *filepath);
	void (*releasepathreference) (const char *filepath);
	void (*parsename) (struct npc_data *nd, const char *name, const char *start, const char *buffer, const char *filepath);
	int (*parseview) (const char *w4, const char *start, const char *buffer, const char *filepath);
	bool (*viewisid) (const char *viewid);
	struct npc_data *(*create_npc) (enum npc_subtype subtype, int m, int x, int y, enum unit_dir dir, int class_);
	struct npc_data* (*add_warp) (char *name, short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y);
	const char *(*parse_warp) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	const char *(*parse_shop) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	const char *(*parse_unknown_object) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	void (*convertlabel_db) (struct npc_label_list *label_list, const char *filepath);
	const char* (*skip_script) (const char *start, const char *buffer, const char *filepath, int *retval);
	const char *(*parse_script) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int options, int *retval);
	void (*add_to_location) (struct npc_data *nd);
	bool (*duplicate_script_sub) (struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options);
	bool (*duplicate_shop_sub) (struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options);
	bool (*duplicate_warp_sub) (struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options);
	bool (*duplicate_sub) (struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options);
	const char *(*parse_duplicate) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int options, int *retval);
	int (*duplicate4instance) (struct npc_data *snd, int16 m);
	void (*setcells) (struct npc_data *nd);
	int (*unsetcells_sub) (struct block_list *bl, va_list ap);
	void (*unsetcells) (struct npc_data *nd);
	void (*movenpc) (struct npc_data *nd, int16 x, int16 y);
	void (*setdisplayname) (struct npc_data *nd, const char *newname);
	void (*setclass) (struct npc_data *nd, int class_);
	int (*do_atcmd_event) (struct map_session_data *sd, const char *command, const char *message, const char *eventname);
	const char *(*parse_function) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	void (*parse_mob2) (struct spawn_data *mobspawn);
	const char *(*parse_mob) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	const char *(*parse_mapflag) (const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	void (*parse_unknown_mapflag) (const char *name, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval);
	int (*parsesrcfile) (const char *filepath, bool runOnInit);
	int (*script_event) (struct map_session_data *sd, enum npce_event type);
	void (*read_event_script) (void);
	int (*path_db_clear_sub) (union DBKey key, struct DBData *data, va_list args);
	int (*ev_label_db_clear_sub) (union DBKey key, struct DBData *data, va_list args);
	int (*reload) (void);
	bool (*unloadfile) (const char *filepath);
	void (*do_clear_npc) (void);
	void (*debug_warps_sub) (struct npc_data *nd);
	void (*debug_warps) (void);
	/* */
	void (*trader_count_funds) (struct npc_data *nd, struct map_session_data *sd);
	bool (*trader_pay) (struct npc_data *nd, struct map_session_data *sd, int price, int points);
	void (*trader_update) (int master);
	enum market_buy_result (*market_buylist) (struct map_session_data *sd, struct itemlist *item_list);
	int (*barter_buylist) (struct map_session_data *sd, struct barteritemlist *item_list);
	bool (*trader_open) (struct map_session_data *sd, struct npc_data *nd);
	void (*market_fromsql) (void);
	void (*market_tosql) (struct npc_data *nd, int index);
	void (*market_delfromsql) (struct npc_data *nd, int index);
	void (*market_delfromsql_sub) (const char *npcname, int index);
	void (*barter_fromsql) (void);
	void (*barter_tosql) (struct npc_data *nd, int index);
	void (*barter_delfromsql) (struct npc_data *nd, int index);
	void (*barter_delfromsql_sub) (const char *npcname, int itemId, int itemId2, int amount2);
	bool (*db_checkid) (const int id);
	void (*refresh) (struct npc_data* nd);
	void (*questinfo_clear) (struct npc_data *nd);
	/**
	 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
	 **/
	int (*secure_timeout_timer) (int tid, int64 tick, int id, intptr_t data);
};

#ifdef HERCULES_CORE
void npc_defaults(void);
#endif // HERCULES_CORE

HPShared struct npc_interface *npc;

/**
 * Structure containing all info associated with a single pattern block
 */
struct pcrematch_entry {
	struct pcrematch_entry* next;
	char* pattern;
	pcre* pcre_;
	pcre_extra* pcre_extra_;
	char* label;
};

/**
 * A set of patterns that can be activated and deactived with a single command
 */
struct pcrematch_set {
	struct pcrematch_set* prev;
	struct pcrematch_set* next;
	struct pcrematch_entry* head;
	int setid;
};

/**
 * Entire data structure hung off a NPC
 */
struct npc_parse {
	struct pcrematch_set* active;
	struct pcrematch_set* inactive;
};

struct npc_chat_interface {
	int (*sub) (struct block_list* bl, va_list ap);
	void (*finalize) (struct npc_data* nd);
	void (*def_pattern) (struct npc_data* nd, int setid, const char* pattern, const char* label);
	struct pcrematch_entry* (*create_pcrematch_entry) (struct pcrematch_set* set);
	void (*delete_pcreset) (struct npc_data* nd, int setid);
	void (*deactivate_pcreset) (struct npc_data* nd, int setid);
	void (*activate_pcreset) (struct npc_data* nd, int setid);
	struct pcrematch_set* (*lookup_pcreset) (struct npc_data* nd, int setid);
	void (*finalize_pcrematch_entry) (struct pcrematch_entry* e);
};

/**
 * pcre interface (libpcre)
 * so that plugins may share and take advantage of the core's pcre
 * should be moved into core/perhaps its own file once hpm is enhanced for login/char
 **/
struct pcre_interface {
	pcre *(*compile) (const char *pattern, int options, const char **errptr, int *erroffset, const unsigned char *tableptr);
	pcre_extra *(*study) (const pcre *code, int options, const char **errptr);
	int (*exec) (const pcre *code, const pcre_extra *extra, PCRE_SPTR subject, int length, int startoffset, int options, int *ovector, int ovecsize);
	void (*free) (void *ptr);
	int (*copy_substring) (const char *subject, int *ovector, int stringcount, int stringnumber, char *buffer, int buffersize);
	void (*free_substring) (const char *stringptr);
	int (*copy_named_substring) (const pcre *code, const char *subject, int *ovector, int stringcount, const char *stringname, char *buffer, int buffersize);
	int (*get_substring) (const char *subject, int *ovector, int stringcount, int stringnumber, const char **stringptr);
};

/**
 * Also defaults libpcre
 **/
#ifdef HERCULES_CORE
void npc_chat_defaults(void);
#endif // HERCULES_CORE

HPShared struct npc_chat_interface *npc_chat;
HPShared struct pcre_interface *libpcre;

#endif /* MAP_NPC_H */
