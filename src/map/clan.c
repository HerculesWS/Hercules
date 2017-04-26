/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017  Hercules Dev Team
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

#include "config/core.h"
#include "clan.h"

#include "map/clif.h"
#include "map/homunculus.h"
#include "map/log.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/status.h"
#include "common/HPM.h"
#include "common/conf.h"
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mapindex.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"
#include "common/sql.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct clan_interface clan_s;
struct clan_interface *clan;

/**
 * Searches a Clan by clan_id
 *
 * @param clan_id Clan ID
 * @return struct clan*
 */
struct clan *clan_search(int clan_id)
{
	if (clan_id <= 0) {
		return NULL;
	}
	return (struct clan *)idb_get(clan->db, clan_id);
}

/**
 * Searches a Clan by clan_name
 *
 * @param name Clan Name
 * @return struct clan*
 */
struct clan *clan_searchname(const char *name)
{
	nullpo_retr(NULL, name);
	struct clan *c;
	struct DBIterator *iter = db_iterator(clan->db);
	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		if (strncmpi(c->name, name, NAME_LENGTH) == 0) {
			break;
		}
	}
	dbi_destroy(iter);
	return c;
}

/**
 * Returns the first online character of clan
 * @method clan_getonlinesd
 * @param  c                clan structure
 * @return                  map_session_data
 */
struct map_session_data *clan_getonlinesd(struct clan *c)
{
	int i;
	nullpo_retr(NULL, c);

	ARR_FIND(0, c->max_member, i, c->member[i].sd != NULL);
	return (i < c->max_member) ? c->member[i].sd : NULL;
}

/**
 * Returns the member index of given Player
 *
 * @param c Clan Data
 * @param account_id Player's Account ID
 * @param char_id Player's Char ID
 * @return int
 */
int clan_getindex(const struct clan *c, int account_id, int char_id)
{
	int i;
	nullpo_retr(INDEX_NOT_FOUND, c);

	ARR_FIND(0, c->max_member, i, c->member[i].account_id == account_id && c->member[i].char_id == char_id);

	if (i == c->max_member) {
		return INDEX_NOT_FOUND;
	}
	return i;
}

/**
 * Starts clan buff
 */
void clan_buff_start(struct map_session_data *sd, struct clan *c)
{
	nullpo_retv(sd);
	nullpo_retv(c);
	if (c->buff.icon <= 0) {
		return;
	}
	clif->sc_load(&sd->bl, sd->bl.id, SELF, c->buff.icon, 0, c->clan_id, 0);
	script->run(c->buff.script, 0, sd->bl.id, npc->fake_nd->bl.id);
}

/**
 * Ends clan buff
 */
void clan_buff_end(struct map_session_data *sd, struct clan *c)
{
	nullpo_retv(sd);
	nullpo_retv(c);
	if (c->buff.icon <= 0) {
		return;
	}
	clif->sc_end(&sd->bl, sd->bl.id, SELF, c->buff.icon);
}

/**
 * Joins a Player into a Clan
 *
 * @param sd Player's Map Session Data
 * @param clan_id Clan which will add this player
 * @return bool
 */
bool clan_join(struct map_session_data *sd, int clan_id)
{
	int i;
	struct clan *c;

	nullpo_ret(sd);
	
	// Already joined a guild or clan
	if (sd->status.guild_id || sd->status.clan_id) {
		return false;
	}

	c = clan->search(clan_id);
	if (c == NULL) {
		return false;
	}

	if (clan->getindex(c, sd->status.account_id, sd->status.char_id) != INDEX_NOT_FOUND) {
		return false;
	}

	ARR_FIND(0, c->max_member, i, c->member[i].account_id == 0);
	if (i == c->max_member) {
		return false;
	}
 	
	c->member[i].sd = sd;
	c->member[i].account_id = sd->status.account_id;
	c->member[i].char_id = sd->status.char_id;
	c->member[i].online = 1;
	c->member[i].last_login = sd->status.last_login;

	sd->status.clan_id = c->clan_id;
	sd->clan = c;

	sc_start2(NULL, &sd->bl, SC_CLAN_INFO, 10000, 0, c->clan_id, INFINITE_DURATION);
	status_calc_pc(sd, SCO_FORCE);

	clif->clan_basicinfo(sd);
	clif->clan_onlinecount(c);
	return true;
}

/**
 * Invoked when a player joins
 *
 * @param sd Player Data
 */
void clan_member_online(struct map_session_data *sd, bool first)
{
	struct clan *c;
	int i;
	nullpo_retv(sd);
	c = clan->search(sd->status.clan_id);
	if (c == NULL) {
		return;
	}

	i = clan->getindex(c, sd->status.account_id, sd->status.char_id);
	if (i == INDEX_NOT_FOUND) {
		ARR_FIND(0, c->max_member, i, c->member[i].sd == NULL);
		c->member[i].sd = sd;
		c->member[i].account_id = sd->status.account_id;
		c->member[i].char_id = sd->status.account_id;
	}

	c->member[i].online = 1;
	// Last Login has not changed yet.
	c->member[i].last_login = sd->status.last_login;

	if (first && c->kick_time > 0) {
		int inactivity = (int)(time(NULL) - sd->status.last_login);
		if (inactivity > c->kick_time) {
			clan->leave(sd, first);
			return;
		}
	}

	sc_start2(NULL, &sd->bl, SC_CLAN_INFO, 10000, 0, c->clan_id, INFINITE_DURATION);

	if (!first) { // When first called from pc.c we don't need to do status_calc
		status_calc_pc(sd, SCO_FORCE);
	}

	clif->clan_basicinfo(sd);
	clif->clan_onlinecount(c);
}

/**
 * Re-join a player on its clan
 */
int clan_rejoin(struct map_session_data *sd, va_list ap)
{
	nullpo_ret(sd);
	if (sd->status.clan_id) {
		clan->member_online(sd, false);
	}
	return 0;
}

/**
 * Removes Player from clan
 */
bool clan_leave(struct map_session_data *sd, bool first)
{
	int i;
	struct clan *c;

	nullpo_ret(sd);

	c = sd->clan;
	if (c == NULL) {
		return false;
	}
	i = clan->getindex(c, sd->status.account_id, sd->status.char_id);
	if (i != INDEX_NOT_FOUND) {
		c->member[i].online = 0;
		c->member[i].sd = NULL;
		c->member[i].account_id = 0;
		c->member[i].char_id = 0;
	}

	sd->status.clan_id = 0;
	sd->clan = NULL;
	clan->buff_end(sd, c);

	status_change_end(&sd->bl, SC_CLAN_INFO, INVALID_TIMER);
	if (!first) {
		status_calc_pc(sd, SCO_FORCE);
	}

	clan->removemember_tosql(sd->status.char_id);
	clif->clan_onlinecount(c);
	clif->clan_leave(sd);
	return true;
}

/**
 * Sets a player offline
 */
void clan_member_offline(struct map_session_data *sd)
{
	struct clan *c;
	int i;
	nullpo_retv(sd);
	c = sd->clan;
	if (c == NULL) {
		return;
	}
	i = clan->getindex(c, sd->status.account_id, sd->status.char_id);
	if (i != INDEX_NOT_FOUND) {
		c->member[i].online = 0;
		clif->clan_onlinecount(c);
	}
}

/**
 * Removes Clan of player on database
 * @param char_id Player Character ID
 */
void clan_removemember_tosql(int char_id)
{
	if (char_id <= 0)
		return;
	if (SQL_ERROR == SQL->Query(map->mysql_handle, "UPDATE `char` SET `clan_id` = '0' WHERE `char_id` = '%d'", char_id))
		Sql_ShowDebug(map->mysql_handle);
}

/**
 * Returns the number of Alliances of a Clan
 * @param c Clan Data
 * @return int
 */
int clan_ally_count(struct clan *c)
{
	int i, count;

	nullpo_ret(c);

	for (i = 0, count = 0; i < clan->maxalliances; i++) {
		if (c->allies[i].clan_id > 0) {
			count++;
		}
	}

	return count;
}

/**
 * Returns the number of Alliances of a Clan
 * @param c Clan Data
 * @return int
 */
int clan_antagonist_count(struct clan *c)
{
	int i, count;
	nullpo_ret(c);
	for (i = 0, count = 0; i < clan->maxalliances; i++) {
		if (c->antagonists[i].clan_id > 0) {
			count++;
		}
	}
	return count;
}

/**
 * Returns the count of online players of given clan
 * @param c The given clan
 * @return int
 */
int clan_getonlinecount(struct clan *c)
{
	int i, count = 0;
	nullpo_ret(c);
	for (i = 0; i < c->max_member; i++) {
		if (c->member[i].online == 1) {
			count++;
		}
	}
	c->connect_member = count;
	return count;
}

/**
 * Sends a message to whole clan
 */
bool clan_send_message(struct map_session_data *sd, const char *mes)
{
	int len;
	nullpo_retr(false, sd);
	nullpo_retr(false, mes);

	len = (int)strlen(mes);

	if (sd->status.clan_id == 0) {
		return false;
	}
	clan->recv_message(sd->clan, mes, len);

	// Chat logging type 'C' / Clan Chat
	logs->chat(LOG_CHAT_CLAN, sd->status.clan_id, sd->status.char_id, sd->status.account_id, mapindex_id2name(sd->mapindex), sd->bl.x, sd->bl.y, NULL, mes);

	return false;
}

/**
 * Clan receive a message, will be displayed to whole clan
 */
bool clan_recv_message(struct clan *c, const char *mes, int len)
{
	clif->clan_message(c, mes, len);
	return false;
}

/**
 * Set constants for each clan
 */
void clan_constants(void)
{
	struct DBIterator *iter = db_iterator(clan->db);
	struct clan *c;

	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		script->set_constant2(c->constant, c->clan_id, false, false);
	}

	dbi_destroy(iter);

	clan->fix_relationships();
}

/**
 * Fix clan relationships (allies and antagonists)
 */
void clan_fix_relationships(void)
{
	struct DBIterator *iter = db_iterator(clan->db);
	struct clan *c;
	int i, clan_id;

	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		for (i = 0; i < ARRAYLENGTH(c->allies); i++) {
			if (script->get_constant(c->allies[i].constant, &clan_id)) {
				c->allies[i].clan_id = clan_id;
			} else {
				memset(&c->allies[i].constant, 0, NAME_LENGTH);
				c->allies[i].clan_id = 0;
			}
		}
		for (i = 0; i < ARRAYLENGTH(c->antagonists); i++) {
			if (script->get_constant(c->antagonists[i].constant, &clan_id)) {
				c->antagonists[i].clan_id = clan_id;
			} else {
				memset(&c->antagonists[i].constant, 0, NAME_LENGTH);
				c->antagonists[i].clan_id = 0;
			}
		}
	}
	dbi_destroy(iter);
}

/**
 * Returns the clan_id of bl
 */
int clan_get_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type ) {
		case BL_PC: {
			const struct map_session_data *sd = BL_UCCAST(BL_PC, bl);
			return sd->status.clan_id;
		}
		case BL_PET: {
			const struct pet_data *pd = BL_UCCAST(BL_PET, bl);
			if (pd->msd != NULL)
				return pd->msd->status.clan_id;
		}
			break;
		case BL_MOB: {
			const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
			const struct map_session_data *msd;
			if (md->special_state.ai != AI_NONE && (msd = map->id2sd(md->master_id)) != NULL) {
				return msd->status.clan_id;
			}
			return md->clan_id;
		}
		case BL_HOM: {
			const struct homun_data *hd = BL_UCCAST(BL_HOM, bl);
			if (hd->master != NULL) {
				return hd->master->status.clan_id;
			}
		}
			break;
		case BL_MER: {
			const struct mercenary_data *md = BL_UCCAST(BL_MER, bl);
			if (md->master != NULL) {
				return md->master->status.clan_id;
			}
		}
			break;
		case BL_SKILL: {
			const struct skill_unit *su = BL_UCCAST(BL_SKILL, bl);
			if (su->group != NULL)
				return su->group->clan_id;
		}
			break;
	}

	return 0;
}

/**
 * Checks every clan player and remove those who are inactive
 */
int clan_inactivity_kick(int tid, int64 tick, int id, intptr_t data)
{
	struct clan *c = NULL;
	int i;

	if ((c = clan->search(id)) != NULL) {
		if (!c->kick_time || c->tid != tid || tid == INVALID_TIMER || c->tid == INVALID_TIMER) {
		  ShowError("Timer Mismatch (Time: %d seconds) %d != %d", c->kick_time, c->tid, tid);
		  return 0;
		}
		for (i = 0; i < ARRAYLENGTH(c->member); i++) {
			if (c->member[i].char_id <= 0 || c->member[i].online <= 0)
				continue;

			if (c->member[i].online) {
				struct map_session_data *sd = c->member[i].sd;
				if (DIFF_TICK32(sockt->last_tick, sd->idletime) > c->kick_time) {
					clan->leave(sd, false);
				}
			} else if ((time(NULL) - c->member[i].last_login) > c->kick_time) {
				c->member[i].online = 0;
				c->member[i].sd = NULL;
				c->member[i].account_id = 0;
				c->member[i].char_id = 0;
				c->member[i].last_login = 0;

				clif->clan_onlinecount(c);
			}

		}
		c->tid = timer->add(timer->gettick() + c->check_time, clan->inactivity_kick, c->clan_id, 0);
	}
	return 1;
}

/**
 * Processes any (plugin-defined) additional fields for a clan entry.
 *
 * @param[in,out] entry  The destination clan entry, already initialized
 *                       (clan_id is expected to be already set).
 * @param[in]     t      The libconfig entry.
 * @param[in]     n      Ordinal number of the entry, to be displayed in case
 *                       of validation errors.
 * @param[in]     source Source of the entry (file name), to be displayed in
 *                       case of validation errors.
 */
void clan_read_db_additional_fields(struct clan *entry, struct config_setting_t *t, int n, const char *source)
{
	// do nothing. plugins can do own work
}

/**
 * Processes one mobdb entry from the libconfig file, loading and inserting it
 * into the mob database.
 *
 * @param mobt   Libconfig setting entry. It is expected to be valid and it
 *               won't be freed (it is care of the caller to do so if
 *               necessary).
 * @param n      Ordinal number of the entry, to be displayed in case of
 *               validation errors.
 * @param source Source of the entry (file name), to be displayed in case of
 *               validation errors.
 * @return bool.
 */
bool clan_read_db_sub(struct config_setting_t *clans, int n, const char *source)
{
	struct config_setting_t *clan_;
	struct config_setting_t *allies;
	struct config_setting_t *antagonists;
	struct config_setting_t *buff;
	struct clan *c;
	const char *aName, *aLeader, *aMap, *aConst;
	int kicktime = 0, kickchecktime = 0, id;

	nullpo_retr(false, clans);

	clan_ = libconfig->setting_get_elem(clans, n);
	allies = libconfig->setting_get_member(clan_, "Allies");
	antagonists = libconfig->setting_get_member(clan_, "Antagonists");

	if (!libconfig->setting_lookup_int(clan_, "Id", &id)) {
		ShowError("clan_config_read: failed to find 'Id' for clan #%d\n", n);
		return false;
	}

	if (!libconfig->setting_lookup_string(clan_, "Const", &aConst)) {
		ShowError("clan_config_read: failed to find 'Const' for clan #%d\n", n);
		return false;
	}

	if (!libconfig->setting_lookup_string(clan_, "Name", &aName)) {
		ShowError("clan_config_read: failed to find 'Name' for clan #%d\n", n);
		return false;
	}

	if (!libconfig->setting_lookup_string(clan_, "Leader", &aLeader)) {
		ShowError("clan_config_read: failed to find 'Leader' for clan #%d\n", n);
		return false;
	}

	if (!libconfig->setting_lookup_string(clan_, "Map", &aMap)) {
		ShowError("clan_config_read: failed to find 'Map' for clan #%d\n", n);
		return false;
	}

	CREATE(c, struct clan, 1);

	c->clan_id = id;
	safestrncpy(c->constant, aConst, NAME_LENGTH);
	safestrncpy(c->name, aName, NAME_LENGTH);
	safestrncpy(c->master, aLeader, NAME_LENGTH);
	safestrncpy(c->map, aMap, MAP_NAME_LENGTH_EXT);
	c->max_member = clan->max;
	if (c->max_member > MAX_CLAN) {
		ShowWarning("Clan %d:%s specifies higher capacity (%d) than MAX_CLAN (%d)\n", c->clan_id, c->name, c->max_member, MAX_CLAN);
		c->max_member = MAX_CLAN;
	}
	c->connect_member = 0;

	if (libconfig->setting_lookup_int(clan_, "KickTime", &kicktime)) {
		c->kick_time = 60 * 60 * kicktime;
	} else {
		c->kick_time = clan->kicktime;
	}
	if (libconfig->setting_lookup_int(clan_, "CheckTime", &kickchecktime)) {
		c->check_time = 60 * 60 * max(1, kickchecktime) * 1000;
	} else {
		c->check_time = clan->checktime;
	}


	if ((buff = libconfig->setting_get_member(clan_, "Buff")) != NULL) {
		struct clan_buff *b = &c->buff;
		const char *str = NULL;

		if (!libconfig->setting_lookup_int(buff, "Icon", &b->icon)) {
			const char *temp_str = NULL;
			if (libconfig->setting_lookup_string(buff, "Icon", &temp_str)) {
				if (*temp_str) {
					if (!script->get_constant(temp_str, &b->icon)) {
						ShowWarning("Clan %d: Invalid Icon Value. Defaulting to SI_BLANK.\n", c->clan_id);
						b->icon = -1;
					}
				}
			}
		}

		if (libconfig->setting_lookup_string(buff, "Script", &str)) {
			b->script = *str ? script->parse(str, source, -b->icon, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;
		}
	}

	if (allies != NULL) {
		int a = 0;
		const char *ally_const;

		for (a = 0; a < libconfig->setting_length(allies) &&  a < clan->maxalliances; a++) {
			if ((ally_const = libconfig->setting_get_string_elem(allies, a)) != NULL)
				safestrncpy(c->allies[a].constant, ally_const, NAME_LENGTH);
		}
	}
	if (antagonists != NULL) {
		int o = 0;
		const char *antg_const;

		for (o = 0; o < libconfig->setting_length(antagonists) && o < clan->maxalliances; o++) {
			if ((antg_const = libconfig->setting_get_string_elem(antagonists, o)) != NULL)
				safestrncpy(c->antagonists[o].constant, antg_const, NAME_LENGTH);
		}
	}
	clan->read_db_additional_fields(c, clan_, n, source);
	idb_put(clan->db, c->clan_id, c);
	c->tid = timer->add(timer->gettick() + c->kick_time, clan->inactivity_kick, c->clan_id, 0);

	return true;
}

/**
 * Reads Clan DB included by clan configuration file.
 *
 * @param settings The Settings Group from config file.
 * @param source File name.
 */
void clan_read_db(struct config_setting_t *settings, const char *source)
{
	struct config_setting_t *clans;

	nullpo_retv(settings);

	if ((clans = libconfig->setting_get_member(settings, "clans")) != NULL ) {
		int i;
		int clan_count = libconfig->setting_length(clans);
		for (i = 0; i < clan_count; i++) {
			clan->read_db_sub(clans, i, source);
		}
		ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", clan_count, source);
		clan->constants();
	}
}

/**
 * Reads clan config file
 *
 * @param bool clear Whether to clear clan->db before reading clans
 */
bool clan_config_read(bool clear)
{
	struct config_t clan_conf;
	struct config_setting_t *data = NULL;
	const char *config_filename = "conf/clans.conf"; // FIXME: hardcoded name

	if (clear) {
		struct DBIterator *iter = db_iterator(clan->db);
		struct clan *c_clear;

		for (c_clear = dbi_first(iter); dbi_exists(iter); c_clear = dbi_next(iter)) {
			if (c_clear->buff.script) {
				script->free_code(c_clear->buff.script);
			}
			if (c_clear->tid != INVALID_TIMER) {
				timer->delete(c_clear->tid, clan->inactivity_kick);
			}
		}
		dbi_destroy(iter);
		clan->db->clear(clan->db, NULL);
	}

	if (!libconfig->load_file(&clan_conf, config_filename)) {
		return false;
	}

	data = libconfig->lookup(&clan_conf, "clan_configuration");

	if (data != NULL) {
		struct config_setting_t *settings = libconfig->setting_get_elem(data, 0);
		int kicktime = 0, kickchecktime = 0;

		libconfig->setting_lookup_int(settings, "MaxMember", &clan->max);
		libconfig->setting_lookup_int(settings, "MaxAlliances", &clan->maxalliances);
		libconfig->setting_lookup_int(settings, "InactivityKickTime", &kicktime);
		if (!libconfig->setting_lookup_int(settings, "InactivityCheckTime", &kickchecktime)) {
			ShowError("clan_config_read: failed to find InactivityCheckTime using official value.\n");
			kickchecktime = 24;
		}

		// On config file we set the time in hours but here we use in seconds
		clan->kicktime = 60 * 60 * kicktime;
		clan->checktime = 60 * 60 * max(kickchecktime, 1) * 1000;

		clan->config_read_additional_settings(settings, config_filename);
		clan->read_db(settings, config_filename);
	}
	libconfig->destroy(&clan_conf);
	return true;
}

/**
 * Processes any (plugin-defined) additional settings for clan config.
 *
 * @param settings The Settings Group from config file.
 * @param source   Source of the entry (file name), to be displayed in
 *                       case of validation errors.
 */
void clan_config_read_additional_settings(struct config_setting_t *settings, const char *source)
{
	// do nothing. plugins can do own work
}

/**
 * Reloads Clan DB
 */
void clan_reload(void)
{
	if (clan->config_read(true)) {
		map->foreachpc(clan->rejoin); // Re-joins every player on their clans);
	}
}

/**
 *
 */
void do_init_clan(bool minimal)
{
	if (minimal) {
		return;
	}
	clan->db = idb_alloc(DB_OPT_RELEASE_DATA);
	clan->config_read(false);
	timer->add_func_list(clan->inactivity_kick, "clan_inactivity_kick");
}

/**
 *
 */
void do_final_clan(void)
{
	struct DBIterator *iter = db_iterator(clan->db);
	struct clan *c;

	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		if (c->buff.script) {
			script->free_code(c->buff.script);
		}
		if (c->tid != INVALID_TIMER) {
			timer->delete(c->tid, clan->inactivity_kick);
		}
		HPM->data_store_destroy(&c->hdata);
	}
	dbi_destroy(iter);
	db_destroy(clan->db);
}

/**
 * Inits Clan Defaults
 */
void clan_defaults(void)
{
	clan = &clan_s;

	clan->init = do_init_clan;
	clan->final = do_final_clan;
	/* */
	clan->db = NULL;
	clan->max = MAX_CLAN;
	clan->maxalliances = MAX_CLANALLIANCE;
	clan->kicktime = 0;
	clan->checktime = 0;
	/* */
	clan->config_read = clan_config_read;
	clan->config_read_additional_settings = clan_config_read_additional_settings;
	clan->read_db = clan_read_db;
	clan->read_db_sub = clan_read_db_sub;
	clan->read_db_additional_fields = clan_read_db_additional_fields;
	clan->search = clan_search;
	clan->searchname = clan_searchname;
	clan->getonlinesd = clan_getonlinesd;
	clan->getindex = clan_getindex;
	clan->join = clan_join;
	clan->member_online = clan_member_online;
	clan->leave = clan_leave;
	clan->removemember_tosql = clan_removemember_tosql;
	clan->ally_count = clan_ally_count;
	clan->antagonist_count = clan_antagonist_count;
	clan->send_message = clan_send_message;
	clan->recv_message = clan_recv_message;
	clan->getonlinecount = clan_getonlinecount;
	clan->member_offline = clan_member_offline;
	clan->constants = clan_constants;
	clan->get_id = clan_get_id;
	clan->buff_start = clan_buff_start;
	clan->buff_end = clan_buff_end;
	clan->reload = clan_reload;
	clan->rejoin = clan_rejoin;
	clan->inactivity_kick = clan_inactivity_kick;
	clan->fix_relationships = clan_fix_relationships;
}
