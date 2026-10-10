/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2026 Hercules Dev Team
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
#ifndef MAP_HPMI_MAP_H
#define MAP_HPMI_MAP_H

#include "common/HPMi.h"

class hpm_plugin_imap : public hpm_plugin_i
{
  public:
	hpm_plugin_imap(const char *a_name, enum server_types a_type, const char *a_version, const char *the_req_version,
	                bool with_hpmhooking, const struct s_HPMDataCheck *the_HPMDataCheck,
	                unsigned int the_HPMDataCheckLen, int the_HPMDataCheckVer, const struct HPMi_interface *the_HPMi)
	    : hpm_plugin_i(a_name, a_type, a_version, the_req_version, with_hpmhooking, the_HPMDataCheck,
	                   the_HPMDataCheckLen, the_HPMDataCheckVer, the_HPMi)
	{
	}

	bool add_atcommand(const char *cname, bool (*func)(const int fd, struct map_session_data *sd, const char *command,
	                                                   const char *message, struct AtCommandInfo *info))
	{
		nullpo_retr(false, HPMi->addCommand);
		return HPMi->addCommand(cname, func);
	}

	bool add_script_command(const char *cname, const char *args, bool (*func)(struct script_state *st),
	                        bool is_deprecated)
	{
		nullpo_retr(false, HPMi->addScript);
		return HPMi->addScript(cname, args, func, is_deprecated);
	}

	bool add_battle_conf(const char *bcname, void (*parse_func)(const char *key, const char *val),
	                     int (*return_func)(const char *key), bool required)
	{
		return this->add_conf(HPCT_BATTLE, bcname, parse_func, return_func, required);
	}

	bool add_map_inter_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_MAP_INTER, bcname, parse_func, nullptr, false);
	}

	bool add_script_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_SCRIPT, bcname, parse_func, nullptr, false);
	}
};

#endif /* MAP_HPMI_MAP_H */
