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
#ifndef MAP_GRADER_H
#define MAP_GRADER_H

#include "common/hercules.h"
#include "common/mmo.h"
#include "map/itemdb.h"
#include "map/map.h"

enum grade_level {
    ITEM_GRADE_NONE = 0,
    ITEM_GRADE_D    = 1,
    ITEM_GRADE_C    = 2,
    ITEM_GRADE_B    = 3,
    ITEM_GRADE_A    = 4,
    ITEM_GRADE_R    = 5,
    ITEM_GRADE_S    = 6,
    ITEM_GRADE_SS   = 7,
#ifndef ITEM_GRADE_MAX
    ITEM_GRADE_MAX
#endif
};

STATIC_ASSERT(MAX_ITEM_GRADE == (ITEM_GRADE_MAX - 1), "Maximum item grade mismatch!");

enum grade_ui_failure_behavior {
    GRADE_FAILURE_BEHAVIOR_KEEP      = 0,
    GRADE_FAILURE_BEHAVIOR_DESTROY   = 1,
    GRADE_FAILURE_BEHAVIOR_DOWNGRADE = 2,
};

enum grade_ui_result {
    GRADE_UPGRADE_SUCCESS          = 0,
    GRADE_UPGRADE_FAILED_KEEP      = 1,
    GRADE_UPGRADE_FAILED_DOWNGRADE = 2,
    GRADE_UPGRADE_FAILED_DESTROY   = 3,
    GRADE_UPGRADE_FAILED_PROTECTED = 4,
};

enum grade_announce_condition {
    GRADE_ANNOUNCE_NONE    = 0,
    GRADE_ANNOUNCE_SUCCESS = 1,
    GRADE_ANNOUNCE_FAILURE = 2,
    GRADE_ANNOUNCE_ALWAYS  = GRADE_ANNOUNCE_SUCCESS | GRADE_ANNOUNCE_FAILURE,
};

/* Structures */
struct grade_material {
    int nameid;
    int amount;
    int zeny_cost;
    enum grade_ui_failure_behavior failure_behavior;
};

struct grade_blessing {
    int nameid;
    int amount;
    int bonus;
    int max_blessing;
};

struct s_grade_info {
    int success_chance;
    enum grade_announce_condition announce;
    struct grade_material materials[MAX_GRADE_MATERIALS];
    struct grade_blessing blessing;
};

struct grade_interface_dbs {
    struct s_grade_info grade_info[MAX_ITEM_GRADE];
};

/**
 * grader.c Interface
 **/
struct grader_interface {
    struct grade_interface_dbs *dbs;

    /* core */
    int (*init) (bool minimal);
    void (*final) (void);

    void (*reload_db) (void);
    bool (*read_db_libconfig) (void);
    bool (*read_db_libconfig_sub) (const struct config_setting_t *it, int n, const char *source);
    bool (*read_db_libconfig_sub_materials) (const struct config_setting_t *it, enum grade_level gl, int n, const char *source);
    bool (*read_db_libconfig_sub_material) (const struct config_setting_t *it, struct grade_material *gm, int n, const char *source);
    bool (*read_db_libconfig_sub_blessing) (const struct config_setting_t *it, enum grade_level gl, int n, const char *source);
    bool (*failure_behavior_string2enum) (const char *str, enum grade_ui_failure_behavior *result);
    bool (*announce_behavior_string2enum) (const char *str, enum grade_announce_condition *result);

    const struct s_grade_info *(*get_grade_info) (int grade);
    void (*enchant_add_item) (struct map_session_data *sd, int idx);
    void (*enchant_start) (struct map_session_data *sd, int idx, int mat_idx, bool use_blessing, int blessing_amount);
};

#ifdef HERCULES_CORE
void grader_defaults(void);
#endif // HERCULES_CORE

HPShared struct grader_interface *grader;

#endif /* MAP_GRADER_H */
