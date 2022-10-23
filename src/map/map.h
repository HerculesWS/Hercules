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
#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "map/atcommand.h"
#include "common/hercules.h"
#include "common/core.h" // CORE_ST_LAST
#include "common/db.h"
#include "common/mapindex.h"
#include "common/mmo.h"
#include "map/unitdefines.h"  // enum unit_dir

#include <stdio.h>
#include <stdarg.h>

/* Forward Declarations */
struct Sql; // common/sql.h
struct config_t; // common/conf.h
struct mob_data;
struct npc_data;
struct channel_data;
struct hplugin_data_store;

#if (defined(__GNUC__) || defined(MINGW)) && !defined(__clang__) && !defined(__ARM_ARCH)
#define GUARD_MAP_LOCK_CONCAT(a, b) GUARD_MAP_LOCK_CONCAT_INNER(a, b)
#define GUARD_MAP_LOCK_CONCAT_INNER(a, b) a ## b
#define GUARD_MAP_LOCK GUARD_MAP_LOCK0(__FILE__, __func__, __COUNTER__, __LINE__)
#define GUARD_MAP_LOCK0(file, func, lineStr, line) \
	void GUARD_MAP_LOCK_CONCAT(map_lock_check, lineStr) (int *lock_count) \
	{ \
		map->lock_check(file, func, line, *lock_count); \
	} \
	int GUARD_MAP_LOCK_CONCAT(current_map_lock_, lineStr) __attribute__((__cleanup__(GUARD_MAP_LOCK_CONCAT(map_lock_check, lineStr)))) = map->block_free_lock;
#else  // defined(__GNUC__) || defined(MINGW)
#define GUARD_MAP_LOCK
#endif  // defined(__GNUC__) || defined(MINGW)

enum E_MAPSERVER_ST {
	MAPSERVER_ST_RUNNING = CORE_ST_LAST,
	MAPSERVER_ST_SHUTDOWN,
	MAPSERVER_ST_LAST
};

//First Jobs
//Note the oddity of the novice:
//Super Novices are considered the 2-1 version of the novice! Novices are considered a first class type.
enum {
	//Novice And 1-1 Jobs
	MAPID_NOVICE = 0,
	MAPID_SWORDMAN,
	MAPID_MAGE,
	MAPID_ARCHER,
	MAPID_ACOLYTE,
	MAPID_MERCHANT,
	MAPID_THIEF,
	MAPID_TAEKWON,
	MAPID_WEDDING,
	MAPID_GUNSLINGER,
	MAPID_NINJA,
	MAPID_XMAS,
	MAPID_SUMMER,
	MAPID_GANGSI,
	MAPID_SUMMONER,
	MAPID_1_1_MAX,

	//2-1 Jobs
	MAPID_SUPER_NOVICE   = JOBL_2_1 | MAPID_NOVICE,
	MAPID_KNIGHT         = JOBL_2_1 | MAPID_SWORDMAN,
	MAPID_WIZARD         = JOBL_2_1 | MAPID_MAGE,
	MAPID_HUNTER         = JOBL_2_1 | MAPID_ARCHER,
	MAPID_PRIEST         = JOBL_2_1 | MAPID_ACOLYTE,
	MAPID_BLACKSMITH     = JOBL_2_1 | MAPID_MERCHANT,
	MAPID_ASSASSIN       = JOBL_2_1 | MAPID_THIEF,
	MAPID_STAR_GLADIATOR = JOBL_2_1 | MAPID_TAEKWON,
	//                   = JOBL_2_1 | MAPID_WEDDING,
	MAPID_REBELLION      = JOBL_2_1 | MAPID_GUNSLINGER,
	MAPID_KAGEROUOBORO   = JOBL_2_1 | MAPID_NINJA,
	//                   = JOBL_2_1 | MAPID_XMAS,
	//                   = JOBL_2_1 | MAPID_SUMMER,
	MAPID_DEATH_KNIGHT   = JOBL_2_1 | MAPID_GANGSI,
	//                   = JOBL_2_1 | MAPID_SUMMONER,

	//2-2 Jobs
	//                   = JOBL_2_1 | MAPID_NOVICE,
	MAPID_CRUSADER       = JOBL_2_2 | MAPID_SWORDMAN,
	MAPID_SAGE           = JOBL_2_2 | MAPID_MAGE,
	MAPID_BARDDANCER     = JOBL_2_2 | MAPID_ARCHER,
	MAPID_MONK           = JOBL_2_2 | MAPID_ACOLYTE,
	MAPID_ALCHEMIST      = JOBL_2_2 | MAPID_MERCHANT,
	MAPID_ROGUE          = JOBL_2_2 | MAPID_THIEF,
	MAPID_SOUL_LINKER    = JOBL_2_2 | MAPID_TAEKWON,
	//                   = JOBL_2_2 | MAPID_WEDDING,
	//                   = JOBL_2_2 | MAPID_GUNSLINGER,
	//                   = JOBL_2_2 | MAPID_NINJA,
	//                   = JOBL_2_2 | MAPID_XMAS,
	//                   = JOBL_2_2 | MAPID_SUMMER,
	MAPID_DARK_COLLECTOR = JOBL_2_2 | MAPID_GANGSI,
	//                   = JOBL_2_2 | MAPID_SUMMONER,

	//Trans Novice And Trans 1-1 Jobs
	MAPID_NOVICE_HIGH   = JOBL_UPPER | MAPID_NOVICE,
	MAPID_SWORDMAN_HIGH = JOBL_UPPER | MAPID_SWORDMAN,
	MAPID_MAGE_HIGH     = JOBL_UPPER | MAPID_MAGE,
	MAPID_ARCHER_HIGH   = JOBL_UPPER | MAPID_ARCHER,
	MAPID_ACOLYTE_HIGH  = JOBL_UPPER | MAPID_ACOLYTE,
	MAPID_MERCHANT_HIGH = JOBL_UPPER | MAPID_MERCHANT,
	MAPID_THIEF_HIGH    = JOBL_UPPER | MAPID_THIEF,
	//                  = JOBL_UPPER | MAPID_TAEKWON,
	//                  = JOBL_UPPER | MAPID_WEDDING,
	//                  = JOBL_UPPER | MAPID_GUNSLINGER,
	//                  = JOBL_UPPER | MAPID_NINJA,
	//                  = JOBL_UPPER | MAPID_XMAS,
	//                  = JOBL_UPPER | MAPID_SUMMER,
	//                  = JOBL_UPPER | MAPID_GANGSI,
	//                  = JOBL_UPPER | MAPID_SUMMONER,

	//Trans 2-1 Jobs
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_NOVICE,
	MAPID_LORD_KNIGHT    = JOBL_UPPER | JOBL_2_1 | MAPID_SWORDMAN,
	MAPID_HIGH_WIZARD    = JOBL_UPPER | JOBL_2_1 | MAPID_MAGE,
	MAPID_SNIPER         = JOBL_UPPER | JOBL_2_1 | MAPID_ARCHER,
	MAPID_HIGH_PRIEST    = JOBL_UPPER | JOBL_2_1 | MAPID_ACOLYTE,
	MAPID_WHITESMITH     = JOBL_UPPER | JOBL_2_1 | MAPID_MERCHANT,
	MAPID_ASSASSIN_CROSS = JOBL_UPPER | JOBL_2_1 | MAPID_THIEF,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_TAEKWON,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_WEDDING,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_GUNSLINGER,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_NINJA,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_XMAS,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_SUMMER,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_GANGSI,
	//                   = JOBL_UPPER | JOBL_2_1 | MAPID_SUMMONER,

	//Trans 2-2 Jobs
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_NOVICE,
	MAPID_PALADIN    = JOBL_UPPER | JOBL_2_2 | MAPID_SWORDMAN,
	MAPID_PROFESSOR  = JOBL_UPPER | JOBL_2_2 | MAPID_MAGE,
	MAPID_CLOWNGYPSY = JOBL_UPPER | JOBL_2_2 | MAPID_ARCHER,
	MAPID_CHAMPION   = JOBL_UPPER | JOBL_2_2 | MAPID_ACOLYTE,
	MAPID_CREATOR    = JOBL_UPPER | JOBL_2_2 | MAPID_MERCHANT,
	MAPID_STALKER    = JOBL_UPPER | JOBL_2_2 | MAPID_THIEF,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_TAEKWON,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_WEDDING,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_GUNSLINGER,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_NINJA,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_XMAS,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_SUMMER,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_GANGSI,
	//               = JOBL_UPPER | JOBL_2_2 | MAPID_SUMMONER,

	//Baby Novice And Baby 1-1 Jobs
	MAPID_BABY          = JOBL_BABY | MAPID_NOVICE,
	MAPID_BABY_SWORDMAN = JOBL_BABY | MAPID_SWORDMAN,
	MAPID_BABY_MAGE     = JOBL_BABY | MAPID_MAGE,
	MAPID_BABY_ARCHER   = JOBL_BABY | MAPID_ARCHER,
	MAPID_BABY_ACOLYTE  = JOBL_BABY | MAPID_ACOLYTE,
	MAPID_BABY_MERCHANT = JOBL_BABY | MAPID_MERCHANT,
	MAPID_BABY_THIEF    = JOBL_BABY | MAPID_THIEF,
	MAPID_BABY_TAEKWON  = JOBL_BABY | MAPID_TAEKWON,
	//                  = JOBL_BABY | MAPID_WEDDING,
	MAPID_BABY_GUNSLINGER = JOBL_BABY | MAPID_GUNSLINGER,
	MAPID_BABY_NINJA    = JOBL_BABY | MAPID_NINJA,
	//                  = JOBL_BABY | MAPID_XMAS,
	//                  = JOBL_BABY | MAPID_SUMMER,
	//                  = JOBL_BABY | MAPID_GANGSI,
	MAPID_BABY_SUMMONER = JOBL_BABY | MAPID_SUMMONER,

	//Baby 2-1 Jobs
	MAPID_SUPER_BABY      = JOBL_BABY | JOBL_2_1 | MAPID_NOVICE,
	MAPID_BABY_KNIGHT     = JOBL_BABY | JOBL_2_1 | MAPID_SWORDMAN,
	MAPID_BABY_WIZARD     = JOBL_BABY | JOBL_2_1 | MAPID_MAGE,
	MAPID_BABY_HUNTER     = JOBL_BABY | JOBL_2_1 | MAPID_ARCHER,
	MAPID_BABY_PRIEST     = JOBL_BABY | JOBL_2_1 | MAPID_ACOLYTE,
	MAPID_BABY_BLACKSMITH = JOBL_BABY | JOBL_2_1 | MAPID_MERCHANT,
	MAPID_BABY_ASSASSIN   = JOBL_BABY | JOBL_2_1 | MAPID_THIEF,
	MAPID_BABY_STAR_GLADIATOR = JOBL_BABY | JOBL_2_1 | MAPID_TAEKWON,
	//                    = JOBL_BABY | JOBL_2_1 | MAPID_WEDDING,
	MAPID_BABY_REBELLION  = JOBL_BABY | JOBL_2_1 | MAPID_GUNSLINGER,
	MAPID_BABY_KAGEROUOBORO = JOBL_BABY | JOBL_2_1 | MAPID_NINJA,
	//                    = JOBL_BABY | JOBL_2_1 | MAPID_XMAS,
	//                    = JOBL_BABY | JOBL_2_1 | MAPID_SUMMER,
	//                    = JOBL_BABY | JOBL_2_1 | MAPID_GANGSI,
	//                    = JOBL_BABY | JOBL_2_1 | MAPID_SUMMONER,

	//Baby 2-2 Jobs
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_NOVICE,
	MAPID_BABY_CRUSADER   = JOBL_BABY | JOBL_2_2 | MAPID_SWORDMAN,
	MAPID_BABY_SAGE       = JOBL_BABY | JOBL_2_2 | MAPID_MAGE,
	MAPID_BABY_BARDDANCER = JOBL_BABY | JOBL_2_2 | MAPID_ARCHER,
	MAPID_BABY_MONK       = JOBL_BABY | JOBL_2_2 | MAPID_ACOLYTE,
	MAPID_BABY_ALCHEMIST  = JOBL_BABY | JOBL_2_2 | MAPID_MERCHANT,
	MAPID_BABY_ROGUE      = JOBL_BABY | JOBL_2_2 | MAPID_THIEF,
	MAPID_BABY_SOUL_LINKER = JOBL_BABY | JOBL_2_2 | MAPID_TAEKWON,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_WEDDING,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_GUNSLINGER,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_NINJA,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_XMAS,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_SUMMER,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_GANGSI,
	//                    = JOBL_BABY | JOBL_2_2 | MAPID_SUMMONER,

	//3-1 Jobs
	MAPID_SUPER_NOVICE_E   = JOBL_THIRD | JOBL_2_1 | MAPID_NOVICE,
	MAPID_RUNE_KNIGHT      = JOBL_THIRD | JOBL_2_1 | MAPID_SWORDMAN,
	MAPID_WARLOCK          = JOBL_THIRD | JOBL_2_1 | MAPID_MAGE,
	MAPID_RANGER           = JOBL_THIRD | JOBL_2_1 | MAPID_ARCHER,
	MAPID_ARCH_BISHOP      = JOBL_THIRD | JOBL_2_1 | MAPID_ACOLYTE,
	MAPID_MECHANIC         = JOBL_THIRD | JOBL_2_1 | MAPID_MERCHANT,
	MAPID_GUILLOTINE_CROSS = JOBL_THIRD | JOBL_2_1 | MAPID_THIEF,
	MAPID_STAR_EMPEROR     = JOBL_THIRD | JOBL_2_1 | MAPID_TAEKWON,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_WEDDING,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_GUNSLINGER,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_NINJA,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_XMAS,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_SUMMER,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_GANGSI,
	//                     = JOBL_THIRD | JOBL_2_1 | MAPID_SUMMONER,

	//3-2 Jobs
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_NOVICE,
	MAPID_ROYAL_GUARD      = JOBL_THIRD | JOBL_2_2 | MAPID_SWORDMAN,
	MAPID_SORCERER         = JOBL_THIRD | JOBL_2_2 | MAPID_MAGE,
	MAPID_MINSTRELWANDERER = JOBL_THIRD | JOBL_2_2 | MAPID_ARCHER,
	MAPID_SURA             = JOBL_THIRD | JOBL_2_2 | MAPID_ACOLYTE,
	MAPID_GENETIC          = JOBL_THIRD | JOBL_2_2 | MAPID_MERCHANT,
	MAPID_SHADOW_CHASER    = JOBL_THIRD | JOBL_2_2 | MAPID_THIEF,
	MAPID_SOUL_REAPER      = JOBL_THIRD | JOBL_2_2 | MAPID_TAEKWON,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_WEDDING,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_GUNSLINGER,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_NINJA,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_XMAS,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_SUMMER,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_GANGSI,
	//                     = JOBL_THIRD | JOBL_2_2 | MAPID_SUMMONER,

	//Trans 3-1 Jobs
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_NOVICE,
	MAPID_RUNE_KNIGHT_T      = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_SWORDMAN,
	MAPID_WARLOCK_T          = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_MAGE,
	MAPID_RANGER_T           = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_ARCHER,
	MAPID_ARCH_BISHOP_T      = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_ACOLYTE,
	MAPID_MECHANIC_T         = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_MERCHANT,
	MAPID_GUILLOTINE_CROSS_T = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_THIEF,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_TAEKWON,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_WEDDING,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_GUNSLINGER,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_NINJA,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_XMAS,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_SUMMER,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_GANGSI,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_1 | MAPID_SUMMONER,

	//Trans 3-2 Jobs
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_NOVICE,
	MAPID_ROYAL_GUARD_T      = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_SWORDMAN,
	MAPID_SORCERER_T         = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_MAGE,
	MAPID_MINSTRELWANDERER_T = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_ARCHER,
	MAPID_SURA_T             = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_ACOLYTE,
	MAPID_GENETIC_T          = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_MERCHANT,
	MAPID_SHADOW_CHASER_T    = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_THIEF,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_TAEKWON,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_WEDDING,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_GUNSLINGER,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_NINJA,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_XMAS,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_SUMMER,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_GANGSI,
	//                       = JOBL_THIRD | JOBL_UPPER | JOBL_2_2 | MAPID_SUMMONER,

	//Baby 3-1 Jobs
	MAPID_SUPER_BABY_E  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_NOVICE,
	MAPID_BABY_RUNE     = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_SWORDMAN,
	MAPID_BABY_WARLOCK  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_MAGE,
	MAPID_BABY_RANGER   = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_ARCHER,
	MAPID_BABY_BISHOP   = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_ACOLYTE,
	MAPID_BABY_MECHANIC = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_MERCHANT,
	MAPID_BABY_CROSS    = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_THIEF,
	MAPID_BABY_STAR_EMPEROR = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_TAEKWON,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_WEDDING,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_GUNSLINGER,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_NINJA,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_XMAS,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_SUMMER,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_GANGSI,
	//                  = JOBL_THIRD | JOBL_BABY | JOBL_2_1 | MAPID_SUMMONER,

	//Baby 3-2 Jobs
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_NOVICE,
	MAPID_BABY_GUARD            = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_SWORDMAN,
	MAPID_BABY_SORCERER         = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_MAGE,
	MAPID_BABY_MINSTRELWANDERER = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_ARCHER,
	MAPID_BABY_SURA             = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_ACOLYTE,
	MAPID_BABY_GENETIC          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_MERCHANT,
	MAPID_BABY_CHASER           = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_THIEF,
	MAPID_BABY_SOUL_REAPER      = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_TAEKWON,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_WEDDING,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_GUNSLINGER,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_NINJA,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_XMAS,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_SUMMER,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_GANGSI,
	//                          = JOBL_THIRD | JOBL_BABY | JOBL_2_2 | MAPID_SUMMONER,
};

STATIC_ASSERT(((MAPID_1_1_MAX - 1) | MAPID_BASEMASK) == MAPID_BASEMASK, "First class map IDs do not fit into MAPID_BASEMASK");

//This stackable implementation does not means a BL can be more than one type at a time, but it's
// meant to make it easier to check for multiple types at a time on invocations such as map_foreach* calls [Skotlex]
enum bl_type {
	BL_NUL   = 0x000,
	BL_PC    = 0x001,
	BL_MOB   = 0x002,
	BL_PET   = 0x004,
	BL_HOM   = 0x008,
	BL_MER   = 0x010,
	BL_ITEM  = 0x020,
	BL_SKILL = 0x040,
	BL_NPC   = 0x080,
	BL_CHAT  = 0x100,
	BL_ELEM  = 0x200,

	BL_ALL   = 0xFFF,
};

enum npc_subtype { WARP, SHOP, SCRIPT, CASHSHOP, TOMB };

/** optional flags for script labels, used by the label db */
enum script_label_flags {
	/** the label can be called from outside the local scope of the NPC */
	LABEL_IS_EXTERN   = 0x1,
	/** the label is a public or private local NPC function */
	LABEL_IS_USERFUNC = 0x2,
};

/**
 * Race type IDs.
 *
 * Mostly used by scripts/bonuses.
 */
enum Race {
	// Base Races
	RC_FORMLESS = 0,  ///< Formless
	RC_UNDEAD,        ///< Undead
	RC_BRUTE,         ///< Beast/Brute
	RC_PLANT,         ///< Plant
	RC_INSECT,        ///< Insect
	RC_FISH,          ///< Fish
	RC_DEMON,         ///< Demon
	RC_DEMIHUMAN,     ///< Demi-Human (not including Player)
	RC_ANGEL,         ///< Angel
	RC_DRAGON,        ///< Dragon
	RC_PLAYER,        ///< Player
	// Boss
	RC_BOSS,          ///< Boss
	RC_NONBOSS,       ///< Non-boss

	RC_MAX,           // Array size delimiter (keep before the combination races)

	// Combination Races
	RC_NONDEMIHUMAN,   ///< Every race except Demi-Human (including Player)
	RC_NONPLAYER,      ///< Every non-player race
	RC_DEMIPLAYER,     ///< Demi-Human (including Player)
	RC_NONDEMIPLAYER,  ///< Every race except Demi-Human (and except Player)
	RC_ALL = 0xFF,     ///< Every race (implemented as equivalent to RC_BOSS and RC_NONBOSS)
};

/**
 * Race type bitmasks.
 *
 * Used by several bonuses internally, to simplify handling of race combinations.
 */
enum RaceMask {
	RCMASK_NONE      = 0,
	RCMASK_FORMLESS  = 1<<RC_FORMLESS,
	RCMASK_UNDEAD    = 1<<RC_UNDEAD,
	RCMASK_BRUTE     = 1<<RC_BRUTE,
	RCMASK_PLANT     = 1<<RC_PLANT,
	RCMASK_INSECT    = 1<<RC_INSECT,
	RCMASK_FISH      = 1<<RC_FISH,
	RCMASK_DEMON     = 1<<RC_DEMON,
	RCMASK_DEMIHUMAN = 1<<RC_DEMIHUMAN,
	RCMASK_ANGEL     = 1<<RC_ANGEL,
	RCMASK_DRAGON    = 1<<RC_DRAGON,
	RCMASK_PLAYER    = 1<<RC_PLAYER,
	RCMASK_BOSS      = 1<<RC_BOSS,
	RCMASK_NONBOSS   = 1<<RC_NONBOSS,
	RCMASK_NONDEMIPLAYER = RCMASK_FORMLESS | RCMASK_UNDEAD | RCMASK_BRUTE | RCMASK_PLANT | RCMASK_INSECT | RCMASK_FISH | RCMASK_DEMON | RCMASK_ANGEL | RCMASK_DRAGON,
	RCMASK_NONDEMIHUMAN = RCMASK_NONDEMIPLAYER | RCMASK_PLAYER,
	RCMASK_NONPLAYER    = RCMASK_NONDEMIPLAYER | RCMASK_DEMIHUMAN,
	RCMASK_DEMIPLAYER   = RCMASK_DEMIHUMAN | RCMASK_PLAYER,
	RCMASK_ALL          = RCMASK_BOSS | RCMASK_NONBOSS,
	RCMASK_ANY          = RCMASK_NONPLAYER | RCMASK_PLAYER,
};

enum {
	RC2_NONE = 0,
	RC2_GOBLIN,
	RC2_KOBOLD,
	RC2_ORC,
	RC2_GOLEM,
	RC2_GUARDIAN,
	RC2_NINJA,
	RC2_SCARABA,
	RC2_TURTLE,
	RC2_MAX
};

enum elements {
	ELE_NEUTRAL=0,
	ELE_WATER,
	ELE_EARTH,
	ELE_FIRE,
	ELE_WIND,
	ELE_POISON,
	ELE_HOLY,
	ELE_DARK,
	ELE_GHOST,
	ELE_UNDEAD,
	ELE_MAX,
	ELE_ALL = 0xFF
};

/**
 * Types of Ball Types
 * Used by clif_spiritball [KeiKun]
 */
enum spirit_ball_types {
	BALL_TYPE_NONE = 0,
	BALL_TYPE_SPIRIT,
	BALL_TYPE_SOUL
};

/**
 * Types of spirit charms.
 *
 * Note: Code assumes that this matches the first entries in enum elements.
 */
enum spirit_charm_types {
	CHARM_TYPE_NONE = 0,
	CHARM_TYPE_WATER,
	CHARM_TYPE_LAND,
	CHARM_TYPE_FIRE,
	CHARM_TYPE_WIND
};

enum auto_trigger_flag {
	ATF_SELF=0x01,
	ATF_TARGET=0x02,
	ATF_SHORT=0x04,
	ATF_LONG=0x08,
	ATF_WEAPON=0x10,
	ATF_MAGIC=0x20,
	ATF_MISC=0x40,
};

/**
 * used for map->search_free_cell parameter flag
 */
enum search_freecell {
	SFC_DEFAULT = 0,
	SFC_XY_CENTER = 1,
	SFC_REACHABLE = 2,
	SFC_AVOIDPLAYER = 4,
};

struct block_list {
	struct block_list *next,*prev;
	int id;
	int16 m, x, y;
	bool deleted;
	enum bl_type type;
};

// Mob List Held in memory for Dynamic Mobs [Wizputer]
// Expanded to specify all mob-related spawn data by [Skotlex]
struct spawn_data {
	int class_;                ///< Class, used because a mob can change it's class
	unsigned short m, x, y;      ///< Spawn information (map, point, spawn-area around point)
	signed short xs, ys;
	unsigned short num;          ///< Number of mobs using this structure
	unsigned short active;       ///< Number of mobs that are already spawned (for mob_remove_damaged: no)
	unsigned int delay1, delay2; ///< Spawn delay (fixed base + random variance)
	unsigned int level;
	struct {
		unsigned int size : 2;    ///< Holds if mob has to be tiny/large
		unsigned int ai : 4;      ///< Special AI for summoned monsters.
		//0: Normal mob | 1: Standard summon, attacks mobs
		//2: Alchemist Marine Sphere | 3: Alchemist Summon Flora | 4: Summon Zanzou
		unsigned int dynamic : 1; ///< Whether this data is indexed by a map's dynamic mob list
		uint8 boss;    ///< 0: Non-boss monster | 1: Boss monster | 2: MVP
	} state;
	char name[NAME_LENGTH], eventname[EVENT_NAME_LENGTH]; //Name/event
};

struct flooritem_data {
	struct block_list bl;
	unsigned char subx,suby;
	int cleartimer;
	int first_get_charid,second_get_charid,third_get_charid;
	int64 first_get_tick,second_get_tick,third_get_tick;
	struct item item_data;
	bool showdropeffect;
};

enum status_point_types { //we better clean up this enum and change it name [Hemagx]
	SP_NONE = -1,
	SP_SPEED = 0,
	SP_BASEEXP = 1,
	SP_JOBEXP = 2,
	SP_KARMA = 3,
	SP_MANNER = 4,
	SP_HP = 5,
	SP_MAXHP = 6,
	SP_SP = 7,
	SP_MAXSP = 8,
	SP_STATUSPOINT = 9,
	SP_0a = 10,
	SP_BASELEVEL = 11,
	SP_SKILLPOINT = 12,
	SP_STR = 13,
	SP_AGI = 14,
	SP_VIT = 15,
	SP_INT = 16,
	SP_DEX = 17,
	SP_LUK = 18,
	SP_CLASS = 19,
	SP_ZENY = 20,
	SP_SEX = 21,
	SP_NEXTBASEEXP = 22,
	SP_NEXTJOBEXP = 23,
	SP_WEIGHT = 24,
	SP_MAXWEIGHT = 25,
	SP_1a = 26,
	SP_1b = 27,
	SP_1c = 28,
	SP_1d = 29,
	SP_1e = 30,
	SP_1f = 31,
	SP_USTR = 32,
	SP_UAGI = 33,
	SP_UVIT = 34,
	SP_UINT = 35,
	SP_UDEX = 36,
	SP_ULUK = 37,
	SP_26 = 38,
	SP_27 = 39,
	SP_28 = 40,
	SP_ATK1 = 41,
	SP_ATK2 = 42,
	SP_MATK1 = 43,
	SP_MATK2 = 44,
	SP_DEF1 = 45,
	SP_DEF2 = 46,
	SP_MDEF1 = 47,
	SP_MDEF2 = 48,
	SP_HIT = 49,
	SP_FLEE1 = 50,
	SP_FLEE2 = 51,
	SP_CRITICAL = 52,
	SP_ASPD = 53,
	SP_36 = 54,
	SP_JOBLEVEL = 55,
	SP_UPPER = 56,
	SP_PARTNER = 57,
	SP_CART = 58,
	SP_FAME = 59,
	SP_UNBREAKABLE = 60,
	SP_CARTINFO = 99,
	SP_BASEJOB = 119,  // 100+19 - celest
	SP_BASECLASS = 120, //Hmm.. why 100+19? I just use the next one... [Skotlex]
	SP_KILLERRID = 121,
	SP_KILLEDRID = 122,
	SP_SLOTCHANGE = 123,
	SP_CHARRENAME = 124,
	SP_MOD_EXP = 125,
	SP_MOD_DROP = 126,
	SP_MOD_DEATH = 127,
	SP_BANKVAULT = 128,

	// Mercenaries
	SP_MERCFLEE = 165,
	SP_MERCKILLS = 189,
	SP_MERCFAITH = 190,

	// 4th job update
	SP_POW = 219,
	SP_STA = 220,
	SP_WIS = 221,
	SP_SPL = 222,
	SP_CON = 223,
	SP_CRT = 224,
	SP_PATK = 225,
	SP_SMATK = 226,
	SP_RES = 227,
	SP_MRES = 228,
	SP_HPLUS = 229,
	SP_CRATE = 230,
	SP_TSTATUSPOINT = 231,
	SP_AP = 232,
	SP_MAXAP = 233,
	SP_UPOW = 247,
	SP_USTA = 248,
	SP_UWIS = 249,
	SP_USPL = 250,
	SP_UCON = 251,
	SP_UCRT = 252,

	// original 1000-
	SP_ATTACKRANGE = 1000,
	SP_ATKELE = 1001,
	SP_DEFELE = 1002,
	SP_CASTRATE = 1003,
	SP_MAXHPRATE = 1004,
	SP_MAXSPRATE = 1005,
	SP_SPRATE = 1006,
	SP_ADDELE = 1007,
	SP_ADDRACE = 1008,
	SP_ADDSIZE = 1009,
	SP_SUBELE = 1010,
	SP_SUBRACE = 1011,
	SP_ADDEFF = 1012,
	SP_RESEFF = 1013,
	SP_BASE_ATK = 1014,
	SP_ASPD_RATE = 1015,
	SP_HP_RECOV_RATE = 1016,
	SP_SP_RECOV_RATE = 1017,
	SP_SPEED_RATE = 1018,
	SP_CRITICAL_DEF = 1019,
	SP_NEAR_ATK_DEF = 1020,
	SP_LONG_ATK_DEF = 1021,
	SP_DOUBLE_RATE = 1022,
	SP_DOUBLE_ADD_RATE = 1023,
	SP_SKILL_HEAL = 1024,
	SP_MATK_RATE = 1025,
	SP_IGNORE_DEF_ELE = 1026,
	SP_IGNORE_DEF_RACE = 1027,
	SP_ATK_RATE = 1028,
	SP_SPEED_ADDRATE = 1029,
	SP_SP_REGEN_RATE = 1030,
	SP_MAGIC_ATK_DEF = 1031,
	SP_MISC_ATK_DEF = 1032,
	SP_IGNORE_MDEF_ELE = 1033,
	SP_IGNORE_MDEF_RACE = 1034,
	SP_MAGIC_ADDELE = 1035,
	SP_MAGIC_ADDRACE = 1036,
	SP_MAGIC_ADDSIZE = 1037,
	SP_PERFECT_HIT_RATE = 1038,
	SP_PERFECT_HIT_ADD_RATE = 1039,
	SP_CRITICAL_RATE = 1040,
	SP_GET_ZENY_NUM = 1041,
	SP_ADD_GET_ZENY_NUM = 1042,
	SP_ADD_DAMAGE_CLASS = 1043,
	SP_ADD_MAGIC_DAMAGE_CLASS = 1044,
	SP_ADD_DEF_CLASS = 1045,
	SP_ADD_MDEF_CLASS = 1046,
	SP_ADD_MONSTER_DROP_ITEM = 1047,
	SP_DEF_RATIO_ATK_ELE = 1048,
	SP_DEF_RATIO_ATK_RACE = 1049,
	SP_UNBREAKABLE_GARMENT = 1050,
	SP_HIT_RATE = 1051,
	SP_FLEE_RATE = 1052,
	SP_FLEE2_RATE = 1053,
	SP_DEF_RATE = 1054,
	SP_DEF2_RATE = 1055,
	SP_MDEF_RATE = 1056,
	SP_MDEF2_RATE = 1057,
	SP_SPLASH_RANGE = 1058,
	SP_SPLASH_ADD_RANGE = 1059,
	SP_AUTOSPELL = 1060,
	SP_HP_DRAIN_RATE = 1061,
	SP_SP_DRAIN_RATE = 1062,
	SP_SHORT_WEAPON_DAMAGE_RETURN = 1063,
	SP_LONG_WEAPON_DAMAGE_RETURN = 1064,
	SP_WEAPON_COMA_ELE = 1065,
	SP_WEAPON_COMA_RACE = 1066,
	SP_ADDEFF2 = 1067,
	SP_BREAK_WEAPON_RATE = 1068,
	SP_BREAK_ARMOR_RATE = 1069,
	SP_ADD_STEAL_RATE = 1070,
	SP_MAGIC_DAMAGE_RETURN = 1071,
	SP_ALL_STATS = 1073,
	SP_AGI_VIT = 1074,
	SP_AGI_DEX_STR = 1075,
	SP_PERFECT_HIDE = 1076,
	SP_NO_KNOCKBACK = 1077,
	SP_CLASSCHANGE = 1078,
	SP_HP_DRAIN_VALUE = 1079,
	SP_SP_DRAIN_VALUE = 1080,
	SP_WEAPON_ATK = 1081,
	SP_WEAPON_ATK_RATE = 1082,
	SP_DELAYRATE = 1083,
	SP_HP_DRAIN_RATE_RACE = 1084,
	SP_SP_DRAIN_RATE_RACE = 1085,
	SP_IGNORE_MDEF_RATE = 1086,
	SP_IGNORE_DEF_RATE = 1087,
	SP_SKILL_HEAL2 = 1088,
	SP_ADDEFF_ONSKILL = 1089,
	SP_ADD_HEAL_RATE = 1090,
	SP_ADD_HEAL2_RATE = 1091,
	SP_HP_VANISH_RATE = 1092,
	SP_RESTART_FULL_RECOVER = 2000,
	SP_NO_CASTCANCEL = 2001,
	SP_NO_SIZEFIX = 2002,
	SP_NO_MAGIC_DAMAGE = 2003,
	SP_NO_WEAPON_DAMAGE = 2004,
	SP_NO_GEMSTONE = 2005,
	SP_NO_CASTCANCEL2 = 2006,
	SP_NO_MISC_DAMAGE = 2007,
	SP_UNBREAKABLE_WEAPON = 2008,
	SP_UNBREAKABLE_ARMOR = 2009,
	SP_UNBREAKABLE_HELM = 2010,
	SP_UNBREAKABLE_SHIELD = 2011,
	SP_LONG_ATK_RATE = 2012,
	SP_CRIT_ATK_RATE = 2013,
	SP_CRITICAL_ADDRACE = 2014,
	SP_NO_REGEN = 2015,
	SP_ADDEFF_WHENHIT = 2016,
	SP_AUTOSPELL_WHENHIT = 2017,
	SP_SKILL_ATK = 2018,
	SP_UNSTRIPABLE = 2019,
	SP_AUTOSPELL_ONSKILL = 2020,
	SP_SP_GAIN_VALUE = 2021,
	SP_HP_REGEN_RATE = 2022,
	SP_HP_LOSS_RATE = 2023,
	SP_ADDRACE2 = 2024,
	SP_HP_GAIN_VALUE = 2025,
	SP_SUBSIZE = 2026,
	SP_HP_DRAIN_VALUE_RACE = 2027,
	SP_ADD_ITEM_HEAL_RATE = 2028,
	SP_SP_DRAIN_VALUE_RACE = 2029,
	SP_EXP_ADDRACE = 2030,
	SP_SP_GAIN_RACE = 2031,
	SP_SUBRACE2 = 2032,
	SP_UNBREAKABLE_SHOES = 2033,
	SP_UNSTRIPABLE_WEAPON = 2034,
	SP_UNSTRIPABLE_ARMOR = 2035,
	SP_UNSTRIPABLE_HELM = 2036,
	SP_UNSTRIPABLE_SHIELD = 2037,
	SP_INTRAVISION = 2038,
	SP_ADD_MONSTER_DROP_CHAINITEM = 2039,
	SP_SP_LOSS_RATE = 2040,
	SP_ADD_SKILL_BLOW = 2041,
	SP_SP_VANISH_RATE = 2042,
	SP_MAGIC_SP_GAIN_VALUE = 2043,
	SP_MAGIC_HP_GAIN_VALUE = 2044,
	SP_ADD_CLASS_DROP_ITEM = 2045,
	SP_EMATK = 2046,
	SP_SP_GAIN_RACE_ATTACK = 2047,
	SP_HP_GAIN_RACE_ATTACK = 2048,
	SP_SKILL_USE_SP_RATE = 2049,
	SP_SKILL_COOLDOWN = 2050,
	SP_SKILL_FIXEDCAST = 2051,
	SP_SKILL_VARIABLECAST = 2052,
	SP_FIXCASTRATE = 2053,
	SP_VARCASTRATE = 2054,
	SP_SKILL_USE_SP = 2055,
	SP_MAGIC_ATK_ELE = 2056,
	SP_ADD_FIXEDCAST = 2057,
	SP_ADD_VARIABLECAST = 2058,
	SP_SET_DEF_RACE = 2059,
	SP_SET_MDEF_RACE = 2060,
	SP_RACE_TOLERANCE = 2061,
	SP_ADDMAXWEIGHT = 2062,
	SP_SUB_DEF_ELE = 2063,
	SP_MAGIC_SUB_DEF_ELE = 2064,
	SP_STATE_NO_RECOVER_RACE = 2065,
	SP_SUB_SKILL = 2066,
	SP_ADD_DROP_RACE = 2067,

#ifndef SP_LAST_KNOWN
	SP_LAST_KNOWN
#endif
};

enum look {
	LOOK_BASE,
	LOOK_HAIR,
	LOOK_WEAPON,
	LOOK_HEAD_BOTTOM,
	LOOK_HEAD_TOP,
	LOOK_HEAD_MID,
	LOOK_HAIR_COLOR,
	LOOK_CLOTHES_COLOR,
	LOOK_SHIELD,
	LOOK_SHOES,
	LOOK_BODY,
	LOOK_FLOOR,
	LOOK_ROBE,
	LOOK_BODY2,

#ifndef LOOK_MAX
	LOOK_MAX
#endif
};

// used by map_setcell()
typedef enum {
	CELL_WALKABLE,
	CELL_SHOOTABLE,
	CELL_WATER,

	CELL_NPC,
	CELL_BASILICA,
	CELL_LANDPROTECTOR,
	CELL_NOVENDING,
	CELL_NOCHAT,
	CELL_ICEWALL,
	CELL_NOICEWALL,
	CELL_NOSKILL,

} cell_t;

// used by map->getcell()
typedef enum {
	CELL_GETTYPE,    ///< retrieves a cell's 'gat' type

	CELL_CHKWALL,    ///< wall (gat type 1)
	CELL_CHKWATER,   ///< water (gat type 3)
	CELL_CHKCLIFF,   ///< cliff/gap (gat type 5)

	CELL_CHKPASS,    ///< passable cell (gat type non-1/5)
	CELL_CHKREACH,   ///< Same as PASS, but ignores the cell-stacking mod.
	CELL_CHKNOPASS,  ///< non-passable cell (gat types 1 and 5)
	CELL_CHKNOREACH, ///< Same as NOPASS, but ignores the cell-stacking mod.
	CELL_CHKSTACK,   ///< whether cell is full (reached cell stacking limit)

	CELL_CHKNPC,
	CELL_CHKBASILICA,
	CELL_CHKLANDPROTECTOR,
	CELL_CHKNOVENDING,
	CELL_CHKNOCHAT,
	CELL_CHKICEWALL,
	CELL_CHKNOICEWALL,
	CELL_CHKNOSKILL,

} cell_chk;

struct mapcell {
	// terrain flags
	unsigned char
		walkable : 1,
		shootable : 1,
		water : 1;

	// dynamic flags
	unsigned char
		npc : 1,
		basilica : 1,
		landprotector : 1,
		novending : 1,
		nochat : 1,
		icewall : 1,
		noicewall : 1,
		noskill : 1;

#ifdef CELL_NOSTACK
	int cell_bl; //Holds amount of bls in this cell.
#endif
};

struct iwall_data {
	char wall_name[50];
	short m, x, y, size;
	int8 dir;
	bool shootable;
};

struct mapflag_skill_adjust {
	unsigned short skill_id;
	unsigned short modifier;
};

enum map_zone_skill_subtype {
	MZS_NONE  = 0x0,
	MZS_CLONE = 0x01,
	MZS_BOSS  = 0x02,

	MZS_ALL   = 0xFFF
};

struct map_zone_disabled_skill_entry {
	int nameid;
	enum bl_type type;
	enum map_zone_skill_subtype subtype;
};
struct map_zone_disabled_command_entry {
	AtCommandFunc cmd;
	int group_lv;
};

struct map_zone_skill_damage_cap_entry {
	int nameid;
	unsigned int cap;
	enum bl_type type;
	enum map_zone_skill_subtype subtype;
};

enum map_zone_merge_type {
	MZMT_NORMAL = 0, ///< MZMT_MERGEABLE zones can merge *into* MZMT_NORMAL zones (but not the converse).
	MZMT_MERGEABLE,  ///< Can merge with other MZMT_MERGEABLE zones and *into* MZMT_NORMAL zones.
	MZMT_NEVERMERGE, ///< Cannot merge with any zones.
};

/**
 * align for packet ZC_SAY_DIALOG_ALIGN
 **/
enum say_dialog_align {
	DIALOG_ALIGN_LEFT   = 0,
	DIALOG_ALIGN_RIGHT  = 1,
	DIALOG_ALIGN_CENTER = 2,
	DIALOG_ALIGN_TOP    = 3,
	DIALOG_ALIGN_MIDDLE = 4,
	DIALOG_ALIGN_BOTTOM = 5
};

struct map_zone_data {
	char name[MAP_ZONE_NAME_LENGTH];/* 20'd */
	enum map_zone_merge_type merge_type;
	struct map_zone_disabled_skill_entry **disabled_skills;
	int disabled_skills_count;
	int *disabled_items;
	int disabled_items_count;
	int *cant_disable_items; /** when a zone wants to ensure such a item is never disabled (i.e. gvg zone enables a item that is restricted everywhere else) **/
	int cant_disable_items_count;
	char **mapflags;
	int mapflags_count;
	struct map_zone_disabled_command_entry **disabled_commands;
	int disabled_commands_count;
	struct map_zone_skill_damage_cap_entry **capped_skills;
	int capped_skills_count;
	struct {
		unsigned int merged : 1;
	} info;
};

struct map_drop_list {
	int drop_id;
	int drop_type;
	int drop_per;
};

/**
 * Map ID (m) for "none" or unallocated map.
 */
#define MAPID_NONE -1

struct map_data {
	char name[MAP_NAME_LENGTH];
	uint16 index; // The map index used by the mapindex* functions.
	struct mapcell* cell; // Holds the information of each map cell (NULL if the map is not on this map-server).

	/* 2D Orthogonal Range Search: Grid Implementation
	   "Algorithms in Java, Parts 1-4" 3.18, Robert Sedgewick
	   Map is divided into squares, called blocks (side length = BLOCK_SIZE).
	   For each block there is a linked list of objects in that block (block_list).
	   Array provides capability to access immediately the set of objects close
	   to a given object.
	   The linked lists provide the flexibility to store the objects without
	   knowing ahead how many objects fall into each block.
	*/
	struct block_list **block; // Grid array of block_lists containing only non-BL_MOB objects
	struct block_list **block_mob; // Grid array of block_lists containing only BL_MOB objects

	int16 m;
	int16 xs,ys; // map dimensions (in cells)
	int16 bxs,bys; // map dimensions (in blocks)
	int16 bgscore_lion, bgscore_eagle; // Battleground ScoreBoard
	int npc_num;
	int users;
	int users_pvp;
	int iwall_num; // Total of invisible walls in this map
	struct map_flag {
		unsigned town : 1; // [Suggestion to protect Mail System]
		unsigned autotrade : 1;
		unsigned allowks : 1; // [Kill Steal Protection]
		unsigned nomemo : 1;
		unsigned noteleport : 1;
		unsigned noreturn : 1;
		unsigned monster_noteleport : 1;
		unsigned nosave : 1;
		unsigned nobranch : 1;
		unsigned noexppenalty : 1;
		unsigned pvp : 1;
		unsigned pvp_noparty : 1;
		unsigned pvp_noguild : 1;
		unsigned pvp_nightmaredrop :1;
		unsigned pvp_nocalcrank : 1;
		unsigned gvg_castle : 1;
		unsigned gvg : 1; // Now it identifies gvg versus maps that are active 24/7
		unsigned gvg_dungeon : 1; // Celest
		unsigned gvg_noparty : 1;
		unsigned battleground : 2; // [BattleGround System]
		unsigned cvc : 1;
		unsigned nozenypenalty : 1;
		unsigned notrade : 1;
		unsigned noskill : 1;
		unsigned nowarp : 1;
		unsigned nowarpto : 1;
		unsigned noicewall : 1; // [Valaris]
		unsigned snow : 1; // [Valaris]
		unsigned clouds : 1;
		unsigned clouds2 : 1; // [Valaris]
		unsigned fog : 1; // [Valaris]
		unsigned fireworks : 1;
		unsigned sakura : 1; // [Valaris]
		unsigned leaves : 1; // [Valaris]
		unsigned nobaseexp : 1; // [Lorky] added by Lupus
		unsigned nojobexp : 1; // [Lorky]
		unsigned nomobloot : 1; // [Lorky]
		unsigned nomvploot : 1; // [Lorky]
		unsigned nightenabled :1; //For night display. [Skotlex]
		unsigned nodrop : 1;
		unsigned novending : 1;
		unsigned loadevent : 1;
		unsigned nochat :1;
		unsigned partylock :1;
		unsigned guildlock :1;
		unsigned src4instance : 1; // To flag this map when it's used as a src map for instances
		unsigned reset :1; // [Daegaladh]
		unsigned chsysnolocalaj : 1;
		unsigned noknockback : 1;
		unsigned notomb : 1;
		unsigned nocashshop : 1;
		unsigned noautoloot : 1;
		unsigned pairship_startable : 1;
		unsigned pairship_endable : 1;
		unsigned nostorage : 2;
		unsigned nogstorage : 2;
		unsigned nopet : 1;
		uint32 noviewid; ///< noviewid (bitmask - @see enum equip_pos)
	} flag;
	struct point save;
	struct npc_data *npc[MAX_NPC_PER_MAP];
	struct map_drop_list *drop_list;
	unsigned short drop_list_count;

	struct spawn_data *moblist[MAX_MOB_LIST_PER_MAP]; // [Wizputer]
	int mob_delete_timer; // [Skotlex]
	int jexp; // map experience multiplicator
	int bexp; // map experience multiplicator
	int nocommand; //Blocks @/# commands for non-gms. [Skotlex]

	// Instance Variables
	int instance_id;
	int instance_src_map;

	/* adjust_unit_duration mapflag */
	struct mapflag_skill_adjust **units;
	unsigned short unit_count;
	/* adjust_skill_damage mapflag */
	struct mapflag_skill_adjust **skills;
	unsigned short skill_count;

	/* Hercules nocast db overhaul */
	struct map_zone_data *zone;
	char **zone_mf;/* used to store this map's zone mapflags that should be re-applied once zone is removed */
	unsigned short zone_mf_count;
	struct map_zone_data *prev_zone;

	/* Hercules Local Chat */
	struct channel_data *channel;

	/* invincible_time_inc mapflag */
	unsigned int invincible_time_inc;

	/* weapon_damage_rate mapflag */
	unsigned short weapon_damage_rate;
	/* magic_damage_rate mapflag */
	unsigned short magic_damage_rate;
	/* misc_damage_rate mapflag */
	unsigned short misc_damage_rate;
	/* short_damage_rate mapflag */
	unsigned short short_damage_rate;
	/* long_damage_rate mapflag */
	unsigned short long_damage_rate;

	bool custom_name; ///< Whether the instanced map is using a custom name

	/* */
	int (*getcellp)(struct map_data* m, const struct block_list *bl, int16 x, int16 y, cell_chk cellchk);
	void (*setcell) (int16 m, int16 x, int16 y, cell_t cell, bool flag);
	struct {
		uint8 *data;
		int len;
	} cell_buf;

	/* questinfo entries list */
	VECTOR_DECL(struct npc_data *) qi_list;

	/* speeds up clif_updatestatus processing by causing hpmeter to run only when someone with the permission can view it */
	unsigned short hpmeter_visible;
	struct hplugin_data_store *hdata; ///< HPM Plugin Data Store
};

#define map_id2index(id) (map->list[(id)].index)

/// Bitfield of flags for the iterator.
enum e_mapitflags {
	MAPIT_NORMAL = 0,
	//MAPIT_PCISPLAYING = 1,// Unneeded as pc_db/id_db will only hold authed, active players.
};

struct s_mapiterator;

/* temporary until the map.c "Hercules Renewal Phase One" design is complete. */
struct mapit_interface {
	struct s_mapiterator*   (*alloc) (enum e_mapitflags flags, enum bl_type types);
	void                    (*free) (struct s_mapiterator* iter);
	struct block_list*      (*first) (struct s_mapiterator* iter);
	struct block_list*      (*last) (struct s_mapiterator* iter);
	struct block_list*      (*next) (struct s_mapiterator* iter);
	struct block_list*      (*prev) (struct s_mapiterator* iter);
	bool                    (*exists) (struct s_mapiterator* iter);
};

#define mapit_getallusers() (mapit->alloc(MAPIT_NORMAL,BL_PC))
#define mapit_geteachpc()   (mapit->alloc(MAPIT_NORMAL,BL_PC))
#define mapit_geteachmob()  (mapit->alloc(MAPIT_NORMAL,BL_MOB))
#define mapit_geteachnpc()  (mapit->alloc(MAPIT_NORMAL,BL_NPC))
#define mapit_geteachiddb() (mapit->alloc(MAPIT_NORMAL,BL_ALL))

//Useful typedefs from jA [Skotlex]
typedef struct map_session_data TBL_PC;
typedef struct npc_data         TBL_NPC;
typedef struct mob_data         TBL_MOB;
typedef struct flooritem_data   TBL_ITEM;
typedef struct chat_data        TBL_CHAT;
typedef struct skill_unit       TBL_SKILL;
typedef struct pet_data         TBL_PET;
typedef struct homun_data       TBL_HOM;
typedef struct mercenary_data   TBL_MER;
typedef struct elemental_data   TBL_ELEM;

/**
 * Casts a block list to a specific type.
 *
 * @remark
 *   The `bl` argument may be evaluated more than once.
 *
 * @param type_ The block list type (using symbols from enum bl_type).
 * @param bl    The source block list to cast.
 * @return The block list, cast to the correct type.
 * @retval NULL if bl is the wrong type or NULL.
 */
#define BL_CAST(type_, bl) \
	( ((bl) == (struct block_list *)NULL || (bl)->type != (type_)) ? (T ## type_ *)NULL : (T ## type_ *)(bl) )

/**
 * Casts a const block list to a specific type.
 *
 * @remark
 *   The `bl` argument may be evaluated more than once.
 *
 * @param type_ The block list type (using symbols from enum bl_type).
 * @param bl    The source block list to cast.
 * @return The block list, cast to the correct type.
 * @retval NULL if bl is the wrong type or NULL.
 */
#define BL_CCAST(type_, bl) \
	( ((bl) == (const struct block_list *)NULL || (bl)->type != (type_)) ? (const T ## type_ *)NULL : (const T ## type_ *)(bl) )

/**
 * Helper function for `BL_UCAST`.
 *
 * @warning
 *   This function shouldn't be called on it own.
 *
 * The purpose of this function is to produce a compile-timer error if a non-bl
 * object is passed to BL_UCAST. It's declared as static inline to let the
 * compiler optimize out the function call overhead.
 */
static inline struct block_list *BL_UCAST_(struct block_list *bl) __attribute__((unused));
static inline struct block_list *BL_UCAST_(struct block_list *bl)
{
	return bl;
}

/**
 * Casts a block list to a specific type, without performing any type checks.
 *
 * @remark
 *   The `bl` argument is guaranteed to be evaluated once and only once.
 *
 * @param type_ The block list type (using symbols from enum bl_type).
 * @param bl    The source block list to cast.
 * @return The block list, cast to the correct type.
 */
#define BL_UCAST(type_, bl) \
	((T ## type_ *)BL_UCAST_(bl))

/**
 * Helper function for `BL_UCCAST`.
 *
 * @warning
 *   This function shouldn't be called on it own.
 *
 * The purpose of this function is to produce a compile-timer error if a non-bl
 * object is passed to BL_UCAST. It's declared as static inline to let the
 * compiler optimize out the function call overhead.
 */
static inline const struct block_list *BL_UCCAST_(const struct block_list *bl) __attribute__((unused));
static inline const struct block_list *BL_UCCAST_(const struct block_list *bl)
{
	return bl;
}

/**
 * Casts a const block list to a specific type, without performing any type checks.
 *
 * @remark
 *   The `bl` argument is guaranteed to be evaluated once and only once.
 *
 * @param type_ The block list type (using symbols from enum bl_type).
 * @param bl    The source block list to cast.
 * @return The block list, cast to the correct type.
 */
#define BL_UCCAST(type_, bl) \
	((const T ## type_ *)BL_UCCAST_(bl))

struct charid_request {
	struct charid_request* next;
	int charid;// who want to be notified of the nick
};
struct charid2nick {
	char nick[NAME_LENGTH];
	struct charid_request* requests;// requests of notification on this nick
};

// New mcache file format header
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris
struct map_cache_header {
	int16 version;
	uint8 md5_checksum[16];
	int16 xs;
	int16 ys;
	int32 len;
} __attribute__((packed));
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

/*=====================================
* Interface : map.h
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
struct map_interface {

	/* vars */
	bool minimal;     ///< Starts the server in minimal initialization mode.
	bool scriptcheck; ///< Starts the server in script-check mode.

	/** Additional scripts requested through the command-line */
	char **extra_scripts;
	int extra_scripts_count;

	int retval;
	int count;

	int autosave_interval;
	int minsave_interval;
	int save_settings;
	int agit_flag;
	int agit2_flag;
	int night_flag; // 0=day, 1=night [Yor]
	int enable_spy; //Determines if @spy commands are active.
	char db_path[256];

	char help_txt[256];
	char charhelp_txt[256];

	char wisp_server_name[NAME_LENGTH];

	char *INTER_CONF_NAME;
	char *LOG_CONF_NAME;
	char *MAP_CONF_NAME;
	char *BATTLE_CONF_FILENAME;
	char *ATCOMMAND_CONF_FILENAME;
	char *SCRIPT_CONF_NAME;
	char *MSG_CONF_NAME;
	char *GRF_PATH_FILENAME;

	char autotrade_merchants_db[32];
	char autotrade_data_db[32];
	char npc_market_data_db[32];
	char npc_barter_data_db[32];
	char npc_expanded_barter_data_db[32];

	char default_codepage[32];
	char default_lang_str[64];
	uint8 default_lang_id;

	int server_port;
	char server_ip[32];
	char server_id[32];
	char server_pw[100];
	char server_db[32];
	struct Sql *mysql_handle;

	uint16 port;
	int users;
	int enable_grf; //To enable/disable reading maps from GRF files, bypassing mapcache [blackhole89]
	bool ip_set;
	bool char_ip_set;

	int16 index2mapid[MAX_MAPINDEX];
	/* */
	struct DBMap *id_db;     // int id -> struct block_list*
	struct DBMap *pc_db;     // int id -> struct map_session_data*
	struct DBMap *mobid_db;  // int id -> struct mob_data*
	struct DBMap *bossid_db; // int id -> struct mob_data* (MVP db)
	struct DBMap *nick_db;   // int char_id -> struct charid2nick* (requested names of offline characters)
	struct DBMap *charid_db; // int char_id -> struct map_session_data*
	struct DBMap *regen_db;  // int id -> struct block_list* (status_natural_heal processing)
	struct DBMap *zone_db;   // string => struct map_zone_data
	struct DBMap *iwall_db;
	struct block_list **block_free;
	int block_free_count, block_free_lock, block_free_list_size;
#ifdef SANITIZE
	int **block_free_sanitize;
#endif  // SANITIZE
	struct block_list **bl_list;
	int bl_list_count, bl_list_size;
BEGIN_ZEROED_BLOCK; // This block is zeroed in map_defaults()
	struct block_list bl_head;
	struct map_zone_data zone_all;/* used as a base on all maps */
	struct map_zone_data zone_pk;/* used for (pk_mode) */
END_ZEROED_BLOCK;
	/* */
	struct map_session_data *cpsd;
	struct map_data *list;
	/* [Ind/Hercules] */
	struct eri *iterator_ers;
	/* */
	struct eri *flooritem_ers;
	/* */
	int bonus_id;
	/* */
	bool cpsd_active;
	/* funcs */
	void (*zone_init) (void);
	void (*zone_remove) (int m);
	void (*zone_remove_all) (int m);
	void (*zone_apply) (int m, struct map_zone_data *zone, const char* start, const char* buffer, const char* filepath);
	void (*zone_change) (int m, struct map_zone_data *zone, const char* start, const char* buffer, const char* filepath);
	void (*zone_change2) (int m, struct map_zone_data *zone);
	void (*zone_reload) (void);

	int (*getcell) (int16 m, const struct block_list *bl, int16 x, int16 y, cell_chk cellchk);
	void (*setgatcell) (int16 m, int16 x, int16 y, int gat);

	void (*cellfromcache) (struct map_data *m);
	// users
	void (*setusers) (int);
	int (*getusers) (void);
	int (*usercount) (void);
	// blocklist lock
	int (*freeblock) (struct block_list *bl);
	int (*freeblock_lock) (void);
	int (*freeblock_unlock) (void);
	// blocklist manipulation
	int (*addblock) (struct block_list* bl);
	int (*delblock) (struct block_list* bl);
	int (*moveblock) (struct block_list *bl, int x1, int y1, int64 tick);
	//blocklist nb in one cell
	int (*count_oncell) (int16 m,int16 x,int16 y,int type,int flag);
	struct skill_unit * (*find_skill_unit_oncell) (struct block_list* target,int16 x,int16 y,uint16 skill_id,struct skill_unit* out_unit, int flag);
	// search and creation
	int (*get_new_object_id) (void);
	int (*search_free_cell) (struct block_list *src, int16 m, int16 *x, int16 *y, int16 range_x, int16 range_y, int flag);
	bool (*closest_freecell) (int16 m, const struct block_list *bl, int16 *x, int16 *y, int type, int flag);
	//
	int (*quit) (struct map_session_data *sd);
	// npc
	bool (*addnpc) (int16 m,struct npc_data *nd);
	// map item
	int (*clearflooritem_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*removemobs_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*clearflooritem) (struct block_list* bl);
	int (*addflooritem) (const struct block_list *bl, struct item *item_data, int amount, int16 m, int16 x, int16 y, int first_charid, int second_charid, int third_charid, int flags, bool showdropeffect);
	// player to map session
	void (*addnickdb) (int charid, const char* nick);
	void (*delnickdb) (int charid, const char* nick);
	void (*reqnickdb) (struct map_session_data* sd,int charid);
	const char* (*charid2nick) (int charid);
	struct map_session_data* (*charid2sd) (int charid);

	void (*vforeachpc) (int (*func)(struct map_session_data* sd, va_list args), va_list args);
	void (*foreachpc) (int (*func)(struct map_session_data* sd, va_list args), ...);
	void (*vforeachmob) (int (*func)(struct mob_data* md, va_list args), va_list args);
	void (*foreachmob) (int (*func)(struct mob_data* md, va_list args), ...);
	void (*vforeachnpc) (int (*func)(struct npc_data* nd, va_list args), va_list args);
	void (*foreachnpc) (int (*func)(struct npc_data* nd, va_list args), ...);
	void (*vforeachregen) (int (*func)(struct block_list* bl, va_list args), va_list args);
	void (*foreachregen) (int (*func)(struct block_list* bl, va_list args), ...);
	void (*vforeachiddb) (int (*func)(struct block_list* bl, va_list args), va_list args);
	void (*foreachiddb) (int (*func)(struct block_list* bl, va_list args), ...);

	int (*vforeachinrange) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, va_list ap);
	int (*foreachinrange) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
	int (*vforeachinshootrange) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, va_list ap);
	int (*foreachinshootrange) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
	int (*vforeachinarea) (int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, va_list ap);
	int (*foreachinarea) (int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
	int (*vforcountinrange) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int count, int type, va_list ap);
	int (*forcountinrange) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int count, int type, ...);
	int (*vforcountinarea) (int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, va_list ap);
	int (*forcountinarea) (int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...);
	int (*vforeachinmovearea) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, va_list ap);
	int (*foreachinmovearea) (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, ...);
	int (*vforeachincell) (int (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int type, va_list ap);
	int (*foreachincell) (int (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int type, ...);
	int (*vforeachinpath) (int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, va_list ap);
	int (*foreachinpath) (int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...);
	int (*vforeachinmap) (int (*func)(struct block_list*,va_list), int16 m, int type, va_list args);
	int (*foreachinmap) (int (*func)(struct block_list*,va_list), int16 m, int type, ...);
	int (*forcountinmap) (int (*func)(struct block_list*,va_list), int16 m, int count, int type, ...);
	int (*vforeachininstance)(int (*func)(struct block_list*,va_list), int16 instance_id, int type, va_list ap);
	int (*foreachininstance)(int (*func)(struct block_list*,va_list), int16 instance_id, int type,...);

	struct map_session_data *(*id2sd) (int id);
	struct npc_data *(*id2nd) (int id);
	struct mob_data *(*id2md) (int id);
	struct flooritem_data *(*id2fi) (int id);
	struct chat_data *(*id2cd) (int id);
	struct skill_unit *(*id2su) (int id);
	struct pet_data *(*id2pd) (int id);
	struct homun_data *(*id2hd) (int id);
	struct mercenary_data *(*id2mc) (int id);
	struct elemental_data *(*id2ed) (int id);
	struct block_list *(*id2bl) (int id);
	bool (*blid_exists) (int id);
	int16 (*mapindex2mapid) (unsigned short map_index);
	int16 (*mapname2mapid) (const char* name);
	int (*mapname2ipport) (unsigned short name, uint32* ip, uint16* port);
	int (*eraseipport) (unsigned short map_index, uint32 ip, uint16 port);
	int (*eraseallipport) (void);
	void (*addiddb) (struct block_list *bl);
	void (*deliddb) (struct block_list *bl);
	/* */
	struct map_session_data * (*nick2sd) (const char *nick, bool allow_partial);
	struct mob_data * (*getmob_boss) (int16 m);
	struct mob_data * (*id2boss) (int id);
	uint32 (*race_id2mask) (int race);
	// reload config file looking only for npcs
	void (*reloadnpc) (bool clear);

	int (*check_dir) (enum unit_dir s_dir, enum unit_dir t_dir);
	enum unit_dir (*calc_dir) (const struct block_list *src, int16 x, int16 y);
	int (*get_random_cell) (struct block_list *bl, int16 m, int16 *x, int16 *y, int16 min_dist, int16 max_dist);
	int (*get_random_cell_in_range) (struct block_list *bl, int16 m, int16 *x, int16 *y, int16 x_range, int16 y_range);
	int (*random_dir) (struct block_list *bl, short *x, short *y); // [Skotlex]

	int (*cleanup_sub) (struct block_list *bl, va_list ap);

	int (*delmap) (const char *mapname);
	void (*flags_init) (void);

	bool (*iwall_set) (int16 m, int16 x, int16 y, int size, int8 dir, bool shootable, const char* wall_name);
	void (*iwall_get) (struct map_session_data *sd);
	bool (*iwall_remove) (const char *wall_name);

	int (*addmobtolist) (unsigned short m, struct spawn_data *spawn); // [Wizputer]
	void (*spawnmobs) (int16 m); // [Wizputer]
	void (*removemobs) (int16 m); // [Wizputer]
	//void (*do_reconnect_map) (void); //Invoked on map-char reconnection [Skotlex] Note used but still keeping it, just in case
	void (*addmap2db) (struct map_data *m);
	void (*removemapdb) (struct map_data *m);
	void (*clean) (int i);

	void (*do_shutdown) (void);

	int (*freeblock_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*searchrandfreecell) (int16 m, const struct block_list *bl, int16 *x, int16 *y, int stack);
	int (*count_sub) (struct block_list *bl, va_list ap);
	struct DBData (*create_charid2nick) (union DBKey key, va_list args);
	int (*removemobs_sub) (struct block_list *bl, va_list ap);
	struct mapcell (*gat2cell) (int gat);
	int (*cell2gat) (struct mapcell cell);
	int (*getcellp) (struct map_data *m, const struct block_list *bl, int16 x, int16 y, cell_chk cellchk);
	void (*setcell) (int16 m, int16 x, int16 y, cell_t cell, bool flag);
	int (*sub_getcellp) (struct map_data *m, const struct block_list *bl, int16 x, int16 y, cell_chk cellchk);
	void (*sub_setcell) (int16 m, int16 x, int16 y, cell_t cell, bool flag);
	void (*iwall_nextxy) (int16 x, int16 y, int8 dir, int pos, int16 *x1, int16 *y1);
	int (*eraseallipport_sub) (union DBKey key, struct DBData *data, va_list va);
	bool (*readfromcache) (struct map_data *m);
	bool (*readfromcache_v1) (FILE *fp, struct map_data *m, unsigned int file_size);
	int (*addmap) (const char *mapname);
	void (*delmapid) (int id);
	void (*zone_db_clear) (void);
	void (*list_final) (void);
	int (*waterheight) (char *mapname);
	int (*readgat) (struct map_data *m);
	int (*readallmaps) (void);
	bool (*config_read) (const char *filename, bool imported);
	bool (*config_read_console) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read_connection) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read_inter) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read_database) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read_map_list) (const char *filename, struct config_t *config, bool imported);

	bool (*read_npclist) (const char *filename, bool imported);
	bool (*inter_config_read) (const char *filename, bool imported);
	bool (*inter_config_read_database_names) (const char *filename, const struct config_t *config, bool imported);
	bool (*inter_config_read_connection) (const char *filename, const struct config_t *config, bool imported);
	bool (*setting_lookup_const) (const struct config_setting_t *setting, const char *name, int *value);
	bool (*setting_lookup_const_mask) (const struct config_setting_t *setting, const char *name, int *value);
	int (*sql_init) (void);
	int (*sql_close) (void);
	bool (*zone_mf_cache) (int m, char *flag, char *params);
	int (*zone_str2itemid) (const char *name);
	unsigned short (*zone_str2skillid) (const char *name);
	enum bl_type (*zone_bl_type) (const char *entry, enum map_zone_skill_subtype *subtype);
	void (*read_zone_db) (void);
	int (*db_final) (union DBKey key, struct DBData *data, va_list ap);
	int (*nick_db_final) (union DBKey key, struct DBData *data, va_list args);
	int (*cleanup_db_sub) (union DBKey key, struct DBData *data, va_list va);
	int (*abort_sub) (struct map_session_data *sd, va_list ap);
	void (*update_cell_bl) (struct block_list *bl, bool increase);
	int (*get_new_bonus_id) (void);
	bool (*add_questinfo) (int m, struct npc_data *nd);
	bool (*remove_questinfo) (int m, struct npc_data *nd);
	struct map_zone_data *(*merge_zone) (struct map_zone_data *main, struct map_zone_data *other);
	void (*zone_clear_single) (struct map_zone_data *zone);
	void (*lock_check) (const char *file, const char *func, int line, int lock_count);
};

#ifdef HERCULES_CORE
void map_defaults(void);
void mapit_defaults(void);
#endif // HERCULES_CORE

HPShared struct mapit_interface *mapit;
HPShared struct map_interface *map;

#endif /* MAP_MAP_H */
