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
#ifndef MAP_ENCHANTUI_H
#define MAP_ENCHANTUI_H

#include "common/db.h"
#include "common/hercules.h"
#include "common/mmo.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/packets_struct.h"

struct config_setting_t;

struct enchant_item_rate_entry {
	int id;
	int rate;
};
VECTOR_STRUCT_DECL(enchant_item_list, struct enchant_item_rate_entry);

struct enchant_info_normal {
	int Zeny;
	struct itemlist Materials;
	struct enchant_item_list ItemList[MAX_ITEM_GRADE];
};

struct enchant_info_perfect_entry {
	int BaseItem;
	int Zeny;
	struct itemlist Materials;
};
VECTOR_STRUCT_DECL(enchant_info_perfect, struct enchant_info_perfect_entry);

struct enchant_info_upgrade_entry {
	int BaseItem;
	int ResultItem;
	int Zeny;
	struct itemlist Materials;
};
VECTOR_STRUCT_DECL(enchant_info_upgrade, struct enchant_info_upgrade_entry);

struct enchant_info_reset {
	int Rate;
	int Zeny;
	struct itemlist Materials;
};

struct enchant_slot_info {
	int SuccessRate;
	int GradeBonus[MAX_ITEM_GRADE];
	struct enchant_info_normal NormalEnchants;
	struct enchant_info_perfect PerfectEnchants;
	struct enchant_info_upgrade UpgradeInfo;
};

struct enchant_info {
	int Id;
	VECTOR_DECL(int) TargetItems;
	int8 MinRefine;
	int8 MinGrade;
	bool AllowRandomOptions;
	VECTOR_DECL(int8) SlotOrder;
	struct enchant_info_reset ResetInfo;
	struct enchant_slot_info SlotInfo[MAX_SLOTS];
};

/**
 * enchantui.c Interface
 **/
struct enchantui_interface {
	/* vars */
	struct DBMap *db; // int enchant_id -> struct enchant_info *

	/* core */
	int (*init) (bool minimal);
	void (*final) (void);

	int (*db_final_sub) (union DBKey key, struct DBData *data, va_list ap);
	struct enchant_info *(*exists) (int64 id);

	/* database */
	void (*read_db_libconfig)(void);
	bool (*read_db_libconfig_sub) (const struct config_setting_t *it, int n, const char *source);
	bool (*read_db_libconfig_slot_order) (const struct config_setting_t *it, struct enchant_info *einfo, int n, const char *source);
	bool (*read_db_libconfig_target_items) (const struct config_setting_t *it, struct enchant_info *einfo, int n, const char *source);
	bool (*read_db_libconfig_slot_info) (const struct config_setting_t *it, struct enchant_info *einfo, int n, const char *source);
	bool (*read_db_libconfig_slot_info_sub) (const struct config_setting_t *it, struct enchant_slot_info *sinfo, int n, const char *source);
	bool (*read_db_libconfig_gradebonus) (const struct config_setting_t *it, struct enchant_slot_info *sinfo, int n, const char *source);
	bool (*read_db_libconfig_normal_info) (const struct config_setting_t *it, struct enchant_info_normal *ninfo, int n, const char *source);
	bool (*read_db_libconfig_perfect_info) (const struct config_setting_t *it, struct enchant_info_perfect *perfect_info, int n, const char *source);
	bool (*read_db_libconfig_upgrade_info) (const struct config_setting_t *it, struct enchant_info_upgrade *uinfo, int n, const char *source);
	bool (*read_db_libconfig_reset_info) (const struct config_setting_t *it, struct enchant_info_reset *rinfo, int n, const char *source);
	bool (*read_db_libconfig_materials_list) (const struct config_setting_t *it, struct itemlist *materials, int n, const char *source);
	bool (*read_db_libconfig_itemrate_list) (const struct config_setting_t *it, struct enchant_item_list *elist, int n, const char *source);

	/* processing requests */
	const struct enchant_info *(*validate_targetitem) (struct map_session_data *sd, int64 enchant_group, int index);
	int (*validate_slot_id) (struct map_session_data *sd, const struct enchant_info *ei, const struct item *it);
	bool (*validate_requirements) (struct map_session_data *sd, const int zeny, const struct itemlist *materials);
	bool (*validate_material_list) (struct map_session_data *sd, const struct itemlist *materials);
	bool (*consume_material_list) (struct map_session_data *sd, const struct itemlist *materials);
	void (*normal_request) (struct map_session_data *sd, int64 enchant_group, int index);
	void (*perfect_request) (struct map_session_data *sd, int64 enchant_group, int index, int itemid);
	void (*upgrade_request) (struct map_session_data *sd, int64 enchant_group, int index, int slot_id);
	void (*reset_request) (struct map_session_data *sd, int64 enchant_group, int index);
};

#ifdef HERCULES_CORE
void enchantui_defaults(void);
#endif // HERCULES_CORE

HPShared struct enchantui_interface *enchantui;

#endif /* MAP_ENCHANTUI_H */
