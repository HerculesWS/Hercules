/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2018  Hercules Dev Team
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

#include "config/core.h" // NPC_SECURE_TIMEOUT_INPUT, NPC_SECURE_TIMEOUT_MENU, NPC_SECURE_TIMEOUT_NEXT, SECURE_NPCTIMEOUT, SECURE_NPCTIMEOUT_INTERVAL
#include "npc.h"

#include "map/battle.h"
#include "map/chat.h"
#include "map/clan.h"
#include "map/clif.h"
#include "map/guild.h"
#include "map/instance.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/quest.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/unit.h"
#include "map/achievement.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct npc_interface npc_s;
struct npc_interface *npc;


//For holding the view data of npc classes. [Skotlex]
static struct view_data npc_viewdb[MAX_NPC_CLASS];
static struct view_data npc_viewdb2[MAX_NPC_CLASS2_END-MAX_NPC_CLASS2_START];

/* for speedup */
static unsigned int npc_market_qty[MAX_INVENTORY];

static struct script_event_s {
	//Holds pointers to the commonly executed scripts for speedup. [Skotlex]
	struct event_data *event[UCHAR_MAX];
	const char *event_name[UCHAR_MAX];
	uint8 event_count;
} script_event[NPCE_MAX];

/**
 * Returns the viewdata for normal npc classes.
 * @param class_ The NPC class ID.
 * @return The viewdata, or NULL if the ID is invalid.
 */
static struct view_data *npc_get_viewdata(int class_)
{
	if (class_ == INVISIBLE_CLASS)
		return &npc_viewdb[0];
	if (npc->db_checkid(class_)) {
		if (class_ < MAX_NPC_CLASS) {
			return &npc_viewdb[class_];
		} else if (class_ >= MAX_NPC_CLASS2_START && class_ < MAX_NPC_CLASS2_END) {
			return &npc_viewdb2[class_-MAX_NPC_CLASS2_START];
		}
	}
	return NULL;
}

/**
 * Checks if a given id is a valid npc id.
 *
 * Since new npcs are added all the time, the max valid value is the one before the first mob (Scorpion = 1001)
 *
 * @param id The NPC ID to validate.
 * @return Whether the value is a valid ID.
 */
static bool npc_db_checkid(int id)
{
	if (id >= WARP_CLASS && id <= 125) // First subrange
		return true;
	if (id == HIDDEN_WARP_CLASS || id == INVISIBLE_CLASS) // Special IDs not included in the valid ranges
		return true;
	if (id > 400 && id < MAX_NPC_CLASS) // Second subrange
		return true;
	if (id >= MAX_NPC_CLASS2_START && id < MAX_NPC_CLASS2_END) // Second range
		return true;
	if (pc->db_checkid(id))
		return true;
	// Anything else is invalid
	return false;
}

/// Returns a new npc id that isn't being used in id_db.
/// Fatal error if nothing is available.
static int npc_get_new_npc_id(void)
{
	if (npc->npc_id >= START_NPC_NUM && !map->blid_exists(npc->npc_id))
		return npc->npc_id++;// available
	else {// find next id
		int base_id = npc->npc_id;
		while (base_id != ++npc->npc_id) {
			if (npc->npc_id < START_NPC_NUM)
				npc->npc_id = START_NPC_NUM;
			if (!map->blid_exists(npc->npc_id))
				return npc->npc_id++;// available
		}
		// full loop, nothing available
		ShowFatalError("npc_get_new_npc_id: All ids are taken. Exiting...");
		exit(1);
	}
}

static int npc_isnear_sub(struct block_list *bl, va_list args)
{
	const struct npc_data *nd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_NPC);
	nd = BL_UCCAST(BL_NPC, bl);

	if( nd->option & (OPTION_HIDE|OPTION_INVISIBLE) )
		return 0;

	if( battle_config.vendchat_near_hiddennpc && ( nd->class_ == FAKE_NPC || nd->class_ == HIDDEN_WARP_CLASS ) )
		return 0;

	return 1;
}

static bool npc_isnear(struct block_list *bl)
{
	if( battle_config.min_npc_vendchat_distance > 0
	 && map->foreachinrange(npc->isnear_sub,bl, battle_config.min_npc_vendchat_distance, BL_NPC) )
		return true;

	return false;
}

static int npc_ontouch_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	nullpo_retr(1, nd);
	if( nd->touching_id )
		return 0; // Attached a player already. Can't trigger on anyone else.

	if( pc_ishiding(sd) )
		return 1; // Can't trigger 'OnTouch_'. try 'OnTouch' later.

	snprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script->config.ontouch_name);
	return npc->event(sd,name,1);
}

static int npc_ontouch2_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	nullpo_retr(1, sd);
	nullpo_retr(1, nd);
	if (sd->areanpc_id == nd->bl.id)
		return 0;

	snprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script->config.ontouch2_name);
	return npc->event(sd, name, 2);
}

static int npc_onuntouch_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	nullpo_ret(sd);
	nullpo_ret(nd);
	if (sd->areanpc_id != nd->bl.id)
		return 0;

	snprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script->config.onuntouch_name);
	return npc->event(sd, name, 2);
}

/*==========================================
 * Sub-function of npc_enable, runs OnTouch event when enabled
 *------------------------------------------*/
static int npc_enable_sub(struct block_list *bl, va_list ap)
{
	struct npc_data *nd;

	nullpo_ret(bl);
	nullpo_ret(nd=va_arg(ap,struct npc_data *));

	if (bl->type == BL_PC) {
		struct map_session_data *sd = BL_UCAST(BL_PC, bl);

		if (nd->option&OPTION_INVISIBLE)
			return 1;

		if( npc->ontouch_event(sd,nd) > 0 && npc->ontouch2_event(sd,nd) > 0 )
		{ // failed to run OnTouch event, so just click the npc
			if (sd->npc_id != 0)
				return 0;

			pc_stop_walking(sd, STOPWALKING_FLAG_FIXPOS);
			npc->click(sd,nd);
		}
	}
	return 0;
}

/*==========================================
 * Disable / Enable NPC
 *------------------------------------------*/
static int npc_enable(const char *name, int flag)
{
	struct npc_data* nd = npc->name2id(name);

	if ( nd == NULL ) {
		ShowError("npc_enable: Attempted to %s a non-existing NPC '%s' (flag=%d).\n", (flag&3) ? "show" : "hide", name, flag);
		return 0;
	}

	if (flag&1) {
		nd->option&=~OPTION_INVISIBLE;
		clif->spawn(&nd->bl);
	} else if (flag&2)
		nd->option&=~OPTION_HIDE;
	else if (flag&4)
		nd->option|= OPTION_HIDE;
	else { //Can't change the view_data to invisible class because the view_data for all npcs is shared! [Skotlex]
		nd->option|= OPTION_INVISIBLE;
		clif->clearunit_area(&nd->bl,CLR_OUTSIGHT);  // Hack to trick maya purple card [Xazax]
	}

	if (nd->class_ == WARP_CLASS || nd->class_ == FLAG_CLASS) { //Client won't display option changes for these classes [Toms]
		if (nd->option&(OPTION_HIDE|OPTION_INVISIBLE))
			clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);
		else
			clif->spawn(&nd->bl);
	} else
		clif->changeoption(&nd->bl);

	if( flag&3 && (nd->u.scr.xs >= 0 || nd->u.scr.ys >= 0) ) //check if player standing on a OnTouchArea
		map->foreachinarea( npc->enable_sub, nd->bl.m, nd->bl.x-nd->u.scr.xs, nd->bl.y-nd->u.scr.ys, nd->bl.x+nd->u.scr.xs, nd->bl.y+nd->u.scr.ys, BL_PC, nd );

	return 0;
}

/*==========================================
 * NPC lookup (get npc_data through npcname)
 *------------------------------------------*/
static struct npc_data *npc_name2id(const char *name)
{
	return strdb_get(npc->name_db, name);
}
/**
 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
 **/
/**
 * Timer to check for idle time and timeout the dialog if necessary
 **/
static int npc_rr_secure_timeout_timer(int tid, int64 tick, int id, intptr_t data)
{
#ifdef SECURE_NPCTIMEOUT
	struct map_session_data* sd = NULL;
	unsigned int timeout = NPC_SECURE_TIMEOUT_NEXT;
	if( (sd = map->id2sd(id)) == NULL || !sd->npc_id ) {
		if( sd ) sd->npc_idle_timer = INVALID_TIMER;
		return 0;//Not logged in anymore OR no longer attached to a npc
	}

	switch( sd->npc_idle_type ) {
		case NPCT_INPUT:
			timeout = NPC_SECURE_TIMEOUT_INPUT;
			break;
		case NPCT_MENU:
			timeout = NPC_SECURE_TIMEOUT_MENU;
			break;
		//case NPCT_WAIT: var starts with this value
	}

	if( DIFF_TICK(tick,sd->npc_idle_tick) > (timeout*1000) ) {
		/**
		 * If we still have the NPC script attached, tell it to stop.
		 **/
		if( sd->st )
			sd->st->state = END;
		sd->state.menu_or_input = 0;
		sd->npc_menu = 0;
		/**
		 * This guy's been idle for longer than allowed, close him.
		 **/
		clif->scriptclose(sd,sd->npc_id);
		sd->npc_idle_timer = INVALID_TIMER;
		/**
		* We will end the script ourselves, client will request to end it again if it have dialog,
		* however it will be ignored, workaround for client stuck if NPC have no dialog. [hemagx]
		**/
		sd->state.dialog = 0;
		npc->scriptcont(sd, sd->npc_id, true);
	} else //Create a new instance of ourselves to continue
		sd->npc_idle_timer = timer->add(timer->gettick() + (SECURE_NPCTIMEOUT_INTERVAL*1000),npc->secure_timeout_timer,sd->bl.id,0);
#endif
	return 0;
}

/*==========================================
 * Dequeue event and add timer for execution (100ms)
 *------------------------------------------*/
static int npc_event_dequeue(struct map_session_data *sd)
{
	nullpo_ret(sd);

	if(sd->npc_id) { //Current script is aborted.
		if(sd->state.using_fake_npc){
			clif->clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
			sd->state.using_fake_npc = 0;
		}
		if (sd->st) {
			script->free_state(sd->st);
			sd->st = NULL;
		}
		sd->npc_id = 0;
	}

	if (!sd->eventqueue[0][0])
		return 0; //Nothing to dequeue

	if (!pc->addeventtimer(sd,100,sd->eventqueue[0])) { //Failed to dequeue, couldn't set a timer.
		ShowWarning("npc_event_dequeue: event timer is full !\n");
		return 0;
	}
	//Event dequeued successfully, shift other elements.
	memmove(sd->eventqueue[0], sd->eventqueue[1], (MAX_EVENTQUEUE-1)*sizeof(sd->eventqueue[0]));
	sd->eventqueue[MAX_EVENTQUEUE-1][0]=0;
	return 1;
}

/**
 * @see DBCreateData
 */
static struct DBData npc_event_export_create(union DBKey key, va_list args)
{
	struct linkdb_node** head_ptr;
	CREATE(head_ptr, struct linkdb_node*, 1);
	*head_ptr = NULL;
	return DB->ptr2data(head_ptr);
}

/*==========================================
 * exports a npc event label
 * called from npc_parse_script
 *------------------------------------------*/
static int npc_event_export(struct npc_data *nd, int i)
{
	char* lname;
	int pos;
	nullpo_ret(nd);
	Assert_ret(i >= 0 && i < nd->u.scr.label_list_num);
	lname = nd->u.scr.label_list[i].name;
	pos = nd->u.scr.label_list[i].pos;
	if ((lname[0] == 'O' || lname[0] == 'o') && (lname[1] == 'N' || lname[1] == 'n')) {
		struct event_data *ev;
		struct linkdb_node **label_linkdb = NULL;
		char buf[EVENT_NAME_LENGTH];
		snprintf(buf, ARRAYLENGTH(buf), "%s::%s", nd->exname, lname);
		if (strdb_exists(npc->ev_db, buf)) // There was already another event of the same name?
			return 1;
		// generate the data and insert it
		CREATE(ev, struct event_data, 1);
		ev->nd = nd;
		ev->pos = pos;
		strdb_put(npc->ev_db, buf, ev);
		label_linkdb = strdb_ensure(npc->ev_label_db, lname, npc->event_export_create);
		linkdb_insert(label_linkdb, nd, ev);
	}
	return 0;
}

static int npc_event_sub(struct map_session_data *sd, struct event_data *ev, const char *eventname); //[Lance]

/**
 * Exec name (NPC events) on player or global
 * Do on all NPC when called with foreach
 */
static void npc_event_doall_sub(void *key, void *data, va_list ap)
{
	struct event_data* ev = data;
	int* c;
	const char* name;
	int rid;

	nullpo_retv(c = va_arg(ap, int*));
	nullpo_retv(name = va_arg(ap, const char*));
	rid = va_arg(ap, int);

	if (ev /* && !ev->nd->src_id */) // Do not run on duplicates. [Paradox924X]
	{
		if(rid) { // a player may only have 1 script running at the same time
			char buf[EVENT_NAME_LENGTH];
			snprintf(buf, ARRAYLENGTH(buf), "%s::%s", ev->nd->exname, name);
			npc->event_sub(map->id2sd(rid), ev, buf);
		}
		else {
			script->run_npc(ev->nd->u.scr.script, ev->pos, rid, ev->nd->bl.id);
		}
		(*c)++;
	}
}

// runs the specified event (supports both single-npc and global events)
static int npc_event_do(const char *name)
{
	nullpo_ret(name);
	if( name[0] == ':' && name[1] == ':' ) {
		return npc->event_doall(name+2); // skip leading "::"
	}
	else {
		struct event_data *ev = strdb_get(npc->ev_db, name);
		if (ev) {
			script->run_npc(ev->nd->u.scr.script, ev->pos, 0, ev->nd->bl.id);
			return 1;
		}
	}
	return 0;
}

// runs the specified event, with a RID attached (global only)
static int npc_event_doall_id(const char *name, int rid)
{
	int c = 0;
	struct linkdb_node **label_linkdb = strdb_get(npc->ev_label_db, name);

	if (label_linkdb == NULL)
		return 0;

	linkdb_foreach(label_linkdb, npc->event_doall_sub, &c, name, rid);
	return c;
}

// runs the specified event (global only)
static int npc_event_doall(const char *name)
{
	return npc->event_doall_id(name, 0);
}

/*==========================================
 * Clock event execution
 * OnMinute/OnClock/OnHour/OnDay/OnDDHHMM
 *------------------------------------------*/
static int npc_event_do_clock(int tid, int64 tick, int id, intptr_t data)
{
	static struct tm ev_tm_b; // tracks previous execution time
	time_t clock;
	struct tm* t;
	char buf[64];
	int c = 0;

	clock = time(NULL);
	t = localtime(&clock);

	if (t->tm_min != ev_tm_b.tm_min ) {
		char* day;

		switch (t->tm_wday) {
			case 0: day = "Sun"; break;
			case 1: day = "Mon"; break;
			case 2: day = "Tue"; break;
			case 3: day = "Wed"; break;
			case 4: day = "Thu"; break;
			case 5: day = "Fri"; break;
			case 6: day = "Sat"; break;
			default:day = ""; break;
		}

		sprintf(buf,"OnMinute%02d",t->tm_min);
		c += npc->event_doall(buf);

		sprintf(buf,"OnClock%02d%02d",t->tm_hour,t->tm_min);
		c += npc->event_doall(buf);

		sprintf(buf,"On%s%02d%02d",day,t->tm_hour,t->tm_min);
		c += npc->event_doall(buf);
	}

	if (t->tm_hour != ev_tm_b.tm_hour) {
		sprintf(buf,"OnHour%02d",t->tm_hour);
		c += npc->event_doall(buf);
	}

	if (t->tm_mday != ev_tm_b.tm_mday) {
		sprintf(buf,"OnDay%02d%02d",t->tm_mon+1,t->tm_mday);
		c += npc->event_doall(buf);
	}

	memcpy(&ev_tm_b,t,sizeof(ev_tm_b));
	return c;
}

/**
 * OnInit event execution (the start of the event and watch)
 * @param reload Is the server reloading?
 **/
static void npc_event_do_oninit(bool reload)
{
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs."CL_CLL"\n", npc->event_doall("OnInit"));

	// This interval has already been added on startup
	if( !reload )
		timer->add_interval(timer->gettick()+100,npc->event_do_clock,0,0,1000);
}

/*==========================================
 * Incorporation of the label for the timer event
 * called from npc_parse_script
 *------------------------------------------*/
static int npc_timerevent_export(struct npc_data *nd, int i)
{
	int t = 0, len = 0;
	char *lname;
	int pos;
	nullpo_ret(nd);
	lname = nd->u.scr.label_list[i].name;
	pos = nd->u.scr.label_list[i].pos;
	if (sscanf(lname, "OnTimer%d%n", &t, &len) == 1 && len < NAME_LENGTH && lname[len] == '\0') {
		// Timer event
		struct npc_timerevent_list *te = nd->u.scr.timer_event;
		int j, k = nd->u.scr.timeramount;
		if (te == NULL)
			te = (struct npc_timerevent_list *)aMalloc(sizeof(struct npc_timerevent_list));
		else
			te = (struct npc_timerevent_list *)aRealloc( te, sizeof(struct npc_timerevent_list) * (k+1) );
		for (j = 0; j < k; j++) {
			if (te[j].timer > t) {
				memmove(te+j+1, te+j, sizeof(struct npc_timerevent_list)*(k-j));
				break;
			}
		}
		te[j].timer = t;
		te[j].pos = pos;
		nd->u.scr.timer_event = te;
		nd->u.scr.timeramount++;
	}
	return 0;
}

struct timer_event_data {
	int rid; //Attached player for this timer.
	int next; //timer index (starts with 0, then goes up to nd->u.scr.timeramount)
	int time; //holds total time elapsed for the script from when timer was started to when last time the event triggered.
};

/*==========================================
 * triger 'OnTimerXXXX' events
 *------------------------------------------*/
static int npc_timerevent(int tid, int64 tick, int id, intptr_t data)
{
	int old_rid, old_timer;
	int64 old_tick;
	struct npc_data *nd = map->id2nd(id);
	struct npc_timerevent_list *te;
	struct timer_event_data *ted = (struct timer_event_data*)data;
	struct map_session_data *sd=NULL;

	nullpo_ret(ted);

	if( nd == NULL ) {
		ShowError("npc_timerevent: NPC not found??\n");
		return 0;
	}

	if (ted->rid && (sd = map->id2sd(ted->rid)) == NULL) {
		ShowError("npc_timerevent: Attached player not found.\n");
		ers_free(npc->timer_event_ers, ted);
		return 0;
	}

	// These stuffs might need to be restored.
	old_rid = nd->u.scr.rid;
	old_tick = nd->u.scr.timertick;
	old_timer = nd->u.scr.timer;

	// Set the values of the timer
	nd->u.scr.rid = sd?sd->bl.id:0; //attached rid
	nd->u.scr.timertick = tick;     //current time tick
	nd->u.scr.timer = ted->time;    //total time from beginning to now

	// Locate the event
	te = nd->u.scr.timer_event + ted->next;

	// Arrange for the next event
	ted->next++;
	if( nd->u.scr.timeramount > ted->next )
	{
		int next = nd->u.scr.timer_event[ ted->next ].timer - nd->u.scr.timer_event[ ted->next - 1 ].timer;
		ted->time += next;
		if( sd )
			sd->npc_timer_id = timer->add(tick+next,npc->timerevent,id,(intptr_t)ted);
		else
			nd->u.scr.timerid = timer->add(tick+next,npc->timerevent,id,(intptr_t)ted);
	}
	else
	{
		if( sd )
			sd->npc_timer_id = INVALID_TIMER;
		else
			nd->u.scr.timerid = INVALID_TIMER;

		ers_free(npc->timer_event_ers, ted);
	}

	// Run the script
	script->run_npc(nd->u.scr.script,te->pos,nd->u.scr.rid,nd->bl.id);

	nd->u.scr.rid = old_rid; // Attached-rid should be restored anyway.
	if( sd )
	{ // Restore previous data, only if this timer is a player-attached one.
		nd->u.scr.timer = old_timer;
		nd->u.scr.timertick = old_tick;
	}

	return 0;
}
/*==========================================
 * Start/Resume NPC timer
 *------------------------------------------*/
static int npc_timerevent_start(struct npc_data *nd, int rid)
{
	int j;
	int64 tick = timer->gettick();
	struct map_session_data *sd = NULL; //Player to whom script is attached.

	nullpo_ret(nd);

	// Check if there is an OnTimer Event
	ARR_FIND( 0, nd->u.scr.timeramount, j, nd->u.scr.timer_event[j].timer > nd->u.scr.timer );

	if (nd->u.scr.rid > 0 && (sd = map->id2sd(nd->u.scr.rid)) == NULL) {
		// Failed to attach timer to this player.
		ShowError("npc_timerevent_start: Attached player not found!\n");
		return 1;
	}
	// Check if timer is already started.
	if( sd ) {
		if( sd->npc_timer_id != INVALID_TIMER )
			return 0;
	} else if( nd->u.scr.timerid != INVALID_TIMER || nd->u.scr.timertick )
		return 0;

	if (j < nd->u.scr.timeramount) {
		int next;
		struct timer_event_data *ted;
		// Arrange for the next event
		ted = ers_alloc(npc->timer_event_ers, struct timer_event_data);
		ted->next = j; // Set event index
		ted->time = nd->u.scr.timer_event[j].timer;
		next = nd->u.scr.timer_event[j].timer - nd->u.scr.timer;
		if( sd )
		{
			ted->rid = sd->bl.id; // Attach only the player if attachplayerrid was used.
			sd->npc_timer_id = timer->add(tick+next,npc->timerevent,nd->bl.id,(intptr_t)ted);
		}
		else
		{
			ted->rid = 0;
			nd->u.scr.timertick = tick; // Set when timer is started
			nd->u.scr.timerid = timer->add(tick+next,npc->timerevent,nd->bl.id,(intptr_t)ted);
		}

	} else if (!sd) {
		nd->u.scr.timertick = tick;

	}

	return 0;
}
/*==========================================
 * Stop NPC timer
 *------------------------------------------*/
static int npc_timerevent_stop(struct npc_data *nd)
{
	struct map_session_data *sd = NULL;
	int *tid;

	nullpo_ret(nd);

	if (nd->u.scr.rid && (sd = map->id2sd(nd->u.scr.rid)) == NULL) {
		ShowError("npc_timerevent_stop: Attached player not found!\n");
		return 1;
	}
	tid = sd?&sd->npc_timer_id:&nd->u.scr.timerid;
	if( *tid == INVALID_TIMER && (sd || !nd->u.scr.timertick) ) // Nothing to stop
		return 0;

	// Delete timer
	if (*tid != INVALID_TIMER) {
		const struct TimerData *td = timer->get(*tid);
		if (td && td->data)
			ers_free(npc->timer_event_ers, (void*)td->data);
		timer->delete(*tid,npc->timerevent);
		*tid = INVALID_TIMER;
	}

	if (!sd && nd->u.scr.timertick) {
		nd->u.scr.timer += DIFF_TICK32(timer->gettick(),nd->u.scr.timertick); // Set 'timer' to the time that has passed since the beginning of the timers
		nd->u.scr.timertick = 0; // Set 'tick' to zero so that we know it's off.
	}

	return 0;
}
/*==========================================
 * Aborts a running NPC timer that is attached to a player.
 *------------------------------------------*/
static void npc_timerevent_quit(struct map_session_data *sd)
{
	const struct TimerData *td;
	struct npc_data* nd;
	struct timer_event_data *ted;

	nullpo_retv(sd);
	// Check timer existence
	if( sd->npc_timer_id == INVALID_TIMER )
		return;
	if( !(td = timer->get(sd->npc_timer_id)) )
	{
		sd->npc_timer_id = INVALID_TIMER;
		return;
	}

	// Delete timer
	nd = map->id2nd(td->id);
	ted = (struct timer_event_data*)td->data;
	timer->delete(sd->npc_timer_id, npc->timerevent);
	sd->npc_timer_id = INVALID_TIMER;

	// Execute OnTimerQuit
	if (nd != NULL) {
		char buf[EVENT_NAME_LENGTH];
		struct event_data *ev;

		snprintf(buf, ARRAYLENGTH(buf), "%s::OnTimerQuit", nd->exname);
		ev = (struct event_data*)strdb_get(npc->ev_db, buf);
		if( ev && ev->nd != nd )
		{
			ShowWarning("npc_timerevent_quit: Unable to execute \"OnTimerQuit\", two NPCs have the same event name [%s]!\n",buf);
			ev = NULL;
		}
		if( ev )
		{
			int old_rid,old_timer;
			int64 old_tick;
			nullpo_retv(ted);

			//Set timer related info.
			old_rid = (nd->u.scr.rid == sd->bl.id ? 0 : nd->u.scr.rid); // Detach rid if the last attached player logged off.
			old_tick = nd->u.scr.timertick;
			old_timer = nd->u.scr.timer;

			nd->u.scr.rid = sd->bl.id;
			nd->u.scr.timertick = timer->gettick();
			nd->u.scr.timer = ted->time;

			//Execute label
			script->run_npc(nd->u.scr.script,ev->pos,sd->bl.id,nd->bl.id);

			//Restore previous data.
			nd->u.scr.rid = old_rid;
			nd->u.scr.timer = old_timer;
			nd->u.scr.timertick = old_tick;
		}
	}
	ers_free(npc->timer_event_ers, ted);
}

/*==========================================
 * Get the tick value of an NPC timer
 * If it's stopped, return stopped time
 *------------------------------------------*/
static int64 npc_gettimerevent_tick(struct npc_data *nd)
{
	int64 tick;
	nullpo_ret(nd);

	// TODO: Get player attached timer's tick. Now we can just get it by using 'getnpctimer' inside OnTimer event.

	tick = nd->u.scr.timer; // The last time it's active(start, stop or event trigger)
	if( nd->u.scr.timertick ) // It's a running timer
		tick += DIFF_TICK(timer->gettick(), nd->u.scr.timertick);

	return tick;
}

/*==========================================
 * Set tick for running and stopped timer
 *------------------------------------------*/
static int npc_settimerevent_tick(struct npc_data *nd, int newtimer)
{
	bool flag;
	int old_rid;
	//struct map_session_data *sd = NULL;

	nullpo_ret(nd);

	// TODO: Set player attached timer's tick.

	old_rid = nd->u.scr.rid;
	nd->u.scr.rid = 0;

	// Check if timer is started
	flag = (nd->u.scr.timerid != INVALID_TIMER || nd->u.scr.timertick);

	if( flag ) npc->timerevent_stop(nd);
	nd->u.scr.timer = newtimer;
	if( flag ) npc->timerevent_start(nd, -1);

	nd->u.scr.rid = old_rid;
	return 0;
}

static int npc_event_sub(struct map_session_data *sd, struct event_data *ev, const char *eventname)
{
	nullpo_retr(2, sd);
	nullpo_retr(2, eventname);
	if ( sd->npc_id != 0 )
	{
		//Enqueue the event trigger.
		int i;
		ARR_FIND( 0, MAX_EVENTQUEUE, i, sd->eventqueue[i][0] == '\0' );
		if( i < MAX_EVENTQUEUE )
		{
			safestrncpy(sd->eventqueue[i],eventname,EVENT_NAME_LENGTH); //Event enqueued.
			return 0;
		}

		ShowWarning("npc_event: player's event queue is full, can't add event '%s' !\n", eventname);
		return 1;
	}
	if( ev->nd->option&OPTION_INVISIBLE )
	{
		//Disabled npc, shouldn't trigger event.
		npc->event_dequeue(sd);
		return 2;
	}
	script->run_npc(ev->nd->u.scr.script,ev->pos,sd->bl.id,ev->nd->bl.id);
	return 0;
}

/*==========================================
 * NPC processing event type
 *------------------------------------------*/
static int npc_event(struct map_session_data *sd, const char *eventname, int ontouch)
{
	struct event_data* ev = (struct event_data*)strdb_get(npc->ev_db, eventname);
	struct npc_data *nd;

	nullpo_ret(sd);

	if( ev == NULL || (nd = ev->nd) == NULL ) {
		if( !ontouch )
			ShowError("npc_event: event not found [%s]\n", eventname);
		return ontouch;
	}

	switch(ontouch) {
		case 1:
			nd->touching_id = sd->bl.id;
			sd->touching_id = nd->bl.id;
			break;
		case 2:
			sd->areanpc_id = nd->bl.id;
			break;
	}

	return npc->event_sub(sd,ev,eventname);
}

/*==========================================
 * Sub chk then execute area event type
 *------------------------------------------*/
static int npc_touch_areanpc_sub(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int pc_id;
	char *name;

	nullpo_ret(bl);
	nullpo_ret((sd = map->id2sd(bl->id)));

	pc_id = va_arg(ap,int);
	name = va_arg(ap,char*);

	if( sd->state.warping )
		return 0;
	if( pc_ishiding(sd) )
		return 0;
	if( pc_id == sd->bl.id )
		return 0;

	npc->event(sd,name,1);

	return 1;
}

/*==========================================
 * Chk if sd is still touching his assigned npc.
 * If not, it unsets it and searches for another player in range.
 *------------------------------------------*/
static int npc_touchnext_areanpc(struct map_session_data *sd, bool leavemap)
{
	struct npc_data *nd;
	short xs, ys;

	nullpo_retr(1, sd);
	nd = map->id2nd(sd->touching_id);
	if( !nd || nd->touching_id != sd->bl.id )
		return 1;

	xs = nd->u.scr.xs;
	ys = nd->u.scr.ys;

	if( sd->bl.m != nd->bl.m ||
		sd->bl.x < nd->bl.x - xs || sd->bl.x > nd->bl.x + xs ||
		sd->bl.y < nd->bl.y - ys || sd->bl.y > nd->bl.y + ys ||
		pc_ishiding(sd) || leavemap )
	{
		char name[EVENT_NAME_LENGTH];

		nd->touching_id = sd->touching_id = 0;
		snprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script->config.ontouch_name);
		map->forcountinarea(npc->touch_areanpc_sub,nd->bl.m,nd->bl.x - xs,nd->bl.y - ys,nd->bl.x + xs,nd->bl.y + ys,1,BL_PC,sd->bl.id,name);
	}
	return 0;
}

/*==========================================
 * Exec OnTouch for player if in range of area event
 *------------------------------------------*/
static int npc_touch_areanpc(struct map_session_data *sd, int16 m, int16 x, int16 y)
{
	int xs,ys;
	int f = 1;
	int i;
	int j, found_warp = 0;

	nullpo_retr(1, sd);
	Assert_retr(1, m >= 0 && m < map->count);
#if 0 // Why not enqueue it? [Inkfish]
	if(sd->npc_id)
		return 1;
#endif // 0

	for(i=0;i<map->list[m].npc_num;i++) {
		if (map->list[m].npc[i]->option&OPTION_INVISIBLE) {
			f=0; // a npc was found, but it is disabled; don't print warning
			continue;
		}

		switch(map->list[m].npc[i]->subtype) {
		case WARP:
			xs=map->list[m].npc[i]->u.warp.xs;
			ys=map->list[m].npc[i]->u.warp.ys;
			break;
		case SCRIPT:
			xs=map->list[m].npc[i]->u.scr.xs;
			ys=map->list[m].npc[i]->u.scr.ys;
			break;
		default:
			continue;
		}
		if( x >= map->list[m].npc[i]->bl.x-xs && x <= map->list[m].npc[i]->bl.x+xs
		&&  y >= map->list[m].npc[i]->bl.y-ys && y <= map->list[m].npc[i]->bl.y+ys )
			break;
	}
	if( i == map->list[m].npc_num ) {
		if( f == 1 ) // no npc found
			ShowError("npc_touch_areanpc : stray NPC cell/NPC not found in the block on coordinates '%s',%d,%d\n", map->list[m].name, x, y);
		return 1;
	}
	switch(map->list[m].npc[i]->subtype) {
		case WARP:
			if( pc_ishiding(sd) || (sd->sc.count && sd->sc.data[SC_CAMOUFLAGE]) )
				break; // hidden chars cannot use warps
			pc->setpos(sd,map->list[m].npc[i]->u.warp.mapindex,map->list[m].npc[i]->u.warp.x,map->list[m].npc[i]->u.warp.y,CLR_OUTSIGHT);
			break;
		case SCRIPT:
			for (j = i; j < map->list[m].npc_num; j++) {
				if (map->list[m].npc[j]->subtype != WARP) {
					continue;
				}

				if ((sd->bl.x >= (map->list[m].npc[j]->bl.x - map->list[m].npc[j]->u.warp.xs)
				  && sd->bl.x <= (map->list[m].npc[j]->bl.x + map->list[m].npc[j]->u.warp.xs))
				 && (sd->bl.y >= (map->list[m].npc[j]->bl.y - map->list[m].npc[j]->u.warp.ys)
				  && sd->bl.y <= (map->list[m].npc[j]->bl.y + map->list[m].npc[j]->u.warp.ys))
				) {
					if( pc_ishiding(sd) || (sd->sc.count && sd->sc.data[SC_CAMOUFLAGE]) )
						break; // hidden chars cannot use warps
					pc->setpos(sd,map->list[m].npc[j]->u.warp.mapindex,map->list[m].npc[j]->u.warp.x,map->list[m].npc[j]->u.warp.y,CLR_OUTSIGHT);
					found_warp = 1;
					break;
				}
			}

			if (found_warp > 0) {
				break;
			}

			if( npc->ontouch_event(sd,map->list[m].npc[i]) > 0 && npc->ontouch2_event(sd,map->list[m].npc[i]) > 0 )
			{ // failed to run OnTouch event, so just click the npc
				struct unit_data *ud = unit->bl2ud(&sd->bl);
				if( ud && ud->walkpath.path_pos < ud->walkpath.path_len )
				{ // Since walktimer always == INVALID_TIMER at this time, we stop walking manually. [Inkfish]
					clif->fixpos(&sd->bl);
					ud->walkpath.path_pos = ud->walkpath.path_len;
				}
				sd->areanpc_id = map->list[m].npc[i]->bl.id;
				npc->click(sd,map->list[m].npc[i]);
			}
			break;
	}
	return 0;
}

/*==========================================
 * Exec OnUnTouch for player if out range of area event
 *------------------------------------------*/
static int npc_untouch_areanpc(struct map_session_data *sd, int16 m, int16 x, int16 y)
{
	struct npc_data *nd = NULL;
	nullpo_retr(1, sd);
	Assert_retr(1, m >= 0 && m < map->count);

	if (!sd->areanpc_id)
		return 0;

	nd = map->id2nd(sd->areanpc_id);
	if (nd == NULL) {
		sd->areanpc_id = 0;
		return 1;
	}

	npc->onuntouch_event(sd, nd);
	sd->areanpc_id = 0;
	return 0;
}

// OnTouch NPC or Warp for Mobs
// Return 1 if Warped
static int npc_touch_areanpc2(struct mob_data *md)
{
	int i, m, x, y, id;
	char eventname[EVENT_NAME_LENGTH];
	struct event_data* ev;
	int xs, ys;

	nullpo_ret(md);
	m = md->bl.m;
	x = md->bl.x;
	y = md->bl.y;

	for( i = 0; i < map->list[m].npc_num; i++ ) {
		if( map->list[m].npc[i]->option&OPTION_INVISIBLE )
			continue;

		switch( map->list[m].npc[i]->subtype ) {
			case WARP:
				if( !( battle_config.mob_warp&1 ) )
					continue;
				xs = map->list[m].npc[i]->u.warp.xs;
				ys = map->list[m].npc[i]->u.warp.ys;
				break;
			case SCRIPT:
				xs = map->list[m].npc[i]->u.scr.xs;
				ys = map->list[m].npc[i]->u.scr.ys;
				break;
			default:
				continue; // Keep Searching
		}

		if( x >= map->list[m].npc[i]->bl.x-xs && x <= map->list[m].npc[i]->bl.x+xs && y >= map->list[m].npc[i]->bl.y-ys && y <= map->list[m].npc[i]->bl.y+ys ) {
			// In the npc touch area
			switch( map->list[m].npc[i]->subtype ) {
				case WARP:
					xs = map->mapindex2mapid(map->list[m].npc[i]->u.warp.mapindex);
					if( m < 0 )
						break; // Cannot Warp between map servers
					if( unit->warp(&md->bl, xs, map->list[m].npc[i]->u.warp.x, map->list[m].npc[i]->u.warp.y, CLR_OUTSIGHT) == 0 )
						return 1; // Warped
					break;
				case SCRIPT:
					if( map->list[m].npc[i]->bl.id == md->areanpc_id )
						break; // Already touch this NPC
					snprintf(eventname, ARRAYLENGTH(eventname), "%s::OnTouchNPC", map->list[m].npc[i]->exname);
					if( (ev = (struct event_data*)strdb_get(npc->ev_db, eventname)) == NULL || ev->nd == NULL )
						break; // No OnTouchNPC Event
					md->areanpc_id = map->list[m].npc[i]->bl.id;
					id = md->bl.id; // Stores Unique ID
					script->run_npc(ev->nd->u.scr.script, ev->pos, md->bl.id, ev->nd->bl.id);
					if( map->id2md(id) == NULL ) return 1; // Not Warped, but killed
					break;
			}

			return 0;
		}
	}

	return 0;
}

//Checks if there are any NPC on-touch objects on the given range.
//Flag determines the type of object to check for:
//&1: NPC Warps
//&2: NPCs with on-touch events.
static int npc_check_areanpc(int flag, int16 m, int16 x, int16 y, int16 range)
{
	int i;
	int x0,y0,x1,y1;
	int xs,ys;

	Assert_retr(1, m >= 0 && m < map->count);

	if (range < 0) return 0;
	x0 = max(x-range, 0);
	y0 = max(y-range, 0);
	x1 = min(x+range, map->list[m].xs-1);
	y1 = min(y+range, map->list[m].ys-1);

	//First check for npc_cells on the range given
	i = 0;
	for (ys = y0; ys <= y1 && !i; ys++) {
		for(xs = x0; xs <= x1 && !i; xs++) {
			if (map->getcell(m, NULL, xs, ys, CELL_CHKNPC))
				i = 1;
		}
	}
	if (!i) return 0; //No NPC_CELLs.

	//Now check for the actual NPC on said range.
	for(i=0;i<map->list[m].npc_num;i++) {
		if (map->list[m].npc[i]->option&OPTION_INVISIBLE)
			continue;

		switch(map->list[m].npc[i]->subtype) {
			case WARP:
				if (!(flag&1))
					continue;
				xs=map->list[m].npc[i]->u.warp.xs;
				ys=map->list[m].npc[i]->u.warp.ys;
				break;
			case SCRIPT:
				if (!(flag&2))
					continue;
				xs=map->list[m].npc[i]->u.scr.xs;
				ys=map->list[m].npc[i]->u.scr.ys;
				break;
			default:
				continue;
		}

		if( x1 >= map->list[m].npc[i]->bl.x-xs && x0 <= map->list[m].npc[i]->bl.x+xs
		&&  y1 >= map->list[m].npc[i]->bl.y-ys && y0 <= map->list[m].npc[i]->bl.y+ys )
			break; // found a npc
	}
	if (i==map->list[m].npc_num)
		return 0;

	return (map->list[m].npc[i]->bl.id);
}

/*==========================================
 * Chk if player not too far to access the npc.
 * Returns npc_data (success) or NULL (fail).
 *------------------------------------------*/
static struct npc_data *npc_checknear(struct map_session_data *sd, struct block_list *bl)
{
	struct npc_data *nd = BL_CAST(BL_NPC, bl);
	int distance = AREA_SIZE + 1;

	nullpo_retr(NULL, sd);

	if (nd == NULL)
		return NULL;

	if (sd->npc_id == bl->id)
		return nd;

	if (nd->class_<0) //Class-less npc, enable click from anywhere.
		return nd;

	if (distance > nd->area_size)
		distance = nd->area_size;

	nullpo_retr(NULL, bl);
	if (bl->m != sd->bl.m ||
	   bl->x < sd->bl.x - distance || bl->x > sd->bl.x + distance ||
	   bl->y < sd->bl.y - distance || bl->y > sd->bl.y + distance)
	{
		return NULL;
	}

	return nd;
}

/*==========================================
 * Make NPC talk in global chat (like npctalk)
 *------------------------------------------*/
static int npc_globalmessage(const char *name, const char *mes)
{
	struct npc_data* nd = npc->name2id(name);
	char temp[100];

	if (!nd)
		return 0;

	nullpo_ret(name);
	nullpo_ret(mes);

	snprintf(temp, sizeof(temp), "%s : %s", name, mes);
	clif->GlobalMessage(&nd->bl,temp);

	return 0;
}

// MvP tomb [GreenBox]
static void run_tomb(struct map_session_data *sd, struct npc_data *nd)
{
	char buffer[200];
	char time[10];

	nullpo_retv(nd);

	sd->npc_id = nd->bl.id;

	strftime(time, sizeof(time), "%H:%M", localtime(&nd->u.tomb.kill_time));

	// TODO: Find exact color?
	snprintf(buffer, sizeof(buffer), msg_sd(sd,857), nd->u.tomb.md->db->name); // "[ ^EE0000%s^000000 ]"
	clif->scriptmes(sd, nd->bl.id, buffer);

	clif->scriptmes(sd, nd->bl.id, msg_sd(sd,858)); // "Has met its demise"

	snprintf(buffer, sizeof(buffer), msg_sd(sd,859), time); // "Time of death : ^EE0000%s^000000"
	clif->scriptmes(sd, nd->bl.id, buffer);

	clif->scriptmes(sd, nd->bl.id, msg_sd(sd,860)); // "Defeated by"

	snprintf(buffer, sizeof(buffer), msg_sd(sd,861), nd->u.tomb.killer_name[0] ? nd->u.tomb.killer_name : msg_sd(sd,15)); // "[^EE0000%s^000000]" / "Unknown"
	clif->scriptmes(sd, nd->bl.id, buffer);

	clif->scriptclose(sd, nd->bl.id);
}

/*==========================================
 * NPC 1st call when clicking on npc
 * Do specific action for NPC type (openshop, run scripts...)
 *------------------------------------------*/
static int npc_click(struct map_session_data *sd, struct npc_data *nd)
{
	nullpo_retr(1, sd);

	// This usually happens when the player clicked on a NPC that has the view id
	// of a mob, to activate this kind of npc it's needed to be in a 2,2 range
	// from it. If the OnTouch area of a npc, coincides with the 2,2 range of
	// another it's expected that the OnTouch event be put first in stack, because
	// unit_walktoxy_timer is executed before any other function in this case.
	// So it's best practice to put an 'end;' before OnTouch events in npcs that
	// have view ids of mobs to avoid this "issue" [Panikon]
	if (sd->npc_id != 0) {
		// The player clicked a npc after entering an OnTouch area
		if( sd->areanpc_id != sd->npc_id )
			ShowError("npc_click: npc_id != 0\n");

		return 1;
	}

	if( !nd )
		return 1;

	if ((nd = npc->checknear(sd,&nd->bl)) == NULL)
		return 1;

	//Hidden/Disabled npc.
	if (nd->class_ < 0 || nd->option&(OPTION_INVISIBLE|OPTION_HIDE))
		return 1;

	switch(nd->subtype) {
		case SHOP:
			clif->npcbuysell(sd,nd->bl.id);
			break;
		case CASHSHOP:
			clif->cashshop_show(sd,nd);
			break;
		case SCRIPT:
			if( nd->u.scr.shop && nd->u.scr.shop->items && nd->u.scr.trader ) {
				if( !npc->trader_open(sd,nd) )
					return 1;
			} else
				script->run_npc(nd->u.scr.script,0,sd->bl.id,nd->bl.id);
			break;
		case TOMB:
			npc->run_tomb(sd,nd);
			break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int npc_scriptcont(struct map_session_data *sd, int id, bool closing)
{
	struct block_list *target = map->id2bl(id);
	nullpo_retr(1, sd);

	if( id != sd->npc_id ){
		struct npc_data *nd_sd = map->id2nd(sd->npc_id);
		struct npc_data *nd = BL_CAST(BL_NPC, target);
		ShowDebug("npc_scriptcont: %s (sd->npc_id=%d) is not %s (id=%d).\n",
			nd_sd?(char*)nd_sd->name:"'Unknown NPC'", (int)sd->npc_id,
			nd?(char*)nd->name:"'Unknown NPC'", (int)id);
		return 1;
	}

	if (id != npc->fake_nd->bl.id) { // Not item script
		if (sd->state.npc_unloaded != 0) {
			sd->state.npc_unloaded = 0;
		} else if ((npc->checknear(sd,target)) == NULL) {
			ShowWarning("npc_scriptcont: failed npc->checknear test.\n");
			return 1;
		}
	}
	/**
	 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
	 **/
#ifdef SECURE_NPCTIMEOUT
	/**
	 * Update the last NPC iteration
	 **/
	sd->npc_idle_tick = timer->gettick();
#endif

	/**
	 * WPE can get to this point with a progressbar; we deny it.
	 **/
	if( sd->progressbar.npc_id && DIFF_TICK(sd->progressbar.timeout,timer->gettick()) > 0 )
		return 1;

	if( !sd->st ) {
		sd->npc_id = 0;
		return 1;
	}

	if( closing && sd->st->state == CLOSE )
		sd->st->state = END;

	script->run_main(sd->st);

	return 0;
}

/*==========================================
 * Chk if valid call then open buy or selling list
 *------------------------------------------*/
static int npc_buysellsel(struct map_session_data *sd, int id, int type)
{
	struct npc_data *nd;

	nullpo_retr(1, sd);

	if ((nd = npc->checknear(sd,map->id2bl(id))) == NULL)
		return 1;

	if ( nd->subtype != SHOP && !(nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->items) ) {
		if( nd->subtype == SCRIPT )
			ShowError("npc_buysellsel: trader '%s' has no shop list!\n",nd->exname);
		else
			ShowError("npc_buysellsel: no such shop npc %d (%s)\n",id,nd->exname);

		if (sd->npc_id == id)
			sd->npc_id = 0;
		return 1;
	}

	if (nd->option & OPTION_INVISIBLE) // can't buy if npc is not visible (hack?)
		return 1;

	if( nd->class_ < 0 && !sd->state.callshop ) {// not called through a script and is not a visible NPC so an invalid call
		return 1;
	}

	// reset the callshop state for future calls
	sd->state.callshop = 0;
	sd->npc_shopid = id;

	if (type==0) {
		clif->buylist(sd,nd);
	} else {
		clif->selllist(sd);
	}
	return 0;
}

/*==========================================
 * Cash Shop Buy List
 *------------------------------------------*/
static int npc_cashshop_buylist(struct map_session_data *sd, int points, struct itemlist *item_list)
{
	int i, j, new_, w, vt;
	struct npc_data *nd = NULL;
	struct npc_item_list *shop = NULL;
	unsigned short shop_size = 0;

	nullpo_retr(ERROR_TYPE_SYSTEM, sd);
	nullpo_retr(ERROR_TYPE_SYSTEM, item_list);
	if( sd->state.trading )
		return ERROR_TYPE_EXCHANGE;

	if (VECTOR_LENGTH(*item_list) <= 0)
		return ERROR_TYPE_ITEM_ID;

	if( points < 0 )
		return ERROR_TYPE_MONEY;

	nd = map->id2nd(sd->npc_shopid);
	if (nd == NULL)
		return ERROR_TYPE_NPC;

	if( nd->subtype != CASHSHOP ) {
		if (nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type != NST_ZENY && nd->u.scr.shop->type != NST_MARKET && nd->u.scr.shop->type != NST_BARTER) {
			shop = nd->u.scr.shop->item;
			shop_size = nd->u.scr.shop->items;
		} else {
			return ERROR_TYPE_NPC;
		}
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;
	}

	new_ = 0;
	w = 0;
	vt = 0; // Global Value

	// Validating Process ----------------------------------------------------
	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);

		if (!itemdb->exists(entry->id) || entry->amount <= 0)
			return ERROR_TYPE_ITEM_ID;

		ARR_FIND(0,shop_size,j,shop[j].nameid == entry->id);
		if (j == shop_size || shop[j].value <= 0)
			return ERROR_TYPE_ITEM_ID;

		if (!itemdb->isstackable(entry->id) && entry->amount > 1) {
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of non-stackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, entry->amount, entry->id);
			entry->amount = 1;
		}

		switch (pc->checkadditem(sd, entry->id, entry->amount)) {
			case ADDITEM_NEW:
				new_++;
				break;
			case ADDITEM_OVERAMOUNT:
				return ERROR_TYPE_INVENTORY_WEIGHT;
		}

		vt += shop[j].value * entry->amount;
		w += itemdb_weight(entry->id) * entry->amount;
	}

	if( w + sd->weight > sd->max_weight )
		return ERROR_TYPE_INVENTORY_WEIGHT;

	if( pc->inventoryblank(sd) < new_ )
		return ERROR_TYPE_INVENTORY_WEIGHT;

	if( points > vt ) points = vt;

	// Payment Process ----------------------------------------------------
	if( nd->subtype == SCRIPT && nd->u.scr.shop->type == NST_CUSTOM ) {
		if( !npc->trader_pay(nd,sd,vt,points) )
			return ERROR_TYPE_MONEY;
	} else {
		if( sd->kafraPoints < points || sd->cashPoints < (vt - points) )
			return ERROR_TYPE_MONEY;
		pc->paycash(sd,vt,points);
	}
	// Delivery Process ----------------------------------------------------
	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		struct item item_tmp;

		memset(&item_tmp,0,sizeof(item_tmp));

		if (!pet->create_egg(sd, entry->id)) {
			item_tmp.nameid = entry->id;
			item_tmp.identify = 1;
			pc->additem(sd, &item_tmp, entry->amount, LOG_TYPE_NPC);
		}
	}

	return ERROR_TYPE_NONE;
}

//npc_buylist for script-controlled shops.
static int npc_buylist_sub(struct map_session_data *sd, struct itemlist *item_list, struct npc_data *nd)
{
	char npc_ev[EVENT_NAME_LENGTH];
	int i;
	int key_nameid = 0;
	int key_amount = 0;

	nullpo_ret(item_list);
	nullpo_ret(nd);

	// discard old contents
	script->cleararray_pc(sd, "@bought_nameid", (void*)0);
	script->cleararray_pc(sd, "@bought_quantity", (void*)0);

	// save list of bought items
	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		intptr_t nameid = entry->id;
		intptr_t amount = entry->amount;
		script->setarray_pc(sd, "@bought_nameid", i, (void *)nameid, &key_nameid);
		script->setarray_pc(sd, "@bought_quantity", i, (void *)amount, &key_amount);
	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::OnBuyItem", nd->exname);
	npc->event(sd, npc_ev, 0);

	return 0;
}
/**
 * Loads persistent NPC Market Data from SQL
 **/
static void npc_market_fromsql(void)
{
	struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);
	char name[NAME_LENGTH+1];
	int itemid;
	int amount;

	if ( SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `name`, `itemid`, `amount` FROM `%s`", map->npc_market_data_db)
		|| SQL_ERROR == SQL->StmtExecute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}

	SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &name,    sizeof name,   NULL, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_INT,    &itemid,  sizeof itemid, NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_INT,    &amount,  sizeof amount, NULL, NULL);

	while ( SQL_SUCCESS == SQL->StmtNextRow(stmt) ) {
		struct npc_data *nd = NULL;
		unsigned short i;

		if( !(nd = npc->name2id(name)) ) {
			ShowError("npc_market_fromsql: NPC '%s' not found! skipping...\n",name);
			npc->market_delfromsql_sub(name, INT_MAX);
			continue;
		} else if (nd->subtype != SCRIPT || !nd->u.scr.shop || !nd->u.scr.shop->items || nd->u.scr.shop->type != NST_MARKET) {
			ShowError("npc_market_fromsql: NPC '%s' is not proper for market, skipping...\n",name);
			npc->market_delfromsql_sub(name, INT_MAX);
			continue;
		}

		for(i = 0; i < nd->u.scr.shop->items; i++) {
			if( nd->u.scr.shop->item[i].nameid == itemid ) {
				nd->u.scr.shop->item[i].qty = amount;
				break;
			}
		}

		if( i == nd->u.scr.shop->items ) {
			ShowError("npc_market_fromsql: NPC '%s' does not sell item %d (qty %d), deleting...\n",name,itemid,amount);
			npc->market_delfromsql_sub(name, itemid);
			continue;
		}
	}
	SQL->StmtFree(stmt);
}
/**
 * Saves persistent NPC Market Data into SQL
 **/
static void npc_market_tosql(struct npc_data *nd, int index)
{
	nullpo_retv(nd);
	Assert_retv(index >= 0 && index < nd->u.scr.shop->items);
	if (SQL_ERROR == SQL->Query(map->mysql_handle, "REPLACE INTO `%s` VALUES ('%s','%d','%u')",
		map->npc_market_data_db, nd->exname, nd->u.scr.shop->item[index].nameid, nd->u.scr.shop->item[index].qty))
		Sql_ShowDebug(map->mysql_handle);
}
/**
 * Removes persistent NPC Market Data from SQL
 */
static void npc_market_delfromsql_sub(const char *npcname, int index)
{
	if (index == INT_MAX ) {
		if( SQL_ERROR == SQL->Query(map->mysql_handle, "DELETE FROM `%s` WHERE `name`='%s'", map->npc_market_data_db, npcname) )
			Sql_ShowDebug(map->mysql_handle);
	} else {
		if( SQL_ERROR == SQL->Query(map->mysql_handle, "DELETE FROM `%s` WHERE `name`='%s' AND `itemid`='%d' LIMIT 1",
			map->npc_market_data_db, npcname, index) )
			Sql_ShowDebug(map->mysql_handle);
	}
}
/**
 * Removes persistent NPC Market Data from SQL
 **/
static void npc_market_delfromsql(struct npc_data *nd, int index)
{
	nullpo_retv(nd);
	Assert_retv(index == INT_MAX || (index >= 0 && index < nd->u.scr.shop->items));
	npc->market_delfromsql_sub(nd->exname, index == INT_MAX ? index : nd->u.scr.shop->item[index].nameid);
}

/**
 * Loads persistent NPC Barter Data from SQL
 **/
static void npc_barter_fromsql(void)
{
	struct SqlStmt *stmt = SQL->StmtMalloc(map->mysql_handle);
	char name[NAME_LENGTH + 1];
	int itemid;
	int amount;
	int removeId;
	int removeAmount;

	if (SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `name`, `itemId`, `amount`, `priceId`, `priceAmount` FROM `%s`", map->npc_barter_data_db)
		|| SQL_ERROR == SQL->StmtExecute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}

	SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &name,          sizeof name,         NULL, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_INT,    &itemid,        sizeof itemid,       NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_UINT32, &amount,        sizeof amount,       NULL, NULL);
	SQL->StmtBindColumn(stmt, 3, SQLDT_INT,    &removeId,      sizeof removeId,     NULL, NULL);
	SQL->StmtBindColumn(stmt, 4, SQLDT_INT,    &removeAmount,  sizeof removeAmount, NULL, NULL);

	while (SQL_SUCCESS == SQL->StmtNextRow(stmt)) {
		struct npc_data *nd = NULL;
		unsigned short i;

		if (!(nd = npc->name2id(name))) {
			ShowError("npc_barter_fromsql: NPC '%s' not found! skipping...\n",name);
			npc->barter_delfromsql_sub(name, INT_MAX, 0, 0);
			continue;
		} else if (nd->subtype != SCRIPT || !nd->u.scr.shop || !nd->u.scr.shop->items || nd->u.scr.shop->type != NST_BARTER) {
			ShowError("npc_barter_fromsql: NPC '%s' is not proper for barter, skipping...\n",name);
			npc->barter_delfromsql_sub(name, INT_MAX, 0, 0);
			continue;
		}

		for (i = 0; i < nd->u.scr.shop->items; i++) {
			struct npc_item_list *const item = &nd->u.scr.shop->item[i];
			if (item->nameid == itemid && item->value == removeId && item->value2 == removeAmount) {
				item->qty = amount;
				break;
			}
		}

		if (i == nd->u.scr.shop->items) {
			ShowError("npc_barter_fromsql: NPC '%s' does not sell item %d (qty %d), deleting...\n", name, itemid, amount);
			npc->barter_delfromsql_sub(name, itemid, removeId, removeAmount);
			continue;
		}
	}
	SQL->StmtFree(stmt);
}

/**
 * Saves persistent NPC Barter Data into SQL
 **/
static void npc_barter_tosql(struct npc_data *nd, int index)
{
	nullpo_retv(nd);
	Assert_retv(index >= 0 && index < nd->u.scr.shop->items);
	const struct npc_item_list *const item = &nd->u.scr.shop->item[index];
	if (SQL_ERROR == SQL->Query(map->mysql_handle, "REPLACE INTO `%s` VALUES ('%s', '%d', '%u', '%u', '%d')",
	    map->npc_barter_data_db, nd->exname, item->nameid, item->qty, item->value, item->value2)) {
		Sql_ShowDebug(map->mysql_handle);
	}
}

/**
 * Removes persistent NPC Barter Data from SQL
 */
static void npc_barter_delfromsql_sub(const char *npcname, int itemId, int itemId2, int amount2)
{
	if (itemId == INT_MAX) {
		if (SQL_ERROR == SQL->Query(map->mysql_handle, "DELETE FROM `%s` WHERE `name`='%s'", map->npc_barter_data_db, npcname))
			Sql_ShowDebug(map->mysql_handle);
	} else {
		if (SQL_ERROR == SQL->Query(map->mysql_handle, "DELETE FROM `%s` WHERE `name`='%s' AND `itemId`='%d' AND `priceId`='%d' AND `priceAmount`='%d' LIMIT 1",
		    map->npc_barter_data_db, npcname, itemId, itemId2, amount2)) {
			Sql_ShowDebug(map->mysql_handle);
		}
	}
}

/**
 * Removes persistent NPC Barter Data from SQL
 **/
static void npc_barter_delfromsql(struct npc_data *nd, int index)
{
	nullpo_retv(nd);
	if (index == INT_MAX) {
		npc->barter_delfromsql_sub(nd->exname, INT_MAX, 0, 0);
	} else {
		Assert_retv(index >= 0 && index < nd->u.scr.shop->items);
		const struct npc_item_list *const item = &nd->u.scr.shop->item[index];
		npc->barter_delfromsql_sub(nd->exname, item->nameid, item->value, item->value2);
	}
}

/**
 * Judges whether to allow and spawn a trader's window.
 **/
static bool npc_trader_open(struct map_session_data *sd, struct npc_data *nd)
{
	nullpo_retr(false, sd);
	nullpo_retr(false, nd);
	if( !nd->u.scr.shop || !nd->u.scr.shop->items )
		return false;

	switch( nd->u.scr.shop->type ) {
		case NST_ZENY:
			sd->state.callshop = 1;
			clif->npcbuysell(sd,nd->bl.id);
			return true;/* we skip sd->npc_shopid, npc->buysell will set it then when the player selects */
		case NST_MARKET: {
				unsigned short i;

				for(i = 0; i < nd->u.scr.shop->items; i++) {
					if( nd->u.scr.shop->item[i].qty )
						break;
				}

				/* nothing to display, no items available */
				if (i == nd->u.scr.shop->items) {
					clif->messagecolor_self(sd->fd, COLOR_RED, msg_sd(sd,881));
					return false;
				}

				clif->npc_market_open(sd,nd);
			}
			break;
		case NST_BARTER:
			clif->npc_barter_open(sd, nd);
			break;
		default:
			clif->cashshop_show(sd,nd);
			break;
	}
	sd->npc_shopid = nd->bl.id;
	return true;
}
/**
 * Creates (npc_data)->u.scr.shop and updates all duplicates across the server to match the created pointer
 *
 * @param master id of the original npc
 **/
static void npc_trader_update(int master)
{
	struct DBIterator *iter;
	struct block_list* bl;
	struct npc_data *master_nd = map->id2nd(master);

	CREATE(master_nd->u.scr.shop,struct npc_shop_data,1);

	iter = db_iterator(map->id_db);
	for (bl = dbi_first(iter); dbi_exists(iter); bl = dbi_next(iter)) {
		if (bl->type == BL_NPC) {
			struct npc_data *nd = BL_UCAST(BL_NPC, bl);
			if (nd->src_id == master) {
				nd->u.scr.shop = master_nd->u.scr.shop;
			}
		}
	}
	dbi_destroy(iter);
}
/**
 * Tries to issue a CountFunds event to the shop.
 *
 * @param nd shop
 * @param sd player
 **/
static void npc_trader_count_funds(struct npc_data *nd, struct map_session_data *sd)
{
	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;

	nullpo_retv(nd);
	nullpo_retv(sd);

	npc->trader_funds[0] = npc->trader_funds[1] = 0;/* clear */

	switch( nd->u.scr.shop->type ) {
		case NST_CASH:
			npc->trader_funds[0] = sd->cashPoints;
			npc->trader_funds[1] = sd->kafraPoints;
			return;
		case NST_CUSTOM:
			break;
		default:
			ShowError("npc_trader_count_funds: unsupported shop type %d\n",nd->u.scr.shop->type);
			return;
	}

	snprintf(evname, EVENT_NAME_LENGTH, "%s::OnCountFunds",nd->exname);

	if ( (ev = strdb_get(npc->ev_db, evname)) )
		script->run_npc(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	else
		ShowError("npc_trader_count_funds: '%s' event '%s' not found, operation failed\n",nd->exname,evname);

	/* the callee will rely on npc->trader_funds, upon success script->run updates them */
}
/**
 * Tries to issue a payment to the NPC Event capable of handling it
 *
 * @param nd shop
 * @param sd player
 * @param price total cost
 * @param points the amount input in the shop by the user to use from the secondary currency (if any is being employed)
 *
 * @return bool whether it was successful (if the script does not respond it will fail)
 **/
static bool npc_trader_pay(struct npc_data *nd, struct map_session_data *sd, int price, int points)
{
	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;

	nullpo_retr(false, nd);
	nullpo_retr(false, sd);
	npc->trader_ok = false;/* clear */

	snprintf(evname, EVENT_NAME_LENGTH, "%s::OnPayFunds",nd->exname);
	if ( (ev = strdb_get(npc->ev_db, evname)) ) {
		pc->setreg(sd,script->add_variable("@price"),price);
		pc->setreg(sd,script->add_variable("@points"),points);
		script->run_npc(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	} else
		ShowError("npc_trader_pay: '%s' event '%s' not found, operation failed\n",nd->exname,evname);

	return npc->trader_ok;/* run script will deal with it */
}
/*==========================================
 * Cash Shop Buy
 *------------------------------------------*/
static int npc_cashshop_buy(struct map_session_data *sd, int nameid, int amount, int points)
{
	struct npc_data *nd = NULL;
	struct item_data *item;
	struct npc_item_list *shop = NULL;
	int i, price, w;
	unsigned short shop_size = 0;

	nullpo_retr(ERROR_TYPE_SYSTEM, sd);
	if( amount <= 0 )
		return ERROR_TYPE_ITEM_ID;

	if( points < 0 )
		return ERROR_TYPE_MONEY;

	if( sd->state.trading )
		return ERROR_TYPE_EXCHANGE;

	nd = map->id2nd(sd->npc_shopid);
	if (nd == NULL)
		return ERROR_TYPE_NPC;

	if( (item = itemdb->exists(nameid)) == NULL )
		return ERROR_TYPE_ITEM_ID; // Invalid Item

	if( nd->subtype != CASHSHOP ) {
		if (nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type != NST_ZENY && nd->u.scr.shop->type != NST_MARKET && nd->u.scr.shop->type != NST_BARTER) {
			shop = nd->u.scr.shop->item;
			shop_size = nd->u.scr.shop->items;
		} else {
			return ERROR_TYPE_NPC;
		}
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;
	}

	ARR_FIND(0, shop_size, i, shop[i].nameid == nameid);

	if( i == shop_size )
		return ERROR_TYPE_ITEM_ID;

	if( shop[i].value <= 0 )
		return ERROR_TYPE_ITEM_ID;

	if(!itemdb->isstackable(nameid) && amount > 1) {
		ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of non-stackable item %d!\n",
			sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
		amount = 1;
	}

	switch( pc->checkadditem(sd, nameid, amount) ) {
		case ADDITEM_NEW:
			if( pc->inventoryblank(sd) == 0 )
				return ERROR_TYPE_INVENTORY_WEIGHT;
			break;
		case ADDITEM_OVERAMOUNT:
			return ERROR_TYPE_INVENTORY_WEIGHT;
	}

	w = item->weight * amount;
	if( w + sd->weight > sd->max_weight )
		return ERROR_TYPE_INVENTORY_WEIGHT;

	if ((int64)shop[i].value * amount > INT_MAX) {
		ShowWarning("npc_cashshop_buy: Item '%s' (%d) price overflow attempt!\n", item->name, nameid);
		ShowDebug("(NPC:'%s' (%s,%d,%d), player:'%s' (%d/%d), value:%u, amount:%d)\n",
				nd->exname, map->list[nd->bl.m].name, nd->bl.x, nd->bl.y,
				sd->status.name, sd->status.account_id, sd->status.char_id,
				shop[i].value, amount);
		return ERROR_TYPE_ITEM_ID;
	}

	price = shop[i].value * amount;

	if( points > price )
		points = price;

	if( nd->subtype == SCRIPT && nd->u.scr.shop->type == NST_CUSTOM ) {
		if( !npc->trader_pay(nd,sd,price,points) )
			return ERROR_TYPE_MONEY;
	} else {
		if( (sd->kafraPoints < points) || (sd->cashPoints < price - points) )
			return ERROR_TYPE_MONEY;

		pc->paycash(sd, price, points);
	}

	if( !pet->create_egg(sd, nameid) ) {
		struct item item_tmp;
		memset(&item_tmp, 0, sizeof(struct item));
		item_tmp.nameid = nameid;
		item_tmp.identify = 1;

		pc->additem(sd,&item_tmp, amount, LOG_TYPE_NPC);
	}

	return ERROR_TYPE_NONE;
}

/**
 * Processes a player item purchase from npc shop.
 *
 * @param sd        Buyer character.
 * @param item_list List of items.
 * @return result code for clif->parse_NpcBuyListSend.
 */
static int npc_buylist(struct map_session_data *sd, struct itemlist *item_list)
{
	struct npc_data* nd;
	struct npc_item_list *shop = NULL;
	int64 z;
	int i,j,w,skill_t,new_, idx = skill->get_index(MC_DISCOUNT);
	unsigned short shop_size = 0;

	nullpo_retr(3, sd);
	nullpo_retr(3, item_list);

	nd = npc->checknear(sd,map->id2bl(sd->npc_shopid));
	if( nd == NULL )
		return 3;

	if( nd->subtype != SHOP ) {
		if( nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type == NST_ZENY ) {
			shop = nd->u.scr.shop->item;
			shop_size = nd->u.scr.shop->items;
		} else
			return 3;
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;
	}

	z = 0;
	w = 0;
	new_ = 0;
	// process entries in buy list, one by one
	for (i = 0; i < VECTOR_LENGTH(*item_list); ++i) {
		int value;
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);

		// find this entry in the shop's sell list
		ARR_FIND( 0, shop_size, j,
				 entry->id == shop[j].nameid || //Normal items
				 entry->id == itemdb_viewid(shop[j].nameid) //item_avail replacement
				 );
		if (j == shop_size)
			return 3; // no such item in shop

		entry->id = shop[j].nameid; //item_avail replacement
		value = shop[j].value;

		if (!itemdb->exists(entry->id))
			return 3; // item no longer in itemdb

		if (!itemdb->isstackable(entry->id) && entry->amount > 1) {
			//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of non-stackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, entry->amount, entry->id);
			entry->amount = 1;
		}

		if( nd->master_nd ) {
			// Script-controlled shops decide by themselves, what can be bought and for what price.
			continue;
		}

		switch (pc->checkadditem(sd, entry->id, entry->amount)) {
			case ADDITEM_EXIST:
				break;

			case ADDITEM_NEW:
				new_++;
				break;

			case ADDITEM_OVERAMOUNT:
#if PACKETVER >= 20110705
				return 9;
#else
				return 2;
#endif
		}

		value = pc->modifybuyvalue(sd,value);

		z += (int64)value * entry->amount;
		w += itemdb_weight(entry->id) * entry->amount;
	}

	if (nd->master_nd != NULL) //Script-based shops.
		return npc->buylist_sub(sd, item_list, nd->master_nd);

	if (z > sd->status.zeny)
		return 1; // Not enough Zeny
	if( w + sd->weight > sd->max_weight )
		return 2; // Too heavy
	if( pc->inventoryblank(sd) < new_ )
		return 3; // Not enough space to store items

	pc->payzeny(sd, (int)z, LOG_TYPE_NPC, NULL);

	for (i = 0; i < VECTOR_LENGTH(*item_list); ++i) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		if (itemdb_type(entry->id) == IT_PETEGG) {
			pet->create_egg(sd, entry->id);
		} else {
			struct item item_tmp;
			memset(&item_tmp,0,sizeof(item_tmp));
			item_tmp.nameid = entry->id;
			item_tmp.identify = 1;

			pc->additem(sd, &item_tmp, entry->amount, LOG_TYPE_NPC);
		}
	}

	// custom merchant shop exp bonus
	if( battle_config.shop_exp > 0 && z > 0 && (skill_t = pc->checkskill2(sd,idx)) > 0 ) {
		if( sd->status.skill[idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill_t = sd->status.skill[idx].flag - SKILL_FLAG_REPLACED_LV_0;

		if( skill_t > 0 ) {
			z = apply_percentrate64(z, skill_t * battle_config.shop_exp, 10000);
			if (z < 1)
				z = 1;
			pc->gainexp(sd, NULL, 0, (int)z, false);
		}
	}

	return 0;
}

/**
 * Processes incoming npc market purchase list
 **/
static enum market_buy_result npc_market_buylist(struct map_session_data *sd, struct itemlist *item_list)
{
	struct npc_data* nd;
	struct npc_item_list *shop = NULL;
	int64 z;
	int i,j,w,new_;
	unsigned short shop_size = 0;

	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	nd = npc->checknear(sd,map->id2bl(sd->npc_shopid));

	if (nd == NULL || nd->subtype != SCRIPT || VECTOR_LENGTH(*item_list) == 0 || !nd->u.scr.shop || nd->u.scr.shop->type != NST_MARKET)
		return MARKET_BUY_RESULT_ERROR;

	shop = nd->u.scr.shop->item;
	shop_size = nd->u.scr.shop->items;

	z = 0;
	w = 0;
	new_ = 0;

	// process entries in buy list, one by one
	for (i = 0; i < VECTOR_LENGTH(*item_list); ++i) {
		int value;
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);

		// find this entry in the shop's sell list
		ARR_FIND( 0, shop_size, j,
				 entry->id == shop[j].nameid || //Normal items
				 entry->id == itemdb_viewid(shop[j].nameid) //item_avail replacement
				 );
		if (j == shop_size) /* TODO find official response for this */
			return MARKET_BUY_RESULT_ERROR; // no such item in shop

		entry->id = shop[j].nameid; //item_avail replacement

		if (entry->amount > (int)shop[j].qty)
			return MARKET_BUY_RESULT_AMOUNT_TOO_BIG;

		value = shop[j].value;
		npc_market_qty[i] = j;

		if (!itemdb->exists(entry->id)) /* TODO find official response for this */
			return MARKET_BUY_RESULT_ERROR; // item no longer in itemdb

		if (!itemdb->isstackable(entry->id) && entry->amount > 1) {
			//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of non-stackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, entry->amount, entry->id);
			entry->amount = 1;
		}

		switch (pc->checkadditem(sd, entry->id, entry->amount)) {
			case ADDITEM_EXIST:
				break;
			case ADDITEM_NEW:
				new_++;
				break;
			case ADDITEM_OVERAMOUNT: /* TODO find official response for this */
				return 1;
		}

		z += (int64)value * entry->amount;
		w += itemdb_weight(entry->id) * entry->amount;
	}

	if (z > sd->status.zeny) /* TODO find official response for this */
		return MARKET_BUY_RESULT_NO_ZENY; // Not enough Zeny

	if( w + sd->weight > sd->max_weight ) /* TODO find official response for this */
		return MARKET_BUY_RESULT_OVER_WEIGHT; // Too heavy

	if( pc->inventoryblank(sd) < new_ ) /* TODO find official response for this */
		return MARKET_BUY_RESULT_OUT_OF_SPACE; // Not enough space to store items

	pc->payzeny(sd,(int)z,LOG_TYPE_NPC, NULL);

	for (i = 0; i < VECTOR_LENGTH(*item_list); ++i) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);

		j = npc_market_qty[i];

		if (entry->amount > (int)shop[j].qty) /* wohoo someone tampered with the packet. */
			return MARKET_BUY_RESULT_AMOUNT_TOO_BIG;

		shop[j].qty -= entry->amount;

		npc->market_tosql(nd,j);

		if (itemdb_type(entry->id) == IT_PETEGG) {
			pet->create_egg(sd, entry->id);
		} else {
			struct item item_tmp;
			memset(&item_tmp,0,sizeof(item_tmp));
			item_tmp.nameid = entry->id;
			item_tmp.identify = 1;

			pc->additem(sd, &item_tmp, entry->amount, LOG_TYPE_NPC);
		}
	}

	return MARKET_BUY_RESULT_SUCCESS;
}

/**
 * Processes incoming npc barter purchase list
 **/
static int npc_barter_buylist(struct map_session_data *sd, struct barteritemlist *item_list)
{
	struct npc_data* nd;
	struct npc_item_list *shop = NULL;
	int w, new_;
	unsigned short shop_size = 0;

	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	nd = npc->checknear(sd, map->id2bl(sd->npc_shopid));

	if (nd == NULL || nd->subtype != SCRIPT || VECTOR_LENGTH(*item_list) == 0 || !nd->u.scr.shop || nd->u.scr.shop->type != NST_BARTER)
		return 11;

	shop = nd->u.scr.shop->item;
	shop_size = nd->u.scr.shop->items;

	w = 0;
	new_ = 0;

	int items[MAX_INVENTORY] = { 0 };

	// process entries in buy list, one by one
	for (int i = 0; i < VECTOR_LENGTH(*item_list); ++i) {
		struct barter_itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);

		const int n = entry->removeIndex;
		if (n < 0 || n >= sd->status.inventorySize)
			return 11;  // wrong inventory index

		int removeId = sd->status.inventory[n].nameid;
		const int j = entry->shopIndex;
		if (j < 0 || j >= shop_size)
			return 13;  // no such item in shop
		if (entry->addId != shop[j].nameid && entry->addId != itemdb_viewid(shop[j].nameid))
			return 13;  // no such item in shop
		if (removeId != shop[j].value && removeId != itemdb_viewid(shop[j].value))
			return 13;  // no such item in shop
		entry->addId = shop[j].nameid;  // item_avail replacement
		removeId = shop[j].value;  // item_avail replacement

		if (!itemdb->exists(entry->addId))
			return 13; // item no longer in itemdb

		const int removeAmount = shop[j].value2;

		if ((int)shop[j].qty != -1 && entry->addAmount > (int)shop[j].qty)
			return 14;  // not enough item amount in shop

		if (removeAmount * entry->addAmount > sd->status.inventory[n].amount)
			return 14;  // not enough item amount in inventory

		items[n] += removeAmount * entry->addAmount;

		if (items[n] > sd->status.inventory[n].amount)
			return 14;  // not enough item amount in inventory

		entry->addId = shop[j].nameid; //item_avail replacement

		npc_market_qty[i] = j;

		if (!itemdb->isstackable(entry->addId) && entry->addAmount > 1) {
			//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of non-stackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, entry->addAmount, entry->addId);
			entry->addAmount = 1;
		}

		switch (pc->checkadditem(sd, entry->addId, entry->addAmount)) {
			case ADDITEM_EXIST:
				break;
			case ADDITEM_NEW:
				new_++;
				break;
			case ADDITEM_OVERAMOUNT: /* TODO find official response for this */
				return 1;
		}

		w += itemdb_weight(entry->addId) * entry->addAmount;
		w -= itemdb_weight(removeId) * removeAmount;
	}

	if (w + sd->weight > sd->max_weight)
		return 2; // Too heavy

	if (pc->inventoryblank(sd) < new_)
		return 3; // Not enough space to store items

	for (int i = 0; i < sd->status.inventorySize; ++i) {
		const int removeAmountTotal = items[i];
		if (removeAmountTotal == 0)
			continue;
		if (pc->delitem(sd, i, removeAmountTotal, 0, DELITEM_SOLD, LOG_TYPE_NPC) != 0) {
			return 11;  // unknown exploit
		}
	}

	for (int i = 0; i < VECTOR_LENGTH(*item_list); ++i) {
		struct barter_itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		const int shopIdx = npc_market_qty[i];

		if ((int)shop[shopIdx].qty != -1) {
			if (entry->addAmount > (int)shop[shopIdx].qty) /* wohoo someone tampered with the packet. */
				return 14;
			shop[shopIdx].qty -= entry->addAmount;
		}

		npc->barter_tosql(nd, shopIdx);

		if (itemdb_type(entry->addId) == IT_PETEGG) {
			pet->create_egg(sd, entry->addId);
		} else {
			struct item item_tmp;
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = entry->addId;
			item_tmp.identify = 1;
			pc->additem(sd, &item_tmp, entry->addAmount, LOG_TYPE_NPC);
		}
	}

	return 12;
}

/// npc_selllist for script-controlled shops
static int npc_selllist_sub(struct map_session_data *sd, struct itemlist *item_list, struct npc_data *nd)
{
	char npc_ev[EVENT_NAME_LENGTH];
	char card_slot[NAME_LENGTH];
	char opt_index_str[NAME_LENGTH];
	char opt_value_str[NAME_LENGTH];
	int i, j;
	int key_nameid = 0;
	int key_amount = 0;
	int key_refine = 0;
	int key_attribute = 0;
	int key_identify = 0;
	int key_card[MAX_SLOTS];
	int key_opt_idx[MAX_ITEM_OPTIONS];
	int key_opt_value[MAX_ITEM_OPTIONS];

	nullpo_ret(sd);
	nullpo_ret(item_list);
	nullpo_ret(nd);

	// discard old contents
	script->cleararray_pc(sd, "@sold_nameid", (void*)0);
	script->cleararray_pc(sd, "@sold_quantity", (void*)0);
	script->cleararray_pc(sd, "@sold_refine", (void*)0);
	script->cleararray_pc(sd, "@sold_attribute", (void*)0);
	script->cleararray_pc(sd, "@sold_identify", (void*)0);

	for( j = 0; j < MAX_SLOTS; j++ )
	{// clear each of the card slot entries
		key_card[j] = 0;
		snprintf(card_slot, sizeof(card_slot), "@sold_card%d", j + 1);
		script->cleararray_pc(sd, card_slot, (void*)0);
	}

	for (j = 0; j < MAX_ITEM_OPTIONS; j++) { // Clear Each item option entry
		key_opt_idx[j] = 0;
		key_opt_value[j] = 0;

		snprintf(opt_index_str, sizeof(opt_index_str), "@slot_opt_idx%d", j + 1);
		script->cleararray_pc(sd, opt_index_str, (void*)0);

		snprintf(opt_value_str, sizeof(opt_value_str), "@slot_opt_val%d", j + 1);
		script->cleararray_pc(sd, opt_value_str, (void*)0);
	}

	// save list of to be sold items
	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		struct item *item = &sd->status.inventory[entry->id];
		intptr_t nameid = item->nameid;
		intptr_t amount = entry->amount;
		intptr_t refine = item->refine;
		intptr_t attribute = item->attribute;
		intptr_t identify = item->identify;

		script->setarray_pc(sd, "@sold_nameid", i, (void*)nameid, &key_nameid);
		script->setarray_pc(sd, "@sold_quantity", i, (void*)amount, &key_amount);

		// process item based information into the arrays
		script->setarray_pc(sd, "@sold_refine", i, (void*)refine, &key_refine);
		script->setarray_pc(sd, "@sold_attribute", i, (void*)attribute, &key_attribute);
		script->setarray_pc(sd, "@sold_identify", i, (void*)identify, &key_identify);

		for (j = 0; j < MAX_SLOTS; j++) {
			intptr_t card = item->card[j];
			// store each of the cards/special info from the item in the array
			snprintf(card_slot, sizeof(card_slot), "@sold_card%d", j + 1);
			script->setarray_pc(sd, card_slot, i, (void*)card, &key_card[j]);
		}

		for (j = 0; j < MAX_ITEM_OPTIONS; j++) {
			intptr_t opt_idx = item->option[j].index;
			intptr_t opt_value = item->option[j].value;

			snprintf(opt_index_str, sizeof(opt_index_str), "@slot_opt_idx%d", j + 1);
			script->setarray_pc(sd, opt_index_str, i, (void*)opt_idx, &key_opt_idx[j]);

			snprintf(opt_value_str, sizeof(opt_value_str), "@slot_opt_val%d", j + 1);
			script->setarray_pc(sd, opt_value_str, i, (void*)opt_value, &key_opt_value[j]);
		}

	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::OnSellItem", nd->exname);
	npc->event(sd, npc_ev, 0);
	return 0;
}

/// Player item selling to npc shop.
///
/// @param item_list 'n' pairs <index,amount>
/// @return result code for clif->parse_NpcSellListSend
static int npc_selllist(struct map_session_data *sd, struct itemlist *item_list)
{
	int64 z;
	int i,skill_t, skill_idx = skill->get_index(MC_OVERCHARGE);
	struct npc_data *nd;
	bool duplicates[MAX_INVENTORY] = { 0 };

	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	if( ( nd = npc->checknear(sd, map->id2bl(sd->npc_shopid)) ) == NULL ) {
		return 1;
	}

	if( nd->subtype != SHOP ) {
		if (!(nd->subtype == SCRIPT && nd->u.scr.shop && (nd->u.scr.shop->type == NST_ZENY || nd->u.scr.shop->type == NST_MARKET)))
			return 1;
	}

	z = 0;

	if (sd->status.zeny >= MAX_ZENY && nd->master_nd == NULL)
		return 1;

	// verify the sell list
	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		int nameid, value, idx = entry->id;

		if (idx >= sd->status.inventorySize || idx < 0 || entry->amount < 0) {
			return 1;
		}

		if (duplicates[idx]) {
			// Sanity check. The client sends each inventory index at most once [Haru]
			return 1;
		}
		duplicates[idx] = true;

		nameid = sd->status.inventory[idx].nameid;

		if (!nameid || !sd->inventory_data[idx] || sd->status.inventory[idx].amount < entry->amount) {
			return 1;
		}

		if (nd->master_nd) {
			// Script-controlled shops decide by themselves, what can be sold and at what price.
			continue;
		}

		value = pc->modifysellvalue(sd, sd->inventory_data[idx]->value_sell);

		z += (int64)value * entry->amount;
	}

	if( nd->master_nd ) { // Script-controlled shops
		return npc->selllist_sub(sd, item_list, nd->master_nd);
	}

	// delete items
	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		int idx = entry->id;

		if (sd->inventory_data[idx]->type == IT_PETEGG && sd->status.inventory[idx].card[0] == CARD0_PET) {
			if (pet->search_petDB_index(sd->status.inventory[idx].nameid, PET_EGG) >= 0) {
				intif->delete_petdata(MakeDWord(sd->status.inventory[idx].card[1], sd->status.inventory[idx].card[2]));
			}
		}

		// Achievements [Smokexyz/Hercules]
		achievement->validate_item_sell(sd, sd->status.inventory[idx].nameid, entry->amount);

		pc->delitem(sd, idx, entry->amount, 0, DELITEM_SOLD, LOG_TYPE_NPC);

	}

	if (z + sd->status.zeny > MAX_ZENY && nd->master_nd == NULL)
		return 1;

	if (z > MAX_ZENY)
		z = MAX_ZENY;

	pc->getzeny(sd, (int)z, LOG_TYPE_NPC, NULL);

	// custom merchant shop exp bonus
	if( battle_config.shop_exp > 0 && z > 0 && ( skill_t = pc->checkskill2(sd,skill_idx) ) > 0) {
		if( sd->status.skill[skill_idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill_t = sd->status.skill[skill_idx].flag - SKILL_FLAG_REPLACED_LV_0;

		if( skill_t > 0 ) {
			z = apply_percentrate64(z, skill_t * battle_config.shop_exp, 10000);
			if (z < 1)
				z = 1;
			pc->gainexp(sd, NULL, 0, (int)z, false);
		}
	}

	return 0;
}

//Atempt to remove an npc from a map
//This doesn't remove it from map_db
static int npc_remove_map(struct npc_data *nd)
{
	int16 m,i;
	nullpo_retr(1, nd);

	if(nd->bl.prev == NULL || nd->bl.m < 0)
		return 1; //Not assigned to a map.
	m = nd->bl.m;
	clif->clearunit_area(&nd->bl,CLR_RESPAWN);
	npc->unsetcells(nd);
	map->delblock(&nd->bl);
	//Remove npc from map->list[].npc list. [Skotlex]
	ARR_FIND( 0, map->list[m].npc_num, i, map->list[m].npc[i] == nd );
	if( i == map->list[m].npc_num ) return 2; //failed to find it?

	map->list[m].npc_num--;
	map->list[m].npc[i] = map->list[m].npc[map->list[m].npc_num];
	map->list[m].npc[map->list[m].npc_num] = NULL;
	return 0;
}

/**
 * @see DBApply
 */
static int npc_unload_ev(union DBKey key, struct DBData *data, va_list ap)
{
	struct event_data* ev = DB->data2ptr(data);
	char* npcname = va_arg(ap, char *);

	if(strcmp(ev->nd->exname,npcname)==0){
		db_remove(npc->ev_db, key);
		return 1;
	}
	return 0;
}

/**
 * @see DBApply
 */
static int npc_unload_ev_label(union DBKey key, struct DBData *data, va_list ap)
{
	struct linkdb_node **label_linkdb = DB->data2ptr(data);
	struct npc_data* nd = va_arg(ap, struct npc_data *);

	linkdb_erase(label_linkdb, nd);

	return 0;
}

//Chk if npc matches src_id, then unload.
//Sub-function used to find duplicates.
static int npc_unload_dup_sub(struct npc_data *nd, va_list args)
{
	int src_id;

	nullpo_ret(nd);
	src_id = va_arg(args, int);
	if (nd->src_id == src_id)
		npc->unload(nd, true);
	return 0;
}

//Removes all npcs that are duplicates of the passed one. [Skotlex]
static void npc_unload_duplicates(struct npc_data *nd)
{
	nullpo_retv(nd);
	map->foreachnpc(npc->unload_dup_sub,nd->bl.id);
}

//Removes an npc from map and db.
//Single is to free name (for duplicates).
static int npc_unload(struct npc_data *nd, bool single)
{
	nullpo_ret(nd);

	if( nd->ud && nd->ud != &npc->base_ud ) {
		skill->clear_unitgroup(&nd->bl);
	}

	npc->remove_map(nd);
	map->deliddb(&nd->bl);
	if( single )
		strdb_remove(npc->name_db, nd->exname);

	if (nd->chat_id) // remove npc chatroom object and kick users
		chat->delete_npc_chat(nd);

	npc_chat->finalize(nd); // deallocate npc PCRE data structures

	if (single && nd->path != NULL) {
		npc->releasepathreference(nd->path);
		nd->path = NULL;
	}

	if (single && nd->bl.m != -1)
		map->remove_questinfo(nd->bl.m, nd);
	npc->questinfo_clear(nd);

	if (nd->src_id == 0 && ( nd->subtype == SHOP || nd->subtype == CASHSHOP)) {
		//src check for duplicate shops [Orcao]
		aFree(nd->u.shop.shop_item);
	} else if (nd->subtype == SCRIPT) {
		struct s_mapiterator *iter;
		struct map_session_data *sd = NULL;

		if( single ) {
			npc->ev_db->foreach(npc->ev_db,npc->unload_ev,nd->exname); //Clean up all events related
			npc->ev_label_db->foreach(npc->ev_label_db,npc->unload_ev_label,nd);
		}

		iter = mapit_geteachpc();
		for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
			if (sd->npc_timer_id != INVALID_TIMER ) {
				const struct TimerData *td = timer->get(sd->npc_timer_id);

				if( td && td->id != nd->bl.id )
					continue;

				if( td && td->data )
					ers_free(npc->timer_event_ers, (void*)td->data);
				timer->delete(sd->npc_timer_id, npc->timerevent);
				sd->npc_timer_id = INVALID_TIMER;
			}
		}
		mapit->free(iter);

		if (nd->u.scr.timerid != INVALID_TIMER) {
			const struct TimerData *td;
			td = timer->get(nd->u.scr.timerid);
			if (td && td->data)
				ers_free(npc->timer_event_ers, (void*)td->data);
			timer->delete(nd->u.scr.timerid, npc->timerevent);
		}
		if (nd->u.scr.timer_event)
			aFree(nd->u.scr.timer_event);
		if (nd->src_id == 0) {
			if(nd->u.scr.script) {
				script->free_code(nd->u.scr.script);
				nd->u.scr.script = NULL;
			}
			if (nd->u.scr.label_list) {
				aFree(nd->u.scr.label_list);
				nd->u.scr.label_list = NULL;
				nd->u.scr.label_list_num = 0;
			}
			if(nd->u.scr.shop) {
				if(nd->u.scr.shop->item)
					aFree(nd->u.scr.shop->item);
				aFree(nd->u.scr.shop);
			}
		}
		if( nd->u.scr.guild_id )
			guild->flag_remove(nd);
	}

	if( nd->ud && nd->ud != &npc->base_ud ) {
		aFree(nd->ud);
		nd->ud = NULL;
	}

	HPM->data_store_destroy(&nd->hdata);

	aFree(nd);

	return 0;
}

//
// NPC Source Files
//

/// Clears the npc source file list
static void npc_clearsrcfile(void)
{
	struct npc_src_list* file = npc->src_files;

	while (file != NULL) {
		struct npc_src_list *file_tofree = file;
		file = file->next;
		aFree(file_tofree);
	}
	npc->src_files = NULL;
}

/**
 * Adds a npc source file.
 *
 * @param name The file name to add.
 */
static void npc_addsrcfile(const char *name)
{
	struct npc_src_list* file;
	struct npc_src_list* file_prev = NULL;

	nullpo_retv(name);

	// prevent multiple insert of source files
	file = npc->src_files;
	while (file != NULL) {
		if (strcmp(name, file->name) == 0)
			return;// found the file, no need to insert it again
		file_prev = file;
		file = file->next;
	}

	file = (struct npc_src_list*)aMalloc(sizeof(struct npc_src_list) + strlen(name));
	file->next = NULL;
	safestrncpy(file->name, name, strlen(name) + 1);
	if( file_prev == NULL )
		npc->src_files = file;
	else
		file_prev->next = file;
}

/**
 * Removes a npc source file.
 *
 * @param name The file name to remove.
 */
static void npc_delsrcfile(const char *name)
{
	struct npc_src_list* file = npc->src_files;
	struct npc_src_list* file_prev = NULL;

	nullpo_retv(name);

	while (file != NULL) {
		if (strcmp(file->name, name) == 0) {
			if (npc->src_files == file)
				npc->src_files = file->next;
			else
				file_prev->next = file->next;
			aFree(file);
			break;
		}
		file_prev = file;
		file = file->next;
	}
}

/**
 * Retains a reference to filepath, for use in nd->path
 *
 * @param filepath The file path to retain.
 * @return A retained reference to filepath.
 */
static const char *npc_retainpathreference(const char *filepath)
{
	struct npc_path_data * npd = NULL;
	nullpo_ret(filepath);

	if (npc->npc_last_path == filepath) {
		if (npc->npc_last_npd != NULL)
			npc->npc_last_npd->references++;
		return npc->npc_last_ref;
	}

	if ((npd = strdb_get(npc->path_db,filepath)) == NULL) {
		CREATE(npd, struct npc_path_data, 1);
		strdb_put(npc->path_db, filepath, npd);

		CREATE(npd->path, char, strlen(filepath)+1);
		safestrncpy(npd->path, filepath, strlen(filepath)+1);

		npd->references = 0;
	}

	npd->references++;

	npc->npc_last_npd = npd;
	npc->npc_last_ref = npd->path;
	npc->npc_last_path = filepath;

	return npd->path;
}

/**
 * Releases a previously retained filepath.
 *
 * @param filepath The file path to release.
 */
static void npc_releasepathreference(const char *filepath)
{
	struct npc_path_data* npd = NULL;

	nullpo_retv(filepath);

	if (filepath != npc->npc_last_ref) {
		npd = strdb_get(npc->path_db, filepath);
	}

	if (npd != NULL && --npd->references == 0) {
		char *npcpath = npd->path;
		strdb_remove(npc->path_db, filepath);/* remove from db */
		aFree(npcpath);
	}
}

/// Parses and sets the name and exname of a npc.
/// Assumes that m, x and y are already set in nd.
static void npc_parsename(struct npc_data *nd, const char *name, const char *start, const char *buffer, const char *filepath)
{
	const char* p;
	struct npc_data* dnd;// duplicate npc
	char newname[NAME_LENGTH];

	nullpo_retv(nd);
	nullpo_retv(name);
	// parse name
	p = strstr(name,"::");
	if( p ) { // <Display name>::<Unique name>
		size_t len = p-name;
		if( len > NAME_LENGTH ) {
			ShowWarning("npc_parsename: Display name of '%s' is too long (len=%u) in file '%s', line '%d'. Truncating to %d characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NAME_LENGTH);
			safestrncpy(nd->name, name, sizeof(nd->name));
		} else {
			memcpy(nd->name, name, len);
			memset(nd->name+len, 0, sizeof(nd->name)-len);
		}
		len = strlen(p+2);
		if( len > NAME_LENGTH )
			ShowWarning("npc_parsename: Unique name of '%s' is too long (len=%u) in file '%s', line '%d'. Truncating to %d characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NAME_LENGTH);
		safestrncpy(nd->exname, p+2, sizeof(nd->exname));
	} else {// <Display name>
		size_t len = strlen(name);
		if( len > NAME_LENGTH )
			ShowWarning("npc_parsename: Name '%s' is too long (len=%u) in file '%s', line '%d'. Truncating to %d characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NAME_LENGTH);
		safestrncpy(nd->name, name, sizeof(nd->name));
		safestrncpy(nd->exname, name, sizeof(nd->exname));
	}

	if( *nd->exname == '\0' || strstr(nd->exname,"::") != NULL ) {// invalid
		snprintf(newname, ARRAYLENGTH(newname), "0_%d_%d_%d", nd->bl.m, nd->bl.x, nd->bl.y);
		ShowWarning("npc_parsename: Invalid unique name in file '%s', line '%d'. Renaming '%s' to '%s'.\n", filepath, strline(buffer,start-buffer), nd->exname, newname);
		safestrncpy(nd->exname, newname, sizeof(nd->exname));
	}

	if( (dnd=npc->name2id(nd->exname)) != NULL ) {// duplicate unique name, generate new one
		char this_mapname[32];
		char other_mapname[32];
		int i = 0;

		do {
			++i;
			safesnprintf(newname, ARRAYLENGTH(newname), "%d_%d_%d_%d", i, nd->bl.m, nd->bl.x, nd->bl.y);
		} while( npc->name2id(newname) != NULL );

		strcpy(this_mapname, (nd->bl.m == -1 ? "(not on a map)" : mapindex_id2name(map_id2index(nd->bl.m))));
		strcpy(other_mapname, (dnd->bl.m == -1 ? "(not on a map)" : mapindex_id2name(map_id2index(dnd->bl.m))));

		ShowWarning("npc_parsename: Duplicate unique name in file '%s', line '%d'. Renaming '%s' to '%s'.\n", filepath, strline(buffer,start-buffer), nd->exname, newname);
		ShowDebug("this npc:\n   display name '%s'\n   unique name '%s'\n   map=%s, x=%d, y=%d\n", nd->name, nd->exname, this_mapname, nd->bl.x, nd->bl.y);
		ShowDebug("other npc in '%s' :\n   display name '%s'\n   unique name '%s'\n   map=%s, x=%d, y=%d\n",dnd->path, dnd->name, dnd->exname, other_mapname, dnd->bl.x, dnd->bl.y);
		safestrncpy(nd->exname, newname, sizeof(nd->exname));
	}
}

// Parse View
// Support for using Constants in place of NPC View IDs.
static int npc_parseview(const char *w4, const char *start, const char *buffer, const char *filepath)
{
	int val = FAKE_NPC, i = 0;
	char viewid[1024]; // Max size of name from constants.conf, see script->read_constdb.

	nullpo_retr(FAKE_NPC, w4);
	// Extract view ID / constant
	while (w4[i] != '\0') {
		if (ISSPACE(w4[i]) || w4[i] == '/' || w4[i] == ',')
			break;

		i++;
	}

	safestrncpy(viewid, w4, i+=1);

	// Check if view id is not an ID (only numbers).
	if(!npc->viewisid(viewid))
	{
		// Check if constant exists and get its value.
		if(!script->get_constant(viewid, &val)) {
			ShowWarning("npc_parseview: Invalid NPC constant '%s' specified in file '%s', line '%d'. Defaulting to INVISIBLE_CLASS.\n", viewid, filepath, strline(buffer,start-buffer));
			val = INVISIBLE_CLASS;
		}
	} else {
		// NPC has an ID specified for view id.
		val = atoi(w4);
		ShowWarning("npc_parseview: Use of numeric NPC view IDs is deprecated and may be removed in a future update. Please use NPC view constants instead. ID '%d' specified in file '%s', line '%d'.\n", val, filepath, strline(buffer, start-buffer));
	}

	return val;
}

// View is ID
// Checks if given view is an ID or constant.
static bool npc_viewisid(const char *viewid)
{
	nullpo_retr(false, viewid);
	if (atoi(viewid) != FAKE_NPC) {
		// Loop through view, looking for non-numeric character.
		while (*viewid) {
			if (ISDIGIT(*viewid++) == 0) return false;
		}
	}

	return true;
}

/**
 * Creates a new NPC.
 *
 * @param subtype The NPC subtype.
 * @param m       The map id.
 * @param x       The x coordinate on map.
 * @param y       The y coordinate on map.
 * @param dir     The facing direction.
 * @param class_  The NPC view class.
 * @return A pointer to the created NPC data (ownership passed to the caller).
 */
static struct npc_data *npc_create_npc(enum npc_subtype subtype, int m, int x, int y, enum unit_dir dir, int class_)
{
	struct npc_data *nd;

	CREATE(nd, struct npc_data, 1);
	nd->subtype = subtype;
	nd->bl.type = BL_NPC;
	nd->bl.id = npc->get_new_npc_id();
	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->dir = dir;
	nd->area_size = AREA_SIZE + 1;
	nd->class_ = class_;
	nd->speed = 200;
	nd->vd = npc_viewdb[0]; // Copy INVISIBLE_CLASS view data. Actual view data is set by npc->add_to_location() later.
	VECTOR_INIT(nd->qi_data);

	return nd;
}

//Add then display an npc warp on map
static struct npc_data *npc_add_warp(char *name, short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y)
{
	int i, flag = 0;
	struct npc_data *nd;

	nullpo_retr(NULL, name);

	nd = npc->create_npc(WARP, from_mapid, from_x, from_y, 0, battle_config.warp_point_debug ? WARP_DEBUG_CLASS : WARP_CLASS);

	safestrncpy(nd->exname, name, ARRAYLENGTH(nd->exname));
	if (npc->name2id(nd->exname) != NULL)
		flag = 1;

	if (flag == 1)
		safesnprintf(nd->exname, ARRAYLENGTH(nd->exname), "warp_%d_%d_%d", from_mapid, from_x, from_y);

	for( i = 0; npc->name2id(nd->exname) != NULL; ++i )
		safesnprintf(nd->exname, ARRAYLENGTH(nd->exname), "warp%d_%d_%d_%d", i, from_mapid, from_x, from_y);
	safestrncpy(nd->name, nd->exname, ARRAYLENGTH(nd->name));

	nd->u.warp.mapindex = to_mapindex;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = xs;

	npc->add_to_location(nd);

	return nd;
}

/**
 * Parses a warp NPC.
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_warp(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	int x, y, xs, ys, to_x, to_y, m;
	unsigned short i;
	char mapname[32], to_mapname[32];
	struct npc_data *nd;

	nullpo_retr(strchr(start,'\n'), w1);
	nullpo_retr(strchr(start,'\n'), w4);

	// w1=<from map name>,<fromX>,<fromY>,<facing>
	// w4=<spanx>,<spany>,<to map name>,<toX>,<toY>
	if( sscanf(w1, "%31[^,],%d,%d", mapname, &x, &y) != 3
	 || sscanf(w4, "%d,%d,%31[^,],%d,%d", &xs, &ys, to_mapname, &to_x, &to_y) != 5
	  ) {
		ShowError("npc_parse_warp: Invalid warp definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	m = map->mapname2mapid(mapname);
	i = mapindex->name2id(to_mapname);
	if( i == 0 ) {
		ShowError("npc_parse_warp: Unknown destination map in file '%s', line '%d' : %s\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), to_mapname, w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	if( m != -1 && ( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) ) {
		ShowError("npc_parse_warp: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d, in file '%s', line '%d'\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys,filepath,strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');;//try next
	}

	nd = npc->create_npc(WARP, m, x, y, 0, battle_config.warp_point_debug ? WARP_DEBUG_CLASS : WARP_CLASS);
	npc->parsename(nd, w3, start, buffer, filepath);
	nd->path = npc->retainpathreference(filepath);

	nd->u.warp.mapindex = i;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;
	npc->npc_warp++;

	npc->add_to_location(nd);

	return strchr(start,'\n');// continue
}

/**
 * Parses a SHOP/CASHSHOP NPC.
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_shop(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	//TODO: could be rewritten to NOT need this temp array [ultramage]
	// We could use nd->u.shop.shop_item to store directly the items, but this could lead
	// to unecessary memory usage by the server, using a temp dynamic array is the
	// best way to do this without having to do multiple reallocs [Panikon]
	struct npc_item_list *items = NULL;
	size_t items_count = 40; // Starting items size

	const char *p;
	int x, y, dir, m, i, class_;
	struct npc_data *nd;
	enum npc_subtype type;

	nullpo_retr(strchr(start,'\n'), w1);
	nullpo_retr(strchr(start,'\n'), w4);
	if( strcmp(w1,"-") == 0 ) {
		// 'floating' shop
		x = y = dir = 0;
		m = -1;
	} else {// w1=<map name>,<x>,<y>,<facing>
		char mapname[32];
		if( sscanf(w1, "%31[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4
		 || strchr(w4, ',') == NULL
		  ) {
			ShowError("npc_parse_shop: Invalid shop definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			if (retval) *retval = EXIT_FAILURE;
			return strchr(start,'\n');// skip and continue
		}

		if (dir < 0 || dir > 7) {
			ShowError("npc_parse_shop: Invalid NPC facing direction '%d' in file '%s', line '%d'.\n", dir, filepath, strline(buffer, start-buffer));
			if (retval) *retval = EXIT_FAILURE;
			return strchr(start,'\n');//continue
		}

		m = map->mapname2mapid(mapname);
	}

	if( m != -1 && ( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) ) {
		ShowError("npc_parse_shop: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d, in file '%s', line '%d'\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys,filepath,strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');//try next
	}

	if( strcmp(w2,"cashshop") == 0 )
		type = CASHSHOP;
	else
		type = SHOP;

	items = aMalloc(sizeof(items[0])*items_count);

	p = strchr(w4,',');
	for( i = 0; p; ++i ) {
		int nameid, value;
		struct item_data* id;

		if( i == items_count-1 ) { // Grow array
			items_count *= 2;
			items = aRealloc(items, sizeof(items[0])*items_count);
		}

		if( sscanf(p, ",%d:%d", &nameid, &value) != 2 ) {
			ShowError("npc_parse_shop: Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			if (retval) *retval = EXIT_FAILURE;
			break;
		}

		if( (id = itemdb->exists(nameid)) == NULL ) {
			ShowWarning("npc_parse_shop: Invalid sell item in file '%s', line '%d' (id '%d').\n", filepath, strline(buffer,start-buffer), nameid);
			p = strchr(p+1,',');
			if (retval) *retval = EXIT_FAILURE;
			continue;
		}

		if( value < 0 ) {
			if( value != -1 )
				ShowWarning("npc_parse_shop: Item %s [%d] with invalid selling value '%d' in file '%s', line '%d', defaulting to buy price...\n",
					id->name, nameid, value, filepath, strline(buffer,start-buffer));

			if( type == SHOP ) value = id->value_buy;
			else value = 0; // Cashshop doesn't have a "buy price" in the item_db
		}

		if( type == SHOP && value == 0 ) {
			// NPC selling items for free!
			ShowWarning("npc_parse_shop: Item %s [%d] is being sold for FREE in file '%s', line '%d'.\n",
				id->name, nameid, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if( type == SHOP && value*0.75 < id->value_sell*1.24 ) {
			// Exploit possible: you can buy and sell back with profit
			ShowWarning("npc_parse_shop: Item %s [%d] discounted buying price (%d->%d) is less than overcharged selling price (%d->%d) in file '%s', line '%d'.\n",
				id->name, nameid, value, (int)(value*0.75), id->value_sell, (int)(id->value_sell*1.24), filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		//for logs filters, atcommands and iteminfo script command
		if( id->maxchance == 0 )
			id->maxchance = -1; // -1 would show that the item's sold in NPC Shop

		items[i].nameid = nameid;
		items[i].value = value;
		p = strchr(p+1,',');
	}
	if( i == 0 ) {
		ShowWarning("npc_parse_shop: Ignoring empty shop in file '%s', line '%d'.\n", filepath, strline(buffer,start-buffer));
		aFree(items);
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// continue
	}

	class_ = m == -1 ? FAKE_NPC : npc->parseview(w4, start, buffer, filepath);
	nd = npc->create_npc(type, m, x, y, dir, class_);
	CREATE(nd->u.shop.shop_item, struct npc_item_list, i);
	memcpy(nd->u.shop.shop_item, items, sizeof(items[0])*i);
	aFree(items);

	nd->u.shop.count = i;
	npc->parsename(nd, w3, start, buffer, filepath);
	nd->path = npc->retainpathreference(filepath);

	++npc->npc_shop;
	npc->add_to_location(nd);

	return strchr(start,'\n');// continue
}

static void npc_convertlabel_db(struct npc_label_list *label_list, const char *filepath)
{
	int i;

	nullpo_retv(label_list);
	for( i = 0; i < script->label_count; i++ ) {
		const char* lname = script->get_str(script->labels[i].key);
		int lpos = script->labels[i].pos;
		struct npc_label_list* label;
		const char *p;
		size_t len;

		// In case of labels not terminated with ':', for user defined function support
		p = lname;

		while( ISALNUM(*p) || *p == '_' )
			++p;
		len = p-lname;

		// here we check if the label fit into the buffer
		if( len > 23 ) {
			ShowError("npc_parse_script: label name longer than 23 chars! (%s) in file '%s'.\n", lname, filepath);
			return;
		}

		label = &label_list[i];

		safestrncpy(label->name, lname, sizeof(label->name));
		label->pos = lpos;
	}
}

// Skip the contents of a script.
static const char *npc_skip_script(const char *start, const char *buffer, const char *filepath, int *retval)
{
	const char* p;
	int curly_count;

	if( start == NULL )
		return NULL;// nothing to skip

	// initial bracket (assumes the previous part is ok)
	p = strchr(start,'{');
	if( p == NULL ) {
		ShowError("npc_skip_script: Missing left curly in file '%s', line '%d'.\n", filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return NULL;// can't continue
	}

	// skip everything
	for( curly_count = 1; curly_count > 0 ; )
	{
		p = script->skip_space(p+1) ;
		if( *p == '}' )
		{// right curly
			--curly_count;
		}
		else if( *p == '{' )
		{// left curly
			++curly_count;
		}
		else if( *p == '"' )
		{// string
			for( ++p; *p != '"' ; ++p )
			{
				if( *p == '\\' && (unsigned char)p[-1] <= 0x7e ) {
					++p;// escape sequence (not part of a multibyte character)
				} else if( *p == '\0' ) {
					script->error(buffer, filepath, 0, "Unexpected end of string.", p);
					if (retval) *retval = EXIT_FAILURE;
					return NULL;// can't continue
				} else if( *p == '\n' ) {
					script->error(buffer, filepath, 0, "Unexpected newline at string.", p);
					if (retval) *retval = EXIT_FAILURE;
					return NULL;// can't continue
				}
			}
		}
		else if( *p == '\0' )
		{// end of buffer
			ShowError("Missing %d right curlys in file '%s', line '%d'.\n", curly_count, filepath, strline(buffer,p-buffer));
			if (retval) *retval = EXIT_FAILURE;
			return NULL;// can't continue
		}
	}

	return p+1;// return after the last '}'
}

/**
 * Parses a NPC script.
 *
 * Example:
 * @code
 * -<TAB>script<TAB><NPC Name><TAB>FAKE_NPC,{
 *   <code>
 * }
 * <map name>,<x>,<y>,<facing><TAB>script<TAB><NPC Name><TAB><sprite id>,{
 *   <code>
 * }
 * <map name>,<x>,<y>,<facing><TAB>script<TAB><NPC Name><TAB><sprite id>,<triggerX>,<triggerY>,{
 *   <code>
 * }
 * @endcode
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[in]  options  NPC parse/load options (@see enum npc_parse_options).
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_script(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int options, int *retval)
{
	int x, y, dir = 0, m, xs = 0, ys = 0; // [Valaris] thanks to fov
	struct script_code *scriptroot;
	int i, class_;
	const char* end;
	const char* script_start;

	struct npc_label_list* label_list;
	int label_list_num;
	struct npc_data* nd;

	nullpo_retr(NULL, w1);
	if (strcmp(w1, "-") == 0) {
		// floating npc
		x = 0;
		y = 0;
		m = -1;
	} else {
		// npc in a map
		char mapname[32];
		if (sscanf(w1, "%31[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4) {
			ShowError("npc_parse_script: Invalid placement format for a script in file '%s', line '%d'. Skipping the rest of file...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			if (retval) *retval = EXIT_FAILURE;
			return NULL;// unknown format, don't continue
		}
		m = map->mapname2mapid(mapname);
	}

	script_start = strstr(start,",{");
	end = strchr(start,'\n');

	if (dir < 0 || dir > 7) {
		ShowError("npc_parse_script: Invalid NPC facing direction '%d' in file '%s', line '%d'.\n", dir, filepath, strline(buffer, start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return npc->skip_script(script_start, buffer, filepath, retval); // continue
	}

	if( strstr(w4,",{") == NULL || script_start == NULL || (end != NULL && script_start > end) )
	{
		ShowError("npc_parse_script: Missing left curly ',{' in file '%s', line '%d'. Skipping the rest of the file.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return NULL;// can't continue
	}
	++script_start;

	end = npc->skip_script(script_start, buffer, filepath, retval);
	if( end == NULL )
		return NULL;// (simple) parse error, don't continue

	script->parser_current_npc_name = w3;
	scriptroot = script->parse(script_start, filepath, strline(buffer,script_start-buffer), SCRIPT_USE_LABEL_DB, retval);
	script->parser_current_npc_name = NULL;

	label_list = NULL;
	label_list_num = 0;
	if( script->label_count ) {
		CREATE(label_list,struct npc_label_list,script->label_count);
		label_list_num = script->label_count;
		npc->convertlabel_db(label_list,filepath);
	}

	class_ = m == -1 ? FAKE_NPC : npc->parseview(w4, start, buffer, filepath);
	nd = npc->create_npc(SCRIPT, m, x, y, dir, class_);
	if (sscanf(w4, "%*[^,],%d,%d", &xs, &ys) == 2) {
		// OnTouch area defined
		nd->u.scr.xs = xs;
		nd->u.scr.ys = ys;
	} else {
		// no OnTouch area
		nd->u.scr.xs = -1;
		nd->u.scr.ys = -1;
	}

	npc->parsename(nd, w3, start, buffer, filepath);
	nd->path = npc->retainpathreference(filepath);
	nd->u.scr.script = scriptroot;
	nd->u.scr.label_list = label_list;
	nd->u.scr.label_list_num = label_list_num;
	if( options&NPO_TRADER )
		nd->u.scr.trader = true;
	nd->u.scr.shop = NULL;
	++npc->npc_script;
	npc->add_to_location(nd);

	//-----------------------------------------
	// Loop through labels to export them as necessary
	for (i = 0; i < nd->u.scr.label_list_num; i++) {
		if (npc->event_export(nd, i)) {
			ShowWarning("npc_parse_script: duplicate event %s::%s in file '%s'.\n",
			             nd->exname, nd->u.scr.label_list[i].name, filepath);
			if (retval) *retval = EXIT_FAILURE;
		}
		npc->timerevent_export(nd, i);
	}

	nd->u.scr.timerid = INVALID_TIMER;

	if( options&NPO_ONINIT ) {
		char evname[EVENT_NAME_LENGTH];
		struct event_data *ev;

		snprintf(evname, ARRAYLENGTH(evname), "%s::OnInit", nd->exname);

		if( ( ev = (struct event_data*)strdb_get(npc->ev_db, evname) ) ) {

			//Execute OnInit
			script->run_npc(nd->u.scr.script,ev->pos,0,nd->bl.id);

		}
	}

	return end;
}

/**
 * Registers the NPC and adds it to its location (on map or floating).
 *
 * @param nd The NPC to register.
 */
static void npc_add_to_location(struct npc_data *nd)
{
	nullpo_retv(nd);

	if (nd->bl.m >= 0) {
		map->addnpc(nd->bl.m, nd);
		npc->setcells(nd);
		map->addblock(&nd->bl);
		nd->ud = &npc->base_ud;
		if (nd->class_ >= 0) {
			status->set_viewdata(&nd->bl, nd->class_);
			if( map->list[nd->bl.m].users )
				clif->spawn(&nd->bl);
		}
	} else {
		// we skip map->addnpc, but still add it to the list of ID's
		map->addiddb(&nd->bl);
	}
	strdb_put(npc->name_db, nd->exname, nd);
}

/**
 * Duplicates a script (@see npc_duplicate_sub)
 */
static bool npc_duplicate_script_sub(struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options)
{
	int i;
	bool retval = true;

	nullpo_retr(false, nd);
	nullpo_retr(false, snd);

	++npc->npc_script;
	nd->u.scr.xs = xs;
	nd->u.scr.ys = ys;
	nd->u.scr.script = snd->u.scr.script;
	nd->u.scr.label_list = snd->u.scr.label_list;
	nd->u.scr.label_list_num = snd->u.scr.label_list_num;
	nd->u.scr.shop = snd->u.scr.shop;
	nd->u.scr.trader = snd->u.scr.trader;

	//add the npc to its location
	npc->add_to_location(nd);

	// Loop through labels to export them as necessary
	for (i = 0; i < nd->u.scr.label_list_num; i++) {
		if (npc->event_export(nd, i)) {
			ShowWarning("npc_parse_duplicate: duplicate event %s::%s in file '%s'.\n",
			             nd->exname, nd->u.scr.label_list[i].name, nd->path);
			retval = false;
		}
		npc->timerevent_export(nd, i);
	}

	nd->u.scr.timerid = INVALID_TIMER;

	if (options&NPO_ONINIT) {
		// From npc_parse_script
		char evname[EVENT_NAME_LENGTH];
		struct event_data *ev;

		snprintf(evname, ARRAYLENGTH(evname), "%s::OnInit", nd->exname);

		if ((ev = (struct event_data*)strdb_get(npc->ev_db, evname)) != NULL) {
			//Execute OnInit
			script->run_npc(nd->u.scr.script,ev->pos,0,nd->bl.id);
		}
	}
	return retval;
}

/**
 * Duplicates a shop or cash shop (@see npc_duplicate_sub)
 */
static bool npc_duplicate_shop_sub(struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options)
{
	nullpo_retr(false, nd);
	nullpo_retr(false, snd);

	++npc->npc_shop;
	nd->u.shop.shop_item = snd->u.shop.shop_item;
	nd->u.shop.count = snd->u.shop.count;

	//add the npc to its location
	npc->add_to_location(nd);

	return true;
}

/**
 * Duplicates a warp (@see npc_duplicate_sub)
 */
static bool npc_duplicate_warp_sub(struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options)
{
	nullpo_retr(false, nd);
	nullpo_retr(false, snd);

	++npc->npc_warp;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;
	nd->u.warp.mapindex = snd->u.warp.mapindex;
	nd->u.warp.x = snd->u.warp.x;
	nd->u.warp.y = snd->u.warp.y;

	//Add the npc to its location
	npc->add_to_location(nd);

	return true;
}

/**
 * Duplicates a warp, shop, cashshop or script.
 *
 * @param nd      An already initialized NPC data. Expects bl->m, bl->x, bl->y,
 *                name, exname, path, dir, class_, speed, subtype to be already
 *                filled.
 * @param snd     The source NPC to duplicate.
 * @param class_  The npc view class.
 * @param dir     The facing direction.
 * @param xs      The x-span, if any.
 * @param ys      The y-span, if any.
 * @param options The NPC options.
 * @retval false if there were any issues while creating and validating the NPC.
 */
static bool npc_duplicate_sub(struct npc_data *nd, const struct npc_data *snd, int xs, int ys, int options)
{
	nullpo_retr(false, nd);
	nullpo_retr(false, snd);

	nd->src_id = snd->bl.id;
	switch (nd->subtype) {
		case SCRIPT:
			return npc->duplicate_script_sub(nd, snd, xs, ys, options);

		case SHOP:
		case CASHSHOP:
			return npc->duplicate_shop_sub(nd, snd, xs, ys, options);

		case WARP:
			return npc->duplicate_warp_sub(nd, snd, xs, ys, options);

		case TOMB:
			return false;
	}
	return false;
}

/**
 * Parses a duplicate warp, shop, cashshop or script NPC. [Orcao]
 *
 * Example:
 * @code
 * warp:
 *   <map name>,<x>,<y>,<facing><TAB>duplicate(<name of target>)<TAB><NPC Name><TAB><spanx>,<spany>
 * shop/cashshop/npc:
 *   -<TAB>duplicate(<name of target>)<TAB><NPC Name><TAB><sprite id>
 * shop/cashshop/npc:
 *   <map name>,<x>,<y>,<facing><TAB>duplicate(<name of target>)<TAB><NPC Name><TAB><sprite id>
 * npc:
 *   -<TAB>duplicate(<name of target>)<TAB><NPC Name><TAB><sprite id>,<triggerX>,<triggerY>
 * npc:
 *   <map name>,<x>,<y>,<facing><TAB>duplicate(<name of target>)<TAB><NPC Name><TAB><sprite id>,<triggerX>,<triggerY>
 * @endcode
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[in]  options  NPC parse/load options (@see enum npc_parse_options).
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 *
 * @remark
 *   Only `NPO_ONINIT` is available trough the options argument.
 */
static const char *npc_parse_duplicate(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int options, int *retval)
{
	int x, y, dir, m, xs = -1, ys = -1;
	char srcname[128];
	const char *end;
	size_t length;

	int class_;
	struct npc_data* nd;
	struct npc_data* dnd;

	end = strchr(start,'\n');
	nullpo_retr(end, w2);
	nullpo_retr(end, w4);
	length = strlen(w2);

	// get the npc being duplicated
	if( w2[length-1] != ')' || length <= 11 || length-11 >= sizeof(srcname) )
	{// does not match 'duplicate(%127s)', name is empty or too long
		ShowError("npc_parse_script: bad duplicate name in file '%s', line '%d': %s\n", filepath, strline(buffer,start-buffer), w2);
		if (retval) *retval = EXIT_FAILURE;
		return end;// next line, try to continue
	}
	safestrncpy(srcname, w2+10, length-10);

	dnd = npc->name2id(srcname);
	if( dnd == NULL) {
		ShowError("npc_parse_script: original npc not found for duplicate in file '%s', line '%d': %s\n", filepath, strline(buffer,start-buffer), srcname);
		if (retval) *retval = EXIT_FAILURE;
		return end;// next line, try to continue
	}

	// get placement
	if ((dnd->subtype==SHOP || dnd->subtype==CASHSHOP || dnd->subtype==SCRIPT) && strcmp(w1, "-") == 0) {
		// floating shop/chashshop/script
		x = y = dir = 0;
		m = -1;
	} else {
		char mapname[32];
		int fields = sscanf(w1, "%31[^,],%d,%d,%d", mapname, &x, &y, &dir);
		if (dnd->subtype == WARP && fields == 3) {
			// <map name>,<x>,<y>
			dir = 0;
		} else if (fields != 4) {
			// <map name>,<x>,<y>,<facing>
			ShowError("npc_parse_duplicate: Invalid placement format for duplicate in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			if (retval) *retval = EXIT_FAILURE;
			return end;// next line, try to continue
		}
		if (dir < 0 || dir > 7) {
			ShowError("npc_parse_duplicate: Invalid NPC facing direction '%d' in file '%s', line '%d'.\n", dir, filepath, strline(buffer, start-buffer));
			if (retval) *retval = EXIT_FAILURE;
			return end; // try next
		}
		m = map->mapname2mapid(mapname);
	}

	if( m != -1 && ( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) ) {
		ShowError("npc_parse_duplicate: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d, in file '%s', line '%d'\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys,filepath,strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return end;//try next
	}

	if (dnd->subtype == WARP && sscanf(w4, "%d,%d", &xs, &ys) == 2) { // <spanx>,<spany>
		;
	} else if (dnd->subtype == SCRIPT && sscanf(w4, "%*[^,],%d,%d", &xs, &ys) == 2) { // <sprite id>,<triggerX>,<triggerY>
		;
	} else if (dnd->subtype == WARP) {
		ShowError("npc_parse_duplicate: Invalid span format for duplicate warp in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return end;// next line, try to continue
	}

	class_ = m == -1 ? FAKE_NPC : npc->parseview(w4, start, buffer, filepath);
	nd = npc->create_npc(dnd->subtype, m, x, y, dir, class_);
	npc->parsename(nd, w3, start, buffer, filepath);
	nd->path = npc->retainpathreference(filepath);
	if (!npc->duplicate_sub(nd, dnd, xs, ys, options)) {
		if (retval) *retval = EXIT_FAILURE;
	}

	return end;
}

/**
 * Duplicates an NPC for instancing purposes.
 *
 * @param snd The NPC to duplicate.
 * @param m   The instanced map ID.
 * @return success state.
 * @retval 0 in case of successful creation.
 */
static int npc_duplicate4instance(struct npc_data *snd, int16 m)
{
	char newname[NAME_LENGTH];
	int dm = -1, im = -1, xs = -1, ys = -1;
	struct npc_data *nd = NULL;

	if (m == -1)
		return 1;

	Assert_retr(1, m >= 0 && m < map->count);
	nullpo_retr(1, snd);

	if (map->list[m].instance_id == -1)
		return 1;

	snprintf(newname, ARRAYLENGTH(newname), "dup_%d_%d", map->list[m].instance_id, snd->bl.id);
	if( npc->name2id(newname) != NULL ) { // Name already in use
		ShowError("npc_duplicate4instance: the npcname (%s) is already in use while trying to duplicate npc %s in instance %d.\n", newname, snd->exname, map->list[m].instance_id);
		return 1;
	}

	switch (snd->subtype) {
	case SCRIPT:
		xs = snd->u.scr.xs;
		ys = snd->u.scr.ys;
		break;
	case WARP:
		xs = snd->u.warp.xs;
		ys = snd->u.warp.ys;
		// Adjust destination, if instanced
		if ((dm = map->mapindex2mapid(snd->u.warp.mapindex)) < 0) {
			return 1;
		}
		if ((im = instance->mapid2imapid(dm, map->list[m].instance_id)) == -1) {
			ShowError("npc_duplicate4instance: warp (%s) leading to instanced map (%s), but instance map is not attached to current instance.\n", map->list[dm].name, snd->exname);
			return 1;
		}
		break;
	default: // Other types have no xs/ys
		break;
	}

	nd = npc->create_npc(snd->subtype, m, snd->bl.x, snd->bl.y, snd->dir, snd->class_);
	safestrncpy(nd->name, snd->name, sizeof(nd->name));
	safestrncpy(nd->exname, newname, sizeof(nd->exname));
	nd->path = npc->retainpathreference("INSTANCING");
	npc->duplicate_sub(nd, snd, xs, ys, NPO_NONE);
	if (nd->subtype == WARP) {
		// Adjust destination, if instanced
		nd->u.warp.mapindex = map_id2index(im);
	}

	return 0;
}

//Set mapcell CELL_NPC to trigger event later
static void npc_setcells(struct npc_data *nd)
{
	int16 m, x, y, xs, ys;
	int i,j;

	nullpo_retv(nd);
	m = nd->bl.m;
	x = nd->bl.x;
	y = nd->bl.y;
	switch(nd->subtype) {
		case WARP:
			xs = nd->u.warp.xs;
			ys = nd->u.warp.ys;
			break;
		case SCRIPT:
			xs = nd->u.scr.xs;
			ys = nd->u.scr.ys;
			break;
		default:
			return; // Other types doesn't have touch area
	}

	if (m < 0 || xs < 0 || ys < 0 || map->list[m].cell == (struct mapcell *)0xdeadbeaf) //invalid range or map
		return;

	for (i = y-ys; i <= y+ys; i++) {
		for (j = x-xs; j <= x+xs; j++) {
			if (map->getcell(m, &nd->bl, j, i, CELL_CHKNOPASS))
				continue;
			map->list[m].setcell(m, j, i, CELL_NPC, true);
		}
	}
}

static int npc_unsetcells_sub(struct block_list *bl, va_list ap)
{
	struct npc_data *nd = NULL;
	int id = va_arg(ap, int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_NPC);
	nd = BL_UCAST(BL_NPC, bl);

	if (nd->bl.id == id)
		return 0;
	npc->setcells(nd);
	return 1;
}

static void npc_unsetcells(struct npc_data *nd)
{
	int16 m, x, y, xs, ys;
	int i,j, x0, x1, y0, y1;

	nullpo_retv(nd);
	m = nd->bl.m;
	x = nd->bl.x;
	y = nd->bl.y;
	switch(nd->subtype) {
		case WARP:
			xs = nd->u.warp.xs;
			ys = nd->u.warp.ys;
			break;
		case SCRIPT:
			xs = nd->u.scr.xs;
			ys = nd->u.scr.ys;
			break;
		default:
			return; // Other types doesn't have touch area
	}

	if (m < 0 || xs < 0 || ys < 0 || map->list[m].cell == (struct mapcell *)0xdeadbeaf)
		return;

	//Locate max range on which we can locate npc cells
	//FIXME: does this really do what it's supposed to do? [ultramage]
	for(x0 = x-xs; x0 > 0 && map->getcell(m, &nd->bl, x0, y, CELL_CHKNPC); x0--);
	for(x1 = x+xs; x1 < map->list[m].xs-1 && map->getcell(m, &nd->bl, x1, y, CELL_CHKNPC); x1++);
	for(y0 = y-ys; y0 > 0 && map->getcell(m, &nd->bl, x, y0, CELL_CHKNPC); y0--);
	for(y1 = y+ys; y1 < map->list[m].ys-1 && map->getcell(m, &nd->bl, x, y1, CELL_CHKNPC); y1++);

	//Erase this npc's cells
	for (i = y-ys; i <= y+ys; i++)
		for (j = x-xs; j <= x+xs; j++)
			map->list[m].setcell(m, j, i, CELL_NPC, false);

	//Re-deploy NPC cells for other nearby npcs.
	map->foreachinarea( npc->unsetcells_sub, m, x0, y0, x1, y1, BL_NPC, nd->bl.id );
}

static void npc_movenpc(struct npc_data *nd, int16 x, int16 y)
{
	int16 m;
	nullpo_retv(nd);
	m = nd->bl.m;
	if (m < 0 || nd->bl.prev == NULL) return; //Not on a map.

	x = cap_value(x, 0, map->list[m].xs-1);
	y = cap_value(y, 0, map->list[m].ys-1);

	map->foreachinrange(clif->outsight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
	map->moveblock(&nd->bl, x, y, timer->gettick());
	map->foreachinrange(clif->insight, &nd->bl, AREA_SIZE, BL_PC, &nd->bl);
}

/// Changes the display name of the npc.
///
/// @param nd Target npc
/// @param newname New display name
static void npc_setdisplayname(struct npc_data *nd, const char *newname)
{
	nullpo_retv(nd);
	nullpo_retv(newname);

	safestrncpy(nd->name, newname, sizeof(nd->name));
	if( map->list[nd->bl.m].users )
		clif->blname_ack(0, &nd->bl);
}

/// Changes the display class of the npc.
///
/// @param nd Target npc
/// @param class_ New display class
static void npc_setclass(struct npc_data *nd, int class_)
{
	nullpo_retv(nd);

	if( nd->class_ == class_ )
		return;

	if( map->list[nd->bl.m].users )
		clif->clearunit_area(&nd->bl, CLR_OUTSIGHT);// fade out
	nd->class_ = class_;
	status->set_viewdata(&nd->bl, class_);
	if( map->list[nd->bl.m].users )
		clif->spawn(&nd->bl);// fade in
}

static void npc_refresh(struct npc_data *nd)
{
	nullpo_retv(nd);

	if (map->list[nd->bl.m].users) {
		// using here CLR_TRICKDEAD because other flags show effects.
		// probably need use other flag or other way to refresh npc.
		clif->clearunit_area(&nd->bl, CLR_TRICKDEAD); // fade out
		clif->spawn(&nd->bl); // fade in
	}
}

// @commands (script based)
static int npc_do_atcmd_event(struct map_session_data *sd, const char *command, const char *message, const char *eventname)
{
	struct event_data* ev = (struct event_data*)strdb_get(npc->ev_db, eventname);
	struct npc_data *nd;
	struct script_state *st;
	int i = 0, nargs = 0;
	size_t len;

	nullpo_ret(sd);
	nullpo_ret(message);

	if( ev == NULL || (nd = ev->nd) == NULL ) {
		ShowError("npc_event: event not found [%s]\n", eventname);
		return 0;
	}

	if( sd->npc_id != 0 ) { // Enqueue the event trigger.
		ARR_FIND( 0, MAX_EVENTQUEUE, i, sd->eventqueue[i][0] == '\0' );
		if( i < MAX_EVENTQUEUE ) {
			safestrncpy(sd->eventqueue[i],eventname,50); //Event enqueued.
			return 0;
		}

		ShowWarning("npc_event: player's event queue is full, can't add event '%s' !\n", eventname);
		return 1;
	}

	if( ev->nd->option&OPTION_INVISIBLE ) { // Disabled npc, shouldn't trigger event.
		npc->event_dequeue(sd);
		return 2;
	}

	st = script->alloc_state(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	script->setd_sub(st, NULL, ".@atcmd_command$", 0, command, NULL);

	len = strlen(message);
	if (len) {
		char *temp, *p;
		p = temp = aStrdup(message);
		// Sanity check - Skip leading spaces (shouldn't happen)
		while (i <= len && temp[i] == ' ') {
			p++;
			i++;
		}
		// split atcmd parameters based on spaces
		while (i <= len) {
			if (temp[i] != ' ' && temp[i] != '\0') {
				i++;
				continue;
			}
			temp[i] = '\0';
			script->setd_sub(st, NULL, ".@atcmd_parameters$", nargs++, (void *)p, NULL);
			i++;
			p = temp + i;
		}
		aFree(temp);
	}
	script->setd_sub(st, NULL, ".@atcmd_numparameters", 0, (void *)h64BPTRSIZE(nargs), NULL);

	script->run_main(st);
	return 0;
}

/**
 * Parses a function.
 *
 * Example:
 * @code
 * function<TAB>script<TAB><function name><TAB>{
 *   <code>
 * }
 * @endcode
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_function(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	struct DBMap *func_db;
	struct DBData old_data;
	struct script_code *scriptroot;
	const char* end;
	const char* script_start;

	nullpo_retr(NULL, w1);
	nullpo_retr(NULL, w2);
	nullpo_retr(NULL, w3);
	nullpo_retr(NULL, w4);
	nullpo_retr(NULL, start);
	nullpo_retr(NULL, retval);

	script_start = strstr(start,"\t{");
	end = strchr(start,'\n');
	if( *w4 != '{' || script_start == NULL || (end != NULL && script_start > end) ) {
		ShowError("npc_parse_function: Missing left curly '%%TAB%%{' in file '%s', line '%d'. Skipping the rest of the file.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return NULL;// can't continue
	}
	++script_start;

	end = npc->skip_script(script_start,buffer,filepath, retval);
	if( end == NULL )
		return NULL;// (simple) parse error, don't continue

	script->parser_current_npc_name = w3;

	scriptroot = script->parse(script_start, filepath, strline(buffer,start-buffer), SCRIPT_RETURN_EMPTY_SCRIPT, retval);

	script->parser_current_npc_name = NULL;

	if( scriptroot == NULL )// parse error, continue
		return end;

	func_db = script->userfunc_db;
	if (func_db->put(func_db, DB->str2key(w3), DB->ptr2data(scriptroot), &old_data)) {
		struct script_code *oldscript = (struct script_code*)DB->data2ptr(&old_data);
		ShowWarning("npc_parse_function: Overwriting user function [%s] in file '%s', line '%d'.\n", w3, filepath, strline(buffer,start-buffer));
		script->free_vars(oldscript->local.vars);
		VECTOR_CLEAR(oldscript->script_buf);
		aFree(oldscript);
	}

	return end;
}

/*==========================================
 * Parse Mob 1 - Parse mob list into each map
 * Parse Mob 2 - Actually Spawns Mob
 * [Wizputer]
 *------------------------------------------*/
static void npc_parse_mob2(struct spawn_data *mobspawn)
{
	int i;

	nullpo_retv(mobspawn);
	for( i = mobspawn->active; i < mobspawn->num; ++i ) {
		struct mob_data* md = mob->spawn_dataset(mobspawn);
		md->spawn = mobspawn;
		md->spawn->active++;
		mob->spawn(md);
	}
}

/**
 * Parses a mob permanent spawn.
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_mob(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	int num, class_, m,x,y,xs,ys, i,j;
	int mob_lv = -1, ai = -1, size = -1;
	char mapname[32], mobname[NAME_LENGTH];
	struct spawn_data mobspawn, *data;
	struct mob_db* db;

	nullpo_retr(strchr(start,'\n'), w1);
	nullpo_retr(strchr(start,'\n'), w2);
	nullpo_retr(strchr(start,'\n'), w3);
	nullpo_retr(strchr(start,'\n'), w4);

	memset(&mobspawn, 0, sizeof(struct spawn_data));

	if (strcmp(w2, "boss_monster") == 0)
		mobspawn.state.boss = BTYPE_MVP;
	else if (strcmp(w2, "miniboss_monster") == 0)
		mobspawn.state.boss = BTYPE_BOSS;
	else
		mobspawn.state.boss = BTYPE_NONE;

	// w1=<map name>,<x>,<y>,<xs>,<ys>
	// w3=<mob name>{,<mob level>}
	// w4=<mob id>,<amount>,<delay1>,<delay2>{,<event>,<mob size>,<mob ai>}
	if( sscanf(w1, "%31[^,],%d,%d,%d,%d", mapname, &x, &y, &xs, &ys) < 5
	 || sscanf(w3, "%23[^,],%d", mobname, &mob_lv) < 1
	 || sscanf(w4, "%d,%d,%u,%u,%50[^,],%d,%d[^\t\r\n]", &class_, &num, &mobspawn.delay1, &mobspawn.delay2, mobspawn.eventname, &size, &ai) < 4
	 ) {
		ShowError("npc_parse_mob: Invalid mob definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}
	if( mapindex->name2id(mapname) == 0 ) {
		ShowError("npc_parse_mob: Unknown map '%s' in file '%s', line '%d'.\n", mapname, filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}
	m =  map->mapname2mapid(mapname);
	if( m < 0 )//Not loaded on this map-server instance.
		return strchr(start,'\n');// skip and continue
	mobspawn.m = (unsigned short)m;

	if( x < 0 || x >= map->list[mobspawn.m].xs || y < 0 || y >= map->list[mobspawn.m].ys ) {
		ShowError("npc_parse_mob: Spawn coordinates out of range: %s (%d,%d), map size is (%d,%d) - %s %s in file '%s', line '%d'.\n", map->list[mobspawn.m].name, x, y, (map->list[mobspawn.m].xs-1), (map->list[mobspawn.m].ys-1), w1, w3, filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	// check monster ID if exists!
	if( mob->db_checkid(class_) == 0 ) {
		ShowError("npc_parse_mob: Unknown mob ID %d in file '%s', line '%d'.\n", class_, filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	if( num < 1 || num > 1000 ) {
		ShowError("npc_parse_mob: Invalid number of monsters %d, must be inside the range [1,1000] in file '%s', line '%d'.\n", num, filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	if (mobspawn.state.size > 2 && size != -1) {
		ShowError("npc_parse_mob: Invalid size number %d for mob ID %d in file '%s', line '%d'.\n", mobspawn.state.size, class_, filepath, strline(buffer, start - buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start, '\n');
	}

	if (mobspawn.state.ai >= AI_MAX && ai != -1) {
		ShowError("npc_parse_mob: Invalid ai %d for mob ID %d in file '%s', line '%d'.\n", mobspawn.state.ai, class_, filepath, strline(buffer, start - buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start, '\n');
	}

	if( (mob_lv == 0 || mob_lv > MAX_LEVEL) && mob_lv != -1 ) {
		ShowError("npc_parse_mob: Invalid level %d for mob ID %d in file '%s', line '%d'.\n", mob_lv, class_, filepath, strline(buffer, start - buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start, '\n');
	}

	mobspawn.num = (unsigned short)num;
	mobspawn.active = 0;
	mobspawn.class_ = class_;
	mobspawn.x = (unsigned short)x;
	mobspawn.y = (unsigned short)y;
	mobspawn.xs = (signed short)xs;
	mobspawn.ys = (signed short)ys;
	if (mob_lv > 0 && mob_lv <= MAX_LEVEL)
		mobspawn.level = mob_lv;
	if (size > 0 && size <= 2)
		mobspawn.state.size = size;
	if (ai > AI_NONE && ai < AI_MAX)
		mobspawn.state.ai = ai;

	if (mobspawn.num > 1 && battle_config.mob_count_rate != 100) {
		if ((mobspawn.num = mobspawn.num * battle_config.mob_count_rate / 100) < 1)
			mobspawn.num = 1;
	}

	if (battle_config.force_random_spawn || (mobspawn.x == 0 && mobspawn.y == 0)) {
		//Force a random spawn anywhere on the map.
		mobspawn.x = mobspawn.y = 0;
		mobspawn.xs = mobspawn.ys = -1;
	}

	if(mobspawn.delay1>0xfffffff || mobspawn.delay2>0xfffffff) {
		ShowError("npc_parse_mob: Invalid spawn delays %u %u in file '%s', line '%d'.\n", mobspawn.delay1, mobspawn.delay2, filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	//Use db names instead of the spawn file ones.
	if (battle_config.override_mob_names == 1)
		strcpy(mobspawn.name, DEFAULT_MOB_NAME);
	else if (battle_config.override_mob_names == 2)
		strcpy(mobspawn.name, DEFAULT_MOB_JNAME);
	else
		safestrncpy(mobspawn.name, mobname, sizeof(mobspawn.name));

	//Verify dataset.
	if( !mob->parse_dataset(&mobspawn) ) {
		ShowError("npc_parse_mob: Invalid dataset for monster ID %d in file '%s', line '%d'.\n", class_, filepath, strline(buffer,start-buffer));
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	//Update mob spawn lookup database
	db = mob->db(class_);
	for( i = 0; i < ARRAYLENGTH(db->spawn); ++i ) {
		if (map_id2index(mobspawn.m) == db->spawn[i].mapindex) {
			//Update total
			db->spawn[i].qty += mobspawn.num;
			//Re-sort list
			for( j = i; j > 0 && db->spawn[j-1].qty < db->spawn[i].qty; --j );
			if( j != i ) {
				xs = db->spawn[i].mapindex;
				ys = db->spawn[i].qty;
				memmove(&db->spawn[j+1], &db->spawn[j], (i-j)*sizeof(db->spawn[0]));
				db->spawn[j].mapindex = xs;
				db->spawn[j].qty = ys;
			}
			break;
		}
		if (mobspawn.num > db->spawn[i].qty) {
			//Insert into list
			if( i != ARRAYLENGTH(db->spawn) - 1 )
				memmove(&db->spawn[i+1], &db->spawn[i], sizeof(db->spawn) -(i+1)*sizeof(db->spawn[0]));
			db->spawn[i].mapindex = map_id2index(mobspawn.m);
			db->spawn[i].qty = mobspawn.num;
			break;
		}
	}

	//Now that all has been validated. We allocate the actual memory that the re-spawn data will use.
	data = (struct spawn_data*)aMalloc(sizeof(struct spawn_data));
	memcpy(data, &mobspawn, sizeof(struct spawn_data));

	// spawn / cache the new mobs
	if( battle_config.dynamic_mobs && map->addmobtolist(data->m, data) >= 0 ) {
		data->state.dynamic = true;
		npc->npc_cache_mob += data->num;

		// check if target map has players
		// (usually shouldn't occur when map server is just starting,
		// but not the case when we do @reloadscript
		if( map->list[data->m].users > 0 ) {
			npc->parse_mob2(data);
		}
	} else {
		data->state.dynamic = false;
		npc->parse_mob2(data);
		npc->npc_delay_mob += data->num;
	}

	npc->npc_mob++;

	return strchr(start,'\n');// continue
}

/**
 * Parses an unknown mapflag.
 *
 * The purpose of this function is to be an override or hooking point for plugins.
 *
 * @see npc_parse_mapflag
 */
static void npc_parse_unknown_mapflag(const char *name, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	ShowError("npc_parse_mapflag: unrecognized mapflag '%s' in file '%s', line '%d'.\n", w3, filepath, strline(buffer,start-buffer));
	if (retval)
		*retval = EXIT_FAILURE;
}

/**
 * Parses a mapflag and sets or disables it on a map.
 *
 * @remark
 *   This also checks if the mapflag conflicts with another.
 *
 * Example:
 * @code
 *   bat_c01<TAB>mapflag<TAB>battleground<TAB>2
 * @endcode
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_mapflag(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	int16 m;
	char mapname[32];
	int state = 1;

	nullpo_retr(strchr(start,'\n'), w1);
	nullpo_retr(strchr(start,'\n'), w3);

	// w1=<mapname>
	if( sscanf(w1, "%31[^,]", mapname) != 1 )
	{
		ShowError("npc_parse_mapflag: Invalid mapflag definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}
	m = map->mapname2mapid(mapname);
	if (m < 0) {
		ShowWarning("npc_parse_mapflag: Unknown map in file '%s', line '%d': %s\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n",
		            filepath, strline(buffer,start-buffer), mapname, w1, w2, w3, w4);
		if (retval) *retval = EXIT_FAILURE;
		return strchr(start,'\n');// skip and continue
	}

	if (w4 && !strcmpi(w4, "off"))
		state = 0; //Disable mapflag rather than enable it. [Skotlex]

	if (!strcmpi(w3, "nosave")) {
		char savemap[32];
		int savex, savey;
		if (state == 0); //Map flag disabled.
		else if (w4 && !strcmpi(w4, "SavePoint")) {
			map->list[m].save.map = 0;
			map->list[m].save.x = -1;
			map->list[m].save.y = -1;
		} else if (w4 && sscanf(w4, "%31[^,],%d,%d", savemap, &savex, &savey) == 3) {
			map->list[m].save.map = mapindex->name2id(savemap);
			map->list[m].save.x = savex;
			map->list[m].save.y = savey;
			if (!map->list[m].save.map) {
				ShowWarning("npc_parse_mapflag: Specified save point map '%s' for mapflag 'nosave' not found in file '%s', line '%d', using 'SavePoint'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", savemap, filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
				if (retval) *retval = EXIT_FAILURE;
				map->list[m].save.x = -1;
				map->list[m].save.y = -1;
			}
		}
		map->list[m].flag.nosave = state;
	}
	else if (!strcmpi(w3,"autotrade"))
		map->list[m].flag.autotrade=state;
	else if (!strcmpi(w3,"allowks"))
		map->list[m].flag.allowks=state; // [Kill Steal Protection]
	else if (!strcmpi(w3,"town"))
		map->list[m].flag.town=state;
	else if (!strcmpi(w3,"nomemo"))
		map->list[m].flag.nomemo=state;
	else if (!strcmpi(w3,"noteleport"))
		map->list[m].flag.noteleport=state;
	else if (!strcmpi(w3,"nowarp"))
		map->list[m].flag.nowarp=state;
	else if (!strcmpi(w3,"nowarpto"))
		map->list[m].flag.nowarpto=state;
	else if (!strcmpi(w3,"noreturn"))
		map->list[m].flag.noreturn=state;
	else if (!strcmpi(w3,"monster_noteleport"))
		map->list[m].flag.monster_noteleport=state;
	else if (!strcmpi(w3,"nobranch"))
		map->list[m].flag.nobranch=state;
	else if (!strcmpi(w3,"nopenalty")) {
		map->list[m].flag.noexppenalty=state;
		map->list[m].flag.nozenypenalty=state;
	}
	else if (!strcmpi(w3,"pvp")) {
		struct map_zone_data *zone;
		map->list[m].flag.pvp = state;
		if( state && (map->list[m].flag.gvg || map->list[m].flag.gvg_dungeon || map->list[m].flag.gvg_castle) ) {
			map->list[m].flag.gvg = 0;
			map->list[m].flag.gvg_dungeon = 0;
			map->list[m].flag.gvg_castle = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and GvG flags for the same map! Removing GvG flags from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if (state && map->list[m].flag.cvc) {
			map->list[m].flag.cvc = 0;
			ShowWarning("npc_parse_mapflag: You can't set CvC and PvP flags for the same map! Removing CvC flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer, start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if( state && map->list[m].flag.battleground ) {
			map->list[m].flag.battleground = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and BattleGround flags for the same map! Removing BattleGround flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if( state && (zone = strdb_get(map->zone_db, MAP_ZONE_PVP_NAME)) != NULL && map->list[m].zone != zone ) {
			map->zone_change(m,zone,start,buffer,filepath);
		} else if ( !state ) {
			map->list[m].zone = &map->zone_all;
		}
	}
	else if (!strcmpi(w3,"pvp_noparty"))
		map->list[m].flag.pvp_noparty=state;
	else if (!strcmpi(w3,"pvp_noguild"))
		map->list[m].flag.pvp_noguild=state;
	else if (!strcmpi(w3, "pvp_nightmaredrop")) {
		char drop_arg1[16], drop_arg2[16];
		int drop_per = 0;
		if (sscanf(w4, "%15[^,],%15[^,],%d", drop_arg1, drop_arg2, &drop_per) == 3) {
			int drop_id = 0, drop_type = 0;
			if (!strcmpi(drop_arg1, "random"))
				drop_id = -1;
			else if (itemdb->exists((drop_id = atoi(drop_arg1))) == NULL)
				drop_id = 0;
			if (!strcmpi(drop_arg2, "inventory"))
				drop_type = 1;
			else if (!strcmpi(drop_arg2,"equip"))
				drop_type = 2;
			else if (!strcmpi(drop_arg2,"all"))
				drop_type = 3;

			if (drop_id != 0) {
				RECREATE(map->list[m].drop_list, struct map_drop_list, ++map->list[m].drop_list_count);
				map->list[m].drop_list[map->list[m].drop_list_count-1].drop_id = drop_id;
				map->list[m].drop_list[map->list[m].drop_list_count-1].drop_type = drop_type;
				map->list[m].drop_list[map->list[m].drop_list_count-1].drop_per = drop_per;
				map->list[m].flag.pvp_nightmaredrop = 1;
			}
		} else if (!state) //Disable
			map->list[m].flag.pvp_nightmaredrop = 0;
	}
	else if (!strcmpi(w3,"pvp_nocalcrank"))
		map->list[m].flag.pvp_nocalcrank=state;
	else if (!strcmpi(w3,"gvg")) {
		struct map_zone_data *zone;

		map->list[m].flag.gvg = state;
		if( state && map->list[m].flag.pvp ) {
			map->list[m].flag.pvp = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and GvG flags for the same map! Removing PvP flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if (state && map->list[m].flag.cvc) {
			map->list[m].flag.cvc = 0;
			ShowWarning("npc_parse_mapflag: You can't set CvC and GvG flags for the same map! Removing CvC flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer, start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if( state && map->list[m].flag.battleground ) {
			map->list[m].flag.battleground = 0;
			ShowWarning("npc_parse_mapflag: You can't set GvG and BattleGround flags for the same map! Removing BattleGround flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if( state && (zone = strdb_get(map->zone_db, MAP_ZONE_GVG_NAME)) != NULL && map->list[m].zone != zone ) {
			map->zone_change(m,zone,start,buffer,filepath);
		}
	}
	else if (!strcmpi(w3,"gvg_noparty"))
		map->list[m].flag.gvg_noparty=state;
	else if (!strcmpi(w3,"gvg_dungeon")) {
		map->list[m].flag.gvg_dungeon=state;
		if (state) map->list[m].flag.pvp=0;
	}
	else if (!strcmpi(w3,"gvg_castle")) {
		map->list[m].flag.gvg_castle=state;
		if (state) map->list[m].flag.pvp=0;
	}
	else if (!strcmpi(w3,"battleground")) {
		struct map_zone_data *zone;
		if (state) {
			if (w4 && sscanf(w4, "%d", &state) == 1)
				map->list[m].flag.battleground = state;
			else
				map->list[m].flag.battleground = 1; // Default value
		} else {
			map->list[m].flag.battleground = 0;
		}

		if( map->list[m].flag.battleground && map->list[m].flag.pvp ) {
			map->list[m].flag.pvp = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and BattleGround flags for the same map! Removing PvP flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if( map->list[m].flag.battleground && (map->list[m].flag.gvg || map->list[m].flag.gvg_dungeon || map->list[m].flag.gvg_castle) ) {
			map->list[m].flag.gvg = 0;
			map->list[m].flag.gvg_dungeon = 0;
			map->list[m].flag.gvg_castle = 0;
			ShowWarning("npc_parse_mapflag: You can't set GvG and BattleGround flags for the same map! Removing GvG flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}
		if (map->list[m].flag.cvc) {
			map->list[m].flag.cvc = 0;
			ShowWarning("npc_parse_mapflag: You can't set CvC and BattleGround flags for the same map! Removing CvC flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer, start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		}

		if( state && (zone = strdb_get(map->zone_db, MAP_ZONE_BG_NAME)) != NULL && map->list[m].zone != zone ) {
			map->zone_change(m,zone,start,buffer,filepath);
		}
	}
	else if (!strcmpi(w3, "cvc")) {
		struct map_zone_data *zone;

		map->list[m].flag.cvc = state;
		if (state && (map->list[m].flag.gvg || map->list[m].flag.gvg_dungeon || map->list[m].flag.gvg_castle)) {
			map->list[m].flag.gvg = 0;
			map->list[m].flag.gvg_dungeon = 0;
			map->list[m].flag.gvg_castle = 0;
			ShowWarning("npc_parse_mapflag: You can't set GvG and CvC flags for the same map! Removing GvG flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer, start-buffer));
			if (retval) {
				*retval = EXIT_FAILURE;
			}
		}
		if (state && map->list[m].flag.pvp) {
			map->list[m].flag.pvp = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and CvC flags for the same map! Removing PvP flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer, start-buffer));
			if (retval) {
				*retval = EXIT_FAILURE;
			}
		}
		if (state && map->list[m].flag.battleground) {
			map->list[m].flag.battleground = 0;
			ShowWarning("npc_parse_mapflag: You can't set CvC and BattleGround flags for the same map! Removing BattleGround flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer, start-buffer));
			if (retval) {
				*retval = EXIT_FAILURE;
			}
		}
		if (state && (zone = strdb_get(map->zone_db, MAP_ZONE_CVC_NAME)) != NULL && map->list[m].zone != zone) {
			map->zone_change(m, zone, start, buffer, filepath);
		}
	}
	else if (!strcmpi(w3,"noexppenalty"))
		map->list[m].flag.noexppenalty=state;
	else if (!strcmpi(w3,"nozenypenalty"))
		map->list[m].flag.nozenypenalty=state;
	else if (!strcmpi(w3,"notrade"))
		map->list[m].flag.notrade=state;
	else if (!strcmpi(w3,"novending"))
		map->list[m].flag.novending=state;
	else if (!strcmpi(w3,"nodrop"))
		map->list[m].flag.nodrop=state;
	else if (!strcmpi(w3,"noskill"))
		map->list[m].flag.noskill=state;
	else if (!strcmpi(w3,"noicewall"))
		map->list[m].flag.noicewall=state;
	else if (!strcmpi(w3,"snow"))
		map->list[m].flag.snow=state;
	else if (!strcmpi(w3,"clouds"))
		map->list[m].flag.clouds=state;
	else if (!strcmpi(w3,"clouds2"))
		map->list[m].flag.clouds2=state;
	else if (!strcmpi(w3,"fog"))
		map->list[m].flag.fog=state;
	else if (!strcmpi(w3,"fireworks"))
		map->list[m].flag.fireworks=state;
	else if (!strcmpi(w3,"sakura"))
		map->list[m].flag.sakura=state;
	else if (!strcmpi(w3,"leaves"))
		map->list[m].flag.leaves=state;
	else if (!strcmpi(w3,"nightenabled"))
		map->list[m].flag.nightenabled=state;
	else if (!strcmpi(w3,"noexp")) {
		map->list[m].flag.nobaseexp=state;
		map->list[m].flag.nojobexp=state;
	}
	else if (!strcmpi(w3,"nobaseexp"))
		map->list[m].flag.nobaseexp=state;
	else if (!strcmpi(w3,"nojobexp"))
		map->list[m].flag.nojobexp=state;
	else if (!strcmpi(w3,"noloot")) {
		map->list[m].flag.nomobloot=state;
		map->list[m].flag.nomvploot=state;
	}
	else if (!strcmpi(w3,"nomobloot"))
		map->list[m].flag.nomobloot=state;
	else if (!strcmpi(w3,"nomvploot"))
		map->list[m].flag.nomvploot=state;
	else if (!strcmpi(w3,"nocommand")) {
		if (state) {
			if (w4 && sscanf(w4, "%d", &state) == 1)
				map->list[m].nocommand =state;
			else //No level specified, block everyone.
				map->list[m].nocommand =100;
		} else
			map->list[m].nocommand=0;
	}
	else if (!strcmpi(w3,"jexp")) {
		map->list[m].jexp = (state) ? atoi(w4) : 100;
		if( map->list[m].jexp < 0 ) map->list[m].jexp = 100;
		map->list[m].flag.nojobexp = (map->list[m].jexp==0)?1:0;
	}
	else if (!strcmpi(w3,"bexp")) {
		map->list[m].bexp = (state) ? atoi(w4) : 100;
		if( map->list[m].bexp < 0 ) map->list[m].bexp = 100;
		 map->list[m].flag.nobaseexp = (map->list[m].bexp==0)?1:0;
	}
	else if (!strcmpi(w3,"loadevent"))
		map->list[m].flag.loadevent=state;
	else if (!strcmpi(w3,"nochat"))
		map->list[m].flag.nochat=state;
	else if (!strcmpi(w3,"partylock"))
		map->list[m].flag.partylock=state;
	else if (!strcmpi(w3,"guildlock"))
		map->list[m].flag.guildlock=state;
	else if (!strcmpi(w3,"reset"))
		map->list[m].flag.reset=state;
	else if (!strcmpi(w3,"notomb"))
		map->list[m].flag.notomb=state;
	else if (!strcmpi(w3, "noautoloot"))
		map->list[m].flag.noautoloot = state;
	else if (!strcmpi(w3,"adjust_unit_duration")) {
		int skill_id, k;
		char skill_name[MAP_ZONE_MAPFLAG_LENGTH], modifier[MAP_ZONE_MAPFLAG_LENGTH];
		size_t len = w4 ? strlen(w4) : 0;

		modifier[0] = '\0';
		if( w4 )
			memcpy(skill_name, w4, MAP_ZONE_MAPFLAG_LENGTH);

		for(k = 0; k < len; k++) {
			if( skill_name[k] == '\t' ) {
				memcpy(modifier, &skill_name[k+1], len - k);
				skill_name[k] = '\0';
				break;
			}
		}

		if (state == 0); //Map flag disabled.
		else if (modifier[0] == '\0') {
			ShowWarning("npc_parse_mapflag: Missing 5th param for 'adjust_unit_duration' flag! removing flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else if( !( skill_id = skill->name2id(skill_name) ) || !skill->get_unit_id( skill->name2id(skill_name), 0) ) {
			ShowWarning("npc_parse_mapflag: Unknown skill (%s) for 'adjust_unit_duration' flag! removing flag from %s in file '%s', line '%d'.\n",skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else if ( atoi(modifier) < 1 || atoi(modifier) > USHRT_MAX ) {
			ShowWarning("npc_parse_mapflag: Invalid modifier '%d' for skill '%s' for 'adjust_unit_duration' flag! removing flag from %s in file '%s', line '%d'.\n", atoi(modifier), skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else {
			int idx = map->list[m].unit_count;

			ARR_FIND(0, idx, k, map->list[m].units[k]->skill_id == skill_id);

			if( k < idx ) {
				if( atoi(modifier) != 100 )
					map->list[m].units[k]->modifier = (unsigned short)atoi(modifier);
				else { /* remove */
					int cursor = 0;
					aFree(map->list[m].units[k]);
					map->list[m].units[k] = NULL;
					for( k = 0; k < idx; k++ ) {
						if( map->list[m].units[k] == NULL )
							continue;

						map->list[m].units[cursor] = map->list[m].units[k];

						cursor++;
					}
					if( !( map->list[m].unit_count = cursor ) ) {
						aFree(map->list[m].units);
						map->list[m].units = NULL;
					}
				}
			} else if( atoi(modifier) != 100 )  {
				RECREATE(map->list[m].units, struct mapflag_skill_adjust*, ++map->list[m].unit_count);
				CREATE(map->list[m].units[idx],struct mapflag_skill_adjust,1);
				map->list[m].units[idx]->skill_id = (unsigned short)skill_id;
				map->list[m].units[idx]->modifier = (unsigned short)atoi(modifier);
			}
		}
	} else if (!strcmpi(w3,"adjust_skill_damage")) {
		int skill_id, k;
		char skill_name[MAP_ZONE_MAPFLAG_LENGTH], modifier[MAP_ZONE_MAPFLAG_LENGTH];
		size_t len = w4 ? strlen(w4) : 0;

		modifier[0] = '\0';

		if( w4 )
			memcpy(skill_name, w4, MAP_ZONE_MAPFLAG_LENGTH);

		for(k = 0; k < len; k++) {
			if( skill_name[k] == '\t' ) {
				memcpy(modifier, &skill_name[k+1], len - k);
				skill_name[k] = '\0';
				break;
			}
		}

		if (state == 0); //Map flag disabled.
		else if (modifier[0] == '\0') {
			ShowWarning("npc_parse_mapflag: Missing 5th param for 'adjust_skill_damage' flag! removing flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else if( !( skill_id = skill->name2id(skill_name) ) ) {
			ShowWarning("npc_parse_mapflag: Unknown skill (%s) for 'adjust_skill_damage' flag! removing flag from %s in file '%s', line '%d'.\n", skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else if ( atoi(modifier) < 1 || atoi(modifier) > USHRT_MAX ) {
			ShowWarning("npc_parse_mapflag: Invalid modifier '%d' for skill '%s' for 'adjust_skill_damage' flag! removing flag from %s in file '%s', line '%d'.\n", atoi(modifier), skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else {
			int idx = map->list[m].skill_count;

			ARR_FIND(0, idx, k, map->list[m].skills[k]->skill_id == skill_id);

			if( k < idx ) {
				if( atoi(modifier) != 100 )
					map->list[m].skills[k]->modifier = (unsigned short)atoi(modifier);
				else { /* remove */
					int cursor = 0;
					aFree(map->list[m].skills[k]);
					map->list[m].skills[k] = NULL;
					for( k = 0; k < idx; k++ ) {
						if( map->list[m].skills[k] == NULL )
							continue;

						map->list[m].skills[cursor] = map->list[m].skills[k];

						cursor++;
					}
					if( !( map->list[m].skill_count = cursor ) ) {
						aFree(map->list[m].skills);
						map->list[m].skills = NULL;
					}
				}
			} else if( atoi(modifier) != 100 ) {
				RECREATE(map->list[m].skills, struct mapflag_skill_adjust*, ++map->list[m].skill_count);
				CREATE(map->list[m].skills[idx],struct mapflag_skill_adjust,1);
				map->list[m].skills[idx]->skill_id = (unsigned short)skill_id;
				map->list[m].skills[idx]->modifier = (unsigned short)atoi(modifier);
			}
		}
	} else if (!strcmpi(w3,"zone")) {
		struct map_zone_data *zone;

		if( !(zone = strdb_get(map->zone_db, w4)) ) {
			ShowWarning("npc_parse_mapflag: Invalid zone '%s'! removing flag from %s in file '%s', line '%d'.\n", w4, map->list[m].name, filepath, strline(buffer,start-buffer));
			if (retval) *retval = EXIT_FAILURE;
		} else if( map->list[m].zone != zone ) {
			map->zone_change(m,zone,start,buffer,filepath);
		}
	} else if ( !strcmpi(w3,"nomapchannelautojoin") ) {
		map->list[m].flag.chsysnolocalaj = state;
	} else if ( !strcmpi(w3,"invincible_time_inc") ) {
		map->list[m].invincible_time_inc = (state) ? atoi(w4) : 0;
	} else if ( !strcmpi(w3,"noknockback") ) {
		map->list[m].flag.noknockback = state;
	} else if ( !strcmpi(w3,"weapon_damage_rate") ) {
		map->list[m].weapon_damage_rate = (state) ? atoi(w4) : 100;
	} else if ( !strcmpi(w3,"magic_damage_rate") ) {
		map->list[m].magic_damage_rate = (state) ? atoi(w4) : 100;
	} else if ( !strcmpi(w3,"misc_damage_rate") ) {
		map->list[m].misc_damage_rate = (state) ? atoi(w4) : 100;
	} else if ( !strcmpi(w3,"short_damage_rate") ) {
		map->list[m].short_damage_rate = (state) ? atoi(w4) : 100;
	} else if ( !strcmpi(w3,"long_damage_rate") ) {
		map->list[m].long_damage_rate = (state) ? atoi(w4) : 100;
	} else if ( !strcmpi(w3,"src4instance") ) {
		map->list[m].flag.src4instance = (state) ? 1 : 0;
	} else if ( !strcmpi(w3,"nocashshop") ) {
		map->list[m].flag.nocashshop = (state) ? 1 : 0;
	} else if (!strcmpi(w3,"noviewid")) {
		map->list[m].flag.noviewid = (state) ? atoi(w4) : 0;
	} else if (!strcmpi(w3, "pairship_startable")) {
		map->list[m].flag.pairship_startable = (state) ? 1 : 0;
	}  else if (!strcmpi(w3, "pairship_endable")) {
		map->list[m].flag.pairship_endable = (state) ? 1 : 0;
	}  else if (!strcmpi(w3, "nostorage")) {
		map->list[m].flag.nostorage = (state) ? cap_value(atoi(w4), 1, 3) : 0;
	}  else if (!strcmpi(w3, "nogstorage")) {
		map->list[m].flag.nogstorage = (state) ? cap_value(atoi(w4), 1, 3) : 0;
	} else {
		npc->parse_unknown_mapflag(mapname, w3, w4, start, buffer, filepath, retval);
	}

	return strchr(start,'\n');// continue
}

/**
 * Parses an unknown NPC object. This function may be used as override or hooking point by plugins.
 *
 * @param[in]  w1       First tab-delimited part of the string to parse.
 * @param[in]  w2       Second tab-delimited part of the string to parse.
 * @param[in]  w3       Third tab-delimited part of the string to parse.
 * @param[in]  w4       Fourth tab-delimited part of the string to parse.
 * @param[in]  start    Pointer to the beginning of the string inside buffer.
 *                      This must point to the same buffer as `buffer`.
 * @param[in]  buffer   Pointer to the buffer containing the script. For
 *                      single-line mapflags not inside a script, this may be
 *                      an empty (but initialized) single-character buffer.
 * @param[in]  filepath Source file path.
 * @param[out] retval   Pointer to return the success (EXIT_SUCCESS) or failure
 *                      (EXIT_FAILURE) status. May be NULL.
 * @return A pointer to the advanced buffer position.
 */
static const char *npc_parse_unknown_object(const char *w1, const char *w2, const char *w3, const char *w4, const char *start, const char *buffer, const char *filepath, int *retval)
{
	nullpo_retr(start, retval);
	ShowError("npc_parsesrcfile: Unable to parse, probably a missing or extra TAB in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
	start = strchr(start,'\n');// skip and continue
	*retval = EXIT_FAILURE;
	return start;
}

/**
 * Parses a script file and creates NPCs/functions/mapflags/monsters/etc
 * accordingly.
 *
 * @param filepath  File name and path.
 * @param runOnInit Whether the OnInit label should be called.
 * @retval EXIT_SUCCESS if filepath was loaded correctly.
 * @retval EXIT_FAILURE if there were errors/warnings when loading filepath.
 */
static int npc_parsesrcfile(const char *filepath, bool runOnInit)
{
	int success = EXIT_SUCCESS;
	int16 m, x, y;
	int lines = 0;
	FILE* fp;
	size_t len;
	char* buffer;
	const char* p;

	nullpo_retr(EXIT_FAILURE, filepath);

	// read whole file to buffer
	fp = fopen(filepath, "rb");
	if( fp == NULL ) {
		ShowError("npc_parsesrcfile: File not found '%s'.\n", filepath);
		return EXIT_FAILURE;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	buffer = (char*)aMalloc(len+1);
	fseek(fp, 0, SEEK_SET);
	len = fread(buffer, sizeof(char), len, fp);
	buffer[len] = '\0';
	if( ferror(fp) ) {
		ShowError("npc_parsesrcfile: Failed to read file '%s' - %s\n", filepath, strerror(errno));
		aFree(buffer);
		fclose(fp);
		return EXIT_FAILURE;
	}
	fclose(fp);

	if ((unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF) {
		// UTF-8 BOM. This is most likely an error on the user's part, because:
		// - BOM is discouraged in UTF-8, and the only place where you see it is Notepad and such.
		// - It's unlikely that the user wants to use UTF-8 data here, since we don't really support it, nor does the client by default.
		// - If the user really wants to use UTF-8 (instead of latin1, EUC-KR, SJIS, etc), then they can still do it <without BOM>.
		// More info at http://unicode.org/faq/utf_bom.html#bom5 and http://en.wikipedia.org/wiki/Byte_order_mark#UTF-8
		ShowError("npc_parsesrcfile: Detected unsupported UTF-8 BOM in file '%s'. Stopping (please consider using another character set.)\n", filepath);
		aFree(buffer);
		return EXIT_FAILURE;
	}

	// parse buffer
	for( p = script->skip_space(buffer); p && *p ; p = script->skip_space(p) ) {
		int pos[9];
		char w1[2048], w2[2048], w3[2048], w4[2048];
		int i, count;
		lines++;

		// w1<TAB>w2<TAB>w3<TAB>w4
		count = sv->parse(p, (int)(len+buffer-p), 0, '\t', pos, ARRAYLENGTH(pos), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));
		if( count < 0 )
		{
			ShowError("npc_parsesrcfile: Parse error in file '%s', line '%d'. Stopping...\n", filepath, strline(buffer,p-buffer));
			success = EXIT_FAILURE;
			break;
		}
		// fill w1
		if( pos[3]-pos[2] > ARRAYLENGTH(w1)-1 ) {
			ShowWarning("npc_parsesrcfile: w1 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[3]-pos[2], filepath, strline(buffer,p-buffer));
			success = EXIT_FAILURE;
		}
		i = min(pos[3]-pos[2], ARRAYLENGTH(w1)-1);
		memcpy(w1, p+pos[2], i*sizeof(char));
		w1[i] = '\0';
		// fill w2
		if( pos[5]-pos[4] > ARRAYLENGTH(w2)-1 ) {
			ShowWarning("npc_parsesrcfile: w2 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[5]-pos[4], filepath, strline(buffer,p-buffer));
			success = EXIT_FAILURE;
		}
		i = min(pos[5]-pos[4], ARRAYLENGTH(w2)-1);
		memcpy(w2, p+pos[4], i*sizeof(char));
		w2[i] = '\0';
		// fill w3
		if( pos[7]-pos[6] > ARRAYLENGTH(w3)-1 ) {
			ShowWarning("npc_parsesrcfile: w3 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[7]-pos[6], filepath, strline(buffer,p-buffer));
			success = EXIT_FAILURE;
		}
		i = min(pos[7]-pos[6], ARRAYLENGTH(w3)-1);
		memcpy(w3, p+pos[6], i*sizeof(char));
		w3[i] = '\0';
		// fill w4 (to end of line)
		if( pos[1]-pos[8] > ARRAYLENGTH(w4)-1 ) {
			ShowWarning("npc_parsesrcfile: w4 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[1]-pos[8], filepath, strline(buffer,p-buffer));
			success = EXIT_FAILURE;
		}
		if( pos[8] != -1 ) {
			i = min(pos[1]-pos[8], ARRAYLENGTH(w4)-1);
			memcpy(w4, p+pos[8], i*sizeof(char));
			w4[i] = '\0';
		} else {
			w4[0] = '\0';
		}

		if( count < 3 ) {
			// Unknown syntax
			ShowError("npc_parsesrcfile: Unknown syntax in file '%s', line '%d'. Stopping...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
			success = EXIT_FAILURE;
			break;
		}

		if( strcmp(w1,"-") != 0 && strcmp(w1,"function") != 0 )
		{// w1 = <map name>,<x>,<y>,<facing>
			char mapname[MAP_NAME_LENGTH*2];
			x = y = 0;
			sscanf(w1,"%23[^,],%hd,%hd[^,]",mapname,&x,&y);
			if( !mapindex->name2id(mapname) ) {
				// Incorrect map, we must skip the script info...
				ShowError("npc_parsesrcfile: Unknown map '%s' in file '%s', line '%d'. Skipping line...\n", mapname, filepath, strline(buffer,p-buffer));
				success = EXIT_FAILURE;
				if( strcmp(w2,"script") == 0 && count > 3 ) {
					if((p = npc->skip_script(p,buffer,filepath, &success)) == NULL) {
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
			m = map->mapname2mapid(mapname);
			if( m < 0 ) {
				// "mapname" is not assigned to this server, we must skip the script info...
				if( strcmp(w2,"script") == 0 && count > 3 ) {
					if((p = npc->skip_script(p,buffer,filepath, &success)) == NULL) {
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
			if (x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys) {
				ShowError("npc_parsesrcfile: Unknown coordinates ('%d', '%d') for map '%s' in file '%s', line '%d'. Skipping line...\n", x, y, mapname, filepath, strline(buffer,p-buffer));
				success = EXIT_FAILURE;
				if( strcmp(w2,"script") == 0 && count > 3 ) {
					if((p = npc->skip_script(p,buffer,filepath, &success)) == NULL) {
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
		}

		if( strcmp(w2,"mapflag") == 0 && count >= 3 )
		{
			p = npc->parse_mapflag(w1, w2, trim(w3), trim(w4), p, buffer, filepath, &success);
		}
		else if( count == 3 ) {
			ShowError("npc_parsesrcfile: Unable to parse, probably a missing TAB in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
			p = strchr(p,'\n');// skip and continue
			success = EXIT_FAILURE;
		}
		else if( strcmp(w2,"script") == 0 )
		{
			if( strcmp(w1,"function") == 0 ) {
				p = npc->parse_function(w1, w2, w3, w4, p, buffer, filepath, &success);
			} else {
				p = npc->parse_script(w1,w2,w3,w4, p, buffer, filepath,runOnInit?NPO_ONINIT:NPO_NONE, &success);
			}
		}
		else if( strcmp(w2,"trader") == 0 ) {
			p = npc->parse_script(w1,w2,w3,w4, p, buffer, filepath,(runOnInit?NPO_ONINIT:NPO_NONE)|NPO_TRADER, &success);
		}
		else if( strcmp(w2,"warp") == 0 )
		{
			p = npc->parse_warp(w1,w2,w3,w4, p, buffer, filepath, &success);
		}
		else if( (i=0, sscanf(w2,"duplicate%n",&i), (i > 0 && w2[i] == '(')) )
		{
			p = npc->parse_duplicate(w1,w2,w3,w4, p, buffer, filepath, (runOnInit?NPO_ONINIT:NPO_NONE), &success);
		}
		else if (strcmp(w2,"monster") == 0 || strcmp(w2,"boss_monster") == 0 || strcmp(w2,"miniboss_monster") == 0)
		{
			p = npc->parse_mob(w1, w2, w3, w4, p, buffer, filepath, &success);
		}
		else if( (strcmp(w2,"shop") == 0 || strcmp(w2,"cashshop") == 0) )
		{
			p = npc->parse_shop(w1,w2,w3,w4, p, buffer, filepath, &success);
		}
		else
		{
			p = npc->parse_unknown_object(w1, w2, w3, w4, p, buffer, filepath, &success);
		}
	}
	aFree(buffer);

	return success;
}

static int npc_script_event(struct map_session_data *sd, enum npce_event type)
{
	int i;
	if (type == NPCE_MAX)
		return 0;
	if (!sd) {
		ShowError("npc_script_event: NULL sd. Event Type %u\n", type);
		return 0;
	}
	for (i = 0; i<script_event[type].event_count; i++)
		npc->event_sub(sd,script_event[type].event[i],script_event[type].event_name[i]);
	return i;
}

static void npc_read_event_script(void)
{
	int i;
	struct {
		char *name;
		const char *event_name;
	} config[] = {
		{"Login Event",script->config.login_event_name},
		{"Logout Event",script->config.logout_event_name},
		{"Load Map Event",script->config.loadmap_event_name},
		{"Base LV Up Event",script->config.baselvup_event_name},
		{"Job LV Up Event",script->config.joblvup_event_name},
		{"Die Event",script->config.die_event_name},
		{"Kill PC Event",script->config.kill_pc_event_name},
		{"Kill NPC Event",script->config.kill_mob_event_name},
	};

	for (i = 0; i < NPCE_MAX; i++) {
		struct DBIterator *iter;
		union DBKey key;
		struct DBData *data;

		char name[64]="::";
		safestrncpy(name+2,config[i].event_name,62);

		script_event[i].event_count = 0;
		iter = db_iterator(npc->ev_db);
		for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) )
		{
			const char* p = key.str;
			struct event_data* ed = DB->data2ptr(data);
			unsigned char count = script_event[i].event_count;

			if( count >= ARRAYLENGTH(script_event[i].event) )
			{
				ShowWarning("npc_read_event_script: too many occurences of event '%s'!\n", config[i].event_name);
				break;
			}

			if( (p=strchr(p,':')) && strcmp(name,p) == 0 )
			{
				script_event[i].event[count] = ed;
				script_event[i].event_name[count] = key.str;
				script_event[i].event_count++;
#ifdef ENABLE_CASE_CHECK
			} else if( p && strcasecmp(name, p) == 0 ) {
				DeprecationCaseWarning("npc_read_event_script", p, name, config[i].event_name); // TODO
#endif // ENABLE_CASE_CHECK
			}
		}
		dbi_destroy(iter);
	}

	if (battle_config.etc_log) {
		//Print summary.
		for (i = 0; i < NPCE_MAX; i++)
			ShowInfo("%s: %d '%s' events.\n", config[i].name, script_event[i].event_count, config[i].event_name);
	}
}

/**
 * @see DBApply
 */
static int npc_path_db_clear_sub(union DBKey key, struct DBData *data, va_list args)
{
	struct npc_path_data *npd = DB->data2ptr(data);
	nullpo_ret(npd);
	if (npd->path)
		aFree(npd->path);
	return 0;
}

/**
 * @see DBApply
 */
static int npc_ev_label_db_clear_sub(union DBKey key, struct DBData *data, va_list args)
{
	struct linkdb_node **label_linkdb = DB->data2ptr(data);
	linkdb_final(label_linkdb); // linked data (struct event_data*) is freed when clearing ev_db
	return 0;
}

/**
 * Main npc file processing
 * @param npc_min Minimum npc id - used to know how many NPCs were loaded
 **/
static void npc_process_files(int npc_min)
{
	struct npc_src_list *file; // Current file

	ShowStatus("Loading NPCs...\r");
	for( file = npc->src_files; file != NULL; file = file->next ) {
		ShowStatus("Loading NPC file: %s"CL_CLL"\r", file->name);
		if (npc->parsesrcfile(file->name, false) != EXIT_SUCCESS)
			map->retval = EXIT_FAILURE;
	}
	ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:"CL_CLL"\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Warps\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Shops\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Scripts\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Spawn sets\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Mobs Cached\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
		npc->npc_id - npc_min, npc->npc_warp, npc->npc_shop, npc->npc_script, npc->npc_mob, npc->npc_cache_mob, npc->npc_delay_mob);
}

//Clear then reload npcs files
static int npc_reload(void)
{
	int npc_new_min = npc->npc_id;
	struct s_mapiterator* iter;
	struct block_list* bl;

	if (map->retval == EXIT_FAILURE)
		map->retval = EXIT_SUCCESS; // Clear return status in case something failed before.

	/* clear guild flag cache */
	guild->flags_clear();

	npc->path_db->clear(npc->path_db, npc->path_db_clear_sub);

	db_clear(npc->name_db);
	db_clear(npc->ev_db);
	npc->ev_label_db->clear(npc->ev_label_db, npc->ev_label_db_clear_sub);

	npc->npc_last_npd = NULL;
	npc->npc_last_path = NULL;
	npc->npc_last_ref = NULL;

	//Remove all npcs/mobs. [Skotlex]
	iter = mapit_geteachiddb();
	for (bl = mapit->first(iter); mapit->exists(iter); bl = mapit->next(iter)) {
		switch(bl->type) {
			case BL_NPC:
				if( bl->id != npc->fake_nd->bl.id )// don't remove fake_nd
					npc->unload(BL_UCAST(BL_NPC, bl), false);
				break;
			case BL_MOB:
				unit->free(bl,CLR_OUTSIGHT);
				break;
		}
	}
	mapit->free(iter);

	if(battle_config.dynamic_mobs) {// dynamic check by [random]
		int16 m;
		for (m = 0; m < map->count; m++) {
			int16 i;
			for (i = 0; i < MAX_MOB_LIST_PER_MAP; i++) {
				if (map->list[m].moblist[i] != NULL) {
					aFree(map->list[m].moblist[i]);
					map->list[m].moblist[i] = NULL;
				}
				if( map->list[m].mob_delete_timer != INVALID_TIMER )
				{ // Mobs were removed anyway,so delete the timer [Inkfish]
					timer->delete(map->list[m].mob_delete_timer, map->removemobs_timer);
					map->list[m].mob_delete_timer = INVALID_TIMER;
				}
			}
			if (map->list[m].npc_num > 0)
				ShowWarning("npc_reload: %d npcs weren't removed at map %s!\n", map->list[m].npc_num, map->list[m].name);
		}
	}

	// clear mob spawn lookup index
	mob->clear_spawninfo();

	npc->npc_warp = npc->npc_shop = npc->npc_script = 0;
	npc->npc_mob = npc->npc_cache_mob = npc->npc_delay_mob = 0;

	// reset mapflags
	map->zone_reload();
	map->flags_init();

	// Reprocess npc files and reload constants
	itemdb->name_constants();
	clan->set_constants();
	npc_process_files( npc_new_min );

	instance->reload();

	map->zone_init();

	npc->motd = npc->name2id("HerculesMOTD"); /* [Ind/Hercules] */

	//Re-read the NPC Script Events cache.
	npc->read_event_script();

	// Execute main initialisation events
	// The correct initialisation order is:
	// OnInit -> OnInterIfInit -> OnInterIfInitOnce -> OnAgitInit -> OnAgitInit2
	npc->event_do_oninit( true );
	npc->market_fromsql();
	npc->barter_fromsql();
	// Execute rest of the startup events if connected to char-server. [Lance]
	// Executed when connection is established with char-server in chrif_connectack
	if( !intif->CheckForCharServer() ) {
		ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc->event_doall("OnInterIfInit"));
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc->event_doall("OnInterIfInitOnce"));
	}
	// Refresh guild castle flags on both woe setups
	// These events are only executed after receiving castle information from char-server
	npc->event_doall("OnAgitInit");
	npc->event_doall("OnAgitInit2");

	return 0;
}

//Unload all npc in the given file
static bool npc_unloadfile(const char *filepath)
{
	struct DBIterator *iter = db_iterator(npc->name_db);
	struct npc_data* nd = NULL;
	bool found = false;

	nullpo_retr(false, filepath);

	for( nd = dbi_first(iter); dbi_exists(iter); nd = dbi_next(iter) ) {
		if( nd->path && strcasecmp(nd->path,filepath) == 0 ) { // FIXME: This can break in case-sensitive file systems
			found = true;
			npc->unload_duplicates(nd);/* unload any npcs which could duplicate this but be in a different file */
			npc->unload(nd, true);
		}
	}

	dbi_destroy(iter);

	if( found ) /* refresh event cache */
		npc->read_event_script();

	return found;
}

static void do_clear_npc(void)
{
	db_clear(npc->name_db);
	db_clear(npc->ev_db);
	npc->ev_label_db->clear(npc->ev_label_db, npc->ev_label_db_clear_sub);
}

/*==========================================
 * Destructor
 *------------------------------------------*/
static int do_final_npc(void)
{
	db_destroy(npc->ev_db);
	npc->ev_label_db->destroy(npc->ev_label_db, npc->ev_label_db_clear_sub);
	db_destroy(npc->name_db);
	npc->path_db->destroy(npc->path_db, npc->path_db_clear_sub);
	ers_destroy(npc->timer_event_ers);
	npc->clearsrcfile();

	return 0;
}

static void npc_debug_warps_sub(struct npc_data *nd)
{
	int16 m;

	nullpo_retv(nd);

	if (nd->bl.type != BL_NPC || nd->subtype != WARP || nd->bl.m < 0)
		return;

	m = map->mapindex2mapid(nd->u.warp.mapindex);
	if (m < 0) return; //Warps to another map, nothing to do about it.
	if (nd->u.warp.x == 0 && nd->u.warp.y == 0) return; // random warp

	if (map->getcell(m, &nd->bl, nd->u.warp.x, nd->u.warp.y, CELL_CHKNPC)) {
		ShowWarning("Warp %s at %s(%d,%d) warps directly on top of an area npc at %s(%d,%d)\n",
			nd->name,
			map->list[nd->bl.m].name, nd->bl.x, nd->bl.y,
			map->list[m].name, nd->u.warp.x, nd->u.warp.y
			);
	}
	if (map->getcell(m, &nd->bl, nd->u.warp.x, nd->u.warp.y, CELL_CHKNOPASS)) {
		ShowWarning("Warp %s at %s(%d,%d) warps to a non-walkable tile at %s(%d,%d)\n",
			nd->name,
			map->list[nd->bl.m].name, nd->bl.x, nd->bl.y,
			map->list[m].name, nd->u.warp.x, nd->u.warp.y
			);
	}
}

static void npc_debug_warps(void)
{
	int16 m, i;
	for (m = 0; m < map->count; m++)
		for (i = 0; i < map->list[m].npc_num; i++)
			npc->debug_warps_sub(map->list[m].npc[i]);
}

static void npc_questinfo_clear(struct npc_data *nd)
{
	nullpo_retv(nd);

	for (int i = 0; i < VECTOR_LENGTH(nd->qi_data); i++) {
		struct questinfo *qi = &VECTOR_INDEX(nd->qi_data, i);
		VECTOR_CLEAR(qi->items);
		VECTOR_CLEAR(qi->quest_requirement);
	}
	VECTOR_CLEAR(nd->qi_data);
}

/*==========================================
 * npc initialization
 *------------------------------------------*/
static int do_init_npc(bool minimal)
{
	int i;

	unit->init_ud(&npc->base_ud);
	npc->base_ud.bl = NULL;

	//Stock view data for normal npcs.
	memset(&npc_viewdb, 0, sizeof(npc_viewdb));

	npc_viewdb[0].class = INVISIBLE_CLASS; //Invisible class is stored here.
	for( i = 1; i < MAX_NPC_CLASS; i++ )
		npc_viewdb[i].class = i;
	for( i = MAX_NPC_CLASS2_START; i < MAX_NPC_CLASS2_END; i++ )
		npc_viewdb2[i - MAX_NPC_CLASS2_START].class = i;
	npc->ev_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, EVENT_NAME_LENGTH);
	npc->ev_label_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, NAME_LENGTH);
	npc->name_db = strdb_alloc(DB_OPT_BASE, NAME_LENGTH);
	npc->path_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, 0);

	npc->npc_last_npd = NULL;
	npc->npc_last_path = NULL;
	npc->npc_last_ref = NULL;

	// Should be loaded before npc processing, otherwise labels could overwrite constant values
	// and lead to undefined behavior [Panikon]
	itemdb->name_constants();
	clan->set_constants();

	if (!minimal) {
		npc->timer_event_ers = ers_new(sizeof(struct timer_event_data),"clif.c::timer_event_ers",ERS_OPT_NONE);

		npc_process_files(START_NPC_NUM);
	}

	if (!minimal) {
		map->zone_init();

		npc->motd = npc->name2id("HerculesMOTD"); /* [Ind/Hercules] */

		// set up the events cache
		memset(script_event, 0, sizeof(script_event));
		npc->read_event_script();

		//Debug function to locate all endless loop warps.
		if (battle_config.warp_point_debug)
			npc->debug_warps();

		timer->add_func_list(npc->event_do_clock,"npc_event_do_clock");
		timer->add_func_list(npc->timerevent,"npc_timerevent");
	}

	// Init dummy NPC
	CREATE(npc->fake_nd, struct npc_data, 1);
	npc->fake_nd->bl.m = -1;
	npc->fake_nd->bl.id = npc->get_new_npc_id();
	npc->fake_nd->class_ = FAKE_NPC;
	npc->fake_nd->speed = 200;
	strcpy(npc->fake_nd->name,"FAKE_NPC");
	memcpy(npc->fake_nd->exname, npc->fake_nd->name, 9);

	npc->npc_script++;
	npc->fake_nd->bl.type = BL_NPC;
	npc->fake_nd->subtype = SCRIPT;

	strdb_put(npc->name_db, npc->fake_nd->exname, npc->fake_nd);
	npc->fake_nd->u.scr.timerid = INVALID_TIMER;
	map->addiddb(&npc->fake_nd->bl);
	// End of initialization

	return 0;
}
void npc_defaults(void)
{
	npc = &npc_s;

	npc->npc_id = START_NPC_NUM;
	npc->npc_warp = 0;
	npc->npc_shop = 0;
	npc->npc_script = 0;
	npc->npc_mob = 0;
	npc->npc_delay_mob = 0;
	npc->npc_cache_mob = 0;
	npc->npc_last_path = NULL;
	npc->npc_last_ref = NULL;
	npc->npc_last_npd = NULL;

	npc->motd = NULL;
	npc->ev_db = NULL;
	npc->ev_label_db = NULL;
	npc->name_db = NULL;
	npc->path_db = NULL;
	npc->timer_event_ers = NULL;
	npc->fake_nd = NULL;
	npc->src_files = NULL;
	/* */
	npc->trader_ok = false;
	npc->trader_funds[0] = npc->trader_funds[1] = 0;
	/* */
	npc->init = do_init_npc;
	npc->final = do_final_npc;
	/* */
	npc->get_new_npc_id = npc_get_new_npc_id;
	npc->get_viewdata = npc_get_viewdata;
	npc->isnear_sub = npc_isnear_sub;
	npc->isnear = npc_isnear;
	npc->ontouch_event = npc_ontouch_event;
	npc->ontouch2_event = npc_ontouch2_event;
	npc->onuntouch_event = npc_onuntouch_event;
	npc->enable_sub = npc_enable_sub;
	npc->enable = npc_enable;
	npc->name2id = npc_name2id;
	npc->event_dequeue = npc_event_dequeue;
	npc->event_export_create = npc_event_export_create;
	npc->event_export = npc_event_export;
	npc->event_sub = npc_event_sub;
	npc->event_doall_sub = npc_event_doall_sub;
	npc->event_do = npc_event_do;
	npc->event_doall_id = npc_event_doall_id;
	npc->event_doall = npc_event_doall;
	npc->event_do_clock = npc_event_do_clock;
	npc->event_do_oninit = npc_event_do_oninit;
	npc->timerevent_export = npc_timerevent_export;
	npc->timerevent = npc_timerevent;
	npc->timerevent_start = npc_timerevent_start;
	npc->timerevent_stop = npc_timerevent_stop;
	npc->timerevent_quit = npc_timerevent_quit;
	npc->gettimerevent_tick = npc_gettimerevent_tick;
	npc->settimerevent_tick = npc_settimerevent_tick;
	npc->event = npc_event;
	npc->touch_areanpc_sub = npc_touch_areanpc_sub;
	npc->touchnext_areanpc = npc_touchnext_areanpc;
	npc->touch_areanpc = npc_touch_areanpc;
	npc->untouch_areanpc = npc_untouch_areanpc;
	npc->touch_areanpc2 = npc_touch_areanpc2;
	npc->check_areanpc = npc_check_areanpc;
	npc->checknear = npc_checknear;
	npc->globalmessage = npc_globalmessage;
	npc->run_tomb = run_tomb;
	npc->click = npc_click;
	npc->scriptcont = npc_scriptcont;
	npc->buysellsel = npc_buysellsel;
	npc->cashshop_buylist = npc_cashshop_buylist;
	npc->buylist_sub = npc_buylist_sub;
	npc->cashshop_buy = npc_cashshop_buy;
	npc->buylist = npc_buylist;
	npc->selllist_sub = npc_selllist_sub;
	npc->selllist = npc_selllist;
	npc->remove_map = npc_remove_map;
	npc->unload_ev = npc_unload_ev;
	npc->unload_ev_label = npc_unload_ev_label;
	npc->unload_dup_sub = npc_unload_dup_sub;
	npc->unload_duplicates = npc_unload_duplicates;
	npc->unload = npc_unload;
	npc->clearsrcfile = npc_clearsrcfile;
	npc->addsrcfile = npc_addsrcfile;
	npc->delsrcfile = npc_delsrcfile;
	npc->retainpathreference = npc_retainpathreference;
	npc->releasepathreference = npc_releasepathreference;
	npc->parsename = npc_parsename;
	npc->parseview = npc_parseview;
	npc->viewisid = npc_viewisid;
	npc->create_npc = npc_create_npc;
	npc->add_warp = npc_add_warp;
	npc->parse_warp = npc_parse_warp;
	npc->parse_shop = npc_parse_shop;
	npc->convertlabel_db = npc_convertlabel_db;
	npc->skip_script = npc_skip_script;
	npc->parse_script = npc_parse_script;
	npc->add_to_location = npc_add_to_location;
	npc->duplicate_script_sub = npc_duplicate_script_sub;
	npc->duplicate_shop_sub = npc_duplicate_shop_sub;
	npc->duplicate_warp_sub = npc_duplicate_warp_sub;
	npc->duplicate_sub = npc_duplicate_sub;
	npc->parse_duplicate = npc_parse_duplicate;
	npc->duplicate4instance = npc_duplicate4instance;
	npc->setcells = npc_setcells;
	npc->unsetcells_sub = npc_unsetcells_sub;
	npc->unsetcells = npc_unsetcells;
	npc->movenpc = npc_movenpc;
	npc->setdisplayname = npc_setdisplayname;
	npc->setclass = npc_setclass;
	npc->do_atcmd_event = npc_do_atcmd_event;
	npc->parse_function = npc_parse_function;
	npc->parse_mob2 = npc_parse_mob2;
	npc->parse_mob = npc_parse_mob;
	npc->parse_mapflag = npc_parse_mapflag;
	npc->parse_unknown_mapflag = npc_parse_unknown_mapflag;
	npc->parsesrcfile = npc_parsesrcfile;
	npc->parse_unknown_object = npc_parse_unknown_object;
	npc->script_event = npc_script_event;
	npc->read_event_script = npc_read_event_script;
	npc->path_db_clear_sub = npc_path_db_clear_sub;
	npc->ev_label_db_clear_sub = npc_ev_label_db_clear_sub;
	npc->reload = npc_reload;
	npc->unloadfile = npc_unloadfile;
	npc->do_clear_npc = do_clear_npc;
	npc->debug_warps_sub = npc_debug_warps_sub;
	npc->debug_warps = npc_debug_warps;
	npc->secure_timeout_timer = npc_rr_secure_timeout_timer;
	/* */
	npc->trader_count_funds = npc_trader_count_funds;
	npc->trader_pay = npc_trader_pay;
	npc->trader_update = npc_trader_update;
	npc->market_buylist = npc_market_buylist;
	npc->barter_buylist = npc_barter_buylist;
	npc->trader_open = npc_trader_open;
	npc->market_fromsql = npc_market_fromsql;
	npc->market_tosql = npc_market_tosql;
	npc->market_delfromsql = npc_market_delfromsql;
	npc->market_delfromsql_sub = npc_market_delfromsql_sub;
	npc->barter_fromsql = npc_barter_fromsql;
	npc->barter_tosql = npc_barter_tosql;
	npc->barter_delfromsql = npc_barter_delfromsql;
	npc->barter_delfromsql_sub = npc_barter_delfromsql_sub;
	npc->db_checkid = npc_db_checkid;
	npc->refresh = npc_refresh;
	npc->questinfo_clear = npc_questinfo_clear;
}
