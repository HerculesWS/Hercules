/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
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

#include "quest.h"

#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/homunculus.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/unit.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct quest_interface quest_s;
static struct quest_db *db_data[MAX_QUEST_DB]; ///< Quest database

struct quest_interface *quest;

/**
 * Searches a quest by ID.
 *
 * @param quest_id ID to lookup
 * @return Quest entry (equals to &quest->dummy if the ID is invalid)
 */
static struct quest_db *quest_db(int quest_id)
{
	if (quest_id < 0 || quest_id >= MAX_QUEST_DB || quest->db_data[quest_id] == NULL)
		return &quest->dummy;
	return quest->db_data[quest_id];
}

/**
 * Sends quest info to the player on login.
 *
 * @param sd Player's data
 * @return 0 in case of success, nonzero otherwise (i.e. the player has no quests)
 */
static int quest_pc_login(struct map_session_data *sd)
{
#if PACKETVER < 20141022
	int i;
#endif

	nullpo_retr(1, sd);
	if(sd->avail_quests == 0)
		return 1;

	clif->quest_send_list(sd);

#if PACKETVER < 20141022
	clif->quest_send_mission(sd);
	for( i = 0; i < sd->avail_quests; i++ ) {
		// TODO[Haru]: is this necessary? Does quest_send_mission not take care of this?
		clif->quest_update_objective(sd, &sd->quest_log[i]);
	}
#endif

	return 0;
}

/**
 * Adds a quest to the player's list.
 *
 * New quest will be added as Q_ACTIVE.
 *
 * @param sd         Player's data
 * @param quest_id   ID of the quest to add.
 * @param time_limit Custom time, in UNIX epoch, for this quest
 * @return 0 in case of success, nonzero otherwise
 */
static int quest_add(struct map_session_data *sd, int quest_id, unsigned int time_limit)
{
	int n;
	struct quest_db *qi = quest->db(quest_id);

	nullpo_retr(-1, sd);
	if (qi == &quest->dummy) {
		ShowError("quest_add: quest %d not found in DB.\n", quest_id);
		return -1;
	}

	if (quest->check(sd, quest_id, HAVEQUEST) >= 0) {
		ShowError("quest_add: Character %d already has quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	n = sd->avail_quests; // Insertion point
	Assert_retr(-1, sd->avail_quests <= sd->num_quests);

	sd->num_quests++;
	sd->avail_quests++;
	RECREATE(sd->quest_log, struct quest, sd->num_quests);

	if (sd->avail_quests != sd->num_quests) {
		// The character has some completed quests, make room before them so that they will stay at the end of the array
		memmove(&sd->quest_log[n+1], &sd->quest_log[n], sizeof(struct quest)*(sd->num_quests-sd->avail_quests));
	}

	memset(&sd->quest_log[n], 0, sizeof(struct quest));

	sd->quest_log[n].quest_id = qi->id;
	if (time_limit != 0)
		sd->quest_log[n].time = time_limit;
	else if (qi->time != 0)
		sd->quest_log[n].time = (unsigned int)(time(NULL) + qi->time);
	sd->quest_log[n].state = Q_ACTIVE;

	sd->save_quest = true;

	clif->quest_add(sd, &sd->quest_log[n]);
#if PACKETVER >= 20150513
	clif->quest_notify_objective(sd, &sd->quest_log[n]);
#else
	clif->quest_update_objective(sd, &sd->quest_log[n]);
#endif
	quest->questinfo_refresh(sd);

	if ((map->save_settings & 64) != 0)
		chrif->save(sd, 0);

	return 0;
}

/**
 * Replaces a quest in a player's list with another one.
 *
 * @param sd   Player's data
 * @param qid1 Current quest to replace
 * @param qid2 New quest to add
 * @return 0 in case of success, nonzero otherwise
 */
static int quest_change(struct map_session_data *sd, int qid1, int qid2)
{
	int i;
	struct quest_db *qi = quest->db(qid2);

	nullpo_retr(-1, sd);
	if( qi == &quest->dummy ) {
		ShowError("quest_change: quest %d not found in DB.\n", qid2);
		return -1;
	}

	if( quest->check(sd, qid2, HAVEQUEST) >= 0 ) {
		ShowError("quest_change: Character %d already has quest %d.\n", sd->status.char_id, qid2);
		return -1;
	}

	if( quest->check(sd, qid1, HAVEQUEST) < 0 ) {
		ShowError("quest_change: Character %d doesn't have quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == qid1);
	if( i == sd->avail_quests ) {
		ShowError("quest_change: Character %d has completed quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	memset(&sd->quest_log[i], 0, sizeof(struct quest));
	sd->quest_log[i].quest_id = qi->id;
	if( qi->time )
		sd->quest_log[i].time = (unsigned int)(time(NULL) + qi->time);
	sd->quest_log[i].state = Q_ACTIVE;

	sd->save_quest = true;

	clif->quest_delete(sd, qid1);
	clif->quest_add(sd, &sd->quest_log[i]);
#if PACKETVER >= 20150513
	clif->quest_notify_objective(sd, &sd->quest_log[i]);
#else
	clif->quest_update_objective(sd, &sd->quest_log[i]);
#endif
	quest->questinfo_refresh(sd);

	if( map->save_settings&64 )
		chrif->save(sd,0);
	return 0;
}

/**
 * Removes a quest from a player's list
 *
 * @param sd       Player's data
 * @param quest_id ID of the quest to remove
 * @return 0 in case of success, nonzero otherwise
 */
static int quest_delete(struct map_session_data *sd, int quest_id)
{
	int i;

	nullpo_retr(-1, sd);
	//Search for quest
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);

	if(i == sd->num_quests) {
		ShowError("quest_delete: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	if( sd->quest_log[i].state != Q_COMPLETE )
		sd->avail_quests--;

	if( i < --sd->num_quests ) {
		// Compact the array
		memmove(&sd->quest_log[i], &sd->quest_log[i+1], sizeof(struct quest)*(sd->num_quests-i));
	}
	if( sd->num_quests == 0 ) {
		aFree(sd->quest_log);
		sd->quest_log = NULL;
	} else {
		RECREATE(sd->quest_log, struct quest, sd->num_quests);
	}
	sd->save_quest = true;

	clif->quest_delete(sd, quest_id);
	quest->questinfo_refresh(sd);

	if( map->save_settings&64 )
		chrif->save(sd,0);

	return 0;
}

/**
 * Map iterator subroutine to update quest objectives for a party after killing a monster.
 *
 * @see map_foreachinrange
 * @param ap Argument list, expecting:
 *           int Party ID
 *           int Mob ID
 */
static int quest_update_objective_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;
	int party_id = va_arg(ap, int);
	const struct mob_data *md = va_arg(ap, const struct mob_data *);

	nullpo_ret(bl);
	nullpo_ret(md);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if( !sd->avail_quests )
		return 0;
	if( sd->status.party_id != party_id )
		return 0;

	quest->update_objective(sd, md);

	return 1;
}


/**
 * Updates the quest objectives for a character after killing a monster, including the handling of quest-granted drops.
 *
 * @param sd     Character's data
 * @param mob_id Monster ID
 */
static void quest_update_objective(struct map_session_data *sd, const struct mob_data *md)
{
	int i,j;

	nullpo_retv(sd);
	nullpo_retv(md);

	for (i = 0; i < sd->avail_quests; i++) {
		struct quest_db *qi = NULL;

		if (sd->quest_log[i].state != Q_ACTIVE) // Skip inactive quests
			continue;

		qi = quest->db(sd->quest_log[i].quest_id);

		for (j = 0; j < qi->objectives_count; j++) {
			if ((qi->objectives[j].mob == 0 || qi->objectives[j].mob == md->class_) &&
				sd->quest_log[i].count[j] < qi->objectives[j].count &&
				(qi->objectives[j].level.min == 0 || qi->objectives[j].level.min <= md->level) &&
				(qi->objectives[j].level.max == 0 || qi->objectives[j].level.max >= md->level) &&
				(qi->objectives[j].mapid < 0 || qi->objectives[j].mapid == md->bl.m) &&
				(qi->objectives[j].mobtype.size_enabled == false  || qi->objectives[j].mobtype.size == md->status.size) &&
				(qi->objectives[j].mobtype.race_enabled == false || qi->objectives[j].mobtype.race == md->status.race) &&
				(qi->objectives[j].mobtype.ele_enabled == false || qi->objectives[j].mobtype.ele == md->status.def_ele)) {
					sd->quest_log[i].count[j]++;
					sd->save_quest = true;
					clif->quest_update_objective(sd, &sd->quest_log[i]);
			}
		}

		// process quest-granted extra drop bonuses
		for (j = 0; j < qi->dropitem_count; j++) {
			struct quest_dropitem *dropitem = &qi->dropitem[j];
			struct item item;
			struct item_data *data = NULL;
			int temp;
			if (dropitem->mob_id != 0 && dropitem->mob_id != md->class_)
				continue;
			// TODO: Should this be affected by server rates?
			if (rnd()%10000 >= dropitem->rate)
				continue;
			if (!(data = itemdb->exists(dropitem->nameid)))
				continue;
			memset(&item,0,sizeof(item));
			item.nameid = dropitem->nameid;
			item.identify = itemdb->isidentified2(data);
			item.amount = 1;
			if((temp = pc->additem(sd, &item, 1, LOG_TYPE_QUEST)) != 0) { // TODO: We might want a new log type here?
				// Failed to obtain the item
				clif->additem(sd, 0, 0, temp);
			}
		}
	}
}

/**
 * Updates a quest's state.
 *
 * Only status of active and inactive quests can be updated. Completed quests can't (for now). [Inkfish]
 *
 * @param sd       Character's data
 * @param quest_id Quest ID to update
 * @param qs       New quest state
 * @return 0 in case of success, nonzero otherwise
 */
static int quest_update_status(struct map_session_data *sd, int quest_id, enum quest_state qs)
{
	int i;

	nullpo_retr(-1, sd);
	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->avail_quests ) {
		ShowError("quest_update_status: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	sd->quest_log[i].state = qs;
	sd->save_quest = true;

	if( qs < Q_COMPLETE ) {
		clif->quest_update_status(sd, quest_id, qs == Q_ACTIVE ? true : false);
		return 0;
	}

	// The quest is complete, so it needs to be moved to the completed quests block at the end of the array.

	if( i < (--sd->avail_quests) ) {
		struct quest tmp_quest;
		memcpy(&tmp_quest, &sd->quest_log[i],sizeof(struct quest));
		memcpy(&sd->quest_log[i], &sd->quest_log[sd->avail_quests],sizeof(struct quest));
		memcpy(&sd->quest_log[sd->avail_quests], &tmp_quest,sizeof(struct quest));
	}

	clif->quest_delete(sd, quest_id);
	quest->questinfo_refresh(sd);

	if( map->save_settings&64 )
		chrif->save(sd,0);

	return 0;
}

/**
 * Queries quest information for a character.
 *
 * @param sd       Character's data
 * @param quest_id Quest ID
 * @param type     Check type
 * @return -1 if the quest was not found, otherwise it depends on the type:
 *         HAVEQUEST: The quest's state
 *         PLAYTIME:  2 if the quest's timeout has expired
 *                    1 if the quest was completed
 *                    0 otherwise
 *         HUNTING:   2 if the quest has not been marked as completed yet, and its objectives have been fulfilled
 *                    1 if the quest's timeout has expired
 *                    0 otherwise
 */
static int quest_check(struct map_session_data *sd, int quest_id, enum quest_check_type type)
{
	int i;

	nullpo_retr(-1, sd);
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if (i == sd->num_quests)
		return -1;

	switch (type) {
		case HAVEQUEST:
			return sd->quest_log[i].state;
		case PLAYTIME:
			return (sd->quest_log[i].time < (unsigned int)time(NULL) ? 2 : sd->quest_log[i].state == Q_COMPLETE ? 1 : 0);
		case HUNTING:
			if( sd->quest_log[i].state == Q_INACTIVE || sd->quest_log[i].state == Q_ACTIVE ) {
				int j;
				struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
				ARR_FIND(0, qi->objectives_count, j, sd->quest_log[i].count[j] < qi->objectives[j].count);
				if (j == qi->objectives_count)
					return 2;
				if (sd->quest_log[i].time < (unsigned int)time(NULL))
					return 1;
			}
			return 0;
		default:
			ShowError("quest_check_quest: Unknown parameter %u", type);
			break;
	}

	return -1;
}

/**
 * Reads a field in tt and resolves it if it is a constant.
 *
 * @param tt        The config setting containing the field.
 * @param name      Field to be read
 * @param value     Where the read value will be returned
 * @param quest_id  Entry quest ID (for error messages)
 * @param idx       Entry index in the list (for error messages)
 * @param kind      Entry kind (for error messages)
 * @param source    Source file (for error messages)
 * @return CONFIG_FALSE if something goes wrong, CONFIG_TRUE otherwise. value will be set to the value value.
 */
static int quest_setting_lookup_const(struct config_setting_t *tt, const char *name, int *value, int quest_id, int idx, const char *kind, const char *source)
{
	nullpo_retr(CONFIG_FALSE, tt);

	if (libconfig->setting_lookup_int(tt, name, value) == CONFIG_TRUE)
		return CONFIG_TRUE;

	const char *str = NULL;
	if (libconfig->setting_lookup_string(tt, name, &str) == CONFIG_TRUE) {
		if (str[0] == '\0' || !script->get_constant(str, value)) {
			ShowError("%s: Invalid %s constant \"%s\" at index (%d) in \"%s\" for quest id (%d), skipping.\n", __func__, kind, str, idx, source, quest_id);
			return CONFIG_FALSE;
		}

		return CONFIG_TRUE;
	}

	if (libconfig->setting_lookup(tt, name) != NULL) {
		ShowError("%s: Invalid '%s' for %s at index (%d) of quest (%d) in \"%s\". Skipping.\n", __func__, name, kind, idx, quest_id, source);
		return CONFIG_FALSE;
	}

	return CONFIG_TRUE; // Simply not set
}

/**
 * Reads and parses an entry from the quest_db.
 *
 * @param cs     The config setting containing the entry.
 * @param n      The sequential index of the current config setting.
 * @param source The source configuration file.
 * @return The parsed quest entry.
 * @retval NULL in case of errors.
 */
static struct quest_db *quest_read_db_sub(struct config_setting_t *cs, int n, const char *source)
{
	struct quest_db *entry = NULL;
	struct config_setting_t *t = NULL;
	int i32 = 0, quest_id;
	const char *str = NULL;
	nullpo_retr(NULL, cs);
	/*
	 * Id: Quest ID                    [int]
	 * Name: Quest Name                [string]
	 * TimeLimit: Time Limit (seconds) [int, optional]
	 * Targets: (                      [array, optional]
	 *     {
	 *         MobId: Mob ID           [int/constant]
	 *         Count:                  [int]
	 *     },
	 *     ... (can repeated up to MAX_QUEST_OBJECTIVES times)
	 * )
	 * Drops: (
	 *     {
	 *         ItemId: Item ID to drop [int/constant]
	 *         Rate: Drop rate         [int]
	 *         MobId: Mob ID to match  [int/constant, optional]
	 *     },
	 *     ... (can be repeated)
	 * )
	 */
	if (!libconfig->setting_lookup_int(cs, "Id", &quest_id)) {
		ShowWarning("quest_read_db: Missing id in \"%s\", entry #%d, skipping.\n", source, n);
		return NULL;
	}
	if (quest_id < 0 || quest_id >= MAX_QUEST_DB) {
		ShowWarning("quest_read_db: Invalid quest ID '%d' in \"%s\", entry #%d (min: 0, max: %d), skipping.\n", quest_id, source, n, MAX_QUEST_DB);
		return NULL;
	}

	if (!libconfig->setting_lookup_string(cs, "Name", &str) || !*str) {
		ShowWarning("quest_read_db_sub: Missing Name in quest %d of \"%s\", skipping.\n", quest_id, source);
		return NULL;
	}

	CREATE(entry, struct quest_db, 1);
	entry->id = quest_id;
	//safestrncpy(entry->name, str, sizeof(entry->name));

	if (libconfig->setting_lookup_int(cs, "TimeLimit", &i32)) // This is an unsigned value, do not check for >= 0
		entry->time = (unsigned int)i32;

	if ((t=libconfig->setting_get_member(cs, "Targets")) && config_setting_is_list(t)) {
		int i, len = libconfig->setting_length(t);
		for (i = 0; i < len && entry->objectives_count < MAX_QUEST_OBJECTIVES; i++) {
			// Note: We ensure that objectives_count < MAX_QUEST_OBJECTIVES because
			//       quest_log (as well as the client) expect this maximum size.
			struct config_setting_t *tt = libconfig->setting_get_elem(t, i);
			int mob_id = 0, count = 0;
			if (!tt)
				break;
			if (!config_setting_is_group(tt))
				continue;
			if (quest->setting_lookup_const(tt, "MobId", &mob_id, entry->id, i, "'Target' monster", source) != CONFIG_TRUE)
				continue;
			if (mob_id < 0) {
				ShowWarning("quest_read_db_sub: Invalid monster (%d, index: %d) in \"%s\", for quest (%d).\n", mob_id, i, source, entry->id);
				continue;
			}
			if (!libconfig->setting_lookup_int(tt, "Count", &count) || count <= 0) {
				ShowWarning("quest_read_db_sub: Invalid 'Count' for 'Target' (%d, index: %d) in \"%s\", for quest (%d).\n", mob_id, i, source, entry->id);
				continue;
			}
			RECREATE(entry->objectives, struct quest_objective, ++entry->objectives_count);
			entry->objectives[entry->objectives_count-1].mob = mob_id;
			entry->objectives[entry->objectives_count-1].count = count;

			const struct config_setting_t *lvt = libconfig->setting_get_member(tt, "Level");
			if (lvt != NULL) {
				if (mob_id != 0) {
					ShowWarning("quest_read_db_sub: Level can't be used when a MobId is defined in \"%s\", for quest (%d), ignoring.\n", source, entry->id);
					continue;
				}

				if (config_setting_is_aggregate(lvt)) {
					int min = libconfig->setting_get_int_elem(lvt, 0);
					int max = libconfig->setting_get_int_elem(lvt, 1);
					if (min < 0 || max < 0) {
						ShowWarning("quest_read_db_sub: level can't be a negative value in \"%s\", for quest (%d), ignoring.\n", source, entry->id);
						continue;
					}
					if (min > max && max != 0) {
						ShowWarning("quest_read_db_sub: minimal level (%d) is bigger than the maximal level (%d) in \"%s\", for quest (%d), ignoring.\n", min, max, source, entry->id);
						continue;
					}
					entry->objectives[entry->objectives_count - 1].level.min = min;
					entry->objectives[entry->objectives_count - 1].level.max = max;
				} else {
					ShowWarning("quest_read_db_sub: Invalid format for Level in \"%s\", for quest (%d).\n", source, entry->id);
				}
			}
			const char *map_name = NULL;
			int16 mapid = -1;

			if (libconfig->setting_lookup_string(tt, "MapName", &map_name) != CONFIG_FALSE) {
				if ((mapid = map->mapname2mapid(map_name)) < 0) {
					ShowWarning("quest_read_db_sub: Invalid MapName \"%s\" in \"%s\", for quest (%d), ignoring.\n", map_name, source, entry->id);
					continue;
				}
			}
			entry->objectives[entry->objectives_count - 1].mapid = mapid;

			const struct config_setting_t *mobt = libconfig->setting_get_member(tt, "MobType");
			if (mobt != NULL) {
				if (mob_id != 0) {
					ShowWarning("quest_read_db_sub: MobType can't be used when a MobId is defined in \"%s\", for quest (%d), ignoring.\n", source, entry->id);
					continue;
				}

				if (mob->lookup_const(mobt, "Size", &i32)) {
					entry->objectives[entry->objectives_count - 1].mobtype.size = (uint8)i32;
					entry->objectives[entry->objectives_count - 1].mobtype.size_enabled = true;
				}
				if (mob->lookup_const(mobt, "Race", &i32)) {
					entry->objectives[entry->objectives_count - 1].mobtype.race = (uint8)i32;
					entry->objectives[entry->objectives_count - 1].mobtype.race_enabled = true;
				}
				if (mob->lookup_const(mobt, "Element", &i32)) {
					entry->objectives[entry->objectives_count - 1].mobtype.ele = (uint8)i32;
					entry->objectives[entry->objectives_count - 1].mobtype.ele_enabled = true;
				}
			}

		}
	}

	if ((t=libconfig->setting_get_member(cs, "Drops")) && config_setting_is_list(t)) {
		int i, len = libconfig->setting_length(t);
		for (i = 0; i < len; i++) {
			struct config_setting_t *tt = libconfig->setting_get_elem(t, i);
			int mob_id = 0, nameid = 0, rate = 0;
			if (!tt)
				break;
			if (!config_setting_is_group(tt))
				continue;
			if (quest->setting_lookup_const(tt, "MobId", &mob_id, entry->id, i, "'Drops' monster", source) != CONFIG_TRUE)
				continue;
			if (mob_id < 0) {
				ShowError("%s: Invalid MobId (%d, index: %d) for 'Drop' monster in \"%s\", for quest id (%d), ignoring.\n", __func__, mob_id, i, source, entry->id);
				continue;
			}
			if (quest->setting_lookup_const(tt, "ItemId", &nameid, entry->id, i, "'Drops' item", source) != CONFIG_TRUE)
				continue;
			if (!itemdb->exists(nameid)) {
				ShowError("%s: Invalid ItemId (%d, index: %d) for 'Drops' in \"%s\", for quest (%d), ignoring.\n", __func__, nameid, i, source, entry->id);
				continue;
			}
			if (!libconfig->setting_lookup_int(tt, "Rate", &rate) || rate <= 0) {
				ShowError("%s: Invalid 'Drops' Rate for item (%d, index: %d) in \"%s\", for quest (%d), ignoring.\n", __func__, nameid, i, source, entry->id);
				continue;
			}
			RECREATE(entry->dropitem, struct quest_dropitem, ++entry->dropitem_count);
			entry->dropitem[entry->dropitem_count-1].mob_id = mob_id;
			entry->dropitem[entry->dropitem_count-1].nameid = nameid;
			entry->dropitem[entry->dropitem_count-1].rate = rate;
		}
	}
	return entry;
}

/**
 * Loads quests from the quest db.
 *
 * @return Number of loaded quests, or -1 if the file couldn't be read.
 */
static int quest_read_db(void)
{
	char filepath[256];
	struct config_t quest_db_conf;
	struct config_setting_t *qdb = NULL, *q = NULL;
	int i = 0, count = 0;
	const char *filename = "quest_db.conf";

	safesnprintf(filepath, 256, "%s/%s", map->db_path, filename);
	if (!libconfig->load_file(&quest_db_conf, filepath))
		return -1;

	if ((qdb = libconfig->setting_get_member(quest_db_conf.root, "quest_db")) == NULL) {
		ShowError("can't read %s\n", filepath);
		return -1;
	}

	while ((q = libconfig->setting_get_elem(qdb, i++))) {
		struct quest_db *entry = quest->read_db_sub(q, i-1, filepath);
		if (!entry)
			continue;

		if (quest->db_data[entry->id] != NULL) {
			ShowWarning("quest_read_db: Duplicate quest %d.\n", entry->id);
			if (quest->db_data[entry->id]->dropitem)
				aFree(quest->db_data[entry->id]->dropitem);
			if (quest->db_data[entry->id]->objectives)
				aFree(quest->db_data[entry->id]->objectives);
			aFree(quest->db_data[entry->id]);
		}
		quest->db_data[entry->id] = entry;

		count++;
	}
	libconfig->destroy(&quest_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
	return count;
}

/**
 * Map iterator to ensures a player has no invalid quest log entries.
 *
 * Any entries that are no longer in the db are removed.
 *
 * @see map->foreachpc
 * @param ap Ignored
 */
static int quest_reload_check_sub(struct map_session_data *sd, va_list ap)
{
	int i, j;

	nullpo_ret(sd);

	j = 0;
	for (i = 0; i < sd->num_quests; i++) {
		struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
		if (qi == &quest->dummy) { // Remove no longer existing entries
			if (sd->quest_log[i].state != Q_COMPLETE) // And inform the client if necessary
				clif->quest_delete(sd, sd->quest_log[i].quest_id);
			continue;
		}
		if (i != j) {
			// Move entries if there's a gap to fill
			memcpy(&sd->quest_log[j], &sd->quest_log[i], sizeof(struct quest));
		}
		j++;
	}
	sd->num_quests = j;
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].state == Q_COMPLETE);
	sd->avail_quests = i;

	return 1;
}

/**
 * Clears the quest database for shutdown or reload.
 */
static void quest_clear_db(void)
{
	int i;

	for (i = 0; i < MAX_QUEST_DB; i++) {
		if (quest->db_data[i]) {
			if (quest->db_data[i]->objectives)
				aFree(quest->db_data[i]->objectives);
			if (quest->db_data[i]->dropitem)
				aFree(quest->db_data[i]->dropitem);
			aFree(quest->db_data[i]);
			quest->db_data[i] = NULL;
		}
	}
}

/*
* Limit the questinfo icon id to avoid client problems
*/
static int quest_questinfo_validate_icon(int icon)
{
#if PACKETVER >= 20170315
	if (icon < 0 || (icon > 10 && icon != 9999))
		icon = 9999;
#elif PACKETVER >= 20120410
	if (icon < 0 || (icon > 8 && icon != 9999) || icon == 7)
		icon = 9999; // Default to nothing if icon id is invalid.
#else
	if (icon < 0 || icon > 7)
		icon = 0;
	else
		icon = icon + 1;
#endif
	return icon;
}

/**
 * Refresh the questinfo bubbles on the player map.
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 */
static void quest_questinfo_refresh(struct map_session_data *sd)
{
	nullpo_retv(sd);

	for (int i = 0; i < VECTOR_LENGTH(map->list[sd->bl.m].qi_list); i++) {
		struct npc_data *nd = VECTOR_INDEX(map->list[sd->bl.m].qi_list, i);

		int j;
		ARR_FIND(0, VECTOR_LENGTH(nd->qi_data), j, quest->questinfo_validate(sd, &VECTOR_INDEX(nd->qi_data, j)) == true);
		if (j != VECTOR_LENGTH(nd->qi_data)) {
			struct questinfo *qi = &VECTOR_INDEX(nd->qi_data, j);
			clif->quest_show_event(sd, &nd->bl, qi->icon, qi->color);
		} else {
#if PACKETVER >= 20120410
			clif->quest_show_event(sd, &nd->bl, 9999, 0);
#else
			clif->quest_show_event(sd, &nd->bl, 0, 0);
#endif
		}
	}
}

/**
 * Validate all possible conditions required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if conditions are correct.
 * @retval false if one condition or more are in-correct.
 */
static bool quest_questinfo_validate(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);
	if (qi->hasJob && quest->questinfo_validate_job(sd, qi) == false)
		return false;
	if (qi->sex_enabled && quest->questinfo_validate_sex(sd, qi) == false)
		return false;
	if ((qi->base_level.min != 0 || qi->base_level.max != 0) && quest->questinfo_validate_baselevel(sd, qi) == false)
		return false;
	if ((qi->job_level.min != 0 || qi->job_level.max != 0) && quest->questinfo_validate_joblevel(sd, qi) == false)
		return false;
	if (VECTOR_LENGTH(qi->items) > 0 && quest->questinfo_validate_items(sd, qi) == false)
		return false;
	if (qi->homunculus.level != 0 && quest->questinfo_validate_homunculus_level(sd, qi) == false)
		return false;
	if (qi->homunculus.class_ != 0 && quest->questinfo_validate_homunculus_type(sd, qi) == false)
		return false;
	if (VECTOR_LENGTH(qi->quest_requirement) > 0 && quest->questinfo_validate_quests(sd, qi) == false)
		return false;
	if (qi->mercenary_class != 0 && quest->questinfo_validate_mercenary_class(sd, qi) == false)
		return false;
	return true;
}

/**
 * Validate job required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player job is matching the required.
 * @retval false if player job is NOT matching the required.
 */
static bool quest_questinfo_validate_job(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);
	if (sd->status.class == qi->job)
		return true;
	return false;
}

/**
 * Validate sex required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player sex is matching the required.
 * @retval false if player sex is NOT matching the required.
 */
static bool quest_questinfo_validate_sex(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);
	if (sd->status.sex == qi->sex)
		return true;
	return false;
}

/**
 * Validate base level required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player base level is included in the required level range.
 * @retval false if player base level is NOT included in the required level range.
 */
static bool quest_questinfo_validate_baselevel(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);
	if (sd->status.base_level >= qi->base_level.min && sd->status.base_level <= qi->base_level.max)
		return true;
	return false;
}

/**
 * Validate job level required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player job level is included in the required level range.
 * @retval false if player job level is NOT included in the required level range.
 */
static bool quest_questinfo_validate_joblevel(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);
	if (sd->status.job_level >= qi->job_level.min && sd->status.job_level <= qi->job_level.max)
		return true;
	return false;
}

/**
 * Validate items list required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player have all the items required.
 * @retval false if player is missing one or more of the items required.
 */
static bool quest_questinfo_validate_items(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);

	for (int i = 0; i < VECTOR_LENGTH(qi->items); i++) {
		struct questinfo_itemreq *item = &VECTOR_INDEX(qi->items, i);
		int count = 0;
		for (int j = 0; j < sd->status.inventorySize; j++) {
			if (sd->status.inventory[j].nameid == item->nameid)
				count += sd->status.inventory[j].amount;
		}
		if (count < item->min || count > item->max)
			return false;
	}

	return true;
}

/**
 * Validate minimal homunculus level required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if homunculus level >= the required value.
 * @retval false if homunculus level smaller than the required value.
 */
static bool quest_questinfo_validate_homunculus_level(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);

	if (sd->hd == NULL)
		return false;
	if (!homun_alive(sd->hd))
		return false;
	if (sd->hd->homunculus.level < qi->homunculus.level)
		return false;
	return true;
}

/**
 * Validate homunculus type required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player's homunculus is matching the required.
 * @retval false if player's homunculus is NOT matching the required.
 */
static bool quest_questinfo_validate_homunculus_type(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);

	if (sd->hd == NULL)
		return false;
	if (!homun_alive(sd->hd))
		return false;
	if (homun->class2type(sd->hd->homunculus.class_) != qi->homunculus_type)
		return false;
	return true;
}

/**
 * Validate quest list required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player have all the quests required.
 * @retval false if player is missing one or more of the quests required.
 */
static bool quest_questinfo_validate_quests(struct map_session_data *sd, struct questinfo *qi)
{
	int i;

	nullpo_retr(false, sd);
	nullpo_retr(false, qi);
	
	for (i = 0; i < VECTOR_LENGTH(qi->quest_requirement); i++) {
		struct questinfo_qreq *quest_requirement = &VECTOR_INDEX(qi->quest_requirement, i);
		int quest_progress = quest->check(sd, quest_requirement->id, HAVEQUEST);
		if (quest_progress == -1)
			quest_progress = 0;
		else if (quest_progress == 0 || quest_progress == 1)
			quest_progress = 1;
		else
			quest_progress = 2;
		if (quest_progress != quest_requirement->state)
			return false;
	}

	return true;
}

/**
 * Validate mercenary class required for the questinfo
 *
 * @param sd session data.
 * @param qi questinfo data.
 *
 * @retval true if player have a mercenary with the given class.
 * @retval false if player does NOT have a mercenary with the given class.
 */
static bool quest_questinfo_validate_mercenary_class(struct map_session_data *sd, struct questinfo *qi)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, qi);

	if (sd->md == NULL)
		return false;

	if (sd->md->mercenary.class_ != qi->mercenary_class)
		return false;

	return true;
}

static enum quest_mobtype quest_mobele2client(uint8 type)
{
	switch (type) {
	case ELE_WATER:
		return QMT_ELE_WATER;
	case ELE_WIND:
		return QMT_ELE_WIND;
	case ELE_EARTH:
		return QMT_ELE_EARTH;
	case ELE_FIRE:
		return QMT_ELE_FIRE;
	case ELE_DARK:
		return QMT_ELE_DARK;
	case ELE_HOLY:
		return QMT_ELE_HOLY;
	case ELE_POISON:
		return QMT_ELE_POISON;
	case ELE_GHOST:
		return QMT_ELE_GHOST;
	case ELE_NEUTRAL:
		return QMT_ELE_NEUTRAL;
	case ELE_UNDEAD:
		return QMT_ELE_UNDEAD;
	default:
		return 0;
	}
}

static enum quest_mobtype quest_mobrace2client(uint8 type)
{
	switch (type) {
	case RC_DEMIHUMAN:
		return QMT_RC_DEMIHUMAN;
	case RC_BRUTE:
		return QMT_RC_BRUTE;
	case RC_INSECT:
		return QMT_RC_INSECT;
	case RC_FISH:
		return QMT_RC_FISH;
	case RC_PLANT:
		return QMT_RC_PLANT;
	case RC_DEMON:
		return QMT_RC_DEMON;
	case RC_ANGEL:
		return QMT_RC_ANGEL;
	case RC_UNDEAD:
		return QMT_RC_UNDEAD;
	case RC_FORMLESS:
		return QMT_RC_FORMLESS;
	case RC_DRAGON:
		return QMT_RC_DRAGON;
	default:
		return 0;
	}
}

/**
 * Initializes the quest interface.
 *
 * @param minimal Run in minimal mode (skips most of the loading)
 */
static void do_init_quest(bool minimal)
{
	if (minimal)
		return;

	quest->read_db();
}

/**
 * Finalizes the quest interface before shutdown.
 */
static void do_final_quest(void)
{
	quest->clear();
}

/**
 * Reloads the quest database.
 */
static void do_reload_quest(void)
{
	quest->clear();

	quest->read_db();

	// Update quest data for players, to ensure no entries about removed quests are left over.
	map->foreachpc(&quest_reload_check_sub);
}

/**
 * Initializes default values for the quest interface.
 */
void quest_defaults(void)
{
	quest = &quest_s;
	quest->db_data = db_data;

	memset(&quest->db, 0, sizeof(quest->db));
	memset(&quest->dummy, 0, sizeof(quest->dummy));
	/* */
	quest->init = do_init_quest;
	quest->final = do_final_quest;
	quest->reload = do_reload_quest;
	/* */
	quest->db = quest_db;
	quest->pc_login = quest_pc_login;
	quest->add = quest_add;
	quest->change = quest_change;
	quest->delete = quest_delete;
	quest->update_objective_sub = quest_update_objective_sub;
	quest->update_objective = quest_update_objective;
	quest->update_status = quest_update_status;
	quest->check = quest_check;
	quest->clear = quest_clear_db;
	quest->read_db = quest_read_db;
	quest->read_db_sub = quest_read_db_sub;
	quest->setting_lookup_const = quest_setting_lookup_const;

	quest->questinfo_validate_icon = quest_questinfo_validate_icon;
	quest->questinfo_refresh = quest_questinfo_refresh;
	quest->questinfo_validate = quest_questinfo_validate;
	quest->questinfo_validate_job = quest_questinfo_validate_job;
	quest->questinfo_validate_sex = quest_questinfo_validate_sex;
	quest->questinfo_validate_baselevel = quest_questinfo_validate_baselevel;
	quest->questinfo_validate_joblevel = quest_questinfo_validate_joblevel;
	quest->questinfo_validate_items = quest_questinfo_validate_items;
	quest->questinfo_validate_homunculus_level = quest_questinfo_validate_homunculus_level;
	quest->questinfo_validate_homunculus_type = quest_questinfo_validate_homunculus_type;
	quest->questinfo_validate_quests = quest_questinfo_validate_quests;
	quest->questinfo_validate_mercenary_class = quest_questinfo_validate_mercenary_class;
	quest->mobele2client = quest_mobele2client;
	quest->mobrace2client = quest_mobrace2client;
}
