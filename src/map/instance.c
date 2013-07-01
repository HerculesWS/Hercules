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
#include "../common/db.h"

#include "clif.h"
#include "instance.h"
#include "map.h"
#include "npc.h"
#include "party.h"
#include "pc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

struct instance_interface instance_s;

/// Checks whether given instance id is valid or not.
bool instance_is_valid(int instance_id) {
	if( instance_id < 0 || instance_id >= instance->instances ) {// out of range
		return false;
	}

	if( instances[instance_id].state == INSTANCE_FREE ) {// uninitialized/freed instance slot
		return false;
	}

	return true;
}


/*--------------------------------------
 * name : instance name
 * Return value could be
 * -4 = already exists | -3 = no free instances | -2 = owner not found | -1 = invalid type
 * On success return instance_id
 *--------------------------------------*/
int instance_create(int owner_id, const char *name, enum instance_owner_type type) {
	struct map_session_data *sd = NULL;
	unsigned short *icptr = NULL;
	struct party_data *p = NULL;
	struct guild *g = NULL;
	short *iptr = NULL;
	int i, j;
	
	switch ( type ) {
		case IOT_NONE:
			break;
		case IOT_CHAR:
			if( ( sd = iMap->id2sd(owner_id) ) == NULL ) {
				ShowError("instance_create: character %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			iptr = sd->instance;
			icptr = &sd->instances;
			break;
		case IOT_PARTY:
			if( ( p = party->search(owner_id) ) == NULL ) {
				ShowError("instance_create: party %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			iptr = p->instance;
			icptr = &p->instances;
			break;
		case IOT_GUILD:
			if( ( g = guild->search(owner_id) ) == NULL ) {
				ShowError("instance_create: guild %d not found for instance '%s'.\n", owner_id, name);
				return -2;
			}
			iptr = g->instance;
			icptr = &g->instances;
			break;
		default:
			ShowError("instance_create: unknown type %d for owner_id %d and name %s.\n", type,owner_id,name);
			return -1;
	}
	
	if( type != IOT_NONE && *icptr ) {
		ARR_FIND(0, *icptr, i, strcmp(instances[iptr[i]].name,name) == 0 );
		if( i != *icptr )
			return -4;/* already got this instance */
	}
		
	ARR_FIND(0, instance->instances, i, instances[i].state == INSTANCE_FREE);
		
	if( i == instance->instances )
		RECREATE(instances, struct instance_data, ++instance->instances);

	instances[i].state = INSTANCE_IDLE;
	instances[i].id = i;
	instances[i].idle_timer = INVALID_TIMER;
	instances[i].idle_timeout = instances[i].idle_timeoutval = 0;
	instances[i].progress_timer = INVALID_TIMER;
	instances[i].progress_timeout = 0;
	instances[i].users = 0;
	instances[i].map = NULL;
	instances[i].num_map = 0;
	instances[i].owner_id = owner_id;
	instances[i].owner_type = type;
	instances[i].vars = idb_alloc(DB_OPT_RELEASE_DATA);

	safestrncpy( instances[i].name, name, sizeof(instances[i].name) );
	
	if( type != IOT_NONE ) {
		ARR_FIND(0, *icptr, j, iptr[j] == -1);
		if( j == *icptr ) {
			switch( type ) {
				case IOT_CHAR:
					RECREATE(sd->instance, unsigned short, ++*icptr);
					sd->instance[sd->instances-1] = i;
					break;
				case IOT_PARTY:
					RECREATE(p->instance, unsigned short, ++*icptr);
					p->instance[p->instances-1] = i;
					break;
				case IOT_GUILD:
					RECREATE(g->instance, unsigned short, ++*icptr);
					g->instance[g->instances-1] = i;
					break;
			}
		} else
			iptr[j] = i;
	}
	
	clif->instance(i, 1, 0); // Start instancing window
	return i;
}

/*--------------------------------------
 * Add a map to the instance using src map "name"
 *--------------------------------------*/
int instance_add_map(const char *name, int instance_id, bool usebasename, const char *map_name) {
	int16 m = iMap->mapname2mapid(name);
	int i, im = -1;
	size_t num_cell, size;

	if( m < 0 )
		return -1; // source map not found

	if( !instance->valid(instance_id) ) {
		ShowError("instance_add_map: trying to attach '%s' map to non-existing instance %d.\n", name, instance_id);
		return -1;
	}
	
	if( map_name != NULL && strdb_iget(mapindex_db, map_name) ) {
		ShowError("instance_add_map: trying to create instanced map with existent name '%s'\n", map_name);
		return -2;
	}
	
	if( map[m].instance_id >= 0 ) { // Source map already belong to a Instance.
		ShowError("instance_add_map: trying to instance already instanced map %s.\n", name);
		return -4;
	}
	
	ARR_FIND( instance->start_id, iMap->map_num, i, map[i].name[0] == 0 ); // Searching for a Free Map
		
	if( i < iMap->map_num )
		im = i; // Unused map found (old instance)
	else {
		im = iMap->map_num; // Using next map index
		RECREATE(map,struct map_data,++iMap->map_num);
	}
	
	if( map[m].cell == (struct mapcell *)0xdeadbeaf )
		iMap->cellfromcache(&map[m]);

	memcpy( &map[im], &map[m], sizeof(struct map_data) ); // Copy source map
	if( map_name != NULL ) {
		snprintf(map[im].name, MAP_NAME_LENGTH, "%s", map_name);
		map[im].cName = map[m].name;
	} else
		snprintf(map[im].name, MAP_NAME_LENGTH, (usebasename ? "%.3d#%s" : "%.3d%s"), instance_id, name); // Generate Name for Instance Map
	map[im].index = mapindex_addmap(-1, map[im].name); // Add map index

	map[im].channel = NULL;
	
	if( !map[im].index ) {
		map[im].name[0] = '\0';
		ShowError("instance_add_map: no more free map indexes.\n");
		return -3; // No free map index
	}
	
	// Reallocate cells
	num_cell = map[im].xs * map[im].ys;
	CREATE( map[im].cell, struct mapcell, num_cell );
	memcpy( map[im].cell, map[m].cell, num_cell * sizeof(struct mapcell) );

	size = map[im].bxs * map[im].bys * sizeof(struct block_list*);
	map[im].block = (struct block_list**)aCalloc(size, 1);
	map[im].block_mob = (struct block_list**)aCalloc(size, 1);

	memset(map[im].npc, 0x00, sizeof(map[i].npc));
	map[im].npc_num = 0;

	memset(map[im].moblist, 0x00, sizeof(map[im].moblist));
	map[im].mob_delete_timer = INVALID_TIMER;

	map[im].m = im;
	map[im].instance_id = instance_id;
	map[im].instance_src_map = m;
	map[m].flag.src4instance = 1; // Flag this map as a src map for instances

	RECREATE(instances[instance_id].map, unsigned short, ++instances[instance_id].num_map);

	instances[instance_id].map[instances[instance_id].num_map - 1] = im; // Attach to actual instance
	iMap->addmap2db(&map[im]);
	
	return im;
}

/*--------------------------------------
 * m : source map of this instance
 * party_id : source party of this instance
 * type : result (0 = map id | 1 = instance id)
 *--------------------------------------*/
int instance_map2imap(int16 m, int instance_id) {
 	int i;

	if( !instance->valid(instance_id) ) {
		return -1;
	}

	for( i = 0; i < instances[instance_id].num_map; i++ ) {
		if( instances[instance_id].map[i] && map[instances[instance_id].map[i]].instance_src_map == m )
			return instances[instance_id].map[i];
 	}
 	return -1;
}

/*--------------------------------------
 * m : source map
 * instance_id : where to search
 * result : mapid of map "m" in this instance
 *--------------------------------------*/
int instance_mapid2imapid(int16 m, int instance_id) {
	if( map[m].flag.src4instance == 0 )
		return m; // not instances found for this map
	else if( map[m].instance_id >= 0 ) { // This map is a instance, not a src map instance
		ShowError("map_instance_mapid2imapid: already instanced (%d / %d)\n", m, instance_id);
		return -1;
	}

	if( !instance->valid(instance_id) )
		return -1;

	return instance->map2imap(m, instance_id);
}

/*--------------------------------------
 * map_instance_map_npcsub
 * Used on Init instance. Duplicates each script on source map
 *--------------------------------------*/
int instance_map_npcsub(struct block_list* bl, va_list args) {
	struct npc_data* nd = (struct npc_data*)bl;
	int16 m = va_arg(args, int); // Destination Map

	if ( npc_duplicate4instance(nd, m) )
		ShowDebug("instance_map_npcsub:npc_duplicate4instance failed (%s/%d)\n",nd->name,m);

	return 1;
}

/*--------------------------------------
 * Init all map on the instance. Npcs are created here
 *--------------------------------------*/
void instance_init(int instance_id) {
	int i;

	if( !instance->valid(instance_id) )
		return; // nothing to do

	for( i = 0; i < instances[instance_id].num_map; i++ )
		iMap->foreachinmap(instance_map_npcsub, map[instances[instance_id].map[i]].instance_src_map, BL_NPC, instances[instance_id].map[i]);

	instances[instance_id].state = INSTANCE_BUSY;
}

/*--------------------------------------
 * Used on instance deleting process.
 * Warps all players on each instance map to its save points.
 *--------------------------------------*/
int instance_del_load(struct map_session_data* sd, va_list args) {
	int16 m = va_arg(args,int);
	
	if( !sd || sd->bl.m != m )
		return 0;

	pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_OUTSIGHT);
	return 1;
}

/* for npcs behave differently when being unloaded within a instance */
int instance_cleanup_sub(struct block_list *bl, va_list ap) {
	nullpo_ret(bl);

	switch(bl->type) {
		case BL_PC:
			iMap->quit((struct map_session_data *) bl);
			break;
		case BL_NPC:
			npc_unload((struct npc_data *)bl,true);
			break;
		case BL_MOB:
			unit_free(bl,CLR_OUTSIGHT);
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

/*--------------------------------------
 * Removes a simple instance map
 *--------------------------------------*/
void instance_del_map(int16 m) {
	int i;
	
	if( m <= 0 || map[m].instance_id == -1 ) {
		ShowError("instance_del_map: tried to remove non-existing instance map (%d)\n", m);
		return;
	}

	iMap->map_foreachpc(instance_del_load, m);
	iMap->foreachinmap(instance_cleanup_sub, m, BL_ALL);

	if( map[m].mob_delete_timer != INVALID_TIMER )
		iTimer->delete_timer(map[m].mob_delete_timer, iMap->removemobs_timer);
	
	mapindex_removemap( map[m].index );

	// Free memory
	aFree(map[m].cell);
	aFree(map[m].block);
	aFree(map[m].block_mob);
	
	// Remove from instance
	for( i = 0; i < instances[map[m].instance_id].num_map; i++ ) {
		if( instances[map[m].instance_id].map[i] == m ) {
			instances[map[m].instance_id].num_map--;
			for( ; i < instances[map[m].instance_id].num_map; i++ )
				instances[map[m].instance_id].map[i] = instances[map[m].instance_id].map[i+1];
			i = -1;
			break;
		}
	}
	
	if( i == instances[map[m].instance_id].num_map )
		ShowError("map_instance_del: failed to remove %s from instance list (%s): %d\n", map[m].name, instances[map[m].instance_id].name, m);
	
	if( map[m].channel )
		clif->chsys_delete(map[m].channel);

	iMap->removemapdb(&map[m]);
	memset(&map[m], 0x00, sizeof(map[0]));
	map[m].name[0] = 0;
	map[m].instance_id = -1;
	map[m].mob_delete_timer = INVALID_TIMER;
}

/*--------------------------------------
 * Timer to destroy instance by process or idle
 *--------------------------------------*/
int instance_destroy_timer(int tid, unsigned int tick, int id, intptr_t data) {
	instance->destroy(id);
	return 0;
}

/*--------------------------------------
 * Removes a instance, all its maps and npcs.
 *--------------------------------------*/
void instance_destroy(int instance_id) {
	struct map_session_data *sd = NULL;
	unsigned short *icptr = NULL;
	struct party_data *p = NULL;
	struct guild *g = NULL;
	short *iptr = NULL;
	int type, j, last = 0;
	unsigned int now = (unsigned int)time(NULL);
	
	if( !instance->valid(instance_id) )
		return; // nothing to do

	if( instances[instance_id].progress_timeout && instances[instance_id].progress_timeout <= now )
		type = 1;
	else if( instances[instance_id].idle_timeout && instances[instance_id].idle_timeout <= now )
		type = 2;
	else
		type = 3;
		
	clif->instance(instance_id, 5, type); // Report users this instance has been destroyed

	switch ( instances[instance_id].owner_type ) {
		case IOT_NONE:
			break;
		case IOT_CHAR:
			if( ( sd = iMap->id2sd(instances[instance_id].owner_id) ) == NULL ) {
				break;
			}
			iptr = sd->instance;
			icptr = &sd->instances;
			break;
		case IOT_PARTY:
			if( ( p = party->search(instances[instance_id].owner_id) ) == NULL ) {
				break;
			}
			iptr = p->instance;
			icptr = &p->instances;
			break;
		case IOT_GUILD:
			if( ( g = guild->search(instances[instance_id].owner_id) ) == NULL ) {
				break;
			}
			iptr = g->instance;
			icptr = &g->instances;
			break;
		default:
			ShowError("instance_destroy: unknown type %d for owner_id %d and name '%s'.\n", instances[instance_id].owner_type,instances[instance_id].owner_id,instances[instance_id].name);
			break;
	}
	
	if( iptr != NULL ) {
		ARR_FIND(0, *icptr, j, iptr[j] == instance_id);
		if( j != *icptr )
			iptr[j] = -1;
	}
	
	while( instances[instance_id].num_map && last != instances[instance_id].map[0] ) { // Remove all maps from instance
		last = instances[instance_id].map[0];
		instance->del_map( instances[instance_id].map[0] );
	}
	
	if( instances[instance_id].vars )
		db_destroy(instances[instance_id].vars);

	if( instances[instance_id].progress_timer != INVALID_TIMER )
		iTimer->delete_timer( instances[instance_id].progress_timer, instance_destroy_timer);
	if( instances[instance_id].idle_timer != INVALID_TIMER )
		iTimer->delete_timer( instances[instance_id].idle_timer, instance_destroy_timer);

	instances[instance_id].vars = NULL;

	if( instances[instance_id].map )
		aFree(instances[instance_id].map);
	
	instances[instance_id].map = NULL;
	instances[instance_id].state = INSTANCE_FREE;
	instances[instance_id].num_map = 0;
}

/*--------------------------------------
 * Checks if there are users in the instance or not to start idle timer
 *--------------------------------------*/
void instance_check_idle(int instance_id) {
	bool idle = true;
	unsigned int now = (unsigned int)time(NULL);

	if( !instance->valid(instance_id) || instances[instance_id].idle_timeoutval == 0 )
		return;

	if( instances[instance_id].users )
		idle = false;

	if( instances[instance_id].idle_timer != INVALID_TIMER && !idle ) {
		iTimer->delete_timer(instances[instance_id].idle_timer, instance_destroy_timer);
		instances[instance_id].idle_timer = INVALID_TIMER;
		instances[instance_id].idle_timeout = 0;
		clif->instance(instance_id, 3, 0); // Notify instance users normal instance expiration
	} else if( instances[instance_id].idle_timer == INVALID_TIMER && idle ) {
		instances[instance_id].idle_timeout = now + instances[instance_id].idle_timeoutval;
		instances[instance_id].idle_timer = iTimer->add_timer( iTimer->gettick() + instances[instance_id].idle_timeoutval * 1000, instance_destroy_timer, instance_id, 0);
		clif->instance(instance_id, 4, 0); // Notify instance users it will be destroyed of no user join it again in "X" time
	}
}

/*--------------------------------------
 * Set instance Timers
 *--------------------------------------*/
void instance_set_timeout(int instance_id, unsigned int progress_timeout, unsigned int idle_timeout)
{
	unsigned int now = (unsigned int)time(0);

	if( !instance->valid(instance_id) )
		return;

	if( instances[instance_id].progress_timer != INVALID_TIMER )
		iTimer->delete_timer( instances[instance_id].progress_timer, instance_destroy_timer);
	if( instances[instance_id].idle_timer != INVALID_TIMER )
		iTimer->delete_timer( instances[instance_id].idle_timer, instance_destroy_timer);

	if( progress_timeout ) {
		instances[instance_id].progress_timeout = now + progress_timeout;
		instances[instance_id].progress_timer = iTimer->add_timer( iTimer->gettick() + progress_timeout * 1000, instance_destroy_timer, instance_id, 0);
	} else {
		instances[instance_id].progress_timeout = 0;
		instances[instance_id].progress_timer = INVALID_TIMER;
	}

	if( idle_timeout ) {
		instances[instance_id].idle_timeoutval = idle_timeout;
		instances[instance_id].idle_timer = INVALID_TIMER;
		instance_check_idle(instance_id);
	} else {
		instances[instance_id].idle_timeoutval = 0;
		instances[instance_id].idle_timeout = 0;
		instances[instance_id].idle_timer = INVALID_TIMER;
	}

	if( instances[instance_id].idle_timer == INVALID_TIMER && instances[instance_id].progress_timer != INVALID_TIMER )
		clif->instance(instance_id, 3, 0);
}

/*--------------------------------------
 * Checks if sd in on a instance and should be kicked from it
 *--------------------------------------*/
void instance_check_kick(struct map_session_data *sd) {
	int16 m = sd->bl.m;

	clif->instance_leave(sd->fd);
	if( map[m].instance_id >= 0 ) { // User was on the instance map
		if( map[m].save.map )
			pc->setpos(sd, map[m].save.map, map[m].save.x, map[m].save.y, CLR_TELEPORT);
		else
			pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
	}
}

void do_final_instance(void) {
	int i;
	
	for(i = 0; i < instance->instances; i++) {
		instance->destroy(i);
	}
	
	if( instances )
		aFree(instances);

	instance->instances = 0;
}

void do_init_instance(void) {
	iTimer->add_timer_func_list(instance_destroy_timer, "instance_destroy_timer");
}

void instance_defaults(void) {
	instance = &instance_s;
	
	instance->init = do_init_instance;
	instance->final = do_final_instance;
	
	/* start point */
	instance->start_id = 0;
	/* count */
	instance->instances = 0;
	
	/* */
	instance->create = instance_create;
	instance->add_map = instance_add_map;
	instance->del_map = instance_del_map;
	instance->map2imap = instance_map2imap;
	instance->mapid2imapid = instance_mapid2imapid;
	instance->destroy = instance_destroy;
	instance->start = instance_init;
	instance->check_idle = instance_check_idle;
	instance->check_kick = instance_check_kick;
	instance->set_timeout = instance_set_timeout;
	instance->valid = instance_is_valid;
}
