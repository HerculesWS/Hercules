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
#ifndef CHAR_HPMI_CHAR_H
#define CHAR_HPMI_CHAR_H

#include "common/HPMi.h"

class hpm_plugin_ichar : public hpm_plugin_i
{
  public:
	hpm_plugin_ichar(const char *a_name, enum server_types a_type, const char *a_version, const char *the_req_version,
	                 bool with_hpmhooking, const struct s_HPMDataCheck *the_HPMDataCheck,
	                 unsigned int the_HPMDataCheckLen, int the_HPMDataCheckVer, const struct HPMi_interface *the_HPMi)
	    : hpm_plugin_i(a_name, a_type, a_version, the_req_version, with_hpmhooking, the_HPMDataCheck,
	                   the_HPMDataCheckLen, the_HPMDataCheckVer, the_HPMi)
	{
	}

	bool add_char_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_CHAR, bcname, parse_func, nullptr, false);
	}

	bool add_char_inter_conf(const char *bcname, void (*parse_func)(const char *key, const char *val))
	{
		return this->add_conf(HPCT_CHAR_INTER, bcname, parse_func, nullptr, false);
	}
};

#endif /* CHAR_HPMI_CHAR_H */
