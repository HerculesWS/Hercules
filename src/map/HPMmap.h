// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef _HPM_MAP_
#define _HPM_MAP_

#include "../common/cbasetypes.h"

struct hplugin;
struct map_session_data;

void HPM_map_addToMSD(struct map_session_data *sd, void *data, unsigned int id, unsigned int type, bool autofree);
void *HPM_map_getFromMSD(struct map_session_data *sd, unsigned int id, unsigned int type);
void HPM_map_removeFromMSD(struct map_session_data *sd, unsigned int id, unsigned int type);

void HPM_map_plugin_load_sub(struct hplugin *plugin);

#endif /* _HPM_MAP_ */
