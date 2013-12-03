// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/core.h"
#include "../common/timer.h"
#include "../common/ers.h"
#include "../common/grfio.h"
#include "../common/malloc.h"
#include "../common/socket.h" // WFIFO*()
#include "../common/showmsg.h"
#include "../common/nullpo.h"
#include "../common/random.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/conf.h"
#include "../common/console.h"
#include "../common/HPM.h"

#include "map.h"
#include "path.h"
#include "chrif.h"
#include "clif.h"
#include "duel.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "status.h"
#include "mob.h"
#include "npc.h" // npc_setcells(), npc_unsetcells()
#include "chat.h"
#include "itemdb.h"
#include "storage.h"
#include "skill.h"
#include "trade.h"
#include "party.h"
#include "unit.h"
#include "battle.h"
#include "battleground.h"
#include "quest.h"
#include "script.h"
#include "mapreg.h"
#include "guild.h"
#include "pet.h"
#include "homunculus.h"
#include "instance.h"
#include "mercenary.h"
#include "elemental.h"
#include "atcommand.h"
#include "log.h"
#include "mail.h"
#include "irc-bot.h"
#include "HPMmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#ifndef _WIN32
#include <unistd.h>
#endif

struct map_interface map_s;
struct mapit_interface mapit_s;

/*==========================================
 * server player count (of all mapservers)
 *------------------------------------------*/
void map_setusers(int users) {
	map->users = users;
}

int map_getusers(void) {
	return map->users;
}

/*==========================================
 * server player count (this mapserver only)
 *------------------------------------------*/
int map_usercount(void) {
	return db_size(map->pc_db);
}

/*==========================================
 * Attempt to free a map blocklist
 *------------------------------------------*/
int map_freeblock (struct block_list *bl) {
	nullpo_retr(map->block_free_lock, bl);
	
	if (map->block_free_lock == 0 || map->block_free_count >= block_free_max) {
		if( bl->type == BL_ITEM )
			ers_free(map->flooritem_ers, bl);
		else
			aFree(bl);
		bl = NULL;
		if (map->block_free_count >= block_free_max)
			ShowWarning("map_freeblock: too many free block! %d %d\n", map->block_free_count, map->block_free_lock);
	} else
		map->block_free[map->block_free_count++] = bl;

	return map->block_free_lock;
}
/*==========================================
 * Lock blocklist, (prevent map->freeblock usage)
 *------------------------------------------*/
int map_freeblock_lock (void) {
	return ++map->block_free_lock;
}

/*==========================================
 * Remove the lock on map_bl
 *------------------------------------------*/
int map_freeblock_unlock (void) {
	
	if ((--map->block_free_lock) == 0) {
		int i;
		for (i = 0; i < map->block_free_count; i++) {
			if( map->block_free[i]->type == BL_ITEM )
				ers_free(map->flooritem_ers, map->block_free[i]);
			else
				aFree(map->block_free[i]);
			map->block_free[i] = NULL;
		}
		map->block_free_count = 0;
	} else if (map->block_free_lock < 0) {
		ShowError("map_freeblock_unlock: lock count < 0 !\n");
		map->block_free_lock = 0;
	}

	return map->block_free_lock;
}

// Timer function to check if there some remaining lock and remove them if so.
// Called each 1s
int map_freeblock_timer(int tid, int64 tick, int id, intptr_t data) {
	if (map->block_free_lock > 0) {
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", map->block_free_lock);
		map->block_free_lock = 1;
		map->freeblock_unlock();
	}

	return 0;
}

/*==========================================
 * These pair of functions update the counter of how many objects
 * lie on a tile.
 *------------------------------------------*/
void map_addblcell(struct block_list *bl) {
#ifdef CELL_NOSTACK
	if( bl->m < 0 || bl->x < 0 || bl->x >= map->list[bl->m].xs
	              || bl->y < 0 || bl->y >= map->list[bl->m].ys
	              || !(bl->type&BL_CHAR) )
		return;
	map->list[bl->m].cell[bl->x+bl->y*map->list[bl->m].xs].cell_bl++;
#else
	return;
#endif
}

void map_delblcell(struct block_list *bl) {
#ifdef CELL_NOSTACK
	if( bl->m < 0 || bl->x < 0 || bl->x >= map->list[bl->m].xs
	              || bl->y < 0 || bl->y >= map->list[bl->m].ys
	              || !(bl->type&BL_CHAR) )
	map->list[bl->m].cell[bl->x+bl->y*map->list[bl->m].xs].cell_bl--;
#else
	return;
#endif
}

/*==========================================
 * Adds a block to the map.
 * Returns 0 on success, 1 on failure (illegal coordinates).
 *------------------------------------------*/
int map_addblock(struct block_list* bl)
{
	int16 m, x, y;
	int pos;

	nullpo_ret(bl);

	if (bl->prev != NULL) {
		ShowError("map_addblock: bl->prev != NULL\n");
		return 1;
	}

	m = bl->m;
	x = bl->x;
	y = bl->y;
	if( m < 0 || m >= map->count ) {
		ShowError("map_addblock: invalid map id (%d), only %d are loaded.\n", m, map->count);
		return 1;
	}
	if( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) {
		ShowError("map_addblock: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d\n", map->list[m].name, x, y, map->list[m].xs, map->list[m].ys);
		return 1;
	}

	pos = x/BLOCK_SIZE+(y/BLOCK_SIZE)*map->list[m].bxs;

	if (bl->type == BL_MOB) {
		bl->next = map->list[m].block_mob[pos];
		bl->prev = &map->bl_head;
		if (bl->next) bl->next->prev = bl;
		map->list[m].block_mob[pos] = bl;
	} else {
		bl->next = map->list[m].block[pos];
		bl->prev = &map->bl_head;
		if (bl->next) bl->next->prev = bl;
		map->list[m].block[pos] = bl;
	}

#ifdef CELL_NOSTACK
	map->addblcell(bl);
#endif

	return 0;
}

/*==========================================
 * Removes a block from the map.
 *------------------------------------------*/
int map_delblock(struct block_list* bl)
{
	int pos;
	nullpo_ret(bl);

	// blocklist (2ways chainlist)
	if (bl->prev == NULL) {
		if (bl->next != NULL) {
			// can't delete block (already at the begining of the chain)
			ShowError("map_delblock error : bl->next!=NULL\n");
		}
		return 0;
	}

#ifdef CELL_NOSTACK
	map->delblcell(bl);
#endif

	pos = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*map->list[bl->m].bxs;

	if (bl->next)
		bl->next->prev = bl->prev;
	if (bl->prev == &map->bl_head) {
		//Since the head of the list, update the block_list map of []
		if (bl->type == BL_MOB) {
			map->list[bl->m].block_mob[pos] = bl->next;
		} else {
			map->list[bl->m].block[pos] = bl->next;
		}
	} else {
		bl->prev->next = bl->next;
	}
	bl->next = NULL;
	bl->prev = NULL;

	return 0;
}

/*==========================================
 * Moves a block a x/y target position. [Skotlex]
 * Pass flag as 1 to prevent doing skill->unit_move checks
 * (which are executed by default on BL_CHAR types)
 *------------------------------------------*/
int map_moveblock(struct block_list *bl, int x1, int y1, int64 tick) {
	int x0 = bl->x, y0 = bl->y;
	struct status_change *sc = NULL;
	int moveblock = ( x0/BLOCK_SIZE != x1/BLOCK_SIZE || y0/BLOCK_SIZE != y1/BLOCK_SIZE);

	if (!bl->prev) {
		//Block not in map, just update coordinates, but do naught else.
		bl->x = x1;
		bl->y = y1;
		return 0;
	}

	//TODO: Perhaps some outs of bounds checking should be placed here?
	if (bl->type&BL_CHAR) {
		sc = status->get_sc(bl);

		skill->unit_move(bl,tick,2);
		status_change_end(bl, SC_RG_CCONFINE_M, INVALID_TIMER);
		status_change_end(bl, SC_RG_CCONFINE_S, INVALID_TIMER);
		//		status_change_end(bl, SC_BLADESTOP, INVALID_TIMER); //Won't stop when you are knocked away, go figure...
		status_change_end(bl, SC_NJ_TATAMIGAESHI, INVALID_TIMER);
		status_change_end(bl, SC_MAGICROD, INVALID_TIMER);
		if (sc && sc->data[SC_PROPERTYWALK] &&
			sc->data[SC_PROPERTYWALK]->val3 >= skill->get_maxcount(sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2) )
			status_change_end(bl,SC_PROPERTYWALK,INVALID_TIMER);
	} else if (bl->type == BL_NPC)
		npc->unsetcells((TBL_NPC*)bl);

	if (moveblock) map->delblock(bl);
#ifdef CELL_NOSTACK
	else map->delblcell(bl);
#endif
	bl->x = x1;
	bl->y = y1;
	if (moveblock) map->addblock(bl);
#ifdef CELL_NOSTACK
	else map->addblcell(bl);
#endif

	if (bl->type&BL_CHAR) {

		skill->unit_move(bl,tick,3);

		if( bl->type == BL_PC && ((TBL_PC*)bl)->shadowform_id ) {//Shadow Form Target Moving
			struct block_list *d_bl;
			if( (d_bl = map->id2bl(((TBL_PC*)bl)->shadowform_id)) == NULL || !check_distance_bl(bl,d_bl,10) ) {
				if( d_bl )
					status_change_end(d_bl,SC__SHADOWFORM,INVALID_TIMER);
				((TBL_PC*)bl)->shadowform_id = 0;
			}
		}

		if (sc && sc->count) {
			if (sc->data[SC_DANCING])
				skill->unit_move_unit_group(skill->id2group(sc->data[SC_DANCING]->val2), bl->m, x1-x0, y1-y0);
			else {
				if (sc->data[SC_CLOAKING])
					skill->check_cloaking(bl, sc->data[SC_CLOAKING]);
				if (sc->data[SC_WARM])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_WARM]->val4), bl->m, x1-x0, y1-y0);
				if (sc->data[SC_BANDING])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_BANDING]->val4), bl->m, x1-x0, y1-y0);

				if (sc->data[SC_NEUTRALBARRIER_MASTER])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_NEUTRALBARRIER_MASTER]->val2), bl->m, x1-x0, y1-y0);
				else if (sc->data[SC_STEALTHFIELD_MASTER])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_STEALTHFIELD_MASTER]->val2), bl->m, x1-x0, y1-y0);

				if( sc->data[SC__SHADOWFORM] ) {//Shadow Form Caster Moving
					struct block_list *d_bl;
					if( (d_bl = map->id2bl(sc->data[SC__SHADOWFORM]->val2)) == NULL || !check_distance_bl(bl,d_bl,10) )
						status_change_end(bl,SC__SHADOWFORM,INVALID_TIMER);
				}

				if (sc->data[SC_PROPERTYWALK]
				 && sc->data[SC_PROPERTYWALK]->val3 < skill->get_maxcount(sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2)
				 && map->find_skill_unit_oncell(bl,bl->x,bl->y,SO_ELECTRICWALK,NULL,0) == NULL
				 && map->find_skill_unit_oncell(bl,bl->x,bl->y,SO_FIREWALK,NULL,0) == NULL
				 && skill->unitsetting(bl,sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2,x0, y0,0)
				) {
					sc->data[SC_PROPERTYWALK]->val3++;
				}


			}
			/* Guild Aura Moving */
			if( bl->type == BL_PC && ((TBL_PC*)bl)->state.gmaster_flag ) {
				if (sc->data[SC_LEADERSHIP])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_LEADERSHIP]->val4), bl->m, x1-x0, y1-y0);
				if (sc->data[SC_GLORYWOUNDS])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_GLORYWOUNDS]->val4), bl->m, x1-x0, y1-y0);
				if (sc->data[SC_SOULCOLD])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_SOULCOLD]->val4), bl->m, x1-x0, y1-y0);
				if (sc->data[SC_HAWKEYES])
					skill->unit_move_unit_group(skill->id2group(sc->data[SC_HAWKEYES]->val4), bl->m, x1-x0, y1-y0);
			}
		}
	} else if (bl->type == BL_NPC)
		npc->setcells((TBL_NPC*)bl);

	return 0;
}

/*==========================================
 * Counts specified number of objects on given cell.
 * TODO: merge with bl_getall_area
 *------------------------------------------*/
int map_count_oncell(int16 m, int16 x, int16 y, int type) {
	int bx,by;
	struct block_list *bl;
	int count = 0;

	if (x < 0 || y < 0 || (x >= map->list[m].xs) || (y >= map->list[m].ys))
		return 0;

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if (type&~BL_MOB)
		for( bl = map->list[m].block[bx+by*map->list[m].bxs] ; bl != NULL ; bl = bl->next )
			if(bl->x == x && bl->y == y && bl->type&type)
				count++;

	if (type&BL_MOB)
		for( bl = map->list[m].block_mob[bx+by*map->list[m].bxs] ; bl != NULL ; bl = bl->next )
			if(bl->x == x && bl->y == y)
				count++;

	return count;
}
/*
 * Looks for a skill unit on a given cell
 * flag&1: runs battle_check_target check based on unit->group->target_flag
 */
struct skill_unit* map_find_skill_unit_oncell(struct block_list* target,int16 x,int16 y,uint16 skill_id,struct skill_unit* out_unit, int flag) {
	int16 m,bx,by;
	struct block_list *bl;
	struct skill_unit *su;
	m = target->m;

	if (x < 0 || y < 0 || (x >= map->list[m].xs) || (y >= map->list[m].ys))
		return NULL;

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	for( bl = map->list[m].block[bx+by*map->list[m].bxs] ; bl != NULL ; bl = bl->next ) {
		if (bl->x != x || bl->y != y || bl->type != BL_SKILL)
			continue;

		su = (struct skill_unit *) bl;
		if( su == out_unit || !su->alive || !su->group || su->group->skill_id != skill_id )
			continue;
		if( !(flag&1) || battle->check_target(&su->bl,target,su->group->target_flag) > 0 )
			return su;
	}
	return NULL;
}

/** @name Functions for block_list search and manipulation
 */

/* @{ */
/**
 * Applies func to every block_list in bl_list starting with bl_list[blockcount].
 * Sets bl_list_count back to blockcount.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param blockcount Index of first relevant entry in bl_list
 * @param max Maximum sum of values returned by func (usually max number of func calls)
 * @param args Extra arguments for func
 * @return Sum of the values returned by func
 */
static int bl_vforeach(int (*func)(struct block_list*, va_list), int blockcount, int max, va_list args) {
	int i;
	int returnCount = 0;

	map->freeblock_lock();
	for (i = blockcount; i < map->bl_list_count && returnCount < max; i++) {
		if (map->bl_list[i]->prev) { //func() may delete this bl_list[] slot, checking for prev ensures it wasnt queued for deletion.
			va_list argscopy;
			va_copy(argscopy, args);
			returnCount += func(map->bl_list[i], argscopy);
			va_end(argscopy);
		}
	}
	map->freeblock_unlock();

	map->bl_list_count = blockcount;

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type on map m.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param m Map id
 * @param type enum bl_type
 * @param args Extra arguments for func
 * @return Sum of the values returned by func
 */
static int map_vforeachinmap(int (*func)(struct block_list*, va_list), int16 m, int type, va_list args) {
	int i;
	int returnCount = 0;
	int bsize; 
	va_list argscopy;
	struct block_list *bl;
	int blockcount = map->bl_list_count;

	if (m < 0)
		return 0;

	bsize = map->list[m].bxs * map->list[m].bys;
	for (i = 0; i < bsize; i++) {
		if (type&~BL_MOB) {
			for (bl = map->list[m].block[i]; bl != NULL; bl = bl->next) {
				if (bl->type&type && map->bl_list_count < BL_LIST_MAX) {
					map->bl_list[map->bl_list_count++] = bl;
				}
			}
		}
		if (type&BL_MOB) {
			for (bl = map->list[m].block_mob[i]; bl != NULL; bl = bl->next) {
				if (map->bl_list_count < BL_LIST_MAX) {
					map->bl_list[map->bl_list_count++] = bl;
				}
			}
		}
	}

	if (map->bl_list_count >= BL_LIST_MAX)
		ShowError("map.c:map_vforeachinmap: bl_list size (%d) exceeded\n", BL_LIST_MAX);

	va_copy(argscopy, args);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, argscopy);
	va_end(argscopy);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type on map m.
 * Returns the sum of values returned by func.
 * @see map_vforeachinmap
 * @param func Function to be applied
 * @param m Map id
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinmap(int (*func)(struct block_list*, va_list), int16 m, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachinmap(func, m, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type on all maps
 * of instance instance_id.
 * Returns the sum of values returned by func.
 * @see map_vforeachinmap.
 * @param func Function to be applied
 * @param m Map id
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachininstance(int (*func)(struct block_list*, va_list), int16 instance_id, int type, va_list ap) {
	int i;
	int returnCount = 0;

	for (i = 0; i < instance->list[instance_id].num_map; i++) {
		int m = instance->list[instance_id].map[i];
		va_list apcopy;
		va_copy(apcopy, ap);
		returnCount += map->vforeachinmap(func, m, type, apcopy);
		va_end(apcopy);
	}

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type on all maps
 * of instance instance_id.
 * Returns the sum of values returned by func.
 * @see map_vforeachininstance.
 * @param func Function to be applied
 * @param m Map id
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachininstance(int (*func)(struct block_list*, va_list), int16 instance_id, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachininstance(func, instance_id, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Retrieves all map objects in area that are matched by the type
 * and func. Appends them at the end of global bl_list array.
 * @param type Matching enum bl_type
 * @param m Map
 * @param func Matching function
 * @param ... Extra arguments for func
 * @return Number of found objects
 */
static int bl_getall_area(int type, int m, int x0, int y0, int x1, int y1, int (*func)(struct block_list*, va_list), ...) {
	va_list args;
	int bx, by;
	struct block_list *bl;
	int found = 0;

	if (m < 0)
		return 0;

	if (x1 < x0) swap(x0, x1);
	if (y1 < y0) swap(y0, y1);

	// Limit search area to map size
	x0 = max(x0, 0);
	y0 = max(y0, 0);
	x1 = min(x1, map->list[m].xs - 1);
	y1 = min(y1, map->list[m].ys - 1);

	for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
		for (bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++) {
			if (type&~BL_MOB) {
				for (bl = map->list[m].block[bx + by * map->list[m].bxs]; bl != NULL; bl = bl->next) {
					if (map->bl_list_count < BL_LIST_MAX
						&& bl->type&type
						&& bl->x >= x0 && bl->x <= x1
						&& bl->y >= y0 && bl->y <= y1) {
							if (func) {
								va_start(args, func);
								if (func(bl, args)) {
									map->bl_list[map->bl_list_count++] = bl;
									found++;
								}
								va_end(args);
							}
							else {
								map->bl_list[map->bl_list_count++] = bl;
								found++;
							}
					}
				}
			}
			if (type&BL_MOB) { // TODO: fix this code duplication
				for (bl = map->list[m].block_mob[bx + by * map->list[m].bxs]; bl != NULL; bl = bl->next) {
					if (map->bl_list_count < BL_LIST_MAX
						//&& bl->type&type // block_mob contains BL_MOBs only
						&& bl->x >= x0 && bl->x <= x1
						&& bl->y >= y0 && bl->y <= y1) {
							if (func) {
								va_start(args, func);
								if (func(bl, args)) {
									map->bl_list[map->bl_list_count++] = bl;
									found++;
								}
								va_end(args);
							}
							else {
								map->bl_list[map->bl_list_count++] = bl;
								found++;
							}
					}
				}
			}
		}
	}

	if (map->bl_list_count >= BL_LIST_MAX)
		ShowError("map.c:bl_getall_area: bl_list size (%d) exceeded\n", BL_LIST_MAX);

	return found;
}

/**
 * Checks if bl is within range cells from center.
 * If CIRCULAR AREA is not used always returns 1, since
 * preliminary range selection is already done in bl_getall_area.
 * @return 1 if matches, 0 otherwise
 */
static int bl_vgetall_inrange(struct block_list *bl, va_list args)
{
#ifdef CIRCULAR_AREA
	struct block_list *center = va_arg(args, struct block_list*);
	int range = va_arg(args, int);
	if (!check_distance_bl(center, bl, range))
		return 0;
#endif
	return 1;
}

/**
 * Applies func to every block_list object of bl_type type within range cells from center.
 * Area is rectangular, unless CIRCULAR_AREA is defined.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachinrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	if (range < 0) range *= -1;

	bl_getall_area(type, center->m, center->x - range, center->y - range, center->x + range, center->y + range, bl_vgetall_inrange, center, range);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, apcopy);
	va_end(ap);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type within range cells from center.
 * Area is rectangular, unless CIRCULAR_AREA is defined.
 * Returns the sum of values returned by func.
 * @see map_vforeachinrange
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachinrange(func, center, range, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Applies func to some block_list objects of bl_type type within range cells from center.
 * Limit is set by count parameter.
 * Area is rectangular, unless CIRCULAR_AREA is defined.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param count Maximum sum of values returned by func (usually max number of func calls)
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforcountinrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int count, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	if (range < 0) range *= -1;

	bl_getall_area(type, center->m, center->x - range, center->y - range, center->x + range, center->y + range, bl_vgetall_inrange, center, range);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, count, apcopy);
	va_end(apcopy);

	return returnCount;
}

/**
 * Applies func to some block_list objects of bl_type type within range cells from center.
 * Limit is set by count parameter.
 * Area is rectangular, unless CIRCULAR_AREA is defined.
 * Returns the sum of values returned by func.
 * @see map_vforcountinrange
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param count Maximum sum of values returned by func (usually max number of func calls)
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_forcountinrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int count, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforcountinrange(func, center, range, count, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Checks if bl is within shooting range from center.
 * There must be a shootable path between bl and center.
 * Does not check for range if CIRCULAR AREA is not defined, since
 * preliminary range selection is already done in bl_getall_area.
 * @return 1 if matches, 0 otherwise
 */
static int bl_vgetall_inshootrange(struct block_list *bl, va_list args)
{
	struct block_list *center = va_arg(args, struct block_list*);
#ifdef CIRCULAR_AREA
	int range = va_arg(args, int);
	if (!check_distance_bl(center, bl, range))
		return 0;
#endif
	if (!path->search_long(NULL, center->m, center->x, center->y, bl->x, bl->y, CELL_CHKWALL))
		return 0;
	return 1;
}

/**
 * Applies func to every block_list object of bl_type type within shootable range from center.
 * There must be a shootable path between bl and center.
 * Area is rectangular, unless CIRCULAR_AREA is defined.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachinshootrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	if (range < 0) range *= -1;

	bl_getall_area(type, center->m, center->x - range, center->y - range, center->x + range, center->y + range, bl_vgetall_inshootrange, center, range);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
	va_end(apcopy);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type within shootable range from center.
 * There must be a shootable path between bl and center.
 * Area is rectangular, unless CIRCULAR_AREA is defined.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinshootrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachinshootrange(func, center, range, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type in
 * rectangular area (x0,y0)~(x1,y1) on map m.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param m Map id
 * @param x0 Starting X-coordinate
 * @param y0 Starting Y-coordinate
 * @param x1 Ending X-coordinate
 * @param y1 Ending Y-coordinate
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachinarea(int (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	bl_getall_area(type, m, x0, y0, x1, y1, NULL);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, apcopy);
	va_end(apcopy);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type in
 * rectangular area (x0,y0)~(x1,y1) on map m.
 * Returns the sum of values returned by func.
 * @see map_vforeachinarea
 * @param func Function to be applied
 * @param m Map id
 * @param x0 Starting X-coordinate
 * @param y0 Starting Y-coordinate
 * @param x1 Ending X-coordinate
 * @param y1 Ending Y-coordinate
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinarea(int (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachinarea(func, m, x0, y0, x1, y1, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Applies func to some block_list objects of bl_type type in
 * rectangular area (x0,y0)~(x1,y1) on map m.
 * Limit is set by @count parameter.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param m Map id
 * @param x0 Starting X-coordinate
 * @param y0 Starting Y-coordinate
 * @param x1 Ending X-coordinate
 * @param y1 Ending Y-coordinate
 * @param count Maximum sum of values returned by func (usually max number of func calls)
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforcountinarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	bl_getall_area(type, m, x0, y0, x1, y1, NULL);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, count, apcopy);
	va_end(apcopy);

	return returnCount;
}

/**
 * Applies func to some block_list objects of bl_type type in
 * rectangular area (x0,y0)~(x1,y1) on map m.
 * Limit is set by @count parameter.
 * Returns the sum of values returned by func.
 * @see map_vforcountinarea
 * @param func Function to be applied
 * @param m Map id
 * @param x0 Starting X-coordinate
 * @param y0 Starting Y-coordinate
 * @param x1 Ending X-coordinate
 * @param y1 Ending Y-coordinate
 * @param count Maximum sum of values returned by func (usually max number of func calls)
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_forcountinarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforcountinarea(func, m, x0, y0, x1, y1, count, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Checks if bl is inside area that was in range cells from the center
 * before it was moved by (dx,dy) cells, but it is not in range cells
 * from center after movement is completed.
 * In other words, checks if bl is inside area that is no longer covered
 * by center's range.
 * Preliminary range selection is already done in bl_getall_area.
 * @return 1 if matches, 0 otherwise
 */
static int bl_vgetall_inmovearea(struct block_list *bl, va_list args)
{
	int dx = va_arg(args, int);
	int dy = va_arg(args, int);
	struct block_list *center = va_arg(args, struct block_list*);
	int range = va_arg(args, int);

	if ((dx > 0 && bl->x < center->x - range + dx) ||
		(dx < 0 && bl->x > center->x + range + dx) ||
		(dy > 0 && bl->y < center->y - range + dy) ||
		(dy < 0 && bl->y > center->y + range + dy))
		return 1;
	return 0;
}

/**
 * Applies func to every block_list object of bl_type type in
 * area that was covered by range cells from center, but is no
 * longer after center is moved by (dx,dy) cells (i.e. area that
 * center has lost sight of).
 * If used after center has reached its destination and with
 * opposed movement vector (-dx,-dy), selection corresponds
 * to new area in center's view).
 * Uses rectangular area.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param dx Center's movement on X-axis
 * @param dy Center's movement on Y-axis
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachinmovearea(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	int m, x0, x1, y0, y1;
	va_list apcopy;

	if (!range) return 0;
	if (!dx && !dy) return 0; // No movement.

	if (range < 0) range *= -1;

	m = center->m;
	x0 = center->x - range;
	x1 = center->x + range;
	y0 = center->y - range;
	y1 = center->y + range;

	if (dx == 0 || dy == 0) { // Movement along one axis only.
		if (dx == 0) {
			if (dy < 0) { y0 = y1 + dy + 1; } // Moving south
			else        { y1 = y0 + dy - 1; } // North
		} else { //dy == 0
			if (dx < 0) { x0 = x1 + dx + 1; } // West
			else        { x1 = x0 + dx - 1; } // East
		}
		bl_getall_area(type, m, x0, y0, x1, y1, NULL);
	}
	else { // Diagonal movement
		bl_getall_area(type, m, x0, y0, x1, y1, bl_vgetall_inmovearea, dx, dy, center, range);
	}

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, apcopy);
	va_end(apcopy);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type in
 * area that was covered by range cells from center, but is no
 * longer after center is moved by (dx,dy) cells (i.e. area that
 * center has lost sight of).
 * If used after center has reached its destination and with
 * opposed movement vector (-dx,-dy), selection corresponds
 * to new area in center's view).
 * Uses rectangular area.
 * Returns the sum of values returned by func.
 * @see map_vforeachinmovearea
 * @param func Function to be applied
 * @param center Center of the selection area
 * @param range Range in cells from center
 * @param dx Center's movement on X-axis
 * @param dy Center's movement on Y-axis
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinmovearea(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachinmovearea(func, center, range, dx, dy, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type in
 * cell (x,y) on map m.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param m Map id
 * @param x Target cell X-coordinate
 * @param y Target cell Y-coordinate
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachincell(int (*func)(struct block_list*, va_list), int16 m, int16 x, int16 y, int type, va_list ap) {
	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	bl_getall_area(type, m, x, y, x, y, NULL);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, apcopy);
	va_end(apcopy);

	return returnCount;
}

/**
 * Applies func to every block_list object of bl_type type in
 * cell (x,y) on map m.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param m Map id
 * @param x Target cell X-coordinate
 * @param y Target cell Y-coordinate
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachincell(int (*func)(struct block_list*, va_list), int16 m, int16 x, int16 y, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachincell(func, m, x, y, type, ap);
	va_end(ap);

	return returnCount;
}

/**
 * Helper function for map_foreachinpath()
 * Checks if shortest distance from bl to path
 * between (x0,y0) and (x1,y1) is shorter than range.
 * @see map_foreachinpath
 */
static int bl_vgetall_inpath(struct block_list *bl, va_list args)
{
	int m  = va_arg(args, int);
	int x0 = va_arg(args, int);
	int y0 = va_arg(args, int);
	int x1 = va_arg(args, int);
	int y1 = va_arg(args, int);
	int range = va_arg(args, int);
	int len_limit = va_arg(args, int);
	int magnitude2 = va_arg(args, int);

	int xi = bl->x;
	int yi = bl->y;
	int xu, yu;

	int k = ( xi - x0 ) * ( x1 - x0 ) + ( yi - y0 ) * ( y1 - y0 );

	if ( k < 0 || k > len_limit ) //Since more skills use this, check for ending point as well.
		return 0;

	if ( k > magnitude2 && !path->search_long(NULL, m, x0, y0, xi, yi, CELL_CHKWALL) )
		return 0; //Targets beyond the initial ending point need the wall check.

	//All these shifts are to increase the precision of the intersection point and distance considering how it's
	//int math.
	k  = ( k << 4 ) / magnitude2; //k will be between 1~16 instead of 0~1
	xi <<= 4;
	yi <<= 4;
	xu = ( x0 << 4 ) + k * ( x1 - x0 );
	yu = ( y0 << 4 ) + k * ( y1 - y0 );

//Avoid needless calculations by not getting the sqrt right away.
#define MAGNITUDE2(x0, y0, x1, y1) ( ( ( x1 ) - ( x0 ) ) * ( ( x1 ) - ( x0 ) ) + ( ( y1 ) - ( y0 ) ) * ( ( y1 ) - ( y0 ) ) )

	k  = MAGNITUDE2(xi, yi, xu, yu);

	//If all dot coordinates were <<4 the square of the magnitude is <<8
	if ( k > range )
		return 0;

	return 1;
}

/**
 * Applies func to every block_list object of bl_type type in
 * path on a line between (x0,y0) and (x1,y1) on map m.
 * Path starts at (x0,y0) and is \a length cells long and \a range cells wide.
 * Objects beyond the initial (x1,y1) ending point are checked
 * for walls in the path.
 * Returns the sum of values returned by func.
 * @param func Function to be applied
 * @param m Map id
 * @param x Target cell X-coordinate
 * @param y Target cell Y-coordinate
 * @param type enum bl_type
 * @param ap Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_vforeachinpath(int (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, va_list ap) {
	// [Skotlex]
	// check for all targets in the square that
	// contains the initial and final positions (area range increased to match the
	// radius given), then for each object to test, calculate the distance to the
	// path and include it if the range fits and the target is in the line (0<k<1,
	// as they call it).
	// The implementation I took as reference is found at
	// http://web.archive.org/web/20050720125314/http://astronomy.swin.edu.au/~pbourke/geometry/pointline/
	// http://paulbourke.net/geometry/pointlineplane/
	// I won't use doubles/floats, but pure int math for
	// speed purposes. The range considered is always the same no matter how
	// close/far the target is because that's how SharpShooting works currently in
	// kRO

	int returnCount = 0;
	int blockcount = map->bl_list_count;
	va_list apcopy;

	//method specific variables
	int magnitude2, len_limit; //The square of the magnitude
	int k;
	int mx0 = x0, mx1 = x1, my0 = y0, my1 = y1;

	len_limit = magnitude2 = MAGNITUDE2(x0, y0, x1, y1);
	if (magnitude2 < 1) //Same begin and ending point, can't trace path.
		return 0;

	if (length) { //Adjust final position to fit in the given area.
		//TODO: Find an alternate method which does not requires a square root calculation.
		k = (int)sqrt((float)magnitude2);
		mx1 = x0 + (x1 - x0) * length / k;
		my1 = y0 + (y1 - y0) * length / k;
		len_limit = MAGNITUDE2(x0, y0, mx1, my1);
	}
	//Expand target area to cover range.
	if (mx0 > mx1) {
		mx0 += range;
		mx1 -= range;
	} else {
		mx0 -= range;
		mx1 += range;
	}
	if (my0 > my1) {
		my0 += range;
		my1 -= range;
	} else {
		my0 -= range;
		my1 += range;
	}
	range *= range << 8; //Values are shifted later on for higher precision using int math.

	bl_getall_area(type, m, mx0, my0, mx1, my1, bl_vgetall_inpath, m, x0, y0, x1, y1, range, len_limit, magnitude2);

	va_copy(apcopy, ap);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, apcopy);
	va_end(apcopy);

	return returnCount;
}
#undef MAGNITUDE2

/**
 * Applies func to every block_list object of bl_type type in
 * path on a line between (x0,y0) and (x1,y1) on map m.
 * Path starts at (x0,y0) and is \a length cells long and \a range cells wide.
 * Objects beyond the initial (x1,y1) ending point are checked
 * for walls in the path.
 * Returns the sum of values returned by func.
 * @see map_vforeachinpath
 * @param func Function to be applied
 * @param m Map id
 * @param x Target cell X-coordinate
 * @param y Target cell Y-coordinate
 * @param type enum bl_type
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinpath(int (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...) {
	int returnCount;
	va_list ap;

	va_start(ap, type);
	returnCount = map->vforeachinpath(func, m, x0, y0, x1, y1, range, length, type, ap);
	va_end(ap);

	return returnCount;
}

/** @} */

/// Generates a new flooritem object id from the interval [MIN_FLOORITEM, MAX_FLOORITEM).
/// Used for floor items, skill units and chatroom objects.
/// @return The new object id
int map_get_new_object_id(void)
{
	static int last_object_id = MIN_FLOORITEM - 1;
	int i;

	// find a free id
	i = last_object_id + 1;
	while( i != last_object_id ) {
		if( i == MAX_FLOORITEM )
			i = MIN_FLOORITEM;

		if( !idb_exists(map->id_db, i) )
			break;

		++i;
	}

	if( i == last_object_id ) {
		ShowError("map_addobject: no free object id!\n");
		return 0;
	}

	// update cursor
	last_object_id = i;

	return i;
}

/*==========================================
 * Timered function to clear the floor (remove remaining item)
 * Called each flooritem_lifetime ms
 *------------------------------------------*/
int map_clearflooritem_timer(int tid, int64 tick, int id, intptr_t data) {
	struct flooritem_data* fitem = (struct flooritem_data*)idb_get(map->id_db, id);

	if (fitem == NULL || fitem->bl.type != BL_ITEM || (fitem->cleartimer != tid)) {
		ShowError("map_clearflooritem_timer : error\n");
		return 1;
	}


	if (pet->search_petDB_index(fitem->item_data.nameid, PET_EGG) >= 0)
		intif->delete_petdata(MakeDWord(fitem->item_data.card[1], fitem->item_data.card[2]));

	clif->clearflooritem(fitem, 0);
	map->deliddb(&fitem->bl);
	map->delblock(&fitem->bl);
	map->freeblock(&fitem->bl);
	return 0;
}

/*
 * clears a single bl item out of the bazooonga.
 */
void map_clearflooritem(struct block_list *bl) {
	struct flooritem_data* fitem = (struct flooritem_data*)bl;

	if( fitem->cleartimer != INVALID_TIMER )
		timer->delete(fitem->cleartimer,map->clearflooritem_timer);

	clif->clearflooritem(fitem, 0);
	map->deliddb(&fitem->bl);
	map->delblock(&fitem->bl);
	map->freeblock(&fitem->bl);
}

/*==========================================
 * (m,x,y) locates a random available free cell around the given coordinates
 * to place an BL_ITEM object. Scan area is 9x9, returns 1 on success.
 * x and y are modified with the target cell when successful.
 *------------------------------------------*/
int map_searchrandfreecell(int16 m,int16 *x,int16 *y,int stack) {
	int free_cell,i,j;
	int free_cells[9][2];

	for(free_cell=0,i=-1;i<=1;i++){
		if(i+*y<0 || i+*y>=map->list[m].ys)
			continue;
		for(j=-1;j<=1;j++){
			if(j+*x<0 || j+*x>=map->list[m].xs)
				continue;
			if(map->getcell(m,j+*x,i+*y,CELL_CHKNOPASS) && !map->getcell(m,j+*x,i+*y,CELL_CHKICEWALL))
				continue;
			//Avoid item stacking to prevent against exploits. [Skotlex]
			if(stack && map->count_oncell(m,j+*x,i+*y, BL_ITEM) > stack)
				continue;
			free_cells[free_cell][0] = j+*x;
			free_cells[free_cell++][1] = i+*y;
		}
	}
	if(free_cell==0)
		return 0;
	free_cell = rnd()%free_cell;
	*x = free_cells[free_cell][0];
	*y = free_cells[free_cell][1];
	return 1;
}


int map_count_sub(struct block_list *bl,va_list ap) {
	return 1;
}

/*==========================================
 * Locates a random spare cell around the object given, using range as max
 * distance from that spot. Used for warping functions. Use range < 0 for
 * whole map range.
 * Returns 1 on success. when it fails and src is available, x/y are set to src's
 * src can be null as long as flag&1
 * when ~flag&1, m is not needed.
 * Flag values:
 * &1 = random cell must be around given m,x,y, not around src
 * &2 = the target should be able to walk to the target tile.
 * &4 = there shouldn't be any players around the target tile (use the no_spawn_on_player setting)
 *------------------------------------------*/
int map_search_freecell(struct block_list *src, int16 m, int16 *x,int16 *y, int16 rx, int16 ry, int flag)
{
	int tries, spawn=0;
	int bx, by;
	int rx2 = 2*rx+1;
	int ry2 = 2*ry+1;

	if( !src && (!(flag&1) || flag&2) )
	{
		ShowDebug("map_search_freecell: Incorrect usage! When src is NULL, flag has to be &1 and can't have &2\n");
		return 0;
	}

	if (flag&1) {
		bx = *x;
		by = *y;
	} else {
		bx = src->x;
		by = src->y;
		m = src->m;
	}
	if (!rx && !ry) {
		//No range? Return the target cell then....
		*x = bx;
		*y = by;
		return map->getcell(m,*x,*y,CELL_CHKREACH);
	}

	if (rx >= 0 && ry >= 0) {
		tries = rx2*ry2;
		if (tries > 100) tries = 100;
	} else {
		tries = map->list[m].xs*map->list[m].ys;
		if (tries > 500) tries = 500;
	}

	while(tries--) {
		*x = (rx >= 0)?(rnd()%rx2-rx+bx):(rnd()%(map->list[m].xs-2)+1);
		*y = (ry >= 0)?(rnd()%ry2-ry+by):(rnd()%(map->list[m].ys-2)+1);

		if (*x == bx && *y == by)
			continue; //Avoid picking the same target tile.

		if (map->getcell(m,*x,*y,CELL_CHKREACH)) {
			if(flag&2 && !unit->can_reach_pos(src, *x, *y, 1))
				continue;
			if(flag&4) {
				if (spawn >= 100) return 0; //Limit of retries reached.
				if (spawn++ < battle_config.no_spawn_on_player
				 && map->foreachinarea(map->count_sub, m, *x-AREA_SIZE, *y-AREA_SIZE,
				                                         *x+AREA_SIZE, *y+AREA_SIZE, BL_PC)
				)
					continue;
			}
			return 1;
		}
	}
	*x = bx;
	*y = by;
	return 0;
}

/*==========================================
 * Add an item to location (m,x,y)
 * Parameters
 * @item_data item attributes
 * @amount quantity
 * @m, @x, @y mapid,x,y
 * @first_charid, @second_charid, @third_charid, looting priority
 * @flag: &1 MVP item. &2 do stacking check.
 *------------------------------------------*/
int map_addflooritem(struct item *item_data,int amount,int16 m,int16 x,int16 y,int first_charid,int second_charid,int third_charid,int flags)
{
	int r;
	struct flooritem_data *fitem=NULL;

	nullpo_ret(item_data);

	if(!map->searchrandfreecell(m,&x,&y,flags&2?1:0))
		return 0;
	r=rnd();

	fitem = ers_alloc(map->flooritem_ers, struct flooritem_data);
	
	fitem->bl.type = BL_ITEM;
	fitem->bl.prev = fitem->bl.next = NULL;
	fitem->bl.m = m;
	fitem->bl.x = x;
	fitem->bl.y = y;
	fitem->bl.id = map->get_new_object_id();
	if(fitem->bl.id==0){
		ers_free(map->flooritem_ers, fitem);
		return 0;
	}

	fitem->first_get_charid = first_charid;
	fitem->first_get_tick = timer->gettick() + (flags&1 ? battle_config.mvp_item_first_get_time : battle_config.item_first_get_time);
	fitem->second_get_charid = second_charid;
	fitem->second_get_tick = fitem->first_get_tick + (flags&1 ? battle_config.mvp_item_second_get_time : battle_config.item_second_get_time);
	fitem->third_get_charid = third_charid;
	fitem->third_get_tick = fitem->second_get_tick + (flags&1 ? battle_config.mvp_item_third_get_time : battle_config.item_third_get_time);

	memcpy(&fitem->item_data,item_data,sizeof(*item_data));
	fitem->item_data.amount=amount;
	fitem->subx=(r&3)*3+3;
	fitem->suby=((r>>2)&3)*3+3;
	fitem->cleartimer=timer->add(timer->gettick()+battle_config.flooritem_lifetime,map->clearflooritem_timer,fitem->bl.id,0);

	map->addiddb(&fitem->bl);
	map->addblock(&fitem->bl);
	clif->dropflooritem(fitem);

	return fitem->bl.id;
}

/**
 * @see DBCreateData
 */
DBData create_charid2nick(DBKey key, va_list args)
{
	struct charid2nick *p;
	CREATE(p, struct charid2nick, 1);
	return DB->ptr2data(p);
}

/// Adds(or replaces) the nick of charid to nick_db and fullfils pending requests.
/// Does nothing if the character is online.
void map_addnickdb(int charid, const char* nick)
{
	struct charid2nick* p;
	struct charid_request* req;
	struct map_session_data* sd;

	if( map->charid2sd(charid) )
		return;// already online

	p = idb_ensure(map->nick_db, charid, map->create_charid2nick);
	safestrncpy(p->nick, nick, sizeof(p->nick));

	while( p->requests ) {
		req = p->requests;
		p->requests = req->next;
		sd = map->charid2sd(req->charid);
		if( sd )
			clif->solved_charname(sd->fd, charid, p->nick);
		aFree(req);
	}
}

/// Removes the nick of charid from nick_db.
/// Sends name to all pending requests on charid.
void map_delnickdb(int charid, const char* name)
{
	struct charid2nick* p;
	struct charid_request* req;
	struct map_session_data* sd;
	DBData data;

	if (!map->nick_db->remove(map->nick_db, DB->i2key(charid), &data) || (p = DB->data2ptr(&data)) == NULL)
		return;

	while( p->requests ) {
		req = p->requests;
		p->requests = req->next;
		sd = map->charid2sd(req->charid);
		if( sd )
			clif->solved_charname(sd->fd, charid, name);
		aFree(req);
	}
	aFree(p);
}

/// Notifies sd of the nick of charid.
/// Uses the name in the character if online.
/// Uses the name in nick_db if offline.
void map_reqnickdb(struct map_session_data * sd, int charid)
{
	struct charid2nick* p;
	struct charid_request* req;
	struct map_session_data* tsd;

	nullpo_retv(sd);

	tsd = map->charid2sd(charid);
	if( tsd ) {
		clif->solved_charname(sd->fd, charid, tsd->status.name);
		return;
	}

	p = idb_ensure(map->nick_db, charid, map->create_charid2nick);
	if( *p->nick ) {
		clif->solved_charname(sd->fd, charid, p->nick);
		return;
	}
	// not in cache, request it
	CREATE(req, struct charid_request, 1);
	req->next = p->requests;
	p->requests = req;
	chrif->searchcharid(charid);
}

/*==========================================
 * add bl to id_db
 *------------------------------------------*/
void map_addiddb(struct block_list *bl)
{
	nullpo_retv(bl);

	if( bl->type == BL_PC )
	{
		TBL_PC* sd = (TBL_PC*)bl;
		idb_put(map->pc_db,sd->bl.id,sd);
		idb_put(map->charid_db,sd->status.char_id,sd);
	}
	else if( bl->type == BL_MOB )
	{
		TBL_MOB* md = (TBL_MOB*)bl;
		idb_put(map->mobid_db,bl->id,bl);

		if( md->state.boss )
			idb_put(map->bossid_db, bl->id, bl);
	}

	if( bl->type & BL_REGEN )
		idb_put(map->regen_db, bl->id, bl);

	idb_put(map->id_db,bl->id,bl);
}

/*==========================================
 * remove bl from id_db
 *------------------------------------------*/
void map_deliddb(struct block_list *bl)
{
	nullpo_retv(bl);

	if( bl->type == BL_PC )
	{
		TBL_PC* sd = (TBL_PC*)bl;
		idb_remove(map->pc_db,sd->bl.id);
		idb_remove(map->charid_db,sd->status.char_id);
	}
	else if( bl->type == BL_MOB )
	{
		idb_remove(map->mobid_db,bl->id);
		idb_remove(map->bossid_db,bl->id);
	}

	if( bl->type & BL_REGEN )
		idb_remove(map->regen_db,bl->id);

	idb_remove(map->id_db,bl->id);
}

/*==========================================
 * Standard call when a player connection is closed.
 *------------------------------------------*/
int map_quit(struct map_session_data *sd) {
	int i;

	if(!sd->state.active) { //Removing a player that is not active.
		struct auth_node *node = chrif->search(sd->status.account_id);
		if (node && node->char_id == sd->status.char_id &&
			node->state != ST_LOGOUT)
			//Except when logging out, clear the auth-connect data immediately.
			chrif->auth_delete(node->account_id, node->char_id, node->state);
		//Non-active players should not have loaded any data yet (or it was cleared already) so no additional cleanups are needed.
		return 0;
	}
	
	if( sd->expiration_tid != INVALID_TIMER )
		timer->delete(sd->expiration_tid,pc->expiration_timer);

	if (sd->npc_timer_id != INVALID_TIMER) //Cancel the event timer.
		npc->timerevent_quit(sd);

	if (sd->npc_id)
		npc->event_dequeue(sd);

	if( sd->bg_id && !sd->bg_queue.arena ) /* TODO: dump this chunk after bg_queue is fully enabled */
		bg->team_leave(sd,1);

	skill->cooldown_save(sd);
	pc->itemcd_do(sd,false);

	for( i = 0; i < sd->queues_count; i++ ) {
		struct hQueue *queue;
		if( (queue = script->queue(sd->queues[i])) && queue->onLogOut[0] != '\0' ) {
			npc->event(sd, queue->onLogOut, 0);
		}
	}
	/* two times, the npc event above may assign a new one or delete others */
	for( i = 0; i < sd->queues_count; i++ ) {
		if( sd->queues[i] != -1 )
			script->queue_remove(sd->queues[i],sd->status.account_id);
	}

	npc->script_event(sd, NPCE_LOGOUT);

	//Unit_free handles clearing the player related data,
	//map->quit handles extra specific data which is related to quitting normally
	//(changing map-servers invokes unit_free but bypasses map->quit)
	if( sd->sc.count ) {
		//Status that are not saved...
		for(i=0; i < SC_MAX; i++){
			if ( status->get_sc_type(i)&SC_NO_SAVE ) {
				if ( !sd->sc.data[i] )
					continue;
				switch( i ){
					case SC_ENDURE:
					case SC_GDSKILL_REGENERATION:
						if( !sd->sc.data[i]->val4 )
							break;
					default:
						status_change_end(&sd->bl, (sc_type)i, INVALID_TIMER);
				}
			}
		}
	}

	for( i = 0; i < EQI_MAX; i++ ) {
		if( sd->equip_index[ i ] >= 0 )
			if( !pc->isequip( sd , sd->equip_index[ i ] ) )
				pc->unequipitem( sd , sd->equip_index[ i ] , 2 );
	}

	// Return loot to owner
	if( sd->pd ) pet->lootitem_drop(sd->pd, sd);

	if( sd->state.storage_flag == 1 ) sd->state.storage_flag = 0; // No need to Double Save Storage on Quit.

	if( sd->ed ) {
		elemental->clean_effect(sd->ed);
		unit->remove_map(&sd->ed->bl,CLR_TELEPORT,ALC_MARK);
	}

	if( hChSys.local && map->list[sd->bl.m].channel && idb_exists(map->list[sd->bl.m].channel->users, sd->status.char_id) ) {
		clif->chsys_left(map->list[sd->bl.m].channel,sd);
	}

	clif->chsys_quit(sd);

	unit->remove_map_pc(sd,CLR_RESPAWN);

	if( map->list[sd->bl.m].instance_id >= 0 ) { // Avoid map conflicts and warnings on next login
		int16 m;
		struct point *pt;
		if( map->list[sd->bl.m].save.map )
			pt = &map->list[sd->bl.m].save;
		else
			pt = &sd->status.save_point;

		if( (m=map->mapindex2mapid(pt->map)) >= 0 ) {
			sd->bl.m = m;
			sd->bl.x = pt->x;
			sd->bl.y = pt->y;
			sd->mapindex = pt->map;
		}
	}

	if( sd->state.vending ) {
		idb_remove(vending->db, sd->status.char_id);
	}
	
	party->booking_delete(sd); // Party Booking [Spiria]
	pc->makesavestatus(sd);
	pc->clean_skilltree(sd);
	chrif->save(sd,1);
	unit->free_pc(sd);
	return 0;
}

/*==========================================
 * Lookup, id to session (player,mob,npc,homon,merc..)
 *------------------------------------------*/
struct map_session_data *map_id2sd(int id) {
	if (id <= 0) return NULL;
	return (struct map_session_data*)idb_get(map->pc_db,id);
}

struct mob_data *map_id2md(int id) {
	if (id <= 0) return NULL;
	return (struct mob_data*)idb_get(map->mobid_db,id);
}

struct npc_data *map_id2nd(int id) {
	// just a id2bl lookup because there's no npc_db
	struct block_list* bl = map->id2bl(id);

	return BL_CAST(BL_NPC, bl);
}

struct homun_data *map_id2hd(int id) {
	struct block_list* bl = map->id2bl(id);

	return BL_CAST(BL_HOM, bl);
}

struct mercenary_data *map_id2mc(int id) {
	struct block_list* bl = map->id2bl(id);

	return BL_CAST(BL_MER, bl);
}

struct chat_data *map_id2cd(int id) {
	struct block_list* bl = map->id2bl(id);

	return BL_CAST(BL_CHAT, bl);
}

/// Returns the nick of the target charid or NULL if unknown (requests the nick to the char server).
const char *map_charid2nick(int charid) {
	struct charid2nick *p;
	struct map_session_data* sd;

	sd = map->charid2sd(charid);
	if( sd )
		return sd->status.name;// character is online, return it's name

	p = idb_ensure(map->nick_db, charid, map->create_charid2nick);
	if( *p->nick )
		return p->nick;// name in nick_db

	chrif->searchcharid(charid);// request the name
	return NULL;
}

/// Returns the struct map_session_data of the charid or NULL if the char is not online.
struct map_session_data* map_charid2sd(int charid)
{
	return (struct map_session_data*)idb_get(map->charid_db, charid);
}

/*==========================================
 * Search session data from a nick name
 * (without sensitive case if necessary)
 * return map_session_data pointer or NULL
 *------------------------------------------*/
struct map_session_data * map_nick2sd(const char *nick)
{
	struct map_session_data* sd;
	struct map_session_data* found_sd;
	struct s_mapiterator* iter;
	size_t nicklen;
	int qty = 0;

	if( nick == NULL )
		return NULL;

	nicklen = strlen(nick);
	iter = mapit_getallusers();

	found_sd = NULL;
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
	{
		if( battle_config.partial_name_scan )
		{// partial name search
			if( strnicmp(sd->status.name, nick, nicklen) == 0 )
			{
				found_sd = sd;

				if( strcmp(sd->status.name, nick) == 0 )
				{// Perfect Match
					qty = 1;
					break;
				}

				qty++;
			}
		}
		else if( strcasecmp(sd->status.name, nick) == 0 )
		{// exact search only
			found_sd = sd;
			break;
		}
	}
	mapit->free(iter);

	if( battle_config.partial_name_scan && qty != 1 )
		found_sd = NULL;

	return found_sd;
}

/*==========================================
 * Looksup id_db DBMap and returns BL pointer of 'id' or NULL if not found
 *------------------------------------------*/
struct block_list * map_id2bl(int id) {
	return (struct block_list*)idb_get(map->id_db,id);
}

/**
 * Same as map->id2bl except it only checks for its existence
 **/
bool map_blid_exists( int id ) {
	return (idb_exists(map->id_db,id));
}

/*==========================================
 * Convext Mirror
 *------------------------------------------*/
struct mob_data * map_getmob_boss(int16 m)
{
	DBIterator* iter;
	struct mob_data *md = NULL;
	bool found = false;

	iter = db_iterator(map->bossid_db);
	for( md = (struct mob_data*)dbi_first(iter); dbi_exists(iter); md = (struct mob_data*)dbi_next(iter) )
	{
		if( md->bl.m == m )
		{
			found = true;
			break;
		}
	}
	dbi_destroy(iter);

	return (found)? md : NULL;
}

struct mob_data * map_id2boss(int id)
{
	if (id <= 0) return NULL;
	return (struct mob_data*)idb_get(map->bossid_db,id);
}

/// Applies func to all the players in the db.
/// Stops iterating if func returns -1.
void map_vforeachpc(int (*func)(struct map_session_data* sd, va_list args), va_list args) {
	DBIterator* iter;
	struct map_session_data* sd;

	iter = db_iterator(map->pc_db);
	for( sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter) )
	{
		va_list argscopy;
		int ret;

		va_copy(argscopy, args);
		ret = func(sd, argscopy);
		va_end(argscopy);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to all the players in the db.
/// Stops iterating if func returns -1.
/// @see map_vforeachpc
void map_foreachpc(int (*func)(struct map_session_data* sd, va_list args), ...) {
	va_list args;

	va_start(args, func);
	map->vforeachpc(func, args);
	va_end(args);
}

/// Applies func to all the mobs in the db.
/// Stops iterating if func returns -1.
void map_vforeachmob(int (*func)(struct mob_data* md, va_list args), va_list args) {
	DBIterator* iter;
	struct mob_data* md;

	iter = db_iterator(map->mobid_db);
	for( md = (struct mob_data*)dbi_first(iter); dbi_exists(iter); md = (struct mob_data*)dbi_next(iter) ) {
		va_list argscopy;
		int ret;

		va_copy(argscopy, args);
		ret = func(md, argscopy);
		va_end(argscopy);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to all the mobs in the db.
/// Stops iterating if func returns -1.
/// @see map_vforeachmob
void map_foreachmob(int (*func)(struct mob_data* md, va_list args), ...) {
	va_list args;

	va_start(args, func);
	map->vforeachmob(func, args);
	va_end(args);
}

/// Applies func to all the npcs in the db.
/// Stops iterating if func returns -1.
void map_vforeachnpc(int (*func)(struct npc_data* nd, va_list args), va_list args) {
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(map->id_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) ) {
		if( bl->type == BL_NPC ) {
			struct npc_data* nd = (struct npc_data*)bl;
			va_list argscopy;
			int ret;

			va_copy(argscopy, args);
			ret = func(nd, argscopy);
			va_end(argscopy);
			if( ret == -1 )
				break;// stop iterating
		}
	}
	dbi_destroy(iter);
}

/// Applies func to all the npcs in the db.
/// Stops iterating if func returns -1.
/// @see map_vforeachnpc
void map_foreachnpc(int (*func)(struct npc_data* nd, va_list args), ...) {
	va_list args;

	va_start(args, func);
	map->vforeachnpc(func, args);
	va_end(args);
}

/// Applies func to everything in the db.
/// Stops iteratin gif func returns -1.
void map_vforeachregen(int (*func)(struct block_list* bl, va_list args), va_list args) {
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(map->regen_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) ) {
		va_list argscopy;
		int ret;

		va_copy(argscopy, args);
		ret = func(bl, argscopy);
		va_end(argscopy);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to everything in the db.
/// Stops iteratin gif func returns -1.
/// @see map_vforeachregen
void map_foreachregen(int (*func)(struct block_list* bl, va_list args), ...) {
	va_list args;

	va_start(args, func);
	map->vforeachregen(func, args);
	va_end(args);
}

/// Applies func to everything in the db.
/// Stops iterating if func returns -1.
void map_vforeachiddb(int (*func)(struct block_list* bl, va_list args), va_list args) {
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(map->id_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) ) {
		va_list argscopy;
		int ret;

		va_copy(argscopy, args);
		ret = func(bl, argscopy);
		va_end(argscopy);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to everything in the db.
/// Stops iterating if func returns -1.
/// @see map_vforeachiddb
void map_foreachiddb(int (*func)(struct block_list* bl, va_list args), ...) {
	va_list args;

	va_start(args, func);
	map->vforeachiddb(func, args);
	va_end(args);
}

/// Iterator.
/// Can filter by bl type.
struct s_mapiterator
{
	enum e_mapitflags flags;// flags for special behaviour
	enum bl_type types;// what bl types to return
	DBIterator* dbi;// database iterator
};

/// Returns true if the block_list matches the description in the iterator.
///
/// @param _mapit_ Iterator
/// @param _bl_ block_list
/// @return true if it matches
#define MAPIT_MATCHES(_mapit_,_bl_) \
	( (_bl_)->type & (_mapit_)->types /* type matches */ )

/// Allocates a new iterator.
/// Returns the new iterator.
/// types can represent several BL's as a bit field.
/// TODO should this be expanded to allow filtering of map/guild/party/chat/cell/area/...?
///
/// @param flags Flags of the iterator
/// @param type Target types
/// @return Iterator
struct s_mapiterator* mapit_alloc(enum e_mapitflags flags, enum bl_type types) {
	struct s_mapiterator* iter;

	iter = ers_alloc(map->iterator_ers, struct s_mapiterator);
	iter->flags = flags;
	iter->types = types;
	if( types == BL_PC )       iter->dbi = db_iterator(map->pc_db);
	else if( types == BL_MOB ) iter->dbi = db_iterator(map->mobid_db);
	else                       iter->dbi = db_iterator(map->id_db);
	return iter;
}

/// Frees the iterator.
///
/// @param iter Iterator
void mapit_free(struct s_mapiterator* iter) {
	nullpo_retv(iter);

	dbi_destroy(iter->dbi);
	ers_free(map->iterator_ers, iter);
}

/// Returns the first block_list that matches the description.
/// Returns NULL if not found.
///
/// @param iter Iterator
/// @return first block_list or NULL
struct block_list* mapit_first(struct s_mapiterator* iter) {
	struct block_list* bl;

	nullpo_retr(NULL,iter);

	for( bl = (struct block_list*)dbi_first(iter->dbi); bl != NULL; bl = (struct block_list*)dbi_next(iter->dbi) ) {
		if( MAPIT_MATCHES(iter,bl) )
			break;// found match
	}
	return bl;
}

/// Returns the last block_list that matches the description.
/// Returns NULL if not found.
///
/// @param iter Iterator
/// @return last block_list or NULL
struct block_list* mapit_last(struct s_mapiterator* iter) {
	struct block_list* bl;

	nullpo_retr(NULL,iter);

	for( bl = (struct block_list*)dbi_last(iter->dbi); bl != NULL; bl = (struct block_list*)dbi_prev(iter->dbi) ) {
		if( MAPIT_MATCHES(iter,bl) )
			break;// found match
	}
	return bl;
}

/// Returns the next block_list that matches the description.
/// Returns NULL if not found.
///
/// @param iter Iterator
/// @return next block_list or NULL
struct block_list* mapit_next(struct s_mapiterator* iter) {
	struct block_list* bl;

	nullpo_retr(NULL,iter);

	for( ; ; ) {
		bl = (struct block_list*)dbi_next(iter->dbi);
		if( bl == NULL )
			break;// end
		if( MAPIT_MATCHES(iter,bl) )
			break;// found a match
		// try next
	}
	return bl;
}

/// Returns the previous block_list that matches the description.
/// Returns NULL if not found.
///
/// @param iter Iterator
/// @return previous block_list or NULL
struct block_list* mapit_prev(struct s_mapiterator* iter) {
	struct block_list* bl;

	nullpo_retr(NULL,iter);

	for( ; ; ) {
		bl = (struct block_list*)dbi_prev(iter->dbi);
		if( bl == NULL )
			break;// end
		if( MAPIT_MATCHES(iter,bl) )
			break;// found a match
		// try prev
	}
	return bl;
}

/// Returns true if the current block_list exists in the database.
///
/// @param iter Iterator
/// @return true if it exists
bool mapit_exists(struct s_mapiterator* iter) {
	nullpo_retr(false,iter);

	return dbi_exists(iter->dbi);
}

/*==========================================
 * Add npc-bl to id_db, basically register npc to map
 *------------------------------------------*/
bool map_addnpc(int16 m,struct npc_data *nd) {
	nullpo_ret(nd);

	if( m < 0 || m >= map->count )
		return false;

	if( map->list[m].npc_num == MAX_NPC_PER_MAP ) {
		ShowWarning("too many NPCs in one map %s\n",map->list[m].name);
		return false;
	}

	map->list[m].npc[map->list[m].npc_num]=nd;
	map->list[m].npc_num++;
	idb_put(map->id_db,nd->bl.id,nd);
	return true;
}

/*=========================================
 * Dynamic Mobs [Wizputer]
 *-----------------------------------------*/
// Stores the spawn data entry in the mob list.
// Returns the index of successful, or -1 if the list was full.
int map_addmobtolist(unsigned short m, struct spawn_data *spawn) {
	size_t i;
	ARR_FIND( 0, MAX_MOB_LIST_PER_MAP, i, map->list[m].moblist[i] == NULL );
	if( i < MAX_MOB_LIST_PER_MAP ) {
		map->list[m].moblist[i] = spawn;
		return i;
	}
	return -1;
}

void map_spawnmobs(int16 m) {
	int i, k=0;
	if (map->list[m].mob_delete_timer != INVALID_TIMER) {
		//Mobs have not been removed yet [Skotlex]
		timer->delete(map->list[m].mob_delete_timer, map->removemobs_timer);
		map->list[m].mob_delete_timer = INVALID_TIMER;
		return;
	}
	for(i=0; i<MAX_MOB_LIST_PER_MAP; i++)
		if(map->list[m].moblist[i]!=NULL) {
			k+=map->list[m].moblist[i]->num;
			npc->parse_mob2(map->list[m].moblist[i]);
		}

	if (battle_config.etc_log && k > 0) {
		ShowStatus("Map %s: Spawned '"CL_WHITE"%d"CL_RESET"' mobs.\n",map->list[m].name, k);
	}
}

int map_removemobs_sub(struct block_list *bl, va_list ap)
{
	struct mob_data *md = (struct mob_data *)bl;
	nullpo_ret(md);

	//When not to remove mob:
	// doesn't respawn and is not a slave
	if( !md->spawn && !md->master_id )
		return 0;
	// respawn data is not in cache
	if( md->spawn && !md->spawn->state.dynamic )
		return 0;
	// hasn't spawned yet
	if( md->spawn_timer != INVALID_TIMER )
		return 0;
	// is damaged and mob_remove_damaged is off
	if( !battle_config.mob_remove_damaged && md->status.hp < md->status.max_hp )
		return 0;
	// is a mvp
	if( md->db->mexp > 0 )
		return 0;

	unit->free(&md->bl,CLR_OUTSIGHT);

	return 1;
}

int map_removemobs_timer(int tid, int64 tick, int id, intptr_t data) {
	int count;
	const int16 m = id;

	if (m < 0 || m >= map->count) { //Incorrect map id!
		ShowError("map_removemobs_timer error: timer %d points to invalid map %d\n",tid, m);
		return 0;
	}
	if (map->list[m].mob_delete_timer != tid) { //Incorrect timer call!
		ShowError("map_removemobs_timer mismatch: %d != %d (map %s)\n",map->list[m].mob_delete_timer, tid, map->list[m].name);
		return 0;
	}
	map->list[m].mob_delete_timer = INVALID_TIMER;
	if (map->list[m].users > 0) //Map not empty!
		return 1;

	count = map->foreachinmap(map->removemobs_sub, m, BL_MOB);

	if (battle_config.etc_log && count > 0)
		ShowStatus("Map %s: Removed '"CL_WHITE"%d"CL_RESET"' mobs.\n",map->list[m].name, count);

	return 1;
}

void map_removemobs(int16 m) {
	if (map->list[m].mob_delete_timer != INVALID_TIMER) // should never happen
		return; //Mobs are already scheduled for removal

	map->list[m].mob_delete_timer = timer->add(timer->gettick()+battle_config.mob_remove_delay, map->removemobs_timer, m, 0);
}

/*==========================================
 * Hookup, get map_id from map_name
 *------------------------------------------*/
int16 map_mapname2mapid(const char* name) {
	unsigned short map_index;
	map_index = mapindex_name2id(name);
	if (!map_index)
		return -1;
	return map->mapindex2mapid(map_index);
}

/*==========================================
 * Returns the map of the given mapindex. [Skotlex]
 *------------------------------------------*/
int16 map_mapindex2mapid(unsigned short mapindex) {

	if (!mapindex || mapindex > MAX_MAPINDEX)
		return -1;

	return map->index2mapid[mapindex];
}

/*==========================================
 * Switching Ip, port ? (like changing map_server) get ip/port from map_name
 *------------------------------------------*/
int map_mapname2ipport(unsigned short name, uint32* ip, uint16* port) {
	struct map_data_other_server *mdos;

	mdos = (struct map_data_other_server*)uidb_get(map->map_db,(unsigned int)name);
	if(mdos==NULL || mdos->cell) //If gat isn't null, this is a local map.
		return -1;
	*ip=mdos->ip;
	*port=mdos->port;
	return 0;
}

/*==========================================
* Checks if both dirs point in the same direction.
*------------------------------------------*/
int map_check_dir(int s_dir,int t_dir)
{
	if(s_dir == t_dir)
		return 0;
	switch(s_dir) {
		case 0: if(t_dir == 7 || t_dir == 1 || t_dir == 0) return 0; break;
		case 1: if(t_dir == 0 || t_dir == 2 || t_dir == 1) return 0; break;
		case 2: if(t_dir == 1 || t_dir == 3 || t_dir == 2) return 0; break;
		case 3: if(t_dir == 2 || t_dir == 4 || t_dir == 3) return 0; break;
		case 4: if(t_dir == 3 || t_dir == 5 || t_dir == 4) return 0; break;
		case 5: if(t_dir == 4 || t_dir == 6 || t_dir == 5) return 0; break;
		case 6: if(t_dir == 5 || t_dir == 7 || t_dir == 6) return 0; break;
		case 7: if(t_dir == 6 || t_dir == 0 || t_dir == 7) return 0; break;
	}
	return 1;
}

/*==========================================
 * Returns the direction of the given cell, relative to 'src'
 *------------------------------------------*/
uint8 map_calc_dir(struct block_list* src, int16 x, int16 y)
{
	uint8 dir = 0;
	int dx, dy;

	nullpo_ret(src);

	dx = x-src->x;
	dy = y-src->y;
	if( dx == 0 && dy == 0 )
	{	// both are standing on the same spot
		//dir = 6; // aegis-style, makes knockback default to the left
		dir = unit->getdir(src); // athena-style, makes knockback default to behind 'src'
	}
	else if( dx >= 0 && dy >=0 )
	{	// upper-right
		if( dx*2 <= dy )      dir = 0;	// up
		else if( dx > dy*2 )  dir = 6;	// right
		else                  dir = 7;	// up-right
	}
	else if( dx >= 0 && dy <= 0 )
	{	// lower-right
		if( dx*2 <= -dy )     dir = 4;	// down
		else if( dx > -dy*2 ) dir = 6;	// right
		else                  dir = 5;	// down-right
	}
	else if( dx <= 0 && dy <= 0 )
	{	// lower-left
		if( dx*2 >= dy )      dir = 4;	// down
		else if( dx < dy*2 )  dir = 2;	// left
		else                  dir = 3;	// down-left
	}
	else
	{	// upper-left
		if( -dx*2 <= dy )     dir = 0;	// up
		else if( -dx > dy*2 ) dir = 2;	// left
		else                  dir = 1;	// up-left

	}
	return dir;
}

/*==========================================
 * Randomizes target cell x,y to a random walkable cell that
 * has the same distance from object as given coordinates do. [Skotlex]
 *------------------------------------------*/
int map_random_dir(struct block_list *bl, int16 *x, int16 *y)
{
	short xi = *x-bl->x;
	short yi = *y-bl->y;
	short i=0, j;
	int dist2 = xi*xi + yi*yi;
	short dist = (short)sqrt((float)dist2);
	short segment;

	if (dist < 1) dist =1;

	do {
		j = 1 + 2*(rnd()%4); //Pick a random diagonal direction
		segment = 1+(rnd()%dist); //Pick a random interval from the whole vector in that direction
		xi = bl->x + segment*dirx[j];
		segment = (short)sqrt((float)(dist2 - segment*segment)); //The complement of the previously picked segment
		yi = bl->y + segment*diry[j];
	} while ( (map->getcell(bl->m,xi,yi,CELL_CHKNOPASS) || !path->search(NULL,bl->m,bl->x,bl->y,xi,yi,1,CELL_CHKNOREACH))
	       && (++i)<100 );

	if (i < 100) {
		*x = xi;
		*y = yi;
		return 1;
	}
	return 0;
}

// gat system
inline static struct mapcell map_gat2cell(int gat) {
	struct mapcell cell;

	memset(&cell,0,sizeof(struct mapcell));

	switch( gat ) {
		case 0: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // walkable ground
		case 1: cell.walkable = 0; cell.shootable = 0; cell.water = 0; break; // non-walkable ground
		case 2: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // ???
		case 3: cell.walkable = 1; cell.shootable = 1; cell.water = 1; break; // walkable water
		case 4: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // ???
		case 5: cell.walkable = 0; cell.shootable = 1; cell.water = 0; break; // gap (snipable)
		case 6: cell.walkable = 1; cell.shootable = 1; cell.water = 0; break; // ???
	default:
		ShowWarning("map_gat2cell: unrecognized gat type '%d'\n", gat);
		break;
	}

	return cell;
}

int map_cell2gat(struct mapcell cell) {
	if( cell.walkable == 1 && cell.shootable == 1 && cell.water == 0 ) return 0;
	if( cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 ) return 1;
	if( cell.walkable == 1 && cell.shootable == 1 && cell.water == 1 ) return 3;
	if( cell.walkable == 0 && cell.shootable == 1 && cell.water == 0 ) return 5;

	ShowWarning("map_cell2gat: cell has no matching gat type\n");
	return 1; // default to 'wall'
}
void map_cellfromcache(struct map_data *m) {
	char decode_buffer[MAX_MAP_SIZE];
	struct map_cache_map_info *info = NULL;

	if( (info = (struct map_cache_map_info *)m->cellPos) ) {
		unsigned long size, xy;
		int i;

		size = (unsigned long)info->xs*(unsigned long)info->ys;

		// TO-DO: Maybe handle the scenario, if the decoded buffer isn't the same size as expected? [Shinryo]
		decode_zip(decode_buffer, &size, m->cellPos+sizeof(struct map_cache_map_info), info->len);
		CREATE(m->cell, struct mapcell, size);

		for( xy = 0; xy < size; ++xy )
			m->cell[xy] = map->gat2cell(decode_buffer[xy]);

		m->getcellp = map->getcellp;
		m->setcell  = map->setcell;

		for(i = 0; i < m->npc_num; i++) {
			npc->setcells(m->npc[i]);
		}
	}
}

/*==========================================
 * Confirm if celltype in (m,x,y) match the one given in cellchk
 *------------------------------------------*/
int map_getcell(int16 m,int16 x,int16 y,cell_chk cellchk) {
	return (m < 0 || m >= map->count) ? 0 : map->list[m].getcellp(&map->list[m],x,y,cellchk);
}

int map_getcellp(struct map_data* m,int16 x,int16 y,cell_chk cellchk) {
	struct mapcell cell;

	nullpo_ret(m);

	//NOTE: this intentionally overrides the last row and column
	if(x<0 || x>=m->xs-1 || y<0 || y>=m->ys-1)
		return( cellchk == CELL_CHKNOPASS );

	cell = m->cell[x + y*m->xs];

	switch(cellchk) {
		// gat type retrieval
	case CELL_GETTYPE:
		return map->cell2gat(cell);

		// base gat type checks
	case CELL_CHKWALL:
		return (!cell.walkable && !cell.shootable);

	case CELL_CHKWATER:
		return (cell.water);

	case CELL_CHKCLIFF:
		return (!cell.walkable && cell.shootable);


		// base cell type checks
	case CELL_CHKNPC:
		return (cell.npc);
	case CELL_CHKBASILICA:
		return (cell.basilica);
	case CELL_CHKLANDPROTECTOR:
		return (cell.landprotector);
	case CELL_CHKNOVENDING:
		return (cell.novending);
	case CELL_CHKNOCHAT:
		return (cell.nochat);
	case CELL_CHKMAELSTROM:
		return (cell.maelstrom);
	case CELL_CHKICEWALL:
		return (cell.icewall);

		// special checks
	case CELL_CHKPASS:
#ifdef CELL_NOSTACK
		if (cell.cell_bl >= battle_config.cell_stack_limit) return 0;
#endif
	case CELL_CHKREACH:
		return (cell.walkable);

	case CELL_CHKNOPASS:
#ifdef CELL_NOSTACK
		if (cell.cell_bl >= battle_config.cell_stack_limit) return 1;
#endif
	case CELL_CHKNOREACH:
		return (!cell.walkable);

	case CELL_CHKSTACK:
#ifdef CELL_NOSTACK
		return (cell.cell_bl >= battle_config.cell_stack_limit);
#else
		return 0;
#endif

	default:
		return 0;
	}
}

/* [Ind/Hercules] */
int map_sub_getcellp(struct map_data* m,int16 x,int16 y,cell_chk cellchk) {
	map->cellfromcache(m);
	m->getcellp = map->getcellp;
	m->setcell  = map->setcell;
	return m->getcellp(m,x,y,cellchk);
}
/*==========================================
 * Change the type/flags of a map cell
 * 'cell' - which flag to modify
 * 'flag' - true = on, false = off
 *------------------------------------------*/
void map_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag) {
	int j;

	if( m < 0 || m >= map->count || x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys )
		return;

	j = x + y*map->list[m].xs;

	switch( cell ) {
	case CELL_WALKABLE:      map->list[m].cell[j].walkable = flag;      break;
	case CELL_SHOOTABLE:     map->list[m].cell[j].shootable = flag;     break;
	case CELL_WATER:         map->list[m].cell[j].water = flag;         break;

	case CELL_NPC:           map->list[m].cell[j].npc = flag;           break;
	case CELL_BASILICA:      map->list[m].cell[j].basilica = flag;      break;
	case CELL_LANDPROTECTOR: map->list[m].cell[j].landprotector = flag; break;
	case CELL_NOVENDING:     map->list[m].cell[j].novending = flag;     break;
	case CELL_NOCHAT:        map->list[m].cell[j].nochat = flag;        break;
	case CELL_MAELSTROM:     map->list[m].cell[j].maelstrom = flag;     break;
	case CELL_ICEWALL:       map->list[m].cell[j].icewall = flag;       break;
	default:
		ShowWarning("map_setcell: invalid cell type '%d'\n", (int)cell);
		break;
	}
}
void map_sub_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag) {
	if( m < 0 || m >= map->count || x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys )
		return;

	map->cellfromcache(&map->list[m]);
	map->list[m].setcell = map->setcell;
	map->list[m].getcellp = map->getcellp;
	map->list[m].setcell(m,x,y,cell,flag);
}
void map_setgatcell(int16 m, int16 x, int16 y, int gat) {
	int j;
	struct mapcell cell;

	if( m < 0 || m >= map->count || x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys )
		return;

	j = x + y*map->list[m].xs;

	cell = map->gat2cell(gat);
	map->list[m].cell[j].walkable = cell.walkable;
	map->list[m].cell[j].shootable = cell.shootable;
	map->list[m].cell[j].water = cell.water;
}

/*==========================================
* Invisible Walls
*------------------------------------------*/
void map_iwall_nextxy(int16 x, int16 y, int8 dir, int pos, int16 *x1, int16 *y1)
{
	if( dir == 0 || dir == 4 )
		*x1 = x; // Keep X
	else if( dir > 0 && dir < 4 )
		*x1 = x - pos; // Going left
	else
		*x1 = x + pos; // Going right

	if( dir == 2 || dir == 6 )
		*y1 = y;
	else if( dir > 2 && dir < 6 )
		*y1 = y - pos;
	else
		*y1 = y + pos;
}

bool map_iwall_set(int16 m, int16 x, int16 y, int size, int8 dir, bool shootable, const char* wall_name)
{
	struct iwall_data *iwall;
	int i;
	int16 x1 = 0, y1 = 0;

	if( size < 1 || !wall_name )
		return false;

	if( (iwall = (struct iwall_data *)strdb_get(map->iwall_db, wall_name)) != NULL )
		return false; // Already Exists

	if( map->getcell(m, x, y, CELL_CHKNOREACH) )
		return false; // Starting cell problem

	CREATE(iwall, struct iwall_data, 1);
	iwall->m = m;
	iwall->x = x;
	iwall->y = y;
	iwall->size = size;
	iwall->dir = dir;
	iwall->shootable = shootable;
	safestrncpy(iwall->wall_name, wall_name, sizeof(iwall->wall_name));

	for( i = 0; i < size; i++ ) {
		map->iwall_nextxy(x, y, dir, i, &x1, &y1);

		if( map->getcell(m, x1, y1, CELL_CHKNOREACH) )
			break; // Collision

		map->list[m].setcell(m, x1, y1, CELL_WALKABLE, false);
		map->list[m].setcell(m, x1, y1, CELL_SHOOTABLE, shootable);

		clif->changemapcell(0, m, x1, y1, map->getcell(m, x1, y1, CELL_GETTYPE), ALL_SAMEMAP);
	}

	iwall->size = i;

	strdb_put(map->iwall_db, iwall->wall_name, iwall);
	map->list[m].iwall_num++;

	return true;
}

void map_iwall_get(struct map_session_data *sd) {
	struct iwall_data *iwall;
	DBIterator* iter;
	int16 x1, y1;
	int i;

	if( map->list[sd->bl.m].iwall_num < 1 )
		return;

	iter = db_iterator(map->iwall_db);
	for( iwall = dbi_first(iter); dbi_exists(iter); iwall = dbi_next(iter) ) {
		if( iwall->m != sd->bl.m )
			continue;

		for( i = 0; i < iwall->size; i++ ) {
			map->iwall_nextxy(iwall->x, iwall->y, iwall->dir, i, &x1, &y1);
			clif->changemapcell(sd->fd, iwall->m, x1, y1, map->getcell(iwall->m, x1, y1, CELL_GETTYPE), SELF);
		}
	}
	dbi_destroy(iter);
}

void map_iwall_remove(const char *wall_name)
{
	struct iwall_data *iwall;
	int16 i, x1, y1;

	if( (iwall = (struct iwall_data *)strdb_get(map->iwall_db, wall_name)) == NULL )
		return; // Nothing to do

	for( i = 0; i < iwall->size; i++ ) {
		map->iwall_nextxy(iwall->x, iwall->y, iwall->dir, i, &x1, &y1);

		map->list[iwall->m].setcell(iwall->m, x1, y1, CELL_SHOOTABLE, true);
		map->list[iwall->m].setcell(iwall->m, x1, y1, CELL_WALKABLE, true);

		clif->changemapcell(0, iwall->m, x1, y1, map->getcell(iwall->m, x1, y1, CELL_GETTYPE), ALL_SAMEMAP);
	}

	map->list[iwall->m].iwall_num--;
	strdb_remove(map->iwall_db, iwall->wall_name);
}

/**
 * @see DBCreateData
 */
DBData create_map_data_other_server(DBKey key, va_list args)
{
	struct map_data_other_server *mdos;
	unsigned short mapindex = (unsigned short)key.ui;
	mdos=(struct map_data_other_server *)aCalloc(1,sizeof(struct map_data_other_server));
	mdos->index = mapindex;
	memcpy(mdos->name, mapindex_id2name(mapindex), MAP_NAME_LENGTH);
	return DB->ptr2data(mdos);
}

/*==========================================
 * Add mapindex to db of another map server
 *------------------------------------------*/
int map_setipport(unsigned short mapindex, uint32 ip, uint16 port)
{
	struct map_data_other_server *mdos;

	mdos= uidb_ensure(map->map_db,(unsigned int)mapindex, map->create_map_data_other_server);

	if(mdos->cell) //Local map,Do nothing. Give priority to our own local maps over ones from another server. [Skotlex]
		return 0;
	if(ip == clif->map_ip && port == clif->map_port) {
		//That's odd, we received info that we are the ones with this map, but... we don't have it.
		ShowFatalError("map_setipport : received info that this map-server SHOULD have map '%s', but it is not loaded.\n",mapindex_id2name(mapindex));
		exit(EXIT_FAILURE);
	}
	mdos->ip   = ip;
	mdos->port = port;
	return 1;
}

/**
 * Delete all the other maps server management
 * @see DBApply
 */
int map_eraseallipport_sub(DBKey key, DBData *data, va_list va)
{
	struct map_data_other_server *mdos = DB->data2ptr(data);
	if(mdos->cell == NULL) {
		db_remove(map->map_db,key);
		aFree(mdos);
	}
	return 0;
}

int map_eraseallipport(void) {
	map->map_db->foreach(map->map_db,map->eraseallipport_sub);
	return 1;
}

/*==========================================
 * Delete mapindex from db of another map server
 *------------------------------------------*/
int map_eraseipport(unsigned short mapindex, uint32 ip, uint16 port) {
	struct map_data_other_server *mdos;

	mdos = (struct map_data_other_server*)uidb_get(map->map_db,(unsigned int)mapindex);
	if(!mdos || mdos->cell) //Map either does not exists or is a local map.
		return 0;

	if(mdos->ip==ip && mdos->port == port) {
		uidb_remove(map->map_db,(unsigned int)mapindex);
		aFree(mdos);
		return 1;
	}
	return 0;
}

/*==========================================
 * [Shinryo]: Init the mapcache
 *------------------------------------------*/
char *map_init_mapcache(FILE *fp) {
	size_t size = 0;
	char *buffer;

	// No file open? Return..
	nullpo_ret(fp);

	// Get file size
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Allocate enough space
	CREATE(buffer, char, size);

	// No memory? Return..
	nullpo_ret(buffer);

	// Read file into buffer..
	if(fread(buffer, sizeof(char), size, fp) != size) {
		ShowError("map_init_mapcache: Could not read entire mapcache file\n");
		return NULL;
	}

	return buffer;
}

/*==========================================
 * Map cache reading
 * [Shinryo]: Optimized some behaviour to speed this up
 *==========================================*/
int map_readfromcache(struct map_data *m, char *buffer) {
	int i;
	struct map_cache_main_header *header = (struct map_cache_main_header *)buffer;
	struct map_cache_map_info *info = NULL;
	char *p = buffer + sizeof(struct map_cache_main_header);

	for(i = 0; i < header->map_count; i++) {
		info = (struct map_cache_map_info *)p;

		if( strcmp(m->name, info->name) == 0 )
			break; // Map found

		// Jump to next entry..
		p += sizeof(struct map_cache_map_info) + info->len;
	}

	if( info && i < header->map_count ) {
		unsigned long size;

		if( info->xs <= 0 || info->ys <= 0 )
			return 0;// Invalid

		m->xs = info->xs;
		m->ys = info->ys;
		size = (unsigned long)info->xs*(unsigned long)info->ys;

		if(size > MAX_MAP_SIZE) {
			ShowWarning("map_readfromcache: %s exceeded MAX_MAP_SIZE of %d\n", info->name, MAX_MAP_SIZE);
			return 0; // Say not found to remove it from list.. [Shinryo]
		}

		m->cellPos = p;
		m->cell = (struct mapcell *)0xdeadbeaf;

		return 1;
	}

	return 0; // Not found
}


int map_addmap(const char* mapname) {
	map->list[map->count].instance_id = -1;
	mapindex_getmapname(mapname, map->list[map->count++].name);
	return 0;
}

void map_delmapid(int id) {
	ShowNotice("Removing map [ %s ] from maplist"CL_CLL"\n",map->list[id].name);
	memmove(map->list+id, map->list+id+1, sizeof(map->list[0])*(map->count-id-1));
	map->count--;
}

int map_delmap(char* mapname) {
	int i;
	char map_name[MAP_NAME_LENGTH];

	if (strcmpi(mapname, "all") == 0) {
		map->count = 0;
		return 0;
	}

	mapindex_getmapname(mapname, map_name);
	for(i = 0; i < map->count; i++) {
		if (strcmp(map->list[i].name, map_name) == 0) {
			map->delmapid(i);
			return 1;
		}
	}
	return 0;
}
void map_zone_db_clear(void) {
	struct map_zone_data *zone;
	int i;

	DBIterator *iter = db_iterator(map->zone_db);
	for(zone = dbi_first(iter); dbi_exists(iter); zone = dbi_next(iter)) {
		for(i = 0; i < zone->disabled_skills_count; i++) {
			aFree(zone->disabled_skills[i]);
		}
		aFree(zone->disabled_skills);
		aFree(zone->disabled_items);
		for(i = 0; i < zone->mapflags_count; i++) {
			aFree(zone->mapflags[i]);
		}
		aFree(zone->mapflags);
		for(i = 0; i < zone->disabled_commands_count; i++) {
			aFree(zone->disabled_commands[i]);
		}
		aFree(zone->disabled_commands);
		for(i = 0; i < zone->capped_skills_count; i++) {
			aFree(zone->capped_skills[i]);
		}
		aFree(zone->capped_skills);
	}
	dbi_destroy(iter);

	db_destroy(map->zone_db);/* will aFree(zone) */

	/* clear the pk zone stuff */
	for(i = 0; i < map->zone_pk.disabled_skills_count; i++) {
		aFree(map->zone_pk.disabled_skills[i]);
	}
	aFree(map->zone_pk.disabled_skills);
	aFree(map->zone_pk.disabled_items);
	for(i = 0; i < map->zone_pk.mapflags_count; i++) {
		aFree(map->zone_pk.mapflags[i]);
	}
	aFree(map->zone_pk.mapflags);
	for(i = 0; i < map->zone_pk.disabled_commands_count; i++) {
		aFree(map->zone_pk.disabled_commands[i]);
	}
	aFree(map->zone_pk.disabled_commands);
	for(i = 0; i < map->zone_pk.capped_skills_count; i++) {
		aFree(map->zone_pk.capped_skills[i]);
	}
	aFree(map->zone_pk.capped_skills);
	/* clear the main zone stuff */
	for(i = 0; i < map->zone_all.disabled_skills_count; i++) {
		aFree(map->zone_all.disabled_skills[i]);
	}
	aFree(map->zone_all.disabled_skills);
	aFree(map->zone_all.disabled_items);
	for(i = 0; i < map->zone_all.mapflags_count; i++) {
		aFree(map->zone_all.mapflags[i]);
	}
	aFree(map->zone_all.mapflags);
	for(i = 0; i < map->zone_all.disabled_commands_count; i++) {
		aFree(map->zone_all.disabled_commands[i]);
	}
	aFree(map->zone_all.disabled_commands);
	for(i = 0; i < map->zone_all.capped_skills_count; i++) {
		aFree(map->zone_all.capped_skills[i]);
	}
	aFree(map->zone_all.capped_skills);
}
void map_clean(int i) {
	int v;
	if(map->list[i].cell && map->list[i].cell != (struct mapcell *)0xdeadbeaf) aFree(map->list[i].cell);
	if(map->list[i].block) aFree(map->list[i].block);
	if(map->list[i].block_mob) aFree(map->list[i].block_mob);

	if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
		int j;
		if(map->list[i].mob_delete_timer != INVALID_TIMER)
			timer->delete(map->list[i].mob_delete_timer, map->removemobs_timer);
		for (j=0; j<MAX_MOB_LIST_PER_MAP; j++)
			if (map->list[i].moblist[j]) aFree(map->list[i].moblist[j]);
	}

	if( map->list[i].unit_count ) {
		for(v = 0; v < map->list[i].unit_count; v++) {
			aFree(map->list[i].units[v]);
		}
		if( map->list[i].units ) {
			aFree(map->list[i].units);
			map->list[i].units = NULL;
		}
		map->list[i].unit_count = 0;
	}

	if( map->list[i].skill_count ) {
		for(v = 0; v < map->list[i].skill_count; v++) {
			aFree(map->list[i].skills[v]);
		}
		if( map->list[i].skills ) {
			aFree(map->list[i].skills);
			map->list[i].skills = NULL;
		}
		map->list[i].skill_count = 0;
	}

	if( map->list[i].zone_mf_count ) {
		for(v = 0; v < map->list[i].zone_mf_count; v++) {
			aFree(map->list[i].zone_mf[v]);
		}
		if( map->list[i].zone_mf ) {
			aFree(map->list[i].zone_mf);
			map->list[i].zone_mf = NULL;
		}
		map->list[i].zone_mf_count = 0;
	}

	if( map->list[i].channel )
		clif->chsys_delete(map->list[i].channel);
}
void do_final_maps(void) {
	int i, v = 0;

	for( i = 0; i < map->count; i++ ) {

		if(map->list[i].cell && map->list[i].cell != (struct mapcell *)0xdeadbeaf ) aFree(map->list[i].cell);
		if(map->list[i].block) aFree(map->list[i].block);
		if(map->list[i].block_mob) aFree(map->list[i].block_mob);

		if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
			int j;
			if(map->list[i].mob_delete_timer != INVALID_TIMER)
				timer->delete(map->list[i].mob_delete_timer, map->removemobs_timer);
			for (j=0; j<MAX_MOB_LIST_PER_MAP; j++)
				if (map->list[i].moblist[j]) aFree(map->list[i].moblist[j]);
		}

		if( map->list[i].unit_count ) {
			for(v = 0; v < map->list[i].unit_count; v++) {
				aFree(map->list[i].units[v]);
			}
			if( map->list[i].units ) {
				aFree(map->list[i].units);
				map->list[i].units = NULL;
			}
			map->list[i].unit_count = 0;
		}

		if( map->list[i].skill_count ) {
			for(v = 0; v < map->list[i].skill_count; v++) {
				aFree(map->list[i].skills[v]);
			}
			if( map->list[i].skills ) {
				aFree(map->list[i].skills);
				map->list[i].skills = NULL;
			}
			map->list[i].skill_count = 0;
		}

		if( map->list[i].zone_mf_count ) {
			for(v = 0; v < map->list[i].zone_mf_count; v++) {
				aFree(map->list[i].zone_mf[v]);
			}
			if( map->list[i].zone_mf ) {
				aFree(map->list[i].zone_mf);
				map->list[i].zone_mf = NULL;
			}
			map->list[i].zone_mf_count = 0;
		}

		if( map->list[i].drop_list_count ) {
			map->list[i].drop_list_count = 0;
		}
		if( map->list[i].drop_list != NULL )
			aFree(map->list[i].drop_list);

		if( map->list[i].channel )
			clif->chsys_delete(map->list[i].channel);
		
		if( map->list[i].qi_data )
			aFree(map->list[i].qi_data);
		
	}

	map->zone_db_clear();

}
/// Initializes map flags and adjusts them depending on configuration.
void map_flags_init(void) {
	int i, v = 0;

	for( i = 0; i < map->count; i++ ) {
		// mapflags
		memset(&map->list[i].flag, 0, sizeof(map->list[i].flag));

		// additional mapflag data
		map->list[i].nocommand = 0;   // nocommand mapflag level
		map->list[i].bexp      = 100; // per map base exp multiplicator
		map->list[i].jexp      = 100; // per map job exp multiplicator
		if( map->list[i].drop_list != NULL )
			aFree(map->list[i].drop_list);
		map->list[i].drop_list = NULL;
		map->list[i].drop_list_count = 0;

		if( map->list[i].unit_count ) {
			for(v = 0; v < map->list[i].unit_count; v++) {
				aFree(map->list[i].units[v]);
			}
			aFree(map->list[i].units);
		}
		map->list[i].units = NULL;
		map->list[i].unit_count = 0;

		if( map->list[i].skill_count ) {
			for(v = 0; v < map->list[i].skill_count; v++) {
				aFree(map->list[i].skills[v]);
			}
			aFree(map->list[i].skills);
		}
		map->list[i].skills = NULL;
		map->list[i].skill_count = 0;

		// adjustments
		if( battle_config.pk_mode ) {
			map->list[i].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]
			map->list[i].zone = &map->zone_pk;
		} else /* align with 'All' zone */
			map->list[i].zone = &map->zone_all;

		if( map->list[i].zone_mf_count ) {
			for(v = 0; v < map->list[i].zone_mf_count; v++) {
				aFree(map->list[i].zone_mf[v]);
			}
			aFree(map->list[i].zone_mf);
		}

		map->list[i].zone_mf = NULL;
		map->list[i].zone_mf_count = 0;
		map->list[i].prev_zone = map->list[i].zone;

		map->list[i].invincible_time_inc = 0;

		map->list[i].weapon_damage_rate = 100;
		map->list[i].magic_damage_rate  = 100;
		map->list[i].misc_damage_rate   = 100;
		map->list[i].short_damage_rate  = 100;
		map->list[i].long_damage_rate   = 100;
		
		if( map->list[i].qi_data )
			aFree(map->list[i].qi_data);
		
		map->list[i].qi_data = NULL;
		map->list[i].qi_count = 0;
	}
}

#define NO_WATER 1000000

/*
 * Reads from the .rsw for each map
 * Returns water height (or NO_WATER if file doesn't exist) or other error is encountered.
 * Assumed path for file is data/mapname.rsw
 * Credits to LittleWolf
 */
int map_waterheight(char* mapname)
{
	char fn[256];
	char *rsw, *found;

	//Look up for the rsw
	sprintf(fn, "data\\%s.rsw", mapname);

	found = grfio_find_file(fn);
	if (found) strcpy(fn, found); // replace with real name

	// read & convert fn
	rsw = (char *) grfio_read (fn);
	if (rsw)
	{	//Load water height from file
		int wh = (int) *(float*)(rsw+166);
		aFree(rsw);
		return wh;
	}
	ShowWarning("Failed to find water level for (%s)\n", mapname, fn);
	return NO_WATER;
}

/*==================================
 * .GAT format
 *----------------------------------*/
int map_readgat (struct map_data* m)
{
	char filename[256];
	uint8* gat;
	int water_height;
	size_t xy, off, num_cells;

	sprintf(filename, "data\\%s.gat", m->name);

	gat = (uint8 *) grfio_read(filename);
	if (gat == NULL)
		return 0;

	m->xs = *(int32*)(gat+6);
	m->ys = *(int32*)(gat+10);
	num_cells = m->xs * m->ys;
	CREATE(m->cell, struct mapcell, num_cells);

	water_height = map->waterheight(m->name);

	// Set cell properties
	off = 14;
	for( xy = 0; xy < num_cells; ++xy )
	{
		// read cell data
		float height = *(float*)( gat + off      );
		uint32 type = *(uint32*)( gat + off + 16 );
		off += 20;

		if( type == 0 && water_height != NO_WATER && height > water_height )
			type = 3; // Cell is 0 (walkable) but under water level, set to 3 (walkable water)

		m->cell[xy] = map->gat2cell(type);
	}

	aFree(gat);

	return 1;
}

/*======================================
 * Add/Remove map to the map_db
 *--------------------------------------*/
void map_addmap2db(struct map_data *m) {
	map->index2mapid[m->index] = m->m;
}

void map_removemapdb(struct map_data *m) {
	map->index2mapid[m->index] = -1;
}

/*======================================
 * Initiate maps loading stage
 *--------------------------------------*/
int map_readallmaps (void) {
	int i;
	FILE* fp=NULL;
	int maps_removed = 0;

	if( map->enable_grf )
		ShowStatus("Loading maps (using GRF files)...\n");
	else {
		char mapcachefilepath[254];
		sprintf(mapcachefilepath,"%s/%s%s",map->db_path,DBPATH,"map_cache.dat");
		ShowStatus("Loading maps (using %s as map cache)...\n", mapcachefilepath);
		if( (fp = fopen(mapcachefilepath, "rb")) == NULL ) {
			ShowFatalError("Unable to open map cache file "CL_WHITE"%s"CL_RESET"\n", mapcachefilepath);
			exit(EXIT_FAILURE); //No use launching server if maps can't be read.
		}

		// Init mapcache data.. [Shinryo]
		map->cache_buffer = map->init_mapcache(fp);
		if(!map->cache_buffer) {
			ShowFatalError("Failed to initialize mapcache data (%s)..\n", mapcachefilepath);
			exit(EXIT_FAILURE);
		}
	}

	for(i = 0; i < map->count; i++) {
		size_t size;

		// show progress
		if(map->enable_grf)
			ShowStatus("Loading maps [%i/%i]: %s"CL_CLL"\r", i, map->count, map->list[i].name);

		// try to load the map
		if( !
			(map->enable_grf?
			map->readgat(&map->list[i])
			:map->readfromcache(&map->list[i], map->cache_buffer))
			) {
				map->delmapid(i);
				maps_removed++;
				i--;
				continue;
		}

		map->list[i].index = mapindex_name2id(map->list[i].name);

		if ( map->index2mapid[map_id2index(i)] != -1 ) {
			ShowWarning("Map %s already loaded!"CL_CLL"\n", map->list[i].name);
			if (map->list[i].cell && map->list[i].cell != (struct mapcell *)0xdeadbeaf) {
				aFree(map->list[i].cell);
				map->list[i].cell = NULL;
			}
			map->delmapid(i);
			maps_removed++;
			i--;
			continue;
		}

		map->list[i].m = i;
		map->addmap2db(&map->list[i]);

		memset(map->list[i].moblist, 0, sizeof(map->list[i].moblist));	//Initialize moblist [Skotlex]
		map->list[i].mob_delete_timer = INVALID_TIMER;	//Initialize timer [Skotlex]

		map->list[i].bxs = (map->list[i].xs + BLOCK_SIZE - 1) / BLOCK_SIZE;
		map->list[i].bys = (map->list[i].ys + BLOCK_SIZE - 1) / BLOCK_SIZE;

		size = map->list[i].bxs * map->list[i].bys * sizeof(struct block_list*);
		map->list[i].block = (struct block_list**)aCalloc(size, 1);
		map->list[i].block_mob = (struct block_list**)aCalloc(size, 1);

		map->list[i].getcellp = map->sub_getcellp;
		map->list[i].setcell  = map->sub_setcell;
	}

	// intialization and configuration-dependent adjustments of mapflags
	map->flags_init();

	if( !map->enable_grf ) {
		fclose(fp);
	}

	// finished map loading
	ShowInfo("Successfully loaded '"CL_WHITE"%d"CL_RESET"' maps."CL_CLL"\n",map->count);
	instance->start_id = map->count; // Next Map Index will be instances

	if (maps_removed)
		ShowNotice("Maps removed: '"CL_WHITE"%d"CL_RESET"'\n",maps_removed);

	return 0;
}

/*==========================================
 * Read map server configuration files (conf/map_server.conf...)
 *------------------------------------------*/
int map_config_read(char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if( fp == NULL ) {
		ShowError("Map configuration file not found at: %s\n", cfgName);
		return 1;
	}

	while( fgets(line, sizeof(line), fp) ) {
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( (ptr = strstr(line, "//")) != NULL )
			*ptr = '\n'; //Strip comments
		if( sscanf(line, "%[^:]: %[^\t\r\n]", w1, w2) < 2 )
			continue;

		//Strip trailing spaces
		ptr = w2 + strlen(w2);
		while (--ptr >= w2 && *ptr == ' ');
		ptr++;
		*ptr = '\0';

		if(strcmpi(w1,"timestamp_format")==0)
			safestrncpy(timestamp_format, w2, 20);
		else if(strcmpi(w1,"stdout_with_ansisequence")==0)
			stdout_with_ansisequence = config_switch(w2);
		else if(strcmpi(w1,"console_silent")==0) {
			msg_silent = atoi(w2);
			if( msg_silent ) // only bother if its actually enabled
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
		} else if (strcmpi(w1, "userid")==0)
			chrif->setuserid(w2);
		else if (strcmpi(w1, "passwd") == 0)
			chrif->setpasswd(w2);
		else if (strcmpi(w1, "char_ip") == 0)
			map->char_ip_set = chrif->setip(w2);
		else if (strcmpi(w1, "char_port") == 0)
			chrif->setport(atoi(w2));
		else if (strcmpi(w1, "map_ip") == 0)
			map->ip_set = clif->setip(w2);
		else if (strcmpi(w1, "bind_ip") == 0)
			clif->setbindip(w2);
		else if (strcmpi(w1, "map_port") == 0) {
			clif->setport(atoi(w2));
			map->port = (atoi(w2));
		} else if (strcmpi(w1, "map") == 0)
			map->count++;
		else if (strcmpi(w1, "delmap") == 0)
			map->count--;
		else if (strcmpi(w1, "npc") == 0)
			npc->addsrcfile(w2);
		else if (strcmpi(w1, "delnpc") == 0)
			npc->delsrcfile(w2);
		else if (strcmpi(w1, "autosave_time") == 0) {
			map->autosave_interval = atoi(w2);
			if (map->autosave_interval < 1) //Revert to default saving.
				map->autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			else
				map->autosave_interval *= 1000; //Pass from sec to ms
		} else if (strcmpi(w1, "minsave_time") == 0) {
			map->minsave_interval= atoi(w2);
			if (map->minsave_interval < 1)
				map->minsave_interval = 1;
		} else if (strcmpi(w1, "save_settings") == 0)
			map->save_settings = atoi(w2);
		else if (strcmpi(w1, "help_txt") == 0)
			strcpy(map->help_txt, w2);
		else if (strcmpi(w1, "help2_txt") == 0)
			strcpy(map->help2_txt, w2);
		else if (strcmpi(w1, "charhelp_txt") == 0)
			strcpy(map->charhelp_txt, w2);
		else if(strcmpi(w1,"db_path") == 0)
			safestrncpy(map->db_path,w2,255);
		else if (strcmpi(w1, "enable_spy") == 0)
			map->enable_spy = config_switch(w2);
		else if (strcmpi(w1, "use_grf") == 0)
			map->enable_grf = config_switch(w2);
		else if (strcmpi(w1, "console_msg_log") == 0)
			console_msg_log = atoi(w2);//[Ind]
		else if (strcmpi(w1, "import") == 0)
			map->config_read(w2);
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}

	fclose(fp);
	return 0;
}
int map_config_read_sub(char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if( fp == NULL ) {
		ShowError("Map configuration file not found at: %s\n", cfgName);
		return 1;
	}

	while( fgets(line, sizeof(line), fp) ) {
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( (ptr = strstr(line, "//")) != NULL )
			*ptr = '\n'; //Strip comments
		if( sscanf(line, "%[^:]: %[^\t\r\n]", w1, w2) < 2 )
			continue;

		//Strip trailing spaces
		ptr = w2 + strlen(w2);
		while (--ptr >= w2 && *ptr == ' ');
		ptr++;
		*ptr = '\0';

		if (strcmpi(w1, "map") == 0)
			map->addmap(w2);
		else if (strcmpi(w1, "delmap") == 0)
			map->delmap(w2);
		else if (strcmpi(w1, "import") == 0)
			map->config_read_sub(w2);
	}

	fclose(fp);
	return 0;
}
void map_reloadnpc_sub(char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName,"r");
	if( fp == NULL )
	{
		ShowError("Map configuration file not found at: %s\n", cfgName);
		return;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		char* ptr;

		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( (ptr = strstr(line, "//")) != NULL )
			*ptr = '\n'; //Strip comments
		if( sscanf(line, "%[^:]: %[^\t\r\n]", w1, w2) < 2 )
			continue;

		//Strip trailing spaces
		ptr = w2 + strlen(w2);
		while (--ptr >= w2 && *ptr == ' ');
		ptr++;
		*ptr = '\0';

		if (strcmpi(w1, "npc") == 0)
			npc->addsrcfile(w2);
		else if (strcmpi(w1, "import") == 0)
			map->reloadnpc_sub(w2);
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}

	fclose(fp);
}

void map_reloadnpc(bool clear)
{
	if (clear)
		npc->addsrcfile("clear"); // this will clear the current script list

#ifdef RENEWAL
	map->reloadnpc_sub("npc/re/scripts_main.conf");
#else
	map->reloadnpc_sub("npc/pre-re/scripts_main.conf");
#endif
}

int inter_config_read(char *cfgName) {
	char line[1024],w1[1024],w2[1024];
	FILE *fp;

	if( !( fp = fopen(cfgName,"r") ) ){
		ShowError("File not found: %s\n",cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) {
		if(line[0] == '/' && line[1] == '/')
			continue;
		
		if( sscanf(line,"%[^:]: %[^\r\n]",w1,w2) < 2 )
			continue;
		/* table names */
		if(strcmpi(w1,"item_db_db")==0)
			strcpy(map->item_db_db,w2);
		else if(strcmpi(w1,"mob_db_db")==0)
			strcpy(map->mob_db_db,w2);
		else if(strcmpi(w1,"item_db2_db")==0)
			strcpy(map->item_db2_db,w2);
		else if(strcmpi(w1,"item_db_re_db")==0)
			strcpy(map->item_db_re_db,w2);
		else if(strcmpi(w1,"mob_db2_db")==0)
			strcpy(map->mob_db2_db,w2);
		else if(strcmpi(w1,"mob_skill_db_db")==0)
			strcpy(map->mob_skill_db_db,w2);
		else if(strcmpi(w1,"mob_skill_db2_db")==0)
			strcpy(map->mob_skill_db2_db,w2);
		else if(strcmpi(w1,"interreg_db")==0)
			strcpy(map->interreg_db,w2);
		/* map sql stuff */
		else if(strcmpi(w1,"map_server_ip")==0)
			strcpy(map->server_ip, w2);
		else if(strcmpi(w1,"map_server_port")==0)
			map->server_port=atoi(w2);
		else if(strcmpi(w1,"map_server_id")==0)
			strcpy(map->server_id, w2);
		else if(strcmpi(w1,"map_server_pw")==0)
			strcpy(map->server_pw, w2);
		else if(strcmpi(w1,"map_server_db")==0)
			strcpy(map->server_db, w2);
		else if(strcmpi(w1,"default_codepage")==0)
			strcpy(map->default_codepage, w2);
		else if(strcmpi(w1,"use_sql_item_db")==0) {
			map->db_use_sql_item_db = config_switch(w2);
			ShowStatus ("Using item database as SQL: '%s'\n", w2);
		}
		else if(strcmpi(w1,"use_sql_mob_db")==0) {
			map->db_use_sql_mob_db = config_switch(w2);
			ShowStatus ("Using monster database as SQL: '%s'\n", w2);
		}
		else if(strcmpi(w1,"use_sql_mob_skill_db")==0) {
			map->db_use_sql_mob_skill_db = config_switch(w2);
			ShowStatus ("Using monster skill database as SQL: '%s'\n", w2);
		}
		/* sql log db */
		else if(strcmpi(w1,"log_db_ip")==0)
			strcpy(logs->db_ip, w2);
		else if(strcmpi(w1,"log_db_id")==0)
			strcpy(logs->db_id, w2);
		else if(strcmpi(w1,"log_db_pw")==0)
			strcpy(logs->db_pw, w2);
		else if(strcmpi(w1,"log_db_port")==0)
			logs->db_port = atoi(w2);
		else if(strcmpi(w1,"log_db_db")==0)
			strcpy(logs->db_name, w2);
		/* mapreg */
		else if( mapreg->config_read(w1,w2) )
			continue;
		/* import */
		else if(strcmpi(w1,"import")==0)
			map->inter_config_read(w2);
	}
	fclose(fp);

	return 0;
}

/*=======================================
 *  MySQL Init
 *---------------------------------------*/
int map_sql_init(void)
{
	// main db connection
	map->mysql_handle = SQL->Malloc();

	ShowInfo("Connecting to the Map DB Server....\n");
	if( SQL_ERROR == SQL->Connect(map->mysql_handle, map->server_id, map->server_pw, map->server_ip, map->server_port, map->server_db) )
		exit(EXIT_FAILURE);
	ShowStatus("connect success! (Map Server Connection)\n");

	if( strlen(map->default_codepage) > 0 )
		if ( SQL_ERROR == SQL->SetEncoding(map->mysql_handle, map->default_codepage) )
			Sql_ShowDebug(map->mysql_handle);

	return 0;
}

int map_sql_close(void)
{
	ShowStatus("Close Map DB Connection....\n");
	SQL->Free(map->mysql_handle);
	map->mysql_handle = NULL;
	if (logs->config.sql_logs) {
		logs->sql_final();
	}
	return 0;
}

void map_zone_change2(int m, struct map_zone_data *zone) {
	char empty[1] = "\0";

	map->list[m].prev_zone = map->list[m].zone;

	if( map->list[m].zone_mf_count )
		map->zone_remove(m);

	map->zone_apply(m,zone,empty,empty,empty);
}
/* when changing from a mapflag to another during runtime */
void map_zone_change(int m, struct map_zone_data *zone, const char* start, const char* buffer, const char* filepath) {
	map->list[m].prev_zone = map->list[m].zone;

	if( map->list[m].zone_mf_count )
		map->zone_remove(m);
	map->zone_apply(m,zone,start,buffer,filepath);
}
/* removes previous mapflags from this map */
void map_zone_remove(int m) {
	char flag[MAP_ZONE_MAPFLAG_LENGTH], params[MAP_ZONE_MAPFLAG_LENGTH];
	unsigned short k;
	char empty[1] = "\0";
	for(k = 0; k < map->list[m].zone_mf_count; k++) {
		int len = strlen(map->list[m].zone_mf[k]),j;
		params[0] = '\0';
		memcpy(flag, map->list[m].zone_mf[k], MAP_ZONE_MAPFLAG_LENGTH);
		for(j = 0; j < len; j++) {
			if( flag[j] == '\t' ) {
				memcpy(params, &flag[j+1], len - j);
				flag[j] = '\0';
				break;
			}
		}

		npc->parse_mapflag(map->list[m].name,empty,flag,params,empty,empty,empty);
		aFree(map->list[m].zone_mf[k]);
		map->list[m].zone_mf[k] = NULL;
	}

	aFree(map->list[m].zone_mf);
	map->list[m].zone_mf = NULL;
	map->list[m].zone_mf_count = 0;
}
static inline void map_zone_mf_cache_add(int m, char *rflag) {
	RECREATE(map->list[m].zone_mf, char *, ++map->list[m].zone_mf_count);
	CREATE(map->list[m].zone_mf[map->list[m].zone_mf_count - 1], char, MAP_ZONE_MAPFLAG_LENGTH);
	safestrncpy(map->list[m].zone_mf[map->list[m].zone_mf_count - 1], rflag, MAP_ZONE_MAPFLAG_LENGTH);
}
/* TODO: introduce enumerations to each mapflag so instead of reading the string a number of times we read it only once and use its value wherever we need */
/* cache previous values to revert */
bool map_zone_mf_cache(int m, char *flag, char *params) {
	char rflag[MAP_ZONE_MAPFLAG_LENGTH];
	int state = 1;

	if (params[0] != '\0' && !strcmpi(params, "off"))
		state = 0;

	if (!strcmpi(flag, "nosave")) {
#if 0 /* not yet supported to be reversed */
		char savemap[32];
		int savex, savey;
		if (state == 0) {
			if( map->list[m].flag.nosave ) {
				sprintf(rflag, "nosave\tSavePoint");
				map_zone_mf_cache_add(m,nosave);
			}
		} else if (!strcmpi(params, "SavePoint")) {
			if( map->list[m].save.map ) {
				sprintf(rflag, "nosave\t%s,%d,%d",mapindex_id2name(map->list[m].save.map),map->list[m].save.x,map->list[m].save.y);
			} else
				sprintf(rflag, "nosave\t%s,%d,%d",mapindex_id2name(map->list[m].save.map),map->list[m].save.x,map->list[m].save.y);
			map_zone_mf_cache_add(m,nosave);
		} else if (sscanf(params, "%31[^,],%d,%d", savemap, &savex, &savey) == 3) {
			if( map->list[m].save.map ) {
				sprintf(rflag, "nosave\t%s,%d,%d",mapindex_id2name(map->list[m].save.map),map->list[m].save.x,map->list[m].save.y);
				map_zone_mf_cache_add(m,nosave);
			}
		}
#endif // 0
	} else if (!strcmpi(flag,"autotrade")) {
		if( state && map->list[m].flag.autotrade )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"autotrade\toff");
			else if( !map->list[m].flag.autotrade )
				map_zone_mf_cache_add(m,"autotrade");
		}
	} else if (!strcmpi(flag,"allowks")) {
		if( state && map->list[m].flag.allowks )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"allowks\toff");
			else if( !map->list[m].flag.allowks )
				map_zone_mf_cache_add(m,"allowks");
		}
	} else if (!strcmpi(flag,"town")) {
		if( state && map->list[m].flag.town )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"town\toff");
			else if( !map->list[m].flag.town )
				map_zone_mf_cache_add(m,"town");
		}
	} else if (!strcmpi(flag,"nomemo")) {
		if( state && map->list[m].flag.nomemo )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomemo\toff");
			else if( !map->list[m].flag.nomemo )
				map_zone_mf_cache_add(m,"nomemo");
		}
	} else if (!strcmpi(flag,"noteleport")) {
		if( state && map->list[m].flag.noteleport )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noteleport\toff");
			else if( !map->list[m].flag.noteleport )
				map_zone_mf_cache_add(m,"noteleport");
		}
	} else if (!strcmpi(flag,"nowarp")) {
		if( state && map->list[m].flag.nowarp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nowarp\toff");
			else if( !map->list[m].flag.nowarp )
				map_zone_mf_cache_add(m,"nowarp");
		}
	} else if (!strcmpi(flag,"nowarpto")) {
		if( state && map->list[m].flag.nowarpto )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nowarpto\toff");
			else if( !map->list[m].flag.nowarpto )
				map_zone_mf_cache_add(m,"nowarpto");
		}
	} else if (!strcmpi(flag,"noreturn")) {
		if( state && map->list[m].flag.noreturn )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noreturn\toff");
			else if( map->list[m].flag.noreturn )
				map_zone_mf_cache_add(m,"noreturn");
		}
	} else if (!strcmpi(flag,"monster_noteleport")) {
		if( state && map->list[m].flag.monster_noteleport )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"monster_noteleport\toff");
			else if( map->list[m].flag.monster_noteleport )
				map_zone_mf_cache_add(m,"monster_noteleport");
		}
	} else if (!strcmpi(flag,"nobranch")) {
		if( state && map->list[m].flag.nobranch )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nobranch\toff");
			else if( map->list[m].flag.nobranch )
				map_zone_mf_cache_add(m,"nobranch");
		}
	} else if (!strcmpi(flag,"nopenalty")) {
		if( state && map->list[m].flag.noexppenalty ) /* they are applied together, no need to check both */
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nopenalty\toff");
			else if( map->list[m].flag.noexppenalty )
				map_zone_mf_cache_add(m,"nopenalty");
		}
	} else if (!strcmpi(flag,"pvp")) {
		if( state && map->list[m].flag.pvp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp\toff");
			else if( map->list[m].flag.pvp )
				map_zone_mf_cache_add(m,"pvp");
		}
	}
	else if (!strcmpi(flag,"pvp_noparty")) {
		if( state && map->list[m].flag.pvp_noparty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_noparty\toff");
			else if( map->list[m].flag.pvp_noparty )
				map_zone_mf_cache_add(m,"pvp_noparty");
		}
	} else if (!strcmpi(flag,"pvp_noguild")) {
		if( state && map->list[m].flag.pvp_noguild )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_noguild\toff");
			else if( map->list[m].flag.pvp_noguild )
				map_zone_mf_cache_add(m,"pvp_noguild");
		}
	} else if (!strcmpi(flag, "pvp_nightmaredrop")) {
		if( state && map->list[m].flag.pvp_nightmaredrop )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_nightmaredrop\toff");
			else if( map->list[m].flag.pvp_nightmaredrop )
				map_zone_mf_cache_add(m,"pvp_nightmaredrop");
		}
#if 0 /* not yet fully supported */
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
				int i;
				for (i = 0; i < MAX_DROP_PER_MAP; i++) {
					if (map->list[m].drop_list[i].drop_id == 0){
						map->list[m].drop_list[i].drop_id = drop_id;
						map->list[m].drop_list[i].drop_type = drop_type;
						map->list[m].drop_list[i].drop_per = drop_per;
						break;
					}
				}
				map->list[m].flag.pvp_nightmaredrop = 1;
			}
		} else if (!state) //Disable
			map->list[m].flag.pvp_nightmaredrop = 0;
#endif // 0
	} else if (!strcmpi(flag,"pvp_nocalcrank")) {
		if( state && map->list[m].flag.pvp_nocalcrank )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_nocalcrank\toff");
			else if( map->list[m].flag.pvp_nocalcrank )
				map_zone_mf_cache_add(m,"pvp_nocalcrank");
		}
	} else if (!strcmpi(flag,"gvg")) {
		if( state && map->list[m].flag.gvg )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg\toff");
			else if( map->list[m].flag.gvg )
				map_zone_mf_cache_add(m,"gvg");
		}
	} else if (!strcmpi(flag,"gvg_noparty")) {
		if( state && map->list[m].flag.gvg_noparty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg_noparty\toff");
			else if( map->list[m].flag.gvg_noparty )
				map_zone_mf_cache_add(m,"gvg_noparty");
		}
	} else if (!strcmpi(flag,"gvg_dungeon")) {
		if( state && map->list[m].flag.gvg_dungeon )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg_dungeon\toff");
			else if( map->list[m].flag.gvg_dungeon )
				map_zone_mf_cache_add(m,"gvg_dungeon");
		}
	}
	else if (!strcmpi(flag,"gvg_castle")) {
		if( state && map->list[m].flag.gvg_castle )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg_castle\toff");
			else if( map->list[m].flag.gvg_castle )
				map_zone_mf_cache_add(m,"gvg_castle");
		}
	}
	else if (!strcmpi(flag,"battleground")) {
		if( state && map->list[m].flag.battleground )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"battleground\toff");
			else if( map->list[m].flag.battleground )
				map_zone_mf_cache_add(m,"battleground");
		}
	} else if (!strcmpi(flag,"noexppenalty")) {
		if( state && map->list[m].flag.noexppenalty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noexppenalty\toff");
			else if( map->list[m].flag.noexppenalty )
				map_zone_mf_cache_add(m,"noexppenalty");
		}
	} else if (!strcmpi(flag,"nozenypenalty")) {
		if( state && map->list[m].flag.nozenypenalty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nozenypenalty\toff");
			else if( map->list[m].flag.nozenypenalty )
				map_zone_mf_cache_add(m,"nozenypenalty");
		}
	} else if (!strcmpi(flag,"notrade")) {
		if( state && map->list[m].flag.notrade )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"notrade\toff");
			else if( map->list[m].flag.notrade )
				map_zone_mf_cache_add(m,"notrade");
		}
	} else if (!strcmpi(flag,"novending")) {
		if( state && map->list[m].flag.novending )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"novending\toff");
			else if( map->list[m].flag.novending )
				map_zone_mf_cache_add(m,"novending");
		}
	} else if (!strcmpi(flag,"nodrop")) {
		if( state && map->list[m].flag.nodrop )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nodrop\toff");
			else if( map->list[m].flag.nodrop )
				map_zone_mf_cache_add(m,"nodrop");
		}
	} else if (!strcmpi(flag,"noskill")) {
		if( state && map->list[m].flag.noskill )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noskill\toff");
			else if( map->list[m].flag.noskill )
				map_zone_mf_cache_add(m,"noskill");
		}
	} else if (!strcmpi(flag,"noicewall")) {
		if( state && map->list[m].flag.noicewall )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noicewall\toff");
			else if( map->list[m].flag.noicewall )
				map_zone_mf_cache_add(m,"noicewall");
		}
	} else if (!strcmpi(flag,"snow")) {
		if( state && map->list[m].flag.snow )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"snow\toff");
			else if( map->list[m].flag.snow )
				map_zone_mf_cache_add(m,"snow");
		}
	} else if (!strcmpi(flag,"clouds")) {
		if( state && map->list[m].flag.clouds )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"clouds\toff");
			else if( map->list[m].flag.clouds )
				map_zone_mf_cache_add(m,"clouds");
		}
	} else if (!strcmpi(flag,"clouds2")) {
		if( state && map->list[m].flag.clouds2 )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"clouds2\toff");
			else if( map->list[m].flag.clouds2 )
				map_zone_mf_cache_add(m,"clouds2");
		}
	} else if (!strcmpi(flag,"fog")) {
		if( state && map->list[m].flag.fog )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"fog\toff");
			else if( map->list[m].flag.fog )
				map_zone_mf_cache_add(m,"fog");
		}
	} else if (!strcmpi(flag,"fireworks")) {
		if( state && map->list[m].flag.fireworks )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"fireworks\toff");
			else if( map->list[m].flag.fireworks )
				map_zone_mf_cache_add(m,"fireworks");
		}
	} else if (!strcmpi(flag,"sakura")) {
		if( state && map->list[m].flag.sakura )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"sakura\toff");
			else if( map->list[m].flag.sakura )
				map_zone_mf_cache_add(m,"sakura");
		}
	} else if (!strcmpi(flag,"leaves")) {
		if( state && map->list[m].flag.leaves )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"leaves\toff");
			else if( map->list[m].flag.leaves )
				map_zone_mf_cache_add(m,"leaves");
		}
	} else if (!strcmpi(flag,"nightenabled")) {
		if( state && map->list[m].flag.nightenabled )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nightenabled\toff");
			else if( map->list[m].flag.nightenabled )
				map_zone_mf_cache_add(m,"nightenabled");
		}
	} else if (!strcmpi(flag,"noexp")) {
		if( state && map->list[m].flag.nobaseexp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noexp\toff");
			else if( map->list[m].flag.nobaseexp )
				map_zone_mf_cache_add(m,"noexp");
		}
	}
	else if (!strcmpi(flag,"nobaseexp")) {
		if( state && map->list[m].flag.nobaseexp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nobaseexp\toff");
			else if( map->list[m].flag.nobaseexp )
				map_zone_mf_cache_add(m,"nobaseexp");
		}
	} else if (!strcmpi(flag,"nojobexp")) {
		if( state && map->list[m].flag.nojobexp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nojobexp\toff");
			else if( map->list[m].flag.nojobexp )
				map_zone_mf_cache_add(m,"nojobexp");
		}
	} else if (!strcmpi(flag,"noloot")) {
		if( state && map->list[m].flag.nomobloot )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noloot\toff");
			else if( map->list[m].flag.nomobloot )
				map_zone_mf_cache_add(m,"noloot");
		}
	} else if (!strcmpi(flag,"nomobloot")) {
		if( state && map->list[m].flag.nomobloot )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomobloot\toff");
			else if( map->list[m].flag.nomobloot )
				map_zone_mf_cache_add(m,"nomobloot");
		}
	} else if (!strcmpi(flag,"nomvploot")) {
		if( state && map->list[m].flag.nomvploot )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomvploot\toff");
			else if( map->list[m].flag.nomvploot )
				map_zone_mf_cache_add(m,"nomvploot");
		}
	} else if (!strcmpi(flag,"nocommand")) {
		/* implementation may be incomplete */
		if( state && sscanf(params, "%d", &state) == 1 ) {
			sprintf(rflag, "nocommand\t%s",params);
			map_zone_mf_cache_add(m,rflag);
		} else if( !state && map->list[m].nocommand ) {
			sprintf(rflag, "nocommand\t%d",map->list[m].nocommand);
			map_zone_mf_cache_add(m,rflag);
		} else if( map->list[m].nocommand ) {
			map_zone_mf_cache_add(m,"nocommand\toff");
		}
	} else if (!strcmpi(flag,"jexp")) {
		if( !state ) {
			if( map->list[m].jexp != 100 ) {
				sprintf(rflag,"jexp\t%d",map->list[m].jexp);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].jexp ) {
				sprintf(rflag,"jexp\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if (!strcmpi(flag,"bexp")) {
		if( !state ) {
			if( map->list[m].bexp != 100 ) {
				sprintf(rflag,"bexp\t%d",map->list[m].jexp);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].bexp ) {
				sprintf(rflag,"bexp\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if (!strcmpi(flag,"loadevent")) {
		if( state && map->list[m].flag.loadevent )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"loadevent\toff");
			else if( map->list[m].flag.loadevent )
				map_zone_mf_cache_add(m,"loadevent");
		}
	} else if (!strcmpi(flag,"nochat")) {
		if( state && map->list[m].flag.nochat )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nochat\toff");
			else if( map->list[m].flag.nochat )
				map_zone_mf_cache_add(m,"nochat");
		}
	} else if (!strcmpi(flag,"partylock")) {
		if( state && map->list[m].flag.partylock )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"partylock\toff");
			else if( map->list[m].flag.partylock )
				map_zone_mf_cache_add(m,"partylock");
		}
	} else if (!strcmpi(flag,"guildlock")) {
		if( state && map->list[m].flag.guildlock )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"guildlock\toff");
			else if( map->list[m].flag.guildlock )
				map_zone_mf_cache_add(m,"guildlock");
		}
	} else if (!strcmpi(flag,"reset")) {
		if( state && map->list[m].flag.reset )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"reset\toff");
			else if( map->list[m].flag.reset )
				map_zone_mf_cache_add(m,"reset");
		}
	} else if (!strcmpi(flag,"adjust_unit_duration")) {
		int skill_id, k;
		char skill_name[MAP_ZONE_MAPFLAG_LENGTH], modifier[MAP_ZONE_MAPFLAG_LENGTH];
		int len = strlen(params);

		modifier[0] = '\0';
		memcpy(skill_name, params, MAP_ZONE_MAPFLAG_LENGTH);

		for(k = 0; k < len; k++) {
			if( skill_name[k] == '\t' ) {
				memcpy(modifier, &skill_name[k+1], len - k);
				skill_name[k] = '\0';
				break;
			}
		}

		if( modifier[0] == '\0' || !( skill_id = skill->name2id(skill_name) ) || !skill->get_unit_id( skill->name2id(skill_name), 0) || atoi(modifier) < 1 || atoi(modifier) > USHRT_MAX ) {
			;/* we dont mind it, the server will take care of it next. */
		} else {
			int idx = map->list[m].unit_count;

			k = 0;
			ARR_FIND(0, idx, k, map->list[m].units[k]->skill_id == skill_id);

			if( k < idx ) {
				if( atoi(modifier) != map->list[m].units[k]->modifier ) {
					sprintf(rflag,"adjust_unit_duration\t%s\t%d",skill_name,map->list[m].units[k]->modifier);
					map_zone_mf_cache_add(m,rflag);
				}
			} else {
				sprintf(rflag,"adjust_unit_duration\t%s\t100",skill_name);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if (!strcmpi(flag,"adjust_skill_damage")) {
		int skill_id, k;
		char skill_name[MAP_ZONE_MAPFLAG_LENGTH], modifier[MAP_ZONE_MAPFLAG_LENGTH];
		int len = strlen(params);

		modifier[0] = '\0';
		memcpy(skill_name, params, MAP_ZONE_MAPFLAG_LENGTH);

		for(k = 0; k < len; k++) {
			if( skill_name[k] == '\t' ) {
				memcpy(modifier, &skill_name[k+1], len - k);
				skill_name[k] = '\0';
				break;
			}
		}

		if( modifier[0] == '\0' || !( skill_id = skill->name2id(skill_name) ) || atoi(modifier) < 1 || atoi(modifier) > USHRT_MAX ) {
			;/* we dont mind it, the server will take care of it next. */
		} else {
			int idx = map->list[m].skill_count;

			k = 0;
			ARR_FIND(0, idx, k, map->list[m].skills[k]->skill_id == skill_id);

			if( k < idx ) {
				if( atoi(modifier) != map->list[m].skills[k]->modifier ) {
					sprintf(rflag,"adjust_skill_damage\t%s\t%d",skill_name,map->list[m].skills[k]->modifier);
					map_zone_mf_cache_add(m,rflag);
				}
			} else {
				sprintf(rflag,"adjust_skill_damage\t%s\t100",skill_name);
				map_zone_mf_cache_add(m,rflag);
			}

		}
	} else if (!strcmpi(flag,"zone")) {
		ShowWarning("You can't add a zone through a zone! ERROR, skipping for '%s'...\n",map->list[m].name);
		return true;
	} else if ( !strcmpi(flag,"nomapchannelautojoin") ) {
		if( state && map->list[m].flag.chsysnolocalaj )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomapchannelautojoin\toff");
			else if( map->list[m].flag.chsysnolocalaj )
				map_zone_mf_cache_add(m,"nomapchannelautojoin");
		}
	} else if ( !strcmpi(flag,"invincible_time_inc") ) {
		if( !state ) {
			if( map->list[m].invincible_time_inc != 0 ) {
				sprintf(rflag,"invincible_time_inc\t%d",map->list[m].invincible_time_inc);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].invincible_time_inc ) {
				sprintf(rflag,"invincible_time_inc\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"noknockback") ) {
		if( state && map->list[m].flag.noknockback )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noknockback\toff");
			else if( map->list[m].flag.noknockback )
				map_zone_mf_cache_add(m,"noknockback");
		}
	} else if ( !strcmpi(flag,"weapon_damage_rate") ) {
		if( !state ) {
			if( map->list[m].weapon_damage_rate != 100 ) {
				sprintf(rflag,"weapon_damage_rate\t%d",map->list[m].weapon_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].weapon_damage_rate ) {
				sprintf(rflag,"weapon_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"magic_damage_rate") ) {
		if( !state ) {
			if( map->list[m].magic_damage_rate != 100 ) {
				sprintf(rflag,"magic_damage_rate\t%d",map->list[m].magic_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].magic_damage_rate ) {
				sprintf(rflag,"magic_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"misc_damage_rate") ) {
		if( !state ) {
			if( map->list[m].misc_damage_rate != 100 ) {
				sprintf(rflag,"misc_damage_rate\t%d",map->list[m].misc_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].misc_damage_rate ) {
				sprintf(rflag,"misc_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"short_damage_rate") ) {
		if( !state ) {
			if( map->list[m].short_damage_rate != 100 ) {
				sprintf(rflag,"short_damage_rate\t%d",map->list[m].short_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].short_damage_rate ) {
				sprintf(rflag,"short_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"long_damage_rate") ) {
		if( !state ) {
			if( map->list[m].long_damage_rate != 100 ) {
				sprintf(rflag,"long_damage_rate\t%d",map->list[m].long_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != map->list[m].long_damage_rate ) {
				sprintf(rflag,"long_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if (!strcmpi(flag,"nocashshop")) {
		if( state && map->list[m].flag.nocashshop )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nocashshop\toff");
			else if( map->list[m].flag.nocashshop )
				map_zone_mf_cache_add(m,"nocashshop");
		}
	}

	return false;
}
void map_zone_apply(int m, struct map_zone_data *zone, const char* start, const char* buffer, const char* filepath) {
	int i;
	char empty[1] = "\0";
	char flag[MAP_ZONE_MAPFLAG_LENGTH], params[MAP_ZONE_MAPFLAG_LENGTH];
	map->list[m].zone = zone;
	for(i = 0; i < zone->mapflags_count; i++) {
		int len = strlen(zone->mapflags[i]);
		int k;
		params[0] = '\0';
		memcpy(flag, zone->mapflags[i], MAP_ZONE_MAPFLAG_LENGTH);
		for(k = 0; k < len; k++) {
			if( flag[k] == '\t' ) {
				memcpy(params, &flag[k+1], len - k);
				flag[k] = '\0';
				break;
			}
		}

		if( map->zone_mf_cache(m,flag,params) )
			continue;

		npc->parse_mapflag(map->list[m].name,empty,flag,params,start,buffer,filepath);
	}
}
/* used on npc load and reload to apply all "Normal" and "PK Mode" zones */
void map_zone_init(void) {
	char flag[MAP_ZONE_MAPFLAG_LENGTH], params[MAP_ZONE_MAPFLAG_LENGTH];
	struct map_zone_data *zone;
	char empty[1] = "\0";
	int i,k,j;

	zone = &map->zone_all;

	for(i = 0; i < zone->mapflags_count; i++) {
		int len = strlen(zone->mapflags[i]);
		params[0] = '\0';
		memcpy(flag, zone->mapflags[i], MAP_ZONE_MAPFLAG_LENGTH);
		for(k = 0; k < len; k++) {
			if( flag[k] == '\t' ) {
				memcpy(params, &flag[k+1], len - k);
				flag[k] = '\0';
				break;
			}
		}

		for(j = 0; j < map->count; j++) {
			if( map->list[j].zone == zone ) {
				if( map->zone_mf_cache(j,flag,params) )
					break;
				npc->parse_mapflag(map->list[j].name,empty,flag,params,empty,empty,empty);
			}
		}
	}

	if( battle_config.pk_mode ) {
		zone = &map->zone_pk;
		for(i = 0; i < zone->mapflags_count; i++) {
			int len = strlen(zone->mapflags[i]);
			params[0] = '\0';
			memcpy(flag, zone->mapflags[i], MAP_ZONE_MAPFLAG_LENGTH);
			for(k = 0; k < len; k++) {
				if( flag[k] == '\t' ) {
					memcpy(params, &flag[k+1], len - k);
					flag[k] = '\0';
					break;
				}
			}
			for(j = 0; j < map->count; j++) {
				if( map->list[j].zone == zone ) {
					if( map->zone_mf_cache(j,flag,params) )
						break;
					npc->parse_mapflag(map->list[j].name,empty,flag,params,empty,empty,empty);
				}
			}
		}
	}

}
unsigned short map_zone_str2itemid(const char *name) {
	struct item_data *data;

	if( !name )
		return 0;
	if( name[0] == 'I' && name[1] == 'D' && strlen(name) < 8 ) {
		if( !( data = itemdb->exists(atoi(name+2))) ) {
			return 0;
		}
	} else {
		if( !( data = itemdb->search_name(name) ) ) {
			return 0;
		}
	}
	return data->nameid;
}
unsigned short map_zone_str2skillid(const char *name) {
	unsigned short nameid = 0;

	if( !name )
		return 0;

	if( name[0] == 'I' && name[1] == 'D' && strlen(name) < 8 ) {
		if( !skill->get_index((nameid = atoi(name+2))) )
			return 0;
	} else {
		if( !( nameid = strdb_iget(skill->name2id_db, name) ) ) {
			return 0;
		}
	}
	return nameid;
}
enum bl_type map_zone_bl_type(const char *entry, enum map_zone_skill_subtype *subtype) {
	char temp[200], *parse;
	enum bl_type bl = BL_NUL;
	*subtype = MZS_NONE;

	if( !entry )
		return BL_NUL;

	safestrncpy(temp, entry, 200);

	parse = strtok(temp,"|");

	while (parse != NULL) {
		normalize_name(parse," ");
		if( strcmpi(parse,"player") == 0 )
			bl |= BL_PC;
		else if( strcmpi(parse,"homun") == 0 )
			bl |= BL_HOM;
		else if( strcmpi(parse,"mercenary") == 0 )
			bl |= BL_MER;
		else if( strcmpi(parse,"monster") == 0 )
			bl |= BL_MOB;
		else if( strcmpi(parse,"clone") == 0 ) {
			bl |= BL_MOB;
			*subtype |= MZS_CLONE;
		} else if( strcmpi(parse,"mob_boss") == 0 ) {
			bl |= BL_MOB;
			*subtype |= MZS_BOSS;
		} else if( strcmpi(parse,"elemental") == 0 )
			bl |= BL_ELEM;
		else if( strcmpi(parse,"pet") == 0 )
			bl |= BL_PET;
		else if( strcmpi(parse,"all") == 0 ) {
			bl |= BL_ALL;
			*subtype |= MZS_ALL;
		} else if( strcmpi(parse,"none") == 0 ) {
			bl = BL_NUL;
		} else {
			ShowError("map_zone_db: '%s' unknown type, skipping...\n",parse);
		}
		parse = strtok(NULL,"|");
	}
	return bl;
}
/* [Ind/Hercules] */
void read_map_zone_db(void) {
	config_t map_zone_db;
	config_setting_t *zones = NULL;
	/* TODO: #ifndef required for re/pre-re */
#ifdef RENEWAL
	const char *config_filename = "db/re/map_zone_db.conf"; // FIXME hardcoded name
#else
	const char *config_filename = "db/pre-re/map_zone_db.conf"; // FIXME hardcoded name
#endif
	if (conf_read_file(&map_zone_db, config_filename))
		return;

	zones = config_lookup(&map_zone_db, "zones");

	if (zones != NULL) {
		struct map_zone_data *zone;
		config_setting_t *zone_e;
		config_setting_t *skills;
		config_setting_t *items;
		config_setting_t *mapflags;
		config_setting_t *commands;
		config_setting_t *caps;
		const char *name;
		const char *zonename;
		int i,h,v;
		int zone_count = 0, disabled_skills_count = 0, disabled_items_count = 0, mapflags_count = 0,
			disabled_commands_count = 0, capped_skills_count = 0;
		enum map_zone_skill_subtype subtype;

		zone_count = config_setting_length(zones);
		for (i = 0; i < zone_count; ++i) {
			bool is_all = false;

			zone_e = config_setting_get_elem(zones, i);

			if (!config_setting_lookup_string(zone_e, "name", &zonename)) {
				ShowError("map_zone_db: missing zone name, skipping... (%s:%d)\n",
					config_setting_source_file(zone_e), config_setting_source_line(zone_e));
				config_setting_remove_elem(zones,i);/* remove from the tree */
				--zone_count;
				--i;
				continue;
			}

			if( strdb_exists(map->zone_db, zonename) ) {
				ShowError("map_zone_db: duplicate zone name '%s', skipping...\n",zonename);
				config_setting_remove_elem(zones,i);/* remove from the tree */
				--zone_count;
				--i;
				continue;
			}

			/* is this the global template? */
			if( strncmpi(zonename,MAP_ZONE_NORMAL_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
				zone = &map->zone_all;
				is_all = true;
			} else if( strncmpi(zonename,MAP_ZONE_PK_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
				zone = &map->zone_pk;
				is_all = true;
			} else {
				CREATE( zone, struct map_zone_data, 1 );
				zone->disabled_skills_count = 0;
				zone->disabled_items_count  = 0;
			}
			safestrncpy(zone->name, zonename, MAP_ZONE_NAME_LENGTH);

			if( (skills = config_setting_get_member(zone_e, "disabled_skills")) != NULL ) {
				disabled_skills_count = config_setting_length(skills);
				/* validate */
				for(h = 0; h < config_setting_length(skills); h++) {
					config_setting_t *skillinfo = config_setting_get_elem(skills, h);
					name = config_setting_name(skillinfo);
					if( !map->zone_str2skillid(name) ) {
						ShowError("map_zone_db: unknown skill (%s) in disabled_skills for zone '%s', skipping skill...\n",name,zone->name);
						config_setting_remove_elem(skills,h);
						--disabled_skills_count;
						--h;
						continue;
					}
					if( !map->zone_bl_type(config_setting_get_string_elem(skills,h),&subtype) )/* we dont remove it from the three due to inheritance */
						--disabled_skills_count;
				}
				/* all ok, process */
				CREATE( zone->disabled_skills, struct map_zone_disabled_skill_entry *, disabled_skills_count );
				for(h = 0, v = 0; h < config_setting_length(skills); h++) {
					config_setting_t *skillinfo = config_setting_get_elem(skills, h);
					struct map_zone_disabled_skill_entry * entry;
					enum bl_type type;
					name = config_setting_name(skillinfo);

					if( (type = map->zone_bl_type(config_setting_get_string_elem(skills,h),&subtype)) ) { /* only add if enabled */
						CREATE( entry, struct map_zone_disabled_skill_entry, 1 );

						entry->nameid = map->zone_str2skillid(name);
						entry->type = type;
						entry->subtype = subtype;

						zone->disabled_skills[v++] = entry;
					}

				}
				zone->disabled_skills_count = disabled_skills_count;
			}

			if( (items = config_setting_get_member(zone_e, "disabled_items")) != NULL ) {
				disabled_items_count = config_setting_length(items);
				/* validate */
				for(h = 0; h < config_setting_length(items); h++) {
					config_setting_t *item = config_setting_get_elem(items, h);
					name = config_setting_name(item);
					if( !map->zone_str2itemid(name) ) {
						ShowError("map_zone_db: unknown item (%s) in disabled_items for zone '%s', skipping item...\n",name,zone->name);
						config_setting_remove_elem(items,h);
						--disabled_items_count;
						--h;
						continue;
					}
					if( !config_setting_get_bool(item) )/* we dont remove it from the three due to inheritance */
						--disabled_items_count;
				}
				/* all ok, process */
				CREATE( zone->disabled_items, int, disabled_items_count );
				for(h = 0, v = 0; h < config_setting_length(items); h++) {
					config_setting_t *item = config_setting_get_elem(items, h);

					if( config_setting_get_bool(item) ) { /* only add if enabled */
						name = config_setting_name(item);
						zone->disabled_items[v++] = map->zone_str2itemid(name);
					}

				}
				zone->disabled_items_count = disabled_items_count;
			}

			if( (mapflags = config_setting_get_member(zone_e, "mapflags")) != NULL ) {
				mapflags_count = config_setting_length(mapflags);
				/* mapflags are not validated here, so we save all anyway */
				CREATE( zone->mapflags, char *, mapflags_count );
				for(h = 0; h < mapflags_count; h++) {
					CREATE( zone->mapflags[h], char, MAP_ZONE_MAPFLAG_LENGTH );

					name = config_setting_get_string_elem(mapflags, h);

					safestrncpy(zone->mapflags[h], name, MAP_ZONE_MAPFLAG_LENGTH);

				}
				zone->mapflags_count = mapflags_count;
			}

			if( (commands = config_setting_get_member(zone_e, "disabled_commands")) != NULL ) {
				disabled_commands_count = config_setting_length(commands);
				/* validate */
				for(h = 0; h < config_setting_length(commands); h++) {
					config_setting_t *command = config_setting_get_elem(commands, h);
					name = config_setting_name(command);
					if( !atcommand->exists(name) ) {
						ShowError("map_zone_db: unknown command '%s' in disabled_commands for zone '%s', skipping entry...\n",name,zone->name);
						config_setting_remove_elem(commands,h);
						--disabled_commands_count;
						--h;
						continue;
					}
					if( !config_setting_get_int(command) )/* we dont remove it from the three due to inheritance */
						--disabled_commands_count;
				}
				/* all ok, process */
				CREATE( zone->disabled_commands, struct map_zone_disabled_command_entry *, disabled_commands_count );
				for(h = 0, v = 0; h < config_setting_length(commands); h++) {
					config_setting_t *command = config_setting_get_elem(commands, h);
					struct map_zone_disabled_command_entry * entry;
					int group_lv;
					name = config_setting_name(command);

					if( (group_lv = config_setting_get_int(command)) ) { /* only add if enabled */
						CREATE( entry, struct map_zone_disabled_command_entry, 1 );

						entry->cmd  = atcommand->exists(name)->func;
						entry->group_lv = group_lv;

						zone->disabled_commands[v++] = entry;
					}
				}
				zone->disabled_commands_count = disabled_commands_count;
			}

			if( (caps = config_setting_get_member(zone_e, "skill_damage_cap")) != NULL ) {
				capped_skills_count = config_setting_length(caps);
				/* validate */
				for(h = 0; h < config_setting_length(caps); h++) {
					config_setting_t *cap = config_setting_get_elem(caps, h);
					name = config_setting_name(cap);
					if( !map->zone_str2skillid(name) ) {
						ShowError("map_zone_db: unknown skill (%s) in skill_damage_cap for zone '%s', skipping skill...\n",name,zone->name);
						config_setting_remove_elem(caps,h);
						--capped_skills_count;
						--h;
						continue;
					}
					if( !map->zone_bl_type(config_setting_get_string_elem(cap,1),&subtype) )/* we dont remove it from the three due to inheritance */
						--capped_skills_count;
				}
				/* all ok, process */
				CREATE( zone->capped_skills, struct map_zone_skill_damage_cap_entry *, capped_skills_count );
				for(h = 0, v = 0; h < config_setting_length(caps); h++) {
					config_setting_t *cap = config_setting_get_elem(caps, h);
					struct map_zone_skill_damage_cap_entry * entry;
					enum bl_type type;
					name = config_setting_name(cap);

					if( (type = map->zone_bl_type(config_setting_get_string_elem(cap,1),&subtype)) ) { /* only add if enabled */
						CREATE( entry, struct map_zone_skill_damage_cap_entry, 1 );

						entry->nameid = map->zone_str2skillid(name);
						entry->cap = config_setting_get_int_elem(cap,0);
						entry->type = type;
						entry->subtype = subtype;
						zone->capped_skills[v++] = entry;
					}
				}
				zone->capped_skills_count = capped_skills_count;
			}

			if( !is_all ) /* global template doesn't go into db -- since it isn't a alloc'd piece of data */
				strdb_put(map->zone_db, zonename, zone);

		}

		/* process inheritance, aka loop through the whole thing again :P */
		for (i = 0; i < zone_count; ++i) {
			config_setting_t *inherit_tree = NULL;
			config_setting_t *new_entry = NULL;
			int inherit_count;

			zone_e = config_setting_get_elem(zones, i);
			config_setting_lookup_string(zone_e, "name", &zonename);

			if( strncmpi(zonename,MAP_ZONE_ALL_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
				continue;/* all zone doesn't inherit anything (if it did, everything would link to each other and boom endless loop) */
			}

			if( (inherit_tree = config_setting_get_member(zone_e, "inherit")) != NULL ) {
				/* append global zone to this */
				new_entry = config_setting_add(inherit_tree,MAP_ZONE_ALL_NAME,CONFIG_TYPE_STRING);
				config_setting_set_string(new_entry,MAP_ZONE_ALL_NAME);
			} else {
				/* create inherit member and add global zone to it */
				inherit_tree = config_setting_add(zone_e, "inherit",CONFIG_TYPE_ARRAY);
				new_entry = config_setting_add(inherit_tree,MAP_ZONE_ALL_NAME,CONFIG_TYPE_STRING);
				config_setting_set_string(new_entry,MAP_ZONE_ALL_NAME);
			}
			inherit_count = config_setting_length(inherit_tree);
			for(h = 0; h < inherit_count; h++) {
				struct map_zone_data *izone; /* inherit zone */
				int disabled_skills_count_i = 0; /* disabled skill count from inherit zone */
				int disabled_items_count_i = 0; /* disabled item count from inherit zone */
				int mapflags_count_i = 0; /* mapflag count from inherit zone */
				int disabled_commands_count_i = 0; /* commands count from inherit zone */
				int capped_skills_count_i = 0; /* skill capped count from inherit zone */
				int j;

				name = config_setting_get_string_elem(inherit_tree, h);
				config_setting_lookup_string(zone_e, "name", &zonename);/* will succeed for we validated it earlier */

				if( !(izone = strdb_get(map->zone_db, name)) ) {
					ShowError("map_zone_db: Unknown zone '%s' being inherit by zone '%s', skipping...\n",name,zonename);
					continue;
				}

				if( strncmpi(zonename,MAP_ZONE_NORMAL_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
					zone = &map->zone_all;
				} else if( strncmpi(zonename,MAP_ZONE_PK_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
					zone = &map->zone_pk;
				} else
					zone = strdb_get(map->zone_db, zonename);/* will succeed for we just put it in here */

				disabled_skills_count_i = izone->disabled_skills_count;
				disabled_items_count_i = izone->disabled_items_count;
				mapflags_count_i = izone->mapflags_count;
				disabled_commands_count_i = izone->disabled_commands_count;
				capped_skills_count_i = izone->capped_skills_count;

				/* process everything to override, paying attention to config_setting_get_bool */
				if( disabled_skills_count_i ) {
					if( (skills = config_setting_get_member(zone_e, "disabled_skills")) == NULL )
						skills = config_setting_add(zone_e, "disabled_skills",CONFIG_TYPE_GROUP);
					disabled_skills_count = config_setting_length(skills);
					for(j = 0; j < disabled_skills_count_i; j++) {
						int k;
						for(k = 0; k < disabled_skills_count; k++) {
							config_setting_t *skillinfo = config_setting_get_elem(skills, k);
							if( map->zone_str2skillid(config_setting_name(skillinfo)) == izone->disabled_skills[j]->nameid ) {
								break;
							}
						}
						if( k == disabled_skills_count ) {/* we didn't find it */
							struct map_zone_disabled_skill_entry *entry;
							RECREATE( zone->disabled_skills, struct map_zone_disabled_skill_entry *, ++zone->disabled_skills_count );
							CREATE( entry, struct map_zone_disabled_skill_entry, 1 );
							entry->nameid = izone->disabled_skills[j]->nameid;
							entry->type = izone->disabled_skills[j]->type;
							zone->disabled_skills[zone->disabled_skills_count-1] = entry;
						}
					}
				}

				if( disabled_items_count_i ) {
					if( (items = config_setting_get_member(zone_e, "disabled_items")) == NULL )
						items = config_setting_add(zone_e, "disabled_items",CONFIG_TYPE_GROUP);
					disabled_items_count = config_setting_length(items);
					for(j = 0; j < disabled_items_count_i; j++) {
						int k;
						for(k = 0; k < disabled_items_count; k++) {
							config_setting_t *item = config_setting_get_elem(items, k);

							name = config_setting_name(item);

							if( map->zone_str2itemid(name) == izone->disabled_items[j] ) {
								if( config_setting_get_bool(item) )
									continue;
								break;
							}
						}
						if( k == disabled_items_count ) {/* we didn't find it */
							RECREATE( zone->disabled_items, int, ++zone->disabled_items_count );
							zone->disabled_items[zone->disabled_items_count-1] = izone->disabled_items[j];
						}
					}
				}

				if( mapflags_count_i ) {
					if( (mapflags = config_setting_get_member(zone_e, "mapflags")) == NULL )
						mapflags = config_setting_add(zone_e, "mapflags",CONFIG_TYPE_ARRAY);
					mapflags_count = config_setting_length(mapflags);
					for(j = 0; j < mapflags_count_i; j++) {
						int k;
						for(k = 0; k < mapflags_count; k++) {
							name = config_setting_get_string_elem(mapflags, k);

							if( strcmpi(name,izone->mapflags[j]) == 0 ) {
								break;
							}
						}
						if( k == mapflags_count ) {/* we didn't find it */
							RECREATE( zone->mapflags, char*, ++zone->mapflags_count );
							CREATE( zone->mapflags[zone->mapflags_count-1], char, MAP_ZONE_MAPFLAG_LENGTH );
							safestrncpy(zone->mapflags[zone->mapflags_count-1], izone->mapflags[j], MAP_ZONE_MAPFLAG_LENGTH);
						}
					}
				}

				if( disabled_commands_count_i ) {
					if( (commands = config_setting_get_member(zone_e, "disabled_commands")) == NULL )
						commands = config_setting_add(zone_e, "disabled_commands",CONFIG_TYPE_GROUP);

					disabled_commands_count = config_setting_length(commands);
					for(j = 0; j < disabled_commands_count_i; j++) {
						int k;
						for(k = 0; k < disabled_commands_count; k++) {
							config_setting_t *command = config_setting_get_elem(commands, k);
							if( atcommand->exists(config_setting_name(command))->func == izone->disabled_commands[j]->cmd ) {
								break;
							}
						}
						if( k == disabled_commands_count ) {/* we didn't find it */
							struct map_zone_disabled_command_entry *entry;
							RECREATE( zone->disabled_commands, struct map_zone_disabled_command_entry *, ++zone->disabled_commands_count );
							CREATE( entry, struct map_zone_disabled_command_entry, 1 );
							entry->cmd = izone->disabled_commands[j]->cmd;
							entry->group_lv = izone->disabled_commands[j]->group_lv;
							zone->disabled_commands[zone->disabled_commands_count-1] = entry;
						}
					}
				}

				if( capped_skills_count_i ) {
					if( (caps = config_setting_get_member(zone_e, "skill_damage_cap")) == NULL )
						caps = config_setting_add(zone_e, "skill_damage_cap",CONFIG_TYPE_GROUP);

					capped_skills_count = config_setting_length(caps);
					for(j = 0; j < capped_skills_count_i; j++) {
						int k;
						for(k = 0; k < capped_skills_count; k++) {
							config_setting_t *cap = config_setting_get_elem(caps, k);
							if( map->zone_str2skillid(config_setting_name(cap)) == izone->capped_skills[j]->nameid ) {
								break;
							}
						}
						if( k == capped_skills_count ) {/* we didn't find it */
							struct map_zone_skill_damage_cap_entry *entry;
							RECREATE( zone->capped_skills, struct map_zone_skill_damage_cap_entry *, ++zone->capped_skills_count );
							CREATE( entry, struct map_zone_skill_damage_cap_entry, 1 );
							entry->nameid = izone->capped_skills[j]->nameid;
							entry->cap = izone->capped_skills[j]->cap;
							entry->type = izone->capped_skills[j]->type;
							zone->capped_skills[zone->capped_skills_count-1] = entry;
						}
					}
				}

			}
		}

		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' zones in '"CL_WHITE"%s"CL_RESET"'.\n", zone_count, config_filename);
		/* not supposed to go in here but in skill_final whatever */
		config_destroy(&map_zone_db);
	}
}

int map_get_new_bonus_id (void) {
	return map->bonus_id++;
}

void map_add_questinfo(int m, struct questinfo *qi) {
	unsigned short i;
	
	/* duplicate, override */
	for(i = 0; i < map->list[m].qi_count; i++) {
		if( map->list[m].qi_data[i].nd == qi->nd )
			break;
	}
		
	if( i == map->list[m].qi_count )
		RECREATE(map->list[m].qi_data, struct questinfo, ++map->list[m].qi_count);
	
	memcpy(&map->list[m].qi_data[i], qi, sizeof(struct questinfo));
}

bool map_remove_questinfo(int m, struct npc_data *nd) {
	unsigned short i;
	
	for(i = 0; i < map->list[m].qi_count; i++) {
		struct questinfo *qi = &map->list[m].qi_data[i];
		if( qi->nd == nd ) {
			memset(&map->list[m].qi_data[i], 0, sizeof(struct questinfo));
			if( i != --map->list[m].qi_count ) {
				memmove(&map->list[m].qi_data[i],&map->list[m].qi_data[i+1],sizeof(struct questinfo)*(map->list[m].qi_count-i));
			}
			return true;
		}
	}
	
	return false;
}

/**
 * @see DBApply
 */
int map_db_final(DBKey key, DBData *data, va_list ap) {
	struct map_data_other_server *mdos = DB->data2ptr(data);

	if(mdos && iMalloc->verify_ptr(mdos) && mdos->cell == NULL)
		aFree(mdos);

	return 0;
}

/**
 * @see DBApply
 */
int nick_db_final(DBKey key, DBData *data, va_list args)
{
	struct charid2nick* p = DB->data2ptr(data);
	struct charid_request* req;

	if( p == NULL )
		return 0;
	while( p->requests )
	{
		req = p->requests;
		p->requests = req->next;
		aFree(req);
	}
	aFree(p);
	return 0;
}

int cleanup_sub(struct block_list *bl, va_list ap) {
	nullpo_ret(bl);

	switch(bl->type) {
		case BL_PC:
			map->quit((struct map_session_data *) bl);
			break;
		case BL_NPC:
			npc->unload((struct npc_data *)bl,false);
			break;
		case BL_MOB:
			unit->free(bl,CLR_OUTSIGHT);
			break;
		case BL_PET:
			//There is no need for this, the pet is removed together with the player. [Skotlex]
			break;
		case BL_ITEM:
			map->clearflooritem(bl);
			break;
		case BL_SKILL:
			skill->delunit((struct skill_unit *) bl);
			break;
	}

	return 1;
}

/**
 * @see DBApply
 */
int cleanup_db_sub(DBKey key, DBData *data, va_list va) {
	return map->cleanup_sub(DB->data2ptr(data), va);
}

/*==========================================
 * map destructor
 *------------------------------------------*/
void do_final(void)
{
	int i;
	struct map_session_data* sd;
	struct s_mapiterator* iter;

	ShowStatus("Terminating...\n");
	
	hChSys.closing = true;
	HPM->event(HPET_FINAL);
	
	if (map->cpsd) aFree(map->cpsd);

	//Ladies and babies first.
	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
		map->quit(sd);
	mapit->free(iter);

	/* prepares npcs for a faster shutdown process */
	npc->do_clear_npc();

	// remove all objects on maps
	for (i = 0; i < map->count; i++) {
		ShowStatus("Cleaning up maps [%d/%d]: %s..."CL_CLL"\r", i+1, map->count, map->list[i].name);
		if (map->list[i].m >= 0)
			map->foreachinmap(map->cleanup_sub, i, BL_ALL);
	}
	ShowStatus("Cleaned up %d maps."CL_CLL"\n", map->count);

	map->id_db->foreach(map->id_db,map->cleanup_db_sub);
	chrif->char_reset_offline();
	chrif->flush_fifo();

	atcommand->final();
	battle->final();
	chrif->final();
	ircbot->final();/* before clif. */
	clif->final();
	npc->final();
	quest->final();
	script->final();
	itemdb->final();
	instance->final();
	gstorage->final();
	guild->final();
	party->final();
	pc->final();
	pet->final();
	mob->final();
	homun->final();
	atcommand->final_msg();
	skill->final();
	status->final();
	unit->final();
	bg->final();
	duel->final();
	elemental->final();
	map->list_final();
	vending->final();

	HPM_map_do_final();
	
	map->map_db->destroy(map->map_db, map->db_final);

	mapindex_final();
	if(map->enable_grf)
		grfio_final();

	db_destroy(map->id_db);
	db_destroy(map->pc_db);
	db_destroy(map->mobid_db);
	db_destroy(map->bossid_db);
	map->nick_db->destroy(map->nick_db, map->nick_db_final);
	db_destroy(map->charid_db);
	db_destroy(map->iwall_db);
	db_destroy(map->regen_db);

	map->sql_close();
	ers_destroy(map->iterator_ers);
	ers_destroy(map->flooritem_ers);

	aFree(map->list);

	if( !map->enable_grf )
		aFree(map->cache_buffer);

	HPM->event(HPET_POST_FINAL);
	
	ShowStatus("Finished.\n");
}

int map_abort_sub(struct map_session_data* sd, va_list ap) {
	chrif->save(sd,1);
	return 1;
}


//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
	static int run = 0;
	//Save all characters and then flush the inter-connection.
	if (run) {
		ShowFatalError("Server has crashed while trying to save characters. Character data can't be saved!\n");
		return;
	}
	run = 1;
	if (!chrif->isconnected())
	{
		if (db_size(map->pc_db))
			ShowFatalError("Server has crashed without a connection to the char-server, %u characters can't be saved!\n", db_size(map->pc_db));
		return;
	}
	ShowError("Server received crash signal! Attempting to save all online characters!\n");
	map->foreachpc(map->abort_sub);
	chrif->flush_fifo();
}

/*======================================================
* Map-Server Version Screen [MC Cameri]
*------------------------------------------------------*/
void map_helpscreen(bool do_exit)
{
	ShowInfo("Usage: %s [options]\n", SERVER_NAME);
	ShowInfo("\n");
	ShowInfo("Options:\n");
	ShowInfo("  -?, -h [--help]           Displays this help screen.\n");
	ShowInfo("  -v [--version]            Displays the server's version.\n");
	ShowInfo("  --run-once                Closes server after loading (testing).\n");
	ShowInfo("  --map-config <file>       Alternative map-server configuration.\n");
	ShowInfo("  --battle-config <file>    Alternative battle configuration.\n");
	ShowInfo("  --atcommand-config <file> Alternative atcommand configuration.\n");
	ShowInfo("  --script-config <file>    Alternative script configuration.\n");
	ShowInfo("  --msg-config <file>       Alternative message configuration.\n");
	ShowInfo("  --grf-path <file>         Alternative GRF path configuration.\n");
	ShowInfo("  --inter-config <file>     Alternative inter-server configuration.\n");
	ShowInfo("  --log-config <file>       Alternative logging configuration.\n");
	ShowInfo("  --script-check <file>     Tests a script for errors, without running the server.\n");
	HPM->arg_help();/* display help for commands implemented thru HPM */
	if( do_exit )
		exit(EXIT_SUCCESS);
}

/*======================================================
 * Map-Server Version Screen [MC Cameri]
 *------------------------------------------------------*/
void map_versionscreen(bool do_exit) {
	const char *svn = get_svn_revision();
	const char *git = get_git_hash();
	ShowInfo(CL_WHITE"Hercules version: %s" CL_RESET"\n", git[0] != HERC_UNKNOWN_VER ? git : svn[0] != HERC_UNKNOWN_VER ? svn : "Unknown");
	ShowInfo(CL_GREEN"Website/Forum:"CL_RESET"\thttp://hercules.ws/\n");
	ShowInfo(CL_GREEN"IRC Channel:"CL_RESET"\tirc://irc.rizon.net/#Hercules\n");
	ShowInfo("Open "CL_WHITE"readme.txt"CL_RESET" for more information.\n");
	if( do_exit )
		exit(EXIT_SUCCESS);
}

void set_server_type(void) {
	SERVER_TYPE = SERVER_TYPE_MAP;
}


/// Called when a terminate signal is received.
void do_shutdown(void)
{
	if( runflag != MAPSERVER_ST_SHUTDOWN )
	{
		runflag = MAPSERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		{
			struct map_session_data* sd;
			struct s_mapiterator* iter = mapit_getallusers();
			for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
				clif->GM_kick(NULL, sd);
			mapit->free(iter);
			flush_fifos();
		}
		chrif->check_shutdown();
	}
}

bool map_arg_next_value(const char* option, int i, int argc, bool must)
{
	if( i >= argc-1 ) {
		if( must )
			ShowWarning("Missing value for option '%s'.\n", option);
		return false;
	}

	return true;
}

CPCMD(gm_position) {
	int x = 0, y = 0, m = 0;
	char map_name[25];

	if( line == NULL || sscanf(line, "%d %d %24s",&x,&y,map_name) < 3 ) {
		ShowError("gm:info invalid syntax. use '"CL_WHITE"gm:info xCord yCord map_name"CL_RESET"'\n");
		return;
	}

	if ( (m = map->mapname2mapid(map_name) <= 0 ) ) {
		ShowError("gm:info '"CL_WHITE"%s"CL_RESET"' is not a known map\n",map_name);
		return;
	}

	if( x < 0 || x >= map->list[m].xs || y < 0 || y >= map->list[m].ys ) {
		ShowError("gm:info '"CL_WHITE"%d %d"CL_RESET"' is out of '"CL_WHITE"%s"CL_RESET"' map bounds!\n",x,y,map_name);
		return;
	}

	ShowInfo("HCP: updated console's game position to '"CL_WHITE"%d %d %s"CL_RESET"'\n",x,y,map_name);
	map->cpsd->bl.x = x;
	map->cpsd->bl.y = y;
	map->cpsd->bl.m = m;
}
CPCMD(gm_use) {

	if( line == NULL ) {
		ShowError("gm:use invalid syntax. use '"CL_WHITE"gm:use @command <optional params>"CL_RESET"'\n");
		return;
	}
	map->cpsd->fd = -2;
	if( !atcommand->parse(map->cpsd->fd, map->cpsd, line, 0) )
		ShowInfo("HCP: '"CL_WHITE"%s"CL_RESET"' failed\n",line);
	else
		ShowInfo("HCP: '"CL_WHITE"%s"CL_RESET"' was used\n",line);
	map->cpsd->fd = 0;

}
/* Hercules Console Parser */
void map_cp_defaults(void) {
#ifdef CONSOLE_INPUT
	/* default HCP data */
	map->cpsd = pc->get_dummy_sd();
	strcpy(map->cpsd->status.name, "Hercules Console");
	map->cpsd->bl.x = MAP_DEFAULT_X;
	map->cpsd->bl.y = MAP_DEFAULT_Y;
	map->cpsd->bl.m = map->mapname2mapid(MAP_DEFAULT);

	console->addCommand("gm:info",CPCMD_A(gm_position));
	console->addCommand("gm:use",CPCMD_A(gm_use));
#endif
}
/* Hercules Plugin Mananger */
void map_hp_symbols(void) {
	/* full interfaces */
	HPM->share(atcommand,"atcommand");
	HPM->share(battle,"battle");
	HPM->share(bg,"battlegrounds");
	HPM->share(buyingstore,"buyingstore");
	HPM->share(clif,"clif");
	HPM->share(chrif,"chrif");
	HPM->share(guild,"guild");
	HPM->share(gstorage,"gstorage");
	HPM->share(homun,"homun");
	HPM->share(map,"map");
	HPM->share(ircbot,"ircbot");
	HPM->share(itemdb,"itemdb");
	HPM->share(logs,"logs");
	HPM->share(mail,"mail");
	HPM->share(instance,"instance");
	HPM->share(script,"script");
	HPM->share(searchstore,"searchstore");
	HPM->share(skill,"skill");
	HPM->share(vending,"vending");
	HPM->share(pc,"pc");
	HPM->share(pcg,"pc_groups");
	HPM->share(party,"party");
	HPM->share(storage,"storage");
	HPM->share(trade,"trade");
	HPM->share(status,"status");
	HPM->share(chat, "chat");
	HPM->share(duel,"duel");
	HPM->share(elemental,"elemental");
	HPM->share(intif,"intif");
	HPM->share(mercenary,"mercenary");
	HPM->share(mob,"mob");
	HPM->share(unit,"unit");
	HPM->share(npc,"npc");
	HPM->share(mapreg,"mapreg");
	HPM->share(pet,"pet");
	HPM->share(path,"path");
	HPM->share(quest,"quest");
#ifdef PCRE_SUPPORT
	HPM->share(npc_chat,"npc_chat");
#endif
	/* partial */
	HPM->share(mapit,"mapit");
	/* sql link */
	HPM->share(map->mysql_handle,"sql_handle");
	/* specific */
	HPM->share(atcommand->create,"addCommand");
	HPM->share(script->addScript,"addScript");
	/* vars */
	HPM->share(map->list,"map->list");
}

void map_load_defaults(void) {
	map_defaults();
	/* */
	atcommand_defaults();
	battle_defaults();
	battleground_defaults();
	buyingstore_defaults();
	clif_defaults();
	chrif_defaults();
	guild_defaults();
	gstorage_defaults();
	homunculus_defaults();
	instance_defaults();
	ircbot_defaults();
	itemdb_defaults();
	log_defaults();
	mail_defaults();
	npc_defaults();
	script_defaults();
	searchstore_defaults();
	skill_defaults();
	vending_defaults();
	pc_defaults();
	pc_groups_defaults();
	party_defaults();
	storage_defaults();
	trade_defaults();
	status_defaults();
	chat_defaults();
	duel_defaults();
	elemental_defaults();
	intif_defaults();
	mercenary_defaults();
	mob_defaults();
	unit_defaults();
	mapreg_defaults();
	pet_defaults();
	path_defaults();
	quest_defaults();
#ifdef PCRE_SUPPORT
	npc_chat_defaults();
#endif
}
int do_init(int argc, char *argv[])
{
	bool minimal = false;
	char *scriptcheck = NULL;
	int i;

#ifdef GCOLLECT
	GC_enable_incremental();
#endif
	
	map_load_defaults();

	HPM->load_sub = HPM_map_plugin_load_sub;
	HPM->symbol_defaults_sub = map_hp_symbols;
	HPM->grabHPDataSub = HPM_map_grabHPData;
	HPM->config_read();
	
	HPM->event(HPET_PRE_INIT);
	
	for( i = 1; i < argc ; i++ ) {
		const char* arg = argv[i];

		if( arg[0] != '-' && ( arg[0] != '/' || arg[1] == '-' ) ) {// -, -- and /
			ShowError("Unknown option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		} else if ( HPM->parse_arg(arg,&i,argv,map->arg_next_value(arg, i, argc, false)) ) {
			continue; /* HPM Triggered */
		} else if( (++arg)[0] == '-' ) {// long option
			arg++;

			if( strcmp(arg, "help") == 0 ) {
				map->helpscreen(true);
			} else if( strcmp(arg, "version") == 0 ) {
				map->versionscreen(true);
			} else if( strcmp(arg, "map-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->MAP_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "battle-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->BATTLE_CONF_FILENAME = argv[++i];
			} else if( strcmp(arg, "atcommand-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->ATCOMMAND_CONF_FILENAME = argv[++i];
			} else if( strcmp(arg, "script-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->SCRIPT_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "msg-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->MSG_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "grf-path-file") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->GRF_PATH_FILENAME = argv[++i];
			} else if( strcmp(arg, "inter-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->INTER_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "log-config") == 0 ) {
				if( map->arg_next_value(arg, i, argc, true) )
					map->LOG_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "run-once") == 0 ) { // close the map-server as soon as its done.. for testing [Celest]
				runflag = CORE_ST_STOP;
			} else if( strcmp(arg, "script-check") == 0 ) {
				map->minimal = true;
				runflag = CORE_ST_STOP;
				if( map->arg_next_value(arg, i, argc, true) )
					scriptcheck = argv[++i];
			} else {
				ShowError("Unknown option '%s'.\n", argv[i]);
				exit(EXIT_FAILURE);
			}
		} else {
			switch( arg[0] ) {// short option
				case '?':
				case 'h':
					map->helpscreen(true);
					break;
				case 'v':
					map->versionscreen(true);
					break;
				default:
					ShowError("Unknown option '%s'.\n", argv[i]);
					exit(EXIT_FAILURE);
			}
		}
	}
	minimal = map->minimal;/* temp (perhaps make minimal a mask with options of what to load? e.g. plugin 1 does minimal |= mob_db; */
	if (!minimal) {
		map->config_read(map->MAP_CONF_NAME);
		CREATE(map->list,struct map_data,map->count);
		map->count = 0;
		map->config_read_sub(map->MAP_CONF_NAME);

		// loads npcs
		map->reloadnpc(false);

		chrif->checkdefaultlogin();

		if (!map->ip_set || !map->char_ip_set) {
			char ip_str[16];
			ip2str(addr_[0], ip_str);

			ShowWarning("Not all IP addresses in /conf/map-server.conf configured, autodetecting...\n");

			if (naddr_ == 0)
				ShowError("Unable to determine your IP address...\n");
			else if (naddr_ > 1)
				ShowNotice("Multiple interfaces detected...\n");

			ShowInfo("Defaulting to %s as our IP address\n", ip_str);

			if (!map->ip_set)
				clif->setip(ip_str);
			if (!map->char_ip_set)
				chrif->setip(ip_str);
		}

		battle->config_read(map->BATTLE_CONF_FILENAME);
		atcommand->msg_read(map->MSG_CONF_NAME);
		map->inter_config_read(map->INTER_CONF_NAME);
		logs->config_read(map->LOG_CONF_NAME);
	}
	script->config_read(map->SCRIPT_CONF_NAME);

	map->id_db     = idb_alloc(DB_OPT_BASE);
	map->pc_db     = idb_alloc(DB_OPT_BASE); //Added for reliable map->id2sd() use. [Skotlex]
	map->mobid_db  = idb_alloc(DB_OPT_BASE); //Added to lower the load of the lazy mob ai. [Skotlex]
	map->bossid_db = idb_alloc(DB_OPT_BASE); // Used for Convex Mirror quick MVP search
	map->map_db    = uidb_alloc(DB_OPT_BASE);
	map->nick_db   = idb_alloc(DB_OPT_BASE);
	map->charid_db = idb_alloc(DB_OPT_BASE);
	map->regen_db  = idb_alloc(DB_OPT_BASE); // efficient status_natural_heal processing
	map->iwall_db  = strdb_alloc(DB_OPT_RELEASE_DATA,2*NAME_LENGTH+2+1); // [Zephyrus] Invisible Walls
	map->zone_db   = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, MAP_ZONE_NAME_LENGTH);

	map->iterator_ers = ers_new(sizeof(struct s_mapiterator),"map.c::map_iterator_ers",ERS_OPT_NONE);
	
	map->flooritem_ers = ers_new(sizeof(struct flooritem_data),"map.c::map_flooritem_ers",ERS_OPT_NONE);
	ers_chunk_size(map->flooritem_ers, 100);

	if (!minimal) {
		map->sql_init();
		if (logs->config.sql_logs)
			logs->sql_init();
	}

	i = mapindex_init();

	if (minimal) {
		// Pretend all maps from the mapindex are on this mapserver
		CREATE(map->list,struct map_data,i);

		for( i = 0; i < MAX_MAPINDEX; i++ ) {
			if (mapindex_exists(i)) {
				map->addmap(mapindex_id2name(i));
			}
		}
	}

	if(map->enable_grf)
		grfio_init(map->GRF_PATH_FILENAME);

	map->readallmaps();


	if (!minimal) {
		timer->add_func_list(map->freeblock_timer, "map_freeblock_timer");
		timer->add_func_list(map->clearflooritem_timer, "map_clearflooritem_timer");
		timer->add_func_list(map->removemobs_timer, "map_removemobs_timer");
		timer->add_interval(timer->gettick()+1000, map->freeblock_timer, 0, 0, 60*1000);

		HPM->event(HPET_INIT);
	}

	atcommand->init(minimal);
	battle->init(minimal);
	instance->init(minimal);
	chrif->init(minimal);
	clif->init(minimal);
	ircbot->init(minimal);
	script->init(minimal);
	itemdb->init(minimal);
	skill->init(minimal);
	if (!minimal)
		map->read_zone_db();/* read after item and skill initalization */
	mob->init(minimal);
	pc->init(minimal);
	status->init(minimal);
	party->init(minimal);
	guild->init(minimal);
	gstorage->init(minimal);
	pet->init(minimal);
	homun->init(minimal);
	mercenary->init(minimal);
	elemental->init(minimal);
	quest->init(minimal);
	npc->init(minimal);
	unit->init(minimal);
	bg->init(minimal);
	duel->init(minimal);
	vending->init(minimal);

	if (scriptcheck) {
		if (npc->parsesrcfile(scriptcheck, false) == 0)
			exit(EXIT_SUCCESS);
		exit(EXIT_FAILURE);
	}

	if( minimal ) {
		HPM->event(HPET_READY);
		exit(EXIT_SUCCESS);
	}
	
	npc->event_do_oninit();	// Init npcs (OnInit)

	if (battle_config.pk_mode)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	Sql_HerculesUpdateCheck(map->mysql_handle);
	
#ifdef CONSOLE_INPUT
	console->setSQL(map->mysql_handle);
#endif
	
	ShowStatus("Server is '"CL_GREEN"ready"CL_RESET"' and listening on port '"CL_WHITE"%d"CL_RESET"'.\n\n", map->port);

	if( runflag != CORE_ST_STOP ) {
		shutdown_callback = map->do_shutdown;
		runflag = MAPSERVER_ST_RUNNING;
	}

	map_cp_defaults();

	HPM->event(HPET_READY);

	return 0;
}

/*=====================================
* Default Functions : map.h 
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void map_defaults(void) {
	map = &map_s;

	/* */
	map->minimal = false;
	map->count = 0;
	
	sprintf(map->db_path ,"db");
	sprintf(map->help_txt ,"conf/help.txt");
	sprintf(map->help2_txt ,"conf/help2.txt");
	sprintf(map->charhelp_txt ,"conf/charhelp.txt");
	
	sprintf(map->wisp_server_name ,"Server"); // can be modified in char-server configuration file
	
	map->autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
	map->minsave_interval = 100;
	map->save_settings = 0xFFFF;
	map->agit_flag = 0;
	map->agit2_flag = 0;
	map->night_flag = 0; // 0=day, 1=night [Yor]
	map->enable_spy = 0; //To enable/disable @spy commands, which consume too much cpu time when sending packets. [Skotlex]
	
	map->db_use_sql_item_db = 0;
	map->db_use_sql_mob_db = 0;
	map->db_use_sql_mob_skill_db = 0;
	
	sprintf(map->item_db_db, "item_db");
	sprintf(map->item_db2_db, "item_db2");
	sprintf(map->item_db_re_db, "item_db_re");
	sprintf(map->mob_db_db, "mob_db");
	sprintf(map->mob_db2_db, "mob_db2");
	sprintf(map->mob_skill_db_db, "mob_skill_db");
	sprintf(map->mob_skill_db2_db, "mob_skill_db2");
	sprintf(map->interreg_db, "interreg");
	
	map->INTER_CONF_NAME="conf/inter-server.conf";
	map->LOG_CONF_NAME="conf/logs.conf";
	map->MAP_CONF_NAME = "conf/map-server.conf";
	map->BATTLE_CONF_FILENAME = "conf/battle.conf";
	map->ATCOMMAND_CONF_FILENAME = "conf/atcommand.conf";
	map->SCRIPT_CONF_NAME = "conf/script.conf";
	map->MSG_CONF_NAME = "conf/messages.conf";
	map->GRF_PATH_FILENAME = "conf/grf-files.txt";
	
	map->default_codepage[0] = '\0';
	map->server_port = 3306;
	sprintf(map->server_ip,"127.0.0.1");
	sprintf(map->server_id,"ragnarok");
	sprintf(map->server_pw,"ragnarok");
	sprintf(map->server_db,"ragnarok");
	map->mysql_handle = NULL;

	map->port = 0;
	map->users = 0;
	map->ip_set = 0;
	map->char_ip_set = 0;
	map->enable_grf = 0;
	
	memset(&map->index2mapid, -1, sizeof(map->index2mapid));
	
	map->id_db = NULL;
	map->pc_db = NULL;
	map->mobid_db = NULL;
	map->bossid_db = NULL;
	map->map_db = NULL;
	map->nick_db = NULL;
	map->charid_db = NULL;
	map->regen_db = NULL;
	map->zone_db = NULL;
	map->iwall_db = NULL;

	//all in a big chunk, respects order
	memset(map->block_free,0,sizeof(map->block_free)
		   + sizeof(map->block_free_count)
		   + sizeof(map->block_free_lock)
		   + sizeof(map->bl_list)
		   + sizeof(map->bl_list_count)
		   + sizeof(map->bl_head)
		   + sizeof(map->zone_all)
		   + sizeof(map->zone_pk)
		   );

	map->cpsd = NULL;
	map->list = NULL;
	
	map->iterator_ers = NULL;
	map->cache_buffer = NULL;
	
	map->flooritem_ers = NULL;
	/* */
	map->bonus_id = SP_LAST_KNOWN;
	/* funcs */
	map->zone_init = map_zone_init;
	map->zone_remove = map_zone_remove;
	map->zone_apply = map_zone_apply;
	map->zone_change = map_zone_change;
	map->zone_change2 = map_zone_change2;

	map->getcell = map_getcell;
	map->setgatcell = map_setgatcell;

	map->cellfromcache = map_cellfromcache;
	// users
	map->setusers = map_setusers;
	map->getusers = map_getusers;
	map->usercount = map_usercount;
	// blocklist lock
	map->freeblock = map_freeblock;
	map->freeblock_lock = map_freeblock_lock;
	map->freeblock_unlock = map_freeblock_unlock;
	// blocklist manipulation
	map->addblock = map_addblock;
	map->delblock = map_delblock;
	map->moveblock = map_moveblock;
	//blocklist nb in one cell
	map->count_oncell = map_count_oncell;
	map->find_skill_unit_oncell = map_find_skill_unit_oncell;
	// search and creation
	map->get_new_object_id = map_get_new_object_id;
	map->search_freecell = map_search_freecell;
	//
	map->quit = map_quit;
	// npc
	map->addnpc = map_addnpc;
	// map item
	map->clearflooritem_timer = map_clearflooritem_timer;
	map->removemobs_timer = map_removemobs_timer;
	map->clearflooritem = map_clearflooritem;
	map->addflooritem = map_addflooritem;
	// player to map session
	map->addnickdb = map_addnickdb;
	map->delnickdb = map_delnickdb;
	map->reqnickdb = map_reqnickdb;
	map->charid2nick = map_charid2nick;
	map->charid2sd = map_charid2sd;

	map->vforeachpc = map_vforeachpc;
	map->foreachpc = map_foreachpc;
	map->vforeachmob = map_vforeachmob;
	map->foreachmob = map_foreachmob;
	map->vforeachnpc = map_vforeachnpc;
	map->foreachnpc = map_foreachnpc;
	map->vforeachregen = map_vforeachregen;
	map->foreachregen = map_foreachregen;
	map->vforeachiddb = map_vforeachiddb;
	map->foreachiddb = map_foreachiddb;

	map->vforeachinrange = map_vforeachinrange;
	map->foreachinrange = map_foreachinrange;
	map->vforeachinshootrange = map_vforeachinshootrange;
	map->foreachinshootrange = map_foreachinshootrange;
	map->vforeachinarea = map_vforeachinarea;
	map->foreachinarea = map_foreachinarea;
	map->vforcountinrange = map_vforcountinrange;
	map->forcountinrange = map_forcountinrange;
	map->vforcountinarea = map_vforcountinarea;
	map->forcountinarea = map_forcountinarea;
	map->vforeachinmovearea = map_vforeachinmovearea;
	map->foreachinmovearea = map_foreachinmovearea;
	map->vforeachincell = map_vforeachincell;
	map->foreachincell = map_foreachincell;
	map->vforeachinpath = map_vforeachinpath;
	map->foreachinpath = map_foreachinpath;
	map->vforeachinmap = map_vforeachinmap;
	map->foreachinmap = map_foreachinmap;
	map->vforeachininstance = map_vforeachininstance;
	map->foreachininstance = map_foreachininstance;

	map->id2sd = map_id2sd;
	map->id2md = map_id2md;
	map->id2nd = map_id2nd;
	map->id2hd = map_id2hd;
	map->id2mc = map_id2mc;
	map->id2cd = map_id2cd;
	map->id2bl = map_id2bl;
	map->blid_exists = map_blid_exists;
	map->mapindex2mapid = map_mapindex2mapid;
	map->mapname2mapid = map_mapname2mapid;
	map->mapname2ipport = map_mapname2ipport;
	map->setipport = map_setipport;
	map->eraseipport = map_eraseipport;
	map->eraseallipport = map_eraseallipport;
	map->addiddb = map_addiddb;
	map->deliddb = map_deliddb;
	/* */
	map->nick2sd = map_nick2sd;
	map->getmob_boss = map_getmob_boss;
	map->id2boss = map_id2boss;
	// reload config file looking only for npcs
	map->reloadnpc = map_reloadnpc;

	map->check_dir = map_check_dir;
	map->calc_dir = map_calc_dir;
	map->random_dir = map_random_dir; // [Skotlex]

	map->cleanup_sub = cleanup_sub;

	map->delmap = map_delmap;
	map->flags_init = map_flags_init;

	map->iwall_set = map_iwall_set;
	map->iwall_get = map_iwall_get;
	map->iwall_remove = map_iwall_remove;

	map->addmobtolist = map_addmobtolist; // [Wizputer]
	map->spawnmobs = map_spawnmobs; // [Wizputer]
	map->removemobs = map_removemobs; // [Wizputer]
	map->addmap2db = map_addmap2db;
	map->removemapdb = map_removemapdb;
	map->clean = map_clean;

	map->do_shutdown = do_shutdown;

	map->freeblock_timer = map_freeblock_timer;
	map->searchrandfreecell = map_searchrandfreecell;
	map->count_sub = map_count_sub;
	map->create_charid2nick = create_charid2nick;
	map->removemobs_sub = map_removemobs_sub;
	map->gat2cell = map_gat2cell;
	map->cell2gat = map_cell2gat;
	map->getcellp = map_getcellp;
	map->setcell = map_setcell;
	map->sub_getcellp = map_sub_getcellp;
	map->sub_setcell = map_sub_setcell;
	map->iwall_nextxy = map_iwall_nextxy;
	map->create_map_data_other_server = create_map_data_other_server;
	map->eraseallipport_sub = map_eraseallipport_sub;
	map->init_mapcache = map_init_mapcache;
	map->readfromcache = map_readfromcache;
	map->addmap = map_addmap;
	map->delmapid = map_delmapid;
	map->zone_db_clear = map_zone_db_clear;
	map->list_final = do_final_maps;
	map->waterheight = map_waterheight;
	map->readgat = map_readgat;
	map->readallmaps = map_readallmaps;
	map->config_read = map_config_read;
	map->config_read_sub = map_config_read_sub;
	map->reloadnpc_sub = map_reloadnpc_sub;
	map->inter_config_read = inter_config_read;
	map->sql_init = map_sql_init;
	map->sql_close = map_sql_close;
	map->zone_mf_cache = map_zone_mf_cache;
	map->zone_str2itemid = map_zone_str2itemid;
	map->zone_str2skillid = map_zone_str2skillid;
	map->zone_bl_type = map_zone_bl_type;
	map->read_zone_db = read_map_zone_db;
	map->db_final = map_db_final;
	map->nick_db_final = nick_db_final;
	map->cleanup_db_sub = cleanup_db_sub;
	map->abort_sub = map_abort_sub;
	map->helpscreen = map_helpscreen;
	map->versionscreen = map_versionscreen;
	map->arg_next_value = map_arg_next_value;

	map->addblcell = map_addblcell;
	map->delblcell = map_delblcell;
	
	map->get_new_bonus_id = map_get_new_bonus_id;
	
	map->add_questinfo = map_add_questinfo;
	map->remove_questinfo = map_remove_questinfo;
	
	/**
	 * mapit interface
	 **/
	
	mapit = &mapit_s;

	mapit->alloc = mapit_alloc;
	mapit->free = mapit_free;
	mapit->first = mapit_first;
	mapit->last = mapit_last;
	mapit->next = mapit_next;
	mapit->prev = mapit_prev;
	mapit->exists = mapit_exists;
}
