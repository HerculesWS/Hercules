/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
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
#ifndef MAP_MAPREG_H
#define MAP_MAPREG_H

#include "map/script.h"
#include "common/db.h"
#include "common/hercules.h"

/** Forward Declarations **/
struct config_setting_t;
struct eri;

#ifndef MAPREG_AUTOSAVE_INTERVAL
#define MAPREG_AUTOSAVE_INTERVAL (300 * 1000) //!< Interval for auto-saving permanent global variables to the database in milliseconds.
#endif /** MAPREG_AUTOSAVE_INTERVAL **/

/** Global variable structure. **/
struct mapreg_save {
	int64 uid;         //!< The variable's unique ID.
	union value {      //!< The variable's value container.
		int i;     //!< The variable's integer value.
		char *str; //!< The variable's string value.
	} u;
	bool is_string;    //!< Whether the variable's value is a string.
	bool save;         //!< Whether the variable's save operation is pending.
};

/** The mapreg interface structure. **/
struct mapreg_interface {
	/** Interface variables. **/
	struct eri *ers;    //!< Entry manager for global variables.
	struct reg_db regs; //!< Generic database for global variables.
	bool dirty;         //!< Whether there are modified global variables to be saved.
	bool skip_insert;   //!< Whether to skip inserting the variable into the SQL database in mapreg_set_*_db().
	char num_db[32];    //!< Name of SQL table which holds permanent global integer variables.
	char str_db[32];    //!< Name of SQL table which holds permanent global string variables.

	/** Interface functions. **/
	int (*readreg) (int64 uid);
	char *(*readregstr) (int64 uid);
	bool (*set_num_db) (int64 uid, const char *name, unsigned int index, int value);
	bool (*delete_num_db) (int64 uid, const char *name, unsigned int index);
	bool (*setreg) (int64 uid, int val);
	bool (*set_str_db) (int64 uid, const char *name, unsigned int index, const char *value);
	bool (*delete_str_db) (int64 uid, const char *name, unsigned int index);
	bool (*setregstr) (int64 uid, const char *str);
	void (*load_num_db) (void);
	void (*load_str_db) (void);
	void (*load) (void);
	void (*save_num_db) (const char *name, unsigned int index, int value);
	void (*save_str_db) (const char *name, unsigned int index, const char *value);
	void (*save) (void);
	int (*save_timer) (int tid, int64 tick, int id, intptr_t data);
	int (*destroyreg) (union DBKey key, struct DBData *data, va_list ap);
	void (*reload) (void);
	bool (*config_read_registry) (const char *filename, const struct config_setting_t *config, bool imported);
	void (*final) (void);
	void (*init) (void);
};

#ifdef HERCULES_CORE
void mapreg_defaults(void);
#endif /** HERCULES_CORE **/

HPShared struct mapreg_interface *mapreg;

#endif /** MAP_MAPREG_H **/
