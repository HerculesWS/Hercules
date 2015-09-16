// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef LOGIN_HPMLOGIN_H
#define LOGIN_HPMLOGIN_H

#ifndef HERCULES_CORE
#error You should never include HPMlogin.h from a plugin.
#endif

#include "common/cbasetypes.h"
#include "common/HPM.h"

struct hplugin;

bool HPM_login_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **store);

void HPM_login_plugin_load_sub(struct hplugin *plugin);

void HPM_login_do_final(void);

void HPM_login_do_init(void);

#endif /* LOGIN_HPMLOGIN_H */
