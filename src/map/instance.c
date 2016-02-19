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

#include "config/core.h" // CELL_NOSTACK
#include "instance.h"

#include "map/channel.h"
#include "map/clif.h"
#include "map/guild.h"
#include "map/map.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct instance_interface instance_s;
struct instance_interface *instance;

/// Checks whether given instance id is valid or not.
bool instance_is_valid(int instance_id) {
	if( instance_id < 0 || instance_id >= instance->instances ) {// out of range
		return false;
	}

	if( instance->list[instance_id].state == INSTANCE_FREE ) {// uninitialized/freed instance slot
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
	int i;

	nullpo_retr(-1, name);
	switch ( type ) {
		case IOT_NONE:
			break;
		case IOT_CHAR:
			if( ( sd = map->id2sd(owner_id) ) == NULL ) {
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
			ShowError("instance_create: unknown type %u for owner_id %d and name %s.\n", type, owner_id, name);
			return -1;
	}

	if( type != IOT_NONE && *icptr ) {
		ARR_FIND(0, *icptr, i, iptr[i] != -1 && strcmp(instance->list[iptr[i]].name, name) == 0 );
		if( i != *icptr )
			return -4; /* already got this instance */
	}

	ARR_FIND(0, instance->instances, i, instance->list[i].state == INSTANCE_FREE);

	if( i == instance->instances )
		RECREATE(instance->list, struct instance_data, ++instance->instances);

	instance->list[i].state = INSTANCE_IDLE;
	instance->list[i].id = i;
	instance->list[i].idle_timer = INVALID_TIMER;
	instance->list[i].idle_timeout = instance->list[i].idle_timeoutval = 0;
	instance->list[i].progress_timer = INVALID_TIMER;
	instance->list[i].progress_timeout = 0;
	instance->list[i].users = 0;
	instance->list[i].map = NULL;
	instance->list[i].num_map = 0;
	instance->list[i].owner_id = owner_id;
	instance->list[i].owner_type = type;
	instance->list[i].regs.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	instance->list[i].regs.arrays = NULL;
	instance->list[i].respawn.map = 0;
	instance->list[i].respawn.y = 0;
	instance->list[i].respawn.x = 0;

	safestrncpy( instance->list[i].name, name, sizeof(instance->list[i].name) );

	if( type != IOT_NONE ) {
		int j;
		ARR_FIND(0, *icptr, j, iptr[j] == -1);
		if (j == *icptr) {
			switch( type ) {
				case IOT_CHAR:
					RECREATE(sd->instance, short, ++*icptr);
					sd->instance[sd->instances-1] = i;
					break;
				case IOT_PARTY:
					RECREATE(p->instance, short, ++*icptr);
					p->instance[p->instances-1] = i;
					break;
				case IOT_GUILD:
					RECREATE(g->instance, short, ++*icptr);
					g->instance[g->instances-1] = i;
					break;
			}
		} else {
			iptr[j] = i;
		}
	}

	clif->instance(i, 1, 0); // Start instancing window
	return i;
}

/**
 * Add a map to the instance using src map "name"
 *
 * @param name Source map name.
 * @param instance_id The destination instance ID.
 * @param usebasename Whether to generate a standard instance map name (only used if map_name is not NULL).
 * @param map_name    The name for the instanced map (may be NULL to generate a new one).
 * @return The generated map's index.
 * @retval -1 Map or instance not found.
 * @retval -2 Duplicate map name.
 * @retval -3 No more map indices available.
 * @retval -4 Source map is already an instance.
 **/
int instance_add_map(const char *name, int instance_id, bool usebasename, const char *map_name) {
	int16 m = map->mapname2mapid(name);
	int i, im = -1;
	size_t num_cell, size, j;

	nullpo_retr(-1, name);

	if( m < 0 )
		return -1; // source map not found

	if( !instance->valid(instance_id) ) {
		ShowError("instance_add_map: trying to attach '%s' map to non-existing instance %d.\n", name, instance_id);
		return -1;
	}

	if( map_name != NULL && strdb_iget(mapindex->db, map_name) ) {
		ShowError("instance_add_map: trying to create instanced map with existent name '%s'\n", map_name);
		return -2;
	}

	if( map->list[m].instance_id >= 0 ) {
		// Source map already belong to a Instance.
		ShowError("instance_add_map: trying to instance already instanced map %s.\n", name);
		return -4;
	}

	ARR_FIND( instance->start_id, map->count, i, map->list[i].name[0] == 0 ); // Searching for a Free Map

	if( i < map->count )
		im = i; // Unused map found (old instance)
	else {
		im = map->count; // Using next map index
		RECREATE(map->list,struct map_data,++map->count);
	}

	if( map->list[m].cell == (struct mapcell *)0xdeadbeaf )
		map->cellfromcache(&map->list[m]);

	memcpy( &map->list[im], &map->list[m], sizeof(struct map_data) ); // Copy source map
	if( map_name != NULL ) {
		snprintf(map->list[im].name, MAP_NAME_LENGTH, "%s", map_name);
		map->list[im].custom_name = true;
	} else
		snprintf(map->list[im].name, MAP_NAME_LENGTH, (usebasename ? "%.3d#%s" : "%.3d%s"), instance_id, name); // Generate Name for Instance Map
	map->list[im].index = mapindex->addmap(-1, map->list[im].name); // Add map index

	map->list[im].channel = NULL;

	if( !map->list[im].index ) {
		map->list[im].name[0] = '\0';
		ShowError("instance_add_map: no more free map indexes.\n");
		return -3; // No free map index
	}

	// Reallocate cells
	num_cell = map->list[im].xs * map->list[im].ys;
	CREATE( map->list[im].cell, struct mapcell, num_cell );
	memcpy( map->list[im].cell, map->list[m].cell, num_cell * sizeof(struct mapcell) );

	// Appropriately clear cell data
	for(j = 0; j < num_cell; j++) {
#ifdef CELL_NOSTACK
		map->list[im].cell[j].cell_bl = 0;
#endif // CELL_NOSTACK
		map->list[im].cell[j].basilica = 0;
		map->list[im].cell[j].icewall = 0;
		map->list[im].cell[j].npc = 0;
		map->list[im].cell[j].landprotector = 0;
	}

	size = map->list[im].bxs * map->list[im].bys * sizeof(struct block_list*);
	map->list[im].block = (struct block_list**)aCalloc(size, 1);
	map->list[im].block_mob = (struct block_list**)aCalloc(size, 1);

	memset(map->list[im].npc, 0x00, sizeof(map->list[i].npc));
	map->list[im].npc_num = 0;

	memset(map->list[im].moblist, 0x00, sizeof(map->list[im].moblist));
	map->list[im].mob_delete_timer = INVALID_TIMER;

	//Mimic unit
	if( map->list[m].unit_count ) {
		map->list[im].unit_count = map->list[m].unit_count;
		CREATE( map->list[im].units, struct mapflag_skill_adjust*, map->list[im].unit_count );

		for(i = 0; i < map->list[im].unit_count; i++) {
			CREATE( map->list[im].units[i], struct mapflag_skill_adjust, 1);
			memcpy( map->list[im].units[i],map->list[m].units[i],sizeof(struct mapflag_skill_adjust));
		}
	}
	//Mimic skills
	if( map->list[m].skill_count ) {
		map->list[im].skill_count = map->list[m].skill_count;
		CREATE( map->list[im].skills, struct mapflag_skill_adjust*, map->list[im].skill_count );

		for(i = 0; i < map->list[im].skill_count; i++) {
			CREATE( map->list[im].skills[i], struct mapflag_skill_adjust, 1);
			memcpy( map->list[im].skills[i],map->list[m].skills[i],sizeof(struct mapflag_skill_adjust));
		}
	}
	//Mimic zone mf
	if( map->list[m].zone_mf_count ) {
		map->list[im].zone_mf_count = map->list[m].zone_mf_count;
		CREATE( map->list[im].zone_mf, char *, map->list[im].zone_mf_count );

		for(i = 0; i < map->list[im].zone_mf_count; i++) {
			CREATE(map->list[im].zone_mf[i], char, MAP_ZONE_MAPFLAG_LENGTH);
			safestrncpy(map->list[im].zone_mf[i],map->list[m].zone_mf[i],MAP_ZONE_MAPFLAG_LENGTH);
		}
	}

	//Mimic questinfo
	if( map->list[m].qi_count ) {
		map->list[im].qi_count = map->list[m].qi_count;
		CREATE( map->list[im].qi_data, struct questinfo, map->list[im].qi_count );
		memcpy( map->list[im].qi_data, map->list[m].qi_data, map->list[im].qi_count * sizeof(struct questinfo) );
	}

	map->list[im].m = im;
	map->list[im].instance_id = instance_id;
	map->list[im].instance_src_map = m;
	map->list[im].flag.src4instance = 0; //clear
	map->list[m].flag.src4instance = 1; // Flag this map as a src map for instances

	RECREATE(instance->list[instance_id].map, unsigned short, ++instance->list[instance_id].num_map);

	instance->list[instance_id].map[instance->list[instance_id].num_map - 1] = im; // Attach to actual instance
	map->addmap2db(&map->list[im]);

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

	for( i = 0; i < instance->list[instance_id].num_map; i++ ) {
		if( instance->list[instance_id].map[i] && map->list[instance->list[instance_id].map[i]].instance_src_map == m )
			return instance->list[instance_id].map[i];
	}
	return -1;
}

int instance_mapname2imap(const char *map_name, int instance_id) {
	int i;

	nullpo_retr(-1, map_name);
	if( !instance->valid(instance_id) ) {
		return -1;
	}

	for( i = 0; i < instance->list[instance_id].num_map; i++ ) {
		if( instance->list[instance_id].map[i] && !strcmpi(map->list[map->list[instance->list[instance_id].map[i]].instance_src_map].name,map_name) )
			return instance->list[instance_id].map[i];
	}
	return -1;
}

/*--------------------------------------
 * m : source map
 * instance_id : where to search
 * result : mapid of map "m" in this instance
 *--------------------------------------*/
int instance_mapid2imapid(int16 m, int instance_id) {
	Assert_retr(-1, m >= 0 && m < map->count);
	if( map->list[m].flag.src4instance == 0 )
		return m; // not instances found for this map
	else if( map->list[m].instance_id >= 0 ) { // This map is a instance, not a src map instance
		ShowError("map_instance_mapid2imapid: already instanced (%d / %d)\n", m, instance_id);
		return -1;
	}

	if( !instance->valid(instance_id) )
		return -1;

	return instance->map2imap(m, instance_id);
}

/*--------------------------------------
 * Used on Init instance. Duplicates each script on source map
 *--------------------------------------*/
int instance_map_npcsub(struct block_list* bl, va_list args)
{
	struct npc_data *nd = NULL;
	int16 m = va_arg(args, int); // Destination Map

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_NPC);
	nd = BL_UCAST(BL_NPC, bl);

	if (npc->duplicate4instance(nd, m))
		ShowDebug("instance_map_npcsub:npc_duplicate4instance failed (%s/%d)\n",nd->name,m);

	return 1;
}

int instance_init_npc(struct block_list* bl, va_list args)
{
	struct npc_data *nd = NULL;
	struct event_data *ev;
	char evname[EVENT_NAME_LENGTH];

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_NPC);
	nd = BL_UCAST(BL_NPC, bl);

	snprintf(evname, EVENT_NAME_LENGTH, "%s::OnInstanceInit", nd->exname);

	if( ( ev = strdb_get(npc->ev_db, evname) ) )
		script->run_npc(ev->nd->u.scr.script, ev->pos, 0, ev->nd->bl.id);

	return 1;
}

/*--------------------------------------
 * Init all map on the instance. Npcs are created here
 *--------------------------------------*/
void instance_init(int instance_id) {
	int i;

	if( !instance->valid(instance_id) )
		return; // nothing to do

	for( i = 0; i < instance->list[instance_id].num_map; i++ )
		map->foreachinmap(instance->map_npcsub, map->list[instance->list[instance_id].map[i]].instance_src_map, BL_NPC, instance->list[instance_id].map[i]);

	/* cant be together with the previous because it will rely on all of them being up */
	map->foreachininstance(instance->init_npc, instance_id, BL_NPC);

	instance->list[instance_id].state = INSTANCE_BUSY;
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
			map->quit(BL_UCAST(BL_PC, bl));
			break;
		case BL_NPC:
			npc->unload(BL_UCAST(BL_NPC, bl), true);
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
			skill->delunit(BL_UCAST(BL_SKILL, bl));
			break;
	}

	return 1;
}

/*--------------------------------------
 * Removes a simple instance map
 *--------------------------------------*/
void instance_del_map(int16 m) {
	int i;

	if( m <= 0 || map->list[m].instance_id == -1 ) {
		ShowError("instance_del_map: tried to remove non-existing instance map (%d)\n", m);
		return;
	}

	map->foreachpc(instance_del_load, m);
	map->foreachinmap(instance_cleanup_sub, m, BL_ALL);

	if( map->list[m].mob_delete_timer != INVALID_TIMER )
		timer->delete(map->list[m].mob_delete_timer, map->removemobs_timer);

	mapindex->removemap(map_id2index(m));

	// Free memory
	aFree(map->list[m].cell);
	aFree(map->list[m].block);
	aFree(map->list[m].block_mob);

	if (map->list[m].unit_count && map->list[m].units) {
		for(i = 0; i < map->list[m].unit_count; i++) {
			aFree(map->list[m].units[i]);
		}
		aFree(map->list[m].units);
	}

	if (map->list[m].skill_count && map->list[m].skills) {
		for(i = 0; i < map->list[m].skill_count; i++) {
			aFree(map->list[m].skills[i]);
		}
		aFree(map->list[m].skills);
	}

	if (map->list[m].zone_mf_count && map->list[m].zone_mf) {
		for(i = 0; i < map->list[m].zone_mf_count; i++) {
			aFree(map->list[m].zone_mf[i]);
		}
		aFree(map->list[m].zone_mf);
	}

	if( map->list[m].qi_data )
		aFree(map->list[m].qi_data);

	// Remove from instance
	for( i = 0; i < instance->list[map->list[m].instance_id].num_map; i++ ) {
		if( instance->list[map->list[m].instance_id].map[i] == m ) {
			instance->list[map->list[m].instance_id].num_map--;
			for( ; i < instance->list[map->list[m].instance_id].num_map; i++ )
				instance->list[map->list[m].instance_id].map[i] = instance->list[map->list[m].instance_id].map[i+1];
			i = -1;
			break;
		}
	}

	if( i == instance->list[map->list[m].instance_id].num_map )
		ShowError("map_instance_del: failed to remove %s from instance list (%s): %d\n", map->list[m].name, instance->list[map->list[m].instance_id].name, m);

	if( map->list[m].channel )
		channel->delete(map->list[m].channel);

	map->removemapdb(&map->list[m]);
	memset(&map->list[m], 0x00, sizeof(map->list[0]));
	map->list[m].name[0] = 0;
	map->list[m].instance_id = -1;
	map->list[m].mob_delete_timer = INVALID_TIMER;
}

/*--------------------------------------
 * Timer to destroy instance by process or idle
 *--------------------------------------*/
int instance_destroy_timer(int tid, int64 tick, int id, intptr_t data) {
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
	int type, j;
	unsigned int now = (unsigned int)time(NULL);

	if( !instance->valid(instance_id) )
		return; // nothing to do

	if( instance->list[instance_id].progress_timeout && instance->list[instance_id].progress_timeout <= now )
		type = 1;
	else if( instance->list[instance_id].idle_timeout && instance->list[instance_id].idle_timeout <= now )
		type = 2;
	else
		type = 3;

	clif->instance(instance_id, 5, type); // Report users this instance has been destroyed

	switch ( instance->list[instance_id].owner_type ) {
		case IOT_NONE:
			break;
		case IOT_CHAR:
			if( ( sd = map->id2sd(instance->list[instance_id].owner_id) ) == NULL ) {
				break;
			}
			iptr = sd->instance;
			icptr = &sd->instances;
			break;
		case IOT_PARTY:
			if( ( p = party->search(instance->list[instance_id].owner_id) ) == NULL ) {
				break;
			}
			iptr = p->instance;
			icptr = &p->instances;
			break;
		case IOT_GUILD:
			if( ( g = guild->search(instance->list[instance_id].owner_id) ) == NULL ) {
				break;
			}
			iptr = g->instance;
			icptr = &g->instances;
			break;
		default:
			ShowError("instance_destroy: unknown type %u for owner_id %d and name '%s'.\n", instance->list[instance_id].owner_type, instance->list[instance_id].owner_id, instance->list[instance_id].name);
			break;
	}

	if( iptr != NULL ) {
		ARR_FIND(0, *icptr, j, iptr[j] == instance_id);
		if( j != *icptr )
			iptr[j] = -1;
	}

	if (instance->list[instance_id].map) {
		int last = 0;
		while (instance->list[instance_id].num_map && last != instance->list[instance_id].map[0]) {
			// Remove all maps from instance
			last = instance->list[instance_id].map[0];
			instance->del_map( instance->list[instance_id].map[0] );
		}
	}

	if( instance->list[instance_id].regs.vars )
		db_destroy(instance->list[instance_id].regs.vars);
	if( instance->list[instance_id].regs.arrays )
		instance->list[instance_id].regs.arrays->destroy(instance->list[instance_id].regs.arrays, script->array_free_db);

	if( instance->list[instance_id].progress_timer != INVALID_TIMER )
		timer->delete( instance->list[instance_id].progress_timer, instance->destroy_timer);
	if( instance->list[instance_id].idle_timer != INVALID_TIMER )
		timer->delete( instance->list[instance_id].idle_timer, instance->destroy_timer);

	instance->list[instance_id].regs.vars = NULL;

	if( instance->list[instance_id].map )
		aFree(instance->list[instance_id].map);

	instance->list[instance_id].map = NULL;
	instance->list[instance_id].state = INSTANCE_FREE;
	instance->list[instance_id].num_map = 0;

	HPM->data_store_destroy(&instance->list[instance_id].hdata);
}

/*--------------------------------------
 * Checks if there are users in the instance or not to start idle timer
 *--------------------------------------*/
void instance_check_idle(int instance_id) {
	bool idle = true;
	unsigned int now = (unsigned int)time(NULL);

	if( !instance->valid(instance_id) || instance->list[instance_id].idle_timeoutval == 0 )
		return;

	if( instance->list[instance_id].users )
		idle = false;

	if( instance->list[instance_id].idle_timer != INVALID_TIMER && !idle ) {
		timer->delete(instance->list[instance_id].idle_timer, instance->destroy_timer);
		instance->list[instance_id].idle_timer = INVALID_TIMER;
		instance->list[instance_id].idle_timeout = 0;
		clif->instance(instance_id, 3, 0); // Notify instance users normal instance expiration
	} else if( instance->list[instance_id].idle_timer == INVALID_TIMER && idle ) {
		instance->list[instance_id].idle_timeout = now + instance->list[instance_id].idle_timeoutval;
		instance->list[instance_id].idle_timer = timer->add( timer->gettick() + instance->list[instance_id].idle_timeoutval * 1000, instance->destroy_timer, instance_id, 0);
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

	if( instance->list[instance_id].progress_timer != INVALID_TIMER )
		timer->delete( instance->list[instance_id].progress_timer, instance->destroy_timer);
	if( instance->list[instance_id].idle_timer != INVALID_TIMER )
		timer->delete( instance->list[instance_id].idle_timer, instance->destroy_timer);

	if( progress_timeout ) {
		instance->list[instance_id].progress_timeout = now + progress_timeout;
		instance->list[instance_id].progress_timer = timer->add( timer->gettick() + progress_timeout * 1000, instance->destroy_timer, instance_id, 0);
		instance->list[instance_id].original_progress_timeout = progress_timeout;
	} else {
		instance->list[instance_id].progress_timeout = 0;
		instance->list[instance_id].progress_timer = INVALID_TIMER;
		instance->list[instance_id].original_progress_timeout = 0;
	}

	if( idle_timeout ) {
		instance->list[instance_id].idle_timeoutval = idle_timeout;
		instance->list[instance_id].idle_timer = INVALID_TIMER;
		instance->check_idle(instance_id);
	} else {
		instance->list[instance_id].idle_timeoutval = 0;
		instance->list[instance_id].idle_timeout = 0;
		instance->list[instance_id].idle_timer = INVALID_TIMER;
	}

	if( instance->list[instance_id].idle_timer == INVALID_TIMER && instance->list[instance_id].progress_timer != INVALID_TIMER )
		clif->instance(instance_id, 3, 0);
}

/*--------------------------------------
 * Checks if sd in on a instance and should be kicked from it
 *--------------------------------------*/
void instance_check_kick(struct map_session_data *sd) {
	int16 m = sd->bl.m;

	nullpo_retv(sd);
	clif->instance_leave(sd->fd);
	if( map->list[m].instance_id >= 0 ) { // User was on the instance map
		if( map->list[m].save.map )
			pc->setpos(sd, map->list[m].save.map, map->list[m].save.x, map->list[m].save.y, CLR_TELEPORT);
		else
			pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
	}
}

void do_reload_instance(void) {
	struct s_mapiterator *iter;
	struct map_session_data *sd;
	int i, k;

	for(i = 0; i < instance->instances; i++) {
		for(k = 0; k < instance->list[i].num_map; k++) {
			if( !map->list[map->list[instance->list[i].map[k]].instance_src_map].flag.src4instance )
				break;
		}

		if( k != instance->list[i].num_map ) /* any (or all) of them were disabled, we destroy */
			instance->destroy(i);
		else {
			/* populate the instance again */
			instance->start(i);
			/* restart timers */
			instance->set_timeout(i,instance->list[i].original_progress_timeout,instance->list[i].idle_timeoutval);
		}
	}

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if(sd && map->list[sd->bl.m].instance_id >= 0) {
			pc->setpos(sd,instance->list[map->list[sd->bl.m].instance_id].respawn.map,instance->list[map->list[sd->bl.m].instance_id].respawn.x,instance->list[map->list[sd->bl.m].instance_id].respawn.y,CLR_TELEPORT);
		}
	}
	mapit->free(iter);
}

void do_final_instance(void) {
	int i;

	for(i = 0; i < instance->instances; i++) {
		instance->destroy(i);
	}

	if( instance->list )
		aFree(instance->list);

	instance->list = NULL;
	instance->instances = 0;
}

void do_init_instance(bool minimal) {
	if (minimal)
		return;

	timer->add_func_list(instance->destroy_timer, "instance_destroy_timer");
}

void instance_defaults(void) {
	instance = &instance_s;

	instance->init = do_init_instance;
	instance->final = do_final_instance;
	instance->reload = do_reload_instance;
	/* start point */
	instance->start_id = 0;
	/* count */
	instance->instances = 0;
	/* */
	instance->list = NULL;
	/* */
	instance->create = instance_create;
	instance->add_map = instance_add_map;
	instance->del_map = instance_del_map;
	instance->map2imap = instance_map2imap;
	instance->mapid2imapid = instance_mapid2imapid;
	instance->mapname2imap = instance_mapname2imap;
	instance->map_npcsub = instance_map_npcsub;
	instance->init_npc = instance_init_npc;
	instance->destroy = instance_destroy;
	instance->start = instance_init;
	instance->check_idle = instance_check_idle;
	instance->check_kick = instance_check_kick;
	instance->set_timeout = instance_set_timeout;
	instance->valid = instance_is_valid;
	instance->destroy_timer = instance_destroy_timer;
}
