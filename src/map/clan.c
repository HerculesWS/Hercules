/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017-2023 Hercules Dev Team
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
#include "map/chrif.h"
#include "map/homunculus.h"
#include "map/intif.h"
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct clan_interface clan_s;
struct clan_interface *clan;

/**
 * Searches a Clan by clan_id
 *
 * @param clan_id Clan ID
 * @return struct clan*
 */
static struct clan *clan_search(int clan_id)
{
	if (clan_id <= 0) {
		return NULL;
	}
	return (struct clan *)idb_get(clan->db, clan_id);
}

/**
 * Searches a Clan by clan_name or constant
 *
 * @param name Clan Name
 * @return struct clan*
 */
static struct clan *clan_searchname(const char *name)
{
	struct clan *c;
	struct DBIterator *iter;

	nullpo_retr(NULL, name);

	iter = db_iterator(clan->db);
	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		if (strncmpi(c->name, name, NAME_LENGTH) == 0 || strncmpi(c->constant, name, NAME_LENGTH) == 0) {
			break;
		}
	}
	dbi_destroy(iter);
	return c;
}

/**
 * Returns the first online character of clan
 *
 * @param  (struct clan *) c clan structure
 * @return (struct map_session_data *)
 */
static struct map_session_data *clan_getonlinesd(struct clan *c)
{
	int i;
	nullpo_retr(NULL, c);

	ARR_FIND(0, VECTOR_LENGTH(c->members), i, (VECTOR_INDEX(c->members, i).sd != NULL && VECTOR_INDEX(c->members, i).online == 1));
	return (i < VECTOR_LENGTH(c->members)) ? VECTOR_INDEX(c->members, i).sd : NULL;
}

/**
 * Returns the member index of given Player
 *
 * @param c Clan Data
 * @param char_id Player's Char ID
 * @return int
 */
static int clan_getindex(const struct clan *c, int char_id)
{
	int i;
	nullpo_retr(INDEX_NOT_FOUND, c);

	ARR_FIND(0, VECTOR_LENGTH(c->members), i, VECTOR_INDEX(c->members, i).char_id == char_id);

	if (i == VECTOR_LENGTH(c->members)) {
		return INDEX_NOT_FOUND;
	}
	return i;
}

/**
 * Starts clan buff
 */
static void clan_buff_start(struct map_session_data *sd, struct clan *c)
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
static void clan_buff_end(struct map_session_data *sd, struct clan *c)
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
static bool clan_join(struct map_session_data *sd, int clan_id)
{
	struct clan *c;
	struct clan_member m;

	nullpo_ret(sd);

	// Already joined a guild or clan
	if (sd->status.guild_id > 0 || sd->guild != NULL) {
		ShowError("clan_join: Player already joined in a guild. char_id: %d\n", sd->status.char_id);
		return false;
	} else if ( sd->status.clan_id > 0 || sd->clan != NULL) {
		ShowError("clan_join: Player already joined in a clan. char_id: %d\n", sd->status.char_id);
		return false;
	}

	c = clan->search(clan_id);
	if (c == NULL) {
		ShowError("clan_join: Invalid Clan ID: %d\n", clan_id);
		return false;
	}

	if (!c->received) {
		return false;
	}

	if (clan->getindex(c, sd->status.char_id) != INDEX_NOT_FOUND) {
		ShowError("clan_join: Player already joined this clan. char_id: %d clan_id: %d\n", sd->status.char_id, clan_id);
		return false;
	}

	if (VECTOR_LENGTH(c->members) >= c->max_member || c->member_count >= c->max_member) {
		ShowError("clan_join: Clan '%s' already reached its max capacity!\n", c->name);
		return false;
	}

	VECTOR_ENSURE(c->members, 1, 1);

	m.sd = sd;
	m.char_id = sd->status.char_id;
	m.online = 1;
	m.last_login = sd->status.last_login;
	VECTOR_PUSH(c->members, m);

	c->connect_member++;
	c->member_count++;

	sd->status.clan_id = c->clan_id;
	sd->clan = c;

	sc_start2(NULL, &sd->bl, SC_CLAN_INFO, 10000, 0, c->clan_id, INFINITE_DURATION, 0);
	status_calc_pc(sd, SCO_FORCE);

	chrif->save(sd, 0);
	clif->clan_basicinfo(sd);
	clif->clan_onlinecount(c);
	return true;
}

/**
 * Invoked when a player joins
 * It assumes that clan_id is not 0
 *
 * @param sd Player Data
 */
static void clan_member_online(struct map_session_data *sd, bool first)
{
	struct clan *c;
	int i, inactivity;
	nullpo_retv(sd);

	// For invalid values we must reset it to 0 (no clan)
	if (sd->status.clan_id < 0) {
		ShowError("clan_member_online: Invalid clan id, changing to '0'. clan_id='%d' char_id='%d'\n", sd->status.clan_id, sd->status.char_id);
		sd->status.clan_id = 0;
		return;
	}

	c = clan->search(sd->status.clan_id);
	if (c == NULL) {
		// This is a silent return because it will reset clan_id in case
		// a custom clan that was removed and this is a remaining member
		sd->status.clan_id = 0;
		sd->clan = NULL;
		if (!first) {
			status_change_end(&sd->bl, SC_CLAN_INFO, INVALID_TIMER); // Remove the status
			status_calc_pc(sd, SCO_FORCE);
		}
		clif->clan_leave(sd);
		return;
	}

	if (!c->received) {
		return;
	}

	if (c->max_member <= c->member_count || VECTOR_LENGTH(c->members) >= c->max_member) {
		ShowError("Clan System: More members than the maximum allowed in clan #%d\n", c->clan_id);
		return;
	}

	i = clan->getindex(c, sd->status.char_id);
	inactivity = (int)(time(NULL) - sd->status.last_login);
	if (i == INDEX_NOT_FOUND) {
		struct clan_member m;

		if (c->kick_time > 0 && inactivity > c->kick_time) {
			sd->status.clan_id = 0;
			sd->clan = NULL;
			clan->buff_end(sd, c);
			status_change_end(&sd->bl, SC_CLAN_INFO, INVALID_TIMER);
			clif->clan_leave(sd);
			return;
		}

		VECTOR_ENSURE(c->members, 1, 1);

		m.sd = sd;
		m.char_id = sd->status.char_id;
		m.online = 1;
		m.last_login = sd->status.last_login;
		VECTOR_PUSH(c->members, m);
	} else {
		struct clan_member *m = &VECTOR_INDEX(c->members, i);


		if (c->kick_time > 0 && inactivity > c->kick_time) {
			if (m->online == 1) {
				m->online = 0;
				m->sd = NULL;
				c->connect_member--;
				c->member_count--;
			}
			clan->buff_end(sd, c);
			sd->status.clan_id = 0;
			sd->clan = NULL;
			status_change_end(&sd->bl, SC_CLAN_INFO, INVALID_TIMER);
			VECTOR_ERASE(c->members, i);
			clif->clan_leave(sd);
			return;
		}

		m->sd = sd;
		m->online = 1;
		m->last_login = sd->status.last_login;
	}

	sd->clan = c;
	c->connect_member++;

	sc_start2(NULL, &sd->bl, SC_CLAN_INFO, 10000, 0, c->clan_id, INFINITE_DURATION, 0);

	if (!first) {
		// When first called from pc.c we don't need to do status_calc
		status_calc_pc(sd, SCO_FORCE);
	}

	clif->clan_basicinfo(sd);
	clif->clan_onlinecount(c);
}

/**
 * Re-join a player on its clan
 */
static int clan_rejoin(struct map_session_data *sd, va_list ap)
{
	nullpo_ret(sd);

	if (sd->status.clan_id != 0) {
		// Note: Even if the value is invalid (lower than zero)
		// the function will fix the invalid value
		clan->member_online(sd, false);
	}
	return 0;
}

/**
 * Removes Player from clan
 */
static bool clan_leave(struct map_session_data *sd, bool first)
{
	int i;
	struct clan *c;

	nullpo_ret(sd);

	c = sd->clan;

	if (c == NULL) {
		return false;
	}

	if (!c->received) {
		return false;
	}

	i = clan->getindex(c, sd->status.char_id);
	if (i != INDEX_NOT_FOUND) {
		VECTOR_ERASE(c->members, i);
		c->connect_member--;
		c->member_count--;
	}

	sd->status.clan_id = 0;
	sd->clan = NULL;
	clan->buff_end(sd, c);

	status_change_end(&sd->bl, SC_CLAN_INFO, INVALID_TIMER);
	if (!first) {
		status_calc_pc(sd, SCO_FORCE);
	}

	chrif->save(sd, 0);
	clif->clan_onlinecount(c);
	clif->clan_leave(sd);
	return true;
}

/**
 * Sets a player offline
 *
 * @param (struct map_session_data *) sd Player Data
 */
static void clan_member_offline(struct map_session_data *sd)
{
	struct clan *c;
	int i;

	nullpo_retv(sd);

	c = sd->clan;

	if (c == NULL) {
		return;
	}

	i = clan->getindex(c, sd->status.char_id);
	if (i != INDEX_NOT_FOUND && VECTOR_INDEX(c->members, i).online == 1) {
		// Only if it is online, because unit->free is called twice
		VECTOR_INDEX(c->members, i).online = 0;
		VECTOR_INDEX(c->members, i).sd = NULL;
		c->connect_member--;
	}
	clif->clan_onlinecount(c);
}


/**
 * Sends a message to the whole clan
 */
static bool clan_send_message(struct map_session_data *sd, const char *mes)
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

	return true;
}

/**
 * Clan receive a message, will be displayed to whole clan
 */
static void clan_recv_message(struct clan *c, const char *mes, int len)
{
	clif->clan_message(c, mes, len);
}

/**
 * Set constants for each clan
 */
static void clan_set_constants(void)
{
	struct DBIterator *iter = db_iterator(clan->db);
	struct clan *c;

	for (c = dbi_first(iter); dbi_exists(iter); c = dbi_next(iter)) {
		script->set_constant2(c->constant, c->clan_id, false, false);
	}

	dbi_destroy(iter);
}

/**
 * Returns the clan_id of bl
 */
static int clan_get_id(const struct block_list *bl)
{
	nullpo_ret(bl);

	switch (bl->type) {
	case BL_PC: {
		const struct map_session_data *sd = BL_UCCAST(BL_PC, bl);
		return sd->status.clan_id;
	}
	case BL_NPC: {
		const struct npc_data *nd = BL_UCCAST(BL_NPC, bl);
		return nd->clan_id;
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
	case BL_NUL:
	case BL_ITEM:
	case BL_ELEM:
	case BL_CHAT:
	case BL_ALL:
		break;
	}

	return 0;
}

/**
 * Checks every clan player and remove those who are inactive
 */
static int clan_inactivity_kick(int tid, int64 tick, int id, intptr_t data)
{
	struct clan *c = NULL;
	int i;

	if ((c = clan->search(id)) != NULL) {
		if (!c->kick_time || c->tid != tid || tid == INVALID_TIMER || c->tid == INVALID_TIMER) {
		  ShowError("Timer Mismatch (Time: %d seconds) %d != %d", c->kick_time, c->tid, tid);
		  Assert_report(0);
		  return 0;
		}
		for (i = 0; i < VECTOR_LENGTH(c->members); i++) {
			struct clan_member *m = &VECTOR_INDEX(c->members, i);
			if (m->char_id <= 0 || m->online <= 0)
				continue;

			if (m->online) {
				struct map_session_data *sd = m->sd;
				if (DIFF_TICK32(sockt->last_tick, sd->idletime) > c->kick_time) {
					clan->leave(sd, false);
				}
			} else if ((time(NULL) - m->last_login) > c->kick_time) {
				VECTOR_ERASE(c->members, i);
				c->member_count--;
				clif->clan_onlinecount(c);
			}
		}
		//Perform the kick for offline members that didn't connect after a server restart
		c->received = false;
		intif->clan_kickoffline(c->clan_id, c->kick_time);
		c->tid = timer->add(timer->gettick() + c->check_time, clan->inactivity_kick, c->clan_id, 0);
	}
	return 1;
}

/**
 * Timeout for the request of offline kick
 */
static int clan_request_kickoffline(int tid, int64 tick, int id, intptr_t data)
{
	struct clan *c = NULL;

	if ((c = clan->search(id)) != NULL) {
		if (c->req_kick_tid != tid || c->req_kick_tid == INVALID_TIMER) {
		  ShowError("Timer Mismatch %d != %d", c->tid, tid);
		  return 0;
		}

		if (c->received) {
			c->req_kick_tid = INVALID_TIMER;
			return 1;
		}

		intif->clan_kickoffline(c->clan_id, c->kick_time);
		c->req_kick_tid = timer->add(timer->gettick() + clan->req_timeout, clan->request_kickoffline, c->clan_id, 0);
	}
	return 1;
}

/**
 * Timeout of the request for counting members
 */
static int clan_request_membercount(int tid, int64 tick, int id, intptr_t data)
{
	struct clan *c = NULL;

	if ((c = clan->search(id)) != NULL) {
		if (c->req_count_tid != tid || c->req_count_tid == INVALID_TIMER) {
		  ShowError("Timer Mismatch %d != %d", c->tid, tid);
		  return 0;
		}

		if (c->received) {
			c->req_count_tid = INVALID_TIMER;
			return 1;
		}

		intif->clan_membercount(c->clan_id, c->kick_time);
		c->req_count_tid = timer->add(timer->gettick() + clan->req_timeout, clan->request_membercount, c->clan_id, 0);
	}
	return 1;
}

/**
 * Processes any (plugin-defined) additional fields for a clan entry.
 *
 * @param[in, out] entry The destination clan entry, already initialized (clan_id is expected to be already set).
 * @param[in] t The libconfig entry.
 * @param[in] n Ordinal number of the entry, to be displayed in case of validation errors.
 * @param[in] source Source of the entry (file name), to be displayed in case of validation errors.
 */
static void clan_read_db_additional_fields(struct clan *entry, struct config_setting_t *t, int n, const char *source)
{
	// do nothing. plugins can do own work
}

static void clan_read_buffs(struct clan *c, struct config_setting_t *buff, const char *source)
{
	struct clan_buff *b;
	const char *str = NULL;

	nullpo_retv(c);
	nullpo_retv(buff);

	b = &c->buff;

	if (!libconfig->setting_lookup_int(buff, "Icon", &b->icon)) {
		const char *temp_str = NULL;
		if (libconfig->setting_lookup_string(buff, "Icon", &temp_str)) {
			if (*temp_str && !script->get_constant(temp_str, &b->icon)) {
				ShowWarning("Clan %d: Invalid Buff icon value. Defaulting to SI_BLANK.\n", c->clan_id);
				b->icon = -1; // SI_BLANK
			}
		}
	}

	if (libconfig->setting_lookup_string(buff, "Script", &str)) {
		b->script = *str ? script->parse(str, source, -b->icon, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;
	}
}

/**
 * Processes one clandb entry from the libconfig file, loading and inserting it
 * into the clan database.
 *
 * @param settings Libconfig setting entry. It is expected to be valid and it
 *                 won't be freed (it is care of the caller to do so if
 *                 necessary).
 * @param source   Source of the entry (file name), to be displayed in case of
 *                 validation errors.
 * @return int.
 */
static int clan_read_db_sub(struct config_setting_t *settings, const char *source, bool reload)
{
	int total, s, valid = 0;

	nullpo_retr(false, settings);

	total = libconfig->setting_length(settings);

	for (s = 0; s < total; s++) {
		struct clan *c;
		struct config_setting_t *cl, *buff, *allies, *antagonists;
		const char *aName, *aLeader, *aMap, *aConst;
		int max_members, kicktime = 0, kickchecktime = 0, id, i, j;

		cl = libconfig->setting_get_elem(settings, s);

		if (libconfig->setting_lookup_int(cl, "Id", &id)) {
			if (id <= 0) {
				ShowError("clan_read_db: Invalid Clan Id %d, skipping...\n", id);
				return false;
			}

			if (clan->search(id) != NULL) {
				ShowError("clan_read_db: Duplicate entry for Clan Id %d, skipping...\n", id);
				return false;
			}
		} else {
			ShowError("clan_read_db: failed to find 'Id' for clan #%d\n", s);
			return false;
		}

		if (libconfig->setting_lookup_string(cl, "Const", &aConst)) {
			if (!*aConst) {
				ShowError("clan_read_db: Invalid Clan Const '%s', skipping...\n", aConst);
				return false;
			}

			if (strlen(aConst) > NAME_LENGTH) {
				ShowError("clan_read_db: Clan Name '%s' is longer than %d characters, skipping...\n", aConst, NAME_LENGTH);
				return false;
			}

			if (clan->searchname(aConst) != NULL) {
				ShowError("clan_read_db: Duplicate entry for Clan Const '%s', skipping...\n", aConst);
				return false;
			}
		} else {
			ShowError("clan_read_db: failed to find 'Const' for clan #%d\n", s);
			return false;
		}

		if (libconfig->setting_lookup_string(cl, "Name", &aName)) {
			if (!*aName) {
				ShowError("clan_read_db: Invalid Clan Name '%s', skipping...\n", aName);
				return false;
			}

			if (strlen(aName) > NAME_LENGTH) {
				ShowError("clan_read_db: Clan Name '%s' is longer than %d characters, skipping...\n", aName, NAME_LENGTH);
				return false;
			}

			if (clan->searchname(aName) != NULL) {
				ShowError("clan_read_db: Duplicate entry for Clan Name '%s', skipping...\n", aName);
				return false;
			}
		} else {
			ShowError("clan_read_db: failed to find 'Name' for clan #%d\n", s);
			return false;
		}

		if (!libconfig->setting_lookup_string(cl, "Leader", &aLeader)) {
			ShowError("clan_read_db: failed to find 'Leader' for clan #%d\n", s);
			return false;
		}

		if (!libconfig->setting_lookup_string(cl, "Map", &aMap)) {
			ShowError("clan_read_db: failed to find 'Map' for clan #%d\n", s);
			return false;
		}

		CREATE(c, struct clan, 1);

		c->clan_id = id;
		safestrncpy(c->constant, aConst, NAME_LENGTH);
		safestrncpy(c->name, aName, NAME_LENGTH);
		safestrncpy(c->master, aLeader, NAME_LENGTH);
		safestrncpy(c->map, aMap, MAP_NAME_LENGTH_EXT);
		c->connect_member = 0;
		c->member_count = 0; // Char server will count members for us
		c->received = false;
		c->req_count_tid = INVALID_TIMER;
		c->req_kick_tid = INVALID_TIMER;
		c->tid = INVALID_TIMER;

		if (libconfig->setting_lookup_int(cl, "MaxMembers", &max_members)) {
			if (max_members > 0) {
				c->max_member = max_members;
			} else {
				ShowError("clan_read_db: Clan #%d has invalid value for 'MaxMembers' setting, defaulting to 'clan->max'...\n", id);
				c->max_member = clan->max;
			}
		} else {
			c->max_member = clan->max;
		}

		if (libconfig->setting_lookup_int(cl, "KickTime", &kicktime)) {
			c->kick_time = 60 * 60 * kicktime;
		} else {
			c->kick_time = clan->kicktime;
		}

		if (libconfig->setting_lookup_int(cl, "CheckTime", &kickchecktime)) {
			c->check_time = 60 * 60 * max(1, kickchecktime) * 1000;
		} else {
			c->check_time = clan->checktime;
		}

		if ((buff = libconfig->setting_get_member(cl, "Buff")) != NULL) {
			clan->read_buffs(c, buff, source);
		}

		allies = libconfig->setting_get_member(cl, "Allies");

		if (allies != NULL) {
			int a = libconfig->setting_length(allies);

			VECTOR_INIT(c->allies);
			if (a > 0 && clan->max_relations > 0) {
				const char *allyConst;

				if (a > clan->max_relations) {
					ShowWarning("clan_read_db: Clan %d has more allies(%d) than allowed(%d), reading only the first %d...\n", c->clan_id, a, clan->max_relations, clan->max_relations);
					a = clan->max_relations;
				}

				VECTOR_ENSURE(c->allies, a, 1);
				for (i = 0; i < a; i++) {
					struct clan_relationship r;
					if ((allyConst = libconfig->setting_get_string_elem(allies, i)) != NULL) {
						ARR_FIND(0, VECTOR_LENGTH(c->allies), j,  strcmp(VECTOR_INDEX(c->allies, j).constant, allyConst) == 0);
						if (j != VECTOR_LENGTH(c->allies)) {
							ShowError("clan_read_db: Duplicate entry '%s' in allies for Clan %d in '%s', skipping...\n", allyConst, c->clan_id, source);
							continue;
						} else if (strcmp(allyConst, c->constant) == 0) {
							ShowError("clan_read_db: Clans can't be allies of themselves! Clan Id: %d, in '%s'\n", c->clan_id, source);
							continue;
						}
						safestrncpy(r.constant, allyConst, NAME_LENGTH);
						VECTOR_PUSH(c->allies, r);
					}

				}
			}
		}
		antagonists = libconfig->setting_get_member(cl, "Antagonists");

		if (antagonists != NULL) {
			int a = libconfig->setting_length(antagonists);

			VECTOR_INIT(c->antagonists);
			if (a > 0 && clan->max_relations > 0) {
				const char *antagonistConst;

				if (a > clan->max_relations) {
					ShowWarning("clan_read_db: Clan %d has more antagonists(%d) than allowed(%d), reading only the first %d...\n", c->clan_id, a, clan->max_relations, clan->max_relations);
					a = clan->max_relations;
				}

				VECTOR_ENSURE(c->antagonists, a, 1);
				for (i = 0; i < a; i++) {
					struct clan_relationship r;
					if ((antagonistConst = libconfig->setting_get_string_elem(antagonists, i)) != NULL) {
						ARR_FIND(0, VECTOR_LENGTH(c->antagonists), j,  strcmp(VECTOR_INDEX(c->antagonists, j).constant, antagonistConst) == 0);
						if (j != VECTOR_LENGTH(c->antagonists)) {
							ShowError("clan_read_db: Duplicate entry '%s' in antagonists for Clan %d in '%s', skipping...\n", antagonistConst, c->clan_id, source);
							continue;
						} else if (strcmp(antagonistConst, c->constant) == 0) {
							ShowError("clan_read_db: Clans can't be antagonists of themselves! Clan Id: %d, in '%s'\n", c->clan_id, source);
							continue;
						}
						safestrncpy(r.constant, antagonistConst, NAME_LENGTH);
						VECTOR_PUSH(c->antagonists, r);
					}
				}
			}
		}

		clan->read_db_additional_fields(c, cl, s, source);
		if (c->kick_time > 0) {
			c->tid = timer->add(timer->gettick() + c->check_time, clan->inactivity_kick, c->clan_id, 0);
		}
		c->received = false;
		c->req_state = reload ? CLAN_REQ_RELOAD : CLAN_REQ_FIRST;
		c->req_count_tid = timer->add(timer->gettick() + clan->req_timeout, clan->request_membercount, c->clan_id, 0);
		idb_put(clan->db, c->clan_id, c);
		VECTOR_INIT(c->members);
		valid++;
	}

	// Validating Relations
	if (valid > 0) {
		struct DBIterator *iter = db_iterator(clan->db);
		struct clan *c_ok, *c;
		int i;

		for (c_ok = dbi_first(iter); dbi_exists(iter); c_ok = dbi_next(iter)) {
			i = VECTOR_LENGTH(c_ok->allies);
			while ( i > 0) {
				struct clan_relationship *r;

				i--;
				r = &VECTOR_INDEX(c_ok->allies, i);
				if ((c = clan->searchname(r->constant)) == NULL) {
					ShowError("clan_read_db: Invalid (nonexistent) Ally '%s' for clan %d in '%s', skipping ally...\n", r->constant, c_ok->clan_id, source);
					VECTOR_ERASE(c_ok->allies, i);
				} else {
					r->clan_id = c->clan_id;
				}
			}

			i = VECTOR_LENGTH(c_ok->antagonists);
			while ( i > 0) {
				struct clan_relationship *r;

				i--;
				r = &VECTOR_INDEX(c_ok->antagonists, i);
				if ((c = clan->searchname(r->constant)) == NULL) {
					ShowError("clan_read_db: Invalid (nonexistent) Antagonist '%s' for clan %d in '%s', skipping antagonist...", r->constant, c_ok->clan_id, source);
					VECTOR_ERASE(c_ok->antagonists, i);
				} else {
					r->clan_id = c->clan_id;
				}
			}
		}
		dbi_destroy(iter);
	}

	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' valid clans of '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", valid, total, source);
	return valid;
}

/**
 * Reads Clan DB included by clan configuration file.
 *
 * @param settings The Settings Group from config file.
 * @param source File name.
 */
static void clan_read_db(struct config_setting_t *settings, const char *source, bool reload)
{
	struct config_setting_t *clans;

	nullpo_retv(settings);

	if ((clans = libconfig->setting_get_member(settings, "clans")) != NULL) {
		int read;

		read = clan->read_db_sub(clans, source, reload);
		if (read > 0) {
			clan->set_constants();
		}
	} else {
		ShowError("clan_read_db: Can't find setting 'clans' in '%s'. No Clans found.\n", source);
	}
}

/**
 * Reads clan config file
 *
 * @param bool clear Whether to clear clan->db before reading clans
 */
static bool clan_config_read(bool reload)
{
	struct config_t clan_conf;
	struct config_setting_t *settings = NULL;
	const char *config_filename = "conf/clans.conf"; // FIXME: hardcoded name
	int kicktime = 0, kickchecktime = 0;

	if (reload) {
		struct DBIterator *iter = db_iterator(clan->db);
		struct clan *c_clear;

		for (c_clear = dbi_first(iter); dbi_exists(iter); c_clear = dbi_next(iter)) {
			if (c_clear->buff.script != NULL) {
				script->free_code(c_clear->buff.script);
			}
			if (c_clear->tid != INVALID_TIMER) {
				timer->delete(c_clear->tid, clan->inactivity_kick);
			}
			VECTOR_CLEAR(c_clear->allies);
			VECTOR_CLEAR(c_clear->antagonists);
			VECTOR_CLEAR(c_clear->members);
			HPM->data_store_destroy(&c_clear->hdata);
		}
		dbi_destroy(iter);
		clan->db->clear(clan->db, NULL);
	}

	if (!libconfig->load_file(&clan_conf, config_filename)) {
		return false;
	}

	if ((settings = libconfig->lookup(&clan_conf, "clan_configuration")) == NULL) {
		ShowError("clan_config_read: failed to find 'clan_configuration'.\n");
		return false;
	}

	libconfig->setting_lookup_int(settings, "MaxMembers", &clan->max);
	libconfig->setting_lookup_int(settings, "MaxRelations", &clan->max_relations);
	libconfig->setting_lookup_int(settings, "InactivityKickTime", &kicktime);
	if (!libconfig->setting_lookup_int(settings, "InactivityCheckTime", &kickchecktime)) {
		ShowError("clan_config_read: failed to find InactivityCheckTime using official value.\n");
		kickchecktime = 24;
	}

	// On config file we set the time in hours but here we use in seconds
	clan->kicktime = 60 * 60 * kicktime;
	clan->checktime = 60 * 60 * max(kickchecktime, 1) * 1000;

	clan->config_read_additional_settings(settings, config_filename);
	clan->read_db(settings, config_filename, reload);
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
static void clan_config_read_additional_settings(struct config_setting_t *settings, const char *source)
{
	// do nothing. plugins can do own work
}

/**
 * Reloads Clan DB
 */
static void clan_reload(void)
{
	clan->config_read(true);
}

/**
 *
 */
static void do_init_clan(bool minimal)
{
	clan->db = idb_alloc(DB_OPT_RELEASE_DATA);

	if (minimal) {
		return;
	}
	clan->config_read(false);
	timer->add_func_list(clan->inactivity_kick, "clan_inactivity_kick");
}

/**
 *
 */
static void do_final_clan(void)
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
		VECTOR_CLEAR(c->allies);
		VECTOR_CLEAR(c->antagonists);
		VECTOR_CLEAR(c->members);
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
	clan->max = 0;
	clan->max_relations = 0;
	clan->kicktime = 0;
	clan->checktime = 0;
	clan->req_timeout = 60;
	/* */
	clan->config_read = clan_config_read;
	clan->config_read_additional_settings = clan_config_read_additional_settings;
	clan->read_db = clan_read_db;
	clan->read_db_sub = clan_read_db_sub;
	clan->read_db_additional_fields = clan_read_db_additional_fields;
	clan->read_buffs = clan_read_buffs;
	clan->search = clan_search;
	clan->searchname = clan_searchname;
	clan->getonlinesd = clan_getonlinesd;
	clan->getindex = clan_getindex;
	clan->join = clan_join;
	clan->member_online = clan_member_online;
	clan->leave = clan_leave;
	clan->send_message = clan_send_message;
	clan->recv_message = clan_recv_message;
	clan->member_offline = clan_member_offline;
	clan->set_constants = clan_set_constants;
	clan->get_id = clan_get_id;
	clan->buff_start = clan_buff_start;
	clan->buff_end = clan_buff_end;
	clan->reload = clan_reload;
	clan->rejoin = clan_rejoin;
	clan->inactivity_kick = clan_inactivity_kick;
	clan->request_kickoffline = clan_request_kickoffline;
	clan->request_membercount = clan_request_membercount;
}
