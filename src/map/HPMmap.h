// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef MAP_HPMMAP_H
#define MAP_HPMMAP_H

#ifndef HERCULES_CORE
#error You should never include HPMmap.h from a plugin.
#endif

#include "../common/cbasetypes.h"
#include "../map/atcommand.h"
#include "../common/HPM.h"

struct hplugin;
struct map_session_data;

bool HPM_map_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr);

bool HPM_map_add_atcommand(char *name, AtCommandFunc func);
void HPM_map_atcommands(void);

void HPM_map_plugin_load_sub(struct hplugin *plugin);

void HPM_map_do_final(void);

void HPM_map_add_group_permission(unsigned int pluginID, char *name, unsigned int *mask);

void HPM_map_do_init(void);

#endif /* MAP_HPMMAP_H */
