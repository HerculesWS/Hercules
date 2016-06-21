/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h"
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

struct quest_interface quest_s;
struct quest_db *db_data[MAX_QUEST_DB]; ///< Quest database

struct quest_interface *quest;

/**
 * Searches a quest by ID.
 *
 * @param quest_id ID to lookup
 * @return Quest entry (equals to &quest->dummy if the ID is invalid)
 */
struct quest_db *quest_db(int quest_id) {
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
int quest_pc_login(struct map_session_data *sd)
{
#if PACKETVER < 20141022
	int i;
#endif

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
 * @param sd       Player's data
 * @param quest_id ID of the quest to add.
 * @return 0 in case of success, nonzero otherwise
 */
int quest_add(struct map_session_data *sd, int quest_id)
{
	int n;
	struct quest_db *qi = quest->db(quest_id);

	if( qi == &quest->dummy ) {
		ShowError("quest_add: quest %d not found in DB.\n", quest_id);
		return -1;
	}

	if( quest->check(sd, quest_id, HAVEQUEST) >= 0 ) {
		ShowError("quest_add: Character %d already has quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	n = sd->avail_quests; // Insertion point

	sd->num_quests++;
	sd->avail_quests++;
	RECREATE(sd->quest_log, struct quest, sd->num_quests);

	if( sd->avail_quests != sd->num_quests ) {
		// The character has some completed quests, make room before them so that they will stay at the end of the array
		memmove(&sd->quest_log[n+1], &sd->quest_log[n], sizeof(struct quest)*(sd->num_quests-sd->avail_quests));
	}

	memset(&sd->quest_log[n], 0, sizeof(struct quest));

	sd->quest_log[n].quest_id = qi->id;
	if( qi->time )
		sd->quest_log[n].time = (unsigned int)(time(NULL) + qi->time);
	sd->quest_log[n].state = Q_ACTIVE;

	sd->save_quest = true;

	clif->quest_add(sd, &sd->quest_log[n]);
	clif->quest_update_objective(sd, &sd->quest_log[n]);

	if( map->save_settings&64 )
		chrif->save(sd,0);

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
int quest_change(struct map_session_data *sd, int qid1, int qid2)
{
	int i;
	struct quest_db *qi = quest->db(qid2);

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
	clif->quest_update_objective(sd, &sd->quest_log[i]);

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
int quest_delete(struct map_session_data *sd, int quest_id)
{
	int i;

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
int quest_update_objective_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = NULL;
	int party_id = va_arg(ap, int);
	int mob_id = va_arg(ap, int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if( !sd->avail_quests )
		return 0;
	if( sd->status.party_id != party_id )
		return 0;

	quest->update_objective(sd, mob_id);

	return 1;
}


/**
 * Updates the quest objectives for a character after killing a monster, including the handling of quest-granted drops.
 *
 * @param sd     Character's data
 * @param mob_id Monster ID
 */
void quest_update_objective(struct map_session_data *sd, int mob_id)
{
	int i,j;

	for (i = 0; i < sd->avail_quests; i++) {
		struct quest_db *qi = NULL;

		if (sd->quest_log[i].state != Q_ACTIVE) // Skip inactive quests
			continue;

		qi = quest->db(sd->quest_log[i].quest_id);

		for (j = 0; j < qi->objectives_count; j++) {
			if (qi->objectives[j].mob == mob_id && sd->quest_log[i].count[j] < qi->objectives[j].count) {
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
			if (dropitem->mob_id != 0 && dropitem->mob_id != mob_id)
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
int quest_update_status(struct map_session_data *sd, int quest_id, enum quest_state qs)
{
	int i;

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
int quest_check(struct map_session_data *sd, int quest_id, enum quest_check_type type)
{
	int i;

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
 * Reads and parses an entry from the quest_db.
 *
 * @param cs     The config setting containing the entry.
 * @param n      The sequential index of the current config setting.
 * @param source The source configuration file.
 * @return The parsed quest entry.
 * @retval NULL in case of errors.
 */
struct quest_db *quest_read_db_sub(struct config_setting_t *cs, int n, const char *source)
{
	struct quest_db *entry = NULL;
	struct config_setting_t *t = NULL;
	int i32 = 0, quest_id;
	const char *str = NULL;
	/*
	 * Id: Quest ID                    [int]
	 * Name: Quest Name                [string]
	 * TimeLimit: Time Limit (seconds) [int, optional]
	 * Targets: (                      [array, optional]
	 *     {
	 *         MobId: Mob ID           [int]
	 *         Count:                  [int]
	 *     },
	 *     ... (can repeated up to MAX_QUEST_OBJECTIVES times)
	 * )
	 * Drops: (
	 *     {
	 *         ItemId: Item ID to drop [int]
	 *         Rate: Drop rate         [int]
	 *         MobId: Mob ID to match  [int, optional]
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
			if (!libconfig->setting_lookup_int(tt, "MobId", &mob_id) || mob_id <= 0)
				continue;
			if (!libconfig->setting_lookup_int(tt, "Count", &count) || count <= 0)
				continue;
			RECREATE(entry->objectives, struct quest_objective, ++entry->objectives_count);
			entry->objectives[entry->objectives_count-1].mob = mob_id;
			entry->objectives[entry->objectives_count-1].count = count;
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
			if (!libconfig->setting_lookup_int(tt, "MobId", &mob_id))
				mob_id = 0; // Zero = any monster
			if (mob_id < 0)
				continue;
			if (!libconfig->setting_lookup_int(tt, "ItemId", &nameid) || !itemdb->exists(nameid))
				continue;
			if (!libconfig->setting_lookup_int(tt, "Rate", &rate) || rate <= 0)
				continue;
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
int quest_read_db(void)
{
	char filepath[256];
	struct config_t quest_db_conf;
	struct config_setting_t *qdb = NULL, *q = NULL;
	int i = 0, count = 0;
	const char *filename = "quest_db.conf";

	sprintf(filepath, "%s/%s", map->db_path, filename);
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
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filename);
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
int quest_reload_check_sub(struct map_session_data *sd, va_list ap) {
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
void quest_clear_db(void) {
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

/**
 * Initializes the quest interface.
 *
 * @param minimal Run in minimal mode (skips most of the loading)
 */
void do_init_quest(bool minimal) {
	if (minimal)
		return;

	quest->read_db();
}

/**
 * Finalizes the quest interface before shutdown.
 */
void do_final_quest(void) {
	quest->clear();
}

/**
 * Reloads the quest database.
 */
void do_reload_quest(void) {
	quest->clear();

	quest->read_db();

	// Update quest data for players, to ensure no entries about removed quests are left over.
	map->foreachpc(&quest_reload_check_sub);
}

/**
 * Initializes default values for the quest interface.
 */
void quest_defaults(void) {
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
}
