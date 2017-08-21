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
#ifndef MAP_ITEMDB_H
#define MAP_ITEMDB_H

/* #include "map/map.h" */
#include "common/hercules.h"
#include "common/db.h"
#include "common/mmo.h" // ITEM_NAME_LENGTH

struct config_setting_t;
struct script_code;
struct hplugin_data_store;

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
#define itemdb_isspecial(i) ((i) == CARD0_FORGE || (i) == CARD0_CREATE || (i) == CARD0_PET)

//Use apple for unknown items.
#define UNKNOWN_ITEM_ID 512

enum item_itemid {
	ITEMID_RED_POTION            = 501,
	ITEMID_YELLOW_POTION         = 503,
	ITEMID_WHITE_POTION          = 504,
	ITEMID_BLUE_POTION           = 505,
	ITEMID_APPLE                 = 512,
	ITEMID_CARROT                = 515,
	ITEMID_HOLY_WATER            = 523,
	ITEMID_PUMPKIN               = 535,
	ITEMID_RED_SLIM_POTION       = 545,
	ITEMID_YELLOW_SLIM_POTION    = 546,
	ITEMID_WHITE_SLIM_POTION     = 547,
	ITEMID_WING_OF_FLY           = 601,
	ITEMID_WING_OF_BUTTERFLY     = 602,
	ITEMID_BRANCH_OF_DEAD_TREE   = 604,
	ITEMID_ANODYNE               = 605,
	ITEMID_ALOEBERA              = 606,
	ITEMID_SPECTACLES            = 611,
	ITEMID_POISON_BOTTLE         = 678,
	ITEMID_EMPTY_BOTTLE          = 713,
	ITEMID_EMPERIUM              = 714,
	ITEMID_YELLOW_GEMSTONE       = 715,
	ITEMID_RED_GEMSTONE          = 716,
	ITEMID_BLUE_GEMSTONE         = 717,
	ITEMID_ORIDECON_STONE        = 756,
	ITEMID_ALCHOL                = 970,
	ITEMID_ORIDECON              = 984,
	ITEMID_ANVIL                 = 986,
	ITEMID_ORIDECON_ANVIL        = 987,
	ITEMID_GOLDEN_ANVIL          = 988,
	ITEMID_EMPERIUM_ANVIL        = 989,
	ITEMID_BOODY_RED             = 990,
	ITEMID_CRYSTAL_BLUE          = 991,
	ITEMID_WIND_OF_VERDURE       = 992,
	ITEMID_YELLOW_LIVE           = 993,
	ITEMID_FLAME_HEART           = 994,
	ITEMID_MISTIC_FROZEN         = 995,
	ITEMID_ROUGH_WIND            = 996,
	ITEMID_GREAT_NATURE          = 997,
	ITEMID_IRON                  = 998,
	ITEMID_STEEL                 = 999,
	ITEMID_STAR_CRUMB            = 1000,
	ITEMID_IRON_ORE              = 1002,
	ITEMID_PHRACON               = 1010,
	ITEMID_EMVERETARCON          = 1011,
	ITEMID_BOOBY_TRAP            = 1065,
	ITEMID_PILEBUNCKER           = 1549,
	ITEMID_ANGRA_MANYU           = 1599,
	ITEMID_STRANGE_EMBRYO        = 6415,
	ITEMID_FACE_PAINT            = 6120,
	ITEMID_SCARLET_PTS           = 6360,
	ITEMID_INDIGO_PTS            = 6361,
	ITEMID_YELLOW_WISH_PTS       = 6362,
	ITEMID_LIME_GREEN_PTS        = 6363,
	ITEMID_STONE                 = 7049,
	ITEMID_FIRE_BOTTLE           = 7135,
	ITEMID_ACID_BOTTLE           = 7136,
	ITEMID_MENEATER_PLANT_BOTTLE = 7137,
	ITEMID_MINI_BOTTLE           = 7138,
	ITEMID_COATING_BOTTLE        = 7139,
	ITEMID_FRAGMENT_OF_CRYSTAL   = 7321,
	ITEMID_SKULL_                = 7420,
	ITEMID_TOKEN_OF_SIEGFRIED    = 7621,
	ITEMID_SPECIAL_ALLOY_TRAP    = 7940,
	ITEMID_CATNIP_FRUIT          = 11602,
	ITEMID_RED_POUCH_OF_SURPRISE = 12024,
	ITEMID_BLOODY_DEAD_BRANCH    = 12103,
	ITEMID_PORING_BOX            = 12109,
	ITEMID_MERCENARY_RED_POTION  = 12184,
	ITEMID_MERCENARY_BLUE_POTION = 12185,
	ITEMID_BATTLE_MANUAL         = 12208,
	ITEMID_BUBBLE_GUM            = 12210,
	ITEMID_GIANT_FLY_WING        = 12212,
	ITEMID_NEURALIZER            = 12213,
	ITEMID_M_CENTER_POTION       = 12241,
	ITEMID_M_AWAKENING_POTION    = 12242,
	ITEMID_M_BERSERK_POTION      = 12243,
	ITEMID_COMP_BATTLE_MANUAL    = 12263,
	ITEMID_COMP_BUBBLE_GUM       = 12264,
	ITEMID_LOVE_ANGEL            = 12287,
	ITEMID_SQUIRREL              = 12288,
	ITEMID_GOGO                  = 12289,
	ITEMID_PICTURE_DIARY         = 12304,
	ITEMID_MINI_HEART            = 12305,
	ITEMID_NEWCOMER              = 12306,
	ITEMID_KID                   = 12307,
	ITEMID_MAGIC_CASTLE          = 12308,
	ITEMID_BULGING_HEAD          = 12309,
	ITEMID_THICK_MANUAL50        = 12312,
	ITEMID_N_MAGNIFIER           = 12325,
	ITEMID_ANSILA                = 12333,
	ITEMID_REPAIRA               = 12392,
	ITEMID_REPAIRB               = 12393,
	ITEMID_REPAIRC               = 12394,
	ITEMID_BLACK_THING           = 12435,
	ITEMID_BOARDING_HALTER       = 12622,
	ITEMID_NOBLE_NAMEPLATE       = 12705,
	ITEMID_POISON_PARALYSIS      = 12717,
	ITEMID_POISON_LEECH          = 12718,
	ITEMID_POISON_OBLIVION       = 12719,
	ITEMID_POISON_CONTAMINATION  = 12720,
	ITEMID_POISON_NUMB           = 12721,
	ITEMID_POISON_FEVER          = 12722,
	ITEMID_POISON_LAUGHING       = 12723,
	ITEMID_POISON_FATIGUE        = 12724,
	ITEMID_NAUTHIZ               = 12725,
	ITEMID_RAIDO                 = 12726,
	ITEMID_BERKANA               = 12727,
	ITEMID_ISA                   = 12728,
	ITEMID_OTHILA                = 12729,
	ITEMID_URUZ                  = 12730,
	ITEMID_THURISAZ              = 12731,
	ITEMID_WYRD                  = 12732,
	ITEMID_HAGALAZ               = 12733,
	ITEMID_DUN_TELE_SCROLL1      = 14527,
	ITEMID_BATTLE_MANUAL25       = 14532,
	ITEMID_BATTLE_MANUAL100      = 14533,
	ITEMID_BATTLE_MANUAL_X3      = 14545,
	ITEMID_DUN_TELE_SCROLL2      = 14581,
	ITEMID_WOB_RUNE              = 14582,
	ITEMID_WOB_SCHWALTZ          = 14583,
	ITEMID_WOB_RACHEL            = 14584,
	ITEMID_WOB_LOCAL             = 14585,
	ITEMID_SIEGE_TELEPORT_SCROLL = 14591,
	ITEMID_JOB_MANUAL50          = 14592,
	ITEMID_PILEBUNCKER_S         = 16030,
	ITEMID_PILEBUNCKER_P         = 16031,
	ITEMID_PILEBUNCKER_T         = 16032,
	ITEMID_LUX_ANIMA             = 22540,
};

enum cards_item_list {
	ITEMID_GHOSTRING_CARD = 4047,
	ITEMID_PHREEONI_CARD  = 4121,
	ITEMID_MISTRESS_CARD  = 4132,
	ITEMID_ORC_LOAD_CARD  = 4135,
	ITEMID_ORC_HERO_CARD  = 4143,
	ITEMID_TAO_GUNKA_CARD = 4302,
};

/**
 * Mechanic
 **/
enum mechanic_item_list {
	ITEMID_ACCELERATOR                = 2800,
	ITEMID_HOVERING_BOOSTER,         // 2801
	ITEMID_SUICIDAL_DEVICE,          // 2802
	ITEMID_SHAPE_SHIFTER,            // 2803
	ITEMID_COOLING_DEVICE,           // 2804
	ITEMID_MAGNETIC_FIELD_GENERATOR, // 2805
	ITEMID_BARRIER_BUILDER,          // 2806
	ITEMID_REPAIR_KIT,               // 2807
	ITEMID_CAMOUFLAGE_GENERATOR,     // 2808
	ITEMID_HIGH_QUALITY_COOLER,      // 2809
	ITEMID_SPECIAL_COOLER,           // 2810
	ITEMID_MONKEY_SPANNER             = 6186,
};

/**
 * Spell Books
 */
enum spell_book_item_list {
	ITEMID_MAGIC_BOOK_FB    = 6189,
	ITEMID_MAGIC_BOOK_CB,  // 6190
	ITEMID_MAGIC_BOOK_LB,  // 6191
	ITEMID_MAGIC_BOOK_SG,  // 6192
	ITEMID_MAGIC_BOOK_LOV, // 6193
	ITEMID_MAGIC_BOOK_MS,  // 6194
	ITEMID_MAGIC_BOOK_CM,  // 6195
	ITEMID_MAGIC_BOOK_TV,  // 6196
	ITEMID_MAGIC_BOOK_TS,  // 6197
	ITEMID_MAGIC_BOOK_JT,  // 6198
	ITEMID_MAGIC_BOOK_WB,  // 6199
	ITEMID_MAGIC_BOOK_HD,  // 6200
	ITEMID_MAGIC_BOOK_ES,  // 6201
	ITEMID_MAGIC_BOOK_ES_, // 6202
	ITEMID_MAGIC_BOOK_CL,  // 6203
	ITEMID_MAGIC_BOOK_CR,  // 6204
	ITEMID_MAGIC_BOOK_DL,  // 6205
};

/**
 * Mercenary Scrolls
 */
enum mercenary_scroll_item_list {
	ITEMID_BOW_MERCENARY_SCROLL1     = 12153,
	ITEMID_BOW_MERCENARY_SCROLL2,   // 12154
	ITEMID_BOW_MERCENARY_SCROLL3,   // 12155
	ITEMID_BOW_MERCENARY_SCROLL4,   // 12156
	ITEMID_BOW_MERCENARY_SCROLL5,   // 12157
	ITEMID_BOW_MERCENARY_SCROLL6,   // 12158
	ITEMID_BOW_MERCENARY_SCROLL7,   // 12159
	ITEMID_BOW_MERCENARY_SCROLL8,   // 12160
	ITEMID_BOW_MERCENARY_SCROLL9,   // 12161
	ITEMID_BOW_MERCENARY_SCROLL10,  // 12162
	ITEMID_SWORDMERCENARY_SCROLL1,  // 12163
	ITEMID_SWORDMERCENARY_SCROLL2,  // 12164
	ITEMID_SWORDMERCENARY_SCROLL3,  // 12165
	ITEMID_SWORDMERCENARY_SCROLL4,  // 12166
	ITEMID_SWORDMERCENARY_SCROLL5,  // 12167
	ITEMID_SWORDMERCENARY_SCROLL6,  // 12168
	ITEMID_SWORDMERCENARY_SCROLL7,  // 12169
	ITEMID_SWORDMERCENARY_SCROLL8,  // 12170
	ITEMID_SWORDMERCENARY_SCROLL9,  // 12171
	ITEMID_SWORDMERCENARY_SCROLL10, // 12172
	ITEMID_SPEARMERCENARY_SCROLL1,  // 12173
	ITEMID_SPEARMERCENARY_SCROLL2,  // 12174
	ITEMID_SPEARMERCENARY_SCROLL3,  // 12175
	ITEMID_SPEARMERCENARY_SCROLL4,  // 12176
	ITEMID_SPEARMERCENARY_SCROLL5,  // 12177
	ITEMID_SPEARMERCENARY_SCROLL6,  // 12178
	ITEMID_SPEARMERCENARY_SCROLL7,  // 12179
	ITEMID_SPEARMERCENARY_SCROLL8,  // 12180
	ITEMID_SPEARMERCENARY_SCROLL9,  // 12181
	ITEMID_SPEARMERCENARY_SCROLL10, // 12182
};

/**
 * Geneticist
 */
enum geneticist_item_list {
	/// Pharmacy / Cooking
	ITEMID_SEED_OF_HORNY_PLANT      = 6210,
	ITEMID_BLOODSUCK_PLANT_SEED,   // 6211
	ITEMID_BOMB_MUSHROOM_SPORE,    // 6212
	ITEMID_HP_INCREASE_POTIONS      = 12422,
	ITEMID_HP_INCREASE_POTIONM,    // 12423
	ITEMID_HP_INCREASE_POTIONL,    // 12424
	ITEMID_SP_INCREASE_POTIONS,    // 12425
	ITEMID_SP_INCREASE_POTIONM,    // 12426
	ITEMID_SP_INCREASE_POTIONL,    // 12427
	ITEMID_ENRICH_WHITE_POTIONZ,   // 12428
	ITEMID_SAVAGE_BBQ,             // 12429
	ITEMID_WUG_BLOOD_COCKTAIL,     // 12430
	ITEMID_MINOR_BRISKET,          // 12431
	ITEMID_SIROMA_ICETEA,          // 12432
	ITEMID_DROCERA_HERB_STEW,      // 12433
	ITEMID_PETTI_TAIL_NOODLE,      // 12434
	ITEMID_VITATA500,              // 12436
	ITEMID_ENRICH_CELERMINE_JUICE, // 12437
	ITEMID_CURE_FREE,              // 12475
	/// Bombs
	ITEMID_APPLE_BOMB               = 13260,
	ITEMID_COCONUT_BOMB,           // 13261
	ITEMID_MELON_BOMB,             // 13262
	ITEMID_PINEAPPLE_BOMB,         // 13263
	ITEMID_BANANA_BOMB,            // 13264
	ITEMID_BLACK_LUMP,             // 13265
	ITEMID_BLACK_HARD_LUMP,        // 13266
	ITEMID_VERY_HARD_LUMP,         // 13267
	/// Throwables
	ITEMID_MYSTERIOUS_POWDER,      // 13268
	ITEMID_BOOST500_TO_THROW,      // 13269
	ITEMID_FULL_SWINGK_TO_THROW,   // 13270
	ITEMID_MANA_PLUS_TO_THROW,     // 13271
	ITEMID_CURE_FREE_TO_THROW,     // 13272
	ITEMID_STAMINA_UP_M_TO_THROW,  // 13273
	ITEMID_DIGESTIVE_F_TO_THROW,   // 13274
	ITEMID_HP_INC_POTS_TO_THROW,   // 13275
	ITEMID_HP_INC_POTM_TO_THROW,   // 13276
	ITEMID_HP_INC_POTL_TO_THROW,   // 13277
	ITEMID_SP_INC_POTS_TO_THROW,   // 13278
	ITEMID_SP_INC_POTM_TO_THROW,   // 13279
	ITEMID_SP_INC_POTL_TO_THROW,   // 13280
	ITEMID_EN_WHITE_POTZ_TO_THROW, // 13281
	ITEMID_VITATA500_TO_THROW,     // 13282
	ITEMID_EN_CEL_JUICE_TO_THROW,  // 13283
	ITEMID_SAVAGE_BBQ_TO_THROW,    // 13284
	ITEMID_WUG_COCKTAIL_TO_THROW,  // 13285
	ITEMID_M_BRISKET_TO_THROW,     // 13286
	ITEMID_SIROMA_ICETEA_TO_THROW, // 13287
	ITEMID_DROCERA_STEW_TO_THROW,  // 13288
	ITEMID_PETTI_NOODLE_TO_THROW,  // 13289
	ITEMID_BLACK_THING_TO_THROW,   // 13290
};

//
enum e_chain_cache {
	ECC_ORE,
	/* */
	ECC_MAX,
};

enum item_class_upper {
	ITEMUPPER_NONE       = 0x00,
	ITEMUPPER_NORMAL     = 0x01,
	ITEMUPPER_UPPER      = 0x02,
	ITEMUPPER_BABY       = 0x04,
	ITEMUPPER_THIRD      = 0x08,
	ITEMUPPER_THIRDUPPER = 0x10,
	ITEMUPPER_THIRDBABY  = 0x20,
	ITEMUPPER_ALL        = 0x3f, // Sum of all the above
};

/**
 * Item Trade restrictions
 */
enum ItemTradeRestrictions {
	ITR_NONE            = 0x000, ///< No restrictions
	ITR_NODROP          = 0x001, ///< Item can't be dropped
	ITR_NOTRADE         = 0x002, ///< Item can't be traded (nor vended)
	ITR_PARTNEROVERRIDE = 0x004, ///< Wedded partner can override ITR_NOTRADE restriction
	ITR_NOSELLTONPC     = 0x008, ///< Item can't be sold to NPCs
	ITR_NOCART          = 0x010, ///< Item can't be placed in the cart
	ITR_NOSTORAGE       = 0x020, ///< Item can't be placed in the storage
	ITR_NOGSTORAGE      = 0x040, ///< Item can't be placed in the guild storage
	ITR_NOMAIL          = 0x080, ///< Item can't be attached to mail messages
	ITR_NOAUCTION       = 0x100, ///< Item can't be auctioned

	ITR_ALL             = 0x1ff  ///< Sum of all the above values
};

/**
 * Item No-use restrictions
 */
enum ItemNouseRestrictions {
	INR_NONE    = 0x0, ///< No restrictions
	INR_SITTING = 0x1, ///< Item can't be used while sitting

	INR_ALL     = 0x1 ///< Sum of all the above values
};

/**
 * Item Option Types
 */
enum ItemOptionTypes {
	IT_OPT_INDEX = 0,
	IT_OPT_VALUE,
	IT_OPT_PARAM,
	IT_OPT_MAX
};

/** Convenience item list (entry) used in various functions */
struct itemlist_entry {
	int id;       ///< Item ID or (inventory) index
	int16 amount; ///< Amount
};
/** Convenience item list used in various functions */
VECTOR_STRUCT_DECL(itemlist, struct itemlist_entry);

struct item_combo {
	struct script_code *script;
	unsigned short nameid[MAX_ITEMS_PER_COMBO];/* nameid array */
	unsigned char count;
	unsigned short id;/* id of this combo */
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
	unsigned int force_serial: 1;
	struct item_package_rand_entry *next;
};

struct item_package_must_entry {
	unsigned short id;
	unsigned short qty;
	unsigned short hours;
	unsigned int announce : 1;
	unsigned int named : 1;
	unsigned int force_serial : 1;
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

struct item_option {
	int16 index;
	struct script_code *script;
};

struct item_data {
	uint16 nameid;
	char name[ITEM_NAME_LENGTH],jname[ITEM_NAME_LENGTH];

	int value_buy;
	int value_sell;
	int type;
	int subtype;
	int maxchance; //For logs, for external game info, for scripts: Max drop chance of this item (e.g. 0.01% , etc.. if it = 0, then monsters don't drop it, -1 denotes items sold in shops only) [Lupus]
	int sex;
	int equip;
	int weight;
	int atk;
	int def;
	int range;
	int slot;
	int view_sprite;
	int elv;
	int wlv;
	int view_id;
	int matk;
	int elvmax;/* maximum level for this item */

	int delay;
	uint64 class_base[3]; ///< Specifies if the base can wear this item (split in 3 indexes per type: 1-1, 2-1, 2-2)
	unsigned class_upper : 6;   ///< Specifies if the upper-type can equip it (bitfield, 0x01: normal, 0x02: upper, 0x04: baby normal, 0x08: third normal, 0x10: third upper, 0x20: third baby)
	struct {
		unsigned short chance;
		int id;
	} mob[MAX_SEARCH];                  ///< Holds the mobs that have the highest drop rate for this item. [Skotlex]
	struct script_code *script;         ///< Default script for everything.
	struct script_code *equip_script;   ///< Script executed once when equipping.
	struct script_code *unequip_script; ///< Script executed once when unequipping.
	struct {
		unsigned available : 1;
		unsigned no_refine : 1; // [celest]
		unsigned delay_consume : 1;     ///< Signifies items that are not consumed immediately upon double-click [Skotlex]
		unsigned trade_restriction : 9; ///< Item trade restrictions mask (@see enum ItemTradeRestrictions)
		unsigned autoequip: 1;
		unsigned buyingstore : 1;
		unsigned bindonequip : 1;
		unsigned keepafteruse : 1;
		unsigned force_serial : 1;
		unsigned no_options: 1; // < disallows use of item options on the item. (non-equippable items are automatically flagged) [Smokexyz]
		unsigned drop_announce : 1; // Official Drop Announce [Jedzkie]
	} flag;
	struct {// item stacking limitation
		unsigned short amount;
		unsigned int inventory:1;
		unsigned int cart:1;
		unsigned int storage:1;
		unsigned int guildstorage:1;
	} stack;
	struct {
		unsigned int flag; ///< Item nouse restriction mask (@see enum ItemNouseRestrictions)
		unsigned short override;
	} item_usage;
	short gm_lv_trade_override; ///< GM-level to override trade_restriction
	/* bugreport:309 */
	struct item_combo **combos;
	unsigned char combos_count;
	/* TODO add a pointer to some sort of (struct extra) and gather all the not-common vals into it to save memory */
	struct item_group *group;
	struct item_package *package;
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};

#define itemdb_name(n)        (itemdb->search(n)->name)
#define itemdb_jname(n)       (itemdb->search(n)->jname)
#define itemdb_type(n)        (itemdb->search(n)->type)
#define itemdb_atk(n)         (itemdb->search(n)->atk)
#define itemdb_def(n)         (itemdb->search(n)->def)
#define itemdb_subtype(n)     (itemdb->search(n)->subtype)
#define itemdb_sprite(n)      (itemdb->search(n)->view_sprite)
#define itemdb_weight(n)      (itemdb->search(n)->weight)
#define itemdb_equip(n)       (itemdb->search(n)->equip)
#define itemdb_usescript(n)   (itemdb->search(n)->script)
#define itemdb_equipscript(n) (itemdb->search(n)->script)
#define itemdb_wlv(n)         (itemdb->search(n)->wlv)
#define itemdb_range(n)       (itemdb->search(n)->range)
#define itemdb_slot(n)        (itemdb->search(n)->slot)
#define itemdb_available(n)   (itemdb->search(n)->flag.available)
#define itemdb_viewid(n)      (itemdb->search(n)->view_id)
#define itemdb_autoequip(n)   (itemdb->search(n)->flag.autoequip)
#define itemdb_value_buy(n)   (itemdb->search(n)->value_buy)
#define itemdb_value_sell(n)  (itemdb->search(n)->value_sell)
#define itemdb_canrefine(n)   (!itemdb->search(n)->flag.no_refine)
#define itemdb_allowoption(n) (!itemdb->search(n)->flag.no_options)

#define itemdb_is_element(n)     ((n) >= ITEMID_SCARLET_PTS && (n) <= ITEMID_LIME_GREEN_PTS)
#define itemdb_is_spellbook(n)   ((n) >= ITEMID_MAGIC_BOOK_FB && (n) <= ITEMID_MAGIC_BOOK_DL)
#define itemdb_is_poison(n)      ((n) >= ITEMID_POISON_PARALYSIS && (n) <= ITEMID_POISON_FATIGUE)
#define itemid_isgemstone(n)     ((n) >= ITEMID_YELLOW_GEMSTONE && (n) <= ITEMID_BLUE_GEMSTONE)
#define itemdb_is_GNbomb(n)      ((n) >= ITEMID_APPLE_BOMB && (n) <= ITEMID_VERY_HARD_LUMP)
#define itemdb_is_GNthrowable(n) ((n) >= ITEMID_MYSTERIOUS_POWDER && (n) <= ITEMID_BLACK_THING_TO_THROW)
#define itemid_is_pilebunker(n)  ((n) == ITEMID_PILEBUNCKER || (n) == ITEMID_PILEBUNCKER_P || (n) == ITEMID_PILEBUNCKER_S || (n) == ITEMID_PILEBUNCKER_T)
#define itemdb_is_shadowequip(n) ((n) & (EQP_SHADOW_ARMOR|EQP_SHADOW_WEAPON|EQP_SHADOW_SHIELD|EQP_SHADOW_SHOES|EQP_SHADOW_ACC_R|EQP_SHADOW_ACC_L))
#define itemdb_is_costumeequip(n) ((n) & (EQP_COSTUME_HEAD_TOP|EQP_COSTUME_HEAD_MID|EQP_COSTUME_HEAD_LOW|EQP_COSTUME_GARMENT))

//Item trade restrictions [Skotlex]
#define itemdb_isdropable(item, gmlv)             (itemdb->isrestricted((item), (gmlv), 0, itemdb->isdropable_sub))
#define itemdb_cantrade(item, gmlv, gmlv2)        (itemdb->isrestricted((item), (gmlv), (gmlv2), itemdb->cantrade_sub))
#define itemdb_canpartnertrade(item, gmlv, gmlv2) (itemdb->isrestricted((item), (gmlv), (gmlv2), itemdb->canpartnertrade_sub))
#define itemdb_cansell(item, gmlv)                (itemdb->isrestricted((item), (gmlv), 0, itemdb->cansell_sub))
#define itemdb_cancartstore(item, gmlv)           (itemdb->isrestricted((item), (gmlv), 0, itemdb->cancartstore_sub))
#define itemdb_canstore(item, gmlv)               (itemdb->isrestricted((item), (gmlv), 0, itemdb->canstore_sub))
#define itemdb_canguildstore(item, gmlv)          (itemdb->isrestricted((item), (gmlv), 0, itemdb->canguildstore_sub))
#define itemdb_canmail(item, gmlv)                (itemdb->isrestricted((item), (gmlv), 0, itemdb->canmail_sub))
#define itemdb_canauction(item, gmlv)             (itemdb->isrestricted((item), (gmlv), 0, itemdb->canauction_sub))

struct itemdb_interface {
	void (*init) (bool minimal);
	void (*final) (void);
	void (*reload) (void);
	void (*name_constants) (void);
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
	/* list of item combos loaded */
	struct item_combo **combos;
	unsigned short combo_count;
	/* */
	struct DBMap *names;
	/* */
	struct item_data *array[MAX_ITEMDB];
	struct DBMap *other;// int nameid -> struct item_data*
	struct DBMap *options; // int opt_id -> struct item_option*
	struct item_data dummy; //This is the default dummy item used for non-existant items. [Skotlex]
	/* */
	void (*read_groups) (void);
	void (*read_chains) (void);
	void (*read_packages) (void);
	void (*read_options) (void);
	/* */
	void (*write_cached_packages) (const char *config_filename);
	bool (*read_cached_packages) (const char *config_filename);
	/* */
	struct item_data* (*name2id) (const char *str);
	struct item_data* (*search_name) (const char *name);
	int (*search_name_array) (struct item_data** data, int size, const char *str, int flag);
	struct item_data* (*load)(int nameid);
	struct item_data* (*search)(int nameid);
	struct item_data* (*exists) (int nameid);
	struct item_option* (*option_exists) (int idx);
	bool (*in_group) (struct item_group *group, int nameid);
	int (*group_item) (struct item_group *group);
	int (*chain_item) (unsigned short chain_id, int *rate);
	void (*package_item) (struct map_session_data *sd, struct item_package *package);
	int (*searchname_sub) (union DBKey key, struct DBData *data, va_list ap);
	int (*searchname_array_sub) (union DBKey key, struct DBData data, va_list ap);
	int (*searchrandomid) (struct item_group *group);
	const char* (*typename) (int type);
	void (*jobmask2mapid) (uint64 *bclass, uint64 jobmask);
	void (*jobid2mapid) (uint64 *bclass, int job_class, bool enable);
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
	int (*combo_split_atoi) (char *str, int *val);
	void (*read_combos) (void);
	int (*gendercheck) (struct item_data *id);
	int (*validate_entry) (struct item_data *entry, int n, const char *source);
	void (*readdb_options_additional_fields) (struct item_option *ito, struct config_setting_t *t, const char *source);
	void (*readdb_additional_fields) (int itemid, struct config_setting_t *it, int n, const char *source);
	void (*readdb_job_sub) (struct item_data *id, struct config_setting_t *t);
	int (*readdb_libconfig_sub) (struct config_setting_t *it, int n, const char *source);
	int (*readdb_libconfig) (const char *filename);
	uint64 (*unique_id) (struct map_session_data *sd);
	void (*read) (bool minimal);
	void (*destroy_item_data) (struct item_data *self, int free_self);
	int (*final_sub) (union DBKey key, struct DBData *data, va_list ap);
	int (*options_final_sub) (union DBKey key, struct DBData *data, va_list ap);
	void (*clear) (bool total);
	struct item_combo * (*id2combo) (unsigned short id);
	bool (*is_item_usable) (struct item_data *item);
	bool (*lookup_const) (const struct config_setting_t *it, const char *name, int *value);
	bool (*lookup_const_mask) (const struct config_setting_t *it, const char *name, int *value);
};

#ifdef HERCULES_CORE
void itemdb_defaults(void);
#endif // HERCULES_CORE

HPShared struct itemdb_interface *itemdb;

#endif /* MAP_ITEMDB_H */
