// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _CLIF_H_
#define _CLIF_H_

#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/cobj.h"
#include <stdarg.h>

/**
 * Declarations
 **/
struct item;
struct item_data;
struct storage_data;
struct guild_storage;
struct block_list;
struct unit_data;
struct map_session_data;
struct homun_data;
struct mercenary_data;
struct pet_data;
struct mob_data;
struct npc_data;
struct chat_data;
struct flooritem_data;
struct skill_unit;
struct s_vending;
struct party;
struct party_data;
struct guild;
struct battleground_data;
struct quest;
struct party_booking_ad_info;
struct view_data;
struct eri;

/**
 * Defines
 **/
#define packet_len(cmd) packet_db[cmd].len
#define clif_menuskill_clear(sd) (sd)->menuskill_id = (sd)->menuskill_val = (sd)->menuskill_val2 = 0;
#define HCHSYS_NAME_LENGTH 20

/**
 * Enumerations
 **/
enum {// packet DB
	MAX_PACKET_DB  = 0xF00,
	MAX_PACKET_POS = 20,
};

typedef enum send_target {
	ALL_CLIENT,
	ALL_SAMEMAP,
	AREA,				// area
	AREA_WOS,			// area, without self
	AREA_WOC,			// area, without chatrooms
	AREA_WOSC,			// area, without own chatroom
	AREA_CHAT_WOC,		// hearable area, without chatrooms
	CHAT,				// current chatroom
	CHAT_WOS,			// current chatroom, without self
	PARTY,
	PARTY_WOS,
	PARTY_SAMEMAP,
	PARTY_SAMEMAP_WOS,
	PARTY_AREA,
	PARTY_AREA_WOS,
	GUILD,
	GUILD_WOS,
	GUILD_SAMEMAP,
	GUILD_SAMEMAP_WOS,
	GUILD_AREA,
	GUILD_AREA_WOS,
	GUILD_NOBG,
	DUEL,
	DUEL_WOS,
	SELF,
	
	BG,					// BattleGround System
	BG_WOS,
	BG_SAMEMAP,
	BG_SAMEMAP_WOS,
	BG_AREA,
	BG_AREA_WOS,
} send_target;

typedef enum emotion_type {
	E_GASP = 0,     // /!
	E_WHAT,         // /?
	E_HO,
	E_LV,
	E_SWT,
	E_IC,
	E_AN,
	E_AG,
	E_CASH,         // /$
	E_DOTS,         // /...
	E_SCISSORS,     // /gawi --- 10
	E_ROCK,         // /bawi
	E_PAPER,        // /bo
	E_KOREA,
	E_LV2,
	E_THX,
	E_WAH,
	E_SRY,
	E_HEH,
	E_SWT2,
	E_HMM,          // --- 20
	E_NO1,
	E_NO,           // /??
	E_OMG,
	E_OH,
	E_X,
	E_HLP,
	E_GO,
	E_SOB,
	E_GG,
	E_KIS,          // --- 30
	E_KIS2,
	E_PIF,
	E_OK,
	E_MUTE,         // red /... used for muted characters
	E_INDONESIA,
	E_BZZ,          // /bzz, /stare
	E_RICE,
	E_AWSM,         // /awsm, /cool
	E_MEH,
	E_SHY,          // --- 40
	E_PAT,          // /pat, /goodboy
	E_MP,           // /mp, /sptime
	E_SLUR,
	E_COM,          // /com, /comeon
	E_YAWN,         // /yawn, /sleepy
	E_GRAT,         // /grat, /congrats
	E_HP,           // /hp, /hptime
	E_PHILIPPINES,
	E_MALAYSIA,
	E_SINGAPORE,    // --- 50
	E_BRAZIL,
	E_FLASH,        // /fsh
	E_SPIN,         // /spin
	E_SIGH,
	E_DUM,          // /dum
	E_LOUD,         // /crwd
	E_OTL,          // /otl, /desp
	E_DICE1,
	E_DICE2,
	E_DICE3,        // --- 60
	E_DICE4,
	E_DICE5,
	E_DICE6,
	E_INDIA,
	E_LUV,          // /love
	E_RUSSIA,
	E_VIRGIN,
	E_MOBILE,
	E_MAIL,
	E_CHINESE,      // --- 70
	E_ANTENNA1,
	E_ANTENNA2,
	E_ANTENNA3,
	E_HUM,
	E_ABS,
	E_OOPS,
	E_SPIT,
	E_ENE,
	E_PANIC,
	E_WHISP,        // --- 80
	E_YUT1,
	E_YUT2,
	E_YUT3,
	E_YUT4,
	E_YUT5,
	E_YUT6,
	E_YUT7,
	/* ... */
	E_MAX
} emotion_type;

typedef enum clr_type {
	CLR_OUTSIGHT = 0,
	CLR_DEAD,
	CLR_RESPAWN,
	CLR_TELEPORT,
	CLR_TRICKDEAD,
} clr_type;

enum map_property { // clif_map_property
	MAPPROPERTY_NOTHING       = 0,
	MAPPROPERTY_FREEPVPZONE   = 1,
	MAPPROPERTY_EVENTPVPZONE  = 2,
	MAPPROPERTY_AGITZONE      = 3,
	MAPPROPERTY_PKSERVERZONE  = 4, // message "You are in a PK area. Please beware of sudden attacks." in color 0x9B9BFF (light red)
	MAPPROPERTY_PVPSERVERZONE = 5,
	MAPPROPERTY_DENYSKILLZONE = 6,
};

enum map_type { // clif_map_type
	MAPTYPE_VILLAGE              = 0,
	MAPTYPE_VILLAGE_IN           = 1,
	MAPTYPE_FIELD                = 2,
	MAPTYPE_DUNGEON              = 3,
	MAPTYPE_ARENA                = 4,
	MAPTYPE_PENALTY_FREEPKZONE   = 5,
	MAPTYPE_NOPENALTY_FREEPKZONE = 6,
	MAPTYPE_EVENT_GUILDWAR       = 7,
	MAPTYPE_AGIT                 = 8,
	MAPTYPE_DUNGEON2             = 9,
	MAPTYPE_DUNGEON3             = 10,
	MAPTYPE_PKSERVER             = 11,
	MAPTYPE_PVPSERVER            = 12,
	MAPTYPE_DENYSKILL            = 13,
	MAPTYPE_TURBOTRACK           = 14,
	MAPTYPE_JAIL                 = 15,
	MAPTYPE_MONSTERTRACK         = 16,
	MAPTYPE_PORINGBATTLE         = 17,
	MAPTYPE_AGIT_SIEGEV15        = 18,
	MAPTYPE_BATTLEFIELD          = 19,
	MAPTYPE_PVP_TOURNAMENT       = 20,
	//Map types 21 - 24 not used.
	MAPTYPE_SIEGE_LOWLEVEL       = 25,
	//Map types 26 - 28 remains opens for future types.
	MAPTYPE_UNUSED               = 29,
};

enum useskill_fail_cause { // clif_skill_fail
	USESKILL_FAIL_LEVEL = 0,
	USESKILL_FAIL_SP_INSUFFICIENT = 1,
	USESKILL_FAIL_HP_INSUFFICIENT = 2,
	USESKILL_FAIL_STUFF_INSUFFICIENT = 3,
	USESKILL_FAIL_SKILLINTERVAL = 4,
	USESKILL_FAIL_MONEY = 5,
	USESKILL_FAIL_THIS_WEAPON = 6,
	USESKILL_FAIL_REDJAMSTONE = 7,
	USESKILL_FAIL_BLUEJAMSTONE = 8,
	USESKILL_FAIL_WEIGHTOVER = 9,
	USESKILL_FAIL = 10,
	USESKILL_FAIL_TOTARGET = 11,
	USESKILL_FAIL_ANCILLA_NUMOVER = 12,
	USESKILL_FAIL_HOLYWATER = 13,
	USESKILL_FAIL_ANCILLA = 14,
	USESKILL_FAIL_DUPLICATE_RANGEIN = 15,
	USESKILL_FAIL_NEED_OTHER_SKILL = 16,
	USESKILL_FAIL_NEED_HELPER = 17,
	USESKILL_FAIL_INVALID_DIR = 18,
	USESKILL_FAIL_SUMMON = 19,
	USESKILL_FAIL_SUMMON_NONE = 20,
	USESKILL_FAIL_IMITATION_SKILL_NONE = 21,
	USESKILL_FAIL_DUPLICATE = 22,
	USESKILL_FAIL_CONDITION = 23,
	USESKILL_FAIL_PAINTBRUSH = 24,
	USESKILL_FAIL_DRAGON = 25,
	USESKILL_FAIL_POS = 26,
	USESKILL_FAIL_HELPER_SP_INSUFFICIENT = 27,
	USESKILL_FAIL_NEER_WALL = 28,
	USESKILL_FAIL_NEED_EXP_1PERCENT = 29,
	USESKILL_FAIL_CHORUS_SP_INSUFFICIENT = 30,
	USESKILL_FAIL_GC_WEAPONBLOCKING = 31,
	USESKILL_FAIL_GC_POISONINGWEAPON = 32,
	USESKILL_FAIL_MADOGEAR = 33,
	USESKILL_FAIL_NEED_EQUIPMENT_KUNAI = 34,
	USESKILL_FAIL_TOTARGET_PLAYER = 35,
	USESKILL_FAIL_SIZE = 36,
	USESKILL_FAIL_CANONBALL = 37,
	//XXX_USESKILL_FAIL_II_MADOGEAR_ACCELERATION = 38,
	//XXX_USESKILL_FAIL_II_MADOGEAR_HOVERING_BOOSTER = 39,
	USESKILL_FAIL_MADOGEAR_HOVERING = 40,
	//XXX_USESKILL_FAIL_II_MADOGEAR_SELFDESTRUCTION_DEVICE = 41,
	//XXX_USESKILL_FAIL_II_MADOGEAR_SHAPESHIFTER = 42,
	USESKILL_FAIL_GUILLONTINE_POISON = 43,
	//XXX_USESKILL_FAIL_II_MADOGEAR_COOLING_DEVICE = 44,
	//XXX_USESKILL_FAIL_II_MADOGEAR_MAGNETICFIELD_GENERATOR = 45,
	//XXX_USESKILL_FAIL_II_MADOGEAR_BARRIER_GENERATOR = 46,
	//XXX_USESKILL_FAIL_II_MADOGEAR_OPTICALCAMOUFLAGE_GENERATOR = 47,
	//XXX_USESKILL_FAIL_II_MADOGEAR_REPAIRKIT = 48,
	//XXX_USESKILL_FAIL_II_MONKEY_SPANNER = 49,
	USESKILL_FAIL_MADOGEAR_RIDE = 50,
	USESKILL_FAIL_SPELLBOOK = 51,
	USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP = 52,
	USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT = 53,
	USESKILL_FAIL_SPELLBOOK_READING = 54,
	//XXX_USESKILL_FAIL_II_FACE_PAINTS = 55,
	//XXX_USESKILL_FAIL_II_MAKEUP_BRUSH = 56,
	USESKILL_FAIL_CART = 57,
	//XXX_USESKILL_FAIL_II_THORNS_SEED = 58,
	//XXX_USESKILL_FAIL_II_BLOOD_SUCKER_SEED = 59,
	USESKILL_FAIL_NO_MORE_SPELL = 60,
	//XXX_USESKILL_FAIL_II_BOMB_MUSHROOM_SPORE = 61,
	//XXX_USESKILL_FAIL_II_GASOLINE_BOOMB = 62,
	//XXX_USESKILL_FAIL_II_OIL_BOTTLE = 63,
	//XXX_USESKILL_FAIL_II_EXPLOSION_POWDER = 64,
	//XXX_USESKILL_FAIL_II_SMOKE_POWDER = 65,
	//XXX_USESKILL_FAIL_II_TEAR_GAS = 66,
	//XXX_USESKILL_FAIL_II_HYDROCHLORIC_ACID_BOTTLE = 67,
	//XXX_USESKILL_FAIL_II_HELLS_PLANT_BOTTLE = 68,
	//XXX_USESKILL_FAIL_II_MANDRAGORA_FLOWERPOT = 69,
	USESKILL_FAIL_MANUAL_NOTIFY = 70,
	USESKILL_FAIL_NEED_ITEM = 71,
	USESKILL_FAIL_NEED_EQUIPMENT = 72,
	USESKILL_FAIL_COMBOSKILL = 73,
	USESKILL_FAIL_SPIRITS = 74,
	USESKILL_FAIL_EXPLOSIONSPIRITS = 75,
	USESKILL_FAIL_HP_TOOMANY = 76,
	USESKILL_FAIL_NEED_ROYAL_GUARD_BANDING = 77,
	USESKILL_FAIL_NEED_EQUIPPED_WEAPON_CLASS = 78,
	USESKILL_FAIL_EL_SUMMON = 79,
	USESKILL_FAIL_RELATIONGRADE = 80,
	USESKILL_FAIL_STYLE_CHANGE_FIGHTER = 81,
	USESKILL_FAIL_STYLE_CHANGE_GRAPPLER = 82,
	USESKILL_FAIL_THERE_ARE_NPC_AROUND = 83,
};

enum clif_messages {
	SKILL_CANT_USE_AREA = 0x536,
};

/**
 * Color Table
 **/
enum clif_colors {
	COLOR_RED,
	
	COLOR_MAX
};

enum hChSysChOpt {
	hChSys_OPT_BASE				= 0,
	hChSys_OPT_ANNOUNCE_JOIN	= 1,
};

enum hChSysChType {
	hChSys_PUBLIC	= 0,
	hChSys_PRIVATE	= 1,
	hChSys_MAP		= 2,
	hChSys_ALLY		= 3,
};


/**
 * Structures
 **/
typedef VTABLE_ENTRY(void, pFunc, int, struct map_session_data *) //cant help but put it first
struct s_packet_db {
	short len;
	pFunc func;
	short pos[MAX_PACKET_POS];
};

struct {
	unsigned long *colors;
	char **colors_name;
	unsigned char colors_count;
	bool local, ally;
	bool local_autojoin, ally_autojoin;
	char local_name[HCHSYS_NAME_LENGTH], ally_name[HCHSYS_NAME_LENGTH];
	unsigned char local_color, ally_color;
	bool closing;
	bool allow_user_channel_creation;
} hChSys;

struct hChSysCh {
	char name[HCHSYS_NAME_LENGTH];
	char pass[HCHSYS_NAME_LENGTH];
	unsigned char color;
	DBMap *users;
	unsigned int opt;
	unsigned int owner;
	enum hChSysChType type;
	uint16 m;
};

/**
 * Vars
 **/
struct s_packet_db packet_db[MAX_PACKET_DB + 1];
unsigned long color_table[COLOR_MAX];

#define PACKET_ENTRY(name) VTABLE_ENTRY(void, p##name, int fd, struct map_session_data *sd)

/**
 * Clif.c Interface
 **/
struct clif_interface {
	/* vars */
	uint32 map_ip;
	uint32 bind_ip;
	uint16 map_port;
	char map_ip_str[128];
	int map_fd;
	DBMap* channel_db;
	/* for clif_clearunit_delayed */
	struct eri *delay_clearunit_ers;
	/* core */
	VTABLE_ENTRY(int, init)
	VTABLE_ENTRY(void, final)
	VTABLE_ENTRY(int, setip, const char* ip)
	VTABLE_ENTRY(void, setbindip, const char* ip)
	VTABLE_ENTRY(void, setport, uint16 port)
	VTABLE_ENTRY(uint32, refresh_ip)
	VTABLE_ENTRY(int, send, const uint8* buf, int len, struct block_list* bl, enum send_target type)
	VTABLE_ENTRY(int, send_sub, struct block_list *bl, va_list ap)
	VTABLE_ENTRY(int, parse, int fd)
	/* auth */
	VTABLE_ENTRY(void, authok, struct map_session_data *sd)
	VTABLE_ENTRY(void, authrefuse, int fd, uint8 error_code)
	VTABLE_ENTRY(void, authfail_fd, int fd, int type)
	VTABLE_ENTRY(void, charselectok, int id, uint8 ok)
	/* item-related */
	VTABLE_ENTRY(void, dropflooritem, struct flooritem_data* fitem)
	VTABLE_ENTRY(void, clearflooritem, struct flooritem_data *fitem, int fd)
	VTABLE_ENTRY(void, additem, struct map_session_data *sd, int n, int amount, int fail)
	VTABLE_ENTRY(void, dropitem, struct map_session_data *sd,int n,int amount)
	VTABLE_ENTRY(void, delitem, struct map_session_data *sd,int n,int amount, short reason)
	VTABLE_ENTRY(void, takeitem, struct block_list* src, struct block_list* dst)
	VTABLE_ENTRY(void, arrowequip, struct map_session_data *sd,int val)
	VTABLE_ENTRY(void, arrow_fail, struct map_session_data *sd,int type)
	VTABLE_ENTRY(void, use_card, struct map_session_data *sd,int idx)
	VTABLE_ENTRY(void, cart_additem, struct map_session_data *sd,int n,int amount,int fail)
	VTABLE_ENTRY(void, cart_delitem, struct map_session_data *sd,int n,int amount)
	VTABLE_ENTRY(void, equipitemack, struct map_session_data *sd,int n,int pos,int ok)
	VTABLE_ENTRY(void, unequipitemack, struct map_session_data *sd,int n,int pos,int ok)
	VTABLE_ENTRY(void, useitemack, struct map_session_data *sd,int index,int amount,bool ok)
	VTABLE_ENTRY(void, addcards, unsigned char* buf, struct item* item)
	VTABLE_ENTRY(void, item_sub, unsigned char *buf, int n, struct item *i, struct item_data *id, int equip)
	VTABLE_ENTRY(void, getareachar_item, struct map_session_data* sd,struct flooritem_data* fitem)
	/* unit-related */
	VTABLE_ENTRY(void, clearunit_single, int id, clr_type type, int fd)
	VTABLE_ENTRY(void, clearunit_area, struct block_list* bl, clr_type type)
	VTABLE_ENTRY(void, clearunit_delayed, struct block_list* bl, clr_type type, unsigned int tick)
	VTABLE_ENTRY(void, walkok, struct map_session_data *sd)
	VTABLE_ENTRY(void, move, struct unit_data *ud)
	VTABLE_ENTRY(void, move2, struct block_list *bl, struct view_data *vd, struct unit_data *ud)
	VTABLE_ENTRY(void, blown, struct block_list *bl)
	VTABLE_ENTRY(void, slide, struct block_list *bl, int x, int y)
	VTABLE_ENTRY(void, fixpos, struct block_list *bl)
	VTABLE_ENTRY(void, changelook, struct block_list *bl,int type,int val)
	VTABLE_ENTRY(void, changetraplook, struct block_list *bl,int val)
	VTABLE_ENTRY(void, refreshlook, struct block_list *bl,int id,int type,int val,enum send_target target)
	VTABLE_ENTRY(void, class_change, struct block_list *bl,int class_,int type)
	VTABLE_ENTRY(void, skill_setunit, struct skill_unit *unit)
	VTABLE_ENTRY(void, skill_delunit, struct skill_unit *unit)
	VTABLE_ENTRY(void, skillunit_update, struct block_list* bl)
	VTABLE_ENTRY(int, clearunit_delayed_sub, int tid, unsigned int tick, int id, intptr_t data)
	VTABLE_ENTRY(int, set_unit_idle, struct block_list* bl, unsigned char* buffer, bool spawn)
	VTABLE_ENTRY(void, setdisguise, struct block_list *bl, unsigned char *buf,int len)
	VTABLE_ENTRY(int, set_unit_walking, struct block_list* bl, struct unit_data* ud, unsigned char* buffer)
	VTABLE_ENTRY(int, calc_walkdelay, struct block_list *bl,int delay, int type, int damage, int div_)
	VTABLE_ENTRY(void, getareachar_skillunit, struct map_session_data *sd, struct skill_unit *unit)
	VTABLE_ENTRY(void, getareachar_unit, struct map_session_data* sd,struct block_list *bl)
	VTABLE_ENTRY(void, clearchar_skillunit, struct skill_unit *unit, int fd)
	VTABLE_ENTRY(int, getareachar, struct block_list* bl,va_list ap)
	/* main unit spawn */
	VTABLE_ENTRY(int, spawn, struct block_list *bl)
	/* map-related */
	VTABLE_ENTRY(void, changemap, struct map_session_data *sd, short map, int x, int y)
	VTABLE_ENTRY(void, changemapcell, int fd, int16 m, int x, int y, int type, enum send_target target)
	VTABLE_ENTRY(void, map_property, struct map_session_data* sd, enum map_property property)
	VTABLE_ENTRY(void, pvpset, struct map_session_data *sd, int pvprank, int pvpnum,int type)
	VTABLE_ENTRY(void, map_property_mapall, int map, enum map_property property)
	VTABLE_ENTRY(void, bossmapinfo, int fd, struct mob_data *md, short flag)
	VTABLE_ENTRY(void, map_type, struct map_session_data* sd, enum map_type type)
	/* multi-map-server */
	VTABLE_ENTRY(void, changemapserver, struct map_session_data* sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port)
	/* npc-shop-related */
	VTABLE_ENTRY(void, npcbuysell, struct map_session_data* sd, int id)
	VTABLE_ENTRY(void, buylist, struct map_session_data *sd, struct npc_data *nd)
	VTABLE_ENTRY(void, selllist, struct map_session_data *sd)
	VTABLE_ENTRY(void, cashshop_show, struct map_session_data *sd, struct npc_data *nd)
	VTABLE_ENTRY(void, npc_buy_result, struct map_session_data* sd, unsigned char result)
	VTABLE_ENTRY(void, npc_sell_result, struct map_session_data* sd, unsigned char result)
	VTABLE_ENTRY(void, cashshop_ack, struct map_session_data* sd, int error)
	/* npc-script-related */
	VTABLE_ENTRY(void, scriptmes, struct map_session_data *sd, int npcid, const char *mes)
	VTABLE_ENTRY(void, scriptnext, struct map_session_data *sd,int npcid)
	VTABLE_ENTRY(void, scriptclose, struct map_session_data *sd, int npcid)
	VTABLE_ENTRY(void, scriptmenu, struct map_session_data* sd, int npcid, const char* mes)
	VTABLE_ENTRY(void, scriptinput, struct map_session_data *sd, int npcid)
	VTABLE_ENTRY(void, scriptinputstr, struct map_session_data *sd, int npcid)
	VTABLE_ENTRY(void, cutin, struct map_session_data* sd, const char* image, int type)
	VTABLE_ENTRY(void, sendfakenpc, struct map_session_data *sd, int npcid)
	/* client-user-interface-related */
	VTABLE_ENTRY(void, viewpoint, struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color)
	VTABLE_ENTRY(int, damage, struct block_list* src, struct block_list* dst, unsigned int tick, int sdelay, int ddelay, int damage, int div, int type, int damage2)
	VTABLE_ENTRY(void, sitting, struct block_list* bl)
	VTABLE_ENTRY(void, standing, struct block_list* bl)
	VTABLE_ENTRY(void, arrow_create_list, struct map_session_data *sd)
	VTABLE_ENTRY(void, refresh, struct map_session_data *sd)
	VTABLE_ENTRY(void, fame_blacksmith, struct map_session_data *sd, int points)
	VTABLE_ENTRY(void, fame_alchemist, struct map_session_data *sd, int points)
	VTABLE_ENTRY(void, fame_taekwon, struct map_session_data *sd, int points)
	VTABLE_ENTRY(void, hotkeys, struct map_session_data *sd)
	VTABLE_ENTRY(int, insight, struct block_list *bl,va_list ap)
	VTABLE_ENTRY(int, outsight, struct block_list *bl,va_list ap)
	VTABLE_ENTRY(void, skillcastcancel, struct block_list* bl)
	VTABLE_ENTRY(void, skill_fail, struct map_session_data *sd,uint16 skill_id,enum useskill_fail_cause cause,int btype)
	VTABLE_ENTRY(void, skill_cooldown, struct map_session_data *sd, uint16 skill_id, unsigned int tick)
	VTABLE_ENTRY(void, skill_memomessage, struct map_session_data* sd, int type)
	VTABLE_ENTRY(void, skill_teleportmessage, struct map_session_data *sd, int type)
	VTABLE_ENTRY(void, skill_produce_mix_list, struct map_session_data *sd, int skill_id, int trigger)
	VTABLE_ENTRY(void, cooking_list, struct map_session_data *sd, int trigger, uint16 skill_id, int qty, int list_type)
	VTABLE_ENTRY(void, autospell, struct map_session_data *sd,uint16 skill_lv)
	VTABLE_ENTRY(void, combo_delay, struct block_list *bl,int wait)
	VTABLE_ENTRY(void, status_change, struct block_list *bl,int type,int flag,int tick,int val1, int val2, int val3)
	VTABLE_ENTRY(void, insert_card, struct map_session_data *sd,int idx_equip,int idx_card,int flag)
	VTABLE_ENTRY(void, inventorylist, struct map_session_data *sd)
	VTABLE_ENTRY(void, equiplist, struct map_session_data *sd)
	VTABLE_ENTRY(void, cartlist, struct map_session_data *sd)
	VTABLE_ENTRY(void, favorite_item, struct map_session_data* sd, unsigned short index)
	VTABLE_ENTRY(void, clearcart, int fd)
	VTABLE_ENTRY(void, item_identify_list, struct map_session_data *sd)
	VTABLE_ENTRY(void, item_identified, struct map_session_data *sd,int idx,int flag)
	VTABLE_ENTRY(void, item_repair_list, struct map_session_data *sd, struct map_session_data *dstsd, int lv)
	VTABLE_ENTRY(void, item_repaireffect, struct map_session_data *sd, int idx, int flag)
	VTABLE_ENTRY(void, item_damaged, struct map_session_data* sd, unsigned short position)
	VTABLE_ENTRY(void, item_refine_list, struct map_session_data *sd)
	VTABLE_ENTRY(void, item_skill, struct map_session_data *sd,uint16 skill_id,uint16 skill_lv)
	VTABLE_ENTRY(void, mvp_item, struct map_session_data *sd,int nameid)
	VTABLE_ENTRY(void, mvp_exp, struct map_session_data *sd, unsigned int exp)
	VTABLE_ENTRY(void, mvp_noitem, struct map_session_data* sd)
	VTABLE_ENTRY(void, changed_dir, struct block_list *bl, enum send_target target)
	VTABLE_ENTRY(void, charnameack, int fd, struct block_list *bl)
	VTABLE_ENTRY(void, monster_hp_bar,  struct mob_data* md, int fd )
	VTABLE_ENTRY(int, hpmeter, struct map_session_data *sd)
	VTABLE_ENTRY(void, hpmeter_single, int fd, int id, unsigned int hp, unsigned int maxhp)
	VTABLE_ENTRY(int, hpmeter_sub, struct block_list *bl, va_list ap)
	VTABLE_ENTRY(void, upgrademessage, int fd, int result, int item_id)
	VTABLE_ENTRY(void, get_weapon_view, struct map_session_data* sd, unsigned short *rhand, unsigned short *lhand)
	VTABLE_ENTRY(void, gospel_info, struct map_session_data *sd, int type)
	VTABLE_ENTRY(void, feel_req, int fd, struct map_session_data *sd, uint16 skill_lv)
	VTABLE_ENTRY(void, starskill, struct map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result)
	VTABLE_ENTRY(void, feel_info, struct map_session_data* sd, unsigned char feel_level, unsigned char type)
	VTABLE_ENTRY(void, hate_info, struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type)
	VTABLE_ENTRY(void, mission_info, struct map_session_data *sd, int mob_id, unsigned char progress)
	VTABLE_ENTRY(void, feel_hate_reset, struct map_session_data *sd)
	VTABLE_ENTRY(void, equiptickack, struct map_session_data* sd, int flag)
	VTABLE_ENTRY(void, viewequip_ack, struct map_session_data* sd, struct map_session_data* tsd)
	VTABLE_ENTRY(void, viewequip_fail, struct map_session_data* sd)
	VTABLE_ENTRY(void, equipcheckbox, struct map_session_data* sd)
	VTABLE_ENTRY(void, displayexp, struct map_session_data *sd, unsigned int exp, char type, bool quest)
	VTABLE_ENTRY(void, font, struct map_session_data *sd)
	VTABLE_ENTRY(void, progressbar, struct map_session_data * sd, unsigned long color, unsigned int second)
	VTABLE_ENTRY(void, progressbar_abort, struct map_session_data * sd)
	VTABLE_ENTRY(void, showdigit, struct map_session_data* sd, unsigned char type, int value)
	VTABLE_ENTRY(int, elementalconverter_list, struct map_session_data *sd)
	VTABLE_ENTRY(int, spellbook_list, struct map_session_data *sd)
	VTABLE_ENTRY(int, magicdecoy_list, struct map_session_data *sd, uint16 skill_lv, short x, short y)
	VTABLE_ENTRY(int, poison_list, struct map_session_data *sd, uint16 skill_lv)
	VTABLE_ENTRY(int, autoshadowspell_list, struct map_session_data *sd)
	VTABLE_ENTRY(int, skill_itemlistwindow,  struct map_session_data *sd, uint16 skill_id, uint16 skill_lv )
	VTABLE_ENTRY(int, sc_notick, struct block_list *bl,int type,int flag,int val1, int val2, int val3)
	VTABLE_ENTRY(int, sc_single, int fd, int id,int type,int flag,int val1, int val2, int val3)
	VTABLE_ENTRY(void, initialstatus, struct map_session_data *sd)
	/* player-unit-specific-related */
	VTABLE_ENTRY(void, updatestatus, struct map_session_data *sd,int type)
	VTABLE_ENTRY(void, changestatus, struct map_session_data* sd,int type,int val)
	VTABLE_ENTRY(void, statusupack, struct map_session_data *sd,int type,int ok,int val)
	VTABLE_ENTRY(void, movetoattack, struct map_session_data *sd,struct block_list *bl)
	VTABLE_ENTRY(void, solved_charname, int fd, int charid, const char* name)
	VTABLE_ENTRY(void, charnameupdate, struct map_session_data *ssd)
	VTABLE_ENTRY(int, delayquit, int tid, unsigned int tick, int id, intptr_t data)
	VTABLE_ENTRY(void, getareachar_pc, struct map_session_data* sd,struct map_session_data* dstsd)
	VTABLE_ENTRY(void, disconnect_ack, struct map_session_data* sd, short result)
	VTABLE_ENTRY(void, PVPInfo, struct map_session_data* sd)
	VTABLE_ENTRY(void, blacksmith, struct map_session_data* sd)
	VTABLE_ENTRY(void, alchemist, struct map_session_data* sd)
	VTABLE_ENTRY(void, taekwon, struct map_session_data* sd)
	VTABLE_ENTRY(void, ranking_pk, struct map_session_data* sd)
	VTABLE_ENTRY(void, quitsave, int fd,struct map_session_data *sd)
	/* visual effects client-side */
	VTABLE_ENTRY(void, misceffect, struct block_list* bl,int type)
	VTABLE_ENTRY(void, changeoption, struct block_list* bl)
	VTABLE_ENTRY(void, changeoption2, struct block_list* bl)
	VTABLE_ENTRY(void, emotion, struct block_list *bl,int type)
	VTABLE_ENTRY(void, talkiebox, struct block_list* bl, const char* talkie)
	VTABLE_ENTRY(void, wedding_effect, struct block_list *bl)
	VTABLE_ENTRY(void, divorced, struct map_session_data* sd, const char* name)
	VTABLE_ENTRY(void, callpartner, struct map_session_data *sd)
	VTABLE_ENTRY(int, skill_damage, struct block_list *src,struct block_list *dst,unsigned int tick,int sdelay,int ddelay,int damage,int div,uint16 skill_id,uint16 skill_lv,int type)
	VTABLE_ENTRY(int, skill_nodamage, struct block_list *src,struct block_list *dst,uint16 skill_id,int heal,int fail)
	VTABLE_ENTRY(void, skill_poseffect, struct block_list *src,uint16 skill_id,int val,int x,int y,int tick)
	VTABLE_ENTRY(void, skill_estimation, struct map_session_data *sd,struct block_list *dst)
	VTABLE_ENTRY(void, skill_warppoint, struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4)
	VTABLE_ENTRY(void, skillcasting, struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, int property, int casttime)
	VTABLE_ENTRY(void, produce_effect, struct map_session_data* sd,int flag,int nameid)
	VTABLE_ENTRY(void, devotion, struct block_list *src, struct map_session_data *tsd)
	VTABLE_ENTRY(void, spiritball, struct block_list *bl)
	VTABLE_ENTRY(void, spiritball_single, int fd, struct map_session_data *sd)
	VTABLE_ENTRY(void, bladestop, struct block_list *src, int dst_id, int active)
	VTABLE_ENTRY(void, mvp_effect, struct map_session_data *sd)
	VTABLE_ENTRY(void, heal, int fd,int type,int val)
	VTABLE_ENTRY(void, resurrection, struct block_list *bl,int type)
	VTABLE_ENTRY(void, refine, int fd, int fail, int index, int val)
	VTABLE_ENTRY(void, weather, int16 m)
	VTABLE_ENTRY(void, specialeffect, struct block_list* bl, int type, enum send_target target)
	VTABLE_ENTRY(void, specialeffect_single, struct block_list* bl, int type, int fd)
	VTABLE_ENTRY(void, specialeffect_value, struct block_list* bl, int effect_id, int num, send_target target)
	VTABLE_ENTRY(void, millenniumshield, struct map_session_data *sd, short shields )
	VTABLE_ENTRY(void, talisman, struct map_session_data *sd, short type)
	VTABLE_ENTRY(void, talisman_single, int fd, struct map_session_data *sd, short type)
	VTABLE_ENTRY(void, snap,  struct block_list *bl, short x, short y )
	VTABLE_ENTRY(void, weather_check, struct map_session_data *sd)
	/* sound effects client-side */
	VTABLE_ENTRY(void, playBGM, struct map_session_data* sd, const char* name)
	VTABLE_ENTRY(void, soundeffect, struct map_session_data* sd, struct block_list* bl, const char* name, int type)
	VTABLE_ENTRY(void, soundeffectall, struct block_list* bl, const char* name, int type, enum send_target coverage)
	/* chat/message-related */
	VTABLE_ENTRY(void, GlobalMessage, struct block_list* bl, const char* message)
	VTABLE_ENTRY(void, createchat, struct map_session_data* sd, int flag)
	VTABLE_ENTRY(void, dispchat, struct chat_data* cd, int fd)
	VTABLE_ENTRY(void, joinchatfail, struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, joinchatok, struct map_session_data *sd,struct chat_data* cd)
	VTABLE_ENTRY(void, addchat, struct chat_data* cd,struct map_session_data *sd)
	VTABLE_ENTRY(void, changechatowner, struct chat_data* cd, struct map_session_data* sd)
	VTABLE_ENTRY(void, clearchat, struct chat_data *cd,int fd)
	VTABLE_ENTRY(void, leavechat, struct chat_data* cd, struct map_session_data* sd, bool flag)
	VTABLE_ENTRY(void, changechatstatus, struct chat_data* cd)
	VTABLE_ENTRY(void, wis_message, int fd, const char* nick, const char* mes, int mes_len)
	VTABLE_ENTRY(void, wis_end, int fd, int flag)
	VTABLE_ENTRY(void, disp_onlyself, struct map_session_data *sd, const char *mes, int len)
	VTABLE_ENTRY(void, disp_message, struct block_list* src, const char* mes, int len, enum send_target target)
	VTABLE_ENTRY(void, broadcast, struct block_list* bl, const char* mes, int len, int type, enum send_target target)
	VTABLE_ENTRY(void, broadcast2, struct block_list* bl, const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target)
	VTABLE_ENTRY(void, messagecolor, struct block_list* bl, unsigned long color, const char* msg)
	VTABLE_ENTRY(void, disp_overhead, struct block_list *bl, const char* mes)
	VTABLE_ENTRY(void, msg, struct map_session_data* sd, unsigned short id)
	VTABLE_ENTRY(void, msg_value, struct map_session_data* sd, unsigned short id, int value)
	VTABLE_ENTRY(void, msg_skill, struct map_session_data* sd, uint16 skill_id, int msg_id)
	VTABLE_ENTRY(void, msgtable, int fd, int line)
	VTABLE_ENTRY(void, msgtable_num, int fd, int line, int num)
	VTABLE_ENTRY(void, message, const int fd, const char* mes)
	VTABLE_ENTRY(int, colormes, struct map_session_data * sd, enum clif_colors color, const char* msg)
	VTABLE_ENTRY(bool, process_message, struct map_session_data* sd, int format, char** name_, int* namelen_, char** message_, int* messagelen_)
	VTABLE_ENTRY(void, wisexin, struct map_session_data *sd,int type,int flag)
	VTABLE_ENTRY(void, wisall, struct map_session_data *sd,int type,int flag)
	VTABLE_ENTRY(void, PMIgnoreList, struct map_session_data* sd)
	/* trade handling */
	VTABLE_ENTRY(void, traderequest, struct map_session_data* sd, const char* name)
	VTABLE_ENTRY(void, tradestart, struct map_session_data* sd, uint8 type)
	VTABLE_ENTRY(void, tradeadditem, struct map_session_data* sd, struct map_session_data* tsd, int index, int amount)
	VTABLE_ENTRY(void, tradeitemok, struct map_session_data* sd, int index, int fail)
	VTABLE_ENTRY(void, tradedeal_lock, struct map_session_data* sd, int fail)
	VTABLE_ENTRY(void, tradecancelled, struct map_session_data* sd)
	VTABLE_ENTRY(void, tradecompleted, struct map_session_data* sd, int fail)
	VTABLE_ENTRY(void, tradeundo, struct map_session_data* sd)
	/* vending handling */
	VTABLE_ENTRY(void, openvendingreq, struct map_session_data* sd, int num)
	VTABLE_ENTRY(void, showvendingboard, struct block_list* bl, const char* message, int fd)
	VTABLE_ENTRY(void, closevendingboard, struct block_list* bl, int fd)
	VTABLE_ENTRY(void, vendinglist, struct map_session_data* sd, int id, struct s_vending* vending)
	VTABLE_ENTRY(void, buyvending, struct map_session_data* sd, int index, int amount, int fail)
	VTABLE_ENTRY(void, openvending, struct map_session_data* sd, int id, struct s_vending* vending)
	VTABLE_ENTRY(void, vendingreport, struct map_session_data* sd, int index, int amount)
	/* storage handling */
	VTABLE_ENTRY(void, storagelist, struct map_session_data* sd, struct item* items, int items_length)
	VTABLE_ENTRY(void, updatestorageamount, struct map_session_data* sd, int amount, int max_amount)
	VTABLE_ENTRY(void, storageitemadded, struct map_session_data* sd, struct item* i, int index, int amount)
	VTABLE_ENTRY(void, storageitemremoved, struct map_session_data* sd, int index, int amount)
	VTABLE_ENTRY(void, storageclose, struct map_session_data* sd)
	/* skill-list handling */
	VTABLE_ENTRY(void, skillinfoblock, struct map_session_data *sd)
	VTABLE_ENTRY(void, skillup, struct map_session_data *sd,uint16 skill_id)
	VTABLE_ENTRY(void, skillinfo, struct map_session_data *sd,int skill, int inf)
	VTABLE_ENTRY(void, addskill, struct map_session_data *sd, int id)
	VTABLE_ENTRY(void, deleteskill, struct map_session_data *sd, int id)
	/* party-specific */
	VTABLE_ENTRY(void, party_created, struct map_session_data *sd,int result)
	VTABLE_ENTRY(void, party_member_info, struct party_data *p, struct map_session_data *sd)
	VTABLE_ENTRY(void, party_info, struct party_data* p, struct map_session_data *sd)
	VTABLE_ENTRY(void, party_invite, struct map_session_data *sd,struct map_session_data *tsd)
	VTABLE_ENTRY(void, party_inviteack, struct map_session_data* sd, const char* nick, int result)
	VTABLE_ENTRY(void, party_option, struct party_data *p,struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, party_withdraw, struct party_data* p, struct map_session_data* sd, int account_id, const char* name, int flag)
	VTABLE_ENTRY(void, party_message, struct party_data* p, int account_id, const char* mes, int len)
	VTABLE_ENTRY(void, party_xy, struct map_session_data *sd)
	VTABLE_ENTRY(void, party_xy_single, int fd, struct map_session_data *sd)
	VTABLE_ENTRY(void, party_hp, struct map_session_data *sd)
	VTABLE_ENTRY(void, party_xy_remove, struct map_session_data *sd)
	VTABLE_ENTRY(void, party_show_picker, struct map_session_data * sd, struct item * item_data)
	VTABLE_ENTRY(void, partyinvitationstate, struct map_session_data* sd)
	/* guild-specific */
	VTABLE_ENTRY(void, guild_created, struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, guild_belonginfo, struct map_session_data *sd, struct guild *g)
	VTABLE_ENTRY(void, guild_masterormember, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_basicinfo, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_allianceinfo, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_memberlist, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_skillinfo, struct map_session_data* sd)
	VTABLE_ENTRY(void, guild_send_onlineinfo, struct map_session_data *sd) //[LuzZza]
	VTABLE_ENTRY(void, guild_memberlogin_notice, struct guild *g,int idx,int flag)
	VTABLE_ENTRY(void, guild_invite, struct map_session_data *sd,struct guild *g)
	VTABLE_ENTRY(void, guild_inviteack, struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, guild_leave, struct map_session_data *sd,const char *name,const char *mes)
	VTABLE_ENTRY(void, guild_expulsion, struct map_session_data* sd, const char* name, const char* mes, int account_id)
	VTABLE_ENTRY(void, guild_positionchanged, struct guild *g,int idx)
	VTABLE_ENTRY(void, guild_memberpositionchanged, struct guild *g,int idx)
	VTABLE_ENTRY(void, guild_emblem, struct map_session_data *sd,struct guild *g)
	VTABLE_ENTRY(void, guild_emblem_area, struct block_list* bl)
	VTABLE_ENTRY(void, guild_notice, struct map_session_data* sd, struct guild* g)
	VTABLE_ENTRY(void, guild_message, struct guild *g,int account_id,const char *mes,int len)
	VTABLE_ENTRY(int, guild_skillup, struct map_session_data *sd,uint16 skill_id,int lv)
	VTABLE_ENTRY(void, guild_reqalliance, struct map_session_data *sd,int account_id,const char *name)
	VTABLE_ENTRY(void, guild_allianceack, struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, guild_delalliance, struct map_session_data *sd,int guild_id,int flag)
	VTABLE_ENTRY(void, guild_oppositionack, struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, guild_broken, struct map_session_data *sd,int flag)
	VTABLE_ENTRY(void, guild_xy, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_xy_single, int fd, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_xy_remove, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_positionnamelist, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_positioninfolist, struct map_session_data *sd)
	VTABLE_ENTRY(void, guild_expulsionlist, struct map_session_data* sd)
	VTABLE_ENTRY(bool, validate_emblem, const uint8* emblem, unsigned long emblem_len)
	/* battleground-specific */
	VTABLE_ENTRY(void, bg_hp, struct map_session_data *sd)
	VTABLE_ENTRY(void, bg_xy, struct map_session_data *sd)
	VTABLE_ENTRY(void, bg_xy_remove, struct map_session_data *sd)
	VTABLE_ENTRY(void, bg_message, struct battleground_data *bg, int src_id, const char *name, const char *mes, int len)
	VTABLE_ENTRY(void, bg_updatescore, int16 m)
	VTABLE_ENTRY(void, bg_updatescore_single, struct map_session_data *sd)
	VTABLE_ENTRY(void, sendbgemblem_area, struct map_session_data *sd)
	VTABLE_ENTRY(void, sendbgemblem_single, int fd, struct map_session_data *sd)
	/* instance-related */
	VTABLE_ENTRY(int, instance, int instance_id, int type, int flag)
	VTABLE_ENTRY(void, instance_join, int fd, int instance_id)
	VTABLE_ENTRY(void, instance_leave, int fd)
	/* pet-related */
	VTABLE_ENTRY(void, catch_process, struct map_session_data *sd)
	VTABLE_ENTRY(void, pet_roulette, struct map_session_data *sd,int data)
	VTABLE_ENTRY(void, sendegg, struct map_session_data *sd)
	VTABLE_ENTRY(void, send_petstatus, struct map_session_data *sd)
	VTABLE_ENTRY(void, send_petdata, struct map_session_data* sd, struct pet_data* pd, int type, int param)
	VTABLE_ENTRY(void, pet_emotion, struct pet_data *pd,int param)
	VTABLE_ENTRY(void, pet_food, struct map_session_data *sd,int foodid,int fail)
	/* friend-related */
	VTABLE_ENTRY(int, friendslist_toggle_sub, struct map_session_data *sd,va_list ap)
	VTABLE_ENTRY(void, friendslist_send, struct map_session_data *sd)
	VTABLE_ENTRY(void, friendslist_reqack, struct map_session_data *sd, struct map_session_data *f_sd, int type)
	VTABLE_ENTRY(void, friendslist_toggle, struct map_session_data *sd,int account_id, int char_id, int online)
	VTABLE_ENTRY(void, friendlist_req, struct map_session_data* sd, int account_id, int char_id, const char* name)
	/* gm-related */
	VTABLE_ENTRY(void, GM_kickack, struct map_session_data *sd, int id)
	VTABLE_ENTRY(void, GM_kick, struct map_session_data *sd,struct map_session_data *tsd)
	VTABLE_ENTRY(void, manner_message, struct map_session_data* sd, uint32 type)
	VTABLE_ENTRY(void, GM_silence, struct map_session_data* sd, struct map_session_data* tsd, uint8 type)
	VTABLE_ENTRY(void, account_name, struct map_session_data* sd, int account_id, const char* accname)
	VTABLE_ENTRY(void, check, int fd, struct map_session_data* pl_sd)
	/* hom-related */
	VTABLE_ENTRY(void, hominfo, struct map_session_data *sd, struct homun_data *hd, int flag)
	VTABLE_ENTRY(int, homskillinfoblock, struct map_session_data *sd)
	VTABLE_ENTRY(void, homskillup, struct map_session_data *sd, uint16 skill_id)
	VTABLE_ENTRY(int, hom_food, struct map_session_data *sd,int foodid,int fail)
	VTABLE_ENTRY(void, send_homdata, struct map_session_data *sd, int state, int param)
	/* questlog-related */
	VTABLE_ENTRY(void, quest_send_list, struct map_session_data * sd)
	VTABLE_ENTRY(void, quest_send_mission, struct map_session_data * sd)
	VTABLE_ENTRY(void, quest_add, struct map_session_data * sd, struct quest * qd, int index)
	VTABLE_ENTRY(void, quest_delete, struct map_session_data * sd, int quest_id)
	VTABLE_ENTRY(void, quest_update_status, struct map_session_data * sd, int quest_id, bool active)
	VTABLE_ENTRY(void, quest_update_objective, struct map_session_data * sd, struct quest * qd, int index)
	VTABLE_ENTRY(void, quest_show_event, struct map_session_data *sd, struct block_list *bl, short state, short color)
	/* mail-related */
	VTABLE_ENTRY(void, mail_window, int fd, int flag)
	VTABLE_ENTRY(void, mail_read, struct map_session_data *sd, int mail_id)
	VTABLE_ENTRY(void, mail_delete, int fd, int mail_id, short fail)
	VTABLE_ENTRY(void, mail_return, int fd, int mail_id, short fail)
	VTABLE_ENTRY(void, mail_send, int fd, bool fail)
	VTABLE_ENTRY(void, mail_new, int fd, int mail_id, const char *sender, const char *title)
	VTABLE_ENTRY(void, mail_refreshinbox, struct map_session_data *sd)
	VTABLE_ENTRY(void, mail_getattachment, int fd, uint8 flag)
	VTABLE_ENTRY(void, mail_setattachment, int fd, int index, uint8 flag)
	/* auction-related */
	VTABLE_ENTRY(void, auction_openwindow, struct map_session_data *sd)
	VTABLE_ENTRY(void, auction_results, struct map_session_data *sd, short count, short pages, uint8 *buf)
	VTABLE_ENTRY(void, auction_message, int fd, unsigned char flag)
	VTABLE_ENTRY(void, auction_close, int fd, unsigned char flag)
	VTABLE_ENTRY(void, auction_setitem, int fd, int index, bool fail)
	/* mercenary-related */
	VTABLE_ENTRY(void, mercenary_info, struct map_session_data *sd)
	VTABLE_ENTRY(void, mercenary_skillblock, struct map_session_data *sd)
	VTABLE_ENTRY(void, mercenary_message, struct map_session_data* sd, int message)
	VTABLE_ENTRY(void, mercenary_updatestatus, struct map_session_data *sd, int type)
	/* item rental */
	VTABLE_ENTRY(void, rental_time, int fd, int nameid, int seconds)
	VTABLE_ENTRY(void, rental_expired, int fd, int index, int nameid)
	/* party booking related */
	VTABLE_ENTRY(void, PartyBookingRegisterAck, struct map_session_data *sd, int flag)
	VTABLE_ENTRY(void, PartyBookingDeleteAck, struct map_session_data* sd, int flag)
	VTABLE_ENTRY(void, PartyBookingSearchAck, int fd, struct party_booking_ad_info** results, int count, bool more_result)
	VTABLE_ENTRY(void, PartyBookingUpdateNotify, struct map_session_data* sd, struct party_booking_ad_info* pb_ad)
	VTABLE_ENTRY(void, PartyBookingDeleteNotify, struct map_session_data* sd, int index)
	VTABLE_ENTRY(void, PartyBookingInsertNotify, struct map_session_data* sd, struct party_booking_ad_info* pb_ad)
	/* buying store-related */
	VTABLE_ENTRY(void, buyingstore_open, struct map_session_data* sd)
	VTABLE_ENTRY(void, buyingstore_open_failed, struct map_session_data* sd, unsigned short result, unsigned int weight)
	VTABLE_ENTRY(void, buyingstore_myitemlist, struct map_session_data* sd)
	VTABLE_ENTRY(void, buyingstore_entry, struct map_session_data* sd)
	VTABLE_ENTRY(void, buyingstore_entry_single, struct map_session_data* sd, struct map_session_data* pl_sd)
	VTABLE_ENTRY(void, buyingstore_disappear_entry, struct map_session_data* sd)
	VTABLE_ENTRY(void, buyingstore_disappear_entry_single, struct map_session_data* sd, struct map_session_data* pl_sd)
	VTABLE_ENTRY(void, buyingstore_itemlist, struct map_session_data* sd, struct map_session_data* pl_sd)
	VTABLE_ENTRY(void, buyingstore_trade_failed_buyer, struct map_session_data* sd, short result)
	VTABLE_ENTRY(void, buyingstore_update_item, struct map_session_data* sd, unsigned short nameid, unsigned short amount)
	VTABLE_ENTRY(void, buyingstore_delete_item, struct map_session_data* sd, short index, unsigned short amount, int price)
	VTABLE_ENTRY(void, buyingstore_trade_failed_seller, struct map_session_data* sd, short result, unsigned short nameid)
	/* search store-related */
	VTABLE_ENTRY(void, search_store_info_ack, struct map_session_data* sd)
	VTABLE_ENTRY(void, search_store_info_failed, struct map_session_data* sd, unsigned char reason)
	VTABLE_ENTRY(void, open_search_store_info, struct map_session_data* sd)
	VTABLE_ENTRY(void, search_store_info_click_ack, struct map_session_data* sd, short x, short y)
	/* elemental-related */
	VTABLE_ENTRY(void, elemental_info, struct map_session_data *sd)
	VTABLE_ENTRY(void, elemental_updatestatus, struct map_session_data *sd, int type)
	/* misc-handling */
	VTABLE_ENTRY(void, adopt_reply, struct map_session_data *sd, int type)
	VTABLE_ENTRY(void, adopt_request, struct map_session_data *sd, struct map_session_data *src, int p_id)
	VTABLE_ENTRY(void, readbook, int fd, int book_id, int page)
	VTABLE_ENTRY(void, notify_time, struct map_session_data* sd, unsigned long time)
	VTABLE_ENTRY(void, user_count, struct map_session_data* sd, int count)
	VTABLE_ENTRY(void, noask_sub, struct map_session_data *src, struct map_session_data *target, int type)
	VTABLE_ENTRY(void, chsys_create, struct hChSysCh *channel, char *name, char *pass, unsigned char color)
	VTABLE_ENTRY(void, chsys_msg, struct hChSysCh *channel, struct map_session_data *sd, char *msg)
	VTABLE_ENTRY(void, chsys_send, struct hChSysCh *channel, struct map_session_data *sd, char *msg)
	VTABLE_ENTRY(void, chsys_join, struct hChSysCh *channel, struct map_session_data *sd)
	VTABLE_ENTRY(void, chsys_left, struct hChSysCh *channel, struct map_session_data *sd)
	VTABLE_ENTRY(void, chsys_delete, struct hChSysCh *channel)
	VTABLE_ENTRY(void, chsys_mjoin, struct map_session_data *sd)
	/*------------------------
	 *- Parse Incoming Packet
	 *------------------------*/
	PACKET_ENTRY(WantToConnection)
	PACKET_ENTRY(LoadEndAck)
	PACKET_ENTRY(TickSend)
	PACKET_ENTRY(Hotkey)
	PACKET_ENTRY(Progressbar)
	PACKET_ENTRY(WalkToXY)
	PACKET_ENTRY(QuitGame)
	PACKET_ENTRY(GetCharNameRequest)
	PACKET_ENTRY(GlobalMessage)
	PACKET_ENTRY(MapMove)
	PACKET_ENTRY(ChangeDir)
	PACKET_ENTRY(Emotion)
	PACKET_ENTRY(HowManyConnections)
	PACKET_ENTRY(ActionRequest)
	VTABLE_ENTRY(void, pActionRequest_sub, struct map_session_data *sd, int action_type, int target_id, unsigned int tick)
	PACKET_ENTRY(Restart)
	PACKET_ENTRY(WisMessage)
	PACKET_ENTRY(Broadcast)
	PACKET_ENTRY(TakeItem)
	PACKET_ENTRY(DropItem)
	PACKET_ENTRY(UseItem)
	PACKET_ENTRY(EquipItem)
	PACKET_ENTRY(UnequipItem)
	PACKET_ENTRY(NpcClicked)
	PACKET_ENTRY(NpcBuySellSelected)
	PACKET_ENTRY(NpcBuyListSend)
	PACKET_ENTRY(NpcSellListSend)
	PACKET_ENTRY(CreateChatRoom)
	PACKET_ENTRY(ChatAddMember)
	PACKET_ENTRY(ChatRoomStatusChange)
	PACKET_ENTRY(ChangeChatOwner)
	PACKET_ENTRY(KickFromChat)
	PACKET_ENTRY(ChatLeave)
	PACKET_ENTRY(TradeRequest)
	VTABLE_ENTRY(void, chann_config_read)
	PACKET_ENTRY(TradeAck)
	PACKET_ENTRY(TradeAddItem)
	PACKET_ENTRY(TradeOk)
	PACKET_ENTRY(TradeCancel)
	PACKET_ENTRY(TradeCommit)
	PACKET_ENTRY(StopAttack)
	PACKET_ENTRY(PutItemToCart)
	PACKET_ENTRY(GetItemFromCart)
	PACKET_ENTRY(RemoveOption)
	PACKET_ENTRY(ChangeCart)
	PACKET_ENTRY(StatusUp)
	PACKET_ENTRY(SkillUp)
	PACKET_ENTRY(UseSkillToId)
	VTABLE_ENTRY(void, pUseSkillToId_homun, struct homun_data *hd, struct map_session_data *sd, unsigned int tick, uint16 skill_id, uint16 skill_lv, int target_id)
	VTABLE_ENTRY(void, pUseSkillToId_mercenary, struct mercenary_data *md, struct map_session_data *sd, unsigned int tick, uint16 skill_id, uint16 skill_lv, int target_id)
	PACKET_ENTRY(UseSkillToPos)
	VTABLE_ENTRY(void, pUseSkillToPosSub, int fd, struct map_session_data *sd, uint16 skill_lv, uint16 skill_id, short x, short y, int skillmoreinfo)
	VTABLE_ENTRY(void, pUseSkillToPos_homun, struct homun_data *hd, struct map_session_data *sd, unsigned int tick, uint16 skill_id, uint16 skill_lv, short x, short y, int skillmoreinfo)
	VTABLE_ENTRY(void, pUseSkillToPos_mercenary, struct mercenary_data *md, struct map_session_data *sd, unsigned int tick, uint16 skill_id, uint16 skill_lv, short x, short y, int skillmoreinfo)
	PACKET_ENTRY(UseSkillToPosMoreInfo)
	PACKET_ENTRY(UseSkillMap)
	PACKET_ENTRY(RequestMemo)
	PACKET_ENTRY(ProduceMix)
	PACKET_ENTRY(Cooking)
	PACKET_ENTRY(RepairItem)
	PACKET_ENTRY(WeaponRefine)
	PACKET_ENTRY(NpcSelectMenu)
	PACKET_ENTRY(NpcNextClicked)
	PACKET_ENTRY(NpcAmountInput)
	PACKET_ENTRY(NpcStringInput)
	PACKET_ENTRY(NpcCloseClicked)
	PACKET_ENTRY(ItemIdentify)
	PACKET_ENTRY(SelectArrow)
	PACKET_ENTRY(AutoSpell)
	PACKET_ENTRY(UseCard)
	PACKET_ENTRY(InsertCard)
	PACKET_ENTRY(SolveCharName)
	PACKET_ENTRY(ResetChar)
	PACKET_ENTRY(LocalBroadcast)
	PACKET_ENTRY(MoveToKafra)
	PACKET_ENTRY(MoveFromKafra)
	PACKET_ENTRY(MoveToKafraFromCart)
	PACKET_ENTRY(MoveFromKafraToCart)
	PACKET_ENTRY(CloseKafra)
	PACKET_ENTRY(StoragePassword)
	PACKET_ENTRY(CreateParty)
	PACKET_ENTRY(CreateParty2)
	PACKET_ENTRY(PartyInvite)
	PACKET_ENTRY(PartyInvite2)
	PACKET_ENTRY(ReplyPartyInvite)
	PACKET_ENTRY(ReplyPartyInvite2)
	PACKET_ENTRY(LeaveParty)
	PACKET_ENTRY(RemovePartyMember)
	PACKET_ENTRY(PartyChangeOption)
	PACKET_ENTRY(PartyMessage)
	PACKET_ENTRY(PartyChangeLeader)
	PACKET_ENTRY(PartyBookingRegisterReq)
	PACKET_ENTRY(PartyBookingSearchReq)
	PACKET_ENTRY(PartyBookingDeleteReq)
	PACKET_ENTRY(PartyBookingUpdateReq)
	PACKET_ENTRY(CloseVending)
	PACKET_ENTRY(VendingListReq)
	PACKET_ENTRY(PurchaseReq)
	PACKET_ENTRY(PurchaseReq2)
	PACKET_ENTRY(OpenVending)
	PACKET_ENTRY(CreateGuild)
	PACKET_ENTRY(GuildCheckMaster)
	PACKET_ENTRY(GuildRequestInfo)
	PACKET_ENTRY(GuildChangePositionInfo)
	PACKET_ENTRY(GuildChangeMemberPosition)
	PACKET_ENTRY(GuildRequestEmblem)
	PACKET_ENTRY(GuildChangeEmblem)
	PACKET_ENTRY(GuildChangeNotice)
	PACKET_ENTRY(GuildInvite)
	PACKET_ENTRY(GuildReplyInvite)
	PACKET_ENTRY(GuildLeave)
	PACKET_ENTRY(GuildExpulsion)
	PACKET_ENTRY(GuildMessage)
	PACKET_ENTRY(GuildRequestAlliance)
	PACKET_ENTRY(GuildReplyAlliance)
	PACKET_ENTRY(GuildDelAlliance)
	PACKET_ENTRY(GuildOpposition)
	PACKET_ENTRY(GuildBreak)
	PACKET_ENTRY(PetMenu)
	PACKET_ENTRY(CatchPet)
	PACKET_ENTRY(SelectEgg)
	PACKET_ENTRY(SendEmotion)
	PACKET_ENTRY(ChangePetName)
	PACKET_ENTRY(GMKick)
	PACKET_ENTRY(GMKickAll)
	PACKET_ENTRY(GMShift)
	PACKET_ENTRY(GMRemove2)
	PACKET_ENTRY(GMRecall)
	PACKET_ENTRY(GMRecall2)
	PACKET_ENTRY(GM_Monster_Item)
	PACKET_ENTRY(GMHide)
	PACKET_ENTRY(GMReqNoChat)
	PACKET_ENTRY(GMRc)
	PACKET_ENTRY(GMReqAccountName)
	PACKET_ENTRY(GMChangeMapType)
	PACKET_ENTRY(PMIgnore)
	PACKET_ENTRY(PMIgnoreAll)
	PACKET_ENTRY(PMIgnoreList)
	PACKET_ENTRY(NoviceDoriDori)
	PACKET_ENTRY(NoviceExplosionSpirits)
	PACKET_ENTRY(FriendsListAdd)
	PACKET_ENTRY(FriendsListReply)
	PACKET_ENTRY(FriendsListRemove)
	PACKET_ENTRY(PVPInfo)
	PACKET_ENTRY(Blacksmith)
	PACKET_ENTRY(Alchemist)
	PACKET_ENTRY(Taekwon)
	PACKET_ENTRY(RankingPk)
	PACKET_ENTRY(FeelSaveOk)
	PACKET_ENTRY(ChangeHomunculusName)
	PACKET_ENTRY(HomMoveToMaster)
	PACKET_ENTRY(HomMoveTo)
	PACKET_ENTRY(HomAttack)
	PACKET_ENTRY(HomMenu)
	PACKET_ENTRY(AutoRevive)
	PACKET_ENTRY(Check)
	PACKET_ENTRY(Mail_refreshinbox)
	PACKET_ENTRY(Mail_read)
	PACKET_ENTRY(Mail_getattach)
	PACKET_ENTRY(Mail_delete)
	PACKET_ENTRY(Mail_return)
	PACKET_ENTRY(Mail_setattach)
	PACKET_ENTRY(Mail_winopen)
	PACKET_ENTRY(Mail_send)
	PACKET_ENTRY(Auction_cancelreg)
	PACKET_ENTRY(Auction_setitem)
	PACKET_ENTRY(Auction_register)
	PACKET_ENTRY(Auction_cancel)
	PACKET_ENTRY(Auction_close)
	PACKET_ENTRY(Auction_bid)
	PACKET_ENTRY(Auction_search)
	PACKET_ENTRY(Auction_buysell)
	PACKET_ENTRY(cashshop_buy)
	PACKET_ENTRY(Adopt_request)
	PACKET_ENTRY(Adopt_reply)
	PACKET_ENTRY(ViewPlayerEquip)
	PACKET_ENTRY(EquipTick)
	PACKET_ENTRY(questStateAck)
	PACKET_ENTRY(mercenary_action)
	PACKET_ENTRY(BattleChat)
	PACKET_ENTRY(LessEffect)
	PACKET_ENTRY(ItemListWindowSelected)
	PACKET_ENTRY(ReqOpenBuyingStore)
	PACKET_ENTRY(ReqCloseBuyingStore)
	PACKET_ENTRY(ReqClickBuyingStore)
	PACKET_ENTRY(ReqTradeBuyingStore)
	PACKET_ENTRY(SearchStoreInfo)
	PACKET_ENTRY(SearchStoreInfoNextPage)
	PACKET_ENTRY(CloseSearchStoreInfo)
	PACKET_ENTRY(SearchStoreInfoListItemClick)
	PACKET_ENTRY(Debug)
	PACKET_ENTRY(SkillSelectMenu)
	PACKET_ENTRY(MoveItem)
	PACKET_ENTRY(Dull)
};

#undef p

extern struct clif_interface *clif;
extern void clif_defaults(void);

#endif /* _CLIF_H_ */
