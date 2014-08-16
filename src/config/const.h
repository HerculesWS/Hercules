// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CONFIG_CONSTANTS_H
#define CONFIG_CONSTANTS_H

/**
 * Hercules configuration file (http://hercules.ws)
 * For detailed guidance on these check http://hercules.ws/wiki/SRC/config/
 **/

/**
 * @INFO: This file holds constants that aims at making code smoother and more efficient
 */

/**
 * "Sane Checks" to save you from compiling with cool bugs
 **/
#if SECURE_NPCTIMEOUT_INTERVAL <= 0
	#error SECURE_NPCTIMEOUT_INTERVAL should be at least 1 (1s)
#endif
#if NPC_SECURE_TIMEOUT_INPUT < 0
	#error NPC_SECURE_TIMEOUT_INPUT cannot be lower than 0
#endif
#if NPC_SECURE_TIMEOUT_MENU < 0
	#error NPC_SECURE_TIMEOUT_MENU cannot be lower than 0
#endif
#if NPC_SECURE_TIMEOUT_NEXT < 0
	#error NPC_SECURE_TIMEOUT_NEXT cannot be lower than 0
#endif

/**
 * Path within the /db folder to (non-)renewal specific db files
 **/
#ifdef RENEWAL
	#define DBPATH "re/"
#else
	#define DBPATH "pre-re/"
#endif

/**
 * DefType
 **/
#ifdef RENEWAL
	typedef short defType;
	#define DEFTYPE_MIN SHRT_MIN
	#define DEFTYPE_MAX SHRT_MAX
#else
	typedef signed char defType;
	#define DEFTYPE_MIN CHAR_MIN
	#define DEFTYPE_MAX CHAR_MAX
#endif

/* ATCMD_FUNC(mobinfo) HIT and FLEE calculations */
#ifdef RENEWAL
	#define MOB_FLEE(mobdata) ( (mobdata)->lv + (mobdata)->status.agi + 100 )
	#define MOB_HIT(mobdata)  ( (mobdata)->lv + (mobdata)->status.dex + 150 )
	#define RE_SKILL_REDUCTION() do { \
		wd.damage = battle->calc_elefix(src, target, skill_id, skill_lv, battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage, 0, wd.flag), nk, n_ele, s_ele, s_ele_, false, flag.arrow); \
		if( flag.lh ) \
			wd.damage2 = battle->calc_elefix(src, target, skill_id, skill_lv, battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage2, 1, wd.flag), nk, n_ele, s_ele, s_ele_, true, flag.arrow); \
	} while(0)
#else
	#define MOB_FLEE(mobdata) ( (mobdata)->lv + (mobdata)->status.agi )
	#define MOB_HIT(mobdata)  ( (mobdata)->lv + (mobdata)->status.dex )
#endif

/* Renewal's dmg level modifier, used as a macro for a easy way to turn off. */
#ifdef RENEWAL_LVDMG
	#define RE_LVL_DMOD(val) do { \
		if( status->get_lv(src) > 100 && (val) > 0 ) \
			skillratio = skillratio * status->get_lv(src) / (val); \
	} while(0)
	#define RE_LVL_MDMOD(val) do { \
		if( status->get_lv(src) > 100 && (val) > 0) \
			md.damage = md.damage * status->get_lv(src) / (val); \
	} while(0)
	/* ranger traps special */
	#define RE_LVL_TMDMOD() do { \
		if( status->get_lv(src) > 100 ) \
			md.damage = md.damage * 150 / 100 + md.damage * status->get_lv(src) / 100; \
	} while(0)
#else
	#define RE_LVL_DMOD(val)
	#define RE_LVL_MDMOD(val)
	#define RE_LVL_TMDMOD()
#endif

// Renewal variable cast time reduction
#ifdef RENEWAL_CAST
	#define VARCAST_REDUCTION(val) do { \
		if( (varcast_r += (val)) != 0 && varcast_r >= 0 ) \
			time = time * (1 - (float)min((val), 100) / 100); \
	} while(0)
#endif

/* console_input doesn't go well with minicore */
#ifdef MINICORE
	#undef CONSOLE_INPUT
#endif

/**
 * End of File
 **/
#endif /* CONFIG_CONSTANTS_H */
