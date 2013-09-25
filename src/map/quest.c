// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "map.h"
#include "pc.h"
#include "npc.h"
#include "itemdb.h"
#include "script.h"
#include "intif.h"
#include "battle.h"
#include "mob.h"
#include "party.h"
#include "unit.h"
#include "log.h"
#include "clif.h"
#include "quest.h"
#include "chrif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>


struct quest_interface quest_s;

int quest_search_db(int quest_id)
{
	int i;

	ARR_FIND(0, MAX_QUEST_DB,i,quest_id == quest->db[i].id);
	if( i == MAX_QUEST_DB )
		return -1;

	return i;
}

//Send quest info on login
int quest_pc_login(TBL_PC * sd)
{
	int i;

	if(sd->avail_quests == 0)
		return 1;

	clif->quest_send_list(sd);
	clif->quest_send_mission(sd);
	for( i = 0; i < sd->avail_quests; i++ ) {
		clif->quest_update_objective(sd, &sd->quest_log[i], sd->quest_index[i]);
	}

	return 0;
}

int quest_add(TBL_PC * sd, int quest_id)
{

	int i, j;

	if( sd->num_quests >= MAX_QUEST_DB )
	{
		ShowError("quest_add: Character %d has got all the quests.(max quests: %d)\n", sd->status.char_id, MAX_QUEST_DB);
		return 1;
	}

	if( quest->check(sd, quest_id, HAVEQUEST) >= 0 )
	{
		ShowError("quest_add: Character %d already has quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	if( (j = quest->search_db(quest_id)) < 0 )
	{
		ShowError("quest_add: quest %d not found in DB.\n", quest_id);
		return -1;
	}

	i = sd->avail_quests;
	memmove(&sd->quest_log[i+1], &sd->quest_log[i], sizeof(struct quest)*(sd->num_quests-sd->avail_quests));
	memmove(sd->quest_index+i+1, sd->quest_index+i, sizeof(int)*(sd->num_quests-sd->avail_quests));

	memset(&sd->quest_log[i], 0, sizeof(struct quest));
	sd->quest_log[i].quest_id = quest->db[j].id;
	if( quest->db[j].time )
		sd->quest_log[i].time = (unsigned int)(time(NULL) + quest->db[j].time);
	sd->quest_log[i].state = Q_ACTIVE;

	sd->quest_index[i] = j;
	sd->num_quests++;
	sd->avail_quests++;
	sd->save_quest = true;

	clif->quest_add(sd, &sd->quest_log[i], sd->quest_index[i]);
	clif->quest_update_objective(sd, &sd->quest_log[i], sd->quest_index[i]);
	if( iMap->save_settings&64 )
		chrif->save(sd,0);

	return 0;
}

int quest_change(TBL_PC * sd, int qid1, int qid2)
{

	int i, j;

	if( quest->check(sd, qid2, HAVEQUEST) >= 0 )
	{
		ShowError("quest_change: Character %d already has quest %d.\n", sd->status.char_id, qid2);
		return -1;
	}

	if( quest->check(sd, qid1, HAVEQUEST) < 0 )
	{
		ShowError("quest_change: Character %d doesn't have quest %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	if( (j = quest->search_db(qid2)) < 0 )
	{
		ShowError("quest_change: quest %d not found in DB.\n",qid2);
		return -1;
	}

	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == qid1);
	if(i == sd->avail_quests)
	{
		ShowError("quest_change: Character %d has completed quests %d.\n", sd->status.char_id, qid1);
		return -1;
	}

	memset(&sd->quest_log[i], 0, sizeof(struct quest));
	sd->quest_log[i].quest_id = quest->db[j].id;
	if( quest->db[j].time )
		sd->quest_log[i].time = (unsigned int)(time(NULL) + quest->db[j].time);
	sd->quest_log[i].state = Q_ACTIVE;

	sd->quest_index[i] = j;
	sd->save_quest = true;

	clif->quest_delete(sd, qid1);
	clif->quest_add(sd, &sd->quest_log[i], sd->quest_index[i]);
	clif->quest_update_objective(sd, &sd->quest_log[i], sd->quest_index[i]);

	if( iMap->save_settings&64 )
		chrif->save(sd,0);

	return 0;
}

int quest_delete(TBL_PC * sd, int quest_id)
{
	int i;

	//Search for quest
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if(i == sd->num_quests)
	{
		ShowError("quest_delete: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	if( sd->quest_log[i].state != Q_COMPLETE )
		sd->avail_quests--;
	if( sd->num_quests-- < MAX_QUEST_DB && sd->quest_log[i+1].quest_id )
	{
		memmove(&sd->quest_log[i], &sd->quest_log[i+1], sizeof(struct quest)*(sd->num_quests-i));
		memmove(sd->quest_index+i, sd->quest_index+i+1, sizeof(int)*(sd->num_quests-i));
	}
	memset(&sd->quest_log[sd->num_quests], 0, sizeof(struct quest));
	sd->quest_index[sd->num_quests] = 0;
	sd->save_quest = true;

	clif->quest_delete(sd, quest_id);

	if( iMap->save_settings&64 )
		chrif->save(sd,0);

	return 0;
}

int quest_update_objective_sub(struct block_list *bl, va_list ap) {
	struct map_session_data * sd;
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


void quest_update_objective(TBL_PC * sd, int mob_id) {
	int i,j;

	for( i = 0; i < sd->avail_quests; i++ ) {
		if( sd->quest_log[i].state != Q_ACTIVE )
			continue;

		for( j = 0; j < MAX_QUEST_OBJECTIVES; j++ )
			if( quest->db[sd->quest_index[i]].mob[j] == mob_id && sd->quest_log[i].count[j] < quest->db[sd->quest_index[i]].count[j] )  {
				sd->quest_log[i].count[j]++;
				sd->save_quest = true;
				clif->quest_update_objective(sd,&sd->quest_log[i],sd->quest_index[i]);
			}
	}
}

int quest_update_status(TBL_PC * sd, int quest_id, quest_state status) {
	int i;

	//Only status of active and inactive quests can be updated. Completed quests can't (for now). [Inkfish]
	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == quest_id);
	if(i == sd->avail_quests) {
		ShowError("quest_update_status: Character %d doesn't have quest %d.\n", sd->status.char_id, quest_id);
		return -1;
	}

	sd->quest_log[i].state = status;
	sd->save_quest = true;

	if( status < Q_COMPLETE ) {
		clif->quest_update_status(sd, quest_id, (bool)status);
		return 0;
	}

	if( i != (--sd->avail_quests) ) {
		struct quest tmp_quest;
		memcpy(&tmp_quest, &sd->quest_log[i],sizeof(struct quest));
		memcpy(&sd->quest_log[i], &sd->quest_log[sd->avail_quests],sizeof(struct quest));
		memcpy(&sd->quest_log[sd->avail_quests], &tmp_quest,sizeof(struct quest));
	}

	clif->quest_delete(sd, quest_id);

	if( iMap->save_settings&64 )
		chrif->save(sd,0);

	return 0;
}

int quest_check(TBL_PC * sd, int quest_id, quest_check_type type) {
	int i;

	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->num_quests )
		return -1;

	switch( type ) {
		case HAVEQUEST:
			return sd->quest_log[i].state;
		case PLAYTIME:
			return (sd->quest_log[i].time < (unsigned int)time(NULL) ? 2 : sd->quest_log[i].state == Q_COMPLETE ? 1 : 0);
		case HUNTING: {
				if( sd->quest_log[i].state == 0 || sd->quest_log[i].state == 1 ) {
					int j;
					ARR_FIND(0, MAX_QUEST_OBJECTIVES, j, sd->quest_log[i].count[j] < quest->db[sd->quest_index[i]].count[j]);
					if( j == MAX_QUEST_OBJECTIVES )
						return 2;
					if( sd->quest_log[i].time < (unsigned int)time(NULL) )
						return 1;
					return 0;
				} else
					return 0;
			}
		default:
			ShowError("quest_check_quest: Unknown parameter %d",type);
			break;
	}

	return -1;
}

int quest_read_db(void) {
	FILE *fp;
	char line[1024];
	int i,j,k = 0;
	char *str[20],*p,*np;

	sprintf(line, "%s/quest_db.txt", iMap->db_path);
	if( (fp=fopen(line,"r"))==NULL ){
		ShowError("can't read %s\n", line);
		return -1;
	}
	
	while(fgets(line, sizeof(line), fp)) {
		
		if (k == MAX_QUEST_DB) {
			ShowError("quest_read_db: Too many entries specified in %s/quest_db.txt!\n", iMap->db_path);
			break;
		}
		
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));

		for( j = 0, p = line; j < 8; j++ ) {
			if( ( np = strchr(p,',') ) != NULL ) {
				str[j] = p;
				*np = 0;
				p = np + 1;
			}
			else if (str[0] == NULL)
				continue;
			else {
				ShowError("quest_read_db: insufficient columns in line %s\n", line);
				continue;
			}
		}
		if(str[0]==NULL)
			continue;

		memset(&quest->db[k], 0, sizeof(quest->db[0]));

		quest->db[k].id = atoi(str[0]);
		quest->db[k].time = atoi(str[1]);
		
		for( i = 0; i < MAX_QUEST_OBJECTIVES; i++ ) {
			quest->db[k].mob[i] = atoi(str[2*i+2]);
			quest->db[k].count[i] = atoi(str[2*i+3]);

			if( !quest->db[k].mob[i] || !quest->db[k].count[i] )
				break;
		}
		
		quest->db[k].num_objectives = i;

		k++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", k, "quest_db.txt");
	return 0;
}

void do_init_quest(void) {
	quest->read_db();
}

void do_reload_quest(void) {
	memset(&quest->db, 0, sizeof(quest->db));
	quest->read_db();
}

void quest_defaults(void) {
	quest = &quest_s;
	
	memset(&quest->db, 0, sizeof(quest->db));
	/* */
	quest->init = do_init_quest;
	quest->reload = do_reload_quest;
	/* */
	quest->search_db = quest_search_db;
	quest->pc_login = quest_pc_login;
	quest->add = quest_add;
	quest->change = quest_change;
	quest->delete = quest_delete;
	quest->update_objective_sub = quest_update_objective_sub;
	quest->update_objective = quest_update_objective;
	quest->update_status = quest_update_status;
	quest->check = quest_check;
	quest->read_db = quest_read_db;
}
