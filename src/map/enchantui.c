/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2021-2022 Hercules Dev Team
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

#include "enchantui.h"

#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/timer.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/refine.h"

#include <stdlib.h>

static struct enchantui_interface enchantui_s;
struct enchantui_interface *enchantui;

static void enchantui_read_db_libconfig(void)
{
	char filepath[512];
	snprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, DBPATH"enchant_db.conf");

	struct config_t enchant_conf;
	if (libconfig->load_file(&enchant_conf, filepath) == CONFIG_FALSE) {
		ShowError("%s: can't read %s\n", __func__, filepath);
		return;
	}

	struct config_setting_t *enchant_db = NULL;
	if ((enchant_db = libconfig->setting_get_member(enchant_conf.root, "enchant_db")) == NULL) {
		ShowError("%s: can't read %s\n", __func__, filepath);
		libconfig->destroy(&enchant_conf);
		return;
	}

	int i = 0;
	int count = 0;
	struct config_setting_t *it = NULL;

	while ((it = libconfig->setting_get_elem(enchant_db, i++)) != NULL) {
		if (enchantui->read_db_libconfig_sub(it, i, filepath))
			++count;
	}

	libconfig->destroy(&enchant_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
}

static bool enchantui_read_db_libconfig_sub(const struct config_setting_t *it, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, source);

	struct enchant_info ei = { 0 };

	if (libconfig->setting_lookup_int(it, "Id", &ei.Id) == CONFIG_FALSE) {
		ShowError("%s: Invalid Enchant Id provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	int i32 = 0;
	libconfig->setting_lookup_int(it, "MinRefine", &i32);
	if (i32 < 0 || i32 > MAX_REFINE) {
		ShowError("%s: Invalid MinRefine %d provided for entry %d in '%s', skipping...\n", __func__, i32, n, source);
		return false;
	}
	ei.MinRefine = i32;

	i32 = 0;
	libconfig->setting_lookup_int(it, "MinGrade", &i32);
	if (i32 < 0 || i32 > MAX_ITEM_GRADE) {
		ShowError("%s: Invalid MinGrade %d provided for entry %d in '%s', skipping...\n", __func__, i32, n, source);
		return false;
	}
	ei.MinGrade = i32;

	libconfig->setting_lookup_bool_real(it, "AllowRandomOptions", &ei.AllowRandomOptions);

	const struct config_setting_t *slotorder = libconfig->setting_get_member(it, "SlotOrder");
	if (slotorder != NULL && !enchantui->read_db_libconfig_slot_order(slotorder, &ei, n, source))
		return false;

	const struct config_setting_t *targetitems = libconfig->setting_get_member(it, "TargetItems");
	if (targetitems != NULL && !enchantui->read_db_libconfig_target_items(targetitems, &ei, n, source))
		return false;

	const struct config_setting_t *slotinfo = libconfig->setting_get_member(it, "SlotInfo");
	if (slotinfo != NULL && !enchantui->read_db_libconfig_slot_info(slotinfo, &ei, n, source))
		return false;

	const struct config_setting_t *resetinfo = libconfig->setting_get_member(it, "ResetInfo");
	if (resetinfo != NULL && !enchantui->read_db_libconfig_reset_info(resetinfo, &ei.ResetInfo, n, source))
		return false;

	// Copy the entry into the database
	struct enchant_info *s_ei = aCalloc(1, sizeof(struct enchant_info));
	*s_ei = ei;
	idb_put(enchantui->db, ei.Id, s_ei);

	return true;
}

static bool enchantui_read_db_libconfig_slot_order(const struct config_setting_t *it, struct enchant_info *einfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, einfo);
	nullpo_retr(false, source);

	if (!config_setting_is_aggregate(it)) {
		ShowError("%s: SlotOrder must be a list for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const int len = libconfig->setting_length(it);

	if (len < 0 || len > MAX_SLOTS) {
		ShowError("%s: SlotOrder must be shorter than MAX_SLOTS for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	for (int i = 0; i < len; i++) {
		int slot_order = libconfig->setting_get_int_elem(it, i);

		if (slot_order < 0 || slot_order > MAX_SLOTS) {
			ShowError("%s: Invalid SlotOrder (%d) must be in range 0..MAX_SLOTS for entry %d in '%s', skipping...\n", __func__, slot_order, n, source);
			return false;
		}
	}

	VECTOR_ENSURE(einfo->SlotOrder, len, 1);

	for (int i = 0; i < len; i++) {
		int slot_order = libconfig->setting_get_int_elem(it, i);
		VECTOR_PUSH(einfo->SlotOrder, slot_order);
	}

	return true;
}

static bool enchantui_read_db_libconfig_target_items(const struct config_setting_t *it, struct enchant_info *einfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, einfo);
	nullpo_retr(false, source);

	if (!config_setting_is_aggregate(it)) {
		ShowError("%s: TargetItems must be a list for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	VECTOR_INIT(einfo->TargetItems);

	int i = 0;
	const char *name = NULL;
	while ((name = libconfig->setting_get_string_elem(it, i++)) != NULL) {
		const struct item_data *id = itemdb->search_name(name);
		if (id == NULL) {
			ShowError("%s: Invalid target item \"%s\" for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}

		if (id->slot > (MAX_SLOTS - VECTOR_LENGTH(einfo->SlotOrder))) {
			ShowError("%s: Target item \"%s\" has more active slots than allowed for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}

		VECTOR_ENSURE(einfo->TargetItems, 1, 1);
		VECTOR_PUSH(einfo->TargetItems, id->nameid);
	}
	return true;
}

static bool enchantui_read_db_libconfig_slot_info(const struct config_setting_t *it, struct enchant_info *einfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, einfo);
	nullpo_retr(false, source);

	int i = 0;
	struct config_setting_t *entry = NULL;
	while ((entry = libconfig->setting_get_elem(it, i++)) != NULL) {
		int slot_id = 0;
		if (libconfig->setting_lookup_int(entry, "SlotId", &slot_id) == CONFIG_FALSE || slot_id < 0 || slot_id > MAX_SLOTS) {
			ShowError("%s: Invalid SlotId provided for entry %d in '%s', skipping...\n", __func__, n, source);
			continue;
		}

		if (!enchantui->read_db_libconfig_slot_info_sub(entry, &einfo->SlotInfo[slot_id], n, source))
			return false;
	}
	return true;
}

static bool enchantui_read_db_libconfig_slot_info_sub(const struct config_setting_t *it, struct enchant_slot_info *sinfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, sinfo);
	nullpo_retr(false, source);

	libconfig->setting_lookup_int(it, "SuccessRate", &sinfo->SuccessRate);
	if (sinfo->SuccessRate < 0 || sinfo->SuccessRate > 100000) {
		ShowError("%s: Invalid SuccessRate provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const struct config_setting_t *gradebonus = libconfig->setting_get_member(it, "GradeBonus");
	if (gradebonus != NULL && !enchantui->read_db_libconfig_gradebonus(gradebonus, sinfo, n, source)) {
		ShowError("%s: Failed to load GradeBonus for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const struct config_setting_t *normal = libconfig->setting_get_member(it, "NormalEnchants");
	if (normal != NULL && !enchantui->read_db_libconfig_normal_info(normal, &sinfo->NormalEnchants, n, source)) {
		ShowError("%s: Failed to load NormalEnchants for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const struct config_setting_t *perfect = libconfig->setting_get_member(it, "PerfectEnchants");
	if (perfect != NULL && !enchantui->read_db_libconfig_perfect_info(perfect, &sinfo->PerfectEnchants, n, source)) {
		ShowError("%s: Failed to load PerfectEnchants for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const struct config_setting_t *upgrade = libconfig->setting_get_member(it, "UpgradeInfo");
	if (upgrade != NULL && !enchantui->read_db_libconfig_upgrade_info(upgrade, &sinfo->UpgradeInfo, n, source)) {
		ShowError("%s: Failed to load UpgradeInfo for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}
	return true;
}

static bool enchantui_read_db_libconfig_gradebonus(const struct config_setting_t *it, struct enchant_slot_info *sinfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, sinfo);
	nullpo_retr(false, source);

	int i = 0;
	struct config_setting_t *entry = NULL;
	while ((entry = libconfig->setting_get_elem(it, i++)) != NULL) {
		const char *name = config_setting_name(entry);

		if (strncmp(name, "Grade", 5) != 0 || strspn(&name[strlen(name) - 1], "0123456789") == 0) {
			ShowError("%s: Invalid key name %s provided for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}

		int grade_level = atoi(name + 5);
		sinfo->GradeBonus[grade_level] = libconfig->setting_get_int(entry);
	}
	return true;
}

static bool enchantui_read_db_libconfig_normal_info(const struct config_setting_t *it, struct enchant_info_normal *ninfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, ninfo);
	nullpo_retr(false, source);

	libconfig->setting_lookup_int(it, "Zeny", &ninfo->Zeny);
	if (ninfo->Zeny < 0 || ninfo->Zeny > MAX_ZENY) {
		ShowError("%s: Invalid ResetInfo Zeny provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const struct config_setting_t *materials = libconfig->setting_get_member(it, "Materials");
	if (materials != NULL && !enchantui->read_db_libconfig_materials_list(materials, &ninfo->Materials, n, source))
		return false;

	int i = 0;
	struct config_setting_t *entry = NULL;
	const struct config_setting_t *itemrate = libconfig->setting_get_member(it, "ItemList");
	while ((entry = libconfig->setting_get_elem(itemrate, i++)) != NULL) {
		const char *name = config_setting_name(entry);

		if (strncmp(name, "Grade", 5) != 0 || strspn(&name[strlen(name) - 1], "0123456789") == 0) {
			ShowError("%s: Invalid key name %s provided for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}

		int grade_level = atoi(name + 5);
		if (!enchantui->read_db_libconfig_itemrate_list(entry, &ninfo->ItemList[grade_level], n, source))
			return false;
	}
	return true;
}

static bool enchantui_read_db_libconfig_perfect_info(const struct config_setting_t *it, struct enchant_info_perfect *perfect_info, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, perfect_info);
	nullpo_retr(false, source);

	VECTOR_INIT(*perfect_info);

	int i = 0;
	struct config_setting_t *entry = NULL;
	while ((entry = libconfig->setting_get_elem(it, i++)) != NULL) {
		struct enchant_info_perfect_entry perfentry = { 0 };
		const char *name = config_setting_name(entry);

		struct item_data *idata = itemdb->name2id(name);
		if (idata == NULL) {
			ShowWarning("%s: unknown PerfectEnchants item '%s' for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}
		perfentry.BaseItem = idata->nameid;

		libconfig->setting_lookup_int(entry, "Zeny", &perfentry.Zeny);
		if (perfentry.Zeny < 0 || perfentry.Zeny > MAX_ZENY) {
			ShowWarning("%s: invalid PerfectEnchants zeny (%d) for entry %d in '%s', skipping...\n", __func__, perfentry.Zeny, n, source);
			continue;
		}

		const struct config_setting_t *materials = libconfig->setting_get_member(entry, "Materials");
		if (materials != NULL && !enchantui->read_db_libconfig_materials_list(materials, &perfentry.Materials, n, source))
			continue;

		VECTOR_ENSURE(*perfect_info, 1, 1);
		VECTOR_PUSH(*perfect_info, perfentry);
	}

	return true;
}

static bool enchantui_read_db_libconfig_upgrade_info(const struct config_setting_t *it, struct enchant_info_upgrade *uinfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, uinfo);
	nullpo_retr(false, source);

	VECTOR_INIT(*uinfo);

	int i = 0;
	struct config_setting_t *entry = NULL;
	while ((entry = libconfig->setting_get_elem(it, i++)) != NULL) {
		struct enchant_info_upgrade_entry upgentry = { 0 };
		const char *name = config_setting_name(entry);

		struct item_data *idata = itemdb->name2id(name);
		if (idata == NULL) {
			ShowWarning("%s: unknown UpgradeInfo item '%s' for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}
		upgentry.BaseItem = idata->nameid;

		int resultitem = 0;
		if (!map->setting_lookup_const(entry, "ResultItem", &resultitem)) {
			ShowError("%s: Invalid UpgradeInfo ResultItem for entry %d in '%s', skipping...\n", __func__, n, source);
			return false;
		}
		upgentry.ResultItem = resultitem;

		libconfig->setting_lookup_int(entry, "Zeny", &upgentry.Zeny);
		if (upgentry.Zeny < 0 || upgentry.Zeny > MAX_ZENY) {
			ShowWarning("%s: invalid UpgradeInfo zeny (%d) for entry %d in '%s', skipping...\n", __func__, upgentry.Zeny, n, source);
			continue;
		}

		const struct config_setting_t *materials = libconfig->setting_get_member(entry, "Materials");
		if (materials != NULL && !enchantui->read_db_libconfig_materials_list(materials, &upgentry.Materials, n, source))
			continue;

		VECTOR_ENSURE(*uinfo, 1, 1);
		VECTOR_PUSH(*uinfo, upgentry);
	}

	return true;
}

static bool enchantui_read_db_libconfig_reset_info(const struct config_setting_t *it, struct enchant_info_reset *rinfo, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, rinfo);
	nullpo_retr(false, source);

	libconfig->setting_lookup_int(it, "Rate", &rinfo->Rate);
	if (rinfo->Rate < 0 || rinfo->Rate > 100000) {
		ShowError("%s: Invalid ResetInfo Rate provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	libconfig->setting_lookup_int(it, "Zeny", &rinfo->Zeny);
	if (rinfo->Zeny < 0 || rinfo->Zeny > MAX_ZENY) {
		ShowError("%s: Invalid ResetInfo Zeny provided for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	const struct config_setting_t *materials = libconfig->setting_get_member(it, "Materials");
	if (materials != NULL && !enchantui->read_db_libconfig_materials_list(materials, &rinfo->Materials, n, source))
		return false;

	return true;
}

static bool enchantui_read_db_libconfig_materials_list(const struct config_setting_t *it, struct itemlist *materials, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, materials);
	nullpo_retr(false, source);

	if (!config_setting_is_group(it)) {
		ShowError("%s: Materials must be a list for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	VECTOR_INIT(*materials);

	int i = 0;
	struct config_setting_t *entry = NULL;
	while ((entry = libconfig->setting_get_elem(it, i++)) != NULL) {
		const char *name = config_setting_name(entry);

		struct item_data *idata = itemdb->name2id(name);
		if (idata == NULL) {
			ShowWarning("%s: unknown materials item '%s' for entry %d in '%s', skipping...\n", __func__, name, n , source);
			continue;
		}

		int i32 = 0;
		if ((i32 = libconfig->setting_get_int(entry)) == CONFIG_TRUE && (i32 <= 0 || i32 > MAX_AMOUNT)) {
			ShowWarning("%s: invalid materials amount (%d) for entry %d in '%s', skipping...\n", __func__, i32, n, source);
			continue;
		}

		struct itemlist_entry mitem = { 0 };
		mitem.id = idata->nameid;
		mitem.amount = i32;
		VECTOR_ENSURE(*materials, 1, 1);
		VECTOR_PUSH(*materials, mitem);
	}
	return true;
}

static bool enchantui_read_db_libconfig_itemrate_list(const struct config_setting_t *it, struct enchant_item_list *elist, int n, const char *source)
{
	nullpo_retr(false, it);
	nullpo_retr(false, elist);
	nullpo_retr(false, source);

	if (!config_setting_is_group(it)) {
		ShowError("%s: ItemList must be a list for entry %d in '%s', skipping...\n", __func__, n, source);
		return false;
	}

	VECTOR_INIT(*elist);

	int i = 0;
	struct config_setting_t *entry = NULL;
	while ((entry = libconfig->setting_get_elem(it, i++)) != NULL) {
		const char *name = config_setting_name(entry);

		struct item_data *idata = itemdb->name2id(name);
		if (idata == NULL) {
			ShowWarning("%s: unknown ItemList item '%s' for entry %d in '%s', skipping...\n", __func__, name, n, source);
			continue;
		}

		int i32 = 0;
		if ((i32 = libconfig->setting_get_int(entry)) == CONFIG_TRUE && (i32 <= 0 || i32 > MAX_AMOUNT)) {
			ShowWarning("%s: invalid ItemList amount (%d) for entry %d in '%s', skipping...\n", __func__, i32, n, source);
			continue;
		}

		struct enchant_item_rate_entry ritem = { 0 };
		ritem.id = idata->nameid;
		ritem.rate = i32;
		VECTOR_ENSURE(*elist, 1, 1);
		VECTOR_PUSH(*elist, ritem);
	}
	return true;
}

/**
 * Validates the existence of the enchant group, and the target item requirement.
 *
 * @param[in] sd                  a pointer to map_session_data.
 * @param[in] enchant_group       the enchantment group id.
 * @param[in] index               the target item index.
 * @return pointer to the enchant info data or NULL on failure.
 */
static const struct enchant_info *enchantui_validate_targetitem(struct map_session_data *sd, int64 enchant_group, int index)
{
	nullpo_retr(NULL, sd);

	// Validate the item index
	if (index < 0 || index >= sd->status.inventorySize || sd->status.inventory[index].nameid <= 0 || sd->inventory_data[index] == NULL)
		return NULL;

	// Validate the enchant group
	const struct enchant_info *ei = enchantui->exists(enchant_group);
	if (ei == NULL)
		return NULL;

	// Validate the existence of the item in the group
	int i = 0;
	ARR_FIND(0, VECTOR_LENGTH(ei->TargetItems), i, VECTOR_INDEX(ei->TargetItems, i) == sd->status.inventory[index].nameid);
	if (i == VECTOR_LENGTH(ei->TargetItems))
		return NULL;

	// Validate target item requirements for the group
	if (ei->MinGrade > sd->status.inventory[index].grade)
		return NULL;

	if (ei->MinRefine > sd->status.inventory[index].refine)
		return NULL;

	if (!ei->AllowRandomOptions) {
		int option_count = 0;
		for (int j = 0; j < MAX_ITEM_OPTIONS; ++j) {
			if (sd->status.inventory[index].option[j].index != 0)
				++option_count;
		}
		if (option_count > 0)
			return NULL;
	}

	return ei;
}

/**
 * Validates the item next slot id to be enchanted.
 *
 * @param[in] sd			a pointer to map_session_data.
 * @param[in] ei			a pointer to enchant_info.
 * @param[in] it			a pointer to item.
 * @return the available enchant slot id or INDEX_NOT_FOUND on failure.
 */
static int enchantui_validate_slot_id(struct map_session_data *sd, const struct enchant_info *ei, const struct item *it)
{
	nullpo_retr(INDEX_NOT_FOUND, sd);
	nullpo_retr(INDEX_NOT_FOUND, ei);
	nullpo_retr(INDEX_NOT_FOUND, it);

	// Special items cannot be enchanted.
	if (itemdb_isspecial(it->card[0]))
		return INDEX_NOT_FOUND;

	// Validate the availability of the slot
	int slot_count = 0;
	for (int i = 0; i < VECTOR_LENGTH(ei->SlotOrder); ++i) {
		if (it->card[VECTOR_INDEX(ei->SlotOrder, i)] != 0)
			++slot_count;
	}

	// If all available enchanting slots are filled
	if (slot_count >= VECTOR_LENGTH(ei->SlotOrder))
		return INDEX_NOT_FOUND;

	// Pick the next slot available for enchant
	return VECTOR_INDEX(ei->SlotOrder, slot_count);
}

/**
 * Validates the enchant requirements
 *
 * @param[in] sd			a pointer to map_session_data.
 * @param[in] zeny		    zeny amount.
 * @param[in] materials		a pointer to itemlist.
 * @return true if all items are available or false if not.
 */
static bool enchantui_validate_requirements(struct map_session_data *sd, const int zeny, const struct itemlist *materials)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, materials);

	if (sd->status.zeny < zeny)
		return false;

	if (!enchantui->validate_material_list(sd, materials))
		return false;

	pc->payzeny(sd, zeny, LOG_TYPE_NPC, NULL);
	enchantui->consume_material_list(sd, materials);
	return true;
}

/**
 * Validates that the player has all the items in the materials list in his inventory.
 *
 * @param[in] sd			a pointer to map_session_data.
 * @param[in] materials		a pointer to itemlist.
 * @return true if all items are available or false if not.
 */
static bool enchantui_validate_material_list(struct map_session_data *sd, const struct itemlist *materials)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, materials);

	for (int i = 0; i < VECTOR_LENGTH(*materials); ++i) {
		const struct itemlist_entry *entry = &VECTOR_INDEX(*materials, i);
		int mat_idx = pc->search_inventory(sd, entry->id);

		if (mat_idx == INDEX_NOT_FOUND || sd->status.inventory[mat_idx].amount < entry->amount)
			return false;
	}
	return true;
}

/**
 * Consumes the materials list from the player inventory
 *
 * @param[in] sd			a pointer to map_session_data.
 * @param[in] materials		a pointer to itemlist.
 * @return true if all items are consumed or false if not.
 */
static bool enchantui_consume_material_list(struct map_session_data *sd, const struct itemlist *materials)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, materials);

	for (int i = 0; i < VECTOR_LENGTH(*materials); ++i) {
		const struct itemlist_entry *entry = &VECTOR_INDEX(*materials, i);
		int mat_idx = pc->search_inventory(sd, entry->id);

		if (pc->delitem(sd, mat_idx, entry->amount, 0, DELITEM_NORMAL, LOG_TYPE_NPC) != 0)
			return false;
	}
	return true;
}

/**
 * Processes a normal enchantment request.
 *
 * @param[in] sd                  a pointer to map_session_data.
 * @param[in] enchant_group       the enchantment group id.
 * @param[in] index               the target item index.
 */
static void enchantui_normal_request(struct map_session_data *sd, int64 enchant_group, int index)
{
	nullpo_retv(sd);

	const struct enchant_info *ei = enchantui->validate_targetitem(sd, enchant_group, index);
	if (ei == NULL)
		return;

	struct item *it = &sd->status.inventory[index];

	// Get the slot id for enchanting
	const int slot_id = enchantui->validate_slot_id(sd, ei, it);
	if (slot_id == INDEX_NOT_FOUND)
		return;

	// Process the normal enchant
	const struct enchant_info_normal *ei_normal = &ei->SlotInfo[slot_id].NormalEnchants;

	// Validate existence of enchants for the item grade
	const int enchant_count = VECTOR_LENGTH(ei_normal->ItemList[(int)it->grade]);
	if (enchant_count == 0)
		return;

	// Validate the requirements
	if (!enchantui->validate_requirements(sd, ei_normal->Zeny, &ei_normal->Materials))
		return;

	// Calculate the enchant success rate!
	if (rnd() % 100000 > (ei->SlotInfo[slot_id].SuccessRate + ei->SlotInfo[slot_id].GradeBonus[(int)it->grade])) {
		clif->enchantui_status(sd, ENCHANTUI_FAILURE, 0);
		return;
	}

	// Roll for an enchant
	int enchant_itemid = 0;
	{
		// Limit the iterations to avoid lag
		int count = battle->bc->enchant_ui_max_loop * enchant_count + (rnd() % enchant_count);
		int idx = 0;
		while (count > 0 && rnd() % 100000 >= VECTOR_INDEX(ei_normal->ItemList[(int)it->grade], idx).rate) {
			idx = (idx + 1) % enchant_count;
			--count;
		}

		enchant_itemid = VECTOR_INDEX(ei_normal->ItemList[(int)it->grade], idx).id;
	}

	// Enchant the item
	it->card[slot_id] = enchant_itemid;

	// Update the client
	clif->enchantui_status(sd, ENCHANTUI_SUCCESS, enchant_itemid);
}

/**
 * Processes a perfect enchantment request.
 *
 * @param[in] sd                  a pointer to map_session_data.
 * @param[in] enchant_group       the enchantment group id.
 * @param[in] index               the target item index.
 * @param[in] itemid              item id of the perfect enchantment.
 */
static void enchantui_perfect_request(struct map_session_data *sd, int64 enchant_group, int index, int itemid)
{
	nullpo_retv(sd);

	const struct enchant_info *ei = enchantui->validate_targetitem(sd, enchant_group, index);
	if (ei == NULL)
		return;

	struct item *it = &sd->status.inventory[index];

	// Get the slot id for enchanting
	const int slot_id = enchantui->validate_slot_id(sd, ei, it);
	if (slot_id == INDEX_NOT_FOUND)
		return;

	// Validate the perfect enchant id
	const struct enchant_info_perfect *ei_perfect = &ei->SlotInfo[slot_id].PerfectEnchants;
	const struct enchant_info_perfect_entry *ei_entry = NULL;
	{
		int i = 0;
		ARR_FIND(0, VECTOR_LENGTH(*ei_perfect), i, VECTOR_INDEX(*ei_perfect, i).BaseItem == itemid);

		// Item not found, ignore the request.
		if (i == VECTOR_LENGTH(*ei_perfect))
			return;
		else
			ei_entry = &VECTOR_INDEX(*ei_perfect, i);
	}

	// Validate the requirements
	if (!enchantui->validate_requirements(sd, ei_entry->Zeny, &ei_entry->Materials))
		return;

	// Calculate the enchant success rate!
	if (rnd() % 100000 > (ei->SlotInfo[slot_id].SuccessRate + ei->SlotInfo[slot_id].GradeBonus[(int)it->grade])) {
		clif->enchantui_status(sd, ENCHANTUI_FAILURE, 0);
		return;
	}

	// Enchant the item
	it->card[slot_id] = itemid;

	// Update the client
	clif->enchantui_status(sd, ENCHANTUI_SUCCESS, itemid);
}

/**
 * Processes an enchantment upgrade request.
 *
 * @param[in] sd                  a pointer to map_session_data.
 * @param[in] enchant_group       the enchantment group id.
 * @param[in] index               the target item index.
 * @param[in] slot                slot id of the upgrade.
 */
static void enchantui_upgrade_request(struct map_session_data *sd, int64 enchant_group, int index, int slot_id)
{
	nullpo_retv(sd);

	const struct enchant_info *ei = enchantui->validate_targetitem(sd, enchant_group, index);
	if (ei == NULL)
		return;

	// Bad slot id!
	if (slot_id < 0 || slot_id > MAX_SLOTS)
		return;

	struct item *it = &sd->status.inventory[index];

	// Special items cannot be enchanted.
	if (itemdb_isspecial(it->card[0]))
		return;

	// Validate the upgrade enchant id
	const struct enchant_info_upgrade *ei_upgrade = &ei->SlotInfo[slot_id].UpgradeInfo;
	const struct enchant_info_upgrade_entry *ei_entry = NULL;
	{
		int i = 0;
		ARR_FIND(0, VECTOR_LENGTH(*ei_upgrade), i, VECTOR_INDEX(*ei_upgrade, i).BaseItem == it->card[slot_id]);

		// Item not found, ignore the request.
		if (i == VECTOR_LENGTH(*ei_upgrade))
			return;
		else
			ei_entry = &VECTOR_INDEX(*ei_upgrade, i);
	}

	// Validate the requirements
	if (!enchantui->validate_requirements(sd, ei_entry->Zeny, &ei_entry->Materials))
		return;

	// Calculate the enchant success rate!
	if (rnd() % 100000 > (ei->SlotInfo[slot_id].SuccessRate + ei->SlotInfo[slot_id].GradeBonus[(int)it->grade])) {
		clif->enchantui_status(sd, ENCHANTUI_FAILURE, 0);
		return;
	}

	// Upgrade the enchant
	it->card[slot_id] = ei_entry->ResultItem;

	// Update the client
	clif->enchantui_status(sd, ENCHANTUI_SUCCESS, ei_entry->ResultItem);
}

/**
 * Processes a enchantment reset request.
 *
 * @param[in] sd                  a pointer to map_session_data.
 * @param[in] enchant_group       the enchantment group id.
 * @param[in] index               the target item index.
 */
static void enchantui_reset_request(struct map_session_data *sd, int64 enchant_group, int index)
{
	nullpo_retv(sd);

	const struct enchant_info *ei = enchantui->validate_targetitem(sd, enchant_group, index);
	if (ei == NULL)
		return;

	struct item *it = &sd->status.inventory[index];

	// Process enchant reset
	const struct enchant_info_reset *ei_reset = &ei->ResetInfo;

	// Validate the requirements
	if (!enchantui->validate_requirements(sd, ei_reset->Zeny, &ei_reset->Materials))
		return;

	// Calculate the reset success rate!
	if (rnd() % 100000 > ei_reset->Rate) {
		clif->enchantui_status(sd, ENCHANTUI_FAILURE, 0);
		return;
	}

	// Reset the enchants
	for (int i = 0; i < VECTOR_LENGTH(ei->SlotOrder); ++i)
		it->card[VECTOR_INDEX(ei->SlotOrder, i)] = 0;

	// Update the client
	clif->enchantui_status(sd, ENCHANTUI_SUCCESS, 0);
}

static struct enchant_info *enchantui_db_exists(int64 id)
{
	return (struct enchant_info *)idb_get(enchantui->db, (int)id);
}

static int enchantui_db_final_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct enchant_info *ei = DB->data2ptr(data);

	VECTOR_CLEAR(ei->SlotOrder);
	VECTOR_CLEAR(ei->TargetItems);
	VECTOR_CLEAR(ei->ResetInfo.Materials);

	for (int i = 0; i < MAX_SLOTS; ++i) {
		for (int j = 0; j < MAX_ITEM_GRADE; ++j)
			VECTOR_CLEAR(ei->SlotInfo[i].NormalEnchants.ItemList[j]);
		VECTOR_CLEAR(ei->SlotInfo[i].NormalEnchants.Materials);

		for (int j = 0; j < VECTOR_LENGTH(ei->SlotInfo[i].PerfectEnchants); ++j)
			VECTOR_CLEAR(VECTOR_INDEX(ei->SlotInfo[i].PerfectEnchants, j).Materials);
		VECTOR_CLEAR(ei->SlotInfo[i].PerfectEnchants);

		for (int j = 0; j < VECTOR_LENGTH(ei->SlotInfo[i].UpgradeInfo); ++j)
			VECTOR_CLEAR(VECTOR_INDEX(ei->SlotInfo[i].UpgradeInfo, j).Materials);
		VECTOR_CLEAR(ei->SlotInfo[i].UpgradeInfo);
	}
	return 0;
}

static int do_init_enchantui(bool minimal)
{
	if (minimal)
		return 0;

	enchantui->db = idb_alloc(DB_OPT_RELEASE_DATA);
	enchantui->read_db_libconfig();
	return 0;
}

static void do_final_enchantui(void)
{
	enchantui->db->destroy(enchantui->db, enchantui->db_final_sub);
}

void enchantui_defaults(void)
{
	enchantui = &enchantui_s;

	/* core */
	enchantui->init = do_init_enchantui;
	enchantui->final = do_final_enchantui;
	enchantui->db_final_sub = enchantui_db_final_sub;
	enchantui->exists = enchantui_db_exists;

	/* database */
	enchantui->read_db_libconfig = enchantui_read_db_libconfig;
	enchantui->read_db_libconfig_sub = enchantui_read_db_libconfig_sub;
	enchantui->read_db_libconfig_slot_order = enchantui_read_db_libconfig_slot_order;
	enchantui->read_db_libconfig_target_items = enchantui_read_db_libconfig_target_items;
	enchantui->read_db_libconfig_slot_info = enchantui_read_db_libconfig_slot_info;
	enchantui->read_db_libconfig_slot_info_sub = enchantui_read_db_libconfig_slot_info_sub;
	enchantui->read_db_libconfig_gradebonus = enchantui_read_db_libconfig_gradebonus;
	enchantui->read_db_libconfig_normal_info = enchantui_read_db_libconfig_normal_info;
	enchantui->read_db_libconfig_perfect_info = enchantui_read_db_libconfig_perfect_info;
	enchantui->read_db_libconfig_upgrade_info = enchantui_read_db_libconfig_upgrade_info;
	enchantui->read_db_libconfig_reset_info = enchantui_read_db_libconfig_reset_info;
	enchantui->read_db_libconfig_materials_list = enchantui_read_db_libconfig_materials_list;
	enchantui->read_db_libconfig_itemrate_list = enchantui_read_db_libconfig_itemrate_list;

	/* processing requests */
	enchantui->validate_targetitem = enchantui_validate_targetitem;
	enchantui->validate_slot_id = enchantui_validate_slot_id;
	enchantui->validate_requirements = enchantui_validate_requirements;
	enchantui->validate_material_list = enchantui_validate_material_list;
	enchantui->consume_material_list = enchantui_consume_material_list;
	enchantui->normal_request = enchantui_normal_request;
	enchantui->perfect_request = enchantui_perfect_request;
	enchantui->upgrade_request = enchantui_upgrade_request;
	enchantui->reset_request = enchantui_reset_request;
}
