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

/// Base author: Haru <haru@dotalux.com>
/// mapquit() script command

#include "common/hercules.h"
#include "map/map.h"
#include "map/script.h"

#include "common/HPMDataCheck.h"

HPExport struct hplugin_info pinfo = {
	"script_mapquit",    // Plugin name
	SERVER_TYPE_MAP,     // Which server types this plugin works with?
	"0.1",               // Plugin version
	HPM_VERSION,         // HPM Version (don't change, macro is automatically updated)
};

BUILDIN(mapquit) {
	if (script_hasdata(st, 2)) {
		map->retval = script_getnum(st, 2);
	}
	map->do_shutdown();
	return true;
}
HPExport void server_preinit(void) {
}
HPExport void plugin_init(void) {
	addScriptCommand("mapquit", "?", mapquit);
}
