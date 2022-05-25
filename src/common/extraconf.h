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
#ifndef COMMON_EXTRACONF_H
#define COMMON_EXTRACONF_H

#include "common/hercules.h"

struct config_t;
struct config_setting_t;

struct config_data {
	const char* str;
	int* val;
	int defval;
	int min;
	int max;
};

// defines
#define CONFIG(type, name, def, min, max) type name;

// structs
struct emblems_config {
#include "common/config/emblems.h"
};

// undefines
#undef CONFIG

/**
 * The extraconf interface
 **/
struct extraconf_interface {
	char *EMBLEMS_CONF_NAME;

	struct emblems_config *emblems;

	void (*init) (void);
	void (*final) (void);

	bool (*read_conf_file) (const char *filename, bool imported, const char *node, const struct config_data *conf_vars);
	bool (*read_conf) (const char *filename, bool imported, struct config_t *config, const char *node, const struct config_data *conf_vars);
	bool (*read_vars) (const char *filename, bool imported, struct config_t *config, const char *node, const struct config_data *conf_vars);
	bool (*set_var) (struct config_data *conf_var, int val);
	bool (*read_emblems) (void);
};

#ifdef HERCULES_CORE
void extraconf_defaults(void);
#endif // HERCULES_CORE

HPShared struct extraconf_interface *extraconf;

#endif // COMMON_EXTRACONF_H
