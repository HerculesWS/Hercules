// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_HOMUNCULUS_H_
#define _MAP_HOMUNCULUS_H_

#include "pc.h"
#include "status.h" // struct status_data, struct status_change
#include "unit.h" // struct unit_data
#include "../common/mmo.h"

#define MAX_HOM_SKILL_REQUIRE 5
#define homdb_checkid(id) ((id) >=  HM_CLASS_BASE && (id) <= HM_CLASS_MAX)
#define homun_alive(x) ((x) && (x)->homunculus.vaporize == HOM_ST_ACTIVE && (x)->battle_status.hp > 0)

struct h_stats {
	unsigned int HP, SP;
	unsigned short str, agi, vit, int_, dex, luk;
};

struct s_homunculus_db {
	int base_class, evo_class;
	char name[NAME_LENGTH];
	struct h_stats base, gmin, gmax, emin, emax;
	int foodID ;
	int baseASPD ;
	long hungryDelay ;
	unsigned char element, race, base_size, evo_size;
};

enum {
	HOMUNCULUS_CLASS,
	HOMUNCULUS_FOOD
};

enum {
	MH_MD_FIGHTING = 1,
	MH_MD_GRAPPLING
};

enum {
	SP_ACK      = 0x0,
	SP_INTIMATE = 0x1,
	SP_HUNGRY   = 0x2,
};

enum homun_state {
	HOM_ST_ACTIVE = 0,/* either alive or dead */
	HOM_ST_REST   = 1,/* is resting (vaporized) */
	HOM_ST_MORPH  = 2,/* in morph state */
};

struct homun_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;
	struct s_homunculus_db *homunculusDB; //[orn]
	struct s_homunculus homunculus;       //[orn]

	struct map_session_data *master;      //pointer back to its master
	int hungry_timer;                     //[orn]
	unsigned int exp_next;
	char blockskill[MAX_SKILL];           // [orn]
	
	int64 masterteleport_timer;
};

struct homun_skill_tree_entry {
	short id;
	unsigned char max;
	unsigned char joblv;
	short intimacylv;
	struct {
		short id;
		unsigned char lv;
	} need[MAX_HOM_SKILL_REQUIRE];
}; // Celest

enum homun_type {
	HT_REG,          // Regular Homunculus
	HT_EVO,          // Evolved Homunculus
	HT_S,            // Homunculus S
	HT_INVALID = -1, // Invalid Homunculus
};

/* homunculus.c interface */
struct homunculus_interface {
	unsigned int exptable[MAX_LEVEL];
	struct view_data viewdb[MAX_HOMUNCULUS_CLASS];
	struct s_homunculus_db db[MAX_HOMUNCULUS_CLASS];
	struct homun_skill_tree_entry skill_tree[MAX_HOMUNCULUS_CLASS][MAX_SKILL_TREE];
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	void (*reload) (void);
	void (*reload_skill) (void);
	/* */
	struct view_data* (*get_viewdata) (int class_);
	enum homun_type (*class2type) (int class_);
	void (*damaged) (struct homun_data *hd);
	int (*dead) (struct homun_data *hd);
	int (*vaporize) (struct map_session_data *sd, enum homun_state flag);
	int (*delete) (struct homun_data *hd, int emote);
	int (*checkskill) (struct homun_data *hd, uint16 skill_id);
	int (*calc_skilltree) (struct homun_data *hd, int flag_evolve);
	int (*skill_tree_get_max) (int id, int b_class);
	void (*skillup) (struct homun_data *hd, uint16 skill_id);
	bool (*levelup) (struct homun_data *hd);
	int (*change_class) (struct homun_data *hd, short class_);
	bool (*evolve) (struct homun_data *hd);
	bool (*mutate) (struct homun_data *hd, int homun_id);
	int (*gainexp) (struct homun_data *hd, unsigned int exp);
	unsigned int (*add_intimacy) (struct homun_data * hd, unsigned int value);
	unsigned int (*consume_intimacy) (struct homun_data *hd, unsigned int value);
	void (*healed) (struct homun_data *hd);
	void (*save) (struct homun_data *hd);
	unsigned char (*menu) (struct map_session_data *sd,unsigned char menu_num);
	bool (*feed) (struct map_session_data *sd, struct homun_data *hd);
	int (*hunger_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*hunger_timer_delete) (struct homun_data *hd);
	int (*change_name) (struct map_session_data *sd,char *name);
	bool (*change_name_ack) (struct map_session_data *sd, char* name, int flag);
	int (*db_search) (int key,int type);
	bool (*create) (struct map_session_data *sd, struct s_homunculus *hom);
	void (*init_timers) (struct homun_data * hd);
	bool (*call) (struct map_session_data *sd);
	bool (*recv_data) (int account_id, struct s_homunculus *sh, int flag);
	bool (*creation_request) (struct map_session_data *sd, int class_);
	bool (*ressurect) (struct map_session_data* sd, unsigned char per, short x, short y);
	void (*revive) (struct homun_data *hd, unsigned int hp, unsigned int sp);
	void (*stat_reset) (struct homun_data *hd);
	bool (*shuffle) (struct homun_data *hd);
	bool (*read_db_sub) (char* str[], int columns, int current);
	void (*read_db) (void);
	bool (*read_skill_db_sub) (char* split[], int columns, int current);
	void (*skill_db_read) (void);
	void (*exp_db_read) (void);
	void (*addspiritball) (struct homun_data *hd, int max);
	void (*delspiritball) (struct homun_data *hd, int count, int type);
	int8 (*get_intimacy_grade) (struct homun_data *hd);
};

struct homunculus_interface *homun;

void homunculus_defaults(void);

#endif /* _MAP_HOMUNCULUS_H_ */
