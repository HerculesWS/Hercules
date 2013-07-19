// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _NPC_H_
#define _NPC_H_

#include "map.h" // struct block_list
#include "status.h" // struct status_change
#include "unit.h" // struct unit_data
struct block_list;
struct npc_data;
struct view_data;

struct unit_data npc_base_ud;

struct npc_timerevent_list {
	int timer,pos;
};
struct npc_label_list {
	char name[NAME_LENGTH];
	int pos;
};
struct npc_item_list {
	unsigned int nameid,value;
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
	unsigned int next_walktime;
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
			unsigned int timertick;
			struct npc_timerevent_list *timer_event;
			int label_list_num;
			struct npc_label_list *label_list;
		} scr;
		struct {
			struct npc_item_list* shop_item;
			int count;
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
};



#define START_NPC_NUM 110000000

enum actor_classes
{
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
#define MAX_NPC_CLASS2_END 10049

//Checks if a given id is a valid npc id. [Skotlex]
//Since new npcs are added all the time, the max valid value is the one before the first mob (Scorpion = 1001)
#define npcdb_checkid(id) ( ( (id) >= 46 && (id) <= 125) || (id) == HIDDEN_WARP_CLASS || ( (id) > 400 && (id) < MAX_NPC_CLASS ) || (id) == INVISIBLE_CLASS || ( (id) > MAX_NPC_CLASS2_START && (id) < MAX_NPC_CLASS2_END ) )

#ifdef PCRE_SUPPORT
void npc_chat_finalize(struct npc_data* nd);
#endif

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
struct view_data* npc_get_viewdata(int class_);
int npc_chat_sub(struct block_list* bl, va_list ap);
int npc_event_dequeue(struct map_session_data* sd);
int npc_event(struct map_session_data* sd, const char* eventname, int ontouch);
int npc_touch_areanpc(struct map_session_data* sd, int16 m, int16 x, int16 y);
int npc_touch_areanpc2(struct mob_data *md); // [Skotlex]
int npc_check_areanpc(int flag, int16 m, int16 x, int16 y, int16 range);
int npc_touchnext_areanpc(struct map_session_data* sd,bool leavemap);
int npc_click(struct map_session_data* sd, struct npc_data* nd);
int npc_scriptcont(struct map_session_data* sd, int id, bool closing);
struct npc_data* npc_checknear(struct map_session_data* sd, struct block_list* bl);
int npc_buysellsel(struct map_session_data* sd, int id, int type);
int npc_buylist(struct map_session_data* sd,int n, unsigned short* item_list);
int npc_selllist(struct map_session_data* sd, int n, unsigned short* item_list);
void npc_parse_mob2(struct spawn_data* mob);
const char* npc_parse_mapflag(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath);
struct npc_data* npc_add_warp(char* name, short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y);
int npc_globalmessage(const char* name,const char* mes);

void npc_setcells(struct npc_data* nd);
void npc_unsetcells(struct npc_data* nd);
void npc_movenpc(struct npc_data* nd, int16 x, int16 y);
int npc_enable(const char* name, int flag);
void npc_setdisplayname(struct npc_data* nd, const char* newname);
void npc_setclass(struct npc_data* nd, short class_);
struct npc_data* npc_name2id(const char* name);
bool npc_isnear(struct block_list * bl);

int npc_get_new_npc_id(void);

void npc_addsrcfile(const char* name);
void npc_delsrcfile(const char* name);
void npc_parsesrcfile(const char* filepath, bool runOnInit);
void do_clear_npc(void);
int do_final_npc(void);
int do_init_npc(void);
void npc_event_do_oninit(void);
int npc_do_ontimer(int npc_id, int option);

int npc_event_do(const char* name);
int npc_event_doall(const char* name);
int npc_event_doall_id(const char* name, int rid);

int npc_timerevent_start(struct npc_data* nd, int rid);
int npc_timerevent_stop(struct npc_data* nd);
void npc_timerevent_quit(struct map_session_data* sd);
int npc_gettimerevent_tick(struct npc_data* nd);
int npc_settimerevent_tick(struct npc_data* nd, int newtimer);
int npc_remove_map(struct npc_data* nd);
void npc_unload_duplicates (struct npc_data* nd);
int npc_unload(struct npc_data* nd, bool single);
int npc_reload(void);
void npc_read_event_script(void);
int npc_script_event(struct map_session_data* sd, enum npce_event type);

int npc_duplicate4instance(struct npc_data *snd, int16 m);
int npc_cashshop_buy(struct map_session_data *sd, int nameid, int amount, int points);

extern struct npc_data* fake_nd;

int npc_cashshop_buylist(struct map_session_data *sd, int points, int count, unsigned short* item_list);

/**
 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
 **/
#ifdef SECURE_NPCTIMEOUT
	int npc_rr_secure_timeout_timer(int tid, unsigned int tick, int id, intptr_t data);
#endif

// @commands (script-based)
int npc_do_atcmd_event(struct map_session_data* sd, const char* command, const char* message, const char* eventname);

bool npc_unloadfile( const char* path );

/* npc.c interface (barely started/WIP) */
struct npc_interface {
	/* */
	struct npc_data *motd;
	/* */
} npc_s;

struct npc_interface *npc;

void npc_defaults(void);

#endif /* _NPC_H_ */
