// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "../common/db.h"
#include "../common/mmo.h" // ITEM_NAME_LENGTH
#include "map.h"

/**
 * Declarations
 **/
struct item_group;
struct item_package;

/**
 * Defines
 **/
#define MAX_ITEMDB 0x8000 // 32k array entries in array (the rest goes to the db)
#define MAX_ITEMDELAYS 10 // The maximum number of item delays
#define MAX_SEARCH 5 //Designed for search functions, species max number of matches to display.
#define MAX_ITEMS_PER_COMBO 6 /* maximum amount of items a combo may require */

#define CARD0_FORGE 0x00FF
#define CARD0_CREATE 0x00FE
#define CARD0_PET ((short)0xFF00)

//Marks if the card0 given is "special" (non-item id used to mark pets/created items. [Skotlex]
#define itemdb_isspecial(i) (i == CARD0_FORGE || i == CARD0_CREATE || i == CARD0_PET)

//Use apple for unknown items.
#define UNKNOWN_ITEM_ID 512

enum item_itemid {
	ITEMID_HOLY_WATER = 523,
	ITEMID_EMPERIUM = 714,
	ITEMID_YELLOW_GEMSTONE = 715,
	ITEMID_RED_GEMSTONE = 716,
	ITEMID_BLUE_GEMSTONE = 717,
	ITEMID_TRAP = 1065,
	ITEMID_FACE_PAINT = 6120,
	ITEMID_STONE = 7049,
	ITEMID_SKULL_ = 7420,
	ITEMID_TOKEN_OF_SIEGFRIED = 7621,
	ITEMID_TRAP_ALLOY = 7940,
	ITEMID_ANCILLA = 12333,
	ITEMID_REINS_OF_MOUNT = 12622,
};

/**
 * Rune Knight
 **/

enum {
	ITEMID_NAUTHIZ = 12725,
	ITEMID_RAIDO,
	ITEMID_BERKANA,
	ITEMID_ISA,
	ITEMID_OTHILA,
	ITEMID_URUZ,
	ITEMID_THURISAZ,
	ITEMID_WYRD,
	ITEMID_HAGALAZ,
	ITEMID_LUX_ANIMA = 22540,
} rune_list;

/**
 * Mechanic
 **/
enum {
	ITEMID_ACCELERATOR = 2800,
	ITEMID_HOVERING_BOOSTER,
	ITEMID_SUICIDAL_DEVICE,
	ITEMID_SHAPE_SHIFTER,
	ITEMID_COOLING_DEVICE,
	ITEMID_MAGNETIC_FIELD_GENERATOR,
	ITEMID_BARRIER_BUILDER,
	ITEMID_REPAIR_KIT,
	ITEMID_CAMOUFLAGE_GENERATOR,
	ITEMID_HIGH_QUALITY_COOLER,
	ITEMID_SPECIAL_COOLER,
	ITEMID_MONKEY_SPANNER = 6186,
} mecha_item_list;

enum {
	NOUSE_SITTING = 0x01,
} item_nouse_list;

//
enum e_chain_cache {
	ECC_ORE,
	/* */
	ECC_MAX,
};

struct item_data {
	uint16 nameid;
	char name[ITEM_NAME_LENGTH],jname[ITEM_NAME_LENGTH];
	
	//Do not add stuff between value_buy and view_id (see how getiteminfo works)
	int value_buy;
	int value_sell;
	int type;
	int maxchance; //For logs, for external game info, for scripts: Max drop chance of this item (e.g. 0.01% , etc.. if it = 0, then monsters don't drop it, -1 denotes items sold in shops only) [Lupus]
	int sex;
	int equip;
	int weight;
	int atk;
	int def;
	int range;
	int slot;
	int look;
	int elv;
	int wlv;
	int view_id;
#ifdef RENEWAL
	int matk;
	int elvmax;/* maximum level for this item */
#endif

	int delay;
//Lupus: I rearranged order of these fields due to compatibility with ITEMINFO script command
//		some script commands should be revised as well...
	unsigned int class_base[3];	//Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
	unsigned class_upper : 6; //Specifies if the upper-type can equip it (bitfield, 0x01: normal, 0x02: upper, 0x04: baby normal, 0x08: third normal, 0x10: third upper, 0x20: third baby)
	struct {
		unsigned short chance;
		int id;
	} mob[MAX_SEARCH]; //Holds the mobs that have the highest drop rate for this item. [Skotlex]
	struct script_code *script;	//Default script for everything.
	struct script_code *equip_script;	//Script executed once when equipping.
	struct script_code *unequip_script;//Script executed once when unequipping.
	struct {
		unsigned available : 1;
		unsigned no_refine : 1;	// [celest]
		unsigned delay_consume : 1;	// Signifies items that are not consumed immediately upon double-click [Skotlex]
		unsigned trade_restriction : 9;	//Item restrictions mask [Skotlex]
		unsigned autoequip: 1;
		unsigned buyingstore : 1;
	} flag;
	struct {// item stacking limitation
		unsigned short amount;
		unsigned int inventory:1;
		unsigned int cart:1;
		unsigned int storage:1;
		unsigned int guildstorage:1;
	} stack;
	struct {// used by item_nouse.txt
		unsigned int flag;
		unsigned short override;
	} item_usage;
	short gm_lv_trade_override;	//GM-level to override trade_restriction
	/* bugreport:309 */
	struct item_combo **combos;
	unsigned char combos_count;
	/* TODO add a pointer to some sort of (struct extra) and gather all the not-common vals into it to save memory */
	struct item_group *group;
	struct item_package *package;
};

struct item_combo {
	struct script_code *script;
	unsigned short *nameid;/* nameid array */
	unsigned char count;
	unsigned short id;/* id of this combo */
	bool isRef;/* whether this struct is a reference or the master */
};

struct item_group {
	unsigned short id;
	unsigned short *nameid;
	unsigned short qty;
};

struct item_chain_entry {
	unsigned short id;
	unsigned short rate;
	struct item_chain_entry *next;
};

struct item_chain {
	struct item_chain_entry *items;
	unsigned short qty;
};

struct item_package_rand_entry {
	unsigned short id;
	unsigned short qty;
	unsigned short rate;
	unsigned short hours;
	unsigned int announce : 1;
	unsigned int named : 1;
	struct item_package_rand_entry *next;
};

struct item_package_must_entry {
	unsigned short id;
	unsigned short qty;
	unsigned short hours;
	unsigned int announce : 1;
	unsigned int named : 1;
};

struct item_package_rand_group {
	struct item_package_rand_entry *random_list;
	unsigned short random_qty;
};

struct item_package {
	unsigned short id;
	struct item_package_rand_group *random_groups;
	struct item_package_must_entry *must_items;
	unsigned short random_qty;
	unsigned short must_qty;
};

#define itemdb_name(n) itemdb->search(n)->name
#define itemdb_jname(n) itemdb->search(n)->jname
#define itemdb_type(n) itemdb->search(n)->type
#define itemdb_atk(n) itemdb->search(n)->atk
#define itemdb_def(n) itemdb->search(n)->def
#define itemdb_look(n) itemdb->search(n)->look
#define itemdb_weight(n) itemdb->search(n)->weight
#define itemdb_equip(n) itemdb->search(n)->equip
#define itemdb_usescript(n) itemdb->search(n)->script
#define itemdb_equipscript(n) itemdb->search(n)->script
#define itemdb_wlv(n) itemdb->search(n)->wlv
#define itemdb_range(n) itemdb->search(n)->range
#define itemdb_slot(n) itemdb->search(n)->slot
#define itemdb_available(n) (itemdb->search(n)->flag.available)
#define itemdb_viewid(n) (itemdb->search(n)->view_id)
#define itemdb_autoequip(n) (itemdb->search(n)->flag.autoequip)
#define itemdb_is_rune(n) ((n >= ITEMID_NAUTHIZ && n <= ITEMID_HAGALAZ) || n == ITEMID_LUX_ANIMA)
#define itemdb_is_element(n) (n >= 990 && n <= 993)
#define itemdb_is_spellbook(n) (n >= 6188 && n <= 6205)
#define itemdb_is_poison(n) (n >= 12717 && n <= 12724)
#define itemid_isgemstone(id) ( (id) >= ITEMID_YELLOW_GEMSTONE && (id) <= ITEMID_BLUE_GEMSTONE )
#define itemdb_iscashfood(id) ( (id) >= 12202 && (id) <= 12207 )
#define itemdb_is_GNbomb(n) (n >= 13260 && n <= 13267)
#define itemdb_is_GNthrowable(n) (n >= 13268 && n <= 13290)

#define itemdb_value_buy(n) itemdb->search(n)->value_buy
#define itemdb_value_sell(n) itemdb->search(n)->value_sell
#define itemdb_canrefine(n) (!itemdb->search(n)->flag.no_refine)
//Item trade restrictions [Skotlex]
#define itemdb_isdropable(item, gmlv) itemdb->isrestricted(item, gmlv, 0, itemdb->isdropable_sub)
#define itemdb_cantrade(item, gmlv, gmlv2) itemdb->isrestricted(item, gmlv, gmlv2, itemdb->cantrade_sub)
#define itemdb_canpartnertrade(item, gmlv, gmlv2) itemdb->isrestricted(item, gmlv, gmlv2, itemdb->canpartnertrade_sub)
#define itemdb_cansell(item, gmlv) itemdb->isrestricted(item, gmlv, 0, itemdb->cansell_sub)
#define itemdb_cancartstore(item, gmlv)  itemdb->isrestricted(item, gmlv, 0, itemdb->cancartstore_sub)
#define itemdb_canstore(item, gmlv) itemdb->isrestricted(item, gmlv, 0, itemdb->canstore_sub)
#define itemdb_canguildstore(item, gmlv) itemdb->isrestricted(item , gmlv, 0, itemdb->canguildstore_sub)
#define itemdb_canmail(item, gmlv) itemdb->isrestricted(item , gmlv, 0, itemdb->canmail_sub)
#define itemdb_canauction(item, gmlv) itemdb->isrestricted(item , gmlv, 0, itemdb->canauction_sub)

struct itemdb_interface {
	void (*init) (void);
	void (*final) (void);
	void (*reload) (void);
	void (*name_constants) (void);
	void (*force_name_constants) (void);
	/* */
	struct item_group *groups;
	unsigned short group_count;
	/* */
	struct item_chain *chains;
	unsigned short chain_count;
	unsigned short chain_cache[ECC_MAX];
	/* */
	struct item_package *packages;
	unsigned short package_count;
	/* */
	DBMap *names;
	/* */
	struct item_data *array[MAX_ITEMDB];
	DBMap *other;// int nameid -> struct item_data*
	struct item_data dummy; //This is the default dummy item used for non-existant items. [Skotlex]
	/* */
	void (*read_groups) (void);
	void (*read_chains) (void);
	void (*read_packages) (void);
	/* */
	void (*write_cached_packages) (const char *config_filename);
	bool (*read_cached_packages) (const char *config_filename);
	/* */
	struct item_data* (*name2id) (const char *str);
	struct item_data* (*search_name) (const char *name);
	int (*search_name_array) (struct item_data** data, int size, const char *str, int flag);
	struct item_data* (*load)(int nameid);
	struct item_data* (*search)(int nameid);
	int (*parse_dbrow) (char** str, const char* source, int line, int scriptopt);
	struct item_data* (*exists) (int nameid);
	bool (*in_group) (struct item_group *group, int nameid);
	int (*group_item) (struct item_group *group);
	int (*chain_item) (unsigned short chain_id, int *rate);
	void (*package_item) (struct map_session_data *sd, struct item_package *package);
	int (*searchname_sub) (DBKey key, DBData *data, va_list ap);
	int (*searchname_array_sub) (DBKey key, DBData data, va_list ap);
	int (*searchrandomid) (struct item_group *group);
	const char* (*typename) (int type);
	void (*jobid2mapid) (unsigned int *bclass, unsigned int jobmask);
	void (*create_dummy_data) (void);
	struct item_data* (*create_item_data) (int nameid);
	int (*isequip) (int nameid);
	int (*isequip2) (struct item_data *data);
	int (*isstackable) (int nameid);
	int (*isstackable2) (struct item_data *data);
	int (*isdropable_sub) (struct item_data *item, int gmlv, int unused);
	int (*cantrade_sub) (struct item_data *item, int gmlv, int gmlv2);
	int (*canpartnertrade_sub) (struct item_data *item, int gmlv, int gmlv2);
	int (*cansell_sub) (struct item_data *item, int gmlv, int unused);
	int (*cancartstore_sub) (struct item_data *item, int gmlv, int unused);
	int (*canstore_sub) (struct item_data *item, int gmlv, int unused);
	int (*canguildstore_sub) (struct item_data *item, int gmlv, int unused);
	int (*canmail_sub) (struct item_data *item, int gmlv, int unused);
	int (*canauction_sub) (struct item_data *item, int gmlv, int unused);
	int (*isrestricted) (struct item *item, int gmlv, int gmlv2, int(*func)(struct item_data *, int, int));
	int (*isidentified) (int nameid);
	int (*isidentified2) (struct item_data *data);
	bool (*read_itemavail) (char *str[], int columns, int current);
	bool (*read_itemtrade) (char *str[], int columns, int current);
	bool (*read_itemdelay) (char *str[], int columns, int current);
	bool (*read_stack) (char *fields[], int columns, int current);
	bool (*read_buyingstore) (char *fields[], int columns, int current);
	bool (*read_nouse) (char *fields[], int columns, int current);
	int (*combo_split_atoi) (char *str, int *val);
	void (*read_combos) ();
	int (*gendercheck) (struct item_data *id);
	void (*re_split_atoi) (char *str, int *atk, int *matk);
	int (*readdb) (void);
	int (*read_sqldb) (void);
	uint64 (*unique_id) (int8 flag, int64 value);
	int (*uid_load) ();
	void (*read) (void);
	void (*destroy_item_data) (struct item_data *self, int free_self);
	int (*final_sub) (DBKey key, DBData *data, va_list ap);
};

struct itemdb_interface *itemdb;

void itemdb_defaults(void);

#endif /* _ITEMDB_H_ */
