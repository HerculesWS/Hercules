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
#ifndef MAP_MOB_H
#define MAP_MOB_H

#include "map/map.h" // struct block_list
#include "map/status.h" // struct status_data, struct status_change
#include "map/unit.h" // struct unit_data, view_data
#include "common/hercules.h"
#include "common/mmo.h" // struct item

struct hplugin_data_store;

#define MAX_RANDOMMONSTER 5

// Change this to increase the table size in your mob_db to accommodate a larger mob database.
// Be sure to note that IDs 4001 to 4048 are reserved for advanced/baby/expanded classes.
// Notice that the last 1000 entries are used for player clones, so always set this to desired value +1000
#define MAX_MOB_DB 5000

//The number of drops all mobs have and the max drop-slot that the steal skill will attempt to steal from.
#define MAX_MOB_DROP 10
#define MAX_MVP_DROP 3
#define MAX_STEAL_DROP 7

//Min time between AI executions
#define MIN_MOBTHINKTIME 100
//Min time before mobs do a check to call nearby friends for help (or for slaves to support their master)
#define MIN_MOBLINKTIME 1000
//Min time between random walks
#define MIN_RANDOMWALKTIME 4000

//Distance that slaves should keep from their master.
#define MOB_SLAVEDISTANCE 2

// These define the range of available IDs for clones. [Valaris]
#define MOB_CLONE_START (MAX_MOB_DB-999)
#define MOB_CLONE_END MAX_MOB_DB

//Used to determine default enemy type of mobs (for use in each in range calls)
#define DEFAULT_ENEMY_TYPE(md) ((md)->special_state.ai != AI_NONE ?BL_CHAR:BL_MOB|BL_PC|BL_HOM|BL_MER)

#define MAX_MOB_CHAT 250 //Max Skill's messages

// On official servers, monsters will only seek targets that are closer to walk to than their
// search range. The search range is affected depending on if the monster is walking or not.
// On some maps there can be a quite long path for just walking two cells in a direction and
// the client does not support displaying walk paths that are longer than 14 cells, so this
// option reduces position lag in such situation. But doing a complex search for every possible
// target, might be CPU intensive.
// Disable this to make monsters not do any path search when looking for a target (old behavior).
#define ACTIVEPATHSEARCH

//Mob skill states.
enum MobSkillState {
	MSS_ANY = -1,
	MSS_IDLE,
	MSS_WALK,
	MSS_LOOT,
	MSS_DEAD,
	MSS_BERSERK, //Aggressive mob attacking
	MSS_ANGRY,   //Mob retaliating from being attacked.
	MSS_RUSH,    //Mob following a player after being attacked.
	MSS_FOLLOW,  //Mob following a player without being attacked.
	MSS_ANYTARGET,
};

enum MobDamageLogFlag
{
	MDLF_NORMAL = 0,
	MDLF_HOMUN,
	MDLF_PET,
};

enum size {
	SZ_SMALL = 0,
	SZ_MEDIUM,
	SZ_BIG,
};

enum ai {
	AI_NONE = 0, //0: Normal mob.
	AI_ATTACK,   //1: Standard summon, attacks mobs.
	AI_SPHERE,   //2: Alchemist Marine Sphere
	AI_FLORA,    //3: Alchemist Summon Flora
	AI_ZANZOU,   //4: Summon Zanzou

	AI_MAX
};

/**
 * Acceptable values for map_session_data.state.noks
 */
enum ksprotection_mode {
	KSPROTECT_NONE  = 0,
	KSPROTECT_SELF  = 1,
	KSPROTECT_PARTY = 2,
	KSPROTECT_GUILD = 3,
};

struct mob_skill {
	enum MobSkillState state;
	uint16 skill_id,skill_lv;
	short permillage;
	int casttime,delay;
	short cancel;
	short cond1,cond2;
	short target;
	int val[5];
	short emotion;
	unsigned short msg_id;
};

struct mob_chat {
	unsigned short msg_id;
	unsigned int color;
	char msg[CHAT_SIZE_MAX];
};

struct spawn_info {
	unsigned short mapindex;
	unsigned short qty;
};

struct mob_db {
	int mob_id;
	char sprite[NAME_LENGTH],name[NAME_LENGTH],jname[NAME_LENGTH];
	unsigned int base_exp,job_exp;
	unsigned int mexp;
	short range2,range3;
	short race2; // celest
	unsigned short lv;
	struct { int nameid,p; } dropitem[MAX_MOB_DROP];
	struct { int nameid,p; } mvpitem[MAX_MVP_DROP];
	struct status_data status;
	struct view_data vd;
	unsigned int option;
	int summonper[MAX_RANDOMMONSTER];
	int maxskill;
	struct mob_skill skill[MAX_MOBSKILL];
	struct spawn_info spawn[10];
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};

struct mob_data {
	struct block_list bl;
	struct unit_data  ud;
	struct view_data *vd;
	struct status_data status, *base_status; //Second one is in case of leveling up mobs, or tiny/large mobs.
	struct status_change sc;
	struct mob_db *db; //For quick data access (saves doing mob_db(md->class_) all the time) [Skotlex]
	char name[NAME_LENGTH];
	struct {
		unsigned int size : 2; //Small/Big monsters. @see enum size
		unsigned int ai : 4; //Special AI for summoned monsters. @see enum ai
		unsigned int clone : 1;/* is clone? 1:0 */
	} special_state; //Special mob information that does not needs to be zero'ed on mob respawn.
	struct {
		unsigned int aggressive : 1; //Signals whether the mob AI is in aggressive mode or reactive mode. [Skotlex]
		unsigned int steal_coin_flag : 1;
		unsigned int soul_change_flag : 1; // Celest
		unsigned int alchemist: 1;
		unsigned int spotted: 1;
		unsigned int npc_killmonster: 1; //for new killmonster behavior
		unsigned int rebirth: 1; // NPC_Rebirth used
		unsigned int boss : 1;
		enum MobSkillState skillstate;
		unsigned char steal_flag; //number of steal tries (to prevent steal exploit on mobs with few items) [Lupus]
		unsigned char attacked_count; //For rude attacked.
		int provoke_flag; // Celest
	} state;
	struct guardian_data* guardian_data;
	struct {
		int id;
		unsigned int dmg;
		unsigned int flag : 2; //0: Normal. 1: Homunc exp. 2: Pet exp
	} dmglog[DAMAGELOG_SIZE];
	struct spawn_data *spawn; //Spawn data.
	int spawn_timer; //Required for Convex Mirror
	struct item *lootitem;
	short class_;
	unsigned int tdmg; //Stores total damage given to the mob, for exp calculations. [Skotlex]
	int level;
	int target_id,attacked_id;
	int areanpc_id; //Required in OnTouchNPC (to avoid multiple area touchs)
	unsigned int bg_id; // BattleGround System

	int64 next_walktime, last_thinktime, last_linktime, last_pcneartime, dmgtick;
	short move_fail_count;
	short lootitem_count;
	short min_chase;
	unsigned char walktoxy_fail_count; //Pathfinding succeeds but the actual walking failed (e.g. Icewall lock)

	int deletetimer;
	int master_id,master_dist;

	int8 skill_idx;// key of array
	int64 skilldelay[MAX_MOBSKILL];
	char npc_event[EVENT_NAME_LENGTH];
	/**
	 * Did this monster summon something?
	 * Used to flag summon deletions, saves a worth amount of memory
	 **/
	bool can_summon;
	/**
	 * MvP Tombstone NPC ID
	 **/
	int tomb_nid;
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};


enum {
	MST_TARGET = 0,
	MST_RANDOM, //Random Target!
	MST_SELF,
	MST_FRIEND,
	MST_MASTER,
	MST_AROUND5,
	MST_AROUND6,
	MST_AROUND7,
	MST_AROUND8,
	MST_AROUND1,
	MST_AROUND2,
	MST_AROUND3,
	MST_AROUND4,
	MST_AROUND = MST_AROUND4,

	MSC_ALWAYS = 0x0000,
	MSC_MYHPLTMAXRATE,
	MSC_MYHPINRATE,
	MSC_FRIENDHPLTMAXRATE,
	MSC_FRIENDHPINRATE,
	MSC_MYSTATUSON,
	MSC_MYSTATUSOFF,
	MSC_FRIENDSTATUSON,
	MSC_FRIENDSTATUSOFF,
	MSC_ATTACKPCGT,
	MSC_ATTACKPCGE,
	MSC_SLAVELT,
	MSC_SLAVELE,
	MSC_CLOSEDATTACKED,
	MSC_LONGRANGEATTACKED,
	MSC_AFTERSKILL,
	MSC_SKILLUSED,
	MSC_CASTTARGETED,
	MSC_RUDEATTACKED,
	MSC_MASTERHPLTMAXRATE,
	MSC_MASTERATTACKED,
	MSC_ALCHEMIST,
	MSC_SPAWN,
};

/**
 * Mob IDs
 */
enum mob_id {
	MOBID_PORING           = 1002, ///<           PORING / Poring

	MOBID_HORNET           = 1004, ///<           HORNET / Hornet

	MOBID_RED_PLANT        = 1078, ///<        RED_PLANT / Red Plant
	MOBID_BLUE_PLANT       = 1079, ///<       BLUE_PLANT / Blue Plant
	MOBID_GREEN_PLANT      = 1080, ///<      GREEN_PLANT / Green Plant
	MOBID_YELLOW_PLANT     = 1081, ///<     YELLOW_PLANT / Yellow Plant
	MOBID_WHITE_PLANT      = 1082, ///<      WHITE_PLANT / White Plant
	MOBID_SHINING_PLANT    = 1083, ///<    SHINING_PLANT / Shining Plant
	MOBID_BLACK_MUSHROOM   = 1084, ///<   BLACK_MUSHROOM / Black Mushroom
	MOBID_RED_MUSHROOM     = 1085, ///<     RED_MUSHROOM / Red Mushroom

	MOBID_MARINE_SPHERE    = 1142, ///<    MARINE_SPHERE / Marine Sphere

	MOBID_EMPELIUM         = 1288, ///<         EMPELIUM / Emperium

	MOBID_GIANT_HONET      = 1303, ///<      GIANT_HONET / Giant Hornet

	MOBID_TREASURE_BOX1    = 1324, ///<    TREASURE_BOX1 / Treasure Chest
	MOBID_TREASURE_BOX2    = 1325, ///<    TREASURE_BOX2 / Treasure Chest
	MOBID_TREASURE_BOX3    = 1326, ///<    TREASURE_BOX3 / Treasure Chest
	MOBID_TREASURE_BOX4    = 1327, ///<    TREASURE_BOX4 / Treasure Chest
	MOBID_TREASURE_BOX5    = 1328, ///<    TREASURE_BOX5 / Treasure Chest
	MOBID_TREASURE_BOX6    = 1329, ///<    TREASURE_BOX6 / Treasure Chest
	MOBID_TREASURE_BOX7    = 1330, ///<    TREASURE_BOX7 / Treasure Chest
	MOBID_TREASURE_BOX8    = 1331, ///<    TREASURE_BOX8 / Treasure Chest
	MOBID_TREASURE_BOX9    = 1332, ///<    TREASURE_BOX9 / Treasure Chest
	MOBID_TREASURE_BOX10   = 1333, ///<   TREASURE_BOX10 / Treasure Chest
	MOBID_TREASURE_BOX11   = 1334, ///<   TREASURE_BOX11 / Treasure Chest
	MOBID_TREASURE_BOX12   = 1335, ///<   TREASURE_BOX12 / Treasure Chest
	MOBID_TREASURE_BOX13   = 1336, ///<   TREASURE_BOX13 / Treasure Chest
	MOBID_TREASURE_BOX14   = 1337, ///<   TREASURE_BOX14 / Treasure Chest
	MOBID_TREASURE_BOX15   = 1338, ///<   TREASURE_BOX15 / Treasure Chest
	MOBID_TREASURE_BOX16   = 1339, ///<   TREASURE_BOX16 / Treasure Chest
	MOBID_TREASURE_BOX17   = 1340, ///<   TREASURE_BOX17 / Treasure Chest
	MOBID_TREASURE_BOX18   = 1341, ///<   TREASURE_BOX18 / Treasure Chest
	MOBID_TREASURE_BOX19   = 1342, ///<   TREASURE_BOX19 / Treasure Chest
	MOBID_TREASURE_BOX20   = 1343, ///<   TREASURE_BOX20 / Treasure Chest
	MOBID_TREASURE_BOX21   = 1344, ///<   TREASURE_BOX21 / Treasure Chest
	MOBID_TREASURE_BOX22   = 1345, ///<   TREASURE_BOX22 / Treasure Chest
	MOBID_TREASURE_BOX23   = 1346, ///<   TREASURE_BOX23 / Treasure Chest
	MOBID_TREASURE_BOX24   = 1347, ///<   TREASURE_BOX24 / Treasure Chest
	MOBID_TREASURE_BOX25   = 1348, ///<   TREASURE_BOX25 / Treasure Chest
	MOBID_TREASURE_BOX26   = 1349, ///<   TREASURE_BOX26 / Treasure Chest
	MOBID_TREASURE_BOX27   = 1350, ///<   TREASURE_BOX27 / Treasure Chest
	MOBID_TREASURE_BOX28   = 1351, ///<   TREASURE_BOX28 / Treasure Chest
	MOBID_TREASURE_BOX29   = 1352, ///<   TREASURE_BOX29 / Treasure Chest
	MOBID_TREASURE_BOX30   = 1353, ///<   TREASURE_BOX30 / Treasure Chest
	MOBID_TREASURE_BOX31   = 1354, ///<   TREASURE_BOX31 / Treasure Chest
	MOBID_TREASURE_BOX32   = 1355, ///<   TREASURE_BOX32 / Treasure Chest
	MOBID_TREASURE_BOX33   = 1356, ///<   TREASURE_BOX33 / Treasure Chest
	MOBID_TREASURE_BOX34   = 1357, ///<   TREASURE_BOX34 / Treasure Chest
	MOBID_TREASURE_BOX35   = 1358, ///<   TREASURE_BOX35 / Treasure Chest
	MOBID_TREASURE_BOX36   = 1359, ///<   TREASURE_BOX36 / Treasure Chest
	MOBID_TREASURE_BOX37   = 1360, ///<   TREASURE_BOX37 / Treasure Chest
	MOBID_TREASURE_BOX38   = 1361, ///<   TREASURE_BOX38 / Treasure Chest
	MOBID_TREASURE_BOX39   = 1362, ///<   TREASURE_BOX39 / Treasure Chest
	MOBID_TREASURE_BOX40   = 1363, ///<   TREASURE_BOX40 / Treasure Chest

	MOBID_G_PARASITE       = 1555, ///<       G_PARASITE / Parasite
	MOBID_G_FLORA          = 1575, ///<          G_FLORA / Flora
	MOBID_G_HYDRA          = 1579, ///<          G_HYDRA / Hydra
	MOBID_G_MANDRAGORA     = 1589, ///<     G_MANDRAGORA / Mandragora
	MOBID_G_GEOGRAPHER     = 1590, ///<     G_GEOGRAPHER / Geographer

	MOBID_BARRICADE        = 1905, ///<        BARRICADE / Barricade
	MOBID_BARRICADE_       = 1906, ///<       BARRICADE_ / Barricade
	MOBID_S_EMPEL_1        = 1907, ///<        S_EMPEL_1 / Guardian Stone
	MOBID_S_EMPEL_2        = 1908, ///<        S_EMPEL_2 / Guardian Stone
	MOBID_OBJ_A            = 1909, ///<            OBJ_A / Food Storage
	MOBID_OBJ_B            = 1910, ///<            OBJ_B / Food Depot
	MOBID_OBJ_NEUTRAL      = 1911, ///<      OBJ_NEUTRAL / Neutrality Flag
	MOBID_OBJ_FLAG_A       = 1912, ///<       OBJ_FLAG_A / Lion Flag
	MOBID_OBJ_FLAG_B       = 1913, ///<       OBJ_FLAG_B / Eagle Flag
	MOBID_OBJ_A2           = 1914, ///<           OBJ_A2 / Blue Crystal
	MOBID_OBJ_B2           = 1915, ///<           OBJ_B2 / Pink Crystal

	MOBID_TREASURE_BOX41   = 1938, ///<   TREASURE_BOX41 / Treasure Chest
	MOBID_TREASURE_BOX42   = 1939, ///<   TREASURE_BOX42 / Treasure Chest
	MOBID_TREASURE_BOX43   = 1940, ///<   TREASURE_BOX43 / Treasure Chest
	MOBID_TREASURE_BOX44   = 1941, ///<   TREASURE_BOX44 / Treasure Chest
	MOBID_TREASURE_BOX45   = 1942, ///<   TREASURE_BOX45 / Treasure Chest
	MOBID_TREASURE_BOX46   = 1943, ///<   TREASURE_BOX46 / Treasure Chest
	MOBID_TREASURE_BOX47   = 1944, ///<   TREASURE_BOX47 / Treasure Chest
	MOBID_TREASURE_BOX48   = 1945, ///<   TREASURE_BOX48 / Treasure Chest
	MOBID_TREASURE_BOX49   = 1946, ///<   TREASURE_BOX49 / Treasure Chest

	// Manuk
	MOBID_TATACHO          = 1986, ///<          TATACHO / Tatacho
	MOBID_CENTIPEDE        = 1987, ///<        CENTIPEDE / Centipede
	MOBID_NEPENTHES        = 1988, ///<        NEPENTHES / Nepenthes
	MOBID_HILLSRION        = 1989, ///<        HILLSRION / Hillslion
	MOBID_HARDROCK_MOMMOTH = 1990, ///< HARDROCK_MOMMOTH / Hardrock Mammoth

	// Splendide
	MOBID_TENDRILRION      = 1991, ///<      TENDRILRION / Tenrillion
	MOBID_CORNUS           = 1992, ///<           CORNUS / Cornus
	MOBID_NAGA             = 1993, ///<             NAGA / Naga
	MOBID_LUCIOLA_VESPA    = 1994, ///<    LUCIOLA_VESPA / Luciola Vespa
	MOBID_PINGUICULA       = 1995, ///<       PINGUICULA / Pinguicola

	// Manuk (continue)
	MOBID_G_TATACHO        = 1997, ///<        G_TATACHO / Tatacho
	MOBID_G_HILLSRION      = 1998, ///<      G_HILLSRION / Hillslion
	MOBID_CENTIPEDE_LARVA  = 1999, ///<  CENTIPEDE_LARVA / Centipede Larva

	MOBID_SILVERSNIPER     = 2042, ///<     SILVERSNIPER / Silver Sniper
	MOBID_MAGICDECOY_FIRE  = 2043, ///<  MAGICDECOY_FIRE / Magic Decoy
	MOBID_MAGICDECOY_WATER = 2044, ///< MAGICDECOY_WATER / Magic Decoy
	MOBID_MAGICDECOY_EARTH = 2045, ///< MAGICDECOY_EARTH / Magic Decoy
	MOBID_MAGICDECOY_WIND  = 2046, ///<  MAGICDECOY_WIND / Magic Decoy

	// Mora
	MOBID_POM_SPIDER       = 2132, ///<       POM_SPIDER / Pom Spider
	MOBID_ANGRA_MANTIS     = 2133, ///<     ANGRA_MANTIS / Angra Mantis
	MOBID_PARUS            = 2134, ///<            PARUS / Parus
	//...
	MOBID_LITTLE_FATUM     = 2136, ///<     LITTLE_FATUM / Little Fatum
	MOBID_MIMING           = 2137, ///<           MIMING / Miming

	MOBID_KO_KAGE          = 2308, ///<          KO_KAGE / Zanzou
};

// The data structures for storing delayed item drops
struct item_drop {
	struct item item_data;
	struct item_drop* next;
};
struct item_drop_list {
	int16 m, x, y;                       // coordinates
	int first_charid, second_charid, third_charid; // charid's of players with higher pickup priority
	struct item_drop* item;            // linked list of drops
};


#define mob_stop_walking(md, type) (unit->stop_walking(&(md)->bl, (type)))
#define mob_stop_attack(md)        (unit->stop_attack(&(md)->bl))

#define mob_is_battleground(md) (map->list[(md)->bl.m].flag.battleground && ((md)->class_ == MOBID_BARRICADE_ || ((md)->class_ >= MOBID_OBJ_A && (md)->class_ <= MOBID_OBJ_B2)))
#define mob_is_gvg(md) (map->list[(md)->bl.m].flag.gvg_castle && ( (md)->class_ == MOBID_EMPELIUM || (md)->class_ == MOBID_BARRICADE || (md)->class_ == MOBID_S_EMPEL_1 || (md)->class_ == MOBID_S_EMPEL_2))
#define mob_is_treasure(md) (((md)->class_ >= MOBID_TREASURE_BOX1 && (md)->class_ <= MOBID_TREASURE_BOX40) || ((md)->class_ >= MOBID_TREASURE_BOX41 && (md)->class_ <= MOBID_TREASURE_BOX49))

struct mob_interface {
	// Dynamic mob database, allows saving of memory when there's big gaps in the mob_db [Skotlex]
	struct mob_db *db_data[MAX_MOB_DB + 1];
	struct mob_db *dummy; //Dummy mob to be returned when a non-existant one is requested.
	// Dynamic mob chat database
	struct mob_chat *chat_db[MAX_MOB_CHAT + 1];
	// Defines the Manuk/Splendide/Mora mob groups for the status reductions [Epoque & Frost]
	int manuk[8];
	int splendide[5];
	int mora[5];
	/* */
	int (*init) (bool mimimal);
	int (*final) (void);
	void (*reload) (void);
	/* */
	struct mob_db* (*db) (int index);
	struct mob_chat* (*chat) (short id);
	int (*makedummymobdb) (int);
	int (*spawn_guardian_sub) (int tid, int64 tick, int id, intptr_t data);
	int (*skill_id2skill_idx) (int class_, uint16 skill_id);
	int (*db_searchname) (const char *str);
	int (*db_searchname_array_sub) (struct mob_db *monster, const char *str, int flag);
	// MvP Tomb System
	void (*mvptomb_create) (struct mob_data *md, char *killer, time_t time);
	void (*mvptomb_destroy) (struct mob_data *md);
	int (*db_searchname_array) (struct mob_db **data, int size, const char *str, int flag);
	int (*db_checkid) (const int id);
	struct view_data* (*get_viewdata) (int class_);
	int (*parse_dataset) (struct spawn_data *data);
	struct mob_data* (*spawn_dataset) (struct spawn_data *data);
	int (*get_random_id) (int type, int flag, int lv);
	bool (*ksprotected) (struct block_list *src, struct block_list *target);
	struct mob_data* (*once_spawn_sub) (struct block_list *bl, int16 m, int16 x, int16 y, const char *mobname, int class_, const char *event, unsigned int size, unsigned int ai);
	int (*once_spawn) (struct map_session_data *sd, int16 m, int16 x, int16 y, const char *mobname, int class_, int amount, const char *event, unsigned int size, unsigned int ai);
	int (*once_spawn_area) (struct map_session_data *sd, int16 m, int16 x0, int16 y0, int16 x1, int16 y1, const char *mobname, int class_, int amount, const char *event, unsigned int size, unsigned int ai);
	int (*spawn_guardian) (const char *mapname, short x, short y, const char *mobname, int class_, const char *event, int guardian, bool has_index);
	int (*spawn_bg) (const char *mapname, short x, short y, const char *mobname, int class_, const char *event, unsigned int bg_id);
	int (*can_reach) (struct mob_data *md, struct block_list *bl, int range, int state);
	int (*linksearch) (struct block_list *bl, va_list ap);
	int (*delayspawn) (int tid, int64 tick, int id, intptr_t data);
	int (*setdelayspawn) (struct mob_data *md);
	int (*count_sub) (struct block_list *bl, va_list ap);
	int (*spawn) (struct mob_data *md);
	int (*can_changetarget) (const struct mob_data *md, const struct block_list *target, uint32 mode);
	int (*target) (struct mob_data *md, struct block_list *bl, int dist);
	int (*ai_sub_hard_activesearch) (struct block_list *bl, va_list ap);
	int (*ai_sub_hard_changechase) (struct block_list *bl, va_list ap);
	int (*ai_sub_hard_bg_ally) (struct block_list *bl, va_list ap);
	int (*ai_sub_hard_lootsearch) (struct block_list *bl, va_list ap);
	int (*warpchase_sub) (struct block_list *bl, va_list ap);
	int (*ai_sub_hard_slavemob) (struct mob_data *md, int64 tick);
	int (*unlocktarget) (struct mob_data *md, int64 tick);
	int (*randomwalk) (struct mob_data *md, int64 tick);
	int (*warpchase) (struct mob_data *md, struct block_list *target);
	bool (*ai_sub_hard) (struct mob_data *md, int64 tick);
	int (*ai_sub_hard_timer) (struct block_list *bl, va_list ap);
	int (*ai_sub_foreachclient) (struct map_session_data *sd, va_list ap);
	int (*ai_sub_lazy) (struct mob_data *md, va_list args);
	int (*ai_lazy) (int tid, int64 tick, int id, intptr_t data);
	int (*ai_hard) (int tid, int64 tick, int id, intptr_t data);
	struct item_drop* (*setdropitem) (int nameid, int qty, struct item_data *data);
	struct item_drop* (*setlootitem) (struct item *item);
	int (*delay_item_drop) (int tid, int64 tick, int id, intptr_t data);
	void (*item_drop) (struct mob_data *md, struct item_drop_list *dlist, struct item_drop *ditem, int loot, int drop_rate, unsigned short flag);
	int (*timer_delete) (int tid, int64 tick, int id, intptr_t data);
	int (*deleteslave_sub) (struct block_list *bl, va_list ap);
	int (*deleteslave) (struct mob_data *md);
	int (*respawn) (int tid, int64 tick, int id, intptr_t data);
	void (*log_damage) (struct mob_data *md, struct block_list *src, int damage);
	void (*damage) (struct mob_data *md, struct block_list *src, int damage);
	int (*dead) (struct mob_data *md, struct block_list *src, int type);
	void (*revive) (struct mob_data *md, unsigned int hp);
	int (*guardian_guildchange) (struct mob_data *md);
	int (*random_class) (int *value, size_t count);
	int (*class_change) (struct mob_data *md, int class_);
	void (*heal) (struct mob_data *md, unsigned int heal);
	int (*warpslave_sub) (struct block_list *bl, va_list ap);
	int (*warpslave) (struct block_list *bl, int range);
	int (*countslave_sub) (struct block_list *bl, va_list ap);
	int (*countslave) (struct block_list *bl);
	int (*summonslave) (struct mob_data *md2, int *value, int amount, uint16 skill_id);
	int (*getfriendhprate_sub) (struct block_list *bl, va_list ap);
	struct block_list* (*getfriendhprate) (struct mob_data *md, int min_rate, int max_rate);
	struct block_list* (*getmasterhpltmaxrate) (struct mob_data *md, int rate);
	int (*getfriendstatus_sub) (struct block_list *bl, va_list ap);
	struct mob_data* (*getfriendstatus) (struct mob_data *md, int cond1, int cond2);
	int (*skill_use) (struct mob_data *md, int64 tick, int event);
	int (*skill_event) (struct mob_data *md, struct block_list *src, int64 tick, int flag);
	int (*is_clone) (int class_);
	int (*clone_spawn) (struct map_session_data *sd, int16 m, int16 x, int16 y, const char *event, int master_id, uint32 mode, int flag, unsigned int duration);
	int (*clone_delete) (struct mob_data *md);
	unsigned int (*drop_adjust) (int baserate, int rate_adjust, unsigned short rate_min, unsigned short rate_max);
	void (*item_dropratio_adjust) (int nameid, int mob_id, int *rate_adjust);
	void (*readdb) (void);
	bool (*lookup_const) (const struct config_setting_t *it, const char *name, int *value);
	bool (*get_const) (const struct config_setting_t *it, int *value);
	int (*db_validate_entry) (struct mob_db *entry, int n, const char *source);
	int (*read_libconfig) (const char *filename, bool ignore_missing);
	void (*read_db_additional_fields) (struct mob_db *entry, struct config_setting_t *it, int n, const char *source);
	int (*read_db_sub) (struct config_setting_t *mobt, int id, const char *source);
	void (*read_db_drops_sub) (struct mob_db *entry, struct config_setting_t *t);
	void (*read_db_mvpdrops_sub) (struct mob_db *entry, struct config_setting_t *t);
	uint32 (*read_db_mode_sub) (struct mob_db *entry, struct config_setting_t *t);
	void (*read_db_stats_sub) (struct mob_db *entry, struct config_setting_t *t);
	void (*name_constants) (void);
	bool (*readdb_mobavail) (char *str[], int columns, int current);
	int (*read_randommonster) (void);
	bool (*parse_row_chatdb) (char **str, const char *source, int line, int *last_msg_id);
	void (*readchatdb) (void);
	bool (*parse_row_mobskilldb) (char **str, int columns, int current);
	void (*readskilldb) (void);
	bool (*readdb_race2) (char *fields[], int columns, int current);
	bool (*readdb_itemratio) (char *str[], int columns, int current);
	void (*load) (bool minimal);
	void (*clear_spawninfo) (void);
	void (*destroy_mob_db) (int index);
};

#ifdef HERCULES_CORE
void mob_defaults(void);
#endif // HERCULES_CORE

HPShared struct mob_interface *mob;

#endif /* MAP_MOB_H */
