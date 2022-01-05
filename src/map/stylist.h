/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2018-2022 Hercules Dev Team
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
#ifndef MAP_STYLIST_H
#define MAP_STYLIST_H

#include "common/hercules.h"
#include "map/map.h" // LOOK_MAX

struct map_session_data;

/* Maximum available types for stylist */
#ifndef MAX_STYLIST_TYPE
#define MAX_STYLIST_TYPE LOOK_MAX
#endif

/* Stylist data [Asheraf/Hercules]*/
struct stylist_data_entry {
	int16 id;
	int32 zeny;
	int itemid;
	int boxid;
	bool allow_doram;
};

/**
 * stylist.c Interface
 **/
struct stylist_interface {
	VECTOR_DECL(struct stylist_data_entry) data[MAX_STYLIST_TYPE];

	void (*init) (bool minimal);
	void (*final) (void);

	void (*vector_init) (void);
	void (*vector_clear) (void);

	bool (*read_db_libconfig) (void);
	bool (*read_db_libconfig_sub) (struct config_setting_t *it, int idx, const char *source);

	void (*request_style_change) (struct map_session_data *sd, int type, int16 idx, bool isitem);
	bool (*validate_requirements) (struct map_session_data *sd, int type, int16 idx);
	void (*send_rodexitem) (struct map_session_data *sd, int itemid);

};

#ifdef HERCULES_CORE
void stylist_defaults(void);
#endif // HERCULES_CORE

HPShared struct stylist_interface *stylist; ///< Pointer to the stylist interface.

#endif /* MAP_STYLIST_H */
