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

#include "config/core.h" // DBPATH, MAGIC_REFLECTION_TYPE, OFFICIAL_WALKPATH, RENEWAL, RENEWAL_CAST, VARCAST_REDUCTION()
#include "skill.h"

#include "map/battle.h"
#include "map/battleground.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/date.h"
#include "map/elemental.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h"
#include "map/mercenary.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/script.h"
#include "map/status.h"
#include "map/unit.h"
#include "common/cbasetypes.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SKILLUNITTIMER_INTERVAL 100

// ranges reserved for mapping skill ids to skilldb offsets
#define HM_SKILLRANGEMIN 750
#define HM_SKILLRANGEMAX (HM_SKILLRANGEMIN + MAX_HOMUNSKILL)
#define MC_SKILLRANGEMIN (HM_SKILLRANGEMAX + 1)
#define MC_SKILLRANGEMAX (MC_SKILLRANGEMIN + MAX_MERCSKILL)
#define EL_SKILLRANGEMIN (MC_SKILLRANGEMAX + 1)
#define EL_SKILLRANGEMAX (EL_SKILLRANGEMIN + MAX_ELEMENTALSKILL)
#define GD_SKILLRANGEMIN (EL_SKILLRANGEMAX + 1)
#define GD_SKILLRANGEMAX (GD_SKILLRANGEMIN + MAX_GUILDSKILL)

#if GD_SKILLRANGEMAX > 999
	#error GD_SKILLRANGEMAX is greater than 999
#endif

struct skill_interface skill_s;
struct s_skill_dbs skilldbs;

struct skill_interface *skill;

//Since only mob-casted splash skills can hit ice-walls
static inline int splash_target(struct block_list* bl) {
#ifndef RENEWAL
	return ( bl->type == BL_MOB ) ? BL_SKILL|BL_CHAR : BL_CHAR;
#else // Some skills can now hit ground skills(traps, ice wall & etc.)
	return BL_SKILL|BL_CHAR;
#endif
}

/// Returns the id of the skill, or 0 if not found.
int skill_name2id(const char* name) {
	if( name == NULL )
		return 0;

	return strdb_iget(skill->name2id_db, name);
}

/// Maps skill ids to skill db offsets.
/// Returns the skill's array index, or 0 (Unknown Skill).
int skill_get_index( uint16 skill_id ) {
	// avoid ranges reserved for mapping guild/homun/mercenary skills
	if( (skill_id >= GD_SKILLRANGEMIN && skill_id <= GD_SKILLRANGEMAX)
	||  (skill_id >= HM_SKILLRANGEMIN && skill_id <= HM_SKILLRANGEMAX)
	||  (skill_id >= MC_SKILLRANGEMIN && skill_id <= MC_SKILLRANGEMAX)
	||  (skill_id >= EL_SKILLRANGEMIN && skill_id <= EL_SKILLRANGEMAX) )
		return 0;

	// map skill id to skill db index
	if( skill_id >= GD_SKILLBASE )
		skill_id = GD_SKILLRANGEMIN + skill_id - GD_SKILLBASE;
	else if( skill_id >= EL_SKILLBASE )
		skill_id = EL_SKILLRANGEMIN + skill_id - EL_SKILLBASE;
	else if( skill_id >= MC_SKILLBASE )
		skill_id = MC_SKILLRANGEMIN + skill_id - MC_SKILLBASE;
	else if( skill_id >= HM_SKILLBASE )
		skill_id = HM_SKILLRANGEMIN + skill_id - HM_SKILLBASE;
	//[Ind/Hercules] GO GO GO LESS! - http://herc.ws/board/topic/512-skill-id-processing-overhaul/
	else if( skill_id > 1019 && skill_id < 8001 ) {
		if( skill_id < 2058 ) // 1020 - 2000 are empty
			skill_id = 1020 + skill_id - 2001;
		else if( skill_id < 2549 ) // 2058 - 2200 are empty - 1020+57
			skill_id = (1077) + skill_id - 2201;
		else if ( skill_id < 3036 ) // 2549 - 3000 are empty - 1020+57+348
			skill_id = (1425) + skill_id - 3001;
		else if ( skill_id < 5019 ) // 3036 - 5000 are empty - 1020+57+348+35
			skill_id = (1460) + skill_id - 5001;
		else
			ShowWarning("skill_get_index: skill id '%d' is not being handled!\n",skill_id);
	}

	// validate result
	if( !skill_id || skill_id >= MAX_SKILL_DB )
		return 0;

	return skill_id;
}

const char* skill_get_name( uint16 skill_id ) {
	return skill->dbs->db[skill->get_index(skill_id)].name;
}

const char* skill_get_desc( uint16 skill_id ) {
	return skill->dbs->db[skill->get_index(skill_id)].desc;
}

// out of bounds error checking [celest]
void skill_chk(uint16* skill_id) {
	*skill_id = skill->get_index(*skill_id); // checks/adjusts id
}

#define skill_get(var,id) do { skill->chk(&(id)); if(!(id)) return 0; return (var); } while(0)
#define skill_get2(var,id,lv) do { \
	skill->chk(&(id)); \
	if(!(id)) return 0; \
	if( (lv) > MAX_SKILL_LEVEL && (var) > 1 ) { \
		int lv2__ = (lv); (lv) = skill->dbs->db[(id)].max; \
		return (var) + ((lv2__-(lv))/2);\
	} \
	return (var);\
} while(0)
#define skill_glv(lv) min((lv),MAX_SKILL_LEVEL-1)
// Skill DB
int skill_get_hit( uint16 skill_id )               { skill_get (skill->dbs->db[skill_id].hit, skill_id); }
int skill_get_inf( uint16 skill_id )               { skill_get (skill->dbs->db[skill_id].inf, skill_id); }
int skill_get_ele( uint16 skill_id , uint16 skill_lv )      { Assert_ret(skill_lv > 0); skill_get (skill->dbs->db[skill_id].element[skill_glv(skill_lv-1)], skill_id); }
int skill_get_nk( uint16 skill_id )                { skill_get (skill->dbs->db[skill_id].nk, skill_id); }
int skill_get_max( uint16 skill_id )               { skill_get (skill->dbs->db[skill_id].max, skill_id); }
int skill_get_range( uint16 skill_id , uint16 skill_lv )    { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].range[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_splash( uint16 skill_id , uint16 skill_lv )   { Assert_ret(skill_lv > 0); skill_get2 ( (skill->dbs->db[skill_id].splash[skill_glv(skill_lv-1)]>=0?skill->dbs->db[skill_id].splash[skill_glv(skill_lv-1)]:AREA_SIZE), skill_id, skill_lv);  }
int skill_get_hp( uint16 skill_id ,uint16 skill_lv )        { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].hp[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_sp( uint16 skill_id ,uint16 skill_lv )        { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].sp[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_hp_rate(uint16 skill_id, uint16 skill_lv )    { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].hp_rate[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_sp_rate(uint16 skill_id, uint16 skill_lv )    { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].sp_rate[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_state(uint16 skill_id)               { skill_get (skill->dbs->db[skill_id].state, skill_id); }
int skill_get_spiritball(uint16 skill_id, uint16 skill_lv)  { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].spiritball[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_itemid(uint16 skill_id, int idx)     { skill_get (skill->dbs->db[skill_id].itemid[idx], skill_id); }
int skill_get_itemqty(uint16 skill_id, int idx)    { skill_get (skill->dbs->db[skill_id].amount[idx], skill_id); }
int skill_get_zeny( uint16 skill_id ,uint16 skill_lv )      { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].zeny[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_num( uint16 skill_id ,uint16 skill_lv )       { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].num[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_cast( uint16 skill_id ,uint16 skill_lv )      { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].cast[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_delay( uint16 skill_id ,uint16 skill_lv )     { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].delay[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_walkdelay( uint16 skill_id ,uint16 skill_lv ) { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].walkdelay[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_time( uint16 skill_id ,uint16 skill_lv )      { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].upkeep_time[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_time2( uint16 skill_id ,uint16 skill_lv )     { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].upkeep_time2[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_castdef( uint16 skill_id )           { skill_get (skill->dbs->db[skill_id].cast_def_rate, skill_id); }
int skill_get_weapontype( uint16 skill_id )        { skill_get (skill->dbs->db[skill_id].weapon, skill_id); }
int skill_get_ammotype( uint16 skill_id )          { skill_get (skill->dbs->db[skill_id].ammo, skill_id); }
int skill_get_ammo_qty( uint16 skill_id, uint16 skill_lv )  { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].ammo_qty[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_inf2( uint16 skill_id )              { skill_get (skill->dbs->db[skill_id].inf2, skill_id); }
int skill_get_castcancel( uint16 skill_id )        { skill_get (skill->dbs->db[skill_id].castcancel, skill_id); }
int skill_get_maxcount( uint16 skill_id ,uint16 skill_lv )  { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].maxcount[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_blewcount( uint16 skill_id ,uint16 skill_lv ) { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].blewcount[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_mhp( uint16 skill_id ,uint16 skill_lv )       { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].mhp[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_castnodex( uint16 skill_id ,uint16 skill_lv ) { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].castnodex[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_delaynodex( uint16 skill_id ,uint16 skill_lv ){ Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].delaynodex[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_type( uint16 skill_id )              { skill_get (skill->dbs->db[skill_id].skill_type, skill_id); }
int skill_get_unit_id ( uint16 skill_id, int flag ){ skill_get (skill->dbs->db[skill_id].unit_id[flag], skill_id); }
int skill_get_unit_interval( uint16 skill_id )     { skill_get (skill->dbs->db[skill_id].unit_interval, skill_id); }
int skill_get_unit_range( uint16 skill_id, uint16 skill_lv ) { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].unit_range[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_unit_target( uint16 skill_id )       { skill_get (skill->dbs->db[skill_id].unit_target&BCT_ALL, skill_id); }
int skill_get_unit_bl_target( uint16 skill_id )    { skill_get (skill->dbs->db[skill_id].unit_target&BL_ALL, skill_id); }
int skill_get_unit_flag( uint16 skill_id )         { skill_get (skill->dbs->db[skill_id].unit_flag, skill_id); }
int skill_get_unit_layout_type( uint16 skill_id ,uint16 skill_lv ){ Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].unit_layout_type[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_cooldown( uint16 skill_id, uint16 skill_lv )     { Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].cooldown[skill_glv(skill_lv-1)], skill_id, skill_lv); }
int skill_get_fixed_cast( uint16 skill_id ,uint16 skill_lv ) {
#ifdef RENEWAL_CAST
	Assert_ret(skill_lv > 0); skill_get2 (skill->dbs->db[skill_id].fixed_cast[skill_glv(skill_lv-1)], skill_id, skill_lv);
#else
	return 0;
#endif
}
int skill_tree_get_max(uint16 skill_id, int b_class)
{
	int i;
	b_class = pc->class2idx(b_class);

	ARR_FIND( 0, MAX_SKILL_TREE, i, pc->skill_tree[b_class][i].id == 0 || pc->skill_tree[b_class][i].id == skill_id );
	if( i < MAX_SKILL_TREE && pc->skill_tree[b_class][i].id == skill_id )
		return pc->skill_tree[b_class][i].max;
	else
		return skill->get_max(skill_id);
}

int skill_get_casttype (uint16 skill_id) {
	int inf = skill->get_inf(skill_id);
	if (inf&(INF_GROUND_SKILL))
		return CAST_GROUND;
	if (inf&INF_SUPPORT_SKILL)
		return CAST_NODAMAGE;
	if (inf&INF_SELF_SKILL) {
		if(skill->get_inf2(skill_id)&INF2_NO_TARGET_SELF)
			return CAST_DAMAGE; //Combo skill.
		return CAST_NODAMAGE;
	}
	if (skill->get_nk(skill_id)&NK_NO_DAMAGE)
		return CAST_NODAMAGE;
	return CAST_DAMAGE;
}

int skill_get_casttype2 (uint16 index) {
	int inf = skill->dbs->db[index].inf;
	if (inf&(INF_GROUND_SKILL))
		return CAST_GROUND;
	if (inf&INF_SUPPORT_SKILL)
		return CAST_NODAMAGE;
	if (inf&INF_SELF_SKILL) {
		if(skill->dbs->db[index].inf2&INF2_NO_TARGET_SELF)
			return CAST_DAMAGE; //Combo skill.
		return CAST_NODAMAGE;
	}
	if (skill->dbs->db[index].nk&NK_NO_DAMAGE)
		return CAST_NODAMAGE;
	return CAST_DAMAGE;
}

//Returns actual skill range taking into account attack range and AC_OWL [Skotlex]
int skill_get_range2 (struct block_list *bl, uint16 skill_id, uint16 skill_lv) {
	int range;
	struct map_session_data *sd = BL_CAST(BL_PC, bl);
	if( bl->type == BL_MOB && battle_config.mob_ai&0x400 )
		return 9; //Mobs have a range of 9 regardless of skill used.

	range = skill->get_range(skill_id, skill_lv);

	if( range < 0 ) {
		if( battle_config.use_weapon_skill_range&bl->type )
			return status_get_range(bl);
		range *=-1;
	}

	//TODO: Find a way better than hardcoding the list of skills affected by AC_VULTURE
	switch( skill_id ) {
		case AC_SHOWER:
		case MA_SHOWER:
		case AC_DOUBLE:
		case MA_DOUBLE:
		case HT_BLITZBEAT:
		case AC_CHARGEARROW:
		case MA_CHARGEARROW:
		case SN_FALCONASSAULT:
		case HT_POWER:
		/**
		 * Ranger
		 **/
		case RA_ARROWSTORM:
		case RA_AIMEDBOLT:
		case RA_WUGBITE:
			if (sd != NULL)
				range += pc->checkskill(sd, AC_VULTURE);
			else
				range += 10; //Assume level 10?
			break;
		// added to allow GS skills to be effected by the range of Snake Eyes [Reddozen]
		case GS_RAPIDSHOWER:
		case GS_PIERCINGSHOT:
		case GS_FULLBUSTER:
		case GS_SPREADATTACK:
		case GS_GROUNDDRIFT:
			if (sd != NULL)
				range += pc->checkskill(sd, GS_SNAKEEYE);
			else
				range += 10; //Assume level 10?
			break;
		case NJ_KIRIKAGE:
			if (sd != NULL)
				range = skill->get_range(NJ_SHADOWJUMP, pc->checkskill(sd, NJ_SHADOWJUMP));
			break;
		/**
		 * Warlock
		 **/
		case WL_WHITEIMPRISON:
		case WL_SOULEXPANSION:
		case WL_MARSHOFABYSS:
		case WL_SIENNAEXECRATE:
		case WL_DRAINLIFE:
		case WL_CRIMSONROCK:
		case WL_HELLINFERNO:
		case WL_COMET:
		case WL_CHAINLIGHTNING:
		case WL_TETRAVORTEX:
		case WL_EARTHSTRAIN:
		case WL_RELEASE:
			if (sd != NULL)
				range += pc->checkskill(sd, WL_RADIUS);
			break;
		/**
		 * Ranger Bonus
		 **/
		case HT_LANDMINE:
		case HT_FREEZINGTRAP:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
		case RA_CLUSTERBOMB:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			if (sd != NULL)
				range += (1 + pc->checkskill(sd, RA_RESEARCHTRAP))/2;
	}

	if( !range && bl->type != BL_PC )
		return 9; // Enable non players to use self skills on others. [Skotlex]
	return range;
}

int skill_calc_heal(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, bool heal) {
	int skill2_lv, hp;
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_change* sc;

	nullpo_ret(src);

	switch( skill_id ) {
		case BA_APPLEIDUN:
#ifdef RENEWAL
			hp = 100+5*skill_lv+5*(status_get_vit(src)/10); // HP recovery
#else // not RENEWAL
			hp = 30+5*skill_lv+5*(status_get_vit(src)/10); // HP recovery
#endif // RENEWAL
			if( sd )
				hp += 5*pc->checkskill(sd,BA_MUSICALLESSON);
			break;
		case PR_SANCTUARY:
			hp = (skill_lv>6)?777:skill_lv*100;
			break;
		case NPC_EVILLAND:
			hp = (skill_lv>6)?666:skill_lv*100;
			break;
		default:
			if (skill_lv >= battle_config.max_heal_lv)
				return battle_config.max_heal;
#ifdef RENEWAL
			/**
			 * Renewal Heal Formula
			 * Formula: ( [(Base Level + INT) / 5] ? 30 ) ? (Heal Level / 10) ? (Modifiers) + MATK
			 **/
			hp = (status->get_lv(src) + status_get_int(src)) / 5 * 30  * skill_lv / 10;
#else // not RENEWAL
			hp = ( status->get_lv(src) + status_get_int(src) ) / 8 * (4 + ( skill_id == AB_HIGHNESSHEAL ? ( sd ? pc->checkskill(sd,AL_HEAL) : 10 ) : skill_lv ) * 8);
#endif // RENEWAL
			if (sd && (skill2_lv = pc->checkskill(sd, HP_MEDITATIO)) > 0)
				hp += hp * skill2_lv * 2 / 100;
			else if (src->type == BL_HOM && (skill2_lv = homun->checkskill(BL_UCAST(BL_HOM, src), HLIF_BRAIN)) > 0)
				hp += hp * skill2_lv * 2 / 100;
			break;
	}

	if( ( (target && target->type == BL_MER) || !heal ) && skill_id != NPC_EVILLAND )
		hp >>= 1;

	if (sd && (skill2_lv = pc->skillheal_bonus(sd, skill_id)) != 0)
		hp += hp*skill2_lv/100;

	if (tsd && (skill2_lv = pc->skillheal2_bonus(tsd, skill_id)) != 0)
		hp += hp*skill2_lv/100;

	sc = status->get_sc(src);
	if( sc && sc->count && sc->data[SC_OFFERTORIUM] ) {
		if( skill_id == AB_HIGHNESSHEAL || skill_id == AB_CHEAL || skill_id == PR_SANCTUARY || skill_id == AL_HEAL )
			hp += hp * sc->data[SC_OFFERTORIUM]->val2 / 100;
	}
	sc = status->get_sc(target);
	if (sc && sc->count) {
		if(sc->data[SC_CRITICALWOUND] && heal) // Critical Wound has no effect on offensive heal. [Inkfish]
			hp -= hp * sc->data[SC_CRITICALWOUND]->val2/100;
		if(sc->data[SC_DEATHHURT] && heal)
			hp -= hp * 20/100;
		if(sc->data[SC_HEALPLUS] && skill_id != NPC_EVILLAND && skill_id != BA_APPLEIDUN)
			hp += hp * sc->data[SC_HEALPLUS]->val1/100; // Only affects Heal, Sanctuary and PotionPitcher.(like bHealPower) [Inkfish]
		if(sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2)
			hp += hp / 10;
		if (sc->data[SC_VITALITYACTIVATION])
			hp = hp * 150 / 100;
	}

#ifdef RENEWAL
	// MATK part of the RE heal formula [malufett]
	// Note: in this part matk bonuses from items or skills are not applied
	switch( skill_id ) {
		case BA_APPLEIDUN:
		case PR_SANCTUARY:
		case NPC_EVILLAND:
			break;
		default:
			hp += status->get_matk(src, 3);
	}
#endif // RENEWAL
	return hp;
}

// Making plagiarize check its own function [Aru]
int can_copy (struct map_session_data *sd, uint16 skill_id, struct block_list* bl)
{
	// Never copy NPC/Wedding Skills
	if (skill->get_inf2(skill_id)&(INF2_NPC_SKILL|INF2_WEDDING_SKILL))
		return 0;

	// High-class skills
	if((skill_id >= LK_AURABLADE && skill_id <= ASC_CDP) || (skill_id >= ST_PRESERVE && skill_id <= CR_CULTIVATION))
	{
		if(battle_config.copyskill_restrict == 2)
			return 0;
		else if(battle_config.copyskill_restrict)
			return (sd->status.class_ == JOB_STALKER);
	}

	//Added so plagarize can't copy agi/bless if you're undead since it damages you
	if ((skill_id == AL_INCAGI || skill_id == AL_BLESSING ||
		skill_id == CASH_BLESSING || skill_id == CASH_INCAGI ||
		skill_id == MER_INCAGI || skill_id == MER_BLESSING))
		return 0;

	// Couldn't preserve 3rd Class skills except only when using Reproduce skill. [Jobbie]
	if( !(sd->sc.data[SC__REPRODUCE]) && ((skill_id >= RK_ENCHANTBLADE && skill_id <= LG_OVERBRAND_PLUSATK) || (skill_id >= RL_GLITTERING_GREED && skill_id <= OB_AKAITSUKI) || (skill_id >= GC_DARKCROW && skill_id <= NC_MAGMA_ERUPTION_DOTDAMAGE)))
		return 0;
	// Reproduce will only copy skills according on the list. [Jobbie]
	else if( sd->sc.data[SC__REPRODUCE] && !skill->dbs->reproduce_db[skill->get_index(skill_id)] )
		return 0;

	return 1;
}

// [MouseJstr] - skill ok to cast? and when?
int skillnotok (uint16 skill_id, struct map_session_data *sd)
{
	int16 idx,m;
	nullpo_retr (1, sd);
	m = sd->bl.m;
	idx = skill->get_index(skill_id);

	if (idx == 0)
		return 1; // invalid skill id

	if( pc_has_permission(sd, PC_PERM_DISABLE_SKILL_USAGE) )
		return 1;

	if (pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL))
		return 0; // can do any damn thing they want

	if( skill_id == AL_TELEPORT && sd->skillitem == skill_id && sd->skillitemlv > 2 )
		return 0; // Teleport lv 3 bypasses this check.[Inkfish]

	// Epoque:
	// This code will compare the player's attack motion value which is influenced by ASPD before
	// allowing a skill to be cast. This is to prevent no-delay ACT files from spamming skills such as
	// AC_DOUBLE which do not have a skill delay and are not regarded in terms of attack motion.
	if( !sd->state.autocast && sd->skillitem != skill_id && sd->canskill_tick &&
		DIFF_TICK(timer->gettick(), sd->canskill_tick) < (sd->battle_status.amotion * (battle_config.skill_amotion_leniency) / 100) )
	{// attempted to cast a skill before the attack motion has finished
		return 1;
	}

	if (sd->blockskill[idx]) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0);
		return 1;
	}

	/**
	 * It has been confirmed on a official server (thanks to Yommy) that item-cast skills bypass all the restrictions below
	 * Also, without this check, an exploit where an item casting + healing (or any other kind buff) isn't deleted after used on a restricted map
	 **/
	if( sd->skillitem == skill_id )
		return 0;

	if( sd->sc.data[SC_ALL_RIDING] )
		return 1;//You can't use skills while in the new mounts (The client doesn't let you, this is to make cheat-safe)

	switch (skill_id) {
		case AL_WARP:
		case RETURN_TO_ELDICASTES:
		case ALL_GUARDIAN_RECALL:
		case ECLAGE_RECALL:
			if(map->list[m].flag.nowarp) {
				clif->skill_mapinfomessage(sd,0);
				return 1;
			}
			return 0;
		case AL_TELEPORT:
		case SC_FATALMENACE:
		case SC_DIMENSIONDOOR:
			if(map->list[m].flag.noteleport) {
				clif->skill_mapinfomessage(sd,0);
				return 1;
			}
			return 0; // gonna be checked in 'skill->castend_nodamage_id'
		case WE_CALLPARTNER:
		case WE_CALLPARENT:
		case WE_CALLBABY:
			if (map->list[m].flag.nomemo) {
				clif->skill_mapinfomessage(sd,1);
				return 1;
			}
			break;
		case MC_VENDING:
		case ALL_BUYING_STORE:
			if( npc->isnear(&sd->bl) ) {
				// uncomment for more verbose message.
				//char output[150];
				//sprintf(output, msg_txt(862), battle_config.min_npc_vendchat_distance); // "You're too close to a NPC, you must be at least %d cells away from any NPC."
				//clif->message(sd->fd, output);
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_THERE_ARE_NPC_AROUND,0);
				return 1;
			}
		case MC_IDENTIFY:
			return 0; // always allowed
		case WZ_ICEWALL:
			// noicewall flag [Valaris]
			if (map->list[m].flag.noicewall) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;
		case GC_DARKILLUSION:
			if( map_flag_gvg2(m) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;
		case GD_EMERGENCYCALL:
			if( !(battle_config.emergency_call&((map->agit_flag || map->agit2_flag)?2:1))
			 || !(battle_config.emergency_call&(map->list[m].flag.gvg || map->list[m].flag.gvg_castle?8:4))
			 || (battle_config.emergency_call&16 && map->list[m].flag.nowarpto && !map->list[m].flag.gvg_castle)
			) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;

		case SC_MANHOLE:
		case WM_SOUND_OF_DESTRUCTION:
		case WM_SATURDAY_NIGHT_FEVER:
		case WM_LULLABY_DEEPSLEEP:
			if( !map_flag_vs(m) ) {
				clif->skill_mapinfomessage(sd,2); // This skill uses this msg instead of skill fails.
				return 1;
			}
			break;

	}
	return (map->list[m].flag.noskill);
}

int skillnotok_hom(uint16 skill_id, struct homun_data *hd)
{
	uint16 idx = skill->get_index(skill_id);
	nullpo_retr(1,hd);

	if (idx == 0)
		return 1; // invalid skill id

	if (hd->blockskill[idx] > 0)
		return 1;
	switch(skill_id){
	    case MH_LIGHT_OF_REGENE:
			if( homun->get_intimacy_grade(hd) != 4 ){
				if( hd->master )
					clif->skill_fail(hd->master, skill_id, USESKILL_FAIL_RELATIONGRADE, 0);
				return 1;
			}
			break;
	    case MH_GOLDENE_FERSE: //can be used with angriff
			if(hd->sc.data[SC_ANGRIFFS_MODUS])
				return 1;
			/* Fall through */
	    case MH_ANGRIFFS_MODUS:
			if(hd->sc.data[SC_GOLDENE_FERSE])
				return 1;
			break;
	}

	//Use master's criteria.
	return skill->not_ok(skill_id, hd->master);
}

int skillnotok_mercenary(uint16 skill_id, struct mercenary_data *md)
{
	uint16 idx = skill->get_index(skill_id);
	nullpo_retr(1,md);

	if( idx == 0 )
		return 1; // Invalid Skill ID
	if( md->blockskill[idx] > 0 )
		return 1;

	return skill->not_ok(skill_id, md->master);
}

struct s_skill_unit_layout* skill_get_unit_layout (uint16 skill_id, uint16 skill_lv, struct block_list* src, int x, int y) {
	int pos = skill->get_unit_layout_type(skill_id,skill_lv);
	uint8 dir;

	if (pos < -1 || pos >= MAX_SKILL_UNIT_LAYOUT) {
		ShowError("skill_get_unit_layout: unsupported layout type %d for skill %d (level %d)\n", pos, skill_id, skill_lv);
		pos = cap_value(pos, 0, MAX_SQUARE_LAYOUT); // cap to nearest square layout
	}

	if (pos != -1) // simple single-definition layout
		return &skill->dbs->unit_layout[pos];

	dir = (src->x == x && src->y == y) ? 6 : map->calc_dir(src,x,y); // 6 - default aegis direction

	if (skill_id == MG_FIREWALL)
		return &skill->dbs->unit_layout [skill->firewall_unit_pos + dir];
	else if (skill_id == WZ_ICEWALL)
		return &skill->dbs->unit_layout [skill->icewall_unit_pos + dir];
	else if( skill_id == WL_EARTHSTRAIN ) //Warlock
		return &skill->dbs->unit_layout [skill->earthstrain_unit_pos + dir];

	ShowError("skill_get_unit_layout: unknown unit layout for skill %d (level %d)\n", skill_id, skill_lv);
	return &skill->dbs->unit_layout[0]; // default 1x1 layout
}

/*==========================================
 *
 *------------------------------------------*/
int skill_additional_effect(struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, int dmg_lv, int64 tick) {
	struct map_session_data *sd, *dstsd;
	struct mob_data *md, *dstmd;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc, *tsc;

	int temp;
	int rate;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(skill_id > 0 && !skill_lv) return 0; // don't forget auto attacks! - celest

	if( dmg_lv < ATK_BLOCK ) // Don't apply effect if miss.
		return 0;

	sd = BL_CAST(BL_PC, src);
	md = BL_CAST(BL_MOB, src);
	dstsd = BL_CAST(BL_PC, bl);
	dstmd = BL_CAST(BL_MOB, bl);

	sc = status->get_sc(src);
	tsc = status->get_sc(bl);
	sstatus = status->get_status_data(src);
	tstatus = status->get_status_data(bl);
	if (!tsc) //skill additional effect is about adding effects to the target...
		//So if the target can't be inflicted with statuses, this is pointless.
		return 0;

	if( sd ) { // These statuses would be applied anyway even if the damage was blocked by some skills. [Inkfish]
		if( skill_id != WS_CARTTERMINATION && skill_id != AM_DEMONSTRATION && skill_id != CR_REFLECTSHIELD && skill_id != MS_REFLECTSHIELD && skill_id != ASC_BREAKER ) {
			// Trigger status effects
			enum sc_type type;
			int i, flag;
			for( i = 0; i < ARRAYLENGTH(sd->addeff) && sd->addeff[i].flag; i++ ) {
				rate = sd->addeff[i].rate;
				if( attack_type&BF_LONG ) // Any ranged physical attack takes status arrows into account (Grimtooth...) [DracoRPG]
					rate += sd->addeff[i].arrow_rate;
				if( !rate ) continue;

				if( (sd->addeff[i].flag&(ATF_WEAPON|ATF_MAGIC|ATF_MISC)) != (ATF_WEAPON|ATF_MAGIC|ATF_MISC) ) {
					// Trigger has attack type consideration.
					if( (sd->addeff[i].flag&ATF_WEAPON && attack_type&BF_WEAPON) ||
						(sd->addeff[i].flag&ATF_MAGIC && attack_type&BF_MAGIC) ||
						(sd->addeff[i].flag&ATF_MISC && attack_type&BF_MISC) ) ;
					else
						continue;
				}

				if( (sd->addeff[i].flag&(ATF_LONG|ATF_SHORT)) != (ATF_LONG|ATF_SHORT) ) {
					// Trigger has range consideration.
					if((sd->addeff[i].flag&ATF_LONG && !(attack_type&BF_LONG)) ||
						(sd->addeff[i].flag&ATF_SHORT && !(attack_type&BF_SHORT)))
						continue; //Range Failed.
				}

				type =  sd->addeff[i].id;

				if (sd->addeff[i].duration > 0) {
					// Fixed duration
					temp = sd->addeff[i].duration;
					flag = SCFLAG_FIXEDRATE|SCFLAG_FIXEDTICK;
				} else {
					// Default duration
					temp = skill->get_time2(status->sc2skill(type),7);
					flag = SCFLAG_NONE;
				}

				if (sd->addeff[i].flag&ATF_TARGET)
					status->change_start(src,bl,type,rate,7,0,(type == SC_BURNING)?src->id:0,0,temp,flag);

				if (sd->addeff[i].flag&ATF_SELF)
					status->change_start(src,src,type,rate,7,0,(type == SC_BURNING)?src->id:0,0,temp,flag);
			}
		}

		if( skill_id ) {
			// Trigger status effects on skills
			enum sc_type type;
			int i;
			for( i = 0; i < ARRAYLENGTH(sd->addeff3) && sd->addeff3[i].skill; i++ ) {
				if( skill_id != sd->addeff3[i].skill || !sd->addeff3[i].rate )
					continue;
				type = sd->addeff3[i].id;
				temp = skill->get_time2(status->sc2skill(type),7);

				if( sd->addeff3[i].target&ATF_TARGET )
					status->change_start(src,bl,type,sd->addeff3[i].rate,7,0,0,0,temp,SCFLAG_NONE);
				if( sd->addeff3[i].target&ATF_SELF )
					status->change_start(src,src,type,sd->addeff3[i].rate,7,0,0,0,temp,SCFLAG_NONE);
			}
		}
	}

	if( dmg_lv < ATK_DEF ) // no damage, return;
		return 0;

	switch(skill_id) {
		case 0: { // Normal attacks (no skill used)
			if( attack_type&BF_SKILL )
				break; // If a normal attack is a skill, it's splash damage. [Inkfish]
			if(sd) {
				// Automatic trigger of Blitz Beat
				if (pc_isfalcon(sd) && sd->status.weapon == W_BOW && (temp=pc->checkskill(sd,HT_BLITZBEAT))>0 &&
					rnd()%1000 <= sstatus->luk*3 ) {
					rate = sd->status.job_level / 10 + 1;
					skill->castend_damage_id(src,bl,HT_BLITZBEAT,(temp<rate)?temp:rate,tick,SD_LEVEL);
				}
				// Automatic trigger of Warg Strike [Jobbie]
				if( pc_iswug(sd) && (temp=pc->checkskill(sd,RA_WUGSTRIKE)) > 0 && rnd()%1000 <= sstatus->luk*3 )
					skill->castend_damage_id(src,bl,RA_WUGSTRIKE,temp,tick,0);
				// Gank
				if(dstmd && sd->status.weapon != W_BOW &&
					(temp=pc->checkskill(sd,RG_SNATCHER)) > 0 &&
					(temp*15 + 55) + pc->checkskill(sd,TF_STEAL)*10 > rnd()%1000) {
					if(pc->steal_item(sd,bl,pc->checkskill(sd,TF_STEAL)))
						clif->skill_nodamage(src,bl,TF_STEAL,temp,1);
					else
						clif->skill_fail(sd,RG_SNATCHER,USESKILL_FAIL_LEVEL,0);
				}
				// Chance to trigger Taekwon kicks [Dralnu]
				if(sc && !sc->data[SC_COMBOATTACK]) {
					if(sc->data[SC_STORMKICK_READY] &&
						sc_start4(src,src,SC_COMBOATTACK, 15, TK_STORMKICK,
							bl->id, 2, 0,
							(2000 - 4*sstatus->agi - 2*sstatus->dex)))
						; //Stance triggered
					else if(sc->data[SC_DOWNKICK_READY] &&
						sc_start4(src,src,SC_COMBOATTACK, 15, TK_DOWNKICK,
							bl->id, 2, 0,
							(2000 - 4*sstatus->agi - 2*sstatus->dex)))
						; //Stance triggered
					else if(sc->data[SC_TURNKICK_READY] &&
						sc_start4(src,src,SC_COMBOATTACK, 15, TK_TURNKICK,
							bl->id, 2, 0,
							(2000 - 4*sstatus->agi - 2*sstatus->dex)))
						; //Stance triggered
						else if (sc->data[SC_COUNTERKICK_READY]) { //additional chance from SG_FRIEND [Komurka]
						rate = 20;
						if (sc->data[SC_SKILLRATE_UP] && sc->data[SC_SKILLRATE_UP]->val1 == TK_COUNTER) {
							rate += rate*sc->data[SC_SKILLRATE_UP]->val2/100;
							status_change_end(src, SC_SKILLRATE_UP, INVALID_TIMER);
						}
						sc_start2(src, src, SC_COMBOATTACK, rate, TK_COUNTER, bl->id,
							(2000 - 4*sstatus->agi - 2*sstatus->dex));
					}
				}
				if(sc && sc->data[SC_PYROCLASTIC] && (rnd() % 1000 <= sstatus->luk * 10 / 3 + 1) )
					skill->castend_pos2(src, bl->x, bl->y, BS_HAMMERFALL,sc->data[SC_PYROCLASTIC]->val1, tick, 0);
			}

			if (sc) {
				struct status_change_entry *sce;
				// Enchant Poison gives a chance to poison attacked enemies
				if((sce=sc->data[SC_ENCHANTPOISON])) //Don't use sc_start since chance comes in 1/10000 rate.
					status->change_start(src,bl,SC_POISON,sce->val2, sce->val1,src->id,0,0,
						skill->get_time2(AS_ENCHANTPOISON,sce->val1),SCFLAG_NONE);
				// Enchant Deadly Poison gives a chance to deadly poison attacked enemies
				if((sce=sc->data[SC_EDP]))
					sc_start4(src,bl,SC_DPOISON,sce->val2, sce->val1,src->id,0,0,
						skill->get_time2(ASC_EDP,sce->val1));
			}
		}
			break;

		case SM_BASH:
			if( sd && skill_lv > 5 && pc->checkskill(sd,SM_FATALBLOW)>0 )
				status->change_start(src,bl,SC_STUN,500*(skill_lv-5)*sd->status.base_level/50,
					skill_lv,0,0,0,skill->get_time2(SM_FATALBLOW,skill_lv),SCFLAG_NONE);
			break;

		case MER_CRASH:
			sc_start(src,bl,SC_STUN,(6*skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case AS_VENOMKNIFE:
			if (sd) //Poison chance must be that of Envenom. [Skotlex]
				skill_lv = pc->checkskill(sd, TF_POISON);
			/* Fall through */
		case TF_POISON:
		case AS_SPLASHER:
			if (!sc_start2(src,bl,SC_POISON,(4*skill_lv+10),skill_lv,src->id,skill->get_time2(skill_id,skill_lv))
			 && sd && skill_id==TF_POISON
			)
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			break;

		case AS_SONICBLOW:
			sc_start(src,bl,SC_STUN,(2*skill_lv+10),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case WZ_FIREPILLAR:
			unit->set_walkdelay(bl, tick, skill->get_time2(skill_id, skill_lv), 1);
			break;

		case MG_FROSTDIVER:
	#ifndef RENEWAL
		case WZ_FROSTNOVA:
	#endif
			if (!sc_start(src,bl,SC_FREEZE,skill_lv*3+35,skill_lv,skill->get_time2(skill_id,skill_lv))
			 && sd && skill_id == MG_FROSTDIVER
			)
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			break;

	#ifdef RENEWAL
		case WZ_FROSTNOVA:
			sc_start(src,bl,SC_FREEZE,skill_lv*5+33,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
	#endif

		case WZ_STORMGUST:
		/**
		 * Storm Gust counter was dropped in renewal
		 **/
		#ifdef RENEWAL
			sc_start(src,bl,SC_FREEZE,65-(5*skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv));
		#else
			//On third hit, there is a 150% to freeze the target
			if(tsc->sg_counter >= 3 &&
				sc_start(src,bl,SC_FREEZE,150,skill_lv,skill->get_time2(skill_id,skill_lv)))
				tsc->sg_counter = 0;
			/**
			 * being it only resets on success it'd keep stacking and eventually overflowing on mvps, so we reset at a high value
			 **/
			else if( tsc->sg_counter > 250 )
				tsc->sg_counter = 0;
		#endif
			break;

		case WZ_METEOR:
			sc_start(src,bl,SC_STUN,3*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case WZ_VERMILION:
			sc_start(src,bl,SC_BLIND,4*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
			sc_start(src,bl,SC_FREEZE,(3*skill_lv+35),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case HT_FLASHER:
			sc_start(src,bl,SC_BLIND,(10*skill_lv+30),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case HT_LANDMINE:
		case MA_LANDMINE:
			sc_start(src,bl,SC_STUN,(5*skill_lv+30),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case HT_SHOCKWAVE:
			status_percent_damage(src, bl, 0, 15*skill_lv+5, false);
			break;

		case HT_SANDMAN:
		case MA_SANDMAN:
			sc_start(src,bl,SC_SLEEP,(10*skill_lv+40),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case TF_SPRINKLESAND:
			sc_start(src,bl,SC_BLIND,20,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case TF_THROWSTONE:
			if( !sc_start(src,bl,SC_STUN,3,skill_lv,skill->get_time(skill_id,skill_lv)) )
				sc_start(src,bl,SC_BLIND,3,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case NPC_DARKCROSS:
		case CR_HOLYCROSS:
			sc_start(src,bl,SC_BLIND,3*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case CR_GRANDCROSS:
		case NPC_GRANDDARKNESS:
			//Chance to cause blind status vs demon and undead element, but not against players
			if(!dstsd && (battle->check_undead(tstatus->race,tstatus->def_ele) || tstatus->race == RC_DEMON))
				sc_start(src,bl,SC_BLIND,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case AM_ACIDTERROR:
			sc_start2(src, bl, SC_BLOODING, (skill_lv * 3), skill_lv, src->id, skill->get_time2(skill_id, skill_lv));
			if ( bl->type == BL_PC && rnd() % 1000 < 10 * skill->get_time(skill_id, skill_lv) ) {
				skill->break_equip(bl, EQP_ARMOR, 10000, BCT_ENEMY);
				clif->emotion(bl, E_OMG); // emote icon still shows even there is no armor equip.
			}
			break;

		case AM_DEMONSTRATION:
			skill->break_equip(bl, EQP_WEAPON, 100*skill_lv, BCT_ENEMY);
			break;

		case CR_SHIELDCHARGE:
			sc_start(src,bl,SC_STUN,(15+skill_lv*5),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case PA_PRESSURE:
			status_percent_damage(src, bl, 0, 15+5*skill_lv, false);
			break;

		case RG_RAID:
			sc_start(src,bl,SC_STUN,(10+3*skill_lv),skill_lv,skill->get_time(skill_id,skill_lv));
			sc_start(src,bl,SC_BLIND,(10+3*skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv));

	#ifdef RENEWAL
			sc_start(src,bl,SC_RAID,100,7,5000);
			break;

		case RG_BACKSTAP:
			sc_start(src,bl,SC_STUN,(5+2*skill_lv),skill_lv,skill->get_time(skill_id,skill_lv));
	#endif
			break;

		case BA_FROSTJOKER:
			sc_start(src,bl,SC_FREEZE,(15+5*skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case DC_SCREAM:
			sc_start(src,bl,SC_STUN,(25+5*skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case BD_LULLABY:
			sc_start(src,bl,SC_SLEEP,15,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case DC_UGLYDANCE:
			rate = 5+5*skill_lv;
			if (sd && (temp=pc->checkskill(sd,DC_DANCINGLESSON)) > 0)
				rate += 5+temp;
			status_zap(bl, 0, rate);
			break;
		case SL_STUN:
			if (tstatus->size==SZ_MEDIUM) //Only stuns mid-sized mobs.
				sc_start(src,bl,SC_STUN,(30+10*skill_lv),skill_lv,skill->get_time(skill_id,skill_lv));
			break;

		case NPC_PETRIFYATTACK:
			sc_start4(src,bl,status->skill2sc(skill_id),50+10*skill_lv,
				skill_lv,0,0,skill->get_time(skill_id,skill_lv),
				skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_CURSEATTACK:
		case NPC_SLEEPATTACK:
		case NPC_BLINDATTACK:
		case NPC_POISON:
		case NPC_SILENCEATTACK:
		case NPC_STUNATTACK:
		case NPC_HELLPOWER:
			sc_start(src,bl,status->skill2sc(skill_id),50+10*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_ACIDBREATH:
		case NPC_ICEBREATH:
			sc_start(src,bl,status->skill2sc(skill_id),70,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_BLEEDING:
			sc_start2(src,bl,SC_BLOODING,(20*skill_lv),skill_lv,src->id,skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_MENTALBREAKER:
		{
			//Based on observations by [Tharis], Mental Breaker should do SP damage
			//equal to Matk*skLevel.
			rate = status->get_matk(src, 2);
			rate*=skill_lv;
			status_zap(bl, 0, rate);
			break;
		}
		// Equipment breaking monster skills [Celest]
		case NPC_WEAPONBRAKER:
			skill->break_equip(bl, EQP_WEAPON, 150*skill_lv, BCT_ENEMY);
			break;
		case NPC_ARMORBRAKE:
			skill->break_equip(bl, EQP_ARMOR, 150*skill_lv, BCT_ENEMY);
			break;
		case NPC_HELMBRAKE:
			skill->break_equip(bl, EQP_HELM, 150*skill_lv, BCT_ENEMY);
			break;
		case NPC_SHIELDBRAKE:
			skill->break_equip(bl, EQP_SHIELD, 150*skill_lv, BCT_ENEMY);
			break;

		case CH_TIGERFIST:
			sc_start(src,bl,SC_STOP,(10+skill_lv*10),0,skill->get_time2(skill_id,skill_lv));
			break;

		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
			if( dstsd || ( dstmd && !is_boss(bl) ) ) //Does not work on bosses
				sc_start(src,bl,SC_STOP,100,0,skill->get_time2(skill_id,skill_lv));
			break;

		case ST_REJECTSWORD:
			sc_start(src,bl,SC_AUTOCOUNTER,(skill_lv*15),skill_lv,skill->get_time(skill_id,skill_lv));
			break;

		case PF_FOGWALL:
			if (src != bl && !tsc->data[SC_DELUGE])
				sc_start(src,bl,SC_BLIND,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case LK_HEADCRUSH: // Headcrush has chance of causing Bleeding status, except on demon and undead element
			if (!(battle->check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON))
				sc_start2(src, bl, SC_BLOODING,50, skill_lv, src->id, skill->get_time2(skill_id,skill_lv));
			break;

		case LK_JOINTBEAT:
			if (tsc->jb_flag) {
				enum sc_type type = status->skill2sc(skill_id);
				sc_start4(src,bl,type,(5*skill_lv+5),skill_lv,tsc->jb_flag&BREAK_FLAGS,src->id,0,skill->get_time2(skill_id,skill_lv));
				tsc->jb_flag = 0;
			}
			break;
		case ASC_METEORASSAULT:
			//Any enemies hit by this skill will receive Stun, Darkness, or external bleeding status ailment with a 5%+5*skill_lv% chance.
			switch(rnd()%3) {
				case 0:
					sc_start(src,bl,SC_BLIND,(5+skill_lv*5),skill_lv,skill->get_time2(skill_id,1));
					break;
				case 1:
					sc_start(src,bl,SC_STUN,(5+skill_lv*5),skill_lv,skill->get_time2(skill_id,2));
					break;
				default:
					sc_start2(src,bl,SC_BLOODING,(5+skill_lv*5),skill_lv,src->id,skill->get_time2(skill_id,3));
			}
			break;

		case HW_NAPALMVULCAN:
			sc_start(src,bl,SC_CURSE,5*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case WS_CARTTERMINATION:
			sc_start(src,bl,SC_STUN,5*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case CR_ACIDDEMONSTRATION:
			skill->break_equip(bl, EQP_WEAPON|EQP_ARMOR, 100*skill_lv, BCT_ENEMY);
			break;

		case TK_DOWNKICK:
			sc_start(src,bl,SC_STUN,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case TK_JUMPKICK:
			if( dstsd && dstsd->class_ != MAPID_SOUL_LINKER && !tsc->data[SC_PRESERVE] )
			{// debuff the following statuses
				status_change_end(bl, SC_SOULLINK, INVALID_TIMER);
				status_change_end(bl, SC_ADRENALINE2, INVALID_TIMER);
				status_change_end(bl, SC_KAITE, INVALID_TIMER);
				status_change_end(bl, SC_KAAHI, INVALID_TIMER);
				status_change_end(bl, SC_ONEHANDQUICKEN, INVALID_TIMER);
				status_change_end(bl, SC_ATTHASTE_POTION3, INVALID_TIMER);
			}
			break;
		case TK_TURNKICK:
		case MO_BALKYOUNG: //Note: attack_type is passed as BF_WEAPON for the actual target, BF_MISC for the splash-affected mobs.
			if(attack_type&BF_MISC) //70% base stun chance...
				sc_start(src,bl,SC_STUN,70,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case GS_BULLSEYE: //0.1% coma rate.
			if(tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN)
				status->change_start(src,bl,SC_COMA,10,skill_lv,0,src->id,0,0,SCFLAG_NONE);
			break;
		case GS_PIERCINGSHOT:
			sc_start2(src,bl,SC_BLOODING,(skill_lv*3),skill_lv,src->id,skill->get_time2(skill_id,skill_lv));
			break;
		case NJ_HYOUSYOURAKU:
			sc_start(src,bl,SC_FREEZE,(10+10*skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case GS_FLING:
			sc_start(src,bl,SC_FLING,100, sd?sd->spiritball_old:5,skill->get_time(skill_id,skill_lv));
			break;
		case GS_DISARM:
			rate = 3*skill_lv;
			if (sstatus->dex > tstatus->dex)
				rate += (sstatus->dex - tstatus->dex)/5; //TODO: Made up formula
			skill->strip_equip(bl, EQP_WEAPON, rate, skill_lv, skill->get_time(skill_id,skill_lv));
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case NPC_EVILLAND:
			sc_start(src,bl,SC_BLIND,5*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_HELLJUDGEMENT:
			sc_start(src,bl,SC_CURSE,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_CRITICALWOUND:
			sc_start(src,bl,SC_CRITICALWOUND,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case RK_WINDCUTTER:
			sc_start(src,bl,SC_FEAR,3+2*skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case RK_DRAGONBREATH:
			sc_start4(src,bl,SC_BURNING,15,skill_lv,1000,src->id,0,skill->get_time(skill_id,skill_lv));
			break;
		case RK_DRAGONBREATH_WATER:
			sc_start4(src,bl,SC_FROSTMISTY,15,skill_lv,1000,src->id,0,skill->get_time(skill_id,skill_lv));
			break;
		case AB_ADORAMUS:
			if( tsc && !tsc->data[SC_DEC_AGI] ) //Prevent duplicate agi-down effect.
				sc_start(src, bl, SC_ADORAMUS, skill_lv * 4 + (sd? sd->status.job_level:50)/2, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case WL_CRIMSONROCK:
			sc_start(src, bl, SC_STUN, 40, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case WL_COMET:
			sc_start4(src,bl,SC_BURNING,100,skill_lv,0,src->id,0,skill->get_time2(skill_id,skill_lv));
			break;
		case WL_EARTHSTRAIN:
			{
				int i;
				const int pos[5] = { EQP_WEAPON, EQP_HELM, EQP_SHIELD, EQP_ARMOR, EQP_ACC };

				for( i = 0; i < skill_lv; i++ )
					skill->strip_equip(bl,pos[i], (5 + skill_lv) * skill_lv,
						skill_lv,skill->get_time2(skill_id,skill_lv));
			}
			break;
		case WL_JACKFROST:
			sc_start(src,bl,SC_FREEZE,100,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case WL_FROSTMISTY:
			sc_start(src,bl,SC_FROSTMISTY,25 + 5 * skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case RA_WUGBITE:
			rate = 50 + 10 * skill_lv + 2 * (sd ? pc->checkskill(sd,RA_TOOTHOFWUG) : 0) - tstatus->agi / 4;
			if ( rate < 50 )
				rate = 50;
			sc_start(src,bl,SC_WUGBITE, rate, skill_lv, skill->get_time(skill_id, skill_lv) + (sd ? pc->checkskill(sd,RA_TOOTHOFWUG) * 500 : 0));
			break;
		case RA_SENSITIVEKEEN:
			if( rnd()%100 < 8 * skill_lv )
				skill->castend_damage_id(src, bl, RA_WUGBITE, sd ? pc->checkskill(sd, RA_WUGBITE):skill_lv, tick, SD_ANIMATION);
			break;
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
			if( dstmd && !(dstmd->status.mode&MD_BOSS) )
				sc_start2(src,bl,SC_ARMOR_PROPERTY,100,skill_lv,skill->get_ele(skill_id,skill_lv),skill->get_time2(skill_id,skill_lv));
			break;
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			sc_start4(src, bl, (skill_id == RA_FIRINGTRAP) ? SC_BURNING:SC_FROSTMISTY, 50 + 10 * skill_lv, skill_lv, 0, src->id, 0, skill->get_time2(skill_id, skill_lv));
			break;
		case NC_PILEBUNKER:
			if( rnd()%100 < 25 + 15 *skill_lv ) {
				//Deactivatable Statuses: Kyrie Eleison, Auto Guard, Steel Body, Assumptio, and Millennium Shield
				status_change_end(bl, SC_KYRIE, INVALID_TIMER);
				status_change_end(bl, SC_ASSUMPTIO, INVALID_TIMER);
				status_change_end(bl, SC_STEELBODY, INVALID_TIMER);
				status_change_end(bl, SC_GENTLETOUCH_CHANGE, INVALID_TIMER);
				status_change_end(bl, SC_GENTLETOUCH_REVITALIZE, INVALID_TIMER);
				status_change_end(bl, SC_AUTOGUARD, INVALID_TIMER);
				status_change_end(bl, SC_REFLECTSHIELD, INVALID_TIMER);
				status_change_end(bl, SC_DEFENDER, INVALID_TIMER);
				status_change_end(bl, SC_LG_REFLECTDAMAGE, INVALID_TIMER);
				status_change_end(bl, SC_PRESTIGE, INVALID_TIMER);
				status_change_end(bl, SC_BANDING, INVALID_TIMER);
				status_change_end(bl, SC_MILLENNIUMSHIELD, INVALID_TIMER);
			}
			break;
		case NC_FLAMELAUNCHER:
			sc_start4(src, bl, SC_BURNING, 20 + 10 * skill_lv, skill_lv, 0, src->id, 0, skill->get_time2(skill_id, skill_lv));
			break;
		case NC_COLDSLOWER:
			sc_start(src, bl, SC_FREEZE, 10 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			if ( tsc && !tsc->data[SC_FREEZE] )
				sc_start(src, bl, SC_FROSTMISTY, 20 + 10 * skill_lv, skill_lv, skill->get_time2(skill_id, skill_lv));
			break;
		case NC_POWERSWING:
			// Use flag=2, the stun duration is not vit-reduced.
			status->change_start(src, bl, SC_STUN, 5*skill_lv*100, skill_lv, 0, 0, 0, skill->get_time(skill_id, skill_lv), SCFLAG_FIXEDTICK);
			if( rnd()%100 < 5*skill_lv )
				skill->castend_damage_id(src, bl, NC_AXEBOOMERANG, pc->checkskill(sd, NC_AXEBOOMERANG), tick, 1);
			break;
		case NC_MAGMA_ERUPTION:
			sc_start4(src, bl, SC_BURNING, 10 * skill_lv, skill_lv, 0, src->id, 0, skill->get_time2(skill_id, skill_lv));
			sc_start(src, bl, SC_STUN, 10 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case GC_WEAPONCRUSH:
			skill->castend_nodamage_id(src,bl,skill_id,skill_lv,tick,BCT_ENEMY);
			break;
		case GC_DARKCROW:
			sc_start(src, bl, SC_DARKCROW, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case LG_SHIELDPRESS:
			rate = 30 + 8 * skill_lv + sstatus->dex / 10 + (sd? sd->status.job_level:0) / 4;
			sc_start(src, bl, SC_STUN, rate, skill_lv, skill->get_time(skill_id,skill_lv));
			break;
		case LG_HESPERUSLIT:
			if ( sc && sc->data[SC_BANDING] ) {
				if ( sc->data[SC_BANDING]->val2 == 4 ) // 4 banding RGs: Targets will be stunned at 100% chance for 4 ~ 8 seconds, irreducible by STAT.
					status->change_start(src, bl, SC_STUN, 10000, skill_lv, 0, 0, 0, 1000*(4+rnd()%4), SCFLAG_FIXEDTICK);
				else if ( sc->data[SC_BANDING]->val2 == 6 ) // 6 banding RGs: activate Pinpoint Attack Lv1-5
					skill->castend_damage_id(src,bl,LG_PINPOINTATTACK,1+rnd()%5,tick,0);
			}
			break;
		case LG_PINPOINTATTACK:
			rate = 30 + 5 * (sd ? pc->checkskill(sd,LG_PINPOINTATTACK) : 1) + (sstatus->agi + status->get_lv(src)) / 10;
			switch( skill_lv ) {
				case 1:
					sc_start(src, bl,SC_BLOODING,rate,skill_lv,skill->get_time(skill_id,skill_lv));
					break;
				case 2:
					skill->break_equip(bl, EQP_HELM, rate*100, BCT_ENEMY);
					break;
				case 3:
					skill->break_equip(bl, EQP_SHIELD, rate*100, BCT_ENEMY);
					break;
				case 4:
					skill->break_equip(bl, EQP_ARMOR, rate*100, BCT_ENEMY);
					break;
				case 5:
					skill->break_equip(bl, EQP_WEAPON, rate*100, BCT_ENEMY);
					break;
			}
			break;
		case LG_MOONSLASHER:
			rate = 32 + 8 * skill_lv;
			if( rnd()%100 < rate && dstsd ) // Uses skill->addtimerskill to avoid damage and setsit packet overlaping. Officially clif->setsit is received about 500 ms after damage packet.
				skill->addtimerskill(src,tick+500,bl->id,0,0,skill_id,skill_lv,BF_WEAPON,0);
			else if( dstmd && !is_boss(bl) )
				sc_start(src, bl,SC_STOP,100,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case LG_RAYOFGENESIS: // 50% chance to cause Blind on Undead and Demon monsters.
			if ( battle->check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON )
				sc_start(src, bl, SC_BLIND,50, skill_lv, skill->get_time(skill_id,skill_lv));
			break;
		case LG_EARTHDRIVE:
			skill->break_equip(src, EQP_SHIELD, 100 * skill_lv, BCT_SELF);
			sc_start(src, bl, SC_EARTHDRIVE, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SR_DRAGONCOMBO:
			sc_start(src, bl, SC_STUN, 1 + skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SR_FALLENEMPIRE:
			sc_start(src, bl, SC_FALLENEMPIRE, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SR_WINDMILL:
			if( dstsd )
				skill->addtimerskill(src,tick+status_get_amotion(src),bl->id,0,0,skill_id,skill_lv,BF_WEAPON,0);
			else if( dstmd && !is_boss(bl) )
				sc_start(src, bl, SC_STUN, 100, skill_lv, 1000 + 1000 * (rnd() %3));
			break;
		case SR_GENTLETOUCH_QUIET:  //  [(Skill Level x 5) + (Caster?s DEX + Caster?s Base Level) / 10]
			sc_start(src, bl, SC_SILENCE, 5 * skill_lv + (sstatus->dex + status->get_lv(src)) / 10, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SR_EARTHSHAKER:
			sc_start(src, bl,SC_STUN, 25 + 5 * skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case SR_HOWLINGOFLION:
			sc_start(src, bl, SC_FEAR, 5 + 5 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SO_EARTHGRAVE:
			sc_start2(src, bl, SC_BLOODING, 5 * skill_lv, skill_lv, src->id, skill->get_time2(skill_id, skill_lv)); // Need official rate. [LimitLine]
			break;
		case SO_DIAMONDDUST:
			rate = 5 + 5 * skill_lv;
			if( sc && sc->data[SC_COOLER_OPTION] )
				rate += sc->data[SC_COOLER_OPTION]->val3 / 5;
			sc_start(src, bl, SC_COLD, rate, skill_lv, skill->get_time2(skill_id, skill_lv));
			break;
		case SO_VARETYR_SPEAR:
			sc_start(src, bl, SC_STUN, 5 + 5 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case GN_SLINGITEM_RANGEMELEEATK:
			if( sd ) {
				switch( sd->itemid ) {
					// Starting SCs here instead of do it in skill->additional_effect to simplify the code.
					case ITEMID_COCONUT_BOMB:
						sc_start(src, bl, SC_STUN, 100, skill_lv, 5000); // 5 seconds until I get official
						sc_start(src, bl, SC_BLOODING, 100, skill_lv, 10000);
						break;
					case ITEMID_MELON_BOMB:
						sc_start(src, bl, SC_MELON_BOMB, 100, skill_lv, 60000); // Reduces ASPD and movement speed
						break;
					case ITEMID_BANANA_BOMB:
						sc_start(src, bl, SC_BANANA_BOMB, 100, skill_lv, 60000); // Reduces LUK? Needed confirm it, may be it's bugged in kRORE?
						sc_start(src, bl, SC_BANANA_BOMB_SITDOWN_POSTDELAY, (sd? sd->status.job_level:0) + sstatus->dex / 6 + tstatus->agi / 4 - tstatus->luk / 5 - status->get_lv(bl) + status->get_lv(src), skill_lv, 1000); // Sit down for 3 seconds.
						break;
				}
				sd->itemid = -1;
			}
			break;
		case GN_HELLS_PLANT_ATK:
			sc_start(src, bl, SC_STUN,  20 + 10 * skill_lv, skill_lv, skill->get_time2(skill_id, skill_lv));
			sc_start2(src, bl, SC_BLOODING, 5 + 5 * skill_lv, skill_lv, src->id,skill->get_time2(skill_id, skill_lv));
			break;
		case EL_WIND_SLASH: // Non confirmed rate.
			sc_start2(src, bl, SC_BLOODING, 25, skill_lv, src->id, skill->get_time(skill_id,skill_lv));
			break;
		case EL_STONE_HAMMER:
			rate = 10 * skill_lv;
			sc_start(src, bl, SC_STUN, rate, skill_lv, skill->get_time(skill_id,skill_lv));
			break;
		case EL_ROCK_CRUSHER:
		case EL_ROCK_CRUSHER_ATK:
			sc_start(src, bl,status->skill2sc(skill_id),50,skill_lv,skill->get_time(EL_ROCK_CRUSHER,skill_lv));
			break;
		case EL_TYPOON_MIS:
			sc_start(src, bl,SC_SILENCE,10*skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case KO_JYUMONJIKIRI:
			sc_start(src, bl,SC_KO_JYUMONJIKIRI,90,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case KO_MAKIBISHI:
			sc_start(src, bl, SC_STUN, 10 * skill_lv, skill_lv, 1000 * (skill_lv / 2 + 2));
			break;
		case MH_LAVA_SLIDE:
			if (tsc && !tsc->data[SC_BURNING]) sc_start4(src, bl, SC_BURNING, 10 * skill_lv, skill_lv, 0, src->id, 0, skill->get_time(skill_id, skill_lv));
			break;
		case MH_STAHL_HORN:
			sc_start(src, bl, SC_STUN, (20 + 4 * (skill_lv-1)), skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case MH_NEEDLE_OF_PARALYZE:
			sc_start(src, bl, SC_NEEDLE_OF_PARALYZE, 40 + (5*skill_lv), skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case GN_ILLUSIONDOPING:
			if( sc_start(src, bl, SC_ILLUSIONDOPING, 10 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv)) ) //custom rate.
				sc_start(src, bl, SC_ILLUSION, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case MH_XENO_SLASHER:
			sc_start2(src, bl, SC_BLOODING, 10 * skill_lv, skill_lv, src->id, skill->get_time(skill_id,skill_lv));
			break;
		default:
			skill->additional_effect_unknown(src, bl, &skill_id, &skill_lv, &attack_type, &dmg_lv, &tick);
			break;
	}

	if (md && battle_config.summons_trigger_autospells && md->master_id && md->special_state.ai != AI_NONE) {
		//Pass heritage to Master for status causing effects. [Skotlex]
		sd = map->id2sd(md->master_id);
		src = sd?&sd->bl:src;
	}

	if( attack_type&BF_WEAPON && skill_id != CR_REFLECTSHIELD ) {
		// Coma, Breaking Equipment
		if( sd && sd->special_state.bonus_coma ) {
			rate  = sd->weapon_coma_ele[tstatus->def_ele];
			rate += sd->weapon_coma_race[tstatus->race];
			rate += sd->weapon_coma_race[(tstatus->mode&MD_BOSS) ? RC_BOSS : RC_NONBOSS];
			if (rate)
				status->change_start(src, bl, SC_COMA, rate, 0, 0, src->id, 0, 0, SCFLAG_NONE);
		}
		if (sd && battle_config.equip_self_break_rate) {
			// Self weapon breaking
			rate = battle_config.equip_natural_break_rate;
			if( sc )
			{
				if(sc->data[SC_GIANTGROWTH])
					rate += 10;
				if(sc->data[SC_OVERTHRUST])
					rate += 10;
				if(sc->data[SC_OVERTHRUSTMAX])
					rate += 10;
			}
			if( rate )
				skill->break_equip(src, EQP_WEAPON, rate, BCT_SELF);
		}
		if (battle_config.equip_skill_break_rate && skill_id != WS_CARTTERMINATION && skill_id != ITM_TOMAHAWK) {
			// Cart Termination/Tomahawk won't trigger breaking data. Why? No idea, go ask Gravity.
			// Target weapon breaking
			rate = 0;
			if( sd )
				rate += sd->bonus.break_weapon_rate;
			if( sc && sc->data[SC_MELTDOWN] )
				rate += sc->data[SC_MELTDOWN]->val2;
			if( rate )
				skill->break_equip(bl, EQP_WEAPON, rate, BCT_ENEMY);

			// Target armor breaking
			rate = 0;
			if( sd )
				rate += sd->bonus.break_armor_rate;
			if( sc && sc->data[SC_MELTDOWN] )
				rate += sc->data[SC_MELTDOWN]->val3;
			if( rate )
				skill->break_equip(bl, EQP_ARMOR, rate, BCT_ENEMY);
		}
		if (sd && !skill_id && bl->type == BL_PC) { // This effect does not work with skills.
			if (sd->def_set_race[tstatus->race].rate)
					status->change_start(src,bl, SC_DEFSET, sd->def_set_race[tstatus->race].rate, sd->def_set_race[tstatus->race].value,
					0, 0, 0, sd->def_set_race[tstatus->race].tick, SCFLAG_FIXEDTICK);
			if (sd->mdef_set_race[tstatus->race].rate)
					status->change_start(src,bl, SC_MDEFSET, sd->mdef_set_race[tstatus->race].rate, sd->mdef_set_race[tstatus->race].value,
					0, 0, 0, sd->mdef_set_race[tstatus->race].tick, SCFLAG_FIXEDTICK);
		}
	}

	if( sd && sd->ed && sc && !status->isdead(bl) && !skill_id ) {
		struct unit_data *ud = unit->bl2ud(src);

		if( sc->data[SC_WILD_STORM_OPTION] )
			temp = sc->data[SC_WILD_STORM_OPTION]->val2;
		else if( sc->data[SC_UPHEAVAL_OPTION] )
			temp = sc->data[SC_UPHEAVAL_OPTION]->val2;
		else if( sc->data[SC_TROPIC_OPTION] )
			temp = sc->data[SC_TROPIC_OPTION]->val3;
		else if( sc->data[SC_CHILLY_AIR_OPTION] )
			temp = sc->data[SC_CHILLY_AIR_OPTION]->val3;
		else
			temp = 0;

		if ( rnd()%100 < 25 && temp ){
			skill->castend_damage_id(src, bl, temp, 5, tick, 0);

			if (ud) {
				rate = skill->delay_fix(src, temp, skill_lv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if ( battle_config.display_status_timers )
						clif->status_change(src, SI_POSTDELAY, 1, rate, 0, 0, 0);
				}
			}
		}
	}

	// Autospell when attacking
	if( sd && !status->isdead(bl) && sd->autospell[0].id ) {
		struct block_list *tbl;
		struct unit_data *ud;
		int i, auto_skill_lv, type, notok;

		for (i = 0; i < ARRAYLENGTH(sd->autospell) && sd->autospell[i].id; i++) {

			if(!(sd->autospell[i].flag&attack_type&BF_WEAPONMASK &&
				 sd->autospell[i].flag&attack_type&BF_RANGEMASK &&
				 sd->autospell[i].flag&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled

			temp = (sd->autospell[i].id > 0) ? sd->autospell[i].id : -sd->autospell[i].id;

			sd->state.autocast = 1;
			notok = skill->not_ok(temp, sd);
			sd->state.autocast = 0;

			if ( notok )
				continue;

			auto_skill_lv = sd->autospell[i].lv?sd->autospell[i].lv:1;
			if (auto_skill_lv < 0) auto_skill_lv = 1+rnd()%(-auto_skill_lv);

			rate = (!sd->state.arrow_atk) ? sd->autospell[i].rate : sd->autospell[i].rate / 2;

			if (rnd()%1000 >= rate)
				continue;

			tbl = (sd->autospell[i].id < 0) ? src : bl;

			if( (type = skill->get_casttype(temp)) == CAST_GROUND ) {
				int maxcount = 0;
				if( !(BL_PC&battle_config.skill_reiteration) &&
					skill->get_unit_flag(temp)&UF_NOREITERATION &&
					skill->check_unit_range(src,tbl->x,tbl->y,temp,auto_skill_lv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.skill_nofootset &&
					skill->get_unit_flag(temp)&UF_NOFOOTSET &&
					skill->check_unit_range2(src,tbl->x,tbl->y,temp,auto_skill_lv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.land_skill_limit &&
					(maxcount = skill->get_maxcount(temp, auto_skill_lv)) > 0
				  ) {
					int v;
					for(v=0;v<MAX_SKILLUNITGROUP && sd->ud.skillunit[v] && maxcount;v++) {
						if(sd->ud.skillunit[v]->skill_id == temp)
							maxcount--;
					}
					if( maxcount == 0 ) {
						continue;
					}
				}
			}
			if( battle_config.autospell_check_range &&
				!battle->check_range(src, tbl, skill->get_range2(src, temp,auto_skill_lv) + (temp == RG_CLOSECONFINE?0:1)) )
				continue;

			if (temp == AS_SONICBLOW)
				pc_stop_attack(sd); //Special case, Sonic Blow autospell should stop the player attacking.
			else if (temp == PF_SPIDERWEB) //Special case, due to its nature of coding.
				type = CAST_GROUND;

			sd->state.autocast = 1;
			skill->consume_requirement(sd,temp,auto_skill_lv,1);
			skill->toggle_magicpower(src, temp);
			switch (type) {
				case CAST_GROUND:
					skill->castend_pos2(src, tbl->x, tbl->y, temp, auto_skill_lv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill->castend_nodamage_id(src, tbl, temp, auto_skill_lv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill->castend_damage_id(src, tbl, temp, auto_skill_lv, tick, 0);
					break;
			}
			sd->state.autocast = 0;
			//Set canact delay. [Skotlex]
			ud = unit->bl2ud(src);
			if (ud) {
				rate = skill->delay_fix(src, temp, auto_skill_lv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if (battle_config.display_status_timers)
						clif->status_change(src, SI_POSTDELAY, 1, rate, 0, 0, 0);
				}
			}
		}
	}

	//Autobonus when attacking
	if( sd && sd->autobonus[0].rate )
	{
		int i;
		for( i = 0; i < ARRAYLENGTH(sd->autobonus); i++ )
		{
			if( rnd()%1000 >= sd->autobonus[i].rate )
				continue;
			if( sd->autobonus[i].active != INVALID_TIMER )
				continue;
			if(!(sd->autobonus[i].atk_type&attack_type&BF_WEAPONMASK &&
				 sd->autobonus[i].atk_type&attack_type&BF_RANGEMASK &&
				 sd->autobonus[i].atk_type&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled
			pc->exeautobonus(sd,&sd->autobonus[i]);
		}
	}

	//Polymorph
	if(sd && sd->bonus.classchange && attack_type&BF_WEAPON &&
		dstmd && !(tstatus->mode&MD_BOSS) &&
		(rnd()%10000 < sd->bonus.classchange))
	{
		struct mob_db *monster;
		int class_;
		temp = 0;
		do {
			do {
				class_ = rnd() % MAX_MOB_DB;
			} while (!mob->db_checkid(class_));

			rate = rnd() % 1000000;
			monster = mob->db(class_);
		} while (
			(monster->status.mode&(MD_BOSS|MD_PLANT) || monster->summonper[0] <= rate) &&
			(temp++) < 2000);
		if (temp < 2000)
			mob->class_change(dstmd,class_);
	}

	return 0;
}

void skill_additional_effect_unknown(struct block_list* src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int *attack_type, int *dmg_lv, int64 *tick) {
}

int skill_onskillusage(struct map_session_data *sd, struct block_list *bl, uint16 skill_id, int64 tick) {
	int temp, skill_lv, i, type, notok;
	struct block_list *tbl;

	if( sd == NULL || !skill_id )
		return 0;

	for( i = 0; i < ARRAYLENGTH(sd->autospell3) && sd->autospell3[i].flag; i++ ) {
		if( sd->autospell3[i].flag != skill_id )
			continue;

		if( sd->autospell3[i].lock )
			continue;  // autospell already being executed

		temp = (sd->autospell3[i].id > 0) ? sd->autospell3[i].id : -sd->autospell3[i].id;

		sd->state.autocast = 1;
		notok = skill->not_ok(temp, sd);
		sd->state.autocast = 0;

		if ( notok )
			continue;

		skill_lv = sd->autospell3[i].lv ? sd->autospell3[i].lv : 1;
		if( skill_lv < 0 ) skill_lv = 1 + rnd()%(-skill_lv);

		if( sd->autospell3[i].id >= 0 && bl == NULL )
			continue; // No target
		if( rnd()%1000 >= sd->autospell3[i].rate )
			continue;

		tbl = (sd->autospell3[i].id < 0) ? &sd->bl : bl;

		if( (type = skill->get_casttype(temp)) == CAST_GROUND ) {
			int maxcount = 0;
			if( !(BL_PC&battle_config.skill_reiteration) &&
				skill->get_unit_flag(temp)&UF_NOREITERATION &&
				skill->check_unit_range(&sd->bl,tbl->x,tbl->y,temp,skill_lv)
			  ) {
				continue;
			}
			if( BL_PC&battle_config.skill_nofootset &&
				skill->get_unit_flag(temp)&UF_NOFOOTSET &&
				skill->check_unit_range2(&sd->bl,tbl->x,tbl->y,temp,skill_lv)
			  ) {
				continue;
			}
			if( BL_PC&battle_config.land_skill_limit &&
				(maxcount = skill->get_maxcount(temp, skill_lv)) > 0
			  ) {
				int v;
				for(v=0;v<MAX_SKILLUNITGROUP && sd->ud.skillunit[v] && maxcount;v++) {
					if(sd->ud.skillunit[v]->skill_id == temp)
						maxcount--;
				}
				if( maxcount == 0 ) {
					continue;
				}
			}
		}
		if( battle_config.autospell_check_range &&
			!battle->check_range(&sd->bl, tbl, skill->get_range2(&sd->bl, temp,skill_lv) + (temp == RG_CLOSECONFINE?0:1)) )
			continue;

		sd->state.autocast = 1;
		sd->autospell3[i].lock = true;
		skill->consume_requirement(sd,temp,skill_lv,1);
		switch( type ) {
			case CAST_GROUND:   skill->castend_pos2(&sd->bl, tbl->x, tbl->y, temp, skill_lv, tick, 0); break;
			case CAST_NODAMAGE: skill->castend_nodamage_id(&sd->bl, tbl, temp, skill_lv, tick, 0); break;
			case CAST_DAMAGE:   skill->castend_damage_id(&sd->bl, tbl, temp, skill_lv, tick, 0); break;
		}
		sd->autospell3[i].lock = false;
		sd->state.autocast = 0;
	}

	if (sd->autobonus3[0].rate) {
		for( i = 0; i < ARRAYLENGTH(sd->autobonus3); i++ ) {
			if( rnd()%1000 >= sd->autobonus3[i].rate )
				continue;
			if( sd->autobonus3[i].active != INVALID_TIMER )
				continue;
			if( sd->autobonus3[i].atk_type != skill_id )
				continue;
			pc->exeautobonus(sd,&sd->autobonus3[i]);
		}
	}

	return 1;
}
/* Split off from skill->additional_effect, which is never called when the
 * attack skill kills the enemy. Place in this function counter status effects
 * when using skills (eg: Asura's sp regen penalty, or counter-status effects
 * from cards) that will take effect on the source, not the target. [Skotlex]
 * Note: Currently this function only applies to Extremity Fist and BF_WEAPON
 * type of skills, so not every instance of skill->additional_effect needs a call
 * to this one.
 */
int skill_counter_additional_effect(struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, int64 tick) {
	int rate;
	struct map_session_data *sd=NULL;
	struct map_session_data *dstsd=NULL;
	struct status_change *sc;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(skill_id > 0 && !skill_lv) return 0; // don't forget auto attacks! [celest]

	sd = BL_CAST(BL_PC, src);
	dstsd = BL_CAST(BL_PC, bl);
	sc = status->get_sc(src);

	if(dstsd && attack_type&BF_WEAPON) {
		//Counter effects.
		enum sc_type type;
		int i, time;
		for(i=0; i < ARRAYLENGTH(dstsd->addeff2) && dstsd->addeff2[i].flag; i++) {
			rate = dstsd->addeff2[i].rate;
			if (attack_type&BF_LONG)
				rate+=dstsd->addeff2[i].arrow_rate;
			if (!rate) continue;

			if ((dstsd->addeff2[i].flag&(ATF_LONG|ATF_SHORT)) != (ATF_LONG|ATF_SHORT)) {
				//Trigger has range consideration.
				if((dstsd->addeff2[i].flag&ATF_LONG && !(attack_type&BF_LONG)) ||
					(dstsd->addeff2[i].flag&ATF_SHORT && !(attack_type&BF_SHORT)))
					continue; //Range Failed.
			}
			type = dstsd->addeff2[i].id;
			time = skill->get_time2(status->sc2skill(type),7);

			if (dstsd->addeff2[i].flag&ATF_TARGET)
				status->change_start(bl,src,type,rate,7,0,0,0,time,SCFLAG_NONE);

			if (dstsd->addeff2[i].flag&ATF_SELF && !status->isdead(bl))
				status->change_start(bl,bl,type,rate,7,0,0,0,time,SCFLAG_NONE);
		}
	}

	switch(skill_id){
		case MO_EXTREMITYFIST:
			sc_start(src,src,SC_EXTREMITYFIST,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case GS_FULLBUSTER:
			sc_start(src,src,SC_BLIND,2*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case HFLI_SBR44: // [orn]
		case HVAN_EXPLOSION:
			if (src->type == BL_HOM) {
				struct homun_data *hd = BL_UCAST(BL_HOM, src);
				hd->homunculus.intimacy = 200;
				if (hd->master)
					clif->send_homdata(hd->master,SP_INTIMATE,hd->homunculus.intimacy/100);
			}
			break;
		case CR_GRANDCROSS:
		case NPC_GRANDDARKNESS:
			attack_type |= BF_WEAPON;
			break;
		case LG_HESPERUSLIT:
			if ( sc && sc->data[SC_FORCEOFVANGUARD] && sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 6 ) {
					char i;
					for( i = 0; i < sc->data[SC_FORCEOFVANGUARD]->val3 && sc->fv_counter <= sc->data[SC_FORCEOFVANGUARD]->val3 ; i++)
					clif->millenniumshield(bl, sc->fv_counter++);
				}
				break;
		default:
			skill->counter_additional_effect_unknown(src, bl, &skill_id, &skill_lv, &attack_type, &tick);
			break;
	}

	if( sd && (sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR
	 && rnd()%10000 < battle_config.sg_miracle_skill_ratio) // SG_MIRACLE [Komurka]
		sc_start(src,src,SC_MIRACLE,100,1,battle_config.sg_miracle_skill_duration);

	if( sd && skill_id && attack_type&BF_MAGIC && status->isdead(bl)
	 && !(skill->get_inf(skill_id)&(INF_GROUND_SKILL|INF_SELF_SKILL))
	 && (rate=pc->checkskill(sd,HW_SOULDRAIN)) > 0
	) {
		// Soul Drain should only work on targeted spells [Skotlex]
		if( pc_issit(sd) ) pc->setstand(sd); // Character stuck in attacking animation while 'sitting' fix. [Skotlex]
		if( skill->get_nk(skill_id)&NK_SPLASH && skill->area_temp[1] != bl->id )
			;
		else {
			clif->skill_nodamage(src,bl,HW_SOULDRAIN,rate,1);
			status->heal(src, 0, status->get_lv(bl)*(95+15*rate)/100, 2);
		}
	}

	if( sd && status->isdead(bl) ) {
		int sp = 0, hp = 0;
		if( attack_type&BF_WEAPON ) {
			sp += sd->bonus.sp_gain_value;
			sp += sd->sp_gain_race[status_get_race(bl)];
			sp += sd->sp_gain_race[is_boss(bl)?RC_BOSS:RC_NONBOSS];
			hp += sd->bonus.hp_gain_value;
		}
		if( attack_type&BF_MAGIC ) {
			sp += sd->bonus.magic_sp_gain_value;
			hp += sd->bonus.magic_hp_gain_value;
			if( skill_id == WZ_WATERBALL ) {// (bugreport:5303)
				if( sc->data[SC_SOULLINK]
				  && sc->data[SC_SOULLINK]->val2 == SL_WIZARD
				  && sc->data[SC_SOULLINK]->val3 == WZ_WATERBALL
				)
					sc->data[SC_SOULLINK]->val3 = 0; //Clear bounced spell check.
			}
		}
		if( hp || sp ) {
			// updated to force healing to allow healing through berserk
			status->heal(src, hp, sp, battle_config.show_hp_sp_gain ? 3 : 1);
		}
	}

	// Trigger counter-spells to retaliate against damage causing skills.
	if(dstsd && !status->isdead(bl) && dstsd->autospell2[0].id && !(skill_id && skill->get_nk(skill_id)&NK_NO_DAMAGE)) {
		struct block_list *tbl;
		struct unit_data *ud;
		int i, auto_skill_id, auto_skill_lv, type, notok;

		for (i = 0; i < ARRAYLENGTH(dstsd->autospell2) && dstsd->autospell2[i].id; i++) {

			if(!(dstsd->autospell2[i].flag&attack_type&BF_WEAPONMASK &&
				 dstsd->autospell2[i].flag&attack_type&BF_RANGEMASK &&
				 dstsd->autospell2[i].flag&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled

			auto_skill_id = (dstsd->autospell2[i].id > 0) ? dstsd->autospell2[i].id : -dstsd->autospell2[i].id;
			auto_skill_lv = dstsd->autospell2[i].lv?dstsd->autospell2[i].lv:1;
			if (auto_skill_lv < 0) auto_skill_lv = 1+rnd()%(-auto_skill_lv);

			rate = dstsd->autospell2[i].rate;
			if (attack_type&BF_LONG)
				 rate>>=1;

			dstsd->state.autocast = 1;
			notok = skill->not_ok(auto_skill_id, dstsd);
			dstsd->state.autocast = 0;

			if ( notok )
				continue;

			if (rnd()%1000 >= rate)
				continue;

			tbl = (dstsd->autospell2[i].id < 0) ? bl : src;

			if( (type = skill->get_casttype(auto_skill_id)) == CAST_GROUND ) {
				int maxcount = 0;
				if( !(BL_PC&battle_config.skill_reiteration) &&
					skill->get_unit_flag(auto_skill_id)&UF_NOREITERATION &&
					skill->check_unit_range(bl,tbl->x,tbl->y,auto_skill_id,auto_skill_lv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.skill_nofootset &&
					skill->get_unit_flag(auto_skill_id)&UF_NOFOOTSET &&
					skill->check_unit_range2(bl,tbl->x,tbl->y,auto_skill_id,auto_skill_lv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.land_skill_limit &&
					(maxcount = skill->get_maxcount(auto_skill_id, auto_skill_lv)) > 0
				  ) {
					int v;
					for(v=0;v<MAX_SKILLUNITGROUP && dstsd->ud.skillunit[v] && maxcount;v++) {
						if(dstsd->ud.skillunit[v]->skill_id == auto_skill_id)
							maxcount--;
					}
					if( maxcount == 0 ) {
						continue;
					}
				}
			}

			if( !battle->check_range(src, tbl, skill->get_range2(src, auto_skill_id,auto_skill_lv) + (auto_skill_id == RG_CLOSECONFINE?0:1)) && battle_config.autospell_check_range )
				continue;

			dstsd->state.autocast = 1;
			skill->consume_requirement(dstsd,auto_skill_id,auto_skill_lv,1);
			switch (type) {
				case CAST_GROUND:
					skill->castend_pos2(bl, tbl->x, tbl->y, auto_skill_id, auto_skill_lv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill->castend_nodamage_id(bl, tbl, auto_skill_id, auto_skill_lv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill->castend_damage_id(bl, tbl, auto_skill_id, auto_skill_lv, tick, 0);
					break;
			}
			dstsd->state.autocast = 0;
			// Set canact delay. [Skotlex]
			ud = unit->bl2ud(bl);
			if (ud) {
				rate = skill->delay_fix(bl, auto_skill_id, auto_skill_lv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if (battle_config.display_status_timers)
						clif->status_change(bl, SI_POSTDELAY, 1, rate, 0, 0, 0);
				}
			}
		}
	}

	//Autobonus when attacked
	if( dstsd && !status->isdead(bl) && dstsd->autobonus2[0].rate && !(skill_id && skill->get_nk(skill_id)&NK_NO_DAMAGE) ) {
		int i;
		for( i = 0; i < ARRAYLENGTH(dstsd->autobonus2); i++ ) {
			if( rnd()%1000 >= dstsd->autobonus2[i].rate )
				continue;
			if( dstsd->autobonus2[i].active != INVALID_TIMER )
				continue;
			if(!(dstsd->autobonus2[i].atk_type&attack_type&BF_WEAPONMASK &&
				 dstsd->autobonus2[i].atk_type&attack_type&BF_RANGEMASK &&
				 dstsd->autobonus2[i].atk_type&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled
			pc->exeautobonus(dstsd,&dstsd->autobonus2[i]);
		}
	}

	return 0;
}

void skill_counter_additional_effect_unknown(struct block_list* src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int *attack_type, int64 *tick) {
}

/*=========================================================================
 * Breaks equipment. On-non players causes the corresponding strip effect.
 * - rate goes from 0 to 10000 (100.00%)
 * - flag is a BCT_ flag to indicate which type of adjustment should be used
 *   (BCT_ENEMY/BCT_PARTY/BCT_SELF) are the valid values.
 *------------------------------------------------------------------------*/
int skill_break_equip (struct block_list *bl, unsigned short where, int rate, int flag)
{
	const int where_list[4]     = {EQP_WEAPON, EQP_ARMOR, EQP_SHIELD, EQP_HELM};
	const enum sc_type scatk[4] = {SC_NOEQUIPWEAPON, SC_NOEQUIPARMOR, SC_NOEQUIPSHIELD, SC_NOEQUIPHELM};
	const enum sc_type scdef[4] = {SC_PROTECTWEAPON, SC_PROTECTARMOR, SC_PROTECTSHIELD, SC_PROTECTHELM};
	struct status_change *sc = status->get_sc(bl);
	int i;
	struct map_session_data *sd = BL_CAST(BL_PC, bl);
	if (sc && !sc->count)
		sc = NULL;

	if (sd) {
		if (sd->bonus.unbreakable_equip)
			where &= ~sd->bonus.unbreakable_equip;
		if (sd->bonus.unbreakable)
			rate -= rate*sd->bonus.unbreakable/100;
		if (where&EQP_WEAPON) {
			switch (sd->status.weapon) {
				case W_FIST: //Bare fists should not break :P
				case W_1HAXE:
				case W_2HAXE:
				case W_MACE: // Axes and Maces can't be broken [DracoRPG]
				case W_2HMACE:
				case W_STAFF:
				case W_2HSTAFF:
				case W_BOOK: //Rods and Books can't be broken [Skotlex]
				case W_HUUMA:
					where &= ~EQP_WEAPON;
			}
		}
	}
	if (flag&BCT_ENEMY) {
		if (battle_config.equip_skill_break_rate != 100)
			rate = rate*battle_config.equip_skill_break_rate/100;
	} else if (flag&(BCT_PARTY|BCT_SELF)) {
		if (battle_config.equip_self_break_rate != 100)
			rate = rate*battle_config.equip_self_break_rate/100;
	}

	for (i = 0; i < 4; i++) {
		if (where&where_list[i]) {
			if (sc && sc->count && sc->data[scdef[i]])
				where&=~where_list[i];
			else if (rnd()%10000 >= rate)
				where&=~where_list[i];
			else if (!sd && !(status_get_mode(bl)&MD_BOSS)) //Cause Strip effect.
				sc_start(bl,bl,scatk[i],100,0,skill->get_time(status->sc2skill(scatk[i]),1));
		}
	}
	if (!where) //Nothing to break.
		return 0;
	if (sd) {
		for (i = 0; i < EQI_MAX; i++) {
			int j = sd->equip_index[i];
			if (j < 0 || sd->status.inventory[j].attribute == 1 || !sd->inventory_data[j])
				continue;

			switch(i) {
				case EQI_HEAD_TOP: //Upper Head
					flag = (where&EQP_HELM);
					break;
				case EQI_ARMOR: //Body
					flag = (where&EQP_ARMOR);
					break;
				case EQI_HAND_R: //Left/Right hands
				case EQI_HAND_L:
					flag = (
						(where&EQP_WEAPON && sd->inventory_data[j]->type == IT_WEAPON) ||
						(where&EQP_SHIELD && sd->inventory_data[j]->type == IT_ARMOR));
					break;
				case EQI_SHOES:
					flag = (where&EQP_SHOES);
					break;
				case EQI_GARMENT:
					flag = (where&EQP_GARMENT);
					break;
				default:
					continue;
			}
			if (flag) {
				sd->status.inventory[j].attribute = 1;
				pc->unequipitem(sd, j, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
			}
		}
		clif->equiplist(sd);
	}

	return where; //Return list of pieces broken.
}

int skill_strip_equip(struct block_list *bl, unsigned short where, int rate, int lv, int time) {
	struct status_change *sc;
	const int pos[5]             = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HELM, EQP_ACC};
	const enum sc_type sc_atk[5] = {SC_NOEQUIPWEAPON, SC_NOEQUIPSHIELD, SC_NOEQUIPARMOR, SC_NOEQUIPHELM, SC__STRIPACCESSARY};
	const enum sc_type sc_def[5] = {SC_PROTECTWEAPON, SC_PROTECTSHIELD, SC_PROTECTARMOR, SC_PROTECTHELM, 0};
	int i;

	if (rnd()%100 >= rate)
		return 0;

	sc = status->get_sc(bl);
	if (!sc || sc->option&OPTION_MADOGEAR ) // Mado Gear cannot be divested [Ind]
		return 0;

	for (i = 0; i < ARRAYLENGTH(pos); i++) {
		if (where&pos[i] && sc->data[sc_def[i]])
			where&=~pos[i];
	}
	if (!where) return 0;

	for (i = 0; i < ARRAYLENGTH(pos); i++) {
		if (where&pos[i] && !sc_start(bl, bl, sc_atk[i], 100, lv, time))
			where&=~pos[i];
	}
	return where?1:0;
}
/*=========================================================================
 Used to knock back players, monsters, traps, etc
 - 'count' is the number of squares to knock back
 - 'direction' indicates the way OPPOSITE to the knockback direction (or -1 for default behavior)
 - if 'flag&0x1', position update packets must not be sent.
 - if 'flag&0x2', skill blown ignores players' special_state.no_knockback
 -------------------------------------------------------------------------*/
int skill_blown(struct block_list* src, struct block_list* target, int count, int8 dir, int flag)
{
	int dx = 0, dy = 0;

	nullpo_ret(src);

	if (src != target && map->list[src->m].flag.noknockback)
		return 0; // No knocking
	if (count == 0)
		return 0; // Actual knockback distance is 0.

	switch (target->type) {
		case BL_MOB:
		{
			const struct mob_data *md = BL_UCCAST(BL_MOB, target);
			if (md->status.mode&MD_NOKNOCKBACK)
				return 0;
			if (src != target && is_boss(target)) // Bosses can't be knocked-back
				return 0;
		}
			break;
		case BL_PC:
		{
			struct map_session_data *sd = BL_UCAST(BL_PC, target);
			if (sd->sc.data[SC_BASILICA] && sd->sc.data[SC_BASILICA]->val4 == sd->bl.id && !is_boss(src))
				return 0; // Basilica caster can't be knocked-back by normal monsters.
			if (!(flag&0x2) && src != target && sd->special_state.no_knockback)
				return 0;
		}
			break;
		case BL_SKILL:
		{
			struct skill_unit *su = BL_UCAST(BL_SKILL, target);
			if (su->group && (su->group->unit_id == UNT_ANKLESNARE || su->group->unit_id == UNT_REVERBERATION))
				return 0; // ankle snare cannot be knocked back
		}
			break;
	}

	if (dir == -1) // <optimized>: do the computation here instead of outside
		dir = map->calc_dir(target, src->x, src->y); // direction from src to target, reversed

	if (dir >= 0 && dir < 8) {
		// take the reversed 'direction' and reverse it
		dx = -dirx[dir];
		dy = -diry[dir];
	}

	return unit->blown(target, dx, dy, count, flag); // send over the proper flag
}

/*
	Checks if 'bl' should reflect back a spell cast by 'src'.
	type is the type of magic attack: 0: indirect (aoe), 1: direct (targeted)
	In case of success returns type of reflection, otherwise 0
		1 - Regular reflection (Maya)
		2 - SL_KAITE reflection
*/
int skill_magic_reflect(struct block_list* src, struct block_list* bl, int type) {
	struct status_change *sc = status->get_sc(bl);
	struct map_session_data* sd = BL_CAST(BL_PC, bl);

	if( sc && sc->data[SC_KYOMU] ) // Nullify reflecting ability
		return  0;

	// item-based reflection
	if( sd && sd->bonus.magic_damage_return && type && rnd()%100 < sd->bonus.magic_damage_return )
		return 1;

	if( is_boss(src) )
		return 0;

	// status-based reflection
	if( !sc || sc->count == 0 )
		return 0;

	if( sc->data[SC_MAGICMIRROR] && rnd()%100 < sc->data[SC_MAGICMIRROR]->val2 )
		return 1;

	if( sc->data[SC_KAITE] && (src->type == BL_PC || status->get_lv(src) <= 80) )
	{// Kaite only works against non-players if they are low-level.
		clif->specialeffect(bl, 438, AREA);
		if( --sc->data[SC_KAITE]->val2 <= 0 )
			status_change_end(bl, SC_KAITE, INVALID_TIMER);
		return 2;
	}

	return 0;
}

/*
 * =========================================================================
 * Does a skill attack with the given properties.
 * src is the master behind the attack (player/mob/pet)
 * dsrc is the actual originator of the damage, can be the same as src, or a BL_SKILL
 * bl is the target to be attacked.
 * flag can hold a bunch of information:
 * flag&0xFFF is passed to the underlying battle_calc_attack for processing
 *      (usually holds number of targets, or just 1 for simple splash attacks)
 * flag&0x1000 is used to tag that this is a splash-attack (so the damage
 *      packet shouldn't display a skill animation)
 * flag&0x2000 is used to signal that the skill_lv should be passed as -1 to the
 *      client (causes player characters to not scream skill name)
 * flag&0x4000 - Return 0 if damage was reflected
 *-------------------------------------------------------------------------*/
int skill_attack(int attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag) {
	struct Damage dmg;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc;
	struct map_session_data *sd, *tsd;
	int type;
	int64 damage;
	bool rmdamage = false;//magic reflected
	bool additional_effects = true, shadow_flag = false;

	if(skill_id > 0 && !skill_lv) return 0;

	nullpo_ret(src); // Source is the master behind the attack (player/mob/pet)
	nullpo_ret(dsrc); // dsrc is the actual originator of the damage, can be the same as src, or a skill casted by src.
	nullpo_ret(bl); //Target to be attacked.

	if (src != dsrc) {
		//When caster is not the src of attack, this is a ground skill, and as such, do the relevant target checking. [Skotlex]
		if (!status->check_skilluse(battle_config.skill_caster_check?src:NULL, bl, skill_id, 2))
			return 0;
	} else if ((flag&SD_ANIMATION) && skill->get_nk(skill_id)&NK_SPLASH) {
		//Note that splash attacks often only check versus the targeted mob, those around the splash area normally don't get checked for being hidden/cloaked/etc. [Skotlex]
		if (!status->check_skilluse(src, bl, skill_id, 2))
			return 0;
	}

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, bl);

	// To block skills that aren't called via battle_check_target [Panikon]
	// issue: 8203
	if( sd
		&& ( (bl->type == BL_MOB && pc_has_permission(sd, PC_PERM_DISABLE_PVM))
			|| (bl->type == BL_PC && pc_has_permission(sd, PC_PERM_DISABLE_PVP)) )
			)
		return 0;

	sstatus = status->get_status_data(src);
	tstatus = status->get_status_data(bl);
	sc = status->get_sc(bl);
	if (sc && !sc->count) sc = NULL; //Don't need it.

	// Is this check really needed? FrostNova won't hurt you if you step right where the caster is?
	if(skill_id == WZ_FROSTNOVA && dsrc->x == bl->x && dsrc->y == bl->y)
		return 0;
	 //Trick Dead protects you from damage, but not from buffs and the like, hence it's placed here.
	if (sc && sc->data[SC_TRICKDEAD])
		return 0;
	if ( skill_id != HW_GRAVITATION ) {
		struct status_change *csc = status->get_sc(src);
		if(csc && csc->data[SC_GRAVITATION] && csc->data[SC_GRAVITATION]->val3 == BCT_SELF )
			return 0;
	}

	dmg = battle->calc_attack(attack_type,src,bl,skill_id,skill_lv,flag&0xFFF);

	//Skotlex: Adjusted to the new system
	if (src->type == BL_PET) { // [Valaris]
		struct pet_data *pd = BL_UCAST(BL_PET, src);
		if (pd->a_skill && pd->a_skill->div_ && pd->a_skill->id == skill_id) {
			int element = skill->get_ele(skill_id, skill_lv);
			/*if (skill_id == -1) Does it ever worked?
				element = sstatus->rhw.ele;*/
			if (element != ELE_NEUTRAL || !(battle_config.attack_attr_none&BL_PET))
				dmg.damage = battle->attr_fix(src, bl, skill_lv, element, tstatus->def_ele, tstatus->ele_lv);
			else
				dmg.damage= skill_lv;
			dmg.damage2=0;
			dmg.div_= pd->a_skill->div_;
		}
	}

	if( dmg.flag&BF_MAGIC
		&& (skill_id != NPC_EARTHQUAKE || (battle_config.eq_single_target_reflectable && (flag & 0xFFF) == 1)) ) { /* Need more info cause NPC_EARTHQUAKE is ground type */
		// Earthquake on multiple targets is not counted as a target skill. [Inkfish]
		int reflecttype;
		if ((dmg.damage || dmg.damage2) && (reflecttype = skill->magic_reflect(src, bl, src==dsrc)) != 0) {
			//Magic reflection, switch caster/target
			struct block_list *tbl = bl;
			rmdamage = true;
			bl = src;
			src = tbl;
			dsrc = tbl;
			sd = BL_CAST(BL_PC, src);
			tsd = BL_CAST(BL_PC, bl);
			sc = status->get_sc(bl);
			if (sc && !sc->count)
				sc = NULL; //Don't need it.
			/* bugreport:2564 flag&2 disables double casting trigger */
			flag |= 2;
			/* bugreport:7859 magical reflected zeroes blow count */
			dmg.blewcount = 0;
			//Spirit of Wizard blocks Kaite's reflection
			if (reflecttype == 2 && sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_WIZARD) {
				//Consume one Fragment per hit of the casted skill? [Skotlex]
				int consumeitem = tsd ? pc->search_inventory(tsd, ITEMID_FRAGMENT_OF_CRYSTAL) : 0;
				if (consumeitem != INDEX_NOT_FOUND) {
					if ( tsd ) pc->delitem(tsd, consumeitem, 1, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);
					dmg.damage = dmg.damage2 = 0;
					dmg.dmg_lv = ATK_MISS;
					sc->data[SC_SOULLINK]->val3 = skill_id;
					sc->data[SC_SOULLINK]->val4 = dsrc->id;
				}
			} else if( reflecttype != 2 ) /* Kaite bypasses */
				additional_effects = false;

		/**
		 * Official Magic Reflection Behavior : damage reflected depends on gears caster wears, not target
		 **/
		#if MAGIC_REFLECTION_TYPE

		#ifdef RENEWAL
			if( dmg.dmg_lv != ATK_MISS ) // Wiz SL canceled and consumed fragment
		#else
			// issue:6415 in pre-renewal Kaite reflected the entire damage received
			// regardless of caster's equipment (Aegis 11.1)
			if( dmg.dmg_lv != ATK_MISS && reflecttype == 1 ) //Wiz SL canceled and consumed fragment
		#endif
			{
				short s_ele = skill->get_ele(skill_id, skill_lv);

				if (s_ele == -1) // the skill takes the weapon's element
					s_ele = sstatus->rhw.ele;
				else if (s_ele == -2) //Use status element
					s_ele = status_get_attack_sc_element(src,status->get_sc(src));
				else if( s_ele == -3 ) //Use random element
					s_ele = rnd()%ELE_MAX;

				dmg.damage = battle->attr_fix(bl, bl, dmg.damage, s_ele, status_get_element(bl), status_get_element_level(bl));

				if( sc && sc->data[SC_ENERGYCOAT] ) {
					struct status_data *st = status->get_status_data(bl);
					int per = 100*st->sp / st->max_sp -1; //100% should be counted as the 80~99% interval
					per /=20; //Uses 20% SP intervals.
					//SP Cost: 1% + 0.5% per every 20% SP
					if (!status->charge(bl, 0, (10+5*per)*st->max_sp/1000))
						status_change_end(bl, SC_ENERGYCOAT, INVALID_TIMER);
					//Reduction: 6% + 6% every 20%
					dmg.damage -= dmg.damage * (6 * (1+per)) / 100;
				}
			}
		#endif /* MAGIC_REFLECTION_TYPE */
		}
		if(sc && sc->data[SC_MAGICROD] && src == dsrc) {
			int sp = skill->get_sp(skill_id,skill_lv);
			dmg.damage = dmg.damage2 = 0;
			dmg.dmg_lv = ATK_MISS; //This will prevent skill additional effect from taking effect. [Skotlex]
			sp = sp * sc->data[SC_MAGICROD]->val2 / 100;
			if(skill_id == WZ_WATERBALL && skill_lv > 1)
				sp = sp/((skill_lv|1)*(skill_lv|1)); //Estimate SP cost of a single water-ball
			status->heal(bl, 0, sp, 2);
		}
	}

	damage = dmg.damage + dmg.damage2;

	if( (skill_id == AL_INCAGI || skill_id == AL_BLESSING ||
		skill_id == CASH_BLESSING || skill_id == CASH_INCAGI ||
		skill_id == MER_INCAGI || skill_id == MER_BLESSING) && tsd && tsd->sc.data[SC_PROPERTYUNDEAD] )
		damage = 1;

	if( damage && sc && sc->data[SC_GENSOU] && dmg.flag&BF_MAGIC ){
		struct block_list *nbl;
		nbl = battle->get_enemy_area(bl,bl->x,bl->y,2,BL_CHAR,bl->id);
		if (nbl) { // Only one target is chosen.
			clif->skill_damage(bl, nbl, tick, status_get_amotion(src), 0, status_fix_damage(bl,nbl,damage * skill_lv / 10,0), 1, OB_OBOROGENSOU_TRANSITION_ATK, -1, BDT_SKILL);
		}
	}

	//Skill hit type
	type=(skill_id==0)?BDT_SPLASH:skill->get_hit(skill_id);

	if(damage < dmg.div_
		//Only skills that knockback even when they miss. [Skotlex]
		&& skill_id != CH_PALMSTRIKE)
		dmg.blewcount = 0;

	if(skill_id == CR_GRANDCROSS||skill_id == NPC_GRANDDARKNESS) {
		if(battle_config.gx_disptype) dsrc = src;
		if(src == bl) type = BDT_ENDURE;
		else flag|=SD_ANIMATION;
	}
	if(skill_id == NJ_TATAMIGAESHI) {
		dsrc = src; //For correct knockback.
		flag|=SD_ANIMATION;
	}

	if(sd) {
		int combo = 0; //Used to signal if this skill can be combo'ed later on.
		struct status_change_entry *sce;
		if ((sce = sd->sc.data[SC_COMBOATTACK])) {//End combo state after skill is invoked. [Skotlex]
			switch (skill_id) {
			case TK_TURNKICK:
			case TK_STORMKICK:
			case TK_DOWNKICK:
			case TK_COUNTER:
				if (pc->famerank(sd->status.char_id,MAPID_TAEKWON)) {//Extend combo time.
					sce->val1 = skill_id; //Update combo-skill
					sce->val3 = skill_id;
					if( sce->timer != INVALID_TIMER )
						timer->delete(sce->timer, status->change_timer);
					sce->timer = timer->add(tick+sce->val4, status->change_timer, src->id, SC_COMBOATTACK);
					break;
				}
				unit->cancel_combo(src); // Cancel combo wait
				break;
			default:
				skill->attack_combo1_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag, sce, &combo);
				break;
			}
		}
		switch(skill_id) {
			case MO_TRIPLEATTACK:
				if (pc->checkskill(sd, MO_CHAINCOMBO) > 0 || pc->checkskill(sd, SR_DRAGONCOMBO) > 0)
					combo=1;
				break;
			case MO_CHAINCOMBO:
				if(pc->checkskill(sd, MO_COMBOFINISH) > 0 && sd->spiritball > 0)
					combo=1;
				break;
			case MO_COMBOFINISH:
				if (sd->status.party_id>0) //bonus from SG_FRIEND [Komurka]
					party->skill_check(sd, sd->status.party_id, MO_COMBOFINISH, skill_lv);
				if (pc->checkskill(sd, CH_TIGERFIST) > 0 && sd->spiritball > 0)
					combo=1;
			/* Fall through */
			case CH_TIGERFIST:
				if (!combo && pc->checkskill(sd, CH_CHAINCRUSH) > 0 && sd->spiritball > 1)
					combo=1;
			/* Fall through */
			case CH_CHAINCRUSH:
				if (!combo && pc->checkskill(sd, MO_EXTREMITYFIST) > 0 && sd->spiritball > 0 && sd->sc.data[SC_EXPLOSIONSPIRITS])
					combo=1;
				break;
			case AC_DOUBLE:
				// AC_DOUBLE can start the combo with other monster types, but the
				// monster that's going to be hit by HT_POWER should be RC_BRUTE or RC_INSECT [Panikon]
				if (pc->checkskill(sd, HT_POWER)) {
					sc_start4(NULL,src,SC_COMBOATTACK,100,HT_POWER,0,1,0,2000);
					clif->combo_delay(src,2000);
				}
				break;
			case TK_COUNTER:
			{
				//bonus from SG_FRIEND [Komurka]
				int level;
				if(sd->status.party_id>0 && (level = pc->checkskill(sd,SG_FRIEND)) > 0)
					party->skill_check(sd, sd->status.party_id, TK_COUNTER,level);
			}
				break;
			case SL_STIN:
			case SL_STUN:
				if (skill_lv >= 7 && !sd->sc.data[SC_SMA_READY])
					sc_start(src, src,SC_SMA_READY,100,skill_lv,skill->get_time(SL_SMA, skill_lv));
				break;
			case GS_FULLBUSTER:
				//Can't attack nor use items until skill's delay expires. [Skotlex]
				sd->ud.attackabletime = sd->canuseitem_tick = sd->ud.canact_tick;
				break;
			case TK_DODGE:
				if( pc->checkskill(sd, TK_JUMPKICK) > 0 )
					combo = 1;
				break;
			case SR_DRAGONCOMBO:
				if( pc->checkskill(sd, SR_FALLENEMPIRE) > 0 )
					combo = 1;
				break;
			case SR_FALLENEMPIRE:
				if( pc->checkskill(sd, SR_TIGERCANNON) > 0 || pc->checkskill(sd, SR_GATEOFHELL) > 0 )
					combo = 1;
				break;
			default:
				skill->attack_combo2_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag, &combo);
				break;
		} //Switch End
		if (combo) { //Possible to chain
			combo = max(status_get_amotion(src), DIFF_TICK32(sd->ud.canact_tick, tick));
			sc_start2(NULL,src,SC_COMBOATTACK,100,skill_id,bl->id,combo);
			clif->combo_delay(src, combo);
		}
	}

	//Display damage.
	switch( skill_id ) {
		case PA_GOSPEL: //Should look like Holy Cross [Skotlex]
			dmg.dmotion = clif->skill_damage(dsrc,bl,tick,dmg.amotion,dmg.dmotion, damage, dmg.div_, CR_HOLYCROSS, -1, BDT_SPLASH);
			break;
		//Skills that need be passed as a normal attack for the client to display correctly.
		case HVAN_EXPLOSION:
		case NPC_SELFDESTRUCTION:
			if(src->type==BL_PC)
				dmg.blewcount = 10;
			dmg.amotion = 0; //Disable delay or attack will do no damage since source is dead by the time it takes effect. [Skotlex]
			// fall through
		case KN_AUTOCOUNTER:
		case NPC_CRITICALSLASH:
		case TF_DOUBLE:
		case GS_CHAINACTION:
			dmg.dmotion = clif->damage(src,bl,dmg.amotion,dmg.dmotion,damage,dmg.div_,dmg.type,dmg.damage2);
			break;

		case AS_SPLASHER:
			if( flag&SD_ANIMATION ) // the surrounding targets
				dmg.dmotion = clif->skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, BDT_SPLASH); // needs -1 as skill level
			else // the central target doesn't display an animation
				dmg.dmotion = clif->skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, BDT_SPLASH); // needs -2(!) as skill level
			break;
		case WL_HELLINFERNO:
		case SR_EARTHSHAKER:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,skill_id,-2,BDT_SKILL);
			break;
		case KO_MUCHANAGE:
			if( dmg.dmg_lv == ATK_FLEE )
				break;
		case WL_SOULEXPANSION:
		case WL_COMET:
		case NJ_HUUMA:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,skill_lv,BDT_MULTIHIT);
			break;
		case WL_CHAINLIGHTNING_ATK:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,WL_CHAINLIGHTNING,-2,BDT_SKILL);
			break;
		case LG_OVERBRAND_BRANDISH:
		case LG_OVERBRAND:
			/* Fall through */
			dmg.amotion = status_get_amotion(src) * 2;
		case LG_OVERBRAND_PLUSATK:
			dmg.dmotion = clif->skill_damage(dsrc,bl,tick,status_get_amotion(src),dmg.dmotion,damage,dmg.div_,skill_id,-1,BDT_SPLASH);
			break;
		case EL_FIRE_BOMB:
		case EL_FIRE_BOMB_ATK:
		case EL_FIRE_WAVE:
		case EL_FIRE_WAVE_ATK:
		case EL_FIRE_MANTLE:
		case EL_CIRCLE_OF_FIRE:
		case EL_FIRE_ARROW:
		case EL_ICE_NEEDLE:
		case EL_WATER_SCREW:
		case EL_WATER_SCREW_ATK:
		case EL_WIND_SLASH:
		case EL_TIDAL_WEAPON:
		case EL_ROCK_CRUSHER:
		case EL_ROCK_CRUSHER_ATK:
		case EL_HURRICANE:
		case EL_HURRICANE_ATK:
		case EL_TYPOON_MIS:
		case EL_TYPOON_MIS_ATK:
		case GN_CRAZYWEED_ATK:
		case KO_BAKURETSU:
		case NC_MAGMA_ERUPTION:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,-1,BDT_SPLASH);
			break;
		case GN_SLINGITEM_RANGEMELEEATK:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,GN_SLINGITEM,-2,BDT_SKILL);
			break;
		case SC_FEINTBOMB:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,skill_id,skill_lv,BDT_SPLASH);
			break;
		case EL_STONE_RAIN:
			dmg.dmotion = clif->skill_damage(dsrc,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,-1,(flag&1)?BDT_MULTIHIT:BDT_SPLASH);
			break;
		case WM_SEVERE_RAINSTORM_MELEE:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,WM_SEVERE_RAINSTORM,-2,BDT_SPLASH);
			break;
		case WM_REVERBERATION_MELEE:
		case WM_REVERBERATION_MAGIC:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,WM_REVERBERATION,-2,BDT_SKILL);
			break;
		case WL_TETRAVORTEX_FIRE:
		case WL_TETRAVORTEX_WATER:
		case WL_TETRAVORTEX_WIND:
		case WL_TETRAVORTEX_GROUND:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_, WL_TETRAVORTEX,-1,BDT_SPLASH);
			break;
		case HT_CLAYMORETRAP:
		case HT_BLASTMINE:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case RA_CLUSTERBOMB:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			dmg.dmotion = clif->skill_damage(src,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, (flag&SD_LEVEL) ? -1 : skill_lv, BDT_SPLASH);
			if( dsrc != src ) // avoid damage display redundancy
				break;
		case HT_LANDMINE:
			dmg.dmotion = clif->skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, type);
			break;
		case HW_GRAVITATION:
			dmg.dmotion = clif->damage(bl, bl, 0, 0, damage, 1, BDT_ENDURE, 0);
			break;
		case WZ_SIGHTBLASTER:
			dmg.dmotion = clif->skill_damage(src,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, (flag&SD_LEVEL) ? -1 : skill_lv, BDT_SPLASH);
			break;
		case AB_DUPLELIGHT_MELEE:
		case AB_DUPLELIGHT_MAGIC:
			dmg.amotion = 300;/* makes the damage value not overlap with previous damage (when displayed by the client) */
		default:
			skill->attack_display_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag, &type, &dmg, &damage);
			break;
	}

	map->freeblock_lock();

	if (damage > 0 && dmg.flag&BF_SKILL && tsd
	 && pc->checkskill(tsd,RG_PLAGIARISM)
	 && (!sc || !sc->data[SC_PRESERVE])
	 && damage < tsd->battle_status.hp
	) {
		//Updated to not be able to copy skills if the blow will kill you. [Skotlex]
		int copy_skill = skill_id, cidx = 0;
		/**
		 * Copy Referral: dummy skills should point to their source upon copying
		 **/
		switch( skill_id ) {
			case AB_DUPLELIGHT_MELEE:
			case AB_DUPLELIGHT_MAGIC:
				copy_skill = AB_DUPLELIGHT;
				break;
			case WL_CHAINLIGHTNING_ATK:
				copy_skill = WL_CHAINLIGHTNING;
				break;
			case WM_REVERBERATION_MELEE:
			case WM_REVERBERATION_MAGIC:
				copy_skill = WM_REVERBERATION;
				break;
			case WM_SEVERE_RAINSTORM_MELEE:
				copy_skill = WM_SEVERE_RAINSTORM;
				break;
			case GN_CRAZYWEED_ATK:
				copy_skill = GN_CRAZYWEED;
				break;
			case GN_HELLS_PLANT_ATK:
				copy_skill = GN_HELLS_PLANT;
				break;
			case GN_SLINGITEM_RANGEMELEEATK:
				copy_skill = GN_SLINGITEM;
				break;
			case LG_OVERBRAND_BRANDISH:
			case LG_OVERBRAND_PLUSATK:
				copy_skill = LG_OVERBRAND;
				break;
			default:
				copy_skill = skill->attack_copy_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag);
				break;
		}
		cidx = skill->get_index(copy_skill);
		if ((tsd->status.skill[cidx].id == 0 || tsd->status.skill[cidx].flag == SKILL_FLAG_PLAGIARIZED) &&
			can_copy(tsd,copy_skill,bl)) // Split all the check into their own function [Aru]
		{
			int lv, idx = 0;
			if (sc && sc->data[SC__REPRODUCE] && (lv = sc->data[SC__REPRODUCE]->val1) > 0) {
				//Level dependent and limitation.
				lv = min(lv,skill->get_max(copy_skill));

				if( tsd->reproduceskill_id ) {
					idx = skill->get_index(tsd->reproduceskill_id);
					if(tsd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED ) {
						tsd->status.skill[idx].id = 0;
						tsd->status.skill[idx].lv = 0;
						tsd->status.skill[idx].flag = 0;
						clif->deleteskill(tsd,tsd->reproduceskill_id);
					}
				}

				tsd->reproduceskill_id = copy_skill;
				pc_setglobalreg(tsd, script->add_str("REPRODUCE_SKILL"), copy_skill);
				pc_setglobalreg(tsd, script->add_str("REPRODUCE_SKILL_LV"), lv);

				tsd->status.skill[cidx].id = copy_skill;
				tsd->status.skill[cidx].lv = lv;
				tsd->status.skill[cidx].flag = SKILL_FLAG_PLAGIARIZED;
				clif->addskill(tsd,copy_skill);
			} else {
				int plagiarismlvl;
				lv = skill_lv;
				if ( tsd->cloneskill_id ) {
					idx = skill->get_index(tsd->cloneskill_id);
					if ( tsd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED){
						tsd->status.skill[idx].id = 0;
						tsd->status.skill[idx].lv = 0;
						tsd->status.skill[idx].flag = 0;
						clif->deleteskill(tsd,tsd->cloneskill_id);
					}
				}

				if ((plagiarismlvl = pc->checkskill(tsd,RG_PLAGIARISM)) < lv)
					lv = plagiarismlvl;

				tsd->cloneskill_id = copy_skill;
				pc_setglobalreg(tsd, script->add_str("CLONE_SKILL"), copy_skill);
				pc_setglobalreg(tsd, script->add_str("CLONE_SKILL_LV"), lv);

				tsd->status.skill[cidx].id = copy_skill;
				tsd->status.skill[cidx].lv = lv;
				tsd->status.skill[cidx].flag = SKILL_FLAG_PLAGIARIZED;
				clif->addskill(tsd,copy_skill);
			}
		}
	}

	if (dmg.dmg_lv >= ATK_MISS && (type = skill->get_walkdelay(skill_id, skill_lv)) > 0) {
		//Skills with can't walk delay also stop normal attacking for that
		//duration when the attack connects. [Skotlex]
		struct unit_data *ud = unit->bl2ud(src);
		if (ud && DIFF_TICK(ud->attackabletime, tick + type) < 0)
			ud->attackabletime = tick + type;
	}

	shadow_flag = skill->check_shadowform(bl, damage, dmg.div_);

	if( !dmg.amotion ) {
		//Instant damage
		if( (!sc || (!sc->data[SC_DEVOTION] && skill_id != CR_REFLECTSHIELD)) && !shadow_flag)
			status_fix_damage(src,bl,damage,dmg.dmotion); //Deal damage before knockback to allow stuff like firewall+storm gust combo.
		if( !status->isdead(bl) && additional_effects )
			skill->additional_effect(src,bl,skill_id,skill_lv,dmg.flag,dmg.dmg_lv,tick);
		if( damage > 0 ) //Counter status effects [Skotlex]
			skill->counter_additional_effect(src,bl,skill_id,skill_lv,dmg.flag,tick);
	}
	// Hell Inferno burning status only starts if Fire part hits.
	if( skill_id == WL_HELLINFERNO && dmg.damage > 0 && !(flag&ELE_DARK) )
		sc_start4(src,bl,SC_BURNING,55+5*skill_lv,skill_lv,0,src->id,0,skill->get_time(skill_id,skill_lv));
	// Apply knock back chance in SC_TRIANGLESHOT skill.
	else if( skill_id == SC_TRIANGLESHOT && rnd()%100 > (1 + skill_lv) )
		dmg.blewcount = 0;

	//Only knockback if it's still alive, otherwise a "ghost" is left behind. [Skotlex]
	//Reflected spells do not bounce back (bl == dsrc since it only happens for direct skills)
	if (dmg.blewcount > 0 && bl!=dsrc && !status->isdead(bl)) {
		int8 dir = -1; // default
		switch(skill_id) {//direction
			case MG_FIREWALL:
			case PR_SANCTUARY:
			case SC_TRIANGLESHOT:
			case SR_KNUCKLEARROW:
			case GN_WALLOFTHORN:
			case EL_FIRE_MANTLE:
				dir = unit->getdir(bl);// backwards
				break;
			// This ensures the storm randomly pushes instead of exactly a cell backwards per official mechanics.
			case WZ_STORMGUST:
				if(!battle_config.stormgust_knockback)
					dir = rnd()%8;
				break;
			case WL_CRIMSONROCK:
				dir = map->calc_dir(bl,skill->area_temp[4],skill->area_temp[5]);
				break;
			case MC_CARTREVOLUTION:
				dir = 6; // Official servers push target to the West
				break;
			default:
				dir = skill->attack_dir_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag);
				break;

		}

		/* monsters with skill lv higher than MAX_SKILL_LEVEL may get this value beyond the max depending on conditions, we cap to the system's limit */
		if (dsrc->type == BL_MOB && skill_lv > MAX_SKILL_LEVEL && dmg.blewcount > 25)
			dmg.blewcount = 25;

		//blown-specific handling
		switch( skill_id ) {
			case LG_OVERBRAND_BRANDISH:
				if( skill->blown(dsrc,bl,dmg.blewcount,dir,0) < dmg.blewcount )
					skill->addtimerskill(src, tick + status_get_amotion(src), bl->id, 0, 0, LG_OVERBRAND_PLUSATK, skill_lv, BF_WEAPON, flag|SD_ANIMATION);
				break;
			case SR_KNUCKLEARROW:
				if( skill->blown(dsrc,bl,dmg.blewcount,dir,0) && !(flag&4) ) {
					short dir_x, dir_y;
					dir_x = dirx[(dir+4)%8];
					dir_y = diry[(dir+4)%8];
					if (map->getcell(bl->m, bl, bl->x + dir_x, bl->y + dir_y, CELL_CHKNOPASS) != 0)
						skill->addtimerskill(src, tick + 300 * ((flag&2) ? 1 : 2), bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag|4);
				}
				break;
			default:
				skill->attack_blow_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag, &type, &dmg, &damage, &dir);
				break;
		}
	}

	//Delayed damage must be dealt after the knockback (it needs to know actual position of target)
	if (dmg.amotion){
		if( shadow_flag ){
			if( !status->isdead(bl) && additional_effects )
				skill->additional_effect(src,bl,skill_id,skill_lv,dmg.flag,dmg.dmg_lv,tick);
			if( dmg.flag > ATK_BLOCK )
				skill->counter_additional_effect(src,bl,skill_id,skill_lv,dmg.flag,tick);
		}else
			battle->delay_damage(tick, dmg.amotion,src,bl,dmg.flag,skill_id,skill_lv,damage,dmg.dmg_lv,dmg.dmotion, additional_effects);
	}

	if( sc && sc->data[SC_DEVOTION] && skill_id != PA_PRESSURE ) {
		struct status_change_entry *sce = sc->data[SC_DEVOTION];
		struct block_list *d_bl = map->id2bl(sce->val1);
		struct mercenary_data *d_md = BL_CAST(BL_MER, d_bl);
		struct map_session_data *d_sd = BL_CAST(BL_PC, d_bl);

		if (d_bl != NULL
		 && ((d_md != NULL && d_md->master && d_md->master->bl.id == bl->id) || (d_sd != NULL && d_sd->devotion[sce->val2] == bl->id))
		 && check_distance_bl(bl, d_bl, sce->val3)
		) {
			if(!rmdamage){
				clif->damage(d_bl,d_bl, 0, 0, damage, 0, BDT_NORMAL, 0);
				status_fix_damage(NULL,d_bl, damage, 0);
			} else{ //Reflected magics are done directly on the target not on paladin
				//This check is only for magical skill.
				//For BF_WEAPON skills types track var rdamage and function battle_calc_return_damage
				clif->damage(bl,bl, 0, 0, damage, 0, BDT_NORMAL, 0);
				status_fix_damage(bl,bl, damage, 0);
			}
		}
		else {
			status_change_end(bl, SC_DEVOTION, INVALID_TIMER);
			if( !dmg.amotion )
				status_fix_damage(src,bl,damage,dmg.dmotion);
		}
	}

	if(damage > 0 && !(tstatus->mode&MD_BOSS)) {
		if( skill_id == RG_INTIMIDATE ) {
			int rate = 50 + skill_lv * 5;
			rate = rate + (status->get_lv(src) - status->get_lv(bl));
			if(rnd()%100 < rate)
				skill->addtimerskill(src,tick + 800,bl->id,0,0,skill_id,skill_lv,0,flag);
		} else if( skill_id == SC_FATALMENACE )
			skill->addtimerskill(src,tick + 800,bl->id,skill->area_temp[4],skill->area_temp[5],skill_id,skill_lv,0,flag);
	}

	if(skill_id == CR_GRANDCROSS || skill_id == NPC_GRANDDARKNESS)
		dmg.flag |= BF_WEAPON;

	if( sd && src != bl && damage > 0 && ( dmg.flag&BF_WEAPON ||
		(dmg.flag&BF_MISC && (skill_id == RA_CLUSTERBOMB || skill_id == RA_FIRINGTRAP || skill_id == RA_ICEBOUNDTRAP || skill_id == RK_DRAGONBREATH || skill_id == RK_DRAGONBREATH_WATER)) ) )
	{
		if (battle_config.left_cardfix_to_right)
			battle->drain(sd, bl, dmg.damage, dmg.damage, tstatus->race, tstatus->mode&MD_BOSS);
		else
			battle->drain(sd, bl, dmg.damage, dmg.damage2, tstatus->race, tstatus->mode&MD_BOSS);
	}

	if( damage > 0 ) {
		/**
		 * Post-damage effects
		 **/
		switch( skill_id ) {
			case GC_VENOMPRESSURE:
			{
				struct status_change *ssc = status->get_sc(src);
				if( ssc && ssc->data[SC_POISONINGWEAPON] && rnd()%100 < 70 + 5*skill_lv ) {
					short rate = 100;
					if ( ssc->data[SC_POISONINGWEAPON]->val1 == 9 )// Oblivion Curse gives a 2nd success chance after the 1st one passes which is reducible. [Rytech]
						rate = 100 - tstatus->int_ * 4 / 5;
					sc_start(src, bl,ssc->data[SC_POISONINGWEAPON]->val2,rate,ssc->data[SC_POISONINGWEAPON]->val1,skill->get_time2(GC_POISONINGWEAPON,1) - (tstatus->vit + tstatus->luk) / 2 * 1000);
					status_change_end(src, SC_POISONINGWEAPON, INVALID_TIMER);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				}
			}
				break;
			case WM_METALICSOUND:
				status_zap(bl, 0, damage*100/(100*(110-pc->checkskill(sd,WM_LESSON)*10)));
				break;
			case SR_TIGERCANNON:
				status_zap(bl, 0, damage/10); // 10% of damage dealt
				break;
			default:
				skill->attack_post_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag);
				break;
		}
		if( sd )
			skill->onskillusage(sd, bl, skill_id, tick);
	}

	if (!(flag&2)
	 && (skill_id == MG_COLDBOLT || skill_id == MG_FIREBOLT || skill_id == MG_LIGHTNINGBOLT)
	 && (sc = status->get_sc(src)) != NULL
	 && sc->data[SC_DOUBLECASTING]
	 && rnd() % 100 < sc->data[SC_DOUBLECASTING]->val2
	) {
		//skill->addtimerskill(src, tick + dmg.div_*dmg.amotion, bl->id, 0, 0, skill_id, skill_lv, BF_MAGIC, flag|2);
		skill->addtimerskill(src, tick + dmg.amotion, bl->id, 0, 0, skill_id, skill_lv, BF_MAGIC, flag|2);
	}

	map->freeblock_unlock();

	if ((flag&0x4000) && rmdamage == 1)
		return 0; //Should return 0 when damage was reflected

	return (int)cap_value(damage,INT_MIN,INT_MAX);
}

void skill_attack_combo1_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, struct status_change_entry *sce, int *combo) {
	if (src == dsrc) // Ground skills are exceptions. [Inkfish]
		status_change_end(src, SC_COMBOATTACK, INVALID_TIMER);
}

void skill_attack_combo2_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, int *combo) {
}

void skill_attack_display_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, int *type, struct Damage *dmg, int64 *damage) {
	if (*flag & SD_ANIMATION && dmg->div_ < 2) //Disabling skill animation doesn't works on multi-hit.
		*type = BDT_SPLASH;
	if (bl->type == BL_SKILL) {
		struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
		if (su->group && skill->get_inf2(su->group->skill_id) & INF2_TRAP)  // show damage on trap targets
			clif->skill_damage(src, bl, *tick, dmg->amotion, dmg->dmotion, *damage, dmg->div_, *skill_id, (*flag & SD_LEVEL) ? -1 : *skill_lv, BDT_SPLASH);
	}
	dmg->dmotion = clif->skill_damage(dsrc, bl, *tick, dmg->amotion, dmg->dmotion, *damage, dmg->div_, *skill_id, (*flag & SD_LEVEL) ? -1 : *skill_lv, *type);
}

int skill_attack_copy_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag) {
    return *skill_id;
}

int skill_attack_dir_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag) {
    return -1;
}

void skill_attack_blow_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, int *type, struct Damage *dmg, int64 *damage, int8 *dir) {
	skill->blown(dsrc, bl, dmg->blewcount, *dir, 0x0);
	if (!dmg->blewcount && bl->type == BL_SKILL && *damage > 0){
		struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
		if (su->group && su->group->skill_id == HT_BLASTMINE)
			skill->blown(src, bl, 3, -1, 0);
	}
}

void skill_attack_post_unknown(int *attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag) {
}

/*==========================================
 * sub function for recursive skill call.
 * Checking bl battle flag and display damage
 * then call func with source,target,skill_id,skill_lv,tick,flag
 *------------------------------------------*/
int skill_area_sub(struct block_list *bl, va_list ap) {
	struct block_list *src;
	uint16 skill_id,skill_lv;
	int flag;
	int64 tick;
	SkillFunc func;

	nullpo_ret(bl);

	src = va_arg(ap,struct block_list *);
	skill_id = va_arg(ap,int);
	skill_lv = va_arg(ap,int);
	tick = va_arg(ap,int64);
	flag = va_arg(ap,int);
	func = va_arg(ap,SkillFunc);

	if(battle->check_target(src,bl,flag) > 0) {
		// several splash skills need this initial dummy packet to display correctly
		if (flag&SD_PREAMBLE && skill->area_temp[2] == 0)
			clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);

		if (flag&(SD_SPLASH|SD_PREAMBLE))
			skill->area_temp[2]++;

		return func(src,bl,skill_id,skill_lv,tick,flag);
	}
	return 0;
}

int skill_check_unit_range_sub(struct block_list *bl, va_list ap)
{
	const struct skill_unit *su = NULL;
	uint16 skill_id,g_skill_id;

	nullpo_ret(bl);

	if (bl->type != BL_SKILL || bl->prev == NULL)
		return 0;

	su = BL_UCCAST(BL_SKILL, bl);

	if(!su->alive)
		return 0;

	skill_id = va_arg(ap,int);
	g_skill_id = su->group->skill_id;

	switch (skill_id) {
		case AL_PNEUMA:
			if(g_skill_id == SA_LANDPROTECTOR)
				break;
		case MG_SAFETYWALL:
		case MH_STEINWAND:
		case SC_MAELSTROM:
		case SO_ELEMENTAL_SHIELD:
			if(g_skill_id != MH_STEINWAND && g_skill_id != MG_SAFETYWALL && g_skill_id != AL_PNEUMA && g_skill_id != SC_MAELSTROM && g_skill_id != SO_ELEMENTAL_SHIELD)
				return 0;
			break;
		case AL_WARP:
		case HT_SKIDTRAP:
		case MA_SKIDTRAP:
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_ANKLESNARE:
		case HT_SHOCKWAVE:
		case HT_SANDMAN:
		case MA_SANDMAN:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
		case HT_TALKIEBOX:
		case HP_BASILICA:
		case RA_ELECTRICSHOCKER:
		case RA_CLUSTERBOMB:
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
		case SC_DIMENSIONDOOR:
		case SC_BLOODYLUST:
		case SC_CHAOSPANIC:
		case GN_HELLS_PLANT:
			//Non stackable on themselves and traps (including venom dust which does not has the trap inf2 set)
			if (skill_id != g_skill_id && !(skill->get_inf2(g_skill_id)&INF2_TRAP) && g_skill_id != AS_VENOMDUST && g_skill_id != MH_POISON_MIST)
				return 0;
			break;
		default: //Avoid stacking with same kind of trap. [Skotlex]
			if (g_skill_id != skill_id)
				return 0;
			break;
	}

	return 1;
}

int skill_check_unit_range (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv) {
	//Non players do not check for the skill's splash-trigger area.
	int range = bl->type == BL_PC ? skill->get_unit_range(skill_id, skill_lv):0;
	int layout_type = skill->get_unit_layout_type(skill_id,skill_lv);
	if ( layout_type == - 1 || layout_type > MAX_SQUARE_LAYOUT ) {
		ShowError("skill_check_unit_range: unsupported layout type %d for skill %d\n",layout_type,skill_id);
		return 0;
	}

	range += layout_type;
	return map->foreachinarea(skill->check_unit_range_sub,bl->m,x-range,y-range,x+range,y+range,BL_SKILL,skill_id);
}

int skill_check_unit_range2_sub (struct block_list *bl, va_list ap) {
	uint16 skill_id;

	if(bl->prev == NULL)
		return 0;

	skill_id = va_arg(ap,int);

	if( status->isdead(bl) && skill_id != AL_WARP )
		return 0;

	if( skill_id == HP_BASILICA && bl->type == BL_PC )
		return 0;

	if (skill_id == AM_DEMONSTRATION && bl->type == BL_MOB && BL_UCCAST(BL_MOB, bl)->class_ == MOBID_EMPELIUM)
		return 0; //Allow casting Bomb/Demonstration Right under emperium [Skotlex]
	return 1;
}

int skill_check_unit_range2 (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv) {
	int range, type;

	switch (skill_id) {
		// to be expanded later
		case WZ_ICEWALL:
			range = 2;
			break;
		case SC_MANHOLE:
			range = 0;
			break;
		case GN_HELLS_PLANT:
			range = 0;
			break;
		default: {
				int layout_type = skill->get_unit_layout_type(skill_id,skill_lv);
				if (layout_type==-1 || layout_type>MAX_SQUARE_LAYOUT) {
					ShowError("skill_check_unit_range2: unsupported layout type %d for skill %d\n",layout_type,skill_id);
					return 0;
				}
				range = skill->get_unit_range(skill_id,skill_lv) + layout_type;
			}
			break;
	}

	// if the caster is a monster/NPC, only check for players
	// otherwise just check characters
	if (bl->type == BL_PC)
		type = BL_CHAR;
	else
		type = BL_PC;

	return map->foreachinarea(skill->check_unit_range2_sub, bl->m,
			x - range, y - range, x + range, y + range,
			type, skill_id);
}

/*==========================================
 * Checks that you have the requirements for casting a skill for homunculus/mercenary.
 * Flag:
 * &1: finished casting the skill (invoke hp/sp/item consumption)
 * &2: picked menu entry (Warp Portal, Teleport and other menu based skills)
 *------------------------------------------*/
int skill_check_condition_mercenary(struct block_list *bl, int skill_id, int lv, int type) {
	struct status_data *st;
	struct map_session_data *sd = NULL;
	int i, hp, sp, hp_rate, sp_rate, state, mhp;
	uint16 idx;
	int itemid[MAX_SKILL_ITEM_REQUIRE],amount[ARRAYLENGTH(itemid)],index[ARRAYLENGTH(itemid)];

	if( lv < 1 || lv > MAX_SKILL_LEVEL )
		return 0;
	nullpo_ret(bl);

	switch( bl->type ) {
		case BL_HOM: sd = BL_UCAST(BL_HOM, bl)->master; break;
		case BL_MER: sd = BL_UCAST(BL_MER, bl)->master; break;
	}

	st = status->get_status_data(bl);
	if( (idx = skill->get_index(skill_id)) == 0 )
		return 0;

	// Requirements
	for( i = 0; i < ARRAYLENGTH(itemid); i++ )
	{
		itemid[i] = skill->dbs->db[idx].itemid[i];
		amount[i] = skill->dbs->db[idx].amount[i];
	}
	hp = skill->dbs->db[idx].hp[lv-1];
	sp = skill->dbs->db[idx].sp[lv-1];
	hp_rate = skill->dbs->db[idx].hp_rate[lv-1];
	sp_rate = skill->dbs->db[idx].sp_rate[lv-1];
	state = skill->dbs->db[idx].state;
	if( (mhp = skill->dbs->db[idx].mhp[lv-1]) > 0 )
		hp += (st->max_hp * mhp) / 100;
	if( hp_rate > 0 )
		hp += (st->hp * hp_rate) / 100;
	else
		hp += (st->max_hp * (-hp_rate)) / 100;
	if( sp_rate > 0 )
		sp += (st->sp * sp_rate) / 100;
	else
		sp += (st->max_sp * (-sp_rate)) / 100;

	if( bl->type == BL_HOM ) { // Intimacy Requirements
		struct homun_data *hd = BL_CAST(BL_HOM, bl);
		switch( skill_id ) {
			case HFLI_SBR44:
				if( hd->homunculus.intimacy <= 200 )
					return 0;
				break;
			case HVAN_EXPLOSION:
				if( hd->homunculus.intimacy < (unsigned int)battle_config.hvan_explosion_intimate )
					return 0;
				break;
		}
	}

	if( !(type&2) ) {
		if( hp > 0 && st->hp <= (unsigned int)hp ) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0);
			return 0;
		}
		if( sp > 0 && st->sp <= (unsigned int)sp ) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT, 0);
			return 0;
		}
	}

	if( !type )
		switch( state ) {
			case ST_MOVE_ENABLE:
				if( !unit->can_move(bl) ) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
					return 0;
				}
				break;
		}
	if( !(type&1) )
		return 1;

	// Check item existences
	for (i = 0; i < ARRAYLENGTH(itemid); i++) {
		index[i] = INDEX_NOT_FOUND;
		if (itemid[i] < 1) continue; // No item
		index[i] = pc->search_inventory(sd, itemid[i]);
		if (index[i] == INDEX_NOT_FOUND || sd->status.inventory[index[i]].amount < amount[i]) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_NEED_ITEM, amount[i]|(itemid[i] << 16));
			return 0;
		}
	}

	// Consume items
	for (i = 0; i < ARRAYLENGTH(itemid); i++) {
		if (index[i] != INDEX_NOT_FOUND)
			pc->delitem(sd, index[i], amount[i], 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);
	}

	if( type&2 )
		return 1;

	if( sp || hp )
		status_zap(bl, hp, sp);

	return 1;
}

/*==========================================
 * what the hell it doesn't need to receive this many params, it doesn't do anything ~_~
 *------------------------------------------*/
int skill_area_sub_count(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 tick, int flag) {
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_timerskill(int tid, int64 tick, int id, intptr_t data) {
	struct block_list *src = map->id2bl(id),*target = NULL;
	struct unit_data *ud = unit->bl2ud(src);
	struct skill_timerskill *skl;
	int range;

	nullpo_ret(src);
	nullpo_ret(ud);
	skl = ud->skilltimerskill[data];
	nullpo_ret(skl);
	ud->skilltimerskill[data] = NULL;

	do {
		if(src->prev == NULL)
			break; // Source not on Map
		if(skl->target_id) {
			target = map->id2bl(skl->target_id);
			if( ( skl->skill_id == RG_INTIMIDATE || skl->skill_id == SC_FATALMENACE ) && (!target || target->prev == NULL || !check_distance_bl(src,target,AREA_SIZE)) )
				target = src; //Required since it has to warp.
			if(target == NULL)
				break; // Target offline?
			if(target->prev == NULL)
				break; // Target not on Map
			if(src->m != target->m)
				break; // Different Maps
			if(status->isdead(src)){
				// Exceptions
				switch(skl->skill_id){
					case WL_CHAINLIGHTNING_ATK:
					case WL_TETRAVORTEX_FIRE:
					case WL_TETRAVORTEX_WATER:
					case WL_TETRAVORTEX_WIND:
					case WL_TETRAVORTEX_GROUND:
					// SR_FLASHCOMBO
					case SR_DRAGONCOMBO:
					case SR_FALLENEMPIRE:
					case SR_TIGERCANNON:
					case SR_SKYNETBLOW:
						break;
					default:
						if (!skill->timerskill_dead_unknown(src, ud, skl))
							continue; // Caster is Dead
				}
			}
			if(status->isdead(target) && skl->skill_id != RG_INTIMIDATE && skl->skill_id != WZ_WATERBALL)
				break;

			switch(skl->skill_id) {
				case RG_INTIMIDATE:
					if (unit->warp(src,-1,-1,-1,CLR_TELEPORT) == 0) {
						short x,y;
						map->search_freecell(src, 0, &x, &y, 1, 1, 0);
						if (target != src && !status->isdead(target))
							unit->warp(target, -1, x, y, CLR_TELEPORT);
					}
					break;
				case BA_FROSTJOKER:
				case DC_SCREAM:
					range= skill->get_splash(skl->skill_id, skl->skill_lv);
					map->foreachinarea(skill->frostjoke_scream,skl->map,skl->x-range,skl->y-range,
					                   skl->x+range,skl->y+range,BL_CHAR,src,skl->skill_id,skl->skill_lv,tick);
					break;
				case KN_AUTOCOUNTER:
					clif->skill_nodamage(src,target,skl->skill_id,skl->skill_lv,1);
					break;
				case WZ_WATERBALL:
					skill->toggle_magicpower(src, skl->skill_id); // only the first hit will be amplify
					if (!status->isdead(target))
						skill->attack(BF_MAGIC,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
					if (skl->type>1 && !status->isdead(target) && !status->isdead(src)) {
						skill->addtimerskill(src,tick+125,target->id,0,0,skl->skill_id,skl->skill_lv,skl->type-1,skl->flag);
					} else {
						struct status_change *sc = status->get_sc(src);
						if(sc) {
							if(sc->data[SC_SOULLINK] &&
								sc->data[SC_SOULLINK]->val2 == SL_WIZARD &&
								sc->data[SC_SOULLINK]->val3 == skl->skill_id)
								sc->data[SC_SOULLINK]->val3 = 0; //Clear bounced spell check.
						}
					}
					break;
				/**
				 * Warlock
				 **/
				case WL_CHAINLIGHTNING_ATK:
					skill->attack(BF_MAGIC, src, src, target, skl->skill_id, skl->skill_lv, tick, (9-skl->type)); // Hit a Lightning on the current Target
					skill->toggle_magicpower(src, skl->skill_id); // only the first hit will be amplify

					if (skl->type < (4 + skl->skill_lv - 1) && skl->x < 3) {
						// Remaining Chains Hit
						struct block_list *nbl = battle->get_enemy_area(src, target->x, target->y, (skl->type>2)?2:3, /* After 2 bounces, it will bounce to other targets in 7x7 range. */
								BL_CHAR|BL_SKILL, target->id); // Search for a new Target around current one...
						if (nbl == NULL)
							skl->x++;
						else
							skl->x = 0;

						skill->addtimerskill(src, tick + 651, (nbl?nbl:target)->id, skl->x, 0, WL_CHAINLIGHTNING_ATK, skl->skill_lv, skl->type + 1, skl->flag);
					}
					break;
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
					clif->skill_nodamage(src, target, skl->skill_id, skl->skill_lv, 1);
					skill->attack(BF_MAGIC, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
					skill->toggle_magicpower(src, skl->skill_id); // only the first hit will be amplify
					if( skl->type == 4 ){
						const enum sc_type scs[] = { SC_BURNING, SC_BLOODING, SC_FROSTMISTY, SC_STUN }; // status inflicts are depend on what summoned element is used.
						int rate = skl->y, index = skl->x-1;
						sc_start2(src,target, scs[index], rate, skl->skill_lv, src->id, skill->get_time(WL_TETRAVORTEX,index+1));
					}
					break;
				case WM_REVERBERATION_MELEE:
				case WM_REVERBERATION_MAGIC:
					skill->attack(skill->get_type(skl->skill_id),src, src, target, skl->skill_id, skl->skill_lv, 0, SD_LEVEL);
					break;
				case SC_FATALMENACE:
					if( src == target ) // Casters Part
						unit->warp(src, -1, skl->x, skl->y, CLR_TELEPORT);
					else { // Target's Part
						short x = skl->x, y = skl->y;
						map->search_freecell(NULL, target->m, &x, &y, 2, 2, 1);
						unit->warp(target,-1,x,y,CLR_TELEPORT);
					}
					break;
				case LG_MOONSLASHER:
				case SR_WINDMILL:
					if (target->type == BL_PC) {
						struct map_session_data *tsd = BL_UCAST(BL_PC, target);
						if (!pc_issit(tsd)) {
							pc_setsit(tsd);
							skill->sit(tsd,1);
							clif->sitting(&tsd->bl);
						}
					}
					break;
				case SR_KNUCKLEARROW:
					skill->attack(BF_WEAPON, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag|SD_LEVEL);
					break;
				case GN_SPORE_EXPLOSION:
					map->foreachinrange(skill->area_sub, target, skill->get_splash(skl->skill_id, skl->skill_lv), BL_CHAR,
					                    src, skl->skill_id, skl->skill_lv, (int64)0, skl->flag|1|BCT_ENEMY, skill->castend_damage_id);
					break;
				// SR_FLASHCOMBO
				case SR_DRAGONCOMBO:
				case SR_FALLENEMPIRE:
				case SR_TIGERCANNON:
				case SR_SKYNETBLOW:
					if (src->type == BL_PC) {
						struct map_session_data *sd = BL_UCAST(BL_PC, src);
						if( distance_xy(src->x, src->y, target->x, target->y) >= 3 )
							break;

						skill->castend_damage_id(src, target, skl->skill_id, pc->checkskill(sd, skl->skill_id), tick, 0);
					}
					break;
				case SC_ESCAPE:
					if( skl->type < 4+skl->skill_lv ){
						clif->skill_damage(src,src,tick,0,0,-30000,1,skl->skill_id,skl->skill_lv,BDT_SPLASH);
						skill->blown(src,src,1,unit->getdir(src),0);
						skill->addtimerskill(src,tick+80,src->id,0,0,skl->skill_id,skl->skill_lv,skl->type+1,0);
					}
					break;
				case RK_HUNDREDSPEAR:
					if (src->type == BL_PC) {
						struct map_session_data *sd = BL_UCAST(BL_PC, src);
						int skill_lv = pc->checkskill(sd, KN_SPEARBOOMERANG);
						if (skill_lv > 0)
							skill->attack(BF_WEAPON, src, src, target, KN_SPEARBOOMERANG, skill_lv, tick, skl->flag);
					} else {
						skill->attack(BF_WEAPON, src, src, target, KN_SPEARBOOMERANG, 1, tick, skl->flag);
					}
					break;
				case CH_PALMSTRIKE:
				{
					struct status_change* tsc = status->get_sc(target);
					struct status_change* sc = status->get_sc(src);
					if( (tsc && tsc->option&OPTION_HIDE)
					 || (sc && sc->option&OPTION_HIDE)
					) {
						skill->blown(src,target,skill->get_blewcount(skl->skill_id, skl->skill_lv), -1, 0x0 );
						break;
					}
				}
				default:
					skill->timerskill_target_unknown(tid, tick, src, target, ud, skl);
					break;
			}
		} else {
			if(src->m != skl->map)
				break;
			switch( skl->skill_id ) {
				case WZ_METEOR:
					if( skl->type >= 0 ) {
						int x = skl->type>>16, y = skl->type&0xFFFF;
						if( path->search_long(NULL, src, src->m, src->x, src->y, x, y, CELL_CHKWALL) )
							skill->unitsetting(src,skl->skill_id,skl->skill_lv,x,y,skl->flag);
						if( path->search_long(NULL, src, src->m, src->x, src->y, skl->x, skl->y, CELL_CHKWALL)
							&& !map->getcell(src->m, src, skl->x, skl->y, CELL_CHKLANDPROTECTOR) )
							clif->skill_poseffect(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,tick);
					}
					else if( path->search_long(NULL, src, src->m, src->x, src->y, skl->x, skl->y, CELL_CHKWALL) )
						skill->unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,skl->flag);
					break;
				case GN_CRAZYWEED_ATK: {
						int dummy = 1, i = skill->get_unit_range(skl->skill_id,skl->skill_lv);

						map->foreachinarea(skill->cell_overlap,src->m,skl->x-i,skl->y-i,skl->x+i,skl->y+i,BL_SKILL,skl->skill_id,&dummy,src);
					}
				// fall through ...
				case WL_EARTHSTRAIN:
					skill->unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,(skl->type<<16)|skl->flag);
					break;
				case LG_OVERBRAND_BRANDISH:
					skill->area_temp[1] = 0;
					map->foreachinpath(skill->attack_area,src->m,src->x,src->y,skl->x,skl->y,4,2,BL_CHAR,
						skill->get_type(skl->skill_id),src,src,skl->skill_id,skl->skill_lv,tick,skl->flag,BCT_ENEMY);
					break;
				default:
					skill->timerskill_notarget_unknown(tid, tick, src, ud, skl);
					break;
			}
		}
	} while (0);
	//Free skl now that it is no longer needed.
	ers_free(skill->timer_ers, skl);
	return 0;
}

bool skill_timerskill_dead_unknown(struct block_list *src, struct unit_data *ud, struct skill_timerskill *skl)
{
	return false;
}

void skill_timerskill_target_unknown(int tid, int64 tick, struct block_list *src, struct block_list *target, struct unit_data *ud, struct skill_timerskill *skl)
{
	skill->attack(skl->type, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
}

void skill_timerskill_notarget_unknown(int tid, int64 tick, struct block_list *src, struct unit_data *ud, struct skill_timerskill *skl)
{
}

/*==========================================
 *
 *------------------------------------------*/
int skill_addtimerskill(struct block_list *src, int64 tick, int target, int x,int y, uint16 skill_id, uint16 skill_lv, int type, int flag) {
	int i;
	struct unit_data *ud;
	nullpo_retr(1, src);
	if (src->prev == NULL)
		return 0;
	ud = unit->bl2ud(src);
	nullpo_retr(1, ud);

	ARR_FIND( 0, MAX_SKILLTIMERSKILL, i, ud->skilltimerskill[i] == 0 );
	if( i == MAX_SKILLTIMERSKILL ) return 1;

	ud->skilltimerskill[i] = ers_alloc(skill->timer_ers, struct skill_timerskill);
	ud->skilltimerskill[i]->timer = timer->add(tick, skill->timerskill, src->id, i);
	ud->skilltimerskill[i]->src_id = src->id;
	ud->skilltimerskill[i]->target_id = target;
	ud->skilltimerskill[i]->skill_id = skill_id;
	ud->skilltimerskill[i]->skill_lv = skill_lv;
	ud->skilltimerskill[i]->map = src->m;
	ud->skilltimerskill[i]->x = x;
	ud->skilltimerskill[i]->y = y;
	ud->skilltimerskill[i]->type = type;
	ud->skilltimerskill[i]->flag = flag;
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_cleartimerskill (struct block_list *src)
{
	int i;
	struct unit_data *ud;
	nullpo_ret(src);
	ud = unit->bl2ud(src);
	nullpo_ret(ud);

	for(i=0;i<MAX_SKILLTIMERSKILL;i++) {
		if(ud->skilltimerskill[i]) {
			switch(ud->skilltimerskill[i]->skill_id){
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
				// SR_FLASHCOMBO
				case SR_DRAGONCOMBO:
				case SR_FALLENEMPIRE:
				case SR_TIGERCANNON:
				case SR_SKYNETBLOW:
					continue;
				default:
					if(skill->cleartimerskill_exception(ud->skilltimerskill[i]->skill_id))
						continue;
			}
			timer->delete(ud->skilltimerskill[i]->timer, skill->timerskill);
			ers_free(skill->timer_ers, ud->skilltimerskill[i]);
			ud->skilltimerskill[i]=NULL;
		}
	}
	return 1;
}

bool skill_cleartimerskill_exception(int skill_id)
{
	return false;
}

int skill_activate_reverberation(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;
	struct skill_unit_group *sg = NULL;

	nullpo_ret(bl);
	if (bl->type != BL_SKILL)
		return 0;
	su = BL_UCAST(BL_SKILL, bl);

	if( su->alive && (sg = su->group) != NULL && sg->skill_id == WM_REVERBERATION && sg->unit_id == UNT_REVERBERATION ) {
		int64 tick = timer->gettick();
		clif->changetraplook(bl,UNT_USED_TRAPS);
		map->foreachinrange(skill->trap_splash, bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, bl, tick);
		su->limit = DIFF_TICK32(tick,sg->tick)+1500;
		sg->unit_id = UNT_USED_TRAPS;
	}
	return 0;
}

int skill_reveal_trap(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_SKILL);
	su = BL_UCAST(BL_SKILL, bl);

	if (su->alive && su->group && skill->get_inf2(su->group->skill_id)&INF2_TRAP) { //Reveal trap.
		//Change look is not good enough, the client ignores it as an actual trap still. [Skotlex]
		//clif->changetraplook(bl, su->group->unit_id);
		clif->getareachar_skillunit(&su->bl,su,AREA);
		return 1;
	}
	return 0;
}

/*==========================================
 *
 *
 *------------------------------------------*/
int skill_castend_damage_id(struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag) {
	struct map_session_data *sd = NULL;
	struct status_data *tstatus;
	struct status_change *sc;

	if (skill_id > 0 && !skill_lv) return 0;

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m)
		return 1;

	if (bl->prev == NULL)
		return 1;

	sd = BL_CAST(BL_PC, src);

	if (status->isdead(bl))
		return 1;

	if (skill_id && skill->get_type(skill_id) == BF_MAGIC && status->isimmune(bl) == 100) {
		//GTB makes all targeted magic display miss with a single bolt.
		sc_type sct = status->skill2sc(skill_id);
		if(sct != SC_NONE)
			status_change_end(bl, sct, INVALID_TIMER);
		clif->skill_damage(src, bl, tick, status_get_amotion(src), status_get_dmotion(bl), 0, 1, skill_id, skill_lv, skill->get_hit(skill_id));
		return 1;
	}

	sc = status->get_sc(src);
	if (sc && !sc->count)
		sc = NULL; //Unneeded

	tstatus = status->get_status_data(bl);

	map->freeblock_lock();

	switch(skill_id) {
		case MER_CRASH:
		case SM_BASH:
		case MS_BASH:
		case MC_MAMMONITE:
		case TF_DOUBLE:
		case AC_DOUBLE:
		case MA_DOUBLE:
		case AS_SONICBLOW:
		case KN_PIERCE:
		case ML_PIERCE:
		case KN_SPEARBOOMERANG:
		case TF_POISON:
		case TF_SPRINKLESAND:
		case AC_CHARGEARROW:
		case MA_CHARGEARROW:
		case RG_INTIMIDATE:
		case AM_ACIDTERROR:
		case BA_MUSICALSTRIKE:
		case DC_THROWARROW:
		case BA_DISSONANCE:
		case CR_HOLYCROSS:
		case NPC_DARKCROSS:
		case CR_SHIELDCHARGE:
		case CR_SHIELDBOOMERANG:
		case NPC_PIERCINGATT:
		case NPC_MENTALBREAKER:
		case NPC_RANGEATTACK:
		case NPC_CRITICALSLASH:
		case NPC_COMBOATTACK:
		case NPC_GUIDEDATTACK:
		case NPC_POISON:
		case NPC_RANDOMATTACK:
		case NPC_WATERATTACK:
		case NPC_GROUNDATTACK:
		case NPC_FIREATTACK:
		case NPC_WINDATTACK:
		case NPC_POISONATTACK:
		case NPC_HOLYATTACK:
		case NPC_DARKNESSATTACK:
		case NPC_TELEKINESISATTACK:
		case NPC_UNDEADATTACK:
		case NPC_ARMORBRAKE:
		case NPC_WEAPONBRAKER:
		case NPC_HELMBRAKE:
		case NPC_SHIELDBRAKE:
		case NPC_BLINDATTACK:
		case NPC_SILENCEATTACK:
		case NPC_STUNATTACK:
		case NPC_PETRIFYATTACK:
		case NPC_CURSEATTACK:
		case NPC_SLEEPATTACK:
		case LK_AURABLADE:
		case LK_SPIRALPIERCE:
		case ML_SPIRALPIERCE:
		case LK_HEADCRUSH:
		case CG_ARROWVULCAN:
		case HW_MAGICCRASHER:
		case ITM_TOMAHAWK:
		case MO_TRIPLEATTACK:
		case CH_CHAINCRUSH:
		case CH_TIGERFIST:
		case PA_SHIELDCHAIN:
		case PA_SACRIFICE:
		case WS_CARTTERMINATION:
		case AS_VENOMKNIFE:
		case HT_PHANTASMIC:
		case TK_DOWNKICK:
		case TK_COUNTER:
		case GS_CHAINACTION:
		case GS_TRIPLEACTION:
		case GS_MAGICALBULLET:
		case GS_TRACKING:
		case GS_PIERCINGSHOT:
		case GS_RAPIDSHOWER:
		case GS_DUST:
		case GS_DISARM:
		case GS_FULLBUSTER:
		case NJ_SYURIKEN:
		case NJ_KUNAI:
#ifndef RENEWAL
		case ASC_BREAKER:
#endif
		case HFLI_MOON: //[orn]
		case HFLI_SBR44: //[orn]
		case NPC_BLEEDING:
		case NPC_CRITICALWOUND:
		case NPC_HELLPOWER:
		case RK_SONICWAVE:
		case RK_STORMBLAST:
		case AB_DUPLELIGHT_MELEE:
		case RA_AIMEDBOLT:
		case NC_AXEBOOMERANG:
		case NC_POWERSWING:
		case GC_CROSSIMPACT:
		case GC_VENOMPRESSURE:
		case SC_TRIANGLESHOT:
		case SC_FEINTBOMB:
		case LG_BANISHINGPOINT:
		case LG_SHIELDPRESS:
		case LG_RAGEBURST:
		case LG_RAYOFGENESIS:
		case LG_HESPERUSLIT:
		case SR_FALLENEMPIRE:
		case SR_CRESCENTELBOW_AUTOSPELL:
		case SR_GATEOFHELL:
		case SR_GENTLETOUCH_QUIET:
		case WM_SEVERE_RAINSTORM_MELEE:
		case WM_GREAT_ECHO:
		case GN_SLINGITEM_RANGEMELEEATK:
		case KO_SETSUDAN:
		case GC_DARKCROW:
		case LG_OVERBRAND_BRANDISH:
		case LG_OVERBRAND:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

		/**
		 * Mechanic (MADO GEAR)
		 **/
		case NC_BOOSTKNUCKLE:
		case NC_PILEBUNKER:
		case NC_COLDSLOWER:
			if (sd) pc->overheat(sd,1);
			/* Fall through */
		case RK_WINDCUTTER:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag|SD_ANIMATION);
			break;

		case LK_JOINTBEAT: // decide the ailment first (affects attack damage and effect)
			switch( rnd()%6 ){
			case 0: flag |= BREAK_ANKLE; break;
			case 1: flag |= BREAK_WRIST; break;
			case 2: flag |= BREAK_KNEE; break;
			case 3: flag |= BREAK_SHOULDER; break;
			case 4: flag |= BREAK_WAIST; break;
			case 5: flag |= BREAK_NECK; break;
			}
			//TODO: is there really no cleaner way to do this?
			sc = status->get_sc(bl);
			if (sc) sc->jb_flag = flag;
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case MO_COMBOFINISH:
			if (!(flag&1) && sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_MONK) {
				//Becomes a splash attack when Soul Linked.
				map->foreachinrange(skill->area_sub, bl,
				                    skill->get_splash(skill_id, skill_lv),splash_target(src),
				                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
				                    skill->castend_damage_id);
			} else
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case TK_STORMKICK: // Taekwon kicks [Dralnu]
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			skill->area_temp[1] = 0;
			map->foreachinrange(skill->attack_area, src,
			                    skill->get_splash(skill_id, skill_lv), splash_target(src),
			                    BF_WEAPON, src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
			break;

		case KN_CHARGEATK: {
				bool path_exists = path->search_long(NULL, src, src->m, src->x, src->y, bl->x, bl->y,CELL_CHKWALL);
				unsigned int dist = distance_bl(src, bl);
				uint8 dir = map->calc_dir(bl, src->x, src->y);

				// teleport to target (if not on WoE grounds)
				if( !map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground && unit->movepos(src, bl->x, bl->y, 0, 1) )
					clif->slide(src, bl->x, bl->y);

				// cause damage and knockback if the path to target was a straight one
				if( path_exists ) {
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, dist);
					skill->blown(src, bl, dist, dir, 0);
					//HACK: since knockback officially defaults to the left, the client also turns to the left... therefore,
					// make the caster look in the direction of the target
					unit->setdir(src, (dir+4)%8);
				}

			}
			break;

		case NC_FLAMELAUNCHER:
			if (sd) pc->overheat(sd,1);
			/* Fall through */
		case SN_SHARPSHOOTING:
		case MA_SHARPSHOOTING:
		case NJ_KAMAITACHI:
		case LG_CANNONSPEAR:
			//It won't shoot through walls since on castend there has to be a direct
			//line of sight between caster and target.
			skill->area_temp[1] = bl->id;
			map->foreachinpath(skill->attack_area,src->m,src->x,src->y,bl->x,bl->y,
			                   skill->get_splash(skill_id, skill_lv),skill->get_maxcount(skill_id,skill_lv), splash_target(src),
			                   skill->get_type(skill_id),src,src,skill_id,skill_lv,tick,flag,BCT_ENEMY);
			break;

		case NPC_ACIDBREATH:
		case NPC_DARKNESSBREATH:
		case NPC_FIREBREATH:
		case NPC_ICEBREATH:
		case NPC_THUNDERBREATH:
			skill->area_temp[1] = bl->id;
			map->foreachinpath(skill->attack_area,src->m,src->x,src->y,bl->x,bl->y,
			                   skill->get_splash(skill_id, skill_lv),skill->get_maxcount(skill_id,skill_lv), splash_target(src),
			                   skill->get_type(skill_id),src,src,skill_id,skill_lv,tick,flag,BCT_ENEMY);
			break;

		case MO_INVESTIGATE:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
			break;

		case RG_BACKSTAP:
			{
				uint8 dir = map->calc_dir(src, bl->x, bl->y), t_dir = unit->getdir(bl);
				if ((!check_distance_bl(src, bl, 0) && !map->check_dir(dir, t_dir)) || bl->type == BL_SKILL) {
					status_change_end(src, SC_HIDING, INVALID_TIMER);
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
					dir = dir < 4 ? dir+4 : dir-4; // change direction [Celest]
					unit->setdir(bl,dir);
				}
				else if (sd)
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case MO_FINGEROFFENSIVE:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			if (battle_config.finger_offensive_type && sd) {
				int i;
				for (i = 1; i < sd->spiritball_old; i++)
					skill->addtimerskill(src, tick + i * 200, bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
			}
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
			break;

		case MO_CHAINCOMBO:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
			break;

		case NJ_ISSEN:
		case MO_EXTREMITYFIST:
			{
				short x, y, i = 2; // Move 2 cells for Issen(from target)
				struct block_list *mbl = bl;
				short dir = 0;

				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);

				if( skill_id == MO_EXTREMITYFIST ) {
					mbl = src;
					i = 3; // for Asura(from caster)
					status->set_sp(src, 0, 0);
					status_change_end(src, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
					status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
#ifdef RENEWAL
					sc_start(src, src,SC_EXTREMITYFIST2,100,skill_lv,skill->get_time(skill_id,skill_lv));
#endif // RENEWAL
				} else {
					status_change_end(src, SC_NJ_NEN, INVALID_TIMER);
					status_change_end(src, SC_HIDING, INVALID_TIMER);
#ifdef RENEWAL
					status->set_hp(src, max(status_get_max_hp(src)/100, 1), 0);
#else // not RENEWAL
					status->set_hp(src, 1, 0);
#endif // RENEWAL
				}
				dir = map->calc_dir(src,bl->x,bl->y);
				if( dir > 0 && dir < 4) x = -i;
				else if( dir > 4 ) x = i;
				else x = 0;
				if( dir > 2 && dir < 6 ) y = -i;
				else if( dir == 7 || dir < 2 ) y = i;
				else y = 0;
				if ((mbl == src || (!map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground))) { // only NJ_ISSEN don't have slide effect in GVG
					if (!(unit->movepos(src, mbl->x+x, mbl->y+y, 1, 1))) {
						// The cell is not reachable (wall, object, ...), move next to the target
						if (x > 0) x = -1;
						else if (x < 0) x = 1;
						if (y > 0) y = -1;
						else if (y < 0) y = 1;

						unit->movepos(src, bl->x+x, bl->y+y, 1, 1);
					}
					clif->slide(src, src->x, src->y);
					clif->fixpos(src);
					clif->spiritball(src);
				}
			}
			break;

		case HT_POWER:
			if( tstatus->race == RC_BRUTE || tstatus->race == RC_INSECT )
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		//Splash attack skills.
		case AS_GRIMTOOTH:
		case MC_CARTREVOLUTION:
		case NPC_SPLASHATTACK:
			flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit
		case AS_SPLASHER:
		case HT_BLITZBEAT:
		case AC_SHOWER:
		case MA_SHOWER:
		case MG_NAPALMBEAT:
		case MG_FIREBALL:
		case RG_RAID:
		case HW_NAPALMVULCAN:
		case NJ_HUUMA:
		case NJ_BAKUENRYU:
		case ASC_METEORASSAULT:
		case GS_DESPERADO:
		case GS_SPREADATTACK:
		case NPC_PULSESTRIKE:
		case NPC_HELLJUDGEMENT:
		case NPC_VAMPIRE_GIFT:
		case RK_IGNITIONBREAK:
		case AB_JUDEX:
		case WL_SOULEXPANSION:
		case WL_CRIMSONROCK:
		case WL_COMET:
		case WL_JACKFROST:
		case RA_ARROWSTORM:
		case RA_WUGDASH:
		case NC_VULCANARM:
		case NC_ARMSCANNON:
		case NC_SELFDESTRUCTION:
		case NC_AXETORNADO:
		case GC_ROLLINGCUTTER:
		case GC_COUNTERSLASH:
		case LG_MOONSLASHER:
		case LG_EARTHDRIVE:
		case SR_TIGERCANNON:
		case SR_RAMPAGEBLASTER:
		case SR_SKYNETBLOW:
		case SR_WINDMILL:
		case SR_RIDEINLIGHTNING:
		case WM_REVERBERATION:
		case SO_VARETYR_SPEAR:
		case GN_CART_TORNADO:
		case GN_CARTCANNON:
		case KO_HAPPOKUNAI:
		case KO_HUUMARANKA:
		case KO_MUCHANAGE:
		case KO_BAKURETSU:
		case GN_ILLUSIONDOPING:
		case MH_XENO_SLASHER:
			if( flag&1 ) {//Recursive invocation
				// skill->area_temp[0] holds number of targets in area
				// skill->area_temp[1] holds the id of the original target
				// skill->area_temp[2] counts how many targets have already been processed
				int sflag = skill->area_temp[0] & 0xFFF, heal;
				struct status_change *tsc = status->get_sc(bl);
				if( flag&SD_LEVEL )
					sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
				if( (skill->area_temp[1] != bl->id && !(skill->get_inf2(skill_id)&INF2_NPC_SKILL)) || flag&SD_ANIMATION )
					sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

				if ( tsc && tsc->data[SC_HOVERING] && ( skill_id == SR_WINDMILL || skill_id == LG_MOONSLASHER ) )
					break;

				heal = skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, sflag);
				if( skill_id == NPC_VAMPIRE_GIFT && heal > 0 ) {
					clif->skill_nodamage(NULL, src, AL_HEAL, heal, 1);
					status->heal(src,heal,0,0);
				}
			} else {
				switch ( skill_id ) {
					case NJ_BAKUENRYU:
					case LG_EARTHDRIVE:
					case GN_CARTCANNON:
						clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
						break;
					case SR_TIGERCANNON:
					case GC_COUNTERSLASH:
					case GC_ROLLINGCUTTER:
						flag |= SD_ANIMATION;
						/* Fall through */
					case LG_MOONSLASHER:
					case MH_XENO_SLASHER:
						clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
						break;
					default:
						break;
				}

				skill->area_temp[0] = 0;
				skill->area_temp[1] = bl->id;
				skill->area_temp[2] = 0;
				if( skill_id == WL_CRIMSONROCK ) {
					skill->area_temp[4] = bl->x;
					skill->area_temp[5] = bl->y;
				}

				if( skill_id == NC_VULCANARM )
					if (sd) pc->overheat(sd,1);

				// if skill damage should be split among targets, count them
				//SD_LEVEL -> Forced splash damage for Auto Blitz-Beat -> count targets
				//special case: Venom Splasher uses a different range for searching than for splashing
				if( flag&SD_LEVEL || skill->get_nk(skill_id)&NK_SPLASHSPLIT )
					skill->area_temp[0] = map->foreachinrange(skill->area_sub, bl, (skill_id == AS_SPLASHER)?1:skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill->area_sub_count);

				// recursive invocation of skill->castend_damage_id() with flag|1
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);
			}
			break;

		case SM_MAGNUM:
		case MS_MAGNUM:
			if( flag&1 ) {
				//Damage depends on distance, so add it to flag if it is > 1
				skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION|distance_bl(src, bl));
			}
			break;

		case KN_BRANDISHSPEAR:
		case ML_BRANDISH:
			//Coded apart for it needs the flag passed to the damage calculation.
			if (skill->area_temp[1] != bl->id)
				skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
			else
				skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
			break;

		case KN_BOWLINGBASH:
		case MS_BOWLINGBASH:
			{
				int min_x,max_x,min_y,max_y,i,c,dir,tx,ty;
				// Chain effect and check range gets reduction by recursive depth, as this can reach 0, we don't use blowcount
				c = (skill_lv-(flag&0xFFF)+1)/2;
				// Determine the Bowling Bash area depending on configuration
				if (battle_config.bowling_bash_area == 0) {
					// Gutter line system
					min_x = ((src->x)-c) - ((src->x)-c)%40;
					if(min_x < 0) min_x = 0;
					max_x = min_x + 39;
					min_y = ((src->y)-c) - ((src->y)-c)%40;
					if(min_y < 0) min_y = 0;
					max_y = min_y + 39;
				} else if (battle_config.bowling_bash_area == 1) {
					// Gutter line system without demi gutter bug
					min_x = src->x - (src->x)%40;
					max_x = min_x + 39;
					min_y = src->y - (src->y)%40;
					max_y = min_y + 39;
				} else {
					// Area around caster
					min_x = src->x - battle_config.bowling_bash_area;
					max_x = src->x + battle_config.bowling_bash_area;
					min_y = src->y - battle_config.bowling_bash_area;
					max_y = src->y + battle_config.bowling_bash_area;
				}
				// Initialization, break checks, direction
				if((flag&0xFFF) > 0) {
					// Ignore monsters outside area
					if(bl->x < min_x || bl->x > max_x || bl->y < min_y || bl->y > max_y)
						break;
					// Ignore monsters already in list
					if(idb_exists(skill->bowling_db, bl->id))
						break;
					// Random direction
					dir = rnd()%8;
				} else {
					// Create an empty list of already hit targets
					db_clear(skill->bowling_db);
					// Direction is walkpath
					dir = (unit->getdir(src)+4)%8;
				}
				// Add current target to the list of already hit targets
				idb_put(skill->bowling_db, bl->id, bl);
				// Keep moving target in direction square by square
				tx = bl->x;
				ty = bl->y;
				for(i=0;i<c;i++) {
					// Target coordinates (get changed even if knockback fails)
					tx -= dirx[dir];
					ty -= diry[dir];
					// If target cell is a wall then break
					if(map->getcell(bl->m, bl, tx, ty, CELL_CHKWALL))
						break;
					skill->blown(src,bl,1,dir,0);
					// Splash around target cell, but only cells inside area; we first have to check the area is not negative
					if((max(min_x,tx-1) <= min(max_x,tx+1)) &&
						(max(min_y,ty-1) <= min(max_y,ty+1)) &&
						(map->foreachinarea(skill->area_sub, bl->m, max(min_x,tx-1), max(min_y,ty-1), min(max_x,tx+1), min(max_y,ty+1), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY, skill->area_sub_count))) {
						// Recursive call
						map->foreachinarea(skill->area_sub, bl->m, max(min_x,tx-1), max(min_y,ty-1), min(max_x,tx+1), min(max_y,ty+1), splash_target(src), src, skill_id, skill_lv, tick, (flag|BCT_ENEMY)+1, skill->castend_damage_id);
						// Self-collision
						if(bl->x >= min_x && bl->x <= max_x && bl->y >= min_y && bl->y <= max_y)
							skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,(flag&0xFFF)>0?SD_ANIMATION:0);
						break;
					}
				}
				// Original hit or chain hit depending on flag
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,(flag&0xFFF)>0?SD_ANIMATION:0);
			}
			break;

		case KN_SPEARSTAB:
			if(flag&1) {
				if (bl->id==skill->area_temp[1])
					break;
				if (skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,SD_ANIMATION))
					skill->blown(src,bl,skill->area_temp[2],-1,0);
			} else {
				int x=bl->x,y=bl->y,i,dir;
				dir = map->calc_dir(bl,src->x,src->y);
				skill->area_temp[1] = bl->id;
				skill->area_temp[2] = skill->get_blewcount(skill_id,skill_lv);
				// all the enemies between the caster and the target are hit, as well as the target
				if (skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,0))
					skill->blown(src,bl,skill->area_temp[2],-1,0);
				for (i=0;i<4;i++) {
					map->foreachincell(skill->area_sub,bl->m,x,y,BL_CHAR,src,skill_id,skill_lv,
					                   tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
					x += dirx[dir];
					y += diry[dir];
				}
			}
			break;

		case TK_TURNKICK:
		case MO_BALKYOUNG: //Active part of the attack. Skill-attack [Skotlex]
		{
			skill->area_temp[1] = bl->id; //NOTE: This is used in skill->castend_nodamage_id to avoid affecting the target.
			if (skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag))
				map->foreachinrange(skill->area_sub,bl,
				                    skill->get_splash(skill_id, skill_lv),BL_CHAR,
				                    src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,
				                    skill->castend_nodamage_id);
		}
			break;
		case CH_PALMSTRIKE: // Palm Strike takes effect 1sec after casting. [Skotlex]
			//clif->skill_nodamage(src,bl,skill_id,skill_lv,0); //Can't make this one display the correct attack animation delay :/
			clif->damage(src,bl,status_get_amotion(src),0,-1,1,BDT_ENDURE,0); //Display an absorbed damage attack.
			skill->addtimerskill(src, tick + (1000+status_get_amotion(src)), bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
			break;

		case PR_TURNUNDEAD:
		case ALL_RESURRECTION:
			if (!battle->check_undead(tstatus->race, tstatus->def_ele))
				break;
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case MG_SOULSTRIKE:
		case NPC_DARKSTRIKE:
		case MG_COLDBOLT:
		case MG_FIREBOLT:
		case MG_LIGHTNINGBOLT:
		case WZ_EARTHSPIKE:
		case AL_HEAL:
		case AL_HOLYLIGHT:
		case WZ_JUPITEL:
		case NPC_DARKTHUNDER:
		case PR_ASPERSIO:
		case MG_FROSTDIVER:
		case WZ_SIGHTBLASTER:
		case WZ_SIGHTRASHER:
		case NJ_KOUENKA:
		case NJ_HYOUSENSOU:
		case NJ_HUUJIN:
		case AB_ADORAMUS:
		case AB_RENOVATIO:
		case AB_HIGHNESSHEAL:
		case AB_DUPLELIGHT_MAGIC:
		case WM_METALICSOUND:
		case MH_ERASER_CUTTER:
		case KO_KAIHOU:
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case NPC_MAGICALATTACK:
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			sc_start(src, src,status->skill2sc(skill_id),100,skill_lv,skill->get_time(skill_id,skill_lv));
			break;

		case HVAN_CAPRICE: //[blackhole89]
			{
				int ran=rnd()%4;
				int sid = 0;
				switch(ran)
				{
				case 0: sid=MG_COLDBOLT; break;
				case 1: sid=MG_FIREBOLT; break;
				case 2: sid=MG_LIGHTNINGBOLT; break;
				case 3: sid=WZ_EARTHSPIKE; break;
				}
				skill->attack(BF_MAGIC,src,src,bl,sid,skill_lv,tick,flag|SD_LEVEL);
			}
			break;
		case WZ_WATERBALL:
			{
				int range = skill_lv / 2;
				int maxlv = skill->get_max(skill_id); // learnable level
				int count = 0;
				int x, y;
				struct skill_unit *su;

				if( skill_lv > maxlv ) {
					if( src->type == BL_MOB && skill_lv == 10 )
						range = 4;
					else
						range = maxlv / 2;
				}

				for( y = src->y - range; y <= src->y + range; ++y )
					for( x = src->x - range; x <= src->x + range; ++x ) {
						if( !map->find_skill_unit_oncell(src,x,y,SA_LANDPROTECTOR,NULL,1) ) {
							if (src->type != BL_PC || map->getcell(src->m, src, x, y, CELL_CHKWATER)) // non-players bypass the water requirement
								count++; // natural water cell
							else if( (su = map->find_skill_unit_oncell(src,x,y,SA_DELUGE,NULL,1)) != NULL
							      || (su = map->find_skill_unit_oncell(src,x,y,NJ_SUITON,NULL,1)) != NULL ) {
								count++; // skill-induced water cell
								skill->delunit(su); // consume cell
							}
						}
					}

				if( count > 1 ) // queue the remaining count - 1 timerskill Waterballs
					skill->addtimerskill(src,tick+150,bl->id,0,0,skill_id,skill_lv,count-1,flag);
			}
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case PR_BENEDICTIO:
			//Should attack undead and demons. [Skotlex]
			if (battle->check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON)
				skill->attack(BF_MAGIC, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

		case SL_SMA:
			status_change_end(src, SC_SMA_READY, INVALID_TIMER);
			/* Fall through */
		case SL_STIN:
		case SL_STUN:
			if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
				status->change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case NPC_DARKBREATH:
			clif->emotion(src,E_AG);
			/* Fall through */
		case SN_FALCONASSAULT:
		case PA_PRESSURE:
		case CR_ACIDDEMONSTRATION:
		case TF_THROWSTONE:
		case NPC_SMOKING:
		case GS_FLING:
		case NJ_ZENYNAGE:
		case GN_THORNS_TRAP:
		case GN_HELLS_PLANT_ATK:
#ifdef RENEWAL
		case ASC_BREAKER:
#endif
			skill->attack(BF_MISC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;
		/**
		 * Rune Knight
		 **/
		case RK_DRAGONBREATH_WATER:
		case RK_DRAGONBREATH:
		{
			struct status_change *tsc = NULL;
			if( (tsc = status->get_sc(bl)) && (tsc->data[SC_HIDING] )) {
				clif->skill_nodamage(src,src,skill_id,skill_lv,1);
			} else
				skill->attack(BF_MISC,src,src,bl,skill_id,skill_lv,tick,flag);
		}
			break;
		case NPC_SELFDESTRUCTION: {
			struct status_change *tsc = NULL;
			if( (tsc = status->get_sc(bl)) && tsc->data[SC_HIDING] )
				break;
			}
		case HVAN_EXPLOSION:
			if (src != bl)
				skill->attack(BF_MISC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		// [Celest]
		case PF_SOULBURN:
			if (rnd()%100 < (skill_lv < 5 ? 30 + skill_lv * 10 : 70)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if (skill_lv == 5)
					skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
				status_percent_damage(src, bl, 0, 100, false);
			} else {
				clif->skill_nodamage(src,src,skill_id,skill_lv,1);
				if (skill_lv == 5)
					skill->attack(BF_MAGIC,src,src,src,skill_id,skill_lv,tick,flag);
				status_percent_damage(src, src, 0, 100, false);
			}
			break;

		case NPC_BLOODDRAIN:
		case NPC_ENERGYDRAIN:
		{
			int heal = skill->attack( (skill_id == NPC_BLOODDRAIN) ? BF_WEAPON : BF_MAGIC,
			                         src, src, bl, skill_id, skill_lv, tick, flag);
			if (heal > 0){
				clif->skill_nodamage(NULL, src, AL_HEAL, heal, 1);
				status->heal(src, heal, 0, 0);
			}
		}
			break;

		case GS_BULLSEYE:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case NJ_KASUMIKIRI:
			if (skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag) > 0)
				sc_start(src,src,SC_HIDING,100,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case NJ_KIRIKAGE:
			if( !map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground ) {
				//You don't move on GVG grounds.
				short x, y;
				map->search_freecell(bl, 0, &x, &y, 1, 1, 0);
				if (unit->movepos(src, x, y, 0, 0))
					clif->slide(src,src->x,src->y);
			}
			status_change_end(src, SC_HIDING, INVALID_TIMER);
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;
		case RK_HUNDREDSPEAR:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			if(rnd()%100 < (10 + 3*skill_lv)) {
				if( !sd || pc->checkskill(sd,KN_SPEARBOOMERANG) == 0 )
					break; // Spear Boomerang auto cast chance only works if you have mastered Spear Boomerang.
				skill->blown(src,bl,6,-1,0);
				skill->addtimerskill(src,tick+800,bl->id,0,0,skill_id,skill_lv,BF_WEAPON,flag);
				skill->castend_damage_id(src,bl,KN_SPEARBOOMERANG,1,tick,0);
			}
			break;
		case RK_PHANTOMTHRUST:
		{
			struct map_session_data *tsd = BL_CAST(BL_PC, bl);
			unit->setdir(src,map->calc_dir(src, bl->x, bl->y));
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);

			skill->blown(src,bl,distance_bl(src,bl)-1,unit->getdir(src),0);
			if( sd && tsd && sd->status.party_id && tsd->status.party_id && sd->status.party_id == tsd->status.party_id ) // Don't damage party members.
				; // No damage to Members
			else
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		}
			break;

		case KO_JYUMONJIKIRI:
		case GC_DARKILLUSION:
			{
				short x, y;
				short dir = map->calc_dir(bl, src->x, src->y);

				if ( dir < 4 ) {
					x = bl->x + 2 * (dir > 0) - 3 * (dir > 0);
					y = bl->y + 1 - (dir / 2) - (dir > 2);
				} else {
					x = bl->x + 2 * (dir > 4) - 1 * (dir > 4);
					y = bl->y + (dir / 6) - 1 + (dir > 6);
				}

				if ( unit->movepos(src, x, y, 1, 1) ) {
					clif->slide(src, x, y);
					clif->fixpos(src); // the official server send these two packets.
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
					if (rnd() % 100 < 4 * skill_lv && skill_id == GC_DARKILLUSION)
						skill->castend_damage_id(src, bl, GC_CROSSIMPACT, skill_lv, tick, flag);
				}
			}
			break;
		case GC_WEAPONCRUSH:
			if( sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == GC_WEAPONBLOCKING )
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			else if( sd )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_GC_WEAPONBLOCKING,0);
			break;

		case GC_CROSSRIPPERSLASHER:
			if( sd && !(sc && sc->data[SC_ROLLINGCUTTER]) )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_CONDITION,0);
			else
			{
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
				status_change_end(src,SC_ROLLINGCUTTER,INVALID_TIMER);
			}
			break;

		case GC_PHANTOMMENACE:
			if( flag&1 ) {
				// Only Hits Invisible Targets
				struct status_change *tsc = status->get_sc(bl);
				if(tsc && (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->data[SC__INVISIBILITY]) )
					skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			}
			break;
		case WL_CHAINLIGHTNING:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			skill->addtimerskill(src,tick+status_get_amotion(src),bl->id,0,0,WL_CHAINLIGHTNING_ATK,skill_lv,0,flag);
			break;
		case WL_DRAINLIFE:
		{
			int heal = skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
			int rate = 70 + 5 * skill_lv;

			heal = heal * (5 + 5 * skill_lv) / 100;

			if( bl->type == BL_SKILL || status_get_hp(src) == status_get_max_hp(src)) // Don't absorb when caster was in full HP
				heal = 0; // Don't absorb heal from Ice Walls or other skill units.

			if( heal && rnd()%100 < rate ) {
				status->heal(src, heal, 0, 0);
				clif->skill_nodamage(NULL, src, AL_HEAL, heal, 1);
			}
		}
			break;

		case WL_TETRAVORTEX:
			if (sc) {
				int i = SC_SUMMON5, x = 0;
				int types[][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
				for(; i >= SC_SUMMON1; i--) {
					if (sc->data[i]) {
						int skillid = WL_TETRAVORTEX_FIRE + (sc->data[i]->val1 - WLS_FIRE) + (sc->data[i]->val1 == WLS_WIND) - (sc->data[i]->val1 == WLS_WATER);
						if (x < 4) {
							int sc_index = 0, rate = 0;
							types[x][0] = (sc->data[i]->val1 - WLS_FIRE) + 1;
							types[x][1] = 25; // 25% each for equal sharing
							if (x == 3) {
								x = 0;
								sc_index = types[rnd()%4][0];
								for(; x < 4; x++)
									if(types[x][0] == sc_index)
										rate += types[x][1];
							}
							skill->addtimerskill(src, tick + (SC_SUMMON5-i) * 206, bl->id, sc_index, rate, skillid, skill_lv, x, flag);
						}
						status_change_end(src, (sc_type)i, INVALID_TIMER);
						x++;
					}
				}
			}
			break;

		case WL_RELEASE:
			if (sd) {
				int i;
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				skill->toggle_magicpower(src, skill_id);
				// Priority is to release SpellBook
				if (sc && sc->data[SC_READING_SB]) {
					// SpellBook
					uint16 spell_skill_id, spell_skill_lv, point, s = 0;
					int spell[SC_SPELLBOOK7-SC_SPELLBOOK1 + 1];
					int cooldown;

					for(i = SC_SPELLBOOK7; i >= SC_SPELLBOOK1; i--) // List all available spell to be released
					if( sc->data[i] ) spell[s++] = i;

					if ( s == 0 )
						break;

					i = spell[s==1?0:rnd()%s];// Random select of spell to be released.
					if( s && sc->data[i] ){// Now extract the data from the preserved spell
						spell_skill_id = sc->data[i]->val1;
						spell_skill_lv = sc->data[i]->val2;
						point = sc->data[i]->val3;
						status_change_end(src, (sc_type)i, INVALID_TIMER);
					} else {
						//something went wrong :(
						break;
					}

					if( sc->data[SC_READING_SB]->val2 > point )
						sc->data[SC_READING_SB]->val2 -= point;
					else // Last spell to be released
						status_change_end(src, SC_READING_SB, INVALID_TIMER);

					if( !skill->check_condition_castbegin(sd, spell_skill_id, spell_skill_lv) )
						break;

					switch( skill->get_casttype(spell_skill_id) ) {
						case CAST_GROUND:
							skill->castend_pos2(src, bl->x, bl->y, spell_skill_id, spell_skill_lv, tick, 0);
							break;
						case CAST_NODAMAGE:
							skill->castend_nodamage_id(src, bl, spell_skill_id, spell_skill_lv, tick, 0);
							break;
						case CAST_DAMAGE:
							skill->castend_damage_id(src, bl, spell_skill_id, spell_skill_lv, tick, 0);
							break;
					}

					sd->ud.canact_tick = tick + skill->delay_fix(src, spell_skill_id, spell_skill_lv);
					clif->status_change(src, SI_POSTDELAY, 1, skill->delay_fix(src, spell_skill_id, spell_skill_lv), 0, 0, 0);

					cooldown = skill->get_cooldown(spell_skill_id, spell_skill_lv);
					for (i = 0; i < ARRAYLENGTH(sd->skillcooldown) && sd->skillcooldown[i].id; i++) {
						if (sd->skillcooldown[i].id == spell_skill_id){
							cooldown += sd->skillcooldown[i].val;
							break;
						}
					}
					if(cooldown)
						skill->blockpc_start(sd, spell_skill_id, cooldown);
				}else if( sc ){ // Summon Balls
					for(i = SC_SUMMON5; i >= SC_SUMMON1; i--){
						if( sc->data[i] ){
							int skillid = WL_SUMMON_ATK_FIRE + (sc->data[i]->val1 - WLS_FIRE);
							skill->addtimerskill(src, tick + status_get_adelay(src) * (SC_SUMMON5 - i), bl->id, 0, 0, skillid, skill_lv, BF_MAGIC, flag);
							status_change_end(src, (sc_type)i, INVALID_TIMER);
							if(skill_lv == 1)
								break;
						}
					}
				}
			}
			break;
		case WL_FROSTMISTY:
			// Doesn't deal damage through non-shootable walls.
			if( path->search(NULL,src,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKWALL) )
				skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag|SD_ANIMATION);
			break;
		case WL_HELLINFERNO:
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag|ELE_DARK);
			break;
		case RA_WUGSTRIKE:
			if( sd && pc_isridingwug(sd) ){
				short x[8]={0,-1,-1,-1,0,1,1,1};
				short y[8]={1,1,0,-1,-1,-1,0,1};
				uint8 dir = map->calc_dir(bl, src->x, src->y);

				if( unit->movepos(src, bl->x+x[dir], bl->y+y[dir], 1, 1) )
				{
					clif->slide(src, bl->x+x[dir], bl->y+y[dir]);
					clif->fixpos(src);
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
				}
				break;
			}
		case RA_WUGBITE:
			if( path->search(NULL,src,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKNOREACH) ) {
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			}else if( sd && skill_id == RA_WUGBITE ) // Only RA_WUGBITE has the skill fail message.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);

			break;

		case RA_SENSITIVEKEEN:
			if( bl->type != BL_SKILL ) { // Only Hits Invisible Targets
				struct status_change * tsc = status->get_sc(bl);
				if( tsc && tsc->option&(OPTION_HIDE|OPTION_CLOAK) ){
					skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
					status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
				}
			} else {
				struct skill_unit *su = BL_CAST(BL_SKILL,bl);
				struct skill_unit_group* sg;

				if( su && (sg=su->group) != NULL && skill->get_inf2(sg->skill_id)&INF2_TRAP ) {
					if( !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) ) {
						struct item item_tmp;
						memset(&item_tmp,0,sizeof(item_tmp));
						item_tmp.nameid = sg->item_id?sg->item_id:ITEMID_TRAP;
						item_tmp.identify = 1;
						if( item_tmp.nameid )
							map->addflooritem(bl, &item_tmp, 1, bl->m, bl->x, bl->y, 0, 0, 0, 0);
					}
					skill->delunit(su);
				}
			}
			break;
		case NC_INFRAREDSCAN:
			if( flag&1 ) {
				//TODO: Need a confirmation if the other type of hidden status is included to be scanned. [Jobbie]
				sc_start(src, bl, SC_INFRAREDSCAN, 10000, skill_lv, skill->get_time(skill_id, skill_lv));
				status_change_end(bl, SC_HIDING, INVALID_TIMER);
				status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
				status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER); // Need confirm it.
			} else {
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);
				clif->skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( sd ) pc->overheat(sd,1);
			}
			break;

		case NC_MAGNETICFIELD:
			sc_start2(src,bl,SC_MAGNETICFIELD,100,skill_lv,src->id,skill->get_time(skill_id,skill_lv));
			break;
		case SC_FATALMENACE:
			if( flag&1 )
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			else {
				short x, y;
				map->search_freecell(src, 0, &x, &y, -1, -1, 0);
				// Destination area
				skill->area_temp[4] = x;
				skill->area_temp[5] = y;
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_damage_id);
				skill->addtimerskill(src,tick + 800,src->id,x,y,skill_id,skill_lv,0,flag); // To teleport Self
				clif->skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,BDT_SKILL);
			}
			break;
		case LG_PINPOINTATTACK:
			if( !map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground && unit->movepos(src, bl->x, bl->y, 1, 1) )
				clif->slide(src,bl->x,bl->y);
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case LG_SHIELDSPELL:
			if ( skill_lv == 1 )
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			else if ( skill_lv == 2 )
				skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case SR_DRAGONCOMBO:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case SR_KNUCKLEARROW:
				if( !map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground && unit->movepos(src, bl->x, bl->y, 1, 1) ) {
					clif->slide(src,bl->x,bl->y);
					clif->fixpos(src); // Aegis send this packet too.
				}

				if( flag&1 )
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag|SD_LEVEL);
				else
					skill->addtimerskill(src, tick + 300, bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag|SD_LEVEL|2);
			break;

		case SR_HOWLINGOFLION:
				status_change_end(bl, SC_SWING, INVALID_TIMER);
				status_change_end(bl, SC_SYMPHONY_LOVE, INVALID_TIMER);
				status_change_end(bl, SC_MOONLIT_SERENADE, INVALID_TIMER);
				status_change_end(bl, SC_RUSH_WINDMILL, INVALID_TIMER);
				status_change_end(bl, SC_ECHOSONG, INVALID_TIMER);
				status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
				status_change_end(bl, SC_NETHERWORLD, INVALID_TIMER);
				status_change_end(bl, SC_SIREN, INVALID_TIMER);
				status_change_end(bl, SC_GLOOMYDAY, INVALID_TIMER);
				status_change_end(bl, SC_SONG_OF_MANA, INVALID_TIMER);
				status_change_end(bl, SC_DANCE_WITH_WUG, INVALID_TIMER);
				status_change_end(bl, SC_SATURDAY_NIGHT_FEVER, INVALID_TIMER);
				status_change_end(bl, SC_MELODYOFSINK, INVALID_TIMER);
				status_change_end(bl, SC_BEYOND_OF_WARCRY, INVALID_TIMER);
				status_change_end(bl, SC_UNLIMITED_HUMMING_VOICE, INVALID_TIMER);
				skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
			break;

		case SR_EARTHSHAKER:
			if( flag&1 ) { //by default cloaking skills are remove by aoe skills so no more checking/removing except hiding and cloaking exceed.
				skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
				status_change_end(bl, SC_HIDING, INVALID_TIMER);
				status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			} else{
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);
				clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			}
			break;

		case WM_SOUND_OF_DESTRUCTION:
			{
				struct status_change *tsc = status->get_sc(bl);
				if( tsc && tsc->count && ( tsc->data[SC_SWING] || tsc->data[SC_SYMPHONY_LOVE] || tsc->data[SC_MOONLIT_SERENADE] ||
						tsc->data[SC_RUSH_WINDMILL] || tsc->data[SC_ECHOSONG] || tsc->data[SC_HARMONIZE] ||
						tsc->data[SC_SIREN] || tsc->data[SC_DEEP_SLEEP] || tsc->data[SC_SIRCLEOFNATURE] ||
						tsc->data[SC_GLOOMYDAY] || tsc->data[SC_SONG_OF_MANA] ||
						tsc->data[SC_DANCE_WITH_WUG] || tsc->data[SC_SATURDAY_NIGHT_FEVER] || tsc->data[SC_LERADS_DEW] ||
						tsc->data[SC_MELODYOFSINK] || tsc->data[SC_BEYOND_OF_WARCRY] || tsc->data[SC_UNLIMITED_HUMMING_VOICE] ) &&
						rnd()%100 < 4 * skill_lv + 2 * (sd ? pc->checkskill(sd,WM_LESSON) : 10) + 10 * battle->calc_chorusbonus(sd)) {
					skill->attack(BF_MISC,src,src,bl,skill_id,skill_lv,tick,flag);
					status->change_start(src,bl,SC_STUN,10000,skill_lv,0,0,0,skill->get_time(skill_id,skill_lv),SCFLAG_FIXEDRATE);
					status_change_end(bl, SC_SWING, INVALID_TIMER);
					status_change_end(bl, SC_SYMPHONY_LOVE, INVALID_TIMER);
					status_change_end(bl, SC_MOONLIT_SERENADE, INVALID_TIMER);
					status_change_end(bl, SC_RUSH_WINDMILL, INVALID_TIMER);
					status_change_end(bl, SC_ECHOSONG, INVALID_TIMER);
					status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
					status_change_end(bl, SC_SIREN, INVALID_TIMER);
					status_change_end(bl, SC_DEEP_SLEEP, INVALID_TIMER);
					status_change_end(bl, SC_SIRCLEOFNATURE, INVALID_TIMER);
					status_change_end(bl, SC_GLOOMYDAY, INVALID_TIMER);
					status_change_end(bl, SC_SONG_OF_MANA, INVALID_TIMER);
					status_change_end(bl, SC_DANCE_WITH_WUG, INVALID_TIMER);
					status_change_end(bl, SC_SATURDAY_NIGHT_FEVER, INVALID_TIMER);
					status_change_end(bl, SC_LERADS_DEW, INVALID_TIMER);
					status_change_end(bl, SC_MELODYOFSINK, INVALID_TIMER);
					status_change_end(bl, SC_BEYOND_OF_WARCRY, INVALID_TIMER);
					status_change_end(bl, SC_UNLIMITED_HUMMING_VOICE, INVALID_TIMER);
				}
			}
			break;

		case SO_POISON_BUSTER:
		{
			struct status_change *tsc = status->get_sc(bl);
			if( tsc && tsc->data[SC_POISON] ) {
				skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
				status_change_end(bl, SC_POISON, INVALID_TIMER);
			} else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
		}
			break;

		case GN_SPORE_EXPLOSION:
			if( flag&1 )
				skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			else {
				clif->skill_nodamage(src, bl, skill_id, 0, 1);
				skill->addtimerskill(src, timer->gettick() + skill->get_time(skill_id, skill_lv), bl->id, 0, 0, skill_id, skill_lv, 0, 0);
			}
			break;

		case EL_FIRE_BOMB:
		case EL_FIRE_WAVE:
		case EL_WATER_SCREW:
		case EL_HURRICANE:
		case EL_TYPOON_MIS:
			if( flag&1 )
				skill->attack(skill->get_type(skill_id+1),src,src,bl,skill_id+1,skill_lv,tick,flag);
			else {
				int i = skill->get_splash(skill_id,skill_lv);
				clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
				clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( rnd()%100 < 30 )
					map->foreachinrange(skill->area_sub,bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
				else
					skill->attack(skill->get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
			}
			break;

		case EL_ROCK_CRUSHER:
			clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
			clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			if( rnd()%100 < 50 )
				skill->attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			else
				skill->attack(BF_WEAPON,src,src,bl,EL_ROCK_CRUSHER_ATK,skill_lv,tick,flag);
			break;

		case EL_STONE_RAIN:
			if( flag&1 )
				skill->attack(skill->get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
			else {
				int i = skill->get_splash(skill_id,skill_lv);
				clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
				clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( rnd()%100 < 30 )
					map->foreachinrange(skill->area_sub,bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
				else
					skill->attack(skill->get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
			}
			break;

		case EL_FIRE_ARROW:
		case EL_ICE_NEEDLE:
		case EL_WIND_SLASH:
		case EL_STONE_HAMMER:
			clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
			clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			skill->attack(skill->get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case EL_TIDAL_WEAPON:
			if( src->type == BL_ELEM ) {
				struct elemental_data *ele = BL_CAST(BL_ELEM,src);
				struct status_change *esc = status->get_sc(&ele->bl);
				struct status_change *tsc = status->get_sc(bl);
				sc_type type = status->skill2sc(skill_id), type2;
				type2 = type-1;

				clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
				clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( (esc && esc->data[type2]) || (tsc && tsc->data[type]) ) {
					elemental->clean_single_effect(ele, skill_id);
				}
				if( rnd()%100 < 50 )
					skill->attack(skill->get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
				else {
					sc_start(src, src,type2,100,skill_lv,skill->get_time(skill_id,skill_lv));
					sc_start(src, battle->get_master(src),type,100,ele->bl.id,skill->get_time(skill_id,skill_lv));
				}
				clif->skill_nodamage(src,src,skill_id,skill_lv,1);
			}
			break;

		// Recursive homun skill
		case MH_MAGMA_FLOW:
		case MH_HEILIGE_STANGE:
			if(flag & 1)
				skill->attack(skill->get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
			else {
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill->castend_damage_id);
			}
			break;

		case MH_STAHL_HORN:
		case MH_NEEDLE_OF_PARALYZE:
			skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			break;
		case MH_TINDER_BREAKER:
			if (unit->movepos(src, bl->x, bl->y, 1, 1)) {
	#if PACKETVER >= 20111005
				clif->snap(src, bl->x, bl->y);
	#else
				clif->skill_poseffect(src,skill_id,skill_lv,bl->x,bl->y,tick);
	#endif
			}
					clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,SC_RG_CCONFINE_S,100,skill_lv,src->id,0,0,skill->get_time(skill_id,skill_lv)));
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			break;

		case 0:/* no skill - basic/normal attack */
			if(sd) {
				if (flag & 3){
					if (bl->id != skill->area_temp[1])
						skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, SD_LEVEL|flag);
				} else {
					skill->area_temp[1] = bl->id;
					map->foreachinrange(skill->area_sub, bl,
					                    sd->bonus.splash_range, BL_CHAR,
					                    src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1,
					                    skill->castend_damage_id);
					flag|=1; //Set flag to 1 so ammo is not double-consumed. [Skotlex]
				}
			}
			break;

		default:
			if (skill->castend_damage_id_unknown(src, bl, &skill_id, &skill_lv, &tick, &flag, tstatus, sc))
				return 1;
			break;
	}

	if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);

	map->freeblock_unlock();

	if( sd && !(flag&1) )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = timer->gettick();

		if( sd->state.arrow_atk )
		{// consume arrow on last invocation to this skill.
			battle->consume_ammo(sd, skill_id, skill_lv);
		}

		// perform skill requirement consumption
		skill->consume_requirement(sd,skill_id,skill_lv,2);
	}

	return 0;
}

bool skill_castend_damage_id_unknown(struct block_list* src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, struct status_data *tstatus, struct status_change *sc)
{
	ShowWarning("skill_castend_damage_id: Unknown skill used:%d\n", *skill_id);
	clif->skill_damage(src, bl, *tick, status_get_amotion(src), tstatus->dmotion,
		0, abs(skill->get_num(*skill_id, *skill_lv)),
		*skill_id, *skill_lv, skill->get_hit(*skill_id));
	map->freeblock_unlock();
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_id(int tid, int64 tick, int id, intptr_t data) {
	struct block_list *target, *src;
	struct map_session_data *sd;
	struct mob_data *md;
	struct unit_data *ud;
	struct status_change *sc = NULL;
	int inf,inf2,flag = 0;

	src = map->id2bl(id);
	if( src == NULL )
	{
		ShowDebug("skill_castend_id: src == NULL (tid=%d, id=%d)\n", tid, id);
		return 0;// not found
	}

	ud = unit->bl2ud(src);
	if( ud == NULL )
	{
		ShowDebug("skill_castend_id: ud == NULL (tid=%d, id=%d)\n", tid, id);
		return 0;// ???
	}

	sd = BL_CAST(BL_PC,  src);
	md = BL_CAST(BL_MOB, src);

	if( src->prev == NULL ) {
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if(ud->skill_id != SA_CASTCANCEL && ud->skill_id != SO_SPELLFIST) {// otherwise handled in unit->skillcastcancel()
		if( ud->skilltimer != tid ) {
			ShowError("skill_castend_id: Timer mismatch %d!=%d!\n", ud->skilltimer, tid);
			ud->skilltimer = INVALID_TIMER;
			return 0;
		}

		if( sd && ud->skilltimer != INVALID_TIMER && (pc->checkskill(sd,SA_FREECAST) > 0 || ud->skill_id == LG_EXEEDBREAK) )
		{// restore original walk speed
			ud->skilltimer = INVALID_TIMER;
			status_calc_bl(&sd->bl, SCB_SPEED|SCB_ASPD);
		}

		ud->skilltimer = INVALID_TIMER;
	}

	if (ud->skilltarget == id)
		target = src;
	else
		target = map->id2bl(ud->skilltarget);

	// Use a do so that you can break out of it when the skill fails.
	do {
		if(!target || target->prev==NULL) break;

		if(src->m != target->m || status->isdead(src)) break;

		switch (ud->skill_id) {
			//These should become skill_castend_pos
			case WE_CALLPARTNER:
				if(sd) clif->callpartner(sd);
				/* Fall through */
			case WE_CALLPARENT:
			case WE_CALLBABY:
			case AM_RESURRECTHOMUN:
			case PF_SPIDERWEB:
				//Find a random spot to place the skill. [Skotlex]
				inf2 = skill->get_splash(ud->skill_id, ud->skill_lv);
				ud->skillx = target->x + inf2;
				ud->skilly = target->y + inf2;
				if (inf2 && !map->random_dir(target, &ud->skillx, &ud->skilly)) {
					ud->skillx = target->x;
					ud->skilly = target->y;
				}
				ud->skilltimer=tid;
				return skill->castend_pos(tid,tick,id,data);
			case GN_WALLOFTHORN:
				ud->skillx = target->x;
				ud->skilly = target->y;
				ud->skilltimer = tid;
				return skill->castend_pos(tid,tick,id,data);
			default:
				if (skill->castend_id_unknown(ud, src, target))
					return 0;
				break;
		}

		if(ud->skill_id == RG_BACKSTAP) {
			uint8 dir = map->calc_dir(src,target->x,target->y),t_dir = unit->getdir(target);
			if(check_distance_bl(src, target, 0) || map->check_dir(dir,t_dir)) {
				break;
			}
		}

		if( ud->skill_id == PR_TURNUNDEAD ) {
			struct status_data *tstatus = status->get_status_data(target);
			if( !battle->check_undead(tstatus->race, tstatus->def_ele) )
				break;
		}

		if( ud->skill_id == RA_WUGSTRIKE ){
			if( !path->search(NULL,src,src->m,src->x,src->y,target->x,target->y,1,CELL_CHKNOREACH))
				break;
		}

		if( ud->skill_id == PR_LEXDIVINA || ud->skill_id == MER_LEXDIVINA ) {
			sc = status->get_sc(target);
			if( battle->check_target(src,target, BCT_ENEMY) <= 0 && (!sc || !sc->data[SC_SILENCE]) )
			{ //If it's not an enemy, and not silenced, you can't use the skill on them. [Skotlex]
				clif->skill_nodamage (src, target, ud->skill_id, ud->skill_lv, 0);
				break;
			}
		}
		else
		{ // Check target validity.
			inf = skill->get_inf(ud->skill_id);
			inf2 = skill->get_inf2(ud->skill_id);

			if(inf&INF_ATTACK_SKILL ||
				(inf&INF_SELF_SKILL && inf2&INF2_NO_TARGET_SELF) //Combo skills
					) // Casted through combo.
				inf = BCT_ENEMY; //Offensive skill.
			else if(inf2&INF2_NO_ENEMY)
				inf = BCT_NOENEMY;
			else
				inf = 0;

			if (inf2 & (INF2_PARTY_ONLY|INF2_GUILD_ONLY) && src != target) {
				if (inf2&INF2_PARTY_ONLY)
					inf |= BCT_PARTY;
				if (inf2&INF2_GUILD_ONLY)
					inf |= BCT_GUILD;
				//Remove neutral targets (but allow enemy if skill is designed to be so)
				inf &= ~BCT_NEUTRAL;
			}

			if( sd && (inf2&INF2_CHORUS_SKILL) && skill->check_pc_partner(sd, ud->skill_id, &ud->skill_lv, 1, 0) < 1 ) {
				clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_NEED_HELPER, 0);
				break;
			}

			if (ud->skill_id >= SL_SKE && ud->skill_id <= SL_SKA && target->type == BL_MOB) {
				if (BL_UCCAST(BL_MOB, target)->class_ == MOBID_EMPELIUM)
					break;
			} else if (inf && battle->check_target(src, target, inf) <= 0) {
				if (sd) clif->skill_fail(sd,ud->skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			} else if (ud->skill_id == RK_PHANTOMTHRUST && target->type != BL_MOB) {
				if( !map_flag_vs(src->m) && battle->check_target(src,target,BCT_PARTY) <= 0 )
					break; // You can use Phantom Thurst on party members in normal maps too. [pakpil]
			}

			if( inf&BCT_ENEMY
			 && (sc = status->get_sc(target)) != NULL && sc->data[SC_FOGWALL]
			 && rnd() % 100 < 75
			) {
				// Fogwall makes all offensive-type targeted skills fail at 75%
				if (sd) clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0);
				break;
			}
		}

		//Avoid doing double checks for instant-cast skills.
		if (tid != INVALID_TIMER && !status->check_skilluse(src, target, ud->skill_id, 1))
			break;

		if(md) {
			md->last_thinktime=tick +MIN_MOBTHINKTIME;
			if(md->skill_idx >= 0 && md->db->skill[md->skill_idx].emotion >= 0)
				clif->emotion(src, md->db->skill[md->skill_idx].emotion);
		}

		if(src != target && battle_config.skill_add_range &&
			!check_distance_bl(src, target, skill->get_range2(src,ud->skill_id,ud->skill_lv)+battle_config.skill_add_range))
		{
			if (sd) {
				clif->skill_fail(sd,ud->skill_id,USESKILL_FAIL_LEVEL,0);
				if(battle_config.skill_out_range_consume) //Consume items anyway. [Skotlex]
					skill->consume_requirement(sd,ud->skill_id,ud->skill_lv,3);
			}
			break;
		}

		if( sd )
		{
			if( !skill->check_condition_castend(sd, ud->skill_id, ud->skill_lv) )
				break;
			else
				skill->consume_requirement(sd,ud->skill_id,ud->skill_lv,1);
		}
#ifdef OFFICIAL_WALKPATH
		if( !path->search_long(NULL, src, src->m, src->x, src->y, target->x, target->y, CELL_CHKWALL) )
			break;
#endif
		if( (src->type == BL_MER || src->type == BL_HOM) && !skill->check_condition_mercenary(src, ud->skill_id, ud->skill_lv, 1) )
			break;

		if (ud->state.running && ud->skill_id == TK_JUMPKICK) {
			ud->state.running = 0;
			status_change_end(src, SC_RUN, INVALID_TIMER);
			flag = 1;
		}

		if (ud->walktimer != INVALID_TIMER && ud->skill_id != TK_RUN && ud->skill_id != RA_WUGDASH)
			unit->stop_walking(src, STOPWALKING_FLAG_FIXPOS);

		if( !sd || sd->skillitem != ud->skill_id || skill->get_delay(ud->skill_id,ud->skill_lv) )
			ud->canact_tick = tick + skill->delay_fix(src, ud->skill_id, ud->skill_lv); // Tests show wings don't overwrite the delay but skill scrolls do. [Inkfish]
		if (sd) { // Cooldown application
			int i, cooldown = skill->get_cooldown(ud->skill_id, ud->skill_lv);
			for (i = 0; i < ARRAYLENGTH(sd->skillcooldown) && sd->skillcooldown[i].id; i++) { // Increases/Decreases cooldown of a skill by item/card bonuses.
				if (sd->skillcooldown[i].id == ud->skill_id){
					cooldown += sd->skillcooldown[i].val;
					break;
				}
			}
			if(cooldown)
				skill->blockpc_start(sd, ud->skill_id, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif->status_change(src, SI_POSTDELAY, 1, skill->delay_fix(src, ud->skill_id, ud->skill_lv), 0, 0, 0);
		if( sd )
		{
			switch( ud->skill_id )
			{
			case GS_DESPERADO:
				sd->canequip_tick = tick + skill->get_time(ud->skill_id, ud->skill_lv);
				break;
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS:
				if( (sc = status->get_sc(src)) && sc->data[SC_NOEQUIPSHIELD] ) {
					const struct TimerData *td = timer->get(sc->data[SC_NOEQUIPSHIELD]->timer);
					if( td && td->func == status->change_timer && DIFF_TICK(td->tick,timer->gettick()+skill->get_time(ud->skill_id, ud->skill_lv)) > 0 )
						break;
				}
				sc_start2(src,src, SC_NOEQUIPSHIELD, 100, 0, 1, skill->get_time(ud->skill_id, ud->skill_lv));
				break;
			}
		}
		if (skill->get_state(ud->skill_id) != ST_MOVE_ENABLE)
			unit->set_walkdelay(src, tick, battle_config.default_walk_delay+skill->get_walkdelay(ud->skill_id, ud->skill_lv), 1);

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %u, ID %d skill castend id [id =%d, lv=%d, target ID %d]\n",
				src->type, src->id, ud->skill_id, ud->skill_lv, target->id);

		map->freeblock_lock();

		// SC_MAGICPOWER needs to switch states before any damage is actually dealt
		skill->toggle_magicpower(src, ud->skill_id);

#if 0 // On aegis damage skills are also increase by camouflage. Need confirmation on kRO.
		if( ud->skill_id != RA_CAMOUFLAGE ) // only normal attack and auto cast skills benefit from its bonuses
			status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);
#endif // 0

		if (skill->get_casttype(ud->skill_id) == CAST_NODAMAGE)
			skill->castend_nodamage_id(src,target,ud->skill_id,ud->skill_lv,tick,flag);
		else
			skill->castend_damage_id(src,target,ud->skill_id,ud->skill_lv,tick,flag);

		sc = status->get_sc(src);
		if(sc && sc->count) {
			if( sc->data[SC_SOULLINK]
			 && sc->data[SC_SOULLINK]->val2 == SL_WIZARD
			 && sc->data[SC_SOULLINK]->val3 == ud->skill_id
			 && ud->skill_id != WZ_WATERBALL
			)
				sc->data[SC_SOULLINK]->val3 = 0; //Clear bounced spell check.

			if( sc->data[SC_DANCING] && skill->get_inf2(ud->skill_id)&INF2_SONG_DANCE && sd )
				skill->blockpc_start(sd,BD_ADAPTATION,3000);
		}

		if( sd && ud->skill_id != SA_ABRACADABRA && ud->skill_id != WM_RANDOMIZESPELL ) // they just set the data so leave it as it is.[Inkfish]
			sd->skillitem = sd->skillitemlv = 0;

		if (ud->skilltimer == INVALID_TIMER) {
			if(md) md->skill_idx = -1;
			else ud->skill_id = 0; //mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skill_lv = ud->skilltarget = 0;
		}

		if (src->id != target->id)
			unit->setdir(src, map->calc_dir(src, target->x, target->y));

		map->freeblock_unlock();
		return 1;
	} while(0);

	//Skill failed.
	if (ud->skill_id == MO_EXTREMITYFIST && sd && !(sc && sc->data[SC_FOGWALL])) {
		//When Asura fails... (except when it fails from Fog of Wall)
		//Consume SP/spheres
		skill->consume_requirement(sd,ud->skill_id, ud->skill_lv,1);
		status->set_sp(src, 0, 0);
		sc = &sd->sc;
		if (sc->count) {
			//End states
			status_change_end(src, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
#ifdef RENEWAL
			sc_start(src, src, SC_EXTREMITYFIST2, 100, ud->skill_lv, skill->get_time(ud->skill_id, ud->skill_lv));
#endif
		}
		if (target && target->m == src->m) {
			//Move character to target anyway.
			int dir, x, y;
			dir = map->calc_dir(src,target->x,target->y);
			if( dir > 0 && dir < 4) x = -2;
			else if( dir > 4 ) x = 2;
			else x = 0;
			if( dir > 2 && dir < 6 ) y = -2;
			else if( dir == 7 || dir < 2 ) y = 2;
			else y = 0;
			if (unit->movepos(src, src->x+x, src->y+y, 1, 1)) {
				//Display movement + animation.
				clif->slide(src,src->x,src->y);
				clif->spiritball(src);
			}
			clif->skill_fail(sd,ud->skill_id,USESKILL_FAIL_LEVEL,0);
		}
	}

	if( !sd || sd->skillitem != ud->skill_id || skill->get_delay(ud->skill_id,ud->skill_lv) )
		ud->canact_tick = tick;
	ud->skill_id = ud->skill_lv = ud->skilltarget = 0;
	//You can't place a skill failed packet here because it would be
	//sent in ALL cases, even cases where skill_check_condition fails
	//which would lead to double 'skill failed' messages u.u [Skotlex]
	if(sd)
		sd->skillitem = sd->skillitemlv = 0;
	else if(md)
		md->skill_idx = -1;
	return 0;
}

bool skill_castend_id_unknown(struct unit_data *ud, struct block_list *src, struct block_list *target)
{
    return false;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_nodamage_id(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag) {
	struct map_session_data *sd, *dstsd;
	struct mob_data *md, *dstmd;
	struct homun_data *hd;
	struct mercenary_data *mer;
	struct status_data *sstatus, *tstatus;
	struct status_change *tsc;
	struct status_change_entry *tsce;

	int element = 0;
	enum sc_type type;

	if(skill_id > 0 && !skill_lv) return 0; // [Celest]

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m)
		return 1;

	sd = BL_CAST(BL_PC, src);
	hd = BL_CAST(BL_HOM, src);
	md = BL_CAST(BL_MOB, src);
	mer = BL_CAST(BL_MER, src);

	dstsd = BL_CAST(BL_PC, bl);
	dstmd = BL_CAST(BL_MOB, bl);

	if(bl->prev == NULL)
		return 1;
	if(status->isdead(src))
		return 1;

	if( src != bl && status->isdead(bl) ) {
		/**
		 * Skills that may be cast on dead targets
		 **/
		switch( skill_id ) {
			case NPC_WIDESOULDRAIN:
			case PR_REDEMPTIO:
			case ALL_RESURRECTION:
			case WM_DEADHILLHERE:
				break;
			default:
				if (skill->castend_nodamage_id_dead_unknown(src, bl, &skill_id, &skill_lv, &tick, &flag))
					return 1;
				break;
		}
	}

	// Supportive skills that can't be cast in users with mado
	if( sd && dstsd && pc_ismadogear(dstsd) ) {
		switch( skill_id ) {
			case AL_HEAL:
			case AL_INCAGI:
			case AL_DECAGI:
			case AB_RENOVATIO:
			case AB_HIGHNESSHEAL:
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_TOTARGET,0);
				return 0;
			default:
				if (skill->castend_nodamage_id_mado_unknown(src, bl, &skill_id, &skill_lv, &tick, &flag))
					return 0;
				break;
		}
	}

	tstatus = status->get_status_data(bl);
	sstatus = status->get_status_data(src);

	//Check for undead skills that convert a no-damage skill into a damage one. [Skotlex]
	switch (skill_id) {
		case HLIF_HEAL: // [orn]
			if (bl->type != BL_HOM) {
				if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0) ;
				break ;
			}
		case AL_HEAL:

		/**
		 * Arch Bishop
		 **/
		case AB_RENOVATIO:
		case AB_HIGHNESSHEAL:
		case AL_INCAGI:
		case ALL_RESURRECTION:
		case PR_ASPERSIO:
			//Apparently only player casted skills can be offensive like this.
			if (sd && battle->check_undead(tstatus->race,tstatus->def_ele) && skill_id != AL_INCAGI) {
				if (battle->check_target(src, bl, BCT_ENEMY) < 1) {
					//Offensive heal does not works on non-enemies. [Skotlex]
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
				return skill->castend_damage_id(src, bl, skill_id, skill_lv, tick, flag);
			}
			break;
		case SO_ELEMENTAL_SHIELD:
			{
				struct party_data *p;
				short ret = 0;
				int x0, y0, x1, y1, range, i;

				if(sd == NULL || !sd->ed)
					break;
				if((p = party->search(sd->status.party_id)) == NULL)
					break;

				range = skill->get_splash(skill_id,skill_lv);
				x0 = sd->bl.x - range;
				y0 = sd->bl.y - range;
				x1 = sd->bl.x + range;
				y1 = sd->bl.y + range;

				elemental->delete(sd->ed,0);

				if(!skill->check_unit_range(src,src->x,src->y,skill_id,skill_lv))
					ret = skill->castend_pos2(src,src->x,src->y,skill_id,skill_lv,tick,flag);
				for(i = 0; i < MAX_PARTY; i++) {
					struct map_session_data *psd = p->data[i].sd;
					if(!psd)
						continue;
					if(psd->bl.m != sd->bl.m || !psd->bl.prev)
						continue;
					if(range && (psd->bl.x < x0 || psd->bl.y < y0 ||
						psd->bl.x > x1 || psd->bl.y > y1))
						continue;
					if(!skill->check_unit_range(bl,psd->bl.x,psd->bl.y,skill_id,skill_lv))
						ret |= skill->castend_pos2(bl,psd->bl.x,psd->bl.y,skill_id,skill_lv,tick,flag);
				}
				return ret;
			}
			break;
		case NPC_SMOKING: //Since it is a self skill, this one ends here rather than in damage_id. [Skotlex]
			return skill->castend_damage_id(src, bl, skill_id, skill_lv, tick, flag);
		case MH_STEINWAND: {
			struct block_list *s_src = battle->get_master(src);
			short ret = 0;
			if(!skill->check_unit_range(src, src->x, src->y, skill_id, skill_lv))  //prevent reiteration
			    ret = skill->castend_pos2(src,src->x,src->y,skill_id,skill_lv,tick,flag); //cast on homun
			if(s_src && !skill->check_unit_range(s_src, s_src->x, s_src->y, skill_id, skill_lv))
			    ret |= skill->castend_pos2(s_src,s_src->x,s_src->y,skill_id,skill_lv,tick,flag); //cast on master
			if (hd)
			    skill->blockhomun_start(hd, skill_id, skill->get_cooldown(skill_id, skill_lv));
			return ret;
		    }
		    break;
		case RK_MILLENNIUMSHIELD:
		case RK_CRUSHSTRIKE:
		case RK_REFRESH:
		case RK_GIANTGROWTH:
		case RK_STONEHARDSKIN:
		case RK_VITALITYACTIVATION:
		case RK_STORMBLAST:
		case RK_FIGHTINGSPIRIT:
		case RK_ABUNDANCE:
			if( sd && !pc->checkskill(sd, RK_RUNEMASTERY) ){
				if( status->change_start(src,&sd->bl, (sc_type)(rnd()%SC_CONFUSION), 1000, 1, 0, 0, 0, skill->get_time2(skill_id,skill_lv),SCFLAG_FIXEDRATE) ){
					skill->consume_requirement(sd,skill_id,skill_lv,2);
					map->freeblock_unlock();
					return 0;
				}
			}
			break;
		default:
			if (skill->castend_nodamage_id_undead_unknown(src, bl, &skill_id, &skill_lv, &tick, &flag))
			{
				//Skill is actually ground placed.
				if (src == bl && skill->get_unit_id(skill_id,0))
					return skill->castend_pos2(src,bl->x,bl->y,skill_id,skill_lv,tick,0);
			}
			break;
	}

	type = status->skill2sc(skill_id);
	tsc = status->get_sc(bl);
	tsce = (tsc != NULL && type != SC_NONE) ? tsc->data[type] : NULL;

	if (src != bl && type > SC_NONE
	 && (element = skill->get_ele(skill_id, skill_lv)) > ELE_NEUTRAL
	 && skill->get_inf(skill_id) != INF_SUPPORT_SKILL
	 && battle->attr_fix(NULL, NULL, 100, element, tstatus->def_ele, tstatus->ele_lv) <= 0)
		return 1; //Skills that cause an status should be blocked if the target element blocks its element.

	map->freeblock_lock();
	switch(skill_id) {
		case HLIF_HEAL: // [orn]
		case AL_HEAL:
		/**
		 * Arch Bishop
		 **/
		case AB_HIGHNESSHEAL:
			{
				int heal = skill->calc_heal(src, bl, (skill_id == AB_HIGHNESSHEAL)?AL_HEAL:skill_id, (skill_id == AB_HIGHNESSHEAL)?10:skill_lv, true);
				int heal_get_jobexp;
				//Highness Heal: starts at 1.7 boost + 0.3 for each level
				if (skill_id == AB_HIGHNESSHEAL) {
					heal = heal * (17 + 3 * skill_lv) / 10;
				}
				if (status->isimmune(bl) || (dstmd != NULL && (dstmd->class_ == MOBID_EMPELIUM || mob_is_battleground(dstmd))))
					heal = 0;

				if (sd && dstsd && sd->status.partner_id == dstsd->status.char_id && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.sex == 0)
					heal = heal * 2;

				if (tsc && tsc->count)
				{
					if (tsc->data[SC_KAITE] && !(sstatus->mode&MD_BOSS))
					{ //Bounce back heal
						if (--tsc->data[SC_KAITE]->val2 <= 0)
							status_change_end(bl, SC_KAITE, INVALID_TIMER);
						if (src == bl)
							heal=0; //When you try to heal yourself under Kaite, the heal is voided.
						else {
							bl = src;
							dstsd = sd;
						}
					}
					else if (tsc->data[SC_BERSERK])
						heal = 0; //Needed so that it actually displays 0 when healing.
				}
				clif->skill_nodamage (src, bl, skill_id, heal, 1);
				if( tsc && tsc->data[SC_AKAITSUKI] && heal && skill_id != HLIF_HEAL )
					heal = ~heal + 1;
				heal_get_jobexp = status->heal(bl,heal,0,0);

				if(sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0){
					heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
					if (heal_get_jobexp <= 0)
						heal_get_jobexp = 1;
					pc->gainexp (sd, bl, 0, heal_get_jobexp, false);
				}
			}
			break;

		case PR_REDEMPTIO:
			if (sd && !(flag&1)) {
				if (sd->status.party_id == 0) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				skill->area_temp[0] = 0;
				party->foreachsamemap(skill->area_sub,
					sd,skill->get_splash(skill_id, skill_lv),
					src,skill_id,skill_lv,tick, flag|BCT_PARTY|1,
					skill->castend_nodamage_id);
				if (skill->area_temp[0] == 0) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				skill->area_temp[0] = 5 - skill->area_temp[0]; // The actual penalty...
				if (skill->area_temp[0] > 0 && !map->list[src->m].flag.noexppenalty) { //Apply penalty
					sd->status.base_exp -= min(sd->status.base_exp, pc->nextbaseexp(sd) * skill->area_temp[0] * 2/1000); //0.2% penalty per each.
					sd->status.job_exp -= min(sd->status.job_exp, pc->nextjobexp(sd) * skill->area_temp[0] * 2/1000);
					clif->updatestatus(sd,SP_BASEEXP);
					clif->updatestatus(sd,SP_JOBEXP);
				}
				status->set_hp(src, 1, 0);
				status->set_sp(src, 0, 0);
				break;
			} else if (status->isdead(bl) && flag&1) { //Revive
				skill->area_temp[0]++; //Count it in, then fall-through to the Resurrection code.
				skill_lv = 3; //Resurrection level 3 is used
			} else //Invalid target, skip resurrection.
				break;

		case ALL_RESURRECTION:
			if(sd && (map_flag_gvg2(bl->m) || map->list[bl->m].flag.battleground)) {
				//No reviving in WoE grounds!
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if (!status->isdead(bl))
				break;
			{
				int per = 0, sper = 0;
				if (tsc && tsc->data[SC_HELLPOWER])
					break;

				if (map->list[bl->m].flag.pvp && dstsd && dstsd->pvp_point < 0)
					break;

				switch(skill_lv){
				case 1: per=10; break;
				case 2: per=30; break;
				case 3: per=50; break;
				case 4: per=80; break;
				}
				if(dstsd && dstsd->special_state.restart_full_recover)
					per = sper = 100;
				if (status->revive(bl, per, sper))
				{
					clif->skill_nodamage(src,bl,ALL_RESURRECTION,skill_lv,1); //Both Redemptio and Res show this skill-animation.
					if(sd && dstsd && battle_config.resurrection_exp > 0)
					{
						int exp = 0,jexp = 0;
						int lv = dstsd->status.base_level - sd->status.base_level, jlv = dstsd->status.job_level - sd->status.job_level;
						if(lv > 0 && pc->nextbaseexp(dstsd)) {
							exp = (int)((double)dstsd->status.base_exp * (double)lv * (double)battle_config.resurrection_exp / 1000000.);
							if (exp < 1) exp = 1;
						}
						if(jlv > 0 && pc->nextjobexp(dstsd)) {
							jexp = (int)((double)dstsd->status.job_exp * (double)jlv * (double)battle_config.resurrection_exp / 1000000.);
							if (jexp < 1) jexp = 1;
						}
						if(exp > 0 || jexp > 0)
							pc->gainexp (sd, bl, exp, jexp, false);
					}
				}
			}
			break;

		case AL_DECAGI:
			clif->skill_nodamage (src, bl, skill_id, skill_lv,
								  sc_start(src, bl, type, (40 + skill_lv * 2 + (status->get_lv(src) + sstatus->int_)/5), skill_lv,
										   /* monsters using lvl 48 get the rate benefit but the duration of lvl 10 */
										   ( src->type == BL_MOB && skill_lv == 48 ) ? skill->get_time(skill_id,10) : skill->get_time(skill_id,skill_lv)));
			break;

		case MER_DECAGI:
			if( tsc && !tsc->data[SC_ADORAMUS] ) //Prevent duplicate agi-down effect.
				clif->skill_nodamage(src, bl, skill_id, skill_lv,
					sc_start(src, bl, type, (40 + skill_lv * 2 + (status->get_lv(src) + sstatus->int_)/5), skill_lv, skill->get_time(skill_id,skill_lv)));
			break;

		case AL_CRUCIS:
			if (flag&1)
				sc_start(src, bl,type, 23+skill_lv*4 +status->get_lv(src) -status->get_lv(bl), skill_lv,skill->get_time(skill_id,skill_lv));
			else {
				map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR,
				                    src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;

		case PR_LEXDIVINA:
		case MER_LEXDIVINA:
			if( tsce )
				status_change_end(bl,type, INVALID_TIMER);
			else
				sc_start(src, bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
			clif->skill_nodamage (src, bl, skill_id, skill_lv, 1);
			break;

		case SA_ABRACADABRA:
			{
				int abra_skill_id = 0, abra_skill_lv, abra_idx;
				do {
					abra_idx = rnd() % MAX_SKILL_ABRA_DB;
					abra_skill_id = skill->dbs->abra_db[abra_idx].skill_id;
				} while (abra_skill_id == 0 ||
					skill->dbs->abra_db[abra_idx].req_lv > skill_lv || //Required lv for it to appear
					rnd()%10000 >= skill->dbs->abra_db[abra_idx].per
				);
				abra_skill_lv = min(skill_lv, skill->get_max(abra_skill_id));
				clif->skill_nodamage (src, bl, skill_id, skill_lv, 1);

				if (sd) {
					// player-casted
					sd->state.abra_flag = 1;
					sd->skillitem = abra_skill_id;
					sd->skillitemlv = abra_skill_lv;
					clif->item_skill(sd, abra_skill_id, abra_skill_lv);
				} else {
					// mob-casted
					struct unit_data *ud = unit->bl2ud(src);
					int inf = skill->get_inf(abra_skill_id);
					if (ud == NULL)
						break;
					if (inf&INF_SELF_SKILL || inf&INF_SUPPORT_SKILL) {
						int id = src->id;
						struct pet_data *pd = BL_CAST(BL_PET, src);
						if (pd != NULL && pd->msd != NULL)
							id = pd->msd->bl.id;
						unit->skilluse_id(src, id, abra_skill_id, abra_skill_lv);
					} else { //Assume offensive skills
						int target_id = 0;
						if (ud->target)
							target_id = ud->target;
						else switch (src->type) {
							case BL_MOB: target_id = BL_UCAST(BL_MOB, src)->target_id; break;
							case BL_PET: target_id = BL_UCAST(BL_PET, src)->target_id; break;
						}
						if (!target_id)
							break;
						if (skill->get_casttype(abra_skill_id) == CAST_GROUND) {
							bl = map->id2bl(target_id);
							if (!bl) bl = src;
							unit->skilluse_pos(src, bl->x, bl->y, abra_skill_id, abra_skill_lv);
						} else
							unit->skilluse_id(src, target_id, abra_skill_id, abra_skill_lv);
					}
				}
			}
			break;

		case SA_COMA:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src, bl,type,100,skill_lv,skill->get_time2(skill_id,skill_lv)));
			break;
		case SA_FULLRECOVERY:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (status->isimmune(bl))
				break;
			status_percent_heal(bl, 100, 100);
			break;
		case NPC_ALLHEAL:
		{
			int heal;
			if( status->isimmune(bl) )
				break;
			heal = status_percent_heal(bl, 100, 0);
			clif->skill_nodamage(NULL, bl, AL_HEAL, heal, 1);
			if( dstmd ) {
				// Reset Damage Logs
				memset(dstmd->dmglog, 0, sizeof(dstmd->dmglog));
				dstmd->tdmg = 0;
			}
		}
			break;
		case SA_SUMMONMONSTER:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (sd) mob->once_spawn(sd, src->m, src->x, src->y," --ja--", -1, 1, "", SZ_SMALL, AI_NONE);
			break;
		case SA_LEVELUP:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (sd && pc->nextbaseexp(sd)) pc->gainexp(sd, NULL, pc->nextbaseexp(sd) * 10 / 100, 0, false);
			break;
		case SA_INSTANTDEATH:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status->set_hp(bl,1,0);
			break;
		case SA_QUESTION:
		case SA_GRAVITY:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case SA_CLASSCHANGE:
		case SA_MONOCELL:
			if (dstmd)
			{
				int class_;
				if ( sd && dstmd->status.mode&MD_BOSS )
				{
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				class_ = skill_id == SA_MONOCELL ? MOBID_PORING : mob->get_random_id(4, 1, 0);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				mob->class_change(dstmd,class_);
				if( tsc && dstmd->status.mode&MD_BOSS )
				{
					int i;
					const enum sc_type scs[] = { SC_QUAGMIRE, SC_PROVOKE, SC_ROKISWEIL, SC_GRAVITATION, SC_NJ_SUITON, SC_NOEQUIPWEAPON, SC_NOEQUIPSHIELD, SC_NOEQUIPARMOR, SC_NOEQUIPHELM, SC_BLADESTOP };
					for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++)
						if (tsc->data[i]) status_change_end(bl, (sc_type)i, INVALID_TIMER);
					for (i = 0; i < ARRAYLENGTH(scs); i++)
						if (tsc->data[scs[i]]) status_change_end(bl, scs[i], INVALID_TIMER);
				}
			}
			break;
		case SA_DEATH:
			if ( sd && dstmd && dstmd->status.mode&MD_BOSS )
			{
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status_kill(bl);
			break;
		case SA_REVERSEORCISH:
		case ALL_REVERSEORCISH:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src, bl,type,100,skill_lv,skill->get_time(skill_id, skill_lv)));
			break;
		case SA_FORTUNE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if(sd) pc->getzeny(sd,status->get_lv(bl)*100,LOG_TYPE_STEAL,NULL);
			break;
		case SA_TAMINGMONSTER:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (sd && dstmd) {
				int i;
				ARR_FIND( 0, MAX_PET_DB, i, dstmd->class_ == pet->db[i].class_ );
				if( i < MAX_PET_DB )
					pet->catch_process1(sd, dstmd->class_);
			}
			break;

		case CR_PROVIDENCE:
			if(sd && dstsd){ //Check they are not another crusader [Skotlex]
				if ((dstsd->class_&MAPID_UPPERMASK) == MAPID_CRUSADER) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					map->freeblock_unlock();
					return 1;
				}
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src, bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;

		case CG_MARIONETTE:
			{
				struct status_change* sc = status->get_sc(src);

				if( sd && dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER && dstsd->status.sex == sd->status.sex ) {
					// Cannot cast on another bard/dancer-type class of the same gender as caster
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					map->freeblock_unlock();
					return 1;
				}

				if( sc && tsc ) {
					if( !sc->data[SC_MARIONETTE_MASTER] && !tsc->data[SC_MARIONETTE] ) {
						sc_start(src,src,SC_MARIONETTE_MASTER,100,bl->id,skill->get_time(skill_id,skill_lv));
						sc_start(src,bl,SC_MARIONETTE,100,src->id,skill->get_time(skill_id,skill_lv));
						clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					} else if( sc->data[SC_MARIONETTE_MASTER ] && sc->data[SC_MARIONETTE_MASTER ]->val1 == bl->id
					        && tsc->data[SC_MARIONETTE] && tsc->data[SC_MARIONETTE]->val1 == src->id
					) {
						status_change_end(src, SC_MARIONETTE_MASTER, INVALID_TIMER);
						status_change_end(bl, SC_MARIONETTE, INVALID_TIMER);
					} else {
						if( sd )
							clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						map->freeblock_unlock();
						return 1;
					}
				}
			}
			break;

		case RG_CLOSECONFINE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,src->id,0,0,skill->get_time(skill_id,skill_lv)));
			break;
		case SA_FLAMELAUNCHER: // added failure chance and chance to break weapon if turned on [Valaris]
		case SA_FROSTWEAPON:
		case SA_LIGHTNINGLOADER:
		case SA_SEISMICWEAPON:
			if (dstsd) {
				if(dstsd->status.weapon == W_FIST ||
					(dstsd->sc.count && !dstsd->sc.data[type] &&
					( //Allow re-enchanting to lengthen time. [Skotlex]
						dstsd->sc.data[SC_PROPERTYFIRE] ||
						dstsd->sc.data[SC_PROPERTYWATER] ||
						dstsd->sc.data[SC_PROPERTYWIND] ||
						dstsd->sc.data[SC_PROPERTYGROUND] ||
						dstsd->sc.data[SC_PROPERTYDARK] ||
						dstsd->sc.data[SC_PROPERTYTELEKINESIS] ||
						dstsd->sc.data[SC_ENCHANTPOISON]
					))
					) {
					if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
					break;
				}
			}
			// 100% success rate at lv4 & 5, but lasts longer at lv5
			if(!clif->skill_nodamage(src,bl,skill_id,skill_lv, sc_start(src,bl,type,(60+skill_lv*10),skill_lv, skill->get_time(skill_id,skill_lv)))) {
				if (sd)
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				if (skill->break_equip(bl, EQP_WEAPON, 10000, BCT_PARTY) && sd && sd != dstsd)
					clif->message(sd->fd, msg_sd(sd,869)); // "You broke the target's weapon."
			}
			break;

		case PR_ASPERSIO:
			if (sd && dstmd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
				break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;

		case ITEM_ENCHANTARMS:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,100,skill_lv,
					skill->get_ele(skill_id,skill_lv), skill->get_time(skill_id,skill_lv)));
			break;

		case TK_SEVENWIND:
			switch(skill->get_ele(skill_id,skill_lv)) {
				case ELE_EARTH : type = SC_PROPERTYGROUND;  break;
				case ELE_WIND  : type = SC_PROPERTYWIND;   break;
				case ELE_WATER : type = SC_PROPERTYWATER;  break;
				case ELE_FIRE  : type = SC_PROPERTYFIRE;   break;
				case ELE_GHOST : type = SC_PROPERTYTELEKINESIS;  break;
				case ELE_DARK  : type = SC_PROPERTYDARK; break;
				case ELE_HOLY  : type = SC_ASPERSIO;     break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));

			sc_start2(src,bl,SC_TK_SEVENWIND,100,skill_lv,skill->get_ele(skill_id,skill_lv),skill->get_time(skill_id,skill_lv));

			break;

		case PR_KYRIE:
		case MER_KYRIE:
			clif->skill_nodamage(bl, bl, skill_id, -1,
				sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;
		//Passive Magnum, should had been casted on yourself.
		case SM_MAGNUM:
		case MS_MAGNUM:
			skill->area_temp[1] = 0;
			map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_SKILL|BL_CHAR,
			                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1, skill->castend_damage_id);
			clif->skill_nodamage (src,src,skill_id,skill_lv,1);
			// Initiate 10% of your damage becomes fire element.
			sc_start4(src,src,SC_SUB_WEAPONPROPERTY,100,3,20,0,0,skill->get_time2(skill_id, skill_lv));
			if( sd )
				skill->blockpc_start(sd, skill_id, skill->get_time(skill_id, skill_lv));
			else if( bl->type == BL_MER )
				skill->blockmerc_start(BL_UCAST(BL_MER, bl), skill_id, skill->get_time(skill_id, skill_lv));
			break;

		case TK_JUMPKICK:
			/* Check if the target is an enemy; if not, skill should fail so the character doesn't unit->movepos (exploitable) */
			if( battle->check_target(src, bl, BCT_ENEMY) > 0 )
			{
				if( unit->movepos(src, bl->x, bl->y, 1, 1) )
				{
					skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
					clif->slide(src,bl->x,bl->y);
				}
			}
			else
				clif->skill_fail(sd,skill_id,USESKILL_FAIL,0);
			break;

		case AL_INCAGI:
		case AL_BLESSING:
		case MER_INCAGI:
		case MER_BLESSING:
			if (dstsd != NULL && tsc->data[SC_PROPERTYUNDEAD]) {
				skill->attack(BF_MISC,src,src,bl,skill_id,skill_lv,tick,flag);
				break;
			}
		case PR_SLOWPOISON:
		case PR_IMPOSITIO:
		case PR_LEXAETERNA:
		case PR_SUFFRAGIUM:
		case PR_BENEDICTIO:
		case LK_BERSERK:
		case MS_BERSERK:
		case KN_TWOHANDQUICKEN:
		case KN_ONEHAND:
		case MER_QUICKEN:
		case CR_SPEARQUICKEN:
		case CR_REFLECTSHIELD:
		case MS_REFLECTSHIELD:
		case AS_POISONREACT:
		case MC_LOUD:
		case MG_ENERGYCOAT:
		case MO_EXPLOSIONSPIRITS:
		case MO_STEELBODY:
		case MO_BLADESTOP:
		case LK_AURABLADE:
		case LK_PARRYING:
		case MS_PARRYING:
		case LK_CONCENTRATION:
		case WS_CARTBOOST:
		case SN_SIGHT:
		case WS_MELTDOWN:
		case WS_OVERTHRUSTMAX:
		case ST_REJECTSWORD:
		case HW_MAGICPOWER:
		case PF_MEMORIZE:
		case PA_SACRIFICE:
		case ASC_EDP:
		case PF_DOUBLECASTING:
		case SG_SUN_COMFORT:
		case SG_MOON_COMFORT:
		case SG_STAR_COMFORT:
		case NPC_HALLUCINATION:
		case GS_MADNESSCANCEL:
		case GS_ADJUSTMENT:
		case GS_INCREASING:
		case NJ_KASUMIKIRI:
		case NJ_UTSUSEMI:
		case NJ_NEN:
		case NPC_DEFENDER:
		case NPC_MAGICMIRROR:
		case ST_PRESERVE:
		case NPC_INVINCIBLE:
		case NPC_INVINCIBLEOFF:
		case RK_DEATHBOUND:
		case AB_RENOVATIO:
		case AB_EXPIATIO:
		case AB_DUPLELIGHT:
		case AB_SECRAMENT:
		case NC_ACCELERATION:
		case NC_HOVERING:
		case NC_SHAPESHIFT:
		case WL_RECOGNIZEDSPELL:
		case GC_VENOMIMPRESS:
		case SC_INVISIBILITY:
		case SC_DEADLYINFECT:
		case LG_EXEEDBREAK:
		case LG_PRESTIGE:
		case SR_CRESCENTELBOW:
		case SR_LIGHTNINGWALK:
		case SR_GENTLETOUCH_ENERGYGAIN:
		case GN_CARTBOOST:
		case KO_MEIKYOUSISUI:
		case ALL_FULL_THROTTLE:
		case RA_UNLIMIT:
		case WL_TELEKINESIS_INTENSE:
		case AB_OFFERTORIUM:
		case RK_GIANTGROWTH:
		case RK_VITALITYACTIVATION:
		case RK_ABUNDANCE:
		case RK_CRUSHSTRIKE:
		case ALL_ODINS_POWER:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;
		case KN_AUTOCOUNTER:
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
				skill->addtimerskill(src, tick + 100, bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
			break;
		case SO_STRIKING:
			if (sd) {
				int bonus = 25 + 10 * skill_lv;
				bonus += (pc->checkskill(sd, SA_FLAMELAUNCHER)+pc->checkskill(sd, SA_FROSTWEAPON)+pc->checkskill(sd, SA_LIGHTNINGLOADER)+pc->checkskill(sd, SA_SEISMICWEAPON))*5;
				clif->skill_nodamage( src, bl, skill_id, skill_lv,
									battle->check_target(src,bl,BCT_PARTY) > 0 ?
									sc_start2(src, bl, type, 100, skill_lv, bonus, skill->get_time(skill_id,skill_lv)) :
									0
					);
			}
			break;
		case NPC_STOP:
			if( clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,100,skill_lv,src->id,skill->get_time(skill_id,skill_lv)) ) )
				sc_start2(src,src,type,100,skill_lv,bl->id,skill->get_time(skill_id,skill_lv));
			break;
		case HP_ASSUMPTIO:
			if( sd && dstmd )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			else
				clif->skill_nodamage(src,bl,skill_id,skill_lv,
					sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;
		case MG_SIGHT:
		case MER_SIGHT:
		case AL_RUWACH:
		case WZ_SIGHTBLASTER:
		case NPC_WIDESIGHT:
		case NPC_STONESKIN:
		case NPC_ANTIMAGIC:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,100,skill_lv,skill_id,skill->get_time(skill_id,skill_lv)));
			break;
		case HLIF_AVOID:
		case HAMI_DEFENCE:
		{
			int duration = skill->get_time(skill_id,skill_lv);
			clif->skill_nodamage(bl,bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,duration)); // Master
			clif->skill_nodamage(src,src,skill_id,skill_lv,sc_start(src,src,type,100,skill_lv,duration)); // Homun
		}
			break;
		case NJ_BUNSINJYUTSU:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			status_change_end(bl, SC_NJ_NEN, INVALID_TIMER);
			break;
#if 0 /* Was modified to only affect targetted char. [Skotlex] */
		case HP_ASSUMPTIO:
			if (flag&1)
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
			else {
				map->foreachinrange(skill->area_sub, bl,
				                    skill->get_splash(skill_id, skill_lv), BL_PC,
				                    src, skill_id, skill_lv, tick, flag|BCT_ALL|1,
				                    skill->castend_nodamage_id);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;
#endif // 0
		case SM_ENDURE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			if (sd)
				skill->blockpc_start (sd, skill_id, skill->get_time2(skill_id,skill_lv));
			break;

		case ALL_ANGEL_PROTECT:
			if( dstsd )
				clif->skill_nodamage(src,bl,skill_id,skill_lv,
					sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			else if( sd )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			break;
		case AS_ENCHANTPOISON: // Prevent spamming [Valaris]
			if (sd && dstsd && dstsd->sc.count) {
				if (dstsd->sc.data[SC_PROPERTYFIRE] ||
					dstsd->sc.data[SC_PROPERTYWATER] ||
					dstsd->sc.data[SC_PROPERTYWIND] ||
					dstsd->sc.data[SC_PROPERTYGROUND] ||
					dstsd->sc.data[SC_PROPERTYDARK] ||
					dstsd->sc.data[SC_PROPERTYTELEKINESIS]
					//dstsd->sc.data[SC_ENCHANTPOISON] //People say you should be able to recast to lengthen the timer. [Skotlex]
				) {
						clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						break;
				}
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;

		case LK_TENSIONRELAX:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,0,0,skill->get_time2(skill_id,skill_lv),
					skill->get_time(skill_id,skill_lv)));
			break;

		case MC_CHANGECART:
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			break;

		case MC_CARTDECORATE:
			if (sd) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				clif->selectcart(sd);
			}
			break;

		case TK_MISSION:
			if (sd) {
				int id;
				if (sd->mission_mobid && (sd->mission_count || rnd()%100)) { //Cannot change target when already have one
					clif->mission_info(sd, sd->mission_mobid, sd->mission_count);
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				id = mob->get_random_id(0,0xF, sd->status.base_level);
				if (!id) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				sd->mission_mobid = id;
				sd->mission_count = 0;
				pc_setglobalreg(sd,script->add_str("TK_MISSION_ID"), id);
				clif->mission_info(sd, id, 0);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case AC_CONCENTRATION:
		{
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
			                     sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			map->foreachinrange(status->change_timer_sub, src,
			                    skill->get_splash(skill_id, skill_lv), BL_CHAR,
			                    src,NULL,type,tick);
		}
			break;

		case SM_PROVOKE:
		case SM_SELFPROVOKE:
		case MER_PROVOKE:
		{
			int failure;
			if( (tstatus->mode&MD_BOSS) || battle->check_undead(tstatus->race,tstatus->def_ele) ) {
				map->freeblock_unlock();
				return 1;
			}
			//TODO: How much does base level affects? Dummy value of 1% per level difference used. [Skotlex]
			clif->skill_nodamage(src,bl,skill_id == SM_SELFPROVOKE ? SM_PROVOKE : skill_id,skill_lv,
				(failure = sc_start(src,bl,type, skill_id == SM_SELFPROVOKE ? 100:( 50 + 3*skill_lv + status->get_lv(src) - status->get_lv(bl)), skill_lv, skill->get_time(skill_id,skill_lv))));
			if( !failure ) {
				if( sd )
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				map->freeblock_unlock();
				return 0;
			}
			unit->skillcastcancel(bl, 2);

			if( tsc && tsc->count )
			{
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				if( tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE )
					status_change_end(bl, SC_STONE, INVALID_TIMER);
				status_change_end(bl, SC_SLEEP, INVALID_TIMER);
				status_change_end(bl, SC_TRICKDEAD, INVALID_TIMER);
			}

			if( dstmd )
			{
				dstmd->state.provoke_flag = src->id;
				mob->target(dstmd, src, skill->get_range2(src,skill_id,skill_lv));
			}
		}
			break;

		case ML_DEVOTION:
		case CR_DEVOTION:
			{
				int count, lv, i;
				if( !dstsd || (!sd && !mer) )
				{ // Only players can be devoted
					if( sd )
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
					break;
				}

				if( (lv = status->get_lv(src) - dstsd->status.base_level) < 0 )
					lv = -lv;
				if( lv > battle_config.devotion_level_difference || // Level difference requeriments
					(dstsd->sc.data[type] && dstsd->sc.data[type]->val1 != src->id) || // Cannot Devote a player devoted from another source
					(skill_id == ML_DEVOTION && (!mer || mer != dstsd->md)) || // Mercenary only can devote owner
					(dstsd->class_&MAPID_UPPERMASK) == MAPID_CRUSADER || // Crusader Cannot be devoted
					(dstsd->sc.data[SC_HELLPOWER])) // Players affected by SC_HELLPOWERR cannot be devoted.
				{
					if( sd )
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					map->freeblock_unlock();
					return 1;
				}

				i = 0;
				count = (sd)? min(skill_lv,MAX_PC_DEVOTION) : 1; // Mercenary only can Devote owner
				if( sd )
				{ // Player Devoting Player
					ARR_FIND(0, count, i, sd->devotion[i] == bl->id );
					if( i == count )
					{
						ARR_FIND(0, count, i, sd->devotion[i] == 0 );
						if( i == count ) {
							// No free slots, skill Fail
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
							map->freeblock_unlock();
							return 1;
						}
					}

					sd->devotion[i] = bl->id;
				}
				else
					mer->devotion_flag = 1; // Mercenary Devoting Owner

				clif->skill_nodamage(src, bl, skill_id, skill_lv,
					sc_start4(src, bl, type, 100, src->id, i, skill->get_range2(src,skill_id,skill_lv),0, skill->get_time2(skill_id, skill_lv)));
				clif->devotion(src, NULL);
			}
			break;

		case MO_CALLSPIRITS:
			if(sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				pc->addspiritball(sd, skill->get_time(skill_id, skill_lv), pc->getmaxspiritball(sd, 0));
			}
			break;

		case CH_SOULCOLLECT:
			if(sd) {
				int i, limit = pc->getmaxspiritball(sd, 5);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				for ( i = 0; i < limit; i++ )
					pc->addspiritball(sd, skill->get_time(skill_id, skill_lv), limit);
			}
			break;

		case MO_KITRANSLATION:
			if(dstsd && ((dstsd->class_&MAPID_BASEMASK)!=MAPID_GUNSLINGER || (dstsd->class_&MAPID_UPPERMASK)!=MAPID_REBELLION)) {
				pc->addspiritball(dstsd,skill->get_time(skill_id,skill_lv),5);
			}
			break;

		case TK_TURNKICK:
		case MO_BALKYOUNG: //Passive part of the attack. Splash knock-back+stun. [Skotlex]
			if (skill->area_temp[1] != bl->id) {
				skill->blown(src,bl,skill->get_blewcount(skill_id,skill_lv),-1,0);
				skill->additional_effect(src,bl,skill_id,skill_lv,BF_MISC,ATK_DEF,tick); //Use Misc rather than weapon to signal passive pushback
			}
			break;

		case MO_ABSORBSPIRITS:
		{
			int sp = 0;
			if ( dstsd && dstsd->spiritball
				&& (sd == dstsd || map_flag_vs(src->m) || (sd && sd->duel_group && sd->duel_group == dstsd->duel_group))
				&& ((dstsd->class_&MAPID_BASEMASK) != MAPID_GUNSLINGER || (dstsd->class_&MAPID_UPPERMASK) != MAPID_REBELLION)
				) {
				// split the if for readability, and included gunslingers in the check so that their coins cannot be removed [Reddozen]
				sp = dstsd->spiritball * 7;
				pc->delspiritball(dstsd, dstsd->spiritball, 0);
			} else if ( dstmd && !(tstatus->mode&MD_BOSS) && rnd() % 100 < 20 ) {
				// check if target is a monster and not a Boss, for the 20% chance to absorb 2 SP per monster's level [Reddozen]
				sp = 2 * dstmd->level;
				mob->target(dstmd,src,0);
			}
			if (dstsd && dstsd->charm_type != CHARM_TYPE_NONE && dstsd->charm_count > 0) {
				pc->del_charm(dstsd, dstsd->charm_count, dstsd->charm_type);
			}
			if (sp) status->heal(src, 0, sp, 3);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,sp?1:0);
		}
			break;

		case AC_MAKINGARROW:
			if(sd) {
				clif->arrow_create_list(sd);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case AM_PHARMACY:
			if(sd) {
				clif->skill_produce_mix_list(sd,skill_id,22);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case SA_CREATECON:
			if(sd) {
				clif->elementalconverter_list(sd);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case BS_HAMMERFALL:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,SC_STUN,(20 + 10 * skill_lv),skill_lv,skill->get_time2(skill_id,skill_lv)));
			break;
		case RG_RAID:
			skill->area_temp[1] = 0;
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			map->foreachinrange(skill->area_sub, bl,
			                    skill->get_splash(skill_id, skill_lv), splash_target(src),
			                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
			                    skill->castend_damage_id);
			status_change_end(src, SC_HIDING, INVALID_TIMER);
			break;

		case ASC_METEORASSAULT:
		case GS_SPREADATTACK:
		case RK_STORMBLAST:
		case NC_AXETORNADO:
		case GC_COUNTERSLASH:
		case SR_SKYNETBLOW:
		case SR_RAMPAGEBLASTER:
		case SR_HOWLINGOFLION:
		case KO_HAPPOKUNAI:
		{
			int count = 0;
			skill->area_temp[1] = 0;
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			count = map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src),
			                        src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);
			if( !count && ( skill_id == NC_AXETORNADO || skill_id == SR_SKYNETBLOW || skill_id == KO_HAPPOKUNAI ) )
				clif->skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
		}
			break;

		case NC_EMERGENCYCOOL:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status_change_end(src,SC_OVERHEAT_LIMITPOINT,INVALID_TIMER);
			status_change_end(src,SC_OVERHEAT,INVALID_TIMER);
			break;
		case SR_WINDMILL:
		case GN_CART_TORNADO:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			/* Fall through */
		case SR_EARTHSHAKER:
		case NC_INFRAREDSCAN:
		case NPC_VAMPIRE_GIFT:
		case NPC_HELLJUDGEMENT:
		case NPC_PULSESTRIKE:
		case LG_MOONSLASHER:
			skill->castend_damage_id(src, src, skill_id, skill_lv, tick, flag);
			break;

		case KN_BRANDISHSPEAR:
		case ML_BRANDISH:
			skill->brandishspear(src, bl, skill_id, skill_lv, tick, flag);
			break;

		case WZ_SIGHTRASHER:
			//Passive side of the attack.
			status_change_end(src, SC_SIGHT, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			map->foreachinrange(skill->area_sub,src,
			                    skill->get_splash(skill_id, skill_lv),BL_CHAR|BL_SKILL,
			                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
			                    skill->castend_damage_id);
			break;

		case NJ_HYOUSYOURAKU:
		case NJ_RAIGEKISAI:
		case WZ_FROSTNOVA:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			skill->area_temp[1] = 0;
			map->foreachinrange(skill->attack_area, src,
			                    skill->get_splash(skill_id, skill_lv), splash_target(src),
			                    BF_MAGIC, src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
			break;

		case HVAN_EXPLOSION: // [orn]
		case NPC_SELFDESTRUCTION:
		{
			//Self Destruction hits everyone in range (allies+enemies)
			//Except for Summoned Marine spheres on non-versus maps, where it's just enemy.
			int targetmask = ((!md || md->special_state.ai == AI_SPHERE) && !map_flag_vs(src->m))?
				BCT_ENEMY:BCT_ALL;
			clif->skill_nodamage(src, src, skill_id, -1, 1);
			map->delblock(src); //Required to prevent chain-self-destructions hitting back.
			map->foreachinrange(skill->area_sub, bl,
			                    skill->get_splash(skill_id, skill_lv), splash_target(src),
			                    src, skill_id, skill_lv, tick, flag|targetmask,
			                    skill->castend_damage_id);
			map->addblock(src);
			status->damage(src, src, sstatus->max_hp,0,0,1);
		}
			break;

		case AL_ANGELUS:
		case PR_MAGNIFICAT:
		case PR_GLORIA:
		case SN_WINDWALK:
		case CASH_BLESSING:
		case CASH_INCAGI:
		case CASH_ASSUMPTIO:
		case WM_FRIGG_SONG:
			if( sd == NULL || sd->status.party_id == 0 || (flag & 1) )
				clif->skill_nodamage(bl, bl, skill_id, skill_lv, sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			else if( sd )
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			break;
		case MER_MAGNIFICAT:
			if( mer != NULL )
			{
				clif->skill_nodamage(bl, bl, skill_id, skill_lv, sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
				if( mer->master && mer->master->status.party_id != 0 && !(flag&1) )
					party->foreachsamemap(skill->area_sub, mer->master, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
				else if( mer->master && !(flag&1) )
					clif->skill_nodamage(src, &mer->master->bl, skill_id, skill_lv, sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			}
			break;

		case BS_ADRENALINE:
		case BS_ADRENALINE2:
		case BS_WEAPONPERFECT:
		case BS_OVERTHRUST:
			if (sd == NULL || sd->status.party_id == 0 || (flag & 1)) {
				clif->skill_nodamage(bl,bl,skill_id,skill_lv,
					sc_start2(src,bl,type,100,skill_lv,(src == bl)? 1:0,skill->get_time(skill_id,skill_lv)));
			} else if (sd) {
				party->foreachsamemap(skill->area_sub,
					sd,skill->get_splash(skill_id, skill_lv),
					src,skill_id,skill_lv,tick, flag|BCT_PARTY|1,
					skill->castend_nodamage_id);
			}
			break;

		case BS_MAXIMIZE:
		case NV_TRICKDEAD:
		case CR_DEFENDER:
		case ML_DEFENDER:
		case CR_AUTOGUARD:
		case ML_AUTOGUARD:
		case TK_READYSTORM:
		case TK_READYDOWN:
		case TK_READYTURN:
		case TK_READYCOUNTER:
		case TK_DODGE:
		case CR_SHRINK:
		case SG_FUSION:
		case GS_GATLINGFEVER:
			if( tsce )
			{
				clif->skill_nodamage(src,bl,skill_id,skill_lv,status_change_end(bl, type, INVALID_TIMER));
				map->freeblock_unlock();
				return 0;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;
		case SL_KAITE:
		case SL_KAAHI:
		case SL_KAIZEL:
		case SL_KAUPE:
			if (sd) {
				if (!dstsd || !(
				                (sd->sc.data[SC_SOULLINK] && sd->sc.data[SC_SOULLINK]->val2 == SL_SOULLINKER)
				             || (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER
				             || dstsd->status.char_id == sd->status.char_id
				             || dstsd->status.char_id == sd->status.partner_id
				             || dstsd->status.char_id == sd->status.child
				               )
				) {
					status->change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCFLAG_FIXEDRATE);
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id, skill_lv)));
			break;
		case SM_AUTOBERSERK:
		case MER_AUTOBERSERK:
		{
			int failure;
			if( tsce )
				failure = status_change_end(bl, type, INVALID_TIMER);
			else
				failure = sc_start(src,bl,type,100,skill_lv,60000);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,failure);
		}
			break;
		case TF_HIDING:
		case ST_CHASEWALK:
		case KO_YAMIKUMO:
			if (tsce) {
				clif->skill_nodamage(src,bl,skill_id,-1,status_change_end(bl, type, INVALID_TIMER)); //Hide skill-scream animation.
				map->freeblock_unlock();
				return 0;
			} else if( tsc && tsc->option&OPTION_MADOGEAR ) {
				//Mado Gear cannot hide
				if( sd ) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				map->freeblock_unlock();
				return 0;
			}
			clif->skill_nodamage(src,bl,skill_id,-1,sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;
		case TK_RUN:
			if (tsce) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,status_change_end(bl, type, INVALID_TIMER));
				map->freeblock_unlock();
				return 0;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start4(src,bl,type,100,skill_lv,unit->getdir(bl),0,0,0));
			if (sd) // If the client receives a skill-use packet immediately before a walkok packet, it will discard the walk packet! [Skotlex]
				clif->walkok(sd); // So aegis has to resend the walk ok.
			break;
		case AS_CLOAKING:
		case GC_CLOAKINGEXCEED:
		case LG_FORCEOFVANGUARD:
		case SC_REPRODUCE:
		case RA_CAMOUFLAGE:
			if (tsce) {
				int failure = status_change_end(bl, type, INVALID_TIMER);
				if( failure )
					clif->skill_nodamage(src,bl,skill_id,( skill_id == LG_FORCEOFVANGUARD ) ? skill_lv : -1,failure);
				else if( sd )
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				if ( skill_id == LG_FORCEOFVANGUARD || skill_id == RA_CAMOUFLAGE )
					break;
				map->freeblock_unlock();
				return 0;
			} else {
				int failure = sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
				if( failure )
					clif->skill_nodamage(src,bl,skill_id,( skill_id == LG_FORCEOFVANGUARD ) ? skill_lv : -1,failure);
				else if( sd )
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case BD_ADAPTATION:
			if(tsc && tsc->data[SC_DANCING]){
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				status_change_end(bl, SC_DANCING, INVALID_TIMER);
			}
			break;

		case BA_FROSTJOKER:
		case DC_SCREAM:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			skill->addtimerskill(src,tick+2000,bl->id,src->x,src->y,skill_id,skill_lv,0,flag);

			if (md) {
				// custom hack to make the mob display the skill, because these skills don't show the skill use text themselves
				//NOTE: mobs don't have the sprite animation that is used when performing this skill (will cause glitches)
				char temp[70];
				snprintf(temp, sizeof(temp), "%s : %s !!",md->name,skill->dbs->db[skill_id].desc);
				clif->disp_overhead(&md->bl,temp);
			}
			break;

		case BA_PANGVOICE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv, sc_start(src,bl,SC_CONFUSION,50,7,skill->get_time(skill_id,skill_lv)));
			break;

		case DC_WINKCHARM:
			if( dstsd )
				clif->skill_nodamage(src,bl,skill_id,skill_lv, sc_start(src,bl,SC_CONFUSION,30,7,skill->get_time2(skill_id,skill_lv)));
			else if( dstmd ) {
				if( status->get_lv(src) > status->get_lv(bl)
				 && (tstatus->race == RC_DEMON || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_ANGEL)
				 && !(tstatus->mode&MD_BOSS)
				) {
					clif->skill_nodamage(src,bl,skill_id,skill_lv, sc_start2(src,bl,type,70,skill_lv,src->id,skill->get_time(skill_id,skill_lv)));
				} else {
					clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
					if(sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				}
			}
			break;

		case TF_STEAL:
			if(sd) {
				if(pc->steal_item(sd,bl,skill_lv))
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				else
					clif->skill_fail(sd,skill_id,USESKILL_FAIL,0);
			}
			break;

		case RG_STEALCOIN:
			if(sd) {
				int amount = pc->steal_coin(sd, bl);
				if( amount > 0 ) {
					dstmd->state.provoke_flag = src->id;
					mob->target(dstmd, src, skill->get_range2(src, skill_id, skill_lv));
					clif->skill_nodamage(src, bl, skill_id, amount, 1);

				} else
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
			}
			break;

		case MG_STONECURSE:
			{
				int brate = 0;
				if (tstatus->mode&MD_BOSS) {
					if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				if(status->isimmune(bl) || !tsc)
					break;

				if (sd && sd->sc.data[SC_PETROLOGY_OPTION])
					brate = sd->sc.data[SC_PETROLOGY_OPTION]->val3;

				if (tsc->data[SC_STONE]) {
					status_change_end(bl, SC_STONE, INVALID_TIMER);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					break;
				}
				if (sc_start4(src,bl,SC_STONE,(skill_lv*4+20)+brate,
					skill_lv, 0, 0, skill->get_time(skill_id, skill_lv),
					skill->get_time2(skill_id,skill_lv)))
						clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				else if(sd) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					// Level 6-10 doesn't consume a red gem if it fails [celest]
					if (skill_lv > 5) {
						// not to consume items
						map->freeblock_unlock();
						return 0;
					}
				}
			}
			break;

		case NV_FIRSTAID:
			clif->skill_nodamage(src,bl,skill_id,5,1);
			status->heal(bl,5,0,0);
			break;

		case AL_CURE:
			if(status->isimmune(bl)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
				break;
			}
			status_change_end(bl, SC_SILENCE, INVALID_TIMER);
			status_change_end(bl, SC_BLIND, INVALID_TIMER);
			status_change_end(bl, SC_CONFUSION, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case TF_DETOXIFY:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status_change_end(bl, SC_POISON, INVALID_TIMER);
			status_change_end(bl, SC_DPOISON, INVALID_TIMER);
			break;

		case PR_STRECOVERY:
			if(status->isimmune(bl)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
				break;
			}
			if (tsc && tsc->opt1) {
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_STONE, INVALID_TIMER);
				status_change_end(bl, SC_SLEEP, INVALID_TIMER);
				status_change_end(bl, SC_STUN, INVALID_TIMER);
				status_change_end(bl, SC_WHITEIMPRISON, INVALID_TIMER);
			}
			status_change_end(bl, SC_NETHERWORLD, INVALID_TIMER);
			//Is this equation really right? It looks so... special.
			if( battle->check_undead(tstatus->race,tstatus->def_ele) ) {
				status->change_start(src, bl, SC_BLIND,
				                     100*(100-(tstatus->int_/2+tstatus->vit/3+tstatus->luk/10)), 1,0,0,0,
				                     skill->get_time2(skill_id, skill_lv) * (100-(tstatus->int_+tstatus->vit)/2)/100,SCFLAG_NONE);
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if(dstmd)
				mob->unlocktarget(dstmd,tick);
			break;

		// Mercenary Supportive Skills
		case MER_BENEDICTION:
			status_change_end(bl, SC_CURSE, INVALID_TIMER);
			status_change_end(bl, SC_BLIND, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case MER_COMPRESS:
			status_change_end(bl, SC_BLOODING, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case MER_MENTALCURE:
			status_change_end(bl, SC_CONFUSION, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case MER_RECUPERATE:
			status_change_end(bl, SC_POISON, INVALID_TIMER);
			status_change_end(bl, SC_SILENCE, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case MER_REGAIN:
			status_change_end(bl, SC_SLEEP, INVALID_TIMER);
			status_change_end(bl, SC_STUN, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case MER_TENDER:
			status_change_end(bl, SC_FREEZE, INVALID_TIMER);
			status_change_end(bl, SC_STONE, INVALID_TIMER);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case MER_SCAPEGOAT:
			if( mer && mer->master ) {
				status->heal(&mer->master->bl, mer->battle_status.hp, 0, 2);
				status->damage(src, src, mer->battle_status.max_hp, 0, 0, 1);
			}
			break;

		case MER_ESTIMATION:
			if( !mer )
				break;
			sd = mer->master;
		case WZ_ESTIMATION:
			if( sd == NULL )
				break;
			if( dstsd )
			{ // Fail on Players
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if (dstmd && dstmd->class_ == MOBID_EMPELIUM)
				break; // Cannot be Used on Emperium

			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			clif->skill_estimation(sd, bl);
			if( skill_id == MER_ESTIMATION )
				sd = NULL;
			break;

		case BS_REPAIRWEAPON:
			if(sd && dstsd)
				clif->item_repair_list(sd,dstsd,skill_lv);
			break;

		case MC_IDENTIFY:
			if(sd) {
				clif->item_identify_list(sd);
				if( sd->menuskill_id != MC_IDENTIFY ) {/* failed, don't consume anything, return */
					map->freeblock_unlock();
					return 1;
				}
				if( sd->skillitem != skill_id )
					status_zap(src,0,skill->dbs->db[skill->get_index(skill_id)].sp[skill_lv]); // consume sp only if succeeded
			}
			break;

		// Weapon Refining [Celest]
		case WS_WEAPONREFINE:
			if(sd){
				sd->state.prerefining = 1;
				clif->item_refine_list(sd);
			}
			break;

		case MC_VENDING:
			if (sd) {
				//Prevent vending of GMs with unnecessary Level to trade/drop. [Skotlex]
				if ( !pc_can_give_items(sd) )
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				else {
					sd->state.prevend = sd->state.workinprogress = 3;
					clif->openvendingreq(sd,2+skill_lv);
				}
			}
			break;

		case AL_TELEPORT:
			if(sd) {
				if (map->list[bl->m].flag.noteleport && skill_lv <= 2) {
					clif->skill_mapinfomessage(sd,0);
					break;
				}
				if(!battle_config.duel_allow_teleport && sd->duel_group && skill_lv <= 2) { // duel restriction [LuzZza]
					char output[128]; sprintf(output, msg_sd(sd,365), skill->get_name(AL_TELEPORT));
					clif->message(sd->fd, output); //"Duel: Can't use %s in duel."
					break;
				}

				if( sd->state.autocast || ( (sd->skillitem == AL_TELEPORT || battle_config.skip_teleport_lv1_menu) && skill_lv == 1 ) || skill_lv == 3 )
				{
					if( skill_lv == 1 )
						pc->randomwarp(sd,CLR_TELEPORT);
					else
						pc->setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
					break;
				}

				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if( skill_lv == 1 )
					clif->skill_warppoint(sd,skill_id,skill_lv, (unsigned short)-1,0,0,0);
				else
					clif->skill_warppoint(sd,skill_id,skill_lv, (unsigned short)-1,sd->status.save_point.map,0,0);
			} else
				unit->warp(bl,-1,-1,-1,CLR_TELEPORT);
			break;

		case NPC_EXPULSION:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			unit->warp(bl,-1,-1,-1,CLR_TELEPORT);
			break;

		case AL_HOLYWATER:
			if(sd) {
				if (skill->produce_mix(sd, skill_id, ITEMID_HOLY_WATER, 0, 0, 0, 1))
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				else
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case TF_PICKSTONE:
			if(sd) {
				int eflag;
				struct item item_tmp;
				struct block_list tbl;
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				memset(&item_tmp,0,sizeof(item_tmp));
				memset(&tbl,0,sizeof(tbl)); // [MouseJstr]
				item_tmp.nameid = ITEMID_STONE;
				item_tmp.identify = 1;
				tbl.id = 0;
				clif->takeitem(&sd->bl,&tbl);
				eflag = pc->additem(sd,&item_tmp,1,LOG_TYPE_PRODUCE);
				if(eflag) {
					clif->additem(sd,0,0,eflag);
					map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
				}
			}
			break;
		case ASC_CDP:
			if(sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				skill->produce_mix(sd, skill_id, ITEMID_POISON_BOTTLE, 0, 0, 0, 1); //Produce a Deadly Poison Bottle.
			}
			break;

		case RG_STRIPWEAPON:
		case RG_STRIPSHIELD:
		case RG_STRIPARMOR:
		case RG_STRIPHELM:
		case ST_FULLSTRIP:
		case GC_WEAPONCRUSH:
		case SC_STRIPACCESSARY: {
			unsigned short location = 0;
			int d = 0, rate;

			//Rate in percent
			if ( skill_id == ST_FULLSTRIP ) {
				rate = 5 + 2*skill_lv + (sstatus->dex - tstatus->dex)/5;
			} else if( skill_id == SC_STRIPACCESSARY ) {
				rate = 12 + 2 * skill_lv + (sstatus->dex - tstatus->dex)/5;
			} else {
				rate = 5 + 5*skill_lv + (sstatus->dex - tstatus->dex)/5;
			}

			if (rate < 5) rate = 5; //Minimum rate 5%

			//Duration in ms
			if( skill_id == GC_WEAPONCRUSH){
				d = skill->get_time(skill_id,skill_lv);
				if(bl->type == BL_PC)
					d += 1000 * ( skill_lv * 15 + ( sstatus->dex - tstatus->dex ) );
				else
					d += 1000 * ( skill_lv * 30 + ( sstatus->dex - tstatus->dex ) / 2 );
			}else
				d = skill->get_time(skill_id,skill_lv) + (sstatus->dex - tstatus->dex)*500;

			if (d < 0) d = 0; //Minimum duration 0ms

			switch (skill_id) {
			case RG_STRIPWEAPON:
			case GC_WEAPONCRUSH:
				location = EQP_WEAPON;
				break;
			case RG_STRIPSHIELD:
				location = EQP_SHIELD;
				break;
			case RG_STRIPARMOR:
				location = EQP_ARMOR;
				break;
			case RG_STRIPHELM:
				location = EQP_HELM;
				break;
			case ST_FULLSTRIP:
				location = EQP_WEAPON|EQP_SHIELD|EQP_ARMOR|EQP_HELM;
				break;
			case SC_STRIPACCESSARY:
				location = EQP_ACC;
				break;
			}

			//Special message when trying to use strip on FCP [Jobbie]
			if( sd && skill_id == ST_FULLSTRIP && tsc && tsc->data[SC_PROTECTWEAPON] && tsc->data[SC_PROTECTHELM] && tsc->data[SC_PROTECTARMOR] && tsc->data[SC_PROTECTSHIELD])
			{
				clif->gospel_info(sd, 0x28);
				break;
			}

			//Attempts to strip at rate i and duration d
			if( (rate = skill->strip_equip(bl, location, rate, skill_lv, d)) || (skill_id != ST_FULLSTRIP && skill_id != GC_WEAPONCRUSH ) )
				clif->skill_nodamage(src,bl,skill_id,skill_lv,rate);

			//Nothing stripped.
			if( sd && !rate )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			break;
		}

		case AM_BERSERKPITCHER:
		case AM_POTIONPITCHER:
			{
				int i,sp = 0;
				int64 hp = 0;
				if (dstmd && dstmd->class_ == MOBID_EMPELIUM) {
					map->freeblock_unlock();
					return 1;
				}
				if( sd ) {
					int x,bonus=100, potion = min(500+skill_lv,505);
					x = skill_lv%11 - 1;
					i = pc->search_inventory(sd,skill->dbs->db[skill_id].itemid[x]);
					if (i == INDEX_NOT_FOUND || skill->dbs->db[skill_id].itemid[x] <= 0) {
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						map->freeblock_unlock();
						return 1;
					}
					if(sd->inventory_data[i] == NULL || sd->status.inventory[i].amount < skill->dbs->db[skill_id].amount[x]) {
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						map->freeblock_unlock();
						return 1;
					}
					if( skill_id == AM_BERSERKPITCHER ) {
						if( dstsd && dstsd->status.base_level < (unsigned int)sd->inventory_data[i]->elv ) {
							clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
							map->freeblock_unlock();
							return 1;
						}
					}
					script->potion_flag = 1;
					script->potion_hp = script->potion_sp = script->potion_per_hp = script->potion_per_sp = 0;
					script->potion_target = bl->id;
					script->run_use_script(sd, sd->inventory_data[i], 0);
					script->potion_flag = script->potion_target = 0;
					if( sd->sc.data[SC_SOULLINK] && sd->sc.data[SC_SOULLINK]->val2 == SL_ALCHEMIST )
						bonus += sd->status.base_level;
					if( script->potion_per_hp > 0 || script->potion_per_sp > 0 ) {
						hp = tstatus->max_hp * script->potion_per_hp / 100;
						hp = hp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						if( dstsd ) {
							sp = dstsd->status.max_sp * script->potion_per_sp / 100;
							sp = sp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						}
					} else {
						if( script->potion_hp > 0 ) {
							hp = script->potion_hp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
							hp = hp * (100 + (tstatus->vit<<1)) / 100;
							if( dstsd )
								hp = hp * (100 + pc->checkskill(dstsd,SM_RECOVERY)*10) / 100;
						}
						if( script->potion_sp > 0 ) {
							sp = script->potion_sp * (100 + pc->checkskill(sd,AM_POTIONPITCHER)*10 + pc->checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
							sp = sp * (100 + (tstatus->int_<<1)) / 100;
							if( dstsd )
								sp = sp * (100 + pc->checkskill(dstsd,MG_SRECOVERY)*10) / 100;
						}
					}

					for(i = 0; i < ARRAYLENGTH(sd->itemhealrate) && sd->itemhealrate[i].nameid; i++) {
						if (sd->itemhealrate[i].nameid == potion) {
							hp += hp * sd->itemhealrate[i].rate / 100;
							sp += sp * sd->itemhealrate[i].rate / 100;
							break;
						}
					}

					if( (i = pc->skillheal_bonus(sd, skill_id)) ) {
						hp += hp * i / 100;
						sp += sp * i / 100;
					}
				} else {
					//Maybe replace with potion_hp, but I'm unsure how that works [Playtester]
					switch (skill_lv) {
						case 1: hp = 45; break;
						case 2: hp = 105; break;
						case 3: hp = 175; break;
						default: hp = 325; break;
					}
					hp = (hp + rnd()%(skill_lv*20+1)) * (150 + skill_lv*10) / 100;
					hp = hp * (100 + (tstatus->vit<<1)) / 100;
					if( dstsd )
						hp = hp * (100 + pc->checkskill(dstsd,SM_RECOVERY)*10) / 100;
				}
				if (dstsd && (i = pc->skillheal2_bonus(dstsd, skill_id)) != 0) {
					hp += hp * i / 100;
					sp += sp * i / 100;
				}
				if( tsc && tsc->count ) {
					if( tsc->data[SC_CRITICALWOUND] ) {
						hp -= hp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
						sp -= sp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
					}
					if( tsc->data[SC_DEATHHURT] ) {
						hp -= hp * 20 / 100;
						sp -= sp * 20 / 100;
					}
					if( tsc->data[SC_WATER_INSIGNIA] && tsc->data[SC_WATER_INSIGNIA]->val1 == 2 ) {
						hp += hp / 10;
						sp += sp / 10;
					}
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if( hp > 0 || (skill_id == AM_POTIONPITCHER && sp <= 0) )
					clif->skill_nodamage(NULL,bl,AL_HEAL,(int)hp,1);
				if( sp > 0 )
					clif->skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
		#ifdef RENEWAL
				if( tsc && tsc->data[SC_EXTREMITYFIST2] )
					sp = 0;
		#endif
				status->heal(bl,(int)hp,sp,0);
			}
			break;
		case AM_CP_WEAPON:
		case AM_CP_SHIELD:
		case AM_CP_ARMOR:
		case AM_CP_HELM:
		{
			unsigned int equip[] = { EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP };
			int index;
			if ( sd && (bl->type != BL_PC || (dstsd && pc->checkequip(dstsd, equip[skill_id - AM_CP_WEAPON]) < 0) ||
				(dstsd && equip[skill_id - AM_CP_WEAPON] == EQP_SHIELD && pc->checkequip(dstsd, EQP_SHIELD) > 0
				&& (index = dstsd->equip_index[EQI_HAND_L]) >= 0 && dstsd->inventory_data[index]
				&& dstsd->inventory_data[index]->type != IT_ARMOR)) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
				map->freeblock_unlock(); // Don't consume item requirements
				return 0;
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv,
				sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;
		}
		case AM_TWILIGHT1:
			if (sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				//Prepare 200 White Potions.
				if (!skill->produce_mix(sd, skill_id, ITEMID_WHITE_POTION, 0, 0, 0, 200))
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;
		case AM_TWILIGHT2:
			if (sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				//Prepare 200 Slim White Potions.
				if (!skill->produce_mix(sd, skill_id, ITEMID_WHITE_SLIM_POTION, 0, 0, 0, 200))
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;
		case AM_TWILIGHT3:
			if (sd) {
				int ebottle = pc->search_inventory(sd,ITEMID_EMPTY_BOTTLE);
				if (ebottle != INDEX_NOT_FOUND)
					ebottle = sd->status.inventory[ebottle].amount;
				//check if you can produce all three, if not, then fail:
				if (!skill->can_produce_mix(sd,ITEMID_ALCHOL,-1, 100) //100 Alcohol
					|| !skill->can_produce_mix(sd,ITEMID_ACID_BOTTLE,-1, 50) //50 Acid Bottle
					|| !skill->can_produce_mix(sd,ITEMID_FIRE_BOTTLE,-1, 50) //50 Flame Bottle
					|| ebottle < 200 //200 empty bottle are required at total.
				) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				skill->produce_mix(sd, skill_id, ITEMID_ALCHOL, 0, 0, 0, 100);
				skill->produce_mix(sd, skill_id, ITEMID_ACID_BOTTLE, 0, 0, 0, 50);
				skill->produce_mix(sd, skill_id, ITEMID_FIRE_BOTTLE, 0, 0, 0, 50);
			}
			break;
		case SA_DISPELL:
		{
			int splash;
			if (flag&1 || (splash = skill->get_splash(skill_id, skill_lv)) < 1) {
				int i;
				if( sd && dstsd && !map_flag_vs(sd->bl.m)
					&& (sd->status.party_id == 0 || sd->status.party_id != dstsd->status.party_id) ) {
					// Outside PvP it should only affect party members and no skill fail message.
					break;
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if((dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
					|| (tsc && tsc->data[SC_SOULLINK] && tsc->data[SC_SOULLINK]->val2 == SL_ROGUE) //Rogue's spirit defends against dispel.
					|| (dstsd && pc_ismadogear(dstsd))
					|| rnd()%100 >= 50+10*skill_lv )
				{
					if (sd)
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				if(status->isimmune(bl) || !tsc || !tsc->count)
					break;
				for(i = 0; i < SC_MAX; i++) {
					if ( !tsc->data[i] )
							continue;
					if( SC_COMMON_MAX < i ) {
						if ( status->get_sc_type(i)&SC_NO_DISPELL )
							continue;
					}
					switch (i) {
						/**
						 * bugreport:4888 these songs may only be dispelled if you're not in their song area anymore
						 **/
						case SC_WHISTLE:
						case SC_ASSNCROS:
						case SC_POEMBRAGI:
						case SC_APPLEIDUN:
						case SC_HUMMING:
						case SC_DONTFORGETME:
						case SC_FORTUNE:
						case SC_SERVICEFORYOU:
							if( tsc->data[i]->val4 ) //val4 = out-of-song-area
								continue;
							break;
						case SC_ASSUMPTIO:
							if( bl->type == BL_MOB )
								continue;
							break;
						case SC_BERSERK:
						case SC_SATURDAY_NIGHT_FEVER:
							tsc->data[i]->val2=0;  //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
							break;
					}
					status_change_end(bl, (sc_type)i, INVALID_TIMER);
				}
				break;
			} else {
				//Affect all targets on splash area.
				map->foreachinrange(skill->area_sub, bl, splash, BL_CHAR,
				                    src, skill_id, skill_lv, tick, flag|1,
				                    skill->castend_damage_id);
			}
		}
			break;

		case TF_BACKSLIDING: //This is the correct implementation as per packet logging information. [Skotlex]
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			skill->blown(src,bl,skill->get_blewcount(skill_id,skill_lv),unit->getdir(bl),0);
			clif->fixpos(bl);
			break;

		case TK_HIGHJUMP:
			{
				int x,y, dir = unit->getdir(src);

				//Fails on noteleport maps, except for GvG and BG maps [Skotlex]
				if( map->list[src->m].flag.noteleport
				 && !(map->list[src->m].flag.battleground || map_flag_gvg2(src->m))
				) {
					x = src->x;
					y = src->y;
				} else {
					x = src->x + dirx[dir]*skill_lv*2;
					y = src->y + diry[dir]*skill_lv*2;
				}

				clif->skill_nodamage(src,bl,TK_HIGHJUMP,skill_lv,1);
				if (!map->count_oncell(src->m, x, y, BL_PC | BL_NPC | BL_MOB, 0) && map->getcell(src->m, src, x, y, CELL_CHKREACH)) {
					clif->slide(src,x,y);
					unit->movepos(src, x, y, 1, 0);
				}
			}
			break;

		case SA_CASTCANCEL:
		case SO_SPELLFIST:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			unit->skillcastcancel(src,1);
			if(sd) {
				int sp = skill->get_sp(sd->skill_id_old,sd->skill_lv_old);
				if( skill_id == SO_SPELLFIST ){
					sc_start4(src,src,type,100,skill_lv+1,skill_lv,sd->skill_id_old,sd->skill_lv_old,skill->get_time(skill_id,skill_lv));
					sd->skill_id_old = sd->skill_lv_old = 0;
					break;
				}
				sp = sp * (90 - (skill_lv-1)*20) / 100;
				if(sp < 0) sp = 0;
				status_zap(src, 0, sp);
			}
			break;
		case SA_SPELLBREAKER:
			{
				int sp;
				if(tsc && tsc->data[SC_MAGICROD]) {
					sp = skill->get_sp(skill_id,skill_lv);
					sp = sp * tsc->data[SC_MAGICROD]->val2 / 100;
					if(sp < 1) sp = 1;
					status->heal(bl,0,sp,2);
					status_percent_damage(bl, src, 0, -20, false); //20% max SP damage.
				} else {
					struct unit_data *ud = unit->bl2ud(bl);
					int bl_skill_id=0,bl_skill_lv=0,hp = 0;
					if (!ud || ud->skilltimer == INVALID_TIMER)
						break; //Nothing to cancel.
					bl_skill_id = ud->skill_id;
					bl_skill_lv = ud->skill_lv;
					if (tstatus->mode & MD_BOSS) {
						//Only 10% success chance against bosses. [Skotlex]
						if (rnd()%100 < 90)
						{
							if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
							break;
						}
					} else if (!dstsd || map_flag_vs(bl->m)) //HP damage only on pvp-maps when against players.
						hp = tstatus->max_hp/50; //Recover 2% HP [Skotlex]

					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					unit->skillcastcancel(bl,0);
					sp = skill->get_sp(bl_skill_id,bl_skill_lv);
					status_zap(bl, hp, sp);

					if (hp && skill_lv >= 5)
						hp>>=1; //Recover half damaged HP at level 5 [Skotlex]
					else
						hp = 0;

					if (sp) //Recover some of the SP used
						sp = sp*(25*(skill_lv-1))/100;

					if(hp || sp)
						status->heal(src, hp, sp, 2);
				}
			}
			break;
		case SA_MAGICROD:
			clif->skill_nodamage(src,src,SA_MAGICROD,skill_lv,1);
			sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case SA_AUTOSPELL:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if(sd){
				sd->state.workinprogress = 3;
				clif->autospell(sd,skill_lv);
			}else {
				int maxlv=1,spellid=0;
				static const int spellarray[3] = { MG_COLDBOLT,MG_FIREBOLT,MG_LIGHTNINGBOLT };
				if(skill_lv >= 10) {
					spellid = MG_FROSTDIVER;
#if 0
					if (tsc && tsc->data[SC_SOULLINK] && tsc->data[SC_SOULLINK]->val2 == SA_SAGE)
						maxlv = 10;
					else
#endif // 0
						maxlv = skill_lv - 9;
				}
				else if(skill_lv >=8) {
					spellid = MG_FIREBALL;
					maxlv = skill_lv - 7;
				}
				else if(skill_lv >=5) {
					spellid = MG_SOULSTRIKE;
					maxlv = skill_lv - 4;
				}
				else if(skill_lv >=2) {
					int i = rnd()%3;
					spellid = spellarray[i];
					maxlv = skill_lv - 1;
				}
				else if(skill_lv > 0) {
					spellid = MG_NAPALMBEAT;
					maxlv = 3;
				}
				if(spellid > 0)
					sc_start4(src,src,SC_AUTOSPELL,100,skill_lv,spellid,maxlv,0,
						skill->get_time(SA_AUTOSPELL,skill_lv));
			}
			break;

		case BS_GREED:
			if(sd){
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->greed,bl,
				                    skill->get_splash(skill_id, skill_lv),BL_ITEM,bl);
			}
			break;

		case SA_ELEMENTWATER:
		case SA_ELEMENTFIRE:
		case SA_ELEMENTGROUND:
		case SA_ELEMENTWIND:
			if(sd && !dstmd) //Only works on monsters.
				break;
			if(tstatus->mode&MD_BOSS)
				break;
		case NPC_ATTRICHANGE:
		case NPC_CHANGEWATER:
		case NPC_CHANGEGROUND:
		case NPC_CHANGEFIRE:
		case NPC_CHANGEWIND:
		case NPC_CHANGEPOISON:
		case NPC_CHANGEHOLY:
		case NPC_CHANGEDARKNESS:
		case NPC_CHANGETELEKINESIS:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src, bl, type, 100, skill_lv, skill->get_ele(skill_id,skill_lv),
					skill->get_time(skill_id, skill_lv)));
			break;
		case NPC_CHANGEUNDEAD:
			//This skill should fail if target is wearing bathory/evil druid card [Brainstorm]
			//TO-DO This is ugly, fix it
			if(tstatus->def_ele==ELE_UNDEAD || tstatus->def_ele==ELE_DARK) break;
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src, bl, type, 100, skill_lv, skill->get_ele(skill_id,skill_lv),
					skill->get_time(skill_id, skill_lv)));
			break;

		case NPC_PROVOCATION:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (md) mob->unlocktarget(md, tick);
			break;

		case NPC_KEEPING:
		case NPC_BARRIER:
			{
				int skill_time = skill->get_time(skill_id,skill_lv);
				struct unit_data *ud = unit->bl2ud(bl);
				if (clif->skill_nodamage(src,bl,skill_id,skill_lv,
						sc_start(src,bl,type,100,skill_lv,skill_time))
				&& ud) {
					//Disable attacking/acting/moving for skill's duration.
					ud->attackabletime =
					ud->canact_tick =
					ud->canmove_tick = tick + skill_time;
				}
			}
			break;

		case NPC_REBIRTH:
			if( md && md->state.rebirth )
				break; // only works once
			sc_start(src, bl, type, 100, skill_lv, INFINITE_DURATION);
			break;

		case NPC_DARKBLESSING:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,(50+skill_lv*5),skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv)));
			break;

		case NPC_LICK:
			status_zap(bl, 0, 100);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,(skill_lv*5),skill_lv,skill->get_time2(skill_id,skill_lv)));
			break;

		case NPC_SUICIDE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status_kill(src); //When suiciding, neither exp nor drops is given.
			break;

		case NPC_SUMMONSLAVE:
		case NPC_SUMMONMONSTER:
			if(md && md->skill_idx >= 0)
				mob->summonslave(md,md->db->skill[md->skill_idx].val,skill_lv,skill_id);
			break;

		case NPC_CALLSLAVE:
			mob->warpslave(src,MOB_SLAVEDISTANCE);
			break;

		case NPC_RANDOMMOVE:
			if (md) {
				md->next_walktime = tick - 1;
				mob->randomwalk(md,tick);
			}
			break;

		case NPC_SPEEDUP:
			{
				// or does it increase casting rate? just a guess xD
				int i = SC_ATTHASTE_POTION1 + skill_lv - 1;
				if (i > SC_ATTHASTE_INFINITY)
					i = SC_ATTHASTE_INFINITY;
				clif->skill_nodamage(src,bl,skill_id,skill_lv,
					sc_start(src,bl,(sc_type)i,100,skill_lv,skill_lv * 60000));
			}
			break;

		case NPC_REVENGE:
			// not really needed... but adding here anyway ^^
			if (md && md->master_id > 0) {
				struct block_list *mbl, *tbl;
				if ((mbl = map->id2bl(md->master_id)) == NULL ||
					(tbl = battle->get_targeted(mbl)) == NULL)
					break;
				md->state.provoke_flag = tbl->id;
				mob->target(md, tbl, sstatus->rhw.range);
			}
			break;

		case NPC_RUN:
			{
				const int mask[8][2] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
				uint8 dir = (bl == src)?unit->getdir(src):map->calc_dir(src,bl->x,bl->y); //If cast on self, run forward, else run away.
				unit->stop_attack(src);
				//Run skillv tiles overriding the can-move check.
				if (unit->walktoxy(src, src->x + skill_lv * mask[dir][0], src->y + skill_lv * mask[dir][1], 2) && md)
					md->state.skillstate = MSS_WALK; //Otherwise it isn't updated in the AI.
			}
			break;

		case NPC_TRANSFORMATION:
		case NPC_METAMORPHOSIS:
			if(md && md->skill_idx >= 0) {
				int class_ = mob->random_class (md->db->skill[md->skill_idx].val,0);
				if (skill_lv > 1) //Multiply the rest of mobs. [Skotlex]
					mob->summonslave(md,md->db->skill[md->skill_idx].val,skill_lv-1,skill_id);
				if (class_) mob->class_change(md, class_);
			}
			break;

		case NPC_EMOTION_ON:
		case NPC_EMOTION:
			//val[0] is the emotion to use.
			//NPC_EMOTION & NPC_EMOTION_ON can change a mob's mode 'permanently' [Skotlex]
			//val[1] 'sets' the mode
			//val[2] adds to the current mode
			//val[3] removes from the current mode
			//val[4] if set, asks to delete the previous mode change.
			if(md && md->skill_idx >= 0 && tsc) {
				clif->emotion(bl, md->db->skill[md->skill_idx].val[0]);
				if(md->db->skill[md->skill_idx].val[4] && tsce)
					status_change_end(bl, type, INVALID_TIMER);

				//If mode gets set by NPC_EMOTION then the target should be reset [Playtester]
				if(skill_id == NPC_EMOTION && md->db->skill[md->skill_idx].val[1])
					mob->unlocktarget(md,tick);

				if(md->db->skill[md->skill_idx].val[1] || md->db->skill[md->skill_idx].val[2])
					sc_start4(src, src, type, 100, skill_lv,
						md->db->skill[md->skill_idx].val[1],
						md->db->skill[md->skill_idx].val[2],
						md->db->skill[md->skill_idx].val[3],
						skill->get_time(skill_id, skill_lv));
			}
			break;

		case NPC_POWERUP:
			sc_start(src,bl,SC_INCATKRATE,100,200,skill->get_time(skill_id, skill_lv));
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,100,skill->get_time(skill_id, skill_lv)));
			break;

		case NPC_AGIUP:
			sc_start(src, bl, SC_MOVHASTE_INFINITY, 100, 100, skill->get_time(skill_id, skill_lv)); // Fix 100% movement speed in all levels. [Frost]
			clif->skill_nodamage(src, bl, skill_id, skill_lv,
				sc_start(src, bl, type, 100, 100, skill->get_time(skill_id, skill_lv)));
			break;

		case NPC_INVISIBLE:
			//Have val4 passed as 6 is for "infinite cloak" (do not end on attack/skill use).
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,0,0,6,skill->get_time(skill_id,skill_lv)));
			break;

		case NPC_SIEGEMODE:
			// not sure what it does
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case WE_MALE:
		{
			int hp_rate = (!skill_lv)? 0:skill->dbs->db[skill_id].hp_rate[skill_lv-1];
			int gain_hp = tstatus->max_hp*abs(hp_rate)/100; // The earned is the same % of the target HP than it cost the caster. [Skotlex]
			clif->skill_nodamage(src,bl,skill_id,status->heal(bl, gain_hp, 0, 0),1);
		}
			break;
		case WE_FEMALE:
		{
			int sp_rate = (!skill_lv)? 0:skill->dbs->db[skill_id].sp_rate[skill_lv-1];
			int gain_sp = tstatus->max_sp*abs(sp_rate)/100;// The earned is the same % of the target SP than it cost the caster. [Skotlex]
			clif->skill_nodamage(src,bl,skill_id,status->heal(bl, 0, gain_sp, 0),1);
		}
			break;

		// parent-baby skills
		case WE_BABY:
			if(sd) {
				struct map_session_data *f_sd = pc->get_father(sd);
				struct map_session_data *m_sd = pc->get_mother(sd);
				bool we_baby_parents = false;
				if(m_sd && check_distance_bl(bl,&m_sd->bl,AREA_SIZE)) {
					sc_start(src,&m_sd->bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
					clif->specialeffect(&m_sd->bl,408,AREA);
					we_baby_parents = true;
				}
				if(f_sd && check_distance_bl(bl,&f_sd->bl,AREA_SIZE)) {
					sc_start(src,&f_sd->bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
					clif->specialeffect(&f_sd->bl,408,AREA);
					we_baby_parents = true;
				}
				if (!we_baby_parents) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					map->freeblock_unlock();
					return 0;
				}
				else
					status->change_start(src,bl,SC_STUN,10000,skill_lv,0,0,0,skill->get_time2(skill_id,skill_lv),SCFLAG_FIXEDRATE);
			}
			break;

		case PF_HPCONVERSION:
		{
			int hp, sp;
			hp = sstatus->max_hp/10;
			sp = hp * 10 * skill_lv / 100;
			if (!status->charge(src,hp,0)) {
				if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			status->heal(bl,0,sp,2);
		}
			break;

		case MA_REMOVETRAP:
		case HT_REMOVETRAP:
			{
				struct skill_unit* su;
				struct skill_unit_group* sg;
				su = BL_CAST(BL_SKILL, bl);

				// Mercenaries can remove any trap
				// Players can only remove their own traps or traps on Vs maps.
				if( su && (sg = su->group) != NULL && (src->type == BL_MER || sg->src_id == src->id || map_flag_vs(bl->m)) && (skill->get_inf2(sg->skill_id)&INF2_TRAP) )
				{
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
					if( sd && !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) && sg->unit_id != UNT_THORNS_TRAP ) {
						// prevent picking up expired traps
						if( battle_config.skill_removetrap_type ) {
							int i;
							// get back all items used to deploy the trap
							for( i = 0; i < 10; i++ ) {
								if( skill->dbs->db[su->group->skill_id].itemid[i] > 0 ) {
									int success;
									struct item item_tmp;
									memset(&item_tmp,0,sizeof(item_tmp));
									item_tmp.nameid = skill->dbs->db[su->group->skill_id].itemid[i];
									item_tmp.identify = 1;
									if (item_tmp.nameid && (success=pc->additem(sd,&item_tmp,skill->dbs->db[su->group->skill_id].amount[i],LOG_TYPE_SKILL)) != 0) {
										clif->additem(sd,0,0,success);
										map->addflooritem(&sd->bl, &item_tmp, skill->dbs->db[su->group->skill_id].amount[i], sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
									}
								}
							}
						} else {
							// get back 1 trap
							struct item item_tmp;
							memset(&item_tmp,0,sizeof(item_tmp));
							item_tmp.nameid = su->group->item_id?su->group->item_id:ITEMID_TRAP;
							item_tmp.identify = 1;
							if (item_tmp.nameid && (flag=pc->additem(sd,&item_tmp,1,LOG_TYPE_SKILL)) != 0) {
								clif->additem(sd,0,0,flag);
								map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
							}
						}
					}
					skill->delunit(su);
				}else if(sd)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);

			}
			break;
		case HT_SPRINGTRAP:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (bl->type == BL_SKILL) {
				struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
				if (su->group != NULL) {
					switch (su->group->unit_id) {
						case UNT_ANKLESNARE:
							if (su->group->val2 != 0)
								// if it is already trapping something don't spring it,
								// remove trap should be used instead
								break;
							// otherwise fall through to below
						case UNT_BLASTMINE:
						case UNT_SKIDTRAP:
						case UNT_LANDMINE:
						case UNT_SHOCKWAVE:
						case UNT_SANDMAN:
						case UNT_FLASHER:
						case UNT_FREEZINGTRAP:
						case UNT_CLAYMORETRAP:
						case UNT_TALKIEBOX:
							su->group->unit_id = UNT_USED_TRAPS;
							clif->changetraplook(bl, UNT_USED_TRAPS);
							su->group->limit=DIFF_TICK32(tick+1500,su->group->tick);
							su->limit=DIFF_TICK32(tick+1500,su->group->tick);
					}
				}
			}
			break;
		case BD_ENCORE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if(sd)
				unit->skilluse_id(src,src->id,sd->skill_id_dance,sd->skill_lv_dance);
			break;

		case AS_SPLASHER:
			if( tstatus->mode&MD_BOSS
#ifndef RENEWAL
			  /** Renewal dropped the 3/4 hp requirement **/
			 || tstatus-> hp > tstatus->max_hp*3/4
#endif // RENEWAL
			) {
				if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				map->freeblock_unlock();
				return 1;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,skill_id,src->id,skill->get_time(skill_id,skill_lv),1000));
		#ifndef RENEWAL
			if (sd) skill->blockpc_start (sd, skill_id, skill->get_time(skill_id, skill_lv)+3000);
		#endif
			break;

		case PF_MINDBREAKER:
			{
				if(tstatus->mode&MD_BOSS || battle->check_undead(tstatus->race,tstatus->def_ele) ) {
					map->freeblock_unlock();
					return 1;
				}

				if (tsce) {
					//HelloKitty2 (?) explained that this silently fails when target is
					//already inflicted. [Skotlex]
					map->freeblock_unlock();
					return 1;
				}

				//Has a 55% + skill_lv*5% success chance.
				if (!clif->skill_nodamage(src,bl,skill_id,skill_lv,
				                          sc_start(src,bl,type,55+5*skill_lv,skill_lv,skill->get_time(skill_id,skill_lv)))
				) {
					if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					map->freeblock_unlock();
					return 0;
				}

				unit->skillcastcancel(bl,0);

				if(tsc && tsc->count){
					status_change_end(bl, SC_FREEZE, INVALID_TIMER);
					if(tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE)
						status_change_end(bl, SC_STONE, INVALID_TIMER);
					status_change_end(bl, SC_SLEEP, INVALID_TIMER);
				}

				if(dstmd)
					mob->target(dstmd,src,skill->get_range2(src,skill_id,skill_lv));
			}
			break;

		case PF_SOULCHANGE:
			{
				unsigned int sp1 = 0, sp2 = 0;
				if (dstmd) {
					if (dstmd->state.soul_change_flag) {
						if(sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						break;
					}
					dstmd->state.soul_change_flag = 1;
					sp2 = sstatus->max_sp * 3 /100;
					status->heal(src, 0, sp2, 2);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					break;
				}
				sp1 = sstatus->sp;
				sp2 = tstatus->sp;
#ifdef RENEWAL
				sp1 = sp1 / 2;
				sp2 = sp2 / 2;
				if( tsc && tsc->data[SC_EXTREMITYFIST2] )
					sp1 = tstatus->sp;
#endif // RENEWAL
				status->set_sp(src, sp2, 3);
				status->set_sp(bl, sp1, 3);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		// Slim Pitcher
		case CR_SLIMPITCHER:
			// Updated to block Slim Pitcher from working on barricades and guardian stones.
			if (dstmd != NULL && (dstmd->class_ == MOBID_EMPELIUM || (dstmd->class_ >= MOBID_BARRICADE && dstmd->class_ <= MOBID_S_EMPEL_2)))
				break;
			if (script->potion_hp || script->potion_sp) {
				int hp = script->potion_hp, sp = script->potion_sp;
				hp = hp * (100 + (tstatus->vit<<1))/100;
				sp = sp * (100 + (tstatus->int_<<1))/100;
				if (dstsd) {
					if (hp)
						hp = hp * (100 + pc->checkskill(dstsd,SM_RECOVERY)*10 + pc->skillheal2_bonus(dstsd, skill_id))/100;
					if (sp)
						sp = sp * (100 + pc->checkskill(dstsd,MG_SRECOVERY)*10 + pc->skillheal2_bonus(dstsd, skill_id))/100;
				}
				if( tsc && tsc->count ) {
					if (tsc->data[SC_CRITICALWOUND]) {
						hp -= hp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
						sp -= sp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
					}
					if (tsc->data[SC_DEATHHURT]) {
						hp -= hp * 20 / 100;
						sp -= sp * 20 / 100;
					}
					if( tsc->data[SC_WATER_INSIGNIA] && tsc->data[SC_WATER_INSIGNIA]->val1 == 2) {
						hp += hp / 10;
						sp += sp / 10;
					}
				}
				if(hp > 0)
					clif->skill_nodamage(NULL,bl,AL_HEAL,hp,1);
				if(sp > 0)
					clif->skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
				status->heal(bl,hp,sp,0);
			}
			break;
		// Full Chemical Protection
		case CR_FULLPROTECTION:
		{
			unsigned int equip[] = { EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP };
			int i, s = 0, skilltime = skill->get_time(skill_id, skill_lv);
			for ( i = 0; i < 4; i++ ) {
				if ( bl->type != BL_PC || (dstsd && pc->checkequip(dstsd, equip[i]) < 0) )
					continue;
				if ( dstsd && equip[i] == EQP_SHIELD ) {
					short index = dstsd->equip_index[EQI_HAND_L];
					if ( index >= 0 && dstsd->inventory_data[index] && dstsd->inventory_data[index]->type != IT_ARMOR )
						continue;
				}
				sc_start(src, bl, (sc_type)(SC_PROTECTWEAPON + i), 100, skill_lv, skilltime);
				s++;
			}
			if ( sd && !s ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
				map->freeblock_unlock(); // Don't consume item requirements
				return 0;
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
		}
		break;

		case RG_CLEANER: //AppleGirl
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case CG_LONGINGFREEDOM:
			{
				if (tsc && !tsce && (tsce=tsc->data[SC_DANCING]) != NULL && tsce->val4
					&& (tsce->val1&0xFFFF) != CG_MOONLIT) //Can't use Longing for Freedom while under Moonlight Petals. [Skotlex]
				{
					clif->skill_nodamage(src,bl,skill_id,skill_lv,
						sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
				}
			}
			break;

		case CG_TAROTCARD:
			{
				int count = -1;
				if (tsc && tsc->data[type]) {
					map->freeblock_unlock();
					return 0;
				}
				if (rnd() % 100 > skill_lv * 8 || (dstmd && ((dstmd->guardian_data && dstmd->class_ == MOBID_EMPELIUM) || mob_is_battleground(dstmd)))) {
					if (sd != NULL)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);

					map->freeblock_unlock();
					return 0;
				}
				status_zap(src,0,skill->dbs->db[skill->get_index(skill_id)].sp[skill_lv]); // consume sp only if succeeded [Inkfish]
				do {
					int eff = rnd() % 14;
					if( eff == 5 )
						clif->specialeffect(src, 528, AREA);
					else
						clif->specialeffect(bl, 523 + eff, AREA);
					switch (eff)
					{
					case 0: // heals SP to 0
						status_percent_damage(src, bl, 0, 100, false);
						break;
					case 1: // matk halved
						sc_start(src,bl,SC_INCMATKRATE,100,-50,skill->get_time2(skill_id,skill_lv));
						break;
					case 2: // all buffs removed
						status->change_clear_buffs(bl,1);
						break;
					case 3: // 1000 damage, random armor destroyed
						{
							status_fix_damage(src, bl, 1000, 0);
							clif->damage(src,bl,0,0,1000,0,BDT_NORMAL,0);
							if( !status->isdead(bl) ) {
								int where[] = { EQP_ARMOR, EQP_SHIELD, EQP_HELM, EQP_SHOES, EQP_GARMENT };
								skill->break_equip(bl, where[rnd()%5], 10000, BCT_ENEMY);
							}
						}
						break;
					case 4: // atk halved
						sc_start(src,bl,SC_INCATKRATE,100,-50,skill->get_time2(skill_id,skill_lv));
						break;
					case 5: // 2000HP heal, random teleported
						status->heal(src, 2000, 0, 0);
						if( !map_flag_vs(bl->m) )
							unit->warp(bl, -1,-1,-1, CLR_TELEPORT);
						break;
					case 6: // random 2 other effects
						if (count == -1)
							count = 3;
						else
							count++; //Should not re-trigger this one.
						break;
					case 7: // stop freeze or stoned
						{
							enum sc_type sc[] = { SC_STOP, SC_FREEZE, SC_STONE };
							sc_start(src,bl,sc[rnd()%3],100,skill_lv,skill->get_time2(skill_id,skill_lv));
						}
						break;
					case 8: // curse coma and poison
						sc_start(src,bl,SC_COMA,100,skill_lv,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_CURSE,100,skill_lv,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_POISON,100,skill_lv,skill->get_time2(skill_id,skill_lv));
						break;
					case 9: // confusion
						sc_start(src,bl,SC_CONFUSION,100,skill_lv,skill->get_time2(skill_id,skill_lv));
						break;
					case 10: // 6666 damage, atk matk halved, cursed
						status_fix_damage(src, bl, 6666, 0);
						clif->damage(src,bl,0,0,6666,0,BDT_NORMAL,0);
						sc_start(src,bl,SC_INCATKRATE,100,-50,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_INCMATKRATE,100,-50,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_CURSE,skill_lv,100,skill->get_time2(skill_id,skill_lv));
						break;
					case 11: // 4444 damage
						status_fix_damage(src, bl, 4444, 0);
						clif->damage(src,bl,0,0,4444,0,BDT_NORMAL,0);
						break;
					case 12: // stun
						sc_start(src,bl,SC_STUN,100,skill_lv,5000);
						break;
					case 13: // atk,matk,hit,flee,def reduced
						sc_start(src,bl,SC_INCATKRATE,100,-20,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_INCMATKRATE,100,-20,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_INCHITRATE,100,-20,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_INCFLEERATE,100,-20,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,SC_INCDEFRATE,100,-20,skill->get_time2(skill_id,skill_lv));
						sc_start(src,bl,type,100,skill_lv,skill->get_time2(skill_id,skill_lv));
						break;
					default:
						break;
					}
				} while ((--count) > 0);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case SL_ALCHEMIST:
		case SL_ASSASIN:
		case SL_BARDDANCER:
		case SL_BLACKSMITH:
		case SL_CRUSADER:
		case SL_HUNTER:
		case SL_KNIGHT:
		case SL_MONK:
		case SL_PRIEST:
		case SL_ROGUE:
		case SL_SAGE:
		case SL_SOULLINKER:
		case SL_STAR:
		case SL_SUPERNOVICE:
		case SL_WIZARD:
			//NOTE: here, 'type' has the value of the associated MAPID, not of the SC_SOULLINK constant.
			if (sd && !(dstsd && (dstsd->class_&MAPID_UPPERMASK) == type)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if (skill_id == SL_SUPERNOVICE && dstsd && dstsd->die_counter && !(rnd()%100)) {
				//Erase death count 1% of the casts
				dstsd->die_counter = 0;
				pc_setglobalreg(dstsd,script->add_str("PC_DIE_COUNTER"), 0);
				clif->specialeffect(bl, 0x152, AREA);
				//SC_SOULLINK invokes status_calc_pc for us.
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,SC_SOULLINK,100,skill_lv,skill_id,0,0,skill->get_time(skill_id,skill_lv)));
			sc_start(src,src,SC_SMA_READY,100,skill_lv,skill->get_time(SL_SMA,skill_lv));
			break;
		case SL_HIGH:
			if (sd && !(dstsd && (dstsd->class_&JOBL_UPPER) && !(dstsd->class_&JOBL_2) && dstsd->status.base_level < 70)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,skill_id,0,0,skill->get_time(skill_id,skill_lv)));
			sc_start(src,src,SC_SMA_READY,100,skill_lv,skill->get_time(SL_SMA,skill_lv));
			break;

		case SL_SWOO:
			if (tsce) {
				if(sd)
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				status->change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,10000,SCFLAG_FIXEDRATE);
				status_change_end(bl, SC_SWOO, INVALID_TIMER);
				break;
			}
		case SL_SKA: // [marquis007]
		case SL_SKE:
			if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				status->change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
				break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			if (skill_id == SL_SKE)
				sc_start(src,src,SC_SMA_READY,100,skill_lv,skill->get_time(SL_SMA,skill_lv));
			break;

		// New guild skills [Celest]
		case GD_BATTLEORDER:
			if(flag&1) {
				if (status->get_guild_id(src) == status->get_guild_id(bl))
					sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id, skill_lv));
			} else if (status->get_guild_id(src)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->area_sub, src,
				                    skill->get_splash(skill_id, skill_lv), BL_PC,
				                    src,skill_id,skill_lv,tick, flag|BCT_GUILD|1,
				                    skill->castend_nodamage_id);
				if (sd)
					guild->block_skill(sd,skill->get_time2(skill_id,skill_lv));
			}
			break;
		case GD_REGENERATION:
			if(flag&1) {
				if (status->get_guild_id(src) == status->get_guild_id(bl))
					sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id, skill_lv));
			} else if (status->get_guild_id(src)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->area_sub, src,
				                    skill->get_splash(skill_id, skill_lv), BL_PC,
				                    src,skill_id,skill_lv,tick, flag|BCT_GUILD|1,
				                    skill->castend_nodamage_id);
				if (sd)
					guild->block_skill(sd,skill->get_time2(skill_id,skill_lv));
			}
			break;
		case GD_RESTORE:
			if(flag&1) {
				if (status->get_guild_id(src) == status->get_guild_id(bl))
					clif->skill_nodamage(src,bl,AL_HEAL,status_percent_heal(bl,90,90),1);
			} else if (status->get_guild_id(src)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->area_sub, src,
				                    skill->get_splash(skill_id, skill_lv), BL_PC,
				                    src,skill_id,skill_lv,tick, flag|BCT_GUILD|1,
				                    skill->castend_nodamage_id);
				if (sd)
					guild->block_skill(sd,skill->get_time2(skill_id,skill_lv));
			}
			break;
		case GD_EMERGENCYCALL:
			{
				int dx[9]={-1, 1, 0, 0,-1, 1,-1, 1, 0};
				int dy[9]={ 0, 0, 1,-1, 1,-1,-1, 1, 0};
				int i, j = 0;
				struct guild *g;
				// i don't know if it actually summons in a circle, but oh well. ;P
				g = sd ? sd->guild : guild->search(status->get_guild_id(src));
				if (!g)
					break;
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				for(i = 0; i < g->max_member; i++, j++) {
					if (j>8) j=0;
					if ((dstsd = g->member[i].sd) != NULL && sd != dstsd && !dstsd->state.autotrade && !pc_isdead(dstsd)) {
						if (map->list[dstsd->bl.m].flag.nowarp && !map_flag_gvg2(dstsd->bl.m))
							continue;
						if (map->getcell(src->m, src, src->x + dx[j], src->y + dy[j], CELL_CHKNOREACH))
							dx[j] = dy[j] = 0;
						pc->setpos(dstsd, map_id2index(src->m), src->x+dx[j], src->y+dy[j], CLR_RESPAWN);
					}
				}
				if (sd)
					guild->block_skill(sd,skill->get_time2(skill_id,skill_lv));
			}
			break;

		case SG_FEEL:
			//AuronX reported you CAN memorize the same map as all three. [Skotlex]
			if (sd) {
				if(!sd->feel_map[skill_lv-1].index)
					clif->feel_req(sd->fd,sd, skill_lv);
				else
					clif->feel_info(sd, skill_lv-1, 1);
			}
			break;

		case SG_HATE:
			if (sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if (!pc->set_hate_mob(sd, skill_lv-1, bl))
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case GS_GLITTERING:
			if(sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if(rnd()%100 < (20+10*skill_lv))
					pc->addspiritball(sd,skill->get_time(skill_id,skill_lv),10);
				else if(sd->spiritball > 0)
					pc->delspiritball(sd,1,0);
			}
			break;

		case GS_CRACKER:
			/* per official standards, this skill works on players and mobs. */
			if (sd && (dstsd || dstmd))
			{
				int rate = 65 -5*distance_bl(src,bl); //Base rate
				if (rate < 30) rate = 30;
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				sc_start(src,bl,SC_STUN, rate,skill_lv,skill->get_time2(skill_id,skill_lv));
			}
			break;

		case AM_CALLHOMUN: // [orn]
			if( sd ) {
				if (homun->call(sd))
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				else
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case AM_REST:
			if (sd) {
				if (homun->vaporize(sd,HOM_ST_REST))
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				else
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case HAMI_CASTLE: // [orn]
			if(rnd()%100 < 20*skill_lv && src != bl)
			{
				int x,y;
				x = src->x;
				y = src->y;
				if (hd)
					skill->blockhomun_start(hd, skill_id, skill->get_time2(skill_id,skill_lv));

				if (unit->movepos(src,bl->x,bl->y,0,0)) {
					clif->skill_nodamage(src,src,skill_id,skill_lv,1); // Homun
					clif->slide(src,bl->x,bl->y) ;
					if (unit->movepos(bl,x,y,0,0))
					{
						clif->skill_nodamage(bl,bl,skill_id,skill_lv,1); // Master
						clif->slide(bl,x,y) ;
					}

					//TODO: Shouldn't also players and the like switch targets?
					map->foreachinrange(skill->chastle_mob_changetarget,src,
					                    AREA_SIZE, BL_MOB, bl, src);
				}
			}
			// Failed
			else if (hd && hd->master)
				clif->skill_fail(hd->master, skill_id, USESKILL_FAIL_LEVEL, 0);
			else if (sd)
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
			break;
		case HVAN_CHAOTIC: // [orn]
			{
				static const int per[5][2]={{20,50},{50,60},{25,75},{60,64},{34,67}};
				int r = rnd()%100;
				int target = (skill_lv-1)%5;
				int hp;
				if(r<per[target][0]) //Self
					bl = src;
				else if(r<per[target][1]) //Master
					bl = battle->get_master(src);
				else //Enemy
					bl = map->id2bl(battle->get_target(src));

				if (!bl) bl = src;
				hp = skill->calc_heal(src, bl, skill_id, 1+rnd()%skill_lv, true);
				//Eh? why double skill packet?
				clif->skill_nodamage(src,bl,AL_HEAL,hp,1);
				clif->skill_nodamage(src,bl,skill_id,hp,1);
				status->heal(bl, hp, 0, 0);
			}
			break;
		// Homun single-target support skills [orn]
		case HAMI_BLOODLUST:
		case HFLI_FLEET:
		case HFLI_SPEED:
		case HLIF_CHANGE:
		case MH_ANGRIFFS_MODUS:
		case MH_GOLDENE_FERSE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			if (hd)
				skill->blockhomun_start(hd, skill_id, skill->get_time2(skill_id,skill_lv));
			break;

		case NPC_DRAGONFEAR:
			if (flag&1) {
				const enum sc_type sc[] = { SC_STUN, SC_SILENCE, SC_CONFUSION, SC_BLOODING };
				int i, j;
				j = i = rnd()%ARRAYLENGTH(sc);
				while ( !sc_start2(src,bl,sc[i],100,skill_lv,src->id,skill->get_time2(skill_id,i+1)) ) {
					i++;
					if ( i == ARRAYLENGTH(sc) )
						i = 0;
					if (i == j)
						break;
				}
				break;
			}
		case NPC_WIDEBLEEDING:
		case NPC_WIDECONFUSE:
		case NPC_WIDECURSE:
		case NPC_WIDEFREEZE:
		case NPC_WIDESLEEP:
		case NPC_WIDESILENCE:
		case NPC_WIDESTONE:
		case NPC_WIDESTUN:
		case NPC_SLOWCAST:
		case NPC_WIDEHELLDIGNITY:
		case NPC_WIDEHEALTHFEAR:
		case NPC_WIDEBODYBURNNING:
		case NPC_WIDEFROSTMISTY:
		case NPC_WIDECOLD:
		case NPC_WIDE_DEEP_SLEEP:
		case NPC_WIDESIREN:
			if (flag&1){
				switch( type ){
					case SC_BURNING:
						sc_start4(src,bl,type,100,skill_lv,0,src->id,0,skill->get_time2(skill_id,skill_lv));
						break;
					case SC_SIREN:
						sc_start2(src,bl,type,100,skill_lv,src->id,skill->get_time2(skill_id,skill_lv));
						break;
					default:
						sc_start2(src,bl,type,100,skill_lv,src->id,skill->get_time2(skill_id,skill_lv));
				}
			} else {
				skill->area_temp[2] = 0; //For SD_PREAMBLE
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->area_sub, bl,
				                    skill->get_splash(skill_id, skill_lv),BL_CHAR,
				                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
				                    skill->castend_nodamage_id);
			}
			break;
		case NPC_WIDESOULDRAIN:
			if (flag&1)
				status_percent_damage(src,bl,0,((skill_lv-1)%5+1)*20,false);
			else {
				skill->area_temp[2] = 0; //For SD_PREAMBLE
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->area_sub, bl,
				                    skill->get_splash(skill_id, skill_lv),BL_CHAR,
				                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
				                    skill->castend_nodamage_id);
			}
			break;
		case ALL_PARTYFLEE:
			if( sd  && !(flag&1) )
			{
				if( !sd->status.party_id )
				{
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			}
			else
				clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;
		case NPC_TALK:
		case ALL_WEWISH:
		case ALL_CATCRY:
		case ALL_DREAM_SUMMERNIGHT:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;
		case ALL_BUYING_STORE:
			if( sd )
			{// players only, skill allows 5 buying slots
				clif->skill_nodamage(src, bl, skill_id, skill_lv, buyingstore->setup(sd, MAX_BUYINGSTORE_SLOTS));
			}
			break;
		case RK_ENCHANTBLADE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,// formula not confirmed
				sc_start2(src,bl,type,100,skill_lv,(100+20*skill_lv)*status->get_lv(src)/150+sstatus->int_,skill->get_time(skill_id,skill_lv)));
			break;
		case RK_DRAGONHOWLING:
			if( flag&1)
				sc_start(src,bl,type,50 + 6 * skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
			else {
				skill->area_temp[2] = 0;
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				map->foreachinrange(skill->area_sub, src,
				                    skill->get_splash(skill_id,skill_lv),BL_CHAR,
				                    src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SD_PREAMBLE|1,
				                    skill->castend_nodamage_id);
			}
			break;
		case RK_IGNITIONBREAK:
		case LG_EARTHDRIVE:
			{
				int splash;
				clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				splash = skill->get_splash(skill_id,skill_lv);
				if( skill_id == LG_EARTHDRIVE ) {
					int dummy = 1;
					map->foreachinarea(skill->cell_overlap, src->m, src->x-splash, src->y-splash, src->x+splash, src->y+splash, BL_SKILL, LG_EARTHDRIVE, &dummy, src);
				}
				map->foreachinrange(skill->area_sub, bl,splash,BL_CHAR,
				                    src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			}
			break;
		case RK_STONEHARDSKIN:
			if( sd ) {
				int heal = sstatus->hp / 5; // 20% HP
				if( status->charge(bl,heal,0) )
					clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start2(src,bl,type,100,skill_lv,heal,skill->get_time(skill_id,skill_lv)));
				else
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;
		case RK_REFRESH:
		{
			int heal = status_get_max_hp(bl) * 25 / 100;
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
			                     sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			status->heal(bl,heal,0,1);
			status->change_clear_buffs(bl,4);
		}
			break;

		case RK_MILLENNIUMSHIELD:
			if( sd && pc->checkskill(sd,RK_RUNEMASTERY) >= 9 ) {
				short chance = 0;
				short num_shields = 0;
				chance = rnd()%100 + 1;//Generates a random number between 1 - 100 which is then used to determine how many shields will generate.
				if ( chance >= 1 && chance <= 20 )//20% chance for 4 shields.
					num_shields = 4;
				else if ( chance >= 21 && chance <= 50 )//30% chance for 3 shields.
					num_shields = 3;
				else if ( chance >= 51 && chance <= 100 )//50% chance for 2 shields.
					num_shields = 2;
				sc_start4(src,bl,type,100,skill_lv,num_shields,1000,0,skill->get_time(skill_id,skill_lv));
				clif->millenniumshield(src,num_shields);
				clif->skill_nodamage(src,bl,skill_id,1,1);
			}
			break;

		case RK_FIGHTINGSPIRIT:
			if( flag&1 ) {
				int atkbonus = 7 * party->foreachsamemap(skill->area_sub,sd,skill->get_splash(skill_id,skill_lv),src,skill_id,skill_lv,tick,BCT_PARTY,skill->area_sub_count);
				if( src == bl )
					sc_start2(src,bl,type,100,atkbonus,10*(sd?pc->checkskill(sd,RK_RUNEMASTERY):10),skill->get_time(skill_id,skill_lv));
				else
					sc_start(src,bl,type,100,atkbonus / 4,skill->get_time(skill_id,skill_lv));
			} else if( sd && pc->checkskill(sd,RK_RUNEMASTERY) >= 5 ) {
				if( sd->status.party_id )
					party->foreachsamemap(skill->area_sub,sd,skill->get_splash(skill_id,skill_lv),src,skill_id,skill_lv,tick,flag|BCT_PARTY|1,skill->castend_nodamage_id);
				else
					sc_start2(src,bl,type,100,7,10*(sd?pc->checkskill(sd,RK_RUNEMASTERY):10),skill->get_time(skill_id,skill_lv));
				clif->skill_nodamage(src,bl,skill_id,1,1);
			}
			break;

		case RK_LUXANIMA:
			if( sd == NULL || sd->status.party_id == 0 || flag&1 ){
				if( src == bl )
					break;
				while( skill->area_temp[5] >= 0x10 ){
					int value = 0;
					type = SC_NONE;
					if( skill->area_temp[5]&0x10 ){
						value = (rnd()%100<50) ? 4 : ((rnd()%100<80) ? 3 : 2);
						clif->millenniumshield(bl,value);
						skill->area_temp[5] &= ~0x10;
						type = SC_MILLENNIUMSHIELD;
					}else if( skill->area_temp[5]&0x20 ){
						value = status_get_max_hp(bl) * 25 / 100;
						status->change_clear_buffs(bl,4);
						skill->area_temp[5] &= ~0x20;
						status->heal(bl,value,0,1);
						type = SC_REFRESH;
					}else if( skill->area_temp[5]&0x40 ){
						skill->area_temp[5] &= ~0x40;
						type = SC_GIANTGROWTH;
					}else if( skill->area_temp[5]&0x80 ){
						if( dstsd ){
							value = sstatus->hp / 4;
							if( status->charge(bl,value,0) )
								type = SC_STONEHARDSKIN;
							skill->area_temp[5] &= ~0x80;
						}
					}else if( skill->area_temp[5]&0x100 ){
						skill->area_temp[5] &= ~0x100;
						type = SC_VITALITYACTIVATION;
					}else if( skill->area_temp[5]&0x200 ){
						skill->area_temp[5] &= ~0x200;
						type = SC_ABUNDANCE;
					}
					if( type > SC_NONE )
						clif->skill_nodamage(bl, bl, skill_id, skill_lv,
							sc_start4(src,bl, type, 100, skill_lv, value, 0, 1, skill->get_time(skill_id, skill_lv)));
				}
			}else if( sd ){
				if( tsc && tsc->count ){
					if(tsc->data[SC_MILLENNIUMSHIELD])
						skill->area_temp[5] |= 0x10;
					if(tsc->data[SC_REFRESH])
						skill->area_temp[5] |= 0x20;
					if(tsc->data[SC_GIANTGROWTH])
						skill->area_temp[5] |= 0x40;
					if(tsc->data[SC_STONEHARDSKIN])
						skill->area_temp[5] |= 0x80;
					if(tsc->data[SC_VITALITYACTIVATION])
						skill->area_temp[5] |= 0x100;
					if(tsc->data[SC_ABUNDANCE])
						skill->area_temp[5] |= 0x200;
				}
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			}
			break;
		/**
		 * Guilotine Cross
		 **/
		case GC_ROLLINGCUTTER:
			{
				short count = 1;
				skill->area_temp[2] = 0;
				map->foreachinrange(skill->area_sub,src,skill->get_splash(skill_id,skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SD_PREAMBLE|SD_SPLASH|1,skill->castend_damage_id);
				if( tsc && tsc->data[SC_ROLLINGCUTTER] )
				{ // Every time the skill is casted the status change is reseted adding a counter.
					count += (short)tsc->data[SC_ROLLINGCUTTER]->val1;
					if( count > 10 )
						count = 10; // Max counter
					status_change_end(bl, SC_ROLLINGCUTTER, INVALID_TIMER);
				}
				sc_start(src,bl,SC_ROLLINGCUTTER,100,count,skill->get_time(skill_id,skill_lv));
				clif->skill_nodamage(src,src,skill_id,skill_lv,1);
			}
			break;

		case GC_WEAPONBLOCKING:
			if( tsc && tsc->data[SC_WEAPONBLOCKING] )
				status_change_end(bl, SC_WEAPONBLOCKING, INVALID_TIMER);
			else
				sc_start(src,bl,SC_WEAPONBLOCKING,100,skill_lv,skill->get_time(skill_id,skill_lv));
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case GC_CREATENEWPOISON:
			if( sd )
			{
				clif->skill_produce_mix_list(sd,skill_id,25);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;

		case GC_POISONINGWEAPON:
			if( sd ) {
				clif->poison_list(sd,skill_lv);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case GC_ANTIDOTE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if( tsc )
			{
				status_change_end(bl, SC_PARALYSE, INVALID_TIMER);
				status_change_end(bl, SC_PYREXIA, INVALID_TIMER);
				status_change_end(bl, SC_DEATHHURT, INVALID_TIMER);
				status_change_end(bl, SC_LEECHESEND, INVALID_TIMER);
				status_change_end(bl, SC_VENOMBLEED, INVALID_TIMER);
				status_change_end(bl, SC_MAGICMUSHROOM, INVALID_TIMER);
				status_change_end(bl, SC_TOXIN, INVALID_TIMER);
				status_change_end(bl, SC_OBLIVIONCURSE, INVALID_TIMER);
			}
			break;

		case GC_PHANTOMMENACE:
		{
			int r;
			clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinrange(skill->area_sub,src,skill->get_splash(skill_id,skill_lv),BL_CHAR,
				src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			map->foreachinarea( status->change_timer_sub,
				src->m, src->x-r, src->y-r, src->x+r, src->y+r, BL_CHAR, src, NULL, SC_SIGHT, tick);
		}
			break;
		case GC_HALLUCINATIONWALK:
			{
				int heal = status_get_max_hp(bl) * ( 18 - 2 * skill_lv ) / 100;
				if( status_get_hp(bl) < heal ) { // if you haven't enough HP skill fails.
					if( sd ) clif->skill_fail(sd,skill_id,USESKILL_FAIL_HP_INSUFFICIENT,0);
					break;
				}
				if( !status->charge(bl,heal,0) ) {
					if( sd ) clif->skill_fail(sd,skill_id,USESKILL_FAIL_HP_INSUFFICIENT,0);
					break;
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			}
			break;
		/**
		 * Arch Bishop
		 **/
		case AB_ANCILLA:
			if( sd ) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				skill->produce_mix(sd, skill_id, ITEMID_ANCILLA, 0, 0, 0, 1);
			}
			break;

		case AB_CLEMENTIA:
		case AB_CANTO:
		{
			int level = 0;
			if( sd )
				level = skill_id == AB_CLEMENTIA ? pc->checkskill(sd,AL_BLESSING) : pc->checkskill(sd,AL_INCAGI);
			if( sd == NULL || sd->status.party_id == 0 || flag&1 )
				clif->skill_nodamage(bl, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, level + (sd?(sd->status.job_level / 10):0), skill->get_time(skill_id,skill_lv)));
			else if( sd ) {
				if( !level )
					clif->skill_fail(sd,skill_id,USESKILL_FAIL,0);
				else
					party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			}
		}
			break;

		case AB_PRAEFATIO:
			if( (flag&1) || sd == NULL || sd->status.party_id == 0 ) {
				int count = 1;

				if( dstsd && dstsd->special_state.no_magic_damage )
					break;

				if( sd && sd->status.party_id != 0 )
						count = party->foreachsamemap(party->sub_count, sd, 0);

				clif->skill_nodamage(bl, bl, skill_id, skill_lv,
					sc_start4(src, bl, type, 100, skill_lv, 0, 0, count, skill->get_time(skill_id, skill_lv)));
			} else if( sd )
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			break;
		case AB_CHEAL:
			if( sd == NULL || sd->status.party_id == 0 || flag&1 ) {
				if (sd && tstatus && !battle->check_undead(tstatus->race, tstatus->def_ele) && !(tsc && tsc->data[SC_BERSERK])) {
					int lv = pc->checkskill(sd, AL_HEAL);
					int heal = skill->calc_heal(src, bl, AL_HEAL, lv, true);

					if( sd->status.party_id ) {
						int partycount = party->foreachsamemap(party->sub_count, sd, 0);
						if (partycount > 1)
							heal += ((heal / 100) * (partycount * 10) / 4);
					}
					if( status->isimmune(bl) || (dstsd && pc_ismadogear(dstsd)) )
						heal = 0;

					clif->skill_nodamage(bl, bl, skill_id, heal, 1);
					if( tsc && tsc->data[SC_AKAITSUKI] && heal )
						heal = ~heal + 1;
					status->heal(bl, heal, 0, 1);
				}
			} else if( sd )
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			break;
		case AB_ORATIO:
			if( flag&1 )
				sc_start(src, bl, type, 40 + 5 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			else {
				map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR,
					src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;

		case AB_LAUDAAGNUS:
			if( (flag&1 || sd == NULL) || !sd->status.party_id) {
				if( tsc && (tsc->data[SC_FREEZE] || tsc->data[SC_STONE] || tsc->data[SC_BLIND] ||
					tsc->data[SC_BURNING] || tsc->data[SC_FROSTMISTY] || tsc->data[SC_COLD])) {
					// Success Chance: (40 + 10 * Skill Level) %
					if( rnd()%100 > 40+10*skill_lv ) break;
					status_change_end(bl, SC_FREEZE, INVALID_TIMER);
					status_change_end(bl, SC_STONE, INVALID_TIMER);
					status_change_end(bl, SC_BLIND, INVALID_TIMER);
					status_change_end(bl, SC_BURNING, INVALID_TIMER);
					status_change_end(bl, SC_FROSTMISTY, INVALID_TIMER);
					status_change_end(bl, SC_COLD, INVALID_TIMER);
				}else //Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
					clif->skill_nodamage(bl, bl, skill_id, skill_lv,
						sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			} else if( sd )
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv),
					src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			break;

		case AB_LAUDARAMUS:
			if( (flag&1 || sd == NULL) || !sd->status.party_id ) {
				if( tsc && (tsc->data[SC_SLEEP] || tsc->data[SC_STUN] || tsc->data[SC_MANDRAGORA] ||
					tsc->data[SC_SILENCE] || tsc->data[SC_DEEP_SLEEP]) ){
					// Success Chance: (40 + 10 * Skill Level) %
					if( rnd()%100 > 40+10*skill_lv )  break;
					status_change_end(bl, SC_SLEEP, INVALID_TIMER);
					status_change_end(bl, SC_STUN, INVALID_TIMER);
					status_change_end(bl, SC_MANDRAGORA, INVALID_TIMER);
					status_change_end(bl, SC_SILENCE, INVALID_TIMER);
					status_change_end(bl, SC_DEEP_SLEEP, INVALID_TIMER);
				}else // Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
					clif->skill_nodamage(bl, bl, skill_id, skill_lv,
						sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			} else if( sd )
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv),
					src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			break;

		case AB_CLEARANCE:
		{
			int splash;
			if( flag&1 || (splash = skill->get_splash(skill_id, skill_lv)) < 1 ) {
				int i;
				//As of the behavior in official server Clearance is just a super version of Dispell skill. [Jobbie]
				if( bl->type != BL_MOB && battle->check_target(src,bl,BCT_PARTY) <= 0 && sd ) // Only affect mob, party or self.
					break;

				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);

				if((dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER) || rnd()%100 >= 60 + 8 * skill_lv) {
					if (sd)
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				if(status->isimmune(bl) || !tsc || !tsc->count)
					break;
				for(i = 0; i < SC_MAX; i++) {
					if ( !tsc->data[i] )
						continue;
					if (status->get_sc_type(i)&SC_NO_CLEARANCE)
						continue;
					switch (i) {
						case SC_ASSUMPTIO:
							if( bl->type == BL_MOB )
								continue;
							break;
						case SC_BERSERK:
						case SC_SATURDAY_NIGHT_FEVER:
							tsc->data[i]->val2=0;  //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
							break;
					}
					status_change_end(bl,(sc_type)i,INVALID_TIMER);
				}
				break;
			} else {
				map->foreachinrange(skill->area_sub, bl, splash, BL_CHAR, src, skill_id, skill_lv, tick, flag|1, skill->castend_damage_id);
			}
		}
			break;

		case AB_SILENTIUM:
			// Should the level of Lex Divina be equivalent to the level of Silentium or should the highest level learned be used? [LimitLine]
			map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR,
				src, PR_LEXDIVINA, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			break;
		/**
		 * Warlock
		 **/
		case WL_STASIS:
			if( flag&1 )
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
			else {
				map->foreachinrange(skill->area_sub,src,skill->get_splash(skill_id, skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,(map_flag_vs(src->m)?BCT_ALL:BCT_ENEMY|BCT_SELF)|flag|1,skill->castend_nodamage_id);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;

		case WL_WHITEIMPRISON:
			if( (src == bl || battle->check_target(src, bl, BCT_ENEMY) > 0 ) && !is_boss(bl) )// Should not work with bosses.
			{
				int rate = ( sd? sd->status.job_level : 50 ) / 4;

				if( src == bl ) rate = 100; // Success Chance: On self, 100%
				else if(bl->type == BL_PC) rate += 20 + 10 * skill_lv; // On Players, (20 + 10 * Skill Level) %
				else rate += 40 + 10 * skill_lv; // On Monsters, (40 + 10 * Skill Level) %

				if( sd )
					skill->blockpc_start(sd,skill_id,4000);

				if( !(tsc && tsc->data[type]) ){
					int failure = sc_start2(src,bl,type,rate,skill_lv,src->id,(src == bl)?5000:(bl->type == BL_PC)?skill->get_time(skill_id,skill_lv):skill->get_time2(skill_id, skill_lv));
					clif->skill_nodamage(src,bl,skill_id,skill_lv,failure);
					if( sd && !failure )
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				}
			}else
			if( sd )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_TOTARGET,0);
			break;

		case WL_FROSTMISTY:
			if( tsc && (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)))
				break; // Doesn't hit/cause Freezing to invisible enemy // Really? [Rytech]
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			map->foreachinrange(skill->area_sub,bl,skill->get_splash(skill_id,skill_lv),BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY,skill->castend_damage_id);
			break;

		case WL_JACKFROST:
			if( tsc && (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)))
				break; // Do not hit invisible enemy
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			map->foreachinshootrange(skill->area_sub,bl,skill->get_splash(skill_id,skill_lv),BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			break;

		case WL_MARSHOFABYSS:
			clif->skill_nodamage(src, bl, skill_id, skill_lv,
				sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;

		case WL_SIENNAEXECRATE:
			if( flag&1 ) {
				if( status->isimmune(bl) || !tsc )
					break;
				if( tsc && tsc->data[SC_STONE] )
					status_change_end(bl,SC_STONE,INVALID_TIMER);
				else
					status->change_start(src,bl,SC_STONE,10000,skill_lv,0,0,500,skill->get_time(skill_id, skill_lv),SCFLAG_FIXEDTICK);
			} else {
				int rate = 45 + 5 * skill_lv;
				if( rnd()%100 < rate ){
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
					map->foreachinrange(skill->area_sub,bl,skill->get_splash(skill_id,skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_nodamage_id);
				}else if( sd ) // Failure on Rate
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			}
			break;

		case WL_SUMMONFB:
		case WL_SUMMONBL:
		case WL_SUMMONWB:
		case WL_SUMMONSTONE:
		{
			int i;
			for( i = SC_SUMMON1; i <= SC_SUMMON5; i++ ){
				if( tsc && !tsc->data[i] ){ // officially it doesn't work like a stack
					int ele = WLS_FIRE + (skill_id - WL_SUMMONFB) - (skill_id == WL_SUMMONSTONE ? 4 : 0);
					clif->skill_nodamage(src, bl, skill_id, skill_lv,
						sc_start(src, bl, (sc_type)i, 100, ele, skill->get_time(skill_id, skill_lv)));
					break;
				}
			}
		}
			break;

		case WL_READING_SB:
			if( sd ) {
				struct status_change *sc = status->get_sc(bl);
				int i;

				for( i = SC_SPELLBOOK1; i <= SC_SPELLBOOK7; i++)
					if( sc && !sc->data[i] )
						break;
				if( i == SC_SPELLBOOK7 ) {
					clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING, 0);
					break;
				}

				sc_start(src, bl, SC_STOP, 100, skill_lv, INFINITE_DURATION); //Can't move while selecting a spellbook.
				clif->spellbook_list(sd);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;
		/**
		 * Ranger
		 **/
		case RA_FEARBREEZE:
			clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;

		case RA_WUGMASTERY:
			if( sd ) {
				if( !pc_iswug(sd) )
					pc->setoption(sd,sd->sc.option|OPTION_WUG);
				else
					pc->setoption(sd,sd->sc.option&~OPTION_WUG);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case RA_WUGRIDER:
			if( sd ) {
				if( !pc_isridingwug(sd) && pc_iswug(sd) ) {
					pc->setoption(sd,sd->sc.option&~OPTION_WUG);
					pc->setoption(sd,sd->sc.option|OPTION_WUGRIDER);
				} else if( pc_isridingwug(sd) ) {
					pc->setoption(sd,sd->sc.option&~OPTION_WUGRIDER);
					pc->setoption(sd,sd->sc.option|OPTION_WUG);
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case RA_WUGDASH:
			if( tsce ) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,status_change_end(bl, type, INVALID_TIMER));
				map->freeblock_unlock();
				return 0;
			}
			if( sd && pc_isridingwug(sd) ) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start4(src,bl,type,100,skill_lv,unit->getdir(bl),0,0,1));
				clif->walkok(sd);
			}
			break;

		case RA_SENSITIVEKEEN:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			clif->skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			map->foreachinrange(skill->area_sub,src,skill->get_splash(skill_id,skill_lv),BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY,skill->castend_damage_id);
			break;
		/**
		 * Mechanic
		 **/
		case NC_F_SIDESLIDE:
		case NC_B_SIDESLIDE:
			{
				uint8 dir = (skill_id == NC_F_SIDESLIDE) ? (unit->getdir(src)+4)%8 : unit->getdir(src);
				skill->blown(src,bl,skill->get_blewcount(skill_id,skill_lv),dir,0);
				clif->slide(src,src->x,src->y);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case NC_SELFDESTRUCTION:
			if (sd) {
				if (pc_ismadogear(sd))
					 pc->setmadogear(sd, false);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				skill->castend_damage_id(src, src, skill_id, skill_lv, tick, flag);
				status->set_sp(src, 0, 0);
			}
			break;

		case NC_ANALYZE:
			clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			clif->skill_nodamage(src, bl, skill_id, skill_lv,
				sc_start(src,bl,type, 30 + 12 * skill_lv,skill_lv,skill->get_time(skill_id,skill_lv)));
			if( sd ) pc->overheat(sd,1);
			break;

		case NC_MAGNETICFIELD:
		{
			int failure;
			if( (failure = sc_start2(src,bl,type,100,skill_lv,src->id,skill->get_time(skill_id,skill_lv))) )
			{
				map->foreachinrange(skill->area_sub,src,skill->get_splash(skill_id,skill_lv),splash_target(src),src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SD_SPLASH|1,skill->castend_damage_id);;
				clif->skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,BDT_SKILL);
				if (sd) pc->overheat(sd,1);
			}
			clif->skill_nodamage(src,src,skill_id,skill_lv,failure);
		}
			break;

		case NC_REPAIR:
			if( sd ) {
				int heal, hp = 0; // % of max hp regen
				if( !dstsd || !pc_ismadogear(dstsd) ) {
					clif->skill_fail(sd, skill_id,USESKILL_FAIL_TOTARGET,0);
					break;
				}
				switch (cap_value(skill_lv, 1, 5)) {
					case 1: hp = 4; break;
					case 2: hp = 7; break;
					case 3: hp = 13; break;
					case 4: hp = 17; break;
					case 5: hp = 23; break;
				}
				heal = tstatus->max_hp * hp / 100;
				status->heal(bl,heal,0,2);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, heal);
			}
			break;

		case NC_DISJOINT:
			{
				if( bl->type != BL_MOB ) break;
				md = map->id2md(bl->id);
				if (md && md->class_ >= MOBID_SILVERSNIPER && md->class_ <= MOBID_MAGICDECOY_WIND)
					status_kill(bl);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;
		case SC_AUTOSHADOWSPELL:
			if( sd ) {
				int idx1 = skill->get_index(sd->reproduceskill_id), idx2 = skill->get_index(sd->cloneskill_id);
				if( sd->status.skill[idx1].id || sd->status.skill[idx2].id ) {
					sc_start(src, src, SC_STOP, 100, skill_lv, INFINITE_DURATION); // The skill_lv is stored in val1 used in skill_select_menu to determine the used skill lvl [Xazax]
					clif->autoshadowspell_list(sd);
					clif->skill_nodamage(src,bl,skill_id,1,1);
				}
				else
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_IMITATION_SKILL_NONE,0);
			}
			break;

		case SC_SHADOWFORM:
			if( sd && dstsd && src != bl && !dstsd->shadowform_id ) {
				if( clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start4(src,src,type,100,skill_lv,bl->id,4+skill_lv,0,skill->get_time(skill_id, skill_lv))) )
					dstsd->shadowform_id = src->id;
			}
			else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
			break;

		case SC_BODYPAINT:
			if( flag&1 ) {
				if( tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
							tsc->data[SC_CHASEWALK] || tsc->data[SC_CLOAKINGEXCEED] ) ) {
					status_change_end(bl, SC_HIDING, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
					status_change_end(bl, SC_CHASEWALK, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);

					sc_start(src,bl,type,20 + 5 * skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
					sc_start(src,bl,SC_BLIND,53 + 2 * skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
				}
			} else {
				clif->skill_nodamage(src, bl, skill_id, 0, 1);
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), BL_CHAR,
				                    src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
			}
			break;

		case SC_ENERVATION:
		case SC_GROOMY:
		case SC_IGNORANCE:
		case SC_LAZINESS:
		case SC_UNLUCKY:
		case SC_WEAKNESS:
			if( !(tsc && tsc->data[type]) ) {
				int joblvbonus = 0;
				int rate = 0;
				if (is_boss(bl)) break;
				joblvbonus = ( sd ? sd->status.job_level : 50 );
				//First we set the success chance based on the caster's build which increases the chance.
				rate = 10 * skill_lv + rnd_value( sstatus->dex / 12, sstatus->dex / 4 ) + joblvbonus + status->get_lv(src) / 10;
				// We then reduce the success chance based on the target's build.
				rate -= rnd_value( tstatus->agi / 6, tstatus->agi / 3 ) + tstatus->luk / 10 + ( dstsd ? (dstsd->max_weight / 10 - dstsd->weight / 10 ) / 100 : 0 ) + status->get_lv(bl) / 10;
				//Finally we set the minimum success chance cap based on the caster's skill level and DEX.
				rate = cap_value( rate, skill_lv + sstatus->dex / 20, 100);
				clif->skill_nodamage(src,bl,skill_id,0,sc_start(src,bl,type,rate,skill_lv,skill->get_time(skill_id,skill_lv)));
				if ( tsc && tsc->data[SC__IGNORANCE] && skill_id == SC_IGNORANCE) {
					//If the target was successfully inflected with the Ignorance status, drain some of the targets SP.
					int sp = 100 * skill_lv;
					if( dstmd ) sp = dstmd->level * 2;
					if( status_zap(bl,0,sp) )
						status->heal(src,0,sp/2,3);//What does flag 3 do? [Rytech]
				}
				if ( tsc && tsc->data[SC__UNLUCKY] && skill_id == SC_UNLUCKY) {
					//If the target was successfully inflected with the Unlucky status, give 1 of 3 random status's.
					switch(rnd()%3) {//Targets in the Unlucky status will be affected by one of the 3 random status's regardless of resistance.
						case 0:
							status->change_start(src,bl,SC_POISON,10000,skill_lv,0,0,0,skill->get_time(skill_id,skill_lv),SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
							break;
						case 1:
							status->change_start(src,bl,SC_SILENCE,10000,skill_lv,0,0,0,skill->get_time(skill_id,skill_lv),SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
							break;
						case 2:
							status->change_start(src,bl,SC_BLIND,10000,skill_lv,0,0,0,skill->get_time(skill_id,skill_lv),SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
						}
				}
			} else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
			break;

		case LG_TRAMPLE:
			clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			map->foreachinrange(skill->destroy_trap,bl,skill->get_splash(skill_id,skill_lv),BL_SKILL,tick);
			break;

		case LG_REFLECTDAMAGE:
			if( tsc && tsc->data[type] )
				status_change_end(bl,type,INVALID_TIMER);
			else
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case LG_SHIELDSPELL:
			if( !sd )
				break;
			if( flag&1 ) {
				sc_start(src,bl,SC_SILENCE,100,skill_lv,sd->bonus.shieldmdef * 30000);
			} else {
				int opt = 0, val = 0, splashrange = 0;
				struct item_data *shield_data = NULL;
				if( sd->equip_index[EQI_HAND_L] < 0 || !( shield_data = sd->inventory_data[sd->equip_index[EQI_HAND_L]] ) || shield_data->type != IT_ARMOR ) {
					//Skill will first check if a shield is equipped. If none is found on the caster the skill will fail.
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				//Generates a number between 1 - 3. The number generated will determine which effect will be triggered.
				opt = rnd()%3 + 1;
				switch( skill_lv ) {
					case 1:
						if ( shield_data->def >= 0 && shield_data->def <= 40)
							splashrange = 1;
						else if ( shield_data->def >= 41 && shield_data->def <= 80)
							splashrange = 2;
						else
							splashrange = 3;
						switch( opt ) {
							case 1:
								sc_start(src, bl, SC_SHIELDSPELL_DEF, 100, opt, INFINITE_DURATION); // Splash AoE ATK
								clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
								map->foreachinrange(skill->area_sub,src,splashrange,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
								status_change_end(bl,SC_SHIELDSPELL_DEF,INVALID_TIMER);
								break;
							case 2:
								val = shield_data->def/10; //Damage Reflecting Increase.
								sc_start2(src,bl,SC_SHIELDSPELL_DEF,100,opt,val,shield_data->def * 1000);
								break;
							case 3:
								//Weapon Attack Increase.
								val = shield_data->def;
								sc_start2(src,bl,SC_SHIELDSPELL_DEF,100,opt,val,shield_data->def * 3000);
								break;
						}
						break;
					case 2:
						if( sd->bonus.shieldmdef == 0 )
							break; // Nothing should happen if the shield has no mdef, not even displaying a message
						if ( sd->bonus.shieldmdef >= 1 && sd->bonus.shieldmdef <= 3 )
							splashrange = 1;
						else if ( sd->bonus.shieldmdef >= 4 && sd->bonus.shieldmdef <= 5 )
							splashrange = 2;
						else
							splashrange = 3;
						switch( opt ) {
							case 1:
								sc_start(src, bl, SC_SHIELDSPELL_MDEF, 100, opt, INFINITE_DURATION); // Splash AoE MATK
								clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
								map->foreachinrange(skill->area_sub,src,splashrange,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
								status_change_end(bl,SC_SHIELDSPELL_MDEF,INVALID_TIMER);
								break;
							case 2:
								sc_start(src,bl,SC_SHIELDSPELL_MDEF,100,opt,sd->bonus.shieldmdef * 2000); //Splash AoE Lex Divina
								clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
								map->foreachinrange(skill->area_sub,src,splashrange,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_nodamage_id);
								break;
							case 3:
								if( sc_start(src,bl,SC_SHIELDSPELL_MDEF,100,opt,sd->bonus.shieldmdef * 30000) ) //Magnificat
									clif->skill_nodamage(src,bl,PR_MAGNIFICAT,skill_lv,
											sc_start(src,bl,SC_MAGNIFICAT,100,1,sd->bonus.shieldmdef * 30000));
								break;
						}
						break;
					case 3:
					{
						int rate = 0;
						struct item *shield = &sd->status.inventory[sd->equip_index[EQI_HAND_L]];

						if( shield->refine == 0 )
							break; // Nothing should happen if the shield has no refine, not even displaying a message

						switch( opt ) {
							case 1:
								sc_start(src,bl,SC_SHIELDSPELL_REF,100,opt,shield->refine * 30000); //Now breaks Armor at 100% rate
								break;
							case 2:
								val = shield->refine * 10 * status->get_lv(src) / 100; //DEF Increase
								rate = (shield->refine * 2) + (status_get_luk(src) / 10); //Status Resistance Rate
								if( sc_start2(src,bl,SC_SHIELDSPELL_REF,100,opt,val,shield->refine * 20000))
									clif->skill_nodamage(src,bl,SC_SCRESIST,skill_lv,
											sc_start(src,bl,SC_SCRESIST,100,rate,shield->refine * 30000));
								break;
							case 3:
								sc_start(src, bl, SC_SHIELDSPELL_REF, 100, opt, INFINITE_DURATION); // HP Recovery
								val = sstatus->max_hp * ((status->get_lv(src) / 10) + (shield->refine + 1)) / 100;
								status->heal(bl, val, 0, 2);
								status_change_end(bl,SC_SHIELDSPELL_REF,INVALID_TIMER);
								break;
						}
					}
					break;
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case LG_PIETY:
			if( flag&1 )
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
			else {
				skill->area_temp[2] = 0;
				map->foreachinrange(skill->area_sub,bl,skill->get_splash(skill_id,skill_lv),BL_PC,src,skill_id,skill_lv,tick,flag|SD_PREAMBLE|BCT_PARTY|BCT_SELF|1,skill->castend_nodamage_id);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;
		case LG_KINGS_GRACE:
			if( flag&1 ){
				int i;
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
				for(i=0; i<SC_MAX; i++)
				{
					if (!tsc->data[i])
					continue;
					switch(i){
						case SC_POISON:
						case SC_BLIND:
						case SC_FREEZE:
						case SC_STONE:
						case SC_STUN:
						case SC_SLEEP:
						case SC_BLOODING:
						case SC_CURSE:
						case SC_CONFUSION:
						case SC_ILLUSION:
						case SC_SILENCE:
						case SC_BURNING:
						case SC_COLD:
						case SC_FROSTMISTY:
						case SC_DEEP_SLEEP:
						case SC_FEAR:
						case SC_MANDRAGORA:
						case SC__CHAOS:
							status_change_end(bl, (sc_type)i, INVALID_TIMER);
					}
				}
			}else {
				skill->area_temp[2] = 0;
				if( !map_flag_vs(src->m) && !map_flag_gvg(src->m) )
					flag |= BCT_GUILD;
				map->foreachinrange(skill->area_sub,bl,skill->get_splash(skill_id,skill_lv),BL_PC,src,skill_id,skill_lv,tick,flag|SD_PREAMBLE|BCT_PARTY|BCT_SELF|1,skill->castend_nodamage_id);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;
		case LG_INSPIRATION:
			if( sd && !map->list[sd->bl.m].flag.noexppenalty && sd->status.base_level != MAX_LEVEL ) {
					sd->status.base_exp -= min(sd->status.base_exp, pc->nextbaseexp(sd) * 1 / 100); // 1% penalty.
					sd->status.job_exp -= min(sd->status.job_exp, pc->nextjobexp(sd) * 1 / 100);
					clif->updatestatus(sd,SP_BASEEXP);
					clif->updatestatus(sd,SP_JOBEXP);
			}
				clif->skill_nodamage(bl,src,skill_id,skill_lv,
					sc_start(src,bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;
		case SR_CURSEDCIRCLE:
			if( flag&1 ) {
				if( is_boss(bl) ) break;
				if( sc_start2(src,bl, type, 100, skill_lv, src->id, skill->get_time(skill_id, skill_lv))) {
					if (bl->type == BL_MOB)
						mob->unlocktarget(BL_UCAST(BL_MOB, bl), timer->gettick());
					unit->stop_attack(bl);
					clif->bladestop(src, bl->id, 1);
					map->freeblock_unlock();
					return 1;
				}
			} else {
				int count = 0;
				clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				count = map->forcountinrange(skill->area_sub, src, skill->get_splash(skill_id,skill_lv), (sd)?sd->spiritball_old:15, // Assume 15 spiritballs in non-characters
					BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
				if( sd ) pc->delspiritball(sd, count, 0);
				clif->skill_nodamage(src, src, skill_id, skill_lv,
					sc_start2(src, src, SC_CURSEDCIRCLE_ATKER, 100, skill_lv, count, skill->get_time(skill_id,skill_lv)));
			}
			break;

		case SR_RAISINGDRAGON:
			if ( sd ) {
				int i, max;
				sc_start(src, bl, SC_EXPLOSIONSPIRITS, 100, skill_lv, skill->get_time(skill_id, skill_lv));
				clif->skill_nodamage(src, bl, skill_id, skill_lv,
						sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
				max = pc->getmaxspiritball(sd, 0);
				for ( i = 0; i < max; i++ )
					pc->addspiritball(sd, skill->get_time(MO_CALLSPIRITS, skill_lv), max);
			}
			break;

		case SR_ASSIMILATEPOWER:
			if( flag&1 ) {
				int sp = 0;
				if( dstsd && dstsd->spiritball && (sd == dstsd || map_flag_vs(src->m)) && (dstsd->class_&MAPID_BASEMASK)!=MAPID_GUNSLINGER )
				{
					sp = dstsd->spiritball; //1%sp per spiritball.
					pc->delspiritball(dstsd, dstsd->spiritball, 0);
					status_percent_heal(src, 0, sp);
				}
				if (dstsd && dstsd->charm_type != CHARM_TYPE_NONE && dstsd->charm_count > 0) {
					pc->del_charm(dstsd, dstsd->charm_count, dstsd->charm_type);
				}
				clif->skill_nodamage(src, bl, skill_id, skill_lv, sp ? 1:0);
			} else {
				clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|BCT_SELF|SD_SPLASH|1, skill->castend_nodamage_id);
			}
			break;

		case SR_POWERVELOCITY:
			if( !dstsd )
				break;
			if ( sd && (dstsd->class_&MAPID_BASEMASK) != MAPID_GUNSLINGER ) {
				int i, max = pc->getmaxspiritball(dstsd, 5);
				for ( i = 0; i < max; i++ ) {
					pc->addspiritball(dstsd, skill->get_time(MO_CALLSPIRITS, 1), max);
				}
				pc->delspiritball(sd, sd->spiritball, 0);
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			break;

		case SR_GENTLETOUCH_CURE:
			{
				int heal;

				if( status->isimmune(bl) ) {
					clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
					break;
				}

				heal = 120 * skill_lv + status_get_max_hp(bl) * (2 + skill_lv) / 100;
				status->heal(bl, heal, 0, 0);

				if( (tsc && tsc->opt1) && (rnd()%100 < ((skill_lv * 5) + (status_get_dex(src) + status->get_lv(src)) / 4) - (1 + (rnd() % 10))) ) {
					status_change_end(bl, SC_STONE, INVALID_TIMER);
					status_change_end(bl, SC_FREEZE, INVALID_TIMER);
					status_change_end(bl, SC_STUN, INVALID_TIMER);
					status_change_end(bl, SC_POISON, INVALID_TIMER);
					status_change_end(bl, SC_SILENCE, INVALID_TIMER);
					status_change_end(bl, SC_BLIND, INVALID_TIMER);
					status_change_end(bl, SC_ILLUSION, INVALID_TIMER);
					status_change_end(bl, SC_BURNING, INVALID_TIMER);
					status_change_end(bl, SC_FROSTMISTY, INVALID_TIMER);
				}

				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;
		case SR_GENTLETOUCH_CHANGE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,100,skill_lv,bl->id,skill->get_time(skill_id,skill_lv)));
			break;
		case SR_GENTLETOUCH_REVITALIZE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,100,skill_lv,status_get_vit(src),skill->get_time(skill_id,skill_lv)));
			break;
		case SR_FLASHCOMBO:
		{
			const int combo[] = {
				SR_DRAGONCOMBO, SR_FALLENEMPIRE, SR_TIGERCANNON, SR_SKYNETBLOW
			};
			int i;

			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start2(src,bl,type,100,skill_lv,bl->id,skill->get_time(skill_id,skill_lv)));

			for( i = 0; i < ARRAYLENGTH(combo); i++ )
				skill->addtimerskill(src, tick + 400 * i, bl->id, 0, 0, combo[i], skill_lv, BF_WEAPON, flag|SD_LEVEL);

			break;
		}
		case WA_SWING_DANCE:
		case WA_SYMPHONY_OF_LOVER:
		case WA_MOONLIT_SERENADE:
		case MI_RUSH_WINDMILL:
		case MI_ECHOSONG:
			if( flag&1 )
				sc_start2(src,bl,type,100,skill_lv,(sd?pc->checkskill(sd,WM_LESSON):0),skill->get_time(skill_id,skill_lv));
			else if( sd ) {
				party->foreachsamemap(skill->area_sub,sd,skill->get_splash(skill_id,skill_lv),src,skill_id,skill_lv,tick,flag|BCT_PARTY|1,skill->castend_nodamage_id);
				sc_start2(src,bl,type,100,skill_lv,pc->checkskill(sd,WM_LESSON),skill->get_time(skill_id,skill_lv));
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case MI_HARMONIZE:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start2(src,bl,type,100,skill_lv,(sd?pc->checkskill(sd,WM_LESSON):1),skill->get_time(skill_id,skill_lv)));
			break;

		case WM_DEADHILLHERE:
			if( bl->type == BL_PC ) {
				if( !status->isdead(bl) )
					break;

				if( rnd()%100 < 88 + 2 * skill_lv ) {
					int heal = 0;
					status_zap(bl, 0, tstatus->sp * (60 - 10 * skill_lv) / 100);
					heal = tstatus->sp;
					if ( heal <= 0 )
						heal = 1;
					status->fixed_revive(bl, heal, 0);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					status->set_sp(bl, 0, 0);
				}
			}
			break;

		case WM_LULLABY_DEEPSLEEP:
			if ( flag&1 )
				sc_start2(src,bl,type,100,skill_lv,src->id,skill->get_time(skill_id,skill_lv));
			else if ( sd ) {
				int rate = 4 * skill_lv + 2 * pc->checkskill(sd,WM_LESSON) + status->get_lv(src)/15 + sd->status.job_level/5;
				if ( rnd()%100 < rate ) {
					flag |= BCT_PARTY|BCT_GUILD;
					map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id,skill_lv),BL_CHAR|BL_NPC|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					status_change_end(bl, SC_DEEP_SLEEP, INVALID_TIMER);
				}
			}
			break;
		case WM_SIRCLEOFNATURE:
			flag |= BCT_SELF|BCT_PARTY|BCT_GUILD;
			/* Fall through */
		case WM_VOICEOFSIREN:
			if( skill_id != WM_SIRCLEOFNATURE )
				flag &= ~BCT_SELF;
			if( flag&1 ) {
				sc_start2(src,bl,type,100,skill_lv,(skill_id==WM_VOICEOFSIREN)?src->id:0,skill->get_time(skill_id,skill_lv));
			} else if( sd ) {
				int rate = 6 * skill_lv + pc->checkskill(sd,WM_LESSON) + sd->status.job_level/2;
				if ( rnd()%100 < rate ) {
					flag |= BCT_PARTY|BCT_GUILD;
					map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id,skill_lv),(skill_id==WM_VOICEOFSIREN)?BL_CHAR|BL_NPC|BL_SKILL:BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					status_change_end(bl, SC_SIREN, INVALID_TIMER);
				}
			}
			break;

		case WM_GLOOMYDAY:
			if ( tsc && tsc->data[type] ) {
				 clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
				break;
			}
			// val4 indicates caster's voice lesson level
			sc_start4(src,bl,type,100,skill_lv, 0, 0, sd?pc->checkskill(sd,WM_LESSON):10, skill->get_time(skill_id,skill_lv));
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			break;

		case WM_SONG_OF_MANA:
		case WM_DANCE_WITH_WUG:
		case WM_LERADS_DEW:
		case WM_UNLIMITED_HUMMING_VOICE:
		{
			int chorusbonus = battle->calc_chorusbonus(sd);
			if( flag&1 )
				sc_start2(src,bl,type,100,skill_lv,chorusbonus,skill->get_time(skill_id,skill_lv));
			else if( sd ) {
				party->foreachsamemap(skill->area_sub,sd,skill->get_splash(skill_id,skill_lv),src,skill_id,skill_lv,tick,flag|BCT_PARTY|1,skill->castend_nodamage_id);
				sc_start2(src,bl,type,100,skill_lv,chorusbonus,skill->get_time(skill_id,skill_lv));
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
		}
			break;
		case WM_SATURDAY_NIGHT_FEVER:
		{
			if( flag&1 ) {
				int madnesscheck = 0;
				if ( sd )//Required to check if the lord of madness effect will be applied.
					madnesscheck = map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id,skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY, skill->area_sub_count);
				sc_start(src, bl, type, 100, skill_lv,skill->get_time(skill_id, skill_lv));
				if ( madnesscheck >= 8 )//The god of madness deals 9999 fixed unreduceable damage when 8 or more enemy players are affected.
					status_fix_damage(src, bl, 9999, clif->damage(src, bl, 0, 0, 9999, 0, BDT_NORMAL, 0));
					//skill->attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag);//To renable when I can confirm it deals damage like this. Data shows its dealt as reflected damage which I don't have it coded like that yet. [Rytech]
			} else if( sd ) {
				int rate = sstatus->int_ / 6 + (sd? sd->status.job_level:0) / 5 + skill_lv * 4;
				if ( rnd()%100 < rate ) {
					map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id,skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				}
			}
		}
			break;

		case WM_MELODYOFSINK:
		case WM_BEYOND_OF_WARCRY:
		{
			int chorusbonus = battle->calc_chorusbonus(sd);
			if( flag&1 )
				sc_start2(src,bl,type,100,skill_lv,chorusbonus,skill->get_time(skill_id,skill_lv));
			else if( sd ) {
				if ( rnd()%100 < 15 + 5 * skill_lv + 5 * chorusbonus ) {
					map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id,skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				}
			}
		}
			break;

		case WM_RANDOMIZESPELL: {
				int improv_skill_id = 0, improv_skill_lv, improv_idx;
				do {
					improv_idx = rnd() % MAX_SKILL_IMPROVISE_DB;
					improv_skill_id = skill->dbs->improvise_db[improv_idx].skill_id;
				} while( improv_skill_id == 0 || rnd()%10000 >= skill->dbs->improvise_db[improv_idx].per );
				improv_skill_lv = 4 + skill_lv;
				clif->skill_nodamage (src, bl, skill_id, skill_lv, 1);

				if (sd != NULL) {
					sd->state.abra_flag = 2;
					sd->skillitem = improv_skill_id;
					sd->skillitemlv = improv_skill_lv;
					clif->item_skill(sd, improv_skill_id, improv_skill_lv);
				} else {
					struct unit_data *ud = unit->bl2ud(src);
					int inf = skill->get_inf(improv_skill_id);
					if (ud == NULL)
						break;
					if (inf&INF_SELF_SKILL || inf&INF_SUPPORT_SKILL) {
						int id = src->id;
						struct pet_data *pd = BL_CAST(BL_PET, src);
						if (pd != NULL && pd->msd != NULL)
							id = pd->msd->bl.id;
						unit->skilluse_id(src, id, improv_skill_id, improv_skill_lv);
					} else {
						int target_id = 0;
						if (ud->target) {
							target_id = ud->target;
						} else {
							switch (src->type) {
							case BL_MOB: target_id = BL_UCAST(BL_MOB, src)->target_id; break;
							case BL_PET: target_id = BL_UCAST(BL_PET, src)->target_id; break;
							}
						}
						if (!target_id)
							break;
						if (skill->get_casttype(improv_skill_id) == CAST_GROUND) {
							bl = map->id2bl(target_id);
							if (!bl) bl = src;
							unit->skilluse_pos(src, bl->x, bl->y, improv_skill_id, improv_skill_lv);
						} else
							unit->skilluse_id(src, target_id, improv_skill_id, improv_skill_lv);
					}
				}
			}
			break;

		case RETURN_TO_ELDICASTES:
		case ALL_GUARDIAN_RECALL:
		case ECLAGE_RECALL:
			if( sd ) {
				short x = 0, y = 0; //Destiny position.
				unsigned short map_index = 0;

				switch( skill_id ) {
					default:
					case RETURN_TO_ELDICASTES:
						x = 198;
						y = 187;
						map_index  = mapindex->name2id(MAP_DICASTES);
						break;
					case ALL_GUARDIAN_RECALL:
						x = 44;
						y = 151;
						map_index  = mapindex->name2id(MAP_MORA);
						break;
					case ECLAGE_RECALL:
						x = 47;
						y = 31;
						map_index  = mapindex->name2id(MAP_ECLAGE_IN);
						break;
				}
				if( !mapindex ) { //Given map not found?
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					map->freeblock_unlock();
					return 0;
				}
				pc->setpos(sd,map_index,x,y,CLR_TELEPORT);
			}
			break;

		case ECL_SNOWFLIP:
		case ECL_PEONYMAMY:
		case ECL_SADAGUI:
		case ECL_SEQUOIADUST:
			switch( skill_id ) {
				case ECL_SNOWFLIP:
					status_change_end(bl,SC_SLEEP,INVALID_TIMER);
					status_change_end(bl,SC_BLOODING,INVALID_TIMER);
					status_change_end(bl,SC_BURNING,INVALID_TIMER);
					status_change_end(bl,SC_DEEP_SLEEP,INVALID_TIMER);
					break;
				case ECL_PEONYMAMY:
					status_change_end(bl,SC_FREEZE,INVALID_TIMER);
					status_change_end(bl,SC_FROSTMISTY,INVALID_TIMER);
					status_change_end(bl,SC_COLD,INVALID_TIMER);
					break;
				case ECL_SADAGUI:
					status_change_end(bl,SC_STUN,INVALID_TIMER);
					status_change_end(bl,SC_CONFUSION,INVALID_TIMER);
					status_change_end(bl,SC_ILLUSION,INVALID_TIMER);
					status_change_end(bl,SC_FEAR,INVALID_TIMER);
					break;
				case ECL_SEQUOIADUST:
					status_change_end(bl,SC_STONE,INVALID_TIMER);
					status_change_end(bl,SC_POISON,INVALID_TIMER);
					status_change_end(bl,SC_CURSE,INVALID_TIMER);
					status_change_end(bl,SC_BLIND,INVALID_TIMER);
					status_change_end(bl,SC_ORCISH,INVALID_TIMER);
					break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, 0, 1, skill_id, -2, BDT_SKILL);
			break;

		case GM_SANDMAN:
			if( tsc ) {
				if( tsc->opt1 == OPT1_SLEEP )
					tsc->opt1 = 0;
				else
					tsc->opt1 = OPT1_SLEEP;
				clif->changeoption(bl);
				clif->skill_nodamage (src, bl, skill_id, skill_lv, 1);
			}
			break;

		case SO_ARRULLO:
			{
				// [(15 + 5 * Skill Level) + ( Caster?s INT / 5 ) + ( Caster?s Job Level / 5 ) - ( Target?s INT / 6 ) - ( Target?s LUK / 10 )] %
				int rate = (15 + 5 * skill_lv) + status_get_int(src)/5 + (sd? sd->status.job_level:0)/5;
				rate -= status_get_int(bl)/6 - status_get_luk(bl)/10;
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				sc_start2(src,bl, type, rate, skill_lv, 1, skill->get_time(skill_id, skill_lv));
			}
			break;

		case SO_SUMMON_AGNI:
		case SO_SUMMON_AQUA:
		case SO_SUMMON_VENTUS:
		case SO_SUMMON_TERA:
			if( sd ) {
				int elemental_class = skill->get_elemental_type(skill_id,skill_lv);

				// Remove previous elemental first.
				if( sd->ed )
					elemental->delete(sd->ed,0);

				// Summoning the new one.
				if( !elemental->create(sd,elemental_class,skill->get_time(skill_id,skill_lv)) ) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case SO_EL_CONTROL:
			if( sd ) {
				uint32 mode = EL_MODE_PASSIVE; // Standard mode.

				if( !sd->ed ) break;

				if( skill_lv == 4 ) {// At level 4 delete elementals.
					elemental->delete(sd->ed, 0);
					break;
				}
				switch( skill_lv ) {// Select mode based on skill level used.
					case 2: mode = EL_MODE_ASSIST; break;
					case 3: mode = EL_MODE_AGGRESSIVE; break;
				}
				if (!elemental->change_mode(sd->ed, mode)) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case SO_EL_ACTION:
			if( sd ) {
				int duration = 3000;
				if( !sd->ed )
					break;

				switch (sd->ed->db->class_) {
					case ELEID_EL_AGNI_M:
					case ELEID_EL_AQUA_M:
					case ELEID_EL_VENTUS_M:
					case ELEID_EL_TERA_M:
						duration = 6000;
						break;
					case ELEID_EL_AGNI_L:
					case ELEID_EL_AQUA_L:
					case ELEID_EL_VENTUS_L:
					case ELEID_EL_TERA_L:
						duration = 9000;
						break;
				}

				sd->skill_id_old = skill_id;
				elemental->action(sd->ed, bl, tick);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);

				skill->blockpc_start(sd, skill_id, duration);
			}
			break;

		case SO_EL_CURE:
			if( sd ) {
				struct elemental_data *ed = sd->ed;
				int s_hp = sd->battle_status.hp * 10 / 100, s_sp = sd->battle_status.sp * 10 / 100;
				int e_hp, e_sp;

				if( !ed ) break;
				if( !status->charge(&sd->bl,s_hp,s_sp) ) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
				e_hp = ed->battle_status.max_hp * 10 / 100;
				e_sp = ed->battle_status.max_sp * 10 / 100;
				status->heal(&ed->bl,e_hp,e_sp,3);
				clif->skill_nodamage(src,&ed->bl,skill_id,skill_lv,1);
			}
			break;

		case GN_CHANGEMATERIAL:
		case SO_EL_ANALYSIS:
			if( sd ) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				clif->skill_itemlistwindow(sd,skill_id,skill_lv);
			}
			break;

		case GN_BLOOD_SUCKER:
			{
				struct status_change *sc = status->get_sc(src);

				if( sc && sc->bs_counter < skill->get_maxcount( skill_id , skill_lv) ) {
					if( tsc && tsc->data[type] ){
						(sc->bs_counter)--;
						status_change_end(src, type, INVALID_TIMER); // the first one cancels and the last one will take effect resetting the timer
					}
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
					sc_start2(src,bl, type, 100, skill_lv, src->id, skill->get_time(skill_id,skill_lv));
					(sc->bs_counter)++;
				} else if( sd ) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
					break;
				}
			}
			break;

		case GN_MANDRAGORA:
			if( flag&1 ) {
				int chance = 25 + 10 * skill_lv - (status_get_vit(bl) + status_get_luk(bl)) / 5;
				if ( chance < 10 )
					chance = 10;//Minimal chance is 10%.
				if ( rnd()%100 < chance ) {//Coded to both inflect the status and drain the target's SP only when successful. [Rytech]
				sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
				status_zap(bl, 0, status_get_max_sp(bl) * (25 + 5 * skill_lv) / 100);
				}
			} else if ( sd ) {
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), BL_CHAR,src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
				clif->skill_nodamage(bl, src, skill_id, skill_lv, 1);
			}
			break;

		case GN_SLINGITEM:
			if( sd ) {
				short ammo_id;
				int equip_idx = sd->equip_index[EQI_AMMO];
				if( equip_idx <= 0 )
					break; // No ammo.
				ammo_id = sd->inventory_data[equip_idx]->nameid;
				if( ammo_id <= 0 )
					break;
				sd->itemid = ammo_id;
				if( itemdb_is_GNbomb(ammo_id) ) {
					if(battle->check_target(src,bl,BCT_ENEMY) > 0) {// Only attack if the target is an enemy.
						if( ammo_id == ITEMID_PINEAPPLE_BOMB )
							map->foreachincell(skill->area_sub,bl->m,bl->x,bl->y,BL_CHAR,src,GN_SLINGITEM_RANGEMELEEATK,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
						else
							skill->attack(BF_WEAPON,src,src,bl,GN_SLINGITEM_RANGEMELEEATK,skill_lv,tick,flag);
					} else //Otherwise, it fails, shows animation and removes items.
						clif->skill_fail(sd,GN_SLINGITEM_RANGEMELEEATK,0xa,0);
				} else if( itemdb_is_GNthrowable(ammo_id) ) {
					struct script_code *scriptroot = sd->inventory_data[equip_idx]->script;
					if( !scriptroot )
						break;
					if( dstsd )
						script->run(scriptroot,0,dstsd->bl.id,npc->fake_nd->bl.id);
					else
						script->run(scriptroot,0,src->id,0);
				}
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);// This packet is received twice actually, I think it is to show the animation.
			break;

		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
			if( sd ) {
				int qty = 1;
				sd->skill_id_old = skill_id;
				sd->skill_lv_old = skill_lv;
				if( skill_id != GN_S_PHARMACY && skill_lv > 1 )
					qty = 10;
				clif->cooking_list(sd,(skill_id - GN_MIX_COOKING) + 27,skill_id,qty,skill_id==GN_MAKEBOMB?5:6);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;
		case EL_CIRCLE_OF_FIRE:
		case EL_PYROTECHNIC:
		case EL_HEATER:
		case EL_TROPIC:
		case EL_AQUAPLAY:
		case EL_COOLER:
		case EL_CHILLY_AIR:
		case EL_GUST:
		case EL_BLAST:
		case EL_WILD_STORM:
		case EL_PETROLOGY:
		case EL_CURSED_SOIL:
		case EL_UPHEAVAL:
		case EL_FIRE_CLOAK:
		case EL_WATER_DROP:
		case EL_WIND_CURTAIN:
		case EL_SOLID_SKIN:
		case EL_STONE_SHIELD:
		case EL_WIND_STEP:
		{
			struct elemental_data *ele = BL_CAST(BL_ELEM, src);
			if( ele ) {
				sc_type type2 = type-1;
				struct status_change *sc = status->get_sc(&ele->bl);

				if( (sc && sc->data[type2]) || (tsc && tsc->data[type]) ) {
					elemental->clean_single_effect(ele, skill_id);
				} else {
					clif->skill_nodamage(src,src,skill_id,skill_lv,1);
					clif->skill_damage(src, ( skill_id == EL_GUST || skill_id == EL_BLAST || skill_id == EL_WILD_STORM )?src:bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
					if( skill_id == EL_WIND_STEP ) // There aren't teleport, just push the master away.
						skill->blown(src,bl,(rnd()%skill->get_blewcount(skill_id,skill_lv))+1,rnd()%8,0);
					sc_start(src, src,type2,100,skill_lv,skill->get_time(skill_id,skill_lv));
					sc_start(src, bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
				}
			}
		}
			break;

		case EL_FIRE_MANTLE:
		case EL_WATER_BARRIER:
		case EL_ZEPHYR:
		case EL_POWER_OF_GAIA:
			clif->skill_nodamage(src,src,skill_id,skill_lv,1);
			clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			skill->unitsetting(src,skill_id,skill_lv,bl->x,bl->y,0);
			break;

		case EL_WATER_SCREEN:
		{
			struct elemental_data *ele = BL_CAST(BL_ELEM, src);
			if( ele ) {
				struct status_change *sc = status->get_sc(&ele->bl);
				sc_type type2 = type-1;

				clif->skill_nodamage(src,src,skill_id,skill_lv,1);
				if( (sc && sc->data[type2]) || (tsc && tsc->data[type]) ) {
					elemental->clean_single_effect(ele, skill_id);
				} else {
					// This not heals at the end.
					clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
					sc_start(src, src,type2,100,skill_lv,skill->get_time(skill_id,skill_lv));
					sc_start(src, bl,type,100,src->id,skill->get_time(skill_id,skill_lv));
				}
			}
		}
			break;

		case KO_KAHU_ENTEN:
		case KO_HYOUHU_HUBUKI:
		case KO_KAZEHU_SEIRAN:
		case KO_DOHU_KOUKAI:
			if(sd) {
				int ttype = skill->get_ele(skill_id, skill_lv);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				pc->add_charm(sd, skill->get_time(skill_id, skill_lv), MAX_SPIRITCHARM, ttype); // replace existing charms of other type
			}
			break;

		case KO_ZANZOU:
			if(sd) {
				struct mob_data *summon_md;

				summon_md = mob->once_spawn_sub(src, src->m, src->x, src->y, clif->get_bl_name(src), MOBID_KO_KAGE, "", SZ_SMALL, AI_NONE);
				if( summon_md ) {
					summon_md->master_id = src->id;
					summon_md->special_state.ai = AI_ZANZOU;
					if( summon_md->deletetimer != INVALID_TIMER )
						timer->delete(summon_md->deletetimer, mob->timer_delete);
					summon_md->deletetimer = timer->add(timer->gettick() + skill->get_time(skill_id, skill_lv), mob->timer_delete, summon_md->bl.id, 0);
					mob->spawn( summon_md );
					pc->setinvincibletimer(sd,500);// unlock target lock
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					skill->blown(src,bl,skill->get_blewcount(skill_id,skill_lv),unit->getdir(bl),0);
				}
			}
			break;

		case KO_KYOUGAKU:
			if (!map_flag_vs(src->m) || !dstsd) {
				if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_SIZE, 0);
				break;
			} else {
				int time;
				int rate = 45+ 5*skill_lv - status_get_int(bl)/10;
				if (rate < 5) rate = 5;

				time =  skill->get_time(skill_id, skill_lv) - 1000*status_get_int(bl)/20;
				sc_start(src,bl, type, rate, skill_lv, time);
			}
			break;

		case KO_JYUSATSU:
			if( dstsd && tsc && !tsc->data[type]
			 && rnd()%100 < (10 * (5 * skill_lv - status_get_int(bl) / 2 + 45 + 5 * skill_lv))
			) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv,
				                     status->change_start(src, bl, type, 10000, skill_lv, 0, 0, 0, skill->get_time(skill_id, skill_lv), SCFLAG_NOAVOID));
				status_zap(bl, tstatus->max_hp * skill_lv * 5 / 100 , 0);
				if( status->get_lv(bl) <= status->get_lv(src) )
					status->change_start(src, bl, SC_COMA, skill_lv, skill_lv, 0, src->id, 0, 0, SCFLAG_NONE);
			} else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
			break;

		case KO_GENWAKU:
			if ( !map_flag_gvg2(src->m) && ( dstsd || dstmd ) && !(tstatus->mode&MD_PLANT) && battle->check_target(src,bl,BCT_ENEMY) > 0 ) {
				int x = src->x, y = src->y;
				if( sd && rnd()%100 > max(5, (45 + 5 * skill_lv) - status_get_int(bl) / 10) ){//[(Base chance of success) - ( target's int / 10)]%.
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0);
					break;
				}

				if (unit->movepos(src, bl->x, bl->y, 0, 0)) {
					clif->skill_nodamage(src, src, skill_id, skill_lv, 1);
					clif->slide(src, bl->x, bl->y) ;
					sc_start(src, src, SC_CONFUSION, 25, skill_lv, skill->get_time(skill_id, skill_lv));
					if ( !is_boss(bl) && unit->stop_walking(&sd->bl, STOPWALKING_FLAG_FIXPOS) && unit->movepos(bl, x, y, 0, 0) ) {
						if( dstsd && pc_issit(dstsd) )
							pc->setstand(dstsd);
						clif->slide(bl, x, y) ;
						sc_start(src, bl, SC_CONFUSION, 75, skill_lv, skill->get_time(skill_id, skill_lv));
					}
				}
			}
			break;

		case OB_AKAITSUKI:
		case OB_OBOROGENSOU:
			if( sd && ( (skill_id == OB_OBOROGENSOU && bl->type == BL_MOB) // This skill does not work on monsters.
				|| is_boss(bl) ) ){ // Does not work on Boss monsters.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_TOTARGET_PLAYER, 0);
				break;
			}
		case KO_IZAYOI:
		case OB_ZANGETSU:
		case KG_KYOMU:
		case KG_KAGEMUSYA:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src, bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			break;

		case KG_KAGEHUMI:
			if( flag&1 ){
				if(tsc && ( tsc->option&(OPTION_CLOAK|OPTION_HIDE) ||
					tsc->data[SC_CAMOUFLAGE] || tsc->data[SC__SHADOWFORM] ||
					tsc->data[SC_MARIONETTE_MASTER] || tsc->data[SC_HARMONIZE])){
						sc_start(src, src, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
						sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
						status_change_end(bl, SC_HIDING, INVALID_TIMER);
						status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
						status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
						status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
						status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
						status_change_end(bl, SC_MARIONETTE_MASTER, INVALID_TIMER);
						status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
				}
				if( skill->area_temp[2] == 1 ){
					clif->skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
					sc_start(src, src, SC_STOP, 100, skill_lv, skill->get_time(skill_id, skill_lv));
				}
			} else {
				skill->area_temp[2] = 0;
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_nodamage_id);
			}
			break;

		case MH_LIGHT_OF_REGENE:
			if( hd && battle->get_master(src) ) {
				hd->homunculus.intimacy = (751 + rnd()%99) * 100; // random between 751 ~ 850
				clif->send_homdata(hd->master, SP_INTIMATE, hd->homunculus.intimacy / 100); //refresh intimacy info
				sc_start(src, battle->get_master(src), type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			}
			break;

		case MH_OVERED_BOOST:
			if ( hd && battle->get_master(src) ) {
				sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
				sc_start(src, battle->get_master(src), type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			}
			break;

		case MH_SILENT_BREEZE:
		{
			const enum sc_type scs[] = {
				SC_MANDRAGORA, SC_HARMONIZE, SC_DEEP_SLEEP, SC_SIREN, SC_SLEEP, SC_CONFUSION, SC_ILLUSION
			};
			int heal;
			if(tsc){
				int i;
				for (i = 0; i < ARRAYLENGTH(scs); i++) {
					if (tsc->data[scs[i]]) status_change_end(bl, scs[i], INVALID_TIMER);
				}
			}
			heal = 5 * status->get_lv(&hd->bl) + status->base_matk(&hd->bl, &hd->battle_status, status->get_lv(&hd->bl));
			status->heal(bl, heal, 0, 0);
			clif->skill_nodamage(src, src, skill_id, skill_lv, clif->skill_nodamage(src, bl, AL_HEAL, heal, 1));
			status->change_start(src, src, type, 1000, skill_lv, 0, 0, 0, skill->get_time(skill_id,skill_lv), SCFLAG_NOAVOID|SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
			status->change_start(src, bl,  type, 1000, skill_lv, 0, 0, 0, skill->get_time(skill_id,skill_lv), SCFLAG_NOAVOID|SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
		}
			break;

		case MH_GRANITIC_ARMOR:
		case MH_PYROCLASTIC:
			if( hd ){
				struct block_list *s_bl = battle->get_master(src);

				if(s_bl)
					sc_start2(src, s_bl, type, 100, skill_lv, hd->homunculus.level, skill->get_time(skill_id, skill_lv)); //start on master

				sc_start2(src, bl, type, 100, skill_lv, hd->homunculus.level, skill->get_time(skill_id, skill_lv));

				skill->blockhomun_start(hd, skill_id, skill->get_cooldown(skill_id, skill_lv));
			}
			break;

		case MH_MAGMA_FLOW:
		case MH_PAIN_KILLER:
			sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			if (hd)
				skill->blockhomun_start(hd, skill_id, skill->get_cooldown(skill_id, skill_lv));
			break;
		case MH_SUMMON_LEGION:
		{
			struct {
				int mob_id;
				int quantity;
			} summons[5] = {
				{ MOBID_HORNET,        3 },
				{ MOBID_GIANT_HONET,   3 },
				{ MOBID_GIANT_HONET,   4 },
				{ MOBID_LUCIOLA_VESPA, 4 },
				{ MOBID_LUCIOLA_VESPA, 5 },
			};
			int i, dummy = 0;
			Assert_retb(skill_lv < ARRAYLENGTH(summons));

			i = map->foreachinmap(skill->check_condition_mob_master_sub, src->m, BL_MOB, src->id, summons[skill_lv-1].mob_id, skill_id, &dummy);
			if(i >= summons[skill_lv-1].quantity)
				break;

			for (i = 0; i < summons[skill_lv-1].quantity; i++) {
				struct mob_data *summon_md = mob->once_spawn_sub(src, src->m, src->x, src->y, clif->get_bl_name(src),
				                                                 summons[skill_lv-1].mob_id, "", SZ_SMALL, AI_ATTACK);
				if (summon_md != NULL) {
					summon_md->master_id = src->id;
					if (summon_md->deletetimer != INVALID_TIMER)
						timer->delete(summon_md->deletetimer, mob->timer_delete);
					summon_md->deletetimer = timer->add(timer->gettick() + skill->get_time(skill_id, skill_lv), mob->timer_delete, summon_md->bl.id, 0);
					mob->spawn(summon_md); //Now it is ready for spawning.
					sc_start4(src,&summon_md->bl, SC_MODECHANGE, 100, 1, 0, MD_CANATTACK|MD_AGGRESSIVE, 0, 60000);
				}
			}
			if (hd)
				skill->blockhomun_start(hd, skill_id, skill->get_cooldown(skill_id, skill_lv));
		}
			break;
		case SO_ELEMENTAL_SHIELD:/* somehow its handled outside this switch, so we need a empty case otherwise default would be triggered. */
			break;
		default:
			if (skill->castend_nodamage_id_unknown(src, bl, &skill_id, &skill_lv, &tick, &flag))
				return 1;
			break;
	}

	if(skill_id != SR_CURSEDCIRCLE) {
		struct status_change *sc = status->get_sc(src);
		if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] )//Should only remove after the skill had been casted.
			status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);
	}

	if (dstmd) { //Mob skill event for no damage skills (damage ones are handled in battle_calc_damage) [Skotlex]
		mob->log_damage(dstmd, src, 0); //Log interaction (counts as 'attacker' for the exp bonus)
		mob->skill_event(dstmd, src, tick, MSC_SKILLUSED|(skill_id<<16));
	}

	if( sd && !(flag&1) ) { // ensure that the skill last-cast tick is recorded
		sd->canskill_tick = timer->gettick();

		if( sd->state.arrow_atk ) { // consume arrow on last invocation to this skill.
			battle->consume_ammo(sd, skill_id, skill_lv);
		}
		skill->onskillusage(sd, bl, skill_id, tick);
		// perform skill requirement consumption
		if( skill_id != NC_SELFDESTRUCTION )
			skill->consume_requirement(sd,skill_id,skill_lv,2);
	}

	map->freeblock_unlock();
	return 0;
}

bool skill_castend_nodamage_id_dead_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
    return true;
}

bool skill_castend_nodamage_id_undead_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
    return true;
}

bool skill_castend_nodamage_id_mado_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
    return false;
}

bool skill_castend_nodamage_id_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	ShowWarning("skill_castend_nodamage_id: Unknown skill used:%d\n", *skill_id);
	clif->skill_nodamage(src, bl, *skill_id, *skill_lv, 1);
	map->freeblock_unlock();
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_pos(int tid, int64 tick, int id, intptr_t data)
{
	struct block_list* src = map->id2bl(id);
	struct map_session_data *sd;
	struct unit_data *ud = unit->bl2ud(src);
	struct mob_data *md;

	nullpo_ret(ud);

	sd = BL_CAST(BL_PC , src);
	md = BL_CAST(BL_MOB, src);

	if( src->prev == NULL ) {
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if( ud->skilltimer != tid )
	{
		ShowError("skill_castend_pos: Timer mismatch %d!=%d\n", ud->skilltimer, tid);
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if( sd && ud->skilltimer != INVALID_TIMER && ( pc->checkskill(sd,SA_FREECAST) > 0 || ud->skill_id == LG_EXEEDBREAK ) )
	{// restore original walk speed
		ud->skilltimer = INVALID_TIMER;
		status_calc_bl(&sd->bl, SCB_SPEED|SCB_ASPD);
	}
	ud->skilltimer = INVALID_TIMER;

	do {
		int maxcount;
		if( status->isdead(src) )
			break;

		if( !(src->type&battle_config.skill_reiteration) &&
			skill->get_unit_flag(ud->skill_id)&UF_NOREITERATION &&
			skill->check_unit_range(src,ud->skillx,ud->skilly,ud->skill_id,ud->skill_lv)
		  )
		{
			if (sd) clif->skill_fail(sd,ud->skill_id,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if( src->type&battle_config.skill_nofootset &&
			skill->get_unit_flag(ud->skill_id)&UF_NOFOOTSET &&
			skill->check_unit_range2(src,ud->skillx,ud->skilly,ud->skill_id,ud->skill_lv)
		  )
		{
			if (sd) clif->skill_fail(sd,ud->skill_id,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if( src->type&battle_config.land_skill_limit &&
			(maxcount = skill->get_maxcount(ud->skill_id, ud->skill_lv)) > 0
		  ) {
			int i;
			for(i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i] && maxcount;i++) {
				if(ud->skillunit[i]->skill_id == ud->skill_id)
					maxcount--;
			}
			if( maxcount == 0 )
			{
				if (sd) clif->skill_fail(sd,ud->skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}
		}

		if(tid != INVALID_TIMER) {
			//Avoid double checks on instant cast skills. [Skotlex]
			if (!status->check_skilluse(src, NULL, ud->skill_id, 1))
				break;
			if(battle_config.skill_add_range &&
				!check_distance_blxy(src, ud->skillx, ud->skilly, skill->get_range2(src,ud->skill_id,ud->skill_lv)+battle_config.skill_add_range)) {
				if (sd && battle_config.skill_out_range_consume) //Consume items anyway.
					skill->consume_requirement(sd,ud->skill_id,ud->skill_lv,3);
				break;
			}
		}

		if( sd )
		{
			if( ud->skill_id != AL_WARP && !skill->check_condition_castend(sd, ud->skill_id, ud->skill_lv) ) {
				if( ud->skill_id == SA_LANDPROTECTOR )
					clif->skill_poseffect(&sd->bl,ud->skill_id,ud->skill_lv,sd->bl.x,sd->bl.y,tick);
				break;
			}else
				skill->consume_requirement(sd,ud->skill_id,ud->skill_lv,1);
		}

		if( (src->type == BL_MER || src->type == BL_HOM) && !skill->check_condition_mercenary(src, ud->skill_id, ud->skill_lv, 1) )
			break;

		if(md) {
			md->last_thinktime=tick +MIN_MOBTHINKTIME;
			if(md->skill_idx >= 0 && md->db->skill[md->skill_idx].emotion >= 0)
				clif->emotion(src, md->db->skill[md->skill_idx].emotion);
		}

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %u, ID %d skill castend pos [id =%d, lv=%d, (%d,%d)]\n",
				src->type, src->id, ud->skill_id, ud->skill_lv, ud->skillx, ud->skilly);

		if (ud->walktimer != INVALID_TIMER)
			unit->stop_walking(src, STOPWALKING_FLAG_FIXPOS);

		if( !sd || sd->skillitem != ud->skill_id || skill->get_delay(ud->skill_id,ud->skill_lv) )
			ud->canact_tick = tick + skill->delay_fix(src, ud->skill_id, ud->skill_lv);
		if (sd) { //Cooldown application
			int i, cooldown = skill->get_cooldown(ud->skill_id, ud->skill_lv);
			for (i = 0; i < ARRAYLENGTH(sd->skillcooldown) && sd->skillcooldown[i].id; i++) { // Increases/Decreases cooldown of a skill by item/card bonuses.
				if (sd->skillcooldown[i].id == ud->skill_id){
					cooldown += sd->skillcooldown[i].val;
					break;
				}
			}
			if(cooldown)
				skill->blockpc_start(sd, ud->skill_id, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif->status_change(src, SI_POSTDELAY, 1, skill->delay_fix(src, ud->skill_id, ud->skill_lv), 0, 0, 0);
#if 0
		if (sd) {
			switch (ud->skill_id) {
			case ????:
				sd->canequip_tick = tick + ????;
				break;
			}
		}
#endif // 0
		unit->set_walkdelay(src, tick, battle_config.default_walk_delay+skill->get_walkdelay(ud->skill_id, ud->skill_lv), 1);
		status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);// only normal attack and auto cast skills benefit from its bonuses
		map->freeblock_lock();
		skill->castend_pos2(src,ud->skillx,ud->skilly,ud->skill_id,ud->skill_lv,tick,0);

		if( sd && sd->skillitem != AL_WARP ) // Warp-Portal thru items will clear data in skill_castend_map. [Inkfish]
			sd->skillitem = sd->skillitemlv = 0;

		unit->setdir(src, map->calc_dir(src, ud->skillx, ud->skilly));

		if (ud->skilltimer == INVALID_TIMER) {
			if (md) md->skill_idx = -1;
			else ud->skill_id = 0; //Non mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skill_lv = ud->skillx = ud->skilly = 0;
		}

		map->freeblock_unlock();
		return 1;
	} while(0);

	if( !sd || sd->skillitem != ud->skill_id || skill->get_delay(ud->skill_id,ud->skill_lv) )
		ud->canact_tick = tick;
	ud->skill_id = ud->skill_lv = 0;
	if(sd)
		sd->skillitem = sd->skillitemlv = 0;
	else if(md)
		md->skill_idx  = -1;
	return 0;

}

static int check_npc_chaospanic(struct block_list *bl, va_list args)
{
	const struct npc_data *nd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_NPC);
	nd = BL_UCCAST(BL_NPC, bl);

	if (nd->option&(OPTION_HIDE|OPTION_INVISIBLE) || nd->class_ != WARP_CLASS)
		return 0;

	return 1;
}
/* skill count without self */
static int skill_count_wos(struct block_list *bl,va_list ap) {
	struct block_list* src = va_arg(ap, struct block_list*);
	if( src->id != bl->id ) {
		return 1;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_map (struct map_session_data *sd, uint16 skill_id, const char *mapname) {
	nullpo_ret(sd);

//Simplify skill_failed code.
#define skill_failed(sd) ( (sd)->menuskill_id = (sd)->menuskill_val = 0 )
	if(skill_id != sd->menuskill_id)
		return 0;

	if( sd->bl.prev == NULL || pc_isdead(sd) ) {
		skill_failed(sd);
		return 0;
	}

	if( ( sd->sc.opt1 && sd->sc.opt1 != OPT1_BURNING ) || sd->sc.option&OPTION_HIDE ) {
		skill_failed(sd);
		return 0;
	}
	if(sd->sc.count && (
		sd->sc.data[SC_SILENCE] ||
		sd->sc.data[SC_ROKISWEIL] ||
		sd->sc.data[SC_AUTOCOUNTER] ||
		sd->sc.data[SC_STEELBODY] ||
		(sd->sc.data[SC_DANCING] && skill_id < RK_ENCHANTBLADE && !pc->checkskill(sd, WM_LESSON)) ||
		sd->sc.data[SC_BERSERK] ||
		sd->sc.data[SC_BASILICA] ||
		sd->sc.data[SC_MARIONETTE_MASTER] ||
		sd->sc.data[SC_WHITEIMPRISON] ||
		(sd->sc.data[SC_STASIS] && skill->block_check(&sd->bl, SC_STASIS, skill_id)) ||
		(sd->sc.data[SC_KG_KAGEHUMI] && skill->block_check(&sd->bl, SC_KG_KAGEHUMI, skill_id)) ||
		sd->sc.data[SC_OBLIVIONCURSE] ||
		sd->sc.data[SC__MANHOLE] ||
		(sd->sc.data[SC_VOLCANIC_ASH] && rnd()%2) //50% fail chance under ASH
	 )) {
		skill_failed(sd);
		return 0;
	}

	pc_stop_attack(sd);

	if(battle_config.skill_log && battle_config.skill_log&BL_PC)
		ShowInfo("PC %d skill castend skill =%d map=%s\n",sd->bl.id,skill_id,mapname);

	if(strcmp(mapname,"cancel")==0) {
		skill_failed(sd);
		return 0;
	}

	switch(skill_id) {
		case AL_TELEPORT:
			// The storage window is closed automatically by the client when there's
			// any kind of map change, so we need to restore it automatically
			// issue: 8027
			if(strcmp(mapname,"Random")==0)
				pc->randomwarp(sd,CLR_TELEPORT);
			else if (sd->menuskill_val > 1) //Need lv2 to be able to warp here.
				pc->setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);

			clif->refresh_storagewindow(sd);
			break;

		case AL_WARP:
			{
				const struct point *p[4];
				struct skill_unit_group *group;
				int i, lv, wx, wy;
				int maxcount=0;
				int x,y;
				unsigned short map_index;

				map_index = mapindex->name2id(mapname);
				if(!map_index) { //Given map not found?
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					skill_failed(sd);
					return 0;
				}
				p[0] = &sd->status.save_point;
				p[1] = &sd->status.memo_point[0];
				p[2] = &sd->status.memo_point[1];
				p[3] = &sd->status.memo_point[2];

				if((maxcount = skill->get_maxcount(skill_id, sd->menuskill_val)) > 0) {
					for(i=0;i<MAX_SKILLUNITGROUP && sd->ud.skillunit[i] && maxcount;i++) {
						if(sd->ud.skillunit[i]->skill_id == skill_id)
							maxcount--;
					}
					if(!maxcount) {
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						skill_failed(sd);
						return 0;
					}
				}

				lv = sd->skillitem==skill_id?sd->skillitemlv:pc->checkskill(sd,skill_id);
				wx = sd->menuskill_val>>16;
				wy = sd->menuskill_val&0xffff;

				if( lv <= 0 ) return 0;
				if( lv > 4 ) lv = 4; // crash prevention

				// check if the chosen map exists in the memo list
				ARR_FIND( 0, lv, i, map_index == p[i]->map );
				if( i < lv ) {
					x=p[i]->x;
					y=p[i]->y;
				} else {
					skill_failed(sd);
					return 0;
				}

				if(!skill->check_condition_castend(sd, sd->menuskill_id, lv)) { // This checks versus skill_id/skill_lv...
					skill_failed(sd);
					return 0;
				}

				skill->consume_requirement(sd,sd->menuskill_id,lv,2);
				sd->skillitem = sd->skillitemlv = 0; // Clear data that's skipped in 'skill_castend_pos' [Inkfish]

				if((group=skill->unitsetting(&sd->bl,skill_id,lv,wx,wy,0))==NULL) {
					skill_failed(sd);
					return 0;
				}

				group->val1 = (group->val1<<16)|(short)0;
				// record the destination coordinates
				group->val2 = (x<<16)|y;
				group->val3 = map_index;
			}
			break;
	}

	sd->menuskill_id = sd->menuskill_val = 0;
	return 0;
#undef skill_failed
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_pos2(struct block_list* src, int x, int y, uint16 skill_id, uint16 skill_lv, int64 tick, int flag) {
	struct map_session_data* sd;
	struct status_change* sc;
	struct status_change_entry *sce;
	struct skill_unit_group* sg;
	enum sc_type type;
	int r;

	//if(skill_lv <= 0) return 0;
	if(skill_id > 0 && !skill_lv) return 0; // [Celest]

	nullpo_ret(src);

	if(status->isdead(src))
		return 0;

	sd = BL_CAST(BL_PC, src);

	sc = status->get_sc(src);
	type = status->skill2sc(skill_id);
	sce = (sc != NULL && type != SC_NONE) ? sc->data[type] : NULL;

	switch (skill_id) { //Skill effect.
		case WZ_METEOR:
		case MO_BODYRELOCATION:
		case CR_CULTIVATION:
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
		case SC_ESCAPE:
			break; //Effect is displayed on respective switch case.
		default:
			skill->castend_pos2_effect_unknown(src, &x, &y, &skill_id, &skill_lv, &tick, &flag);
			break;
	}

	// SC_MAGICPOWER needs to switch states before any damage is actually dealt
	skill->toggle_magicpower(src, skill_id);

	switch(skill_id) {
		case PR_BENEDICTIO:
			r = skill->get_splash(skill_id, skill_lv);
			skill->area_temp[1] = src->id;
			map->foreachinarea(skill->area_sub,
			                   src->m, x-r, y-r, x+r, y+r, BL_PC,
			                   src, skill_id, skill_lv, tick, flag|BCT_ALL|1,
			                   skill->castend_nodamage_id);
			map->foreachinarea(skill->area_sub,
			                   src->m, x-r, y-r, x+r, y+r, BL_CHAR,
			                   src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1,
			                   skill->castend_damage_id);
			break;

		case BS_HAMMERFALL:
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinarea(skill->area_sub,
			                   src->m, x-r, y-r, x+r, y+r, BL_CHAR,
			                   src, skill_id, skill_lv, tick, flag|BCT_ENEMY|2,
			                   skill->castend_nodamage_id);
			break;

		case HT_DETECTING:
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinarea(status->change_timer_sub,
			                   src->m, x-r, y-r, x+r,y+r,BL_CHAR,
			                   src,NULL,SC_SIGHT,tick);
			if(battle_config.traps_setting&1)
			map->foreachinarea(skill_reveal_trap,
			                   src->m, x-r, y-r, x+r, y+r, BL_SKILL);
			break;

		case SR_RIDEINLIGHTNING:
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinarea(skill->area_sub, src->m, x-r, y-r, x+r, y+r, BL_CHAR,
			                   src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_damage_id);
			break;

		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
			//Does not consumes if the skill is already active. [Skotlex]
			if ((sg= skill->locate_element_field(src)) != NULL && ( sg->skill_id == SA_VOLCANO || sg->skill_id == SA_DELUGE || sg->skill_id == SA_VIOLENTGALE ))
			{
				if (sg->limit - DIFF_TICK(timer->gettick(), sg->tick) > 0) {
					skill->unitsetting(src,skill_id,skill_lv,x,y,0);
					return 0; // not to consume items
				} else
					sg->limit = 0; //Disable it.
			}
			skill->unitsetting(src,skill_id,skill_lv,x,y,0);
			break;

		case SC_CHAOSPANIC:
		case SC_MAELSTROM:
			if (sd && map->foreachinarea(&check_npc_chaospanic,src->m, x-3, y-3, x+3, y+3, BL_NPC) > 0 ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				break;
			}

		case MG_SAFETYWALL:
		{
			int alive = 1;
			if ( map->foreachincell(skill->cell_overlap, src->m, x, y, BL_SKILL, skill_id, &alive, src) ) {
				skill->unitsetting(src, skill_id, skill_lv, x, y, 0);
				return 0; // Don't consume gems if cast on LP
			}
		}
		case MG_FIREWALL:
		case MG_THUNDERSTORM:

		case AL_PNEUMA:
		case WZ_FIREPILLAR:
		case WZ_QUAGMIRE:
		case WZ_VERMILION:
		case WZ_STORMGUST:
		case WZ_HEAVENDRIVE:
		case PR_SANCTUARY:
		case PR_MAGNUS:
		case CR_GRANDCROSS:
		case NPC_GRANDDARKNESS:
		case HT_SKIDTRAP:
		case MA_SKIDTRAP:
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_ANKLESNARE:
		case HT_SHOCKWAVE:
		case HT_SANDMAN:
		case MA_SANDMAN:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
		case AS_VENOMDUST:
		case AM_DEMONSTRATION:
		case PF_FOGWALL:
		case PF_SPIDERWEB:
		case HT_TALKIEBOX:
		case WE_CALLPARTNER:
		case WE_CALLPARENT:
		case WE_CALLBABY:
		case AC_SHOWER: //Ground-placed skill implementation.
		case MA_SHOWER:
		case SA_LANDPROTECTOR:
		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
		case BA_DISSONANCE:
		case BA_POEMBRAGI:
		case BA_WHISTLE:
		case BA_ASSASSINCROSS:
		case BA_APPLEIDUN:
		case DC_UGLYDANCE:
		case DC_HUMMING:
		case DC_DONTFORGETME:
		case DC_FORTUNEKISS:
		case DC_SERVICEFORYOU:
		case CG_MOONLIT:
		case GS_DESPERADO:
		case NJ_KAENSIN:
		case NJ_BAKUENRYU:
		case NJ_SUITON:
		case NJ_HYOUSYOURAKU:
		case NJ_RAIGEKISAI:
		case NJ_KAMAITACHI:
	#ifdef RENEWAL
		case NJ_HUUMA:
	#endif
		case NPC_EARTHQUAKE:
		case NPC_EVILLAND:
		case WL_COMET:
		case RA_ELECTRICSHOCKER:
		case RA_CLUSTERBOMB:
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
		case SC_MANHOLE:
		case SC_DIMENSIONDOOR:
		case SC_BLOODYLUST:
		case WM_REVERBERATION:
		case WM_SEVERE_RAINSTORM:
		case WM_POEMOFNETHERWORLD:
		case SO_PSYCHIC_WAVE:
		case SO_VACUUM_EXTREME:
		case GN_WALLOFTHORN:
		case GN_THORNS_TRAP:
		case GN_DEMONIC_FIRE:
		case GN_HELLS_PLANT:
		case GN_FIRE_EXPANSION_SMOKE_POWDER:
		case GN_FIRE_EXPANSION_TEAR_GAS:
		case SO_EARTHGRAVE:
		case SO_DIAMONDDUST:
		case SO_FIRE_INSIGNIA:
		case SO_WATER_INSIGNIA:
		case SO_WIND_INSIGNIA:
		case SO_EARTH_INSIGNIA:
		case KO_HUUMARANKA:
		case KO_MUCHANAGE:
		case KO_BAKURETSU:
		case KO_ZENKAI:
		case MH_LAVA_SLIDE:
		case MH_VOLCANIC_ASH:
		case MH_POISON_MIST:
		case MH_STEINWAND:
		case NC_MAGMA_ERUPTION:
		case SO_ELEMENTAL_SHIELD:
		case RL_B_TRAP:
		case MH_XENO_SLASHER:
			flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
		case GS_GROUNDDRIFT: //Ammo should be deleted right away.
			if ( skill_id == WM_SEVERE_RAINSTORM )
				sc_start(src,src,SC_NO_SWITCH_EQUIP,100,0,skill->get_time(skill_id,skill_lv));
			skill->unitsetting(src,skill_id,skill_lv,x,y,0);
			break;
		case WZ_ICEWALL:
			flag |= 1;
			if( skill->unitsetting(src,skill_id,skill_lv,x,y,0) )
				map->list[src->m].setcell(src->m, x, y, CELL_NOICEWALL, true);
			break;
		case RG_GRAFFITI:
			skill->clear_unitgroup(src);
			skill->unitsetting(src,skill_id,skill_lv,x,y,0);
			flag|=1;
			break;
		case HP_BASILICA:
			if( sc && sc->data[SC_BASILICA] )
				status_change_end(src, SC_BASILICA, INVALID_TIMER); // Cancel Basilica
			else { // Create Basilica. Start SC on caster. Unit timer start SC on others.
				if( map->foreachinrange(skill_count_wos, src, 2, BL_MOB|BL_PC, src) ) {
					if( sd )
						clif->skill_fail(sd,skill_id,USESKILL_FAIL,0);
					return 1;
				}

				skill->clear_unitgroup(src);
				if( skill->unitsetting(src,skill_id,skill_lv,x,y,0) )
					sc_start4(src,src,type,100,skill_lv,0,0,src->id,skill->get_time(skill_id,skill_lv));
				flag|=1;
			}
			break;
		case CG_HERMODE:
			skill->clear_unitgroup(src);
			if ((sg = skill->unitsetting(src,skill_id,skill_lv,x,y,0)))
				sc_start4(src,src,SC_DANCING,100,
					skill_id,0,skill_lv,sg->group_id,skill->get_time(skill_id,skill_lv));
			flag|=1;
			break;
		case RG_CLEANER: // [Valaris]
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinarea(skill->graffitiremover,src->m,x-r,y-r,x+r,y+r,BL_SKILL);
			break;

		case SO_WARMER:
			flag|= 8;
			/* Fall through */
		case SO_CLOUD_KILL:
			skill->unitsetting(src,skill_id,skill_lv,x,y,0);
			break;

		case WZ_METEOR:
			{
				int area = skill->get_splash(skill_id, skill_lv);
				short tmpx = 0, tmpy = 0, x1 = 0, y1 = 0;
				int i;

				for( i = 0; i < 2 + (skill_lv>>1); i++ ) {
					// Creates a random Cell in the Splash Area
					tmpx = x - area + rnd()%(area * 2 + 1);
					tmpy = y - area + rnd()%(area * 2 + 1);

					if (i == 0 && path->search_long(NULL, src, src->m, src->x, src->y, tmpx, tmpy, CELL_CHKWALL)
						&& !map->getcell(src->m, src, tmpx, tmpy, CELL_CHKLANDPROTECTOR))
						clif->skill_poseffect(src,skill_id,skill_lv,tmpx,tmpy,tick);

					if( i > 0 )
						skill->addtimerskill(src,tick+i*1000,0,tmpx,tmpy,skill_id,skill_lv,(x1<<16)|y1,0);

					x1 = tmpx;
					y1 = tmpy;
				}

				skill->addtimerskill(src,tick+i*1000,0,tmpx,tmpy,skill_id,skill_lv,-1,0);
			}
			break;

		case AL_WARP:
			if(sd)
			{
				clif->skill_warppoint(sd, skill_id, skill_lv, sd->status.save_point.map,
					(skill_lv >= 2) ? sd->status.memo_point[0].map : 0,
					(skill_lv >= 3) ? sd->status.memo_point[1].map : 0,
					(skill_lv >= 4) ? sd->status.memo_point[2].map : 0
				);
			}
			if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] ) //Should only remove after the skill has been casted.
				status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);
			return 0; // not to consume item.

		case MO_BODYRELOCATION:
			if (unit->movepos(src, x, y, 1, 1)) {
	#if PACKETVER >= 20111005
				clif->snap(src, src->x, src->y);
	#else
				clif->skill_poseffect(src,skill_id,skill_lv,src->x,src->y,tick);
	#endif
				if (sd)
					skill->blockpc_start (sd, MO_EXTREMITYFIST, 2000);
			}
			break;
		case NJ_SHADOWJUMP:
			if( !map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground ) { //You don't move on GVG grounds.
				unit->movepos(src, x, y, 1, 0);
				clif->slide(src,x,y);
			}
			status_change_end(src, SC_HIDING, INVALID_TIMER);
			break;
		case AM_SPHEREMINE:
		case AM_CANNIBALIZE:
			{
				struct mob_data *md;
				int class_ = 0;
				if (skill_id == AM_SPHEREMINE) {
					class_ = MOBID_MARINE_SPHERE;
				} else {
					Assert_retb(skill_lv > 0 && skill_lv <= 5);
					switch (skill_lv) {
					case 1: class_ = MOBID_G_MANDRAGORA; break;
					case 2: class_ = MOBID_G_HYDRA;      break;
					case 3: class_ = MOBID_G_FLORA;      break;
					case 4: class_ = MOBID_G_PARASITE;   break;
					case 5: class_ = MOBID_G_GEOGRAPHER; break;
					}
				}

				// Correct info, don't change any of this! [Celest]
				md = mob->once_spawn_sub(src, src->m, x, y, clif->get_bl_name(src), class_, "", SZ_SMALL, AI_NONE);
				if (md) {
					md->master_id = src->id;
					md->special_state.ai = (skill_id == AM_SPHEREMINE) ? AI_SPHERE : AI_FLORA;
					if( md->deletetimer != INVALID_TIMER )
						timer->delete(md->deletetimer, mob->timer_delete);
					md->deletetimer = timer->add(timer->gettick() + skill->get_time(skill_id,skill_lv), mob->timer_delete, md->bl.id, 0);
					mob->spawn (md); //Now it is ready for spawning.
				}
			}
			break;

		// Slim Pitcher [Celest]
		case CR_SLIMPITCHER:
			if (sd) {
				int i = skill_lv%11 - 1;
				int j = pc->search_inventory(sd,skill->dbs->db[skill_id].itemid[i]);
				if (j == INDEX_NOT_FOUND || skill->dbs->db[skill_id].itemid[i] <= 0
				 || sd->inventory_data[j] == NULL || sd->status.inventory[j].amount < skill->dbs->db[skill_id].amount[i]
				) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 1;
				}
				script->potion_flag = 1;
				script->potion_hp = 0;
				script->potion_sp = 0;
				script->run_use_script(sd, sd->inventory_data[j], 0);
				script->potion_flag = 0;
				//Apply skill bonuses
				i = pc->checkskill(sd,CR_SLIMPITCHER)*10
					+ pc->checkskill(sd,AM_POTIONPITCHER)*10
					+ pc->checkskill(sd,AM_LEARNINGPOTION)*5
					+ pc->skillheal_bonus(sd, skill_id);

				script->potion_hp = script->potion_hp * (100+i)/100;
				script->potion_sp = script->potion_sp * (100+i)/100;

				if(script->potion_hp > 0 || script->potion_sp > 0) {
					i = skill->get_splash(skill_id, skill_lv);
					map->foreachinarea(skill->area_sub,
					                   src->m,x-i,y-i,x+i,y+i,BL_CHAR,
					                   src,skill_id,skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					                   skill->castend_nodamage_id);
				}
			} else {
				int i = skill_lv%11 - 1;
				struct item_data *item;
				i = skill->dbs->db[skill_id].itemid[i];
				item = itemdb->search(i);
				script->potion_flag = 1;
				script->potion_hp = 0;
				script->potion_sp = 0;
				script->run(item->script,0,src->id,0);
				script->potion_flag = 0;
				i = skill->get_max(CR_SLIMPITCHER)*10;

				script->potion_hp = script->potion_hp * (100+i)/100;
				script->potion_sp = script->potion_sp * (100+i)/100;

				if(script->potion_hp > 0 || script->potion_sp > 0) {
					i = skill->get_splash(skill_id, skill_lv);
					map->foreachinarea(skill->area_sub,
					                   src->m,x-i,y-i,x+i,y+i,BL_CHAR,
					                   src,skill_id,skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					                   skill->castend_nodamage_id);
				}
			}
			break;

		case HW_GANBANTEIN:
			if (rnd()%100 < 80) {
				int dummy = 1;
				clif->skill_poseffect(src,skill_id,skill_lv,x,y,tick);
				r = skill->get_splash(skill_id, skill_lv);
				map->foreachinarea(skill->cell_overlap, src->m, x-r, y-r, x+r, y+r, BL_SKILL, HW_GANBANTEIN, &dummy, src);
			} else {
				if (sd) clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;

		case HW_GRAVITATION:
			if ((sg = skill->unitsetting(src,skill_id,skill_lv,x,y,0)))
				sc_start4(src,src,type,100,skill_lv,0,BCT_SELF,sg->group_id,skill->get_time(skill_id,skill_lv));
			flag|=1;
			break;

		// Plant Cultivation [Celest]
		case CR_CULTIVATION:
			if (sd) {
				if( map->count_oncell(src->m,x,y,BL_CHAR,0) > 0 ) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 1;
				}
				clif->skill_poseffect(src,skill_id,skill_lv,x,y,tick);
				if (rnd()%100 < 50) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				} else {
					int mob_id = skill_lv < 2 ? MOBID_BLACK_MUSHROOM + rnd()%2 : MOBID_RED_PLANT + rnd()%6;
					struct mob_data *md = mob->once_spawn_sub(src, src->m, x, y, "--ja--", mob_id, "", SZ_SMALL, AI_NONE);
					int i;
					if (md == NULL)
						break;
					if ((i = skill->get_time(skill_id, skill_lv)) > 0)
					{
						if( md->deletetimer != INVALID_TIMER )
							timer->delete(md->deletetimer, mob->timer_delete);
						md->deletetimer = timer->add(tick + i, mob->timer_delete, md->bl.id, 0);
					}
					mob->spawn (md);
				}
			}
			break;

		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM:
			skill->clear_unitgroup(src);
			if ((sg = skill->unitsetting(src,skill_id,skill_lv,src->x,src->y,0)))
				sc_start4(src,src,type,100,skill_lv,0,0,sg->group_id,skill->get_time(skill_id,skill_lv));
			flag|=1;
			break;

		case PA_GOSPEL:
			if (sce && sce->val4 == BCT_SELF) {
				status_change_end(src, SC_GOSPEL, INVALID_TIMER);
				return 0;
			} else {
				sg = skill->unitsetting(src,skill_id,skill_lv,src->x,src->y,0);
				if (!sg) break;
				if (sce)
					status_change_end(src, type, INVALID_TIMER); //Was under someone else's Gospel. [Skotlex]
				status->change_clear_buffs(src,3);
				sc_start4(src,src,type,100,skill_lv,0,sg->group_id,BCT_SELF,skill->get_time(skill_id,skill_lv));
				clif->skill_poseffect(src, skill_id, skill_lv, 0, 0, tick); // PA_GOSPEL music packet
			}
			break;
		case NJ_TATAMIGAESHI:
			if (skill->unitsetting(src,skill_id,skill_lv,src->x,src->y,0))
				sc_start(src,src,type,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case AM_RESURRECTHOMUN: // [orn]
			if (sd) {
				if (!homun->ressurect(sd, 20*skill_lv, x, y)) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					break;
				}
			}
			break;

		case RK_WINDCUTTER:
			clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			/* Fall through */
		case NC_COLDSLOWER:
		case RK_DRAGONBREATH:
		case RK_DRAGONBREATH_WATER:
			r = skill->get_splash(skill_id,skill_lv);
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,splash_target(src),
			                   src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			break;
		case WM_GREAT_ECHO:
		case WM_SOUND_OF_DESTRUCTION:
			r = skill->get_splash(skill_id,skill_lv);
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			break;

		case WM_LULLABY_DEEPSLEEP:
			r = skill->get_splash(skill_id,skill_lv);
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,BL_CHAR,
				src,skill_id,skill_lv,tick,flag|BCT_ALL|1,skill->castend_damage_id);
			break;

		case WM_VOICEOFSIREN:
			r = skill->get_splash(skill_id,skill_lv);
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,BL_CHAR,
				src,skill_id,skill_lv,tick,flag|BCT_ALL|1,skill->castend_damage_id);
			break;
		case SO_ARRULLO:
			r = skill->get_splash(skill_id,skill_lv);
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,splash_target(src),
			                   src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
			break;
		/**
		 * Guilotine Cross
		 **/
		case GC_POISONSMOKE:
			if( !(sc && sc->data[SC_POISONINGWEAPON]) ) {
				if( sd )
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_GC_POISONINGWEAPON,0);
				return 0;
			}
			clif->skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,BDT_SKILL);
			skill->unitsetting(src, skill_id, skill_lv, x, y, flag);
			//status_change_end(src,SC_POISONINGWEAPON,INVALID_TIMER); // 08/31/2011 - When using poison smoke, you no longer lose the poisoning weapon effect.
			break;
		/**
		 * Arch Bishop
		 **/
		case AB_EPICLESIS:
			if( (sg = skill->unitsetting(src, skill_id, skill_lv, x, y, 0)) ) {
				r = skill->get_unit_range(skill_id, skill_lv);
				map->foreachinarea(skill->area_sub, src->m, x - r, y - r, x + r, y + r, BL_CHAR, src, ALL_RESURRECTION, 1, tick, flag|BCT_NOENEMY|1,skill->castend_nodamage_id);
			}
			break;

		case WL_EARTHSTRAIN:
			{
				int i, wave = skill_lv + 4, dir = map->calc_dir(src,x,y);
				int sx = x = src->x, sy = y = src->y; // Store first caster's location to avoid glitch on unit setting

				for( i = 1; i <= wave; i++ )
				{
					switch( dir ){
						case 0: case 1: case 7: sy = y + i; break;
						case 3: case 4: case 5: sy = y - i; break;
						case 2: sx = x - i; break;
						case 6: sx = x + i; break;
					}
					skill->addtimerskill(src,timer->gettick() + (140 * i),0,sx,sy,skill_id,skill_lv,dir,flag&2);
				}
			}
			break;
		/**
		 * Ranger
		 **/
		case RA_DETONATOR:
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinarea(skill->detonator, src->m, x-r, y-r, x+r, y+r, BL_SKILL, src);
			clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			break;
		/**
		 * Mechanic
		 **/
		case NC_NEUTRALBARRIER:
		case NC_STEALTHFIELD:
			skill->clear_unitgroup(src); // To remove previous skills - cannot used combined
			if( (sg = skill->unitsetting(src,skill_id,skill_lv,src->x,src->y,0)) != NULL ) {
				sc_start2(src,src,skill_id == NC_NEUTRALBARRIER ? SC_NEUTRALBARRIER_MASTER : SC_STEALTHFIELD_MASTER,100,skill_lv,sg->group_id,skill->get_time(skill_id,skill_lv));
				if( sd ) pc->overheat(sd,1);
			}
			break;

		case NC_SILVERSNIPER:
			{
				struct mob_data *md = mob->once_spawn_sub(src, src->m, x, y, clif->get_bl_name(src), MOBID_SILVERSNIPER, "", SZ_SMALL, AI_NONE);
				if (md) {
					md->master_id = src->id;
					md->special_state.ai = AI_FLORA;
					if( md->deletetimer != INVALID_TIMER )
						timer->delete(md->deletetimer, mob->timer_delete);
					md->deletetimer = timer->add(timer->gettick() + skill->get_time(skill_id, skill_lv), mob->timer_delete, md->bl.id, 0);
					mob->spawn( md );
				}
			}
			break;

		case NC_MAGICDECOY:
			if( sd ) clif->magicdecoy_list(sd,skill_lv,x,y);
			break;

		case SC_FEINTBOMB:
			skill->unitsetting(src, skill_id, skill_lv, x, y, 0); // Set bomb on current Position
			clif->skill_nodamage(src, src, skill_id, skill_lv, 1);
			if( skill->blown(src, src, 3 * skill_lv, unit->getdir(src), 0) && sc) {
				sc_start(src, src, SC__FEINTBOMB_MASTER, 100, 0, skill->get_unit_interval(SC_FEINTBOMB));
			}
			break;

		case SC_ESCAPE:
			clif->skill_nodamage(src,src,skill_id,-1,1);
			skill->unitsetting(src,HT_ANKLESNARE,skill_lv,x,y,2);
			skill->addtimerskill(src,tick,src->id,0,0,skill_id,skill_lv,0,0);
			break;

		case LG_OVERBRAND:
			skill->area_temp[1] = 0;
			map->foreachinpath(skill->attack_area,src->m,src->x,src->y,x,y,1,5,BL_CHAR,
				skill->get_type(skill_id),src,src,skill_id,skill_lv,tick,flag,BCT_ENEMY);
			skill->addtimerskill(src,timer->gettick() + status_get_amotion(src), 0, x, y, LG_OVERBRAND_BRANDISH, skill_lv, 0, flag);
			break;

		case LG_BANDING:
			if( sc && sc->data[SC_BANDING] )
				status_change_end(src,SC_BANDING,INVALID_TIMER);
			else if( (sg = skill->unitsetting(src,skill_id,skill_lv,src->x,src->y,0)) != NULL ) {
				sc_start4(src,src,SC_BANDING,100,skill_lv,0,0,sg->group_id,skill->get_time(skill_id,skill_lv));
				if( sd ) pc->banding(sd,skill_lv);
			}
			clif->skill_nodamage(src,src,skill_id,skill_lv,1);
			break;

		case LG_RAYOFGENESIS:
			if( status->charge(src,status_get_max_hp(src)*3*skill_lv / 100,0) ) {
				r = skill->get_splash(skill_id,skill_lv);
				map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,splash_target(src),
					src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			} else if( sd )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL,0);
			break;

		case WM_DOMINION_IMPULSE:
			r = skill->get_splash(skill_id, skill_lv);
			map->foreachinarea( skill->activate_reverberation,src->m, x-r, y-r, x+r,y+r,BL_SKILL);
			break;

		case GN_CRAZYWEED:
			{
				int area = skill->get_splash(skill_id, skill_lv);

				for( r = 0; r < 3 + (skill_lv>>1); r++ ) {
					// Creates a random Cell in the Splash Area
					int tmpx = x - area + rnd()%(area * 2 + 1);
					int tmpy = y - area + rnd()%(area * 2 + 1);

					skill->addtimerskill(src,tick+r*250,0,tmpx,tmpy,GN_CRAZYWEED_ATK,skill_lv,-1,0);
				}
			}
			break;

		case GN_FIRE_EXPANSION: {
			int i;
			int aciddemocast = 5;//If player doesent know Acid Demonstration or knows level 5 or lower, effect 5 will cast level 5 Acid Demo.
			struct unit_data *ud = unit->bl2ud(src);

			if( !ud ) break;

			r = skill->get_unit_range(GN_DEMONIC_FIRE, skill_lv);

			for (i = 0; i < MAX_SKILLUNITGROUP && ud->skillunit[i]; i++) {
				if (ud->skillunit[i]->skill_id != GN_DEMONIC_FIRE)
					continue;
				// FIXME: Code after this point assumes that the group has one and only one unit, regardless of what the skill_unit_db says.
				if (ud->skillunit[i]->unit.count != 1)
					continue;
				if (distance_xy(x, y, ud->skillunit[i]->unit.data[0].bl.x, ud->skillunit[i]->unit.data[0].bl.y) < r) {
					switch (skill_lv) {
						case 3:
							ud->skillunit[i]->unit_id = UNT_FIRE_EXPANSION_SMOKE_POWDER;
							clif->changetraplook(&ud->skillunit[i]->unit.data[0].bl, UNT_FIRE_EXPANSION_SMOKE_POWDER);
							break;
						case 4:
							ud->skillunit[i]->unit_id = UNT_FIRE_EXPANSION_TEAR_GAS;
							clif->changetraplook(&ud->skillunit[i]->unit.data[0].bl, UNT_FIRE_EXPANSION_TEAR_GAS);
							break;
						case 5:// If player knows a level of Acid Demonstration greater then 5, that level will be casted.
							if ( pc->checkskill(sd, CR_ACIDDEMONSTRATION) > 5 )
								aciddemocast = pc->checkskill(sd, CR_ACIDDEMONSTRATION);
							map->foreachinarea(skill->area_sub, src->m,
							                   ud->skillunit[i]->unit.data[0].bl.x - 2, ud->skillunit[i]->unit.data[0].bl.y - 2,
							                   ud->skillunit[i]->unit.data[0].bl.x + 2, ud->skillunit[i]->unit.data[0].bl.y + 2, BL_CHAR,
							                   src, CR_ACIDDEMONSTRATION, aciddemocast, tick, flag|BCT_ENEMY|1|SD_LEVEL, skill->castend_damage_id);
							skill->delunit(&ud->skillunit[i]->unit.data[0]);
							break;
						default:
							ud->skillunit[i]->unit.data[0].val2 = skill_lv;
							ud->skillunit[i]->val2 = skill_lv;
							break;
						}
					}
				}
			}
			break;

		case SO_FIREWALK:
		case SO_ELECTRICWALK:
			if( sce )
				status_change_end(src,type,INVALID_TIMER);
			clif->skill_nodamage(src, src ,skill_id, skill_lv,
								sc_start2(src,src, type, 100, skill_id, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;

		case KO_MAKIBISHI:
		{
			int i;
			for( i = 0; i < (skill_lv+2); i++ ) {
				x = src->x - 1 + rnd()%3;
				y = src->y - 1 + rnd()%3;
				skill->unitsetting(src,skill_id,skill_lv,x,y,0);
			}
		}
			break;

		default:
			if (skill->castend_pos2_unknown(src, &x, &y, &skill_id, &skill_lv, &tick, &flag))
				return 1;
			break;
	}

	if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);

	if( sd ) {// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = timer->gettick();

		if( sd->state.arrow_atk && !(flag&1) ) {
			// consume arrow if this is a ground skill
			battle->consume_ammo(sd, skill_id, skill_lv);
		}

		// perform skill requirement consumption
		skill->consume_requirement(sd,skill_id,skill_lv,2);
	}

	return 0;
}

void skill_castend_pos2_effect_unknown(struct block_list* src, int *x, int *y, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag) {
	if (skill->get_inf(*skill_id) & INF_SELF_SKILL)
		clif->skill_nodamage(src, src, *skill_id, *skill_lv, 1);
	else
		clif->skill_poseffect(src, *skill_id, *skill_lv, *x, *y, *tick);
}

bool skill_castend_pos2_unknown(struct block_list* src, int *x, int *y, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag) {
	ShowWarning("skill_castend_pos2: Unknown skill used:%d\n", *skill_id);
	return true;
}

/// transforms 'target' skill unit into dissonance (if conditions are met)
int skill_dance_overlap_sub(struct block_list *bl, va_list ap)
{
	struct skill_unit *target = NULL;
	struct skill_unit *src = va_arg(ap, struct skill_unit*);
	int flag = va_arg(ap, int);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_SKILL);
	target = BL_UCAST(BL_SKILL, bl);

	if (src == target)
		return 0;
	if (!target->group || !(target->group->state.song_dance&0x1))
		return 0;
	if (!(target->val2 & src->val2 & ~UF_ENSEMBLE)) //They don't match (song + dance) is valid.
		return 0;

	if (flag) //Set dissonance
		target->val2 |= UF_ENSEMBLE; //Add ensemble to signal this unit is overlapping.
	else //Remove dissonance
		target->val2 &= ~UF_ENSEMBLE;

	clif->getareachar_skillunit(&target->bl,target,AREA); //Update look of affected cell.

	return 1;
}

//Does the song/dance overlapping -> dissonance check. [Skotlex]
//When flag is 0, this unit is about to be removed, cancel the dissonance effect
//When 1, this unit has been positioned, so start the cancel effect.
int skill_dance_overlap(struct skill_unit* su, int flag) {
	if (!su || !su->group || !(su->group->state.song_dance&0x1))
		return 0;

	if (su->val1 != su->group->skill_id) {
		//Reset state
		su->val1 = su->group->skill_id;
		su->val2 &= ~UF_ENSEMBLE;
	}

	return map->foreachincell(skill->dance_overlap_sub, su->bl.m,su->bl.x,su->bl.y,BL_SKILL, su,flag);
}

/**
 * Converts this group information so that it is handled as a Dissonance or Ugly Dance cell.
 * This function is safe to call even when the unit or the group were freed by other function
 * previously.
 * @param su Skill unit data (from BA_DISSONANCE or DC_UGLYDANCE)
 * @param flag 0 Convert
 * @param flag 1 Revert
 * @retval true success
 **/
bool skill_dance_switch(struct skill_unit* su, int flag) {
	static int prevflag = 1;  // by default the backup is empty
	static struct skill_unit_group backup;
	struct skill_unit_group* group;

	if( su == NULL || (group = su->group) == NULL )
		return false;

	// val2&UF_ENSEMBLE is a hack to indicate dissonance
	if ( !(group->state.song_dance&0x1 && su->val2&UF_ENSEMBLE) )
		return false;

	if( flag == prevflag ) {
		// protection against attempts to read an empty backup / write to a full backup
		ShowError("skill_dance_switch: Attempted to %s (skill_id=%d, skill_lv=%d, src_id=%d).\n",
			flag ? "read an empty backup" : "write to a full backup",
			group->skill_id, group->skill_lv, group->src_id);
		return false;
	}
	prevflag = flag;

	if (!flag) {
		//Transform
		uint16 skill_id = (su->val2&UF_SONG) ? BA_DISSONANCE : DC_UGLYDANCE;

		// backup
		backup.skill_id    = group->skill_id;
		backup.skill_lv    = group->skill_lv;
		backup.unit_id     = group->unit_id;
		backup.target_flag = group->target_flag;
		backup.bl_flag     = group->bl_flag;
		backup.interval    = group->interval;

		// replace
		group->skill_id    = skill_id;
		group->skill_lv    = 1;
		group->unit_id     = skill->get_unit_id(skill_id,0);
		group->target_flag = skill->get_unit_target(skill_id);
		group->bl_flag     = skill->get_unit_bl_target(skill_id);
		group->interval    = skill->get_unit_interval(skill_id);
	} else {
		//Restore
		group->skill_id    = backup.skill_id;
		group->skill_lv    = backup.skill_lv;
		group->unit_id     = backup.unit_id;
		group->target_flag = backup.target_flag;
		group->bl_flag     = backup.bl_flag;
		group->interval    = backup.interval;
	}

	return true;
}
/*==========================================
 * Initializes and sets a ground skill.
 * flag&1 is used to determine when the skill 'morphs' (Warp portal becomes active, or Fire Pillar becomes active)
 *------------------------------------------*/
struct skill_unit_group* skill_unitsetting(struct block_list *src, uint16 skill_id, uint16 skill_lv, int16 x, int16 y, int flag) {
	struct skill_unit_group *group;
	int i,limit,val1=0,val2=0,val3=0;
	int target,interval,range,unit_flag,req_item=0;
	struct s_skill_unit_layout *layout;
	struct map_session_data *sd;
	struct status_data *st;
	struct status_change *sc;
	int active_flag=1;
	int subunt=0;

	nullpo_retr(NULL, src);

	limit = skill->get_time(skill_id,skill_lv);
	range = skill->get_unit_range(skill_id,skill_lv);
	interval = skill->get_unit_interval(skill_id);
	target = skill->get_unit_target(skill_id);
	unit_flag = skill->get_unit_flag(skill_id);
	layout = skill->get_unit_layout(skill_id,skill_lv,src,x,y);

	if( map->list[src->m].unit_count ) {
		ARR_FIND(0, map->list[src->m].unit_count, i, map->list[src->m].units[i]->skill_id == skill_id );

		if( i < map->list[src->m].unit_count ) {
			limit = limit * map->list[src->m].units[i]->modifier / 100;
		}
	}

	sd = BL_CAST(BL_PC, src);
	st = status->get_status_data(src);
	sc = status->get_sc(src); // for traps, firewall and fogwall - celest

	switch( skill_id ) {
		case SO_ELEMENTAL_SHIELD:
			val2 = 300 * skill_lv + 65 * (st->int_ + status->get_lv(src)) + st->max_sp;
			break;
		case MH_STEINWAND:
			val2 = 4 + skill_lv; //nb of attack blocked
			break;
		case MG_SAFETYWALL:
		#ifdef RENEWAL
			/**
			 * According to data provided in RE, SW life is equal to 3 times caster's health
			 **/
			val2 = status_get_max_hp(src) * 3;
			val3 = skill_lv+1;
		#else
			val2 = skill_lv+1;
		#endif
			break;
		case MG_FIREWALL:
			if(sc && sc->data[SC_VIOLENTGALE])
				limit = limit*3/2;
			val2=4+skill_lv;
			break;

		case AL_WARP:
			val1=skill_lv+6;
			if(!(flag&1)) {
				limit=2000;
			} else { // previous implementation (not used anymore)
				//Warp Portal morphing to active mode, extract relevant data from src. [Skotlex]
				struct skill_unit *su = BL_CAST(BL_SKILL, src);
				if (su == NULL)
					return NULL;
				group = su->group;
				src = map->id2bl(group->src_id);
				if (src == NULL)
					return NULL;
				val2 = group->val2; //Copy the (x,y) position you warp to
				val3 = group->val3; //as well as the mapindex to warp to.
			}
			break;
		case HP_BASILICA:
			val1 = src->id; // Store caster id.
			break;

		case PR_SANCTUARY:
		case NPC_EVILLAND:
			val1=(skill_lv+3)*2;
			break;

		case WZ_FIREPILLAR:
			if (map->getcell(src->m, src, x, y, CELL_CHKLANDPROTECTOR))
				return NULL;
			if((flag&1)!=0)
				limit=1000;
			val1=skill_lv+2;
			break;
		case WZ_QUAGMIRE: //The target changes to "all" if used in a gvg map. [Skotlex]
		case AM_DEMONSTRATION:
		case GN_HELLS_PLANT:
			if (skill_id == GN_HELLS_PLANT && map->getcell(src->m, src, x, y, CELL_CHKLANDPROTECTOR))
				return NULL;
			if (map_flag_vs(src->m) && battle_config.vs_traps_bctall
				&& (src->type&battle_config.vs_traps_bctall))
				target = BCT_ALL;
			break;
		case HT_ANKLESNARE:
			if( flag&2 )
				val3 = SC_ESCAPE;
		case HT_SHOCKWAVE:
			val1=skill_lv*15+10;
		case HT_SANDMAN:
		case MA_SANDMAN:
		case HT_CLAYMORETRAP:
		case HT_SKIDTRAP:
		case MA_SKIDTRAP:
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
		case HT_BLASTMINE:
		/**
		 * Ranger
		 **/
		case RA_ELECTRICSHOCKER:
		case RA_CLUSTERBOMB:
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			{
				struct skill_condition req = skill->get_requirement(sd,skill_id,skill_lv);
				ARR_FIND(0, MAX_SKILL_ITEM_REQUIRE, i, req.itemid[i] && (req.itemid[i] == ITEMID_TRAP || req.itemid[i] == ITEMID_TRAP_ALLOY));
				if( i != MAX_SKILL_ITEM_REQUIRE && req.itemid[i] )
					req_item = req.itemid[i];
				if( map_flag_gvg2(src->m) || map->list[src->m].flag.battleground )
					limit *= 4; // longer trap times in WOE [celest]
				if( battle_config.vs_traps_bctall && map_flag_vs(src->m) && (src->type&battle_config.vs_traps_bctall) )
					target = BCT_ALL;
			}
			break;

		case SA_LANDPROTECTOR:
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
		{
			struct skill_unit_group *old_sg;
			if ((old_sg = skill->locate_element_field(src)) != NULL) {
				//HelloKitty confirmed that these are interchangeable,
				//so you can change element and not consume gemstones.
				if (( old_sg->skill_id == SA_VOLCANO
				   || old_sg->skill_id == SA_DELUGE
				   || old_sg->skill_id == SA_VIOLENTGALE
				    )
				 && old_sg->limit > 0
				) {
					//Use the previous limit (minus the elapsed time) [Skotlex]
					limit = old_sg->limit - DIFF_TICK32(timer->gettick(), old_sg->tick);
					if (limit < 0) //This can happen...
						limit = skill->get_time(skill_id,skill_lv);
				}
				skill->clear_group(src,1);
			}
			break;
		}

		case BA_DISSONANCE:
		case DC_UGLYDANCE:
			val1 = 10; //FIXME: This value is not used anywhere, what is it for? [Skotlex]
			break;
		case BA_WHISTLE:
			val1 = skill_lv +st->agi/10; // Flee increase
			val2 = ((skill_lv+1)/2)+st->luk/10; // Perfect dodge increase
			if(sd){
				val1 += pc->checkskill(sd,BA_MUSICALLESSON);
				val2 += pc->checkskill(sd,BA_MUSICALLESSON);
			}
			break;
		case DC_HUMMING:
			val1 = 2*skill_lv+st->dex/10; // Hit increase
			#ifdef RENEWAL
				val1 *= 2;
			#endif
			if(sd)
				val1 += pc->checkskill(sd,DC_DANCINGLESSON);
			break;
		case BA_POEMBRAGI:
			val1 = 3*skill_lv+st->dex/10; // Casting time reduction
			//For some reason at level 10 the base delay reduction is 50%.
			val2 = (skill_lv<10?3*skill_lv:50)+st->int_/5; // After-cast delay reduction
			if(sd){
				val1 += 2*pc->checkskill(sd,BA_MUSICALLESSON);
				val2 += 2*pc->checkskill(sd,BA_MUSICALLESSON);
			}
			break;
		case DC_DONTFORGETME:
#ifdef RENEWAL
			val1 = st->dex/10 + 3*skill_lv; // ASPD decrease
			val2 = st->agi/10 + 2*skill_lv; // Movement speed adjustment.
#else
			val1 = st->dex/10 + 3*skill_lv + 5; // ASPD decrease
			val2 = st->agi/10 + 3*skill_lv + 5; // Movement speed adjustment.
#endif
			if(sd){
				val1 += pc->checkskill(sd,DC_DANCINGLESSON);
				val2 += pc->checkskill(sd,DC_DANCINGLESSON);
			}
			break;
		case BA_APPLEIDUN:
			val1 = 5+2*skill_lv+st->vit/10; // MaxHP percent increase
			if(sd)
				val1 += pc->checkskill(sd,BA_MUSICALLESSON);
			break;
		case DC_SERVICEFORYOU:
			val1 = 15+skill_lv+(st->int_/10); // MaxSP percent increase
			val2 = 20+3*skill_lv+(st->int_/10); // SP cost reduction
			if(sd){
				val1 += pc->checkskill(sd,DC_DANCINGLESSON) / 2;
				val2 += pc->checkskill(sd,DC_DANCINGLESSON) / 2;
			}
			break;
		case BA_ASSASSINCROSS:
			if(sd)
				val1 = pc->checkskill(sd,BA_MUSICALLESSON) / 2;
#ifdef RENEWAL
			// This formula was taken from a RE calculator
			// and the changes published on irowiki
			// Luckily, official tests show it's the right one
			val1 += skill_lv + (st->agi/20);
#else
			val1 += 10 + skill_lv + (st->agi/10); // ASPD increase
			val1 *= 10; // ASPD works with 1000 as 100%
#endif
			break;
		case DC_FORTUNEKISS:
			val1 = 10+skill_lv+(st->luk/10); // Critical increase
			if(sd)
				val1 += pc->checkskill(sd,DC_DANCINGLESSON);
			val1*=10; //Because every 10 crit is an actual cri point.
			break;
		case BD_DRUMBATTLEFIELD:
		#ifdef RENEWAL
			val1 = (skill_lv+5)*25; //Watk increase
			val2 = skill_lv*10; //Def increase
		#else
			val1 = (skill_lv+1)*25; //Watk increase
			val2 = (skill_lv+1)*2; //Def increase
		#endif
			break;
		case BD_RINGNIBELUNGEN:
			val1 = (skill_lv+2)*25; //Watk increase
			break;
		case BD_RICHMANKIM:
			val1 = 25 + 11*skill_lv; //Exp increase bonus.
			break;
		case BD_SIEGFRIED:
			val1 = 55 + skill_lv*5; //Elemental Resistance
			val2 = skill_lv*10; //Status ailment resistance
			break;
		case WE_CALLPARTNER:
			if (sd) val1 = sd->status.partner_id;
			break;
		case WE_CALLPARENT:
			if (sd) {
				val1 = sd->status.father;
				val2 = sd->status.mother;
			}
			break;
		case WE_CALLBABY:
			if (sd) val1 = sd->status.child;
			break;
		case NJ_KAENSIN:
			skill->clear_group(src, 1); //Delete previous Kaensins/Suitons
			val2 = (skill_lv+1)/2 + 4;
			break;
		case NJ_SUITON:
			skill->clear_group(src, 1);
			break;

		case GS_GROUNDDRIFT:
			{
			int element[5]={ELE_WIND,ELE_DARK,ELE_POISON,ELE_WATER,ELE_FIRE};

			val1 = st->rhw.ele;
			if (!val1)
				val1=element[rnd()%5];

			switch (val1)
			{
				case ELE_FIRE:
					subunt++;
				case ELE_WATER:
					subunt++;
				case ELE_POISON:
					subunt++;
				case ELE_DARK:
					subunt++;
				case ELE_WIND:
					break;
				default:
					subunt=rnd()%5;
					break;
			}

			break;
			}
		case GC_POISONSMOKE:
			if( !(sc && sc->data[SC_POISONINGWEAPON]) )
				return NULL;
			val2 = sc->data[SC_POISONINGWEAPON]->val2; // Type of Poison
			val3 = sc->data[SC_POISONINGWEAPON]->val1;
			limit = 4000 + 2000 * skill_lv;
			break;
		case GD_LEADERSHIP:
		case GD_GLORYWOUNDS:
		case GD_SOULCOLD:
		case GD_HAWKEYES:
			limit = 1000000;//it doesn't matter
			break;
		case WL_COMET:
			if( sc ) {
				sc->comet_x = x;
				sc->comet_y = y;
			}
			break;
		case LG_BANDING:
			limit = -1;
			break;
		case WM_REVERBERATION:
			if( battle_config.vs_traps_bctall && map_flag_vs(src->m) && (src->type&battle_config.vs_traps_bctall) )
				target = BCT_ALL;
			val1 = skill_lv + 1;
			val2 = 1;
		case WM_POEMOFNETHERWORLD: // Can't be placed on top of Land Protector.
		case SO_WATER_INSIGNIA:
		case SO_FIRE_INSIGNIA:
		case SO_WIND_INSIGNIA:
		case SO_EARTH_INSIGNIA:
			if (map->getcell(src->m, src, x, y, CELL_CHKLANDPROTECTOR))
				return NULL;
			break;
		case SO_CLOUD_KILL:
			skill->clear_group(src, 4);
			break;
		case SO_WARMER:
			skill->clear_group(src, 8);
			break;
		case SO_VACUUM_EXTREME:
			val1 = x;
			val2 = y;
			break;
		case GN_WALLOFTHORN:
			if( flag&1 )
				limit = 3000;
			val3 = (x<<16)|y;
			break;
		case KO_ZENKAI:
			if (sd) {
				if (sd->charm_type != CHARM_TYPE_NONE && sd->charm_count > 0) {
					val1 = sd->charm_count; // no. of aura
					val2 = sd->charm_type; // aura type
					limit += val1 * 1000;
					subunt = sd->charm_type - 1;
					pc->del_charm(sd, sd->charm_count, sd->charm_type);
				}
			}
			break;
		case NPC_EARTHQUAKE:
			clif->skill_damage(src, src, timer->gettick(), status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			break;
		default:
			skill->unitsetting1_unknown(src, &skill_id, &skill_lv, &x, &y, &flag, &val1, &val2, &val3);
			break;
	}

	nullpo_retr(NULL, group=skill->init_unitgroup(src,layout->count,skill_id,skill_lv,skill->get_unit_id(skill_id,flag&1)+subunt, limit, interval));
	group->val1=val1;
	group->val2=val2;
	group->val3=val3;
	group->target_flag=target;
	group->bl_flag= skill->get_unit_bl_target(skill_id);
	group->state.ammo_consume = (sd && sd->state.arrow_atk && skill_id != GS_GROUNDDRIFT); //Store if this skill needs to consume ammo.
	group->state.song_dance = ((unit_flag&(UF_DANCE|UF_SONG)) ? 1 : 0)|((unit_flag&UF_ENSEMBLE) ? 2 : 0); //Signals if this is a song/dance/duet
	group->state.guildaura = ( skill_id >= GD_LEADERSHIP && skill_id <= GD_HAWKEYES )?1:0;
	group->item_id = req_item;
	//if tick is greater than current, do not invoke onplace function just yet. [Skotlex]
	if (DIFF_TICK(group->tick, timer->gettick()) > SKILLUNITTIMER_INTERVAL)
		active_flag = 0;

	if(skill_id==HT_TALKIEBOX || skill_id==RG_GRAFFITI){
		group->valstr=(char *) aMalloc(MESSAGE_SIZE*sizeof(char));
		if (sd)
			safestrncpy(group->valstr, sd->message, MESSAGE_SIZE);
		else //Eh... we have to write something here... even though mobs shouldn't use this. [Skotlex]
			safestrncpy(group->valstr, "Boo!", MESSAGE_SIZE);
	}

	if (group->state.song_dance) {
		if(sd){
			sd->skill_id_dance = skill_id;
			sd->skill_lv_dance = skill_lv;
		}
		if (
			sc_start4(src,src, SC_DANCING, 100, skill_id, group->group_id, skill_lv,
				(group->state.song_dance&2) ? BCT_SELF : 0, limit+1000) &&
			sd && group->state.song_dance&2 && skill_id != CG_HERMODE //Hermod is a encore with a warp!
		)
			skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 1);
	}

	limit = group->limit;
	for( i = 0; i < layout->count; i++ ) {
		struct skill_unit *su;
		int ux = x + layout->dx[i];
		int uy = y + layout->dy[i];
		int alive = 1;
		val1 = skill_lv;
		val2 = 0;

		if (!group->state.song_dance && !map->getcell(src->m, src, ux, uy, CELL_CHKREACH))
			continue; // don't place skill units on walls (except for songs/dances/encores)
		if( battle_config.skill_wall_check && skill->get_unit_flag(skill_id)&UF_PATHCHECK && !path->search_long(NULL,src,src->m,ux,uy,x,y,CELL_CHKWALL) )
			continue; // no path between cell and center of casting.

		switch( skill_id ) {
			case MG_FIREWALL:
			case NJ_KAENSIN:
				val2=group->val2;
				break;
			case WZ_ICEWALL:
				val1 = (skill_lv <= 1) ? 500 : 200 + 200*skill_lv;
				val2 = map->getcell(src->m, src, ux, uy, CELL_GETTYPE);
				break;
			case HT_LANDMINE:
			case MA_LANDMINE:
			case HT_ANKLESNARE:
			case HT_SHOCKWAVE:
			case HT_SANDMAN:
			case MA_SANDMAN:
			case HT_FLASHER:
			case HT_FREEZINGTRAP:
			case MA_FREEZINGTRAP:
			case HT_TALKIEBOX:
			case HT_SKIDTRAP:
			case MA_SKIDTRAP:
			case HT_CLAYMORETRAP:
			case HT_BLASTMINE:
			case RA_ELECTRICSHOCKER:
			case RA_CLUSTERBOMB:
			case RA_MAGENTATRAP:
			case RA_COBALTTRAP:
			case RA_MAIZETRAP:
			case RA_VERDURETRAP:
			case RA_FIRINGTRAP:
			case RA_ICEBOUNDTRAP:
				val1 = 3500;
				break;
			case GS_DESPERADO:
				val1 = abs(layout->dx[i]);
				val2 = abs(layout->dy[i]);
				if (val1 < 2 || val2 < 2) { //Nearby cross, linear decrease with no diagonals
					if (val2 > val1) val1 = val2;
					if (val1) val1--;
					val1 = 36 -12*val1;
				} else //Diagonal edges
					val1 = 28 -4*val1 -4*val2;
				if (val1 < 1) val1 = 1;
				val2 = 0;
				break;
			case GN_WALLOFTHORN:
				val1 = 2000 + 2000 * skill_lv;
				val2 = src->id;
				break;
			default:
				skill->unitsetting2_unknown(src, &skill_id, &skill_lv, &x, &y, &flag, &unit_flag, &val1, &val2, &val3, group);
				break;
		}
		if (skill->get_unit_flag(skill_id) & UF_RANGEDSINGLEUNIT && i == (layout->count / 2))
			val2 |= UF_RANGEDSINGLEUNIT; // center.

		map->foreachincell(skill->cell_overlap,src->m,ux,uy,BL_SKILL,skill_id, &alive, src);
		if( !alive )
			continue;

		nullpo_retr(NULL, su=skill->initunit(group,i,ux,uy,val1,val2));
		su->limit=limit;
		su->range=range;

		if (skill_id == PF_FOGWALL && alive == 2) {
			//Double duration of cells on top of Deluge/Suiton
			su->limit *= 2;
			group->limit = su->limit;
		}

		// execute on all targets standing on this cell
		if (range==0 && active_flag)
			map->foreachincell(skill->unit_effect,su->bl.m,su->bl.x,su->bl.y,group->bl_flag,&su->bl,timer->gettick(),1);
	}

	if (!group->alive_count) {
		//No cells? Something that was blocked completely by Land Protector?
		skill->del_unitgroup(group,ALC_MARK);
		return NULL;
	}

	//success, unit created.
	switch( skill_id ) {
		case NJ_TATAMIGAESHI: //Store number of tiles.
			group->val1 = group->alive_count;
			break;
	}

	return group;
}

void skill_unitsetting1_unknown(struct block_list *src, uint16 *skill_id, uint16 *skill_lv, int16 *x, int16 *y, int *flag, int *val1, int *val2, int *val3) {
}

void skill_unitsetting2_unknown(struct block_list *src, uint16 *skill_id, uint16 *skill_lv, int16 *x, int16 *y, int *flag, int *unit_flag, int *val1, int *val2, int *val3, struct skill_unit_group *group) {
	if (group->state.song_dance & 0x1)
		*val2 = *unit_flag & (UF_DANCE | UF_SONG); //Store whether this is a song/dance
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_onplace(struct skill_unit *src, struct block_list *bl, int64 tick) {
	struct skill_unit_group *sg;
	struct block_list *ss;
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;
	uint16 skill_id;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(bl->prev==NULL || !src->alive || status->isdead(bl))
		return 0;

	nullpo_ret(sg=src->group);
	nullpo_ret(ss=map->id2bl(sg->src_id));

	if (skill->get_type(sg->skill_id) == BF_MAGIC && map->getcell(src->bl.m, &src->bl, src->bl.x, src->bl.y, CELL_CHKLANDPROTECTOR) && sg->skill_id != SA_LANDPROTECTOR)
		return 0; //AoE skills are ineffective. [Skotlex]
	sc = status->get_sc(bl);

	if (sc && sc->option&OPTION_HIDE && sg->skill_id != WZ_HEAVENDRIVE && sg->skill_id != WL_EARTHSTRAIN )
		return 0; //Hidden characters are immune to AoE skills except to these. [Skotlex]

	if (sc && sc->data[SC_VACUUM_EXTREME] && map->getcell(bl->m, bl, bl->x, bl->y, CELL_CHKLANDPROTECTOR))
		status_change_end(bl, SC_VACUUM_EXTREME, INVALID_TIMER);

	if ( sc && sc->data[SC_HOVERING] && ( sg->skill_id == SO_VACUUM_EXTREME || sg->skill_id == SO_ELECTRICWALK || sg->skill_id == SO_FIREWALK || sg->skill_id == WZ_QUAGMIRE ) )
		return 0;

	type = status->skill2sc(sg->skill_id);
	sce = (sc != NULL && type != SC_NONE) ? sc->data[type] : NULL;
	skill_id = sg->skill_id; //In case the group is deleted, we need to return the correct skill id, still.
	switch (sg->unit_id) {
		case UNT_SPIDERWEB:
			if( sc && sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1 > 0 ) {
				// If you are fiberlocked and can't move, it will only increase your fireweakness level. [Inkfish]
				sc->data[SC_SPIDERWEB]->val2++;
				break;
			} else if (sc && battle->check_target(&src->bl,bl,sg->target_flag) > 0) {
				int sec = skill->get_time2(sg->skill_id,sg->skill_lv);
				if( status->change_start(ss, bl,type,10000,sg->skill_lv,1,sg->group_id,0,sec,SCFLAG_FIXEDRATE) ) {
					const struct TimerData* td = sce?timer->get(sce->timer):NULL;
					if( td )
						sec = DIFF_TICK32(td->tick, tick);
					map->moveblock(bl, src->bl.x, src->bl.y, tick);
					clif->fixpos(bl);
					sg->val2 = bl->id;
				} else {
					sec = 3000; //Couldn't trap it?
				}
				sg->limit = DIFF_TICK32(tick,sg->tick)+sec;
			}
			break;
		case UNT_SAFETYWALL:
			if (!sce)
				sc_start4(ss,bl,type,100,sg->skill_lv,sg->skill_id,sg->group_id,0,sg->limit);
			break;
		case UNT_BLOODYLUST:
			if (sg->src_id == bl->id)
				break; //Does not affect the caster.
			if (bl->type == BL_MOB)
				break; //Does not affect the caster.
			if( !sce && sc_start4(ss,bl,type,100,sg->skill_lv,0,SC__BLOODYLUST,0,sg->limit) )
				sc_start(ss,bl,SC__BLOODYLUST,100,sg->skill_lv,sg->limit);
			break;
		case UNT_PNEUMA:
		case UNT_CHAOSPANIC:
		case UNT_MAELSTROM:
			if (!sce)
				sc_start4(ss,bl,type,100,sg->skill_lv,sg->group_id,0,0,sg->limit);
			break;

		case UNT_WARP_WAITING: {
			int working = sg->val1&0xffff;

			if (bl->type == BL_PC && !working) {
				struct map_session_data *sd = BL_UCAST(BL_PC, bl);
				if ((sd->chat_id == 0 || battle_config.chat_warpportal) && sd->ud.to_x == src->bl.x && sd->ud.to_y == src->bl.y) {
					int x = sg->val2>>16;
					int y = sg->val2&0xffff;
					int count = sg->val1>>16;
					unsigned short m = sg->val3;

					if( --count <= 0 )
						skill->del_unitgroup(sg,ALC_MARK);

					if ( map->mapindex2mapid(sg->val3) == sd->bl.m && x == sd->bl.x && y == sd->bl.y )
						working = 1;/* we break it because officials break it, lovely stuff. */

					sg->val1 = (count<<16)|working;

					pc->setpos(sd,m,x,y,CLR_TELEPORT);
				}
			} else if(bl->type == BL_MOB && battle_config.mob_warp&2) {
				int16 m = map->mapindex2mapid(sg->val3);
				if (m < 0) break; //Map not available on this map-server.
				unit->warp(bl,m,sg->val2>>16,sg->val2&0xffff,CLR_TELEPORT);
			}
		}
			break;

		case UNT_QUAGMIRE:
			if (!sce && battle->check_target(&src->bl,bl,sg->target_flag) > 0)
				sc_start4(ss,bl,type,100,sg->skill_lv,sg->group_id,0,0,sg->limit);
			break;

		case UNT_VOLCANO:
		case UNT_DELUGE:
		case UNT_VIOLENTGALE:
			if(!sce)
				sc_start(ss,bl,type,100,sg->skill_lv,sg->limit);
			break;

		case UNT_SUITON:
			if(!sce)
				sc_start4(ss,bl,type,100,sg->skill_lv,
				map_flag_vs(bl->m) || battle->check_target(&src->bl,bl,BCT_ENEMY)>0?1:0, //Send val3 =1 to reduce agi.
				0,0,sg->limit);
			break;

		case UNT_HERMODE:
			if (sg->src_id!=bl->id && battle->check_target(&src->bl,bl,BCT_PARTY|BCT_GUILD) > 0)
				status->change_clear_buffs(bl,1); //Should dispell only allies.
		case UNT_RICHMANKIM:
		case UNT_ETERNALCHAOS:
		case UNT_DRUMBATTLEFIELD:
		case UNT_RINGNIBELUNGEN:
		case UNT_ROKISWEIL:
		case UNT_INTOABYSS:
		case UNT_SIEGFRIED:
			 //Needed to check when a dancer/bard leaves their ensemble area.
			if (sg->src_id==bl->id && !(sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_BARDDANCER))
				return skill_id;
			if (!sce)
				sc_start4(ss,bl,type,100,sg->skill_lv,sg->val1,sg->val2,0,sg->limit);
			break;
		case UNT_APPLEIDUN:
			// If Aegis, apple of idun doesn't update its effect
			if (!battle_config.song_timer_reset && sc && sce)
				return 0;
			// Let it fall through
		case UNT_WHISTLE:
		case UNT_ASSASSINCROSS:
		case UNT_POEMBRAGI:
		case UNT_HUMMING:
		case UNT_DONTFORGETME:
		case UNT_FORTUNEKISS:
		case UNT_SERVICEFORYOU:
			// Don't buff themselves without link!
			if (sg->src_id==bl->id && !(sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_BARDDANCER))
				return 0;

			if (!sc) return 0;
			if (!sce)
				sc_start4(ss,bl,type,100,sg->skill_lv,sg->val1,sg->val2,0,sg->limit);
			// From here songs are already active
			else if (battle_config.song_timer_reset && sce->val4 == 1) {
				// eA style:
				// Readjust timers since the effect will not last long.
				sce->val4 = 0;
				timer->delete(sce->timer, status->change_timer);
				sce->timer = timer->add(tick+sg->limit, status->change_timer, bl->id, type);
			} else if (!battle_config.song_timer_reset) {
				// Aegis style:
				// Songs won't renew unless finished
				const struct TimerData *td = timer->get(sce->timer);
				if (DIFF_TICK32(td->tick, timer->gettick()) < sg->interval) {
					// Update with new values as the current one will vanish soon
					timer->delete(sce->timer, status->change_timer);
					sce->timer = timer->add(tick+sg->limit, status->change_timer, bl->id, type);
					sce->val1 = sg->skill_lv; // Why are we storing skill_lv as val1?
					sce->val2 = sg->val1;
					sce->val3 = sg->val2;
					sce->val4 = 0;
				}
			}
			break;

		case UNT_FOGWALL:
			if (!sce)
			{
				sc_start4(ss, bl, type, 100, sg->skill_lv, sg->val1, sg->val2, sg->group_id, sg->limit);
				if (battle->check_target(&src->bl,bl,BCT_ENEMY)>0)
					skill->additional_effect (ss, bl, sg->skill_id, sg->skill_lv, BF_MISC, ATK_DEF, tick);
			}
			break;

		case UNT_GRAVITATION:
			if (!sce)
				sc_start4(ss,bl,type,100,sg->skill_lv,0,BCT_ENEMY,sg->group_id,sg->limit);
			break;

#if 0 // officially, icewall has no problems existing on occupied cells [ultramage]
		case UNT_ICEWALL: //Destroy the cell. [Skotlex]
			src->val1 = 0;
			if(src->limit + sg->tick > tick + 700)
				src->limit = DIFF_TICK32(tick+700,sg->tick);
			break;
#endif // 0

		case UNT_MOONLIT:
			//Knockback out of area if affected char isn't in Moonlit effect
			if (sc && sc->data[SC_DANCING] && (sc->data[SC_DANCING]->val1&0xFFFF) == CG_MOONLIT)
				break;
			if (ss == bl) //Also needed to prevent infinite loop crash.
				break;
			skill->blown(ss,bl,skill->get_blewcount(sg->skill_id,sg->skill_lv),unit->getdir(bl),0);
			break;

		case UNT_WALLOFTHORN:
			if( status_get_mode(bl)&MD_BOSS )
				break; // iRO Wiki says that this skill don't affect to Boss monsters.
			if( map_flag_vs(bl->m) || bl->id == src->bl.id || battle->check_target(&src->bl,bl, BCT_ENEMY) == 1 )
				skill->attack(skill->get_type(sg->skill_id), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_REVERBERATION:
			if (sg->src_id == bl->id)
				break; //Does not affect the caster.
			clif->changetraplook(&src->bl,UNT_USED_TRAPS);
			map->foreachinrange(skill->trap_splash,&src->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl,tick);
			sg->unit_id = UNT_USED_TRAPS;
			sg->limit = DIFF_TICK32(tick,sg->tick) + 1500;
			break;

		case UNT_VOLCANIC_ASH:
			if (!sce)
				sc_start(ss, bl, SC_VOLCANIC_ASH, 100, sg->skill_lv, skill->get_time(MH_VOLCANIC_ASH, sg->skill_lv));
			break;

		case UNT_GD_LEADERSHIP:
		case UNT_GD_GLORYWOUNDS:
		case UNT_GD_SOULCOLD:
		case UNT_GD_HAWKEYES:
			if (!sce && battle->check_target(&src->bl,bl,sg->target_flag) > 0)
				sc_start4(ss,bl,type,100,sg->skill_lv,0,0,0,1000);
			break;
		default:
			skill->unit_onplace_unknown(src, bl, &tick);
			break;
	}
	return skill_id;
}

void skill_unit_onplace_unknown(struct skill_unit *src, struct block_list *bl, int64 *tick) {
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_onplace_timer(struct skill_unit *src, struct block_list *bl, int64 tick) {
	struct skill_unit_group *sg;
	struct block_list *ss;
	struct map_session_data *tsd;
	struct status_data *tstatus, *bst;
	struct status_change *tsc, *ssc;
	struct skill_unit_group_tickset *ts;
	enum sc_type type;
	uint16 skill_id;
	int diff=0;

	nullpo_ret(src);
	nullpo_ret(bl);

	if (bl->prev==NULL || !src->alive || status->isdead(bl))
		return 0;

	nullpo_ret(sg=src->group);
	nullpo_ret(ss=map->id2bl(sg->src_id));
	tsd = BL_CAST(BL_PC, bl);
	tsc = status->get_sc(bl);
	ssc = status->get_sc(ss); // Status Effects for Unit caster.

	// Maestro or Wanderer is unaffected by traps of trappers he or she charmed [SuperHulk]
	if ( ssc && ssc->data[SC_SIREN] && ssc->data[SC_SIREN]->val2 == bl->id && (skill->get_inf2(sg->skill_id)&INF2_TRAP) )
		return 0;

	tstatus = status->get_status_data(bl);
	bst = status->get_base_status(bl);
	type = status->skill2sc(sg->skill_id);
	skill_id = sg->skill_id;

	if ( tsc && tsc->data[SC_HOVERING] ) {
		switch ( skill_id ) {
		case HT_SKIDTRAP:
		case HT_LANDMINE:
		case HT_ANKLESNARE:
		case HT_FLASHER:
		case HT_SHOCKWAVE:
		case HT_SANDMAN:
		case HT_FREEZINGTRAP:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
		case HW_GRAVITATION:
		case SA_DELUGE:
		case SA_VOLCANO:
		case SA_VIOLENTGALE:
		case NJ_SUITON:
			return 0;
		}
	}

	if (sg->interval == -1) {
		switch (sg->unit_id) {
			case UNT_ANKLESNARE: //These happen when a trap is splash-triggered by multiple targets on the same cell.
			case UNT_FIREPILLAR_ACTIVE:
			case UNT_ELECTRICSHOCKER:
			case UNT_MANHOLE:
				return 0;
			default:
				ShowError("skill_unit_onplace_timer: interval error (unit id %x)\n", (unsigned int)sg->unit_id);
				return 0;
		}
	}

	if ((ts = skill->unitgrouptickset_search(bl,sg,tick))) {
		//Not all have it, eg: Traps don't have it even though they can be hit by Heaven's Drive [Skotlex]
		diff = DIFF_TICK32(tick,ts->tick);
		if (diff < 0)
			return 0;
		ts->tick = tick+sg->interval;

		if ((skill_id==CR_GRANDCROSS || skill_id==NPC_GRANDDARKNESS) && !battle_config.gx_allhit)
			ts->tick += sg->interval*(map->count_oncell(bl->m,bl->x,bl->y,BL_CHAR,0)-1);
	}

	switch (sg->unit_id) {
		case UNT_FIREWALL:
		case UNT_KAEN: {
			int count=0;
			const int x = bl->x, y = bl->y;

			if( sg->skill_id == GN_WALLOFTHORN && !map_flag_vs(bl->m) )
				break;

			//Take into account these hit more times than the timer interval can handle.
			do
				skill->attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick+count*sg->interval,0);
			while (src->alive != 0 && --src->val2 != 0 && x == bl->x && y == bl->y
			    && ++count < SKILLUNITTIMER_INTERVAL/sg->interval && !status->isdead(bl));

			if (src->val2<=0)
				skill->delunit(src);
		}
		break;

		case UNT_SANCTUARY:
			if( battle->check_undead(tstatus->race, tstatus->def_ele) || tstatus->race==RC_DEMON )
			{ //Only damage enemies with offensive Sanctuary. [Skotlex]
				if( battle->check_target(&src->bl,bl,BCT_ENEMY) > 0 && skill->attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0) )
					sg->val1 -= 2; // reduce healing count if this was meant for damaging [hekate]
			} else {
				int heal = skill->calc_heal(ss,bl,sg->skill_id,sg->skill_lv,true);
				struct mob_data *md = BL_CAST(BL_MOB, bl);
#ifdef RENEWAL
				if (md != NULL && md->class_ == MOBID_EMPELIUM)
					break;
#endif
				if (md && mob_is_battleground(md))
					break;
				if (tstatus->hp >= tstatus->max_hp)
					break;
				if (status->isimmune(bl))
					heal = 0;
				clif->skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
				if (tsc && tsc->data[SC_AKAITSUKI] && heal)
					heal = ~heal + 1;
				status->heal(bl, heal, 0, 0);
				if (diff >= 500)
					sg->val1--;
			}
			if (sg->val1 <= 0)
				skill->del_unitgroup(sg, ALC_MARK);
			break;

		case UNT_EVILLAND:
			//Will heal demon and undead element monsters, but not players.
			if ((bl->type == BL_PC) || (!battle->check_undead(tstatus->race, tstatus->def_ele) && tstatus->race!=RC_DEMON)) {
				//Damage enemies
				if(battle->check_target(&src->bl,bl,BCT_ENEMY)>0)
					skill->attack(BF_MISC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			} else {
				int heal = skill->calc_heal(ss,bl,sg->skill_id,sg->skill_lv,true);
				if (tstatus->hp >= tstatus->max_hp)
					break;
				if (status->isimmune(bl))
					heal = 0;
				clif->skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
				status->heal(bl, heal, 0, 0);
			}
			break;

		case UNT_MAGNUS:
			if (!battle->check_undead(tstatus->race,tstatus->def_ele) && tstatus->race!=RC_DEMON)
				break;
			skill->attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_DUMMYSKILL:
			switch (sg->skill_id) {
				case SG_SUN_WARM: //SG skills [Komurka]
				case SG_MOON_WARM:
				case SG_STAR_WARM:
				{
					int count = 0;
					const int x = bl->x, y = bl->y;

					map->freeblock_lock();
					//If target isn't knocked back it should hit every "interval" ms [Playtester]
					do {
						if( bl->type == BL_PC )
							status_zap(bl, 0, 15); // sp damage to players
						else if( status->charge(ss, 0, 2) ) { // mobs
							// costs 2 SP per hit
							if( !skill->attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick+count*sg->interval,0) )
								status->charge(ss, 0, 8); //costs additional 8 SP if miss
						} else { // mobs
							//should end when out of sp.
							sg->limit = DIFF_TICK32(tick,sg->tick);
							break;
						}
					} while (src->alive != 0 && x == bl->x && y == bl->y && sg->alive_count != 0
					      && ++count < SKILLUNITTIMER_INTERVAL/sg->interval && !status->isdead(bl));
					map->freeblock_unlock();
				}
				break;
		/**
		 * The storm gust counter was dropped in renewal
		 **/
		#ifndef RENEWAL
				case WZ_STORMGUST: //SG counter does not reset per stormgust. IE: One hit from a SG and two hits from another will freeze you.
					if (tsc)
						tsc->sg_counter++; //SG hit counter.
					if (skill->attack(skill->get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0) <= 0 && tsc)
						tsc->sg_counter=0; //Attack absorbed.
				break;
		#endif
				case GS_DESPERADO:
					if (rnd()%100 < src->val1)
						skill->attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
				break;
				default:
					skill->attack(skill->get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			}
			break;

		case UNT_EARTHQUAKE:
			skill->attack(BF_WEAPON, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick,
				map->foreachinrange(skill->area_sub, &src->bl, skill->get_splash(sg->skill_id, sg->skill_lv), BL_CHAR, &src->bl, sg->skill_id, sg->skill_lv, tick, BCT_ENEMY, skill->area_sub_count));
			break;

		case UNT_FIREPILLAR_WAITING:
			skill->unitsetting(ss,sg->skill_id,sg->skill_lv,src->bl.x,src->bl.y,1);
			skill->delunit(src);
			break;

		case UNT_SKIDTRAP:
			{
				skill->blown(&src->bl,bl,skill->get_blewcount(sg->skill_id,sg->skill_lv),unit->getdir(bl),0);
				sg->unit_id = UNT_USED_TRAPS;
				clif->changetraplook(&src->bl, UNT_USED_TRAPS);
				sg->limit=DIFF_TICK32(tick,sg->tick)+1500;
			}
			break;

		case UNT_ANKLESNARE:
		case UNT_MANHOLE:
			if( sg->val2 == 0 && tsc && (sg->unit_id == UNT_ANKLESNARE || bl->id != sg->src_id) ) {
				int sec = skill->get_time2(sg->skill_id,sg->skill_lv);
				if( status->change_start(ss,bl,type,10000,sg->skill_lv,sg->group_id,0,0,sec, SCFLAG_FIXEDRATE) ) {
					const struct TimerData* td = tsc->data[type]?timer->get(tsc->data[type]->timer):NULL;
					if( td )
						sec = DIFF_TICK32(td->tick, tick);
					if( sg->unit_id == UNT_MANHOLE || battle_config.skill_trap_type || !map_flag_gvg2(src->bl.m) ) {
						unit->movepos(bl, src->bl.x, src->bl.y, 0, 0);
						clif->fixpos(bl);
					}
					sg->val2 = bl->id;
				} else
					sec = 3000; //Couldn't trap it?
				if( sg->unit_id == UNT_ANKLESNARE ) {
					clif->skillunit_update(&src->bl);
					/**
					 * If you're snared from a trap that was invisible this makes the trap be
					 * visible again -- being you stepped on it (w/o this the trap remains invisible and you go "WTF WHY I CANT MOVE")
					 * bugreport:3961
					 **/
					clif->changetraplook(&src->bl, UNT_ANKLESNARE);
				}
				sg->limit = DIFF_TICK32(tick,sg->tick)+sec;
				sg->interval = -1;
				src->range = 0;
			}
			break;

		case UNT_ELECTRICSHOCKER:
			if( bl->id != ss->id ) {
				if( status_get_mode(bl)&MD_BOSS )
					break;
				if( status->change_start(ss,bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill->get_time2(sg->skill_id, sg->skill_lv), SCFLAG_FIXEDRATE) ) {
					map->moveblock(bl, src->bl.x, src->bl.y, tick);
					clif->fixpos(bl);

				}

				map->foreachinrange(skill->trap_splash, &src->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl, tick);
				sg->unit_id = UNT_USED_TRAPS; //Changed ID so it does not invoke a for each in area again.
			}
			break;

		case UNT_VENOMDUST:
			if(tsc && !tsc->data[type])
				status->change_start(ss,bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill->get_time2(sg->skill_id,sg->skill_lv),SCFLAG_NONE);
			break;

		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
			if( bl->type == BL_PC )// it won't work on players
				break;
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
		case UNT_CLUSTERBOMB:
			if( bl->id == ss->id )// it won't trigger on caster
				break;
		case UNT_LANDMINE:
		case UNT_BLASTMINE:
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
		case UNT_FREEZINGTRAP:
		case UNT_FIREPILLAR_ACTIVE:
		case UNT_CLAYMORETRAP:
			if( sg->unit_id == UNT_FIRINGTRAP || sg->unit_id == UNT_ICEBOUNDTRAP || sg->unit_id == UNT_CLAYMORETRAP )
				map->foreachinrange(skill->trap_splash,&src->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag|BL_SKILL|~BCT_SELF, &src->bl,tick);
			else
				map->foreachinrange(skill->trap_splash,&src->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl,tick);
			if (sg->unit_id != UNT_FIREPILLAR_ACTIVE)
				clif->changetraplook(&src->bl, sg->unit_id==UNT_LANDMINE?UNT_FIREPILLAR_ACTIVE:UNT_USED_TRAPS);
			sg->limit=DIFF_TICK32(tick,sg->tick)+1500 +
				(sg->unit_id== UNT_CLUSTERBOMB || sg->unit_id== UNT_ICEBOUNDTRAP?1000:0);// Cluster Bomb/Icebound has 1s to disappear once activated.
			sg->unit_id = UNT_USED_TRAPS; //Changed ID so it does not invoke a for each in area again.
			break;

		case UNT_TALKIEBOX:
			if (sg->src_id == bl->id)
				break;
			if (sg->val2 == 0){
				clif->talkiebox(&src->bl, sg->valstr);
				sg->unit_id = UNT_USED_TRAPS;
				clif->changetraplook(&src->bl, UNT_USED_TRAPS);
				sg->limit = DIFF_TICK32(tick, sg->tick) + 5000;
				sg->val2 = -1;
			}
			break;

		case UNT_LULLABY:
			if (ss->id == bl->id)
				break;
			skill->additional_effect(ss, bl, sg->skill_id, sg->skill_lv, BF_LONG|BF_SKILL|BF_MISC, ATK_DEF, tick);
			break;

		case UNT_UGLYDANCE:
			if (ss->id != bl->id)
				skill->additional_effect(ss, bl, sg->skill_id, sg->skill_lv, BF_LONG|BF_SKILL|BF_MISC, ATK_DEF, tick);
			break;

		case UNT_DISSONANCE:
			skill->attack(BF_MISC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_APPLEIDUN: //Apple of Idun [Skotlex]
		{
			int heal;
#ifdef RENEWAL
			struct mob_data *md = BL_CAST(BL_MOB, bl);
			if (md && md->class_ == MOBID_EMPELIUM)
				break;
#endif
			// Don't buff themselves!
			if ((sg->src_id == bl->id && !(tsc && tsc->data[SC_SOULLINK] && tsc->data[SC_SOULLINK]->val2 == SL_BARDDANCER)))
				break;

			// Aegis style
			// Check if the remaining time is enough to survive the next update
			if (!battle_config.song_timer_reset
					&& !(tsc && tsc->data[type] && tsc->data[type]->val4 == 1)) {
				// Apple of Idun is not active. Start it now
				sc_start4(ss, bl, type, 100, sg->skill_lv, sg->val1, sg->val2, 0, sg->limit);
			}

			if (tstatus->hp < tstatus->max_hp) {
				heal = skill->calc_heal(ss,bl,sg->skill_id, sg->skill_lv, true);
				if( tsc && tsc->data[SC_AKAITSUKI] && heal )
					heal = ~heal + 1;
				clif->skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
				status->heal(bl, heal, 0, 0);
			}
		}
			break;
		case UNT_POEMBRAGI:
		case UNT_WHISTLE:
		case UNT_ASSASSINCROSS:
		case UNT_HUMMING:
		case UNT_DONTFORGETME:
		case UNT_FORTUNEKISS:
		case UNT_SERVICEFORYOU:
			// eA style: doesn't need this
			if (battle_config.song_timer_reset)
				break;
			// Don't let buff themselves!
			if (sg->src_id == bl->id && !(tsc && tsc->data[SC_SOULLINK] && tsc->data[SC_SOULLINK]->val2 == SL_BARDDANCER))
				break;

			// Aegis style
			// Check if song has enough time to survive the next check
			if (!(battle_config.song_timer_reset) && tsc && tsc->data[type] && tsc->data[type]->val4 == 1) {
				const struct TimerData *td = timer->get(tsc->data[type]->timer);
				if (DIFF_TICK32(td->tick, timer->gettick()) < sg->interval) {
					// Update with new values as the current one will vanish
					timer->delete(tsc->data[type]->timer, status->change_timer);
					tsc->data[type]->timer = timer->add(tick+sg->limit, status->change_timer, bl->id, type);
					tsc->data[type]->val1 = sg->skill_lv;
					tsc->data[type]->val2 = sg->val1;
					tsc->data[type]->val3 = sg->val2;
					tsc->data[type]->val4 = 0;
				}
				break; // Had enough time or not, it now has. Exit
			}

			// Song was not active. Start it now
			sc_start4(ss, bl, type, 100, sg->skill_lv, sg->val1, sg->val2, 0, sg->limit);
			break;
		case UNT_TATAMIGAESHI:
		case UNT_DEMONSTRATION:
			skill->attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_GOSPEL:
			if (rnd()%100 > sg->skill_lv*10 || ss == bl)
				break;
			if (battle->check_target(ss,bl,BCT_PARTY)>0)
			{ // Support Effect only on party, not guild
				int heal;
				int i = rnd()%13; // Positive buff count
				int time = skill->get_time2(sg->skill_id, sg->skill_lv); //Duration
				switch (i) {
					case 0: // Heal 1~9999 HP
						heal = rnd() %9999+1;
						clif->skill_nodamage(ss,bl,AL_HEAL,heal,1);
						status->heal(bl,heal,0,0);
						break;
					case 1: // End all negative status
						status->change_clear_buffs(bl,2);
						if (tsd) clif->gospel_info(tsd, 0x15);
						break;
					case 2: // Immunity to all status
						sc_start(ss,bl,SC_SCRESIST,100,100,time);
						if (tsd) clif->gospel_info(tsd, 0x16);
						break;
					case 3: // MaxHP +100%
						sc_start(ss,bl,SC_INCMHPRATE,100,100,time);
						if (tsd) clif->gospel_info(tsd, 0x17);
						break;
					case 4: // MaxSP +100%
						sc_start(ss,bl,SC_INCMSPRATE,100,100,time);
						if (tsd) clif->gospel_info(tsd, 0x18);
						break;
					case 5: // All stats +20
						sc_start(ss,bl,SC_INCALLSTATUS,100,20,time);
						if (tsd) clif->gospel_info(tsd, 0x19);
						break;
					case 6: // Level 10 Blessing
						sc_start(ss,bl,SC_BLESSING,100,10,time);
						break;
					case 7: // Level 10 Increase AGI
						sc_start(ss,bl,SC_INC_AGI,100,10,time);
						break;
					case 8: // Enchant weapon with Holy element
						sc_start(ss,bl,SC_ASPERSIO,100,1,time);
						if (tsd) clif->gospel_info(tsd, 0x1c);
						break;
					case 9: // Enchant armor with Holy element
						sc_start(ss,bl,SC_BENEDICTIO,100,1,time);
						if (tsd) clif->gospel_info(tsd, 0x1d);
						break;
					case 10: // DEF +25%
						sc_start(ss,bl,SC_INCDEFRATE,100,25,time);
						if (tsd) clif->gospel_info(tsd, 0x1e);
						break;
					case 11: // ATK +100%
						sc_start(ss,bl,SC_INCATKRATE,100,100,time);
						if (tsd) clif->gospel_info(tsd, 0x1f);
						break;
					case 12: // HIT/Flee +50
						sc_start(ss,bl,SC_INCHIT,100,50,time);
						sc_start(ss,bl,SC_INCFLEE,100,50,time);
						if (tsd) clif->gospel_info(tsd, 0x20);
						break;
				}
			}
			else if (battle->check_target(&src->bl,bl,BCT_ENEMY)>0)
			{ // Offensive Effect
				int i = rnd()%9; // Negative buff count
				int time = skill->get_time2(sg->skill_id, sg->skill_lv);
				switch (i)
				{
					case 0: // Deal 1~9999 damage
						skill->attack(BF_MISC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
						break;
					case 1: // Curse
						sc_start(ss,bl,SC_CURSE,100,1,time);
						break;
					case 2: // Blind
						sc_start(ss,bl,SC_BLIND,100,1,time);
						break;
					case 3: // Poison
						sc_start(ss,bl,SC_POISON,100,1,time);
						break;
					case 4: // Level 10 Provoke
						sc_start(ss,bl,SC_PROVOKE,100,10,time);
						break;
					case 5: // DEF -100%
						sc_start(ss,bl,SC_INCDEFRATE,100,-100,time);
						break;
					case 6: // ATK -100%
						sc_start(ss,bl,SC_INCATKRATE,100,-100,time);
						break;
					case 7: // Flee -100%
						sc_start(ss,bl,SC_INCFLEERATE,100,-100,time);
						break;
					case 8: // Speed/ASPD -25%
						sc_start4(ss,bl,SC_GOSPEL,100,1,0,0,BCT_ENEMY,time);
						break;
				}
			}
			break;

		case UNT_BASILICA:
			{
				int i = battle->check_target(&src->bl, bl, BCT_ENEMY);
				if( i > 0 && !(status_get_mode(bl)&MD_BOSS) )
				{ // knock-back any enemy except Boss
					skill->blown(&src->bl, bl, 2, unit->getdir(bl), 0);
					clif->fixpos(bl);
				}

				if( sg->src_id != bl->id && i <= 0 )
					sc_start4(ss, bl, type, 100, 0, 0, 0, src->bl.id, sg->interval + 100);
			}
			break;

		case UNT_GRAVITATION:
		case UNT_EARTHSTRAIN:
		case UNT_FIREWALK:
		case UNT_ELECTRICWALK:
		case UNT_PSYCHIC_WAVE:
		case UNT_MAGMA_ERUPTION:
		case UNT_MAKIBISHI:
			skill->attack(skill->get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_GROUNDDRIFT_WIND:
		case UNT_GROUNDDRIFT_DARK:
		case UNT_GROUNDDRIFT_POISON:
		case UNT_GROUNDDRIFT_WATER:
		case UNT_GROUNDDRIFT_FIRE:
			map->foreachinrange(skill->trap_splash,&src->bl,
			                    skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag,
			                    &src->bl,tick);
			sg->unit_id = UNT_USED_TRAPS;
			//clif->changetraplook(&src->bl, UNT_FIREPILLAR_ACTIVE);
			sg->limit=DIFF_TICK32(tick,sg->tick)+1500;
			break;
		/**
		 * 3rd stuff
		 **/
		case UNT_POISONSMOKE:
			if( battle->check_target(ss,bl,BCT_ENEMY) > 0 && !(tsc && tsc->data[sg->val2]) && rnd()%100 < 50 ) {
				short rate = 100;
				if ( sg->val1 == 9 )//Oblivion Curse gives a 2nd success chance after the 1st one passes which is reducible. [Rytech]
					rate = 100 - tstatus->int_ * 4 / 5 ;
				sc_start(ss,bl,sg->val2,rate,sg->val1,skill->get_time2(GC_POISONINGWEAPON,1) - (tstatus->vit + tstatus->luk) / 2 * 1000);
			}
			break;

		case UNT_EPICLESIS:
			if( bl->type == BL_PC && !battle->check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON ) {
				if( ++sg->val2 % 3 == 0 ) {
					int hp, sp;
					switch( sg->skill_lv ) {
						case 1: case 2: hp = 3; sp = 2; break;
						case 3: case 4: hp = 4; sp = 3; break;
						case 5: default: hp = 5; sp = 4; break;
					}
					hp = tstatus->max_hp * hp / 100;
					sp = tstatus->max_sp * sp / 100;
					status->heal(bl, hp, sp, 2);
					sc_start(ss, bl, type, 100, sg->skill_lv, (sg->interval * 3) + 100);
				}
				// Reveal hidden players every 5 seconds.
				if( sg->val2 % 5 == 0 ) {
					// TODO: check if other hidden status can be removed.
					status_change_end(bl,SC_HIDING,INVALID_TIMER);
					status_change_end(bl,SC_CLOAKING,INVALID_TIMER);
					status_change_end(bl,SC_CLOAKINGEXCEED,INVALID_TIMER);
				}
			}
			/* Enable this if kRO fix the current skill. Currently no damage on undead and demon monster. [Jobbie]
			else if( battle->check_target(ss, bl, BCT_ENEMY) > 0 && battle->check_undead(tstatus->race, tstatus->def_ele) )
				skill->castend_damage_id(&src->bl, bl, sg->skill_id, sg->skill_lv, 0, 0);*/
			break;

		case UNT_STEALTHFIELD:
			if( bl->id == sg->src_id )
				break; // Don't work on Self (video shows that)
		case UNT_NEUTRALBARRIER:
			sc_start(ss,bl,type,100,sg->skill_lv,sg->interval + 100);
			break;

		case UNT_DIMENSIONDOOR:
			if( tsd && !map->list[bl->m].flag.noteleport )
				pc->randomwarp(tsd,3);
			else if( bl->type == BL_MOB && battle_config.mob_warp&8 )
				unit->warp(bl,-1,-1,-1,3);
			break;

		case UNT_REVERBERATION:
			clif->changetraplook(&src->bl,UNT_USED_TRAPS);
			map->foreachinrange(skill->trap_splash,&src->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl,tick);
			sg->limit = DIFF_TICK32(tick,sg->tick)+1500;
			sg->unit_id = UNT_USED_TRAPS;
			break;

		case UNT_SEVERE_RAINSTORM:
			if( battle->check_target(&src->bl, bl, BCT_ENEMY))
				skill->attack(BF_WEAPON,ss,&src->bl,bl,WM_SEVERE_RAINSTORM_MELEE,sg->skill_lv,tick,0);
			break;
		case UNT_NETHERWORLD:
			if ( battle->check_target(&src->bl, bl, BCT_PARTY) == -1 && bl->id != sg->src_id ) {
				sc_start(ss, bl, type, 100, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
				sg->limit = 0;
				clif->changetraplook(&src->bl, UNT_USED_TRAPS);
				sg->unit_id = UNT_USED_TRAPS;
			}
			break;
		case UNT_THORNS_TRAP:
			if( tsc ) {
				if( !sg->val2 ) {
					int sec = skill->get_time2(sg->skill_id, sg->skill_lv);
					if( sc_start(ss, bl, type, 100, sg->skill_lv, sec) ) {
						const struct TimerData* td = tsc->data[type]?timer->get(tsc->data[type]->timer):NULL;
						if( td )
							sec = DIFF_TICK32(td->tick, tick);
						///map->moveblock(bl, src->bl.x, src->bl.y, tick); // in official server it doesn't behave like this. [malufett]
						clif->fixpos(bl);
						sg->val2 = bl->id;
					} else
						sec = 3000; // Couldn't trap it?
					sg->limit = DIFF_TICK32(tick, sg->tick) + sec;
				} else if( tsc->data[SC_THORNS_TRAP] && bl->id == sg->val2 )
					skill->attack(skill->get_type(GN_THORNS_TRAP), ss, ss, bl, sg->skill_id, sg->skill_lv, tick, SD_LEVEL|SD_ANIMATION);
			}
			break;

		case UNT_DEMONIC_FIRE: {
				struct map_session_data *sd =  BL_CAST(BL_PC, ss);
				switch( sg->val2 ) {
					case 1:
					case 2:
					default:
						sc_start4(ss, bl, SC_BURNING, 4 + 4 * sg->skill_lv, sg->skill_lv, 0, ss->id, 0,
								 skill->get_time2(sg->skill_id, sg->skill_lv));
						skill->attack(skill->get_type(sg->skill_id), ss, &src->bl, bl,
									 sg->skill_id, sg->skill_lv + 10 * sg->val2, tick, 0);
						break;
					case 3:
						skill->attack(skill->get_type(CR_ACIDDEMONSTRATION), ss, &src->bl, bl,
									 CR_ACIDDEMONSTRATION, sd ? pc->checkskill(sd, CR_ACIDDEMONSTRATION) : sg->skill_lv, tick, 0);
						break;

				}
			}
			break;

		case UNT_FIRE_EXPANSION_SMOKE_POWDER:
			sc_start(ss, bl, SC_FIRE_EXPANSION_SMOKE_POWDER, 100, sg->skill_lv, 1000);
			break;

		case UNT_FIRE_EXPANSION_TEAR_GAS:
			sc_start(ss, bl, SC_FIRE_EXPANSION_TEAR_GAS, 100, sg->skill_lv, 1000);
			break;

		case UNT_HELLS_PLANT:
			if( battle->check_target(&src->bl,bl,BCT_ENEMY) > 0 )
				skill->attack(skill->get_type(GN_HELLS_PLANT_ATK), ss, &src->bl, bl, GN_HELLS_PLANT_ATK, sg->skill_lv, tick, 0);
			if( ss != bl) //The caster is the only one who can step on the Plants, without destroying them
				sg->limit = DIFF_TICK32(tick, sg->tick) + 100;
			break;

		case UNT_CLOUD_KILL:
			if(tsc && !tsc->data[type])
				status->change_start(ss,bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill->get_time2(sg->skill_id,sg->skill_lv),SCFLAG_FIXEDRATE);
			skill->attack(skill->get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_WARMER:
			{
				// It has effect on everything, including monsters, undead property and demon
				int hp = 0;
				if( ssc && ssc->data[SC_HEATER_OPTION] )
					hp = tstatus->max_hp * 3 * sg->skill_lv / 100;
				else
					hp = tstatus->max_hp * sg->skill_lv / 100;
				if( tstatus->hp != tstatus->max_hp )
					clif->skill_nodamage(&src->bl, bl, AL_HEAL, hp, 0);
				if( tsc && tsc->data[SC_AKAITSUKI] && hp )
					hp = ~hp + 1;
				status->heal(bl, hp, 0, 0);
				sc_start(ss, bl, type, 100, sg->skill_lv, sg->interval + 100);
			}
			break;
		case UNT_FIRE_INSIGNIA:
		case UNT_WATER_INSIGNIA:
		case UNT_WIND_INSIGNIA:
		case UNT_EARTH_INSIGNIA:
		case UNT_ZEPHYR:
			sc_start(ss, bl,type, 100, sg->skill_lv, sg->interval);
			if (sg->unit_id != UNT_ZEPHYR && !battle->check_undead(tstatus->race, tstatus->def_ele)) {
				int hp = tstatus->max_hp / 100; //+1% each 5s
				if ((sg->val3) % 5) { //each 5s
					if (tstatus->def_ele == skill->get_ele(sg->skill_id,sg->skill_lv)) {
						status->heal(bl, hp, 0, 2);
					} else if( (sg->unit_id ==  UNT_FIRE_INSIGNIA && tstatus->def_ele == ELE_EARTH)
					        || (sg->unit_id ==  UNT_WATER_INSIGNIA && tstatus->def_ele == ELE_FIRE)
					        || (sg->unit_id ==  UNT_WIND_INSIGNIA && tstatus->def_ele == ELE_WATER)
					        || (sg->unit_id ==  UNT_EARTH_INSIGNIA && tstatus->def_ele == ELE_WIND)
					) {
						status->heal(bl, -hp, 0, 0);
					}
				}
				sg->val3++; //timer
				if (sg->val3 > 5) sg->val3 = 0;
			}
			break;

		case UNT_VACUUM_EXTREME:
			if (tsc && (tsc->data[SC_HALLUCINATIONWALK] || tsc->data[SC_VACUUM_EXTREME])) {
				return 0;
			} else {
				sg->limit -= 1000 * bst->str/20;
				sc_start(ss, bl, SC_VACUUM_EXTREME, 100, sg->skill_lv, sg->limit);

				if ( !map_flag_gvg(bl->m) && !map->list[bl->m].flag.battleground && !is_boss(bl) ) {
					if (unit->movepos(bl, sg->val1, sg->val2, 0, 0)) {
						clif->slide(bl, sg->val1, sg->val2);
						clif->fixpos(bl);
					}
				}
			}
			break;

		case UNT_FIRE_MANTLE:
			if( battle->check_target(&src->bl, bl, BCT_ENEMY) > 0 )
				skill->attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_ZENKAI_WATER:
		case UNT_ZENKAI_LAND:
		case UNT_ZENKAI_FIRE:
		case UNT_ZENKAI_WIND:
			if( battle->check_target(&src->bl,bl,BCT_ENEMY) > 0 ){
				switch( sg->unit_id ){
					case UNT_ZENKAI_WATER:
						sc_start(ss, bl, SC_COLD, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						sc_start(ss, bl, SC_FREEZE, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						sc_start(ss, bl, SC_FROSTMISTY, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case UNT_ZENKAI_LAND:
						sc_start(ss, bl, SC_STONE, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						sc_start(ss, bl, SC_POISON, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case UNT_ZENKAI_FIRE:
						sc_start4(ss, bl, SC_BURNING, sg->val1*5, sg->skill_lv, 0, ss->id, 0, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case UNT_ZENKAI_WIND:
						sc_start(ss, bl, SC_SILENCE, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						sc_start(ss, bl, SC_SLEEP, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						sc_start(ss, bl, SC_DEEP_SLEEP, sg->val1*5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
				}
			}else
				sc_start2(ss, bl,type,100,sg->val1,sg->val2,skill->get_time2(sg->skill_id, sg->skill_lv));
			break;

		case UNT_LAVA_SLIDE:
			skill->attack(BF_WEAPON, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			if(++sg->val1 > 4) //after 5 stop hit and destroy me
				sg->limit = DIFF_TICK32(tick, sg->tick);
			break;

		case UNT_POISON_MIST:
			skill->attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			status->change_start(ss, bl, SC_BLIND, rnd() % 100 > sg->skill_lv * 10, sg->skill_lv, sg->skill_id, 0, 0,
			                     skill->get_time2(sg->skill_id, sg->skill_lv), SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
			break;
	}

	if (bl->type == BL_MOB && ss != bl)
		mob->skill_event(BL_UCAST(BL_MOB, bl), ss, tick, MSC_SKILLUSED|(skill_id<<16));

	return skill_id;
}
/*==========================================
 * Triggered when a char steps out of a skill cell
 *------------------------------------------*/
int skill_unit_onout(struct skill_unit *src, struct block_list *bl, int64 tick) {
	struct skill_unit_group *sg;
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	nullpo_ret(src);
	nullpo_ret(bl);
	nullpo_ret(sg=src->group);
	sc = status->get_sc(bl);
	type = status->skill2sc(sg->skill_id);
	sce = (sc != NULL && type != SC_NONE) ? sc->data[type] : NULL;

	if( bl->prev == NULL
	 || (status->isdead(bl) && sg->unit_id != UNT_ANKLESNARE && sg->unit_id != UNT_SPIDERWEB && sg->unit_id != UNT_THORNS_TRAP)
	) //Need to delete the trap if the source died.
		return 0;

	switch(sg->unit_id){
		case UNT_SAFETYWALL:
		case UNT_PNEUMA:
		case UNT_NEUTRALBARRIER:
		case UNT_STEALTHFIELD:
			if (sce)
				status_change_end(bl, type, INVALID_TIMER);
			break;

		case UNT_BASILICA:
			if( sce && sce->val4 == src->bl.id )
				status_change_end(bl, type, INVALID_TIMER);
			break;
		case UNT_HERMODE:
			//Clear Hermode if the owner moved.
			if (sce && sce->val3 == BCT_SELF && sce->val4 == sg->group_id)
				status_change_end(bl, type, INVALID_TIMER);
			break;

		case UNT_THORNS_TRAP:
		case UNT_SPIDERWEB:
		{
			struct block_list *target = map->id2bl(sg->val2);
			if (target && target==bl) {
				if (sce && sce->val3 == sg->group_id)
					status_change_end(bl, type, INVALID_TIMER);
				sg->limit = DIFF_TICK32(tick,sg->tick)+1000;
			}
		}
			break;
		case UNT_WHISTLE:
		case UNT_ASSASSINCROSS:
		case UNT_POEMBRAGI:
		case UNT_APPLEIDUN:
		case UNT_HUMMING:
		case UNT_DONTFORGETME:
		case UNT_FORTUNEKISS:
		case UNT_SERVICEFORYOU:
			if (sg->src_id==bl->id && !(sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_BARDDANCER))
				return -1;
	}
	return sg->skill_id;
}

/*==========================================
 * Triggered when a char steps out of a skill group (entirely) [Skotlex]
 *------------------------------------------*/
int skill_unit_onleft(uint16 skill_id, struct block_list *bl, int64 tick) {
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	sc = status->get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	type = status->skill2sc(skill_id);
	sce = (sc != NULL && type != SC_NONE) ? sc->data[type] : NULL;

	switch (skill_id) {
		case WZ_QUAGMIRE:
			if (bl->type==BL_MOB)
				break;
			if (sce)
				status_change_end(bl, type, INVALID_TIMER);
			break;

		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
			if(sc && sc->data[SC_DANCING] && (sc->data[SC_DANCING]->val1&0xFFFF) == skill_id) {
				//Check if you just stepped out of your ensemble skill to cancel dancing. [Skotlex]
				//We don't check for SC_LONGING because someone could always have knocked you back and out of the song/dance.
				//FIXME: This code is not perfect, it doesn't checks for the real ensemble's owner,
				//it only checks if you are doing the same ensemble. So if there's two chars doing an ensemble
				//which overlaps, by stepping outside of the other partner's ensemble will cause you to cancel
				//your own. Let's pray that scenario is pretty unlikely and none will complain too much about it.
				status_change_end(bl, SC_DANCING, INVALID_TIMER);
			}
			/* Fall through */
		case MH_STEINWAND:
		case MG_SAFETYWALL:
		case AL_PNEUMA:
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
		case CG_HERMODE:
		case HW_GRAVITATION:
		case NJ_SUITON:
		case SC_MAELSTROM:
		case EL_WATER_BARRIER:
		case EL_ZEPHYR:
		case EL_POWER_OF_GAIA:
		case SO_ELEMENTAL_SHIELD:
		case SC_BLOODYLUST:
			if (sce)
				status_change_end(bl, type, INVALID_TIMER);
			break;
		case SO_FIRE_INSIGNIA:
		case SO_WATER_INSIGNIA:
		case SO_WIND_INSIGNIA:
		case SO_EARTH_INSIGNIA:
			if (sce && bl->type != BL_ELEM)
				status_change_end(bl, type, INVALID_TIMER);
			break;
		case BA_POEMBRAGI:
		case BA_WHISTLE:
		case BA_ASSASSINCROSS:
		case BA_APPLEIDUN:
		case DC_HUMMING:
		case DC_DONTFORGETME:
		case DC_FORTUNEKISS:
		case DC_SERVICEFORYOU:

			if ((battle_config.song_timer_reset && sce) // eAthena style: update everytime
			  || (!battle_config.song_timer_reset && sce && sce->val4 != 1) // Aegis style: update only when it was not a reduced effect
			) {
				timer->delete(sce->timer, status->change_timer);
				//NOTE: It'd be nice if we could get the skill_lv for a more accurate extra time, but alas...
				//not possible on our current implementation.
				sce->val4 = 1; //Store the fact that this is a "reduced" duration effect.
				sce->timer = timer->add(tick+skill->get_time2(skill_id,1), status->change_timer, bl->id, type);
			}
			break;
		case PF_FOGWALL:
			if (sce) {
				status_change_end(bl, type, INVALID_TIMER);
				if ((sce=sc->data[SC_BLIND])) {
					if (bl->type == BL_PC) //Players get blind ended immediately, others have it still for 30 secs. [Skotlex]
						status_change_end(bl, SC_BLIND, INVALID_TIMER);
					else {
						timer->delete(sce->timer, status->change_timer);
						sce->timer = timer->add(30000+tick, status->change_timer, bl->id, SC_BLIND);
					}
				}
			}
			break;
		case GD_LEADERSHIP:
		case GD_GLORYWOUNDS:
		case GD_SOULCOLD:
		case GD_HAWKEYES:
			if( !(sce && sce->val4) )
				status_change_end(bl, type, INVALID_TIMER);
			break;
	}

	return skill_id;
}

/*==========================================
 * Invoked when a unit cell has been placed/removed/deleted.
 * flag values:
 * flag&1: Invoke onplace function (otherwise invoke onout)
 * flag&4: Invoke a onleft call (the unit might be scheduled for deletion)
 *------------------------------------------*/
int skill_unit_effect(struct block_list* bl, va_list ap) {
	struct skill_unit* su = va_arg(ap,struct skill_unit*);
	struct skill_unit_group* group = su->group;
	int64 tick = va_arg(ap,int64);
	unsigned int flag = va_arg(ap,unsigned int);
	uint16 skill_id;
	bool dissonance;

	if( (!su->alive && !(flag&4)) || bl->prev == NULL )
		return 0;

	nullpo_ret(group);

	dissonance = skill->dance_switch(su, 0);

	//Necessary in case the group is deleted after calling on_place/on_out [Skotlex]
	skill_id = group->skill_id;
	//Target-type check.
	if( !(group->bl_flag&bl->type && battle->check_target(&su->bl,bl,group->target_flag)>0) ) {
		if( (flag&4) && ( group->state.song_dance&0x1 || (group->src_id == bl->id && group->state.song_dance&0x2) ) )
			skill->unit_onleft(skill_id, bl, tick);//Ensemble check to terminate it.
	} else {
		if( flag&1 )
			skill->unit_onplace(su,bl,tick);
		else if (skill->unit_onout(su,bl,tick) == -1)
			return 0; // Don't let a Bard/Dancer update their own song timer

		if( flag&4 )
			skill->unit_onleft(skill_id, bl, tick);
	}

	if( dissonance ) skill->dance_switch(su, 1);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_ondamaged(struct skill_unit *src, struct block_list *bl, int64 damage, int64 tick) {
	struct skill_unit_group *sg;

	nullpo_ret(src);
	nullpo_ret(sg=src->group);

	switch( sg->unit_id ) {
	case UNT_BLASTMINE:
	case UNT_SKIDTRAP:
	case UNT_LANDMINE:
	case UNT_SHOCKWAVE:
	case UNT_SANDMAN:
	case UNT_FLASHER:
	case UNT_CLAYMORETRAP:
	case UNT_FREEZINGTRAP:
	case UNT_TALKIEBOX:
	case UNT_ANKLESNARE:
	case UNT_ICEWALL:
	case UNT_WALLOFTHORN:
		src->val1 -= (int)cap_value(damage,INT_MIN,INT_MAX);
		break;
	case UNT_REVERBERATION:
		src->val1--;
		break;
	default:
		damage = 0;
		break;
	}
	return (int)cap_value(damage,INT_MIN,INT_MAX);
}

/*==========================================
 *
 *------------------------------------------*/
int skill_check_condition_char_sub (struct block_list *bl, va_list ap)
{
	struct map_session_data *tsd = NULL;
	struct block_list *src = va_arg(ap, struct block_list *);
	struct map_session_data *sd = NULL;
	int *c = va_arg(ap, int *);
	int *p_sd = va_arg(ap, int *); //Contains the list of characters found.
	int skill_id = va_arg(ap, int);

	nullpo_ret(bl);
	nullpo_ret(src);
	Assert_ret(bl->type == BL_PC);
	Assert_ret(src->type == BL_PC);
	tsd = BL_UCAST(BL_PC, bl);
	sd = BL_UCAST(BL_PC, src);

	if ( ((skill_id != PR_BENEDICTIO && *c >=1) || *c >=2) && !(skill->get_inf2(skill_id)&INF2_CHORUS_SKILL) )
		return 0; //Partner found for ensembles, or the two companions for Benedictio. [Skotlex]

	if (bl == src)
		return 0;

	if(pc_isdead(tsd))
		return 0;

	if (tsd->sc.data[SC_SILENCE] || ( tsd->sc.opt1 && tsd->sc.opt1 != OPT1_BURNING ))
		return 0;

	if( skill->get_inf2(skill_id)&INF2_CHORUS_SKILL ) {
		if( tsd->status.party_id == sd->status.party_id && (tsd->class_&MAPID_THIRDMASK) == MAPID_MINSTRELWANDERER )
			p_sd[(*c)++] = tsd->bl.id;
		return 1;
	} else {

		switch(skill_id) {
			case PR_BENEDICTIO: {
				uint8 dir = map->calc_dir(&sd->bl,tsd->bl.x,tsd->bl.y);
				dir = (unit->getdir(&sd->bl) + dir)%8; //This adjusts dir to account for the direction the sd is facing.
				if ((tsd->class_&MAPID_BASEMASK) == MAPID_ACOLYTE && (dir == 2 || dir == 6) //Must be standing to the left/right of Priest.
					&& sd->status.sp >= 10)
					p_sd[(*c)++]=tsd->bl.id;
				return 1;
			}
			case AB_ADORAMUS:
			// Adoramus does not consume Blue Gemstone when there is at least 1 Priest class next to the caster
				if( (tsd->class_&MAPID_UPPERMASK) == MAPID_PRIEST )
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			case WL_COMET:
			// Comet does not consume Red Gemstones when there is at least 1 Warlock class next to the caster
				if( ( tsd->class_&MAPID_THIRDMASK ) == MAPID_WARLOCK )
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			case LG_RAYOFGENESIS:
				if( tsd->status.party_id == sd->status.party_id && (tsd->class_&MAPID_THIRDMASK) == MAPID_ROYAL_GUARD &&
					tsd->sc.data[SC_BANDING] )
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			default: //Warning: Assuming Ensemble Dance/Songs for code speed. [Skotlex]
				{
					uint16 skill_lv;
					if(pc_issit(tsd) || !unit->can_move(&tsd->bl))
						return 0;
					if (sd->status.sex != tsd->status.sex &&
							(tsd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER &&
							(skill_lv = pc->checkskill(tsd, skill_id)) > 0 &&
							(tsd->weapontype1==W_MUSICAL || tsd->weapontype1==W_WHIP) &&
							sd->status.party_id && tsd->status.party_id &&
							sd->status.party_id == tsd->status.party_id &&
							!tsd->sc.data[SC_DANCING])
					{
						p_sd[(*c)++]=tsd->bl.id;
						return skill_lv;
					} else {
						return 0;
					}
				}
				break;
		}

	}
	return 0;
}

/*==========================================
 * Checks and stores partners for ensemble skills [Skotlex]
 *------------------------------------------*/
int skill_check_pc_partner (struct map_session_data *sd, uint16 skill_id, uint16* skill_lv, int range, int cast_flag) {
	static int c=0;
	static int p_sd[2] = { 0, 0 };
	int i;
	bool is_chorus = ( skill->get_inf2(skill_id)&INF2_CHORUS_SKILL );

	if (!battle_config.player_skill_partner_check || pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL))
		return is_chorus ? MAX_PARTY : 99; //As if there were infinite partners.

	if (cast_flag) {
		//Execute the skill on the partners.
		struct map_session_data* tsd;
		switch (skill_id) {
			case PR_BENEDICTIO:
				for (i = 0; i < c; i++) {
					if ((tsd = map->id2sd(p_sd[i])) != NULL)
						status->charge(&tsd->bl, 0, 10);
				}
				return c;
			case AB_ADORAMUS:
				if( c > 0 && (tsd = map->id2sd(p_sd[0])) != NULL ) {
					i = 2 * (*skill_lv);
					status->charge(&tsd->bl, 0, i);
				}
				break;
			default: //Warning: Assuming Ensemble skills here (for speed)
				if( is_chorus )
					break;//Chorus skills are not to be parsed as ensambles
				if (c > 0 && sd->sc.data[SC_DANCING] && (tsd = map->id2sd(p_sd[0])) != NULL) {
					sd->sc.data[SC_DANCING]->val4 = tsd->bl.id;
					sc_start4(&tsd->bl,&tsd->bl,SC_DANCING,100,skill_id,sd->sc.data[SC_DANCING]->val2,*skill_lv,sd->bl.id,skill->get_time(skill_id,*skill_lv)+1000);
					clif->skill_nodamage(&tsd->bl, &sd->bl, skill_id, *skill_lv, 1);
					tsd->skill_id_dance = skill_id;
					tsd->skill_lv_dance = *skill_lv;
				}
				return c;
		}
	}

	//Else: new search for partners.
	c = 0;
	memset (p_sd, 0, sizeof(p_sd));
	if( is_chorus )
		i = party->foreachsamemap(skill->check_condition_char_sub,sd,AREA_SIZE,&sd->bl, &c, &p_sd, skill_id, *skill_lv);
	else
		i = map->foreachinrange(skill->check_condition_char_sub, &sd->bl, range, BL_PC, &sd->bl, &c, &p_sd, skill_id);

	if ( skill_id != PR_BENEDICTIO && skill_id != AB_ADORAMUS && skill_id != WL_COMET ) //Apply the average lv to encore skills.
		*skill_lv = (i+(*skill_lv))/(c+1); //I know c should be one, but this shows how it could be used for the average of n partners.
	return c;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_check_condition_mob_master_sub (struct block_list *bl, va_list ap)
{
	const struct mob_data *md = NULL;
	int src_id = va_arg(ap, int);
	int mob_class = va_arg(ap, int);
	int skill_id = va_arg(ap, int);
	int *c = va_arg(ap, int *);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_MOB);
	md = BL_UCCAST(BL_MOB, bl);

	if( md->master_id != src_id
	 || md->special_state.ai != (skill_id == AM_SPHEREMINE?AI_SPHERE:skill_id == KO_ZANZOU?AI_ZANZOU:skill_id == MH_SUMMON_LEGION?AI_ATTACK:AI_FLORA) )
		return 0; //Non alchemist summoned mobs have nothing to do here.
	if(md->class_==mob_class)
		(*c)++;

	return 1;
}

/*==========================================
 * Determines if a given skill should be made to consume ammo
 * when used by the player. [Skotlex]
 *------------------------------------------*/
int skill_isammotype (struct map_session_data *sd, int skill_id)
{
	return (
		battle_config.arrow_decrement==2 &&
		(sd->status.weapon == W_BOW || (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE)) &&
		skill_id != HT_PHANTASMIC &&
		skill->get_type(skill_id) == BF_WEAPON &&
		!(skill->get_nk(skill_id)&NK_NO_DAMAGE) &&
		!skill->get_spiritball(skill_id,1) //Assume spirit spheres are used as ammo instead.
	);
}

/**
 * Checks whether a skill can be used in combos or not
 **/
bool skill_is_combo( int skill_id )
{
	switch( skill_id )
	{
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
		case MO_EXTREMITYFIST:
		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
		case TK_JUMPKICK:
		case HT_POWER:
		case GC_COUNTERSLASH:
		case GC_WEAPONCRUSH:
		case SR_FALLENEMPIRE:
		case SR_DRAGONCOMBO:
		case SR_TIGERCANNON:
		case SR_GATEOFHELL:
			return true;
	}
	return false;
}

int skill_check_condition_castbegin(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv) {
	struct status_data *st;
	struct status_change *sc;
	struct skill_condition require;

	nullpo_ret(sd);

	if (sd->chat_id != 0)
		return 0;

	if (pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL) && sd->skillitem != skill_id) {
		//GMs don't override the skillItem check, otherwise they can use items without them being consumed! [Skotlex]
		sd->state.arrow_atk = skill->get_ammotype(skill_id)?1:0; //Need to do arrow state check.
		sd->spiritball_old = sd->spiritball; //Need to do Spiritball check.
		return 1;
	}

	switch( sd->menuskill_id ) {
		case AM_PHARMACY:
			switch( skill_id ) {
				case AM_PHARMACY:
				case AC_MAKINGARROW:
				case BS_REPAIRWEAPON:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					return 0;
			}
			break;
		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
		case GN_CHANGEMATERIAL:
			if( sd->menuskill_id != skill_id )
				return 0;
			break;
	}
	st = &sd->battle_status;
	sc = &sd->sc;
	if( !sc->count )
		sc = NULL;

	if( sd->skillitem == skill_id ) {
		if( sd->state.abra_flag ) // Hocus-Pocus was used. [Inkfish]
			sd->state.abra_flag = 0;
		else {
			int i;
			// When a target was selected, consume items that were skipped in pc_use_item [Skotlex]
			if( (i = sd->itemindex) == -1 ||
				sd->status.inventory[i].nameid != sd->itemid ||
				sd->inventory_data[i] == NULL ||
				!sd->inventory_data[i]->flag.delay_consume ||
				sd->status.inventory[i].amount < 1
			) {
				//Something went wrong, item exploit?
				sd->itemid = sd->itemindex = -1;
				return 0;
			}
			//Consume
			sd->itemid = sd->itemindex = -1;
			if( skill_id == WZ_EARTHSPIKE && sc && sc->data[SC_EARTHSCROLL] && rnd()%100 > sc->data[SC_EARTHSCROLL]->val2 ) // [marquis007]
				; //Do not consume item.
			else if( sd->status.inventory[i].expire_time == 0 ) // Rental usable items are not consumed until expiration
				pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME);
		}
		return 1;
	}

	if( pc_is90overweight(sd) ) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_WEIGHTOVER,0);
		return 0;
	}

	if( sc && ( sc->data[SC__SHADOWFORM] || sc->data[SC__IGNORANCE] ) )
		return 0;

	switch( skill_id ) { // Turn off check.
		case BS_MAXIMIZE:
		case NV_TRICKDEAD:
		case TF_HIDING:
		case AS_CLOAKING:
		case CR_AUTOGUARD:
		case ML_AUTOGUARD:
		case CR_DEFENDER:
		case ML_DEFENDER:
		case ST_CHASEWALK:
		case PA_GOSPEL:
		case CR_SHRINK:
		case TK_RUN:
		case GS_GATLINGFEVER:
		case TK_READYCOUNTER:
		case TK_READYDOWN:
		case TK_READYSTORM:
		case TK_READYTURN:
		case SG_FUSION:
		case RA_WUGDASH:
		case KO_YAMIKUMO:
			if( sc && sc->data[status->skill2sc(skill_id)] )
				return 1;
		default:
		{
			int ret = skill->check_condition_castbegin_off_unknown(sc, &skill_id);
			if (ret >= 0)
				return ret;
		}
	}

	// Check the skills that can be used while mounted on a warg
	if( pc_isridingwug(sd) ) {
		switch( skill_id ) {
			// Hunter skills
			case HT_SKIDTRAP:
			case HT_LANDMINE:
			case HT_ANKLESNARE:
			case HT_SHOCKWAVE:
			case HT_SANDMAN:
			case HT_FLASHER:
			case HT_FREEZINGTRAP:
			case HT_BLASTMINE:
			case HT_CLAYMORETRAP:
			case HT_TALKIEBOX:
			// Ranger skills
			case RA_DETONATOR:
			case RA_ELECTRICSHOCKER:
			case RA_CLUSTERBOMB:
			case RA_MAGENTATRAP:
			case RA_COBALTTRAP:
			case RA_MAIZETRAP:
			case RA_VERDURETRAP:
			case RA_FIRINGTRAP:
			case RA_ICEBOUNDTRAP:
			case RA_WUGDASH:
			case RA_WUGRIDER:
			case RA_WUGSTRIKE:
			// Other
			case BS_GREED:
			case ALL_FULL_THROTTLE:
				break;
			default: // in official there is no message.
			{
				int ret = skill->check_condition_castbegin_mount_unknown(sc, &skill_id);
				if (ret >= 0)
					return ret;
			}
		}

	}

	// Check the skills that can be used whiled using mado
	if( pc_ismadogear(sd) ) {
		switch ( skill_id ) {
				case BS_GREED:
				case NC_BOOSTKNUCKLE:
				case NC_PILEBUNKER:
				case NC_VULCANARM:
				case NC_FLAMELAUNCHER:
				case NC_COLDSLOWER:
				case NC_ARMSCANNON:
				case NC_ACCELERATION:
				case NC_HOVERING:
				case NC_F_SIDESLIDE:
				case NC_B_SIDESLIDE:
				case NC_SELFDESTRUCTION:
				case NC_SHAPESHIFT:
				case NC_EMERGENCYCOOL:
				case NC_INFRAREDSCAN:
				case NC_ANALYZE:
				case NC_MAGNETICFIELD:
				case NC_NEUTRALBARRIER:
				case NC_STEALTHFIELD:
				case NC_REPAIR:
				case NC_AXEBOOMERANG:
				case NC_POWERSWING:
				case NC_AXETORNADO:
				case NC_SILVERSNIPER:
				case NC_MAGICDECOY:
				case NC_DISJOINT:
				case NC_MAGMA_ERUPTION:
				case ALL_FULL_THROTTLE:
				case NC_MAGMA_ERUPTION_DOTDAMAGE:
					break;
				default:
				{
					int ret = skill->check_condition_castbegin_madogear_unknown(sc, &skill_id);
					if (ret >= 0)
						return ret;
				}
			}
	}

	if( skill_lv < 1 || skill_lv > MAX_SKILL_LEVEL )
		return 0;

	require = skill->get_requirement(sd,skill_id,skill_lv);

	//Can only update state when weapon/arrow info is checked.
	sd->state.arrow_atk = require.ammo?1:0;

	// perform skill-specific checks (and actions)
	switch( skill_id ) {
		case SO_SPELLFIST:
			if(sd->skill_id_old != MG_FIREBOLT && sd->skill_id_old != MG_COLDBOLT && sd->skill_id_old != MG_LIGHTNINGBOLT){
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case SA_CASTCANCEL:
			if(sd->ud.skilltimer == INVALID_TIMER) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case AL_WARP:
			if(!battle_config.duel_allow_teleport && sd->duel_group) { // duel restriction [LuzZza]
				char output[128]; sprintf(output, msg_sd(sd,365), skill->get_name(AL_WARP));
				clif->message(sd->fd, output); //"Duel: Can't use %s in duel."
				return 0;
			}
			break;
		case MO_CALLSPIRITS:
			if(sc && sc->data[SC_RAISINGDRAGON])
				skill_lv += sc->data[SC_RAISINGDRAGON]->val1;
			if(sd->spiritball >= skill_lv) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case MO_FINGEROFFENSIVE:
		case GS_FLING:
		case SR_RAMPAGEBLASTER:
		case SR_RIDEINLIGHTNING:
			if( sd->spiritball > 0 && sd->spiritball < require.spiritball )
				sd->spiritball_old = require.spiritball = sd->spiritball;
			else
				sd->spiritball_old = require.spiritball;
			break;
		case MO_CHAINCOMBO:
			if(!sc)
				return 0;
			if(sc->data[SC_BLADESTOP])
				break;
			if (sc->data[SC_COMBOATTACK]) {
				if( sc->data[SC_COMBOATTACK]->val1 == MO_TRIPLEATTACK )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, MO_TRIPLEATTACK);
			}
			return 0;
		case MO_COMBOFINISH:
			if(!sc)
				return 0;
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if ( sc->data[SC_COMBOATTACK]->val1 == MO_CHAINCOMBO )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, MO_CHAINCOMBO);
			}
			return 0;
		case CH_TIGERFIST:
			if(!sc)
				return 0;
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if ( sc->data[SC_COMBOATTACK]->val1 == MO_COMBOFINISH )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, MO_COMBOFINISH);
			}
			return 0;
		case CH_CHAINCRUSH:
			if(!sc)
				return 0;
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if( sc->data[SC_COMBOATTACK]->val1 == CH_TIGERFIST )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, CH_TIGERFIST);
			}
			return 0;
		case MO_EXTREMITYFIST:
#if 0 //To disable Asura during the 5 min skill block uncomment this block...
			if(sc && sc->data[SC_EXTREMITYFIST])
				return 0;
#endif // 0
			if (sc && (sc->data[SC_BLADESTOP] || sc->data[SC_CURSEDCIRCLE_ATKER]))
				break;
			if (sc && sc->data[SC_COMBOATTACK]) {
				switch(sc->data[SC_COMBOATTACK]->val1) {
					case MO_COMBOFINISH:
					case CH_TIGERFIST:
					case CH_CHAINCRUSH:
						break;
					default:
						return 0;
				}
			} else if (!unit->can_move(&sd->bl)) {
				//Placed here as ST_MOVE_ENABLE should not apply if rooted or on a combo. [Skotlex]
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case TK_MISSION:
			if( (sd->class_&MAPID_UPPERMASK) != MAPID_TAEKWON ) {
				// Cannot be used by Non-Taekwon classes
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case TK_READYCOUNTER:
		case TK_READYDOWN:
		case TK_READYSTORM:
		case TK_READYTURN:
		case TK_JUMPKICK:
			if( (sd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER ) {
				// Soul Linkers cannot use this skill
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
			if ((sd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
				return 0; //Anti-Soul Linker check in case you job-changed with Stances active.
			if(!(sc && sc->data[SC_COMBOATTACK]) || sc->data[SC_COMBOATTACK]->val1 == TK_JUMPKICK)
				return 0; //Combo needs to be ready

			if (sc->data[SC_COMBOATTACK]->val3) { //Kick chain
				//Do not repeat a kick.
				if (sc->data[SC_COMBOATTACK]->val3 != skill_id)
					break;
				status_change_end(&sd->bl, SC_COMBOATTACK, INVALID_TIMER);
				return 0;
			}
			if(sc->data[SC_COMBOATTACK]->val1 != skill_id && !( sd && sd->status.base_level >= 90 && pc->famerank(sd->status.char_id, MAPID_TAEKWON) )) {
				//Cancel combo wait.
				unit->cancel_combo(&sd->bl);
				return 0;
			}
			break; //Combo ready.
		case BD_ADAPTATION:
			{
				int time;
				if(!(sc && sc->data[SC_DANCING]))
				{
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
				time = 1000*(sc->data[SC_DANCING]->val3>>16);
				if (skill->get_time(
					(sc->data[SC_DANCING]->val1&0xFFFF), //Dance Skill ID
					(sc->data[SC_DANCING]->val1>>16)) //Dance Skill LV
					- time < skill->get_time2(skill_id,skill_lv))
				{
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
			}
			break;

		case PR_BENEDICTIO:
			if (skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 0) < 2)
			{
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case SL_SMA:
			if(!(sc && sc->data[SC_SMA_READY]))
				return 0;
			break;

		case HT_POWER:
			if(!(sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == skill_id))
				return 0;
			break;

		case CG_HERMODE:
			if(!npc->check_areanpc(1,sd->bl.m,sd->bl.x,sd->bl.y,skill->get_splash(skill_id, skill_lv)))
			{
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case CG_MOONLIT: //Check there's no wall in the range+1 area around the caster. [Skotlex]
			{
				int i,range = skill->get_splash(skill_id, skill_lv)+1;
				int size = range*2+1;
				for (i=0;i<size*size;i++) {
					int x = sd->bl.x+(i%size-range);
					int y = sd->bl.y+(i/size-range);
					if (map->getcell(sd->bl.m, &sd->bl, x, y, CELL_CHKWALL)) {
						clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
						return 0;
					}
				}
			}
			break;
		case PR_REDEMPTIO:
			{
				int exp;
				if( ((exp = pc->nextbaseexp(sd)) > 0 && get_percentage(sd->status.base_exp, exp) < 1) ||
					((exp = pc->nextjobexp(sd)) > 0 && get_percentage(sd->status.job_exp, exp) < 1)) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0); //Not enough exp.
					return 0;
				}
				break;
			}
		case AM_TWILIGHT2:
		case AM_TWILIGHT3:
			if (!party->skill_check(sd, sd->status.party_id, skill_id, skill_lv))
			{
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM:
			if (sc && sc->data[SC_MIRACLE])
				break;
			if (sd->bl.m == sd->feel_map[skill_id-SG_SUN_WARM].m)
				break;
			clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0;
			break;
		case SG_SUN_COMFORT:
		case SG_MOON_COMFORT:
		case SG_STAR_COMFORT:
			if (sc && sc->data[SC_MIRACLE])
				break;
			if (sd->bl.m == sd->feel_map[skill_id-SG_SUN_COMFORT].m &&
				(battle_config.allow_skill_without_day || pc->sg_info[skill_id-SG_SUN_COMFORT].day_func()))
				break;
			clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0;
		case SG_FUSION:
			if (sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_STAR)
				break;
			//Auron insists we should implement SP consumption when you are not Soul Linked. [Skotlex]
			//Only invoke on skill begin cast (instant cast skill). [Kevin]
			if( require.sp > 0 ) {
				if (st->sp < (unsigned int)require.sp)
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_SP_INSUFFICIENT,0);
				else
					status_zap(&sd->bl, 0, require.sp);
			}
			return 0;
		case GD_BATTLEORDER:
		case GD_REGENERATION:
		case GD_RESTORE:
			if (!map_flag_gvg2(sd->bl.m)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case GD_EMERGENCYCALL:
			// other checks were already done in skillnotok()
			if (!sd->status.guild_id || !sd->state.gmaster_flag)
				return 0;
			break;

		case GS_GLITTERING:
			if(sd->spiritball >= 10) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case NJ_ISSEN:
#ifdef RENEWAL
			if (st->hp < (st->hp/100)) {
#else
			if (st->hp < 2) {
#endif
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case NJ_BUNSINJYUTSU:
			if (!(sc && sc->data[SC_NJ_NEN])) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case NJ_ZENYNAGE:
		case KO_MUCHANAGE:
			if(sd->status.zeny < require.zeny) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_MONEY,0);
				return 0;
			}
			break;
		case PF_HPCONVERSION:
			if (st->sp == st->max_sp)
				return 0; //Unusable when at full SP.
			break;
		case AM_CALLHOMUN: //Can't summon if a hom is already out
			if (sd->status.hom_id && sd->hd && !sd->hd->homunculus.vaporize) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case AM_REST: //Can't vapo homun if you don't have an active homun or it's hp is < 80%
			if (!homun_alive(sd->hd) || sd->hd->battle_status.hp < (sd->hd->battle_status.max_hp*80/100))
			{
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		/**
		 * Arch Bishop
		 **/
		case AB_ANCILLA:
			{
				int count = 0, i;
				for( i = 0; i < MAX_INVENTORY; i ++ )
					if( sd->status.inventory[i].nameid == ITEMID_ANCILLA )
						count += sd->status.inventory[i].amount;
				if( count >= 3 ) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_ANCILLA_NUMOVER, 0);
					return 0;
				}
			}
			break;

		case AB_ADORAMUS:
		/**
		 * Warlock
		 **/
		case WL_COMET:
		{
			int idx;

			if( !require.itemid[0] ) // issue: 7935
				break;
			if (skill->check_pc_partner(sd,skill_id,&skill_lv,1,0) <= 0
			 && ((idx = pc->search_inventory(sd,require.itemid[0])) == INDEX_NOT_FOUND
			    || sd->status.inventory[idx].amount < require.amount[0])
			) {
				//clif->skill_fail(sd,skill_id,USESKILL_FAIL_NEED_ITEM,require.amount[0],require.itemid[0]);
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		}
		case WL_SUMMONFB:
		case WL_SUMMONBL:
		case WL_SUMMONWB:
		case WL_SUMMONSTONE:
		case WL_TETRAVORTEX:
		case WL_RELEASE:
			{
				int j, i = 0;
				for(j = SC_SUMMON1; j <= SC_SUMMON5; j++)
					if( sc && sc->data[j] )
						i++;

				switch(skill_id){
					case WL_TETRAVORTEX:
						if( i < 4 ){
							clif->skill_fail(sd,skill_id,USESKILL_FAIL_CONDITION,0);
							return 0;
						}
						break;
					case WL_RELEASE:
						for(j = SC_SPELLBOOK7; j >= SC_SPELLBOOK1; j--)
							if( sc && sc->data[j] )
								i++;
						if( i == 0 ){
							clif->skill_fail(sd,skill_id,USESKILL_FAIL_SUMMON_NONE,0);
							return 0;
						}
						break;
					default:
						if( i == 5 ){
							clif->skill_fail(sd,skill_id,USESKILL_FAIL_SUMMON,0);
							return 0;
						}
				}
			}
			break;
		/**
		 * Guilotine Cross
		 **/
		case GC_HALLUCINATIONWALK:
			if( sc && (sc->data[SC_HALLUCINATIONWALK] || sc->data[SC_HALLUCINATIONWALK_POSTDELAY]) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case GC_COUNTERSLASH:
		case GC_WEAPONCRUSH:
			if( !(sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == GC_WEAPONBLOCKING) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_GC_WEAPONBLOCKING, 0);
				return 0;
			}
			break;
		/**
		 * Ranger
		 **/
		case RA_WUGMASTERY:
			if( pc_isfalcon(sd) || pc_isridingwug(sd) || sd->sc.data[SC__GROOMY] ) {
				clif->skill_fail(sd,skill_id,sd->sc.data[SC__GROOMY]?USESKILL_FAIL_MANUAL_NOTIFY:USESKILL_FAIL_CONDITION,0);
				return 0;
			}
			break;
		case RA_WUGSTRIKE:
			if( !pc_iswug(sd) && !pc_isridingwug(sd) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_CONDITION,0);
				return 0;
			}
			break;
		case RA_WUGRIDER:
			if( pc_isfalcon(sd) || ( !pc_isridingwug(sd) && !pc_iswug(sd) ) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_CONDITION,0);
				return 0;
			}
			break;
		case RA_WUGDASH:
			if(!pc_isridingwug(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_CONDITION,0);
				return 0;
			}
			break;
		/**
		 * Royal Guard
		 **/
		case LG_BANDING:
			if( sc && sc->data[SC_INSPIRATION] ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case LG_PRESTIGE:
			if( sc && (sc->data[SC_BANDING] || sc->data[SC_INSPIRATION]) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case LG_RAYOFGENESIS:
		case LG_HESPERUSLIT:
			if( sc && sc->data[SC_INSPIRATION]  )
				return 1; // Don't check for partner.
			if( !(sc && sc->data[SC_BANDING]) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL,0);
				return 0;
			}
			if( sc->data[SC_BANDING] &&
				sc->data[SC_BANDING]->val2 < (skill_id == LG_RAYOFGENESIS ? 2 : 3) )
				return 0; // Just fails, no msg here.
			break;
		case SR_FALLENEMPIRE:
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if( sc->data[SC_COMBOATTACK]->val1 == SR_DRAGONCOMBO )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, SR_DRAGONCOMBO);
			}
			return 0;
		case SR_CRESCENTELBOW:
			if( sc && sc->data[SC_CRESCENTELBOW] ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_DUPLICATE, 0);
				return 0;
			}
			break;
		case SR_CURSEDCIRCLE:
			if (map_flag_gvg2(sd->bl.m)) {
				if (map->foreachinrange(mob->count_sub, &sd->bl, skill->get_splash(skill_id, skill_lv), BL_MOB,
				                        MOBID_EMPELIUM, MOBID_S_EMPEL_1, MOBID_S_EMPEL_2)) {
					char output[128];
					sprintf(output, "You're too close to a stone or emperium to do this skill"); /* TODO official response? or message.conf it */
					clif->messagecolor_self(sd->fd, COLOR_RED, output);
					return 0;
				}
			}
			if( sd->spiritball > 0 )
				sd->spiritball_old = require.spiritball = sd->spiritball;
			else {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case SR_GATEOFHELL:
			if( sd->spiritball > 0 )
				sd->spiritball_old = require.spiritball;
			break;
		case SC_MANHOLE:
		case SC_DIMENSIONDOOR:
			if( sc && sc->data[SC_MAGNETICFIELD] ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case WM_GREAT_ECHO: {
				int count;
				count = skill->check_pc_partner(sd, skill_id, &skill_lv, skill->get_splash(skill_id,skill_lv), 0);
				if( count < 1 ) {
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_NEED_HELPER,0);
					return 0;
				} else
					require.sp -= require.sp * 20 * count / 100; //  -20% each W/M in the party.
			}
			break;
		case NC_PILEBUNKER:
			if (sd->equip_index[EQI_HAND_R] < 0
			 || !itemid_is_pilebunker(sd->status.inventory[sd->equip_index[EQI_HAND_R]].nameid)
			 ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_THIS_WEAPON, 0);
				return 0;
			}
			break;
		case NC_HOVERING:
			if (( sd->equip_index[EQI_ACC_L] >= 0 &&  sd->status.inventory[sd->equip_index[EQI_ACC_L]].nameid == ITEMID_HOVERING_BOOSTER ) ||
				( sd->equip_index[EQI_ACC_R] >= 0 &&  sd->status.inventory[sd->equip_index[EQI_ACC_R]].nameid == ITEMID_HOVERING_BOOSTER ));
			else {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0;
			}
			break;
		case SO_FIREWALK:
		case SO_ELECTRICWALK: // Can't be casted until you've walked all cells.
			if( sc && sc->data[SC_PROPERTYWALK] &&
			   sc->data[SC_PROPERTYWALK]->val3 < skill->get_maxcount(sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case SO_EL_CONTROL:
			if( !sd->status.ele_id || !sd->ed ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_EL_SUMMON,0);
				return 0;
			}
			break;
		case RETURN_TO_ELDICASTES:
			if( pc_ismadogear(sd) ) { //Cannot be used if Mado is equipped.
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case CR_REFLECTSHIELD:
			if( sc && sc->data[SC_KYOMU] && rnd()%100 < 5 * sc->data[SC_KYOMU]->val1 ){
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case KO_KAHU_ENTEN:
		case KO_HYOUHU_HUBUKI:
		case KO_KAZEHU_SEIRAN:
		case KO_DOHU_KOUKAI:
			if (sd->charm_type == skill->get_ele(skill_id, skill_lv) && sd->charm_count >= MAX_SPIRITCHARM) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_SUMMON, 0);
				return 0;
			}
			break;
		case KO_KAIHOU:
		case KO_ZENKAI:
			if (sd->charm_type == CHARM_TYPE_NONE || sd->charm_count <= 0) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_SUMMON,0);
				return 0;
			}
			break;
		default:
		{
			int ret = skill->check_condition_castbegin_unknown(sc, &skill_id);
			if (ret >= 0)
				return ret;
		}
	}

	switch(require.state) {
		case ST_HIDING:
			if(!(sc && sc->option&OPTION_HIDE)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_CLOAKING:
			if(!pc_iscloaking(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_HIDDEN:
			if(!pc_ishiding(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_RIDING:
			if (!pc_isridingpeco(sd) && !pc_isridingdragon(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_FALCON:
			if(!pc_isfalcon(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_CARTBOOST:
			if(!(sc && sc->data[SC_CARTBOOST])) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case ST_CART:
			if(!pc_iscarton(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_CART,0);
				return 0;
			}
			break;
		case ST_SHIELD:
			if(sd->status.shield <= 0) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_SIGHT:
			if(!(sc && sc->data[SC_SIGHT])) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_EXPLOSIONSPIRITS:
			if(!(sc && sc->data[SC_EXPLOSIONSPIRITS])) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_EXPLOSIONSPIRITS,0);
				return 0;
			}
			break;
		case ST_RECOV_WEIGHT_RATE:
			if(battle_config.natural_heal_weight_rate <= 100 && sd->weight*100/sd->max_weight >= (unsigned int)battle_config.natural_heal_weight_rate) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_MOVE_ENABLE:
			if (sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == skill_id)
				sd->ud.canmove_tick = timer->gettick(); //When using a combo, cancel the can't move delay to enable the skill. [Skotlex]

			if (!unit->can_move(&sd->bl)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_WATER:
			if (sc && (sc->data[SC_DELUGE] || sc->data[SC_NJ_SUITON]))
				break;
			if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKWATER))
				break;
			clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
			return 0;
		case ST_RIDINGDRAGON:
			if( !pc_isridingdragon(sd) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_DRAGON,0);
				return 0;
			}
			break;
		case ST_WUG:
			if( !pc_iswug(sd) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_RIDINGWUG:
			if( !pc_isridingwug(sd) ){
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case ST_MADO:
			if( !pc_ismadogear(sd) ) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_MADOGEAR,0);
				return 0;
			}
			break;
		case ST_ELEMENTALSPIRIT:
			if(!sd->ed) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_EL_SUMMON,0);
				return 0;
			}
			break;
		case ST_POISONINGWEAPON:
			if (!(sc && sc->data[SC_POISONINGWEAPON])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_GC_POISONINGWEAPON, 0);
				return 0;
			}
			break;
		case ST_ROLLINGCUTTER:
			if (!(sc && sc->data[SC_ROLLINGCUTTER])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0);
				return 0;
			}
			break;
		case ST_MH_FIGHTING:
			if (!(sc && sc->data[SC_STYLE_CHANGE] && sc->data[SC_STYLE_CHANGE]->val2 == MH_MD_FIGHTING)){
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case ST_MH_GRAPPLING:
			if (!(sc && sc->data[SC_STYLE_CHANGE] && sc->data[SC_STYLE_CHANGE]->val2 == MH_MD_GRAPPLING)){
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case ST_PECO:
			if (!pc_isridingpeco(sd)) {
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
	}

	if(require.mhp > 0 && get_percentage(st->hp, st->max_hp) > require.mhp) {
		//mhp is the max-hp-requirement, that is,
		//you must have this % or less of HP to cast it.
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_HP_INSUFFICIENT,0);
		return 0;
	}

	if( require.weapon && !pc_check_weapontype(sd,require.weapon) ) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_THIS_WEAPON,0);
		return 0;
	}

	if( require.sp > 0 && st->sp < (unsigned int)require.sp) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_SP_INSUFFICIENT,0);
		return 0;
	}

	if( require.zeny > 0 && sd->status.zeny < require.zeny ) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_MONEY,0);
		return 0;
	}

	if( require.spiritball > 0 && sd->spiritball < require.spiritball) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_SPIRITS,require.spiritball);
		return 0;
	}

#if 0
	// There's no need to check if the skill is part of a combo if it's
	// already been checked before, see unit_skilluse_id2 [Panikon]
	// Note that if this check is read part of issue:8047 will reappear!
	if( sd->sc.data[SC_COMBOATTACK] && !skill->is_combo(skill_id ) )
		return 0;
#endif // 0

	return 1;
}

int skill_check_condition_castbegin_off_unknown(struct status_change *sc, uint16 *skill_id)
{
    return -1;
}

int skill_check_condition_castbegin_mount_unknown(struct status_change *sc, uint16 *skill_id)
{
    return 0;
}

int skill_check_condition_castbegin_madogear_unknown(struct status_change *sc, uint16 *skill_id)
{
    return 0;
}

int skill_check_condition_castbegin_unknown(struct status_change *sc, uint16 *skill_id)
{
    return -1;
}

int skill_check_condition_castend(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv) {
	struct skill_condition require;
	struct status_data *st;
	int i;
	int index[MAX_SKILL_ITEM_REQUIRE];

	nullpo_ret(sd);

	if (sd->chat_id != 0)
		return 0;

	if( pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL) && sd->skillitem != skill_id ) {
		//GMs don't override the skillItem check, otherwise they can use items without them being consumed! [Skotlex]
		sd->state.arrow_atk = skill->get_ammotype(skill_id)?1:0; //Need to do arrow state check.
		sd->spiritball_old = sd->spiritball; //Need to do Spiritball check.
		return 1;
	}

	switch( sd->menuskill_id ) { // Cast start or cast end??
		case AM_PHARMACY:
			switch( skill_id ) {
				case AM_PHARMACY:
				case AC_MAKINGARROW:
				case BS_REPAIRWEAPON:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					return 0;
			}
			break;
		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
		case GN_CHANGEMATERIAL:
			if( sd->menuskill_id != skill_id )
				return 0;
			break;
	}
	/* temporarily disabled, awaiting for kenpachi to detail this so we can make it work properly */
#if 0
	if( sd->state.abra_flag ) // Casting finished (Hocus-Pocus)
		return 1;
#endif
	if( sd->skillitem == skill_id )
		return 1;
	if( pc_is90overweight(sd) ) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_WEIGHTOVER,0);
		return 0;
	}

	// perform skill-specific checks (and actions)
	switch( skill_id ) {
		case PR_BENEDICTIO:
			skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 1);
			break;
		case AM_CANNIBALIZE:
		case AM_SPHEREMINE: {
			int c=0;
			int maxcount = 0;
			int mob_class = 0;
			if (skill_id == AM_CANNIBALIZE) {
				Assert_retb(skill_lv > 0 && skill_lv <= 5);
				maxcount = 6-skill_lv;
				switch (skill_lv) {
					case 1: mob_class = MOBID_G_MANDRAGORA; break;
					case 2: mob_class = MOBID_G_HYDRA;      break;
					case 3: mob_class = MOBID_G_FLORA;      break;
					case 4: mob_class = MOBID_G_PARASITE;   break;
					case 5: mob_class = MOBID_G_GEOGRAPHER; break;
				}
			} else {
				maxcount = skill->get_maxcount(skill_id,skill_lv);
				mob_class = MOBID_MARINE_SPHERE;
			}
			if(battle_config.land_skill_limit && maxcount>0 && (battle_config.land_skill_limit&BL_PC)) {
				i = map->foreachinmap(skill->check_condition_mob_master_sub ,sd->bl.m, BL_MOB, sd->bl.id, mob_class, skill_id, &c);
				if( c >= maxcount
				 || (skill_id==AM_CANNIBALIZE && c != i && battle_config.summon_flora&2)
				) {
					//Fails when: exceed max limit. There are other plant types already out.
					clif->skill_fail(sd,skill_id,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
			}
			break;
		}
		case NC_SILVERSNIPER:
		case NC_MAGICDECOY: {
				int c = 0;
				int maxcount = skill->get_maxcount(skill_id,skill_lv);

				if( battle_config.land_skill_limit && maxcount > 0 && ( battle_config.land_skill_limit&BL_PC ) ) {
					if (skill_id == NC_MAGICDECOY) {
						int j;
						for (j = MOBID_MAGICDECOY_FIRE; j <= MOBID_MAGICDECOY_WIND; j++)
							map->foreachinmap(skill->check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, j, skill_id, &c);
					} else {
						map->foreachinmap(skill->check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, MOBID_SILVERSNIPER, skill_id, &c);
					}
					if( c >= maxcount ) {
						clif->skill_fail(sd , skill_id, USESKILL_FAIL_SUMMON, 0);
						return 0;
					}
				}
			}
			break;
		case KO_ZANZOU: {
				int c = 0;
				i = map->foreachinmap(skill->check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, MOBID_KO_KAGE, skill_id, &c);
				if( c >= skill->get_maxcount(skill_id,skill_lv) || c != i) {
					clif->skill_fail(sd , skill_id, USESKILL_FAIL_LEVEL, 0);
					return 0;
				}
			}
			break;
		default:
			skill->check_condition_castend_unknown(sd, &skill_id, &skill_lv);
			break;
	}

	st = &sd->battle_status;

	require = skill->get_requirement(sd,skill_id,skill_lv);

	if( require.hp > 0 && st->hp <= (unsigned int)require.hp) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_HP_INSUFFICIENT,0);
		return 0;
	}

	if( require.weapon && !pc_check_weapontype(sd,require.weapon) ) {
		clif->skill_fail(sd,skill_id,USESKILL_FAIL_THIS_WEAPON,0);
		return 0;
	}

	if( require.ammo ) { //Skill requires stuff equipped in the arrow slot.
		if((i=sd->equip_index[EQI_AMMO]) < 0 || !sd->inventory_data[i] ) {
			if( require.ammo&1<<8 )
				clif->skill_fail(sd,skill_id,USESKILL_FAIL_CANONBALL,0);
			else
				clif->arrow_fail(sd,0);
			return 0;
		} else if( sd->status.inventory[i].amount < require.ammo_qty ) {
			char e_msg[100];
			sprintf(e_msg,"Skill Failed. [%s] requires %dx %s.",
						skill->get_desc(skill_id),
						require.ammo_qty,
						itemdb_jname(sd->status.inventory[i].nameid));
			clif->messagecolor_self(sd->fd, COLOR_RED, e_msg);
			return 0;
		}
		if (!(require.ammo&1<<sd->inventory_data[i]->look)) { //Ammo type check. Send the "wrong weapon type" message
			//which is the closest we have to wrong ammo type. [Skotlex]
			clif->arrow_fail(sd,0); //Haplo suggested we just send the equip-arrows message instead. [Skotlex]
			//clif->skill_fail(sd,skill_id,USESKILL_FAIL_THIS_WEAPON,0);
			return 0;
		}
	}

	for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; ++i ) {
		if( !require.itemid[i] )
			continue;
		index[i] = pc->search_inventory(sd,require.itemid[i]);
		if (index[i] == INDEX_NOT_FOUND || sd->status.inventory[index[i]].amount < require.amount[i]) {
			useskill_fail_cause cause = USESKILL_FAIL_NEED_ITEM;
			switch( skill_id ){
				case NC_SILVERSNIPER:
				case NC_MAGICDECOY:
					cause = USESKILL_FAIL_STUFF_INSUFFICIENT;
					break;
				default:
					switch(require.itemid[i]){
						case ITEMID_RED_GEMSTONE:
							cause = USESKILL_FAIL_REDJAMSTONE; break;
						case ITEMID_BLUE_GEMSTONE:
							cause = USESKILL_FAIL_BLUEJAMSTONE; break;
						case ITEMID_HOLY_WATER:
							cause = USESKILL_FAIL_HOLYWATER; break;
						case ITEMID_ANCILLA:
							cause = USESKILL_FAIL_ANCILLA; break;
						case ITEMID_ACCELERATOR:
						case ITEMID_HOVERING_BOOSTER:
						case ITEMID_SUICIDAL_DEVICE:
						case ITEMID_SHAPE_SHIFTER:
						case ITEMID_COOLING_DEVICE:
						case ITEMID_MAGNETIC_FIELD_GENERATOR:
						case ITEMID_BARRIER_BUILDER:
						case ITEMID_CAMOUFLAGE_GENERATOR:
						case ITEMID_REPAIR_KIT:
						case ITEMID_MONKEY_SPANNER:
							cause = USESKILL_FAIL_NEED_EQUIPMENT;
							/* Fall through */
						default:
							clif->skill_fail(sd, skill_id, cause, max(1,require.amount[i])|(require.itemid[i] << 16));
							return 0;
					}
			}
			clif->skill_fail(sd, skill_id, cause, 0);
			return 0;
		}
	}

	return 1;
}

void skill_check_condition_castend_unknown(struct map_session_data* sd, uint16 *skill_id, uint16 *skill_lv) {
}

// type&2: consume items (after skill was used)
// type&1: consume the others (before skill was used)
int skill_consume_requirement( struct map_session_data *sd, uint16 skill_id, uint16 skill_lv, short type) {
	struct skill_condition req;

	nullpo_ret(sd);

	req = skill->get_requirement(sd,skill_id,skill_lv);

	if (type&1) {
		switch( skill_id ) {
			case CG_TAROTCARD: // TarotCard will consume sp in skill_cast_nodamage_id [Inkfish]
			case MC_IDENTIFY:
				req.sp = 0;
				break;
			default:
				if( sd->state.autocast )
					req.sp = 0;
				break;
		}

		if(req.hp || req.sp)
			status_zap(&sd->bl, req.hp, req.sp);

		if(req.spiritball > 0)
			pc->delspiritball(sd,req.spiritball,0);

		if(req.zeny > 0)
		{
			if( skill_id == NJ_ZENYNAGE )
				req.zeny = 0; //Zeny is reduced on skill->attack.
			if( sd->status.zeny < req.zeny )
				req.zeny = sd->status.zeny;
			pc->payzeny(sd,req.zeny,LOG_TYPE_CONSUME,NULL);
		}
	}

	if( type&2 )
	{
		struct status_change *sc = &sd->sc;
		int n,i;

		if( !sc->count )
			sc = NULL;

		for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; ++i )
		{
			if( !req.itemid[i] )
				continue;

			if( itemid_isgemstone(req.itemid[i]) && skill_id != HW_GANBANTEIN && sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_WIZARD )
				continue; //Gemstones are checked, but not subtracted from inventory.

			switch( skill_id ){
				case SA_SEISMICWEAPON:
					if( sc && sc->data[SC_UPHEAVAL_OPTION] && rnd()%100 < 50 )
						continue;
					break;
				case SA_FLAMELAUNCHER:
				case SA_VOLCANO:
					if( sc && sc->data[SC_TROPIC_OPTION] && rnd()%100 < 50 )
						continue;
					break;
				case SA_FROSTWEAPON:
				case SA_DELUGE:
					if( sc && sc->data[SC_CHILLY_AIR_OPTION] && rnd()%100 < 50 )
						continue;
					break;
				case SA_LIGHTNINGLOADER:
				case SA_VIOLENTGALE:
					if( sc && sc->data[SC_WILD_STORM_OPTION] && rnd()%100 < 50 )
						continue;
					break;
			}

			if ((n = pc->search_inventory(sd,req.itemid[i])) != INDEX_NOT_FOUND)
				pc->delitem(sd, n, req.amount[i], 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);
		}
	}

	return 1;
}

struct skill_condition skill_get_requirement(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv) {
	struct skill_condition req;
	struct status_data *st;
	struct status_change *sc;
	int i,hp_rate,sp_rate, sp_skill_rate_bonus = 100;
	uint16 idx;

	memset(&req,0,sizeof(req));

	if( !sd )
		return req;
#if 0 /* temporarily disabled, awaiting for kenpachi to detail this so we can make it work properly */
	if( sd->state.abra_flag )
#else // not 0
	if( sd->skillitem == skill_id )
#endif // 0
		return req; // Hocus-Pocus don't have requirements.

	sc = &sd->sc;
	if( !sc->count )
		sc = NULL;

	switch( skill_id ) { // Turn off check.
		case BS_MAXIMIZE:
		case NV_TRICKDEAD:
		case TF_HIDING:
		case AS_CLOAKING:
		case CR_AUTOGUARD:
		case ML_AUTOGUARD:
		case CR_DEFENDER:
		case ML_DEFENDER:
		case ST_CHASEWALK:
		case PA_GOSPEL:
		case CR_SHRINK:
		case TK_RUN:
		case GS_GATLINGFEVER:
		case TK_READYCOUNTER:
		case TK_READYDOWN:
		case TK_READYSTORM:
		case TK_READYTURN:
		case SG_FUSION:
		case KO_YAMIKUMO:
			if( sc && sc->data[status->skill2sc(skill_id)] )
				return req;
			/* Fall through */
		default:
			if (skill->get_requirement_off_unknown(sc, &skill_id))
				return req;
			break;
	}

	idx = skill->get_index(skill_id);
	if( idx == 0 ) // invalid skill id
		return req;
	if( skill_lv < 1 || skill_lv > MAX_SKILL_LEVEL )
		return req;

	st = &sd->battle_status;

	req.hp = skill->dbs->db[idx].hp[skill_lv-1];
	hp_rate = skill->dbs->db[idx].hp_rate[skill_lv-1];
	if(hp_rate > 0)
		req.hp += (st->hp * hp_rate)/100;
	else
		req.hp += (st->max_hp * (-hp_rate))/100;

	req.sp = skill->dbs->db[idx].sp[skill_lv-1];
	if((sd->skill_id_old == BD_ENCORE) && skill_id == sd->skill_id_dance)
		req.sp /= 2;
	sp_rate = skill->dbs->db[idx].sp_rate[skill_lv-1];
	if(sp_rate > 0)
		req.sp += (st->sp * sp_rate)/100;
	else
		req.sp += (st->max_sp * (-sp_rate))/100;
	if( sd->dsprate != 100 )
		req.sp = req.sp * sd->dsprate / 100;

	ARR_FIND(0, ARRAYLENGTH(sd->skillusesprate), i, sd->skillusesprate[i].id == skill_id);
	if( i < ARRAYLENGTH(sd->skillusesprate) )
		sp_skill_rate_bonus += sd->skillusesprate[i].val;
	ARR_FIND(0, ARRAYLENGTH(sd->skillusesp), i, sd->skillusesp[i].id == skill_id);
	if( i < ARRAYLENGTH(sd->skillusesp) )
		req.sp -= sd->skillusesp[i].val;

	req.sp = cap_value(req.sp * sp_skill_rate_bonus / 100, 0, SHRT_MAX);

	if (sc) {
		if (sc->data[SC__LAZINESS])
			req.sp += req.sp + sc->data[SC__LAZINESS]->val1 * 10;
		if (sc->data[SC_UNLIMITED_HUMMING_VOICE])
			req.sp += req.sp * sc->data[SC_UNLIMITED_HUMMING_VOICE]->val3 / 100;
		if (sc->data[SC_RECOGNIZEDSPELL])
			req.sp += req.sp / 4;
		if (sc->data[SC_TELEKINESIS_INTENSE] && skill->get_ele(skill_id, skill_lv) == ELE_GHOST)
			req.sp -= req.sp * sc->data[SC_TELEKINESIS_INTENSE]->val2 / 100;
		if (sc->data[SC_TARGET_ASPD])
			req.sp -= req.sp * sc->data[SC_TARGET_ASPD]->val1 / 100;
		if (sc->data[SC_MVPCARD_MISTRESS])
			req.sp -= req.sp * sc->data[SC_MVPCARD_MISTRESS]->val1 / 100;
	}

	req.zeny = skill->dbs->db[idx].zeny[skill_lv-1];

	if( sc && sc->data[SC__UNLUCKY] )
		req.zeny += sc->data[SC__UNLUCKY]->val1 * 500;

	req.spiritball = skill->dbs->db[idx].spiritball[skill_lv-1];

	req.state = skill->dbs->db[idx].state;

	req.mhp = skill->dbs->db[idx].mhp[skill_lv-1];

	req.weapon = skill->dbs->db[idx].weapon;

	req.ammo_qty = skill->dbs->db[idx].ammo_qty[skill_lv-1];
	if (req.ammo_qty)
		req.ammo = skill->dbs->db[idx].ammo;

	if (!req.ammo && skill_id && skill->isammotype(sd, skill_id)) {
		//Assume this skill is using the weapon, therefore it requires arrows.
		req.ammo = 0xFFFFFFFF; //Enable use on all ammo types.
		req.ammo_qty = 1;
	}

	for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++ ) {
		if( (skill_id == AM_POTIONPITCHER || skill_id == CR_SLIMPITCHER || skill_id == CR_CULTIVATION) && i != skill_lv%11 - 1 )
			continue;

		switch( skill_id ) {
			case AM_CALLHOMUN:
				if (sd->status.hom_id) //Don't delete items when hom is already out.
					continue;
				break;
			case NC_SHAPESHIFT:
				if( i < 4 )
					continue;
				break;
			case WZ_FIREPILLAR: // celest
				if (skill_lv <= 5) // no gems required at level 1-5
					continue;
				break;
			case AB_ADORAMUS:
				if( itemid_isgemstone(skill->dbs->db[idx].itemid[i]) && skill->check_pc_partner(sd,skill_id,&skill_lv, 1, 2) )
					continue;
				break;
			case WL_COMET:
				if( itemid_isgemstone(skill->dbs->db[idx].itemid[i]) && skill->check_pc_partner(sd,skill_id,&skill_lv, 1, 0) )
					continue;
				break;
			case GN_FIRE_EXPANSION:
				if( i < 5 )
					continue;
				break;
			case SO_SUMMON_AGNI:
			case SO_SUMMON_AQUA:
			case SO_SUMMON_VENTUS:
			case SO_SUMMON_TERA:
			case SO_WATER_INSIGNIA:
			case SO_FIRE_INSIGNIA:
			case SO_WIND_INSIGNIA:
			case SO_EARTH_INSIGNIA:
				if( i < 3 )
					continue;
				break;
			default:
			{
				if (skill->get_requirement_item_unknown(sc, sd, &skill_id, &skill_lv, &idx, &i))
					continue;
				break;
			}
		}

		req.itemid[i] = skill->dbs->db[idx].itemid[i];
		req.amount[i] = skill->dbs->db[idx].amount[i];

		if (itemid_isgemstone(req.itemid[i]) && skill_id != HW_GANBANTEIN) {
			if (sd->special_state.no_gemstone) {
				// All gem skills except Hocus Pocus and Ganbantein can cast for free with Mistress card [helvetica]
				if (skill_id != SA_ABRACADABRA)
					req.itemid[i] = req.amount[i] = 0;
				else if (--req.amount[i] < 1)
					req.amount[i] = 1; // Hocus Pocus always use at least 1 gem
			}
			if (sc && sc->data[SC_INTOABYSS]) {
				if (skill_id != SA_ABRACADABRA)
					req.itemid[i] = req.amount[i] = 0;
				else if (--req.amount[i] < 1)
					req.amount[i] = 1; // Hocus Pocus always use at least 1 gem
			}
			if (sc && sc->data[SC_MVPCARD_MISTRESS]) {
				req.itemid[i] = req.amount[i] = 0;
			}
		}
		if (skill_id >= HT_SKIDTRAP && skill_id <= HT_TALKIEBOX && pc->checkskill(sd, RA_RESEARCHTRAP) > 0) {
			int16 item_index;
			if ((item_index = pc->search_inventory(sd, req.itemid[i])) == INDEX_NOT_FOUND
			  || sd->status.inventory[item_index].amount < req.amount[i]
			) {
				req.itemid[i] = ITEMID_TRAP_ALLOY;
				req.amount[i] = 1;
			}
			break;
		}
	}

	/* requirements are level-dependent */
	switch( skill_id ) {
		case NC_SHAPESHIFT:
		case GN_FIRE_EXPANSION:
		case SO_SUMMON_AGNI:
		case SO_SUMMON_AQUA:
		case SO_SUMMON_VENTUS:
		case SO_SUMMON_TERA:
		case SO_WATER_INSIGNIA:
		case SO_FIRE_INSIGNIA:
		case SO_WIND_INSIGNIA:
		case SO_EARTH_INSIGNIA:
			req.itemid[skill_lv-1] = skill->dbs->db[idx].itemid[skill_lv-1];
			req.amount[skill_lv-1] = skill->dbs->db[idx].amount[skill_lv-1];
			break;
	}
	if (skill_id == NC_REPAIR) {
		switch(skill_lv) {
			case 1:
			case 2:
				req.itemid[1] = ITEMID_REPAIR_A;
				break;
			case 3:
			case 4:
				req.itemid[1] = ITEMID_REPAIR_B;
				break;
			case 5:
				req.itemid[1] = ITEMID_REPAIR_C;
				break;
		}
		req.amount[1] = 1;
	}

	// Check for cost reductions due to skills & SCs
	switch(skill_id) {
		case MC_MAMMONITE:
			if(pc->checkskill(sd,BS_UNFAIRLYTRICK)>0)
				req.zeny -= req.zeny*10/100;
			break;
		case AL_HOLYLIGHT:
			if(sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_PRIEST)
				req.sp *= 5;
			break;
		case SL_SMA:
		case SL_STUN:
		case SL_STIN:
		{
			int kaina_lv = pc->checkskill(sd,SL_KAINA);

			if(kaina_lv==0 || sd->status.base_level<70)
				break;
			if(sd->status.base_level>=90)
				req.sp -= req.sp*7*kaina_lv/100;
			else if(sd->status.base_level>=80)
				req.sp -= req.sp*5*kaina_lv/100;
			else if(sd->status.base_level>=70)
				req.sp -= req.sp*3*kaina_lv/100;
		}
			break;
		case MO_TRIPLEATTACK:
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
			if(sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_MONK)
				req.sp -= req.sp*25/100; //FIXME: Need real data. this is a custom value.
			break;
		case MO_BODYRELOCATION:
			if( sc && sc->data[SC_EXPLOSIONSPIRITS] )
				req.spiritball = 0;
			break;
		case MO_EXTREMITYFIST:
			if( sc )
			{
				if( sc->data[SC_BLADESTOP] )
					req.spiritball--;
				else if( sc->data[SC_COMBOATTACK] )
				{
					switch( sc->data[SC_COMBOATTACK]->val1 )
					{
						case MO_COMBOFINISH:
							req.spiritball = 4;
							break;
						case CH_TIGERFIST:
							req.spiritball = 3;
							break;
						case CH_CHAINCRUSH: //It should consume whatever is left as long as it's at least 1.
							req.spiritball = sd->spiritball?sd->spiritball:1;
							break;
					}
				}else if( sc->data[SC_RAISINGDRAGON] && sd->spiritball > 5)
					req.spiritball = sd->spiritball; // must consume all regardless of the amount required
			}
			break;
		case SR_RAMPAGEBLASTER:
			req.spiritball = sd->spiritball?sd->spiritball:15;
			break;
		case SR_GATEOFHELL:
			if( sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == SR_FALLENEMPIRE )
				req.sp -= req.sp * 10 / 100;
			break;
		case SO_SUMMON_AGNI:
		case SO_SUMMON_AQUA:
		case SO_SUMMON_VENTUS:
		case SO_SUMMON_TERA:
		{
			int spirit_sympathy = pc->checkskill(sd,SO_EL_SYMPATHY);
			if (spirit_sympathy)
				req.sp -= req.sp * (5 + 5 * spirit_sympathy) / 100;
		}
			break;
		case SO_PSYCHIC_WAVE:
			if( sc && (sc->data[SC_HEATER_OPTION] || sc->data[SC_COOLER_OPTION] || sc->data[SC_BLAST_OPTION] ||  sc->data[SC_CURSED_SOIL_OPTION] ))
				req.sp += req.sp * 150 / 100;
			break;
		default:
			skill->get_requirement_unknown(sc, sd, &skill_id, &skill_lv, &req);
			break;
	}

	return req;
}

bool skill_get_requirement_off_unknown(struct status_change *sc, uint16 *skill_id)
{
    return false;
}

bool skill_get_requirement_item_unknown(struct status_change *sc, struct map_session_data* sd, uint16 *skill_id, uint16 *skill_lv, uint16 *idx, int *i)
{
    return false;
}

void skill_get_requirement_unknown(struct status_change *sc, struct map_session_data* sd, uint16 *skill_id, uint16 *skill_lv, struct skill_condition *req)
{
}

/*==========================================
 * Does cast-time reductions based on dex, item bonuses and config setting
 *------------------------------------------*/
int skill_castfix (struct block_list *bl, uint16 skill_id, uint16 skill_lv) {
	int time = skill->get_cast(skill_id, skill_lv);

	nullpo_ret(bl);
#ifndef RENEWAL_CAST
	{
		struct map_session_data *sd;

		sd = BL_CAST(BL_PC, bl);

		// calculate base cast time (reduced by dex)
		if( !(skill->get_castnodex(skill_id, skill_lv)&1) ) {
			int scale = battle_config.castrate_dex_scale - status_get_dex(bl);
			if( scale > 0 ) // not instant cast
				time = time * scale / battle_config.castrate_dex_scale;
			else
				return 0; // instant cast
		}

		// calculate cast time reduced by item/card bonuses
		if( !(skill->get_castnodex(skill_id, skill_lv)&4) && sd )
		{
			int i;
			if( sd->castrate != 100 )
				time = time * sd->castrate / 100;
			for( i = 0; i < ARRAYLENGTH(sd->skillcast) && sd->skillcast[i].id; i++ )
			{
				if( sd->skillcast[i].id == skill_id )
				{
					time+= time * sd->skillcast[i].val / 100;
					break;
				}
			}
		}

	}
#endif
	// config cast time multiplier
	if (battle_config.cast_rate != 100)
		time = time * battle_config.cast_rate / 100;
	// return final cast time
	time = max(time, 0);

	//ShowInfo("Castime castfix = %d\n",time);
	return time;
}

/*==========================================
 * Does cast-time reductions based on sc data.
 *------------------------------------------*/
int skill_castfix_sc (struct block_list *bl, int time) {
	struct status_change *sc = status->get_sc(bl);

	if( time < 0 )
		return 0;

	if( bl->type == BL_MOB ) // mobs casttime is fixed nothing to alter.
		return time;

	if (sc && sc->count) {
		if (sc->data[SC_SLOWCAST])
			time += time * sc->data[SC_SLOWCAST]->val2 / 100;
		if (sc->data[SC_NEEDLE_OF_PARALYZE])
			time += sc->data[SC_NEEDLE_OF_PARALYZE]->val3;
		if (sc->data[SC_SUFFRAGIUM]) {
			time -= time * sc->data[SC_SUFFRAGIUM]->val2 / 100;
			status_change_end(bl, SC_SUFFRAGIUM, INVALID_TIMER);
		}
		if (sc->data[SC_MEMORIZE]) {
			time>>=1;
			if ((--sc->data[SC_MEMORIZE]->val2) <= 0)
				status_change_end(bl, SC_MEMORIZE, INVALID_TIMER);
		}
		if (sc->data[SC_POEMBRAGI])
			time -= time * sc->data[SC_POEMBRAGI]->val2 / 100;
		if (sc->data[SC_IZAYOI])
			time -= time * 50 / 100;
	}
	time = max(time, 0);

	//ShowInfo("Castime castfix_sc = %d\n",time);
	return time;
}
int skill_vfcastfix(struct block_list *bl, double time, uint16 skill_id, uint16 skill_lv) {
#ifdef RENEWAL_CAST
	struct status_change *sc = status->get_sc(bl);
	struct map_session_data *sd = BL_CAST(BL_PC,bl);
	int fixed = skill->get_fixed_cast(skill_id, skill_lv), fixcast_r = 0, varcast_r = 0, i = 0;

	if( time < 0 )
		return 0;

	if( bl->type == BL_MOB ) // mobs casttime is fixed nothing to alter.
		return (int)time;

	if( fixed == 0 ){
		fixed = (int)time * 20 / 100; // fixed time
		time = time * 80 / 100; // variable time
	}else if( fixed < 0 ) // no fixed cast time
		fixed = 0;

	if(sd  && !(skill->get_castnodex(skill_id, skill_lv)&4) ){ // Increases/Decreases fixed/variable cast time of a skill by item/card bonuses.
		if( sd->bonus.varcastrate < 0 )
			VARCAST_REDUCTION(sd->bonus.varcastrate);
		if( sd->bonus.add_varcast != 0 ) // bonus bVariableCast
			time += sd->bonus.add_varcast;
		if( sd->bonus.add_fixcast != 0 ) // bonus bFixedCast
			fixed += sd->bonus.add_fixcast;
		for (i = 0; i < ARRAYLENGTH(sd->skillfixcast) && sd->skillfixcast[i].id; i++)
			if (sd->skillfixcast[i].id == skill_id){ // bonus2 bSkillFixedCast
				fixed += sd->skillfixcast[i].val;
				break;
			}
		for( i = 0; i < ARRAYLENGTH(sd->skillvarcast) && sd->skillvarcast[i].id; i++ )
			if( sd->skillvarcast[i].id == skill_id ){ // bonus2 bSkillVariableCast
				time += sd->skillvarcast[i].val;
				break;
			}
		for( i = 0; i < ARRAYLENGTH(sd->skillcast) && sd->skillcast[i].id; i++ )
			if( sd->skillcast[i].id == skill_id ){ // bonus2 bVariableCastrate
				if( (i=sd->skillcast[i].val) < 0)
					VARCAST_REDUCTION(i);
				break;
			}
		for( i = 0; i < ARRAYLENGTH(sd->skillfixcastrate) && sd->skillfixcastrate[i].id; i++ )
			if( sd->skillfixcastrate[i].id == skill_id ){ // bonus2 bFixedCastrate
				fixcast_r = sd->skillfixcastrate[i].val;
				break;
			}
	}

	if (sc && sc->count && !(skill->get_castnodex(skill_id, skill_lv)&2) ) {
		// All variable cast additive bonuses must come first
		if (sc->data[SC_SLOWCAST])
			VARCAST_REDUCTION(-sc->data[SC_SLOWCAST]->val2);
		if (sc->data[SC_FROSTMISTY])
			VARCAST_REDUCTION(-15);

		// Variable cast reduction bonuses
		if (sc->data[SC_SUFFRAGIUM]) {
			VARCAST_REDUCTION(sc->data[SC_SUFFRAGIUM]->val2);
			status_change_end(bl, SC_SUFFRAGIUM, INVALID_TIMER);
		}
		if (sc->data[SC_MEMORIZE]) {
			VARCAST_REDUCTION(50);
			if ((--sc->data[SC_MEMORIZE]->val2) <= 0)
				status_change_end(bl, SC_MEMORIZE, INVALID_TIMER);
		}
		if (sc->data[SC_POEMBRAGI])
			VARCAST_REDUCTION(sc->data[SC_POEMBRAGI]->val2);
		if (sc->data[SC_IZAYOI])
			VARCAST_REDUCTION(50);
		if (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 3 && (skill->get_ele(skill_id, skill_lv) == ELE_WATER))
			VARCAST_REDUCTION(30); //Reduces 30% Variable Cast Time of Water spells.
		if (sc->data[SC_TELEKINESIS_INTENSE])
			VARCAST_REDUCTION(sc->data[SC_TELEKINESIS_INTENSE]->val2);
		if (sc->data[SC_SOULLINK]){
			if(sc->data[SC_SOULLINK]->val2 == SL_WIZARD || sc->data[SC_SOULLINK]->val2 == SL_BARDDANCER)
				switch(skill_id){
					case WZ_FIREPILLAR:
						if(skill_lv < 5)
							break;
					case HW_GRAVITATION:
					case MG_SAFETYWALL:
					case MG_STONECURSE:
					case BA_MUSICALSTRIKE:
					case DC_THROWARROW:
						VARCAST_REDUCTION(50);
				}
		}
		if (sc->data[SC_MYSTICSCROLL])
			VARCAST_REDUCTION(sc->data[SC_MYSTICSCROLL]->val1);

		// Fixed cast reduction bonuses
		if( sc->data[SC__LAZINESS] )
			fixcast_r = max(fixcast_r, sc->data[SC__LAZINESS]->val2);
		if( sc->data[SC_DANCE_WITH_WUG])
			fixcast_r = max(fixcast_r, sc->data[SC_DANCE_WITH_WUG]->val4);
		if( sc->data[SC_SECRAMENT] )
			fixcast_r = max(fixcast_r, sc->data[SC_SECRAMENT]->val2);
		if (sd && skill_id >= WL_WHITEIMPRISON && skill_id < WL_FREEZE_SP) {
			int radius_lv = pc->checkskill(sd, WL_RADIUS);
			if (radius_lv)
				fixcast_r = max(fixcast_r, (status_get_int(bl) + status->get_lv(bl)) / 15 + radius_lv * 5); // [{(Caster?s INT / 15) + (Caster?s Base Level / 15) + (Radius Skill Level x 5)}] %
		}
		if (sc->data[SC_FENRIR_CARD])
			fixcast_r = max(fixcast_r, sc->data[SC_FENRIR_CARD]->val2);
		if (sc->data[SC_MAGIC_CANDY])
			fixcast_r = max(fixcast_r, sc->data[SC_MAGIC_CANDY]->val2);

		// Fixed cast non percentage bonuses
		if( sc->data[SC_MANDRAGORA] )
			fixed += sc->data[SC_MANDRAGORA]->val1 * 500;
		if( sc->data[SC_IZAYOI] )
			fixed = 0;
		if( sc->data[SC_GUST_OPTION] || sc->data[SC_BLAST_OPTION] || sc->data[SC_WILD_STORM_OPTION] )
			fixed -= 1000;
	}

	if( sd && !(skill->get_castnodex(skill_id, skill_lv)&4) ){
		VARCAST_REDUCTION( max(sd->bonus.varcastrate, 0) + max(i, 0) );
		fixcast_r = max(fixcast_r, sd->bonus.fixcastrate) + min(sd->bonus.fixcastrate,0);
		for( i = 0; i < ARRAYLENGTH(sd->skillcast) && sd->skillcast[i].id; i++ )
			if( sd->skillcast[i].id == skill_id ){ // bonus2 bVariableCastrate
				if( (i=sd->skillcast[i].val) > 0)
					VARCAST_REDUCTION(i);
				break;
			}
	}

	if( varcast_r < 0 ) // now compute overall factors
		time = time * (1 - (float)varcast_r / 100);
	if( !(skill->get_castnodex(skill_id, skill_lv)&1) )// reduction from status point
		time = (1 - sqrt( ((float)(status_get_dex(bl)*2 + status_get_int(bl)) / battle_config.vcast_stat_scale) )) * time;
	// underflow checking/capping
	time = max(time, 0) + (1 - (float)min(fixcast_r, 100) / 100) * max(fixed,0);
#endif
	return (int)time;
}

/*==========================================
 * Does delay reductions based on dex/agi, sc data, item bonuses, ...
 *------------------------------------------*/
int skill_delay_fix (struct block_list *bl, uint16 skill_id, uint16 skill_lv) {
	int delaynodex = skill->get_delaynodex(skill_id, skill_lv);
	int time = skill->get_delay(skill_id, skill_lv);
	struct map_session_data *sd;
	struct status_change *sc = status->get_sc(bl);

	nullpo_ret(bl);
	sd = BL_CAST(BL_PC, bl);

	if (skill_id == SA_ABRACADABRA || skill_id == WM_RANDOMIZESPELL)
		return 0; //Will use picked skill's delay.

	if (bl->type&battle_config.no_skill_delay)
		return battle_config.min_skill_delay_limit;

	if (time < 0)
		time = -time + status_get_amotion(bl); // If set to <0, add to attack motion.

	// Delay reductions
	switch (skill_id) {
		//Monk combo skills have their delay reduced by agi/dex.
		case MO_TRIPLEATTACK:
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
		case SR_DRAGONCOMBO:
		case SR_FALLENEMPIRE:
			time -= 4*status_get_agi(bl) - 2*status_get_dex(bl);
			break;
		case HP_BASILICA:
			if( sc && !sc->data[SC_BASILICA] )
				time = 0; // There is no Delay on Basilica creation, only on cancel
			break;
		default:
			if (battle_config.delay_dependon_dex && !(delaynodex&1)) {
				// if skill delay is allowed to be reduced by dex
				int scale = battle_config.castrate_dex_scale - status_get_dex(bl);
				if (scale > 0)
					time = time * scale / battle_config.castrate_dex_scale;
				else //To be capped later to minimum.
					time = 0;
			}
			if (battle_config.delay_dependon_agi && !(delaynodex&1)) {
				// if skill delay is allowed to be reduced by agi
				int scale = battle_config.castrate_dex_scale - status_get_agi(bl);
				if (scale > 0)
					time = time * scale / battle_config.castrate_dex_scale;
				else //To be capped later to minimum.
					time = 0;
			}
	}

	if ( sc && sc->data[SC_SOULLINK] ) {
		switch (skill_id) {
			case CR_SHIELDBOOMERANG:
				if (sc->data[SC_SOULLINK]->val2 == SL_CRUSADER)
					time /= 2;
				break;
			case AS_SONICBLOW:
				if (!map_flag_gvg2(bl->m) && !map->list[bl->m].flag.battleground && sc->data[SC_SOULLINK]->val2 == SL_ASSASIN)
					time /= 2;
				break;
		}
	}

	if (!(delaynodex&2))
	{
		if (sc && sc->count) {
			if (sc->data[SC_POEMBRAGI])
				time -= time * sc->data[SC_POEMBRAGI]->val3 / 100;
			if (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 3 && (skill->get_ele(skill_id, skill_lv) == ELE_WIND))
				time /= 2; // After Delay of Wind element spells reduced by 50%.
		}

	}

	if( !(delaynodex&4) && sd && sd->delayrate != 100 )
		time = time * sd->delayrate / 100;

	if (battle_config.delay_rate != 100)
		time = time * battle_config.delay_rate / 100;

	//min delay
	time = max(time, status_get_amotion(bl)); // Delay can never be below amotion [Playtester]
	time = max(time, battle_config.min_skill_delay_limit);

//        ShowInfo("Delay delayfix = %d\n",time);
	return time;
}

/*=========================================
 *
 *-----------------------------------------*/
struct square {
	int val1[5];
	int val2[5];
};

void skill_brandishspear_first (struct square *tc, uint8 dir, int16 x, int16 y) {
	nullpo_retv(tc);

	if(dir == 0){
		tc->val1[0]=x-2;
		tc->val1[1]=x-1;
		tc->val1[2]=x;
		tc->val1[3]=x+1;
		tc->val1[4]=x+2;
		tc->val2[0]=
		tc->val2[1]=
		tc->val2[2]=
		tc->val2[3]=
		tc->val2[4]=y-1;
	} else if(dir==2){
		tc->val1[0]=
		tc->val1[1]=
		tc->val1[2]=
		tc->val1[3]=
		tc->val1[4]=x+1;
		tc->val2[0]=y+2;
		tc->val2[1]=y+1;
		tc->val2[2]=y;
		tc->val2[3]=y-1;
		tc->val2[4]=y-2;
	} else if(dir==4){
		tc->val1[0]=x-2;
		tc->val1[1]=x-1;
		tc->val1[2]=x;
		tc->val1[3]=x+1;
		tc->val1[4]=x+2;
		tc->val2[0]=
		tc->val2[1]=
		tc->val2[2]=
		tc->val2[3]=
		tc->val2[4]=y+1;
	} else if(dir==6){
		tc->val1[0]=
		tc->val1[1]=
		tc->val1[2]=
		tc->val1[3]=
		tc->val1[4]=x-1;
		tc->val2[0]=y+2;
		tc->val2[1]=y+1;
		tc->val2[2]=y;
		tc->val2[3]=y-1;
		tc->val2[4]=y-2;
	} else if(dir==1){
		tc->val1[0]=x-1;
		tc->val1[1]=x;
		tc->val1[2]=x+1;
		tc->val1[3]=x+2;
		tc->val1[4]=x+3;
		tc->val2[0]=y-4;
		tc->val2[1]=y-3;
		tc->val2[2]=y-1;
		tc->val2[3]=y;
		tc->val2[4]=y+1;
	} else if(dir==3){
		tc->val1[0]=x+3;
		tc->val1[1]=x+2;
		tc->val1[2]=x+1;
		tc->val1[3]=x;
		tc->val1[4]=x-1;
		tc->val2[0]=y-1;
		tc->val2[1]=y;
		tc->val2[2]=y+1;
		tc->val2[3]=y+2;
		tc->val2[4]=y+3;
	} else if(dir==5){
		tc->val1[0]=x+1;
		tc->val1[1]=x;
		tc->val1[2]=x-1;
		tc->val1[3]=x-2;
		tc->val1[4]=x-3;
		tc->val2[0]=y+3;
		tc->val2[1]=y+2;
		tc->val2[2]=y+1;
		tc->val2[3]=y;
		tc->val2[4]=y-1;
	} else if(dir==7){
		tc->val1[0]=x-3;
		tc->val1[1]=x-2;
		tc->val1[2]=x-1;
		tc->val1[3]=x;
		tc->val1[4]=x+1;
		tc->val2[1]=y;
		tc->val2[0]=y+1;
		tc->val2[2]=y-1;
		tc->val2[3]=y-2;
		tc->val2[4]=y-3;
	}

}

void skill_brandishspear_dir (struct square* tc, uint8 dir, int are) {
	int c;
	nullpo_retv(tc);

	for( c = 0; c < 5; c++ ) {
		switch( dir ) {
			case 0:                   tc->val2[c]+=are; break;
			case 1: tc->val1[c]-=are; tc->val2[c]+=are; break;
			case 2: tc->val1[c]-=are;                   break;
			case 3: tc->val1[c]-=are; tc->val2[c]-=are; break;
			case 4:                   tc->val2[c]-=are; break;
			case 5: tc->val1[c]+=are; tc->val2[c]-=are; break;
			case 6: tc->val1[c]+=are;                   break;
			case 7: tc->val1[c]+=are; tc->val2[c]+=are; break;
		}
	}
}

void skill_brandishspear(struct block_list* src, struct block_list* bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag) {
	int c,n=4;
	uint8 dir = map->calc_dir(src,bl->x,bl->y);
	struct square tc;
	int x=bl->x,y=bl->y;
	skill->brandishspear_first(&tc,dir,x,y);
	skill->brandishspear_dir(&tc,dir,4);
	skill->area_temp[1] = bl->id;

	if(skill_lv > 9){
		for(c=1;c<4;c++){
			map->foreachincell(skill->area_sub,
			                   bl->m,tc.val1[c],tc.val2[c],BL_CHAR,
			                   src,skill_id,skill_lv,tick, flag|BCT_ENEMY|n,
			                   skill->castend_damage_id);
		}
	}
	if(skill_lv > 6){
		skill->brandishspear_dir(&tc,dir,-1);
		n--;
	} else {
		skill->brandishspear_dir(&tc,dir,-2);
		n-=2;
	}

	if(skill_lv > 3){
		for(c=0;c<5;c++){
			map->foreachincell(skill->area_sub,
			                   bl->m,tc.val1[c],tc.val2[c],BL_CHAR,
			                   src,skill_id,skill_lv,tick, flag|BCT_ENEMY|n,
			                   skill->castend_damage_id);
			if(skill_lv > 6 && n==3 && c==4) {
				skill->brandishspear_dir(&tc,dir,-1);
				n--;c=-1;
			}
		}
	}
	for(c=0;c<10;c++){
		if(c==0||c==5) skill->brandishspear_dir(&tc,dir,-1);
		map->foreachincell(skill->area_sub,
			bl->m,tc.val1[c%5],tc.val2[c%5],BL_CHAR,
			src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
			skill->castend_damage_id);
	}
}

/*==========================================
 * Weapon Repair [Celest/DracoRPG]
 *------------------------------------------*/
void skill_repairweapon (struct map_session_data *sd, int idx) {
	int material;
	int materials[4] = {
		ITEMID_IRON_ORE,
		ITEMID_IRON,
		ITEMID_STEEL,
		ITEMID_ORIDECON_STONE,
	};
	struct item *item;
	struct map_session_data *target_sd;

	nullpo_retv(sd);

	if ( !( target_sd = map->id2sd(sd->menuskill_val) ) ) //Failed....
		return;

	if( idx == 0xFFFF ) // No item selected ('Cancel' clicked)
		return;
	if( idx < 0 || idx >= MAX_INVENTORY )
		return; //Invalid index??

	item = &target_sd->status.inventory[idx];
	if( item->nameid <= 0 || item->attribute == 0 )
		return; //Again invalid item....

	if( sd != target_sd && !battle->check_range(&sd->bl,&target_sd->bl, skill->get_range2(&sd->bl, sd->menuskill_id,sd->menuskill_val2) ) ){
		clif->item_repaireffect(sd,idx,1);
		return;
	}

	if ( target_sd->inventory_data[idx]->type == IT_WEAPON )
		material = materials[ target_sd->inventory_data[idx]->wlv - 1 ]; // Lv1/2/3/4 weapons consume 1 Iron Ore/Iron/Steel/Rough Oridecon
	else
		material = materials[2]; // Armors consume 1 Steel
	if (pc->search_inventory(sd,material) == INDEX_NOT_FOUND) {
		clif->skill_fail(sd,sd->menuskill_id,USESKILL_FAIL_LEVEL,0);
		return;
	}

	clif->skill_nodamage(&sd->bl,&target_sd->bl,sd->menuskill_id,1,1);

	item->attribute = 0;/* clear broken state */

	clif->equiplist(target_sd);

	pc->delitem(sd, pc->search_inventory(sd, material), 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME); // FIXME: is this the correct reason flag?

	clif->item_repaireffect(sd,idx,0);

	if( sd != target_sd )
		clif->item_repaireffect(target_sd,idx,0);
}

/*==========================================
 * Item Appraisal
 *------------------------------------------*/
void skill_identify (struct map_session_data *sd, int idx)
{
	int flag=1;

	nullpo_retv(sd);
	sd->state.workinprogress = 0;
	if(idx >= 0 && idx < MAX_INVENTORY) {
		if(sd->status.inventory[idx].nameid > 0 && sd->status.inventory[idx].identify == 0 ){
			flag=0;
			sd->status.inventory[idx].identify=1;
		}
	}
	clif->item_identified(sd,idx,flag);
}

/*==========================================
 * Weapon Refine [Celest]
 *------------------------------------------*/
void skill_weaponrefine (struct map_session_data *sd, int idx)
{
	nullpo_retv(sd);

	if (idx >= 0 && idx < MAX_INVENTORY) {
		struct item *item;
		struct item_data *ditem = sd->inventory_data[idx];
		item = &sd->status.inventory[idx];

		if (item->nameid > 0 && ditem->type == IT_WEAPON) {
			int material[5] = {
				0,
				ITEMID_PHRACON,
				ITEMID_EMVERETARCON,
				ITEMID_ORIDECON,
				ITEMID_ORIDECON,
			};
			int i = 0, per;
			if( ditem->flag.no_refine ) {
				// if the item isn't refinable
				clif->skill_fail(sd,sd->menuskill_id,USESKILL_FAIL_LEVEL,0);
				return;
			}
			if( item->refine >= sd->menuskill_val || item->refine >= 10 ){
				clif->upgrademessage(sd->fd, 2, item->nameid);
				return;
			}
			if ((i = pc->search_inventory(sd, material[ditem->wlv])) == INDEX_NOT_FOUND) {
				clif->upgrademessage(sd->fd, 3, material[ditem->wlv]);
				return;
			}

			per = status->get_refine_chance(ditem->wlv, (int)item->refine) * 10;

			// Aegis leaked formula. [malufett]
			if( sd->status.class_ == JOB_MECHANIC_T )
				per += 100;
			else
				per += 5 * ((signed int)sd->status.job_level - 50);

			pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_REFINE); // FIXME: is this the correct reason flag?
			if (per > rnd() % 1000) {
				int ep = 0;
				logs->pick_pc(sd, LOG_TYPE_REFINE, -1, item, ditem);
				item->refine++;
				logs->pick_pc(sd, LOG_TYPE_REFINE,  1, item, ditem);
				if(item->equip) {
					ep = item->equip;
					pc->unequipitem(sd, idx, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
				}
				clif->delitem(sd, idx, 1, DELITEM_NORMAL);
				clif->upgrademessage(sd->fd, 0,item->nameid);
				clif->inventorylist(sd);
				clif->refine(sd->fd,0,idx,item->refine);
				if (ep)
					pc->equipitem(sd,idx,ep);
				clif->misceffect(&sd->bl,3);
				if(item->refine == 10 &&
					item->card[0] == CARD0_FORGE &&
					(int)MakeDWord(item->card[2],item->card[3]) == sd->status.char_id)
				{ // Fame point system [DracoRPG]
					switch(ditem->wlv){
						case 1:
							pc->addfame(sd,1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
							break;
						case 2:
							pc->addfame(sd,25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
							break;
						case 3:
							pc->addfame(sd,1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
							break;
					}
				}
			} else {
				item->refine = 0;
				if(item->equip)
					pc->unequipitem(sd, idx, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
				clif->refine(sd->fd,1,idx,item->refine);
				pc->delitem(sd, idx, 1, 0, DELITEM_NORMAL, LOG_TYPE_REFINE);
				clif->misceffect(&sd->bl,2);
				clif->emotion(&sd->bl, E_OMG);
			}
		}
	}
}

/*==========================================
 *
 *------------------------------------------*/
int skill_autospell (struct map_session_data *sd, uint16 skill_id)
{
	uint16 skill_lv;
	int maxlv=1,lv;

	nullpo_ret(sd);

	skill_lv = sd->menuskill_val;
	lv=pc->checkskill(sd,skill_id);

	if(!skill_lv || !lv) return 0; // Player must learn the skill before doing auto-spell [Lance]

	if(skill_id==MG_NAPALMBEAT) maxlv=3;
	else if(skill_id==MG_COLDBOLT || skill_id==MG_FIREBOLT || skill_id==MG_LIGHTNINGBOLT){
		if (sd->sc.data[SC_SOULLINK] && sd->sc.data[SC_SOULLINK]->val2 == SL_SAGE)
			maxlv =10; //Soul Linker bonus. [Skotlex]
		else if(skill_lv==2) maxlv=1;
		else if(skill_lv==3) maxlv=2;
		else if(skill_lv>=4) maxlv=3;
	}
	else if(skill_id==MG_SOULSTRIKE){
		if(skill_lv==5) maxlv=1;
		else if(skill_lv==6) maxlv=2;
		else if(skill_lv>=7) maxlv=3;
	}
	else if(skill_id==MG_FIREBALL){
		if(skill_lv==8) maxlv=1;
		else if(skill_lv>=9) maxlv=2;
	}
	else if(skill_id==MG_FROSTDIVER) maxlv=1;
	else return 0;

	if(maxlv > lv)
		maxlv = lv;

	sc_start4(&sd->bl,&sd->bl,SC_AUTOSPELL,100,skill_lv,skill_id,maxlv,0,
		skill->get_time(SA_AUTOSPELL,skill_lv));
	return 0;
}

/*==========================================
 * Sitting skills functions.
 *------------------------------------------*/
int skill_sit_count(struct block_list *bl, va_list ap)
{
	int type = va_arg(ap, int);
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if(!pc_issit(sd))
		return 0;

	if(type&1 && pc->checkskill(sd,RG_GANGSTER) > 0)
		return 1;

	if(type&2 && (pc->checkskill(sd,TK_HPTIME) > 0 || pc->checkskill(sd,TK_SPTIME) > 0))
		return 1;

	return 0;
}

int skill_sit_in(struct block_list *bl, va_list ap)
{
	int type = va_arg(ap, int);
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if(!pc_issit(sd))
		return 0;

	if(type&1 && pc->checkskill(sd,RG_GANGSTER) > 0)
		sd->state.gangsterparadise=1;

	if(type&2 && (pc->checkskill(sd,TK_HPTIME) > 0 || pc->checkskill(sd,TK_SPTIME) > 0 )) {
		sd->state.rest=1;
		status->calc_regen(bl, &sd->battle_status, &sd->regen);
		status->calc_regen_rate(bl, &sd->regen, &sd->sc);
	}

	return 0;
}

int skill_sit_out(struct block_list *bl, va_list ap)
{
	int type = va_arg(ap, int);
	struct map_session_data *sd = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_PC);
	sd = BL_UCAST(BL_PC, bl);

	if(sd->state.gangsterparadise && type&1)
		sd->state.gangsterparadise=0;
	if(sd->state.rest && type&2) {
		sd->state.rest=0;
		status->calc_regen(bl, &sd->battle_status, &sd->regen);
		status->calc_regen_rate(bl, &sd->regen, &sd->sc);
	}
	return 0;
}

int skill_sit (struct map_session_data *sd, int type)
{
	int flag = 0;
	int range = 0, lv;
	nullpo_ret(sd);

	if((lv = pc->checkskill(sd,RG_GANGSTER)) > 0) {
		flag|=1;
		range = skill->get_splash(RG_GANGSTER, lv);
	}
	if((lv = pc->checkskill(sd,TK_HPTIME)) > 0) {
		flag|=2;
		range = skill->get_splash(TK_HPTIME, lv);
	}
	else if ((lv = pc->checkskill(sd,TK_SPTIME)) > 0) {
		flag|=2;
		range = skill->get_splash(TK_SPTIME, lv);
	}

	if( type ) {
		clif->sc_load(&sd->bl,sd->bl.id,SELF,SI_SIT,0,0,0);
	} else {
		clif->sc_end(&sd->bl,sd->bl.id,SELF,SI_SIT);
	}

	if (!flag) return 0;

	if(type) {
		if (map->foreachinrange(skill->sit_count,&sd->bl, range, BL_PC, flag) > 1)
			map->foreachinrange(skill->sit_in,&sd->bl, range, BL_PC, flag);
	} else {
		if (map->foreachinrange(skill->sit_count,&sd->bl, range, BL_PC, flag) < 2)
			map->foreachinrange(skill->sit_out,&sd->bl, range, BL_PC, flag);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_frostjoke_scream(struct block_list *bl, va_list ap)
{
	struct block_list *src;
	uint16 skill_id,skill_lv;
	int64 tick;

	nullpo_ret(bl);
	nullpo_ret(src=va_arg(ap,struct block_list*));

	skill_id=va_arg(ap,int);
	skill_lv=va_arg(ap,int);
	if(!skill_lv) return 0;
	tick=va_arg(ap,int64);

	if (src == bl || status->isdead(bl))
		return 0;
	if (bl->type == BL_PC) {
		const struct map_session_data *sd = BL_UCCAST(BL_PC, bl);
		if (pc_isinvisible(sd) || pc_ismadogear(sd))
			return 0; //Frost Joke / Scream cannot target invisible or MADO Gear characters [Ind]
	}
	//It has been reported that Scream/Joke works the same regardless of woe-setting. [Skotlex]
	if(battle->check_target(src,bl,BCT_ENEMY) > 0)
		skill->additional_effect(src,bl,skill_id,skill_lv,BF_MISC,ATK_DEF,tick);
	else if(battle->check_target(src,bl,BCT_PARTY) > 0 && rnd()%100 < 10)
		skill->additional_effect(src,bl,skill_id,skill_lv,BF_MISC,ATK_DEF,tick);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
void skill_unitsetmapcell (struct skill_unit *src, uint16 skill_id, uint16 skill_lv, cell_t cell, bool flag) {
	int range = skill->get_unit_range(skill_id,skill_lv);
	int x,y;

	for( y = src->bl.y - range; y <= src->bl.y + range; ++y )
		for( x = src->bl.x - range; x <= src->bl.x + range; ++x )
			map->list[src->bl.m].setcell(src->bl.m, x, y, cell, flag);
}

/*==========================================
 *
 *------------------------------------------*/
int skill_attack_area(struct block_list *bl, va_list ap) {
	struct block_list *src,*dsrc;
	int atk_type,skill_id,skill_lv,flag,type;
	int64 tick;

	if(status->isdead(bl))
		return 0;

	atk_type = va_arg(ap,int);
	src = va_arg(ap,struct block_list*);
	dsrc = va_arg(ap,struct block_list*);
	skill_id = va_arg(ap,int);
	skill_lv = va_arg(ap,int);
	tick = va_arg(ap,int64);
	flag = va_arg(ap,int);
	type = va_arg(ap,int);

	if (skill->area_temp[1] == bl->id) //This is the target of the skill, do a full attack and skip target checks.
		return skill->attack(atk_type,src,dsrc,bl,skill_id,skill_lv,tick,flag);

	if( battle->check_target(dsrc,bl,type) <= 0
	 || !status->check_skilluse(NULL, bl, skill_id, 2))
		return 0;

	switch (skill_id) {
		case WZ_FROSTNOVA: //Skills that don't require the animation to be removed
		case NPC_ACIDBREATH:
		case NPC_DARKNESSBREATH:
		case NPC_FIREBREATH:
		case NPC_ICEBREATH:
		case NPC_THUNDERBREATH:
			return skill->attack(atk_type,src,dsrc,bl,skill_id,skill_lv,tick,flag);
		default:
			//Area-splash, disable skill animation.
			return skill->attack(atk_type,src,dsrc,bl,skill_id,skill_lv,tick,flag|SD_ANIMATION);
	}
}
/*==========================================
 *
 *------------------------------------------*/
int skill_clear_group (struct block_list *bl, int flag)
{
	struct unit_data *ud = unit->bl2ud(bl);
	struct skill_unit_group *group[MAX_SKILLUNITGROUP];
	int i, count=0;

	nullpo_ret(bl);
	if (!ud) return 0;

	//All groups to be deleted are first stored on an array since the array elements shift around when you delete them. [Skotlex]
	for (i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++) {
		switch (ud->skillunit[i]->skill_id) {
			case SA_DELUGE:
			case SA_VOLCANO:
			case SA_VIOLENTGALE:
			case SA_LANDPROTECTOR:
			case NJ_SUITON:
			case NJ_KAENSIN:
				if (flag&1)
					group[count++]= ud->skillunit[i];
				break;
			case SO_CLOUD_KILL:
				if( flag&4 )
					group[count++]= ud->skillunit[i];
				break;
			case SO_WARMER:
				if( flag&8 )
					group[count++]= ud->skillunit[i];
				break;
			default:
				if (flag&2 && skill->get_inf2(ud->skillunit[i]->skill_id)&INF2_TRAP)
					group[count++]= ud->skillunit[i];
				break;
		}

	}
	for (i=0;i<count;i++)
		skill->del_unitgroup(group[i],ALC_MARK);
	return count;
}

/*==========================================
 * Returns the first element field found [Skotlex]
 *------------------------------------------*/
struct skill_unit_group *skill_locate_element_field(struct block_list *bl) {
	struct unit_data *ud = unit->bl2ud(bl);
	int i;
	nullpo_ret(bl);
	if (!ud) return NULL;

	for (i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++) {
		switch (ud->skillunit[i]->skill_id) {
			case SA_DELUGE:
			case SA_VOLCANO:
			case SA_VIOLENTGALE:
			case SA_LANDPROTECTOR:
			case NJ_SUITON:
			case SO_CLOUD_KILL:
			case SO_WARMER:
				return ud->skillunit[i];
		}
	}
	return NULL;
}

// for graffiti cleaner [Valaris]
int skill_graffitiremover(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;

	nullpo_ret(bl);

	if (bl->type != BL_SKILL)
		return 0;
	su = BL_UCAST(BL_SKILL, bl);

	if((su->group) && (su->group->unit_id == UNT_GRAFFITI))
		skill->delunit(su);

	return 0;
}

int skill_greed(struct block_list *bl, va_list ap)
{
	struct block_list *src = va_arg(ap, struct block_list *);

	nullpo_ret(bl);
	nullpo_ret(src);

	if (src->type == BL_PC && bl->type == BL_ITEM) {
		struct map_session_data *sd = BL_UCAST(BL_PC, src);
		struct flooritem_data *fitem = BL_UCAST(BL_ITEM, bl);
		pc->takeitem(sd, fitem);
	}

	return 0;
}

//For Ranger's Detonator [Jobbie/3CeAM]
int skill_detonator(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;
	struct block_list *src = va_arg(ap,struct block_list *);
	int unit_id;

	nullpo_ret(bl);
	nullpo_ret(src);

	if (bl->type != BL_SKILL)
		return 0;
	su = BL_UCAST(BL_SKILL, bl);

	if( !su->group || su->group->src_id != src->id )
		return 0;

	unit_id = su->group->unit_id;
	switch( unit_id ) {
		//List of Hunter and Ranger Traps that can be detonate.
		case UNT_BLASTMINE:
		case UNT_SANDMAN:
		case UNT_CLAYMORETRAP:
		case UNT_TALKIEBOX:
		case UNT_CLUSTERBOMB:
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
			switch(unit_id) {
				case UNT_TALKIEBOX:
					clif->talkiebox(bl,su->group->valstr);
					su->group->val2 = -1;
					break;
				case UNT_CLAYMORETRAP:
				case UNT_FIRINGTRAP:
				case UNT_ICEBOUNDTRAP:
					map->foreachinrange(skill->trap_splash,bl,skill->get_splash(su->group->skill_id,su->group->skill_lv),su->group->bl_flag|BL_SKILL|~BCT_SELF,bl,su->group->tick);
					break;
				default:
					map->foreachinrange(skill->trap_splash,bl,skill->get_splash(su->group->skill_id,su->group->skill_lv),su->group->bl_flag,bl,su->group->tick);
			}
			clif->changetraplook(bl, UNT_USED_TRAPS);
			su->group->limit = DIFF_TICK32(timer->gettick(),su->group->tick) +
				(unit_id == UNT_TALKIEBOX ? 5000 : (unit_id == UNT_CLUSTERBOMB || unit_id == UNT_ICEBOUNDTRAP? 2500 : (unit_id == UNT_FIRINGTRAP ? 0 : 1500)) );
			su->group->unit_id = UNT_USED_TRAPS;
			break;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_cell_overlap(struct block_list *bl, va_list ap)
{
	uint16 skill_id;
	int *alive;
	struct skill_unit *su = NULL;

	skill_id = va_arg(ap,int);
	alive = va_arg(ap,int *);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_SKILL);
	nullpo_ret(alive);
	su = BL_UCAST(BL_SKILL, bl);

	if (bl->type != BL_SKILL || su->group == NULL || (*alive) == 0)
		return 0;

	if( su->group->state.guildaura ) /* guild auras are not canceled! */
		return 0;

	switch (skill_id) {
		case SA_LANDPROTECTOR:
			if( su->group->skill_id == SA_LANDPROTECTOR ) {//Check for offensive Land Protector to delete both. [Skotlex]
				(*alive) = 0;
				skill->delunit(su);
				return 1;
			}
			// SA_LANDPROTECTOR blocks everything except songs/dances/traps (and NOLP)
			// TODO: Do these skills ignore land protector when placed on top?
			if( !(skill->get_inf2(su->group->skill_id)&(INF2_SONG_DANCE|INF2_TRAP|INF2_NOLP)) || su->group->skill_id == WZ_FIREPILLAR || su->group->skill_id == GN_HELLS_PLANT) {
				skill->delunit(su);
				return 1;
			}
			break;
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
		case GN_CRAZYWEED_ATK:
			if( !(su->group->state.song_dance&0x1) ) {// Don't touch song/dance.
				skill->delunit(su);
				return 1;
			}
			break;
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
// The official implementation makes them fail to appear when casted on top of ANYTHING
// but I wonder if they didn't actually meant to fail when casted on top of each other?
// hence, I leave the alternate implementation here, commented. [Skotlex]
			if (su->range <= 0) {
				(*alive) = 0;
				return 1;
			}
/*
			switch (su->group->skill_id) {
				//These cannot override each other.
				case SA_VOLCANO:
				case SA_DELUGE:
				case SA_VIOLENTGALE:
					(*alive) = 0;
					return 1;
			}
*/
			break;
		case PF_FOGWALL:
			switch(su->group->skill_id) {
				case SA_VOLCANO: //Can't be placed on top of these
				case SA_VIOLENTGALE:
					(*alive) = 0;
					return 1;
				case SA_DELUGE:
				case NJ_SUITON:
				//Cheap 'hack' to notify the calling function that duration should be doubled [Skotlex]
					(*alive) = 2;
					break;
			}
			break;
		case WZ_ICEWALL:
		case HP_BASILICA:
			if (su->group->skill_id == skill_id) {
				//These can't be placed on top of themselves (duration can't be refreshed)
				(*alive) = 0;
				return 1;
			}
			break;
	}

	if (su->group->skill_id == SA_LANDPROTECTOR && !(skill->get_inf2(skill_id)&(INF2_SONG_DANCE|INF2_TRAP|INF2_NOLP))) {
		//SA_LANDPROTECTOR blocks everything except songs/dances/traps (and NOLP)
		(*alive) = 0;
		return 1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_chastle_mob_changetarget(struct block_list *bl, va_list ap)
{
	struct unit_data *ud = unit->bl2ud(bl);
	struct block_list *from_bl = va_arg(ap, struct block_list *);
	struct block_list *to_bl = va_arg(ap, struct block_list *);

	nullpo_ret(bl);
	nullpo_ret(from_bl);
	nullpo_ret(to_bl);

	if (ud != NULL && ud->target == from_bl->id)
		ud->target = to_bl->id;

	if (bl->type == BL_MOB) {
		struct mob_data *md = BL_UCAST(BL_MOB, bl);
		if (md->target_id == from_bl->id)
			md->target_id = to_bl->id;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_trap_splash(struct block_list *bl, va_list ap)
{
	struct block_list *src = va_arg(ap, struct block_list *);
	int64 tick = va_arg(ap, int64);
	struct skill_unit *src_su = NULL;
	struct skill_unit_group *sg;
	struct block_list *ss;

	nullpo_ret(bl);
	nullpo_ret(src);
	Assert_ret(src->type == BL_SKILL);
	src_su = BL_UCAST(BL_SKILL, src);

	if (!src_su->alive || bl->prev == NULL)
		return 0;

	nullpo_ret(sg = src_su->group);
	nullpo_ret(ss = map->id2bl(sg->src_id));

	if(battle->check_target(src,bl,sg->target_flag) <= 0)
		return 0;

	switch(sg->unit_id){
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
			skill->additional_effect(ss,bl,sg->skill_id,sg->skill_lv,BF_MISC,ATK_DEF,tick);
			break;
		case UNT_GROUNDDRIFT_WIND:
			if(skill->attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(src,bl,SC_STUN,5,sg->skill_lv,skill->get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_DARK:
			if(skill->attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(src,bl,SC_BLIND,5,sg->skill_lv,skill->get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_POISON:
			if(skill->attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(src,bl,SC_POISON,5,sg->skill_lv,skill->get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_WATER:
			if(skill->attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(src,bl,SC_FREEZE,5,sg->skill_lv,skill->get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_FIRE:
			if(skill->attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				skill->blown(src,bl,skill->get_blewcount(sg->skill_id,sg->skill_lv),-1,0);
			break;
		case UNT_ELECTRICSHOCKER:
			clif->skill_damage(src,bl,tick,0,0,-30000,1,sg->skill_id,sg->skill_lv,BDT_SPLASH);
			break;
		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
			if( bl->type != BL_PC && !is_boss(bl) )
				sc_start2(ss,bl,SC_ARMOR_PROPERTY,100,sg->skill_lv,skill->get_ele(sg->skill_id,sg->skill_lv),skill->get_time2(sg->skill_id,sg->skill_lv));
			break;
		case UNT_REVERBERATION:
			if( battle->check_target(src,bl,BCT_ENEMY) > 0 ) {
				skill->attack(BF_WEAPON,ss,src,bl,WM_REVERBERATION_MELEE,sg->skill_lv,tick,0);
				skill->addtimerskill(ss,tick+200,bl->id,0,0,WM_REVERBERATION_MAGIC,sg->skill_lv,BF_MAGIC,SD_LEVEL);
			}
			break;
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
			if (src->id == bl->id)
				break;
			if (bl->type == BL_SKILL) {
				struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
				if (su->group->unit_id == UNT_USED_TRAPS)
					break;
			}
			/* Fall through */
		case UNT_CLUSTERBOMB:
			if( ss != bl )
				skill->attack(BF_MISC,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1|SD_LEVEL);
			break;
		case UNT_CLAYMORETRAP:
			if (src->id == bl->id)
				break;
			if (bl->type == BL_SKILL) {
				struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
				switch (su->group->unit_id) {
					case UNT_CLAYMORETRAP:
					case UNT_LANDMINE:
					case UNT_BLASTMINE:
					case UNT_SHOCKWAVE:
					case UNT_SANDMAN:
					case UNT_FLASHER:
					case UNT_FREEZINGTRAP:
					case UNT_FIRINGTRAP:
					case UNT_ICEBOUNDTRAP:
						clif->changetraplook(bl, UNT_USED_TRAPS);
						su->group->limit = DIFF_TICK32(timer->gettick(),su->group->tick) + 1500;
						su->group->unit_id = UNT_USED_TRAPS;
				}
				break;
			}
			/* Fall through */
		default:
			skill->attack(skill->get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;
	}
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_enchant_elemental_end (struct block_list *bl, int type) {
	struct status_change *sc;
	const enum sc_type scs[] = { SC_ENCHANTPOISON, SC_ASPERSIO, SC_PROPERTYFIRE, SC_PROPERTYWATER, SC_PROPERTYWIND, SC_PROPERTYGROUND, SC_PROPERTYDARK, SC_PROPERTYTELEKINESIS, SC_ENCHANTARMS };
	int i;
	nullpo_ret(bl);
	nullpo_ret(sc = status->get_sc(bl));

	if (!sc->count) return 0;

	for (i = 0; i < ARRAYLENGTH(scs); i++)
		if (type != scs[i] && sc->data[scs[i]])
			status_change_end(bl, scs[i], INVALID_TIMER);

	return 0;
}

bool skill_check_cloaking(struct block_list *bl, struct status_change_entry *sce)
{
	static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
	static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
	bool wall = true;

	if( (bl->type == BL_PC && battle_config.pc_cloak_check_type&1)
	 || (bl->type != BL_PC && battle_config.monster_cloak_check_type&1)
	) {
		//Check for walls.
		int i;
		ARR_FIND( 0, 8, i, map->getcell(bl->m, bl, bl->x+dx[i], bl->y+dy[i], CELL_CHKNOPASS) != 0 );
		if( i == 8 )
			wall = false;
	}

	if( sce ) {
		if( !wall ) {
			if( sce->val1 < 3 ) //End cloaking.
				status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			else if( sce->val4&1 ) { //Remove wall bonus
				sce->val4&=~1;
				status_calc_bl(bl,SCB_SPEED);
			}
		} else {
			if( !(sce->val4&1) ) { //Add wall speed bonus
				sce->val4|=1;
				status_calc_bl(bl,SCB_SPEED);
			}
		}
	}

	return wall;
}

/**
 * Verifies if an user can use SC_CLOAKING
 **/
bool skill_can_cloak(struct map_session_data *sd) {
	nullpo_retr(false, sd);

	//Avoid cloaking with no wall and low skill level. [Skotlex]
	//Due to the cloaking card, we have to check the wall versus to known
	//skill level rather than the used one. [Skotlex]
	//if (sd && val1 < 3 && skill->check_cloaking(bl,NULL))
	if (pc->checkskill(sd, AS_CLOAKING) < 3 && !skill->check_cloaking(&sd->bl,NULL))
		return false;

	return true;
}

/**
 * Verifies if an user can still be cloaked (AS_CLOAKING)
 * Is called via map->foreachinrange when any kind of wall disapears
 **/
int skill_check_cloaking_end(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = BL_CAST(BL_PC, bl);

	if (sd && sd->sc.data[SC_CLOAKING] && !skill->can_cloak(sd))
		status_change_end(bl, SC_CLOAKING, INVALID_TIMER);

	return 0;
}

bool skill_check_camouflage(struct block_list *bl, struct status_change_entry *sce)
{
	static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
	static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
	bool wall = true;

	if( bl->type == BL_PC ) { //Check for walls.
		int i;
		ARR_FIND( 0, 8, i, map->getcell(bl->m, bl, bl->x+dx[i], bl->y+dy[i], CELL_CHKNOPASS) != 0 );
		if( i == 8 )
			wall = false;
	}

	if( sce ) {
		if( !wall ) {
			if( sce->val1 < 3 ) //End camouflage.
				status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
			else if( sce->val3&1 ) { //Remove wall bonus
				sce->val3&=~1;
				status_calc_bl(bl,SCB_SPEED);
			}
		}
	}

	return wall;
}

bool skill_check_shadowform(struct block_list *bl, int64 damage, int hit)
{
	struct status_change *sc;

	nullpo_retr(false, bl);

	sc = status->get_sc(bl);

	if (sc && sc->data[SC__SHADOWFORM] && damage) {
		struct block_list *src = map->id2bl(sc->data[SC__SHADOWFORM]->val2);
		struct map_session_data *sd = BL_CAST(BL_PC, src);

		if( !src || src->m != bl->m ) {
			status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
			return false;
		}

		if( src && (status->isdead(src) || !battle->check_target(bl,src,BCT_ENEMY)) ){
			if (sd != NULL)
				sd->shadowform_id = 0;
			status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
			return false;
		}

		status->damage(bl, src, damage, 0, clif->damage(src, src, 500, 500, damage, hit, (hit > 1 ? BDT_MULTIHIT : BDT_NORMAL), 0), 0);

		/* because damage can cancel it */
		if( sc->data[SC__SHADOWFORM] && (--sc->data[SC__SHADOWFORM]->val3) <= 0 ) {
			status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
			if (sd != NULL)
				sd->shadowform_id = 0;
		}
		return true;
	}
	return false;
}
/*==========================================
 *
 *------------------------------------------*/
struct skill_unit *skill_initunit (struct skill_unit_group *group, int idx, int x, int y, int val1, int val2) {
	struct skill_unit *su;

	nullpo_retr(NULL, group);
	nullpo_retr(NULL, group->unit.data); // crash-protection against poor coding
	nullpo_retr(NULL, su=&group->unit.data[idx]);

	if(!su->alive)
		group->alive_count++;

	su->bl.id=map->get_new_object_id();
	su->bl.type=BL_SKILL;
	su->bl.m=group->map;
	su->bl.x=x;
	su->bl.y=y;
	su->group=group;
	su->alive=1;
	su->val1=val1;
	su->val2 = val2;
	su->prev = 0;

	idb_put(skill->unit_db, su->bl.id, su);
	map->addiddb(&su->bl);
	map->addblock(&su->bl);

	// perform oninit actions
	switch (group->skill_id) {
		case WZ_ICEWALL:
			map->setgatcell(su->bl.m,su->bl.x,su->bl.y,5);
			clif->changemapcell(0,su->bl.m,su->bl.x,su->bl.y,5,AREA);
			skill->unitsetmapcell(su,WZ_ICEWALL,group->skill_lv,CELL_ICEWALL,true);
			break;
		case SA_LANDPROTECTOR:
			skill->unitsetmapcell(su,SA_LANDPROTECTOR,group->skill_lv,CELL_LANDPROTECTOR,true);
			break;
		case HP_BASILICA:
			skill->unitsetmapcell(su,HP_BASILICA,group->skill_lv,CELL_BASILICA,true);
			break;
		default:
			if (group->state.song_dance&0x1) //Check for dissonance.
				skill->dance_overlap(su, 1);
			break;
	}

	clif->getareachar_skillunit(&su->bl,su,AREA);

	return su;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_delunit (struct skill_unit* su) {
	struct skill_unit_group *group;

	nullpo_ret(su);
	if( !su->alive )
		return 0;
	su->alive=0;

	nullpo_ret(group=su->group);

	if( group->state.song_dance&0x1 ) //Cancel dissonance effect.
		skill->dance_overlap(su, 0);

	// invoke onout event
	if( !su->range )
		map->foreachincell(skill->unit_effect,su->bl.m,su->bl.x,su->bl.y,group->bl_flag,&su->bl,timer->gettick(),4);

	// perform ondelete actions
	switch (group->skill_id) {
		case HT_ANKLESNARE:
		{
			struct block_list* target = map->id2bl(group->val2);
			if( target )
				status_change_end(target, SC_ANKLESNARE, INVALID_TIMER);
		}
			break;
		case WZ_ICEWALL:
			map->list[su->bl.m].setcell(su->bl.m, su->bl.x, su->bl.y, CELL_NOICEWALL, false);
			map->setgatcell(su->bl.m,su->bl.x,su->bl.y,su->val2);
			clif->changemapcell(0,su->bl.m,su->bl.x,su->bl.y,su->val2,ALL_SAMEMAP); // hack to avoid clientside cell bug
			skill->unitsetmapcell(su,WZ_ICEWALL,group->skill_lv,CELL_ICEWALL,false);
			// AS_CLOAKING in low levels requires a wall to be cast, thus it needs to be
			// checked again when a wall disapears! issue:8182 [Panikon]
			map->foreachinarea(skill->check_cloaking_end, su->bl.m,
								// Use 3x3 area to check for users near cell
								su->bl.x - 1, su->bl.y - 1,
								su->bl.x + 1, su->bl.x + 1,
								BL_PC);
			break;
		case SA_LANDPROTECTOR:
			skill->unitsetmapcell(su,SA_LANDPROTECTOR,group->skill_lv,CELL_LANDPROTECTOR,false);
			break;
		case HP_BASILICA:
			skill->unitsetmapcell(su,HP_BASILICA,group->skill_lv,CELL_BASILICA,false);
			break;
		case RA_ELECTRICSHOCKER: {
				struct block_list* target = map->id2bl(group->val2);
				if( target )
					status_change_end(target, SC_ELECTRICSHOCKER, INVALID_TIMER);
			}
			break;
		case SC_MANHOLE: // Note : Removing the unit don't remove the status (official info)
			if( group->val2 ) { // Someone Trapped
				struct status_change *tsc = status->get_sc(map->id2bl(group->val2));
				if( tsc && tsc->data[SC__MANHOLE] )
					tsc->data[SC__MANHOLE]->val4 = 0; // Remove the Unit ID
			}
			break;
	}

	clif->skill_delunit(su);

	su->group=NULL;
	map->delblock(&su->bl); // don't free yet
	map->deliddb(&su->bl);
	idb_remove(skill->unit_db, su->bl.id);
	if(--group->alive_count==0)
		skill->del_unitgroup(group,ALC_MARK);

	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
/// Returns the target skill_unit_group or NULL if not found.
struct skill_unit_group* skill_id2group(int group_id)
{
	return (struct skill_unit_group*)idb_get(skill->group_db, group_id);
}

/// Returns a new group_id that isn't being used in skill->group_db.
/// Fatal error if nothing is available.
int skill_get_new_group_id(void)
{
	if( skill->unit_group_newid >= MAX_SKILL_DB && skill->id2group(skill->unit_group_newid) == NULL )
		return skill->unit_group_newid++;// available
	{// find next id
		int base_id = skill->unit_group_newid;
		while( base_id != ++skill->unit_group_newid )
		{
			if( skill->unit_group_newid < MAX_SKILL_DB )
				skill->unit_group_newid = MAX_SKILL_DB;
			if( skill->id2group(skill->unit_group_newid) == NULL )
				return skill->unit_group_newid++;// available
		}
		// full loop, nothing available
		ShowFatalError("skill_get_new_group_id: All ids are taken. Exiting...");
		exit(1);
	}
}

struct skill_unit_group* skill_initunitgroup (struct block_list* src, int count, uint16 skill_id, uint16 skill_lv, int unit_id, int limit, int interval)
{
	struct unit_data* ud = unit->bl2ud( src );
	struct skill_unit_group* group;
	int i;

	if(!(skill_id && skill_lv)) return 0;

	nullpo_retr(NULL, src);
	nullpo_retr(NULL, ud);

	// find a free spot to store the new unit group
	ARR_FIND( 0, MAX_SKILLUNITGROUP, i, ud->skillunit[i] == NULL );
	if(i == MAX_SKILLUNITGROUP) {
		// array is full, make room by discarding oldest group
		int j=0;
		int64 maxdiff = 0, tick = timer->gettick();
		for(i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++) {
			int64 diff = DIFF_TICK(tick,ud->skillunit[i]->tick);
			if (diff > maxdiff) {
				maxdiff = diff;
				j = i;
			}
		}
		skill->del_unitgroup(ud->skillunit[j],ALC_MARK);
		//Since elements must have shifted, we use the last slot.
		i = MAX_SKILLUNITGROUP-1;
	}

	group              = ers_alloc(skill->unit_ers, struct skill_unit_group);
	group->src_id      = src->id;
	group->party_id    = status->get_party_id(src);
	group->guild_id    = status->get_guild_id(src);
	group->bg_id       = bg->team_get_id(src);
	group->group_id    = skill->get_new_group_id();
	CREATE(group->unit.data, struct skill_unit, count);
	group->unit.count  = count;
	group->alive_count = 0;
	group->val1        = 0;
	group->val2        = 0;
	group->val3        = 0;
	group->skill_id    = skill_id;
	group->skill_lv    = skill_lv;
	group->unit_id     = unit_id;
	group->map         = src->m;
	group->limit       = limit;
	group->interval    = interval;
	group->tick        = timer->gettick();
	group->valstr      = NULL;

	ud->skillunit[i] = group;

	if (skill_id == PR_SANCTUARY) //Sanctuary starts healing +1500ms after casted. [Skotlex]
		group->tick += 1500;

	idb_put(skill->group_db, group->group_id, group);
	return group;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_delunitgroup(struct skill_unit_group *group, const char *file, int line, const char *func)
{
	struct block_list* src;
	struct unit_data *ud;
	int i,j;
	struct map_session_data *sd = NULL;

	if( group == NULL ) {
		ShowDebug("skill_delunitgroup: group is NULL (source=%s:%d, %s)! Please report this! (#3504)\n", file, line, func);
		return 0;
	}

	src = map->id2bl(group->src_id);
	ud = unit->bl2ud(src);
	sd = BL_CAST(BL_PC, src);
	if (src == NULL || ud == NULL) {
		ShowError("skill_delunitgroup: Group's source not found! (src_id: %d skill_id: %d)\n", group->src_id, group->skill_id);
		return 0;
	}

	if (sd != NULL && !status->isdead(src) && sd->state.warping && !sd->state.changemap) {
		switch( group->skill_id ) {
			case BA_DISSONANCE:
			case BA_POEMBRAGI:
			case BA_WHISTLE:
			case BA_ASSASSINCROSS:
			case BA_APPLEIDUN:
			case DC_UGLYDANCE:
			case DC_HUMMING:
			case DC_DONTFORGETME:
			case DC_FORTUNEKISS:
			case DC_SERVICEFORYOU:
				skill->usave_add(sd, group->skill_id, group->skill_lv);
				break;
		}
	}

	if (skill->get_unit_flag(group->skill_id)&(UF_DANCE|UF_SONG|UF_ENSEMBLE)) {
		struct status_change* sc = status->get_sc(src);
		if (sc && sc->data[SC_DANCING])
		{
			sc->data[SC_DANCING]->val2 = 0 ; //This prevents status_change_end attempting to re-delete the group. [Skotlex]
			status_change_end(src, SC_DANCING, INVALID_TIMER);
		}
	}

	// end Gospel's status change on 'src'
	// (needs to be done when the group is deleted by other means than skill deactivation)
	if (group->unit_id == UNT_GOSPEL) {
		struct status_change *sc = status->get_sc(src);
		if(sc && sc->data[SC_GOSPEL]) {
			sc->data[SC_GOSPEL]->val3 = 0; //Remove reference to this group. [Skotlex]
			status_change_end(src, SC_GOSPEL, INVALID_TIMER);
		}
	}

	switch( group->skill_id ) {
		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM:
		{
			struct status_change *sc = NULL;
			if( (sc = status->get_sc(src)) != NULL  && sc->data[SC_WARM] ) {
				sc->data[SC_WARM]->val4 = 0;
				status_change_end(src, SC_WARM, INVALID_TIMER);
			}
		}
			break;
		case NC_NEUTRALBARRIER:
		{
			struct status_change *sc = NULL;
			if( (sc = status->get_sc(src)) != NULL && sc->data[SC_NEUTRALBARRIER_MASTER] ) {
				sc->data[SC_NEUTRALBARRIER_MASTER]->val2 = 0;
				status_change_end(src,SC_NEUTRALBARRIER_MASTER,INVALID_TIMER);
			}
		}
			break;
		case NC_STEALTHFIELD:
		{
			struct status_change *sc = NULL;
			if( (sc = status->get_sc(src)) != NULL && sc->data[SC_STEALTHFIELD_MASTER] ) {
				sc->data[SC_STEALTHFIELD_MASTER]->val2 = 0;
				status_change_end(src,SC_STEALTHFIELD_MASTER,INVALID_TIMER);
			}
		}
			break;
		case LG_BANDING:
		{
			struct status_change *sc = NULL;
			if( (sc = status->get_sc(src)) && sc->data[SC_BANDING] ) {
				sc->data[SC_BANDING]->val4 = 0;
				status_change_end(src,SC_BANDING,INVALID_TIMER);
			}
		}
			break;
	}

	if (sd != NULL && group->state.ammo_consume)
		battle->consume_ammo(sd, group->skill_id, group->skill_lv);

	group->alive_count=0;

	// remove all unit cells
	if (group->unit.data != NULL) {
		for (i = 0; i < group->unit.count; i++)
			skill->delunit(&group->unit.data[i]);
	}

	// clear Talkie-box string
	if( group->valstr != NULL ) {
		aFree(group->valstr);
		group->valstr = NULL;
	}

	idb_remove(skill->group_db, group->group_id);
	map->freeblock(&group->unit.data[0].bl); // schedules deallocation of whole array (HACK)
	group->unit.data=NULL;
	group->group_id=0;
	group->unit.count=0;

	// locate this group, swap with the last entry and delete it
	ARR_FIND( 0, MAX_SKILLUNITGROUP, i, ud->skillunit[i] == group );
	ARR_FIND( i, MAX_SKILLUNITGROUP, j, ud->skillunit[j] == NULL ); j--;
	if( i < MAX_SKILLUNITGROUP ) {
		ud->skillunit[i] = ud->skillunit[j];
		ud->skillunit[j] = NULL;
		ers_free(skill->unit_ers, group);
	} else
		ShowError("skill_delunitgroup: Group not found! (src_id: %d skill_id: %d)\n", group->src_id, group->skill_id);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_clear_unitgroup (struct block_list *src)
{
	struct unit_data *ud = unit->bl2ud(src);

	nullpo_ret(ud);

	while (ud->skillunit[0])
		skill->del_unitgroup(ud->skillunit[0],ALC_MARK);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl, struct skill_unit_group *group, int64 tick)
{
	int i,j=-1,s,id;
	struct unit_data *ud;
	struct skill_unit_group_tickset *set;

	nullpo_ret(bl);
	if (group->interval==-1)
		return NULL;

	ud = unit->bl2ud(bl);
	if (!ud) return NULL;

	set = ud->skillunittick;

	if (skill->get_unit_flag(group->skill_id)&UF_NOOVERLAP)
		id = s = group->skill_id;
	else
		id = s = group->group_id;

	for (i=0; i<MAX_SKILLUNITGROUPTICKSET; i++) {
		int k = (i+s) % MAX_SKILLUNITGROUPTICKSET;
		if (set[k].id == id)
			return &set[k];
		else if (j==-1 && (DIFF_TICK(tick,set[k].tick)>0 || set[k].id==0))
			j=k;
	}

	if (j == -1) {
		ShowWarning ("skill_unitgrouptickset_search: tickset is full. ( failed for skill '%s' on unit %u )\n", skill->get_name(group->skill_id), bl->type);
		j = id % MAX_SKILLUNITGROUPTICKSET;
	}

	set[j].id = id;
	set[j].tick = tick;
	return &set[j];
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_timer_sub_onplace(struct block_list* bl, va_list ap) {
	struct skill_unit* su = va_arg(ap,struct skill_unit *);
	struct skill_unit_group* group = su->group;
	int64 tick = va_arg(ap,int64);

	if( !su->alive || bl->prev == NULL )
		return 0;

	nullpo_ret(group);

	if (!(skill->get_inf2(group->skill_id)&(INF2_SONG_DANCE|INF2_TRAP|INF2_NOLP)) && map->getcell(su->bl.m, &su->bl, su->bl.x, su->bl.y, CELL_CHKLANDPROTECTOR))
		return 0; //AoE skills are ineffective. [Skotlex]

	if( battle->check_target(&su->bl,bl,group->target_flag) <= 0 )
		return 0;

	skill->unit_onplace_timer(su,bl,tick);

	return 1;
}

/**
 * @see DBApply
 */
int skill_unit_timer_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct skill_unit* su = DB->data2ptr(data);
	struct skill_unit_group* group = su->group;
	int64 tick = va_arg(ap,int64);
	bool dissonance;
	struct block_list* bl = &su->bl;

	if( !su->alive )
		return 0;

	nullpo_ret(group);

	// check for expiration
	if( !group->state.guildaura && (DIFF_TICK(tick,group->tick) >= group->limit || DIFF_TICK(tick,group->tick) >= su->limit) ) {
		// skill unit expired (inlined from skill_unit_onlimit())
		switch( group->unit_id ) {
			case UNT_BLASTMINE:
#ifdef RENEWAL
			case UNT_CLAYMORETRAP:
#endif
			case UNT_GROUNDDRIFT_WIND:
			case UNT_GROUNDDRIFT_DARK:
			case UNT_GROUNDDRIFT_POISON:
			case UNT_GROUNDDRIFT_WATER:
			case UNT_GROUNDDRIFT_FIRE:
				group->unit_id = UNT_USED_TRAPS;
				//clif->changetraplook(bl, UNT_FIREPILLAR_ACTIVE);
				group->limit=DIFF_TICK32(tick+1500,group->tick);
				su->limit=DIFF_TICK32(tick+1500,group->tick);
			break;

			case UNT_ANKLESNARE:
			case UNT_ELECTRICSHOCKER:
				if( group->val2 > 0 || group->val3 == SC_ESCAPE ) {
					// Used Trap don't returns back to item
					skill->delunit(su);
					break;
				}
			case UNT_SKIDTRAP:
			case UNT_LANDMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
#ifndef RENEWAL
			case UNT_CLAYMORETRAP:
#endif
			case UNT_TALKIEBOX:
			case UNT_CLUSTERBOMB:
			case UNT_MAGENTATRAP:
			case UNT_COBALTTRAP:
			case UNT_MAIZETRAP:
			case UNT_VERDURETRAP:
			case UNT_FIRINGTRAP:
			case UNT_ICEBOUNDTRAP:

			{
				struct block_list* src;
				if( su->val1 > 0 && (src = map->id2bl(group->src_id)) != NULL && src->type == BL_PC ) {
					// revert unit back into a trap
					struct item item_tmp;
					memset(&item_tmp,0,sizeof(item_tmp));
					item_tmp.nameid = group->item_id?group->item_id:ITEMID_TRAP;
					item_tmp.identify = 1;
					map->addflooritem(bl, &item_tmp, 1, bl->m, bl->x, bl->y, 0, 0, 0, 0);
				}
				skill->delunit(su);
			}
			break;

			case UNT_WARP_ACTIVE:
				// warp portal opens (morph to a UNT_WARP_WAITING cell)
				group->unit_id = skill->get_unit_id(group->skill_id, 1); // UNT_WARP_WAITING
				clif->changelook(&su->bl, LOOK_BASE, group->unit_id);
				// restart timers
				group->limit = skill->get_time(group->skill_id,group->skill_lv);
				su->limit = skill->get_time(group->skill_id,group->skill_lv);
				// apply effect to all units standing on it
				map->foreachincell(skill->unit_effect,su->bl.m,su->bl.x,su->bl.y,group->bl_flag,&su->bl,timer->gettick(),1);
			break;

			case UNT_CALLFAMILY:
			{
				struct map_session_data *sd = NULL;
				if(group->val1) {
					sd = map->charid2sd(group->val1);
					group->val1 = 0;
					if (sd && !map->list[sd->bl.m].flag.nowarp)
						pc->setpos(sd,map_id2index(su->bl.m),su->bl.x,su->bl.y,CLR_TELEPORT);
				}
				if(group->val2) {
					sd = map->charid2sd(group->val2);
					group->val2 = 0;
					if (sd && !map->list[sd->bl.m].flag.nowarp)
						pc->setpos(sd,map_id2index(su->bl.m),su->bl.x,su->bl.y,CLR_TELEPORT);
				}
				skill->delunit(su);
			}
			break;

			case UNT_REVERBERATION:
				if( su->val1 <= 0 ) { // If it was deactivated.
					skill->delunit(su);
					break;
				}
				clif->changetraplook(bl,UNT_USED_TRAPS);
				map->foreachinrange(skill->trap_splash, bl, skill->get_splash(group->skill_id, group->skill_lv), group->bl_flag, bl, tick);
				group->limit = DIFF_TICK32(tick,group->tick)+1500;
				su->limit = DIFF_TICK32(tick,group->tick)+1500;
				group->unit_id = UNT_USED_TRAPS;
			break;

			case UNT_FEINTBOMB: {
				struct block_list *src = map->id2bl(group->src_id);
				if( src ) {
					map->foreachinrange(skill->area_sub, &su->bl, su->range, splash_target(src), src, SC_FEINTBOMB, group->skill_lv, tick, BCT_ENEMY|SD_ANIMATION|1, skill->castend_damage_id);
					status_change_end(src, SC__FEINTBOMB_MASTER, INVALID_TIMER);
				}
				skill->delunit(su);
				break;
			}

			case UNT_BANDING:
			{
				struct block_list *src = map->id2bl(group->src_id);
				struct status_change *sc;
				if( !src || (sc = status->get_sc(src)) == NULL || !sc->data[SC_BANDING] ) {
					skill->delunit(su);
					break;
				}
				// This unit isn't removed while SC_BANDING is active.
				group->limit = DIFF_TICK32(tick+group->interval,group->tick);
				su->limit = DIFF_TICK32(tick+group->interval,group->tick);
			}
			break;

			default:
				skill->delunit(su);
		}
	} else {// skill unit is still active
		switch( group->unit_id ) {
			case UNT_ICEWALL:
				// icewall loses 50 hp every second
				su->val1 -= SKILLUNITTIMER_INTERVAL/20; // trap's hp
				if( su->val1 <= 0 && su->limit + group->tick > tick + 700 )
					su->limit = DIFF_TICK32(tick+700,group->tick);
				break;
			case UNT_BLASTMINE:
			case UNT_SKIDTRAP:
			case UNT_LANDMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_CLAYMORETRAP:
			case UNT_FREEZINGTRAP:
			case UNT_TALKIEBOX:
			case UNT_ANKLESNARE:
				if( su->val1 <= 0 ) {
					if( group->unit_id == UNT_ANKLESNARE && group->val2 > 0 )
						skill->delunit(su);
					else {
						clif->changetraplook(bl, group->unit_id==UNT_LANDMINE?UNT_FIREPILLAR_ACTIVE:UNT_USED_TRAPS);
						group->limit = DIFF_TICK32(tick, group->tick) + 1500;
						group->unit_id = UNT_USED_TRAPS;
					}
				}
				break;
			case UNT_REVERBERATION:
				if( su->val1 <= 0 )
					su->limit = DIFF_TICK32(tick + 700,group->tick);
				break;
			case UNT_WALLOFTHORN:
				if( su->val1 <= 0 ) {
					group->unit_id = UNT_USED_TRAPS;
					group->limit = DIFF_TICK32(tick, group->tick) + 1500;
				}
				break;
		}
	}

	//Don't continue if unit or even group is expired and has been deleted.
	if( !su->alive )
		return 0;

	dissonance = skill->dance_switch(su, 0);

	if( su->range >= 0 && group->interval != -1 && su->bl.id != su->prev) {
		if( battle_config.skill_wall_check )
			map->foreachinshootrange(skill->unit_timer_sub_onplace, bl, su->range, group->bl_flag, bl,tick);
		else
			map->foreachinrange(skill->unit_timer_sub_onplace, bl, su->range, group->bl_flag, bl,tick);

		if(su->range == -1) //Unit disabled, but it should not be deleted yet.
			group->unit_id = UNT_USED_TRAPS;
		else if( group->unit_id == UNT_TATAMIGAESHI ) {
			su->range = -1; //Disable processed cell.
			if (--group->val1 <= 0) { // number of live cells
				//All tiles were processed, disable skill.
				group->target_flag=BCT_NOONE;
				group->bl_flag= BL_NUL;
			}
		}
		if ( group->limit == group->interval )
			su->prev = su->bl.id;
	}

	if( dissonance ) skill->dance_switch(su, 1);

	return 0;
}
/*==========================================
 * Executes on all skill units every SKILLUNITTIMER_INTERVAL milliseconds.
 *------------------------------------------*/
int skill_unit_timer(int tid, int64 tick, int id, intptr_t data) {
	map->freeblock_lock();

	skill->unit_db->foreach(skill->unit_db, skill->unit_timer_sub, tick);

	map->freeblock_unlock();

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_move_sub(struct block_list* bl, va_list ap)
{
	struct skill_unit *su = NULL;
	struct skill_unit_group *group = NULL;

	struct block_list* target = va_arg(ap,struct block_list*);
	int64 tick = va_arg(ap,int64);
	int flag = va_arg(ap,int);

	bool dissonance;
	uint16 skill_id;
	int i;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_SKILL);
	su = BL_UCAST(BL_SKILL, bl);
	group = su->group;
	nullpo_ret(group);

	if( !su->alive || target->prev == NULL )
		return 0;

	if( flag&1 && ( su->group->skill_id == PF_SPIDERWEB || su->group->skill_id == GN_THORNS_TRAP ) )
		return 0; // Fiberlock is never supposed to trigger on skill->unit_move. [Inkfish]

	dissonance = skill->dance_switch(su, 0);

	//Necessary in case the group is deleted after calling on_place/on_out [Skotlex]
	skill_id = su->group->skill_id;

	if( su->group->interval != -1 && !(skill->get_unit_flag(skill_id)&UF_DUALMODE) && skill_id != BD_LULLABY ) { //Lullaby is the exception, bugreport:411
		//Non-dualmode unit skills with a timer don't trigger when walking, so just return
		if( dissonance ) skill->dance_switch(su, 1);
		return 0;
	}

	//Target-type check.
	if( !(group->bl_flag&target->type && battle->check_target(&su->bl,target,group->target_flag) > 0) ) {
		if( group->src_id == target->id && group->state.song_dance&0x2 ) { //Ensemble check to see if they went out/in of the area [Skotlex]
			if( flag&1 ) {
				if( flag&2 ) { //Clear this skill id.
					ARR_FIND( 0, ARRAYLENGTH(skill->unit_temp), i, skill->unit_temp[i] == skill_id );
					if( i < ARRAYLENGTH(skill->unit_temp) )
						skill->unit_temp[i] = 0;
				}
			} else {
				if( flag&2 ) { //Store this skill id.
					ARR_FIND( 0, ARRAYLENGTH(skill->unit_temp), i, skill->unit_temp[i] == 0 );
					if( i < ARRAYLENGTH(skill->unit_temp) )
						skill->unit_temp[i] = skill_id;
					else
						ShowError("skill_unit_move_sub: Reached limit of unit objects per cell!\n");
				}

			}

			if( flag&4 )
				skill->unit_onleft(skill_id,target,tick);
		}

		if( dissonance ) skill->dance_switch(su, 1);

		return 0;
	} else {
		if( flag&1 ) {
			int result = skill->unit_onplace(su,target,tick);
			if( flag&2 && result ) { //Clear skill ids we have stored in onout.
				ARR_FIND( 0, ARRAYLENGTH(skill->unit_temp), i, skill->unit_temp[i] == result );
				if( i < ARRAYLENGTH(skill->unit_temp) )
					skill->unit_temp[i] = 0;
			}
		} else {
			int result = skill->unit_onout(su,target,tick);
			if( flag&2 && result ) { //Store this unit id.
				ARR_FIND( 0, ARRAYLENGTH(skill->unit_temp), i, skill->unit_temp[i] == 0 );
				if( i < ARRAYLENGTH(skill->unit_temp) )
					skill->unit_temp[i] = skill_id;
				else
					ShowError("skill_unit_move_sub: Reached limit of unit objects per cell!\n");
			}
		}

		if( dissonance ) skill->dance_switch(su, 1);

		if( flag&4 )
			skill->unit_onleft(skill_id,target,tick);

		return 1;
	}
}

/*==========================================
 * Invoked when a char has moved and unit cells must be invoked (onplace, onout, onleft)
 * Flag values:
 * flag&1: invoke skill_unit_onplace (otherwise invoke skill_unit_onout)
 * flag&2: this function is being invoked twice as a bl moves, store in memory the affected
 * units to figure out when they have left a group.
 * flag&4: Force a onleft event (triggered when the bl is killed, for example)
 *------------------------------------------*/
int skill_unit_move(struct block_list *bl, int64 tick, int flag) {
	nullpo_ret(bl);

	if( bl->prev == NULL )
		return 0;

	if( flag&2 && !(flag&1) ) { //Onout, clear data
		memset(skill->unit_temp, 0, sizeof(skill->unit_temp));
	}

	map->foreachincell(skill->unit_move_sub,bl->m,bl->x,bl->y,BL_SKILL,bl,tick,flag);

	if( flag&2 && flag&1 ) { //Onplace, check any skill units you have left.
		int i;
		for( i = 0; i < ARRAYLENGTH(skill->unit_temp); i++ )
			if( skill->unit_temp[i] )
				skill->unit_onleft(skill->unit_temp[i], bl, tick);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_move_unit_group(struct skill_unit_group *group, int16 m, int16 dx, int16 dy) {
	int i,j;
	int64 tick = timer->gettick();
	int *m_flag;
	struct skill_unit *su1;
	struct skill_unit *su2;

	if (group == NULL)
		return 0;
	if (group->unit.count<=0)
		return 0;
	if (group->unit.data==NULL)
		return 0;

	if (skill->get_unit_flag(group->skill_id)&UF_ENSEMBLE)
		return 0; //Ensembles may not be moved around.

	if( group->unit_id == UNT_ICEWALL || group->unit_id == UNT_WALLOFTHORN )
		return 0; //Icewalls and Wall of Thorns don't get knocked back

	m_flag = (int *) aCalloc(group->unit.count, sizeof(int));
	// m_flag:
	//  0: Neither of the following (skill_unit_onplace & skill_unit_onout are needed)
	//  1: Unit will move to a slot that had another unit of the same group (skill_unit_onplace not needed)
	//  2: Another unit from same group will end up positioned on this unit (skill_unit_onout not needed)
	//  3: Both 1+2.
	for (i = 0; i < group->unit.count; i++) {
		su1=&group->unit.data[i];
		if (!su1->alive || su1->bl.m!=m)
			continue;
		for (j = 0; j < group->unit.count; j++) {
			su2=&group->unit.data[j];
			if (!su2->alive)
				continue;
			if (su1->bl.x+dx==su2->bl.x && su1->bl.y+dy==su2->bl.y) {
				m_flag[i] |= 0x1;
			}
			if (su1->bl.x-dx==su2->bl.x && su1->bl.y-dy==su2->bl.y) {
				m_flag[i] |= 0x2;
			}
		}
	}
	j = 0;
	for (i = 0; i < group->unit.count; i++) {
		su1=&group->unit.data[i];
		if (!su1->alive)
			continue;
		if (!(m_flag[i]&0x2)) {
			if (group->state.song_dance&0x1) //Cancel dissonance effect.
				skill->dance_overlap(su1, 0);
			map->foreachincell(skill->unit_effect,su1->bl.m,su1->bl.x,su1->bl.y,group->bl_flag,&su1->bl,tick,4);
		}
		//Move Cell using "smart" criteria (avoid useless moving around)
		switch(m_flag[i]) {
			case 0:
			//Cell moves independently, safely move it.
				map->moveblock(&su1->bl, su1->bl.x+dx, su1->bl.y+dy, tick);
				break;
			case 1:
			//Cell moves unto another cell, look for a replacement cell that won't collide
			//and has no cell moving into it (flag == 2)
				for (; j < group->unit.count; j++) {
					if (m_flag[j]!=2 || !group->unit.data[j].alive)
						continue;
					//Move to where this cell would had moved.
					su2 = &group->unit.data[j];
					map->moveblock(&su1->bl, su2->bl.x+dx, su2->bl.y+dy, tick);
					j++; //Skip this cell as we have used it.
					break;
				}
				break;
			case 2:
			case 3:
				break; //Don't move the cell as a cell will end on this tile anyway.
		}
		if (!(m_flag[i]&0x2)) { //We only moved the cell in 0-1
			if (group->state.song_dance&0x1) //Check for dissonance effect.
				skill->dance_overlap(su1, 1);
			clif->getareachar_skillunit(&su1->bl,su1,AREA);
			map->foreachincell(skill->unit_effect,su1->bl.m,su1->bl.x,su1->bl.y,group->bl_flag,&su1->bl,tick,1);
		}
	}
	aFree(m_flag);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_can_produce_mix (struct map_session_data *sd, int nameid, int trigger, int qty)
{
	int i,j;

	nullpo_ret(sd);

	if(nameid<=0)
		return 0;

	for(i=0;i<MAX_SKILL_PRODUCE_DB;i++){
		if(skill->dbs->produce_db[i].nameid == nameid ){
			if((j=skill->dbs->produce_db[i].req_skill)>0 &&
				pc->checkskill(sd,j) < skill->dbs->produce_db[i].req_skill_lv)
					continue; // must iterate again to check other skills that produce it. [malufett]
			if( j > 0 && sd->menuskill_id > 0 && sd->menuskill_id != j )
				continue; // special case
			break;
		}
	}

	if( i >= MAX_SKILL_PRODUCE_DB )
		return 0;

	if( pc->checkadditem(sd, nameid, qty) == ADDITEM_OVERAMOUNT )
	{// cannot carry the produced stuff
		return 0;
	}

	if(trigger>=0){
		if(trigger>20) { // Non-weapon, non-food item (itemlv must match)
			if(skill->dbs->produce_db[i].itemlv!=trigger)
				return 0;
		} else if(trigger>10) { // Food (any item level between 10 and 20 will do)
			if(skill->dbs->produce_db[i].itemlv<=10 || skill->dbs->produce_db[i].itemlv>20)
				return 0;
		} else { // Weapon (itemlv must be higher or equal)
			if(skill->dbs->produce_db[i].itemlv>trigger)
				return 0;
		}
	}

	for (j = 0; j < MAX_PRODUCE_RESOURCE; j++) {
		int id = skill->dbs->produce_db[i].mat_id[j];
		if (id <= 0)
			continue;
		if (skill->dbs->produce_db[i].mat_amount[j] <= 0) {
			if (pc->search_inventory(sd,id) == INDEX_NOT_FOUND)
				return 0;
		} else {
			int x, y;
			for(y=0,x=0;y<MAX_INVENTORY;y++)
				if( sd->status.inventory[y].nameid == id )
					x+=sd->status.inventory[y].amount;
			if(x<qty*skill->dbs->produce_db[i].mat_amount[j])
				return 0;
		}
	}
	return i+1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_produce_mix(struct map_session_data *sd, uint16 skill_id, int nameid, int slot1, int slot2, int slot3, int qty) {
	int slot[3];
	int i,sc,ele,idx,equip,wlv,make_per = 0,flag = 0,skill_lv = 0;
	int num = -1; // exclude the recipe
	struct status_data *st;
	struct item_data* data;

	nullpo_ret(sd);
	st = status->get_status_data(&sd->bl);

	if( sd->skill_id_old == skill_id )
		skill_lv = sd->skill_lv_old;

	if( !(idx=skill->can_produce_mix(sd,nameid,-1, qty)) )
		return 0;
	idx--;

	if (qty < 1)
		qty = 1;

	if (!skill_id) //A skill can be specified for some override cases.
		skill_id = skill->dbs->produce_db[idx].req_skill;

	if( skill_id == GC_RESEARCHNEWPOISON )
		skill_id = GC_CREATENEWPOISON;

	slot[0]=slot1;
	slot[1]=slot2;
	slot[2]=slot3;

	for(i=0,sc=0,ele=0;i<3;i++){ //Note that qty should always be one if you are using these!
		int j;
		if( slot[i]<=0 )
			continue;
		j = pc->search_inventory(sd,slot[i]);
		if (j == INDEX_NOT_FOUND)
			continue;
		if( slot[i]==ITEMID_STAR_CRUMB ) {
			pc->delitem(sd, j, 1, 1, DELITEM_NORMAL, LOG_TYPE_PRODUCE); // FIXME: is this the correct reason flag?
			sc++;
		}
		if( slot[i] >= ITEMID_FLAME_HEART && slot[i] <= ITEMID_GREAT_NATURE && ele == 0 ) {
			static const int ele_table[4]={3,1,4,2};
			pc->delitem(sd, j, 1, 1, DELITEM_NORMAL, LOG_TYPE_PRODUCE); // FIXME: is this the correct reason flag?
			ele=ele_table[slot[i]-994];
		}
	}

	if( skill_id == RK_RUNEMASTERY ) {
		int temp_qty, rune_skill_lv = pc->checkskill(sd,skill_id);
		data = itemdb->search(nameid);

		if( rune_skill_lv == 10 ) temp_qty = 1 + rnd()%3;
		else if( rune_skill_lv > 5 ) temp_qty = 1 + rnd()%2;
		else temp_qty = 1;

		if (data->stack.inventory) {
			for( i = 0; i < MAX_INVENTORY; i++ ) {
				if( sd->status.inventory[i].nameid == nameid ) {
					if( sd->status.inventory[i].amount >= data->stack.amount ) {
						clif->msgtable(sd, MSG_RUNE_STONE_MAX_AMOUNT);
						return 0;
					} else {
						/**
						 * the amount fits, say we got temp_qty 4 and 19 runes, we trim temp_qty to 1.
						 **/
						if( temp_qty + sd->status.inventory[i].amount >= data->stack.amount )
							temp_qty = data->stack.amount - sd->status.inventory[i].amount;
					}
					break;
				}
			}
		}
		qty = temp_qty;
	}

	for(i=0;i<MAX_PRODUCE_RESOURCE;i++){
		int j,id,x;
		if( (id=skill->dbs->produce_db[idx].mat_id[i]) <= 0 )
			continue;
		num++;
		x=( skill_id == RK_RUNEMASTERY ? 1 : qty)*skill->dbs->produce_db[idx].mat_amount[i];
		do{
			int y=0;
			j = pc->search_inventory(sd,id);

			if (j != INDEX_NOT_FOUND) {
				y = sd->status.inventory[j].amount;
				if(y>x)y=x;
				pc->delitem(sd, j, y, 0, DELITEM_NORMAL, LOG_TYPE_PRODUCE); // FIXME: is this the correct reason flag?
			} else
				ShowError("skill_produce_mix: material item error\n");

			x-=y;
		}while( j>=0 && x>0 );
	}

	if( (equip = (itemdb->isequip(nameid) && skill_id != GN_CHANGEMATERIAL && skill_id != GN_MAKEBOMB )) )
		wlv = itemdb_wlv(nameid);
	if(!equip) {
		switch(skill_id){
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				// Ores & Metals Refining - skill bonuses are straight from kRO website [DracoRPG]
				i = pc->checkskill(sd,skill_id);
				make_per = sd->status.job_level*20 + st->dex*10 + st->luk*10; //Base chance
				switch(nameid){
					case ITEMID_IRON:
						make_per += 4000+i*500; // Temper Iron bonus: +26/+32/+38/+44/+50
						break;
					case ITEMID_STEEL:
						make_per += 3000+i*500; // Temper Steel bonus: +35/+40/+45/+50/+55
						break;
					case ITEMID_STAR_CRUMB:
						make_per = 100000; // Star Crumbs are 100% success crafting rate? (made 1000% so it succeeds even after penalties) [Skotlex]
						break;
					default: // Enchanted Stones
						make_per += 1000+i*500; // Enchanted stone Craft bonus: +15/+20/+25/+30/+35
					break;
				}
				break;
			case ASC_CDP:
				make_per = (2000 + 40*st->dex + 20*st->luk);
				break;
			case AL_HOLYWATER:
			/**
			 * Arch Bishop
			 **/
			case AB_ANCILLA:
				make_per = 100000; //100% success
				break;
			case AM_PHARMACY: // Potion Preparation - reviewed with the help of various Ragnainfo sources [DracoRPG]
			case AM_TWILIGHT1:
			case AM_TWILIGHT2:
			case AM_TWILIGHT3:
				make_per = pc->checkskill(sd,AM_LEARNINGPOTION)*50
					+ pc->checkskill(sd,AM_PHARMACY)*300 + sd->status.job_level*20
					+ (st->int_/2)*10 + st->dex*10+st->luk*10;
				if(homun_alive(sd->hd)) {//Player got a homun
					int skill2_lv;
					if((skill2_lv=homun->checkskill(sd->hd,HVAN_INSTRUCT)) > 0) //His homun is a vanil with instruction change
						make_per += skill2_lv*100; //+1% bonus per level
				}
				switch(nameid){
					case ITEMID_RED_POTION:
					case ITEMID_YELLOW_POTION:
					case ITEMID_WHITE_POTION:
						make_per += (1+rnd()%100)*10 + 2000;
						break;
					case ITEMID_ALCHOL:
						make_per += (1+rnd()%100)*10 + 1000;
						break;
					case ITEMID_FIRE_BOTTLE:
					case ITEMID_ACID_BOTTLE:
					case ITEMID_MENEATER_PLANT_BOTTLE:
					case ITEMID_MINI_BOTTLE:
						make_per += (1+rnd()%100)*10;
						break;
					case ITEMID_YELLOW_SLIM_POTION:
						make_per -= (1+rnd()%50)*10;
						break;
					case ITEMID_WHITE_SLIM_POTION:
					case ITEMID_COATING_BOTTLE:
						make_per -= (1+rnd()%100)*10;
					    break;
					//Common items, receive no bonus or penalty, listed just because they are commonly produced
					case ITEMID_BLUE_POTION:
					case ITEMID_RED_SLIM_POTION:
					case ITEMID_ANODYNE:
					case ITEMID_ALOEBERA:
					default:
						break;
				}
				if(battle_config.pp_rate != 100)
					make_per = make_per * battle_config.pp_rate / 100;
				break;
			case SA_CREATECON: // Elemental Converter Creation
				make_per = 100000; // should be 100% success rate
				break;
			/**
			 * Rune Knight
			 **/
			case RK_RUNEMASTERY:
			    {
				int A = 5100 + 200 * pc->checkskill(sd, skill_id);
				int B = 10 * st->dex / 3 + (st->luk + sd->status.job_level);
				int C = 100 * cap_value(sd->itemid,0,100); //itemid depend on makerune()
				int D = 2500;
				switch (nameid) { //rune rank it_diff 9 craftable rune
					case ITEMID_RAIDO:
					case ITEMID_THURISAZ:
					case ITEMID_HAGALAZ:
					case ITEMID_OTHILA:
						D -= 500; //Rank C
					case ITEMID_ISA:
					case ITEMID_WYRD:
						D -= 500; //Rank B
					case ITEMID_NAUTHIZ:
					case ITEMID_URUZ:
						D -= 500; //Rank A
					case ITEMID_BERKANA:
					case ITEMID_LUX_ANIMA:
						D -= 500; //Rank S
				}
				make_per = A + B + C - D;
				break;
			    }
			/**
			 * Guilotine Cross
			 **/
			case GC_CREATENEWPOISON:
				{
					const int min[10] = {2, 2, 3, 3, 4, 4, 5, 5, 6, 6};
					const int max[10] = {4, 5, 5, 6, 6, 7, 7, 8, 8, 9};
					int lv = max(0, pc->checkskill(sd,GC_RESEARCHNEWPOISON) - 1);
					qty = min[lv] + rnd()%(max[lv] - min[lv]);
					make_per = 3000 + 500 * lv + st->dex / 3 * 10 + st->luk * 10 + sd->status.job_level * 10;
				}
				break;
			case GN_CHANGEMATERIAL:
				for(i=0; i<MAX_SKILL_PRODUCE_DB; i++)
					if( skill->dbs->changematerial_db[i].itemid == nameid ){
						make_per = skill->dbs->changematerial_db[i].rate * 10;
						break;
					}
				break;
			case GN_S_PHARMACY:
				{
					int difficulty = 0;

					difficulty = (620 - 20 * skill_lv);// (620 - 20 * Skill Level)

					make_per = st->int_ + st->dex/2 + st->luk + sd->status.job_level + (30+rnd()%120) + // (Caster?s INT) + (Caster?s DEX / 2) + (Caster?s LUK) + (Caster?s Job Level) + Random number between (30 ~ 150) +
								(sd->status.base_level-100) + pc->checkskill(sd, AM_LEARNINGPOTION) + pc->checkskill(sd, CR_FULLPROTECTION)*(4+rnd()%6); // (Caster?s Base Level - 100) + (Potion Research x 5) + (Full Chemical Protection Skill Level) x (Random number between 4 ~ 10)

					switch(nameid){// difficulty factor
						case ITEMID_HP_INCREASE_POTIONS:
						case ITEMID_SP_INCREASE_POTIONS:
						case ITEMID_ENRICH_WHITE_POTIONZ:
							difficulty += 10;
							break;
						case ITEMID_BOMB_MUSHROOM_SPORE:
						case ITEMID_SP_INCREASE_POTIONM:
							difficulty += 15;
							break;
						case ITEMID_BANANA_BOMB:
						case ITEMID_HP_INCREASE_POTIONM:
						case ITEMID_SP_INCREASE_POTIONL:
						case ITEMID_VITATA500:
							difficulty += 20;
							break;
						case ITEMID_SEED_OF_HORNY_PLANT:
						case ITEMID_BLOODSUCK_PLANT_SEED:
						case ITEMID_ENRICH_CELERMINE_JUICE:
							difficulty += 30;
							break;
						case ITEMID_HP_INCREASE_POTIONL:
						case ITEMID_CURE_FREE:
							difficulty += 40;
							break;
					}

					if( make_per >= 400 && make_per > difficulty)
						qty = 10;
					else if( make_per >= 300 && make_per > difficulty)
						qty = 7;
					else if( make_per >= 100 && make_per > difficulty)
						qty = 6;
					else if( make_per >= 1 && make_per > difficulty)
						qty = 5;
					else
						qty = 4;
					make_per = 10000;
				}
				break;
			case GN_MAKEBOMB:
			case GN_MIX_COOKING:
				{
					int difficulty = 30 + rnd()%120; // Random number between (30 ~ 150)

					make_per = sd->status.job_level / 4 + st->luk / 2 + st->dex / 3; // (Caster?s Job Level / 4) + (Caster?s LUK / 2) + (Caster?s DEX / 3)
					qty = ~(5 + rnd()%5) + 1;

					switch(nameid){// difficulty factor
						case ITEMID_APPLE_BOMB:
							difficulty += 5;
							break;
						case ITEMID_COCONUT_BOMB:
						case ITEMID_MELON_BOMB:
							difficulty += 10;
							break;
						case ITEMID_SAVAGE_BBQ:
						case ITEMID_WUG_BLOOD_COCKTAIL:
						case ITEMID_MINOR_BRISKET:
						case ITEMID_SIROMA_ICETEA:
						case ITEMID_DROCERA_HERB_STEW:
						case ITEMID_PETTI_TAIL_NOODLE:
						case ITEMID_PINEAPPLE_BOMB:
							difficulty += 15;
							break;
						case ITEMID_BANANA_BOMB:
							difficulty += 20;
							break;
					}

					if( make_per >= 30 && make_per > difficulty)
						qty = 10 + rnd()%2;
					else if( make_per >= 10 && make_per > difficulty)
						qty = 10;
					else if( make_per == 10 && make_per > difficulty)
						qty = 8;
					else if( (make_per >= 50 || make_per < 30) && make_per < difficulty)
						;// Food/Bomb creation fails.
					else if( make_per >= 30 && make_per < difficulty)
						qty = 5;

					if( qty < 0 || (skill_lv == 1 && make_per < difficulty)){
						qty = ~qty + 1;
						make_per = 0;
					}else
						make_per = 10000;
					qty = (skill_lv > 1 ? qty : 1);
				}
				break;
			default:
				if (sd->menuskill_id == AM_PHARMACY && sd->menuskill_val > 10 && sd->menuskill_val <= 20) {
					//Assume Cooking Dish
					if (sd->menuskill_val >= 15) //Legendary Cooking Set.
						make_per = 10000; //100% Success
					else
						make_per = 1200 * (sd->menuskill_val - 10)
							+ 20  * (sd->status.base_level + 1)
							+ 20  * (st->dex + 1)
							+ 100 * (rnd()%(30+5*(sd->cook_mastery/400) - (6+sd->cook_mastery/80)) + (6+sd->cook_mastery/80))
							- 400 * (skill->dbs->produce_db[idx].itemlv - 11 + 1)
							- 10  * (100 - st->luk + 1)
							- 500 * (num - 1)
							- 100 * (rnd()%4 + 1);
					break;
				}
				make_per = 5000;
				break;
		}
	} else { // Weapon Forging - skill bonuses are straight from kRO website, other things from a jRO calculator [DracoRPG]
		make_per = 5000 + sd->status.job_level*20 + st->dex*10 + st->luk*10; // Base
		make_per += pc->checkskill(sd,skill_id)*500; // Smithing skills bonus: +5/+10/+15
		make_per += pc->checkskill(sd,BS_WEAPONRESEARCH)*100 +((wlv >= 3)? pc->checkskill(sd,BS_ORIDEOCON)*100:0); // Weaponry Research bonus: +1/+2/+3/+4/+5/+6/+7/+8/+9/+10, Oridecon Research bonus (custom): +1/+2/+3/+4/+5
		make_per -= (ele?2000:0) + sc*1500 + (wlv>1?wlv*1000:0); // Element Stone: -20%, Star Crumb: -15% each, Weapon level malus: -0/-20/-30
		if (pc->search_inventory(sd,ITEMID_EMPERIUM_ANVIL) != INDEX_NOT_FOUND)
			make_per+= 1000; // +10
		else if(pc->search_inventory(sd,ITEMID_GOLDEN_ANVIL) != INDEX_NOT_FOUND)
			make_per+= 500; // +5
		else if(pc->search_inventory(sd,ITEMID_ORIDECON_ANVIL) != INDEX_NOT_FOUND)
			make_per+= 300; // +3
		else if(pc->search_inventory(sd,ITEMID_ANVIL) != INDEX_NOT_FOUND)
			make_per+= 0; // +0?
		if(battle_config.wp_rate != 100)
			make_per = make_per * battle_config.wp_rate / 100;
	}

	if (sd->class_&JOBL_BABY) //if it's a Baby Class
		make_per = (make_per * 50) / 100; //Baby penalty is 50% (bugreport:4847)

	if(make_per < 1) make_per = 1;

	if(rnd()%10000 < make_per || qty > 1){ //Success, or crafting multiple items.
		struct item tmp_item;
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid=nameid;
		tmp_item.amount=1;
		tmp_item.identify=1;
		if(equip){
			tmp_item.card[0]=CARD0_FORGE;
			tmp_item.card[1]=((sc*5)<<8)+ele;
			tmp_item.card[2]=GetWord(sd->status.char_id,0); // CharId
			tmp_item.card[3]=GetWord(sd->status.char_id,1);
		} else {
			//Flag is only used on the end, so it can be used here. [Skotlex]
			switch (skill_id) {
				case BS_DAGGER:
				case BS_SWORD:
				case BS_TWOHANDSWORD:
				case BS_AXE:
				case BS_MACE:
				case BS_KNUCKLE:
				case BS_SPEAR:
					flag = battle_config.produce_item_name_input&0x1;
					break;
				case AM_PHARMACY:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					flag = battle_config.produce_item_name_input&0x2;
					break;
				case AL_HOLYWATER:
				/**
				 * Arch Bishop
				 **/
				case AB_ANCILLA:
					flag = battle_config.produce_item_name_input&0x8;
					break;
				case ASC_CDP:
					flag = battle_config.produce_item_name_input&0x10;
					break;
				default:
					flag = battle_config.produce_item_name_input&0x80;
					break;
			}
			if (flag) {
				tmp_item.card[0]=CARD0_CREATE;
				tmp_item.card[1]=0;
				tmp_item.card[2]=GetWord(sd->status.char_id,0); // CharId
				tmp_item.card[3]=GetWord(sd->status.char_id,1);
			}
		}

#if 0 // TODO: update PICKLOG
		if(log_config.produce > 0)
			log_produce(sd,nameid,slot1,slot2,slot3,1);
#endif // 0

		if(equip){
			clif->produce_effect(sd,0,nameid);
			clif->misceffect(&sd->bl,3);
			if(itemdb_wlv(nameid) >= 3 && ((ele? 1 : 0) + sc) >= 3) // Fame point system [DracoRPG]
				pc->addfame(sd,10); // Success to forge a lv3 weapon with 3 additional ingredients = +10 fame point
		} else {
			int fame = 0;
			tmp_item.amount = 0;

			for (i=0; i< qty; i++) {
				//Apply quantity modifiers.
				if( (skill_id == GN_MIX_COOKING || skill_id == GN_MAKEBOMB || skill_id == GN_S_PHARMACY) && make_per > 1){
					tmp_item.amount = qty;
					break;
				}
				if (rnd()%10000 < make_per || qty == 1) { //Success
					tmp_item.amount++;
					if(nameid < ITEMID_RED_SLIM_POTION || nameid > ITEMID_WHITE_SLIM_POTION)
						continue;
					if( skill_id != AM_PHARMACY &&
						skill_id != AM_TWILIGHT1 &&
						skill_id != AM_TWILIGHT2 &&
						skill_id != AM_TWILIGHT3 )
						continue;
					//Add fame as needed.
					switch(++sd->potion_success_counter) {
						case 3:
							fame+=1; // Success to prepare 3 Condensed Potions in a row
							break;
						case 5:
							fame+=3; // Success to prepare 5 Condensed Potions in a row
							break;
						case 7:
							fame+=10; // Success to prepare 7 Condensed Potions in a row
							break;
						case 10:
							fame+=50; // Success to prepare 10 Condensed Potions in a row
							sd->potion_success_counter = 0;
							break;
					}
				} else //Failure
					sd->potion_success_counter = 0;
			}

			if (fame)
				pc->addfame(sd,fame);
			//Visual effects and the like.
			switch (skill_id) {
				case AM_PHARMACY:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
				case ASC_CDP:
					clif->produce_effect(sd,2,nameid);
					clif->misceffect(&sd->bl,5);
					break;
				case BS_IRON:
				case BS_STEEL:
				case BS_ENCHANTEDSTONE:
					clif->produce_effect(sd,0,nameid);
					clif->misceffect(&sd->bl,3);
					break;
				case RK_RUNEMASTERY:
				case GC_CREATENEWPOISON:
					clif->produce_effect(sd,2,nameid);
					clif->misceffect(&sd->bl,5);
					break;
				default: //Those that don't require a skill?
					if( skill->dbs->produce_db[idx].itemlv > 10 && skill->dbs->produce_db[idx].itemlv <= 20)
					{ //Cooking items.
						clif->specialeffect(&sd->bl, 608, AREA);
						if( sd->cook_mastery < 1999 )
							pc_setglobalreg(sd, script->add_str("COOK_MASTERY"),sd->cook_mastery + ( 1 << ( (skill->dbs->produce_db[idx].itemlv - 11) / 2 ) ) * 5);
					}
					break;
			}
		}
		if ( skill_id == GN_CHANGEMATERIAL && tmp_item.amount) { //Success
			int j, k = 0;
			for(i=0; i<MAX_SKILL_PRODUCE_DB; i++)
				if( skill->dbs->changematerial_db[i].itemid == nameid ){
					for(j=0; j<5; j++){
						if( rnd()%1000 < skill->dbs->changematerial_db[i].qty_rate[j] ){
							tmp_item.amount = qty * skill->dbs->changematerial_db[i].qty[j];
							if((flag = pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
								clif->additem(sd,0,0,flag);
								map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
							}
							k++;
						}
					}
					break;
				}
			if( k ){
				clif->msgtable_skill(sd, skill_id, MSG_SKILL_SUCCESS);
				return 1;
			}
		} else if (tmp_item.amount) { //Success
			if((flag = pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
				clif->additem(sd,0,0,flag);
				map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
			}
			if( skill_id == GN_MIX_COOKING || skill_id == GN_MAKEBOMB || skill_id ==  GN_S_PHARMACY )
				clif->msgtable_skill(sd, skill_id, MSG_SKILL_SUCCESS);
			return 1;
		}
	}
	//Failure
#if 0 // TODO: update PICKLOG
	if(log_config.produce)
		log_produce(sd,nameid,slot1,slot2,slot3,0);
#endif // 0

	if(equip){
		clif->produce_effect(sd,1,nameid);
		clif->misceffect(&sd->bl,2);
	} else {
		switch (skill_id) {
			case ASC_CDP: //25% Damage yourself, and display same effect as failed potion.
				status_percent_damage(NULL, &sd->bl, -25, 0, true);
				/* Fall through */
			case AM_PHARMACY:
			case AM_TWILIGHT1:
			case AM_TWILIGHT2:
			case AM_TWILIGHT3:
				clif->produce_effect(sd,3,nameid);
				clif->misceffect(&sd->bl,6);
				sd->potion_success_counter = 0; // Fame point system [DracoRPG]
				break;
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				clif->produce_effect(sd,1,nameid);
				clif->misceffect(&sd->bl,2);
				break;
			case RK_RUNEMASTERY:
			case GC_CREATENEWPOISON:
				clif->produce_effect(sd,3,nameid);
				clif->misceffect(&sd->bl,6);
				break;
			case GN_MIX_COOKING: {
					struct item tmp_item;
					const int compensation[5] = {
						ITEMID_BLACK_LUMP,
						ITEMID_BLACK_HARD_LUMP,
						ITEMID_VERY_HARD_LUMP,
						ITEMID_BLACK_THING,
						ITEMID_MYSTERIOUS_POWDER,
					};
					int rate = rnd()%500;
					memset(&tmp_item,0,sizeof(tmp_item));
					if( rate < 50) i = 4;
					else if( rate < 100) i = 2+rnd()%1;
					else if( rate < 250 ) i = 1;
					else if( rate < 500 ) i = 0;
					tmp_item.nameid = compensation[i];
					tmp_item.amount = qty;
					tmp_item.identify = 1;
					if( pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE) ) {
						clif->additem(sd,0,0,flag);
						map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
					}
					clif->msgtable_skill(sd, skill_id, MSG_SKILL_FAILURE);
				}
				break;
			case GN_MAKEBOMB:
			case GN_S_PHARMACY:
			case GN_CHANGEMATERIAL:
				clif->msgtable_skill(sd, skill_id, MSG_SKILL_FAILURE);
				break;
			default:
				if( skill->dbs->produce_db[idx].itemlv > 10 && skill->dbs->produce_db[idx].itemlv <= 20 )
				{ //Cooking items.
					clif->specialeffect(&sd->bl, 609, AREA);
					if( sd->cook_mastery > 0 )
						pc_setglobalreg(sd, script->add_str("COOK_MASTERY"), sd->cook_mastery - ( 1 << ((skill->dbs->produce_db[idx].itemlv - 11) / 2) ) - ( ( ( 1 << ((skill->dbs->produce_db[idx].itemlv - 11) / 2) ) >> 1 ) * 3 ));
				}
		}
	}
	return 0;
}

int skill_arrow_create (struct map_session_data *sd, int nameid)
{
	int i,j,flag,index=-1;
	struct item tmp_item;

	nullpo_ret(sd);

	if(nameid <= 0)
		return 1;

	for(i=0;i<MAX_SKILL_ARROW_DB;i++)
		if(nameid == skill->dbs->arrow_db[i].nameid) {
			index = i;
			break;
		}

	if(index < 0 || (j = pc->search_inventory(sd,nameid)) == INDEX_NOT_FOUND)
		return 1;

	pc->delitem(sd, j, 1, 0, DELITEM_NORMAL, LOG_TYPE_PRODUCE); // FIXME: is this the correct reason flag?
	for(i=0;i<MAX_ARROW_RESOURCE;i++) {
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.identify = 1;
		tmp_item.nameid = skill->dbs->arrow_db[index].cre_id[i];
		tmp_item.amount = skill->dbs->arrow_db[index].cre_amount[i];
		if(battle_config.produce_item_name_input&0x4) {
			tmp_item.card[0]=CARD0_CREATE;
			tmp_item.card[1]=0;
			tmp_item.card[2]=GetWord(sd->status.char_id,0); // CharId
			tmp_item.card[3]=GetWord(sd->status.char_id,1);
		}
		if(tmp_item.nameid <= 0 || tmp_item.amount <= 0)
			continue;
		if((flag = pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
			clif->additem(sd,0,0,flag);
			map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
		}
	}

	return 0;
}
int skill_poisoningweapon( struct map_session_data *sd, int nameid) {
	sc_type type;
	int chance, i;
	nullpo_ret(sd);
	if (nameid <= 0 || (i = pc->search_inventory(sd,nameid)) == INDEX_NOT_FOUND || pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME)) {
		clif->skill_fail(sd,GC_POISONINGWEAPON,USESKILL_FAIL_LEVEL,0);
		return 0;
	}
	switch( nameid )
	{ // t_lv used to take duration from skill->get_time2
		case ITEMID_POISON_PARALYSIS:     type = SC_PARALYSE;      break;
		case ITEMID_POISON_FEVER:         type = SC_PYREXIA;       break;
		case ITEMID_POISON_CONTAMINATION: type = SC_DEATHHURT;     break;
		case ITEMID_POISON_LEECH:         type = SC_LEECHESEND;    break;
		case ITEMID_POISON_FATIGUE:       type = SC_VENOMBLEED;    break;
		case ITEMID_POISON_NUMB:          type = SC_TOXIN;         break;
		case ITEMID_POISON_LAUGHING:      type = SC_MAGICMUSHROOM; break;
		case ITEMID_POISON_OBLIVION:      type = SC_OBLIVIONCURSE; break;
		default:
			clif->skill_fail(sd,GC_POISONINGWEAPON,USESKILL_FAIL_LEVEL,0);
			return 0;
	}

	status_change_end(&sd->bl, SC_POISONINGWEAPON, INVALID_TIMER); // Status must be forced to end so that a new poison will be applied if a player decides to change poisons. [Rytech]
	chance = 2 + 2 * sd->menuskill_val; // 2 + 2 * skill_lv
	sc_start4(&sd->bl, &sd->bl, SC_POISONINGWEAPON, 100, pc->checkskill(sd, GC_RESEARCHNEWPOISON), //in Aegis it store the level of GC_RESEARCHNEWPOISON in val1
		type, chance, 0, skill->get_time(GC_POISONINGWEAPON, sd->menuskill_val));

	return 0;
}

void skill_toggle_magicpower(struct block_list *bl, uint16 skill_id) {
	struct status_change *sc = status->get_sc(bl);

	// non-offensive and non-magic skills do not affect the status
	if (skill->get_nk(skill_id)&NK_NO_DAMAGE || !(skill->get_type(skill_id)&BF_MAGIC))
		return;

	if (sc && sc->count && sc->data[SC_MAGICPOWER]) {
		if (sc->data[SC_MAGICPOWER]->val4) {
			status_change_end(bl, SC_MAGICPOWER, INVALID_TIMER);
		} else {
			sc->data[SC_MAGICPOWER]->val4 = 1;
			status_calc_bl(bl, status->sc2scb_flag(SC_MAGICPOWER));
#ifndef RENEWAL
			if (bl->type == BL_PC) {// update current display.
				struct map_session_data *sd = BL_UCAST(BL_PC, bl);
				clif->updatestatus(sd, SP_MATK1);
				clif->updatestatus(sd, SP_MATK2);
			}
#endif
		}
	}
}

int skill_magicdecoy(struct map_session_data *sd, int nameid) {
	int x, y, i, class_ = 0, skill_id;
	struct mob_data *md;
	nullpo_ret(sd);
	skill_id = sd->menuskill_val;

	if (nameid <= 0 || !itemdb_is_element(nameid) || !skill_id
	 || (i = pc->search_inventory(sd, nameid)) == INDEX_NOT_FOUND
	 || pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME) != 0
	) {
		clif->skill_fail(sd,NC_MAGICDECOY,USESKILL_FAIL_LEVEL,0);
		return 0;
	}

	// Spawn Position
	pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME); // FIXME: is this intended to be there twice?
	x = sd->sc.comet_x;
	y = sd->sc.comet_y;
	sd->sc.comet_x = sd->sc.comet_y = 0;
	sd->menuskill_val = 0;

	switch (nameid) {
	case ITEMID_SCARLET_POINT:
		class_ = MOBID_MAGICDECOY_FIRE;
		break;
	case ITEMID_INDIGO_POINT:
		class_ = MOBID_MAGICDECOY_WATER;
		break;
	case ITEMID_LIME_GREEN_POINT:
		class_ = MOBID_MAGICDECOY_WIND;
		break;
	case ITEMID_YELLOW_WISH_POINT:
		class_ = MOBID_MAGICDECOY_EARTH;
		break;
	}

	md = mob->once_spawn_sub(&sd->bl, sd->bl.m, x, y, sd->status.name, class_, "", SZ_SMALL, AI_NONE);
	if( md ) {
		md->master_id = sd->bl.id;
		md->special_state.ai = AI_FLORA;
		if( md->deletetimer != INVALID_TIMER )
			timer->delete(md->deletetimer, mob->timer_delete);
		md->deletetimer = timer->add(timer->gettick() + skill->get_time(NC_MAGICDECOY,skill_id), mob->timer_delete, md->bl.id, 0);
		mob->spawn(md);
		md->status.matk_min = md->status.matk_max = 250 + (50 * skill_id);
	}

	return 0;
}

// Warlock Spellbooks. [LimitLine/3CeAM]
int skill_spellbook (struct map_session_data *sd, int nameid) {
	int i, max_preserve, skill_id, point;
	struct status_change *sc;

	nullpo_ret(sd);

	sc = status->get_sc(&sd->bl);
	status_change_end(&sd->bl, SC_STOP, INVALID_TIMER);

	for(i=SC_SPELLBOOK1; i <= SC_SPELLBOOK7; i++) if( sc && !sc->data[i] ) break;
	if( i > SC_SPELLBOOK7 )
	{
		clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING, 0);
		return 0;
	}

	ARR_FIND(0,MAX_SKILL_SPELLBOOK_DB,i,skill->dbs->spellbook_db[i].nameid == nameid); // Search for information of this item
	if( i == MAX_SKILL_SPELLBOOK_DB ) return 0;

	if( !pc->checkskill(sd, (skill_id = skill->dbs->spellbook_db[i].skill_id)) )
	{ // User don't know the skill
		sc_start(&sd->bl, &sd->bl, SC_SLEEP, 100, 1, skill->get_time(WL_READING_SB, pc->checkskill(sd,WL_READING_SB)));
		clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP, 0);
		return 0;
	}

	max_preserve = 4 * pc->checkskill(sd, WL_FREEZE_SP) + (status_get_int(&sd->bl) + sd->status.base_level) / 10;
	point = skill->dbs->spellbook_db[i].point;

	if( sc && sc->data[SC_READING_SB] ) {
		if( (sc->data[SC_READING_SB]->val2 + point) > max_preserve ) {
			clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT, 0);
			return 0;
		}
		for(i = SC_SPELLBOOK7; i >= SC_SPELLBOOK1; i--){ // This is how official saves spellbook. [malufett]
			if( !sc->data[i] ){
				sc->data[SC_READING_SB]->val2 += point; // increase points
				sc_start4(&sd->bl, &sd->bl, (sc_type)i, 100, skill_id, pc->checkskill(sd, skill_id), point, 0, INFINITE_DURATION);
				break;
			}
		}
	} else {
		sc_start2(&sd->bl, &sd->bl, SC_READING_SB, 100, 0, point, INFINITE_DURATION);
		sc_start4(&sd->bl, &sd->bl, SC_SPELLBOOK7, 100, skill_id, pc->checkskill(sd, skill_id), point, 0, INFINITE_DURATION);
	}

	return 1;
}
int skill_select_menu(struct map_session_data *sd,uint16 skill_id) {
	int id, lv, prob, aslvl = 0, idx = 0;
	nullpo_ret(sd);

	if (sd->sc.data[SC_STOP]) {
		aslvl = sd->sc.data[SC_STOP]->val1;
		status_change_end(&sd->bl,SC_STOP,INVALID_TIMER);
	}

	idx = skill->get_index(skill_id);

	if( skill_id >= GS_GLITTERING || skill->get_type(skill_id) != BF_MAGIC ||
		(id = sd->status.skill[idx].id) == 0 || sd->status.skill[idx].flag != SKILL_FLAG_PLAGIARIZED ) {
		clif->skill_fail(sd,SC_AUTOSHADOWSPELL,0,0);
		return 0;
	}

	lv = (aslvl + 1) / 2; // The level the skill will be autocasted
	lv = min(lv,sd->status.skill[idx].lv);
	prob = (aslvl == 10) ? 15 : (32 - 2 * aslvl); // Probability at level 10 was increased to 15.
	sc_start4(&sd->bl,&sd->bl,SC__AUTOSHADOWSPELL,100,id,lv,prob,0,skill->get_time(SC_AUTOSHADOWSPELL,aslvl));
	return 0;
}

int skill_elementalanalysis(struct map_session_data *sd, uint16 skill_lv, const struct itemlist *item_list)
{
	int i;

	nullpo_ret(sd);
	nullpo_ret(item_list);

	if (VECTOR_LENGTH(*item_list) <= 0)
		return 1;

	for (i = 0; i < VECTOR_LENGTH(*item_list); i++) {
		struct item tmp_item;
		const struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, i);
		int nameid, add_amount, product;
		int del_amount = entry->amount;
		int idx = entry->id;

		if( skill_lv == 2 )
			del_amount -= (del_amount % 10);
		add_amount = (skill_lv == 1) ? del_amount * (5 + rnd()%5) : del_amount / 10 ;

		if (idx < 0 || idx >= MAX_INVENTORY
		 || (nameid = sd->status.inventory[idx].nameid) <= 0
		 || del_amount < 0 || del_amount > sd->status.inventory[idx].amount) {
			clif->skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
			return 1;
		}

		switch( nameid ) {
				// Level 1
			case ITEMID_FLAME_HEART:   product = ITEMID_BOODY_RED;       break;
			case ITEMID_MISTIC_FROZEN: product = ITEMID_CRYSTAL_BLUE;    break;
			case ITEMID_ROUGH_WIND:    product = ITEMID_WIND_OF_VERDURE; break;
			case ITEMID_GREAT_NATURE:  product = ITEMID_YELLOW_LIVE;     break;
				// Level 2
			case ITEMID_BOODY_RED:       product = ITEMID_FLAME_HEART;   break;
			case ITEMID_CRYSTAL_BLUE:    product = ITEMID_MISTIC_FROZEN; break;
			case ITEMID_WIND_OF_VERDURE: product = ITEMID_ROUGH_WIND;    break;
			case ITEMID_YELLOW_LIVE:     product = ITEMID_GREAT_NATURE;  break;
			default:
				clif->skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
				return 1;
		}

		if( pc->delitem(sd, idx, del_amount, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME) ) {
			clif->skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
			return 1;
		}

		if( skill_lv == 2 && rnd()%100 < 25 ) {
			// At level 2 have a fail chance. You loose your items if it fails.
			clif->skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
			return 1;
		}

		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid = product;
		tmp_item.amount = add_amount;
		tmp_item.identify = 1;

		if (tmp_item.amount) {
			int flag = pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_CONSUME);
			if (flag) {
				clif->additem(sd,0,0,flag);
				map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0);
			}
		}

	}

	return 0;
}

int skill_changematerial(struct map_session_data *sd, const struct itemlist *item_list)
{
	int i, j, k, c, p = 0, nameid, amount;

	nullpo_ret(sd);
	nullpo_ret(item_list);

	// Search for objects that can be created.
	for( i = 0; i < MAX_SKILL_PRODUCE_DB; i++ ) {
		if( skill->dbs->produce_db[i].itemlv == 26 ) {
			p = 0;
			do {
				c = 0;
				// Verification of overlap between the objects required and the list submitted.
				for( j = 0; j < MAX_PRODUCE_RESOURCE; j++ ) {
					if( skill->dbs->produce_db[i].mat_id[j] > 0 ) {
						for (k = 0; k < VECTOR_LENGTH(*item_list); k++) {
							const struct itemlist_entry *entry = &VECTOR_INDEX(*item_list, k);
							int idx = entry->id;
							Assert_ret(idx >= 0 && idx < MAX_INVENTORY);
							amount = entry->amount;
							nameid = sd->status.inventory[idx].nameid;
							if (nameid > 0 && sd->status.inventory[idx].identify == 0) {
								clif->msgtable_skill(sd, GN_CHANGEMATERIAL, MSG_SKILL_ITEM_NEED_IDENTIFY);
								return 0;
							}
							if( nameid == skill->dbs->produce_db[i].mat_id[j] && (amount-p*skill->dbs->produce_db[i].mat_amount[j]) >= skill->dbs->produce_db[i].mat_amount[j]
								&& (amount-p*skill->dbs->produce_db[i].mat_amount[j])%skill->dbs->produce_db[i].mat_amount[j] == 0 ) // must be in exact amount
								c++; // match
						}
					}
					else
						break; // No more items required
				}
				p++;
			} while (c == j && VECTOR_LENGTH(*item_list) == c);
			p--;
			if ( p > 0 ) {
				skill->produce_mix(sd,GN_CHANGEMATERIAL,skill->dbs->produce_db[i].nameid,0,0,0,p);
				return 1;
			}
		}
	}

	if( p == 0)
		clif->msgtable_skill(sd, GN_CHANGEMATERIAL, MSG_SKILL_ITEM_NOT_FOUND);

	return 0;
}
/**
 * for Royal Guard's LG_TRAMPLE
 **/
int skill_destroy_trap(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;
	struct skill_unit_group *sg;
	int64 tick = va_arg(ap, int64);

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_SKILL);
	su = BL_UCAST(BL_SKILL, bl);

	if (su->alive && (sg = su->group) != NULL && skill->get_inf2(sg->skill_id)&INF2_TRAP) {
		switch( sg->unit_id ) {
			case UNT_CLAYMORETRAP:
			case UNT_FIRINGTRAP:
			case UNT_ICEBOUNDTRAP:
				map->foreachinrange(skill->trap_splash,&su->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag|BL_SKILL|~BCT_SELF, &su->bl,tick);
				break;
			case UNT_LANDMINE:
			case UNT_BLASTMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
			case UNT_CLUSTERBOMB:
				map->foreachinrange(skill->trap_splash,&su->bl, skill->get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &su->bl,tick);
				break;
		}
		// Traps aren't recovered.
		skill->delunit(su);
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
int skill_blockpc_end(int tid, int64 tick, int id, intptr_t data) {
	struct map_session_data *sd = map->id2sd(id);
	struct skill_cd * cd = NULL;

	if (data <= 0 || data >= MAX_SKILL)
		return 0;
	if (!sd || !sd->blockskill[data])
		return 0;

	if( ( cd = idb_get(skill->cd_db,sd->status.char_id) ) ) {
		int i;

		for( i = 0; i < cd->cursor; i++ ) {
			if( cd->entry[i]->skidx == data )
				break;
		}

		if (i == cd->cursor) {
			ShowError("skill_blockpc_end: '%s': no data found for '%"PRIdPTR"'\n", sd->status.name, data);
		} else {
			int cursor = 0;

			ers_free(skill->cd_entry_ers, cd->entry[i]);

			cd->entry[i] = NULL;

			for( i = 0, cursor = 0; i < cd->cursor; i++ ) {
				if( !cd->entry[i] )
					continue;
				if( cursor != i )
					cd->entry[cursor] = cd->entry[i];
				cursor++;
			}

			if( (cd->cursor = cursor) == 0 ) {
				idb_remove(skill->cd_db,sd->status.char_id);
				ers_free(skill->cd_ers, cd);
			}
		}
	}

	sd->blockskill[data] = false;
	return 1;
}

/**
 * flags a singular skill as being blocked from persistent usage.
 * @param   sd        the player the skill delay affects
 * @param   skill_id  the skill which should be delayed
 * @param   tick      the length of time the delay should last
 * @return  0 if successful, -1 otherwise
 */
int skill_blockpc_start_(struct map_session_data *sd, uint16 skill_id, int tick) {
	struct skill_cd* cd = NULL;
	uint16 idx = skill->get_index(skill_id);
	int64 now = timer->gettick();

	nullpo_retr (-1, sd);

	if (idx == 0)
		return -1;

	if (tick < 1) {
		sd->blockskill[idx] = false;
		return -1;
	}

	if( battle_config.display_status_timers )
		clif->skill_cooldown(sd, skill_id, tick);

	if( !(cd = idb_get(skill->cd_db,sd->status.char_id)) ) {// create a new skill cooldown object for map storage
		cd = ers_alloc(skill->cd_ers, struct skill_cd);

		idb_put( skill->cd_db, sd->status.char_id, cd );
	} else {
		int i;

		for(i = 0; i < cd->cursor; i++) {
			if( cd->entry[i] && cd->entry[i]->skidx == idx )
				break;
		}

		if( i != cd->cursor ) {/* duplicate, update necessary */
			// Don't do anything if there's already a tick longer than the incoming one
			if (DIFF_TICK32(cd->entry[i]->started + cd->entry[i]->duration, now) > tick)
				return 0;
			cd->entry[i]->duration = tick;
#if PACKETVER >= 20120604
			cd->entry[i]->total = tick;
#endif
			cd->entry[i]->started = now;
			if( timer->settick(cd->entry[i]->timer,now+tick) != -1 )
				return 0;
			else {
				int cursor;
				/* somehow, the timer vanished. (bugreport:8367) */
				ers_free(skill->cd_entry_ers, cd->entry[i]);

				cd->entry[i] = NULL;

				for( i = 0, cursor = 0; i < cd->cursor; i++ ) {
					if( !cd->entry[i] )
						continue;
					if( cursor != i )
						cd->entry[cursor] = cd->entry[i];
					cursor++;
				}

				cd->cursor = cursor;
			}
		}
	}

	if( cd->cursor == MAX_SKILL_TREE ) {
		ShowError("skill_blockpc_start: '%s' got over '%d' skill cooldowns, no room to save!\n",sd->status.name,MAX_SKILL_TREE);
		return -1;
	}

	cd->entry[cd->cursor] = ers_alloc(skill->cd_entry_ers,struct skill_cd_entry);

	cd->entry[cd->cursor]->duration = tick;
#if PACKETVER >= 20120604
	cd->entry[cd->cursor]->total = tick;
#endif
	cd->entry[cd->cursor]->skidx = idx;
	cd->entry[cd->cursor]->skill_id = skill_id;
	cd->entry[cd->cursor]->started = now;
	cd->entry[cd->cursor]->timer = timer->add(now+tick,skill->blockpc_end,sd->bl.id,idx);

	cd->cursor++;

	sd->blockskill[idx] = true;
	return 0;
}

// [orn]
int skill_blockhomun_end(int tid, int64 tick, int id, intptr_t data)
{
	struct homun_data *hd = map->id2hd(id);
	if (data <= 0 || data >= MAX_SKILL)
		return 0;
	if (hd != NULL)
		hd->blockskill[data] = 0;

	return 1;
}

int skill_blockhomun_start(struct homun_data *hd, uint16 skill_id, int tick) { // [orn]
	uint16 idx = skill->get_index(skill_id);
	nullpo_retr (-1, hd);

	if (idx == 0)
		return -1;

	if (tick < 1) {
		hd->blockskill[idx] = 0;
		return -1;
	}
	hd->blockskill[idx] = 1;
	return timer->add(timer->gettick() + tick, skill->blockhomun_end, hd->bl.id, idx);
}

// [orn]
int skill_blockmerc_end(int tid, int64 tick, int id, intptr_t data)
{
	struct mercenary_data *md = map->id2mc(id);
	if (data <= 0 || data >= MAX_SKILL)
		return 0;
	if (md != NULL)
		md->blockskill[data] = 0;

	return 1;
}

int skill_blockmerc_start(struct mercenary_data *md, uint16 skill_id, int tick)
{
	uint16 idx = skill->get_index(skill_id);
	nullpo_retr (-1, md);

	if (idx == 0)
		return -1;
	if( tick < 1 )
	{
		md->blockskill[idx] = 0;
		return -1;
	}
	md->blockskill[idx] = 1;
	return timer->add(timer->gettick() + tick, skill->blockmerc_end, md->bl.id, idx);
}
/**
 * Adds a new skill unit entry for this player to recast after map load
 **/
void skill_usave_add(struct map_session_data * sd, uint16 skill_id, uint16 skill_lv) {
	struct skill_unit_save * sus = NULL;

	if( idb_exists(skill->usave_db,sd->status.char_id) ) {
		idb_remove(skill->usave_db,sd->status.char_id);
	}

	CREATE( sus, struct skill_unit_save, 1 );
	idb_put( skill->usave_db, sd->status.char_id, sus );

	sus->skill_id = skill_id;
	sus->skill_lv = skill_lv;

	return;
}
void skill_usave_trigger(struct map_session_data *sd) {
	struct skill_unit_save * sus = NULL;

	if( ! (sus = idb_get(skill->usave_db,sd->status.char_id)) ) {
		return;
	}

	skill->unitsetting(&sd->bl,sus->skill_id,sus->skill_lv,sd->bl.x,sd->bl.y,0);

	idb_remove(skill->usave_db,sd->status.char_id);

	return;
}
/*
 *
 */
int skill_split_atoi(char *str, int *val)
{
	int i, j, step = 1;

	for (i=0; i<MAX_SKILL_LEVEL; i++) {
		if (!str) break;
		val[i] = atoi(str);
		str = strchr(str,':');
		if (str)
			*str++=0;
	}
	if(i==0) //No data found.
		return 0;
	if(i==1) {
		//Single value, have the whole range have the same value.
		for (; i < MAX_SKILL_LEVEL; i++)
			val[i] = val[i-1];
		return i;
	}
	//Check for linear change with increasing steps until we reach half of the data acquired.
	for (step = 1; step <= i/2; step++) {
		int diff = val[i-1] - val[i-step-1];
		for(j = i-1; j >= step; j--)
			if ((val[j]-val[j-step]) != diff)
				break;

		if (j>=step) //No match, try next step.
			continue;

		for(; i < MAX_SKILL_LEVEL; i++) { //Apply linear increase
			val[i] = val[i-step]+diff;
			if (val[i] < 1 && val[i-1] >=0) //Check if we have switched from + to -, cap the decrease to 0 in said cases.
			{ val[i] = 1; diff = 0; step = 1; }
		}
		return i;
	}
	//Okay.. we can't figure this one out, just fill out the stuff with the previous value.
	for (;i<MAX_SKILL_LEVEL; i++)
		val[i] = val[i-1];
	return i;
}

/*
 *
 */
void skill_init_unit_layout (void)
{
	int i,j,pos = 0;

	//when != it was already cleared during skill_defaults() no need to repeat
	if( core->runflag == MAPSERVER_ST_RUNNING )
		memset(skill->dbs->unit_layout, 0, sizeof(skill->dbs->unit_layout));

	// standard square layouts go first
	for (i=0; i<=MAX_SQUARE_LAYOUT; i++) {
		int size = i*2+1;
		skill->dbs->unit_layout[i].count = size*size;
		for (j=0; j<size*size; j++) {
			skill->dbs->unit_layout[i].dx[j] = (j%size-i);
			skill->dbs->unit_layout[i].dy[j] = (j/size-i);
		}
	}

	// afterwards add special ones
	pos = i;
	for (i=0;i<MAX_SKILL_DB;i++) {
		if (!skill->dbs->db[i].unit_id[0] || skill->dbs->db[i].unit_layout_type[0] != -1)
			continue;

		switch (skill->dbs->db[i].nameid) {
			case MG_FIREWALL:
			case WZ_ICEWALL:
			case WL_EARTHSTRAIN://Warlock
				// these will be handled later
				break;
			case PR_SANCTUARY:
			case NPC_EVILLAND: {
					static const int dx[] = {
						-1, 0, 1,-2,-1, 0, 1, 2,-2,-1,
						 0, 1, 2,-2,-1, 0, 1, 2,-1, 0, 1};
					static const int dy[]={
						-2,-2,-2,-1,-1,-1,-1,-1, 0, 0,
						 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2};
					skill->dbs->unit_layout[pos].count = 21;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case PR_MAGNUS: {
					static const int dx[] = {
						-1, 0, 1,-1, 0, 1,-3,-2,-1, 0,
						 1, 2, 3,-3,-2,-1, 0, 1, 2, 3,
						-3,-2,-1, 0, 1, 2, 3,-1, 0, 1,-1, 0, 1};
					static const int dy[] = {
						-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,
						-1,-1,-1, 0, 0, 0, 0, 0, 0, 0,
						 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};
					skill->dbs->unit_layout[pos].count = 33;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case MH_POISON_MIST:
			case AS_VENOMDUST: {
					static const int dx[] = {-1, 0, 0, 0, 1};
					static const int dy[] = { 0,-1, 0, 1, 0};
					skill->dbs->unit_layout[pos].count = 5;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS: {
					static const int dx[] = {
						 0, 0,-1, 0, 1,-2,-1, 0, 1, 2,
						-4,-3,-2,-1, 0, 1, 2, 3, 4,-2,
						-1, 0, 1, 2,-1, 0, 1, 0, 0};
					static const int dy[] = {
						-4,-3,-2,-2,-2,-1,-1,-1,-1,-1,
						 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
						 1, 1, 1, 1, 2, 2, 2, 3, 4};
					skill->dbs->unit_layout[pos].count = 29;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case PF_FOGWALL: {
					static const int dx[] = {
						-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
					static const int dy[] = {
						-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
					skill->dbs->unit_layout[pos].count = 15;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case PA_GOSPEL: {
					static const int dx[] = {
						-1, 0, 1,-1, 0, 1,-3,-2,-1, 0,
						 1, 2, 3,-3,-2,-1, 0, 1, 2, 3,
						-3,-2,-1, 0, 1, 2, 3,-1, 0, 1,
						-1, 0, 1};
					static const int dy[] = {
						-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,
						-1,-1,-1, 0, 0, 0, 0, 0, 0, 0,
						 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
						 3, 3, 3};
					skill->dbs->unit_layout[pos].count = 33;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case NJ_KAENSIN: {
					static const int dx[] = {-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
					static const int dy[] = { 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2};
					skill->dbs->unit_layout[pos].count = 24;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case NJ_TATAMIGAESHI: {
					//Level 1 (count 4, cross of 3x3)
					static const int dx1[] = {-1, 1, 0, 0};
					static const int dy1[] = { 0, 0,-1, 1};
					//Level 2-3 (count 8, cross of 5x5)
					static const int dx2[] = {-2,-1, 1, 2, 0, 0, 0, 0};
					static const int dy2[] = { 0, 0, 0, 0,-2,-1, 1, 2};
					//Level 4-5 (count 12, cross of 7x7
					static const int dx3[] = {-3,-2,-1, 1, 2, 3, 0, 0, 0, 0, 0, 0};
					static const int dy3[] = { 0, 0, 0, 0, 0, 0,-3,-2,-1, 1, 2, 3};
					//lv1
					j = 0;
					skill->dbs->unit_layout[pos].count = 4;
					memcpy(skill->dbs->unit_layout[pos].dx,dx1,sizeof(dx1));
					memcpy(skill->dbs->unit_layout[pos].dy,dy1,sizeof(dy1));
					skill->dbs->db[i].unit_layout_type[j] = pos;
					//lv2/3
					j++;
					pos++;
					skill->dbs->unit_layout[pos].count = 8;
					memcpy(skill->dbs->unit_layout[pos].dx,dx2,sizeof(dx2));
					memcpy(skill->dbs->unit_layout[pos].dy,dy2,sizeof(dy2));
					skill->dbs->db[i].unit_layout_type[j] = pos;
					skill->dbs->db[i].unit_layout_type[++j] = pos;
					//lv4/5
					j++;
					pos++;
					skill->dbs->unit_layout[pos].count = 12;
					memcpy(skill->dbs->unit_layout[pos].dx,dx3,sizeof(dx3));
					memcpy(skill->dbs->unit_layout[pos].dy,dy3,sizeof(dy3));
					skill->dbs->db[i].unit_layout_type[j] = pos;
					skill->dbs->db[i].unit_layout_type[++j] = pos;
					//Fill in the rest using lv 5.
					for (;j<MAX_SKILL_LEVEL;j++)
						skill->dbs->db[i].unit_layout_type[j] = pos;
					//Skip, this way the check below will fail and continue to the next skill.
					pos++;
				}
				break;
			case GN_WALLOFTHORN: {
					static const int dx[] = {-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2, 2, 2, 2, 1, 0};
					static const int dy[] = { 2, 2, 1, 0,-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2, 2};
					skill->dbs->unit_layout[pos].count = 16;
					memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case EL_FIRE_MANTLE: {
				static const int dx[] = {-1, 0, 1, 1, 1, 0,-1,-1};
				static const int dy[] = { 1, 1, 1, 0,-1,-1,-1, 0};
				skill->dbs->unit_layout[pos].count = 8;
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			default:
				ShowError("unknown unit layout at skill %d\n",i);
				break;
		}
		if (!skill->dbs->unit_layout[pos].count)
			continue;
		for (j=0;j<MAX_SKILL_LEVEL;j++)
			skill->dbs->db[i].unit_layout_type[j] = pos;
		pos++;
	}

	// firewall and icewall have 8 layouts (direction-dependent)
	skill->firewall_unit_pos = pos;
	for (i=0;i<8;i++) {
		if (i&1) {
			skill->dbs->unit_layout[pos].count = 5;
			if (i&0x2) {
				int dx[] = {-1,-1, 0, 0, 1};
				int dy[] = { 1, 0, 0,-1,-1};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 1, 1 ,0, 0,-1};
				int dy[] = { 1, 0, 0,-1,-1};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			}
		} else {
			skill->dbs->unit_layout[pos].count = 3;
			if (i%4==0) {
				int dx[] = {-1, 0, 1};
				int dy[] = { 0, 0, 0};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 0, 0, 0};
				int dy[] = {-1, 0, 1};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			}
		}
		pos++;
	}
	skill->icewall_unit_pos = pos;
	for (i=0;i<8;i++) {
		skill->dbs->unit_layout[pos].count = 5;
		if (i&1) {
			if (i&0x2) {
				int dx[] = {-2,-1, 0, 1, 2};
				int dy[] = { 2, 1, 0,-1,-2};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 2, 1 ,0,-1,-2};
				int dy[] = { 2, 1, 0,-1,-2};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			}
		} else {
			if (i%4==0) {
				int dx[] = {-2,-1, 0, 1, 2};
				int dy[] = { 0, 0, 0, 0, 0};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 0, 0, 0, 0, 0};
				int dy[] = {-2,-1, 0, 1, 2};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			}
		}
		pos++;
	}
	skill->earthstrain_unit_pos = pos;
	for( i = 0; i < 8; i++ )
	{ // For each Direction
		skill->dbs->unit_layout[pos].count = 15;
		switch( i )
		{
		case 0: case 1: case 3: case 4: case 5: case 7:
			{
				int dx[] = {-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7};
				int dy[] = { 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			}
			break;
		case 2:
		case 6:
			{
				int dx[] = { 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0};
				int dy[] = {-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7};
				memcpy(skill->dbs->unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy,dy,sizeof(dy));
			}
			break;
		}
		pos++;
	}

}

int skill_block_check(struct block_list *bl, sc_type type , uint16 skill_id) {
	int inf = 0;
	struct status_change *sc = status->get_sc(bl);

	if( !sc || !bl || !skill_id )
		return 0; // Can do it

	switch(type){
		case SC_STASIS:
			inf = skill->get_inf2(skill_id);
			if( inf == INF2_SONG_DANCE || skill->get_inf2(skill_id) == INF2_CHORUS_SKILL || inf == INF2_SPIRIT_SKILL )
				return 1; // Can't do it.
			switch( skill_id ) {
				case NV_FIRSTAID:
				case TF_HIDING:
				case AS_CLOAKING:
				case WZ_SIGHTRASHER:
				case RG_STRIPWEAPON:
				case RG_STRIPSHIELD:
				case RG_STRIPARMOR:
				case WZ_METEOR:
				case RG_STRIPHELM:
				case SC_STRIPACCESSARY:
				case ST_FULLSTRIP:
				case WZ_SIGHTBLASTER:
				case ST_CHASEWALK:
				case SC_ENERVATION:
				case SC_GROOMY:
				case WZ_ICEWALL:
				case SC_IGNORANCE:
				case SC_LAZINESS:
				case SC_UNLUCKY:
				case WZ_STORMGUST:
				case SC_WEAKNESS:
				case AL_RUWACH:
				case AL_PNEUMA:
				case WZ_JUPITEL:
				case AL_HEAL:
				case AL_BLESSING:
				case AL_INCAGI:
				case WZ_VERMILION:
				case AL_TELEPORT:
				case AL_WARP:
				case AL_HOLYWATER:
				case WZ_EARTHSPIKE:
				case AL_HOLYLIGHT:
				case PR_IMPOSITIO:
				case PR_ASPERSIO:
				case WZ_HEAVENDRIVE:
				case PR_SANCTUARY:
				case PR_STRECOVERY:
				case PR_MAGNIFICAT:
				case WZ_QUAGMIRE:
				case ALL_RESURRECTION:
				case PR_LEXDIVINA:
				case PR_LEXAETERNA:
				case HW_GRAVITATION:
				case PR_MAGNUS:
				case PR_TURNUNDEAD:
				case MG_SRECOVERY:
				case HW_MAGICPOWER:
				case MG_SIGHT:
				case MG_NAPALMBEAT:
				case MG_SAFETYWALL:
				case HW_GANBANTEIN:
				case MG_SOULSTRIKE:
				case MG_COLDBOLT:
				case MG_FROSTDIVER:
				case WL_DRAINLIFE:
				case MG_STONECURSE:
				case MG_FIREBALL:
				case MG_FIREWALL:
				case WL_SOULEXPANSION:
				case MG_FIREBOLT:
				case MG_LIGHTNINGBOLT:
				case MG_THUNDERSTORM:
				case MG_ENERGYCOAT:
				case WL_WHITEIMPRISON:
				case WL_SUMMONFB:
				case WL_SUMMONBL:
				case WL_SUMMONWB:
				case WL_SUMMONSTONE:
				case WL_SIENNAEXECRATE:
				case WL_RELEASE:
				case WL_EARTHSTRAIN:
				case WL_RECOGNIZEDSPELL:
				case WL_READING_SB:
				case SA_MAGICROD:
				case SA_SPELLBREAKER:
				case SA_DISPELL:
				case SA_FLAMELAUNCHER:
				case SA_FROSTWEAPON:
				case SA_LIGHTNINGLOADER:
				case SA_SEISMICWEAPON:
				case SA_VOLCANO:
				case SA_DELUGE:
				case SA_VIOLENTGALE:
				case SA_LANDPROTECTOR:
				case PF_HPCONVERSION:
				case PF_SOULCHANGE:
				case PF_SPIDERWEB:
				case PF_FOGWALL:
				case TK_RUN:
				case TK_HIGHJUMP:
				case TK_SEVENWIND:
				case SL_KAAHI:
				case SL_KAUPE:
				case SL_KAITE:

				// Skills that need to be confirmed.
				case SO_FIREWALK:
				case SO_ELECTRICWALK:
				case SO_SPELLFIST:
				case SO_EARTHGRAVE:
				case SO_DIAMONDDUST:
				case SO_POISON_BUSTER:
				case SO_PSYCHIC_WAVE:
				case SO_CLOUD_KILL:
				case SO_STRIKING:
				case SO_WARMER:
				case SO_VACUUM_EXTREME:
				case SO_VARETYR_SPEAR:
				case SO_ARRULLO:
					return 1; // Can't do it.
			}
			break;
		case SC_KG_KAGEHUMI:
			switch(skill_id) {
				case TF_HIDING:
				case AS_CLOAKING:
				case GC_CLOAKINGEXCEED:
				case SC_SHADOWFORM:
				case MI_HARMONIZE:
				case CG_MARIONETTE:
				case AL_TELEPORT:
				case TF_BACKSLIDING:
				case RA_CAMOUFLAGE:
				case ST_CHASEWALK:
				case GD_EMERGENCYCALL:
					return 1; // needs more info
			}
			break;
	}

	return 0;
}

int skill_get_elemental_type( uint16 skill_id , uint16 skill_lv ) {
	int type = 0;

	switch (skill_id) {
		case SO_SUMMON_AGNI:   type = ELEID_EL_AGNI_S;   break;
		case SO_SUMMON_AQUA:   type = ELEID_EL_AQUA_S;   break;
		case SO_SUMMON_VENTUS: type = ELEID_EL_VENTUS_S; break;
		case SO_SUMMON_TERA:   type = ELEID_EL_TERA_S;   break;
	}

	type += skill_lv - 1;

	return type;
}

/**
 * update stored skill cooldowns for player logout
 * @param   sd     the affected player structure
 */
void skill_cooldown_save(struct map_session_data * sd) {
	int i;
	struct skill_cd* cd = NULL;
	int64 now = 0;

	// always check to make sure the session properly exists
	nullpo_retv(sd);

	if( !(cd = idb_get(skill->cd_db, sd->status.char_id)) ) {// no skill cooldown is associated with this character
		return;
	}

	now = timer->gettick();

	// process each individual cooldown associated with the character
	for( i = 0; i < cd->cursor; i++ ) {
		cd->entry[i]->duration = DIFF_TICK32(cd->entry[i]->started+cd->entry[i]->duration,now);
		if( cd->entry[i]->timer != INVALID_TIMER ) {
			timer->delete(cd->entry[i]->timer,skill->blockpc_end);
			cd->entry[i]->timer = INVALID_TIMER;
		}
	}
}

/**
 * reload stored skill cooldowns when a player logs in.
 * @param   sd     the affected player structure
 */
void skill_cooldown_load(struct map_session_data * sd) {
	int i;
	struct skill_cd* cd = NULL;
	int64 now = 0;

	// always check to make sure the session properly exists
	nullpo_retv(sd);

	if( !(cd = idb_get(skill->cd_db, sd->status.char_id)) ) {// no skill cooldown is associated with this character
		return;
	}

	clif->cooldown_list(sd->fd,cd);

	now = timer->gettick();

	// process each individual cooldown associated with the character
	for( i = 0; i < cd->cursor; i++ ) {
		cd->entry[i]->started = now;
		cd->entry[i]->timer   = timer->add(timer->gettick()+cd->entry[i]->duration,skill->blockpc_end,sd->bl.id,cd->entry[i]->skidx);
		sd->blockskill[cd->entry[i]->skidx] = true;
	}
}

/*==========================================
 * sub-function of DB reading.
 * skill_db.txt
 *------------------------------------------*/
bool skill_parse_row_skilldb(char* split[], int columns, int current) {
// id,range,hit,inf,element,nk,splash,max,list_num,castcancel,cast_defence_rate,inf2,maxcount,skill_type,blow_count,name,description
	uint16 skill_id = atoi(split[0]);
	uint16 idx;
	if( (skill_id >= GD_SKILLRANGEMIN && skill_id <= GD_SKILLRANGEMAX)
	||  (skill_id >= HM_SKILLRANGEMIN && skill_id <= HM_SKILLRANGEMAX)
	||  (skill_id >= MC_SKILLRANGEMIN && skill_id <= MC_SKILLRANGEMAX)
	||  (skill_id >= EL_SKILLRANGEMIN && skill_id <= EL_SKILLRANGEMAX) ) {
		ShowWarning("skill_parse_row_skilldb: Skill id %d is forbidden (interferes with guild/homun/mercenary skill mapping)!\n", skill_id);
		return false;
	}

	idx = skill->get_index(skill_id);
	if( !idx ) // invalid skill id
		return false;

	skill->dbs->db[idx].nameid = skill_id;
	skill->split_atoi(split[1],skill->dbs->db[idx].range);
	skill->dbs->db[idx].hit = atoi(split[2]);
	skill->dbs->db[idx].inf = atoi(split[3]);
	skill->split_atoi(split[4],skill->dbs->db[idx].element);
	skill->dbs->db[idx].nk = (int)strtol(split[5], NULL, 0);
	skill->split_atoi(split[6],skill->dbs->db[idx].splash);
	skill->dbs->db[idx].max = atoi(split[7]);
	skill->split_atoi(split[8],skill->dbs->db[idx].num);

	if( strcmpi(split[9],"yes") == 0 )
		skill->dbs->db[idx].castcancel = 1;
	else
		skill->dbs->db[idx].castcancel = 0;
	skill->dbs->db[idx].cast_def_rate = atoi(split[10]);
	skill->dbs->db[idx].inf2 = (int)strtol(split[11], NULL, 0);
	skill->split_atoi(split[12],skill->dbs->db[idx].maxcount);
	if( strcmpi(split[13],"weapon") == 0 )
		skill->dbs->db[idx].skill_type = BF_WEAPON;
	else if( strcmpi(split[13],"magic") == 0 )
		skill->dbs->db[idx].skill_type = BF_MAGIC;
	else if( strcmpi(split[13],"misc") == 0 )
		skill->dbs->db[idx].skill_type = BF_MISC;
	else
		skill->dbs->db[idx].skill_type = 0;
	skill->split_atoi(split[14],skill->dbs->db[idx].blewcount);
	safestrncpy(skill->dbs->db[idx].name, trim(split[15]), sizeof(skill->dbs->db[idx].name));
	safestrncpy(skill->dbs->db[idx].desc, trim(split[16]), sizeof(skill->dbs->db[idx].desc));
	strdb_iput(skill->name2id_db, skill->dbs->db[idx].name, skill_id);
	script->set_constant2(skill->dbs->db[idx].name, (int)skill_id, false, false);

	return true;
}

bool skill_parse_row_requiredb(char* split[], int columns, int current) {
// skill_id,HPCost,MaxHPTrigger,SPCost,HPRateCost,SPRateCost,ZenyCost,RequiredWeapons,RequiredAmmoTypes,RequiredAmmoAmount,RequiredState,SpiritSphereCost,RequiredItemID1,RequiredItemAmount1,RequiredItemID2,RequiredItemAmount2,RequiredItemID3,RequiredItemAmount3,RequiredItemID4,RequiredItemAmount4,RequiredItemID5,RequiredItemAmount5,RequiredItemID6,RequiredItemAmount6,RequiredItemID7,RequiredItemAmount7,RequiredItemID8,RequiredItemAmount8,RequiredItemID9,RequiredItemAmount9,RequiredItemID10,RequiredItemAmount10
	char* p;
	int j;

	uint16 skill_id = atoi(split[0]);
	uint16 idx = skill->get_index(skill_id);
	if( !idx ) // invalid skill id
		return false;

	skill->split_atoi(split[1],skill->dbs->db[idx].hp);
	skill->split_atoi(split[2],skill->dbs->db[idx].mhp);
	skill->split_atoi(split[3],skill->dbs->db[idx].sp);
	skill->split_atoi(split[4],skill->dbs->db[idx].hp_rate);
	skill->split_atoi(split[5],skill->dbs->db[idx].sp_rate);
	skill->split_atoi(split[6],skill->dbs->db[idx].zeny);

	//Which weapon type are required, see doc/item_db for types
	p = split[7];
	for( j = 0; j < 32; j++ ) {
		int l = atoi(p);
		if( l == 99 ) { // Any weapon
			skill->dbs->db[idx].weapon = 0;
			break;
		} else
			skill->dbs->db[idx].weapon |= 1<<l;
		p = strchr(p,':');
		if(!p)
			break;
		p++;
	}

	//FIXME: document this
	p = split[8];
	for( j = 0; j < 32; j++ ) {
		int l = atoi(p);
		if( l == 99 ) { // Any ammo type
			skill->dbs->db[idx].ammo = 0xFFFFFFFF;
			break;
		} else if( l ) // 0 stands for no requirement
			skill->dbs->db[idx].ammo |= 1<<l;
		p = strchr(p,':');
		if( !p )
			break;
		p++;
	}
	skill->split_atoi(split[9],skill->dbs->db[idx].ammo_qty);

	if(      strcmpi(split[10],"hiding")              == 0 ) skill->dbs->db[idx].state = ST_HIDING;
	else if( strcmpi(split[10],"cloaking")            == 0 ) skill->dbs->db[idx].state = ST_CLOAKING;
	else if( strcmpi(split[10],"hidden")              == 0 ) skill->dbs->db[idx].state = ST_HIDDEN;
	else if( strcmpi(split[10],"riding")              == 0 ) skill->dbs->db[idx].state = ST_RIDING;
	else if( strcmpi(split[10],"falcon")              == 0 ) skill->dbs->db[idx].state = ST_FALCON;
	else if( strcmpi(split[10],"cart")                == 0 ) skill->dbs->db[idx].state = ST_CART;
	else if( strcmpi(split[10],"shield")              == 0 ) skill->dbs->db[idx].state = ST_SHIELD;
	else if( strcmpi(split[10],"sight")               == 0 ) skill->dbs->db[idx].state = ST_SIGHT;
	else if( strcmpi(split[10],"explosionspirits")    == 0 ) skill->dbs->db[idx].state = ST_EXPLOSIONSPIRITS;
	else if( strcmpi(split[10],"cartboost")           == 0 ) skill->dbs->db[idx].state = ST_CARTBOOST;
	else if( strcmpi(split[10],"recover_weight_rate") == 0 ) skill->dbs->db[idx].state = ST_RECOV_WEIGHT_RATE;
	else if( strcmpi(split[10],"move_enable")         == 0 ) skill->dbs->db[idx].state = ST_MOVE_ENABLE;
	else if( strcmpi(split[10],"water")               == 0 ) skill->dbs->db[idx].state = ST_WATER;
	else if( strcmpi(split[10],"dragon")              == 0 ) skill->dbs->db[idx].state = ST_RIDINGDRAGON;
	else if( strcmpi(split[10],"warg")                == 0 ) skill->dbs->db[idx].state = ST_WUG;
	else if( strcmpi(split[10],"ridingwarg")          == 0 ) skill->dbs->db[idx].state = ST_RIDINGWUG;
	else if( strcmpi(split[10],"mado")                == 0 ) skill->dbs->db[idx].state = ST_MADO;
	else if( strcmpi(split[10],"elementalspirit")     == 0 ) skill->dbs->db[idx].state = ST_ELEMENTALSPIRIT;
	else if( strcmpi(split[10],"poisonweapon")        == 0 ) skill->dbs->db[idx].state = ST_POISONINGWEAPON;
	else if( strcmpi(split[10],"rollingcutter")       == 0 ) skill->dbs->db[idx].state = ST_ROLLINGCUTTER;
	else if( strcmpi(split[10],"mh_fighting")         == 0 ) skill->dbs->db[idx].state = ST_MH_FIGHTING;
	else if( strcmpi(split[10],"mh_grappling")        == 0 ) skill->dbs->db[idx].state = ST_MH_GRAPPLING;
	else if( strcmpi(split[10],"peco")                == 0 ) skill->dbs->db[idx].state = ST_PECO;
	/**
	 * Unknown or no state
	 **/
	else skill->dbs->db[idx].state = ST_NONE;

	skill->split_atoi(split[11],skill->dbs->db[idx].spiritball);
	for( j = 0; j < MAX_SKILL_ITEM_REQUIRE; j++ ) {
		skill->dbs->db[idx].itemid[j] = atoi(split[12+ 2*j]);
		skill->dbs->db[idx].amount[j] = atoi(split[13+ 2*j]);
	}

	return true;
}

bool skill_parse_row_castdb(char* split[], int columns, int current) {
// skill_id,CastingTime,AfterCastActDelay,AfterCastWalkDelay,Duration1,Duration2
	uint16 skill_id = atoi(split[0]);
	uint16 idx = skill->get_index(skill_id);
	if( !idx ) // invalid skill id
		return false;

	skill->split_atoi(split[1],skill->dbs->db[idx].cast);
	skill->split_atoi(split[2],skill->dbs->db[idx].delay);
	skill->split_atoi(split[3],skill->dbs->db[idx].walkdelay);
	skill->split_atoi(split[4],skill->dbs->db[idx].upkeep_time);
	skill->split_atoi(split[5],skill->dbs->db[idx].upkeep_time2);
	skill->split_atoi(split[6],skill->dbs->db[idx].cooldown);
#ifdef RENEWAL_CAST
	skill->split_atoi(split[7],skill->dbs->db[idx].fixed_cast);
#endif
	return true;
}

bool skill_parse_row_castnodexdb(char* split[], int columns, int current) {
// Skill id,Cast,Delay (optional)
	uint16 skill_id = atoi(split[0]);
	uint16 idx = skill->get_index(skill_id);
	if( !idx ) // invalid skill id
		return false;

	skill->split_atoi(split[1],skill->dbs->db[idx].castnodex);
	if( split[2] ) // optional column
		skill->split_atoi(split[2],skill->dbs->db[idx].delaynodex);

	return true;
}

bool skill_parse_row_unitdb(char* split[], int columns, int current) {
// ID,unit ID,unit ID 2,layout,range,interval,target,flag
	uint16 skill_id = atoi(split[0]);
	uint16 idx = skill->get_index(skill_id);
	if( !idx ) // invalid skill id
		return false;

	skill->dbs->db[idx].unit_id[0] = (int)strtol(split[1],NULL,16);
	skill->dbs->db[idx].unit_id[1] = (int)strtol(split[2],NULL,16);
	skill->split_atoi(split[3],skill->dbs->db[idx].unit_layout_type);
	skill->split_atoi(split[4],skill->dbs->db[idx].unit_range);
	skill->dbs->db[idx].unit_interval = atoi(split[5]);

	if( strcmpi(split[6],"noenemy")==0 ) skill->dbs->db[idx].unit_target = BCT_NOENEMY;
	else if( strcmpi(split[6],"friend")==0 ) skill->dbs->db[idx].unit_target = BCT_NOENEMY;
	else if( strcmpi(split[6],"party")==0 ) skill->dbs->db[idx].unit_target = BCT_PARTY;
	else if( strcmpi(split[6],"ally")==0 ) skill->dbs->db[idx].unit_target = BCT_PARTY|BCT_GUILD;
	else if( strcmpi(split[6],"guild")==0 ) skill->dbs->db[idx].unit_target = BCT_GUILD;
	else if( strcmpi(split[6],"all")==0 ) skill->dbs->db[idx].unit_target = BCT_ALL;
	else if( strcmpi(split[6],"enemy")==0 ) skill->dbs->db[idx].unit_target = BCT_ENEMY;
	else if( strcmpi(split[6],"self")==0 ) skill->dbs->db[idx].unit_target = BCT_SELF;
	else if( strcmpi(split[6],"sameguild")==0 ) skill->dbs->db[idx].unit_target = BCT_GUILD|BCT_SAMEGUILD;
	else if( strcmpi(split[6],"noone")==0 ) skill->dbs->db[idx].unit_target = BCT_NOONE;
	else skill->dbs->db[idx].unit_target = (int)strtol(split[6],NULL,16);

	skill->dbs->db[idx].unit_flag = (int)strtol(split[7],NULL,16);

	if (skill->dbs->db[idx].unit_flag&UF_DEFNOTENEMY && battle_config.defnotenemy)
		skill->dbs->db[idx].unit_target = BCT_NOENEMY;

	//By default, target just characters.
	skill->dbs->db[idx].unit_target |= BL_CHAR;
	if (skill->dbs->db[idx].unit_flag&UF_NOPC)
		skill->dbs->db[idx].unit_target &= ~BL_PC;
	if (skill->dbs->db[idx].unit_flag&UF_NOMOB)
		skill->dbs->db[idx].unit_target &= ~BL_MOB;
	if (skill->dbs->db[idx].unit_flag&UF_SKILL)
		skill->dbs->db[idx].unit_target |= BL_SKILL;

	return true;
}

bool skill_parse_row_producedb(char* split[], int columns, int current) {
// ProduceItemID,ItemLV,RequireSkill,Requireskill_lv,MaterialID1,MaterialAmount1,......
	int x,y;

	int i = atoi(split[0]);
	if( !i )
		return false;

	skill->dbs->produce_db[current].nameid = i;
	skill->dbs->produce_db[current].itemlv = atoi(split[1]);
	skill->dbs->produce_db[current].req_skill = atoi(split[2]);
	skill->dbs->produce_db[current].req_skill_lv = atoi(split[3]);

	for( x = 4, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_PRODUCE_RESOURCE; x += 2, y++ ) {
		skill->dbs->produce_db[current].mat_id[y] = atoi(split[x]);
		skill->dbs->produce_db[current].mat_amount[y] = atoi(split[x+1]);
	}

	return true;
}

bool skill_parse_row_createarrowdb(char* split[], int columns, int current) {
// SourceID,MakeID1,MakeAmount1,...,MakeID5,MakeAmount5
	int x,y;

	int i = atoi(split[0]);
	if( !i )
		return false;

	skill->dbs->arrow_db[current].nameid = i;

	for( x = 1, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_ARROW_RESOURCE; x += 2, y++ ) {
		skill->dbs->arrow_db[current].cre_id[y] = atoi(split[x]);
		skill->dbs->arrow_db[current].cre_amount[y] = atoi(split[x+1]);
	}

	return true;
}
bool skill_parse_row_spellbookdb(char* split[], int columns, int current) {
// skill_id,PreservePoints

	uint16 skill_id = atoi(split[0]);
	int points = atoi(split[1]);
	int nameid = atoi(split[2]);

	if( !skill->get_index(skill_id) || !skill->get_max(skill_id) )
		ShowError("spellbook_db: Invalid skill ID %d\n", skill_id);
	if ( !skill->get_inf(skill_id) )
		ShowError("spellbook_db: Passive skills cannot be memorized (%d/%s)\n", skill_id, skill->get_name(skill_id));
	if( points < 1 )
		ShowError("spellbook_db: PreservePoints have to be 1 or above! (%d/%s)\n", skill_id, skill->get_name(skill_id));
	else {
		skill->dbs->spellbook_db[current].skill_id = skill_id;
		skill->dbs->spellbook_db[current].point = points;
		skill->dbs->spellbook_db[current].nameid = nameid;

		return true;
	}

	return false;
}
bool skill_parse_row_improvisedb(char* split[], int columns, int current) {
// SkillID,Rate
	uint16 skill_id = atoi(split[0]);
	short j = atoi(split[1]);

	if( !skill->get_index(skill_id) || !skill->get_max(skill_id) ) {
		ShowError("skill_improvise_db: Invalid skill ID %d\n", skill_id);
		return false;
	}
	if ( !skill->get_inf(skill_id) ) {
		ShowError("skill_improvise_db: Passive skills cannot be casted (%d/%s)\n", skill_id, skill->get_name(skill_id));
		return false;
	}
	if( j < 1 ) {
		ShowError("skill_improvise_db: Chances have to be 1 or above! (%d/%s)\n", skill_id, skill->get_name(skill_id));
		return false;
	}
	if( current >= MAX_SKILL_IMPROVISE_DB ) {
		ShowError("skill_improvise_db: Maximum amount of entries reached (%d), increase MAX_SKILL_IMPROVISE_DB\n",MAX_SKILL_IMPROVISE_DB);
		return false;
	}
	skill->dbs->improvise_db[current].skill_id = skill_id;
	skill->dbs->improvise_db[current].per = j; // Still need confirm it.

	return true;
}
bool skill_parse_row_magicmushroomdb(char* split[], int column, int current) {
// SkillID
	uint16 skill_id = atoi(split[0]);

	if( !skill->get_index(skill_id) || !skill->get_max(skill_id) ) {
		ShowError("magicmushroom_db: Invalid skill ID %d\n", skill_id);
		return false;
	}
	if ( !skill->get_inf(skill_id) ) {
		ShowError("magicmushroom_db: Passive skills cannot be casted (%d/%s)\n", skill_id, skill->get_name(skill_id));
		return false;
	}

	skill->dbs->magicmushroom_db[current].skill_id = skill_id;

	return true;
}

bool skill_parse_row_reproducedb(char* split[], int column, int current) {
	uint16 skill_id = atoi(split[0]);
	uint16 idx = skill->get_index(skill_id);
	if( !idx )
		return false;

	skill->dbs->reproduce_db[idx] = true;

	return true;
}

bool skill_parse_row_abradb(char* split[], int columns, int current) {
// skill_id,DummyName,RequiredHocusPocusLevel,Rate
	uint16 skill_id = atoi(split[0]);
	if( !skill->get_index(skill_id) || !skill->get_max(skill_id) ) {
		ShowError("abra_db: Invalid skill ID %d\n", skill_id);
		return false;
	}
	if ( !skill->get_inf(skill_id) ) {
		ShowError("abra_db: Passive skills cannot be casted (%d/%s)\n", skill_id, skill->get_name(skill_id));
		return false;
	}

	skill->dbs->abra_db[current].skill_id = skill_id;
	skill->dbs->abra_db[current].req_lv = atoi(split[2]);
	skill->dbs->abra_db[current].per = atoi(split[3]);

	return true;
}

bool skill_parse_row_changematerialdb(char* split[], int columns, int current) {
// ProductID,BaseRate,MakeAmount1,MakeAmountRate1...,MakeAmount5,MakeAmountRate5
	uint16 skill_id = atoi(split[0]);
	short j = atoi(split[1]);
	int x,y;

	for(x=0; x<MAX_SKILL_PRODUCE_DB; x++){
		if( skill->dbs->produce_db[x].nameid == skill_id )
			if( skill->dbs->produce_db[x].req_skill == GN_CHANGEMATERIAL )
				break;
	}

	if( x >= MAX_SKILL_PRODUCE_DB ){
		ShowError("changematerial_db: Not supported item ID(%d) for Change Material. \n", skill_id);
		return false;
	}

	if( current >= MAX_SKILL_PRODUCE_DB ) {
		ShowError("skill_changematerial_db: Maximum amount of entries reached (%d), increase MAX_SKILL_PRODUCE_DB\n",MAX_SKILL_PRODUCE_DB);
		return false;
	}

	skill->dbs->changematerial_db[current].itemid = skill_id;
	skill->dbs->changematerial_db[current].rate = j;

	for( x = 2, y = 0; x+1 < columns && split[x] && split[x+1] && y < 5; x += 2, y++ ) {
		skill->dbs->changematerial_db[current].qty[y] = atoi(split[x]);
		skill->dbs->changematerial_db[current].qty_rate[y] = atoi(split[x+1]);
	}

	return true;
}

/*===============================
 * DB reading.
 * skill_db.txt
 * skill_require_db.txt
 * skill_cast_db.txt
 * skill_castnodex_db.txt
 * skill_nocast_db.txt
 * skill_unit_db.txt
 * produce_db.txt
 * create_arrow_db.txt
 * abra_db.txt
 *------------------------------*/
void skill_readdb(bool minimal) {
	// init skill db structures
	db_clear(skill->name2id_db);

	/* when != it was called during init and this procedure was already performed by skill_defaults()  */
	if( core->runflag == MAPSERVER_ST_RUNNING ) {
		memset(ZEROED_BLOCK_POS(skill->dbs), 0, ZEROED_BLOCK_SIZE(skill->dbs));
	}

	// load skill databases
	safestrncpy(skill->dbs->db[0].name, "UNKNOWN_SKILL", sizeof(skill->dbs->db[0].name));
	safestrncpy(skill->dbs->db[0].desc, "Unknown Skill", sizeof(skill->dbs->db[0].desc));

#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = DBPATH"skill_db.txt";
#endif // ENABLE_CASE_CHECK
	sv->readdb(map->db_path, DBPATH"skill_db.txt",           ',',  17,                       17,               MAX_SKILL_DB, skill->parse_row_skilldb);
#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = NULL;
#endif // ENABLE_CASE_CHECK

	if (minimal)
		return;

	sv->readdb(map->db_path, DBPATH"skill_require_db.txt",   ',',  32,                       32,               MAX_SKILL_DB, skill->parse_row_requiredb);
#ifdef RENEWAL_CAST
	sv->readdb(map->db_path, "re/skill_cast_db.txt",         ',',   8,                        8,               MAX_SKILL_DB, skill->parse_row_castdb);
#else
	sv->readdb(map->db_path, "pre-re/skill_cast_db.txt",     ',',   7,                        7,               MAX_SKILL_DB, skill->parse_row_castdb);
#endif
	sv->readdb(map->db_path, DBPATH"skill_castnodex_db.txt", ',',   2,                        3,               MAX_SKILL_DB, skill->parse_row_castnodexdb);
	sv->readdb(map->db_path, DBPATH"skill_unit_db.txt",      ',',   8,                        8,               MAX_SKILL_DB, skill->parse_row_unitdb);

	skill->init_unit_layout();
	sv->readdb(map->db_path, "produce_db.txt",               ',',   4, 4+2*MAX_PRODUCE_RESOURCE,       MAX_SKILL_PRODUCE_DB, skill->parse_row_producedb);
	sv->readdb(map->db_path, "create_arrow_db.txt",          ',', 1+2,   1+2*MAX_ARROW_RESOURCE,         MAX_SKILL_ARROW_DB, skill->parse_row_createarrowdb);
	sv->readdb(map->db_path, "abra_db.txt",                  ',',   4,                        4,          MAX_SKILL_ABRA_DB, skill->parse_row_abradb);
	//Warlock
	sv->readdb(map->db_path, "spellbook_db.txt",             ',',   3,                        3,     MAX_SKILL_SPELLBOOK_DB, skill->parse_row_spellbookdb);
	//Guillotine Cross
	sv->readdb(map->db_path, "magicmushroom_db.txt",         ',',   1,                        1, MAX_SKILL_MAGICMUSHROOM_DB, skill->parse_row_magicmushroomdb);
	sv->readdb(map->db_path, "skill_reproduce_db.txt",       ',',   1,                        1,               MAX_SKILL_DB, skill->parse_row_reproducedb);
	sv->readdb(map->db_path, "skill_improvise_db.txt",       ',',   2,                        2,     MAX_SKILL_IMPROVISE_DB, skill->parse_row_improvisedb);
	sv->readdb(map->db_path, "skill_changematerial_db.txt",  ',',   4,                    4+2*5,       MAX_SKILL_PRODUCE_DB, skill->parse_row_changematerialdb);
}

void skill_reload(void)
{
	struct s_mapiterator *iter;
	struct map_session_data *sd;
	int i, j, k;

	skill->read_db(false);

	//[Ind/Hercules] refresh index cache
	for (j = 0; j < CLASS_COUNT; j++) {
		for (i = 0; i < MAX_SKILL_TREE; i++) {
			struct skill_tree_entry *entry = &pc->skill_tree[j][i];
			if (entry->id == 0)
				continue;
			entry->idx = skill->get_index(entry->id);
			for (k = 0; k < VECTOR_LENGTH(entry->need); k++) {
				struct skill_tree_requirement *req = &VECTOR_INDEX(entry->need, k);
				req->idx = skill->get_index(req->id);
			}
		}
	}
	chrif->skillid2idx(0);
	/* lets update all players skill tree : so that if any skill modes were changed they're properly updated */
	iter = mapit_getallusers();
	for (sd = BL_UCAST(BL_PC, mapit->first(iter)); mapit->exists(iter); sd = BL_UCAST(BL_PC, mapit->next(iter)))
		clif->skillinfoblock(sd);
	mapit->free(iter);

}

/*==========================================
 *
 *------------------------------------------*/
int do_init_skill(bool minimal) {
	skill->name2id_db = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, MAX_SKILL_NAME_LENGTH);
	skill->read_db(minimal);

	if (minimal)
		return 0;

	skill->group_db = idb_alloc(DB_OPT_BASE);
	skill->unit_db = idb_alloc(DB_OPT_BASE);
	skill->cd_db = idb_alloc(DB_OPT_BASE);
	skill->usave_db = idb_alloc(DB_OPT_RELEASE_DATA);
	skill->bowling_db = idb_alloc(DB_OPT_BASE);
	skill->unit_ers = ers_new(sizeof(struct skill_unit_group),"skill.c::skill_unit_ers",ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);
	skill->timer_ers  = ers_new(sizeof(struct skill_timerskill),"skill.c::skill_timer_ers",ERS_OPT_NONE|ERS_OPT_FLEX_CHUNK);
	skill->cd_ers = ers_new(sizeof(struct skill_cd),"skill.c::skill_cd_ers",ERS_OPT_CLEAR|ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);
	skill->cd_entry_ers = ers_new(sizeof(struct skill_cd_entry),"skill.c::skill_cd_entry_ers",ERS_OPT_CLEAR|ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);

	ers_chunk_size(skill->cd_ers, 25);
	ers_chunk_size(skill->cd_entry_ers, 100);
	ers_chunk_size(skill->unit_ers, 150);
	ers_chunk_size(skill->timer_ers, 150);

	timer->add_func_list(skill->unit_timer,"skill_unit_timer");
	timer->add_func_list(skill->castend_id,"skill_castend_id");
	timer->add_func_list(skill->castend_pos,"skill_castend_pos");
	timer->add_func_list(skill->timerskill,"skill_timerskill");
	timer->add_func_list(skill->blockpc_end, "skill_blockpc_end");

	timer->add_interval(timer->gettick()+SKILLUNITTIMER_INTERVAL,skill->unit_timer,0,0,SKILLUNITTIMER_INTERVAL);

	return 0;
}

int do_final_skill(void) {
	db_destroy(skill->name2id_db);
	db_destroy(skill->group_db);
	db_destroy(skill->unit_db);
	db_destroy(skill->cd_db);
	db_destroy(skill->usave_db);
	db_destroy(skill->bowling_db);
	ers_destroy(skill->unit_ers);
	ers_destroy(skill->timer_ers);
	ers_destroy(skill->cd_ers);
	ers_destroy(skill->cd_entry_ers);
	return 0;
}
/* initialize the interface */
void skill_defaults(void) {
	const int skill_enchant_eff[5] = { 10, 14, 17, 19, 20 };
	const int skill_deluge_eff[5]  = {  5,  9, 12, 14, 15 };

	skill = &skill_s;
	skill->dbs = &skilldbs;

	skill->init = do_init_skill;
	skill->final = do_final_skill;
	skill->reload = skill_reload;
	skill->read_db = skill_readdb;
	/* */
	skill->cd_db = NULL;
	skill->name2id_db = NULL;
	skill->unit_db = NULL;
	skill->usave_db = NULL;
	skill->bowling_db = NULL;
	skill->group_db = NULL;
	/* */
	skill->unit_ers = NULL;
	skill->timer_ers = NULL;
	skill->cd_ers = NULL;
	skill->cd_entry_ers = NULL;

	memset(ZEROED_BLOCK_POS(skill->dbs), 0, ZEROED_BLOCK_SIZE(skill->dbs));
	memset(skill->dbs->unit_layout, 0, sizeof(skill->dbs->unit_layout));

	/* */
	memcpy(skill->enchant_eff, skill_enchant_eff, sizeof(skill->enchant_eff));
	memcpy(skill->deluge_eff, skill_deluge_eff, sizeof(skill->deluge_eff));
	skill->firewall_unit_pos = 0;
	skill->icewall_unit_pos = 0;
	skill->earthstrain_unit_pos = 0;
	memset(&skill->area_temp,0,sizeof(skill->area_temp));
	memset(&skill->unit_temp,0,sizeof(skill->unit_temp));
	skill->unit_group_newid = 0;
	/* accessors */
	skill->get_index = skill_get_index;
	skill->get_type = skill_get_type;
	skill->get_hit = skill_get_hit;
	skill->get_inf = skill_get_inf;
	skill->get_ele = skill_get_ele;
	skill->get_nk = skill_get_nk;
	skill->get_max = skill_get_max;
	skill->get_range = skill_get_range;
	skill->get_range2 = skill_get_range2;
	skill->get_splash = skill_get_splash;
	skill->get_hp = skill_get_hp;
	skill->get_mhp = skill_get_mhp;
	skill->get_sp = skill_get_sp;
	skill->get_state = skill_get_state;
	skill->get_spiritball = skill_get_spiritball;
	skill->get_zeny = skill_get_zeny;
	skill->get_num = skill_get_num;
	skill->get_cast = skill_get_cast;
	skill->get_delay = skill_get_delay;
	skill->get_walkdelay = skill_get_walkdelay;
	skill->get_time = skill_get_time;
	skill->get_time2 = skill_get_time2;
	skill->get_castnodex = skill_get_castnodex;
	skill->get_delaynodex = skill_get_delaynodex;
	skill->get_castdef = skill_get_castdef;
	skill->get_weapontype = skill_get_weapontype;
	skill->get_ammotype = skill_get_ammotype;
	skill->get_ammo_qty = skill_get_ammo_qty;
	skill->get_unit_id = skill_get_unit_id;
	skill->get_inf2 = skill_get_inf2;
	skill->get_castcancel = skill_get_castcancel;
	skill->get_maxcount = skill_get_maxcount;
	skill->get_blewcount = skill_get_blewcount;
	skill->get_unit_flag = skill_get_unit_flag;
	skill->get_unit_target = skill_get_unit_target;
	skill->get_unit_interval = skill_get_unit_interval;
	skill->get_unit_bl_target = skill_get_unit_bl_target;
	skill->get_unit_layout_type = skill_get_unit_layout_type;
	skill->get_unit_range = skill_get_unit_range;
	skill->get_cooldown = skill_get_cooldown;
	skill->tree_get_max = skill_tree_get_max;
	skill->get_name = skill_get_name;
	skill->get_desc = skill_get_desc;
	skill->chk = skill_chk;
	skill->get_casttype = skill_get_casttype;
	skill->get_casttype2 = skill_get_casttype2;
	skill->is_combo = skill_is_combo;
	skill->name2id = skill_name2id;
	skill->isammotype = skill_isammotype;
	skill->castend_id = skill_castend_id;
	skill->castend_pos = skill_castend_pos;
	skill->castend_map = skill_castend_map;
	skill->cleartimerskill = skill_cleartimerskill;
	skill->addtimerskill = skill_addtimerskill;
	skill->additional_effect = skill_additional_effect;
	skill->counter_additional_effect = skill_counter_additional_effect;
	skill->blown = skill_blown;
	skill->break_equip = skill_break_equip;
	skill->strip_equip = skill_strip_equip;
	skill->id2group = skill_id2group;
	skill->unitsetting = skill_unitsetting;
	skill->initunit = skill_initunit;
	skill->delunit = skill_delunit;
	skill->init_unitgroup = skill_initunitgroup;
	skill->del_unitgroup = skill_delunitgroup;
	skill->clear_unitgroup = skill_clear_unitgroup;
	skill->clear_group = skill_clear_group;
	skill->unit_onplace = skill_unit_onplace;
	skill->unit_ondamaged = skill_unit_ondamaged;
	skill->cast_fix = skill_castfix;
	skill->cast_fix_sc = skill_castfix_sc;
	skill->vf_cast_fix = skill_vfcastfix;
	skill->delay_fix = skill_delay_fix;
	skill->check_condition_castbegin = skill_check_condition_castbegin;
	skill->check_condition_castend = skill_check_condition_castend;
	skill->consume_requirement = skill_consume_requirement;
	skill->get_requirement = skill_get_requirement;
	skill->check_pc_partner = skill_check_pc_partner;
	skill->unit_move = skill_unit_move;
	skill->unit_onleft = skill_unit_onleft;
	skill->unit_onout = skill_unit_onout;
	skill->unit_move_unit_group = skill_unit_move_unit_group;
	skill->sit = skill_sit;
	skill->brandishspear = skill_brandishspear;
	skill->repairweapon = skill_repairweapon;
	skill->identify = skill_identify;
	skill->weaponrefine = skill_weaponrefine;
	skill->autospell = skill_autospell;
	skill->calc_heal = skill_calc_heal;
	skill->check_cloaking = skill_check_cloaking;
	skill->check_cloaking_end = skill_check_cloaking_end;
	skill->can_cloak = skill_can_cloak;
	skill->enchant_elemental_end = skill_enchant_elemental_end;
	skill->not_ok = skillnotok;
	skill->not_ok_hom = skillnotok_hom;
	skill->not_ok_mercenary = skillnotok_mercenary;
	skill->chastle_mob_changetarget = skill_chastle_mob_changetarget;
	skill->can_produce_mix = skill_can_produce_mix;
	skill->produce_mix = skill_produce_mix;
	skill->arrow_create = skill_arrow_create;
	skill->castend_nodamage_id = skill_castend_nodamage_id;
	skill->castend_damage_id = skill_castend_damage_id;
	skill->castend_pos2 = skill_castend_pos2;
	skill->blockpc_start = skill_blockpc_start_;
	skill->blockhomun_start = skill_blockhomun_start;
	skill->blockmerc_start = skill_blockmerc_start;
	skill->attack = skill_attack;
	skill->attack_area = skill_attack_area;
	skill->area_sub = skill_area_sub;
	skill->area_sub_count = skill_area_sub_count;
	skill->check_unit_range = skill_check_unit_range;
	skill->check_unit_range_sub = skill_check_unit_range_sub;
	skill->check_unit_range2 = skill_check_unit_range2;
	skill->check_unit_range2_sub = skill_check_unit_range2_sub;
	skill->toggle_magicpower = skill_toggle_magicpower;
	skill->magic_reflect = skill_magic_reflect;
	skill->onskillusage = skill_onskillusage;
	skill->cell_overlap = skill_cell_overlap;
	skill->timerskill = skill_timerskill;
	skill->trap_splash = skill_trap_splash;
	skill->check_condition_mercenary = skill_check_condition_mercenary;
	skill->locate_element_field = skill_locate_element_field;
	skill->graffitiremover = skill_graffitiremover;
	skill->activate_reverberation = skill_activate_reverberation;
	skill->dance_overlap = skill_dance_overlap;
	skill->dance_overlap_sub = skill_dance_overlap_sub;
	skill->get_unit_layout = skill_get_unit_layout;
	skill->frostjoke_scream = skill_frostjoke_scream;
	skill->greed = skill_greed;
	skill->destroy_trap = skill_destroy_trap;
	skill->unitgrouptickset_search = skill_unitgrouptickset_search;
	skill->dance_switch = skill_dance_switch;
	skill->check_condition_char_sub = skill_check_condition_char_sub;
	skill->check_condition_mob_master_sub = skill_check_condition_mob_master_sub;
	skill->brandishspear_first = skill_brandishspear_first;
	skill->brandishspear_dir = skill_brandishspear_dir;
	skill->get_fixed_cast = skill_get_fixed_cast;
	skill->sit_count = skill_sit_count;
	skill->sit_in = skill_sit_in;
	skill->sit_out = skill_sit_out;
	skill->unitsetmapcell = skill_unitsetmapcell;
	skill->unit_onplace_timer = skill_unit_onplace_timer;
	skill->unit_effect = skill_unit_effect;
	skill->unit_timer_sub_onplace = skill_unit_timer_sub_onplace;
	skill->unit_move_sub = skill_unit_move_sub;
	skill->blockpc_end = skill_blockpc_end;
	skill->blockhomun_end = skill_blockhomun_end;
	skill->blockmerc_end = skill_blockmerc_end;
	skill->split_atoi = skill_split_atoi;
	skill->unit_timer = skill_unit_timer;
	skill->unit_timer_sub = skill_unit_timer_sub;
	skill->init_unit_layout = skill_init_unit_layout;
	skill->parse_row_skilldb = skill_parse_row_skilldb;
	skill->parse_row_requiredb = skill_parse_row_requiredb;
	skill->parse_row_castdb = skill_parse_row_castdb;
	skill->parse_row_castnodexdb = skill_parse_row_castnodexdb;
	skill->parse_row_unitdb = skill_parse_row_unitdb;
	skill->parse_row_producedb = skill_parse_row_producedb;
	skill->parse_row_createarrowdb = skill_parse_row_createarrowdb;
	skill->parse_row_abradb = skill_parse_row_abradb;
	skill->parse_row_spellbookdb = skill_parse_row_spellbookdb;
	skill->parse_row_magicmushroomdb = skill_parse_row_magicmushroomdb;
	skill->parse_row_reproducedb = skill_parse_row_reproducedb;
	skill->parse_row_improvisedb = skill_parse_row_improvisedb;
	skill->parse_row_changematerialdb = skill_parse_row_changematerialdb;
	skill->usave_add = skill_usave_add;
	skill->usave_trigger = skill_usave_trigger;
	skill->cooldown_load = skill_cooldown_load;
	skill->spellbook = skill_spellbook;
	skill->block_check = skill_block_check;
	skill->detonator = skill_detonator;
	skill->check_camouflage = skill_check_camouflage;
	skill->magicdecoy = skill_magicdecoy;
	skill->poisoningweapon = skill_poisoningweapon;
	skill->select_menu = skill_select_menu;
	skill->elementalanalysis = skill_elementalanalysis;
	skill->changematerial = skill_changematerial;
	skill->get_elemental_type = skill_get_elemental_type;
	skill->cooldown_save = skill_cooldown_save;
	skill->get_new_group_id = skill_get_new_group_id;
	skill->check_shadowform = skill_check_shadowform;
	skill->additional_effect_unknown = skill_additional_effect_unknown;
	skill->counter_additional_effect_unknown = skill_counter_additional_effect_unknown;
	skill->attack_combo1_unknown = skill_attack_combo1_unknown;
	skill->attack_combo2_unknown = skill_attack_combo2_unknown;
	skill->attack_display_unknown = skill_attack_display_unknown;
	skill->attack_copy_unknown = skill_attack_copy_unknown;
	skill->attack_dir_unknown = skill_attack_dir_unknown;
	skill->attack_blow_unknown = skill_attack_blow_unknown;
	skill->attack_post_unknown = skill_attack_post_unknown;
	skill->timerskill_dead_unknown = skill_timerskill_dead_unknown;
	skill->timerskill_target_unknown = skill_timerskill_target_unknown;
	skill->timerskill_notarget_unknown = skill_timerskill_notarget_unknown;
	skill->cleartimerskill_exception = skill_cleartimerskill_exception;
	skill->castend_damage_id_unknown = skill_castend_damage_id_unknown;
	skill->castend_id_unknown = skill_castend_id_unknown;
	skill->castend_nodamage_id_dead_unknown = skill_castend_nodamage_id_dead_unknown;
	skill->castend_nodamage_id_mado_unknown = skill_castend_nodamage_id_mado_unknown;
	skill->castend_nodamage_id_undead_unknown = skill_castend_nodamage_id_undead_unknown;
	skill->castend_nodamage_id_unknown = skill_castend_nodamage_id_unknown;
	skill->castend_pos2_effect_unknown = skill_castend_pos2_effect_unknown;
	skill->castend_pos2_unknown = skill_castend_pos2_unknown;
	skill->unitsetting1_unknown = skill_unitsetting1_unknown;
	skill->unitsetting2_unknown = skill_unitsetting2_unknown;
	skill->unit_onplace_unknown = skill_unit_onplace_unknown;
	skill->check_condition_castbegin_off_unknown = skill_check_condition_castbegin_off_unknown;
	skill->check_condition_castbegin_mount_unknown = skill_check_condition_castbegin_mount_unknown;
	skill->check_condition_castbegin_madogear_unknown = skill_check_condition_castbegin_madogear_unknown;
	skill->check_condition_castbegin_unknown = skill_check_condition_castbegin_unknown;
	skill->check_condition_castend_unknown = skill_check_condition_castend_unknown;
	skill->get_requirement_off_unknown = skill_get_requirement_off_unknown;
	skill->get_requirement_item_unknown = skill_get_requirement_item_unknown;
	skill->get_requirement_unknown = skill_get_requirement_unknown;
}
