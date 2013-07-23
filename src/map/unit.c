// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/showmsg.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/random.h"

#include "map.h"
#include "path.h"
#include "pc.h"
#include "mob.h"
#include "pet.h"
#include "homunculus.h"
#include "instance.h"
#include "mercenary.h"
#include "elemental.h"
#include "skill.h"
#include "clif.h"
#include "duel.h"
#include "npc.h"
#include "guild.h"
#include "status.h"
#include "unit.h"
#include "battle.h"
#include "battleground.h"
#include "chat.h"
#include "trade.h"
#include "vending.h"
#include "party.h"
#include "intif.h"
#include "chrif.h"
#include "script.h"
#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const short dirx[8]={0,-1,-1,-1,0,1,1,1};
const short diry[8]={1,1,0,-1,-1,-1,0,1};

/**
 * Returns the unit_data for the given block_list. If the object is using
 * shared unit_data (i.e. in case of BL_NPC), it returns the shared data.
 * @param bl block_list to process
 * @return a pointer to the given object's unit_data
 **/
struct unit_data* unit_bl2ud(struct block_list *bl) {
	if( bl == NULL) return NULL;
	if( bl->type == BL_PC)  return &((struct map_session_data*)bl)->ud;
	if( bl->type == BL_MOB) return &((struct mob_data*)bl)->ud;
	if( bl->type == BL_PET) return &((struct pet_data*)bl)->ud;
	if( bl->type == BL_NPC) return ((struct npc_data*)bl)->ud;
	if( bl->type == BL_HOM) return &((struct homun_data*)bl)->ud;
	if( bl->type == BL_MER) return &((struct mercenary_data*)bl)->ud;
	if( bl->type == BL_ELEM) return &((struct elemental_data*)bl)->ud;
	return NULL;
}

/**
 * Returns the unit_data for the given block_list. If the object is using
 * shared unit_data (i.e. in case of BL_NPC), it recreates a copy of the
 * data so that it's safe to modify.
 * @param bl block_list to process
 * @return a pointer to the given object's unit_data
 */
struct unit_data* unit_bl2ud2(struct block_list *bl) {
	if( bl && bl->type == BL_NPC && ((struct npc_data*)bl)->ud == &npc_base_ud ) {
		struct npc_data *nd = (struct npc_data *)bl;
		nd->ud = NULL;
		CREATE(nd->ud, struct unit_data, 1);
		unit_dataset(&nd->bl);
	}
	return unit_bl2ud(bl);
}

static int unit_attack_timer(int tid, unsigned int tick, int id, intptr_t data);
static int unit_walktoxy_timer(int tid, unsigned int tick, int id, intptr_t data);

int unit_walktoxy_sub(struct block_list *bl)
{
	int i;
	struct walkpath_data wpd;
	struct unit_data *ud = NULL;

	nullpo_retr(1, bl);
	ud = unit_bl2ud(bl);
	if(ud == NULL) return 0;

	if( !path_search(&wpd,bl->m,bl->x,bl->y,ud->to_x,ud->to_y,ud->state.walk_easy,CELL_CHKNOPASS) )
		return 0;

	memcpy(&ud->walkpath,&wpd,sizeof(wpd));

	if (ud->target_to && ud->chaserange>1) {
		//Generally speaking, the walk path is already to an adjacent tile
		//so we only need to shorten the path if the range is greater than 1.
		uint8 dir;
		//Trim the last part of the path to account for range,
		//but always move at least one cell when requested to move.
		for (i = ud->chaserange*10; i > 0 && ud->walkpath.path_len>1;) {
		   ud->walkpath.path_len--;
			dir = ud->walkpath.path[ud->walkpath.path_len];
			if(dir&1)
				i -= MOVE_DIAGONAL_COST;
			else
				i -= MOVE_COST;
			ud->to_x -= dirx[dir];
			ud->to_y -= diry[dir];
		}
	}

	ud->state.change_walk_target=0;

	if (bl->type == BL_PC) {
		((TBL_PC *)bl)->head_dir = 0;
		clif->walkok((TBL_PC*)bl);
	}
	clif->move(ud);

	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		i = -1;
	else if(ud->walkpath.path[ud->walkpath.path_pos]&1)
		i = iStatus->get_speed(bl)*MOVE_DIAGONAL_COST/MOVE_COST;
	else
		i = iStatus->get_speed(bl);
	if( i > 0)
		ud->walktimer = iTimer->add_timer(iTimer->gettick()+i,unit_walktoxy_timer,bl->id,i);
	return 1;
}

static int unit_walktoxy_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	int i;
	int x,y,dx,dy;
	uint8 dir;
	struct block_list       *bl;
	struct map_session_data *sd;
	struct mob_data         *md;
	struct unit_data        *ud;
	struct mercenary_data   *mrd;

	bl = iMap->id2bl(id);
	if(bl == NULL)
		return 0;
	sd = BL_CAST(BL_PC, bl);
	md = BL_CAST(BL_MOB, bl);
	mrd = BL_CAST(BL_MER, bl);
	ud = unit_bl2ud(bl);

	if(ud == NULL) return 0;

	if(ud->walktimer != tid){
		ShowError("unit_walk_timer mismatch %d != %d\n",ud->walktimer,tid);
		return 0;
	}
	ud->walktimer = INVALID_TIMER;
	if (bl->prev == NULL) return 0; // Stop moved because it is missing from the block_list

	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		return 0;

	if(ud->walkpath.path[ud->walkpath.path_pos]>=8)
		return 1;
	x = bl->x;
	y = bl->y;

	dir = ud->walkpath.path[ud->walkpath.path_pos];
	ud->dir = dir;

	dx = dirx[(int)dir];
	dy = diry[(int)dir];

	if(iMap->getcell(bl->m,x+dx,y+dy,CELL_CHKNOPASS))
		return unit_walktoxy_sub(bl);

	//Refresh view for all those we lose sight
	iMap->foreachinmovearea(clif->outsight, bl, AREA_SIZE, dx, dy, sd?BL_ALL:BL_PC, bl);

	x += dx;
	y += dy;
	iMap->moveblock(bl, x, y, tick);
	ud->walk_count++; //walked cell counter, to be used for walk-triggered skills. [Skotlex]
	status_change_end(bl, SC_ROLLINGCUTTER, INVALID_TIMER); //If you move, you lose your counters. [malufett]

	if (bl->x != x || bl->y != y || ud->walktimer != INVALID_TIMER)
		return 0; //iMap->moveblock has altered the object beyond what we expected (moved/warped it)

	ud->walktimer = -2; // arbitrary non-INVALID_TIMER value to make the clif code send walking packets
	iMap->foreachinmovearea(clif->insight, bl, AREA_SIZE, -dx, -dy, sd?BL_ALL:BL_PC, bl);
	ud->walktimer = INVALID_TIMER;

	if(sd) {
		if( sd->touching_id )
			npc_touchnext_areanpc(sd,false);
		if(iMap->getcell(bl->m,x,y,CELL_CHKNPC)) {
			npc_touch_areanpc(sd,bl->m,x,y);
			if (bl->prev == NULL) //Script could have warped char, abort remaining of the function.
				return 0;
		} else
			sd->areanpc_id=0;

		if( sd->md && !check_distance_bl(&sd->bl, &sd->md->bl, MAX_MER_DISTANCE) )
		{
			// mercenary should be warped after being 3 seconds too far from the master [greenbox]
			if (sd->md->masterteleport_timer == 0)
			{
				sd->md->masterteleport_timer = iTimer->gettick();
			}
			else if (DIFF_TICK(iTimer->gettick(), sd->md->masterteleport_timer) > 3000)
			{
				sd->md->masterteleport_timer = 0;
				unit_warp( &sd->md->bl, sd->bl.m, sd->bl.x, sd->bl.y, CLR_TELEPORT );
			}
		}
		else if( sd->md )
		{
			// reset the tick, he is not far anymore
			sd->md->masterteleport_timer = 0;
		}
	} else if (md) {
		if( iMap->getcell(bl->m,x,y,CELL_CHKNPC) ) {
			if( npc_touch_areanpc2(md) ) return 0; // Warped
		} else
			md->areanpc_id = 0;
		if (md->min_chase > md->db->range3) md->min_chase--;
		//Walk skills are triggered regardless of target due to the idle-walk mob state.
		//But avoid triggering on stop-walk calls.
		if(tid != INVALID_TIMER &&
			!(ud->walk_count%WALK_SKILL_INTERVAL) &&
			mobskill_use(md, tick, -1))
	  	{
			if (!(ud->skill_id == NPC_SELFDESTRUCTION && ud->skilltimer != INVALID_TIMER))
			{	//Skill used, abort walking
				clif->fixpos(bl); //Fix position as walk has been cancelled.
				return 0;
			}
			//Resend walk packet for proper Self Destruction display.
			clif->move(ud);
		}
	}
	else if( mrd && mrd->master )
	{
		if (!check_distance_bl(&mrd->master->bl, bl, MAX_MER_DISTANCE))
		{
			// mercenary should be warped after being 3 seconds too far from the master [greenbox]
			if (mrd->masterteleport_timer == 0)
			{
				mrd->masterteleport_timer = iTimer->gettick();
			}
			else if (DIFF_TICK(iTimer->gettick(), mrd->masterteleport_timer) > 3000)
			{
				mrd->masterteleport_timer = 0;
				unit_warp( bl, mrd->master->bl.id, mrd->master->bl.x, mrd->master->bl.y, CLR_TELEPORT );
			}
		}
		else
		{
			mrd->masterteleport_timer = 0;
		}
	}

	if(tid == INVALID_TIMER) //A directly invoked timer is from battle_stop_walking, therefore the rest is irrelevant.
		return 0;

	if(ud->state.change_walk_target)
		return unit_walktoxy_sub(bl);

	ud->walkpath.path_pos++;
	if(ud->walkpath.path_pos>=ud->walkpath.path_len)
		i = -1;
	else if(ud->walkpath.path[ud->walkpath.path_pos]&1)
		i = iStatus->get_speed(bl)*14/10;
	else
		i = iStatus->get_speed(bl);

	if(i > 0) {
		ud->walktimer = iTimer->add_timer(tick+i,unit_walktoxy_timer,id,i);
		if( md && DIFF_TICK(tick,md->dmgtick) < 3000 )//not required not damaged recently
			clif->move(ud);
	} else if(ud->state.running) {
		//Keep trying to run.
		if ( !(unit_run(bl) || unit_wugdash(bl,sd)) )
			ud->state.running = 0;
	}
	else if (ud->target_to) {
		//Update target trajectory.
		struct block_list *tbl = iMap->id2bl(ud->target_to);
		if (!tbl || !iStatus->check_visibility(bl, tbl)) {	//Cancel chase.
			ud->to_x = bl->x;
			ud->to_y = bl->y;
			if (tbl && bl->type == BL_MOB && mob_warpchase((TBL_MOB*)bl, tbl) )
				return 0;
			ud->target_to = 0;
			return 0;
		}
		if (tbl->m == bl->m && check_distance_bl(bl, tbl, ud->chaserange))
		{	//Reached destination.
			if (ud->state.attack_continue)
			{	//Aegis uses one before every attack, we should
				//only need this one for syncing purposes. [Skotlex]
				ud->target_to = 0;
				clif->fixpos(bl);
				unit_attack(bl, tbl->id, ud->state.attack_continue);
			}
		} else { //Update chase-path
			unit_walktobl(bl, tbl, ud->chaserange, ud->state.walk_easy|(ud->state.attack_continue?2:0));
			return 0;
		}
	}
	else {	//Stopped walking. Update to_x and to_y to current location [Skotlex]
		ud->to_x = bl->x;
		ud->to_y = bl->y;
	}
	return 0;
}

static int unit_delay_walktoxy_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl = iMap->id2bl(id);

	if (!bl || bl->prev == NULL)
		return 0;
	unit_walktoxy(bl, (short)((data>>16)&0xffff), (short)(data&0xffff), 0);
	return 1;
}

//flag parameter:
//&1 -> 1/0 = easy/hard
//&2 -> force walking
//&4 -> Delay walking if the reason you can't walk is the canwalk delay
int unit_walktoxy( struct block_list *bl, short x, short y, int flag)
{
	struct unit_data* ud = NULL;
	struct status_change* sc = NULL;
	struct walkpath_data wpd;

	nullpo_ret(bl);

	ud = unit_bl2ud(bl);

	if( ud == NULL) return 0;

	if (!path_search(&wpd, bl->m, bl->x, bl->y, x, y, flag&1, CELL_CHKNOPASS)) // Count walk path cells
		return 0;

#ifdef OFFICIAL_WALKPATH
	if( !path_search_long(NULL, bl->m, bl->x, bl->y, x, y, CELL_CHKNOPASS) // Check if there is an obstacle between
		&& (wpd.path_len > (battle_config.max_walk_path/17)*14) // Official number of walkable cells is 14 if and only if there is an obstacle between. [malufett]
		&& (bl->type != BL_NPC) ) // If type is a NPC, please disregard.
		return 0;
#endif
	if ((wpd.path_len > battle_config.max_walk_path) && (bl->type != BL_NPC))
		return 0;

	if (flag&4 && DIFF_TICK(ud->canmove_tick, iTimer->gettick()) > 0 &&
		DIFF_TICK(ud->canmove_tick, iTimer->gettick()) < 2000)
  	{	// Delay walking command. [Skotlex]
		iTimer->add_timer(ud->canmove_tick+1, unit_delay_walktoxy_timer, bl->id, (x<<16)|(y&0xFFFF));
		return 1;
	}

	if(!(flag&2) && (!(status_get_mode(bl)&MD_CANMOVE) || !unit_can_move(bl)))
		return 0;

	ud->state.walk_easy = flag&1;
	ud->to_x = x;
	ud->to_y = y;
	unit_set_target(ud, 0);

	sc = iStatus->get_sc(bl);
	if (sc && sc->data[SC_CONFUSION]) //Randomize the target position
		iMap->random_dir(bl, &ud->to_x, &ud->to_y);

	if(ud->walktimer != INVALID_TIMER) {
		// When you come to the center of the grid because the change of destination while you're walking right now
		// Call a function from a timer unit_walktoxy_sub
		ud->state.change_walk_target = 1;
		return 1;
	}

	if(ud->attacktimer != INVALID_TIMER) {
		iTimer->delete_timer( ud->attacktimer, unit_attack_timer );
		ud->attacktimer = INVALID_TIMER;
	}

	return unit_walktoxy_sub(bl);
}

//To set Mob's CHASE/FOLLOW states (shouldn't be done if there's no path to reach)
static inline void set_mobstate(struct block_list* bl, int flag)
{
	struct mob_data* md = BL_CAST(BL_MOB,bl);

	if( md && flag )
		md->state.skillstate = md->state.aggressive ? MSS_FOLLOW : MSS_RUSH;
}

static int unit_walktobl_sub(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl = iMap->id2bl(id);
	struct unit_data *ud = bl?unit_bl2ud(bl):NULL;

	if (ud && ud->walktimer == INVALID_TIMER && ud->target == data)
	{
		if (DIFF_TICK(ud->canmove_tick, tick) > 0) //Keep waiting?
			iTimer->add_timer(ud->canmove_tick+1, unit_walktobl_sub, id, data);
		else if (unit_can_move(bl))
		{
			if (unit_walktoxy_sub(bl))
				set_mobstate(bl, ud->state.attack_continue);
		}
	}
	return 0;
}

// Chases a tbl. If the flag&1, use hard-path seek,
// if flag&2, start attacking upon arrival within range, otherwise just walk to that character.
int unit_walktobl(struct block_list *bl, struct block_list *tbl, int range, int flag)
{
	struct unit_data        *ud = NULL;
	struct status_change		*sc = NULL;

	nullpo_ret(bl);
	nullpo_ret(tbl);

	ud = unit_bl2ud(bl);
	if( ud == NULL) return 0;

	if (!(status_get_mode(bl)&MD_CANMOVE))
		return 0;

	if (!unit_can_reach_bl(bl, tbl, distance_bl(bl, tbl)+1, flag&1, &ud->to_x, &ud->to_y)) {
		ud->to_x = bl->x;
		ud->to_y = bl->y;
		ud->target_to = 0;
		return 0;
	}

	ud->state.walk_easy = flag&1;
	ud->target_to = tbl->id;
	ud->chaserange = range; //Note that if flag&2, this SHOULD be attack-range
	ud->state.attack_continue = flag&2?1:0; //Chase to attack.
	unit_set_target(ud, 0);

	sc = iStatus->get_sc(bl);
	if (sc && sc->data[SC_CONFUSION]) //Randomize the target position
		iMap->random_dir(bl, &ud->to_x, &ud->to_y);

	if(ud->walktimer != INVALID_TIMER) {
		ud->state.change_walk_target = 1;
		set_mobstate(bl, flag&2);
		return 1;
	}

	if(DIFF_TICK(ud->canmove_tick, iTimer->gettick()) > 0)
	{	//Can't move, wait a bit before invoking the movement.
		iTimer->add_timer(ud->canmove_tick+1, unit_walktobl_sub, bl->id, ud->target);
		return 1;
	}

	if(!unit_can_move(bl))
		return 0;

	if(ud->attacktimer != INVALID_TIMER) {
		iTimer->delete_timer( ud->attacktimer, unit_attack_timer );
		ud->attacktimer = INVALID_TIMER;
	}

	if (unit_walktoxy_sub(bl)) {
		set_mobstate(bl, flag&2);
		return 1;
	}
	return 0;
}

int unit_run(struct block_list *bl)
{
	struct status_change *sc = iStatus->get_sc(bl);
	short to_x,to_y,dir_x,dir_y;
	int lv;
	int i;

	if (!(sc && sc->data[SC_RUN]))
		return 0;

	if (!unit_can_move(bl)) {
		status_change_end(bl, SC_RUN, INVALID_TIMER);
		return 0;
	}

	lv = sc->data[SC_RUN]->val1;
	dir_x = dirx[sc->data[SC_RUN]->val2];
	dir_y = diry[sc->data[SC_RUN]->val2];

	// determine destination cell
	to_x = bl->x;
	to_y = bl->y;
	for(i=0;i<AREA_SIZE;i++)
	{
		if(!iMap->getcell(bl->m,to_x+dir_x,to_y+dir_y,CELL_CHKPASS))
			break;

		//if sprinting and there's a PC/Mob/NPC, block the path [Kevin]
		if(sc->data[SC_RUN] && iMap->count_oncell(bl->m, to_x+dir_x, to_y+dir_y, BL_PC|BL_MOB|BL_NPC))
			break;

		to_x += dir_x;
		to_y += dir_y;
	}

	if( (to_x == bl->x && to_y == bl->y ) || (to_x == (bl->x+1) || to_y == (bl->y+1)) || (to_x == (bl->x-1) || to_y == (bl->y-1))) {
		//If you can't run forward, you must be next to a wall, so bounce back. [Skotlex]
		clif->sc_load(bl,bl->id,AREA,SI_TING,0,0,0);

		//Set running to 0 beforehand so status_change_end knows not to enable spurt [Kevin]
		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl, SC_RUN, INVALID_TIMER);

		skill->blown(bl,bl,skill->get_blewcount(TK_RUN,lv),unit_getdir(bl),0);
		clif->fixpos(bl); //Why is a clif->slide (skill->blown) AND a fixpos needed? Ask Aegis.
		clif->sc_end(bl,bl->id,AREA,SI_TING);
		return 0;
	}
	if (unit_walktoxy(bl, to_x, to_y, 1))
		return 1;
	//There must be an obstacle nearby. Attempt walking one cell at a time.
	do {
		to_x -= dir_x;
		to_y -= dir_y;
	} while (--i > 0 && !unit_walktoxy(bl, to_x, to_y, 1));
	if ( i == 0 ) {
		// copy-paste from above
		clif->sc_load(bl,bl->id,AREA,SI_TING,0,0,0);

		//Set running to 0 beforehand so status_change_end knows not to enable spurt [Kevin]
		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl, SC_RUN, INVALID_TIMER);

		skill->blown(bl,bl,skill->get_blewcount(TK_RUN,lv),unit_getdir(bl),0);
		clif->fixpos(bl);
		clif->sc_end(bl,bl->id,AREA,SI_TING);
		return 0;
	}
	return 1;
}

//Exclusive function to Wug Dash state. [Jobbie/3CeAM]
int unit_wugdash(struct block_list *bl, struct map_session_data *sd) {
	struct status_change *sc = iStatus->get_sc(bl);
	short to_x,to_y,dir_x,dir_y;
	int lv;
	int i;
	if (!(sc && sc->data[SC_WUGDASH]))
		return 0;

	nullpo_ret(sd);
	nullpo_ret(bl);

	if (!unit_can_move(bl)) {
		status_change_end(bl,SC_WUGDASH,INVALID_TIMER);
		return 0;
	}

	lv = sc->data[SC_WUGDASH]->val1;
	dir_x = dirx[sc->data[SC_WUGDASH]->val2];
	dir_y = diry[sc->data[SC_WUGDASH]->val2];

	to_x = bl->x;
	to_y = bl->y;
	for(i=0;i<AREA_SIZE;i++)
	{
		if(!iMap->getcell(bl->m,to_x+dir_x,to_y+dir_y,CELL_CHKPASS))
			break;

		if(sc->data[SC_WUGDASH] && iMap->count_oncell(bl->m, to_x+dir_x, to_y+dir_y, BL_PC|BL_MOB|BL_NPC))
			break;

		to_x += dir_x;
		to_y += dir_y;
	}

	if(to_x == bl->x && to_y == bl->y) {

		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl,SC_WUGDASH,INVALID_TIMER);

		if( sd ){
			clif->fixpos(bl);
			skill->castend_damage_id(bl, &sd->bl, RA_WUGDASH, lv, iTimer->gettick(), SD_LEVEL);
		}
		return 0;
	}
	if (unit_walktoxy(bl, to_x, to_y, 1))
		return 1;
	do {
		to_x -= dir_x;
		to_y -= dir_y;
	} while (--i > 0 && !unit_walktoxy(bl, to_x, to_y, 1));
	if (i==0) {

		unit_bl2ud(bl)->state.running = 0;
		status_change_end(bl,SC_WUGDASH,INVALID_TIMER);

		if( sd ){
			clif->fixpos(bl);
			skill->castend_damage_id(bl, &sd->bl, RA_WUGDASH, lv, iTimer->gettick(), SD_LEVEL);
		}
		return 0;
	}
	return 1;
}

//Makes bl attempt to run dist cells away from target. Uses hard-paths.
int unit_escape(struct block_list *bl, struct block_list *target, short dist)
{
	uint8 dir = iMap->calc_dir(target, bl->x, bl->y);
	while( dist > 0 && iMap->getcell(bl->m, bl->x + dist*dirx[dir], bl->y + dist*diry[dir], CELL_CHKNOREACH) )
		dist--;
	return ( dist > 0 && unit_walktoxy(bl, bl->x + dist*dirx[dir], bl->y + dist*diry[dir], 0) );
}

//Instant warp function.
int unit_movepos(struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath)
{
	short dx,dy;
	uint8 dir;
	struct unit_data        *ud = NULL;
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	sd = BL_CAST(BL_PC, bl);
	ud = unit_bl2ud(bl);

	if( ud == NULL) return 0;

	unit_stop_walking(bl,1);
	unit_stop_attack(bl);

	if( checkpath && (iMap->getcell(bl->m,dst_x,dst_y,CELL_CHKNOPASS) || !path_search(NULL,bl->m,bl->x,bl->y,dst_x,dst_y,easy,CELL_CHKNOREACH)) )
		return 0; // unreachable

	ud->to_x = dst_x;
	ud->to_y = dst_y;

	dir = iMap->calc_dir(bl, dst_x, dst_y);
	ud->dir = dir;

	dx = dst_x - bl->x;
	dy = dst_y - bl->y;

	iMap->foreachinmovearea(clif->outsight, bl, AREA_SIZE, dx, dy, sd?BL_ALL:BL_PC, bl);

	iMap->moveblock(bl, dst_x, dst_y, iTimer->gettick());

	ud->walktimer = -2; // arbitrary non-INVALID_TIMER value to make the clif code send walking packets
	iMap->foreachinmovearea(clif->insight, bl, AREA_SIZE, -dx, -dy, sd?BL_ALL:BL_PC, bl);
	ud->walktimer = INVALID_TIMER;

	if(sd) {
		if( sd->touching_id )
			npc_touchnext_areanpc(sd,false);
		if(iMap->getcell(bl->m,bl->x,bl->y,CELL_CHKNPC)) {
			npc_touch_areanpc(sd,bl->m,bl->x,bl->y);
			if (bl->prev == NULL) //Script could have warped char, abort remaining of the function.
				return 0;
		} else
			sd->areanpc_id=0;

		if( sd->status.pet_id > 0 && sd->pd && sd->pd->pet.intimate > 0 )
		{ // Check if pet needs to be teleported. [Skotlex]
			int flag = 0;
			struct block_list* bl = &sd->pd->bl;
			if( !checkpath && !path_search(NULL,bl->m,bl->x,bl->y,dst_x,dst_y,0,CELL_CHKNOPASS) )
				flag = 1;
			else if (!check_distance_bl(&sd->bl, bl, AREA_SIZE)) //Too far, teleport.
				flag = 2;
			if( flag )
			{
				unit_movepos(bl,sd->bl.x,sd->bl.y, 0, 0);
				clif->slide(bl,bl->x,bl->y);
			}
		}
	}
	return 1;
}

int unit_setdir(struct block_list *bl,unsigned char dir)
{
	struct unit_data *ud;
	nullpo_ret(bl );
	ud = unit_bl2ud(bl);
	if (!ud) return 0;
	ud->dir = dir;
	if (bl->type == BL_PC)
		((TBL_PC *)bl)->head_dir = 0;
	clif->changed_dir(bl, AREA);
	return 0;
}

uint8 unit_getdir(struct block_list *bl) {
	struct unit_data *ud;
	nullpo_ret(bl);
	
	if( bl->type == BL_NPC )
		return ((TBL_NPC*)bl)->dir;
	ud = unit_bl2ud(bl);
	if (!ud) return 0;
	return ud->dir;
}

// Pushes a unit by given amount of cells into given direction. Only
// map cell restrictions are respected.
// flag:
//  &1  Do not send position update packets.
int unit_blown(struct block_list* bl, int dx, int dy, int count, int flag)
{
	if(count) {
		struct map_session_data* sd;
		struct skill_unit* su = NULL;
		int nx, ny, result;

		sd = BL_CAST(BL_PC, bl);
		su = BL_CAST(BL_SKILL, bl);

		result = path_blownpos(bl->m, bl->x, bl->y, dx, dy, count);

		nx = result>>16;
		ny = result&0xffff;

		if(!su) {
			unit_stop_walking(bl, 0);
		}

		if( sd ) {
			sd->ud.to_x = nx;
			sd->ud.to_y = ny;
		}

		dx = nx-bl->x;
		dy = ny-bl->y;

		if(dx || dy) {
			iMap->foreachinmovearea(clif->outsight, bl, AREA_SIZE, dx, dy, bl->type == BL_PC ? BL_ALL : BL_PC, bl);

			if(su) {
				skill->unit_move_unit_group(su->group, bl->m, dx, dy);
			} else {
				iMap->moveblock(bl, nx, ny, iTimer->gettick());
			}

			iMap->foreachinmovearea(clif->insight, bl, AREA_SIZE, -dx, -dy, bl->type == BL_PC ? BL_ALL : BL_PC, bl);

			if(!(flag&1)) {
				clif->blown(bl);
			}

			if(sd) {
				if(sd->touching_id) {
					npc_touchnext_areanpc(sd, false);
				}
				if(iMap->getcell(bl->m, bl->x, bl->y, CELL_CHKNPC)) {
					npc_touch_areanpc(sd, bl->m, bl->x, bl->y);
				} else {
					sd->areanpc_id = 0;
				}
			}
		}

		count = distance(dx, dy);
	}

	return count;  // return amount of knocked back cells
}

//Warps a unit/ud to a given map/position.
//In the case of players, pc->setpos is used.
//it respects the no warp flags, so it is safe to call this without doing nowarpto/nowarp checks.
int unit_warp(struct block_list *bl,short m,short x,short y,clr_type type)
{
	struct unit_data *ud;
	nullpo_ret(bl);
	ud = unit_bl2ud(bl);

	if(bl->prev==NULL || !ud)
		return 1;

	if (type == CLR_DEAD)
		//Type 1 is invalid, since you shouldn't warp a bl with the "death"
		//animation, it messes up with unit_remove_map! [Skotlex]
		return 1;

	if( m<0 ) m=bl->m;

	switch (bl->type) {
		case BL_MOB:
			if (map[bl->m].flag.monster_noteleport && ((TBL_MOB*)bl)->master_id == 0)
				return 1;
			if (m != bl->m && map[m].flag.nobranch && battle_config.mob_warp&4 && !(((TBL_MOB *)bl)->master_id))
				return 1;
			break;
		case BL_PC:
			if (map[bl->m].flag.noteleport)
				return 1;
			break;
	}

	if (x<0 || y<0)
  	{	//Random map position.
		if (!iMap->search_freecell(NULL, m, &x, &y, -1, -1, 1)) {
			ShowWarning("unit_warp failed. Unit Id:%d/Type:%d, target position map %d (%s) at [%d,%d]\n", bl->id, bl->type, m, map[m].name, x, y);
			return 2;

		}
	} else if (iMap->getcell(m,x,y,CELL_CHKNOREACH))
	{	//Invalid target cell
		ShowWarning("unit_warp: Specified non-walkable target cell: %d (%s) at [%d,%d]\n", m, map[m].name, x,y);

		if (!iMap->search_freecell(NULL, m, &x, &y, 4, 4, 1))
	 	{	//Can't find a nearby cell
			ShowWarning("unit_warp failed. Unit Id:%d/Type:%d, target position map %d (%s) at [%d,%d]\n", bl->id, bl->type, m, map[m].name, x, y);
			return 2;
		}
	}

	if (bl->type == BL_PC) //Use pc_setpos
		return pc->setpos((TBL_PC*)bl, map_id2index(m), x, y, type);

	if (!unit_remove_map(bl, type))
		return 3;

	if (bl->m != m && battle_config.clear_unit_onwarp &&
		battle_config.clear_unit_onwarp&bl->type)
		skill->clear_unitgroup(bl);

	bl->x=ud->to_x=x;
	bl->y=ud->to_y=y;
	bl->m=m;

	iMap->addblock(bl);
	clif->spawn(bl);
	skill->unit_move(bl,iTimer->gettick(),1);

	return 0;
}

/*==========================================
 * Caused the target object to stop moving.
 * Flag values:
 * &0x1: Issue a fixpos packet afterwards
 * &0x2: Force the unit to move one cell if it hasn't yet
 * &0x4: Enable moving to the next cell when unit was already half-way there
 *       (may cause on-touch/place side-effects, such as a scripted map change)
 *------------------------------------------*/
int unit_stop_walking(struct block_list *bl,int type)
{
	struct unit_data *ud;
	const struct TimerData* td;
	unsigned int tick;
	nullpo_ret(bl);

	ud = unit_bl2ud(bl);
	if(!ud || ud->walktimer == INVALID_TIMER)
		return 0;
	//NOTE: We are using timer data after deleting it because we know the
	//iTimer->delete_timer function does not messes with it. If the function's
	//behaviour changes in the future, this code could break!
	td = iTimer->get_timer(ud->walktimer);
	iTimer->delete_timer(ud->walktimer, unit_walktoxy_timer);
	ud->walktimer = INVALID_TIMER;
	ud->state.change_walk_target = 0;
	tick = iTimer->gettick();
	if( (type&0x02 && !ud->walkpath.path_pos) //Force moving at least one cell.
	||  (type&0x04 && td && DIFF_TICK(td->tick, tick) <= td->data/2) //Enough time has passed to cover half-cell
	) {
		ud->walkpath.path_len = ud->walkpath.path_pos+1;
		unit_walktoxy_timer(INVALID_TIMER, tick, bl->id, ud->walkpath.path_pos);
	}

	if(type&0x01)
		clif->fixpos(bl);

	ud->walkpath.path_len = 0;
	ud->walkpath.path_pos = 0;
	ud->to_x = bl->x;
	ud->to_y = bl->y;
	if(bl->type == BL_PET && type&~0xff)
		ud->canmove_tick = iTimer->gettick() + (type>>8);

	//Readded, the check in unit_set_walkdelay means dmg during running won't fall through to this place in code [Kevin]
	if (ud->state.running) {
		status_change_end(bl, SC_RUN, INVALID_TIMER);
		status_change_end(bl, SC_WUGDASH, INVALID_TIMER);
	}
	return 1;
}

int unit_skilluse_id(struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv)
{
	return unit_skilluse_id2(
		src, target_id, skill_id, skill_lv,
		skill->cast_fix(src, skill_id, skill_lv),
		skill->get_castcancel(skill_id)
	);
}

int unit_is_walking(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_ret(bl);
	if(!ud) return 0;
	return (ud->walktimer != INVALID_TIMER);
}

/*==========================================
 * Determines if the bl can move based on status changes. [Skotlex]
 *------------------------------------------*/
int unit_can_move(struct block_list *bl) {
	struct map_session_data *sd;
	struct unit_data *ud;
	struct status_change *sc;

	nullpo_ret(bl);
	ud = unit_bl2ud(bl);
	sc = iStatus->get_sc(bl);
	sd = BL_CAST(BL_PC, bl);

	if (!ud)
		return 0;

	if (ud->skilltimer != INVALID_TIMER && ud->skill_id != LG_EXEEDBREAK && (!sd || !pc->checkskill(sd, SA_FREECAST) || skill->get_inf2(ud->skill_id)&INF2_GUILD_SKILL))
		return 0; // prevent moving while casting

	if (DIFF_TICK(ud->canmove_tick, iTimer->gettick()) > 0)
		return 0;

	if (sd && (
		pc_issit(sd) ||
		sd->state.vending ||
		sd->state.buyingstore ||
		sd->state.blockedmove
	))
		return 0; //Can't move

	if (sc) {
		if( sc->count && (
						    sc->data[SC_ANKLESNARE]
						||  sc->data[SC_AUTOCOUNTER]
						||  sc->data[SC_TRICKDEAD]
						||  sc->data[SC_BLADESTOP]
						||  sc->data[SC_BLADESTOP_WAIT]
						|| (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF) // cannot move while gospel is in effect
						|| (sc->data[SC_BASILICA] && sc->data[SC_BASILICA]->val4 == bl->id) // Basilica caster cannot move
						||  sc->data[SC_STOP]
						||  sc->data[SC_RG_CCONFINE_M]
						||  sc->data[SC_RG_CCONFINE_S]
						||  sc->data[SC_GS_MADNESSCANCEL]
						|| (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF)
						||  sc->data[SC_WHITEIMPRISON]
						||  sc->data[SC_ELECTRICSHOCKER]
						||  sc->data[SC_WUGBITE]
						||  sc->data[SC_THORNS_TRAP]
						||  sc->data[SC_MAGNETICFIELD]
						||  sc->data[SC__MANHOLE]
						||  sc->data[SC_CURSEDCIRCLE_ATKER]
						||  sc->data[SC_CURSEDCIRCLE_TARGET]
						|| (sc->data[SC_COLD] && bl->type != BL_MOB)
						||  sc->data[SC_NETHERWORLD]
						|| (sc->data[SC_CAMOUFLAGE] && sc->data[SC_CAMOUFLAGE]->val1 < 3 && !(sc->data[SC_CAMOUFLAGE]->val3&1))
						||  sc->data[SC_MEIKYOUSISUI]
						||  sc->data[SC_KG_KAGEHUMI]
						||  sc->data[SC_KYOUGAKU]
						||  sc->data[SC_NEEDLE_OF_PARALYZE]
						||  sc->data[SC_VACUUM_EXTREME]
						|| (sc->data[SC_FEAR] && sc->data[SC_FEAR]->val2 > 0)
						|| (sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
						|| (sc->data[SC_DANCING] && sc->data[SC_DANCING]->val4 && (
																				!sc->data[SC_LONGING] ||
																				(sc->data[SC_DANCING]->val1&0xFFFF) == CG_MOONLIT ||
																				(sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE
																				) )
						|| (sc->data[SC_CLOAKING] && //Need wall at level 1-2
							sc->data[SC_CLOAKING]->val1 < 3 && !(sc->data[SC_CLOAKING]->val4&1))
			) )
			return 0;
		

		if (sc->opt1 > 0 && sc->opt1 != OPT1_STONEWAIT && sc->opt1 != OPT1_BURNING && !(sc->opt1 == OPT1_CRYSTALIZE && bl->type == BL_MOB))
			return 0;

		if ((sc->option & OPTION_HIDE) && (!sd || pc->checkskill(sd, RG_TUNNELDRIVE) <= 0))
			return 0;

	}
	return 1;
}

/*==========================================
 * Resume running after a walk delay
 *------------------------------------------*/

int unit_resume_running(int tid, unsigned int tick, int id, intptr_t data)
{

	struct unit_data *ud = (struct unit_data *)data;
	TBL_PC * sd = iMap->id2sd(id);

	if(sd && pc_isridingwug(sd))
		clif->skill_nodamage(ud->bl,ud->bl,RA_WUGDASH,ud->skill_lv,
			sc_start4(ud->bl,iStatus->skill2sc(RA_WUGDASH),100,ud->skill_lv,unit_getdir(ud->bl),0,0,1));
	else
		clif->skill_nodamage(ud->bl,ud->bl,TK_RUN,ud->skill_lv,
			sc_start4(ud->bl,iStatus->skill2sc(TK_RUN),100,ud->skill_lv,unit_getdir(ud->bl),0,0,0));

	if (sd) clif->walkok(sd);

	return 0;

}


/*==========================================
 * Applies walk delay to character, considering that
 * if type is 0, this is a damage induced delay: if previous delay is active, do not change it.
 * if type is 1, this is a skill induced delay: walk-delay may only be increased, not decreased.
 *------------------------------------------*/
int unit_set_walkdelay(struct block_list *bl, unsigned int tick, int delay, int type)
{
	struct unit_data *ud = unit_bl2ud(bl);
	if (delay <= 0 || !ud) return 0;

	/**
	 * MvP mobs have no walk delay
	 **/
	if( bl->type == BL_MOB && (((TBL_MOB*)bl)->status.mode&MD_BOSS) )
		return 0;

	if (type) {
		if (DIFF_TICK(ud->canmove_tick, tick+delay) > 0)
			return 0;
	} else {
		//Don't set walk delays when already trapped.
		if (!unit_can_move(bl))
			return 0;
	}
	ud->canmove_tick = tick + delay;
	if (ud->walktimer != INVALID_TIMER)
	{	//Stop walking, if chasing, readjust timers.
		if (delay == 1)
		{	//Minimal delay (walk-delay) disabled. Just stop walking.
			unit_stop_walking(bl,4);
		} else {
			//Resume running after can move again [Kevin]
			if(ud->state.running)
			{
				iTimer->add_timer(ud->canmove_tick, unit_resume_running, bl->id, (intptr_t)ud);
			}
			else
			{
				unit_stop_walking(bl,2|4);
				if(ud->target)
					iTimer->add_timer(ud->canmove_tick+1, unit_walktobl_sub, bl->id, ud->target);
			}
		}
	}
	return 1;
}

int unit_skilluse_id2(struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel)
{
	struct unit_data *ud;
	struct status_data *tstatus;
	struct status_change *sc;
	struct map_session_data *sd = NULL;
	struct block_list * target = NULL;
	unsigned int tick = iTimer->gettick();
	int temp = 0, range;

	nullpo_ret(src);
	if(iStatus->isdead(src))
		return 0; //Do not continue source is dead

	sd = BL_CAST(BL_PC, src);
	ud = unit_bl2ud(src);

	if(ud == NULL) return 0;
	sc = iStatus->get_sc(src);
	if (sc && !sc->count)
		sc = NULL; //Unneeded

	//temp: used to signal combo-skills right now.
	if (sc && sc->data[SC_COMBOATTACK] && (sc->data[SC_COMBOATTACK]->val1 == skill_id ||
		(sd?skill->check_condition_castbegin(sd,skill_id,skill_lv):0) )) {
		if (sc->data[SC_COMBOATTACK]->val2)
			target_id = sc->data[SC_COMBOATTACK]->val2;
		else
			target_id = ud->target;

		if( skill->get_inf(skill_id)&INF_SELF_SKILL && skill->get_nk(skill_id)&NK_NO_DAMAGE )// exploit fix
			target_id = src->id;
		temp = 1;
	} else if ( target_id == src->id &&
		skill->get_inf(skill_id)&INF_SELF_SKILL &&
		skill->get_inf2(skill_id)&INF2_NO_TARGET_SELF )
	{
		target_id = ud->target; //Auto-select target. [Skotlex]
		temp = 1;
	}

	if (sd) {
		//Target_id checking.
		if(skill->not_ok(skill_id, sd)) // [MouseJstr]
			return 0;

		switch(skill_id) {	//Check for skills that auto-select target
			case MO_CHAINCOMBO:
				if (sc && sc->data[SC_BLADESTOP]){
					if ((target=iMap->id2bl(sc->data[SC_BLADESTOP]->val4)) == NULL)
						return 0;
				}
				break;
			case WE_MALE:
			case WE_FEMALE:
				if (!sd->status.partner_id)
					return 0;
				target = (struct block_list*)iMap->charid2sd(sd->status.partner_id);
				if (!target) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
				break;
		}
		if (target)
			target_id = target->id;
	}

	if (src->type==BL_HOM)
		switch(skill_id) { //Homun-auto-target skills.
			case HLIF_HEAL:
			case HLIF_AVOID:
			case HAMI_DEFENCE:
			case HAMI_CASTLE:
				target = battle->get_master(src);
				if (!target) return 0;
				target_id = target->id;
	}

	if( !target ) // choose default target
		target = iMap->id2bl(target_id);

	if( !target || src->m != target->m || !src->prev || !target->prev )
		return 0;

	if( battle_config.ksprotection && sd && mob_ksprotected(src, target) )
		return 0;

	//Normally not needed because clif.c checks for it, but the at/char/script commands don't! [Skotlex]
	if(ud->skilltimer != INVALID_TIMER && skill_id != SA_CASTCANCEL && skill_id != SO_SPELLFIST)
		return 0;

	if(skill->get_inf2(skill_id)&INF2_NO_TARGET_SELF && src->id == target_id)
		return 0;

	if(!iStatus->check_skilluse(src, target, skill_id, 0))
		return 0;

	tstatus = iStatus->get_status_data(target);
	// Record the status of the previous skill)
	if(sd) {

		if( (skill->get_inf2(skill_id)&INF2_ENSEMBLE_SKILL) && skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 0) < 1 ) {
			clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0; 
        }
		
		switch(skill_id){
			case SA_CASTCANCEL:
				if(ud->skill_id != skill_id){
					sd->skill_id_old = ud->skill_id;
					sd->skill_lv_old = ud->skill_lv;
				}
				break;
			case BD_ENCORE:
				//Prevent using the dance skill if you no longer have the skill in your tree.
				if(!sd->skill_id_dance || pc->checkskill(sd,sd->skill_id_dance)<=0){
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
				sd->skill_id_old = skill_id;
				break;
			case WL_WHITEIMPRISON:
				if( battle->check_target(src,target,BCT_SELF|BCT_ENEMY) < 0 ) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_TOTARGET,0);
					return 0;
				}
				break;
			case MG_FIREBOLT:
			case MG_LIGHTNINGBOLT:
			case MG_COLDBOLT:
				sd->skill_id_old = skill_id;
				sd->skill_lv_old = skill_lv;
				break;
		}
		/* temporarily disabled, awaiting for kenpachi to detail this so we can make it work properly */
#if 0
		if ( sd->skillitem != skill_id && !skill->check_condition_castbegin(sd, skill_id, skill_lv) )
#else
		if ( !skill->check_condition_castbegin(sd, skill_id, skill_lv) )
#endif
			return 0;
	}

	if( src->type == BL_MOB )
		switch( skill_id ) {
			case NPC_SUMMONSLAVE:
			case NPC_SUMMONMONSTER:
			case AL_TELEPORT:
				if( ((TBL_MOB*)src)->master_id && ((TBL_MOB*)src)->special_state.ai )
					return 0;
		}

	if (src->type == BL_NPC) // NPC-objects can override cast distance
		range = AREA_SIZE; // Maximum visible distance before NPC goes out of sight
	else
		range = skill->get_range2(src, skill_id, skill_lv); // Skill cast distance from database

	//Check range when not using skill on yourself or is a combo-skill during attack
	//(these are supposed to always have the same range as your attack)
	if( src->id != target_id && (!temp || ud->attacktimer == INVALID_TIMER) ) {
		if( skill->get_state(ud->skill_id) == ST_MOVE_ENABLE ) {
			if( !unit_can_reach_bl(src, target, range + 1, 1, NULL, NULL) )
				return 0; // Walk-path check failed.
		} else if( src->type == BL_MER && skill_id == MA_REMOVETRAP ) {
			if( !battle->check_range(battle->get_master(src), target, range + 1) )
				return 0; // Aegis calc remove trap based on Master position, ignoring mercenary O.O
		} else if( !battle->check_range(src, target, range + (skill_id == RG_CLOSECONFINE?0:2)) ) {
			return 0; // Arrow-path check failed.
		}
	}

	if (!temp) //Stop attack on non-combo skills [Skotlex]
		unit_stop_attack(src);
	else if(ud->attacktimer != INVALID_TIMER) //Elsewise, delay current attack sequence
		ud->attackabletime = tick + status_get_adelay(src);

	ud->state.skillcastcancel = castcancel;

	//temp: Used to signal force cast now.
	temp = 0;

	switch(skill_id){
	case ALL_RESURRECTION:
		if(battle->check_undead(tstatus->race,tstatus->def_ele)) {
			temp = 1;
		} else if (!iStatus->isdead(target))
			return 0; //Can't cast on non-dead characters.
	break;
	case MO_FINGEROFFENSIVE:
		if(sd)
			casttime += casttime * min(skill_lv, sd->spiritball);
	break;
	case MO_EXTREMITYFIST:
		if (sc && sc->data[SC_COMBOATTACK] &&
		   (sc->data[SC_COMBOATTACK]->val1 == MO_COMBOFINISH ||
			sc->data[SC_COMBOATTACK]->val1 == CH_TIGERFIST ||
			sc->data[SC_COMBOATTACK]->val1 == CH_CHAINCRUSH))
			casttime = -1;
		temp = 1;
	break;
	case SR_GATEOFHELL:
	case SR_TIGERCANNON:
		if (sc && sc->data[SC_COMBOATTACK] &&
		   sc->data[SC_COMBOATTACK]->val1 == SR_FALLENEMPIRE)
			casttime = -1;
		temp = 1;
	break;
	case SA_SPELLBREAKER:
		temp = 1;
	break;
	case ST_CHASEWALK:
		if (sc && sc->data[SC_CHASEWALK])
			casttime = -1;
	break;
	case TK_RUN:
		if (sc && sc->data[SC_RUN])
			casttime = -1;
	break;
	case HP_BASILICA:
		if( sc && sc->data[SC_BASILICA] )
			casttime = -1; // No Casting time on basilica cancel
	break;
	case KN_CHARGEATK:
		{
		unsigned int k = (distance_bl(src,target)-1)/3; //+100% every 3 cells of distance
		if( k > 2 ) k = 2; // ...but hard-limited to 300%.
		casttime += casttime * k;
		}
	break;
	case GD_EMERGENCYCALL: //Emergency Call double cast when the user has learned Leap [Daegaladh]
		if( sd && pc->checkskill(sd,TK_HIGHJUMP) )
			casttime *= 2;
		break;
	case RA_WUGDASH:
		if (sc && sc->data[SC_WUGDASH])
			casttime = -1;
        break;
	case EL_WIND_SLASH:
	case EL_HURRICANE:
	case EL_TYPOON_MIS:
	case EL_STONE_HAMMER:
	case EL_ROCK_CRUSHER:
	case EL_STONE_RAIN:
	case EL_ICE_NEEDLE:
	case EL_WATER_SCREW:
	case EL_TIDAL_WEAPON:
		if( src->type == BL_ELEM ){
			sd = BL_CAST(BL_PC, battle->get_master(src));
			if( sd && sd->skill_id_old == SO_EL_ACTION ){
				casttime = -1;
				sd->skill_id_old = 0;
			}
		}
		break;
	}

	// moved here to prevent Suffragium from ending if skill fails
#ifndef RENEWAL_CAST
	if (!(skill->get_castnodex(skill_id, skill_lv)&2))
		casttime = skill->cast_fix_sc(src, casttime);
#else
	casttime = skill->vf_cast_fix(src, casttime, skill_id, skill_lv);
#endif

	if (src->type == BL_NPC) { // NPC-objects do not have cast time
		casttime = 0;
	}

	if(!ud->state.running) //need TK_RUN or WUGDASH handler to be done before that, see bugreport:6026
		unit_stop_walking(src,1);// eventhough this is not how official works but this will do the trick. bugreport:6829
	
	// in official this is triggered even if no cast time.
	clif->skillcasting(src, src->id, target_id, 0,0, skill_id, skill->get_ele(skill_id, skill_lv), casttime);
	if( casttime > 0 || temp )
	{		
		if (sd && target->type == BL_MOB)
		{
			TBL_MOB *md = (TBL_MOB*)target;
			mobskill_event(md, src, tick, -1); //Cast targetted skill event.
			if (tstatus->mode&(MD_CASTSENSOR_IDLE|MD_CASTSENSOR_CHASE) &&
				battle->check_target(target, src, BCT_ENEMY) > 0)
			{
				switch (md->state.skillstate) {
				case MSS_RUSH:
				case MSS_FOLLOW:
					if (!(tstatus->mode&MD_CASTSENSOR_CHASE))
						break;
					md->target_id = src->id;
					md->state.aggressive = (tstatus->mode&MD_ANGRY)?1:0;
					md->min_chase = md->db->range3;
					break;
				case MSS_IDLE:
				case MSS_WALK:
					if (!(tstatus->mode&MD_CASTSENSOR_IDLE))
						break;
					md->target_id = src->id;
					md->state.aggressive = (tstatus->mode&MD_ANGRY)?1:0;
					md->min_chase = md->db->range3;
					break;
				}
			}
		}
	}

	if( casttime <= 0 )
		ud->state.skillcastcancel = 0;

	if( !sd || sd->skillitem != skill_id || skill->get_cast(skill_id,skill_lv) )
		ud->canact_tick = tick + casttime + 100;
	if( sd )
	{
		switch( skill_id )
		{
		case CG_ARROWVULCAN:
			sd->canequip_tick = tick + casttime;
			break;
		}
	}
	ud->skilltarget  = target_id;
	ud->skillx       = 0;
	ud->skilly       = 0;
	ud->skill_id      = skill_id;
	ud->skill_lv      = skill_lv;

	if( sc ) {
		/**
		 * why the if else chain: these 3 status do not stack, so its efficient that way.
		 **/
 		if( sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4&4) && skill_id != AS_CLOAKING ) {
			status_change_end(src, SC_CLOAKING, INVALID_TIMER);
			if (!src->prev) return 0; //Warped away!
		} else if( sc->data[SC_CLOAKINGEXCEED] && !(sc->data[SC_CLOAKINGEXCEED]->val4&4) && skill_id != GC_CLOAKINGEXCEED ) {
			status_change_end(src,SC_CLOAKINGEXCEED, INVALID_TIMER);
			if (!src->prev) return 0;
		}
	}


	if( casttime > 0 ) {
		ud->skilltimer = iTimer->add_timer( tick+casttime, skill->castend_id, src->id, 0 );
		if( sd && (pc->checkskill(sd,SA_FREECAST) > 0 || skill_id == LG_EXEEDBREAK) )
			status_calc_bl(&sd->bl, SCB_SPEED);
	} else
		skill->castend_id(ud->skilltimer,tick,src->id,0);

	return 1;
}

int unit_skilluse_pos(struct block_list *src, short skill_x, short skill_y, uint16 skill_id, uint16 skill_lv)
{
	return unit_skilluse_pos2(
		src, skill_x, skill_y, skill_id, skill_lv,
		skill->cast_fix(src, skill_id, skill_lv),
		skill->get_castcancel(skill_id)
	);
}

int unit_skilluse_pos2( struct block_list *src, short skill_x, short skill_y, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel)
{
	struct map_session_data *sd = NULL;
	struct unit_data        *ud = NULL;
	struct status_change *sc;
	struct block_list    bl;
	unsigned int tick = iTimer->gettick();
	int range;

	nullpo_ret(src);

	if (!src->prev) return 0; // not on the map
	if(iStatus->isdead(src)) return 0;

	sd = BL_CAST(BL_PC, src);
	ud = unit_bl2ud(src);
	if(ud == NULL) return 0;

	if(ud->skilltimer != INVALID_TIMER) //Normally not needed since clif.c checks for it, but at/char/script commands don't! [Skotlex]
		return 0;

	sc = iStatus->get_sc(src);
	if (sc && !sc->count)
		sc = NULL;

	if( sd )
	{
		if( skill->not_ok(skill_id, sd) || !skill->check_condition_castbegin(sd, skill_id, skill_lv) )
			return 0;
		/**
		 * "WHY IS IT HEREE": pneuma cannot be cancelled past this point, the client displays the animation even,
		 * if we cancel it from nodamage_id, so it has to be here for it to not display the animation.
		 **/
		if( skill_id == AL_PNEUMA && iMap->getcell(src->m, skill_x, skill_y, CELL_CHKLANDPROTECTOR) ) {
			clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		if( (skill_id >= SC_MANHOLE && skill_id <= SC_FEINTBOMB) && iMap->getcell(src->m, skill_x, skill_y, CELL_CHKMAELSTROM) ) {
			clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
	}

	if (!iStatus->check_skilluse(src, NULL, skill_id, 0))
		return 0;

	if( iMap->getcell(src->m, skill_x, skill_y, CELL_CHKWALL) )
	{// can't cast ground targeted spells on wall cells
		if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
		return 0;
	}

    /* Check range and obstacle */
	bl.type = BL_NUL;
	bl.m = src->m;
	bl.x = skill_x;
	bl.y = skill_y;

	if (src->type == BL_NPC) // NPC-objects can override cast distance
		range = AREA_SIZE; // Maximum visible distance before NPC goes out of sight
	else
		range = skill->get_range2(src, skill_id, skill_lv); // Skill cast distance from database

	if( skill->get_state(ud->skill_id) == ST_MOVE_ENABLE ) {
		if( !unit_can_reach_bl(src, &bl, range + 1, 1, NULL, NULL) )
			return 0; //Walk-path check failed.
	} else if( !battle->check_range(src, &bl, range + 1) )
		return 0; //Arrow-path check failed.

	unit_stop_attack(src);

	// moved here to prevent Suffragium from ending if skill fails
#ifndef RENEWAL_CAST
	if (!(skill->get_castnodex(skill_id, skill_lv)&2))
		casttime = skill->cast_fix_sc(src, casttime);
#else
	casttime = skill->vf_cast_fix(src, casttime, skill_id, skill_lv );
#endif

	if (src->type == BL_NPC) { // NPC-objects do not have cast time
		casttime = 0;
	}

	ud->state.skillcastcancel = castcancel&&casttime>0?1:0;
	if( !sd || sd->skillitem != skill_id || skill->get_cast(skill_id,skill_lv) )
		ud->canact_tick  = tick + casttime + 100;
//	if( sd )
//	{
//		switch( skill_id )
//		{
//		case ????:
//			sd->canequip_tick = tick + casttime;
//		}
//	}
	ud->skill_id      = skill_id;
	ud->skill_lv      = skill_lv;
	ud->skillx       = skill_x;
	ud->skilly       = skill_y;
	ud->skilltarget  = 0;

	if( sc ) {
		/**
		 * why the if else chain: these 3 status do not stack, so its efficient that way.
		 **/
		if (sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4&4)) {
			status_change_end(src, SC_CLOAKING, INVALID_TIMER);
			if (!src->prev) return 0; //Warped away!
		} else if (sc->data[SC_CLOAKINGEXCEED] && !(sc->data[SC_CLOAKINGEXCEED]->val4&4)) {
			status_change_end(src, SC_CLOAKINGEXCEED, INVALID_TIMER);
			if (!src->prev) return 0;
		}
	}

	unit_stop_walking(src,1);
	// in official this is triggered even if no cast time.
	clif->skillcasting(src, src->id, 0, skill_x, skill_y, skill_id, skill->get_ele(skill_id, skill_lv), casttime);
	if( casttime > 0 ) {
		ud->skilltimer = iTimer->add_timer( tick+casttime, skill->castend_pos, src->id, 0 );
		if( (sd && pc->checkskill(sd,SA_FREECAST) > 0) || skill_id == LG_EXEEDBREAK)
			status_calc_bl(&sd->bl, SCB_SPEED);	
	} else {
		ud->skilltimer = INVALID_TIMER;
		skill->castend_pos(ud->skilltimer,tick,src->id,0);
	}
	return 1;
}

/*========================================
 * update a block's attack target
 *----------------------------------------*/
int unit_set_target(struct unit_data* ud, int target_id)
{
	struct unit_data * ux;
	struct block_list* target;

	nullpo_ret(ud);

	if( ud->target != target_id ) {
		if( ud->target && (target = iMap->id2bl(ud->target)) && (ux = unit_bl2ud(target)) && ux->target_count > 0 )
			ux->target_count --;
		if( target_id && (target = iMap->id2bl(target_id)) && (ux = unit_bl2ud(target)) )
			ux->target_count ++;
	}

	ud->target = target_id;
	return 0;
}

int unit_stop_attack(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_ret(bl);

	if(!ud || ud->attacktimer == INVALID_TIMER)
		return 0;

	iTimer->delete_timer( ud->attacktimer, unit_attack_timer );
	ud->attacktimer = INVALID_TIMER;
	unit_set_target(ud, 0);
	return 0;
}

//Means current target is unattackable. For now only unlocks mobs.
int unit_unattackable(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	if (ud) {
		ud->state.attack_continue = 0;
		unit_set_target(ud, 0);
	}

	if(bl->type == BL_MOB)
		mob_unlocktarget((struct mob_data*)bl, iTimer->gettick()) ;
	else if(bl->type == BL_PET)
		pet_unlocktarget((struct pet_data*)bl);
	return 0;
}

/*==========================================
 * Attack request
 * If type is an ongoing attack
 *------------------------------------------*/
int unit_attack(struct block_list *src,int target_id,int continuous)
{
	struct block_list *target;
	struct unit_data  *ud;

	nullpo_ret(ud = unit_bl2ud(src));

	target = iMap->id2bl(target_id);
	if( target==NULL || iStatus->isdead(target) ) {
		unit_unattackable(src);
		return 1;
	}

	if( src->type == BL_PC ) {
		TBL_PC* sd = (TBL_PC*)src;
		if( target->type == BL_NPC ) { // monster npcs [Valaris]
			npc_click(sd,(TBL_NPC*)target); // submitted by leinsirk10 [Celest]
			return 0;
		}
		if( pc_is90overweight(sd) || pc_isridingwug(sd) ) { // overweight or mounted on warg - stop attacking
			unit_stop_attack(src);
			return 0;
		}
	}
	if( battle->check_target(src,target,BCT_ENEMY) <= 0 || !iStatus->check_skilluse(src, target, 0, 0) ) {
		unit_unattackable(src);
		return 1;
	}
	ud->state.attack_continue = continuous;
	unit_set_target(ud, target_id);

	if (continuous) //If you're to attack continously, set to auto-case character
		ud->chaserange = status_get_range(src);

	//Just change target/type. [Skotlex]
	if(ud->attacktimer != INVALID_TIMER)
		return 0;

	//Set Mob's ANGRY/BERSERK states.
	if(src->type == BL_MOB)
		((TBL_MOB*)src)->state.skillstate = ((TBL_MOB*)src)->state.aggressive?MSS_ANGRY:MSS_BERSERK;

	if(DIFF_TICK(ud->attackabletime, iTimer->gettick()) > 0)
		//Do attack next time it is possible. [Skotlex]
		ud->attacktimer=iTimer->add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
	else //Attack NOW.
		unit_attack_timer(INVALID_TIMER, iTimer->gettick(), src->id, 0);

	return 0;
}

//Cancels an ongoing combo, resets attackable time and restarts the
//attack timer to resume attacking after amotion time. [Skotlex]
int unit_cancel_combo(struct block_list *bl)
{
	struct unit_data  *ud;

	if (!status_change_end(bl, SC_COMBOATTACK, INVALID_TIMER))
		return 0; //Combo wasn't active.

	ud = unit_bl2ud(bl);
	nullpo_ret(ud);

	ud->attackabletime = iTimer->gettick() + status_get_amotion(bl);

	if (ud->attacktimer == INVALID_TIMER)
		return 1; //Nothing more to do.

	iTimer->delete_timer(ud->attacktimer, unit_attack_timer);
	ud->attacktimer=iTimer->add_timer(ud->attackabletime,unit_attack_timer,bl->id,0);
	return 1;
}
/*==========================================
 *
 *------------------------------------------*/
bool unit_can_reach_pos(struct block_list *bl,int x,int y, int easy)
{
	nullpo_retr(false, bl);

	if (bl->x == x && bl->y == y) //Same place
		return true;

	return path_search(NULL,bl->m,bl->x,bl->y,x,y,easy,CELL_CHKNOREACH);
}

/*==========================================
 *
 *------------------------------------------*/
bool unit_can_reach_bl(struct block_list *bl,struct block_list *tbl, int range, int easy, short *x, short *y)
{
	int i;
	short dx,dy;
	nullpo_retr(false, bl);
	nullpo_retr(false, tbl);

	if( bl->m != tbl->m)
		return false;

	if( bl->x==tbl->x && bl->y==tbl->y )
		return true;

	if(range>0 && !check_distance_bl(bl, tbl, range))
		return false;

	// It judges whether it can adjoin or not.
	dx=tbl->x - bl->x;
	dy=tbl->y - bl->y;
	dx=(dx>0)?1:((dx<0)?-1:0);
	dy=(dy>0)?1:((dy<0)?-1:0);

	if (iMap->getcell(tbl->m,tbl->x-dx,tbl->y-dy,CELL_CHKNOPASS))
	{	//Look for a suitable cell to place in.
		for(i=0;i<9 && iMap->getcell(tbl->m,tbl->x-dirx[i],tbl->y-diry[i],CELL_CHKNOPASS);i++);
		if (i==9) return false; //No valid cells.
		dx = dirx[i];
		dy = diry[i];
	}

	if (x) *x = tbl->x-dx;
	if (y) *y = tbl->y-dy;
	return path_search(NULL,bl->m,bl->x,bl->y,tbl->x-dx,tbl->y-dy,easy,CELL_CHKNOREACH);
}
/*==========================================
 * Calculates position of Pet/Mercenary/Homunculus/Elemental
 *------------------------------------------*/
int	unit_calc_pos(struct block_list *bl, int tx, int ty, uint8 dir)
{
	int dx, dy, x, y, i, k;
	struct unit_data *ud = unit_bl2ud(bl);
	nullpo_ret(ud);

	if(dir > 7)
		return 1;

	ud->to_x = tx;
	ud->to_y = ty;

	// 2 cells from Master Position
	dx = -dirx[dir] * 2;
	dy = -diry[dir] * 2;
	x = tx + dx;
	y = ty + dy;

	if( !unit_can_reach_pos(bl, x, y, 0) )
	{
		if( dx > 0 ) x--; else if( dx < 0 ) x++;
		if( dy > 0 ) y--; else if( dy < 0 ) y++;
		if( !unit_can_reach_pos(bl, x, y, 0) )
		{
			for( i = 0; i < 12; i++ )
			{
				k = rnd()%8; // Pick a Random Dir
				dx = -dirx[k] * 2;
				dy = -diry[k] * 2;
				x = tx + dx;
				y = ty + dy;
				if( unit_can_reach_pos(bl, x, y, 0) )
					break;
				else
				{
					if( dx > 0 ) x--; else if( dx < 0 ) x++;
					if( dy > 0 ) y--; else if( dy < 0 ) y++;
					if( unit_can_reach_pos(bl, x, y, 0) )
						break;
				}
			}
			if( i == 12 )
			{
				x = tx; y = tx; // Exactly Master Position
				if( !unit_can_reach_pos(bl, x, y, 0) )
					return 1;
			}
		}
	}
	ud->to_x = x;
	ud->to_y = y;

	return 0;
}

/*==========================================
 * Continuous Attack (function timer)
 *------------------------------------------*/
static int unit_attack_timer_sub(struct block_list* src, int tid, unsigned int tick)
{
	struct block_list *target;
	struct unit_data *ud;
	struct status_data *sstatus;
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	int range;

	if( (ud=unit_bl2ud(src))==NULL )
		return 0;
	if( ud->attacktimer != tid )
	{
		ShowError("unit_attack_timer %d != %d\n",ud->attacktimer,tid);
		return 0;
	}

	sd = BL_CAST(BL_PC, src);
	md = BL_CAST(BL_MOB, src);
	ud->attacktimer = INVALID_TIMER;
	target=iMap->id2bl(ud->target);

	if( src == NULL || src->prev == NULL || target==NULL || target->prev == NULL )
		return 0;

	if( iStatus->isdead(src) || iStatus->isdead(target) ||
			battle->check_target(src,target,BCT_ENEMY) <= 0 || !iStatus->check_skilluse(src, target, 0, 0)
#ifdef OFFICIAL_WALKPATH
	   || !path_search_long(NULL, src->m, src->x, src->y, target->x, target->y, CELL_CHKWALL)
#endif
	   )
		return 0; // can't attack under these conditions

	if( src->m != target->m )
	{
		if( src->type == BL_MOB && mob_warpchase((TBL_MOB*)src, target) )
			return 1; // Follow up.
		return 0;
	}

	if( ud->skilltimer != INVALID_TIMER && !(sd && pc->checkskill(sd,SA_FREECAST) > 0) )
		return 0; // can't attack while casting

	if( !battle_config.sdelay_attack_enable && DIFF_TICK(ud->canact_tick,tick) > 0 && !(sd && pc->checkskill(sd,SA_FREECAST) > 0) )
	{ // attacking when under cast delay has restrictions:
		if( tid == INVALID_TIMER ) { //requested attack.
			if(sd) clif->skill_fail(sd,1,USESKILL_FAIL_SKILLINTERVAL,0);
			return 0;
		}
		//Otherwise, we are in a combo-attack, delay this until your canact time is over. [Skotlex]
		if( ud->state.attack_continue ) {
			if( DIFF_TICK(ud->canact_tick, ud->attackabletime) > 0 )
				ud->attackabletime = ud->canact_tick;
			ud->attacktimer=iTimer->add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
		}
		return 1;
	}

	sstatus = iStatus->get_status_data(src);
	range = sstatus->rhw.range + 1;

	if( unit_is_walking(target) )
		range++; //Extra range when chasing
	if( !check_distance_bl(src,target,range) ) { //Chase if required.
		if(sd)
			clif->movetoattack(sd,target);
		else if(ud->state.attack_continue)
			unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		return 1;
	}
	if( !battle->check_range(src,target,range) ) {
	  	//Within range, but no direct line of attack
		if( ud->state.attack_continue ) {
			if(ud->chaserange > 2) ud->chaserange-=2;
			unit_walktobl(src,target,ud->chaserange,ud->state.walk_easy|2);
		}
		return 1;
	}
	//Sync packet only for players.
	//Non-players use the sync packet on the walk timer. [Skotlex]
	if (tid == INVALID_TIMER && sd) clif->fixpos(src);

	if( DIFF_TICK(ud->attackabletime,tick) <= 0 )
	{
		if (battle_config.attack_direction_change && (src->type&battle_config.attack_direction_change)) {
			ud->dir = iMap->calc_dir(src, target->x,target->y );
		}
		if(ud->walktimer != INVALID_TIMER)
			unit_stop_walking(src,1);
		if(md) {
			if (mobskill_use(md,tick,-1))
				return 1;
			if (sstatus->mode&MD_ASSIST && DIFF_TICK(md->last_linktime, tick) < MIN_MOBLINKTIME)
			{	// Link monsters nearby [Skotlex]
				md->last_linktime = tick;
				iMap->foreachinrange(mob_linksearch, src, md->db->range2, BL_MOB, md->class_, target, tick);
			}
		}
		if(src->type == BL_PET && pet_attackskill((TBL_PET*)src, target->id))
			return 1;

		iMap->freeblock_lock();
		ud->attacktarget_lv = battle->weapon_attack(src,target,tick,0);

		if(sd && sd->status.pet_id > 0 && sd->pd && battle_config.pet_attack_support)
			pet_target_check(sd,target,0);
		iMap->freeblock_unlock();
		/**
		 * Applied when you're unable to attack (e.g. out of ammo)
		 * We should stop here otherwise timer keeps on and this happens endlessly
		 **/
		if( ud->attacktarget_lv == ATK_NONE )
			return 1;

		ud->attackabletime = tick + sstatus->adelay;
//		You can't move if you can't attack neither.
		if (src->type&battle_config.attack_walk_delay)
			unit_set_walkdelay(src, tick, sstatus->amotion, 1);
	}

	if(ud->state.attack_continue) {
		if( src->type == BL_PC )
			((TBL_PC*)src)->idletime = last_tick;
		ud->attacktimer = iTimer->add_timer(ud->attackabletime,unit_attack_timer,src->id,0);
	}

	return 1;
}

static int unit_attack_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *bl;
	bl = iMap->id2bl(id);
	if(bl && unit_attack_timer_sub(bl, tid, tick) == 0)
		unit_unattackable(bl);
	return 0;
}

/*==========================================
 * Cancels an ongoing skill cast.
 * flag&1: Cast-Cancel invoked.
 * flag&2: Cancel only if skill is cancellable.
 *------------------------------------------*/
int unit_skillcastcancel(struct block_list *bl,int type)
{
	struct map_session_data *sd = NULL;
	struct unit_data *ud = unit_bl2ud( bl);
	unsigned int tick=iTimer->gettick();
	int ret=0, skill_id;

	nullpo_ret(bl);
	if (!ud || ud->skilltimer == INVALID_TIMER)
		return 0; //Nothing to cancel.

	sd = BL_CAST(BL_PC, bl);

	if (type&2) {
		//See if it can be cancelled.
		if (!ud->state.skillcastcancel)
			return 0;

		if (sd && (sd->special_state.no_castcancel2 ||
                ((sd->sc.data[SC_UNLIMITED_HUMMING_VOICE] || sd->special_state.no_castcancel) && !map_flag_gvg(bl->m) && !map[bl->m].flag.battleground))) //fixed flags being read the wrong way around [blackhole89]
			return 0;
	}

	ud->canact_tick = tick;

	if(type&1 && sd)
		skill_id = sd->skill_id_old;
	else
		skill_id = ud->skill_id;

	if (skill->get_inf(skill_id) & INF_GROUND_SKILL)
		ret = iTimer->delete_timer( ud->skilltimer, skill->castend_pos );
	else
		ret = iTimer->delete_timer( ud->skilltimer, skill->castend_id );
	if( ret < 0 )
		ShowError("delete timer error : skill_id : %d\n",ret);

	ud->skilltimer = INVALID_TIMER;

	if( sd && pc->checkskill(sd,SA_FREECAST) > 0 )
		status_calc_bl(&sd->bl, SCB_SPEED);

	if( sd ) {
		switch( skill_id ) {
			case CG_ARROWVULCAN:
				sd->canequip_tick = tick;
				break;
		}
	}

	if(bl->type==BL_MOB) ((TBL_MOB*)bl)->skill_idx  = -1;

	clif->skillcastcancel(bl);
	return 1;
}

// unit_data initialization process
void unit_dataset(struct block_list *bl) {
	struct unit_data *ud;
	nullpo_retv(ud = unit_bl2ud(bl));

	memset( ud, 0, sizeof( struct unit_data) );
	ud->bl             = bl;
	ud->walktimer      = INVALID_TIMER;
	ud->skilltimer     = INVALID_TIMER;
	ud->attacktimer    = INVALID_TIMER;
	ud->attackabletime =
	ud->canact_tick    =
	ud->canmove_tick   = iTimer->gettick();
}

/*==========================================
 * Counts the number of units attacking 'bl'
 *------------------------------------------*/
int unit_counttargeted(struct block_list* bl)
{
	struct unit_data* ud;
	if( bl && (ud = unit_bl2ud(bl)) )
		return ud->target_count;
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int unit_fixdamage(struct block_list *src,struct block_list *target,unsigned int tick,int sdelay,int ddelay,int damage,int div,int type,int damage2)
{
	nullpo_ret(target);

	if(damage+damage2 <= 0)
		return 0;

	return status_fix_damage(src,target,damage+damage2,clif->damage(target,target,tick,sdelay,ddelay,damage,div,type,damage2));
}

/*==========================================
 * To change the size of the char (player or mob only)
 *------------------------------------------*/
int unit_changeviewsize(struct block_list *bl,short size)
{
	nullpo_ret(bl);

	size=(size<0)?-1:(size>0)?1:0;

	if(bl->type == BL_PC) {
		((TBL_PC*)bl)->state.size=size;
	} else if(bl->type == BL_MOB) {
		((TBL_MOB*)bl)->special_state.size=size;
	} else
		return 0;
	if(size!=0)
		clif->specialeffect(bl,421+size, AREA);
	return 0;
}

/*==========================================
 * Removes a bl/ud from the map.
 * Returns 1 on success. 0 if it couldn't be removed or the bl was free'd
 * if clrtype is 1 (death), appropiate cleanup is performed.
 * Otherwise it is assumed bl is being warped.
 * On-Kill specific stuff is not performed here, look at iStatus->damage for that.
 *------------------------------------------*/
int unit_remove_map_(struct block_list *bl, clr_type clrtype, const char* file, int line, const char* func)
{
	struct unit_data *ud = unit_bl2ud(bl);
	struct status_change *sc = iStatus->get_sc(bl);
	nullpo_ret(ud);

	if(bl->prev == NULL)
		return 0; //Already removed?

	iMap->freeblock_lock();

	unit_set_target(ud, 0);

	if (ud->walktimer != INVALID_TIMER)
		unit_stop_walking(bl,0);
	if (ud->attacktimer != INVALID_TIMER)
		unit_stop_attack(bl);
	if (ud->skilltimer != INVALID_TIMER)
		unit_skillcastcancel(bl,0);

// Do not reset can-act delay. [Skotlex]
	ud->attackabletime = ud->canmove_tick /*= ud->canact_tick*/ = iTimer->gettick();
	if(sc && sc->count ) { //map-change/warp dispells.
		status_change_end(bl, SC_BLADESTOP, INVALID_TIMER);
		status_change_end(bl, SC_BASILICA, INVALID_TIMER);
		status_change_end(bl, SC_ANKLESNARE, INVALID_TIMER);
		status_change_end(bl, SC_TRICKDEAD, INVALID_TIMER);
		status_change_end(bl, SC_BLADESTOP_WAIT, INVALID_TIMER);
		status_change_end(bl, SC_RUN, INVALID_TIMER);
		status_change_end(bl, SC_DANCING, INVALID_TIMER);
		status_change_end(bl, SC_WARM, INVALID_TIMER);
		status_change_end(bl, SC_DEVOTION, INVALID_TIMER);
		status_change_end(bl, SC_MARIONETTE_MASTER, INVALID_TIMER);
		status_change_end(bl, SC_MARIONETTE, INVALID_TIMER);
		status_change_end(bl, SC_RG_CCONFINE_M, INVALID_TIMER);
		status_change_end(bl, SC_RG_CCONFINE_S, INVALID_TIMER);
		status_change_end(bl, SC_HIDING, INVALID_TIMER);
		// Ensure the bl is a PC; if so, we'll handle the removal of cloaking and cloaking exceed later
		if ( bl->type != BL_PC )
		{
			status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
		}
		status_change_end(bl, SC_CHASEWALK, INVALID_TIMER);
		if (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)
			status_change_end(bl, SC_GOSPEL, INVALID_TIMER);
		status_change_end(bl, SC_HLIF_CHANGE, INVALID_TIMER);
		status_change_end(bl, SC_STOP, INVALID_TIMER);
		status_change_end(bl, SC_WUGDASH, INVALID_TIMER);
		status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
		status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
		status_change_end(bl, SC__MANHOLE, INVALID_TIMER);
		status_change_end(bl, SC_VACUUM_EXTREME, INVALID_TIMER);
		status_change_end(bl, SC_CURSEDCIRCLE_ATKER, INVALID_TIMER); //callme before warp
	}

	if (bl->type&(BL_CHAR|BL_PET)) {
		skill->unit_move(bl,iTimer->gettick(),4);
		skill->cleartimerskill(bl);
	}

	switch( bl->type ) {
		case BL_PC: {
			struct map_session_data *sd = (struct map_session_data*)bl;

			if(sd->shadowform_id){
			    struct block_list *d_bl = iMap->id2bl(sd->shadowform_id);
			    if( d_bl )
				    status_change_end(d_bl,SC__SHADOWFORM,INVALID_TIMER);
			}
			//Leave/reject all invitations.
			if(sd->chatID)
				chat_leavechat(sd,0);
			if(sd->trade_partner)
				trade->cancel(sd);
			buyingstore->close(sd);
			searchstore->close(sd);
			if(sd->state.storage_flag == 1)
				storage->pc_quit(sd,0);
			else if (sd->state.storage_flag == 2)
				gstorage->pc_quit(sd,0);
			sd->state.storage_flag = 0; //Force close it when being warped.
			if(sd->party_invite>0)
				party->reply_invite(sd,sd->party_invite,0);
			if(sd->guild_invite>0)
				guild->reply_invite(sd,sd->guild_invite,0);
			if(sd->guild_alliance>0)
				guild->reply_reqalliance(sd,sd->guild_alliance_account,0);
			if(sd->menuskill_id)
				sd->menuskill_id = sd->menuskill_val = 0;
			if( sd->touching_id )
				npc_touchnext_areanpc(sd,true);

			// Check if warping and not changing the map.
			if ( sd->state.warping && !sd->state.changemap ) {
				status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
				status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			}

			sd->npc_shopid = 0;
			sd->adopt_invite = 0;

			if(sd->pvp_timer != INVALID_TIMER) {
				iTimer->delete_timer(sd->pvp_timer,pc->calc_pvprank_timer);
				sd->pvp_timer = INVALID_TIMER;
				sd->pvp_rank = 0;
			}
			if(sd->duel_group > 0)
				duel_leave(sd->duel_group, sd);

			if(pc_issit(sd)) {
				pc->setstand(sd);
				skill->sit(sd,0);
			}
			party->send_dot_remove(sd);//minimap dot fix [Kevin]
			guild->send_dot_remove(sd);
			bg_send_dot_remove(sd);

			if( map[bl->m].users <= 0 || sd->state.debug_remove_map )
			{// this is only place where map users is decreased, if the mobs were removed too soon then this function was executed too many times [FlavioJS]
				if( sd->debug_file == NULL || !(sd->state.debug_remove_map) )
				{
					sd->debug_file = "";
					sd->debug_line = 0;
					sd->debug_func = "";
				}
				ShowDebug("unit_remove_map: unexpected state when removing player AID/CID:%d/%d"
					" (active=%d connect_new=%d rewarp=%d changemap=%d debug_remove_map=%d)"
					" from map=%s (users=%d)."
					" Previous call from %s:%d(%s), current call from %s:%d(%s)."
					" Please report this!!!\n",
					sd->status.account_id, sd->status.char_id,
					sd->state.active, sd->state.connect_new, sd->state.rewarp, sd->state.changemap, sd->state.debug_remove_map,
					map[bl->m].name, map[bl->m].users,
					sd->debug_file, sd->debug_line, sd->debug_func, file, line, func);
			} else if (--map[bl->m].users == 0 && battle_config.dynamic_mobs)	//[Skotlex]
				iMap->removemobs(bl->m);
			if( !(sd->sc.option&OPTION_INVISIBLE) )
			{// decrement the number of active pvp players on the map
				--map[bl->m].users_pvp;
			}
			if( map[bl->m].instance_id >= 0 ) {
				instances[map[bl->m].instance_id].users--;
				instance->check_idle(map[bl->m].instance_id);
			}
			sd->state.debug_remove_map = 1; // temporary state to track double remove_map's [FlavioJS]
			sd->debug_file = file;
			sd->debug_line = line;
			sd->debug_func = func;

			break;
		}
		case BL_MOB: {
			struct mob_data *md = (struct mob_data*)bl;
			// Drop previous target mob_slave_keep_target: no.
			if (!battle_config.mob_slave_keep_target)
				md->target_id=0;

			md->attacked_id=0;
			md->state.skillstate= MSS_IDLE;

			break;
		}
		case BL_PET: {
			struct pet_data *pd = (struct pet_data*)bl;
			if( pd->pet.intimate <= 0 && !(pd->msd && !pd->msd->state.active) )
			{	//If logging out, this is deleted on unit_free
				clif->clearunit_area(bl,clrtype);
				iMap->delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				iMap->freeblock_unlock();
				return 0;
			}

			break;
		}
		case BL_HOM: {
			struct homun_data *hd = (struct homun_data *)bl;
			ud->canact_tick = ud->canmove_tick; //It appears HOM do reset the can-act tick.
			if( !hd->homunculus.intimacy && !(hd->master && !hd->master->state.active) )
			{	//If logging out, this is deleted on unit_free
				clif->emotion(bl, E_SOB);
				clif->clearunit_area(bl,clrtype);
				iMap->delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				iMap->freeblock_unlock();
				return 0;
			}
			break;
		}
		case BL_MER: {
			struct mercenary_data *md = (struct mercenary_data *)bl;
			ud->canact_tick = ud->canmove_tick;
			if( mercenary_get_lifetime(md) <= 0 && !(md->master && !md->master->state.active) )
			{
				clif->clearunit_area(bl,clrtype);
				iMap->delblock(bl);
				unit_free(bl,CLR_OUTSIGHT);
				iMap->freeblock_unlock();
				return 0;
			}
			break;
		}
		case BL_ELEM: {
			struct elemental_data *ed = (struct elemental_data *)bl;
			ud->canact_tick = ud->canmove_tick;
			if( elemental_get_lifetime(ed) <= 0 && !(ed->master && !ed->master->state.active) )
			{
				clif->clearunit_area(bl,clrtype);
				iMap->delblock(bl);
				unit_free(bl,0);
				iMap->freeblock_unlock();
				return 0;
			}
			break;
		}
		default: break;// do nothing
	}
	/**
	 * BL_MOB is handled by mob_dead unless the monster is not dead.
	 **/
	if( bl->type != BL_MOB || !iStatus->isdead(bl) )
		clif->clearunit_area(bl,clrtype);
	iMap->delblock(bl);
	iMap->freeblock_unlock();
	return 1;
}

void unit_remove_map_pc(struct map_session_data *sd, clr_type clrtype)
{
	unit_remove_map(&sd->bl,clrtype);

	if (clrtype == CLR_TELEPORT) clrtype = CLR_OUTSIGHT; //CLR_TELEPORT is the warp from logging out, but pets/homunc need to just 'vanish' instead of showing the warping out animation.

	if(sd->pd)
		unit_remove_map(&sd->pd->bl, clrtype);
	if(homun_alive(sd->hd))
		unit_remove_map(&sd->hd->bl, clrtype);
	if(sd->md)
		unit_remove_map(&sd->md->bl, clrtype);
	if(sd->ed)
		unit_remove_map(&sd->ed->bl, clrtype);
}

void unit_free_pc(struct map_session_data *sd)
{
	if (sd->pd) unit_free(&sd->pd->bl,CLR_OUTSIGHT);
	if (sd->hd) unit_free(&sd->hd->bl,CLR_OUTSIGHT);
	if (sd->md) unit_free(&sd->md->bl,CLR_OUTSIGHT);
	if (sd->ed) unit_free(&sd->ed->bl,CLR_OUTSIGHT);
	unit_free(&sd->bl,CLR_TELEPORT);
}

/*==========================================
 * Function to free all related resources to the bl
 * if unit is on map, it is removed using the clrtype specified
 *------------------------------------------*/
int unit_free(struct block_list *bl, clr_type clrtype)
{
	struct unit_data *ud = unit_bl2ud( bl );
	nullpo_ret(ud);

	iMap->freeblock_lock();
	if( bl->prev )	//Players are supposed to logout with a "warp" effect.
		unit_remove_map(bl, clrtype);

	switch( bl->type ) {
		case BL_PC:
		{
			struct map_session_data *sd = (struct map_session_data*)bl;
			int i;

			if( iStatus->isdead(bl) )
				pc->setrestartvalue(sd,2);

			pc->delinvincibletimer(sd);
			pc->delautobonus(sd,sd->autobonus,ARRAYLENGTH(sd->autobonus),false);
			pc->delautobonus(sd,sd->autobonus2,ARRAYLENGTH(sd->autobonus2),false);
			pc->delautobonus(sd,sd->autobonus3,ARRAYLENGTH(sd->autobonus3),false);

			if( sd->followtimer != INVALID_TIMER )
				pc->stop_following(sd);

			if( sd->duel_invite > 0 )
				duel_reject(sd->duel_invite, sd);

			// Notify friends that this char logged out. [Skotlex]
			iMap->map_foreachpc(clif->friendslist_toggle_sub, sd->status.account_id, sd->status.char_id, 0);
			party->send_logout(sd);
			guild->send_memberinfoshort(sd,0);
			pc->cleareventtimer(sd);
			pc->inventory_rental_clear(sd);
			pc->delspiritball(sd,sd->spiritball,1);
			for(i = 1; i < 5; i++)
				pc->del_charm(sd, sd->charm[i], i);

			if( sd->reg ) {	//Double logout already freed pointer fix... [Skotlex]
				aFree(sd->reg);
				sd->reg = NULL;
				sd->reg_num = 0;
			}
			if( sd->regstr ) {
				for( i = 0; i < sd->regstr_num; ++i )
					if( sd->regstr[i].data )
						aFree(sd->regstr[i].data);
				aFree(sd->regstr);
				sd->regstr = NULL;
				sd->regstr_num = 0;
			}
			if( sd->st && sd->st->state != RUN ) {// free attached scripts that are waiting
				script_free_state(sd->st);
				sd->st = NULL;
				sd->npc_id = 0;
			}
			if( sd->combos.count ) {
				aFree(sd->combos.bonus);
				aFree(sd->combos.id);
				sd->combos.count = 0;
			}
			/* [Ind/Hercules] */
			if( sd->sc_display_count ) {
				for(i = 0; i < sd->sc_display_count; i++) {
					ers_free(pc_sc_display_ers, sd->sc_display[i]);
				}
				sd->sc_display_count = 0;
			}
			if( sd->sc_display != NULL ) {
				aFree(sd->sc_display);
				sd->sc_display = NULL;
			}
			if( sd->instance != NULL ) {
				aFree(sd->instance);
				sd->instance = NULL;
			}
			if( sd->queues != NULL ) {
				aFree(sd->queues);
				sd->queues = NULL;
			}
			break;
		}
		case BL_PET:
		{
			struct pet_data *pd = (struct pet_data*)bl;
			struct map_session_data *sd = pd->msd;
			pet_hungry_timer_delete(pd);
			if( pd->a_skill )
			{
				aFree(pd->a_skill);
				pd->a_skill = NULL;
			}
			if( pd->s_skill )
			{
				if (pd->s_skill->timer != INVALID_TIMER) {
					if (pd->s_skill->id)
						iTimer->delete_timer(pd->s_skill->timer, pet_skill_support_timer);
					else
						iTimer->delete_timer(pd->s_skill->timer, pet_heal_timer);
				}
				aFree(pd->s_skill);
				pd->s_skill = NULL;
			}
			if( pd->recovery )
			{
				if(pd->recovery->timer != INVALID_TIMER)
					iTimer->delete_timer(pd->recovery->timer, pet_recovery_timer);
				aFree(pd->recovery);
				pd->recovery = NULL;
			}
			if( pd->bonus )
			{
				if (pd->bonus->timer != INVALID_TIMER)
					iTimer->delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
				aFree(pd->bonus);
				pd->bonus = NULL;
			}
			if( pd->loot )
			{
				pet_lootitem_drop(pd,sd);
				if (pd->loot->item)
					aFree(pd->loot->item);
				aFree (pd->loot);
				pd->loot = NULL;
			}
			if( pd->pet.intimate > 0 )
				intif_save_petdata(pd->pet.account_id,&pd->pet);
			else
			{	//Remove pet.
				intif_delete_petdata(pd->pet.pet_id);
				if (sd) sd->status.pet_id = 0;
			}
			if( sd )
				sd->pd = NULL;
			break;
		}
		case BL_MOB:
		{
			struct mob_data *md = (struct mob_data*)bl;
			if( md->spawn_timer != INVALID_TIMER )
			{
				iTimer->delete_timer(md->spawn_timer,mob_delayspawn);
				md->spawn_timer = INVALID_TIMER;
			}
			if( md->deletetimer != INVALID_TIMER )
			{
				iTimer->delete_timer(md->deletetimer,mob_timer_delete);
				md->deletetimer = INVALID_TIMER;
			}
			if( md->lootitem )
			{
				aFree(md->lootitem);
				md->lootitem=NULL;
			}
			if( md->guardian_data )
			{
				struct guild_castle* gc = md->guardian_data->castle;
				if( md->guardian_data->number >= 0 && md->guardian_data->number < MAX_GUARDIANS )
				{
					gc->guardian[md->guardian_data->number].id = 0;
				}
				else
				{
					int i;
					ARR_FIND(0, gc->temp_guardians_max, i, gc->temp_guardians[i] == md->bl.id);
					if( i < gc->temp_guardians_max )
						gc->temp_guardians[i] = 0;
				}
				aFree(md->guardian_data);
				md->guardian_data = NULL;
			}
			if( md->spawn )
			{
				md->spawn->active--;
				if( !md->spawn->state.dynamic )
				{// permanently remove the mob
					if( --md->spawn->num == 0 )
					{// Last freed mob is responsible for deallocating the group's spawn data.
						aFree(md->spawn);
						md->spawn = NULL;
					}
				}
			}
			if( md->base_status)
			{
				aFree(md->base_status);
				md->base_status = NULL;
			}
			if( mob_is_clone(md->class_) )
				mob_clone_delete(md);
			if( md->tomb_nid )
				mvptomb_destroy(md);
			break;
		}
		case BL_HOM:
		{
			struct homun_data *hd = (TBL_HOM*)bl;
			struct map_session_data *sd = hd->master;
			homun->hunger_timer_delete(hd);
			if( hd->homunculus.intimacy > 0 )
				homun->save(hd);
			else {
				intif_homunculus_requestdelete(hd->homunculus.hom_id);
				if( sd )
					sd->status.hom_id = 0;
			}
			if( sd )
				sd->hd = NULL;
			break;
		}
		case BL_MER:
		{
			struct mercenary_data *md = (TBL_MER*)bl;
			struct map_session_data *sd = md->master;
			if( mercenary_get_lifetime(md) > 0 )
				mercenary_save(md);
			else
			{
				intif_mercenary_delete(md->mercenary.mercenary_id);
				if( sd )
					sd->status.mer_id = 0;
			}
			if( sd )
				sd->md = NULL;

			merc_contract_stop(md);
			break;
		}
		case BL_ELEM: {
			struct elemental_data *ed = (TBL_ELEM*)bl;
			struct map_session_data *sd = ed->master;
			if( elemental_get_lifetime(ed) > 0 )
				elemental_save(ed);
			else {
				intif_elemental_delete(ed->elemental.elemental_id);
				if( sd )
					sd->status.ele_id = 0;
			}
			if( sd )
				sd->ed = NULL;

			elemental_summon_stop(ed);
			break;
		}
	}

	skill->clear_unitgroup(bl);
	iStatus->change_clear(bl,1);
	iMap->deliddb(bl);
	if( bl->type != BL_PC ) //Players are handled by map_quit
		iMap->freeblock(bl);
	iMap->freeblock_unlock();
	return 0;
}

int do_init_unit(void)
{
	iTimer->add_timer_func_list(unit_attack_timer,  "unit_attack_timer");
	iTimer->add_timer_func_list(unit_walktoxy_timer,"unit_walktoxy_timer");
	iTimer->add_timer_func_list(unit_walktobl_sub, "unit_walktobl_sub");
	iTimer->add_timer_func_list(unit_delay_walktoxy_timer,"unit_delay_walktoxy_timer");
	return 0;
}

int do_final_unit(void)
{
	// nothing to do
	return 0;
}
