// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef CHAR_HPMCHAR_H
#define CHAR_HPMCHAR_H

#ifndef HERCULES_CORE
#error You should never include HPMchar.h from a plugin.
#endif

#include "common/cbasetypes.h"
#include "common/HPM.h"

struct hplugin;

bool HPM_char_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **store);

void HPM_char_plugin_load_sub(struct hplugin *plugin);

void HPM_char_do_final(void);

void HPM_char_do_init(void);

#endif /* CHAR_HPMCHAR_H */
