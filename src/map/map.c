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

char default_codepage[32] = "";

int map_server_port = 3306;
char map_server_ip[32] = "127.0.0.1";
char map_server_id[32] = "ragnarok";
char map_server_pw[32] = "ragnarok";
char map_server_db[32] = "ragnarok";
Sql* mmysql_handle;

int map_port=0;

// log database
char log_db_ip[32] = "127.0.0.1";
int log_db_port = 3306;
char log_db_id[32] = "ragnarok";
char log_db_pw[32] = "ragnarok";
char log_db_db[32] = "log";
Sql* logmysql_handle;

// DBMap declaration
static DBMap* id_db=NULL; // int id -> struct block_list*
static DBMap* pc_db=NULL; // int id -> struct map_session_data*
static DBMap* mobid_db=NULL; // int id -> struct mob_data*
static DBMap* bossid_db=NULL; // int id -> struct mob_data* (MVP db)
static DBMap* map_db=NULL; // unsigned int mapindex -> struct map_data_other_server*
static DBMap* nick_db=NULL; // int char_id -> struct charid2nick* (requested names of offline characters)
static DBMap* charid_db=NULL; // int char_id -> struct map_session_data*
static DBMap* regen_db=NULL; // int id -> struct block_list* (status_natural_heal processing)

static int map_users=0;

#define BLOCK_SIZE 8
#define block_free_max 1048576
struct block_list *block_free[block_free_max];
static int block_free_count = 0, block_free_lock = 0;

#define BL_LIST_MAX 1048576
static struct block_list *bl_list[BL_LIST_MAX];
static int bl_list_count = 0;

struct charid_request {
	struct charid_request* next;
	int charid;// who want to be notified of the nick
};
struct charid2nick {
	char nick[NAME_LENGTH];
	struct charid_request* requests;// requests of notification on this nick
};

// This is the main header found at the very beginning of the map cache
struct map_cache_main_header {
	uint32 file_size;
	uint16 map_count;
};

// This is the header appended before every compressed map cells info in the map cache
struct map_cache_map_info {
	char name[MAP_NAME_LENGTH];
	int16 xs;
	int16 ys;
	int32 len;
};

int16 index2mapid[MAX_MAPINDEX];

int enable_grf = 0;	//To enable/disable reading maps from GRF files, bypassing mapcache [blackhole89]

/* [Ind/Hercules] */
struct eri *map_iterator_ers;
char *map_cache_buffer = NULL; // Has the uncompressed gat data of all maps, so just one allocation has to be made

struct map_interface iMap_s;

struct map_session_data *cpsd;

/*==========================================
* server player count (of all mapservers)
*------------------------------------------*/
void map_setusers(int users)
{
	map_users = users;
}

int map_getusers(void)
{
	return map_users;
}

/*==========================================
* server player count (this mapserver only)
*------------------------------------------*/
int map_usercount(void)
{
	return pc_db->size(pc_db);
}


/*==========================================
* Attempt to free a map blocklist
*------------------------------------------*/
int map_freeblock (struct block_list *bl)
{
	nullpo_retr(block_free_lock, bl);
	if (block_free_lock == 0 || block_free_count >= block_free_max)
	{
		aFree(bl);
		bl = NULL;
		if (block_free_count >= block_free_max)
			ShowWarning("map_freeblock: too many free block! %d %d\n", block_free_count, block_free_lock);
	} else
		block_free[block_free_count++] = bl;

	return block_free_lock;
}
/*==========================================
* Lock blocklist, (prevent iMap->freeblock usage)
*------------------------------------------*/
int map_freeblock_lock (void)
{
	return ++block_free_lock;
}

/*==========================================
* Remove the lock on map_bl
*------------------------------------------*/
int map_freeblock_unlock (void)
{
	if ((--block_free_lock) == 0) {
		int i;
		for (i = 0; i < block_free_count; i++)
		{
			aFree(block_free[i]);
			block_free[i] = NULL;
		}
		block_free_count = 0;
	} else if (block_free_lock < 0) {
		ShowError("map_freeblock_unlock: lock count < 0 !\n");
		block_free_lock = 0;
	}

	return block_free_lock;
}

// Timer function to check if there some remaining lock and remove them if so.
// Called each 1s
int map_freeblock_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	if (block_free_lock > 0) {
		ShowError("map_freeblock_timer: block_free_lock(%d) is invalid.\n", block_free_lock);
		block_free_lock = 1;
		iMap->freeblock_unlock();
	}

	return 0;
}

//
// blocklist
//
/*==========================================
* Handling of map_bl[]
* The adresse of bl_heal is set in bl->prev
*------------------------------------------*/
static struct block_list bl_head;

#ifdef CELL_NOSTACK
/*==========================================
* These pair of functions update the counter of how many objects
* lie on a tile.
*------------------------------------------*/
static void map_addblcell(struct block_list *bl) {
	if( bl->m < 0 || bl->x < 0 || bl->x >= maplist[bl->m].xs
	              || bl->y < 0 || bl->y >= maplist[bl->m].ys
	              || !(bl->type&BL_CHAR) )
		return;
	maplist[bl->m].cell[bl->x+bl->y*maplist[bl->m].xs].cell_bl++;
	return;
}

static void map_delblcell(struct block_list *bl) {
	if( bl->m < 0 || bl->x < 0 || bl->x >= maplist[bl->m].xs
	              || bl->y < 0 || bl->y >= maplist[bl->m].ys
	              || !(bl->type&BL_CHAR) )
		return;
	maplist[bl->m].cell[bl->x+bl->y*maplist[bl->m].xs].cell_bl--;
}
#endif

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
	if( m < 0 || m >= iMap->map_num )
	{
		ShowError("map_addblock: invalid map id (%d), only %d are loaded.\n", m, iMap->map_num);
		return 1;
	}
	if( x < 0 || x >= maplist[m].xs || y < 0 || y >= maplist[m].ys ) {
		ShowError("map_addblock: out-of-bounds coordinates (\"%s\",%d,%d), map is %dx%d\n", maplist[m].name, x, y, maplist[m].xs, maplist[m].ys);
		return 1;
	}

	pos = x/BLOCK_SIZE+(y/BLOCK_SIZE)*maplist[m].bxs;

	if (bl->type == BL_MOB) {
		bl->next = maplist[m].block_mob[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		maplist[m].block_mob[pos] = bl;
	} else {
		bl->next = maplist[m].block[pos];
		bl->prev = &bl_head;
		if (bl->next) bl->next->prev = bl;
		maplist[m].block[pos] = bl;
	}

#ifdef CELL_NOSTACK
	map_addblcell(bl);
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
	map_delblcell(bl);
#endif

	pos = bl->x/BLOCK_SIZE+(bl->y/BLOCK_SIZE)*maplist[bl->m].bxs;

	if (bl->next)
		bl->next->prev = bl->prev;
	if (bl->prev == &bl_head) {
		//Since the head of the list, update the block_list map of []
		if (bl->type == BL_MOB) {
			maplist[bl->m].block_mob[pos] = bl->next;
		} else {
			maplist[bl->m].block[pos] = bl->next;
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
int map_moveblock(struct block_list *bl, int x1, int y1, unsigned int tick)
{
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
		sc = iStatus->get_sc(bl);

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

	if (moveblock) iMap->delblock(bl);
#ifdef CELL_NOSTACK
	else map_delblcell(bl);
#endif
	bl->x = x1;
	bl->y = y1;
	if (moveblock) iMap->addblock(bl);
#ifdef CELL_NOSTACK
	else map_addblcell(bl);
#endif

	if (bl->type&BL_CHAR) {

		skill->unit_move(bl,tick,3);

		if( bl->type == BL_PC && ((TBL_PC*)bl)->shadowform_id ) {//Shadow Form Target Moving
			struct block_list *d_bl;
			if( (d_bl = iMap->id2bl(((TBL_PC*)bl)->shadowform_id)) == NULL || !check_distance_bl(bl,d_bl,10) ) {
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
					if( (d_bl = iMap->id2bl(sc->data[SC__SHADOWFORM]->val2)) == NULL || !check_distance_bl(bl,d_bl,10) )
						status_change_end(bl,SC__SHADOWFORM,INVALID_TIMER);
				}

				if (sc->data[SC_PROPERTYWALK]
				&& sc->data[SC_PROPERTYWALK]->val3 < skill->get_maxcount(sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2)
					&& iMap->find_skill_unit_oncell(bl,bl->x,bl->y,SO_ELECTRICWALK,NULL,0) == NULL
					&& iMap->find_skill_unit_oncell(bl,bl->x,bl->y,SO_FIREWALK,NULL,0) == NULL
					&& skill->unitsetting(bl,sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2,x0, y0,0)) {
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

	if (x < 0 || y < 0 || (x >= maplist[m].xs) || (y >= maplist[m].ys))
		return 0;

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	if (type&~BL_MOB)
		for( bl = maplist[m].block[bx+by*maplist[m].bxs] ; bl != NULL ; bl = bl->next )
			if(bl->x == x && bl->y == y && bl->type&type)
				count++;

	if (type&BL_MOB)
		for( bl = maplist[m].block_mob[bx+by*maplist[m].bxs] ; bl != NULL ; bl = bl->next )
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

	if (x < 0 || y < 0 || (x >= maplist[m].xs) || (y >= maplist[m].ys))
		return NULL;

	bx = x/BLOCK_SIZE;
	by = y/BLOCK_SIZE;

	for( bl = maplist[m].block[bx+by*maplist[m].bxs] ; bl != NULL ; bl = bl->next ) {
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
static int bl_vforeach(int (*func)(struct block_list*, va_list), int blockcount, int max, va_list args)
{
	int i;
	int returnCount = 0;

	iMap->freeblock_lock();
	for (i = blockcount; i < bl_list_count && returnCount < max; i++) {
		if (bl_list[i]->prev) { //func() may delete this bl_list[] slot, checking for prev ensures it wasnt queued for deletion.
			va_list argscopy;
			va_copy(argscopy, args);
			returnCount += func(bl_list[i], argscopy);
			va_end(argscopy);
		}
	}
	iMap->freeblock_unlock();

	bl_list_count = blockcount;

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
	int blockcount = bl_list_count;

	if (m < 0)
		return 0;

	bsize = maplist[m].bxs * maplist[m].bys;
	for (i = 0; i < bsize; i++) {
		if (type&~BL_MOB) {
			for (bl = maplist[m].block[i]; bl != NULL; bl = bl->next) {
				if (bl->type&type && bl_list_count < BL_LIST_MAX) {
					bl_list[bl_list_count++] = bl;
				}
			}
		}
		if (type&BL_MOB) {
			for (bl = maplist[m].block_mob[i]; bl != NULL; bl = bl->next) {
				if (bl_list_count < BL_LIST_MAX) {
					bl_list[bl_list_count++] = bl;
				}
			}
		}
	}

	if (bl_list_count >= BL_LIST_MAX)
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
int map_foreachinmap(int (*func)(struct block_list*, va_list), int16 m, int type, ...)
{
	int returnCount = 0;
	va_list ap;

	va_start(ap, type);
	returnCount = map_vforeachinmap(func, m, type, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachininstance(int (*func)(struct block_list*, va_list), int16 instance_id, int type, ...)
{
	int i;
	int returnCount = 0;

	for (i = 0; i < instances[instance_id].num_map; i++) {
		int m = instances[instance_id].map[i];
		va_list ap;
		va_start(ap, type);
		returnCount += map_vforeachinmap(func, m, type, ap);
		va_end(ap);
	}

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
	x1 = min(x1, maplist[m].xs - 1);
	y1 = min(y1, maplist[m].ys - 1);

	for (by = y0 / BLOCK_SIZE; by <= y1 / BLOCK_SIZE; by++) {
		for (bx = x0 / BLOCK_SIZE; bx <= x1 / BLOCK_SIZE; bx++) {
			if (type&~BL_MOB) {
				for (bl = maplist[m].block[bx + by * maplist[m].bxs]; bl != NULL; bl = bl->next) {
					if (bl_list_count < BL_LIST_MAX
						&& bl->type&type
						&& bl->x >= x0 && bl->x <= x1
						&& bl->y >= y0 && bl->y <= y1) {
							if (func) {
								va_start(args, func);
								if (func(bl, args)) {
									bl_list[bl_list_count++] = bl;
									found++;
								}
								va_end(args);
							}
							else {
								bl_list[bl_list_count++] = bl;
								found++;
							}
					}
				}
			}
			if (type&BL_MOB) { // TODO: fix this code duplication
				for (bl = maplist[m].block_mob[bx + by * maplist[m].bxs]; bl != NULL; bl = bl->next) {
					if (bl_list_count < BL_LIST_MAX
						//&& bl->type&type // block_mob contains BL_MOBs only
						&& bl->x >= x0 && bl->x <= x1
						&& bl->y >= y0 && bl->y <= y1) {
							if (func) {
								va_start(args, func);
								if (func(bl, args)) {
									bl_list[bl_list_count++] = bl;
									found++;
								}
								va_end(args);
							}
							else {
								bl_list[bl_list_count++] = bl;
								found++;
							}
					}
				}
			}
		}
	}

	if (bl_list_count >= BL_LIST_MAX)
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;

	if (range < 0) range *= -1;

	bl_getall_area(type, center->m, center->x - range, center->y - range, center->x + range, center->y + range, bl_vgetall_inrange, center, range);

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_forcountinrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int count, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;

	if (range < 0) range *= -1;

	bl_getall_area(type, center->m, center->x - range, center->y - range, center->x + range, center->y + range, bl_vgetall_inrange, center, range);

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, count, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinshootrange(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;

	if (range < 0) range *= -1;

	bl_getall_area(type, center->m, center->x - range, center->y - range, center->x + range, center->y + range, bl_vgetall_inshootrange, center, range);

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinarea(int (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;

	bl_getall_area(type, m, x0, y0, x1, y1, NULL);

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_forcountinarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;

	bl_getall_area(type, m, x0, y0, x1, y1, NULL);

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, count, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinmovearea(int (*func)(struct block_list*, va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;
	int m, x0, x1, y0, y1;

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

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachincell(int (*func)(struct block_list*, va_list), int16 m, int16 x, int16 y, int type, ...)
{
	int returnCount = 0;
	int blockcount = bl_list_count;
	va_list ap;

	bl_getall_area(type, m, x, y, x, y, NULL);

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
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
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 */
int map_foreachinpath(int (*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...)
{
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
	int blockcount = bl_list_count;
	va_list ap;

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

	va_start(ap, type);
	returnCount = bl_vforeach(func, blockcount, INT_MAX, ap);
	va_end(ap);

	return returnCount;
}
#undef MAGNITUDE2

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

		if( !idb_exists(id_db, i) )
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
int map_clearflooritem_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct flooritem_data* fitem = (struct flooritem_data*)idb_get(id_db, id);

	if (fitem == NULL || fitem->bl.type != BL_ITEM || (fitem->cleartimer != tid)) {
		ShowError("map_clearflooritem_timer : error\n");
		return 1;
	}


	if (pet->search_petDB_index(fitem->item_data.nameid, PET_EGG) >= 0)
		intif->delete_petdata(MakeDWord(fitem->item_data.card[1], fitem->item_data.card[2]));

	clif->clearflooritem(fitem, 0);
	iMap->deliddb(&fitem->bl);
	iMap->delblock(&fitem->bl);
	iMap->freeblock(&fitem->bl);
	return 0;
}

/*
* clears a single bl item out of the bazooonga.
*/
void map_clearflooritem(struct block_list *bl) {
	struct flooritem_data* fitem = (struct flooritem_data*)bl;

	if( fitem->cleartimer )
		timer->delete(fitem->cleartimer,iMap->clearflooritem_timer);

	clif->clearflooritem(fitem, 0);
	iMap->deliddb(&fitem->bl);
	iMap->delblock(&fitem->bl);
	iMap->freeblock(&fitem->bl);
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
		if(i+*y<0 || i+*y>=maplist[m].ys)
			continue;
		for(j=-1;j<=1;j++){
			if(j+*x<0 || j+*x>=maplist[m].xs)
				continue;
			if(iMap->getcell(m,j+*x,i+*y,CELL_CHKNOPASS) && !iMap->getcell(m,j+*x,i+*y,CELL_CHKICEWALL))
				continue;
			//Avoid item stacking to prevent against exploits. [Skotlex]
			if(stack && iMap->count_oncell(m,j+*x,i+*y, BL_ITEM) > stack)
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


static int map_count_sub(struct block_list *bl,va_list ap)
{
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
		return iMap->getcell(m,*x,*y,CELL_CHKREACH);
	}

	if (rx >= 0 && ry >= 0) {
		tries = rx2*ry2;
		if (tries > 100) tries = 100;
	} else {
		tries = maplist[m].xs*maplist[m].ys;
		if (tries > 500) tries = 500;
	}

	while(tries--) {
		*x = (rx >= 0)?(rnd()%rx2-rx+bx):(rnd()%(maplist[m].xs-2)+1);
		*y = (ry >= 0)?(rnd()%ry2-ry+by):(rnd()%(maplist[m].ys-2)+1);

		if (*x == bx && *y == by)
			continue; //Avoid picking the same target tile.

		if (iMap->getcell(m,*x,*y,CELL_CHKREACH))
		{
			if(flag&2 && !unit->can_reach_pos(src, *x, *y, 1))
				continue;
			if(flag&4) {
				if (spawn >= 100) return 0; //Limit of retries reached.
				if (spawn++ < battle_config.no_spawn_on_player &&
					map_foreachinarea(map_count_sub, m,
					*x-AREA_SIZE, *y-AREA_SIZE,
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

	if(!map_searchrandfreecell(m,&x,&y,flags&2?1:0))
		return 0;
	r=rnd();

	CREATE(fitem, struct flooritem_data, 1);
	fitem->bl.type=BL_ITEM;
	fitem->bl.prev = fitem->bl.next = NULL;
	fitem->bl.m=m;
	fitem->bl.x=x;
	fitem->bl.y=y;
	fitem->bl.id = iMap->get_new_object_id();
	if(fitem->bl.id==0){
		aFree(fitem);
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
	fitem->cleartimer=timer->add(timer->gettick()+battle_config.flooritem_lifetime,iMap->clearflooritem_timer,fitem->bl.id,0);

	iMap->addiddb(&fitem->bl);
	iMap->addblock(&fitem->bl);
	clif->dropflooritem(fitem);

	return fitem->bl.id;
}

/**
* @see DBCreateData
*/
static DBData create_charid2nick(DBKey key, va_list args)
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

	if( iMap->charid2sd(charid) )
		return;// already online

	p = idb_ensure(nick_db, charid, create_charid2nick);
	safestrncpy(p->nick, nick, sizeof(p->nick));

	while( p->requests ) {
		req = p->requests;
		p->requests = req->next;
		sd = iMap->charid2sd(req->charid);
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

	if (!nick_db->remove(nick_db, DB->i2key(charid), &data) || (p = DB->data2ptr(&data)) == NULL)
		return;

	while( p->requests ) {
		req = p->requests;
		p->requests = req->next;
		sd = iMap->charid2sd(req->charid);
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

	tsd = iMap->charid2sd(charid);
	if( tsd ) {
		clif->solved_charname(sd->fd, charid, tsd->status.name);
		return;
	}

	p = idb_ensure(nick_db, charid, create_charid2nick);
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
		idb_put(pc_db,sd->bl.id,sd);
		idb_put(charid_db,sd->status.char_id,sd);
	}
	else if( bl->type == BL_MOB )
	{
		TBL_MOB* md = (TBL_MOB*)bl;
		idb_put(mobid_db,bl->id,bl);

		if( md->state.boss )
			idb_put(bossid_db, bl->id, bl);
	}

	if( bl->type & BL_REGEN )
		idb_put(regen_db, bl->id, bl);

	idb_put(id_db,bl->id,bl);
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
		idb_remove(pc_db,sd->bl.id);
		idb_remove(charid_db,sd->status.char_id);
	}
	else if( bl->type == BL_MOB )
	{
		idb_remove(mobid_db,bl->id);
		idb_remove(bossid_db,bl->id);
	}

	if( bl->type & BL_REGEN )
		idb_remove(regen_db,bl->id);

	idb_remove(id_db,bl->id);
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
	//iMap->quit handles extra specific data which is related to quitting normally
	//(changing map-servers invokes unit_free but bypasses iMap->quit)
	if( sd->sc.count ) {
		//Status that are not saved...
		for(i=0; i < SC_MAX; i++){
			if ( iStatus->get_sc_type(i)&SC_NO_SAVE ){
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

	if (sd->state.permanent_speed == 1) sd->state.permanent_speed = 0; // Remove lock so speed is set back to normal at login.

	if( sd->ed ) {
		elemental->clean_effect(sd->ed);
		unit->remove_map(&sd->ed->bl,CLR_TELEPORT,ALC_MARK);
	}

	if( hChSys.local && maplist[sd->bl.m].channel && idb_exists(maplist[sd->bl.m].channel->users, sd->status.char_id) ) {
		clif->chsys_left(maplist[sd->bl.m].channel,sd);
	}

	clif->chsys_quit(sd);

	unit->remove_map_pc(sd,CLR_TELEPORT);

	if( maplist[sd->bl.m].instance_id >= 0 ) { // Avoid map conflicts and warnings on next login
		int16 m;
		struct point *pt;
		if( maplist[sd->bl.m].save.map )
			pt = &maplist[sd->bl.m].save;
		else
			pt = &sd->status.save_point;

		if( (m=iMap->mapindex2mapid(pt->map)) >= 0 ) {
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
struct map_session_data * map_id2sd(int id)
{
	if (id <= 0) return NULL;
	return (struct map_session_data*)idb_get(pc_db,id);
}

struct mob_data * map_id2md(int id)
{
	if (id <= 0) return NULL;
	return (struct mob_data*)idb_get(mobid_db,id);
}

struct npc_data * map_id2nd(int id)
{// just a id2bl lookup because there's no npc_db
	struct block_list* bl = iMap->id2bl(id);

	return BL_CAST(BL_NPC, bl);
}

struct homun_data* map_id2hd(int id)
{
	struct block_list* bl = iMap->id2bl(id);

	return BL_CAST(BL_HOM, bl);
}

struct mercenary_data* map_id2mc(int id)
{
	struct block_list* bl = iMap->id2bl(id);

	return BL_CAST(BL_MER, bl);
}

struct chat_data* map_id2cd(int id)
{
	struct block_list* bl = iMap->id2bl(id);

	return BL_CAST(BL_CHAT, bl);
}

/// Returns the nick of the target charid or NULL if unknown (requests the nick to the char server).
const char* map_charid2nick(int charid)
{
	struct charid2nick *p;
	struct map_session_data* sd;

	sd = iMap->charid2sd(charid);
	if( sd )
		return sd->status.name;// character is online, return it's name

	p = idb_ensure(nick_db, charid, create_charid2nick);
	if( *p->nick )
		return p->nick;// name in nick_db

	chrif->searchcharid(charid);// request the name
	return NULL;
}

/// Returns the struct map_session_data of the charid or NULL if the char is not online.
struct map_session_data* map_charid2sd(int charid)
{
	return (struct map_session_data*)idb_get(charid_db, charid);
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
	return (struct block_list*)idb_get(id_db,id);
}

/**
* Same as iMap->id2bl except it only checks for its existence
**/
bool map_blid_exists( int id ) {
	return (idb_exists(id_db,id));
}

/*==========================================
* Convext Mirror
*------------------------------------------*/
struct mob_data * map_getmob_boss(int16 m)
{
	DBIterator* iter;
	struct mob_data *md = NULL;
	bool found = false;

	iter = db_iterator(bossid_db);
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
	return (struct mob_data*)idb_get(bossid_db,id);
}

/// Applies func to all the players in the db.
/// Stops iterating if func returns -1.
void map_map_foreachpc(int (*func)(struct map_session_data* sd, va_list args), ...) {
	DBIterator* iter;
	struct map_session_data* sd;

	iter = db_iterator(pc_db);
	for( sd = dbi_first(iter); dbi_exists(iter); sd = dbi_next(iter) )
	{
		va_list args;
		int ret;

		va_start(args, func);
		ret = func(sd, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to all the mobs in the db.
/// Stops iterating if func returns -1.
void map_map_foreachmob(int (*func)(struct mob_data* md, va_list args), ...)
{
	DBIterator* iter;
	struct mob_data* md;

	iter = db_iterator(mobid_db);
	for( md = (struct mob_data*)dbi_first(iter); dbi_exists(iter); md = (struct mob_data*)dbi_next(iter) )
	{
		va_list args;
		int ret;

		va_start(args, func);
		ret = func(md, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to all the npcs in the db.
/// Stops iterating if func returns -1.
void map_map_foreachnpc(int (*func)(struct npc_data* nd, va_list args), ...)
{
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(id_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) )
	{
		if( bl->type == BL_NPC )
		{
			struct npc_data* nd = (struct npc_data*)bl;
			va_list args;
			int ret;

			va_start(args, func);
			ret = func(nd, args);
			va_end(args);
			if( ret == -1 )
				break;// stop iterating
		}
	}
	dbi_destroy(iter);
}

/// Applies func to everything in the db.
/// Stops iteratin gif func returns -1.
void map_map_foreachregen(int (*func)(struct block_list* bl, va_list args), ...)
{
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(regen_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) )
	{
		va_list args;
		int ret;

		va_start(args, func);
		ret = func(bl, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
}

/// Applies func to everything in the db.
/// Stops iterating if func returns -1.
void map_map_foreachiddb(int (*func)(struct block_list* bl, va_list args), ...)
{
	DBIterator* iter;
	struct block_list* bl;

	iter = db_iterator(id_db);
	for( bl = (struct block_list*)dbi_first(iter); dbi_exists(iter); bl = (struct block_list*)dbi_next(iter) )
	{
		va_list args;
		int ret;

		va_start(args, func);
		ret = func(bl, args);
		va_end(args);
		if( ret == -1 )
			break;// stop iterating
	}
	dbi_destroy(iter);
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
	( \
	( (_bl_)->type & (_mapit_)->types /* type matches */ ) \
	)

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

	iter = ers_alloc(map_iterator_ers, struct s_mapiterator);
	iter->flags = flags;
	iter->types = types;
	if( types == BL_PC )       iter->dbi = db_iterator(pc_db);
	else if( types == BL_MOB ) iter->dbi = db_iterator(mobid_db);
	else                       iter->dbi = db_iterator(id_db);
	return iter;
}

/// Frees the iterator.
///
/// @param iter Iterator
void mapit_free(struct s_mapiterator* iter) {
	nullpo_retv(iter);

	dbi_destroy(iter->dbi);
	ers_free(map_iterator_ers, iter);
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

	if( m < 0 || m >= iMap->map_num )
		return false;

	if( maplist[m].npc_num == MAX_NPC_PER_MAP ) {
		ShowWarning("too many NPCs in one map %s\n",maplist[m].name);
		return false;
	}

	maplist[m].npc[maplist[m].npc_num]=nd;
	maplist[m].npc_num++;
	idb_put(id_db,nd->bl.id,nd);
	return true;
}

/*=========================================
* Dynamic Mobs [Wizputer]
*-----------------------------------------*/
// Stores the spawn data entry in the mob list.
// Returns the index of successful, or -1 if the list was full.
int map_addmobtolist(unsigned short m, struct spawn_data *spawn) {
	size_t i;
	ARR_FIND( 0, MAX_MOB_LIST_PER_MAP, i, maplist[m].moblist[i] == NULL );
	if( i < MAX_MOB_LIST_PER_MAP ) {
		maplist[m].moblist[i] = spawn;
		return i;
	}
	return -1;
}

void map_spawnmobs(int16 m) {
	int i, k=0;
	if (maplist[m].mob_delete_timer != INVALID_TIMER) {
		//Mobs have not been removed yet [Skotlex]
		timer->delete(maplist[m].mob_delete_timer, iMap->removemobs_timer);
		maplist[m].mob_delete_timer = INVALID_TIMER;
		return;
	}
	for(i=0; i<MAX_MOB_LIST_PER_MAP; i++)
		if(maplist[m].moblist[i]!=NULL) {
			k+=maplist[m].moblist[i]->num;
			npc->parse_mob2(maplist[m].moblist[i]);
		}

	if (battle_config.etc_log && k > 0) {
		ShowStatus("Map %s: Spawned '"CL_WHITE"%d"CL_RESET"' mobs.\n",maplist[m].name, k);
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

int map_removemobs_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	int count;
	const int16 m = id;

	if (m < 0 || m >= iMap->map_num) { //Incorrect map id!
		ShowError("map_removemobs_timer error: timer %d points to invalid map %d\n",tid, m);
		return 0;
	}
	if (maplist[m].mob_delete_timer != tid) { //Incorrect timer call!
		ShowError("map_removemobs_timer mismatch: %d != %d (map %s)\n",maplist[m].mob_delete_timer, tid, maplist[m].name);
		return 0;
	}
	maplist[m].mob_delete_timer = INVALID_TIMER;
	if (maplist[m].users > 0) //Map not empty!
		return 1;

	count = map_foreachinmap(map_removemobs_sub, m, BL_MOB);

	if (battle_config.etc_log && count > 0)
		ShowStatus("Map %s: Removed '"CL_WHITE"%d"CL_RESET"' mobs.\n",maplist[m].name, count);

	return 1;
}

void map_removemobs(int16 m)
{
	if (maplist[m].mob_delete_timer != INVALID_TIMER) // should never happen
		return; //Mobs are already scheduled for removal

	maplist[m].mob_delete_timer = timer->add(timer->gettick()+battle_config.mob_remove_delay, iMap->removemobs_timer, m, 0);
}

/*==========================================
* Hookup, get map_id from map_name
*------------------------------------------*/
int16 map_mapname2mapid(const char* name) {
	unsigned short map_index;
	map_index = mapindex_name2id(name);
	if (!map_index)
		return -1;
	return iMap->mapindex2mapid(map_index);
}

/*==========================================
* Returns the map of the given mapindex. [Skotlex]
*------------------------------------------*/
int16 map_mapindex2mapid(unsigned short mapindex) {

	if (!mapindex || mapindex > MAX_MAPINDEX)
		return -1;

	return index2mapid[mapindex];
}

/*==========================================
* Switching Ip, port ? (like changing map_server) get ip/port from map_name
*------------------------------------------*/
int map_mapname2ipport(unsigned short name, uint32* ip, uint16* port) {
	struct map_data_other_server *mdos;

	mdos = (struct map_data_other_server*)uidb_get(map_db,(unsigned int)name);
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
	} while (
		(iMap->getcell(bl->m,xi,yi,CELL_CHKNOPASS) || !path->search(NULL,bl->m,bl->x,bl->y,xi,yi,1,CELL_CHKNOREACH))
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

static int map_cell2gat(struct mapcell cell) {
	if( cell.walkable == 1 && cell.shootable == 1 && cell.water == 0 ) return 0;
	if( cell.walkable == 0 && cell.shootable == 0 && cell.water == 0 ) return 1;
	if( cell.walkable == 1 && cell.shootable == 1 && cell.water == 1 ) return 3;
	if( cell.walkable == 0 && cell.shootable == 1 && cell.water == 0 ) return 5;

	ShowWarning("map_cell2gat: cell has no matching gat type\n");
	return 1; // default to 'wall'
}
int map_getcellp(struct map_data* m,int16 x,int16 y,cell_chk cellchk);
void map_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag);
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
			m->cell[xy] = map_gat2cell(decode_buffer[xy]);

		m->getcellp = map_getcellp;
		m->setcell  = map_setcell;

		for(i = 0; i < m->npc_num; i++) {
			npc->setcells(m->npc[i]);
		}
	}
}

/*==========================================
* Confirm if celltype in (m,x,y) match the one given in cellchk
*------------------------------------------*/
int map_getcell(int16 m,int16 x,int16 y,cell_chk cellchk) {
	return (m < 0 || m >= iMap->map_num) ? 0 : maplist[m].getcellp(&maplist[m],x,y,cellchk);
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
		return map_cell2gat(cell);

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
	iMap->cellfromcache(m);
	m->getcellp = map_getcellp;
	m->setcell  = map_setcell;
	return m->getcellp(m,x,y,cellchk);
}
/*==========================================
* Change the type/flags of a map cell
* 'cell' - which flag to modify
* 'flag' - true = on, false = off
*------------------------------------------*/
void map_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag) {
	int j;

	if( m < 0 || m >= iMap->map_num || x < 0 || x >= maplist[m].xs || y < 0 || y >= maplist[m].ys )
		return;

	j = x + y*maplist[m].xs;

	switch( cell ) {
	case CELL_WALKABLE:      maplist[m].cell[j].walkable = flag;      break;
	case CELL_SHOOTABLE:     maplist[m].cell[j].shootable = flag;     break;
	case CELL_WATER:         maplist[m].cell[j].water = flag;         break;

	case CELL_NPC:           maplist[m].cell[j].npc = flag;           break;
	case CELL_BASILICA:      maplist[m].cell[j].basilica = flag;      break;
	case CELL_LANDPROTECTOR: maplist[m].cell[j].landprotector = flag; break;
	case CELL_NOVENDING:     maplist[m].cell[j].novending = flag;     break;
	case CELL_NOCHAT:        maplist[m].cell[j].nochat = flag;        break;
	case CELL_MAELSTROM:     maplist[m].cell[j].maelstrom = flag;     break;
	case CELL_ICEWALL:       maplist[m].cell[j].icewall = flag;       break;
	default:
		ShowWarning("map_setcell: invalid cell type '%d'\n", (int)cell);
		break;
	}
}
void map_sub_setcell(int16 m, int16 x, int16 y, cell_t cell, bool flag) {
	if( m < 0 || m >= iMap->map_num || x < 0 || x >= maplist[m].xs || y < 0 || y >= maplist[m].ys )
		return;

	iMap->cellfromcache(&maplist[m]);
	maplist[m].setcell = map_setcell;
	maplist[m].getcellp = map_getcellp;
	maplist[m].setcell(m,x,y,cell,flag);
}
void map_setgatcell(int16 m, int16 x, int16 y, int gat) {
	int j;
	struct mapcell cell;

	if( m < 0 || m >= iMap->map_num || x < 0 || x >= maplist[m].xs || y < 0 || y >= maplist[m].ys )
		return;

	j = x + y*maplist[m].xs;

	cell = map_gat2cell(gat);
	maplist[m].cell[j].walkable = cell.walkable;
	maplist[m].cell[j].shootable = cell.shootable;
	maplist[m].cell[j].water = cell.water;
}

/*==========================================
* Invisible Walls
*------------------------------------------*/
static DBMap* iwall_db;

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

	if( (iwall = (struct iwall_data *)strdb_get(iwall_db, wall_name)) != NULL )
		return false; // Already Exists

	if( iMap->getcell(m, x, y, CELL_CHKNOREACH) )
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
		map_iwall_nextxy(x, y, dir, i, &x1, &y1);

		if( iMap->getcell(m, x1, y1, CELL_CHKNOREACH) )
			break; // Collision

		maplist[m].setcell(m, x1, y1, CELL_WALKABLE, false);
		maplist[m].setcell(m, x1, y1, CELL_SHOOTABLE, shootable);

		clif->changemapcell(0, m, x1, y1, iMap->getcell(m, x1, y1, CELL_GETTYPE), ALL_SAMEMAP);
	}

	iwall->size = i;

	strdb_put(iwall_db, iwall->wall_name, iwall);
	maplist[m].iwall_num++;

	return true;
}

void map_iwall_get(struct map_session_data *sd) {
	struct iwall_data *iwall;
	DBIterator* iter;
	int16 x1, y1;
	int i;

	if( maplist[sd->bl.m].iwall_num < 1 )
		return;

	iter = db_iterator(iwall_db);
	for( iwall = dbi_first(iter); dbi_exists(iter); iwall = dbi_next(iter) ) {
		if( iwall->m != sd->bl.m )
			continue;

		for( i = 0; i < iwall->size; i++ ) {
			map_iwall_nextxy(iwall->x, iwall->y, iwall->dir, i, &x1, &y1);
			clif->changemapcell(sd->fd, iwall->m, x1, y1, iMap->getcell(iwall->m, x1, y1, CELL_GETTYPE), SELF);
		}
	}
	dbi_destroy(iter);
}

void map_iwall_remove(const char *wall_name)
{
	struct iwall_data *iwall;
	int16 i, x1, y1;

	if( (iwall = (struct iwall_data *)strdb_get(iwall_db, wall_name)) == NULL )
		return; // Nothing to do

	for( i = 0; i < iwall->size; i++ ) {
		map_iwall_nextxy(iwall->x, iwall->y, iwall->dir, i, &x1, &y1);

		maplist[iwall->m].setcell(iwall->m, x1, y1, CELL_SHOOTABLE, true);
		maplist[iwall->m].setcell(iwall->m, x1, y1, CELL_WALKABLE, true);

		clif->changemapcell(0, iwall->m, x1, y1, iMap->getcell(iwall->m, x1, y1, CELL_GETTYPE), ALL_SAMEMAP);
	}

	maplist[iwall->m].iwall_num--;
	strdb_remove(iwall_db, iwall->wall_name);
}

/**
* @see DBCreateData
*/
static DBData create_map_data_other_server(DBKey key, va_list args)
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

	mdos= uidb_ensure(map_db,(unsigned int)mapindex, create_map_data_other_server);

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
		db_remove(map_db,key);
		aFree(mdos);
	}
	return 0;
}

int map_eraseallipport(void) {
	map_db->foreach(map_db,map_eraseallipport_sub);
	return 1;
}

/*==========================================
* Delete mapindex from db of another map server
*------------------------------------------*/
int map_eraseipport(unsigned short mapindex, uint32 ip, uint16 port) {
	struct map_data_other_server *mdos;

	mdos = (struct map_data_other_server*)uidb_get(map_db,(unsigned int)mapindex);
	if(!mdos || mdos->cell) //Map either does not exists or is a local map.
		return 0;

	if(mdos->ip==ip && mdos->port == port) {
		uidb_remove(map_db,(unsigned int)mapindex);
		aFree(mdos);
		return 1;
	}
	return 0;
}

/*==========================================
* [Shinryo]: Init the mapcache
*------------------------------------------*/
static char *map_init_mapcache(FILE *fp)
{
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


int map_addmap(char* mapname) {
	maplist[iMap->map_num].instance_id = -1;
	mapindex_getmapname(mapname, maplist[iMap->map_num++].name);
	return 0;
}

static void map_delmapid(int id) {
	ShowNotice("Removing map [ %s ] from maplist"CL_CLL"\n",maplist[id].name);
	memmove(maplist+id, maplist+id+1, sizeof(maplist[0])*(iMap->map_num-id-1));
	iMap->map_num--;
}

int map_delmap(char* mapname) {
	int i;
	char map_name[MAP_NAME_LENGTH];

	if (strcmpi(mapname, "all") == 0) {
		iMap->map_num = 0;
		return 0;
	}

	mapindex_getmapname(mapname, map_name);
	for(i = 0; i < iMap->map_num; i++) {
		if (strcmp(maplist[i].name, map_name) == 0) {
			map_delmapid(i);
			return 1;
		}
	}
	return 0;
}
void map_zone_db_clear(void) {
	struct map_zone_data *zone;
	int i;

	DBIterator *iter = db_iterator(zone_db);
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

	db_destroy(zone_db);/* will aFree(zone) */

	/* clear the pk zone stuff */
	for(i = 0; i < map_zone_pk.disabled_skills_count; i++) {
		aFree(map_zone_pk.disabled_skills[i]);
	}
	aFree(map_zone_pk.disabled_skills);
	aFree(map_zone_pk.disabled_items);
	for(i = 0; i < map_zone_pk.mapflags_count; i++) {
		aFree(map_zone_pk.mapflags[i]);
	}
	aFree(map_zone_pk.mapflags);
	for(i = 0; i < map_zone_pk.disabled_commands_count; i++) {
		aFree(map_zone_pk.disabled_commands[i]);
	}
	aFree(map_zone_pk.disabled_commands);
	for(i = 0; i < map_zone_pk.capped_skills_count; i++) {
		aFree(map_zone_pk.capped_skills[i]);
	}
	aFree(map_zone_pk.capped_skills);
	/* clear the main zone stuff */
	for(i = 0; i < map_zone_all.disabled_skills_count; i++) {
		aFree(map_zone_all.disabled_skills[i]);
	}
	aFree(map_zone_all.disabled_skills);
	aFree(map_zone_all.disabled_items);
	for(i = 0; i < map_zone_all.mapflags_count; i++) {
		aFree(map_zone_all.mapflags[i]);
	}
	aFree(map_zone_all.mapflags);
	for(i = 0; i < map_zone_all.disabled_commands_count; i++) {
		aFree(map_zone_all.disabled_commands[i]);
	}
	aFree(map_zone_all.disabled_commands);
	for(i = 0; i < map_zone_all.capped_skills_count; i++) {
		aFree(map_zone_all.capped_skills[i]);
	}
	aFree(map_zone_all.capped_skills);
}
void map_clean(int i) {
	int v;
	if(maplist[i].cell && maplist[i].cell != (struct mapcell *)0xdeadbeaf) aFree(maplist[i].cell);
	if(maplist[i].block) aFree(maplist[i].block);
	if(maplist[i].block_mob) aFree(maplist[i].block_mob);

	if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
		int j;
		if(maplist[i].mob_delete_timer != INVALID_TIMER)
			timer->delete(maplist[i].mob_delete_timer, iMap->removemobs_timer);
		for (j=0; j<MAX_MOB_LIST_PER_MAP; j++)
			if (maplist[i].moblist[j]) aFree(maplist[i].moblist[j]);
	}

	if( maplist[i].unit_count ) {
		for(v = 0; v < maplist[i].unit_count; v++) {
			aFree(maplist[i].units[v]);
		}
		if( maplist[i].units ) {
			aFree(maplist[i].units);
			maplist[i].units = NULL;
		}
		maplist[i].unit_count = 0;
	}

	if( maplist[i].skill_count ) {
		for(v = 0; v < maplist[i].skill_count; v++) {
			aFree(maplist[i].skills[v]);
		}
		if( maplist[i].skills ) {
			aFree(maplist[i].skills);
			maplist[i].skills = NULL;
		}
		maplist[i].skill_count = 0;
	}

	if( maplist[i].zone_mf_count ) {
		for(v = 0; v < maplist[i].zone_mf_count; v++) {
			aFree(maplist[i].zone_mf[v]);
		}
		if( maplist[i].zone_mf ) {
			aFree(maplist[i].zone_mf);
			maplist[i].zone_mf = NULL;
		}
		maplist[i].zone_mf_count = 0;
	}

	if( maplist[i].channel )
		clif->chsys_delete(maplist[i].channel);
}
void do_final_maps(void) {
	int i, v = 0;

	for( i = 0; i < iMap->map_num; i++ ) {

		if(maplist[i].cell && maplist[i].cell != (struct mapcell *)0xdeadbeaf ) aFree(maplist[i].cell);
		if(maplist[i].block) aFree(maplist[i].block);
		if(maplist[i].block_mob) aFree(maplist[i].block_mob);

		if(battle_config.dynamic_mobs) { //Dynamic mobs flag by [random]
			int j;
			if(maplist[i].mob_delete_timer != INVALID_TIMER)
				timer->delete(maplist[i].mob_delete_timer, iMap->removemobs_timer);
			for (j=0; j<MAX_MOB_LIST_PER_MAP; j++)
				if (maplist[i].moblist[j]) aFree(maplist[i].moblist[j]);
		}

		if( maplist[i].unit_count ) {
			for(v = 0; v < maplist[i].unit_count; v++) {
				aFree(maplist[i].units[v]);
			}
			if( maplist[i].units ) {
				aFree(maplist[i].units);
				maplist[i].units = NULL;
			}
			maplist[i].unit_count = 0;
		}

		if( maplist[i].skill_count ) {
			for(v = 0; v < maplist[i].skill_count; v++) {
				aFree(maplist[i].skills[v]);
			}
			if( maplist[i].skills ) {
				aFree(maplist[i].skills);
				maplist[i].skills = NULL;
			}
			maplist[i].skill_count = 0;
		}

		if( maplist[i].zone_mf_count ) {
			for(v = 0; v < maplist[i].zone_mf_count; v++) {
				aFree(maplist[i].zone_mf[v]);
			}
			if( maplist[i].zone_mf ) {
				aFree(maplist[i].zone_mf);
				maplist[i].zone_mf = NULL;
			}
			maplist[i].zone_mf_count = 0;
		}

		if( maplist[i].drop_list_count ) {
			maplist[i].drop_list_count = 0;
		}
		if( maplist[i].drop_list != NULL )
			aFree(maplist[i].drop_list);

		if( maplist[i].channel )
			clif->chsys_delete(maplist[i].channel);
	}

	map_zone_db_clear();

}
/// Initializes map flags and adjusts them depending on configuration.
void map_flags_init(void) {
	int i, v = 0;

	for( i = 0; i < iMap->map_num; i++ ) {
		// mapflags
		memset(&maplist[i].flag, 0, sizeof(maplist[i].flag));

		// additional mapflag data
		maplist[i].nocommand = 0;   // nocommand mapflag level
		maplist[i].bexp      = 100; // per map base exp multiplicator
		maplist[i].jexp      = 100; // per map job exp multiplicator
		if( maplist[i].drop_list != NULL )
			aFree(maplist[i].drop_list);
		maplist[i].drop_list = NULL;
		maplist[i].drop_list_count = 0;

		if( maplist[i].unit_count ) {
			for(v = 0; v < maplist[i].unit_count; v++) {
				aFree(maplist[i].units[v]);
			}
			aFree(maplist[i].units);
		}
		maplist[i].units = NULL;
		maplist[i].unit_count = 0;

		if( maplist[i].skill_count ) {
			for(v = 0; v < maplist[i].skill_count; v++) {
				aFree(maplist[i].skills[v]);
			}
			aFree(maplist[i].skills);
		}
		maplist[i].skills = NULL;
		maplist[i].skill_count = 0;

		// adjustments
		if( battle_config.pk_mode ) {
			maplist[i].flag.pvp = 1; // make all maps pvp for pk_mode [Valaris]
			maplist[i].zone = &map_zone_pk;
		} else /* align with 'All' zone */
			maplist[i].zone = &map_zone_all;

		if( maplist[i].zone_mf_count ) {
			for(v = 0; v < maplist[i].zone_mf_count; v++) {
				aFree(maplist[i].zone_mf[v]);
			}
			aFree(maplist[i].zone_mf);
		}

		maplist[i].zone_mf = NULL;
		maplist[i].zone_mf_count = 0;
		maplist[i].prev_zone = maplist[i].zone;

		maplist[i].invincible_time_inc = 0;

		maplist[i].weapon_damage_rate = 100;
		maplist[i].magic_damage_rate  = 100;
		maplist[i].misc_damage_rate   = 100;
		maplist[i].short_damage_rate  = 100;
		maplist[i].long_damage_rate   = 100;
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

	water_height = map_waterheight(m->name);

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

		m->cell[xy] = map_gat2cell(type);
	}

	aFree(gat);

	return 1;
}

/*======================================
* Add/Remove map to the map_db
*--------------------------------------*/
void map_addmap2db(struct map_data *m) {
	index2mapid[m->index] = m->m;
}

void map_removemapdb(struct map_data *m) {
	index2mapid[m->index] = -1;
}

/*======================================
* Initiate maps loading stage
*--------------------------------------*/
int map_readallmaps (void) {
	int i;
	FILE* fp=NULL;
	int maps_removed = 0;

	if( enable_grf )
		ShowStatus("Loading maps (using GRF files)...\n");
	else {
		char mapcachefilepath[254];
		sprintf(mapcachefilepath,"%s/%s%s",iMap->db_path,DBPATH,"map_cache.dat");
		ShowStatus("Loading maps (using %s as map cache)...\n", mapcachefilepath);
		if( (fp = fopen(mapcachefilepath, "rb")) == NULL ) {
			ShowFatalError("Unable to open map cache file "CL_WHITE"%s"CL_RESET"\n", mapcachefilepath);
			exit(EXIT_FAILURE); //No use launching server if maps can't be read.
		}

		// Init mapcache data.. [Shinryo]
		map_cache_buffer = map_init_mapcache(fp);
		if(!map_cache_buffer) {
			ShowFatalError("Failed to initialize mapcache data (%s)..\n", mapcachefilepath);
			exit(EXIT_FAILURE);
		}
	}

	for(i = 0; i < iMap->map_num; i++) {
		size_t size;

		// show progress
		if(enable_grf)
			ShowStatus("Loading maps [%i/%i]: %s"CL_CLL"\r", i, iMap->map_num, maplist[i].name);

		// try to load the map
		if( !
			(enable_grf?
			map_readgat(&maplist[i])
			:map_readfromcache(&maplist[i], map_cache_buffer))
			) {
				map_delmapid(i);
				maps_removed++;
				i--;
				continue;
		}

		maplist[i].index = mapindex_name2id(maplist[i].name);

		if ( index2mapid[map_id2index(i)] != -1 ) {
			ShowWarning("Map %s already loaded!"CL_CLL"\n", maplist[i].name);
			if (maplist[i].cell && maplist[i].cell != (struct mapcell *)0xdeadbeaf) {
				aFree(maplist[i].cell);
				maplist[i].cell = NULL;
			}
			map_delmapid(i);
			maps_removed++;
			i--;
			continue;
		}

		maplist[i].m = i;
		iMap->addmap2db(&maplist[i]);

		memset(maplist[i].moblist, 0, sizeof(maplist[i].moblist));	//Initialize moblist [Skotlex]
		maplist[i].mob_delete_timer = INVALID_TIMER;	//Initialize timer [Skotlex]

		maplist[i].bxs = (maplist[i].xs + BLOCK_SIZE - 1) / BLOCK_SIZE;
		maplist[i].bys = (maplist[i].ys + BLOCK_SIZE - 1) / BLOCK_SIZE;

		size = maplist[i].bxs * maplist[i].bys * sizeof(struct block_list*);
		maplist[i].block = (struct block_list**)aCalloc(size, 1);
		maplist[i].block_mob = (struct block_list**)aCalloc(size, 1);

		maplist[i].getcellp = map_sub_getcellp;
		maplist[i].setcell  = map_sub_setcell;
	}

	// intialization and configuration-dependent adjustments of mapflags
	iMap->flags_init();

	if( !enable_grf ) {
		fclose(fp);
	}

	// finished map loading
	ShowInfo("Successfully loaded '"CL_WHITE"%d"CL_RESET"' maps."CL_CLL"\n",iMap->map_num);
	instance->start_id = iMap->map_num; // Next Map Index will be instances

	if (maps_removed)
		ShowNotice("Maps removed: '"CL_WHITE"%d"CL_RESET"'\n",maps_removed);

	return 0;
}

////////////////////////////////////////////////////////////////////////
static int map_ip_set = 0;
static int char_ip_set = 0;

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
			char_ip_set = chrif->setip(w2);
		else if (strcmpi(w1, "char_port") == 0)
			chrif->setport(atoi(w2));
		else if (strcmpi(w1, "map_ip") == 0)
			map_ip_set = clif->setip(w2);
		else if (strcmpi(w1, "bind_ip") == 0)
			clif->setbindip(w2);
		else if (strcmpi(w1, "map_port") == 0) {
			clif->setport(atoi(w2));
			map_port = (atoi(w2));
		} else if (strcmpi(w1, "map") == 0)
			iMap->map_num++;
		else if (strcmpi(w1, "delmap") == 0)
			iMap->map_num--;
		else if (strcmpi(w1, "npc") == 0)
			npc->addsrcfile(w2);
		else if (strcmpi(w1, "delnpc") == 0)
			npc->delsrcfile(w2);
		else if (strcmpi(w1, "autosave_time") == 0) {
			iMap->autosave_interval = atoi(w2);
			if (iMap->autosave_interval < 1) //Revert to default saving.
				iMap->autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
			else
				iMap->autosave_interval *= 1000; //Pass from sec to ms
		} else if (strcmpi(w1, "minsave_time") == 0) {
			iMap->minsave_interval= atoi(w2);
			if (iMap->minsave_interval < 1)
				iMap->minsave_interval = 1;
		} else if (strcmpi(w1, "save_settings") == 0)
			iMap->save_settings = atoi(w2);
		else if (strcmpi(w1, "help_txt") == 0)
			strcpy(iMap->help_txt, w2);
		else if (strcmpi(w1, "help2_txt") == 0)
			strcpy(iMap->help2_txt, w2);
		else if (strcmpi(w1, "charhelp_txt") == 0)
			strcpy(iMap->charhelp_txt, w2);
		else if(strcmpi(w1,"db_path") == 0)
			safestrncpy(iMap->db_path,w2,255);
		else if (strcmpi(w1, "enable_spy") == 0)
			iMap->enable_spy = config_switch(w2);
		else if (strcmpi(w1, "use_grf") == 0)
			enable_grf = config_switch(w2);
		else if (strcmpi(w1, "console_msg_log") == 0)
			console_msg_log = atoi(w2);//[Ind]
		else if (strcmpi(w1, "import") == 0)
			map_config_read(w2);
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
			map_addmap(w2);
		else if (strcmpi(w1, "delmap") == 0)
			iMap->delmap(w2);
		else if (strcmpi(w1, "import") == 0)
			map_config_read_sub(w2);
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
			map_reloadnpc_sub(w2);
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
	map_reloadnpc_sub("npc/re/scripts_main.conf");
#else
	map_reloadnpc_sub("npc/pre-re/scripts_main.conf");
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
			strcpy(iMap->item_db_db,w2);
		else if(strcmpi(w1,"mob_db_db")==0)
			strcpy(iMap->mob_db_db,w2);
		else if(strcmpi(w1,"item_db2_db")==0)
			strcpy(iMap->item_db2_db,w2);
		else if(strcmpi(w1,"item_db_re_db")==0)
			strcpy(iMap->item_db_re_db,w2);
		else if(strcmpi(w1,"mob_db2_db")==0)
			strcpy(iMap->mob_db2_db,w2);
		else if(strcmpi(w1,"mob_skill_db_db")==0)
			strcpy(iMap->mob_skill_db_db,w2);
		else if(strcmpi(w1,"mob_skill_db2_db")==0)
			strcpy(iMap->mob_skill_db2_db,w2);
		else if(strcmpi(w1,"interreg_db")==0)
			strcpy(iMap->interreg_db,w2);
		/* map sql stuff */
		else if(strcmpi(w1,"map_server_ip")==0)
			strcpy(map_server_ip, w2);
		else if(strcmpi(w1,"map_server_port")==0)
			map_server_port=atoi(w2);
		else if(strcmpi(w1,"map_server_id")==0)
			strcpy(map_server_id, w2);
		else if(strcmpi(w1,"map_server_pw")==0)
			strcpy(map_server_pw, w2);
		else if(strcmpi(w1,"map_server_db")==0)
			strcpy(map_server_db, w2);
		else if(strcmpi(w1,"default_codepage")==0)
			strcpy(default_codepage, w2);
		else if(strcmpi(w1,"use_sql_item_db")==0) {
			iMap->db_use_sql_item_db = config_switch(w2);
			ShowStatus ("Using item database as SQL: '%s'\n", w2);
		}
		else if(strcmpi(w1,"use_sql_mob_db")==0) {
			iMap->db_use_sql_mob_db = config_switch(w2);
			ShowStatus ("Using monster database as SQL: '%s'\n", w2);
		}
		else if(strcmpi(w1,"use_sql_mob_skill_db")==0) {
			iMap->db_use_sql_mob_skill_db = config_switch(w2);
			ShowStatus ("Using monster skill database as SQL: '%s'\n", w2);
		}
		/* sql log db */
		else if(strcmpi(w1,"log_db_ip")==0)
			strcpy(log_db_ip, w2);
		else if(strcmpi(w1,"log_db_id")==0)
			strcpy(log_db_id, w2);
		else if(strcmpi(w1,"log_db_pw")==0)
			strcpy(log_db_pw, w2);
		else if(strcmpi(w1,"log_db_port")==0)
			log_db_port = atoi(w2);
		else if(strcmpi(w1,"log_db_db")==0)
			strcpy(log_db_db, w2);
		/* mapreg */
		else if( mapreg->config_read(w1,w2) )
			continue;
		/* import */
		else if(strcmpi(w1,"import")==0)
			inter_config_read(w2);
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
	mmysql_handle = SQL->Malloc();

	ShowInfo("Connecting to the Map DB Server....\n");
	if( SQL_ERROR == SQL->Connect(mmysql_handle, map_server_id, map_server_pw, map_server_ip, map_server_port, map_server_db) )
		exit(EXIT_FAILURE);
	ShowStatus("connect success! (Map Server Connection)\n");

	if( strlen(default_codepage) > 0 )
		if ( SQL_ERROR == SQL->SetEncoding(mmysql_handle, default_codepage) )
			Sql_ShowDebug(mmysql_handle);

	return 0;
}

int map_sql_close(void)
{
	ShowStatus("Close Map DB Connection....\n");
	SQL->Free(mmysql_handle);
	mmysql_handle = NULL;
	if (logs->config.sql_logs) {
		ShowStatus("Close Log DB Connection....\n");
		SQL->Free(logmysql_handle);
		logmysql_handle = NULL;
	}
	return 0;
}

int log_sql_init(void)
{
	// log db connection
	logmysql_handle = SQL->Malloc();

	ShowInfo(""CL_WHITE"[SQL]"CL_RESET": Connecting to the Log Database "CL_WHITE"%s"CL_RESET" At "CL_WHITE"%s"CL_RESET"...\n",log_db_db,log_db_ip);
	if ( SQL_ERROR == SQL->Connect(logmysql_handle, log_db_id, log_db_pw, log_db_ip, log_db_port, log_db_db) )
		exit(EXIT_FAILURE);
	ShowStatus(""CL_WHITE"[SQL]"CL_RESET": Successfully '"CL_GREEN"connected"CL_RESET"' to Database '"CL_WHITE"%s"CL_RESET"'.\n", log_db_db);

	if( strlen(default_codepage) > 0 )
		if ( SQL_ERROR == SQL->SetEncoding(logmysql_handle, default_codepage) )
			Sql_ShowDebug(logmysql_handle);
	return 0;
}
void map_zone_change2(int m, struct map_zone_data *zone) {
	char empty[1] = "\0";

	maplist[m].prev_zone = maplist[m].zone;

	if( maplist[m].zone_mf_count )
		iMap->zone_remove(m);

	iMap->zone_apply(m,zone,empty,empty,empty);
}
/* when changing from a mapflag to another during runtime */
void map_zone_change(int m, struct map_zone_data *zone, const char* start, const char* buffer, const char* filepath) {
	maplist[m].prev_zone = maplist[m].zone;

	if( maplist[m].zone_mf_count )
		iMap->zone_remove(m);
	iMap->zone_apply(m,zone,start,buffer,filepath);
}
/* removes previous mapflags from this map */
void map_zone_remove(int m) {
	char flag[MAP_ZONE_MAPFLAG_LENGTH], params[MAP_ZONE_MAPFLAG_LENGTH];
	unsigned short k;
	char empty[1] = "\0";
	for(k = 0; k < maplist[m].zone_mf_count; k++) {
		int len = strlen(maplist[m].zone_mf[k]),j;
		params[0] = '\0';
		memcpy(flag, maplist[m].zone_mf[k], MAP_ZONE_MAPFLAG_LENGTH);
		for(j = 0; j < len; j++) {
			if( flag[j] == '\t' ) {
				memcpy(params, &flag[j+1], len - j);
				flag[j] = '\0';
				break;
			}
		}

		npc->parse_mapflag(maplist[m].name,empty,flag,params,empty,empty,empty);
		aFree(maplist[m].zone_mf[k]);
		maplist[m].zone_mf[k] = NULL;
	}

	aFree(maplist[m].zone_mf);
	maplist[m].zone_mf = NULL;
	maplist[m].zone_mf_count = 0;
}
static inline void map_zone_mf_cache_add(int m, char *rflag) {
	RECREATE(maplist[m].zone_mf, char *, ++maplist[m].zone_mf_count);
	CREATE(maplist[m].zone_mf[maplist[m].zone_mf_count - 1], char, MAP_ZONE_MAPFLAG_LENGTH);
	safestrncpy(maplist[m].zone_mf[maplist[m].zone_mf_count - 1], rflag, MAP_ZONE_MAPFLAG_LENGTH);
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
			if( maplist[m].flag.nosave ) {
				sprintf(rflag, "nosave\tSavePoint");
				map_zone_mf_cache_add(m,nosave);
			}
		} else if (!strcmpi(params, "SavePoint")) {
			if( maplist[m].save.map ) {
				sprintf(rflag, "nosave\t%s,%d,%d",mapindex_id2name(maplist[m].save.map),maplist[m].save.x,maplist[m].save.y);
			} else
				sprintf(rflag, "nosave\t%s,%d,%d",mapindex_id2name(maplist[m].save.map),maplist[m].save.x,maplist[m].save.y);
			map_zone_mf_cache_add(m,nosave);
		} else if (sscanf(params, "%31[^,],%d,%d", savemap, &savex, &savey) == 3) {
			if( maplist[m].save.map ) {
				sprintf(rflag, "nosave\t%s,%d,%d",mapindex_id2name(maplist[m].save.map),maplist[m].save.x,maplist[m].save.y);
				map_zone_mf_cache_add(m,nosave);
			}
		}
#endif // 0
	} else if (!strcmpi(flag,"autotrade")) {
		if( state && maplist[m].flag.autotrade )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"autotrade\toff");
			else if( !maplist[m].flag.autotrade )
				map_zone_mf_cache_add(m,"autotrade");
		}
	} else if (!strcmpi(flag,"allowks")) {
		if( state && maplist[m].flag.allowks )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"allowks\toff");
			else if( !maplist[m].flag.allowks )
				map_zone_mf_cache_add(m,"allowks");
		}
	} else if (!strcmpi(flag,"town")) {
		if( state && maplist[m].flag.town )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"town\toff");
			else if( !maplist[m].flag.town )
				map_zone_mf_cache_add(m,"town");
		}
	} else if (!strcmpi(flag,"nomemo")) {
		if( state && maplist[m].flag.nomemo )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomemo\toff");
			else if( !maplist[m].flag.nomemo )
				map_zone_mf_cache_add(m,"nomemo");
		}
	} else if (!strcmpi(flag,"noteleport")) {
		if( state && maplist[m].flag.noteleport )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noteleport\toff");
			else if( !maplist[m].flag.noteleport )
				map_zone_mf_cache_add(m,"noteleport");
		}
	} else if (!strcmpi(flag,"nowarp")) {
		if( state && maplist[m].flag.nowarp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nowarp\toff");
			else if( !maplist[m].flag.nowarp )
				map_zone_mf_cache_add(m,"nowarp");
		}
	} else if (!strcmpi(flag,"nowarpto")) {
		if( state && maplist[m].flag.nowarpto )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nowarpto\toff");
			else if( !maplist[m].flag.nowarpto )
				map_zone_mf_cache_add(m,"nowarpto");
		}
	} else if (!strcmpi(flag,"noreturn")) {
		if( state && maplist[m].flag.noreturn )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noreturn\toff");
			else if( maplist[m].flag.noreturn )
				map_zone_mf_cache_add(m,"noreturn");
		}
	} else if (!strcmpi(flag,"monster_noteleport")) {
		if( state && maplist[m].flag.monster_noteleport )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"monster_noteleport\toff");
			else if( maplist[m].flag.monster_noteleport )
				map_zone_mf_cache_add(m,"monster_noteleport");
		}
	} else if (!strcmpi(flag,"nobranch")) {
		if( state && maplist[m].flag.nobranch )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nobranch\toff");
			else if( maplist[m].flag.nobranch )
				map_zone_mf_cache_add(m,"nobranch");
		}
	} else if (!strcmpi(flag,"nopenalty")) {
		if( state && maplist[m].flag.noexppenalty ) /* they are applied together, no need to check both */
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nopenalty\toff");
			else if( maplist[m].flag.noexppenalty )
				map_zone_mf_cache_add(m,"nopenalty");
		}
	} else if (!strcmpi(flag,"pvp")) {
		if( state && maplist[m].flag.pvp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp\toff");
			else if( maplist[m].flag.pvp )
				map_zone_mf_cache_add(m,"pvp");
		}
	}
	else if (!strcmpi(flag,"pvp_noparty")) {
		if( state && maplist[m].flag.pvp_noparty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_noparty\toff");
			else if( maplist[m].flag.pvp_noparty )
				map_zone_mf_cache_add(m,"pvp_noparty");
		}
	} else if (!strcmpi(flag,"pvp_noguild")) {
		if( state && maplist[m].flag.pvp_noguild )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_noguild\toff");
			else if( maplist[m].flag.pvp_noguild )
				map_zone_mf_cache_add(m,"pvp_noguild");
		}
	} else if (!strcmpi(flag, "pvp_nightmaredrop")) {
		if( state && maplist[m].flag.pvp_nightmaredrop )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_nightmaredrop\toff");
			else if( maplist[m].flag.pvp_nightmaredrop )
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
					if (maplist[m].drop_list[i].drop_id == 0){
						maplist[m].drop_list[i].drop_id = drop_id;
						maplist[m].drop_list[i].drop_type = drop_type;
						maplist[m].drop_list[i].drop_per = drop_per;
						break;
					}
				}
				maplist[m].flag.pvp_nightmaredrop = 1;
			}
		} else if (!state) //Disable
			maplist[m].flag.pvp_nightmaredrop = 0;
#endif // 0
	} else if (!strcmpi(flag,"pvp_nocalcrank")) {
		if( state && maplist[m].flag.pvp_nocalcrank )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"pvp_nocalcrank\toff");
			else if( maplist[m].flag.pvp_nocalcrank )
				map_zone_mf_cache_add(m,"pvp_nocalcrank");
		}
	} else if (!strcmpi(flag,"gvg")) {
		if( state && maplist[m].flag.gvg )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg\toff");
			else if( maplist[m].flag.gvg )
				map_zone_mf_cache_add(m,"gvg");
		}
	} else if (!strcmpi(flag,"gvg_noparty")) {
		if( state && maplist[m].flag.gvg_noparty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg_noparty\toff");
			else if( maplist[m].flag.gvg_noparty )
				map_zone_mf_cache_add(m,"gvg_noparty");
		}
	} else if (!strcmpi(flag,"gvg_dungeon")) {
		if( state && maplist[m].flag.gvg_dungeon )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg_dungeon\toff");
			else if( maplist[m].flag.gvg_dungeon )
				map_zone_mf_cache_add(m,"gvg_dungeon");
		}
	}
	else if (!strcmpi(flag,"gvg_castle")) {
		if( state && maplist[m].flag.gvg_castle )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"gvg_castle\toff");
			else if( maplist[m].flag.gvg_castle )
				map_zone_mf_cache_add(m,"gvg_castle");
		}
	}
	else if (!strcmpi(flag,"battleground")) {
		if( state && maplist[m].flag.battleground )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"battleground\toff");
			else if( maplist[m].flag.battleground )
				map_zone_mf_cache_add(m,"battleground");
		}
	} else if (!strcmpi(flag,"noexppenalty")) {
		if( state && maplist[m].flag.noexppenalty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noexppenalty\toff");
			else if( maplist[m].flag.noexppenalty )
				map_zone_mf_cache_add(m,"noexppenalty");
		}
	} else if (!strcmpi(flag,"nozenypenalty")) {
		if( state && maplist[m].flag.nozenypenalty )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nozenypenalty\toff");
			else if( maplist[m].flag.nozenypenalty )
				map_zone_mf_cache_add(m,"nozenypenalty");
		}
	} else if (!strcmpi(flag,"notrade")) {
		if( state && maplist[m].flag.notrade )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"notrade\toff");
			else if( maplist[m].flag.notrade )
				map_zone_mf_cache_add(m,"notrade");
		}
	} else if (!strcmpi(flag,"novending")) {
		if( state && maplist[m].flag.novending )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"novending\toff");
			else if( maplist[m].flag.novending )
				map_zone_mf_cache_add(m,"novending");
		}
	} else if (!strcmpi(flag,"nodrop")) {
		if( state && maplist[m].flag.nodrop )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nodrop\toff");
			else if( maplist[m].flag.nodrop )
				map_zone_mf_cache_add(m,"nodrop");
		}
	} else if (!strcmpi(flag,"noskill")) {
		if( state && maplist[m].flag.noskill )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noskill\toff");
			else if( maplist[m].flag.noskill )
				map_zone_mf_cache_add(m,"noskill");
		}
	} else if (!strcmpi(flag,"noicewall")) {
		if( state && maplist[m].flag.noicewall )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noicewall\toff");
			else if( maplist[m].flag.noicewall )
				map_zone_mf_cache_add(m,"noicewall");
		}
	} else if (!strcmpi(flag,"snow")) {
		if( state && maplist[m].flag.snow )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"snow\toff");
			else if( maplist[m].flag.snow )
				map_zone_mf_cache_add(m,"snow");
		}
	} else if (!strcmpi(flag,"clouds")) {
		if( state && maplist[m].flag.clouds )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"clouds\toff");
			else if( maplist[m].flag.clouds )
				map_zone_mf_cache_add(m,"clouds");
		}
	} else if (!strcmpi(flag,"clouds2")) {
		if( state && maplist[m].flag.clouds2 )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"clouds2\toff");
			else if( maplist[m].flag.clouds2 )
				map_zone_mf_cache_add(m,"clouds2");
		}
	} else if (!strcmpi(flag,"fog")) {
		if( state && maplist[m].flag.fog )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"fog\toff");
			else if( maplist[m].flag.fog )
				map_zone_mf_cache_add(m,"fog");
		}
	} else if (!strcmpi(flag,"fireworks")) {
		if( state && maplist[m].flag.fireworks )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"fireworks\toff");
			else if( maplist[m].flag.fireworks )
				map_zone_mf_cache_add(m,"fireworks");
		}
	} else if (!strcmpi(flag,"sakura")) {
		if( state && maplist[m].flag.sakura )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"sakura\toff");
			else if( maplist[m].flag.sakura )
				map_zone_mf_cache_add(m,"sakura");
		}
	} else if (!strcmpi(flag,"leaves")) {
		if( state && maplist[m].flag.leaves )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"leaves\toff");
			else if( maplist[m].flag.leaves )
				map_zone_mf_cache_add(m,"leaves");
		}
	} else if (!strcmpi(flag,"nightenabled")) {
		if( state && maplist[m].flag.nightenabled )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nightenabled\toff");
			else if( maplist[m].flag.nightenabled )
				map_zone_mf_cache_add(m,"nightenabled");
		}
	} else if (!strcmpi(flag,"noexp")) {
		if( state && maplist[m].flag.nobaseexp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noexp\toff");
			else if( maplist[m].flag.nobaseexp )
				map_zone_mf_cache_add(m,"noexp");
		}
	}
	else if (!strcmpi(flag,"nobaseexp")) {
		if( state && maplist[m].flag.nobaseexp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nobaseexp\toff");
			else if( maplist[m].flag.nobaseexp )
				map_zone_mf_cache_add(m,"nobaseexp");
		}
	} else if (!strcmpi(flag,"nojobexp")) {
		if( state && maplist[m].flag.nojobexp )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nojobexp\toff");
			else if( maplist[m].flag.nojobexp )
				map_zone_mf_cache_add(m,"nojobexp");
		}
	} else if (!strcmpi(flag,"noloot")) {
		if( state && maplist[m].flag.nomobloot )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noloot\toff");
			else if( maplist[m].flag.nomobloot )
				map_zone_mf_cache_add(m,"noloot");
		}
	} else if (!strcmpi(flag,"nomobloot")) {
		if( state && maplist[m].flag.nomobloot )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomobloot\toff");
			else if( maplist[m].flag.nomobloot )
				map_zone_mf_cache_add(m,"nomobloot");
		}
	} else if (!strcmpi(flag,"nomvploot")) {
		if( state && maplist[m].flag.nomvploot )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomvploot\toff");
			else if( maplist[m].flag.nomvploot )
				map_zone_mf_cache_add(m,"nomvploot");
		}
	} else if (!strcmpi(flag,"nocommand")) {
		/* implementation may be incomplete */
		if( state && sscanf(params, "%d", &state) == 1 ) {
			sprintf(rflag, "nocommand\t%s",params);
			map_zone_mf_cache_add(m,rflag);
		} else if( !state && maplist[m].nocommand ) {
			sprintf(rflag, "nocommand\t%d",maplist[m].nocommand);
			map_zone_mf_cache_add(m,rflag);
		} else if( maplist[m].nocommand ) {
			map_zone_mf_cache_add(m,"nocommand\toff");
		}
	} else if (!strcmpi(flag,"jexp")) {
		if( !state ) {
			if( maplist[m].jexp != 100 ) {
				sprintf(rflag,"jexp\t%d",maplist[m].jexp);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].jexp ) {
				sprintf(rflag,"jexp\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if (!strcmpi(flag,"bexp")) {
		if( !state ) {
			if( maplist[m].bexp != 100 ) {
				sprintf(rflag,"bexp\t%d",maplist[m].jexp);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].bexp ) {
				sprintf(rflag,"bexp\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if (!strcmpi(flag,"loadevent")) {
		if( state && maplist[m].flag.loadevent )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"loadevent\toff");
			else if( maplist[m].flag.loadevent )
				map_zone_mf_cache_add(m,"loadevent");
		}
	} else if (!strcmpi(flag,"nochat")) {
		if( state && maplist[m].flag.nochat )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nochat\toff");
			else if( maplist[m].flag.nochat )
				map_zone_mf_cache_add(m,"nochat");
		}
	} else if (!strcmpi(flag,"partylock")) {
		if( state && maplist[m].flag.partylock )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"partylock\toff");
			else if( maplist[m].flag.partylock )
				map_zone_mf_cache_add(m,"partylock");
		}
	} else if (!strcmpi(flag,"guildlock")) {
		if( state && maplist[m].flag.guildlock )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"guildlock\toff");
			else if( maplist[m].flag.guildlock )
				map_zone_mf_cache_add(m,"guildlock");
		}
	} else if (!strcmpi(flag,"reset")) {
		if( state && maplist[m].flag.reset )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"reset\toff");
			else if( maplist[m].flag.reset )
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
			int idx = maplist[m].unit_count;

			k = 0;
			ARR_FIND(0, idx, k, maplist[m].units[k]->skill_id == skill_id);

			if( k < idx ) {
				if( atoi(modifier) != maplist[m].units[k]->modifier ) {
					sprintf(rflag,"adjust_unit_duration\t%s\t%d",skill_name,maplist[m].units[k]->modifier);
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
			int idx = maplist[m].skill_count;

			k = 0;
			ARR_FIND(0, idx, k, maplist[m].skills[k]->skill_id == skill_id);

			if( k < idx ) {
				if( atoi(modifier) != maplist[m].skills[k]->modifier ) {
					sprintf(rflag,"adjust_skill_damage\t%s\t%d",skill_name,maplist[m].skills[k]->modifier);
					map_zone_mf_cache_add(m,rflag);
				}
			} else {
				sprintf(rflag,"adjust_skill_damage\t%s\t100",skill_name);
				map_zone_mf_cache_add(m,rflag);
			}

		}
	} else if (!strcmpi(flag,"zone")) {
		ShowWarning("You can't add a zone through a zone! ERROR, skipping for '%s'...\n",maplist[m].name);
		return true;
	} else if ( !strcmpi(flag,"nomapchannelautojoin") ) {
		if( state && maplist[m].flag.chsysnolocalaj )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"nomapchannelautojoin\toff");
			else if( maplist[m].flag.chsysnolocalaj )
				map_zone_mf_cache_add(m,"nomapchannelautojoin");
		}
	} else if ( !strcmpi(flag,"invincible_time_inc") ) {
		if( !state ) {
			if( maplist[m].invincible_time_inc != 0 ) {
				sprintf(rflag,"invincible_time_inc\t%d",maplist[m].invincible_time_inc);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].invincible_time_inc ) {
				sprintf(rflag,"invincible_time_inc\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"noknockback") ) {
		if( state && maplist[m].flag.noknockback )
			;/* nothing to do */
		else {
			if( state )
				map_zone_mf_cache_add(m,"noknockback\toff");
			else if( maplist[m].flag.noknockback )
				map_zone_mf_cache_add(m,"noknockback");
		}
	} else if ( !strcmpi(flag,"weapon_damage_rate") ) {
		if( !state ) {
			if( maplist[m].weapon_damage_rate != 100 ) {
				sprintf(rflag,"weapon_damage_rate\t%d",maplist[m].weapon_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].weapon_damage_rate ) {
				sprintf(rflag,"weapon_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"magic_damage_rate") ) {
		if( !state ) {
			if( maplist[m].magic_damage_rate != 100 ) {
				sprintf(rflag,"magic_damage_rate\t%d",maplist[m].magic_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].magic_damage_rate ) {
				sprintf(rflag,"magic_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"misc_damage_rate") ) {
		if( !state ) {
			if( maplist[m].misc_damage_rate != 100 ) {
				sprintf(rflag,"misc_damage_rate\t%d",maplist[m].misc_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].misc_damage_rate ) {
				sprintf(rflag,"misc_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"short_damage_rate") ) {
		if( !state ) {
			if( maplist[m].short_damage_rate != 100 ) {
				sprintf(rflag,"short_damage_rate\t%d",maplist[m].short_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].short_damage_rate ) {
				sprintf(rflag,"short_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	} else if ( !strcmpi(flag,"long_damage_rate") ) {
		if( !state ) {
			if( maplist[m].long_damage_rate != 100 ) {
				sprintf(rflag,"long_damage_rate\t%d",maplist[m].long_damage_rate);
				map_zone_mf_cache_add(m,rflag);
			}
		} if( sscanf(params, "%d", &state) == 1 ) {
			if( state != maplist[m].long_damage_rate ) {
				sprintf(rflag,"long_damage_rate\t%s",params);
				map_zone_mf_cache_add(m,rflag);
			}
		}
	}
	return false;
}
void map_zone_apply(int m, struct map_zone_data *zone, const char* start, const char* buffer, const char* filepath) {
	int i;
	char empty[1] = "\0";
	char flag[MAP_ZONE_MAPFLAG_LENGTH], params[MAP_ZONE_MAPFLAG_LENGTH];
	maplist[m].zone = zone;
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

		if( map_zone_mf_cache(m,flag,params) )
			continue;

		npc->parse_mapflag(maplist[m].name,empty,flag,params,start,buffer,filepath);
	}
}
/* used on npc load and reload to apply all "Normal" and "PK Mode" zones */
void map_zone_init(void) {
	char flag[MAP_ZONE_MAPFLAG_LENGTH], params[MAP_ZONE_MAPFLAG_LENGTH];
	struct map_zone_data *zone;
	char empty[1] = "\0";
	int i,k,j;

	zone = &map_zone_all;

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

		for(j = 0; j < iMap->map_num; j++) {
			if( maplist[j].zone == zone ) {
				if( map_zone_mf_cache(j,flag,params) )
					break;
				npc->parse_mapflag(maplist[j].name,empty,flag,params,empty,empty,empty);
			}
		}
	}

	if( battle_config.pk_mode ) {
		zone = &map_zone_pk;
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
			for(j = 0; j < iMap->map_num; j++) {
				if( maplist[j].zone == zone ) {
					if( map_zone_mf_cache(j,flag,params) )
						break;
					npc->parse_mapflag(maplist[j].name,empty,flag,params,empty,empty,empty);
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
		if( !( nameid = strdb_iget(skilldb_name2id, name) ) ) {
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

			if( strdb_exists(zone_db, zonename) ) {
				ShowError("map_zone_db: duplicate zone name '%s', skipping...\n",zonename);
				config_setting_remove_elem(zones,i);/* remove from the tree */
				--zone_count;
				--i;
				continue;
			}

			/* is this the global template? */
			if( strncmpi(zonename,MAP_ZONE_NORMAL_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
				zone = &map_zone_all;
				is_all = true;
			} else if( strncmpi(zonename,MAP_ZONE_PK_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
				zone = &map_zone_pk;
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
					if( !map_zone_str2skillid(name) ) {
						ShowError("map_zone_db: unknown skill (%s) in disabled_skills for zone '%s', skipping skill...\n",name,zone->name);
						config_setting_remove_elem(skills,h);
						--disabled_skills_count;
						--h;
						continue;
					}
					if( !map_zone_bl_type(config_setting_get_string_elem(skills,h),&subtype) )/* we dont remove it from the three due to inheritance */
						--disabled_skills_count;
				}
				/* all ok, process */
				CREATE( zone->disabled_skills, struct map_zone_disabled_skill_entry *, disabled_skills_count );
				for(h = 0, v = 0; h < config_setting_length(skills); h++) {
					config_setting_t *skillinfo = config_setting_get_elem(skills, h);
					struct map_zone_disabled_skill_entry * entry;
					enum bl_type type;
					name = config_setting_name(skillinfo);

					if( (type = map_zone_bl_type(config_setting_get_string_elem(skills,h),&subtype)) ) { /* only add if enabled */
						CREATE( entry, struct map_zone_disabled_skill_entry, 1 );

						entry->nameid = map_zone_str2skillid(name);
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
					if( !map_zone_str2itemid(name) ) {
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
						zone->disabled_items[v++] = map_zone_str2itemid(name);
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
					if( !map_zone_str2skillid(name) ) {
						ShowError("map_zone_db: unknown skill (%s) in skill_damage_cap for zone '%s', skipping skill...\n",name,zone->name);
						config_setting_remove_elem(caps,h);
						--capped_skills_count;
						--h;
						continue;
					}
					if( !map_zone_bl_type(config_setting_get_string_elem(cap,1),&subtype) )/* we dont remove it from the three due to inheritance */
						--capped_skills_count;
				}
				/* all ok, process */
				CREATE( zone->capped_skills, struct map_zone_skill_damage_cap_entry *, capped_skills_count );
				for(h = 0, v = 0; h < config_setting_length(caps); h++) {
					config_setting_t *cap = config_setting_get_elem(caps, h);
					struct map_zone_skill_damage_cap_entry * entry;
					enum bl_type type;
					name = config_setting_name(cap);

					if( (type = map_zone_bl_type(config_setting_get_string_elem(cap,1),&subtype)) ) { /* only add if enabled */
						CREATE( entry, struct map_zone_skill_damage_cap_entry, 1 );

						entry->nameid = map_zone_str2skillid(name);
						entry->cap = config_setting_get_int_elem(cap,0);
						entry->type = type;
						entry->subtype = subtype;
						zone->capped_skills[v++] = entry;
					}
				}
				zone->capped_skills_count = capped_skills_count;
			}

			if( !is_all ) /* global template doesn't go into db -- since it isn't a alloc'd piece of data */
				strdb_put(zone_db, zonename, zone);

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

				if( !(izone = strdb_get(zone_db, name)) ) {
					ShowError("map_zone_db: Unknown zone '%s' being inherit by zone '%s', skipping...\n",name,zonename);
					continue;
				}

				if( strncmpi(zonename,MAP_ZONE_NORMAL_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
					zone = &map_zone_all;
				} else if( strncmpi(zonename,MAP_ZONE_PK_NAME,MAP_ZONE_NAME_LENGTH) == 0 ) {
					zone = &map_zone_pk;
				} else
					zone = strdb_get(zone_db, zonename);/* will succeed for we just put it in here */

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
							if( map_zone_str2skillid(config_setting_name(skillinfo)) == izone->disabled_skills[j]->nameid ) {
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

							if( map_zone_str2itemid(name) == izone->disabled_items[j] ) {
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
							if( map_zone_str2skillid(config_setting_name(cap)) == izone->capped_skills[j]->nameid ) {
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
		iMap->quit((struct map_session_data *) bl);
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
		iMap->clearflooritem(bl);
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
static int cleanup_db_sub(DBKey key, DBData *data, va_list va)
{
	return iMap->cleanup_sub(DB->data2ptr(data), va);
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
	if (cpsd) aFree(cpsd);

	//Ladies and babies first.
	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) )
		iMap->quit(sd);
	mapit->free(iter);

	/* prepares npcs for a faster shutdown process */
	npc->do_clear_npc();

	// remove all objects on maps
	for (i = 0; i < iMap->map_num; i++) {
		ShowStatus("Cleaning up maps [%d/%d]: %s..."CL_CLL"\r", i+1, iMap->map_num, maplist[i].name);
		if (maplist[i].m >= 0)
			map_foreachinmap(iMap->cleanup_sub, i, BL_ALL);
	}
	ShowStatus("Cleaned up %d maps."CL_CLL"\n", iMap->map_num);

	id_db->foreach(id_db,cleanup_db_sub);
	chrif->char_reset_offline();
	chrif->flush_fifo();

	atcommand->final();
	battle->final();
	chrif->do_final_chrif();
	ircbot->final();/* before clif. */
	clif->final();
	npc->final();
	script->final();
	itemdb->final();
	instance->final();
	storage->final();
	guild->final();
	party->do_final_party();
	pc->do_final_pc();
	pet->final();
	mob->final();
	homun->final();
	atcommand->final_msg();
	skill->final();
	iStatus->do_final_status();
	unit->final();
	bg->final();
	duel->final();
	elemental->do_final_elemental();
	do_final_maps();
	vending->final();

	map_db->destroy(map_db, map_db_final);

	mapindex_final();
	if(enable_grf)
		grfio_final();

	id_db->destroy(id_db, NULL);
	pc_db->destroy(pc_db, NULL);
	mobid_db->destroy(mobid_db, NULL);
	bossid_db->destroy(bossid_db, NULL);
	nick_db->destroy(nick_db, nick_db_final);
	charid_db->destroy(charid_db, NULL);
	iwall_db->destroy(iwall_db, NULL);
	regen_db->destroy(regen_db, NULL);

	map_sql_close();
	ers_destroy(map_iterator_ers);

	aFree(maplist);

	if( !enable_grf )
		aFree(map_cache_buffer);

	ShowStatus("Finished.\n");
}

static int map_abort_sub(struct map_session_data* sd, va_list ap)
{
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
		if (pc_db->size(pc_db))
			ShowFatalError("Server has crashed without a connection to the char-server, %u characters can't be saved!\n", pc_db->size(pc_db));
		return;
	}
	ShowError("Server received crash signal! Attempting to save all online characters!\n");
	iMap->map_foreachpc(map_abort_sub);
	chrif->flush_fifo();
}

/*======================================================
* Map-Server Version Screen [MC Cameri]
*------------------------------------------------------*/
static void map_helpscreen(bool do_exit)
{
	ShowInfo("Usage: %s [options]\n", SERVER_NAME);
	ShowInfo("\n");
	ShowInfo("Options:\n");
	ShowInfo("  -?, -h [--help]\t\tDisplays this help screen.\n");
	ShowInfo("  -v [--version]\t\tDisplays the server's version.\n");
	ShowInfo("  --run-once\t\t\tCloses server after loading (testing).\n");
	ShowInfo("  --map-config <file>\t\tAlternative map-server configuration.\n");
	ShowInfo("  --battle-config <file>\tAlternative battle configuration.\n");
	ShowInfo("  --atcommand-config <file>\tAlternative atcommand configuration.\n");
	ShowInfo("  --script-config <file>\tAlternative script configuration.\n");
	ShowInfo("  --msg-config <file>\t\tAlternative message configuration.\n");
	ShowInfo("  --grf-path <file>\t\tAlternative GRF path configuration.\n");
	ShowInfo("  --inter-config <file>\t\tAlternative inter-server configuration.\n");
	ShowInfo("  --log-config <file>\t\tAlternative logging configuration.\n");
	if( do_exit )
		exit(EXIT_SUCCESS);
}

/*======================================================
* Map-Server Version Screen [MC Cameri]
*------------------------------------------------------*/
static void map_versionscreen(bool do_exit) {
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

static bool map_arg_next_value(const char* option, int i, int argc)
{
	if( i >= argc-1 ) {
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

	if ( (m = iMap->mapname2mapid(map_name) <= 0 ) ) {
		ShowError("gm:info '"CL_WHITE"%s"CL_RESET"' is not a known map\n",map_name);
		return;
	}

	if( x < 0 || x >= maplist[m].xs || y < 0 || y >= maplist[m].ys ) {
		ShowError("gm:info '"CL_WHITE"%d %d"CL_RESET"' is out of '"CL_WHITE"%s"CL_RESET"' map bounds!\n",x,y,map_name);
		return;
	}

	ShowInfo("HCP: updated console's game position to '"CL_WHITE"%d %d %s"CL_RESET"'\n",x,y,map_name);
	cpsd->bl.x = x;
	cpsd->bl.y = y;
	cpsd->bl.m = m;
}
CPCMD(gm_use) {

	if( line == NULL ) {
		ShowError("gm:use invalid syntax. use '"CL_WHITE"gm:use @command <optional params>"CL_RESET"'\n");
		return;
	}
	cpsd->fd = -2;
	if( !atcommand->parse(cpsd->fd, cpsd, line, 0) )
		ShowInfo("HCP: '"CL_WHITE"%s"CL_RESET"' failed\n",line);
	else
		ShowInfo("HCP: '"CL_WHITE"%s"CL_RESET"' was used\n",line);
	cpsd->fd = 0;

}
/* Hercules Console Parser */
void map_cp_defaults(void) {
#ifdef CONSOLE_INPUT
	/* default HCP data */
	cpsd = pc->get_dummy_sd();
	strcpy(cpsd->status.name, "Hercules Console");
	cpsd->bl.x = MAP_DEFAULT_X;
	cpsd->bl.y = MAP_DEFAULT_Y;
	cpsd->bl.m = iMap->mapname2mapid(MAP_DEFAULT);

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
	HPM->share(iMap,"iMap");
	HPM->share(ircbot,"ircbot");
	HPM->share(itemdb,"itemdb");
	HPM->share(logs,"logs");
	HPM->share(mail,"mail");
	HPM->share(script,"script");
	HPM->share(searchstore,"searchstore");
	HPM->share(skill,"skill");
	HPM->share(vending,"vending");
	HPM->share(pc,"pc");
	HPM->share(party,"party");
	HPM->share(storage,"storage");
	HPM->share(trade,"trade");
	HPM->share(iStatus,"iStatus");
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
	HPM->share(mmysql_handle,"sql_handle");
	/* specific */
	HPM->share(atcommand->create,"addCommand");
	HPM->share(script->addScript,"addScript");
	HPM->share(HPM_map_addToMSD,"addToMSD");
	HPM->share(HPM_map_getFromMSD,"getFromMSD");
	HPM->share(HPM_map_removeFromMSD,"removeFromMSD");
	/* vars */
	HPM->share(maplist,"maplist");
}

void map_load_defaults(void) {
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
	int i;

#ifdef GCOLLECT
	GC_enable_incremental();
#endif

	map_defaults();

	iMap->map_num = 0;

	sprintf(iMap->db_path ,"db");
	sprintf(iMap->help_txt ,"conf/help.txt");
	sprintf(iMap->help2_txt ,"conf/help2.txt");
	sprintf(iMap->charhelp_txt ,"conf/charhelp.txt");

	sprintf(iMap->wisp_server_name ,"Server"); // can be modified in char-server configuration file

	iMap->autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
	iMap->minsave_interval = 100;
	iMap->save_settings = 0xFFFF;
	iMap->agit_flag = 0;
	iMap->agit2_flag = 0;
	iMap->night_flag = 0; // 0=day, 1=night [Yor]
	iMap->enable_spy = 0; //To enable/disable @spy commands, which consume too much cpu time when sending packets. [Skotlex]

	iMap->db_use_sql_item_db = 0;
	iMap->db_use_sql_mob_db = 0;
	iMap->db_use_sql_mob_skill_db = 0;

	sprintf(iMap->item_db_db, "item_db");
	sprintf(iMap->item_db2_db, "item_db2");
	sprintf(iMap->item_db_re_db, "item_db_re");
	sprintf(iMap->mob_db_db, "mob_db");
	sprintf(iMap->mob_db2_db, "mob_db2");
	sprintf(iMap->mob_skill_db_db, "mob_skill_db");
	sprintf(iMap->mob_skill_db2_db, "mob_skill_db2");
	sprintf(iMap->interreg_db, "interreg");

	iMap->INTER_CONF_NAME="conf/inter-server.conf";
	iMap->LOG_CONF_NAME="conf/logs.conf";
	iMap->MAP_CONF_NAME = "conf/map-server.conf";
	iMap->BATTLE_CONF_FILENAME = "conf/battle.conf";
	iMap->ATCOMMAND_CONF_FILENAME = "conf/atcommand.conf";
	iMap->SCRIPT_CONF_NAME = "conf/script.conf";
	iMap->MSG_CONF_NAME = "conf/messages.conf";
	iMap->GRF_PATH_FILENAME = "conf/grf-files.txt";
	rnd_init();

	for( i = 1; i < argc ; i++ ) {
		const char* arg = argv[i];

		if( arg[0] != '-' && ( arg[0] != '/' || arg[1] == '-' ) ) {// -, -- and /
			ShowError("Unknown option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		} else if( (++arg)[0] == '-' ) {// long option
			arg++;

			if( strcmp(arg, "help") == 0 ) {
				map_helpscreen(true);
			} else if( strcmp(arg, "version") == 0 ) {
				map_versionscreen(true);
			} else if( strcmp(arg, "map-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->MAP_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "battle-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->BATTLE_CONF_FILENAME = argv[++i];
			} else if( strcmp(arg, "atcommand-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->ATCOMMAND_CONF_FILENAME = argv[++i];
			} else if( strcmp(arg, "script-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->SCRIPT_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "msg-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->MSG_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "grf-path-file") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->GRF_PATH_FILENAME = argv[++i];
			} else if( strcmp(arg, "inter-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->INTER_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "log-config") == 0 ) {
				if( map_arg_next_value(arg, i, argc) )
					iMap->LOG_CONF_NAME = argv[++i];
			} else if( strcmp(arg, "run-once") == 0 ) { // close the map-server as soon as its done.. for testing [Celest]
				runflag = CORE_ST_STOP;
			} else {
				ShowError("Unknown option '%s'.\n", argv[i]);
				exit(EXIT_FAILURE);
			}
		} else switch( arg[0] ) {// short option
		case '?':
		case 'h':
			map_helpscreen(true);
			break;
		case 'v':
			map_versionscreen(true);
			break;
		default:
			ShowError("Unknown option '%s'.\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}
	memset(&index2mapid, -1, sizeof(index2mapid));

	map_load_defaults();
	map_config_read(iMap->MAP_CONF_NAME);
	CREATE(maplist,struct map_data,iMap->map_num);
	iMap->map_num = 0;
	map_config_read_sub(iMap->MAP_CONF_NAME);
	// loads npcs
	iMap->reloadnpc(false);

	chrif->checkdefaultlogin();

	if (!map_ip_set || !char_ip_set) {
		char ip_str[16];
		ip2str(addr_[0], ip_str);

		ShowWarning("Not all IP addresses in /conf/map-server.conf configured, autodetecting...\n");

		if (naddr_ == 0)
			ShowError("Unable to determine your IP address...\n");
		else if (naddr_ > 1)
			ShowNotice("Multiple interfaces detected...\n");

		ShowInfo("Defaulting to %s as our IP address\n", ip_str);

		if (!map_ip_set)
			clif->setip(ip_str);
		if (!char_ip_set)
			chrif->setip(ip_str);
	}

	battle->config_read(iMap->BATTLE_CONF_FILENAME);
	atcommand->msg_read(iMap->MSG_CONF_NAME);
	script->config_read(iMap->SCRIPT_CONF_NAME);
	inter_config_read(iMap->INTER_CONF_NAME);
	logs->config_read(iMap->LOG_CONF_NAME);

	id_db = idb_alloc(DB_OPT_BASE);
	pc_db = idb_alloc(DB_OPT_BASE);	//Added for reliable iMap->id2sd() use. [Skotlex]
	mobid_db = idb_alloc(DB_OPT_BASE);	//Added to lower the load of the lazy mob ai. [Skotlex]
	bossid_db = idb_alloc(DB_OPT_BASE); // Used for Convex Mirror quick MVP search
	map_db = uidb_alloc(DB_OPT_BASE);
	nick_db = idb_alloc(DB_OPT_BASE);
	charid_db = idb_alloc(DB_OPT_BASE);
	regen_db = idb_alloc(DB_OPT_BASE); // efficient status_natural_heal processing

	iwall_db = strdb_alloc(DB_OPT_RELEASE_DATA,2*NAME_LENGTH+2+1); // [Zephyrus] Invisible Walls
	zone_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, MAP_ZONE_NAME_LENGTH);

	map_iterator_ers = ers_new(sizeof(struct s_mapiterator),"map.c::map_iterator_ers",ERS_OPT_NONE);

	map_sql_init();
	if (logs->config.sql_logs)
		log_sql_init();

	mapindex_init();
	if(enable_grf)
		grfio_init(iMap->GRF_PATH_FILENAME);

	map_readallmaps();

	timer->add_func_list(map_freeblock_timer, "map_freeblock_timer");
	timer->add_func_list(map_clearflooritem_timer, "map_clearflooritem_timer");
	timer->add_func_list(map_removemobs_timer, "map_removemobs_timer");
	timer->add_interval(timer->gettick()+1000, map_freeblock_timer, 0, 0, 60*1000);

	HPM->load_sub = HPM_map_plugin_load_sub;
	HPM->symbol_defaults_sub = map_hp_symbols;
	HPM->config_read();
	HPM->event(HPET_INIT);

	atcommand->init();
	battle->init();
	instance->init();
	chrif->do_init_chrif();
	clif->init();
	ircbot->init();
	script->init();
	itemdb->init();
	skill->init();
	read_map_zone_db();/* read after item and skill initalization */
	mob->init();
	pc->do_init_pc();
	iStatus->do_init_status();
	party->do_init_party();
	guild->init();
	storage->init();
	pet->init();
	homun->init();
	mercenary->init();
	elemental->do_init_elemental();
	quest->init();
	npc->init();
	unit->init();
	bg->init();
	duel->init();
	vending->init();

	npc->event_do_oninit();	// Init npcs (OnInit)

	if (battle_config.pk_mode)
		ShowNotice("Server is running on '"CL_WHITE"PK Mode"CL_RESET"'.\n");

	Sql_HerculesUpdateCheck(mmysql_handle);
	
#ifdef CONSOLE_INPUT
	console->setSQL(mmysql_handle);
#endif
	
	ShowStatus("Server is '"CL_GREEN"ready"CL_RESET"' and listening on port '"CL_WHITE"%d"CL_RESET"'.\n\n", map_port);

	if( runflag != CORE_ST_STOP ) {
		shutdown_callback = iMap->do_shutdown;
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
	iMap = &iMap_s;

	/* funcs */
	iMap->zone_init = map_zone_init;
	iMap->zone_remove = map_zone_remove;
	iMap->zone_apply = map_zone_apply;
	iMap->zone_change = map_zone_change;
	iMap->zone_change2 = map_zone_change2;

	iMap->getcell = map_getcell;
	iMap->setgatcell = map_setgatcell;

	iMap->cellfromcache = map_cellfromcache;
	// users
	iMap->setusers = map_setusers;
	iMap->getusers = map_getusers;
	iMap->usercount = map_usercount;
	// blocklist lock
	iMap->freeblock = map_freeblock;
	iMap->freeblock_lock = map_freeblock_lock;
	iMap->freeblock_unlock = map_freeblock_unlock;
	// blocklist manipulation
	iMap->addblock = map_addblock;
	iMap->delblock = map_delblock;
	iMap->moveblock = map_moveblock;
	//blocklist nb in one cell
	iMap->count_oncell = map_count_oncell;
	iMap->find_skill_unit_oncell = map_find_skill_unit_oncell;
	// search and creation
	iMap->get_new_object_id = map_get_new_object_id;
	iMap->search_freecell = map_search_freecell;
	//
	iMap->quit = map_quit;
	// npc
	iMap->addnpc = map_addnpc;
	// map item
	iMap->clearflooritem_timer = map_clearflooritem_timer;
	iMap->removemobs_timer = map_removemobs_timer;
	iMap->clearflooritem = map_clearflooritem;
	iMap->addflooritem = map_addflooritem;
	// player to map session
	iMap->addnickdb = map_addnickdb;
	iMap->delnickdb = map_delnickdb;
	iMap->reqnickdb = map_reqnickdb;
	iMap->charid2nick = map_charid2nick;
	iMap->charid2sd = map_charid2sd;

	iMap->map_foreachpc = map_map_foreachpc;
	iMap->map_foreachmob = map_map_foreachmob;
	iMap->map_foreachnpc = map_map_foreachnpc;
	iMap->map_foreachregen = map_map_foreachregen;
	iMap->map_foreachiddb = map_map_foreachiddb;

	iMap->foreachinrange = map_foreachinrange;
	iMap->foreachinshootrange = map_foreachinshootrange;
	iMap->foreachinarea = map_foreachinarea;
	iMap->forcountinrange = map_forcountinrange;
	iMap->forcountinarea = map_forcountinarea;
	iMap->foreachinmovearea = map_foreachinmovearea;
	iMap->foreachincell = map_foreachincell;
	iMap->foreachinpath = map_foreachinpath;
	iMap->foreachinmap = map_foreachinmap;
	iMap->foreachininstance = map_foreachininstance;

	iMap->id2sd = map_id2sd;
	iMap->id2md = map_id2md;
	iMap->id2nd = map_id2nd;
	iMap->id2hd = map_id2hd;
	iMap->id2mc = map_id2mc;
	iMap->id2cd = map_id2cd;
	iMap->id2bl = map_id2bl;
	iMap->blid_exists = map_blid_exists;
	iMap->mapindex2mapid = map_mapindex2mapid;
	iMap->mapname2mapid = map_mapname2mapid;
	iMap->mapname2ipport = map_mapname2ipport;
	iMap->setipport = map_setipport;
	iMap->eraseipport = map_eraseipport;
	iMap->eraseallipport = map_eraseallipport;
	iMap->addiddb = map_addiddb;
	iMap->deliddb = map_deliddb;
	/* */
	iMap->nick2sd = map_nick2sd;
	iMap->getmob_boss = map_getmob_boss;
	iMap->id2boss = map_id2boss;
	// reload config file looking only for npcs
	iMap->reloadnpc = map_reloadnpc;

	iMap->check_dir = map_check_dir;
	iMap->calc_dir = map_calc_dir;
	iMap->random_dir = map_random_dir; // [Skotlex]

	iMap->cleanup_sub = cleanup_sub;

	iMap->delmap = map_delmap;
	iMap->flags_init = map_flags_init;

	iMap->iwall_set = map_iwall_set;
	iMap->iwall_get = map_iwall_get;
	iMap->iwall_remove = map_iwall_remove;

	iMap->addmobtolist = map_addmobtolist; // [Wizputer]
	iMap->spawnmobs = map_spawnmobs; // [Wizputer]
	iMap->removemobs = map_removemobs; // [Wizputer]
	iMap->addmap2db = map_addmap2db;
	iMap->removemapdb = map_removemapdb;
	iMap->clean = map_clean;

	iMap->do_shutdown = do_shutdown;

	/* FIXME: temporary until the map.c "Hercules Renewal Phase One" design is complete. [Ind] */
	mapit = &mapit_s;

	mapit->alloc = mapit_alloc;
	mapit->free = mapit_free;
	mapit->first = mapit_first;
	mapit->last = mapit_last;
	mapit->next = mapit_next;
	mapit->prev = mapit_prev;
	mapit->exists = mapit_exists;
}
