/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2019-2022 Hercules Dev Team
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
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/utils.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/pc.h"
#include "map/script.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @file
* Implementation of the refine interface.
*/

static struct refine_interface refine_s;
static struct refine_interface_private refine_p;
static struct refine_interface_dbs refine_dbs;
struct refine_interface *refine;

/// @copydoc refine_interface::refinery_refine_request()
static void refine_refinery_refine_request(struct map_session_data *sd, int item_index, int material_id, bool use_blacksmith_blessing)
{
	nullpo_retv(sd);

	if (item_index < 0 || item_index >= sd->status.inventorySize)
		return;

	if (!refine->p->is_refinable(sd, item_index))
		return;

	int weapon_level = itemdb_wlv(sd->status.inventory[item_index].nameid);
	int refine_level = sd->status.inventory[item_index].refine;
	int i = 0;
	const struct s_refine_requirement *req = &refine->p->dbs->refine_info[weapon_level].refine_requirements[refine_level];
	ARR_FIND(0, req->req_count, i, req->req[i].nameid == material_id);

	if (i == req->req_count)
		return;

	if (use_blacksmith_blessing && req->blacksmith_blessing == 0)
		return;

	if (sd->status.zeny < req->req[i].cost)
		return;

	if (use_blacksmith_blessing) {
		int count = 0;
		for (int k = 0; k < sd->status.inventorySize; ++k) {
			if (sd->status.inventory[k].nameid == ITEMID_BLACKSMITH_BLESSING)
				count += sd->status.inventory[k].amount;
		}

		if (count < req->blacksmith_blessing)
			return;
	}

	int idx;
	if ((idx = pc->search_inventory(sd, req->req[i].nameid)) == INDEX_NOT_FOUND)
		return;

	if (use_blacksmith_blessing) {
		int amount = req->blacksmith_blessing;
		for (int k = 0; k < sd->status.inventorySize; ++k) {
			if (sd->status.inventory[k].nameid != ITEMID_BLACKSMITH_BLESSING)
				continue;

			int delamount = (amount < sd->status.inventory[k].amount) ? amount : sd->status.inventory[k].amount;
			if (pc->delitem(sd, k, delamount, 0, DELITEM_NORMAL, LOG_TYPE_REFINE) != 0)
				break;

			amount -= delamount;
			if (amount == 0)
				break;
		}
	}

	if (pc->delitem(sd, idx, 1, 0, DELITEM_NORMAL, LOG_TYPE_REFINE) != 0)
		return;

	if (pc->payzeny(sd, req->req[i].cost, LOG_TYPE_REFINE, NULL) != 0)
		return;

	int refine_chance = refine->get_refine_chance(weapon_level, refine_level, req->req[i].type);
	if (rnd() % 100 >= refine_chance) {
		clif->misceffect(&sd->bl, 2);

		int failure_behabior = (use_blacksmith_blessing) ? REFINE_FAILURE_BEHAVIOR_KEEP : req->req[i].failure_behavior;
		switch (failure_behabior) {
		case REFINE_FAILURE_BEHAVIOR_KEEP:
			clif->refine(sd->fd, 1, 0, sd->status.inventory[item_index].refine);
			refine->refinery_add_item(sd, item_index);
			break;
		case REFINE_FAILURE_BEHAVIOR_DOWNGRADE:
			sd->status.inventory[item_index].refine -= 1;
			sd->status.inventory[item_index].refine = cap_value(sd->status.inventory[item_index].refine, 0, MAX_REFINE);
			clif->refine(sd->fd, 2, item_index, sd->status.inventory[item_index].refine);
			logs->pick_pc(sd, LOG_TYPE_REFINE, 1, &sd->status.inventory[item_index], sd->inventory_data[item_index]);
			refine->refinery_add_item(sd, item_index);
			break;
		case REFINE_FAILURE_BEHAVIOR_DESTROY:
		default:
			clif->refine(sd->fd, 1, item_index, sd->status.inventory[item_index].refine);
			pc->delitem(sd, item_index, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_REFINE);
			break;
		}

		if ((req->announce & REFINE_ANNOUNCE_FAILURE) != 0)
			clif->announce_refine_status(sd, sd->status.inventory[item_index].nameid, sd->status.inventory[item_index].refine, false, ALL_CLIENT);
	} else {
		sd->status.inventory[item_index].refine += 1;
		sd->status.inventory[item_index].refine = cap_value(sd->status.inventory[item_index].refine, 0, MAX_REFINE);

		clif->misceffect(&sd->bl, 3);
		clif->refine(sd->fd, 0, item_index, sd->status.inventory[item_index].refine);
		logs->pick_pc(sd, LOG_TYPE_REFINE, 1, &sd->status.inventory[item_index], sd->inventory_data[item_index]);
		refine->refinery_add_item(sd, item_index);

		if ((req->announce & REFINE_ANNOUNCE_SUCCESS) != 0)
			clif->announce_refine_status(sd, sd->status.inventory[item_index].nameid, sd->status.inventory[item_index].refine, true, ALL_CLIENT);
	}
}

/// @copydoc refine_interface::refinery_add_item()
static void refine_refinery_add_item(struct map_session_data *sd, int item_index)
{
	nullpo_retv(sd);

	if (item_index < 0 || item_index >= sd->status.inventorySize)
		return;

	if (!refine->p->is_refinable(sd, item_index))
		return;
	
	int weapon_level = itemdb_wlv(sd->status.inventory[item_index].nameid);
	int refine_level = sd->status.inventory[item_index].refine;
	clif->AddItemRefineryUIAck(sd, item_index, &refine->p->dbs->refine_info[weapon_level].refine_requirements[refine_level]);
}

/// @copydoc refine_interface_private::is_refinable()
static bool refine_is_refinable(struct map_session_data *sd, int item_index)
{
	nullpo_retr(false, sd);
	Assert_retr(false, item_index >= 0 && item_index < sd->status.inventorySize);

	if (sd->status.inventory[item_index].nameid == 0)
		return false;

	struct item_data *itd = itemdb->search(sd->status.inventory[item_index].nameid);

	if (itd == &itemdb->dummy)
		return false;

	if (itd->type != IT_WEAPON && itd->type != IT_ARMOR)
		return false;

	if (itd->flag.no_refine == 1)
		return false;

	if (sd->status.inventory[item_index].identify == 0)
		return false;

	if (sd->status.inventory[item_index].refine >= MAX_REFINE || sd->status.inventory[item_index].expire_time > 0)
		return false;

	if ((sd->status.inventory[item_index].attribute & ATTR_BROKEN) != 0)
		return false;

	return true;
}

/// @copydoc refine_interface::get_randombonus_max()
static int refine_get_randombonus_max(enum refine_type equipment_type, int refine_level)
{
	Assert_ret((int)equipment_type >= REFINE_TYPE_ARMOR && equipment_type < REFINE_TYPE_MAX);
	Assert_ret(refine_level > 0 && refine_level <= MAX_REFINE);

	return refine->p->dbs->refine_info[equipment_type].randombonus_max[refine_level - 1];
}

/// @copydoc refine_interface::get_bonus()
static int refine_get_bonus(enum refine_type equipment_type, int refine_level)
{
	Assert_ret((int)equipment_type >= REFINE_TYPE_ARMOR && equipment_type < REFINE_TYPE_MAX);
	Assert_ret(refine_level > 0 && refine_level <= MAX_REFINE);

	return refine->p->dbs->refine_info[equipment_type].bonus[refine_level - 1];
}

/// @copydoc refine_interface::get_refine_chance()
static int refine_get_refine_chance(enum refine_type wlv, int refine_level, enum refine_chance_type type)
{
	Assert_ret((int)wlv >= REFINE_TYPE_ARMOR && wlv < REFINE_TYPE_MAX);

	if (refine_level < 0 || refine_level >= MAX_REFINE)
		return 0;

	if (type >= REFINE_CHANCE_TYPE_MAX)
		return 0;

	return refine->p->dbs->refine_info[wlv].chance[type][refine_level];
}

/// @copydoc refine_interface_private::announce_behavior_string2enum()
static bool refine_announce_behavior_string2enum(const char *str, unsigned int *result)
{
	nullpo_retr(false, str);
	nullpo_retr(false, result);

	if (strcasecmp(str, "Success") == 0)
		*result = REFINE_ANNOUNCE_SUCCESS;
	else if (strcasecmp(str, "Failure") == 0)
		*result = REFINE_ANNOUNCE_FAILURE;
	else if (strcasecmp(str, "Always") == 0)
		*result = REFINE_ANNOUNCE_ALWAYS;
	else
		return false;

	return true;
}

/// @copydoc refine_interface_private::failure_behavior_string2enum()
static bool refine_failure_behavior_string2enum(const char *str, enum refine_ui_failure_behavior *result)
{
	nullpo_retr(false, str);
	nullpo_retr(false, result);

	if (strcasecmp(str, "Destroy") == 0)
		*result = REFINE_FAILURE_BEHAVIOR_DESTROY;
	else if (strcasecmp(str, "Keep") == 0)
		*result = REFINE_FAILURE_BEHAVIOR_KEEP;
	else if (strcasecmp(str, "Downgrade") == 0)
		*result = REFINE_FAILURE_BEHAVIOR_DOWNGRADE;
	else
		return false;

	return true;
}

/// @copydoc refine_interface_private::readdb_refinery_ui_settings_items()
static bool refine_readdb_refinery_ui_settings_items(const struct config_setting_t *elem, struct s_refine_requirement *req, const char *name, const char *source)
{
	nullpo_retr(false, elem);
	nullpo_retr(false, req);
	nullpo_retr(false, name);
	nullpo_retr(false, source);
	Assert_retr(false, req->req_count < MAX_REFINE_REQUIREMENTS);

	const char *aegis_name = config_setting_name(elem);
	struct item_data *itd;

	if ((itd = itemdb->search_name(aegis_name)) == NULL) {
		ShowWarning("refine_readdb_requirements_items: Invalid item '%s' passed to requirements of '%s' in \"%s\" skipping...\n", aegis_name, name, source);
		return false;
	}

	for (int i = 0; i < req->req_count; ++i) {
		if (req->req[i].nameid == itd->nameid) {
			ShowWarning("refine_readdb_requirements_items: Duplicated item '%s' passed to requirements of '%s' in \"%s\" skipping...\n", aegis_name, name, source);
			return false;
		}
	}

	const char *type_string = NULL;
	if (libconfig->setting_lookup_string(elem, "Type", &type_string) == CONFIG_FALSE) {
		ShowWarning("refine_readdb_requirements_items: no type passed to item '%s' of requirements of '%s' in \"%s\" skipping...\n", aegis_name, name, source);
		return false;
	}

	int type;
	if (!script->get_constant(type_string, &type)) {
		ShowWarning("refine_readdb_requirements_items: invalid type '%s' passed to item '%s' of requirements of '%s' in \"%s\" skipping...\n", type_string, aegis_name, name, source);
		return false;
	}

	int cost = 0;
	if (libconfig->setting_lookup_int(elem, "Cost", &cost) == CONFIG_TRUE) {
		if (cost < 1) {
			ShowWarning("refine_readdb_requirements_items: invalid cost value %d passed to item '%s' of requirements of '%s' in \"%s\" defaulting to 0...\n", cost, aegis_name, name, source);
			cost = 0;
		}
	}

	enum refine_ui_failure_behavior behavior = REFINE_FAILURE_BEHAVIOR_DESTROY;
	const char *behavior_string = NULL;
	if (libconfig->setting_lookup_string(elem, "FailureBehavior", &behavior_string) != CONFIG_FALSE) {
		if (!refine->p->failure_behavior_string2enum(behavior_string, &behavior)) {
			ShowWarning("refine_readdb_requirements_items: invalid failure behavior value %s passed to item '%s' of requirements of '%s' in \"%s\" defaulting to 'Destroy'...\n", behavior_string, aegis_name, name, source);
		}
	}

	req->req[req->req_count].nameid = itd->nameid;
	req->req[req->req_count].type = type;
	req->req[req->req_count].cost = cost;
	req->req[req->req_count].failure_behavior = behavior;
	req->req_count++;

	return true;
}

/// @copydoc refine_interface_private::readdb_refinery_ui_settings_sub()
static bool refine_readdb_refinery_ui_settings_sub(const struct config_setting_t *elem, int type, const char *name, const char *source)
{
	nullpo_retr(false, elem);
	nullpo_retr(false, name);
	nullpo_retr(false, source);
	Assert_retr(0, type >= REFINE_TYPE_ARMOR && type < REFINE_TYPE_MAX);

	struct config_setting_t *level_t;
	bool levels[MAX_REFINE] = {0};

	if ((level_t = libconfig->setting_get_member(elem, "Level")) == NULL) {
		ShowWarning("refine_readdb_requirements_sub: a requirements element missing level field for entry '%s' in \"%s\" skipping...\n", name, source);
		return false;
	}

	if (config_setting_is_scalar(level_t)) {
		if (!config_setting_is_number(level_t)) {
			ShowWarning("refine_readdb_requirements_sub: expected 'Level' field to be an integer '%s' in \"%s\" skipping...\n", name, source);
			return false;
		}

		int refine_level = libconfig->setting_get_int(level_t);
		if (refine_level < 1 || refine_level > MAX_REFINE) {
			ShowWarning("refine_readdb_requirements_sub: Invalid 'Level' given value %d expected a value between %d and %d '%s' in \"%s\" skipping...\n", refine_level, 1, MAX_REFINE, name, source);
			return false;
		}

		levels[refine_level - 1] = true;
	} else if (config_setting_is_aggregate(level_t)) {
		if (libconfig->setting_length(level_t) != 2) {
			ShowWarning("refine_readdb_requirements_sub: invalid length for Level array, expected 2 found %d for entry '%s' in \"%s\" skipping...\n", libconfig->setting_length(level_t), name, source);
			return false;
		}

		int levels_range[2];
		const struct config_setting_t *level_entry = NULL;
		int i = 0,
			k = 0;
		while ((level_entry = libconfig->setting_get_elem(level_t, i++)) != NULL) {
			if (!config_setting_is_number(level_entry)) {
				ShowWarning("refine_readdb_requirements_sub: expected 'Level' array field to be an integer '%s' in \"%s\" skipping...\n", name, source);
				return false;
			}

			levels_range[k] = libconfig->setting_get_int(level_entry);
			if (levels_range[k] < 1 || levels_range[k] > MAX_REFINE) {
				ShowWarning("refine_readdb_requirements_sub: Invalid 'Level' given value %d expected a value between %d and %d in entry'%s' in \"%s\" skipping...\n", levels_range[k], 1, MAX_REFINE, name, source);
				return false;
			}

			++k;
		}

		if (!(levels_range[0] < levels_range[1])) {
			ShowWarning("refine_readdb_requirements_sub: Invalid 'Level' range was given low %d high %d in entry'%s' in \"%s\" skipping...\n", levels_range[0], levels_range[1], name, source);
			return false;
		}

		for (i = levels_range[0] - 1; i < levels_range[1]; ++i) {
			levels[i] = true;
		}
	}

	struct s_refine_requirement req = {0};
	if (libconfig->setting_lookup_int(elem, "BlacksmithBlessing", &req.blacksmith_blessing) == CONFIG_TRUE) {
		if (req.blacksmith_blessing < 1 || req.blacksmith_blessing > INT8_MAX) {
			ShowWarning("refine_readdb_requirements_sub: Invalid 'BlacksmithBlessing' amount was given value %d expected a value between %d and %d in entry'%s' in \"%s\" defaulting to 0...\n", req.blacksmith_blessing, 1, INT8_MAX, name, source);
			req.blacksmith_blessing = 0;
		}
	}

	req.announce = 0;
	const char *announce_behavior = NULL;
	if (libconfig->setting_lookup_string(elem, "Announce", &announce_behavior) != CONFIG_FALSE) {
		if (!refine->p->announce_behavior_string2enum(announce_behavior, &req.announce)) {
			ShowWarning("refine_readdb_requirements_sub: invalid announce behavior value '%s' in entry '%s' in \"%s\" defaulting to not announce...\n", announce_behavior, name, source);
		}
	}

	struct config_setting_t *items_t;
	if ((items_t = libconfig->setting_get_member(elem, "Items")) == NULL) {
		ShowWarning("refine_readdb_requirements_sub: a requirements element missing Items element for entry '%s' in \"%s\" skipping...\n", name, source);
		return false;
	}

	if (libconfig->setting_length(items_t) < 1) {
		ShowWarning("refine_readdb_requirements_sub: an Items element containing no items passed for entry '%s' in \"%s\" skipping...\n", name, source);
		return false;
	}

	int loaded_items = 0;
	for (int i = 0; i < libconfig->setting_length(items_t); ++i) {
		if (req.req_count >= MAX_REFINE_REQUIREMENTS) {
			ShowWarning("refine_readdb_requirements_sub: Too many items passed to requirements maximum possible items is %d entry '%s' in \"%s\" skipping...\n", MAX_REFINE_REQUIREMENTS, name, source);
			continue;
		}

		struct config_setting_t *item_t = libconfig->setting_get_elem(items_t, i);

		if (!refine->p->readdb_refinery_ui_settings_items(item_t, &req, name, source))
			continue;

		loaded_items++;
	}

	if (loaded_items == 0) {
		ShowWarning("refine_readdb_requirements_sub: no valid items for requirements is passed for entry '%s' in \"%s\" skipping...\n", name, source);
		return false;
	}

	for (int i = 0; i < MAX_REFINE; ++i) {
		if (!levels[i])
			continue;

		refine->p->dbs->refine_info[type].refine_requirements[i] = req;
	}

	return true;
}

/// @copydoc refine_interface_private::readdb_refinery_ui_settings()
static int refine_readdb_refinery_ui_settings(const struct config_setting_t *r, int type, const char *name, const char *source)
{
	nullpo_retr(0, r);
	nullpo_retr(0, name);
	nullpo_retr(0, source);
	Assert_retr(0, type >= REFINE_TYPE_ARMOR && type < REFINE_TYPE_MAX);

	int i = 0;
	const struct config_setting_t *elem = NULL;
	while ((elem = libconfig->setting_get_elem(r, i++)) != NULL) {
		refine->p->readdb_refinery_ui_settings_sub(elem, type, name, source);
	}

	int retval = 0;
	for (i = 0; i < MAX_REFINE; ++i) {
		if (refine->p->dbs->refine_info[type].refine_requirements[i].req_count > 0)
			retval++;
	}

	return retval;
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

	struct config_setting_t *refinery_ui_settings;
	if ((refinery_ui_settings = libconfig->setting_get_member(r, "RefineryUISettings")) == NULL) {
		ShowWarning("status_readdb_refine_libconfig_sub: Missing Requirements for entry '%s' in \"%s\", skipping.\n", name, source);
		return 0;
	}

	if (refine->p->readdb_refinery_ui_settings(refinery_ui_settings, type, name, source) != MAX_REFINE) {
		ShowWarning("status_readdb_refine_libconfig_sub: Not all refine levels have requrements entry for entry '%s' in \"%s\", skipping.\n", name, source);
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
		bool duplicate[MAX_REFINE];
		int bonus[MAX_REFINE], rnd_bonus[MAX_REFINE];
		int chance[REFINE_CHANCE_TYPE_MAX][MAX_REFINE];

		memset(&duplicate, 0, sizeof(duplicate));
		memset(&bonus, 0, sizeof(bonus));
		memset(&rnd_bonus, 0, sizeof(rnd_bonus));

		for (int i = 0; i < REFINE_CHANCE_TYPE_MAX; i++)
			for (int j = 0; j < MAX_REFINE; j++)
				chance[i][j] = 100; // default value for all rates.

		struct config_setting_t *t = NULL;
		for (int i = 0; (t = libconfig->setting_get_elem(rate, i)) != NULL && config_setting_is_group(t); ++i) {
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
		for (int i = 0; i < MAX_REFINE; i++) {
			refine->p->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_NORMAL][i] = chance[REFINE_CHANCE_TYPE_NORMAL][i];
			refine->p->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_E_NORMAL][i] = chance[REFINE_CHANCE_TYPE_E_NORMAL][i];
			refine->p->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_ENRICHED][i] = chance[REFINE_CHANCE_TYPE_ENRICHED][i];
			refine->p->dbs->refine_info[type].chance[REFINE_CHANCE_TYPE_E_ENRICHED][i] = chance[REFINE_CHANCE_TYPE_E_ENRICHED][i];
			refine->p->dbs->refine_info[type].randombonus_max[i] = rnd_bonus[i];
			bonus[i] += bonus_per_level + (i > 0 ? bonus[i - 1] : 0);
			refine->p->dbs->refine_info[type].bonus[i] = bonus[i];
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
	nullpo_retr(0, filename);

	bool duplicate[REFINE_TYPE_MAX];
	struct config_t refine_db_conf;
	struct config_setting_t *r;
	char filepath[256];
	int i = 0, count = 0;

	safesnprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, filename);
	if (!libconfig->load_file(&refine_db_conf, filepath))
		return 0;

	memset(&duplicate, 0, sizeof(duplicate));

	while((r = libconfig->setting_get_elem(refine_db_conf.root, i++))) {
		char *name = config_setting_name(r);
		int type = refine->p->readdb_refine_libconfig_sub(r, name, filename);
		if (type != 0) {
			if (duplicate[type - 1]) {
				ShowWarning("status_readdb_refine_libconfig: duplicate entry for %s in \"%s\", overwriting previous entry...\n", name, filename);
			} else {
				duplicate[type - 1] = true;
			}
			count++;
		}
	}
	libconfig->destroy(&refine_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);

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
	refine->p->dbs = &refine_dbs;

	refine->p->readdb_refine_libconfig = refine_readdb_refine_libconfig;
	refine->p->readdb_refine_libconfig_sub = refine_readdb_refine_libconfig_sub;
	refine->p->announce_behavior_string2enum = refine_announce_behavior_string2enum;
	refine->p->failure_behavior_string2enum = refine_failure_behavior_string2enum;
	refine->p->readdb_refinery_ui_settings_items = refine_readdb_refinery_ui_settings_items;
	refine->p->readdb_refinery_ui_settings_sub = refine_readdb_refinery_ui_settings_sub;
	refine->p->readdb_refinery_ui_settings = refine_readdb_refinery_ui_settings;
	refine->p->is_refinable = refine_is_refinable;

	refine->init = refine_init;
	refine->final = refine_final;
	refine->refinery_refine_request = refine_refinery_refine_request;
	refine->refinery_add_item = refine_refinery_add_item;
	refine->get_refine_chance = refine_get_refine_chance;
	refine->get_bonus = refine_get_bonus;
	refine->get_randombonus_max = refine_get_randombonus_max;
}
