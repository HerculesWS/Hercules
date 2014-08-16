// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "quest.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "intif.h"
#include "itemdb.h"
#include "log.h"
#include "map.h"
#include "mob.h"
#include "npc.h"
#include "party.h"
#include "pc.h"
#include "script.h"
#include "unit.h"
#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/utils.h"

struct quest_interface quest_s;

/**
 * Searches a quest by ID.
 *
 * @param quest_id ID to lookup
 * @return Quest entry (equals to &quest->dummy if the ID is invalid)
 */
struct quest_db *quest_db(int quest_id) {
	if (quest_id < 0 || quest_id > MAX_QUEST_DB || quest->db_data[quest_id] == NULL)
		return &quest->dummy;
	return quest->db_data[quest_id];
}

/**
 * Sends quest info to the player on login.
 *
 * @param sd Player's data
 * @return 0 in case of success, nonzero otherwise (i.e. the player has no quests)
 */
int quest_pc_login(TBL_PC *sd) {
	int i;

	if(sd->avail_quests == 0)
		return 1;

	clif->quest_send_list(sd);
	clif->quest_send_mission(sd);
	for( i = 0; i < sd->avail_quests; i++ ) {
		// TODO[Haru]: is this necessary? Does quest_send_mission not take care of this?
		clif->quest_update_objective(sd, &sd->quest_log[i]);
	}

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
int quest_add(TBL_PC *sd, int quest_id) {
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
int quest_change(TBL_PC *sd, int qid1, int qid2) {
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
int quest_delete(TBL_PC *sd, int quest_id) {
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
int quest_update_objective_sub(struct block_list *bl, va_list ap) {
	struct map_session_data *sd;
	int mob_id, party_id;

	nullpo_ret(bl);
	nullpo_ret(sd = (struct map_session_data *)bl);

	party_id = va_arg(ap,int);
	mob_id = va_arg(ap,int);

	if( !sd->avail_quests )
		return 0;
	if( sd->status.party_id != party_id )
		return 0;

	quest->update_objective(sd, mob_id);

	return 1;
}


/**
 * Updates the quest objectives for a character after killing a monster.
 *
 * @param sd     Character's data
 * @param mob_id Monster ID
 */
void quest_update_objective(TBL_PC *sd, int mob_id) {
	int i,j;

	for( i = 0; i < sd->avail_quests; i++ ) {
		struct quest_db *qi = NULL;

		if( sd->quest_log[i].state != Q_ACTIVE ) // Skip inactive quests
			continue;

		qi = quest->db(sd->quest_log[i].quest_id);

		for( j = 0; j < qi->num_objectives; j++ ) {
			if( qi->mob[j] == mob_id && sd->quest_log[i].count[j] < qi->count[j] )  {
				sd->quest_log[i].count[j]++;
				sd->save_quest = true;
				clif->quest_update_objective(sd, &sd->quest_log[i]);
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
int quest_update_status(TBL_PC *sd, int quest_id, enum quest_state qs) {
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
int quest_check(TBL_PC *sd, int quest_id, enum quest_check_type type) {
	int i;

	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->num_quests )
		return -1;

	switch( type ) {
		case HAVEQUEST:
			return sd->quest_log[i].state;
		case PLAYTIME:
			return (sd->quest_log[i].time < (unsigned int)time(NULL) ? 2 : sd->quest_log[i].state == Q_COMPLETE ? 1 : 0);
		case HUNTING:
			if( sd->quest_log[i].state == Q_INACTIVE || sd->quest_log[i].state == Q_ACTIVE ) {
				int j;
				struct quest_db *qi = quest->db(sd->quest_log[i].quest_id);
				ARR_FIND(0, MAX_QUEST_OBJECTIVES, j, sd->quest_log[i].count[j] < qi->count[j]);
				if( j == MAX_QUEST_OBJECTIVES )
					return 2;
				if( sd->quest_log[i].time < (unsigned int)time(NULL) )
					return 1;
			}
			return 0;
		default:
			ShowError("quest_check_quest: Unknown parameter %d",type);
			break;
	}

	return -1;
}

/**
 * Loads quests from the quest db.
 *
 * @return Number of loaded quests, or -1 if the file couldn't be read.
 */
int quest_read_db(void) {
	// TODO[Haru] This duplicates some sv_readdb functionalities, and it would be
	// nice if it could be replaced by it. The reason why it wasn't is probably
	// because we need to accept commas (which is also used as delimiter) in the
	// last field (quest name), and sv_readdb isn't capable of doing so.
	FILE *fp;
	char line[1024];
	int i, count = 0;
	char *str[20], *p, *np;
	struct quest_db entry;

	sprintf(line, "%s/quest_db.txt", map->db_path);
	if ((fp=fopen(line,"r"))==NULL) {
		ShowError("can't read %s\n", line);
		return -1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));

		for (i = 0, p = line; i < 8; i++) {
			if (( np = strchr(p,',') ) != NULL) {
				str[i] = p;
				*np = 0;
				p = np + 1;
			} else if (str[0] == NULL) {
				break;
			} else {
				ShowError("quest_read_db: insufficient columns in line %s\n", line);
				continue;
			}
		}
		if (str[0] == NULL)
			continue;

		memset(&entry, 0, sizeof(entry));

		entry.id = atoi(str[0]);

		if (entry.id < 0 || entry.id >= MAX_QUEST_DB) {
			ShowError("quest_read_db: Invalid quest ID '%d' in line '%s' (min: 0, max: %d.)\n", entry.id, line, MAX_QUEST_DB);
			continue;
		}

		entry.time = atoi(str[1]);

		for (i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
			entry.mob[i] = atoi(str[2*i+2]);
			entry.count[i] = atoi(str[2*i+3]);

			if (!entry.mob[i] || !entry.count[i])
				break;
		}

		entry.num_objectives = i;

		if (quest->db_data[entry.id] == NULL)
			quest->db_data[entry.id] = aMalloc(sizeof(struct quest_db));

		memcpy(quest->db_data[entry.id], &entry, sizeof(struct quest_db));
		count++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, "quest_db.txt");
	return 0;
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
}
