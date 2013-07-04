// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/conf.h"

#include "battleground.h"
#include "battle.h"
#include "clif.h"
#include "map.h"
#include "npc.h"
#include "pc.h"
#include "party.h"
#include "pet.h"
#include "homunculus.h"
#include "mercenary.h"
#include "mapreg.h"

#include <string.h>
#include <stdio.h>

static DBMap* bg_team_db; // int bg_id -> struct battleground_data*
static unsigned int bg_team_counter = 0; // Next bg_id
struct battleground_interface bg_s;

struct battleground_data* bg_team_search(int bg_id) { // Search a BG Team using bg_id
	if( !bg_id ) return NULL;
	return (struct battleground_data *)idb_get(bg_team_db, bg_id);
}

struct map_session_data* bg_getavailablesd(struct battleground_data *bg) {
	int i;
	nullpo_retr(NULL, bg);
	ARR_FIND(0, MAX_BG_MEMBERS, i, bg->members[i].sd != NULL);
	return( i < MAX_BG_MEMBERS ) ? bg->members[i].sd : NULL;
}

int bg_team_delete(int bg_id)
{ // Deletes BG Team from db
	int i;
	struct map_session_data *sd;
	struct battleground_data *bg = bg_team_search(bg_id);

	if( bg == NULL ) return 0;
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		if( (sd = bg->members[i].sd) == NULL )
			continue;

		bg_send_dot_remove(sd);
		sd->bg_id = 0;
	}
	idb_remove(bg_team_db, bg_id);
	return 1;
}

int bg_team_warp(int bg_id, unsigned short mapindex, short x, short y)
{ // Warps a Team
	int i;
	struct battleground_data *bg = bg_team_search(bg_id);
	if( bg == NULL ) return 0;
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
		if( bg->members[i].sd != NULL ) pc->setpos(bg->members[i].sd, mapindex, x, y, CLR_TELEPORT);
	return 1;
}

int bg_send_dot_remove(struct map_session_data *sd)
{
	if( sd && sd->bg_id )
		clif->bg_xy_remove(sd);
	return 0;
}

int bg_team_join(int bg_id, struct map_session_data *sd)
{ // Player joins team
	int i;
	struct battleground_data *bg = bg_team_search(bg_id);
	struct map_session_data *pl_sd;

	if( bg == NULL || sd == NULL || sd->bg_id ) return 0;

	ARR_FIND(0, MAX_BG_MEMBERS, i, bg->members[i].sd == NULL);
	if( i == MAX_BG_MEMBERS ) return 0; // No free slots

	sd->bg_id = bg_id;
	bg->members[i].sd = sd;
	bg->members[i].x = sd->bl.x;
	bg->members[i].y = sd->bl.y;
	bg->count++;

	guild->send_dot_remove(sd);

	for( i = 0; i < MAX_BG_MEMBERS; i++ ) {
		if( (pl_sd = bg->members[i].sd) != NULL && pl_sd != sd )
			clif->hpmeter_single(sd->fd, pl_sd->bl.id, pl_sd->battle_status.hp, pl_sd->battle_status.max_hp);
	}

	clif->bg_hp(sd);
	clif->bg_xy(sd);
	return 1;
}

int bg_team_leave(struct map_session_data *sd, int flag)
{ // Single Player leaves team
	int i, bg_id;
	struct battleground_data *bg;
	char output[128];

	if( sd == NULL || !sd->bg_id )
		return 0;

	bg_send_dot_remove(sd);
	bg_id = sd->bg_id;
	sd->bg_id = 0;

	if( (bg = bg_team_search(bg_id)) == NULL )
		return 0;

	ARR_FIND(0, MAX_BG_MEMBERS, i, bg->members[i].sd == sd);
	if( i < MAX_BG_MEMBERS ) // Removes member from BG
		memset(&bg->members[i], 0, sizeof(bg->members[0]));
	bg->count--;

	if( flag )
		sprintf(output, "Server : %s has quit the game...", sd->status.name);
	else
		sprintf(output, "Server : %s is leaving the battlefield...", sd->status.name);
	clif->bg_message(bg, 0, "Server", output, strlen(output) + 1);

	if( bg->logout_event[0] && flag )
		npc_event(sd, bg->logout_event, 0);

	return bg->count;
}

int bg_member_respawn(struct map_session_data *sd)
{ // Respawn after killed
	struct battleground_data *bg;
	if( sd == NULL || !pc_isdead(sd) || !sd->bg_id || (bg = bg_team_search(sd->bg_id)) == NULL )
		return 0;
	if( bg->mapindex == 0 )
		return 0; // Respawn not handled by Core
	pc->setpos(sd, bg->mapindex, bg->x, bg->y, CLR_OUTSIGHT);
	iStatus->revive(&sd->bl, 1, 100);

	return 1; // Warped
}

int bg_create(unsigned short mapindex, short rx, short ry, const char *ev, const char *dev) {
	struct battleground_data *bg;
	bg_team_counter++;

	CREATE(bg, struct battleground_data, 1);
	bg->bg_id = bg_team_counter;
	bg->count = 0;
	bg->mapindex = mapindex;
	bg->x = rx;
	bg->y = ry;
	safestrncpy(bg->logout_event, ev, sizeof(bg->logout_event));
	safestrncpy(bg->die_event, dev, sizeof(bg->die_event));

	memset(&bg->members, 0, sizeof(bg->members));
	idb_put(bg_team_db, bg_team_counter, bg);

	return bg->bg_id;
}

int bg_team_get_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type )
	{
		case BL_PC:
			return ((TBL_PC*)bl)->bg_id;
		case BL_PET:
			if( ((TBL_PET*)bl)->msd )
				return ((TBL_PET*)bl)->msd->bg_id;
			break;
		case BL_MOB:
		{
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;
			if( md->special_state.ai && (msd = iMap->id2sd(md->master_id)) != NULL )
				return msd->bg_id;
			return md->bg_id;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return ((TBL_HOM*)bl)->master->bg_id;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return ((TBL_MER*)bl)->master->bg_id;
			break;
		case BL_SKILL:
			return ((TBL_SKILL*)bl)->group->bg_id;
	}

	return 0;
}

int bg_send_message(struct map_session_data *sd, const char *mes, int len)
{
	struct battleground_data *bg;

	nullpo_ret(sd);
	if( sd->bg_id == 0 || (bg = bg_team_search(sd->bg_id)) == NULL )
		return 0;
	clif->bg_message(bg, sd->bl.id, sd->status.name, mes, len);
	return 0;
}

/**
 * @see DBApply
 */
int bg_send_xy_timer_sub(DBKey key, DBData *data, va_list ap)
{
	struct battleground_data *bg = DB->data2ptr(data);
	struct map_session_data *sd;
	int i;
	nullpo_ret(bg);
	for( i = 0; i < MAX_BG_MEMBERS; i++ ) {
		if( (sd = bg->members[i].sd) == NULL )
			continue;
		if( sd->bl.x != bg->members[i].x || sd->bl.y != bg->members[i].y ) { // xy update
			bg->members[i].x = sd->bl.x;
			bg->members[i].y = sd->bl.y;
			clif->bg_xy(sd);
		}
	}
	return 0;
}

int bg_send_xy_timer(int tid, unsigned int tick, int id, intptr_t data) {
	bg_team_db->foreach(bg_team_db, bg_send_xy_timer_sub, tick);
	return 0;
}
void bg_config_read(void) {
	config_t bg_conf;
	config_setting_t *data = NULL;
	const char *config_filename = "conf/battlegrounds.conf"; // FIXME hardcoded name
	
	if (conf_read_file(&bg_conf, config_filename))
		return;
	
	data = config_lookup(&bg_conf, "battlegrounds");
	
	if (data != NULL) {
		config_setting_t *settings = config_setting_get_elem(data, 0);
		config_setting_t *arenas;
		const char *delay_var;
		int i, arena_count = 0, offline = 0;
		
		if( !config_setting_lookup_string(settings, "global_delay_var", &delay_var) )
			delay_var = "BG_Delay_Tick";
		
		safestrncpy(bg->gdelay_var, delay_var, BG_DELAY_VAR_LENGTH);
		
		config_setting_lookup_int(settings, "maximum_afk_seconds", &bg->mafksec);
		
		config_setting_lookup_bool(settings, "feature_off", &offline);

		if( offline == 0 )
			bg->queue_on = true;

		if( (arenas = config_setting_get_member(settings, "arenas")) != NULL ) {
			arena_count = config_setting_length(arenas);
			CREATE( bg->arena, struct bg_arena *, arena_count );
			for(i = 0; i < arena_count; i++) {
				config_setting_t *arena = config_setting_get_elem(arenas, i);
				config_setting_t *reward;
				const char *aName, *aEvent, *aDelayVar;
				int minLevel = 0, maxLevel = 0;
				int prizeWin, prizeLoss, prizeDraw;
				int minPlayers, maxPlayers, minTeamPlayers;
				int maxDuration;
				int fillup_duration = 0, pregame_duration = 0;
				
				bg->arena[i] = NULL;
				
				if( !config_setting_lookup_string(arena, "name", &aName) ) {
					ShowError("bg_config_read: failed to find 'name' for arena #%d\n",i);
					continue;
				}
				
				if( !config_setting_lookup_string(arena, "event", &aEvent) ) {
					ShowError("bg_config_read: failed to find 'event' for arena #%d\n",i);
					continue;
				}

				config_setting_lookup_int(arena, "minLevel", &minLevel);
				config_setting_lookup_int(arena, "maxLevel", &maxLevel);
				
				if( minLevel < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' minLevel\n",minLevel,aName);
					minLevel = 0;
				}
				if( maxLevel > MAX_LEVEL ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' maxLevel\n",maxLevel,aName);
					maxLevel = MAX_LEVEL;
				}
				
				if( !(reward = config_setting_get_member(arena, "reward")) ) {
					ShowError("bg_config_read: failed to find 'reward' for arena '%s'/#%d\n",aName,i);
					continue;
				}
				
				config_setting_lookup_int(reward, "win", &prizeWin);
				config_setting_lookup_int(reward, "loss", &prizeLoss);
				config_setting_lookup_int(reward, "draw", &prizeDraw);
				
				if( prizeWin < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' reward:win\n",prizeWin,aName);
					prizeWin = 0;
				}
				if( prizeLoss < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' reward:loss\n",prizeLoss,aName);
					prizeLoss = 0;
				}
				if( prizeDraw < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' reward:draw\n",prizeDraw,aName);
					prizeDraw = 0;
				}
				
				config_setting_lookup_int(arena, "minPlayers", &minPlayers);
				config_setting_lookup_int(arena, "maxPlayers", &maxPlayers);
				config_setting_lookup_int(arena, "minTeamPlayers", &minTeamPlayers);
				
				if( minPlayers < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' minPlayers\n",minPlayers,aName);
					minPlayers = 0;
				}
				if( maxPlayers > MAX_BG_MEMBERS * 2 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' maxPlayers, change #define MAX_BG_MEMBERS\n",maxPlayers,aName);
					maxPlayers = 0;
				}
				if( minTeamPlayers < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' minTeamPlayers\n",minTeamPlayers,aName);
					minTeamPlayers = 0;
				}

				if( !config_setting_lookup_string(arena, "delay_var", &aDelayVar) ) {
					ShowError("bg_config_read: failed to find 'delay_var' for arena '%s'/#%d\n",aName,i);
					continue;
				}
				
				config_setting_lookup_int(arena, "maxDuration", &maxDuration);
				
				if( maxDuration < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' maxDuration\n",maxDuration,aName);
					maxDuration = 30;
				}
				
				config_setting_lookup_int(arena, "fillDuration", &fillup_duration);
				config_setting_lookup_int(arena, "pGameDuration", &pregame_duration);

				if( fillup_duration < 20 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' fillDuration, minimum has to be 20, defaulting to 20.\n",fillup_duration,aName);
					fillup_duration = 20;
				}

				if( pregame_duration < 20 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' pGameDuration, minimum has to be 20, defaulting to 20.\n",pregame_duration,aName);
					pregame_duration = 20;
				}

				
				CREATE( bg->arena[i], struct bg_arena, 1 );
				
				bg->arena[i]->id = i;
				safestrncpy(bg->arena[i]->name, aName, NAME_LENGTH);
				safestrncpy(bg->arena[i]->npc_event, aEvent, EVENT_NAME_LENGTH);
				bg->arena[i]->min_level = minLevel;
				bg->arena[i]->max_level = maxLevel;
				bg->arena[i]->prize_win = prizeWin;
				bg->arena[i]->prize_loss = prizeLoss;
				bg->arena[i]->prize_draw = prizeDraw;
				bg->arena[i]->min_players = minPlayers;
				bg->arena[i]->max_players = maxPlayers;
				bg->arena[i]->min_team_players = minTeamPlayers;
				safestrncpy(bg->arena[i]->delay_var, aDelayVar, NAME_LENGTH);
				bg->arena[i]->maxDuration = maxDuration;
				bg->arena[i]->queue_id = script->queue_create();
				bg->arena[i]->begin_timer = INVALID_TIMER;
				bg->arena[i]->fillup_timer = INVALID_TIMER;
				bg->arena[i]->pregame_duration = pregame_duration;
				bg->arena[i]->fillup_duration = fillup_duration;

			}
			bg->arenas = arena_count;
		}
		
		config_destroy(&bg_conf);
	}
}
struct bg_arena *bg_name2arena (char *name) {
	int i;
	for(i = 0; i < bg->arenas; i++) {
		if( strcmpi(bg->arena[i]->name,name) == 0 )
			return bg->arena[i];
	}
	return NULL;
}
int bg_id2pos ( int queue_id, int account_id ) {
	struct hQueue *queue = script->queue(queue_id);
	if( queue ) {
		int i;
		for(i = 0; i < queue->items; i++ ) {
			if( queue->item[i] == account_id ) {
				return i;
			}
		}
	}
	return 0;
}
void bg_queue_ready_ack (struct bg_arena *arena, struct map_session_data *sd, bool response) {
	if( arena->begin_timer == INVALID_TIMER || !sd->bg_queue.arena || sd->bg_queue.arena != arena ) {
		bg->queue_pc_cleanup(sd);
		return;
	}
	if( !response )
		bg->queue_pc_cleanup(sd);
	else {
		struct hQueue *queue = &script->hq[arena->queue_id];
		int i, count = 0;
		sd->bg_queue.ready = 1;
		
		for( i = 0; i < queue->items; i++ ) {
			if( ( sd = iMap->id2sd(queue->item[i]) ) ) {
				if( sd->bg_queue.ready == 1 )
					count++;
			}
		}
		/* check if all are ready then cancell timer, and start game  */
		if( count == i ) {
			iTimer->delete_timer(arena->begin_timer,bg->begin_timer);
			arena->begin_timer = INVALID_TIMER;
			bg->begin(arena);
		}

	}
	
}
void bg_queue_player_cleanup(struct map_session_data *sd) {
	if ( sd->bg_queue.client_has_bg_data ) {
		clif->bgqueue_notice_delete(sd,BGQND_CLOSEWINDOW, sd->bg_queue.arena ? sd->bg_queue.arena->id : 0);
	}
	if( sd->bg_queue.arena )
		script->queue_remove(sd->bg_queue.arena->queue_id,sd->status.account_id);
	sd->bg_queue.arena = NULL;
	sd->bg_queue.ready = 0;
	sd->bg_queue.client_has_bg_data = 0;
	sd->bg_queue.type = 0;
}
void bg_match_over(struct bg_arena *arena, bool canceled) {
	struct hQueue *queue = &script->hq[arena->queue_id];
	int i;
	
	for( i = 0; i < queue->items; i++ ) {
		struct map_session_data * sd = NULL;
		
		if( ( sd = iMap->id2sd(queue->item[i]) ) ) {
			bg->queue_pc_cleanup(sd);
			if( canceled )
				clif->colormes(sd->fd,COLOR_RED,"BG Match Cancelled: not enough players");
		}
	}

	bg->arena[i]->begin_timer = INVALID_TIMER;
	bg->arena[i]->fillup_timer = INVALID_TIMER;
	/* reset queue */
	script->queue_clear(arena->queue_id);
}
void bg_begin(struct bg_arena *arena) {
	struct hQueue *queue = &script->hq[arena->queue_id];
	int i, count = 0;

	for( i = 0; i < queue->items; i++ ) {
		struct map_session_data * sd = NULL;
		
		if( ( sd = iMap->id2sd(queue->item[i]) ) ) {
			if( sd->bg_queue.ready == 1 )
				count++;
			else
				bg->queue_pc_cleanup(sd);
		}
	}

	if( count < arena->min_players ) {
		bg->match_over(arena,true);
	} else {
		mapreg_setreg(add_str("$@bg_queue_id"),arena->queue_id);/* TODO: make this a arena-independant var? or just .@? */
		npc_event_do(arena->npc_event);
		/* we split evenly? */
		/* but if a party of say 10 joins, it cant be split evenly unless by luck there are 10 soloers in the queue besides them */
		/* not sure how to split T_T needs more info */
		/* currently running only on solo mode so we do it evenly */
	}
}
int bg_begin_timer(int tid, unsigned int tick, int id, intptr_t data) {
	bg->begin(bg->arena[id]);
	return 0;
}

void bg_queue_pregame(struct bg_arena *arena) {
	struct hQueue *queue = &script->hq[arena->queue_id];
	int i;
	
	for( i = 0; i < queue->items; i++ ) {
		struct map_session_data * sd = NULL;
		
		if( ( sd = iMap->id2sd(queue->item[i]) ) ) {
			clif->bgqueue_battlebegins(sd,arena->id,SELF);
		}
	}
	arena->begin_timer = iTimer->add_timer( iTimer->gettick() + (arena->pregame_duration*1000), bg->begin_timer, arena->id, 0 );
}
int bg_fillup_timer(int tid, unsigned int tick, int id, intptr_t data) {
	bg->queue_pregame(bg->arena[id]);
	return 0;
}

void bg_queue_check(struct bg_arena *arena) {
	int count = script->hq[arena->queue_id].items;
	
	if( count == arena->max_players ) {
		if( arena->fillup_timer != INVALID_TIMER ) {
			iTimer->delete_timer(arena->fillup_timer,bg_fillup_timer);
			arena->fillup_timer = INVALID_TIMER;
		}
		bg->queue_pregame(arena);
	} else if( count >= arena->min_players && arena->fillup_timer == INVALID_TIMER ) {
		arena->fillup_timer = iTimer->add_timer( iTimer->gettick() + (arena->fillup_duration*1000), bg->fillup_timer, arena->id, 0 );
	}
}
void bg_queue_add(struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type) {
	enum BATTLEGROUNDS_QUEUE_ACK result = bg->can_queue(sd,arena,type);
	struct hQueue *queue;
	int i, count = 0;
	
	if( arena->begin_timer != INVALID_TIMER ) {
		clif->bgqueue_ack(sd,BGQA_FAIL_QUEUING_FINISHED,arena->id);
		return;
	}
	
	if( result != BGQA_SUCCESS ) {
		clif->bgqueue_ack(sd,result,arena->id);
		return;
	}
		
	switch( type ) { /* guild/party already validated in can_queue */
		case BGQT_PARTY: {
			struct party_data *p = party->search(sd->status.party_id);
			for( i = 0; i < MAX_PARTY; i++ ) {
				if( !p->data[i].sd || p->data[i].sd->bg_queue.arena != NULL ) continue;
				count++;
			}
		}
			break;
		case BGQT_GUILD:
			for ( i=0; i<sd->guild->max_member; i++ ) {
				if ( !sd->guild->member[i].sd || sd->guild->member[i].sd->bg_queue.arena != NULL )
					continue;
				count++;
			}
			break;
		case BGQT_INDIVIDUAL:
			count = 1;
			break;
	}
	
	if( !(queue = script->queue(arena->queue_id)) || (queue->items+count) >= arena->max_players ) {
		clif->bgqueue_ack(sd,BGQA_FAIL_PPL_OVERAMOUNT,arena->id);
		return;
	}
		
	switch( type ) {
		case BGQT_INDIVIDUAL:
			sd->bg_queue.type = type;
			sd->bg_queue.arena = arena;
			sd->bg_queue.ready = 0;
			script->queue_add(arena->queue_id,sd->status.account_id);
			clif->bgqueue_joined(sd,script->hq[arena->queue_id].items);
			clif->bgqueue_update_info(sd,arena->id,script->hq[arena->queue_id].items);
			break;
		case BGQT_PARTY: {
			struct party_data *p = party->search(sd->status.party_id);
			for( i = 0; i < MAX_PARTY; i++ ) {
				if( !p->data[i].sd || p->data[i].sd->bg_queue.arena != NULL ) continue;
				p->data[i].sd->bg_queue.type = type;
				p->data[i].sd->bg_queue.arena = arena;
				p->data[i].sd->bg_queue.ready = 0;
				script->queue_add(arena->queue_id,p->data[i].sd->status.account_id);
				clif->bgqueue_joined(p->data[i].sd,script->hq[arena->queue_id].items);
				clif->bgqueue_update_info(p->data[i].sd,arena->id,script->hq[arena->queue_id].items);
			}
		}
			break;
		case BGQT_GUILD:
			for ( i=0; i<sd->guild->max_member; i++ ) {
				if ( !sd->guild->member[i].sd || sd->guild->member[i].sd->bg_queue.arena != NULL )
					continue;
				sd->guild->member[i].sd->bg_queue.type = type;
				sd->guild->member[i].sd->bg_queue.arena = arena;
				sd->guild->member[i].sd->bg_queue.ready = 0;
				script->queue_add(arena->queue_id,sd->guild->member[i].sd->status.account_id);
				clif->bgqueue_joined(sd->guild->member[i].sd,script->hq[arena->queue_id].items);
				clif->bgqueue_update_info(sd->guild->member[i].sd,arena->id,script->hq[arena->queue_id].items);
			}
			break;
	}
		
	clif->bgqueue_ack(sd,BGQA_SUCCESS,arena->id);
	
	bg->queue_check(arena);
}
enum BATTLEGROUNDS_QUEUE_ACK bg_canqueue(struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type) {
	int tick;
	unsigned int tsec;
	if ( sd->status.base_level > arena->max_level || sd->status.base_level < arena->min_level )
		return BGQA_FAIL_LEVEL_INCORRECT;
	
	if ( !(sd->class_&JOBL_2) ) /* TODO: maybe make this a per-arena setting, so users may make custom arenas like baby-only,whatever. */
		return BGQA_FAIL_CLASS_INVALID;
	
	tsec = (unsigned int)time(NULL);
	
	if ( ( tick = pc_readglobalreg(sd, bg->gdelay_var) ) && tsec < tick ) {
		char response[100];
		if( (tick-tsec) > 60 )
			sprintf(response, "You are a deserter! Wait %d minute(s) before you can apply again",(tick-tsec)/60);
		else
			sprintf(response, "You are a deserter! Wait %d seconds before you can apply again",(tick-tsec));
		clif->colormes(sd->fd,COLOR_RED,response);
		return BGQA_FAIL_DESERTER;
	}
	
	if ( ( tick = pc_readglobalreg(sd, arena->cooldown_variable) ) && tsec < tick ) {
		char response[100];
		if( (tick-tsec) > 60 )
			sprintf(response, "You can't reapply to this arena so fast. Apply to the different arena or wait %d minute(s)",(tick-tsec)/60);
		else
			sprintf(response, "You can't reapply to this arena so fast. Apply to the different arena or wait %d seconds",(tick-tsec));
		clif->colormes(sd->fd,COLOR_RED,response);
		return BGQA_FAIL_COOLDOWN;
	}

	if( sd->bg_queue.arena != NULL )
		return BGQA_DUPLICATE_REQUEST;
	
	if( type != BGQT_INDIVIDUAL ) {/* until we get the damn balancing correct */
		clif->colormes(sd->fd,COLOR_RED,"Queueing is only currently enabled only for Solo Mode");
		return BGQA_FAIL_TEAM_COUNT;
	}
		
	switch(type) {
		case BGQT_GUILD:
			if( !sd->guild || !sd->state.gmaster_flag )
				return BGQA_NOT_PARTY_GUILD_LEADER;
			else {
				int i, count = 0;
				for ( i=0; i<sd->guild->max_member; i++ ) {
					if ( !sd->guild->member[i].sd || sd->guild->member[i].sd->bg_queue.arena != NULL )
						continue;
					count++;
				}
				if ( count < arena->min_team_players ) {
					char response[100];
					if( count != sd->guild->connect_member && sd->guild->connect_member >= arena->min_team_players )
						sprintf(response, "Can't apply: not enough members in your team/guild that have not entered the queue in individual mode, minimum is %d",arena->min_team_players);
					else
						sprintf(response, "Can't apply: not enough members in your team/guild, minimum is %d",arena->min_team_players);
					clif->colormes(sd->fd,COLOR_RED,response);
					return BGQA_FAIL_TEAM_COUNT;
				}
			}
			break;
		case BGQT_PARTY:
			if( !sd->status.party_id )
				return BGQA_NOT_PARTY_GUILD_LEADER;
			else {
				struct party_data *p;
				if( (p = party->search(sd->status.party_id) ) ) {
					int i, count = 0;
					bool is_leader = false;

					for(i = 0; i < MAX_PARTY; i++) {
						if( !p->data[i].sd )
							continue;
						if( p->party.member[i].leader && sd == p->data[i].sd )
							is_leader = true;
						if( p->data[i].sd->bg_queue.arena == NULL )
							count++;
					}

					if( !is_leader )
						return BGQA_NOT_PARTY_GUILD_LEADER;
										
					if( count < arena->min_team_players ) {
						char response[100];
						if( count != p->party.count && p->party.count >= arena->min_team_players )
							sprintf(response, "Can't apply: not enough members in your team/party that have not entered the queue in individual mode, minimum is %d",arena->min_team_players);
						else
							sprintf(response, "Can't apply: not enough members in your team/party, minimum is %d",arena->min_team_players);
						clif->colormes(sd->fd,COLOR_RED,response);
						return BGQA_FAIL_TEAM_COUNT;
					}
					
				} else
					return BGQA_NOT_PARTY_GUILD_LEADER;
			}
			break;
		case BGQT_INDIVIDUAL:/* already did */
			break;
		default:
			ShowDebug("bg_canqueue: unknown/unsupported type %d\n",type);
			return BGQA_DUPLICATE_REQUEST;
	}
	
	return BGQA_SUCCESS;
}
void do_init_battleground(void) {
	bg_team_db = idb_alloc(DB_OPT_RELEASE_DATA);
	iTimer->add_timer_func_list(bg_send_xy_timer, "bg_send_xy_timer");
	iTimer->add_timer_interval(iTimer->gettick() + battle_config.bg_update_interval, bg_send_xy_timer, 0, 0, battle_config.bg_update_interval);
	bg->config_read();
}

void do_final_battleground(void) {
	int i;
	
	bg_team_db->destroy(bg_team_db, NULL);
	
	for( i = 0; i < bg->arenas; i++ ) {
		if( bg->arena[i] )
			aFree(bg->arena[i]);
	}
	
	if( bg->arena )
		aFree(bg->arena);
}
void battleground_defaults(void) {
	bg = &bg_s;
	
	bg->queue_on = false;
	
	bg->mafksec = 0;
	bg->arena = NULL;
	bg->arenas = 0;
	/* */
	bg->name2arena = bg_name2arena;
	bg->queue_add = bg_queue_add;
	bg->can_queue = bg_canqueue;
	bg->id2pos = bg_id2pos;
	bg->queue_pc_cleanup = bg_queue_player_cleanup;
	bg->begin = bg_begin;
	bg->begin_timer = bg_begin_timer;
	bg->queue_pregame = bg_queue_pregame;
	bg->fillup_timer = bg_fillup_timer;
	bg->queue_ready_ack = bg_queue_ready_ack;
	bg->match_over = bg_match_over;
	bg->queue_check = bg_queue_check;
	/* */
	bg->config_read = bg_config_read;
}
