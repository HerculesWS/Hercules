/**
* This file is part of Hercules.
* https://herc.ws - https://github.com/HerculesWS/Hercules
*
* Copyright (C) 2019-2020 Hercules Dev Team
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
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MAP_REFINE_H
#define MAP_REFINE_H

/** @file
 * Refine Interface.
 **/
#include "common/hercules.h"
#include "common/mmo.h"

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

enum refine_ui_failure_behavior {
	REFINE_FAILURE_BEHAVIOR_DESTROY,
	REFINE_FAILURE_BEHAVIOR_KEEP,
	REFINE_FAILURE_BEHAVIOR_DOWNGRADE
};

/* Structure */
struct s_refine_requirement {
	int blacksmith_blessing;
	int req_count;
	unsigned int announce;

	struct {
		int nameid;
		int cost;
		enum refine_chance_type type;
		enum refine_ui_failure_behavior failure_behavior;
	} req[MAX_REFINE_REQUIREMENTS];
};

/**
 * Refine Interface
 **/
struct refine_interface {
	struct refine_interface_private *p;

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

	/**
	 * Gets the attack/deffense bonus for the given equipment type and refine level
	 * @param equipment_type the equipment type
	 * @param refine_level the equipment refine level
	 * @return returns the bonus from refine db
	 **/
	int (*get_bonus) (enum refine_type equipment_type, int refine_level);

	/**
	* Gets the maximum attack/deffense random bonus for the given equipment type and refine level
	* @param equipment_type the equipment type
	* @param refine_level the equipment refine level
	* @return returns the bonus from refine db
	**/
	int(*get_randombonus_max) (enum refine_type equipment_type, int refine_level);

	/**
	 * Validates and send Item addition packet to the client for refinery UI
	 * @param sd player session data.
	 * @param item_index the requested item index in inventory.
	 **/
	void (*refinery_add_item) (struct map_session_data *sd, int item_index);

	/**
	 * Processes an refine request through Refinery UI
	 * @param sd player session data
	 * @param item_index the index of the requested item
	 * @param material_id the refine material chosen by player
	 * @param use_blacksmith_blessing sets either if blacksmith blessing is requested to be used or not
	 **/
	void (*refinery_refine_request) (struct map_session_data *sd, int item_index, int material_id, bool use_blacksmith_blessing);
};

#ifdef HERCULES_CORE
void refine_defaults(void);
#endif

HPShared struct refine_interface *refine;
#endif
