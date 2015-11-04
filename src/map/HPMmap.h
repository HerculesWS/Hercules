/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
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
#ifndef MAP_HPMMAP_H
#define MAP_HPMMAP_H

#ifndef HERCULES_CORE
#error You should never include HPMmap.h from a plugin.
#endif

#include "map/atcommand.h"
#include "common/cbasetypes.h"
#include "common/HPM.h"

struct hplugin;
struct map_session_data;

bool HPM_map_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize);

bool HPM_map_add_atcommand(char *name, AtCommandFunc func);
void HPM_map_atcommands(void);

void HPM_map_plugin_load_sub(struct hplugin *plugin);

void HPM_map_do_final(void);

void HPM_map_add_group_permission(unsigned int pluginID, char *name, unsigned int *mask);

void HPM_map_do_init(void);

#endif /* MAP_HPMMAP_H */
