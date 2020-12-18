/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
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
 * SC configuration type
 * @see db/sc_config.conf for more information
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
	SC_VISIBLE       = 0x100,
	SC_NO_BOSS       = 0x200,
	SC_BB_NO_RESET   = 0x400,
	SC_NO_MAGIC_BLOCK = 0x800
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

/**
 * Flags to be used with status->heal() and related functions.
 */
enum status_heal_flag {
	STATUS_HEAL_DEFAULT     = 0x00, ///< Default
	STATUS_HEAL_FORCED      = 0x01, ///< Forced healing (bypassing Berserk and similar)
	STATUS_HEAL_SHOWEFFECT  = 0x02, ///< Show the HP/SP heal effect
	STATUS_HEAL_ALLOWREVIVE = 0x04, ///< Force resurrection in case of dead targets.
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

	SC_FIRE_EXPANSION_TEAR_GAS_SOB,
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

	// Summoner
	SC_SUHIDE,
	SC_SU_STOOP,
	SC_SPRITEMABLE,
	SC_CATNIPPOWDER,
	SC_SV_ROOTTWIST,
	SC_BITESCAR,
	SC_ARCLOUSEDASH,
	SC_TUNAPARTY,
	SC_SHRIMP, // 650
	SC_FRESHSHRIMP,

	SC_DRESS_UP,

	// Rodex
	SC_DAILYSENDMAILCNT,

	// Clan System
	SC_CLAN_INFO,

	SC_SIT,
	SC_MOON,
	SC_TING,
	SC_DEVIL1,
	SC_RIDING,
	SC_FALCON,
	SC_WUGRIDER,
	SC_POSTDELAY,
	SC_ON_PUSH_CART,
	SC_RESIST_PROPERTY_WATER,
	SC_RESIST_PROPERTY_GROUND,
	SC_RESIST_PROPERTY_FIRE,
	SC_RESIST_PROPERTY_WIND,
	SC_CLIENT_ONLY_EQUIP_ARROW,
	SC_MADOGEAR,
	SC_POPECOOKIE,
	SC_VITALIZE_POTION,
	SC_SKF_MATK,
	SC_SKF_ATK,
	SC_SKF_ASPD,
	SC_SKF_CAST,
	SC_ALMIGHTY,
	SC_NO_RECOVER_STATE,

	// Rebel
	SC_FALLEN_ANGEL,
	SC_HEAT_BARREL,
	SC_PLATINUM_ALTER,
	SC_ANTI_MATERIAL_BLAST,
	SC_ETERNAL_CHAIN,
	SC_CRIMSON_MARKER,
	SC_QD_SHOT_READY,
	SC_HOWLING_MINE,
	SC_BIND_TRAP,

	// Star Emperor
	SC_SUNSTANCE,
	SC_STARSTANCE,
	SC_LUNARSTANCE,
	SC_NEWMOON,
	SC_FLASHKICK,
	SC_FALLINGSTAR,
	SC_LIGHTOFSUN,
	SC_LIGHTOFMOON,
	SC_LIGHTOFSTAR,
	SC_UNIVERSESTANCE,
	SC_NOVAEXPLOSING,
	SC_GRAVITYCONTROL,
	SC_CREATINGSTAR,
	SC_DIMENSION,
	SC_DIMENSION1,
	SC_DIMENSION2,

	// Soul Emperor
	SC_SOULCOLLECT,
	SC_SOULENERGY,
	SC_SOULREAPER,
	SC_SOULCURSE,
	SC_SP_SHA,
	SC_USE_SKILL_SP_SHA,
	SC_SP_SPA,
	SC_USE_SKILL_SP_SPA,
	SC_SOULUNITY,
	SC_SOULSHADOW,
	SC_SOULFAIRY,
	SC_SOULFALCON,
	SC_SOULGOLEM,
	SC_SOULDIVISION,

	SC_ACTIVE_MONSTER_TRANSFORM,

#ifndef SC_MAX
	SC_MAX, //Automatically updated max, used in for's to check we are within bounds.
#endif
} sc_type;

/// Official status change ids, used to display status icons in the client.
enum si_type {
	SI_BLANK = -1,
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
 * @see doc/mob_db_mode_list.md for a description of each mode.
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
enum e_opt1 {
	OPT1_NONE = 0,
	OPT1_STONE, //Petrified
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
enum e_opt2 {
	OPT2_NORMAL       = 0x0000,
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
enum e_opt3 {
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
	OPT3_ELEMENTAL_VEIL   = 0x00040000,
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
	unsigned int atk, atk2;
	unsigned short range;
	unsigned char ele;
#ifdef RENEWAL
	unsigned int matk;
	unsigned char wlv;
#endif
} weapon_atk;

//For holding basic status (which can be modified by status changes)
struct status_data {
	uint32
		hp, sp,  // see status_cpy before adding members before hp and sp
		max_hp, max_sp;
	uint16 str, agi, vit, int_, dex, luk;
	uint32
		batk,
		matk_min, matk_max,
		speed,
		amotion, adelay, dmotion,
		mode;
	int32 hit, flee, cri, flee2,
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
	int hp;
	int sp;

	//tick accumulation before healing.
	struct {
		unsigned int hp;
		unsigned int sp;
	} tick;

	//Regen rates on a base of 100
	struct {
		int hp;
		int sp;
	} rate;
};

struct regen_data {
	unsigned short flag; //Marks what stuff you may heal or not.
	int hp;
	int sp;

	//tick accumulation before healing.
	struct {
		unsigned int hp;
		unsigned int sp;
	} tick;

	//Regen rates on a base of 100
	struct {
		int hp;
		int sp;
	} rate;

	struct {
		unsigned walk:1;        //Can you regen even when walking?
		unsigned overweight :2; //overweight state (1: 50%, 2: 90%)
		unsigned block :2;      //Block regen flag (1: Hp, 2: Sp)
	} state;

	//skill-regen, sitting-skill-regen (since not all chars with regen need it)
	struct regen_data_sub *skill;
	struct regen_data_sub *sitting;
};

struct sc_display_entry {
	enum sc_type type;
	int val1,val2,val3;
};

struct status_change_entry {
	int timer;
	int total_tick;
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
#define sc_start(src, bl, type, rate, val1, tick, skill_id)                    (status->change_start((src),(bl),(type),100*(rate),(val1),0,0,0,(tick),SCFLAG_NONE,(skill_id)))
#define sc_start2(src, bl, type, rate, val1, val2, tick, skill_id)             (status->change_start((src),(bl),(type),100*(rate),(val1),(val2),0,0,(tick),SCFLAG_NONE,(skill_id)))
#define sc_start4(src, bl, type, rate, val1, val2, val3, val4, tick, skill_id) (status->change_start((src),(bl),(type),100*(rate),(val1),(val2),(val3),(val4),(tick),SCFLAG_NONE,(skill_id)))

#define status_change_end(bl,type,tid) (status->change_end_((bl),(type),(tid)))

#define status_calc_bl(bl, flag)        (status->calc_bl_((bl), (enum scb_flag)(flag), SCO_NONE))
#define status_calc_mob(md, opt)        (status->calc_bl_(&(md)->bl, SCB_ALL, (opt)))
#define status_calc_pet(pd, opt)        (status->calc_bl_(&(pd)->bl, SCB_ALL, (opt)))
#define status_calc_pc(sd, opt)         (status->calc_bl_(&(sd)->bl, SCB_ALL, (opt)))
#define status_calc_homunculus(hd, opt) (status->calc_bl_(&(hd)->bl, SCB_ALL, (opt)))
#define status_calc_mercenary(md, opt)  (status->calc_bl_(&(md)->bl, SCB_ALL, (opt)))
#define status_calc_elemental(ed, opt)  (status->calc_bl_(&(ed)->bl, SCB_ALL, (opt)))
#define status_calc_npc(nd, opt)        (status->calc_bl_(&(nd)->bl, SCB_ALL, (opt)))

struct s_status_dbs {
BEGIN_ZEROED_BLOCK; /* Everything within this block will be memset to 0 when status_defaults() is executed */
	int max_weight_base[CLASS_COUNT];
	int HP_table[CLASS_COUNT][MAX_LEVEL + 1];
	int SP_table[CLASS_COUNT][MAX_LEVEL + 1];
	int aspd_base[CLASS_COUNT][MAX_SINGLE_WEAPON_TYPE+1]; // +1 for RENEWAL_ASPD
	struct {
		int id;
		int relevant_bl_types;
	} IconChangeTable[SC_MAX];
	unsigned int ChangeFlagTable[SC_MAX]; // status -> flags
	int SkillChangeTable[SC_MAX];         // status -> skill
	bool DisplayType[SC_MAX];
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
	int current_equip_option_index;

	struct s_status_dbs *dbs;

	struct eri *data_ers; //For sc_data entries
	struct status_data dummy;
	int64 natural_heal_prev_tick;
	unsigned int natural_heal_diff_tick;
	/* */
	int (*init) (bool minimal);
	void (*final) (void);
	/* funcs */
	// for looking up associated data
	int (*sc2skill) (sc_type sc);
	unsigned int (*sc2scb_flag) (sc_type sc);
	int (*get_sc_relevant_bl_types) (sc_type type);
	int (*get_sc_type) (sc_type idx);
	int (*get_sc_icon) (sc_type type);
	int (*damage) (struct block_list *src,struct block_list *target,int64 hp,int64 sp, int walkdelay, int flag);
	//Define for standard HP/SP skill-related cost triggers (mobs require no HP/SP to use skills)
	int (*charge) (struct block_list* bl, int64 hp, int64 sp);
	int (*percent_change) (struct block_list *src,struct block_list *target,signed char hp_rate, signed char sp_rate, int flag);
	//Used to set the hp/sp of an object to an absolute value (can't kill)
	int (*set_hp) (struct block_list *bl, unsigned int hp, enum status_heal_flag flag);
	int (*set_sp) (struct block_list *bl, unsigned int sp, enum status_heal_flag flag);
	int (*heal) (struct block_list *bl,int64 hp,int64 sp, enum status_heal_flag flag);
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
	int (*get_sc_def) (struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int tick, int flag, int skill_id);
	int (*change_start) (struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int val1, int val2, int val3, int val4, int tick, int flag, int skill_id);
	int (*change_start_sub) (struct block_list *src, struct block_list *bl, enum sc_type type, int rate, int val1, int val2, int val3, int val4, int tick, int total_tick, int flag, int skill_id);
	int (*change_end_) (struct block_list* bl, enum sc_type type, int tid);
	bool (*is_immune_to_status) (struct status_change* sc, enum sc_type type);
	bool (*is_boss_resist_sc) (enum sc_type type);
	bool (*end_sc_before_start) (struct block_list *bl, struct status_data *st, struct status_change* sc, enum sc_type type, int undead_flag, int val1, int val2, int val3, int val4);
	void (*change_start_stop_action) (struct block_list *bl, enum sc_type type);
	int (*change_start_set_option) (struct block_list *bl, struct status_change* sc, enum sc_type type, int val1, int val2, int val3, int val4);
	int (*get_val_flag) (enum sc_type type);
	void (*change_start_display) (struct map_session_data *sd, enum sc_type type, int val1, int val2, int val3, int val4);
	bool (*change_start_unknown_sc) (struct block_list *src, struct block_list *bl, enum sc_type type, int calc_flag, int rate, int val1, int val2, int val3, int val4, int total_tick, int flag);
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
	void (*calc_pc_recover_hp) (struct map_session_data* sd, struct status_data *bstatus);
	int (*calc_homunculus_) (struct homun_data *hd, enum e_status_calc_opt opt);
	int (*calc_mercenary_) (struct mercenary_data *md, enum e_status_calc_opt opt);
	int (*calc_elemental_) (struct elemental_data *ed, enum e_status_calc_opt opt);
	void (*calc_misc) (struct block_list *bl, struct status_data *st, int level);
	void (*calc_regen_pc) (struct map_session_data *sd, struct status_data *st, struct regen_data *regen);
	void (*calc_regen_homunculus) (struct homun_data *hd, struct status_data *st, struct regen_data *regen);
	void (*calc_regen_mercenary) (struct mercenary_data *md, struct status_data *st, struct regen_data *regen);
	void (*calc_regen_elemental) (struct elemental_data *md, struct status_data *st, struct regen_data *regen);
	void (*calc_regen) (struct block_list *bl, struct status_data *st, struct regen_data *regen);
	void (*calc_regen_rate_pc) (struct map_session_data *sd, struct regen_data *regen);
	void (*calc_regen_rate_elemental) (struct elemental_data *md, struct regen_data *regen);
	void (*calc_regen_rate_homunculus) (struct homun_data *hd, struct regen_data *regen);
	void (*calc_regen_rate) (struct block_list *bl, struct regen_data *regen);
	bool (*check_skilluse_mapzone) (struct block_list *src, struct status_data *st, uint16 skill_id);
	int (*check_skilluse) (struct block_list *src, struct block_list *target, uint16 skill_id, int flag); // [Skotlex]
	int (*check_visibility) (struct block_list *src, struct block_list *target); //[Skotlex]
	int (*change_spread) (struct block_list *src, struct block_list *bl, int skill_id);
	defType (*calc_def) (struct block_list *bl, struct status_change *sc, int def, bool viewable);
	short (*calc_def2) (struct block_list *bl, struct status_change *sc, int def2, bool viewable);
	defType (*calc_mdef) (struct block_list *bl, struct status_change *sc, int mdef, bool viewable);
	short (*calc_mdef2) (struct block_list *bl, struct status_change *sc, int mdef2, bool viewable);
	int (*calc_batk)(struct block_list *bl, struct status_change *sc, int batk, bool viewable);
	int (*base_matk) (struct block_list *bl, const struct status_data *st, int level);
	int (*get_weapon_atk) (struct block_list *src, struct weapon_atk *watk, int flag);
	int (*get_total_mdef) (struct block_list *src);
	int (*get_total_def) (struct block_list *src);
	int (*get_matk) (struct block_list *src, int flag);
	void (*update_matk) ( struct block_list *bl );
	int (*readdb) (void);

	void (*initChangeTables) (void);
	void (*initDummyData) (void);
	int (*base_amotion_pc) (struct map_session_data *sd, struct status_data *st);
	int (*base_atk) (const struct block_list *bl, const struct status_data *st);
	unsigned int (*get_base_maxhp) (const struct map_session_data *sd, const struct status_data *st);
	unsigned int (*get_base_maxsp) (const struct map_session_data *sd, const struct status_data *st);
	unsigned int (*get_restart_hp) (const struct map_session_data *sd, const struct status_data *st);
	unsigned int (*get_restart_sp) (const struct map_session_data *sd, const struct status_data *st);
	int (*calc_npc_) (struct npc_data *nd, enum e_status_calc_opt opt);
	unsigned short (*calc_str) (struct block_list *bl, struct status_change *sc, int str);
	unsigned short (*calc_agi) (struct block_list *bl, struct status_change *sc, int agi);
	unsigned short (*calc_vit) (struct block_list *bl, struct status_change *sc, int vit);
	unsigned short (*calc_int) (struct block_list *bl, struct status_change *sc, int int_);
	unsigned short (*calc_dex) (struct block_list *bl, struct status_change *sc, int dex);
	unsigned short (*calc_luk) (struct block_list *bl, struct status_change *sc, int luk);
	int (*calc_watk) (struct block_list *bl, struct status_change *sc, int watk, bool viewable);
	int (*calc_matk) (struct block_list *bl, struct status_change *sc, int matk, bool viewable);
	signed int (*calc_hit) (struct block_list *bl, struct status_change *sc, int hit, bool viewable);
	signed int (*calc_critical) (struct block_list *bl, struct status_change *sc, int critical, bool viewable);
	signed int (*calc_flee) (struct block_list *bl, struct status_change *sc, int flee, bool viewable);
	signed int (*calc_flee2) (struct block_list *bl, struct status_change *sc, int flee2, bool viewable);
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
	int (*calc_ematk) (struct block_list *bl, struct status_change *sc, int matk);
	void (*calc_bl_main) (struct block_list *bl, int flag);
	void (*display_add) (struct map_session_data *sd, enum sc_type type, int dval1, int dval2, int dval3);
	void (*display_remove) (struct map_session_data *sd, enum sc_type type);
	int (*natural_heal) (struct block_list *bl, va_list args);
	int (*natural_heal_timer) (int tid, int64 tick, int id, intptr_t data);
	bool (*readdb_job2) (char *fields[], int columns, int current);
	bool (*readdb_sizefix) (char *fields[], int columns, int current);
	bool (*read_scdb_libconfig) (void);
	bool (*read_scdb_libconfig_sub) (struct config_setting_t *it, int idx, const char *source);
	bool (*read_scdb_libconfig_sub_flag) (struct config_setting_t *it, int type, const char *source);
	bool (*read_scdb_libconfig_sub_calcflag) (struct config_setting_t *it, int type, const char *source);
	bool (*read_scdb_libconfig_sub_flag_additional) (struct config_setting_t *it, int type, const char *source);
	bool (*read_scdb_libconfig_sub_calcflag_additional) (struct config_setting_t *it, int type, const char *source);
	bool (*read_scdb_libconfig_sub_skill) (struct config_setting_t *it, int type, const char *source);
	void (*read_job_db) (void);
	void (*read_job_db_sub) (int idx, const char *name, struct config_setting_t *jdb);
	void (*copy) (struct status_data *a, const struct status_data *b);
	int (*base_matk_min) (const struct status_data *st);
	int (*base_matk_max) (const struct status_data *st);
	void (*check_job_bonus) (int idx, const char *name, int class);
};

#ifdef HERCULES_CORE
void status_defaults(void);
#endif // HERCULES_CORE

HPShared struct status_interface *status;

#endif /* MAP_STATUS_H */
