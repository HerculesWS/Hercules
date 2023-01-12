/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2018-2023 Hercules Dev Team
* Copyright (C) Smokexyz
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
#ifndef MAP_ACHIEVEMENT_H
#define MAP_ACHIEVEMENT_H

#include "common/hercules.h"
#include "common/db.h"
#include "map/map.h" // enum status_point_types

#define ACHIEVEMENT_NAME_LENGTH 50
#define OBJECTIVE_DESCRIPTION_LENGTH 100

struct achievement;
struct map_session_data;
struct char_achievements;

/**
 * Achievement Types
 */
enum achievement_types {
	// Quest
	ACH_QUEST, // achievement and objective specific
	// PC Kills
	ACH_KILL_PC_TOTAL,
	ACH_KILL_PC_JOB,
	ACH_KILL_PC_JOBTYPE,
	// Mob Kills
	ACH_KILL_MOB_CLASS,
	// PC Damage
	ACH_DAMAGE_PC_MAX,
	ACH_DAMAGE_PC_TOTAL,
	ACH_DAMAGE_PC_REC_MAX,
	ACH_DAMAGE_PC_REC_TOTAL,
	// Mob Damage
	ACH_DAMAGE_MOB_MAX,
	ACH_DAMAGE_MOB_TOTAL,
	ACH_DAMAGE_MOB_REC_MAX,
	ACH_DAMAGE_MOB_REC_TOTAL,
	// Job
	ACH_JOB_CHANGE,
	// Status
	ACH_STATUS,
	ACH_STATUS_BY_JOB,
	ACH_STATUS_BY_JOBTYPE,
	// Chatroom
	ACH_CHATROOM_CREATE_DEAD,
	ACH_CHATROOM_CREATE,
	ACH_CHATROOM_MEMBERS,
	// Friend
	ACH_FRIEND_ADD,
	// Party
	ACH_PARTY_CREATE,
	ACH_PARTY_JOIN,
	// Marriage
	ACH_MARRY,
	// Adoption
	ACH_ADOPT_BABY,
	ACH_ADOPT_PARENT,
	// Zeny
	ACH_ZENY_HOLD,
	ACH_ZENY_GET_ONCE,
	ACH_ZENY_GET_TOTAL,
	ACH_ZENY_SPEND_ONCE,
	ACH_ZENY_SPEND_TOTAL,
	// Equipment
	ACH_EQUIP_REFINE_SUCCESS,
	ACH_EQUIP_REFINE_FAILURE,
	ACH_EQUIP_REFINE_SUCCESS_TOTAL,
	ACH_EQUIP_REFINE_FAILURE_TOTAL,
	ACH_EQUIP_REFINE_SUCCESS_WLV,
	ACH_EQUIP_REFINE_FAILURE_WLV,
	ACH_EQUIP_REFINE_SUCCESS_ID,
	ACH_EQUIP_REFINE_FAILURE_ID,
	// Items
	ACH_ITEM_GET_COUNT,
	ACH_ITEM_GET_COUNT_ITEMTYPE,
	ACH_ITEM_GET_WORTH,
	ACH_ITEM_SELL_WORTH,
	// Monsters
	ACH_PET_CREATE,
	// Achievement
	ACH_ACHIEVE,
	ACH_ACHIEVEMENT_RANK,
	ACH_TYPE_MAX
};

enum unique_criteria_types {
	CRITERIA_UNIQUE_NONE,
	CRITERIA_UNIQUE_ACHIEVE_ID,
	CRITERIA_UNIQUE_ITEM_ID,
	CRITERIA_UNIQUE_STATUS_TYPE,
	CRITERIA_UNIQUE_WEAPON_LV
};

/**
 * Achievement Objective Structure
 *
 * @see achievement_type_requires_criteria()
 * @see achievement_criteria_mobid()
 * @see achievement_criteria_jobid()
 * @see achievement_criteria_itemid()
 * @see achievement_criteria_stattype()
 * @see achievement_criteria_itemtype()
 * @see achievement_criteria_weaponlv()
 */
struct achievement_objective {
	int goal;
	char description[OBJECTIVE_DESCRIPTION_LENGTH];
	/**
	 * Those criteria that do not make sense if stacked together.
	 * union identifier is set in unique_type. (@see unique_criteria_type)
	 */
	union {
		int achieve_id;
		int itemid;
		enum status_point_types status_type;
		int weapon_lv;
	} unique;
	enum unique_criteria_types unique_type;
	/* */
	uint32 item_type;
	int mobid;
	VECTOR_DECL(int) jobid;
};

struct achievement_reward_item {
	int id, amount;
};

struct achievement_rewards {
	int title_id;
	struct script_code *bonus;
	VECTOR_DECL(struct achievement_reward_item) item;
};

/**
 * Achievement Data Structure
 */
struct achievement_data {
	int id;
	char name[ACHIEVEMENT_NAME_LENGTH];
	enum achievement_types type;
	int points;
	VECTOR_DECL(struct achievement_objective) objective;
	struct achievement_rewards rewards;
};

// Achievements types that use Mob ID as criteria.
#define achievement_criteria_mobid(t) ( \
		   (t) == ACH_KILL_MOB_CLASS \
		|| (t) == ACH_PET_CREATE )

// Achievements types that use JobID vector as criteria.
#define achievement_criteria_jobid(t) ( \
		   (t) == ACH_KILL_PC_JOB \
		|| (t) == ACH_KILL_PC_JOBTYPE \
		|| (t) == ACH_JOB_CHANGE \
		|| (t) == ACH_STATUS_BY_JOB \
		|| (t) == ACH_STATUS_BY_JOBTYPE )

// Achievements types that use Item ID as criteria.
#define achievement_criteria_itemid(t) ( \
		   (t) == ACH_ITEM_GET_COUNT \
		|| (t) == ACH_EQUIP_REFINE_SUCCESS_ID \
		|| (t) == ACH_EQUIP_REFINE_FAILURE_ID )

// Achievements types that use status type parameter as criteria.
#define achievement_criteria_stattype(t) ( \
		   (t) == ACH_STATUS \
		|| (t) == ACH_STATUS_BY_JOB \
		|| (t) == ACH_STATUS_BY_JOBTYPE )

// Achievements types that use item type mask as criteria.
#define achievement_criteria_itemtype(t) ( \
		   (t) == ACH_ITEM_GET_COUNT_ITEMTYPE )

// Achievements types that use weapon level as criteria.
#define achievement_criteria_weaponlv(t) ( \
		   (t) == ACH_EQUIP_REFINE_SUCCESS_WLV \
		|| (t) == ACH_EQUIP_REFINE_FAILURE_WLV )

// Valid status types for objective criteria.
#define achievement_valid_status_types(s) ( \
		   (s) ==  SP_STR \
		|| (s) ==  SP_AGI \
		|| (s) ==  SP_VIT \
		|| (s) ==  SP_INT \
		|| (s) ==  SP_DEX \
		|| (s) ==  SP_LUK \
		|| (s) ==  SP_BASELEVEL || (s) ==  SP_JOBLEVEL )

struct achievement_interface {
	struct DBMap *db; // int id -> struct achievement_data *
	/* */
	VECTOR_DECL(int) rank_exp; // Achievement Rank Exp Requirements
	VECTOR_DECL(int) category[ACH_TYPE_MAX]; /* A collection of Ids per type for faster processing. */
	/* */
	void (*init) (bool minimal);
	void (*final) (void);
	/* */
	int (*db_finalize) (union DBKey key, struct DBData *data, va_list args);
	/* */
	void (*readdb)(void);
	/* */
	bool (*readdb_objectives_sub) (const struct config_setting_t *conf, int index, struct achievement_data *entry);
	bool (*readdb_objectives) (const struct config_setting_t *conf, struct achievement_data *entry);
	/* */
	bool (*readdb_validate_criteria_mobid) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	bool (*readdb_validate_criteria_jobid) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	bool (*readdb_validate_criteria_itemid) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	bool (*readdb_validate_criteria_statustype) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	bool (*readdb_validate_criteria_itemtype) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	bool (*readdb_validate_criteria_weaponlv) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	bool (*readdb_validate_criteria_achievement) (const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx);
	/* */
	bool (*readdb_rewards) (const struct config_setting_t *conf, struct achievement_data *entry, const char *source);
	void (*readdb_validate_reward_items) (const struct config_setting_t *t, struct achievement_data *entry);
	bool (*readdb_validate_reward_item_sub) (const struct config_setting_t *t, int element, struct achievement_data *entry);
	void (*readdb_validate_reward_bonus) (const struct config_setting_t *t, struct achievement_data *entry, const char *source);
	void (*readdb_validate_reward_titleid) (const struct config_setting_t *t, struct achievement_data *entry);
	/* */
	void (*readdb_additional_fields) (const struct config_setting_t *conf, struct achievement_data *entry, const char *source);
	/* */
	void (*readdb_ranks) (void);
	/* */
	const struct achievement_data *(*get) (int aid);
	struct achievement *(*ensure) (struct map_session_data *sd, const struct achievement_data *ad);
	/* */
	void (*calculate_totals) (const struct map_session_data *sd, int *points, int *completed, int *rank, int *curr_rank_points);
	bool (*check_complete) (struct map_session_data *sd, const struct achievement_data *ad);
	void (*progress_add) (struct map_session_data *sd, const struct achievement_data *ad, unsigned int obj_idx, int progress);
	void (*progress_set) (struct map_session_data *sd, const struct achievement_data *ad, unsigned int obj_idx, int progress);
	bool (*check_criteria) (const struct achievement_objective *objective, const struct achievement_objective *criteria);
	/* */
	bool (*validate) (struct map_session_data *sd, int aid, unsigned int obj_idx, int progress, bool additive);
	int (*validate_type) (struct map_session_data *sd, enum achievement_types type, const struct achievement_objective *criteria, bool additive);
	/* */
	void (*validate_mob_kill) (struct map_session_data *sd, int mob_id);
	void (*validate_mob_damage) (struct map_session_data *sd, unsigned int damage, bool received);
	void (*validate_pc_kill) (struct map_session_data *sd, struct map_session_data *dstsd);
	void (*validate_pc_damage) (struct map_session_data *sd, struct map_session_data *dstsd, unsigned int damage);
	void (*validate_jobchange) (struct map_session_data *sd);
	void (*validate_stats) (struct map_session_data *sd, enum status_point_types stat_type, int progress);
	void (*validate_chatroom_create) (struct map_session_data *sd);
	void (*validate_chatroom_members) (struct map_session_data *sd, int progress);
	void (*validate_friend_add) (struct map_session_data *sd);
	void (*validate_party_create) (struct map_session_data *sd);
	void (*validate_marry) (struct map_session_data *sd);
	void (*validate_adopt) (struct map_session_data *sd, bool parent);
	void (*validate_zeny) (struct map_session_data *sd, int amount);
	void (*validate_refine) (struct map_session_data *sd, unsigned int idx, bool success);
	void (*validate_item_get) (struct map_session_data *sd, int nameid, int amount);
	void (*validate_item_sell) (struct map_session_data *sd, int nameid, int amount);
	void (*validate_achieve) (struct map_session_data *sd, int achid);
	void (*validate_taming) (struct map_session_data *sd, int class);
	void (*validate_achievement_rank) (struct map_session_data *sd, int rank);
	/* */
	bool (*type_requires_criteria) (enum achievement_types type);
	/* */
	void (*init_titles) (struct map_session_data *sd);
	bool (*check_title) (struct map_session_data *sd, int title_id);
	bool (*get_rewards) (struct map_session_data *sd, const struct achievement_data *ad);
	void (*get_rewards_buffs) (struct map_session_data *sd, const struct achievement_data *ad);
	void (*get_rewards_items) (struct map_session_data *sd, const struct achievement_data *ad);
};

#ifdef HERCULES_CORE
void achievement_defaults(void);
#endif // HERCULES_CORE

HPShared struct achievement_interface *achievement;

#endif // MAP_ACHIEVEMENT_H
