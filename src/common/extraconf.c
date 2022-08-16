/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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
#define HERCULES_CORE

#include "common/extraconf.h"

#include "common/core.h"
#include "common/conf.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/memmgr.h"

#include <string.h>

#define CONFIG_DEBUG

// defines for vars
#include "common/config/defc.h"

// include vars files
#define CONFIG_VARS emblems
CONFIG_START
#include "common/config/emblems.h"
CONFIG_END
#undef CONFIG_VARS

// remove all temporary defines
#include "common/config/undefc.h"

/* interface source */
static struct extraconf_interface extraconf_s;
struct extraconf_interface *extraconf;

static bool extraconf_read_conf_file(const char *filename, bool imported, const char *node, const struct config_data *conf_vars)
{
	nullpo_retr(false, filename);
	nullpo_retr(false, node);
	nullpo_retr(false, conf_vars);

	struct config_t config;

	if (!libconfig->load_file(&config, filename))
		return false;

	bool retval = extraconf->read_conf(filename, imported, &config, node, conf_vars);
	libconfig->destroy(&config);
	return retval;
}

static bool extraconf_read_conf(const char *filename, bool imported, struct config_t *config, const char *node, const struct config_data *conf_vars)
{
	nullpo_retr(false, filename);
	nullpo_retr(false, node);
	nullpo_retr(false, conf_vars);

	struct config_setting_t *setting = NULL;
	if ((setting = libconfig->lookup(config, node)) == NULL) {
		if (imported)
			return true;
		ShowError("extraconf_read_conf: %s was not found in %s!\n", node, filename);
		return false;
	}
	bool retval = extraconf->read_vars(filename, imported, config, node, conf_vars);
	if (!retval)
		return retval;

	// import should overwrite any previous configuration, so it should be called last
	const char *import = NULL;
	if (libconfig->lookup_string(config, "import", &import) == CONFIG_TRUE) {
		if (strcmp(import, filename) == 0 || strcmp(import, filename) == 0) {
			ShowWarning("extraconf_read_conf: Loop detected! Skipping 'import'...\n");
		} else {
			if (!extraconf->read_conf_file(import, true, node, conf_vars))
				retval = false;
		}
	}

	return retval;
}

static bool extraconf_read_vars(const char *filename, bool imported, struct config_t *config, const char *node, const struct config_data *conf_vars)
{
	nullpo_retr(false, filename);
	nullpo_retr(false, config);
	nullpo_retr(false, conf_vars);

	bool retval = true;

	int i = 0;
	while (conf_vars[i].str != NULL) {
		char config_name[256];
		snprintf(config_name, sizeof config_name, "%s/%s", node, conf_vars[i].str);

		struct config_setting_t *setting = libconfig->lookup(config, config_name);
		if (setting == NULL) {
			if (!imported) {
				ShowWarning("Missing configuration '%s' in file %s!\n", config_name, filename);
				retval = false;
			}
			i++;
			continue;
		}

		const int type = config_setting_type(setting);
		int val = 0;
		const char *valStr = NULL;
		switch (type) {
		case CONFIG_TYPE_INT:
			val = libconfig->setting_get_int(setting);
			break;
		case CONFIG_TYPE_BOOL:
			val = libconfig->setting_get_bool(setting);
			break;
		case CONFIG_TYPE_STRING:
			valStr = libconfig->setting_get_string(setting);
			break;
		default: // Unsupported type
			ShowWarning("Setting %s has unsupported type %d, ignoring...\n", config_name, type);
			retval = false;
			i++;
			continue;
		}

		if (type == CONFIG_TYPE_STRING) {
			if (!extraconf->set_var_str(&conf_vars[i], valStr))
				retval = false;
		} else {
			if (!extraconf->set_var(&conf_vars[i], val))
				retval = false;
		}
		i++;
	}

	return retval;
}

static bool extraconf_set_var(struct config_data *conf_var, int value)
{
	nullpo_retr(false, conf_var);

	if (conf_var->type == config_type_str) {
		ShowWarning("Setting %s has wrong type string, but need non string, ignoring...\n", conf_var->str);
		return false;
	}

	if (value < conf_var->min || value > conf_var->max) {
		ShowWarning("Value for setting '%s': %d is invalid (min:%d max:%d)! Defaulting to %d...\n",
				conf_var->str, value, conf_var->min, conf_var->max, conf_var->defval);
		value = conf_var->defval;
	}
#ifdef CONFIG_DEBUG
	ShowInfo("Read config setting '%s': %d\n", conf_var->str, value);
#endif
	*conf_var->val = value;
	return true;
}

static bool extraconf_set_var_str(struct config_data *conf_var, const char *val)
{
	nullpo_retr(false, conf_var);
	nullpo_retr(false, val);

	const enum config_type varType = conf_var->type;
	if (varType != config_type_str) {
		ShowWarning("Setting %s has wrong type %d, but need string, ignoring...\n", conf_var->str, (int)varType);
		return false;
	}

	const int len = strlen(val);
	if ((conf_var->min != 0 && len < conf_var->min) || (conf_var->max != 0 && len > conf_var->max)) {
		ShowWarning("Value for setting '%s': '%s' is invalid (min:%d max:%d)! Defaulting to '%s'...\n",
				conf_var->str, val, conf_var->min, conf_var->max, conf_var->defval_str);
		val = conf_var->defval_str;
	}
#ifdef CONFIG_DEBUG
	ShowInfo("Read config setting '%s': '%s'\n", conf_var->str, val);
#endif
	*conf_var->val_str = val;
	return true;
}

static bool extraconf_read_emblems(void)
{
	return extraconf->read_conf_file(extraconf->EMBLEMS_CONF_NAME,
		false,
		"emblem_configuration",
		emblems_data);
}

static void extraconf_init(void)
{
	extraconf->EMBLEMS_CONF_NAME = aStrdup("conf/common/emblems.conf");
}

static void extraconf_final(void)
{
	aFree(extraconf->EMBLEMS_CONF_NAME);
}

void extraconf_defaults(void) {
	extraconf = &extraconf_s;

	extraconf->emblems = &emblems_vars;

	extraconf->init = extraconf_init;
	extraconf->final = extraconf_final;

	extraconf->read_conf_file = extraconf_read_conf_file;
	extraconf->read_conf = extraconf_read_conf;
	extraconf->read_vars = extraconf_read_vars;
	extraconf->set_var = extraconf_set_var;
	extraconf->set_var_str = extraconf_set_var_str;
	extraconf->read_emblems = extraconf_read_emblems;
}
