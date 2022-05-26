/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
 * Copyright (C) 2022 Andrei Karas (4144)
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

#define CONFIG_START_VARS1(varName) \
	struct varName ## _config varName ## _vars;
#define CONFIG_START_VARS0(varName) CONFIG_START_VARS1(varName)
#define CONFIG_START_VARS CONFIG_START_VARS0(CONFIG_VARS)

#define CONFIG_START1(varName) \
	struct varName ## _config varName ## _vars; \
	static const struct config_data varName ## _data[] = {
#define CONFIG_START0(varName) CONFIG_START1(varName)
#define CONFIG_START CONFIG_START0(CONFIG_VARS)

#define CONFIG1(type, name, varName, def, min, max) \
	{ #name, config_type_ ## type, &varName ## _vars.name, NULL,                   def, NULL, min, max },
#define CONFIG0(type, name, varName, def, min, max) CONFIG1(type, name, varName, def, min, max)
#define CONFIG(type, name, def, min, max) CONFIG0(type, name, CONFIG_VARS, def, min, max)

#define CONFIGSTR1(type, name, varName, def, min, max) \
	{ #name, config_type_ ## type, NULL,                   &varName ## _vars.name, 0,   def,  min, max },
#define CONFIGSTR0(type, name, varName, def, min, max) CONFIGSTR1(type, name, varName, def, min, max)
#define CONFIGSTR(type, name, def, min, max) CONFIGSTR0(type, name, CONFIG_VARS, def, min, max)

#define CONFIG_END \
	{ NULL,  config_type_int,      NULL,                   NULL,                   0,   NULL, 0,   0   } \
};
