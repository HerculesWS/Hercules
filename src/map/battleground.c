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

#include "battleground.h"

#include "map/battle.h"
#include "map/clif.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/map.h"
#include "map/mapreg.h"
#include "map/mercenary.h"
#include "map/mob.h" // struct mob_data
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/pet.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/HPM.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdio.h>
#include <string.h>

struct battleground_interface bg_s;
struct battleground_interface *bg;

/// Search a BG Team using bg_id
struct battleground_data* bg_team_search(int bg_id) {
	if( !bg_id ) return NULL;
	return (struct battleground_data *)idb_get(bg->team_db, bg_id);
}

struct map_session_data* bg_getavailablesd(struct battleground_data *bgd) {
	int i;
	nullpo_retr(NULL, bgd);
	ARR_FIND(0, MAX_BG_MEMBERS, i, bgd->members[i].sd != NULL);
	return( i < MAX_BG_MEMBERS ) ? bgd->members[i].sd : NULL;
}

/// Deletes BG Team from db
bool bg_team_delete(int bg_id) {
	int i;
	struct battleground_data *bgd = bg->team_search(bg_id);

	if( bgd == NULL ) return false;
	for( i = 0; i < MAX_BG_MEMBERS; i++ ) {
		struct map_session_data *sd = bgd->members[i].sd;
		if (sd == NULL)
			continue;

		bg->send_dot_remove(sd);
		sd->bg_id = 0;
	}
	idb_remove(bg->team_db, bg_id);
	return true;
}

/// Warps a Team
bool bg_team_warp(int bg_id, unsigned short map_index, short x, short y) {
	int i;
	struct battleground_data *bgd = bg->team_search(bg_id);
	if( bgd == NULL ) return false;
	for( i = 0; i < MAX_BG_MEMBERS; i++ )
		if( bgd->members[i].sd != NULL ) pc->setpos(bgd->members[i].sd, map_index, x, y, CLR_TELEPORT);
	return true;
}

void bg_send_dot_remove(struct map_session_data *sd) {
	if( sd && sd->bg_id )
		clif->bg_xy_remove(sd);
}

/// Player joins team
bool bg_team_join(int bg_id, struct map_session_data *sd) {
	int i;
	struct battleground_data *bgd = bg->team_search(bg_id);

	if( bgd == NULL || sd == NULL || sd->bg_id ) return false;

	ARR_FIND(0, MAX_BG_MEMBERS, i, bgd->members[i].sd == NULL);
	if( i == MAX_BG_MEMBERS ) return false; // No free slots

	sd->bg_id = bg_id;
	bgd->members[i].sd = sd;
	bgd->members[i].x = sd->bl.x;
	bgd->members[i].y = sd->bl.y;
	/* populate 'where i came from' */
	if(map->list[sd->bl.m].flag.nosave || map->list[sd->bl.m].instance_id >= 0) {
		struct map_data *m=&map->list[sd->bl.m];
		if(m->save.map)
			memcpy(&bgd->members[i].source,&m->save,sizeof(struct point));
		else
			memcpy(&bgd->members[i].source,&sd->status.save_point,sizeof(struct point));
	} else
		memcpy(&bgd->members[i].source,&sd->status.last_point,sizeof(struct point));
	bgd->count++;

	guild->send_dot_remove(sd);

	for( i = 0; i < MAX_BG_MEMBERS; i++ ) {
		struct map_session_data *pl_sd = bgd->members[i].sd;
		if (pl_sd != NULL && pl_sd != sd)
			clif->hpmeter_single(sd->fd, pl_sd->bl.id, pl_sd->battle_status.hp, pl_sd->battle_status.max_hp);
	}

	clif->bg_hp(sd);
	clif->bg_xy(sd);
	return true;
}

/// Single Player leaves team
int bg_team_leave(struct map_session_data *sd, enum bg_team_leave_type flag) {
	int i, bg_id;
	struct battleground_data *bgd;

	if( sd == NULL || !sd->bg_id )
		return 0;
	bg->send_dot_remove(sd);
	bg_id = sd->bg_id;
	sd->bg_id = 0;

	if( (bgd = bg->team_search(bg_id)) == NULL )
		return 0;

	ARR_FIND(0, MAX_BG_MEMBERS, i, bgd->members[i].sd == sd);
	if( i < MAX_BG_MEMBERS ) { // Removes member from BG
		if( sd->bg_queue.arena ) {
			bg->queue_pc_cleanup(sd);
			pc->setpos(sd,bgd->members[i].source.map, bgd->members[i].source.x, bgd->members[i].source.y, CLR_OUTSIGHT);
		}
		memset(&bgd->members[i], 0, sizeof(bgd->members[0]));
	}

	if (--bgd->count != 0) {
		char output[128];
		switch (flag) {
			default:
			case BGTL_QUIT:
				sprintf(output, "Server : %s has quit the game...", sd->status.name);
				break;
			case BGTL_LEFT:
				sprintf(output, "Server : %s is leaving the battlefield...", sd->status.name);
				break;
			case BGTL_AFK:
				sprintf(output, "Server : %s has been afk-kicked from the battlefield...", sd->status.name);
				break;
		}
		clif->bg_message(bgd, 0, "Server", output);
	}

	if( bgd->logout_event[0] && flag )
		npc->event(sd, bgd->logout_event, 0);

	if( sd->bg_queue.arena ) {
		bg->queue_pc_cleanup(sd);
	}

	return bgd->count;
}

/// Respawn after killed
bool bg_member_respawn(struct map_session_data *sd) {
	struct battleground_data *bgd;
	if( sd == NULL || !pc_isdead(sd) || !sd->bg_id || (bgd = bg->team_search(sd->bg_id)) == NULL )
		return false;
	if( bgd->mapindex == 0 )
		return false; // Respawn not handled by Core
	pc->setpos(sd, bgd->mapindex, bgd->x, bgd->y, CLR_OUTSIGHT);
	status->revive(&sd->bl, 1, 100);

	return true; // Warped
}

int bg_create(unsigned short map_index, short rx, short ry, const char *ev, const char *dev) {
	struct battleground_data *bgd;
	bg->team_counter++;

	CREATE(bgd, struct battleground_data, 1);
	bgd->bg_id = bg->team_counter;
	bgd->count = 0;
	bgd->mapindex = map_index;
	bgd->x = rx;
	bgd->y = ry;
	safestrncpy(bgd->logout_event, ev, sizeof(bgd->logout_event));
	safestrncpy(bgd->die_event, dev, sizeof(bgd->die_event));

	memset(&bgd->members, 0, sizeof(bgd->members));
	idb_put(bg->team_db, bg->team_counter, bgd);

	return bgd->bg_id;
}

int bg_team_get_id(struct block_list *bl) {
	nullpo_ret(bl);
	switch( bl->type ) {
		case BL_PC:
		{
			const struct map_session_data *sd = BL_UCCAST(BL_PC, bl);
			return sd->bg_id;
		}
		case BL_PET:
		{
			const struct pet_data *pd = BL_UCCAST(BL_PET, bl);
			if (pd->msd != NULL)
				return pd->msd->bg_id;
		}
			break;
		case BL_MOB:
		{
			const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
			const struct map_session_data *msd;
			if (md->special_state.ai != AI_NONE && (msd = map->id2sd(md->master_id)) != NULL)
				return msd->bg_id;
			return md->bg_id;
		}
		case BL_HOM:
		{
			const struct homun_data *hd = BL_UCCAST(BL_HOM, bl);
			if (hd->master != NULL)
				return hd->master->bg_id;
		}
			break;
		case BL_MER:
		{
			const struct mercenary_data *md = BL_UCCAST(BL_MER, bl);
			if (md->master != NULL)
				return md->master->bg_id;
		}
			break;
		case BL_SKILL:
		{
			const struct skill_unit *su = BL_UCCAST(BL_SKILL, bl);
			if (su->group != NULL)
				return su->group->bg_id;
		}
			break;
	}

	return 0;
}

bool bg_send_message(struct map_session_data *sd, const char *mes)
{
	struct battleground_data *bgd;

	nullpo_ret(sd);
	nullpo_ret(mes);
	if( sd->bg_id == 0 || (bgd = bg->team_search(sd->bg_id)) == NULL )
		return false; // Couldn't send message
	clif->bg_message(bgd, sd->bl.id, sd->status.name, mes);
	return true;
}

/**
 * @see DBApply
 */
int bg_send_xy_timer_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct battleground_data *bgd = DB->data2ptr(data);
	struct map_session_data *sd;
	int i;
	nullpo_ret(bgd);
	for( i = 0; i < MAX_BG_MEMBERS; i++ ) {
		if( (sd = bgd->members[i].sd) == NULL )
			continue;
		if( sd->bl.x != bgd->members[i].x || sd->bl.y != bgd->members[i].y ) { // xy update
			bgd->members[i].x = sd->bl.x;
			bgd->members[i].y = sd->bl.y;
			clif->bg_xy(sd);
		}
	}
	return 0;
}

int bg_send_xy_timer(int tid, int64 tick, int id, intptr_t data) {
	bg->team_db->foreach(bg->team_db, bg->send_xy_timer_sub, tick);
	return 0;
}

enum bg_queue_types bg_str2teamtype (const char *str) {
	char temp[200], *parse;
	enum bg_queue_types type = BGQT_INVALID;

	nullpo_retr(type, str);
	safestrncpy(temp, str, 200);

	parse = strtok(temp,"|");

	while (parse != NULL) {
		normalize_name(parse," ");
		if( strcmpi(parse,"all") == 0 )
			type |= BGQT_INDIVIDUAL|BGQT_PARTY|BGQT_GUILD;
		else if( strcmpi(parse,"party") == 0 )
			type |= BGQT_PARTY;
		else if( strcmpi(parse,"guild") == 0 )
			type |= BGQT_GUILD;
		else if( strcmpi(parse,"solo") == 0 )
			type |= BGQT_INDIVIDUAL;
		else {
			ShowError("bg_str2teamtype: '%s' unknown type, skipping...\n",parse);
		}
		parse = strtok(NULL,"|");
	}

	return type;
}

void bg_config_read(void) {
	struct config_t bg_conf;
	struct config_setting_t *data = NULL;
	const char *config_filename = "conf/battlegrounds.conf"; // FIXME hardcoded name

	if (!libconfig->load_file(&bg_conf, config_filename))
		return;

	data = libconfig->lookup(&bg_conf, "battlegrounds");

	if (data != NULL) {
		struct config_setting_t *settings = libconfig->setting_get_elem(data, 0);
		struct config_setting_t *arenas;
		const char *delay_var;
		int offline = 0;

		if( !libconfig->setting_lookup_string(settings, "global_delay_var", &delay_var) )
			delay_var = "BG_Delay_Tick";

		safestrncpy(bg->gdelay_var, delay_var, BG_DELAY_VAR_LENGTH);

		libconfig->setting_lookup_int(settings, "maximum_afk_seconds", &bg->mafksec);
		libconfig->setting_lookup_bool(settings, "feature_off", &offline);

		if( offline == 0 )
			bg->queue_on = true;

		if( (arenas = libconfig->setting_get_member(settings, "arenas")) != NULL ) {
			int i;
			int arena_count = libconfig->setting_length(arenas);
			CREATE( bg->arena, struct bg_arena *, arena_count );
			for(i = 0; i < arena_count; i++) {
				struct config_setting_t *arena = libconfig->setting_get_elem(arenas, i);
				struct config_setting_t *reward;
				const char *aName, *aEvent, *aDelayVar, *aTeamTypes;
				int minLevel = 0, maxLevel = 0;
				int prizeWin, prizeLoss, prizeDraw;
				int minPlayers, maxPlayers, minTeamPlayers;
				int maxDuration;
				int fillup_duration = 0, pregame_duration = 0;
				enum bg_queue_types allowedTypes;

				bg->arena[i] = NULL;

				if( !libconfig->setting_lookup_string(arena, "name", &aName) ) {
					ShowError("bg_config_read: failed to find 'name' for arena #%d\n",i);
					continue;
				}

				if( !libconfig->setting_lookup_string(arena, "event", &aEvent) ) {
					ShowError("bg_config_read: failed to find 'event' for arena #%d\n",i);
					continue;
				}

				libconfig->setting_lookup_int(arena, "minLevel", &minLevel);
				libconfig->setting_lookup_int(arena, "maxLevel", &maxLevel);

				if( minLevel < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' minLevel\n",minLevel,aName);
					minLevel = 0;
				}
				if( maxLevel > MAX_LEVEL ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' maxLevel\n",maxLevel,aName);
					maxLevel = MAX_LEVEL;
				}

				if( !(reward = libconfig->setting_get_member(arena, "reward")) ) {
					ShowError("bg_config_read: failed to find 'reward' for arena '%s'/#%d\n",aName,i);
					continue;
				}

				libconfig->setting_lookup_int(reward, "win", &prizeWin);
				libconfig->setting_lookup_int(reward, "loss", &prizeLoss);
				libconfig->setting_lookup_int(reward, "draw", &prizeDraw);

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

				libconfig->setting_lookup_int(arena, "minPlayers", &minPlayers);
				libconfig->setting_lookup_int(arena, "maxPlayers", &maxPlayers);
				libconfig->setting_lookup_int(arena, "minTeamPlayers", &minTeamPlayers);

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

				if( !libconfig->setting_lookup_string(arena, "delay_var", &aDelayVar) ) {
					ShowError("bg_config_read: failed to find 'delay_var' for arena '%s'/#%d\n",aName,i);
					continue;
				}

				if( !libconfig->setting_lookup_string(arena, "allowedTypes", &aTeamTypes) ) {
					ShowError("bg_config_read: failed to find 'allowedTypes' for arena '%s'/#%d\n",aName,i);
					continue;
				}

				libconfig->setting_lookup_int(arena, "maxDuration", &maxDuration);

				if( maxDuration < 0 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' maxDuration\n",maxDuration,aName);
					maxDuration = 30;
				}

				libconfig->setting_lookup_int(arena, "fillDuration", &fillup_duration);
				libconfig->setting_lookup_int(arena, "pGameDuration", &pregame_duration);

				if( fillup_duration < 20 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' fillDuration, minimum has to be 20, defaulting to 20.\n",fillup_duration,aName);
					fillup_duration = 20;
				}

				if( pregame_duration < 20 ) {
					ShowWarning("bg_config_read: invalid %d value for arena '%s' pGameDuration, minimum has to be 20, defaulting to 20.\n",pregame_duration,aName);
					pregame_duration = 20;
				}

				allowedTypes = bg->str2teamtype(aTeamTypes);

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
				bg->arena[i]->ongoing = false;
				bg->arena[i]->allowed_types = allowedTypes;

			}
			bg->arenas = arena_count;
		}
	}
	libconfig->destroy(&bg_conf);
}
struct bg_arena *bg_name2arena(const char *name)
{
	int i;
	nullpo_retr(NULL, name);
	for(i = 0; i < bg->arenas; i++) {
		if( strcmpi(bg->arena[i]->name,name) == 0 )
			return bg->arena[i];
	}
	return NULL;
}

/**
 * Returns a character's position in a battleground's queue.
 *
 * @param queue_id   The ID of the battleground's queue.
 * @param account_id The character's account ID.
 *
 * @return the position (starting at 1).
 * @retval 0 if the queue doesn't exist or the given account ID isn't present in it.
 */
int bg_id2pos(int queue_id, int account_id)
{
	struct script_queue *queue = script->queue(queue_id);
	if (queue) {
		int i;
		ARR_FIND(0, VECTOR_LENGTH(queue->entries), i, VECTOR_INDEX(queue->entries, i) == account_id);
		if (i != VECTOR_LENGTH(queue->entries)) {
			return i+1;
		}
	}
	return 0;
}

void bg_queue_ready_ack(struct bg_arena *arena, struct map_session_data *sd, bool response)
{
	nullpo_retv(arena);
	nullpo_retv(sd);

	if( arena->begin_timer == INVALID_TIMER || !sd->bg_queue.arena || sd->bg_queue.arena != arena ) {
		bg->queue_pc_cleanup(sd);
		return;
	}
	if( !response )
		bg->queue_pc_cleanup(sd);
	else {
		struct script_queue *queue = script->queue(arena->queue_id);
		int i, count = 0;
		sd->bg_queue.ready = 1;

		for (i = 0; i < VECTOR_LENGTH(queue->entries); i++) {
			struct map_session_data *tsd = map->id2sd(VECTOR_INDEX(queue->entries, i));

			if (tsd != NULL && tsd->bg_queue.ready == 1)
				count++;
		}
		/* check if all are ready then cancel timer, and start game  */
		if (count == VECTOR_LENGTH(queue->entries)) {
			timer->delete(arena->begin_timer,bg->begin_timer);
			arena->begin_timer = INVALID_TIMER;
			bg->begin(arena);
		}
	}
}

void bg_queue_player_cleanup(struct map_session_data *sd) {
	nullpo_retv(sd);
	if ( sd->bg_queue.client_has_bg_data ) {
		if( sd->bg_queue.arena )
			clif->bgqueue_notice_delete(sd,BGQND_CLOSEWINDOW,sd->bg_queue.arena->name);
		else
			clif->bgqueue_notice_delete(sd,BGQND_FAIL_NOT_QUEUING,bg->arena[0]->name);
	}
	if( sd->bg_queue.arena )
		script->queue_remove(sd->bg_queue.arena->queue_id,sd->status.account_id);
	sd->bg_queue.arena = NULL;
	sd->bg_queue.ready = 0;
	sd->bg_queue.client_has_bg_data = 0;
	sd->bg_queue.type = 0;
}
void bg_match_over(struct bg_arena *arena, bool canceled) {
	struct script_queue *queue = script->queue(arena->queue_id);
	int i;

	nullpo_retv(arena);
	if( !arena->ongoing )
		return;
	arena->ongoing = false;

	for (i = 0; i < VECTOR_LENGTH(queue->entries); i++) {
		struct map_session_data *sd = map->id2sd(VECTOR_INDEX(queue->entries, i));

		if (sd == NULL)
			continue;

		if (sd->bg_queue.arena) {
			bg->team_leave(sd, 0);
			bg->queue_pc_cleanup(sd);
		}
		if (canceled)
			clif->messagecolor_self(sd->fd, COLOR_RED, "BG Match Canceled: not enough players");
		else
			pc_setglobalreg(sd, script->add_str(arena->delay_var), (unsigned int)time(NULL));
	}

	arena->begin_timer = INVALID_TIMER;
	arena->fillup_timer = INVALID_TIMER;
	/* reset queue */
	script->queue_clear(arena->queue_id);
}
void bg_begin(struct bg_arena *arena) {
	struct script_queue *queue = script->queue(arena->queue_id);
	int i, count = 0;

	nullpo_retv(arena);
	for (i = 0; i < VECTOR_LENGTH(queue->entries); i++) {
		struct map_session_data *sd = map->id2sd(VECTOR_INDEX(queue->entries, i));

		if (sd == NULL)
			continue;

		if (sd->bg_queue.ready == 1)
			count++;
		else
			bg->queue_pc_cleanup(sd);
	}
	/* TODO/FIXME? I *think* it should check what kind of queue the player used, then check if his party/guild
	 * (his team) still meet the join criteria (sort of what bg->can_queue does)
	 */

	if( count < arena->min_players ) {
		bg->match_over(arena,true);
	} else {
		arena->ongoing = true;

		if( bg->afk_timer_id == INVALID_TIMER && bg->mafksec > 0 )
			bg->afk_timer_id = timer->add(timer->gettick()+10000,bg->afk_timer,0,0);

		/* TODO: make this a arena-independent var? or just .@? */
		mapreg->setreg(script->add_str("$@bg_queue_id"),arena->queue_id);
		mapreg->setregstr(script->add_str("$@bg_delay_var$"),bg->gdelay_var);

		count = 0;
		for (i = 0; i < VECTOR_LENGTH(queue->entries); i++) {
			struct map_session_data *sd = map->id2sd(VECTOR_INDEX(queue->entries, i));

			if (sd == NULL || sd->bg_queue.ready != 1)
				continue;

			mapreg->setreg(reference_uid(script->add_str("$@bg_member"), count), sd->status.account_id);
			mapreg->setreg(reference_uid(script->add_str("$@bg_member_group"), count),
			               sd->bg_queue.type == BGQT_GUILD ? sd->status.guild_id :
			               sd->bg_queue.type == BGQT_PARTY ? sd->status.party_id :
			               0
			               );
			mapreg->setreg(reference_uid(script->add_str("$@bg_member_type"), count),
			               sd->bg_queue.type == BGQT_GUILD ? 1 :
			               sd->bg_queue.type == BGQT_PARTY ? 2 :
			               0
			               );
			count++;
		}
		mapreg->setreg(script->add_str("$@bg_member_size"),count);

		npc->event_do(arena->npc_event);
	}
}
int bg_begin_timer(int tid, int64 tick, int id, intptr_t data) {
	bg->begin(bg->arena[id]);
	bg->arena[id]->begin_timer = INVALID_TIMER;
	return 0;
}

int bg_afk_timer(int tid, int64 tick, int id, intptr_t data) {
	struct s_mapiterator* iter;
	struct map_session_data* sd;
	int count = 0;

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if( !sd->bg_queue.arena || !sd->bg_id )
			continue;
		if( DIFF_TICK(sockt->last_tick, sd->idletime) > bg->mafksec )
			bg->team_leave(sd,BGTL_AFK);
		count++;
	}
	mapit->free(iter);

	if( count )
		bg->afk_timer_id = timer->add(timer->gettick()+10000,bg->afk_timer,0,0);
	else
		bg->afk_timer_id = INVALID_TIMER;
	return 0;
}

void bg_queue_pregame(struct bg_arena *arena) {
	struct script_queue *queue;
	int i;
	nullpo_retv(arena);

	queue = script->queue(arena->queue_id);
	for (i = 0; i < VECTOR_LENGTH(queue->entries); i++) {
		struct map_session_data *sd = map->id2sd(VECTOR_INDEX(queue->entries, i));

		if (sd != NULL) {
			clif->bgqueue_battlebegins(sd,arena->id,SELF);
		}
	}
	arena->begin_timer = timer->add( timer->gettick() + (arena->pregame_duration*1000), bg->begin_timer, arena->id, 0 );
}
int bg_fillup_timer(int tid, int64 tick, int id, intptr_t data) {
	bg->queue_pregame(bg->arena[id]);
	bg->arena[id]->fillup_timer = INVALID_TIMER;
	return 0;
}

void bg_queue_check(struct bg_arena *arena) {
	int count;
	struct script_queue *queue;
	nullpo_retv(arena);

	queue = script->queue(arena->queue_id);
	count = VECTOR_LENGTH(queue->entries);
	if( count == arena->max_players ) {
		if( arena->fillup_timer != INVALID_TIMER ) {
			timer->delete(arena->fillup_timer,bg->fillup_timer);
			arena->fillup_timer = INVALID_TIMER;
		}
		bg->queue_pregame(arena);
	} else if( count >= arena->min_players && arena->fillup_timer == INVALID_TIMER ) {
		arena->fillup_timer = timer->add( timer->gettick() + (arena->fillup_duration*1000), bg->fillup_timer, arena->id, 0 );
	}
}
void bg_queue_add(struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type) {
	enum BATTLEGROUNDS_QUEUE_ACK result = bg->can_queue(sd,arena,type);
	struct script_queue *queue = NULL;
	int i, count = 0;

	nullpo_retv(sd);
	nullpo_retv(arena);
	if( arena->begin_timer != INVALID_TIMER || arena->ongoing ) {
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

	if( !(queue = script->queue(arena->queue_id)) || (VECTOR_LENGTH(queue->entries)+count) > arena->max_players ) {
		clif->bgqueue_ack(sd,BGQA_FAIL_PPL_OVERAMOUNT,arena->id);
		return;
	}

	switch( type ) {
		case BGQT_INDIVIDUAL:
			sd->bg_queue.type = type;
			sd->bg_queue.arena = arena;
			sd->bg_queue.ready = 0;
			script->queue_add(arena->queue_id,sd->status.account_id);
			clif->bgqueue_joined(sd, VECTOR_LENGTH(queue->entries));
			clif->bgqueue_update_info(sd,arena->id, VECTOR_LENGTH(queue->entries));
			break;
		case BGQT_PARTY: {
			struct party_data *p = party->search(sd->status.party_id);
			for( i = 0; i < MAX_PARTY; i++ ) {
				if( !p->data[i].sd || p->data[i].sd->bg_queue.arena != NULL ) continue;
				p->data[i].sd->bg_queue.type = type;
				p->data[i].sd->bg_queue.arena = arena;
				p->data[i].sd->bg_queue.ready = 0;
				script->queue_add(arena->queue_id,p->data[i].sd->status.account_id);
				clif->bgqueue_joined(p->data[i].sd, VECTOR_LENGTH(queue->entries));
				clif->bgqueue_update_info(p->data[i].sd,arena->id, VECTOR_LENGTH(queue->entries));
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
				clif->bgqueue_joined(sd->guild->member[i].sd, VECTOR_LENGTH(queue->entries));
				clif->bgqueue_update_info(sd->guild->member[i].sd,arena->id, VECTOR_LENGTH(queue->entries));
			}
			break;
	}
	clif->bgqueue_ack(sd,BGQA_SUCCESS,arena->id);
	bg->queue_check(arena);
}
enum BATTLEGROUNDS_QUEUE_ACK bg_canqueue(struct map_session_data *sd, struct bg_arena *arena, enum bg_queue_types type) {
	int tick;
	unsigned int tsec;

	nullpo_retr(BGQA_FAIL_TYPE_INVALID, sd);
	nullpo_retr(BGQA_FAIL_TYPE_INVALID, arena);
	if( !(arena->allowed_types & type) )
		return BGQA_FAIL_TYPE_INVALID;

	if ( sd->status.base_level > arena->max_level || sd->status.base_level < arena->min_level )
		return BGQA_FAIL_LEVEL_INCORRECT;

	if ( !(sd->class_&JOBL_2) ) /* TODO: maybe make this a per-arena setting, so users may make custom arenas like baby-only,whatever. */
		return BGQA_FAIL_CLASS_INVALID;

	tsec = (unsigned int)time(NULL);

	if ( ( tick = pc_readglobalreg(sd, script->add_str(bg->gdelay_var)) ) && tsec < tick ) {
		char response[100];
		if( (tick-tsec) > 60 )
			sprintf(response, "You are a deserter! Wait %u minute(s) before you can apply again", (tick - tsec) / 60);
		else
			sprintf(response, "You are a deserter! Wait %u seconds before you can apply again", (tick - tsec));
		clif->messagecolor_self(sd->fd, COLOR_RED, response);
		return BGQA_FAIL_DESERTER;
	}

	if ( ( tick = pc_readglobalreg(sd, script->add_str(arena->delay_var)) ) && tsec < tick ) {
		char response[100];
		if( (tick-tsec) > 60 )
			sprintf(response, "You can't reapply to this arena so fast. Apply to the different arena or wait %u minute(s)", (tick - tsec) / 60);
		else
			sprintf(response, "You can't reapply to this arena so fast. Apply to the different arena or wait %u seconds", (tick - tsec));
		clif->messagecolor_self(sd->fd, COLOR_RED, response);
		return BGQA_FAIL_COOLDOWN;
	}

	if( sd->bg_queue.arena != NULL )
		return BGQA_DUPLICATE_REQUEST;

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
					char response[117];
					if( count != sd->guild->connect_member && sd->guild->connect_member >= arena->min_team_players )
						sprintf(response, "Can't apply: not enough members in your team/guild that have not entered the queue in individual mode, minimum is %d",arena->min_team_players);
					else
						sprintf(response, "Can't apply: not enough members in your team/guild, minimum is %d",arena->min_team_players);
					clif->messagecolor_self(sd->fd, COLOR_RED, response);
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
						char response[117];
						if( count != p->party.count && p->party.count >= arena->min_team_players )
							sprintf(response, "Can't apply: not enough members in your team/party that have not entered the queue in individual mode, minimum is %d",arena->min_team_players);
						else
							sprintf(response, "Can't apply: not enough members in your team/party, minimum is %d",arena->min_team_players);
						clif->messagecolor_self(sd->fd, COLOR_RED, response);
						return BGQA_FAIL_TEAM_COUNT;
					}
				} else
					return BGQA_NOT_PARTY_GUILD_LEADER;
			}
			break;
		case BGQT_INDIVIDUAL:/* already did */
			break;
		default:
			ShowDebug("bg_canqueue: unknown/unsupported type %u\n", type);
			return BGQA_DUPLICATE_REQUEST;
	}
	return BGQA_SUCCESS;
}
void do_init_battleground(bool minimal) {
	if (minimal)
		return;

	bg->team_db = idb_alloc(DB_OPT_RELEASE_DATA);
	timer->add_func_list(bg->send_xy_timer, "bg_send_xy_timer");
	timer->add_interval(timer->gettick() + battle_config.bg_update_interval, bg->send_xy_timer, 0, 0, battle_config.bg_update_interval);
	bg->config_read();
}

/**
 * @see DBApply
 */
int bg_team_db_final(union DBKey key, struct DBData *data, va_list ap)
{
	struct battleground_data* bgd = DB->data2ptr(data);

	HPM->data_store_destroy(&bgd->hdata);

	return 0;
}

void do_final_battleground(void)
{
	bg->team_db->destroy(bg->team_db,bg->team_db_final);

	if (bg->arena) {
		int i;
		for (i = 0; i < bg->arenas; i++) {
			if (bg->arena[i])
				aFree(bg->arena[i]);
		}
		aFree(bg->arena);
	}
}
void battleground_defaults(void) {
	bg = &bg_s;

	bg->queue_on = false;

	bg->mafksec = 0;
	bg->afk_timer_id = INVALID_TIMER;
	bg->arena = NULL;
	bg->arenas = 0;
	/* */
	bg->team_db = NULL;
	bg->team_counter = 0;
	/* */
	bg->init = do_init_battleground;
	bg->final = do_final_battleground;
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
	bg->team_search = bg_team_search;
	bg->getavailablesd = bg_getavailablesd;
	bg->team_delete = bg_team_delete;
	bg->team_warp = bg_team_warp;
	bg->send_dot_remove = bg_send_dot_remove;
	bg->team_join = bg_team_join;
	bg->team_leave = bg_team_leave;
	bg->member_respawn = bg_member_respawn;
	bg->create = bg_create;
	bg->team_get_id = bg_team_get_id;
	bg->send_message = bg_send_message;
	bg->send_xy_timer_sub = bg_send_xy_timer_sub;
	bg->send_xy_timer = bg_send_xy_timer;
	bg->afk_timer = bg_afk_timer;
	bg->team_db_final = bg_team_db_final;
	/* */
	bg->str2teamtype = bg_str2teamtype;
	/* */
	bg->config_read = bg_config_read;
}
