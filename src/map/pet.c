// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/db.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/ers.h"

#include "pc.h"
#include "status.h"
#include "map.h"
#include "path.h"
#include "intif.h"
#include "clif.h"
#include "chrif.h"
#include "pet.h"
#include "itemdb.h"
#include "battle.h"
#include "mob.h"
#include "npc.h"
#include "script.h"
#include "skill.h"
#include "unit.h"
#include "atcommand.h" // msg_txt()
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MIN_PETTHINKTIME 100

struct s_pet_db pet_db[MAX_PET_DB];

static struct eri *item_drop_ers; //For loot drops delay structures.
static struct eri *item_drop_list_ers;

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
		status_calc_pc(sd,0);
}

int pet_create_egg(struct map_session_data *sd, int item_id)
{
	int pet_id = search_petDB_index(item_id, PET_EGG);
	if (pet_id < 0) return 0; //No pet egg here.
	if (!pc->inventoryblank(sd)) return 0; // Inventory full
	sd->catch_target_class = pet_db[pet_id].class_;
	intif_create_pet(sd->status.account_id, sd->status.char_id,
		(short)pet_db[pet_id].class_,
		(short)mob_db(pet_db[pet_id].class_)->lv,
		(short)pet_db[pet_id].EggID, 0,
		(short)pet_db[pet_id].intimate,
		100, 0, 1, pet_db[pet_id].jname);
	return 1;
}

int pet_unlocktarget(struct pet_data *pd)
{
	nullpo_ret(pd);

	pd->target_id=0;
	pet_stop_attack(pd);
	pet_stop_walking(pd,1);
	return 0;
}

/*==========================================
 * Pet Attack Skill [Skotlex]
 *------------------------------------------*/
int pet_attackskill(struct pet_data *pd, int target_id)
{
	if (!battle_config.pet_status_support || !pd->a_skill || 
		(battle_config.pet_equip_required && !pd->pet.equip))
		return 0;

	if (DIFF_TICK(pd->ud.canact_tick, iTimer->gettick()) > 0)
		return 0;
	
	if (rnd()%100 < (pd->a_skill->rate +pd->pet.intimate*pd->a_skill->bonusrate/1000))
	{	//Skotlex: Use pet's skill 
		int inf;
		struct block_list *bl;

		bl=iMap->id2bl(target_id);
		if(bl == NULL || pd->bl.m != bl->m || bl->prev == NULL || iStatus->isdead(bl) ||
			!check_distance_bl(&pd->bl, bl, pd->db->range3))
			return 0;

		inf = skill->get_inf(pd->a_skill->id);
		if (inf & INF_GROUND_SKILL)
			unit_skilluse_pos(&pd->bl, bl->x, bl->y, pd->a_skill->id, pd->a_skill->lv);
		else	//Offensive self skill? Could be stuff like GX.
			unit_skilluse_id(&pd->bl,(inf&INF_SELF_SKILL?pd->bl.id:bl->id), pd->a_skill->id, pd->a_skill->lv);
		return 1; //Skill invoked.
	}
	return 0;
}

int pet_target_check(struct map_session_data *sd,struct block_list *bl,int type)
{
	struct pet_data *pd;
	int rate;

	pd = sd->pd;
	
	Assert((pd->msd == 0) || (pd->msd->pd == pd));

	if(bl == NULL || bl->type != BL_MOB || bl->prev == NULL ||
		pd->pet.intimate < battle_config.pet_support_min_friendly ||
		pd->pet.hungry < 1 ||
		pd->pet.class_ == iStatus->get_class(bl))
		return 0;

	if(pd->bl.m != bl->m ||
		!check_distance_bl(&pd->bl, bl, pd->db->range2))
		return 0;

	if (!iStatus->check_skilluse(&pd->bl, bl, 0, 0))
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

	pd->recovery->timer = iTimer->add_timer(iTimer->gettick()+pd->recovery->delay*1000,pet_recovery_timer,sd->bl.id,0);
	
	return 0;
}

static int pet_hungry(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct pet_data *pd;
	int interval;

	sd=iMap->id2sd(id);
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
		pet_set_intimate(pd, pd->pet.intimate - battle_config.pet_hungry_friendly_decrease);
		if( pd->pet.intimate <= 0 )
		{
			pd->pet.intimate = 0;
			pd->status.speed = pd->db->status.speed;
		}
		status_calc_pet(pd, 0);
		clif->send_petdata(sd,pd,1,pd->pet.intimate);
	}
	clif->send_petdata(sd,pd,2,pd->pet.hungry);

	if(battle_config.pet_hungry_delay_rate != 100)
		interval = (pd->petDB->hungry_delay*battle_config.pet_hungry_delay_rate)/100;
	else
		interval = pd->petDB->hungry_delay;
	if(interval <= 0)
		interval = 1;
	pd->pet_hungry_timer = iTimer->add_timer(tick+interval,pet_hungry,sd->bl.id,0);

	return 0;
}

int search_petDB_index(int key,int type)
{
	int i;

	for( i = 0; i < MAX_PET_DB; i++ )
	{
		if(pet_db[i].class_ <= 0)
			continue;
		switch(type) {
			case PET_CLASS: if(pet_db[i].class_ == key) return i; break;
			case PET_CATCH: if(pet_db[i].itemID == key) return i; break;
			case PET_EGG:   if(pet_db[i].EggID  == key) return i; break;
			case PET_EQUIP: if(pet_db[i].AcceID == key) return i; break;
			case PET_FOOD:  if(pet_db[i].FoodID == key) return i; break;
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
		iTimer->delete_timer(pd->pet_hungry_timer,pet_hungry);
		pd->pet_hungry_timer = INVALID_TIMER;
	}

	return 1;
}

static int pet_performance(struct map_session_data *sd, struct pet_data *pd)
{
	int val;

	if (pd->pet.intimate > 900)
		val = (pd->petDB->s_perfor > 0)? 4:3;
	else if(pd->pet.intimate > 750) //TODO: this is way too high
		val = 2;
	else
		val = 1;

	pet_stop_walking(pd,2000<<8);
	clif->send_petdata(NULL, pd, 4, rnd()%val + 1);
	pet_lootitem_drop(pd,NULL);
	return 1;
}

static int pet_return_egg(struct map_session_data *sd, struct pet_data *pd)
{
	struct item tmp_item;
	int flag;

	pet_lootitem_drop(pd,sd);
	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = pd->petDB->EggID;
	tmp_item.identify = 1;
	tmp_item.card[0] = CARD0_PET;
	tmp_item.card[1] = GetWord(pd->pet.pet_id,0);
	tmp_item.card[2] = GetWord(pd->pet.pet_id,1);
	tmp_item.card[3] = pd->pet.rename_flag;
	if((flag = pc->additem(sd,&tmp_item,1,LOG_TYPE_OTHER))) {
		clif->additem(sd,0,0,flag);
		iMap->addflooritem(&tmp_item,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
	}
	pd->pet.incuvate = 1;
	unit_free(&pd->bl,CLR_OUTSIGHT);

	status_calc_pc(sd,0);
	sd->status.pet_id = 0;

	return 1;
}

int pet_data_init(struct map_session_data *sd, struct s_pet *pet)
{
	struct pet_data *pd;
	int i=0,interval=0;

	nullpo_retr(1, sd);

	Assert((sd->status.pet_id == 0 || sd->pd == 0) || sd->pd->msd == sd); 

	if(sd->status.account_id != pet->account_id || sd->status.char_id != pet->char_id) {
		sd->status.pet_id = 0;
		return 1;
	}
	if (sd->status.pet_id != pet->pet_id) {
		if (sd->status.pet_id) {
			//Wrong pet?? Set incuvate to no and send it back for saving.
			pet->incuvate = 1;
			intif_save_petdata(sd->status.account_id,pet);
			sd->status.pet_id = 0;
			return 1;
		}
		//The pet_id value was lost? odd... restore it.
		sd->status.pet_id = pet->pet_id;
	}
	
	i = search_petDB_index(pet->class_,PET_CLASS);
	if(i < 0) {
		sd->status.pet_id = 0;
		return 1;
	}
	sd->pd = pd = (struct pet_data *)aCalloc(1,sizeof(struct pet_data));
	pd->bl.type = BL_PET;
	pd->bl.id = npc_get_new_npc_id();

	pd->msd = sd;
	pd->petDB = &pet_db[i];
	pd->db = mob_db(pet->class_);
	memcpy(&pd->pet, pet, sizeof(struct s_pet));
	iStatus->set_viewdata(&pd->bl, pet->class_);
	unit_dataset(&pd->bl);
	pd->ud.dir = sd->ud.dir;

	pd->bl.m = sd->bl.m;
	pd->bl.x = sd->bl.x;
	pd->bl.y = sd->bl.y;
	unit_calc_pos(&pd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	pd->bl.x = pd->ud.to_x;
	pd->bl.y = pd->ud.to_y;

	iMap->addiddb(&pd->bl);
	status_calc_pet(pd,1);

	pd->last_thinktime = iTimer->gettick();
	pd->state.skillbonus = 0;
	if( battle_config.pet_status_support )
		run_script(pet_db[i].pet_script,0,sd->bl.id,0);
	if( pd->petDB && pd->petDB->equip_script )
		status_calc_pc(sd,0);

	if( battle_config.pet_hungry_delay_rate != 100 )
		interval = (pd->petDB->hungry_delay*battle_config.pet_hungry_delay_rate)/100;
	else
		interval = pd->petDB->hungry_delay;
	if( interval <= 0 )
		interval = 1;
	pd->pet_hungry_timer = iTimer->add_timer(iTimer->gettick() + interval, pet_hungry, sd->bl.id, 0);
	return 0;
}

int pet_birth_process(struct map_session_data *sd, struct s_pet *pet)
{
	nullpo_retr(1, sd);

	Assert((sd->status.pet_id == 0 || sd->pd == 0) || sd->pd->msd == sd); 

	if(sd->status.pet_id && pet->incuvate == 1) {
		sd->status.pet_id = 0;
		return 1;
	}

	pet->incuvate = 0;
	pet->account_id = sd->status.account_id;
	pet->char_id = sd->status.char_id;
	sd->status.pet_id = pet->pet_id;
	if(pet_data_init(sd, pet)) {
		sd->status.pet_id = 0;
		return 1;
	}

	intif_save_petdata(sd->status.account_id,pet);
	if (iMap->save_settings&8)
		chrif->save(sd,0); //is it REALLY Needed to save the char for hatching a pet? [Skotlex]

	if(sd->bl.prev != NULL) {
		iMap->addblock(&sd->pd->bl);
		clif->spawn(&sd->pd->bl);
		clif->send_petdata(sd,sd->pd, 0,0);
		clif->send_petdata(sd,sd->pd, 5,battle_config.pet_hair_style);
		clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
		clif->send_petstatus(sd);
	}
	Assert((sd->status.pet_id == 0 || sd->pd == 0) || sd->pd->msd == sd); 

	return 0;
}

int pet_recv_petdata(int account_id,struct s_pet *p,int flag)
{
	struct map_session_data *sd;

	sd = iMap->id2sd(account_id);
	if(sd == NULL)
		return 1;
	if(flag == 1) {
		sd->status.pet_id = 0;
		return 1;
	}
	if(p->incuvate == 1) {
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
		if (!pet_birth_process(sd,p)) //Pet hatched. Delete egg.
			pc->delitem(sd,i,1,0,0,LOG_TYPE_OTHER);
	} else {
		pet_data_init(sd,p);
		if(sd->pd && sd->bl.prev != NULL) {
			iMap->addblock(&sd->pd->bl);
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
		intif_request_petdata(sd->status.account_id, sd->status.char_id, MakeDWord(sd->status.inventory[egg_index].card[1], sd->status.inventory[egg_index].card[2]) );
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

int pet_catch_process2(struct map_session_data* sd, int target_id)
{
	struct mob_data* md;
	int i = 0, pet_catch_rate = 0;

	nullpo_retr(1, sd);

	md = (struct mob_data*)iMap->id2bl(target_id);
	if(!md || md->bl.type != BL_MOB || md->bl.prev == NULL)
	{	// Invalid inputs/state, abort capture.
		clif->pet_roulette(sd,0);
		sd->catch_target_class = -1;
		sd->itemid = sd->itemindex = -1;
		return 1;
	}

	//FIXME: delete taming item here, if this was an item-invoked capture and the item was flagged as delay-consume [ultramage]

	i = search_petDB_index(md->class_,PET_CLASS);
	//catch_target_class == 0 is used for universal lures (except bosses for now). [Skotlex]
	if (sd->catch_target_class == 0 && !(md->status.mode&MD_BOSS))
		sd->catch_target_class = md->class_;
	if(i < 0 || sd->catch_target_class != md->class_) {
		clif->emotion(&md->bl, E_AG);	//mob will do /ag if wrong lure is used on them.
		clif->pet_roulette(sd,0);
		sd->catch_target_class = -1;
		return 1;
	}

	pet_catch_rate = (pet_db[i].capture + (sd->status.base_level - md->level)*30 + sd->battle_status.luk*20)*(200 - get_percentage(md->status.hp, md->status.max_hp))/100;

	if(pet_catch_rate < 1) pet_catch_rate = 1;
	if(battle_config.pet_catch_rate != 100)
		pet_catch_rate = (pet_catch_rate*battle_config.pet_catch_rate)/100;

	if(rnd()%10000 < pet_catch_rate)
	{
		unit_remove_map(&md->bl,CLR_OUTSIGHT);
		status_kill(&md->bl);
		clif->pet_roulette(sd,1);
		intif_create_pet(sd->status.account_id,sd->status.char_id,pet_db[i].class_,mob_db(pet_db[i].class_)->lv,
			pet_db[i].EggID,0,pet_db[i].intimate,100,0,1,pet_db[i].jname);
	}
	else
	{
		clif->pet_roulette(sd,0);
		sd->catch_target_class = -1;
	}

	return 0;
}

int pet_get_egg(int account_id,int pet_id,int flag)
{	//This function is invoked when a new pet has been created, and at no other time!
	struct map_session_data *sd;
	struct item tmp_item;
	int i=0,ret=0;

	if(flag)
		return 0;
		
	sd = iMap->id2sd(account_id);
	if(sd == NULL)
		return 0;

	i = search_petDB_index(sd->catch_target_class,PET_CLASS);
	sd->catch_target_class = -1;
	
	if(i < 0) {
		intif_delete_petdata(pet_id);
		return 0;
	}

	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = pet_db[i].EggID;
	tmp_item.identify = 1;
	tmp_item.card[0] = CARD0_PET;
	tmp_item.card[1] = GetWord(pet_id,0);
	tmp_item.card[2] = GetWord(pet_id,1);
	tmp_item.card[3] = 0; //New pets are not named.
	if((ret = pc->additem(sd,&tmp_item,1,LOG_TYPE_PICKDROP_PLAYER))) {
		clif->additem(sd,0,0,ret);
		iMap->addflooritem(&tmp_item,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
	}

	return 1;
}

static int pet_unequipitem(struct map_session_data *sd, struct pet_data *pd);
static int pet_food(struct map_session_data *sd, struct pet_data *pd);
static int pet_ai_sub_hard_lootsearch(struct block_list *bl,va_list ap);

int pet_menu(struct map_session_data *sd,int menunum)
{
	struct item_data *egg_id;
	nullpo_ret(sd);
	if (sd->pd == NULL)
		return 1;
	
	//You lost the pet already.
	if(!sd->status.pet_id || sd->pd->pet.intimate <= 0 || sd->pd->pet.incuvate)
		return 1;
		
	egg_id = itemdb->exists(sd->pd->petDB->EggID);
	if (egg_id) {
		if ((egg_id->flag.trade_restriction&0x01) && !pc->inventoryblank(sd)) {
			clif->message(sd->fd, msg_txt(451)); // You can't return your pet because your inventory is full.
			return 1;
		}
	}
	
	switch(menunum) {
		case 0:
			clif->send_petstatus(sd);
			break;
		case 1:
			pet_food(sd, sd->pd);
			break;
		case 2:
			pet_performance(sd, sd->pd);
			break;
		case 3:
			pet_return_egg(sd, sd->pd);
			break;
		case 4:
			pet_unequipitem(sd, sd->pd);
			break;
	}
	return 0;
}

int pet_change_name(struct map_session_data *sd,char *name)
{
	int i;
	struct pet_data *pd;
	nullpo_retr(1, sd);

	pd = sd->pd;
	if((pd == NULL) || (pd->pet.rename_flag == 1 && !battle_config.pet_rename))
		return 1;

	for(i=0;i<NAME_LENGTH && name[i];i++){
		if( !(name[i]&0xe0) || name[i]==0x7f)
			return 1;
	}

	return intif_rename_pet(sd, name);
}

int pet_change_name_ack(struct map_session_data *sd, char* name, int flag)
{
	struct pet_data *pd = sd->pd;
	if (!pd) return 0;

	normalize_name(name," ");//bugreport:3032

	if ( !flag || !strlen(name) ) {
		clif->message(sd->fd, msg_txt(280)); // You cannot use this name for your pet.
		clif->send_petstatus(sd); //Send status so client knows oet name change got rejected.
		return 0;
	}
	memcpy(pd->pet.name, name, NAME_LENGTH);
	clif->charnameack (0,&pd->bl);
	pd->pet.rename_flag = 1;
	clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
	clif->send_petstatus(sd);
	return 1;
}

int pet_equipitem(struct map_session_data *sd,int index)
{
	struct pet_data *pd;
	int nameid;

	nullpo_retr(1, sd);
	pd = sd->pd;
	if (!pd)  return 1;
	
	nameid = sd->status.inventory[index].nameid;
	
	if(pd->petDB->AcceID == 0 || nameid != pd->petDB->AcceID || pd->pet.equip != 0) {
		clif->equipitemack(sd,0,0,0);
		return 1;
	}

	pc->delitem(sd,index,1,0,0,LOG_TYPE_OTHER);
	pd->pet.equip = nameid;
	iStatus->set_viewdata(&pd->bl, pd->pet.class_); //Updates view_data.
	clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
	if (battle_config.pet_equip_required)
	{ 	//Skotlex: start support timers if need
		unsigned int tick = iTimer->gettick();
		if (pd->s_skill && pd->s_skill->timer == INVALID_TIMER)
		{
			if (pd->s_skill->id)
				pd->s_skill->timer=iTimer->add_timer(tick+pd->s_skill->delay*1000, pet_skill_support_timer, sd->bl.id, 0);
			else
				pd->s_skill->timer=iTimer->add_timer(tick+pd->s_skill->delay*1000, pet_heal_timer, sd->bl.id, 0);
		}
		if (pd->bonus && pd->bonus->timer == INVALID_TIMER)
			pd->bonus->timer=iTimer->add_timer(tick+pd->bonus->delay*1000, pet_skill_bonus_timer, sd->bl.id, 0);
	}

	return 0;
}

static int pet_unequipitem(struct map_session_data *sd, struct pet_data *pd)
{
	struct item tmp_item;
	int nameid,flag;

	if(pd->pet.equip == 0)
		return 1;

	nameid = pd->pet.equip;
	pd->pet.equip = 0;
	iStatus->set_viewdata(&pd->bl, pd->pet.class_);
	clif->send_petdata(NULL, sd->pd, 3, sd->pd->vd.head_bottom);
	memset(&tmp_item,0,sizeof(tmp_item));
	tmp_item.nameid = nameid;
	tmp_item.identify = 1;
	if((flag = pc->additem(sd,&tmp_item,1,LOG_TYPE_OTHER))) {
		clif->additem(sd,0,0,flag);
		iMap->addflooritem(&tmp_item,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
	}
	if( battle_config.pet_equip_required )
	{ // Skotlex: halt support timers if needed
		if( pd->state.skillbonus )
		{
			pd->state.skillbonus = 0;
			status_calc_pc(sd,0);
		}
		if( pd->s_skill && pd->s_skill->timer != INVALID_TIMER )
		{
			if( pd->s_skill->id )
				iTimer->delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				iTimer->delete_timer(pd->s_skill->timer, pet_heal_timer);
			pd->s_skill->timer = INVALID_TIMER;
		}
		if( pd->bonus && pd->bonus->timer != INVALID_TIMER )
		{
			iTimer->delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
			pd->bonus->timer = INVALID_TIMER;
		}
	}

	return 0;
}

static int pet_food(struct map_session_data *sd, struct pet_data *pd)
{
	int i,k;

	k=pd->petDB->FoodID;
	i=pc->search_inventory(sd,k);
	if(i < 0) {
		clif->pet_food(sd,k,0);
		return 1;
	}
	pc->delitem(sd,i,1,0,0,LOG_TYPE_CONSUME);

	if( pd->pet.hungry > 90 )
		pet_set_intimate(pd, pd->pet.intimate - pd->petDB->r_full);
	else
	{
		if( battle_config.pet_friendly_rate != 100 )
			k = (pd->petDB->r_hungry * battle_config.pet_friendly_rate)/100;
		else
			k = pd->petDB->r_hungry;
		if( pd->pet.hungry > 75 )
		{
			k = k >> 1;
			if( k <= 0 )
				k = 1;
		}
		pet_set_intimate(pd, pd->pet.intimate + k);
	}
	if( pd->pet.intimate <= 0 )
	{
		pd->pet.intimate = 0;
		pet_stop_attack(pd);
		pd->status.speed = pd->db->status.speed;
	}
	else if( pd->pet.intimate > 1000 )
		pd->pet.intimate = 1000;
	status_calc_pet(pd, 0);
	pd->pet.hungry += pd->petDB->fullness;
	if( pd->pet.hungry > 100 )
		pd->pet.hungry = 100;

	clif->send_petdata(sd,pd,2,pd->pet.hungry);
	clif->send_petdata(sd,pd,1,pd->pet.intimate);
	clif->pet_food(sd,pd->petDB->FoodID,1);

	return 0;
}

static int pet_randomwalk(struct pet_data *pd,unsigned int tick)
{
	nullpo_ret(pd);

	Assert((pd->msd == 0) || (pd->msd->pd == pd));

	if(DIFF_TICK(pd->next_walktime,tick) < 0 && unit_can_move(&pd->bl)) {
		const int retrycount=20;
		int i,x,y,c,d=12-pd->move_fail_count;
		if(d<5) d=5;
		for(i=0;i<retrycount;i++){
			int r=rnd();
			x=pd->bl.x+r%(d*2+1)-d;
			y=pd->bl.y+r/(d*2+1)%(d*2+1)-d;
			if(iMap->getcell(pd->bl.m,x,y,CELL_CHKPASS) && unit_walktoxy(&pd->bl,x,y,0)){
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
		pd->next_walktime = tick+rnd()%3000+3000+c;

		return 1;
	}
	return 0;
}

static int pet_ai_sub_hard(struct pet_data *pd, struct map_session_data *sd, unsigned int tick)
{
	struct block_list *target = NULL;

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
		pet_randomwalk(pd,tick);
		return 0;
	}
	
	if (!check_distance_bl(&sd->bl, &pd->bl, pd->db->range3)) {
		//Master too far, chase.
		if(pd->target_id)
			pet_unlocktarget(pd);
		if(pd->ud.walktimer != INVALID_TIMER && pd->ud.target == sd->bl.id)
			return 0; //Already walking to him
		if (DIFF_TICK(tick, pd->ud.canmove_tick) < 0)
			return 0; //Can't move yet.
		pd->status.speed = (sd->battle_status.speed>>1);
		if(pd->status.speed <= 0)
			pd->status.speed = 1;
		if (!unit_walktobl(&pd->bl, &sd->bl, 3, 0))
			pet_randomwalk(pd,tick);
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
		target= iMap->id2bl(pd->target_id);
		if (!target || pd->bl.m != target->m || iStatus->isdead(target) ||
			!check_distance_bl(&pd->bl, target, pd->db->range3))
		{
			target = NULL;
			pet_unlocktarget(pd);
		}
	}
	
	if(!target && pd->loot && pd->msd && pc_has_permission(pd->msd, PC_PERM_TRADE) && pd->loot->count < pd->loot->max && DIFF_TICK(tick,pd->ud.canact_tick)>0) {
		//Use half the pet's range of sight.
		iMap->foreachinrange(pet_ai_sub_hard_lootsearch,&pd->bl,
			pd->db->range2/2, BL_ITEM,pd,&target);
	}
	
	if (!target) {
	//Just walk around.
		if (check_distance_bl(&sd->bl, &pd->bl, 3))
			return 0; //Already next to master.

		if(pd->ud.walktimer != INVALID_TIMER && check_distance_blxy(&sd->bl, pd->ud.to_x,pd->ud.to_y, 3))
			return 0; //Already walking to him

		unit_calc_pos(&pd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
		if(!unit_walktoxy(&pd->bl,pd->ud.to_x,pd->ud.to_y,0))
			pet_randomwalk(pd,tick);

		return 0;
	}
	
	if(pd->ud.target == target->id &&
		(pd->ud.attacktimer != INVALID_TIMER || pd->ud.walktimer != INVALID_TIMER))
		return 0; //Target already locked.

	if (target->type != BL_ITEM) 
	{ //enemy targetted
		if(!battle->check_range(&pd->bl,target,pd->status.rhw.range))
		{	//Chase
			if(!unit_walktobl(&pd->bl, target, pd->status.rhw.range, 2))
				pet_unlocktarget(pd); //Unreachable target.
			return 0;
		}
		//Continuous attack.
		unit_attack(&pd->bl, pd->target_id, 1);
	} else {	//Item Targeted, attempt loot
		if (!check_distance_bl(&pd->bl, target, 1))
		{	//Out of range
			if(!unit_walktobl(&pd->bl, target, 1, 1)) //Unreachable target.
				pet_unlocktarget(pd);
			return 0;
		} else{
			struct flooritem_data *fitem = (struct flooritem_data *)target;
			if(pd->loot->count < pd->loot->max){
				memcpy(&pd->loot->item[pd->loot->count++],&fitem->item_data,sizeof(pd->loot->item[0]));
				pd->loot->weight += itemdb_weight(fitem->item_data.nameid)*fitem->item_data.amount;
				iMap->clearflooritem(target);
			} 
			//Target is unlocked regardless of whether it was picked or not.
			pet_unlocktarget(pd);
		}
	}
	return 0;
}

static int pet_ai_sub_foreachclient(struct map_session_data *sd,va_list ap)
{
	unsigned int tick = va_arg(ap,unsigned int);
	if(sd->status.pet_id && sd->pd)
		pet_ai_sub_hard(sd->pd,sd,tick);

	return 0;
}

static int pet_ai_hard(int tid, unsigned int tick, int id, intptr_t data)
{
	iMap->map_foreachpc(pet_ai_sub_foreachclient,tick);

	return 0;
}

static int pet_ai_sub_hard_lootsearch(struct block_list *bl,va_list ap)
{
	struct pet_data* pd;
	struct flooritem_data *fitem = (struct flooritem_data *)bl;
	struct block_list **target;
	int sd_charid =0;

	pd=va_arg(ap,struct pet_data *);
	target=va_arg(ap,struct block_list**);

	sd_charid = fitem->first_get_charid;

	if(sd_charid && sd_charid != pd->msd->status.char_id)
		return 0;
	
	if(unit_can_reach_bl(&pd->bl,bl, pd->db->range2, 1, NULL, NULL) &&
		((*target) == NULL || //New target closer than previous one.
		!check_distance_bl(&pd->bl, *target, distance_bl(&pd->bl, bl))))
	{
		(*target) = bl;
		pd->target_id = bl->id;
		return 1;
	}

	return 0;
}

static int pet_delay_item_drop(int tid, unsigned int tick, int id, intptr_t data)
{
	struct item_drop_list *list;
	struct item_drop *ditem, *ditem_prev;
	list=(struct item_drop_list *)data;
	ditem = list->item;
	while (ditem) {
		iMap->addflooritem(&ditem->item_data,ditem->item_data.amount,
			list->m,list->x,list->y,
			list->first_charid,list->second_charid,list->third_charid,0);
		ditem_prev = ditem;
		ditem = ditem->next;
		ers_free(item_drop_ers, ditem_prev);
	}
	ers_free(item_drop_list_ers, list);
	return 0;
}

int pet_lootitem_drop(struct pet_data *pd,struct map_session_data *sd)
{
	int i,flag=0;
	struct item_drop_list *dlist;
	struct item_drop *ditem;
	struct item *it;
	if(!pd || !pd->loot || !pd->loot->count)
		return 0;
	dlist = ers_alloc(item_drop_list_ers, struct item_drop_list);
	dlist->m = pd->bl.m;
	dlist->x = pd->bl.x;
	dlist->y = pd->bl.y;
	dlist->first_charid = 0;
	dlist->second_charid = 0;
	dlist->third_charid = 0;
	dlist->item = NULL;

	for(i=0;i<pd->loot->count;i++) {
		it = &pd->loot->item[i];
		if(sd){
			if((flag = pc->additem(sd,it,it->amount,LOG_TYPE_PICKDROP_PLAYER))){
				clif->additem(sd,0,0,flag);
				ditem = ers_alloc(item_drop_ers, struct item_drop);
				memcpy(&ditem->item_data, it, sizeof(struct item));
				ditem->next = dlist->item;
				dlist->item = ditem;
			}
		}
		else {
			ditem = ers_alloc(item_drop_ers, struct item_drop);
			memcpy(&ditem->item_data, it, sizeof(struct item));
			ditem->next = dlist->item;
			dlist->item = ditem;
		}
	}
	//The smart thing to do is use pd->loot->max (thanks for pointing it out, Shinomori)
	memset(pd->loot->item,0,pd->loot->max * sizeof(struct item));
	pd->loot->count = 0;
	pd->loot->weight = 0;
	pd->ud.canact_tick = iTimer->gettick()+10000;	//prevent picked up during 10*1000ms

	if (dlist->item)
		iTimer->add_timer(iTimer->gettick()+540,pet_delay_item_drop,0,(intptr_t)dlist);
	else
		ers_free(item_drop_list_ers, dlist);
	return 1;
}

/*==========================================
 * pet bonus giving skills [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/ 
int pet_skill_bonus_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd=iMap->id2sd(id);
	struct pet_data *pd;
	int bonus;
	int timer = 0;

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
		timer = pd->bonus->delay*1000;	// the duration until pet bonuses will be reactivated again
	} else if (pd->pet.intimate) {
		bonus = 1;
		timer = pd->bonus->duration*1000;	// the duration for pet bonuses to be in effect
	} else { //Lost pet...
		pd->bonus->timer = INVALID_TIMER;
		return 0;
	}

	if (pd->state.skillbonus != bonus) {
		pd->state.skillbonus = bonus;
		status_calc_pc(sd, 0);
	}
	// wait for the next timer
	pd->bonus->timer=iTimer->add_timer(tick+timer,pet_skill_bonus_timer,sd->bl.id,0);
	return 0;
}

/*==========================================
 * pet recovery skills [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/ 
int pet_recovery_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd=iMap->id2sd(id);
	struct pet_data *pd;
	
	if(sd==NULL || sd->pd == NULL || sd->pd->recovery == NULL)
		return 1;
	
	pd=sd->pd;

	if(pd->recovery->timer != tid) {
		ShowError("pet_recovery_timer %d != %d\n",pd->recovery->timer,tid);
		return 0;
	}

	if(sd->sc.data[pd->recovery->type])
	{	//Display a heal animation? 
		//Detoxify is chosen for now.
		clif->skill_nodamage(&pd->bl,&sd->bl,TF_DETOXIFY,1,1);
		status_change_end(&sd->bl, pd->recovery->type, INVALID_TIMER);
		clif->emotion(&pd->bl, E_OK);
	}

	pd->recovery->timer = INVALID_TIMER;
	
	return 0;
}

int pet_heal_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd=iMap->id2sd(id);
	struct status_data *status;
	struct pet_data *pd;
	unsigned int rate = 100;
	
	if(sd==NULL || sd->pd == NULL || sd->pd->s_skill == NULL)
		return 1;
	
	pd=sd->pd;
	
	if(pd->s_skill->timer != tid) {
		ShowError("pet_heal_timer %d != %d\n",pd->s_skill->timer,tid);
		return 0;
	}
	
	status = iStatus->get_status_data(&sd->bl);
	
	if(pc_isdead(sd) ||
		(rate = get_percentage(status->sp, status->max_sp)) > pd->s_skill->sp ||
		(rate = get_percentage(status->hp, status->max_hp)) > pd->s_skill->hp ||
		(rate = (pd->ud.skilltimer != INVALID_TIMER)) //Another skill is in effect
	) {  //Wait (how long? 1 sec for every 10% of remaining)
		pd->s_skill->timer=iTimer->add_timer(iTimer->gettick()+(rate>10?rate:10)*100,pet_heal_timer,sd->bl.id,0);
		return 0;
	}
	pet_stop_attack(pd);
	pet_stop_walking(pd,1);
	clif->skill_nodamage(&pd->bl,&sd->bl,AL_HEAL,pd->s_skill->lv,1);
	iStatus->heal(&sd->bl, pd->s_skill->lv,0, 0);
	pd->s_skill->timer=iTimer->add_timer(tick+pd->s_skill->delay*1000,pet_heal_timer,sd->bl.id,0);
	return 0;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------*/ 
int pet_skill_support_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd=iMap->id2sd(id);
	struct pet_data *pd;
	struct status_data *status;
	short rate = 100;
	if(sd==NULL || sd->pd == NULL || sd->pd->s_skill == NULL)
		return 1;
	
	pd=sd->pd;
	
	if(pd->s_skill->timer != tid) {
		ShowError("pet_skill_support_timer %d != %d\n",pd->s_skill->timer,tid);
		return 0;
	}
	
	status = iStatus->get_status_data(&sd->bl);

	if (DIFF_TICK(pd->ud.canact_tick, tick) > 0)
	{	//Wait until the pet can act again.
		pd->s_skill->timer=iTimer->add_timer(pd->ud.canact_tick,pet_skill_support_timer,sd->bl.id,0);
		return 0;
	}
	
	if(pc_isdead(sd) ||
		(rate = get_percentage(status->sp, status->max_sp)) > pd->s_skill->sp ||
		(rate = get_percentage(status->hp, status->max_hp)) > pd->s_skill->hp ||
		(rate = (pd->ud.skilltimer != INVALID_TIMER)) //Another skill is in effect
	) {  //Wait (how long? 1 sec for every 10% of remaining)
		pd->s_skill->timer=iTimer->add_timer(tick+(rate>10?rate:10)*100,pet_skill_support_timer,sd->bl.id,0);
		return 0;
	}
	
	pet_stop_attack(pd);
	pet_stop_walking(pd,1);
	pd->s_skill->timer=iTimer->add_timer(tick+pd->s_skill->delay*1000,pet_skill_support_timer,sd->bl.id,0);
	if (skill->get_inf(pd->s_skill->id) & INF_GROUND_SKILL)
		unit_skilluse_pos(&pd->bl, sd->bl.x, sd->bl.y, pd->s_skill->id, pd->s_skill->lv);
	else
		unit_skilluse_id(&pd->bl, sd->bl.id, pd->s_skill->id, pd->s_skill->lv);
	return 0;
}

/*==========================================
 * Pet read db data
 * pet_db.txt
 * pet_db2.txt
 *------------------------------------------*/ 
int read_petdb()
{
	char* filename[] = {"pet_db.txt","pet_db2.txt"};
	FILE *fp;
	int nameid,i,j,k; 

	// Remove any previous scripts in case reloaddb was invoked.
	for( j = 0; j < MAX_PET_DB; j++ )
	{
		if( pet_db[j].pet_script )
		{
			script_free_code(pet_db[j].pet_script);
			pet_db[j].pet_script = NULL;
		}
		if( pet_db[j].equip_script )
		{
			script_free_code(pet_db[j].equip_script);
			pet_db[j].pet_script = NULL;
		}
	}

	// clear database
	memset(pet_db,0,sizeof(pet_db));

	j = 0; // entry counter
	for( i = 0; i < ARRAYLENGTH(filename); i++ )
	{
		char line[1024];
		int lines, entries;

		sprintf(line, "%s/%s", iMap->db_path, filename[i]);
		fp=fopen(line,"r");
		if( fp == NULL )
		{
			if( i == 0 )
				ShowError("can't read %s\n",line);
			continue;
		}

		lines = entries = 0;
		while( fgets(line, sizeof(line), fp) && j < MAX_PET_DB )
		{
			char *str[22], *p;
			lines++;

			if(line[0] == '/' && line[1] == '/')
				continue;
			memset(str, 0, sizeof(str));
			p = line;
			while( ISSPACE(*p) )
				++p;
			if( *p == '\0' )
				continue; // empty line
			for( k = 0; k < 20; ++k )
			{
				str[k] = p;
				p = strchr(p,',');
				if( p == NULL )
					break; // comma not found
				*p = '\0';
				++p;
			}

			if( p == NULL )
			{
				ShowError("read_petdb: Insufficient columns in line %d, skipping.\n", lines);
				continue;
			}

			// Pet Script
			if( *p != '{' )
			{
				ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
				continue;
			}

			str[20] = p;
			p = strstr(p+1,"},");
			if( p == NULL )
			{
				ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
				continue;
			}
			p[1] = '\0';
			p += 2;

			// Equip Script
			if( *p != '{' )
			{
				ShowError("read_petdb: Invalid format (Equip Script column) in line %d, skipping.\n", lines);
				continue;
			}
			str[21] = p;

			if( (nameid = atoi(str[0])) <= 0 )
				continue;

			if( !mobdb_checkid(nameid) )
			{
				ShowWarning("pet_db reading: Invalid mob-class %d, pet not read.\n", nameid);
				continue;
			}

			pet_db[j].class_ = nameid;
			safestrncpy(pet_db[j].name,str[1],NAME_LENGTH);
			safestrncpy(pet_db[j].jname,str[2],NAME_LENGTH);
			pet_db[j].itemID=atoi(str[3]);
			pet_db[j].EggID=atoi(str[4]);
			pet_db[j].AcceID=atoi(str[5]);
			pet_db[j].FoodID=atoi(str[6]);
			pet_db[j].fullness=atoi(str[7]);
			pet_db[j].hungry_delay=atoi(str[8])*1000;
			pet_db[j].r_hungry=atoi(str[9]);
			if( pet_db[j].r_hungry <= 0 )
				pet_db[j].r_hungry=1;
			pet_db[j].r_full=atoi(str[10]);
			pet_db[j].intimate=atoi(str[11]);
			pet_db[j].die=atoi(str[12]);
			pet_db[j].capture=atoi(str[13]);
			pet_db[j].speed=atoi(str[14]);
			pet_db[j].s_perfor=(char)atoi(str[15]);
			pet_db[j].talk_convert_class=atoi(str[16]);
			pet_db[j].attack_rate=atoi(str[17]);
			pet_db[j].defence_attack_rate=atoi(str[18]);
			pet_db[j].change_target_rate=atoi(str[19]);
			pet_db[j].pet_script = NULL;
			pet_db[j].equip_script = NULL;

			if( *str[20] )
				pet_db[j].pet_script = parse_script(str[20], filename[i], lines, 0);
			if( *str[21] )
				pet_db[j].equip_script = parse_script(str[21], filename[i], lines, 0);

			j++;
			entries++;
		}

		if( j >= MAX_PET_DB )
			ShowWarning("read_petdb: Reached max number of pets [%d]. Remaining pets were not read.\n ", MAX_PET_DB);
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' pets in '"CL_WHITE"%s"CL_RESET"'.\n", entries, filename[i]);
	}
	return 0;
}

/*==========================================
 * Initialization process relationship skills
 *------------------------------------------*/
int do_init_pet(void)
{
	read_petdb();

	item_drop_ers = ers_new(sizeof(struct item_drop),"pet.c::item_drop_ers",ERS_OPT_NONE);
	item_drop_list_ers = ers_new(sizeof(struct item_drop_list),"pet.c::item_drop_list_ers",ERS_OPT_NONE);
	
	iTimer->add_timer_func_list(pet_hungry,"pet_hungry");
	iTimer->add_timer_func_list(pet_ai_hard,"pet_ai_hard");
	iTimer->add_timer_func_list(pet_skill_bonus_timer,"pet_skill_bonus_timer"); // [Valaris]
	iTimer->add_timer_func_list(pet_delay_item_drop,"pet_delay_item_drop");
	iTimer->add_timer_func_list(pet_skill_support_timer, "pet_skill_support_timer"); // [Skotlex]
	iTimer->add_timer_func_list(pet_recovery_timer,"pet_recovery_timer"); // [Valaris]
	iTimer->add_timer_func_list(pet_heal_timer,"pet_heal_timer"); // [Valaris]
	iTimer->add_timer_interval(iTimer->gettick()+MIN_PETTHINKTIME,pet_ai_hard,0,0,MIN_PETTHINKTIME);

	return 0;
}

int do_final_pet(void)
{
	int i;
	for( i = 0; i < MAX_PET_DB; i++ )
	{
		if( pet_db[i].pet_script )
		{
			script_free_code(pet_db[i].pet_script);
			pet_db[i].pet_script = NULL;
		}
		if( pet_db[i].equip_script )
		{
			script_free_code(pet_db[i].equip_script);
			pet_db[i].equip_script = NULL;
		}
	}
	ers_destroy(item_drop_ers);
	ers_destroy(item_drop_list_ers);
	return 0;
}
