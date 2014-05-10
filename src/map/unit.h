// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_UNIT_H_
#define _MAP_UNIT_H_

//#include "map.h"
struct block_list;
struct unit_data;
struct map_session_data;

#include "clif.h"  // clr_type
#include "map.h" // struct block_list
#include "path.h" // struct walkpath_data
#include "skill.h" // struct skill_timerskill, struct skill_unit_group, struct skill_unit_group_tickset

struct unit_data {
	struct block_list *bl;
	struct walkpath_data walkpath;
	struct skill_timerskill *skilltimerskill[MAX_SKILLTIMERSKILL];
	struct skill_unit_group *skillunit[MAX_SKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	short attacktarget_lv;
	short to_x,to_y;
	short skillx,skilly;
	uint16 skill_id,skill_lv;
	int   skilltarget;
	int   skilltimer;
	int   target;
	int   target_to;
	int   attacktimer;
	int   walktimer;
	int   chaserange;
	int64 attackabletime;
	int64 canact_tick;
	int64 canmove_tick;
	uint8 dir;
	unsigned char walk_count;
	unsigned char target_count;
	struct {
		unsigned change_walk_target : 1 ;
		unsigned skillcastcancel : 1 ;
		unsigned attack_continue : 1 ;
		unsigned walk_easy : 1 ;
		unsigned running : 1;
		unsigned speed_changed : 1;
	} state;
};

struct view_data {
#ifdef __64BIT__
	unsigned int class_;
#endif
	unsigned short
#ifndef __64BIT__
		class_,
#endif
		weapon,
		shield, //Or left-hand weapon.
		robe,
		head_top,
		head_mid,
		head_bottom,
		hair_style,
		hair_color,
		cloth_color;
	char sex;
	unsigned dead_sit : 2;
};

extern const short dirx[8];
extern const short diry[8];

struct unit_interface {
	int (*init) (bool minimal);
	int (*final) (void);
	/* */
	struct unit_data* (*bl2ud) (struct block_list *bl);
	struct unit_data* (*bl2ud2) (struct block_list *bl);
	int (*attack_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*walktoxy_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*walktoxy_sub) (struct block_list *bl);
	int (*delay_walktoxy_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*walktoxy) (struct block_list *bl, short x, short y, int flag);
	int (*walktobl_sub) (int tid, int64 tick, int id, intptr_t data);
	int (*walktobl) (struct block_list *bl, struct block_list *tbl, int range, int flag);
	int (*run) (struct block_list *bl);
	int (*wugdash) (struct block_list *bl, struct map_session_data *sd);
	int (*escape) (struct block_list *bl, struct block_list *target, short dist);
	int (*movepos) (struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath);
	int (*setdir) (struct block_list *bl, unsigned char dir);
	uint8 (*getdir) (struct block_list *bl);
	int (*blown) (struct block_list *bl, int dx, int dy, int count, int flag);
	int (*warp) (struct block_list *bl, short m, short x, short y, clr_type type);
	int (*stop_walking) (struct block_list *bl, int type);
	int (*skilluse_id) (struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv);
	int (*is_walking) (struct block_list *bl);
	int (*can_move) (struct block_list *bl);
	int (*resume_running) (int tid, int64 tick, int id, intptr_t data);
	int (*set_walkdelay) (struct block_list *bl, int64 tick, int delay, int type);
	int (*skilluse_id2) (struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel);
	int (*skilluse_pos) (struct block_list *src, short skill_x, short skill_y, uint16 skill_id, uint16 skill_lv);
	int (*skilluse_pos2) (struct block_list *src, short skill_x, short skill_y, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel);
	int (*set_target) (struct unit_data *ud, int target_id);
	int (*stop_attack) (struct block_list *bl);
	int (*unattackable) (struct block_list *bl);
	int (*attack) (struct block_list *src, int target_id, int continuous);
	int (*cancel_combo) (struct block_list *bl);
	bool (*can_reach_pos) (struct block_list *bl, int x, int y, int easy);
	bool (*can_reach_bl) (struct block_list *bl, struct block_list *tbl, int range, int easy, short *x, short *y);
	int (*calc_pos) (struct block_list *bl, int tx, int ty, uint8 dir);
	int (*attack_timer_sub) (struct block_list *src, int tid, int64 tick);
	int (*skillcastcancel) (struct block_list *bl, int type);
	void (*dataset) (struct block_list *bl);
	int (*counttargeted) (struct block_list *bl);
	int (*fixdamage) (struct block_list *src, struct block_list *target, int sdelay, int ddelay, int64 damage, short div, unsigned char type, int64 damage2);
	int (*changeviewsize) (struct block_list *bl, short size);
	int (*remove_map) (struct block_list *bl, clr_type clrtype, const char *file, int line, const char *func);
	void (*remove_map_pc) (struct map_session_data *sd, clr_type clrtype);
	void (*free_pc) (struct map_session_data *sd);
	int (*free) (struct block_list *bl, clr_type clrtype);
};

struct unit_interface *unit;

void unit_defaults(void);

#endif /* _MAP_UNIT_H_ */
