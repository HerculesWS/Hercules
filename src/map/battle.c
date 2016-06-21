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

#include "config/core.h" // CELL_NOSTACK, CIRCULAR_AREA, CONSOLE_INPUT, HMAP_ZONE_DAMAGE_CAP_TYPE, OFFICIAL_WALKPATH, RENEWAL, RENEWAL_ASPD, RENEWAL_CAST, RENEWAL_DROP, RENEWAL_EDP, RENEWAL_EXP, RENEWAL_LVDMG, RE_LVL_DMOD(), RE_LVL_MDMOD(), RE_LVL_TMDMOD(), RE_SKILL_REDUCTION(), SCRIPT_CALLFUNC_CHECK, SECURE_NPCTIMEOUT, STATS_OPT_OUT
#include "battle.h"

#include "map/battleground.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/elemental.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/party.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/skill.h"
#include "map/status.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "common/timer.h"
#include "common/utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Battle_Config battle_config;
struct battle_interface battle_s;
struct battle_interface *battle;

/**
 * Returns the current/last skill in use by this bl.
 *
 * @param bl The bl to check.
 * @return The current/last skill ID.
 */
int battle_getcurrentskill(struct block_list *bl)
{
	const struct unit_data *ud;

	nullpo_ret(bl);
	if (bl->type == BL_SKILL) {
		const struct skill_unit *su = BL_UCCAST(BL_SKILL, bl);
		if (su->group == NULL)
			return 0;
		return su->group->skill_id;
	}

	ud = unit->bl2ud(bl);

	if (ud == NULL)
		return 0;

	return ud->skill_id;
}

/*==========================================
 * Get random targeting enemy
 *------------------------------------------*/
int battle_gettargeted_sub(struct block_list *bl, va_list ap) {
	struct block_list **bl_list;
	struct unit_data *ud;
	int target_id;
	int *c;

	nullpo_ret(bl);
	bl_list = va_arg(ap, struct block_list **);
	c = va_arg(ap, int *);
	target_id = va_arg(ap, int);

	if (bl->id == target_id)
		return 0;

	if (*c >= 24)
		return 0;

	if (!(ud = unit->bl2ud(bl)))
		return 0;

	if (ud->target == target_id || ud->skilltarget == target_id) {
		bl_list[(*c)++] = bl;
		return 1;
	}

	return 0;
}

struct block_list* battle_gettargeted(struct block_list *target) {
	struct block_list *bl_list[24];
	int c = 0;
	nullpo_retr(NULL, target);

	memset(bl_list, 0, sizeof(bl_list));
	map->foreachinrange(battle->get_targeted_sub, target, AREA_SIZE, BL_CHAR, bl_list, &c, target->id);
	if ( c == 0 )
		return NULL;
	if( c > 24 )
		c = 24;
	return bl_list[rnd()%c];
}

//Returns the id of the current targeted character of the passed bl. [Skotlex]
int battle_gettarget(struct block_list* bl) {

	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:  return BL_UCCAST(BL_PC, bl)->ud.target;
		case BL_MOB: return BL_UCCAST(BL_MOB, bl)->target_id;
		case BL_PET: return BL_UCCAST(BL_PET, bl)->target_id;
		case BL_HOM: return BL_UCCAST(BL_HOM, bl)->ud.target;
		case BL_MER: return BL_UCCAST(BL_MER, bl)->ud.target;
		case BL_ELEM: return BL_UCCAST(BL_ELEM, bl)->ud.target;
	}

	return 0;
}

int battle_getenemy_sub(struct block_list *bl, va_list ap) {
	struct block_list **bl_list;
	struct block_list *target;
	int *c;

	nullpo_ret(bl);
	bl_list = va_arg(ap, struct block_list **);
	c = va_arg(ap, int *);
	target = va_arg(ap, struct block_list *);

	if (bl->id == target->id)
		return 0;

	if (*c >= 24)
		return 0;

	if (status->isdead(bl))
		return 0;

	if (battle->check_target(target, bl, BCT_ENEMY) > 0) {
		bl_list[(*c)++] = bl;
		return 1;
	}

	return 0;
}

// Picks a random enemy of the given type (BL_PC, BL_CHAR, etc) within the range given. [Skotlex]
struct block_list* battle_getenemy(struct block_list *target, int type, int range) {
	struct block_list *bl_list[24];
	int c = 0;

	nullpo_retr(NULL, target);
	memset(bl_list, 0, sizeof(bl_list));
	map->foreachinrange(battle->get_enemy_sub, target, range, type, bl_list, &c, target);

	if ( c == 0 )
		return NULL;

	if( c > 24 )
		c = 24;

	return bl_list[rnd()%c];
}
int battle_getenemyarea_sub(struct block_list *bl, va_list ap) {
	struct block_list **bl_list, *src;
	int *c, ignore_id;

	nullpo_ret(bl);
	bl_list = va_arg(ap, struct block_list **);
	nullpo_ret(bl_list);
	c = va_arg(ap, int *);
	nullpo_ret(c);
	src = va_arg(ap, struct block_list *);
	ignore_id = va_arg(ap, int);

	if( bl->id == src->id || bl->id == ignore_id )
		return 0; // Ignores Caster and a possible pre-target

	if( *c >= 23 )
		return 0;

	if( status->isdead(bl) )
		return 0;

	if( battle->check_target(src, bl, BCT_ENEMY) > 0 ) {// Is Enemy!...
		bl_list[(*c)++] = bl;
		return 1;
	}

	return 0;
}

// Pick a random enemy
struct block_list* battle_getenemyarea(struct block_list *src, int x, int y, int range, int type, int ignore_id) {
	struct block_list *bl_list[24];
	int c = 0;

	nullpo_retr(NULL, src);
	memset(bl_list, 0, sizeof(bl_list));
	map->foreachinarea(battle->get_enemy_area_sub, src->m, x - range, y - range, x + range, y + range, type, bl_list, &c, src, ignore_id);

	if( c == 0 )
		return NULL;
	if( c >= 24 )
		c = 23;

	return bl_list[rnd()%c];
}

int battle_delay_damage_sub(int tid, int64 tick, int id, intptr_t data) {
	struct delay_damage *dat = (struct delay_damage *)data;

	if ( dat ) {
		struct block_list *src = map->id2bl(dat->src_id);
		struct map_session_data *sd = BL_CAST(BL_PC, src);
		struct block_list *target = map->id2bl(dat->target_id);

		if (target != NULL && !status->isdead(target)) {
			//Check to see if you haven't teleported. [Skotlex]
			if (src != NULL && (
			    battle_config.fix_warp_hit_delay_abuse ?
			    (dat->skill_id == MO_EXTREMITYFIST || target->m != src->m || check_distance_bl(src, target, dat->distance))
			    :
			    ((target->type != BL_PC || BL_UCAST(BL_PC, target)->invincible_timer == INVALID_TIMER)
			    && (dat->skill_id == MO_EXTREMITYFIST || (target->m == src->m && check_distance_bl(src, target, dat->distance))))
			)) {
				map->freeblock_lock();
				status_fix_damage(src, target, dat->damage, dat->delay);
				if (dat->attack_type && !status->isdead(target) && dat->additional_effects)
					skill->additional_effect(src,target,dat->skill_id,dat->skill_lv,dat->attack_type,dat->dmg_lv,tick);
				if (dat->dmg_lv > ATK_BLOCK && dat->attack_type)
					skill->counter_additional_effect(src,target,dat->skill_id,dat->skill_lv,dat->attack_type,tick);
				map->freeblock_unlock();
			} else if (src == NULL && dat->skill_id == CR_REFLECTSHIELD) {
				// it was monster reflected damage, and the monster died, we pass the damage to the character as expected
				map->freeblock_lock();
				status_fix_damage(target, target, dat->damage, dat->delay);
				map->freeblock_unlock();
			}
		}

		if (sd != NULL && --sd->delayed_damage == 0 && sd->state.hold_recalc) {
			sd->state.hold_recalc = 0;
			status_calc_pc(sd, SCO_FORCE);
		}
	}
	ers_free(battle->delay_damage_ers, dat);
	return 0;
}

int battle_delay_damage(int64 tick, int amotion, struct block_list *src, struct block_list *target, int attack_type, uint16 skill_id, uint16 skill_lv, int64 damage, enum damage_lv dmg_lv, int ddelay, bool additional_effects) {
	struct delay_damage *dat;
	struct status_change *sc;
	struct block_list *d_tbl = NULL;
	nullpo_ret(src);
	nullpo_ret(target);

	sc = status->get_sc(target);

	if (sc && sc->data[SC_DEVOTION] && sc->data[SC_DEVOTION]->val1)
		d_tbl = map->id2bl(sc->data[SC_DEVOTION]->val1);

	if (d_tbl && sc && check_distance_bl(target, d_tbl, sc->data[SC_DEVOTION]->val3) && damage > 0 && skill_id != PA_PRESSURE && skill_id != CR_REFLECTSHIELD)
		damage = 0;

	if ( !battle_config.delay_battle_damage || amotion <= 1 ) {
		map->freeblock_lock();
		status_fix_damage(src, target, damage, ddelay); // We have to separate here between reflect damage and others [icescope]
		if( attack_type && !status->isdead(target) && additional_effects )
			skill->additional_effect(src, target, skill_id, skill_lv, attack_type, dmg_lv, timer->gettick());
		if( dmg_lv > ATK_BLOCK && attack_type )
			skill->counter_additional_effect(src, target, skill_id, skill_lv, attack_type, timer->gettick());
		map->freeblock_unlock();
		return 0;
	}
	dat = ers_alloc(battle->delay_damage_ers, struct delay_damage);
	dat->src_id = src->id;
	dat->target_id = target->id;
	dat->skill_id = skill_id;
	dat->skill_lv = skill_lv;
	dat->attack_type = attack_type;
	dat->damage = damage;
	dat->dmg_lv = dmg_lv;
	dat->delay = ddelay;
	dat->distance = distance_bl(src, target) + (battle_config.snap_dodge ? 10 : battle_config.area_size);
	dat->additional_effects = additional_effects;
	dat->src_type = src->type;
	if (src->type != BL_PC && amotion > 1000)
		amotion = 1000; //Aegis places a damage-delay cap of 1 sec to non player attacks. [Skotlex]

	if (src->type == BL_PC) {
		BL_UCAST(BL_PC, src)->delayed_damage++;
	}

	timer->add(tick+amotion, battle->delay_damage_sub, 0, (intptr_t)dat);

	return 0;
}
int battle_attr_ratio(int atk_elem,int def_type, int def_lv)
{
	if (atk_elem < ELE_NEUTRAL || atk_elem >= ELE_MAX)
		return 100;

	if (def_type < ELE_NEUTRAL || def_type >= ELE_MAX || def_lv < 1 || def_lv > 4)
		return 100;

	return battle->attr_fix_table[def_lv-1][atk_elem][def_type];
}

/*==========================================
 * Does attribute fix modifiers.
 * Added passing of the chars so that the status changes can affect it. [Skotlex]
 * Note: Passing src/target == NULL is perfectly valid, it skips SC_ checks.
 *------------------------------------------*/
int64 battle_attr_fix(struct block_list *src, struct block_list *target, int64 damage,int atk_elem,int def_type, int def_lv)
{
	struct status_change *sc=NULL, *tsc=NULL;
	int ratio;

	if (src) sc = status->get_sc(src);
	if (target) tsc = status->get_sc(target);

	if (atk_elem < ELE_NEUTRAL || atk_elem >= ELE_MAX)
		atk_elem = rnd()%ELE_MAX;

	if (def_type < ELE_NEUTRAL || def_type >= ELE_MAX ||
		def_lv < 1 || def_lv > 4) {
		ShowError("battle_attr_fix: unknown attr type: atk=%d def_type=%d def_lv=%d\n",atk_elem,def_type,def_lv);
		return damage;
	}

	ratio = battle->attr_fix_table[def_lv-1][atk_elem][def_type];
	if (sc && sc->count) {
		if(sc->data[SC_VOLCANO] && atk_elem == ELE_FIRE)
			ratio += skill->enchant_eff[sc->data[SC_VOLCANO]->val1-1];
		if(sc->data[SC_VIOLENTGALE] && atk_elem == ELE_WIND)
			ratio += skill->enchant_eff[sc->data[SC_VIOLENTGALE]->val1-1];
		if(sc->data[SC_DELUGE] && atk_elem == ELE_WATER)
			ratio += skill->enchant_eff[sc->data[SC_DELUGE]->val1-1];
		if(sc->data[SC_FIRE_CLOAK_OPTION] && atk_elem == ELE_FIRE)
			damage += damage * sc->data[SC_FIRE_CLOAK_OPTION]->val2 / 100;
	}
	if( target && target->type == BL_SKILL ) {
		if( atk_elem == ELE_FIRE && battle->get_current_skill(target) == GN_WALLOFTHORN ) {
			struct skill_unit *su = BL_UCAST(BL_SKILL, target);
			struct skill_unit_group *sg;
			struct block_list *sgsrc;

			if(!su->alive
			 || (sg = su->group) == NULL || sg->val3 == -1
			 || (sgsrc = map->id2bl(sg->src_id)) == NULL || status->isdead(sgsrc)
			)
				return 0;

			if( sg->unit_id != UNT_FIREWALL ) {
				int x,y;
				x = sg->val3 >> 16;
				y = sg->val3 & 0xffff;
				skill->unitsetting(sgsrc,su->group->skill_id,su->group->skill_lv,x,y,1);
				sg->val3 = -1;
				sg->limit = DIFF_TICK32(timer->gettick(),sg->tick)+300;
			}
		}
	}
	if( tsc && tsc->count ) { //since an atk can only have one type let's optimize this a bit
		switch(atk_elem){
		case ELE_FIRE:
			if( tsc->data[SC_SPIDERWEB]) {
				tsc->data[SC_SPIDERWEB]->val1 = 0; // free to move now
				if( tsc->data[SC_SPIDERWEB]->val2-- > 0 )
					damage <<= 1; // double damage
				if( tsc->data[SC_SPIDERWEB]->val2 == 0 )
					status_change_end(target, SC_SPIDERWEB, INVALID_TIMER);
			}
			if( tsc->data[SC_THORNS_TRAP])
				status_change_end(target, SC_THORNS_TRAP, INVALID_TIMER);
			if( tsc->data[SC_COLD] && target->type != BL_MOB)
				status_change_end(target, SC_COLD, INVALID_TIMER);
			if( tsc->data[SC_EARTH_INSIGNIA]) damage += damage/2;
			if( tsc->data[SC_FIRE_CLOAK_OPTION])
				damage -= damage * tsc->data[SC_FIRE_CLOAK_OPTION]->val2 / 100;
			if( tsc->data[SC_VOLCANIC_ASH]) damage += damage/2; //150%
			break;
		case ELE_HOLY:
			if( tsc->data[SC_ORATIO]) ratio += tsc->data[SC_ORATIO]->val1 * 2;
			break;
		case ELE_POISON:
			if( tsc->data[SC_VENOMIMPRESS] && atk_elem == ELE_POISON ) ratio += tsc->data[SC_VENOMIMPRESS]->val2;
			break;
		case ELE_WIND:
			if( tsc->data[SC_COLD] && target->type != BL_MOB) damage += damage/2;
			if( tsc->data[SC_WATER_INSIGNIA]) damage += damage/2;
			break;
		case ELE_WATER:
			if( tsc->data[SC_FIRE_INSIGNIA]) damage += damage/2;
			break;
		case ELE_EARTH:
			if( tsc->data[SC_WIND_INSIGNIA]) damage += damage/2;
			break;
		}
	} //end tsc check

	if( ratio < 100 )
		return damage - (damage * (100 - ratio) / 100);
	else
		return damage + (damage * (ratio - 100) / 100);
}

//FIXME: Missing documentation for flag, flag2
int64 battle_calc_weapon_damage(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, struct weapon_atk *watk, int nk, bool n_ele, short s_ele, short s_ele_, int size, int type, int flag, int flag2){ // [malufett]
#ifdef RENEWAL
	int64 damage, eatk = 0;
	struct status_change *sc;
	struct map_session_data *sd;

	if( !src || !bl )
		return 0;

	sc = status->get_sc(src);
	sd = BL_CAST(BL_PC, src);

	damage = status->get_weapon_atk(src, watk, flag);

	if ( sd ) {
		if ( type == EQI_HAND_R )
			damage = battle->calc_sizefix(sd, damage, EQI_HAND_R, size, flag & 8);
		else
			damage = battle->calc_sizefix(sd, damage, EQI_HAND_L, size, flag & 8);

		if ( flag & 2 && sd->bonus.arrow_atk && skill_id != GN_CARTCANNON )
			damage += sd->bonus.arrow_atk;

		if ( sd->battle_status.equip_atk != 0 )
			eatk = sd->base_status.equip_atk;

		if ( sd->bonus.atk_rate )
			damage += damage * sd->bonus.atk_rate / 100;
	}

	if ( skill_id == TF_POISON )
		eatk += 15 * skill_lv;

	if ( skill_id != ASC_METEORASSAULT ) {
		if ( sc && sc->data[SC_SUB_WEAPONPROPERTY] ) // Temporary. [malufett]
			damage += damage * sc->data[SC_SUB_WEAPONPROPERTY]->val2 / 100;
	}

	if( sc && sc->count ){
		if( sc->data[SC_ZENKAI] && watk->ele == sc->data[SC_ZENKAI]->val2 )
			eatk += 200;
	}

#ifdef RENEWAL_EDP
	if ( sc && sc->data[SC_EDP] && skill_id != AS_GRIMTOOTH && skill_id != AS_VENOMKNIFE && skill_id != ASC_BREAKER ) {
		struct status_data *tstatus;
		tstatus = status->get_status_data(bl);
		eatk += damage * 0x19 * battle->attr_fix_table[tstatus->ele_lv - 1][ELE_POISON][tstatus->def_ele] / 10000;
		damage += (eatk + damage) * sc->data[SC_EDP]->val3 / 100 + eatk;
	} else /* fall through */
#endif
	damage += eatk;
	damage = battle->calc_elefix(src, bl, skill_id, skill_lv, damage, nk, n_ele, s_ele, s_ele_, type == EQI_HAND_L, flag);

	/**
	 * In RE Shield Boomerang takes weapon element only for damage calculation,
	 * - resist calculation is always against neutral
	**/
	if ( skill_id == CR_SHIELDBOOMERANG )
		s_ele = s_ele_ = ELE_NEUTRAL;

	// attacker side
	damage = battle->calc_cardfix(BF_WEAPON, src, bl, nk, s_ele, s_ele_, damage, 2|(type == EQI_HAND_L), flag2);

	// target side
	damage = battle->calc_cardfix(BF_WEAPON, src, bl, nk, s_ele, s_ele_, damage, 0, flag2);

	return damage;
#else
	return 0;
#endif
}
/*==========================================
 * Calculates the standard damage of a normal attack assuming it hits,
 * it calculates nothing extra fancy, is needed for magnum breaks WATK_ELEMENT bonus. [Skotlex]
 *------------------------------------------
 * Pass damage2 as NULL to not calc it.
 * Flag values: // TODO: Check whether these values are correct (the flag parameter seems to be passed through to other functions), and replace them with an enum.
 * &1: Critical hit
 * &2: Arrow attack
 * &4: Skill is Magic Crasher
 * &8: Skip target size adjustment (Extremity Fist?)
 *&16: Arrow attack but BOW, REVOLVER, RIFLE, SHOTGUN, GATLING or GRENADE type weapon not equipped (i.e. shuriken, kunai and venom knives not affected by DEX)
 */
/* 'battle_calc_base_damage' is used on renewal, 'battle_calc_base_damage2' otherwise. */
// FIXME: Missing documentation for flag2
int64 battle_calc_base_damage(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int nk, bool n_ele, short s_ele, short s_ele_, int type, int flag, int flag2) {
	int64 damage;
	struct status_data *st = status->get_status_data(src);
	struct status_change *sc = status->get_sc(src);
	const struct map_session_data *sd = NULL;
	nullpo_retr(0, src);

	sd = BL_CCAST(BL_PC, src);

	if ( !skill_id ) {
		s_ele = st->rhw.ele;
		s_ele_ = st->lhw.ele;
		if (sd != NULL) {
			if (sd->charm_type != CHARM_TYPE_NONE && sd->charm_count >= MAX_SPIRITCHARM) {
				s_ele = s_ele_ = sd->charm_type;
			}
			if (flag&2 && sd->bonus.arrow_ele != 0)
				s_ele = sd->bonus.arrow_ele;
		}
	}
	if (src->type == BL_PC) {
		int64 batk;
		// Property from mild wind bypasses it
		if (sc && sc->data[SC_TK_SEVENWIND])
			batk = battle->calc_elefix(src, bl, skill_id, skill_lv, status->calc_batk(bl, sc, st->batk, false), nk, n_ele, s_ele, s_ele_, false, flag);
		else
			batk = battle->calc_elefix(src, bl, skill_id, skill_lv, status->calc_batk(bl, sc, st->batk, false), nk, n_ele, ELE_NEUTRAL, ELE_NEUTRAL, false, flag);
		if (type == EQI_HAND_L)
			damage = batk + 3 * battle->calc_weapon_damage(src, bl, skill_id, skill_lv, &st->lhw, nk, n_ele, s_ele, s_ele_, status_get_size(bl), type, flag, flag2) / 4;
		else
			damage = (batk << 1) + battle->calc_weapon_damage(src, bl, skill_id, skill_lv, &st->rhw, nk, n_ele, s_ele, s_ele_, status_get_size(bl), type, flag, flag2);
	} else {
		damage = st->batk + battle->calc_weapon_damage(src, bl, skill_id, skill_lv, &st->rhw, nk, n_ele, s_ele, s_ele_, status_get_size(bl), type, flag, flag2);
	}

	return damage;
}
int64 battle_calc_base_damage2(struct status_data *st, struct weapon_atk *wa, struct status_change *sc, unsigned short t_size, struct map_session_data *sd, int flag) {
	unsigned int atkmin=0, atkmax=0;
	short type = 0;
	int64 damage = 0;

	nullpo_retr(damage, st);
	nullpo_retr(damage, wa);
	if (!sd) { //Mobs/Pets
		if(flag&4) {
			atkmin = st->matk_min;
			atkmax = st->matk_max;
		} else {
			atkmin = wa->atk;
			atkmax = wa->atk2;
		}
		if (atkmin > atkmax)
			atkmin = atkmax;
	} else { //PCs
		atkmax = wa->atk;
		type = (wa == &st->lhw)?EQI_HAND_L:EQI_HAND_R;

		if (!(flag&1) || (flag&2)) { //Normal attacks
			atkmin = st->dex;

			if (sd->equip_index[type] >= 0 && sd->inventory_data[sd->equip_index[type]])
				atkmin = atkmin*(80 + sd->inventory_data[sd->equip_index[type]]->wlv*20)/100;

			if (atkmin > atkmax)
				atkmin = atkmax;

			if(flag&2 && !(flag&16)) { //Bows
				atkmin = atkmin*atkmax/100;
				if (atkmin > atkmax)
					atkmax = atkmin;
			}
		}
	}

	if (sc && sc->data[SC_MAXIMIZEPOWER])
		atkmin = atkmax;

	//Weapon Damage calculation
	if (!(flag&1))
		damage = (atkmax>atkmin? rnd()%(atkmax-atkmin):0)+atkmin;
	else
		damage = atkmax;

	if (sd) {
		//rodatazone says the range is 0~arrow_atk-1 for non crit
		if (flag&2 && sd->bonus.arrow_atk)
			damage += ( (flag&1) ? sd->bonus.arrow_atk : rnd()%sd->bonus.arrow_atk );

		//SizeFix only for players
		if (!(sd->special_state.no_sizefix || (flag&8)))
			damage = damage * ( type == EQI_HAND_L ? sd->left_weapon.atkmods[t_size] : sd->right_weapon.atkmods[t_size] ) / 100;
	}

	//Finally, add baseatk
	if(flag&4)
		damage += st->matk_min;
	else
		damage += st->batk;

	//rodatazone says that Overrefined bonuses are part of baseatk
	//Here we also apply the weapon_atk_rate bonus so it is correctly applied on left/right hands.
	if(sd) {
		if (type == EQI_HAND_L) {
			if(sd->left_weapon.overrefine)
				damage += rnd()%sd->left_weapon.overrefine+1;
			if (sd->weapon_atk_rate[sd->weapontype2])
				damage += damage * sd->weapon_atk_rate[sd->weapontype2] / 100;
		} else { //Right hand
			if(sd->right_weapon.overrefine)
				damage += rnd()%sd->right_weapon.overrefine+1;
			if (sd->weapon_atk_rate[sd->weapontype1])
				damage += damage * sd->weapon_atk_rate[sd->weapontype1] / 100;
		}
	}
	return damage;
}

int64 battle_calc_sizefix(struct map_session_data *sd, int64 damage, int type, int size,  bool ignore){
	//SizeFix only for players
	nullpo_retr(damage, sd);
	if (!(sd->special_state.no_sizefix || (ignore)))
		damage = damage * ( type == EQI_HAND_L ? sd->left_weapon.atkmods[size] : sd->right_weapon.atkmods[size] ) / 100;
	return damage;
}

/*==========================================
 * Passive skill damages increases
 *------------------------------------------*/
// FIXME: type is undocumented
int64 battle_addmastery(struct map_session_data *sd,struct block_list *target,int64 dmg,int type) {
	int64 damage;
	struct status_data *st = status->get_status_data(target);
	int weapon, skill_lv;
	damage = dmg;

	nullpo_retr(damage, sd);
	nullpo_retr(damage, target);
	if((skill_lv = pc->checkskill(sd,AL_DEMONBANE)) > 0 &&
		target->type == BL_MOB && //This bonus doesn't work against players.
		(battle->check_undead(st->race,st->def_ele) || st->race==RC_DEMON) )
		damage += (int)(skill_lv*(3+sd->status.base_level/20.0));
		//damage += (skill_lv * 3);
	if( (skill_lv = pc->checkskill(sd, RA_RANGERMAIN)) > 0 && (st->race == RC_BRUTE || st->race == RC_PLANT || st->race == RC_FISH) )
		damage += (skill_lv * 5);
	if( (skill_lv = pc->checkskill(sd,NC_RESEARCHFE)) > 0 && (st->def_ele == ELE_FIRE || st->def_ele == ELE_EARTH) )
		damage += (skill_lv * 10);
	if( pc_ismadogear(sd) )
		damage += 15 * pc->checkskill(sd, NC_MADOLICENCE);
#ifdef RENEWAL
	if( (skill_lv = pc->checkskill(sd,BS_WEAPONRESEARCH)) > 0 )
		damage += (skill_lv * 2);
#endif
	if((skill_lv = pc->checkskill(sd,HT_BEASTBANE)) > 0 && (st->race==RC_BRUTE || st->race==RC_INSECT) ) {
		damage += (skill_lv * 4);
		if (sd->sc.data[SC_SOULLINK] && sd->sc.data[SC_SOULLINK]->val2 == SL_HUNTER)
			damage += sd->status.str;
	}

	if(type == 0)
		weapon = sd->weapontype1;
	else
		weapon = sd->weapontype2;
	switch(weapon) {
		case W_1HSWORD:
			#ifdef RENEWAL
				if((skill_lv = pc->checkskill(sd,AM_AXEMASTERY)) > 0)
					damage += (skill_lv * 3);
			#endif
		case W_DAGGER:
			if((skill_lv = pc->checkskill(sd,SM_SWORD)) > 0)
				damage += (skill_lv * 4);
			if((skill_lv = pc->checkskill(sd,GN_TRAINING_SWORD)) > 0)
				damage += skill_lv * 10;
			break;
		case W_2HSWORD:
			#ifdef RENEWAL
				if((skill_lv = pc->checkskill(sd,AM_AXEMASTERY)) > 0)
					damage += (skill_lv * 3);
			#endif
			if((skill_lv = pc->checkskill(sd,SM_TWOHAND)) > 0)
				damage += (skill_lv * 4);
			break;
		case W_1HSPEAR:
		case W_2HSPEAR:
			if ((skill_lv = pc->checkskill(sd,KN_SPEARMASTERY)) > 0) {
				if (pc_isridingdragon(sd))
					damage += (skill_lv * 10);
				else if (pc_isridingpeco(sd))
					damage += (skill_lv * 5);
				else
					damage += (skill_lv * 4);
			}
			break;
		case W_1HAXE:
		case W_2HAXE:
			if((skill_lv = pc->checkskill(sd,AM_AXEMASTERY)) > 0)
				damage += (skill_lv * 3);
			if((skill_lv = pc->checkskill(sd,NC_TRAININGAXE)) > 0)
				damage += (skill_lv * 5);
			break;
		case W_MACE:
		case W_2HMACE:
			if((skill_lv = pc->checkskill(sd,PR_MACEMASTERY)) > 0)
				damage += (skill_lv * 3);
			if((skill_lv = pc->checkskill(sd,NC_TRAININGAXE)) > 0)
				damage += (skill_lv * 5);
			break;
		case W_FIST:
			if((skill_lv = pc->checkskill(sd,TK_RUN)) > 0)
				damage += (skill_lv * 10);
			// No break, fall through to Knuckles
		case W_KNUCKLE:
			if((skill_lv = pc->checkskill(sd,MO_IRONHAND)) > 0)
				damage += (skill_lv * 3);
			break;
		case W_MUSICAL:
			if((skill_lv = pc->checkskill(sd,BA_MUSICALLESSON)) > 0)
				damage += (skill_lv * 3);
			break;
		case W_WHIP:
			if((skill_lv = pc->checkskill(sd,DC_DANCINGLESSON)) > 0)
				damage += (skill_lv * 3);
			break;
		case W_BOOK:
			if((skill_lv = pc->checkskill(sd,SA_ADVANCEDBOOK)) > 0)
				damage += (skill_lv * 3);
			break;
		case W_KATAR:
			if((skill_lv = pc->checkskill(sd,AS_KATAR)) > 0)
				damage += (skill_lv * 3);
			break;
	}

	return damage;
}

/*==========================================
 * Calculates ATK masteries.
 *------------------------------------------*/
int64 battle_calc_masteryfix(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 damage, int div, bool left, bool weapon) {
	int skill2_lv, i;
	struct status_change *sc;
	struct map_session_data *sd;
	struct status_data *tstatus;

	nullpo_ret(src);
	nullpo_ret(target);

	sc = status->get_sc(src);
	sd = BL_CAST(BL_PC, src);
	tstatus = status->get_status_data(target);

	if ( !sd )
		return damage;

	damage = battle->add_mastery(sd, target, damage, left);

	switch( skill_id ){ // specific skill masteries
		case MO_INVESTIGATE:
		case MO_EXTREMITYFIST:
		case CR_GRANDCROSS:
		case NJ_ISSEN:
		case CR_ACIDDEMONSTRATION:
			return damage;
		case NJ_SYURIKEN:
			if( (skill2_lv = pc->checkskill(sd,NJ_TOBIDOUGU)) > 0
#ifndef RENEWAL
				&& weapon
#endif
				)
				damage += 3 * skill2_lv;
			break;
#ifndef RENEWAL
		case NJ_KUNAI:
			if( weapon )
				damage += 60;
			break;
#endif
		case RA_WUGDASH://(Caster Current Weight x 10 / 8)
			if( sd->weight )
				damage += sd->weight / 8;
			/* Fall through */
		case RA_WUGSTRIKE:
		case RA_WUGBITE:
			damage += 30*pc->checkskill(sd, RA_TOOTHOFWUG);
			break;
		case HT_FREEZINGTRAP:
			damage += 40 * pc->checkskill(sd, RA_RESEARCHTRAP);
			break;
		default:
			battle->calc_masteryfix_unknown(src, target, &skill_id, &skill_lv, &damage, &div, &left, &weapon);
			break;
	}

	if( sc ){ // sc considered as masteries
		if(sc->data[SC_GN_CARTBOOST])
			damage += 10 * sc->data[SC_GN_CARTBOOST]->val1;
		if(sc->data[SC_CAMOUFLAGE])
			damage += 30 * ( 10 - sc->data[SC_CAMOUFLAGE]->val4 );
#ifdef RENEWAL
		if(sc->data[SC_NIBELUNGEN] && weapon)
			damage += sc->data[SC_NIBELUNGEN]->val2;
		if(sc->data[SC_IMPOSITIO])
			damage += sc->data[SC_IMPOSITIO]->val2;
		if(sc->data[SC_DRUMBATTLE]){
			if(tstatus->size == SZ_SMALL)
				damage += sc->data[SC_DRUMBATTLE]->val2;
			else if(tstatus->size == SZ_MEDIUM)
				damage += 10 * sc->data[SC_DRUMBATTLE]->val1;
			//else no bonus for large target
		}
		if(sc->data[SC_GS_MADNESSCANCEL])
			damage += 100;
		if(sc->data[SC_GS_GATLINGFEVER]){
			if(tstatus->size == SZ_SMALL)
				damage += 10 * sc->data[SC_GS_GATLINGFEVER]->val1;
			else if(tstatus->size == SZ_MEDIUM)
				damage += -5 * sc->data[SC_GS_GATLINGFEVER]->val1;
			else
				damage += sc->data[SC_GS_GATLINGFEVER]->val1;
		}
#if 0
		if(sc->data[SC_SPECIALZONE])
			damage += sc->data[SC_SPECIALZONE]->val2 >> 4;
#endif // 0
#endif // RENEWAL
	}

	// general skill masteries
#ifdef RENEWAL
	if( div < 0 ) // div fix
		div = 1;
	if( skill_id == MO_FINGEROFFENSIVE )//The finger offensive spheres on moment of attack do count. [Skotlex]
		damage += div * sd->spiritball_old * 3;
	else
		damage += div * sd->spiritball * 3;
	if( skill_id != CR_SHIELDBOOMERANG ) // Only Shield boomerang doesn't takes the Star Crumbs bonus.
		damage += div * (left ? sd->left_weapon.star : sd->right_weapon.star);
	if( skill_id != MC_CARTREVOLUTION && (skill2_lv=pc->checkskill(sd,BS_HILTBINDING)) > 0 )
		damage += 4;

	if(sd->status.party_id && (skill2_lv=pc->checkskill(sd,TK_POWER)) > 0) {
		if( (i = party->foreachsamemap(party->sub_count, sd, 0)) > 1 )
			damage += 2 * skill2_lv * i * (damage /*+ unknown value*/)  / 100 /*+ unknown value*/;
	}
#else
	if( skill_id != ASC_BREAKER && weapon ) // Adv Katar Mastery is does not applies to ASC_BREAKER, but other masteries DO apply >_>
		if( sd->status.weapon == W_KATAR && (skill2_lv=pc->checkskill(sd,ASC_KATAR)) > 0 )
			damage += damage * (10 + 2 * skill2_lv) / 100;
#endif

	// percentage factor masteries
	if ( sc && sc->data[SC_MIRACLE] )
		i = 2; //Star anger
	else
		ARR_FIND(0, MAX_PC_FEELHATE, i, status->get_class(target) == sd->hate_mob[i]);
	if (i < MAX_PC_FEELHATE && (skill2_lv=pc->checkskill(sd,pc->sg_info[i].anger_id)) > 0 && weapon) {
		int ratio = sd->status.base_level + status_get_dex(src) + status_get_luk(src);
		if ( i == 2 ) ratio += status_get_str(src); //Star Anger
		if  (skill2_lv < 4 )
			ratio /= (12 - 3 * skill2_lv);
		damage += damage * ratio / 100;
	}

	if( sd->status.class_ == JOB_ARCH_BISHOP_T || sd->status.class_ == JOB_ARCH_BISHOP ){
		if((skill2_lv = pc->checkskill(sd,AB_EUCHARISTICA)) > 0 &&
			(tstatus->race == RC_DEMON || tstatus->def_ele == ELE_DARK) )
			damage += damage * skill2_lv / 100;
	}

	return damage;
}

void battle_calc_masteryfix_unknown(struct block_list *src, struct block_list *target, uint16 *skill_id, uint16 *skill_lv, int64 *damage, int *div, bool *left, bool *weapon) {
}

/*==========================================
 * Elemental attribute fix.
 *------------------------------------------*/
// FIXME: flag is undocumented
int64 battle_calc_elefix(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 damage, int nk, int n_ele, int s_ele, int s_ele_, bool left, int flag){
	struct status_data *tstatus;

	nullpo_ret(src);
	nullpo_ret(target);

	tstatus = status->get_status_data(target);

	if( (nk&NK_NO_ELEFIX) || n_ele )
		return damage;

	if( damage > 0 ) {
		if( left )
			damage = battle->attr_fix(src, target, damage, s_ele_, tstatus->def_ele, tstatus->ele_lv);
		else{
			damage=battle->attr_fix(src, target, damage, s_ele, tstatus->def_ele, tstatus->ele_lv);
			if( skill_id == MC_CARTREVOLUTION ) //Cart Revolution applies the element fix once more with neutral element
				damage = battle->attr_fix(src,target,damage,ELE_NEUTRAL,tstatus->def_ele, tstatus->ele_lv);
			if( skill_id == NC_ARMSCANNON )
				damage = battle->attr_fix(src,target,damage,ELE_NEUTRAL,tstatus->def_ele, tstatus->ele_lv);
			if( skill_id == GS_GROUNDDRIFT ) //Additional 50*lv Neutral damage.
				damage += battle->attr_fix(src,target,50*skill_lv,ELE_NEUTRAL,tstatus->def_ele, tstatus->ele_lv);
		}
	}

#ifndef RENEWAL
	{
		struct status_data *sstatus;
		struct status_change *sc;

		sstatus = status->get_status_data(src);
		sc = status->get_sc(src);

		if( sc && sc->data[SC_SUB_WEAPONPROPERTY] ) { // Descriptions indicate this means adding a percent of a normal attack in another element. [Skotlex]
			int64 temp = battle->calc_base_damage2(sstatus, &sstatus->rhw, sc, tstatus->size, BL_CAST(BL_PC, src), (flag?2:0)) * sc->data[SC_SUB_WEAPONPROPERTY]->val2 / 100;
			damage += battle->attr_fix(src, target, temp, sc->data[SC_SUB_WEAPONPROPERTY]->val1, tstatus->def_ele, tstatus->ele_lv);
			if( left ) {
				temp = battle->calc_base_damage2(sstatus, &sstatus->lhw, sc, tstatus->size, BL_CAST(BL_PC, src), (flag?2:0)) * sc->data[SC_SUB_WEAPONPROPERTY]->val2 / 100;
				damage += battle->attr_fix(src, target, temp, sc->data[SC_SUB_WEAPONPROPERTY]->val1, tstatus->def_ele, tstatus->ele_lv);
			}
		}
	}
#endif
	return damage;
}
int64 battle_calc_cardfix2(struct block_list *src, struct block_list *bl, int64 damage, int s_ele, int nk, int flag) {
#ifdef RENEWAL
	struct map_session_data *tsd;
	struct status_data *sstatus;

	if ( !damage )
		return 0;

	nullpo_ret(bl);
	nullpo_ret(src);

	tsd = BL_CAST(BL_PC, bl);
	sstatus = status->get_status_data(src);

	if ( tsd ) {
		if ( !(nk&NK_NO_CARDFIX_DEF) ) {
			// RaceAddTolerance
			damage -= damage * tsd->race_tolerance[sstatus->race] / 100;
			damage -= damage * tsd->race_tolerance[is_boss(src) ? RC_BOSS : RC_NONBOSS] / 100;
			if ( flag&BF_SHORT )
				damage -= damage * tsd->bonus.near_attack_def_rate / 100;
			else // SubRangeAttackDamage or bLongAtkDef
				damage -= damage * tsd->bonus.long_attack_def_rate / 100;
		}
		if ( flag&BF_LONG && tsd->sc.data[SC_GS_ADJUSTMENT] ) {
			damage -= 20 * damage / 100;
		}
	}
#endif
	return damage;
}
/*==========================================
 * Calculates card bonuses damage adjustments.
 * cflag(cardfix flag):
 * &1 - calc for left hand.
 * &2 - atker side cardfix(BF_WEAPON) otherwise target side(BF_WEAPON).
 *------------------------------------------*/
// FIXME: wflag is undocumented
int64 battle_calc_cardfix(int attack_type, struct block_list *src, struct block_list *target, int nk, int s_ele, int s_ele_, int64 damage, int cflag, int wflag){
	struct map_session_data *sd, *tsd;
#ifdef RENEWAL
	short cardfix = 100;
#else
	short cardfix = 1000;
#endif
	short t_class, s_class, s_race2, t_race2;
	struct status_data *sstatus, *tstatus;
	int i;

	if( !damage )
		return 0;

	nullpo_ret(src);
	nullpo_ret(target);

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	t_class = status->get_class(target);
	s_class = status->get_class(src);
	sstatus = status->get_status_data(src);
	tstatus = status->get_status_data(target);
	s_race2 = status->get_race2(src);

	switch(attack_type){
		case BF_MAGIC:
			if ( sd && !(nk&NK_NO_CARDFIX_ATK) ) {
				cardfix = cardfix * (100 + sd->magic_addrace[tstatus->race]) / 100;
				if (!(nk&NK_NO_ELEFIX))
					cardfix = cardfix*(100+sd->magic_addele[tstatus->def_ele]) / 100;
				cardfix = cardfix * (100 + sd->magic_addsize[tstatus->size]) / 100;
				cardfix = cardfix * (100 + sd->magic_addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]) / 100;
				cardfix = cardfix * (100 + sd->magic_atk_ele[s_ele])/100;
				for(i=0; i< ARRAYLENGTH(sd->add_mdmg) && sd->add_mdmg[i].rate; i++) {
					if(sd->add_mdmg[i].class_ == t_class) {
						cardfix = cardfix * (100 + sd->add_mdmg[i].rate) / 100;
						break;
					}
				}
			}

			if( tsd && !(nk&NK_NO_CARDFIX_DEF) )
			{ // Target cards.
				if (!(nk&NK_NO_ELEFIX))
				{
					int ele_fix = tsd->subele[s_ele];
					for (i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++)
					{
						if(tsd->subele2[i].ele != s_ele) continue;
						if(!(tsd->subele2[i].flag&wflag&BF_WEAPONMASK &&
							 tsd->subele2[i].flag&wflag&BF_RANGEMASK &&
							 tsd->subele2[i].flag&wflag&BF_SKILLMASK))
							continue;
						ele_fix += tsd->subele2[i].rate;
					}
					cardfix = cardfix * (100 - ele_fix) / 100;
				}
				cardfix = cardfix * (100 - tsd->subsize[sstatus->size]) / 100;
				cardfix = cardfix * (100 - tsd->subrace2[s_race2]) / 100;
				cardfix = cardfix * (100 - tsd->subrace[sstatus->race]) / 100;
				cardfix = cardfix * (100 - tsd->subrace[is_boss(src)?RC_BOSS:RC_NONBOSS]) / 100;

				for(i=0; i < ARRAYLENGTH(tsd->add_mdef) && tsd->add_mdef[i].rate;i++) {
					if(tsd->add_mdef[i].class_ == s_class) {
						cardfix = cardfix * (100-tsd->add_mdef[i].rate) / 100;
						break;
					}
				}
#ifndef RENEWAL
				//It was discovered that ranged defense also counts vs magic! [Skotlex]
				if ( wflag&BF_SHORT )
					cardfix = cardfix * ( 100 - tsd->bonus.near_attack_def_rate ) / 100;
				else
					cardfix = cardfix * ( 100 - tsd->bonus.long_attack_def_rate ) / 100;
#endif

				cardfix = cardfix * ( 100 - tsd->bonus.magic_def_rate ) / 100;

				if( tsd->sc.data[SC_PROTECT_MDEF] )
					cardfix = cardfix * ( 100 - tsd->sc.data[SC_PROTECT_MDEF]->val1 ) / 100;
			}
#ifdef RENEWAL
			if ( cardfix != 100 )
				damage += damage * (cardfix - 100) / 100;
#else
			if ( cardfix != 1000 )
				damage = damage * cardfix / 1000;
#endif
			break;
		case BF_WEAPON:
			t_race2 = status->get_race2(target);
			if( cflag&2 ){
				if( sd && !(nk&NK_NO_CARDFIX_ATK) ){
					short cardfix_ =
#ifdef RENEWAL
						100;
#else
						1000;
#endif
					if( sd->state.arrow_atk ){
						cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->arrow_addrace[tstatus->race]) / 100;
						if( !(nk&NK_NO_ELEFIX) ){
							int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->arrow_addele[tstatus->def_ele];
							for(i = 0; ARRAYLENGTH(sd->right_weapon.addele2) > i && sd->right_weapon.addele2[i].rate != 0; i++){
								if (sd->right_weapon.addele2[i].ele != tstatus->def_ele) continue;
								if(!(sd->right_weapon.addele2[i].flag&wflag&BF_WEAPONMASK &&
									 sd->right_weapon.addele2[i].flag&wflag&BF_RANGEMASK &&
									 sd->right_weapon.addele2[i].flag&wflag&BF_SKILLMASK))
										continue;
								ele_fix += sd->right_weapon.addele2[i].rate;
							}
							cardfix = cardfix * (100 + ele_fix) / 100;
						}
						cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size]+sd->arrow_addsize[tstatus->size]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addrace2[t_race2]) / 100;
						cardfix = cardfix * (100 + sd->right_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS] + sd->arrow_addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]) / 100;
					}else{ // Melee attack
						if( !battle_config.left_cardfix_to_right ){
							cardfix=cardfix*(100+sd->right_weapon.addrace[tstatus->race])/100;
							if( !(nk&NK_NO_ELEFIX) ){
								int ele_fix = sd->right_weapon.addele[tstatus->def_ele];
								for (i = 0; ARRAYLENGTH(sd->right_weapon.addele2) > i && sd->right_weapon.addele2[i].rate != 0; i++) {
									if (sd->right_weapon.addele2[i].ele != tstatus->def_ele) continue;
									if(!(sd->right_weapon.addele2[i].flag&wflag&BF_WEAPONMASK &&
										 sd->right_weapon.addele2[i].flag&wflag&BF_RANGEMASK &&
										 sd->right_weapon.addele2[i].flag&wflag&BF_SKILLMASK))
											continue;
									ele_fix += sd->right_weapon.addele2[i].rate;
								}
								cardfix = cardfix * (100+ele_fix) / 100;
							}
							cardfix = cardfix * (100+sd->right_weapon.addsize[tstatus->size]) / 100;
							cardfix = cardfix * (100+sd->right_weapon.addrace2[t_race2]) / 100;
							cardfix = cardfix * (100+sd->right_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]) / 100;

							if( cflag&1 ){
								cardfix_ = cardfix_*(100+sd->left_weapon.addrace[tstatus->race])/100;
								if (!(nk&NK_NO_ELEFIX)){
									int ele_fix_lh = sd->left_weapon.addele[tstatus->def_ele];
									for (i = 0; ARRAYLENGTH(sd->left_weapon.addele2) > i && sd->left_weapon.addele2[i].rate != 0; i++) {
										if (sd->left_weapon.addele2[i].ele != tstatus->def_ele) continue;
										if(!(sd->left_weapon.addele2[i].flag&wflag&BF_WEAPONMASK &&
											 sd->left_weapon.addele2[i].flag&wflag&BF_RANGEMASK &&
											 sd->left_weapon.addele2[i].flag&wflag&BF_SKILLMASK))
												continue;
										ele_fix_lh += sd->left_weapon.addele2[i].rate;
									}
									cardfix = cardfix * (100+ele_fix_lh) / 100;
								}
								cardfix_ = cardfix_ * (100+sd->left_weapon.addsize[tstatus->size]) / 100;
								cardfix_ = cardfix_ * (100+sd->left_weapon.addrace2[t_race2]) / 100;
								cardfix_ = cardfix_ * (100+sd->left_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]) / 100;
							}
						}else{
							int ele_fix = sd->right_weapon.addele[tstatus->def_ele] + sd->left_weapon.addele[tstatus->def_ele];
							for (i = 0; ARRAYLENGTH(sd->right_weapon.addele2) > i && sd->right_weapon.addele2[i].rate != 0; i++){
								if (sd->right_weapon.addele2[i].ele != tstatus->def_ele) continue;
								if(!(sd->right_weapon.addele2[i].flag&wflag&BF_WEAPONMASK &&
									 sd->right_weapon.addele2[i].flag&wflag&BF_RANGEMASK &&
									 sd->right_weapon.addele2[i].flag&wflag&BF_SKILLMASK))
										continue;
								ele_fix += sd->right_weapon.addele2[i].rate;
							}
							for (i = 0; ARRAYLENGTH(sd->left_weapon.addele2) > i && sd->left_weapon.addele2[i].rate != 0; i++){
								if (sd->left_weapon.addele2[i].ele != tstatus->def_ele) continue;
								if(!(sd->left_weapon.addele2[i].flag&wflag&BF_WEAPONMASK &&
									 sd->left_weapon.addele2[i].flag&wflag&BF_RANGEMASK &&
									 sd->left_weapon.addele2[i].flag&wflag&BF_SKILLMASK))
										continue;
								ele_fix += sd->left_weapon.addele2[i].rate;
							}

							cardfix = cardfix * (100 + sd->right_weapon.addrace[tstatus->race] + sd->left_weapon.addrace[tstatus->race]) / 100;
							cardfix = cardfix * (100 + ele_fix) / 100;
							cardfix = cardfix * (100 + sd->right_weapon.addsize[tstatus->size] + sd->left_weapon.addsize[tstatus->size])/100;
							cardfix = cardfix * (100 + sd->right_weapon.addrace2[t_race2] + sd->left_weapon.addrace2[t_race2])/100;
							cardfix = cardfix * (100 + sd->right_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS] + sd->left_weapon.addrace[is_boss(target)?RC_BOSS:RC_NONBOSS]) / 100;
						}
					}

					for( i = 0; i < ARRAYLENGTH(sd->right_weapon.add_dmg) && sd->right_weapon.add_dmg[i].rate; i++ ){
						if( sd->right_weapon.add_dmg[i].class_ == t_class ){
							cardfix = cardfix * (100 + sd->right_weapon.add_dmg[i].rate) / 100;
							break;
						}
					}

					if( cflag&1 ){
						for( i = 0; i < ARRAYLENGTH(sd->left_weapon.add_dmg) && sd->left_weapon.add_dmg[i].rate; i++ ){
							if( sd->left_weapon.add_dmg[i].class_ == t_class ){
								cardfix_ = cardfix_ * (100 + sd->left_weapon.add_dmg[i].rate) / 100;
								break;
							}
						}
					}
#ifndef RENEWAL
					if( wflag&BF_LONG )
						cardfix = cardfix * (100 + sd->bonus.long_attack_atk_rate) / 100;
					if( (cflag&1) && cardfix_ != 1000 )
						damage = damage * cardfix_ / 1000;
					else if( cardfix != 1000 )
						damage = damage * cardfix / 1000;
#else
					if ((cflag & 1) && cardfix_ != 100)
						damage += damage * (cardfix_ - 100) / 100;
					else if (cardfix != 100)
						damage += damage * (cardfix - 100) / 100;
#endif
				}
			}else{
				// Target side
				if( tsd && !(nk&NK_NO_CARDFIX_DEF) ){
					if( !(nk&NK_NO_ELEFIX) ){
						int ele_fix = tsd->subele[s_ele];
						for (i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++)
						{
							if(tsd->subele2[i].ele != s_ele) continue;
							if(!(tsd->subele2[i].flag&wflag&BF_WEAPONMASK &&
								 tsd->subele2[i].flag&wflag&BF_RANGEMASK &&
								 tsd->subele2[i].flag&wflag&BF_SKILLMASK))
								continue;
							ele_fix += tsd->subele2[i].rate;
						}
						cardfix = cardfix * (100-ele_fix) / 100;
						if( cflag&1 && s_ele_ != s_ele ){
							int ele_fix_lh = tsd->subele[s_ele_];
							for (i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++){
								if(tsd->subele2[i].ele != s_ele_) continue;
								if(!(tsd->subele2[i].flag&wflag&BF_WEAPONMASK &&
									 tsd->subele2[i].flag&wflag&BF_RANGEMASK &&
									 tsd->subele2[i].flag&wflag&BF_SKILLMASK))
									continue;
								ele_fix_lh += tsd->subele2[i].rate;
							}
							cardfix = cardfix * (100 - ele_fix_lh) / 100;
						}
					}
					cardfix = cardfix * (100-tsd->subsize[sstatus->size]) / 100;
					cardfix = cardfix * (100-tsd->subrace2[s_race2]) / 100;
					cardfix = cardfix * (100-tsd->subrace[sstatus->race]) / 100;
					cardfix = cardfix * (100-tsd->subrace[is_boss(src)?RC_BOSS:RC_NONBOSS]) / 100;

					for( i = 0; i < ARRAYLENGTH(tsd->add_def) && tsd->add_def[i].rate;i++ ){
						if( tsd->add_def[i].class_ == s_class )
						{
							cardfix = cardfix * (100 - tsd->add_def[i].rate) / 100;
							break;
						}
					}
#ifndef RENEWAL
					if( wflag&BF_SHORT )
						cardfix = cardfix * (100 - tsd->bonus.near_attack_def_rate) / 100;
					else // BF_LONG (there's no other choice)
						cardfix = cardfix * (100 - tsd->bonus.long_attack_def_rate) / 100;
#endif
					if( tsd->sc.data[SC_PROTECT_DEF] )
						cardfix = cardfix * (100 - tsd->sc.data[SC_PROTECT_DEF]->val1) / 100;
#ifdef RENEWAL
					if ( cardfix != 100 )
						damage += damage * (cardfix - 100) / 100;
#else
					if( cardfix != 1000 )
						damage = damage * cardfix / 1000;
#endif
				}
			}
			break;
		case BF_MISC:
			if ( tsd && !(nk&NK_NO_CARDFIX_DEF) ) {
				// misc damage reduction from equipment
#ifndef RENEWAL
				if ( !(nk&NK_NO_ELEFIX) )
				{
					int ele_fix = tsd->subele[s_ele];
					for (i = 0; ARRAYLENGTH(tsd->subele2) > i && tsd->subele2[i].rate != 0; i++)
					{
						if(tsd->subele2[i].ele != s_ele) continue;
						if(!(tsd->subele2[i].flag&wflag&BF_WEAPONMASK &&
							tsd->subele2[i].flag&wflag&BF_RANGEMASK &&
							tsd->subele2[i].flag&wflag&BF_SKILLMASK))
							continue;
						ele_fix += tsd->subele2[i].rate;
					}
					cardfix = cardfix * (100 - ele_fix) / 100;
				}
				cardfix = cardfix*(100-tsd->subrace[sstatus->race]) / 100;
				cardfix = cardfix*(100-tsd->subrace[is_boss(src)?RC_BOSS:RC_NONBOSS]) / 100;
				if( wflag&BF_SHORT )
					cardfix = cardfix * ( 100 - tsd->bonus.near_attack_def_rate ) / 100;
				else // BF_LONG (there's no other choice)
					cardfix = cardfix * ( 100 - tsd->bonus.long_attack_def_rate ) / 100;
#endif
				cardfix = cardfix*(100 - tsd->subsize[sstatus->size]) / 100;
				cardfix = cardfix*(100 - tsd->subrace2[s_race2]) / 100;
				cardfix = cardfix * (100 - tsd->bonus.misc_def_rate) / 100;
#ifdef RENEWAL
				if ( cardfix != 100 )
					damage += damage * (cardfix - 100) / 100;
#else
				if ( cardfix != 1000 )
					damage = damage * cardfix / 1000;
#endif
			}
			break;
	}

	return damage;
}

/*==========================================
 * Calculates defense reduction. [malufett]
 * flag:
 * &1 - idef/imdef(Ignore defense)
 * &2 - pdef(Pierce defense)
 * &4 - tdef(Total defense reduction)
 *------------------------------------------*/
// TODO: Add an enum for flag
int64 battle_calc_defense(int attack_type, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 damage, int flag, int pdef){
	struct status_data *sstatus, *tstatus;
	struct map_session_data *sd, *tsd;
	struct status_change *sc, *tsc;
	int i;

	if( !damage )
		return 0;

	nullpo_ret(src);
	nullpo_ret(target);

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	sstatus = status->get_status_data(src);
	tstatus = status->get_status_data(target);
	sc = status->get_sc(src);
	tsc = status->get_sc(target);

	switch(attack_type){
		case BF_WEAPON:
			{
			/* Take note in RE
			 *  def1 = equip def
			 *  def2 = status def
			 */
			defType def1 = status->get_def(target); //Don't use tstatus->def1 due to skill timer reductions.
			short def2 = tstatus->def2, vit_def;
#ifdef RENEWAL
			def1 = status->calc_def2(target, tsc, def1, false); // equip def(RE)
			def2 = status->calc_def(target, tsc, def2, false); // status def(RE)
#else
			def1 = status->calc_def(target, tsc, def1, false); // equip def(RE)
			def2 = status->calc_def2(target, tsc, def2, false); // status def(RE)
#endif

			if ( sd ) {
				if ( sd->charm_type == CHARM_TYPE_LAND && sd->charm_count > 0 ) // hidden from status window
					def1 += 10 * def1 * sd->charm_count / 100;

				i = sd->ignore_def[is_boss(target) ? RC_BOSS : RC_NONBOSS];
				i += sd->ignore_def[tstatus->race];
				if ( i ) {
					if ( i > 100 ) i = 100;
					def1 -= def1 * i / 100;
#ifndef RENEWAL
					def2 -= def2 * i / 100;
#endif
				}
			}

			if( sc && sc->data[SC_EXPIATIO] ){
				i = 5 * sc->data[SC_EXPIATIO]->val1; // 5% per level
				def1 -= def1 * i / 100;
#ifndef RENEWAL
				def2 -= def2 * i / 100;
#endif
			}

			if( battle_config.vit_penalty_type && battle_config.vit_penalty_target&target->type ) {
				unsigned char target_count; //256 max targets should be a sane max
				target_count = unit->counttargeted(target);
				if(target_count >= battle_config.vit_penalty_count) {
					if(battle_config.vit_penalty_type == 1) {
						if( !tsc || !tsc->data[SC_STEELBODY] )
							def1 = (def1 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
						def2 = (def2 * (100 - (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num))/100;
					} else { //Assume type 2
						if( !tsc || !tsc->data[SC_STEELBODY] )
							def1 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
						def2 -= (target_count - (battle_config.vit_penalty_count - 1))*battle_config.vit_penalty_num;
					}
				}
#ifndef RENEWAL
				if(skill_id == AM_ACIDTERROR) def1 = 0; //Acid Terror ignores only armor defense. [Skotlex]
#endif
				if(def2 < 1) def2 = 1;
			}
			//Vitality reduction from rodatazone: http://rodatazone.simgaming.net/mechanics/substats.php#def
			if (tsd) {
				//Sd vit-eq
#ifndef RENEWAL
				//[VIT*0.5] + rnd([VIT*0.3], max([VIT*0.3],[VIT^2/150]-1))
				vit_def = def2*(def2-15)/150;
				vit_def = def2/2 + (vit_def>0?rnd()%vit_def:0);
#else
				vit_def = def2;
#endif
				if((battle->check_undead(sstatus->race,sstatus->def_ele) || sstatus->race==RC_DEMON) && //This bonus already doesn't work vs players
					src->type == BL_MOB && (i=pc->checkskill(tsd,AL_DP)) > 0)
					vit_def += i*(int)(3 +(tsd->status.base_level+1)*0.04);   // [orn]
				if( src->type == BL_MOB && (i=pc->checkskill(tsd,RA_RANGERMAIN))>0 &&
					(sstatus->race == RC_BRUTE || sstatus->race == RC_FISH || sstatus->race == RC_PLANT) )
					vit_def += i*5;
			}
			else { //Mob-Pet vit-eq
#ifndef RENEWAL
				//VIT + rnd(0,[VIT/20]^2-1)
				vit_def = (def2/20)*(def2/20);
				vit_def = def2 + (vit_def>0?rnd()%vit_def:0);
#else
				vit_def = def2;
#endif
			}

			if (battle_config.weapon_defense_type) {
				vit_def += def1*battle_config.weapon_defense_type;
				def1 = 0;
			}
		#ifdef RENEWAL
			/**
			* RE DEF Reduction
			* Pierce defense gains 1 atk per def/2
			**/

			if( def1 < -399 ) // it stops at -399
				def1 = 399; // in aegis it set to 1 but in our case it may lead to exploitation so limit it to 399
				//return 1;

			if( flag&2 )
				damage += def1 >> 1;

			if( !(flag&1) && !(flag&2) ) {
				if( flag&4 )
					damage -= (def1 + vit_def);
				else
					damage = (int)((100.0f - def1 / (def1 + 400.0f) * 90.0f) / 100.0f * damage - vit_def);
			}
		#else
				if( def1 > 100 ) def1 = 100;
				if( !(flag&1) ){
					if( flag&2 )
						damage = damage * pdef * (def1+vit_def) / 100;
					else
						damage = damage * (100-def1) / 100;
				}
				if( !(flag&1 || flag&2) )
					damage -= vit_def;
		#endif
			}
			break;

		case BF_MAGIC:
		{
			defType mdef = tstatus->mdef;
			short mdef2= tstatus->mdef2;
#ifdef RENEWAL
			mdef2 = status->calc_mdef(target, tsc, mdef2, false); // status mdef(RE)
			mdef = status->calc_mdef2(target, tsc, mdef, false); // equip mde(RE)
#else
			mdef2 = status->calc_mdef2(target, tsc, mdef2, false); // status mdef(RE)
			mdef = status->calc_mdef(target, tsc, mdef, false); // equip mde(RE)
#endif
			if( flag&1 )
				mdef = 0;

			if(sd) {
				i = sd->ignore_mdef[is_boss(target)?RC_BOSS:RC_NONBOSS];
				i += sd->ignore_mdef[tstatus->race];
				if (i)
				{
					if (i > 100) i = 100;
					mdef -= mdef * i/100;
					//mdef2-= mdef2* i/100;
				}
			}
		#ifdef RENEWAL
			/**
			 * RE MDEF Reduction
			 **/
			if( mdef < -99 ) // it stops at -99
				mdef = 99; // in aegis it set to 1 but in our case it may lead to exploitation so limit it to 99
				//return 1;

			damage = (int)((100.0f - mdef / (mdef + 100.0f) * 90.0f) / 100.0f * damage - mdef2);
		#else
			if(battle_config.magic_defense_type)
				damage = damage - mdef*battle_config.magic_defense_type - mdef2;
			else
				damage = damage * (100-mdef)/100 - mdef2;
		#endif
		}
			break;
	}
	return damage;
}

// Minstrel/Wanderer number check for chorus skills.
int battle_calc_chorusbonus(struct map_session_data *sd) {
	int members = 0;

	if (!sd || !sd->status.party_id)
		return 0;

	members = party->foreachsamemap(party->sub_count_chorus, sd, 0);

	if (members < 3)
		return 0; // Bonus remains 0 unless 3 or more Minstrel's/Wanderer's are in the party.
	if (members > 7)
		return 5; // Maximum effect possible from 7 or more Minstrel's/Wanderer's
	return members - 2; // Effect bonus from additional Minstrel's/Wanderer's if not above the max possible
}

// FIXME: flag is undocumented
int battle_calc_skillratio(int attack_type, struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int skillratio, int flag){
	int i;
	struct status_change *sc, *tsc;
	struct map_session_data *sd, *tsd;
	struct status_data *st, *tst, *bst;

	nullpo_ret(src);
	nullpo_ret(target);

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);
	sc = status->get_sc(src);
	tsc = status->get_sc(target);
	st = status->get_status_data(src);
	bst = status->get_base_status(src);
	tst = status->get_status_data(target);

	switch(attack_type){
		case BF_MAGIC:
			switch(skill_id){
				case MG_NAPALMBEAT:
					skillratio += skill_lv * 10 - 30;
					break;
				case MG_FIREBALL:
			#ifdef RENEWAL
					skillratio += 20 * skill_lv;
			#else
					skillratio += skill_lv * 10 - 30;
			#endif
					break;
				case MG_SOULSTRIKE:
					if (battle->check_undead(tst->race,tst->def_ele))
						skillratio += 5*skill_lv;
					break;
				case MG_FIREWALL:
					skillratio -= 50;
					break;
				case MG_THUNDERSTORM:
					/**
					 * in Renewal Thunder Storm boost is 100% (in pre-re, 80%)
					 **/
					#ifndef RENEWAL
						skillratio -= 20;
					#endif
					break;
				case MG_FROSTDIVER:
					skillratio += 10 * skill_lv;
					break;
				case AL_HOLYLIGHT:
					skillratio += 25;
					if (sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_PRIEST)
						skillratio *= 5; //Does 5x damage include bonuses from other skills?
					break;
				case AL_RUWACH:
					skillratio += 45;
					break;
				case WZ_FROSTNOVA:
					skillratio += (100+skill_lv*10) * 2 / 3 - 100;
					break;
				case WZ_FIREPILLAR:
					if (skill_lv > 10)
						skillratio += 2300; //200% MATK each hit
					else
						skillratio += -60 + 20*skill_lv; //20% MATK each hit
					break;
				case WZ_SIGHTRASHER:
					skillratio += 20 * skill_lv;
					break;
				case WZ_WATERBALL:
					skillratio += 30 * skill_lv;
					break;
				case WZ_STORMGUST:
					skillratio += 40 * skill_lv;
					break;
				case HW_NAPALMVULCAN:
					skillratio += 10 * skill_lv - 30;
					break;
				case SL_STIN:
					skillratio += (tst->size!=SZ_SMALL?-99:10*skill_lv); //target size must be small (0) for full damage.
					break;
				case SL_STUN:
					skillratio += (tst->size!=SZ_BIG?5*skill_lv:-99); //Full damage is dealt on small/medium targets
					break;
				case SL_SMA:
					skillratio += -60 + status->get_lv(src); //Base damage is 40% + lv%
					break;
				case NJ_KOUENKA:
					skillratio -= 10;
					if (sd && sd->charm_type == CHARM_TYPE_FIRE && sd->charm_count > 0)
						skillratio += 20 * sd->charm_count;
					break;
				case NJ_KAENSIN:
					skillratio -= 50;
					if (sd && sd->charm_type == CHARM_TYPE_FIRE && sd->charm_count > 0)
						skillratio += 10 * sd->charm_count;
					break;
				case NJ_BAKUENRYU:
					skillratio += 50 * (skill_lv - 1);
					if (sd && sd->charm_type == CHARM_TYPE_FIRE && sd->charm_count > 0)
						skillratio += 15 * sd->charm_count;
					break;
#ifdef RENEWAL
				case NJ_HYOUSENSOU:
					skillratio -= 30;
					if (sd && sd->charm_type == CHARM_TYPE_WATER && sd->charm_count > 0)
						skillratio += 5 * sd->charm_count;
					break;
#endif
				case NJ_HYOUSYOURAKU:
					skillratio += 50 * skill_lv;
					if (sd && sd->charm_type == CHARM_TYPE_WATER && sd->charm_count > 0)
						skillratio += 25 * sd->charm_count;
					break;
				case NJ_RAIGEKISAI:
					skillratio += 60 + 40 * skill_lv;
					if (sd && sd->charm_type == CHARM_TYPE_WIND && sd->charm_count > 0)
						skillratio += 15 * sd->charm_count;
					break;
				case NJ_KAMAITACHI:
					if (sd && sd->charm_type == CHARM_TYPE_WIND && sd->charm_count > 0)
						skillratio += 10 * sd->charm_count;
					/* Fall through */
				case NPC_ENERGYDRAIN:
					skillratio += 100 * skill_lv;
					break;
			#ifdef RENEWAL
				case WZ_HEAVENDRIVE:
				case WZ_METEOR:
					skillratio += 25;
					break;
				case WZ_VERMILION:
				{
					int interval = 0, per = interval, ratio = per;
					while( (per++) < skill_lv ){
						ratio += interval;
						if(per%3==0) interval += 20;
					}
					if( skill_lv > 9 )
						ratio -= 10;
					skillratio += ratio;
				}
					break;
				case NJ_HUUJIN:
					skillratio += 50;
					if (sd && sd->charm_type == CHARM_TYPE_WIND && sd->charm_count > 0)
						skillratio += 20 * sd->charm_count;
					break;
			#else
				case WZ_VERMILION:
					skillratio += 20*skill_lv-20;
					break;
			#endif
				/**
					* Arch Bishop
					**/
				case AB_JUDEX:
					skillratio = 300 + 20 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case AB_ADORAMUS:
					skillratio = 500 + 100 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case AB_DUPLELIGHT_MAGIC:
					skillratio = 200 + 20 * skill_lv;
					break;
				/**
					* Warlock
					**/
				case WL_SOULEXPANSION: // MATK [{( Skill Level + 4 ) x 100 ) + ( Caster's INT )} x ( Caster's Base Level / 100 )] %
					skillratio = 100 * (skill_lv + 4) + st->int_;
					RE_LVL_DMOD(100);
					break;
				case WL_FROSTMISTY: // MATK [{( Skill Level x 100 ) + 200 } x ( Caster's Base Level / 100 )] %
					skillratio += 100 + 100 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case WL_JACKFROST:
					if( tsc && tsc->data[SC_FROSTMISTY] ){
						skillratio += 900 + 300 * skill_lv;
						RE_LVL_DMOD(100);
					}else{
						skillratio += 400 + 100 * skill_lv;
						RE_LVL_DMOD(150);
					}
					break;
				case WL_DRAINLIFE:
					skillratio = 200 * skill_lv + status_get_int(src);
					RE_LVL_DMOD(100);
					break;
				case WL_CRIMSONROCK:
					skillratio = 300 * skill_lv;
					RE_LVL_DMOD(100);
					skillratio += 1300;
					break;
				case WL_HELLINFERNO:
					skillratio = 300 * skill_lv;
					RE_LVL_DMOD(100);
					// Shadow: MATK [{( Skill Level x 300 ) x ( Caster Base Level / 100 ) x 4/5 }] %
					// Fire : MATK [{( Skill Level x 300 ) x ( Caster Base Level / 100 ) /5 }] %
					if( flag&ELE_DARK )
						skillratio *= 4;
					skillratio /= 5;
					break;
				case WL_COMET:
					i = ( sc ? distance_xy(target->x, target->y, sc->comet_x, sc->comet_y) : 8 );
					if( i <= 3 ) skillratio += 2400 + 500 * skill_lv; // 7 x 7 cell
					else
					if( i <= 5 ) skillratio += 1900 + 500 * skill_lv; // 11 x 11 cell
					else
					if( i <= 7 ) skillratio += 1400 + 500 * skill_lv; // 15 x 15 cell
					else
						skillratio += 900 + 500 * skill_lv; // 19 x 19 cell

					if( sd && sd->status.party_id ){
						struct map_session_data* psd;
						int p_sd[5] = {0, 0, 0, 0, 0}, c; // just limit it to 5

						c = 0;
						memset (p_sd, 0, sizeof(p_sd));
						party->foreachsamemap(skill->check_condition_char_sub, sd, 3, &sd->bl, &c, &p_sd, skill_id);
						c = ( c > 1 ? rnd()%c : 0 );

						if( (psd = map->id2sd(p_sd[c])) && pc->checkskill(psd,WL_COMET) > 0 ){
							skillratio = skill_lv * 400; //MATK [{( Skill Level x 400 ) x ( Caster's Base Level / 120 )} + 2500 ] %
							RE_LVL_DMOD(120);
							skillratio += 2500;
							status_zap(&psd->bl, 0, skill->get_sp(skill_id, skill_lv) / 2);
						}
					}
					break;
				case WL_CHAINLIGHTNING_ATK:
					skillratio += 400 + 100 * skill_lv;
					RE_LVL_DMOD(100);
					if(flag > 0)
						skillratio += 100 * flag;
					break;
				case WL_EARTHSTRAIN:
					skillratio = 2000 + 100 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
					skillratio += 400 + 500 * skill_lv;
					break;
				case WL_SUMMON_ATK_FIRE:
				case WL_SUMMON_ATK_WATER:
				case WL_SUMMON_ATK_WIND:
				case WL_SUMMON_ATK_GROUND:
					skillratio = (1 + skill_lv) / 2 *  (status->get_lv(src) + (sd ? sd->status.job_level : 50));
					RE_LVL_DMOD(100);
					break;
				case LG_RAYOFGENESIS:
				{
					uint16 lv = skill_lv;
					int bandingBonus = 0;
					if( sc && sc->data[SC_BANDING] )
						bandingBonus = 200 * (sd ? skill->check_pc_partner(sd,skill_id,&lv,skill->get_splash(skill_id,skill_lv),0) : 1);
					skillratio = ((300 * skill_lv) + bandingBonus) * (sd ? sd->status.job_level : 1) / 25;
				}
					break;
				case LG_SHIELDSPELL:
					if ( sd && skill_lv == 2 ) // [(Casters Base Level x 4) + (Shield MDEF x 100) + (Casters INT x 2)] %
						skillratio = 4 * status->get_lv(src) + 100 * sd->bonus.shieldmdef + 2 * st->int_;
					else
						skillratio = 0;
					break;
				case WM_METALICSOUND:
					skillratio = 120 * skill_lv + 60 * ( sd? pc->checkskill(sd, WM_LESSON) : 10 );
					RE_LVL_DMOD(100);
					break;
				case WM_REVERBERATION_MAGIC:
					skillratio = 100 * skill_lv + 100;
					RE_LVL_DMOD(100);
					break;
				case SO_FIREWALK:
					skillratio = 60 * skill_lv;
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_HEATER_OPTION] )
						skillratio += sc->data[SC_HEATER_OPTION]->val3 / 2;
					break;
				case SO_ELECTRICWALK:
					skillratio = 60 * skill_lv;
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_BLAST_OPTION] )
						skillratio += sc->data[SC_BLAST_OPTION]->val2 / 2;
					break;
				case SO_EARTHGRAVE:
					skillratio = st->int_ * skill_lv + 200 * (sd ? pc->checkskill(sd,SA_SEISMICWEAPON) : 1);
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_CURSED_SOIL_OPTION] )
						skillratio += sc->data[SC_CURSED_SOIL_OPTION]->val3 * 5;
					break;
				case SO_DIAMONDDUST:
					skillratio = (st->int_ * skill_lv + 200 * (sd ? pc->checkskill(sd, SA_FROSTWEAPON) : 1)) * status->get_lv(src) / 100;
					if( sc && sc->data[SC_COOLER_OPTION] )
						skillratio += sc->data[SC_COOLER_OPTION]->val3 * 5;
					break;
				case SO_POISON_BUSTER:
					skillratio += 900 + 300 * skill_lv;
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_CURSED_SOIL_OPTION] )
						skillratio += sc->data[SC_CURSED_SOIL_OPTION]->val3 * 5;
					break;
				case SO_PSYCHIC_WAVE:
					skillratio = 70 * skill_lv + 3 * st->int_;
					RE_LVL_DMOD(100);
					if( sc && ( sc->data[SC_HEATER_OPTION] || sc->data[SC_COOLER_OPTION]
					         || sc->data[SC_BLAST_OPTION] || sc->data[SC_CURSED_SOIL_OPTION] ) )
						skillratio += skillratio * 20 / 100;
					break;
				case SO_VARETYR_SPEAR:
					skillratio = status_get_int(src) * skill_lv + ( sd ? pc->checkskill(sd, SA_LIGHTNINGLOADER) * 50 : 0 );
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_BLAST_OPTION] )
						skillratio += sc->data[SC_BLAST_OPTION]->val2 * 5;
					break;
				case SO_CLOUD_KILL:
					skillratio = 40 * skill_lv;
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_CURSED_SOIL_OPTION] )
						skillratio += sc->data[SC_CURSED_SOIL_OPTION]->val3;
					break;
				case GN_DEMONIC_FIRE: {
						int fire_expansion_lv = skill_lv / 100;
						skill_lv = skill_lv % 100;
						skillratio = 110 + 20 * skill_lv;
						if ( fire_expansion_lv == 1 )
							skillratio += status_get_int(src) + (sd?sd->status.job_level:50);
						else if ( fire_expansion_lv == 2 )
							skillratio += status_get_int(src) * 10;
					}
					break;
				// Magical Elemental Spirits Attack Skills
				case EL_FIRE_MANTLE:
				case EL_WATER_SCREW:
					skillratio += 900;
					break;
				case EL_FIRE_ARROW:
				case EL_ROCK_CRUSHER_ATK:
					skillratio += 200;
					break;
				case EL_FIRE_BOMB:
				case EL_ICE_NEEDLE:
				case EL_HURRICANE_ATK:
					skillratio += 400;
					break;
				case EL_FIRE_WAVE:
				case EL_TYPOON_MIS_ATK:
					skillratio += 1100;
					break;
				case MH_ERASER_CUTTER:
					skillratio += 400 + 100 * skill_lv + (skill_lv%2 > 0 ? 0 : 300);
					break;
				case MH_XENO_SLASHER:
					if(skill_lv%2) skillratio += 350 + 50 * skill_lv; //500:600:700
					else skillratio += 400 + 100 * skill_lv; //700:900
					break;
				case MH_HEILIGE_STANGE:
					skillratio += 400 + 250 * skill_lv;
					break;
				case MH_POISON_MIST:
					skillratio += 100 * skill_lv;
					break;
				case KO_KAIHOU:
					if (sd && sd->charm_type != CHARM_TYPE_NONE && sd->charm_count > 0) {
						skillratio += -100 + 200 * sd->charm_count;
						RE_LVL_DMOD(100);
						pc->del_charm(sd, sd->charm_count, sd->charm_type);
					}
					break;
				default:
					battle->calc_skillratio_magic_unknown(&attack_type, src, target, &skill_id, &skill_lv, &skillratio, &flag);
					break;
			}
			break;
		case BF_WEAPON:
			switch( skill_id )
			{
				case SM_BASH:
				case MS_BASH:
					skillratio += 30 * skill_lv;
					break;
				case SM_MAGNUM:
				case MS_MAGNUM:
					skillratio += 20 * skill_lv;
					break;
				case MC_MAMMONITE:
					skillratio += 50 * skill_lv;
					break;
				case HT_POWER:
					skillratio += -50 + 8 * status_get_str(src);
					break;
				case AC_DOUBLE:
				case MA_DOUBLE:
					skillratio += 10 * (skill_lv-1);
					break;
				case AC_SHOWER:
				case MA_SHOWER:
					#ifdef RENEWAL
						skillratio += 50 + 10 * skill_lv;
					#else
						skillratio += -25 + 5 * skill_lv;
					#endif
					break;
				case AC_CHARGEARROW:
				case MA_CHARGEARROW:
					skillratio += 50;
					break;
	#ifndef RENEWAL
				case HT_FREEZINGTRAP:
				case MA_FREEZINGTRAP:
					skillratio += -50 + 10 * skill_lv;
					break;
	#endif
				case KN_PIERCE:
				case ML_PIERCE:
					skillratio += 10 * skill_lv;
					break;
				case MER_CRASH:
					skillratio += 10 * skill_lv;
					break;
				case KN_SPEARSTAB:
					skillratio += 20 * skill_lv;
					break;
				case KN_SPEARBOOMERANG:
					skillratio += 50*skill_lv;
					break;
				case KN_BRANDISHSPEAR:
				case ML_BRANDISH:
				{
					int ratio = 100 + 20 * skill_lv;
					skillratio += ratio - 100;
					if(skill_lv>3 && flag==1) skillratio += ratio / 2;
					if(skill_lv>6 && flag==1) skillratio += ratio / 4;
					if(skill_lv>9 && flag==1) skillratio += ratio / 8;
					if(skill_lv>6 && flag==2) skillratio += ratio / 2;
					if(skill_lv>9 && flag==2) skillratio += ratio / 4;
					if(skill_lv>9 && flag==3) skillratio += ratio / 2;
					break;
				}
				case KN_BOWLINGBASH:
				case MS_BOWLINGBASH:
					skillratio+= 40 * skill_lv;
					break;
				case AS_GRIMTOOTH:
					skillratio += 20 * skill_lv;
					break;
				case AS_POISONREACT:
					skillratio += 30 * skill_lv;
					break;
				case AS_SONICBLOW:
					skillratio += 300 + 40 * skill_lv;
					break;
				case TF_SPRINKLESAND:
					skillratio += 30;
					break;
				case MC_CARTREVOLUTION:
					skillratio += 50;
					if( sd && sd->cart_weight )
						skillratio += 100 * sd->cart_weight / sd->cart_weight_max; // +1% every 1% weight
					else if (!sd)
						skillratio += 100; //Max damage for non players.
					break;
				case NPC_RANDOMATTACK:
					skillratio += 100 * skill_lv;
					break;
				case NPC_WATERATTACK:
				case NPC_GROUNDATTACK:
				case NPC_FIREATTACK:
				case NPC_WINDATTACK:
				case NPC_POISONATTACK:
				case NPC_HOLYATTACK:
				case NPC_DARKNESSATTACK:
				case NPC_UNDEADATTACK:
				case NPC_TELEKINESISATTACK:
				case NPC_BLOODDRAIN:
				case NPC_ACIDBREATH:
				case NPC_DARKNESSBREATH:
				case NPC_FIREBREATH:
				case NPC_ICEBREATH:
				case NPC_THUNDERBREATH:
				case NPC_HELLJUDGEMENT:
				case NPC_PULSESTRIKE:
					skillratio += 100 * (skill_lv-1);
					break;
				case NPC_EARTHQUAKE:
					skillratio += 100 + 100 * skill_lv + 100 * (skill_lv / 2);
					break;
				case RG_BACKSTAP:
					if( sd && sd->status.weapon == W_BOW && battle_config.backstab_bow_penalty )
						skillratio += (200 + 40 * skill_lv) / 2;
					else
						skillratio += 200 + 40 * skill_lv;
					break;
				case RG_RAID:
					skillratio += 40 * skill_lv;
					break;
				case RG_INTIMIDATE:
					skillratio += 30 * skill_lv;
					break;
				case CR_SHIELDCHARGE:
					skillratio += 20 * skill_lv;
					break;
				case CR_SHIELDBOOMERANG:
					skillratio += 30 * skill_lv;
					break;
				case NPC_DARKCROSS:
				case CR_HOLYCROSS:
				{
					int ratio = 35 * skill_lv;
					#ifdef RENEWAL
						if(sd && sd->status.weapon == W_2HSPEAR)
							ratio *= 2;
					#endif
					skillratio += ratio;
					break;
				}
				case AM_DEMONSTRATION:
					skillratio += 20 * skill_lv;
					break;
				case AM_ACIDTERROR:
#ifdef RENEWAL
					skillratio += 80 * skill_lv + 100;
#else
					skillratio += 40 * skill_lv;
#endif
					break;
				case MO_FINGEROFFENSIVE:
					skillratio+= 50 * skill_lv;
					break;
				case MO_INVESTIGATE:
					skillratio += 75 * skill_lv;
					break;
				case MO_EXTREMITYFIST:
	#ifndef RENEWAL
					{
						//Overflow check. [Skotlex]
						unsigned int ratio = skillratio + 100*(8 + st->sp/10);
						//You'd need something like 6K SP to reach this max, so should be fine for most purposes.
						if (ratio > 60000) ratio = 60000; //We leave some room here in case skillratio gets further increased.
						skillratio = (unsigned short)ratio;
					}
#endif
					break;
				case MO_TRIPLEATTACK:
					skillratio += 20 * skill_lv;
					break;
				case MO_CHAINCOMBO:
					skillratio += 50 + 50 * skill_lv;
					break;
				case MO_COMBOFINISH:
					skillratio += 140 + 60 * skill_lv;
					break;
				case BA_MUSICALSTRIKE:
				case DC_THROWARROW:
					skillratio += 25 + 25 * skill_lv;
					break;
				case CH_TIGERFIST:
					skillratio += 100 * skill_lv - 60;
					break;
				case CH_CHAINCRUSH:
					skillratio += 300 + 100 * skill_lv;
					break;
				case CH_PALMSTRIKE:
					skillratio += 100 + 100 * skill_lv;
					break;
				case LK_HEADCRUSH:
					skillratio += 40 * skill_lv;
					break;
				case LK_JOINTBEAT:
					i = 10 * skill_lv - 50;
					// Although not clear, it's being assumed that the 2x damage is only for the break neck ailment.
					if (flag&BREAK_NECK) i*=2;
					skillratio += i;
					break;
				case ASC_METEORASSAULT:
					skillratio += 40 * skill_lv - 60;
					break;
				case SN_SHARPSHOOTING:
				case MA_SHARPSHOOTING:
					skillratio += 100 + 50 * skill_lv;
					break;
				case CG_ARROWVULCAN:
					skillratio += 100 + 100 * skill_lv;
					break;
				case AS_SPLASHER:
					skillratio += 400 + 50 * skill_lv;
					if(sd)
						skillratio += 20 * pc->checkskill(sd,AS_POISONREACT);
					break;
	#ifndef RENEWAL
				case ASC_BREAKER:
					skillratio += 100*skill_lv-100;
	#else
				case LK_SPIRALPIERCE:
				case ML_SPIRALPIERCE:
					skillratio += 50 * skill_lv;
	#endif
					break;
				case PA_SACRIFICE:
					skillratio += 10 * skill_lv - 10;
					break;
				case PA_SHIELDCHAIN:
					skillratio += 30 * skill_lv;
					break;
				case WS_CARTTERMINATION:
					i = 10 * (16 - skill_lv);
					if (i < 1) i = 1;
					//Preserve damage ratio when max cart weight is changed.
					if(sd && sd->cart_weight)
						skillratio += sd->cart_weight/i * 80000/battle_config.max_cart_weight - 100;
					else if (!sd)
						skillratio += 80000 / i - 100;
					break;
				case TK_DOWNKICK:
					skillratio += 60 + 20 * skill_lv;
					break;
				case TK_STORMKICK:
					skillratio += 60 + 20 * skill_lv;
					break;
				case TK_TURNKICK:
					skillratio += 90 + 30 * skill_lv;
					break;
				case TK_COUNTER:
					skillratio += 90 + 30 * skill_lv;
					break;
				case TK_JUMPKICK:
					skillratio += -70 + 10*skill_lv;
					if (sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == skill_id)
						skillratio += 10 * status->get_lv(src) / 3; //Tumble bonus
					if (flag) {
						skillratio += 10 * status->get_lv(src) / 3; //Running bonus (TODO: What is the real bonus?)
						if( sc && sc->data[SC_STRUP] )  // Spurt bonus
							skillratio *= 2;
					}
					break;
				case GS_TRIPLEACTION:
					skillratio += 50 * skill_lv;
					break;
				case GS_BULLSEYE:
					//Only works well against brute/demi-humans non bosses.
					if((tst->race == RC_BRUTE || tst->race == RC_DEMIHUMAN)
						&& !(tst->mode&MD_BOSS))
						skillratio += 400;
					break;
				case GS_TRACKING:
					skillratio += 100 * (skill_lv+1);
					break;
#ifndef RENEWAL
				case GS_PIERCINGSHOT:
						skillratio += 20 * skill_lv;
					break;
#endif
				case GS_RAPIDSHOWER:
					skillratio += 10 * skill_lv;
					break;
				case GS_DESPERADO:
					skillratio += 50 * (skill_lv-1);
					break;
				case GS_DUST:
					skillratio += 50 * skill_lv;
					break;
				case GS_FULLBUSTER:
					skillratio += 100 * (skill_lv+2);
					break;
				case GS_SPREADATTACK:
				#ifdef RENEWAL
					skillratio += 20 * (skill_lv);
				#else
					skillratio += 20 * (skill_lv-1);
				#endif
					break;
				case NJ_HUUMA:
					skillratio += 50 + 150 * skill_lv;
					break;
				case NJ_TATAMIGAESHI:
					skillratio += 10 * skill_lv;
					break;
				case NJ_KASUMIKIRI:
					skillratio += 10 * skill_lv;
					break;
				case NJ_KIRIKAGE:
					skillratio += 100 * (skill_lv-1);
					break;
				case KN_CHARGEATK:
					{
						int k = (flag-1)/3; //+100% every 3 cells of distance
						if( k > 2 ) k = 2; // ...but hard-limited to 300%.
						skillratio += 100 * k;
					}
					break;
				case HT_PHANTASMIC:
					skillratio += 50;
					break;
				case MO_BALKYOUNG:
					skillratio += 200;
					break;
				case HFLI_MOON: //[orn]
					skillratio += 10 + 110 * skill_lv;
					break;
				case HFLI_SBR44: //[orn]
					skillratio += 100 * (skill_lv-1);
					break;
				case NPC_VAMPIRE_GIFT:
					skillratio += ((skill_lv-1)%5+1) * 100;
					break;
				case RK_SONICWAVE:
					skillratio = (skill_lv + 5) * 100;
					skillratio = skillratio * (100 + (status->get_lv(src)-100) / 2) / 100;
					break;
				case RK_HUNDREDSPEAR:
						skillratio += 500 + (80 * skill_lv);
						if( sd ){
							short index = sd->equip_index[EQI_HAND_R];
							if( index >= 0 && sd->inventory_data[index]
								&& sd->inventory_data[index]->type == IT_WEAPON )
								skillratio += (10000 - min(10000, sd->inventory_data[index]->weight)) / 10;
							skillratio = skillratio * (100 + (status->get_lv(src)-100) / 2) / 100 + 50 * pc->checkskill(sd,LK_SPIRALPIERCE);
						}
					break;
				case RK_WINDCUTTER:
						skillratio = (skill_lv + 2) * 50;
						RE_LVL_DMOD(100);
					break;
				case RK_IGNITIONBREAK:
					i = distance_bl(src,target);
					if( i < 2 )
						skillratio = 300 * skill_lv;
					else if( i < 4 )
						skillratio = 250 * skill_lv;
					else
						skillratio = 200 * skill_lv;
					skillratio = skillratio * status->get_lv(src) / 100;
					if( st->rhw.ele == ELE_FIRE )
						skillratio += 100 * skill_lv;
					break;
				case RK_STORMBLAST:
					skillratio = ((sd ? pc->checkskill(sd,RK_RUNEMASTERY) : 1) + status_get_int(src) / 8) * 100;
					break;
				case RK_PHANTOMTHRUST:
					skillratio = 50 * skill_lv + 10 * (sd ? pc->checkskill(sd,KN_SPEARMASTERY) : 10);
					RE_LVL_DMOD(150);
					break;
				/**
					* GC Guillotine Cross
					**/
				case GC_CROSSIMPACT:
					skillratio += 900 + 100 * skill_lv;
					RE_LVL_DMOD(120);
					break;
				case GC_PHANTOMMENACE:
					skillratio += 200;
					break;
				case GC_COUNTERSLASH:
					//ATK [{(Skill Level x 100) + 300} x Caster's Base Level / 120]% + ATK [(AGI x 2) + (Caster's Job Level x 4)]%
					skillratio += 200 + (100 * skill_lv);
					RE_LVL_DMOD(120);
					break;
				case GC_ROLLINGCUTTER:
					skillratio = 50 + 50 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case GC_CROSSRIPPERSLASHER:
					skillratio += 300 + 80 * skill_lv;
					RE_LVL_DMOD(100);
					if( sc && sc->data[SC_ROLLINGCUTTER] )
						skillratio += sc->data[SC_ROLLINGCUTTER]->val1 * status_get_agi(src);
					break;
				case GC_DARKCROW:
					skillratio += 100 * (skill_lv - 1);
					break;
				/**
					* Arch Bishop
					**/
				case AB_DUPLELIGHT_MELEE:
					skillratio += 10 * skill_lv;
					break;
				/**
					* Ranger
					**/
				case RA_ARROWSTORM:
					skillratio += 900 + 80 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case RA_AIMEDBOLT:
					skillratio += 400 + 50 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case RA_CLUSTERBOMB:
					skillratio += 100 + 100 * skill_lv;
					break;
				case RA_WUGDASH:// ATK 300%
					skillratio = 300;
					if( sc && sc->data[SC_DANCE_WITH_WUG] )
						skillratio += 10 * sc->data[SC_DANCE_WITH_WUG]->val1 * (2 + battle->calc_chorusbonus(sd));
					break;
				case RA_WUGSTRIKE:
					skillratio = 200 * skill_lv;
					if( sc && sc->data[SC_DANCE_WITH_WUG] )
						skillratio += 10 * sc->data[SC_DANCE_WITH_WUG]->val1 * (2 + battle->calc_chorusbonus(sd));
					break;
				case RA_WUGBITE:
					skillratio += 300 + 200 * skill_lv;
					if ( skill_lv == 5 ) skillratio += 100;
					break;
				case RA_SENSITIVEKEEN:
					skillratio = 150 * skill_lv;
					break;
				/**
					* Mechanic
					**/
				case NC_BOOSTKNUCKLE:
					skillratio = skill_lv * 100 + 200 + st->dex;
					RE_LVL_DMOD(120);
					break;
				case NC_PILEBUNKER:
					skillratio = skill_lv*100 + 300 + status_get_str(src);
					RE_LVL_DMOD(100);
					break;
				case NC_VULCANARM:
					skillratio = 70 * skill_lv + status_get_dex(src);
					RE_LVL_DMOD(120);
					break;
				case NC_FLAMELAUNCHER:
				case NC_COLDSLOWER:
					skillratio += 200 + 300 * skill_lv;
					RE_LVL_DMOD(150);
					break;
				case NC_ARMSCANNON:
					switch( tst->size ) {
						case SZ_SMALL: skillratio = 300 + 350 * skill_lv; break; // Medium
						case SZ_MEDIUM: skillratio = 300 + 400 * skill_lv; break;  // Small
						case SZ_BIG: skillratio = 300 + 300 * skill_lv; break;    // Large
					}
					RE_LVL_DMOD(120);
					break;
				case NC_AXEBOOMERANG:
					skillratio = 250 + 50 * skill_lv;
					if( sd ) {
						short index = sd->equip_index[EQI_HAND_R];
						if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON )
						skillratio += sd->inventory_data[index]->weight / 10;
					}
					RE_LVL_DMOD(100);
					break;
				case NC_POWERSWING:
					skillratio = 300 + 100*skill_lv + ( status_get_str(src)+status_get_dex(src) ) * status->get_lv(src) / 100;
					break;
				case NC_AXETORNADO:
					skillratio = 200 + 100 * skill_lv + st->vit;
					RE_LVL_DMOD(100);
					if( st->rhw.ele == ELE_WIND )
						skillratio = skillratio * 125 / 100;
					if ( distance_bl(src, target) > 2 ) // Will deal 75% damage outside of 5x5 area.
						skillratio = skillratio * 75 / 100;
					break;
				case SC_FATALMENACE:
					skillratio = 100 * (skill_lv+1);
					RE_LVL_DMOD(100);
					break;
				case SC_TRIANGLESHOT:
					skillratio = ( 300 + (skill_lv-1) * status_get_agi(src)/2 );
					RE_LVL_DMOD(120);
					break;
				case SC_FEINTBOMB:
					skillratio = (skill_lv+1) * (st->dex/2) * (sd?sd->status.job_level:50)/10;
					RE_LVL_DMOD(120);
					break;
				case LG_CANNONSPEAR:
					skillratio = (50 + st->str) * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case LG_BANISHINGPOINT:
					skillratio = 50 * skill_lv + 30 * (sd ? pc->checkskill(sd,SM_BASH) : 10);
					RE_LVL_DMOD(100);
					break;
				case LG_SHIELDPRESS:
					skillratio = 150 * skill_lv + st->str;
					if( sd ) {
						short index = sd->equip_index[EQI_HAND_L];
						if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
						skillratio += sd->inventory_data[index]->weight / 10;
					}
					RE_LVL_DMOD(100);
					break;
				case LG_PINPOINTATTACK:
					skillratio = 100 * skill_lv + 5 * st->agi;
					RE_LVL_DMOD(120);
					break;
				case LG_RAGEBURST:
					if( sc ){
						skillratio += -100 + (status_get_max_hp(src) - status_get_hp(src)) / 100 + sc->fv_counter * 200;
						clif->millenniumshield(src, (sc->fv_counter = 0));
					}
					RE_LVL_DMOD(100);
					break;
				case LG_SHIELDSPELL:
					if ( sd && skill_lv == 1 ) {
						struct item_data *shield_data = sd->inventory_data[sd->equip_index[EQI_HAND_L]];
						if( shield_data )
							skillratio = 4 * status->get_lv(src) + 10 * shield_data->def + 2 * st->vit;
						}
					else
						skillratio = 0; // Prevents ATK damage from being done on LV 2 usage since LV 2 us MATK. [Rytech]
					break;
				case LG_MOONSLASHER:
					skillratio = 120 * skill_lv + 80 * (sd ? pc->checkskill(sd,LG_OVERBRAND) : 5);
					RE_LVL_DMOD(100);
					break;
				case LG_OVERBRAND:
					skillratio += -100 + 400 * skill_lv + 50 * ((sd) ? pc->checkskill(sd,CR_SPEARQUICKEN) : 1);
					RE_LVL_DMOD(100);
					break;
				case LG_OVERBRAND_BRANDISH:
					skillratio += -100 + 300 * skill_lv + status_get_str(src) + status_get_dex(src);
					RE_LVL_DMOD(100);
					break;
				case LG_OVERBRAND_PLUSATK:
					skillratio = 200 * skill_lv + rnd_value( 10, 100);
					RE_LVL_DMOD(100);
					break;
				case LG_RAYOFGENESIS:
					skillratio = 300 + 300 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case LG_EARTHDRIVE:
					if( sd ) {
						short index = sd->equip_index[EQI_HAND_L];
						if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
						skillratio = (1 + skill_lv) * sd->inventory_data[index]->weight / 10;
					}
					RE_LVL_DMOD(100);
					break;
				case LG_HESPERUSLIT:
					skillratio = 120 * skill_lv;
					if( sc && sc->data[SC_BANDING] )
						skillratio += 200 * sc->data[SC_BANDING]->val2;
					if( sc && sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 5 )
						skillratio = skillratio * 150 / 100;
					if( sc && sc->data[SC_INSPIRATION] )
						skillratio += 600;
					RE_LVL_DMOD(100);
					break;
				case SR_DRAGONCOMBO:
					skillratio += 40 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case SR_SKYNETBLOW:
					if( sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == SR_DRAGONCOMBO )//ATK [{(Skill Level x 100) + (Caster AGI) + 150} x Caster Base Level / 100] %
						skillratio += 100 * skill_lv + status_get_agi(src) + 50;
					else //ATK [{(Skill Level x 80) + (Caster AGI)} x Caster Base Level / 100] %
						skillratio += -100 + 80 * skill_lv + status_get_agi(src);
					RE_LVL_DMOD(100);
					break;
				case SR_EARTHSHAKER:
					if( tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] || // [(Skill Level x 150) x (Caster Base Level / 100) + (Caster INT x 3)] %
						tsc->data[SC_CHASEWALK] || tsc->data[SC_CLOAKINGEXCEED] || tsc->data[SC__INVISIBILITY]) ){
						skillratio += -100 + 150 * skill_lv;
						RE_LVL_DMOD(100);
						skillratio += status_get_int(src) * 3;
					}else{ //[(Skill Level x 50) x (Caster Base Level / 100) + (Caster INT x 2)] %
						skillratio += 50 * (skill_lv-2);
						RE_LVL_DMOD(100);
						skillratio += status_get_int(src) * 2;
					}
					break;
				case SR_FALLENEMPIRE:// ATK [(Skill Level x 150 + 100) x Caster Base Level / 150] %
					skillratio += 150 *skill_lv;
					RE_LVL_DMOD(150);
					break;
				case SR_TIGERCANNON:// ATK [((Caster consumed HP + SP) / 4) x Caster Base Level / 100] %
					{
						int hp = status_get_max_hp(src) * (10 + 2 * skill_lv) / 100,
							sp = status_get_max_sp(src) * (6 + skill_lv) / 100;
						if( sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == SR_FALLENEMPIRE ) // ATK [((Caster consumed HP + SP) / 2) x Caster Base Level / 100] %
							skillratio += -100 + (hp+sp) / 2;
						else
							skillratio += -100 + (hp+sp) / 4;
						RE_LVL_DMOD(100);
					}
						break;
				case SR_RAMPAGEBLASTER:
					skillratio += 20 * skill_lv * (sd?sd->spiritball_old:5) - 100;
					if( sc && sc->data[SC_EXPLOSIONSPIRITS] ) {
						skillratio += sc->data[SC_EXPLOSIONSPIRITS]->val1 * 20;
						RE_LVL_DMOD(120);
					} else {
						RE_LVL_DMOD(150);
					}
					break;
				case SR_KNUCKLEARROW:
					if ( flag&4 || map->list[src->m].flag.gvg_castle || tst->mode&MD_BOSS ) {
						// ATK [(Skill Level x 150) + (1000 x Target current weight / Maximum weight) + (Target Base Level x 5) x (Caster Base Level / 150)] %
						skillratio = 150 * skill_lv + status->get_lv(target) * 5 * (status->get_lv(src) / 100) ;
						if( tsd && tsd->weight )
							skillratio += 100 * (tsd->weight / tsd->max_weight);
					}else // ATK [(Skill Level x 100 + 500) x Caster Base Level / 100] %
						skillratio += 400 + (100 * skill_lv);
					RE_LVL_DMOD(100);
					break;
				case SR_WINDMILL: // ATK [(Caster Base Level + Caster DEX) x Caster Base Level / 100] %
					skillratio = status->get_lv(src) + status_get_dex(src);
					RE_LVL_DMOD(100);
					break;
				case SR_GATEOFHELL:
					if( sc && sc->data[SC_COMBOATTACK]
						&& sc->data[SC_COMBOATTACK]->val1 == SR_FALLENEMPIRE )
						skillratio += 800 * skill_lv -100;
					else
						skillratio += 500 * skill_lv -100;
					RE_LVL_DMOD(100);
					break;
				case SR_GENTLETOUCH_QUIET:
					skillratio += 100 * skill_lv - 100 + status_get_dex(src);
					RE_LVL_DMOD(100);
					break;
				case SR_HOWLINGOFLION:
					skillratio += 300 * skill_lv - 100;
					RE_LVL_DMOD(150);
					break;
				case SR_RIDEINLIGHTNING: // ATK [{(Skill Level x 200) + Additional Damage} x Caster Base Level / 100] %
					if( (st->rhw.ele) == ELE_WIND || (st->lhw.ele) == ELE_WIND )
						skillratio += skill_lv * 50;
					skillratio += -100 + 200 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case WM_REVERBERATION_MELEE:
					skillratio += 200 + 100 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case WM_SEVERE_RAINSTORM_MELEE:
					skillratio = (st->agi + st->dex) * skill_lv / 5;
					RE_LVL_DMOD(100);
					break;
				case WM_GREAT_ECHO:
				{
					int chorusbonus = battle->calc_chorusbonus(sd);
					skillratio += 300 + 200 * skill_lv;
					//Chorus bonus don't count the first 2 Minstrel's/Wanderer's and only increases when their's 3 or more. [Rytech]
					if (chorusbonus >= 1 && chorusbonus <= 5)
						skillratio += 100<<(chorusbonus-1); // 1->100; 2->200; 3->400; 4->800; 5->1600
					RE_LVL_DMOD(100);
				}
					break;
				case GN_CART_TORNADO:
				{
					int strbonus = bst->str;
					skillratio = 50 * skill_lv + (sd ? sd->cart_weight : battle_config.max_cart_weight) / 10 / max(150 - strbonus, 1) + 50 * (sd ? pc->checkskill(sd, GN_REMODELING_CART) : 5);
				}
					break;
				case GN_CARTCANNON:
					skillratio += -100 + (int)(50.0f * (sd ? pc->checkskill(sd, GN_REMODELING_CART) : 5) * (st->int_ / 40.0f) + 60.0f * skill_lv);
					break;
				case GN_SPORE_EXPLOSION:
					skillratio = 100 * skill_lv + (200 + st->int_) * status->get_lv(src) / 100;
					/* Fall through */
				case GN_CRAZYWEED_ATK:
					skillratio += 400 + 100 * skill_lv;
					break;
				case GN_SLINGITEM_RANGEMELEEATK:
					if( sd ) {
						switch( sd->itemid ) {
							case ITEMID_APPLE_BOMB:
								skillratio = st->str + st->dex + 300;
								break;
							case ITEMID_MELON_BOMB:
								skillratio = st->str + st->dex + 500;
								break;
							case ITEMID_COCONUT_BOMB:
							case ITEMID_PINEAPPLE_BOMB:
							case ITEMID_BANANA_BOMB:
								skillratio = st->str + st->dex + 800;
								break;
							case ITEMID_BLACK_LUMP:
								skillratio = (st->str + st->agi + st->dex) / 3; // Black Lump
								break;
							case ITEMID_BLACK_HARD_LUMP:
								skillratio = (st->str + st->agi + st->dex) / 2; // Hard Black Lump
								break;
							case ITEMID_VERY_HARD_LUMP:
								skillratio = st->str + st->agi + st->dex; // Extremely Hard Black Lump
								break;
						}
					}
					break;
				case SO_VARETYR_SPEAR://ATK [{( Striking Level x 50 ) + ( Varetyr Spear Skill Level x 50 )} x Caster Base Level / 100 ] %
					skillratio += -100 + 50 * skill_lv + ( sd ? pc->checkskill(sd, SO_STRIKING) * 50 : 0 );
					if( sc && sc->data[SC_BLAST_OPTION] )
						skillratio += (sd ? sd->status.job_level * 5 : 0);
					break;
					// Physical Elemental Spirits Attack Skills
				case EL_CIRCLE_OF_FIRE:
				case EL_FIRE_BOMB_ATK:
				case EL_STONE_RAIN:
					skillratio += 200;
					break;
				case EL_FIRE_WAVE_ATK:
					skillratio += 500;
					break;
				case EL_TIDAL_WEAPON:
					skillratio += 1400;
					break;
				case EL_WIND_SLASH:
					skillratio += 100;
					break;
				case EL_HURRICANE:
					skillratio += 600;
					break;
				case EL_TYPOON_MIS:
				case EL_WATER_SCREW_ATK:
					skillratio += 900;
					break;
				case EL_STONE_HAMMER:
					skillratio += 400;
					break;
				case EL_ROCK_CRUSHER:
					skillratio += 700;
					break;
				case KO_JYUMONJIKIRI:
					skillratio += -100 + 150 * skill_lv;
					RE_LVL_DMOD(120);
					if( tsc && tsc->data[SC_KO_JYUMONJIKIRI] )
						skillratio += status->get_lv(src) * skill_lv;
					break;
				case KO_HUUMARANKA:
					skillratio += -100 + 150 * skill_lv + status_get_agi(src) + status_get_dex(src) + 100 * (sd ? pc->checkskill(sd, NJ_HUUMA) : 0);
					break;
				case KO_SETSUDAN:
					skillratio += -100 + 100 * skill_lv;
					RE_LVL_DMOD(100);
					break;
				case MH_NEEDLE_OF_PARALYZE:
					skillratio += 600 + 100 * skill_lv;
					break;
				case MH_STAHL_HORN:
					skillratio += 400 + 100 * skill_lv;
					break;
				case MH_LAVA_SLIDE:
					skillratio += -100 + 70 * skill_lv;
					break;
				case MH_TINDER_BREAKER:
				case MH_MAGMA_FLOW:
					skillratio += -100 + 100 * skill_lv;
					break;
				default:
					battle->calc_skillratio_weapon_unknown(&attack_type, src, target, &skill_id, &skill_lv, &skillratio, &flag);
					break;
			}
			//Skill damage modifiers that stack linearly
			if(sc && skill_id != PA_SACRIFICE){
#ifdef RENEWAL_EDP
				if( sc->data[SC_EDP] ){
					if( skill_id == AS_SONICBLOW ||
						skill_id == GC_COUNTERSLASH ||
						skill_id == GC_CROSSIMPACT )
							skillratio >>= 1;
				}
#endif
				if(sc->data[SC_OVERTHRUST])
					skillratio += sc->data[SC_OVERTHRUST]->val3;
				if(sc->data[SC_OVERTHRUSTMAX])
					skillratio += sc->data[SC_OVERTHRUSTMAX]->val2;
				if(sc->data[SC_BERSERK])
#ifndef RENEWAL
					skillratio += 100;
#else
					skillratio += 200;
				if( sc->data[SC_TRUESIGHT] )
					skillratio += 2*sc->data[SC_TRUESIGHT]->val1;
				if( sc->data[SC_LKCONCENTRATION] )
					skillratio += sc->data[SC_LKCONCENTRATION]->val2;
				if( sd && sd->status.weapon == W_KATAR && (i=pc->checkskill(sd,ASC_KATAR)) > 0 )
					skillratio += skillratio * (10 + 2 * i) / 100;
#endif
				if( (!skill_id || skill_id == KN_AUTOCOUNTER) && sc->data[SC_CRUSHSTRIKE] ){
					if( sd )
					{//ATK [{Weapon Level * (Weapon Upgrade Level + 6) * 100} + (Weapon ATK) + (Weapon Weight)]%
						short index = sd->equip_index[EQI_HAND_R];
						if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON )
							skillratio += -100 + sd->inventory_data[index]->weight/10 + st->rhw.atk +
								100 * sd->inventory_data[index]->wlv * (sd->status.inventory[index].refine + 6);
					}
					status_change_end(src, SC_CRUSHSTRIKE, INVALID_TIMER);
					skill->break_equip(src,EQP_WEAPON,2000,BCT_SELF); // 20% chance to destroy the weapon.
				}
			}
	}
	if( skillratio < 1 )
		return 0;
	return skillratio;
}

void battle_calc_skillratio_magic_unknown(int *attack_type, struct block_list *src, struct block_list *target, uint16 *skill_id, uint16 *skill_lv, int *skillratio, int *flag) {
}

void battle_calc_skillratio_weapon_unknown(int *attack_type, struct block_list *src, struct block_list *target, uint16 *skill_id, uint16 *skill_lv, int *skillratio, int *flag) {
}

/*==========================================
 * Check damage trough status.
 * ATK may be MISS, BLOCKED FAIL, reduce, increase, end status...
 * After this we apply bg/gvg reduction
 *------------------------------------------*/
int64 battle_calc_damage(struct block_list *src,struct block_list *bl,struct Damage *d,int64 damage,uint16 skill_id,uint16 skill_lv) {
	struct map_session_data *s_sd, *t_sd;
	struct status_change *s_sc, *sc;
	struct status_change_entry *sce;
	int div_, flag;

	nullpo_ret(bl);
	nullpo_ret(d);

	s_sd = BL_CAST(BL_PC, src);
	t_sd = BL_CAST(BL_PC, bl);
	div_ = d->div_;
	flag = d->flag;

	// need check src for null pointer?

	if( !damage )
		return 0;
	if( battle_config.ksprotection && mob->ksprotected(src, bl) )
		return 0;
	if (t_sd != NULL) {
		//Special no damage states
		if(flag&BF_WEAPON && t_sd->special_state.no_weapon_damage)
			damage -= damage * t_sd->special_state.no_weapon_damage / 100;

		if(flag&BF_MAGIC && t_sd->special_state.no_magic_damage)
			damage -= damage * t_sd->special_state.no_magic_damage / 100;

		if(flag&BF_MISC && t_sd->special_state.no_misc_damage)
			damage -= damage * t_sd->special_state.no_misc_damage / 100;

		if(!damage) return 0;
	}

	s_sc = status->get_sc(src);
	sc = status->get_sc(bl);

	if( sc && sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
		return 1;

	if (skill_id == PA_PRESSURE)
		return damage; //This skill bypass everything else.

	if( sc && sc->count )
	{
		//First, sc_*'s that reduce damage to 0.
		if( sc->data[SC_BASILICA] && !(status_get_mode(src)&MD_BOSS) )
		{
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}
		if( sc->data[SC_WHITEIMPRISON] && skill_id != HW_GRAVITATION ) { // Gravitation and Pressure do damage without removing the effect
			if( skill_id == MG_NAPALMBEAT ||
				skill_id == MG_SOULSTRIKE ||
				skill_id == WL_SOULEXPANSION ||
				(skill_id && skill->get_ele(skill_id, skill_lv) == ELE_GHOST) ||
				(!skill_id && (status->get_status_data(src))->rhw.ele == ELE_GHOST)
					){
				if( skill_id == WL_SOULEXPANSION )
					damage <<= 1; // If used against a player in White Imprison, the skill deals double damage.
				status_change_end(bl,SC_WHITEIMPRISON,INVALID_TIMER); // Those skills do damage and removes effect
			}else{
				d->dmg_lv = ATK_BLOCK;
				return 0;
			}
		}

		if( sc->data[SC_ZEPHYR] && ((flag&BF_LONG) || rnd()%100 < 10) ) {
				d->dmg_lv = ATK_BLOCK;
				return 0;
		}

		if( sc->data[SC_SAFETYWALL] && (flag&(BF_SHORT|BF_MAGIC))==BF_SHORT )
		{
			struct skill_unit_group* group = skill->id2group(sc->data[SC_SAFETYWALL]->val3);
			uint16 src_skill_id = sc->data[SC_SAFETYWALL]->val2;
			if (group) {
				d->dmg_lv = ATK_BLOCK;
				if(src_skill_id == MH_STEINWAND){
					if (--group->val2<=0)
						skill->del_unitgroup(group,ALC_MARK);
					if( (group->val3 - damage) > 0 )
						group->val3 -= (int)cap_value(damage, INT_MIN, INT_MAX);
					else
						skill->del_unitgroup(group,ALC_MARK);
					return 0;
				}
				if( skill_id == SO_ELEMENTAL_SHIELD ) {
					if ( ( group->val2 - damage) > 0 ) {
						group->val2 -= (int)cap_value(damage,INT_MIN,INT_MAX);
					} else
						skill->del_unitgroup(group,ALC_MARK);
					return 0;
				}
				/**
				 * in RE, SW possesses a lifetime equal to 3 times the caster's health
				 **/
			#ifdef RENEWAL
				if ( ( group->val2 - damage) > 0 ) {
					group->val2 -= (int)cap_value(damage,INT_MIN,INT_MAX);
				} else
					skill->del_unitgroup(group,ALC_MARK);
				if (--group->val3<=0)
					skill->del_unitgroup(group,ALC_MARK);
			#else
				if (--group->val2<=0)
					skill->del_unitgroup(group,ALC_MARK);
			#endif
				return 0;
			}
			status_change_end(bl, SC_SAFETYWALL, INVALID_TIMER);
		}

		if( ( sc->data[SC_PNEUMA] && (flag&(BF_MAGIC|BF_LONG)) == BF_LONG ) || sc->data[SC__MANHOLE] ) {
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}
		if( sc->data[SC_NEUTRALBARRIER] && (flag&(BF_MAGIC|BF_LONG)) == BF_LONG && skill_id != CR_ACIDDEMONSTRATION ) {
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}
		if( sc->data[SC__MAELSTROM] && (flag&BF_MAGIC) && skill_id && (skill->get_inf(skill_id)&INF_GROUND_SKILL) ) {
			// {(Maelstrom Skill LevelxAbsorbed Skill Level)+(Caster's Job/5)}/2
			int sp = (sc->data[SC__MAELSTROM]->val1 * skill_lv + (t_sd ? t_sd->status.job_level / 5 : 0)) / 2;
			status->heal(bl, 0, sp, 3);
			d->dmg_lv = ATK_BLOCK;
			return 0;
		}
		if( sc->data[SC_WEAPONBLOCKING] && flag&(BF_SHORT|BF_WEAPON) && rnd()%100 < sc->data[SC_WEAPONBLOCKING]->val2 )
		{
			clif->skill_nodamage(bl,src,GC_WEAPONBLOCKING,1,1);
			d->dmg_lv = ATK_BLOCK;
			sc_start2(src,bl,SC_COMBOATTACK,100,GC_WEAPONBLOCKING,src->id,2000);
			return 0;
		}
		if ((sce=sc->data[SC_AUTOGUARD]) && flag&BF_WEAPON && !(skill->get_nk(skill_id)&NK_NO_CARDFIX_ATK) && rnd()%100 < sce->val2) {
			int delay;
			struct status_change_entry *sce_d = sc->data[SC_DEVOTION];

			// different delay depending on skill level [celest]
			if (sce->val1 <= 5)
				delay = 300;
			else if (sce->val1 > 5 && sce->val1 <= 9)
				delay = 200;
			else
				delay = 100;

			if (sce_d) {
				// If the target is too far away from the devotion caster, autoguard has no effect
				// Autoguard will be disabled later on
				struct block_list *d_bl = map->id2bl(sce_d->val1);
				struct mercenary_data *d_md = BL_CAST(BL_MER, d_bl);
				struct map_session_data *d_sd = BL_CAST(BL_PC, d_bl);
				if (d_bl != NULL && check_distance_bl(bl, d_bl, sce_d->val3)
				  && ((d_bl->type == BL_MER && d_md->master != NULL && d_md->master->bl.id == bl->id)
				    || (d_bl->type == BL_PC && d_sd->devotion[sce_d->val2] == bl->id))
				) {
					// if player is target of devotion, show guard effect on the devotion caster rather than the target
					clif->skill_nodamage(d_bl, d_bl, CR_AUTOGUARD, sce->val1, 1);
					unit->set_walkdelay(d_bl, timer->gettick(), delay, 1);

					d->dmg_lv = ATK_MISS;
					return 0;
				}
			} else {
				clif->skill_nodamage(bl, bl, CR_AUTOGUARD, sce->val1, 1);
				unit->set_walkdelay(bl, timer->gettick(), delay, 1);

				if(sc->data[SC_CR_SHRINK] && rnd()%100<5*sce->val1)
					skill->blown(bl,src,skill->get_blewcount(CR_SHRINK,1),-1,0);

				d->dmg_lv = ATK_MISS;
				return 0;
			}
		}

		if( (sce = sc->data[SC_MILLENNIUMSHIELD]) && sce->val2 > 0 && damage > 0 ) {
			clif->skill_nodamage(bl, bl, RK_MILLENNIUMSHIELD, 1, 1);
			sce->val3 -= (int)cap_value(damage,INT_MIN,INT_MAX); // absorb damage
			d->dmg_lv = ATK_BLOCK;
			sc_start(src,bl,SC_STUN,15,0,skill->get_time2(RK_MILLENNIUMSHIELD,sce->val1)); // There is a chance to be stunned when one shield is broken.
			if( sce->val3 <= 0 ) { // Shield Down
				sce->val2--;
				if( sce->val2 > 0 ) {
					clif->millenniumshield(bl,sce->val2);
					sce->val3 = 1000; // Next Shield
				} else
					status_change_end(bl,SC_MILLENNIUMSHIELD,INVALID_TIMER); // All shields down
			}
			return 0;
		}

		if( (sce=sc->data[SC_PARRYING]) && flag&BF_WEAPON && skill_id != WS_CARTTERMINATION && rnd()%100 < sce->val2 )
		{ // attack blocked by Parrying
			clif->skill_nodamage(bl, bl, LK_PARRYING, sce->val1,1);
			return 0;
		}

		if(sc->data[SC_DODGE_READY] && ( !sc->opt1 || sc->opt1 == OPT1_BURNING ) &&
			(flag&BF_LONG || sc->data[SC_STRUP])
			&& rnd()%100 < 20) {
			if (t_sd && pc_issit(t_sd)) pc->setstand(t_sd); //Stand it to dodge.
			clif->skill_nodamage(bl,bl,TK_DODGE,1,1);
			if (!sc->data[SC_COMBOATTACK])
				sc_start4(src, bl, SC_COMBOATTACK, 100, TK_JUMPKICK, src->id, 1, 0, 2000);
			return 0;
		}

		if((sc->data[SC_HERMODE]) && flag&BF_MAGIC)
			return 0;

		if(sc->data[SC_NJ_TATAMIGAESHI] && (flag&(BF_MAGIC|BF_LONG)) == BF_LONG)
			return 0;

		if ((sce=sc->data[SC_KAUPE]) && rnd()%100 < sce->val2) {
			//Kaupe blocks damage (skill or otherwise) from players, mobs, homuns, mercenaries.
			clif->specialeffect(bl, 462, AREA);
			//Shouldn't end until Breaker's non-weapon part connects.
			if (skill_id != ASC_BREAKER || !(flag&BF_WEAPON))
				if (--(sce->val3) <= 0) //We make it work like Safety Wall, even though it only blocks 1 time.
					status_change_end(bl, SC_KAUPE, INVALID_TIMER);
			return 0;
		}

		if (flag&BF_MAGIC && (sce=sc->data[SC_PRESTIGE]) != NULL && rnd()%100 < sce->val2) {
			clif->specialeffect(bl, 462, AREA); // Still need confirm it.
			return 0;
		}

		if (((sce=sc->data[SC_NJ_UTSUSEMI]) || sc->data[SC_NJ_BUNSINJYUTSU])
		&& flag&BF_WEAPON && !(skill->get_nk(skill_id)&NK_NO_CARDFIX_ATK)) {

			skill->additional_effect (src, bl, skill_id, skill_lv, flag, ATK_BLOCK, timer->gettick() );
			if( !status->isdead(src) )
				skill->counter_additional_effect( src, bl, skill_id, skill_lv, flag, timer->gettick() );
			if (sce) {
				clif->specialeffect(bl, 462, AREA);
				skill->blown(src,bl,sce->val3,-1,0);
			}
			//Both need to be consumed if they are active.
			if (sce && --(sce->val2) <= 0)
				status_change_end(bl, SC_NJ_UTSUSEMI, INVALID_TIMER);
			if ((sce=sc->data[SC_NJ_BUNSINJYUTSU]) && --(sce->val2) <= 0)
				status_change_end(bl, SC_NJ_BUNSINJYUTSU, INVALID_TIMER);

			return 0;
		}

		//Now damage increasing effects
		if( sc->data[SC_LEXAETERNA] && skill_id != PF_SOULBURN
#ifdef RENEWAL
		&& skill_id != CR_ACIDDEMONSTRATION
#endif
		)
		{
			if( src->type != BL_MER || skill_id == 0 )
				damage <<= 1; // Lex Aeterna only doubles damage of regular attacks from mercenaries

			if( skill_id != ASC_BREAKER || !(flag&BF_WEAPON) )
				status_change_end(bl, SC_LEXAETERNA, INVALID_TIMER); //Shouldn't end until Breaker's non-weapon part connects.
		}

#ifdef RENEWAL
		if( sc->data[SC_RAID] ) {
			damage += damage * 20 / 100;

			if (--sc->data[SC_RAID]->val1 == 0)
				status_change_end(bl, SC_RAID, INVALID_TIMER);
		}
#endif

		if( damage ) {
			if( sc->data[SC_DEEP_SLEEP] ) {
				damage += damage / 2; // 1.5 times more damage while in Deep Sleep.
				status_change_end(bl,SC_DEEP_SLEEP,INVALID_TIMER);
			}
			if( s_sd && t_sd && sc->data[SC_COLD] && flag&BF_WEAPON ){
				switch(s_sd->status.weapon){
					case W_MACE:
					case W_2HMACE:
					case W_1HAXE:
					case W_2HAXE:
						damage = damage * 150/100;
						break;
					case W_MUSICAL:
					case W_WHIP:
						if(!t_sd->state.arrow_atk)
							break;
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
					case W_DAGGER:
					case W_1HSWORD:
					case W_2HSWORD:
						damage = damage * 50/100;
						break;
				}
			}
			if( sc->data[SC_SIREN] )
				status_change_end(bl,SC_SIREN,INVALID_TIMER);
		}

		//Finally damage reductions....
		// Assumptio doubles the def & mdef on RE mode, otherwise gives a reduction on the final damage. [Igniz]
#ifndef RENEWAL
		if( sc->data[SC_ASSUMPTIO] ) {
			if( map_flag_vs(bl->m) )
				damage = damage*2/3; //Receive 66% damage
			else
				damage >>= 1; //Receive 50% damage
		}
#endif

		if(sc->data[SC_DEFENDER] &&
#ifdef RENEWAL
			((flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON) || skill_id == CR_ACIDDEMONSTRATION))
#else
			(flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON)) // In pre-re Defender doesn't reduce damage from Acid Demonstration
#endif
			damage = damage * ( 100 - sc->data[SC_DEFENDER]->val2 ) / 100;

#ifndef RENEWAL
		if(sc->data[SC_GS_ADJUSTMENT] &&
			(flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON))
			damage -= damage * 20 / 100;
#endif
		if(sc->data[SC_FOGWALL]) {
			if(flag&BF_SKILL) { //25% reduction
				if ( !(skill->get_inf(skill_id)&INF_GROUND_SKILL) && !(skill->get_nk(skill_id)&NK_SPLASH) )
					damage -= 25*damage/100;
			} else if ((flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON)) {
				damage >>= 2; //75% reduction
			}
		}

		if ( sc->data[SC_WATER_BARRIER] )
			damage = damage * ( 100 - 20 ) / 100;

		if( sc->data[SC_FIRE_EXPANSION_SMOKE_POWDER] ) {
			if( (flag&(BF_SHORT|BF_WEAPON)) == (BF_SHORT|BF_WEAPON) )
				damage -= 15 * damage / 100;//15% reduction to physical melee attacks
			else if( (flag&(BF_LONG|BF_WEAPON)) == (BF_LONG|BF_WEAPON) )
				damage -= 50 * damage / 100;//50% reduction to physical ranged attacks
		}

		// Compressed code, fixed by map.h [Epoque]
		if (src->type == BL_MOB) {
			const struct mob_data *md = BL_UCCAST(BL_MOB, src);
			int i;
			if (sc->data[SC_MANU_DEF] != NULL) {
				for (i = 0; i < ARRAYLENGTH(mob->manuk); i++) {
					if (mob->manuk[i] == md->class_) {
						damage -= damage * sc->data[SC_MANU_DEF]->val1 / 100;
						break;
					}
				}
			}
			if (sc->data[SC_SPL_DEF] != NULL) {
				for (i = 0; i < ARRAYLENGTH(mob->splendide); i++) {
					if (mob->splendide[i] == md->class_) {
						damage -= damage * sc->data[SC_SPL_DEF]->val1 / 100;
						break;
					}
				}
			}
			if (sc->data[SC_MORA_BUFF] != NULL) {
				for (i = 0; i < ARRAYLENGTH(mob->mora); i++) {
					if (mob->mora[i] == md->class_) {
						damage -= damage * sc->data[SC_MORA_BUFF]->val1 / 100;
						break;
					}
				}
			}
		}

		if((sce=sc->data[SC_ARMOR]) && //NPC_DEFENDER
			sce->val3&flag && sce->val4&flag)
			damage -= damage * sc->data[SC_ARMOR]->val2 / 100;

		if( sc->data[SC_ENERGYCOAT] && (skill_id == GN_HELLS_PLANT_ATK ||
#ifdef RENEWAL
			((flag&BF_WEAPON || flag&BF_MAGIC) && skill_id != WS_CARTTERMINATION)
#else
			(flag&BF_WEAPON && skill_id != WS_CARTTERMINATION)
#endif
			) )
		{
			struct status_data *sstatus = status->get_status_data(bl);
			int per = 100*sstatus->sp / sstatus->max_sp -1; //100% should be counted as the 80~99% interval
			per /=20; //Uses 20% SP intervals.
			//SP Cost: 1% + 0.5% per every 20% SP
			if (!status->charge(bl, 0, (10+5*per)*sstatus->max_sp/1000))
				status_change_end(bl, SC_ENERGYCOAT, INVALID_TIMER);
			//Reduction: 6% + 6% every 20%
			damage -= damage * (6 * (1+per)) / 100;
		}
		if(sc->data[SC_GRANITIC_ARMOR]){
			damage -= damage * sc->data[SC_GRANITIC_ARMOR]->val2 / 100;
		}
		if(sc->data[SC_PAIN_KILLER]){
			damage -= damage * sc->data[SC_PAIN_KILLER]->val3 / 100;
		}
		if((sce=sc->data[SC_MAGMA_FLOW]) && (rnd()%100 <= sce->val2) ){
			skill->castend_damage_id(bl,src,MH_MAGMA_FLOW,sce->val1,timer->gettick(),0);
		}

		if( sc->data[SC_DARKCROW] && (flag&(BF_SHORT|BF_MAGIC)) == BF_SHORT )
			damage += damage * sc->data[SC_DARKCROW]->val2 / 100;

		if( (sce = sc->data[SC_STONEHARDSKIN]) && flag&(BF_SHORT|BF_WEAPON) && damage > 0 ) {
			sce->val2 -= (int)cap_value(damage,INT_MIN,INT_MAX);
			if( src->type == BL_PC ) {
				if (s_sd && s_sd->status.weapon != W_BOW)
					skill->break_equip(src, EQP_WEAPON, 3000, BCT_SELF);
			} else
				skill->break_equip(src, EQP_WEAPON, 3000, BCT_SELF);
			// 30% chance to reduce monster's ATK by 25% for 10 seconds.
			if( src->type == BL_MOB )
				sc_start(bl,src, SC_NOEQUIPWEAPON, 30, 0, skill->get_time2(RK_STONEHARDSKIN, sce->val1));
			if( sce->val2 <= 0 )
				status_change_end(bl, SC_STONEHARDSKIN, INVALID_TIMER);
		}

/**
 * In renewal steel body reduces all incoming damage by 1/10
 **/
#ifdef RENEWAL
		if( sc->data[SC_STEELBODY] ) {
			damage = damage > 10 ? damage / 10 : 1;
		}
#endif

		//Finally added to remove the status of immobile when aimedbolt is used. [Jobbie]
		if( skill_id == RA_AIMEDBOLT && (sc->data[SC_WUGBITE] || sc->data[SC_ANKLESNARE] || sc->data[SC_ELECTRICSHOCKER]) )
		{
			status_change_end(bl, SC_WUGBITE, INVALID_TIMER);
			status_change_end(bl, SC_ANKLESNARE, INVALID_TIMER);
			status_change_end(bl, SC_ELECTRICSHOCKER, INVALID_TIMER);
		}

		//Finally Kyrie because it may, or not, reduce damage to 0.
		if((sce = sc->data[SC_KYRIE]) && damage > 0){
			sce->val2 -= (int)cap_value(damage,INT_MIN,INT_MAX);
			if(flag&BF_WEAPON || skill_id == TF_THROWSTONE){
				if(sce->val2>=0)
					damage=0;
				else
					damage=-sce->val2;
			}
			if((--sce->val3)<=0 || (sce->val2<=0) || skill_id == AL_HOLYLIGHT)
				status_change_end(bl, SC_KYRIE, INVALID_TIMER);
		}

		if( sc->data[SC_MEIKYOUSISUI] && rnd()%100 < 40 ) // custom value
			damage = 0;

		if (!damage) return 0;

		if( (sce = sc->data[SC_LIGHTNINGWALK]) && flag&BF_LONG && rnd()%100 < sce->val1 ) {
			int dx[8]={0,-1,-1,-1,0,1,1,1};
			int dy[8]={1,1,0,-1,-1,-1,0,1};
			uint8 dir = map->calc_dir(bl, src->x, src->y);
			if( unit->movepos(bl, src->x-dx[dir], src->y-dy[dir], 1, 1) ) {
				clif->slide(bl,src->x-dx[dir],src->y-dy[dir]);
				unit->setdir(bl, dir);
			}
			d->dmg_lv = ATK_DEF;
			status_change_end(bl, SC_LIGHTNINGWALK, INVALID_TIMER);
			return 0;
		}

		//Probably not the most correct place, but it'll do here
		//(since battle_drain is strictly for players currently)
		if ((sce=sc->data[SC_HAMI_BLOODLUST]) && flag&BF_WEAPON && damage > 0 &&
			rnd()%100 < sce->val3)
			status->heal(src, damage*sce->val4/100, 0, 3);

		if( (sce = sc->data[SC_FORCEOFVANGUARD]) && flag&BF_WEAPON
			&& rnd()%100 < sce->val2 && sc->fv_counter <= sce->val3 )
				clif->millenniumshield(bl, sc->fv_counter++);

		if (sc->data[SC_STYLE_CHANGE] && rnd()%2) {
			struct homun_data *hd = BL_CAST(BL_HOM,bl);
			if (hd) homun->addspiritball(hd, 10); //add a sphere
		}

		if( sc->data[SC__DEADLYINFECT] && flag&BF_SHORT && damage > 0 && rnd()%100 < 30 + 10 * sc->data[SC__DEADLYINFECT]->val1 && !is_boss(src) )
			status->change_spread(bl, src); // Deadly infect attacked side

		if (t_sd && damage > 0 && (sce = sc->data[SC_GENTLETOUCH_ENERGYGAIN]) != NULL) {
			if ( rnd() % 100 < sce->val2 )
				pc->addspiritball(t_sd, skill->get_time(MO_CALLSPIRITS, 1), pc->getmaxspiritball(t_sd, 0));
		}
	}

	//SC effects from caster side.
	if (s_sc && s_sc->count) {
		if( s_sc->data[SC_INVINCIBLE] && !s_sc->data[SC_INVINCIBLEOFF] )
			damage += damage * 75 / 100;
		// [Epoque]
		if (bl->type == BL_MOB) {
			const struct mob_data *md = BL_UCCAST(BL_MOB, bl);
			int i;

			if (((sce=s_sc->data[SC_MANU_ATK]) != NULL && (flag&BF_WEAPON))
			 || ((sce=s_sc->data[SC_MANU_MATK]) != NULL && (flag&BF_MAGIC))) {
				for (i = 0; i < ARRAYLENGTH(mob->manuk); i++)
					if (md->class_ == mob->manuk[i]) {
						damage += damage * sce->val1 / 100;
						break;
					}
			}
			if (((sce=s_sc->data[SC_SPL_ATK]) != NULL && (flag&BF_WEAPON))
			 || ((sce=s_sc->data[SC_SPL_MATK]) != NULL && (flag&BF_MAGIC))) {
				for (i = 0; i < ARRAYLENGTH(mob->splendide); i++)
					if (md->class_ == mob->splendide[i]) {
						damage += damage * sce->val1 / 100;
						break;
					}
			}
		}
		if( s_sc->data[SC_POISONINGWEAPON] ) {
			struct status_data *tstatus = status->get_status_data(bl);
			if ( !(flag&BF_SKILL) && (flag&BF_WEAPON) && damage > 0 && rnd()%100 < s_sc->data[SC_POISONINGWEAPON]->val3 ) {
				short rate = 100;
				if ( s_sc->data[SC_POISONINGWEAPON]->val1 == 9 ) // Oblivion Curse gives a 2nd success chance after the 1st one passes which is reducible. [Rytech]
					rate = 100 - tstatus->int_ * 4 / 5;
				sc_start(src,bl,s_sc->data[SC_POISONINGWEAPON]->val2,rate,s_sc->data[SC_POISONINGWEAPON]->val1,skill->get_time2(GC_POISONINGWEAPON,1) - (tstatus->vit + tstatus->luk) / 2 * 1000);
			}
		}
		if( s_sc->data[SC__DEADLYINFECT] && flag&BF_SHORT && damage > 0 && rnd()%100 < 30 + 10 * s_sc->data[SC__DEADLYINFECT]->val1 && !is_boss(src) )
			status->change_spread(src, bl);
		if (s_sc->data[SC_SHIELDSPELL_REF] && s_sc->data[SC_SHIELDSPELL_REF]->val1 == 1 && damage > 0)
			skill->break_equip(bl,EQP_ARMOR,10000,BCT_ENEMY );
		if (s_sc->data[SC_STYLE_CHANGE] && rnd()%2) {
			struct homun_data *hd = BL_CAST(BL_HOM,bl);
			if (hd) homun->addspiritball(hd, 10);
		}
		if (src->type == BL_PC && damage > 0 && (sce = s_sc->data[SC_GENTLETOUCH_ENERGYGAIN]) != NULL) {
			if (s_sd != NULL && rnd() % 100 < sce->val2)
				pc->addspiritball(s_sd, skill->get_time(MO_CALLSPIRITS, 1), pc->getmaxspiritball(s_sd, 0));
		}
	}
	/* no data claims these settings affect anything other than players */
	if( damage && t_sd && bl->type == BL_PC ) {
		switch( skill_id ) {
			//case PA_PRESSURE: /* pressure also belongs to this list but it doesn't reach this area -- so don't worry about it */
			case HW_GRAVITATION:
			case NJ_ZENYNAGE:
			case KO_MUCHANAGE:
				break;
			default:
				if (flag & BF_SKILL) { //Skills get a different reduction than non-skills. [Skotlex]
					if (flag&BF_WEAPON)
						damage = damage * map->list[bl->m].weapon_damage_rate / 100;
					if (flag&BF_MAGIC)
						damage = damage * map->list[bl->m].magic_damage_rate / 100;
					if (flag&BF_MISC)
						damage = damage * map->list[bl->m].misc_damage_rate / 100;
				} else { //Normal attacks get reductions based on range.
					if (flag & BF_SHORT)
						damage = damage * map->list[bl->m].short_damage_rate / 100;
					if (flag & BF_LONG)
						damage = damage * map->list[bl->m].long_damage_rate / 100;
				}
				if(!damage) damage  = 1;
				break;
		}
	}

	if(battle_config.skill_min_damage && damage > 0 && damage < div_)
	{
		if ((flag&BF_WEAPON && battle_config.skill_min_damage&1)
			|| (flag&BF_MAGIC && battle_config.skill_min_damage&2)
			|| (flag&BF_MISC && battle_config.skill_min_damage&4)
		)
			damage = div_;
	}

	if( bl->type == BL_MOB && !status->isdead(bl) && src != bl) {
		struct mob_data *md = BL_UCAST(BL_MOB, bl);
		if (damage > 0)
			mob->skill_event(md, src, timer->gettick(), flag);
		if (skill_id)
			mob->skill_event(md, src, timer->gettick(), MSC_SKILLUSED|(skill_id<<16));
	}
	if (t_sd && pc_ismadogear(t_sd) && rnd()%100 < 50) {
		int element = -1;
		if (!skill_id || (element = skill->get_ele(skill_id, skill_lv)) == -1) {
			// Take weapon's element
			struct status_data *sstatus = NULL;
			if (s_sd != NULL && s_sd->bonus.arrow_ele != 0) {
				element = s_sd->bonus.arrow_ele;
			} else if ((sstatus = status->get_status_data(src)) != NULL) {
				element = sstatus->rhw.ele;
			}
		} else if (element == -2) {
			// Use enchantment's element
			element = status_get_attack_sc_element(src,status->get_sc(src));
		} else if (element == -3) {
			// Use random element
			element = rnd()%ELE_MAX;
		}
		if (element == ELE_FIRE)
			pc->overheat(t_sd, 1);
		else if (element == ELE_WATER)
			pc->overheat(t_sd, -1);
	}

	return damage;
}

/*==========================================
 * Calculates BG related damage adjustments.
 *------------------------------------------*/
// FIXME: flag is undocumented
int64 battle_calc_bg_damage(struct block_list *src, struct block_list *bl, int64 damage, int div_, uint16 skill_id, uint16 skill_lv, int flag) {

	if (!damage)
		return 0;

	nullpo_retr(damage, bl);
	if (bl->type == BL_MOB) {
		struct mob_data* md = BL_CAST(BL_MOB, bl);

		if (flag&BF_SKILL && (md->class_ == MOBID_OBJ_A2 || md->class_ == MOBID_OBJ_B2))
			return 0; // Crystal cannot receive skill damage on battlegrounds
	}

	return damage;
}

/*==========================================
 * Calculates GVG related damage adjustments.
 *------------------------------------------*/
// FIXME: flag is undocumented
int64 battle_calc_gvg_damage(struct block_list *src,struct block_list *bl,int64 damage,int div_,uint16 skill_id,uint16 skill_lv,int flag) {
	struct mob_data* md = BL_CAST(BL_MOB, bl);
	int class_ = status->get_class(bl);

	if (!damage) //No reductions to make.
		return 0;
	nullpo_retr(damage, src);
	nullpo_retr(damage, bl);

	if(md && md->guardian_data) {
		if (class_ == MOBID_EMPELIUM && flag&BF_SKILL) {
		//Skill immunity.
			switch (skill_id) {
#ifndef RENEWAL
			case MO_TRIPLEATTACK:
			case HW_GRAVITATION:
#endif
			case TF_DOUBLE:
				break;
			default:
				return 0;
			}
		}
		if(src->type != BL_MOB) {
			struct guild *g = NULL;
			if (src->type == BL_PC) {
				struct map_session_data *sd = BL_UCAST(BL_PC, src);
				g = sd->guild;
			} else {
				g = guild->search(status->get_guild_id(src));
			}

			if (class_ == MOBID_EMPELIUM && (!g || guild->checkskill(g,GD_APPROVAL) <= 0))
				return 0;

			if (g && battle_config.guild_max_castles && guild->checkcastles(g)>=battle_config.guild_max_castles)
				return 0; // [MouseJstr]
		}
	}

	switch (skill_id) {
		case PA_PRESSURE:
		case HW_GRAVITATION:
		case NJ_ZENYNAGE:
		case KO_MUCHANAGE:
		case NC_SELFDESTRUCTION:
			break;
		default:
			/* Uncomment if you want god-mode Emperiums at 100 defense. [Kisuka]
			if (md && md->guardian_data) {
				damage -= damage * (md->guardian_data->castle->defense/100) * battle_config.castle_defense_rate/100;
			}
			*/
			break;
	}
	return damage;
}

/*==========================================
 * HP/SP drain calculation
 *------------------------------------------*/
int battle_calc_drain(int64 damage, int rate, int per) {
	int64 diff = 0;

	if (per && rnd()%1000 < rate) {
		diff = (damage * per) / 100;
		if (diff == 0) {
			if (per > 0)
				diff = 1;
			else
				diff = -1;
		}
	}
	return (int)cap_value(diff,INT_MIN,INT_MAX);
}

/*==========================================
 * Consumes ammo for the given skill.
 *------------------------------------------*/
void battle_consume_ammo(struct map_session_data *sd, int skill_id, int lv)
{
	int qty=1;

	nullpo_retv(sd);
	if (!battle_config.arrow_decrement)
		return;

	if (skill_id && lv) {
		qty = skill->get_ammo_qty(skill_id, lv);
		if (!qty) qty = 1;
	}

	if(sd->equip_index[EQI_AMMO]>=0) //Qty check should have been done in skill_check_condition
		pc->delitem(sd, sd->equip_index[EQI_AMMO], qty, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);

	sd->state.arrow_atk = 0;
}

//Skill Range Criteria
int battle_range_type(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv) {
	nullpo_retr(BF_SHORT, src);
	nullpo_retr(BF_SHORT, target);

	if (battle_config.skillrange_by_distance &&
		(src->type&battle_config.skillrange_by_distance)
	) { //based on distance between src/target [Skotlex]
		if (check_distance_bl(src, target, 5))
			return BF_SHORT;
		return BF_LONG;
	}

	if (skill_id == SR_GATEOFHELL) {
		if (skill_lv < 5)
			return BF_SHORT;
		else
			return BF_LONG;
	}

	//based on used skill's range
	if (skill->get_range2(src, skill_id, skill_lv) < 5)
		return BF_SHORT;
	return BF_LONG;
}
int battle_adjust_skill_damage(int m, unsigned short skill_id) {
	if( map->list[m].skill_count ) {
		int i;
		ARR_FIND(0, map->list[m].skill_count, i, map->list[m].skills[i]->skill_id == skill_id );

		if( i < map->list[m].skill_count ) {
			return map->list[m].skills[i]->modifier;
		}
	}

	return 0;
}

int battle_blewcount_bonus(struct map_session_data *sd, uint16 skill_id) {
	int i;
	nullpo_ret(sd);
	if (!sd->skillblown[0].id)
		return 0;
	//Apply the bonus blow count. [Skotlex]
	for (i = 0; i < ARRAYLENGTH(sd->skillblown) && sd->skillblown[i].id; i++) {
		if (sd->skillblown[i].id == skill_id)
			return sd->skillblown[i].val;
	}
	return 0;
}
//For quick div adjustment.
#define damage_div_fix(dmg, div) do { if ((div) > 1) (dmg)*=(div); else if ((div) < 0) (div)*=-1; } while(0)
/*==========================================
 * battle_calc_magic_attack [DracoRPG]
 *------------------------------------------*/
// FIXME: mflag is undocumented
struct Damage battle_calc_magic_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int mflag) {
	int nk;
	short s_ele = 0;
	struct map_session_data *sd = NULL;
	struct status_change *sc;
	struct Damage ad;
	struct status_data *sstatus = status->get_status_data(src);
	struct status_data *tstatus = status->get_status_data(target);
	struct {
		unsigned imdef : 2;
		unsigned infdef : 1;
	} flag;

	memset(&ad,0,sizeof(ad));
	memset(&flag,0,sizeof(flag));

	nullpo_retr(ad, src);
	nullpo_retr(ad, target);

	//Initial Values
	ad.damage = 1;
	ad.div_=skill->get_num(skill_id,skill_lv);
	ad.amotion = (skill->get_inf(skill_id)&INF_GROUND_SKILL) ? 0 : sstatus->amotion; //Amotion should be 0 for ground skills.
	ad.dmotion=tstatus->dmotion;
	ad.blewcount = skill->get_blewcount(skill_id,skill_lv);
	ad.flag=BF_MAGIC|BF_SKILL;
	ad.dmg_lv=ATK_DEF;
	nk = skill->get_nk(skill_id);
	flag.imdef = (nk&NK_IGNORE_DEF)? 1 : 0;

	sd = BL_CAST(BL_PC, src);

	sc = status->get_sc(src);

	//Initialize variables that will be used afterwards
	s_ele = skill->get_ele(skill_id, skill_lv);

	if (s_ele == -1){ // pl=-1 : the skill takes the weapon's element
		s_ele = sstatus->rhw.ele;
		if (sd && sd->charm_type != CHARM_TYPE_NONE && sd->charm_count >= MAX_SPIRITCHARM) {
			//Summoning 10 spiritcharm will endow your weapon
			s_ele = sd->charm_type;
		}
	}else if (s_ele == -2) //Use status element
		s_ele = status_get_attack_sc_element(src,status->get_sc(src));
	else if( s_ele == -3 ) //Use random element
		s_ele = rnd()%ELE_MAX;

	if( skill_id == SO_PSYCHIC_WAVE ) {
		if( sc && sc->count ) {
			if( sc->data[SC_HEATER_OPTION] ) s_ele = sc->data[SC_HEATER_OPTION]->val4;
			else if( sc->data[SC_COOLER_OPTION] ) s_ele = sc->data[SC_COOLER_OPTION]->val4;
			else if( sc->data[SC_BLAST_OPTION] ) s_ele = sc->data[SC_BLAST_OPTION]->val3;
			else if( sc->data[SC_CURSED_SOIL_OPTION] ) s_ele = sc->data[SC_CURSED_SOIL_OPTION]->val4;
		}
	}

	//Set miscellaneous data that needs be filled
	if(sd) {
		sd->state.arrow_atk = 0;
		ad.blewcount += battle->blewcount_bonus(sd, skill_id);
	}

	//Skill Range Criteria
	ad.flag |= battle->range_type(src, target, skill_id, skill_lv);
	flag.infdef = (tstatus->mode&MD_PLANT) ? 1 : 0;
	if (!flag.infdef && target->type == BL_SKILL) {
		const struct skill_unit *su = BL_UCCAST(BL_SKILL, target);
		if (su->group->unit_id == UNT_REVERBERATION)
			flag.infdef = 1; // Reverberation takes 1 damage
	}

	switch(skill_id) {
		case MG_FIREWALL:
			if ( tstatus->def_ele == ELE_FIRE || battle->check_undead(tstatus->race, tstatus->def_ele) )
				ad.blewcount = 0; //No knockback
			break;
		case NJ_KAENSIN:
		case PR_SANCTUARY:
			ad.dmotion = 0; //No flinch animation.
			break;
		case WL_HELLINFERNO:
			if( mflag&ELE_DARK )
				s_ele = ELE_DARK;
			break;
		case KO_KAIHOU:
			if (sd && sd->charm_type != CHARM_TYPE_NONE && sd->charm_count > 0) {
				s_ele = sd->charm_type;
			}
			break;
#ifdef RENEWAL
		case CR_ACIDDEMONSTRATION:
		case ASC_BREAKER:
		case HW_MAGICCRASHER:
			flag.imdef = 2;
			break;
#endif
	}

	if (!flag.infdef) //No need to do the math for plants
	{
		int i;
#ifdef RENEWAL
		ad.damage = 0; //reinitialize..
#endif
//MATK_RATE scales the damage. 100 = no change. 50 is halved, 200 is doubled, etc
#define MATK_RATE( a ) ( ad.damage= ad.damage*(a)/100 )
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define MATK_ADDRATE( a ) ( ad.damage+= ad.damage*(a)/100 )
//Adds an absolute value to damage. 100 = +100 damage
#define MATK_ADD( a ) ( ad.damage+= (a) )

		switch (skill_id) {
			//Calc base damage according to skill
			case AL_HEAL:
			case PR_BENEDICTIO:
			case PR_SANCTUARY:
				ad.damage = skill->calc_heal(src, target, skill_id, skill_lv, false);
				break;
			/**
			 * Arch Bishop
			 **/
			case AB_HIGHNESSHEAL:
				ad.damage = skill->calc_heal(src, target, AL_HEAL, 10, false) * ( 17 + 3 * skill_lv ) / 10;
				break;
			case PR_ASPERSIO:
				ad.damage = 40;
				break;
			case ALL_RESURRECTION:
			case PR_TURNUNDEAD:
				//Undead check is on skill_castend_damageid code.
				i = 20*skill_lv + sstatus->luk + sstatus->int_ + status->get_lv(src)
				  + 200 - 200*tstatus->hp/tstatus->max_hp; // there is no changed in success chance in renewal. [malufett]
				if(i > 700) i = 700;
				if(rnd()%1000 < i && !(tstatus->mode&MD_BOSS))
					ad.damage = tstatus->hp;
				else {
				#ifdef RENEWAL
					MATK_ADD(status->get_matk(src, 2));
				#else
					ad.damage = status->get_lv(src) + sstatus->int_ + skill_lv * 10;
				#endif
				}
				break;
			case PF_SOULBURN:
				ad.damage = tstatus->sp * 2;
				break;
			/**
			 * Arch Bishop
			 **/
			case AB_RENOVATIO:
				//Damage calculation from iRO wiki. [Jobbie]
				ad.damage = status->get_lv(src) * 10 + sstatus->int_;
				break;
			default: {
				unsigned int skillratio = 100; //Skill dmg modifiers.
				MATK_ADD( status->get_matk(src, 2) );
#ifdef RENEWAL
				ad.damage = battle->calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, ad.damage, 0, ad.flag);
				ad.damage = battle->calc_cardfix2(src, target, ad.damage, s_ele, nk, ad.flag);
#endif
				if (nk&NK_SPLASHSPLIT) { // Divide MATK in case of multiple targets skill
					if(mflag>0)
						ad.damage/= mflag;
					else
						ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill->get_name(skill_id));
				}

				if (sc){
					if( sc->data[SC_TELEKINESIS_INTENSE] && s_ele == ELE_GHOST )
						ad.damage += sc->data[SC_TELEKINESIS_INTENSE]->val3;
				}
				switch(skill_id){
					case MG_FIREBOLT:
					case MG_COLDBOLT:
					case MG_LIGHTNINGBOLT:
						if ( sc && sc->data[SC_SPELLFIST] && mflag&BF_SHORT )  {
							skillratio = sc->data[SC_SPELLFIST]->val2 * 50 + sc->data[SC_SPELLFIST]->val4 * 100;// val4 = used bolt level, val2 = used spellfist level. [Rytech]
							ad.div_ = 1;// ad mods, to make it work similar to regular hits [Xazax]
							ad.flag = BF_WEAPON|BF_SHORT;
							ad.type = BDT_NORMAL;
						}
					/* Fall through */
					default:
						MATK_RATE(battle->calc_skillratio(BF_MAGIC, src, target, skill_id, skill_lv, skillratio, mflag));
				}
				//Constant/misc additions from skills
				if (skill_id == WZ_FIREPILLAR)
					MATK_ADD(100+50*skill_lv);
				if( sd && ( sd->status.class_ == JOB_ARCH_BISHOP_T || sd->status.class_ == JOB_ARCH_BISHOP ) &&
					(i=pc->checkskill(sd,AB_EUCHARISTICA)) > 0 &&
					(tstatus->race == RC_DEMON || tstatus->def_ele == ELE_DARK) )
					MATK_ADDRATE(i);
			}
		}
#ifndef HMAP_ZONE_DAMAGE_CAP_TYPE
		if (skill_id) {
			for(i = 0; i < map->list[target->m].zone->capped_skills_count; i++) {
				if( skill_id == map->list[target->m].zone->capped_skills[i]->nameid && (map->list[target->m].zone->capped_skills[i]->type & target->type) ) {
					if (target->type == BL_MOB && map->list[target->m].zone->capped_skills[i]->subtype != MZS_NONE) {
						const struct mob_data *md = BL_UCCAST(BL_MOB, target);
						if ((md->status.mode&MD_BOSS) && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_BOSS))
							continue;
						if (md->special_state.clone && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_CLONE))
							continue;
					}
					if( ad.damage > map->list[target->m].zone->capped_skills[i]->cap )
						ad.damage = map->list[target->m].zone->capped_skills[i]->cap;
					if( ad.damage2 > map->list[target->m].zone->capped_skills[i]->cap )
						ad.damage2 = map->list[target->m].zone->capped_skills[i]->cap;
					break;
				}
			}
		}
#endif
		if(sd) {
			uint16 rskill;/* redirect skill */
			//Damage bonuses
			if ((i = pc->skillatk_bonus(sd, skill_id)))
				ad.damage += ad.damage*i/100;
			switch(skill_id){
				case WL_CHAINLIGHTNING_ATK:
					rskill = WL_CHAINLIGHTNING;
					break;
				case AB_DUPLELIGHT_MAGIC:
					rskill = AB_DUPLELIGHT;
					break;
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
					rskill = WL_TETRAVORTEX;
					break;
				case WL_SUMMON_ATK_FIRE:
				case WL_SUMMON_ATK_WIND:
				case WL_SUMMON_ATK_WATER:
				case WL_SUMMON_ATK_GROUND:
					rskill = WL_RELEASE;
					break;
				case WM_REVERBERATION_MAGIC:
					rskill = WM_REVERBERATION;
					break;
				default:
					rskill = skill_id;
			}
			if( (i = battle->adjust_skill_damage(src->m,rskill)) )
				MATK_RATE(i);

			//Ignore Defense?
			if (!flag.imdef && (
				sd->bonus.ignore_mdef_ele & ( 1 << tstatus->def_ele ) ||
				sd->bonus.ignore_mdef_race & map->race_id2mask(tstatus->race) ||
				sd->bonus.ignore_mdef_race & map->race_id2mask(is_boss(target) ? RC_BOSS : RC_NONBOSS)
			))
				flag.imdef = 1;
		}

		ad.damage = battle->calc_defense(BF_MAGIC, src, target, skill_id, skill_lv, ad.damage, flag.imdef, 0);

		if(ad.damage<1)
			ad.damage=1;
		else if(sc){//only applies when hit
			// TODO: there is another factor that contribute with the damage and need to be formulated. [malufett]
			switch(skill_id){
				case MG_LIGHTNINGBOLT:
				case MG_THUNDERSTORM:
				case MG_FIREBOLT:
				case MG_FIREWALL:
				case MG_COLDBOLT:
				case MG_FROSTDIVER:
				case WZ_EARTHSPIKE:
				case WZ_HEAVENDRIVE:
					if(sc->data[SC_GUST_OPTION] || sc->data[SC_PETROLOGY_OPTION]
						|| sc->data[SC_PYROTECHNIC_OPTION] || sc->data[SC_AQUAPLAY_OPTION])
						ad.damage += (6 + sstatus->int_/4) + max(sstatus->dex-10,0)/30;
					break;
			}
		}

		if (!(nk&NK_NO_ELEFIX))
			ad.damage=battle->attr_fix(src, target, ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

		if( skill_id == CR_GRANDCROSS || skill_id == NPC_GRANDDARKNESS )
		{ //Apply the physical part of the skill's damage. [Skotlex]
			struct Damage wd = battle->calc_weapon_attack(src,target,skill_id,skill_lv,mflag);
			ad.damage = battle->attr_fix(src, target, wd.damage + ad.damage, s_ele, tstatus->def_ele, tstatus->ele_lv) * (100 + 40*skill_lv)/100;
			if( src == target )
			{
				if( src->type == BL_PC )
					ad.damage = ad.damage/2;
				else
					ad.damage = 0;
			}
		}
#ifndef RENEWAL
		ad.damage = battle->calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, ad.damage, 0, ad.flag);
#endif
	}

	damage_div_fix(ad.damage, ad.div_);

	if (flag.infdef && ad.damage)
		ad.damage = ad.damage>0?1:-1;
	if (skill_id != ASC_BREAKER)
		ad.damage = battle->calc_damage(src, target, &ad, ad.damage, skill_id, skill_lv);
	if( map_flag_gvg2(target->m) )
		ad.damage=battle->calc_gvg_damage(src,target,ad.damage,ad.div_,skill_id,skill_lv,ad.flag);
	else if( map->list[target->m].flag.battleground )
		ad.damage=battle->calc_bg_damage(src,target,ad.damage,ad.div_,skill_id,skill_lv,ad.flag);

	switch( skill_id ) { /* post-calc modifiers */
		case SO_VARETYR_SPEAR: { // Physical damage.
			struct Damage wd = battle->calc_weapon_attack(src,target,skill_id,skill_lv,mflag);
			if(!flag.infdef && ad.damage > 1)
				ad.damage += wd.damage;
			break;
		}
		//case HM_ERASER_CUTTER:
	}

	return ad;
#undef MATK_RATE
#undef MATK_ADDRATE
#undef MATK_ADD
}

/*==========================================
 * Calculate Misc damage for skill_id
 *------------------------------------------*/
// FIXME: mflag is undocumented
struct Damage battle_calc_misc_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int mflag) {
	int temp;
	short i, nk;
	short s_ele;

	struct map_session_data *sd, *tsd;
	struct Damage md; //DO NOT CONFUSE with md of mob_data!
	struct status_data *sstatus = status->get_status_data(src);
	struct status_data *tstatus = status->get_status_data(target);
	struct status_change *tsc = status->get_sc(target);
#ifdef RENEWAL
	struct status_change *sc = status->get_sc(src);
#endif

	memset(&md,0,sizeof(md));

	nullpo_retr(md, src);
	nullpo_retr(md, target);

	//Some initial values
	md.amotion = (skill->get_inf(skill_id)&INF_GROUND_SKILL) ? 0 : sstatus->amotion;
	md.dmotion=tstatus->dmotion;
	md.div_=skill->get_num( skill_id,skill_lv );
	md.blewcount=skill->get_blewcount(skill_id,skill_lv);
	md.dmg_lv=ATK_DEF;
	md.flag=BF_MISC|BF_SKILL;

	nk = skill->get_nk(skill_id);

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	if(sd) {
		sd->state.arrow_atk = 0;
		md.blewcount += battle->blewcount_bonus(sd, skill_id);
	}

	s_ele = skill->get_ele(skill_id, skill_lv);
	if (s_ele < 0 && s_ele != -3) //Attack that takes weapon's element for misc attacks? Make it neutral [Skotlex]
		s_ele = ELE_NEUTRAL;
	else if (s_ele == -3) //Use random element
		s_ele = rnd()%ELE_MAX;

	//Skill Range Criteria
	md.flag |= battle->range_type(src, target, skill_id, skill_lv);

	switch( skill_id )
	{
#ifdef RENEWAL
	case HT_LANDMINE:
	case MA_LANDMINE:
	case HT_BLASTMINE:
	case HT_CLAYMORETRAP:
		md.damage = skill_lv * sstatus->dex * (3+status->get_lv(src)/100) * (1+sstatus->int_/35);
		md.damage += md.damage * (rnd()%20-10) / 100;
		md.damage += 40 * (sd?pc->checkskill(sd,RA_RESEARCHTRAP):0);
		break;
#else
	case HT_LANDMINE:
	case MA_LANDMINE:
		md.damage=skill_lv*(sstatus->dex+75)*(100+sstatus->int_)/100;
		break;
	case HT_BLASTMINE:
		md.damage=skill_lv*(sstatus->dex/2+50)*(100+sstatus->int_)/100;
		break;
	case HT_CLAYMORETRAP:
		md.damage=skill_lv*(sstatus->dex/2+75)*(100+sstatus->int_)/100;
		break;
#endif
	case HT_BLITZBEAT:
	case SN_FALCONASSAULT:
		//Blitz-beat Damage.
		if(!sd || (temp = pc->checkskill(sd,HT_STEELCROW)) <= 0)
			temp=0;
		md.damage=(sstatus->dex/10+sstatus->int_/2+temp*3+40)*2;
		if(mflag > 1) //Autocasted Blitz.
			nk|=NK_SPLASHSPLIT;

		if (skill_id == SN_FALCONASSAULT) {
			//Div fix of Blitzbeat
			temp = skill->get_num(HT_BLITZBEAT, 5);
			damage_div_fix(md.damage, temp);

			//Falcon Assault Modifier
			md.damage=md.damage*(150+70*skill_lv)/100;
		}
		break;
	case TF_THROWSTONE:
		md.damage=50;
		break;
	case BA_DISSONANCE:
		md.damage=30+skill_lv*10;
		if (sd)
			md.damage+= 3*pc->checkskill(sd,BA_MUSICALLESSON);
		break;
	case NPC_SELFDESTRUCTION:
		md.damage = sstatus->hp;
		break;
	case NPC_SMOKING:
		md.damage=3;
		break;
	case NPC_DARKBREATH:
		md.damage = 500 + (skill_lv-1)*1000 + rnd()%1000;
		if(md.damage > 9999) md.damage = 9999;
		break;
	case PA_PRESSURE:
		md.damage=500+300*skill_lv;
		break;
	case PA_GOSPEL:
		md.damage = 1+rnd()%9999;
		break;
	case CR_ACIDDEMONSTRATION:
#ifdef RENEWAL
		{// [malufett]
			int64 matk=0, atk;
			short tdef = status->get_total_def(target);
			short tmdef =  status->get_total_mdef(target);
			int targetVit = min(120, status_get_vit(target));
			short totaldef = (tmdef + tdef - ((uint64)(tmdef + tdef) >> 32)) >> 1; // FIXME: What's the >> 32 supposed to do here? tmdef and tdef are both 16-bit...

			matk = battle->calc_magic_attack(src, target, skill_id, skill_lv, mflag).damage;
			atk = battle->calc_base_damage(src, target, skill_id, skill_lv, nk, false, s_ele, ELE_NEUTRAL, EQI_HAND_R, (sc && sc->data[SC_MAXIMIZEPOWER]?1:0)|(sc && sc->data[SC_WEAPONPERFECT]?8:0), md.flag);
			md.damage = matk + atk;
			if( src->type == BL_MOB ){
				totaldef = (tdef + tmdef) >> 1;
				md.damage = 7 * targetVit * skill_lv * (atk + matk) / 100;
				/*
				// Pending [malufett]
				if( unknown condition ){
					md.damage = 7 * md.damage % 20;
					md.damage = 7 * md.damage / 20;
				}*/
			}else{
				float vitfactor = 0.0f, ftemp;

				if( (vitfactor=(status_get_vit(target)-120.0f)) > 0)
					vitfactor = (vitfactor * (matk + atk) / 10) / status_get_vit(target);
				ftemp = max(0, vitfactor) + (targetVit * (matk + atk)) / 10;
				md.damage = (int64)(ftemp * 70 * skill_lv / 100);
				if (target->type == BL_PC)
					md.damage >>= 1;
			}
			md.damage -= totaldef;
			if( tsc && tsc->data[SC_LEXAETERNA] ) {
				md.damage <<= 1;
				status_change_end(target, SC_LEXAETERNA, INVALID_TIMER);
			}
		}
#else
		// updated the formula based on a Japanese formula found to be exact [Reddozen]
		if(tstatus->vit+sstatus->int_) //crash fix
			md.damage = (int)(7*tstatus->vit*sstatus->int_*sstatus->int_ / (10*(tstatus->vit+sstatus->int_)));
		else
			md.damage = 0;
		if (tsd) md.damage>>=1;
#endif
		// Some monsters have totaldef higher than md.damage in some cases, leading to md.damage < 0
		if( md.damage < 0 )
			md.damage = 0;
		if( md.damage > INT_MAX>>1 )
			//Overflow prevention, will anyone whine if I cap it to a few billion?
			//Not capped to INT_MAX to give some room for further damage increase.
			md.damage = INT_MAX>>1;
		break;

	case KO_MUCHANAGE:
		md.damage = skill->get_zeny(skill_id ,skill_lv);
		md.damage = md.damage * (50 + rnd()%50) / 100;
		if ( is_boss(target) || (sd && !pc->checkskill(sd,NJ_TOBIDOUGU)) )
			md.damage >>= 1;
		break;
	case NJ_ZENYNAGE:
		md.damage = skill->get_zeny(skill_id ,skill_lv);
		if (!md.damage) md.damage = 2;
		md.damage = rnd()%md.damage + md.damage;
		if (is_boss(target))
			md.damage=md.damage / 3;
		else if (tsd)
			md.damage=md.damage / 2;
		break;
	case GS_FLING:
		md.damage = sd?sd->status.job_level:status->get_lv(src);
		break;
	case HVAN_EXPLOSION: //[orn]
		md.damage = sstatus->max_hp * (50 + 50 * skill_lv) / 100;
		break ;
	case ASC_BREAKER:
		{
#ifndef RENEWAL
		md.damage = 500+rnd()%500 + 5*skill_lv * sstatus->int_;
		nk|=NK_IGNORE_FLEE|NK_NO_ELEFIX; //These two are not properties of the weapon based part.
#else
		int ratio = 300 + 50 * skill_lv;
		int64 matk = battle->calc_magic_attack(src, target, skill_id, skill_lv, mflag).damage;
		short totaldef = status->get_total_def(target) + status->get_total_mdef(target);
		int64 atk = battle->calc_base_damage(src, target, skill_id, skill_lv, nk, false, s_ele, ELE_NEUTRAL, EQI_HAND_R, (sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), md.flag);
#ifdef RENEWAL_EDP
		if( sc && sc->data[SC_EDP] )
			ratio >>= 1;
#endif
		md.damage = (matk + atk) * ratio / 100;
		md.damage -= totaldef;
#endif
		}
		break;
	case HW_GRAVITATION:
		md.damage = 200+200*skill_lv;
		md.dmotion = 0; //No flinch animation.
		break;
	case NPC_EVILLAND:
		md.damage = skill->calc_heal(src,target,skill_id,skill_lv,false);
		break;
	case RK_DRAGONBREATH:
	case RK_DRAGONBREATH_WATER:
		md.damage = ((status_get_hp(src) / 50) + (status_get_max_sp(src) / 4)) * skill_lv;
		RE_LVL_MDMOD(150);
		if (sd) md.damage = md.damage * (95 + 5 * pc->checkskill(sd,RK_DRAGONTRAINING)) / 100;
		md.flag |= BF_LONG|BF_WEAPON;
		break;
	/**
	 * Ranger
	 **/
	case RA_CLUSTERBOMB:
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
		md.damage = (int64)skill_lv * sstatus->dex + sstatus->int_ * 5 ;
		RE_LVL_TMDMOD();
		if(sd)
		{
			int researchskill_lv = pc->checkskill(sd,RA_RESEARCHTRAP);
			if(researchskill_lv)
				md.damage = md.damage * 20 * researchskill_lv / (skill_id == RA_CLUSTERBOMB?50:100);
			else
				md.damage = 0;
		}else
			md.damage = md.damage * 200 / (skill_id == RA_CLUSTERBOMB?50:100);

		break;
	case WM_SOUND_OF_DESTRUCTION:
		md.damage = 1000 * (int64)skill_lv + sstatus->int_ * (sd ? pc->checkskill(sd,WM_LESSON) : 10);
		md.damage += md.damage * 10 * battle->calc_chorusbonus(sd) / 100;
		break;
	/**
	 * Mechanic
	 **/
	case NC_SELFDESTRUCTION:
		{
#ifdef RENEWAL
			short totaldef = status->get_total_def(target);
#else
			short totaldef = tstatus->def2 + (short)status->get_def(target);
#endif
			md.damage = ( (sd?pc->checkskill(sd,NC_MAINFRAME):10) + 8 ) * ( skill_lv + 1 ) * ( status_get_sp(src) + sstatus->vit );
			RE_LVL_MDMOD(100);
			md.damage += status_get_hp(src) - totaldef;
		}
		break;
	case NC_MAGMA_ERUPTION:
		md.damage = 1200 + 400 * skill_lv;
		break;
	case GN_THORNS_TRAP:
		md.damage = 100 + 200 * skill_lv + sstatus->int_;
		break;
	case GN_HELLS_PLANT_ATK:
		md.damage = skill_lv * status->get_lv(target) * 10 + sstatus->int_ * 7 / 2 * (18 + (sd ? sd->status.job_level : 0) / 4) * (5 / (10 - (sd ? pc->checkskill(sd, AM_CANNIBALIZE) : 0)));
		md.damage = md.damage*(1000 + tstatus->mdef) / (1000 + tstatus->mdef * 10) - tstatus->mdef2;
		break;
	case KO_HAPPOKUNAI:
		{
			struct Damage wd = battle->calc_weapon_attack(src, target, 0, 1, mflag);
#ifdef RENEWAL
			short totaldef = status->get_total_def(target);
#else
			short totaldef = tstatus->def2 + (short)status->get_def(target);
#endif
			if (sd != NULL)
				wd.damage += sd->bonus.arrow_atk;
			md.damage = (int)(3 * (1 + wd.damage) * (5 + skill_lv) / 5.0f);
			md.damage -= totaldef;

		}
		break;
	default:
		battle->calc_misc_attack_unknown(src, target, &skill_id, &skill_lv, &mflag, &md);
		break;
	}

	if (nk&NK_SPLASHSPLIT){ // Divide ATK among targets
		if(mflag>0)
			md.damage/= mflag;
		else
			ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill->get_name(skill_id));
	}

	damage_div_fix(md.damage, md.div_);

	if (!(nk&NK_IGNORE_FLEE))
	{
		i = 0; //Temp for "hit or no hit"
		if(tsc && tsc->opt1 && tsc->opt1 != OPT1_STONEWAIT && tsc->opt1 != OPT1_BURNING)
			i = 1;
		else {
			short
				flee = tstatus->flee,
#ifdef RENEWAL
				hitrate = 0; //Default hitrate
#else
				hitrate = 80; //Default hitrate
#endif

			if(battle_config.agi_penalty_type && battle_config.agi_penalty_target&target->type) {
				unsigned char attacker_count; //256 max targets should be a sane max
				attacker_count = unit->counttargeted(target);
				if(attacker_count >= battle_config.agi_penalty_count)
				{
					if (battle_config.agi_penalty_type == 1)
						flee = (flee * (100 - (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
					else // assume type 2: absolute reduction
						flee -= (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
					if(flee < 1) flee = 1;
				}
			}

			hitrate+= sstatus->hit - flee;
#ifdef RENEWAL
			if( sd ) //in Renewal hit bonus from Vultures Eye is not anymore shown in status window
				hitrate += pc->checkskill(sd,AC_VULTURE);
#endif
			if( skill_id == KO_MUCHANAGE )
				hitrate = (int)((10 - ((float)1 / (status_get_dex(src) + status_get_luk(src))) * 500) * ((float)skill_lv / 2 + 5));

			hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate);

			if(rnd()%100 < hitrate)
				i = 1;
		}
		if (!i) {
			md.damage = 0;
			md.dmg_lv=ATK_FLEE;
		}
	}
#ifndef HMAP_ZONE_DAMAGE_CAP_TYPE
	if (skill_id) {
		for(i = 0; i < map->list[target->m].zone->capped_skills_count; i++) {
			if( skill_id == map->list[target->m].zone->capped_skills[i]->nameid && (map->list[target->m].zone->capped_skills[i]->type & target->type) ) {
				if (target->type == BL_MOB && map->list[target->m].zone->capped_skills[i]->subtype != MZS_NONE) {
					const struct mob_data *t_md = BL_UCCAST(BL_MOB, target);
					if ((t_md->status.mode&MD_BOSS) && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_BOSS))
						continue;
					if (t_md->special_state.clone && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_CLONE))
						continue;
				}
				if( md.damage > map->list[target->m].zone->capped_skills[i]->cap )
					md.damage = map->list[target->m].zone->capped_skills[i]->cap;
				if( md.damage2 > map->list[target->m].zone->capped_skills[i]->cap )
					md.damage2 = map->list[target->m].zone->capped_skills[i]->cap;
				break;
			}
		}
	}
#endif
	md.damage = battle->calc_cardfix(BF_MISC, src, target, nk, s_ele, 0, md.damage, 0, md.flag);
	md.damage = battle->calc_cardfix2(src, target, md.damage, s_ele, nk, md.flag);
	if(skill_id){
		uint16 rskill;/* redirect skill id */
		switch(skill_id){
			case GN_HELLS_PLANT_ATK:
				rskill = GN_HELLS_PLANT;
				break;
			default:
				rskill = skill_id;
		}
		if (sd && (i = pc->skillatk_bonus(sd, rskill)) != 0)
			md.damage += md.damage*i/100;
	}
	if( (i = battle->adjust_skill_damage(src->m,skill_id)) )
		md.damage = md.damage * i / 100;

	if(md.damage < 0)
		md.damage = 0;
	else if(md.damage && tstatus->mode&MD_PLANT){
		switch(skill_id){
			case HT_LANDMINE:
			case MA_LANDMINE:
			case HT_BLASTMINE:
			case HT_CLAYMORETRAP:
			case RA_CLUSTERBOMB:
#ifdef RENEWAL
				break;
#endif
			default:
				md.damage = 1;
		}
	}

	if(!(nk&NK_NO_ELEFIX))
		md.damage=battle->attr_fix(src, target, md.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);

	md.damage=battle->calc_damage(src,target,&md,md.damage,skill_id,skill_lv);
	if( map_flag_gvg2(target->m) )
		md.damage=battle->calc_gvg_damage(src,target,md.damage,md.div_,skill_id,skill_lv,md.flag);
	else if( map->list[target->m].flag.battleground )
		md.damage=battle->calc_bg_damage(src,target,md.damage,md.div_,skill_id,skill_lv,md.flag);

	switch( skill_id ) {
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			if( md.damage == 1 ) break;
		case RA_CLUSTERBOMB:
			{
				struct Damage wd;
				wd = battle->calc_weapon_attack(src,target,skill_id,skill_lv,mflag);
				md.damage += wd.damage;
			}
			break;
		case NJ_ZENYNAGE:
			if( sd ) {
				if ( md.damage > sd->status.zeny )
					md.damage = sd->status.zeny;
				pc->payzeny(sd, (int)cap_value(md.damage,INT_MIN,INT_MAX),LOG_TYPE_STEAL,NULL);
			}
		break;
	}

	return md;
}

void battle_calc_misc_attack_unknown(struct block_list *src, struct block_list *target, uint16 *skill_id, uint16 *skill_lv, int *mflag, struct Damage *md) {
}

/*==========================================
 * battle_calc_weapon_attack (by Skotlex)
 *------------------------------------------*/
// FIXME: wflag is undocumented
struct Damage battle_calc_weapon_attack(struct block_list *src,struct block_list *target,uint16 skill_id,uint16 skill_lv,int wflag)
{
	short temp=0;
	short s_ele, s_ele_;
	int i, nk;
	bool n_ele = false; // non-elemental

	struct map_session_data *sd, *tsd;
	struct Damage wd;
	struct status_change *sc = status->get_sc(src);
	struct status_change *tsc = status->get_sc(target);
	struct status_data *sstatus = status->get_status_data(src);
	struct status_data *tstatus = status->get_status_data(target);
	struct {
		unsigned hit : 1;    ///< the attack Hit? (not a miss)
		unsigned cri : 1;    ///< Critical hit
		unsigned idef : 1;   ///< Ignore defense
		unsigned idef2 : 1;  ///< Ignore defense (left weapon)
		unsigned pdef : 2;   ///< Pierces defense (Investigate/Ice Pick)
		unsigned pdef2 : 2;  ///< 1: Use def+def2/100, 2: Use def+def2/50
		unsigned infdef : 1; ///< Infinite defense (plants)
		unsigned arrow : 1;  ///< Attack is arrow-based
		unsigned rh : 1;     ///< Attack considers right hand (wd.damage)
		unsigned lh : 1;     ///< Attack considers left hand (wd.damage2)
		unsigned weapon : 1; ///< It's a weapon attack (consider VVS, and all that)
#ifdef RENEWAL
		unsigned tdef : 1;   ///< Total defense reduction
		unsigned distinct : 1; ///< Has its own battle calc formula
#endif
	} flag;

	memset(&wd,0,sizeof(wd));
	memset(&flag,0,sizeof(flag));

	nullpo_retr(wd, src);
	nullpo_retr(wd, target);

	//Initial flag
	flag.rh=1;
	flag.weapon=1;
	flag.infdef = (tstatus->mode&MD_PLANT && skill_id != RA_CLUSTERBOMB?1:0);
#ifdef RENEWAL
	if (skill_id == HT_FREEZINGTRAP)
		flag.infdef = 0;
#endif
	if (!flag.infdef && target->type == BL_SKILL) {
		const struct skill_unit *su = BL_UCCAST(BL_SKILL, target);
		if (su->group->unit_id == UNT_REVERBERATION)
			flag.infdef = 1; // Reverberation takes 1 damage
	}

	//Initial Values
	wd.type = BDT_NORMAL;
	wd.div_ = skill_id ? skill->get_num(skill_id,skill_lv) : 1;
	wd.amotion=(skill_id && skill->get_inf(skill_id)&INF_GROUND_SKILL)?0:sstatus->amotion; //Amotion should be 0 for ground skills.
	if(skill_id == KN_AUTOCOUNTER)
		wd.amotion >>= 1;
	wd.dmotion=tstatus->dmotion;
	wd.blewcount = skill_id ? skill->get_blewcount(skill_id,skill_lv) : 0;
	wd.flag = BF_WEAPON; //Initial Flag
	wd.flag |= (skill_id||wflag)?BF_SKILL:BF_NORMAL; // Baphomet card's splash damage is counted as a skill. [Inkfish]
	wd.dmg_lv=ATK_DEF; //This assumption simplifies the assignation later
	nk = skill->get_nk(skill_id);
	if( !skill_id && wflag ) //If flag, this is splash damage from Baphomet Card and it always hits.
		nk |= NK_NO_CARDFIX_ATK|NK_IGNORE_FLEE;
	flag.hit = (nk&NK_IGNORE_FLEE) ? 1 : 0;
	flag.idef = flag.idef2 = (nk&NK_IGNORE_DEF) ? 1 : 0;
#ifdef RENEWAL
	flag.tdef = 0;
#endif
	if (sc && !sc->count)
		sc = NULL; //Skip checking as there are no status changes active.
	if (tsc && !tsc->count)
		tsc = NULL; //Skip checking as there are no status changes active.

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	if(sd)
		wd.blewcount += battle->blewcount_bonus(sd, skill_id);

	//Set miscellaneous data that needs be filled regardless of hit/miss
	if(
		(sd && sd->state.arrow_atk) ||
		(!sd && ((skill_id && skill->get_ammotype(skill_id)) || sstatus->rhw.range>3))
	)
		flag.arrow = 1;

	if(skill_id) {
		wd.flag |= battle->range_type(src, target, skill_id, skill_lv);
		switch(skill_id) {
			case MO_FINGEROFFENSIVE:
				if(sd) {
					if (battle_config.finger_offensive_type)
						wd.div_ = 1;
					else
						wd.div_ = sd->spiritball_old;
				}
				break;
			case HT_PHANTASMIC:
				//Since these do not consume ammo, they need to be explicitly set as arrow attacks.
				flag.arrow = 1;
				break;
#ifndef RENEWAL
			case PA_SHIELDCHAIN:
			case CR_SHIELDBOOMERANG:
#endif
			case LG_SHIELDPRESS:
			case LG_EARTHDRIVE:
				flag.weapon = 0;
				break;

			case KN_PIERCE:
			case ML_PIERCE:
				wd.div_= (wd.div_>0?tstatus->size+1:-(tstatus->size+1));
				break;

			case TF_DOUBLE: //For NPC used skill.
			case GS_CHAINACTION:
				wd.type = BDT_MULTIHIT;
				break;

			case GS_GROUNDDRIFT:
			case KN_SPEARSTAB:
			case KN_BOWLINGBASH:
			case MS_BOWLINGBASH:
			case MO_BALKYOUNG:
			case TK_TURNKICK:
				wd.blewcount=0;
				break;

			case KN_AUTOCOUNTER:
				wd.flag=(wd.flag&~BF_SKILLMASK)|BF_NORMAL;
				break;

			case NPC_CRITICALSLASH:
			case LG_PINPOINTATTACK:
				flag.cri = 1; //Always critical skill.
				break;

			case LK_SPIRALPIERCE:
				if (!sd) wd.flag=(wd.flag&~(BF_RANGEMASK|BF_WEAPONMASK))|BF_LONG|BF_MISC;
				break;

			//When in banding, the number of hits is equal to the number of Royal Guards in banding.
			case LG_HESPERUSLIT:
				if( sc && sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 3 )
					wd.div_ = sc->data[SC_BANDING]->val2;
				break;

			case MO_INVESTIGATE:
				flag.pdef = flag.pdef2 = 2;
				break;

			case RA_AIMEDBOLT:
				if( tsc && (tsc->data[SC_WUGBITE] || tsc->data[SC_ANKLESNARE] || tsc->data[SC_ELECTRICSHOCKER]) )
					wd.div_ = tstatus->size + 2 + ( (rnd()%100 < 50-tstatus->size*10) ? 1 : 0 );
				break;

			case NPC_EARTHQUAKE:
				wd.flag = (wd.flag&~(BF_WEAPON)) | BF_MAGIC;
				break;
#ifdef RENEWAL
			case MO_EXTREMITYFIST:
			case GS_PIERCINGSHOT:
			case AM_ACIDTERROR:
			case AM_DEMONSTRATION:
			case NJ_ISSEN:
			case PA_SACRIFICE:
				flag.distinct = 1;
				break;
			case GN_CARTCANNON:
			case PA_SHIELDCHAIN:
			case GS_MAGICALBULLET:
			case NJ_SYURIKEN:
			case KO_BAKURETSU:
				flag.distinct = 1;
				/* Fall through */
			case NJ_KUNAI:
			case HW_MAGICCRASHER:
				flag.tdef = 1;
				break;
#endif
		}
	} else //Range for normal attacks.
		wd.flag |= flag.arrow?BF_LONG:BF_SHORT;
	if ((!skill_id || skill_id == PA_SACRIFICE) && tstatus->flee2 && rnd()%1000 < tstatus->flee2) {
		//Check for Lucky Dodge
		wd.type = BDT_PDODGE;
		wd.dmg_lv=ATK_LUCKY;
		if (wd.div_ < 0) wd.div_*=-1;
		return wd;
	}

	s_ele = s_ele_ = skill_id ? skill->get_ele(skill_id, skill_lv) : -1;
	if (s_ele == -1) {
		//Take weapon's element
		s_ele = sstatus->rhw.ele;
		s_ele_ = sstatus->lhw.ele;
		if (sd && sd->charm_type != CHARM_TYPE_NONE && sd->charm_count >= MAX_SPIRITCHARM) {
			//Summoning 10 spiritcharm will endow your weapon.
			s_ele = s_ele_ = sd->charm_type;
		}
		if( flag.arrow && sd && sd->bonus.arrow_ele )
			s_ele = sd->bonus.arrow_ele;
		if( battle_config.attack_attr_none&src->type )
			n_ele = true; //Weapon's element is "not elemental"
	} else if (s_ele == -2) {
		//Use enchantment's element
		s_ele = s_ele_ = status_get_attack_sc_element(src,sc);
	} else if (s_ele == -3) {
		//Use random element
		s_ele = s_ele_ = rnd()%ELE_MAX;
	}
	switch (skill_id) {
		case GS_GROUNDDRIFT:
			s_ele = s_ele_ = wflag; //element comes in flag.
			break;
		case LK_SPIRALPIERCE:
			if (!sd) n_ele = false; //forced neutral for monsters
			break;
		case LG_HESPERUSLIT:
			if ( sc && sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 == 5 )
				s_ele = ELE_HOLY; // Banding with 5 RGs: change atk element to Holy.
		break;
	}

	if (!(nk & NK_NO_ELEFIX) && !n_ele)
	    if (src->type == BL_HOM)
		n_ele = true; //skill is "not elemental"
	if (sc && sc->data[SC_GOLDENE_FERSE] && ((!skill_id && (rnd() % 100 < sc->data[SC_GOLDENE_FERSE]->val4)) || skill_id == MH_STAHL_HORN)) {
	    s_ele = s_ele_ = ELE_HOLY;
	    n_ele = false;
	}

	if(!skill_id) {
		//Skills ALWAYS use ONLY your right-hand weapon (tested on Aegis 10.2)
		if (sd && sd->weapontype1 == 0 && sd->weapontype2 > 0)
		{
			flag.rh=0;
			flag.lh=1;
		}
		if (sstatus->lhw.atk)
			flag.lh=1;
	}

	if (sd && !skill_id) {
		//Check for double attack.
		if (( (skill_lv=pc->checkskill(sd,TF_DOUBLE)) > 0 && sd->weapontype1 == W_DAGGER )
		 || ( sd->bonus.double_rate > 0 && sd->weapontype1 != W_FIST ) //Will fail bare-handed
		 || ( sc && sc->data[SC_KAGEMUSYA] && sd->weapontype1 != W_FIST ) // Need confirmation
		) {
			//Success chance is not added, the higher one is used [Skotlex]
			if( rnd()%100 < ( 5*skill_lv > sd->bonus.double_rate ? 5*skill_lv : sc && sc->data[SC_KAGEMUSYA]?sc->data[SC_KAGEMUSYA]->val1*3:sd->bonus.double_rate ) )
			{
				wd.div_ = skill->get_num(TF_DOUBLE,skill_lv?skill_lv:1);
				wd.type = BDT_MULTIHIT;
			}
		}
		else if( sd->weapontype1 == W_REVOLVER && (skill_lv = pc->checkskill(sd,GS_CHAINACTION)) > 0 && rnd()%100 < 5*skill_lv )
		{
			wd.div_ = skill->get_num(GS_CHAINACTION,skill_lv);
			wd.type = BDT_MULTIHIT;
		}
		else if(sc && sc->data[SC_FEARBREEZE] && sd->weapontype1==W_BOW
			&& (i = sd->equip_index[EQI_AMMO]) >= 0 && sd->inventory_data[i] && sd->status.inventory[i].amount > 1){
				int chance = rnd()%100;
				switch(sc->data[SC_FEARBREEZE]->val1){
					case 5:
						if( chance < 3){// 3 % chance to attack 5 times.
							wd.div_ = 5;
							break;
						}
					case 4:
						if( chance < 7){// 6 % chance to attack 4 times.
							wd.div_ = 4;
							break;
						}
					case 3:
						if( chance < 10){// 9 % chance to attack 3 times.
							wd.div_ = 3;
							break;
						}
					case 2:
					case 1:
						if( chance < 13){// 12 % chance to attack 2 times.
							wd.div_ = 2;
							break;
						}
				}
				if ( wd.div_ > 1 ) {
					wd.div_ = min(wd.div_, sd->status.inventory[i].amount);
					sc->data[SC_FEARBREEZE]->val4 = wd.div_ - 1;
					wd.type = BDT_MULTIHIT;
				}
		}
	}

	//Check for critical
	if( !flag.cri && wd.type != BDT_MULTIHIT && sstatus->cri &&
		(!skill_id ||
		skill_id == KN_AUTOCOUNTER ||
		skill_id == SN_SHARPSHOOTING || skill_id == MA_SHARPSHOOTING ||
		skill_id == NJ_KIRIKAGE))
	{
		short cri = sstatus->cri;
		if (sd != NULL) {
			// if show_katar_crit_bonus is enabled, it already done the calculation in status.c
			if (!battle_config.show_katar_crit_bonus && sd->status.weapon == W_KATAR) {
				cri <<= 1;
			}

			cri+= sd->critaddrace[tstatus->race];

			if (flag.arrow) {
				cri += sd->bonus.arrow_cri;
			}
		}
		if (sc && sc->data[SC_CAMOUFLAGE])
			cri += 10 * (10-sc->data[SC_CAMOUFLAGE]->val4);
#ifndef RENEWAL
		//The official equation is *2, but that only applies when sd's do critical.
		//Therefore, we use the old value 3 on cases when an sd gets attacked by a mob
		cri -= tstatus->luk*(!sd&&tsd?3:2);
#else
		cri -= status->get_lv(target) / 15 + 2 * status_get_luk(target);
#endif

		if( tsc && tsc->data[SC_SLEEP] ) {
			cri <<= 1;
		}
		switch (skill_id) {
			case 0:
				if(!(sc && sc->data[SC_AUTOCOUNTER]))
					break;
				status_change_end(src, SC_AUTOCOUNTER, INVALID_TIMER);
			case KN_AUTOCOUNTER:
				if(battle_config.auto_counter_type &&
					(battle_config.auto_counter_type&src->type))
					flag.cri = 1;
				else
					cri <<= 1;
				break;
			case SN_SHARPSHOOTING:
			case MA_SHARPSHOOTING:
				cri += 200;
				break;
			case NJ_KIRIKAGE:
				cri += 250 + 50*skill_lv;
				break;
		}
		if(tsd && tsd->bonus.critical_def)
			cri = cri * ( 100 - tsd->bonus.critical_def ) / 100;
		if (rnd()%1000 < cri)
			flag.cri = 1;
	}
	if (flag.cri) {
		wd.type = BDT_CRIT;
#ifndef RENEWAL
		flag.idef = flag.idef2 =
#endif
		flag.hit = 1;
	} else {
		//Check for Perfect Hit
		if(sd && sd->bonus.perfect_hit > 0 && rnd()%100 < sd->bonus.perfect_hit)
			flag.hit = 1;
		if (sc && sc->data[SC_FUSION]) {
			flag.hit = 1; //SG_FUSION always hit [Komurka]
			flag.idef = flag.idef2 = 1; //def ignore [Komurka]
		}
		if( !flag.hit )
			switch(skill_id)
			{
				case AS_SPLASHER:
					if( !wflag ) // Always hits the one exploding.
						flag.hit = 1;
					break;
				case CR_SHIELDBOOMERANG:
					if( sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_CRUSADER )
						flag.hit = 1;
					break;
			}
		if (tsc && !flag.hit && tsc->opt1 && tsc->opt1 != OPT1_STONEWAIT && tsc->opt1 != OPT1_BURNING)
			flag.hit = 1;
	}

	if (!flag.hit) {
		//Hit/Flee calculation
		short flee = tstatus->flee;
#ifdef RENEWAL
		short hitrate = 0; //Default hitrate
#else
		short hitrate = 80; //Default hitrate
#endif

		if(battle_config.agi_penalty_type && battle_config.agi_penalty_target&target->type) {
			unsigned char attacker_count; //256 max targets should be a sane max
			attacker_count = unit->counttargeted(target);
			if(attacker_count >= battle_config.agi_penalty_count) {
				if (battle_config.agi_penalty_type == 1)
					flee = (flee * (100 - (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num))/100;
				else //asume type 2: absolute reduction
					flee -= (attacker_count - (battle_config.agi_penalty_count - 1))*battle_config.agi_penalty_num;
				if(flee < 1) flee = 1;
			}
		}

		hitrate+= sstatus->hit - flee;

		if(wd.flag&BF_LONG && !skill_id && //Fogwall's hit penalty is only for normal ranged attacks.
			tsc && tsc->data[SC_FOGWALL])
			hitrate -= 50;

		if(sd && flag.arrow)
			hitrate += sd->bonus.arrow_hit;
#ifdef RENEWAL
		if( sd ) //in Renewal hit bonus from Vultures Eye is not anymore shown in status window
			hitrate += pc->checkskill(sd,AC_VULTURE);
#endif
		switch(skill_id) {
			//Hit skill modifiers
			//It is proven that bonus is applied on final hitrate, not hit.
			case SM_BASH:
			case MS_BASH:
				hitrate += hitrate * 5 * skill_lv / 100;
				break;
			case MS_MAGNUM:
			case SM_MAGNUM:
				hitrate += hitrate * 10 * skill_lv / 100;
				break;
			case KN_AUTOCOUNTER:
			case PA_SHIELDCHAIN:
			case NPC_WATERATTACK:
			case NPC_GROUNDATTACK:
			case NPC_FIREATTACK:
			case NPC_WINDATTACK:
			case NPC_POISONATTACK:
			case NPC_HOLYATTACK:
			case NPC_DARKNESSATTACK:
			case NPC_UNDEADATTACK:
			case NPC_TELEKINESISATTACK:
			case NPC_BLEEDING:
			case NPC_EARTHQUAKE:
			case NPC_FIREBREATH:
			case NPC_ICEBREATH:
			case NPC_THUNDERBREATH:
			case NPC_ACIDBREATH:
			case NPC_DARKNESSBREATH:
				hitrate += hitrate * 20 / 100;
				break;
			case KN_PIERCE:
			case ML_PIERCE:
				hitrate += hitrate * 5 * skill_lv / 100;
				break;
			case AS_SONICBLOW:
				if(sd && pc->checkskill(sd,AS_SONICACCEL)>0)
					hitrate += hitrate * 50 / 100;
				break;
			case MC_CARTREVOLUTION:
			case GN_CART_TORNADO:
			case GN_CARTCANNON:
				if( sd && pc->checkskill(sd, GN_REMODELING_CART) )
					hitrate += pc->checkskill(sd, GN_REMODELING_CART) * 4;
				break;
			case GC_VENOMPRESSURE:
				hitrate += 10 + 4 * skill_lv;
				break;
			case SC_FATALMENACE:
				hitrate -= 35 - 5 * skill_lv;
				break;
			case LG_BANISHINGPOINT:
				hitrate += 3 * skill_lv;
				break;
		}

		if( sd ) {
			// Weaponry Research hidden bonus
			if ((temp = pc->checkskill(sd,BS_WEAPONRESEARCH)) > 0)
				hitrate += hitrate * ( 2 * temp ) / 100;

			if( (sd->status.weapon == W_1HSWORD || sd->status.weapon == W_DAGGER) &&
			   (temp = pc->checkskill(sd, GN_TRAINING_SWORD))>0 )
				hitrate += 3 * temp;
		}

		hitrate = cap_value(hitrate, battle_config.min_hitrate, battle_config.max_hitrate);
#ifdef RENEWAL
		if( !sd )
			hitrate = cap_value(hitrate, 5, 95);
#endif
		if(rnd()%100 >= hitrate){
			wd.dmg_lv = ATK_FLEE;
			if (skill_id == SR_GATEOFHELL)
				flag.hit = 1;/* will hit with the special */
		}
		else
			flag.hit = 1;
	} //End hit/miss calculation

	if (flag.hit && !flag.infdef) { //No need to do the math for plants
		unsigned int skillratio = 100; //Skill dmg modifiers.
		//Hitting attack

//Assuming that 99% of the cases we will not need to check for the flag.rh... we don't.
//ATK_RATE scales the damage. 100 = no change. 50 is halved, 200 is doubled, etc
#define ATK_RATE( a ) do { int64 temp__ = (a); wd.damage= wd.damage*temp__/100 ; if(flag.lh) wd.damage2= wd.damage2*temp__/100; } while(0)
#define ATK_RATER(a) ( wd.damage = wd.damage*(a)/100 )
#define ATK_RATEL(a) ( wd.damage2 = wd.damage2*(a)/100 )
//Adds dmg%. 100 = +100% (double) damage. 10 = +10% damage
#define ATK_ADDRATE( a ) do { int64 temp__ = (a); wd.damage+= wd.damage*temp__/100; if(flag.lh) wd.damage2+= wd.damage2*temp__/100; } while(0)
//Adds an absolute value to damage. 100 = +100 damage
#define ATK_ADD( a ) do { int64 temp__ = (a); wd.damage += temp__; if (flag.lh) wd.damage2 += temp__; } while(0)
#define ATK_ADD2( a , b ) do { wd.damage += (a); if (flag.lh) wd.damage2 += (b); } while(0)
#ifdef RENEWAL
#define GET_NORMAL_ATTACK( f , s ) ( wd.damage = battle->calc_base_damage(src, target, s, skill_lv, nk, n_ele, s_ele, s_ele_, EQI_HAND_R, (f), wd.flag) )
#define GET_NORMAL_ATTACK2( f , s ) ( wd.damage2 = battle->calc_base_damage(src, target, s, skill_lv, nk, n_ele, s_ele, s_ele_, EQI_HAND_L, (f), wd.flag) )
#endif
		switch (skill_id) {
			//Calc base damage according to skill
			case PA_SACRIFICE:
				wd.damage = sstatus->max_hp* 9/100;
				wd.damage2 = 0;
#ifdef RENEWAL
				wd.damage = battle->calc_elefix(src, target, skill_id, skill_lv, wd.damage, nk, n_ele, s_ele, s_ele_, false, wd.flag); // temporary [malufett]
#endif
				break;
			case NJ_ISSEN: // [malufett]
#ifndef RENEWAL
				wd.damage = 40*sstatus->str +skill_lv*(sstatus->hp/10 + 35);
				wd.damage2 = 0;
#else
				{
					short totaldef = status->get_total_def(target);
					i = 0;
					GET_NORMAL_ATTACK( (sc && sc->data[SC_MAXIMIZEPOWER]?1:0)|(sc && sc->data[SC_WEAPONPERFECT]?8:0), 0 );
					if( sc && sc->data[SC_NJ_BUNSINJYUTSU] && (i=sc->data[SC_NJ_BUNSINJYUTSU]->val2) > 0 )
						wd.div_ = ~( i++ + 2 ) + 1;
					if( wd.damage ){
						wd.damage *= sstatus->hp * skill_lv;
						wd.damage = wd.damage / sstatus->max_hp + sstatus->hp + i * (wd.damage / sstatus->max_hp + sstatus->hp) / 5;
					}
					ATK_ADD(-totaldef);
					if( is_boss(target) )
						ATK_RATE(50);
				}
				break;
			case NJ_SYURIKEN: // [malufett]
				GET_NORMAL_ATTACK( (sc && sc->data[SC_MAXIMIZEPOWER]?1:0)|(sc && sc->data[SC_WEAPONPERFECT]?8:0), 0);
				ATK_ADD(battle->calc_masteryfix(src, target, skill_id, skill_lv, 4 * skill_lv + (sd ? sd->bonus.arrow_atk : 0), wd.div_, 0, flag.weapon));
#endif
				break;
#ifndef RENEWAL
			case LK_SPIRALPIERCE:
			case ML_SPIRALPIERCE:
				if (sd) {
					short index = sd->equip_index[EQI_HAND_R];

					if (index >= 0 &&
						sd->inventory_data[index] &&
						sd->inventory_data[index]->type == IT_WEAPON)
						wd.damage = sd->inventory_data[index]->weight*8/100; //80% of weight
					ATK_ADDRATE(50*skill_lv); //Skill modifier applies to weight only.
				} else {
					wd.damage = battle->calc_base_damage2(sstatus, &sstatus->rhw, sc, tstatus->size, sd, 0); //Monsters have no weight and use ATK instead
				}
				i = sstatus->str/10;
				i*=i;
				ATK_ADD(i); //Add str bonus.
				switch (tstatus->size) { //Size-fix. Is this modified by weapon perfection?
					case SZ_SMALL: //Small: 125%
						ATK_RATE(125);
						break;
					//case SZ_MEDIUM: //Medium: 100%
					case SZ_BIG: //Large: 75%
						ATK_RATE(75);
						break;
				}
				break;

			case PA_SHIELDCHAIN:
#endif
			case CR_SHIELDBOOMERANG:
				wd.damage = sstatus->batk;
				if (sd) {
					int damagevalue = 0;
					short index = sd->equip_index[EQI_HAND_L];

					if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
						damagevalue = sd->inventory_data[index]->weight/10;
					ATK_ADD(damagevalue);
				} else
					ATK_ADD(sstatus->rhw.atk2); //Else use Atk2
				break;
			case HFLI_SBR44: //[orn]
				if (src->type == BL_HOM) {
					const struct homun_data *hd = BL_UCCAST(BL_HOM, src);
					wd.damage = hd->homunculus.intimacy;
					break;
				}
				break;
			default:
			{
				i = (flag.cri
#ifdef RENEWAL
					|| (sc && sc->data[SC_MAXIMIZEPOWER])
#endif
					?1:0)|
					(flag.arrow?2:0)|
#ifndef RENEWAL
					(skill_id == HW_MAGICCRASHER?4:0)|
					(skill_id == MO_EXTREMITYFIST?8:0)|
#endif
					(!skill_id && sc && sc->data[SC_HLIF_CHANGE]?4:0)|
					(sc && sc->data[SC_WEAPONPERFECT]?8:0);
				if (flag.arrow && sd)
				switch(sd->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						break;
					default:
						i |= 16; // for ex. shuriken must not be influenced by DEX
				}
#ifdef RENEWAL
				GET_NORMAL_ATTACK( i, skill_id);
				wd.damage = battle->calc_masteryfix(src, target, skill_id, skill_lv, wd.damage, wd.div_, 0, flag.weapon);
				wd.damage = battle->calc_cardfix2(src, target, wd.damage, s_ele, nk, wd.flag);
				if (flag.lh){
					GET_NORMAL_ATTACK2( i, skill_id );
					wd.damage2 = battle->calc_masteryfix(src, target, skill_id, skill_lv, wd.damage2, wd.div_, 1, flag.weapon);
					wd.damage2 = battle->calc_cardfix2(src, target, wd.damage2, s_ele, nk, wd.flag);
				}
#else
				wd.damage = battle->calc_base_damage2(sstatus, &sstatus->rhw, sc, tstatus->size, sd, i);
				if (flag.lh)
					wd.damage2 = battle->calc_base_damage2(sstatus, &sstatus->lhw, sc, tstatus->size, sd, i);
#endif
				if (nk&NK_SPLASHSPLIT){ // Divide ATK among targets
					if(wflag>0)
						wd.damage/= wflag;
					else
						ShowError("0 enemies targeted by %d:%s, divide per 0 avoided!\n", skill_id, skill->get_name(skill_id));
				}

				//Add any bonuses that modify the base baseatk+watk (pre-skills)
				if(sd) {
#ifndef RENEWAL
					if (sd->bonus.atk_rate)
						ATK_ADDRATE(sd->bonus.atk_rate);
#endif
					if(flag.cri && sd->bonus.crit_atk_rate)
						ATK_ADDRATE(sd->bonus.crit_atk_rate);
					if(flag.cri && sc && sc->data[SC_MTF_CRIDAMAGE])
						ATK_ADDRATE(sc->data[SC_MTF_CRIDAMAGE]->val1);// temporary it should be 'bonus.crit_atk_rate'
#ifndef RENEWAL

					if(sd->status.party_id && (temp=pc->checkskill(sd,TK_POWER)) > 0){
						if( (i = party->foreachsamemap(party->sub_count, sd, 0)) > 1 ) // exclude the player himself [Inkfish]
							ATK_ADDRATE(2*temp*i);
					}
#endif
				}
				break;
			} //End default case
		} //End switch(skill_id)

		if( sc && skill_id != PA_SACRIFICE && sc->data[SC_UNLIMIT] && (wd.flag&(BF_LONG|BF_MAGIC)) == BF_LONG) {
			switch(skill_id) {
				case RA_WUGDASH:
				case RA_WUGSTRIKE:
				case RA_WUGBITE:
					break;
				default:
					ATK_ADDRATE( 50 * sc->data[SC_UNLIMIT]->val1 );
			}
		}

		if ( sc && !skill_id && sc->data[SC_EXEEDBREAK] ) {
			ATK_ADDRATE(sc->data[SC_EXEEDBREAK]->val1);
			status_change_end(src, SC_EXEEDBREAK, INVALID_TIMER);
		}

		switch(skill_id){
			case SR_GATEOFHELL:
				if (wd.dmg_lv != ATK_FLEE)
					ATK_RATE(battle->calc_skillratio(BF_WEAPON, src, target, skill_id, skill_lv, skillratio, wflag));
				else
					wd.dmg_lv = ATK_DEF;
				break;

			case KO_BAKURETSU:
			{
#ifdef RENEWAL
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), skill_id);
#endif
				skillratio = skill_lv * (50 + status_get_dex(src) / 4);
				skillratio = (int)(skillratio * (sd ? pc->checkskill(sd, NJ_TOBIDOUGU) : 10) * 40.f / 100.0f * status->get_lv(src) / 120);
				ATK_RATE(skillratio + 10 * (sd ? sd->status.job_level : 0));
			}
				break;

	#ifdef RENEWAL
			case GS_MAGICALBULLET:
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), skill_id);
				ATK_ADD(battle->attr_fix(src, target,
					battle->calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, status->get_matk(src, 2), 0, wd.flag), ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv));
				break;
			case GS_PIERCINGSHOT:
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), 0);
				if ( wd.damage ) {
					if ( sd && sd->weapontype1 == W_RIFLE )
						ATK_RATE(30 * (skill_lv + 5));
					else
						ATK_RATE(20 * (skill_lv + 5));
				}
				break;
			case MO_EXTREMITYFIST: // [malufett]
			{
				short totaldef = status->get_total_def(target);
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | 8, skill_id);
				if ( wd.damage ) {
					ATK_ADD(250 * (skill_lv + 1) + (10 * (status_get_sp(src) + 1) * wd.damage / 100) + (8 * wd.damage));
					ATK_ADD(-totaldef);
				}
			}
				break;
			case PA_SHIELDCHAIN:
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), skill_id);
				if ( sd ) {
					short index = sd->equip_index[EQI_HAND_L];
					if ( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR ) {
						ATK_ADD(sd->inventory_data[index]->weight / 10 + 4 * sd->status.inventory[index].refine);
					}
				} else
					ATK_ADD(sstatus->rhw.atk2); //Else use Atk2
				ATK_RATE(battle->calc_skillratio(BF_WEAPON, src, target, skill_id, skill_lv, skillratio, wflag));
				break;
			case AM_DEMONSTRATION:
			case AM_ACIDTERROR: // [malufett/Hercules]
			{
				int64 matk;
				int totaldef = status->get_total_def(target) + status->get_total_mdef(target);
				matk = battle->calc_cardfix(BF_MAGIC, src, target, nk, s_ele, 0, status->get_matk(src, 2), 0, wd.flag);
				matk = battle->attr_fix(src, target, matk, ELE_NEUTRAL, tstatus->def_ele, tstatus->ele_lv);
				matk = matk * battle->calc_skillratio(BF_WEAPON, src, target, skill_id, skill_lv, skillratio, wflag) / 100;
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), 0);
				ATK_RATE(battle->calc_skillratio(BF_WEAPON, src, target, skill_id, skill_lv, skillratio, wflag));
				ATK_ADD(matk);
				ATK_ADD(-totaldef);
				if ( skill_id == AM_ACIDTERROR && is_boss(target) )
					ATK_RATE(50);
				if ( skill_id == AM_DEMONSTRATION )
					wd.damage = max(wd.damage, 1);
			}
				break;
			case GN_CARTCANNON:
				GET_NORMAL_ATTACK((sc && sc->data[SC_MAXIMIZEPOWER] ? 1 : 0) | (sc && sc->data[SC_WEAPONPERFECT] ? 8 : 0), skill_id);
				ATK_ADD(sd ? sd->bonus.arrow_atk : 0);
				wd.damage = battle->calc_masteryfix(src, target, skill_id, skill_lv, wd.damage, wd.div_, 0, flag.weapon);
				ATK_RATE(battle->calc_skillratio(BF_WEAPON, src, target, skill_id, skill_lv, skillratio, wflag));
				if ( sd && s_ele != sd->bonus.arrow_ele )
					s_ele = sd->bonus.arrow_ele;
				break;
			case NJ_TATAMIGAESHI:
					ATK_RATE(200);
				/* Fall through */
			case LK_SPIRALPIERCE:
			case ML_SPIRALPIERCE: // [malufett]
				if( skill_id != NJ_TATAMIGAESHI ){
					short index = sd?sd->equip_index[EQI_HAND_R]:0;
					GET_NORMAL_ATTACK( (sc && sc->data[SC_MAXIMIZEPOWER]?1:0)|(sc && sc->data[SC_WEAPONPERFECT]?8:0), 0);
					wd.damage = wd.damage * 70 / 100;
					//n_ele = true; // FIXME: This is has no effect if it's after GET_NORMAL_ATTACK (was this intended, or was it supposed to be put above?)

					if (sd && index >= 0 &&
						sd->inventory_data[index] &&
						sd->inventory_data[index]->type == IT_WEAPON)
						ATK_ADD(sd->inventory_data[index]->weight * 7 / 100);

					switch (tstatus->size) {
						case SZ_SMALL: //Small: 115%
							ATK_RATE(115);
							break;
						case SZ_BIG: //Large: 85%
							ATK_RATE(85);
					}
					wd.damage = battle->calc_masteryfix(src, target, skill_id, skill_lv, wd.damage, wd.div_, 0, flag.weapon);
					wd.damage = battle->calc_cardfix2(src, target, wd.damage, s_ele, nk, wd.flag);
				}
				/* Fall through */
	#endif
			default:
				ATK_RATE(battle->calc_skillratio(BF_WEAPON, src, target, skill_id, skill_lv, skillratio, wflag));
		}

			//Constant/misc additions from skills
		switch (skill_id) {
#ifdef RENEWAL
			case HW_MAGICCRASHER:
				ATK_ADD(battle->calc_magic_attack(src, target, skill_id, skill_lv, wflag).damage / 5);
				break;
#else
			case MO_EXTREMITYFIST:
				ATK_ADD(250 + 150*skill_lv);
				break;
#endif
			case TK_DOWNKICK:
			case TK_STORMKICK:
			case TK_TURNKICK:
			case TK_COUNTER:
			case TK_JUMPKICK:
				//TK_RUN kick damage bonus.
				if(sd && sd->weapontype1 == W_FIST && sd->weapontype2 == W_FIST)
					ATK_ADD(10*pc->checkskill(sd, TK_RUN));
				break;

#ifndef RENEWAL
			case GS_MAGICALBULLET:
				ATK_ADD( status->get_matk(src, 2) );
				break;
			case NJ_SYURIKEN:
				ATK_ADD(4*skill_lv);
#endif
				break;
			case GC_COUNTERSLASH:
				ATK_ADD( status_get_agi(src) * 2 + (sd?sd->status.job_level:0) * 4 );
				break;
			case RA_WUGDASH:
				if( sc && sc->data[SC_DANCE_WITH_WUG] )
					ATK_ADD(2 * sc->data[SC_DANCE_WITH_WUG]->val1 * (2 + battle->calc_chorusbonus(sd)));
				break;
			case SR_TIGERCANNON:
				ATK_ADD( skill_lv * 240 + status->get_lv(target) * 40 );
				if( sc && sc->data[SC_COMBOATTACK]
					&& sc->data[SC_COMBOATTACK]->val1 == SR_FALLENEMPIRE )
						ATK_ADD( skill_lv * 500 + status->get_lv(target) * 40 );
				break;
			case RA_WUGSTRIKE:
			case RA_WUGBITE:
				if(sd)
					ATK_ADD(30*pc->checkskill(sd, RA_TOOTHOFWUG));
				if( sc && sc->data[SC_DANCE_WITH_WUG] )
					ATK_ADD(2 * sc->data[SC_DANCE_WITH_WUG]->val1 * (2 + battle->calc_chorusbonus(sd)));
				break;
			case LG_SHIELDPRESS:
				if( sd ) {
					int damagevalue = 0;
					short index = sd->equip_index[EQI_HAND_L];
					if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
						damagevalue = sstatus->vit * sd->status.inventory[index].refine;
					ATK_ADD(damagevalue);
				}
				break;
			case SR_GATEOFHELL:
				ATK_ADD(sstatus->max_hp - status_get_hp(src));
				if ( sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == SR_FALLENEMPIRE ) {
					ATK_ADD((sstatus->max_sp * (1 + skill_lv * 2 / 10)) + 40 * status->get_lv(src));
				} else {
					ATK_ADD((sstatus->sp * (1 + skill_lv * 2 / 10)) + 10 * status->get_lv(src));
				}
				break;
			case SR_FALLENEMPIRE:// [(Target Size value + Skill Level - 1) x Caster STR] + [(Target current weight x Caster DEX / 120)]
				ATK_ADD( ((tstatus->size+1)*2 + (int64)skill_lv - 1) * sstatus->str);
				if( tsd && tsd->weight ){
					ATK_ADD( (tsd->weight/10) * sstatus->dex / 120 );
				}else{
					ATK_ADD( status->get_lv(target) * 50 ); //mobs
				}
				break;
			case KO_SETSUDAN:
				if( tsc && tsc->data[SC_SOULLINK] ){
					ATK_ADDRATE(200*tsc->data[SC_SOULLINK]->val1);
					status_change_end(target,SC_SOULLINK,INVALID_TIMER);
				}
				break;
			case KO_MAKIBISHI:
				wd.damage = 20 * skill_lv;
				break;
		}
#ifndef RENEWAL
		//Div fix.
		damage_div_fix(wd.damage, wd.div_);
#endif
		//The following are applied on top of current damage and are stackable.
		if ( sc ) {
#ifndef RENEWAL
			if( sc->data[SC_TRUESIGHT] )
				ATK_ADDRATE(2*sc->data[SC_TRUESIGHT]->val1);
#endif
#ifndef RENEWAL_EDP
			if( sc->data[SC_EDP] ){
				switch(skill_id){
					case AS_SPLASHER: // Needs more info
					case ASC_BREAKER:
					case ASC_METEORASSAULT: break;
					default:
						ATK_ADDRATE(sc->data[SC_EDP]->val3);
				}
			}
#endif
			if(sc->data[SC_STYLE_CHANGE]){
				struct homun_data *hd = BL_CAST(BL_HOM, src);
				if (hd != NULL)
					ATK_ADD(hd->homunculus.spiritball * 3);
			}
		}

		switch (skill_id) {
			case AS_SONICBLOW:
				if (sc && sc->data[SC_SOULLINK] &&
					sc->data[SC_SOULLINK]->val2 == SL_ASSASIN)
					ATK_ADDRATE(map_flag_gvg(src->m)?25:100); //+25% dmg on woe/+100% dmg on nonwoe

				if(sd && pc->checkskill(sd,AS_SONICACCEL)>0)
					ATK_ADDRATE(10);
			break;
			case CR_SHIELDBOOMERANG:
				if(sc && sc->data[SC_SOULLINK] &&
					sc->data[SC_SOULLINK]->val2 == SL_CRUSADER)
					ATK_ADDRATE(100);
				break;
		}
		if( skill_id ){
			uint16 rskill;/* redirect skill id */
			switch(skill_id){
				case AB_DUPLELIGHT_MELEE:
					rskill = AB_DUPLELIGHT;
					break;
				case LG_OVERBRAND_BRANDISH:
				case LG_OVERBRAND_PLUSATK:
					rskill = LG_OVERBRAND;
					break;
				case WM_SEVERE_RAINSTORM_MELEE:
					rskill = WM_SEVERE_RAINSTORM;
					break;
				case WM_REVERBERATION_MELEE:
					rskill = WM_REVERBERATION;
					break;
				case GN_CRAZYWEED_ATK:
					rskill = GN_CRAZYWEED;
					break;
				case GN_SLINGITEM_RANGEMELEEATK:
					rskill = GN_SLINGITEM;
					break;
				case RL_R_TRIP_PLUSATK:
					rskill = RL_R_TRIP;
					break;
				case RL_B_FLICKER_ATK:
					rskill = RL_FLICKER;
					break;
				case RL_GLITTERING_GREED_ATK:
					rskill = RL_GLITTERING_GREED;
					break;
				default:
					rskill = skill_id;
			}
			if( (i = battle->adjust_skill_damage(src->m,rskill)) )
				ATK_RATE(i);
		}

		if( sd ) {
			if (skill_id && (i = pc->skillatk_bonus(sd, skill_id)))
				ATK_ADDRATE(i);
	#ifdef RENEWAL
			if( wd.flag&BF_LONG )
				ATK_ADDRATE(sd->bonus.long_attack_atk_rate);
			if( sc && sc->data[SC_MTF_RANGEATK] )
				ATK_ADDRATE(sc->data[SC_MTF_RANGEATK]->val1);// temporary it should be 'bonus.long_attack_atk_rate'
	#endif
			if( (i=pc->checkskill(sd,AB_EUCHARISTICA)) > 0 &&
				(tstatus->race == RC_DEMON || tstatus->def_ele == ELE_DARK) )
				ATK_ADDRATE(-i);
			if (skill_id != PA_SACRIFICE && skill_id != MO_INVESTIGATE && skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS && skill_id != PA_SHIELDCHAIN && !flag.cri) {
				//Elemental/Racial adjustments
				if (sd->right_weapon.def_ratio_atk_ele & (1<<tstatus->def_ele)
				 || sd->right_weapon.def_ratio_atk_race & map->race_id2mask(tstatus->race)
				 || sd->right_weapon.def_ratio_atk_race & map->race_id2mask(is_boss(target) ? RC_BOSS : RC_NONBOSS)
				)
					flag.pdef = 1;

				if (sd->left_weapon.def_ratio_atk_ele & (1<<tstatus->def_ele)
				 || sd->left_weapon.def_ratio_atk_race & map->race_id2mask(tstatus->race)
				 || sd->left_weapon.def_ratio_atk_race & map->race_id2mask(is_boss(target) ? RC_BOSS : RC_NONBOSS)
				) {
					//Pass effect onto right hand if configured so. [Skotlex]
					if (battle_config.left_cardfix_to_right && flag.rh)
						flag.pdef = 1;
					else
						flag.pdef2 = 1;
				}
			}

			if (skill_id != CR_GRANDCROSS && skill_id != NPC_GRANDDARKNESS) {
				//Ignore Defense?
				if (!flag.idef && (
					sd->right_weapon.ignore_def_ele & (1<<tstatus->def_ele) ||
					sd->right_weapon.ignore_def_race & map->race_id2mask(tstatus->race) ||
					sd->right_weapon.ignore_def_race & map->race_id2mask(is_boss(target) ? RC_BOSS : RC_NONBOSS)
				))
					flag.idef = 1;

				if (!flag.idef2 && (
					sd->left_weapon.ignore_def_ele & (1<<tstatus->def_ele) ||
					sd->left_weapon.ignore_def_race & map->race_id2mask(tstatus->race) ||
					sd->left_weapon.ignore_def_race & map->race_id2mask(is_boss(target) ? RC_BOSS : RC_NONBOSS)
				)) {
						if(battle_config.left_cardfix_to_right && flag.rh) //Move effect to right hand. [Skotlex]
							flag.idef = 1;
						else
							flag.idef2 = 1;
				}
			}
		}

		if((!flag.idef || !flag.idef2)
#ifdef RENEWAL
			&& (!flag.distinct || flag.tdef)
#endif
			) { //Defense reduction
			wd.damage = battle->calc_defense(BF_WEAPON, src, target, skill_id, skill_lv, wd.damage,
											 (flag.idef?1:0)|(flag.pdef?2:0)
#ifdef RENEWAL
											 |(flag.tdef?4:0)
#endif
											 , flag.pdef);
			if( wd.damage2 )
				wd.damage2 = battle->calc_defense(BF_WEAPON, src, target, skill_id, skill_lv, wd.damage2,
												  (flag.idef2?1:0)|(flag.pdef2?2:0)
#ifdef RENEWAL
												  |(flag.tdef?4:0)
#endif
												  , flag.pdef2);
		}

#ifdef RENEWAL
		if ( flag.distinct ) {
			wd.damage = battle->calc_cardfix2(src, target, wd.damage, s_ele, nk, wd.flag);
			if ( flag.lh ) {
				wd.damage2 = battle->calc_cardfix2(src, target, wd.damage2, s_ele, nk, wd.flag);
			}
		}
		//Div fix.
		damage_div_fix(wd.damage, wd.div_);
		if ( skill_id > 0 && (skill->get_ele(skill_id, skill_lv) == ELE_NEUTRAL || flag.distinct) ) { // re-evaluate forced neutral skills
			wd.damage = battle->attr_fix(src, target, wd.damage, s_ele, tstatus->def_ele, tstatus->ele_lv);
			if ( flag.lh )
				wd.damage2 = battle->attr_fix(src, target, wd.damage2, s_ele_, tstatus->def_ele, tstatus->ele_lv);
		}
#endif
#if 0 // Can't find any source about this one even in eagis
		if (skill_id == NPC_EARTHQUAKE) {
			//Adds atk2 to the damage, should be influenced by number of hits and skill-ratio, but not mdef reductions. [Skotlex]
			//Also divide the extra bonuses from atk2 based on the number in range [Kevin]
			if ( wflag>0 )
				ATK_ADD((sstatus->rhw.atk2*skillratio / 100) / wflag);
			else
				ShowError("Zero range by %d:%s, divide per 0 avoided!\n", skill_id, skill->get_name(skill_id));
		}
#endif
		//Post skill/vit reduction damage increases
		if (sc) {
			//SC skill damages
			if(sc->data[SC_AURABLADE]
#ifndef RENEWAL
					&& skill_id != LK_SPIRALPIERCE && skill_id != ML_SPIRALPIERCE
#endif
			){
				int lv = sc->data[SC_AURABLADE]->val1;
#ifdef RENEWAL
				lv *= ((skill_id == LK_SPIRALPIERCE || skill_id == ML_SPIRALPIERCE)?wd.div_:1); // +100 per hit in lv 5
#endif
				ATK_ADD(20*lv);
			}

			if( !skill_id ) {
				if( sc->data[SC_ENCHANTBLADE] ) {
					//[( ( Skill Lv x 20 ) + 100 ) x ( casterBaseLevel / 150 )] + casterInt
					i = ( sc->data[SC_ENCHANTBLADE]->val1 * 20 + 100 ) * status->get_lv(src) / 150 + status_get_int(src);
					i = i - status->get_total_mdef(target) + status->get_matk(src, 2);
					if( i )
						ATK_ADD(i);
				}
				if( sc->data[SC_GIANTGROWTH] && rnd()%100 < 15 )
					ATK_ADDRATE(200); // Triple Damage
			}
		}
#ifndef RENEWAL
		//Refine bonus
		if( sd && flag.weapon && skill_id != MO_INVESTIGATE && skill_id != MO_EXTREMITYFIST )
		{ // Counts refine bonus multiple times
			if( skill_id == MO_FINGEROFFENSIVE )
			{
				ATK_ADD2(wd.div_*sstatus->rhw.atk2, wd.div_*sstatus->lhw.atk2);
			} else {
				ATK_ADD2(sstatus->rhw.atk2, sstatus->lhw.atk2);
			}
		}
		//Set to min of 1
		if (flag.rh && wd.damage < 1) wd.damage = 1;
		if (flag.lh && wd.damage2 < 1) wd.damage2 = 1;
#else
		if (flag.rh && wd.damage < 1) wd.damage = 0;
		if (flag.lh && wd.damage2 < 1) wd.damage2 = 0;
#endif

#ifndef RENEWAL
		wd.damage = battle->calc_masteryfix(src, target, skill_id, skill_lv, wd.damage, wd.div_, 0, flag.weapon);
		if( flag.lh )
			wd.damage2 = battle->calc_masteryfix(src, target, skill_id, skill_lv, wd.damage2, wd.div_, 1, flag.weapon);
#else
		if( sd && flag.cri )
			ATK_ADDRATE(40);
#endif
	} //Here ends flag.hit section, the rest of the function applies to both hitting and missing attacks
	else if(wd.div_ < 0) //Since the attack missed...
		wd.div_ *= -1;
#ifndef RENEWAL
	if(sd && (temp=pc->checkskill(sd,BS_WEAPONRESEARCH)) > 0)
		ATK_ADD(temp*2);
#endif

#ifndef RENEWAL
	wd.damage = battle->calc_elefix(src, target, skill_id, skill_lv, wd.damage, nk, n_ele, s_ele, s_ele_, false, flag.arrow);
	if( flag.lh )
		wd.damage2 = battle->calc_elefix(src, target, skill_id, skill_lv, wd.damage2, nk, n_ele, s_ele, s_ele_, true, flag.arrow);
#endif

	if(skill_id == CR_GRANDCROSS || skill_id == NPC_GRANDDARKNESS)
		return wd; //Enough, rest is not needed.
#ifndef HMAP_ZONE_DAMAGE_CAP_TYPE
	if (skill_id) {
		for(i = 0; i < map->list[target->m].zone->capped_skills_count; i++) {
			if( skill_id == map->list[target->m].zone->capped_skills[i]->nameid && (map->list[target->m].zone->capped_skills[i]->type & target->type) ) {
				if (target->type == BL_MOB && map->list[target->m].zone->capped_skills[i]->subtype != MZS_NONE) {
					const struct mob_data *md = BL_UCCAST(BL_MOB, target);
					if ((md->status.mode&MD_BOSS) && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_BOSS))
						continue;
					if (md->special_state.clone && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_CLONE))
						continue;
				}
				if( wd.damage > map->list[target->m].zone->capped_skills[i]->cap )
					wd.damage = map->list[target->m].zone->capped_skills[i]->cap;
				if( wd.damage2 > map->list[target->m].zone->capped_skills[i]->cap )
					wd.damage2 = map->list[target->m].zone->capped_skills[i]->cap;
				break;
			}
		}
	}
#endif

#ifndef RENEWAL // Offensive damage increment in renewal is done somewhere else
	if (sd) {
		if (skill_id != CR_SHIELDBOOMERANG) //Only Shield boomerang doesn't takes the Star Crumbs bonus.
			ATK_ADD2(wd.div_*sd->right_weapon.star, wd.div_*sd->left_weapon.star);
		if (skill_id==MO_FINGEROFFENSIVE) { //The finger offensive spheres on moment of attack do count. [Skotlex]
			ATK_ADD(wd.div_*sd->spiritball_old*3);
		} else {
			ATK_ADD(wd.div_*sd->spiritball*3);
		}
		//Card Fix, sd side

		wd.damage = battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage, 2, wd.flag);
		if( flag.lh )
			wd.damage2 = battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage2, 3, wd.flag);

		if( skill_id == CR_SHIELDBOOMERANG || skill_id == PA_SHIELDCHAIN )
		{ //Refine bonus applies after cards and elements.
			short index= sd->equip_index[EQI_HAND_L];
			if( index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR )
				ATK_ADD(10*sd->status.inventory[index].refine);
		}
	}
	//Card Fix, tsd side
	if ( tsd ) { //if player on player then it was already measured above
		wd.damage = battle->calc_cardfix(BF_WEAPON, src, target, nk, s_ele, s_ele_, wd.damage, (flag.lh ? 1 : 0), wd.flag);
	}
#endif

	if( flag.infdef ) { //Plants receive 1 damage when hit
		short class_ = status->get_class(target);
		if( flag.hit || wd.damage > 0 )
			wd.damage = wd.div_; // In some cases, right hand no need to have a weapon to increase damage
		if( flag.lh && (flag.hit || wd.damage2 > 0) )
			wd.damage2 = wd.div_;
		if (flag.hit && class_ == MOBID_EMPELIUM) {
			if(wd.damage2 > 0) {
				wd.damage2 = battle->attr_fix(src,target,wd.damage2,s_ele_,tstatus->def_ele, tstatus->ele_lv);
				wd.damage2 = battle->calc_gvg_damage(src,target,wd.damage2,wd.div_,skill_id,skill_lv,wd.flag);
			}
			else if(wd.damage > 0) {
				wd.damage = battle->attr_fix(src,target,wd.damage,s_ele_,tstatus->def_ele, tstatus->ele_lv);
				wd.damage = battle->calc_gvg_damage(src,target,wd.damage,wd.div_,skill_id,skill_lv,wd.flag);
			}
			return wd;
		}
		if( !(battle_config.skill_min_damage&1) )
			//Do not return if you are supposed to deal greater damage to plants than 1. [Skotlex]
			return wd;
	}

	if (sd) {
		if (!flag.rh && flag.lh) {
			//Move lh damage to the rh
			wd.damage = wd.damage2;
			wd.damage2 = 0;
			flag.rh=1;
			flag.lh=0;
		} else if(flag.rh && flag.lh) {
			//Dual-wield
			if (wd.damage) {
				temp = pc->checkskill(sd,AS_RIGHT) * 10;
				if( (sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO  )
					temp = pc->checkskill(sd,KO_RIGHT) * 10 + 20;
				ATK_RATER( 50 + temp );
			}
			if (wd.damage2) {
				temp = pc->checkskill(sd,AS_LEFT) * 10;
				if( (sd->class_&MAPID_UPPERMASK) == MAPID_KAGEROUOBORO  )
					temp = pc->checkskill(sd,KO_LEFT) * 10 + 20;
				ATK_RATEL( 30 + temp );
			}
#ifdef RENEWAL
			if(wd.damage < 0) wd.damage = 0;
			if(wd.damage2 < 0) wd.damage2 = 0;
#else
			if(wd.damage < 1) wd.damage = 1;
			if(wd.damage2 < 1) wd.damage2 = 1;
#endif
		} else if(sd->status.weapon == W_KATAR && !skill_id) { //Katars (offhand damage only applies to normal attacks, tested on Aegis 10.2)
			temp = pc->checkskill(sd,TF_DOUBLE);
			wd.damage2 = wd.damage * (1 + (temp * 2))/100;

			if(wd.damage && !wd.damage2) {
#ifdef RENEWAL
				wd.damage2 = 0;
#else
				wd.damage2 = 1;
#endif
			}
			flag.lh = 1;
		}
	}

	if(!flag.rh && wd.damage)
		wd.damage=0;

	if(!flag.lh && wd.damage2)
		wd.damage2=0;

	if( sc && sc->data[SC_GLOOMYDAY] ) {
		switch( skill_id ) {
			case KN_BRANDISHSPEAR:
			case LK_SPIRALPIERCE:
			case CR_SHIELDCHARGE:
			case CR_SHIELDBOOMERANG:
			case PA_SHIELDCHAIN:
			case RK_HUNDREDSPEAR:
			case LG_SHIELDPRESS:
				wd.damage += wd.damage * sc->data[SC_GLOOMYDAY]->val2 / 100;
		}
	}

	if( sc ) {
		//SG_FUSION hp penalty [Komurka]
		if (sc->data[SC_FUSION]) {
			int hp= sstatus->max_hp;
			if (sd && tsd) {
				hp = 8*hp/100;
				if ((sstatus->hp * 100) <= (sstatus->max_hp * 20))
					hp = sstatus->hp;
			} else
				hp = 2*hp/100; //2% hp loss per hit
			status_zap(src, hp, 0);
		}
		status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);
	}

	switch(skill_id){
		case LG_RAYOFGENESIS:
			{
			struct Damage md = battle->calc_magic_attack(src, target, skill_id, skill_lv, wflag);
			wd.damage += md.damage;
			break;
			}
	}

	if( wd.damage + wd.damage2 ) { //There is a total damage value
		int64 damage = wd.damage + wd.damage2;

		if (!wd.damage2) {
#ifdef RENEWAL
			if (skill_id != ASC_BREAKER)
#endif
				wd.damage = battle->calc_damage(src, target, &wd, wd.damage, skill_id, skill_lv);
			if( map_flag_gvg2(target->m) )
				wd.damage=battle->calc_gvg_damage(src,target,wd.damage,wd.div_,skill_id,skill_lv,wd.flag);
			else if( map->list[target->m].flag.battleground )
				wd.damage=battle->calc_bg_damage(src,target,wd.damage,wd.div_,skill_id,skill_lv,wd.flag);
		}
		else if (!wd.damage) {
#ifdef RENEWAL
			if (skill_id != ASC_BREAKER)
#endif
				wd.damage2 = battle->calc_damage(src, target, &wd, wd.damage2, skill_id, skill_lv);
			if (map_flag_gvg2(target->m))
				wd.damage2 = battle->calc_gvg_damage(src, target, wd.damage2, wd.div_, skill_id, skill_lv, wd.flag);
			else if (map->list[target->m].flag.battleground)
				wd.damage = battle->calc_bg_damage(src, target, wd.damage2, wd.div_, skill_id, skill_lv, wd.flag);
		} else {
#ifdef RENEWAL
			if( skill_id != ASC_BREAKER ){
				wd.damage = battle->calc_damage(src, target, &wd, wd.damage, skill_id, skill_lv);
				wd.damage2 = battle->calc_damage(src, target, &wd, wd.damage2, skill_id, skill_lv);
			}
#else
			int64 d1 = wd.damage + wd.damage2,d2 = wd.damage2;
			wd.damage = battle->calc_damage(src,target,&wd,d1,skill_id,skill_lv);
#endif
			if( map_flag_gvg2(target->m) )
				wd.damage = battle->calc_gvg_damage(src,target,wd.damage,wd.div_,skill_id,skill_lv,wd.flag);
			else if( map->list[target->m].flag.battleground )
				wd.damage = battle->calc_bg_damage(src,target,wd.damage,wd.div_,skill_id,skill_lv,wd.flag);
#ifndef RENEWAL
			wd.damage2 = d2*100/d1 * wd.damage/100;
			if(wd.damage > 1 && wd.damage2 < 1) wd.damage2 = 1;
			wd.damage-=wd.damage2;
#endif
		}

		if( src != target ) { // Don't reflect your own damage (Grand Cross)
			if( wd.dmg_lv == ATK_MISS || wd.dmg_lv == ATK_BLOCK ) {
				int64 prev1 = wd.damage, prev2 = wd.damage2;

				wd.damage = damage;
				wd.damage2 = 0;

				battle->reflect_damage(target, src, &wd, skill_id);

				wd.damage = prev1;
				wd.damage2 = prev2;

			} else
				battle->reflect_damage(target, src, &wd, skill_id);
		}
	}
	//Reject Sword bugreport:4493 by Daegaladh
	if (wd.damage != 0 && tsc != NULL && tsc->data[SC_SWORDREJECT] != NULL
	 && (sd == NULL || sd->weapontype1 == W_DAGGER || sd->weapontype1 == W_1HSWORD || sd->status.weapon == W_2HSWORD)
	 && rnd()%100 < tsc->data[SC_SWORDREJECT]->val2
	) {
		ATK_RATER(50);
		status_fix_damage(target,src,wd.damage,clif->damage(target,src,0,0,wd.damage,0,BDT_NORMAL,0));
		clif->skill_nodamage(target,target,ST_REJECTSWORD,tsc->data[SC_SWORDREJECT]->val1,1);
		if( --(tsc->data[SC_SWORDREJECT]->val3) <= 0 )
			status_change_end(target, SC_SWORDREJECT, INVALID_TIMER);
	}
#ifndef RENEWAL
	if(skill_id == ASC_BREAKER) {
		//Breaker's int-based damage (a misc attack?)
		struct Damage md = battle->calc_misc_attack(src, target, skill_id, skill_lv, wflag);
		wd.damage += md.damage;
	}
#endif

	return wd;
}

/*==========================================
 * Battle main entry, from skill->attack
 *------------------------------------------*/
struct Damage battle_calc_attack(int attack_type,struct block_list *bl,struct block_list *target,uint16 skill_id,uint16 skill_lv,int count)
{
	struct Damage d;
	struct map_session_data *sd=BL_CAST(BL_PC,bl);
	switch(attack_type) {
		case BF_WEAPON: d = battle->calc_weapon_attack(bl,target,skill_id,skill_lv,count); break;
		case BF_MAGIC:  d = battle->calc_magic_attack(bl,target,skill_id,skill_lv,count);  break;
		case BF_MISC:   d = battle->calc_misc_attack(bl,target,skill_id,skill_lv,count);   break;
	default:
		ShowError("battle_calc_attack: unknown attack type! %d\n",attack_type);
		memset(&d,0,sizeof(d));
		break;
	}

	nullpo_retr(d, target);
#ifdef HMAP_ZONE_DAMAGE_CAP_TYPE
	if( target && skill_id ) {
		int i;
		for(i = 0; i < map->list[target->m].zone->capped_skills_count; i++) {
			if( skill_id == map->list[target->m].zone->capped_skills[i]->nameid && (map->list[target->m].zone->capped_skills[i]->type & target->type) ) {
				if (target->type == BL_MOB && map->list[target->m].zone->capped_skills[i]->subtype != MZS_NONE) {
					const struct mob_data *md = BL_UCCAST(BL_MOB, target);
					if ((md->status.mode&MD_BOSS) && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_BOSS))
						continue;
					if (md->special_state.clone && !(map->list[target->m].zone->disabled_skills[i]->subtype&MZS_CLONE))
						continue;
				}
				if( d.damage > map->list[target->m].zone->capped_skills[i]->cap )
					d.damage = map->list[target->m].zone->capped_skills[i]->cap;
				if( d.damage2 > map->list[target->m].zone->capped_skills[i]->cap )
					d.damage2 = map->list[target->m].zone->capped_skills[i]->cap;
				break;
			}
		}
	}
#endif

	if( d.damage + d.damage2 < 1 ) { //Miss/Absorbed
		//Weapon attacks should go through to cause additional effects.
		if (d.dmg_lv == ATK_DEF /*&& attack_type&(BF_MAGIC|BF_MISC)*/) // Isn't it that additional effects don't apply if miss?
			d.dmg_lv = ATK_MISS;
		d.dmotion = 0;
	} else // Some skills like Weaponry Research will cause damage even if attack is dodged
		d.dmg_lv = ATK_DEF;

	if (sd && d.damage + d.damage2 > 1) {
		// HPVanishRate
		if (sd->bonus.hp_vanish_rate && sd->bonus.hp_vanish_trigger && rnd() % 1000 < sd->bonus.hp_vanish_rate &&
			((d.flag&sd->bonus.hp_vanish_trigger&BF_WEAPONMASK) || (d.flag&sd->bonus.hp_vanish_trigger&BF_RANGEMASK)
			|| (d.flag&sd->bonus.hp_vanish_trigger&BF_SKILLMASK)))
			status_percent_damage(&sd->bl, target, -sd->bonus.hp_vanish_per, 0, false);

		// SPVanishRate
		if (sd->bonus.sp_vanish_rate && sd->bonus.sp_vanish_trigger && rnd() % 1000 < sd->bonus.sp_vanish_rate &&
			((d.flag&sd->bonus.sp_vanish_trigger&BF_WEAPONMASK) || (d.flag&sd->bonus.sp_vanish_trigger&BF_RANGEMASK)
			|| (d.flag&sd->bonus.sp_vanish_trigger&BF_SKILLMASK)))
			status_percent_damage(&sd->bl, target, 0, -sd->bonus.sp_vanish_per, false);
	}
	return d;
}

//Performs reflect damage (magic (maya) is performed over skill.c).
void battle_reflect_damage(struct block_list *target, struct block_list *src, struct Damage *wd,uint16 skill_id) {
	int64 damage, rdamage = 0, trdamage = 0;
	struct map_session_data *sd, *tsd;
	struct status_change *sc;
	int64 tick = timer->gettick();
	int delay = 50, rdelay = 0;
#ifdef RENEWAL
	int max_reflect_damage;

	max_reflect_damage = max(status_get_max_hp(target), status_get_max_hp(target) * status->get_lv(target) / 100);
#endif

	damage = wd->damage + wd->damage2;

	nullpo_retv(wd);

	sd = BL_CAST(BL_PC, src);

	tsd = BL_CAST(BL_PC, target);
	sc = status->get_sc(target);

#ifdef RENEWAL
#define NORMALIZE_RDAMAGE(d) ( trdamage += rdamage = max(1, min(max_reflect_damage, (d))) )
#else
#define NORMALIZE_RDAMAGE(d) ( trdamage += rdamage = max(1, (d)) )
#endif

	if( sc && !sc->count )
		sc = NULL;

	if( sc ) {
		if (wd->flag & BF_SHORT && !(skill->get_inf(skill_id) & (INF_GROUND_SKILL | INF_SELF_SKILL))) {
			if( sc->data[SC_CRESCENTELBOW] && !is_boss(src) && rnd()%100 < sc->data[SC_CRESCENTELBOW]->val2 ){
				//ATK [{(Target HP / 100) x Skill Level} x Caster Base Level / 125] % + [Received damage x {1 + (Skill Level x 0.2)}]
				int ratio = (status_get_hp(src) / 100) * sc->data[SC_CRESCENTELBOW]->val1 * status->get_lv(target) / 125;
				if (ratio > 5000) ratio = 5000; // Maximum of 5000% ATK
				rdamage = ratio + (damage)* (10 + sc->data[SC_CRESCENTELBOW]->val1 * 20 / 10) / 10;
				skill->blown(target, src, skill->get_blewcount(SR_CRESCENTELBOW_AUTOSPELL, sc->data[SC_CRESCENTELBOW]->val1), unit->getdir(src), 0);
				clif->skill_damage(target, src, tick, status_get_amotion(src), 0, rdamage,
						   1, SR_CRESCENTELBOW_AUTOSPELL, sc->data[SC_CRESCENTELBOW]->val1, BDT_SKILL); // This is how official does
				clif->delay_damage(tick + delay, src, target,status_get_amotion(src)+1000,0, rdamage/10, 1, BDT_NORMAL);
				status->damage(src, target, status->damage(target, src, rdamage, 0, 0, 1)/10, 0, 0, 1);
				status_change_end(target, SC_CRESCENTELBOW, INVALID_TIMER);
				/* shouldn't this trigger skill->additional_effect? */
				return; // Just put here to minimize redundancy
			}
		}

		if( wd->flag & BF_SHORT ) {
			if( !is_boss(src) ) {
				if( sc->data[SC_DEATHBOUND] && skill_id != WS_CARTTERMINATION ) {
					uint8 dir = map->calc_dir(target,src->x,src->y),
					t_dir = unit->getdir(target);

					if( !map->check_dir(dir,t_dir) ) {
						int64 rd1 = damage * sc->data[SC_DEATHBOUND]->val2 / 100; // Amplify damage.

						trdamage += rdamage = rd1 - (damage = rd1 * 30 / 100); // not normalized as intended.
						rdelay = clif->skill_damage(src, target, tick, status_get_amotion(src), status_get_dmotion(src), -3000, 1, RK_DEATHBOUND, sc->data[SC_DEATHBOUND]->val1, BDT_SKILL);
						skill->blown(target, src, skill->get_blewcount(RK_DEATHBOUND, sc->data[SC_DEATHBOUND]->val1), unit->getdir(src), 0);

						if( tsd ) /* is this right? rdamage as both left and right? */
							battle->drain(tsd, src, rdamage, rdamage, status_get_race(src), 0);
						battle->delay_damage(tick, wd->amotion,target,src,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,rdelay,true);

						delay += 100;/* gradual increase so the numbers don't clip in the client */
					}
					wd->damage = wd->damage + wd->damage2;
					wd->damage2 = 0;
					status_change_end(target,SC_DEATHBOUND,INVALID_TIMER);
				}
			}
		}

		if( sc->data[SC_KYOMU] ){
			// Nullify reflecting ability of the conditions onwards
			return;
		}

	}

	if( wd->flag & BF_SHORT ) {
		if ( tsd && tsd->bonus.short_weapon_damage_return ) {
			NORMALIZE_RDAMAGE(damage * tsd->bonus.short_weapon_damage_return / 100);

			rdelay = clif->delay_damage(tick+delay,src, src, status_get_amotion(src), status_get_dmotion(src), rdamage, 1, BDT_ENDURE);

			/* is this right? rdamage as both left and right? */
			battle->drain(tsd, src, rdamage, rdamage, status_get_race(src), 0);
			battle->delay_damage(tick, wd->amotion,target,src,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,rdelay,true);

			delay += 100;/* gradual increase so the numbers don't clip in the client */
		}

		if( wd->dmg_lv >= ATK_BLOCK ) {/* yes block still applies, somehow gravity thinks it makes sense. */
			struct status_change *ssc;
			if( sc ) {
				struct status_change_entry *sce_d = sc->data[SC_DEVOTION];
				struct block_list *d_bl = NULL;

				if (sce_d && sce_d->val1)
					d_bl = map->id2bl(sce_d->val1);

				if( sc->data[SC_REFLECTSHIELD] && skill_id != WS_CARTTERMINATION && skill_id != GS_DESPERADO
				  && !(d_bl && !(wd->flag&BF_SKILL)) // It should not be a basic attack if the target is under devotion
				  && !(d_bl && sce_d && !check_distance_bl(target, d_bl, sce_d->val3)) // It should not be out of range if the target is under devotion
				) {

					NORMALIZE_RDAMAGE(damage * sc->data[SC_REFLECTSHIELD]->val2 / 100);
#ifndef RENEWAL
					rdelay = clif->delay_damage(tick+delay,src, src, status_get_amotion(src), status_get_dmotion(src), rdamage, 1, BDT_ENDURE);
#else
					rdelay = clif->skill_damage(src, src, tick, delay, status_get_dmotion(src), rdamage, 1, CR_REFLECTSHIELD, 1, BDT_ENDURE);
#endif
					/* is this right? rdamage as both left and right? */
					if( tsd )
						battle->drain(tsd, src, rdamage, rdamage, status_get_race(src), 0);
					battle->delay_damage(tick, wd->amotion,target,src,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,rdelay,true);

					delay += 100;/* gradual increase so the numbers don't clip in the client */
				}
				if( sc->data[SC_LG_REFLECTDAMAGE] && rnd()%100 < (30 + 10*sc->data[SC_LG_REFLECTDAMAGE]->val1) ) {
					bool change = false;

					NORMALIZE_RDAMAGE(damage * sc->data[SC_LG_REFLECTDAMAGE]->val2 / 100);

					trdamage -= rdamage;/* wont count towards total */

					if( sd && !sd->state.autocast ) {
						change = true;
						sd->state.autocast = 1;
					}

					map->foreachinshootrange(battle->damage_area,target,skill->get_splash(LG_REFLECTDAMAGE,1),BL_CHAR,tick,target,delay,wd->dmotion,rdamage,status_get_race(target));

					if( change )
						sd->state.autocast = 0;

					delay += 150;/* gradual increase so the numbers don't clip in the client */

					if( (--sc->data[SC_LG_REFLECTDAMAGE]->val3) <= 0 )
						status_change_end(target, SC_LG_REFLECTDAMAGE, INVALID_TIMER);
				}
				if( sc->data[SC_SHIELDSPELL_DEF] && sc->data[SC_SHIELDSPELL_DEF]->val1 == 2 ){
					NORMALIZE_RDAMAGE(damage * sc->data[SC_SHIELDSPELL_DEF]->val2 / 100);

					rdelay = clif->delay_damage(tick+delay,src, src, status_get_amotion(src), status_get_dmotion(src), rdamage, 1, BDT_ENDURE);

					/* is this right? rdamage as both left and right? */
					if( tsd )
						battle->drain(tsd, src, rdamage, rdamage, status_get_race(src), 0);
					battle->delay_damage(tick, wd->amotion,target,src,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,rdelay,true);

					delay += 100;/* gradual increase so the numbers don't clip in the client */
				}
				if (sc->data[SC_MVPCARD_ORCLORD]) {
					NORMALIZE_RDAMAGE(damage * sc->data[SC_MVPCARD_ORCLORD]->val1 / 100);

					rdelay = clif->delay_damage(tick + delay, src, src, status_get_amotion(src), status_get_dmotion(src), rdamage, 1, BDT_ENDURE);

					if (tsd)
						battle->drain(tsd, src, rdamage, rdamage, status_get_race(src), 0);
					battle->delay_damage(tick, wd->amotion, target, src, 0, CR_REFLECTSHIELD, 0, rdamage, ATK_DEF, rdelay, true);

					delay += 100;
				}
			}
			if( ( ssc = status->get_sc(src) ) ) {
				if( ssc->data[SC_INSPIRATION] ) {
					NORMALIZE_RDAMAGE(damage / 100);

					rdelay = clif->delay_damage(tick+delay,target, target, status_get_amotion(target), status_get_dmotion(target), rdamage, 1, BDT_ENDURE);

					/* is this right? rdamage as both left and right? */
					if( sd )
						battle->drain(sd, target, rdamage, rdamage, status_get_race(target), 0);
					battle->delay_damage(tick, wd->amotion,src,target,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,rdelay,true);

					delay += 100;/* gradual increase so the numbers don't clip in the client */
				}
			}
		}
	} else {/* long */
		if ( tsd && tsd->bonus.long_weapon_damage_return ) {
			NORMALIZE_RDAMAGE(damage * tsd->bonus.long_weapon_damage_return / 100);

			rdelay = clif->delay_damage(tick+delay,src, src, status_get_amotion(src), status_get_dmotion(src), rdamage, 1, BDT_ENDURE);

			/* is this right? rdamage as both left and right? */
			battle->drain(tsd, src, rdamage, rdamage, status_get_race(src), 0);
			battle->delay_damage(tick, wd->amotion,target,src,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,rdelay,true);

			delay += 100;/* gradual increase so the numbers don't clip in the client */
		}
	}

	// Tell analyzers/compilers that we want to += it even the value is currently unused (it'd be used if we added new checks)
	(void)delay;

	/* something caused reflect */
	if( trdamage ) {
		skill->additional_effect(target, src, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL,ATK_DEF,tick);
	}

	return;
#undef NORMALIZE_RDAMAGE
}

void battle_drain(struct map_session_data *sd, struct block_list *tbl, int64 rdamage, int64 ldamage, int race, int boss)
{
	struct weapon_data *wd;
	int type, thp = 0, tsp = 0, rhp = 0, rsp = 0, hp, sp, i;
	int64 *damage;

	nullpo_retv(sd);
	for (i = 0; i < 4; i++) {
		//First two iterations: Right hand
		if (i < 2) { wd = &sd->right_weapon; damage = &rdamage; }
		else { wd = &sd->left_weapon; damage = &ldamage; }
		if (*damage <= 0) continue;
		//First and Third iterations: race, other two boss/nonboss state
		if (i == 0 || i == 2)
			type = race;
		else
			type = boss ? RC_BOSS : RC_NONBOSS;

		hp = wd->hp_drain[type].value;
		if (wd->hp_drain[type].rate)
			hp += battle->calc_drain(*damage, wd->hp_drain[type].rate, wd->hp_drain[type].per);

		sp = wd->sp_drain[type].value;
		if (wd->sp_drain[type].rate)
			sp += battle->calc_drain(*damage, wd->sp_drain[type].rate, wd->sp_drain[type].per);

		// HPVanishRate
		if (sd->bonus.hp_vanish_rate && rnd() % 1000 < sd->bonus.hp_vanish_rate && !sd->bonus.hp_vanish_trigger)
			status_percent_damage(&sd->bl, tbl, (unsigned char)sd->bonus.hp_vanish_per, 0, false);

		// SPVanishRate
		if (sd->bonus.sp_vanish_rate && rnd() % 1000 < sd->bonus.sp_vanish_rate && !sd->bonus.sp_vanish_trigger)
			status_percent_damage(&sd->bl, tbl, 0, (unsigned char)sd->bonus.sp_vanish_per, false);

		if (hp) {
			if (wd->hp_drain[type].type)
				rhp += hp;
			thp += hp;
		}
		if (sp) {
			if (wd->sp_drain[type].type)
				rsp += sp;
			tsp += sp;
		}
	}

	if (sd->sp_gain_race_attack[race])
		tsp += sd->sp_gain_race_attack[race];
	if (sd->hp_gain_race_attack[race])
		thp += sd->hp_gain_race_attack[race];

	if (!thp && !tsp) return;

	status->heal(&sd->bl, thp, tsp, battle_config.show_hp_sp_drain ? 3 : 1);

	if (rhp || rsp)
		status_zap(tbl, rhp, rsp);
}
// Deals the same damage to targets in area. [pakpil]
int battle_damage_area(struct block_list *bl, va_list ap) {
	int64 tick;
	int amotion, dmotion, damage;
	struct block_list *src;

	nullpo_ret(bl);

	tick = va_arg(ap, int64);
	src=va_arg(ap,struct block_list *);
	amotion=va_arg(ap,int);
	dmotion=va_arg(ap,int);
	damage=va_arg(ap,int);
	if (bl->type == BL_MOB && BL_UCCAST(BL_MOB, bl)->class_ == MOBID_EMPELIUM)
		return 0;
	if( bl != src && battle->check_target(src,bl,BCT_ENEMY) > 0 ) {
		struct map_session_data *sd = NULL;
		nullpo_ret(src);

		map->freeblock_lock();
		sd = BL_CAST(BL_PC, src);

		if (src->type == BL_PC)
			battle->drain(sd, bl, damage, damage, status_get_race(bl), is_boss(bl));
		if( amotion )
			battle->delay_damage(tick, amotion,src,bl,0,CR_REFLECTSHIELD,0,damage,ATK_DEF,0,true);
		else
			status_fix_damage(src,bl,damage,0);
		clif->damage(bl,bl,amotion,dmotion,damage,1,BDT_ENDURE,0);
		if (src->type != BL_PC || !sd->state.autocast)
			skill->additional_effect(src, bl, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL,ATK_DEF,tick);
		map->freeblock_unlock();
	}

	return 0;
}
/*==========================================
 * Do a basic physical attack (call trough unit_attack_timer)
 *------------------------------------------*/
// FIXME: flag is undocumented
enum damage_lv battle_weapon_attack(struct block_list* src, struct block_list* target, int64 tick, int flag) {
	struct map_session_data *sd = NULL, *tsd = NULL;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc, *tsc;
	int64 damage;
	int skillv;
	struct Damage wd;

	nullpo_retr(ATK_NONE, src);
	nullpo_retr(ATK_NONE, target);

	if (src->prev == NULL || target->prev == NULL)
		return ATK_NONE;

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, target);

	sstatus = status->get_status_data(src);
	tstatus = status->get_status_data(target);

	sc = status->get_sc(src);
	tsc = status->get_sc(target);

	if (sc && !sc->count) //Avoid sc checks when there's none to check for. [Skotlex]
		sc = NULL;
	if (tsc && !tsc->count)
		tsc = NULL;

	if (sd)
	{
		sd->state.arrow_atk = (sd->status.weapon == W_BOW || (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE));
		if (sd->state.arrow_atk)
		{
			int index = sd->equip_index[EQI_AMMO];
			if (index<0) {
				if ( sd->weapontype1 > W_KATAR && sd->weapontype1 < W_HUUMA )
					clif->skill_fail(sd, 0, USESKILL_FAIL_NEED_MORE_BULLET, 0);
				else
					clif->arrow_fail(sd, 0);
				return ATK_NONE;
			}
			//Ammo check by Ishizu-chan
			if (sd->inventory_data[index])
			switch (sd->status.weapon) {
			case W_BOW:
				if (sd->inventory_data[index]->look != A_ARROW) {
					clif->arrow_fail(sd,0);
					return ATK_NONE;
				}
			break;
			case W_REVOLVER:
			case W_RIFLE:
			case W_GATLING:
			case W_SHOTGUN:
				if (sd->inventory_data[index]->look != A_BULLET) {
					clif->skill_fail(sd, 0, USESKILL_FAIL_NEED_MORE_BULLET, 0);
					return ATK_NONE;
				}
			break;
			case W_GRENADE:
				if (sd->inventory_data[index]->look != A_GRENADE) {
					clif->skill_fail(sd, 0, USESKILL_FAIL_NEED_MORE_BULLET, 0);
					return ATK_NONE;
				}
			break;
			}
		}
	}
	if (sc && sc->count) {
		if (sc->data[SC_CLOAKING] && !(sc->data[SC_CLOAKING]->val4 & 2))
			status_change_end(src, SC_CLOAKING, INVALID_TIMER);
		else if (sc->data[SC_CLOAKINGEXCEED] && !(sc->data[SC_CLOAKINGEXCEED]->val4 & 2))
			status_change_end(src, SC_CLOAKINGEXCEED, INVALID_TIMER);
	}
	if( tsc && tsc->data[SC_AUTOCOUNTER] && status->check_skilluse(target, src, KN_AUTOCOUNTER, 1) ) {
		uint8 dir = map->calc_dir(target,src->x,src->y);
		int t_dir = unit->getdir(target);
		int dist = distance_bl(src, target);
		if(dist <= 0 || (!map->check_dir(dir,t_dir) && dist <= tstatus->rhw.range+1)) {
			uint16 skill_lv = tsc->data[SC_AUTOCOUNTER]->val1;
			clif->skillcastcancel(target); //Remove the casting bar. [Skotlex]
			clif->damage(src, target, sstatus->amotion, 1, 0, 1, BDT_NORMAL, 0); //Display MISS.
			status_change_end(target, SC_AUTOCOUNTER, INVALID_TIMER);
			skill->attack(BF_WEAPON,target,target,src,KN_AUTOCOUNTER,skill_lv,tick,0);
			return ATK_BLOCK;
		}
	}
	if( tsc && tsc->data[SC_BLADESTOP_WAIT] && !is_boss(src) && (src->type == BL_PC || tsd == NULL || distance_bl(src, target) <= (tsd->status.weapon == W_FIST ? 1 : 2)) )
	{
		uint16 skill_lv = tsc->data[SC_BLADESTOP_WAIT]->val1;
		int duration = skill->get_time2(MO_BLADESTOP,skill_lv);
		status_change_end(target, SC_BLADESTOP_WAIT, INVALID_TIMER);
		if(sc_start4(target, src, SC_BLADESTOP, 100, sd?pc->checkskill(sd, MO_BLADESTOP):5, 0, 0, target->id, duration)) {
			//Target locked.
			clif->damage(src, target, sstatus->amotion, 1, 0, 1, BDT_NORMAL, 0); //Display MISS.
			clif->bladestop(target, src->id, 1);
			sc_start4(target, target, SC_BLADESTOP, 100, skill_lv, 0, 0, src->id, duration);
			return ATK_BLOCK;
		}
	}

	if(sd && (skillv = pc->checkskill(sd,MO_TRIPLEATTACK)) > 0) {
		int triple_rate= 30 - skillv; //Base Rate
		if (sc && sc->data[SC_SKILLRATE_UP] && sc->data[SC_SKILLRATE_UP]->val1 == MO_TRIPLEATTACK) {
			triple_rate+= triple_rate*(sc->data[SC_SKILLRATE_UP]->val2)/100;
			status_change_end(src, SC_SKILLRATE_UP, INVALID_TIMER);
		}
		if (rnd()%100 < triple_rate) {
			if( skill->attack(BF_WEAPON,src,src,target,MO_TRIPLEATTACK,skillv,tick,0) )
				return ATK_DEF;
			return ATK_MISS;
		}
	}

	if (sc) {
		if (sc->data[SC_SACRIFICE]) {
			uint16 skill_lv = sc->data[SC_SACRIFICE]->val1;
			damage_lv ret_val;

			if( --sc->data[SC_SACRIFICE]->val2 <= 0 )
				status_change_end(src, SC_SACRIFICE, INVALID_TIMER);

			/**
			 * We need to calculate the DMG before the hp reduction, because it can kill the source.
			 * For further information: bugreport:4950
			 **/
			ret_val = (damage_lv)skill->attack(BF_WEAPON,src,src,target,PA_SACRIFICE,skill_lv,tick,0);

			status_zap(src, sstatus->max_hp*9/100, 0);//Damage to self is always 9%
			if( ret_val == ATK_NONE )
				return ATK_MISS;
			return ret_val;
		}
		if (sc->data[SC_MAGICALATTACK]) {
			if( skill->attack(BF_MAGIC,src,src,target,NPC_MAGICALATTACK,sc->data[SC_MAGICALATTACK]->val1,tick,0) )
				return ATK_DEF;
			return ATK_MISS;
		}

		if( tsc && tsc->data[SC_MTF_MLEATKED] && rnd()%100 < 20 )
			clif->skill_nodamage(target, target, SM_ENDURE, 5,
				sc_start(target,target, SC_ENDURE, 100, 5, skill->get_time(SM_ENDURE, 5)));
	}

	if(tsc && tsc->data[SC_KAAHI] && tsc->data[SC_KAAHI]->val4 == INVALID_TIMER && tstatus->hp < tstatus->max_hp)
		tsc->data[SC_KAAHI]->val4 = timer->add(tick + skill->get_time2(SL_KAAHI,tsc->data[SC_KAAHI]->val1), status->kaahi_heal_timer, target->id, SC_KAAHI); //Activate heal.

	wd = battle->calc_attack(BF_WEAPON, src, target, 0, 0, flag);

	if( sc && sc->count ) {
		if( sc->data[SC_SPELLFIST] ) {
			if( --(sc->data[SC_SPELLFIST]->val1) >= 0 ){
				struct Damage ad = battle->calc_attack(BF_MAGIC,src,target,sc->data[SC_SPELLFIST]->val3,sc->data[SC_SPELLFIST]->val4,flag|BF_SHORT);
				wd.damage = ad.damage;
				damage_div_fix(wd.damage, wd.div_);
			}else
				status_change_end(src,SC_SPELLFIST,INVALID_TIMER);
		}

		if( sd && sc->data[SC_FEARBREEZE] && sc->data[SC_FEARBREEZE]->val4 > 0 && sd->status.inventory[sd->equip_index[EQI_AMMO]].amount >= sc->data[SC_FEARBREEZE]->val4 && battle_config.arrow_decrement){
			pc->delitem(sd, sd->equip_index[EQI_AMMO], sc->data[SC_FEARBREEZE]->val4, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);
			sc->data[SC_FEARBREEZE]->val4 = 0;
		}
	}
	if (sd && sd->state.arrow_atk) //Consume arrow.
		battle->consume_ammo(sd, 0, 0);

	damage = wd.damage + wd.damage2;
	if( damage > 0 && src != target ) {
		if( sc && sc->data[SC_DUPLELIGHT] && (wd.flag&BF_SHORT) && rnd()%100 <= 10+2*sc->data[SC_DUPLELIGHT]->val1 ){
			// Activates it only from melee damage
			uint16 skill_id;
			if( rnd()%2 == 1 )
				skill_id = AB_DUPLELIGHT_MELEE;
			else
				skill_id = AB_DUPLELIGHT_MAGIC;
			skill->attack(skill->get_type(skill_id), src, src, target, skill_id, sc->data[SC_DUPLELIGHT]->val1, tick, SD_LEVEL);
		}
	}

	wd.dmotion = clif->damage(src, target, wd.amotion, wd.dmotion, wd.damage, wd.div_ , wd.type, wd.damage2);

	if (sd && sd->bonus.splash_range > 0 && damage > 0)
		skill->castend_damage_id(src, target, 0, 1, tick, 0);
	if (target->type == BL_SKILL && damage > 0) {
		struct skill_unit *su = BL_UCAST(BL_SKILL, target);
		if (su->group && su->group->skill_id == HT_BLASTMINE)
			skill->blown(src, target, 3, -1, 0);
	}
	map->freeblock_lock();

	if( skill->check_shadowform(target, damage, wd.div_) ){
		if( !status->isdead(target) )
			skill->additional_effect(src, target, 0, 0, wd.flag, wd.dmg_lv, tick);
		if( wd.dmg_lv > ATK_BLOCK)
			skill->counter_additional_effect(src, target, 0, 0, wd.flag,tick);
	}else
		battle->delay_damage(tick, wd.amotion, src, target, wd.flag, 0, 0, damage, wd.dmg_lv, wd.dmotion, true);
	if( tsc ) {
		if( tsc->data[SC_DEVOTION] ) {
			struct status_change_entry *sce = tsc->data[SC_DEVOTION];
			struct block_list *d_bl = map->id2bl(sce->val1);
			struct mercenary_data *d_md = BL_CAST(BL_MER, d_bl);
			struct map_session_data *d_sd = BL_CAST(BL_PC, d_bl);

			if (d_bl != NULL
			 && ((d_bl->type == BL_MER && d_md->master != NULL && d_md->master->bl.id == target->id)
			  || (d_bl->type == BL_PC && d_sd->devotion[sce->val2] == target->id)
			    )
			 && check_distance_bl(target, d_bl, sce->val3)
			) {
				clif->damage(d_bl, d_bl, 0, 0, damage, 0, BDT_NORMAL, 0);
				status_fix_damage(NULL, d_bl, damage, 0);
			} else {
				status_change_end(target, SC_DEVOTION, INVALID_TIMER);
			}
		} else if( tsc->data[SC_CIRCLE_OF_FIRE_OPTION] && (wd.flag&BF_SHORT) && target->type == BL_PC ) {
			struct elemental_data *ed = BL_UCAST(BL_PC, target)->ed;
			if (ed != NULL) {
				clif->skill_damage(&ed->bl, target, tick, status_get_amotion(src), 0, -30000, 1, EL_CIRCLE_OF_FIRE, tsc->data[SC_CIRCLE_OF_FIRE_OPTION]->val1, BDT_SKILL);
				skill->attack(BF_MAGIC,&ed->bl,&ed->bl,src,EL_CIRCLE_OF_FIRE,tsc->data[SC_CIRCLE_OF_FIRE_OPTION]->val1,tick,wd.flag);
			}
		} else if( tsc->data[SC_WATER_SCREEN_OPTION] && tsc->data[SC_WATER_SCREEN_OPTION]->val1 ) {
			struct block_list *e_bl = map->id2bl(tsc->data[SC_WATER_SCREEN_OPTION]->val1);
			if( e_bl && !status->isdead(e_bl) ) {
				clif->damage(e_bl,e_bl,wd.amotion,wd.dmotion,damage,wd.div_,wd.type,wd.damage2);
				status->damage(target,e_bl,damage,0,0,0);
				// Just show damage in target.
				clif->damage(src, target, wd.amotion, wd.dmotion, damage, wd.div_, wd.type, wd.damage2 );
				map->freeblock_unlock();
				return ATK_NONE;
			}
		}
	}
	if (sc && sc->data[SC_AUTOSPELL] && rnd()%100 < sc->data[SC_AUTOSPELL]->val4) {
		int sp = 0;
		uint16 skill_id = sc->data[SC_AUTOSPELL]->val2;
		uint16 skill_lv = sc->data[SC_AUTOSPELL]->val3;
		int i = rnd()%100;
		if (sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_SAGE)
			i = 0; //Max chance, no skill_lv reduction. [Skotlex]
		if (i >= 50) skill_lv -= 2;
		else if (i >= 15) skill_lv--;
		if (skill_lv < 1) skill_lv = 1;
		sp = skill->get_sp(skill_id,skill_lv) * 2 / 3;

		if (status->charge(src, 0, sp)) {
			switch (skill->get_casttype(skill_id)) {
				case CAST_GROUND:
					skill->castend_pos2(src, target->x, target->y, skill_id, skill_lv, tick, flag);
					break;
				case CAST_NODAMAGE:
					skill->castend_nodamage_id(src, target, skill_id, skill_lv, tick, flag);
					break;
				case CAST_DAMAGE:
					skill->castend_damage_id(src, target, skill_id, skill_lv, tick, flag);
					break;
			}
		}
	}
	if (sd) {
		if( wd.flag&BF_SHORT && sc
		 && sc->data[SC__AUTOSHADOWSPELL] && rnd()%100 < sc->data[SC__AUTOSHADOWSPELL]->val3
		 && sd->status.skill[skill->get_index(sc->data[SC__AUTOSHADOWSPELL]->val1)].id != 0
		 && sd->status.skill[skill->get_index(sc->data[SC__AUTOSHADOWSPELL]->val1)].flag == SKILL_FLAG_PLAGIARIZED
		) {
			int r_skill = sd->status.skill[skill->get_index(sc->data[SC__AUTOSHADOWSPELL]->val1)].id;
			int r_lv = sc->data[SC__AUTOSHADOWSPELL]->val2;

			if (r_skill != AL_HOLYLIGHT && r_skill != PR_MAGNUS) {
				int type;
				if( (type = skill->get_casttype(r_skill)) == CAST_GROUND ) {
					int maxcount = 0;

					if( !(BL_PC&battle_config.skill_reiteration)
					 && skill->get_unit_flag(r_skill)&UF_NOREITERATION )
						type = -1;

					if( BL_PC&battle_config.skill_nofootset
					 && skill->get_unit_flag(r_skill)&UF_NOFOOTSET )
						type = -1;

					if( BL_PC&battle_config.land_skill_limit
					 && (maxcount = skill->get_maxcount(r_skill, r_lv)) > 0
					) {
						int v;
						for(v=0;v<MAX_SKILLUNITGROUP && sd->ud.skillunit[v] && maxcount;v++) {
							if(sd->ud.skillunit[v]->skill_id == r_skill)
								maxcount--;
						}
						if( maxcount == 0 )
							type = -1;
					}

					if( type != CAST_GROUND ) {
						clif->skill_fail(sd,r_skill,USESKILL_FAIL_LEVEL,0);
						map->freeblock_unlock();
						return wd.dmg_lv;
					}
				}

				sd->state.autocast = 1;
				skill->consume_requirement(sd,r_skill,r_lv,3);
				switch( type ) {
					case CAST_GROUND:
						skill->castend_pos2(src, target->x, target->y, r_skill, r_lv, tick, flag);
						break;
					case CAST_NODAMAGE:
						skill->castend_nodamage_id(src, target, r_skill, r_lv, tick, flag);
						break;
					case CAST_DAMAGE:
						skill->castend_damage_id(src, target, r_skill, r_lv, tick, flag);
						break;
				}
				sd->state.autocast = 0;

				sd->ud.canact_tick = tick + skill->delay_fix(src, r_skill, r_lv);
				clif->status_change(src, SI_POSTDELAY, 1, skill->delay_fix(src, r_skill, r_lv), 0, 0, 1);
			}
		}

		if (wd.flag & BF_WEAPON && src != target && damage > 0) {
			if (battle_config.left_cardfix_to_right)
				battle->drain(sd, target, wd.damage, wd.damage, tstatus->race, is_boss(target));
			else
				battle->drain(sd, target, wd.damage, wd.damage2, tstatus->race, is_boss(target));
		}
	}

	if (tsc) {
		if (tsc->data[SC_POISONREACT]
		 && ( rnd()%100 < tsc->data[SC_POISONREACT]->val3
		    || sstatus->def_ele == ELE_POISON
		    )
		 /* && check_distance_bl(src, target, tstatus->rhw.range+1) Doesn't check range! o.O; */
		 && status->check_skilluse(target, src, TF_POISON, 0)
		) {
			//Poison React
			struct status_change_entry *sce = tsc->data[SC_POISONREACT];
			if (sstatus->def_ele == ELE_POISON) {
				sce->val2 = 0;
				skill->attack(BF_WEAPON,target,target,src,AS_POISONREACT,sce->val1,tick,0);
			} else {
				skill->attack(BF_WEAPON,target,target,src,TF_POISON, 5, tick, 0);
				--sce->val2;
			}
			if (sce->val2 <= 0)
				status_change_end(target, SC_POISONREACT, INVALID_TIMER);
		}
	}
	map->freeblock_unlock();
	return wd.dmg_lv;
}
#undef ATK_RATE
#undef ATK_RATER
#undef ATK_RATEL
#undef ATK_ADDRATE
#undef ATK_ADD
#undef ATK_ADD2
#undef GET_NORMAL_ATTACK
#undef GET_NORMAL_ATTACK2

bool battle_check_undead(int race,int element)
{
	if(battle_config.undead_detect_type == 0) {
		if(element == ELE_UNDEAD)
			return true;
	}
	else if(battle_config.undead_detect_type == 1) {
		if(race == RC_UNDEAD)
			return true;
	}
	else {
		if(element == ELE_UNDEAD || race == RC_UNDEAD)
			return true;
	}
	return false;
}

//Returns the upmost level master starting with the given object
struct block_list *battle_get_master(struct block_list *src)
{
	struct block_list *prev = NULL; //Used for infinite loop check (master of yourself?)
	nullpo_retr(NULL, src);
	do {
		prev = src;
		switch (src->type) {
			case BL_PET:
			{
				struct pet_data *pd = BL_UCAST(BL_PET, src);
				if (pd->msd != NULL)
					src = &pd->msd->bl;
			}
				break;
			case BL_MOB:
			{
				struct mob_data *md = BL_UCAST(BL_MOB, src);
				if (md->master_id != 0)
					src = map->id2bl(md->master_id);
			}
				break;
			case BL_HOM:
			{
				struct homun_data *hd = BL_UCAST(BL_HOM, src);
				if (hd->master != NULL)
					src = &hd->master->bl;
			}
				break;
			case BL_MER:
			{
				struct mercenary_data *md = BL_UCAST(BL_MER, src);
				if (md->master != NULL)
					src = &md->master->bl;
			}
				break;
			case BL_ELEM:
			{
				struct elemental_data *ed = BL_UCAST(BL_ELEM, src);
				if (ed->master != NULL)
					src = &ed->master->bl;
			}
				break;
			case BL_SKILL:
			{
				struct skill_unit *su = BL_UCAST(BL_SKILL, src);
				if (su->group != NULL && su->group->src_id != 0)
					src = map->id2bl(su->group->src_id);
			}
				break;
		}
	} while (src && src != prev);
	return prev;
}

/*==========================================
 * Checks the state between two targets (rewritten by Skotlex)
 * (enemy, friend, party, guild, etc)
 * See battle.h for possible values/combinations
 * to be used here (BCT_* constants)
 * Return value is:
 * 1: flag holds true (is enemy, party, etc)
 * -1: flag fails
 * 0: Invalid target (non-targetable ever)
 *------------------------------------------*/
int battle_check_target( struct block_list *src, struct block_list *target,int flag)
{
	int16 m; //map
	int state = 0; //Initial state none
	int strip_enemy = 1; //Flag which marks whether to remove the BCT_ENEMY status if it's also friend/ally.
	struct block_list *s_bl = src, *t_bl = target;

	nullpo_ret(src);
	nullpo_ret(target);

	m = target->m;

	if (flag & BCT_ENEMY && (map->getcell(m, src, src->x, src->y, CELL_CHKBASILICA) || map->getcell(m, src, target->x, target->y, CELL_CHKBASILICA))) {
		return -1;
	}

	//t_bl/s_bl hold the 'master' of the attack, while src/target are the actual
	//objects involved.
	if( (t_bl = battle->get_master(target)) == NULL )
		t_bl = target;

	if( (s_bl = battle->get_master(src)) == NULL )
		s_bl = src;

	if (s_bl->type == BL_PC) {
		const struct map_session_data *s_sd = BL_UCCAST(BL_PC, s_bl);
		switch (t_bl->type) {
			case BL_MOB: // Source => PC, Target => MOB
				if (pc_has_permission(s_sd, PC_PERM_DISABLE_PVM))
					return 0;
				break;
			case BL_PC:
				if (pc_has_permission(s_sd, PC_PERM_DISABLE_PVP))
					return 0;
				break;
			default:/* anything else goes */
				break;
		}
	}

	switch( target->type ) { // Checks on actual target
		case BL_PC:
		{
			const struct status_change *sc = status->get_sc(src);
			const struct map_session_data *t_sd = BL_UCCAST(BL_PC, target);
			if (t_sd->invincible_timer != INVALID_TIMER) {
				switch( battle->get_current_skill(src) ) {
					/* TODO a proper distinction should be established bugreport:8397 */
					case PR_SANCTUARY:
					case PR_MAGNIFICAT:
						break;
					default:
						return -1;
				}
			}
			if (pc_isinvisible(t_sd))
				return -1; //Cannot be targeted yet.
			if (sc && sc->count) {
				if (sc->data[SC_SIREN] && sc->data[SC_SIREN]->val2 == target->id)
					return -1;
			}
		}
			break;
		case BL_MOB:
		{
			const struct mob_data *md = BL_UCCAST(BL_MOB, target);
			if((
			     (md->special_state.ai == AI_SPHERE || (md->special_state.ai == AI_FLORA && battle_config.summon_flora&1))
			     && s_bl->type == BL_PC && src->type != BL_MOB
			   )
			 || (md->special_state.ai == AI_ZANZOU && t_bl->id != s_bl->id)
			) {
				//Targetable by players
				state |= BCT_ENEMY;
				strip_enemy = 0;
			}
			break;
		}
		case BL_SKILL:
		{
			const struct skill_unit *su = BL_UCCAST(BL_SKILL, target);
			if( !su->group )
				return 0;
			if( skill->get_inf2(su->group->skill_id)&INF2_TRAP &&
				su->group->unit_id != UNT_USED_TRAPS &&
				su->group->unit_id != UNT_NETHERWORLD ) { //Only a few skills can target traps...
				switch( battle->get_current_skill(src) ) {
					case RK_DRAGONBREATH:// it can only hit traps in pvp/gvg maps
					case RK_DRAGONBREATH_WATER:
						if( !map->list[m].flag.pvp && !map->list[m].flag.gvg )
							break;
					case 0://you can hit them without skills
					case MA_REMOVETRAP:
					case HT_REMOVETRAP:
					case AC_SHOWER:
					case MA_SHOWER:
					case WZ_SIGHTRASHER:
					case WZ_SIGHTBLASTER:
					case SM_MAGNUM:
					case MS_MAGNUM:
					case RA_DETONATOR:
					case RA_SENSITIVEKEEN:
					case RK_STORMBLAST:
					case SR_RAMPAGEBLASTER:
					case NC_COLDSLOWER:
#ifdef RENEWAL
					case KN_BOWLINGBASH:
					case KN_SPEARSTAB:
					case LK_SPIRALPIERCE:
					case ML_SPIRALPIERCE:
					case MO_FINGEROFFENSIVE:
					case MO_INVESTIGATE:
					case MO_TRIPLEATTACK:
					case MO_EXTREMITYFIST:
					case CR_HOLYCROSS:
					case ASC_METEORASSAULT:
					case RG_RAID:
					case MC_CARTREVOLUTION:
					case HT_CLAYMORETRAP:
					case RA_ICEBOUNDTRAP:
					case RA_FIRINGTRAP:
#endif
						state |= BCT_ENEMY;
						strip_enemy = 0;
						break;
					default:
						return 0;
				}
			} else if (su->group->skill_id==WZ_ICEWALL ||
					su->group->skill_id == GN_WALLOFTHORN) {
				state |= BCT_ENEMY;
				strip_enemy = 0;
			} else {
				//Excepting traps and icewall, you should not be able to target skills.
				return 0;
			}
		}
			break;
		//Valid targets with no special checks here.
		case BL_MER:
		case BL_HOM:
		case BL_ELEM:
			break;
		//All else not specified is an invalid target.
		default:
			return 0;
	} //end switch actual target

	switch( t_bl->type ) { //Checks on target master
		case BL_PC:
		{
			const struct map_session_data *sd = BL_UCCAST(BL_PC, t_bl);
			if (t_bl == s_bl)
				break;

			if( sd->state.monster_ignore && flag&BCT_ENEMY )
				return 0; // Global immunity only to Attacks
			if (sd->status.karma && s_bl->type == BL_PC && BL_UCCAST(BL_PC, s_bl)->status.karma)
				state |= BCT_ENEMY; // Characters with bad karma may fight amongst them
			if( sd->state.killable ) {
				state |= BCT_ENEMY; // Everything can kill it
				strip_enemy = 0;
			}
			break;
		}
		case BL_MOB:
		{
			const struct mob_data *md = BL_UCCAST(BL_MOB, t_bl);

			if( !((map->agit_flag || map->agit2_flag) && map->list[m].flag.gvg_castle)
				&& md->guardian_data && (md->guardian_data->g || md->guardian_data->castle->guild_id) )
				return 0; // Disable guardians/emperiums owned by Guilds on non-woe times.
			break;
		}
		default: break; //other type doesn't have slave yet
	} //end switch master target

	switch( src->type ) { //Checks on actual src type
		case BL_PET:
			if (t_bl->type != BL_MOB && flag&BCT_ENEMY)
				return 0; //Pet may not attack non-mobs.
			if (t_bl->type == BL_MOB && BL_UCCAST(BL_MOB, t_bl)->guardian_data && flag&BCT_ENEMY)
				return 0; //pet may not attack Guardians/Emperium
			break;
		case BL_SKILL:
		{
			const struct skill_unit *su = BL_UCCAST(BL_SKILL, src);
			const struct status_change *sc = status->get_sc(target);
			if (su->group == NULL)
				return 0;

			if (su->group->src_id == target->id) {
				int inf2 = skill->get_inf2(su->group->skill_id);
				if (inf2&INF2_NO_TARGET_SELF)
					return -1;
				if (inf2&INF2_TARGET_SELF)
					return 1;
			}
			//Status changes that prevent traps from triggering
			if (sc != NULL && sc->count != 0 && skill->get_inf2(su->group->skill_id)&INF2_TRAP) {
				if (sc->data[SC_WZ_SIGHTBLASTER] != NULL && sc->data[SC_WZ_SIGHTBLASTER]->val2 > 0 && sc->data[SC_WZ_SIGHTBLASTER]->val4%2 == 0)
					return -1;
			}
		}
			break;
		case BL_MER:
			if (t_bl->type == BL_MOB && BL_UCCAST(BL_MOB, t_bl)->class_ == MOBID_EMPELIUM && flag&BCT_ENEMY)
				return 0; //mercenary may not attack Emperium
			break;
	} //end switch actual src

	switch( s_bl->type ) { //Checks on source master
		case BL_PC:
		{
			const struct map_session_data *sd = BL_UCCAST(BL_PC, s_bl);
			if( s_bl != t_bl ) {
				if( sd->state.killer ) {
					state |= BCT_ENEMY; // Can kill anything
					strip_enemy = 0;
				} else if( sd->duel_group
				        && !((!battle_config.duel_allow_pvp && map->list[m].flag.pvp) || (!battle_config.duel_allow_gvg && map_flag_gvg(m)))
				) {
					if (t_bl->type == BL_PC && sd->duel_group == BL_UCCAST(BL_PC, t_bl)->duel_group)
						return (BCT_ENEMY&flag)?1:-1; // Duel targets can ONLY be your enemy, nothing else.
					else if (src->type != BL_SKILL || (flag&BCT_ALL) != BCT_ALL)
						return 0;
				}
			}
			if (map_flag_gvg(m) && !sd->status.guild_id && t_bl->type == BL_MOB && BL_UCCAST(BL_MOB, t_bl)->class_ == MOBID_EMPELIUM)
				return 0; //If you don't belong to a guild, can't target emperium.
			if( t_bl->type != BL_PC )
				state |= BCT_ENEMY; //Natural enemy.
			break;
		}
		case BL_MOB:
		{
			const struct mob_data *md = BL_UCCAST(BL_MOB, s_bl);
			if( !((map->agit_flag || map->agit2_flag) && map->list[m].flag.gvg_castle)
				&& md->guardian_data && (md->guardian_data->g || md->guardian_data->castle->guild_id) )
				return 0; // Disable guardians/emperium owned by Guilds on non-woe times.

			if (md->special_state.ai == AI_NONE) {
				//Normal mobs
				const struct mob_data *target_md = BL_CCAST(BL_MOB, target);
				if ((target_md != NULL && t_bl->type == BL_PC && target_md->special_state.ai != AI_ZANZOU && target_md->special_state.ai != AI_ATTACK)
				 || (t_bl->type == BL_MOB && BL_UCCAST(BL_MOB, t_bl)->special_state.ai == AI_NONE))
					state |= BCT_PARTY; //Normal mobs with no ai are friends.
				else
					state |= BCT_ENEMY; //However, all else are enemies.
			} else {
				if (t_bl->type == BL_MOB && BL_UCCAST(BL_MOB, t_bl)->special_state.ai == AI_NONE)
					state |= BCT_ENEMY; //Natural enemy for AI mobs are normal mobs.
			}
			break;
		}
		default:
		//Need some sort of default behavior for unhandled types.
			if (t_bl->type != s_bl->type)
				state |= BCT_ENEMY;
			break;
	} //end switch on src master

	if( (flag&BCT_ALL) == BCT_ALL )
	{ //All actually stands for all attackable chars
		if( target->type&BL_CHAR )
			return 1;
		else
			return -1;
	}
	if( flag == BCT_NOONE ) //Why would someone use this? no clue.
		return -1;

	if( t_bl == s_bl )
	{ //No need for further testing.
		state |= BCT_SELF|BCT_PARTY|BCT_GUILD;
		if( state&BCT_ENEMY && strip_enemy )
			state&=~BCT_ENEMY;
		return (flag&state)?1:-1;
	}

	if( map_flag_vs(m) ) {
		//Check rivalry settings.
		int sbg_id = 0, tbg_id = 0;
		if( map->list[m].flag.battleground ) {
			sbg_id = bg->team_get_id(s_bl);
			tbg_id = bg->team_get_id(t_bl);
		}
		if( flag&(BCT_PARTY|BCT_ENEMY) ) {
			int s_party = status->get_party_id(s_bl);
			int s_guild = status->get_guild_id(s_bl);

			if( s_party && s_party == status->get_party_id(t_bl)
			 && !(map->list[m].flag.pvp && map->list[m].flag.pvp_noparty)
			 && !(map_flag_gvg(m) && map->list[m].flag.gvg_noparty && !( s_guild && s_guild == status->get_guild_id(t_bl) ))
			 && (!map->list[m].flag.battleground || sbg_id == tbg_id) )
				state |= BCT_PARTY;
			else
				state |= BCT_ENEMY;
		}
		if( flag&(BCT_GUILD|BCT_ENEMY) ) {
			int s_guild = status->get_guild_id(s_bl);
			int t_guild = status->get_guild_id(t_bl);
			if( !(map->list[m].flag.pvp && map->list[m].flag.pvp_noguild)
			 && s_guild && t_guild
			 && (s_guild == t_guild || (!(flag&BCT_SAMEGUILD) && guild->isallied(s_guild, t_guild)))
			 && (!map->list[m].flag.battleground || sbg_id == tbg_id) )
				state |= BCT_GUILD;
			else
				state |= BCT_ENEMY;
		}
		if( state&BCT_ENEMY && map->list[m].flag.battleground && sbg_id && sbg_id == tbg_id )
			state &= ~BCT_ENEMY;

		if (state&BCT_ENEMY && battle_config.pk_mode && !map_flag_gvg(m) && s_bl->type == BL_PC && t_bl->type == BL_PC) {
			// Prevent novice engagement on pk_mode (feature by Valaris)
			const struct map_session_data *s_sd = BL_UCCAST(BL_PC, s_bl);
			const struct map_session_data *t_sd = BL_UCCAST(BL_PC, t_bl);
			if (
				(s_sd->class_&MAPID_UPPERMASK) == MAPID_NOVICE ||
				(t_sd->class_&MAPID_UPPERMASK) == MAPID_NOVICE ||
				(int)s_sd->status.base_level < battle_config.pk_min_level ||
				(int)t_sd->status.base_level < battle_config.pk_min_level ||
				(battle_config.pk_level_range && abs((int)s_sd->status.base_level - (int)t_sd->status.base_level) > battle_config.pk_level_range)
			)
				state &= ~BCT_ENEMY;
		}
	}//end map_flag_vs chk rivality
	else
	{ //Non pvp/gvg, check party/guild settings.
		if( flag&BCT_PARTY || state&BCT_ENEMY ) {
			int s_party = status->get_party_id(s_bl);
			if(s_party && s_party == status->get_party_id(t_bl))
				state |= BCT_PARTY;
		}
		if( flag&BCT_GUILD || state&BCT_ENEMY ) {
			int s_guild = status->get_guild_id(s_bl);
			int t_guild = status->get_guild_id(t_bl);
			if(s_guild && t_guild && (s_guild == t_guild || (!(flag&BCT_SAMEGUILD) && guild->isallied(s_guild, t_guild))))
				state |= BCT_GUILD;
		}
	} //end non pvp/gvg chk rivality

	if( !state ) //If not an enemy, nor a guild, nor party, nor yourself, it's neutral.
		state = BCT_NEUTRAL;
	//Alliance state takes precedence over enemy one.
	else if( state&BCT_ENEMY && strip_enemy && state&(BCT_SELF|BCT_PARTY|BCT_GUILD) )
		state&=~BCT_ENEMY;

	return (flag&state)?1:-1;
}
/*==========================================
 * Check if can attack from this range
 * Basic check then calling path->search for obstacle etc..
 *------------------------------------------*/
bool battle_check_range(struct block_list *src, struct block_list *bl, int range)
{
	int d;
	nullpo_retr(false, src);
	nullpo_retr(false, bl);

	if( src->m != bl->m )
		return false;

#ifndef CIRCULAR_AREA
	if( src->type == BL_PC ) { // Range for players' attacks and skills should always have a circular check. [Angezerus]
		if ( !check_distance_client_bl(src, bl, range) )
			return false;
	} else
#endif
	if( !check_distance_bl(src, bl, range) )
		return false;

	if( (d = distance_bl(src, bl)) < 2 )
		return true;  // No need for path checking.

	if( d > AREA_SIZE )
		return false; // Avoid targeting objects beyond your range of sight.

	return path->search_long(NULL,src,src->m,src->x,src->y,bl->x,bl->y,CELL_CHKWALL);
}

static const struct battle_data {
	const char* str;
	int* val;
	int defval;
	int min;
	int max;
} battle_data[] = {
	{ "warp_point_debug",                   &battle_config.warp_point_debug,                0,      0,      1,              },
	{ "enable_critical",                    &battle_config.enable_critical,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "mob_critical_rate",                  &battle_config.mob_critical_rate,               100,    0,      INT_MAX,        },
	{ "critical_rate",                      &battle_config.critical_rate,                   100,    0,      INT_MAX,        },
	{ "enable_baseatk",                     &battle_config.enable_baseatk,                  BL_PC|BL_HOM, BL_NUL, BL_ALL,   },
	{ "enable_perfect_flee",                &battle_config.enable_perfect_flee,             BL_PC|BL_PET, BL_NUL, BL_ALL,   },
	{ "casting_rate",                       &battle_config.cast_rate,                       100,    0,      INT_MAX,        },
	{ "delay_rate",                         &battle_config.delay_rate,                      100,    0,      INT_MAX,        },
	{ "delay_dependon_dex",                 &battle_config.delay_dependon_dex,              0,      0,      1,              },
	{ "delay_dependon_agi",                 &battle_config.delay_dependon_agi,              0,      0,      1,              },
	{ "skill_delay_attack_enable",          &battle_config.sdelay_attack_enable,            0,      0,      1,              },
	{ "left_cardfix_to_right",              &battle_config.left_cardfix_to_right,           0,      0,      1,              },
	{ "skill_add_range",                    &battle_config.skill_add_range,                 0,      0,      INT_MAX,        },
	{ "skill_out_range_consume",            &battle_config.skill_out_range_consume,         1,      0,      1,              },
	{ "skillrange_by_distance",             &battle_config.skillrange_by_distance,          ~BL_PC, BL_NUL, BL_ALL,         },
	{ "skillrange_from_weapon",             &battle_config.use_weapon_skill_range,          BL_NUL, BL_NUL, BL_ALL,         },
	{ "player_damage_delay_rate",           &battle_config.pc_damage_delay_rate,            100,    0,      INT_MAX,        },
	{ "defunit_not_enemy",                  &battle_config.defnotenemy,                     0,      0,      1,              },
	{ "gvg_traps_target_all",               &battle_config.vs_traps_bctall,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "traps_setting",                      &battle_config.traps_setting,                   0,      0,      1,              },
	{ "summon_flora_setting",               &battle_config.summon_flora,                    1|2,    0,      1|2,            },
	{ "clear_skills_on_death",              &battle_config.clear_unit_ondeath,              BL_NUL, BL_NUL, BL_ALL,         },
	{ "clear_skills_on_warp",               &battle_config.clear_unit_onwarp,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "random_monster_checklv",             &battle_config.random_monster_checklv,          0,      0,      1,              },
	{ "attribute_recover",                  &battle_config.attr_recover,                    1,      0,      1,              },
	{ "flooritem_lifetime",                 &battle_config.flooritem_lifetime,              60000,  1000,   INT_MAX,        },
	{ "item_auto_get",                      &battle_config.item_auto_get,                   0,      0,      1,              },
	{ "item_first_get_time",                &battle_config.item_first_get_time,             3000,   0,      INT_MAX,        },
	{ "item_second_get_time",               &battle_config.item_second_get_time,            1000,   0,      INT_MAX,        },
	{ "item_third_get_time",                &battle_config.item_third_get_time,             1000,   0,      INT_MAX,        },
	{ "mvp_item_first_get_time",            &battle_config.mvp_item_first_get_time,         10000,  0,      INT_MAX,        },
	{ "mvp_item_second_get_time",           &battle_config.mvp_item_second_get_time,        10000,  0,      INT_MAX,        },
	{ "mvp_item_third_get_time",            &battle_config.mvp_item_third_get_time,         2000,   0,      INT_MAX,        },
	{ "drop_rate0item",                     &battle_config.drop_rate0item,                  0,      0,      1,              },
	{ "base_exp_rate",                      &battle_config.base_exp_rate,                   100,    0,      INT_MAX,        },
	{ "job_exp_rate",                       &battle_config.job_exp_rate,                    100,    0,      INT_MAX,        },
	{ "pvp_exp",                            &battle_config.pvp_exp,                         1,      0,      1,              },
	{ "death_penalty_type",                 &battle_config.death_penalty_type,              0,      0,      2,              },
	{ "death_penalty_base",                 &battle_config.death_penalty_base,              0,      0,      INT_MAX,        },
	{ "death_penalty_job",                  &battle_config.death_penalty_job,               0,      0,      INT_MAX,        },
	{ "zeny_penalty",                       &battle_config.zeny_penalty,                    0,      0,      INT_MAX,        },
	{ "hp_rate",                            &battle_config.hp_rate,                         100,    1,      INT_MAX,        },
	{ "sp_rate",                            &battle_config.sp_rate,                         100,    1,      INT_MAX,        },
	{ "restart_hp_rate",                    &battle_config.restart_hp_rate,                 0,      0,      100,            },
	{ "restart_sp_rate",                    &battle_config.restart_sp_rate,                 0,      0,      100,            },
	{ "guild_aura",                         &battle_config.guild_aura,                      31,     0,      31,             },
	{ "mvp_hp_rate",                        &battle_config.mvp_hp_rate,                     100,    1,      INT_MAX,        },
	{ "mvp_exp_rate",                       &battle_config.mvp_exp_rate,                    100,    0,      INT_MAX,        },
	{ "monster_hp_rate",                    &battle_config.monster_hp_rate,                 100,    1,      INT_MAX,        },
	{ "monster_max_aspd",                   &battle_config.monster_max_aspd,                199,    100,    199,            },
	{ "view_range_rate",                    &battle_config.view_range_rate,                 100,    0,      INT_MAX,        },
	{ "chase_range_rate",                   &battle_config.chase_range_rate,                100,    0,      INT_MAX,        },
	{ "gtb_sc_immunity",                    &battle_config.gtb_sc_immunity,                 50,     0,      INT_MAX,        },
	{ "guild_max_castles",                  &battle_config.guild_max_castles,               0,      0,      INT_MAX,        },
	{ "guild_skill_relog_delay",            &battle_config.guild_skill_relog_delay,         0,      0,      1,              },
	{ "emergency_call",                     &battle_config.emergency_call,                  11,     0,      31,             },
	{ "atcommand_spawn_quantity_limit",     &battle_config.atc_spawn_quantity_limit,        100,    0,      INT_MAX,        },
	{ "atcommand_slave_clone_limit",        &battle_config.atc_slave_clone_limit,           25,     0,      INT_MAX,        },
	{ "partial_name_scan",                  &battle_config.partial_name_scan,               0,      0,      1,              },
	{ "player_skillfree",                   &battle_config.skillfree,                       0,      0,      1,              },
	{ "player_skillup_limit",               &battle_config.skillup_limit,                   1,      0,      1,              },
	{ "weapon_produce_rate",                &battle_config.wp_rate,                         100,    0,      INT_MAX,        },
	{ "potion_produce_rate",                &battle_config.pp_rate,                         100,    0,      INT_MAX,        },
	{ "monster_active_enable",              &battle_config.monster_active_enable,           1,      0,      1,              },
	{ "monster_damage_delay_rate",          &battle_config.monster_damage_delay_rate,       100,    0,      INT_MAX,        },
	{ "monster_loot_type",                  &battle_config.monster_loot_type,               0,      0,      1,              },
	//{ "mob_skill_use",                      &battle_config.mob_skill_use,                   1,      0,      1,              }, //Deprecated
	{ "mob_skill_rate",                     &battle_config.mob_skill_rate,                  100,    0,      INT_MAX,        },
	{ "mob_skill_delay",                    &battle_config.mob_skill_delay,                 100,    0,      INT_MAX,        },
	{ "mob_count_rate",                     &battle_config.mob_count_rate,                  100,    0,      INT_MAX,        },
	{ "mob_spawn_delay",                    &battle_config.mob_spawn_delay,                 100,    0,      INT_MAX,        },
	{ "plant_spawn_delay",                  &battle_config.plant_spawn_delay,               100,    0,      INT_MAX,        },
	{ "boss_spawn_delay",                   &battle_config.boss_spawn_delay,                100,    0,      INT_MAX,        },
	{ "no_spawn_on_player",                 &battle_config.no_spawn_on_player,              0,      0,      100,            },
	{ "force_random_spawn",                 &battle_config.force_random_spawn,              0,      0,      1,              },
	{ "slaves_inherit_mode",                &battle_config.slaves_inherit_mode,             2,      0,      3,              },
	{ "slaves_inherit_speed",               &battle_config.slaves_inherit_speed,            3,      0,      3,              },
	{ "summons_trigger_autospells",         &battle_config.summons_trigger_autospells,      1,      0,      1,              },
	{ "pc_damage_walk_delay_rate",          &battle_config.pc_walk_delay_rate,              20,     0,      INT_MAX,        },
	{ "damage_walk_delay_rate",             &battle_config.walk_delay_rate,                 100,    0,      INT_MAX,        },
	{ "multihit_delay",                     &battle_config.multihit_delay,                  80,     0,      INT_MAX,        },
	{ "quest_skill_learn",                  &battle_config.quest_skill_learn,               0,      0,      1,              },
	{ "quest_skill_reset",                  &battle_config.quest_skill_reset,               0,      0,      1,              },
	{ "basic_skill_check",                  &battle_config.basic_skill_check,               1,      0,      1,              },
	{ "guild_emperium_check",               &battle_config.guild_emperium_check,            1,      0,      1,              },
	{ "guild_exp_limit",                    &battle_config.guild_exp_limit,                 50,     0,      99,             },
	{ "player_invincible_time",             &battle_config.pc_invincible_time,              5000,   0,      INT_MAX,        },
	{ "pet_catch_rate",                     &battle_config.pet_catch_rate,                  100,    0,      INT_MAX,        },
	{ "pet_rename",                         &battle_config.pet_rename,                      0,      0,      1,              },
	{ "pet_friendly_rate",                  &battle_config.pet_friendly_rate,               100,    0,      INT_MAX,        },
	{ "pet_hungry_delay_rate",              &battle_config.pet_hungry_delay_rate,           100,    10,     INT_MAX,        },
	{ "pet_hungry_friendly_decrease",       &battle_config.pet_hungry_friendly_decrease,    5,      0,      INT_MAX,        },
	{ "pet_status_support",                 &battle_config.pet_status_support,              0,      0,      1,              },
	{ "pet_attack_support",                 &battle_config.pet_attack_support,              0,      0,      1,              },
	{ "pet_damage_support",                 &battle_config.pet_damage_support,              0,      0,      1,              },
	{ "pet_support_min_friendly",           &battle_config.pet_support_min_friendly,        900,    0,      950,            },
	{ "pet_equip_min_friendly",             &battle_config.pet_equip_min_friendly,          900,    0,      950,            },
	{ "pet_support_rate",                   &battle_config.pet_support_rate,                100,    0,      INT_MAX,        },
	{ "pet_attack_exp_to_master",           &battle_config.pet_attack_exp_to_master,        0,      0,      1,              },
	{ "pet_attack_exp_rate",                &battle_config.pet_attack_exp_rate,             100,    0,      INT_MAX,        },
	{ "pet_lv_rate",                        &battle_config.pet_lv_rate,                     0,      0,      INT_MAX,        },
	{ "pet_max_stats",                      &battle_config.pet_max_stats,                   99,     0,      INT_MAX,        },
	{ "pet_max_atk1",                       &battle_config.pet_max_atk1,                    750,    0,      INT_MAX,        },
	{ "pet_max_atk2",                       &battle_config.pet_max_atk2,                    1000,   0,      INT_MAX,        },
	{ "pet_disable_in_gvg",                 &battle_config.pet_no_gvg,                      0,      0,      1,              },
	{ "skill_min_damage",                   &battle_config.skill_min_damage,                2|4,    0,      1|2|4,          },
	{ "finger_offensive_type",              &battle_config.finger_offensive_type,           0,      0,      1,              },
	{ "heal_exp",                           &battle_config.heal_exp,                        0,      0,      INT_MAX,        },
	{ "resurrection_exp",                   &battle_config.resurrection_exp,                0,      0,      INT_MAX,        },
	{ "shop_exp",                           &battle_config.shop_exp,                        0,      0,      INT_MAX,        },
	{ "max_heal_lv",                        &battle_config.max_heal_lv,                     11,     1,      INT_MAX,        },
	{ "max_heal",                           &battle_config.max_heal,                        9999,   0,      INT_MAX,        },
	{ "combo_delay_rate",                   &battle_config.combo_delay_rate,                100,    0,      INT_MAX,        },
	{ "item_check",                         &battle_config.item_check,                      0,      0,      1,              },
	{ "item_use_interval",                  &battle_config.item_use_interval,               100,    0,      INT_MAX,        },
	{ "cashfood_use_interval",              &battle_config.cashfood_use_interval,           60000,  0,      INT_MAX,        },
	{ "wedding_modifydisplay",              &battle_config.wedding_modifydisplay,           0,      0,      1,              },
	{ "wedding_ignorepalette",              &battle_config.wedding_ignorepalette,           0,      0,      1,              },
	{ "xmas_ignorepalette",                 &battle_config.xmas_ignorepalette,              0,      0,      1,              },
	{ "summer_ignorepalette",               &battle_config.summer_ignorepalette,            0,      0,      1,              },
	{ "hanbok_ignorepalette",               &battle_config.hanbok_ignorepalette,            0,      0,      1,              },
	{ "natural_healhp_interval",            &battle_config.natural_healhp_interval,         6000,   NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_healsp_interval",            &battle_config.natural_healsp_interval,         8000,   NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_heal_skill_interval",        &battle_config.natural_heal_skill_interval,     10000,  NATURAL_HEAL_INTERVAL, INT_MAX, },
	{ "natural_heal_weight_rate",           &battle_config.natural_heal_weight_rate,        50,     50,     101             },
	{ "arrow_decrement",                    &battle_config.arrow_decrement,                 1,      0,      2,              },
	{ "max_aspd",                           &battle_config.max_aspd,                        190,    100,    199,            },
	{ "max_third_aspd",                     &battle_config.max_third_aspd,                  193,    100,    199,            },
	{ "max_walk_speed",                     &battle_config.max_walk_speed,                  300,    100,    100*DEFAULT_WALK_SPEED, },
	{ "max_lv",                             &battle_config.max_lv,                          99,     0,      MAX_LEVEL,      },
	{ "aura_lv",                            &battle_config.aura_lv,                         99,     0,      INT_MAX,        },
	{ "max_hp",                             &battle_config.max_hp,                          1000000, 100,   21474836,       },
	{ "max_sp",                             &battle_config.max_sp,                          1000000, 100,   21474836,       },
	{ "max_cart_weight",                    &battle_config.max_cart_weight,                 8000,   100,    1000000,        },
	{ "max_parameter",                      &battle_config.max_parameter,                   99,     10,     10000,          },
	{ "max_baby_parameter",                 &battle_config.max_baby_parameter,              80,     10,     10000,          },
	{ "max_def",                            &battle_config.max_def,                         99,     0,      INT_MAX,        },
	{ "over_def_bonus",                     &battle_config.over_def_bonus,                  0,      0,      1000,           },
	{ "skill_log",                          &battle_config.skill_log,                       BL_NUL, BL_NUL, BL_ALL,         },
	{ "battle_log",                         &battle_config.battle_log,                      0,      0,      1,              },
	{ "etc_log",                            &battle_config.etc_log,                         1,      0,      1,              },
	{ "save_clothcolor",                    &battle_config.save_clothcolor,                 1,      0,      1,              },
	{ "undead_detect_type",                 &battle_config.undead_detect_type,              0,      0,      2,              },
	{ "auto_counter_type",                  &battle_config.auto_counter_type,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "min_hitrate",                        &battle_config.min_hitrate,                     5,      0,      100,            },
	{ "max_hitrate",                        &battle_config.max_hitrate,                     100,    0,      100,            },
	{ "agi_penalty_target",                 &battle_config.agi_penalty_target,              BL_PC,  BL_NUL, BL_ALL,         },
	{ "agi_penalty_type",                   &battle_config.agi_penalty_type,                1,      0,      2,              },
	{ "agi_penalty_count",                  &battle_config.agi_penalty_count,               3,      2,      INT_MAX,        },
	{ "agi_penalty_num",                    &battle_config.agi_penalty_num,                 10,     0,      INT_MAX,        },
	{ "vit_penalty_target",                 &battle_config.vit_penalty_target,              BL_PC,  BL_NUL, BL_ALL,         },
	{ "vit_penalty_type",                   &battle_config.vit_penalty_type,                1,      0,      2,              },
	{ "vit_penalty_count",                  &battle_config.vit_penalty_count,               3,      2,      INT_MAX,        },
	{ "vit_penalty_num",                    &battle_config.vit_penalty_num,                 5,      0,      INT_MAX,        },
	{ "weapon_defense_type",                &battle_config.weapon_defense_type,             0,      0,      INT_MAX,        },
	{ "magic_defense_type",                 &battle_config.magic_defense_type,              0,      0,      INT_MAX,        },
	{ "skill_reiteration",                  &battle_config.skill_reiteration,               BL_NUL, BL_NUL, BL_ALL,         },
	{ "skill_nofootset",                    &battle_config.skill_nofootset,                 BL_PC,  BL_NUL, BL_ALL,         },
	{ "player_cloak_check_type",            &battle_config.pc_cloak_check_type,             1,      0,      1|2|4,          },
	{ "monster_cloak_check_type",           &battle_config.monster_cloak_check_type,        4,      0,      1|2|4,          },
	{ "sense_type",                         &battle_config.estimation_type,                 1|2,    0,      1|2,            },
	{ "gvg_flee_penalty",                   &battle_config.gvg_flee_penalty,                20,     0,      INT_MAX,        },
	{ "mob_changetarget_byskill",           &battle_config.mob_changetarget_byskill,        0,      0,      1,              },
	{ "attack_direction_change",            &battle_config.attack_direction_change,         BL_ALL, BL_NUL, BL_ALL,         },
	{ "land_skill_limit",                   &battle_config.land_skill_limit,                BL_ALL, BL_NUL, BL_ALL,         },
	{ "monster_class_change_full_recover",  &battle_config.monster_class_change_recover,    1,      0,      1,              },
	{ "produce_item_name_input",            &battle_config.produce_item_name_input,         0x1|0x2, 0,     0x9F,           },
	{ "display_skill_fail",                 &battle_config.display_skill_fail,              2,      0,      1|2|4|8,        },
	{ "chat_warpportal",                    &battle_config.chat_warpportal,                 0,      0,      1,              },
	{ "mob_warp",                           &battle_config.mob_warp,                        0,      0,      1|2|4,          },
	{ "dead_branch_active",                 &battle_config.dead_branch_active,              1,      0,      1,              },
	{ "vending_max_value",                  &battle_config.vending_max_value,               10000000, 1,    MAX_ZENY,       },
	{ "vending_over_max",                   &battle_config.vending_over_max,                1,      0,      1,              },
	{ "show_steal_in_same_party",           &battle_config.show_steal_in_same_party,        0,      0,      1,              },
	{ "party_hp_mode",                      &battle_config.party_hp_mode,                   0,      0,      1,              },
	{ "show_party_share_picker",            &battle_config.party_show_share_picker,         1,      0,      1,              },
	{ "show_picker.item_type",              &battle_config.show_picker_item_type,           112,    0,      INT_MAX,        },
	{ "party_update_interval",              &battle_config.party_update_interval,           1000,   100,    INT_MAX,        },
	{ "party_item_share_type",              &battle_config.party_share_type,                0,      0,      1|2|3,          },
	{ "attack_attr_none",                   &battle_config.attack_attr_none,                ~BL_PC, BL_NUL, BL_ALL,         },
	{ "gx_allhit",                          &battle_config.gx_allhit,                       0,      0,      1,              },
	{ "gx_disptype",                        &battle_config.gx_disptype,                     1,      0,      1,              },
	{ "devotion_level_difference",          &battle_config.devotion_level_difference,       10,     0,      INT_MAX,        },
	{ "player_skill_partner_check",         &battle_config.player_skill_partner_check,      1,      0,      1,              },
	{ "invite_request_check",               &battle_config.invite_request_check,            1,      0,      1,              },
	{ "skill_removetrap_type",              &battle_config.skill_removetrap_type,           0,      0,      1,              },
	{ "disp_experience",                    &battle_config.disp_experience,                 0,      0,      1,              },
	{ "disp_zeny",                          &battle_config.disp_zeny,                       0,      0,      1,              },
	{ "castle_defense_rate",                &battle_config.castle_defense_rate,             100,    0,      100,            },
	{ "bone_drop",                          &battle_config.bone_drop,                       0,      0,      2,              },
	{ "buyer_name",                         &battle_config.buyer_name,                      1,      0,      1,              },
	{ "skill_wall_check",                   &battle_config.skill_wall_check,                1,      0,      1,              },
	{ "official_cell_stack_limit",          &battle_config.official_cell_stack_limit,       1,      0,      255,            },
	{ "custom_cell_stack_limit",            &battle_config.custom_cell_stack_limit,         1,      1,      255,            },
	{ "dancing_weaponswitch_fix",           &battle_config.dancing_weaponswitch_fix,        1,      0,      1,              },
	{ "check_occupied_cells",               &battle_config.check_occupied_cells,            1,      0,      1,              },

// eAthena additions
	{ "item_logarithmic_drops",             &battle_config.logarithmic_drops,               0,      0,      1,              },
	{ "item_drop_common_min",               &battle_config.item_drop_common_min,            1,      1,      10000,          },
	{ "item_drop_common_max",               &battle_config.item_drop_common_max,            10000,  1,      10000,          },
	{ "item_drop_equip_min",                &battle_config.item_drop_equip_min,             1,      1,      10000,          },
	{ "item_drop_equip_max",                &battle_config.item_drop_equip_max,             10000,  1,      10000,          },
	{ "item_drop_card_min",                 &battle_config.item_drop_card_min,              1,      1,      10000,          },
	{ "item_drop_card_max",                 &battle_config.item_drop_card_max,              10000,  1,      10000,          },
	{ "item_drop_mvp_min",                  &battle_config.item_drop_mvp_min,               1,      1,      10000,          },
	{ "item_drop_mvp_max",                  &battle_config.item_drop_mvp_max,               10000,  1,      10000,          },
	{ "item_drop_heal_min",                 &battle_config.item_drop_heal_min,              1,      1,      10000,          },
	{ "item_drop_heal_max",                 &battle_config.item_drop_heal_max,              10000,  1,      10000,          },
	{ "item_drop_use_min",                  &battle_config.item_drop_use_min,               1,      1,      10000,          },
	{ "item_drop_use_max",                  &battle_config.item_drop_use_max,               10000,  1,      10000,          },
	{ "item_drop_add_min",                  &battle_config.item_drop_adddrop_min,           1,      1,      10000,          },
	{ "item_drop_add_max",                  &battle_config.item_drop_adddrop_max,           10000,  1,      10000,          },
	{ "item_drop_treasure_min",             &battle_config.item_drop_treasure_min,          1,      1,      10000,          },
	{ "item_drop_treasure_max",             &battle_config.item_drop_treasure_max,          10000,  1,      10000,          },
	{ "item_rate_mvp",                      &battle_config.item_rate_mvp,                   100,    0,      1000000,        },
	{ "item_rate_common",                   &battle_config.item_rate_common,                100,    0,      1000000,        },
	{ "item_rate_common_boss",              &battle_config.item_rate_common_boss,           100,    0,      1000000,        },
	{ "item_rate_equip",                    &battle_config.item_rate_equip,                 100,    0,      1000000,        },
	{ "item_rate_equip_boss",               &battle_config.item_rate_equip_boss,            100,    0,      1000000,        },
	{ "item_rate_card",                     &battle_config.item_rate_card,                  100,    0,      1000000,        },
	{ "item_rate_card_boss",                &battle_config.item_rate_card_boss,             100,    0,      1000000,        },
	{ "item_rate_heal",                     &battle_config.item_rate_heal,                  100,    0,      1000000,        },
	{ "item_rate_heal_boss",                &battle_config.item_rate_heal_boss,             100,    0,      1000000,        },
	{ "item_rate_use",                      &battle_config.item_rate_use,                   100,    0,      1000000,        },
	{ "item_rate_use_boss",                 &battle_config.item_rate_use_boss,              100,    0,      1000000,        },
	{ "item_rate_adddrop",                  &battle_config.item_rate_adddrop,               100,    0,      1000000,        },
	{ "item_rate_treasure",                 &battle_config.item_rate_treasure,              100,    0,      1000000,        },
	{ "prevent_logout",                     &battle_config.prevent_logout,                  10000,  0,      60000,          },
	{ "alchemist_summon_reward",            &battle_config.alchemist_summon_reward,         1,      0,      2,              },
	{ "drops_by_luk",                       &battle_config.drops_by_luk,                    0,      0,      INT_MAX,        },
	{ "drops_by_luk2",                      &battle_config.drops_by_luk2,                   0,      0,      INT_MAX,        },
	{ "equip_natural_break_rate",           &battle_config.equip_natural_break_rate,        0,      0,      INT_MAX,        },
	{ "equip_self_break_rate",              &battle_config.equip_self_break_rate,           100,    0,      INT_MAX,        },
	{ "equip_skill_break_rate",             &battle_config.equip_skill_break_rate,          100,    0,      INT_MAX,        },
	{ "pk_mode",                            &battle_config.pk_mode,                         0,      0,      2,              },
	{ "pk_level_range",                     &battle_config.pk_level_range,                  0,      0,      INT_MAX,        },
	{ "manner_system",                      &battle_config.manner_system,                   0xFFF,  0,      0xFFF,          },
	{ "pet_equip_required",                 &battle_config.pet_equip_required,              0,      0,      1,              },
	{ "multi_level_up",                     &battle_config.multi_level_up,                  0,      0,      1,              },
	{ "max_exp_gain_rate",                  &battle_config.max_exp_gain_rate,               0,      0,      INT_MAX,        },
	{ "backstab_bow_penalty",               &battle_config.backstab_bow_penalty,            0,      0,      1,              },
	{ "night_at_start",                     &battle_config.night_at_start,                  0,      0,      1,              },
	{ "show_mob_info",                      &battle_config.show_mob_info,                   0,      0,      1|2|4,          },
	{ "ban_hack_trade",                     &battle_config.ban_hack_trade,                  0,      0,      INT_MAX,        },
	{ "min_hair_style",                     &battle_config.min_hair_style,                  0,      0,      INT_MAX,        },
	{ "max_hair_style",                     &battle_config.max_hair_style,                  23,     0,      INT_MAX,        },
	{ "min_hair_color",                     &battle_config.min_hair_color,                  0,      0,      INT_MAX,        },
	{ "max_hair_color",                     &battle_config.max_hair_color,                  9,      0,      INT_MAX,        },
	{ "min_cloth_color",                    &battle_config.min_cloth_color,                 0,      0,      INT_MAX,        },
	{ "max_cloth_color",                    &battle_config.max_cloth_color,                 4,      0,      INT_MAX,        },
	{ "pet_hair_style",                     &battle_config.pet_hair_style,                  100,    0,      INT_MAX,        },
	{ "castrate_dex_scale",                 &battle_config.castrate_dex_scale,              150,    1,      INT_MAX,        },
	{ "vcast_stat_scale",                   &battle_config.vcast_stat_scale,                530,    1,      INT_MAX,        },
	{ "area_size",                          &battle_config.area_size,                       14,     0,      INT_MAX,        },
	{ "zeny_from_mobs",                     &battle_config.zeny_from_mobs,                  0,      0,      1,              },
	{ "mobs_level_up",                      &battle_config.mobs_level_up,                   0,      0,      1,              },
	{ "mobs_level_up_exp_rate",             &battle_config.mobs_level_up_exp_rate,          1,      1,      INT_MAX,        },
	{ "pk_min_level",                       &battle_config.pk_min_level,                    55,     1,      INT_MAX,        },
	{ "skill_steal_max_tries",              &battle_config.skill_steal_max_tries,           0,      0,      UCHAR_MAX,      },
	{ "exp_calc_type",                      &battle_config.exp_calc_type,                   0,      0,      1,              },
	{ "exp_bonus_attacker",                 &battle_config.exp_bonus_attacker,              25,     0,      INT_MAX,        },
	{ "exp_bonus_max_attacker",             &battle_config.exp_bonus_max_attacker,          12,     2,      INT_MAX,        },
	{ "min_skill_delay_limit",              &battle_config.min_skill_delay_limit,           100,    10,     INT_MAX,        },
	{ "default_walk_delay",                 &battle_config.default_walk_delay,              300,    0,      INT_MAX,        },
	{ "no_skill_delay",                     &battle_config.no_skill_delay,                  BL_MOB, BL_NUL, BL_ALL,         },
	{ "attack_walk_delay",                  &battle_config.attack_walk_delay,               BL_ALL, BL_NUL, BL_ALL,         },
	{ "require_glory_guild",                &battle_config.require_glory_guild,             0,      0,      1,              },
	{ "idle_no_share",                      &battle_config.idle_no_share,                   0,      0,      INT_MAX,        },
	{ "party_even_share_bonus",             &battle_config.party_even_share_bonus,          0,      0,      INT_MAX,        },
	{ "delay_battle_damage",                &battle_config.delay_battle_damage,             1,      0,      1,              },
	{ "hide_woe_damage",                    &battle_config.hide_woe_damage,                 0,      0,      1,              },
	{ "display_version",                    &battle_config.display_version,                 1,      0,      1,              },
	{ "display_hallucination",              &battle_config.display_hallucination,           1,      0,      1,              },
	{ "use_statpoint_table",                &battle_config.use_statpoint_table,             1,      0,      1,              },
	{ "ignore_items_gender",                &battle_config.ignore_items_gender,             1,      0,      1,              },
	{ "copyskill_restrict",                 &battle_config.copyskill_restrict,              2,      0,      2,              },
	{ "berserk_cancels_buffs",              &battle_config.berserk_cancels_buffs,           0,      0,      1,              },
	{ "monster_ai",                         &battle_config.mob_ai,                          0x000,  0x000,  0x77F,          },
	{ "hom_setting",                        &battle_config.hom_setting,                     0xFFFF, 0x0000, 0xFFFF,         },
	{ "dynamic_mobs",                       &battle_config.dynamic_mobs,                    1,      0,      1,              },
	{ "mob_remove_damaged",                 &battle_config.mob_remove_damaged,              1,      0,      1,              },
	{ "show_hp_sp_drain",                   &battle_config.show_hp_sp_drain,                0,      0,      1,              },
	{ "show_hp_sp_gain",                    &battle_config.show_hp_sp_gain,                 1,      0,      1,              },
	{ "show_katar_crit_bonus",              &battle_config.show_katar_crit_bonus,           0,      0,      1,              },
	{ "mob_npc_event_type",                 &battle_config.mob_npc_event_type,              1,      0,      1,              },
	{ "character_size",                     &battle_config.character_size,                  1|2,    0,      1|2,            },
	{ "retaliate_to_master",                &battle_config.retaliate_to_master,             1,      0,      1,              },
	{ "rare_drop_announce",                 &battle_config.rare_drop_announce,              0,      0,      10000,          },
	{ "duel_allow_pvp",                     &battle_config.duel_allow_pvp,                  0,      0,      1,              },
	{ "duel_allow_gvg",                     &battle_config.duel_allow_gvg,                  0,      0,      1,              },
	{ "duel_allow_teleport",                &battle_config.duel_allow_teleport,             0,      0,      1,              },
	{ "duel_autoleave_when_die",            &battle_config.duel_autoleave_when_die,         1,      0,      1,              },
	{ "duel_time_interval",                 &battle_config.duel_time_interval,              60,     0,      INT_MAX,        },
	{ "duel_only_on_same_map",              &battle_config.duel_only_on_same_map,           0,      0,      1,              },
	{ "skip_teleport_lv1_menu",             &battle_config.skip_teleport_lv1_menu,          0,      0,      1,              },
	{ "mob_max_skilllvl",                   &battle_config.mob_max_skilllvl,                100,    1,      INT_MAX,        },
	{ "allow_skill_without_day",            &battle_config.allow_skill_without_day,         0,      0,      1,              },
	{ "allow_es_magic_player",              &battle_config.allow_es_magic_pc,               0,      0,      1,              },
	{ "skill_caster_check",                 &battle_config.skill_caster_check,              1,      0,      1,              },
	{ "status_cast_cancel",                 &battle_config.sc_castcancel,                   BL_NUL, BL_NUL, BL_ALL,         },
	{ "pc_status_def_rate",                 &battle_config.pc_sc_def_rate,                  100,    0,      INT_MAX,        },
	{ "mob_status_def_rate",                &battle_config.mob_sc_def_rate,                 100,    0,      INT_MAX,        },
	{ "pc_max_status_def",                  &battle_config.pc_max_sc_def,                   100,    0,      INT_MAX,        },
	{ "mob_max_status_def",                 &battle_config.mob_max_sc_def,                  100,    0,      INT_MAX,        },
	{ "sg_miracle_skill_ratio",             &battle_config.sg_miracle_skill_ratio,          1,      0,      10000,          },
	{ "sg_angel_skill_ratio",               &battle_config.sg_angel_skill_ratio,            10,     0,      10000,          },
	{ "autospell_stacking",                 &battle_config.autospell_stacking,              0,      0,      1,              },
	{ "override_mob_names",                 &battle_config.override_mob_names,              0,      0,      2,              },
	{ "min_chat_delay",                     &battle_config.min_chat_delay,                  0,      0,      INT_MAX,        },
	{ "friend_auto_add",                    &battle_config.friend_auto_add,                 1,      0,      1,              },
	{ "hom_rename",                         &battle_config.hom_rename,                      0,      0,      1,              },
	{ "homunculus_show_growth",             &battle_config.homunculus_show_growth,          0,      0,      1,              },
	{ "homunculus_friendly_rate",           &battle_config.homunculus_friendly_rate,        100,    0,      INT_MAX,        },
	{ "vending_tax",                        &battle_config.vending_tax,                     0,      0,      10000,          },
	{ "day_duration",                       &battle_config.day_duration,                    0,      0,      INT_MAX,        },
	{ "night_duration",                     &battle_config.night_duration,                  0,      0,      INT_MAX,        },
	{ "mob_remove_delay",                   &battle_config.mob_remove_delay,                60000,  1000,   INT_MAX,        },
	{ "mob_active_time",                    &battle_config.mob_active_time,                 0,      0,      INT_MAX,        },
	{ "boss_active_time",                   &battle_config.boss_active_time,                0,      0,      INT_MAX,        },
	{ "sg_miracle_skill_duration",          &battle_config.sg_miracle_skill_duration,       3600000, 0,     INT_MAX,        },
	{ "hvan_explosion_intimate",            &battle_config.hvan_explosion_intimate,         45000,  0,      100000,         },
	{ "quest_exp_rate",                     &battle_config.quest_exp_rate,                  100,    0,      INT_MAX,        },
	{ "at_mapflag",                         &battle_config.autotrade_mapflag,               0,      0,      1,              },
	{ "at_timeout",                         &battle_config.at_timeout,                      0,      0,      INT_MAX,        },
	{ "homunculus_autoloot",                &battle_config.homunculus_autoloot,             0,      0,      1,              },
	{ "idle_no_autoloot",                   &battle_config.idle_no_autoloot,                0,      0,      INT_MAX,        },
	{ "max_guild_alliance",                 &battle_config.max_guild_alliance,              3,      0,      3,              },
	{ "ksprotection",                       &battle_config.ksprotection,                    5000,   0,      INT_MAX,        },
	{ "auction_feeperhour",                 &battle_config.auction_feeperhour,              12000,  0,      INT_MAX,        },
	{ "auction_maximumprice",               &battle_config.auction_maximumprice,            500000000, 0,   MAX_ZENY,       },
	{ "homunculus_auto_vapor",              &battle_config.homunculus_auto_vapor,           1,      0,      1,              },
	{ "display_status_timers",              &battle_config.display_status_timers,           1,      0,      1,              },
	{ "skill_add_heal_rate",                &battle_config.skill_add_heal_rate,             7,      0,      INT_MAX,        },
	{ "eq_single_target_reflectable",       &battle_config.eq_single_target_reflectable,    1,      0,      1,              },
	{ "invincible.nodamage",                &battle_config.invincible_nodamage,             0,      0,      1,              },
	{ "mob_slave_keep_target",              &battle_config.mob_slave_keep_target,           0,      0,      1,              },
	{ "autospell_check_range",              &battle_config.autospell_check_range,           0,      0,      1,              },
	{ "knockback_left",                     &battle_config.knockback_left,                  1,      0,      1,              },
	{ "client_reshuffle_dice",              &battle_config.client_reshuffle_dice,           0,      0,      1,              },
	{ "client_sort_storage",                &battle_config.client_sort_storage,             0,      0,      1,              },
	{ "feature.buying_store",               &battle_config.feature_buying_store,            1,      0,      1,              },
	{ "feature.search_stores",              &battle_config.feature_search_stores,           1,      0,      1,              },
	{ "searchstore_querydelay",             &battle_config.searchstore_querydelay,         10,      0,      INT_MAX,        },
	{ "searchstore_maxresults",             &battle_config.searchstore_maxresults,         30,      1,      INT_MAX,        },
	{ "display_party_name",                 &battle_config.display_party_name,              0,      0,      1,              },
	{ "cashshop_show_points",               &battle_config.cashshop_show_points,            0,      0,      1,              },
	{ "mail_show_status",                   &battle_config.mail_show_status,                0,      0,      2,              },
	{ "client_limit_unit_lv",               &battle_config.client_limit_unit_lv,            0,      0,      BL_ALL,         },
	{ "client_emblem_max_blank_percent",    &battle_config.client_emblem_max_blank_percent, 100,    0,      100,            },
	// BattleGround Settings
	{ "bg_update_interval",                 &battle_config.bg_update_interval,              1000,   100,    INT_MAX,        },
	{ "bg_flee_penalty",                    &battle_config.bg_flee_penalty,                 20,     0,      INT_MAX,        },
	/**
	 * rAthena
	 **/
	{ "max_third_parameter",                &battle_config.max_third_parameter,             130,    10,     10000,          },
	{ "max_baby_third_parameter",           &battle_config.max_baby_third_parameter,        117,    10,     10000,          },
	{ "max_extended_parameter",             &battle_config.max_extended_parameter,          125,    10,     10000,          },
	{ "atcommand_max_stat_bypass",          &battle_config.atcommand_max_stat_bypass,       0,      0,      100,            },
	{ "skill_amotion_leniency",             &battle_config.skill_amotion_leniency,          90,     0,      300             },
	{ "mvp_tomb_enabled",                   &battle_config.mvp_tomb_enabled,                1,      0,      1               },
	{ "feature.atcommand_suggestions",      &battle_config.atcommand_suggestions_enabled,   0,      0,      1               },
	{ "min_npc_vendchat_distance",          &battle_config.min_npc_vendchat_distance,       3,      0,      100             },
	{ "vendchat_near_hiddennpc",            &battle_config.vendchat_near_hiddennpc,         0,      0,      1               },
	{ "atcommand_mobinfo_type",             &battle_config.atcommand_mobinfo_type,          0,      0,      1               },
	{ "homunculus_max_level",               &battle_config.hom_max_level,                   99,     0,      MAX_LEVEL,      },
	{ "homunculus_S_max_level",             &battle_config.hom_S_max_level,                 150,    0,      MAX_LEVEL,      },
	{ "mob_size_influence",                 &battle_config.mob_size_influence,              0,      0,      1,              },
	{ "bowling_bash_area",                  &battle_config.bowling_bash_area,               0,      0,      20,             },
	/**
	 * Hercules
	 **/
	{ "skill_trap_type",                    &battle_config.skill_trap_type,                 0,      0,      1,              },
	{ "item_restricted_consumption_type",   &battle_config.item_restricted_consumption_type,1,      0,      1,              },
	{ "unequip_restricted_equipment",       &battle_config.unequip_restricted_equipment,    0,      0,      3,              },
	{ "max_walk_path",                      &battle_config.max_walk_path,                   17,     1,      MAX_WALKPATH,   },
	{ "item_enabled_npc",                   &battle_config.item_enabled_npc,                1,      0,      1,              },
	{ "gm_ignore_warpable_area",            &battle_config.gm_ignore_warpable_area,         0,      2,      100,            },
	{ "packet_obfuscation",                 &battle_config.packet_obfuscation,              1,      0,      3,              },
	{ "client_accept_chatdori",             &battle_config.client_accept_chatdori,          0,      0,      INT_MAX,        },
	{ "snovice_call_type",                  &battle_config.snovice_call_type,               0,      0,      1,              },
	{ "guild_notice_changemap",             &battle_config.guild_notice_changemap,          2,      0,      2,              },
	{ "feature.banking",                    &battle_config.feature_banking,                 1,      0,      1,              },
	{ "feature.auction",                    &battle_config.feature_auction,                 0,      0,      2,              },
	{ "idletime_criteria",                  &battle_config.idletime_criteria,            0x25,      1,      INT_MAX,        },
	{ "mon_trans_disable_in_gvg",           &battle_config.mon_trans_disable_in_gvg,        0,      0,      1,              },
	{ "case_sensitive_aegisnames",          &battle_config.case_sensitive_aegisnames,       1,      0,      1,              },
	{ "guild_castle_invite",                &battle_config.guild_castle_invite,             0,      0,      1,              },
	{ "guild_castle_expulsion",             &battle_config.guild_castle_expulsion,          0,      0,      1,              },
	{ "song_timer_reset",                   &battle_config.song_timer_reset,                0,      0,      1,              },
	{ "snap_dodge",                         &battle_config.snap_dodge,                      0,      0,      1,              },
	{ "stormgust_knockback",                &battle_config.stormgust_knockback,             1,      0,      1,              },
	{ "monster_chase_refresh",              &battle_config.mob_chase_refresh,               1,      0,      30,             },
	{ "mob_icewall_walk_block",             &battle_config.mob_icewall_walk_block,          75,     0,      255,            },
	{ "boss_icewall_walk_block",            &battle_config.boss_icewall_walk_block,         0,      0,      255,            },
	{ "feature.roulette",                   &battle_config.feature_roulette,                1,      0,      1,              },
	{ "show_monster_hp_bar",                &battle_config.show_monster_hp_bar,             1,      0,      1,              },
	{ "fix_warp_hit_delay_abuse",           &battle_config.fix_warp_hit_delay_abuse,        0,      0,      1,              },
	{ "costume_refine_def",                 &battle_config.costume_refine_def,              1,      0,      1,              },
	{ "shadow_refine_def",                  &battle_config.shadow_refine_def,               1,      0,      1,              },
	{ "shadow_refine_atk",                  &battle_config.shadow_refine_atk,               1,      0,      1,              },
	{ "min_body_style",                     &battle_config.min_body_style,                  0,      0,      SHRT_MAX,       },
	{ "max_body_style",                     &battle_config.max_body_style,                  4,      0,      SHRT_MAX,       },
	{ "save_body_style",                    &battle_config.save_body_style,                 0,      0,      1,              },
};
#ifndef STATS_OPT_OUT
/**
 * Hercules anonymous statistic usage report -- packet is built here, and sent to char server to report.
 **/
void Hercules_report(char* date, char *time_c) {
	int i, bd_size = ARRAYLENGTH(battle_data);
	unsigned int config = 0;
	char timestring[25];
	time_t curtime;
	char* buf;

	enum config_table {
		C_CIRCULAR_AREA         = 0x0001,
		C_CELLNOSTACK           = 0x0002,
		C_CONSOLE_INPUT         = 0x0004,
		C_SCRIPT_CALLFUNC_CHECK = 0x0008,
		C_OFFICIAL_WALKPATH     = 0x0010,
		C_RENEWAL               = 0x0020,
		C_RENEWAL_CAST          = 0x0040,
		C_RENEWAL_DROP          = 0x0080,
		C_RENEWAL_EXP           = 0x0100,
		C_RENEWAL_LVDMG         = 0x0200,
		C_RENEWAL_EDP           = 0x0400,
		C_RENEWAL_ASPD          = 0x0800,
		C_SECURE_NPCTIMEOUT     = 0x1000,
		//C_SQL_DB_ITEM           = 0x2000,
		C_SQL_LOGS              = 0x4000,
		C_MEMWATCH              = 0x8000,
		C_DMALLOC               = 0x10000,
		C_GCOLLECT              = 0x20000,
		C_SEND_SHORTLIST        = 0x40000,
		//C_SQL_DB_MOB            = 0x80000,
		//C_SQL_DB_MOBSKILL       = 0x100000,
		C_PACKETVER_RE          = 0x200000,
	};

	/* we get the current time */
	time(&curtime);
	strftime(timestring, 24, "%Y-%m-%d %H:%M:%S", localtime(&curtime));

#ifdef CIRCULAR_AREA
	config |= C_CIRCULAR_AREA;
#endif

#ifdef CELL_NOSTACK
	config |= C_CELLNOSTACK;
#endif

#ifdef CONSOLE_INPUT
	config |= C_CONSOLE_INPUT;
#endif

#ifdef SCRIPT_CALLFUNC_CHECK
	config |= C_SCRIPT_CALLFUNC_CHECK;
#endif

#ifdef OFFICIAL_WALKPATH
	config |= C_OFFICIAL_WALKPATH;
#endif

#ifdef RENEWAL
	config |= C_RENEWAL;
#endif

#ifdef RENEWAL_CAST
	config |= C_RENEWAL_CAST;
#endif

#ifdef RENEWAL_DROP
	config |= C_RENEWAL_DROP;
#endif

#ifdef RENEWAL_EXP
	config |= C_RENEWAL_EXP;
#endif

#ifdef RENEWAL_LVDMG
	config |= C_RENEWAL_LVDMG;
#endif

#ifdef RENEWAL_EDP
	config |= C_RENEWAL_EDP;
#endif

#ifdef RENEWAL_ASPD
	config |= C_RENEWAL_ASPD;
#endif

#ifdef SECURE_NPCTIMEOUT
	config |= C_SECURE_NPCTIMEOUT;
#endif

#ifdef PACKETVER_RE
	config |= C_PACKETVER_RE;
#endif

	/* non-define part */
	if( logs->config.sql_logs )
		config |= C_SQL_LOGS;

#ifdef MEMWATCH
	config |= C_MEMWATCH;
#endif
#ifdef DMALLOC
	config |= C_DMALLOC;
#endif
#ifdef GCOLLECT
	config |= C_GCOLLECT;
#endif

#ifdef SEND_SHORTLIST
	config |= C_SEND_SHORTLIST;
#endif

#define BFLAG_LENGTH 35

	CREATE(buf, char, 262 + ( bd_size * ( BFLAG_LENGTH + 4 ) ) + 1 );

	/* build packet */

	WBUFW(buf,0) = 0x3000;
	WBUFW(buf,2) = 262 + ( bd_size * ( BFLAG_LENGTH + 4 ) );
	WBUFW(buf,4) = 0x9f;

	safestrncpy(WBUFP(buf,6), date, 12);
	safestrncpy(WBUFP(buf,18), time_c, 9);
	safestrncpy(WBUFP(buf,27), timestring, 24);

	safestrncpy(WBUFP(buf,51), sysinfo->platform(), 16);
	safestrncpy(WBUFP(buf,67), sysinfo->osversion(), 50);
	safestrncpy(WBUFP(buf,117), sysinfo->cpu(), 32);
	WBUFL(buf,149) = sysinfo->cpucores();
	safestrncpy(WBUFP(buf,153), sysinfo->arch(), 8);
	WBUFB(buf,161) = sysinfo->vcstypeid();
	WBUFB(buf,162) = sysinfo->is64bit();
	safestrncpy(WBUFP(buf,163), sysinfo->vcsrevision_src(), 41);
	safestrncpy(WBUFP(buf,204), sysinfo->vcsrevision_scripts(), 41);
	WBUFB(buf,245) = (sysinfo->is_superuser()? 1 : 0);
	WBUFL(buf,246) = map->getusers();

	WBUFL(buf,250) = config;
	WBUFL(buf,254) = PACKETVER;

	WBUFL(buf,258) = bd_size;
	for( i = 0; i < bd_size; i++ ) {
		safestrncpy(WBUFP(buf,262 + ( i * ( BFLAG_LENGTH + 4 ) ) ), battle_data[i].str, BFLAG_LENGTH);
		WBUFL(buf,262 + BFLAG_LENGTH + ( i * ( BFLAG_LENGTH + 4 )  )  ) = *battle_data[i].val;
	}

	chrif->send_report(buf, 262 + ( bd_size * ( BFLAG_LENGTH + 4 ) ) );

	aFree(buf);

#undef BFLAG_LENGTH
}
static int Hercules_report_timer(int tid, int64 tick, int id, intptr_t data) {
	if( chrif->isconnected() ) {/* char server relays it, so it must be online. */
		Hercules_report(__DATE__,__TIME__);
	}
	return 0;
}
#endif

int battle_set_value(const char* w1, const char* w2)
{
	int val = config_switch(w2);
	int i;

	nullpo_retr(1, w1);
	nullpo_retr(1, w2);
	ARR_FIND(0, ARRAYLENGTH(battle_data), i, strcmpi(w1, battle_data[i].str) == 0);
	if (i == ARRAYLENGTH(battle_data)) {
		if( HPM->parseConf(w1,w2,HPCT_BATTLE) ) /* if plugin-owned, succeed */
			return 1;
		return 0; // not found
	}

	if (val < battle_data[i].min || val > battle_data[i].max)
	{
		ShowWarning("Value for setting '%s': %s is invalid (min:%i max:%i)! Defaulting to %i...\n", w1, w2, battle_data[i].min, battle_data[i].max, battle_data[i].defval);
		val = battle_data[i].defval;
	}

	*battle_data[i].val = val;
	return 1;
}

bool battle_get_value(const char *w1, int *value)
{
	int i;

	nullpo_retr(false, w1);
	nullpo_retr(false, value);

	ARR_FIND(0, ARRAYLENGTH(battle_data), i, strcmpi(w1, battle_data[i].str) == 0);
	if (i == ARRAYLENGTH(battle_data)) {
		if (HPM->getBattleConf(w1,value))
			return true;
	} else {
		*value = *battle_data[i].val;
		return true;
	}

	return false;
}

void battle_set_defaults(void) {
	int i;
	for (i = 0; i < ARRAYLENGTH(battle_data); i++)
		*battle_data[i].val = battle_data[i].defval;
}

void battle_adjust_conf(void) {
	battle_config.monster_max_aspd = 2000 - battle_config.monster_max_aspd*10;
	battle_config.max_aspd = 2000 - battle_config.max_aspd*10;
	battle_config.max_third_aspd = 2000 - battle_config.max_third_aspd*10;
	battle_config.max_walk_speed = 100*DEFAULT_WALK_SPEED/battle_config.max_walk_speed;
	battle_config.max_cart_weight *= 10;

	if(battle_config.max_def > 100 && !battle_config.weapon_defense_type) // added by [Skotlex]
		battle_config.max_def = 100;

	if(battle_config.min_hitrate > battle_config.max_hitrate)
		battle_config.min_hitrate = battle_config.max_hitrate;

	if(battle_config.pet_max_atk1 > battle_config.pet_max_atk2) //Skotlex
		battle_config.pet_max_atk1 = battle_config.pet_max_atk2;

	if (battle_config.day_duration && battle_config.day_duration < 60000) // added by [Yor]
		battle_config.day_duration = 60000;
	if (battle_config.night_duration && battle_config.night_duration < 60000) // added by [Yor]
		battle_config.night_duration = 60000;

#if PACKETVER < 20100427
	if( battle_config.feature_buying_store ) {
		ShowWarning("conf/battle/feature.conf buying_store is enabled but it requires PACKETVER 2010-04-27 or newer, disabling...\n");
		battle_config.feature_buying_store = 0;
	}
#endif

#if PACKETVER < 20100803
	if( battle_config.feature_search_stores ) {
		ShowWarning("conf/battle/feature.conf search_stores is enabled but it requires PACKETVER 2010-08-03 or newer, disabling...\n");
		battle_config.feature_search_stores = 0;
	}
#endif

#if PACKETVER < 20130724
	if( battle_config.feature_banking ) {
		ShowWarning("conf/battle/feature.conf banking is enabled but it requires PACKETVER 2013-07-24 or newer, disabling...\n");
		battle_config.feature_banking = 0;
	}
#endif

#if PACKETVER < 20141022
	if( battle_config.feature_roulette ) {
		ShowWarning("conf/battle/feature.conf roulette is enabled but it requires PACKETVER 2014-10-22 or newer, disabling...\n");
		battle_config.feature_roulette = 0;
	}
#endif

#if PACKETVER > 20120000 && PACKETVER < 20130515 /* exact date (when it started) not known */
	if( battle_config.feature_auction == 1 ) {
		ShowWarning("conf/battle/feature.conf:feature.auction is enabled but it is not stable on PACKETVER "EXPAND_AND_QUOTE(PACKETVER)", disabling...\n");
		ShowWarning("conf/battle/feature.conf:feature.auction change value to '2' to silence this warning and maintain it enabled\n");
		battle_config.feature_auction = 0;
	}
#endif

#ifndef CELL_NOSTACK
	if (battle_config.custom_cell_stack_limit != 1)
		ShowWarning("Battle setting 'custom_cell_stack_limit' takes no effect as this server was compiled without Cell Stack Limit support.\n");
#endif
}

int battle_config_read(const char* cfgName)
{
	FILE* fp;
	static int count = 0;

	nullpo_ret(cfgName);

	if (count == 0)
		battle->config_set_defaults();

	count++;

	fp = fopen(cfgName,"r");
	if (fp == NULL)
		ShowError("File not found: %s\n", cfgName);
	else
	{
		char line[1024], w1[1024], w2[1024];
		while(fgets(line, sizeof(line), fp))
		{
			if (line[0] == '/' && line[1] == '/')
				continue;
			if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
				continue;
			if (strcmpi(w1, "import") == 0)
				battle->config_read(w2);
			else
			if (battle->config_set_value(w1, w2) == 0)
				ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}

		fclose(fp);
	}

	count--;

	if (count == 0) {
		battle->config_adjust();
		clif->bc_ready();
	}

	return 0;
}

void do_init_battle(bool minimal) {
	if (minimal)
		return;

	battle->delay_damage_ers = ers_new(sizeof(struct delay_damage),"battle.c::delay_damage_ers",ERS_OPT_CLEAR);
	timer->add_func_list(battle->delay_damage_sub, "battle_delay_damage_sub");

#ifndef STATS_OPT_OUT
	timer->add_func_list(Hercules_report_timer, "Hercules_report_timer");
	timer->add_interval(timer->gettick()+30000, Hercules_report_timer, 0, 0, 60000 * 30);
#endif

}

void do_final_battle(void) {
	ers_destroy(battle->delay_damage_ers);
}

/* initialize the interface */
void battle_defaults(void) {
	battle = &battle_s;

	battle->bc = &battle_config;

	memset(battle->attr_fix_table, 0, sizeof(battle->attr_fix_table));
	battle->delay_damage_ers = NULL;

	battle->init = do_init_battle;
	battle->final = do_final_battle;

	battle->calc_attack = battle_calc_attack;
	battle->calc_damage = battle_calc_damage;
	battle->calc_gvg_damage = battle_calc_gvg_damage;
	battle->calc_bg_damage = battle_calc_bg_damage;
	battle->weapon_attack = battle_weapon_attack;
	battle->calc_weapon_attack = battle_calc_weapon_attack;
	battle->delay_damage = battle_delay_damage;
	battle->drain = battle_drain;
	battle->reflect_damage = battle_reflect_damage;
	battle->attr_ratio = battle_attr_ratio;
	battle->attr_fix = battle_attr_fix;
	battle->calc_cardfix = battle_calc_cardfix;
	battle->calc_cardfix2 = battle_calc_cardfix2;
	battle->calc_elefix = battle_calc_elefix;
	battle->calc_masteryfix = battle_calc_masteryfix;
	battle->calc_chorusbonus = battle_calc_chorusbonus;
	battle->calc_skillratio = battle_calc_skillratio;
	battle->calc_sizefix = battle_calc_sizefix;
	battle->calc_weapon_damage = battle_calc_weapon_damage;
	battle->calc_defense = battle_calc_defense;
	battle->get_master = battle_get_master;
	battle->get_targeted = battle_gettargeted;
	battle->get_enemy = battle_getenemy;
	battle->get_target = battle_gettarget;
	battle->get_current_skill = battle_getcurrentskill;
	battle->check_undead = battle_check_undead;
	battle->check_target = battle_check_target;
	battle->check_range = battle_check_range;
	battle->consume_ammo = battle_consume_ammo;
	battle->get_targeted_sub = battle_gettargeted_sub;
	battle->get_enemy_sub = battle_getenemy_sub;
	battle->get_enemy_area_sub = battle_getenemyarea_sub;
	battle->delay_damage_sub = battle_delay_damage_sub;
	battle->blewcount_bonus = battle_blewcount_bonus;
	battle->range_type = battle_range_type;
	battle->calc_base_damage = battle_calc_base_damage;
	battle->calc_base_damage2 = battle_calc_base_damage2;
	battle->calc_misc_attack = battle_calc_misc_attack;
	battle->calc_magic_attack = battle_calc_magic_attack;
	battle->adjust_skill_damage = battle_adjust_skill_damage;
	battle->add_mastery = battle_addmastery;
	battle->calc_drain = battle_calc_drain;
	battle->config_read = battle_config_read;
	battle->config_set_defaults = battle_set_defaults;
	battle->config_set_value = battle_set_value;
	battle->config_get_value = battle_get_value;
	battle->config_adjust = battle_adjust_conf;
	battle->get_enemy_area = battle_getenemyarea;
	battle->damage_area = battle_damage_area;
	battle->calc_masteryfix_unknown = battle_calc_masteryfix_unknown;
	battle->calc_skillratio_magic_unknown = battle_calc_skillratio_magic_unknown;
	battle->calc_skillratio_weapon_unknown = battle_calc_skillratio_weapon_unknown;
	battle->calc_misc_attack_unknown = battle_calc_misc_attack_unknown;
}
