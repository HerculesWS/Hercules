/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2023-2023 Hercules Dev Team
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
#ifndef MAP_GOLDPC_H
#define MAP_GOLDPC_H

#include "common/hercules.h"

#include "map/pc.h" // struct map_session_data

#include <stdarg.h>

// The limit of seconds the GoldPC timer can display (hardcoded in client)
#define GOLDPC_MAX_TIME 3600

// Maximum number of GoldPC poins you may have (hardcoded in client)
#define GOLDPC_MAX_POINTS 300

// Account variables for persistence
#define GOLDPC_POINTS_VAR "#GOLDPC_POINTS"
#define GOLDPC_PLAYTIME_VAR "#GOLDPC_PLAYTIME"

/**
 * Represents a setting of GoldPC.
 */
struct goldpc_mode {
	int id; //< mode id (sent to client for display)
	int required_time; //< required time to gain points in this mode
	int points; //< points granted once required_time seconds passes
	int time_offset; //< seconds to compensate required_time difference to GOLDPC_MAX_TIME
};

/**
 * goldpc.c Interface
 **/
struct goldpc_interface {
	/* vars */
	struct DBMap *db; // int mode_id -> struct goldpc_mode *

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);
	struct goldpc_mode *(*exists) (int id);

	/* database */
	void (*read_db_libconfig) (void);
	bool (*read_db_libconfig_sub) (const struct config_setting_t *it, int n, const char *source);
	bool (*read_db_validate) (struct goldpc_mode *mode, const char *source);

	/* process */
	int (*timeout) (int tid, int64 tick, int id, intptr_t data);
	void (*addpoints) (struct map_session_data *sd, int points);
	void (*load) (struct map_session_data *sd);
	void (*start) (struct map_session_data *sd);
	void (*stop) (struct map_session_data *sd);
};

#ifdef HERCULES_CORE
void goldpc_defaults(void);
#endif // HERCULES_CORE

HPShared struct goldpc_interface *goldpc;

#endif // MAP_GOLDPC_H
