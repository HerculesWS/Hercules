/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2014-2015  Hercules Dev Team
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
#ifndef CHAR_HPMCHAR_H
#define CHAR_HPMCHAR_H

#ifndef HERCULES_CORE
#error You should never include HPMchar.h from a plugin.
#endif

#include "common/cbasetypes.h"
#include "common/HPM.h"

struct hplugin;

bool HPM_char_data_store_validate(enum HPluginDataTypes type, struct hplugin_data_store **storeptr, bool initialize);

void HPM_char_plugin_load_sub(struct hplugin *plugin);

void HPM_char_do_final(void);

void HPM_char_do_init(void);

#endif /* CHAR_HPMCHAR_H */
