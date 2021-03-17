/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
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

#include "config/core.h" // DBPATH
#include "pet.h"

#include "map/achievement.h"
#include "map/atcommand.h" // msg_txt()
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/status.h"
#include "map/unit.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct pet_interface pet_s;
struct pet_interface *pet;

#define MIN_PETTHINKTIME 100

/**
 * Gets a pet's hunger value, depending it's hunger level.
 * This value is only used in clif_parse_LoadEndAck() when calling clif_pet_emotion().
 *
 * @param pd The pet.
 * @return The pet's hunger value.
 *
 **/
static int pet_hungry_val(struct pet_data *pd)
{
	nullpo_ret(pd);

	if (pd->pet.hungry > PET_HUNGER_SATISFIED)
		return 4;
	else if (pd->pet.hungry > PET_HUNGER_NEUTRAL)
		return 3;
	else if (pd->pet.hungry > PET_HUNGER_HUNGRY)
		return 2;
	else if (pd->pet.hungry > PET_HUNGER_VERY_HUNGRY)
		return 1;
	else
		return 0;
}

/**
 * Sets a pet's hunger value.
 *
 * @param pd The pet.
 * @param value The pet's new hunger value.
 *
 **/
static void pet_set_hunger(struct pet_data *pd, int value)
{
	nullpo_retv(pd);
	nullpo_retv(pd->msd);

	pd->pet.hungry = cap_value(value, PET_HUNGER_STARVING, PET_HUNGER_STUFFED);

	clif->send_petdata(pd->msd, pd, 2, pd->pet.hungry);
}

/**
 * Calculates the value to store in a pet egg's 4th card slot
 * based on the passed rename flag and intimacy value.
 *
 * @param rename_flag The pet's rename flag.
 * @param intimacy The pet's intimacy value.
 * @return The value to store in the pet egg's 4th card slot. (Defaults to 0 in case of error.)
 *
 **/
static int pet_get_card4_value(int rename_flag, int intimacy)
{
	Assert_ret(rename_flag == 0 || rename_flag == 1);
	Assert_ret(intimacy >= PET_INTIMACY_NONE && intimacy <= PET_INTIMACY_MAX);

	int card4 = rename_flag;

	if (intimacy <= PET_INTIMACY_SHY)
		card4 |= (1 << 1);
	else if (intimacy <= PET_INTIMACY_NEUTRAL)
		card4 |= (2 << 1);
	else if (intimacy <= PET_INTIMACY_CORDIAL)
		card4 |= (3 << 1);
	else if (intimacy <= PET_INTIMACY_LOYAL)
		card4 |= (4 << 1);
	else
		card4 |= (5 << 1);

	return card4;
}

/**
 * Sets a pet's intimacy value.
 * Deletes the pet if its intimacy value reaches PET_INTIMACY_NONE (0).
 *
 * @param pd The pet.
 * @param value The pet's new intimacy value.
 *
 **/
static void pet_set_intimate(struct pet_data *pd, int value)
{
	nullpo_retv(pd);
	nullpo_retv(pd->msd);

	pd->pet.intimate = cap_value(value, PET_INTIMACY_NONE, PET_INTIMACY_MAX);

	struct map_session_data *sd = pd->msd;

	if (pd->pet.intimate == PET_INTIMACY_NONE) { // Pet is lost, delete it.
		int i;

		ARR_FIND(0, sd->status.inventorySize, i, sd->status.inventory[i].card[0] == CARD0_PET
			 && pd->pet.pet_id == MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2]));

		if (i != sd->status.inventorySize)
			pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_EGG);

		if (battle_config.pet_remove_immediately != 0) {
			pet_stop_attack(pd);
			unit->remove_map(&pd->bl, CLR_OUTSIGHT, ALC_MARK);
		}
	} else {
		clif->send_petdata(sd, pd, 1, pd->pet.intimate);
	}

	status_calc_pc(sd, SCO_NONE);
}

/**
 * Creates a pet egg.
 *
 * @param sd The character who tries to create the pet egg.
 * @param item_id The pet egg's item ID.
 * @return 0 on failure, 1 on success.
 *
 **/
static int pet_create_egg(struct map_session_data *sd, int item_id)
{
	nullpo_ret(sd);

	int pet_id = pet->search_petDB_index(item_id, PET_EGG);

	if (pet_id == INDEX_NOT_FOUND) // No pet egg here.
		return 0;

	if (pc->inventoryblank(sd) == 0)  // Inventory full.
		return 0;

	sd->catch_target_class = pet->db[pet_id].class_;
	intif->create_pet(sd->status.account_id, sd->status.char_id, pet->db[pet_id].class_,
			  mob->db(pet->db[pet_id].class_)->lv, pet->db[pet_id].EggID, 0,
			  (short)pet->db[pet_id].intimate, PET_HUNGER_STUFFED,
			  0, 1, pet->db[pet_id].jname);

	return 1;
}

static int pet_unlocktarget(struct pet_data *pd)
{
	nullpo_ret(pd);

	pd->target_id=0;
	pet_stop_attack(pd);
	pet_stop_walking(pd, STOPWALKING_FLAG_FIXPOS);
	return 0;
}

/*==========================================
 * Pet Attack Skill [Skotlex]
 *------------------------------------------*/
static int pet_attackskill(struct pet_data *pd, int target_id)
{
	nullpo_ret(pd);
	if (!battle_config.pet_status_support || !pd->a_skill ||
		(battle_config.pet_equip_required && !pd->pet.equip))
		return 0;

	if (DIFF_TICK(pd->ud.canact_tick, timer->gettick()) > 0)
		return 0;

	if (rnd()%100 < (pd->a_skill->rate +pd->pet.intimate*pd->a_skill->bonusrate/1000)) {
		//Skotlex: Use pet's skill
		int inf;
		struct block_list *bl;

		bl=map->id2bl(target_id);
		if( bl == NULL || pd->bl.m != bl->m || bl->prev == NULL
		 || status->isdead(bl) || !check_distance_bl(&pd->bl, bl, pd->db->range3))
			return 0;

		inf = skill->get_inf(pd->a_skill->id);
		if (inf & INF_GROUND_SKILL)
			unit->skilluse_pos(&pd->bl, bl->x, bl->y, pd->a_skill->id, pd->a_skill->lv);
		else //Offensive self skill? Could be stuff like GX.
			unit->skilluse_id(&pd->bl,(inf&INF_SELF_SKILL) ? pd->bl.id : bl->id, pd->a_skill->id, pd->a_skill->lv);
		return 1; //Skill invoked.
	}
	return 0;
}

/**
 * Checks if a pet can attack a target.
 *
 * @param sd The pet's master.
 * @param bl The pet's target.
 * @param type 0 - Support master when attacking. Not 0 - Support master when being attacked.
 * @return 0 on failure, 1 on success.
 *
 **/
static int pet_target_check(struct map_session_data *sd, struct block_list *bl, int type)
{
	nullpo_ret(sd);
	nullpo_ret(sd->pd);
	nullpo_ret(bl);
	Assert_ret(sd->pd->msd == NULL || sd->pd->msd->pd == sd->pd);

	struct pet_data *pd = sd->pd;

	if ((type == 0 && pd->petDB->attack_rate == 0) || (type != 0 && pd->petDB->defence_attack_rate == 0))
		return 0; // If base rate is 0, there's nothing to do.

	if (bl->type != BL_MOB || bl->prev == NULL || pd->pet.intimate < battle_config.pet_support_min_friendly
	    || pd->pet.hungry <= PET_HUNGER_STARVING || pd->pet.class_ == status->get_class(bl)
	    || pd->bl.m != bl->m || !check_distance_bl(&pd->bl, bl, pd->db->range2)
	    || status->check_skilluse(&pd->bl, bl, 0, 0) == 0) {
		return 0;
	}

	int rate = ((type == 0) ? pd->petDB->attack_rate : pd->petDB->defence_attack_rate) * pd->rate_fix / 1000;

	if (rnd() % 10000 < max(rate, 1) && (pd->target_id == 0 || rnd() % 10000 < pd->petDB->change_target_rate))
		pd->target_id = bl->id;

	return 0;
}

/*==========================================
 * Pet SC Check [Skotlex]
 *------------------------------------------*/
static int pet_sc_check(struct map_session_data *sd, int type)
{
	struct pet_data *pd;

	nullpo_ret(sd);
	pd = sd->pd;

	if( pd == NULL
	||  (battle_config.pet_equip_required && pd->pet.equip == 0)
	||  pd->recovery == NULL
	||  pd->recovery->timer != INVALID_TIMER
	||  pd->recovery->type != type )
		return 1;

	pd->recovery->timer = timer->add(timer->gettick()+pd->recovery->delay*1000,pet->recovery_timer,sd->bl.id,0);

	return 0;
}

/**
 * Updates a pet's hunger value and timer and updates the pet's intimacy value if starving.
 *
 * @param tid The timer ID.
 * @param tick The base amount of ticks to add to the pet's hunger timer. (The timer's current ticks when calling this fuction.)
 * @param id The pet master's account ID.
 * @param data Unused.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_hungry(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = map->id2sd(id);

	if (sd == NULL || sd->status.pet_id == 0 || sd->pd == NULL)
		return 1;

	struct pet_data *pd = sd->pd;

	/**
	 * If HungerDelay is 0, there's nothing to do.
	 * Actually this shouldn't happen, since the timer wasn't added in pet_data_init(), but just to be sure...
	 *
	 **/
	if (pd->petDB->hungry_delay == 0) {
		pet->hungry_timer_delete(pd);
		return 0;
	}

	if (pd->pet_hungry_timer != tid) {
		ShowError("pet_hungry: pet_hungry_timer %d != %d\n", pd->pet_hungry_timer, tid);
		return 1;
	}

	pd->pet_hungry_timer = INVALID_TIMER;

	if (pd->pet.intimate <= PET_INTIMACY_NONE)
		return 1; // You lost the pet already, the rest is irrelevant.

	pet->set_hunger(pd, pd->pet.hungry - pd->petDB->hunger_decrement);

	// Pet auto-feed.
	if (battle_config.feature_enable_pet_autofeed == 1) {
		if (pd->petDB->autofeed == 1 && pd->pet.autofeed == 1 && pd->pet.hungry <= PET_HUNGER_HUNGRY)
			pet->food(sd, pd);
	}

	int interval = pd->petDB->hungry_delay;

	if (pd->pet.hungry == PET_HUNGER_STARVING) {
		pet_stop_attack(pd);
		pet->set_intimate(pd, pd->pet.intimate - pd->petDB->starving_decrement);

		if (sd->pd == NULL)
			return 0;

		status_calc_pet(pd, SCO_NONE);

		if (pd->petDB->starving_delay > 0)
			interval = pd->petDB->starving_delay;
	}

	interval = interval * battle_config.pet_hungry_delay_rate / 100;
	pd->pet_hungry_timer = timer->add(tick + max(interval, 1), pet->hungry, sd->bl.id, 0);

	return 0;
}

/**
 * Retrieves a pet db entry's index.
 *
 * @remark only PET_CLASS is guaranteed to be an unique key. The other search
 *         types will return the first of many entries that match the condition.
 *
 * @param key  The key to look up.
 * @param type The key type (@see enum petdb_key_type)
 * @return the index of the first matching entry or INDEX_NOT_FOUND if no such entry was found.
 */
static int search_petDB_index(int key, enum petdb_key_type type)
{
	for (int i = 0; i < MAX_PET_DB; i++) {
		if (pet->db[i].class_ <= 0)
			continue;
		switch (type) {
		case PET_CLASS:
			if (pet->db[i].class_ == key)
				return i;
			break;
		case PET_CATCH:
			if (pet->db[i].itemID == key)
				return i;
			break;
		case PET_EGG:
			if (pet->db[i].EggID == key)
				return i;
			break;
		case PET_EQUIP:
			if (pet->db[i].AcceID == key)
				return i;
			break;
		case PET_FOOD:
			if (pet->db[i].FoodID == key)
				return i;
			break;
		default:
			return INDEX_NOT_FOUND;
		}
	}
	return INDEX_NOT_FOUND;
}

static int pet_hungry_timer_delete(struct pet_data *pd)
{
	nullpo_ret(pd);
	if(pd->pet_hungry_timer != INVALID_TIMER) {
		timer->delete(pd->pet_hungry_timer,pet->hungry);
		pd->pet_hungry_timer = INVALID_TIMER;
	}

	return 1;
}

/**
 * Makes a pet start performing/dancing.
 *
 * @param sd Unused.
 * @param pd The pet.
 * @return 0 on failure, 1 on success.
 *
 **/
static int pet_performance(struct map_session_data *sd, struct pet_data *pd)
{
	nullpo_ret(pd);

	int val;

	if (pd->pet.intimate > PET_INTIMACY_LOYAL)
		val = (pd->petDB->s_perfor > 0) ? 4 : 3;
	else if (pd->pet.intimate > PET_INTIMACY_CORDIAL) //TODO: This is way too high.
		val = 2;
	else
		val = 1;

	pet_stop_walking(pd, STOPWALKING_FLAG_NONE | (2000 << 8)); // Stop walking for 2 seconds.
	clif->send_petdata(NULL, pd, 4, rnd() % val + 1);
	pet->lootitem_drop(pd, NULL);

	return 1;
}

static int pet_return_egg(struct map_session_data *sd, struct pet_data *pd)
{
	int i;

	nullpo_retr(1, sd);
	nullpo_retr(1, pd);
	pet->lootitem_drop(pd,sd);

	// Pet Evolution
	ARR_FIND(0, sd->status.inventorySize, i, sd->status.inventory[i].card[0] == CARD0_PET &&
			pd->pet.pet_id == MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2]));

	if (i != sd->status.inventorySize) {
		sd->status.inventory[i].attribute &= ~ATTR_BROKEN;
		sd->status.inventory[i].bound = IBT_NONE;
		sd->status.inventory[i].card[3] = pet->get_card4_value(pd->pet.rename_flag, pd->pet.intimate);
		clif->inventoryList(sd);
	} else {
		// The pet egg wasn't found: it was probably hatched with the old system that deleted the egg.
		struct item tmp_item = {0};
		int flag;

		tmp_item.nameid = pd->petDB->EggID;
		tmp_item.identify = 1;
		tmp_item.card[0] = CARD0_PET;
		tmp_item.card[1] = GetWord(pd->pet.pet_id, 0);
		tmp_item.card[2] = GetWord(pd->pet.pet_id, 1);
		tmp_item.card[3] = pet->get_card4_value(pd->pet.rename_flag, pd->pet.intimate);
		if ((flag = pc->additem(sd, &tmp_item, 1, LOG_TYPE_EGG)) != 0) {
			clif->additem(sd, 0, 0, flag);
			map->addflooritem(&sd->bl, &tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
		}
	}
#if PACKETVER >= 20180704
	clif->send_petdata(sd, pd, 6, 0);
#endif
	pd->pet.incubate = 1;
	unit->free(&pd->bl,CLR_OUTSIGHT);

	status_calc_pc(sd,SCO_NONE);
	sd->status.pet_id = 0;

	return 1;
}

/**
 * Initializes a pet.
 *
 * @param sd The pet's master.
 * @param petinfo The pet's status data.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_data_init(struct map_session_data *sd, struct s_pet *petinfo)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, petinfo);
	Assert_retr(1, sd->status.pet_id == 0 || sd->pd == NULL || sd->pd->msd == sd);

	if (sd->status.account_id != petinfo->account_id || sd->status.char_id != petinfo->char_id) {
		sd->status.pet_id = 0;
		return 1;
	}

	if (sd->status.pet_id != petinfo->pet_id) {
		if (sd->status.pet_id != 0) { // Wrong pet? Set incubate to no and send it back for saving.
			petinfo->incubate = 1;
			intif->save_petdata(sd->status.account_id, petinfo);
			sd->status.pet_id = 0;
			return 1;
		}

		sd->status.pet_id = petinfo->pet_id; // The pet_id value was lost? Odd... Restore it.
	}

	int i = pet->search_petDB_index(petinfo->class_, PET_CLASS);

	if (i == INDEX_NOT_FOUND) {
		sd->status.pet_id = 0;
		return 1;
	}

	struct pet_data *pd;

	CREATE(pd, struct pet_data, 1);
	memcpy(&pd->pet, petinfo, sizeof(struct s_pet));
	sd->pd = pd;
	pd->msd = sd;
	pd->petDB = &pet->db[i];
	pd->db = mob->db(pd->petDB->class_);
	pd->bl.type = BL_PET;
	pd->bl.id = npc->get_new_npc_id();
	status->set_viewdata(&pd->bl, pd->petDB->class_);
	unit->dataset(&pd->bl);
	pd->ud.dir = sd->ud.dir;
	pd->bl.m = sd->bl.m;
	pd->bl.x = sd->bl.x;
	pd->bl.y = sd->bl.y;
	unit->calc_pos(&pd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	pd->bl.x = pd->ud.to_x;
	pd->bl.y = pd->ud.to_y;
	map->addiddb(&pd->bl);
	status_calc_pet(pd, SCO_FIRST);
	pd->last_thinktime = timer->gettick();
	pd->state.skillbonus = 0;

	if (pd->petDB->pet_script != NULL && battle_config.pet_status_support == 1)
		script->run_pet(pd->petDB->pet_script, 0, sd->bl.id, 0);

	if (pd->petDB->equip_script != NULL)
		status_calc_pc(sd, SCO_NONE);

	pd->pet_hungry_timer = INVALID_TIMER;

	if (pd->petDB->hungry_delay > 0) {
		int interval = pd->petDB->hungry_delay * battle_config.pet_hungry_delay_rate / 100;
		pd->pet_hungry_timer = timer->add(timer->gettick() + max(interval, 1), pet->hungry, sd->bl.id, 0);
	}

	return 0;
}

/**
 * Spawns a pet.
 *
 * @param sd The pet's master.
 * @param birth_process Whether the pet is spawned during birth process.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_spawn(struct map_session_data *sd, bool birth_process)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, sd->pd);

	if (map->addblock(&sd->pd->bl) != 0 || !clif->spawn(&sd->pd->bl))
		return 1;

	clif->send_petdata(sd, sd->pd, 0, 0);
	clif->send_petdata(sd, sd->pd, 5, battle_config.pet_hair_style);

#if PACKETVER >= 20180704
	if (birth_process)
		clif->send_petdata(sd, sd->pd, 6, 1);
#endif

	clif->send_petstatus(sd);

	return 0;
}

static int pet_birth_process(struct map_session_data *sd, struct s_pet *petinfo)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, petinfo);
	Assert_retr(1, sd->status.pet_id == 0 || sd->pd == 0 || sd->pd->msd == sd);

	if(sd->status.pet_id && petinfo->incubate == 1) {
		sd->status.pet_id = 0;
		return 1;
	}

	petinfo->incubate = 0;
	petinfo->account_id = sd->status.account_id;
	petinfo->char_id = sd->status.char_id;
	sd->status.pet_id = petinfo->pet_id;
	if(pet->data_init(sd, petinfo)) {
		sd->status.pet_id = 0;
		return 1;
	}

	intif->save_petdata(sd->status.account_id,petinfo);
	if (map->save_settings&8)
		chrif->save(sd,0); //is it REALLY Needed to save the char for hatching a pet? [Skotlex]

	if (sd->pd != NULL && sd->bl.prev != NULL) {
		if (pet->spawn(sd, true) != 0)
			return 1;
	}

	Assert_retr(1, sd->status.pet_id == 0 || sd->pd == 0 || sd->pd->msd == sd);

	return 0;
}

static int pet_recv_petdata(int account_id, struct s_pet *p, int flag)
{
	struct map_session_data *sd;

	nullpo_retr(1, p);
	sd = map->id2sd(account_id);
	if(sd == NULL)
		return 1;
	if(flag == 1) {
		sd->status.pet_id = 0;
		return 1;
	}
	if(p->incubate == 1) {
		int i;
		// Get Egg Index
		ARR_FIND(0, sd->status.inventorySize, i, sd->status.inventory[i].card[0] == CARD0_PET &&
			p->pet_id == MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2]));

		if(i == sd->status.inventorySize) {
			ShowError("pet_recv_petdata: Hatching pet (%d:%s) aborted, couldn't find egg in inventory for removal!\n",p->pet_id, p->name);
			sd->status.pet_id = 0;
			return 1;
		}


		if (!pet->birth_process(sd,p)) {
			// Pet Evolution, Hide the egg by setting broken attribute (0x2)  [Asheraf]
			sd->status.inventory[i].attribute |= ATTR_BROKEN;
			// bind the egg to the character to avoid moving it via forged packets [Asheraf]
			sd->status.inventory[i].bound = IBT_CHARACTER;
		}
	} else {
		pet->data_init(sd,p);
		if (sd->pd != NULL && sd->bl.prev != NULL) {
			if (pet->spawn(sd, false) != 0)
				return 1;
		}
	}

	return 0;
}

static int pet_select_egg(struct map_session_data *sd, int egg_index)
{
	nullpo_ret(sd);

	if (egg_index < 0 || egg_index >= sd->status.inventorySize)
		return 0; //Forged packet!

	if(sd->status.inventory[egg_index].card[0] == CARD0_PET)
		intif->request_petdata(sd->status.account_id, sd->status.char_id, MakeDWord(sd->status.inventory[egg_index].card[1], sd->status.inventory[egg_index].card[2]) );
	else
		ShowError("wrong egg item inventory %d\n",egg_index);

	return 0;
}

static int pet_catch_process1(struct map_session_data *sd, int target_class)
{
	nullpo_ret(sd);

	sd->catch_target_class = target_class;
	clif->catch_process(sd);

	return 0;
}

/**
 * Begins the actual process of catching a monster.
 *
 * @param sd The character who tries to catch the monster.
 * @param target_id The monster ID of the pet, which the character tries to catch.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_catch_process2(struct map_session_data *sd, int target_id)
{
	nullpo_retr(1, sd);

	struct mob_data *md = BL_CAST(BL_MOB, map->id2bl(target_id)); //TODO: Why does this not use map->id2md?

	if (md == NULL || md->bl.prev == NULL) { // Invalid inputs/state, abort capture.
		clif->pet_roulette(sd, 0);
		sd->catch_target_class = -1;
		sd->itemid = -1;
		sd->itemindex = -1;
		return 1;
	}

	// catch_target_class == 0 is used for universal lures (except bosses for now). [Skotlex]
	if (sd->catch_target_class == 0 && (md->status.mode & MD_BOSS) == 0)
		sd->catch_target_class = md->class_;

	int i = pet->search_petDB_index(md->class_, PET_CLASS);

	if (i == INDEX_NOT_FOUND || sd->catch_target_class != md->class_) {
		clif->emotion(&md->bl, E_AG); // Mob will do /ag if wrong lure is used on it.
		clif->pet_roulette(sd, 0);
		sd->catch_target_class = -1;
		return 1;
	}

	int pet_catch_rate;
	int capture = pet->db[i].capture;
	int mob_hp_perc = get_percentage(md->status.hp, md->status.max_hp);

	if (battle_config.pet_catch_rate_official_formula == 1) {
		pet_catch_rate = capture * (100 - mob_hp_perc) / 100 + capture;
	} else {
		int lvl_diff_mod = (sd->status.base_level - md->level) * 30;
		int char_luk_mod = sd->battle_status.luk * 20;
		pet_catch_rate = (capture + lvl_diff_mod + char_luk_mod) * (200 - mob_hp_perc) / 100;
	}

	pet_catch_rate = cap_value(pet_catch_rate, 1, 10000) * battle_config.pet_catch_rate / 100;

	if (rnd() % 10000 < pet_catch_rate) {
		unit->remove_map(&md->bl, CLR_OUTSIGHT, ALC_MARK);
		status_kill(&md->bl);
		clif->pet_roulette(sd, 1);
		intif->create_pet(sd->status.account_id, sd->status.char_id, pet->db[i].class_, mob->db(pet->db[i].class_)->lv,
				  pet->db[i].EggID, 0, pet->db[i].intimate, PET_HUNGER_STUFFED, 0, 1, pet->db[i].jname);
		achievement->validate_taming(sd, pet->db[i].class_);
	} else {
		clif->pet_roulette(sd, 0);
		sd->catch_target_class = -1;
	}

	return 0;
}

/**
 * Is invoked _only_ when a new pet has been created is a product of packet 0x3880
 * see mapif_pet_created@int_pet.c for more information
 * Handles new pet data from inter-server and prepares item information
 * to add pet egg
 *
 * pet_id - Should contain pet id otherwise means failure
 * returns true on success
 **/
static bool pet_get_egg(int account_id, int pet_class, int pet_id)
{
	struct map_session_data *sd;
	struct item tmp_item;
	int i = 0, ret = 0;

	if( pet_id == 0 || pet_class == 0 )
		return false;

	sd = map->id2sd(account_id);
	if( sd == NULL )
		return false;

	// i = pet->search_petDB_index(sd->catch_target_class,PET_CLASS);
	// issue: 8150
	// Before this change in cases where more than one pet egg were requested in a short
	// period of time it wasn't possible to know which kind of egg was being requested after
	// the first request. [Panikon]
	i = pet->search_petDB_index(pet_class,PET_CLASS);
	sd->catch_target_class = -1;

	if(i < 0) {
		intif->delete_petdata(pet_id);
		return false;
	}

	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = pet->db[i].EggID;
	tmp_item.identify = 1;
	tmp_item.card[0] = CARD0_PET;
	tmp_item.card[1] = GetWord(pet_id,0);
	tmp_item.card[2] = GetWord(pet_id,1);
	tmp_item.card[3] = pet->get_card4_value(0, pet->db[i].intimate);
	if((ret = pc->additem(sd,&tmp_item,1,LOG_TYPE_PICKDROP_PLAYER))) {
		clif->additem(sd,0,0,ret);
		map->addflooritem(&sd->bl, &tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
	}

	return true;
}

/**
 * Performs selected pet menu option.
 *
 * @param sd The pet's master.
 * @param menunum The selected menu option.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_menu(struct map_session_data *sd, int menunum)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, sd->pd);

	if (sd->status.pet_id == 0 || sd->pd->pet.intimate <= PET_INTIMACY_NONE || sd->pd->pet.incubate != 0)
		return 1; // You lost the pet already.

	struct item_data *egg_id = itemdb->exists(sd->pd->petDB->EggID);

	if (egg_id != NULL) {
		if ((egg_id->flag.trade_restriction & ITR_NODROP) != 0 && pc->inventoryblank(sd) == 0) {
			clif->message(sd->fd, msg_sd(sd, 451)); // You can't return your pet because your inventory is full.
			return 1;
		}
	}

	switch (menunum) {
	case 0:
		clif->send_petstatus(sd);
		break;
	case 1:
		pet->food(sd, sd->pd);
		break;
	case 2:
		pet->performance(sd, sd->pd);
		break;
	case 3:
		pet->return_egg(sd, sd->pd);
		break;
	case 4:
		pet->unequipitem(sd, sd->pd);
		break;
	default: ;
		ShowError("pet_menu: Unexpected menu option: %d\n", menunum);
		return 1;
	}

	return 0;
}

static int pet_change_name(struct map_session_data *sd, const char *name)
{
	int i;
	struct pet_data *pd;
	nullpo_retr(1, sd);
	nullpo_retr(1, name);

	pd = sd->pd;
	if((pd == NULL) || (pd->pet.rename_flag == 1 && !battle_config.pet_rename))
		return 1;

	for(i=0;i<NAME_LENGTH && name[i];i++){
		if( !(name[i]&0xe0) || name[i]==0x7f)
			return 1;
	}

	return intif_rename_pet(sd, name);
}

static int pet_change_name_ack(struct map_session_data *sd, const char *name, int flag)
{
	struct pet_data *pd;
	char *newname = NULL;
	nullpo_ret(sd);
	nullpo_ret(name);
	pd = sd->pd;
	if (pd == NULL) return 0;

	newname = aStrndup(name, NAME_LENGTH-1);
	normalize_name(newname, " ");//bugreport:3032 // FIXME[Haru]: This should be normalized by the inter-server (so that it's const here)

	if (flag == 0 || strlen(newname) == 0) {
		clif->message(sd->fd, msg_sd(sd,280)); // You cannot use this name for your pet.
		clif->send_petstatus(sd); //Send status so client knows oet name change got rejected.
		aFree(newname);
		return 0;
	}
	safestrncpy(pd->pet.name, newname, NAME_LENGTH);
	aFree(newname);
	clif->blname_ack(0,&pd->bl);
	pd->pet.rename_flag = 1;
	return 1;
}

static int pet_equipitem(struct map_session_data *sd, int index)
{
	struct pet_data *pd;
	int nameid;

	nullpo_retr(1, sd);
	pd = sd->pd;
	if (!pd)  return 1;

	nameid = sd->status.inventory[index].nameid;

	if(pd->petDB->AcceID == 0 || nameid != pd->petDB->AcceID || pd->pet.equip != 0) {
		clif->equipitemack(sd,0,0,EIA_FAIL);
		return 1;
	}

	pc->delitem(sd, index, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME);
	pd->pet.equip = nameid;
	status->set_viewdata(&pd->bl, pd->pet.class_); //Updates view_data.
	clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
	if (battle_config.pet_equip_required) {
		//Skotlex: start support timers if need
		int64 tick = timer->gettick();
		if (pd->s_skill && pd->s_skill->timer == INVALID_TIMER) {
			pd->s_skill->timer=timer->add(tick+pd->s_skill->delay*1000, pet->skill_support_timer, sd->bl.id, 0);
		}
		if (pd->bonus && pd->bonus->timer == INVALID_TIMER)
			pd->bonus->timer=timer->add(tick+pd->bonus->delay*1000, pet->skill_bonus_timer, sd->bl.id, 0);
	}

	return 0;
}

static int pet_unequipitem(struct map_session_data *sd, struct pet_data *pd)
{
	struct item tmp_item;
	int nameid,flag;

	nullpo_retr(1, sd);
	nullpo_retr(1, pd);
	if(pd->pet.equip == 0)
		return 1;

	nameid = pd->pet.equip;
	pd->pet.equip = 0;
	status->set_viewdata(&pd->bl, pd->pet.class_);
	clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = nameid;
	tmp_item.identify = 1;
	if((flag = pc->additem(sd,&tmp_item,1,LOG_TYPE_CONSUME))) {
		clif->additem(sd,0,0,flag);
		map->addflooritem(&sd->bl, &tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
	}
	if( battle_config.pet_equip_required )
	{ // Skotlex: halt support timers if needed
		if( pd->state.skillbonus )
		{
			pd->state.skillbonus = 0;
			status_calc_pc(sd,SCO_NONE);
		}
		if (pd->s_skill && pd->s_skill->timer != INVALID_TIMER) {
			timer->delete(pd->s_skill->timer, pet->skill_support_timer);
			pd->s_skill->timer = INVALID_TIMER;
		}
		if( pd->bonus && pd->bonus->timer != INVALID_TIMER )
		{
			timer->delete(pd->bonus->timer, pet->skill_bonus_timer);
			pd->bonus->timer = INVALID_TIMER;
		}
	}

	return 0;
}

/**
 * Feeds a pet and updates its intimacy value.
 *
 * @param sd The pet's master.
 * @param pd The pet.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_food(struct map_session_data *sd, struct pet_data *pd)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, pd);
	Assert_retr(1, sd->status.pet_id == pd->pet.pet_id);

	int i = pc->search_inventory(sd, pd->petDB->FoodID);

	if (i == INDEX_NOT_FOUND) {
		clif->pet_food(sd, pd->petDB->FoodID, 0);
		return 1;
	}

	pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME);

	int intimacy = 0;

	if (pd->pet.hungry >= PET_HUNGER_STUFFED)
		intimacy -= pd->petDB->r_full; // Decrease intimacy by OverFeedDecrement.
	else if (pd->pet.hungry > PET_HUNGER_SATISFIED)
		intimacy -= pd->petDB->r_full / 2; // Decrease intimacy by 50% of OverFeedDecrement.
	else if (pd->pet.hungry > PET_HUNGER_NEUTRAL)
		intimacy -= pd->petDB->r_full * 5 / 100; // Decrease intimacy by 5% of OverFeedDecrement.
	else if (pd->pet.hungry > PET_HUNGER_HUNGRY)
		intimacy += pd->petDB->r_hungry * 75 / 100; // Increase intimacy by 75% of FeedIncrement.
	else if (pd->pet.hungry > PET_HUNGER_VERY_HUNGRY)
		intimacy += pd->petDB->r_hungry; // Increase intimacy by FeedIncrement.
	else
		intimacy += pd->petDB->r_hungry / 2; // Increase intimacy by 50% of FeedIncrement.

	intimacy = intimacy * battle_config.pet_friendly_rate / 100;
	pet->set_intimate(pd, pd->pet.intimate + intimacy);

	if (sd->pd == NULL)
		return 0;

	status_calc_pet(pd, SCO_NONE);
	pet->set_hunger(pd, pd->pet.hungry + pd->petDB->fullness);
	clif->pet_food(sd, pd->petDB->FoodID, 1);

	return 0;
}

static int pet_randomwalk(struct pet_data *pd, int64 tick)
{
	nullpo_ret(pd);
	Assert_ret(pd->msd == 0 || pd->msd->pd == pd);

	if (DIFF_TICK(pd->next_walktime,tick) < 0 && unit->can_move(&pd->bl)) {
		const int retrycount=20;
		int i,c,d=12-pd->move_fail_count;
		if (d < 5)
			d=5;
		for (i = 0; i < retrycount; i++) {
			int r=rnd();
			int x=pd->bl.x+r%(d*2+1)-d;
			int y=pd->bl.y+r/(d*2+1)%(d*2+1)-d;
			if (map->getcell(pd->bl.m, &pd->bl, x, y, CELL_CHKPASS) != 0
			    && unit->walk_toxy(&pd->bl, x, y, 0) == 0) {
				pd->move_fail_count=0;
				break;
			}
			if(i+1>=retrycount){
				pd->move_fail_count++;
				if(pd->move_fail_count>1000){
					ShowWarning("PET can't move. hold position %d, class = %d\n",pd->bl.id,pd->pet.class_);
					pd->move_fail_count=0;
					pd->ud.canmove_tick = tick + 60000;
					return 0;
				}
			}
		}
		for(i=c=0;i<pd->ud.walkpath.path_len;i++){
			if(pd->ud.walkpath.path[i]&1)
				c+=pd->status.speed*MOVE_DIAGONAL_COST/MOVE_COST;
			else
				c+=pd->status.speed;
		}
		pd->next_walktime = tick+rnd()%1000+MIN_RANDOMWALKTIME+c;

		return 1;
	}
	return 0;
}

/**
 * Performs pet's AI actions. (Moving, attacking, etc.)
 *
 * @param pd The pet.
 * @param sd The pet's master.
 * @param tick Timestamp of last support.
 * @return Always 0.
 *
 **/
static int pet_ai_sub_hard(struct pet_data *pd, struct map_session_data *sd, int64 tick)
{
	nullpo_ret(pd);
	nullpo_ret(pd->bl.prev);
	nullpo_ret(sd);
	nullpo_ret(sd->bl.prev);

	if (DIFF_TICK(tick, pd->last_thinktime) < MIN_PETTHINKTIME)
		return 0;

	pd->last_thinktime = tick;

	if (pd->ud.attacktimer != INVALID_TIMER || pd->ud.skilltimer != INVALID_TIMER || pd->bl.m != sd->bl.m)
		return 0;

	if (pd->ud.walktimer != INVALID_TIMER && pd->ud.walkpath.path_pos <= 2)
		return 0; // No thinking when you just started to walk.

	if (pd->pet.intimate <= PET_INTIMACY_NONE) {
		pet->randomwalk(pd, tick); // Pet should just... well, random walk.
		return 0;
	}

	if (!check_distance_bl(&sd->bl, &pd->bl, pd->db->range3)) { // Master too far away. Chase him.
		if (pd->target_id != 0)
			pet->unlocktarget(pd);

		if (pd->ud.walktimer != INVALID_TIMER && pd->ud.target == sd->bl.id)
			return 0; // Already walking to him.

		if (DIFF_TICK(tick, pd->ud.canmove_tick) < 0)
			return 0; // Can't move yet.

		pd->status.speed = max(sd->battle_status.speed / 2, MIN_WALK_SPEED);

		if (unit->walk_tobl(&pd->bl, &sd->bl, 3, 0) != 0)
			pet->randomwalk(pd, tick);

		return 0;
	}

	if (pd->status.speed != pd->petDB->speed) { // Reset speed to normal.
		if (pd->ud.walktimer != INVALID_TIMER)
			return 0; // Wait until the pet finishes walking back to master.

		pd->status.speed = pd->petDB->speed;
		pd->ud.state.speed_changed = 1;
		pd->ud.state.change_walk_target = 1;
	}

	struct block_list *target = NULL;

	if (pd->target_id != 0) {
		target = map->id2bl(pd->target_id);

		if (target == NULL || pd->bl.m != target->m || status->isdead(target) == 1
		    || !check_distance_bl(&pd->bl, target, pd->db->range3)) {
			target = NULL;
			pet->unlocktarget(pd);
		}
	}

	if (target == NULL && pd->loot != NULL && pd->msd != NULL && pc_has_permission(pd->msd, PC_PERM_TRADE)
	    && pd->loot->count < pd->loot->max && DIFF_TICK(tick, pd->ud.canact_tick) > 0) { // Use half the pet's range of sight.
		map->foreachinrange(pet->ai_sub_hard_lootsearch, &pd->bl, pd->db->range2 / 2, BL_ITEM, pd, &target);
	}

	if (target == NULL) { // Just walk around.
		if (check_distance_bl(&sd->bl, &pd->bl, 3))
			return 0; // Already next to master.

		if (pd->ud.walktimer != INVALID_TIMER && check_distance_blxy(&sd->bl, pd->ud.to_x, pd->ud.to_y, 3))
			return 0; // Already walking to him.

		unit->calc_pos(&pd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);

		if (unit->walk_toxy(&pd->bl, pd->ud.to_x, pd->ud.to_y, 0) != 0)
			pet->randomwalk(pd, tick);

		return 0;
	}

	if (pd->ud.target == target->id && (pd->ud.attacktimer != INVALID_TIMER || pd->ud.walktimer != INVALID_TIMER))
		return 0; // Target already locked.

	if (target->type != BL_ITEM) { // Target is enemy. Chase or attack it.
		if (!battle->check_range(&pd->bl, target, pd->status.rhw.range)) { // Chase enemy.
			if (unit->walk_tobl(&pd->bl, target, pd->status.rhw.range, 2) != 0) // Enemy is unreachable.
				pet->unlocktarget(pd);

			return 0;
		}

		unit->attack(&pd->bl, pd->target_id, 1); // Start/continue attacking.
	} else { // Target is item. Attempt looting.
		if (!check_distance_bl(&pd->bl, target, 1)) { // Item is out of range.
			if (unit->walk_tobl(&pd->bl, target, 1, 1) != 0) // Item is unreachable.
				pet->unlocktarget(pd);

			return 0;
		}

		if (pd->loot->count < pd->loot->max) {
			struct flooritem_data *fitem = BL_UCAST(BL_ITEM, target);

			memcpy(&pd->loot->item[pd->loot->count++], &fitem->item_data, sizeof(pd->loot->item[0]));
			pd->loot->weight += itemdb_weight(fitem->item_data.nameid) * fitem->item_data.amount;
			map->clearflooritem(target);
		}

		pet->unlocktarget(pd); // Target is unlocked regardless of whether the item was picked or not.
	}

	return 0;
}

/**
 * Calls pet_ai_sub_hard() for a character's pet if conditions are fulfilled.
 *
 * @param sd The character.
 * @param ap Additional arguments. In this case only the time stamp of pet AI timer execution.
 * @return Always 0.
 *
 **/
static int pet_ai_sub_foreachclient(struct map_session_data *sd, va_list ap)
{
	nullpo_ret(sd);

	int64 tick = va_arg(ap, int64);

	if (sd->bl.prev != NULL && sd->status.pet_id != 0 && sd->pd != NULL && sd->pd->bl.prev != NULL)
		pet->ai_sub_hard(sd->pd, sd, tick);

	return 0;
}

static int pet_ai_hard(int tid, int64 tick, int id, intptr_t data)
{
	map->foreachpc(pet->ai_sub_foreachclient,tick);

	return 0;
}

static int pet_ai_sub_hard_lootsearch(struct block_list *bl, va_list ap)
{
	struct pet_data *pd = va_arg(ap,struct pet_data *);
	struct block_list **target = va_arg(ap,struct block_list**);
	struct flooritem_data *fitem = NULL;
	int sd_charid = 0;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_ITEM);
	fitem = BL_UCAST(BL_ITEM, bl);

	sd_charid = fitem->first_get_charid;

	if(sd_charid && sd_charid != pd->msd->status.char_id)
		return 0;

	if(unit->can_reach_bl(&pd->bl,bl, pd->db->range2, 1, NULL, NULL) &&
		((*target) == NULL || //New target closer than previous one.
		!check_distance_bl(&pd->bl, *target, distance_bl(&pd->bl, bl))))
	{
		(*target) = bl;
		pd->target_id = bl->id;
		return 1;
	}

	return 0;
}

static int pet_delay_item_drop(int tid, int64 tick, int id, intptr_t data)
{
	struct item_drop_list *list;
	struct item_drop *ditem;
	list=(struct item_drop_list *)data;
	ditem = list->item;
	while (ditem) {
		struct item_drop *ditem_prev;
		map->addflooritem(NULL, &ditem->item_data, ditem->item_data.amount,
			list->m, list->x, list->y,
			list->first_charid, list->second_charid, list->third_charid, 0, false);
		ditem_prev = ditem;
		ditem = ditem->next;
		ers_free(pet->item_drop_ers, ditem_prev);
	}
	ers_free(pet->item_drop_list_ers, list);
	return 0;
}

static int pet_lootitem_drop(struct pet_data *pd, struct map_session_data *sd)
{
	int i,flag=0;
	struct item_drop_list *dlist;
	struct item_drop *ditem;
	if(!pd || !pd->loot || !pd->loot->count)
		return 0;
	dlist = ers_alloc(pet->item_drop_list_ers, struct item_drop_list);
	dlist->m = pd->bl.m;
	dlist->x = pd->bl.x;
	dlist->y = pd->bl.y;
	dlist->first_charid = 0;
	dlist->second_charid = 0;
	dlist->third_charid = 0;
	dlist->item = NULL;

	for (i = 0; i < pd->loot->count; i++) {
		struct item *it = &pd->loot->item[i];
		if (sd) {
			if ((flag = pc->additem(sd,it,it->amount,LOG_TYPE_PICKDROP_PLAYER))) {
				clif->additem(sd,0,0,flag);
				ditem = ers_alloc(pet->item_drop_ers, struct item_drop);
				memcpy(&ditem->item_data, it, sizeof(struct item));
				ditem->next = dlist->item;
				dlist->item = ditem;
			}
		} else {
			ditem = ers_alloc(pet->item_drop_ers, struct item_drop);
			memcpy(&ditem->item_data, it, sizeof(struct item));
			ditem->next = dlist->item;
			dlist->item = ditem;
		}
	}
	//The smart thing to do is use pd->loot->max (thanks for pointing it out, Shinomori)
	memset(pd->loot->item,0,pd->loot->max * sizeof(struct item));
	pd->loot->count = 0;
	pd->loot->weight = 0;
	pd->ud.canact_tick = timer->gettick()+10000; //prevent picked up during 10*1000ms

	if (dlist->item)
		timer->add(timer->gettick()+540,pet->delay_item_drop,0,(intptr_t)dlist);
	else
		ers_free(pet->item_drop_list_ers, dlist);
	return 1;
}

/**
 * Applies pet's stat bonuses to its master. (See petskillbonus() script command.)
 *
 * @param tid The timer ID
 * @param tick The base amount of ticks to add to the pet's bonus timer. (The timer's current ticks when calling this fuction.)
 * @param id The pet's master's account ID.
 * @param data Unused.
 * @return 1 on failure, 0 on success.
 *
 **/
static int pet_skill_bonus_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = map->id2sd(id);

	if (sd == NULL || sd->pd == NULL || sd->pd->bonus == NULL)
		return 1;

	struct pet_data *pd = sd->pd;

	if (pd->bonus->timer != tid) {
		ShowError("pet_skill_bonus_timer %d != %d\n", pd->bonus->timer, tid);
		pd->bonus->timer = INVALID_TIMER;
		return 1;
	}

	int bonus;
	int duration;

	// Determine the time for the next timer.
	if (pd->state.skillbonus == 1 && pd->bonus->delay > 0) {
		bonus = 0;
		duration = pd->bonus->delay * 1000; // The duration until pet bonuses will be reactivated again.
	} else if (pd->pet.intimate > PET_INTIMACY_NONE) {
		bonus = 1;
		duration = pd->bonus->duration * 1000; // The duration for pet bonuses to be in effect.
	} else { // Lost pet...
		pd->bonus->timer = INVALID_TIMER;
		return 1;
	}

	if (pd->state.skillbonus != bonus) {
		pd->state.skillbonus = bonus;
		status_calc_pc(sd, SCO_NONE);
	}

	// Wait for the next timer.
	pd->bonus->timer = timer->add(tick + duration, pet->skill_bonus_timer, sd->bl.id, 0);

	return 0;
}

/*==========================================
 * pet recovery skills [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
static int pet_recovery_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd=map->id2sd(id);
	struct pet_data *pd;

	if(sd==NULL || sd->pd == NULL || sd->pd->recovery == NULL)
		return 1;

	pd = sd->pd;
	nullpo_retr(1, pd);

	if(pd->recovery->timer != tid) {
		ShowError("pet_recovery_timer %d != %d\n",pd->recovery->timer,tid);
		return 0;
	}

	if (sd->sc.data[pd->recovery->type]) {
		//Display a heal animation?
		//Detoxify is chosen for now.
		clif->skill_nodamage(&pd->bl,&sd->bl,TF_DETOXIFY,1,1);
		status_change_end(&sd->bl, pd->recovery->type, INVALID_TIMER);
		clif->emotion(&pd->bl, E_OK);
	}

	pd->recovery->timer = INVALID_TIMER;

	return 0;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------*/
static int pet_skill_support_timer(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd=map->id2sd(id);
	struct pet_data *pd;
	struct status_data *st;
	short rate = 100;
	if(sd==NULL || sd->pd == NULL || sd->pd->s_skill == NULL)
		return 1;

	pd=sd->pd;
	nullpo_retr(1, pd);

	if(pd->s_skill->timer != tid) {
		ShowError("pet_skill_support_timer %d != %d\n",pd->s_skill->timer,tid);
		return 0;
	}

	st = status->get_status_data(&sd->bl);

	if (DIFF_TICK(pd->ud.canact_tick, tick) > 0) {
		//Wait until the pet can act again.
		pd->s_skill->timer=timer->add(pd->ud.canact_tick,pet->skill_support_timer,sd->bl.id,0);
		return 0;
	}

	if(pc_isdead(sd) ||
		(rate = get_percentage(st->sp, st->max_sp)) > pd->s_skill->sp ||
		(rate = get_percentage(st->hp, st->max_hp)) > pd->s_skill->hp ||
		(rate = (pd->ud.skilltimer != INVALID_TIMER)) //Another skill is in effect
	) {  //Wait (how long? 1 sec for every 10% of remaining)
		pd->s_skill->timer=timer->add(tick+(rate>10?rate:10)*100,pet->skill_support_timer,sd->bl.id,0);
		return 0;
	}

	pet_stop_attack(pd);
	pet_stop_walking(pd, STOPWALKING_FLAG_FIXPOS);
	pd->s_skill->timer=timer->add(tick+pd->s_skill->delay*1000,pet->skill_support_timer,sd->bl.id,0);
	if (skill->get_inf(pd->s_skill->id) & INF_GROUND_SKILL)
		unit->skilluse_pos(&pd->bl, sd->bl.x, sd->bl.y, pd->s_skill->id, pd->s_skill->lv);
	else
		unit->skilluse_id(&pd->bl, sd->bl.id, pd->s_skill->id, pd->s_skill->lv);
	return 0;
}

static void pet_read_db(void)
{
	const char *filename[] = {
		DBPATH"pet_db.conf",
		"pet_db2.conf"
	};

	pet->read_db_clear();

	for (int i = 0; i < ARRAYLENGTH(filename); ++i) {
		pet->read_db_libconfig(filename[i], i > 0 ? true : false);
	}
}

/**
 * Reads from a libconfig-formatted petdb file and inserts the found entries
 * into the pet database, overwriting duplicate ones (i.e. pet_db2 overriding
 * pet_db.)
 *
 * @param filename       File name, relative to the database path.
 * @param ignore_missing Whether to ignore errors caused by a missing db file.
 * @return the number of found entries.
 */
static int pet_read_db_libconfig(const char *filename, bool ignore_missing)
{
	struct config_t pet_db_conf;
	struct config_setting_t *pdb;
	struct config_setting_t *t;
	char filepath[256];
	bool duplicate[MAX_MOB_DB] = { 0 };
	int i = 0;
	int count = 0;

	nullpo_ret(filename);

	safesnprintf(filepath, sizeof(filepath), "%s/%s", map->db_path, filename);

	if (!exists(filepath)) {
		if (!ignore_missing) {
			ShowError("pet_read_db_libconfig: can't find file %s\n", filepath);
		}
		return 0;
	}

	if (!libconfig->load_file(&pet_db_conf, filepath))
		return 0;

	if ((pdb = libconfig->setting_get_member(pet_db_conf.root, "pet_db")) == NULL) {
		ShowError("can't read %s\n", filepath);
		return 0;
	}

	while ((t = libconfig->setting_get_elem(pdb, i++))) {
		int pet_id = pet->read_db_sub(t, i - 1, filename);

		if (pet_id <= 0 || pet_id >= MAX_MOB_DB)
			continue;

		if (duplicate[pet_id]) {
			ShowWarning("pet_read_db_libconfig:%s: duplicate entry of ID #%d\n", filename, pet_id);
		} else {
			duplicate[pet_id] = true;
		}

		count++;
	}
	libconfig->destroy(&pet_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);

	return count;
}

/**
 * Processes one petdb entry from the libconfig file, loading and inserting it
 * into the pet database.
 *
 * @param it     Libconfig setting entry. It is expected to be valid and it
 *               won't be freed (it is care of the caller to do so if
 *               necessary).
 * @param n      Ordinal number of the entry, to be displayed in case of
 *               validation errors.
 * @param source Source of the entry (file name), to be displayed in case of
 *               validation errors.
 * @return Pet/Mob ID of the validated entry, or 0 in case of failure.
 */
static int pet_read_db_sub(struct config_setting_t *it, int n, const char *source)
{
	nullpo_ret(it);
	nullpo_ret(source);

	struct s_pet_db entry = { 0 };
	int i32 = 0;

	if (libconfig->setting_lookup_int(it, "Id", &i32) == CONFIG_FALSE) {
		ShowWarning("pet_read_db_sub: Missing Id in \"%s\", entry #%d, skipping.\n", source, n);
		return 0;
	}

	if (mob->db_checkid(i32) == 0) {
		ShowWarning("pet_read_db_sub: Invalid Id in \"%s\", entry #%d, skipping.\n", source, n);
		return 0;
	}

	entry.class_ = i32;

	struct config_setting_t *t = NULL;
	bool inherit = false;
	int index = INDEX_NOT_FOUND;

	if ((t = libconfig->setting_get_member(it, "Inherit")) != NULL && (inherit = libconfig->setting_get_bool(t))) {
		index = pet->search_petDB_index(entry.class_, PET_CLASS);
		if (index == INDEX_NOT_FOUND) {
			ShowWarning("pet_read_db_sub: Trying to inherit nonexistent pet %d, defailt values will be used instead.\n", entry.class_);
			inherit = false;
		} else {
			// Use old entry as default
			entry = pet->db[index];
		}
	}
	if (!inherit) {
		ARR_FIND(0, MAX_PET_DB, index, pet->db[index].class_ == 0);
		if (index == MAX_PET_DB) {
			ShowError("pet_read_db_sub: Pet DB exceeds the maximum size of %d. MAX_PET_DB needs to be increased.\n", MAX_PET_DB);
			return 0;
		}
	}

	safestrncpy(entry.name, mob->db(entry.class_)->sprite, sizeof(entry.name));

	const char *str;

	if (libconfig->setting_lookup_string(it, "Name", &str) == CONFIG_FALSE || *str == '\0') {
		if (!inherit) {
			ShowWarning("pet_read_db_sub: Missing Name in pet %d of \"%s\", skipping.\n", entry.class_, source);
			return 0;
		}
	} else {
		safestrncpy(entry.jname, str, sizeof(entry.jname));
	}

	if (libconfig->setting_lookup_string(it, "EggItem", &str) == CONFIG_FALSE || *str == '\0') {
		if (!inherit) {
			ShowWarning("pet_read_db_sub: Missing EggItem in pet %d of \"%s\", skipping.\n", entry.class_, source);
			return 0;
		}
	} else {
		struct item_data *data;
		if ((data = itemdb->name2id(str)) == NULL) {
			if (!inherit) {
				ShowWarning("pet_read_db_sub: Invalid EggItem '%s' in pet %d of \"%s\", skipping.\n", str, entry.class_, source);
				return 0;
			}
		} else {
			entry.EggID = data->nameid;
		}
	}

	if (libconfig->setting_lookup_string(it, "TamingItem", &str) == CONFIG_TRUE) {
		struct item_data *data;
		if ((data = itemdb->name2id(str)) == NULL) {
			ShowWarning("pet_read_db_sub: Invalid TamingItem '%s' in pet %d of \"%s\", defaulting to 0.\n", str, entry.class_, source);
			entry.itemID = 0;
		} else {
			entry.itemID = data->nameid;
		}
	}

	if (libconfig->setting_lookup_string(it, "FoodItem", &str) == CONFIG_TRUE) {
		struct item_data *data;
		if ((data = itemdb->name2id(str)) == NULL) {
			ShowWarning("pet_read_db_sub: Invalid FoodItem '%s' in pet %d of \"%s\", defaulting to Pet_Food (ID=%d).\n",
				    str, entry.class_, source, ITEMID_PET_FOOD);
			entry.FoodID = ITEMID_PET_FOOD;
		} else {
			entry.FoodID = data->nameid;
		}
	} else if (!inherit) {
		entry.FoodID = ITEMID_PET_FOOD;
	}

	if (libconfig->setting_lookup_string(it, "AccessoryItem", &str) == CONFIG_TRUE) {
		struct item_data *data;
		if ((data = itemdb->name2id(str)) == NULL) {
			ShowWarning("pet_read_db_sub: Invalid AccessoryItem '%s' in pet %d of \"%s\", defaulting to 0.\n",
				    str, entry.class_, source);
			entry.AcceID = 0;
		} else {
			entry.AcceID = data->nameid;
		}
	}

	if (libconfig->setting_lookup_int(it, "FoodEffectiveness", &i32) == CONFIG_TRUE)
		entry.fullness = cap_value(i32, 1, PET_HUNGER_STUFFED);
	else if (!inherit)
		entry.fullness = 80;

	if (libconfig->setting_lookup_int(it, "HungerDelay", &i32) == CONFIG_TRUE)
		entry.hungry_delay = cap_value(1000 * i32, 0, INT_MAX);
	else if (!inherit)
		entry.hungry_delay = 60000;

	if (libconfig->setting_lookup_int(it, "HungerDecrement", &i32) == CONFIG_TRUE)
		entry.hunger_decrement = cap_value(i32, PET_HUNGER_STARVING, PET_HUNGER_STUFFED - 1);
	else if (!inherit)
		entry.hunger_decrement = 1;

	if (entry.hunger_decrement == PET_HUNGER_STARVING)
		entry.hungry_delay = 0;

	if (!inherit) {
		/*
		 * Preventively set default intimacy values here, just in case that 'Intimacy' block is not defined,
		 * or pet_read_db_sub_intimacy() fails execution.
		 */
		entry.intimate = PET_INTIMACY_NEUTRAL;
		entry.r_hungry = 10;
		entry.r_full = 100;
		entry.die = 20;
		entry.starving_delay = min(20000, entry.hungry_delay);
		entry.starving_decrement = 20;
	}

	if ((t = libconfig->setting_get_member(it, "Intimacy")) != NULL && config_setting_is_group(t))
		pet->read_db_sub_intimacy(&entry, t);

	if (libconfig->setting_lookup_int(it, "CaptureRate", &i32) == CONFIG_TRUE)
		entry.capture = cap_value(i32, 1, 10000);
	else if (!inherit)
		entry.capture = 1000;

	if (libconfig->setting_lookup_int(it, "Speed", &i32) == CONFIG_TRUE)
		entry.speed = cap_value(i32, MIN_WALK_SPEED, MAX_WALK_SPEED);
	else if (!inherit)
		entry.speed = DEFAULT_WALK_SPEED;

	if ((t = libconfig->setting_get_member(it, "SpecialPerformance")) != NULL
	    && (i32 = libconfig->setting_get_bool(t)) != 0) {
		entry.s_perfor = (char)i32;
	}

	if ((t = libconfig->setting_get_member(it, "TalkWithEmotes")) != NULL
	    && (i32 = libconfig->setting_get_bool(t)) != 0) {
		entry.talk_convert_class = i32;
	}

	if (libconfig->setting_lookup_int(it, "AttackRate", &i32) == CONFIG_TRUE)
		entry.attack_rate = cap_value(i32, 0, 10000);
	else if (!inherit)
		entry.attack_rate = 300;

	if (libconfig->setting_lookup_int(it, "DefendRate", &i32) == CONFIG_TRUE)
		entry.defence_attack_rate = cap_value(i32, 0, 10000);
	else if (!inherit)
		entry.defence_attack_rate = 300;

	if (libconfig->setting_lookup_int(it, "ChangeTargetRate", &i32) == CONFIG_TRUE)
		entry.change_target_rate = cap_value(i32, 0, 10000);
	else if (!inherit)
		entry.change_target_rate = 800;

	if ((t = libconfig->setting_get_member(it, "AutoFeed")) != NULL && (i32 = libconfig->setting_get_bool(t)) != 0)
		entry.autofeed = i32;

	if (libconfig->setting_lookup_string(it, "PetScript", &str) == CONFIG_TRUE && *str != '\0') {
		entry.pet_script = script->parse(str, source, -entry.class_, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL);
	} else if (!inherit) {
		entry.pet_script = NULL;
	}

	if (libconfig->setting_lookup_string(it, "EquipScript", &str) == CONFIG_TRUE && *str != '\0') {
		entry.equip_script = script->parse(str, source, -entry.class_, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL);
	} else if (!inherit) {
		entry.equip_script = NULL;
	}

	if ((t = libconfig->setting_get_member(it, "Evolve")) != NULL && config_setting_is_group(t))
		pet->read_db_sub_evolution(&entry, t);

	// Ready to insert - free existing data if overriding
	if (pet->db[index].pet_script != NULL && pet->db[index].pet_script != entry.pet_script) {
		script->free_code(pet->db[index].pet_script);
		pet->db[index].pet_script = NULL;
	}
	if (pet->db[index].equip_script != NULL && pet->db[index].equip_script != entry.equip_script) {
		script->free_code(pet->db[index].equip_script);
		pet->db[index].equip_script = NULL;
	}

	pet->db[index] = entry;
	return pet->db[index].class_;
}

/**
 * Read Pet Evolution Database [Dastgir/Hercules]
 * @param  entry The pet db entry.
 * @param  t     The libconfig evolution block, to read information from.
 * @return false on failure, true on success.
 */
static bool pet_read_db_sub_evolution(struct s_pet_db *entry, struct config_setting_t *t)
{
	struct config_setting_t *pett;
	int i = 0;
	const char *str = NULL;

	nullpo_retr(false, entry);
	nullpo_retr(false, t);

	VECTOR_INIT(entry->evolve_data);

	while ((pett = libconfig->setting_get_elem(t, i))) {
		if (config_setting_is_group(pett)) {
			struct pet_evolve_data ped;
			struct item_data *data;
			struct config_setting_t *item;
			int j = 0, i32 = 0;

			str = config_setting_name(pett);

			if (!(data = itemdb->name2id(str))) {
				ShowWarning("pet_read_evolve_db_sub: Invalid Egg '%s' in Pet #%d, skipping.\n", str, entry->class_);
				return false;
			} else {
				ped.petEggId = data->nameid;
			}

			VECTOR_INIT(ped.items);

			while ((item = libconfig->setting_get_elem(pett, j))) {
				struct itemlist_entry list = { 0 };
				int quantity = 0;

				str = config_setting_name(item);
				data = itemdb->search_name(str);

				if (!data) {
					ShowWarning("pet_read_evolve_db_sub: required item %s not found in egg %d\n", str, ped.petEggId);
					j++;
					continue;
				}

				list.id = data->nameid;

				if (mob->get_const(item, &i32) && i32 >= 0) {
					quantity = i32;
				}

				if (quantity <= 0) {
					ShowWarning("pet_read_evolve_db_sub: invalid quantity %d for egg %d\n", quantity, ped.petEggId);
					j++;
					continue;
				}

				list.amount = quantity;

				VECTOR_ENSURE(ped.items, 1, 1);
				VECTOR_PUSH(ped.items, list);

				j++;

			}

			VECTOR_ENSURE(entry->evolve_data, 1, 1);
			VECTOR_PUSH(entry->evolve_data, ped);
		}
		i++;
	}
	return true;
}

/**
 * Reads a pet's intimacy data from DB.
 *
 * @param entry The pet db entry.
 * @param t     The libconfig settings block, which contains the pet's intimacy data.
 * @return false on failure, true on success.
 */
static bool pet_read_db_sub_intimacy(struct s_pet_db *entry, struct config_setting_t *t)
{
	nullpo_retr(false, entry);
	nullpo_retr(false, t);

	int i32 = 0;

	if (libconfig->setting_lookup_int(t, "Initial", &i32) == CONFIG_TRUE)
		entry->intimate = cap_value(i32, PET_INTIMACY_AWKWARD, PET_INTIMACY_MAX);

	if (libconfig->setting_lookup_int(t, "FeedIncrement", &i32) == CONFIG_TRUE)
		entry->r_hungry = cap_value(i32, PET_INTIMACY_AWKWARD, PET_INTIMACY_MAX);

	if (libconfig->setting_lookup_int(t, "OverFeedDecrement", &i32) == CONFIG_TRUE)
		entry->r_full = cap_value(i32, PET_INTIMACY_NONE, PET_INTIMACY_MAX);

	if (libconfig->setting_lookup_int(t, "OwnerDeathDecrement", &i32) == CONFIG_TRUE)
		entry->die = cap_value(i32, PET_INTIMACY_NONE, PET_INTIMACY_MAX);

	if (libconfig->setting_lookup_int(t, "StarvingDelay", &i32) == CONFIG_TRUE)
		entry->starving_delay = cap_value(1000 * i32, 0, entry->hungry_delay);

	if (libconfig->setting_lookup_int(t, "StarvingDecrement", &i32) == CONFIG_TRUE)
		entry->starving_decrement = cap_value(i32, PET_INTIMACY_NONE, PET_INTIMACY_MAX);

	if (entry->starving_decrement == PET_INTIMACY_NONE)
		entry->starving_delay = 0;

	return true;
}

static void pet_read_db_clear(void)
{
	int i;

	// Remove any previous scripts in case reloaddb was invoked.
	for (i = 0; i < MAX_PET_DB; i++) {
		int j;
		if (pet->db[i].pet_script) {
			script->free_code(pet->db[i].pet_script);
			pet->db[i].pet_script = NULL;
		}
		if (pet->db[i].equip_script) {
			script->free_code(pet->db[i].equip_script);
			pet->db[i].equip_script = NULL;
		}

		for (j = 0; j < VECTOR_LENGTH(pet->db[i].evolve_data); j++) {
			VECTOR_CLEAR(VECTOR_INDEX(pet->db[i].evolve_data, j).items);
		}
		VECTOR_CLEAR(pet->db[i].evolve_data);
	}
	memset(pet->db, 0, sizeof(pet->db));
	return;
}

/*==========================================
 * Initialization process relationship skills
 *------------------------------------------*/
static int do_init_pet(bool minimal)
{
	if (minimal)
		return 0;

	pet->read_db();

	pet->item_drop_ers = ers_new(sizeof(struct item_drop),"pet.c::item_drop_ers",ERS_OPT_NONE);
	pet->item_drop_list_ers = ers_new(sizeof(struct item_drop_list),"pet.c::item_drop_list_ers",ERS_OPT_NONE);

	timer->add_func_list(pet->hungry,"pet_hungry");
	timer->add_func_list(pet->ai_hard,"pet_ai_hard");
	timer->add_func_list(pet->skill_bonus_timer,"pet_skill_bonus_timer"); // [Valaris]
	timer->add_func_list(pet->delay_item_drop,"pet_delay_item_drop");
	timer->add_func_list(pet->skill_support_timer, "pet_skill_support_timer"); // [Skotlex]
	timer->add_func_list(pet->recovery_timer,"pet_recovery_timer"); // [Valaris]
	timer->add_interval(timer->gettick()+MIN_PETTHINKTIME,pet->ai_hard,0,0,MIN_PETTHINKTIME);

	return 0;
}

static int do_final_pet(void)
{
	int i;
	for( i = 0; i < MAX_PET_DB; i++ )
	{
		int j;
		if( pet->db[i].pet_script )
		{
			script->free_code(pet->db[i].pet_script);
			pet->db[i].pet_script = NULL;
		}
		if( pet->db[i].equip_script )
		{
			script->free_code(pet->db[i].equip_script);
			pet->db[i].equip_script = NULL;
		}

		/* Pet Evolution [Dastgir/Hercules] */
		for (j = 0; j < VECTOR_LENGTH(pet->db[i].evolve_data); j++) {
			VECTOR_CLEAR(VECTOR_INDEX(pet->db[i].evolve_data, j).items);
		}
		VECTOR_CLEAR(pet->db[i].evolve_data);
	}
	ers_destroy(pet->item_drop_ers);
	ers_destroy(pet->item_drop_list_ers);

	return 0;
}
void pet_defaults(void)
{
	pet = &pet_s;

	memset(pet->db,0,sizeof(pet->db));
	pet->item_drop_ers = NULL;
	pet->item_drop_list_ers = NULL;

	pet->init = do_init_pet;
	pet->final = do_final_pet;

	pet->hungry_val = pet_hungry_val;
	pet->set_hunger = pet_set_hunger;
	pet->get_card4_value = pet_get_card4_value;
	pet->set_intimate = pet_set_intimate;
	pet->create_egg = pet_create_egg;
	pet->unlocktarget = pet_unlocktarget;
	pet->attackskill = pet_attackskill;
	pet->target_check = pet_target_check;
	pet->sc_check = pet_sc_check;
	pet->hungry = pet_hungry;
	pet->search_petDB_index = search_petDB_index;
	pet->hungry_timer_delete = pet_hungry_timer_delete;
	pet->performance = pet_performance;
	pet->return_egg = pet_return_egg;
	pet->data_init = pet_data_init;
	pet->spawn = pet_spawn;
	pet->birth_process = pet_birth_process;
	pet->recv_petdata = pet_recv_petdata;
	pet->select_egg = pet_select_egg;
	pet->catch_process1 = pet_catch_process1;
	pet->catch_process2 = pet_catch_process2;
	pet->get_egg = pet_get_egg;
	pet->unequipitem = pet_unequipitem;
	pet->food = pet_food;
	pet->ai_sub_hard_lootsearch = pet_ai_sub_hard_lootsearch;
	pet->menu = pet_menu;
	pet->change_name = pet_change_name;
	pet->change_name_ack = pet_change_name_ack;
	pet->equipitem = pet_equipitem;
	pet->randomwalk = pet_randomwalk;
	pet->ai_sub_hard = pet_ai_sub_hard;
	pet->ai_sub_foreachclient = pet_ai_sub_foreachclient;
	pet->ai_hard = pet_ai_hard;
	pet->delay_item_drop = pet_delay_item_drop;
	pet->lootitem_drop = pet_lootitem_drop;
	pet->skill_bonus_timer = pet_skill_bonus_timer;
	pet->recovery_timer = pet_recovery_timer;
	pet->skill_support_timer = pet_skill_support_timer;

	pet->read_db = pet_read_db;
	pet->read_db_libconfig = pet_read_db_libconfig;
	pet->read_db_sub = pet_read_db_sub;
	pet->read_db_sub_intimacy = pet_read_db_sub_intimacy;
	pet->read_db_clear = pet_read_db_clear;

	pet->read_db_sub_evolution = pet_read_db_sub_evolution;
}
