// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_SKILL_H_
#define _MAP_SKILL_H_

#include "../common/mmo.h" // MAX_SKILL, struct square
#include "../common/db.h"
#include "map.h" // struct block_list

/**
 * Declarations
 **/
struct map_session_data;
struct homun_data;
struct skill_unit;
struct skill_unit_group;
struct status_change_entry;
struct square;

/**
 * Defines
 **/
#define MAX_SKILL_DB			MAX_SKILL
#define MAX_SKILL_PRODUCE_DB	270
#define MAX_PRODUCE_RESOURCE	10
#define MAX_SKILL_ARROW_DB		140
#define MAX_ARROW_RESOURCE		5
#define MAX_SKILL_ABRA_DB		210
#define MAX_SKILL_IMPROVISE_DB 30
#define MAX_SKILL_LEVEL 10
#define MAX_SKILL_UNIT_LAYOUT	45
#define MAX_SQUARE_LAYOUT		5	// 11*11 Placement of a maximum unit
#define MAX_SKILL_UNIT_COUNT ((MAX_SQUARE_LAYOUT*2+1)*(MAX_SQUARE_LAYOUT*2+1))
#define MAX_SKILLTIMERSKILL 15
#define MAX_SKILLUNITGROUP 25
#define MAX_SKILL_ITEM_REQUIRE	10
#define MAX_SKILLUNITGROUPTICKSET 25
#define MAX_SKILL_NAME_LENGTH 30

// (Epoque:) To-do: replace this macro with some sort of skill tree check (rather than hard-coded skill names)
#define skill_ischangesex(id) ( \
	((id) >= BD_ADAPTATION     && (id) <= DC_SERVICEFORYOU) || ((id) >= CG_ARROWVULCAN && (id) <= CG_MARIONETTE) || \
	((id) >= CG_LONGINGFREEDOM && (id) <= CG_TAROTCARD)     || ((id) >= WA_SWING_DANCE && (id) <= WM_UNLIMITED_HUMMING_VOICE))

#define MAX_SKILL_SPELLBOOK_DB	17
#define MAX_SKILL_MAGICMUSHROOM_DB 23

//Walk intervals at which chase-skills are attempted to be triggered.
#define WALK_SKILL_INTERVAL 5

/**
 * Enumerations
 **/

//Constants to identify the skill's inf value:
enum e_skill_inf {
	INF_ATTACK_SKILL  = 0x01,
	INF_GROUND_SKILL  = 0x02,
	INF_SELF_SKILL    = 0x04, // Skills casted on self where target is automatically chosen
	// 0x08 not assigned
	INF_SUPPORT_SKILL = 0x10,
	INF_TARGET_TRAP   = 0x20,
};

//Constants to identify a skill's nk value (damage properties)
//The NK value applies only to non INF_GROUND_SKILL skills
//when determining skill castend function to invoke.
enum e_skill_nk {
	NK_NO_DAMAGE      = 0x01,
	NK_SPLASH         = 0x02|0x04, // 0x4 = splash & split
	NK_SPLASHSPLIT    = 0x04,
	NK_NO_CARDFIX_ATK = 0x08,
	NK_NO_ELEFIX      = 0x10,
	NK_IGNORE_DEF     = 0x20,
	NK_IGNORE_FLEE    = 0x40,
	NK_NO_CARDFIX_DEF = 0x80,
};

//A skill with 3 would be no damage + splash: area of effect.
//Constants to identify a skill's inf2 value.
enum e_skill_inf2 {
	INF2_QUEST_SKILL    = 0x0001,
	INF2_NPC_SKILL      = 0x0002, // NPC skills are those that players can't have in their skill tree.
	INF2_WEDDING_SKILL  = 0x0004,
	INF2_SPIRIT_SKILL   = 0x0008,
	INF2_GUILD_SKILL    = 0x0010,
	INF2_SONG_DANCE     = 0x0020,
	INF2_ENSEMBLE_SKILL = 0x0040,
	INF2_TRAP           = 0x0080,
	INF2_TARGET_SELF    = 0x0100, // Refers to ground placed skills that will target the caster as well (like Grandcross)
	INF2_NO_TARGET_SELF = 0x0200,
	INF2_PARTY_ONLY     = 0x0400,
	INF2_GUILD_ONLY     = 0x0800,
	INF2_NO_ENEMY       = 0x1000,
	INF2_NOLP           = 0x2000, // Spells that can ignore Land Protector
	INF2_CHORUS_SKILL	= 0x4000, // Chorus skill
};


// Flags passed to skill_attack/skill_area_sub
enum e_skill_display {
	SD_LEVEL     = 0x1000, // skill_attack will send -1 instead of skill level (affects display of some skills)
	SD_ANIMATION = 0x2000, // skill_attack will use '5' instead of the skill's 'type' (this makes skills show an animation)
	SD_SPLASH    = 0x4000, // skill_area_sub will count targets in skill_area_temp[2]
	SD_PREAMBLE  = 0x8000, // skill_area_sub will transmit a 'magic' damage packet (-30000 dmg) for the first target selected
};

enum {
	UF_DEFNOTENEMY   = 0x0001,	 // If 'defunit_not_enemy' is set, the target is changed to 'friend'
	UF_NOREITERATION = 0x0002,	 // Spell cannot be stacked
	UF_NOFOOTSET     = 0x0004,	 // Spell cannot be cast near/on targets
	UF_NOOVERLAP     = 0x0008,	 // Spell effects do not overlap
	UF_PATHCHECK     = 0x0010,	 // Only cells with a shootable path will be placed
	UF_NOPC          = 0x0020,	 // May not target players
	UF_NOMOB         = 0x0040,	 // May not target mobs
	UF_SKILL         = 0x0080,	 // May target skills
	UF_DANCE         = 0x0100,	 // Dance
	UF_ENSEMBLE      = 0x0200,	 // Duet
	UF_SONG          = 0x0400,	 // Song
	UF_DUALMODE      = 0x0800,	 // Spells should trigger both ontimer and onplace/onout/onleft effects.
    UF_RANGEDSINGLEUNIT = 0x2000 // Hack for ranged layout, only display center
};

//Returns the cast type of the skill: ground cast, castend damage, castend no damage
enum { CAST_GROUND, CAST_DAMAGE, CAST_NODAMAGE };

enum wl_spheres {
	WLS_FIRE = 0x44,
	WLS_WIND,
	WLS_WATER,
	WLS_STONE,
};

enum {
	ST_NONE,
	ST_HIDING,
	ST_CLOAKING,
	ST_HIDDEN,
	ST_RIDING,
	ST_FALCON,
	ST_CART,
	ST_SHIELD,
	ST_SIGHT,
	ST_EXPLOSIONSPIRITS,
	ST_CARTBOOST,
	ST_RECOV_WEIGHT_RATE,
	ST_MOVE_ENABLE,
	ST_WATER,
	ST_RIDINGDRAGON,
	ST_WUG,
	ST_RIDINGWUG,
	ST_MADO,
	ST_ELEMENTALSPIRIT,
	ST_POISONINGWEAPON,
	ST_ROLLINGCUTTER,
	ST_MH_FIGHTING,
	ST_MH_GRAPPLING,
	ST_PECO,
};

enum e_skill {
	NV_BASIC = 1,
	
	SM_SWORD,
	SM_TWOHAND,
	SM_RECOVERY,
	SM_BASH,
	SM_PROVOKE,
	SM_MAGNUM,
	SM_ENDURE,
	
	MG_SRECOVERY,
	MG_SIGHT,
	MG_NAPALMBEAT,
	MG_SAFETYWALL,
	MG_SOULSTRIKE,
	MG_COLDBOLT,
	MG_FROSTDIVER,
	MG_STONECURSE,
	MG_FIREBALL,
	MG_FIREWALL,
	MG_FIREBOLT,
	MG_LIGHTNINGBOLT,
	MG_THUNDERSTORM,
	
	AL_DP,
	AL_DEMONBANE,
	AL_RUWACH,
	AL_PNEUMA,
	AL_TELEPORT,
	AL_WARP,
	AL_HEAL,
	AL_INCAGI,
	AL_DECAGI,
	AL_HOLYWATER,
	AL_CRUCIS,
	AL_ANGELUS,
	AL_BLESSING,
	AL_CURE,
	
	MC_INCCARRY,
	MC_DISCOUNT,
	MC_OVERCHARGE,
	MC_PUSHCART,
	MC_IDENTIFY,
	MC_VENDING,
	MC_MAMMONITE,
	
	AC_OWL,
	AC_VULTURE,
	AC_CONCENTRATION,
	AC_DOUBLE,
	AC_SHOWER,
	
	TF_DOUBLE,
	TF_MISS,
	TF_STEAL,
	TF_HIDING,
	TF_POISON,
	TF_DETOXIFY,
	
	ALL_RESURRECTION,
	
	KN_SPEARMASTERY,
	KN_PIERCE,
	KN_BRANDISHSPEAR,
	KN_SPEARSTAB,
	KN_SPEARBOOMERANG,
	KN_TWOHANDQUICKEN,
	KN_AUTOCOUNTER,
	KN_BOWLINGBASH,
	KN_RIDING,
	KN_CAVALIERMASTERY,
	
	PR_MACEMASTERY,
	PR_IMPOSITIO,
	PR_SUFFRAGIUM,
	PR_ASPERSIO,
	PR_BENEDICTIO,
	PR_SANCTUARY,
	PR_SLOWPOISON,
	PR_STRECOVERY,
	PR_KYRIE,
	PR_MAGNIFICAT,
	PR_GLORIA,
	PR_LEXDIVINA,
	PR_TURNUNDEAD,
	PR_LEXAETERNA,
	PR_MAGNUS,
	
	WZ_FIREPILLAR,
	WZ_SIGHTRASHER,
	WZ_FIREIVY,
	WZ_METEOR,
	WZ_JUPITEL,
	WZ_VERMILION,
	WZ_WATERBALL,
	WZ_ICEWALL,
	WZ_FROSTNOVA,
	WZ_STORMGUST,
	WZ_EARTHSPIKE,
	WZ_HEAVENDRIVE,
	WZ_QUAGMIRE,
	WZ_ESTIMATION,
	
	BS_IRON,
	BS_STEEL,
	BS_ENCHANTEDSTONE,
	BS_ORIDEOCON,
	BS_DAGGER,
	BS_SWORD,
	BS_TWOHANDSWORD,
	BS_AXE,
	BS_MACE,
	BS_KNUCKLE,
	BS_SPEAR,
	BS_HILTBINDING,
	BS_FINDINGORE,
	BS_WEAPONRESEARCH,
	BS_REPAIRWEAPON,
	BS_SKINTEMPER,
	BS_HAMMERFALL,
	BS_ADRENALINE,
	BS_WEAPONPERFECT,
	BS_OVERTHRUST,
	BS_MAXIMIZE,
	
	HT_SKIDTRAP,
	HT_LANDMINE,
	HT_ANKLESNARE,
	HT_SHOCKWAVE,
	HT_SANDMAN,
	HT_FLASHER,
	HT_FREEZINGTRAP,
	HT_BLASTMINE,
	HT_CLAYMORETRAP,
	HT_REMOVETRAP,
	HT_TALKIEBOX,
	HT_BEASTBANE,
	HT_FALCON,
	HT_STEELCROW,
	HT_BLITZBEAT,
	HT_DETECTING,
	HT_SPRINGTRAP,
	
	AS_RIGHT,
	AS_LEFT,
	AS_KATAR,
	AS_CLOAKING,
	AS_SONICBLOW,
	AS_GRIMTOOTH,
	AS_ENCHANTPOISON,
	AS_POISONREACT,
	AS_VENOMDUST,
	AS_SPLASHER,
	
	NV_FIRSTAID,
	NV_TRICKDEAD,
	SM_MOVINGRECOVERY,
	SM_FATALBLOW,
	SM_AUTOBERSERK,
	AC_MAKINGARROW,
	AC_CHARGEARROW,
	TF_SPRINKLESAND,
	TF_BACKSLIDING,
	TF_PICKSTONE,
	TF_THROWSTONE,
	MC_CARTREVOLUTION,
	MC_CHANGECART,
	MC_LOUD,
	AL_HOLYLIGHT,
	MG_ENERGYCOAT,
	
	NPC_PIERCINGATT,
	NPC_MENTALBREAKER,
	NPC_RANGEATTACK,
	NPC_ATTRICHANGE,
	NPC_CHANGEWATER,
	NPC_CHANGEGROUND,
	NPC_CHANGEFIRE,
	NPC_CHANGEWIND,
	NPC_CHANGEPOISON,
	NPC_CHANGEHOLY,
	NPC_CHANGEDARKNESS,
	NPC_CHANGETELEKINESIS,
	NPC_CRITICALSLASH,
	NPC_COMBOATTACK,
	NPC_GUIDEDATTACK,
	NPC_SELFDESTRUCTION,
	NPC_SPLASHATTACK,
	NPC_SUICIDE,
	NPC_POISON,
	NPC_BLINDATTACK,
	NPC_SILENCEATTACK,
	NPC_STUNATTACK,
	NPC_PETRIFYATTACK,
	NPC_CURSEATTACK,
	NPC_SLEEPATTACK,
	NPC_RANDOMATTACK,
	NPC_WATERATTACK,
	NPC_GROUNDATTACK,
	NPC_FIREATTACK,
	NPC_WINDATTACK,
	NPC_POISONATTACK,
	NPC_HOLYATTACK,
	NPC_DARKNESSATTACK,
	NPC_TELEKINESISATTACK,
	NPC_MAGICALATTACK,
	NPC_METAMORPHOSIS,
	NPC_PROVOCATION,
	NPC_SMOKING,
	NPC_SUMMONSLAVE,
	NPC_EMOTION,
	NPC_TRANSFORMATION,
	NPC_BLOODDRAIN,
	NPC_ENERGYDRAIN,
	NPC_KEEPING,
	NPC_DARKBREATH,
	NPC_DARKBLESSING,
	NPC_BARRIER,
	NPC_DEFENDER,
	NPC_LICK,
	NPC_HALLUCINATION,
	NPC_REBIRTH,
	NPC_SUMMONMONSTER,
	
	RG_SNATCHER,
	RG_STEALCOIN,
	RG_BACKSTAP,
	RG_TUNNELDRIVE,
	RG_RAID,
	RG_STRIPWEAPON,
	RG_STRIPSHIELD,
	RG_STRIPARMOR,
	RG_STRIPHELM,
	RG_INTIMIDATE,
	RG_GRAFFITI,
	RG_FLAGGRAFFITI,
	RG_CLEANER,
	RG_GANGSTER,
	RG_COMPULSION,
	RG_PLAGIARISM,
	
	AM_AXEMASTERY,
	AM_LEARNINGPOTION,
	AM_PHARMACY,
	AM_DEMONSTRATION,
	AM_ACIDTERROR,
	AM_POTIONPITCHER,
	AM_CANNIBALIZE,
	AM_SPHEREMINE,
	AM_CP_WEAPON,
	AM_CP_SHIELD,
	AM_CP_ARMOR,
	AM_CP_HELM,
	AM_BIOETHICS,
	AM_BIOTECHNOLOGY,
	AM_CREATECREATURE,
	AM_CULTIVATION,
	AM_FLAMECONTROL,
	AM_CALLHOMUN,
	AM_REST,
	AM_DRILLMASTER,
	AM_HEALHOMUN,
	AM_RESURRECTHOMUN,
	
	CR_TRUST,
	CR_AUTOGUARD,
	CR_SHIELDCHARGE,
	CR_SHIELDBOOMERANG,
	CR_REFLECTSHIELD,
	CR_HOLYCROSS,
	CR_GRANDCROSS,
	CR_DEVOTION,
	CR_PROVIDENCE,
	CR_DEFENDER,
	CR_SPEARQUICKEN,
	
	MO_IRONHAND,
	MO_SPIRITSRECOVERY,
	MO_CALLSPIRITS,
	MO_ABSORBSPIRITS,
	MO_TRIPLEATTACK,
	MO_BODYRELOCATION,
	MO_DODGE,
	MO_INVESTIGATE,
	MO_FINGEROFFENSIVE,
	MO_STEELBODY,
	MO_BLADESTOP,
	MO_EXPLOSIONSPIRITS,
	MO_EXTREMITYFIST,
	MO_CHAINCOMBO,
	MO_COMBOFINISH,
	
	SA_ADVANCEDBOOK,
	SA_CASTCANCEL,
	SA_MAGICROD,
	SA_SPELLBREAKER,
	SA_FREECAST,
	SA_AUTOSPELL,
	SA_FLAMELAUNCHER,
	SA_FROSTWEAPON,
	SA_LIGHTNINGLOADER,
	SA_SEISMICWEAPON,
	SA_DRAGONOLOGY,
	SA_VOLCANO,
	SA_DELUGE,
	SA_VIOLENTGALE,
	SA_LANDPROTECTOR,
	SA_DISPELL,
	SA_ABRACADABRA,
	SA_MONOCELL,
	SA_CLASSCHANGE,
	SA_SUMMONMONSTER,
	SA_REVERSEORCISH,
	SA_DEATH,
	SA_FORTUNE,
	SA_TAMINGMONSTER,
	SA_QUESTION,
	SA_GRAVITY,
	SA_LEVELUP,
	SA_INSTANTDEATH,
	SA_FULLRECOVERY,
	SA_COMA,
	
	BD_ADAPTATION,
	BD_ENCORE,
	BD_LULLABY,
	BD_RICHMANKIM,
	BD_ETERNALCHAOS,
	BD_DRUMBATTLEFIELD,
	BD_RINGNIBELUNGEN,
	BD_ROKISWEIL,
	BD_INTOABYSS,
	BD_SIEGFRIED,
	BD_RAGNAROK,
	
	BA_MUSICALLESSON,
	BA_MUSICALSTRIKE,
	BA_DISSONANCE,
	BA_FROSTJOKER,
	BA_WHISTLE,
	BA_ASSASSINCROSS,
	BA_POEMBRAGI,
	BA_APPLEIDUN,
	
	DC_DANCINGLESSON,
	DC_THROWARROW,
	DC_UGLYDANCE,
	DC_SCREAM,
	DC_HUMMING,
	DC_DONTFORGETME,
	DC_FORTUNEKISS,
	DC_SERVICEFORYOU,
	
	NPC_RANDOMMOVE,
	NPC_SPEEDUP,
	NPC_REVENGE,
	
	WE_MALE,
	WE_FEMALE,
	WE_CALLPARTNER,
	
	ITM_TOMAHAWK,
	
	NPC_DARKCROSS,
	NPC_GRANDDARKNESS,
	NPC_DARKSTRIKE,
	NPC_DARKTHUNDER,
	NPC_STOP,
	NPC_WEAPONBRAKER,
	NPC_ARMORBRAKE,
	NPC_HELMBRAKE,
	NPC_SHIELDBRAKE,
	NPC_UNDEADATTACK,
	NPC_CHANGEUNDEAD,
	NPC_POWERUP,
	NPC_AGIUP,
	NPC_SIEGEMODE,
	NPC_CALLSLAVE,
	NPC_INVISIBLE,
	NPC_RUN,
	
	LK_AURABLADE,
	LK_PARRYING,
	LK_CONCENTRATION,
	LK_TENSIONRELAX,
	LK_BERSERK,
	LK_FURY,
	HP_ASSUMPTIO,
	HP_BASILICA,
	HP_MEDITATIO,
	HW_SOULDRAIN,
	HW_MAGICCRASHER,
	HW_MAGICPOWER,
	PA_PRESSURE,
	PA_SACRIFICE,
	PA_GOSPEL,
	CH_PALMSTRIKE,
	CH_TIGERFIST,
	CH_CHAINCRUSH,
	PF_HPCONVERSION,
	PF_SOULCHANGE,
	PF_SOULBURN,
	ASC_KATAR,
	ASC_HALLUCINATION,
	ASC_EDP,
	ASC_BREAKER,
	SN_SIGHT,
	SN_FALCONASSAULT,
	SN_SHARPSHOOTING,
	SN_WINDWALK,
	WS_MELTDOWN,
	WS_CREATECOIN,
	WS_CREATENUGGET,
	WS_CARTBOOST,
	WS_SYSTEMCREATE,
	ST_CHASEWALK,
	ST_REJECTSWORD,
	ST_STEALBACKPACK,
	CR_ALCHEMY,
	CR_SYNTHESISPOTION,
	CG_ARROWVULCAN,
	CG_MOONLIT,
	CG_MARIONETTE,
	LK_SPIRALPIERCE,
	LK_HEADCRUSH,
	LK_JOINTBEAT,
	HW_NAPALMVULCAN,
	CH_SOULCOLLECT,
	PF_MINDBREAKER,
	PF_MEMORIZE,
	PF_FOGWALL,
	PF_SPIDERWEB,
	ASC_METEORASSAULT,
	ASC_CDP,
	WE_BABY,
	WE_CALLPARENT,
	WE_CALLBABY,
	
	TK_RUN,
	TK_READYSTORM,
	TK_STORMKICK,
	TK_READYDOWN,
	TK_DOWNKICK,
	TK_READYTURN,
	TK_TURNKICK,
	TK_READYCOUNTER,
	TK_COUNTER,
	TK_DODGE,
	TK_JUMPKICK,
	TK_HPTIME,
	TK_SPTIME,
	TK_POWER,
	TK_SEVENWIND,
	TK_HIGHJUMP,
	
	SG_FEEL,
	SG_SUN_WARM,
	SG_MOON_WARM,
	SG_STAR_WARM,
	SG_SUN_COMFORT,
	SG_MOON_COMFORT,
	SG_STAR_COMFORT,
	SG_HATE,
	SG_SUN_ANGER,
	SG_MOON_ANGER,
	SG_STAR_ANGER,
	SG_SUN_BLESS,
	SG_MOON_BLESS,
	SG_STAR_BLESS,
	SG_DEVIL,
	SG_FRIEND,
	SG_KNOWLEDGE,
	SG_FUSION,
	
	SL_ALCHEMIST,
	AM_BERSERKPITCHER,
	SL_MONK,
	SL_STAR,
	SL_SAGE,
	SL_CRUSADER,
	SL_SUPERNOVICE,
	SL_KNIGHT,
	SL_WIZARD,
	SL_PRIEST,
	SL_BARDDANCER,
	SL_ROGUE,
	SL_ASSASIN,
	SL_BLACKSMITH,
	BS_ADRENALINE2,
	SL_HUNTER,
	SL_SOULLINKER,
	SL_KAIZEL,
	SL_KAAHI,
	SL_KAUPE,
	SL_KAITE,
	SL_KAINA,
	SL_STIN,
	SL_STUN,
	SL_SMA,
	SL_SWOO,
	SL_SKE,
	SL_SKA,
	
	SM_SELFPROVOKE,
	NPC_EMOTION_ON,
	ST_PRESERVE,
	ST_FULLSTRIP,
	WS_WEAPONREFINE,
	CR_SLIMPITCHER,
	CR_FULLPROTECTION,
	PA_SHIELDCHAIN,
	HP_MANARECHARGE,
	PF_DOUBLECASTING,
	HW_GANBANTEIN,
	HW_GRAVITATION,
	WS_CARTTERMINATION,
	WS_OVERTHRUSTMAX,
	CG_LONGINGFREEDOM,
	CG_HERMODE,
	CG_TAROTCARD,
	CR_ACIDDEMONSTRATION,
	CR_CULTIVATION,
	ITEM_ENCHANTARMS,
	TK_MISSION,
	SL_HIGH,
	KN_ONEHAND,
	AM_TWILIGHT1,
	AM_TWILIGHT2,
	AM_TWILIGHT3,
	HT_POWER,
	
	GS_GLITTERING,
	GS_FLING,
	GS_TRIPLEACTION,
	GS_BULLSEYE,
	GS_MADNESSCANCEL,
	GS_ADJUSTMENT,
	GS_INCREASING,
	GS_MAGICALBULLET,
	GS_CRACKER,
	GS_SINGLEACTION,
	GS_SNAKEEYE,
	GS_CHAINACTION,
	GS_TRACKING,
	GS_DISARM,
	GS_PIERCINGSHOT,
	GS_RAPIDSHOWER,
	GS_DESPERADO,
	GS_GATLINGFEVER,
	GS_DUST,
	GS_FULLBUSTER,
	GS_SPREADATTACK,
	GS_GROUNDDRIFT,
	
	NJ_TOBIDOUGU,
	NJ_SYURIKEN,
	NJ_KUNAI,
	NJ_HUUMA,
	NJ_ZENYNAGE,
	NJ_TATAMIGAESHI,
	NJ_KASUMIKIRI,
	NJ_SHADOWJUMP,
	NJ_KIRIKAGE,
	NJ_UTSUSEMI,
	NJ_BUNSINJYUTSU,
	NJ_NINPOU,
	NJ_KOUENKA,
	NJ_KAENSIN,
	NJ_BAKUENRYU,
	NJ_HYOUSENSOU,
	NJ_SUITON,
	NJ_HYOUSYOURAKU,
	NJ_HUUJIN,
	NJ_RAIGEKISAI,
	NJ_KAMAITACHI,
	NJ_NEN,
	NJ_ISSEN,
	
	MB_FIGHTING,
	MB_NEUTRAL,
	MB_TAIMING_PUTI,
	MB_WHITEPOTION,
	MB_MENTAL,
	MB_CARDPITCHER,
	MB_PETPITCHER,
	MB_BODYSTUDY,
	MB_BODYALTER,
	MB_PETMEMORY,
	MB_M_TELEPORT,
	MB_B_GAIN,
	MB_M_GAIN,
	MB_MISSION,
	MB_MUNAKKNOWLEDGE,
	MB_MUNAKBALL,
	MB_SCROLL,
	MB_B_GATHERING,
	MB_M_GATHERING,
	MB_B_EXCLUDE,
	MB_B_DRIFT,
	MB_B_WALLRUSH,
	MB_M_WALLRUSH,
	MB_B_WALLSHIFT,
	MB_M_WALLCRASH,
	MB_M_REINCARNATION,
	MB_B_EQUIP,
	
	SL_DEATHKNIGHT,
	SL_COLLECTOR,
	SL_NINJA,
	SL_GUNNER,
	AM_TWILIGHT4,
	DA_RESET,
	DE_BERSERKAIZER,
	DA_DARKPOWER,
	
	DE_PASSIVE,
	DE_PATTACK,
	DE_PSPEED,
	DE_PDEFENSE,
	DE_PCRITICAL,
	DE_PHP,
	DE_PSP,
	DE_RESET,
	DE_RANKING,
	DE_PTRIPLE,
	DE_ENERGY,
	DE_NIGHTMARE,
	DE_SLASH,
	DE_COIL,
	DE_WAVE,
	DE_REBIRTH,
	DE_AURA,
	DE_FREEZER,
	DE_CHANGEATTACK,
	DE_PUNISH,
	DE_POISON,
	DE_INSTANT,
	DE_WARNING,
	DE_RANKEDKNIFE,
	DE_RANKEDGRADIUS,
	DE_GAUGE,
	DE_GTIME,
	DE_GPAIN,
	DE_GSKILL,
	DE_GKILL,
	DE_ACCEL,
	DE_BLOCKDOUBLE,
	DE_BLOCKMELEE,
	DE_BLOCKFAR,
	DE_FRONTATTACK,
	DE_DANGERATTACK,
	DE_TWINATTACK,
	DE_WINDATTACK,
	DE_WATERATTACK,
	
	DA_ENERGY,
	DA_CLOUD,
	DA_FIRSTSLOT,
	DA_HEADDEF,
	DA_SPACE,
	DA_TRANSFORM,
	DA_EXPLOSION,
	DA_REWARD,
	DA_CRUSH,
	DA_ITEMREBUILD,
	DA_ILLUSION,
	DA_NUETRALIZE,
	DA_RUNNER,
	DA_TRANSFER,
	DA_WALL,
	DA_ZENY,
	DA_REVENGE,
	DA_EARPLUG,
	DA_CONTRACT,
	DA_BLACK,
	DA_DREAM,
	DA_MAGICCART,
	DA_COPY,
	DA_CRYSTAL,
	DA_EXP,
	DA_CARTSWING,
	DA_REBUILD,
	DA_JOBCHANGE,
	DA_EDARKNESS,
	DA_EGUARDIAN,
	DA_TIMEOUT,
	ALL_TIMEIN,
	DA_ZENYRANK,
	DA_ACCESSORYMIX,
	
	NPC_EARTHQUAKE,
	NPC_FIREBREATH,
	NPC_ICEBREATH,
	NPC_THUNDERBREATH,
	NPC_ACIDBREATH,
	NPC_DARKNESSBREATH,
	NPC_DRAGONFEAR,
	NPC_BLEEDING,
	NPC_PULSESTRIKE,
	NPC_HELLJUDGEMENT,
	NPC_WIDESILENCE,
	NPC_WIDEFREEZE,
	NPC_WIDEBLEEDING,
	NPC_WIDESTONE,
	NPC_WIDECONFUSE,
	NPC_WIDESLEEP,
	NPC_WIDESIGHT,
	NPC_EVILLAND,
	NPC_MAGICMIRROR,
	NPC_SLOWCAST,
	NPC_CRITICALWOUND,
	NPC_EXPULSION,
	NPC_STONESKIN,
	NPC_ANTIMAGIC,
	NPC_WIDECURSE,
	NPC_WIDESTUN,
	NPC_VAMPIRE_GIFT,
	NPC_WIDESOULDRAIN,
	
	ALL_INCCARRY,
	NPC_TALK,
	NPC_HELLPOWER,
	NPC_WIDEHELLDIGNITY,
	NPC_INVINCIBLE,
	NPC_INVINCIBLEOFF,
	NPC_ALLHEAL,
	GM_SANDMAN,
	CASH_BLESSING,
	CASH_INCAGI,
	CASH_ASSUMPTIO,
	ALL_CATCRY,
	ALL_PARTYFLEE,
	ALL_ANGEL_PROTECT,
	ALL_DREAM_SUMMERNIGHT,
	NPC_CHANGEUNDEAD2,
	ALL_REVERSEORCISH,
	ALL_WEWISH,
	ALL_SONKRAN,
	NPC_WIDEHEALTHFEAR,
	NPC_WIDEBODYBURNNING,
	NPC_WIDEFROSTMISTY,
	NPC_WIDECOLD,
	NPC_WIDE_DEEP_SLEEP,
	NPC_WIDESIREN,
	NPC_VENOMFOG,
	NPC_MILLENNIUMSHIELD,
	NPC_COMET,
	NPC_WIDEWEB,
	NPC_WIDESUCK,
	NPC_STORMGUST2,
	NPC_FIRESTORM,
	NPC_REVERBERATION,
	NPC_REVERBERATION_ATK,
	NPC_LEX_AETERNA,

	KN_CHARGEATK = 1001,
	CR_SHRINK,
	AS_SONICACCEL,
	AS_VENOMKNIFE,
	RG_CLOSECONFINE,
	WZ_SIGHTBLASTER,
	SA_CREATECON,
	SA_ELEMENTWATER,
	HT_PHANTASMIC,
	BA_PANGVOICE,
	DC_WINKCHARM,
	BS_UNFAIRLYTRICK,
	BS_GREED,
	PR_REDEMPTIO,
	MO_KITRANSLATION,
	MO_BALKYOUNG,
	SA_ELEMENTGROUND,
	SA_ELEMENTFIRE,
	SA_ELEMENTWIND,
	
	RK_ENCHANTBLADE = 2001,
	RK_SONICWAVE,
	RK_DEATHBOUND,
	RK_HUNDREDSPEAR,
	RK_WINDCUTTER,
	RK_IGNITIONBREAK,
	RK_DRAGONTRAINING,
	RK_DRAGONBREATH,
	RK_DRAGONHOWLING,
	RK_RUNEMASTERY,
	RK_MILLENNIUMSHIELD,
	RK_CRUSHSTRIKE,
	RK_REFRESH,
	RK_GIANTGROWTH,
	RK_STONEHARDSKIN,
	RK_VITALITYACTIVATION,
	RK_STORMBLAST,
	RK_FIGHTINGSPIRIT,
	RK_ABUNDANCE,
	RK_PHANTOMTHRUST,
	
	GC_VENOMIMPRESS,
	GC_CROSSIMPACT,
	GC_DARKILLUSION,
	GC_RESEARCHNEWPOISON,
	GC_CREATENEWPOISON,
	GC_ANTIDOTE,
	GC_POISONINGWEAPON,
	GC_WEAPONBLOCKING,
	GC_COUNTERSLASH,
	GC_WEAPONCRUSH,
	GC_VENOMPRESSURE,
	GC_POISONSMOKE,
	GC_CLOAKINGEXCEED,
	GC_PHANTOMMENACE,
	GC_HALLUCINATIONWALK,
	GC_ROLLINGCUTTER,
	GC_CROSSRIPPERSLASHER,
	
	AB_JUDEX,
	AB_ANCILLA,
	AB_ADORAMUS,
	AB_CLEMENTIA,
	AB_CANTO,
	AB_CHEAL,
	AB_EPICLESIS,
	AB_PRAEFATIO,
	AB_ORATIO,
	AB_LAUDAAGNUS,
	AB_LAUDARAMUS,
	AB_EUCHARISTICA,
	AB_RENOVATIO,
	AB_HIGHNESSHEAL,
	AB_CLEARANCE,
	AB_EXPIATIO,
	AB_DUPLELIGHT,
	AB_DUPLELIGHT_MELEE,
	AB_DUPLELIGHT_MAGIC,
	AB_SILENTIUM,
	
	WL_WHITEIMPRISON = 2201,
	WL_SOULEXPANSION,
	WL_FROSTMISTY,
	WL_JACKFROST,
	WL_MARSHOFABYSS,
	WL_RECOGNIZEDSPELL,
	WL_SIENNAEXECRATE,
	WL_RADIUS,
	WL_STASIS,
	WL_DRAINLIFE,
	WL_CRIMSONROCK,
	WL_HELLINFERNO,
	WL_COMET,
	WL_CHAINLIGHTNING,
	WL_CHAINLIGHTNING_ATK,
	WL_EARTHSTRAIN,
	WL_TETRAVORTEX,
	WL_TETRAVORTEX_FIRE,
	WL_TETRAVORTEX_WATER,
	WL_TETRAVORTEX_WIND,
	WL_TETRAVORTEX_GROUND,
	WL_SUMMONFB,
	WL_SUMMONBL,
	WL_SUMMONWB,
	WL_SUMMON_ATK_FIRE,
	WL_SUMMON_ATK_WIND,
	WL_SUMMON_ATK_WATER,
	WL_SUMMON_ATK_GROUND,
	WL_SUMMONSTONE,
	WL_RELEASE,
	WL_READING_SB,
	WL_FREEZE_SP,
	
	RA_ARROWSTORM,
	RA_FEARBREEZE,
	RA_RANGERMAIN,
	RA_AIMEDBOLT,
	RA_DETONATOR,
	RA_ELECTRICSHOCKER,
	RA_CLUSTERBOMB,
	RA_WUGMASTERY,
	RA_WUGRIDER,
	RA_WUGDASH,
	RA_WUGSTRIKE,
	RA_WUGBITE,
	RA_TOOTHOFWUG,
	RA_SENSITIVEKEEN,
	RA_CAMOUFLAGE,
	RA_RESEARCHTRAP,
	RA_MAGENTATRAP,
	RA_COBALTTRAP,
	RA_MAIZETRAP,
	RA_VERDURETRAP,
	RA_FIRINGTRAP,
	RA_ICEBOUNDTRAP,
	
	NC_MADOLICENCE,
	NC_BOOSTKNUCKLE,
	NC_PILEBUNKER,
	NC_VULCANARM,
	NC_FLAMELAUNCHER,
	NC_COLDSLOWER,
	NC_ARMSCANNON,
	NC_ACCELERATION,
	NC_HOVERING,
	NC_F_SIDESLIDE,
	NC_B_SIDESLIDE,
	NC_MAINFRAME,
	NC_SELFDESTRUCTION,
	NC_SHAPESHIFT,
	NC_EMERGENCYCOOL,
	NC_INFRAREDSCAN,
	NC_ANALYZE,
	NC_MAGNETICFIELD,
	NC_NEUTRALBARRIER,
	NC_STEALTHFIELD,
	NC_REPAIR,
	NC_TRAININGAXE,
	NC_RESEARCHFE,
	NC_AXEBOOMERANG,
	NC_POWERSWING,
	NC_AXETORNADO,
	NC_SILVERSNIPER,
	NC_MAGICDECOY,
	NC_DISJOINT,
	
	SC_FATALMENACE,
	SC_REPRODUCE,
	SC_AUTOSHADOWSPELL,
	SC_SHADOWFORM,
	SC_TRIANGLESHOT,
	SC_BODYPAINT,
	SC_INVISIBILITY,
	SC_DEADLYINFECT,
	SC_ENERVATION,
	SC_GROOMY,
	SC_IGNORANCE,
	SC_LAZINESS,
	SC_UNLUCKY,
	SC_WEAKNESS,
	SC_STRIPACCESSARY,
	SC_MANHOLE,
	SC_DIMENSIONDOOR,
	SC_CHAOSPANIC,
	SC_MAELSTROM,
	SC_BLOODYLUST,
	SC_FEINTBOMB,
	
	LG_CANNONSPEAR = 2307,
	LG_BANISHINGPOINT,
	LG_TRAMPLE,
	LG_SHIELDPRESS,
	LG_REFLECTDAMAGE,
	LG_PINPOINTATTACK,
	LG_FORCEOFVANGUARD,
	LG_RAGEBURST,
	LG_SHIELDSPELL,
	LG_EXEEDBREAK,
	LG_OVERBRAND,
	LG_PRESTIGE,
	LG_BANDING,
	LG_MOONSLASHER,
	LG_RAYOFGENESIS,
	LG_PIETY,
	LG_EARTHDRIVE,
	LG_HESPERUSLIT,
	LG_INSPIRATION,
	
	SR_DRAGONCOMBO,
	SR_SKYNETBLOW,
	SR_EARTHSHAKER,
	SR_FALLENEMPIRE,
	SR_TIGERCANNON,
	SR_HELLGATE,
	SR_RAMPAGEBLASTER,
	SR_CRESCENTELBOW,
	SR_CURSEDCIRCLE,
	SR_LIGHTNINGWALK,
	SR_KNUCKLEARROW,
	SR_WINDMILL,
	SR_RAISINGDRAGON,
	SR_GENTLETOUCH,
	SR_ASSIMILATEPOWER,
	SR_POWERVELOCITY,
	SR_CRESCENTELBOW_AUTOSPELL,
	SR_GATEOFHELL,
	SR_GENTLETOUCH_QUIET,
	SR_GENTLETOUCH_CURE,
	SR_GENTLETOUCH_ENERGYGAIN,
	SR_GENTLETOUCH_CHANGE,
	SR_GENTLETOUCH_REVITALIZE,
	
	WA_SWING_DANCE = 2350,
	WA_SYMPHONY_OF_LOVER,
	WA_MOONLIT_SERENADE,
	
	MI_RUSH_WINDMILL = 2381,
	MI_ECHOSONG,
	MI_HARMONIZE,
	
	WM_LESSON = 2412,
	WM_METALICSOUND,
	WM_REVERBERATION,
	WM_REVERBERATION_MELEE,
	WM_REVERBERATION_MAGIC,
	WM_DOMINION_IMPULSE,
	WM_SEVERE_RAINSTORM,
	WM_POEMOFNETHERWORLD,
	WM_VOICEOFSIREN,
	WM_DEADHILLHERE,
	WM_LULLABY_DEEPSLEEP,
	WM_SIRCLEOFNATURE,
	WM_RANDOMIZESPELL,
	WM_GLOOMYDAY,
	WM_GREAT_ECHO,
	WM_SONG_OF_MANA,
	WM_DANCE_WITH_WUG,
	WM_SOUND_OF_DESTRUCTION,
	WM_SATURDAY_NIGHT_FEVER,
	WM_LERADS_DEW,
	WM_MELODYOFSINK,
	WM_BEYOND_OF_WARCRY,
	WM_UNLIMITED_HUMMING_VOICE,
	
	SO_FIREWALK = 2443,
	SO_ELECTRICWALK,
	SO_SPELLFIST,
	SO_EARTHGRAVE,
	SO_DIAMONDDUST,
	SO_POISON_BUSTER,
	SO_PSYCHIC_WAVE,
	SO_CLOUD_KILL,
	SO_STRIKING,
	SO_WARMER,
	SO_VACUUM_EXTREME,
	SO_VARETYR_SPEAR,
	SO_ARRULLO,
	SO_EL_CONTROL,
	SO_SUMMON_AGNI,
	SO_SUMMON_AQUA,
	SO_SUMMON_VENTUS,
	SO_SUMMON_TERA,
	SO_EL_ACTION,
	SO_EL_ANALYSIS,
	SO_EL_SYMPATHY,
	SO_EL_CURE,
	SO_FIRE_INSIGNIA,
	SO_WATER_INSIGNIA,
	SO_WIND_INSIGNIA,
	SO_EARTH_INSIGNIA,
	
	GN_TRAINING_SWORD = 2474,
	GN_REMODELING_CART,
	GN_CART_TORNADO,
	GN_CARTCANNON,
	GN_CARTBOOST,
	GN_THORNS_TRAP,
	GN_BLOOD_SUCKER,
	GN_SPORE_EXPLOSION,
	GN_WALLOFTHORN,
	GN_CRAZYWEED,
	GN_CRAZYWEED_ATK,
	GN_DEMONIC_FIRE,
	GN_FIRE_EXPANSION,
	GN_FIRE_EXPANSION_SMOKE_POWDER,
	GN_FIRE_EXPANSION_TEAR_GAS,
	GN_FIRE_EXPANSION_ACID,
	GN_HELLS_PLANT,
	GN_HELLS_PLANT_ATK,
	GN_MANDRAGORA,
	GN_SLINGITEM,
	GN_CHANGEMATERIAL,
	GN_MIX_COOKING,
	GN_MAKEBOMB,
	GN_S_PHARMACY,
	GN_SLINGITEM_RANGEMELEEATK,
	
	AB_SECRAMENT = 2515,
	WM_SEVERE_RAINSTORM_MELEE,
	SR_HOWLINGOFLION,
	SR_RIDEINLIGHTNING,
	LG_OVERBRAND_BRANDISH,
	LG_OVERBRAND_PLUSATK,
	
	ALL_ODINS_RECALL = 2533,
	RETURN_TO_ELDICASTES,
	ALL_BUYING_STORE,
	ALL_GUARDIAN_RECALL,
	ALL_ODINS_POWER,
	BEER_BOTTLE_CAP,
	NPC_ASSASSINCROSS,
	NPC_DISSONANCE,
	NPC_UGLYDANCE,
	ALL_TETANY,
	ALL_RAY_OF_PROTECTION,
	MC_CARTDECORATE,
	
	RL_GLITTERING_GREED = 2551,
	RL_RICHS_COIN,
	RL_MASS_SPIRAL,
	RL_BANISHING_BUSTER,
	RL_B_TRAP,
	RL_FLICKER,
	RL_S_STORM,
	RL_E_CHAIN,
	RL_QD_SHOT,
	RL_C_MARKER,
	RL_FIREDANCE,
	RL_H_MINE,
	RL_P_ALTER,
	RL_FALLEN_ANGEL,
	RL_R_TRIP,
	RL_D_TAIL,
	RL_FIRE_RAIN,
	RL_HEAT_BARREL,
	RL_AM_BLAST,
	RL_SLUGSHOT,
	RL_HAMMER_OF_GOD,
	RL_R_TRIP_PLUSATK,
	RL_B_FLICKER_ATK,
	RL_GLITTERING_GREED_ATK,
	
	KO_YAMIKUMO = 3001,
	KO_RIGHT,
	KO_LEFT,
	KO_JYUMONJIKIRI,
	KO_SETSUDAN,
	KO_BAKURETSU,
	KO_HAPPOKUNAI,
	KO_MUCHANAGE,
	KO_HUUMARANKA,
	KO_MAKIBISHI,
	KO_MEIKYOUSISUI,
	KO_ZANZOU,
	KO_KYOUGAKU,
	KO_JYUSATSU,
	KO_KAHU_ENTEN,
	KO_HYOUHU_HUBUKI,
	KO_KAZEHU_SEIRAN,
	KO_DOHU_KOUKAI,
	KO_KAIHOU,
	KO_ZENKAI,
	KO_GENWAKU,
	KO_IZAYOI,
	KG_KAGEHUMI,
	KG_KYOMU,
	KG_KAGEMUSYA,
	OB_ZANGETSU,
	OB_OBOROGENSOU,
	OB_OBOROGENSOU_TRANSITION_ATK,
	OB_AKAITSUKI,
	
	ECL_SNOWFLIP = 3031,
	ECL_PEONYMAMY,
	ECL_SADAGUI,
	ECL_SEQUOIADUST,
	ECLAGE_RECALL,

	GC_DARKCROW = 5001,
	RA_UNLIMIT,
	GN_ILLUSIONDOPING,
	RK_DRAGONBREATH_WATER,
	RK_LUXANIMA,
	NC_MAGMA_ERUPTION,
	WM_FRIGG_SONG,
	SO_ELEMENTAL_SHIELD,
	SR_FLASHCOMBO,
	SC_ESCAPE,
	AB_OFFERTORIUM,
	WL_TELEKINESIS_INTENSE,
	LG_KINGS_GRACE,
	ALL_FULL_THROTTLE,
	SR_FLASHCOMBO_ATK_STEP1,
	SR_FLASHCOMBO_ATK_STEP2,
	SR_FLASHCOMBO_ATK_STEP3,
	SR_FLASHCOMBO_ATK_STEP4,

	HLIF_HEAL = 8001,
	HLIF_AVOID,
	HLIF_BRAIN,
	HLIF_CHANGE,
	HAMI_CASTLE,
	HAMI_DEFENCE,
	HAMI_SKIN,
	HAMI_BLOODLUST,
	HFLI_MOON,
	HFLI_FLEET,
	HFLI_SPEED,
	HFLI_SBR44,
	HVAN_CAPRICE,
	HVAN_CHAOTIC,
	HVAN_INSTRUCT,
	HVAN_EXPLOSION,
	MUTATION_BASEJOB,
	MH_SUMMON_LEGION,
	MH_NEEDLE_OF_PARALYZE,
	MH_POISON_MIST,
	MH_PAIN_KILLER,
	MH_LIGHT_OF_REGENE,
	MH_OVERED_BOOST,
	MH_ERASER_CUTTER,
	MH_XENO_SLASHER,
	MH_SILENT_BREEZE,
	MH_STYLE_CHANGE,
	MH_SONIC_CRAW,
	MH_SILVERVEIN_RUSH,
	MH_MIDNIGHT_FRENZY,
	MH_STAHL_HORN,
	MH_GOLDENE_FERSE,
	MH_STEINWAND,
	MH_HEILIGE_STANGE,
	MH_ANGRIFFS_MODUS,
	MH_TINDER_BREAKER,
	MH_CBC,
	MH_EQC,
	MH_MAGMA_FLOW,
	MH_GRANITIC_ARMOR,
	MH_LAVA_SLIDE,
	MH_PYROCLASTIC,
	MH_VOLCANIC_ASH,
	
	MS_BASH = 8201,
	MS_MAGNUM,
	MS_BOWLINGBASH,
	MS_PARRYING,
	MS_REFLECTSHIELD,
	MS_BERSERK,
	MA_DOUBLE,
	MA_SHOWER,
	MA_SKIDTRAP,
	MA_LANDMINE,
	MA_SANDMAN,
	MA_FREEZINGTRAP,
	MA_REMOVETRAP,
	MA_CHARGEARROW,
	MA_SHARPSHOOTING,
	ML_PIERCE,
	ML_BRANDISH,
	ML_SPIRALPIERCE,
	ML_DEFENDER,
	ML_AUTOGUARD,
	ML_DEVOTION,
	MER_MAGNIFICAT,
	MER_QUICKEN,
	MER_SIGHT,
	MER_CRASH,
	MER_REGAIN,
	MER_TENDER,
	MER_BENEDICTION,
	MER_RECUPERATE,
	MER_MENTALCURE,
	MER_COMPRESS,
	MER_PROVOKE,
	MER_AUTOBERSERK,
	MER_DECAGI,
	MER_SCAPEGOAT,
	MER_LEXDIVINA,
	MER_ESTIMATION,
	MER_KYRIE,
	MER_BLESSING,
	MER_INCAGI,
	
	EL_CIRCLE_OF_FIRE = 8401,
	EL_FIRE_CLOAK,
	EL_FIRE_MANTLE,
	EL_WATER_SCREEN,
	EL_WATER_DROP,
	EL_WATER_BARRIER,
	EL_WIND_STEP,
	EL_WIND_CURTAIN,
	EL_ZEPHYR,
	EL_SOLID_SKIN,
	EL_STONE_SHIELD,
	EL_POWER_OF_GAIA,
	EL_PYROTECHNIC,
	EL_HEATER,
	EL_TROPIC,
	EL_AQUAPLAY,
	EL_COOLER,
	EL_CHILLY_AIR,
	EL_GUST,
	EL_BLAST,
	EL_WILD_STORM,
	EL_PETROLOGY,
	EL_CURSED_SOIL,
	EL_UPHEAVAL,
	EL_FIRE_ARROW,
	EL_FIRE_BOMB,
	EL_FIRE_BOMB_ATK,
	EL_FIRE_WAVE,
	EL_FIRE_WAVE_ATK,
	EL_ICE_NEEDLE,
	EL_WATER_SCREW,
	EL_WATER_SCREW_ATK,
	EL_TIDAL_WEAPON,
	EL_WIND_SLASH,
	EL_HURRICANE,
	EL_HURRICANE_ATK,
	EL_TYPOON_MIS,
	EL_TYPOON_MIS_ATK,
	EL_STONE_HAMMER,
	EL_ROCK_CRUSHER,
	EL_ROCK_CRUSHER_ATK,
	EL_STONE_RAIN,
};

/// The client view ids for land skills.
enum {
	UNT_SAFETYWALL = 0x7e,
	UNT_FIREWALL,
	UNT_WARP_WAITING,
	UNT_WARP_ACTIVE,
	UNT_BENEDICTIO, //TODO
	UNT_SANCTUARY,
	UNT_MAGNUS,
	UNT_PNEUMA,
	UNT_DUMMYSKILL, //These show no effect on the client
	UNT_FIREPILLAR_WAITING,
	UNT_FIREPILLAR_ACTIVE,
	UNT_HIDDEN_TRAP, //TODO
	UNT_TRAP, //TODO
	UNT_HIDDEN_WARP_NPC, //TODO
	UNT_USED_TRAPS,
	UNT_ICEWALL,
	UNT_QUAGMIRE,
	UNT_BLASTMINE,
	UNT_SKIDTRAP,
	UNT_ANKLESNARE,
	UNT_VENOMDUST,
	UNT_LANDMINE,
	UNT_SHOCKWAVE,
	UNT_SANDMAN,
	UNT_FLASHER,
	UNT_FREEZINGTRAP,
	UNT_CLAYMORETRAP,
	UNT_TALKIEBOX,
	UNT_VOLCANO,
	UNT_DELUGE,
	UNT_VIOLENTGALE,
	UNT_LANDPROTECTOR,
	UNT_LULLABY,
	UNT_RICHMANKIM,
	UNT_ETERNALCHAOS,
	UNT_DRUMBATTLEFIELD,
	UNT_RINGNIBELUNGEN,
	UNT_ROKISWEIL,
	UNT_INTOABYSS,
	UNT_SIEGFRIED,
	UNT_DISSONANCE,
	UNT_WHISTLE,
	UNT_ASSASSINCROSS,
	UNT_POEMBRAGI,
	UNT_APPLEIDUN,
	UNT_UGLYDANCE,
	UNT_HUMMING,
	UNT_DONTFORGETME,
	UNT_FORTUNEKISS,
	UNT_SERVICEFORYOU,
	UNT_GRAFFITI,
	UNT_DEMONSTRATION,
	UNT_CALLFAMILY,
	UNT_GOSPEL,
	UNT_BASILICA,
	UNT_MOONLIT,
	UNT_FOGWALL,
	UNT_SPIDERWEB,
	UNT_GRAVITATION,
	UNT_HERMODE,
	UNT_KAENSIN, //TODO
	UNT_SUITON,
	UNT_TATAMIGAESHI,
	UNT_KAEN,
	UNT_GROUNDDRIFT_WIND,
	UNT_GROUNDDRIFT_DARK,
	UNT_GROUNDDRIFT_POISON,
	UNT_GROUNDDRIFT_WATER,
	UNT_GROUNDDRIFT_FIRE,
	UNT_DEATHWAVE, //TODO
	UNT_WATERATTACK, //TODO
	UNT_WINDATTACK, //TODO
	UNT_EARTHQUAKE, //TODO
	UNT_EVILLAND,
	UNT_DARK_RUNNER, //TODO
	UNT_DARK_TRANSFER, //TODO
	UNT_EPICLESIS,
	UNT_EARTHSTRAIN,
	UNT_MANHOLE,
	UNT_DIMENSIONDOOR,
	UNT_CHAOSPANIC,
	UNT_MAELSTROM,
	UNT_BLOODYLUST,
	UNT_FEINTBOMB,
	UNT_MAGENTATRAP,
	UNT_COBALTTRAP,
	UNT_MAIZETRAP,
	UNT_VERDURETRAP,
	UNT_FIRINGTRAP,
	UNT_ICEBOUNDTRAP,
	UNT_ELECTRICSHOCKER,
	UNT_CLUSTERBOMB,
	UNT_REVERBERATION,
	UNT_SEVERE_RAINSTORM,
	UNT_FIREWALK,
	UNT_ELECTRICWALK,
	UNT_NETHERWORLD,
	UNT_PSYCHIC_WAVE,
	UNT_CLOUD_KILL,
	UNT_POISONSMOKE,
	UNT_NEUTRALBARRIER,
	UNT_STEALTHFIELD,
	UNT_WARMER,
	UNT_THORNS_TRAP,
	UNT_WALLOFTHORN,
	UNT_DEMONIC_FIRE,
	UNT_FIRE_EXPANSION_SMOKE_POWDER,
	UNT_FIRE_EXPANSION_TEAR_GAS,
	UNT_HELLS_PLANT,
	UNT_VACUUM_EXTREME,
	UNT_BANDING,
	UNT_FIRE_MANTLE,
	UNT_WATER_BARRIER,
	UNT_ZEPHYR,
	UNT_POWER_OF_GAIA,
	UNT_FIRE_INSIGNIA,
	UNT_WATER_INSIGNIA,
	UNT_WIND_INSIGNIA,
	UNT_EARTH_INSIGNIA,
	UNT_POISON_MIST,
	UNT_LAVA_SLIDE,
	UNT_VOLCANIC_ASH,
	UNT_ZENKAI_WATER,
	UNT_ZENKAI_LAND,
	UNT_ZENKAI_FIRE,
	UNT_ZENKAI_WIND,
	UNT_MAKIBISHI,
	UNT_VENOMFOG,
	UNT_ICEMINE, 
 	UNT_FLAMECROSS,
 	UNT_HELLBURNING,
 	UNT_MAGMA_ERUPTION,
	UNT_KINGS_GRACE,
	UNT_GLITTERING_GREED,
	UNT_B_TRAP,
	UNT_FIRE_RAIN,
	
	/**
	 * Guild Auras
	 **/
	UNT_GD_LEADERSHIP = 0xc1,
	UNT_GD_GLORYWOUNDS = 0xc2,
	UNT_GD_SOULCOLD = 0xc3,
	UNT_GD_HAWKEYES = 0xc4,
	
	UNT_MAX = 0x190
};

/**
 * Structures
 **/

struct skill_condition {
	int weapon,ammo,ammo_qty,hp,sp,zeny,spiritball,mhp,state;
	int itemid[MAX_SKILL_ITEM_REQUIRE],amount[MAX_SKILL_ITEM_REQUIRE];
};

// Database skills
struct s_skill_db {
	unsigned short nameid;
	char name[MAX_SKILL_NAME_LENGTH];
	char desc[40];
	int range[MAX_SKILL_LEVEL],hit,inf,element[MAX_SKILL_LEVEL],nk,splash[MAX_SKILL_LEVEL],max;
	int num[MAX_SKILL_LEVEL];
	int cast[MAX_SKILL_LEVEL],walkdelay[MAX_SKILL_LEVEL],delay[MAX_SKILL_LEVEL];
#ifdef RENEWAL_CAST
	int fixed_cast[MAX_SKILL_LEVEL];
#endif
	int upkeep_time[MAX_SKILL_LEVEL],upkeep_time2[MAX_SKILL_LEVEL],cooldown[MAX_SKILL_LEVEL];
	int castcancel,cast_def_rate;
	int inf2,maxcount[MAX_SKILL_LEVEL],skill_type;
	int blewcount[MAX_SKILL_LEVEL];
	int hp[MAX_SKILL_LEVEL],sp[MAX_SKILL_LEVEL],mhp[MAX_SKILL_LEVEL],hp_rate[MAX_SKILL_LEVEL],sp_rate[MAX_SKILL_LEVEL],zeny[MAX_SKILL_LEVEL];
	int weapon,ammo,ammo_qty[MAX_SKILL_LEVEL],state,spiritball[MAX_SKILL_LEVEL];
	int itemid[MAX_SKILL_ITEM_REQUIRE],amount[MAX_SKILL_ITEM_REQUIRE];
	int castnodex[MAX_SKILL_LEVEL], delaynodex[MAX_SKILL_LEVEL];
	int nocast;
	int unit_id[2];
	int unit_layout_type[MAX_SKILL_LEVEL];
	int unit_range[MAX_SKILL_LEVEL];
	int unit_interval;
	int unit_target;
	int unit_flag;
};

struct s_skill_unit_layout {
	int count;
	int dx[MAX_SKILL_UNIT_COUNT];
	int dy[MAX_SKILL_UNIT_COUNT];
};

struct skill_timerskill {
	int timer;
	int src_id;
	int target_id;
	int map;
	short x,y;
	uint16 skill_id,skill_lv;
	int type; // a BF_ type (NOTE: some places use this as general-purpose storage...)
	int flag;
};

struct skill_unit_group {
	int src_id;
	int party_id;
	int guild_id;
	int bg_id;
	int map;
	int target_flag; //Holds BCT_* flag for battle_check_target
	int bl_flag;	//Holds BL_* flag for map_foreachin* functions
	int64 tick;
	int limit,interval;
	
	uint16 skill_id,skill_lv;
	int val1,val2,val3;
	char *valstr;
	int unit_id;
	int group_id;
	int unit_count,alive_count;
	int item_id; //store item used.
	struct skill_unit *unit;
	struct {
		unsigned ammo_consume : 1;
		unsigned song_dance : 2; //0x1 Song/Dance, 0x2 Ensemble
		unsigned guildaura : 1;
	} state;
};

struct skill_unit {
	struct block_list bl;
	
	struct skill_unit_group *group;
	
	int limit;
	int val1,val2;
	short alive,range;
};

struct skill_unit_group_tickset {
	int64 tick;
	int id;
};

// Create Database item
struct s_skill_produce_db {
	int nameid, trigger;
	int req_skill,req_skill_lv,itemlv;
	int mat_id[MAX_PRODUCE_RESOURCE],mat_amount[MAX_PRODUCE_RESOURCE];
};

// Creating database arrow
struct s_skill_arrow_db {
	int nameid, trigger;
	int cre_id[MAX_ARROW_RESOURCE],cre_amount[MAX_ARROW_RESOURCE];
};

// Abracadabra database
struct s_skill_abra_db {
	uint16 skill_id;
	int req_lv;
	int per;
};

//GCross magic mushroom database
struct s_skill_magicmushroom_db {
	uint16 skill_id;
};

struct skill_cd_entry {
	int duration;//milliseconds
#if PACKETVER >= 20120604
	int total;/* used for display on newer clients */
#endif
	short skidx;//the skill index entries belong to
	int64 started;/* gettick() of when it started, used vs duration to measure how much left upon logout */
	int timer;/* timer id */
	uint16 skill_id;//skill id
};

/**
 * Skill Cool Down Delay Saving
 * Struct skill_cd is not a member of struct map_session_data
 * to keep cooldowns in memory between player log-ins.
 * All cooldowns are reset when server is restarted.
 **/
struct skill_cd {
	struct skill_cd_entry *entry[MAX_SKILL_TREE];
	unsigned char cursor;
};

/**
 * Skill Unit Persistency during endack routes (mostly for songs see bugreport:4574)
 **/
struct skill_unit_save {
	uint16 skill_id, skill_lv;
};

struct s_skill_improvise_db {
	uint16 skill_id;
	short per;//1-10000
};

struct s_skill_changematerial_db {
	int itemid;
	short rate;
	int qty[5];
	short qty_rate[5];
};

struct s_skill_spellbook_db {
	int nameid;
	uint16 skill_id;
	int point;
};

typedef int (*SkillFunc)(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 tick, int flag);

/**
 * Skill.c Interface
 **/
struct skill_interface {
	int (*init) (bool minimal);
	int (*final) (void);
	void (*reload) (void);
	void (*read_db) (bool minimal);
	/* */
	DBMap* cd_db; // char_id -> struct skill_cd
	DBMap* name2id_db;
	DBMap* unit_db; // int id -> struct skill_unit*
	DBMap* usave_db; // char_id -> struct skill_unit_save
	DBMap* group_db;// int group_id -> struct skill_unit_group*
	/* */
	struct eri *unit_ers; //For handling skill_unit's [Skotlex]
	struct eri *timer_ers; //For handling skill_timerskills [Skotlex]
	struct eri *cd_ers; // ERS Storage for skill cool down managers [Ind/Hercules]
	struct eri *cd_entry_ers; // ERS Storage for skill cool down entries [Ind/Hercules]
	/* */
	struct s_skill_db db[MAX_SKILL_DB];
	struct s_skill_produce_db produce_db[MAX_SKILL_PRODUCE_DB];
	struct s_skill_arrow_db arrow_db[MAX_SKILL_ARROW_DB];
	struct s_skill_abra_db abra_db[MAX_SKILL_ABRA_DB];
	struct s_skill_magicmushroom_db magicmushroom_db[MAX_SKILL_MAGICMUSHROOM_DB];
	struct s_skill_improvise_db improvise_db[MAX_SKILL_IMPROVISE_DB];
	struct s_skill_changematerial_db changematerial_db[MAX_SKILL_PRODUCE_DB];
	struct s_skill_spellbook_db spellbook_db[MAX_SKILL_SPELLBOOK_DB];
	bool reproduce_db[MAX_SKILL_DB];
	struct s_skill_unit_layout unit_layout[MAX_SKILL_UNIT_LAYOUT];
	/* */
	int enchant_eff[5];
	int deluge_eff[5];
	int firewall_unit_pos;
	int icewall_unit_pos;
	int earthstrain_unit_pos;
	int area_temp[8];
	int unit_temp[20];  // temporary storage for tracking skill unit skill ids as players move in/out of them
	int unit_group_newid;
	/* accesssors */
	int	(*get_index) ( uint16 skill_id );
	int	(*get_type) ( uint16 skill_id );
	int	(*get_hit) ( uint16 skill_id );
	int	(*get_inf) ( uint16 skill_id );
	int	(*get_ele) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_nk) ( uint16 skill_id );
	int	(*get_max) ( uint16 skill_id );
	int	(*get_range) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_range2) (struct block_list *bl, uint16 skill_id, uint16 skill_lv);
	int	(*get_splash) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_hp) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_mhp) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_sp) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_state) (uint16 skill_id);
	int	(*get_spiritball) (uint16 skill_id, uint16 skill_lv);
	int	(*get_zeny) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_num) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_cast) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_delay) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_walkdelay) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_time) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_time2) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_castnodex) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_delaynodex) ( uint16 skill_id ,uint16 skill_lv );
	int	(*get_castdef) ( uint16 skill_id );
	int	(*get_weapontype) ( uint16 skill_id );
	int	(*get_ammotype) ( uint16 skill_id );
	int	(*get_ammo_qty) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_unit_id) (uint16 skill_id,int flag);
	int	(*get_inf2) ( uint16 skill_id );
	int	(*get_castcancel) ( uint16 skill_id );
	int	(*get_maxcount) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_blewcount) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_unit_flag) ( uint16 skill_id );
	int	(*get_unit_target) ( uint16 skill_id );
	int	(*get_unit_interval) ( uint16 skill_id );
	int	(*get_unit_bl_target) ( uint16 skill_id );
	int	(*get_unit_layout_type) ( uint16 skill_id ,uint16 skill_lv );
	int	(*get_unit_range) ( uint16 skill_id, uint16 skill_lv );
	int	(*get_cooldown) ( uint16 skill_id, uint16 skill_lv );
	int	(*tree_get_max) ( uint16 skill_id, int b_class );
	const char*	(*get_name) ( uint16 skill_id );
	const char*	(*get_desc) ( uint16 skill_id );
	/* check */
	void (*chk) (uint16* skill_id);
	/* whether its CAST_GROUND, CAST_DAMAGE or CAST_NODAMAGE */
	int (*get_casttype) (uint16 skill_id);
	int (*get_casttype2) (uint16 index);
	bool (*is_combo) (int skill_id);
	int (*name2id) (const char* name);
	int (*isammotype) (struct map_session_data *sd, int skill_id);
	int (*castend_id) (int tid, int64 tick, int id, intptr_t data);
	int (*castend_pos) (int tid, int64 tick, int id, intptr_t data);
	int (*castend_map) ( struct map_session_data *sd,uint16 skill_id, const char *mapname);
	int (*cleartimerskill) (struct block_list *src);
	int (*addtimerskill) (struct block_list *src, int64 tick, int target, int x, int y, uint16 skill_id, uint16 skill_lv, int type, int flag);
	int (*additional_effect) (struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, int dmg_lv, int64 tick);
	int (*counter_additional_effect) (struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, int64 tick);
	int (*blown) (struct block_list* src, struct block_list* target, int count, int8 dir, int flag);
	int (*break_equip) (struct block_list *bl, unsigned short where, int rate, int flag);
	int (*strip_equip) (struct block_list *bl, unsigned short where, int rate, int lv, int time);
	struct skill_unit_group* (*id2group) (int group_id);
	struct skill_unit_group *(*unitsetting) (struct block_list* src, uint16 skill_id, uint16 skill_lv, short x, short y, int flag);
	struct skill_unit *(*initunit) (struct skill_unit_group *group, int idx, int x, int y, int val1, int val2);
	int (*delunit) (struct skill_unit *su);
	struct skill_unit_group *(*init_unitgroup) (struct block_list* src, int count, uint16 skill_id, uint16 skill_lv, int unit_id, int limit, int interval);
	int (*del_unitgroup) (struct skill_unit_group *group, const char* file, int line, const char* func);
	int (*clear_unitgroup) (struct block_list *src);
	int (*clear_group) (struct block_list *bl, int flag);
	int (*unit_onplace) (struct skill_unit *src, struct block_list *bl, int64 tick);
	int (*unit_ondamaged) (struct skill_unit *src, struct block_list *bl, int64 damage, int64 tick);
	int (*cast_fix) ( struct block_list *bl, uint16 skill_id, uint16 skill_lv);
	int (*cast_fix_sc) ( struct block_list *bl, int time);
	int (*vf_cast_fix) ( struct block_list *bl, double time, uint16 skill_id, uint16 skill_lv);
	int (*delay_fix) ( struct block_list *bl, uint16 skill_id, uint16 skill_lv);
	int (*check_condition_castbegin) (struct map_session_data *sd, uint16 skill_id, uint16 skill_lv);
	int (*check_condition_castend) (struct map_session_data *sd, uint16 skill_id, uint16 skill_lv);
	int (*consume_requirement) (struct map_session_data *sd, uint16 skill_id, uint16 skill_lv, short type);
	struct skill_condition (*get_requirement) (struct map_session_data *sd, uint16 skill_id, uint16 skill_lv);
	int (*check_pc_partner) (struct map_session_data *sd, uint16 skill_id, uint16* skill_lv, int range, int cast_flag);
	int (*unit_move) (struct block_list *bl, int64 tick, int flag);
	int (*unit_onleft) (uint16 skill_id, struct block_list *bl, int64 tick);
	int (*unit_onout) (struct skill_unit *src, struct block_list *bl, int64 tick);
	int (*unit_move_unit_group) ( struct skill_unit_group *group, int16 m,int16 dx,int16 dy);
	int (*sit) (struct map_session_data *sd, int type);
	void (*brandishspear) (struct block_list* src, struct block_list* bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag);
	void (*repairweapon) (struct map_session_data *sd, int idx);
	void (*identify) (struct map_session_data *sd,int idx);
	void (*weaponrefine) (struct map_session_data *sd,int idx);
	int (*autospell) (struct map_session_data *md,uint16 skill_id);
	int (*calc_heal) (struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, bool heal);
	bool (*check_cloaking) (struct block_list *bl, struct status_change_entry *sce);
	int (*enchant_elemental_end) (struct block_list *bl, int type);
	int (*not_ok) (uint16 skill_id, struct map_session_data *sd);
	int (*not_ok_hom) (uint16 skill_id, struct homun_data *hd);
	int (*not_ok_mercenary) (uint16 skill_id, struct mercenary_data *md);
	int (*chastle_mob_changetarget) (struct block_list *bl,va_list ap);
	int (*can_produce_mix) ( struct map_session_data *sd, int nameid, int trigger, int qty);
	int (*produce_mix) ( struct map_session_data *sd, uint16 skill_id, int nameid, int slot1, int slot2, int slot3, int qty );
	int (*arrow_create) ( struct map_session_data *sd,int nameid);
	int (*castend_nodamage_id) (struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag);
	int (*castend_damage_id) (struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick,int flag);
	int (*castend_pos2) (struct block_list *src, int x, int y, uint16 skill_id, uint16 skill_lv, int64 tick, int flag);
	int (*blockpc_start) (struct map_session_data *sd, uint16 skill_id, int tick);
	int (*blockhomun_start) (struct homun_data *hd, uint16 skill_id, int tick);
	int (*blockmerc_start) (struct mercenary_data *md, uint16 skill_id, int tick);
	int (*attack) (int attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag);
	int (*attack_area) (struct block_list *bl,va_list ap);
	int (*area_sub) (struct block_list *bl, va_list ap);
	int (*area_sub_count) (struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 tick, int flag);
	int (*check_unit_range) (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv);
	int (*check_unit_range_sub) (struct block_list *bl, va_list ap);
	int (*check_unit_range2) (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv);
	int (*check_unit_range2_sub) (struct block_list *bl, va_list ap);
	void (*toggle_magicpower) (struct block_list *bl, uint16 skill_id);
	int (*magic_reflect) (struct block_list* src, struct block_list* bl, int type);
	int (*onskillusage) (struct map_session_data *sd, struct block_list *bl, uint16 skill_id, int64 tick);
	int (*cell_overlap) (struct block_list *bl, va_list ap);
	int (*timerskill) (int tid, int64 tick, int id, intptr_t data);
	int (*trap_splash) (struct block_list *bl, va_list ap);
	int (*check_condition_mercenary) (struct block_list *bl, int skill_id, int lv, int type);
	struct skill_unit_group *(*locate_element_field) (struct block_list *bl);
	int (*graffitiremover) (struct block_list *bl, va_list ap);
	int (*activate_reverberation) ( struct block_list *bl, va_list ap);
	int (*dance_overlap_sub) (struct block_list* bl, va_list ap);
	int (*dance_overlap) (struct skill_unit* su, int flag);
	struct s_skill_unit_layout *(*get_unit_layout) (uint16 skill_id, uint16 skill_lv, struct block_list* src, int x, int y);
	int (*frostjoke_scream) (struct block_list *bl, va_list ap);
	int (*greed) (struct block_list *bl, va_list ap);
	int (*destroy_trap) ( struct block_list *bl, va_list ap );
	int (*icewall_block) (struct block_list *bl,va_list ap);
	struct skill_unit_group_tickset *(*unitgrouptickset_search) (struct block_list *bl, struct skill_unit_group *group, int64 tick);
	bool (*dance_switch) (struct skill_unit* su, int flag);
	int (*check_condition_char_sub) (struct block_list *bl, va_list ap);
	int (*check_condition_mob_master_sub) (struct block_list *bl, va_list ap);
	void (*brandishspear_first) (struct square *tc, uint8 dir, int16 x, int16 y);
	void (*brandishspear_dir) (struct square* tc, uint8 dir, int are);
	int (*get_fixed_cast) ( uint16 skill_id ,uint16 skill_lv );
	int (*sit_count) (struct block_list *bl, va_list ap);
	int (*sit_in) (struct block_list *bl, va_list ap);
	int (*sit_out) (struct block_list *bl, va_list ap);
	void (*unitsetmapcell) (struct skill_unit *src, uint16 skill_id, uint16 skill_lv, cell_t cell, bool flag);
	int (*unit_onplace_timer) (struct skill_unit *src, struct block_list *bl, int64 tick);
	int (*unit_effect) (struct block_list* bl, va_list ap);
	int (*unit_timer_sub_onplace) (struct block_list* bl, va_list ap);
	int (*unit_move_sub) (struct block_list* bl, va_list ap);
	int (*blockpc_end) (int tid, int64 tick, int id, intptr_t data);
	int (*blockhomun_end) (int tid, int64 tick, int id, intptr_t data);
	int (*blockmerc_end) (int tid, int64 tick, int id, intptr_t data);
	int (*split_atoi) (char *str, int *val);
	int (*unit_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*unit_timer_sub) (DBKey key, DBData *data, va_list ap);
	void (*init_unit_layout) (void);
	bool (*parse_row_skilldb) (char* split[], int columns, int current);
	bool (*parse_row_requiredb) (char* split[], int columns, int current);
	bool (*parse_row_castdb) (char* split[], int columns, int current);
	bool (*parse_row_castnodexdb) (char* split[], int columns, int current);
	bool (*parse_row_unitdb) (char* split[], int columns, int current);
	bool (*parse_row_producedb) (char* split[], int columns, int current);
	bool (*parse_row_createarrowdb) (char* split[], int columns, int current);
	bool (*parse_row_abradb) (char* split[], int columns, int current);
	bool (*parse_row_spellbookdb) (char* split[], int columns, int current);
	bool (*parse_row_magicmushroomdb) (char* split[], int column, int current);
	bool (*parse_row_reproducedb) (char* split[], int column, int current);
	bool (*parse_row_improvisedb) (char* split[], int columns, int current);
	bool (*parse_row_changematerialdb) (char* split[], int columns, int current);
	/* save new unit skill */
	void (*usave_add) (struct map_session_data * sd, uint16 skill_id, uint16 skill_lv);
	/* trigger saved unit skills */
	void (*usave_trigger) (struct map_session_data *sd);
	/* load all stored skill cool downs */
	void (*cooldown_load) (struct map_session_data * sd);
	/* run spellbook of nameid id */
	int (*spellbook) (struct map_session_data *sd, int nameid);
	/* */
	int (*block_check) (struct block_list *bl, enum sc_type type, uint16 skill_id);
	int (*detonator) (struct block_list *bl, va_list ap);
	bool (*check_camouflage) (struct block_list *bl, struct status_change_entry *sce);
	int (*magicdecoy) (struct map_session_data *sd, int nameid);
	int (*poisoningweapon) ( struct map_session_data *sd, int nameid);
	int (*select_menu) (struct map_session_data *sd,uint16 skill_id);
	int (*elementalanalysis) (struct map_session_data *sd, int n, uint16 skill_lv, unsigned short *item_list);
	int (*changematerial) (struct map_session_data *sd, int n, unsigned short *item_list);
	int (*get_elemental_type) (uint16 skill_id, uint16 skill_lv);
	void (*cooldown_save) (struct map_session_data * sd);
	int (*get_new_group_id) (void);
	bool (*check_shadowform) (struct block_list *bl, int64 damage, int hit);
};

struct skill_interface *skill;

void skill_defaults(void);

#endif /* _MAP_SKILL_H_ */
