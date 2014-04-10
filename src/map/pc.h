// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_PC_H_
#define _MAP_PC_H_

#include "../common/mmo.h" // JOB_*, MAX_FAME_LIST, struct fame_list, struct mmo_charstatus
#include "../common/ers.h"
#include "../common/timer.h" // INVALID_TIMER
#include "atcommand.h" // AtCommandType
#include "battle.h" // battle_config
#include "battleground.h"
#include "buyingstore.h"  // struct s_buyingstore
#include "itemdb.h"
#include "log.h"
#include "map.h" // RC_MAX
#include "mob.h"
#include "pc_groups.h"
#include "script.h" // struct script_reg, struct script_regstr
#include "searchstore.h"  // struct s_search_store_info
#include "status.h" // OPTION_*, struct weapon_atk
#include "unit.h" // unit_stop_attack(), unit_stop_walking()
#include "vending.h" // struct s_vending

/**
 * Defines
 **/
#define MAX_PC_BONUS 10
#define MAX_PC_SKILL_REQUIRE 5
#define MAX_PC_FEELHATE 3
#define PVP_CALCRANK_INTERVAL 1000	// PVP calculation interval

//Equip indexes constants. (eg: sd->equip_index[EQI_AMMO] returns the index
//where the arrows are equipped)
enum equip_index {
	EQI_ACC_L = 0,
	EQI_ACC_R,
	EQI_SHOES,
	EQI_GARMENT,
	EQI_HEAD_LOW,
	EQI_HEAD_MID,
	EQI_HEAD_TOP,
	EQI_ARMOR,
	EQI_HAND_L,
	EQI_HAND_R,
	EQI_COSTUME_TOP,
	EQI_COSTUME_MID,
	EQI_COSTUME_LOW,
	EQI_COSTUME_GARMENT,
	EQI_AMMO,
	EQI_SHADOW_ARMOR,
	EQI_SHADOW_WEAPON,
	EQI_SHADOW_SHIELD,
	EQI_SHADOW_SHOES,
	EQI_SHADOW_ACC_R,
	EQI_SHADOW_ACC_L,
	EQI_MAX
};
struct weapon_data {
	int atkmods[3];
	// all the variables except atkmods get zero'ed in each call of status_calc_pc
	// NOTE: if you want to add a non-zeroed variable, you need to update the memset call
	//  in status_calc_pc as well! All the following are automatically zero'ed. [Skotlex]
	int overrefine;
	int star;
	int ignore_def_ele;
	int ignore_def_race;
	int def_ratio_atk_ele;
	int def_ratio_atk_race;
	int addele[ELE_MAX];
	int addrace[RC_MAX];
	int addrace2[RC2_MAX];
	int addsize[3];
	struct drain_data {
		short rate;
		short per;
		short value;
		unsigned type:1;
	} hp_drain[RC_MAX], sp_drain[RC_MAX];
	struct {
		short class_, rate;
	}	add_dmg[MAX_PC_BONUS];
	struct {
		short flag, rate;
		unsigned char ele;
	} addele2[MAX_PC_BONUS];
};
struct s_autospell {
	short id, lv, rate, card_id, flag;
	bool lock;  // bAutoSpellOnSkill: blocks autospell from triggering again, while being executed
};
struct s_addeffect {
	enum sc_type id;
	short rate, arrow_rate;
	unsigned char flag;
};
struct s_addeffectonskill {
	enum sc_type id;
	short rate, skill;
	unsigned char target;
};
struct s_add_drop {
	short id, group;
	int race, rate;
};
struct s_autobonus {
	short rate,atk_type;
	unsigned int duration;
	char *bonus_script, *other_script;
	int active;
	unsigned short pos;
};
enum npc_timeout_type {
	NPCT_INPUT = 0,
	NPCT_MENU  = 1,
	NPCT_WAIT  = 2,
};

struct pc_combos {
	struct script_code *bonus;/* the script of the combo */
	unsigned short id;/* this combo id */
};

struct map_session_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;
	struct regen_data_sub sregen, ssregen;
	//NOTE: When deciding to add a flag to state or special_state, take into consideration that state is preserved in
	//status_calc_pc, while special_state is recalculated in each call. [Skotlex]
	struct {
		unsigned int active : 1; //Marks active player (not active is logging in/out, or changing map servers)
		unsigned int menu_or_input : 1;// if a script is waiting for feedback from the player
		unsigned int dead_sit : 2;
		unsigned int lr_flag : 3;//1: left h. weapon; 2: arrow; 3: shield
		unsigned int connect_new : 1;
		unsigned int arrow_atk : 1;
		unsigned int gangsterparadise : 1;
		unsigned int rest : 1;
		unsigned int storage_flag : 2; //0: closed, 1: Normal Storage open, 2: guild storage open [Skotlex]
		unsigned int snovice_dead_flag : 1; //Explosion spirits on death: 0 off, 1 used.
		unsigned int abra_flag : 2; // Abracadabra bugfix by Aru
		unsigned int autocast : 1; // Autospell flag [Inkfish]
		unsigned int autotrade : 1;	//By Fantik
		unsigned int showdelay :1;
		unsigned int showexp :1;
		unsigned int showzeny :1;
		unsigned int noask :1; // [LuzZza]
		unsigned int trading :1; //[Skotlex] is 1 only after a trade has started.
		unsigned int deal_locked :2; //1: Clicked on OK. 2: Clicked on TRADE
		unsigned int monster_ignore :1; // for monsters to ignore a character [Valaris] [zzo]
		unsigned int size :2; // for tiny/large types
		unsigned int night :1; //Holds whether or not the player currently has the SI_NIGHT effect on. [Skotlex]
		unsigned int blockedmove :1;
		unsigned int using_fake_npc :1;
		unsigned int rewarp :1; //Signals that a player should warp as soon as he is done loading a map. [Skotlex]
		unsigned int killer : 1;
		unsigned int killable : 1;
		unsigned int doridori : 1;
		unsigned int ignoreAll : 1;
		unsigned int debug_remove_map : 1; // temporary state to track double remove_map's [FlavioJS]
		unsigned int buyingstore : 1;
		unsigned int lesseffect : 1;
		unsigned int vending : 1;
		unsigned int noks : 3; // [Zeph Kill Steal Protection]
		unsigned int changemap : 1;
		unsigned int callshop : 1; // flag to indicate that a script used callshop; on a shop
		short pmap; // Previous map on Map Change
		unsigned short autoloot;
		unsigned short autolootid[AUTOLOOTITEM_SIZE]; // [Zephyrus]
		unsigned short autoloottype;
		unsigned int autolooting : 1; //performance-saver, autolooting state for @alootid
		unsigned short autobonus; //flag to indicate if an autobonus is activated. [Inkfish]
		unsigned int gmaster_flag : 1;
		unsigned int prevend : 1;//used to flag wheather you've spent 40sp to open the vending or not.
		unsigned int warping : 1;//states whether you're in the middle of a warp processing
		unsigned int permanent_speed : 1; // When 1, speed cannot be changed through status_calc_pc().
		unsigned int dialog : 1;
		unsigned int prerefining : 1;
		unsigned int workinprogress : 3; // 1 = disable skill/item, 2 = disable npc interaction, 3 = disable both
		unsigned int hold_recalc : 1;
		unsigned int snovice_call_flag : 3; //Summon Angel (stage 1~3)
		unsigned int hpmeter_visible : 1;
		unsigned int itemcheck : 1;
		unsigned int standalone : 1;/* [Ind/Hercules <3] */
		unsigned int loggingout : 1;
	} state;
	struct {
		unsigned char no_weapon_damage, no_magic_damage, no_misc_damage;
		unsigned int restart_full_recover : 1;
		unsigned int no_castcancel : 1;
		unsigned int no_castcancel2 : 1;
		unsigned int no_sizefix : 1;
		unsigned int no_gemstone : 1;
		unsigned int intravision : 1; // Maya Purple Card effect [DracoRPG]
		unsigned int perfect_hiding : 1; // [Valaris]
		unsigned int no_knockback : 1;
		unsigned int bonus_coma : 1;
	} special_state;
	int login_id1, login_id2;
	unsigned short class_;	//This is the internal job ID used by the map server to simplify comparisons/queries/etc. [Skotlex]
	
	/// Groups & permissions
	int group_id;
	GroupSettings *group;
	unsigned int extra_temp_permissions; /* permissions from @addperm */
	
	struct mmo_charstatus status;
	struct item_data* inventory_data[MAX_INVENTORY]; // direct pointers to itemdb entries (faster than doing item_id lookups)
	short equip_index[EQI_MAX];
	unsigned int weight,max_weight;
	int cart_weight,cart_num,cart_weight_max;
	int fd;
	unsigned short mapindex;
	unsigned char head_dir; //0: Look forward. 1: Look right, 2: Look left.
	unsigned int client_tick;
	int npc_id,areanpc_id,npc_shopid,touching_id; //for script follow scriptoid;   ,npcid
	int npc_item_flag; //Marks the npc_id with which you can change equipments during interactions with said npc (see script command enable_itemuse)
	int npc_menu; // internal variable, used in npc menu handling
	int npc_amount;
	struct script_state *st;
	char npc_str[CHATBOX_SIZE]; // for passing npc input box text to script engine
	int npc_timer_id; //For player attached npc timers. [Skotlex]
	unsigned int chatID;
	int64 idletime;
	struct {
		int npc_id;
		int64 timeout;
	} progressbar; //Progress Bar [Inkfish]
	struct{
		char name[NAME_LENGTH];
	} ignore[MAX_IGNORE_LIST];
	int followtimer; // [MouseJstr]
	int followtarget;
	time_t emotionlasttime; // to limit flood with emotion packets
	short skillitem,skillitemlv;
	uint16 skill_id_old,skill_lv_old;
	uint16 skill_id_dance,skill_lv_dance;
	short cook_mastery; // range: [0,1999] [Inkfish]
	bool blockskill[MAX_SKILL];
	int cloneskill_id, reproduceskill_id;
	int menuskill_id, menuskill_val, menuskill_val2;
	int invincible_timer;
	int64 canlog_tick;
	int64 canuseitem_tick; // [Skotlex]
	int64 canusecashfood_tick;
	int64 canequip_tick; // [Inkfish]
	int64 cantalk_tick;
	int64 canskill_tick;        /// used to prevent abuse from no-delay ACT files
	int64 cansendmail_tick;     /// Mail System Flood Protection
	int64 ks_floodprotect_tick; /// [Kill Steal Protection]
	struct {
		short nameid;
		int64 tick;
	} item_delay[MAX_ITEMDELAYS]; // [Paradox924X]
	short weapontype1,weapontype2;
	short disguise; // [Valaris]
	struct weapon_data right_weapon, left_weapon;
	// here start arrays to be globally zeroed at the beginning of status_calc_pc()
	int param_bonus[6],param_equip[6]; //Stores card/equipment bonuses.
	int subele[ELE_MAX];
	int subrace[RC_MAX];
	int subrace2[RC2_MAX];
	int subsize[3];
	int reseff[SC_COMMON_MAX-SC_COMMON_MIN+1];
	int weapon_coma_ele[ELE_MAX];
	int weapon_coma_race[RC_MAX];
	int weapon_atk[16];
	int weapon_atk_rate[16];
	int arrow_addele[ELE_MAX];
	int arrow_addrace[RC_MAX];
	int arrow_addsize[3];
	int magic_addele[ELE_MAX];
	int magic_addrace[RC_MAX];
	int magic_addsize[3];
	int magic_atk_ele[ELE_MAX];
	int critaddrace[RC_MAX];
	int expaddrace[RC_MAX];
	int ignore_mdef[RC_MAX];
	int ignore_def[RC_MAX];
	short sp_gain_race[RC_MAX];
	short sp_gain_race_attack[RC_MAX];
	short hp_gain_race_attack[RC_MAX];
	// zeroed arrays end here.
	// zeroed structures start here
	struct s_autospell autospell[15], autospell2[15], autospell3[15];
	struct s_addeffect addeff[MAX_PC_BONUS], addeff2[MAX_PC_BONUS];
	struct s_addeffectonskill addeff3[MAX_PC_BONUS];
	struct { //skillatk raises bonus dmg% of skills, skillheal increases heal%, skillblown increases bonus blewcount for some skills.
		unsigned short id;
		short val;
	} skillatk[MAX_PC_BONUS], skillusesprate[MAX_PC_BONUS], skillusesp[MAX_PC_BONUS], skillheal[5], skillheal2[5], skillblown[MAX_PC_BONUS], skillcast[MAX_PC_BONUS], skillcooldown[MAX_PC_BONUS], skillfixcast[MAX_PC_BONUS], skillvarcast[MAX_PC_BONUS], skillfixcastrate[MAX_PC_BONUS];
	struct {
		short value;
		int rate;
		int tick;
	} hp_loss, sp_loss, hp_regen, sp_regen;
	struct {
		short class_, rate;
	}	add_def[MAX_PC_BONUS], add_mdef[MAX_PC_BONUS], add_mdmg[MAX_PC_BONUS];
	struct s_add_drop add_drop[MAX_PC_BONUS];
	struct {
		int nameid;
		int rate;
	} itemhealrate[MAX_PC_BONUS];
	struct {
		short flag, rate;
		unsigned char ele;
	} subele2[MAX_PC_BONUS];
	// zeroed structures end here
	// manually zeroed structures start here.
	struct s_autobonus autobonus[MAX_PC_BONUS], autobonus2[MAX_PC_BONUS], autobonus3[MAX_PC_BONUS]; //Auto script on attack, when attacked, on skill usage
	// manually zeroed structures end here.
	// zeroed vars start here.
	struct {
		int atk_rate;
		int arrow_atk,arrow_ele,arrow_cri,arrow_hit;
		int nsshealhp,nsshealsp;
		int critical_def,double_rate;
		int long_attack_atk_rate; //Long range atk rate, not weapon based. [Skotlex]
		int near_attack_def_rate,long_attack_def_rate,magic_def_rate,misc_def_rate;
		int ignore_mdef_ele;
		int ignore_mdef_race;
		int perfect_hit;
		int perfect_hit_add;
		int get_zeny_rate;
		int get_zeny_num; //Added Get Zeny Rate [Skotlex]
		int double_add_rate;
		int short_weapon_damage_return,long_weapon_damage_return;
		int magic_damage_return; // AppleGirl Was Here
		int break_weapon_rate,break_armor_rate;
		int crit_atk_rate;
		int classchange; // [Valaris]
		int speed_rate, speed_add_rate, aspd_add;
		int itemhealrate2; // [Epoque] Increase heal rate of all healing items.
		int shieldmdef;//royal guard's
		unsigned int setitem_hash, setitem_hash2; //Split in 2 because shift operations only work on int ranges. [Skotlex]
		short splash_range, splash_add_range;
		short add_steal_rate;
		short add_heal_rate, add_heal2_rate;
		short sp_gain_value, hp_gain_value, magic_sp_gain_value, magic_hp_gain_value;
		short sp_vanish_rate;
		short sp_vanish_per;
		unsigned short unbreakable;	// chance to prevent ANY equipment breaking [celest]
		unsigned short unbreakable_equip; //100% break resistance on certain equipment
		unsigned short unstripable_equip;
		int fixcastrate,varcastrate;
		int add_fixcast,add_varcast;
		int ematk; // matk bonus from equipment
	} bonus;
	// zeroed vars end here.
	int castrate,delayrate,hprate,sprate,dsprate;
	int hprecov_rate,sprecov_rate;
	int matk_rate;
	int critical_rate,hit_rate,flee_rate,flee2_rate,def_rate,def2_rate,mdef_rate,mdef2_rate;
	int itemid;
	short itemindex;	//Used item's index in sd->inventory [Skotlex]
	short catch_target_class; // pet catching, stores a pet class to catch (short now) [zzo]
	short spiritball, spiritball_old;
	int spirit_timer[MAX_SPIRITBALL];
	short charm[ELE_POISON+1]; // There are actually 5 charm Fire, Ice, Wind, Earth & Poison maybe because its color violet.
	int charm_timer[ELE_POISON+1][10];
	unsigned char potion_success_counter; //Potion successes in row counter
	unsigned char mission_count; //Stores the bounty kill count for TK_MISSION
	short mission_mobid; //Stores the target mob_id for TK_MISSION
	int die_counter; //Total number of times you've died
	int devotion[5]; //Stores the account IDs of chars devoted to.
	int trade_partner;
	struct {
		struct {
			short index, amount;
		} item[10];
		int zeny, weight;
	} deal;
	bool party_creating; // whether the char is requesting party creation
	bool party_joining; // whether the char is accepting party invitation
	int party_invite, party_invite_account; // for handling party invitation (holds party id and account id)
	int adopt_invite; // Adoption
	struct guild *guild;/* [Ind/Hercules] speed everything up */
	int guild_invite,guild_invite_account;
	int guild_emblem_id,guild_alliance,guild_alliance_account;
	short guild_x,guild_y; // For guildmate position display. [Skotlex] should be short [zzo]
	int guildspy; // [Syrus22]
	int partyspy; // [Syrus22]
	unsigned int vended_id;
	unsigned int vender_id;
	int vend_num;
	char message[MESSAGE_SIZE];
	struct s_vending vending[MAX_VENDING];
	unsigned int buyer_id;  // uid of open buying store
	struct s_buyingstore buyingstore;
	struct s_search_store_info searchstore;

	struct pet_data *pd;
	struct homun_data *hd;	// [blackhole89]
	struct mercenary_data *md;
	struct elemental_data *ed;

	struct{
		int  m; //-1 - none, other: map index corresponding to map name.
		unsigned short index; //map index
	} feel_map[3];// 0 - Sun; 1 - Moon; 2 - Stars
	short hate_mob[3];

	int pvp_timer;
	short pvp_point;
	unsigned short pvp_rank, pvp_lastusers;
	unsigned short pvp_won, pvp_lost;

	char eventqueue[MAX_EVENTQUEUE][EVENT_NAME_LENGTH];
	int eventtimer[MAX_EVENTTIMER];
	unsigned short eventcount; // [celest]

	unsigned char change_level_2nd; // job level when changing from 1st to 2nd class [jobchange_level in global_reg_value]
	unsigned char change_level_3rd; // job level when changing from 2nd to 3rd class [jobchange_level_3rd in global_reg_value]

	char fakename[NAME_LENGTH]; // fake names [Valaris]

	int duel_group; // duel vars [LuzZza]
	int duel_invite;

	int killerrid, killedrid;

	int cashPoints, kafraPoints;
	int rental_timer;

	// Auction System [Zephyrus]
	struct {
		int index, amount;
	} auction;

	// Mail System [Zephyrus]
	struct {
		short nameid;
		int index, amount, zeny;
		struct mail_data inbox;
		bool changed; // if true, should sync with charserver on next mailbox request
	} mail;

	// Quest log system
	int num_quests;          ///< Number of entries in quest_log
	int avail_quests;        ///< Number of Q_ACTIVE and Q_INACTIVE entries in quest log (index of the first Q_COMPLETE entry)
	struct quest *quest_log; ///< Quest log entries (note: Q_COMPLETE quests follow the first <avail_quests>th enties
	bool save_quest;         ///< Whether the quest_log entries were modified and are waitin to be saved

	// temporary debug [flaviojs]
	const char* debug_file;
	int debug_line;
	const char* debug_func;

	unsigned int bg_id;

	/**
	 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
	 **/
#ifdef SECURE_NPCTIMEOUT
	/**
	 * ID of the timer
	 * @info
	 * - value is -1 (INVALID_TIMER constant) when not being used
	 * - timer is cancelled upon closure of the current npc's instance
	 **/
	int npc_idle_timer;
	/**
	 * Tick on the last recorded NPC iteration (next/menu/whatever)
	 * @info
	 * - It is updated on every NPC iteration as mentioned above
	 **/
	int64 npc_idle_tick;
	/* */
	enum npc_timeout_type npc_idle_type;
#endif

	struct pc_combos *combos;
	unsigned char combo_count;
	
	/**
	 * Guarantees your friend request is legit (for bugreport:4629)
	 **/
	int friend_req;

	int shadowform_id;
	
	/* [Ind/Hercules] */
	struct hChSysCh **channels;
	unsigned char channel_count;
	struct hChSysCh *gcbind;
	bool stealth;
	unsigned char fontcolor;
	unsigned int fontcolor_tid;
	int64 hchsysch_tick;
	
	/* [Ind/Hercules] */
	struct sc_display_entry **sc_display;
	unsigned char sc_display_count;
	
	short *instance;
	unsigned short instances;
	
	/* Possible Thanks to Yommy~! */
	struct {
		unsigned int ready : 1;/* did he accept the 'match is about to start, enter' dialog? */
		unsigned int client_has_bg_data : 1; /* flags whether the client has the "in queue" window (aka the client knows it is in a queue) */
		struct bg_arena *arena;
		enum bg_queue_types type;
	} bg_queue;

	int *queues;
	unsigned int queues_count;
	
	/* Made Possible Thanks to Yommy~! */
	unsigned int cryptKey;                                                 ///< Packet obfuscation key to be used for the next received packet
	unsigned short (*parse_cmd_func)(int fd, struct map_session_data *sd); ///< parse_cmd_func used by this player
	
	unsigned char delayed_damage;//ref. counter bugreport:7307 [Ind/Hercules]
	
	/* HPM Custom Struct */
	struct HPluginData **hdata;
	unsigned int hdatac;
	
	/* expiration_time timer id */
	int expiration_tid;
	time_t expiration_time;
	
	/* */
	struct {
		unsigned int second,third;
	} sktree;
	
	/**
	 * Account/Char variables & array control of those variables
	 **/
	struct reg_db regs;
	unsigned char vars_received;/* char loading is only complete when you get it all. */
	bool vars_ok;
	bool vars_dirty;
	
	// temporary debugging of bug #3504
	const char* delunit_prevfile;
	int delunit_prevline;

};

#define EQP_WEAPON EQP_HAND_R
#define EQP_SHIELD EQP_HAND_L
#define EQP_ARMS (EQP_HAND_R|EQP_HAND_L)
#define EQP_HELM (EQP_HEAD_LOW|EQP_HEAD_MID|EQP_HEAD_TOP)
#define EQP_ACC (EQP_ACC_L|EQP_ACC_R)
#define EQP_COSTUME (EQP_COSTUME_HEAD_TOP|EQP_COSTUME_HEAD_MID|EQP_COSTUME_HEAD_LOW|EQP_COSTUME_GARMENT)
#define EQP_SHADOW_ACC (EQP_SHADOW_ACC_R|EQP_SHADOW_ACC_L)
#define EQP_SHADOW_ARMS (EQP_SHADOW_WEAPON|EQP_SHADOW_SHIELD)

/// Equip positions that use a visible sprite
#if PACKETVER < 20110111
	#define EQP_VISIBLE EQP_HELM
#else
	#define EQP_VISIBLE (EQP_HELM|EQP_GARMENT|EQP_COSTUME)
#endif

#define pc_setdead(sd)        ( (sd)->state.dead_sit = (sd)->vd.dead_sit = 1 )
#define pc_setsit(sd)         ( (sd)->state.dead_sit = (sd)->vd.dead_sit = 2 )
#define pc_isdead(sd)         ( (sd)->state.dead_sit == 1 )
#define pc_issit(sd)          ( (sd)->vd.dead_sit == 2 )
#define pc_isidle(sd)         ( (sd)->chatID || (sd)->state.vending || (sd)->state.buyingstore || DIFF_TICK(sockt->last_tick, (sd)->idletime) >= battle_config.idle_no_share )
#define pc_istrading(sd)      ( (sd)->npc_id || (sd)->state.vending || (sd)->state.buyingstore || (sd)->state.trading )
#define pc_cant_act(sd)       ( (sd)->npc_id || (sd)->state.vending || (sd)->state.buyingstore || (sd)->chatID || ((sd)->sc.opt1 && (sd)->sc.opt1 != OPT1_BURNING) || (sd)->state.trading || (sd)->state.storage_flag || (sd)->state.prevend )

/* equals pc_cant_act except it doesn't check for chat rooms */
#define pc_cant_act2(sd)       ( (sd)->npc_id || (sd)->state.buyingstore || ((sd)->sc.opt1 && (sd)->sc.opt1 != OPT1_BURNING) || (sd)->state.trading || (sd)->state.storage_flag || (sd)->state.prevend )

#define pc_setdir(sd,b,h)     ( (sd)->ud.dir = (b) ,(sd)->head_dir = (h) )
#define pc_setchatid(sd,n)    ( (sd)->chatID = n )
#define pc_ishiding(sd)       ( (sd)->sc.option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) )
#define pc_iscloaking(sd)     ( !((sd)->sc.option&OPTION_CHASEWALK) && ((sd)->sc.option&OPTION_CLOAK) )
#define pc_ischasewalk(sd)    ( (sd)->sc.option&OPTION_CHASEWALK )

#ifdef NEW_CARTS
	#define pc_iscarton(sd)       ( (sd)->sc.data[SC_PUSH_CART] )
#else
	#define pc_iscarton(sd)       ( (sd)->sc.option&OPTION_CART )
#endif

#define pc_isfalcon(sd)       ( (sd)->sc.option&OPTION_FALCON )
#define pc_isriding(sd)       ( (sd)->sc.option&OPTION_RIDING )
#define pc_isinvisible(sd)    ( (sd)->sc.option&OPTION_INVISIBLE )
#define pc_is50overweight(sd) ( (sd)->weight*100 >= (sd)->max_weight*battle_config.natural_heal_weight_rate )
#define pc_is90overweight(sd) ( (sd)->weight*10 >= (sd)->max_weight*9 )
#define pc_maxparameter(sd)   ( ((((sd)->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO) || (((sd)->class_&MAPID_UPPERMASK) == MAPID_REBELLION) || (sd)->class_&JOBL_THIRD ? ((sd)->class_&JOBL_BABY ? battle_config.max_baby_third_parameter : battle_config.max_third_parameter) : ((sd)->class_&JOBL_BABY ? battle_config.max_baby_parameter : battle_config.max_parameter)) )
/**
 * Ranger
 **/
#define pc_iswug(sd)       ( (sd)->sc.option&OPTION_WUG )
#define pc_isridingwug(sd) ( (sd)->sc.option&OPTION_WUGRIDER )
// Mechanic Magic Gear
#define pc_ismadogear(sd) ( (sd)->sc.option&OPTION_MADOGEAR )
// Rune Knight Dragon
#define pc_isridingdragon(sd) ( (sd)->sc.option&OPTION_DRAGON )

#define pc_stop_walking(sd, type) (unit->stop_walking(&(sd)->bl, (type)))
#define pc_stop_attack(sd)        (unit->stop_attack(&(sd)->bl))

//Weapon check considering dual wielding.
#define pc_check_weapontype(sd, type) ((type)&((sd)->status.weapon < MAX_WEAPON_TYPE? \
	1<<(sd)->status.weapon:(1<<(sd)->weapontype1)|(1<<(sd)->weapontype2)|(1<<(sd)->status.weapon)))
//Checks if the given class value corresponds to a player class. [Skotlex]
//JOB_NOVICE isn't checked for class_ is supposed to be unsigned
#define pcdb_checkid_sub(class_) \
( \
	( (class_) <  JOB_MAX_BASIC ) \
||	( (class_) >= JOB_NOVICE_HIGH    && (class_) <= JOB_DARK_COLLECTOR ) \
||	( (class_) >= JOB_RUNE_KNIGHT    && (class_) <= JOB_MECHANIC_T2    ) \
||	( (class_) >= JOB_BABY_RUNE      && (class_) <= JOB_BABY_MECHANIC2 ) \
||	( (class_) >= JOB_SUPER_NOVICE_E && (class_) <= JOB_SUPER_BABY_E   ) \
||	( (class_) >= JOB_KAGEROU        && (class_) <= JOB_OBORO          ) \
||	( (class_) >= JOB_REBELLION      && (class_) <  JOB_MAX            ) \
)
#define pcdb_checkid(class_) pcdb_checkid_sub((unsigned int)(class_))

// clientside display macros (values to the left/right of the "+")
#ifdef RENEWAL
	#define pc_leftside_atk(sd) ((sd)->battle_status.batk)
	#define pc_rightside_atk(sd) ((sd)->battle_status.rhw.atk + (sd)->battle_status.lhw.atk + (sd)->battle_status.rhw.atk2 + (sd)->battle_status.lhw.atk2 + (sd)->battle_status.equip_atk )
	#define pc_leftside_def(sd) ((sd)->battle_status.def2)
	#define pc_rightside_def(sd) ((sd)->battle_status.def)
	#define pc_leftside_mdef(sd) ((sd)->battle_status.mdef2)
	#define pc_rightside_mdef(sd) ((sd)->battle_status.mdef)
#define pc_leftside_matk(sd) (status->base_matk(status->get_status_data(&(sd)->bl), (sd)->status.base_level))
#define pc_rightside_matk(sd) ((sd)->battle_status.rhw.matk+(sd)->battle_status.lhw.matk+(sd)->bonus.ematk)
#else
	#define pc_leftside_atk(sd) ((sd)->battle_status.batk + (sd)->battle_status.rhw.atk + (sd)->battle_status.lhw.atk)
	#define pc_rightside_atk(sd) ((sd)->battle_status.rhw.atk2 + (sd)->battle_status.lhw.atk2)
	#define pc_leftside_def(sd) ((sd)->battle_status.def)
	#define pc_rightside_def(sd) ((sd)->battle_status.def2)
	#define pc_leftside_mdef(sd) ((sd)->battle_status.mdef)
	#define pc_rightside_mdef(sd) ( (sd)->battle_status.mdef2 - ((sd)->battle_status.vit>>1) )
#define pc_leftside_matk(sd) \
    (\
    ((sd)->sc.data[SC_MAGICPOWER] && (sd)->sc.data[SC_MAGICPOWER]->val4) \
		?((sd)->battle_status.matk_min * 100 + 50) / ((sd)->sc.data[SC_MAGICPOWER]->val3+100) \
        :(sd)->battle_status.matk_min \
    )
#define pc_rightside_matk(sd) \
    (\
    ((sd)->sc.data[SC_MAGICPOWER] && (sd)->sc.data[SC_MAGICPOWER]->val4) \
		?((sd)->battle_status.matk_max * 100 + 50) / ((sd)->sc.data[SC_MAGICPOWER]->val3+100) \
        :(sd)->battle_status.matk_max \
    )
#endif

#define pc_get_group_id(sd) ( (sd)->group_id )

#define pc_checkoverhp(sd) ((sd)->battle_status.hp == (sd)->battle_status.max_hp)
#define pc_checkoversp(sd) ((sd)->battle_status.sp == (sd)->battle_status.max_sp)

#define pc_readglobalreg(sd,reg)         (pc->readregistry((sd),(reg)))
#define pc_setglobalreg(sd,reg,val)      (pc->setregistry((sd),(reg),(val)))
#define pc_readglobalreg_str(sd,reg)     (pc->readregistry_str((sd),(reg)))
#define pc_setglobalreg_str(sd,reg,val)  (pc->setregistry_str((sd),(reg),(val)))
#define pc_readaccountreg(sd,reg)        (pc->readregistry((sd),(reg)))
#define pc_setaccountreg(sd,reg,val)     (pc->setregistry((sd),(reg),(val)))
#define pc_readaccountregstr(sd,reg)     (pc->readregistry_str((sd),(reg)))
#define pc_setaccountregstr(sd,reg,val)  (pc->setregistry_str((sd),(reg),(val)))
#define pc_readaccountreg2(sd,reg)       (pc->readregistry((sd),(reg)))
#define pc_setaccountreg2(sd,reg,val)    (pc->setregistry((sd),(reg),(val)))
#define pc_readaccountreg2str(sd,reg)    (pc->readregistry_str((sd),(reg)))
#define pc_setaccountreg2str(sd,reg,val) (pc->setregistry_str((sd),(reg),(val)))

/* pc_groups easy access */
#define pc_get_group_level(sd) ( (sd)->group->level )
#define pc_has_permission(sd,permission) ( ((sd)->extra_temp_permissions&(permission)) != 0 || ((sd)->group->e_permissions&(permission)) != 0 )
#define pc_can_give_items(sd) ( pc_has_permission((sd),PC_PERM_TRADE) )
#define pc_can_give_bound_items(sd) ( pc_has_permission((sd),PC_PERM_TRADE_BOUND) )

struct skill_tree_entry {
	short id;
	unsigned short idx;
	unsigned char max;
	unsigned char joblv;
	struct {
		short id;
		unsigned short idx;
		unsigned char lv;
	} need[MAX_PC_SKILL_REQUIRE];
}; // Celest

struct sg_data {
	short anger_id;
	short bless_id;
	short comfort_id;
	char feel_var[NAME_LENGTH];
	char hate_var[NAME_LENGTH];
	bool (*day_func)(void);
};

enum { ADDITEM_EXIST , ADDITEM_NEW , ADDITEM_OVERAMOUNT };

/**
 * Item Cool Down Delay Saving
 * Struct item_cd is not a member of struct map_session_data
 * to keep cooldowns in memory between player log-ins.
 * All cooldowns are reset when server is restarted.
 **/
struct item_cd {
	int64 tick[MAX_ITEMDELAYS];//tick
	short nameid[MAX_ITEMDELAYS];//skill id
};

enum e_pc_autotrade_update_action {
	PAUC_START,
	PAUC_REFRESH,
	PAUC_REMOVE,
};

/**
 * Used to temporarily remember vending data
 **/
struct autotrade_vending {
	struct item list[MAX_VENDING];
	struct s_vending vending[MAX_VENDING];
	unsigned char vend_num;
};

/*=====================================
* Interface : pc.h 
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
struct pc_interface {

	/* */
	DBMap *at_db;/* char id -> struct autotrade_vending */
	/* */
	DBMap* itemcd_db;
	/* */
	int day_timer_tid;
	int night_timer_tid;
	/* */
	unsigned int exp_table[CLASS_COUNT][2][MAX_LEVEL];
	unsigned int max_level[CLASS_COUNT][2];
	unsigned int statp[MAX_LEVEL+1];
#if defined(RENEWAL_DROP) || defined(RENEWAL_EXP)
	unsigned int level_penalty[3][RC_MAX][MAX_LEVEL*2+1];
#endif
	unsigned int equip_pos[EQI_MAX];
	/* */
	struct skill_tree_entry skill_tree[CLASS_COUNT][MAX_SKILL_TREE];
	struct fame_list smith_fame_list[MAX_FAME_LIST];
	struct fame_list chemist_fame_list[MAX_FAME_LIST];
	struct fame_list taekwon_fame_list[MAX_FAME_LIST];
	struct sg_data sg_info[MAX_PC_FEELHATE];
	/* */
	struct eri *sc_display_ers;
	/* global expiration timer id */
	int expiration_tid;
	/**
	 * ERS for the bulk of pc vars
	 **/
	struct eri *num_reg_ers;
	struct eri *str_reg_ers;
	/* */
	bool reg_load;
	/* funcs */
	void (*init) (bool minimal);
	void (*final) (void);
	
	struct map_session_data* (*get_dummy_sd) (void);
	int (*class2idx) (int class_);
	//int (*getrefinebonus) (int lv,int type); FIXME: This function does not exist, nor it is ever called
	bool (*can_give_items) (struct map_session_data *sd);
	bool (*can_give_bound_items) (struct map_session_data *sd);
 	
	bool (*can_use_command) (struct map_session_data *sd, const char *command);
	int (*set_group) (struct map_session_data *sd, int group_id);
	bool (*should_log_commands) (struct map_session_data *sd);

	int (*setrestartvalue) (struct map_session_data *sd,int type);
	int (*makesavestatus) (struct map_session_data *sd);
	void (*respawn) (struct map_session_data* sd, clr_type clrtype);
	int (*setnewpc) (struct map_session_data *sd, int account_id, int char_id, int login_id1, unsigned int client_tick, int sex, int fd);
	bool (*authok) (struct map_session_data *sd, int login_id2, time_t expiration_time, int group_id, struct mmo_charstatus *st, bool changing_mapservers);
	void (*authfail) (struct map_session_data *sd);
	int (*reg_received) (struct map_session_data *sd);
	
	int (*isequip) (struct map_session_data *sd,int n);
	int (*equippoint) (struct map_session_data *sd,int n);
	int (*setinventorydata) (struct map_session_data *sd);
	
	int (*checkskill) (struct map_session_data *sd,uint16 skill_id);
	int (*checkskill2) (struct map_session_data *sd,uint16 index);
	int (*checkallowskill) (struct map_session_data *sd);
	int (*checkequip) (struct map_session_data *sd,int pos);
	
	int (*calc_skilltree) (struct map_session_data *sd);
	int (*calc_skilltree_normalize_job) (struct map_session_data *sd);
	int (*clean_skilltree) (struct map_session_data *sd);
	
	int (*setpos) (struct map_session_data* sd, unsigned short map_index, int x, int y, clr_type clrtype);
	int (*setsavepoint) (struct map_session_data *sd, short map_index, int x, int y);
	int (*randomwarp) (struct map_session_data *sd,clr_type type);
	int (*memo) (struct map_session_data* sd, int pos);
	
	int (*checkadditem) (struct map_session_data *sd,int nameid,int amount);
	int (*inventoryblank) (struct map_session_data *sd);
	int (*search_inventory) (struct map_session_data *sd,int item_id);
	int (*payzeny) (struct map_session_data *sd,int zeny, enum e_log_pick_type type, struct map_session_data *tsd);
	int (*additem) (struct map_session_data *sd,struct item *item_data,int amount,e_log_pick_type log_type);
	int (*getzeny) (struct map_session_data *sd,int zeny, enum e_log_pick_type type, struct map_session_data *tsd);
	int (*delitem) (struct map_session_data *sd,int n,int amount,int type, short reason, e_log_pick_type log_type);

	// Special Shop System
	int (*paycash) (struct map_session_data *sd, int price, int points);
	int (*getcash) (struct map_session_data *sd, int cash, int points);
	
	int (*cart_additem) (struct map_session_data *sd,struct item *item_data,int amount,e_log_pick_type log_type);
	int (*cart_delitem) (struct map_session_data *sd,int n,int amount,int type,e_log_pick_type log_type);
	int (*putitemtocart) (struct map_session_data *sd,int idx,int amount);
	int (*getitemfromcart) (struct map_session_data *sd,int idx,int amount);
	int (*cartitem_amount) (struct map_session_data *sd,int idx,int amount);
	
	int (*takeitem) (struct map_session_data *sd,struct flooritem_data *fitem);
	int (*dropitem) (struct map_session_data *sd,int n,int amount);
	
	bool (*isequipped) (struct map_session_data *sd, int nameid);
	bool (*can_Adopt) (struct map_session_data *p1_sd, struct map_session_data *p2_sd, struct map_session_data *b_sd);
	bool (*adoption) (struct map_session_data *p1_sd, struct map_session_data *p2_sd, struct map_session_data *b_sd);
	
	int (*updateweightstatus) (struct map_session_data *sd);
	
	int (*addautobonus) (struct s_autobonus *bonus,char max,const char *bonus_script,short rate,unsigned int dur,short atk_type,const char *o_script,unsigned short pos,bool onskill);
	int (*exeautobonus) (struct map_session_data* sd,struct s_autobonus *bonus);
	int (*endautobonus) (int tid, int64 tick, int id, intptr_t data);
	int (*delautobonus) (struct map_session_data* sd,struct s_autobonus *bonus,char max,bool restore);
	
	int (*bonus) (struct map_session_data *sd,int type,int val);
	int (*bonus2) (struct map_session_data *sd,int type,int type2,int val);
	int (*bonus3) (struct map_session_data *sd,int type,int type2,int type3,int val);
	int (*bonus4) (struct map_session_data *sd,int type,int type2,int type3,int type4,int val);
	int (*bonus5) (struct map_session_data *sd,int type,int type2,int type3,int type4,int type5,int val);
	int (*skill) (struct map_session_data *sd, int id, int level, int flag);
	
	int (*insert_card) (struct map_session_data *sd,int idx_card,int idx_equip);
	
	int (*steal_item) (struct map_session_data *sd,struct block_list *bl, uint16 skill_lv);
	int (*steal_coin) (struct map_session_data *sd,struct block_list *bl);
	
	int (*modifybuyvalue) (struct map_session_data *sd,int orig_value);
	int (*modifysellvalue) (struct map_session_data *sd,int orig_value);
	
	int (*follow) (struct map_session_data *sd, int target_id); // [MouseJstr]
	int (*stop_following) (struct map_session_data *sd);
	
	unsigned int (*maxbaselv) (struct map_session_data *sd);
	unsigned int (*maxjoblv) (struct map_session_data *sd);
	int (*checkbaselevelup) (struct map_session_data *sd);
	int (*checkjoblevelup) (struct map_session_data *sd);
	int (*gainexp) (struct map_session_data *sd, struct block_list *src, unsigned int base_exp, unsigned int job_exp, bool is_quest);
	unsigned int (*nextbaseexp) (struct map_session_data *sd);
	unsigned int (*thisbaseexp) (struct map_session_data *sd);
	unsigned int (*nextjobexp) (struct map_session_data *sd);
	unsigned int (*thisjobexp) (struct map_session_data *sd);
	int (*gets_status_point) (int level);
	int (*need_status_point) (struct map_session_data *sd,int type,int val);
	int (*maxparameterincrease) (struct map_session_data* sd, int type);
	bool (*statusup) (struct map_session_data *sd, int type, int increase);
	int (*statusup2) (struct map_session_data *sd,int type,int val);
	int (*skillup) (struct map_session_data *sd,uint16 skill_id);
	int (*allskillup) (struct map_session_data *sd);
	int (*resetlvl) (struct map_session_data *sd,int type);
	int (*resetstate) (struct map_session_data *sd);
	int (*resetskill) (struct map_session_data *sd, int flag);
	int (*resetfeel) (struct map_session_data *sd);
	int (*resethate) (struct map_session_data *sd);
	int (*equipitem) (struct map_session_data *sd,int n,int req_pos);
	int (*unequipitem) (struct map_session_data *sd,int n,int flag);
	int (*checkitem) (struct map_session_data *sd);
	int (*useitem) (struct map_session_data *sd,int n);
	
	int (*skillatk_bonus) (struct map_session_data *sd, uint16 skill_id);
	int (*skillheal_bonus) (struct map_session_data *sd, uint16 skill_id);
	int (*skillheal2_bonus) (struct map_session_data *sd, uint16 skill_id);
	
	void (*damage) (struct map_session_data *sd,struct block_list *src,unsigned int hp, unsigned int sp);
	int (*dead) (struct map_session_data *sd,struct block_list *src);
	void (*revive) (struct map_session_data *sd,unsigned int hp, unsigned int sp);
	void (*heal) (struct map_session_data *sd,unsigned int hp,unsigned int sp, int type);
	int (*itemheal) (struct map_session_data *sd,int itemid, int hp,int sp);
	int (*percentheal) (struct map_session_data *sd,int hp,int sp);
	int (*jobchange) (struct map_session_data *sd,int job, int upper);
	int (*setoption) (struct map_session_data *sd,int type);
	int (*setcart) (struct map_session_data* sd, int type);
	int (*setfalcon) (struct map_session_data* sd, int flag);
	int (*setriding) (struct map_session_data* sd, int flag);
	int (*setmadogear) (struct map_session_data* sd, int flag);
	int (*changelook) (struct map_session_data *sd,int type,int val);
	int (*equiplookall) (struct map_session_data *sd);
	
	int (*readparam) (struct map_session_data *sd,int type);
	int (*setparam) (struct map_session_data *sd,int type,int val);
	int (*readreg) (struct map_session_data *sd, int64 reg);
	void (*setreg) (struct map_session_data *sd, int64 reg,int val);
	char * (*readregstr) (struct map_session_data *sd, int64 reg);
	void (*setregstr) (struct map_session_data *sd, int64 reg, const char *str);
	int (*readregistry) (struct map_session_data *sd, int64 reg);
	int (*setregistry) (struct map_session_data *sd, int64 reg, int val);
	char * (*readregistry_str) (struct map_session_data *sd, int64 reg);
	int (*setregistry_str) (struct map_session_data *sd, int64 reg, const char *val);
	
	int (*addeventtimer) (struct map_session_data *sd,int tick,const char *name);
	int (*deleventtimer) (struct map_session_data *sd,const char *name);
	int (*cleareventtimer) (struct map_session_data *sd);
	int (*addeventtimercount) (struct map_session_data *sd,const char *name,int tick);
	
	int (*calc_pvprank) (struct map_session_data *sd);
	int (*calc_pvprank_timer) (int tid, int64 tick, int id, intptr_t data);
	
	int (*ismarried) (struct map_session_data *sd);
	int (*marriage) (struct map_session_data *sd,struct map_session_data *dstsd);
	int (*divorce) (struct map_session_data *sd);
	struct map_session_data * (*get_partner) (struct map_session_data *sd);
	struct map_session_data * (*get_father) (struct map_session_data *sd);
	struct map_session_data * (*get_mother) (struct map_session_data *sd);
	struct map_session_data * (*get_child) (struct map_session_data *sd);
	
	void (*bleeding) (struct map_session_data *sd, unsigned int diff_tick);
	void (*regen) (struct map_session_data *sd, unsigned int diff_tick);
	
	void (*setstand) (struct map_session_data *sd);
	int (*candrop) (struct map_session_data *sd,struct item *item);
	
	int (*jobid2mapid) (unsigned short b_class); // Skotlex
	int (*mapid2jobid) (unsigned short class_, int sex); // Skotlex
	
	const char * (*job_name) (int class_);
	
	void (*setinvincibletimer) (struct map_session_data* sd, int val);
	void (*delinvincibletimer) (struct map_session_data* sd);
	
	int (*addspiritball) (struct map_session_data *sd,int interval,int max);
	int (*delspiritball) (struct map_session_data *sd,int count,int type);
	void (*addfame) (struct map_session_data *sd,int count);
	unsigned char (*famerank) (int char_id, int job);
	int (*set_hate_mob) (struct map_session_data *sd, int pos, struct block_list *bl);
	
	int (*readdb) (void);
	int (*map_day_timer) (int tid, int64 tick, int id, intptr_t data); // by [yor]
	int (*map_night_timer) (int tid, int64 tick, int id, intptr_t data); // by [yor]
	// Rental System
	void (*inventory_rentals) (struct map_session_data *sd);
	int (*inventory_rental_clear) (struct map_session_data *sd);
	void (*inventory_rental_add) (struct map_session_data *sd, int seconds);
	
	int (*disguise) (struct map_session_data *sd, int class_);
	bool (*isautolooting) (struct map_session_data *sd, int nameid);
	
	void (*overheat) (struct map_session_data *sd, int val);
	
	int (*banding) (struct map_session_data *sd, uint16 skill_lv);
	
	void (*itemcd_do) (struct map_session_data *sd, bool load);
	
	int (*load_combo) (struct map_session_data *sd);
	
	int (*add_charm) (struct map_session_data *sd,int interval,int max,int type);
	int (*del_charm) (struct map_session_data *sd,int count,int type);
	
	void (*baselevelchanged) (struct map_session_data *sd);
	int (*level_penalty_mod) (int diff, unsigned char race, unsigned short mode, int type);
	int (*calc_skillpoint) (struct map_session_data* sd);
	
	int (*invincible_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*spiritball_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*check_banding) ( struct block_list *bl, va_list ap );
	int (*inventory_rental_end) (int tid, int64 tick, int id, intptr_t data);
	void (*check_skilltree) (struct map_session_data *sd, int skill_id);
	int (*bonus_autospell) (struct s_autospell *spell, int max, short id, short lv, short rate, short flag, short card_id);
	int (*bonus_autospell_onskill) (struct s_autospell *spell, int max, short src_skill, short id, short lv, short rate, short card_id);
	int (*bonus_addeff) (struct s_addeffect* effect, int max, enum sc_type id, short rate, short arrow_rate, unsigned char flag);
	int (*bonus_addeff_onskill) (struct s_addeffectonskill* effect, int max, enum sc_type id, short rate, short skill_id, unsigned char target);
	int (*bonus_item_drop) (struct s_add_drop *drop, const short max, short id, short group, int race, int rate);
	void (*calcexp) (struct map_session_data *sd, unsigned int *base_exp, unsigned int *job_exp, struct block_list *src);
	int (*respawn_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*jobchange_killclone) (struct block_list *bl, va_list ap);
	int (*getstat) (struct map_session_data* sd, int type);
	int (*setstat) (struct map_session_data* sd, int type, int val);
	int (*eventtimer) (int tid, int64 tick, int id, intptr_t data);
	int (*daynight_timer_sub) (struct map_session_data *sd,va_list ap);
	int (*charm_timer) (int tid, int64 tick, int id, intptr_t data);
	bool (*readdb_levelpenalty) (char* fields[], int columns, int current);
	int (*autosave) (int tid, int64 tick, int id, intptr_t data);
	int (*follow_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*read_skill_tree) (void);
	int (*isUseitem) (struct map_session_data *sd,int n);
	int (*show_steal) (struct block_list *bl,va_list ap);
	int (*checkcombo) (struct map_session_data *sd, struct item_data *data );
	int (*calcweapontype) (struct map_session_data *sd);
	int (*removecombo) (struct map_session_data *sd, struct item_data *data );
	
	void (*bank_deposit) (struct map_session_data *sd, int money);
	void (*bank_withdraw) (struct map_session_data *sd, int money);
	
	void (*rental_expire) (struct map_session_data *sd, int i);
	void (*scdata_received) (struct map_session_data *sd);
	
	void (*bound_clear) (struct map_session_data *sd, enum e_item_bound_type type);
	
	int (*expiration_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*global_expiration_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*expire_check) (struct map_session_data *sd);
	
	/**
	 * Autotrade persistency [Ind/Hercules <3]
	 **/
	void (*autotrade_load) (void);
	void (*autotrade_update) (struct map_session_data *sd, enum e_pc_autotrade_update_action action);
	void (*autotrade_start) (struct map_session_data *sd);
	void (*autotrade_prepare) (struct map_session_data *sd);
	void (*autotrade_populate) (struct map_session_data *sd);
};

struct pc_interface *pc;

void pc_defaults(void);

#endif /* _MAP_PC_H_ */
