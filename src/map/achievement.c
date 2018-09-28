/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2017  Hercules Dev Team
* Copyright (C) Smokexyz
* Copyright (C) Dastgir
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

#include "map/achievement.h"

#include "map/itemdb.h"
#include "map/mob.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/script.h"

#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct achievement_interface achievement_s;
struct achievement_interface *achievement;

/**
 * Retrieve an achievement via it's ID.
 * @param aid as the achievement ID.
 * @return NULL or pointer to the achievement data.
 */
static const struct achievement_data *achievement_get(int aid)
{
	return (struct achievement_data *) idb_get(achievement->db, aid);
}

/**
 * Searches the provided achievement data for an achievement,
 * optionally creates a new one if no key exists.
 * @param[in] sd       a pointer to map_session_data.
 * @param[in] aid      ID of the achievement provided as key.
 * @param[in] create   new key creation flag.
 * @return pointer to the session's achievement data.
 */
static struct achievement *achievement_ensure(struct map_session_data *sd, const struct achievement_data *ad)
{
	struct achievement *s_ad = NULL;
	int i = 0;

	nullpo_retr(NULL, sd);
	nullpo_retr(NULL, ad);

	/* Lookup for achievement entry */
	ARR_FIND(0, VECTOR_LENGTH(sd->achievement), i, (s_ad = &VECTOR_INDEX(sd->achievement, i)) && s_ad->id == ad->id);

	if (i == VECTOR_LENGTH(sd->achievement)) {
		struct achievement ta = { 0 };
		ta.id = ad->id;

		VECTOR_ENSURE(sd->achievement, 1, 1);
		VECTOR_PUSH(sd->achievement, ta);

		s_ad = &VECTOR_LAST(sd->achievement);
	}

	return s_ad;
}

/**
 * Calculates the achievement's totals via reference.
 * @param[in] sd               pointer to map_session_data
 * @param[out] tota_points      pointer to total points var
 * @param[out] completed        pointer to total var
 * @param[out] rank             pointer to completed var
 * @param[out] curr_rank_points pointer to achievement rank var
 */
static void achievement_calculate_totals(const struct map_session_data *sd, int *total_points, int *completed, int *rank, int *curr_rank_points)
{
	const struct achievement *a = NULL;
	const struct achievement_data *ad = NULL;
	int tmp_curr_points = 0;
	int tmp_total_points = 0;
	int tmp_total_completed = 0;
	int tmp_rank = 0;
	int i = 0;

	nullpo_retv(sd);

	for (i = 0; i < VECTOR_LENGTH(sd->achievement); i++) {
		a = &VECTOR_INDEX(sd->achievement, i);

		if ((ad = achievement->get(a->id)) == NULL)
			continue;

		if (a->completed_at != 0) {
			tmp_total_points += ad->points;
			tmp_total_completed++;
		}
	}

	if (tmp_total_points > 0) {
		tmp_curr_points = tmp_total_points;
		for (i = 0; i < MAX_ACHIEVEMENT_RANKS
			 && tmp_curr_points >= VECTOR_INDEX(achievement->rank_exp, i)
			 && i < VECTOR_LENGTH(achievement->rank_exp); i++) {
			tmp_curr_points -= VECTOR_INDEX(achievement->rank_exp, i);
			tmp_rank++;
		}
	}

	if (total_points != NULL)
		*total_points = tmp_total_points;

	if (completed != NULL)
		*completed = tmp_total_completed;

	if (rank != NULL)
		*rank = tmp_rank;

	if (curr_rank_points != NULL)
		*curr_rank_points = tmp_curr_points;
}

/**
 * Checks whether all objectives of the achievement are completed.
 * @param[in] sd       as the map_session_data pointer
 * @param[in] ad       as the achievement_data pointer
 * @return true if complete, false if not.
 */
static bool achievement_check_complete(struct map_session_data *sd, const struct achievement_data *ad)
{
	int i;
	struct achievement *ach = NULL;

	nullpo_retr(false, sd);
	nullpo_retr(false, ad);

	if ((ach = achievement->ensure(sd, ad)) == NULL)
		return false;
	for (i = 0; i < VECTOR_LENGTH(ad->objective); i++)
		if (ach->objective[i] < VECTOR_INDEX(ad->objective, i).goal)
			return false;

	return true;
}

/**
 * Compares the progress of an objective against it's goal.
 * Increments the progress of the objective by the specified amount, towards the goal.
 * @param sd         [in] as a pointer to map_session_data
 * @param ad         [in] as a pointer to the achievement_data
 * @param obj_idx    [in] as the index of the objective.
 * @param progress   [in] as the progress of the objective to be added.
 */
static void achievement_progress_add(struct map_session_data *sd, const struct achievement_data *ad, unsigned int obj_idx, int progress)
{
	struct achievement *ach = NULL;

	nullpo_retv(sd);
	nullpo_retv(ad);

	Assert_retv(progress != 0);
	Assert_retv(obj_idx < VECTOR_LENGTH(ad->objective));

	if ((ach = achievement->ensure(sd, ad)) == NULL)
		return;

	if (ach->completed_at)
		return; // ignore the call if the achievement is completed.

	// Check and increment the objective count.
	if (!ach->objective[obj_idx] || ach->objective[obj_idx] < VECTOR_INDEX(ad->objective, obj_idx).goal) {
		ach->objective[obj_idx] = min(progress + ach->objective[obj_idx], VECTOR_INDEX(ad->objective, obj_idx).goal);

		// Check if the Achievement is complete.
		if (achievement->check_complete(sd, ad)) {
			achievement->validate_achieve(sd, ad->id);
			ach->completed_at = time(NULL);
		}

		// update client.
		clif->achievement_send_update(sd->fd, sd, ad);
	}
}

/**
 * Compare an absolute progress value against the goal of an objective.
 * Does not add/increase progress.
 * @param sd          [in] pointer to map-server session data.
 * @param ad          [in] pointer to achievement data.
 * @param obj_idx     [in] index of the objective in question.
 * @param progress  progress of the objective in question.
 */
static void achievement_progress_set(struct map_session_data *sd, const struct achievement_data *ad, unsigned int obj_idx, int progress)
{
	struct achievement *ach = NULL;

	nullpo_retv(sd);
	nullpo_retv(ad);

	Assert_retv(progress != 0);
	Assert_retv(obj_idx < VECTOR_LENGTH(ad->objective));

	if (progress >= VECTOR_INDEX(ad->objective, obj_idx).goal) {

		if ((ach = achievement->ensure(sd, ad)) == NULL)
			return;

		if (ach->completed_at)
			return;

		ach->objective[obj_idx] = VECTOR_INDEX(ad->objective, obj_idx).goal;

		if (achievement->check_complete(sd, ad)) {
			achievement->validate_achieve(sd, ad->id);
			ach->completed_at = time(NULL);
		}

		clif->achievement_send_update(sd->fd, sd, ad);
	}
}

/**
* Checks if the given criteria satisfies the achievement's objective.
* @param objective   [in] pointer to the achievement's objectives data.
* @param criteria    [in] pointer to the current session's criteria as a comparand.
* @return true if all criteria are satisfied, else false.
*/
static bool achievement_check_criteria(const struct achievement_objective *objective, const struct achievement_objective *criteria)
{
	int i = 0, j = 0;

	nullpo_retr(false, objective);
	nullpo_retr(false, criteria);

	/* Item Id */
	if (objective->unique_type == CRITERIA_UNIQUE_ITEM_ID && objective->unique.itemid != criteria->unique.itemid)
		return false;
	/* Weapon Level */
	else if (objective->unique_type == CRITERIA_UNIQUE_WEAPON_LV && objective->unique.weapon_lv != criteria->unique.weapon_lv)
		return false;
	/* Status Types */
	else if (objective->unique_type == CRITERIA_UNIQUE_STATUS_TYPE && objective->unique.status_type != criteria->unique.status_type)
		return false;
	/* Achievement Id */
	else if (objective->unique_type == CRITERIA_UNIQUE_ACHIEVE_ID && objective->unique.achieve_id != criteria->unique.achieve_id)
		return false;

	/* Monster Id */
	if (objective->mobid > 0 && objective->mobid != criteria->mobid)
		return false;

	/* Item Type */
	if (objective->item_type > 0 && (objective->item_type & (2 << criteria->item_type)) == 0)
		return false;

	/* Job Ids */
	for (i = 0; i < VECTOR_LENGTH(objective->jobid); i++) {
		ARR_FIND(0, VECTOR_LENGTH(criteria->jobid), j, VECTOR_INDEX(criteria->jobid, j) != VECTOR_INDEX(objective->jobid, i));
		if (j < VECTOR_LENGTH(criteria->jobid))
			return false;
	}

	return true;
}

/**
 * Validates an Achievement Objective of similar types.
 * @param[in] sd         as a pointer to the map session data.
 * @param[in] type       as the type of the achievement.
 * @param[in] criteria   as the criteria of the objective (mob id, job id etc.. 0 for no criteria).
 * @param[in] progress   as the current progress of the objective.
 * @return total number of updated achievements on success, 0 on failure.
 */
static int achievement_validate_type(struct map_session_data *sd, enum achievement_types type, const struct achievement_objective *criteria, bool additive)
{
	int i = 0, total = 0;
	struct achievement *ach = NULL;

	nullpo_ret(sd);
	nullpo_ret(criteria);

	Assert_ret(criteria->goal != 0);

	if (type == ACH_QUEST) {
		ShowError("achievement_validate_type: ACH_QUEST is not handled by this function. (use achievement_validate())\n");
		return 0;
	} else if (type >= ACH_TYPE_MAX) {
		ShowError("achievement_validate_type: Invalid Achievement Type %d! (min: %d, max: %d)\n", (int)type, (int)ACH_QUEST, (int)ACH_TYPE_MAX - 1);
		return 0;
	}

	/* Loop through all achievements of the type, checking for possible matches. */
	for (i = 0; i < VECTOR_LENGTH(achievement->category[type]); i++) {
		int j = 0;
		bool updated = false;
		const struct achievement_data *ad = NULL;

		if ((ad = achievement->get(VECTOR_INDEX(achievement->category[type], i))) == NULL)
			continue;

		for (j = 0; j < VECTOR_LENGTH(ad->objective); j++) {
			// Check if objective criteria matches.
			if (achievement->check_criteria(&VECTOR_INDEX(ad->objective, j), criteria) == false)
				continue;
			// Ensure availability of the achievement.
			if ((ach = achievement->ensure(sd, ad)) == NULL)
				return false;
			// Criteria passed, check if not completed and update progress.
			if ((ach->completed_at == 0 && ach->objective[j] < VECTOR_INDEX(ad->objective, j).goal)) {
				if (additive == true)
					achievement->progress_add(sd, ad, j, criteria->goal);
				else
					achievement->progress_set(sd, ad, j, criteria->goal);
				updated = true;
			}
		}

		if (updated == true)
			total++;
	}

	return total;
}

/**
 * Validates any achievement's specific objective index.
 * @param[in] sd         pointer to the session data.
 * @param[in] aid        ID of the achievement.
 * @param[in] index      index of the objective.
 * @param[in] progress   progress to be added towards the goal.
 */
static bool achievement_validate(struct map_session_data *sd, int aid, unsigned int obj_idx, int progress, bool additive)
{
	const struct achievement_data *ad = NULL;
	struct achievement *ach = NULL;

	nullpo_retr(false, sd);
	Assert_retr(false, progress > 0);
	Assert_retr(false, obj_idx < MAX_ACHIEVEMENT_OBJECTIVES);

	if ((ad = achievement->get(aid)) == NULL) {
		ShowError("achievement_validate: Invalid Achievement %d provided.", aid);
		return false;
	}

	// Ensure availability of the achievement.
	if ((ach = achievement->ensure(sd, ad)) == NULL)
		return false;

	// Check if not completed and update progress.
	if ((!ach->completed_at && ach->objective[obj_idx] < VECTOR_INDEX(ad->objective, obj_idx).goal)) {
		if (additive == true)
			achievement->progress_add(sd, ad, obj_idx, progress);
		else
			achievement->progress_set(sd, ad, obj_idx, progress);
	}

	return true;
}

/**
 * Validates monster kill type objectives.
 * @type ACH_KILL_MOB_CLASS
 * @param[in] sd          pointer to session data.
 * @param[in] mob_id      (criteria) class of the monster checked for.
 * @param[in] progress    (goal) progress to be added.
 * @see achievement_vaildate_type()
 */
static void achievement_validate_mob_kill(struct map_session_data *sd, int mob_id)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);
	Assert_retv(mob_id > 0 && mob->db(mob_id) != NULL);

	if (sd->achievements_received == false)
		return;

	criteria.mobid = mob_id;
	criteria.goal = 1;

	achievement->validate_type(sd, ACH_KILL_MOB_CLASS, &criteria, true);
}

/**
 * Validate monster damage type objectives.
 * @types ACH_DAMAGE_MOB_REC_MAX
 *        ACH_DAMAGE_MOB_REC_TOTAL
 * @param[in] sd       pointer to session data.
 * @param[in] damage   amount of damage received/dealt.
 * @param received received/dealt boolean switch.
 */
static void achievement_validate_mob_damage(struct map_session_data *sd, unsigned int damage, bool received)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);
	Assert_retv(damage > 0);

	if (sd->achievements_received == false)
		return;

	criteria.goal = (int) damage;

	if (received) {
		achievement->validate_type(sd, ACH_DAMAGE_MOB_REC_MAX, &criteria, false);
		achievement->validate_type(sd, ACH_DAMAGE_MOB_REC_TOTAL, &criteria, true);
	} else {
		achievement->validate_type(sd, ACH_DAMAGE_MOB_MAX, &criteria, false);
		achievement->validate_type(sd, ACH_DAMAGE_MOB_TOTAL, &criteria, true);
	}

}

/**
 * Validate player kill (PVP) achievements.
 * @types ACH_KILL_PC_TOTAL
 *        ACH_KILL_PC_JOB
 *        ACH_KILL_PC_JOBTYPE
 * @param[in] sd         pointer to killed player's session data.
 * @param[in] dstsd      pointer to killer's session data.
 */
static void achievement_validate_pc_kill(struct map_session_data *sd, struct map_session_data *dstsd)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);
	nullpo_retv(dstsd);

	if (sd->achievements_received == false)
		return;

	criteria.goal = 1;

	/* */
	achievement->validate_type(sd, ACH_KILL_PC_TOTAL, &criteria, true);

	/* */
	VECTOR_INIT(criteria.jobid);
	VECTOR_ENSURE(criteria.jobid, 1, 1);
	VECTOR_PUSH(criteria.jobid, dstsd->status.class);

	/* Job class */
	achievement->validate_type(sd, ACH_KILL_PC_JOB, &criteria, true);
	/* Job Type */
	achievement->validate_type(sd, ACH_KILL_PC_JOBTYPE, &criteria, true);

	VECTOR_CLEAR(criteria.jobid);
}

/**
 * Validate player kill (PVP) achievements.
 * @types ACH_DAMAGE_PC_MAX
 *        ACH_DAMAGE_PC_TOTAL
 *        ACH_DAMAGE_PC_REC_MAX
 *        ACH_DAMAGE_PC_REC_TOTAL
 * @param[in] sd         pointer to source player's session data.
 * @param[in] dstsd      pointer to target player's session data.
 * @param[in] damage     amount of damage dealt / received.
 */
static void achievement_validate_pc_damage(struct map_session_data *sd, struct map_session_data *dstsd, unsigned int damage)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	if (damage == 0)
		return;

	criteria.goal = (int) damage;

	/* */
	achievement->validate_type(sd, ACH_DAMAGE_PC_MAX, &criteria, false);
	achievement->validate_type(sd, ACH_DAMAGE_PC_TOTAL, &criteria, true);

	/* */
	achievement->validate_type(dstsd, ACH_DAMAGE_PC_REC_MAX, &criteria, false);
	achievement->validate_type(dstsd, ACH_DAMAGE_PC_REC_TOTAL, &criteria, true);
}

/**
 * Validates job change objectives.
 * @type ACH_JOB_CHANGE
 * @param[in] sd         pointer to session data.
 * @see achivement_validate_type()
 */
static void achievement_validate_jobchange(struct map_session_data *sd)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	VECTOR_INIT(criteria.jobid);
	VECTOR_ENSURE(criteria.jobid, 1, 1);
	VECTOR_PUSH(criteria.jobid, sd->status.class);

	criteria.goal = 1;

	achievement->validate_type(sd, ACH_JOB_CHANGE, &criteria, false);

	VECTOR_CLEAR(criteria.jobid);
}

/**
 * Validates stat type objectives.
 * @types ACH_STATUS
 *        ACH_STATUS_BY_JOB
 *        ACH_STATUS_BY_JOBTYPE
 * @param[in] sd         pointer to session data.
 * @param[in] stat_type  (criteria) status point type. (see status_point_types)
 * @param[in] progress   (goal) amount of absolute progress to check.
 * @see achievement_validate_type()
 */
static void achievement_validate_stats(struct map_session_data *sd, enum status_point_types stat_type, int progress)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);
	Assert_retv(progress > 0);

	if (sd->achievements_received == false)
		return;

	if (!achievement_valid_status_types(stat_type)) {
		ShowError("achievement_validate_stats: Invalid status type %d given.\n", (int) stat_type);
		return;
	}

	criteria.unique.status_type = stat_type;

	criteria.goal = progress;

	achievement->validate_type(sd, ACH_STATUS, &criteria, false);

	VECTOR_INIT(criteria.jobid);
	VECTOR_ENSURE(criteria.jobid, 1, 1);
	VECTOR_PUSH(criteria.jobid, sd->status.class);

	/* Stat and Job class */
	achievement->validate_type(sd, ACH_STATUS_BY_JOB, &criteria, false);

	/* Stat and Job Type */
	achievement->validate_type(sd, ACH_STATUS_BY_JOBTYPE, &criteria, false);

	VECTOR_CLEAR(criteria.jobid);
}

/**
 * Validates chatroom creation type objectives.
 * @types ACH_CHATROOM_CREATE
 *        ACH_CHATROOM_CREATE_DEAD
 * @param[in] sd    pointer to session data.
 * @see achievement_validate_type()
 */
static void achievement_validate_chatroom_create(struct map_session_data *sd)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	criteria.goal = 1;

	if (pc_isdead(sd)) {
		achievement->validate_type(sd, ACH_CHATROOM_CREATE_DEAD, &criteria, true);
		return;
	}

	achievement->validate_type(sd, ACH_CHATROOM_CREATE, &criteria, true);
}

/**
 * Validates chatroom member count type objectives.
 * @type ACH_CHATROOM_MEMBERS
 * @param[in] sd         pointer to session data.
 * @param[in] progress   (goal) amount of progress to be added.
 * @see achievement_validate_type()
 */
static void achievement_validate_chatroom_members(struct map_session_data *sd, int progress)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	Assert_retv(progress > 0);

	criteria.goal = progress;

	achievement->validate_type(sd, ACH_CHATROOM_MEMBERS, &criteria, false);
}

/**
 * Validates friend add type objectives.
 * @type ACH_FRIEND_ADD
 * @param[in] sd        pointer to session data.
 * @see achievement_validate_type()
 */
static void achievement_validate_friend_add(struct map_session_data *sd)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	criteria.goal = 1;

	achievement->validate_type(sd, ACH_FRIEND_ADD, &criteria, true);
}

/**
 * Validates party creation type objectives.
 * @type ACH_PARTY_CREATE
 * @param[in] sd        pointer to session data.
 * @see achievement_validate_type()
 */
static void achievement_validate_party_create(struct map_session_data *sd)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	criteria.goal = 1;
	achievement->validate_type(sd, ACH_PARTY_CREATE, &criteria, true);
}

/**
 * Validates marriage type objectives.
 * @type ACH_MARRY
 * @param[in] sd        pointer to session data.
 * @see achievement_validate_type()
 */
static void achievement_validate_marry(struct map_session_data *sd)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	criteria.goal = 1;

	achievement->validate_type(sd, ACH_MARRY, &criteria, true);
}

/**
 * Validates adoption type objectives.
 * @types ACH_ADOPT_PARENT
 *        ACH_ADOPT_BABY
 * @param[in] sd        pointer to session data.
 * @param[in] parent    (type) boolean value to indicate if parent (true) or baby (false).
 * @see achievement_validate_type()
 */
static void achievement_validate_adopt(struct map_session_data *sd, bool parent)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	criteria.goal = 1;

	if (parent)
		achievement->validate_type(sd, ACH_ADOPT_PARENT, &criteria, true);
	else
		achievement->validate_type(sd, ACH_ADOPT_BABY, &criteria, true);
}

/**
 * Validates zeny type objectives.
 * @types ACH_ZENY_HOLD
 *        ACH_ZENY_GET_ONCE
 *        ACH_ZENY_GET_TOTAL
 *        ACH_ZENY_SPEND_ONCE
 *        ACH_ZENY_SPEND_TOTAL
 * @param[in] sd        pointer to session data.
 * @param[in] amount    (goal) amount of zeny earned or spent.
 * @see achievement_validate_type()
 */
static void achievement_validate_zeny(struct map_session_data *sd, int amount)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	Assert_retv(amount != 0);

	if (amount > 0) {
		criteria.goal = sd->status.zeny;
		achievement->validate_type(sd, ACH_ZENY_HOLD, &criteria, false);
		criteria.goal = amount;
		achievement->validate_type(sd, ACH_ZENY_GET_ONCE, &criteria, false);
		achievement->validate_type(sd, ACH_ZENY_GET_TOTAL, &criteria, true);
	}  else {
		criteria.goal = -amount;
		achievement->validate_type(sd, ACH_ZENY_SPEND_ONCE, &criteria, false);
		achievement->validate_type(sd, ACH_ZENY_SPEND_TOTAL, &criteria, true);
	}
}

/**
 * Validates equipment refinement type objectives.
 * @types ACH_EQUIP_REFINE_SUCCESS
 *        ACH_EQUIP_REFINE_FAILURE
 *        ACH_EQUIP_REFINE_SUCCESS_TOTAL
 *        ACH_EQUIP_REFINE_FAILURE_TOTAL
 *        ACH_EQUIP_REFINE_SUCCESS_WLV
 *        ACH_EQUIP_REFINE_FAILURE_WLV
 *        ACH_EQUIP_REFINE_SUCCESS_ID
 *        ACH_EQUIP_REFINE_FAILURE_ID
 * @param[in] sd         pointer to session data.
 * @param[in] idx        Inventory index of the item.
 * @param[in] success    (type) boolean switch for failure / success.
 * @see achievement_validate_type()
 */
static void achievement_validate_refine(struct map_session_data *sd, unsigned int idx, bool success)
{
	struct achievement_objective criteria = { 0 };
	struct item_data *id = NULL;

	nullpo_retv(sd);
	Assert_retv(idx < MAX_INVENTORY);

	id = itemdb->exists(sd->status.inventory[idx].nameid);

	if (sd->achievements_received == false)
		return;

	Assert_retv(idx < MAX_INVENTORY);
	Assert_retv(id != NULL);

	criteria.goal = sd->status.inventory[idx].refine;

	/* Universal */
	achievement->validate_type(sd,
			success ? ACH_EQUIP_REFINE_SUCCESS : ACH_EQUIP_REFINE_FAILURE,
			&criteria, false);

	/* Total */
	criteria.goal = 1;
	achievement->validate_type(sd,
			success ? ACH_EQUIP_REFINE_SUCCESS_TOTAL : ACH_EQUIP_REFINE_FAILURE_TOTAL,
			&criteria, true);

	/* By Weapon Level */
	if (id->type == IT_WEAPON) {
		criteria.item_type = id->type;
		criteria.unique.weapon_lv = id->wlv;
		criteria.goal = sd->status.inventory[idx].refine;
		achievement->validate_type(sd,
				success ? ACH_EQUIP_REFINE_SUCCESS_WLV : ACH_EQUIP_REFINE_FAILURE_WLV,
				&criteria, false);
		criteria.item_type = 0;
		criteria.unique.weapon_lv = 0; // cleanup
	}

	/* By NameId */
	criteria.unique.itemid = id->nameid;
	criteria.goal = sd->status.inventory[idx].refine;
	achievement->validate_type(sd,
			success ? ACH_EQUIP_REFINE_SUCCESS_ID : ACH_EQUIP_REFINE_FAILURE_ID,
			&criteria, false);
	criteria.unique.itemid = 0; // cleanup
}

/**
 * Validates item received type objectives.
 * @types ACH_ITEM_GET_COUNT
 *        ACH_ITEM_GET_WORTH
 *        ACH_ITEM_GET_COUNT_ITEMTYPE
 * @param[in] sd         pointer to session data.
 * @param[in] nameid     (criteria) ID of the item.
 * @param[in] amount     (goal) amount of the item collected.
 */
static void achievement_validate_item_get(struct map_session_data *sd, int nameid, int amount)
{
	struct item_data *it = itemdb->exists(nameid);
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	Assert_retv(amount > 0);
	nullpo_retv(it);

	criteria.unique.itemid = it->nameid;
	criteria.goal = amount;
	achievement->validate_type(sd, ACH_ITEM_GET_COUNT, &criteria, false);
	criteria.unique.itemid = 0; // cleanup

	/* Item Buy Value*/
	criteria.goal = max(it->value_buy, 1);
	achievement->validate_type(sd, ACH_ITEM_GET_WORTH, &criteria, false);

	/* Item Type */
	criteria.item_type = it->type;
	criteria.goal = 1;
	achievement->validate_type(sd, ACH_ITEM_GET_COUNT_ITEMTYPE, &criteria, false);
	criteria.item_type = 0; // cleanup
}

/**
 * Validates item sold type objectives.
 * @type ACH_ITEM_SELL_WORTH
 * @param[in] sd         pointer to session data.
 * @param[in] nameid     Item Id in question.
 * @param[in] amount     amount of item in question.
 */
static void achievement_validate_item_sell(struct map_session_data *sd, int nameid, int amount)
{
	struct item_data *it = itemdb->exists(nameid);
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	Assert_retv(amount > 0);
	nullpo_retv(it);

	criteria.unique.itemid = it->nameid;

	criteria.goal = max(it->value_sell, 1);

	achievement->validate_type(sd, ACH_ITEM_SELL_WORTH, &criteria, false);
}

/**
 * Validates achievement type objectives.
 * @type ACH_ACHIEVE
 * @param[in] sd        pointer to session data.
 * @param[in] achid     (criteria) achievement id.
 */
static void achievement_validate_achieve(struct map_session_data *sd, int achid)
{
	const struct achievement_data *ad = achievement->get(achid);
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);
	nullpo_retv(ad);

	if (sd->achievements_received == false)
		return;

	Assert_retv(achid > 0);

	criteria.unique.achieve_id = ad->id;

	criteria.goal = 1;

	achievement->validate_type(sd, ACH_ACHIEVE, &criteria, false);
}

/**
 * Validates taming type objectives.
 * @type ACH_PET_CREATE
 * @param[in] sd        pointer to session data.
 * @param[in] class     (criteria) class of the monster tamed.
 */
static void achievement_validate_taming(struct map_session_data *sd, int class)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	Assert_retv(class > 0);
	Assert_retv(mob->db(class) != mob->dummy);

	criteria.mobid = class;
	criteria.goal = 1;

	achievement->validate_type(sd, ACH_PET_CREATE, &criteria, true);
}

/**
 * Validated achievement rank type objectives.
 * @type ACH_ACHIEVEMENT_RANK
 * @param[in] sd         pointer to session data.
 * @param[in] rank       (goal) rank of earned.
 */
static void achievement_validate_achievement_rank(struct map_session_data *sd, int rank)
{
	struct achievement_objective criteria = { 0 };

	nullpo_retv(sd);

	if (sd->achievements_received == false)
		return;

	Assert_retv(rank >= 0 && rank <= VECTOR_LENGTH(achievement->rank_exp));

	criteria.goal = 1;

	achievement->validate_type(sd, ACH_ACHIEVEMENT_RANK, &criteria, false);
}

/**
 * Verifies if an achievement type requires a criteria field.
 * @param[in] type       achievement type in question.
 * @return true if required, false if not.
 */
static bool achievement_type_requires_criteria(enum achievement_types type)
{
	if (type == ACH_KILL_PC_JOB
		|| type == ACH_KILL_PC_JOBTYPE
		|| type == ACH_KILL_MOB_CLASS
		|| type == ACH_JOB_CHANGE
		|| type == ACH_STATUS
		|| type == ACH_STATUS_BY_JOB
		|| type == ACH_STATUS_BY_JOBTYPE
		|| type == ACH_EQUIP_REFINE_SUCCESS_WLV
		|| type == ACH_EQUIP_REFINE_FAILURE_WLV
		|| type == ACH_EQUIP_REFINE_SUCCESS_ID
		|| type == ACH_EQUIP_REFINE_FAILURE_ID
		|| type == ACH_ITEM_GET_COUNT
		|| type == ACH_PET_CREATE
		|| type == ACH_ACHIEVE)
		return true;

	return false;
}

/**
 * Stores all the title ID that has been earned by player
 * @param[in] sd        pointer to session data.
 */
static void achievement_init_titles(struct map_session_data *sd)
{
	int i;
	nullpo_retv(sd);

	VECTOR_INIT(sd->title_ids);
	/* Browse through the session's achievement list and gather their values. */
	for (i = 0; i < VECTOR_LENGTH(sd->achievement); i++) {
		struct achievement *a = &VECTOR_INDEX(sd->achievement, i);
		const struct achievement_data *ad = NULL;

		/* Sanity check for nonull pointers. */
		if (a == NULL || (ad = achievement->get(a->id)) == NULL)
			continue;

		if (a->completed_at > 0 && a->rewarded_at > 0 && ad->rewards.title_id > 0) {
			VECTOR_ENSURE(sd->title_ids, 1, 1);
			VECTOR_PUSH(sd->title_ids, ad->rewards.title_id);
		}
	}
}

/**
 * Validates whether player has earned the title.
 * @param[in]  sd              pointer to session data.
 * @param[in]  title_id        Title ID
 * @return true, if title has been earned, else false
 */
static bool achievement_check_title(struct map_session_data *sd, int title_id) {
	int i;

	nullpo_retr(false, sd);

	if (title_id == 0)
		return true;

	for (i = 0; i < VECTOR_LENGTH(sd->title_ids); i++) {
		if (VECTOR_INDEX(sd->title_ids, i) == title_id) {
			return true;
		}
	}

	return false;
}

/**
 * Achievement rewards are given to player
 * @param  sd        session data
 * @param  ad        achievement data
 */
static void achievement_get_rewards(struct map_session_data *sd, const struct achievement_data *ad)
{
	int i = 0;
	struct achievement *ach = NULL;

	nullpo_retv(sd);
	nullpo_retv(ad);

	if ((ach = achievement->ensure(sd, ad)) == NULL)
		return;

	/* Buff */
	if (ad->rewards.bonus != NULL)
		script->run(ad->rewards.bonus, 0, sd->bl.id, 0);

	/* Give Items */
	for (i = 0; i < VECTOR_LENGTH(ad->rewards.item); i++) {
		struct item it = { 0 };
		int total = 0;

		it.nameid = VECTOR_INDEX(ad->rewards.item, i).id;
		total = VECTOR_INDEX(ad->rewards.item, i).amount;

		it.identify = 1;

		//Check if it's stackable.
		if (!itemdb->isstackable(it.nameid)) {
			int j = 0;
			for (j = 0; j < total; ++j)
				pc->additem(sd, &it, (it.amount = 1), LOG_TYPE_SCRIPT);
		} else {
			pc->additem(sd, &it, (it.amount = total), LOG_TYPE_SCRIPT);
		}
	}

	ach->rewarded_at = time(NULL);

	if (ad->rewards.title_id > 0) { // Add Title
		VECTOR_ENSURE(sd->title_ids, 1, 1);
		VECTOR_PUSH(sd->title_ids, ad->rewards.title_id);
		clif->achievement_send_list(sd->fd, sd);
	} else {
		clif->achievement_send_update(sd->fd, sd, ad); // send update.
		clif->achievement_reward_ack(sd->fd, sd, ad);
	}
}

/**
 * Parses the Achievement Ranks.
 * @read db/achievement_rank_db.conf
 */
static void achievement_readdb_ranks(void)
{
	const char *filename = "db/achievement_rank_db.conf";
	struct config_t ar_conf = { 0 };
	struct config_setting_t *ardb = NULL, *conf = NULL;
	int entry = 0;

	if (!libconfig->load_file(&ar_conf, filename))
		return; // report error.

	if (!(ardb = libconfig->setting_get_member(ar_conf.root, "achievement_rank_db"))) {
		ShowError("achievement_readdb_ranks: Could not process contents of file '%s', skipping...\n", filename);
		libconfig->destroy(&ar_conf);
		return;
	}

	while (entry < libconfig->setting_length(ardb) && entry < MAX_ACHIEVEMENT_RANKS) {
		char rank[16];

		if (!(conf = libconfig->setting_get_elem(ardb, entry))) {
			ShowError("achievement_readdb_ranks: Could not read value for entry %d, skipping...\n", entry+1);
			continue;
		}

		entry++; // Rank counter;

		sprintf(rank, "Rank%d", entry);

		if (strcmp(rank, config_setting_name(conf)) == 0) {
			int exp = 0;

			if ((exp = libconfig->setting_get_int(conf)) <= 0) {
				ShowError("achievement_readdb_ranks: Invalid value provided for %s in '%s'.\n", rank, filename);
				continue;
			}

			VECTOR_ENSURE(achievement->rank_exp, 1, 1);
			VECTOR_PUSH(achievement->rank_exp, exp);
		} else  {
			ShowWarning("achievement_readdb_ranks: Ranks are not in order! Ignoring all ranks after Rank %d...\n", entry);
			break; // break if elements are not in order or rank doesn't exist.
		}
	}

	if (libconfig->setting_length(ardb) > MAX_ACHIEVEMENT_RANKS)
		ShowWarning("achievement_rankdb_ranks: Maximum number of achievement ranks exceeded. Skipping all after entry %d...\n", entry);

	libconfig->destroy(&ar_conf);

	if (!entry) {
		ShowError("achievement_readdb_ranks: No ranks provided in '%s'!\n", filename);
		return;
	}

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", entry, filename);
}

/**
 * Validates Mob Id criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success
 */
static bool achievement_readdb_validate_criteria_mobid(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int val = 0;
	const char *string = NULL;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);

	if (libconfig->setting_lookup_int(t, "MobId", &val)) {
		if (mob->db_checkid(val) == 0) {
			ShowError("achievement_readdb_validate_criteria_mobid: Non-existant monster with ID %id provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		}

		obj->mobid = val;
	} else if (libconfig->setting_lookup_string(t, "MobId", &string)) {
		if (!script->get_constant(string, &val)) {
			ShowError("achievement_readdb_validate_criteria_mobid: Non-existant constant %s provided (Achievement: %d, Objective: %d). Skipping...\n", string, entry_id, obj_idx);
			return false;
		}

		obj->mobid = val;
	} else if (achievement_criteria_mobid(type)) {
		ShowError("achievement_readdb_validate_criteria_mobid: Achievement type of ID %d requires MobId as objective criteria, setting not provided. Skipping...\n", entry_id);
		return false;
	}

	return true;
}

/**
 * Validates Job Id criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_criteria_jobid(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int job_id = 0;
	const char *string = NULL;
	struct config_setting_t *tt = NULL;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);

	// initialize the buffered objective jobid vector.
	VECTOR_INIT(obj->jobid);

	if (libconfig->setting_lookup_int(t, "JobId", &job_id)) {
		if (pc->jobid2mapid(job_id) == -1) {
			ShowError("achievement_readdb_validate_criteria_jobid: Invalid JobId %d provided (Achievement: %d, Objective: %d). Skipping...\n", job_id, entry_id, obj_idx);
			return false;
		}

		VECTOR_ENSURE(obj->jobid, 1, 1);
		VECTOR_PUSH(obj->jobid, job_id);
	} else if (libconfig->setting_lookup_string(t, "JobId", &string)) {
		if (script->get_constant(string, &job_id) == false) {
			ShowError("achievement_readdb_validate_criteria_jobid: Invalid JobId %d provided (Achievement: %d, Objective: %d). Skipping...\n", job_id, entry_id, obj_idx);
			return false;
		}

		VECTOR_ENSURE(obj->jobid, 1, 1);
		VECTOR_PUSH(obj->jobid, job_id);
	} else if ((tt = libconfig->setting_get_member(t, "JobId")) && config_setting_is_array(tt)) {
		int j = 0;

		while (j < libconfig->setting_length(tt)) {
			if ((job_id = libconfig->setting_get_int_elem(tt, j)) == 0) {
				if ((string = libconfig->setting_get_string_elem(tt, j)) != NULL && script->get_constant(string, &job_id) == false) {
					ShowError("achievement_readdb_validate_criteria_jobid: Invalid JobId provided at index %d (Achievement: %d, Objective: %d). Skipping...\n", j, entry_id, obj_idx);
					continue;
				}
			}

			/* Ensure size and allocation */
			VECTOR_ENSURE(obj->jobid, 1, 1);
			/* push buffer */
			VECTOR_PUSH(obj->jobid, job_id);
			j++;
		}
	} else if (achievement_criteria_jobid(type)) {
		ShowError("achievement_readdb_validate_criteria_jobid: Achievement type of ID %d requires a JobId field in the objective criteria, setting not provided. Skipping...\n", entry_id);
		return false;
	}

	return true;

}

/**
 * Validates Item Id criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_criteria_itemid(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int val = 0;
	const char *string = NULL;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);


	if (libconfig->setting_lookup_int(t, "ItemId", &val)) {
		if (itemdb->exists(val) == NULL) {
			ShowError("achievement_readdb_validate_criteria_itemid: Invalid ItemID %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		} else if (obj->unique_type != CRITERIA_UNIQUE_NONE) {
			ShowError("achievement_readdb_validate_criteria_itemid: Unique criteria has already been set to type %d. (Achievement: %d, Objective: %d). Skipping...\n", (int) obj->unique_type, entry_id, obj_idx);
			return false;
		}

		obj->unique.itemid = val;
		obj->unique_type = CRITERIA_UNIQUE_ITEM_ID;
	} else if (libconfig->setting_lookup_string(t, "ItemId", &string)) {
		if (script->get_constant(string, &val) == false) {
			ShowError("achievement_readdb_validate_criteria_itemid: Invalid ItemID %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		} else if (obj->unique_type != CRITERIA_UNIQUE_NONE) {
			ShowError("achievement_readdb_validate_criteria_itemid: Unique criteria has already been set to type %d. (Achievement: %d, Objective: %d). Skipping...\n", (int) obj->unique_type, entry_id, obj_idx);
			return false;
		}

		obj->unique.itemid = val;
		obj->unique_type = CRITERIA_UNIQUE_ITEM_ID;
	} else if (achievement_criteria_itemid(type)) {
		ShowError("achievement_readdb_validate_criteria_itemid: Criteria requires a ItemId field (Achievement: %d, Objective: %d). Skipping...\n", entry_id, obj_idx);
		return false;
	}

	return true;
}

/**
 * Validates Status Type criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_criteria_statustype(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int val = 0;
	const char *string = NULL;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);

	if (libconfig->setting_lookup_int(t, "StatusType", &val)) {
		if (!achievement_valid_status_types(val)) {
			ShowError("achievement_readdb_validate_criteria_statustype: Invalid StatusType %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		} else if (obj->unique_type != CRITERIA_UNIQUE_NONE) {
			ShowError("achievement_readdb_validate_criteria_statustype: Unique criteria has already been set to type %d. (Achievement: %d, Objective: %d). Skipping...\n", (int) obj->unique_type, entry_id, obj_idx);
			return false;
		}

		obj->unique.status_type = (enum status_point_types) val;
		obj->unique_type = CRITERIA_UNIQUE_STATUS_TYPE;
	} else if (libconfig->setting_lookup_string(t, "StatusType", &string)) {
		if      (strcmp(string, "SP_STR") == 0)       val = SP_STR;
		else if (strcmp(string, "SP_AGI") == 0)       val = SP_AGI;
		else if (strcmp(string, "SP_VIT") == 0)       val = SP_VIT;
		else if (strcmp(string, "SP_INT") == 0)       val = SP_INT;
		else if (strcmp(string, "SP_DEX") == 0)       val = SP_DEX;
		else if (strcmp(string, "SP_LUK") == 0)       val = SP_LUK;
		else if (strcmp(string, "SP_BASELEVEL") == 0) val = SP_BASELEVEL;
		else if (strcmp(string, "SP_JOBLEVEL") == 0)  val = SP_JOBLEVEL;
		else val = SP_NONE;

		if (!achievement_valid_status_types(val)) {
			ShowError("achievement_readdb_validate_criteria_statustype: Invalid StatusType %s provided (Achievement: %d, Objective: %d). Skipping...\n", string, entry_id, obj_idx);
			return false;
		} else if (obj->unique_type != CRITERIA_UNIQUE_NONE) {
			ShowError("achievement_readdb_validate_criteria_statustype: Unique criteria has already been set to type %d. (Achievement: %d, Objective: %d). Skipping...\n", (int) obj->unique_type, entry_id, obj_idx);
			return false;
		}

		obj->unique.status_type = (enum status_point_types) val;
		obj->unique_type = CRITERIA_UNIQUE_STATUS_TYPE;
	} else if (achievement_criteria_stattype(type)) {
		ShowError("achievement_readdb_validate_criteria_statustype: Criteria requires a StatusType field (Achievement: %d, Objective: %d). Skipping...\n", entry_id, obj_idx);
		return false;
	}

	return true;
}

/**
 * Validates Item Type criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_criteria_itemtype(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int val = 0;
	const char *string = NULL;
	struct config_setting_t *tt = NULL;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);

	if (libconfig->setting_lookup_int(t, "ItemType", &val)) {
		if (val < IT_HEALING || val > IT_MAX) {
			ShowError("achievement_readdb_validate_criteria_itemtype: Invalid ItemType %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		}

		if (val == IT_MAX) {
			obj->item_type |= (2 << val) - 1;
		} else {
			obj->item_type |= (2 << val);
		}

	} else if (libconfig->setting_lookup_string(t, "ItemType", &string)) {
		if (!script->get_constant(string, &val) || val < IT_HEALING || val > IT_MAX) {
			ShowError("achievement_readdb_validate_criteria_itemtype: Invalid ItemType %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		}

		if (val == IT_MAX) {
			obj->item_type |= (2 << val) - 1;
		} else {
			obj->item_type |= (2 << val);
		}
	} else if ((tt = libconfig->setting_get_member(t, "ItemType")) && config_setting_is_array(tt)) {
		int j = 0;

		while (j < libconfig->setting_length(tt)) {
			if ((val = libconfig->setting_get_int_elem(tt, j))) {
				if (val < IT_HEALING || val > IT_MAX) {
					ShowError("achievement_readdb_validate_criteria_itemtype: Invalid ItemType %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
					continue;
				}
				if (val == IT_MAX) {
					obj->item_type |= (2 << val) - 1;
				} else {
					obj->item_type |= (2 << val);
				}
			} else if ((string = libconfig->setting_get_string_elem(tt, j))) {
				if (!script->get_constant(string, &val)) {
					ShowError("achievement_readdb_validate_criteria_itemtype: Invalid ItemType %s provided (Achievement: %d, Objective: %d). Skipping...\n", string, entry_id, obj_idx);
					continue;			
				}
				
				if (val == IT_MAX) {
					obj->item_type |= (2 << val) - 1;
				} else {
					obj->item_type |= (2 << val);
				}
			}
			j++;
		}
	} else if (achievement_criteria_itemtype(type)) {
		ShowError("achievement_readdb_validate_criteria_itemtype: Criteria requires a ItemType field (Achievement: %d, Objective: %d). Skipping...\n", entry_id, obj_idx);
		return false;
	}

	return true;
}

/**
 * Validates Weapon Level criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_criteria_weaponlv(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int val = 0;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);

	if (libconfig->setting_lookup_int(t, "WeaponLevel", &val)) {
		if (val < 1 || val > 4) {
			ShowError("achievement_readdb_validate_criteria_weaponlv: Invalid WeaponLevel %d provided (Achievement: %d, Objective: %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		} else if (obj->unique_type != CRITERIA_UNIQUE_NONE) {
			ShowError("achievement_readdb_validate_criteria_weaponlv: Unique criteria has already been set to type %d. (Achievement: %d, Objective: %d). Skipping...\n", (int) obj->unique_type, entry_id, obj_idx);
			return false;
		}

		obj->unique.weapon_lv = val;
		obj->unique_type = CRITERIA_UNIQUE_WEAPON_LV;
	} else if (achievement_criteria_weaponlv(type)) {
		ShowError("achievement_readdb_validate_criteria_weaponlv: Criteria requires a WeaponType field. (Achievement: %d, Objective: %d). Skipping...\n", entry_id, obj_idx);
		return false;
	}

	return true;
}

/**
 * Validates achievement Id criteria of an objective while parsing an achievement entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] obj      pointer to the achievement objective entry being parsed.
 * @param[in]  type     Type of the achievement being parsed.
 * @param[in]  entry_id Id of the entry being parsed.
 * @param[in]  obj_idx  Index of the objective entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_criteria_achievement(const struct config_setting_t *t, struct achievement_objective *obj, enum achievement_types type, int entry_id, int obj_idx)
{
	int val = 0;

	nullpo_retr(false, t);
	nullpo_retr(false, obj);

	if (libconfig->setting_lookup_int(t, "Achieve", &val)) {
		if (achievement->get(val) == NULL) {
			ShowError("achievement_readdb_validate_criteria_achievement: Invalid Achievement %d provided as objective (Achievement %d, Objective %d). Skipping...\n", val, entry_id, obj_idx);
			return false;
		} else if (obj->unique_type != CRITERIA_UNIQUE_NONE) {
			ShowError("achievement_readdb_validate_criteria_achievement: Unique criteria has already been set to type %d. (Achievement: %d, Objective: %d). Skipping...\n", (int) obj->unique_type, entry_id, obj_idx);
			return false;
		}

		obj->unique.achieve_id = val;
		obj->unique_type = CRITERIA_UNIQUE_ACHIEVE_ID;
	} else if (type == ACH_ACHIEVE) {
		ShowError("achievement_readdb_validate_criteria_achievement: Achievement type of ID %d requires an Achieve field in the objective criteria, setting not provided. Skipping...\n", entry_id);
		return false;
	}

	return true;
}

/**
 * Read a single objective entry of an achievement.
 * @param[in]  conf    config pointer.
 * @param[in]  index   Index of the objective being in the objective list being parsed.
 * @param[out] entry   pointer to the achievement db entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_objective_sub(const struct config_setting_t *conf, int index, struct achievement_data *entry)
{
	struct config_setting_t *tt = NULL;
	char objnum[12];

	nullpo_retr(false, conf);
	nullpo_retr(false, entry);

	sprintf(objnum, "*%d", index); // Search Objective 1..MAX
	if ((tt = libconfig->setting_get_member(conf, objnum)) && config_setting_is_group(tt)) {
		struct achievement_objective obj = { 0 };
		struct config_setting_t *c = NULL;

		/* Description */
		if (libconfig->setting_lookup_mutable_string(tt, "Description", obj.description, OBJECTIVE_DESCRIPTION_LENGTH) == 0) {
			ShowError("achievement_readdb_objective_sub: Objective %d has no description for Achievement %d, skipping...\n", index, entry->id);
			return false;
		}

		/* Criteria */
		if ((c = libconfig->setting_get_member(tt, "Criteria")) && config_setting_is_group(tt)) {
			/* MobId */
			if (achievement->readdb_validate_criteria_mobid(c, &obj, entry->type, entry->id, index) == false)
				return false;

			/* ItemId */
			if (achievement->readdb_validate_criteria_itemid(c, &obj, entry->type, entry->id, index) == false)
				return false;

			/* StatusType */
			if (achievement->readdb_validate_criteria_statustype(c, &obj, entry->type, entry->id, index) == false)
				return false;

			/* ItemType */
			if (achievement->readdb_validate_criteria_itemtype(c, &obj, entry->type, entry->id, index) == false)
				return false;

			/* WeaponLevel */
			if (achievement->readdb_validate_criteria_weaponlv(c, &obj, entry->type, entry->id, index) == false)
				return false;

			/* Achievement */
			if (achievement->readdb_validate_criteria_achievement(c, &obj, entry->type, entry->id, index) == false)
				return false;

			/**
			 * Vectors are read last to avoid memory leaks if either of the above break, in cases where they are stacked with other criteria.
			 * Note to future editors - be sure to cleanup previous vectors before breaks.
			 */
			/* JobId */
			if (achievement->readdb_validate_criteria_jobid(c, &obj, entry->type, entry->id, index) == false)
				return false;
		} else if (achievement->type_requires_criteria(entry->type)) {
			ShowError("achievement_readdb_objective_sub: No criteria field added (Achievement: %d, Objective: %d)! Skipping...\n", entry->id, index);
			return false;
		}

		/* Goal */
		if (libconfig->setting_lookup_int(tt, "Goal", &obj.goal) <= 0)
			obj.goal = 1; // 1 count by default.

		/* Ensure size and allocation */
		VECTOR_ENSURE(entry->objective, 1, 1);

		/* Push buffer */
		VECTOR_PUSH(entry->objective, obj);
	} else { // Break if not in order, to comply with the client's objective order.
		ShowWarning("achievement_readdb_objective_sub: Objectives for Achievement %d are not in order (starting from *%d). Remaining objectives will be skipped.\n", entry->id, index);
		return false;
	}

	return true;
}

/**
 * Parses achievement objective entries of the current achievement db entry being parsed.
 * @param[in]  conf       config pointer.
 * @param[out] entry      pointer to the achievement db entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_objectives(const struct config_setting_t *conf, struct achievement_data *entry)
{
	struct config_setting_t *t = NULL;

	nullpo_retr(false, conf);
	nullpo_retr(false, entry);

	if ((t = libconfig->setting_get_member(conf, "Objectives")) && config_setting_is_group(t)) {
		int i = 0;
		// Initialize the buffer objective vector.
		VECTOR_INIT(entry->objective);

		for (i = 0; i < libconfig->setting_length(t) && i < MAX_ACHIEVEMENT_OBJECTIVES; i++)
			if (achievement_readdb_objective_sub(t, i + 1, entry) == false)
				break;

		// Assess total objectives.
		if (libconfig->setting_length(t) > MAX_ACHIEVEMENT_OBJECTIVES)
			ShowWarning("achievement_readdb_objectives: Exceeded maximum number of objectives (%d) for Achievement %d. Remaining objectives will be skipped.\n", MAX_ACHIEVEMENT_OBJECTIVES, entry->id);
		if (i == 0) {
			ShowError("achievement_readdb_objectives: No Objectives provided for Achievement %d, skipping...\n", entry->id);
			return false;
		}
	} else {
		ShowError("achievement_readdb_objectives: No Objectives provided for Achievement %d, skipping...\n", entry->id);
		return false;
	} // end of "Objectives"

	return true;
}

/**
 * Validates a single reward item in the reward item list of an achievement.
 * @param[in] t       pointer to the config setting.
 * @param[in] index   Index of the item in the reward list.
 * @param[out] entry   pointer to the achievement entry being parsed.
 * @return false on failure, true on success.
 */
static bool achievement_readdb_validate_reward_item_sub(const struct config_setting_t *t, int index, struct achievement_data *entry)
{
	struct config_setting_t *it = NULL;
	struct achievement_reward_item item = { 0 };
	const char *name = NULL;
	int amount = 0;
	int val = 0;

	nullpo_retr(false, t);
	nullpo_retr(false, entry);

	if ((it = libconfig->setting_get_elem(t, index)) == NULL)
		return false;

	name = config_setting_name(it);
	amount = libconfig->setting_get_int(it);

	if (name[0] == 'I' && name[1] == 'D' && itemdb->exists(atoi(name+2))) {
		val = atoi(name);
	} else if (!script->get_constant(name, &val)) {
		ShowWarning("achievement_readdb_validate_reward_item_sub: Non existant Item %s provided as a reward in Achievement %d, skipping...\n", name, entry->id);
		return false;
	}

	if (amount <= 0) {
		ShowWarning("achievement_readdb_validate_reward_item_sub: No amount provided for Item %s as a reward in Achievement %d, skipping...\n", name, entry->id);
		return false;
	}

	/* Ensure size and allocation */
	VECTOR_ENSURE(entry->rewards.item, 1, 1);

	item.id = val;
	item.amount = amount;

	/* push buffer */
	VECTOR_PUSH(entry->rewards.item, item);

	return true;
}

/**
 * Validates the reward items when parsing an achievement db entry.
 * @param[in]  t        pointer to the config setting.
 * @param[out] entry    pointer to the achievement entry being parsed.
 */
static void achievement_readdb_validate_reward_items(const struct config_setting_t *t, struct achievement_data *entry)
{
	struct config_setting_t *tt = NULL;
	int i = 0;

	nullpo_retv(t);
	nullpo_retv(entry);

	if ((tt = libconfig->setting_get_member(t, "Items")) && config_setting_is_group(t)) {
		for (i = 0; i < libconfig->setting_length(tt) && i < MAX_ACHIEVEMENT_ITEM_REWARDS; i++)
			if (achievement->readdb_validate_reward_item_sub(tt, i, entry) == false)
				continue;

		if (libconfig->setting_length(tt) > MAX_ACHIEVEMENT_ITEM_REWARDS)
			ShowError("achievement_readdb_validate_reward_items: Maximum amount of item rewards (%d) exceeded for Achievement %d. Remaining items will be skipped.\n", MAX_ACHIEVEMENT_ITEM_REWARDS, entry->id);
	}
}

/**
 * Validates the reward Bonus script when parsing an achievement db entry.
 * @param[in] t        pointer to the config setting.
 * @param[out] entry    pointer to the achievement entry being parsed.
 * @param[in] source   pointer to the source file name.
 */
static void achievement_readdb_validate_reward_bonus(const struct config_setting_t *t, struct achievement_data *entry, const char *source)
{
	const char *string = NULL;

	nullpo_retv(source);
	nullpo_retv(t);
	nullpo_retv(entry);

	if (libconfig->setting_lookup_string(t, "Bonus", &string))
		entry->rewards.bonus = string ? script->parse(string, source, 0, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;
}

/**
 * Validates the reward Title Id when parsing an achievement entry.
 * @param[in] t     pointer to the config setting.
 * @param[out] entry pointer to the entry.
 */
static void achievement_readdb_validate_reward_titleid(const struct config_setting_t *t, struct achievement_data *entry)
{
	nullpo_retv(t);
	nullpo_retv(entry);

	libconfig->setting_lookup_int(t, "TitleId", &entry->rewards.title_id);
}

/**
 * Validates the reward Bonus script when parsing an achievement db entry.
 * @param[in] conf     pointer to the config setting.
 * @param[out] entry    pointer to the achievement entry being parsed.
 * @param[in] source   pointer to the source file name.
 */
static bool achievement_readdb_rewards(const struct config_setting_t *conf, struct achievement_data *entry, const char *source)
{
	struct config_setting_t *t = NULL;

	nullpo_retr(false, conf);
	nullpo_retr(false, entry);
	nullpo_retr(false, source);

	VECTOR_INIT(entry->rewards.item);

	if ((t = libconfig->setting_get_member(conf, "Rewards")) && config_setting_is_group(t)) {
		/* Items */
		achievement->readdb_validate_reward_items(t, entry);

		/* Buff/Bonus Script Code */
		achievement->readdb_validate_reward_bonus(t, entry, source);

		/* Title Id */
		// @TODO Check Title ID against title DB!
		achievement->readdb_validate_reward_titleid(t, entry);

	}

	return true;
}

/**
 * Validates the reward Bonus script when parsing an achievement db entry.
 * @param[in] conf     pointer to the config setting.
 * @param[out] entry    pointer to the achievement entry being parsed.
 * @param[in] source   pointer to the source file name.
 */
static void achievement_readdb_additional_fields(const struct config_setting_t *conf, struct achievement_data *entry, const char *source)
{
	// plugins do their own thing.
}

/**
 * Parses the Achievement DB.
 * @read achievement_db.conf
 */
static void achievement_readb(void)
{
	const char *filename = "db/"DBPATH"achievement_db.conf";
	struct config_t ach_conf = { 0 };
	struct config_setting_t *achdb = NULL, *conf = NULL;
	int entry = 0, count = 0;
	VECTOR_DECL(int) duplicate;

	if (!libconfig->load_file(&ach_conf, filename))
		return; // report error.

	if (!(achdb = libconfig->setting_get_member(ach_conf.root, "achievement_db"))) {
		ShowError("achievement_readdb: Could not process contents of file '%s', skipping...\n", filename);
		libconfig->destroy(&ach_conf);
		return;
	}

	VECTOR_INIT(duplicate);

	while ((conf = libconfig->setting_get_elem(achdb, entry++))) {
		const char *string = NULL;
		int val = 0, i = 0;
		struct achievement_data t_ad = { 0 }, *p_ad = NULL;

		/* Achievement ID */
		if (libconfig->setting_lookup_int(conf, "Id", &t_ad.id) == 0) {
			ShowError("achievement_readdb: Id field for entry %d is not provided! Skipping...\n", entry);
			continue;
		} else if (t_ad.id <= 0) {
			ShowError("achievement_readdb: Invalid Id %d for entry %d. Skipping...\n", t_ad.id, entry);
			continue;
		}

		ARR_FIND(0, VECTOR_LENGTH(duplicate), i, VECTOR_INDEX(duplicate, i) == t_ad.id);
		if (i != VECTOR_LENGTH(duplicate)) {
			ShowError("achievement_readdb: Duplicate Id %d for entry %d. Skipping...\n", t_ad.id, entry);
			continue;
		}

		/* Achievement Name */
		if (libconfig->setting_lookup_mutable_string(conf, "Name", t_ad.name, ACHIEVEMENT_NAME_LENGTH) == 0) {
			ShowError("achievement_readdb: Name field not provided for Achievement %d!\n", t_ad.id);
			continue;
		}

		/* Achievement Type*/
		if (libconfig->setting_lookup_string(conf, "Type", &string) == 0) {
			ShowError("achievement_readdb: Type field not provided for Achievement %d! Skipping...\n", t_ad.id);
			continue;
		} else if (!script->get_constant(string, &val)) {
			ShowError("achievement_readdb: Invalid constant %s provided as type for Achievement %d! Skipping...\n", string, t_ad.id);
			continue;
		}

		t_ad.type = (enum achievement_types) val;

		/* Objectives */
		achievement->readdb_objectives(conf, &t_ad);

		/* Rewards */
		achievement->readdb_rewards(conf, &t_ad, filename);

		/* Achievement Points */
		libconfig->setting_lookup_int(conf, "Points", &t_ad.points);

		/* Additional Fields */
		achievement->readdb_additional_fields(conf, &t_ad, filename);

		/* Allocate memory for data. */
		CREATE(p_ad, struct achievement_data, 1);
		*p_ad = t_ad;

		/* Place in the database. */
		idb_put(achievement->db, p_ad->id, p_ad);

		/* Put achievement key in categories. */
		VECTOR_ENSURE(achievement->category[p_ad->type], 1, 1);
		VECTOR_PUSH(achievement->category[p_ad->type], p_ad->id);

		/* Qualify for duplicate Id checks */
		VECTOR_ENSURE(duplicate, 1, 1);
		VECTOR_PUSH(duplicate, t_ad.id);

		count++;
	} // end of achievement_db parsing.

	/* Destroy the buffer */
	libconfig->destroy(&ach_conf);

	VECTOR_CLEAR(duplicate);

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filename);
}

/**
 * On server initiation.
 * @param[in] minimal ignores alocating/reading databases.
 */
static void do_init_achievement(bool minimal)
{
	int i = 0;

	if (minimal)
		return;

	/* DB Initialization */
	achievement->db = idb_alloc(DB_OPT_RELEASE_DATA);

	for (i = 0; i < ACH_TYPE_MAX; i++)
		VECTOR_INIT(achievement->category[i]);

	VECTOR_INIT(achievement->rank_exp);

	/* Read LibConfig Files */
	achievement->readdb();
	achievement->readdb_ranks();
}

/**
 * Cleaning function called through achievement->db->destroy()
 */
static int achievement_db_finalize(union DBKey key, struct DBData *data, va_list args)
{
	int i = 0;
	struct achievement_data *ad = DB->data2ptr(data);

	for(i = 0; i < VECTOR_LENGTH(ad->objective); i++)
		VECTOR_CLEAR(VECTOR_INDEX(ad->objective, i).jobid);

	VECTOR_CLEAR(ad->objective);
	VECTOR_CLEAR(ad->rewards.item);

	// Free the script
	if (ad->rewards.bonus != NULL) {
		script->free_code(ad->rewards.bonus);
		ad->rewards.bonus = NULL;
	}

	return 0;
}

/**
 * On server finalizing.
 */
static void do_final_achievement(void)
{
	int i = 0;

	achievement->db->destroy(achievement->db, achievement->db_finalize);

	for (i = 0; i < ACH_TYPE_MAX; i++)
		VECTOR_CLEAR(achievement->category[i]);

	VECTOR_CLEAR(achievement->rank_exp);
}

/**
 * Achievement Interface
 */
void achievement_defaults(void)
{
	achievement = &achievement_s;
	/* */
	achievement->init = do_init_achievement;
	achievement->final = do_final_achievement;
	/* */
	achievement->db_finalize = achievement_db_finalize;
	/* */
	achievement->readdb = achievement_readb;
	/* */
	achievement->readdb_objectives_sub = achievement_readdb_objective_sub;
	achievement->readdb_objectives = achievement_readdb_objectives;
	/* */
	achievement->readdb_validate_criteria_mobid = achievement_readdb_validate_criteria_mobid;
	achievement->readdb_validate_criteria_jobid = achievement_readdb_validate_criteria_jobid;
	achievement->readdb_validate_criteria_itemid = achievement_readdb_validate_criteria_itemid;
	achievement->readdb_validate_criteria_statustype = achievement_readdb_validate_criteria_statustype;
	achievement->readdb_validate_criteria_itemtype = achievement_readdb_validate_criteria_itemtype;
	achievement->readdb_validate_criteria_weaponlv = achievement_readdb_validate_criteria_weaponlv;
	achievement->readdb_validate_criteria_achievement = achievement_readdb_validate_criteria_achievement;
	/* */
	achievement->readdb_rewards = achievement_readdb_rewards;
	achievement->readdb_validate_reward_items = achievement_readdb_validate_reward_items;
	achievement->readdb_validate_reward_item_sub = achievement_readdb_validate_reward_item_sub;
	achievement->readdb_validate_reward_bonus = achievement_readdb_validate_reward_bonus;
	achievement->readdb_validate_reward_titleid = achievement_readdb_validate_reward_titleid;
	/* */
	achievement->readdb_additional_fields = achievement_readdb_additional_fields;
	/* */
	achievement->readdb_ranks = achievement_readdb_ranks;
	/* */
	achievement->get = achievement_get;
	achievement->ensure = achievement_ensure;
	/* */
	achievement->calculate_totals = achievement_calculate_totals;
	achievement->check_complete = achievement_check_complete;
	achievement->progress_add = achievement_progress_add;
	achievement->progress_set = achievement_progress_set;
	achievement->check_criteria = achievement_check_criteria;
	/* */
	achievement->validate = achievement_validate;
	achievement->validate_type = achievement_validate_type;
	/* */
	achievement->validate_mob_kill = achievement_validate_mob_kill;
	achievement->validate_mob_damage = achievement_validate_mob_damage;
	achievement->validate_pc_kill = achievement_validate_pc_kill;
	achievement->validate_pc_damage = achievement_validate_pc_damage;
	achievement->validate_jobchange = achievement_validate_jobchange;
	achievement->validate_stats = achievement_validate_stats;
	achievement->validate_chatroom_create = achievement_validate_chatroom_create;
	achievement->validate_chatroom_members = achievement_validate_chatroom_members;
	achievement->validate_friend_add = achievement_validate_friend_add;
	achievement->validate_party_create = achievement_validate_party_create;
	achievement->validate_marry = achievement_validate_marry;
	achievement->validate_adopt = achievement_validate_adopt;
	achievement->validate_zeny = achievement_validate_zeny;
	achievement->validate_refine = achievement_validate_refine;
	achievement->validate_item_get = achievement_validate_item_get;
	achievement->validate_item_sell = achievement_validate_item_sell;
	achievement->validate_achieve = achievement_validate_achieve;
	achievement->validate_taming = achievement_validate_taming;
	achievement->validate_achievement_rank = achievement_validate_achievement_rank;
	/* */
	achievement->type_requires_criteria = achievement_type_requires_criteria;
	/* */
	achievement->init_titles = achievement_init_titles;
	achievement->check_title = achievement_check_title;
	achievement->get_rewards = achievement_get_rewards;
}
