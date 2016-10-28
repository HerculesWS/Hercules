/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
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

#include "config/core.h" // DBPATH
#include "pet.h"

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

struct pet_interface pet_s;
struct pet_interface *pet;

#define MIN_PETTHINKTIME 100

int pet_hungry_val(struct pet_data *pd)
{
	nullpo_ret(pd);

	if(pd->pet.hungry > 90)
		return 4;
	else if(pd->pet.hungry > 75)
		return 3;
	else if(pd->pet.hungry > 25)
		return 2;
	else if(pd->pet.hungry > 10)
		return 1;
	else
		return 0;
}

void pet_set_intimate(struct pet_data *pd, int value)
{
	int intimate;
	struct map_session_data *sd;

	nullpo_retv(pd);
	intimate = pd->pet.intimate;
	sd = pd->msd;

	pd->pet.intimate = value;
	if( (intimate >= battle_config.pet_equip_min_friendly && pd->pet.intimate < battle_config.pet_equip_min_friendly) || (intimate < battle_config.pet_equip_min_friendly && pd->pet.intimate >= battle_config.pet_equip_min_friendly) )
		status_calc_pc(sd,SCO_NONE);
}

int pet_create_egg(struct map_session_data *sd, int item_id)
{
	int pet_id = pet->search_petDB_index(item_id, PET_EGG);
	nullpo_ret(sd);
	if (pet_id < 0) return 0; //No pet egg here.
	if (!pc->inventoryblank(sd)) return 0; // Inventory full
	sd->catch_target_class = pet->db[pet_id].class_;
	intif->create_pet(sd->status.account_id, sd->status.char_id,
		(short)pet->db[pet_id].class_,
		(short)mob->db(pet->db[pet_id].class_)->lv,
		(short)pet->db[pet_id].EggID, 0,
		(short)pet->db[pet_id].intimate,
		100, 0, 1, pet->db[pet_id].jname);
	return 1;
}

int pet_unlocktarget(struct pet_data *pd)
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
int pet_attackskill(struct pet_data *pd, int target_id) {
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

int pet_target_check(struct map_session_data *sd,struct block_list *bl,int type) {
	struct pet_data *pd;
	int rate;

	nullpo_ret(sd);
	pd = sd->pd;

	Assert_ret(pd->msd == 0 || pd->msd->pd == pd);

	if( bl == NULL || bl->type != BL_MOB || bl->prev == NULL
	 || pd->pet.intimate < battle_config.pet_support_min_friendly
	 || pd->pet.hungry < 1
	 || pd->pet.class_ == status->get_class(bl))
		return 0;

	if( pd->bl.m != bl->m
	 || !check_distance_bl(&pd->bl, bl, pd->db->range2))
		return 0;

	if (!status->check_skilluse(&pd->bl, bl, 0, 0))
		return 0;

	if(!type) {
		rate = pd->petDB->attack_rate;
		rate = rate * pd->rate_fix/1000;
		if(pd->petDB->attack_rate > 0 && rate <= 0)
			rate = 1;
	} else {
		rate = pd->petDB->defence_attack_rate;
		rate = rate * pd->rate_fix/1000;
		if(pd->petDB->defence_attack_rate > 0 && rate <= 0)
			rate = 1;
	}
	if(rnd()%10000 < rate)
	{
		if(pd->target_id == 0 || rnd()%10000 < pd->petDB->change_target_rate)
			pd->target_id = bl->id;
	}

	return 0;
}
/*==========================================
 * Pet SC Check [Skotlex]
 *------------------------------------------*/
int pet_sc_check(struct map_session_data *sd, int type)
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

int pet_hungry(int tid, int64 tick, int id, intptr_t data) {
	struct map_session_data *sd;
	struct pet_data *pd;
	int interval;

	sd=map->id2sd(id);
	if(!sd)
		return 1;

	if(!sd->status.pet_id || !sd->pd)
		return 1;

	pd = sd->pd;
	if(pd->pet_hungry_timer != tid){
		ShowError("pet_hungry_timer %d != %d\n",pd->pet_hungry_timer,tid);
		return 0;
	}
	pd->pet_hungry_timer = INVALID_TIMER;

	if (pd->pet.intimate <= 0)
		return 1; //You lost the pet already, the rest is irrelevant.

	pd->pet.hungry--;
	if( pd->pet.hungry < 0 )
	{
		pet_stop_attack(pd);
		pd->pet.hungry = 0;
		pet->set_intimate(pd, pd->pet.intimate - battle_config.pet_hungry_friendly_decrease);
		if( pd->pet.intimate <= 0 )
		{
			pd->pet.intimate = 0;
			pd->status.speed = pd->db->status.speed;
		}
		status_calc_pet(pd, SCO_NONE);
		clif->send_petdata(sd,pd,1,pd->pet.intimate);
	}
	clif->send_petdata(sd,pd,2,pd->pet.hungry);

	if(battle_config.pet_hungry_delay_rate != 100)
		interval = (pd->petDB->hungry_delay*battle_config.pet_hungry_delay_rate)/100;
	else
		interval = pd->petDB->hungry_delay;
	if(interval <= 0)
		interval = 1;
	pd->pet_hungry_timer = timer->add(tick+interval,pet->hungry,sd->bl.id,0);

	return 0;
}

int search_petDB_index(int key,int type)
{
	int i;

	for( i = 0; i < MAX_PET_DB; i++ )
	{
		if(pet->db[i].class_ <= 0)
			continue;
		switch(type) {
			case PET_CLASS: if(pet->db[i].class_ == key) return i; break;
			case PET_CATCH: if(pet->db[i].itemID == key) return i; break;
			case PET_EGG:   if(pet->db[i].EggID  == key) return i; break;
			case PET_EQUIP: if(pet->db[i].AcceID == key) return i; break;
			case PET_FOOD:  if(pet->db[i].FoodID == key) return i; break;
			default:
				return -1;
		}
	}
	return -1;
}

int pet_hungry_timer_delete(struct pet_data *pd)
{
	nullpo_ret(pd);
	if(pd->pet_hungry_timer != INVALID_TIMER) {
		timer->delete(pd->pet_hungry_timer,pet->hungry);
		pd->pet_hungry_timer = INVALID_TIMER;
	}

	return 1;
}

int pet_performance(struct map_session_data *sd, struct pet_data *pd)
{
	int val;

	nullpo_retr(1, pd);
	if (pd->pet.intimate > 900)
		val = (pd->petDB->s_perfor > 0)? 4:3;
	else if(pd->pet.intimate > 750) //TODO: this is way too high
		val = 2;
	else
		val = 1;

	pet_stop_walking(pd,STOPWALKING_FLAG_NONE | (2000<<8)); // Stop walking for 2000ms
	clif->send_petdata(NULL, pd, 4, rnd()%val + 1);
	pet->lootitem_drop(pd,NULL);
	return 1;
}

int pet_return_egg(struct map_session_data *sd, struct pet_data *pd)
{
	struct item tmp_item;
	int flag;

	nullpo_retr(1, sd);
	nullpo_retr(1, pd);
	pet->lootitem_drop(pd,sd);
	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = pd->petDB->EggID;
	tmp_item.identify = 1;
	tmp_item.card[0] = CARD0_PET;
	tmp_item.card[1] = GetWord(pd->pet.pet_id,0);
	tmp_item.card[2] = GetWord(pd->pet.pet_id,1);
	tmp_item.card[3] = pd->pet.rename_flag;
	if((flag = pc->additem(sd,&tmp_item,1,LOG_TYPE_EGG))) {
		clif->additem(sd,0,0,flag);
		map->addflooritem(&sd->bl, &tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
	}
	pd->pet.incubate = 1;
	unit->free(&pd->bl,CLR_OUTSIGHT);

	status_calc_pc(sd,SCO_NONE);
	sd->status.pet_id = 0;

	return 1;
}

int pet_data_init(struct map_session_data *sd, struct s_pet *petinfo)
{
	struct pet_data *pd;
	int i=0,interval=0;

	nullpo_retr(1, sd);
	nullpo_retr(1, petinfo);
	Assert_retr(1, sd->status.pet_id == 0 || sd->pd == 0 || sd->pd->msd == sd);

	if(sd->status.account_id != petinfo->account_id || sd->status.char_id != petinfo->char_id) {
		sd->status.pet_id = 0;
		return 1;
	}
	if (sd->status.pet_id != petinfo->pet_id) {
		if (sd->status.pet_id) {
			//Wrong pet?? Set incubate to no and send it back for saving.
			petinfo->incubate = 1;
			intif->save_petdata(sd->status.account_id,petinfo);
			sd->status.pet_id = 0;
			return 1;
		}
		//The pet_id value was lost? odd... restore it.
		sd->status.pet_id = petinfo->pet_id;
	}

	i = pet->search_petDB_index(petinfo->class_,PET_CLASS);
	if(i < 0) {
		sd->status.pet_id = 0;
		return 1;
	}
	CREATE(pd, struct pet_data, 1);
	pd->bl.type = BL_PET;
	pd->bl.id = npc->get_new_npc_id();
	sd->pd = pd;

	pd->msd = sd;
	pd->petDB = &pet->db[i];
	pd->db = mob->db(petinfo->class_);
	memcpy(&pd->pet, petinfo, sizeof(struct s_pet));
	status->set_viewdata(&pd->bl, petinfo->class_);
	unit->dataset(&pd->bl);
	pd->ud.dir = sd->ud.dir;

	pd->bl.m = sd->bl.m;
	pd->bl.x = sd->bl.x;
	pd->bl.y = sd->bl.y;
	unit->calc_pos(&pd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	pd->bl.x = pd->ud.to_x;
	pd->bl.y = pd->ud.to_y;

	map->addiddb(&pd->bl);
	status_calc_pet(pd,SCO_FIRST);

	pd->last_thinktime = timer->gettick();
	pd->state.skillbonus = 0;

	if( battle_config.pet_status_support )
		script->run_pet(pet->db[i].pet_script,0,sd->bl.id,0);

	if( pd->petDB ) {
		if( pd->petDB->equip_script )
			status_calc_pc(sd,SCO_NONE);

		if( battle_config.pet_hungry_delay_rate != 100 )
			interval = (pd->petDB->hungry_delay*battle_config.pet_hungry_delay_rate)/100;
		else
			interval = pd->petDB->hungry_delay;
	}

	if( interval <= 0 )
		interval = 1;
	pd->pet_hungry_timer = timer->add(timer->gettick() + interval, pet->hungry, sd->bl.id, 0);
	return 0;
}

int pet_birth_process(struct map_session_data *sd, struct s_pet *petinfo)
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

	if(sd->bl.prev != NULL) {
		map->addblock(&sd->pd->bl);
		clif->spawn(&sd->pd->bl);
		clif->send_petdata(sd,sd->pd, 0,0);
		clif->send_petdata(sd,sd->pd, 5,battle_config.pet_hair_style);
		clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
		clif->send_petstatus(sd);
	}
	Assert_retr(1, sd->status.pet_id == 0 || sd->pd == 0 || sd->pd->msd == sd);

	return 0;
}

int pet_recv_petdata(int account_id,struct s_pet *p,int flag) {
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
		//Delete egg from inventory. [Skotlex]
		for (i = 0; i < MAX_INVENTORY; i++) {
			if(sd->status.inventory[i].card[0] == CARD0_PET &&
				p->pet_id == MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2]))
				break;
		}
		if(i >= MAX_INVENTORY) {
			ShowError("pet_recv_petdata: Hatching pet (%d:%s) aborted, couldn't find egg in inventory for removal!\n",p->pet_id, p->name);
			sd->status.pet_id = 0;
			return 1;
		}
		if (!pet->birth_process(sd,p)) //Pet hatched. Delete egg.
			pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_EGG);
	} else {
		pet->data_init(sd,p);
		if(sd->pd && sd->bl.prev != NULL) {
			map->addblock(&sd->pd->bl);
			clif->spawn(&sd->pd->bl);
			clif->send_petdata(sd,sd->pd,0,0);
			clif->send_petdata(sd,sd->pd,5,battle_config.pet_hair_style);
			clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
			clif->send_petstatus(sd);
		}
	}

	return 0;
}

int pet_select_egg(struct map_session_data *sd,short egg_index)
{
	nullpo_ret(sd);

	if(egg_index < 0 || egg_index >= MAX_INVENTORY)
		return 0; //Forged packet!

	if(sd->status.inventory[egg_index].card[0] == CARD0_PET)
		intif->request_petdata(sd->status.account_id, sd->status.char_id, MakeDWord(sd->status.inventory[egg_index].card[1], sd->status.inventory[egg_index].card[2]) );
	else
		ShowError("wrong egg item inventory %d\n",egg_index);

	return 0;
}

int pet_catch_process1(struct map_session_data *sd,int target_class)
{
	nullpo_ret(sd);

	sd->catch_target_class = target_class;
	clif->catch_process(sd);

	return 0;
}

int pet_catch_process2(struct map_session_data* sd, int target_id) {
	struct mob_data *md = NULL;
	struct block_list *bl = NULL;
	int i = 0, pet_catch_rate = 0;

	nullpo_retr(1, sd);

	bl = map->id2bl(target_id); // TODO: Why does this not use map->id2md?
	md = BL_CAST(BL_MOB, bl);
	if (md == NULL || md->bl.prev == NULL) {
		// Invalid inputs/state, abort capture.
		clif->pet_roulette(sd,0);
		sd->catch_target_class = -1;
		sd->itemid = sd->itemindex = -1;
		return 1;
	}

	//FIXME: delete taming item here, if this was an item-invoked capture and the item was flagged as delay-consume [ultramage]

	i = pet->search_petDB_index(md->class_,PET_CLASS);
	//catch_target_class == 0 is used for universal lures (except bosses for now). [Skotlex]
	if (sd->catch_target_class == 0 && !(md->status.mode&MD_BOSS))
		sd->catch_target_class = md->class_;
	if(i < 0 || sd->catch_target_class != md->class_) {
		clif->emotion(&md->bl, E_AG); //mob will do /ag if wrong lure is used on them.
		clif->pet_roulette(sd,0);
		sd->catch_target_class = -1;
		return 1;
	}

	pet_catch_rate = (pet->db[i].capture + (sd->status.base_level - md->level)*30 + sd->battle_status.luk*20)*(200 - get_percentage(md->status.hp, md->status.max_hp))/100;

	if(pet_catch_rate < 1) pet_catch_rate = 1;
	if(battle_config.pet_catch_rate != 100)
		pet_catch_rate = (pet_catch_rate*battle_config.pet_catch_rate)/100;

	if(rnd()%10000 < pet_catch_rate)
	{
		unit->remove_map(&md->bl,CLR_OUTSIGHT,ALC_MARK);
		status_kill(&md->bl);
		clif->pet_roulette(sd,1);
		intif->create_pet(sd->status.account_id,sd->status.char_id,pet->db[i].class_,mob->db(pet->db[i].class_)->lv,
			pet->db[i].EggID,0,pet->db[i].intimate,100,0,1,pet->db[i].jname);
	}
	else
	{
		clif->pet_roulette(sd,0);
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
bool pet_get_egg(int account_id, short pet_class, int pet_id ) {
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
	tmp_item.card[3] = 0; //New pets are not named.
	if((ret = pc->additem(sd,&tmp_item,1,LOG_TYPE_PICKDROP_PLAYER))) {
		clif->additem(sd,0,0,ret);
		map->addflooritem(&sd->bl, &tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
	}

	return true;
}

int pet_menu(struct map_session_data *sd,int menunum)
{
	struct item_data *egg_id;
	nullpo_ret(sd);
	if (sd->pd == NULL)
		return 1;

	//You lost the pet already.
	if(!sd->status.pet_id || sd->pd->pet.intimate <= 0 || sd->pd->pet.incubate)
		return 1;

	egg_id = itemdb->exists(sd->pd->petDB->EggID);
	if (egg_id) {
		if ((egg_id->flag.trade_restriction&ITR_NODROP) && !pc->inventoryblank(sd)) {
			clif->message(sd->fd, msg_sd(sd,451)); // You can't return your pet because your inventory is full.
			return 1;
		}
	}

	switch(menunum) {
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
	}
	return 0;
}

int pet_change_name(struct map_session_data *sd, const char *name)
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

int pet_change_name_ack(struct map_session_data *sd, const char *name, int flag)
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
	clif->charnameack (0,&pd->bl);
	pd->pet.rename_flag = 1;
	clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
	clif->send_petstatus(sd);
	return 1;
}

int pet_equipitem(struct map_session_data *sd,int index) {
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

int pet_unequipitem(struct map_session_data *sd, struct pet_data *pd) {
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
		map->addflooritem(&sd->bl, &tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
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

int pet_food(struct map_session_data *sd, struct pet_data *pd) {
	int i, food_id;

	nullpo_retr(1, pd);
	food_id = pd->petDB->FoodID;
	i = pc->search_inventory(sd, food_id);
	if(i == INDEX_NOT_FOUND) {
		clif->pet_food(sd, food_id, 0);
		return 1;
	}
	pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME);

	if (pd->pet.hungry > 90) {
		pet->set_intimate(pd, pd->pet.intimate - pd->petDB->r_full);
	} else {
		int add_intimate = 0;
		if (battle_config.pet_friendly_rate != 100)
			add_intimate = (pd->petDB->r_hungry * battle_config.pet_friendly_rate)/100;
		else
			add_intimate = pd->petDB->r_hungry;
		if (pd->pet.hungry > 75) {
			add_intimate = add_intimate >> 1;
			if (add_intimate <= 0)
				add_intimate = 1;
		}
		pet->set_intimate(pd, pd->pet.intimate + add_intimate);
	}
	if (pd->pet.intimate <= 0) {
		pd->pet.intimate = 0;
		pet_stop_attack(pd);
		pd->status.speed = pd->db->status.speed;
	} else if (pd->pet.intimate > 1000) {
		pd->pet.intimate = 1000;
	}
	status_calc_pet(pd, SCO_NONE);
	pd->pet.hungry += pd->petDB->fullness;
	if( pd->pet.hungry > 100 )
		pd->pet.hungry = 100;

	clif->send_petdata(sd,pd,2,pd->pet.hungry);
	clif->send_petdata(sd,pd,1,pd->pet.intimate);
	clif->pet_food(sd,pd->petDB->FoodID,1);

	return 0;
}

int pet_randomwalk(struct pet_data *pd, int64 tick)
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
			if(map->getcell (pd->bl.m, &pd->bl, x, y, CELL_CHKPASS) && unit->walktoxy(&pd->bl, x, y, 0)) {
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

int pet_ai_sub_hard(struct pet_data *pd, struct map_session_data *sd, int64 tick) {
	struct block_list *target = NULL;
	nullpo_ret(pd);

	if(pd->bl.prev == NULL || sd == NULL || sd->bl.prev == NULL)
		return 0;

	if(DIFF_TICK(tick,pd->last_thinktime) < MIN_PETTHINKTIME)
		return 0;
	pd->last_thinktime=tick;

	if(pd->ud.attacktimer != INVALID_TIMER || pd->ud.skilltimer != INVALID_TIMER || pd->bl.m != sd->bl.m)
		return 0;

	if(pd->ud.walktimer != INVALID_TIMER && pd->ud.walkpath.path_pos <= 2)
		return 0; //No thinking when you just started to walk.

	if(pd->pet.intimate <= 0) {
		//Pet should just... well, random walk.
		pet->randomwalk(pd,tick);
		return 0;
	}

	if (!check_distance_bl(&sd->bl, &pd->bl, pd->db->range3)) {
		//Master too far, chase.
		if(pd->target_id)
			pet->unlocktarget(pd);
		if(pd->ud.walktimer != INVALID_TIMER && pd->ud.target == sd->bl.id)
			return 0; //Already walking to him
		if (DIFF_TICK(tick, pd->ud.canmove_tick) < 0)
			return 0; //Can't move yet.
		pd->status.speed = (sd->battle_status.speed>>1);
		if(pd->status.speed <= 0)
			pd->status.speed = 1;
		if (!unit->walktobl(&pd->bl, &sd->bl, 3, 0))
			pet->randomwalk(pd,tick);
		return 0;
	}

	//Return speed to normal.
	if (pd->status.speed != pd->petDB->speed) {
		if (pd->ud.walktimer != INVALID_TIMER)
			return 0; //Wait until the pet finishes walking back to master.
		pd->status.speed = pd->petDB->speed;
		pd->ud.state.change_walk_target = pd->ud.state.speed_changed = 1;
	}

	if (pd->target_id) {
		target= map->id2bl(pd->target_id);
		if (!target || pd->bl.m != target->m || status->isdead(target)
		 || !check_distance_bl(&pd->bl, target, pd->db->range3)
		) {
			target = NULL;
			pet->unlocktarget(pd);
		}
	}

	if(!target && pd->loot && pd->msd && pc_has_permission(pd->msd, PC_PERM_TRADE) && pd->loot->count < pd->loot->max && DIFF_TICK(tick,pd->ud.canact_tick)>0) {
		//Use half the pet's range of sight.
		map->foreachinrange(pet->ai_sub_hard_lootsearch,&pd->bl,
			pd->db->range2/2, BL_ITEM,pd,&target);
	}

	if (!target) {
	//Just walk around.
		if (check_distance_bl(&sd->bl, &pd->bl, 3))
			return 0; //Already next to master.

		if(pd->ud.walktimer != INVALID_TIMER && check_distance_blxy(&sd->bl, pd->ud.to_x,pd->ud.to_y, 3))
			return 0; //Already walking to him

		unit->calc_pos(&pd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
		if(!unit->walktoxy(&pd->bl,pd->ud.to_x,pd->ud.to_y,0))
			pet->randomwalk(pd,tick);

		return 0;
	}

	if(pd->ud.target == target->id &&
		(pd->ud.attacktimer != INVALID_TIMER || pd->ud.walktimer != INVALID_TIMER))
		return 0; //Target already locked.

	if (target->type != BL_ITEM)
	{ //enemy targetted
		if(!battle->check_range(&pd->bl,target,pd->status.rhw.range)) {
			//Chase
			if(!unit->walktobl(&pd->bl, target, pd->status.rhw.range, 2))
				pet->unlocktarget(pd); //Unreachable target.
			return 0;
		}
		//Continuous attack.
		unit->attack(&pd->bl, pd->target_id, 1);
	} else {
		//Item Targeted, attempt loot
		if (!check_distance_bl(&pd->bl, target, 1)) {
			//Out of range
			if(!unit->walktobl(&pd->bl, target, 1, 1)) //Unreachable target.
				pet->unlocktarget(pd);
			return 0;
		} else{
			struct flooritem_data *fitem = BL_UCAST(BL_ITEM, target);
			if(pd->loot->count < pd->loot->max){
				memcpy(&pd->loot->item[pd->loot->count++],&fitem->item_data,sizeof(pd->loot->item[0]));
				pd->loot->weight += itemdb_weight(fitem->item_data.nameid)*fitem->item_data.amount;
				map->clearflooritem(target);
			}
			//Target is unlocked regardless of whether it was picked or not.
			pet->unlocktarget(pd);
		}
	}
	return 0;
}

int pet_ai_sub_foreachclient(struct map_session_data *sd,va_list ap) {
	int64 tick = va_arg(ap,int64);
	nullpo_ret(sd);
	if(sd->status.pet_id && sd->pd)
		pet->ai_sub_hard(sd->pd,sd,tick);

	return 0;
}

int pet_ai_hard(int tid, int64 tick, int id, intptr_t data) {
	map->foreachpc(pet->ai_sub_foreachclient,tick);

	return 0;
}

int pet_ai_sub_hard_lootsearch(struct block_list *bl, va_list ap)
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

int pet_delay_item_drop(int tid, int64 tick, int id, intptr_t data) {
	struct item_drop_list *list;
	struct item_drop *ditem;
	list=(struct item_drop_list *)data;
	ditem = list->item;
	while (ditem) {
		struct item_drop *ditem_prev;
		map->addflooritem(NULL, &ditem->item_data, ditem->item_data.amount,
			list->m, list->x, list->y,
			list->first_charid, list->second_charid, list->third_charid, 0);
		ditem_prev = ditem;
		ditem = ditem->next;
		ers_free(pet->item_drop_ers, ditem_prev);
	}
	ers_free(pet->item_drop_list_ers, list);
	return 0;
}

int pet_lootitem_drop(struct pet_data *pd,struct map_session_data *sd)
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

/*==========================================
 * pet bonus giving skills [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
int pet_skill_bonus_timer(int tid, int64 tick, int id, intptr_t data) {
	struct map_session_data *sd=map->id2sd(id);
	struct pet_data *pd;
	int bonus;
	int duration = 0;

	if(sd == NULL || sd->pd==NULL || sd->pd->bonus == NULL)
		return 1;

	pd=sd->pd;

	if(pd->bonus->timer != tid) {
		ShowError("pet_skill_bonus_timer %d != %d\n",pd->bonus->timer,tid);
		pd->bonus->timer = INVALID_TIMER;
		return 0;
	}

	// determine the time for the next timer
	if (pd->state.skillbonus && pd->bonus->delay > 0) {
		bonus = 0;
		duration = pd->bonus->delay*1000; // the duration until pet bonuses will be reactivated again
	} else if (pd->pet.intimate) {
		bonus = 1;
		duration = pd->bonus->duration*1000; // the duration for pet bonuses to be in effect
	} else { //Lost pet...
		pd->bonus->timer = INVALID_TIMER;
		return 0;
	}

	if (pd->state.skillbonus != bonus) {
		pd->state.skillbonus = bonus;
		status_calc_pc(sd, SCO_NONE);
	}
	// wait for the next timer
	pd->bonus->timer=timer->add(tick+duration,pet->skill_bonus_timer,sd->bl.id,0);
	return 0;
}

/*==========================================
 * pet recovery skills [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
int pet_recovery_timer(int tid, int64 tick, int id, intptr_t data) {
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
int pet_skill_support_timer(int tid, int64 tick, int id, intptr_t data) {
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

/**
 * Loads (or reloads) the pet database.
 */
int read_petdb(void)
{
	const char *filename[] = {
		DBPATH"pet_db.txt",
		"pet_db2.txt",
	};
	int i,j;

	// Remove any previous scripts in case reloaddb was invoked.
	for (j = 0; j < MAX_PET_DB; j++) {
		if (pet->db[j].pet_script) {
			script->free_code(pet->db[j].pet_script);
			pet->db[j].pet_script = NULL;
		}
		if (pet->db[j].equip_script) {
			script->free_code(pet->db[j].equip_script);
			pet->db[j].equip_script = NULL;
		}
	}

	// clear database
	memset(pet->db,0,sizeof(pet->db));

	j = 0; // entry counter
	for (i = 0; i < ARRAYLENGTH(filename); i++) {
		char line[1024];
		int lines, entries;
		FILE *fp;

		sprintf(line, "%s/%s", map->db_path, filename[i]);
		fp=fopen(line,"r");
		if (fp == NULL) {
			if (i == 0)
				ShowError("can't read %s\n",line);
			continue;
		}

		lines = entries = 0;
		while (fgets(line, sizeof(line), fp) && j < MAX_PET_DB) {
			char *str[22], *p;
			int nameid, k;
			lines++;

			if (line[0] == '/' && line[1] == '/')
				continue;
			memset(str, 0, sizeof(str));
			p = line;
			while (ISSPACE(*p))
				++p;
			if (*p == '\0')
				continue; // empty line
			for (k = 0; k < 20; ++k) {
				str[k] = p;
				p = strchr(p,',');
				if (p == NULL)
					break; // comma not found
				*p = '\0';
				++p;
			}

			if (p == NULL) {
				ShowError("read_petdb: Insufficient columns in line %d, skipping.\n", lines);
				continue;
			}

			// Pet Script
			if (*p != '{') {
				ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
				continue;
			}

			str[20] = p;
			p = strstr(p+1,"},");
			if (p == NULL) {
				ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
				continue;
			}
			p[1] = '\0';
			p += 2;

			// Equip Script
			if (*p != '{') {
				ShowError("read_petdb: Invalid format (Equip Script column) in line %d, skipping.\n", lines);
				continue;
			}
			str[21] = p;

			nameid = atoi(str[0]);
			if (nameid <= 0)
				continue;

			if (!mob->db_checkid(nameid)) {
				ShowWarning("pet_db reading: Invalid mob-class %d, pet not read.\n", nameid);
				continue;
			}

			pet->db[j].class_ = nameid;
			safestrncpy(pet->db[j].name,str[1],NAME_LENGTH);
			safestrncpy(pet->db[j].jname,str[2],NAME_LENGTH);
			pet->db[j].itemID=atoi(str[3]);
			pet->db[j].EggID=atoi(str[4]);
			pet->db[j].AcceID=atoi(str[5]);
			pet->db[j].FoodID=atoi(str[6]);
			pet->db[j].fullness=atoi(str[7]);
			pet->db[j].hungry_delay=atoi(str[8])*1000;
			pet->db[j].r_hungry=atoi(str[9]);
			if (pet->db[j].r_hungry <= 0)
				pet->db[j].r_hungry=1;
			pet->db[j].r_full=atoi(str[10]);
			pet->db[j].intimate=atoi(str[11]);
			pet->db[j].die=atoi(str[12]);
			pet->db[j].capture=atoi(str[13]);
			pet->db[j].speed=atoi(str[14]);
			pet->db[j].s_perfor=(char)atoi(str[15]);
			pet->db[j].talk_convert_class=atoi(str[16]);
			pet->db[j].attack_rate=atoi(str[17]);
			pet->db[j].defence_attack_rate=atoi(str[18]);
			pet->db[j].change_target_rate=atoi(str[19]);
			pet->db[j].pet_script = NULL;
			pet->db[j].equip_script = NULL;

			if (*str[20])
				pet->db[j].pet_script = script->parse(str[20], filename[i], lines, 0, NULL);
			if (*str[21])
				pet->db[j].equip_script = script->parse(str[21], filename[i], lines, 0, NULL);

			j++;
			entries++;
		}

		if (j >= MAX_PET_DB)
			ShowWarning("read_petdb: Reached max number of pets [%d]. Remaining pets were not read.\n ", MAX_PET_DB);
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' pets in '"CL_WHITE"%s"CL_RESET"'.\n", entries, filename[i]);
	}
	return 0;
}

/*==========================================
 * Initialization process relationship skills
 *------------------------------------------*/
int do_init_pet(bool minimal) {
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

int do_final_pet(void)
{
	int i;
	for( i = 0; i < MAX_PET_DB; i++ )
	{
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
	}
	ers_destroy(pet->item_drop_ers);
	ers_destroy(pet->item_drop_list_ers);
	return 0;
}
void pet_defaults(void) {
	pet = &pet_s;

	memset(pet->db,0,sizeof(pet->db));
	pet->item_drop_ers = NULL;
	pet->item_drop_list_ers = NULL;

	pet->init = do_init_pet;
	pet->final = do_final_pet;

	pet->hungry_val = pet_hungry_val;
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
	pet->read_db = read_petdb;
}
