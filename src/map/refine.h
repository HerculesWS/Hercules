/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2019  Hercules Dev Team
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

#ifndef MAP_REFINE_H
#define MAP_REFINE_H

/** @file
 * Refine Interface.
 **/
#include "common/hercules.h"

/* Defines */
/**
* Max Refine available to your server
* Changing this limit requires edits to refine_db.conf
**/
#ifdef RENEWAL
	#define MAX_REFINE 20
#else
	#define MAX_REFINE 10
#endif

/* Forward Declarations */
struct refine_interface_private;

/* Enums */
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

enum refine_chance_type {
	REFINE_CHANCE_TYPE_NORMAL     = 0, // Normal Chance
	REFINE_CHANCE_TYPE_ENRICHED   = 1, // Enriched Ore Chance
	REFINE_CHANCE_TYPE_E_NORMAL   = 2, // Event Normal Ore Chance
	REFINE_CHANCE_TYPE_E_ENRICHED = 3, // Event Enriched Ore Chance
	REFINE_CHANCE_TYPE_MAX
};

/* Structures */
struct s_refine_info {
	int chance[REFINE_CHANCE_TYPE_MAX][MAX_REFINE]; // success chance
	int bonus[MAX_REFINE];                          // cumulative fixed bonus damage
	int randombonus_max[MAX_REFINE];                // cumulative maximum random bonus damage
};

struct refine_interface_dbs {
	struct s_refine_info refine_info[REFINE_TYPE_MAX];
};

/**
 * Refine Interface
 **/
struct refine_interface {
	struct refine_interface_private *p;
	struct refine_interface_dbs *dbs;

	/**
	 * Initialize refine system
	 * @param minimal sets refine system to minimal mode in which it won't load or initialize itself
	 * @return returns 0 in-case of success 1 otherwise
	 **/
	int (*init)(bool minimal);

	/**
	 * Finalize refine system
	 **/
	void (*final)(void);

	/**
	 * Get the chance to upgrade a piece of equipment.
	 * @param wlv The weapon type of the item to refine (see see enum refine_type)
	 * @param refine The target refine level
	 * @return The chance to refine the item, in percent (0~100)
	 **/
	int (*get_refine_chance) (enum refine_type wlv, int refine_level, enum refine_chance_type type);
};

#ifdef HERCULES_CORE
void refine_defaults(void);
#endif

HPShared struct refine_interface *refine;
#endif