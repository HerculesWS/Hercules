// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _NPC_H_
#define _NPC_H_

#include "map.h" // struct block_list
#include "status.h" // struct status_change
#include "unit.h" // struct unit_data

struct HPluginData;
struct block_list;
struct npc_data;
struct view_data;

enum npc_parse_options {
	NPO_NONE  = 0x0,
	NPO_ONINIT  = 0x1,
	NPO_TRADER  = 0x2,
};

enum npc_shop_types {
	NST_ZENY,/* default */
	NST_CASH,/* official npc cash shop */
	NST_MARKET,/* official npc market type */
	NST_CUSTOM,
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
	unsigned short nameid;
	unsigned int value;
	unsigned int qty;
};
struct npc_shop_data {
	unsigned char type;/* what am i */
	struct npc_item_list *item;/* list */
	unsigned short items;/* total */
};
struct npc_data {
	struct block_list bl;
	struct unit_data *ud;
	struct view_data *vd;
	unsigned int option;
	struct npc_data *master_nd;
	short class_;
	short speed;
	char name[NAME_LENGTH+1];// display name
	char exname[NAME_LENGTH+1];// unique npc name
	int chat_id;
	int touching_id;
	int64 next_walktime;
	uint8 dir;
	
	unsigned size : 2;

	struct status_data status;
	unsigned short level;
	unsigned short stat_point;

	void* chatdb; // pointer to a npc_parse struct (see npc_chat.c)
	char* path;/* path dir */
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
		} tomb;
	} u;
	/* HPData Support for npc_data */
	struct HPluginData **hdata;
	unsigned int hdatac;
};


#define START_NPC_NUM 110000000

enum actor_classes {
	WARP_CLASS = 45,
	HIDDEN_WARP_CLASS = 139,
	WARP_DEBUG_CLASS = 722,
	FLAG_CLASS = 722,
	INVISIBLE_CLASS = 32767,
};

// Old NPC range
#define MAX_NPC_CLASS 1000
// New NPC range
#define MAX_NPC_CLASS2_START 10000
#define MAX_NPC_CLASS2_END 10070

//Checks if a given id is a valid npc id. [Skotlex]
//Since new npcs are added all the time, the max valid value is the one before the first mob (Scorpion = 1001)
#define npcdb_checkid(id) ( ( (id) >= 46 && (id) <= 125) || (id) == HIDDEN_WARP_CLASS || ( (id) > 400 && (id) < MAX_NPC_CLASS ) || (id) == INVISIBLE_CLASS || ( (id) > MAX_NPC_CLASS2_START && (id) < MAX_NPC_CLASS2_END ) )

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
	DBMap *ev_db; // const char* event_name -> struct event_data*
	DBMap *ev_label_db; // const char* label_name (without leading "::") -> struct linkdb_node**   (key: struct npc_data*; data: struct event_data*)
	DBMap *name_db; // const char* npc_name -> struct npc_data*
	DBMap *path_db;
	struct eri *timer_event_ers; //For the npc timer data. [Skotlex]
	struct npc_data *fake_nd;
	struct npc_src_list *src_files;
	struct unit_data base_ud;
	/* npc trader global data, for ease of transition between the script, cleared on every usage */
	bool trader_ok;
	int trader_funds[2];
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
	int (*enable_sub) (struct block_list *bl, va_list ap);
	int (*enable) (const char *name, int flag);
	struct npc_data* (*name2id) (const char *name);
	int (*event_dequeue) (struct map_session_data *sd);
	DBData (*event_export_create) (DBKey key, va_list args);
	int (*event_export) (struct npc_data *nd, int i);
	int (*event_sub) (struct map_session_data *sd, struct event_data *ev, const char *eventname);
	void (*event_doall_sub) (void *key, void *data, va_list ap);
	int (*event_do) (const char *name);
	int (*event_doall_id) (const char *name, int rid);
	int (*event_doall) (const char *name);
	int (*event_do_clock) (int tid, int64 tick, int id, intptr_t data);
	void (*event_do_oninit) (void);
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
	int (*touch_areanpc2) (struct mob_data *md);
	int (*check_areanpc) (int flag, int16 m, int16 x, int16 y, int16 range);
	struct npc_data* (*checknear) (struct map_session_data *sd, struct block_list *bl);
	int (*globalmessage) (const char *name, const char *mes);
	void (*run_tomb) (struct map_session_data *sd, struct npc_data *nd);
	int (*click) (struct map_session_data *sd, struct npc_data *nd);
	int (*scriptcont) (struct map_session_data *sd, int id, bool closing);
	int (*buysellsel) (struct map_session_data *sd, int id, int type);
	int (*cashshop_buylist) (struct map_session_data *sd, int points, int count, unsigned short *item_list);
	int (*buylist_sub) (struct map_session_data *sd, int n, unsigned short *item_list, struct npc_data *nd);
	int (*cashshop_buy) (struct map_session_data *sd, int nameid, int amount, int points);
	int (*buylist) (struct map_session_data *sd, int n, unsigned short *item_list);
	int (*selllist_sub) (struct map_session_data *sd, int n, unsigned short *item_list, struct npc_data *nd);
	int (*selllist) (struct map_session_data *sd, int n, unsigned short *item_list);
	int (*remove_map) (struct npc_data *nd);
	int (*unload_ev) (DBKey key, DBData *data, va_list ap);
	int (*unload_ev_label) (DBKey key, DBData *data, va_list ap);
	int (*unload_dup_sub) (struct npc_data *nd, va_list args);
	void (*unload_duplicates) (struct npc_data *nd);
	int (*unload) (struct npc_data *nd, bool single);
	void (*clearsrcfile) (void);
	void (*addsrcfile) (const char *name);
	void (*delsrcfile) (const char *name);
	void (*parsename) (struct npc_data *nd, const char *name, const char *start, const char *buffer, const char *filepath);
	int (*parseview) (const char *w4, const char *start, const char *buffer, const char *filepath);
	bool (*viewisid) (const char *viewid);
	struct npc_data* (*add_warp) (char *name, short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y);
	const char* (*parse_warp) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath);
	const char* (*parse_shop) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath);
	void (*convertlabel_db) (struct npc_label_list *label_list, const char *filepath);
	const char* (*skip_script) (const char *start, const char *buffer, const char *filepath);
	const char* (*parse_script) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath, int options);
	const char* (*parse_duplicate) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath);
	int (*duplicate4instance) (struct npc_data *snd, int16 m);
	void (*setcells) (struct npc_data *nd);
	int (*unsetcells_sub) (struct block_list *bl, va_list ap);
	void (*unsetcells) (struct npc_data *nd);
	void (*movenpc) (struct npc_data *nd, int16 x, int16 y);
	void (*setdisplayname) (struct npc_data *nd, const char *newname);
	void (*setclass) (struct npc_data *nd, short class_);
	int (*do_atcmd_event) (struct map_session_data *sd, const char *command, const char *message, const char *eventname);
	const char* (*parse_function) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath);
	void (*parse_mob2) (struct spawn_data *mobspawn);
	const char* (*parse_mob) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath);
	const char* (*parse_mapflag) (char *w1, char *w2, char *w3, char *w4, const char *start, const char *buffer, const char *filepath);
	int (*parsesrcfile) (const char *filepath, bool runOnInit);
	int (*script_event) (struct map_session_data *sd, enum npce_event type);
	void (*read_event_script) (void);
	int (*path_db_clear_sub) (DBKey key, DBData *data, va_list args);
	int (*ev_label_db_clear_sub) (DBKey key, DBData *data, va_list args);
	int (*reload) (void);
	bool (*unloadfile) (const char *filepath);
	void (*do_clear_npc) (void);
	void (*debug_warps_sub) (struct npc_data *nd);
	void (*debug_warps) (void);
	/* */
	void (*trader_count_funds) (struct npc_data *nd, struct map_session_data *sd);
	bool (*trader_pay) (struct npc_data *nd, struct map_session_data *sd, int price, int points);
	void (*trader_update) (int master);
	int (*market_buylist) (struct map_session_data* sd, unsigned short list_size, struct packet_npc_market_purchase *p);
	bool (*trader_open) (struct map_session_data *sd, struct npc_data *nd);
	void (*market_fromsql) (void);
	void (*market_tosql) (struct npc_data *nd, unsigned short index);
	void (*market_delfromsql) (struct npc_data *nd, unsigned short index);
	void (*market_delfromsql_sub) (const char *npcname, unsigned short index);
	/**
	 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
	 **/
	int (*secure_timeout_timer) (int tid, int64 tick, int id, intptr_t data);
};

struct npc_interface *npc;

void npc_defaults(void);


/* comes from npc_chat.c */
#ifdef PCRE_SUPPORT
#include "../../3rdparty/pcre/include/pcre.h"
/* Structure containing all info associated with a single pattern block */
struct pcrematch_entry {
	struct pcrematch_entry* next;
	char* pattern;
	pcre* pcre_;
	pcre_extra* pcre_extra_;
	char* label;
};

/* A set of patterns that can be activated and deactived with a single command */
struct pcrematch_set {
	struct pcrematch_set* prev;
	struct pcrematch_set* next;
	struct pcrematch_entry* head;
	int setid;
};

/*
 * Entire data structure hung off a NPC
 *
 * The reason I have done it this way (a void * in npc_data and then
 * this) was to reduce the number of patches that needed to be applied
 * to a ragnarok distribution to bring this code online.  I
 * also wanted people to be able to grab this one file to get updates
 * without having to do a large number of changes.
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

struct npc_chat_interface *npc_chat;

void npc_chat_defaults(void);
#endif

#endif /* _NPC_H_ */
