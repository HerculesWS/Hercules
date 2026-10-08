/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2014-2026 Hercules Dev Team
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

class hpm_plugin_mapquit : public hpm_plugin_i
{
  public:
	void init() override;

	void final() override
	{
	}

	void server_online() override
	{
	}

	void server_post_final() override
	{
	}

	void server_preinit() override
	{
	}
};

hpm_plugin_mapquit plugin;

HPM_DECLARE_PLUGIN(plugin, "script_mapquit", "0.1")

BUILDIN(mapquit)
{
	if (script_hasdata(st, 2)) {
		map->retval = script_getnum(st, 2);
	}
	map->do_shutdown();
	return true;
}

void hpm_plugin_mapquit::init()
{
	addScriptCommand("mapquit", "?", mapquit);
}
