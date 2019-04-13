/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2019  Hercules Dev Team
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

#include "refine.p.h"
#include "common/cbasetypes.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "map/map.h"

#include <stdio.h>
#include <stdlib.h>

/** @file
* Implementation of the refine interface.
*/

static struct refine_interface refine_s;
static struct refine_interface_private refine_p;
static struct refine_interface_dbs refine_dbs;
struct refine_interface *refine;

/// @copydoc refine_interface::get_randombonus_max()
static int refine_get_randombonus_max(enum refine_type equipment_type, int refine_level)
{
	Assert_ret((int)equipment_type >= REFINE_TYPE_ARMOR && equipment_type < REFINE_TYPE_MAX);
	Assert_ret(refine_level > 0 && refine_level <= MAX_REFINE);

	return refine->dbs->refine_info[equipment_type].randombonus_max[refine_level - 1];
}

/// @copydoc refine_interface::get_bonus()
static int refine_get_bonus(enum refine_type equipment_type, int refine_level)
{
	Assert_ret((int)equipment_type >= REFINE_TYPE_ARMOR && equipment_type < REFINE_TYPE_MAX);
	Assert_ret(refine_level > 0 && refine_level <= MAX_REFINE);

	return refine->dbs->refine_info[equipment_type].bonus[refine_level - 1];
}

/// @copydoc refine_interface::get_refine_chance()
static int refine_get_refine_chance(enum refine_type wlv, int refine_level, enum refine_chance_type type)
{
	Assert_ret((int)wlv >= REFINE_TYPE_ARMOR && wlv < REFINE_TYPE_MAX);

	if (refine_level < 0 || refine_level >= MAX_REFINE)
		return 0;

	if (type >= REFINE_CHANCE_TYPE_MAX)
		return 0;

	return refine->dbs->refine_info[wlv].chance[type][refine_level];
}

/// @copydoc refine_interface_private::readdb_refine_libconfig_sub()
static int refine_readdb_refine_libconfig_sub(struct config_setting_t *r, const char *name, const char *source)
{
	struct config_setting_t *rate = NULL;
	int type = REFINE_TYPE_ARMOR, bonus_per_level = 0, rnd_bonus_v = 0, rnd_bonus_lv = 0;
	char lv[4];
	nullpo_ret(r);
	nullpo_ret(name);
	nullpo_ret(source);

	if (strncmp(name, "Armors", 6) == 0) {
		type = REFINE_TYPE_ARMOR;
	} else if (strncmp(name, "WeaponLevel", 11) != 0 || !strspn(&name[strlen(name)-1], "0123456789") || (type = atoi(strncpy(lv, name+11, 2))) == REFINE_TYPE_ARMOR) {
		ShowError("status_readdb_refine_libconfig_sub: Invalid key name for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}
	if (type < REFINE_TYPE_ARMOR || type >= REFINE_TYPE_MAX) {
		ShowError("status_readdb_refine_libconfig_sub: Out of range level for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}
	if (!libconfig->setting_lookup_int(r, "StatsPerLevel", &bonus_per_level)) {
		ShowWarning("status_readdb_refine_libconfig_sub: Missing StatsPerLevel for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}
	if (!libconfig->setting_lookup_int(r, "RandomBonusStartLevel", &rnd_bonus_lv)) {
		ShowWarning("status_readdb_refine_libconfig_sub: Missing RandomBonusStartLevel for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}
	if (!libconfig->setting_lookup_int(r, "RandomBonusValue", &rnd_bonus_v)) {
		ShowWarning("status_readdb_refine_libconfig_sub: Missing RandomBonusValue for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}

	if ((rate=libconfig->setting_get_member(r, "Rates")) != NULL && config_setting_is_group(rate)) {
		struct config_setting_t *t = NULL;
		bool duplicate[MAX_REFINE];
		int bonus[MAX_REFINE], rnd_bonus[MAX_REFINE];
		int chance[REFINE_CHANCE_TYPE_MAX][MAX_REFINE];
		int i, j;

		memset(&duplicate, 0, sizeof(duplicate));
		memset(&bonus, 0, sizeof(bonus));
		memset(&rnd_bonus, 0, sizeof(rnd_bonus));

		for (i = 0; i < REFINE_CHANCE_TYPE_MAX; i++)
			for (j = 0; j < MAX_REFINE; j++)
				chance[i][j] = 100; // default value for all rates.

		i = 0;
		j = 0;
		while ((t = libconfig->setting_get_elem(rate,i++)) != NULL && config_setting_is_group(t)) {
			int level = 0, i32;
			char *rlvl = config_setting_name(t);
			memset(&lv, 0, sizeof(lv));

			if (!strspn(&rlvl[strlen(rlvl) - 1], "0123456789") || (level = atoi(strncpy(lv, rlvl + 2, 3))) <= 0) {
				ShowError("status_readdb_refine_libconfig_sub: Invalid refine level format '%s' for entry %s in \"%s\"... skipping.\n", rlvl, name, source);
				continue;
			}

			if (level <= 0 || level > MAX_REFINE) {
				ShowError("status_readdb_refine_libconfig_sub: Out of range refine level '%s' for entry %s in \"%s\"... skipping.\n", rlvl, name, source);
				continue;
			}

			level--;

			if (duplicate[level]) {
				ShowWarning("status_readdb_refine_libconfig_sub: duplicate rate '%s' for entry %s in \"%s\", overwriting previous entry...\n", rlvl, name, source);
			} else {
				duplicate[level] = true;
			}

			if (libconfig->setting_lookup_int(t, "NormalChance", &i32) != 0)
				chance[REFINE_CHANCE_TYPE_NORMAL][level] = i32;
			else
				chance[REFINE_CHANCE_TYPE_NORMAL][level] = 100;

			if (libconfig->setting_lookup_int(t, "EnrichedChance", &i32) != 0)
				chance[REFINE_CHANCE_TYPE_ENRICHED][level] = i32;
			else
				chance[REFINE_CHANCE_TYPE_ENRICHED][level] = level > 10 ? 0 : 100; // enriched ores up to +10 only.

			if (libconfig->setting_lookup_int(t, "EventNormalChance", &i32) != 0)
				chance[REFINE_CHANCE_TYPE_E_NORMAL][level] = i32;
			else
				chance[REFINE_CHANCE_TYPE_E_NORMAL][level] = 100;

			if (libconfig->setting_lookup_int(t, "EventEnrichedChance", &i32) != 0)
				chance[REFINE_CHANCE_TYPE_E_ENRICHED][level] = i32;
			else
				chance[REFINE_CHANCE_TYPE_E_ENRICHED][level] = level > 10 ? 0 : 100; // enriched ores up to +10 only.

			if (libconfig->setting_lookup_int(t, "Bonus", &i32) != 0)
				bonus[level] += i32;

			if (level >= rnd_bonus_lv - 1)
				rnd_bonus[level] = rnd_bonus_v * (level - rnd_bonus_lv + 2);
		}
		for (i = 0; i < MAX_REFINE; i++) {
			refine->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_NORMAL][i] = chance[REFINE_CHANCE_TYPE_NORMAL][i];
			refine->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_E_NORMAL][i] = chance[REFINE_CHANCE_TYPE_E_NORMAL][i];
			refine->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_ENRICHED][i] = chance[REFINE_CHANCE_TYPE_ENRICHED][i];
			refine->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_E_ENRICHED][i] = chance[REFINE_CHANCE_TYPE_E_ENRICHED][i];
			refine->dbs->refine_info[type].randombonus_max[i] = rnd_bonus[i];
			bonus[i] += bonus_per_level + (i > 0 ? bonus[i - 1] : 0);
			refine->dbs->refine_info[type].bonus[i] = bonus[i];
		}
	} else {
		ShowWarning("status_readdb_refine_libconfig_sub: Missing refine rates for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}

	return type + 1;
}

/// @copydoc refine_interface_private::readdb_refine_libconfig()
static int refine_readdb_refine_libconfig(const char *filename)
{
	bool duplicate[REFINE_TYPE_MAX];
	struct config_t refine_db_conf;
	struct config_setting_t *r;
	char filepath[256];
	int i = 0, count = 0;

	safesnprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, filename);
	if (!libconfig->load_file(&refine_db_conf, filepath))
		return 0;

	memset(&duplicate,0,sizeof(duplicate));

	while((r = libconfig->setting_get_elem(refine_db_conf.root,i++))) {
		char *name = config_setting_name(r);
		int type = refine->p->readdb_refine_libconfig_sub(r, name, filename);
		if (type != 0) {
			if (duplicate[type-1]) {
				ShowWarning("status_readdb_refine_libconfig: duplicate entry for %s in \"%s\", overwriting previous entry...\n", name, filename);
			} else {
				duplicate[type-1] = true;
			}
			count++;
		}
	}
	libconfig->destroy(&refine_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filename);

	return count;
}

/// @copydoc refine_interface::init()
static int refine_init(bool minimal)
{
	if (minimal)
		return 0;

	refine->p->readdb_refine_libconfig(DBPATH"refine_db.conf");
	return 0;
}

/// @copydoc refine_interface::final()
static void refine_final(void)
{
}

void refine_defaults(void)
{
	refine = &refine_s;
	refine->p = &refine_p;

	refine->p->readdb_refine_libconfig = refine_readdb_refine_libconfig;
	refine->p->readdb_refine_libconfig_sub = refine_readdb_refine_libconfig_sub;

	refine->dbs = &refine_dbs;
	refine->init = refine_init;
	refine->final = refine_final;
	refine->get_refine_chance = refine_get_refine_chance;
	refine->get_bonus = refine_get_bonus;
	refine->get_randombonus_max = refine_get_randombonus_max;
}
