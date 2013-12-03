// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef	_MMO_H_
#define	_MMO_H_

#include "cbasetypes.h"
#include "../common/db.h"
#include <time.h>

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
// 20090408 - 2009-04-08aSakexe+   - 0x44a (dont use as it overlaps with RE client packets)
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
#define PACKETVER 20120418
#endif // PACKETVER

#ifndef DISABLE_PACKETVER_RE
// Comment the following line if your client is NOT ragexeRE (required because of conflicting packets in ragexe vs ragexeRE).
#define PACKETVER_RE
#endif // DISABLE_PACKETVER_RE

// Client support for experimental RagexeRE UI present in 2012-04-10 and 2012-04-18
#if defined(PACKETVER_RE) && ( PACKETVER == 20120410 || PACKETVER == 20120418 )
#define	PARTY_RECRUIT
#endif // PACKETVER_RE && (PACKETVER == 20120410 || PACKETVER == 10120418)

// Comment the following line to disable sc_data saving. [Skotlex]
#define ENABLE_SC_SAVING

#if PACKETVER >= 20070227
// Comment the following like to disable server-side hot-key saving support. [Skotlex]
// Note that newer clients no longer save hotkeys in the registry!
#define HOTKEY_SAVING

#if PACKETVER < 20090603
        // (27 = 9 skills x 3 bars)               (0x02b9,191)
        #define MAX_HOTKEYS 27
#elif PACKETVER < 20090617
        // (36 = 9 skills x 4 bars)               (0x07d9,254)
        #define MAX_HOTKEYS 36
#else // >= 20090617
        // (38 = 9 skills x 4 bars & 2 Quickslots)(0x07d9,268)
        #define MAX_HOTKEYS 38
#endif // 20090603
#endif // 20070227

#define MAX_INVENTORY 100
//Max number of characters per account. Note that changing this setting alone is not enough if the client is not hexed to support more characters as well.
#define MAX_CHARS 9
//Number of slots carded equipment can have. Never set to less than 4 as they are also used to keep the data of forged items/equipment. [Skotlex]
//Note: The client seems unable to receive data for more than 4 slots due to all related packets having a fixed size.
#define MAX_SLOTS 4
//Max amount of a single stacked item
#define MAX_AMOUNT 30000
#define MAX_ZENY 1000000000

//Official Limit: 2.1b ( the var that stores the money doesn't go much higher than this by default )
#define MAX_BANK_ZENY 2100000000

#define MAX_FAME 1000000000
#define MAX_CART 100
#define MAX_SKILL 1478
#define MAX_SKILL_ID 10015   // [Ind/Hercules] max used skill ID
//Update this max as necessary. 55 is the value needed for Super Baby currently
//Raised to 84 since Expanded Super Novice needs it.
#define MAX_SKILL_TREE 84
#define GLOBAL_REG_NUM 256   // Max permanent character variables per char
#define ACCOUNT_REG_NUM 64   // Max permanent local account variables per account
#define ACCOUNT_REG2_NUM 16  // Max permanent global account variables per account
//Should hold the max of GLOBAL/ACCOUNT/ACCOUNT2 (needed for some arrays that hold all three)
#define MAX_REG_NUM 256
#define DEFAULT_WALK_SPEED 150
#define MIN_WALK_SPEED 20 /* below 20 clips animation */
#define MAX_WALK_SPEED 1000
#define MAX_STORAGE 600
#define MAX_GUILD_STORAGE 600
#define MAX_PARTY 12
#define MAX_GUILD (16+10*6)     // Increased max guild members +6 per 1 extension levels [Lupus]
#define MAX_GUILDPOSITION 20    // Increased max guild positions to accomodate for all members [Valaris] (removed) [PoW]
#define MAX_GUILDEXPULSION 32
#define MAX_GUILDALLIANCE 16
#define MAX_GUILDSKILL	15      // Increased max guild skills because of new skills [Sara-chan]
#define MAX_GUILDLEVEL 50
#define MAX_GUARDIANS 8         // Local max per castle. [Skotlex]
#define MAX_QUEST_OBJECTIVES 3  // Max quest objectives for a quest
#define MAX_START_ITEMS 32	    // Max number of items allowed to be given to a char whenever it's created. [mkbu95]

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

#define MAX_FRIENDS 40
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
#define HM_SKILLBASE 8001
#define MAX_HOMUNSKILL 43
#define MAX_HOMUNCULUS_CLASS	52	// [orn] Increased to 60 from 16 to allow new Homun-S.
#define HM_CLASS_BASE 6001
#define HM_CLASS_MAX (HM_CLASS_BASE+MAX_HOMUNCULUS_CLASS-1)

// Mail System
#define MAIL_MAX_INBOX 30
#define MAIL_TITLE_LENGTH 40
#define MAIL_BODY_LENGTH 200

// Mercenary System
#define MC_SKILLBASE 8201
#define MAX_MERCSKILL 40
#define MAX_MERCENARY_CLASS 61

// Elemental System
#define MAX_ELEMENTALSKILL 42
#define EL_SKILLBASE 8401
#define MAX_ELESKILLTREE 3
#define MAX_ELEMENTAL_CLASS 12
#define EL_CLASS_BASE 2114
#define EL_CLASS_MAX (EL_CLASS_BASE+MAX_ELEMENTAL_CLASS-1)

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
	IT_MAX
};


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

struct item {
	int id;
	short nameid;
	short amount;
	unsigned int equip; // Location(s) where item is equipped (using enum equip_pos for bitmasking).
	char identify;
	char refine;
	char attribute;
	short card[MAX_SLOTS];
	unsigned int expire_time;
	char favorite;
	unsigned char bound;
	uint64 unique_id;
};

struct point {
	unsigned short map;
	short x,y;
};

enum e_skill_flag
{
	SKILL_FLAG_PERMANENT,
	SKILL_FLAG_TEMPORARY,
	SKILL_FLAG_PLAGIARIZED,
	SKILL_FLAG_UNUSED,			/* needed to maintain the order since the values are saved, can be renamed and used if a new flag is necessary */
	SKILL_FLAG_PERM_GRANTED,    // Permanent, granted through someway (e.g. script).
	/* */
	/* MUST be the last, because with it the flag value stores a dynamic value (flag+lv) */
	SKILL_FLAG_REPLACED_LV_0,   // Temporary skill overshadowing permanent skill of level 'N - SKILL_FLAG_REPLACED_LV_0',
};

enum e_mmo_charstatus_opt {
	OPT_NONE		= 0x0,
	OPT_SHOW_EQUIP	= 0x1,
	OPT_ALLOW_PARTY	= 0x2,
};

enum e_item_bound_type {
	IBT_MIN       = 0x1,
	IBT_ACCOUNT   = 0x1,
	IBT_GUILD     = 0x2,
	IBT_PARTY     = 0x3,
	IBT_CHARACTER = 0x4,
	IBT_MAX       = 0x4,
};

struct s_skill {
	unsigned short id;
	unsigned char lv;
	unsigned char flag; // See enum e_skill_flag
};

struct global_reg {
	char str[32];
	char value[256];
};

// Holds array of global registries, used by the char server and converter.
struct accreg {
	int account_id, char_id;
	int reg_num;
	struct global_reg reg[MAX_REG_NUM];
};

// For saving status changes across sessions. [Skotlex]
struct status_change_data {
	unsigned short type; //SC_type
	long val1, val2, val3, val4, tick; //Remaining duration.
};

struct storage_data {
	int storage_amount;
	struct item items[MAX_STORAGE];
};

struct guild_storage {
	int dirty;
	int guild_id;
	short storage_status;
	short storage_amount;
	struct item items[MAX_GUILD_STORAGE];
	unsigned short lock;
};

struct s_pet {
	int account_id;
	int char_id;
	int pet_id;
	short class_;
	short level;
	short egg_id;//pet egg id
	short equip;//pet equip name_id
	short intimate;//pet friendly
	short hungry;//pet hungry
	char name[NAME_LENGTH];
	char rename_flag;
	char incuvate;
};

struct s_homunculus {	//[orn]
	char name[NAME_LENGTH];
	int hom_id;
	int char_id;
	short class_;
	short prev_class;
	int hp,max_hp,sp,max_sp;
	unsigned int intimacy;	//[orn]
	short hunger;
	struct s_skill hskill[MAX_HOMUNSKILL]; //albator
	short skillpts;
	short level;
	unsigned int exp;
	short rename_flag;
	short vaporize; //albator
	int str ;
	int agi ;
	int vit ;
	int int_ ;
	int dex ;
	int luk ;

	char spiritball; //for homun S [lighta]
};

struct s_mercenary {
	int mercenary_id;
	int char_id;
	short class_;
	int hp, sp;
	unsigned int kill_count;
	unsigned int life_time;
};

struct s_elemental {
	int elemental_id;
	int char_id;
	short class_;
	int mode;
	int hp, sp, max_hp, max_sp, matk, atk, atk2;
	short hit, flee, amotion, def, mdef;
	int life_time;
};

struct s_friend {
	int account_id;
	int char_id;
	char name[NAME_LENGTH];
};

#ifdef HOTKEY_SAVING
struct hotkey {
	unsigned int id;
	unsigned short lv;
	unsigned char type; // 0: item, 1: skill
};
#endif

struct mmo_charstatus {
	int char_id;
	int account_id;
	int partner_id;
	int father;
	int mother;
	int child;

	unsigned int base_exp,job_exp;
	int zeny;
	int bank_vault;

	short class_;
	unsigned int status_point,skill_point;
	int hp,max_hp,sp,max_sp;
	unsigned int option;
	short manner;
	unsigned char karma;
	short hair,hair_color,clothes_color;
	int party_id,guild_id,pet_id,hom_id,mer_id,ele_id;
	int fame;

	// Mercenary Guilds Rank
	int arch_faith, arch_calls;
	int spear_faith, spear_calls;
	int sword_faith, sword_calls;

	short weapon; // enum weapon_type
	short shield; // view-id
	short head_top,head_mid,head_bottom;
	short robe;

	char name[NAME_LENGTH];
	unsigned int base_level,job_level;
	short str,agi,vit,int_,dex,luk;
	unsigned char slot,sex;

	uint32 mapip;
	uint16 mapport;

	struct point last_point,save_point,memo_point[MAX_MEMOPOINTS];
	struct item inventory[MAX_INVENTORY],cart[MAX_CART];
	struct storage_data storage;
	struct s_skill skill[MAX_SKILL];

	struct s_friend friends[MAX_FRIENDS]; //New friend system [Skotlex]
#ifdef HOTKEY_SAVING
	struct hotkey hotkeys[MAX_HOTKEYS];
#endif
	bool show_equip, allow_party;
	unsigned short rename;
	unsigned short slotchange;

	time_t delete_date;

	/* `account_data` modifiers */
	unsigned short mod_exp,mod_drop,mod_death;
	
	unsigned char font;
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

struct registry {
	int global_num;
	struct global_reg global[GLOBAL_REG_NUM];
	int account_num;
	struct global_reg account[ACCOUNT_REG_NUM];
	int account2_num;
	struct global_reg account2[ACCOUNT_REG2_NUM];
};

struct party_member {
	int account_id;
	int char_id;
	char name[NAME_LENGTH];
	unsigned short class_;
	unsigned short map;
	unsigned short lv;
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
	short hair,hair_color,gender,class_,lv;
	uint64 exp;
	int exp_payper;
	short online,position;
	char name[NAME_LENGTH];
	struct map_session_data *sd;
	unsigned char modified;
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
};

struct guild_skill {
	int id,lv;
};

struct guild {
	int guild_id;
	short guild_lv, connect_member, max_member, average_lv;
	uint64 exp;
	unsigned int next_exp;
	int skill_point;
	char name[NAME_LENGTH],master[NAME_LENGTH];
	struct guild_member member[MAX_GUILD];
	struct guild_position position[MAX_GUILDPOSITION];
	char mes1[MAX_GUILDMES1],mes2[MAX_GUILDMES2];
	int emblem_len,emblem_id;
	char emblem_data[2048];
	struct guild_alliance alliance[MAX_GUILDALLIANCE];
	struct guild_expulsion expulsion[MAX_GUILDEXPULSION];
	struct guild_skill skill[MAX_GUILDSKILL];
	
	/* TODO: still used for something?|: */
	unsigned short save_flag; // for TXT saving
	
	short *instance;
	unsigned short instances;
	
	void *channel;
};

struct guild_castle {
	int castle_id;
	int mapindex;
	char castle_name[NAME_LENGTH];
	char castle_event[NAME_LENGTH];
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

struct fame_list {
	int id;
	int fame;
	char name[NAME_LENGTH];
};

enum fame_list_type {
	RANKTYPE_BLACKSMITH = 0,
	RANKTYPE_ALCHEMIST  = 1,
	RANKTYPE_TAEKWON    = 2,
	RANKTYPE_PK         = 3, //Not supported yet
};

enum { //Change Guild Infos
	GBI_EXP	=1,		// Guild Experience (EXP)
	GBI_GUILDLV,		// Guild level
	GBI_SKILLPOINT,		// Guild skillpoints
	GBI_SKILLLV,		// Guild skill_lv ?? seem unused
};

enum { //Change Member Infos
	GMI_POSITION	=0,
	GMI_EXP,
	GMI_HAIR,
	GMI_HAIR_COLOR,
	GMI_GENDER,
	GMI_CLASS,
	GMI_LEVEL,
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
	GD_MAX,
};


//These mark the ID of the jobs, as expected by the client. [Skotlex]
enum {
	JOB_NOVICE,
	JOB_SWORDMAN,
	JOB_MAGE,
	JOB_ARCHER,
	JOB_ACOLYTE,
	JOB_MERCHANT,
	JOB_THIEF,
	JOB_KNIGHT,
	JOB_PRIEST,
	JOB_WIZARD,
	JOB_BLACKSMITH,
	JOB_HUNTER,
	JOB_ASSASSIN,
	JOB_KNIGHT2,
	JOB_CRUSADER,
	JOB_MONK,
	JOB_SAGE,
	JOB_ROGUE,
	JOB_ALCHEMIST,
	JOB_BARD,
	JOB_DANCER,
	JOB_CRUSADER2,
	JOB_WEDDING,
	JOB_SUPER_NOVICE,
	JOB_GUNSLINGER,
	JOB_NINJA,
	JOB_XMAS,
	JOB_SUMMER,
	JOB_MAX_BASIC,

	JOB_NOVICE_HIGH = 4001,
	JOB_SWORDMAN_HIGH,
	JOB_MAGE_HIGH,
	JOB_ARCHER_HIGH,
	JOB_ACOLYTE_HIGH,
	JOB_MERCHANT_HIGH,
	JOB_THIEF_HIGH,
	JOB_LORD_KNIGHT,
	JOB_HIGH_PRIEST,
	JOB_HIGH_WIZARD,
	JOB_WHITESMITH,
	JOB_SNIPER,
	JOB_ASSASSIN_CROSS,
	JOB_LORD_KNIGHT2,
	JOB_PALADIN,
	JOB_CHAMPION,
	JOB_PROFESSOR,
	JOB_STALKER,
	JOB_CREATOR,
	JOB_CLOWN,
	JOB_GYPSY,
	JOB_PALADIN2,

	JOB_BABY,
	JOB_BABY_SWORDMAN,
	JOB_BABY_MAGE,
	JOB_BABY_ARCHER,
	JOB_BABY_ACOLYTE,
	JOB_BABY_MERCHANT,
	JOB_BABY_THIEF,
	JOB_BABY_KNIGHT,
	JOB_BABY_PRIEST,
	JOB_BABY_WIZARD,
	JOB_BABY_BLACKSMITH,
	JOB_BABY_HUNTER,
	JOB_BABY_ASSASSIN,
	JOB_BABY_KNIGHT2,
	JOB_BABY_CRUSADER,
	JOB_BABY_MONK,
	JOB_BABY_SAGE,
	JOB_BABY_ROGUE,
	JOB_BABY_ALCHEMIST,
	JOB_BABY_BARD,
	JOB_BABY_DANCER,
	JOB_BABY_CRUSADER2,
	JOB_SUPER_BABY,

	JOB_TAEKWON,
	JOB_STAR_GLADIATOR,
	JOB_STAR_GLADIATOR2,
	JOB_SOUL_LINKER,

	JOB_GANGSI,
	JOB_DEATH_KNIGHT,
	JOB_DARK_COLLECTOR,

	JOB_RUNE_KNIGHT = 4054,
	JOB_WARLOCK,
	JOB_RANGER,
	JOB_ARCH_BISHOP,
	JOB_MECHANIC,
	JOB_GUILLOTINE_CROSS,

	JOB_RUNE_KNIGHT_T,
	JOB_WARLOCK_T,
	JOB_RANGER_T,
	JOB_ARCH_BISHOP_T,
	JOB_MECHANIC_T,
	JOB_GUILLOTINE_CROSS_T,

	JOB_ROYAL_GUARD,
	JOB_SORCERER,
	JOB_MINSTREL,
	JOB_WANDERER,
	JOB_SURA,
	JOB_GENETIC,
	JOB_SHADOW_CHASER,

	JOB_ROYAL_GUARD_T,
	JOB_SORCERER_T,
	JOB_MINSTREL_T,
	JOB_WANDERER_T,
	JOB_SURA_T,
	JOB_GENETIC_T,
	JOB_SHADOW_CHASER_T,

	JOB_RUNE_KNIGHT2,
	JOB_RUNE_KNIGHT_T2,
	JOB_ROYAL_GUARD2,
	JOB_ROYAL_GUARD_T2,
	JOB_RANGER2,
	JOB_RANGER_T2,
	JOB_MECHANIC2,
	JOB_MECHANIC_T2,

	JOB_BABY_RUNE = 4096,
	JOB_BABY_WARLOCK,
	JOB_BABY_RANGER,
	JOB_BABY_BISHOP,
	JOB_BABY_MECHANIC,
	JOB_BABY_CROSS,

	JOB_BABY_GUARD,
	JOB_BABY_SORCERER,
	JOB_BABY_MINSTREL,
	JOB_BABY_WANDERER,
	JOB_BABY_SURA,
	JOB_BABY_GENETIC,
	JOB_BABY_CHASER,

	JOB_BABY_RUNE2,
	JOB_BABY_GUARD2,
	JOB_BABY_RANGER2,
	JOB_BABY_MECHANIC2,

	JOB_SUPER_NOVICE_E = 4190,
	JOB_SUPER_BABY_E,

	JOB_KAGEROU = 4211,
	JOB_OBORO,
	JOB_REBELLION = 4215,

	JOB_MAX,
};

//Total number of classes (for data storage)
#define CLASS_COUNT (JOB_MAX - JOB_NOVICE_HIGH + JOB_MAX_BASIC)

enum {
	SEX_FEMALE = 0,
	SEX_MALE,
	SEX_SERVER
};

enum weapon_type {
	W_FIST,	//Bare hands
	W_DAGGER,	//1
	W_1HSWORD,	//2
	W_2HSWORD,	//3
	W_1HSPEAR,	//4
	W_2HSPEAR,	//5
	W_1HAXE,	//6
	W_2HAXE,	//7
	W_MACE,	//8
	W_2HMACE,	//9 (unused)
	W_STAFF,	//10
	W_BOW,	//11
	W_KNUCKLE,	//12
	W_MUSICAL,	//13
	W_WHIP,	//14
	W_BOOK,	//15
	W_KATAR,	//16
	W_REVOLVER,	//17
	W_RIFLE,	//18
	W_GATLING,	//19
	W_SHOTGUN,	//20
	W_GRENADE,	//21
	W_HUUMA,	//22
	W_2HSTAFF,	//23
	MAX_WEAPON_TYPE,
	// dual-wield constants
	W_DOUBLE_DD, // 2 daggers
	W_DOUBLE_SS, // 2 swords
	W_DOUBLE_AA, // 2 axes
	W_DOUBLE_DS, // dagger + sword
	W_DOUBLE_DA, // dagger + axe
	W_DOUBLE_SA, // sword + axe
};

enum ammo_type {
	A_ARROW = 1,
	A_DAGGER,   //2
	A_BULLET,   //3
	A_SHELL,    //4
	A_GRENADE,  //5
	A_SHURIKEN, //6
	A_KUNAI,     //7
	A_CANNONBALL,	//8
	A_THROWWEAPON	//9
};

enum e_char_server_type {
	CST_NORMAL      = 0,
	CST_MAINTENANCE = 1,
	CST_OVER18      = 2,
	CST_PAYING      = 3,
	CST_P2P         = 4,
};

/* packet size constant for itemlist */
#if MAX_INVENTORY > MAX_STORAGE && MAX_INVENTORY > MAX_CART
#define MAX_ITEMLIST MAX_INVENTORY
#elif MAX_CART > MAX_INVENTORY && MAX_CART > MAX_STORAGE
#define MAX_ITEMLIST MAX_CART
#else
#define MAX_ITEMLIST MAX_STORAGE
#endif

// sanity checks...
#if MAX_ZENY > INT_MAX
#error MAX_ZENY is too big
#endif

#endif /* _MMO_H_ */
