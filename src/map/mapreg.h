/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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

#include "map/script.h" // struct reg_db
#include "common/hercules.h"
#include "common/db.h"

/* Forward Declarations */
struct config_setting_t; // common/conf.h
struct eri;

/** Container for a mapreg value */
struct mapreg_save {
	int64 uid;         ///< Unique ID
	union {
		int i;     ///< Numeric value
		char *str; ///< String value
	} u;
	bool is_string;    ///< true if it's a string, false if it's a number
	bool save;         ///< Whether a save operation is pending
};

struct mapreg_interface {
	struct reg_db regs;
	/* */
	bool skip_insert;
	/* */
	struct eri *ers; //[Ind/Hercules]
	/* */
	char num_db[32]; //!< Name of SQL table which holds permanent global integer variables.
	char str_db[32]; //!< Name of SQL table which holds permanent global string variables.
	/* */
	bool dirty; ///< Whether there are modified regs to be saved
	/* */
	void (*init) (void);
	void (*final) (void);
	/* */
	int (*readreg) (int64 uid);
	char* (*readregstr) (int64 uid);
	bool (*set_num_db) (int64 uid, const char *name, unsigned int index, int value);
	bool (*delete_num_db) (int64 uid, const char *name, unsigned int index);
	bool (*setreg) (int64 uid, int val);
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
};

#ifdef HERCULES_CORE
void mapreg_defaults(void);
#endif // HERCULES_CORE

HPShared struct mapreg_interface *mapreg;

#endif /* MAP_MAPREG_H */
