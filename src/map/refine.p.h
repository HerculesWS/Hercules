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

#ifndef MAP_REFINE_P_H
#define MAP_REFINE_P_H

/** @file
 * Private header for the refine interface.
 **/

#include "refine.h"
#include "common/conf.h"
/* Enums */
enum refine_announce_condition {
	REFINE_ANNOUNCE_SUCCESS = 0x1,
	REFINE_ANNOUNCE_FAILURE = 0x2,
	REFINE_ANNOUNCE_ALWAYS = REFINE_ANNOUNCE_SUCCESS | REFINE_ANNOUNCE_FAILURE,
};

/* Structures */
struct s_refine_info {
	int chance[REFINE_CHANCE_TYPE_MAX][MAX_REFINE]; //< success chance
	int bonus[MAX_REFINE];                          //< cumulative fixed bonus damage
	int randombonus_max[MAX_REFINE];                //< cumulative maximum random bonus damage
	struct s_refine_requirement refine_requirements[MAX_REFINE]; //< The requirements used for refinery UI
};

struct refine_interface_dbs {
	struct s_refine_info refine_info[REFINE_TYPE_MAX];
};

/**
 * Refine Private Interface
 **/
struct refine_interface_private {
	struct refine_interface_dbs *dbs;

	/**
	 * Processes a refine_db.conf entry.
	 *
	 * @param r      Libconfig setting entry. It is expected to be valid and it
	 *               won't be freed (it is care of the caller to do so if
	 *               necessary)
	 * @param n      Ordinal number of the entry, to be displayed in case of
	 *               validation errors.
	 * @param source Source of the entry (file name), to be displayed in case of
	 *               validation errors.
	 * @return # of the validated entry, or 0 in case of failure.
	 **/
	int (*readdb_refine_libconfig_sub) (struct config_setting_t *r, const char *name, const char *source);

	/**
	 * Reads from a libconfig-formatted refine_db.conf file.
	 *
	 * @param *filename File name, relative to the database path.
	 * @return The number of found entries.
	 **/
	int (*readdb_refine_libconfig) (const char *filename);

	/**
	 * Converts refine database announce behvaior string to enum refine_announce_condition
	 * @param str the string to convert
	 * @param result pointer to where the converted value will be held
	 * @return true on success, false otherwise.
	**/
	bool (*announce_behavior_string2enum) (const char *str, unsigned int *result);

	/**
	 * Converts refine database failure behvaior string to enum refine_ui_failure_behavior
	 * @param str the string to convert
	 * @param result pointer to where the converted value will be held
	 * @return true on success, false otherwise.
	 **/
	bool (*failure_behavior_string2enum) (const char *str, enum refine_ui_failure_behavior *result);

	/**
	 * Processes a refine_db.conf RefineryUISettings items entry.
	 *
	 * @param elem   Libconfig setting entry. It is expected to be valid and it
	 *               won't be freed (it is care of the caller to do so if
	 *               necessary)
	 * @param req    a pointer to requirements struct to fill with the item data
	 * @param name   the current element name
	 * @param source Source of the entry (file name), to be displayed in case of
	 *               validation errors.
	 * @return true on success, false otherwise.
	 **/
	bool (*readdb_refinery_ui_settings_items) (const struct config_setting_t *elem, struct s_refine_requirement *req, const char *name, const char *source);

	/**
	 * Processes a refine_db.conf RefineryUISettings entry.
	 *
	 * @param elem   Libconfig setting entry. It is expected to be valid and it
	 *               won't be freed (it is care of the caller to do so if
	 *               necessary)
	 * @param type   the type index in refine database to fill the data
	 * @param name   the current element name
	 * @param source Source of the entry (file name), to be displayed in case of
	 *               validation errors.
	 * @return true on success, false otherwise.
	 **/
	bool (*readdb_refinery_ui_settings_sub) (const struct config_setting_t *elem, int type, const char *name, const char *source);

	/**
	 * Reads a refine_db.conf RefineryUISettings entry and sends it to be processed.
	 *
	 * @param r      Libconfig setting entry. It is expected to be valid and it
	 *               won't be freed (it is care of the caller to do so if
	 *               necessary)
	 * @param type   the type index in refine database to fill the data
	 * @param name   the current element name
	 * @param source Source of the entry (file name), to be displayed in case of
	 *               validation errors.
	 * @return true on success, false otherwise.
	 **/
	int (*readdb_refinery_ui_settings) (const struct config_setting_t *r, int type, const char *name, const char *source);

	/**
	 * Checks if a given item in player's inventory is refineable.
	 * @param sd player session data.
	 * @param item_index the item index in player's inventory.
	 * @return true if item is refineable, false otherwise.
	 **/
	bool (*is_refinable) (struct map_session_data *sd, int item_index);
};

#endif
