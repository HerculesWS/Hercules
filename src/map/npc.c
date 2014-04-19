// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/ers.h"
#include "../common/db.h"
#include "../common/socket.h"
#include "../common/HPM.h"

#include "map.h"
#include "log.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "status.h"
#include "itemdb.h"
#include "script.h"
#include "mob.h"
#include "pet.h"
#include "instance.h"
#include "battle.h"
#include "skill.h"
#include "unit.h"
#include "npc.h"
#include "chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

struct npc_interface npc_s;

static int npc_id=START_NPC_NUM;
static int npc_warp=0;
static int npc_shop=0;
static int npc_script=0;
static int npc_mob=0;
static int npc_delay_mob=0;
static int npc_cache_mob=0;

static char *npc_last_path;
static char *npc_last_ref;
struct npc_path_data *npc_last_npd;

//For holding the view data of npc classes. [Skotlex]
static struct view_data npc_viewdb[MAX_NPC_CLASS];
static struct view_data npc_viewdb2[MAX_NPC_CLASS2_END-MAX_NPC_CLASS2_START];

/* for speedup */
unsigned int npc_market_qty[MAX_INVENTORY];

static struct script_event_s
{	//Holds pointers to the commonly executed scripts for speedup. [Skotlex]
	struct event_data *event[UCHAR_MAX];
	const char *event_name[UCHAR_MAX];
	uint8 event_count;
} script_event[NPCE_MAX];

struct view_data* npc_get_viewdata(int class_)
{	//Returns the viewdata for normal npc classes.
	if( class_ == INVISIBLE_CLASS )
		return &npc_viewdb[0];
	if (npcdb_checkid(class_) || class_ == WARP_CLASS){
		if( class_ > MAX_NPC_CLASS2_START ){
			return &npc_viewdb2[class_-MAX_NPC_CLASS2_START];
		}else{
			return &npc_viewdb[class_];
		}
	}
	return NULL;
}

/// Returns a new npc id that isn't being used in id_db.
/// Fatal error if nothing is available.
int npc_get_new_npc_id(void) {
	if( npc_id >= START_NPC_NUM && !map->blid_exists(npc_id) )
		return npc_id++;// available
	else {// find next id
		int base_id = npc_id;
		while( base_id != ++npc_id ) {
			if( npc_id < START_NPC_NUM )
				npc_id = START_NPC_NUM;
			if( !map->blid_exists(npc_id) )
				return npc_id++;// available
		}
		// full loop, nothing available
		ShowFatalError("npc_get_new_npc_id: All ids are taken. Exiting...");
		exit(1);
	}
}

int npc_isnear_sub(struct block_list* bl, va_list args) {
    struct npc_data *nd = (struct npc_data*)bl;
    
    if( nd->option & (OPTION_HIDE|OPTION_INVISIBLE) )
        return 0;

    return 1;
}

bool npc_isnear(struct block_list * bl) {
	if( battle_config.min_npc_vendchat_distance > 0
	 && map->foreachinrange(npc->isnear_sub,bl, battle_config.min_npc_vendchat_distance, BL_NPC) )
		return true;

	return false;
}

int npc_ontouch_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	if( nd->touching_id )
		return 0; // Attached a player already. Can't trigger on anyone else.

	if( pc_ishiding(sd) )
		return 1; // Can't trigger 'OnTouch_'. try 'OnTouch' later.

	snprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script->config.ontouch_name);
	return npc->event(sd,name,1);
}

int npc_ontouch2_event(struct map_session_data *sd, struct npc_data *nd)
{
	char name[EVENT_NAME_LENGTH];

	if( sd->areanpc_id == nd->bl.id )
		return 0;

	snprintf(name, ARRAYLENGTH(name), "%s::%s", nd->exname, script->config.ontouch2_name);
	return npc->event(sd,name,2);
}

/*==========================================
 * Sub-function of npc_enable, runs OnTouch event when enabled
 *------------------------------------------*/
int npc_enable_sub(struct block_list *bl, va_list ap)
{
	struct npc_data *nd;

	nullpo_ret(bl);
	nullpo_ret(nd=va_arg(ap,struct npc_data *));
	if(bl->type == BL_PC)
	{
		TBL_PC *sd = (TBL_PC*)bl;

		if (nd->option&OPTION_INVISIBLE)
			return 1;

		if( npc->ontouch_event(sd,nd) > 0 && npc->ontouch2_event(sd,nd) > 0 )
		{ // failed to run OnTouch event, so just click the npc
			if (sd->npc_id != 0)
				return 0;

			pc_stop_walking(sd,1);
			npc->click(sd,nd);
		}
	}
	return 0;
}

/*==========================================
 * Disable / Enable NPC
 *------------------------------------------*/
int npc_enable(const char* name, int flag)
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
struct npc_data* npc_name2id(const char* name)
{
	return (struct npc_data *) strdb_get(npc->name_db, name);
}
/**
 * For the Secure NPC Timeout option (check config/Secure.h) [RR]
 **/
/**
 * Timer to check for idle time and timeout the dialog if necessary
 **/
int npc_rr_secure_timeout_timer(int tid, int64 tick, int id, intptr_t data) {
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
		clif->scriptmes(sd, sd->npc_id, " ");
		/**
		 * This guy's been idle for longer than allowed, close him.
		 **/
		clif->scriptclose(sd,sd->npc_id);
		clif->scriptclear(sd,sd->npc_id);
		sd->npc_idle_timer = INVALID_TIMER;
	} else //Create a new instance of ourselves to continue
		sd->npc_idle_timer = timer->add(timer->gettick() + (SECURE_NPCTIMEOUT_INTERVAL*1000),npc->secure_timeout_timer,sd->bl.id,0);
#endif
	return 0;
}

/*==========================================
 * Dequeue event and add timer for execution (100ms)
 *------------------------------------------*/
int npc_event_dequeue(struct map_session_data* sd)
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
DBData npc_event_export_create(DBKey key, va_list args)
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
int npc_event_export(struct npc_data *nd, int i)
{
	char* lname = nd->u.scr.label_list[i].name;
	int pos = nd->u.scr.label_list[i].pos;
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

int npc_event_sub(struct map_session_data* sd, struct event_data* ev, const char* eventname); //[Lance]

/**
 * Exec name (NPC events) on player or global
 * Do on all NPC when called with foreach
 */
void npc_event_doall_sub(void *key, void *data, va_list ap)
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
			script->run(ev->nd->u.scr.script, ev->pos, rid, ev->nd->bl.id);
		}
		(*c)++;
	}
}

// runs the specified event (supports both single-npc and global events)
int npc_event_do(const char* name)
{
	if( name[0] == ':' && name[1] == ':' ) {
		return npc->event_doall(name+2); // skip leading "::"
	}
	else {
		struct event_data *ev = strdb_get(npc->ev_db, name);
		if (ev) {
			script->run(ev->nd->u.scr.script, ev->pos, 0, ev->nd->bl.id);
			return 1;
		}
	}
	return 0;
}

// runs the specified event, with a RID attached (global only)
int npc_event_doall_id(const char* name, int rid)
{
	int c = 0;
	struct linkdb_node **label_linkdb = strdb_get(npc->ev_label_db, name);

	if (label_linkdb == NULL)
		return 0;

	linkdb_foreach(label_linkdb, npc->event_doall_sub, &c, name, rid);
	return c;
}

// runs the specified event (global only)
int npc_event_doall(const char* name)
{
	return npc->event_doall_id(name, 0);
}

/*==========================================
 * Clock event execution
 * OnMinute/OnClock/OnHour/OnDay/OnDDHHMM
 *------------------------------------------*/
int npc_event_do_clock(int tid, int64 tick, int id, intptr_t data) {
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

/*==========================================
 * OnInit Event execution (the start of the event and watch)
 *------------------------------------------*/
void npc_event_do_oninit(void)
{
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs."CL_CLL"\n", npc->event_doall("OnInit"));

	timer->add_interval(timer->gettick()+100,npc->event_do_clock,0,0,1000);
}

/*==========================================
 * Incorporation of the label for the timer event
 * called from npc_parse_script
 *------------------------------------------*/
int npc_timerevent_export(struct npc_data *nd, int i)
{
	int t = 0, len = 0;
	char *lname = nd->u.scr.label_list[i].name;
	int pos = nd->u.scr.label_list[i].pos;
	if (sscanf(lname, "OnTimer%d%n", &t, &len) == 1 && lname[len] == '\0') {
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
int npc_timerevent(int tid, int64 tick, int id, intptr_t data) {
	int old_rid, old_timer;
	int64 old_tick;
	struct npc_data* nd=(struct npc_data *)map->id2bl(id);
	struct npc_timerevent_list *te;
	struct timer_event_data *ted = (struct timer_event_data*)data;
	struct map_session_data *sd=NULL;

	if( nd == NULL ) {
		ShowError("npc_timerevent: NPC not found??\n");
		return 0;
	}

	if( ted->rid && !(sd = map->id2sd(ted->rid)) ) {
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
	script->run(nd->u.scr.script,te->pos,nd->u.scr.rid,nd->bl.id);

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
int npc_timerevent_start(struct npc_data* nd, int rid) {
	int j;
	int64 tick = timer->gettick();
	struct map_session_data *sd = NULL; //Player to whom script is attached.

	nullpo_ret(nd);

	// Check if there is an OnTimer Event
	ARR_FIND( 0, nd->u.scr.timeramount, j, nd->u.scr.timer_event[j].timer > nd->u.scr.timer );

	if( nd->u.scr.rid > 0 && !(sd = map->id2sd(nd->u.scr.rid)) ) {
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
int npc_timerevent_stop(struct npc_data* nd)
{
	struct map_session_data *sd = NULL;
	const struct TimerData *td = NULL;
	int *tid;

	nullpo_ret(nd);

	if( nd->u.scr.rid && !(sd = map->id2sd(nd->u.scr.rid)) ) {
		ShowError("npc_timerevent_stop: Attached player not found!\n");
		return 1;
	}
	tid = sd?&sd->npc_timer_id:&nd->u.scr.timerid;
	if( *tid == INVALID_TIMER && (sd || !nd->u.scr.timertick) ) // Nothing to stop
		return 0;

	// Delete timer
	if ( *tid != INVALID_TIMER )
	{
		td = timer->get(*tid);
		if( td && td->data )
			ers_free(npc->timer_event_ers, (void*)td->data);
		timer->delete(*tid,npc->timerevent);
		*tid = INVALID_TIMER;
	}

	if( !sd && nd->u.scr.timertick )
	{
		nd->u.scr.timer += DIFF_TICK32(timer->gettick(),nd->u.scr.timertick); // Set 'timer' to the time that has passed since the beginning of the timers
		nd->u.scr.timertick = 0; // Set 'tick' to zero so that we know it's off.
	}

	return 0;
}
/*==========================================
 * Aborts a running NPC timer that is attached to a player.
 *------------------------------------------*/
void npc_timerevent_quit(struct map_session_data* sd)
{
	const struct TimerData *td;
	struct npc_data* nd;
	struct timer_event_data *ted;

	// Check timer existance
	if( sd->npc_timer_id == INVALID_TIMER )
		return;
	if( !(td = timer->get(sd->npc_timer_id)) )
	{
		sd->npc_timer_id = INVALID_TIMER;
		return;
	}

	// Delete timer
	nd = (struct npc_data *)map->id2bl(td->id);
	ted = (struct timer_event_data*)td->data;
	timer->delete(sd->npc_timer_id, npc->timerevent);
	sd->npc_timer_id = INVALID_TIMER;

	// Execute OnTimerQuit
	if( nd && nd->bl.type == BL_NPC )
	{
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

			//Set timer related info.
			old_rid = (nd->u.scr.rid == sd->bl.id ? 0 : nd->u.scr.rid); // Detach rid if the last attached player logged off.
			old_tick = nd->u.scr.timertick;
			old_timer = nd->u.scr.timer;

			nd->u.scr.rid = sd->bl.id;
			nd->u.scr.timertick = timer->gettick();
			nd->u.scr.timer = ted->time;

			//Execute label
			script->run(nd->u.scr.script,ev->pos,sd->bl.id,nd->bl.id);

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
int64 npc_gettimerevent_tick(struct npc_data* nd) {
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
int npc_settimerevent_tick(struct npc_data* nd, int newtimer)
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

int npc_event_sub(struct map_session_data* sd, struct event_data* ev, const char* eventname)
{
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
	script->run(ev->nd->u.scr.script,ev->pos,sd->bl.id,ev->nd->bl.id);
	return 0;
}

/*==========================================
 * NPC processing event type
 *------------------------------------------*/
int npc_event(struct map_session_data* sd, const char* eventname, int ontouch)
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
int npc_touch_areanpc_sub(struct block_list *bl, va_list ap) {
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
int npc_touchnext_areanpc(struct map_session_data* sd, bool leavemap) {
	struct npc_data *nd = map->id2nd(sd->touching_id);
	short xs, ys;

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
int npc_touch_areanpc(struct map_session_data* sd, int16 m, int16 x, int16 y)
{
	int xs,ys;
	int f = 1;
	int i;
	int j, found_warp = 0;

	nullpo_retr(1, sd);

	// Why not enqueue it? [Inkfish]
	//if(sd->npc_id)
	//	return 1;

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

// OnTouch NPC or Warp for Mobs
// Return 1 if Warped
int npc_touch_areanpc2(struct mob_data *md)
{
	int i, m = md->bl.m, x = md->bl.x, y = md->bl.y, id;
	char eventname[EVENT_NAME_LENGTH];
	struct event_data* ev;
	int xs, ys;

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
					script->run(ev->nd->u.scr.script, ev->pos, md->bl.id, ev->nd->bl.id);
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
int npc_check_areanpc(int flag, int16 m, int16 x, int16 y, int16 range) {
	int i;
	int x0,y0,x1,y1;
	int xs,ys;

	if (range < 0) return 0;
	x0 = max(x-range, 0);
	y0 = max(y-range, 0);
	x1 = min(x+range, map->list[m].xs-1);
	y1 = min(y+range, map->list[m].ys-1);

	//First check for npc_cells on the range given
	i = 0;
	for (ys = y0; ys <= y1 && !i; ys++) {
		for(xs = x0; xs <= x1 && !i; xs++) {
			if (map->getcell(m,xs,ys,CELL_CHKNPC))
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
struct npc_data* npc_checknear(struct map_session_data* sd, struct block_list* bl)
{
	struct npc_data *nd;

	nullpo_retr(NULL, sd);
	if(bl == NULL) return NULL;
	if(bl->type != BL_NPC) return NULL;
	nd = (TBL_NPC*)bl;

	if(sd->state.using_fake_npc && sd->npc_id == bl->id)
		return nd;

	if (nd->class_<0) //Class-less npc, enable click from anywhere.
		return nd;

	if (bl->m!=sd->bl.m ||
	   bl->x<sd->bl.x-AREA_SIZE-1 || bl->x>sd->bl.x+AREA_SIZE+1 ||
	   bl->y<sd->bl.y-AREA_SIZE-1 || bl->y>sd->bl.y+AREA_SIZE+1)
		return NULL;

	return nd;
}

/*==========================================
 * Make NPC talk in global chat (like npctalk)
 *------------------------------------------*/
int npc_globalmessage(const char* name, const char* mes)
{
	struct npc_data* nd = npc->name2id(name);
	char temp[100];

	if (!nd)
		return 0;

	snprintf(temp, sizeof(temp), "%s : %s", name, mes);
	clif->GlobalMessage(&nd->bl,temp);

	return 0;
}

// MvP tomb [GreenBox]
void run_tomb(struct map_session_data* sd, struct npc_data* nd) {
	char buffer[200];
	char time[10];

	strftime(time, sizeof(time), "%H:%M", localtime(&nd->u.tomb.kill_time));

	// TODO: Find exact color?
	snprintf(buffer, sizeof(buffer), msg_txt(857), nd->u.tomb.md->db->name); // "[ ^EE0000%s^000000 ]"
	clif->scriptmes(sd, nd->bl.id, buffer);

	clif->scriptmes(sd, nd->bl.id, msg_txt(858)); // "Has met its demise"

	snprintf(buffer, sizeof(buffer), msg_txt(859), time); // "Time of death : ^EE0000%s^000000"
	clif->scriptmes(sd, nd->bl.id, buffer);

	clif->scriptmes(sd, nd->bl.id, msg_txt(860)); // "Defeated by"

	snprintf(buffer, sizeof(buffer), msg_txt(861), nd->u.tomb.killer_name[0] ? nd->u.tomb.killer_name : msg_txt(15)); // "[^EE0000%s^000000]" / "Unknown"
	clif->scriptmes(sd, nd->bl.id, buffer);

	clif->scriptclose(sd, nd->bl.id);
}

/*==========================================
 * NPC 1st call when clicking on npc
 * Do specific action for NPC type (openshop, run scripts...)
 *------------------------------------------*/
int npc_click(struct map_session_data* sd, struct npc_data* nd)
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
				script->run(nd->u.scr.script,0,sd->bl.id,nd->bl.id);
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
int npc_scriptcont(struct map_session_data* sd, int id, bool closing) {
	struct block_list *target = map->id2bl(id);
	nullpo_retr(1, sd);

	if( id != sd->npc_id ){
		TBL_NPC* nd_sd=(TBL_NPC*)map->id2bl(sd->npc_id);
		TBL_NPC* nd = BL_CAST(BL_NPC, target);
		ShowDebug("npc_scriptcont: %s (sd->npc_id=%d) is not %s (id=%d).\n",
			nd_sd?(char*)nd_sd->name:"'Unknown NPC'", (int)sd->npc_id,
		  	nd?(char*)nd->name:"'Unknown NPC'", (int)id);
		return 1;
	}
	
	if(id != npc->fake_nd->bl.id) { // Not item script
		if ((npc->checknear(sd,target)) == NULL){
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
	
	if( !sd->st )
		return 1;
	
	if( closing && sd->st->state == CLOSE )
		sd->st->state = END;

	script->run_main(sd->st);

	return 0;
}

/*==========================================
 * Chk if valid call then open buy or selling list
 *------------------------------------------*/
int npc_buysellsel(struct map_session_data* sd, int id, int type) {
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
int npc_cashshop_buylist(struct map_session_data *sd, int points, int count, unsigned short* item_list) {
	int i, j, nameid, amount, new_, w, vt;
	struct npc_data *nd = NULL;
	struct npc_item_list *shop = NULL;
	unsigned short shop_size = 0;

	if( sd->state.trading )
		return 4;

	if( count <= 0 )
		return 5;
	
	if( points < 0 )
		return 6;
	
	if( !(nd = (struct npc_data *)map->id2bl(sd->npc_shopid)) )
		return 1;
	
	if( nd->subtype != CASHSHOP ) {
		if( nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type != NST_ZENY && nd->u.scr.shop->type != NST_MARKET ) {
			shop = nd->u.scr.shop->item;
			shop_size = nd->u.scr.shop->items;
		} else
			return 1;
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;
	}
	
	new_ = 0;
	w = 0;
	vt = 0; // Global Value

	// Validating Process ----------------------------------------------------
	for( i = 0; i < count; i++ ) {
		nameid = item_list[i*2+1];
		amount = item_list[i*2+0];

		if( !itemdb->exists(nameid) || amount <= 0 )
			return 5;

		ARR_FIND(0,shop_size,j,shop[j].nameid == nameid);
		if( j == shop_size || shop[j].value <= 0 )
			return 5;

		if( !itemdb->isstackable(nameid) && amount > 1 ) {
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = item_list[i*2+0] = 1;
		}

		switch( pc->checkadditem(sd,nameid,amount) ) {
			case ADDITEM_NEW:
				new_++;
				break;
			case ADDITEM_OVERAMOUNT:
				return 3;
		}

		vt += shop[j].value * amount;
		w += itemdb_weight(nameid) * amount;
	}

	if( w + sd->weight > sd->max_weight )
		return 3;
	
	if( pc->inventoryblank(sd) < new_ )
		return 3;
	
	if( points > vt ) points = vt;

	// Payment Process ----------------------------------------------------
	if( nd->subtype == SCRIPT && nd->u.scr.shop->type == NST_CUSTOM ) {
		if( !npc->trader_pay(nd,sd,vt,points) )
			return 6;
	} else {
		if( sd->kafraPoints < points || sd->cashPoints < (vt - points) )
			return 6;
		pc->paycash(sd,vt,points);
	}
	// Delivery Process ----------------------------------------------------
	for( i = 0; i < count; i++ ) {
		struct item item_tmp;

		nameid = item_list[i*2+1];
		amount = item_list[i*2+0];

		memset(&item_tmp,0,sizeof(item_tmp));

		if( !pet->create_egg(sd,nameid) ) {
			item_tmp.nameid = nameid;
			item_tmp.identify = 1;
			pc->additem(sd,&item_tmp,amount,LOG_TYPE_NPC);
		}
	}

	return 0;
}

//npc_buylist for script-controlled shops.
int npc_buylist_sub(struct map_session_data* sd, int n, unsigned short* item_list, struct npc_data* nd)
{
	char npc_ev[EVENT_NAME_LENGTH];
	int i;
	int key_nameid = 0;
	int key_amount = 0;

	// discard old contents
	script->cleararray_pc(sd, "@bought_nameid", (void*)0);
	script->cleararray_pc(sd, "@bought_quantity", (void*)0);

	// save list of bought items
	for( i = 0; i < n; i++ ) {
		script->setarray_pc(sd, "@bought_nameid", i, (void*)(intptr_t)item_list[i*2+1], &key_nameid);
		script->setarray_pc(sd, "@bought_quantity", i, (void*)(intptr_t)item_list[i*2], &key_amount);
	}

	// invoke event
	snprintf(npc_ev, ARRAYLENGTH(npc_ev), "%s::OnBuyItem", nd->exname);
	npc->event(sd, npc_ev, 0);

	return 0;
}
/**
 * Loads persistent NPC Market Data from SQL
 **/
void npc_market_fromsql(void) {
	SqlStmt* stmt = SQL->StmtMalloc(map->mysql_handle);
	char name[NAME_LENGTH+1];
	int itemid;
	int amount;
	
	/* TODO inter-server.conf npc_market_data */
	if ( SQL_ERROR == SQL->StmtPrepare(stmt, "SELECT `name`, `itemid`, `amount` FROM `npc_market_data`")
		|| SQL_ERROR == SQL->StmtExecute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}
	
	SQL->StmtBindColumn(stmt, 0, SQLDT_STRING, &name[0], sizeof(name), NULL, NULL);
	SQL->StmtBindColumn(stmt, 1, SQLDT_INT, &itemid, 0, NULL, NULL);
	SQL->StmtBindColumn(stmt, 2, SQLDT_INT, &amount, 0, NULL, NULL);
	
	while ( SQL_SUCCESS == SQL->StmtNextRow(stmt) ) {
		struct npc_data *nd = NULL;
		unsigned short i;
		
		if( !(nd = npc->name2id(name)) ) {
			ShowError("npc_market_fromsql: NPC '%s' not found! skipping...\n",name);
			npc->market_delfromsql_sub(name, USHRT_MAX);
			continue;
		} else if ( nd->subtype != SCRIPT || !nd->u.scr.shop || !nd->u.scr.shop->items || nd->u.scr.shop->type != NST_MARKET ) {
			ShowError("npc_market_fromsql: NPC '%s' is not proper for market, skipping...\n",name);
			npc->market_delfromsql_sub(name, USHRT_MAX);
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
void npc_market_tosql(struct npc_data *nd, unsigned short index) {
	/* TODO inter-server.conf npc_market_data */
	if( SQL_ERROR == SQL->Query(map->mysql_handle, "REPLACE INTO `npc_market_data` VALUES ('%s','%d','%d')", nd->exname, nd->u.scr.shop->item[index].nameid, nd->u.scr.shop->item[index].qty) )
		Sql_ShowDebug(map->mysql_handle);
}
/**
 * Removes persistent NPC Market Data from SQL
 */
void npc_market_delfromsql_sub(const char *npcname, unsigned short index) {
	/* TODO inter-server.conf npc_market_data */
	if( index == USHRT_MAX ) {
		if( SQL_ERROR == SQL->Query(map->mysql_handle, "DELETE FROM `npc_market_data` WHERE `name`='%s'", npcname) )
			Sql_ShowDebug(map->mysql_handle);
	} else {
		if( SQL_ERROR == SQL->Query(map->mysql_handle, "DELETE FROM `npc_market_data` WHERE `name`='%s' AND `itemid`='%d' LIMIT 1", npcname, index) )
			Sql_ShowDebug(map->mysql_handle);
	}
}
/**
 * Removes persistent NPC Market Data from SQL
 **/
void npc_market_delfromsql(struct npc_data *nd, unsigned short index) {
	npc->market_delfromsql_sub(nd->exname, index == USHRT_MAX ? index : nd->u.scr.shop->item[index].nameid);
}
/**
 * Judges whether to allow and spawn a trader's window.
 **/
bool npc_trader_open(struct map_session_data *sd, struct npc_data *nd) {
	
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
				if( i == nd->u.scr.shop->items ) {
					clif->colormes(sd->fd,COLOR_RED,"Shop is out of stock! Come again later!");/* TODO messages.conf-it */
					return false;
				}

				clif->npc_market_open(sd,nd);
			}
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
void npc_trader_update(int master) {
	DBIterator* iter;
	struct block_list* bl;
	struct npc_data *master_nd = map->id2nd(master);
	
	CREATE(master_nd->u.scr.shop,struct npc_shop_data,1);
	
	iter = db_iterator(map->id_db);
	
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) ) {
		if( bl->type == BL_NPC ) {
			struct npc_data* nd = (struct npc_data*)bl;
			
			if( nd->src_id == master ) {
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
void npc_trader_count_funds(struct npc_data *nd, struct map_session_data *sd) {
	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;
	
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
		script->run(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
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
bool npc_trader_pay(struct npc_data *nd, struct map_session_data *sd, int price, int points) {
	char evname[EVENT_NAME_LENGTH];
	struct event_data *ev = NULL;
	
	npc->trader_ok = false;/* clear */
	
	snprintf(evname, EVENT_NAME_LENGTH, "%s::OnPayFunds",nd->exname);
	
	if ( (ev = strdb_get(npc->ev_db, evname)) ) {
		pc->setreg(sd,script->add_str("@price"),price);
		pc->setreg(sd,script->add_str("@points"),points);
		
		script->run(ev->nd->u.scr.script, ev->pos, sd->bl.id, ev->nd->bl.id);
	} else
		ShowError("npc_trader_pay: '%s' event '%s' not found, operation failed\n",nd->exname,evname);
	
	return npc->trader_ok;/* run script will deal with it */
}
/*==========================================
 * Cash Shop Buy
 *------------------------------------------*/
int npc_cashshop_buy(struct map_session_data *sd, int nameid, int amount, int points) {
	struct npc_data *nd = NULL;
	struct item_data *item;
	struct npc_item_list *shop = NULL;
	int i, price, w;
	unsigned short shop_size = 0;
	
	if( amount <= 0 )
		return 5;

	if( points < 0 )
		return 6;

	if( sd->state.trading )
		return 4;
	
	if( !(nd = (struct npc_data *)map->id2bl(sd->npc_shopid)) )
		return 1;

	if( (item = itemdb->exists(nameid)) == NULL )
		return 5; // Invalid Item

	if( nd->subtype != CASHSHOP ) {
		if( nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type != NST_ZENY && nd->u.scr.shop->type != NST_MARKET ) {
			shop = nd->u.scr.shop->item;
			shop_size = nd->u.scr.shop->items;
		} else
			return 1;
	} else {
		shop = nd->u.shop.shop_item;
		shop_size = nd->u.shop.count;
	}
	
	ARR_FIND(0, shop_size, i, shop[i].nameid == nameid);
	
	if( i == shop_size )
		return 5;
	
	if( shop[i].value <= 0 )
		return 5;

	if(!itemdb->isstackable(nameid) && amount > 1) {
		ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!\n",
			sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
		amount = 1;
	}

	switch( pc->checkadditem(sd, nameid, amount) ) {
		case ADDITEM_NEW:
			if( pc->inventoryblank(sd) == 0 )
				return 3;
			break;
		case ADDITEM_OVERAMOUNT:
			return 3;
	}

	w = item->weight * amount;
	if( w + sd->weight > sd->max_weight )
		return 3;

	if( (double)shop[i].value * amount > INT_MAX ) {
		ShowWarning("npc_cashshop_buy: Item '%s' (%d) price overflow attempt!\n", item->name, nameid);
		ShowDebug("(NPC:'%s' (%s,%d,%d), player:'%s' (%d/%d), value:%d, amount:%d)\n",
				nd->exname, map->list[nd->bl.m].name, nd->bl.x, nd->bl.y,
				sd->status.name, sd->status.account_id, sd->status.char_id,
				shop[i].value, amount);
		return 5;
	}

	price = shop[i].value * amount;
	
	if( points > price )
		points = price;

	if( nd->subtype == SCRIPT && nd->u.scr.shop->type == NST_CUSTOM ) {
		if( !npc->trader_pay(nd,sd,price,points) )
			return 6;
	} else {
		if( (sd->kafraPoints < points) || (sd->cashPoints < price - points) )
			return 6;
			
		pc->paycash(sd, price, points);
	}

	if( !pet->create_egg(sd, nameid) ) {
		struct item item_tmp;
		memset(&item_tmp, 0, sizeof(struct item));
		item_tmp.nameid = nameid;
		item_tmp.identify = 1;

		pc->additem(sd,&item_tmp, amount, LOG_TYPE_NPC);
	}

	return 0;
}

/// Player item purchase from npc shop.
///
/// @param item_list 'n' pairs <amount,itemid>
/// @return result code for clif->parse_NpcBuyListSend
int npc_buylist(struct map_session_data* sd, int n, unsigned short* item_list) {
	struct npc_data* nd;
	struct npc_item_list *shop = NULL;
	double z;
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
	for( i = 0; i < n; ++i ) {
		int nameid, amount, value;
		
		// find this entry in the shop's sell list
		ARR_FIND( 0, shop_size, j,
				 item_list[i*2+1] == shop[j].nameid || //Normal items
				 item_list[i*2+1] == itemdb_viewid(shop[j].nameid) //item_avail replacement
				 );
		
		if( j == shop_size )
			return 3; // no such item in shop
		
		amount = item_list[i*2+0];
		nameid = item_list[i*2+1] = shop[j].nameid; //item_avail replacement
		value = shop[j].value;
		
		if( !itemdb->exists(nameid) )
			return 3; // item no longer in itemdb
		
		if( !itemdb->isstackable(nameid) && amount > 1 ) {
			//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = item_list[i*2+0] = 1;
		}
		
		if( nd->master_nd ) {
			// Script-controlled shops decide by themselves, what can be bought and for what price.
			continue;
		}
		
		switch( pc->checkadditem(sd,nameid,amount) ) {
			case ADDITEM_EXIST:
				break;
				
			case ADDITEM_NEW:
				new_++;
				break;
				
			case ADDITEM_OVERAMOUNT:
				return 2;
		}
		
		value = pc->modifybuyvalue(sd,value);
		
		z += (double)value * amount;
		w += itemdb_weight(nameid) * amount;
	}
	
	if( nd->master_nd != NULL ) //Script-based shops.
		return npc->buylist_sub(sd,n,item_list,nd->master_nd);
	
	if( z > (double)sd->status.zeny )
		return 1;	// Not enough Zeny
	if( w + sd->weight > sd->max_weight )
		return 2;	// Too heavy
	if( pc->inventoryblank(sd) < new_ )
		return 3;	// Not enough space to store items
	
	pc->payzeny(sd,(int)z,LOG_TYPE_NPC, NULL);
	
	for( i = 0; i < n; ++i ) {
		int nameid = item_list[i*2+1];
		int amount = item_list[i*2+0];
		struct item item_tmp;
		
		if (itemdb_type(nameid) == IT_PETEGG)
			pet->create_egg(sd, nameid);
		else {
			memset(&item_tmp,0,sizeof(item_tmp));
			item_tmp.nameid = nameid;
			item_tmp.identify = 1;
			
			pc->additem(sd,&item_tmp,amount,LOG_TYPE_NPC);
		}
	}
	
	// custom merchant shop exp bonus
	if( battle_config.shop_exp > 0 && z > 0 && (skill_t = pc->checkskill2(sd,idx)) > 0 ) {
		if( sd->status.skill[idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill_t = sd->status.skill[idx].flag - SKILL_FLAG_REPLACED_LV_0;
		
		if( skill_t > 0 ) {
			z = z * (double)skill_t * (double)battle_config.shop_exp/10000.;
			if( z < 1 )
				z = 1;
			pc->gainexp(sd,NULL,0,(int)z, false);
		}
	}
	
	return 0;
}

/**
 * parses incoming npc market purchase list
 **/
int npc_market_buylist(struct map_session_data* sd, unsigned short list_size, struct packet_npc_market_purchase *p) {
	struct npc_data* nd;
	struct npc_item_list *shop = NULL;
	double z;
	int i,j,w,new_;
	unsigned short shop_size = 0;
	
	nullpo_retr(1, sd);
	nullpo_retr(1, p);

	nd = npc->checknear(sd,map->id2bl(sd->npc_shopid));

	if( nd == NULL || nd->subtype != SCRIPT || !list_size || !nd->u.scr.shop || nd->u.scr.shop->type != NST_MARKET )
		return 1;

	shop = nd->u.scr.shop->item;
	shop_size = nd->u.scr.shop->items;
	
	z = 0;
	w = 0;
	new_ = 0;
	
	// process entries in buy list, one by one
	for( i = 0; i < list_size; ++i ) {
		int nameid, amount, value;
		
		// find this entry in the shop's sell list
		ARR_FIND( 0, shop_size, j,
				 p->list[i].ITID == shop[j].nameid || //Normal items
				 p->list[i].ITID == itemdb_viewid(shop[j].nameid) //item_avail replacement
				 );
		
		if( j == shop_size ) /* TODO find official response for this */
			return 1; // no such item in shop

		if( p->list[i].qty > shop[j].qty )
			return 1;

		amount = p->list[i].qty;
		nameid = p->list[i].ITID = shop[j].nameid; //item_avail replacement
		value = shop[j].value;
		npc_market_qty[i] = j;

		if( !itemdb->exists(nameid) ) /* TODO find official response for this */
			return 1; // item no longer in itemdb
		
		if( !itemdb->isstackable(nameid) && amount > 1 ) {
			//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!\n",
						sd->status.name, sd->status.account_id, sd->status.char_id, amount, nameid);
			amount = p->list[i].qty = 1;
		}
				
		switch( pc->checkadditem(sd,nameid,amount) ) {
			case ADDITEM_EXIST:
				break;
				
			case ADDITEM_NEW:
				new_++;
				break;
				
			case ADDITEM_OVERAMOUNT: /* TODO find official response for this */
				return 1;
		}
				
		z += (double)value * amount;
		w += itemdb_weight(nameid) * amount;
	}

	if( z > (double)sd->status.zeny ) /* TODO find official response for this */
		return 1;	// Not enough Zeny

	if( w + sd->weight > sd->max_weight ) /* TODO find official response for this */
		return 1;	// Too heavy

	if( pc->inventoryblank(sd) < new_ ) /* TODO find official response for this */
		return 1;	// Not enough space to store items

	pc->payzeny(sd,(int)z,LOG_TYPE_NPC, NULL);
	
	for( i = 0; i < list_size; ++i ) {
		int nameid = p->list[i].ITID;
		int amount = p->list[i].qty;
		struct item item_tmp;
		
		j = npc_market_qty[i];
		
		if( p->list[i].qty > shop[j].qty ) /* wohoo someone tampered with the packet. */
			return 1;
		
		shop[j].qty -= amount;
		
		npc->market_tosql(nd,j);
		
		if (itemdb_type(nameid) == IT_PETEGG)
			pet->create_egg(sd, nameid);
		else {
			memset(&item_tmp,0,sizeof(item_tmp));
			item_tmp.nameid = nameid;
			item_tmp.identify = 1;
			
			pc->additem(sd,&item_tmp,amount,LOG_TYPE_NPC);
		}
	}

	return 0;
}

/// npc_selllist for script-controlled shops
int npc_selllist_sub(struct map_session_data* sd, int n, unsigned short* item_list, struct npc_data* nd)
{
	char npc_ev[EVENT_NAME_LENGTH];
	char card_slot[NAME_LENGTH];
	int i, j, idx;
	int key_nameid = 0;
	int key_amount = 0;
	int key_refine = 0;
	int key_attribute = 0;
	int key_identify = 0;
	int key_card[MAX_SLOTS];

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

	// save list of to be sold items
	for( i = 0; i < n; i++ )
	{
		idx = item_list[i*2]-2;

		script->setarray_pc(sd, "@sold_nameid", i, (void*)(intptr_t)sd->status.inventory[idx].nameid, &key_nameid);
		script->setarray_pc(sd, "@sold_quantity", i, (void*)(intptr_t)item_list[i*2+1], &key_amount);

		if( itemdb->isequip(sd->status.inventory[idx].nameid) )
		{// process equipment based information into the arrays
			script->setarray_pc(sd, "@sold_refine", i, (void*)(intptr_t)sd->status.inventory[idx].refine, &key_refine);
			script->setarray_pc(sd, "@sold_attribute", i, (void*)(intptr_t)sd->status.inventory[idx].attribute, &key_attribute);
			script->setarray_pc(sd, "@sold_identify", i, (void*)(intptr_t)sd->status.inventory[idx].identify, &key_identify);

			for( j = 0; j < MAX_SLOTS; j++ )
			{// store each of the cards from the equipment in the array
				snprintf(card_slot, sizeof(card_slot), "@sold_card%d", j + 1);
				script->setarray_pc(sd, card_slot, i, (void*)(intptr_t)sd->status.inventory[idx].card[j], &key_card[j]);
			}
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
int npc_selllist(struct map_session_data* sd, int n, unsigned short* item_list) {
	double z;
	int i,skill_t, skill_idx = skill->get_index(MC_OVERCHARGE);
	struct npc_data *nd;

	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	if( ( nd = npc->checknear(sd, map->id2bl(sd->npc_shopid)) ) == NULL ) {
		return 1;
	}

	if( nd->subtype != SHOP ) {
		if( !(nd->subtype == SCRIPT && nd->u.scr.shop && nd->u.scr.shop->type == NST_ZENY) )
			return 1;
	}

	
	z = 0;

	// verify the sell list
	for( i = 0; i < n; i++ ) {
		int nameid, amount, idx, value;

		idx    = item_list[i*2]-2;
		amount = item_list[i*2+1];

		if( idx >= MAX_INVENTORY || idx < 0 || amount < 0 ) {
			return 1;
		}

		nameid = sd->status.inventory[idx].nameid;

		if( !nameid || !sd->inventory_data[idx] || sd->status.inventory[idx].amount < amount ) {
			return 1;
		}

		if( nd->master_nd ) {// Script-controlled shops decide by themselves, what can be sold and at what price.
			continue;
		}

		value = pc->modifysellvalue(sd, sd->inventory_data[idx]->value_sell);

		z+= (double)value*amount;
	}

	if( nd->master_nd ) { // Script-controlled shops
		return npc->selllist_sub(sd, n, item_list, nd->master_nd);
	}

	// delete items
	for( i = 0; i < n; i++ ) {
		int amount, idx;

		idx    = item_list[i*2]-2;
		amount = item_list[i*2+1];

		if( sd->inventory_data[idx]->type == IT_PETEGG && sd->status.inventory[idx].card[0] == CARD0_PET ) {
			if( pet->search_petDB_index(sd->status.inventory[idx].nameid, PET_EGG) >= 0 ) {
				intif->delete_petdata(MakeDWord(sd->status.inventory[idx].card[1], sd->status.inventory[idx].card[2]));
			}
		}

		pc->delitem(sd, idx, amount, 0, 6, LOG_TYPE_NPC);
	}

	if( z > MAX_ZENY )
		z = MAX_ZENY;

	pc->getzeny(sd, (int)z, LOG_TYPE_NPC, NULL);

	// custom merchant shop exp bonus
	if( battle_config.shop_exp > 0 && z > 0 && ( skill_t = pc->checkskill2(sd,skill_idx) ) > 0) {
		if( sd->status.skill[skill_idx].flag >= SKILL_FLAG_REPLACED_LV_0 )
			skill_t = sd->status.skill[skill_idx].flag - SKILL_FLAG_REPLACED_LV_0;

		if( skill_t > 0 ) {
			z = z * (double)skill_t * (double)battle_config.shop_exp/10000.;
			if( z < 1 )
				z = 1;
			pc->gainexp(sd, NULL, 0, (int)z, false);
		}
	}

	return 0;
}

//Atempt to remove an npc from a map
//This doesn't remove it from map_db
int npc_remove_map(struct npc_data* nd) {
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
int npc_unload_ev(DBKey key, DBData *data, va_list ap)
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
int npc_unload_ev_label(DBKey key, DBData *data, va_list ap)
{
	struct linkdb_node **label_linkdb = DB->data2ptr(data);
	struct npc_data* nd = va_arg(ap, struct npc_data *);

	linkdb_erase(label_linkdb, nd);

	return 0;
}

//Chk if npc matches src_id, then unload.
//Sub-function used to find duplicates.
int npc_unload_dup_sub(struct npc_data* nd, va_list args)
{
	int src_id;

	src_id = va_arg(args, int);
	if (nd->src_id == src_id)
		npc->unload(nd, true);
	return 0;
}

//Removes all npcs that are duplicates of the passed one. [Skotlex]
void npc_unload_duplicates(struct npc_data* nd) {
	map->foreachnpc(npc->unload_dup_sub,nd->bl.id);
}

//Removes an npc from map and db.
//Single is to free name (for duplicates).
int npc_unload(struct npc_data* nd, bool single) {
	unsigned int i;
	
	nullpo_ret(nd);

	npc->remove_map(nd);
	map->deliddb(&nd->bl);
	if( single )
		strdb_remove(npc->name_db, nd->exname);

	if (nd->chat_id) // remove npc chatroom object and kick users
		chat->delete_npc_chat(nd);

#ifdef PCRE_SUPPORT
	npc_chat->finalize(nd); // deallocate npc PCRE data structures
#endif

	if( single && nd->path ) {
		struct npc_path_data* npd = NULL;
		if( nd->path && nd->path != npc_last_ref ) {
			npd = strdb_get(npc->path_db, nd->path);
		}

		if( npd && --npd->references == 0 ) {
			strdb_remove(npc->path_db, nd->path);/* remove from db */
			aFree(nd->path);/* remove now that no other instances exist */
		}
	}
	
	if( single && nd->bl.m != -1 )
		map->remove_questinfo(nd->bl.m,nd);

	if( nd->src_id == 0 && ( nd->subtype == SHOP || nd->subtype == CASHSHOP ) ) //src check for duplicate shops [Orcao]
		aFree(nd->u.shop.shop_item);
	else if( nd->subtype == SCRIPT ) {
		struct s_mapiterator* iter;
		struct block_list* bl;

		if( single ) {
			npc->ev_db->foreach(npc->ev_db,npc->unload_ev,nd->exname); //Clean up all events related
			npc->ev_label_db->foreach(npc->ev_label_db,npc->unload_ev_label,nd);
		}

		iter = mapit_geteachpc();
		for( bl = (struct block_list*)mapit->first(iter); mapit->exists(iter); bl = (struct block_list*)mapit->next(iter) ) {
			struct map_session_data *sd = ((TBL_PC*)bl);
			if( sd && sd->npc_timer_id != INVALID_TIMER ) {
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

	if( nd->ud != &npc->base_ud ) {
		aFree(nd->ud);
		nd->ud = NULL;
	}
	
	for( i = 0; i < nd->hdatac; i++ ) {
		if( nd->hdata[i]->flag.free ) {
			aFree(nd->hdata[i]->data);
		}
		aFree(nd->hdata[i]);
	}
	if( nd->hdata )
		aFree(nd->hdata);
	
	aFree(nd);

	return 0;
}

//
// NPC Source Files
//

/// Clears the npc source file list
void npc_clearsrcfile(void)
{
	struct npc_src_list* file = npc->src_files;
	struct npc_src_list* file_tofree;

	while( file != NULL )
	{
		file_tofree = file;
		file = file->next;
		aFree(file_tofree);
	}
	npc->src_files = NULL;
}

/// Adds a npc source file (or removes all)
void npc_addsrcfile(const char* name)
{
	struct npc_src_list* file;
	struct npc_src_list* file_prev = NULL;

	if( strcmpi(name, "clear") == 0 )
	{
		npc->clearsrcfile();
		return;
	}

	// prevent multiple insert of source files
	file = npc->src_files;
	while( file != NULL )
	{
		if( strcmp(name, file->name) == 0 )
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

/// Removes a npc source file (or all)
void npc_delsrcfile(const char* name)
{
	struct npc_src_list* file = npc->src_files;
	struct npc_src_list* file_prev = NULL;

	if( strcmpi(name, "all") == 0 )
	{
		npc->clearsrcfile();
		return;
	}

	while( file != NULL )
	{
		if( strcmp(file->name, name) == 0 )
		{
			if( npc->src_files == file )
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

/// Parses and sets the name and exname of a npc.
/// Assumes that m, x and y are already set in nd.
void npc_parsename(struct npc_data* nd, const char* name, const char* start, const char* buffer, const char* filepath) {
	const char* p;
	struct npc_data* dnd;// duplicate npc
	char newname[NAME_LENGTH];

	// parse name
	p = strstr(name,"::");
	if( p ) { // <Display name>::<Unique name>
		size_t len = p-name;
		if( len > NAME_LENGTH ) {
			ShowWarning("npc_parsename: Display name of '%s' is too long (len=%u) in file '%s', line '%d'. Truncating to %u characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NAME_LENGTH);
			safestrncpy(nd->name, name, sizeof(nd->name));
		} else {
			memcpy(nd->name, name, len);
			memset(nd->name+len, 0, sizeof(nd->name)-len);
		}
		len = strlen(p+2);
		if( len > NAME_LENGTH )
			ShowWarning("npc_parsename: Unique name of '%s' is too long (len=%u) in file '%s', line '%d'. Truncating to %u characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NAME_LENGTH);
		safestrncpy(nd->exname, p+2, sizeof(nd->exname));
	} else {// <Display name>
		size_t len = strlen(name);
		if( len > NAME_LENGTH )
			ShowWarning("npc_parsename: Name '%s' is too long (len=%u) in file '%s', line '%d'. Truncating to %u characters.\n", name, (unsigned int)len, filepath, strline(buffer,start-buffer), NAME_LENGTH);
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
			snprintf(newname, ARRAYLENGTH(newname), "%d_%d_%d_%d", i, nd->bl.m, nd->bl.x, nd->bl.y);
		} while( npc->name2id(newname) != NULL );

		strcpy(this_mapname, (nd->bl.m == -1 ? "(not on a map)" : mapindex_id2name(map_id2index(nd->bl.m))));
		strcpy(other_mapname, (dnd->bl.m == -1 ? "(not on a map)" : mapindex_id2name(map_id2index(dnd->bl.m))));

		ShowWarning("npc_parsename: Duplicate unique name in file '%s', line '%d'. Renaming '%s' to '%s'.\n", filepath, strline(buffer,start-buffer), nd->exname, newname);
		ShowDebug("this npc:\n   display name '%s'\n   unique name '%s'\n   map=%s, x=%d, y=%d\n", nd->name, nd->exname, this_mapname, nd->bl.x, nd->bl.y);
		ShowDebug("other npc in '%s' :\n   display name '%s'\n   unique name '%s'\n   map=%s, x=%d, y=%d\n",dnd->path, dnd->name, dnd->exname, other_mapname, dnd->bl.x, dnd->bl.y);
		safestrncpy(nd->exname, newname, sizeof(nd->exname));
	}

	if( npc_last_path != filepath ) {
		struct npc_path_data * npd = NULL;

		if( !(npd = strdb_get(npc->path_db,filepath) ) ) {
			CREATE(npd, struct npc_path_data, 1);
			strdb_put(npc->path_db, filepath, npd);

			CREATE(npd->path, char, strlen(filepath)+1);
			safestrncpy(npd->path, filepath, strlen(filepath)+1);

			npd->references = 0;
		}

		nd->path = npd->path;
		npd->references++;

		npc_last_npd = npd;
		npc_last_ref = npd->path;
		npc_last_path = (char*) filepath;
	} else {
		nd->path = npc_last_ref;
		if( npc_last_npd )
			npc_last_npd->references++;
	}
}

// Parse View
// Support for using Constants in place of NPC View IDs.
int npc_parseview(const char* w4, const char* start, const char* buffer, const char* filepath) {
	int val = -1, i = 0;
	char viewid[1024];	// Max size of name from const.txt, see script->read_constdb.

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
			ShowWarning("npc_parseview: Invalid NPC constant '%s' specified in file '%s', line'%d'. Defaulting to INVISIBLE_CLASS. \n", viewid, filepath, strline(buffer,start-buffer));
			val = INVISIBLE_CLASS;
		}
	} else {
		// NPC has an ID specified for view id.
		val = atoi(w4);
	}

	return val;
}

// View is ID
// Checks if given view is an ID or constant.
bool npc_viewisid(const char * viewid)
{
	if(atoi(viewid) != -1)
	{
		// Loop through view, looking for non-numeric character.
		while (*viewid) {
			if (ISDIGIT(*viewid++) == 0) return false;
		}
	}

    return true;
}

//Add then display an npc warp on map
struct npc_data* npc_add_warp(char* name, short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y) {
	int i, flag = 0;
	struct npc_data *nd;

	CREATE(nd, struct npc_data, 1);
	nd->bl.id = npc->get_new_npc_id();
	map->addnpc(from_mapid, nd);
	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = from_mapid;
	nd->bl.x = from_x;
	nd->bl.y = from_y;

	safestrncpy(nd->exname, name, ARRAYLENGTH(nd->exname));
	if (npc->name2id(nd->exname) != NULL)
		flag = 1;

	if (flag == 1)
		snprintf(nd->exname, ARRAYLENGTH(nd->exname), "warp_%d_%d_%d", from_mapid, from_x, from_y);

	for( i = 0; npc->name2id(nd->exname) != NULL; ++i )
		snprintf(nd->exname, ARRAYLENGTH(nd->exname), "warp%d_%d_%d_%d", i, from_mapid, from_x, from_y);
	safestrncpy(nd->name, nd->exname, ARRAYLENGTH(nd->name));

	if( battle_config.warp_point_debug )
		nd->class_ = WARP_DEBUG_CLASS;
	else
		nd->class_ = WARP_CLASS;
	nd->speed = 200;

	nd->u.warp.mapindex = to_mapindex;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = xs;
	nd->bl.type = BL_NPC;
	nd->subtype = WARP;
	npc->setcells(nd);
	map->addblock(&nd->bl);
	status->set_viewdata(&nd->bl, nd->class_);
	nd->ud = &npc->base_ud;
	if( map->list[nd->bl.m].users )
		clif->spawn(&nd->bl);
	strdb_put(npc->name_db, nd->exname, nd);

	return nd;
}

/// Parses a warp npc.
const char* npc_parse_warp(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	int x, y, xs, ys, to_x, to_y, m;
	unsigned short i;
	char mapname[32], to_mapname[32];
	struct npc_data *nd;

	// w1=<from map name>,<fromX>,<fromY>,<facing>
	// w4=<spanx>,<spany>,<to map name>,<toX>,<toY>
	if( sscanf(w1, "%31[^,],%d,%d", mapname, &x, &y) != 3
	||	sscanf(w4, "%d,%d,%31[^,],%d,%d", &xs, &ys, to_mapname, &to_x, &to_y) != 5 )
	{
		ShowError("npc_parse_warp: Invalid warp definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}

	m = map->mapname2mapid(mapname);
	i = mapindex->name2id(to_mapname);
	if( i == 0 )
	{
		ShowError("npc_parse_warp: Unknown destination map in file '%s', line '%d' : %s\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), to_mapname, w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}

	if( m != -1 && ( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) ) {
		ShowError("npc_parse_warp: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d, in file '%s', line '%d'\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys,filepath,strline(buffer,start-buffer));
		return strchr(start,'\n');;//try next
	}
	
	CREATE(nd, struct npc_data, 1);

	nd->bl.id = npc->get_new_npc_id();
	map->addnpc(m, nd);
	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	npc->parsename(nd, w3, start, buffer, filepath);

	if (!battle_config.warp_point_debug)
		nd->class_ = WARP_CLASS;
	else
		nd->class_ = WARP_DEBUG_CLASS;
	nd->speed = 200;

	nd->u.warp.mapindex = i;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;
	npc_warp++;
	nd->bl.type = BL_NPC;
	nd->subtype = WARP;
	npc->setcells(nd);
	map->addblock(&nd->bl);
	status->set_viewdata(&nd->bl, nd->class_);
	nd->ud = &npc->base_ud;
	if( map->list[nd->bl.m].users )
		clif->spawn(&nd->bl);
	strdb_put(npc->name_db, nd->exname, nd);

	return strchr(start,'\n');// continue
}

/// Parses a shop/cashshop npc.
const char* npc_parse_shop(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	//TODO: could be rewritten to NOT need this temp array [ultramage]
	#define MAX_SHOPITEM 100
	struct npc_item_list items[MAX_SHOPITEM];
	char *p;
	int x, y, dir, m, i;
	struct npc_data *nd;
	enum npc_subtype type;

	if( strcmp(w1,"-") == 0 ) {// 'floating' shop?
		x = y = dir = 0;
		m = -1;
	} else {// w1=<map name>,<x>,<y>,<facing>
		char mapname[32];
		if( sscanf(w1, "%31[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4
		||	strchr(w4, ',') == NULL )
		{
			ShowError("npc_parse_shop: Invalid shop definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return strchr(start,'\n');// skip and continue
		}

		m = map->mapname2mapid(mapname);
	}

	if( m != -1 && ( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) ) {
		ShowError("npc_parse_shop: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d, in file '%s', line '%d'\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys,filepath,strline(buffer,start-buffer));
		return strchr(start,'\n');;//try next
	}
	
	if( strcmp(w2,"cashshop") == 0 )
		type = CASHSHOP;
	else
		type = SHOP;

	p = strchr(w4,',');
	for( i = 0; i < ARRAYLENGTH(items) && p; ++i )
	{
		int nameid, value;
		struct item_data* id;
		if( sscanf(p, ",%d:%d", &nameid, &value) != 2 )
		{
			ShowError("npc_parse_shop: Invalid item definition in file '%s', line '%d'. Ignoring the rest of the line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			break;
		}

		if( (id = itemdb->exists(nameid)) == NULL )
		{
			ShowWarning("npc_parse_shop: Invalid sell item in file '%s', line '%d' (id '%d').\n", filepath, strline(buffer,start-buffer), nameid);
			p = strchr(p+1,',');
			continue;
		}

		if( value < 0 )
		{
			if( type == SHOP ) value = id->value_buy;
			else value = 0; // Cashshop doesn't have a "buy price" in the item_db
		}

		if( type == SHOP && value == 0 )
		{ // NPC selling items for free!
			ShowWarning("npc_parse_shop: Item %s [%d] is being sold for FREE in file '%s', line '%d'.\n",
				id->name, nameid, filepath, strline(buffer,start-buffer));
		}
		if( type == SHOP && value*0.75 < id->value_sell*1.24 )
		{// Exploit possible: you can buy and sell back with profit
			ShowWarning("npc_parse_shop: Item %s [%d] discounted buying price (%d->%d) is less than overcharged selling price (%d->%d) in file '%s', line '%d'.\n",
				id->name, nameid, value, (int)(value*0.75), id->value_sell, (int)(id->value_sell*1.24), filepath, strline(buffer,start-buffer));
		}
		//for logs filters, atcommands and iteminfo script command
		if( id->maxchance == 0 )
			id->maxchance = -1; // -1 would show that the item's sold in NPC Shop

		items[i].nameid = nameid;
		items[i].value = value;
		p = strchr(p+1,',');
	}
	if( i == 0 )
	{
		ShowWarning("npc_parse_shop: Ignoring empty shop in file '%s', line '%d'.\n", filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// continue
	}

	CREATE(nd, struct npc_data, 1);
	CREATE(nd->u.shop.shop_item, struct npc_item_list, i);
	memcpy(nd->u.shop.shop_item, items, sizeof(struct npc_item_list)*i);
	nd->u.shop.count = i;
	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->bl.id = npc->get_new_npc_id();
	npc->parsename(nd, w3, start, buffer, filepath);
	nd->class_ = m == -1 ? -1 : npc->parseview(w4, start, buffer, filepath);
	nd->speed = 200;

	++npc_shop;
	nd->bl.type = BL_NPC;
	nd->subtype = type;
	if( m >= 0 ) {// normal shop npc
		map->addnpc(m,nd);
		map->addblock(&nd->bl);
		status->set_viewdata(&nd->bl, nd->class_);
		nd->ud = &npc->base_ud;
		nd->dir = dir;
		if( map->list[nd->bl.m].users )
			clif->spawn(&nd->bl);
	} else {// 'floating' shop?
		map->addiddb(&nd->bl);
	}
	strdb_put(npc->name_db, nd->exname, nd);

	return strchr(start,'\n');// continue
}

void npc_convertlabel_db(struct npc_label_list* label_list, const char *filepath) {
	int i;
	
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
const char* npc_skip_script(const char* start, const char* buffer, const char* filepath)
{
	const char* p;
	int curly_count;

	if( start == NULL )
		return NULL;// nothing to skip

	// initial bracket (assumes the previous part is ok)
	p = strchr(start,'{');
	if( p == NULL )
	{
		ShowError("npc_skip_script: Missing left curly in file '%s', line '%d'.\n", filepath, strline(buffer,start-buffer));
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
				if( *p == '\\' && (unsigned char)p[-1] <= 0x7e )
					++p;// escape sequence (not part of a multibyte character)
				else if( *p == '\0' )
				{
					script->error(buffer, filepath, 0, "Unexpected end of string.", p);
					return NULL;// can't continue
				}
				else if( *p == '\n' )
				{
					script->error(buffer, filepath, 0, "Unexpected newline at string.", p);
					return NULL;// can't continue
				}
			}
		}
		else if( *p == '\0' )
		{// end of buffer
			ShowError("Missing %d right curlys in file '%s', line '%d'.\n", curly_count, filepath, strline(buffer,p-buffer));
			return NULL;// can't continue
		}
	}

	return p+1;// return after the last '}'
}

/// Parses a npc script.
///
/// -%TAB%script%TAB%<NPC Name>%TAB%-1,{<code>}
/// <map name>,<x>,<y>,<facing>%TAB%script%TAB%<NPC Name>%TAB%<sprite id>,{<code>}
/// <map name>,<x>,<y>,<facing>%TAB%script%TAB%<NPC Name>%TAB%<sprite id>,<triggerX>,<triggerY>,{<code>}
const char* npc_parse_script(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath, int options) {
	int x, y, dir = 0, m, xs = 0, ys = 0;	// [Valaris] thanks to fov
	char mapname[32];
	struct script_code *scriptroot;
	int i;
	const char* end;
	const char* script_start;

	struct npc_label_list* label_list;
	int label_list_num;
	struct npc_data* nd;

	if( strcmp(w1, "-") == 0 )
	{// floating npc
		x = 0;
		y = 0;
		m = -1;
	} else {// npc in a map
		if( sscanf(w1, "%31[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ) {
			ShowError("npc_parse_script: Invalid placement format for a script in file '%s', line '%d'. Skipping the rest of file...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return NULL;// unknown format, don't continue
		}
		m = map->mapname2mapid(mapname);
	}

	script_start = strstr(start,",{");
	end = strchr(start,'\n');
	if( strstr(w4,",{") == NULL || script_start == NULL || (end != NULL && script_start > end) )
	{
		ShowError("npc_parse_script: Missing left curly ',{' in file '%s', line '%d'. Skipping the rest of the file.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return NULL;// can't continue
	}
	++script_start;

	end = npc->skip_script(script_start, buffer, filepath);
	if( end == NULL )
		return NULL;// (simple) parse error, don't continue

	scriptroot = script->parse(script_start, filepath, strline(buffer,script_start-buffer), SCRIPT_USE_LABEL_DB);
	label_list = NULL;
	label_list_num = 0;
	if( script->label_count ) {
		CREATE(label_list,struct npc_label_list,script->label_count);
		label_list_num = script->label_count;
		npc->convertlabel_db(label_list,filepath);
	}

	CREATE(nd, struct npc_data, 1);

	if( sscanf(w4, "%*[^,],%d,%d", &xs, &ys) == 2 )
	{// OnTouch area defined
		nd->u.scr.xs = xs;
		nd->u.scr.ys = ys;
	}
	else
	{// no OnTouch area
		nd->u.scr.xs = -1;
		nd->u.scr.ys = -1;
	}

	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	npc->parsename(nd, w3, start, buffer, filepath);
	nd->bl.id = npc->get_new_npc_id();
	nd->class_ = m == -1 ? -1 : npc->parseview(w4, start, buffer, filepath);
	nd->speed = 200;
	nd->u.scr.script = scriptroot;
	nd->u.scr.label_list = label_list;
	nd->u.scr.label_list_num = label_list_num;
	if( options&NPO_TRADER )
		nd->u.scr.trader = true;
	nd->u.scr.shop = NULL;
	
	++npc_script;
	nd->bl.type = BL_NPC;
	nd->subtype = SCRIPT;

	if( m >= 0 ) {
		map->addnpc(m, nd);
		nd->ud = &npc->base_ud;
		nd->dir = dir;
		npc->setcells(nd);
		map->addblock(&nd->bl);
		if( nd->class_ >= 0 ) {
			status->set_viewdata(&nd->bl, nd->class_);
			if( map->list[nd->bl.m].users )
				clif->spawn(&nd->bl);
		}
	} else {
		// we skip map->addnpc, but still add it to the list of ID's
		map->addiddb(&nd->bl);
	}
	strdb_put(npc->name_db, nd->exname, nd);

	//-----------------------------------------
	// Loop through labels to export them as necessary
	for (i = 0; i < nd->u.scr.label_list_num; i++) {
		if (npc->event_export(nd, i)) {
			ShowWarning("npc_parse_script: duplicate event %s::%s in file '%s'.\n",
			             nd->exname, nd->u.scr.label_list[i].name, filepath);
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
			script->run(nd->u.scr.script,ev->pos,0,nd->bl.id);

		}
	}

	return end;
}

/// Duplicate a warp, shop, cashshop or script. [Orcao]
/// warp: <map name>,<x>,<y>,<facing>%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<spanx>,<spany>
/// shop/cashshop/npc: -%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>
/// shop/cashshop/npc: <map name>,<x>,<y>,<facing>%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>
/// npc: -%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>,<triggerX>,<triggerY>
/// npc: <map name>,<x>,<y>,<facing>%TAB%duplicate(<name of target>)%TAB%<NPC Name>%TAB%<sprite id>,<triggerX>,<triggerY>
const char* npc_parse_duplicate(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	int x, y, dir, m, xs = -1, ys = -1;
	char mapname[32];
	char srcname[128];
	int i;
	const char* end;
	size_t length;

	int src_id;
	int type;
	struct npc_data* nd;
	struct npc_data* dnd;

	end = strchr(start,'\n');
	length = strlen(w2);

	// get the npc being duplicated
	if( w2[length-1] != ')' || length <= 11 || length-11 >= sizeof(srcname) )
	{// does not match 'duplicate(%127s)', name is empty or too long
		ShowError("npc_parse_script: bad duplicate name in file '%s', line '%d': %s\n", filepath, strline(buffer,start-buffer), w2);
		return end;// next line, try to continue
	}
	safestrncpy(srcname, w2+10, length-10);

	dnd = npc->name2id(srcname);
	if( dnd == NULL) {
		ShowError("npc_parse_script: original npc not found for duplicate in file '%s', line '%d': %s\n", filepath, strline(buffer,start-buffer), srcname);
		return end;// next line, try to continue
	}
	src_id = dnd->bl.id;
	type = dnd->subtype;

	// get placement
	if( (type==SHOP || type==CASHSHOP || type==SCRIPT) && strcmp(w1, "-") == 0 ) {// floating shop/chashshop/script
		x = y = dir = 0;
		m = -1;
	} else {
		int fields = sscanf(w1, "%31[^,],%d,%d,%d", mapname, &x, &y, &dir);
		if( type == WARP && fields == 3 ) { // <map name>,<x>,<y>
			dir = 0;
		} else if( fields != 4 ) {// <map name>,<x>,<y>,<facing>
			ShowError("npc_parse_duplicate: Invalid placement format for duplicate in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
			return end;// next line, try to continue
		}
		m = map->mapname2mapid(mapname);
	}

	if( m != -1 && ( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) ) {
		ShowError("npc_parse_duplicate: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d, in file '%s', line '%d'\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys,filepath,strline(buffer,start-buffer));
		return end;//try next
	}
	
	if( type == WARP && sscanf(w4, "%d,%d", &xs, &ys) == 2 );// <spanx>,<spany>
	else if( type == SCRIPT && sscanf(w4, "%*[^,],%d,%d", &xs, &ys) == 2);// <sprite id>,<triggerX>,<triggerY>
	else if( type == WARP ) {
		ShowError("npc_parse_duplicate: Invalid span format for duplicate warp in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return end;// next line, try to continue
	}

	CREATE(nd, struct npc_data, 1);

	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	npc->parsename(nd, w3, start, buffer, filepath);
	nd->bl.id = npc->get_new_npc_id();
	nd->class_ = m == -1 ? -1 : npc->parseview(w4, start, buffer, filepath);
	nd->speed = 200;
	nd->src_id = src_id;
	nd->bl.type = BL_NPC;
	nd->subtype = (enum npc_subtype)type;
	switch( type ) {
		case SCRIPT:
			++npc_script;
			nd->u.scr.xs = xs;
			nd->u.scr.ys = ys;
			nd->u.scr.script = dnd->u.scr.script;
			nd->u.scr.label_list = dnd->u.scr.label_list;
			nd->u.scr.label_list_num = dnd->u.scr.label_list_num;
			nd->u.scr.shop = dnd->u.scr.shop;
			nd->u.scr.trader = dnd->u.scr.trader;
			break;

		case SHOP:
		case CASHSHOP:
			++npc_shop;
			nd->u.shop.shop_item = dnd->u.shop.shop_item;
			nd->u.shop.count = dnd->u.shop.count;
			break;

		case WARP:
			++npc_warp;
			if( !battle_config.warp_point_debug )
				nd->class_ = WARP_CLASS;
			else
				nd->class_ = WARP_DEBUG_CLASS;
			nd->u.warp.xs = xs;
			nd->u.warp.ys = ys;
			nd->u.warp.mapindex = dnd->u.warp.mapindex;
			nd->u.warp.x = dnd->u.warp.x;
			nd->u.warp.y = dnd->u.warp.y;
			break;
	}

	//Add the npc to its location
	if( m >= 0 ) {
		map->addnpc(m, nd);
		nd->ud = &npc->base_ud;
		nd->dir = dir;
		npc->setcells(nd);
		map->addblock(&nd->bl);
		if( nd->class_ >= 0 ) {
			status->set_viewdata(&nd->bl, nd->class_);
			if( map->list[nd->bl.m].users )
				clif->spawn(&nd->bl);
		}
	} else {
		// we skip map->addnpc, but still add it to the list of ID's
		map->addiddb(&nd->bl);
	}
	strdb_put(npc->name_db, nd->exname, nd);

	if( type != SCRIPT )
		return end;

	//-----------------------------------------
	// Loop through labels to export them as necessary
	for (i = 0; i < nd->u.scr.label_list_num; i++) {
		if (npc->event_export(nd, i)) {
			ShowWarning("npc_parse_duplicate: duplicate event %s::%s in file '%s'.\n",
			             nd->exname, nd->u.scr.label_list[i].name, filepath);
		}
		npc->timerevent_export(nd, i);
	}

	nd->u.scr.timerid = INVALID_TIMER;

	return end;
}

int npc_duplicate4instance(struct npc_data *snd, int16 m) {
	char newname[NAME_LENGTH];

	if( m == -1 || map->list[m].instance_id == -1 )
		return 1;

	snprintf(newname, ARRAYLENGTH(newname), "dup_%d_%d", map->list[m].instance_id, snd->bl.id);
	if( npc->name2id(newname) != NULL ) { // Name already in use
		ShowError("npc_duplicate4instance: the npcname (%s) is already in use while trying to duplicate npc %s in instance %d.\n", newname, snd->exname, map->list[m].instance_id);
		return 1;
	}

	if( snd->subtype == WARP ) { // Adjust destination, if instanced
		struct npc_data *wnd = NULL; // New NPC
		int dm = map->mapindex2mapid(snd->u.warp.mapindex), im;
		if( dm < 0 ) return 1;

		if( ( im = instance->mapid2imapid(dm, map->list[m].instance_id) ) == -1 ) {
			ShowError("npc_duplicate4instance: warp (%s) leading to instanced map (%s), but instance map is not attached to current instance.\n", map->list[dm].name, snd->exname);
			return 1;
		}

		CREATE(wnd, struct npc_data, 1);
		wnd->bl.id = npc->get_new_npc_id();
		map->addnpc(m, wnd);
		wnd->bl.prev = wnd->bl.next = NULL;
		wnd->bl.m = m;
		wnd->bl.x = snd->bl.x;
		wnd->bl.y = snd->bl.y;
		safestrncpy(wnd->name, "", ARRAYLENGTH(wnd->name));
		safestrncpy(wnd->exname, newname, ARRAYLENGTH(wnd->exname));
		wnd->class_ = WARP_CLASS;
		wnd->speed = 200;
		wnd->u.warp.mapindex = map_id2index(im);
		wnd->u.warp.x = snd->u.warp.x;
		wnd->u.warp.y = snd->u.warp.y;
		wnd->u.warp.xs = snd->u.warp.xs;
		wnd->u.warp.ys = snd->u.warp.ys;
		wnd->bl.type = BL_NPC;
		wnd->subtype = WARP;
		npc->setcells(wnd);
		map->addblock(&wnd->bl);
		status->set_viewdata(&wnd->bl, wnd->class_);
		wnd->ud = &npc->base_ud;
		if( map->list[wnd->bl.m].users )
			clif->spawn(&wnd->bl);
		strdb_put(npc->name_db, wnd->exname, wnd);
	} else {
		static char w1[50], w2[50], w3[50], w4[50];
		const char* stat_buf = "- call from instancing subsystem -\n";

		snprintf(w1, sizeof(w1), "%s,%d,%d,%d", map->list[m].name, snd->bl.x, snd->bl.y, snd->dir);
		snprintf(w2, sizeof(w2), "duplicate(%s)", snd->exname);
		snprintf(w3, sizeof(w3), "%s::%s", snd->name, newname);

		if( snd->u.scr.xs >= 0 && snd->u.scr.ys >= 0 )
			snprintf(w4, sizeof(w4), "%d,%d,%d", snd->class_, snd->u.scr.xs, snd->u.scr.ys); // Touch Area
		else
			snprintf(w4, sizeof(w4), "%d", snd->class_);

		npc->parse_duplicate(w1, w2, w3, w4, stat_buf, stat_buf, "INSTANCING");
	}

	return 0;
}

//Set mapcell CELL_NPC to trigger event later
void npc_setcells(struct npc_data* nd) {
	int16 m = nd->bl.m, x = nd->bl.x, y = nd->bl.y, xs, ys;
	int i,j;

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
			if (map->getcell(m, j, i, CELL_CHKNOPASS))
				continue;
			map->list[m].setcell(m, j, i, CELL_NPC, true);
		}
	}
}

int npc_unsetcells_sub(struct block_list* bl, va_list ap) {
	struct npc_data *nd = (struct npc_data*)bl;
	int id =  va_arg(ap,int);
	if (nd->bl.id == id) return 0;
	npc->setcells(nd);
	return 1;
}

void npc_unsetcells(struct npc_data* nd) {
	int16 m = nd->bl.m, x = nd->bl.x, y = nd->bl.y, xs, ys;
	int i,j, x0, x1, y0, y1;

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
	for(x0 = x-xs; x0 > 0 && map->getcell(m, x0, y, CELL_CHKNPC); x0--);
	for(x1 = x+xs; x1 < map->list[m].xs-1 && map->getcell(m, x1, y, CELL_CHKNPC); x1++);
	for(y0 = y-ys; y0 > 0 && map->getcell(m, x, y0, CELL_CHKNPC); y0--);
	for(y1 = y+ys; y1 < map->list[m].ys-1 && map->getcell(m, x, y1, CELL_CHKNPC); y1++);

	//Erase this npc's cells
	for (i = y-ys; i <= y+ys; i++)
		for (j = x-xs; j <= x+xs; j++)
			map->list[m].setcell(m, j, i, CELL_NPC, false);

	//Re-deploy NPC cells for other nearby npcs.
	map->foreachinarea( npc->unsetcells_sub, m, x0, y0, x1, y1, BL_NPC, nd->bl.id );
}

void npc_movenpc(struct npc_data* nd, int16 x, int16 y)
{
	const int16 m = nd->bl.m;
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
void npc_setdisplayname(struct npc_data* nd, const char* newname)
{
	nullpo_retv(nd);

	safestrncpy(nd->name, newname, sizeof(nd->name));
	if( map->list[nd->bl.m].users )
		clif->charnameack(0, &nd->bl);
}

/// Changes the display class of the npc.
///
/// @param nd Target npc
/// @param class_ New display class
void npc_setclass(struct npc_data* nd, short class_) {
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

// @commands (script based)
int npc_do_atcmd_event(struct map_session_data* sd, const char* command, const char* message, const char* eventname)
{
	struct event_data* ev = (struct event_data*)strdb_get(npc->ev_db, eventname);
	struct npc_data *nd;
	struct script_state *st;
	int i = 0, j = 0, k = 0;
	char *temp;

	nullpo_ret(sd);

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
	script->setd_sub(st, NULL, ".@atcmd_command$", 0, (void *)command, NULL);

	// split atcmd parameters based on spaces
	temp = (char*)aMalloc(strlen(message) + 1);

	for( i = 0; i < ( strlen( message ) + 1 ) && k < 127; i ++ ) {
		if( message[i] == ' ' || message[i] == '\0' ) {
			if( message[ ( i - 1 ) ] == ' ' ) {
				continue; // To prevent "@atcmd [space][space]" and .@atcmd_numparameters return 1 without any parameter.
			}
			temp[k] = '\0';
			k = 0;
			if( temp[0] != '\0' ) {
				script->setd_sub( st, NULL, ".@atcmd_parameters$", j++, (void *)temp, NULL );
			}
		} else {
			temp[k] = message[i];
			k++;
		}
	}

	script->setd_sub(st, NULL, ".@atcmd_numparameters", 0, (void *)__64BPTRSIZE(j), NULL);
	aFree(temp);

	script->run_main(st);
	return 0;
}

/// Parses a function.
/// function%TAB%script%TAB%<function name>%TAB%{<code>}
const char* npc_parse_function(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath)
{
	DBMap* func_db;
	DBData old_data;
	struct script_code *scriptroot;
	const char* end;
	const char* script_start;

	script_start = strstr(start,"\t{");
	end = strchr(start,'\n');
	if( *w4 != '{' || script_start == NULL || (end != NULL && script_start > end) )
	{
		ShowError("npc_parse_function: Missing left curly '%%TAB%%{' in file '%s', line '%d'. Skipping the rest of the file.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return NULL;// can't continue
	}
	++script_start;

	end = npc->skip_script(script_start,buffer,filepath);
	if( end == NULL )
		return NULL;// (simple) parse error, don't continue

	scriptroot = script->parse(script_start, filepath, strline(buffer,start-buffer), SCRIPT_RETURN_EMPTY_SCRIPT);
	if( scriptroot == NULL )// parse error, continue
		return end;

	func_db = script->userfunc_db;
	if (func_db->put(func_db, DB->str2key(w3), DB->ptr2data(scriptroot), &old_data)) {
		struct script_code *oldscript = (struct script_code*)DB->data2ptr(&old_data);
		ShowWarning("npc_parse_function: Overwriting user function [%s] in file '%s', line '%d'.\n", w3, filepath, strline(buffer,start-buffer));
		script->free_vars(oldscript->local.vars);
		aFree(oldscript->script_buf);
		aFree(oldscript);
	}

	return end;
}


/*==========================================
 * Parse Mob 1 - Parse mob list into each map
 * Parse Mob 2 - Actually Spawns Mob
 * [Wizputer]
 *------------------------------------------*/
void npc_parse_mob2(struct spawn_data* mobspawn)
{
	int i;

	for( i = mobspawn->active; i < mobspawn->num; ++i ) {
		struct mob_data* md = mob->spawn_dataset(mobspawn);
		md->spawn = mobspawn;
		md->spawn->active++;
		mob->spawn(md);
	}
}

const char* npc_parse_mob(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath) {
	int num, class_, m,x,y,xs,ys, i,j;
	int mob_lv = -1, ai = -1, size = -1;
	char mapname[32], mobname[NAME_LENGTH];
	struct spawn_data mobspawn, *data;
	struct mob_db* db;

	memset(&mobspawn, 0, sizeof(struct spawn_data));

	mobspawn.state.boss = (strcmp(w2,"boss_monster") == 0 ? 1 : 0);

	// w1=<map name>,<x>,<y>,<xs>,<ys>
	// w3=<mob name>{,<mob level>}
	// w4=<mob id>,<amount>,<delay1>,<delay2>,<event>{,<mob size>,<mob ai>}
	if( sscanf(w1, "%31[^,],%d,%d,%d,%d", mapname, &x, &y, &xs, &ys) < 3
	 || sscanf(w3, "%23[^,],%d", mobname, &mob_lv) < 1
	 || sscanf(w4, "%d,%d,%u,%u,%127[^,],%d,%d[^\t\r\n]", &class_, &num, &mobspawn.delay1, &mobspawn.delay2, mobspawn.eventname, &size, &ai) < 2
	 ) {
		ShowError("npc_parse_mob: Invalid mob definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}
	if( mapindex->name2id(mapname) == 0 ) {
		ShowError("npc_parse_mob: Unknown map '%s' in file '%s', line '%d'.\n", mapname, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}
	m =  map->mapname2mapid(mapname);
	if( m < 0 )//Not loaded on this map-server instance.
		return strchr(start,'\n');// skip and continue
	mobspawn.m = (unsigned short)m;

	if( x < 0 || x >= map->list[mobspawn.m].xs || y < 0 || y >= map->list[mobspawn.m].ys ) {
		ShowError("npc_parse_mob: Spawn coordinates out of range: %s (%d,%d), map size is (%d,%d) - %s %s in file '%s', line '%d'.\n", map->list[mobspawn.m].name, x, y, (map->list[mobspawn.m].xs-1), (map->list[mobspawn.m].ys-1), w1, w3, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	// check monster ID if exists!
	if( mob->db_checkid(class_) == 0 ) {
		ShowError("npc_parse_mob: Unknown mob ID %d in file '%s', line '%d'.\n", class_, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	if( num < 1 || num > 1000 ) {
		ShowError("npc_parse_mob: Invalid number of monsters %d, must be inside the range [1,1000] in file '%s', line '%d'.\n", num, filepath, strline(buffer,start-buffer));
		return strchr(start,'\n');// skip and continue
	}

	if( (mobspawn.state.size < 0 || mobspawn.state.size > 2) && size != -1 ) {
		ShowError("npc_parse_mob: Invalid size number %d for mob ID %d in file '%s', line '%d'.\n", mobspawn.state.size, class_, filepath, strline(buffer, start - buffer));
		return strchr(start, '\n');
	}

	if( (mobspawn.state.ai < 0 || mobspawn.state.ai > 4) && ai != -1 ) {
		ShowError("npc_parse_mob: Invalid ai %d for mob ID %d in file '%s', line '%d'.\n", mobspawn.state.ai, class_, filepath, strline(buffer, start - buffer));
		return strchr(start, '\n');
	}

	if( (mob_lv == 0 || mob_lv > MAX_LEVEL) && mob_lv != -1 ) {
		ShowError("npc_parse_mob: Invalid level %d for mob ID %d in file '%s', line '%d'.\n", mob_lv, class_, filepath, strline(buffer, start - buffer));
		return strchr(start, '\n');
	}

	mobspawn.num = (unsigned short)num;
	mobspawn.active = 0;
	mobspawn.class_ = (short) class_;
	mobspawn.x = (unsigned short)x;
	mobspawn.y = (unsigned short)y;
	mobspawn.xs = (signed short)xs;
	mobspawn.ys = (signed short)ys;
	if (mob_lv > 0 && mob_lv <= MAX_LEVEL)
		mobspawn.level = mob_lv;
	if (size > 0 && size <= 2)
		mobspawn.state.size = size;
	if (ai > 0 && ai <= 4)
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
		return strchr(start,'\n');// skip and continue
	}

	//Use db names instead of the spawn file ones.
	if(battle_config.override_mob_names==1)
		strcpy(mobspawn.name,"--en--");
	else if (battle_config.override_mob_names==2)
		strcpy(mobspawn.name,"--ja--");
	else
		safestrncpy(mobspawn.name, mobname, sizeof(mobspawn.name));

	//Verify dataset.
	if( !mob->parse_dataset(&mobspawn) ) {
		ShowError("npc_parse_mob: Invalid dataset for monster ID %d in file '%s', line '%d'.\n", class_, filepath, strline(buffer,start-buffer));
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
		npc_cache_mob += data->num;

		// check if target map has players
		// (usually shouldn't occur when map server is just starting,
		// but not the case when we do @reloadscript
		if( map->list[data->m].users > 0 ) {
			npc->parse_mob2(data);
		}
	} else {
		data->state.dynamic = false;
		npc->parse_mob2(data);
		npc_delay_mob += data->num;
	}

	npc_mob++;

	return strchr(start,'\n');// continue
}
/*==========================================
 * Set or disable mapflag on map
 * eg : bat_c01	mapflag	battleground	2
 * also chking if mapflag conflict with another
 *------------------------------------------*/
const char* npc_parse_mapflag(char* w1, char* w2, char* w3, char* w4, const char* start, const char* buffer, const char* filepath) {
	int16 m;
	char mapname[32];
	int state = 1;

	// w1=<mapname>
	if( sscanf(w1, "%31[^,]", mapname) != 1 )
	{
		ShowError("npc_parse_mapflag: Invalid mapflag definition in file '%s', line '%d'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}
	m = map->mapname2mapid(mapname);
	if( m < 0 ) {
		ShowWarning("npc_parse_mapflag: Unknown map in file '%s', line '%d' : %s\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", mapname, filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
		return strchr(start,'\n');// skip and continue
	}

	if (w4 && !strcmpi(w4, "off"))
		state = 0;	//Disable mapflag rather than enable it. [Skotlex]

	if (!strcmpi(w3, "nosave")) {
		char savemap[32];
		int savex, savey;
		if (state == 0)
			; //Map flag disabled.
		else if (w4 && !strcmpi(w4, "SavePoint")) {
			map->list[m].save.map = 0;
			map->list[m].save.x = -1;
			map->list[m].save.y = -1;
		} else if (sscanf(w4, "%31[^,],%d,%d", savemap, &savex, &savey) == 3) {
			map->list[m].save.map = mapindex->name2id(savemap);
			map->list[m].save.x = savex;
			map->list[m].save.y = savey;
			if (!map->list[m].save.map) {
				ShowWarning("npc_parse_mapflag: Specified save point map '%s' for mapflag 'nosave' not found in file '%s', line '%d', using 'SavePoint'.\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", savemap, filepath, strline(buffer,start-buffer), w1, w2, w3, w4);
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
		}
		if( state && map->list[m].flag.battleground ) {
			map->list[m].flag.battleground = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and BattleGround flags for the same map! Removing BattleGround flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
		}
		if( state && (zone = strdb_get(map->zone_db, MAP_ZONE_PVP_NAME)) && map->list[m].zone != zone ) {
			map->zone_change(m,zone,start,buffer,filepath);
		} else if ( !state ) {
			map->list[m].zone = &map->zone_pk;
		}
	}
	else if (!strcmpi(w3,"pvp_noparty"))
		map->list[m].flag.pvp_noparty=state;
	else if (!strcmpi(w3,"pvp_noguild"))
		map->list[m].flag.pvp_noguild=state;
	else if (!strcmpi(w3, "pvp_nightmaredrop")) {
		char drop_arg1[16], drop_arg2[16];
		int drop_per = 0;
		if (sscanf(w4, "%[^,],%[^,],%d", drop_arg1, drop_arg2, &drop_per) == 3) {
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
		}
		if( state && map->list[m].flag.battleground ) {
			map->list[m].flag.battleground = 0;
			ShowWarning("npc_parse_mapflag: You can't set GvG and BattleGround flags for the same map! Removing BattleGround flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
		}
		if( state && (zone = strdb_get(map->zone_db, MAP_ZONE_GVG_NAME)) && map->list[m].zone != zone ) {
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
		if( state ) {
			if( sscanf(w4, "%d", &state) == 1 )
				map->list[m].flag.battleground = state;
			else
				map->list[m].flag.battleground = 1; // Default value
		} else
			map->list[m].flag.battleground = 0;

		if( map->list[m].flag.battleground && map->list[m].flag.pvp ) {
			map->list[m].flag.pvp = 0;
			ShowWarning("npc_parse_mapflag: You can't set PvP and BattleGround flags for the same map! Removing PvP flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
		}
		if( map->list[m].flag.battleground && (map->list[m].flag.gvg || map->list[m].flag.gvg_dungeon || map->list[m].flag.gvg_castle) ) {
			map->list[m].flag.gvg = 0;
			map->list[m].flag.gvg_dungeon = 0;
			map->list[m].flag.gvg_castle = 0;
			ShowWarning("npc_parse_mapflag: You can't set GvG and BattleGround flags for the same map! Removing GvG flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
		}
		
		if( state && (zone = strdb_get(map->zone_db, MAP_ZONE_BG_NAME)) && map->list[m].zone != zone ) {
			map->zone_change(m,zone,start,buffer,filepath);
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
			if (sscanf(w4, "%d", &state) == 1)
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
		
		if( modifier[0] == '\0' ) {
			ShowWarning("npc_parse_mapflag: Missing 5th param for 'adjust_unit_duration' flag! removing flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
		} else if( !( skill_id = skill->name2id(skill_name) ) || !skill->get_unit_id( skill->name2id(skill_name), 0) ) {
			ShowWarning("npc_parse_mapflag: Unknown skill (%s) for 'adjust_unit_duration' flag! removing flag from %s in file '%s', line '%d'.\n",skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
		} else if ( atoi(modifier) < 1 || atoi(modifier) > USHRT_MAX ) {
			ShowWarning("npc_parse_mapflag: Invalid modifier '%d' for skill '%s' for 'adjust_unit_duration' flag! removing flag from %s in file '%s', line '%d'.\n", atoi(modifier), skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
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
				
		if( modifier[0] == '\0' ) {
			ShowWarning("npc_parse_mapflag: Missing 5th param for 'adjust_skill_damage' flag! removing flag from %s in file '%s', line '%d'.\n", map->list[m].name, filepath, strline(buffer,start-buffer));
		} else if( !( skill_id = skill->name2id(skill_name) ) ) {
			ShowWarning("npc_parse_mapflag: Unknown skill (%s) for 'adjust_skill_damage' flag! removing flag from %s in file '%s', line '%d'.\n", skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
		} else if ( atoi(modifier) < 1 || atoi(modifier) > USHRT_MAX ) {
			ShowWarning("npc_parse_mapflag: Invalid modifier '%d' for skill '%s' for 'adjust_skill_damage' flag! removing flag from %s in file '%s', line '%d'.\n", atoi(modifier), skill_name, map->list[m].name, filepath, strline(buffer,start-buffer));
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
	} else
		ShowError("npc_parse_mapflag: unrecognized mapflag '%s' in file '%s', line '%d'.\n", w3, filepath, strline(buffer,start-buffer));

	return strchr(start,'\n');// continue
}

//Read file and create npc/func/mapflag/monster... accordingly.
//@runOnInit should we exec OnInit when it's done ?
int npc_parsesrcfile(const char* filepath, bool runOnInit) {
	int16 m, x, y;
	int lines = 0;
	FILE* fp;
	size_t len;
	char* buffer;
	const char* p;

	// read whole file to buffer
	fp = fopen(filepath, "rb");
	if( fp == NULL )
	{
		ShowError("npc_parsesrcfile: File not found '%s'.\n", filepath);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	buffer = (char*)aMalloc(len+1);
	fseek(fp, 0, SEEK_SET);
	len = fread(buffer, sizeof(char), len, fp);
	buffer[len] = '\0';
	if( ferror(fp) )
	{
		ShowError("npc_parsesrcfile: Failed to read file '%s' - %s\n", filepath, strerror(errno));
		aFree(buffer);
		fclose(fp);
		return -1;
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
		fclose(fp);
		return -1;
	}

	// parse buffer
	for( p = script->skip_space(buffer); p && *p ; p = script->skip_space(p) )
	{
		int pos[9];
		char w1[2048], w2[2048], w3[2048], w4[2048];
		int i, count;
		lines++;

		// w1<TAB>w2<TAB>w3<TAB>w4
		count = sv->parse(p, (int)(len+buffer-p), 0, '\t', pos, ARRAYLENGTH(pos), (e_svopt)(SV_TERMINATE_LF|SV_TERMINATE_CRLF));
		if( count < 0 )
		{
			ShowError("npc_parsesrcfile: Parse error in file '%s', line '%d'. Stopping...\n", filepath, strline(buffer,p-buffer));
			break;
		}
		// fill w1
		if( pos[3]-pos[2] > ARRAYLENGTH(w1)-1 )
			ShowWarning("npc_parsesrcfile: w1 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[3]-pos[2], filepath, strline(buffer,p-buffer));
		i = min(pos[3]-pos[2], ARRAYLENGTH(w1)-1);
		memcpy(w1, p+pos[2], i*sizeof(char));
		w1[i] = '\0';
		// fill w2
		if( pos[5]-pos[4] > ARRAYLENGTH(w2)-1 )
			ShowWarning("npc_parsesrcfile: w2 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[5]-pos[4], filepath, strline(buffer,p-buffer));
		i = min(pos[5]-pos[4], ARRAYLENGTH(w2)-1);
		memcpy(w2, p+pos[4], i*sizeof(char));
		w2[i] = '\0';
		// fill w3
		if( pos[7]-pos[6] > ARRAYLENGTH(w3)-1 )
			ShowWarning("npc_parsesrcfile: w3 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[7]-pos[6], filepath, strline(buffer,p-buffer));
		i = min(pos[7]-pos[6], ARRAYLENGTH(w3)-1);
		memcpy(w3, p+pos[6], i*sizeof(char));
		w3[i] = '\0';
		// fill w4 (to end of line)
		if( pos[1]-pos[8] > ARRAYLENGTH(w4)-1 )
			ShowWarning("npc_parsesrcfile: w4 truncated, too much data (%d) in file '%s', line '%d'.\n", pos[1]-pos[8], filepath, strline(buffer,p-buffer));
		if( pos[8] != -1 )
		{
			i = min(pos[1]-pos[8], ARRAYLENGTH(w4)-1);
			memcpy(w4, p+pos[8], i*sizeof(char));
			w4[i] = '\0';
		}
		else
			w4[0] = '\0';

		if( count < 3 )
		{// Unknown syntax
			ShowError("npc_parsesrcfile: Unknown syntax in file '%s', line '%d'. Stopping...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
			break;
		}

		if( strcmp(w1,"-") != 0 && strcmp(w1,"function") != 0 )
		{// w1 = <map name>,<x>,<y>,<facing>
			char mapname[MAP_NAME_LENGTH*2];
			x = y = 0;
			sscanf(w1,"%23[^,],%hd,%hd[^,]",mapname,&x,&y);
			if( !mapindex->name2id(mapname) )
			{// Incorrect map, we must skip the script info...
				ShowError("npc_parsesrcfile: Unknown map '%s' in file '%s', line '%d'. Skipping line...\n", mapname, filepath, strline(buffer,p-buffer));
				if( strcmp(w2,"script") == 0 && count > 3 )
				{
					if((p = npc->skip_script(p,buffer,filepath)) == NULL)
					{
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
			m = map->mapname2mapid(mapname);
			if( m < 0 ) {
				// "mapname" is not assigned to this server, we must skip the script info...
				if( strcmp(w2,"script") == 0 && count > 3 )
				{
					if((p = npc->skip_script(p,buffer,filepath)) == NULL)
					{
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
			if (x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys) {
				ShowError("npc_parsesrcfile: Unknown coordinates ('%d', '%d') for map '%s' in file '%s', line '%d'. Skipping line...\n", x, y, mapname, filepath, strline(buffer,p-buffer));
				if( strcmp(w2,"script") == 0 && count > 3 )
				{
					if((p = npc->skip_script(p,buffer,filepath)) == NULL)
					{
						break;
					}
				}
				p = strchr(p,'\n');// next line
				continue;
			}
		}

		if( strcmp(w2,"warp") == 0 && count > 3 )
		{
			p = npc->parse_warp(w1,w2,w3,w4, p, buffer, filepath);
		}
		else if( (strcmp(w2,"shop") == 0 || strcmp(w2,"cashshop") == 0) && count > 3 )
		{
			p = npc->parse_shop(w1,w2,w3,w4, p, buffer, filepath);
		}
		else if( strcmp(w2,"script") == 0 && count > 3 )
		{
			if( strcmp(w1,"function") == 0 ) {
				p = npc->parse_function(w1, w2, w3, w4, p, buffer, filepath);
			} else {
#ifdef ENABLE_CASE_CHECK
				if( strcasecmp(w1, "function") == 0 ) DeprecationWarning("npc_parsesrcfile", w1, "function", filepath, strline(buffer, p-buffer)); // TODO
#endif // ENABLE_CASE_CHECK
				p = npc->parse_script(w1,w2,w3,w4, p, buffer, filepath,runOnInit?NPO_ONINIT:NPO_NONE);
			}
		}
		else if( strcmp(w2,"trader") == 0 && count > 3 ) {
			p = npc->parse_script(w1,w2,w3,w4, p, buffer, filepath,(runOnInit?NPO_ONINIT:NPO_NONE)|NPO_TRADER);
		}
		else if( (i=0, sscanf(w2,"duplicate%n",&i), (i > 0 && w2[i] == '(')) && count > 3 )
		{
			p = npc->parse_duplicate(w1,w2,w3,w4, p, buffer, filepath);
		}
		else if( (strcmp(w2,"monster") == 0 || strcmp(w2,"boss_monster") == 0) && count > 3 )
		{
			p = npc->parse_mob(w1, w2, w3, w4, p, buffer, filepath);
		}
		else if( strcmp(w2,"mapflag") == 0 && count >= 3 )
		{
			p = npc->parse_mapflag(w1, w2, trim(w3), trim(w4), p, buffer, filepath);
		}
		else
		{
#ifdef ENABLE_CASE_CHECK
			if( strcasecmp(w2, "warp") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "warp", filepath, strline(buffer, p-buffer)); } // TODO
			else if( strcasecmp(w2,"shop") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "shop", filepath, strline(buffer, p-buffer)); } // TODO
			else if( strcasecmp(w2,"cashshop") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "cashshop", filepath, strline(buffer, p-buffer)); } // TODO
			else if( strcasecmp(w2, "script") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "script", filepath, strline(buffer, p-buffer)); } // TODO
			else if( strcasecmp(w2,"trader") == 0 ) DeprecationWarning("npc_parsesrcfile", w2, "trader", filepath, strline(buffer, p-buffer)) // TODO
			else if( strncasecmp(w2, "duplicate", 9) == 0 ) {
				char temp[10];
				safestrncpy(temp, w2, 10);
				DeprecationWarning("npc_parsesrcfile", temp, "duplicate", filepath, strline(buffer, p-buffer)); // TODO
			}
			else if( strcasecmp(w2,"monster") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "monster", filepath, strline(buffer, p-buffer)); } // TODO:
			else if( strcasecmp(w2,"boss_monster") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "boss_monster", filepath, strline(buffer, p-buffer)); } // TODO
			else if( strcasecmp(w2, "mapflag") == 0 ) { DeprecationWarning("npc_parsesrcfile", w2, "mapflag", filepath, strline(buffer, p-buffer)); } // TODO
			else
#endif // ENABLE_CASE_CHECK
			ShowError("npc_parsesrcfile: Unable to parse, probably a missing or extra TAB in file '%s', line '%d'. Skipping line...\n * w1=%s\n * w2=%s\n * w3=%s\n * w4=%s\n", filepath, strline(buffer,p-buffer), w1, w2, w3, w4);
			p = strchr(p,'\n');// skip and continue
		}
	}
	aFree(buffer);

	return 0;
}

int npc_script_event(struct map_session_data* sd, enum npce_event type)
{
	int i;
	if (type == NPCE_MAX)
		return 0;
	if (!sd) {
		ShowError("npc_script_event: NULL sd. Event Type %d\n", type);
		return 0;
	}
	for (i = 0; i<script_event[type].event_count; i++)
		npc->event_sub(sd,script_event[type].event[i],script_event[type].event_name[i]);
	return i;
}

void npc_read_event_script(void)
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

	for (i = 0; i < NPCE_MAX; i++)
	{
		DBIterator* iter;
		DBKey key;
		DBData *data;

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
				DeprecationWarning2("npc_read_event_script", p, name, config[i].event_name); // TODO
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
int npc_path_db_clear_sub(DBKey key, DBData *data, va_list args)
{
	struct npc_path_data *npd = DB->data2ptr(data);
	if (npd->path)
		aFree(npd->path);
	return 0;
}

/**
 * @see DBApply
 */
int npc_ev_label_db_clear_sub(DBKey key, DBData *data, va_list args)
{
	struct linkdb_node **label_linkdb = DB->data2ptr(data);
	linkdb_final(label_linkdb); // linked data (struct event_data*) is freed when clearing ev_db
	return 0;
}

//Clear then reload npcs files
int npc_reload(void) {
	struct npc_src_list *nsl;
	int16 m, i;
	int npc_new_min = npc_id;
	struct s_mapiterator* iter;
	struct block_list* bl;

	/* clear guild flag cache */
	guild->flags_clear();

	npc->path_db->clear(npc->path_db, npc->path_db_clear_sub);

	db_clear(npc->name_db);
	db_clear(npc->ev_db);
	npc->ev_label_db->clear(npc->ev_label_db, npc->ev_label_db_clear_sub);

	npc_last_npd = NULL;
	npc_last_path = NULL;
	npc_last_ref = NULL;
	
	//Remove all npcs/mobs. [Skotlex]

	iter = mapit_geteachiddb();
	for( bl = (struct block_list*)mapit->first(iter); mapit->exists(iter); bl = (struct block_list*)mapit->next(iter) ) {
		switch(bl->type) {
			case BL_NPC:
				if( bl->id != npc->fake_nd->bl.id )// don't remove fake_nd
					npc->unload((struct npc_data *)bl, false);
				break;
			case BL_MOB:
				unit->free(bl,CLR_OUTSIGHT);
				break;
		}
	}
	mapit->free(iter);

	if(battle_config.dynamic_mobs) {// dynamic check by [random]
		for (m = 0; m < map->count; m++) {
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

	npc_warp = npc_shop = npc_script = 0;
	npc_mob = npc_cache_mob = npc_delay_mob = 0;

	// reset mapflags
	map->flags_init();

	//TODO: the following code is copy-pasted from do_init_npc(); clean it up
	// Reloading npcs now
	for (nsl = npc->src_files; nsl; nsl = nsl->next) {
		ShowStatus("Loading NPC file: %s"CL_CLL"\r", nsl->name);
		npc->parsesrcfile(nsl->name,false);
	}
	ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:"CL_CLL"\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Warps\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Shops\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Scripts\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Spawn sets\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Mobs Cached\n"
		"\t-'"CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
		npc_id - npc_new_min, npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);
		
	itemdb->name_constants();
	
	instance->reload();

	map->zone_init();
	
	npc->motd = npc->name2id("HerculesMOTD"); /* [Ind/Hercules] */
	
	//Re-read the NPC Script Events cache.
	npc->read_event_script();

	/* refresh guild castle flags on both woe setups */
	npc->event_doall("OnAgitInit");
	npc->event_doall("OnAgitInit2");

	//Execute the OnInit event for freshly loaded npcs. [Skotlex]
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n",npc->event_doall("OnInit"));

	npc->market_fromsql();/* after OnInit */
	
	// Execute rest of the startup events if connected to char-server. [Lance]
	if(!intif->CheckForCharServer()){
		ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc->event_doall("OnInterIfInit"));
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc->event_doall("OnInterIfInitOnce"));
	}
	return 0;
}

//Unload all npc in the given file
bool npc_unloadfile( const char* filepath ) {
	DBIterator * iter = db_iterator(npc->name_db);
	struct npc_data* nd = NULL;
	bool found = false;

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

void do_clear_npc(void) {
	db_clear(npc->name_db);
	db_clear(npc->ev_db);
	npc->ev_label_db->clear(npc->ev_label_db, npc->ev_label_db_clear_sub);
}

/*==========================================
 * Destructor
 *------------------------------------------*/
int do_final_npc(void) {
	db_destroy(npc->ev_db);
	npc->ev_label_db->destroy(npc->ev_label_db, npc->ev_label_db_clear_sub);
	db_destroy(npc->name_db);
	npc->path_db->destroy(npc->path_db, npc->path_db_clear_sub);
	ers_destroy(npc->timer_event_ers);
	npc->clearsrcfile();

	return 0;
}

void npc_debug_warps_sub(struct npc_data* nd) {
	int16 m;
	if (nd->bl.type != BL_NPC || nd->subtype != WARP || nd->bl.m < 0)
		return;

	m = map->mapindex2mapid(nd->u.warp.mapindex);
	if (m < 0) return; //Warps to another map, nothing to do about it.
	if (nd->u.warp.x == 0 && nd->u.warp.y == 0) return; // random warp

	if (map->getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNPC)) {
		ShowWarning("Warp %s at %s(%d,%d) warps directly on top of an area npc at %s(%d,%d)\n",
			nd->name,
			map->list[nd->bl.m].name, nd->bl.x, nd->bl.y,
			map->list[m].name, nd->u.warp.x, nd->u.warp.y
			);
	}
	if (map->getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNOPASS)) {
		ShowWarning("Warp %s at %s(%d,%d) warps to a non-walkable tile at %s(%d,%d)\n",
			nd->name,
			map->list[nd->bl.m].name, nd->bl.x, nd->bl.y,
			map->list[m].name, nd->u.warp.x, nd->u.warp.y
			);
	}
}

static void npc_debug_warps(void) {
	int16 m, i;
	for (m = 0; m < map->count; m++)
		for (i = 0; i < map->list[m].npc_num; i++)
			npc->debug_warps_sub(map->list[m].npc[i]);
}

/*==========================================
 * npc initialization
 *------------------------------------------*/
int do_init_npc(bool minimal) {
	struct npc_src_list *file;
	int i;

	memset(&npc->base_ud, 0, sizeof( struct unit_data) );
	npc->base_ud.bl             = NULL;
	npc->base_ud.walktimer      = INVALID_TIMER;
	npc->base_ud.skilltimer     = INVALID_TIMER;
	npc->base_ud.attacktimer    = INVALID_TIMER;
	npc->base_ud.attackabletime =
	npc->base_ud.canact_tick    =
	npc->base_ud.canmove_tick   = timer->gettick();
	
	//Stock view data for normal npcs.
	memset(&npc_viewdb, 0, sizeof(npc_viewdb));

	npc_viewdb[0].class_ = INVISIBLE_CLASS; //Invisible class is stored here.
	for( i = 1; i < MAX_NPC_CLASS; i++ )
		npc_viewdb[i].class_ = i;
	for( i = MAX_NPC_CLASS2_START; i < MAX_NPC_CLASS2_END; i++ )
		npc_viewdb2[i - MAX_NPC_CLASS2_START].class_ = i;

	npc->ev_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, EVENT_NAME_LENGTH);
	npc->ev_label_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, NAME_LENGTH);
	npc->name_db = strdb_alloc(DB_OPT_BASE, NAME_LENGTH);
	npc->path_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, 0);

	npc_last_npd = NULL;
	npc_last_path = NULL;
	npc_last_ref = NULL;
	
	if (!minimal) {
		npc->timer_event_ers = ers_new(sizeof(struct timer_event_data),"clif.c::timer_event_ers",ERS_OPT_NONE);

		// process all npc files
		ShowStatus("Loading NPCs...\r");
		for( file = npc->src_files; file != NULL; file = file->next ) {
			ShowStatus("Loading NPC file: %s"CL_CLL"\r", file->name);
			npc->parsesrcfile(file->name,false);
		}
		ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:"CL_CLL"\n"
			"\t-'"CL_WHITE"%d"CL_RESET"' Warps\n"
			"\t-'"CL_WHITE"%d"CL_RESET"' Shops\n"
			"\t-'"CL_WHITE"%d"CL_RESET"' Scripts\n"
			"\t-'"CL_WHITE"%d"CL_RESET"' Spawn sets\n"
			"\t-'"CL_WHITE"%d"CL_RESET"' Mobs Cached\n"
			"\t-'"CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
			npc_id - START_NPC_NUM, npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);
	}
	
	itemdb->name_constants();

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
	npc->fake_nd = (struct npc_data *)aCalloc(1,sizeof(struct npc_data));
	npc->fake_nd->bl.m = -1;
	npc->fake_nd->bl.id = npc->get_new_npc_id();
	npc->fake_nd->class_ = -1;
	npc->fake_nd->speed = 200;
	strcpy(npc->fake_nd->name,"FAKE_NPC");
	memcpy(npc->fake_nd->exname, npc->fake_nd->name, 9);

	npc_script++;
	npc->fake_nd->bl.type = BL_NPC;
	npc->fake_nd->subtype = SCRIPT;

	strdb_put(npc->name_db, npc->fake_nd->exname, npc->fake_nd);
	npc->fake_nd->u.scr.timerid = INVALID_TIMER;
	map->addiddb(&npc->fake_nd->bl);
	// End of initialization

	return 0;
}
void npc_defaults(void) {
	npc = &npc_s;

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
	npc->parsename = npc_parsename;
	npc->parseview = npc_parseview;
	npc->viewisid = npc_viewisid;
	npc->add_warp = npc_add_warp;
	npc->parse_warp = npc_parse_warp;
	npc->parse_shop = npc_parse_shop;
	npc->convertlabel_db = npc_convertlabel_db;
	npc->skip_script = npc_skip_script;
	npc->parse_script = npc_parse_script;
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
	npc->parsesrcfile = npc_parsesrcfile;
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
	npc->trader_open = npc_trader_open;
	npc->market_fromsql = npc_market_fromsql;
	npc->market_tosql = npc_market_tosql;
	npc->market_delfromsql = npc_market_delfromsql;
	npc->market_delfromsql_sub = npc_market_delfromsql_sub;
}
