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

/**
 * Refine Private Interface
 **/
struct refine_interface_private {
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
};

#endif
