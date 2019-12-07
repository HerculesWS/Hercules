/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "config/core.h" // GP_BOUND_ITEMS
#include "guild.h"

#include "map/battle.h"
#include "map/channel.h"
#include "map/clif.h"
#include "map/instance.h"
#include "map/intif.h"
#include "map/log.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/storage.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct guild_interface guild_s;
struct guild_interface *guild;

/*==========================================
 * Retrieves and validates the sd pointer for this guild member [Skotlex]
 *------------------------------------------*/
static struct map_session_data *guild_sd_check(int guild_id, int account_id, int char_id)
{
	struct map_session_data *sd = map->id2sd(account_id);

	if (!(sd && sd->status.char_id == char_id))
		return NULL;

	if (sd->status.guild_id != guild_id) {
		//If player belongs to a different guild, kick him out.
		intif->guild_leave(guild_id,account_id,char_id,0,"** Guild Mismatch **");
		return NULL;
	}

	return sd;
}

// Modified [Komurka]
static int guild_skill_get_max(int id)
{
	if (id < GD_SKILLBASE || id >= GD_SKILLBASE+MAX_GUILDSKILL)
		return 0;
	return guild->skill_tree[id-GD_SKILLBASE].max;
}

// Retrieve skill_lv learned by guild
static int guild_checkskill(struct guild *g, int id)
{
	int idx = id - GD_SKILLBASE;
	nullpo_ret(g);
	if (idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;
	return g->skill[idx].lv;
}

/*==========================================
 * guild_skill_tree.txt reading - from jA [Komurka]
 *------------------------------------------*/
static bool guild_read_guildskill_tree_db(char *split[], int columns, int current)
{// <skill id>,<max lv>,<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>
	int k, id, skill_id;

	skill_id = atoi(split[0]);
	id = skill_id - GD_SKILLBASE;

	if( id < 0 || id >= MAX_GUILDSKILL )
	{
		ShowWarning("guild_read_guildskill_tree_db: Invalid skill id %d.\n", skill_id);
		return false;
	}

	guild->skill_tree[id].id = skill_id;
	guild->skill_tree[id].max = atoi(split[1]);

	if( guild->skill_tree[id].id == GD_GLORYGUILD && battle_config.require_glory_guild && guild->skill_tree[id].max == 0 )
	{// enable guild's glory when required for emblems
		guild->skill_tree[id].max = 1;
	}

	for( k = 0; k < MAX_GUILD_SKILL_REQUIRE; k++ )
	{
		guild->skill_tree[id].need[k].id = atoi(split[k*2+2]);
		guild->skill_tree[id].need[k].lv = atoi(split[k*2+3]);
	}

	return true;
}

/*==========================================
 * Guild skill check - from jA [Komurka]
 *------------------------------------------*/
static int guild_check_skill_require(struct guild *g, int id)
{
	int i;
	int idx = id-GD_SKILLBASE;

	if(g == NULL)
		return 0;

	if (idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;

	for(i=0;i<MAX_GUILD_SKILL_REQUIRE;i++)
	{
		if(guild->skill_tree[idx].need[i].id == 0) break;
		if(guild->skill_tree[idx].need[i].lv > guild->checkskill(g,guild->skill_tree[idx].need[i].id))
			return 0;
	}
	return 1;
}

static bool guild_read_castledb_libconfig(void)
{
	struct config_t castle_conf;
	struct config_setting_t *castle_db = NULL, *it = NULL;
	char config_filename[256];
	libconfig->format_db_path("castle_db.conf", config_filename, sizeof(config_filename));
	int i = 0;

	if (libconfig->load_file(&castle_conf, config_filename) == 0)
		return false;

	if ((castle_db = libconfig->setting_get_member(castle_conf.root, "castle_db")) == NULL) {
		libconfig->destroy(&castle_conf);
		ShowError("guild_read_castledb_libconfig: can't read %s\n", config_filename);
		return false;
	}

	while ((it = libconfig->setting_get_elem(castle_db, i++)) != NULL) {
		guild->read_castledb_libconfig_sub(it, i - 1, config_filename);
	}

	libconfig->destroy(&castle_conf);
	ShowStatus("Done reading '"CL_WHITE"%u"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", db_size(guild->castle_db), config_filename);
	return true;
}

static bool guild_read_castledb_libconfig_sub(struct config_setting_t *it, int idx, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(source);

	struct guild_castle *gc = NULL;
	const char *name = NULL;
	int i32 = 0;
	CREATE(gc, struct guild_castle, 1);

	if (libconfig->setting_lookup_int(it, "CastleID", &i32) == CONFIG_FALSE) {
		aFree(gc);
		ShowWarning("guild_read_castledb_libconfig_sub: Invalid or missing CastleID (%d) in \"%s\", entry #%d, skipping.\n", i32, source, idx);
		return false;
	}
	gc->castle_id = i32;

	if (libconfig->setting_lookup_string(it, "MapName", &name) == CONFIG_FALSE) {
		aFree(gc);
		ShowWarning("guild_read_castledb_libconfig_sub: Invalid or missing MapName in \"%s\", entry #%d, skipping.\n", source, idx);
		return false;
	}
	int index = mapindex->name2id(name);
	if (map->mapindex2mapid(index) < 0) {
		aFree(gc);
		ShowWarning("guild_read_castledb_libconfig_sub: Invalid MapName (%s) in \"%s\", entry #%d, skipping.\n", name, source, idx);
		return false;
	}
	gc->mapindex = index;

	if (libconfig->setting_lookup_string(it, "CastleName", &name) == CONFIG_FALSE) {
		aFree(gc);
		ShowWarning("guild_read_castledb_libconfig_sub: Invalid CastleName in \"%s\", entry #%d, skipping.\n", source, idx);
		return false;
	}
	safestrncpy(gc->castle_name, name, sizeof(gc->castle_name));

	if (libconfig->setting_lookup_string(it, "OnGuildBreakEventName", &name) == CONFIG_FALSE){
		aFree(gc);
		ShowWarning("guild_read_castledb_libconfig_sub: Invalid OnGuildBreakEventName in \"%s\", entry #%d, skipping.\n", source, idx);
		return false;
	}
	safestrncpy(gc->castle_event, name, sizeof(gc->castle_event));

	if (itemdb->lookup_const(it, "SiegeType", &i32) && (i32 >= SIEGE_TYPE_MAX || i32 < 0)) {
		ShowWarning("guild_read_castledb_libconfig_sub: Invalid SiegeType in \"%s\", entry #%d, defaulting to SIEGE_TYPE_FE.\n", source, idx);
		gc->siege_type = SIEGE_TYPE_FE;
	} else {
		gc->siege_type = i32;
	}

	libconfig->setting_lookup_bool_real(it, "EnableClientWarp", &gc->enable_client_warp);
	if (gc->enable_client_warp == true) {
		struct config_setting_t *wd = libconfig->setting_get_member(it, "ClientWarp");
		guild->read_castledb_libconfig_sub_warp(wd, source, gc);
	}
	idb_put(guild->castle_db, gc->castle_id, gc);
	return true;
}

static bool guild_read_castledb_libconfig_sub_warp(struct config_setting_t *wd, const char *source, struct guild_castle *gc)
{
	nullpo_retr(false, wd);
	nullpo_retr(false, gc);
	nullpo_retr(false, source);

	int64 i64 = 0;
	struct config_setting_t *it = libconfig->setting_get_member(wd, "Position");
	if (config_setting_is_list(it)) {
		int m = map->mapindex2mapid(gc->mapindex);

		gc->client_warp.x = libconfig->setting_get_int_elem(it, 0);
		gc->client_warp.y = libconfig->setting_get_int_elem(it, 1);
		if (gc->client_warp.x < 0 || gc->client_warp.x >= map->list[m].xs || gc->client_warp.y < 0 || gc->client_warp.y >= map->list[m].ys) {
			ShowWarning("guild_read_castledb_libconfig_sub_warp: Invalid Position in \"%s\", for castle (%d).\n", source, gc->castle_id);
			return false;
		}
	} else {
		ShowWarning("guild_read_castledb_libconfig_sub_warp: Invalid format for Position in \"%s\", for castle (%d).\n", source, gc->castle_id);
		return false;
	}

	if (libconfig->setting_lookup_int64(wd, "ZenyCost", &i64)) {
		if (i64 > MAX_ZENY) {
			ShowWarning("guild_read_castledb_libconfig_sub_warp: ZenyCost is too big in \"%s\", for castle (%d), capping to MAX_ZENY.\n", source, gc->castle_id);
		}
		gc->client_warp.zeny = cap_value((int)i64, 0, MAX_ZENY);
	}
	if (libconfig->setting_lookup_int64(wd, "ZenyCostSiegeTime", &i64)) {
		if (i64 > MAX_ZENY) {
			ShowWarning("guild_read_castledb_libconfig_sub_warp: ZenyCostSiegeTime is too big in \"%s\", for castle (%d), capping to MAX_ZENY.\n", source, gc->castle_id);
		}
		gc->client_warp.zeny_siege = cap_value((int)i64, 0, MAX_ZENY);
	}
	return true;
}

/// lookup: guild id -> guild*
static struct guild *guild_search(int guild_id)
{
	return (struct guild*)idb_get(guild->db,guild_id);
}

/// lookup: guild name -> guild*
static struct guild *guild_searchname(const char *str)
{
	struct guild* g;
	struct DBIterator *iter = db_iterator(guild->db);

	nullpo_retr(NULL, str);
	for( g = dbi_first(iter); dbi_exists(iter); g = dbi_next(iter) )
	{
		if( strcmpi(g->name, str) == 0 )
			break;
	}
	dbi_destroy(iter);

	return g;
}

/// lookup: castle id -> castle*
static struct guild_castle *guild_castle_search(int gcid)
{
	return (struct guild_castle*)idb_get(guild->castle_db,gcid);
}

/// lookup: map index -> castle*
static struct guild_castle *guild_mapindex2gc(short map_index)
{
	struct guild_castle* gc;
	struct DBIterator *iter = db_iterator(guild->castle_db);

	for( gc = dbi_first(iter); dbi_exists(iter); gc = dbi_next(iter) )
	{
		if( gc->mapindex == map_index )
			break;
	}
	dbi_destroy(iter);

	return gc;
}

/// lookup: map name -> castle*
static struct guild_castle *guild_mapname2gc(const char *mapname)
{
	return guild->mapindex2gc(mapindex->name2id(mapname));
}

static struct map_session_data *guild_getavailablesd(struct guild *g)
{
	int i;

	nullpo_retr(NULL, g);

	ARR_FIND( 0, g->max_member, i, g->member[i].sd != NULL );
	return( i < g->max_member ) ? g->member[i].sd : NULL;
}

/// lookup: player AID/CID -> member index
static int guild_getindex(const struct guild *g, int account_id, int char_id)
{
	int i;

	if( g == NULL )
		return INDEX_NOT_FOUND;

	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if (i == g->max_member)
		return INDEX_NOT_FOUND;

	return i;
}

/// lookup: player sd -> member position
static int guild_getposition(struct guild *g, struct map_session_data *sd)
{
	int i;

	if( g == NULL && (g=sd->guild) == NULL )
		return -1;

	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == sd->status.account_id && g->member[i].char_id == sd->status.char_id );
	return( i < g->max_member ) ? g->member[i].position : -1;
}

//Creation of member information
static void guild_makemember(struct guild_member *m, struct map_session_data *sd)
{
	nullpo_retv(sd);
	nullpo_retv(m);

	memset(m,0,sizeof(struct guild_member));
	m->account_id = sd->status.account_id;
	m->char_id    = sd->status.char_id;
	m->hair       = sd->status.hair;
	m->hair_color = sd->status.hair_color;
	m->gender     = sd->status.sex;
	m->class      = sd->status.class;
	m->lv         = sd->status.base_level;
	//m->exp        = 0;
	//m->exp_payper = 0;
	m->online     = 1;
	m->position   = MAX_GUILDPOSITION-1;
	memcpy(m->name,sd->status.name,NAME_LENGTH);
	m->last_login = (uint32)time(NULL); // When player create or join a guild the date is updated
	return;
}

/**
 * Server cache to be flushed to inter the Guild EXP
 * @see DBApply
 */
static int guild_payexp_timer_sub(union DBKey key, struct DBData *data, va_list ap)
{
	int i;
	struct guild_expcache *c;
	struct guild *g;

	c = DB->data2ptr(data);

	if ((g = guild->search(c->guild_id)) == NULL
	 || (i = guild->getindex(g, c->account_id, c->char_id)) == INDEX_NOT_FOUND
	) {
		ers_free(guild->expcache_ers, c);
		return 0;
	}

	if (g->member[i].exp > UINT64_MAX - c->exp)
		g->member[i].exp = UINT64_MAX;
	else
		g->member[i].exp+= c->exp;

	intif->guild_change_memberinfo(g->guild_id,c->account_id,c->char_id,
		GMI_EXP,&g->member[i].exp,sizeof(g->member[i].exp));
	c->exp=0;

	ers_free(guild->expcache_ers, c);
	return 0;
}

static int guild_payexp_timer(int tid, int64 tick, int id, intptr_t data)
{
	guild->expcache_db->clear(guild->expcache_db,guild->payexp_timer_sub);
	return 0;
}

/**
 * Taken from party_send_xy_timer_sub. [Skotlex]
 * @see DBApply
 */
static int guild_send_xy_timer_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct guild *g = DB->data2ptr(data);
	int i;

	nullpo_ret(g);

	if( !g->connect_member )
	{// no members connected to this guild so do not iterate
		return 0;
	}

	for(i=0;i<g->max_member;i++){
		struct map_session_data* sd = g->member[i].sd;
		if( sd != NULL && sd->fd && (sd->guild_x != sd->bl.x || sd->guild_y != sd->bl.y) && !sd->bg_id )
		{
			clif->guild_xy(sd);
			sd->guild_x = sd->bl.x;
			sd->guild_y = sd->bl.y;
		}
	}
	return 0;
}

//Code from party_send_xy_timer [Skotlex]
static int guild_send_xy_timer(int tid, int64 tick, int id, intptr_t data)
{
	guild->db->foreach(guild->db,guild->send_xy_timer_sub,tick);
	return 0;
}

static int guild_send_dot_remove(struct map_session_data *sd)
{
	nullpo_ret(sd);
	if (sd->status.guild_id)
		clif->guild_xy_remove(sd);
	return 0;
}
//------------------------------------------------------------------------

static int guild_create(struct map_session_data *sd, const char *name)
{
	char tname[NAME_LENGTH];
	struct guild_member m;
	nullpo_ret(sd);
	nullpo_ret(name);

	if (sd->clan != NULL) {
		clif->messagecolor_self(sd->fd, COLOR_RED, "You cannot create a guild because you are in a clan.");
		return 0;
	}

	safestrncpy(tname, name, NAME_LENGTH);
	trim(tname);

	if( !tname[0] )
		return 0; // empty name

	if( sd->status.guild_id ) {
		clif->guild_created(sd,1); // You're already in a guild
		return 0;
	}
	if (battle_config.guild_emperium_check && pc->search_inventory(sd, ITEMID_EMPERIUM) == INDEX_NOT_FOUND) {
		clif->guild_created(sd,3); // You need the necessary item to create a guild
		return 0;
	}

	guild->makemember(&m,sd);
	m.position=0;
	intif->guild_create(name,&m);
	return 1;
}

//Whether or not to create guild
static int guild_created(int account_id, int guild_id)
{
	struct map_session_data *sd=map->id2sd(account_id);

	if(sd==NULL)
		return 0;
	if(!guild_id) {
		clif->guild_created(sd, 2); // Creation failure (The guild name already exists)
		return 0;
	}
	//struct guild *g;
	sd->status.guild_id=guild_id;
	clif->guild_created(sd,0); // Success
	if (battle_config.guild_emperium_check) {
		int n = pc->search_inventory(sd, ITEMID_EMPERIUM);
		if (n != INDEX_NOT_FOUND)
			pc->delitem(sd, n, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME); //emperium consumption
	}
	return 0;
}

//Information request
static int guild_request_info(int guild_id)
{
	return intif->guild_request_info(guild_id);
}

//Information request with event
static int guild_npc_request_info(int guild_id, const char *event)
{
	if (guild->search(guild_id) != NULL) {
		if (event != NULL && *event != '\0')
			npc->event_do(event);

		return 0;
	}

	if (event != NULL && *event != '\0') {
		struct eventlist *ev;
		struct DBData prev;
		CREATE(ev, struct eventlist, 1);
		safestrncpy(ev->name, event, sizeof(ev->name));
		//The one in the db (if present) becomes the next event from this.
		if (guild->infoevent_db->put(guild->infoevent_db, DB->i2key(guild_id), DB->ptr2data(ev), &prev))
			ev->next = DB->data2ptr(&prev);
	}

	return guild->request_info(guild_id);
}

//Confirmation of the character belongs to guild
static int guild_check_member(const struct guild *g)
{
	int i;
	struct map_session_data *sd;
	struct s_mapiterator* iter;

	nullpo_ret(g);

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if( sd->status.guild_id != g->guild_id )
			continue;

		i = guild->getindex(g,sd->status.account_id,sd->status.char_id);
		if (i == INDEX_NOT_FOUND) {
			sd->status.guild_id=0;
			sd->guild_emblem_id=0;
			sd->guild = NULL;
			ShowWarning("guild: check_member %d[%s] is not member\n",sd->status.account_id,sd->status.name);
		}
	}
	mapit->free(iter);

	return 0;
}

//Delete association with guild_id for all characters
static int guild_recv_noinfo(int guild_id)
{
	struct map_session_data *sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
		if (sd->status.guild_id == guild_id) {
			sd->status.guild_id = 0; // erase guild
			sd->guild_emblem_id = 0;
			sd->guild = NULL;
		}
	}
	mapit->free(iter);

	return 0;
}

//Get and display information for all member
static int guild_recv_info(const struct guild *sg)
{
	struct guild *g,before;
	int i,bm,m;
	struct DBData data;
	struct map_session_data *sd;
	bool guild_new = false;
	struct channel_data *aChSysSave = NULL;
	short *instance_save = NULL;
	unsigned short instances_save = 0;

	nullpo_ret(sg);

	if((g = (struct guild*)idb_get(guild->db,sg->guild_id))==NULL) {
		guild_new = true;
		g=(struct guild *)aCalloc(1,sizeof(struct guild));
		g->instance = NULL;
		g->instances = 0;
		idb_put(guild->db,sg->guild_id,g);
		if (channel->config->ally) {
			struct channel_data *chan = channel->create(HCS_TYPE_ALLY, channel->config->ally_name, channel->config->ally_color);
			if (channel->config->ally_autojoin) {
				struct s_mapiterator* iter = mapit_getallusers();
				struct guild *tg[MAX_GUILDALLIANCE];

				for (i = 0; i < MAX_GUILDALLIANCE; i++) {
					tg[i] = NULL;
					if( sg->alliance[i].opposition == 0 && sg->alliance[i].guild_id )
						tg[i] = guild->search(sg->alliance[i].guild_id);
				}

				for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
					if (!sd->status.guild_id)
						continue; // Not interested in guildless users

					if (sd->status.guild_id == sg->guild_id) {
						// Guild member
						channel->join_sub(chan,sd,false);
						sd->guild = g;

						for (i = 0; i < MAX_GUILDALLIANCE; i++) {
							// Join channels from allied guilds
							if (tg[i] && !(tg[i]->channel->banned && idb_exists(tg[i]->channel->banned, sd->status.account_id)))
								channel->join_sub(tg[i]->channel, sd, false);
						}
						continue;
					}

					for (i = 0; i < MAX_GUILDALLIANCE; i++) {
						if (tg[i] && sd->status.guild_id == tg[i]->guild_id) { // Shortcut to skip the alliance checks again
							// Alliance member
							if( !(chan->banned && idb_exists(chan->banned, sd->status.account_id)))
								channel->join_sub(chan, sd, false);
						}
					}
				}
				mapit->free(iter);
			}
			aChSysSave = chan;

		}
		before=*sg;
		//Perform the check on the user because the first load
		guild->check_member(sg);
		if ((sd = map->nick2sd(sg->master, false)) != NULL) {
			//If the guild master is online the first time the guild_info is received,
			//that means he was the first to join, so apply guild skill blocking here.
			if( battle_config.guild_skill_relog_delay == 1)
				guild->block_skill(sd, 300000);

			//Also set the guild master flag.
			sd->guild = g;
			sd->state.gmaster_flag = 1;
			clif->charnameupdate(sd); // [LuzZza]
			clif->guild_masterormember(sd);
		}
	} else {
		before=*g;
		if( g->channel )
			aChSysSave = g->channel;
		if( g->instance )
			instance_save = g->instance;
		if( g->instances )
			instances_save = g->instances;
	}
	memcpy(g,sg,sizeof(struct guild));

	g->channel = aChSysSave;
	g->instance = instance_save;
	g->instances = instances_save;

	if(g->max_member > MAX_GUILD) {
		ShowError("guild_recv_info: Received guild with %d members, but MAX_GUILD is only %d. Extra guild-members have been lost!\n", g->max_member, MAX_GUILD);
		g->max_member = MAX_GUILD;
	}

	for(i=bm=m=0;i<g->max_member;i++){
		if(g->member[i].account_id>0){
			sd = g->member[i].sd = guild->sd_check(g->guild_id, g->member[i].account_id, g->member[i].char_id);
			if (sd) clif->charnameupdate(sd); // [LuzZza]
			m++;
		}else
			g->member[i].sd=NULL;
		if(before.member[i].account_id>0)
			bm++;
	}

	for (i = 0; i < g->max_member; i++) { //Transmission of information at all members
		sd = g->member[i].sd;
		if( sd==NULL )
			continue;
		sd->guild = g;
		if (before.guild_lv != g->guild_lv || bm != m
		 || before.max_member != g->max_member) {
			clif->guild_basicinfo(sd); //Submit basic information
			clif->guild_emblem(sd, g); //Submit emblem
		}

		if (bm != m) { //Send members information
			clif->guild_memberlist(g->member[i].sd);
		}

		if (before.skill_point != g->skill_point)
			clif->guild_skillinfo(sd); //Submit information skills

		if (guild_new) { // Send information and affiliation if unsent
			clif->guild_belonginfo(sd, g);
			//clif->guild_notice(sd, g); Is already sent in clif_parse_LoadEndAck
			sd->guild_emblem_id = g->emblem_id;
		}
	}

	//Occurrence of an event
	if (guild->infoevent_db->remove(guild->infoevent_db, DB->i2key(sg->guild_id), &data)) {
		struct eventlist *ev = DB->data2ptr(&data), *ev2;
		while(ev) {
			npc->event_do(ev->name);
			ev2=ev->next;
			aFree(ev);
			ev=ev2;
		}
	}

	return 0;
}

/*=============================================
 * Player sd send a guild invatation to player tsd to join his guild
 *--------------------------------------------*/
static int guild_invite(struct map_session_data *sd, struct map_session_data *tsd)
{
	struct guild *g;
	int i;

	nullpo_ret(sd);

	g=sd->guild;

	if(tsd==NULL || g==NULL)
		return 0;

	if( (i=guild->getposition(g,sd)) < 0 || !(g->position[i].mode&GPERM_INVITE) )
		return 0; //Invite permission.

	if(!battle_config.invite_request_check) {
		if (tsd->party_invite > 0 || tsd->trade_partner || tsd->adopt_invite) { //checking if there no other invitation pending
			clif->guild_inviteack(sd,0);
			return 0;
		}
	}

	if (!tsd->fd) { //You can't invite someone who has already disconnected.
		clif->guild_inviteack(sd,1);
		return 0;
	}

	if( tsd->status.guild_id > 0
	 || tsd->guild_invite > 0
	 || ( (map->agit_flag || map->agit2_flag)
		   && map->list[tsd->bl.m].flag.gvg_castle
		   && !battle_config.guild_castle_invite
		   )
	) {
		//Can't invite people inside castles. [Skotlex]
		clif->guild_inviteack(sd,0);
		return 0;
	}

	//search an empty spot in guild
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == 0 );
	if(i==g->max_member){
		clif->guild_inviteack(sd,3);
		return 0;
	}

	tsd->guild_invite=sd->status.guild_id;
	tsd->guild_invite_account=sd->status.account_id;

	clif->guild_invite(tsd,g);
	return 0;
}

/// Guild invitation reply.
/// flag: 0:rejected, 1:accepted
static int guild_reply_invite(struct map_session_data *sd, int guild_id, int flag)
{
	struct map_session_data* tsd;

	nullpo_ret(sd);

	// subsequent requests may override the value
	if( sd->guild_invite != guild_id )
		return 0; // mismatch

	// look up the person who sent the invite
	//NOTE: this can be NULL because the person might have logged off in the meantime
	tsd = map->id2sd(sd->guild_invite_account);

	if ( sd->status.guild_id > 0 ) {
		// Already in another guild. [Paradox924X]
		if ( tsd ) clif->guild_inviteack(tsd,0);
		return 0;
	}
	else if( flag == 0 )
	{// rejected
		sd->guild_invite = 0;
		sd->guild_invite_account = 0;
		if( tsd ) clif->guild_inviteack(tsd,1);
	}
	else
	{// accepted
		struct guild* g;
		int i;

		if( (g=guild->search(guild_id)) == NULL )
		{
			sd->guild_invite = 0;
			sd->guild_invite_account = 0;
			return 0;
		}

		ARR_FIND( 0, g->max_member, i, g->member[i].account_id == 0 );
		if( i == g->max_member )
		{
			sd->guild_invite = 0;
			sd->guild_invite_account = 0;
			if( tsd ) clif->guild_inviteack(tsd,3);
			return 0;
		}

		guild->makemember(&g->member[i], sd);
		intif->guild_addmember(guild_id, &g->member[i]);
		//TODO: send a minimap update to this player
	}

	return 0;
}

//Invoked when a player joins.
//- If guild is not in memory, it is requested
//- Otherwise sd pointer is set up.
//- Player must be authed and must belong to a guild before invoking this method
static void guild_member_joined(struct map_session_data *sd)
{
	struct guild* g;
	int i;
	nullpo_retv(sd);
	g=guild->search(sd->status.guild_id);
	if (!g) {
		guild->request_info(sd->status.guild_id);
		return;
	}
	if (strcmp(sd->status.name,g->master) == 0) {
		// set the Guild Master flag
		sd->state.gmaster_flag = 1;
		// prevent Guild Skills from being used directly after relog
		if( battle_config.guild_skill_relog_delay == 1 )
			guild->block_skill(sd, 300000);
	}
	i = guild->getindex(g, sd->status.account_id, sd->status.char_id);
	if (i == INDEX_NOT_FOUND) {
		sd->status.guild_id = 0;
		sd->guild_emblem_id = 0;
		sd->guild = NULL;
	} else {
		g->member[i].sd = sd;
		sd->guild = g;

		if (channel->config->ally && channel->config->ally_autojoin) {
			channel->join(g->channel, sd, "", true);
		}

		for (int j = 0; j < g->instances; j++) {
			if (g->instance[j] >= 0) {
				clif->instance_join(sd->fd, g->instance[j]);
				break;
			}
		}
	}
}

/*==========================================
 * Add a player to a given guild_id
 *----------------------------------------*/
static int guild_member_added(int guild_id, int account_id, int char_id, int flag)
{
	struct map_session_data *sd = map->id2sd(account_id),*sd2;
	struct guild *g;

	if( (g=guild->search(guild_id))==NULL )
		return 0;

	if(sd==NULL || sd->guild_invite==0){
		// cancel if player not present or invalid guild_id invitation
		if (flag == 0) {
			ShowError("guild: member added error %d is not online\n",account_id);
			intif->guild_leave(guild_id,account_id,char_id,0,"** Data Error **");
		}
		return 0;
	}
	sd2 = map->id2sd(sd->guild_invite_account);
	sd->guild_invite = 0;
	sd->guild_invite_account = 0;

	if (flag == 1) { //failure
		if( sd2!=NULL )
			clif->guild_inviteack(sd2,3);
		return 0;
	}

	//if all ok add player to guild
	sd->status.guild_id = g->guild_id;
	sd->guild_emblem_id = g->emblem_id;
	sd->guild = g;
	//Packets which were sent in the previous 'guild_sent' implementation.
	clif->guild_belonginfo(sd,g);
	clif->guild_notice(sd,g);

	//TODO: send new emblem info to others

	if( sd2!=NULL )
		clif->guild_inviteack(sd2,2);

	//Next line commented because it do nothing, look at guild_recv_info [LuzZza]
	//clif->charnameupdate(sd); //Update display name [Skotlex]

	// Makes the character join their respective guild's channel for #ally chat
	if (channel->config->ally && channel->config->ally_autojoin) {
		channel->join(g->channel, sd, "", true);
	}

	for (int i = 0; i < g->instances; i++) {
		if (g->instance[i] >= 0) {
			clif->instance_join(sd->fd, g->instance[i]);
			break;
		}
	}

	return 0;
}

/*==========================================
 * Player request leaving a given guild_id
 * mes - non null terminated string
 *----------------------------------------*/
static int guild_leave(struct map_session_data *sd, int guild_id, int account_id, int char_id, const char *mes)
{
	struct guild *g;

	nullpo_ret(sd);

	g = sd->guild;

	if(g==NULL)
		return 0;

	if( sd->status.account_id != account_id
	 || sd->status.char_id != char_id
	 || sd->status.guild_id != guild_id
	 // Can't leave inside castles
	 || ((map->agit_flag || map->agit2_flag)
			&& map->list[sd->bl.m].flag.gvg_castle
			&& !battle_config.guild_castle_expulsion)
		)
		return 0;

	intif->guild_leave(sd->status.guild_id, sd->status.account_id, sd->status.char_id,0,mes);
	return 0;
}

/*==========================================
 * Request remove a player to a given guild_id
 * mes - non null terminated string
 *----------------------------------------*/
static int guild_expulsion(struct map_session_data *sd, int guild_id, int account_id, int char_id, const char *mes)
{
	struct map_session_data *tsd;
	struct guild *g;
	int i,ps;

	nullpo_ret(sd);

	g = sd->guild;

	if(g==NULL)
		return 0;

	if(sd->status.guild_id!=guild_id)
		return 0;

	if ((ps=guild->getposition(g,sd))<0 || !(g->position[ps].mode&GPERM_EXPEL))
		return 0; //Expulsion permission

	//Can't leave inside guild castles.
	if ((tsd = map->id2sd(account_id))
	 && tsd->status.char_id == char_id
	 && ((map->agit_flag || map->agit2_flag)
			&& map->list[sd->bl.m].flag.gvg_castle
			&& !battle_config.guild_castle_expulsion)
			)
		return 0;

	// find the member and perform expulsion
	i = guild->getindex(g, account_id, char_id);
	if (i != INDEX_NOT_FOUND && strcmp(g->member[i].name,g->master) != 0) //Can't expel the GL!
		intif->guild_leave(g->guild_id,account_id,char_id,1,mes);

	return 0;
}

static int guild_member_withdraw(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes)
{
	int i;
	struct guild* g = guild->search(guild_id);
	struct map_session_data* sd = map->charid2sd(char_id);
	struct map_session_data* online_member_sd;

	if(g == NULL)
		return 0; // no such guild (error!)

	i = guild->getindex(g, account_id, char_id);
	if (i == INDEX_NOT_FOUND)
		return 0; // not a member (inconsistency!)

	online_member_sd = guild->getavailablesd(g);
	if(online_member_sd == NULL)
		return 0; // no one online to inform

#ifdef GP_BOUND_ITEMS
	//Guild bound item check
	guild->retrieveitembound(char_id,account_id,guild_id);
#endif

	if(!flag)
		clif->guild_leave(online_member_sd, name, char_id, mes);
	else
		clif->guild_expulsion(online_member_sd, name, char_id, mes, account_id);

	// remove member from guild
	memset(&g->member[i],0,sizeof(struct guild_member));
	clif->guild_memberlist(online_member_sd);

	// update char, if online
	if(sd != NULL && sd->status.guild_id == guild_id) {
		// do stuff that needs the guild_id first, BEFORE we wipe it
		if (sd->state.storage_flag == STORAGE_FLAG_GUILD) //Close the guild storage.
			gstorage->close(sd);
		guild->send_dot_remove(sd);
		if (channel->config->ally) {
			channel->quit_guild(sd);
		}
		sd->status.guild_id = 0;
		sd->guild = NULL;
		sd->guild_emblem_id = 0;
		if( g->instances )
			instance->check_kick(sd);
		clif->charnameupdate(sd); //Update display name [Skotlex]
		status_change_end(&sd->bl, SC_LEADERSHIP, INVALID_TIMER);
		status_change_end(&sd->bl, SC_GLORYWOUNDS, INVALID_TIMER);
		status_change_end(&sd->bl, SC_SOULCOLD, INVALID_TIMER);
		status_change_end(&sd->bl, SC_HAWKEYES, INVALID_TIMER);
		//TODO: send emblem update to self and people around
	}
	return 0;
}

static void guild_retrieveitembound(int char_id, int aid, int guild_id)
{
#ifdef GP_BOUND_ITEMS
	struct map_session_data *sd = map->charid2sd(char_id);
	if (sd != NULL) { //Character is online
		pc->bound_clear(sd,IBT_GUILD);
	} else { //Character is offline, ask char server to do the job
		struct guild_storage *gstor = idb_get(gstorage->db,guild_id);
		if(gstor && gstor->storage_status == 1) { //Someone is in guild storage, close them
			struct s_mapiterator* iter = mapit_getallusers();
			for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter))) {
				if(sd->status.guild_id == guild_id && sd->state.storage_flag == STORAGE_FLAG_GUILD) {
					gstorage->close(sd);
					break;
				}
			}
			mapit->free(iter);
		}
		intif->itembound_req(char_id,aid,guild_id);
	}
#endif
}

// cleaned up [LuzZza]
static int guild_send_memberinfoshort(struct map_session_data *sd, int online)
{
	struct guild *g;

	nullpo_ret(sd);

	if(sd->status.guild_id <= 0)
		return 0;

	if(!(g = sd->guild))
		return 0;

	intif->guild_memberinfoshort(g->guild_id,
		sd->status.account_id,sd->status.char_id,online,sd->status.base_level,sd->status.class);

	if(!online){
		int i = guild->getindex(g,sd->status.account_id,sd->status.char_id);
		if (i != INDEX_NOT_FOUND)
			g->member[i].sd=NULL;
		else
			ShowError("guild_send_memberinfoshort: Failed to locate member %d:%d in guild %d!\n", sd->status.account_id, sd->status.char_id, g->guild_id);
		return 0;
	}

	if (sd->state.connect_new) {
		//Note that this works because it is invoked in parse_LoadEndAck before connect_new is cleared.
		clif->guild_belonginfo(sd,g);
		sd->guild_emblem_id = g->emblem_id;
	}
	return 0;
}

// cleaned up [LuzZza]
static int guild_recv_memberinfoshort(int guild_id, int account_id, int char_id, int online, int lv, int class, uint32 last_login)
{
	int i, alv, c, idx = INDEX_NOT_FOUND, om = 0, oldonline = -1;
	struct guild *g = guild->search(guild_id);

	if(g == NULL)
		return 0;

	for(i=0,alv=0,c=0,om=0;i<g->max_member;i++){
		struct guild_member *m=&g->member[i];
		if(!m->account_id) continue;
		if(m->account_id==account_id && m->char_id==char_id ){
			oldonline=m->online;
			m->online=online;
			m->lv=lv;
			m->class = class;
			m->last_login = last_login;
			idx=i;
		}
		alv+=m->lv;
		c++;
		if(m->online)
			om++;
	}

	if (idx == INDEX_NOT_FOUND || c == 0) {
		//Treat char_id who doesn't match guild_id (not found as member)
		struct map_session_data *sd = map->id2sd(account_id);
		if(sd && sd->status.char_id == char_id) {
			sd->status.guild_id=0;
			sd->guild_emblem_id=0;
		}
		ShowWarning("guild: not found member %d,%d on %d[%s]\n", account_id,char_id,guild_id,g->name);
		return 0;
	}

	g->average_lv=alv/c;
	g->connect_member=om;

	//Ensure validity of pointer (ie: player logs in/out, changes map-server)
	g->member[idx].sd = guild->sd_check(guild_id, account_id, char_id);

	if(oldonline!=online)
		clif->guild_memberlogin_notice(g, idx, online);

	if(!g->member[idx].sd)
		return 0;

	//Send XY dot updates. [Skotlex]
	//Moved from guild_send_memberinfoshort [LuzZza]
	for(i=0; i < g->max_member; i++) {

		if(!g->member[i].sd || i == idx ||
			g->member[i].sd->bl.m != g->member[idx].sd->bl.m)
			continue;

		clif->guild_xy_single(g->member[idx].sd->fd, g->member[i].sd);
		clif->guild_xy_single(g->member[i].sd->fd, g->member[idx].sd);
	}

	return 0;
}

/*====================================================
 * Send a message to whole guild
 *---------------------------------------------------*/
static int guild_send_message(struct map_session_data *sd, const char *mes)
{
	nullpo_ret(sd);

	if (sd->status.guild_id == 0 || sd->guild == NULL)
		return 0;

	int len = (int)strlen(mes);

	clif->guild_message(sd->guild, sd->status.account_id, mes, len);

	// Chat logging type 'G' / Guild Chat
	logs->chat(LOG_CHAT_GUILD, sd->status.guild_id, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, NULL, mes);

	return 0;
}

/*====================================================
 * Member changing position in guild
 *---------------------------------------------------*/
static int guild_change_memberposition(int guild_id, int account_id, int char_id, short idx)
{
	return intif->guild_change_memberinfo(guild_id,account_id,char_id,GMI_POSITION,&idx,sizeof(idx));
}

/*====================================================
 * Notification of new position for member
 *---------------------------------------------------*/
static int guild_memberposition_changed(struct guild *g, int idx, int pos)
{
	nullpo_ret(g);
	Assert_ret(idx >= 0 && idx < MAX_GUILD);

	g->member[idx].position=pos;
	clif->guild_memberpositionchanged(g,idx);

	// Update char position in client [LuzZza]
	if(g->member[idx].sd != NULL)
		clif->guild_position_selected(g->member[idx].sd);
	return 0;
}

/*====================================================
 * Change guild title or member
 *---------------------------------------------------*/
static int guild_change_position(int guild_id, int idx, int mode, int exp_mode, const char *name)
{
	struct guild_position p;
	nullpo_ret(name);

	exp_mode = cap_value(exp_mode, 0, battle_config.guild_exp_limit);
	p.mode=mode&GPERM_MASK;
	p.exp_mode=exp_mode;
	safestrncpy(p.name,name,NAME_LENGTH);
	return intif->guild_position(guild_id,idx,&p);
}

/*====================================================
 * Notification of member has changed his guild title
 *---------------------------------------------------*/
static int guild_position_changed(int guild_id, int idx, const struct guild_position *p)
{
	struct guild *g=guild->search(guild_id);
	int i;
	nullpo_ret(p);
	Assert_ret(idx >= 0 && idx < MAX_GUILD);
	if(g==NULL)
		return 0;
	memcpy(&g->position[idx],p,sizeof(struct guild_position));
	clif->guild_positionchanged(g,idx);

	// Update char name in client [LuzZza]
	for(i=0;i<g->max_member;i++)
		if(g->member[i].position == idx && g->member[i].sd != NULL)
			clif->guild_position_selected(g->member[i].sd);
	return 0;
}

/*====================================================
 * Change guild notice
 *---------------------------------------------------*/
static int guild_change_notice(struct map_session_data *sd, int guild_id, const char *mes1, const char *mes2)
{
	nullpo_ret(sd);

	if(guild_id!=sd->status.guild_id)
		return 0;
	return intif->guild_notice(guild_id,mes1,mes2);
}

/*====================================================
 * Notification of guild has changed his notice
 *---------------------------------------------------*/
static int guild_notice_changed(int guild_id, const char *mes1, const char *mes2)
{
	int i;
	struct guild *g=guild->search(guild_id);
	nullpo_ret(mes1);
	nullpo_ret(mes2);
	if(g==NULL)
		return 0;

	memcpy(g->mes1,mes1,MAX_GUILDMES1);
	memcpy(g->mes2,mes2,MAX_GUILDMES2);

	for(i=0;i<g->max_member;i++){
		struct map_session_data *sd = g->member[i].sd;
		if (sd != NULL)
			clif->guild_notice(sd,g);
	}
	return 0;
}

/*====================================================
 * Change guild emblem
 *---------------------------------------------------*/
static int guild_change_emblem(struct map_session_data *sd, int len, const char *data)
{
	struct guild *g;
	nullpo_ret(sd);

	if (battle_config.require_glory_guild &&
		!((g = sd->guild) && guild->checkskill(g, GD_GLORYGUILD)>0)) {
		clif->skill_fail(sd, GD_GLORYGUILD, USESKILL_FAIL_LEVEL, 0, 0);
		return 0;
	}

	return intif->guild_emblem(sd->status.guild_id,len,data);
}

/*====================================================
 * Notification of guild emblem changed
 *---------------------------------------------------*/
static int guild_emblem_changed(int len, int guild_id, int emblem_id, const char *data)
{
	int i;
	struct map_session_data *sd;
	struct guild *g=guild->search(guild_id);
	nullpo_ret(data);
	if(g==NULL)
		return 0;

	memcpy(g->emblem_data,data,len);
	g->emblem_len=len;
	g->emblem_id=emblem_id;

	for(i=0;i<g->max_member;i++){
		if((sd=g->member[i].sd)!=NULL){
			sd->guild_emblem_id=emblem_id;
			clif->guild_belonginfo(sd,g);
			clif->guild_emblem(sd,g);
			clif->guild_emblem_area(&sd->bl);
		}
	}
	{// update guardians (mobs)
		struct DBIterator *iter = db_iterator(guild->castle_db);
		struct guild_castle* gc;
		for( gc = (struct guild_castle*)dbi_first(iter) ; dbi_exists(iter); gc = (struct guild_castle*)dbi_next(iter) )
		{
			if( gc->guild_id != guild_id )
				continue;
			// update permanent guardians
			for( i = 0; i < ARRAYLENGTH(gc->guardian); ++i ) {
				struct mob_data *md = gc->guardian[i].id ? map->id2md(gc->guardian[i].id) : NULL;
				if( md == NULL || md->guardian_data == NULL )
					continue;

				clif->guild_emblem_area(&md->bl);
			}
			// update temporary guardians
			for( i = 0; i < gc->temp_guardians_max; ++i ) {
				struct mob_data *md = gc->temp_guardians[i] ? map->id2md(gc->temp_guardians[i]) : NULL;
				if( md == NULL || md->guardian_data == NULL )
					continue;

				clif->guild_emblem_area(&md->bl);
			}
		}
		dbi_destroy(iter);
	}
	{// update npcs (flags or other npcs that used flagemblem to attach to this guild)
		for( i = 0; i < guild->flags_count; i++ ) {
			if( guild->flags[i] && guild->flags[i]->u.scr.guild_id == guild_id ) {
				clif->guild_emblem_area(&guild->flags[i]->bl);
			}
		}
	}
	return 0;
}

/**
 * @see DBCreateData
 */
static struct DBData create_expcache(union DBKey key, va_list args)
{
	struct guild_expcache *c;
	struct map_session_data *sd = va_arg(args, struct map_session_data*);

	c = ers_alloc(guild->expcache_ers, struct guild_expcache);
	nullpo_retr(DB->ptr2data(c), sd);
	c->guild_id = sd->status.guild_id;
	c->account_id = sd->status.account_id;
	c->char_id = sd->status.char_id;
	c->exp = 0;
	return DB->ptr2data(c);
}

/*====================================================
 * Return taxed experience from player sd to guild
 *---------------------------------------------------*/
static uint64 guild_payexp(struct map_session_data *sd, uint64 exp)
{
	struct guild *g;
	struct guild_expcache *c;
	int per;

	nullpo_ret(sd);

	if (!exp) return 0;

	if (sd->status.guild_id == 0 ||
		(g = sd->guild) == NULL ||
		(per = guild->getposition(g,sd)) < 0 ||
		(per = g->position[per].exp_mode) < 1)
		return 0;

	if (per < 100)
		exp = exp * per / 100;
	//Otherwise tax everything.
	c = DB->data2ptr(guild->expcache_db->ensure(guild->expcache_db, DB->i2key(sd->status.char_id), guild->create_expcache, sd));

	if (c->exp > UINT64_MAX - exp)
		c->exp = UINT64_MAX;
	else
		c->exp += exp;

	return exp;
}

/*====================================================
 * Player sd pay a tribute experience to his guild
 * Add this experience to guild exp
 * [Celest]
 *---------------------------------------------------*/
static int guild_getexp(struct map_session_data *sd, int exp)
{
	struct guild_expcache *c;
	nullpo_ret(sd);

	if (sd->status.guild_id == 0 || sd->guild == NULL)
		return 0;

	c = DB->data2ptr(guild->expcache_db->ensure(guild->expcache_db, DB->i2key(sd->status.char_id), guild->create_expcache, sd));
	if (c->exp > UINT64_MAX - exp)
		c->exp = UINT64_MAX;
	else
		c->exp += exp;
	return exp;
}

/*====================================================
 * Ask to increase guildskill skill_id
 *---------------------------------------------------*/
static int guild_skillup(struct map_session_data *sd, uint16 skill_id)
{
	struct guild* g;
	int idx = skill_id - GD_SKILLBASE;
	int max = guild->skill_get_max(skill_id);

	nullpo_ret(sd);

	if( idx < 0 || idx >= MAX_GUILDSKILL || // not a guild skill
			sd->status.guild_id == 0 || (g=sd->guild) == NULL || // no guild
			strcmp(sd->status.name, g->master) ) // not the guild master
		return 0;

	if( g->skill_point > 0 &&
			g->skill[idx].id != 0 &&
			g->skill[idx].lv < max )
		intif->guild_skillup(g->guild_id, skill_id, sd->status.account_id, max);

	return 0;
}

/*====================================================
 * Notification of guildskill skill_id increase request
 *---------------------------------------------------*/
static int guild_skillupack(int guild_id, uint16 skill_id, int account_id)
{
	struct map_session_data *sd=map->id2sd(account_id);
	struct guild *g=guild->search(guild_id);
	int i;
	if(g==NULL)
		return 0;
	Assert_ret(skill_id >= GD_SKILLBASE && skill_id - GD_SKILLBASE < MAX_GUILDSKILL);
	if( sd != NULL ) {
		clif->skillup(sd,skill_id,g->skill[skill_id-GD_SKILLBASE].lv, 0);

		/* Guild Aura handling */
		switch( skill_id ) {
			case GD_LEADERSHIP:
			case GD_GLORYWOUNDS:
			case GD_SOULCOLD:
			case GD_HAWKEYES:
				guild->aura_refresh(sd,skill_id,g->skill[skill_id-GD_SKILLBASE].lv);
				break;
		}
	}

	// Inform all members
	for(i=0;i<g->max_member;i++)
		if((sd=g->member[i].sd)!=NULL)
			clif->guild_skillinfo(sd);

	return 0;
}

static void guild_guildaura_refresh(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	struct skill_unit_group* group = NULL;
	int type = status->skill2sc(skill_id);
	nullpo_retv(sd);
	if( !(battle_config.guild_aura&((map->agit_flag || map->agit2_flag)?2:1))
	 && !(battle_config.guild_aura&(map_flag_gvg2(sd->bl.m)?8:4)) )
		return;
	if( !skill_lv )
		return;
	if (sd->sc.data[type] && (group = skill->id2group(sd->sc.data[type]->val4)) != NULL) {
		skill->del_unitgroup(group);
		status_change_end(&sd->bl,type,INVALID_TIMER);
	}
	group = skill->unitsetting(&sd->bl,skill_id,skill_lv,sd->bl.x,sd->bl.y,0);
	if( group ) {
		sc_start4(NULL,&sd->bl,type,100,(battle_config.guild_aura&16)?0:skill_lv,0,0,group->group_id,600000);//duration doesn't matter these status never end with val4
	}
	return;
}

/*====================================================
 * Count number of relations the guild has.
 * Flag:
 *   0 = allied
 *   1 = enemy
 *---------------------------------------------------*/
static int guild_get_alliance_count(struct guild *g, int flag)
{
	int i,c;

	nullpo_ret(g);

	for(i=c=0;i<MAX_GUILDALLIANCE;i++) {
		if(g->alliance[i].guild_id>0 && g->alliance[i].opposition==flag)
			c++;
	}
	return c;
}

// Blocks all guild skills which have a common delay time.
static void guild_block_skill(struct map_session_data *sd, int time)
{
	uint16 skill_id[] = { GD_BATTLEORDER, GD_REGENERATION, GD_RESTORE, GD_EMERGENCYCALL };
	int i;
	for (i = 0; i < 4; i++)
		skill->blockpc_start(sd, skill_id[i], time);
}

/*====================================================
 * Check relation between guild_id1 and guild_id2.
 * Flag:
 *   0 = allied
 *   1 = enemy
 * Returns true if yes.
 *---------------------------------------------------*/
static int guild_check_alliance(int guild_id1, int guild_id2, int flag)
{
	struct guild *g;
	int i;

	g = guild->search(guild_id1);
	if (g == NULL)
		return 0;

	ARR_FIND( 0, MAX_GUILDALLIANCE, i, g->alliance[i].guild_id == guild_id2 && g->alliance[i].opposition == flag );
	return( i < MAX_GUILDALLIANCE ) ? 1 : 0;
}

/*====================================================
 * Player sd, asking player tsd an alliance between their 2 guilds
 *---------------------------------------------------*/
static int guild_reqalliance(struct map_session_data *sd, struct map_session_data *tsd)
{
	struct guild *g[2];
	int i;

	if(map->agit_flag || map->agit2_flag) {
		// Disable alliance creation during woe [Valaris]
		clif->message(sd->fd,msg_sd(sd,876)); //"Alliances cannot be made during Guild Wars!"
		return 0;
	}

	nullpo_ret(sd);

	if(tsd==NULL || tsd->status.guild_id<=0)
		return 0;

	g[0]=sd->guild;
	g[1]=tsd->guild;

	if(g[0]==NULL || g[1]==NULL)
		return 0;

	// Prevent creation alliance with same guilds [LuzZza]
	if(sd->status.guild_id == tsd->status.guild_id)
		return 0;

	if( guild->get_alliance_count(g[0],0) >= battle_config.max_guild_alliance ) {
		clif->guild_allianceack(sd,4);
		return 0;
	}
	if( guild->get_alliance_count(g[1],0) >= battle_config.max_guild_alliance ) {
		clif->guild_allianceack(sd,3);
		return 0;
	}

	if( tsd->guild_alliance>0 ){
		clif->guild_allianceack(sd,1);
		return 0;
	}

	for (i = 0; i < MAX_GUILDALLIANCE; i++) { // check if already allied
		if(g[0]->alliance[i].guild_id==tsd->status.guild_id && g[0]->alliance[i].opposition==0) {
			clif->guild_allianceack(sd,0);
			return 0;
		}
	}

	tsd->guild_alliance=sd->status.guild_id;
	tsd->guild_alliance_account=sd->status.account_id;

	clif->guild_reqalliance(tsd,sd->status.account_id,g[0]->name);
	return 0;
}

/*====================================================
 * Player sd, answer to player tsd (account_id) for an alliance request
 *---------------------------------------------------*/
static int guild_reply_reqalliance(struct map_session_data *sd, int account_id, int flag)
{
	struct map_session_data *tsd;

	nullpo_ret(sd);
	tsd = map->id2sd( account_id );
	if (!tsd) { //Character left? Cancel alliance.
		clif->guild_allianceack(sd,3);
		return 0;
	}

	if (sd->guild_alliance != tsd->status.guild_id) // proposed guild_id alliance doesn't match tsd guildid
		return 0;

	if (flag == 1) { // consent
		int i;

		struct guild *g, *tg; // Reconfirm the number of alliance
		g=sd->guild;
		tg=tsd->guild;

		if(g==NULL || guild->get_alliance_count(g,0) >= battle_config.max_guild_alliance){
			clif->guild_allianceack(sd,4);
			clif->guild_allianceack(tsd,3);
			return 0;
		}
		if(tg==NULL || guild->get_alliance_count(tg,0) >= battle_config.max_guild_alliance){
			clif->guild_allianceack(sd,3);
			clif->guild_allianceack(tsd,4);
			return 0;
		}

		for(i=0;i<MAX_GUILDALLIANCE;i++){
			if(g->alliance[i].guild_id==tsd->status.guild_id &&
				g->alliance[i].opposition==1)
				intif->guild_alliance( sd->status.guild_id,tsd->status.guild_id,
					sd->status.account_id,tsd->status.account_id,9 );
		}
		for(i=0;i<MAX_GUILDALLIANCE;i++){
			if(tg->alliance[i].guild_id==sd->status.guild_id &&
				tg->alliance[i].opposition==1)
				intif->guild_alliance( tsd->status.guild_id,sd->status.guild_id,
					tsd->status.account_id,sd->status.account_id,9 );
		}

		// inform other servers
		intif->guild_alliance( sd->status.guild_id,tsd->status.guild_id,
			sd->status.account_id,tsd->status.account_id,0 );
		return 0;
	} else { // deny
		sd->guild_alliance=0;
		sd->guild_alliance_account=0;
		if(tsd!=NULL)
			clif->guild_allianceack(tsd,3);
	}
	return 0;
}

/*====================================================
 * Player sd asking to break alliance with guild guild_id
 *---------------------------------------------------*/
static int guild_delalliance(struct map_session_data *sd, int guild_id, int flag)
{
	nullpo_ret(sd);

	if(map->agit_flag || map->agit2_flag) {
		// Disable alliance breaking during woe [Valaris]
		clif->message(sd->fd,msg_sd(sd,877)); //"Alliances cannot be broken during Guild Wars!"
		return 0;
	}

	intif->guild_alliance( sd->status.guild_id,guild_id,sd->status.account_id,0,flag|8 );
	return 0;
}

/*====================================================
 * Player sd, asking player tsd a formal enemy relation between their 2 guilds
 *---------------------------------------------------*/
static int guild_opposition(struct map_session_data *sd, struct map_session_data *tsd)
{
	struct guild *g;
	int i;

	nullpo_ret(sd);

	g=sd->guild;
	if(g==NULL || tsd==NULL)
		return 0;

	// Prevent creation opposition with same guilds [LuzZza]
	if(sd->status.guild_id == tsd->status.guild_id)
		return 0;

	if( guild->get_alliance_count(g,1) >= battle_config.max_guild_alliance ) {
		clif->guild_oppositionack(sd,1);
		return 0;
	}

	for (i = 0; i < MAX_GUILDALLIANCE; i++) { // checking relations
		if(g->alliance[i].guild_id==tsd->status.guild_id){
			if (g->alliance[i].opposition == 1) { // check if not already hostile
				clif->guild_oppositionack(sd,2);
				return 0;
			}
			if(map->agit_flag || map->agit2_flag) // Prevent the changing of alliances to oppositions during WoE.
				return 0;
			//Change alliance to opposition.
			intif->guild_alliance(sd->status.guild_id,tsd->status.guild_id,
			                      sd->status.account_id,tsd->status.account_id,8);
		}
	}

	// inform other serv
	intif->guild_alliance(sd->status.guild_id,tsd->status.guild_id,
	                      sd->status.account_id,tsd->status.account_id,1);
	return 0;
}

/*====================================================
 * Notification of a relationship between 2 guilds
 *---------------------------------------------------*/
static int guild_allianceack(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2)
{
	struct guild *g[2] = { NULL };
	int guild_id[2] = { 0 };
	const char *guild_name[2] = { NULL };
	struct map_session_data *sd[2] = { NULL };
	int j,i;

	nullpo_ret(name1);
	nullpo_ret(name2);
	guild_id[0] = guild_id1;
	guild_id[1] = guild_id2;
	guild_name[0] = name1;
	guild_name[1] = name2;
	sd[0] = map->id2sd(account_id1);
	sd[1] = map->id2sd(account_id2);

	g[0]=guild->search(guild_id1);
	g[1]=guild->search(guild_id2);

	if(sd[0]!=NULL && (flag&0x0f)==0){
		sd[0]->guild_alliance=0;
		sd[0]->guild_alliance_account=0;
	}

	if (flag & 0x70) { // failure
		for(i=0;i<2-(flag&1);i++)
			if( sd[i]!=NULL )
				clif->guild_allianceack(sd[i],((flag>>4)==i+1)?3:4);
		return 0;
	}

	if (g[0] && g[1] && channel->config->ally && ( flag & 1 ) == 0) {
		if( !(flag & 0x08) ) {
			if (channel->config->ally_autojoin) {
				channel->guild_join_alliance(g[0],g[1]);
				channel->guild_join_alliance(g[1],g[0]);
			}
		} else {
			channel->guild_leave_alliance(g[0],g[1]);
			channel->guild_leave_alliance(g[1],g[0]);
		}
	}

	if (!(flag & 0x08)) { // new relationship
		for(i=0;i<2-(flag&1);i++) {
			if(g[i]!=NULL) {
				ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == 0 );
				if( j < MAX_GUILDALLIANCE ) {
					g[i]->alliance[j].guild_id=guild_id[1-i];
					memcpy(g[i]->alliance[j].name,guild_name[1-i],NAME_LENGTH);
					g[i]->alliance[j].opposition=flag&1;
				}
			}
		}
	} else { // remove relationship
		for(i=0;i<2-(flag&1);i++) {
			if( g[i] != NULL ) {
				ARR_FIND( 0, MAX_GUILDALLIANCE, j, g[i]->alliance[j].guild_id == guild_id[1-i] && g[i]->alliance[j].opposition == (flag&1) );
				if( j < MAX_GUILDALLIANCE )
					g[i]->alliance[j].guild_id = 0;
			}
			if (sd[i] != NULL) // notify players
				clif->guild_delalliance(sd[i],guild_id[1-i],(flag&1));
		}
	}

	if ((flag & 0x0f) == 0) { // alliance notification
		if( sd[1]!=NULL )
			clif->guild_allianceack(sd[1],2);
	} else if ((flag & 0x0f) == 1) { // enemy notification
		if( sd[0]!=NULL )
			clif->guild_oppositionack(sd[0],0);
	}

	for (i = 0; i < 2 - (flag & 1); i++) { // Retransmission of the relationship list to all members
		if (g[i] != NULL) {
			for (j = 0; j < g[i]->max_member; j++) {
				struct map_session_data *msd = g[i]->member[j].sd;
				if (msd != NULL)
					clif->guild_allianceinfo(msd);
			}
		}
	}
	return 0;
}

/**
 * Notification for the guild disbanded
 * @see DBApply
 */
static int guild_broken_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct guild *g = DB->data2ptr(data);
	int guild_id=va_arg(ap,int);
	int i,j;
	struct map_session_data *sd=NULL;

	nullpo_ret(g);

	for(i=0;i<MAX_GUILDALLIANCE;i++) {
		// Destroy all relationships
		if(g->alliance[i].guild_id==guild_id){
			for(j=0;j<g->max_member;j++)
				if( (sd=g->member[j].sd)!=NULL )
					clif->guild_delalliance(sd,guild_id,g->alliance[i].opposition);
			intif->guild_alliance(g->guild_id, guild_id,0,0,g->alliance[i].opposition|8);
			g->alliance[i].guild_id=0;
		}
	}
	return 0;
}

/**
 * Invoked on Castles when a guild is broken. [Skotlex]
 * @see DBApply
 */
static int castle_guild_broken_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct guild_castle *gc = DB->data2ptr(data);
	int guild_id = va_arg(ap, int);

	nullpo_ret(gc);

	if (gc->guild_id == guild_id) {
		char name[EVENT_NAME_LENGTH];
		// We call castle_event::OnGuildBreak of all castles of the guild
		// You can set all castle_events in the 'db/castle_db.txt'
		safestrncpy(name, gc->castle_event, sizeof(name));
		npc->event_do(strcat(name, "::OnGuildBreak"));

		//Save the new 'owner', this should invoke guardian clean up and other such things.
		guild->castledatasave(gc->castle_id, 1, 0);
	}
	return 0;
}

//Invoked on /breakguild "Guild name"
static int guild_broken(int guild_id, int flag)
{
	struct guild *g = guild->search(guild_id);
	struct map_session_data *sd = NULL;
	int i;

	if(flag!=0 || g==NULL)
		return 0;

	for(i=0;i<g->max_member;i++){
		// Destroy all relationships
		if((sd=g->member[i].sd)!=NULL){
			if(sd->state.storage_flag == STORAGE_FLAG_GUILD)
				gstorage->pc_quit(sd,1);
			sd->status.guild_id=0;
			sd->guild = NULL;
			sd->state.gmaster_flag = 0;
			clif->guild_broken(g->member[i].sd,0);
			clif->charnameupdate(sd); // [LuzZza]
			status_change_end(&sd->bl, SC_LEADERSHIP, INVALID_TIMER);
			status_change_end(&sd->bl, SC_GLORYWOUNDS, INVALID_TIMER);
			status_change_end(&sd->bl, SC_SOULCOLD, INVALID_TIMER);
			status_change_end(&sd->bl, SC_HAWKEYES, INVALID_TIMER);
		}
	}

	guild->db->foreach(guild->db,guild->broken_sub,guild_id);
	guild->castle_db->foreach(guild->castle_db,guild->castle_broken_sub,guild_id);
	gstorage->delete(guild_id);
	if (channel->config->ally) {
		if( g->channel != NULL ) {
			channel->delete(g->channel);
		}
	}
	if( g->instance )
		aFree(g->instance);

	HPM->data_store_destroy(&g->hdata);

	idb_remove(guild->db,guild_id);
	return 0;
}

//Changes the Guild Master to the specified player. [Skotlex]
static int guild_gm_change(int guild_id, int char_id)
{
	struct guild *g = guild->search(guild_id);
	char *name;
	int i;

	nullpo_ret(g);

	ARR_FIND(0, MAX_GUILD, i, g->member[i].char_id == char_id);

	if (i == MAX_GUILD ) {
		// Not part of the guild
		return 0;
	}

	name = g->member[i].name;

	if (strcmp(g->master, name) == 0) //Nothing to change.
		return 0;

	//Notify servers that master has changed.
	intif->guild_change_gm(guild_id, name, (int)strlen(name) + 1);
	return 1;
}

//Notification from Char server that a guild's master has changed. [Skotlex]
static int guild_gm_changed(int guild_id, int account_id, int char_id)
{
	struct guild *g;
	struct guild_member gm;
	int pos, i;

	g=guild->search(guild_id);

	if (!g)
		return 0;

	for(pos=0; pos<g->max_member && !(
		g->member[pos].account_id==account_id &&
		g->member[pos].char_id==char_id);
		pos++);

	if (pos == 0 || pos == g->max_member) return 0;

	memcpy(&gm, &g->member[pos], sizeof (struct guild_member));
	memcpy(&g->member[pos], &g->member[0], sizeof(struct guild_member));
	memcpy(&g->member[0], &gm, sizeof(struct guild_member));

	g->member[pos].position = g->member[0].position;
	g->member[0].position = 0; //Position 0: guild Master.
	strcpy(g->master, g->member[0].name);

	if (g->member[pos].sd && g->member[pos].sd->fd) {
		clif->message(g->member[pos].sd->fd, msg_sd(g->member[pos].sd,878)); //"You no longer are the Guild Master."
		g->member[pos].sd->state.gmaster_flag = 0;
		clif->blname_ack(0, &g->member[pos].sd->bl);
	}

	if (g->member[0].sd && g->member[0].sd->fd) {
		clif->message(g->member[0].sd->fd, msg_sd(g->member[0].sd,879)); //"You have become the Guild Master!"
		g->member[0].sd->state.gmaster_flag = 1;
		//Block his skills for 5 minutes to prevent abuse.
		guild->block_skill(g->member[0].sd, 300000);
		clif->blname_ack(0, &g->member[pos].sd->bl);
	}

	// announce the change to all guild members
	for( i = 0; i < g->max_member; i++ )
	{
		if( g->member[i].sd && g->member[i].sd->fd )
		{
			clif->guild_basicinfo(g->member[i].sd);
			clif->guild_memberlist(g->member[i].sd);
			clif->guild_belonginfo(g->member[i].sd, g); // Update clientside guildmaster flag
		}
	}

	return 1;
}

/*====================================================
 * Guild disbanded
 *---------------------------------------------------*/
static int guild_break(struct map_session_data *sd, const char *name)
{
	struct guild *g;
	struct unit_data *ud;
	int i;

	nullpo_ret(sd);
	nullpo_ret(name);

	if( (g=sd->guild)==NULL )
		return 0;
	if(strcmp(g->name,name)!=0)
		return 0;
	if(!sd->state.gmaster_flag)
		return 0;
	for (i = 0; i < g->max_member; i++) {
		if (g->member[i].account_id > 0
		 && (g->member[i].account_id!=sd->status.account_id
		  || g->member[i].char_id!=sd->status.char_id
		)) {
			break;
		}
	}
	if(i<g->max_member){
		clif->guild_broken(sd,2);
		return 0;
	}

	/* regardless of char server allowing it, we clear the guild master's auras */
	if( (ud = unit->bl2ud(&sd->bl)) ) {
		int count = 0;
		struct skill_unit_group *groups[4];
		for (i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++) {
			switch (ud->skillunit[i]->skill_id) {
				case GD_LEADERSHIP:
				case GD_GLORYWOUNDS:
				case GD_SOULCOLD:
				case GD_HAWKEYES:
					if( count == 4 )
						ShowWarning("guild_break:'%s' got more than 4 guild aura instances! (%d)\n",sd->status.name,ud->skillunit[i]->skill_id);
					else
						groups[count++] = ud->skillunit[i];
					break;
			}
		}
		for(i = 0; i < count; i++) { // FIXME: Why is this not done in the above loop?
			skill->del_unitgroup(groups[i]);
		}
	}

#ifdef GP_BOUND_ITEMS
	pc->bound_clear(sd,IBT_GUILD);
#endif

	intif->guild_break(g->guild_id);
	return 1;
}

/**
 * Creates a list of guild castle IDs to be requested
 * from char-server.
 */
static void guild_castle_map_init(void)
{
	int num = db_size(guild->castle_db);

	if (num > 0) {
		struct guild_castle* gc = NULL;
		int *castle_ids, *cursor;
		struct DBIterator *iter = NULL;

		CREATE(castle_ids, int, num);
		cursor = castle_ids;
		iter = db_iterator(guild->castle_db);
		for (gc = dbi_first(iter); dbi_exists(iter); gc = dbi_next(iter)) {
			*(cursor++) = gc->castle_id;
		}
		dbi_destroy(iter);
		if (intif->guild_castle_dataload(num, castle_ids))
			ShowStatus("Requested '"CL_WHITE"%d"CL_RESET"' guild castles from char-server...\n", num);
		aFree(castle_ids);
	}
}

static int guild_castle_owner_change_foreach(struct map_session_data *sd, va_list ap)
{
	int map_index = va_arg(ap, int);

	if (sd == NULL || sd->bl.m != map_index)
		return 0;

	status->calc_regen(&sd->bl, &sd->battle_status, &sd->regen);
	status->calc_regen_rate(&sd->bl, &sd->regen);
	return 1;
}

/**
 * Setter function for members of guild_castle struct.
 * Handles all side-effects, like updating guardians.
 * Sends updated info to char-server for saving.
 * @param castle_id Castle ID
 * @param index Type of data to change
 * @param value New value
 */
static int guild_castledatasave(int castle_id, int index, int value)
{
	struct guild_castle *gc = guild->castle_search(castle_id);

	if (gc == NULL) {
		ShowWarning("guild_castledatasave: guild castle '%d' not found\n", castle_id);
		return 0;
	}

	switch (index) {
	case 1: // The castle's owner has changed? Update or remove Guardians too. [Skotlex]
	{
		int i;
		gc->guild_id = value;
		for (i = 0; i < MAX_GUARDIANS; i++) {
			struct mob_data *gd;
			if (gc->guardian[i].visible && (gd = map->id2md(gc->guardian[i].id)) != NULL)
				mob->guardian_guildchange(gd);
		}

		if (gc->guild_id > 0)
			map->foreachpc(guild->castle_owner_change_foreach, gc->mapindex);
		break;
	}
	case 2:
		gc->economy = value; break;
	case 3: // defense invest change -> recalculate guardian hp
	{
		int i;
		gc->defense = value;
		for (i = 0; i < MAX_GUARDIANS; i++) {
			struct mob_data *gd;
			if (gc->guardian[i].visible && (gd = map->id2md(gc->guardian[i].id)) != NULL)
				status_calc_mob(gd, SCO_NONE);
		}
		break;
	}
	case 4:
		gc->triggerE = value; break;
	case 5:
		gc->triggerD = value; break;
	case 6:
		gc->nextTime = value; break;
	case 7:
		gc->payTime = value; break;
	case 8:
		gc->createTime = value; break;
	case 9:
		gc->visibleC = value; break;
	default:
		if (index > 9 && index <= 9+MAX_GUARDIANS) {
			gc->guardian[index-10].visible = value;
			break;
		}
		ShowWarning("guild_castledatasave: index = '%d' is out of allowed range\n", index);
		return 0;
	}

	if (!intif->guild_castle_datasave(castle_id, index, value)) {
		guild->castle_reconnect(castle_id, index, value);
	}
	return 0;
}

static void guild_castle_reconnect_sub(void *key, void *data, va_list ap)
{
	int castle_id = GetWord((int)h64BPTRSIZE(key), 0);
	int index = GetWord((int)h64BPTRSIZE(key), 1);
	intif->guild_castle_datasave(castle_id, index, *(int *)data);
	aFree(data);
}

/**
 * Saves pending guild castle data changes when char-server is
 * disconnected.
 * On reconnect pushes all changes to char-server for saving.
 */
static void guild_castle_reconnect(int castle_id, int index, int value)
{
	static struct linkdb_node *gc_save_pending = NULL;

	if (castle_id < 0) { // char-server reconnected
		linkdb_foreach(&gc_save_pending, guild->castle_reconnect_sub);
		linkdb_final(&gc_save_pending);
	} else {
		int *data;
		CREATE(data, int, 1);
		*data = value;
		linkdb_replace(&gc_save_pending, (void*)h64BPTRSIZE((MakeDWord(castle_id, index))), data);
	}
}

// Load castle data then invoke OnAgitInit* on last
static int guild_castledataloadack(int len, const struct guild_castle *gc)
{
	int i;
	int n = (len-4) / sizeof(struct guild_castle);
	int ev;

	nullpo_ret(gc);

	//Last owned castle in the list invokes ::OnAgitInit
	for( i = n-1; i >= 0 && !(gc[i].guild_id); --i );
	ev = i; // offset of castle or -1

	if( ev < 0 ) { //No castles owned, invoke OnAgitInit as it is.
		npc->event_doall("OnAgitInit");
		npc->event_doall("OnAgitInit2");
	} else { // load received castles into memory, one by one
		for( i = 0; i < n; i++, gc++ ) {
			struct guild_castle *c = guild->castle_search(gc->castle_id);
			if (!c) {
				ShowError("guild_castledataloadack: castle id=%d not found.\n", gc->castle_id);
				continue;
			}

			// update map-server castle data with new info
			memcpy(&c->guild_id, &gc->guild_id, sizeof(struct guild_castle) - offsetof(struct guild_castle, guild_id));

			if( c->guild_id ) {
				if( i != ev )
					guild->request_info(c->guild_id);
				else { // last owned one
					guild->npc_request_info(c->guild_id, "::OnAgitInit");
					guild->npc_request_info(c->guild_id, "::OnAgitInit2");
				}
			}
		}
	}
	ShowStatus("Received '"CL_WHITE"%d"CL_RESET"' guild castles from char-server.\n", n);
	return 0;
}

/*====================================================
 * Start normal woe and triggers all npc OnAgitStart
 *---------------------------------------------------*/
static void guild_agit_start(void)
{
	// Run All NPC_Event[OnAgitStart]
	int c = npc->event_doall("OnAgitStart");
	ShowStatus("NPC_Event:[OnAgitStart] Run (%d) Events by @AgitStart.\n",c);
}

/*====================================================
 * End normal woe and triggers all npc OnAgitEnd
 *---------------------------------------------------*/
static void guild_agit_end(void)
{
	// Run All NPC_Event[OnAgitEnd]
	int c = npc->event_doall("OnAgitEnd");
	ShowStatus("NPC_Event:[OnAgitEnd] Run (%d) Events by @AgitEnd.\n",c);
}

/*====================================================
 * Start woe2 and triggers all npc OnAgitStart2
 *---------------------------------------------------*/
static void guild_agit2_start(void)
{
	// Run All NPC_Event[OnAgitStart2]
	int c = npc->event_doall("OnAgitStart2");
	ShowStatus("NPC_Event:[OnAgitStart2] Run (%d) Events by @AgitStart2.\n",c);
}

/*====================================================
 * End woe2 and triggers all npc OnAgitEnd2
 *---------------------------------------------------*/
static void guild_agit2_end(void)
{
	// Run All NPC_Event[OnAgitEnd2]
	int c = npc->event_doall("OnAgitEnd2");
	ShowStatus("NPC_Event:[OnAgitEnd2] Run (%d) Events by @AgitEnd2.\n",c);
}

// How many castles does this guild have?
static int guild_checkcastles(struct guild *g)
{
	int nb_cas = 0;
	struct guild_castle* gc = NULL;
	struct DBIterator *iter = db_iterator(guild->castle_db);

	for (gc = dbi_first(iter); dbi_exists(iter); gc = dbi_next(iter)) {
		if (gc->guild_id == g->guild_id) {
			nb_cas++;
		}
	}
	dbi_destroy(iter);
	return nb_cas;
}

// Are these two guilds allied?
static bool guild_isallied(int guild_id, int guild_id2)
{
	int i;
	struct guild* g = guild->search(guild_id);
	nullpo_ret(g);

	ARR_FIND( 0, MAX_GUILDALLIANCE, i, g->alliance[i].guild_id == guild_id2 );
	return( i < MAX_GUILDALLIANCE && g->alliance[i].opposition == 0 );
}

static void guild_flag_add(struct npc_data *nd)
{
	int i;

	nullpo_retv(nd);
	/* check */
	for( i = 0; i < guild->flags_count; i++ ) {
		if( guild->flags[i] && guild->flags[i]->bl.id == nd->bl.id ) {
			return;/* exists, most likely updated the id. */
		}
	}

	i = guild->flags_count;/* save the current slot */
	/* add */
	RECREATE(guild->flags,struct npc_data*,++guild->flags_count);
	/* save */
	guild->flags[i] = nd;
}

static void guild_flag_remove(struct npc_data *nd)
{
	int i, cursor;
	nullpo_retv(nd);
	if( guild->flags_count == 0 )
		return;
	/* find it */
	for( i = 0; i < guild->flags_count; i++ ) {
		if( guild->flags[i] && guild->flags[i]->bl.id == nd->bl.id ) {/* found */
			guild->flags[i] = NULL;
			break;
		}
	}

	/* compact list */
	for( i = 0, cursor = 0; i < guild->flags_count; i++ ) {
		if( guild->flags[i] == NULL )
			continue;

		if( cursor != i ) {
			memmove(&guild->flags[cursor], &guild->flags[i], sizeof(guild->flags[0]));
		}
		cursor++;
	}

}

/**
 * @see DBApply
 */
static int eventlist_db_final(union DBKey key, struct DBData *data, va_list ap)
{
	struct eventlist *next = NULL;
	struct eventlist *current = DB->data2ptr(data);
	while (current != NULL) {
		next = current->next;
		aFree(current);
		current = next;
	}
	return 0;
}

/**
 * @see DBApply
 */
static int guild_expcache_db_final(union DBKey key, struct DBData *data, va_list ap)
{
	ers_free(guild->expcache_ers, DB->data2ptr(data));
	return 0;
}

/**
 * @see DBApply
 */
static int guild_castle_db_final(union DBKey key, struct DBData *data, va_list ap)
{
	struct guild_castle* gc = DB->data2ptr(data);
	if( gc->temp_guardians )
		aFree(gc->temp_guardians);
	aFree(gc);
	return 0;
}

/* called when scripts are reloaded/unloaded */
static void guild_flags_clear(void)
{
	int i;
	for( i = 0; i < guild->flags_count; i++ ) {
		if( guild->flags[i] )
			guild->flags[i] = NULL;
	}

	guild->flags_count = 0;
}

static void do_init_guild(bool minimal)
{
	if (minimal)
		return;

	guild->db           = idb_alloc(DB_OPT_RELEASE_DATA);
	guild->castle_db    = idb_alloc(DB_OPT_BASE);
	guild->expcache_db  = idb_alloc(DB_OPT_BASE);
	guild->infoevent_db = idb_alloc(DB_OPT_BASE);
	guild->expcache_ers = ers_new(sizeof(struct guild_expcache),"guild.c::expcache_ers",ERS_OPT_NONE);

	guild->read_castledb_libconfig();
	sv->readdb(map->db_path, "guild_skill_tree.txt", ',', 2+MAX_GUILD_SKILL_REQUIRE*2, 2+MAX_GUILD_SKILL_REQUIRE*2, -1, guild->read_guildskill_tree_db); //guild skill tree [Komurka]

	timer->add_func_list(guild->payexp_timer,"guild_payexp_timer");
	timer->add_func_list(guild->send_xy_timer, "guild_send_xy_timer");
	timer->add_interval(timer->gettick()+GUILD_PAYEXP_INVERVAL,guild->payexp_timer,0,0,GUILD_PAYEXP_INVERVAL);
	timer->add_interval(timer->gettick()+GUILD_SEND_XY_INVERVAL,guild->send_xy_timer,0,0,GUILD_SEND_XY_INVERVAL);
}

static void do_final_guild(void)
{
	struct DBIterator *iter = db_iterator(guild->db);
	struct guild *g;

	for( g = dbi_first(iter); dbi_exists(iter); g = dbi_next(iter) ) {
		if( g->channel != NULL )
			channel->delete(g->channel);
		if( g->instance != NULL ) {
			aFree(g->instance);
			g->instance = NULL;
		}
		HPM->data_store_destroy(&g->hdata);
	}

	dbi_destroy(iter);

	db_destroy(guild->db);
	guild->castle_db->destroy(guild->castle_db,guild->castle_db_final);
	guild->expcache_db->destroy(guild->expcache_db,guild->expcache_db_final);
	guild->infoevent_db->destroy(guild->infoevent_db,guild->eventlist_db_final);
	ers_destroy(guild->expcache_ers);

	if( guild->flags )
		aFree(guild->flags);
}

void guild_defaults(void)
{
	guild = &guild_s;

	guild->init = do_init_guild;
	guild->final = do_final_guild;
	/* */
	guild->db = NULL;
	guild->castle_db = NULL;
	guild->expcache_db = NULL;
	guild->infoevent_db = NULL;
	/* */
	guild->expcache_ers = NULL;
	/* */
	memset(guild->skill_tree, 0, sizeof(guild->skill_tree));
	/* guild flags cache */
	guild->flags = NULL;
	guild->flags_count = 0;
	/* */
	guild->skill_get_max = guild_skill_get_max;
	/* */
	guild->checkskill = guild_checkskill;
	guild->check_skill_require = guild_check_skill_require;
	guild->checkcastles = guild_checkcastles;
	guild->isallied = guild_isallied;
	/* */
	guild->search = guild_search;
	guild->searchname = guild_searchname;
	guild->castle_search = guild_castle_search;
	/* */
	guild->mapname2gc = guild_mapname2gc;
	guild->mapindex2gc = guild_mapindex2gc;
	/* */
	guild->getavailablesd = guild_getavailablesd;
	guild->getindex = guild_getindex;
	guild->getposition = guild_getposition;
	guild->payexp = guild_payexp;
	guild->getexp = guild_getexp;
	/* */
	guild->create = guild_create;
	guild->created = guild_created;
	guild->request_info = guild_request_info;
	guild->recv_noinfo = guild_recv_noinfo;
	guild->recv_info = guild_recv_info;
	guild->npc_request_info = guild_npc_request_info;
	guild->invite = guild_invite;
	guild->reply_invite = guild_reply_invite;
	guild->member_joined = guild_member_joined;
	guild->member_added = guild_member_added;
	guild->leave = guild_leave;
	guild->member_withdraw = guild_member_withdraw;
	guild->expulsion = guild_expulsion;
	guild->skillup = guild_skillup;
	guild->block_skill = guild_block_skill;
	guild->reqalliance = guild_reqalliance;
	guild->reply_reqalliance = guild_reply_reqalliance;
	guild->allianceack = guild_allianceack;
	guild->delalliance = guild_delalliance;
	guild->opposition = guild_opposition;
	guild->check_alliance = guild_check_alliance;
	/* */
	guild->send_memberinfoshort = guild_send_memberinfoshort;
	guild->recv_memberinfoshort = guild_recv_memberinfoshort;
	guild->change_memberposition = guild_change_memberposition;
	guild->memberposition_changed = guild_memberposition_changed;
	guild->change_position = guild_change_position;
	guild->position_changed = guild_position_changed;
	guild->change_notice = guild_change_notice;
	guild->notice_changed = guild_notice_changed;
	guild->change_emblem = guild_change_emblem;
	guild->emblem_changed = guild_emblem_changed;
	guild->send_message = guild_send_message;
	guild->send_dot_remove = guild_send_dot_remove;
	guild->skillupack = guild_skillupack;
	guild->dobreak = guild_break;
	guild->broken = guild_broken;
	guild->gm_change = guild_gm_change;
	guild->gm_changed = guild_gm_changed;
	/* */
	guild->castle_map_init = guild_castle_map_init;
	guild->castledatasave = guild_castledatasave;
	guild->castledataloadack = guild_castledataloadack;
	guild->castle_reconnect = guild_castle_reconnect;
	/* */
	guild->agit_start = guild_agit_start;
	guild->agit_end = guild_agit_end;
	guild->agit2_start = guild_agit2_start;
	guild->agit2_end = guild_agit2_end;
	/* guild flag caching */
	guild->flag_add = guild_flag_add;
	guild->flag_remove = guild_flag_remove;
	guild->flags_clear = guild_flags_clear;
	/* guild aura */
	guild->aura_refresh = guild_guildaura_refresh;
	/* */
	guild->payexp_timer = guild_payexp_timer;
	guild->sd_check = guild_sd_check;
	guild->read_guildskill_tree_db = guild_read_guildskill_tree_db;
	guild->read_castledb_libconfig = guild_read_castledb_libconfig;
	guild->read_castledb_libconfig_sub = guild_read_castledb_libconfig_sub;
	guild->read_castledb_libconfig_sub_warp = guild_read_castledb_libconfig_sub_warp;
	guild->payexp_timer_sub = guild_payexp_timer_sub;
	guild->send_xy_timer_sub = guild_send_xy_timer_sub;
	guild->send_xy_timer = guild_send_xy_timer;
	guild->create_expcache = create_expcache;
	guild->eventlist_db_final = eventlist_db_final;
	guild->expcache_db_final = guild_expcache_db_final;
	guild->castle_db_final = guild_castle_db_final;
	guild->broken_sub = guild_broken_sub;
	guild->castle_broken_sub = castle_guild_broken_sub;
	guild->makemember = guild_makemember;
	guild->check_member = guild_check_member;
	guild->get_alliance_count = guild_get_alliance_count;
	guild->castle_reconnect_sub = guild_castle_reconnect_sub;
	guild->castle_owner_change_foreach = guild_castle_owner_change_foreach;
	/* */
	guild->retrieveitembound = guild_retrieveitembound;
}
