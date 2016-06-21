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

#include "mercenary.h"

#include "map/atcommand.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/guild.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/trade.h"
#include "map/unit.h"
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mercenary_interface mercenary_s;
struct s_mercenary_db mercdb[MAX_MERCENARY_CLASS];

struct mercenary_interface *mercenary;

int merc_search_index(int class_)
{
	int i;
	ARR_FIND(0, MAX_MERCENARY_CLASS, i, mercenary->db[i].class_ == class_);
	if (i == MAX_MERCENARY_CLASS)
		return INDEX_NOT_FOUND;
	return i;
}

bool merc_class(int class_)
{
	if (mercenary->search_index(class_) != INDEX_NOT_FOUND)
		return true;
	return false;
}

struct view_data * merc_get_viewdata(int class_)
{
	int i = mercenary->search_index(class_);
	if (i == INDEX_NOT_FOUND)
		return NULL;

	return &mercenary->db[i].vd;
}

int merc_create(struct map_session_data *sd, int class_, unsigned int lifetime)
{
	struct s_mercenary merc;
	struct s_mercenary_db *db;
	int i;
	nullpo_retr(0,sd);

	if ((i = mercenary->search_index(class_)) == INDEX_NOT_FOUND)
		return 0;

	db = &mercenary->db[i];
	memset(&merc,0,sizeof(struct s_mercenary));

	merc.char_id = sd->status.char_id;
	merc.class_ = class_;
	merc.hp = db->status.max_hp;
	merc.sp = db->status.max_sp;
	merc.life_time = lifetime;

	// Request Char Server to create this mercenary
	intif->mercenary_create(&merc);

	return 1;
}

int mercenary_get_lifetime(struct mercenary_data *md)
{
	const struct TimerData * td;
	if( md == NULL || md->contract_timer == INVALID_TIMER )
		return 0;

	td = timer->get(md->contract_timer);
	return (td != NULL) ? DIFF_TICK32(td->tick, timer->gettick()) : 0;
}

int mercenary_get_guild(struct mercenary_data *md)
{
	int class_;

	if( md == NULL || md->db == NULL )
		return -1;

	class_ = md->db->class_;

	if (class_ >= MERID_MER_ARCHER01 && class_ <= MERID_MER_ARCHER10)
		return ARCH_MERC_GUILD;
	if (class_ >= MERID_MER_LANCER01 && class_ <= MERID_MER_LANCER10)
		return SPEAR_MERC_GUILD;
	if (class_ >= MERID_MER_SWORDMAN01 && class_ <= MERID_MER_SWORDMAN10)
		return SWORD_MERC_GUILD;

	return -1;
}

int mercenary_get_faith(struct mercenary_data *md)
{
	struct map_session_data *sd;
	int class_;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return 0;

	class_ = md->db->class_;

	if (class_ >= MERID_MER_ARCHER01 && class_ <= MERID_MER_ARCHER10)
		return sd->status.arch_faith;
	if (class_ >= MERID_MER_LANCER01 && class_ <= MERID_MER_LANCER10)
		return sd->status.spear_faith;
	if (class_ >= MERID_MER_SWORDMAN01 && class_ <= MERID_MER_SWORDMAN10)
		return sd->status.sword_faith;

	return 0;
}

int mercenary_set_faith(struct mercenary_data *md, int value)
{
	struct map_session_data *sd;
	int class_, *faith;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return 0;

	class_ = md->db->class_;

	if (class_ >= MERID_MER_ARCHER01 && class_ <= MERID_MER_ARCHER10)
		faith = &sd->status.arch_faith;
	else if (class_ >= MERID_MER_LANCER01 && class_ <= MERID_MER_LANCER10)
		faith = &sd->status.spear_faith;
	else if (class_ >= MERID_MER_SWORDMAN01 && class_ <= MERID_MER_SWORDMAN10)
		faith = &sd->status.sword_faith;
	else
		return 0;

	*faith += value;
	*faith = cap_value(*faith, 0, SHRT_MAX);
	clif->mercenary_updatestatus(sd, SP_MERCFAITH);

	return 0;
}

int mercenary_get_calls(struct mercenary_data *md)
{
	struct map_session_data *sd;
	int class_;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return 0;

	class_ = md->db->class_;

	if (class_ >= MERID_MER_ARCHER01 && class_ <= MERID_MER_ARCHER10)
		return sd->status.arch_calls;
	if (class_ >= MERID_MER_LANCER01 && class_ <= MERID_MER_LANCER10)
		return sd->status.spear_calls;
	if (class_ >= MERID_MER_SWORDMAN01 && class_ <= MERID_MER_SWORDMAN10)
		return sd->status.sword_calls;

	return 0;
}

int mercenary_set_calls(struct mercenary_data *md, int value)
{
	struct map_session_data *sd;
	int class_, *calls;

	if( md == NULL || md->db == NULL || (sd = md->master) == NULL )
		return 0;

	class_ = md->db->class_;

	if (class_ >= MERID_MER_ARCHER01 && class_ <= MERID_MER_ARCHER10)
		calls = &sd->status.arch_calls;
	else if (class_ >= MERID_MER_LANCER01 && class_ <= MERID_MER_LANCER10)
		calls = &sd->status.spear_calls;
	else if (class_ >= MERID_MER_SWORDMAN01 && class_ <= MERID_MER_SWORDMAN10)
		calls = &sd->status.sword_calls;
	else
		return 0;

	*calls += value;
	*calls = cap_value(*calls, 0, INT_MAX);

	return 0;
}

int mercenary_save(struct mercenary_data *md)
{
	nullpo_retr(1, md);
	md->mercenary.hp = md->battle_status.hp;
	md->mercenary.sp = md->battle_status.sp;
	md->mercenary.life_time = mercenary->get_lifetime(md);

	intif->mercenary_save(&md->mercenary);
	return 1;
}

int merc_contract_end_timer(int tid, int64 tick, int id, intptr_t data) {
	struct map_session_data *sd;
	struct mercenary_data *md;

	if( (sd = map->id2sd(id)) == NULL )
		return 1;
	if( (md = sd->md) == NULL )
		return 1;

	if( md->contract_timer != tid )
	{
		ShowError("merc_contract_end_timer %d != %d.\n", md->contract_timer, tid);
		return 0;
	}

	md->contract_timer = INVALID_TIMER;
	mercenary->delete(md, 0); // Mercenary soldier's duty hour is over.

	return 0;
}

int merc_delete(struct mercenary_data *md, int reply)
{
	struct map_session_data *sd;

	nullpo_retr(0, md);
	sd = md->master;
	md->mercenary.life_time = 0;

	mercenary->contract_stop(md);

	if( !sd )
		return unit->free(&md->bl, CLR_OUTSIGHT);

	if( md->devotion_flag )
	{
		md->devotion_flag = 0;
		status_change_end(&sd->bl, SC_DEVOTION, INVALID_TIMER);
	}

	switch( reply )
	{
		case 0: mercenary->set_faith(md, 1); break; // +1 Loyalty on Contract ends.
		case 1: mercenary->set_faith(md, -1); break; // -1 Loyalty on Mercenary killed
	}

	clif->mercenary_message(sd, reply);
	return unit->remove_map(&md->bl, CLR_OUTSIGHT, ALC_MARK);
}

void merc_contract_stop(struct mercenary_data *md)
{
	nullpo_retv(md);
	if( md->contract_timer != INVALID_TIMER )
		timer->delete(md->contract_timer, mercenary->contract_end_timer);
	md->contract_timer = INVALID_TIMER;
}

void merc_contract_init(struct mercenary_data *md)
{
	nullpo_retv(md);
	if( md->contract_timer == INVALID_TIMER )
		md->contract_timer = timer->add(timer->gettick() + md->mercenary.life_time, mercenary->contract_end_timer, md->master->bl.id, 0);

	md->regen.state.block = 0;
}

int merc_data_received(const struct s_mercenary *merc, bool flag)
{
	struct map_session_data *sd;
	struct mercenary_data *md;
	struct s_mercenary_db *db;
	int i;

	nullpo_ret(merc);
	i = mercenary->search_index(merc->class_);
	if( (sd = map->charid2sd(merc->char_id)) == NULL )
		return 0;
	if (!flag || i == INDEX_NOT_FOUND) {
		// Not created - loaded - DB info
		sd->status.mer_id = 0;
		return 0;
	}

	db = &mercenary->db[i];
	if( !sd->md ) {
		CREATE(md, struct mercenary_data, 1);
		md->bl.type = BL_MER;
		md->bl.id = npc->get_new_npc_id();
		md->devotion_flag = 0;
		sd->md = md;

		md->master = sd;
		md->db = db;
		memcpy(&md->mercenary, merc, sizeof(struct s_mercenary));
		status->set_viewdata(&md->bl, md->mercenary.class_);
		status->change_init(&md->bl);
		unit->dataset(&md->bl);
		md->ud.dir = sd->ud.dir;

		md->bl.m = sd->bl.m;
		md->bl.x = sd->bl.x;
		md->bl.y = sd->bl.y;
		unit->calc_pos(&md->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
		md->bl.x = md->ud.to_x;
		md->bl.y = md->ud.to_y;

		map->addiddb(&md->bl);
		status_calc_mercenary(md,SCO_FIRST);
		md->contract_timer = INVALID_TIMER;
		merc_contract_init(md);
	}
	else
	{
		memcpy(&sd->md->mercenary, merc, sizeof(struct s_mercenary));
		md = sd->md;
	}

	if( sd->status.mer_id == 0 )
		mercenary->set_calls(md, 1);
	sd->status.mer_id = merc->mercenary_id;

	if( md->bl.prev == NULL && sd->bl.prev != NULL ) {
		map->addblock(&md->bl);
		clif->spawn(&md->bl);
		clif->mercenary_info(sd);
		clif->mercenary_skillblock(sd);
	}

	return 1;
}

void mercenary_heal(struct mercenary_data *md, int hp, int sp)
{
	nullpo_retv(md);
	if( hp )
		clif->mercenary_updatestatus(md->master, SP_HP);
	if( sp )
		clif->mercenary_updatestatus(md->master, SP_SP);
}

int mercenary_dead(struct mercenary_data *md)
{
	mercenary->delete(md, 1);
	return 0;
}

int mercenary_killbonus(struct mercenary_data *md)
{
	const enum sc_type scs[] = { SC_MER_FLEE, SC_MER_ATK, SC_MER_HP, SC_MER_SP, SC_MER_HIT };
	int index = rnd() % ARRAYLENGTH(scs);

	nullpo_ret(md);
	sc_start(NULL,&md->bl, scs[index], 100, rnd() % 5, 600000);
	return 0;
}

int mercenary_kills(struct mercenary_data *md)
{
	nullpo_ret(md);
	md->mercenary.kill_count++;
	md->mercenary.kill_count = cap_value(md->mercenary.kill_count, 0, INT_MAX);

	if( (md->mercenary.kill_count % 50) == 0 )
	{
		mercenary->set_faith(md, 1);
		mercenary->killbonus(md);
	}

	if( md->master )
		clif->mercenary_updatestatus(md->master, SP_MERCKILLS);

	return 0;
}

int mercenary_checkskill(struct mercenary_data *md, uint16 skill_id)
{
	int i = skill_id - MC_SKILLBASE;

	if( !md || !md->db )
		return 0;
	if( md->db->skill[i].id == skill_id )
		return md->db->skill[i].lv;

	return 0;
}

bool read_mercenarydb_sub(char* str[], int columns, int current) {
	int ele;
	struct s_mercenary_db *db;
	struct status_data *mstatus;

	nullpo_retr(false, str);
	Assert_retr(false, current >= 0 && current < MAX_MERCENARY_CLASS);
	db = &mercenary->db[current];
	db->class_ = atoi(str[0]);
	safestrncpy(db->sprite, str[1], NAME_LENGTH);
	safestrncpy(db->name, str[2], NAME_LENGTH);
	db->lv = atoi(str[3]);

	mstatus = &db->status;
	db->vd.class_ = db->class_;

	mstatus->max_hp = atoi(str[4]);
	mstatus->max_sp = atoi(str[5]);
	mstatus->rhw.range = atoi(str[6]);
	mstatus->rhw.atk = atoi(str[7]);
	mstatus->rhw.atk2 = mstatus->rhw.atk + atoi(str[8]);
	mstatus->def = atoi(str[9]);
	mstatus->mdef = atoi(str[10]);
	mstatus->str = atoi(str[11]);
	mstatus->agi = atoi(str[12]);
	mstatus->vit = atoi(str[13]);
	mstatus->int_ = atoi(str[14]);
	mstatus->dex = atoi(str[15]);
	mstatus->luk = atoi(str[16]);
	db->range2 = atoi(str[17]);
	db->range3 = atoi(str[18]);
	mstatus->size = atoi(str[19]);
	mstatus->race = atoi(str[20]);

	ele = atoi(str[21]);
	mstatus->def_ele = ele%10;
	mstatus->ele_lv = ele/20;
	if( mstatus->def_ele >= ELE_MAX ) {
		ShowWarning("Mercenary %d has invalid element type %d (max element is %d)\n", db->class_, mstatus->def_ele, ELE_MAX - 1);
		mstatus->def_ele = ELE_NEUTRAL;
	}
	if( mstatus->ele_lv < 1 || mstatus->ele_lv > 4 ) {
		ShowWarning("Mercenary %d has invalid element level %d (max is 4)\n", db->class_, mstatus->ele_lv);
		mstatus->ele_lv = 1;
	}

	mstatus->aspd_rate = 1000;
	mstatus->speed = atoi(str[22]);
	mstatus->adelay = atoi(str[23]);
	mstatus->amotion = atoi(str[24]);
	mstatus->dmotion = atoi(str[25]);

	return true;
}

int read_mercenarydb(void) {
	memset(mercenary->db, 0, sizeof(struct s_mercenary_db) * MAX_MERCENARY_CLASS);
	sv->readdb(map->db_path, "mercenary_db.txt", ',', 26, 26, MAX_MERCENARY_CLASS, mercenary->read_db_sub);

	return 0;
}

bool read_mercenary_skilldb_sub(char* str[], int columns, int current)
{// <merc id>,<skill id>,<skill level>
	struct s_mercenary_db *db;
	int i, class_;
	uint16 skill_id, skill_lv;

	nullpo_retr(false, str);
	class_ = atoi(str[0]);
	ARR_FIND(0, MAX_MERCENARY_CLASS, i, class_ == mercenary->db[i].class_);
	if( i == MAX_MERCENARY_CLASS )
	{
		ShowError("read_mercenary_skilldb : Class %d not found in mercenary_db for skill entry.\n", class_);
		return false;
	}

	skill_id = atoi(str[1]);
	if( skill_id < MC_SKILLBASE || skill_id >= MC_SKILLBASE + MAX_MERCSKILL )
	{
		ShowError("read_mercenary_skilldb : Skill %d out of range.\n", skill_id);
		return false;
	}

	db = &mercenary->db[i];
	skill_lv = atoi(str[2]);

	i = skill_id - MC_SKILLBASE;
	db->skill[i].id = skill_id;
	db->skill[i].lv = skill_lv;

	return true;
}

int read_mercenary_skilldb(void) {
	sv->readdb(map->db_path, "mercenary_skill_db.txt", ',', 3, 3, -1, mercenary->read_skill_db_sub);

	return 0;
}

void do_init_mercenary(bool minimal) {
	if (minimal)
		return;

	mercenary->read_db();
	mercenary->read_skilldb();

	timer->add_func_list(mercenary->contract_end_timer, "merc_contract_end_timer");
}

/*=====================================
* Default Functions : mercenary.h
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void mercenary_defaults(void) {
	mercenary = &mercenary_s;

	/* vars */
	mercenary->db = mercdb;
	memset(mercenary->db, 0, sizeof(struct s_mercenary_db) * MAX_MERCENARY_CLASS);

	/* funcs */
	mercenary->init = do_init_mercenary;

	mercenary->class = merc_class;
	mercenary->get_viewdata = merc_get_viewdata;

	mercenary->create = merc_create;
	mercenary->data_received = merc_data_received;
	mercenary->save = mercenary_save;

	mercenary->heal = mercenary_heal;
	mercenary->dead = mercenary_dead;

	mercenary->delete = merc_delete;
	mercenary->contract_stop = merc_contract_stop;

	mercenary->get_lifetime = mercenary_get_lifetime;
	mercenary->get_guild = mercenary_get_guild;
	mercenary->get_faith = mercenary_get_faith;
	mercenary->set_faith = mercenary_set_faith;
	mercenary->get_calls = mercenary_get_calls;
	mercenary->set_calls = mercenary_set_calls;
	mercenary->kills = mercenary_kills;

	mercenary->checkskill = mercenary_checkskill;
	mercenary->read_db = read_mercenarydb;
	mercenary->read_skilldb = read_mercenary_skilldb;

	mercenary->killbonus = mercenary_killbonus;
	mercenary->search_index = merc_search_index;

	mercenary->contract_end_timer = merc_contract_end_timer;
	mercenary->read_db_sub = read_mercenarydb_sub;
	mercenary->read_skill_db_sub = read_mercenary_skilldb_sub;
}
