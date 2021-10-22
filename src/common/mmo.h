/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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
#ifndef COMMON_MMO_H
#define COMMON_MMO_H

#include "config/core.h"
#include "common/cbasetypes.h"
#include "common/db.h" // VECTORS

// server->client protocol version
//        0 - pre-?
//        1 - ?                    - 0x196
//        2 - ?                    - 0x78, 0x79
//        3 - ?                    - 0x1c8, 0x1c9, 0x1de
//        4 - ?                    - 0x1d7, 0x1d8, 0x1d9, 0x1da
//        5 - 2003-12-18aSakexe+   - 0x1ee, 0x1ef, 0x1f0, ?0x1c4, 0x1c5?
//        6 - 2004-03-02aSakexe+   - 0x1f4, 0x1f5
//        7 - 2005-04-11aSakexe+   - 0x229, 0x22a, 0x22b, 0x22c
// 20061023 - 2006-10-23aSakexe+   - 0x6b, 0x6d
// 20070521 - 2007-05-21aSakexe+   - 0x283
// 20070821 - 2007-08-21aSakexe+   - 0x2c5
// 20070918 - 2007-09-18aSakexe+   - 0x2d7, 0x2d9, 0x2da
// 20071106 - 2007-11-06aSakexe+   - 0x78, 0x7c, 0x22c
// 20080102 - 2008-01-02aSakexe+   - 0x2ec, 0x2ed , 0x2ee
// 20081126 - 2008-11-26aSakexe+   - 0x1a2
// 20090408 - 2009-04-08aSakexe+   - 0x44a (don't use as it overlaps with RE client packets)
// 20080827 - 2008-08-27aRagexeRE+ - First RE Client
// 20081217 - 2008-12-17aRagexeRE+ - 0x6d (Note: This one still use old Char Info Packet Structure)
// 20081218 - 2008-12-17bRagexeRE+ - 0x6d (Note: From this one client use new Char Info Packet Structure)
// 20090603 - 2009-06-03aRagexeRE+ - 0x7d7, 0x7d8, 0x7d9, 0x7da
// 20090617 - 2009-06-17aRagexeRE+ - 0x7d9
// 20090922 - 2009-09-22aRagexeRE+ - 0x7e5, 0x7e7, 0x7e8, 0x7e9
// 20091103 - 2009-11-03aRagexeRE+ - 0x7f7, 0x7f8, 0x7f9
// 20100105 - 2010-01-05aRagexeRE+ - 0x133, 0x800, 0x801
// 20100126 - 2010-01-26aRagexeRE+ - 0x80e
// 20100223 - 2010-02-23aRagexeRE+ - 0x80f
// 20100413 - 2010-04-13aRagexeRE+ - 0x6b
// 20100629 - 2010-06-29aRagexeRE+ - 0x2d0, 0xaa, 0x2d1, 0x2d2
// 20100721 - 2010-07-21aRagexeRE+ - 0x6b, 0x6d
// 20100727 - 2010-07-27aRagexeRE+ - 0x6b, 0x6d
// 20100803 - 2010-08-03aRagexeRE+ - 0x6b, 0x6d, 0x827, 0x828, 0x829, 0x82a, 0x82b, 0x82c, 0x842, 0x843
// 20101124 - 2010-11-24aRagexeRE+ - 0x856, 0x857, 0x858
// 20110111 - 2011-01-11aRagexeRE+ - 0x6b, 0x6d
// 20110928 - 2011-09-28aRagexeRE+ - 0x6b, 0x6d
// 20111025 - 2011-10-25aRagexeRE+ - 0x6b, 0x6d
// 20120307 - 2012-03-07aRagexeRE+ - 0x970

#ifndef PACKETVER
	#define PACKETVER 20190530
#endif // PACKETVER

//Uncomment the following line if your client is ragexeRE instead of ragexe (required because of conflicting packets in ragexe vs ragexeRE).
//#define ENABLE_PACKETVER_RE
#ifdef ENABLE_PACKETVER_RE
	#define PACKETVER_RE
	#undef ENABLE_PACKETVER_RE
#endif // DISABLE_PACKETVER_RE

//Uncomment the following line if your client is zero ragexe instead of normal kro clients
//#define ENABLE_PACKETVER_ZERO
#ifdef ENABLE_PACKETVER_ZERO
	#define PACKETVER_ZERO
	#undef ENABLE_PACKETVER_ZERO
#endif // DISABLE_PACKETVER_ZERO

//Uncomment the following line if your client is sakexe
//#define ENABLE_PACKETVER_SAK
#ifdef ENABLE_PACKETVER_SAK
	#define PACKETVER_SAK
	#undef ENABLE_PACKETVER_SAK
#endif // DISABLE_PACKETVER_SAK

//Uncomment the following line if your client is ragexeAD
//#define ENABLE_PACKETVER_AD
#ifdef ENABLE_PACKETVER_AD
	#define PACKETVER_AD
	#undef ENABLE_PACKETVER_AD
#endif // DISABLE_PACKETVER_AD

#if !defined(PACKETVER_RE) && !defined(PACKETVER_ZERO) && !defined(PACKETVER_SAK) && !defined(PACKETVER_AD)
	#define PACKETVER_MAIN_NUM PACKETVER
	#define PACKETTYPE "main"
#else
	#define PACKETVER_MAIN_NUM 0
#endif
#ifdef PACKETVER_RE
	#define PACKETVER_RE_NUM PACKETVER
	#define PACKETTYPE "RE"
#else
	#define PACKETVER_RE_NUM 0
#endif
#ifdef PACKETVER_ZERO
	#define PACKETVER_ZERO_NUM PACKETVER
	#define PACKETTYPE "zero"
#else
	#define PACKETVER_ZERO_NUM 0
#endif
#ifdef PACKETVER_SAK
	#define PACKETVER_SAK_NUM PACKETVER
	#define PACKETTYPE "sak"
#else
	#define PACKETVER_SAK_NUM 0
#endif
#ifdef PACKETVER_AD
	#define PACKETVER_AD_NUM PACKETVER
	#define PACKETTYPE "ad"
#else
	#define PACKETVER_AD_NUM 0
#endif

// Client support for experimental RagexeRE UI present in 2012-04-10 and 2012-04-18
#if defined(PACKETVER_RE) && ( PACKETVER == 20120410 || PACKETVER == 20120418 )
#define PARTY_RECRUIT
#endif // PACKETVER_RE && (PACKETVER == 20120410 || PACKETVER == 10120418)

// Comment the following line to disable sc_data saving. [Skotlex]
#ifndef ENABLE_SC_SAVING
#define ENABLE_SC_SAVING
#endif

#if PACKETVER_MAIN_NUM >= 20070711 || PACKETVER_RE_NUM >= 20080827 || PACKETVER_AD_NUM >= 20070711 || PACKETVER_SAK_NUM >= 20070628 || defined(PACKETVER_ZERO)
// Comment the following like to disable server-side hot-key saving support. [Skotlex]
// Note that newer clients no longer save hotkeys in the registry!
#define HOTKEY_SAVING

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
#define MAX_HOTKEYS 38
#elif PACKETVER_MAIN_NUM >= 20141022 || PACKETVER_RE_NUM >= 20141015 || defined(PACKETVER_ZERO)
// (38 = 9 skills x 4 bars & 2 Quickslots)(0x07d9,268)
#define MAX_HOTKEYS 38
#elif PACKETVER_MAIN_NUM >= 20090617 || PACKETVER_RE_NUM >= 20090617 || PACKETVER_SAK_NUM >= 20090617
// (38 = 9 skills x 4 bars & 2 Quickslots)(0x07d9,268)
#define MAX_HOTKEYS 38
#elif PACKETVER_MAIN_NUM >= 20090603 || PACKETVER_RE_NUM >= 20090603 || PACKETVER_SAK_NUM >= 20090603
// (36 = 9 skills x 4 bars)               (0x07d9,254)
#define MAX_HOTKEYS 36
#elif PACKETVER_MAIN_NUM >= 20070711 || PACKETVER_RE_NUM >= 20080827 || PACKETVER_AD_NUM >= 20070711 || PACKETVER_SAK_NUM >= 20070628
// (27 = 9 skills x 3 bars)               (0x02b9,191)
#define MAX_HOTKEYS 27
#endif
#endif  // PACKETVER_MAIN_NUM >= 20070711 || PACKETVER_RE_NUM >= 20080827 || PACKETVER_AD_NUM >= 20070711 || PACKETVER_SAK_NUM >= 20070628 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
#define MAX_HOTKEYS_DB ((MAX_HOTKEYS) * 2)
#else
#define MAX_HOTKEYS_DB MAX_HOTKEYS
#endif

#if PACKETVER >= 20150805 /* Cart Decoration */
	#define CART_DECORATION
	#define MAX_CARTDECORATION_CARTS 3 // Currently there are 3 Carts available in kRO. [Frost]
#else
	#define MAX_CARTDECORATION_CARTS 0
#endif
#if PACKETVER >= 20120201 /* New Geneticist Carts */
	#define NEW_CARTS
	#define MAX_BASE_CARTS 9
#else
	#define MAX_BASE_CARTS 5
#endif
#ifndef MAX_CARTS
#define MAX_CARTS (MAX_BASE_CARTS + MAX_CARTDECORATION_CARTS)
#endif

#ifndef MAX_INVENTORY
#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
#define MAX_INVENTORY 200
#else
#define MAX_INVENTORY 100
#endif  // PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
#endif  // MAX_INVENTORY

#ifndef FIXED_INVENTORY_SIZE
#define FIXED_INVENTORY_SIZE 100
#endif

#if FIXED_INVENTORY_SIZE > MAX_INVENTORY
#error FIXED_INVENTORY_SIZE must be same or smaller than MAX_INVENTORY
#endif

#if PACKETVER >= 20131223
	#define OFFICIAL_GUILD_STORAGE
#endif // PACKETVER >= 20131223
#ifdef DISABLE_OFFICIAL_GUILD_STORAGE
	#undef OFFICIAL_GUILD_STORAGE
	#undef DISABLE_OFFICIAL_GUILD_STORAGE
#endif // DISABLE_OFFICIAL_GUILD_STORAGE
#if PACKETVER < 20131223 && defined(OFFICIAL_GUILD_STORAGE)
	#error The current PACKETVER does not support the official guild storage system.
#endif // PACKETVER >= 20131223

//Max number of characters per account. Note that changing this setting alone is not enough if the client is not hexed to support more characters as well.
#if PACKETVER >= 20100413
#ifndef MAX_CHARS
	#define MAX_CHARS 12
#endif
#else
#ifndef MAX_CHARS
	#define MAX_CHARS 9
#endif
#endif
//Number of slots carded equipment can have. Never set to less than 4 as they are also used to keep the data of forged items/equipment. [Skotlex]
//Note: The client seems unable to receive data for more than 4 slots due to all related packets having a fixed size.
#ifndef MAX_SLOTS
#define MAX_SLOTS 4
#endif
//Max amount of a single stacked item
#ifndef MAX_AMOUNT
#define MAX_AMOUNT 30000
#endif
#ifndef MAX_ZENY
#define MAX_ZENY INT_MAX
#endif

//Official Limit: 2.1b ( the var that stores the money doesn't go much higher than this by default )
#ifndef MAX_BANK_ZENY
#define MAX_BANK_ZENY INT_MAX
#endif

#ifndef MAX_LEVEL
#define MAX_LEVEL 175
#endif
#ifndef MAX_FAME
#define MAX_FAME 1000000000
#endif
#ifndef MAX_CART
#define MAX_CART 100
#endif
#ifndef MAX_SKILL_DB
#define MAX_SKILL_DB 1355 ///< Maximum number of skills in the skill DB (compacted array size)
#endif
#ifndef MAX_SKILL_ID
#define MAX_SKILL_ID 10020   // [Ind/Hercules] max used skill ID
#endif
#ifndef MAX_SKILL_TREE
// Update this max as necessary. 86 is the value needed for Expanded Super Novice.
#define MAX_SKILL_TREE 86
#endif
#ifndef DEFAULT_WALK_SPEED
#define DEFAULT_WALK_SPEED 150
#endif
#ifndef MIN_WALK_SPEED
#define MIN_WALK_SPEED 20 /* below 20 clips animation */
#endif
#ifndef MAX_WALK_SPEED
#define MAX_WALK_SPEED 1000
#endif
#ifndef MAX_STORAGE
#define MAX_STORAGE 600
#endif
#ifndef OFFICIAL_GUILD_STORAGE
	#ifndef MAX_GUILD_STORAGE
	#define MAX_GUILD_STORAGE 600
	#endif
	#undef GUILD_STORAGE_EXPANSION_STEP
#else // ! OFFICIAL_GUILD_STORAGE
	#ifdef MAX_GUILD_STORAGE
	#error You cannot redefine MAX_GUILD_STORAGE when using the official guild storage system.
	#endif
	#ifndef GUILD_STORAGE_EXPANSION_STEP
		#define GUILD_STORAGE_EXPANSION_STEP 100
	#endif
	// GUILD_STORAGE_EXPANSION_STEP slots * 5 skill levels
	#define MAX_GUILD_STORAGE (GUILD_STORAGE_EXPANSION_STEP * 5)
#endif // OFFICIAL_GUILD_STORAGE
#ifndef MAX_PARTY
#define MAX_PARTY 12
#endif
#ifndef BASE_GUILD_SIZE
#define BASE_GUILD_SIZE 16               // Base guild members (without GD_EXTENSION)
#endif
#ifndef MAX_GUILD
#define MAX_GUILD (BASE_GUILD_SIZE+10*6) // Increased max guild members +6 per 1 extension levels [Lupus]
#endif
#ifndef MAX_GUILDPOSITION
#define MAX_GUILDPOSITION 20             // Increased max guild positions to accomodate for all members [Valaris] (removed) [PoW]
#endif
#ifndef MAX_GUILDEXPULSION
#define MAX_GUILDEXPULSION 32
#endif
#ifndef MAX_GUILDALLIANCE
#define MAX_GUILDALLIANCE 16
#endif
#ifndef MAX_GUILDSKILL
#define MAX_GUILDSKILL 17                // Increased max guild skills because of new skills [Sara-chan]
#endif
#ifndef MAX_GUILDLEVEL
#define MAX_GUILDLEVEL 50
#endif
#ifndef MAX_GUARDIANS
#define MAX_GUARDIANS 8                  // Local max per castle. [Skotlex]
#endif
#ifndef MAX_QUEST_OBJECTIVES
#define MAX_QUEST_OBJECTIVES 3           // Max quest objectives for a quest
#endif

// Achievements [Smokexyz/Hercules]
#ifndef MAX_ACHIEVEMENT_DB
#define MAX_ACHIEVEMENT_DB 360          // Maximum number of achievements
#define MAX_ACHIEVEMENT_OBJECTIVES 10   // Maximum number of achievement objectives
STATIC_ASSERT(MAX_ACHIEVEMENT_OBJECTIVES <= 10, "This value is limited by the client and database layout and should only be increased if you know the consequences.");
#define MAX_ACHIEVEMENT_RANKS 20 // Achievement Ranks
STATIC_ASSERT(MAX_ACHIEVEMENT_RANKS <= 255, "This value is limited by the client and database layout and should only be increased if you know the consequences.");
#define MAX_ACHIEVEMENT_ITEM_REWARDS 10 // Achievement Rewards
#endif

// for produce
#define MIN_ATTRIBUTE 0
#define MAX_ATTRIBUTE 4
#define ATTRIBUTE_NORMAL 0
#define MIN_STAR 0
#define MAX_STAR 3

#define MAX_STATUS_TYPE 5

#define WEDDING_RING_M 2634
#define WEDDING_RING_F 2635

//For character names, title names, guilds, maps, etc.
//Includes null-terminator as it is the length of the array.
#define NAME_LENGTH (23 + 1)
//For item names, which tend to have much longer names.
#define ITEM_NAME_LENGTH 50
//For Map Names, which the client considers to be 16 in length including the .gat extension.
#define MAP_NAME_LENGTH (11 + 1)
#define MAP_NAME_LENGTH_EXT (MAP_NAME_LENGTH + 4)

#ifndef MAX_FRIENDS
#define MAX_FRIENDS 40
#endif
#define MAX_MEMOPOINTS 3

// Size of the fame list arrays.
#define MAX_FAME_LIST 10

// Limits to avoid ID collision with other game objects
#define START_ACCOUNT_NUM 2000000
#define END_ACCOUNT_NUM 100000000
#define START_CHAR_NUM 150000

// Guilds
#define MAX_GUILDMES1 60
#define MAX_GUILDMES2 120

// Base Homun skill.
#ifndef HM_SKILLBASE
#define HM_SKILLBASE 8001
#endif
#ifndef MAX_HOMUNSKILL
#define MAX_HOMUNSKILL 43
#endif

// Mail System
#define MAIL_MAX_INBOX 30
#define MAIL_TITLE_LENGTH 40
#define MAIL_BODY_LENGTH 200

// Mercenary System
#ifndef MC_SKILLBASE
#define MC_SKILLBASE 8201
#endif
#ifndef MAX_MERCSKILL
#define MAX_MERCSKILL 40
#endif

// Elemental System
#ifndef MAX_ELEMENTALSKILL
#define MAX_ELEMENTALSKILL 42
#endif
#ifndef EL_SKILLBASE
#define EL_SKILLBASE 8401
#endif
#ifndef MAX_ELESKILLTREE
#define MAX_ELESKILLTREE 3
#endif

// Maximum item options [Smokexyz]
#ifndef MAX_ITEM_OPTIONS
#define MAX_ITEM_OPTIONS 5
#endif
STATIC_ASSERT(MAX_ITEM_OPTIONS <= 5, "This value is limited by the client and database layout and should only be increased if you know the consequences.");

// RoDEX
#define RODEX_TITLE_LENGTH (40 + 1)
#define RODEX_BODY_LENGTH (500 + 1)
#define RODEX_MAX_ITEM (5)
#define RODEX_EXPIRE (1 * 15 * 24 * 60 * 60)
#if PACKETVER >= 20170419
#define RODEX_MAIL_PER_PAGE 32
#else
#define RODEX_MAIL_PER_PAGE 7
#endif

#ifndef MAX_GRADE_MATERIALS
#define MAX_GRADE_MATERIALS 3
#endif

// The following system marks a different job ID system used by the map server,
// which makes a lot more sense than the normal one. [Skotlex]
// These marks the "level" of the job.
#define JOBL_2_1   0x0100
#define JOBL_2_2   0x0200
#define JOBL_2     0x0300 // JOBL_2_1 | JOBL_2_2
#define JOBL_UPPER 0x1000
#define JOBL_BABY  0x2000
#define JOBL_THIRD 0x4000

#define SCRIPT_VARNAME_LENGTH 32 ///< Maximum length of a script variable's name including affixes and excluding NULL-terminator.
STATIC_ASSERT(SCRIPT_VARNAME_LENGTH <= 32, "This value is limited by the inter-server communication and database layout and should only be increased if you know the consequences.");
#define SCRIPT_STRING_VAR_LENGTH 255 ///< Maximum length of strings stored in script variables excluding NULL-terminator.
STATIC_ASSERT(SCRIPT_STRING_VAR_LENGTH <= 255, "This value is limited by the inter-server communication and database layout and should only be increased if you know the consequences.");

#define INFINITE_DURATION (-1) // Infinite duration for status changes

struct hplugin_data_store;
struct script_code;

enum item_types {
	IT_HEALING = 0,
	IT_UNKNOWN, //1
	IT_USABLE,  //2
	IT_ETC,     //3
	IT_WEAPON,  //4
	IT_ARMOR,   //5
	IT_CARD,    //6
	IT_PETEGG,  //7
	IT_PETARMOR,//8
	IT_UNKNOWN2,//9
	IT_AMMO,    //10
	IT_DELAYCONSUME,//11
	IT_CASH = 18,
#ifndef IT_MAX
	IT_MAX
#endif
};

#define INDEX_NOT_FOUND (-1) ///< Used as invalid/failure value in various functions that return an index

// Questlog states
enum quest_state {
	Q_INACTIVE, ///< Inactive quest (the user can toggle between active and inactive quests)
	Q_ACTIVE,   ///< Active quest
	Q_COMPLETE, ///< Completed quest
};

/// Questlog entry
struct quest {
	int quest_id;                    ///< Quest ID
	unsigned int time;               ///< Expiration time
	int count[MAX_QUEST_OBJECTIVES]; ///< Kill counters of each quest objective
	enum quest_state state;          ///< Current quest state
};

enum attribute_flag {
	ATTR_NONE   = 0,
	ATTR_BROKEN = 1,
};

struct item_option {
	int16 index;
	int16 value;
	uint8 param;
};

struct item {
	int id;
	int nameid;
	short amount;
	unsigned int equip; // Location(s) where item is equipped (using enum equip_pos for bitmasking).
	char identify;
	char refine;
	char grade;
	char attribute;
	int card[MAX_SLOTS];
	unsigned int expire_time;
	char favorite;
	unsigned char bound;
	uint64 unique_id;
	struct item_option option[MAX_ITEM_OPTIONS];
};

/**
 * Prevents @ref MAX_STORAGE from causing oversized 0x3011 inter-server packets.
 *
 * @attention If the size of packet 0x3011 changes, this assertion check needs to be adjusted, too.
 *
 * @see intif_send_account_storage() @n
 *      mapif_parse_AccountStorageSave()
 *
 * @anchor MAX_STORAGE_ASSERT
 *
 **/
STATIC_ASSERT(MAX_STORAGE * sizeof(struct item) + 8 <= 0xFFFF, "The maximum amount of item slots per account storage is limited by the inter-server communication layout. Use a smaller value!");

//Equip position constants
enum equip_pos {
	EQP_NONE               = 0x000000,
	EQP_HEAD_LOW           = 0x000001,
	EQP_HEAD_MID           = 0x000200, //512
	EQP_HEAD_TOP           = 0x000100, //256
	EQP_HAND_R             = 0x000002, //2
	EQP_HAND_L             = 0x000020, //32
	EQP_ARMOR              = 0x000010, //16
	EQP_SHOES              = 0x000040, //64
	EQP_GARMENT            = 0x000004, //4
	EQP_ACC_L              = 0x000008, //8
	EQP_ACC_R              = 0x000080, //128
	EQP_COSTUME_HEAD_TOP   = 0x000400, //1024
	EQP_COSTUME_HEAD_MID   = 0x000800, //2048
	EQP_COSTUME_HEAD_LOW   = 0x001000, //4096
	EQP_COSTUME_GARMENT    = 0x002000, //8192
	//UNUSED_COSTUME_FLOOR = 0x004000, //16384
	EQP_AMMO               = 0x008000, //32768
	EQP_SHADOW_ARMOR       = 0x010000, //65536
	EQP_SHADOW_WEAPON      = 0x020000, //131072
	EQP_SHADOW_SHIELD      = 0x040000, //262144
	EQP_SHADOW_SHOES       = 0x080000, //524288
	EQP_SHADOW_ACC_R       = 0x100000, //1048576
	EQP_SHADOW_ACC_L       = 0x200000, //2097152
};

struct point {
	unsigned short map;
	int16 x, y;
};

enum e_skill_flag
{
	SKILL_FLAG_PERMANENT,
	SKILL_FLAG_TEMPORARY,
	SKILL_FLAG_PLAGIARIZED,
	SKILL_FLAG_UNUSED,       ///< needed to maintain the order since the values are saved, can be renamed and used if a new flag is necessary
	SKILL_FLAG_PERM_GRANTED, ///< Permanent, granted through someway (e.g. script).
	/* */
	/* MUST be the last, because with it the flag value stores a dynamic value (flag+lv) */
	SKILL_FLAG_REPLACED_LV_0,   // Temporary skill overshadowing permanent skill of level 'N - SKILL_FLAG_REPLACED_LV_0',
};

enum e_mmo_charstatus_opt {
	OPT_NONE        = 0x0,
	OPT_SHOW_EQUIP  = 0x1,
	OPT_ALLOW_PARTY = 0x2,
	OPT_ALLOW_CALL  = 0x4,
};

enum e_item_bound_type {
	IBT_NONE      = 0x0,
	IBT_MIN       = 0x1,
	IBT_ACCOUNT   = 0x1,
	IBT_GUILD     = 0x2,
	IBT_PARTY     = 0x3,
	IBT_CHARACTER = 0x4,
#ifndef IBT_MAX
	IBT_MAX       = 0x4,
#endif
};

enum e_option {
	OPTION_NOTHING      = 0x00000000,
	OPTION_SIGHT        = 0x00000001,
	OPTION_HIDE         = 0x00000002,
	OPTION_CLOAK        = 0x00000004,
	OPTION_FALCON       = 0x00000010,
	OPTION_RIDING       = 0x00000020,
	OPTION_INVISIBLE    = 0x00000040,
	OPTION_ORCISH       = 0x00000800,
	OPTION_WEDDING      = 0x00001000,
	OPTION_RUWACH       = 0x00002000,
	OPTION_CHASEWALK    = 0x00004000,
	OPTION_FLYING       = 0x00008000, //Note that clientside Flying and Xmas are 0x8000 for clients prior to 2007.
	OPTION_XMAS         = 0x00010000,
	OPTION_TRANSFORM    = 0x00020000,
	OPTION_SUMMER       = 0x00040000,
	OPTION_DRAGON1      = 0x00080000,
	OPTION_WUG          = 0x00100000,
	OPTION_WUGRIDER     = 0x00200000,
	OPTION_MADOGEAR     = 0x00400000,
	OPTION_DRAGON2      = 0x00800000,
	OPTION_DRAGON3      = 0x01000000,
	OPTION_DRAGON4      = 0x02000000,
	OPTION_DRAGON5      = 0x04000000,
	OPTION_HANBOK       = 0x08000000,
	OPTION_OKTOBERFEST  = 0x10000000,
	OPTION_SUMMER2      = 0x20000000,
#ifndef NEW_CARTS
	OPTION_CART1     = 0x00000008,
	OPTION_CART2     = 0x00000080,
	OPTION_CART3     = 0x00000100,
	OPTION_CART4     = 0x00000200,
	OPTION_CART5     = 0x00000400,
	/*  compound constant for older carts */
	OPTION_CART      = OPTION_CART1|OPTION_CART2|OPTION_CART3|OPTION_CART4|OPTION_CART5,
#endif
	// compound constants
	OPTION_DRAGON    = OPTION_DRAGON1|OPTION_DRAGON2|OPTION_DRAGON3|OPTION_DRAGON4|OPTION_DRAGON5,
	OPTION_COSTUME   = OPTION_WEDDING | OPTION_XMAS | OPTION_SUMMER | OPTION_HANBOK | OPTION_OKTOBERFEST | OPTION_SUMMER2,
};

struct s_skill {
	unsigned short id;
	unsigned char lv;
	unsigned char flag; // See enum e_skill_flag
};

struct script_reg_state {
	unsigned int type   : 1;/* because I'm a memory hoarder and having them in the same struct would be a 8-byte/instance waste while ints outnumber str on a 10000-to-1 ratio. */
	unsigned int update : 1;/* whether it needs to be sent to char server for insertion/update/delete */
};

struct script_reg_num {
	struct script_reg_state flag;
	int value;
};

struct script_reg_str {
	struct script_reg_state flag;
	char *value;
};

/// For saving status changes across sessions. [Skotlex]
struct status_change_data {
	unsigned short type;        ///< Status change type (@see enum sc_type)
	int val1, val2, val3, val4; ///< Parameters (meaning depends on type).
	int tick;                   ///< Remaining duration.
	int total_tick;             ///< Total duration.
};

struct storage_data {
	bool save;                     ///< save flag.
	bool received;                 ///< received flag.
	int aggregate;                 ///< total item count.
	VECTOR_DECL(struct item) item; ///< item vector.
};

struct guild_storage {
	int guild_id; ///< Owner guild ID
	bool in_use;  ///< Whether storage is in use by other guild members
	bool dirty;   ///< Whether the struct was modified and needs to be saved
	bool locked;  ///< Whenever item retrieval is happening and the storage can't be accessed
	struct {
		int amount;        ///< Currently stored items (Note: the array is not compacted!)
		int capacity;      ///< Current size of the data array
		struct item *data; ///< Data array
	} items;      ///< Items
};

/**
 * Prevents @ref MAX_GUILD_STORAGE from causing oversized 0x3019 inter-server packets.
 *
 * @attention If the size of packet 0x3019 changes, this assertion check needs to be adjusted, too.
 *
 * @see intif_send_guild_storage() @n
 *      mapif_parse_SaveGuildStorage()
 *
 * @anchor MAX_GUILD_STORAGE_ASSERT
 *
 **/
STATIC_ASSERT(20 + sizeof(struct item) * MAX_GUILD_STORAGE <= 0xFFFF, "The maximum amount of item slots per guild storage is limited by the inter-server communication layout. Use a smaller value!");

struct s_pet {
	int account_id;
	int char_id;
	int pet_id;
	int class_;
	short level;
	int egg_id;//pet egg id
	int equip;//pet equip name_id
	short intimate;//pet friendly
	short hungry;//pet hungry
	char name[NAME_LENGTH];
	char rename_flag;
	char incubate;
	int autofeed;
};

struct s_homunculus { //[orn]
	char name[NAME_LENGTH];
	int hom_id;
	int char_id;
	int class_;
	int prev_class;
	int hp,max_hp,sp,max_sp;
	unsigned int intimacy;
	short hunger;
	struct s_skill hskill[MAX_HOMUNSKILL]; //albator
	short skillpts;
	short level;
	uint64 exp;
	short rename_flag;
	short vaporize; //albator
	int str;
	int agi;
	int vit;
	int int_;
	int dex;
	int luk;

	int str_value;
	int agi_value;
	int vit_value;
	int int_value;
	int dex_value;
	int luk_value;

	int8 spiritball; //for homun S [lighta]
	int autofeed;
};

struct s_mercenary {
	int mercenary_id;
	int char_id;
	int class_;
	int hp, sp;
	unsigned int kill_count;
	unsigned int life_time;
};

struct s_elemental {
	int elemental_id;
	int char_id;
	int class_;
	uint32 mode;
	int hp, sp, max_hp, max_sp, matk, atk, atk2;
	short hit, flee, amotion, def, mdef;
	int life_time;
};

struct s_friend {
	int account_id;
	int char_id;
	char name[NAME_LENGTH];
};

struct hotkey {
#ifdef HOTKEY_SAVING
	unsigned int id;
	unsigned short lv;
	unsigned char type; // 0: item, 1: skill
#else // not HOTKEY_SAVING
	UNAVAILABLE_STRUCT;
#endif
};

struct achievement { // Achievements [Smokexyz/Hercules]
	int id;
	int objective[MAX_ACHIEVEMENT_OBJECTIVES];
	time_t completed_at, rewarded_at;
};

VECTOR_STRUCT_DECL(char_achievements, struct achievement);

struct mmo_charstatus {
	int char_id;
	int account_id;
	int partner_id;
	int father;
	int mother;
	int child;

	uint64 base_exp, job_exp;
	int zeny;
	int bank_vault;

	int class;
	int status_point, skill_point;
	int hp,max_hp,sp,max_sp;
	unsigned int option;
	short manner; // Defines how many minutes a char will be muted, each negative point is equivalent to a minute.
	unsigned char karma;
	short hair, hair_color, clothes_color;
	int body;
	int party_id,guild_id,clan_id,pet_id,hom_id,mer_id,ele_id;
	int fame;

	// 4th job basic stats. client using only char but here we save as int [4144]
	int pow;
	int sta;
	int wis;
	int spl;
	int con;
	int crt;

	// Mercenary Guilds Rank
	int arch_faith, arch_calls;
	int spear_faith, spear_calls;
	int sword_faith, sword_calls;

	struct {
		int weapon;      ///< Weapon view sprite id.
		int shield;      ///< Shield view sprite id.
		int head_top;    ///< Top headgear view sprite id.
		int head_mid;    ///< Middle headgear view sprite id.
		int head_bottom; ///< Bottom headgear view sprite id.
		int robe;        ///< Robe view sprite id.
	} look;

	char name[NAME_LENGTH];
	int base_level, job_level;
	short str,agi,vit,int_,dex,luk;
	unsigned char slot,sex;

	uint32 mapip;
	uint16 mapport;

	int64 last_login;
	struct point last_point,save_point,memo_point[MAX_MEMOPOINTS];
	int inventorySize;
	struct item inventory[MAX_INVENTORY],cart[MAX_CART];
	struct s_skill skill[MAX_SKILL_DB];

	struct s_friend friends[MAX_FRIENDS]; //New friend system [Skotlex]
#ifdef HOTKEY_SAVING
	struct hotkey hotkeys[MAX_HOTKEYS_DB];
#endif
	bool show_equip;
	bool allow_party;
	bool allow_call;
	unsigned short rename;
	unsigned short slotchange;

	time_t delete_date;

	/* `account_data` modifiers */
	unsigned short mod_exp,mod_drop,mod_death;

	unsigned char font;

	uint32 uniqueitem_counter;

	int64 attendance_timer;
	short attendance_count;

	unsigned char hotkey_rowshift;
	unsigned char hotkey_rowshift2;

	int32 title_id; // Achievement Title[Dastgir/Hercules]
};

typedef enum mail_status {
	MAIL_NEW,
	MAIL_UNREAD,
	MAIL_READ,
} mail_status;

struct mail_message {
	int id;
	int send_id;
	char send_name[NAME_LENGTH];
	int dest_id;
	char dest_name[NAME_LENGTH];
	char title[MAIL_TITLE_LENGTH];
	char body[MAIL_BODY_LENGTH];

	mail_status status;
	time_t timestamp; // marks when the message was sent

	int zeny;
	struct item item;
};

struct mail_data {
	short amount;
	bool full;
	short unchecked, unread;
	struct mail_message msg[MAIL_MAX_INBOX];
};

struct auction_data {
	unsigned int auction_id;
	int seller_id;
	char seller_name[NAME_LENGTH];
	int buyer_id;
	char buyer_name[NAME_LENGTH];

	struct item item;
	// This data is required for searching, as itemdb is not read by char server
	char item_name[ITEM_NAME_LENGTH];
	short type;

	unsigned short hours;
	int price, buynow;
	time_t timestamp; // auction's end time
	int auction_end_timer;
};

struct party_member {
	int account_id;
	int char_id;
	char name[NAME_LENGTH];
	int class;
	int lv;
	unsigned short map;
	unsigned leader : 1,
	         online : 1;
};

struct party {
	int party_id;
	char name[NAME_LENGTH];
	unsigned char count; //Count of online characters.
	unsigned exp : 1,
				item : 2; //&1: Party-Share (round-robin), &2: pickup style: shared.
	struct party_member member[MAX_PARTY];
};

struct map_session_data;
struct guild_member {
	int account_id, char_id;
	short hair,hair_color,gender;
	int class;
	short lv;
	uint64 exp;
	int exp_payper;
	short online,position;
	char name[NAME_LENGTH];
	struct map_session_data *sd;
	unsigned char modified;
	uint32 last_login;
};

struct guild_position {
	char name[NAME_LENGTH];
	int mode;
	int exp_mode;
	unsigned char modified;
};

struct guild_alliance {
	int opposition;
	int guild_id;
	char name[NAME_LENGTH];
};

struct guild_expulsion {
	char name[NAME_LENGTH];
	char mes[40];
	int account_id;
	int char_id;
};

struct guild_skill {
	int id,lv;
};

struct channel_data;
struct guild {
	int guild_id;                                         ///< Guild's unique identifier
	int16 guild_lv;                                       ///< Guild level
	int16 connect_member;                                 ///< Current online members
	int16 max_member;                                     ///< Total guild member slots
	int16 average_lv;                                     ///< Average level of guild members
	int16 max_storage;                                    ///< Current guild storage capacity (note: NOT the same as gstor->items.capacity)
	uint64 exp;                                           ///< Current guild experience
	unsigned int next_exp;                                ///< Experience needed for the next level
	int skill_point;                                      ///< Available skill points
	char name[NAME_LENGTH];                               ///< Guild name
	char master[NAME_LENGTH];                             ///< Guild leader's name
	struct guild_member member[MAX_GUILD];                ///< Guild members data
	struct guild_position position[MAX_GUILDPOSITION];    ///< Guild positions data
	char mes1[MAX_GUILDMES1];                             ///< Guild message (first line)
	char mes2[MAX_GUILDMES2];                             ///< Guild message (second line)
	int emblem_id;                                        ///< Sequential ID of the current emblem
	int emblem_len;                                       ///< Guild emblem data length
	char emblem_data[2048];                               ///< Guild emblem data
	struct guild_alliance alliance[MAX_GUILDALLIANCE];    ///< Guild alliances data
	struct guild_expulsion expulsion[MAX_GUILDEXPULSION]; ///< Guild expulsion records
	struct guild_skill skill[MAX_GUILDSKILL];             ///< Guild skills data

	unsigned short save_flag; ///< Flag used in char.c to state what kind of data is being saved/processed

	short *instance;                                      ///< Array of instances
	unsigned short instances;                             ///< Amount of instances

	struct channel_data *channel;                         ///< Guild's `#ally` channel
	struct hplugin_data_store *hdata;                     ///< HPM Plugin Data Store
};

struct guild_castle {
	int castle_id;
	int mapindex;
	char castle_name[NAME_LENGTH];
	char castle_event[NAME_LENGTH];
	int siege_type;
	bool enable_client_warp;
	struct {
		int x;
		int y;
		int zeny;
		int zeny_siege;
	} client_warp;
	int guild_id;
	int economy;
	int defense;
	int triggerE;
	int triggerD;
	int nextTime;
	int payTime;
	int createTime;
	int visibleC;
	struct {
		unsigned visible : 1;
		int id; // object id
	} guardian[MAX_GUARDIANS];
	int* temp_guardians; // ids of temporary guardians (mobs)
	int temp_guardians_max;
};

/**
 * Clan Member Struct
 */
struct clan_member {
	int char_id; ///< Char Id of member
	short online; ///< Flag to know if the player is online or not
	int64 last_login; ///< Last login of this member, used to kick if inactive for long time
	struct map_session_data *sd; ///< Player data of member
};

/**
 * Clan Buff Struct
 */
struct clan_buff {
	int icon; ///< Status Icon to be shown in client (Use one of the 'SI_' constants)
	struct script_code *script; ///< The script to be executed as CLan Buff
};

/**
 * Clan Relationship Struct
 */
struct clan_relationship {
	char constant[NAME_LENGTH]; ///< Unique name of the related clan
	int clan_id; ///< Id of the related clan
};

/**
 * Clan Struct
 */
struct clan {
	int clan_id; ///< CLan's Id
	char constant[NAME_LENGTH]; ///< Clan's Unique Name
	char name[NAME_LENGTH]; ///< Clan's Name
	char master[NAME_LENGTH]; ///< Name of the clan's master (used for clan information on client)
	char map[MAP_NAME_LENGTH_EXT]; ///< The map of that clan (used for clan information on client)
	struct clan_buff buff; ///< The buff for a clan when a member joins it
	short max_member; ///< Limit of Members
	short member_count; ///< Holds the amount of members in this clan, online and offline
	short connect_member; ///< Members that are Online
	VECTOR_DECL(struct clan_member) members; ///< Vector of Members
	VECTOR_DECL(struct clan_relationship) allies; ///< Vector of Allies
	VECTOR_DECL(struct clan_relationship) antagonists; ///< Vector of Antagonists
	int kick_time; /// Needed inactive time to be kicked
	int check_time; ///< Interval to check for inactive players
	int tid; ///< Timer ID for inactivity kick
	bool received; ///< Whether or not the requested data was received
	int req_state; ///< Flag for knowing what to do after receiving the data from inter server
	int req_count_tid; ///< Timer ID for the timer that handles the timeout of requests for interserver to count members
	int req_kick_tid; ///< Timer ID for the timer that handles the timeout of requests for interserver to kick inactive members

	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};

struct fame_list {
	int id;
	int fame;
	char name[NAME_LENGTH];
};

enum fame_list_type {
	RANKTYPE_UNKNOWN    = -1,
	RANKTYPE_BLACKSMITH = 0,
	RANKTYPE_ALCHEMIST  = 1,
	RANKTYPE_TAEKWON    = 2,
	RANKTYPE_PK         = 3, //Not supported yet
};

struct rodex_item {
	struct item item;
	// inventory idx when composing a message (-1 if not filled)
	int idx;
};

struct rodex_message {
	int64 id;
	int sender_id;
	char sender_name[NAME_LENGTH];
	int receiver_id;
	int receiver_accountid;
	char receiver_name[NAME_LENGTH];
	char title[RODEX_TITLE_LENGTH];
	char body[RODEX_BODY_LENGTH];
	struct rodex_item items[RODEX_MAX_ITEM];
	int64 zeny;
	uint8 type;
	int8 opentype;
	bool is_read;
	bool sender_read;
	bool is_deleted;
	int send_date;
	int expire_date;
	int weight;
	int items_count;
};

VECTOR_STRUCT_DECL(rodex_maillist, struct rodex_message);

enum rodex_opentype {
	RODEX_OPENTYPE_MAIL = 0,
	RODEX_OPENTYPE_ACCOUNT = 1,
	RODEX_OPENTYPE_RETURN = 2,
	RODEX_OPENTYPE_UNSET = 3,
};

enum MAIL_TYPE {
	MAIL_TYPE_TEXT = 0x0,
	MAIL_TYPE_ZENY = 0x2,
	MAIL_TYPE_ITEM = 0x4,
	MAIL_TYPE_NPC = 0x8
};

/**
 * Guild Basic Information
 * It is used to request changes via intif_guild_change_basicinfo in map-server and to
 * signalize changes made in char-server via mapif_parse_GuildMemberInfoChange
 **/
enum guild_basic_info {
	GBI_EXP = 1,    ///< Guild Experience (EXP)
	GBI_GUILDLV,    ///< Guild level
	GBI_SKILLPOINT, ///< Guild skillpoints

	/**
	 * Changes a skill level, struct guild_skill should be sent.
	 * All checks regarding max skill level should be done in _map-server_
	 **/
	GBI_SKILLLV,    ///< Guild skill_lv
};

enum guild_member_info { //Change Member Infos
	GMI_POSITION = 0,
	GMI_EXP,
	GMI_HAIR,
	GMI_HAIR_COLOR,
	GMI_GENDER,
	GMI_CLASS,
	GMI_LEVEL,
};

enum guild_permission { // Guild permissions
	GPERM_INVITE  = 0x001,
	GPERM_EXPEL   = 0x010,
#if PACKETVER >= 20140205
	GPERM_STORAGE = 0x100,
	GPERM_ALL     = GPERM_INVITE|GPERM_EXPEL|GPERM_STORAGE,
#else
	GPERM_ALL     = GPERM_INVITE|GPERM_EXPEL,
#endif
	GPERM_MASK    = GPERM_ALL,
	GPERM_DEFAULT = GPERM_ALL,
};

enum {
	GD_SKILLBASE=10000,
	GD_APPROVAL=10000,
	GD_KAFRACONTRACT=10001,
	GD_GUARDRESEARCH=10002,
	GD_GUARDUP=10003,
	GD_EXTENSION=10004,
	GD_GLORYGUILD=10005,
	GD_LEADERSHIP=10006,
	GD_GLORYWOUNDS=10007,
	GD_SOULCOLD=10008,
	GD_HAWKEYES=10009,
	GD_BATTLEORDER=10010,
	GD_REGENERATION=10011,
	GD_RESTORE=10012,
	GD_EMERGENCYCALL=10013,
	GD_DEVELOPMENT=10014,
	GD_ITEMEMERGENCYCALL = 10015,
	GD_GUILD_STORAGE = 10016,
#ifndef GD_MAX
	GD_MAX,
#endif
};

#define ENUM_VALUE(name, id) JOB_ ## name = id,
//These mark the ID of the jobs, as expected by the client. [Skotlex]
enum e_class {
#include "common/class_hidden.h"
#include "common/class_special.h"
#include "common/class.h"
#ifndef JOB_MAX
	JOB_MAX,
#endif
};
#undef ENUM_VALUE


//Total number of classes (for data storage)
#define CLASS_COUNT (JOB_MAX - JOB_NOVICE_HIGH + JOB_MAX_BASIC)

enum {
	SEX_FEMALE = 0,
	SEX_MALE,
	SEX_SERVER
};

enum weapon_type {
	W_FIST,      ///< Bare hands
	W_DAGGER,    //1
	W_1HSWORD,   //2
	W_2HSWORD,   //3
	W_1HSPEAR,   //4
	W_2HSPEAR,   //5
	W_1HAXE,     //6
	W_2HAXE,     //7
	W_MACE,      //8
	W_2HMACE,    //9 (unused)
	W_STAFF,     //10
	W_BOW,       //11
	W_KNUCKLE,   //12
	W_MUSICAL,   //13
	W_WHIP,      //14
	W_BOOK,      //15
	W_KATAR,     //16
	W_REVOLVER,  //17
	W_RIFLE,     //18
	W_GATLING,   //19
	W_SHOTGUN,   //20
	W_GRENADE,   //21
	W_HUUMA,     //22
	W_2HSTAFF,   //23
#ifndef MAX_SINGLE_WEAPON_TYPE
	MAX_SINGLE_WEAPON_TYPE,
#endif
	// dual-wield constants
	W_DOUBLE_DD, ///< 2 daggers
	W_DOUBLE_SS, ///< 2 swords
	W_DOUBLE_AA, ///< 2 axes
	W_DOUBLE_DS, ///< dagger + sword
	W_DOUBLE_DA, ///< dagger + axe
	W_DOUBLE_SA, ///< sword + axe
#ifndef MAX_WEAPON_TYPE
	MAX_WEAPON_TYPE,
#endif
};

enum ammo_type {
	A_ARROW        = 1,
	A_DAGGER,      //2
	A_BULLET,      //3
	A_SHELL,       //4
	A_GRENADE,     //5
	A_SHURIKEN,    //6
	A_KUNAI,       //7
	A_CANNONBALL,  //8
	A_THROWWEAPON, //9
	MAX_AMMO_TYPE
};

enum e_char_server_type {
	CST_NORMAL      = 0,
	CST_MAINTENANCE = 1,
	CST_OVER18      = 2,
	CST_PAYING      = 3,
	CST_F2P         = 4,
	CST_MAX,
};

enum e_pc_reg_loading {
	PRL_NONE = 0x0,
	PRL_CHAR = 0x1,
	PRL_ACCL = 0x2,/* local */
	PRL_ACCG = 0x4,/* global */
	PRL_ALL  = 0xFF,
};

/**
 * Values to be used as operation_type in chrif_char_ask_name
 */
enum zh_char_ask_name_type {
	CHAR_ASK_NAME_BLOCK         = 1, // account block
	CHAR_ASK_NAME_BAN           = 2, // account ban
	CHAR_ASK_NAME_UNBLOCK       = 3, // account unblock
	CHAR_ASK_NAME_UNBAN         = 4, // account unban
	CHAR_ASK_NAME_CHANGESEX     = 5, // change sex
	CHAR_ASK_NAME_CHARBAN       = 6, // character ban
	CHAR_ASK_NAME_CHARUNBAN     = 7, // character unban
	CHAR_ASK_NAME_CHANGECHARSEX = 8, // change character sex
};

/**
 * Values to be used as answer in chrig_char_ask_name_answer
 */
enum hz_char_ask_name_answer {
	CHAR_ASK_NAME_ANS_DONE     = 0, // login-server request done
	CHAR_ASK_NAME_ANS_NOTFOUND = 1, // player not found
	CHAR_ASK_NAME_ANS_GMLOW    = 2, // gm level too low
	CHAR_ASK_NAME_ANS_OFFLINE  = 3, // login-server offline
};

/**
 * Quest Info Types
 */
enum questinfo_type {
	QINFO_JOB,
	QINFO_SEX,
	QINFO_BASE_LEVEL,
	QINFO_JOB_LEVEL,
	QINFO_ITEM,
	QINFO_HOMUN_LEVEL,
	QINFO_HOMUN_TYPE,
	QINFO_QUEST,
	QINFO_MERCENARY_CLASS
};

/** Pet hunger level **/
enum e_pet_hunger_level {
	PET_HUNGER_STARVING = 0,
	PET_HUNGER_VERY_HUNGRY = 10,
	PET_HUNGER_HUNGRY = 25,
	PET_HUNGER_NEUTRAL = 75,
	PET_HUNGER_SATISFIED = 90,
	PET_HUNGER_STUFFED = 100
};

/** Pet intimacy level **/
enum e_pet_intimacy_level {
	PET_INTIMACY_NONE = 0,
	PET_INTIMACY_AWKWARD = 1,
	PET_INTIMACY_SHY = 100,
	PET_INTIMACY_NEUTRAL = 250,
	PET_INTIMACY_CORDIAL = 750,
	PET_INTIMACY_LOYAL = 900,
	PET_INTIMACY_MAX = 1000
};

/* packet size constant for itemlist */
#if MAX_INVENTORY > MAX_STORAGE && MAX_INVENTORY > MAX_CART
#define MAX_ITEMLIST MAX_INVENTORY
#elif MAX_CART > MAX_INVENTORY && MAX_CART > MAX_STORAGE
#define MAX_ITEMLIST MAX_CART
#else
#define MAX_ITEMLIST MAX_STORAGE
#endif

#ifndef MAX_REFINE_REQUIREMENTS
	#define MAX_REFINE_REQUIREMENTS 4
#endif

// sanity checks...
#if MAX_ZENY > INT_MAX
#error MAX_ZENY is too big
#endif

#if MAX_SLOTS < 4
#error MAX_SLOTS it too small
#endif

#ifdef MAX_SKILL
#error MAX_SKILL has been replaced by MAX_SKILL_DB. Please update your custom definitions.
#endif

#if MAX_REFINE_REQUIREMENTS > 4
#error MAX_REFINE_REQUIREMENTS is bigger than allowed, this is a hardcoded limit in the client
#endif

#endif /* COMMON_MMO_H */
