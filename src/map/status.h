/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
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
#ifndef MAP_STATUS_H
#define MAP_STATUS_H

#include "common/hercules.h"
#include "common/mmo.h" // NEW_CARTS

struct block_list;
struct config_setting_t;
struct elemental_data;
struct homun_data;
struct mercenary_data;
struct mob_data;
struct npc_data;
struct pet_data;

//Change the equation when the values are high enough to discard the
//imprecision in exchange of overflow protection [Skotlex]
//Also add 100% checks since those are the most used cases where we don't
//want approximation errors.
#define APPLY_RATE(value, rate) ( \
	(rate) == 100 ? \
		(value) \
	: ( \
		(value) > 100000 ? \
			(rate) * ( (value) / 100 ) \
		: \
			(value) * (rate) / 100 \
	) \
)

/**
 * Max Refine available to your server
 * Changing this limit requires edits to refine_db.txt
 **/
#ifdef RENEWAL
	#define MAX_REFINE 20
#else
	#define MAX_REFINE 10
#endif

enum refine_type {
	REFINE_TYPE_ARMOR   = 0,
	REFINE_TYPE_WEAPON1 = 1,
	REFINE_TYPE_WEAPON2 = 2,
	REFINE_TYPE_WEAPON3 = 3,
	REFINE_TYPE_WEAPON4 = 4,
#ifndef REFINE_TYPE_MAX
	REFINE_TYPE_MAX     = 5
#endif
};

/**
 * SC configuration type
 * @see db/sc_config.txt for more information
 **/
typedef enum sc_conf_type {
	SC_NO_REM_DEATH  = 0x001,
	SC_NO_SAVE       = 0x002,
	SC_NO_DISPELL    = 0x004,
	SC_NO_CLEARANCE  = 0x008,
	SC_BUFF          = 0x010,
	SC_DEBUFF        = 0x020,
	SC_MADO_NO_RESET = 0x040,
	SC_NO_CLEAR      = 0x080,
	SC_VISIBLE       = 0x100
} sc_conf_type;

/**
 * Flags to be used with status->change_start
 */
enum scstart_flag {
	// Note: When updating this enum, also update the documentation in doc/script_commands.txt and the constants in db/const.txt
	SCFLAG_NONE      = 0x00, ///< No special behavior.
	SCFLAG_NOAVOID   = 0x01, ///< Cannot be avoided (it has to start).
	SCFLAG_FIXEDTICK = 0x02, ///< Tick should not be reduced (by vit, luk, lv, etc).
	SCFLAG_LOADED    = 0x04, ///< sc_data was loaded, no value has to be altered.
	SCFLAG_FIXEDRATE = 0x08, ///< rate should not be reduced (not evaluated in status_change_start, but in some calls to other functions).
	SCFLAG_NOICON    = 0x10, ///< Status icon (SI) should not be sent.
	SCFLAG_ALL = SCFLAG_NONE|SCFLAG_NOAVOID|SCFLAG_FIXEDTICK|SCFLAG_LOADED|SCFLAG_FIXEDRATE|SCFLAG_NOICON
};

// Status changes listing. These code are for use by the server.
typedef enum sc_type {
	SC_NONE = -1,
	//First we enumerate common status ailments which are often used around.
	SC_STONE = 0,
	SC_COMMON_MIN = 0, // begin
	SC_FREEZE,
	SC_STUN,
	SC_SLEEP,
	SC_POISON,
	SC_CURSE,
	SC_SILENCE,
	SC_CONFUSION,
	SC_BLIND,
	SC_BLOODING,
	SC_DPOISON, //10
	SC_FEAR,
	SC_COLD,
	SC_BURNING,
	SC_DEEP_SLEEP,
	SC_COMMON_MAX = 14, // end

	//Next up, we continue on 20, to leave enough room for additional "common" ailments in the future.
	SC_PROVOKE = 20,
	SC_ENDURE,
	SC_TWOHANDQUICKEN,
	SC_CONCENTRATION,
	SC_HIDING,
	SC_CLOAKING,
	SC_ENCHANTPOISON,
	SC_POISONREACT,
	SC_QUAGMIRE,
	SC_ANGELUS,
	SC_BLESSING, //30
	SC_CRUCIS,
	SC_INC_AGI,
	SC_DEC_AGI,
	SC_SLOWPOISON,
	SC_IMPOSITIO  ,
	SC_SUFFRAGIUM,
	SC_ASPERSIO,
	SC_BENEDICTIO,
	SC_KYRIE,
	SC_MAGNIFICAT, //40
	SC_GLORIA,
	SC_LEXAETERNA,
	SC_ADRENALINE,
	SC_WEAPONPERFECT,
	SC_OVERTHRUST,
	SC_MAXIMIZEPOWER,
	SC_TRICKDEAD,
	SC_SHOUT,
	SC_ENERGYCOAT,
	SC_BROKENARMOR, //50 - NOTE: These two aren't used anywhere, and they have an icon...
	SC_BROKENWEAPON,
	SC_ILLUSION,
	SC_WEIGHTOVER50,
	SC_WEIGHTOVER90,
	SC_ATTHASTE_POTION1,
	SC_ATTHASTE_POTION2,
	SC_ATTHASTE_POTION3,
	SC_ATTHASTE_INFINITY,
	SC_MOVHASTE_HORSE,
	SC_MOVHASTE_INFINITY, //60
	SC_PLUSATTACKPOWER,
	SC_PLUSMAGICPOWER,
	SC_WEDDING,
	SC_SLOWDOWN,
	SC_ANKLESNARE,
	SC_KEEPING,
	SC_BARRIER,
	SC_NOEQUIPWEAPON,
	SC_NOEQUIPSHIELD,
	SC_NOEQUIPARMOR, //70
	SC_NOEQUIPHELM,
	SC_PROTECTWEAPON,
	SC_PROTECTSHIELD,
	SC_PROTECTARMOR,
	SC_PROTECTHELM,
	SC_AUTOGUARD,
	SC_REFLECTSHIELD,
	SC_SPLASHER,
	SC_PROVIDENCE,
	SC_DEFENDER, //80
	SC_MAGICROD,
	SC_SPELLBREAKER,
	SC_AUTOSPELL,
	SC_SIGHTTRASHER,
	SC_AUTOBERSERK,
	SC_SPEARQUICKEN,
	SC_AUTOCOUNTER,
	SC_SIGHT,
	SC_SAFETYWALL,
	SC_RUWACH, //90
	SC_EXTREMITYFIST,
	SC_EXPLOSIONSPIRITS,
	SC_COMBOATTACK,
	SC_BLADESTOP_WAIT,
	SC_BLADESTOP,
	SC_PROPERTYFIRE,
	SC_PROPERTYWATER,
	SC_PROPERTYWIND,
	SC_PROPERTYGROUND,
	SC_VOLCANO, //100,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_SUB_WEAPONPROPERTY,
	SC_ARMOR,
	SC_ARMORPROPERTY,
	SC_NOCHAT,
	SC_BABY,
	SC_AURABLADE,
	SC_PARRYING,
	SC_LKCONCENTRATION, //110
	SC_TENSIONRELAX,
	SC_BERSERK,
	SC_FURY,
	SC_GOSPEL,
	SC_ASSUMPTIO,
	SC_BASILICA,
	SC_GUILDAURA,
	SC_MAGICPOWER,
	SC_EDP,
	SC_TRUESIGHT, //120
	SC_WINDWALK,
	SC_MELTDOWN,
	SC_CARTBOOST,
	SC_CHASEWALK,
	SC_SWORDREJECT,
	SC_MARIONETTE_MASTER,
	SC_MARIONETTE,
	SC_PROPERTYUNDEAD,
	SC_JOINTBEAT,
	SC_MINDBREAKER, //130
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	SC_DEVOTION,
	SC_SACRIFICE,
	SC_STEELBODY,
	SC_ORCISH,
	SC_STORMKICK_READY,
	SC_DOWNKICK_READY,
	SC_TURNKICK_READY, //140
	SC_COUNTERKICK_READY,
	SC_DODGE_READY,
	SC_RUN,
	SC_PROPERTYDARK,
	SC_ADRENALINE2,
	SC_PROPERTYTELEKINESIS,
	SC_KAIZEL,
	SC_KAAHI,
	SC_KAUPE,
	SC_ONEHANDQUICKEN, //150
	SC_PRESERVE,
	SC_GDSKILL_BATTLEORDER,
	SC_GDSKILL_REGENERATION,
	SC_DOUBLECASTING,
	SC_GRAVITATION,
	SC_OVERTHRUSTMAX,
	SC_LONGING,
	SC_HERMODE,
	SC_TAROTCARD,
	SC_CR_SHRINK, //160
	SC_WZ_SIGHTBLASTER,
	SC_DC_WINKCHARM,
	SC_RG_CCONFINE_M,
	SC_RG_CCONFINE_S,
	SC_DANCING,
	SC_ARMOR_PROPERTY,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
	SC_NIBELUNGEN, //170
	SC_ROKISWEIL,
	SC_INTOABYSS,
	SC_SIEGFRIED,
	SC_WHISTLE,
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	SC_MODECHANGE,
	SC_HUMMING,
	SC_DONTFORGETME, //180
	SC_FORTUNE,
	SC_SERVICEFORYOU,
	SC_STOP, //Prevents inflicted chars from walking. [Skotlex]
	SC_STRUP,
	SC_SOULLINK,
	SC_COMA, //Not a real SC_, it makes a char's HP/SP hit 1.
	SC_CLAIRVOYANCE,
	SC_INCALLSTATUS,
	SC_CHASEWALK2,
	SC_INCAGI, //190
	SC_INCVIT,
	SC_INCINT,
	SC_INCDEX,
	SC_INCLUK,
	SC_INCHIT,
	SC_INCHITRATE,
	SC_INCFLEE,
	SC_INCFLEERATE,
	SC_INCMHPRATE,
	SC_INCMSPRATE, //200
	SC_INCATKRATE,
	SC_INCMATKRATE,
	SC_INCDEFRATE,
	SC_FOOD_STR,
	SC_FOOD_AGI,
	SC_FOOD_VIT,
	SC_FOOD_INT,
	SC_FOOD_DEX,
	SC_FOOD_LUK,
	SC_FOOD_BASICHIT, //210
	SC_FOOD_BASICAVOIDANCE,
	SC_BATKFOOD,
	SC_WATKFOOD,
	SC_MATKFOOD,
	SC_SCRESIST, //Increases resistance to status changes.
	SC_XMAS, // Xmas Suit [Valaris]
	SC_WARM, //SG skills [Komurka]
	SC_SUN_COMFORT,
	SC_MOON_COMFORT,
	SC_STAR_COMFORT, //220
	SC_FUSION,
	SC_SKILLRATE_UP,
	SC_SKE,
	SC_KAITE,
	SC_SWOO, // [marquis007]
	SC_SKA, // [marquis007]
	SC_EARTHSCROLL,
	SC_MIRACLE, //SG 'hidden' skill [Komurka]
	SC_GS_MADNESSCANCEL,
	SC_GS_ADJUSTMENT,  //230
	SC_GS_ACCURACY,
	SC_GS_GATLINGFEVER,
	SC_NJ_TATAMIGAESHI,
	SC_NJ_UTSUSEMI,
	SC_NJ_BUNSINJYUTSU,
	SC_NJ_KAENSIN,
	SC_NJ_SUITON,
	SC_NJ_NEN,
	SC_KNOWLEDGE,
	SC_SMA_READY, //240
	SC_FLING,
	SC_HLIF_AVOID,
	SC_HLIF_CHANGE,
	SC_HAMI_BLOODLUST,
	SC_HLIF_FLEET,
	SC_HLIF_SPEED,
	SC_HAMI_DEFENCE,
	SC_INCASPDRATE,
	SC_PLUSAVOIDVALUE,
	SC_JAILED, //250
	SC_ENCHANTARMS,
	SC_MAGICALATTACK,
	SC_STONESKIN,
	SC_CRITICALWOUND,
	SC_MAGICMIRROR,
	SC_SLOWCAST,
	SC_SUMMER,
	SC_CASH_PLUSEXP,
	SC_CASH_RECEIVEITEM,
	SC_CASH_BOSS_ALARM,  //260
	SC_CASH_DEATHPENALTY,
	SC_CRITICALPERCENT,
	//SC_INCDEF,
	//SC_INCBASEATK = 264,
	//SC_FASTCAST,
	SC_PROTECT_MDEF = 266,
	//SC_HPREGEN,
	SC_HEALPLUS = 268,
	SC_PNEUMA,
	SC_AUTOTRADE, //270
	SC_KSPROTECTED,
	SC_ARMOR_RESIST,
	SC_ATKER_BLOOD,
	SC_TARGET_BLOOD,
	SC_TK_SEVENWIND,
	SC_PROTECT_DEF,
	//SC_SPREGEN,
	SC_WALKSPEED = 278,
	// Mercenary Only Bonus Effects
	SC_MER_FLEE,
	SC_MER_ATK, //280
	SC_MER_HP,
	SC_MER_SP,
	SC_MER_HIT,
	SC_MER_QUICKEN,
	SC_REBIRTH,
	//SC_SKILLCASTRATE, //286
	//SC_DEFRATIOATK,
	//SC_HPDRAIN,
	//SC_SKILLATKBONUS,
	//SC_ITEMSCRIPT,
	SC_S_LIFEPOTION = 291,
	SC_L_LIFEPOTION,
	SC_CASH_PLUSONLYJOBEXP,
	//SC_IGNOREDEF,
	SC_HELLPOWER = 295,
	SC_INVINCIBLE,
	SC_INVINCIBLEOFF,
	SC_MANU_ATK,
	SC_MANU_DEF,
	SC_SPL_ATK, //300
	SC_SPL_DEF,
	SC_MANU_MATK,
	SC_SPL_MATK,
	SC_FOOD_STR_CASH,
	SC_FOOD_AGI_CASH,
	SC_FOOD_VIT_CASH,
	SC_FOOD_DEX_CASH,
	SC_FOOD_INT_CASH,
	SC_FOOD_LUK_CASH,
	/**
	 * 3rd
	 **/
	//SC_FEAR,
	SC_FROSTMISTY = 311,
	/**
	 * Rune Knight
	 **/
	SC_ENCHANTBLADE,
	SC_DEATHBOUND,
	SC_MILLENNIUMSHIELD,
	SC_CRUSHSTRIKE,
	SC_REFRESH,
	SC_REUSE_REFRESH,
	SC_GIANTGROWTH,
	SC_STONEHARDSKIN,
	SC_VITALITYACTIVATION, // 320
	SC_STORMBLAST,
	SC_FIGHTINGSPIRIT,
	SC_ABUNDANCE,
	/**
	 * Arch Bishop
	 **/
	SC_ADORAMUS,
	SC_EPICLESIS,
	SC_ORATIO,
	SC_LAUDAAGNUS,
	SC_LAUDARAMUS,
	SC_RENOVATIO,
	SC_EXPIATIO, // 330
	SC_DUPLELIGHT,
	SC_SECRAMENT,
	/**
	 * Warlock
	 **/
	SC_WHITEIMPRISON,
	SC_MARSHOFABYSS,
	SC_RECOGNIZEDSPELL,
	SC_STASIS,
	SC_SUMMON1,
	SC_SUMMON2,
	SC_SUMMON3,
	SC_SUMMON4, // 340
	SC_SUMMON5,
	SC_READING_SB,
	SC_FREEZINGSP,
	/**
	 * Ranger
	 **/
	SC_FEARBREEZE,
	SC_ELECTRICSHOCKER,
	SC_WUGDASH,
	SC_WUGBITE,
	SC_CAMOUFLAGE,
	/**
	 * Mechanic
	 **/
	SC_ACCELERATION,
	SC_HOVERING, // 350
	SC_SHAPESHIFT,
	SC_INFRAREDSCAN,
	SC_ANALYZE,
	SC_MAGNETICFIELD,
	SC_NEUTRALBARRIER,
	SC_NEUTRALBARRIER_MASTER,
	SC_STEALTHFIELD,
	SC_STEALTHFIELD_MASTER,
	SC_OVERHEAT,
	SC_OVERHEAT_LIMITPOINT, // 360
	/**
	 * Guillotine Cross
	 **/
	SC_VENOMIMPRESS,
	SC_POISONINGWEAPON,
	SC_WEAPONBLOCKING,
	SC_CLOAKINGEXCEED,
	SC_HALLUCINATIONWALK,
	SC_HALLUCINATIONWALK_POSTDELAY,
	SC_ROLLINGCUTTER,
	SC_TOXIN,
	SC_PARALYSE,
	SC_VENOMBLEED, // 370
	SC_MAGICMUSHROOM,
	SC_DEATHHURT,
	SC_PYREXIA,
	SC_OBLIVIONCURSE,
	SC_LEECHESEND,
	/**
	 * Royal Guard
	 **/
	SC_LG_REFLECTDAMAGE,
	SC_FORCEOFVANGUARD,
	SC_SHIELDSPELL_DEF,
	SC_SHIELDSPELL_MDEF,
	SC_SHIELDSPELL_REF, // 380
	SC_EXEEDBREAK,
	SC_PRESTIGE,
	SC_BANDING,
	SC_BANDING_DEFENCE,
	SC_EARTHDRIVE,
	SC_INSPIRATION,
	/**
	 * Sorcerer
	 **/
	SC_SPELLFIST,
	//SC_COLD,
	SC_STRIKING = 389,
	SC_WARMER, // 390
	SC_VACUUM_EXTREME,
	SC_PROPERTYWALK,
	/**
	 * Minstrel / Wanderer
	 **/
	SC_SWING,
	SC_SYMPHONY_LOVE,
	SC_MOONLIT_SERENADE,
	SC_RUSH_WINDMILL,
	SC_ECHOSONG,
	SC_HARMONIZE,
	SC_SIREN,
	//SC_DEEP_SLEEP, // 400
	SC_SIRCLEOFNATURE = 401,
	SC_GLOOMYDAY,
	//SC_GLOOMYDAY_SK,
	SC_SONG_OF_MANA = 404,
	SC_DANCE_WITH_WUG,
	SC_SATURDAY_NIGHT_FEVER,
	SC_LERADS_DEW,
	SC_MELODYOFSINK,
	SC_BEYOND_OF_WARCRY,
	SC_UNLIMITED_HUMMING_VOICE, // 410
	SC_SITDOWN_FORCE,
	SC_NETHERWORLD,
	/**
	 * Sura
	 **/
	SC_CRESCENTELBOW = 413,
	SC_CURSEDCIRCLE_ATKER,
	SC_CURSEDCIRCLE_TARGET,
	SC_LIGHTNINGWALK,
	SC_RAISINGDRAGON,
	SC_GENTLETOUCH_ENERGYGAIN,
	SC_GENTLETOUCH_CHANGE,
	SC_GENTLETOUCH_REVITALIZE, // 420
	/**
	 * Genetic
	 **/
	SC_GN_CARTBOOST,
	SC_THORNS_TRAP,
	SC_BLOOD_SUCKER,
	SC_FIRE_EXPANSION_SMOKE_POWDER,
	SC_FIRE_EXPANSION_TEAR_GAS,
	SC_MANDRAGORA,
	SC_STOMACHACHE,
	SC_MYSTERIOUS_POWDER,
	SC_MELON_BOMB,
	SC_BANANA_BOMB, // 430
	SC_BANANA_BOMB_SITDOWN_POSTDELAY,
	SC_SAVAGE_STEAK,
	SC_COCKTAIL_WARG_BLOOD,
	SC_MINOR_BBQ,
	SC_SIROMA_ICE_TEA,
	SC_DROCERA_HERB_STEAMED,
	SC_PUTTI_TAILS_NOODLES,
	SC_BOOST500,
	SC_FULL_SWING_K,
	SC_MANA_PLUS, // 440
	SC_MUSTLE_M,
	SC_LIFE_FORCE_F,
	SC_EXTRACT_WHITE_POTION_Z,
	SC_VITATA_500,
	SC_EXTRACT_SALAMINE_JUICE,
	/**
	 * Shadow Chaser
	 **/
	SC__REPRODUCE,
	SC__AUTOSHADOWSPELL,
	SC__SHADOWFORM,
	SC__BODYPAINT,
	SC__INVISIBILITY, // 450
	SC__DEADLYINFECT,
	SC__ENERVATION,
	SC__GROOMY,
	SC__IGNORANCE,
	SC__LAZINESS,
	SC__UNLUCKY,
	SC__WEAKNESS,
	SC__STRIPACCESSARY,
	SC__MANHOLE,
	SC__BLOODYLUST, // 460
	/**
	 * Elemental Spirits
	 **/
	SC_CIRCLE_OF_FIRE,
	SC_CIRCLE_OF_FIRE_OPTION,
	SC_FIRE_CLOAK,
	SC_FIRE_CLOAK_OPTION,
	SC_WATER_SCREEN,
	SC_WATER_SCREEN_OPTION,
	SC_WATER_DROP,
	SC_WATER_DROP_OPTION,
	SC_WATER_BARRIER,
	SC_WIND_STEP, // 470
	SC_WIND_STEP_OPTION,
	SC_WIND_CURTAIN,
	SC_WIND_CURTAIN_OPTION,
	SC_ZEPHYR,
	SC_SOLID_SKIN,
	SC_SOLID_SKIN_OPTION,
	SC_STONE_SHIELD,
	SC_STONE_SHIELD_OPTION,
	SC_POWER_OF_GAIA,
	SC_PYROTECHNIC, // 480
	SC_PYROTECHNIC_OPTION,
	SC_HEATER,
	SC_HEATER_OPTION,
	SC_TROPIC,
	SC_TROPIC_OPTION,
	SC_AQUAPLAY,
	SC_AQUAPLAY_OPTION,
	SC_COOLER,
	SC_COOLER_OPTION,
	SC_CHILLY_AIR, // 490
	SC_CHILLY_AIR_OPTION,
	SC_GUST,
	SC_GUST_OPTION,
	SC_BLAST,
	SC_BLAST_OPTION,
	SC_WILD_STORM,
	SC_WILD_STORM_OPTION,
	SC_PETROLOGY,
	SC_PETROLOGY_OPTION,
	SC_CURSED_SOIL, // 500
	SC_CURSED_SOIL_OPTION,
	SC_UPHEAVAL,
	SC_UPHEAVAL_OPTION,
	SC_TIDAL_WEAPON,
	SC_TIDAL_WEAPON_OPTION,
	SC_ROCK_CRUSHER,
	SC_ROCK_CRUSHER_ATK,
	/* Guild Aura */
	SC_LEADERSHIP,
	SC_GLORYWOUNDS,
	SC_SOULCOLD, // 510
	SC_HAWKEYES,
	/* ... */
	SC_ODINS_POWER,
	/* Sorcerer .extra */
	SC_FIRE_INSIGNIA,
	SC_WATER_INSIGNIA,
	SC_WIND_INSIGNIA,
	SC_EARTH_INSIGNIA,
	/* new pushcart */
	SC_PUSH_CART,
	/* Warlock Spell books */
	SC_SPELLBOOK1,
	SC_SPELLBOOK2,
	SC_SPELLBOOK3, // 520
	SC_SPELLBOOK4,
	SC_SPELLBOOK5,
	SC_SPELLBOOK6,
/**
 * In official server there are only 7 maximum number of spell books that can be memorized
 * To increase the maximum value just add another status type before SC_SPELLBOOK7 (ex. SC_SPELLBOOK8, SC_SPELLBOOK9 and so on)
 **/
	SC_SPELLBOOK7,
	/* Max HP & SP */
	SC_INCMHP,
	SC_INCMSP,
	SC_PARTYFLEE,
	/**
	* Kagerou & Oboro [malufett]
	**/
	SC_MEIKYOUSISUI,
	SC_KO_JYUMONJIKIRI,
	SC_KYOUGAKU, // 530
	SC_IZAYOI,
	SC_ZENKAI,
	SC_KG_KAGEHUMI,
	SC_KYOMU,
	SC_KAGEMUSYA,
	SC_ZANGETSU,
	SC_GENSOU,
	SC_AKAITSUKI,

	//homon S
	SC_STYLE_CHANGE,
	SC_GOLDENE_FERSE, // 540
	SC_ANGRIFFS_MODUS,
	SC_ERASER_CUTTER,
	SC_OVERED_BOOST,
	SC_LIGHT_OF_REGENE,
	SC_VOLCANIC_ASH,
	SC_GRANITIC_ARMOR,
	SC_MAGMA_FLOW,
	SC_PYROCLASTIC,
	SC_NEEDLE_OF_PARALYZE,
	SC_PAIN_KILLER, // 550
#ifdef RENEWAL
	SC_EXTREMITYFIST2,
	SC_RAID,
#endif
	SC_DARKCROW = 553,
	SC_FULL_THROTTLE,
	SC_REBOUND,
	SC_UNLIMIT,
	SC_KINGS_GRACE,
	SC_TELEKINESIS_INTENSE,
	SC_OFFERTORIUM,
	SC_FRIGG_SONG, // 560

	SC_ALL_RIDING,
	SC_HANBOK,
	SC_MONSTER_TRANSFORM,
	SC_ANGEL_PROTECT,
	SC_ILLUSIONDOPING,

	SC_MTF_ASPD,
	SC_MTF_RANGEATK,
	SC_MTF_MATK,
	SC_MTF_MLEATKED,
	SC_MTF_CRIDAMAGE, // 570

	SC_MOONSTAR,
	SC_SUPER_STAR,

	SC_OKTOBERFEST,
	SC_STRANGELIGHTS,
	SC_DECORATION_OF_MUSIC,

	SC__MAELSTROM,
	SC__CHAOS,

	SC__FEINTBOMB_MASTER,
	SC_FALLENEMPIRE,
	SC_FLASHCOMBO, // 580

	//Vellum Weapon reductions
	SC_DEFSET,
	SC_MDEFSET,

	SC_NO_SWITCH_EQUIP,

	// 2014 Halloween Event
	SC_MTF_MHP,
	SC_MTF_MSP,
	SC_MTF_PUMPKIN,
	SC_MTF_HITFLEE,

	SC_LJOSALFAR,
	SC_MERMAID_LONGING,

	SC_ACARAJE, // 590
	SC_TARGET_ASPD,

	// Geffen Scrolls
	SC_SKELSCROLL,
	SC_DISTRUCTIONSCROLL,
	SC_ROYALSCROLL,
	SC_IMMUNITYSCROLL,
	SC_MYSTICSCROLL,
	SC_BATTLESCROLL,
	SC_ARMORSCROLL,
	SC_FREYJASCROLL,
	SC_SOULSCROLL, // 600

	// Eden Crystal Synthesis
	SC_QUEST_BUFF1,
	SC_QUEST_BUFF2,
	SC_QUEST_BUFF3,

	// Geffen Magic Tournament
	SC_GEFFEN_MAGIC1,
	SC_GEFFEN_MAGIC2,
	SC_GEFFEN_MAGIC3,
	SC_FENRIR_CARD,

	SC_ATKER_ASPD,
	SC_ATKER_MOVESPEED,
	SC_FOOD_CRITICALSUCCESSVALUE, // 610
	SC_CUP_OF_BOZA,
	SC_OVERLAPEXPUP,
	SC_MORA_BUFF,

	// MVP Scrolls
	SC_MVPCARD_TAOGUNKA,
	SC_MVPCARD_MISTRESS,
	SC_MVPCARD_ORCHERO,
	SC_MVPCARD_ORCLORD,

	SC_HAT_EFFECT,
	SC_FLOWERSMOKE,
	SC_FSTONE, // 620
	SC_HAPPINESS_STAR,
	SC_MAPLE_FALLS,
	SC_TIME_ACCESSORY,
	SC_MAGICAL_FEATHER,
	SC_BLOSSOM_FLUTTERING,

	SC_GM_BATTLE,
	SC_GM_BATTLE2,
	SC_2011RWC,
	SC_STR_SCROLL,
	SC_INT_SCROLL, // 630
	SC_STEAMPACK,
	SC_MOVHASTE_POTION,
	SC_MOVESLOW_POTION,
	SC_BUCHEDENOEL,
	SC_PHI_DEMON,
	SC_PROMOTE_HEALTH_RESERCH,
	SC_ENERGY_DRINK_RESERCH,
	SC_MAGIC_CANDY,
	SC_M_LIFEPOTION,
	SC_G_LIFEPOTION, // 640
	SC_MYSTICPOWDER,
#ifndef SC_MAX
	SC_MAX, //Automatically updated max, used in for's to check we are within bounds.
#endif
} sc_type;

/// Official status change ids, used to display status icons in the client.
enum si_type {
	SI_BLANK                                 = -1,

	SI_PROVOKE                               =  0,
	SI_ENDURE                                =  1,
	SI_TWOHANDQUICKEN                        =  2,
	SI_CONCENTRATION                         =  3,
	SI_HIDING                                =  4,
	SI_CLOAKING                              =  5,
	SI_ENCHANTPOISON                         =  6,
	SI_POISONREACT                           =  7,
	SI_QUAGMIRE                              =  8,
	SI_ANGELUS                               =  9,
	SI_BLESSING                              = 10,
	SI_CRUCIS                                = 11,
	SI_INC_AGI                               = 12,
	SI_DEC_AGI                               = 13,
	SI_SLOWPOISON                            = 14,
	SI_IMPOSITIO                             = 15,
	SI_SUFFRAGIUM                            = 16,
	SI_ASPERSIO                              = 17,
	SI_BENEDICTIO                            = 18,
	SI_KYRIE                                 = 19,
	SI_MAGNIFICAT                            = 20,
	SI_GLORIA                                = 21,
	SI_LEXAETERNA                            = 22,
	SI_ADRENALINE                            = 23,
	SI_WEAPONPERFECT                         = 24,
	SI_OVERTHRUST                            = 25,
	SI_MAXIMIZE                              = 26,
	SI_RIDING                                = 27,
	SI_FALCON                                = 28,
	SI_TRICKDEAD                             = 29,
	SI_SHOUT                                 = 30,
	SI_ENERGYCOAT                            = 31,
	SI_BROKENARMOR                           = 32,
	SI_BROKENWEAPON                          = 33,
	SI_ILLUSION                              = 34,
	SI_WEIGHTOVER50                          = 35,
	SI_WEIGHTOVER90                          = 36,
	SI_ATTHASTE_POTION1                      = 37,
	SI_ATTHASTE_POTION2                      = 38,
	SI_ATTHASTE_POTION3                      = 39,
	SI_ATTHASTE_INFINITY                     = 40,
	SI_MOVHASTE_POTION                       = 41,
	SI_MOVHASTE_INFINITY                     = 42,
	//SI_AUTOCOUNTER                         = 43,
	//SI_SPLASHER                            = 44,
	SI_ANKLESNARE                            = 45,
	SI_POSTDELAY                             = 46,
	//SI_NOACTION                            = 47,
	//SI_IMPOSSIBLEPICKUP                    = 48,
	//SI_BARRIER                             = 49,

	SI_NOEQUIPWEAPON                         = 50,
	SI_NOEQUIPSHIELD                         = 51,
	SI_NOEQUIPARMOR                          = 52,
	SI_NOEQUIPHELM                           = 53,
	SI_PROTECTWEAPON                         = 54,
	SI_PROTECTSHIELD                         = 55,
	SI_PROTECTARMOR                          = 56,
	SI_PROTECTHELM                           = 57,
	SI_AUTOGUARD                             = 58,
	SI_REFLECTSHIELD                         = 59,
	//SI_DEVOTION                            = 60,
	SI_PROVIDENCE                            = 61,
	SI_DEFENDER                              = 62,
	//SI_MAGICROD                            = 63,
	//SI_WEAPONPROPERTY                      = 64,
	SI_AUTOSPELL                             = 65,
	//SI_SPECIALZONE                         = 66,
	//SI_MASK                                = 67,
	SI_SPEARQUICKEN                          = 68,
	//SI_BDPLAYING                           = 69,
	//SI_WHISTLE                             = 70,
	//SI_ASSASSINCROSS                       = 71,
	//SI_POEMBRAGI                           = 72,
	//SI_APPLEIDUN                           = 73,
	//SI_HUMMING                             = 74,
	//SI_DONTFORGETME                        = 75,
	//SI_FORTUNEKISS                         = 76,
	//SI_SERVICEFORYOU                       = 77,
	//SI_RICHMANKIM                          = 78,
	//SI_ETERNALCHAOS                        = 79,
	//SI_DRUMBATTLEFIELD                     = 80,
	//SI_RINGNIBELUNGEN                      = 81,
	//SI_ROKISWEIL                           = 82,
	//SI_INTOABYSS                           = 83,
	//SI_SIEGFRIED                           = 84,
	//SI_BLADESTOP                           = 85,
	SI_EXPLOSIONSPIRITS                      = 86,
	SI_STEELBODY                             = 87,
	SI_EXTREMITYFIST                         = 88,
	//SI_COMBOATTACK                         = 89,
	SI_PROPERTYFIRE                          = 90,
	SI_PROPERTYWATER                         = 91,
	SI_PROPERTYWIND                          = 92,
	SI_PROPERTYGROUND                        = 93,
	//SI_MAGICATTACK                         = 94,
	SI_STOP                                  = 95,
	//SI_WEAPONBRAKER                        = 96,
	SI_PROPERTYUNDEAD                        = 97,
	//SI_POWERUP                             = 98,
	//SI_AGIUP                               = 99,

	//SI_SIEGEMODE                           = 100,
	//SI_INVISIBLE                           = 101,
	//SI_STATUSONE                           = 102,
	SI_AURABLADE                             = 103,
	SI_PARRYING                              = 104,
	SI_LKCONCENTRATION                       = 105,
	SI_TENSIONRELAX                          = 106,
	SI_BERSERK                               = 107,
	//SI_SACRIFICE                           = 108,
	//SI_GOSPEL                              = 109,
	SI_ASSUMPTIO                             = 110,
	//SI_BASILICA                            = 111,
	SI_GROUNDMAGIC                           = 112,
	SI_MAGICPOWER                            = 113,
	SI_EDP                                   = 114,
	SI_TRUESIGHT                             = 115,
	SI_WINDWALK                              = 116,
	SI_MELTDOWN                              = 117,
	SI_CARTBOOST                             = 118,
	//SI_CHASEWALK                           = 119,
	SI_SWORDREJECT                           = 120,
	SI_MARIONETTE_MASTER                     = 121,
	SI_MARIONETTE                            = 122,
	SI_MOON                                  = 123,
	SI_BLOODING                              = 124,
	SI_JOINTBEAT                             = 125,
	//SI_MINDBREAKER                         = 126,
	//SI_MEMORIZE                            = 127,
	//SI_FOGWALL                             = 128,
	//SI_SPIDERWEB                           = 129,
	SI_PROTECTEXP                            = 130,
	//SI_SUB_WEAPONPROPERTY                  = 131,
	SI_AUTOBERSERK                           = 132,
	SI_RUN                                   = 133,
	SI_TING                                  = 134,
	SI_STORMKICK_ON                          = 135,
	SI_STORMKICK_READY                       = 136,
	SI_DOWNKICK_ON                           = 137,
	SI_DOWNKICK_READY                        = 138,
	SI_TURNKICK_ON                           = 139,
	SI_TURNKICK_READY                        = 140,
	SI_COUNTER_ON                            = 141,
	SI_COUNTER_READY                         = 142,
	SI_DODGE_ON                              = 143,
	SI_DODGE_READY                           = 144,
	SI_STRUP                                 = 145,
	SI_PROPERTYDARK                          = 146,
	SI_ADRENALINE2                           = 147,
	SI_PROPERTYTELEKINESIS                   = 148,
	SI_SOULLINK                              = 149,

	SI_PLUSATTACKPOWER                       = 150,
	SI_PLUSMAGICPOWER                        = 151,
	SI_DEVIL1                                = 152,
	SI_KAITE                                 = 153,
	//SI_SWOO                                = 154,
	//SI_STAR2                               = 155,
	SI_KAIZEL                                = 156,
	SI_KAAHI                                 = 157,
	SI_KAUPE                                 = 158,
	SI_SMA_READY                             = 159,
	SI_SKE                                   = 160,
	SI_ONEHANDQUICKEN                        = 161,
	//SI_FRIEND                              = 162,
	//SI_FRIENDUP                            = 163,
	//SI_SG_WARM                             = 164,
	SI_SG_SUN_WARM                           = 165,
	//SI_SG_MOON_WARM                        = 166, // The three show the exact same display: ultra red character (165, 166, 167)
	//SI_SG_STAR_WARM                        = 167,
	//SI_EMOTION                             = 168,
	SI_SUN_COMFORT                           = 169,
	SI_MOON_COMFORT                          = 170,
	SI_STAR_COMFORT                          = 171,
	//SI_EXPUP                               = 172,
	//SI_GDSKILL_BATTLEORDER                 = 173,
	//SI_GDSKILL_REGENERATION                = 174,
	//SI_GDSKILL_POSTDELAY                   = 175,
	//SI_RESISTHANDICAP                      = 176,
	//SI_MAXHPPERCENT                        = 177,
	//SI_MAXSPPERCENT                        = 178,
	//SI_DEFENCE                             = 179,
	//SI_SLOWDOWN                            = 180,
	SI_PRESERVE                              = 181,
	SI_INCSTR                                = 182,
	//SI_NOT_EXTREMITYFIST                   = 183,
	SI_CLAIRVOYANCE                          = 184,
	SI_MOVESLOW_POTION                       = 185,
	SI_DOUBLECASTING                         = 186,
	//SI_GRAVITATION                         = 187,
	SI_OVERTHRUSTMAX                         = 188,
	//SI_LONGING                             = 189,
	//SI_HERMODE                             = 190,
	SI_TAROTCARD                             = 191, // the icon allows no doubt... but what is it really used for ?? [DracoRPG]
	//SI_HLIF_AVOID                          = 192,
	//SI_HFLI_FLEET                          = 193,
	//SI_HFLI_SPEED                          = 194,
	//SI_HLIF_CHANGE                         = 195,
	//SI_HAMI_BLOODLUST                      = 196,
	SI_CR_SHRINK                             = 197,
	SI_WZ_SIGHTBLASTER                       = 198,
	SI_DC_WINKCHARM                          = 199,

	SI_RG_CCONFINE_M                         = 200,
	SI_RG_CCONFINE_S                         = 201,
	//SI_DISABLEMOVE                         = 202,
	SI_GS_MADNESSCANCEL                      = 203, //[blackhole89]
	SI_GS_GATLINGFEVER                       = 204,
	SI_EARTHSCROLL                           = 205,
	SI_NJ_UTSUSEMI                           = 206,
	SI_NJ_BUNSINJYUTSU                       = 207,
	SI_NJ_NEN                                = 208,
	SI_GS_ADJUSTMENT                         = 209,
	SI_GS_ACCURACY                           = 210,
	SI_NJ_SUITON                             = 211,
	//SI_PET                                 = 212,
	//SI_MENTAL                              = 213,
	//SI_EXPMEMORY                           = 214,
	//SI_PERFORMANCE                         = 215,
	//SI_GAIN                                = 216,
	//SI_GRIFFON                             = 217,
	//SI_DRIFT                               = 218,
	//SI_WALLSHIFT                           = 219,
	//SI_REINCARNATION                       = 220,
	//SI_PATTACK                             = 221,
	//SI_PSPEED                              = 222,
	//SI_PDEFENSE                            = 223,
	//SI_PCRITICAL                           = 224,
	//SI_RANKING                             = 225,
	//SI_PTRIPLE                             = 226,
	//SI_DENERGY                             = 227,
	//SI_WAVE1                               = 228,
	//SI_WAVE2                               = 229,
	//SI_WAVE3                               = 230,
	//SI_WAVE4                               = 231,
	//SI_DAURA                               = 232,
	//SI_DFREEZER                            = 233,
	//SI_DPUNISH                             = 234,
	//SI_DBARRIER                            = 235,
	//SI_DWARNING                            = 236,
	//SI_MOUSEWHEEL                          = 237,
	//SI_DGAUGE                              = 238,
	//SI_DACCEL                              = 239,
	//SI_DBLOCK                              = 240,
	SI_FOOD_STR                              = 241,
	SI_FOOD_AGI                              = 242,
	SI_FOOD_VIT                              = 243,
	SI_FOOD_DEX                              = 244,
	SI_FOOD_INT                              = 245,
	SI_FOOD_LUK                              = 246,
	SI_FOOD_BASICAVOIDANCE                   = 247,
	SI_FOOD_BASICHIT                         = 248,
	SI_FOOD_CRITICALSUCCESSVALUE             = 249,

	SI_CASH_PLUSEXP                          = 250,
	SI_CASH_DEATHPENALTY                     = 251,
	SI_CASH_RECEIVEITEM                      = 252,
	SI_CASH_BOSS_ALARM                       = 253,
	//SI_DA_ENERGY                           = 254,
	//SI_DA_FIRSTSLOT                        = 255,
	//SI_DA_HEADDEF                          = 256,
	//SI_DA_SPACE                            = 257,
	//SI_DA_TRANSFORM                        = 258,
	//SI_DA_ITEMREBUILD                      = 259,
	//SI_DA_ILLUSION                         = 260, // All mobs display as Turtle General
	//SI_DA_DARKPOWER                        = 261,
	//SI_DA_EARPLUG                          = 262,
	//SI_DA_CONTRACT                         = 263, // Bio Mob effect on you and SI_TRICKDEAD icon
	//SI_DA_BLACK                            = 264, // For short time blurry screen
	//SI_DA_MAGICCART                        = 265,
	//SI_CRYSTAL                             = 266,
	//SI_DA_REBUILD                          = 267,
	//SI_DA_EDARKNESS                        = 268,
	//SI_DA_EGUARDIAN                        = 269,
	//SI_DA_TIMEOUT                          = 270,
	SI_FOOD_STR_CASH                         = 271,
	SI_FOOD_AGI_CASH                         = 272,
	SI_FOOD_VIT_CASH                         = 273,
	SI_FOOD_DEX_CASH                         = 274,
	SI_FOOD_INT_CASH                         = 275,
	SI_FOOD_LUK_CASH                         = 276,
	SI_MER_FLEE                              = 277,
	SI_MER_ATK                               = 278,
	SI_MER_HP                                = 279,
	SI_MER_SP                                = 280,
	SI_MER_HIT                               = 281,
	SI_SLOWCAST                              = 282,
	//SI_MAGICMIRROR                         = 283,
	//SI_STONESKIN                           = 284,
	//SI_ANTIMAGIC                           = 285,
	SI_CRITICALWOUND                         = 286,
	//SI_NPC_DEFENDER                        = 287,
	//SI_NOACTION_WAIT                       = 288,
	SI_MOVHASTE_HORSE                        = 289,
	SI_PROTECT_DEF                           = 290,
	SI_PROTECT_MDEF                          = 291,
	SI_HEALPLUS                              = 292,
	SI_S_LIFEPOTION                          = 293,
	SI_L_LIFEPOTION                          = 294,
	SI_CRITICALPERCENT                       = 295,
	SI_PLUSAVOIDVALUE                        = 296,
	SI_ATKER_ASPD                            = 297,
	SI_TARGET_ASPD                           = 298,
	SI_ATKER_MOVESPEED                       = 299,

	SI_ATKER_BLOOD                           = 300,
	SI_TARGET_BLOOD                          = 301,
	SI_ARMOR_PROPERTY                        = 302,
	//SI_REUSE_LIMIT_A                       = 303,
	SI_HELLPOWER                             = 304,
	SI_STEAMPACK                             = 305,
	//SI_REUSE_LIMIT_B                       = 306,
	//SI_REUSE_LIMIT_C                       = 307,
	//SI_REUSE_LIMIT_D                       = 308,
	//SI_REUSE_LIMIT_E                       = 309,
	//SI_REUSE_LIMIT_F                       = 310,
	SI_INVINCIBLE                            = 311,
	SI_CASH_PLUSONLYJOBEXP                   = 312,
	SI_PARTYFLEE                             = 313,
	SI_ANGEL_PROTECT                         = 314,
	//SI_ENDURE_MDEF                         = 315,
	SI_ENCHANTBLADE                          = 316,
	SI_DEATHBOUND                            = 317,
	SI_REFRESH                               = 318,
	SI_GIANTGROWTH                           = 319,
	SI_STONEHARDSKIN                         = 320,
	SI_VITALITYACTIVATION                    = 321,
	SI_FIGHTINGSPIRIT                        = 322,
	SI_ABUNDANCE                             = 323,
	SI_REUSE_MILLENNIUMSHIELD                = 324,
	SI_REUSE_CRUSHSTRIKE                     = 325,
	SI_REUSE_REFRESH                         = 326,
	SI_REUSE_STORMBLAST                      = 327,
	SI_VENOMIMPRESS                          = 328,
	SI_EPICLESIS                             = 329,
	SI_ORATIO                                = 330,
	SI_LAUDAAGNUS                            = 331,
	SI_LAUDARAMUS                            = 332,
	SI_CLOAKINGEXCEED                        = 333,
	SI_HALLUCINATIONWALK                     = 334,
	SI_HALLUCINATIONWALK_POSTDELAY           = 335,
	SI_RENOVATIO                             = 336,
	SI_WEAPONBLOCKING                        = 337,
	SI_WEAPONBLOCKING_POSTDELAY              = 338,
	SI_ROLLINGCUTTER                         = 339,
	SI_EXPIATIO                              = 340,
	SI_POISONINGWEAPON                       = 341,
	SI_TOXIN                                 = 342,
	SI_PARALYSE                              = 343,
	SI_VENOMBLEED                            = 344,
	SI_MAGICMUSHROOM                         = 345,
	SI_DEATHHURT                             = 346,
	SI_PYREXIA                               = 347,
	SI_OBLIVIONCURSE                         = 348,
	SI_LEECHESEND                            = 349,

	SI_DUPLELIGHT                            = 350,
	SI_FROSTMISTY                            = 351,
	SI_FEARBREEZE                            = 352,
	SI_ELECTRICSHOCKER                       = 353,
	SI_MARSHOFABYSS                          = 354,
	SI_RECOGNIZEDSPELL                       = 355,
	SI_STASIS                                = 356,
	SI_WUGRIDER                              = 357,
	SI_WUGDASH                               = 358,
	SI_WUGBITE                               = 359,
	SI_CAMOUFLAGE                            = 360,
	SI_ACCELERATION                          = 361,
	SI_HOVERING                              = 362,
	SI_SPHERE_1                              = 363,
	SI_SPHERE_2                              = 364,
	SI_SPHERE_3                              = 365,
	SI_SPHERE_4                              = 366,
	SI_SPHERE_5                              = 367,
	SI_MVPCARD_TAOGUNKA                      = 368,
	SI_MVPCARD_MISTRESS                      = 369,
	SI_MVPCARD_ORCHERO                       = 370,
	SI_MVPCARD_ORCLORD                       = 371,
	SI_OVERHEAT_LIMITPOINT                   = 372,
	SI_OVERHEAT                              = 373,
	SI_SHAPESHIFT                            = 374,
	SI_INFRAREDSCAN                          = 375,
	SI_MAGNETICFIELD                         = 376,
	SI_NEUTRALBARRIER                        = 377,
	SI_NEUTRALBARRIER_MASTER                 = 378,
	SI_STEALTHFIELD                          = 379,
	SI_STEALTHFIELD_MASTER                   = 380,
	SI_MANU_ATK                              = 381,
	SI_MANU_DEF                              = 382,
	SI_SPL_ATK                               = 383,
	SI_SPL_DEF                               = 384,
	SI_REPRODUCE                             = 385,
	SI_MANU_MATK                             = 386,
	SI_SPL_MATK                              = 387,
	SI_STR_SCROLL                            = 388,
	SI_INT_SCROLL                            = 389,
	SI_LG_REFLECTDAMAGE                      = 390,
	SI_FORCEOFVANGUARD                       = 391,
	SI_BUCHEDENOEL                           = 392,
	SI_AUTOSHADOWSPELL                       = 393,
	SI_SHADOWFORM                            = 394,
	SI_RAID                                  = 395,
	SI_SHIELDSPELL_DEF                       = 396,
	SI_SHIELDSPELL_MDEF                      = 397,
	SI_SHIELDSPELL_REF                       = 398,
	SI_BODYPAINT                             = 399,

	SI_EXEEDBREAK                            = 400,
	SI_ADORAMUS                              = 401,
	SI_PRESTIGE                              = 402,
	SI_INVISIBILITY                          = 403,
	SI_DEADLYINFECT                          = 404,
	SI_BANDING                               = 405,
	SI_EARTHDRIVE                            = 406,
	SI_INSPIRATION                           = 407,
	SI_ENERVATION                            = 408,
	SI_GROOMY                                = 409,
	SI_RAISINGDRAGON                         = 410,
	SI_IGNORANCE                             = 411,
	SI_LAZINESS                              = 412,
	SI_LIGHTNINGWALK                         = 413,
	SI_ACARAJE                               = 414,
	SI_UNLUCKY                               = 415,
	SI_CURSEDCIRCLE_ATKER                    = 416,
	SI_CURSEDCIRCLE_TARGET                   = 417,
	SI_WEAKNESS                              = 418,
	SI_CRESCENTELBOW                         = 419,
	SI_NOEQUIPACCESSARY                      = 420,
	SI_STRIPACCESSARY                        = 421,
	SI_MANHOLE                               = 422,
	SI_POPECOOKIE                            = 423,
	SI_FALLENEMPIRE                          = 424,
	SI_GENTLETOUCH_ENERGYGAIN                = 425,
	SI_GENTLETOUCH_CHANGE                    = 426,
	SI_GENTLETOUCH_REVITALIZE                = 427,
	SI_BLOODYLUST                            = 428,
	SI_SWINGDANCE                            = 429,
	SI_SYMPHONYOFLOVERS                      = 430,
	SI_PROPERTYWALK                          = 431,
	SI_SPELLFIST                             = 432,
	SI_NETHERWORLD                           = 433,
	SI_SIREN                                 = 434,
	SI_DEEPSLEEP                             = 435,
	SI_SIRCLEOFNATURE                        = 436,
	SI_COLD                                  = 437,
	SI_GLOOMYDAY                             = 438,
	SI_SONG_OF_MANA                          = 439,
	SI_CLOUDKILL                             = 440,
	SI_DANCEWITHWUG                          = 441,
	SI_RUSHWINDMILL                          = 442,
	SI_ECHOSONG                              = 443,
	SI_HARMONIZE                             = 444,
	SI_STRIKING                              = 445,
	//SI_WARMER                              = 446,
	SI_MOONLITSERENADE                       = 447,
	SI_SATURDAYNIGHTFEVER                    = 448,
	SI_SITDOWN_FORCE                         = 449,

	SI_ANALYZE                               = 450,
	SI_LERADSDEW                             = 451,
	SI_MELODYOFSINK                          = 452,
	SI_WARCRYOFBEYOND                        = 453,
	SI_UNLIMITEDHUMMINGVOICE                 = 454,
	SI_SPELLBOOK1                            = 455,
	SI_SPELLBOOK2                            = 456,
	SI_SPELLBOOK3                            = 457,
	SI_FREEZE_SP                             = 458,
	SI_GN_TRAINING_SWORD                     = 459,
	SI_GN_REMODELING_CART                    = 460,
	SI_CARTSBOOST                            = 461,
	SI_FIXEDCASTINGTM_REDUCE                 = 462,
	SI_THORNTRAP                             = 463,
	SI_BLOODSUCKER                           = 464,
	SI_SPORE_EXPLOSION                       = 465,
	SI_DEMONIC_FIRE                          = 466,
	SI_FIRE_EXPANSION_SMOKE_POWDER           = 467,
	SI_FIRE_EXPANSION_TEAR_GAS               = 468,
	SI_BLOCKING_PLAY                         = 469,
	SI_MANDRAGORA                            = 470,
	SI_ACTIVATE                              = 471,
	SI_SECRAMENT                             = 472,
	SI_ASSUMPTIO2                            = 473,
	SI_TK_SEVENWIND                          = 474,
	SI_LIMIT_ODINS_RECALL                    = 475,
	SI_STOMACHACHE                           = 476,
	SI_MYSTERIOUS_POWDER                     = 477,
	SI_MELON_BOMB                            = 478,
	SI_BANANA_BOMB_SITDOWN_POSTDELAY         = 479,
	SI_PROMOTE_HEALTH_RESERCH                = 480,
	SI_ENERGY_DRINK_RESERCH                  = 481,
	SI_EXTRACT_WHITE_POTION_Z                = 482,
	SI_VITATA_500                            = 483,
	SI_EXTRACT_SALAMINE_JUICE                = 484,
	SI_BOOST500                              = 485,
	SI_FULL_SWING_K                          = 486,
	SI_MANA_PLUS                             = 487,
	SI_MUSTLE_M                              = 488,
	SI_LIFE_FORCE_F                          = 489,
	SI_VACUUM_EXTREME                        = 490,
	SI_SAVAGE_STEAK                          = 491,
	SI_COCKTAIL_WARG_BLOOD                   = 492,
	SI_MINOR_BBQ                             = 493,
	SI_SIROMA_ICE_TEA                        = 494,
	SI_DROCERA_HERB_STEAMED                  = 495,
	SI_PUTTI_TAILS_NOODLES                   = 496,
	SI_BANANA_BOMB                           = 497,
	SI_SUMMON_AGNI                           = 498,
	SI_SPELLBOOK4                            = 499,

	SI_SPELLBOOK5                            = 500,
	SI_SPELLBOOK6                            = 501,
	SI_SPELLBOOK7                            = 502,
	SI_ELEMENTAL_AGGRESSIVE                  = 503,
	SI_RETURN_TO_ELDICASTES                  = 504,
	SI_BANDING_DEFENCE                       = 505,
	SI_SKELSCROLL                            = 506,
	SI_DISTRUCTIONSCROLL                     = 507,
	SI_ROYALSCROLL                           = 508,
	SI_IMMUNITYSCROLL                        = 509,
	SI_MYSTICSCROLL                          = 510,
	SI_BATTLESCROLL                          = 511,
	SI_ARMORSCROLL                           = 512,
	SI_FREYJASCROLL                          = 513,
	SI_SOULSCROLL                            = 514,
	SI_CIRCLE_OF_FIRE                        = 515,
	SI_CIRCLE_OF_FIRE_OPTION                 = 516,
	SI_FIRE_CLOAK                            = 517,
	SI_FIRE_CLOAK_OPTION                     = 518,
	SI_WATER_SCREEN                          = 519,
	SI_WATER_SCREEN_OPTION                   = 520,
	SI_WATER_DROP                            = 521,
	SI_WATER_DROP_OPTION                     = 522,
	SI_WIND_STEP                             = 523,
	SI_WIND_STEP_OPTION                      = 524,
	SI_WIND_CURTAIN                          = 525,
	SI_WIND_CURTAIN_OPTION                   = 526,
	SI_WATER_BARRIER                         = 527,
	SI_ZEPHYR                                = 528,
	SI_SOLID_SKIN                            = 529,
	SI_SOLID_SKIN_OPTION                     = 530,
	SI_STONE_SHIELD                          = 531,
	SI_STONE_SHIELD_OPTION                   = 532,
	SI_POWER_OF_GAIA                         = 533,
	//SI_EL_WAIT                             = 534,
	//SI_EL_PASSIVE                          = 535,
	//SI_EL_DEFENSIVE                        = 536,
	//SI_EL_OFFENSIVE                        = 537,
	//SI_EL_COST                             = 538,
	SI_PYROTECHNIC                           = 539,
	SI_PYROTECHNIC_OPTION                    = 540,
	SI_HEATER                                = 541,
	SI_HEATER_OPTION                         = 542,
	SI_TROPIC                                = 543,
	SI_TROPIC_OPTION                         = 544,
	SI_AQUAPLAY                              = 545,
	SI_AQUAPLAY_OPTION                       = 546,
	SI_COOLER                                = 547,
	SI_COOLER_OPTION                         = 548,
	SI_CHILLY_AIR                            = 549,

	SI_CHILLY_AIR_OPTION                     = 550,
	SI_GUST                                  = 551,
	SI_GUST_OPTION                           = 552,
	SI_BLAST                                 = 553,
	SI_BLAST_OPTION                          = 554,
	SI_WILD_STORM                            = 555,
	SI_WILD_STORM_OPTION                     = 556,
	SI_PETROLOGY                             = 557,
	SI_PETROLOGY_OPTION                      = 558,
	SI_CURSED_SOIL                           = 559,
	SI_CURSED_SOIL_OPTION                    = 560,
	SI_UPHEAVAL                              = 561,
	SI_UPHEAVAL_OPTION                       = 562,
	SI_TIDAL_WEAPON                          = 563,
	SI_TIDAL_WEAPON_OPTION                   = 564,
	SI_ROCK_CRUSHER                          = 565,
	SI_ROCK_CRUSHER_ATK                      = 566,
	SI_FIRE_INSIGNIA                         = 567,
	SI_WATER_INSIGNIA                        = 568,
	SI_WIND_INSIGNIA                         = 569,
	SI_EARTH_INSIGNIA                        = 570,
	SI_EQUIPED_FLOOR                         = 571,
	SI_GUARDIAN_RECALL                       = 572,
	SI_MORA_BUFF                             = 573,
	SI_REUSE_LIMIT_G                         = 574,
	SI_REUSE_LIMIT_H                         = 575,
	SI_NEEDLE_OF_PARALYZE                    = 576,
	SI_PAIN_KILLER                           = 577,
	SI_G_LIFEPOTION                          = 578,
	SI_VITALIZE_POTION                       = 579,
	SI_LIGHT_OF_REGENE                       = 580,
	SI_OVERED_BOOST                          = 581,
	SI_SILENT_BREEZE                         = 582,
	SI_ODINS_POWER                           = 583,
	SI_STYLE_CHANGE                          = 584,
	SI_SONIC_CLAW_POSTDELAY                  = 585,
	//SI_                                    = 586,
	//SI_                                    = 587,
	//SI_                                    = 588,
	//SI_                                    = 589,
	//SI_                                    = 590,
	//SI_                                    = 591,
	//SI_                                    = 592,
	//SI_                                    = 593,
	//SI_                                    = 594,
	//SI_                                    = 595,
	SI_SILVERVEIN_RUSH_POSTDELAY             = 596,
	SI_MIDNIGHT_FRENZY_POSTDELAY             = 597,
	SI_GOLDENE_FERSE                         = 598,
	SI_ANGRIFFS_MODUS                        = 599,

	SI_TINDER_BREAKER                        = 600,
	SI_TINDER_BREAKER_POSTDELAY              = 601,
	SI_CBC                                   = 602,
	SI_CBC_POSTDELAY                         = 603,
	SI_EQC                                   = 604,
	SI_MAGMA_FLOW                            = 605,
	SI_GRANITIC_ARMOR                        = 606,
	SI_PYROCLASTIC                           = 607,
	SI_VOLCANIC_ASH                          = 608,
	SI_SPIRITS_SAVEINFO1                     = 609,
	SI_SPIRITS_SAVEINFO2                     = 610,
	SI_MAGIC_CANDY                           = 611,
	SI_SEARCH_STORE_INFO                     = 612,
	SI_ALL_RIDING                            = 613,
	SI_ALL_RIDING_REUSE_LIMIT                = 614,
	SI_MACRO                                 = 615,
	SI_MACRO_POSTDELAY                       = 616,
	SI_BEER_BOTTLE_CAP                       = 617,
	SI_OVERLAPEXPUP                          = 618,
	SI_PC_IZ_DUN05                           = 619,
	SI_CRUSHSTRIKE                           = 620,
	SI_MONSTER_TRANSFORM                     = 621,
	SI_SIT                                   = 622,
	SI_ONAIR                                 = 623,
	SI_MTF_ASPD                              = 624,
	SI_MTF_RANGEATK                          = 625,
	SI_MTF_MATK                              = 626,
	SI_MTF_MLEATKED                          = 627,
	SI_MTF_CRIDAMAGE                         = 628,
	SI_REUSE_LIMIT_MTF                       = 629,
	SI_MACRO_PERMIT                          = 630,
	SI_MACRO_PLAY                            = 631,
	SI_SKF_CAST                              = 632,
	SI_SKF_ASPD                              = 633,
	SI_SKF_ATK                               = 634,
	SI_SKF_MATK                              = 635,
	SI_REWARD_PLUSONLYJOBEXP                 = 636,
	SI_HANDICAPSTATE_NORECOVER               = 637,
	SI_SET_NUM_DEF                           = 638,
	SI_SET_NUM_MDEF                          = 639,
	SI_SET_PER_DEF                           = 640,
	SI_SET_PER_MDEF                          = 641,
	SI_PARTYBOOKING_SEARCH_DEALY             = 642,
	SI_PARTYBOOKING_REGISTER_DEALY           = 643,
	SI_PERIOD_TIME_CHECK_DETECT_SKILL        = 644,
	SI_KO_JYUMONJIKIRI                       = 645,
	SI_MEIKYOUSISUI                          = 646,
	SI_ATTHASTE_CASH                         = 647,
	SI_EQUIPPED_DIVINE_ARMOR                 = 648,
	SI_EQUIPPED_HOLY_ARMOR                   = 649,

	SI_2011RWC                               = 650,
	SI_KYOUGAKU                              = 651,
	SI_IZAYOI                                = 652,
	SI_ZENKAI                                = 653,
	SI_KG_KAGEHUMI                           = 654,
	SI_KYOMU                                 = 655,
	SI_KAGEMUSYA                             = 656,
	SI_ZANGETSU                              = 657,
	SI_PHI_DEMON                             = 658,
	SI_GENSOU                                = 659,
	SI_AKAITSUKI                             = 660,
	SI_TETANY                                = 661,
	SI_GM_BATTLE                             = 662,
	SI_GM_BATTLE2                            = 663,
	SI_2011RWC_SCROLL                        = 664,
	SI_ACTIVE_MONSTER_TRANSFORM              = 665,
	SI_MYSTICPOWDER                          = 666,
	SI_ECLAGE_RECALL                         = 667,
	SI_ENTRY_QUEUE_APPLY_DELAY               = 668,
	SI_REUSE_LIMIT_ECL                       = 669,
	SI_M_LIFEPOTION                          = 670,
	SI_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT = 671,
	SI_UNKNOWN_NAME                          = 672,
	SI_ON_PUSH_CART                          = 673,
	SI_HAT_EFFECT                            = 674,
	SI_FLOWER_LEAF                           = 675,
	SI_RAY_OF_PROTECTION                     = 676,
	SI_GLASTHEIM_ATK                         = 677,
	SI_GLASTHEIM_DEF                         = 678,
	SI_GLASTHEIM_HEAL                        = 679,
	SI_GLASTHEIM_HIDDEN                      = 680,
	SI_GLASTHEIM_STATE                       = 681,
	SI_GLASTHEIM_ITEMDEF                     = 682,
	SI_GLASTHEIM_HPSP                        = 683,
	SI_HOMUN_SKILL_POSTDELAY                 = 684,
	SI_ALMIGHTY                              = 685,
	SI_GVG_GIANT                             = 686,
	SI_GVG_GOLEM                             = 687,
	SI_GVG_STUN                              = 688,
	SI_GVG_STONE                             = 689,
	SI_GVG_FREEZ                             = 690,
	SI_GVG_SLEEP                             = 691,
	SI_GVG_CURSE                             = 692,
	SI_GVG_SILENCE                           = 693,
	SI_GVG_BLIND                             = 694,
	SI_CLIENT_ONLY_EQUIP_ARROW               = 695,
	SI_CLAN_INFO                             = 696,
	SI_JP_EVENT01                            = 697,
	SI_JP_EVENT02                            = 698,
	SI_JP_EVENT03                            = 699,

	SI_JP_EVENT04                            = 700,
	SI_TELEPORT_FIXEDCASTINGDELAY            = 701,
	SI_GEFFEN_MAGIC1                         = 702,
	SI_GEFFEN_MAGIC2                         = 703,
	SI_GEFFEN_MAGIC3                         = 704,
	SI_QUEST_BUFF1                           = 705,
	SI_QUEST_BUFF2                           = 706,
	SI_QUEST_BUFF3                           = 707,
	SI_REUSE_LIMIT_RECALL                    = 708,
	SI_SAVEPOSITION                          = 709,
	SI_HANDICAPSTATE_ICEEXPLO                = 710,
	SI_FENRIR_CARD                           = 711,
	SI_REUSE_LIMIT_ASPD_POTION               = 712,
	SI_MAXPAIN                               = 713,
	SI_PC_STOP                               = 714,
	SI_FRIGG_SONG                            = 715,
	SI_OFFERTORIUM                           = 716,
	SI_TELEKINESIS_INTENSE                   = 717,
	SI_MOONSTAR                              = 718,
	SI_STRANGELIGHTS                         = 719,
	SI_FULL_THROTTLE                         = 720,
	SI_REBOUND                               = 721,
	SI_UNLIMIT                               = 722,
	SI_KINGS_GRACE                           = 723,
	SI_ITEM_ATKMAX                           = 724,
	SI_ITEM_ATKMIN                           = 725,
	SI_ITEM_MATKMAX                          = 726,
	SI_ITEM_MATKMIN                          = 727,
	SI_SUPER_STAR                            = 728,
	SI_HIGH_RANKER                           = 729,
	SI_DARKCROW                              = 730,
	SI_2013_VALENTINE1                       = 731,
	SI_2013_VALENTINE2                       = 732,
	SI_2013_VALENTINE3                       = 733,
	SI_ILLUSIONDOPING                        = 734,
	//SI_WIDEWEB                             = 735,
	SI_CHILL                                 = 736,
	SI_BURNT                                 = 737,
	//SI_PCCAFE_PLAY_TIME                    = 738,
	//SI_TWISTED_TIME                        = 739,
	SI_FLASHCOMBO                            = 740,
	//SI_JITTER_BUFF1                        = 741,
	//SI_JITTER_BUFF2                        = 742,
	//SI_JITTER_BUFF3                        = 743,
	//SI_JITTER_BUFF4                        = 744,
	//SI_JITTER_BUFF5                        = 745,
	//SI_JITTER_BUFF6                        = 746,
	//SI_JITTER_BUFF7                        = 747,
	//SI_JITTER_BUFF8                        = 748,
	//SI_JITTER_BUFF9                        = 749,

	//SI_JITTER_BUFF10                       = 750,
	SI_CUP_OF_BOZA                           = 751,
	SI_B_TRAP                                = 752,
	SI_E_CHAIN                               = 753,
	SI_E_QD_SHOT_READY                       = 754,
	SI_C_MARKER                              = 755,
	SI_H_MINE                                = 756,
	SI_H_MINE_SPLASH                         = 757,
	SI_P_ALTER                               = 758,
	SI_HEAT_BARREL                           = 759,
	SI_ANTI_M_BLAST                          = 760,
	SI_SLUGSHOT                              = 761,
	SI_SWORDCLAN                             = 762,
	SI_ARCWANDCLAN                           = 763,
	SI_GOLDENMACECLAN                        = 764,
	SI_CROSSBOWCLAN                          = 765,
	SI_PACKING_ENVELOPE1                     = 766,
	SI_PACKING_ENVELOPE2                     = 767,
	SI_PACKING_ENVELOPE3                     = 768,
	SI_PACKING_ENVELOPE4                     = 769,
	SI_PACKING_ENVELOPE5                     = 770,
	SI_PACKING_ENVELOPE6                     = 771,
	SI_PACKING_ENVELOPE7                     = 772,
	SI_PACKING_ENVELOPE8                     = 773,
	SI_PACKING_ENVELOPE9                     = 774,
	SI_PACKING_ENVELOPE10                    = 775,
	SI_GLASTHEIM_TRANS                       = 776,
	//SI_ZONGZI_POUCH_TRANS                  = 777,
	SI_HEAT_BARREL_AFTER                     = 778,
	SI_DECORATION_OF_MUSIC                   = 779,
	//SI_OVERSEAEXPUP                        = 780,
	//SI_CLOWN_N_GYPSY_CARD                  = 781,
	//SI_OPEN_NPC_MARKET                     = 782,
	//SI_BEEF_RIB_STEW                       = 783,
	//SI_PORK_RIB_STEW                       = 784,
	//SI_CHUSEOK_MONDAY                      = 785,
	//SI_CHUSEOK_TUESDAY                     = 786,
	//SI_CHUSEOK_WEDNESDAY                   = 787,
	//SI_CHUSEOK_THURSDAY                    = 788,
	//SI_CHUSEOK_FRIDAY                      = 789,
	//SI_CHUSEOK_WEEKEND                     = 790,
	//SI_ALL_LIGHTGUARD                      = 791,
	//SI_ALL_LIGHTGUARD_COOL_TIME            = 792,
	SI_MTF_MHP                               = 793,
	SI_MTF_MSP                               = 794,
	SI_MTF_PUMPKIN                           = 795,
	SI_MTF_HITFLEE                           = 796,
	//SI_MTF_CRIDAMAGE2                      = 797,
	//SI_MTF_SPDRAIN                         = 798,
	//SI_ACUO_MINT_GUM                       = 799,

	//SI_S_HEALPOTION                        = 800,
	//SI_REUSE_LIMIT_S_HEAL_POTION           = 801,
	//SI_PLAYTIME_STATISTICS                 = 802,
	//SI_GN_CHANGEMATERIAL_OPERATOR          = 803,
	//SI_GN_MIX_COOKING_OPERATOR             = 804,
	//SI_GN_MAKEBOMB_OPERATOR                = 805,
	//SI_GN_S_PHARMACY_OPERATOR              = 806,
	//SI_SO_EL_ANALYSIS_DISASSEMBLY_OPERATOR = 807,
	//SI_SO_EL_ANALYSIS_COMBINATION_OPERATOR = 808,
	//SI_NC_MAGICDECOY_OPERATOR              = 809,
	//SI_GUILD_STORAGE                       = 810,
	//SI_GC_POISONINGWEAPON_OPERATOR         = 811,
	//SI_WS_WEAPONREFINE_OPERATOR            = 812,
	//SI_BS_REPAIRWEAPON_OPERATOR            = 813,
	//SI_GET_MAILBOX                         = 814,
	//SI_JUMPINGCLAN                         = 815,
	//SI_JP_OTP                              = 816,
	//SI_HANDICAPTOLERANCE_LEVELGAP          = 817,
	//SI_MTF_RANGEATK2                       = 818,
	//SI_MTF_ASPD2                           = 819,
	//SI_MTF_MATK2                           = 820,
	//SI_SHOW_NPCHPBAR                       = 821,
	SI_FLOWERSMOKE                           = 822,
	SI_FSTONE                                = 823,
	//SI_DAILYSENDMAILCNT                    = 824,
	//SI_QSCARABA                            = 825,
	SI_LJOSALFAR                             = 826,
	//SI_PAD_READER_KNIGHT                   = 827,
	//SI_PAD_READER_CRUSADER                 = 828,
	//SI_PAD_READER_BLACKSMITH               = 829,
	//SI_PAD_READER_ALCHEMIST                = 830,
	//SI_PAD_READER_ASSASSIN                 = 831,
	//SI_PAD_READER_ROGUE                    = 832,
	//SI_PAD_READER_WIZARD                   = 833,
	//SI_PAD_READER_SAGE                     = 834,
	//SI_PAD_READER_PRIEST                   = 835,
	//SI_PAD_READER_MONK                     = 836,
	//SI_PAD_READER_HUNTER                   = 837,
	//SI_PAD_READER_BARD                     = 838,
	//SI_PAD_READER_DANCER                   = 839,
	//SI_PAD_READER_TAEKWON                  = 840,
	//SI_PAD_READER_NINJA                    = 841,
	//SI_PAD_READER_GUNSLINGER               = 842,
	//SI_PAD_READER_SUPERNOVICE              = 843,
	//SI_ESSENCE_OF_TIME                     = 844,
	//SI_MINIGAME_ROULETTE                   = 845,
	//SI_MINIGAME_GOLD_POINT                 = 846,
	//SI_MINIGAME_SILVER_POINT               = 847,
	//SI_MINIGAME_BRONZE_POINT               = 848,
	SI_HAPPINESS_STAR                        = 849,

	//SI_SUMMEREVENT01                       = 850,
	//SI_SUMMEREVENT02                       = 851,
	//SI_SUMMEREVENT03                       = 852,
	//SI_SUMMEREVENT04                       = 853,
	//SI_SUMMEREVENT05                       = 854,
	//SI_MINIGAME_ROULETTE_BONUS_ITEM        = 855,
	//SI_DRESS_UP                            = 856,
	SI_MAPLE_FALLS                           = 857,
	//SI_ALL_NIFLHEIM_RECALL                 = 858,
	//SI_                                    = 859,
	//SI_MTF_MARIONETTE                      = 860,
	//SI_MTF_LUDE                            = 861,
	//SI_MTF_CRUISER                         = 862,
	SI_MERMAID_LONGING                       = 863,
	SI_MAGICAL_FEATHER                       = 864,
	//SI_DRACULA_CARD                        = 865,
	//SI_                                    = 866,
	//SI_LIMIT_POWER_BOOSTER                 = 867,
	//SI_                                    = 868,
	//SI_                                    = 869,
	//SI_                                    = 870,
	//SI_                                    = 871,
	SI_TIME_ACCESSORY                        = 872,
	//SI_EP16_DEF                            = 873,
	//SI_NORMAL_ATKED_SP                     = 874,
	//SI_BODYSTATE_STONECURSE                = 875,
	//SI_BODYSTATE_FREEZING                  = 876,
	//SI_BODYSTATE_STUN                      = 877,
	//SI_BODYSTATE_SLEEP                     = 878,
	//SI_BODYSTATE_UNDEAD                    = 879,
	//SI_BODYSTATE_STONECURSE_ING            = 880,
	//SI_BODYSTATE_BURNNING                  = 881,
	//SI_BODYSTATE_IMPRISON                  = 882,
	//SI_HEALTHSTATE_POISON                  = 883,
	//SI_HEALTHSTATE_CURSE                   = 884,
	//SI_HEALTHSTATE_SILENCE                 = 885,
	//SI_HEALTHSTATE_CONFUSION               = 886,
	//SI_HEALTHSTATE_BLIND                   = 887,
	//SI_HEALTHSTATE_ANGELUS                 = 888,
	//SI_HEALTHSTATE_BLOODING                = 889,
	//SI_HEALTHSTATE_HEAVYPOISON             = 890,
	//SI_HEALTHSTATE_FEAR                    = 891,
	//SI_CHERRY_BLOSSOM_CAKE                 = 892,
	//SI_SU_STOOP                            = 893,
	//SI_CATNIPPOWDER                        = 894,
	SI_BLOSSOM_FLUTTERING                    = 895,
	//SI_SV_ROOTTWIST                        = 896,
	//SI_ATTACK_PROPERTY_NOTHING             = 897,
	//SI_ATTACK_PROPERTY_WATER               = 898,
	//SI_ATTACK_PROPERTY_GROUND              = 899,

	//SI_ATTACK_PROPERTY_FIRE                = 900,
	//SI_ATTACK_PROPERTY_WIND                = 901,
	//SI_ATTACK_PROPERTY_POISON              = 902,
	//SI_ATTACK_PROPERTY_SAINT               = 903,
	//SI_ATTACK_PROPERTY_DARKNESS            = 904,
	//SI_ATTACK_PROPERTY_TELEKINESIS         = 905,
	//SI_ATTACK_PROPERTY_UNDEAD              = 906,
	//SI_RESIST_PROPERTY_NOTHING             = 907,
	//SI_RESIST_PROPERTY_WATER               = 908,
	//SI_RESIST_PROPERTY_GROUND              = 909,
	//SI_RESIST_PROPERTY_FIRE                = 910,
	//SI_RESIST_PROPERTY_WIND                = 911,
	//SI_RESIST_PROPERTY_POISON              = 912,
	//SI_RESIST_PROPERTY_SAINT               = 913,
	//SI_RESIST_PROPERTY_DARKNESS            = 914,
	//SI_RESIST_PROPERTY_TELEKINESIS         = 915,
	//SI_RESIST_PROPERTY_UNDEAD              = 916,
	//SI_BITESCAR                            = 917,
	//SI_ARCLOUSEDASH                        = 918,
	//SI_TUNAPARTY                           = 919,
	//SI_SHRIMP                              = 920,
	//SI_FRESHSHRIMP                         = 921,
	//SI_PERIOD_RECEIVEITEM                  = 922,
	//SI_PERIOD_PLUSEXP                      = 923,
	//SI_PERIOD_PLUSJOBEXP                   = 924,
	//SI_RUNEHELM                            = 925,
	//SI_HELM_VERKANA                        = 926,
	//SI_HELM_RHYDO                          = 927,
	//SI_HELM_TURISUS                        = 928,
	//SI_HELM_HAGALAS                        = 929,
	//SI_HELM_ISIA                           = 930,
	//SI_HELM_ASIR                           = 931,
	//SI_HELM_URJ                            = 932,
	//SI_SUHIDE                              = 933,
	//SI_                                    = 934,
	//SI_DORAM_BUF_01                        = 935,
	//SI_DORAM_BUF_02                        = 936,
	//SI_SPRITEMABLE                         = 937,
	//SI_EP16_2_BUFF_SS                      = 963,
	//SI_EP16_2_BUFF_SC                      = 964,
	//SI_EP16_2_BUFF_AC                      = 965,
#ifndef SI_MAX
	SI_MAX,
#endif
};

// JOINTBEAT stackable ailments
enum e_joint_break
{
	BREAK_ANKLE    = 0x01, // MoveSpeed reduced by 50%
	BREAK_WRIST    = 0x02, // ASPD reduced by 25%
	BREAK_KNEE     = 0x04, // MoveSpeed reduced by 30%, ASPD reduced by 10%
	BREAK_SHOULDER = 0x08, // DEF reduced by 50%
	BREAK_WAIST    = 0x10, // DEF reduced by 25%, ATK reduced by 25%
	BREAK_NECK     = 0x20, // current attack does 2x damage, inflicts 'bleeding' for 30 seconds
	BREAK_FLAGS    = BREAK_ANKLE | BREAK_WRIST | BREAK_KNEE | BREAK_SHOULDER | BREAK_WAIST | BREAK_NECK,
};

/**
 * Mob mode definitions. [Skotlex]
 *
 * @see doc/mob_db_mode_list.txt for a description of each mode.
 */
enum e_mode
{
	MD_NONE               = 0x00000000,
	MD_CANMOVE            = 0x00000001,
	MD_LOOTER             = 0x00000002,
	MD_AGGRESSIVE         = 0x00000004,
	MD_ASSIST             = 0x00000008,
	MD_CASTSENSOR_IDLE    = 0x00000010,
	MD_BOSS               = 0x00000020,
	MD_PLANT              = 0x00000040,
	MD_CANATTACK          = 0x00000080,
	MD_DETECTOR           = 0x00000100,
	MD_CASTSENSOR_CHASE   = 0x00000200,
	MD_CHANGECHASE        = 0x00000400,
	MD_ANGRY              = 0x00000800,
	MD_CHANGETARGET_MELEE = 0x00001000,
	MD_CHANGETARGET_CHASE = 0x00002000,
	MD_TARGETWEAK         = 0x00004000,
	MD_NOKNOCKBACK        = 0x00008000,
	//MD_RANDOMTARGET     = 0x00010000, // Not implemented
	// Note: This should be kept within INT_MAX, since it's often cast to int.
	MD_MASK               = 0x7FFFFFFF,
};

//Status change option definitions (options are what makes status changes visible to chars
//who were not on your field of sight when it happened)

//opt1: Non stackable status changes.
enum {
	OPT1_STONE = 1, //Petrified
	OPT1_FREEZE,
	OPT1_STUN,
	OPT1_SLEEP,
	//Aegis uses OPT1 = 5 to identify undead enemies (which also grants them immunity to the other opt1 changes)
	OPT1_STONEWAIT=6, //Petrifying
	OPT1_BURNING,
	OPT1_IMPRISON,
	OPT1_CRYSTALIZE,
};

//opt2: Stackable status changes.
enum {
	OPT2_POISON       = 0x0001,
	OPT2_CURSE        = 0x0002,
	OPT2_SILENCE      = 0x0004,
	OPT2_SIGNUMCRUCIS = 0x0008,
	OPT2_BLIND        = 0x0010,
	OPT2_ANGELUS      = 0x0020,
	OPT2_BLEEDING     = 0x0040,
	OPT2_DPOISON      = 0x0080,
	OPT2_FEAR         = 0x0100,
};

//opt3: (SHOW_EFST_*)
enum {
	OPT3_NORMAL           = 0x00000000,
	OPT3_QUICKEN          = 0x00000001,
	OPT3_OVERTHRUST       = 0x00000002,
	OPT3_ENERGYCOAT       = 0x00000004,
	OPT3_EXPLOSIONSPIRITS = 0x00000008,
	OPT3_STEELBODY        = 0x00000010,
	OPT3_BLADESTOP        = 0x00000020,
	OPT3_AURABLADE        = 0x00000040,
	OPT3_BERSERK          = 0x00000080,
	OPT3_LIGHTBLADE       = 0x00000100,
	OPT3_MOONLIT          = 0x00000200,
	OPT3_MARIONETTE       = 0x00000400,
	OPT3_ASSUMPTIO        = 0x00000800,
	OPT3_WARM             = 0x00001000,
	OPT3_KAITE            = 0x00002000,
	OPT3_BUNSIN           = 0x00004000,
	OPT3_SOULLINK         = 0x00008000,
	OPT3_UNDEAD           = 0x00010000,
	OPT3_CONTRACT         = 0x00020000,
};

//Defines for the manner system [Skotlex]
enum manner_flags
{
	MANNER_NOCHAT    = 0x01,
	MANNER_NOSKILL   = 0x02,
	MANNER_NOCOMMAND = 0x04,
	MANNER_NOITEM    = 0x08,
	MANNER_NOROOM    = 0x10,
};

//Define flags for the status_calc_bl function. [Skotlex]
enum scb_flag
{
	SCB_NONE    = 0x00000000,
	SCB_BASE    = 0x00000001,
	SCB_MAXHP   = 0x00000002,
	SCB_MAXSP   = 0x00000004,
	SCB_STR     = 0x00000008,
	SCB_AGI     = 0x00000010,
	SCB_VIT     = 0x00000020,
	SCB_INT     = 0x00000040,
	SCB_DEX     = 0x00000080,
	SCB_LUK     = 0x00000100,
	SCB_BATK    = 0x00000200,
	SCB_WATK    = 0x00000400,
	SCB_MATK    = 0x00000800,
	SCB_HIT     = 0x00001000,
	SCB_FLEE    = 0x00002000,
	SCB_DEF     = 0x00004000,
	SCB_DEF2    = 0x00008000,
	SCB_MDEF    = 0x00010000,
	SCB_MDEF2   = 0x00020000,
	SCB_SPEED   = 0x00040000,
	SCB_ASPD    = 0x00080000,
	SCB_DSPD    = 0x00100000,
	SCB_CRI     = 0x00200000,
	SCB_FLEE2   = 0x00400000,
	SCB_ATK_ELE = 0x00800000,
	SCB_DEF_ELE = 0x01000000,
	SCB_MODE    = 0x02000000,
	SCB_SIZE    = 0x04000000,
	SCB_RACE    = 0x08000000,
	SCB_RANGE   = 0x10000000,
	SCB_REGEN   = 0x20000000,
	SCB_DYE     = 0x40000000, // force cloth-dye change to 0 to avoid client crashes.
#if 0 // Currently No SC use it. Also, when this will be implemented, there will be need to change to 64bit variable
	SCB_BODY    = 0x80000000, // Force bodysStyle change to 0
#endif

	SCB_BATTLE  = 0x3FFFFFFE,
	SCB_ALL     = 0x3FFFFFFF
};

//Regen related flags.
enum e_regen {
	RGN_HP  = 0x01,
	RGN_SP  = 0x02,
	RGN_SHP = 0x04,
	RGN_SSP = 0x08,
};

enum e_status_calc_opt {
	SCO_NONE  = 0x0,
	SCO_FIRST = 0x1, /* trigger the calculations that should take place only onspawn/once */
	SCO_FORCE = 0x2, /* only relevant to BL_PC types, ensures call bypasses the queue caused by delayed damage */
};

//Define to determine who gets HP/SP consumed on doing skills/etc. [Skotlex]
#define BL_CONSUME (BL_PC|BL_HOM|BL_MER|BL_ELEM)
//Define to determine who has regen
#define BL_REGEN (BL_PC|BL_HOM|BL_MER|BL_ELEM)
//Define to determine who will receive a clif_status_change packet for effects that require one to display correctly
#define BL_SCEFFECT (BL_PC|BL_HOM|BL_MER|BL_MOB|BL_ELEM)

//Basic damage info of a weapon
//Required because players have two of these, one in status_data
//and another for their left hand weapon.
typedef struct weapon_atk {
	unsigned short atk, atk2;
	unsigned short range;
	unsigned char ele;
#ifdef RENEWAL
	unsigned short matk;
	unsigned char wlv;
#endif
} weapon_atk;

//For holding basic status (which can be modified by status changes)
struct status_data {
	unsigned int
		hp, sp,  // see status_cpy before adding members before hp and sp
		max_hp, max_sp;
	unsigned short
		str, agi, vit, int_, dex, luk,
		batk,
		matk_min, matk_max,
		speed,
		amotion, adelay, dmotion;
	uint32 mode;
	short
		hit, flee, cri, flee2,
		def2, mdef2,
#ifdef RENEWAL_ASPD
		aspd_rate2,
#endif
		aspd_rate;
	/**
	 * defType is RENEWAL dependent and defined in src/map/config/data/const.h
	 **/
	defType def,mdef;

	unsigned char
		def_ele, ele_lv,
		size, race;

	struct weapon_atk rhw, lhw; //Right Hand/Left Hand Weapon.
#ifdef RENEWAL
	int equip_atk;
#endif
};

//Additional regen data that only players have.
struct regen_data_sub {
	unsigned short
		hp,sp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp;
	} tick;

	//Regen rates (where every 1 means +100% regen)
	struct {
		unsigned char hp,sp;
	} rate;
};

struct regen_data {

	unsigned short flag; //Marks what stuff you may heal or not.
	unsigned short
		hp,sp,shp,ssp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp,shp,ssp;
	} tick;

	//Regen rates (where every 1 means +100% regen)
	struct {
		unsigned char
		hp,sp,shp,ssp;
	} rate;

	struct {
		unsigned walk:1;        //Can you regen even when walking?
		unsigned gc:1;          //Tags when you should have double regen due to GVG castle
		unsigned overweight :2; //overweight state (1: 50%, 2: 90%)
		unsigned block :2;      //Block regen flag (1: Hp, 2: Sp)
	} state;

	//skill-regen, sitting-skill-regen (since not all chars with regen need it)
	struct regen_data_sub *sregen, *ssregen;
};

struct sc_display_entry {
	enum sc_type type;
	int val1,val2,val3;
};

struct status_change_entry {
	int timer;
	int val1,val2,val3,val4;
	bool infinite_duration;
};

struct status_change {
	unsigned int option;// effect state (bitfield)
	unsigned int opt3;// skill state (bitfield)
	unsigned short opt1;// body state
	unsigned short opt2;// health state (bitfield)
	unsigned char count;
	//TODO: See if it is possible to implement the following SC's without requiring extra parameters while the SC is inactive.
	unsigned char jb_flag; //Joint Beat type flag
	//int sg_id; //ID of the previous Storm gust that hit you
	short comet_x, comet_y; // Point where src casted Comet - required to calculate damage from this point
/**
 * The Storm Gust counter was dropped in renewal
 **/
#ifndef RENEWAL
	unsigned char sg_counter; //Storm gust counter (previous hits from storm gust)
#endif
	unsigned char bs_counter; // Blood Sucker counter
	unsigned char fv_counter; // Force of vanguard counter
	struct status_change_entry *data[SC_MAX];
};


//Define for standard HP damage attacks.
#define status_fix_damage(src, target, hp, walkdelay) (status->damage((src), (target), (hp), 0, (walkdelay), 0))
//Define for standard HP/SP damage triggers.
#define status_zap(bl, hp, sp) (status->damage(NULL, (bl), (hp), (sp), 0, 1))
//Easier handling of status->percent_change
#define status_percent_heal(bl, hp_rate, sp_rate) (status->percent_change(NULL, (bl), -(hp_rate), -(sp_rate), 0))
#define status_percent_damage(src, target, hp_rate, sp_rate, kill) (status->percent_change((src), (target), (hp_rate), (sp_rate), (kill)?1:2))
//Instant kill with no drops/exp/etc
#define status_kill(bl) status_percent_damage(NULL, (bl), 100, 0, true)

#define status_get_range(bl)                 (status->get_status_data(bl)->rhw.range)
#define status_get_hp(bl)                    (status->get_status_data(bl)->hp)
#define status_get_max_hp(bl)                (status->get_status_data(bl)->max_hp)
#define status_get_sp(bl)                    (status->get_status_data(bl)->sp)
#define status_get_max_sp(bl)                (status->get_status_data(bl)->max_sp)
#define status_get_str(bl)                   (status->get_status_data(bl)->str)
#define status_get_agi(bl)                   (status->get_status_data(bl)->agi)
#define status_get_vit(bl)                   (status->get_status_data(bl)->vit)
#define status_get_int(bl)                   (status->get_status_data(bl)->int_)
#define status_get_dex(bl)                   (status->get_status_data(bl)->dex)
#define status_get_luk(bl)                   (status->get_status_data(bl)->luk)
#define status_get_hit(bl)                   (status->get_status_data(bl)->hit)
#define status_get_flee(bl)                  (status->get_status_data(bl)->flee)
#define status_get_mdef(bl)                  (status->get_status_data(bl)->mdef)
#define status_get_flee2(bl)                 (status->get_status_data(bl)->flee2)
#define status_get_def2(bl)                  (status->get_status_data(bl)->def2)
#define status_get_mdef2(bl)                 (status->get_status_data(bl)->mdef2)
#define status_get_critical(bl)              (status->get_status_data(bl)->cri)
#define status_get_batk(bl)                  (status->get_status_data(bl)->batk)
#define status_get_watk(bl)                  (status->get_status_data(bl)->rhw.atk)
#define status_get_watk2(bl)                 (status->get_status_data(bl)->rhw.atk2)
#define status_get_matk_max(bl)              (status->get_status_data(bl)->matk_max)
#define status_get_matk_min(bl)              (status->get_status_data(bl)->matk_min)
#define status_get_lwatk(bl)                 (status->get_status_data(bl)->lhw.atk)
#define status_get_lwatk2(bl)                (status->get_status_data(bl)->lhw.atk2)
#define status_get_adelay(bl)                (status->get_status_data(bl)->adelay)
#define status_get_amotion(bl)               (status->get_status_data(bl)->amotion)
#define status_get_dmotion(bl)               (status->get_status_data(bl)->dmotion)
#define status_get_element(bl)               (status->get_status_data(bl)->def_ele)
#define status_get_element_level(bl)         (status->get_status_data(bl)->ele_lv)
#define status_get_attack_sc_element(bl, sc) (status->calc_attack_element((bl), (sc), 0))
#define status_get_attack_element(bl)        (status->get_status_data(bl)->rhw.ele)
#define status_get_attack_lelement(bl)       (status->get_status_data(bl)->lhw.ele)
#define status_get_race(bl)                  (status->get_status_data(bl)->race)
#define status_get_size(bl)                  (status->get_status_data(bl)->size)
#define status_get_mode(bl)                  (status->get_status_data(bl)->mode)

//Short version, receives rate in 1->100 range, and does not uses a flag setting.
#define sc_start(src, bl, type, rate, val1, tick)                    (status->change_start((src),(bl),(type),100*(rate),(val1),0,0,0,(tick),SCFLAG_NONE))
#define sc_start2(src, bl, type, rate, val1, val2, tick)             (status->change_start((src),(bl),(type),100*(rate),(val1),(val2),0,0,(tick),SCFLAG_NONE))
#define sc_start4(src, bl, type, rate, val1, val2, val3, val4, tick) (status->change_start((src),(bl),(type),100*(rate),(val1),(val2),(val3),(val4),(tick),SCFLAG_NONE))

#define status_change_end(bl,type,tid) (status->change_end_((bl),(type),(tid),__FILE__,__LINE__))

#define status_calc_bl(bl, flag)        (status->calc_bl_((bl), (enum scb_flag)(flag), SCO_NONE))
#define status_calc_mob(md, opt)        (status->calc_bl_(&(md)->bl, SCB_ALL, (opt)))
#define status_calc_pet(pd, opt)        (status->calc_bl_(&(pd)->bl, SCB_ALL, (opt)))
#define status_calc_pc(sd, opt)         (status->calc_bl_(&(sd)->bl, SCB_ALL, (opt)))
#define status_calc_homunculus(hd, opt) (status->calc_bl_(&(hd)->bl, SCB_ALL, (opt)))
#define status_calc_mercenary(md, opt)  (status->calc_bl_(&(md)->bl, SCB_ALL, (opt)))
#define status_calc_elemental(ed, opt)  (status->calc_bl_(&(ed)->bl, SCB_ALL, (opt)))
#define status_calc_npc(nd, opt)        (status->calc_bl_(&(nd)->bl, SCB_ALL, (opt)))

// bonus values and upgrade chances for refining equipment
struct s_refine_info {
	int chance[MAX_REFINE]; // success chance
	int bonus[MAX_REFINE]; // cumulative fixed bonus damage
	int randombonus_max[MAX_REFINE]; // cumulative maximum random bonus damage
};

struct s_status_dbs {
BEGIN_ZEROED_BLOCK; /* Everything within this block will be memset to 0 when status_defaults() is executed */
	int max_weight_base[CLASS_COUNT];
	int HP_table[CLASS_COUNT][MAX_LEVEL + 1];
	int SP_table[CLASS_COUNT][MAX_LEVEL + 1];
	int aspd_base[CLASS_COUNT][MAX_SINGLE_WEAPON_TYPE+1]; // +1 for RENEWAL_ASPD
	sc_type Skill2SCTable[MAX_SKILL];  // skill  -> status
	int IconChangeTable[SC_MAX];          // status -> "icon" (icon is a bit of a misnomer, since there exist values with no icon associated)
	unsigned int ChangeFlagTable[SC_MAX]; // status -> flags
	int SkillChangeTable[SC_MAX];         // status -> skill
	int RelevantBLTypes[SI_MAX];          // "icon" -> enum bl_type (for clif->status_change to identify for which bl types to send packets)
	bool DisplayType[SC_MAX];
	/* */
	struct s_refine_info refine_info[REFINE_TYPE_MAX];
	/* */
	int atkmods[3][MAX_SINGLE_WEAPON_TYPE];//ATK weapon modification for size (size_fix.txt)
	char job_bonus[CLASS_COUNT][MAX_LEVEL];
	sc_conf_type sc_conf[SC_MAX];
END_ZEROED_BLOCK; /* End */
};

/*=====================================
* Interface : status.h
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
struct status_interface {

	/* vars */
	int current_equip_item_index;
	int current_equip_card_id;

	struct s_status_dbs *dbs;

	struct eri *data_ers; //For sc_data entries
	struct status_data dummy;
	int64 natural_heal_prev_tick;
	unsigned int natural_heal_diff_tick;
	/* */
	int (*init) (bool minimal);
	void (*final) (void);
	/* funcs */
	int (*get_refine_chance) (enum refine_type wlv, int refine);
	// for looking up associated data
	sc_type (*skill2sc) (int skill_id);
	int (*sc2skill) (sc_type sc);
	unsigned int (*sc2scb_flag) (sc_type sc);
	int (*type2relevant_bl_types) (int type);
	int (*get_sc_type) (sc_type idx);
	int (*damage) (struct block_list *src,struct block_list *target,int64 hp,int64 sp, int walkdelay, int flag);
	//Define for standard HP/SP skill-related cost triggers (mobs require no HP/SP to use skills)
	int (*charge) (struct block_list* bl, int64 hp, int64 sp);
	int (*percent_change) (struct block_list *src,struct block_list *target,signed char hp_rate, signed char sp_rate, int flag);
	//Used to set the hp/sp of an object to an absolute value (can't kill)
	int (*set_hp) (struct block_list *bl, unsigned int hp, int flag);
	int (*set_sp) (struct block_list *bl, unsigned int sp, int flag);
	int (*heal) (struct block_list *bl,int64 hp,int64 sp, int flag);
	int (*revive) (struct block_list *bl, unsigned char per_hp, unsigned char per_sp);
	int (*fixed_revive) (struct block_list *bl, unsigned int per_hp, unsigned int per_sp);
	struct regen_data * (*get_regen_data) (struct block_list *bl);
	struct status_data * (*get_status_data) (struct block_list *bl);
	struct status_data * (*get_base_status) (struct block_list *bl);
	const char *(*get_name) (const struct block_list *bl);
	int (*get_class) (const struct block_list *bl);
	int (*get_lv) (const struct block_list *bl);
	defType (*get_def) (struct block_list *bl);
	unsigned short (*get_speed) (struct block_list *bl);
	unsigned char (*calc_attack_element) (struct block_list *bl, struct status_change *sc, int element);
	int (*get_party_id) (const struct block_list *bl);
	int (*get_guild_id) (const struct block_list *bl);
	int (*get_emblem_id) (const struct block_list *bl);
	int (*get_mexp) (const struct block_list *bl);
	int (*get_race2) (const struct block_list *bl);
	struct view_data * (*get_viewdata) (struct block_list *bl);
	void (*set_viewdata) (struct block_list *bl, int class_);
	void (*change_init) (struct block_list *bl);
	struct status_change * (*get_sc) (struct block_list *bl);
	int (*isdead) (struct block_list *bl);
	int (*isimmune) (struct block_list *bl);
	int (*get_sc_def) (struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int tick, int flag);
	int (*change_start) (struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int val1, int val2, int val3, int val4, int tick, int flag);
	int (*change_end_) (struct block_list* bl, enum sc_type type, int tid, const char* file, int line);
	int (*kaahi_heal_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*change_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*change_timer_sub) (struct block_list* bl, va_list ap);
	int (*change_clear) (struct block_list* bl, int type);
	int (*change_clear_buffs) (struct block_list* bl, int type);
	void (*calc_bl_) (struct block_list *bl, enum scb_flag flag, enum e_status_calc_opt opt);
	int (*calc_mob_) (struct mob_data* md, enum e_status_calc_opt opt);
	int (*calc_pet_) (struct pet_data* pd, enum e_status_calc_opt opt);
	int (*calc_pc_) (struct map_session_data* sd, enum e_status_calc_opt opt);
	void (*calc_pc_additional) (struct map_session_data* sd, enum e_status_calc_opt opt);
	int (*calc_homunculus_) (struct homun_data *hd, enum e_status_calc_opt opt);
	int (*calc_mercenary_) (struct mercenary_data *md, enum e_status_calc_opt opt);
	int (*calc_elemental_) (struct elemental_data *ed, enum e_status_calc_opt opt);
	void (*calc_misc) (struct block_list *bl, struct status_data *st, int level);
	void (*calc_regen) (struct block_list *bl, struct status_data *st, struct regen_data *regen);
	void (*calc_regen_rate) (struct block_list *bl, struct regen_data *regen, struct status_change *sc);
	int (*check_skilluse) (struct block_list *src, struct block_list *target, uint16 skill_id, int flag); // [Skotlex]
	int (*check_visibility) (struct block_list *src, struct block_list *target); //[Skotlex]
	int (*change_spread) (struct block_list *src, struct block_list *bl);
	defType (*calc_def) (struct block_list *bl, struct status_change *sc, int def, bool viewable);
	short (*calc_def2) (struct block_list *bl, struct status_change *sc, int def2, bool viewable);
	defType (*calc_mdef) (struct block_list *bl, struct status_change *sc, int mdef, bool viewable);
	short (*calc_mdef2) (struct block_list *bl, struct status_change *sc, int mdef2, bool viewable);
	unsigned short (*calc_batk)(struct block_list *bl, struct status_change *sc, int batk, bool viewable);
	unsigned short(*base_matk) (struct block_list *bl, const struct status_data *st, int level);
	int (*get_weapon_atk) (struct block_list *src, struct weapon_atk *watk, int flag);
	int (*get_total_mdef) (struct block_list *src);
	int (*get_total_def) (struct block_list *src);
	int (*get_matk) (struct block_list *src, int flag);
	void (*update_matk) ( struct block_list *bl );
	int (*readdb) (void);

	void (*initChangeTables) (void);
	void (*initDummyData) (void);
	int (*base_amotion_pc) (struct map_session_data *sd, struct status_data *st);
	unsigned short (*base_atk) (const struct block_list *bl, const struct status_data *st);
	unsigned int (*get_base_maxhp) (const struct map_session_data *sd, const struct status_data *st);
	unsigned int (*get_base_maxsp) (const struct map_session_data *sd, const struct status_data *st);
	int (*calc_npc_) (struct npc_data *nd, enum e_status_calc_opt opt);
	unsigned short (*calc_str) (struct block_list *bl, struct status_change *sc, int str);
	unsigned short (*calc_agi) (struct block_list *bl, struct status_change *sc, int agi);
	unsigned short (*calc_vit) (struct block_list *bl, struct status_change *sc, int vit);
	unsigned short (*calc_int) (struct block_list *bl, struct status_change *sc, int int_);
	unsigned short (*calc_dex) (struct block_list *bl, struct status_change *sc, int dex);
	unsigned short (*calc_luk) (struct block_list *bl, struct status_change *sc, int luk);
	unsigned short (*calc_watk) (struct block_list *bl, struct status_change *sc, int watk, bool viewable);
	unsigned short (*calc_matk) (struct block_list *bl, struct status_change *sc, int matk, bool viewable);
	signed short (*calc_hit) (struct block_list *bl, struct status_change *sc, int hit, bool viewable);
	signed short (*calc_critical) (struct block_list *bl, struct status_change *sc, int critical, bool viewable);
	signed short (*calc_flee) (struct block_list *bl, struct status_change *sc, int flee, bool viewable);
	signed short (*calc_flee2) (struct block_list *bl, struct status_change *sc, int flee2, bool viewable);
	unsigned short (*calc_speed) (struct block_list *bl, struct status_change *sc, int speed);
	short (*calc_aspd_rate) (struct block_list *bl, struct status_change *sc, int aspd_rate);
	unsigned short (*calc_dmotion) (struct block_list *bl, struct status_change *sc, int dmotion);
	short (*calc_aspd) (struct block_list *bl, struct status_change *sc, short flag);
	short (*calc_fix_aspd) (struct block_list *bl, struct status_change *sc, int aspd);
	unsigned int (*calc_maxhp) (struct block_list *bl, struct status_change *sc, uint64 maxhp);
	unsigned int (*calc_maxsp) (struct block_list *bl, struct status_change *sc, unsigned int maxsp);
	unsigned char (*calc_element) (struct block_list *bl, struct status_change *sc, int element);
	unsigned char (*calc_element_lv) (struct block_list *bl, struct status_change *sc, int lv);
	uint32 (*calc_mode) (const struct block_list *bl, const struct status_change *sc, uint32 mode);
	unsigned short (*calc_ematk) (struct block_list *bl, struct status_change *sc, int matk);
	void (*calc_bl_main) (struct block_list *bl, int flag);
	void (*display_add) (struct map_session_data *sd, enum sc_type type, int dval1, int dval2, int dval3);
	void (*display_remove) (struct map_session_data *sd, enum sc_type type);
	int (*natural_heal) (struct block_list *bl, va_list args);
	int (*natural_heal_timer) (int tid, int64 tick, int id, intptr_t data);
	bool (*readdb_job2) (char *fields[], int columns, int current);
	bool (*readdb_sizefix) (char *fields[], int columns, int current);
	int (*readdb_refine_libconfig) (const char *filename);
	int (*readdb_refine_libconfig_sub) (struct config_setting_t *r, const char *name, const char *source);
	bool (*readdb_scconfig) (char *fields[], int columns, int current);
	void (*read_job_db) (void);
	void (*read_job_db_sub) (int idx, const char *name, struct config_setting_t *jdb);
	void (*set_sc) (uint16 skill_id, sc_type sc, int icon, unsigned int flag);
	void (*copy) (struct status_data *a, const struct status_data *b);
	unsigned short (*base_matk_min) (const struct status_data *st);
	unsigned short (*base_matk_max) (const struct status_data *st);
};

#ifdef HERCULES_CORE
void status_defaults(void);
#endif // HERCULES_CORE

HPShared struct status_interface *status;

#endif /* MAP_STATUS_H */
