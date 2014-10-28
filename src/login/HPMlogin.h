// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#ifndef LOGIN_HPMLOGIN_H
#define LOGIN_HPMLOGIN_H

#include "../common/cbasetypes.h"
#include "../common/HPM.h"

struct hplugin;

bool HPM_login_grabHPData(struct HPDataOperationStorage *ret, enum HPluginDataTypes type, void *ptr);

void HPM_login_plugin_load_sub(struct hplugin *plugin);

void HPM_login_do_final(void);

void HPM_login_do_init(void);

#endif /* LOGIN_HPMLOGIN_H */
