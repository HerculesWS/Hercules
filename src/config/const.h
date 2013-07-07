// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams
#ifndef _H_CONSTANTS_
#define _H_CONSTANTS_

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

/* pointer size fix which fixes several gcc warnings */
#ifdef __64BIT__
	#define __64BPTRSIZE(y) (intptr)y
#else
	#define __64BPTRSIZE(y) y
#endif

/* ATCMD_FUNC(mobinfo) HIT and FLEE calculations */
#ifdef RENEWAL
	#define MOB_FLEE(mob) ( mob->lv + mob->status.agi + 100 )
	#define MOB_HIT(mob)  ( mob->lv + mob->status.dex + 150 )
	#define RE_SKILL_REDUCTION(){ \
			wd.damage = battle->calc_elefix(src, target, skill_id, skill_lv, battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage, 0, wd.flag), nk, n_ele, s_ele, s_ele_, false, flag.arrow); \
			if( flag.lh ) \
				wd.damage2 = battle->calc_elefix(src, target, skill_id, skill_lv, battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage2, 1, wd.flag), nk, n_ele, s_ele, s_ele_, true, flag.arrow); \
		}
#else
	#define MOB_FLEE(mob) ( mob->lv + mob->status.agi )
	#define MOB_HIT(mob)  ( mob->lv + mob->status.dex )
#endif

/* Renewal's dmg level modifier, used as a macro for a easy way to turn off. */
#ifdef RENEWAL_LVDMG
	#define RE_LVL_DMOD(val) \
		if( iStatus->get_lv(src) > 100 && val > 0 ) \
			skillratio = skillratio * iStatus->get_lv(src) / val;
	#define RE_LVL_MDMOD(val) \
		if( iStatus->get_lv(src) > 100 && val > 0) \
			md.damage = md.damage * iStatus->get_lv(src) / val;
	/* ranger traps special */
	#define RE_LVL_TMDMOD() \
		if( iStatus->get_lv(src) > 100 ) \
			md.damage = md.damage * 150 / 100 + md.damage * iStatus->get_lv(src) / 100;
#else
	#define RE_LVL_DMOD(val) 
	#define RE_LVL_MDMOD(val)
	#define RE_LVL_TMDMOD()
#endif

/* Feb 1st 2012 */
#if PACKETVER >= 20120201
	#define NEW_CARTS
	#define MAX_CARTS 9
#else
	#define MAX_CARTS 5
#endif

// Renewal variable cast time reduction
#ifdef RENEWAL_CAST
	#define VARCAST_REDUCTION(val){ \
		if( (varcast_r += val) != 0 && varcast_r >= 0 ) \
			time = time * (1 - (float)min(val, 100) / 100); \
	}
#endif

/* console_input doesn't go well with minicore */
#ifdef MINICORE
	#undef CONSOLE_INPUT
#endif

#ifdef RENEWAL
	#define ITEMDB_SQL_COLUMNS 24
#else
	#define ITEMDB_SQL_COLUMNS 22
#endif


/**
 * End of File
 **/
#endif /* _H_CONSTANTS_ */
