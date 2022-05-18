/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2021 Hercules Dev Team
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

#include "grader.h"

#include "common/conf.h"
#include "common/showmsg.h"
#include "common/utils.h"
#include "common/strlib.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/pc.h"

#include <stdlib.h>

static struct grade_interface_dbs grade_dbs;
static struct grader_interface grader_s;
struct grader_interface *grader;

static bool grader_read_db_libconfig(void)
{
	char filepath[256];
	safesnprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, DBPATH"grade_db.conf");

	struct config_t grade_db_conf;
	if (libconfig->load_file(&grade_db_conf, filepath) == CONFIG_FALSE) {
		ShowError("%s: can't read %s\n", __func__, filepath);
		return false;
	}

	int i = 0;
	int count = 0;
	struct config_setting_t *it = NULL;
	struct config_setting_t *cdb = libconfig->lookup(&grade_db_conf, "grade_db");

	while ((it = libconfig->setting_get_elem(cdb, i++)) != NULL) {
		if (grader->read_db_libconfig_sub(it, i - 1, filepath))
			++count;
	}

	libconfig->destroy(&grade_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
	return true;
}

static bool grader_read_db_libconfig_sub(const struct config_setting_t *it, int n, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(source);

	int i32 = 0;
	const char *name = NULL;
	if (libconfig->setting_lookup_string(it, "Grade", &name) != CONFIG_TRUE) {
		ShowWarning("%s: Invalid Grade constant entry #%d at %s skipping...\n", __func__, n, source);
		return false;
	}
	if (!script->get_constant(name, &i32) || i32 < 0 || i32 >= ITEM_GRADE_MAX) {
		ShowError("%s: Invalid Grade constant %s entry #%d at %s. Skipping...\n", __func__, name, n, source);
		return false;
	}

	int success_chance = 0;
	if (libconfig->setting_lookup_int(it, "SuccessChance", &success_chance) != CONFIG_TRUE || success_chance < 0 || success_chance > 100) {
		ShowWarning("%s: Invalid SuccessChance %d in entry #%d at %s defaulting to 0.\n", __func__, success_chance, n, source);
		success_chance = 0;
	}
	grader->dbs->grade_info[i32].success_chance = success_chance;

	const char *behavior_string = NULL;
	enum grade_announce_condition behavior = GRADE_ANNOUNCE_NONE;
	if (libconfig->setting_lookup_string(it, "Announce", &behavior_string) == CONFIG_TRUE) {
		if (!grader->announce_behavior_string2enum(behavior_string, &behavior) || behavior < GRADE_ANNOUNCE_NONE || behavior > GRADE_ANNOUNCE_ALWAYS) {
			ShowWarning("%s: Invalid Announce behavior %s in entry #%d at %s defaulting to None.\n", __func__, behavior_string, n, source);
			behavior = GRADE_ANNOUNCE_NONE;
		}
	}
	grader->dbs->grade_info[i32].announce = behavior;

	struct config_setting_t *mi = libconfig->setting_get_member(it, "MaterialInfo");
	if (!grader->read_db_libconfig_sub_materials(mi, (enum grade_level)i32, n, source))
		return false;

	struct config_setting_t *bi = libconfig->setting_get_member(it, "BlessingInfo");
	if (!grader->read_db_libconfig_sub_blessing(bi, (enum grade_level)i32, n, source))
		return false;
	return true;
}

static bool grader_read_db_libconfig_sub_materials(const struct config_setting_t *it, enum grade_level gl, int n, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(source);

	int i = 0;
	int count = 0;
	struct config_setting_t *itm = NULL;

	while ((itm = libconfig->setting_get_elem(it, i++)) != NULL) {
		if (grader->read_db_libconfig_sub_material(itm, &grader->dbs->grade_info[gl].materials[count], n, source))
			++count;
	}

	return (count > 0);
}

static bool grader_read_db_libconfig_sub_material(const struct config_setting_t *it, struct grade_material *gm, int n, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(gm);
	nullpo_ret(source);

	const char *str = NULL;
	if (libconfig->setting_lookup_string(it, "ItemId", &str) != CONFIG_TRUE || str == NULL) {
		ShowWarning("%s: Unable to find ItemId in MaterialInfo entry #%d at %s skipping...\n", __func__, n, source);
		return false;
	}

	const struct item_data *itd = itemdb->name2id(str);
	if (itd == NULL) {
		ShowWarning("%s: Invalid ItemId '%s' passed to MaterialInfo entry #%d at %s skipping...\n", __func__, str, n, source);
		return false;
	}

	int amount = 0;
	if (libconfig->setting_lookup_int(it, "ItemAmount", &amount) != CONFIG_TRUE || amount <= 0 || amount > MAX_AMOUNT) {
		ShowWarning("%s: Invalid ItemAmount %d in MaterialInfo entry #%d at %s defaulting to 1.\n", __func__, amount, n, source);
		amount = 1;
	}

	int zeny_cost = 0;
	if (libconfig->setting_lookup_int(it, "ZenyCost", &zeny_cost) != CONFIG_TRUE || zeny_cost < 0 || zeny_cost > MAX_ZENY) {
		ShowWarning("%s: Invalid ZenyCost %d in MaterialInfo entry #%d at %s defaulting to 0.\n", __func__, zeny_cost, n, source);
		zeny_cost = 0;
	}

	const char *behavior_string = NULL;
	enum grade_ui_failure_behavior behavior = GRADE_FAILURE_BEHAVIOR_KEEP;
	if (libconfig->setting_lookup_string(it, "FailureBehavior", &behavior_string) == CONFIG_TRUE) {
		if (!grader->failure_behavior_string2enum(behavior_string, &behavior) || behavior < 0 || behavior > GRADE_FAILURE_BEHAVIOR_DOWNGRADE) {
			ShowWarning("%s: Invalid FailureBehavior %s in MaterialInfo entry #%d at %s defaulting to Keep.\n", __func__, behavior_string, n, source);
			behavior = GRADE_FAILURE_BEHAVIOR_KEEP;
		}
	}

	gm->nameid = itd->nameid;
	gm->amount = amount;
	gm->zeny_cost = zeny_cost;
	gm->failure_behavior = behavior;
	return true;
}

static bool grader_read_db_libconfig_sub_blessing(const struct config_setting_t *it, enum grade_level gl, int n, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(source);

	const char *str = NULL;
	if (libconfig->setting_lookup_string(it, "ItemId", &str) != CONFIG_TRUE || str == NULL) {
		ShowWarning("%s: Unable to find ItemId in BlessingInfo entry #%d at %s skipping...\n", __func__, n, source);
		return false;
	}

	const struct item_data *itd = itemdb->name2id(str);
	if (itd == NULL) {
		ShowWarning("%s: Invalid ItemId '%s' passed to BlessingInfo entry #%d at %s skipping...\n", __func__, str, n, source);
		return false;
	}

	int amount = 0;
	if (libconfig->setting_lookup_int(it, "ItemAmount", &amount) != CONFIG_TRUE || amount <= 0 || amount > MAX_AMOUNT) {
		ShowWarning("%s: Invalid ItemAmount %d in BlessingInfo entry #%d at %s defaulting to 1.\n", __func__, amount, n, source);
		amount = 1;
	}

	int bonus = 0;
	if (libconfig->setting_lookup_int(it, "BonusPerItem", &bonus) != CONFIG_TRUE || bonus < 0 || bonus > 100) {
		ShowWarning("%s: Invalid BonusPerItem %d in BlessingInfo entry #%d at %s defaulting to 0.\n", __func__, bonus, n, source);
		bonus = 0;
	}

	int max_blessing = 0;
	if (libconfig->setting_lookup_int(it, "MaxUsable", &max_blessing) != CONFIG_TRUE || max_blessing < 0 || max_blessing > 100) {
		ShowWarning("%s: Invalid MaxUsable %d in BlessingInfo entry #%d at %s defaulting to 0.\n", __func__, max_blessing, n, source);
		max_blessing = 0;
	}

	grader->dbs->grade_info[gl].blessing.nameid = itd->nameid;
	grader->dbs->grade_info[gl].blessing.amount = amount;
	grader->dbs->grade_info[gl].blessing.bonus = bonus;
	grader->dbs->grade_info[gl].blessing.max_blessing = max_blessing;
	return true;
}

static bool grader_failure_behavior_string2enum(const char *str, enum grade_ui_failure_behavior *result)
{
	nullpo_retr(false, str);
	nullpo_retr(false, result);

	if (strcasecmp(str, "Keep") == 0)
		*result = GRADE_FAILURE_BEHAVIOR_KEEP;
	else if (strcasecmp(str, "Destroy") == 0)
		*result = GRADE_FAILURE_BEHAVIOR_DESTROY;
	else if (strcasecmp(str, "Downgrade") == 0)
		*result = GRADE_FAILURE_BEHAVIOR_DOWNGRADE;
	else
		return false;

	return true;
}

static bool grader_announce_behavior_string2enum(const char *str, enum grade_announce_condition *result)
{
	nullpo_retr(false, str);
	nullpo_retr(false, result);

	if (strcasecmp(str, "None") == 0)
		*result = GRADE_ANNOUNCE_NONE;
	if (strcasecmp(str, "Success") == 0)
		*result = GRADE_ANNOUNCE_SUCCESS;
	else if (strcasecmp(str, "Failure") == 0)
		*result = GRADE_ANNOUNCE_FAILURE;
	else if (strcasecmp(str, "Always") == 0)
		*result = GRADE_ANNOUNCE_ALWAYS;
	else
		return false;

	return true;
}

const struct s_grade_info *grader_get_grade_info(int grade)
{
	Assert_retr(NULL, grade >= ITEM_GRADE_NONE && grade < ITEM_GRADE_MAX);

	return &grader->dbs->grade_info[(enum grade_level)grade];
}

void grader_enchant_add_item(struct map_session_data *sd, int idx)
{
	nullpo_retv(sd);

	if (idx < 0 || idx >= sd->status.inventorySize)
		return;

	if (sd->inventory_data[idx] == NULL || sd->inventory_data[idx]->flag.no_grade == 0) {
		clif->grade_enchant_add_item_result_fail(sd);
		return;
	}

	if (sd->status.inventory[idx].grade < ITEM_GRADE_NONE || sd->status.inventory[idx].grade >= battle->bc->grader_max_used) {
		clif->grade_enchant_add_item_result_fail(sd);
		return;
	}

	const struct s_grade_info *gi = grader->get_grade_info(sd->status.inventory[idx].grade);
	clif->grade_enchant_add_item_result_success(sd, idx, gi);
}

void grader_enchant_start(struct map_session_data *sd, int idx, int mat_idx, bool use_blessing, int blessing_amount)
{
	nullpo_retv(sd);

	if (idx < 0 || idx >= sd->status.inventorySize)
		return;

	if (mat_idx < 0 || mat_idx >= MAX_GRADE_MATERIALS)
		return;

	const struct s_grade_info *gi = grader->get_grade_info(sd->status.inventory[idx].grade);
	const struct grade_material *gmaterial = &gi->materials[mat_idx];

	// Validate the grading material
	if (gmaterial->nameid == 0)
		return;

	const int mat_invidx = pc->search_inventory(sd, gmaterial->nameid);
	if (mat_invidx == INDEX_NOT_FOUND)
		return;

	if (sd->status.inventory[mat_invidx].amount < gmaterial->amount)
		return;

	if (sd->status.zeny < gmaterial->zeny_cost)
		return;

	// Validate the blessing material
	if (use_blessing) {
		if (gi->blessing.nameid == 0)
			return;

		const int bless_invidx = pc->search_inventory(sd, gi->blessing.nameid);

		if (bless_invidx == INDEX_NOT_FOUND)
			return;

		if (blessing_amount < 0 || blessing_amount > gi->blessing.max_blessing)
			return;

		int req_amount = gi->blessing.amount * blessing_amount;
		if (sd->status.inventory[bless_invidx].amount < req_amount)
			return;

		if (pc->delitem(sd, bless_invidx, req_amount, 0, DELITEM_NORMAL, LOG_TYPE_GRADE) != 0)
			return;
	}

	if (pc->delitem(sd, mat_invidx, gmaterial->amount, 0, DELITEM_NORMAL, LOG_TYPE_GRADE) != 0)
		return;

	if (pc->payzeny(sd, gmaterial->zeny_cost, LOG_TYPE_GRADE, NULL) != 0)
		return;

	const int grade_chance = gi->success_chance + (use_blessing ? gi->blessing.bonus * blessing_amount : 0);
	if (rnd() % 100 >= grade_chance) {
		if ((gi->announce & GRADE_ANNOUNCE_FAILURE) != 0)
			clif->announce_grade_status(sd, sd->status.inventory[idx].nameid, sd->status.inventory[idx].grade, false, ALL_CLIENT);

		switch (gmaterial->failure_behavior) {
		case GRADE_FAILURE_BEHAVIOR_KEEP:
			clif->grade_enchant_result(sd, idx, (enum grade_level)sd->status.inventory[idx].grade, GRADE_UPGRADE_FAILED_KEEP);
			break;
		case GRADE_FAILURE_BEHAVIOR_DOWNGRADE:
			clif->grade_enchant_result(sd, idx, (enum grade_level)sd->status.inventory[idx].grade, GRADE_UPGRADE_FAILED_DOWNGRADE);
			sd->status.inventory[idx].grade -= 1;
			sd->status.inventory[idx].grade = cap_value(sd->status.inventory[idx].grade, ITEM_GRADE_NONE, ITEM_GRADE_MAX - 1);
			break;
		case GRADE_FAILURE_BEHAVIOR_DESTROY:
			clif->grade_enchant_result(sd, idx, (enum grade_level)sd->status.inventory[idx].grade, GRADE_UPGRADE_FAILED_DESTROY);
			pc->delitem(sd, idx, 1, 0, DELITEM_FAILREFINE, LOG_TYPE_GRADE);
			break;
		}
	} else {
		sd->status.inventory[idx].refine = 0; // Hardcoded in the client
		sd->status.inventory[idx].grade += 1;
		clif->grade_enchant_result(sd, idx, (enum grade_level)sd->status.inventory[idx].grade, GRADE_UPGRADE_SUCCESS);

		if ((gi->announce & GRADE_ANNOUNCE_SUCCESS) != 0)
			clif->announce_grade_status(sd, sd->status.inventory[idx].nameid, sd->status.inventory[idx].grade, true, ALL_CLIENT);
	}
}

static void grader_reload_db(void)
{
	memset(grader->dbs, 0, sizeof(struct grade_interface_dbs));
	grader->read_db_libconfig();
}

static int do_init_grader(bool minimal)
{
	if (minimal)
		return 0;

	memset(grader->dbs, 0, sizeof(struct grade_interface_dbs));
	grader->read_db_libconfig();
	return 0;
}

static void do_final_grader(void)
{
}

void grader_defaults(void)
{
	grader = &grader_s;
	grader->dbs = &grade_dbs;

	/* core */
	grader->init = do_init_grader;
	grader->final = do_final_grader;

	grader->reload_db = grader_reload_db;
	grader->read_db_libconfig = grader_read_db_libconfig;
	grader->read_db_libconfig_sub = grader_read_db_libconfig_sub;
	grader->read_db_libconfig_sub_materials = grader_read_db_libconfig_sub_materials;
	grader->read_db_libconfig_sub_material = grader_read_db_libconfig_sub_material;
	grader->read_db_libconfig_sub_blessing = grader_read_db_libconfig_sub_blessing;
	grader->failure_behavior_string2enum = grader_failure_behavior_string2enum;
	grader->announce_behavior_string2enum = grader_announce_behavior_string2enum;

	grader->get_grade_info = grader_get_grade_info;
	grader->enchant_add_item = grader_enchant_add_item;
	grader->enchant_start = grader_enchant_start;
}
