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
#include "homunculus.h"

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

struct homunculus_interface homunculus_s;
struct homun_dbs homundbs;

struct homunculus_interface *homun;

//Returns the viewdata for homunculus
struct view_data* homunculus_get_viewdata(int class_) {
	Assert_retr(NULL, homdb_checkid(class_));

	return &homun->dbs->viewdb[class_-HM_CLASS_BASE];
}

enum homun_type homunculus_class2type(int class_) {
	switch(class_) {
		// Normal Homunculus
		case HOMID_LIF:
		case HOMID_AMISTR:
		case HOMID_FILIR:
		case HOMID_VANILMIRTH:
		case HOMID_LIF2:
		case HOMID_AMISTR2:
		case HOMID_FILIR2:
		case HOMID_VANILMIRTH2:
			return HT_REG;
		// Evolved Homunculus
		case HOMID_LIF_E:
		case HOMID_AMISTR_E:
		case HOMID_FILIR_E:
		case HOMID_VANILMIRTH_E:
		case HOMID_LIF2_E:
		case HOMID_AMISTR2_E:
		case HOMID_FILIR2_E:
		case HOMID_VANILMIRTH2_E:
			return HT_EVO;
		// Homunculus S
		case HOMID_EIRA:
		case HOMID_BAYERI:
		case HOMID_SERA:
		case HOMID_DIETR:
		case HOMID_ELEANOR:
			return HT_S;
		default:
			return HT_INVALID;
	}
}

void homunculus_addspiritball(struct homun_data *hd, int max) {
	nullpo_retv(hd);

	if (max > MAX_SKILL_LEVEL)
		max = MAX_SKILL_LEVEL;
	if (hd->homunculus.spiritball < 0)
		hd->homunculus.spiritball = 0;

	if (hd->homunculus.spiritball && hd->homunculus.spiritball >= max) {
		hd->homunculus.spiritball = max;
	}
	else
		hd->homunculus.spiritball++;

	clif->spiritball(&hd->bl);
}

void homunculus_delspiritball(struct homun_data *hd, int count, int type) {
	nullpo_retv(hd);

	if (hd->homunculus.spiritball <= 0) {
		hd->homunculus.spiritball = 0;
		return;
	}
	if (count <= 0)
		return;
	if (count > MAX_SKILL_LEVEL)
		count = MAX_SKILL_LEVEL;
	if (count > hd->homunculus.spiritball)
		count = hd->homunculus.spiritball;

	hd->homunculus.spiritball -= count;
	if (!type)
		clif->spiritball(&hd->bl);
}

void homunculus_damaged(struct homun_data *hd) {
	clif->hominfo(hd->master,hd,0);
}

int homunculus_dead(struct homun_data *hd) {
	//There's no intimacy penalties on death (from Tharis)
	struct map_session_data *sd;

	nullpo_retr(3, hd);
	sd = hd->master;
	clif->emotion(&hd->bl, E_WAH);

	//Delete timers when dead.
	homun->hunger_timer_delete(hd);
	hd->homunculus.hp = 0;

	if (!sd) //unit remove map will invoke unit free
		return 3;

	clif->emotion(&sd->bl, E_SOB);
	//Remove from map (if it has no intimacy, it is auto-removed from memory)
	return 3;
}

//Vaporize a character's homun. If flag, HP needs to be 80% or above.
int homunculus_vaporize(struct map_session_data *sd, enum homun_state flag) {
	struct homun_data *hd;

	nullpo_ret(sd);

	hd = sd->hd;
	if (!hd || hd->homunculus.vaporize != HOM_ST_ACTIVE)
		return 0;

	if (status->isdead(&hd->bl))
		return 0; //Can't vaporize a dead homun.

	if (flag == HOM_ST_REST && get_percentage(hd->battle_status.hp, hd->battle_status.max_hp) < 80)
		return 0;

	hd->regen.state.block = 3; //Block regen while vaporized.
	//Delete timers when vaporized.
	homun->hunger_timer_delete(hd);
	hd->homunculus.vaporize = flag;
	if(battle_config.hom_setting&0x40)
		memset(hd->blockskill, 0, sizeof(hd->blockskill));
	clif->hominfo(sd, sd->hd, 0);
	homun->save(hd);
	return unit->remove_map(&hd->bl, CLR_OUTSIGHT, ALC_MARK);
}

//delete a homunculus, completely "killing it".
//Emote is the emotion the master should use, send negative to disable.
int homunculus_delete(struct homun_data *hd, int emote) {
	struct map_session_data *sd;
	nullpo_ret(hd);
	sd = hd->master;

	if (!sd)
		return unit->free(&hd->bl,CLR_DEAD);

	if (emote >= 0)
		clif->emotion(&sd->bl, emote);

	//This makes it be deleted right away.
	hd->homunculus.intimacy = 0;
	// Send homunculus_dead to client
	hd->homunculus.hp = 0;
	clif->hominfo(sd, hd, 0);
	return unit->remove_map(&hd->bl,CLR_OUTSIGHT, ALC_MARK);
}

int homunculus_calc_skilltree(struct homun_data *hd, int flag_evolve) {
	int i, id = 0;
	int j, f = 1;
	int c = 0;

	nullpo_ret(hd);
	/* load previous homunculus form skills first. */
	if( hd->homunculus.prev_class != 0 ) {
		c = hd->homunculus.prev_class - HM_CLASS_BASE;
		Assert_ret(c >= 0 && c < MAX_HOMUNCULUS_CLASS);

		for( i = 0; i < MAX_SKILL_TREE && ( id = homun->dbs->skill_tree[c][i].id ) > 0; i++ ) {
			if( hd->homunculus.hskill[ id - HM_SKILLBASE ].id )
				continue; //Skill already known.
			if(!battle_config.skillfree) {
				for (j = 0; j < MAX_HOM_SKILL_REQUIRE; j++) {
					if( homun->dbs->skill_tree[c][i].need[j].id &&
					   homun->checkskill(hd,homun->dbs->skill_tree[c][i].need[j].id) < homun->dbs->skill_tree[c][i].need[j].lv ) {
						f = 0;
						break;
					}
				}
			}
			if ( f )
				hd->homunculus.hskill[id-HM_SKILLBASE].id = id;
		}

		f = 1;
	}

	c = hd->homunculus.class_ - HM_CLASS_BASE;
	Assert_ret(c >= 0 && c < MAX_HOMUNCULUS_CLASS);

	for( i = 0; i < MAX_SKILL_TREE && ( id = homun->dbs->skill_tree[c][i].id ) > 0; i++ ) {
		if( hd->homunculus.hskill[ id - HM_SKILLBASE ].id )
			continue; //Skill already known.
		j = ( flag_evolve ) ? 10 : hd->homunculus.intimacy;
		if( j < homun->dbs->skill_tree[c][i].intimacylv )
			continue;
		if(!battle_config.skillfree) {
			for (j = 0; j < MAX_HOM_SKILL_REQUIRE; j++) {
				if( homun->dbs->skill_tree[c][i].need[j].id &&
					homun->checkskill(hd,homun->dbs->skill_tree[c][i].need[j].id) < homun->dbs->skill_tree[c][i].need[j].lv ) {
					f = 0;
					break;
				}
			}
		}
		if ( f )
			hd->homunculus.hskill[id-HM_SKILLBASE].id = id;
	}

	if( hd->master )
		clif->homskillinfoblock(hd->master);
	return 0;
}

int homunculus_checkskill(struct homun_data *hd,uint16 skill_id) {
	int i = skill_id - HM_SKILLBASE;
	if(!hd)
		return 0;

	Assert_ret(i >= 0 && i < MAX_HOMUNSKILL);
	if(hd->homunculus.hskill[i].id == skill_id)
		return (hd->homunculus.hskill[i].lv);

	return 0;
}

int homunculus_skill_tree_get_max(int id, int b_class) {
	int i, skill_id;
	b_class -= HM_CLASS_BASE;
	Assert_ret(b_class >= 0 && b_class < MAX_HOMUNCULUS_CLASS);
	for(i=0;(skill_id=homun->dbs->skill_tree[b_class][i].id)>0;i++)
		if (id == skill_id)
			return homun->dbs->skill_tree[b_class][i].max;
	return skill->get_max(id);
}

void homunculus_skillup(struct homun_data *hd,uint16 skill_id) {
	int i = 0 ;
	nullpo_retv(hd);

	if(hd->homunculus.vaporize != HOM_ST_ACTIVE)
		return;

	i = skill_id - HM_SKILLBASE;
	Assert_retv(i >= 0 && i < MAX_HOMUNSKILL);
	if(hd->homunculus.skillpts > 0 &&
		hd->homunculus.hskill[i].id &&
		hd->homunculus.hskill[i].flag == SKILL_FLAG_PERMANENT && //Don't allow raising while you have granted skills. [Skotlex]
		hd->homunculus.hskill[i].lv < homun->skill_tree_get_max(skill_id, hd->homunculus.class_)
		)
	{
		hd->homunculus.hskill[i].lv++;
		hd->homunculus.skillpts-- ;
		status_calc_homunculus(hd,SCO_NONE);
		if (hd->master) {
			clif->homskillup(hd->master, skill_id);
			clif->hominfo(hd->master,hd,0);
			clif->homskillinfoblock(hd->master);
		}
	}
}

bool homunculus_levelup(struct homun_data *hd) {
	struct s_homunculus *hom;
	struct h_stats *min, *max;
	int growth_str, growth_agi, growth_vit, growth_int, growth_dex, growth_luk ;
	int growth_max_hp, growth_max_sp;
	enum homun_type htype;

	nullpo_retr(false, hd);
	if( (htype = homun->class2type(hd->homunculus.class_)) == HT_INVALID ) {
		ShowError("homunculus_levelup: Invalid class %d. \n", hd->homunculus.class_);
		return false;
	}

	if( !hd->exp_next || hd->homunculus.exp < hd->exp_next )
		return false;

	switch( htype ) {
		case HT_REG:
		case HT_EVO:
			if( hd->homunculus.level >= battle_config.hom_max_level )
				return false;
			break;
		case HT_S:
			if( hd->homunculus.level >= battle_config.hom_S_max_level )
				return false;
			break;
	}

	hom = &hd->homunculus;
	hom->level++ ;
	if (!(hom->level % 3))
		hom->skillpts++; //1 skillpoint each 3 base level

	hom->exp -= hd->exp_next;
	hd->exp_next = homun->dbs->exptable[hom->level - 1];

	max  = &hd->homunculusDB->gmax;
	min  = &hd->homunculusDB->gmin;

	growth_max_hp = rnd->value(min->HP, max->HP);
	growth_max_sp = rnd->value(min->SP, max->SP);
	growth_str = rnd->value(min->str, max->str);
	growth_agi = rnd->value(min->agi, max->agi);
	growth_vit = rnd->value(min->vit, max->vit);
	growth_dex = rnd->value(min->dex, max->dex);
	growth_int = rnd->value(min->int_,max->int_);
	growth_luk = rnd->value(min->luk, max->luk);

	//Aegis discards the decimals in the stat growth values!
	growth_str-=growth_str%10;
	growth_agi-=growth_agi%10;
	growth_vit-=growth_vit%10;
	growth_dex-=growth_dex%10;
	growth_int-=growth_int%10;
	growth_luk-=growth_luk%10;

	hom->max_hp += growth_max_hp;
	hom->max_sp += growth_max_sp;
	hom->str += growth_str;
	hom->agi += growth_agi;
	hom->vit += growth_vit;
	hom->dex += growth_dex;
	hom->int_+= growth_int;
	hom->luk += growth_luk;

	APPLY_HOMUN_LEVEL_STATWEIGHT();

	if ( battle_config.homunculus_show_growth ) {
		char output[256] ;
		sprintf(output,
			"Growth: hp:%d sp:%d str(%.2f) agi(%.2f) vit(%.2f) int(%.2f) dex(%.2f) luk(%.2f) ",
			growth_max_hp, growth_max_sp,
			growth_str/10.0, growth_agi/10.0, growth_vit/10.0,
			growth_int/10.0, growth_dex/10.0, growth_luk/10.0);
		clif_disp_onlyself(hd->master, output);
	}
	return true;
}

int homunculus_change_class(struct homun_data *hd, short class_) {
	int i = homun->db_search(class_,HOMUNCULUS_CLASS);
	nullpo_retr(0, hd);
	if (i == INDEX_NOT_FOUND)
		return 0;
	hd->homunculusDB = &homun->dbs->db[i];
	hd->homunculus.class_ = class_;
	status->set_viewdata(&hd->bl, class_);
	homun->calc_skilltree(hd, 1);
	return 1;
}

bool homunculus_evolve(struct homun_data *hd) {
	struct s_homunculus *hom;
	struct h_stats *max, *min;
	struct map_session_data *sd;
	nullpo_ret(hd);

	sd = hd->master;
	if (!sd)
		return false;

	if(!hd->homunculusDB->evo_class || hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		clif->emotion(&hd->bl, E_SWT);
		return false;
	}

	if (!homun->change_class(hd, hd->homunculusDB->evo_class)) {
		ShowError("homunculus_evolve: Can't evolve homunc from %d to %d", hd->homunculus.class_, hd->homunculusDB->evo_class);
		return false;
	}

	//Apply evolution bonuses
	hom = &hd->homunculus;
	max = &hd->homunculusDB->emax;
	min = &hd->homunculusDB->emin;
	hom->max_hp += rnd->value(min->HP, max->HP);
	hom->max_sp += rnd->value(min->SP, max->SP);
	hom->str += 10*rnd->value(min->str, max->str);
	hom->agi += 10*rnd->value(min->agi, max->agi);
	hom->vit += 10*rnd->value(min->vit, max->vit);
	hom->int_+= 10*rnd->value(min->int_,max->int_);
	hom->dex += 10*rnd->value(min->dex, max->dex);
	hom->luk += 10*rnd->value(min->luk, max->luk);
	hom->intimacy = 500;

	unit->remove_map(&hd->bl, CLR_OUTSIGHT, ALC_MARK);
	map->addblock(&hd->bl);

	clif->spawn(&hd->bl);
	clif->emotion(&sd->bl, E_NO1);
	clif->specialeffect(&hd->bl,568,AREA);

	//status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	status_calc_homunculus(hd,SCO_FIRST);

	if (!(battle_config.hom_setting&0x2))
		skill->unit_move(&sd->hd->bl,timer->gettick(),1); // apply land skills immediately

	return true;
}

bool homunculus_mutate(struct homun_data *hd, int homun_id) {
	struct s_homunculus *hom;
	struct map_session_data *sd;
	int prev_class = 0;
	enum homun_type m_class, m_id;
	nullpo_ret(hd);

	sd = hd->master;
	if (!sd)
		return false;

	m_class = homun->class2type(hd->homunculus.class_);
	m_id    = homun->class2type(homun_id);

	if( m_class == HT_INVALID || m_id == HT_INVALID || m_class != HT_EVO || m_id != HT_S ) {
		clif->emotion(&hd->bl, E_SWT);
		return false;
	}

	prev_class = hd->homunculus.class_;

	if( !homun->change_class(hd, homun_id) ) {
		ShowError("homunculus_mutate: Can't evolve homunc from %d to %d", hd->homunculus.class_, homun_id);
		return false;
	}

	unit->remove_map(&hd->bl, CLR_OUTSIGHT, ALC_MARK);
	map->addblock(&hd->bl);

	clif->spawn(&hd->bl);
	clif->emotion(&sd->bl, E_NO1);
	clif->specialeffect(&hd->bl,568,AREA);

	//status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom = &hd->homunculus;
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	hom->prev_class = prev_class;
	status_calc_homunculus(hd,SCO_FIRST);

	if (!(battle_config.hom_setting&0x2))
		skill->unit_move(&sd->hd->bl,timer->gettick(),1); // apply land skills immediately

	return true;
}

int homunculus_gainexp(struct homun_data *hd,unsigned int exp) {
	enum homun_type htype;

	nullpo_ret(hd);
	if(hd->homunculus.vaporize != HOM_ST_ACTIVE)
		return 1;

	if( (htype = homun->class2type(hd->homunculus.class_)) == HT_INVALID ) {
		ShowError("homunculus_gainexp: Invalid class %d. \n", hd->homunculus.class_);
		return 0;
	}

	switch( htype ) {
		case HT_REG:
		case HT_EVO:
			if( hd->homunculus.level >= battle_config.hom_max_level )
				return 0;
			break;
		case HT_S:
			if( hd->homunculus.level >= battle_config.hom_S_max_level )
				return 0;
			break;
	}

	hd->homunculus.exp += exp;

	if(hd->homunculus.exp < hd->exp_next) {
		clif->hominfo(hd->master,hd,0);
		return 0;
	}

	//levelup
	while( hd->homunculus.exp > hd->exp_next && homun->levelup(hd) );

	if( hd->exp_next == 0 )
		hd->homunculus.exp = 0;

	clif->specialeffect(&hd->bl,568,AREA);
	status_calc_homunculus(hd,SCO_NONE);
	status_percent_heal(&hd->bl, 100, 100);
	return 0;
}

// Return the new value
unsigned int homunculus_add_intimacy(struct homun_data *hd, unsigned int value) {
	nullpo_ret(hd);
	if (battle_config.homunculus_friendly_rate != 100)
		value = (value * battle_config.homunculus_friendly_rate) / 100;

	if (hd->homunculus.intimacy + value <= 100000)
		hd->homunculus.intimacy += value;
	else
		hd->homunculus.intimacy = 100000;
	return hd->homunculus.intimacy;
}

// Return 0 if decrease fails or intimacy became 0 else the new value
unsigned int homunculus_consume_intimacy(struct homun_data *hd, unsigned int value) {
	nullpo_ret(hd);
	if (hd->homunculus.intimacy >= value)
		hd->homunculus.intimacy -= value;
	else
		hd->homunculus.intimacy = 0;

	return hd->homunculus.intimacy;
}

void homunculus_healed (struct homun_data *hd) {
	nullpo_retv(hd);
	clif->hominfo(hd->master,hd,0);
}

void homunculus_save(struct homun_data *hd) {
	// copy data that must be saved in homunculus struct ( hp / sp )
	struct map_session_data *sd = NULL;
	//Do not check for max_hp/max_sp caps as current could be higher to max due
	//to status changes/skills (they will be capped as needed upon stat
	//calculation on login)
	nullpo_retv(hd);
	sd = hd->master;
	nullpo_retv(sd);
	hd->homunculus.hp = hd->battle_status.hp;
	hd->homunculus.sp = hd->battle_status.sp;
	intif->homunculus_requestsave(sd->status.account_id, &hd->homunculus);
}

unsigned char homunculus_menu(struct map_session_data *sd,unsigned char menu_num) {
	nullpo_ret(sd);
	if (sd->hd == NULL)
		return 1;

	switch(menu_num) {
		case 0:
			break;
		case 1:
			homun->feed(sd, sd->hd);
			break;
		case 2:
			homun->delete(sd->hd, -1);
			break;
		default:
			ShowError("homunculus_menu : unknown menu choice : %d\n", menu_num) ;
			break;
	}
	return 0;
}

bool homunculus_feed(struct map_session_data *sd, struct homun_data *hd) {
	int i, foodID, emotion;

	nullpo_retr(false, hd);
	nullpo_retr(false, sd);
	if(hd->homunculus.vaporize == HOM_ST_REST)
		return false;

	foodID = hd->homunculusDB->foodID;
	i = pc->search_inventory(sd,foodID);
	if (i == INDEX_NOT_FOUND) {
		clif->hom_food(sd,foodID,0);
		return false;
	}
	pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME);

	if ( hd->homunculus.hunger >= 91 ) {
		homun->consume_intimacy(hd, 50);
		emotion = E_WAH;
	} else if ( hd->homunculus.hunger >= 76 ) {
		homun->consume_intimacy(hd, 5);
		emotion = E_SWT2;
	} else if ( hd->homunculus.hunger >= 26 ) {
		homun->add_intimacy(hd, 75);
		emotion = E_HO;
	} else if ( hd->homunculus.hunger >= 11 ) {
		homun->add_intimacy(hd, 100);
		emotion = E_HO;
	} else {
		homun->add_intimacy(hd, 50);
		emotion = E_HO;
	}

	hd->homunculus.hunger += 10; //dunno increase value for each food
	if(hd->homunculus.hunger > 100)
		hd->homunculus.hunger = 100;

	clif->emotion(&hd->bl,emotion);
	clif->send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	clif->send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	clif->hom_food(sd,foodID,1);

	// Too much food :/
	if(hd->homunculus.intimacy == 0)
		return homun->delete(sd->hd, E_OMG);
	return true;
}

int homunculus_hunger_timer(int tid, int64 tick, int id, intptr_t data) {
	struct map_session_data *sd;
	struct homun_data *hd;

	if(!(sd=map->id2sd(id)) || !sd->status.hom_id || !(hd=sd->hd))
		return 1;

	if(hd->hungry_timer != tid){
		ShowError("homunculus_hunger_timer %d != %d\n",hd->hungry_timer,tid);
		return 0;
	}

	hd->hungry_timer = INVALID_TIMER;

	hd->homunculus.hunger-- ;
	if(hd->homunculus.hunger <= 10) {
		clif->emotion(&hd->bl, E_AN);
	} else if(hd->homunculus.hunger == 25) {
		clif->emotion(&hd->bl, E_HMM);
	} else if(hd->homunculus.hunger == 75) {
		clif->emotion(&hd->bl, E_OK);
	}

	if(hd->homunculus.hunger < 0) {
		hd->homunculus.hunger = 0;
		// Delete the homunculus if intimacy <= 100
		if ( !homun->consume_intimacy(hd, 100) )
			return homun->delete(hd, E_OMG);
		clif->send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	}

	clif->send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	hd->hungry_timer = timer->add(tick+hd->homunculusDB->hungryDelay,homun->hunger_timer,sd->bl.id,0); //simple Fix albator
	return 0;
}

void homunculus_hunger_timer_delete(struct homun_data *hd) {
	nullpo_retv(hd);
	if(hd->hungry_timer != INVALID_TIMER) {
		timer->delete(hd->hungry_timer,homun->hunger_timer);
		hd->hungry_timer = INVALID_TIMER;
	}
}

int homunculus_change_name(struct map_session_data *sd, const char *name)
{
	int i;
	struct homun_data *hd;
	nullpo_retr(1, sd);
	nullpo_retr(1, name);

	hd = sd->hd;
	if (!homun_alive(hd))
		return 1;
	if(hd->homunculus.rename_flag && !battle_config.hom_rename)
		return 1;

	for(i=0;i<NAME_LENGTH && name[i];i++){
		if( !(name[i]&0xe0) || name[i]==0x7f)
			return 1;
	}

	return intif_rename_hom(sd, name);
}

bool homunculus_change_name_ack(struct map_session_data *sd, const char *name, int flag)
{
	struct homun_data *hd;
	char *newname = NULL;
	nullpo_retr(false, sd);
	nullpo_retr(false, name);
	hd = sd->hd;
	nullpo_retr(false, hd);
	if (!homun_alive(hd)) return false;

	newname = aStrndup(name, NAME_LENGTH-1);
	normalize_name(newname, " ");//bugreport:3032 // FIXME[Haru]: This should be normalized by the inter-server (so that it's const here)

	if (flag == 0 || strlen(newname) == 0) {
		clif->message(sd->fd, msg_sd(sd,280)); // You cannot use this name
		aFree(newname);
		return false;
	}
	safestrncpy(hd->homunculus.name, newname, NAME_LENGTH);
	aFree(newname);
	clif->charnameack (0,&hd->bl);
	hd->homunculus.rename_flag = 1;
	clif->hominfo(sd,hd,0);
	return true;
}

int homunculus_db_search(int key,int type) {
	int i;

	for(i=0;i<MAX_HOMUNCULUS_CLASS;i++) {
		if(homun->dbs->db[i].base_class <= 0)
			continue;
		switch(type) {
			case HOMUNCULUS_CLASS:
				if(homun->dbs->db[i].base_class == key ||
					homun->dbs->db[i].evo_class == key)
					return i;
				break;
			case HOMUNCULUS_FOOD:
				if(homun->dbs->db[i].foodID == key)
					return i;
				break;
			default:
				return INDEX_NOT_FOUND;
		}
	}
	return INDEX_NOT_FOUND;
}

/**
 * Creates and initializes an homunculus.
 *
 * @remark
 *   The char_id field in the source homunculus data is ignored (the sd's
 *   character ID is used instead).
 *
 * @param sd  The owner character.
 * @param hom The homunculus source data.
 * @retval false in case of errors.
 */
bool homunculus_create(struct map_session_data *sd, const struct s_homunculus *hom)
{
	struct homun_data *hd;
	int i = 0;

	nullpo_retr(false, sd);
	nullpo_retr(false, hom);

	Assert_retr(false, sd->status.hom_id == 0 || sd->hd == 0 || sd->hd->master == sd);

	i = homun->db_search(hom->class_,HOMUNCULUS_CLASS);
	if (i == INDEX_NOT_FOUND) {
		ShowError("homunculus_create: unknown class [%d] for homunculus '%s', requesting deletion.\n", hom->class_, hom->name);
		sd->status.hom_id = 0;
		intif->homunculus_requestdelete(hom->hom_id);
		return false;
	}
	CREATE(hd, struct homun_data, 1);
	hd->bl.type = BL_HOM;
	hd->bl.id = npc->get_new_npc_id();
	sd->hd = hd;

	hd->master = sd;
	hd->homunculusDB = &homun->dbs->db[i];
	memcpy(&hd->homunculus, hom, sizeof(struct s_homunculus));
	hd->homunculus.char_id = sd->status.char_id; // Fix character ID if necessary.
	hd->exp_next = homun->dbs->exptable[hd->homunculus.level - 1];

	status->set_viewdata(&hd->bl, hd->homunculus.class_);
	status->change_init(&hd->bl);
	unit->dataset(&hd->bl);
	hd->ud.dir = sd->ud.dir;

	// Find a random valid pos around the player
	hd->bl.m = sd->bl.m;
	hd->bl.x = sd->bl.x;
	hd->bl.y = sd->bl.y;
	unit->calc_pos(&hd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	hd->bl.x = hd->ud.to_x;
	hd->bl.y = hd->ud.to_y;
	hd->masterteleport_timer = 0;

	map->addiddb(&hd->bl);
	status_calc_homunculus(hd,SCO_FIRST);
	status_percent_heal(&hd->bl, 100, 100);

	hd->hungry_timer = INVALID_TIMER;
	return true;
}

void homunculus_init_timers(struct homun_data * hd) {
	nullpo_retv(hd);
	if (hd->hungry_timer == INVALID_TIMER)
		hd->hungry_timer = timer->add(timer->gettick()+hd->homunculusDB->hungryDelay,homun->hunger_timer,hd->master->bl.id,0);
	hd->regen.state.block = 0; //Restore HP/SP block.
}

bool homunculus_call(struct map_session_data *sd) {
	struct homun_data *hd;

	nullpo_retr(false, sd);
	if (!sd->status.hom_id) //Create a new homun.
		return homun->creation_request(sd, HM_CLASS_BASE + rnd->value(0, 7));

	// If homunc not yet loaded, load it
	if (!sd->hd)
		return intif->homunculus_requestload(sd->status.account_id, sd->status.hom_id);

	hd = sd->hd;

	if (hd->homunculus.vaporize != HOM_ST_REST)
		return false; //Can't use this if homun wasn't vaporized.

	homun->init_timers(hd);
	hd->homunculus.vaporize = HOM_ST_ACTIVE;
	if (hd->bl.prev == NULL) { //Spawn him
		hd->bl.x = sd->bl.x;
		hd->bl.y = sd->bl.y;
		hd->bl.m = sd->bl.m;
		map->addblock(&hd->bl);
		clif->spawn(&hd->bl);
		clif->send_homdata(sd,SP_ACK,0);
		clif->hominfo(sd,hd,1);
		clif->hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif->homskillinfoblock(sd);
		if (battle_config.slaves_inherit_speed&1)
			status_calc_bl(&hd->bl, SCB_SPEED);
		homun->save(hd);
	} else
		//Warp him to master.
		unit->warp(&hd->bl,sd->bl.m, sd->bl.x, sd->bl.y,CLR_OUTSIGHT);
	return true;
}

// Receive homunculus data from char server
bool homunculus_recv_data(int account_id, const struct s_homunculus *sh, int flag)
{
	struct map_session_data *sd;
	struct homun_data *hd;

	nullpo_retr(false, sh);

	sd = map->id2sd(account_id);
	if (sd == NULL)
		return false;

	if (flag == 0) { // Failed to load
		sd->status.hom_id = 0;
		return false;
	}

	if (sd->status.char_id != sh->char_id && sd->status.hom_id != sh->hom_id)
		return false;

	if (sd->status.hom_id == 0) //Hom just created.
		sd->status.hom_id = sh->hom_id;

	if (sd->hd != NULL) {
		//uh? Overwrite the data.
		memcpy(&sd->hd->homunculus, sh, sizeof sd->hd->homunculus);
		sd->hd->homunculus.char_id = sd->status.char_id; // Correct char id if necessary.
	} else {
		homun->create(sd, sh);
	}

	hd = sd->hd;
	if(hd != NULL && hd->homunculus.hp && hd->homunculus.vaporize == HOM_ST_ACTIVE && hd->bl.prev == NULL && sd->bl.prev != NULL) {
		enum homun_type htype = homun->class2type(hd->homunculus.class_);

		map->addblock(&hd->bl);
		clif->spawn(&hd->bl);
		clif->send_homdata(sd,SP_ACK,0);
		clif->hominfo(sd,hd,1);
		clif->hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif->homskillinfoblock(sd);
		homun->init_timers(hd);
		/* force shuffle if your level is higher than the allowed */
		switch( htype ) {
			case HT_REG:
			case HT_EVO:
				if( hd->homunculus.level > battle_config.hom_max_level )
					homun->shuffle(hd);
				break;
			case HT_S:
				if( hd->homunculus.level > battle_config.hom_S_max_level )
					homun->shuffle(hd);
				break;
		}

	}
	return true;
}

// Ask homunculus creation to char server
bool homunculus_creation_request(struct map_session_data *sd, int class_) {
	struct s_homunculus hom;
	struct h_stats *base;
	int i;

	nullpo_retr(false, sd);

	i = homun->db_search(class_,HOMUNCULUS_CLASS);
	if (i == INDEX_NOT_FOUND)
		return false;

	memset(&hom, 0, sizeof(struct s_homunculus));
	//Initial data
	safestrncpy(hom.name, homun->dbs->db[i].name, NAME_LENGTH-1);
	hom.class_ = class_;
	hom.level = 1;
	hom.hunger = 32; //32%
	hom.intimacy = 2100; //21/1000
	hom.char_id = sd->status.char_id;

	hom.hp = 10 ;
	base = &homun->dbs->db[i].base;
	hom.max_hp = base->HP;
	hom.max_sp = base->SP;
	hom.str = base->str *10;
	hom.agi = base->agi *10;
	hom.vit = base->vit *10;
	hom.int_= base->int_*10;
	hom.dex = base->dex *10;
	hom.luk = base->luk *10;

	// Request homunculus creation
	intif->homunculus_create(sd->status.account_id, &hom);
	return true;
}

bool homunculus_ressurect(struct map_session_data* sd, unsigned char per, short x, short y) {
	struct homun_data* hd;
	nullpo_retr(false,sd);

	if (!sd->status.hom_id)
		return false; // no homunculus

	if (!sd->hd) //Load homun data;
		return intif->homunculus_requestload(sd->status.account_id, sd->status.hom_id);

	hd = sd->hd;

	nullpo_retr(false, hd);
	if (hd->homunculus.vaporize != HOM_ST_ACTIVE)
		return false; // vaporized homunculi need to be 'called'

	if (!status->isdead(&hd->bl))
		return false; // already alive

	homun->init_timers(hd);

	if (!hd->bl.prev) {
		//Add it back to the map.
		hd->bl.m = sd->bl.m;
		hd->bl.x = x;
		hd->bl.y = y;
		map->addblock(&hd->bl);
		clif->spawn(&hd->bl);
	}
	status->revive(&hd->bl, per, 0);
	return true;
}

void homunculus_revive(struct homun_data *hd, unsigned int hp, unsigned int sp) {
	struct map_session_data *sd;

	nullpo_retv(hd);
	sd = hd->master;
	hd->homunculus.hp = hd->battle_status.hp;
	if (!sd)
		return;
	clif->send_homdata(sd,SP_ACK,0);
	clif->hominfo(sd,hd,1);
	clif->hominfo(sd,hd,0);
	clif->homskillinfoblock(sd);
}
//Resets a homunc stats back to zero (but doesn't touches hunger or intimacy)
void homunculus_stat_reset(struct homun_data *hd) {
	struct s_homunculus_db *db;
	struct s_homunculus *hom;
	struct h_stats *base;
	nullpo_retv(hd);
	hom = &hd->homunculus;
	db = hd->homunculusDB;
	base = &db->base;
	hom->level = 1;
	hom->hp = 10;
	hom->max_hp = base->HP;
	hom->max_sp = base->SP;
	hom->str = base->str *10;
	hom->agi = base->agi *10;
	hom->vit = base->vit *10;
	hom->int_= base->int_*10;
	hom->dex = base->dex *10;
	hom->luk = base->luk *10;
	hom->exp = 0;
	hd->exp_next = homun->dbs->exptable[0];
	memset(&hd->homunculus.hskill, 0, sizeof hd->homunculus.hskill);
	hd->homunculus.skillpts = 0;
}

bool homunculus_shuffle(struct homun_data *hd) {
	struct map_session_data *sd;
	int lv, skillpts;
	unsigned int exp;
	struct s_skill b_skill[MAX_HOMUNSKILL];

	nullpo_retr(false, hd);
	if (!homun_alive(hd))
		return false;

	sd = hd->master;
	lv = hd->homunculus.level;
	exp = hd->homunculus.exp;
	memcpy(&b_skill, &hd->homunculus.hskill, sizeof(b_skill));
	skillpts = hd->homunculus.skillpts;

	//Reset values to level 1.
	homun->stat_reset(hd);

	//Level it back up
	do {
		hd->homunculus.exp += hd->exp_next;
	} while( hd->homunculus.level < lv && homun->levelup(hd) );

	if(hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		//Evolved bonuses
		struct s_homunculus *hom = &hd->homunculus;
		struct h_stats *max = &hd->homunculusDB->emax, *min = &hd->homunculusDB->emin;
		hom->max_hp += rnd->value(min->HP, max->HP);
		hom->max_sp += rnd->value(min->SP, max->SP);
		hom->str += 10*rnd->value(min->str, max->str);
		hom->agi += 10*rnd->value(min->agi, max->agi);
		hom->vit += 10*rnd->value(min->vit, max->vit);
		hom->int_+= 10*rnd->value(min->int_,max->int_);
		hom->dex += 10*rnd->value(min->dex, max->dex);
		hom->luk += 10*rnd->value(min->luk, max->luk);
	}

	hd->homunculus.exp = exp;
	memcpy(&hd->homunculus.hskill, &b_skill, sizeof(b_skill));
	hd->homunculus.skillpts = skillpts;
	clif->homskillinfoblock(sd);
	status_calc_homunculus(hd,SCO_NONE);
	status_percent_heal(&hd->bl, 100, 100);
	clif->specialeffect(&hd->bl,568,AREA);

	return true;
}

bool homunculus_read_db_sub(char* str[], int columns, int current) {
	int classid;
	struct s_homunculus_db *db;

	nullpo_retr(false, str);
	//Base Class,Evo Class
	classid = atoi(str[0]);
	if (classid < HM_CLASS_BASE || classid > HM_CLASS_MAX) {
		ShowError("homunculus_read_db_sub : Invalid class %d\n", classid);
		return false;
	}
	db = &homun->dbs->db[current];
	db->base_class = classid;
	classid = atoi(str[1]);
	if (classid < HM_CLASS_BASE || classid > HM_CLASS_MAX) {
		db->base_class = 0;
		ShowError("homunculus_read_db_sub : Invalid class %d\n", classid);
		return false;
	}
	db->evo_class = classid;
	//Name, Food, Hungry Delay, Base Size, Evo Size, Race, Element, ASPD
	safestrncpy(db->name,str[2],NAME_LENGTH-1);
	db->foodID = atoi(str[3]);
	db->hungryDelay = atoi(str[4]);
	db->base_size = atoi(str[5]);
	db->evo_size = atoi(str[6]);
	db->race = atoi(str[7]);
	db->element = atoi(str[8]);
	db->baseASPD = atoi(str[9]);
	//base HP, SP, str, agi, vit, int, dex, luk
	db->base.HP = atoi(str[10]);
	db->base.SP = atoi(str[11]);
	db->base.str = atoi(str[12]);
	db->base.agi = atoi(str[13]);
	db->base.vit = atoi(str[14]);
	db->base.int_= atoi(str[15]);
	db->base.dex = atoi(str[16]);
	db->base.luk = atoi(str[17]);
	//Growth Min/Max HP, SP, str, agi, vit, int, dex, luk
	db->gmin.HP = atoi(str[18]);
	db->gmax.HP = atoi(str[19]);
	db->gmin.SP = atoi(str[20]);
	db->gmax.SP = atoi(str[21]);
	db->gmin.str = atoi(str[22]);
	db->gmax.str = atoi(str[23]);
	db->gmin.agi = atoi(str[24]);
	db->gmax.agi = atoi(str[25]);
	db->gmin.vit = atoi(str[26]);
	db->gmax.vit = atoi(str[27]);
	db->gmin.int_= atoi(str[28]);
	db->gmax.int_= atoi(str[29]);
	db->gmin.dex = atoi(str[30]);
	db->gmax.dex = atoi(str[31]);
	db->gmin.luk = atoi(str[32]);
	db->gmax.luk = atoi(str[33]);
	//Evolution Min/Max HP, SP, str, agi, vit, int, dex, luk
	db->emin.HP = atoi(str[34]);
	db->emax.HP = atoi(str[35]);
	db->emin.SP = atoi(str[36]);
	db->emax.SP = atoi(str[37]);
	db->emin.str = atoi(str[38]);
	db->emax.str = atoi(str[39]);
	db->emin.agi = atoi(str[40]);
	db->emax.agi = atoi(str[41]);
	db->emin.vit = atoi(str[42]);
	db->emax.vit = atoi(str[43]);
	db->emin.int_= atoi(str[44]);
	db->emax.int_= atoi(str[45]);
	db->emin.dex = atoi(str[46]);
	db->emax.dex = atoi(str[47]);
	db->emin.luk = atoi(str[48]);
	db->emax.luk = atoi(str[49]);

	//Check that the min/max values really are below the other one.
	if(db->gmin.HP > db->gmax.HP)
		db->gmin.HP = db->gmax.HP;
	if(db->gmin.SP > db->gmax.SP)
		db->gmin.SP = db->gmax.SP;
	if(db->gmin.str > db->gmax.str)
		db->gmin.str = db->gmax.str;
	if(db->gmin.agi > db->gmax.agi)
		db->gmin.agi = db->gmax.agi;
	if(db->gmin.vit > db->gmax.vit)
		db->gmin.vit = db->gmax.vit;
	if(db->gmin.int_> db->gmax.int_)
		db->gmin.int_= db->gmax.int_;
	if(db->gmin.dex > db->gmax.dex)
		db->gmin.dex = db->gmax.dex;
	if(db->gmin.luk > db->gmax.luk)
		db->gmin.luk = db->gmax.luk;

	if(db->emin.HP > db->emax.HP)
		db->emin.HP = db->emax.HP;
	if(db->emin.SP > db->emax.SP)
		db->emin.SP = db->emax.SP;
	if(db->emin.str > db->emax.str)
		db->emin.str = db->emax.str;
	if(db->emin.agi > db->emax.agi)
		db->emin.agi = db->emax.agi;
	if(db->emin.vit > db->emax.vit)
		db->emin.vit = db->emax.vit;
	if(db->emin.int_> db->emax.int_)
		db->emin.int_= db->emax.int_;
	if(db->emin.dex > db->emax.dex)
		db->emin.dex = db->emax.dex;
	if(db->emin.luk > db->emax.luk)
		db->emin.luk = db->emax.luk;

	return true;
}

void homunculus_read_db(void) {
	int i;
	const char *filename[]={DBPATH"homunculus_db.txt","homunculus_db2.txt"};
	memset(homun->dbs->db, 0, sizeof(homun->dbs->db));
	for(i = 0; i<ARRAYLENGTH(filename); i++) {
		if( i > 0 ) {
			char filepath[256];

			safesnprintf(filepath, 256, "%s/%s", map->db_path, filename[i]);

			if( !exists(filepath) ) {
				continue;
			}
		}

		sv->readdb(map->db_path, filename[i], ',', 50, 50, MAX_HOMUNCULUS_CLASS, homun->read_db_sub);
	}

}
// <hom class>,<skill id>,<max level>[,<job level>],<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>,<intimacy lv req>
bool homunculus_read_skill_db_sub(char* split[], int columns, int current) {
	int k, classid;
	int j;
	int minJobLevelPresent = 0;

	nullpo_retr(false, split);
	if( columns == 15 )
		minJobLevelPresent = 1; // MinJobLvl has been added - FIXME: is this extra field even needed anymore?

	// check for bounds [celest]
	classid = atoi(split[0]) - HM_CLASS_BASE;

	if ( classid < 0 || classid >= MAX_HOMUNCULUS_CLASS ) {
		ShowWarning("homunculus_read_skill_db_sub: Invalid homunculus class %d.\n", atoi(split[0]));
		return false;
	}

	k = atoi(split[1]); //This is to avoid adding two lines for the same skill. [Skotlex]
	// Search an empty line or a line with the same skill_id (stored in j)
	ARR_FIND( 0, MAX_SKILL_TREE, j, !homun->dbs->skill_tree[classid][j].id || homun->dbs->skill_tree[classid][j].id == k );
	if (j == MAX_SKILL_TREE) {
		ShowWarning("Unable to load skill %d into homunculus %d's tree. Maximum number of skills per class has been reached.\n", k, classid);
		return false;
	}

	homun->dbs->skill_tree[classid][j].id = k;
	homun->dbs->skill_tree[classid][j].max = atoi(split[2]);
	if (minJobLevelPresent)
		homun->dbs->skill_tree[classid][j].joblv = atoi(split[3]);

	for (k = 0; k < MAX_HOM_SKILL_REQUIRE; k++) {
		homun->dbs->skill_tree[classid][j].need[k].id = atoi(split[3+k*2+minJobLevelPresent]);
		homun->dbs->skill_tree[classid][j].need[k].lv = atoi(split[3+k*2+minJobLevelPresent+1]);
	}

	homun->dbs->skill_tree[classid][j].intimacylv = atoi(split[13+minJobLevelPresent]);

	return true;
}

int8 homunculus_get_intimacy_grade(struct homun_data *hd) {
	unsigned int val;
	nullpo_ret(hd);
	val = hd->homunculus.intimacy / 100;
	if( val > 100 ) {
		if( val > 250 ) {
			if( val > 750 ) {
				if ( val > 900 )
					return 4;
				else
					return 3;
			} else
				return 2;
		} else
			return 1;
	}

	return 0;
}

void homunculus_skill_db_read(void) {
	memset(homun->dbs->skill_tree, 0, sizeof(homun->dbs->skill_tree));
	sv->readdb(map->db_path, "homun_skill_tree.txt", ',', 13, 15, -1, homun->read_skill_db_sub);
}

void homunculus_exp_db_read(void) {
	char line[1024];
	int i, j=0;
	char *filename[]={
		DBPATH"exp_homun.txt",
		"exp_homun2.txt"};

	memset(homun->dbs->exptable, 0, sizeof(homun->dbs->exptable));
	for(i = 0; i < 2; i++) {
		FILE *fp;
		sprintf(line, "%s/%s", map->db_path, filename[i]);
		if( (fp=fopen(line,"r")) == NULL) {
			if(i != 0)
				continue;
			ShowError("can't read %s\n",line);
			return;
		}
		while(fgets(line, sizeof(line), fp) && j < MAX_LEVEL) {
			if(line[0] == '/' && line[1] == '/')
				continue;

			if (!(homun->dbs->exptable[j++] = (unsigned int)strtoul(line, NULL, 10)))
				break;
		}
		// Last permitted level have to be 0!
		if (homun->dbs->exptable[MAX_LEVEL - 1]) {
			ShowWarning("homunculus_exp_db_read: Reached max level in exp_homun [%d]. Remaining lines were not read.\n ", MAX_LEVEL);
			homun->dbs->exptable[MAX_LEVEL - 1] = 0;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' levels in '"CL_WHITE"%s"CL_RESET"'.\n", j, filename[i]);
	}
}

void homunculus_reload(void) {
	homun->read_db();
	homun->exp_db_read();
}

void homunculus_skill_reload(void) {
	homun->skill_db_read();
}

void do_init_homunculus(bool minimal) {
	int class_;

	if (minimal)
		return;

	homun->read_db();
	homun->exp_db_read();
	homun->skill_db_read();
	// Add homunc timer function to timer func list [Toms]
	timer->add_func_list(homun->hunger_timer, "homunculus_hunger_timer");

	//Stock view data for homuncs
	memset(homun->dbs->viewdb, 0, sizeof(homun->dbs->viewdb));
	for (class_ = 0; class_ < MAX_HOMUNCULUS_CLASS; class_++)
		homun->dbs->viewdb[class_].class = HM_CLASS_BASE + class_;
}

void do_final_homunculus(void) {

}

void homunculus_defaults(void) {
	homun = &homunculus_s;
	homun->dbs = &homundbs;

	homun->init = do_init_homunculus;
	homun->final = do_final_homunculus;
	homun->reload = homunculus_reload;
	homun->reload_skill = homunculus_skill_reload;
	/* */
	homun->get_viewdata = homunculus_get_viewdata;
	homun->class2type = homunculus_class2type;
	homun->damaged = homunculus_damaged;
	homun->dead = homunculus_dead;
	homun->vaporize = homunculus_vaporize;
	homun->delete = homunculus_delete;
	homun->checkskill = homunculus_checkskill;
	homun->calc_skilltree = homunculus_calc_skilltree;
	homun->skill_tree_get_max = homunculus_skill_tree_get_max;
	homun->skillup = homunculus_skillup;
	homun->levelup = homunculus_levelup;
	homun->change_class = homunculus_change_class;
	homun->evolve = homunculus_evolve;
	homun->mutate = homunculus_mutate;
	homun->gainexp = homunculus_gainexp;
	homun->add_intimacy = homunculus_add_intimacy;
	homun->consume_intimacy = homunculus_consume_intimacy;
	homun->healed = homunculus_healed;
	homun->save = homunculus_save;
	homun->menu = homunculus_menu;
	homun->feed = homunculus_feed;
	homun->hunger_timer = homunculus_hunger_timer;
	homun->hunger_timer_delete = homunculus_hunger_timer_delete;
	homun->change_name = homunculus_change_name;
	homun->change_name_ack = homunculus_change_name_ack;
	homun->db_search = homunculus_db_search;
	homun->create = homunculus_create;
	homun->init_timers = homunculus_init_timers;
	homun->call = homunculus_call;
	homun->recv_data = homunculus_recv_data;
	homun->creation_request = homunculus_creation_request;
	homun->ressurect = homunculus_ressurect;
	homun->revive = homunculus_revive;
	homun->stat_reset = homunculus_stat_reset;
	homun->shuffle = homunculus_shuffle;
	homun->read_db_sub = homunculus_read_db_sub;
	homun->read_db = homunculus_read_db;
	homun->read_skill_db_sub = homunculus_read_skill_db_sub;
	homun->skill_db_read = homunculus_skill_db_read;
	homun->exp_db_read = homunculus_exp_db_read;
	homun->addspiritball = homunculus_addspiritball;
	homun->delspiritball = homunculus_delspiritball;
	homun->get_intimacy_grade = homunculus_get_intimacy_grade;
}
