// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _PET_H_
#define _PET_H_

#define MAX_PET_DB	300
#define MAX_PETLOOT_SIZE	30

struct s_pet_db {
	short class_;
	char name[NAME_LENGTH],jname[NAME_LENGTH];
	short itemID;
	short EggID;
	short AcceID;
	short FoodID;
	int fullness;
	int hungry_delay;
	int r_hungry;
	int r_full;
	int intimate;
	int die;
	int capture;
	int speed;
	char s_perfor;
	int talk_convert_class;
	int attack_rate;
	int defence_attack_rate;
	int change_target_rate;
	struct script_code *equip_script;
	struct script_code *pet_script;
};

enum { PET_CLASS,PET_CATCH,PET_EGG,PET_EQUIP,PET_FOOD };

struct pet_recovery { //Stat recovery
	enum sc_type type;	//Status Change id
	unsigned short delay; //How long before curing (secs).
	int timer;
};

struct pet_bonus {
	unsigned short type; //bStr, bVit?
	unsigned short val;	//Qty
	unsigned short duration; //in secs
	unsigned short delay;	//Time before RENEWAL_CAST (secs)
	int timer;
};

struct pet_skill_attack { //Attack Skill
	unsigned short id;
	unsigned short lv;
	unsigned short div_; //0 = Normal skill. >0 = Fixed damage (lv), fixed div_.
	unsigned short rate; //Base chance of skill ocurrance (10 = 10% of attacks)
	unsigned short bonusrate; //How being 100% loyal affects cast rate (10 = At 1000 intimacy->rate+10%
};

struct pet_skill_support { //Support Skill
	unsigned short id;
	unsigned short lv;
	unsigned short hp; //Max HP% for skill to trigger (50 -> 50% for Magnificat)
	unsigned short sp; //Max SP% for skill to trigger (100 = no check)
	unsigned short delay; //Time (secs) between being able to recast.
	int timer;
};

struct pet_loot {
	struct item *item;
	unsigned short count;
	unsigned short weight;
	unsigned short max;
};

struct pet_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data vd;
	struct s_pet pet;
	struct status_data status;
	struct mob_db *db;
	struct s_pet_db *petDB;
	int pet_hungry_timer;
	int target_id;
	struct {
		unsigned skillbonus : 1;
	} state;
	int move_fail_count;
	int64 next_walktime, last_thinktime;
	short rate_fix;	//Support rate as modified by intimacy (1000 = 100%) [Skotlex]

	struct pet_recovery* recovery;
	struct pet_bonus* bonus;
	struct pet_skill_attack* a_skill;
	struct pet_skill_support* s_skill;
	struct pet_loot* loot;

	struct map_session_data *msd;
};

#define pet_stop_walking(pd, type) unit->stop_walking(&(pd)->bl, type)
#define pet_stop_attack(pd) unit->stop_attack(&(pd)->bl)

struct pet_interface {
	struct s_pet_db db[MAX_PET_DB];
	struct eri *item_drop_ers; //For loot drops delay structures.
	struct eri *item_drop_list_ers;
	/* */
	int (*init) (void);
	int (*final) (void);
	/* */
	int (*hungry_val) (struct pet_data *pd);
	void (*set_intimate) (struct pet_data *pd, int value);
	int (*create_egg) (struct map_session_data *sd, int item_id);
	int (*unlocktarget) (struct pet_data *pd);
	int (*attackskill) (struct pet_data *pd, int target_id);
	int (*target_check) (struct map_session_data *sd, struct block_list *bl, int type);
	int (*sc_check) (struct map_session_data *sd, int type);
	int (*hungry) (int tid, int64 tick, int id, intptr_t data);
	int (*search_petDB_index) (int key, int type);
	int (*hungry_timer_delete) (struct pet_data *pd);
	int (*performance) (struct map_session_data *sd, struct pet_data *pd);
	int (*return_egg) (struct map_session_data *sd, struct pet_data *pd);
	int (*data_init) (struct map_session_data *sd, struct s_pet *petinfo);
	int (*birth_process) (struct map_session_data *sd, struct s_pet *petinfo);
	int (*recv_petdata) (int account_id, struct s_pet *p, int flag);
	int (*select_egg) (struct map_session_data *sd, short egg_index);
	int (*catch_process1) (struct map_session_data *sd, int target_class);
	int (*catch_process2) (struct map_session_data *sd, int target_id);
	int (*get_egg) (int account_id, int pet_id, int flag);
	int (*unequipitem) (struct map_session_data *sd, struct pet_data *pd);
	int (*food) (struct map_session_data *sd, struct pet_data *pd);
	int (*ai_sub_hard_lootsearch) (struct block_list *bl, va_list ap);
	int (*menu) (struct map_session_data *sd, int menunum);
	int (*change_name) (struct map_session_data *sd, char *name);
	int (*change_name_ack) (struct map_session_data *sd, char *name, int flag);
	int (*equipitem) (struct map_session_data *sd, int index);
	int (*randomwalk) (struct pet_data *pd, int64 tick);
	int (*ai_sub_hard) (struct pet_data *pd, struct map_session_data *sd, int64 tick);
	int (*ai_sub_foreachclient) (struct map_session_data *sd, va_list ap);
	int (*ai_hard) (int tid, int64 tick, int id, intptr_t data);
	int (*delay_item_drop) (int tid, int64 tick, int id, intptr_t data);
	int (*lootitem_drop) (struct pet_data *pd, struct map_session_data *sd);
	int (*skill_bonus_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*recovery_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*heal_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*skill_support_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*read_db) ();
};

struct pet_interface *pet;

void pet_defaults(void);

#endif /* _PET_H_ */
