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
#ifndef MAP_HOMUNCULUS_H
#define MAP_HOMUNCULUS_H

#include "map/status.h" // struct status_data, struct status_change
#include "map/unit.h" // struct unit_data
#include "common/hercules.h"
#include "common/mmo.h"

struct map_session_data;

/// Homunuculus IDs
enum homun_id {
	HOMID_LIF           = 6001, ///< Lif
	HOMID_AMISTR        = 6002, ///< Amistr
	HOMID_FILIR         = 6003, ///< Filir
	HOMID_VANILMIRTH    = 6004, ///< Vanilmirth
	HOMID_LIF2          = 6005, ///< Lif (Alternate)
	HOMID_AMISTR2       = 6006, ///< Amistr (Alternate)
	HOMID_FILIR2        = 6007, ///< Filir (Alternate)
	HOMID_VANILMIRTH2   = 6008, ///< Vanilmirth (Alternate)
	HOMID_LIF_E         = 6009, ///< Lif (Evolved)
	HOMID_AMISTR_E      = 6010, ///< Amistr (Evolved)
	HOMID_FILIR_E       = 6011, ///< Filir (Evolved)
	HOMID_VANILMIRTH_E  = 6012, ///< Vanilmirth (Evolved)
	HOMID_LIF2_E        = 6013, ///< Lif (Alternate, Evolved)
	HOMID_AMISTR2_E     = 6014, ///< Amistr (Alternate, Evolved)
	HOMID_FILIR2_E      = 6015, ///< Filir (Alternate, Evolved)
	HOMID_VANILMIRTH2_E = 6016, ///< Vanilmirth (Alternate, Evolved)

	HOMID_EIRA          = 6048, ///< Eira
	HOMID_BAYERI        = 6049, ///< Bayeri
	HOMID_SERA          = 6050, ///< Sera
	HOMID_DIETR         = 6051, ///< Dietr
	HOMID_ELEANOR       = 6052, ///< Eleanor
};

#define MAX_HOMUNCULUS_CLASS 52 // [orn] Increased to 60 from 16 to allow new Homun-S.
#define HM_CLASS_BASE 6001
#define HM_CLASS_MAX (HM_CLASS_BASE+MAX_HOMUNCULUS_CLASS-1)

#define MAX_HOM_SKILL_REQUIRE 5
#define homdb_checkid(id) ((id) >=  HM_CLASS_BASE && (id) <= HM_CLASS_MAX)
#define homun_alive(x) ((x) && (x)->homunculus.vaporize == HOM_ST_ACTIVE && (x)->battle_status.hp > 0)

#ifdef RENEWAL
#define HOMUN_LEVEL_STATWEIGHT_VALUE 0
#define APPLY_HOMUN_LEVEL_STATWEIGHT() \
	do { \
		hom->str_value = hom->agi_value = \
		hom->vit_value = hom->int_value = \
		hom->dex_value = hom->luk_value = hom->level / 10 - HOMUN_LEVEL_STATWEIGHT_VALUE; \
	} while (false)
#else
#define APPLY_HOMUN_LEVEL_STATWEIGHT() (void)0
#endif

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

struct homun_dbs {
	unsigned int exptable[MAX_LEVEL];
	struct view_data viewdb[MAX_HOMUNCULUS_CLASS];
	struct s_homunculus_db db[MAX_HOMUNCULUS_CLASS];
	struct homun_skill_tree_entry skill_tree[MAX_HOMUNCULUS_CLASS][MAX_SKILL_TREE];
};

/* homunculus.c interface */
struct homunculus_interface {
	struct homun_dbs *dbs;
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
	int (*change_name) (struct map_session_data *sd, const char *name);
	bool (*change_name_ack) (struct map_session_data *sd, const char *name, int flag);
	int (*db_search) (int key,int type);
	bool (*create) (struct map_session_data *sd, const struct s_homunculus *hom);
	void (*init_timers) (struct homun_data * hd);
	bool (*call) (struct map_session_data *sd);
	bool (*recv_data) (int account_id, const struct s_homunculus *sh, int flag);
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

#ifdef HERCULES_CORE
void homunculus_defaults(void);
#endif // HERCULES_CORE

HPShared struct homunculus_interface *homun;

#endif /* MAP_HOMUNCULUS_H */
