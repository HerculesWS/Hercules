/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
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

#include "config/core.h" // DBPATH, MAGIC_REFLECTION_TYPE, OFFICIAL_WALKPATH, RENEWAL, RENEWAL_CAST, VARCAST_REDUCTION()
#include "skill.h"

#include "map/battle.h"
#include "map/battleground.h"
#include "map/chrif.h"
#include "map/clan.h"
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
#include "map/messages.h"
#include "map/mob.h"
#include "map/npc.h"
#include "map/party.h"
#include "map/path.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/refine.h"
#include "map/script.h"
#include "map/status.h"
#include "map/storage.h"
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
#include "common/conf.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SKILLUNITTIMER_INTERVAL 100

static struct skill_interface skill_s;
static struct s_skill_dbs skilldbs;

struct skill_interface *skill;

static const struct {
	int start;
	int end;
} skill_idx_ranges[] = {
	{ NV_BASIC, NPC_LEX_AETERNA },
	{ KN_CHARGEATK, SA_ELEMENTWIND },
	{ RK_ENCHANTBLADE, AB_SILENTIUM },
	{ WL_WHITEIMPRISON, SC_FEINTBOMB },
	{ LG_CANNONSPEAR, SR_GENTLETOUCH_REVITALIZE },
	{ WA_SWING_DANCE, WA_MOONLIT_SERENADE },
	{ MI_RUSH_WINDMILL, MI_HARMONIZE },
	{ WM_LESSON, WM_UNLIMITED_HUMMING_VOICE },
	{ SO_FIREWALK, SO_EARTH_INSIGNIA },
	{ GN_TRAINING_SWORD, GN_SLINGITEM_RANGEMELEEATK },
	{ AB_SECRAMENT, LG_OVERBRAND_PLUSATK },
	{ ALL_ODINS_RECALL, ALL_LIGHTGUARD },
	{ RL_GLITTERING_GREED, RL_GLITTERING_GREED_ATK },
	{ SJ_LIGHTOFMOON, SJ_FALLINGSTAR_ATK2 },
	{ SP_SOULGOLEM, SP_KAUTE },
	{ KO_YAMIKUMO, OB_AKAITSUKI },
	{ ECL_SNOWFLIP, ALL_THANATOS_RECALL },
	{ GC_DARKCROW, NC_MAGMA_ERUPTION_DOTDAMAGE },
	{ SU_BASIC_SKILL, SU_SPIRITOFSEA },
	{ HLIF_HEAL, MH_VOLCANIC_ASH },
	{ MS_BASH, MER_INVINCIBLEOFF2 },
	{ EL_CIRCLE_OF_FIRE, EL_STONE_RAIN },
	{ GD_APPROVAL, GD_GUILD_STORAGE },
	CUSTOM_SKILL_RANGES
};

//Since only mob-casted splash skills can hit ice-walls
static int skill_splash_target(struct block_list *bl)
{
	nullpo_retr(BL_CHAR, bl);
#ifndef RENEWAL
	return ( bl->type == BL_MOB ) ? BL_SKILL|BL_CHAR : BL_CHAR;
#else // Some skills can now hit ground skills(traps, ice wall & etc.)
	return BL_SKILL|BL_CHAR;
#endif
}

/// Returns the id of the skill, or 0 if not found.
static int skill_name2id(const char *name)
{
	if( name == NULL )
		return 0;

	return strdb_iget(skill->name2id_db, name);
}

/// Maps skill ids to skill db offsets.
/// Returns the skill's array index, or 0 (Unknown Skill).
static int skill_get_index(int skill_id)
{
	int length = ARRAYLENGTH(skill_idx_ranges);


	if (skill_id < skill_idx_ranges[0].start || skill_id > skill_idx_ranges[length - 1].end) {
		ShowWarning("skill_get_index: skill id '%d' is not being handled!\n", skill_id);
		Assert_report(0);
		return 0;
	}

	int skill_idx = 0;
	bool found = false;
	// Map Skill ID to Skill Indexes (in reverse order)
	for (int i = 0; i < length; i++) {
		// Check if SkillID belongs to this range.
		if (skill_id <= skill_idx_ranges[i].end && skill_id >= skill_idx_ranges[i].start) {
			skill_idx += (skill_idx_ranges[i].end - skill_id);
			found = true;
			break;
		}
		// Add the difference of current range
		skill_idx += (skill_idx_ranges[i].end - skill_idx_ranges[i].start + 1);
	}

	if (!found) {
		ShowWarning("skill_get_index: skill id '%d' (idx: %d) is not handled as it lies outside the defined ranges!\n", skill_id, skill_idx);
		Assert_report(0);
		return 0;
	}
	if (skill_idx >= MAX_SKILL_DB) {
		ShowWarning("skill_get_index: skill id '%d'(idx: %d) is not being handled as it exceeds MAX_SKILL_DB!\n", skill_id, skill_idx);
		Assert_report(0);
		return 0;
	}

	return skill_idx;
}

static const char *skill_get_name(int skill_id)
{
	return skill->dbs->db[skill->get_index(skill_id)].name;
}

static const char *skill_get_desc(int skill_id)
{
	return skill->dbs->db[skill->get_index(skill_id)].desc;
}

#define skill_get_lvl_idx(lv) (min((lv), MAX_SKILL_LEVEL) - 1)
#define skill_adjust_over_level(val, lv, max_lv) ((val) > 1 ? ((val) + ((lv) - (max_lv)) / 2) : (val))

// Skill DB

/**
 * Gets a skill's hit type by its ID and level. (See enum battle_dmg_type.)
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's hit type corresponding to the passed level. Defaults to BDT_NORMAL (0) in case of error.
 *
 **/
static int skill_get_hit(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return BDT_NORMAL;

	Assert_retr(BDT_NORMAL, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(BDT_NORMAL, idx != 0);

	return skill->dbs->db[idx].hit[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_inf(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return INF_NONE;
	idx = skill->get_index(skill_id);
	Assert_retr(INF_NONE, idx != 0);
	return skill->dbs->db[idx].inf;
}

static int skill_get_ele(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return ELE_NEUTRAL;
	idx = skill->get_index(skill_id);
	Assert_retr(ELE_NEUTRAL, idx != 0);
	Assert_retr(ELE_NEUTRAL, skill_lv > 0);
	return skill->dbs->db[idx].element[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_nk(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return NK_NONE;
	idx = skill->get_index(skill_id);
	Assert_retr(NK_NONE, idx != 0);
	return skill->dbs->db[idx].nk;
}

static int skill_get_max(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	return skill->dbs->db[idx].max;
}

static int skill_get_range(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].range[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].range[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_splash(int skill_id, int skill_lv)
{
	int idx, val;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	val = skill->dbs->db[idx].splash[skill_get_lvl_idx(skill_lv)];
	if (val < 0) {
		val = AREA_SIZE;
	}
	if (skill_lv > MAX_SKILL_LEVEL) {
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return val;
}

static int skill_get_hp(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].hp[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].hp[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_sp(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].sp[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].sp[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_hp_rate(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].hp_rate[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].hp_rate[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_sp_rate(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].sp_rate[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].sp_rate[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's required state by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's required state corresponding to the passed level. Defaults to ST_NONE (0) in case of error.
 *
 **/
static int skill_get_state(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return ST_NONE;

	Assert_retr(ST_NONE, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(ST_NONE, idx != 0);

	return skill->dbs->db[idx].state[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_spiritball(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].spiritball[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].spiritball[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets the index of the first required item for a skill at given level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The required item's index. Defaults to INDEX_NOT_FOUND (-1) in case of error or if no appropriate index was found.
 *
 **/
static int skill_get_item_index(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return INDEX_NOT_FOUND;

	Assert_retr(INDEX_NOT_FOUND, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(INDEX_NOT_FOUND, idx != 0);

	int item_index = INDEX_NOT_FOUND;
	int level_index = skill_get_lvl_idx(skill_lv);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		if (skill->dbs->db[idx].req_items.item[i].id == 0)
			continue;

		if (skill->dbs->db[idx].req_items.item[i].amount[level_index] != -1) {
			item_index = i;
			break;
		}
	}

	return item_index;
}

/**
 * Gets a skill's required item's ID by the skill's ID and the item's index.
 *
 * @param skill_id The skill's ID.
 * @param item_idx The item's index.
 * @return The skill's required item's ID corresponding to the passed index. Defaults to 0 in case of error.
 *
 **/
static int skill_get_itemid(int skill_id, int item_idx)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(item_idx >= 0 && item_idx < MAX_SKILL_ITEM_REQUIRE);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].req_items.item[item_idx].id;
}

/**
 * Gets a skill's required item's amount by the skill's ID and level and the item's index.
 *
 * @param skill_id The skill's ID.
 * @param item_idx The item's index.
 * @param skill_lv The skill's level.
 * @return The skill's required item's amount corresponding to the passed index and level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_itemqty(int skill_id, int item_idx, int skill_lv)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(item_idx >= 0 && item_idx < MAX_SKILL_ITEM_REQUIRE);
	Assert_ret(skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].req_items.item[item_idx].amount[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's required items any-flag by the skill's ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's required items any-flag corresponding to the passed level. Defaults to false in case of error.
 *
 **/
static bool skill_get_item_any_flag(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return false;

	Assert_retr(false, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(false, idx != 0);

	return skill->dbs->db[idx].req_items.any[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's required equipment's ID by the skill's ID and the equipment item's index.
 *
 * @param skill_id The skill's ID.
 * @param item_idx The equipment item's index.
 * @return The skill's required equipment's ID corresponding to the passed index. Defaults to 0 in case of error.
 *
 **/
static int skill_get_equip_id(int skill_id, int item_idx)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(item_idx >= 0 && item_idx < MAX_SKILL_ITEM_REQUIRE);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].req_equip.item[item_idx].id;
}

/**
 * Gets a skill's required equipment's amount by the skill's ID and level and the equipment item's index.
 *
 * @param skill_id The skill's ID.
 * @param item_idx The equipment item's index.
 * @param skill_lv The skill's level.
 * @return The skill's required equipment item's amount corresponding to the passed index and level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_equip_amount(int skill_id, int item_idx, int skill_lv)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(item_idx >= 0 && item_idx < MAX_SKILL_ITEM_REQUIRE);
	Assert_ret(skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].req_equip.item[item_idx].amount[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's required equipment any-flag by the skill's ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's required equipment's any-flag corresponding to the passed level. Defaults to false in case of error.
 *
 **/
static bool skill_get_equip_any_flag(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return false;

	Assert_retr(false, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(false, idx != 0);

	return skill->dbs->db[idx].req_equip.any[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_zeny(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].zeny[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].zeny[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_num(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].num[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].num[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_cast(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].cast[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].cast[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_delay(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].delay[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].delay[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_walkdelay(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].walkdelay[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].walkdelay[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_time(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].upkeep_time[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].upkeep_time[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_time2(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].upkeep_time2[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].upkeep_time2[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's cast defence rate by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's cast defence rate corresponding to the passed level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_castdef(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].cast_def_rate[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_weapontype(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	return skill->dbs->db[idx].weapon;
}

static int skill_get_ammotype(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	return skill->dbs->db[idx].ammo;
}

static int skill_get_ammo_qty(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].ammo_qty[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].ammo_qty[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_inf2(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return INF2_NONE;
	idx = skill->get_index(skill_id);
	Assert_retr(INF2_NONE, idx != 0);
	return skill->dbs->db[idx].inf2;
}

/**
 * Gets a skill's cast interruptibility by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's cast interruptibility corresponding to the passed level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_castcancel(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].castcancel[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_maxcount(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].maxcount[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].maxcount[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_blewcount(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].blewcount[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].blewcount[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_mhp(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].mhp[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].mhp[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's maximum SP trigger by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's maximum SP trigger corresponding to the passed level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_msp(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].msp[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_castnodex(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].castnodex[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].castnodex[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_delaynodex(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].delaynodex[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].delaynodex[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's attack type by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's attack type corresponding to the passed level. Defaults to BF_NONE (0) in case of error.
 *
 **/
static int skill_get_type(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return BF_NONE;

	Assert_retr(BF_NONE, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(BF_NONE, idx != 0);

	return skill->dbs->db[idx].skill_type[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's unit ID by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @param flag
 * @return The skill's unit ID corresponding to the passed level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_unit_id(int skill_id, int skill_lv, int flag)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(skill_lv > 0);
	Assert_ret(flag >= 0 && flag < ARRAYLENGTH(skill->dbs->db[0].unit_id[0]));

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].unit_id[skill_get_lvl_idx(skill_lv)][flag];
}

/**
 * Gets a skill's unit interval by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's unit interval corresponding to the passed level. Defaults to 0 in case of error.
 *
 **/
static int skill_get_unit_interval(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return 0;

	Assert_ret(skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_ret(idx != 0);

	return skill->dbs->db[idx].unit_interval[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_unit_range(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].unit_range[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].unit_range[skill_get_lvl_idx(skill_lv)];
}

/**
 * Gets a skill's unit target by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's unit target corresponding to the passed level. Defaults to BCT_NOONE (0) in case of error.
 *
 **/
static int skill_get_unit_target(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return BCT_NOONE;

	Assert_retr(BCT_NOONE, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(BCT_NOONE, idx != 0);

	return (skill->dbs->db[idx].unit_target[skill_get_lvl_idx(skill_lv)] & BCT_ALL);
}

/**
 * Gets a skill's unit target as bl type by its ID and level.
 *
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return The skill's unit target as bl type corresponding to the passed level. Defaults to BL_NUL (0) in case of error.
 *
 **/
static int skill_get_unit_bl_target(int skill_id, int skill_lv)
{
	if (skill_id == 0)
		return BL_NUL;

	Assert_retr(BCT_NOONE, skill_lv > 0);

	int idx = skill->get_index(skill_id);

	Assert_retr(BCT_NOONE, idx != 0);

	return (skill->dbs->db[idx].unit_target[skill_get_lvl_idx(skill_lv)] & BL_ALL);
}

static int skill_get_unit_flag(int skill_id)
{
	int idx;
	if (skill_id == 0)
		return UF_NONE;
	idx = skill->get_index(skill_id);
	Assert_retr(UF_NONE, idx != 0);
	return skill->dbs->db[idx].unit_flag;
}

static int skill_get_unit_layout_type(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].unit_layout_type[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].unit_layout_type[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_cooldown(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].cooldown[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].cooldown[skill_get_lvl_idx(skill_lv)];
}

static int skill_get_fixed_cast(int skill_id, int skill_lv)
{
	int idx;
	if (skill_id == 0)
		return 0;
	idx = skill->get_index(skill_id);
	Assert_ret(idx != 0);
	Assert_ret(skill_lv > 0);
#ifdef RENEWAL_CAST
	if (skill_lv > MAX_SKILL_LEVEL) {
		int val = skill->dbs->db[idx].fixed_cast[skill_get_lvl_idx(skill_lv)];
		return skill_adjust_over_level(val, skill_lv, skill->dbs->db[idx].max);
	}
	return skill->dbs->db[idx].fixed_cast[skill_get_lvl_idx(skill_lv)];
#else
	return 0;
#endif
}

static int skill_tree_get_max(int skill_id, int class)
{
	int i;
	int class_idx = pc->class2idx(class);

	ARR_FIND( 0, MAX_SKILL_TREE, i, pc->skill_tree[class_idx][i].id == 0 || pc->skill_tree[class_idx][i].id == skill_id );
	if( i < MAX_SKILL_TREE && pc->skill_tree[class_idx][i].id == skill_id )
		return pc->skill_tree[class_idx][i].max;
	else
		return skill->get_max(skill_id);
}

static int skill_get_casttype(int skill_id)
{
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

static int skill_get_casttype2(int index)
{
	int inf;
	Assert_retr(CAST_NODAMAGE, index < MAX_SKILL_DB);
	inf = skill->dbs->db[index].inf;
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
static int skill_get_range2(struct block_list *bl, int skill_id, int skill_lv)
{
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

	int inf = skill->get_inf2(skill_id);

	if (sd != NULL) {
		if (inf & INF2_RANGE_VULTURE)
			range += pc->checkskill(sd, AC_VULTURE);

		if (inf & INF2_RANGE_SNAKEEYE)
			range += pc->checkskill(sd, GS_SNAKEEYE);

		if (inf & INF2_RANGE_SHADOWJUMP)
			range = skill->get_range(NJ_SHADOWJUMP, pc->checkskill(sd, NJ_SHADOWJUMP));

		if (inf & INF2_RANGE_RADIUS)
			range += pc->checkskill(sd, WL_RADIUS);

		if (inf & INF2_RANGE_RESEARCHTRAP)
			range += (1 + pc->checkskill(sd, RA_RESEARCHTRAP)) / 2;
	} else {
		if (inf & INF2_RANGE_VULTURE)
			range += battle->bc->mob_eye_range_bonus;

		if (inf & INF2_RANGE_SNAKEEYE)
			range += battle->bc->mob_eye_range_bonus;
	}

	if( !range && bl->type != BL_PC )
		return 9; // Enable non players to use self skills on others. [Skotlex]
	return range;
}

static sc_type skill_get_sc_type(int skill_id)
{
	if (skill_id == 0)
		return SC_NONE;
	return skill->dbs->db[skill->get_index(skill_id)].status_type;
}

static int skill_calc_heal(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, bool heal)
{
	int skill2_lv, hp;
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_change* sc;

	nullpo_ret(src);

	switch (skill_id) {
		case SU_TUNABELLY:
			hp = status_get_max_hp(target) * ((20 * skill_lv) - 10) / 100;
			break;
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
			if (sd != NULL && ((skill2_lv = pc->checkskill(sd, SU_POWEROFSEA)) > 0)) {
				hp += hp * 10 / 100;
				if (pc->checkskill(sd, SU_TUNABELLY) == 5 && pc->checkskill(sd, SU_TUNAPARTY) == 5 && pc->checkskill(sd, SU_BUNCHOFSHRIMP) == 5 && pc->checkskill(sd, SU_FRESHSHRIMP) == 5)
					hp += hp * 20 / 100;
			}
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
		if (sc->data[SC_VITALIZE_POTION] != NULL && skill_id != NPC_EVILLAND && skill_id != BA_APPLEIDUN)
			hp += hp * sc->data[SC_VITALIZE_POTION]->val3 / 100;
		if(sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2)
			hp += hp / 10;
		if (sc->data[SC_VITALITYACTIVATION])
			hp = hp * 150 / 100;
		if (sc->data[SC_NO_RECOVER_STATE])
			hp = 0;
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

/**
 * Making plagiarize check its own function [Aru]
 * Note: If a particular skill can be copied by both skills,
 *       Skill will be copied by the first condition which is Plagiarism[KeiKun]
 *
 * @param sd The character who cast the skill.
 * @return 1 Skill can be copied via Plagiarism
 *         2 Skill can be copied via Reproduce
 **/
static int can_copy(struct map_session_data *sd, uint16 skill_id)
{
	int cidx = skill->get_index(skill_id);
	nullpo_ret(sd);

	/// Checks if preserve is active and if skill can be copied by Plagiarism
	if (!cidx)
		return 0;

	if (sd->status.skill[cidx].id && sd->status.skill[cidx].flag == SKILL_FLAG_PLAGIARIZED)
		return 0;

	// Checks if preserve is active and if skill can be copied by Plagiarism
	if (!sd->sc.data[SC_PRESERVE] && (skill->get_inf2(skill_id) & INF2_ALLOW_PLAGIARIZE))
		return 1;

	/// Reproduce will only copy skills according on the list. [Jobbie]
	if (sd->sc.data[SC__REPRODUCE] && sd->sc.data[SC__REPRODUCE]->val1 && (skill->get_inf2(skill_id) & INF2_ALLOW_REPRODUCE))
		return 2;

	return 0;
}

// [MouseJstr] - skill ok to cast? and when?
static int skillnotok(uint16 skill_id, struct map_session_data *sd)
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

	if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNOSKILL))
		return 1; // block usage on 'noskill' cells [Wolfie]

	if (skill_id == AL_TELEPORT && sd->auto_cast_current.type == AUTOCAST_ITEM && sd->auto_cast_current.skill_lv > 2)
		return 0; // Teleport level 3 and higher bypasses this check if cast by itemskill() script commands.

	// Epoque:
	// This code will compare the player's attack motion value which is influenced by ASPD before
	// allowing a skill to be cast. This is to prevent no-delay ACT files from spamming skills such as
	// AC_DOUBLE which do not have a skill delay and are not regarded in terms of attack motion.
	if (sd->auto_cast_current.type == AUTOCAST_NONE && sd->canskill_tick != 0 &&
		DIFF_TICK(timer->gettick(), sd->canskill_tick) < (sd->battle_status.amotion * (battle_config.skill_amotion_leniency) / 100) )
	{// attempted to cast a skill before the attack motion has finished
		return 1;
	}

	if (sd->blockskill[idx]) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_SKILLINTERVAL, 0, 0);
		return 1;
	}

	/**
	 * It has been confirmed on a official server (thanks to Yommy) that item-cast skills bypass all the restrictions below
	 * Also, without this check, an exploit where an item casting + healing (or any other kind buff) isn't deleted after used on a restricted map
	 **/
	if (sd->auto_cast_current.type == AUTOCAST_ITEM)
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_THERE_ARE_NPC_AROUND, 0, 0);
				return 1;
			}
			FALLTHROUGH
		case MC_IDENTIFY:
			return 0; // always allowed
		case WZ_ICEWALL:
			// noicewall flag [Valaris]
			if (map->list[m].flag.noicewall) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 1;
			}
			break;
		case GC_DARKILLUSION:
			if( map_flag_gvg2(m) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 1;
			}
			break;
		case GD_EMERGENCYCALL:
			if( !(battle_config.emergency_call&((map->agit_flag || map->agit2_flag)?2:1))
			 || !(battle_config.emergency_call&(map->list[m].flag.gvg || map->list[m].flag.gvg_castle?8:4))
			 || (battle_config.emergency_call&16 && map->list[m].flag.nowarpto && !map->list[m].flag.gvg_castle)
			) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
		default:
			return skill->not_ok_unknown(skill_id, sd);
	}
	return (map->list[m].flag.noskill);
}

static int skill_notok_unknown(uint16 skill_id, struct map_session_data *sd)
{
	int16 m;
	nullpo_retr (1, sd);
	m = sd->bl.m;
	return (map->list[m].flag.noskill);
}

static int skillnotok_hom(uint16 skill_id, struct homun_data *hd)
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
					clif->skill_fail(hd->master, skill_id, USESKILL_FAIL_RELATIONGRADE, 0, 0);
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
	    default:
			return skill->not_ok_hom_unknown(skill_id, hd);
	}

	//Use master's criteria.
	return skill->not_ok(skill_id, hd->master);
}

static int skillnotok_hom_unknown(uint16 skill_id, struct homun_data *hd)
{
	nullpo_retr(1, hd);
	//Use master's criteria.
	return skill->not_ok(skill_id, hd->master);
}

static int skillnotok_mercenary(uint16 skill_id, struct mercenary_data *md)
{
	uint16 idx = skill->get_index(skill_id);
	nullpo_retr(1,md);

	if( idx == 0 )
		return 1; // Invalid Skill ID
	if( md->blockskill[idx] > 0 )
		return 1;

	return skill->not_ok(skill_id, md->master);
}

/**
 * Validates the plausibility of auto-cast related data and calls pc_autocast_clear() if necessary.
 *
 * @param sd The character who cast the skill.
 * @param skill_id The cast skill's ID.
 * @param skill_lv The cast skill's level. (clif_parse_UseSkillMap() passes 0.)
 *
 **/
static void skill_validate_autocast_data(struct map_session_data *sd, int skill_id, int skill_lv)
{
	nullpo_retv(sd);

	// Determine if called by clif_parse_UseSkillMap().
	bool use_skill_map = (skill_lv == 0 && (skill_id == AL_WARP || skill_id == AL_TELEPORT));

	struct autocast_data *auto_cast = &sd->auto_cast_current;

	if (auto_cast->type == AUTOCAST_NONE)
		pc->autocast_clear(sd); // No auto-cast type set. Preventively unset all auto-cast related data.
	else if (auto_cast->type == AUTOCAST_TEMP)
		pc->autocast_clear(sd); // AUTOCAST_TEMP should have been unset straight after usage.
	else if (auto_cast->skill_id == 0 || skill_id == 0 || auto_cast->skill_id != skill_id)
		pc->autocast_remove(sd, auto_cast->type, auto_cast->skill_id, auto_cast->skill_lv); // Implausible skill ID.
	else if (auto_cast->skill_lv == 0 || (!use_skill_map && (skill_lv == 0 || auto_cast->skill_lv != skill_lv)))
		pc->autocast_remove(sd, auto_cast->type, auto_cast->skill_id, auto_cast->skill_lv); // Implausible skill level.
}

static struct s_skill_unit_layout *skill_get_unit_layout(uint16 skill_id, uint16 skill_lv, struct block_list *src, int x, int y)
{
	int pos = skill->get_unit_layout_type(skill_id,skill_lv);

	nullpo_retr(&skill->dbs->unit_layout[0], src);
	if (pos < -1 || pos >= MAX_SKILL_UNIT_LAYOUT) {
		ShowError("skill_get_unit_layout: unsupported layout type %d for skill %d (level %d)\n", pos, skill_id, skill_lv);
		pos = cap_value(pos, 0, MAX_SQUARE_LAYOUT); // cap to nearest square layout
	}

	if (pos != -1) // simple single-definition layout
		return &skill->dbs->unit_layout[pos];

	enum unit_dir dir = UNIT_DIR_EAST; // default aegis direction
	if (src->x != x || src->y != y)
		dir = map->calc_dir(src, x, y);

	if (skill_id == MG_FIREWALL)
		return &skill->dbs->unit_layout [skill->firewall_unit_pos + dir];
	else if (skill_id == WZ_ICEWALL)
		return &skill->dbs->unit_layout [skill->icewall_unit_pos + dir];
	else if( skill_id == WL_EARTHSTRAIN ) //Warlock
		return &skill->dbs->unit_layout [skill->earthstrain_unit_pos + dir];
	else if (skill_id == RL_FIRE_RAIN)
		return &skill->dbs->unit_layout [skill->firerain_unit_pos + dir];

	ShowError("skill_get_unit_layout: unknown unit layout for skill %d (level %d)\n", skill_id, skill_lv);
	return &skill->dbs->unit_layout[0]; // default 1x1 layout
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_additional_effect(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, int dmg_lv, int64 tick)
{
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
				if (pc_isfalcon(sd) && sd->weapontype == W_BOW && (temp=pc->checkskill(sd,HT_BLITZBEAT))>0 &&
					rnd()%1000 <= sstatus->luk*3 ) {
					rate = sd->status.job_level / 10 + 1;
					skill->castend_damage_id(src,bl,HT_BLITZBEAT,(temp<rate)?temp:rate,tick,SD_LEVEL);
				}
				// Automatic trigger of Warg Strike [Jobbie]
				if( pc_iswug(sd) && (temp=pc->checkskill(sd,RA_WUGSTRIKE)) > 0 && rnd()%1000 <= sstatus->luk*3 )
					skill->castend_damage_id(src,bl,RA_WUGSTRIKE,temp,tick,0);
				// Gank
				if (dstmd && sd->weapontype != W_BOW &&
					(temp = pc->checkskill(sd, RG_SNATCHER)) > 0 &&
#ifdef RENEWAL
					(temp * 10) + pc->checkskill(sd, TF_STEAL) * 10 > rnd() % 1000) {
#else
					(temp * 15 + 55) + pc->checkskill(sd, TF_STEAL) * 10 > rnd() % 1000) {
#endif
					if (pc->steal_item(sd, bl, pc->checkskill(sd, TF_STEAL)))
						clif->skill_nodamage(src, bl, TF_STEAL, temp, 1);
					else
						clif->skill_fail(sd, RG_SNATCHER, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			break;

	#ifdef RENEWAL
		case WZ_FROSTNOVA:
			sc_start(src,bl,SC_FREEZE,skill_lv*5+33,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
	#endif

		case WZ_HEAVENDRIVE:
			status_change_end(bl, SC_SV_ROOTTWIST, INVALID_TIMER);
			break;

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
			sc_start4(src,bl,skill->get_sc_type(skill_id),50+10*skill_lv,
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
			sc_start(src,bl,skill->get_sc_type(skill_id),50+10*skill_lv,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;
		case NPC_ACIDBREATH:
		case NPC_ICEBREATH:
			sc_start(src,bl,skill->get_sc_type(skill_id),70,skill_lv,skill->get_time2(skill_id,skill_lv));
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
				enum sc_type type = skill->get_sc_type(skill_id);
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
			if (dstsd != NULL) // [Aegis] Doesn't apply for non-player characters.
				skill->break_equip(bl, EQP_WEAPON | EQP_ARMOR, 100 * skill_lv, BCT_ENEMY);
			break;

		case TK_DOWNKICK:
			sc_start(src,bl,SC_STUN,100,skill_lv,skill->get_time2(skill_id,skill_lv));
			break;

		case TK_JUMPKICK:
			if (dstsd != NULL && (dstsd->job & MAPID_UPPERMASK) != MAPID_SOUL_LINKER && tsc->data[SC_PRESERVE] == NULL) {
				// debuff the following statuses
				status_change_end(bl, SC_SOULLINK, INVALID_TIMER);
				status_change_end(bl, SC_ADRENALINE2, INVALID_TIMER);
				status_change_end(bl, SC_KAITE, INVALID_TIMER);
				status_change_end(bl, SC_KAAHI, INVALID_TIMER);
				status_change_end(bl, SC_ONEHANDQUICKEN, INVALID_TIMER);
				status_change_end(bl, SC_ATTHASTE_POTION3, INVALID_TIMER);
				// New soul links confirmed to not dispell with this skill
				// but thats likely a bug since soul links can't stack and
				// soul cutter skill works on them. So ill add this here for now. [Rytech]
				status_change_end(bl, SC_SOULGOLEM, INVALID_TIMER);
				status_change_end(bl, SC_SOULSHADOW, INVALID_TIMER);
				status_change_end(bl, SC_SOULFALCON, INVALID_TIMER);
				status_change_end(bl, SC_SOULFAIRY, INVALID_TIMER);
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
			sc_start(src, bl,skill->get_sc_type(skill_id),50,skill_lv,skill->get_time(EL_ROCK_CRUSHER,skill_lv));
			break;
		case EL_TYPOON_MIS:
			sc_start(src, bl,SC_SILENCE,10*skill_lv,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case KO_JYUMONJIKIRI:
			sc_start(src, bl,SC_KO_JYUMONJIKIRI,90,skill_lv,skill->get_time(skill_id,skill_lv));
			break;
		case SP_SOULEXPLOSION:
		case KO_SETSUDAN: // Remove soul link when hit.
			status_change_end(bl, SC_SOULLINK, INVALID_TIMER);
			status_change_end(bl, SC_SOULGOLEM, INVALID_TIMER);
			status_change_end(bl, SC_SOULSHADOW, INVALID_TIMER);
			status_change_end(bl, SC_SOULFALCON, INVALID_TIMER);
			status_change_end(bl, SC_SOULFAIRY, INVALID_TIMER);
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
		/**
		 * Summoner
		 */
		case SU_SCRATCH:
			sc_start2(src, bl, SC_BLOODING, (skill_lv * 3), skill_lv, src->id, skill->get_time(skill_id, skill_lv)); // TODO: What's the chance/time?
			break;
		case SU_SV_STEMSPEAR:
			sc_start2(src, bl, SC_BLOODING, 10, skill_lv, src->id, skill->get_time(skill_id, skill_lv));
			break;
		case SU_CN_METEOR:
			sc_start(src, bl, SC_CURSE, 10, skill_lv, skill->get_time2(skill_id, skill_lv)); // TODO: What's the chance/time?
			break;
		case SU_SCAROFTAROU:
			sc_start(src, bl, SC_STUN, 10, skill_lv, skill->get_time2(skill_id, skill_lv)); // TODO: What's the chance/time?
			break;
		case SU_LUNATICCARROTBEAT:
			if (skill->area_temp[3] == 1)
				sc_start(src, bl, SC_STUN, 10, skill_lv, skill_get_time(skill_id, skill_lv)); // TODO: What's the chance/time?
			break;
		case RL_S_STORM:
			skill->break_equip(bl, EQP_HEAD_TOP, max(skill_lv * 500, (sstatus->dex * skill_lv * 10) - (tstatus->agi * 20)), BCT_ENEMY);
			break;
		case RL_AM_BLAST:
			sc_start(src, bl, SC_ANTI_MATERIAL_BLAST, 20 + 10 * skill_lv, skill_lv, skill->get_time2(skill_id, skill_lv));
			break;
		case RL_BANISHING_BUSTER:
			{
				int i;
				if ((dstsd != NULL && (dstsd->job & MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
					|| (tsc && tsc->data[SC_SOULLINK] && tsc->data[SC_SOULLINK]->val2 == SL_ROGUE)
					|| (dstsd && pc_ismadogear(dstsd))
					|| rnd() % 100 >= 50 + 10 * skill_lv)
				{
					if (sd)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				if (status->isimmune(bl) || !tsc || !tsc->count)
					break;
				for (i = 0; i < SC_MAX; i++) {
					if (!tsc->data[i])
						continue;
					if (SC_COMMON_MAX < i) {
						if (status->get_sc_type(i) & SC_BB_NO_RESET)
							continue;
					}
					switch (i) {
					case SC_WHISTLE:
					case SC_ASSNCROS:
					case SC_POEMBRAGI:
					case SC_APPLEIDUN:
					case SC_HUMMING:
					case SC_DONTFORGETME:
					case SC_FORTUNE:
					case SC_SERVICEFORYOU:
						if (tsc->data[i]->val4)
							continue;
						break;
					case SC_ASSUMPTIO:
						if (bl->type == BL_MOB)
							continue;
						break;
					case SC_BERSERK:
					case SC_SATURDAY_NIGHT_FEVER:
						tsc->data[i]->val2 = 0;
						break;
					}
					status_change_end(bl, (sc_type)i, INVALID_TIMER);
				}
			}
			break;
		case RL_SLUGSHOT:
			sc_start(src, bl, SC_STUN, 100, skill_lv, skill->get_time2(skill_id, skill_lv));
			break;
		case RL_MASS_SPIRAL:
			sc_start(src, bl, SC_BLOODING, 30 + 10 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SJ_FULLMOONKICK:
			sc_start(src, bl, SC_BLIND, 15 + 5 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SJ_STAREMPEROR:
			sc_start(src, bl, SC_SILENCE, 50 + 10 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SP_CURSEEXPLOSION:
			status_change_end(bl, SC_SOULCURSE, INVALID_TIMER);
			break;
		case SP_SHA:
			sc_start(src, bl, SC_SP_SHA, 100, skill_lv, skill->get_time(skill_id, skill_lv));
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
			if (sd->no_recover_state_race[tstatus->race].rate)
				status->change_start(src, bl, SC_NO_RECOVER_STATE, sd->no_recover_state_race[tstatus->race].rate,
					0, 0, 0, 0, sd->no_recover_state_race[tstatus->race].tick, SCFLAG_FIXEDTICK);
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
						clif->status_change(src, status->get_sc_icon(SC_POSTDELAY), status->get_sc_relevant_bl_types(SC_POSTDELAY), 1, rate, 0, 0, 0);
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

			sd->auto_cast_current.type = AUTOCAST_TEMP;
			notok = skill->not_ok(temp, sd);
			sd->auto_cast_current.type = AUTOCAST_NONE;

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

			sd->auto_cast_current.type = AUTOCAST_TEMP;
			skill->consume_requirement(sd,temp,auto_skill_lv,1);
			skill->toggle_magicpower(src, temp, auto_skill_lv);
			skill->castend_type(type, src, tbl, temp, auto_skill_lv, tick, 0);
			sd->auto_cast_current.type = AUTOCAST_NONE;

			//Set canact delay. [Skotlex]
			ud = unit->bl2ud(src);
			if (ud) {
				rate = skill->delay_fix(src, temp, auto_skill_lv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if (battle_config.display_status_timers)
						clif->status_change(src, status->get_sc_icon(SC_POSTDELAY), status->get_sc_relevant_bl_types(SC_POSTDELAY), 1, rate, 0, 0, 0);
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
			(monster->status.mode&(MD_BOSS|MD_PLANT) || monster->summonper[MOBG_DEAD_BRANCH] <= rate) &&
			(temp++) < 2000);
		if (temp < 2000)
			mob->class_change(dstmd,class_);
	}

	return 0;
}

static void skill_additional_effect_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int *attack_type, int *dmg_lv, int64 *tick)
{
}

static int skill_onskillusage(struct map_session_data *sd, struct block_list *bl, uint16 skill_id, int64 tick)
{
	int temp, skill_lv, i, type, notok;
	struct block_list *tbl;

	if( sd == NULL || !skill_id )
		return 0;

	// Preserve auto-cast type if bAutoSpellOnSkill was triggered by a skill which was cast by Abracadabra, Improvised Song or an item.
	enum autocast_type ac_type = sd->auto_cast_current.type;

	for( i = 0; i < ARRAYLENGTH(sd->autospell3) && sd->autospell3[i].flag; i++ ) {
		if( sd->autospell3[i].flag != skill_id )
			continue;

		if( sd->autospell3[i].lock )
			continue;  // autospell already being executed

		temp = (sd->autospell3[i].id > 0) ? sd->autospell3[i].id : -sd->autospell3[i].id;

		sd->auto_cast_current.type = AUTOCAST_TEMP;
		notok = skill->not_ok(temp, sd);
		sd->auto_cast_current.type = AUTOCAST_NONE;

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

		sd->autospell3[i].lock = true;
		sd->auto_cast_current.type = AUTOCAST_TEMP;
		skill->consume_requirement(sd,temp,skill_lv,1);
		skill->castend_type(type, &sd->bl, tbl, temp, skill_lv, tick, 0);
		sd->auto_cast_current.type = AUTOCAST_NONE;
		sd->autospell3[i].lock = false;
	}

	sd->auto_cast_current.type = ac_type;

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
static int skill_counter_additional_effect(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, int64 tick)
{
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
					for(int i = 0; i < sc->data[SC_FORCEOFVANGUARD]->val3 && sc->fv_counter <= sc->data[SC_FORCEOFVANGUARD]->val3 ; i++)
						clif->millenniumshield(bl, sc->fv_counter++);
				}
				break;
		case SP_SPA:
			sc_start(src, src, SC_USE_SKILL_SP_SPA, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		case SP_SHA:
			sc_start(src, src, SC_USE_SKILL_SP_SHA, 100, skill_lv, skill->get_time2(skill_id, skill_lv));
			break;
		case SP_SWHOO:
			sc_start(src, src, SC_USE_SKILL_SP_SHA, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;
		default:
			skill->counter_additional_effect_unknown(src, bl, &skill_id, &skill_lv, &attack_type, &tick);
			break;
	}

	if (sd != NULL && (sd->job & MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR
	 && rnd()%10000 < battle_config.sg_miracle_skill_ratio) // SG_MIRACLE [Komurka]
		sc_start(src,src,SC_MIRACLE,100,1,battle_config.sg_miracle_skill_duration);

	if( sd && skill_id && attack_type&BF_MAGIC && status->isdead(bl)
	 && !(skill->get_inf(skill_id)&(INF_GROUND_SKILL|INF_SELF_SKILL))
	 && (rate=pc->checkskill(sd,HW_SOULDRAIN)) > 0
	) {
		// Soul Drain should only work on targeted spells [Skotlex]
		if( pc_issit(sd) ) pc->setstand(sd); // Character stuck in attacking animation while 'sitting' fix. [Skotlex]
		if (skill->get_nk(skill_id)&NK_SPLASH && skill->area_temp[1] != bl->id) {
			;
		} else {
			clif->skill_nodamage(src,bl,HW_SOULDRAIN,rate,1);
			status->heal(src, 0, status->get_lv(bl)*(95+15*rate)/100, STATUS_HEAL_SHOWEFFECT);
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
		if (hp != 0 || sp != 0) {
			// updated to force healing to allow healing through berserk
			status->heal(src, hp, sp, STATUS_HEAL_FORCED | (battle_config.show_hp_sp_gain ? STATUS_HEAL_SHOWEFFECT : STATUS_HEAL_DEFAULT));
		}
	}

	// Trigger counter-spells to retaliate against damage causing skills.
	if(dstsd && !status->isdead(bl) && dstsd->autospell2[0].id && !(skill_id && skill->get_nk(skill_id)&NK_NO_DAMAGE)) {
		struct block_list *tbl;
		struct unit_data *ud;
		int i, auto_skill_id, auto_skill_lv, type, notok;

		// Preserve auto-cast type if bAutoSpellWhenHit was triggered during cast of a skill which was cast by Abracadabra, Improvised Song or an item.
		enum autocast_type ac_type = dstsd->auto_cast_current.type;

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

			dstsd->auto_cast_current.type = AUTOCAST_TEMP;
			notok = skill->not_ok(auto_skill_id, dstsd);
			dstsd->auto_cast_current.type = AUTOCAST_NONE;

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

			dstsd->auto_cast_current.type = AUTOCAST_TEMP;
			skill->consume_requirement(dstsd,auto_skill_id,auto_skill_lv,1);
			skill->castend_type(type, bl, tbl, auto_skill_id, auto_skill_lv, tick, 0);
			dstsd->auto_cast_current.type = AUTOCAST_NONE;

			// Set canact delay. [Skotlex]
			ud = unit->bl2ud(bl);
			if (ud) {
				rate = skill->delay_fix(bl, auto_skill_id, auto_skill_lv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if (battle_config.display_status_timers)
						clif->status_change(bl, status->get_sc_icon(SC_POSTDELAY), status->get_sc_relevant_bl_types(SC_POSTDELAY), 1, rate, 0, 0, 0);
				}
			}
		}

		dstsd->auto_cast_current.type = ac_type;
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

static void skill_counter_additional_effect_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int *attack_type, int64 *tick)
{
}

/*=========================================================================
 * Breaks equipment. On-non players causes the corresponding strip effect.
 * - rate goes from 0 to 10000 (100.00%)
 * - flag is a BCT_ flag to indicate which type of adjustment should be used
 *   (BCT_ENEMY/BCT_PARTY/BCT_SELF) are the valid values.
 *------------------------------------------------------------------------*/
static int skill_break_equip(struct block_list *bl, unsigned short where, int rate, int flag)
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
			switch (sd->weapontype) {
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
			if (j < 0 || (sd->status.inventory[j].attribute & ATTR_BROKEN) != 0 || !sd->inventory_data[j])
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
				sd->status.inventory[j].attribute |= ATTR_BROKEN;
				pc->unequipitem(sd, j, PCUNEQUIPITEM_RECALC|PCUNEQUIPITEM_FORCE);
			}
		}
		clif->equipList(sd);
	}

	return where; //Return list of pieces broken.
}

static int skill_strip_equip(struct block_list *bl, unsigned short where, int rate, int lv, int time)
{
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
 * Used to knock back players, monsters, traps, etc
 * 'count' is the number of squares to knock back
 * 'direction' indicates the way OPPOSITE to the knockback direction (or UNIT_DIR_UNDEFINED for default behavior)
 * if 'flag&0x1', position update packets must not be sent.
 * if 'flag&0x2', skill blown ignores players' special_state.no_knockback
 */
static int skill_blown(struct block_list *src, struct block_list *target, int count, enum unit_dir dir, int flag)
{
	struct status_change *tsc = status->get_sc(target);

	nullpo_ret(src);

	if (src != target && map->list[src->m].flag.noknockback)
		return 0; // No knocking

	nullpo_ret(target);
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
		case BL_NUL:
		case BL_CHAT:
		case BL_HOM:
		case BL_MER:
		case BL_ELEM:
		case BL_PET:
		case BL_ITEM:
		case BL_NPC:
		case BL_ALL:
			break;
	}

	if (dir == UNIT_DIR_UNDEFINED) // <optimized>: do the computation here instead of outside
		dir = map->calc_dir(target, src->x, src->y); // direction from src to target, reversed

	dir = unit_get_opposite_dir(dir); // take the reversed 'direction' and reverse it

	if (tsc != NULL && tsc->data[SC_SU_STOOP]) // Any knockback will cancel it.
		status_change_end(target, SC_SU_STOOP, INVALID_TIMER);

	return unit->push(target, dir, count, (flag & 0x1) == 0x0); // send over the proper flag
}

/*
	Checks if 'bl' should reflect back a spell cast by 'src'.
	type is the type of magic attack: 0: indirect (aoe), 1: direct (targeted)
	In case of success returns type of reflection, otherwise 0
		1 - Regular reflection (Maya)
		2 - SL_KAITE reflection
*/
static int skill_magic_reflect(struct block_list *src, struct block_list *bl, int type)
{
	struct status_change *sc = status->get_sc(bl);
	struct map_session_data* sd = BL_CAST(BL_PC, bl);

	nullpo_ret(src);
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
static int skill_attack(int attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
	GUARD_MAP_LOCK

	struct Damage dmg;
#if MAGIC_REFLECTION_TYPE
	struct status_data *sstatus, *tstatus;
#else
	struct status_data *tstatus;
#endif
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

#if MAGIC_REFLECTION_TYPE
	sstatus = status->get_status_data(src);
#endif
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
		if (sc && sc->data[SC_MAGICROD] && src == dsrc) {
			int sp = skill->get_sp(skill_id, skill_lv);
			dmg.damage = dmg.damage2 = 0;
			dmg.dmg_lv = ATK_MISS; //This will prevent skill additional effect from taking effect. [Skotlex]
			sp = sp * sc->data[SC_MAGICROD]->val2 / 100;
			if (skill_id == WZ_WATERBALL && skill_lv > 1)
				sp = sp / ((skill_lv | 1) * (skill_lv | 1)); //Estimate SP cost of a single water-ball
			status->heal(bl, 0, sp, STATUS_HEAL_SHOWEFFECT);
			if (battle->bc->magicrod_type == 1)
				clif->skill_nodamage(bl, bl, SA_MAGICROD, sc->data[SC_MAGICROD]->val1, 1); // Animation used here in eAthena [Wolfie]
		}
	}

	if (bl->type == BL_MOB) {
		struct mob_data *md = BL_CAST(BL_MOB, bl);
		if (md != NULL) {
			if (md->db->dmg_taken_rate != 100) {
				if (dmg.damage > 0)
					dmg.damage = apply_percentrate64(dmg.damage, md->db->dmg_taken_rate, 100);
				if (dmg.damage2 > 0)
					dmg.damage2 = apply_percentrate64(dmg.damage2, md->db->dmg_taken_rate, 100);
			}
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
	type = (skill_id == 0) ? BDT_SPLASH : skill->get_hit(skill_id, skill_lv);

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
				if (pc->fame_rank(sd->status.char_id, RANKTYPE_TAEKWON) > 0) { //Extend combo time.
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
			case SJ_PROMINENCEKICK:
				if (pc->checkskill(sd, SJ_SOLARBURST) > 0)
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
			FALLTHROUGH
		case WL_SOULEXPANSION:
		case WL_COMET:
		case NJ_HUUMA:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,skill_lv,BDT_MULTIHIT);
			break;
		case WL_CHAINLIGHTNING_ATK:
			dmg.dmotion = clif->skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,WL_CHAINLIGHTNING_ATK,-2,BDT_SKILL);
			break;
		case LG_OVERBRAND_BRANDISH:
		case LG_OVERBRAND:
			/* Fall through */
			dmg.amotion = status_get_amotion(src) * 2;
			FALLTHROUGH
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
		case SP_CURSEEXPLOSION:
		case SP_SPA:
		case SP_SHA:
			if (dmg.div_ < 2)
				type = BDT_SPLASH;
			if ((flag & SD_ANIMATION) == 0)
				clif->skill_nodamage(dsrc, bl, skill_id, skill_lv, 1);
			FALLTHROUGH
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
			FALLTHROUGH
		case HT_LANDMINE:
			dmg.dmotion = clif->skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, type);
			break;
		case HW_GRAVITATION:
			dmg.dmotion = clif->damage(bl, bl, 0, 0, damage, 1, BDT_ENDURE, 0);
			break;
		case WZ_SIGHTBLASTER:
			dmg.dmotion = clif->skill_damage(src,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, (flag&SD_LEVEL) ? -1 : skill_lv, BDT_SPLASH);
			break;
		case RL_R_TRIP_PLUSATK:
		case RL_S_STORM:
			dmg.dmotion = clif->skill_damage(dsrc, bl, tick, status_get_amotion(src), dmg.dmotion, damage, dmg.div_, skill_id, -1, BDT_SPLASH);
			break;
		case SJ_FALLINGSTAR_ATK:
		case SJ_FALLINGSTAR_ATK2:
			dmg.dmotion = clif->skill_damage(src, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, BDT_MULTIHIT);
			break;
		case SJ_NOVAEXPLOSING:
			dmg.dmotion = clif->skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, BDT_SKILL);
			break;
		case AB_DUPLELIGHT_MELEE:
		case AB_DUPLELIGHT_MAGIC:
			dmg.amotion = 300;/* makes the damage value not overlap with previous damage (when displayed by the client) */
			FALLTHROUGH
		default:
			skill->attack_display_unknown(&attack_type, src, dsrc, bl, &skill_id, &skill_lv, &tick, &flag, &type, &dmg, &damage);
			break;
	}

	map->freeblock_lock();

	// Plagiarism and Reproduce Code Block [KeiKun]
	if (bl->type == BL_PC && damage > 0 && dmg.flag&BF_SKILL && tsd
		&& (pc->checkskill(tsd, RG_PLAGIARISM) || pc->checkskill(tsd, SC_REPRODUCE))
		&& (!tsd->sc.data[SC_PRESERVE] || tsd->sc.data[SC__REPRODUCE])
		&& damage < tsd->battle_status.hp // Updated to not be able to copy skills if the blow will kill you. [Skotlex]
		) {
		int copy_skill = 0;
		/**
		 * Copy Referral: dummy skills should point to their source upon copying
		 **/
		switch(skill_id) {
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

		int cidx, idx, lv = 0;
		cidx = skill->get_index(copy_skill);
		switch(can_copy(tsd, copy_skill)) {
		case 1: // Plagiarism
		{
			if (tsd->cloneskill_id) {
				idx = skill->get_index(tsd->cloneskill_id);
				if (tsd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED) {
					tsd->status.skill[idx].id = 0;
					tsd->status.skill[idx].lv = 0;
					tsd->status.skill[idx].flag = 0;
					clif->deleteskill(tsd, tsd->cloneskill_id);
				}
			}

			lv = min(skill_lv, pc->checkskill(tsd, RG_PLAGIARISM));

			tsd->cloneskill_id = copy_skill;
			pc_setglobalreg(tsd, script->add_variable("CLONE_SKILL"), copy_skill);
			pc_setglobalreg(tsd, script->add_variable("CLONE_SKILL_LV"), lv);

			tsd->status.skill[cidx].id = copy_skill;
			tsd->status.skill[cidx].lv = lv;
			tsd->status.skill[cidx].flag = SKILL_FLAG_PLAGIARIZED;
			clif->addskill(tsd, copy_skill);
		}
		break;
		case 2: // Reproduce
		{	
			lv = sc ? sc->data[SC__REPRODUCE]->val1 : 1;
			if (tsd->reproduceskill_id) {
				idx = skill->get_index(tsd->reproduceskill_id);
				if (tsd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED) {
					tsd->status.skill[idx].id = 0;
					tsd->status.skill[idx].lv = 0;
					tsd->status.skill[idx].flag = 0;
					clif->deleteskill(tsd, tsd->reproduceskill_id);
				}
			}
			lv = min(lv, skill->get_max(copy_skill));

			tsd->reproduceskill_id = copy_skill;
			pc_setglobalreg(tsd, script->add_variable("REPRODUCE_SKILL"), copy_skill);
			pc_setglobalreg(tsd, script->add_variable("REPRODUCE_SKILL_LV"), lv);

			tsd->status.skill[cidx].id = copy_skill;
			tsd->status.skill[cidx].lv = lv;
			tsd->status.skill[cidx].flag = SKILL_FLAG_PLAGIARIZED;
			clif->addskill(tsd, copy_skill);
		}
		break;
		default:
		break;
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
		if ((!sc || (!sc->data[SC_DEVOTION] && skill_id != CR_REFLECTSHIELD && !sc->data[SC_WATER_SCREEN_OPTION])) && !shadow_flag)
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
		enum unit_dir dir = UNIT_DIR_UNDEFINED; // default
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
					dir = rnd() % UNIT_DIR_MAX;
				break;
			case WL_CRIMSONROCK:
				dir = map->calc_dir(bl,skill->area_temp[4],skill->area_temp[5]);
				break;
			case MC_CARTREVOLUTION:
				dir = UNIT_DIR_EAST; // Official servers push target to the West
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
					if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
						map->freeblock_unlock(); // unblock before assert-returning
						return 0;
					}
					dir_x = dirx[unit_get_opposite_dir(dir)];
					dir_y = diry[unit_get_opposite_dir(dir)];
					if (map->getcell(bl->m, bl, bl->x + dir_x, bl->y + dir_y, CELL_CHKNOPASS) != 0)
						skill->addtimerskill(src, tick + 300 * ((flag&2) ? 1 : 2), bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag|4);
				}
				break;
			case RL_R_TRIP:
				if (skill->blown(dsrc, bl, dmg.blewcount, dir, 0) < dmg.blewcount)
					skill->addtimerskill(src, tick + status_get_amotion(src), bl->id, 0, 0, RL_R_TRIP_PLUSATK, skill_lv, BF_WEAPON, flag | SD_ANIMATION);
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

	if (sc != NULL && skill_id != PA_PRESSURE && skill_id != SJ_NOVAEXPLOSING && skill_id != SP_SOULEXPLOSION) {
		if (sc->data[SC_DEVOTION]) {
			struct status_change_entry *sce = sc->data[SC_DEVOTION];
			struct block_list *d_bl = map->id2bl(sce->val1);
			struct mercenary_data *d_md = BL_CAST(BL_MER, d_bl);
			struct map_session_data *d_sd = BL_CAST(BL_PC, d_bl);

			if (d_bl != NULL
			 && ((d_md != NULL && d_md->master && d_md->master->bl.id == bl->id) || (d_sd != NULL && d_sd->devotion[sce->val2] == bl->id))
			 && check_distance_bl(bl, d_bl, sce->val3)
			) {
				if (!rmdamage){
					clif->damage(d_bl, d_bl, 0, 0, damage, 0, BDT_NORMAL, 0);
					status_fix_damage(NULL, d_bl, damage, 0);
				} else { //Reflected magics are done directly on the target not on paladin
					//This check is only for magical skill.
					//For BF_WEAPON skills types track var rdamage and function battle_calc_return_damage
					clif->damage(bl, bl, 0, 0, damage, 0, BDT_NORMAL, 0);
					status_fix_damage(bl, bl, damage, 0);
				}
			} else {
				status_change_end(bl, SC_DEVOTION, INVALID_TIMER);
				if (!dmg.amotion)
					status_fix_damage(src, bl, damage, dmg.dmotion);
			}
		}
		if (sc->data[SC_WATER_SCREEN_OPTION]) {
			struct status_change_entry *sce = sc->data[SC_WATER_SCREEN_OPTION];
			struct block_list *e_bl = map->id2bl(sce->val1);

			if (e_bl) {
				if (!rmdamage) {
					clif->skill_damage(e_bl, e_bl, timer->gettick(), 0, 0, damage, dmg.div_, skill_id, -1, skill->get_hit(skill_id, skill_lv));
					status_fix_damage(NULL, e_bl, damage, 0);
				} else {
					clif->skill_damage(bl, bl, timer->gettick(), 0, 0, damage, dmg.div_, skill_id, -1, skill->get_hit(skill_id, skill_lv));
					status_fix_damage(bl, bl, damage, 0);
				}
			}
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

static void skill_attack_combo1_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, struct status_change_entry *sce, int *combo)
{
	if (src == dsrc) // Ground skills are exceptions. [Inkfish]
		status_change_end(src, SC_COMBOATTACK, INVALID_TIMER);
}

static void skill_attack_combo2_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, int *combo)
{
}

static void skill_attack_display_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, int *type, struct Damage *dmg, int64 *damage)
{
	nullpo_retv(bl);
	nullpo_retv(dmg);
	nullpo_retv(tick);
	nullpo_retv(flag);
	nullpo_retv(damage);
	nullpo_retv(skill_id);
	nullpo_retv(skill_lv);
	nullpo_retv(type);

	if (*flag & SD_ANIMATION && dmg->div_ < 2) //Disabling skill animation doesn't works on multi-hit.
		*type = BDT_SPLASH;
	if (bl->type == BL_SKILL) {
		struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
		if (su->group && skill->get_inf2(su->group->skill_id) & INF2_TRAP)  // show damage on trap targets
			clif->skill_damage(src, bl, *tick, dmg->amotion, dmg->dmotion, *damage, dmg->div_, *skill_id, (*flag & SD_LEVEL) ? -1 : *skill_lv, BDT_SPLASH);
	}
	dmg->dmotion = clif->skill_damage(dsrc, bl, *tick, dmg->amotion, dmg->dmotion, *damage, dmg->div_, *skill_id, (*flag & SD_LEVEL) ? -1 : *skill_lv, *type);
}

static int skill_attack_copy_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	nullpo_ret(skill_id);
	return *skill_id;
}

static int skill_attack_dir_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	return UNIT_DIR_UNDEFINED;
}

static void skill_attack_blow_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl,
                                      uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, int *type,
                                      struct Damage *dmg, int64 *damage, enum unit_dir *dir)
{
	nullpo_retv(bl);
	nullpo_retv(dmg);
	nullpo_retv(dir);
	nullpo_retv(damage);

	skill->blown(dsrc, bl, dmg->blewcount, *dir, 0x0);
	if (!dmg->blewcount && bl->type == BL_SKILL && *damage > 0){
		struct skill_unit *su = BL_UCAST(BL_SKILL, bl);
		if (su->group && su->group->skill_id == HT_BLASTMINE)
			skill->blown(src, bl, 3, UNIT_DIR_UNDEFINED, 0);
	}
}

static void skill_attack_post_unknown(int *attack_type, struct block_list *src, struct block_list *dsrc, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
}

/*==========================================
 * sub function for recursive skill call.
 * Checking bl battle flag and display damage
 * then call func with source,target,skill_id,skill_lv,tick,flag
 *------------------------------------------*/
static int skill_area_sub(struct block_list *bl, va_list ap)
{
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

static int skill_check_unit_range_sub(struct block_list *bl, va_list ap)
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
			FALLTHROUGH
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
		case RL_B_TRAP:
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

static int skill_check_unit_range(struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv)
{
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

static int skill_check_unit_range2_sub(struct block_list *bl, va_list ap)
{
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

static int skill_check_unit_range2(struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv)
{
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
static int skill_check_condition_mercenary(struct block_list *bl, int skill_id, int lv, int type)
{
	struct status_data *st;
	struct map_session_data *sd = NULL;
	int hp, sp, hp_rate, sp_rate, state;
	int idx;
	int itemid[MAX_SKILL_ITEM_REQUIRE], amount[MAX_SKILL_ITEM_REQUIRE];

	if( lv < 1 || lv > MAX_SKILL_LEVEL )
		return 0;
	nullpo_ret(bl);

	switch( bl->type ) {
		case BL_HOM: sd = BL_UCAST(BL_HOM, bl)->master; break;
		case BL_MER: sd = BL_UCAST(BL_MER, bl)->master; break;
		case BL_NUL:
		case BL_CHAT:
		case BL_ELEM:
		case BL_MOB:
		case BL_PET:
		case BL_ITEM:
		case BL_SKILL:
		case BL_NPC:
		case BL_PC:
		case BL_ALL:
			break;
	}

	st = status->get_status_data(bl);
	if( (idx = skill->get_index(skill_id)) == 0 )
		return 0;

	// Requirements
	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		itemid[i] = skill->get_itemid(skill_id, i);
		amount[i] = skill->get_itemqty(skill_id, i, lv);
	}
	hp = skill->dbs->db[idx].hp[lv-1];
	sp = skill->dbs->db[idx].sp[lv-1];
	hp_rate = skill->dbs->db[idx].hp_rate[lv-1];
	sp_rate = skill->dbs->db[idx].sp_rate[lv-1];
	state = skill->dbs->db[idx].state[lv - 1];

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
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0, 0);
			return 0;
		}
		if( sp > 0 && st->sp <= (unsigned int)sp ) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT, 0, 0);
			return 0;
		}

		int mhp = skill->get_mhp(skill_id, lv);

		if (mhp > 0 && get_percentage(st->hp, st->max_hp) > mhp) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0, 0);
			return 0;
		}

		int msp = skill->get_msp(skill_id, lv);

		if (msp > 0 && get_percentage(st->sp, st->max_sp) > msp) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT, 0, 0);
			return 0;
		}
	}

	if( !type )
		switch( state ) {
			case ST_MOVE_ENABLE:
				if( !unit->can_move(bl) ) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 0;
				}
				break;
		}
	if( !(type&1) )
		return 1;

	bool items_required = skill->items_required(sd, skill_id, lv);

	if (items_required && skill->check_condition_required_items(sd, skill_id, lv) != 0)
		return 0;

	int any_item_index = INDEX_NOT_FOUND;

	if (items_required)
		any_item_index = skill->get_any_item_index(sd, skill_id, lv);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE && items_required; i++) {
		if (itemid[i] == 0)
			continue;

		if (any_item_index != INDEX_NOT_FOUND && any_item_index != i)
			continue;

		int inventory_index = pc->search_inventory(sd, itemid[i]);

		if (inventory_index != INDEX_NOT_FOUND)
			pc->delitem(sd, inventory_index, amount[i], 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);
	}

	if (skill->check_condition_required_equip(sd, skill_id, lv) != 0)
		return 0;

	if( type&2 )
		return 1;

	if( sp || hp )
		status_zap(bl, hp, sp);

	return 1;
}

/*==========================================
 * what the hell it doesn't need to receive this many params, it doesn't do anything ~_~
 *------------------------------------------*/
static int skill_area_sub_count(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
	switch (skill_id) {
		case RL_QD_SHOT:
		{
			if (src->type == BL_PC && BL_CAST(BL_PC, src)) {
				struct unit_data* ud = unit->bl2ud(src);
				if (ud && ud->target == target->id)
					return 1;
			}
		}
	}
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_timerskill(int tid, int64 tick, int id, intptr_t data)
{
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
						map->search_free_cell(src, 0, &x, &y, 1, 1, SFC_DEFAULT);
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
					skill->toggle_magicpower(src, skl->skill_id, skl->skill_lv); // only the first hit will be amplify
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
					skill->toggle_magicpower(src, skl->skill_id, skl->skill_lv); // only the first hit will be amplify

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
					skill->toggle_magicpower(src, skl->skill_id, skl->skill_lv); // only the first hit will be amplify
					if( skl->type == 4 ){
						const enum sc_type scs[] = { SC_BURNING, SC_BLOODING, SC_FROSTMISTY, SC_STUN }; // status inflicts are depend on what summoned element is used.
						int rate = skl->y, index = skl->x-1;
						sc_start2(src,target, scs[index], rate, skl->skill_lv, src->id, skill->get_time(WL_TETRAVORTEX,index+1));
					}
					break;
				case WM_REVERBERATION_MELEE:
				case WM_REVERBERATION_MAGIC:
					skill->attack(skill->get_type(skl->skill_id, skl->skill_lv), src, src, target, skl->skill_id, skl->skill_lv, 0, SD_LEVEL);
					break;
				case SC_FATALMENACE:
					if( src == target ) // Casters Part
						unit->warp(src, -1, skl->x, skl->y, CLR_TELEPORT);
					else { // Target's Part
						short x = skl->x, y = skl->y;
						map->search_free_cell(NULL, target->m, &x, &y, 2, 2, SFC_XY_CENTER);
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
					FALLTHROUGH
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
				case SU_CN_METEOR:
					if (skl->type >= 0) {
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
					FALLTHROUGH
				// fall through ...
				case WL_EARTHSTRAIN:
					skill->unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,(skl->type<<16)|skl->flag);
					break;
				case LG_OVERBRAND_BRANDISH:
					skill->area_temp[1] = 0;
					map->foreachinpath(skill->attack_area,src->m,src->x,src->y,skl->x,skl->y,4,2,BL_CHAR,
						skill->get_type(skl->skill_id, skl->skill_lv), src, src, skl->skill_id, skl->skill_lv, tick, skl->flag, BCT_ENEMY);
					break;
				case RL_FIRE_RAIN:
					{
						int dummy = 1;
						int i = skill->get_splash(skl->skill_id, skl->skill_lv);

						if (rnd() % 100 < (15 + 5 * skl->skill_lv))
							map->foreachinarea(skill->cell_overlap, src->m, skl->x - i, skl->y - i, skl->x + i, skl->y + i, BL_SKILL, skl->skill_id, &dummy, src);
						skill->unitsetting(src, skl->skill_id, skl->skill_lv, skl->x, skl->y, 0);
					}
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

static bool skill_timerskill_dead_unknown(struct block_list *src, struct unit_data *ud, struct skill_timerskill *skl)
{
	return false;
}

static void skill_timerskill_target_unknown(int tid, int64 tick, struct block_list *src, struct block_list *target, struct unit_data *ud, struct skill_timerskill *skl)
{
	nullpo_retv(skl);
	skill->attack(skl->type, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
}

static void skill_timerskill_notarget_unknown(int tid, int64 tick, struct block_list *src, struct unit_data *ud, struct skill_timerskill *skl)
{
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_addtimerskill(struct block_list *src, int64 tick, int target, int x, int y, uint16 skill_id, uint16 skill_lv, int type, int flag)
{
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
static int skill_cleartimerskill(struct block_list *src)
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

static bool skill_cleartimerskill_exception(int skill_id)
{
	return false;
}

static int skill_activate_reverberation(struct block_list *bl, va_list ap)
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
		skill->trap_do_splash(bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
		su->limit = DIFF_TICK32(tick,sg->tick)+1500;
		sg->unit_id = UNT_USED_TRAPS;
	}
	return 0;
}

static int skill_reveal_trap(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;

	nullpo_ret(bl);
	Assert_ret(bl->type == BL_SKILL);
	su = BL_UCAST(BL_SKILL, bl);

	if (su->alive && su->group && skill->get_inf2(su->group->skill_id) & INF2_HIDDEN_TRAP) { //Reveal trap.
		su->visible = true;
		clif->skillunit_update(bl);
		return 1;
	}
	return 0;
}

static void skill_castend_type(enum cast_enum type, struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
	switch (type) {
		case CAST_GROUND:
			nullpo_retv(bl);
			skill->castend_pos2(src, bl->x, bl->y, skill_id, skill_lv, tick, flag);
			break;
		case CAST_NODAMAGE:
			skill->castend_nodamage_id(src, bl, skill_id, skill_lv, tick, flag);
			break;
		case CAST_DAMAGE:
			skill->castend_damage_id(src, bl, skill_id, skill_lv, tick, flag);
			break;
	}
}

/*==========================================
 *
 *
 *------------------------------------------*/
static int skill_castend_damage_id(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
	GUARD_MAP_LOCK

	struct map_session_data *sd = NULL;
	struct status_data *tstatus;
	struct status_change *sc, *tsc;

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

	if (skill_id != 0 && skill->get_type(skill_id, skill_lv) == BF_MAGIC && status->isimmune(bl) == 100) {
		//GTB makes all targeted magic display miss with a single bolt.
		sc_type sct = skill->get_sc_type(skill_id);
		if(sct != SC_NONE)
			status_change_end(bl, sct, INVALID_TIMER);
		clif->skill_damage(src, bl, tick, status_get_amotion(src), status_get_dmotion(bl), 0, 1, skill_id, skill_lv, skill->get_hit(skill_id, skill_lv));
		return 1;
	}

	sc = status->get_sc(src);
	tsc = status->get_sc(bl);
	if (sc && !sc->count)
		sc = NULL; //Unneeded
	if (tsc && !tsc->count)
		tsc = NULL;

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
		case RL_MASS_SPIRAL:
		case RL_BANISHING_BUSTER:
		case RL_AM_BLAST:
		case RL_SLUGSHOT:
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
				                    skill->get_splash(skill_id, skill_lv),skill->splash_target(src),
				                    src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
				                    skill->castend_damage_id);
			} else
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case TK_STORMKICK: // Taekwon kicks [Dralnu]
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			skill->area_temp[1] = 0;
			map->foreachinrange(skill->attack_area, src,
			                    skill->get_splash(skill_id, skill_lv), skill->splash_target(src),
			                    BF_WEAPON, src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
			break;

		case KN_CHARGEATK: {
				bool path_exists = path->search_long(NULL, src, src->m, src->x, src->y, bl->x, bl->y,CELL_CHKWALL);
				unsigned int dist = distance_bl(src, bl);
				enum unit_dir dir = map->calc_dir(bl, src->x, src->y);

				// teleport to target (if not on WoE grounds)
				if (!map_flag_gvg2(src->m) && map->list[src->m].flag.battleground == 0 && unit->move_pos(src, bl->x, bl->y, 0, true) == 0)
					clif->slide(src, bl->x, bl->y);

				// cause damage and knockback if the path to target was a straight one
				if( path_exists ) {
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, dist);
					skill->blown(src, bl, dist, dir, 0);
					//HACK: since knockback officially defaults to the left, the client also turns to the left... therefore,
					// make the caster look in the direction of the target
					unit->set_dir(src, unit_get_opposite_dir(dir));
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
			                   skill->get_splash(skill_id, skill_lv),skill->get_maxcount(skill_id,skill_lv), skill->splash_target(src),
			                   skill->get_type(skill_id, skill_lv), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
			break;

		case NPC_ACIDBREATH:
		case NPC_DARKNESSBREATH:
		case NPC_FIREBREATH:
		case NPC_ICEBREATH:
		case NPC_THUNDERBREATH:
			skill->area_temp[1] = bl->id;
			map->foreachinpath(skill->attack_area,src->m,src->x,src->y,bl->x,bl->y,
			                   skill->get_splash(skill_id, skill_lv),skill->get_maxcount(skill_id,skill_lv), skill->splash_target(src),
			                   skill->get_type(skill_id, skill_lv), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
			break;

		case MO_INVESTIGATE:
			skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
			break;

		case RG_BACKSTAP:
			{
				enum unit_dir dir = map->calc_dir(src, bl->x, bl->y);
				enum unit_dir t_dir = unit->getdir(bl);
				if ((!check_distance_bl(src, bl, 0) && map->check_dir(dir, t_dir) == 0) || bl->type == BL_SKILL) {
					status_change_end(src, SC_HIDING, INVALID_TIMER);
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
					dir = unit_get_opposite_dir(dir); // change direction [Celest]
					unit->set_dir(bl, dir);
				}
				else if (sd)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);

				if( skill_id == MO_EXTREMITYFIST ) {
					mbl = src;
					i = 3; // for Asura(from caster)
					status->set_sp(src, 0, STATUS_HEAL_DEFAULT);
					status_change_end(src, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
					status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
#ifdef RENEWAL
					sc_start(src, src,SC_EXTREMITYFIST2,100,skill_lv,skill->get_time(skill_id,skill_lv));
#endif // RENEWAL
				} else {
					status_change_end(src, SC_NJ_NEN, INVALID_TIMER);
					status_change_end(src, SC_HIDING, INVALID_TIMER);
#ifdef RENEWAL
					status->set_hp(src, max(status_get_max_hp(src)/100, 1), STATUS_HEAL_DEFAULT);
#else // not RENEWAL
					status->set_hp(src, 1, STATUS_HEAL_DEFAULT);
#endif // RENEWAL
				}
				enum unit_dir dir = map->calc_dir(src, bl->x, bl->y);
				if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
					map->freeblock_unlock(); // unblock before assert-returning
					return 0;
				}
				x = i * dirx[dir];
				y = i * diry[dir];
				if ((mbl == src || (!map_flag_gvg2(src->m) && !map->list[src->m].flag.battleground))) { // only NJ_ISSEN don't have slide effect in GVG
					if (unit->move_pos(src, mbl->x + x, mbl->y + y, 1, true) != 0) {
						// The cell is not reachable (wall, object, ...), move next to the target
						if (x > 0) x = -1;
						else if (x < 0) x = 1;
						if (y > 0) y = -1;
						else if (y < 0) y = 1;

						unit->move_pos(src, bl->x + x, bl->y + y, 1, true);
					}
					clif->slide(src, src->x, src->y);
					clif->fixpos(src);
					clif->spiritball(src, BALL_TYPE_SPIRIT, AREA);
				}
			}
			break;

		case HT_POWER:
			if( tstatus->race == RC_BRUTE || tstatus->race == RC_INSECT )
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			break;

		case SU_PICKYPECK:
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			FALLTHROUGH
		case SU_BITE:
			skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			if (status->get_lv(src) >= 30 && (rnd() % 100 < (int)(status->get_lv(src) / 30) * 10 + 10))
				skill->addtimerskill(src, tick + skill->get_delay(skill_id, skill_lv), bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
			break;

		// Splash attack skills.
		case AS_GRIMTOOTH:
		case MC_CARTREVOLUTION:
		case NPC_SPLASHATTACK:
			flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit
			FALLTHROUGH
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
		case SU_SCRATCH:
		case SU_LUNATICCARROTBEAT:
		case RL_FIREDANCE:
		case RL_S_STORM:
		case RL_R_TRIP:
		case SJ_PROMINENCEKICK:
		case SJ_NEWMOONKICK:
		case SJ_SOLARBURST:
		case SJ_FULLMOONKICK:
		case SJ_FALLINGSTAR_ATK2:
		case SJ_STAREMPEROR:
		case SP_CURSEEXPLOSION:
		case SP_SHA:
		case SP_SWHOO:
			if (flag&1) { //Recursive invocation
				// skill->area_temp[0] holds number of targets in area
				// skill->area_temp[1] holds the id of the original target
				// skill->area_temp[2] counts how many targets have already been processed
				int sflag = skill->area_temp[0] & 0xFFF, heal;
				if( flag&SD_LEVEL )
					sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
				if( (skill->area_temp[1] != bl->id && !(skill->get_inf2(skill_id)&INF2_NPC_SKILL)) || flag&SD_ANIMATION )
					sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

				if ( tsc && tsc->data[SC_HOVERING] && ( skill_id == SR_WINDMILL || skill_id == LG_MOONSLASHER ) )
					break;

				if ((skill_id == SP_SHA || skill_id == SP_SWHOO) && bl->type != BL_MOB)
					break;

				heal = skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, sflag);
				if (skill_id == NPC_VAMPIRE_GIFT && heal > 0) {
					clif->skill_nodamage(NULL, src, AL_HEAL, heal, 1);
					status->heal(src, heal, 0, STATUS_HEAL_DEFAULT);
				}
				if (skill_id == SU_SCRATCH && status->get_lv(src) >= 30 && (rnd() % 100 < (int)(status->get_lv(src) / 30) + 10)) // TODO: Need activation chance.
					skill->addtimerskill(src, tick + skill->get_delay(skill_id, skill_lv), bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
				if (skill_id == SJ_PROMINENCEKICK)
					skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, sflag | 8 | SD_ANIMATION);
			} else {
				switch ( skill_id ) {
					case NJ_BAKUENRYU:
					case LG_EARTHDRIVE:
					case GN_CARTCANNON:
					case SU_SCRATCH:
					case SU_LUNATICCARROTBEAT:
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
				
				if (sd != NULL && (skill_id == SP_SHA || skill_id == SP_SWHOO) && bl->type != BL_MOB) {
					status->change_start(src, bl, SC_STUN, 10000, skill_lv, 0, 0, 0, 500, 10);
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
					break;
				}

				if (skill_id == SP_SWHOO)
					status_change_end(src, SC_USE_SKILL_SP_SPA, INVALID_TIMER);

				if (skill_id == WL_CRIMSONROCK) {
					skill->area_temp[4] = bl->x;
					skill->area_temp[5] = bl->y;
				}
				if (skill_id == SU_LUNATICCARROTBEAT) {
					skill->area_temp[3] = 0;
				}

				if (skill_id == NC_VULCANARM) {
					if (sd != NULL) {
						pc->overheat(sd,1);
					}
				}

				// if skill damage should be split among targets, count them
				//SD_LEVEL -> Forced splash damage for Auto Blitz-Beat -> count targets
				//special case: Venom Splasher uses a different range for searching than for splashing
				if( flag&SD_LEVEL || skill->get_nk(skill_id)&NK_SPLASHSPLIT )
					skill->area_temp[0] = map->foreachinrange(skill->area_sub, bl, (skill_id == AS_SPLASHER)?1:skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill->area_sub_count);

				// recursive invocation of skill->castend_damage_id() with flag|1
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);

				if (skill_id == AS_SPLASHER) {
					// Prevent double item consumption when the target explodes (item requirements have already been processed in skill_castend_nodamage_id)
					flag |= 1;
				}

				if (sd && skill_id == SU_LUNATICCARROTBEAT) {
					short item_idx = pc->search_inventory(sd, ITEMID_CARROT);

					if (item_idx >= 0) {
						pc->delitem(sd, item_idx, 1, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME);
						skill->area_temp[3] = 1;
					}
				}
			}
			break;

		case SM_MAGNUM:
		case MS_MAGNUM:
			if( flag&1 ) {
				//Damage depends on distance, so add it to flag if it is > 1
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION|distance_bl(src, bl));
			}
			break;

		case KN_BRANDISHSPEAR:
		case ML_BRANDISH:
			//Coded apart for it needs the flag passed to the damage calculation.
			if (skill->area_temp[1] != bl->id)
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
			else
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
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
					dir = rnd() % UNIT_DIR_MAX;
				} else {
					// Create an empty list of already hit targets
					db_clear(skill->bowling_db);
					// Direction is walkpath
					dir = unit_get_opposite_dir(unit->getdir(src));
				}
				// Add current target to the list of already hit targets
				idb_put(skill->bowling_db, bl->id, bl);
				// Keep moving target in direction square by square
				tx = bl->x;
				ty = bl->y;
				for(i=0;i<c;i++) {
					// Target coordinates (get changed even if knockback fails)
					if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
						map->freeblock_unlock(); // unblock before assert-returning
						return 0;
					}
					tx -= dirx[dir];
					ty -= diry[dir];
					// If target cell is a wall then break
					if(map->getcell(bl->m, bl, tx, ty, CELL_CHKWALL))
						break;
					skill->blown(src,bl,1,dir,0);
					// Splash around target cell, but only cells inside area; we first have to check the area is not negative
					if((max(min_x,tx-1) <= min(max_x,tx+1)) &&
						(max(min_y,ty-1) <= min(max_y,ty+1)) &&
						(map->foreachinarea(skill->area_sub, bl->m, max(min_x,tx-1), max(min_y,ty-1), min(max_x,tx+1), min(max_y,ty+1), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY, skill->area_sub_count))) {
						// Recursive call
						map->foreachinarea(skill->area_sub, bl->m, max(min_x,tx-1), max(min_y,ty-1), min(max_x,tx+1), min(max_y,ty+1), skill->splash_target(src), src, skill_id, skill_lv, tick, (flag|BCT_ENEMY)+1, skill->castend_damage_id);
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
					skill->blown(src, bl, skill->area_temp[2], UNIT_DIR_UNDEFINED, 0);
			} else {
				int x = bl->x;
				int y = bl->y;
				int i;
				enum unit_dir dir = map->calc_dir(bl, src->x, src->y);
				skill->area_temp[1] = bl->id;
				skill->area_temp[2] = skill->get_blewcount(skill_id,skill_lv);
				// all the enemies between the caster and the target are hit, as well as the target
				if (skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,0))
					skill->blown(src, bl, skill->area_temp[2], UNIT_DIR_UNDEFINED, 0);
				for (i=0;i<4;i++) {
					map->foreachincell(skill->area_sub,bl->m,x,y,BL_CHAR,src,skill_id,skill_lv,
					                   tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
					if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
						map->freeblock_unlock(); // unblock before assert-returning
						return 0;
					}
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

		case AL_HOLYLIGHT:
			status_change_end(bl, SC_PLATINUM_ALTER, INVALID_TIMER);
			FALLTHROUGH
		case MG_SOULSTRIKE:
		case NPC_DARKSTRIKE:
		case MG_COLDBOLT:
		case MG_FIREBOLT:
		case MG_LIGHTNINGBOLT:
		case WZ_EARTHSPIKE:
		case AL_HEAL:
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
			sc_start(src, src,skill->get_sc_type(skill_id),100,skill_lv,skill->get_time(skill_id,skill_lv));
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
		case SP_SPA:
			if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
				status->change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCFLAG_FIXEDTICK|SCFLAG_FIXEDRATE);
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
		case RL_B_TRAP:
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

			if (tsc != NULL && tsc->data[SC_HIDING]) {
				clif->skill_nodamage(src,src,skill_id,skill_lv,1);
			} else
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		}
			break;
		case NPC_SELFDESTRUCTION: {
			if (tsc != NULL && tsc->data[SC_HIDING])
				break;
			}
			FALLTHROUGH
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
				status->heal(src, heal, 0, STATUS_HEAL_DEFAULT);
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
				map->search_free_cell(bl, 0, &x, &y, 1, 1, SFC_DEFAULT);
				if (unit->move_pos(src, x, y, 0, false) == 0)
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
				skill->blown(src, bl, 6, UNIT_DIR_UNDEFINED, 0);
				skill->addtimerskill(src,tick+800,bl->id,0,0,skill_id,skill_lv,BF_WEAPON,flag);
				skill->castend_damage_id(src,bl,KN_SPEARBOOMERANG,1,tick,0);
			}
			break;
		case RK_PHANTOMTHRUST:
		{
			struct map_session_data *tsd = BL_CAST(BL_PC, bl);
			unit->set_dir(src, map->calc_dir(src, bl->x, bl->y));
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
				enum unit_dir dir = map->calc_dir(bl, src->x, src->y);
				if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
					map->freeblock_unlock(); // unblock before assert-returning
					return 0;
				}
				short x = bl->x + dirx[dir];
				short y = bl->y + diry[dir];

				if (unit->move_pos(src, x, y, 1, true) == 0) {
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_GC_WEAPONBLOCKING, 0, 0);
			break;

		case GC_CROSSRIPPERSLASHER:
			if( sd && !(sc && sc->data[SC_ROLLINGCUTTER]) )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0, 0);
			else
			{
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
				status_change_end(src,SC_ROLLINGCUTTER,INVALID_TIMER);
			}
			break;

		case GC_PHANTOMMENACE:
			if( flag&1 ) {
				// Only Hits Invisible Targets
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
			int heal = skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			int rate = 70 + 5 * skill_lv;

			heal = heal * (5 + 5 * skill_lv) / 100;

			if( bl->type == BL_SKILL || status_get_hp(src) == status_get_max_hp(src)) // Don't absorb when caster was in full HP
				heal = 0; // Don't absorb heal from Ice Walls or other skill units.

			if( heal && rnd()%100 < rate ) {
				status->heal(src, heal, 0, STATUS_HEAL_DEFAULT);
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
				skill->toggle_magicpower(src, skill_id, skill_lv);
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

					skill->castend_type(skill->get_casttype(spell_skill_id), src, bl, spell_skill_id, spell_skill_lv, tick, 0);
					sd->ud.canact_tick = tick + skill->delay_fix(src, spell_skill_id, spell_skill_lv);
					clif->status_change(src, status->get_sc_icon(SC_POSTDELAY), status->get_sc_relevant_bl_types(SC_POSTDELAY), 1, skill->delay_fix(src, spell_skill_id, spell_skill_lv), 0, 0, 0);

					cooldown = pc->get_skill_cooldown(sd, spell_skill_id, spell_skill_lv);
					if (cooldown != 0)
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
			if (sd != NULL && pc_isridingwug(sd)) {
				enum unit_dir dir = map->calc_dir(bl, src->x, src->y);
				if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
					map->freeblock_unlock(); // unblock before assert-returning
					return 0;
				}
				short x = bl->x + dirx[dir];
				short y = bl->y + diry[dir];
				if (unit->move_pos(src, x, y, 1, true) == 0) {
					clif->slide(src, x, y);
					clif->fixpos(src);
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
				}
				break;
			}
			FALLTHROUGH
		case RA_WUGBITE:
			if( path->search(NULL,src,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKNOREACH) ) {
				skill->attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			}else if( sd && skill_id == RA_WUGBITE ) // Only RA_WUGBITE has the skill fail message.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);

			break;

		case RA_SENSITIVEKEEN:
			if( bl->type != BL_SKILL ) { // Only Hits Invisible Targets
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
						item_tmp.nameid = sg->item_id ? sg->item_id : ITEMID_BOOBY_TRAP;
						item_tmp.identify = 1;
						if( item_tmp.nameid )
							map->addflooritem(bl, &item_tmp, 1, bl->m, bl->x, bl->y, 0, 0, 0, 0, false);
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
				status_change_end(bl, SC_NEWMOON, INVALID_TIMER);
			} else {
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);
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
				map->search_free_cell(src, 0, &x, &y, -1, -1, SFC_DEFAULT);
				// Destination area
				skill->area_temp[4] = x;
				skill->area_temp[5] = y;
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_damage_id);
				skill->addtimerskill(src,tick + 800,src->id,x,y,skill_id,skill_lv,0,flag); // To teleport Self
				clif->skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,BDT_SKILL);
			}
			break;
		case LG_PINPOINTATTACK:
			if (!map_flag_gvg2(src->m) && map->list[src->m].flag.battleground == 0 && unit->move_pos(src, bl->x, bl->y, 1, true) == 0)
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
				if (!map_flag_gvg2(src->m) && map->list[src->m].flag.battleground == 0 && unit->move_pos(src, bl->x, bl->y, 1, true) == 0) {
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
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_damage_id);
				clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			}
			break;

		case WM_SOUND_OF_DESTRUCTION:
			{
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
			if (tsc != NULL && tsc->data[SC_POISON]) {
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
				status_change_end(bl, SC_POISON, INVALID_TIMER);
			} else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				skill->attack(skill->get_type(skill_id + 1, skill_lv), src, src, bl, skill_id + 1, skill_lv, tick, flag);
			else {
				int i = skill->get_splash(skill_id,skill_lv);
				clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
				clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( rnd()%100 < 30 )
					map->foreachinrange(skill->area_sub,bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
				else
					skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
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
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			else {
				int i = skill->get_splash(skill_id,skill_lv);
				clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
				clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( rnd()%100 < 30 )
					map->foreachinrange(skill->area_sub,bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
				else
					skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			}
			break;

		case EL_FIRE_ARROW:
		case EL_ICE_NEEDLE:
		case EL_WIND_SLASH:
		case EL_STONE_HAMMER:
			clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
			clif->skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			break;

		case EL_TIDAL_WEAPON:
			if( src->type == BL_ELEM ) {
				struct elemental_data *ele = BL_CAST(BL_ELEM,src);
				struct status_change *esc = status->get_sc(&ele->bl);
				sc_type type = skill->get_sc_type(skill_id), type2;
				type2 = type-1;

				clif->skill_nodamage(src,battle->get_master(src),skill_id,skill_lv,1);
				clif->skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
				if( (esc && esc->data[type2]) || (tsc && tsc->data[type]) ) {
					elemental->clean_single_effect(ele, skill_id);
				}
				if( rnd()%100 < 50 )
					skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
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
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			else {
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill->castend_damage_id);
			}
			break;

		case MH_STAHL_HORN:
		case MH_NEEDLE_OF_PARALYZE:
			skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			break;
		case MH_TINDER_BREAKER:
			if (unit->move_pos(src, bl->x, bl->y, 1, true) == 0) {
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

		case SU_SV_STEMSPEAR:
			skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			if (status->get_lv(src) >= 30 && (rnd() % 100 < (int)(status->get_lv(src) / 30) + 10)) // TODO: Need activation chance.
				skill->addtimerskill(src, tick + skill->get_delay(skill_id, skill_lv), bl->id, 0, 0, skill_id, skill_lv, (skill_id == SU_SV_STEMSPEAR) ? BF_MAGIC : BF_WEAPON, flag);
			break;
		case SU_SCAROFTAROU:
			sc_start(src, bl, skill->get_sc_type(skill_id), 10, skill_lv, skill->get_time(skill_id, skill_lv)); // TODO: What's the activation chance for the effect?
			break;
		case RL_H_MINE:
			if (!(flag & 1)) {
				if (sd == NULL || sd->flicker) {
					if (skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag))
						status->change_start(src, bl, SC_HOWLING_MINE, 10000, skill_id, 0, 0, 0, skill->get_time(skill_id, skill_lv), SCFLAG_NOAVOID | SCFLAG_FIXEDTICK | SCFLAG_FIXEDRATE);
					break;
				}
				if (sd != NULL && sd->flicker && tsc != NULL && tsc->data[SC_HOWLING_MINE] != NULL && tsc->data[SC_HOWLING_MINE]->val2 == src->id) {
					map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), BL_CHAR | BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1, skill->castend_damage_id);
					flag |= 1; // Don't consume requirement
					tsc->data[SC_HOWLING_MINE]->val3 = 1; // Mark the SC end because not expired
					status_change_end(bl, SC_HOWLING_MINE, INVALID_TIMER);
					sc_start4(src, bl, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill->get_time2(skill_id, skill_lv));
				}
			} else {
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag);
			}
			if (sd != NULL && sd->flicker)
				flag |= 1; // Don't consume requirement
			break;
		case RL_QD_SHOT:
			if (skill->area_temp[1] == bl->id)
				break;
			if ((flag & 1) && tsc != NULL && tsc->data[SC_CRIMSON_MARKER] != NULL)
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag | SD_ANIMATION);
			break;
		case RL_D_TAIL:
		case RL_HAMMER_OF_GOD:
			if (flag & 1) {
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag | SD_ANIMATION);
			} else {
				if (sd != NULL && tsc != NULL && tsc->data[SC_CRIMSON_MARKER] != NULL) {
					int i;

					ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == bl->id);
					if (i < MAX_SKILL_CRIMSON_MARKER)
						flag |= 8;
				}

				if (skill_id == RL_HAMMER_OF_GOD)
					clif->skill_poseffect(src, skill_id, 1, bl->x, bl->y, timer->gettick());
				else
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);

				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill->castend_damage_id);
			}
			break;
		case SJ_FLASHKICK:
		{
			struct map_session_data *tsd = BL_CAST(BL_PC, bl);
			struct mob_data *md = BL_CAST(BL_MOB, src);
			struct mob_data *tmd = BL_CAST(BL_MOB, bl);

			// Only players and monsters can be tagged....I think??? [Rytech]
			// Lets only allow players and monsters to use this skill for safety reasons.
			if ((tsd == NULL && tmd == NULL) || (sd == NULL && md == NULL)) {
				if (sd != NULL)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
				break;
			}

			// Check if the target is already tagged by another source.
			if ((tsd != NULL && tsd->sc.data[SC_FLASHKICK] != NULL && tsd->sc.data[SC_FLASHKICK]->val1 != src->id) ||
				(tmd != NULL && tmd->sc.data[SC_FLASHKICK] != NULL && tmd->sc.data[SC_FLASHKICK]->val1 != src->id)
			) { // Same as the above check, but for monsters.
				// Can't tag a player that was already tagged from another source.
				if (sd != NULL) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
					map->freeblock_unlock();
					return 1;
				}
			}
	
			if (sd != NULL) { // Tagging the target.
				int i;
				ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == bl->id);
				if (i == MAX_STELLAR_MARKS) {
					ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == 0);
					if (i == MAX_STELLAR_MARKS) { // Max number of targets tagged. Fail the skill.
						clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
						map->freeblock_unlock();
						return 1;
					}
				}

				// Tag the target only if damage was done. If it deals no damage, it counts as a miss and won't tag.
				// Note: Not sure if it works like this in official but you can't mark on something you can't
				// hit, right? For now well just use this logic until we can get a confirm on if it does this or not. [Rytech]
				if (skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag) > 0) { // Add the ID of the tagged target to the player's tag list and start the status on the target.
					sd->stellar_mark[i] = bl->id;
					// Val4 flags if the status was applied by a player or a monster.
					// This will be important for other skills that work together with this one.
					// 1 = Player, 2 = Monster.
					// Note: Because the attacker's ID and the slot number is handled here, we have to
					// apply the status here. We can't pass this data to skill_additional_effect.
					sc_start4(src, bl, SC_FLASHKICK, 100, src->id, i, skill_lv, 1, skill->get_time(skill_id, skill_lv));
				}
			} else if (md != NULL) { // Monsters can't track with this skill. Just give the status.
				if (skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag) > 0)
					sc_start4(src, bl, SC_FLASHKICK, 100, 0, 0, skill_lv, 2, skill->get_time(skill_id, skill_lv));
			}
		}
			break;
		case SJ_FALLINGSTAR_ATK:
			if (sd != NULL) { // If a player used the skill it will search for targets marked by that player. 
				if (tsc != NULL && tsc->data[SC_FLASHKICK] != NULL && tsc->data[SC_FLASHKICK]->val4 == 1) { // Mark placed by a player.
					int i = 0;

					ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == bl->id);
					if (i < MAX_STELLAR_MARKS) {
						skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
						skill->castend_damage_id(src, bl, SJ_FALLINGSTAR_ATK2, skill_lv, tick, 0);
					}
				}
			} else if (tsc != NULL && tsc->data[SC_FLASHKICK] != NULL && tsc->data[SC_FLASHKICK]->val4 == 2) { // Mark placed by a monster.
				// If a monster used the skill it will search for targets marked by any monster since they can't track their own targets.
				skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
				skill->castend_damage_id(src, bl, SJ_FALLINGSTAR_ATK2, skill_lv, tick, 0);
			}
			break;
		case SJ_NOVAEXPLOSING:
		{
			skill->attack(BF_MISC, src, src, bl, skill_id, skill_lv, tick, flag);

			// We can end Dimension here since the cooldown code is processed before this point.
			if (sc != NULL && sc->data[SC_DIMENSION] != NULL)
				status_change_end(src, SC_DIMENSION, INVALID_TIMER);
			else // Dimension not active? Activate the 2 second skill block penalty.
				sc_start(src, &sd->bl, SC_NOVAEXPLOSING, 100, skill_lv, skill->get_time(skill_id, skill_lv));
		}
			break;
		case SP_SOULEXPLOSION:
			if (!((tsc != NULL &&
				(tsc->data[SC_SOULLINK] != NULL ||
				tsc->data[SC_SOULGOLEM] != NULL ||
				tsc->data[SC_SOULSHADOW] != NULL ||
				tsc->data[SC_SOULFALCON] != NULL ||
				tsc->data[SC_SOULFAIRY] != NULL))
				|| tstatus->hp > (10 * tstatus->max_hp / 100)
			)) {
				// Requires target to have a soul link or target to have more than 10% of MaxHP.
				// With this skill requiring a soul link or the target to have more than 10% of MaxHP
				// I wonder, if the cooldown still happens after it fails. Need a confirm. [Rytech] 
				if (sd != NULL)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				break;
			}
			skill->attack(BF_MISC, src, src, bl, skill_id, skill_lv, tick, flag);
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

static bool skill_castend_damage_id_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag, struct status_data *tstatus, struct status_change *sc)
{
	nullpo_retr(true, skill_id);
	nullpo_retr(true, skill_lv);
	nullpo_retr(true, tick);
	nullpo_retr(true, tstatus);
	ShowWarning("skill_castend_damage_id: Unknown skill used:%d\n", *skill_id);
	clif->skill_damage(src, bl, *tick, status_get_amotion(src), tstatus->dmotion,
		0, abs(skill->get_num(*skill_id, *skill_lv)),
		*skill_id, *skill_lv, skill->get_hit(*skill_id, *skill_lv));
	map->freeblock_unlock();
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_castend_id(int tid, int64 tick, int id, intptr_t data)
{
	GUARD_MAP_LOCK

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

		if (sd && ud->skilltimer != INVALID_TIMER && (pc->checkskill(sd, SA_FREECAST) > 0 || ud->skill_id == LG_EXEEDBREAK || (skill->get_inf2(ud->skill_id) & INF2_FREE_CAST_REDUCED) != 0))
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
		bool is_asura = (ud->skill_id == MO_EXTREMITYFIST);

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
				//Find a random spot to place the skill. [Skotlex]
				inf2 = skill->get_splash(ud->skill_id, ud->skill_lv);
				ud->skillx = target->x;
				ud->skilly = target->y;
				map->get_random_cell(target, target->m, &ud->skillx, &ud->skilly, 1, inf2);
				ud->skilltimer=tid;
				return skill->castend_pos(tid,tick,id,data);
			case PF_SPIDERWEB:
			case GN_WALLOFTHORN:
			case SU_CN_POWDERING:
			case SU_SV_ROOTTWIST:
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
			enum unit_dir dir = map->calc_dir(src, target->x, target->y);
			enum unit_dir t_dir = unit->getdir(target);
			if (check_distance_bl(src, target, 0) || map->check_dir(dir, t_dir) != 0) {
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
				clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_NEED_HELPER, 0, 0);
				break;
			}

			if (ud->skill_id >= SL_SKE && ud->skill_id <= SL_SKA && target->type == BL_MOB) {
				if (BL_UCCAST(BL_MOB, target)->class_ == MOBID_EMPELIUM)
					break;
			} else if (inf && battle->check_target(src, target, inf) <= 0) {
				if (sd) clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if (sd) clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

		if (sd == NULL || sd->auto_cast_current.skill_id != ud->skill_id || skill->get_delay(ud->skill_id, ud->skill_lv) != 0)
			ud->canact_tick = tick + skill->delay_fix(src, ud->skill_id, ud->skill_lv); // Tests show wings don't overwrite the delay but skill scrolls do. [Inkfish]
		if (sd != NULL) { // Cooldown application
			int cooldown = pc->get_skill_cooldown(sd, ud->skill_id, ud->skill_lv);
			if (cooldown != 0)
				skill->blockpc_start(sd, ud->skill_id, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif->status_change(src, status->get_sc_icon(SC_POSTDELAY), status->get_sc_relevant_bl_types(SC_POSTDELAY), 1, skill->delay_fix(src, ud->skill_id, ud->skill_lv), 0, 0, 0);
		if( sd )
		{
			switch( ud->skill_id )
			{
			case GS_DESPERADO:
			case RL_FIREDANCE:
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
		if (skill->get_state(ud->skill_id, ud->skill_lv) != ST_MOVE_ENABLE)
			unit->set_walkdelay(src, tick, battle_config.default_walk_delay+skill->get_walkdelay(ud->skill_id, ud->skill_lv), 1);

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %u, ID %d skill castend id [id =%d, lv=%d, target ID %d]\n",
				src->type, src->id, ud->skill_id, ud->skill_lv, target->id);

		map->freeblock_lock();

		// SC_MAGICPOWER needs to switch states before any damage is actually dealt
		skill->toggle_magicpower(src, ud->skill_id, ud->skill_lv);

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

		if (sd != NULL && ud->skill_id != SA_ABRACADABRA && ud->skill_id != WM_RANDOMIZESPELL
		    && ud->skill_id == sd->auto_cast_current.skill_id) { // they just set the data so leave it as it is.[Inkfish]
			pc->autocast_remove(sd, sd->auto_cast_current.type, ud->skill_id, ud->skill_lv);
		}

		if (ud->skilltimer == INVALID_TIMER) {
			if(md) md->skill_idx = -1;
			else ud->skill_id = 0; //mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skill_lv = ud->skilltarget = 0;
		}

		// Asura Strike caster doesn't look to their target in the end
		if (src->id != target->id && !is_asura)
			unit->set_dir(src, map->calc_dir(src, target->x, target->y));

		map->freeblock_unlock();
		return 1;
	} while(0);

	//Skill failed.
	if (ud->skill_id == MO_EXTREMITYFIST && sd && !(sc && sc->data[SC_FOGWALL])) {
		//When Asura fails... (except when it fails from Fog of Wall)
		//Consume SP/spheres
		skill->consume_requirement(sd,ud->skill_id, ud->skill_lv,1);
		status->set_sp(src, 0, STATUS_HEAL_DEFAULT);
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
			enum unit_dir dir = map->calc_dir(src, target->x, target->y);
			Assert_ret(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX);
			int dist = 3; // number of cells that asura caster will walk
			int x = dist * dirx[dir];
			int y = dist * diry[dir];

			if (unit->move_pos(src, src->x + x, src->y + y, 1, true) == 0) {
				//Display movement + animation.
				clif->slide(src, src->x, src->y);
				clif->spiritball(src, BALL_TYPE_SPIRIT, AREA);
			}
			// "Skill Failed" message was already shown when checking that target is invalid
			//clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
		}
	}

	if (sd == NULL || sd->auto_cast_current.skill_id != ud->skill_id || skill->get_delay(ud->skill_id, ud->skill_lv) != 0)
		ud->canact_tick = tick;
	//You can't place a skill failed packet here because it would be
	//sent in ALL cases, even cases where skill_check_condition fails
	//which would lead to double 'skill failed' messages u.u [Skotlex]
	if (sd != NULL && ud->skill_id == sd->auto_cast_current.skill_id)
		pc->autocast_remove(sd, sd->auto_cast_current.type, ud->skill_id, ud->skill_lv);
	else if(md)
		md->skill_idx = -1;

	ud->skill_id = 0;
	ud->skill_lv = 0;
	ud->skilltarget = 0;

	return 0;
}

static bool skill_castend_id_unknown(struct unit_data *ud, struct block_list *src, struct block_list *target)
{
	return false;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_castend_nodamage_id(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
	GUARD_MAP_LOCK

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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_TOTARGET, 0, 0);
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
	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch (skill_id) {
		case HLIF_HEAL: // [orn]
			if (bl->type != BL_HOM) {
				if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0) ;
				break ;
			}
			FALLTHROUGH
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if (src == bl && skill->get_unit_id(skill_id, skill_lv, 0) != 0)
					return skill->castend_pos2(src,bl->x,bl->y,skill_id,skill_lv,tick,0);
			}
			break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)

	type = skill->get_sc_type(skill_id);
	tsc = status->get_sc(bl);
	tsce = (tsc != NULL && type != SC_NONE) ? tsc->data[type] : NULL;

	if (src != bl && type > SC_NONE
	 && (element = skill->get_ele(skill_id, skill_lv)) > ELE_NEUTRAL
	 && skill->get_inf(skill_id) != INF_SUPPORT_SKILL
	 && battle->attr_fix(NULL, NULL, 100, element, tstatus->def_ele, tstatus->ele_lv) <= 0)
		return 1; //Skills that cause an status should be blocked if the target element blocks its element.

	map->freeblock_lock();
	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch(skill_id) {
		case HLIF_HEAL: // [orn]
		case AL_HEAL:
		/**
		 * Arch Bishop
		 **/
		case AB_HIGHNESSHEAL:
		/**
		 * Summoner
		 */
		case SU_TUNABELLY:
			{
				int heal = skill->calc_heal(src, bl, (skill_id == AB_HIGHNESSHEAL)?AL_HEAL:skill_id, (skill_id == AB_HIGHNESSHEAL)?10:skill_lv, true);
				int heal_get_jobexp;
				//Highness Heal: starts at 1.7 boost + 0.3 for each level
				if (skill_id == AB_HIGHNESSHEAL) {
					heal = heal * (17 + 3 * skill_lv) / 10;
				}
				if (status->isimmune(bl) || (dstmd != NULL && (dstmd->class_ == MOBID_EMPELIUM || mob_is_battleground(dstmd))))
					heal = 0;

				if (sd != NULL && dstsd != NULL && sd->status.partner_id == dstsd->status.char_id && (sd->job & MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.sex == 0)
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
				if (skill_id == AL_HEAL) {
					status_change_end(bl, SC_BITESCAR, INVALID_TIMER);
				}
				clif->skill_nodamage (src, bl, skill_id, heal, 1);
				if( tsc && tsc->data[SC_AKAITSUKI] && heal && skill_id != HLIF_HEAL )
					heal = ~heal + 1;
				heal_get_jobexp = status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);

				if(sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0){
					heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
					if (heal_get_jobexp <= 0)
						heal_get_jobexp = 1;
					pc->gainexp(sd, bl, 0, heal_get_jobexp, false);
				}
			}
			break;

		case PR_REDEMPTIO:
			if (sd && !(flag&1)) {
				if (sd->status.party_id == 0) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				skill->area_temp[0] = 0;
				party->foreachsamemap(skill->area_sub,
					sd,skill->get_splash(skill_id, skill_lv),
					src,skill_id,skill_lv,tick, flag|BCT_PARTY|1,
					skill->castend_nodamage_id);
				if (skill->area_temp[0] == 0) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				skill->area_temp[0] = 5 - skill->area_temp[0]; // The actual penalty...
				if (skill->area_temp[0] > 0 && !map->list[src->m].flag.noexppenalty) { //Apply penalty
					sd->status.base_exp -= min(sd->status.base_exp, pc->nextbaseexp(sd) * skill->area_temp[0] * 2/1000); //0.2% penalty per each.
					sd->status.job_exp -= min(sd->status.job_exp, pc->nextjobexp(sd) * skill->area_temp[0] * 2/1000);
					clif->updatestatus(sd,SP_BASEEXP);
					clif->updatestatus(sd,SP_JOBEXP);
				}
				status->set_hp(src, 1, STATUS_HEAL_DEFAULT);
				status->set_sp(src, 0, STATUS_HEAL_DEFAULT);
				break;
			} else if (status->isdead(bl) && flag&1) { //Revive
				skill->area_temp[0]++; //Count it in, then fall-through to the Resurrection code.
				skill_lv = 3; //Resurrection level 3 is used
			} else //Invalid target, skip resurrection.
				break;

		case ALL_RESURRECTION:
			if(sd && (map_flag_gvg2(bl->m) || map->list[bl->m].flag.battleground)) {
				//No reviving in WoE grounds!
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
							pc->gainexp(sd, bl, exp, jexp, false);
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

		case SP_SOULCURSE:
			if (flag&1) {
				sc_start(src, bl, type, 30 + 10 * skill_lv, skill_lv, skill->get_time(skill_id, skill_lv));
			} else {
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY| 1, skill->castend_nodamage_id);
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
					pc->autocast_clear(sd);
					sd->auto_cast_current.type = AUTOCAST_ABRA;
					sd->auto_cast_current.skill_id = abra_skill_id;
					sd->auto_cast_current.skill_lv = abra_skill_lv;
					VECTOR_ENSURE(sd->auto_cast, 1, 1);
					VECTOR_PUSH(sd->auto_cast, sd->auto_cast_current);
					clif->item_skill(sd, abra_skill_id, abra_skill_lv);
					pc->autocast_clear_current(sd);
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
						else {
							switch (src->type) {
							case BL_MOB: target_id = BL_UCAST(BL_MOB, src)->target_id; break;
							case BL_PET: target_id = BL_UCAST(BL_PET, src)->target_id; break;
							case BL_NUL:
							case BL_CHAT:
							case BL_HOM:
							case BL_MER:
							case BL_ELEM:
							case BL_ALL:
								break;
							}
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
			if (sd != NULL)
				mob->once_spawn(sd, src->m, src->x, src->y, DEFAULT_MOB_JNAME, -1, 1, "", SZ_SMALL, AI_NONE);
			break;
		case SA_LEVELUP:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			if (sd && pc->nextbaseexp(sd)) pc->gainexp(sd, NULL, pc->nextbaseexp(sd) * 10 / 100, 0, false);
			break;
		case SA_INSTANTDEATH:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status->set_hp(bl, 1, STATUS_HEAL_DEFAULT);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				class_ = skill_id == SA_MONOCELL ? MOBID_PORING : mob->get_random_id(MOBG_CLASS_CHANGE, 1, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if ((dstsd->job & MAPID_UPPERMASK) == MAPID_CRUSADER) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

				if (sd != NULL && dstsd != NULL && (dstsd->job & MAPID_UPPERMASK) == MAPID_BARDDANCER && dstsd->status.sex == sd->status.sex) {
					// Cannot cast on another bard/dancer-type class of the same gender as caster
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if (dstsd->weapontype == W_FIST ||
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
					if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
					break;
				}
			}
			// 100% success rate at lv4 & 5, but lasts longer at lv5
			if(!clif->skill_nodamage(src,bl,skill_id,skill_lv, sc_start(src,bl,type,(60+skill_lv*10),skill_lv, skill->get_time(skill_id,skill_lv)))) {
				if (sd)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
		case SU_TUNAPARTY:
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
			break;

		case TK_JUMPKICK:
			/* Check if the target is an enemy; if not, skill should fail so the character doesn't unit->move_pos (exploitable) */
			if (battle->check_target(src, bl, BCT_ENEMY) > 0) {
				if (unit->move_pos(src, bl->x, bl->y, 1, true) == 0) {
					skill->attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
					clif->slide(src, bl->x, bl->y);
				}
			} else if (sd != NULL) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
			}
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
		case SU_FRESHSHRIMP:
		case SU_ARCLOUSEDASH:
		case RL_HEAT_BARREL:
		case RL_P_ALTER:
		case RL_E_CHAIN:
		case SJ_LIGHTOFMOON:
		case SJ_LIGHTOFSTAR:
		case SJ_FALLINGSTAR:
		case SJ_LIGHTOFSUN:
		case SJ_BOOKOFDIMENSION:
		case SP_SOULREAPER:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			break;
		// Works just like the above list of skills, except animation caused by
		// status must trigger AFTER the skill cast animation or it will cancel
		// out the status's animation.
		case SU_STOOP:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				id = mob->get_random_id(MOBG_DEAD_BRANCH, 0xF, sd->status.base_level);
				if (!id) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				sd->mission_mobid = id;
				sd->mission_count = 0;
				pc_setglobalreg(sd,script->add_variable("TK_MISSION_ID"), id);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}

				if( (lv = status->get_lv(src) - dstsd->status.base_level) < 0 )
					lv = -lv;
				if( lv > battle_config.devotion_level_difference || // Level difference requeriments
					(dstsd->sc.data[type] && dstsd->sc.data[type]->val1 != src->id) || // Cannot Devote a player devoted from another source
					(skill_id == ML_DEVOTION && (!mer || mer != dstsd->md)) || // Mercenary only can devote owner
					(dstsd->job & MAPID_UPPERMASK) == MAPID_CRUSADER || // Crusader Cannot be devoted
					(dstsd->sc.data[SC_HELLPOWER])) // Players affected by SC_HELLPOWERR cannot be devoted.
				{
					if( sd )
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
							map->freeblock_unlock();
							return 1;
						}
					}

					sd->devotion[i] = bl->id;
				}
				else if (mer != NULL)
					mer->devotion_flag = 1; // Mercenary Devoting Owner

				clif->skill_nodamage(src, bl, skill_id, skill_lv,
					sc_start4(src, bl, type, 100, src->id, i, skill->get_range2(src,skill_id,skill_lv),0, skill->get_time2(skill_id, skill_lv)));
				clif->devotion(src, NULL);
			}
			break;

		case SP_SOULUNITY:
		{
			if (sd == NULL || sd->status.party_id == 0 || (flag & 1)) {
				int i = 0;
				int count = min(5 + skill_lv, MAX_UNITED_SOULS);
				if (dstsd == NULL || sd == NULL) { // Only put player's souls in unity.
					if (sd != NULL)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
					break;
				}

				if (dstsd->sc.data[type] && dstsd->sc.data[type]->val2 != src->id) { // Fail if a player is in unity with another source.
					if (sd != NULL)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
					map->freeblock_unlock();
					return 1;
				}

				if (sd != NULL) { // Unite player's soul with caster's soul.
					ARR_FIND(0, count, i, sd->united_soul[i] == bl->id);
					if (i == count) {
						ARR_FIND(0, count, i, sd->united_soul[i] == 0);
						if (i == count) { // No more free slots? Fail the skill.
							clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
							map->freeblock_unlock();
							return 1;
						}
					}

					sd->united_soul[i] = bl->id;
				}

				clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start4(src, bl, type, 100, skill_lv, src->id, i, 0, skill->get_time(skill_id, skill_lv)));
			} else if (sd != NULL) {
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag | BCT_PARTY | 1, skill->castend_nodamage_id);
			}
		}
			break;
		
		case MO_CALLSPIRITS:
			if (sd != NULL) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				pc->addspiritball(sd, skill->get_time(skill_id, skill_lv), pc->getmaxspiritball(sd, skill_lv));
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
			if (dstsd != NULL && (dstsd->job & MAPID_BASEMASK) != MAPID_GUNSLINGER) {
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
			if (dstsd != NULL && dstsd->spiritball != 0
			 && (sd == dstsd || map_flag_vs(src->m) || (sd && sd->duel_group && sd->duel_group == dstsd->duel_group))
			 && (dstsd->job & MAPID_BASEMASK) != MAPID_GUNSLINGER
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
			if (sp != 0)
				status->heal(src, 0, sp, STATUS_HEAL_FORCED | STATUS_HEAL_SHOWEFFECT);
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
			                    skill->get_splash(skill_id, skill_lv), skill->splash_target(src),
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
		case RL_FIREDANCE:
		case RL_R_TRIP:
		case SJ_NEWMOONKICK:
		case SJ_SOLARBURST:
		case SJ_FULLMOONKICK:
		case SJ_FALLINGSTAR_ATK:
		case SJ_STAREMPEROR:
		{
			struct status_change *sc = status->get_sc(src);
			int count = 0;
	
			if (skill_id == SJ_NEWMOONKICK) {
				if (tsce != NULL) {
					status_change_end(bl, type, INVALID_TIMER);
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
					break;
				} else {
					sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
				}
			}
			if (skill_id == SJ_STAREMPEROR && sc != NULL && sc->data[SC_DIMENSION] != NULL) {
				if (sd != NULL) {
					pc->delspiritball(sd, sd->spiritball, 0);
					sc_start2(src, bl, SC_DIMENSION1, 100, skill_lv, status_get_max_sp(src), skill->get_time2(SJ_BOOKOFDIMENSION, 1));
					sc_start2(src, bl, SC_DIMENSION2, 100, skill_lv, status_get_max_sp(src), skill->get_time2(SJ_BOOKOFDIMENSION, 1));
				}
				status_change_end(src, SC_DIMENSION, INVALID_TIMER);
			}
			skill->area_temp[1] = 0;
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			count = map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src),
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
			                    skill->get_splash(skill_id, skill_lv), skill->splash_target(src),
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
			                    skill->get_splash(skill_id, skill_lv), skill->splash_target(src),
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
		case SJ_LUNARSTANCE:
		case SJ_STARSTANCE:
		case SJ_UNIVERSESTANCE:
		case SJ_SUNSTANCE:
		case SP_SOULCOLLECT:
			if (tsce != NULL) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, status_change_end(bl, type, INVALID_TIMER));
				map->freeblock_unlock();
				return 0;
			}
			if (skill_id == SP_SOULCOLLECT) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start2(src, bl, type, 100, skill_lv, pc->checkskill(sd, SP_SOULENERGY), max(1000, skill->get_time(skill_id, skill_lv))));
			} else {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			}
			break;
		case SL_KAITE:
		case SL_KAAHI:
		case SL_KAIZEL:
		case SL_KAUPE:
		case SP_KAUTE:
			if (sd != NULL) {
				if (!dstsd ||
					!(
						(sd->sc.data[SC_SOULLINK] && sd->sc.data[SC_SOULLINK]->val2 == SL_SOULLINKER)
						|| (dstsd->job & MAPID_UPPERMASK) == MAPID_SOUL_LINKER
						|| dstsd->status.char_id == sd->status.char_id
						|| dstsd->status.char_id == sd->status.partner_id
						|| dstsd->status.char_id == sd->status.child
						|| (skill_id == SP_KAUTE && dstsd->sc.data[SC_SOULUNITY] != NULL)
					)
				) {
					status->change_start(src, src, SC_STUN, 10000, skill_lv, 0, 0, 0, 500, SCFLAG_FIXEDRATE);
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
			}
			if (skill_id == SP_KAUTE) {
				if (!status->charge(src, sstatus->max_hp * (10 + 2 * skill_lv) / 100, 0)) {
					if (sd != NULL)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
					break;
				}
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				status->heal(bl, 0, tstatus->max_sp * (10 + 2 * skill_lv) / 100, 2);
			} else {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			}
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
				if( sd ) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				if ( skill_id == LG_FORCEOFVANGUARD || skill_id == RA_CAMOUFLAGE )
					break;
				map->freeblock_unlock();
				return 0;
			} else {
				int failure = sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv));
				if( failure )
					clif->skill_nodamage(src,bl,skill_id,( skill_id == LG_FORCEOFVANGUARD ) ? skill_lv : -1,failure);
				else if( sd )
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				snprintf(temp, sizeof(temp), msg_txt(882), md->name, skill->get_desc(skill_id)); // %s : %s !!
				clif->disp_overhead(&md->bl, temp, AREA_CHAT_WOC, NULL);
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
					if(sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				}
			}
			break;

		case TF_STEAL:
			if(sd) {
				if(pc->steal_item(sd,bl,skill_lv))
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				else
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
			}
			break;

		case RG_STEALCOIN:
			if(sd) {
				int amount = pc->steal_coin(sd, bl, skill_lv);
				if (amount > 0 && dstmd != NULL) {
					dstmd->state.provoke_flag = src->id;
					mob->target(dstmd, src, skill->get_range2(src, skill_id, skill_lv));
					clif->skill_nodamage(src, bl, skill_id, amount, 1);

				} else
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;

		case MG_STONECURSE:
			{
				int brate = 0;
				if (tstatus->mode&MD_BOSS) {
					if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
			status->heal(bl, 5, 0, STATUS_HEAL_DEFAULT);
			break;

		case AL_CURE:
			if(status->isimmune(bl)) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,0);
				break;
			}
			status_change_end(bl, SC_SILENCE, INVALID_TIMER);
			status_change_end(bl, SC_BLIND, INVALID_TIMER);
			status_change_end(bl, SC_CONFUSION, INVALID_TIMER);
			status_change_end(bl, SC_BITESCAR, INVALID_TIMER);
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			break;

		case TF_DETOXIFY:
			clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			status_change_end(bl, SC_POISON, INVALID_TIMER);
			status_change_end(bl, SC_DPOISON, INVALID_TIMER);
			break;

		case PR_STRECOVERY:
			if (status->isimmune(bl) != 0) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 0);
				break;
			}

			if (!battle->check_undead(tstatus->race, tstatus->def_ele)) {
				if (tsc != NULL && tsc->opt1 != 0) {
					status_change_end(bl, SC_FREEZE, INVALID_TIMER);
					status_change_end(bl, SC_STONE, INVALID_TIMER);
					status_change_end(bl, SC_SLEEP, INVALID_TIMER);
					status_change_end(bl, SC_STUN, INVALID_TIMER);
					status_change_end(bl, SC_WHITEIMPRISON, INVALID_TIMER);
				}

				status_change_end(bl, SC_NETHERWORLD, INVALID_TIMER);
			} else {
				int rate = 100 * (100 - (tstatus->int_ / 2 + tstatus->vit / 3 + tstatus->luk / 10));
				int duration = skill->get_time2(skill_id, skill_lv);

				duration = duration * (100 - (tstatus->int_ + tstatus->vit) / 2) / 100;
				status->change_start(src, bl, SC_BLIND, rate, 1, 0, 0, 0, duration, SCFLAG_NONE);
			}

			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);

			if (dstmd != NULL)
				mob->unlocktarget(dstmd, tick);

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
				status->heal(&mer->master->bl, mer->battle_status.hp, 0, STATUS_HEAL_SHOWEFFECT);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if (sd->auto_cast_current.type == AUTOCAST_NONE)
					status_zap(src, 0, skill->get_sp(skill_id, skill_lv)); // consume sp only if succeeded
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

				if (sd->auto_cast_current.type == AUTOCAST_TEMP || ((sd->auto_cast_current.skill_id == AL_TELEPORT || battle_config.skip_teleport_lv1_menu) && skill_lv == 1) || skill_lv == 3)
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					int bonus = 100, potion = min(500+skill_lv,505);
					int item_idx = skill->get_item_index(skill_id, skill_lv);

					if (item_idx == INDEX_NOT_FOUND) {
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						map->freeblock_unlock();
						return 1;
					}

					int item_id = skill->get_itemid(skill_id, item_idx);
					int inventory_idx = pc->search_inventory(sd, item_id);
					if (inventory_idx == INDEX_NOT_FOUND || item_id <= 0) {
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						map->freeblock_unlock();
						return 1;
					}
					if (sd->inventory_data[inventory_idx] == NULL || sd->status.inventory[inventory_idx].amount < skill->get_itemqty(skill_id, item_idx, skill_lv)) {
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						map->freeblock_unlock();
						return 1;
					}
					if( skill_id == AM_BERSERKPITCHER ) {
						if (dstsd != NULL && dstsd->status.base_level < sd->inventory_data[inventory_idx]->elv) {
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
							map->freeblock_unlock();
							return 1;
						}
					}
					script->potion_flag = 1;
					script->potion_hp = script->potion_sp = script->potion_per_hp = script->potion_per_sp = 0;
					script->potion_target = bl->id;
					script->run_use_script(sd, sd->inventory_data[inventory_idx], 0);
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
					if (tsc->data[SC_NO_RECOVER_STATE]) {
						hp = 0;
						sp = 0;
					}
				}
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				if( hp > 0 || (skill_id == AM_POTIONPITCHER && sp <= 0) )
					clif->skill_nodamage(NULL,bl,AL_HEAL,(int)hp,1);
				if( sp > 0 )
					clif->skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
				if (tsc) {
#ifdef RENEWAL
					if (tsc->data[SC_EXTREMITYFIST2])
						sp = 0;
#endif
					if (tsc->data[SC_NO_RECOVER_STATE]) {
						hp = 0;
						sp = 0;
					}
				}
				status->heal(bl, (int)hp, sp, STATUS_HEAL_DEFAULT);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;
		case AM_TWILIGHT2:
			if (sd) {
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
				//Prepare 200 Slim White Potions.
				if (!skill->produce_mix(sd, skill_id, ITEMID_WHITE_SLIM_POTION, 0, 0, 0, 200))
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if ((dstsd != NULL && (dstsd->job & MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
					|| (tsc && tsc->data[SC_SOULLINK] && tsc->data[SC_SOULLINK]->val2 == SL_ROGUE) //Rogue's spirit defends against dispel.
					|| (dstsd && pc_ismadogear(dstsd))
					|| rnd()%100 >= 50+10*skill_lv )
				{
					if (sd)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				int x;
				int y;
				enum unit_dir dir = unit->getdir(src);

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
					unit->move_pos(src, x, y, 1, false);
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
					status->heal(bl, 0, sp, STATUS_HEAL_SHOWEFFECT);
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
							if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

					if (hp != 0 || sp != 0)
						status->heal(src, hp, sp, STATUS_HEAL_SHOWEFFECT);
				}
			}
			break;
		case SA_MAGICROD:
			if (battle->bc->magicrod_type == 0)
				clif->skill_nodamage(src, src, SA_MAGICROD, skill_lv, 1); // Animation used here in official [Wolfie]
			sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
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
					int i = rnd() % ARRAYLENGTH(spellarray);
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
			clif->skill_nodamage(src, bl, skill_id, skill_lv,
				sc_start(src, bl, type, (skill_lv * 20), skill_lv, skill->get_time2(skill_id, skill_lv)));
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
				enum unit_dir dir;
				if (bl == src) //If cast on self, run forward, else run away.
					dir = unit->getdir(src);
				else
					dir = map->calc_dir(src, bl->x, bl->y);
				if (Assert_chk(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX)) {
					map->freeblock_unlock(); // unblock before assert-returning
					return 0;
				}
				unit->stop_attack(src);
				//Run skillv tiles overriding the can-move check.
				if (unit->walk_toxy(src, src->x + skill_lv * -dirx[dir], src->y + skill_lv * -diry[dir], 2) == 0
				    && md != NULL)
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
			int hp_rate = skill_lv == 0 ? 0 : skill->get_hp_rate(skill_id, skill_lv);
			int gain_hp = tstatus->max_hp*abs(hp_rate)/100; // The earned is the same % of the target HP than it cost the caster. [Skotlex]
			clif->skill_nodamage(src, bl, skill_id, status->heal(bl, gain_hp, 0, STATUS_HEAL_DEFAULT), 1);
		}
			break;
		case WE_FEMALE:
		{
			int sp_rate = skill_lv == 0 ? 0 : skill->get_sp_rate(skill_id, skill_lv);
			int gain_sp = tstatus->max_sp*abs(sp_rate)/100;// The earned is the same % of the target SP than it cost the caster. [Skotlex]
			clif->skill_nodamage(src, bl, skill_id, status->heal(bl, 0, gain_sp, STATUS_HEAL_DEFAULT), 1);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				break;
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			status->heal(bl, 0, sp, STATUS_HEAL_SHOWEFFECT);
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
							for (i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
								int nameid = skill->get_itemid(su->group->skill_id, i);
								if (nameid > 0) {
									int success;
									struct item item_tmp = { 0 };
									int amount = skill->get_itemqty(su->group->skill_id, i, skill_lv);
									item_tmp.nameid = nameid;
									item_tmp.identify = 1;
									if ((success = pc->additem(sd, &item_tmp, amount, LOG_TYPE_SKILL)) != 0) {
										clif->additem(sd,0,0,success);
										map->addflooritem(&sd->bl, &item_tmp, amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
									}
								}
							}
						} else {
							// get back 1 trap
							struct item item_tmp;
							memset(&item_tmp,0,sizeof(item_tmp));
							item_tmp.nameid = su->group->item_id ? su->group->item_id : ITEMID_BOOBY_TRAP;
							item_tmp.identify = 1;
							if (item_tmp.nameid && (flag=pc->additem(sd,&item_tmp,1,LOG_TYPE_SKILL)) != 0) {
								clif->additem(sd,0,0,flag);
								map->addflooritem(&sd->bl, &item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
							}
						}
					}
					skill->delunit(su);
				}else if(sd)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);

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
							FALLTHROUGH
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
				if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				map->freeblock_unlock();
				return 1;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,skill_id,src->id,skill->get_time(skill_id,skill_lv),1000));
		#ifndef RENEWAL
			if (sd)
				skill->blockpc_start(sd, skill_id, skill->get_time(skill_id, skill_lv) + 3000);
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
					if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						if(sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						break;
					}
					dstmd->state.soul_change_flag = 1;
					sp2 = sstatus->max_sp * 3 /100;
					status->heal(src, 0, sp2, STATUS_HEAL_SHOWEFFECT);
					clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
					break;
				}
				sp1 = sstatus->sp;
				sp2 = tstatus->sp;
#ifdef RENEWAL
				sp1 = sp1 / 2;
				sp2 = sp2 / 2;
				if (tsc && tsc->data[SC_EXTREMITYFIST2])
					sp1 = tstatus->sp;
#endif // RENEWAL
				if (tsc && tsc->data[SC_NO_RECOVER_STATE])
					sp1 = tstatus->sp;
				status->set_sp(src, sp2, STATUS_HEAL_FORCED | STATUS_HEAL_SHOWEFFECT);
				status->set_sp(bl, sp1, STATUS_HEAL_FORCED | STATUS_HEAL_SHOWEFFECT);
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
					if (tsc->data[SC_NO_RECOVER_STATE]) {
						hp = 0;
						sp = 0;
					}
				}
				if(hp > 0)
					clif->skill_nodamage(NULL,bl,AL_HEAL,hp,1);
				if(sp > 0)
					clif->skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
				status->heal(bl, hp, sp, STATUS_HEAL_DEFAULT);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);

					map->freeblock_unlock();
					return 0;
				}
				status_zap(src, 0, skill->get_sp(skill_id, skill_lv)); // consume sp only if succeeded [Inkfish]
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
								int where[] = {EQP_ARMOR, EQP_SHIELD, EQP_HELM};
								skill->break_equip(bl, where[rnd() % ARRAYLENGTH(where)], 10000, BCT_ENEMY);
							}
						}
						break;
					case 4: // atk halved
						sc_start(src,bl,SC_INCATKRATE,100,-50,skill->get_time2(skill_id,skill_lv));
						break;
					case 5: // 2000HP heal, random teleported
						status->heal(src, 2000, 0, STATUS_HEAL_DEFAULT);
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
							sc_start(src,bl,sc[rnd() % ARRAYLENGTH(sc)],100,skill_lv,skill->get_time2(skill_id,skill_lv));
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
			if (sd != NULL && tsc != NULL && 
				(tsc->data[SC_SOULGOLEM] || 
				tsc->data[SC_SOULSHADOW] ||
				tsc->data[SC_SOULFALCON] ||
				tsc->data[SC_SOULFAIRY])
			) { // Soul links from Soul Linker and Soul Reaper skills don't stack.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
				break;
			}
			//NOTE: here, 'type' has the value of the associated MAPID, not of the SC_SOULLINK constant.
			if (sd != NULL && !(dstsd != NULL && (dstsd->job & MAPID_UPPERMASK) == type)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				break;
			}
			if (skill_id == SL_SUPERNOVICE && dstsd && dstsd->die_counter && !(rnd()%100)) {
				//Erase death count 1% of the casts
				dstsd->die_counter = 0;
				pc_setglobalreg(dstsd,script->add_variable("PC_DIE_COUNTER"), 0);
				clif->specialeffect(bl, 0x152, AREA);
				//SC_SOULLINK invokes status_calc_pc for us.
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,SC_SOULLINK,100,skill_lv,skill_id,0,0,skill->get_time(skill_id,skill_lv)));
			sc_start(src,src,SC_SMA_READY,100,skill_lv,skill->get_time(SL_SMA,skill_lv));
			break;
		case SL_HIGH:
			if (sd != NULL && tsc != NULL && 
				(tsc->data[SC_SOULGOLEM] != NULL || 
				tsc->data[SC_SOULSHADOW] != NULL ||
				tsc->data[SC_SOULFALCON] != NULL ||
				tsc->data[SC_SOULFAIRY] != NULL)
			) { // Soul links from Soul Linker and Soul Reaper skills don't stack.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
				break;
			}
			if (sd != NULL && !(dstsd != NULL && (dstsd->job & JOBL_UPPER) != 0 && (dstsd->job & JOBL_2) == 0 && dstsd->status.base_level < 70)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				break;
			}
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
				sc_start4(src,bl,type,100,skill_lv,skill_id,0,0,skill->get_time(skill_id,skill_lv)));
			sc_start(src,src,SC_SMA_READY,100,skill_lv,skill->get_time(SL_SMA,skill_lv));
			break;

		case SP_SOULGOLEM:
		case SP_SOULSHADOW:
		case SP_SOULFALCON:
		case SP_SOULFAIRY:
			if (sd != NULL && dstsd == NULL) { // Only player's can be soul linked.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
				break;
			}
			if (tsc != NULL) {
				if (tsc->data[skill->get_sc_type(skill_id)]) { // Allow refreshing an already active soul link.
					clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
					break;
				} else if (
					(tsc->data[SC_SOULLINK] != NULL ||
					tsc->data[SC_SOULGOLEM] != NULL ||
					tsc->data[SC_SOULSHADOW] != NULL ||
					tsc->data[SC_SOULFALCON] != NULL ||
					tsc->data[SC_SOULFAIRY] != NULL)
				) { // Soul links from Soul Linker and Soul Reaper skills don't stack.
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
					break;
				}
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			break;

		case SP_SOULREVOLVE:
			if (!(tsc != NULL &&
				(tsc->data[SC_SOULLINK] != NULL ||
				tsc->data[SC_SOULGOLEM] != NULL ||
				tsc->data[SC_SOULSHADOW] != NULL ||
				tsc->data[SC_SOULFALCON] != NULL ||
				tsc->data[SC_SOULFAIRY] != NULL)
			)) {
				if (sd != NULL)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
				break;
			}
			status->heal(bl, 0, 50 * skill_lv, 2);
			status_change_end(bl, SC_SOULLINK, INVALID_TIMER);
			status_change_end(bl, SC_SOULGOLEM, INVALID_TIMER);
			status_change_end(bl, SC_SOULSHADOW, INVALID_TIMER);
			status_change_end(bl, SC_SOULFALCON, INVALID_TIMER);
			status_change_end(bl, SC_SOULFAIRY, INVALID_TIMER);
			break;
		case SL_SWOO:
			if (tsce) {
				if(sd)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				status->change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,10000,SCFLAG_FIXEDRATE);
				status_change_end(bl, SC_SWOO, INVALID_TIMER);
				break;
			}
		case SL_SKA: // [marquis007]
		case SL_SKE:
			if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;

		case GS_GLITTERING:
			if (sd != NULL) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				if (rnd()%100 < (20 + 10 * skill_lv))
					pc->addspiritball(sd, skill->get_time(skill_id, skill_lv), 10);
				else if (sd->spiritball > 0 && !pc->checkskill(sd, RL_RICHS_COIN))
					pc->delspiritball(sd, 1, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;

		case AM_REST:
			if (sd) {
				if (homun->vaporize(sd, HOM_ST_REST, false))
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				else
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;

		case HAMI_CASTLE: // [orn]
			if(rnd()%100 < 20*skill_lv && src != bl)
			{
				int x,y;
				x = src->x;
				y = src->y;
				if (hd) {
#ifdef RENEWAL
					skill->blockhomun_start(hd, skill_id, skill->get_cooldown(skill_id, skill_lv));
#else
					skill->blockhomun_start(hd, skill_id, skill->get_time2(skill_id, skill_lv));
#endif
				}


				if (unit->move_pos(src, bl->x, bl->y, 0, false) == 0) {
					clif->skill_nodamage(src,src,skill_id,skill_lv,1); // Homun
					clif->slide(src,bl->x,bl->y) ;
					if (unit->move_pos(bl, x, y, 0, false) == 0)
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
				clif->skill_fail(hd->master, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			else if (sd)
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			break;
		case HVAN_CHAOTIC: // [orn]
			{
				static const int per[5][2]={{20,50},{50,60},{25,75},{60,64},{34,67}};
				int r = rnd()%100;
				int target = (skill_lv-1)%5;
				int hp;
				if (r < per[target][0]) { //Self
					bl = src;
				} else if (r < per[target][1]) { //Master
					bl = battle->get_master(src);
				} else if ((per[target][1] - per[target][0]) < per[target][0]
					   && bl == battle->get_master(src)) {
					/**
					 * Skill rolled for enemy, but there's nothing the Homunculus is attacking.
					 * So bl has been set to its master in unit->skilluse_id2.
					 * If it's more likely that it will heal itself,
					 * we let it heal itself.
					 */
					bl = src;
				}

				if (!bl) bl = src;
				hp = skill->calc_heal(src, bl, skill_id, 1+rnd()%skill_lv, true);
				//Eh? why double skill packet?
				clif->skill_nodamage(src,bl,AL_HEAL,hp,1);
				clif->skill_nodamage(src,bl,skill_id,hp,1);
				status->heal(bl, hp, 0, STATUS_HEAL_DEFAULT);
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
				PRAGMA_GCC46(GCC diagnostic push)
				PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
				PRAGMA_GCC46(GCC diagnostic pop)
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
#if PACKETVER >= 20180207 /// unconfirmed
				if (skill_id == RK_IGNITIONBREAK)
					clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
#else
				clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
#endif
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;
		case RK_REFRESH:
		{
			int heal = status_get_max_hp(bl) * 25 / 100;
			clif->skill_nodamage(src,bl,skill_id,skill_lv,
			                     sc_start(src,bl,type,100,skill_lv,skill->get_time(skill_id,skill_lv)));
			status->heal(bl, heal, 0, STATUS_HEAL_FORCED);
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
						status->heal(bl, value, 0, STATUS_HEAL_FORCED);
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
					if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0, 0);
					break;
				}
				if( !status->charge(bl,heal,0) ) {
					if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0, 0);
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
				skill->produce_mix(sd, skill_id, ITEMID_ANSILA, 0, 0, 0, 1);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
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
					status->heal(bl, heal, 0, STATUS_HEAL_FORCED);
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

				if ((dstsd != NULL && (dstsd->job & MAPID_UPPERMASK) == MAPID_SOUL_LINKER) || rnd()%100 >= 60 + 8 * skill_lv) {
					if (sd)
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				if(status->isimmune(bl) || !tsc || !tsc->count)
					break;
				for(i = 0; i < SC_MAX; i++) {
					if ( !tsc->data[i] )
						continue;
					if (status->get_sc_type(i)&SC_NO_CLEARANCE)
						continue;
					PRAGMA_GCC46(GCC diagnostic push)
					PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
					PRAGMA_GCC46(GCC diagnostic pop)
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				}
			}else
			if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_TOTARGET, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING, 0, 0);
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
				enum unit_dir dir = unit->getdir(src);
				if (skill_id == NC_F_SIDESLIDE)
					dir = unit_get_opposite_dir(dir);
				skill->blown(src,bl,skill->get_blewcount(skill_id,skill_lv),dir,0);
				clif->slide(src,src->x,src->y);
				clif->skill_nodamage(src,bl,skill_id,skill_lv,1);
			}
			break;

		case NC_SELFDESTRUCTION:
			if (sd) {
				if (pc_ismadogear(sd))
					 pc->setmadogear(sd, false, MADO_ROBOT);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				skill->castend_damage_id(src, src, skill_id, skill_lv, tick, flag);
				status->set_sp(src, 0, STATUS_HEAL_DEFAULT);
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
				map->foreachinrange(skill->area_sub,src,skill->get_splash(skill_id,skill_lv),skill->splash_target(src),src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SD_SPLASH|1,skill->castend_damage_id);;
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_TOTARGET, 0, 0);
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
				status->heal(bl, heal, 0, STATUS_HEAL_SHOWEFFECT);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_IMITATION_SKILL_NONE, 0, 0);
			}
			break;

		case SC_SHADOWFORM:
			if( sd && dstsd && src != bl && !dstsd->shadowform_id ) {
				if( clif->skill_nodamage(src,bl,skill_id,skill_lv,sc_start4(src,src,type,100,skill_lv,bl->id,4+skill_lv,0,skill->get_time(skill_id, skill_lv))) )
					dstsd->shadowform_id = src->id;
			}
			else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			break;

		case SC_BODYPAINT:
			if( flag&1 ) {
				if( tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
							tsc->data[SC_CHASEWALK] || tsc->data[SC_CLOAKINGEXCEED] ) ) {
					status_change_end(bl, SC_HIDING, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
					status_change_end(bl, SC_CHASEWALK, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
					status_change_end(bl, SC_NEWMOON, INVALID_TIMER);

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
				rate = 10 * skill_lv + rnd->value( sstatus->dex / 12, sstatus->dex / 4 ) + joblvbonus + status->get_lv(src) / 10;
				// We then reduce the success chance based on the target's build.
				rate -= rnd->value( tstatus->agi / 6, tstatus->agi / 3 ) + tstatus->luk / 10 + ( dstsd ? (dstsd->max_weight / 10 - dstsd->weight / 10 ) / 100 : 0 ) + status->get_lv(bl) / 10;
				//Finally we set the minimum success chance cap based on the caster's skill level and DEX.
				rate = cap_value( rate, skill_lv + sstatus->dex / 20, 100);
				clif->skill_nodamage(src,bl,skill_id,0,sc_start(src,bl,type,rate,skill_lv,skill->get_time(skill_id,skill_lv)));
				if ( tsc && tsc->data[SC__IGNORANCE] && skill_id == SC_IGNORANCE) {
					//If the target was successfully inflected with the Ignorance status, drain some of the targets SP.
					int sp = 100 * skill_lv;
					if( dstmd ) sp = dstmd->level * 2;
					if( status_zap(bl,0,sp) )
						status->heal(src, 0, sp / 2, STATUS_HEAL_FORCED | STATUS_HEAL_SHOWEFFECT);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			break;

		case LG_TRAMPLE:
			clif->skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
			map->foreachinrange(skill->destroy_trap,bl,skill->get_splash(skill_id,skill_lv),BL_SKILL,tick);
			status_change_end(bl, SC_SV_ROOTTWIST, INVALID_TIMER);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
								status->heal(bl, val, 0, STATUS_HEAL_SHOWEFFECT);
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
				if (dstsd != NULL && dstsd->spiritball != 0 && (sd == dstsd || map_flag_vs(src->m)) && (dstsd->job & MAPID_BASEMASK) != MAPID_GUNSLINGER) {
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
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|BCT_SELF|SD_SPLASH|1, skill->castend_nodamage_id);
			}
			break;

		case SR_POWERVELOCITY:
			if( !dstsd )
				break;
			if (sd != NULL && (dstsd->job & MAPID_BASEMASK) != MAPID_GUNSLINGER) {
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
				status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);

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
					status->set_sp(bl, 0, STATUS_HEAL_DEFAULT);
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
				 clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					pc->autocast_clear(sd);
					sd->auto_cast_current.type = AUTOCAST_IMPROVISE;
					sd->auto_cast_current.skill_id = improv_skill_id;
					sd->auto_cast_current.skill_lv = improv_skill_lv;
					VECTOR_ENSURE(sd->auto_cast, 1, 1);
					VECTOR_PUSH(sd->auto_cast, sd->auto_cast_current);
					clif->item_skill(sd, improv_skill_id, improv_skill_lv);
					pc->autocast_clear_current(sd);
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
							case BL_NUL:
							case BL_CHAT:
							case BL_HOM:
							case BL_MER:
							case BL_ELEM:
							case BL_ALL:
								break;
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

		case SU_HIDE:
			if (tsce != NULL) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				status_change_end(bl, type, INVALID_TIMER);
				map->freeblock_unlock();
				return 0;
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv));
			break;

		case SU_BUNCHOFSHRIMP:
			if (sd == NULL || sd->status.party_id == 0 || flag&1) {
				clif->skill_nodamage(bl, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
			} else if (sd != NULL) {
				party->foreachsamemap(skill->area_sub, sd, skill->get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill->castend_nodamage_id);
			}
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
				e_hp = ed->battle_status.max_hp * 10 / 100;
				e_sp = ed->battle_status.max_sp * 10 / 100;
				status->heal(&ed->bl, e_hp, e_sp, STATUS_HEAL_FORCED | STATUS_HEAL_SHOWEFFECT);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				int ammo_id;
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
						clif->skill_fail(sd, GN_SLINGITEM_RANGEMELEEATK, 0xa, 0, 0);
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
				enum spirit_charm_types ttype = skill->get_ele(skill_id, skill_lv);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				pc->add_charm(sd, skill->get_time(skill_id, skill_lv), MAX_SPIRITCHARM, ttype); // replace existing charms of other type
			}
			break;

		case KO_ZANZOU:
			if(sd) {
				struct mob_data *summon_md;

				summon_md = mob->once_spawn_sub(src, src->m, src->x, src->y, clif->get_bl_name(src), MOBID_KO_KAGE, "", SZ_SMALL, AI_NONE, 0);
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
				if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_SIZE, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			break;

		case KO_GENWAKU:
			if ( !map_flag_gvg2(src->m) && ( dstsd || dstmd ) && !(tstatus->mode&MD_PLANT) && battle->check_target(src,bl,BCT_ENEMY) > 0 ) {
				int x = src->x, y = src->y;
				if( sd && rnd()%100 > max(5, (45 + 5 * skill_lv) - status_get_int(bl) / 10) ){//[(Base chance of success) - ( target's int / 10)]%.
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}

				if (unit->move_pos(src, bl->x, bl->y, 0, false) == 0) {
					clif->skill_nodamage(src, src, skill_id, skill_lv, 1);
					clif->blown(src);
					sc_start(src, src, SC_CONFUSION, 25, skill_lv, skill->get_time(skill_id, skill_lv));
					if (!is_boss(bl) && unit->move_pos(bl, x, y, 0, false) == 0) {
						if (dstsd != NULL && pc_issit(dstsd))
							pc->setstand(dstsd);
						clif->blown(bl);
						sc_start(src, bl, SC_CONFUSION, 75, skill_lv, skill->get_time(skill_id, skill_lv));
					}
				}
			}
			break;

		case OB_AKAITSUKI:
		case OB_OBOROGENSOU:
			if( sd && ( (skill_id == OB_OBOROGENSOU && bl->type == BL_MOB) // This skill does not work on monsters.
				|| is_boss(bl) ) ){ // Does not work on Boss monsters.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_TOTARGET_PLAYER, 0, 0);
				break;
			}
		case KO_IZAYOI:
		case OB_ZANGETSU:
		case KG_KYOMU:
		case KG_KAGEMUSYA:
		case SP_SOULDIVISION:
			if (skill_id == SP_SOULDIVISION) { // Usable only on other players.
				if (bl->type != BL_PC) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					break;
				}
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill->get_time(skill_id, skill_lv)));
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
						status_change_end(bl, SC_NEWMOON, INVALID_TIMER);
				}
				if( skill->area_temp[2] == 1 ){
					clif->skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, BDT_SKILL);
					sc_start(src, src, SC_STOP, 100, skill_lv, skill->get_time(skill_id, skill_lv));
				}
			} else {
				skill->area_temp[2] = 0;
				map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), skill->splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill->castend_nodamage_id);
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
			if (hd == NULL)
				break;
			if(tsc){
				int i;
				for (i = 0; i < ARRAYLENGTH(scs); i++) {
					if (tsc->data[scs[i]]) status_change_end(bl, scs[i], INVALID_TIMER);
				}
			}
			heal = 5 * status->get_lv(&hd->bl) + status->base_matk(&hd->bl, &hd->battle_status, status->get_lv(&hd->bl));
			status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);
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
			Assert_retb(skill_lv <= ARRAYLENGTH(summons));

			i = map->foreachinmap(skill->check_condition_mob_master_sub, src->m, BL_MOB, src->id, summons[skill_lv-1].mob_id, skill_id, &dummy);
			if(i >= summons[skill_lv-1].quantity)
				break;

			for (i = 0; i < summons[skill_lv-1].quantity; i++) {
				struct mob_data *summon_md = mob->once_spawn_sub(src, src->m, src->x, src->y, clif->get_bl_name(src),
				                                                 summons[skill_lv-1].mob_id, "", SZ_SMALL, AI_ATTACK, 0);
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
		case RL_RICHS_COIN:
			if (sd != NULL) {
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				for (int i = 0; i < 10; i++)
					pc->addspiritball(sd, skill->get_time(skill_id, skill_lv), 10);
			}
			break;
		case RL_C_MARKER:
			if (sd != NULL) {
				int i;

				if (tsce != NULL && tsce->val2 != src->id)
					status_change_end(bl, type, INVALID_TIMER);

				ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == bl->id);
				if (i == MAX_SKILL_CRIMSON_MARKER) {
					ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, !sd->c_marker[i]);
					if (i == MAX_SKILL_CRIMSON_MARKER) {
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						break;
					}
				}
				sd->c_marker[i] = bl->id;
				status->change_start(src, bl, type, 10000, skill_lv, src->id, 0, 0, skill->get_time(skill_id, skill_lv), SCFLAG_NOAVOID | SCFLAG_FIXEDTICK | SCFLAG_FIXEDRATE);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			} else {
				status->change_start(src, bl, type, 10000, skill_lv, src->id, 0, 0, skill->get_time(skill_id, skill_lv), SCFLAG_NOAVOID | SCFLAG_FIXEDTICK | SCFLAG_FIXEDRATE);
				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			}
			break;
		case RL_QD_SHOT:
			if (sd != NULL) {
				skill->area_temp[1] = bl->id;
				// Check surrounding
				skill->area_temp[0] = map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill->area_sub_count);
				if (skill->area_temp[0])
					map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill->castend_damage_id);

				// Main target always receives damage
				clif->skill_nodamage(src, src, skill_id, skill_lv, 1);
				skill->attack(skill->get_type(skill_id, skill_lv), src, src, bl, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL);
			} else {
				clif->skill_nodamage(src, src, skill_id, skill_lv, 1);
				map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill->castend_damage_id);
			}
			status_change_end(src, SC_QD_SHOT_READY, INVALID_TIMER); // End here to prevent spamming of the skill onto the target.
			skill->area_temp[0] = 0;
			skill->area_temp[1] = 0;
			break;
		case RL_FLICKER:
			if (sd != NULL) {
				int i;

				sd->flicker = true;
				skill->area_temp[1] = 0;

				clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
				if (pc->checkskill(sd, RL_B_TRAP))
					map->foreachinrange(skill->bind_trap, src, AREA_SIZE, BL_SKILL, src);
				if ((i = pc->checkskill(sd, RL_H_MINE)))
					map->foreachinrange(skill->area_sub, src, skill->get_splash(skill_id, skill_lv), BL_CHAR, src, RL_H_MINE, i, tick, flag | BCT_ENEMY | SD_SPLASH, skill->castend_damage_id);
				sd->flicker = false;
			}
			break;
		case SJ_DOCUMENT:
			if (sd != NULL) {
				switch (skill_lv) {
				case 1:
					pc->resetfeel(sd);
					break;
				case 2:
					pc->resethate(sd);
					break;
				case 3:
					pc->resetfeel(sd);
					pc->resethate(sd);
					break;
				}
			}
			clif->skill_nodamage(src, bl, skill_id, skill_lv, 1);
			break;
		case SJ_GRAVITYCONTROL:
		{
			int fall_damage = sstatus->batk + sstatus->rhw.atk - tstatus->def2;

			if (bl->type == BL_PC)
				fall_damage += dstsd->weight / 10 - tstatus->def;
			else // Monster's don't have weight. Put something in its place.
				fall_damage += 50 * status->get_lv(src) - tstatus->def;

			fall_damage = max(1, fall_damage);

			clif->skill_nodamage(src, bl, skill_id, skill_lv, sc_start2(src, bl, type, 100, skill_lv, fall_damage, skill->get_time(skill_id, skill_lv)));
		}
			break;
		default:
			if (skill->castend_nodamage_id_unknown(src, bl, &skill_id, &skill_lv, &tick, &flag)) {
				map->freeblock_unlock();
				return 1;
			}
			break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)

	if(skill_id != SR_CURSEDCIRCLE) {
		struct status_change *sc = status->get_sc(src);
		if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] )//Should only remove after the skill had been casted.
			status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);
	}

	if (dstmd) { //Mob skill event for no damage skills (damage ones are handled in battle_calc_damage) [Skotlex]
		mob->log_damage(dstmd, src, 0); //Log interaction (counts as 'attacker' for the exp bonus)
		mob->use_skill_event(dstmd, src, tick, MSC_SKILLUSED | (skill_id << 16));
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

static bool skill_castend_nodamage_id_dead_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	return true;
}

static bool skill_castend_nodamage_id_undead_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	return true;
}

static bool skill_castend_nodamage_id_mado_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	return false;
}

static bool skill_castend_nodamage_id_unknown(struct block_list *src, struct block_list *bl, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	nullpo_retr(true, skill_id);
	nullpo_retr(true, skill_lv);
	ShowWarning("skill_castend_nodamage_id: Unknown skill used:%d\n", *skill_id);
	clif->skill_nodamage(src, bl, *skill_id, *skill_lv, 1);
	return true;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_castend_pos(int tid, int64 tick, int id, intptr_t data)
{
	GUARD_MAP_LOCK

	struct block_list* src = map->id2bl(id);
	struct map_session_data *sd;
	struct unit_data *ud = unit->bl2ud(src);
	struct mob_data *md;

	nullpo_ret(src);
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

	if (sd && ud->skilltimer != INVALID_TIMER && (pc->checkskill(sd, SA_FREECAST) > 0 || ud->skill_id == LG_EXEEDBREAK || (skill->get_inf2(ud->skill_id) & INF2_FREE_CAST_REDUCED) != 0))
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
			if (sd) clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			break;
		}
		if( src->type&battle_config.skill_nofootset &&
			skill->get_unit_flag(ud->skill_id)&UF_NOFOOTSET &&
			skill->check_unit_range2(src,ud->skillx,ud->skilly,ud->skill_id,ud->skill_lv)
		  )
		{
			if (sd) clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				if (sd) clif->skill_fail(sd, ud->skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

		if (sd == NULL || sd->auto_cast_current.skill_id != ud->skill_id || skill->get_delay(ud->skill_id, ud->skill_lv) != 0)
			ud->canact_tick = tick + skill->delay_fix(src, ud->skill_id, ud->skill_lv);
		if (sd != NULL) { //Cooldown application
			int cooldown = pc->get_skill_cooldown(sd, ud->skill_id, ud->skill_lv);
			if (cooldown != 0)
				skill->blockpc_start(sd, ud->skill_id, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif->status_change(src, status->get_sc_icon(SC_POSTDELAY), status->get_sc_relevant_bl_types(SC_POSTDELAY), 1, skill->delay_fix(src, ud->skill_id, ud->skill_lv), 0, 0, 0);
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

		if (sd != NULL && ud->skill_id != AL_WARP && ud->skill_id == sd->auto_cast_current.skill_id) // Warp-Portal thru items will clear data in skill_castend_map. [Inkfish]
			pc->autocast_remove(sd, sd->auto_cast_current.type, ud->skill_id, ud->skill_lv);

		unit->set_dir(src, map->calc_dir(src, ud->skillx, ud->skilly));

		if (ud->skilltimer == INVALID_TIMER) {
			if (md) md->skill_idx = -1;
			else ud->skill_id = 0; //Non mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skill_lv = ud->skillx = ud->skilly = 0;
		}

		map->freeblock_unlock();
		return 1;
	} while(0);

	if (sd == NULL || sd->auto_cast_current.skill_id != ud->skill_id || skill->get_delay(ud->skill_id, ud->skill_lv) != 0)
		ud->canact_tick = tick;

	if (sd != NULL && ud->skill_id == sd->auto_cast_current.skill_id)
		pc->autocast_remove(sd, sd->auto_cast_current.type, ud->skill_id, ud->skill_lv);
	else if(md)
		md->skill_idx  = -1;

	ud->skill_id = 0;
	ud->skill_lv = 0;

	return 0;

}

static int skill_check_npc_chaospanic(struct block_list *bl, va_list args)
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
static int skill_count_wos(struct block_list *bl, va_list ap)
{
	struct block_list* src = va_arg(ap, struct block_list*);
	nullpo_retr(1, bl);
	nullpo_retr(1, src);
	if( src->id != bl->id ) {
		return 1;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_castend_map(struct map_session_data *sd, uint16 skill_id, const char *mapname)
{
	nullpo_ret(sd);
	nullpo_ret(mapname);

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
		sd->sc.data[SC_GRAVITYCONTROL] ||
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

	switch (skill_id) {
		case AL_TELEPORT:
			if (strcmp(mapname, "Random") == 0)
				pc->randomwarp(sd, CLR_TELEPORT);
			else if (sd->menuskill_val > 1) // Need lv2 to be able to warp here.
				pc->setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);

			if (battle_config.teleport_close_storage == 1 && sd->state.storage_flag != STORAGE_FLAG_CLOSED) {
				if (sd->state.storage_flag == STORAGE_FLAG_NORMAL)
					storage->close(sd);
				if (sd->state.storage_flag == STORAGE_FLAG_GUILD)
					gstorage->close(sd);
			} else {
				clif->refresh_storagewindow(sd);
			}
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						skill_failed(sd);
						return 0;
					}
				}

				lv = (sd->auto_cast_current.type > AUTOCAST_TEMP) ? sd->auto_cast_current.skill_lv : pc->checkskill(sd, skill_id);
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

				// Clear data which was skipped in skill_castend_pos().
				pc->autocast_remove(sd, sd->auto_cast_current.type, sd->auto_cast_current.skill_id,
						    sd->auto_cast_current.skill_lv);

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
static int skill_castend_pos2(struct block_list *src, int x, int y, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
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
	type = skill->get_sc_type(skill_id);
	sce = (sc != NULL && type != SC_NONE) ? sc->data[type] : NULL;

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch (skill_id) { //Skill effect.
		case WZ_METEOR:
		case MO_BODYRELOCATION:
		case CR_CULTIVATION:
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
		case SC_ESCAPE:
		case SU_CN_METEOR:
			break; //Effect is displayed on respective switch case.
		default:
			skill->castend_pos2_effect_unknown(src, &x, &y, &skill_id, &skill_lv, &tick, &flag);
			break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)

	// SC_MAGICPOWER needs to switch states before any damage is actually dealt
	skill->toggle_magicpower(src, skill_id, skill_lv);

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
			if (battle_config.trap_visibility != 0) {
				map->foreachinarea(skill_reveal_trap,
			                   src->m, x - r, y - r, x + r, y + r, BL_SKILL);
			}
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
			if (sd && map->foreachinarea(skill->check_npc_chaospanic, src->m, x-3, y-3, x+3, y+3, BL_NPC) > 0 ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				break;
			}
			FALLTHROUGH

		case MG_SAFETYWALL:
		{
			int alive = 1;
			if ( map->foreachincell(skill->cell_overlap, src->m, x, y, BL_SKILL, skill_id, &alive, src) ) {
				skill->unitsetting(src, skill_id, skill_lv, x, y, 0);
				return 0; // Don't consume gems if cast on LP
			}
		}
		FALLTHROUGH
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
		case SU_CN_POWDERING:
		case SU_SV_ROOTTWIST:
		case SJ_BOOKOFCREATINGSTAR:
			flag |= 1; // Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
			FALLTHROUGH
		case GS_GROUNDDRIFT: //Ammo should be deleted right away.
			if ( skill_id == WM_SEVERE_RAINSTORM )
				sc_start(src, src, type, 100, 0, skill->get_time(skill_id, skill_lv));
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
				if( map->foreachinrange(skill->count_wos, src, 2, BL_MOB|BL_PC, src) ) {
					if( sd )
						clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
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
		case SO_CLOUD_KILL:
			flag |= (skill_id == SO_WARMER) ? 8 : 4;
			skill->unitsetting(src,skill_id,skill_lv,x,y,0);
			break;
		case WZ_METEOR:
		case SU_CN_METEOR:
			{
				int area = skill->get_splash(skill_id, skill_lv);
				short tmpx = 0, tmpy = 0, x1 = 0, y1 = 0;
				int i;

#if 0
				// The Meteor should inflict curse if Catnip fruit is consumed.
				// Currently Catnip fruit is added as requirement.
				if (sd && skill_id == SU_CN_METEOR) {
					short item_idx = pc->search_inventory(sd, ITEMID_CATNIP_FRUIT);
					if (item_idx >= 0) {
						pc->delitem(sd, item_idx, 1, 0, DELITEM_SKILLUSE, LOG_TYPE_SKILL);
						flag |= 1;
					}
				}
#endif

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
			if (unit->move_pos(src, x, y, 1, true) == 0) {
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
				unit->move_pos(src, x, y, 1, false);
				clif->slide(src,x,y);
			}
			status_change_end(src, SC_HIDING, INVALID_TIMER);
			break;
		case SU_LOPE:
		{
			if (map->list[src->m].flag.noteleport && !(map->list[src->m].flag.battleground || map_flag_gvg2(src->m))) {
				x = src->x;
				y = src->y;
			}
			clif->skill_nodamage(src, src, SU_LOPE, skill_lv, 1);
			if(!map->count_oncell(src->m, x, y, BL_PC | BL_NPC | BL_MOB, 0) && map->getcell(src->m, src, x, y, CELL_CHKREACH)) {
				clif->slide(src, x, y);
				unit->move_pos(src, x, y, 1, false);
			}
		}
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
				md = mob->once_spawn_sub(src, src->m, x, y, clif->get_bl_name(src), class_, "", SZ_SMALL, AI_NONE, 0);
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
				int item_idx = skill->get_item_index(skill_id, skill_lv);

				if (item_idx == INDEX_NOT_FOUND) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 1;
				}

				int item_id = skill->get_itemid(skill_id, item_idx);
				int inventory_idx = pc->search_inventory(sd, item_id);
				int bonus;
				if (inventory_idx == INDEX_NOT_FOUND || item_id <= 0
				 || sd->inventory_data[inventory_idx] == NULL
				 || sd->status.inventory[inventory_idx].amount < skill->get_itemqty(skill_id, item_idx, skill_lv)
				) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 1;
				}
				script->potion_flag = 1;
				script->potion_hp = 0;
				script->potion_sp = 0;
				script->run_use_script(sd, sd->inventory_data[inventory_idx], 0);
				script->potion_flag = 0;
				//Apply skill bonuses
				bonus = pc->checkskill(sd,CR_SLIMPITCHER)*10
					+ pc->checkskill(sd,AM_POTIONPITCHER)*10
					+ pc->checkskill(sd,AM_LEARNINGPOTION)*5
					+ pc->skillheal_bonus(sd, skill_id);

				script->potion_hp = script->potion_hp * (100 + bonus) / 100;
				script->potion_sp = script->potion_sp * (100 + bonus) / 100;

				if (script->potion_hp > 0 || script->potion_sp > 0) {
					r = skill->get_splash(skill_id, skill_lv);
					map->foreachinarea(skill->area_sub,
					                   src->m, x - r, y - r, x + r, y + r, BL_CHAR,
					                   src, skill_id, skill_lv, tick, flag|BCT_PARTY|BCT_GUILD|1,
					                   skill->castend_nodamage_id);
				}
			} else {
				int item_idx = skill->get_item_index(skill_id, skill_lv);

				if (item_idx == INDEX_NOT_FOUND)
					return 1;

				int item_id = skill->get_itemid(skill_id, item_idx);
				struct item_data *item = itemdb->search(item_id);
				int bonus;
				script->potion_flag = 1;
				script->potion_hp = 0;
				script->potion_sp = 0;
				script->run(item->script,0,src->id,0);
				script->potion_flag = 0;
				bonus = skill->get_max(CR_SLIMPITCHER)*10;

				script->potion_hp = script->potion_hp * (100 + bonus)/100;
				script->potion_sp = script->potion_sp * (100 + bonus)/100;

				if (script->potion_hp > 0 || script->potion_sp > 0) {
					r = skill->get_splash(skill_id, skill_lv);
					map->foreachinarea(skill->area_sub,
					                   src->m, x - r, y - r, x + r, y + r, BL_CHAR,
					                   src, skill_id, skill_lv, tick, flag|BCT_PARTY|BCT_GUILD|1,
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
				if (sd) clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 1;
				}
				clif->skill_poseffect(src,skill_id,skill_lv,x,y,tick);
				if (rnd()%100 < 50) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				} else {
					int mob_id = skill_lv < 2 ? MOBID_BLACK_MUSHROOM + rnd()%2 : MOBID_RED_PLANT + rnd()%6;
					struct mob_data *md = mob->once_spawn_sub(src, src->m, x, y, DEFAULT_MOB_JNAME, mob_id, "", SZ_SMALL, AI_NONE, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
		case RL_HAMMER_OF_GOD:
			r = skill->get_splash(skill_id,skill_lv);
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,skill->splash_target(src),
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
			map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,skill->splash_target(src),
			                   src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill->castend_nodamage_id);
			break;
		/**
		 * Guilotine Cross
		 **/
		case GC_POISONSMOKE:
			if( !(sc && sc->data[SC_POISONINGWEAPON]) ) {
				if( sd )
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_GC_POISONINGWEAPON, 0, 0);
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
				int i;
				int wave = skill_lv + 4;
				enum unit_dir dir = map->calc_dir(src, x, y);
				Assert_ret(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX);
				int sx = x = src->x, sy = y = src->y; // Store first caster's location to avoid glitch on unit setting

				for (i = 1; i <= wave; i++) {
					sy = y + i * diry[dir];
					if (dir == UNIT_DIR_WEST || dir == UNIT_DIR_EAST)
						sx = x + i * dirx[dir];
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
				struct mob_data *md = mob->once_spawn_sub(src, src->m, x, y, clif->get_bl_name(src), MOBID_SILVERSNIPER, "", SZ_SMALL, AI_NONE, 0);
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
				sc_start(src, src, SC__FEINTBOMB_MASTER, 100, 0, skill->get_unit_interval(SC_FEINTBOMB, skill_lv));
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
				skill->get_type(skill_id, skill_lv), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
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
				map->foreachinarea(skill->area_sub,src->m,x-r,y-r,x+r,y+r,skill->splash_target(src),
					src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill->castend_damage_id);
			} else if( sd )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
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

					skill->addtimerskill(src, tick + (int64)r * 250, 0, tmpx, tmpy, GN_CRAZYWEED_ATK, skill_lv, 0, 0);
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
		case RL_FALLEN_ANGEL:
			if (unit->move_pos(src, x, y, 1, true) == 0) {
				clif->snap(src, src->x, src->y);
				sc_start(src, src, type, 100, skill_id, skill->get_time(skill_id, skill_lv));
			} else {
				if (sd != NULL)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			}
			break;
		case RL_FIRE_RAIN:
			{
				int wave = skill_lv + 5;
				int dir = map->calc_dir(src, x, y);
				int sx = x = src->x;
				int sy = y = src->y;

				for (int w = 0; w <= wave; w++) {
					switch (dir) {
					case UNIT_DIR_NORTH:
					case UNIT_DIR_NORTHWEST:
					case UNIT_DIR_NORTHEAST:
						sy = y + w;
						break;
					case UNIT_DIR_WEST:
						sx = x - w;
						break;
					case UNIT_DIR_SOUTHWEST:
					case UNIT_DIR_SOUTH:
					case UNIT_DIR_SOUTHEAST:
						sy = y - w;
						break;
					case UNIT_DIR_EAST:
						sx = x + w;
						break;
					}
					skill->addtimerskill(src, timer->gettick() + (80 * w), 0, sx, sy, skill_id, skill_lv, dir, flag);
				}
			}
			break;
		default:
			if (skill->castend_pos2_unknown(src, &x, &y, &skill_id, &skill_lv, &tick, &flag))
				return 1;
			break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)

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

static void skill_castend_pos2_effect_unknown(struct block_list *src, int *x, int *y, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	if (skill->get_inf(*skill_id) & INF_SELF_SKILL)
		clif->skill_nodamage(src, src, *skill_id, *skill_lv, 1);
	else
		clif->skill_poseffect(src, *skill_id, *skill_lv, *x, *y, *tick);
}

static bool skill_castend_pos2_unknown(struct block_list *src, int *x, int *y, uint16 *skill_id, uint16 *skill_lv, int64 *tick, int *flag)
{
	ShowWarning("skill_castend_pos2: Unknown skill used:%d\n", *skill_id);
	return true;
}

/// transforms 'target' skill unit into dissonance (if conditions are met)
static int skill_dance_overlap_sub(struct block_list *bl, va_list ap)
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
static int skill_dance_overlap(struct skill_unit *su, int flag)
{
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
static bool skill_dance_switch(struct skill_unit *su, int flag)
{
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
		group->unit_id     = skill->get_unit_id(skill_id, 1, 0);
		group->target_flag = skill->get_unit_target(skill_id, 1);
		group->bl_flag     = skill->get_unit_bl_target(skill_id, 1);
		group->interval    = skill->get_unit_interval(skill_id, 1);
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
static struct skill_unit_group *skill_unitsetting(struct block_list *src, uint16 skill_id, uint16 skill_lv, int16 x, int16 y, int flag)
{
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
	interval = skill->get_unit_interval(skill_id, skill_lv);
	target = skill->get_unit_target(skill_id, skill_lv);
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
	nullpo_retr(NULL, st);
	sc = status->get_sc(src); // for traps, firewall and fogwall - celest

	switch (skill_id) {
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
			FALLTHROUGH
		case HT_SHOCKWAVE:
			val1=skill_lv*15+10;
			FALLTHROUGH
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
		case RL_B_TRAP:
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
				ARR_FIND(0, MAX_SKILL_ITEM_REQUIRE, i, req.itemid[i] && (req.itemid[i] == ITEMID_BOOBY_TRAP || req.itemid[i] == ITEMID_SPECIAL_ALLOY_TRAP));
				if( i != MAX_SKILL_ITEM_REQUIRE && req.itemid[i] )
					req_item = req.itemid[i];
				if (skill_id == RL_B_TRAP) // Target type should not change on GvG maps.
					break;
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
			if (val1 == ELE_NEUTRAL)
				val1 = element[rnd() % ARRAYLENGTH(element)];

			switch (val1) {
				case ELE_FIRE:
					subunt = 4;
					break;
				case ELE_WATER:
					subunt = 3;
					break;
				case ELE_POISON:
					subunt = 2;
					break;
				case ELE_DARK:
					subunt = 1;
					break;
				case ELE_WIND:
					subunt = 0;
					break;
				default:
					subunt = rnd() % 5;
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
			FALLTHROUGH
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

	nullpo_retr(NULL, layout);
	nullpo_retr(NULL, group = skill->init_unitgroup(src, layout->count, skill_id, skill_lv, skill->get_unit_id(skill_id, skill_lv, flag & 1) + subunt, limit, interval));
	group->val1=val1;
	group->val2=val2;
	group->val3=val3;
	group->target_flag=target;
	group->bl_flag= skill->get_unit_bl_target(skill_id, skill_lv);
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
			case RL_B_TRAP:
				val1 = 3500;
				val2 = 0;
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
		skill->del_unitgroup(group);
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

static void skill_unitsetting1_unknown(struct block_list *src, uint16 *skill_id, uint16 *skill_lv, int16 *x, int16 *y, int *flag, int *val1, int *val2, int *val3)
{
}

static void skill_unitsetting2_unknown(struct block_list *src, uint16 *skill_id, uint16 *skill_lv, int16 *x, int16 *y, int *flag, int *unit_flag, int *val1, int *val2, int *val3, struct skill_unit_group *group)
{
	nullpo_retv(group);
	nullpo_retv(val2);
	nullpo_retv(unit_flag);
	if (group->state.song_dance & 0x1)
		*val2 = *unit_flag & (UF_DANCE | UF_SONG); //Store whether this is a song/dance
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_unit_onplace(struct skill_unit *src, struct block_list *bl, int64 tick)
{
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

	if (skill->get_type(sg->skill_id, sg->skill_lv) == BF_MAGIC && map->getcell(src->bl.m, &src->bl, src->bl.x, src->bl.y, CELL_CHKLANDPROTECTOR) != 0 && sg->skill_id != SA_LANDPROTECTOR)
		return 0; //AoE skills are ineffective. [Skotlex]
	sc = status->get_sc(bl);

	if (sc && sc->option&OPTION_HIDE && sg->skill_id != WZ_HEAVENDRIVE && sg->skill_id != WL_EARTHSTRAIN )
		return 0; //Hidden characters are immune to AoE skills except to these. [Skotlex]

	if (sc && sc->data[SC_VACUUM_EXTREME] && map->getcell(bl->m, bl, bl->x, bl->y, CELL_CHKLANDPROTECTOR))
		status_change_end(bl, SC_VACUUM_EXTREME, INVALID_TIMER);

	if ( sc && sc->data[SC_HOVERING] && ( sg->skill_id == SO_VACUUM_EXTREME || sg->skill_id == SO_ELECTRICWALK || sg->skill_id == SO_FIREWALK || sg->skill_id == WZ_QUAGMIRE ) )
		return 0;

	type = skill->get_sc_type(sg->skill_id);
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
						skill->del_unitgroup(sg);

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
			FALLTHROUGH
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
			FALLTHROUGH
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
				skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_REVERBERATION:
			if (sg->src_id == bl->id)
				break; //Does not affect the caster.
			clif->changetraplook(&src->bl,UNT_USED_TRAPS);
			skill->trap_do_splash(&src->bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
			sg->unit_id = UNT_USED_TRAPS;
			sg->limit = DIFF_TICK32(tick,sg->tick) + 1500;
			break;

		case UNT_VOLCANIC_ASH:
			if (!sce)
				sc_start(ss, bl, SC_VOLCANIC_ASH, 100, sg->skill_lv, skill->get_time(MH_VOLCANIC_ASH, sg->skill_lv));
			break;

		case UNT_CATNIPPOWDER:
			if (sg->src_id == bl->id || (status_get_mode(bl)&MD_BOSS))
				break; // Does not affect the caster or Boss.
			if (sce == NULL && battle->check_target(&src->bl, bl, BCT_ENEMY) > 0)
				sc_start(ss, bl, type, 100, sg->skill_lv, skill->get_time(sg->skill_id, sg->skill_lv));
			break;

		case UNT_BOOKOFCREATINGSTAR:
			if (sce == NULL)
				sc_start4(ss, bl, type, 100, sg->skill_lv, ss->id, src->bl.id, 0, sg->limit);
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

static void skill_unit_onplace_unknown(struct skill_unit *src, struct block_list *bl, int64 *tick)
{
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_unit_onplace_timer(struct skill_unit *src, struct block_list *bl, int64 tick)
{
	GUARD_MAP_LOCK

	struct skill_unit_group *sg;
	struct block_list *ss;
	struct map_session_data *tsd;
	struct status_data *tstatus;
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
	nullpo_ret(tstatus);
	type = skill->get_sc_type(sg->skill_id);
	skill_id = sg->skill_id;

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
	PRAGMA_GCC46(GCC diagnostic pop)

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

	if (sg->skill_id == HT_ANKLESNARE
		|| (battle_config.trap_trigger == 1 && skill->get_inf2(sg->skill_id) & INF2_HIDDEN_TRAP)
	) {
		src->visible = true;
		clif->skillunit_update(&src->bl);
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
				status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);
				if (diff >= 500)
					sg->val1--;
			}
			if (sg->val1 <= 0)
				skill->del_unitgroup(sg);
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
				status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);
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
					if (skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0) <= 0 && tsc != NULL)
						tsc->sg_counter=0; //Attack absorbed.
					break;
		#endif
				case GS_DESPERADO:
					if (rnd()%100 < src->val1)
						skill->attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
					break;
				default:
					skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
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
			if (sg->val2 == 0 && tsc && (sg->unit_id == UNT_ANKLESNARE || bl->id != sg->src_id)) {
				int sec = skill->get_time2(sg->skill_id, sg->skill_lv);
				struct mob_data *md = NULL;
				if (sg->unit_id == UNT_MANHOLE && (md = BL_CAST(BL_MOB, bl)) != NULL
				 && (md->class_ == MOBID_EMPELIUM || md->class_ == MOBID_BARRICADE || md->class_ == MOBID_S_EMPEL_1 || md->class_ == MOBID_S_EMPEL_2)
					) {
					// Do nothing if target are the specified monsters on manhole
				} else if (status->change_start(ss, bl, type, 10000, sg->skill_lv, sg->group_id, 0, 0, sec, SCFLAG_FIXEDRATE)) {
					const struct TimerData* td = tsc->data[type] ? timer->get(tsc->data[type]->timer) : NULL;
					if (td)
						sec = DIFF_TICK32(td->tick, tick);
					if (sg->unit_id == UNT_MANHOLE || battle_config.skill_trap_type || map_flag_gvg2(src->bl.m) != 0) {
						unit->move_pos(bl, src->bl.x, src->bl.y, 0, false);
						clif->fixpos(bl);
					}
					sg->val2 = bl->id;
				} else {
					sec = 3000; //Couldn't trap it?
				}
				if (sg->unit_id == UNT_ANKLESNARE) {
					/**
					 * If you're snared from a trap that was invisible this makes the trap be
					 * visible again -- being you stepped on it (w/o this the trap remains invisible and you go "WTF WHY I CANT MOVE")
					 * bugreport:3961
					 **/
					clif->changetraplook(&src->bl, UNT_ANKLESNARE);
				}
				sg->limit = DIFF_TICK32(tick, sg->tick) + sec;
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

				skill->trap_do_splash(&src->bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
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
			FALLTHROUGH
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
		case UNT_CLUSTERBOMB:
			if( bl->id == ss->id )// it won't trigger on caster
				break;
			FALLTHROUGH
		case UNT_LANDMINE:
		case UNT_BLASTMINE:
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
		case UNT_FREEZINGTRAP:
		case UNT_FIREPILLAR_ACTIVE:
		case UNT_CLAYMORETRAP:
			if (sg->unit_id == UNT_FIRINGTRAP || sg->unit_id == UNT_ICEBOUNDTRAP || sg->unit_id == UNT_CLAYMORETRAP)
				skill->trap_do_splash(&src->bl, sg->skill_id, sg->skill_lv, sg->bl_flag | BL_SKILL | ~BCT_SELF, tick);
			else
				skill->trap_do_splash(&src->bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
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
				status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);
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
						status->heal(bl, heal, 0, STATUS_HEAL_DEFAULT);
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
			skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_GROUNDDRIFT_WIND:
		case UNT_GROUNDDRIFT_DARK:
		case UNT_GROUNDDRIFT_POISON:
		case UNT_GROUNDDRIFT_WATER:
		case UNT_GROUNDDRIFT_FIRE:
			skill->trap_do_splash(&src->bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
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
					status->heal(bl, hp, sp, STATUS_HEAL_SHOWEFFECT);
					sc_start(ss, bl, type, 100, sg->skill_lv, (sg->interval * 3) + 100);
				}
				// Reveal hidden players every 5 seconds.
				if( sg->val2 % 5 == 0 ) {
					// TODO: check if other hidden status can be removed.
					status_change_end(bl,SC_HIDING,INVALID_TIMER);
					status_change_end(bl,SC_CLOAKING,INVALID_TIMER);
					status_change_end(bl,SC_CLOAKINGEXCEED,INVALID_TIMER);
					status_change_end(bl, SC_NEWMOON, INVALID_TIMER);
				}
			}
			/* Enable this if kRO fix the current skill. Currently no damage on undead and demon monster. [Jobbie]
			else if( battle->check_target(ss, bl, BCT_ENEMY) > 0 && battle->check_undead(tstatus->race, tstatus->def_ele) )
				skill->castend_damage_id(&src->bl, bl, sg->skill_id, sg->skill_lv, 0, 0);*/
			break;

		case UNT_STEALTHFIELD:
			if( bl->id == sg->src_id )
				break; // Don't work on Self (video shows that)
			FALLTHROUGH
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
			skill->trap_do_splash(&src->bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
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
					skill->attack(skill->get_type(GN_THORNS_TRAP, sg->skill_lv), ss, ss, bl, sg->skill_id, sg->skill_lv, tick, SD_LEVEL|SD_ANIMATION);
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
						skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl,
									 sg->skill_id, sg->skill_lv + 10 * sg->val2, tick, 0);
						break;
					case 3:
						skill->attack(skill->get_type(CR_ACIDDEMONSTRATION, sg->skill_lv), ss, &src->bl, bl,
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
				skill->attack(skill->get_type(GN_HELLS_PLANT_ATK, sg->skill_lv), ss, &src->bl, bl, GN_HELLS_PLANT_ATK, sg->skill_lv, tick, 0);
			if( ss != bl) //The caster is the only one who can step on the Plants, without destroying them
				sg->limit = DIFF_TICK32(tick, sg->tick) + 100;
			break;

		case UNT_CLOUD_KILL:
			if(tsc && !tsc->data[type])
				status->change_start(ss,bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill->get_time2(sg->skill_id,sg->skill_lv),SCFLAG_FIXEDRATE);
			skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
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
				status->heal(bl, hp, 0, STATUS_HEAL_DEFAULT);
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
						status->heal(bl, hp, 0, STATUS_HEAL_SHOWEFFECT);
					} else if( (sg->unit_id ==  UNT_FIRE_INSIGNIA && tstatus->def_ele == ELE_EARTH)
					        || (sg->unit_id ==  UNT_WATER_INSIGNIA && tstatus->def_ele == ELE_FIRE)
					        || (sg->unit_id ==  UNT_WIND_INSIGNIA && tstatus->def_ele == ELE_WATER)
					        || (sg->unit_id ==  UNT_EARTH_INSIGNIA && tstatus->def_ele == ELE_WIND)
					) {
						status->heal(bl, -hp, 0, STATUS_HEAL_DEFAULT);
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
				struct status_data *bst = status->get_base_status(bl);
				nullpo_ret(bst);

				int basestr = bst->str;
				if (basestr > 130)
					basestr = 130;
				sg->limit -= 1000 * basestr / 20;
				if (sg->limit < 0)
					sg->limit = 0;
				sc_start(ss, bl, SC_VACUUM_EXTREME, 100, sg->skill_lv, sg->limit);

				if ( !map_flag_gvg(bl->m) && !map->list[bl->m].flag.battleground && !is_boss(bl) ) {
					if (unit->move_pos(bl, sg->val1, sg->val2, 0, false) == 0) {
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
			if (battle->check_target(&src->bl, bl, BCT_ENEMY) > 0) {
				switch (sg->unit_id) {
				case UNT_ZENKAI_WATER:
					switch (rnd() % 3) {
					case 0:
						sc_start(ss, bl, SC_COLD, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case 1:
						sc_start(ss, bl, SC_FREEZE, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case 2:
						sc_start(ss, bl, SC_FROSTMISTY, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					}
					break;
				case UNT_ZENKAI_LAND:
					switch (rnd() % 2) {
					case 0:
						sc_start(ss, bl, SC_STONE, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case 1:
						sc_start(ss, bl, SC_POISON, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					}
					break;
				case UNT_ZENKAI_FIRE:
					sc_start4(ss, bl, SC_BURNING, sg->val1 * 5, sg->skill_lv, 0, ss->id, 0, skill->get_time2(sg->skill_id, sg->skill_lv));
					break;
				case UNT_ZENKAI_WIND:
					switch (rnd() % 3) {
					case 0:
						sc_start(ss, bl, SC_SILENCE, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case 1:
						sc_start(ss, bl, SC_SLEEP, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					case 2:
						sc_start(ss, bl, SC_DEEP_SLEEP, sg->val1 * 5, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
						break;
					}
					break;
				}
			} else {
				sc_start2(ss, bl, type, 100, sg->val1, sg->val2, skill->get_time2(sg->skill_id, sg->skill_lv));
			}
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
		case UNT_SV_ROOTTWIST:
			if (status_get_mode(bl)&MD_BOSS) {
				break;
			}
			if (tsc) {
				if (!sg->val2) {
					int sec = skill->get_time(sg->skill_id, sg->skill_lv);

					if (sc_start2(ss, bl, type, 100, sg->skill_lv, sg->group_id, sec)) {
						const struct TimerData* td = ((tsc->data[type])? timer->get(tsc->data[type]->timer) : NULL);

						if (td != NULL)
							sec = DIFF_TICK32(td->tick, tick);
						clif->fixpos(bl);
						sg->val2 = bl->id;
					} else { // Couldn't trap it?
						sec = 7000;
					}
					sg->limit = DIFF_TICK32(tick, sg->tick) + sec;
				} else if (tsc->data[type] && bl->id == sg->val2) {
					skill->attack(skill->get_type(SU_SV_ROOTTWIST_ATK, sg->skill_lv), ss, &src->bl, bl, SU_SV_ROOTTWIST_ATK, sg->skill_lv, tick, SD_LEVEL|SD_ANIMATION);
				}
			}
			break;
		case UNT_FIRE_RAIN:
			clif->skill_damage(ss, bl, tick, status_get_amotion(ss), 0,
				skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, SD_ANIMATION | SD_SPLASH),
				1, sg->skill_id, sg->skill_lv, BDT_SKILL);
			break;
		case UNT_B_TRAP:
			if (tsc != NULL && tsc->data[type])
				break;
			sc_start(ss, bl, type, 100, sg->skill_lv, skill->get_time2(sg->skill_id, sg->skill_lv));
			src->val2++;
			break;
		default:
			skill->unit_onplace_timer_unknown(src, bl, &tick);
			break;
	}

	if (bl->type == BL_MOB && ss != bl)
		mob->use_skill_event(BL_UCAST(BL_MOB, bl), ss, tick, MSC_SKILLUSED | (skill_id << 16));

	return skill_id;
}

static void skill_unit_onplace_timer_unknown(struct skill_unit *src, struct block_list *bl, int64 *tick)
{
}

/*==========================================
 * Triggered when a char steps out of a skill cell
 *------------------------------------------*/
static int skill_unit_onout(struct skill_unit *src, struct block_list *bl, int64 tick)
{
	struct skill_unit_group *sg;
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	nullpo_ret(src);
	nullpo_ret(bl);
	nullpo_ret(sg=src->group);
	sc = status->get_sc(bl);
	type = skill->get_sc_type(sg->skill_id);
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
static int skill_unit_onleft(uint16 skill_id, struct block_list *bl, int64 tick)
{
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	sc = status->get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	type = skill->get_sc_type(skill_id);
	sce = (sc != NULL && type != SC_NONE) ? sc->data[type] : NULL;

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
		case SJ_BOOKOFCREATINGSTAR:
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
				nullpo_retb(sc);
				if ((sce = sc->data[SC_BLIND])) {
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
	PRAGMA_GCC46(GCC diagnostic pop)

	return skill_id;
}

/*==========================================
 * Invoked when a unit cell has been placed/removed/deleted.
 * flag values:
 * flag&1: Invoke onplace function (otherwise invoke onout)
 * flag&4: Invoke a onleft call (the unit might be scheduled for deletion)
 *------------------------------------------*/
static int skill_unit_effect(struct block_list *bl, va_list ap)
{
	struct skill_unit* su = va_arg(ap,struct skill_unit*);
	struct skill_unit_group* group;
	int64 tick = va_arg(ap,int64);
	unsigned int flag = va_arg(ap,unsigned int);
	uint16 skill_id;
	bool dissonance;

	nullpo_ret(bl);
	nullpo_ret(su);
	group = su->group;

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
static int skill_unit_ondamaged(struct skill_unit *src, struct block_list *bl, int64 damage, int64 tick)
{
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
static int skill_check_condition_char_sub(struct block_list *bl, va_list ap)
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
		if (tsd->status.party_id == sd->status.party_id && (tsd->job & MAPID_THIRDMASK) == MAPID_MINSTRELWANDERER)
			p_sd[(*c)++] = tsd->bl.id;
		return 1;
	} else {

		switch(skill_id) {
			case PR_BENEDICTIO:
			{
				enum unit_dir dir = map->calc_dir(&sd->bl, tsd->bl.x, tsd->bl.y);
				dir = (unit->getdir(&sd->bl) + dir) % UNIT_DIR_MAX; //This adjusts dir to account for the direction the sd is facing.
				if ((tsd->job & MAPID_BASEMASK) == MAPID_ACOLYTE && (dir == UNIT_DIR_WEST || dir == UNIT_DIR_EAST) //Must be standing to the left/right of Priest.
				    && sd->status.sp >= 10) {
					p_sd[(*c)++]=tsd->bl.id;
				}
				return 1;
			}
			case AB_ADORAMUS:
			// Adoramus does not consume Blue Gemstone when there is at least 1 Priest class next to the caster
				if ((tsd->job & MAPID_UPPERMASK) == MAPID_PRIEST)
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			case WL_COMET:
			// Comet does not consume Red Gemstones when there is at least 1 Warlock class next to the caster
				if ((tsd->job & MAPID_THIRDMASK) == MAPID_WARLOCK)
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			case LG_RAYOFGENESIS:
				if (tsd->status.party_id == sd->status.party_id && (tsd->job & MAPID_THIRDMASK) == MAPID_ROYAL_GUARD && tsd->sc.data[SC_BANDING])
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			default: //Warning: Assuming Ensemble Dance/Songs for code speed. [Skotlex]
				{
					uint16 skill_lv;
					if(pc_issit(tsd) || !unit->can_move(&tsd->bl))
						return 0;
					if (sd->status.sex != tsd->status.sex &&
							(tsd->job & MAPID_UPPERMASK) == MAPID_BARDDANCER &&
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
static int skill_check_pc_partner(struct map_session_data *sd, uint16 skill_id, uint16 *skill_lv, int range, int cast_flag)
{
	static int c=0;
	static int p_sd[2] = { 0, 0 };
	int i;
	bool is_chorus = ( skill->get_inf2(skill_id)&INF2_CHORUS_SKILL );

	nullpo_ret(sd);
	nullpo_ret(skill_lv);

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
static int skill_check_condition_mob_master_sub(struct block_list *bl, va_list ap)
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
static int skill_isammotype(struct map_session_data *sd, int skill_id, int skill_lv)
{
	nullpo_ret(sd);
	return (
		battle_config.arrow_decrement==2 &&
		(sd->weapontype == W_BOW || (sd->weapontype >= W_REVOLVER && sd->weapontype <= W_GRENADE)) &&
		skill_id != HT_PHANTASMIC &&
		skill->get_type(skill_id, skill_lv) == BF_WEAPON &&
		!(skill->get_nk(skill_id)&NK_NO_DAMAGE) &&
		!skill->get_spiritball(skill_id, skill_lv) //Assume spirit spheres are used as ammo instead.
	);
}

/**
 * Checks whether a skill can be used in combos or not
 **/
static bool skill_is_combo(int skill_id)
{
	if (skill->get_inf2(skill_id) & INF2_IS_COMBO_SKILL)
		return true;

	return false;
}

/**
 * Checks if a skill's equipment requirements are fulfilled.
 *
 * @param sd The character who casts the skill.
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return 0 on success or 1 in case of error.
 *
 **/
static int skill_check_condition_required_equip(struct map_session_data *sd, int skill_id, int skill_lv)
{
	nullpo_retr(1, sd);

	struct skill_condition req = skill->get_requirement(sd, skill_id, skill_lv);
	bool any_equip_flag = skill->get_equip_any_flag(skill_id, skill_lv);
	bool any_equip_found = false;
	int fail_id = 0;
	int fail_amount = 0;

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		if (req.equip_id[i] == 0)
			continue;

		int req_id = req.equip_id[i];
		int req_amount = req.equip_amount[i];
		int found_amount = 0;

		for (int j = 0; j < EQI_MAX; j++) {
			int inv_idx = sd->equip_index[j];

			if (inv_idx == INDEX_NOT_FOUND || sd->inventory_data[inv_idx] == NULL)
				continue;

			if ((j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == inv_idx)
			    || (j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == inv_idx)
			    || (j == EQI_HEAD_TOP && sd->equip_index[EQI_HEAD_MID] == inv_idx)
			    || (j == EQI_HEAD_TOP && sd->equip_index[EQI_HEAD_LOW] == inv_idx)
			    || (j == EQI_COSTUME_MID && sd->equip_index[EQI_COSTUME_LOW] == inv_idx)
			    || (j == EQI_COSTUME_TOP && sd->equip_index[EQI_COSTUME_MID] == inv_idx)
			    || (j == EQI_COSTUME_TOP && sd->equip_index[EQI_COSTUME_LOW] == inv_idx)) {
				continue; // Equipment uses more than one slot; only process it once!
			}

			if (itemdb_type(req_id) != IT_CARD) {
				if (sd->inventory_data[inv_idx]->nameid != req_id)
					continue;

				if (itemdb_type(req_id) == IT_AMMO)
					found_amount += sd->status.inventory[inv_idx].amount;
				else
					found_amount++;
			} else {
				if (itemdb_isspecial(sd->status.inventory[inv_idx].card[0]))
					continue;

				for (int k = 0; k < sd->inventory_data[inv_idx]->slot; k++) {
					if (sd->status.inventory[inv_idx].card[k] == req_id)
						found_amount++;
				}
			}
		}

		if (any_equip_flag) {
			if (found_amount >= req_amount) {
				any_equip_found = true;
				break;
			} else if (fail_id == 0) { // Save ID/amount of first missing equipment for skill fail message.
				fail_id = req_id;
				fail_amount = req_amount;
			}
		} else if (found_amount < req_amount) {
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_NEED_EQUIPMENT, req_amount, req_id);
			return 1;
		}
	}

	if (any_equip_flag && !any_equip_found) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_NEED_EQUIPMENT, fail_amount, fail_id);
		return 1;
	}

	return 0;
}

static int skill_check_condition_castbegin(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	struct status_data *st;
	struct status_change *sc;
	struct skill_condition require;

	nullpo_ret(sd);

	if (skill_lv < 1 || skill_lv > MAX_SKILL_LEVEL)
		return 0;

	if (sd->chat_id != 0)
		return 0;

	if (((sd->auto_cast_current.itemskill_conditions_checked || !sd->auto_cast_current.itemskill_check_conditions)
	    && sd->auto_cast_current.type == AUTOCAST_ITEM) || sd->auto_cast_current.type == AUTOCAST_IMPROVISE) {
		return 1;
	}

	if ((sd->auto_cast_current.type == AUTOCAST_ABRA || sd->auto_cast_current.type == AUTOCAST_IMPROVISE) && sd->auto_cast_current.skill_id == skill_id)
		return 1;

	if (pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL) && sd->auto_cast_current.type != AUTOCAST_ITEM) {
		// GMs don't override the AUTOCAST_ITEM check, otherwise they can use items without them being consumed!
		sd->state.arrow_atk = skill->get_ammotype(skill_id)?1:0; //Need to do arrow state check.
		sd->spiritball_old = sd->spiritball; //Need to do Spiritball check.
		return 1;
	}

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
	PRAGMA_GCC46(GCC diagnostic pop)
	st = &sd->battle_status;
	sc = &sd->sc;
	if( !sc->count )
		sc = NULL;

	if (pc_is90overweight(sd) && sd->auto_cast_current.type != AUTOCAST_ITEM) { // Skill casting items ignore the overweight restriction.
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_WEIGHTOVER, 0, 0);
		return 0;
	}

	if( sc && ( sc->data[SC__SHADOWFORM] || sc->data[SC__IGNORANCE] ) )
		return 0;

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
		case SU_HIDE:
		case SJ_LUNARSTANCE:
		case SJ_STARSTANCE:
		case SJ_UNIVERSESTANCE:
		case SJ_SUNSTANCE:
		case SP_SOULCOLLECT:
			if (sc && sc->data[skill->get_sc_type(skill_id)])
				return 1;
			FALLTHROUGH
		default:
		{
			int ret = skill->check_condition_castbegin_off_unknown(sc, &skill_id);
			if (ret >= 0)
				return ret;
		}
	}
	PRAGMA_GCC46(GCC diagnostic pop)

	// Check the skills that can be used while mounted on a warg
	if( pc_isridingwug(sd) ) {
		PRAGMA_GCC46(GCC diagnostic push)
		PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
		PRAGMA_GCC46(GCC diagnostic pop)
	}

	// Check the skills that can be used whiled using mado
	if( pc_ismadogear(sd) ) {
		PRAGMA_GCC46(GCC diagnostic push)
		PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
		PRAGMA_GCC46(GCC diagnostic pop)
	}

	require = skill->get_requirement(sd,skill_id,skill_lv);

	//Can only update state when weapon/arrow info is checked.
	sd->state.arrow_atk = require.ammo?1:0;

	// perform skill-specific checks (and actions)
	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch( skill_id ) {
		case MC_VENDING:
		case ALL_BUYING_STORE:
			if (map->list[sd->bl.m].flag.novending) {
				clif->message(sd->fd, msg_sd(sd, 276)); // "You can't open a shop on this map"
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNOVENDING)) {
				clif->message(sd->fd, msg_sd(sd, 204)); // "You can't open a shop on this cell."
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case SO_SPELLFIST:
			if(sd->skill_id_old != MG_FIREBOLT && sd->skill_id_old != MG_COLDBOLT && sd->skill_id_old != MG_LIGHTNINGBOLT){
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			FALLTHROUGH
		case SA_CASTCANCEL:
			if(sd->ud.skilltimer == INVALID_TIMER) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, MO_TRIPLEATTACK, 0);
			}
			return 0;
		case MO_COMBOFINISH:
			if(!sc)
				return 0;
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if ( sc->data[SC_COMBOATTACK]->val1 == MO_CHAINCOMBO )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, MO_CHAINCOMBO, 0);
			}
			return 0;
		case CH_TIGERFIST:
			if(!sc)
				return 0;
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if ( sc->data[SC_COMBOATTACK]->val1 == MO_COMBOFINISH )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, MO_COMBOFINISH, 0);
			}
			return 0;
		case CH_CHAINCRUSH:
			if(!sc)
				return 0;
			if( sc && sc->data[SC_COMBOATTACK] ) {
				if( sc->data[SC_COMBOATTACK]->val1 == CH_TIGERFIST )
					break;
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, CH_TIGERFIST, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;

		case TK_MISSION:
			if ((sd->job & MAPID_UPPERMASK) != MAPID_TAEKWON) {
				// Cannot be used by Non-Taekwon classes
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;

		case TK_READYCOUNTER:
		case TK_READYDOWN:
		case TK_READYSTORM:
		case TK_READYTURN:
		case TK_JUMPKICK:
			if ((sd->job & MAPID_UPPERMASK) == MAPID_SOUL_LINKER) {
				// Soul Linkers cannot use this skill
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;

		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
			if ((sd->job & MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
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
			if (sc->data[SC_COMBOATTACK]->val1 != skill_id
			 && !(sd != NULL && sd->status.base_level >= 90 && pc->fame_rank(sd->status.char_id, RANKTYPE_TAEKWON) > 0)) {
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 0;
				}
				time = 1000*(sc->data[SC_DANCING]->val3>>16);
				if (skill->get_time(
					(sc->data[SC_DANCING]->val1&0xFFFF), //Dance Skill ID
					(sc->data[SC_DANCING]->val1>>16)) //Dance Skill LV
					- time < skill->get_time2(skill_id,skill_lv))
				{
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 0;
				}
			}
			break;

		case PR_BENEDICTIO:
			if (skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 0) < 2)
			{
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;

		case SL_SMA:
			if(!(sc != NULL && (sc->data[SC_SMA_READY] != NULL || sc->data[SC_USE_SKILL_SP_SHA] != NULL)))
				return 0;
			break;

		case HT_POWER:
			if(!(sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == skill_id))
				return 0;
			break;

		case CG_HERMODE:
			if(!npc->check_areanpc(1,sd->bl.m,sd->bl.x,sd->bl.y,skill->get_splash(skill_id, skill_lv)))
			{
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
						return 0;
					}
				}
			}
			break;
		case PR_REDEMPTIO:
			{
				int64 exp;
				if (((exp = pc->nextbaseexp(sd)) > 0 && get_percentage64(sd->status.base_exp, exp) < 1) ||
					((exp = pc->nextjobexp(sd)) > 0 && get_percentage64(sd->status.job_exp, exp) < 1)) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0); //Not enough exp.
					return 0;
				}
				break;
			}
		case AM_TWILIGHT2:
		case AM_TWILIGHT3:
			if (!party->skill_check(sd, sd->status.party_id, skill_id, skill_lv))
			{
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			return 0;
		case SG_FUSION:
			if (sc && sc->data[SC_SOULLINK] && sc->data[SC_SOULLINK]->val2 == SL_STAR)
				break;
			//Auron insists we should implement SP consumption when you are not Soul Linked. [Skotlex]
			//Only invoke on skill begin cast (instant cast skill). [Kevin]
			if( require.sp > 0 ) {
				if (st->sp < (unsigned int)require.sp)
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT, 0, 0);
				else
					status_zap(&sd->bl, 0, require.sp);
			}
			return 0;
		case GD_BATTLEORDER:
		case GD_REGENERATION:
		case GD_RESTORE:
			if (!map_flag_gvg2(sd->bl.m)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			FALLTHROUGH
		case GD_EMERGENCYCALL:
			// other checks were already done in skillnotok()
			if (!sd->status.guild_id || !sd->state.gmaster_flag)
				return 0;
			break;

		case GS_GLITTERING:
		case RL_RICHS_COIN:
			if (sd->spiritball >= 10) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;

		case NJ_ISSEN:
#ifdef RENEWAL
			if (st->hp < (st->hp/100)) {
#else
			if (st->hp < 2) {
#endif
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			FALLTHROUGH
		case NJ_BUNSINJYUTSU:
			if (!(sc && sc->data[SC_NJ_NEN])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;

		case NJ_ZENYNAGE:
		case KO_MUCHANAGE:
			if(sd->status.zeny < require.zeny) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_MONEY, 0, 0);
				return 0;
			}
			break;
		case PF_HPCONVERSION:
			if (st->sp == st->max_sp)
				return 0; //Unusable when at full SP.
			break;
		case AM_CALLHOMUN: //Can't summon if a hom is already out
			if (sd->status.hom_id && sd->hd && !sd->hd->homunculus.vaporize) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case AM_REST: //Can't vapo homun if you don't have an active homun or it's hp is < 80%
			if (!homun_alive(sd->hd) || sd->hd->battle_status.hp < (sd->hd->battle_status.max_hp*80/100))
			{
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		/**
		 * Arch Bishop
		 **/
		case AB_ANCILLA:
			{
				int count = 0;
				for (int i = 0; i < sd->status.inventorySize; i ++)
					if (sd->status.inventory[i].nameid == ITEMID_ANSILA)
						count += sd->status.inventory[i].amount;
				if( count >= 3 ) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_ANCILLA_NUMOVER, 0, 0);
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
				//clif->skill_fail(sd, skill_id, USESKILL_FAIL_NEED_ITEM, require.amount[0], 0, require.itemid[0]);
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0, 0);
							return 0;
						}
						break;
					case WL_RELEASE:
						for(j = SC_SPELLBOOK7; j >= SC_SPELLBOOK1; j--)
							if( sc && sc->data[j] )
								i++;
						if( i == 0 ){
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_SUMMON_NONE, 0, 0);
							return 0;
						}
						break;
					default:
						if( i == 5 ){
							clif->skill_fail(sd, skill_id, USESKILL_FAIL_SUMMON, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case GC_COUNTERSLASH:
		case GC_WEAPONCRUSH:
			if( !(sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == GC_WEAPONBLOCKING) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_GC_WEAPONBLOCKING, 0, 0);
				return 0;
			}
			break;
		/**
		 * Ranger
		 **/
		case RA_WUGMASTERY:
			if( pc_isfalcon(sd) || pc_isridingwug(sd) || sd->sc.data[SC__GROOMY] ) {
				clif->skill_fail(sd, skill_id, sd->sc.data[SC__GROOMY] ? USESKILL_FAIL_MANUAL_NOTIFY : USESKILL_FAIL_CONDITION, 0, 0);
				return 0;
			}
			break;
		case RA_WUGSTRIKE:
			if( !pc_iswug(sd) && !pc_isridingwug(sd) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0, 0);
				return 0;
			}
			break;
		case RA_WUGRIDER:
			if( pc_isfalcon(sd) || ( !pc_isridingwug(sd) && !pc_iswug(sd) ) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0, 0);
				return 0;
			}
			break;
		case RA_WUGDASH:
			if(!pc_isridingwug(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0, 0);
				return 0;
			}
			break;
		/**
		 * Royal Guard
		 **/
		case LG_BANDING:
			if( sc && sc->data[SC_INSPIRATION] ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case LG_PRESTIGE:
			if( sc && (sc->data[SC_BANDING] || sc->data[SC_INSPIRATION]) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case LG_RAYOFGENESIS:
		case LG_HESPERUSLIT:
			if( sc && sc->data[SC_INSPIRATION]  )
				return 1; // Don't check for partner.
			if( !(sc && sc->data[SC_BANDING]) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, SR_DRAGONCOMBO, 0);
			}
			return 0;
		case SR_CRESCENTELBOW:
			if( sc && sc->data[SC_CRESCENTELBOW] ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_DUPLICATE, 0, 0);
				return 0;
			}
			break;
		case SR_CURSEDCIRCLE:
			if (map_flag_gvg2(sd->bl.m)) {
				if (map->foreachinrange(mob->count_sub, &sd->bl, skill->get_splash(skill_id, skill_lv), BL_MOB,
				                        MOBID_EMPELIUM, MOBID_S_EMPEL_1, MOBID_S_EMPEL_2)) {
					char output[128];
					sprintf(output, "%s", msg_txt(883)); /* TODO official response */ // You are too close to a stone or emperium to do this skill
					clif->messagecolor_self(sd->fd, COLOR_RED, output);
					return 0;
				}
			}
			if( sd->spiritball > 0 )
				sd->spiritball_old = require.spiritball = sd->spiritball;
			else {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case WM_GREAT_ECHO: {
				int count;
				count = skill->check_pc_partner(sd, skill_id, &skill_lv, skill->get_splash(skill_id,skill_lv), 0);
				if( count < 1 ) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_NEED_HELPER, 0, 0);
					return 0;
				} else
					require.sp -= require.sp * 20 * count / 100; //  -20% each W/M in the party.
			}
			break;
		case SO_FIREWALK:
		case SO_ELECTRICWALK: // Can't be casted until you've walked all cells.
			if( sc && sc->data[SC_PROPERTYWALK] &&
			   sc->data[SC_PROPERTYWALK]->val3 < skill->get_maxcount(sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case SO_EL_CONTROL:
			if( !sd->status.ele_id || !sd->ed ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_EL_SUMMON, 0, 0);
				return 0;
			}
			break;
		case RETURN_TO_ELDICASTES:
			if( pc_ismadogear(sd) ) { //Cannot be used if Mado is equipped.
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case CR_REFLECTSHIELD:
			if( sc && sc->data[SC_KYOMU] && rnd()%100 < 5 * sc->data[SC_KYOMU]->val1 ){
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case KO_KAHU_ENTEN:
		case KO_HYOUHU_HUBUKI:
		case KO_KAZEHU_SEIRAN:
		case KO_DOHU_KOUKAI:
			if (sd->charm_type == skill->get_ele(skill_id, skill_lv) && sd->charm_count >= MAX_SPIRITCHARM) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_SUMMON, 0, 0);
				return 0;
			}
			break;
		case KO_KAIHOU:
		case KO_ZENKAI:
			if (sd->charm_type == CHARM_TYPE_NONE || sd->charm_count <= 0) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_SUMMON, 0, 0);
				return 0;
			}
			break;
		case KO_JYUMONJIKIRI:
			if (sd->weapontype1 != W_FIST && (sd->weapontype2 != W_FIST || sd->has_shield != W_FIST)) {
				return 1;
			} else {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case SJ_FULLMOONKICK:
			if (!(sc != NULL && sc->data[SC_NEWMOON] != NULL)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case SJ_SOLARBURST:
			if( !(sc != NULL && sc->data[SC_COMBOATTACK] != NULL && sc->data[SC_COMBOATTACK]->val1 == SJ_PROMINENCEKICK) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_COMBOSKILL, SJ_PROMINENCEKICK, 0);
				return 0;
			}
			break;
		case SJ_NOVAEXPLOSING:
		case SJ_GRAVITYCONTROL:
		case SJ_STAREMPEROR:
		case SJ_BOOKOFCREATINGSTAR:
		case SJ_BOOKOFDIMENSION:
		case SP_SOULDIVISION:
		case SP_SOULEXPLOSION:
			if (!map_flag_vs(sd->bl.m)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case SP_SWHOO:
			if (!(sc != NULL && sc->data[SC_USE_SKILL_SP_SPA] != NULL))
				return 0;
			break;
		case SP_KAUTE: // Fail if below 30% MaxHP.
			if (st->hp < 30 * st->max_hp / 100) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL, 0, 0);
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
	PRAGMA_GCC46(GCC diagnostic pop)

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch(require.state) {
		case ST_HIDING:
			if(!(sc && sc->option&OPTION_HIDE)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_CLOAKING:
			if(!pc_iscloaking(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_HIDDEN:
			if(!pc_ishiding(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_RIDING:
			if (!pc_isridingpeco(sd) && !pc_isridingdragon(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_FALCON:
			if(!pc_isfalcon(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_CARTBOOST:
			if(!(sc && sc->data[SC_CARTBOOST])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			FALLTHROUGH
		case ST_CART:
			if(!pc_iscarton(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CART, 0, 0);
				return 0;
			}
			break;
		case ST_SHIELD:
			if (!sd->has_shield) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_SIGHT:
			if(!(sc && sc->data[SC_SIGHT])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_EXPLOSIONSPIRITS:
			if(!(sc && sc->data[SC_EXPLOSIONSPIRITS])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_EXPLOSIONSPIRITS, 0, 0);
				return 0;
			}
			break;
		case ST_RECOV_WEIGHT_RATE:
			if(battle_config.natural_heal_weight_rate <= 100 && sd->weight*100/sd->max_weight >= (unsigned int)battle_config.natural_heal_weight_rate) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_MOVE_ENABLE:
			if (sc && sc->data[SC_COMBOATTACK] && sc->data[SC_COMBOATTACK]->val1 == skill_id)
				sd->ud.canmove_tick = timer->gettick(); //When using a combo, cancel the can't move delay to enable the skill. [Skotlex]

			if (!unit->can_move(&sd->bl)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_WATER:
			if (sc && (sc->data[SC_DELUGE] || sc->data[SC_NJ_SUITON]))
				break;
			if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKWATER))
				break;
			clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
			return 0;
		case ST_RIDINGDRAGON:
			if( !pc_isridingdragon(sd) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_DRAGON, 0, 0);
				return 0;
			}
			break;
		case ST_WUG:
			if( !pc_iswug(sd) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_RIDINGWUG:
			if( !pc_isridingwug(sd) ){
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_MADO:
			if( !pc_ismadogear(sd) ) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_MADOGEAR, 0, 0);
				return 0;
			}
			break;
		case ST_ELEMENTALSPIRIT:
			if(!sd->ed) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_EL_SUMMON, 0, 0);
				return 0;
			}
			break;
		case ST_POISONINGWEAPON:
			if (!(sc && sc->data[SC_POISONINGWEAPON])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_GC_POISONINGWEAPON, 0, 0);
				return 0;
			}
			break;
		case ST_ROLLINGCUTTER:
			if (!(sc && sc->data[SC_ROLLINGCUTTER])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CONDITION, 0, 0);
				return 0;
			}
			break;
		case ST_MH_FIGHTING:
			if (!(sc && sc->data[SC_STYLE_CHANGE] && sc->data[SC_STYLE_CHANGE]->val2 == MH_MD_FIGHTING)){
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			FALLTHROUGH
		case ST_MH_GRAPPLING:
			if (!(sc && sc->data[SC_STYLE_CHANGE] && sc->data[SC_STYLE_CHANGE]->val2 == MH_MD_GRAPPLING)){
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			FALLTHROUGH
		case ST_PECO:
			if (!pc_isridingpeco(sd)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_QD_SHOT_READY:
			if (!(sc != NULL && sc->data[SC_QD_SHOT_READY])) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_SUNSTANCE:
			if (!(sc != NULL && (sc->data[SC_SUNSTANCE] != NULL || sc->data[SC_UNIVERSESTANCE] != NULL))) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_MOONSTANCE:
			if (!(sc != NULL && (sc->data[SC_LUNARSTANCE] != NULL || sc->data[SC_UNIVERSESTANCE] != NULL))) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_STARSTANCE:
			if (!(sc != NULL && (sc->data[SC_STARSTANCE] != NULL || sc->data[SC_UNIVERSESTANCE] != NULL))) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
		case ST_UNIVERSESTANCE:
			if (!(sc != NULL && sc->data[SC_UNIVERSESTANCE] != NULL)) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
				return 0;
			}
			break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)

	if(require.mhp > 0 && get_percentage(st->hp, st->max_hp) > require.mhp) {
		//mhp is the max-hp-requirement, that is,
		//you must have this % or less of HP to cast it.
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0, 0);
		return 0;
	}

	if (require.msp > 0 && get_percentage(st->sp, st->max_sp) > require.msp) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT, 0, 0);
		return 0;
	}

	if( require.weapon && !pc_check_weapontype(sd,require.weapon) ) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_THIS_WEAPON, 0, 0);
		return 0;
	}

	if (skill->check_condition_required_equip(sd, skill_id, skill_lv) != 0)
		return 0;

	if (require.sp > 0 && st->sp < (unsigned int)require.sp && sd->auto_cast_current.type == AUTOCAST_NONE) { // Auto-cast skills don't consume SP.
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT, 0, 0);
		return 0;
	}

	if( require.zeny > 0 && sd->status.zeny < require.zeny ) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_MONEY, 0, 0);
		return 0;
	}

	if (require.spiritball > 0) {
		switch(skill_id) {
		case SP_SOULGOLEM:
		case SP_SOULSHADOW:
		case SP_SOULFALCON:
		case SP_SOULFAIRY:
		case SP_SOULCURSE:
		case SP_SPA:
		case SP_SHA:
		case SP_SWHOO:
		case SP_SOULUNITY:
		case SP_SOULDIVISION:
		case SP_SOULREAPER:
		case SP_SOULEXPLOSION:
		case SP_KAUTE:
			if (sd->soulball < require.spiritball) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_SPIRITS, require.spiritball, 0);
				return 0;
			}
			break;
		default:
			if (sd->spiritball < require.spiritball) {
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_SPIRITS, require.spiritball, 0);
			return 0;
			}
			break;
		}
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

static int skill_check_condition_castbegin_off_unknown(struct status_change *sc, uint16 *skill_id)
{
	return -1;
}

static int skill_check_condition_castbegin_mount_unknown(struct status_change *sc, uint16 *skill_id)
{
	return 0;
}

static int skill_check_condition_castbegin_madogear_unknown(struct status_change *sc, uint16 *skill_id)
{
	return 0;
}

static int skill_check_condition_castbegin_unknown(struct status_change *sc, uint16 *skill_id)
{
	return -1;
}

/**
 * Checks if a skill's item requirements are fulfilled.
 *
 * @param sd The character who casts the skill.
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return 0 on success or 1 in case of error.
 *
 **/
static int skill_check_condition_required_items(struct map_session_data *sd, int skill_id, int skill_lv)
{
	nullpo_retr(1, sd);

	struct skill_condition req = skill->get_requirement(sd, skill_id, skill_lv);

	if (skill->get_item_any_flag(skill_id, skill_lv)) {
		for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
			if (req.itemid[i] == 0)
				continue;

			int inv_idx = pc->search_inventory(sd, req.itemid[i]);

			if (inv_idx == INDEX_NOT_FOUND)
				continue;

			if ((req.amount[i] > 0 && sd->status.inventory[inv_idx].amount >= req.amount[i])
			    || (req.amount[i] == 0 && sd->status.inventory[inv_idx].amount > 0)) {
				return 0;
			}
		}
	}

	/**
	 * Find first missing item and show skill failed message if item any-flag is false
	 * or item any-flag check didn't find an item with sufficient amount.
	 *
	 **/
	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		if (req.itemid[i] == 0)
			continue;

		int inv_idx = pc->search_inventory(sd, req.itemid[i]);

		if (inv_idx == INDEX_NOT_FOUND || sd->status.inventory[inv_idx].amount < req.amount[i]) {
			useskill_fail_cause cause = USESKILL_FAIL_NEED_ITEM;

			switch (skill_id) {
			case NC_SILVERSNIPER:
			case NC_MAGICDECOY:
				cause = USESKILL_FAIL_STUFF_INSUFFICIENT;
				break;
			default:
				switch (req.itemid[i]) {
				case ITEMID_RED_GEMSTONE:
					cause = USESKILL_FAIL_REDJAMSTONE;
					break;
				case ITEMID_BLUE_GEMSTONE:
					cause = USESKILL_FAIL_BLUEJAMSTONE;
					break;
				case ITEMID_HOLY_WATER:
					cause = USESKILL_FAIL_HOLYWATER;
					break;
				case ITEMID_ANSILA:
					cause = USESKILL_FAIL_ANCILLA;
					break;
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
					FALLTHROUGH
				default:
					clif->skill_fail(sd, skill_id, cause, max(1, req.amount[i]), req.itemid[i]);
					return 1;
				}

				break;
			}

			clif->skill_fail(sd, skill_id, cause, 0, 0);
			return 1;
		}
	}

	return 0;
}

/**
 * Checks if a skill has item requirements.
 *
 * @param sd The character who casts the skill.
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return True if skill has item requirements, otherwise false.
 *
 **/
static bool skill_items_required(struct map_session_data *sd, int skill_id, int skill_lv)
{
	nullpo_retr(false, sd);

	struct skill_condition req = skill->get_requirement(sd, skill_id, skill_lv);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		if (req.itemid[i] != 0)
			return true;
	}

	return false;
}

static int skill_check_condition_castend(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	struct skill_condition require;
	struct status_data *st;
	int i;

	nullpo_ret(sd);

	if (sd->chat_id != 0)
		return 0;

	if (((sd->auto_cast_current.itemskill_conditions_checked || !sd->auto_cast_current.itemskill_check_conditions)
	    && sd->auto_cast_current.type == AUTOCAST_ITEM) || sd->auto_cast_current.type == AUTOCAST_IMPROVISE) {
		return 1;
	}

	if ((sd->auto_cast_current.type == AUTOCAST_ABRA || sd->auto_cast_current.type == AUTOCAST_IMPROVISE) && sd->auto_cast_current.skill_id == skill_id)
		return 1;

	if (pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL) && sd->auto_cast_current.type != AUTOCAST_ITEM) {
		// GMs don't override the AUTOCAST_ITEM check, otherwise they can use items without them being consumed!
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

	if (pc_is90overweight(sd) && sd->auto_cast_current.type != AUTOCAST_ITEM) { // Skill casting items ignore the overweight restriction.
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_WEIGHTOVER, 0, 0);
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
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
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
						clif->skill_fail(sd, skill_id, USESKILL_FAIL_SUMMON, 0, 0);
						return 0;
					}
				}
			}
			break;
		case KO_ZANZOU: {
				int c = 0;
				i = map->foreachinmap(skill->check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, MOBID_KO_KAGE, skill_id, &c);
				if( c >= skill->get_maxcount(skill_id,skill_lv) || c != i) {
					clif->skill_fail(sd, skill_id, USESKILL_FAIL_LEVEL, 0, 0);
					return 0;
				}
			}
			break;
		default:
			if (!skill->check_condition_castend_unknown(sd, &skill_id, &skill_lv))
				break;
			return 0;
	}

	st = &sd->battle_status;

	require = skill->get_requirement(sd,skill_id,skill_lv);

	if( require.hp > 0 && st->hp <= (unsigned int)require.hp) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT, 0, 0);
		return 0;
	}

	if( require.weapon && !pc_check_weapontype(sd,require.weapon) ) {
		clif->skill_fail(sd, skill_id, USESKILL_FAIL_THIS_WEAPON, 0, 0);
		return 0;
	}

	if( require.ammo ) { //Skill requires stuff equipped in the arrow slot.
		if((i=sd->equip_index[EQI_AMMO]) < 0 || !sd->inventory_data[i] ) {
			if( require.ammo&1<<8 )
				clif->skill_fail(sd, skill_id, USESKILL_FAIL_CANONBALL, 0, 0);
			else
				clif->arrow_fail(sd,0);
			return 0;
		} else if( sd->status.inventory[i].amount < require.ammo_qty ) {
			char e_msg[100];
			sprintf(e_msg, msg_txt(884), // Skill Failed. [%s] requires %dx %s.
						skill->get_desc(skill_id),
						require.ammo_qty,
						itemdb_jname(sd->status.inventory[i].nameid));
			clif->messagecolor_self(sd->fd, COLOR_RED, e_msg);
			return 0;
		}
		if ((require.ammo & (1 << sd->inventory_data[i]->subtype)) == 0 || !battle->check_arrows(sd)) { // Ammo type check.
			clif->arrow_fail(sd, 0); // "Please equip the proper ammunition first."
			return 0;
		}
	}

	bool items_required = skill->items_required(sd, skill_id, skill_lv);

	if (items_required && skill->check_condition_required_items(sd, skill_id, skill_lv) != 0)
		return 0;

	return 1;
}

static bool skill_check_condition_castend_unknown(struct map_session_data *sd, uint16 *skill_id, uint16 *skill_lv)
{
	return false;
}

/**
 * Gets the array index of the first required item with sufficient amount.
 *
 * @param sd The character who casts the skill.
 * @param skill_id The skill's ID.
 * @param skill_lv The skill's level.
 * @return A number greater than or equal to 0 on success, otherwise INDEX_NOT_FOUND (-1).
 *
 **/
static int skill_get_any_item_index(struct map_session_data *sd, int skill_id, int skill_lv)
{
	nullpo_retr(INDEX_NOT_FOUND, sd);

	int any_item_index = INDEX_NOT_FOUND;

	if (skill->get_item_any_flag(skill_id, skill_lv)) {
		struct skill_condition req = skill->get_requirement(sd, skill_id, skill_lv);

		for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
			if (req.itemid[i] == 0)
				continue;

			int inv_idx = pc->search_inventory(sd, req.itemid[i]);

			if (inv_idx == INDEX_NOT_FOUND)
				continue;

			if (req.amount[i] == 0 || sd->status.inventory[inv_idx].amount >= req.amount[i]) {
				any_item_index = i;
				break;
			}
		}
	}

	return any_item_index;
}

// type&2: consume items (after skill was used)
// type&1: consume the others (before skill was used)
static int skill_consume_requirement(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv, short type)
{
	struct skill_condition req;

	nullpo_ret(sd);

	if ((!sd->auto_cast_current.itemskill_check_conditions && sd->auto_cast_current.type == AUTOCAST_ITEM)
	    || sd->auto_cast_current.type == AUTOCAST_IMPROVISE) {
		return 1;
	}

	req = skill->get_requirement(sd,skill_id,skill_lv);

	if (type&1) {
		switch( skill_id ) {
			case CG_TAROTCARD: // TarotCard will consume sp in skill_cast_nodamage_id [Inkfish]
			case MC_IDENTIFY:
				req.sp = 0;
				break;
			case WZ_EARTHSPIKE:
				if (sd->sc.count > 0 && sd->sc.data[SC_EARTHSCROLL] != NULL) // If Earth Spike Scroll is used while SC_EARTHSCROLL is active, 10 SP are consumed. [Kenpachi]
					req.sp = 10;

				break;
			default:
				if (sd->auto_cast_current.type != AUTOCAST_NONE) // Auto-cast skills don't consume SP.
					req.sp = 0;

				break;
		}

		if(req.hp || req.sp)
			status_zap(&sd->bl, req.hp, req.sp);

		if (req.spiritball > 0) {
			switch(skill_id) {
			case SP_SOULGOLEM:
			case SP_SOULSHADOW:
			case SP_SOULFALCON:
			case SP_SOULFAIRY:
			case SP_SOULCURSE:
			case SP_SPA:
			case SP_SHA:
			case SP_SWHOO:
			case SP_SOULUNITY:
			case SP_SOULDIVISION:
			case SP_SOULREAPER:
			case SP_SOULEXPLOSION:
			case SP_KAUTE:
				pc->delsoulball(sd, req.spiritball, false);
				break;
			default:
				pc->delspiritball(sd, req.spiritball, 0);
				break;
			}
		}
			

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
		int n;

		if( !sc->count )
			sc = NULL;

		bool items_required = skill->items_required(sd, skill_id, skill_lv);
		int any_item_index = INDEX_NOT_FOUND;

		if (items_required)
			any_item_index = skill->get_any_item_index(sd, skill_id, skill_lv);

		for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE && items_required; i++) {
			if( !req.itemid[i] )
				continue;

			if (any_item_index != INDEX_NOT_FOUND && any_item_index != i)
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

static struct skill_condition skill_get_requirement(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	struct skill_condition req;
	struct status_data *st;
	struct status_change *sc;
	int i,hp_rate,sp_rate, sp_skill_rate_bonus = 100;
	uint16 idx;

	memset(&req,0,sizeof(req));

	if( !sd )
		return req;

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
		case SU_HIDE:
		case SJ_LUNARSTANCE:
		case SJ_STARSTANCE:
		case SJ_UNIVERSESTANCE:
		case SJ_SUNSTANCE:
			if (sc && sc->data[skill->get_sc_type(skill_id)])
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

	req.state = skill->dbs->db[idx].state[skill_lv - 1];

	req.mhp = skill->dbs->db[idx].mhp[skill_lv-1];

	req.msp = skill->get_msp(skill_id, skill_lv);

	req.weapon = skill->dbs->db[idx].weapon;

	req.ammo_qty = skill->dbs->db[idx].ammo_qty[skill_lv-1];
	if (req.ammo_qty)
		req.ammo = skill->dbs->db[idx].ammo;

	if (req.ammo == 0 && skill_id != 0 && skill->isammotype(sd, skill_id, skill_lv)) {
		//Assume this skill is using the weapon, therefore it requires arrows.
		req.ammo = 0xFFFFFFFF; //Enable use on all ammo types.
		req.ammo_qty = 1;
	}

	for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++ ) {
		switch( skill_id ) {
			case AM_CALLHOMUN:
				if (sd->status.hom_id) //Don't delete items when hom is already out.
					continue;
				break;
			case AB_ADORAMUS:
				if (itemid_isgemstone(skill->get_itemid(skill_id, i)) && skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 2) != 0)
					continue;
				break;
			case WL_COMET:
				if (itemid_isgemstone(skill->get_itemid(skill_id, i)) && skill->check_pc_partner(sd, skill_id, &skill_lv, 1, 0) != 0)
					continue;
				break;
			default:
			{
				if (skill->get_requirement_item_unknown(sc, sd, &skill_id, &skill_lv, &idx, &i))
					continue;
				break;
			}
		}

		int amount;

		if ((amount = skill->get_itemqty(skill_id, i, skill_lv)) >= 0) {
			req.itemid[i] = skill->get_itemid(skill_id, i);
			req.amount[i] = amount;
		}

		if ((amount = skill->get_equip_amount(skill_id, i, skill_lv)) > 0) {
			req.equip_id[i] = skill->get_equip_id(skill_id, i);
			req.equip_amount[i] = amount;
		}

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
				req.itemid[i] = ITEMID_SPECIAL_ALLOY_TRAP;
				req.amount[i] = 1;
			}
			break;
		}
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

static bool skill_get_requirement_off_unknown(struct status_change *sc, uint16 *skill_id)
{
	return false;
}

static bool skill_get_requirement_item_unknown(struct status_change *sc, struct map_session_data *sd, uint16 *skill_id, uint16 *skill_lv, uint16 *idx, int *i)
{
	return false;
}

static void skill_get_requirement_unknown(struct status_change *sc, struct map_session_data *sd, uint16 *skill_id, uint16 *skill_lv, struct skill_condition *req)
{
}

/*==========================================
 * Does cast-time reductions based on dex, item bonuses and config setting
 *------------------------------------------*/
static int skill_castfix(struct block_list *bl, uint16 skill_id, uint16 skill_lv)
{
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
static int skill_castfix_sc(struct block_list *bl, int time)
{
	struct status_change *sc = status->get_sc(bl);

	if( time < 0 )
		return 0;
	nullpo_ret(bl);

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
		if (sc->data[SC_SKF_CAST] != NULL)
			time -= time * sc->data[SC_SKF_CAST]->val1 / 100;
		if (sc->data[SC_IZAYOI])
			time -= time * 50 / 100;
	}
	time = max(time, 0);

	//ShowInfo("Castime castfix_sc = %d\n",time);
	return time;
}

static int skill_vfcastfix(struct block_list *bl, double time, uint16 skill_id, uint16 skill_lv)
{
#ifdef RENEWAL_CAST
	struct status_change *sc = status->get_sc(bl);
	struct map_session_data *sd = BL_CAST(BL_PC,bl);
	int fixed = skill->get_fixed_cast(skill_id, skill_lv), fixcast_r = 0, varcast_r = 0, i = 0;

	if( time < 0 )
		return 0;
	nullpo_ret(bl);

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
						FALLTHROUGH
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
		if (sc->data[SC_SKF_CAST] != NULL)
			VARCAST_REDUCTION(sc->data[SC_SKF_CAST]->val1);
		if (sc->data[SC_SOULFAIRY] != NULL)
			VARCAST_REDUCTION(sc->data[SC_SOULFAIRY]->val3);

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
		if (sc->data[SC_HEAT_BARREL])
			fixcast_r = max(fixcast_r, sc->data[SC_HEAT_BARREL]->val2);

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
static int skill_delay_fix(struct block_list *bl, uint16 skill_id, uint16 skill_lv)
{
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
		case SJ_PROMINENCEKICK:
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

	if (sc != NULL && sc->data[SC_SOULDIVISION] != NULL)
		time += time * sc->data[SC_SOULDIVISION]->val2 / 100;

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

static void skill_brandishspear_first(struct square *tc, enum unit_dir dir, int16 x, int16 y)
{
	nullpo_retv(tc);

	if (dir == UNIT_DIR_NORTH) {
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
	} else if (dir == UNIT_DIR_WEST) {
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
	} else if (dir == UNIT_DIR_SOUTH) {
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
	} else if (dir == UNIT_DIR_EAST) {
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
	} else if (dir == UNIT_DIR_NORTHWEST) {
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
	} else if (dir == UNIT_DIR_SOUTHWEST) {
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
	} else if (dir == UNIT_DIR_SOUTHEAST) {
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
	} else if (dir == UNIT_DIR_NORTHEAST) {
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

static void skill_brandishspear_dir(struct square *tc, enum unit_dir dir, int are)
{
	nullpo_retv(tc);
	Assert_retv(dir >= UNIT_DIR_FIRST && dir < UNIT_DIR_MAX);

	for (int c = 0; c < 5; c++) {
		tc->val1[c] += dirx[dir] * are;
		tc->val2[c] += diry[dir] * are;
	}
}

static void skill_brandishspear(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int64 tick, int flag)
{
	int c,n=4;
	struct square tc;
	int x, y;

	nullpo_retv(bl);
	x = bl->x;
	y = bl->y;
	enum unit_dir dir = map->calc_dir(src, x, y);
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
static void skill_repairweapon(struct map_session_data *sd, int idx)
{
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

	if (idx == 0xFFFF || idx == -1) // No item selected ('Cancel' clicked)
		return;
	if (idx < 0 || idx >= sd->status.inventorySize)
		return; //Invalid index??

	item = &target_sd->status.inventory[idx];
	if( item->nameid <= 0 || (item->attribute & ATTR_BROKEN) == 0 )
		return; //Again invalid item....

	if (item->card[0] == CARD0_PET)
		return;

	if( sd != target_sd && !battle->check_range(&sd->bl,&target_sd->bl, skill->get_range2(&sd->bl, sd->menuskill_id,sd->menuskill_val2) ) ){
		clif->item_repaireffect(sd,idx,1);
		return;
	}

	if ( target_sd->inventory_data[idx]->type == IT_WEAPON )
		material = materials[ target_sd->inventory_data[idx]->wlv - 1 ]; // Lv1/2/3/4 weapons consume 1 Iron Ore/Iron/Steel/Rough Oridecon
	else
		material = materials[2]; // Armors consume 1 Steel
	if (pc->search_inventory(sd,material) == INDEX_NOT_FOUND) {
		clif->skill_fail(sd, sd->menuskill_id, USESKILL_FAIL_LEVEL, 0, 0);
		return;
	}

	clif->skill_nodamage(&sd->bl,&target_sd->bl,sd->menuskill_id,1,1);

	item->attribute |= ATTR_BROKEN;
	item->attribute ^= ATTR_BROKEN; /* clear broken state */

	clif->equipList(target_sd);

	pc->delitem(sd, pc->search_inventory(sd, material), 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME); // FIXME: is this the correct reason flag?

	clif->item_repaireffect(sd,idx,0);

	if( sd != target_sd )
		clif->item_repaireffect(target_sd,idx,0);
}

/*==========================================
 * Item Appraisal
 *------------------------------------------*/
static void skill_identify(struct map_session_data *sd, int idx)
{
	int flag=1;

	nullpo_retv(sd);
	sd->state.workinprogress = 0;
	if (idx >= 0 && idx < sd->status.inventorySize) {
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
static void skill_weaponrefine(struct map_session_data *sd, int idx)
{
	nullpo_retv(sd);

	if (idx >= 0 && idx < sd->status.inventorySize) {
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
				clif->skill_fail(sd, sd->menuskill_id, USESKILL_FAIL_LEVEL, 0, 0);
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

			per = refine->get_refine_chance(ditem->wlv, (int)item->refine, REFINE_CHANCE_TYPE_NORMAL) * 10;

			// Aegis leaked formula. [malufett]
			if (sd->status.class == JOB_MECHANIC_T)
				per += 100;
			else
				per += 5 * (sd->status.job_level - 50);

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
				clif->inventoryList(sd);
				clif->refine(sd->fd,0,idx,item->refine);
				if (ep)
					pc->equipitem(sd,idx,ep);
				clif->misceffect(&sd->bl,3);
				if(item->refine == 10 &&
					item->card[0] == CARD0_FORGE &&
					(int)MakeDWord(item->card[2],item->card[3]) == sd->status.char_id)
				{ // Fame point system [DracoRPG]
					switch (ditem->wlv) {
					case 1:
						pc->addfame(sd, RANKTYPE_BLACKSMITH, 1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
						break;
					case 2:
						pc->addfame(sd, RANKTYPE_BLACKSMITH, 25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
						break;
					case 3:
						pc->addfame(sd, RANKTYPE_BLACKSMITH, 1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
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
static int skill_autospell(struct map_session_data *sd, uint16 skill_id)
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
static int skill_sit_count(struct block_list *bl, va_list ap)
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

static int skill_sit_in(struct block_list *bl, va_list ap)
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
		status->calc_regen_rate(bl, &sd->regen);
	}

	return 0;
}

static int skill_sit_out(struct block_list *bl, va_list ap)
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
		status->calc_regen_rate(bl, &sd->regen);
	}
	return 0;
}

static int skill_sit(struct map_session_data *sd, int type)
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
		clif->sc_load(&sd->bl, sd->bl.id, SELF, status->get_sc_icon(SC_SIT), 0, 0, 0);
	} else {
		clif->sc_end(&sd->bl, sd->bl.id, SELF, status->get_sc_icon(SC_SIT));
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
static int skill_frostjoke_scream(struct block_list *bl, va_list ap)
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
static void skill_unitsetmapcell(struct skill_unit *src, uint16 skill_id, uint16 skill_lv, cell_t cell, bool flag)
{
	int range = skill->get_unit_range(skill_id,skill_lv);
	int x,y;

	nullpo_retv(src);
	for( y = src->bl.y - range; y <= src->bl.y + range; ++y )
		for( x = src->bl.x - range; x <= src->bl.x + range; ++x )
			map->list[src->bl.m].setcell(src->bl.m, x, y, cell, flag);
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_attack_area(struct block_list *bl, va_list ap)
{
	struct block_list *src,*dsrc;
	int atk_type,skill_id,skill_lv,flag,type;
	int64 tick;

	nullpo_ret(bl);

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
static int skill_clear_group(struct block_list *bl, int flag)
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
		skill->del_unitgroup(group[i]);
	return count;
}

/*==========================================
 * Returns the first element field found [Skotlex]
 *------------------------------------------*/
static struct skill_unit_group *skill_locate_element_field(struct block_list *bl)
{
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
static int skill_graffitiremover(struct block_list *bl, va_list ap)
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

static int skill_greed(struct block_list *bl, va_list ap)
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
static int skill_detonator(struct block_list *bl, va_list ap)
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
					skill->trap_do_splash(bl, su->group->skill_id, su->group->skill_lv, su->group->bl_flag | BL_SKILL | ~BCT_SELF, su->group->tick);
					break;
				default:
					skill->trap_do_splash(bl, su->group->skill_id, su->group->skill_lv, su->group->bl_flag, su->group->tick);
			}
			clif->changetraplook(bl, UNT_USED_TRAPS);
			su->group->limit = DIFF_TICK32(timer->gettick(),su->group->tick) +
				(unit_id == UNT_TALKIEBOX ? 5000 : (unit_id == UNT_CLUSTERBOMB || unit_id == UNT_ICEBOUNDTRAP? 2500 : (unit_id == UNT_FIRINGTRAP ? 0 : 1500)) );
			su->group->unit_id = UNT_USED_TRAPS;
			break;
	}
	return 0;
}

/**
 * Rebellion's Bind Trap explosion
 * @author [Cydh]
 */
static int skill_bind_trap(struct block_list *bl, va_list ap) {
	struct skill_unit *su = NULL;
	struct block_list *src = va_arg(ap, struct block_list*);

	nullpo_ret(bl);
	nullpo_ret(src);

	if (bl->type != BL_SKILL || (su = BL_CAST(BL_SKILL, bl)) != NULL || (su->group) != NULL)
		return 0;
	if (su->group->unit_id != UNT_B_TRAP || su->group->src_id != src->id)
		return 0;

	map->foreachinrange(skill->trap_splash, bl, su->range, BL_CHAR, bl, su->group->tick);
	clif->changetraplook(bl, UNT_USED_TRAPS);
	su->group->unit_id = UNT_USED_TRAPS;
	su->group->limit = DIFF_TICK32(timer->gettick(), su->group->tick) + 500;
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_cell_overlap(struct block_list *bl, va_list ap)
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
		case RL_FIRE_RAIN:
			if (skill->get_unit_flag(su->group->skill_id) & UF_REMOVEDBYFIRERAIN) {
				skill->delunit(su);
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
static int skill_chastle_mob_changetarget(struct block_list *bl, va_list ap)
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

/**
 * Does final adjustments (e.g. count enemies affected by splash) then runs trap splash function (skill_trap_splash).
 *
 * @param bl : trap skill unit's bl
 * @param skill_id : Trap Skill ID
 * @param skill_lv : Trap Skill Level
 * @param bl_flag : Flag representing units affected by this trap
 * @param tick : tick related to this trap
 */
static void skill_trap_do_splash(struct block_list *bl, uint16 skill_id, uint16 skill_lv, int bl_flag, int64 tick)
{
	int enemy_count = 0;

	if (skill->get_nk(skill_id) & NK_SPLASHSPLIT) {
		enemy_count = map->foreachinrange(skill->area_sub, bl, skill->get_splash(skill_id, skill_lv), BL_CHAR, bl, skill_id, skill_lv, tick, BCT_ENEMY, skill->area_sub_count);
		enemy_count = max(1, enemy_count); // Don't let enemy_count be 0 when spliting trap damage
	}

	map->foreachinrange(skill->trap_splash, bl, skill->get_splash(skill_id, skill_lv), bl_flag, bl, tick, enemy_count);
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_trap_splash(struct block_list *bl, va_list ap)
{
	struct block_list *src = va_arg(ap, struct block_list *);
	int64 tick = va_arg(ap, int64);
	struct skill_unit *src_su = NULL;
	struct skill_unit_group *sg;
	struct block_list *ss;
	int enemy_count = va_arg(ap, int);

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
		case UNT_B_TRAP:
			if (battle->check_target(ss, bl, sg->target_flag & ~BCT_SELF) > 0)
				skill->castend_damage_id(ss, bl, sg->skill_id, sg->skill_lv, tick, SD_ANIMATION | SD_LEVEL | SD_SPLASH | 1);
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
			skill->attack(skill->get_type(sg->skill_id, sg->skill_lv), ss, src, bl, sg->skill_id, sg->skill_lv, tick, enemy_count);
			break;
	}
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_enchant_elemental_end(struct block_list *bl, int type)
{
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

static bool skill_check_cloaking(struct block_list *bl, struct status_change_entry *sce)
{
	bool wall = true;

	nullpo_retr(false, bl);
	if( (bl->type == BL_PC && battle_config.pc_cloak_check_type&1)
	 || (bl->type != BL_PC && battle_config.monster_cloak_check_type&1)
	) {
		//Check for walls.
		static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
		static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
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
static bool skill_can_cloak(struct map_session_data *sd)
{
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
static int skill_check_cloaking_end(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd = BL_CAST(BL_PC, bl);

	if (sd && sd->sc.data[SC_CLOAKING] && !skill->can_cloak(sd))
		status_change_end(bl, SC_CLOAKING, INVALID_TIMER);

	return 0;
}

static bool skill_check_camouflage(struct block_list *bl, struct status_change_entry *sce)
{
	bool wall = true;

	nullpo_retr(false, bl);
	if( bl->type == BL_PC ) { //Check for walls.
		static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
		static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
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

static bool skill_check_shadowform(struct block_list *bl, int64 damage, int hit)
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
static struct skill_unit *skill_initunit(struct skill_unit_group *group, int idx, int x, int y, int val1, int val2)
{
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
	su->visible = true;

	if (skill->get_inf2(group->skill_id) & INF2_HIDDEN_TRAP
		&& ((battle_config.trap_visibility == 1 && map_flag_vs(group->map)) // invisible in PvP/GvG
			|| battle_config.trap_visibility == 2 // always invisible
	)) {
	 	su->visible = false;
	}

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
static int skill_delunit(struct skill_unit *su)
{
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
	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
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
	PRAGMA_GCC46(GCC diagnostic pop)

	clif->skill_delunit(su);

	su->group=NULL;
	map->delblock(&su->bl); // don't free yet
	map->deliddb(&su->bl);
	idb_remove(skill->unit_db, su->bl.id);
	if(--group->alive_count==0)
		skill->del_unitgroup(group);

	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
/// Returns the target skill_unit_group or NULL if not found.
static struct skill_unit_group *skill_id2group(int group_id)
{
	return (struct skill_unit_group*)idb_get(skill->group_db, group_id);
}

/// Returns a new group_id that isn't being used in skill->group_db.
/// Fatal error if nothing is available.
static int skill_get_new_group_id(void)
{
	if (skill->unit_group_newid > MAX_SKILL_ID && skill->id2group(skill->unit_group_newid) == NULL)
		return skill->unit_group_newid++;// available

	{
		// find next id
		int base_id = skill->unit_group_newid;
		while (base_id != ++skill->unit_group_newid) {
			if (skill->unit_group_newid <= MAX_SKILL_ID)
				skill->unit_group_newid = MAX_SKILL_ID + 1;
			if (skill->id2group(skill->unit_group_newid) == NULL)
				return skill->unit_group_newid++;// available
		}
		// full loop, nothing available
		ShowFatalError("skill_get_new_group_id: All ids are taken. Exiting...");
		exit(1);
	}
}

static struct skill_unit_group *skill_initunitgroup(struct block_list *src, int count, uint16 skill_id, uint16 skill_lv, int unit_id, int limit, int interval)
{
	struct skill_unit_group* group;
	int i;

	if(!(skill_id && skill_lv)) return 0;

	nullpo_retr(NULL, src);

	struct unit_data *ud;

	if (src->type == BL_NPC)
		ud = unit->bl2ud2(src);
	else
		ud = unit->bl2ud(src);

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
		skill->del_unitgroup(ud->skillunit[j]);
		//Since elements must have shifted, we use the last slot.
		i = MAX_SKILLUNITGROUP-1;
	}

	group              = ers_alloc(skill->unit_ers, struct skill_unit_group);
	group->src_id      = src->id;
	group->party_id    = status->get_party_id(src);
	group->guild_id    = status->get_guild_id(src);
	group->bg_id       = bg->team_get_id(src);
	group->clan_id     = clan->get_id(src);
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
static int skill_delunitgroup(struct skill_unit_group *group)
{
	struct block_list* src;
	struct unit_data *ud;
	int i,j;
	struct map_session_data *sd = NULL;

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
static int skill_clear_unitgroup(struct block_list *src)
{
	struct unit_data *ud = unit->bl2ud(src);

	nullpo_ret(ud);

	while (ud->skillunit[0])
		skill->del_unitgroup(ud->skillunit[0]);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
static struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl, struct skill_unit_group *group, int64 tick)
{
	int i,j=-1,s,id;
	struct unit_data *ud;
	struct skill_unit_group_tickset *set;

	nullpo_ret(bl);
	nullpo_ret(group);
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
static int skill_unit_timer_sub_onplace(struct block_list *bl, va_list ap)
{
	struct skill_unit* su;
	struct skill_unit_group* group;
	int64 tick;

	su = va_arg(ap,struct skill_unit *);
	nullpo_ret(su);
	group = su->group;
	tick = va_arg(ap,int64);

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
static int skill_unit_timer_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct skill_unit* su;
	struct skill_unit_group* group;
	int64 tick = va_arg(ap,int64);
	bool dissonance;
	struct block_list* bl;

	su = DB->data2ptr(data);
	nullpo_ret(su);
	group = su->group;
	bl = &su->bl;

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
				FALLTHROUGH
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
					item_tmp.nameid = group->item_id ? group->item_id : ITEMID_BOOBY_TRAP;
					item_tmp.identify = 1;
					map->addflooritem(bl, &item_tmp, 1, bl->m, bl->x, bl->y, 0, 0, 0, 0, false);
				}
				skill->delunit(su);
			}
			break;

			case UNT_WARP_ACTIVE:
				// warp portal opens (morph to a UNT_WARP_WAITING cell)
				group->unit_id = skill->get_unit_id(group->skill_id, group->skill_lv, 1); // UNT_WARP_WAITING
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
				skill->trap_do_splash(bl, group->skill_id, group->skill_lv, group->bl_flag, tick);
				group->limit = DIFF_TICK32(tick,group->tick)+1500;
				su->limit = DIFF_TICK32(tick,group->tick)+1500;
				group->unit_id = UNT_USED_TRAPS;
			break;

			case UNT_FEINTBOMB: {
				struct block_list *src = map->id2bl(group->src_id);
				if( src ) {
					map->foreachinrange(skill->area_sub, &su->bl, su->range, skill->splash_target(src), src, SC_FEINTBOMB, group->skill_lv, tick, BCT_ENEMY|SD_ANIMATION|1, skill->castend_damage_id);
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

			case UNT_B_TRAP:
				{
					struct block_list* src;
					if (group->item_id && su->val2 <= 0 && (src = map->id2bl(group->src_id)) && src->type == BL_PC) {
						struct item item_tmp;
						memset(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = group->item_id;
						item_tmp.identify = 1;
						map->addflooritem(src, &item_tmp, 1, bl->m, bl->x, bl->y, 0, 0, 0, 4, false);
					}
					skill->delunit(su);
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
			case UNT_B_TRAP:
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
static int skill_unit_timer(int tid, int64 tick, int id, intptr_t data)
{
	GUARD_MAP_LOCK

	map->freeblock_lock();

	skill->unit_db->foreach(skill->unit_db, skill->unit_timer_sub, tick);

	map->freeblock_unlock();

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_unit_move_sub(struct block_list *bl, va_list ap)
{
	struct skill_unit *su = NULL;
	struct skill_unit_group *group = NULL;

	struct block_list* target = va_arg(ap,struct block_list*);
	int64 tick = va_arg(ap,int64);
	int flag = va_arg(ap,int);

	bool dissonance;
	uint16 skill_id;
	int i;

	nullpo_ret(target);
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
static int skill_unit_move(struct block_list *bl, int64 tick, int flag)
{
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
static int skill_unit_move_unit_group(struct skill_unit_group *group, int16 m, int16 dx, int16 dy)
{
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

	if (group->unit_id == UNT_ICEWALL || group->unit_id == UNT_WALLOFTHORN || group->unit_id == UNT_BOOKOFCREATINGSTAR)
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
static int skill_can_produce_mix(struct map_session_data *sd, int nameid, int trigger, int qty)
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
			int x = 0;
			for (int y = 0; y < sd->status.inventorySize; y++)
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
static int skill_produce_mix(struct map_session_data *sd, uint16 skill_id, int nameid, int slot1, int slot2, int slot3, int qty)
{
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
			for (i = 0; i < sd->status.inventorySize; i++ ) {
				if( sd->status.inventory[i].nameid == nameid ) {
					if( sd->status.inventory[i].amount >= data->stack.amount ) {
#if PACKETVER >= 20090729
						clif->msgtable(sd, MSG_RUNESTONE_MAKEERROR_OVERCOUNT);
#endif
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
						FALLTHROUGH
					case ITEMID_ISA:
					case ITEMID_WYRD:
						D -= 500; //Rank B
						FALLTHROUGH
					case ITEMID_NAUTHIZ:
					case ITEMID_URUZ:
						D -= 500; //Rank A
						FALLTHROUGH
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
					qty = ~(5 + rnd()%5) + 1; // FIXME[Haru]: This smells, if anyone knows the intent, please rewrite the expression in a more clear form.

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
						qty = ~qty + 1; // FIXME[Haru]: This smells. If anyone knows the intent, please rewrite the expression in a more clear form.
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

	if ((sd->job & JOBL_BABY) != 0) //if it's a Baby Class
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
			if (itemdb_wlv(nameid) >= 3 && ((ele? 1 : 0) + sc) >= 3) // Fame point system [DracoRPG]
				pc->addfame(sd, RANKTYPE_BLACKSMITH, 10); // Success to forge a lv3 weapon with 3 additional ingredients = +10 fame point
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

			if (fame != 0 && (skill_id == AM_PHARMACY || skill_id == AM_TWILIGHT1 || skill_id == AM_TWILIGHT2 || skill_id == AM_TWILIGHT3)) {
				pc->addfame(sd, RANKTYPE_ALCHEMIST, fame);
			}
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
							pc_setglobalreg(sd, script->add_variable("COOK_MASTERY"),sd->cook_mastery + ( 1 << ( (skill->dbs->produce_db[idx].itemlv - 11) / 2 ) ) * 5);
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
								map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
							}
							k++;
						}
					}
					break;
				}
			if (k) {
#if PACKETVER >= 20091013
				clif->msgtable_skill(sd, skill_id, MSG_SKILL_SUCCESS);
#endif
				return 1;
			}
		} else if (tmp_item.amount) { //Success
			if((flag = pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
				clif->additem(sd,0,0,flag);
				map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
			}
#if PACKETVER >= 20091013
			if (skill_id == GN_MIX_COOKING || skill_id == GN_MAKEBOMB || skill_id ==  GN_S_PHARMACY)
				clif->msgtable_skill(sd, skill_id, MSG_SKILL_SUCCESS);
#endif
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
					else if( rate < 100) i = 2+rnd()%1; // FIXME[Haru]: This '%1' is certainly not intended. If anyone knows the purpose, please rewrite this code.
					else if( rate < 250 ) i = 1;
					else if( rate < 500 ) i = 0;
					tmp_item.nameid = compensation[i];
					tmp_item.amount = qty;
					tmp_item.identify = 1;
					if( pc->additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE) ) {
						clif->additem(sd,0,0,flag);
						map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
					}
#if PACKETVER >= 20091013
					clif->msgtable_skill(sd, skill_id, MSG_SKILL_FAIL);
#endif
				}
				break;
			case GN_MAKEBOMB:
			case GN_S_PHARMACY:
			case GN_CHANGEMATERIAL:
#if PACKETVER >= 20091013
				clif->msgtable_skill(sd, skill_id, MSG_SKILL_FAIL);
#endif
				break;
			default:
				if( skill->dbs->produce_db[idx].itemlv > 10 && skill->dbs->produce_db[idx].itemlv <= 20 )
				{ //Cooking items.
					clif->specialeffect(&sd->bl, 609, AREA);
					if( sd->cook_mastery > 0 )
						pc_setglobalreg(sd, script->add_variable("COOK_MASTERY"), sd->cook_mastery - ( 1 << ((skill->dbs->produce_db[idx].itemlv - 11) / 2) ) - ( ( ( 1 << ((skill->dbs->produce_db[idx].itemlv - 11) / 2) ) >> 1 ) * 3 ));
				}
		}
	}
	return 0;
}

static int skill_arrow_create(struct map_session_data *sd, int nameid)
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
			map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
		}
	}

	return 0;
}

static int skill_poisoningweapon(struct map_session_data *sd, int nameid)
{
	sc_type type;
	int chance, i;
	nullpo_ret(sd);
	if (nameid <= 0 || (i = pc->search_inventory(sd,nameid)) == INDEX_NOT_FOUND || pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME)) {
		clif->skill_fail(sd, GC_POISONINGWEAPON, USESKILL_FAIL_LEVEL, 0, 0);
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
			clif->skill_fail(sd, GC_POISONINGWEAPON, USESKILL_FAIL_LEVEL, 0, 0);
			return 0;
	}

	status_change_end(&sd->bl, SC_POISONINGWEAPON, INVALID_TIMER); // Status must be forced to end so that a new poison will be applied if a player decides to change poisons. [Rytech]
	chance = 2 + 2 * sd->menuskill_val; // 2 + 2 * skill_lv
	sc_start4(&sd->bl, &sd->bl, SC_POISONINGWEAPON, 100, pc->checkskill(sd, GC_RESEARCHNEWPOISON), //in Aegis it store the level of GC_RESEARCHNEWPOISON in val1
		type, chance, 0, skill->get_time(GC_POISONINGWEAPON, sd->menuskill_val));

	return 0;
}

static void skill_toggle_magicpower(struct block_list *bl, uint16 skill_id, int skill_lv)
{
	struct status_change *sc = status->get_sc(bl);

	// non-offensive and non-magic skills do not affect the status
	if ((skill->get_nk(skill_id) & NK_NO_DAMAGE) != 0 || (skill->get_type(skill_id, skill_lv) & BF_MAGIC) == 0)
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

static int skill_magicdecoy(struct map_session_data *sd, int nameid)
{
	int x, y, i, class_ = 0, skill_id;
	struct mob_data *md;
	nullpo_ret(sd);
	skill_id = sd->menuskill_val;

	if (nameid <= 0 || !itemdb_is_element(nameid) || !skill_id
	 || (i = pc->search_inventory(sd, nameid)) == INDEX_NOT_FOUND
	 || pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME) != 0
	) {
		clif->skill_fail(sd, NC_MAGICDECOY, USESKILL_FAIL_LEVEL, 0, 0);
		return 0;
	}

	// Spawn Position
	pc->delitem(sd, i, 1, 0, DELITEM_NORMAL, LOG_TYPE_CONSUME); // FIXME: is this intended to be there twice?
	x = sd->sc.comet_x;
	y = sd->sc.comet_y;
	sd->sc.comet_x = sd->sc.comet_y = 0;
	sd->menuskill_val = 0;

	switch (nameid) {
	case ITEMID_SCARLET_PTS:
		class_ = MOBID_MAGICDECOY_FIRE;
		break;
	case ITEMID_INDIGO_PTS:
		class_ = MOBID_MAGICDECOY_WATER;
		break;
	case ITEMID_LIME_GREEN_PTS:
		class_ = MOBID_MAGICDECOY_WIND;
		break;
	case ITEMID_YELLOW_WISH_PTS:
		class_ = MOBID_MAGICDECOY_EARTH;
		break;
	}

	md = mob->once_spawn_sub(&sd->bl, sd->bl.m, x, y, sd->status.name, class_, "", SZ_SMALL, AI_NONE, 0);
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
static int skill_spellbook(struct map_session_data *sd, int nameid)
{
	int i, max_preserve, skill_id, point;
	struct status_change *sc;

	nullpo_ret(sd);

	sc = status->get_sc(&sd->bl);
	status_change_end(&sd->bl, SC_STOP, INVALID_TIMER);

	for(i=SC_SPELLBOOK1; i <= SC_SPELLBOOK7; i++) if( sc && !sc->data[i] ) break;
	if( i > SC_SPELLBOOK7 )
	{
		clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING, 0, 0);
		return 0;
	}

	ARR_FIND(0,MAX_SKILL_SPELLBOOK_DB,i,skill->dbs->spellbook_db[i].nameid == nameid); // Search for information of this item
	if( i == MAX_SKILL_SPELLBOOK_DB ) return 0;

	if( !pc->checkskill(sd, (skill_id = skill->dbs->spellbook_db[i].skill_id)) )
	{ // User don't know the skill
		sc_start(&sd->bl, &sd->bl, SC_SLEEP, 100, 1, skill->get_time(WL_READING_SB, pc->checkskill(sd,WL_READING_SB)));
		clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP, 0, 0);
		return 0;
	}

	max_preserve = 4 * pc->checkskill(sd, WL_FREEZE_SP) + (status_get_int(&sd->bl) + sd->status.base_level) / 10;
	point = skill->dbs->spellbook_db[i].point;

	if( sc && sc->data[SC_READING_SB] ) {
		if( (sc->data[SC_READING_SB]->val2 + point) > max_preserve ) {
			clif->skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT, 0, 0);
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

static int skill_select_menu(struct map_session_data *sd, uint16 skill_id)
{
	int id, lv, prob, aslvl = 0, idx = 0;
	nullpo_ret(sd);

	if (sd->sc.data[SC_STOP]) {
		aslvl = sd->sc.data[SC_STOP]->val1;
		status_change_end(&sd->bl,SC_STOP,INVALID_TIMER);
	}

	idx = skill->get_index(skill_id);

	if (skill_id >= GS_GLITTERING || (id = sd->status.skill[idx].id) == 0
	    || sd->status.skill[idx].flag != SKILL_FLAG_PLAGIARIZED) {
		clif->skill_fail(sd, SC_AUTOSHADOWSPELL, 0, 0, 0);
		return 0;
	}

	lv = (aslvl + 1) / 2; // The level the skill will be autocasted
	lv = min(lv, sd->status.skill[idx].lv);

	if (skill->get_type(skill_id, lv) != BF_MAGIC) {
		clif->skill_fail(sd, SC_AUTOSHADOWSPELL, 0, 0, 0);
		return 0;
	}

	prob = (aslvl == 10) ? 15 : (32 - 2 * aslvl); // Probability at level 10 was increased to 15.
	sc_start4(&sd->bl,&sd->bl,SC__AUTOSHADOWSPELL,100,id,lv,prob,0,skill->get_time(SC_AUTOSHADOWSPELL,aslvl));
	return 0;
}

static int skill_elementalanalysis(struct map_session_data *sd, uint16 skill_lv, const struct itemlist *item_list)
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

		if (idx < 0 || idx >= sd->status.inventorySize
		 || (nameid = sd->status.inventory[idx].nameid) <= 0
		 || del_amount < 0 || del_amount > sd->status.inventory[idx].amount) {
			clif->skill_fail(sd, SO_EL_ANALYSIS, USESKILL_FAIL_LEVEL, 0, 0);
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
				clif->skill_fail(sd, SO_EL_ANALYSIS, USESKILL_FAIL_LEVEL, 0, 0);
				return 1;
		}

		if( pc->delitem(sd, idx, del_amount, 0, DELITEM_SKILLUSE, LOG_TYPE_CONSUME) ) {
			clif->skill_fail(sd, SO_EL_ANALYSIS, USESKILL_FAIL_LEVEL, 0, 0);
			return 1;
		}

		if( skill_lv == 2 && rnd()%100 < 25 ) {
			// At level 2 have a fail chance. You loose your items if it fails.
			clif->skill_fail(sd, SO_EL_ANALYSIS, USESKILL_FAIL_LEVEL, 0, 0);
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
				map->addflooritem(&sd->bl, &tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, 0, 0, 0, 0, false);
			}
		}

	}

	return 0;
}

static int skill_changematerial(struct map_session_data *sd, const struct itemlist *item_list)
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
							Assert_ret(idx >= 0 && idx < sd->status.inventorySize);
							amount = entry->amount;
							nameid = sd->status.inventory[idx].nameid;
							if (nameid > 0 && sd->status.inventory[idx].identify == 0) {
#if PACKETVER >= 20091013
								clif->msgtable_skill(sd, GN_CHANGEMATERIAL, MSG_SKILL_FAIL_MATERIAL_IDENTITY);
#endif
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
#if PACKETVER >= 20091013
	if (p == 0)
		clif->msgtable_skill(sd, GN_CHANGEMATERIAL, MSG_SKILL_RECIPE_NOTEXIST);
#endif
	return 0;
}

/**
 * for Royal Guard's LG_TRAMPLE
 **/
static int skill_destroy_trap(struct block_list *bl, va_list ap)
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
				skill->trap_do_splash(&su->bl, sg->skill_id, sg->skill_lv, sg->bl_flag | BL_SKILL | ~BCT_SELF, tick);
				break;
			case UNT_LANDMINE:
			case UNT_BLASTMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
			case UNT_CLUSTERBOMB:
				skill->trap_do_splash(&su->bl, sg->skill_id, sg->skill_lv, sg->bl_flag, tick);
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
static int skill_blockpc_end(int tid, int64 tick, int id, intptr_t data)
{
	struct map_session_data *sd = map->id2sd(id);
	struct skill_cd * cd = NULL;

	if (data <= 0 || data >= MAX_SKILL_DB)
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
static int skill_blockpc_start_(struct map_session_data *sd, uint16 skill_id, int tick)
{
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
			cd->entry[i]->total = tick;
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
	cd->entry[cd->cursor]->total = tick;
	cd->entry[cd->cursor]->skidx = idx;
	cd->entry[cd->cursor]->skill_id = skill_id;
	cd->entry[cd->cursor]->started = now;
	cd->entry[cd->cursor]->timer = timer->add(now+tick,skill->blockpc_end,sd->bl.id,idx);

	cd->cursor++;

	sd->blockskill[idx] = true;
	return 0;
}

// [orn]
static int skill_blockhomun_end(int tid, int64 tick, int id, intptr_t data)
{
	struct homun_data *hd = map->id2hd(id);
	if (data <= 0 || data >= MAX_SKILL_DB)
		return 0;
	if (hd != NULL)
		hd->blockskill[data] = 0;

	return 1;
}

// [orn]
static int skill_blockhomun_start(struct homun_data *hd, uint16 skill_id, int tick)
{
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
static int skill_blockmerc_end(int tid, int64 tick, int id, intptr_t data)
{
	struct mercenary_data *md = map->id2mc(id);
	if (data <= 0 || data >= MAX_SKILL_DB)
		return 0;
	if (md != NULL)
		md->blockskill[data] = 0;

	return 1;
}

static int skill_blockmerc_start(struct mercenary_data *md, uint16 skill_id, int tick)
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
static void skill_usave_add(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	struct skill_unit_save * sus = NULL;

	nullpo_retv(sd);
	if( idb_exists(skill->usave_db,sd->status.char_id) ) {
		idb_remove(skill->usave_db,sd->status.char_id);
	}

	CREATE( sus, struct skill_unit_save, 1 );
	idb_put( skill->usave_db, sd->status.char_id, sus );

	sus->skill_id = skill_id;
	sus->skill_lv = skill_lv;

	return;
}

static void skill_usave_trigger(struct map_session_data *sd)
{
	struct skill_unit_save * sus = NULL;

	nullpo_retv(sd);
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
static int skill_split_atoi(char *str, int *val)
{
	int i, j, step = 1;

	nullpo_ret(val);

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
static void skill_init_unit_layout(void)
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
		if (skill->dbs->db[i].unit_layout_type[0] != -1)
			continue;

		switch (skill->dbs->db[i].nameid) {
			case MG_FIREWALL:
			case WZ_ICEWALL:
			case WL_EARTHSTRAIN://Warlock
			case RL_FIRE_RAIN:
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
				skill->init_unit_layout_unknown(i, pos);
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
	skill->firerain_unit_pos = pos;
	for (i = 0; i < 8; i++) {
		skill->dbs->unit_layout[pos].count = 3;
		switch (i) {
		case 0:
		case 1:
		case 3:
		case 4:
		case 5:
		case 7:
			{
				static const int dx[] = { -1, 0, 1 };
				static const int dy[] = { 0, 0, 0 };
				memcpy(skill->dbs->unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy, dy, sizeof(dy));
			}
			break;
		case 2:
		case 6:
			{
				static const int dx[] = { 0, 0, 0 };
				static const int dy[] = { -1, 0, 1 };
				memcpy(skill->dbs->unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill->dbs->unit_layout[pos].dy, dy, sizeof(dy));
			}
			break;
		}
		pos++;
	}

}

static void skill_init_unit_layout_unknown(int skill_idx, int pos)
{
	Assert_retv(skill_idx >= 0 && skill_idx < MAX_SKILL_DB);
	ShowError("unknown unit layout at skill %d\n", skill->dbs->db[skill_idx].nameid);
}

static int skill_block_check(struct block_list *bl, sc_type type, uint16 skill_id)
{
	int inf = 0;
	struct status_change *sc = status->get_sc(bl);

	if( !sc || !bl || !skill_id )
		return 0; // Can do it

	inf = skill->get_inf2(skill_id);

	PRAGMA_GCC46(GCC diagnostic push)
	PRAGMA_GCC46(GCC diagnostic ignored "-Wswitch-enum")
	switch(type) {
	case SC_STASIS:
		if (inf & INF2_NO_STASIS)
			return 1; // Can't do it.
		break;
	case SC_KG_KAGEHUMI:
		if (inf & INF2_NO_KAGEHUMI)
			return 1;
		break;
	}
	PRAGMA_GCC46(GCC diagnostic pop)

	return 0;
}

static int skill_get_elemental_type(uint16 skill_id, uint16 skill_lv)
{
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
static void skill_cooldown_save(struct map_session_data *sd)
{
	int i;
	struct skill_cd *cd = NULL;
	int64 now = 0;

	nullpo_retv(sd);

	if ((cd = idb_get(skill->cd_db, sd->status.char_id)) == NULL)
		return;

	now = timer->gettick();

	for (i = 0; i < cd->cursor; i++) {
		if (battle_config.guild_skill_relog_delay == 1 && cd->entry[i]->skill_id > GD_SKILLBASE && cd->entry[i]->skill_id < GD_MAX)
			continue;

		cd->entry[i]->duration = DIFF_TICK32(cd->entry[i]->started + cd->entry[i]->duration, now);
		if (cd->entry[i]->timer != INVALID_TIMER) {
			timer->delete(cd->entry[i]->timer, skill->blockpc_end);
			cd->entry[i]->timer = INVALID_TIMER;
		}
	}
}

/**
 * reload stored skill cooldowns when a player logs in.
 * @param   sd     the affected player structure
 */
static void skill_cooldown_load(struct map_session_data *sd)
{
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
		int64 remaining;

		if (battle_config.guild_skill_relog_delay == 2 && cd->entry[i]->skill_id >= GD_SKILLBASE && cd->entry[i]->skill_id < GD_MAX) {
			remaining = cd->entry[i]->started + cd->entry[i]->total - now;
			remaining = max(1, remaining); // expired cooldowns will be 1, so they'll expire in the normal way just after this.
		} else {
			cd->entry[i]->started = now;
			remaining = cd->entry[i]->duration;
		}
		cd->entry[i]->timer = timer->add(timer->gettick() + remaining, skill->blockpc_end, sd->bl.id, cd->entry[i]->skidx);
		sd->blockskill[cd->entry[i]->skidx] = true;
	}
}

static bool skill_parse_row_producedb(char *split[], int columns, int current)
{
// ProduceItemID,ItemLV,RequireSkill,Requireskill_lv,MaterialID1,MaterialAmount1,......
	int x,y;
	int i;

	nullpo_retr(false, split);
	i = atoi(split[0]);
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

static bool skill_parse_row_createarrowdb(char *split[], int columns, int current)
{
// SourceID,MakeID1,MakeAmount1,...,MakeID5,MakeAmount5
	int x,y;

	int i;
	nullpo_retr(false, split);
	i = atoi(split[0]);
	if( !i )
		return false;

	skill->dbs->arrow_db[current].nameid = i;

	for( x = 1, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_ARROW_RESOURCE; x += 2, y++ ) {
		skill->dbs->arrow_db[current].cre_id[y] = atoi(split[x]);
		skill->dbs->arrow_db[current].cre_amount[y] = atoi(split[x+1]);
	}

	return true;
}

static bool skill_parse_row_spellbookdb(char *split[], int columns, int current)
{
// skill_id,PreservePoints

	uint16 skill_id;
	int points;
	int nameid;

	nullpo_retr(false, split);
	skill_id = atoi(split[0]);
	points = atoi(split[1]);
	nameid = atoi(split[2]);

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

static bool skill_parse_row_improvisedb(char *split[], int columns, int current)
{
// SkillID,Rate
	uint16 skill_id;
	short j;

	nullpo_retr(false, split);
	skill_id = atoi(split[0]);
	j = atoi(split[1]);

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

static bool skill_parse_row_magicmushroomdb(char *split[], int column, int current)
{
// SkillID
	uint16 skill_id;

	nullpo_retr(false, split);
	skill_id = atoi(split[0]);
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

static bool skill_parse_row_abradb(char *split[], int columns, int current)
{
// skill_id,DummyName,RequiredHocusPocusLevel,Rate
	uint16 skill_id;
	nullpo_retr(false, split);
	skill_id = atoi(split[0]);
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

static bool skill_parse_row_changematerialdb(char *split[], int columns, int current)
{
// ProductID,BaseRate,MakeAmount1,MakeAmountRate1...,MakeAmount5,MakeAmountRate5
	int skill_id;
	short j;
	int x,y;

	nullpo_retr(false, split);
	skill_id = atoi(split[0]);
	j = atoi(split[1]);
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

/**
 * Enable Bard/Dancer to learn songs from their counterpart when spirit link
 * * @param *sd    the actual player gain the skills
 */
static void skill_add_bard_dancer_soullink_songs(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (sd->sc.count == 0 || sd->sc.data[SC_SOULLINK] == NULL || sd->sc.data[SC_SOULLINK]->val2 != SL_BARDDANCER)
		return;

	const int bard_song_skillid[4] = {BA_WHISTLE, BA_ASSASSINCROSS, BA_POEMBRAGI, BA_APPLEIDUN};
	const int dancer_song_skillid[4] = {DC_HUMMING, DC_DONTFORGETME, DC_FORTUNEKISS, DC_SERVICEFORYOU};

	STATIC_ASSERT(ARRAYLENGTH(bard_song_skillid) == ARRAYLENGTH(dancer_song_skillid), "bard_song_skillid and dancer_song_skillid must be the same size");

	int copy_from_index;
	int copy_to_index;
	for (int i = 0; i < ARRAYLENGTH(bard_song_skillid); i++) {
		if (sd->status.sex == SEX_MALE) {
			copy_from_index = skill->get_index(bard_song_skillid[i]);
			copy_to_index = skill->get_index(dancer_song_skillid[i]);
		} else {
			copy_from_index = skill->get_index(dancer_song_skillid[i]);
			copy_to_index = skill->get_index(bard_song_skillid[i]);
		}

		if (copy_from_index == 0 || copy_to_index == 0) {
			Assert_report("Linked bard/dancer skill not found");
			continue;
		}

		if (sd->status.skill[copy_from_index].lv < 10) // Copy only if the linked skill has been mastered
			continue;

		sd->status.skill[copy_to_index].id = skill->dbs->db[copy_to_index].nameid;
		sd->status.skill[copy_to_index].lv = sd->status.skill[copy_from_index].lv; // Set the level to the same as the linking skill
		sd->status.skill[copy_to_index].flag = SKILL_FLAG_TEMPORARY; // Tag it as a non-savable, non-uppable, bonus skill
	}
	return;
}

/**
 * Sets Level based configuration for skill groups from skill_db.conf [ Smokexyz/Hercules ]
 * @param *conf    pointer to config setting.
 * @param *arr     pointer to array to be set.
 */
static void skill_config_set_level(struct config_setting_t *conf, int *arr)
{
	int i=0;

	nullpo_retv(arr);
	if (config_setting_is_group(conf)) {
		for (i=0; i<MAX_SKILL_LEVEL; i++) {
			char level[6]; // enough to contain "Lv100" in case of custom MAX_SKILL_LEVEL
			sprintf(level, "Lv%d", i+1);
			libconfig->setting_lookup_int(conf, level, &arr[i]);
		}
	} else if (config_setting_is_array(conf)) {
		for (i=0; i<config_setting_length(conf) && i < MAX_SKILL_LEVEL; i++) {
			arr[i] = libconfig->setting_get_int_elem(conf, i);
		}
	} else {
		int val=libconfig->setting_get_int(conf);
		for(i=0; i<MAX_SKILL_LEVEL; i++) {
			arr[i] = val;
		}
	}
}

/**
 * Sets all values in a skill level array to a specified value [ Smokexyz/Hercules ]
 * @param *arr    pointer to array being parsed.
 * @param value   value being set for the array.
 * @return (void)
 */
static void skill_level_set_value(int *arr, int value)
{
	int i=0;

	nullpo_retv(arr);
	for(i=0; i<MAX_SKILL_LEVEL; i++) {
		arr[i] = value;
	}
}

/**
 * Validates a skill's ID when reading the skill DB.
 * If validating fails, the ID is set to 0.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the ID should be set it.
 * @param conf_index The 1-based index of the currently processed libconfig settings block.
 *
 **/
static void skill_validate_id(struct config_setting_t *conf, struct s_skill_db *sk, int conf_index)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->nameid = 0;

	int id;

	if (libconfig->setting_lookup_int(conf, "Id", &id) == CONFIG_FALSE)
		ShowError("%s: No skill ID specified in entry %d in %s! Skipping skill...\n",
			  __func__, conf_index, conf->file);
	else if (id <= 0)
		ShowError("%s: Invalid skill ID %d specified in entry %d in %s! Skipping skill...\n",
			  __func__, id, conf_index, conf->file);
	else if(skill->get_index(id) == 0)
		ShowError("%s: Skill ID %d in entry %d in %s is out of range, or within a reserved range (for guild, homunculus, mercenary or elemental skills)! Skipping skill...\n",
			  __func__, id, conf_index, conf->file);
	else if (*skill->get_name(id) != '\0')
		ShowError("%s: Duplicate skill ID %d in entry %d in %s! Skipping skill...\n",
			  __func__, id, conf_index, conf->file);
	else if (id >= MAX_SKILL_ID)
		ShowError("%s: Invalid skill ID %d specified in entry %d in %s! Skill id must be smaller than MAX_SKILL_ID (%d). Skipping skill...\n",
			  __func__, id, conf_index, conf->file, MAX_SKILL_ID);
	else
		sk->nameid = id;
}

/**
 * Validates if a skill's name contains invalid characters when reading the skill DB.
 *
 * @param name The name to validate.
 * @return True if the passed name is a NULL pointer or contains at least one invalid character, otherwise false.
 *
 **/
static bool skill_name_contains_invalid_character(const char *name)
{
	nullpo_retr(true, name);

	for (int i = 0; i < MAX_SKILL_NAME_LENGTH && name[i] != '\0'; i++) {
		if (ISALNUM(name[i]) == 0 && name[i] != '_')
			return true;
	}

	return false;
}

/**
 * Validates a skill's name when reading the skill DB.
 * If validating fails, the name is set to an enpty string.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the name should be set it.
 *
 **/
static void skill_validate_name(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	*sk->name = '\0';

	const char *name;

	if (libconfig->setting_lookup_string(conf, "Name", &name) == CONFIG_FALSE || *name == '\0')
		ShowError("%s: No name specified for skill ID %d in %s! Skipping skill...\n",
			  __func__, sk->nameid, conf->file);
	else if (strlen(name) >= sizeof(sk->name))
		ShowError("%s: Specified name %s for skill ID %d in %s is too long: %d! Maximum is %d. Skipping skill...\n",
			  __func__, name, sk->nameid, conf->file, (int)strlen(name), (int)sizeof(sk->name) - 1);
	else if (skill->name_contains_invalid_character(name))
		ShowError("%s: Specified name %s for skill ID %d in %s contains invalid characters! Allowed characters are letters, numbers and underscores. Skipping skill...\n",
			  __func__, name, sk->nameid, conf->file);
	else if (skill->name2id(name) != 0)
		ShowError("%s: Duplicate name %s for skill ID %d in %s! Skipping skill...\n",
			  __func__, name, sk->nameid, conf->file);
	else
		safestrncpy(sk->name, name, sizeof(sk->name));
}

/**
 * Validates a skill's maximum level when reading the skill DB.
 * If validating fails, the maximum level is set to 0.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the maximum level should be set it.
 *
 **/
static void skill_validate_max_level(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->max = 0;

	int max_level;

	if (libconfig->setting_lookup_int(conf, "MaxLevel", &max_level) == CONFIG_FALSE)
		ShowError("%s: No maximum level specified for skill ID %d in %s! Skipping skill...\n",
			  __func__, sk->nameid, conf->file);
	else if (max_level < 1 || max_level > MAX_SKILL_LEVEL)
		ShowError("%s: Invalid maximum level %d specified for skill ID %d in %s! Minimum is 1, maximum is %d. Skipping skill...\n",
			  __func__, max_level, sk->nameid, conf->file, MAX_SKILL_LEVEL);
	else
		sk->max = max_level;
}

/**
 * Validates a skill's description when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the description should be set it.
 *
 **/
static void skill_validate_description(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	*sk->desc = '\0';

	const char *description;

	if (libconfig->setting_lookup_string(conf, "Description", &description) == CONFIG_TRUE && *description != '\0') {
		if (strlen(description) >= sizeof(sk->desc))
			ShowWarning("%s: Specified description '%s' for skill ID %d in %s is too long: %d! Maximum is %d. Trimming...\n",
				    __func__, description, sk->nameid, conf->file, (int)strlen(description), (int)sizeof(sk->desc) - 1);

		safestrncpy(sk->desc, description, sizeof(sk->desc));
	}
}

/**
 * Validates a skill's range when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the range should be set it.
 *
 **/
static void skill_validate_range(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->range, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Range");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int range;

			if (libconfig->setting_lookup_int(t, lv, &range) == CONFIG_TRUE) {
				if (range >= SHRT_MIN && range <= SHRT_MAX)
					sk->range[i] = range;
				else
					ShowWarning("%s: Invalid range %d specified in level %d for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 0...\n",
						    __func__, range, i + 1, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
			}
		}

		return;
	}

	int range;

	if (libconfig->setting_lookup_int(conf, "Range", &range) == CONFIG_TRUE) {
		if (range >= SHRT_MIN && range <= SHRT_MAX)
			skill->level_set_value(sk->range, range);
		else
			ShowWarning("%s: Invalid range %d specified for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 0...\n",
				    __func__, range, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
	}
}

/**
 * Validates a skill's hit type when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the hit type should be set it.
 *
 **/
static void skill_validate_hittype(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->hit, BDT_NORMAL);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Hit");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			const char *hit_type;

			if (libconfig->setting_lookup_string(t, lv, &hit_type) == CONFIG_TRUE) {
				if (strcmpi(hit_type, "BDT_SKILL") == 0)
					sk->hit[i] = BDT_SKILL;
				else if (strcmpi(hit_type, "BDT_MULTIHIT") == 0)
					sk->hit[i] = BDT_MULTIHIT;
				else if (strcmpi(hit_type, "BDT_NORMAL") != 0)
					ShowWarning("%s: Invalid hit type %s specified in level %d for skill ID %d in %s! Defaulting to BDT_NORMAL...\n",
						    __func__, hit_type, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	const char *hit_type;

	if (libconfig->setting_lookup_string(conf, "Hit", &hit_type) == CONFIG_TRUE) {
		int hit = BDT_NORMAL;

		if (strcmpi(hit_type, "BDT_SKILL") == 0) {
			hit = BDT_SKILL;
		} else if (strcmpi(hit_type, "BDT_MULTIHIT") == 0) {
			hit = BDT_MULTIHIT;
		} else if (strcmpi(hit_type, "BDT_NORMAL") != 0) {
			ShowWarning("%s: Invalid hit type %s specified for skill ID %d in %s! Defaulting to BDT_NORMAL...\n",
				    __func__, hit_type, sk->nameid, conf->file);
			return;
		}

		skill->level_set_value(sk->hit, hit);
	}
}

/**
 * Validates a skill's types when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the types should be set it.
 *
 **/
static void skill_validate_skilltype(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->inf = INF_NONE;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SkillType");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			const char *skill_type = config_setting_name(tt);
			bool on = libconfig->setting_get_bool_real(tt);

			if (strcmpi(skill_type, "Enemy") == 0) {
				if (on)
					sk->inf |= INF_ATTACK_SKILL;
				else
					sk->inf &= ~INF_ATTACK_SKILL;
			} else if (strcmpi(skill_type, "Place") == 0) {
				if (on)
					sk->inf |= INF_GROUND_SKILL;
				else
					sk->inf &= ~INF_GROUND_SKILL;
			} else if (strcmpi(skill_type, "Self") == 0) {
				if (on)
					sk->inf |= INF_SELF_SKILL;
				else
					sk->inf &= ~INF_SELF_SKILL;
			} else if (strcmpi(skill_type, "Friend") == 0) {
				if (on)
					sk->inf |= INF_SUPPORT_SKILL;
				else
					sk->inf &= ~INF_SUPPORT_SKILL;
			} else if (strcmpi(skill_type, "Trap") == 0) {
				if (on)
					sk->inf |= INF_TARGET_TRAP;
				else
					sk->inf &= ~INF_TARGET_TRAP;
			} else if (strcmpi(skill_type, "Item") == 0) {
				if (on)
					sk->inf |= INF_ITEM_SKILL;
				else
					sk->inf &= ~INF_ITEM_SKILL;
			} else if (strcmpi(skill_type, "Passive") != 0) {
				ShowWarning("%s: Invalid skill type %s specified for skill ID %d in %s! Skipping type...\n",
					    __func__, skill_type, sk->nameid, conf->file);
			}
		}
	}
}

/**
 * Validates a skill's sub-types when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the sub-types should be set it.
 *
 **/
static void skill_validate_skillinfo(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->inf2 = INF2_NONE;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SkillInfo");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			const char *skill_info = config_setting_name(tt);
			bool on = libconfig->setting_get_bool_real(tt);

			if (strcmpi(skill_info, "Quest") == 0) {
				if (on)
					sk->inf2 |= INF2_QUEST_SKILL;
				else
					sk->inf2 &= ~INF2_QUEST_SKILL;
			} else if (strcmpi(skill_info, "NPC") == 0) {
				if (on)
					sk->inf2 |= INF2_NPC_SKILL;
				else
					sk->inf2 &= ~INF2_NPC_SKILL;
			} else if (strcmpi(skill_info, "Wedding") == 0) {
				if (on)
					sk->inf2 |= INF2_WEDDING_SKILL;
				else
					sk->inf2 &= ~INF2_WEDDING_SKILL;
			} else if (strcmpi(skill_info, "Spirit") == 0) {
				if (on)
					sk->inf2 |= INF2_SPIRIT_SKILL;
				else
					sk->inf2 &= ~INF2_SPIRIT_SKILL;
			} else if (strcmpi(skill_info, "Guild") == 0) {
				if (on)
					sk->inf2 |= INF2_GUILD_SKILL;
				else
					sk->inf2 &= ~INF2_GUILD_SKILL;
			} else if (strcmpi(skill_info, "Song") == 0) {
				if (on)
					sk->inf2 |= INF2_SONG_DANCE;
				else
					sk->inf2 &= ~INF2_SONG_DANCE;
			} else if (strcmpi(skill_info, "Ensemble") == 0) {
				if (on)
					sk->inf2 |= INF2_ENSEMBLE_SKILL;
				else
					sk->inf2 &= ~INF2_ENSEMBLE_SKILL;
			} else if (strcmpi(skill_info, "Trap") == 0) {
				if (on)
					sk->inf2 |= INF2_TRAP;
				else
					sk->inf2 &= ~INF2_TRAP;
			} else if (strcmpi(skill_info, "TargetSelf") == 0) {
				if (on)
					sk->inf2 |= INF2_TARGET_SELF;
				else
					sk->inf2 &= ~INF2_TARGET_SELF;
			} else if (strcmpi(skill_info, "NoCastSelf") == 0) {
				if (on)
					sk->inf2 |= INF2_NO_TARGET_SELF;
				else
					sk->inf2 &= ~INF2_NO_TARGET_SELF;
			} else if (strcmpi(skill_info, "PartyOnly") == 0) {
				if (on)
					sk->inf2 |= INF2_PARTY_ONLY;
				else
					sk->inf2 &= ~INF2_PARTY_ONLY;
			} else if (strcmpi(skill_info, "GuildOnly") == 0) {
				if (on)
					sk->inf2 |= INF2_GUILD_ONLY;
				else
					sk->inf2 &= ~INF2_GUILD_ONLY;
			} else if (strcmpi(skill_info, "NoEnemy") == 0) {
				if (on)
					sk->inf2 |= INF2_NO_ENEMY;
				else
					sk->inf2 &= ~INF2_NO_ENEMY;
			} else if (strcmpi(skill_info, "IgnoreLandProtector") == 0) {
				if (on)
					sk->inf2 |= INF2_NOLP;
				else
					sk->inf2 &= ~INF2_NOLP;
			} else if (strcmpi(skill_info, "Chorus") == 0) {
				if (on)
					sk->inf2 |= INF2_CHORUS_SKILL;
				else
					sk->inf2 &= ~INF2_CHORUS_SKILL;
			} else if (strcmpi(skill_info, "FreeCastNormal") == 0) {
				if (on)
					sk->inf2 |= INF2_FREE_CAST_NORMAL;
				else
					sk->inf2 &= ~INF2_FREE_CAST_NORMAL;
			} else if (strcmpi(skill_info, "FreeCastReduced") == 0) {
				if (on)
					sk->inf2 |= INF2_FREE_CAST_REDUCED;
				else
					sk->inf2 &= ~INF2_FREE_CAST_REDUCED;
			} else if (strcmpi(skill_info, "ShowSkillScale") == 0) {
				if (on)
					sk->inf2 |= INF2_SHOW_SKILL_SCALE;
				else
					sk->inf2 &= ~INF2_SHOW_SKILL_SCALE;
			} else if (strcmpi(skill_info, "AllowReproduce") == 0) {
				if (on)
					sk->inf2 |= INF2_ALLOW_REPRODUCE;
				else
					sk->inf2 &= ~INF2_ALLOW_REPRODUCE;
			} else if (strcmpi(skill_info, "HiddenTrap") == 0) {
				if (on)
					sk->inf2 |= INF2_HIDDEN_TRAP;
				else
					sk->inf2 &= ~INF2_HIDDEN_TRAP;
			} else if (strcmpi(skill_info, "IsCombo") == 0) {
				if (on)
					sk->inf2 |= INF2_IS_COMBO_SKILL;
				else
					sk->inf2 &= ~INF2_IS_COMBO_SKILL;
			} else if (strcmpi(skill_info, "BlockedByStasis") == 0) {
				if (on)
					sk->inf2 |= INF2_NO_STASIS;
				else
					sk->inf2 &= ~INF2_NO_STASIS;
			} else if (strcmpi(skill_info, "BlockedByKagehumi") == 0) {
				if (on)
					sk->inf2 |= INF2_NO_KAGEHUMI;
				else
					sk->inf2 &= ~INF2_NO_KAGEHUMI;
			} else if (strcmpi(skill_info, "RangeModByVulture") == 0) {
				if (on)
					sk->inf2 |= INF2_RANGE_VULTURE;
				else
					sk->inf2 &= ~INF2_RANGE_VULTURE;
			} else if (strcmpi(skill_info, "RangeModBySnakeEye") == 0) {
				if (on)
					sk->inf2 |= INF2_RANGE_SNAKEEYE;
				else
					sk->inf2 &= ~INF2_RANGE_SNAKEEYE;
			} else if (strcmpi(skill_info, "RangeModByShadowJump") == 0) {
				if (on)
					sk->inf2 |= INF2_RANGE_SHADOWJUMP;
				else
					sk->inf2 &= ~INF2_RANGE_SHADOWJUMP;
			} else if (strcmpi(skill_info, "RangeModByRadius") == 0) {
				if (on)
					sk->inf2 |= INF2_RANGE_RADIUS;
				else
					sk->inf2 &= ~INF2_RANGE_RADIUS;
			} else if (strcmpi(skill_info, "RangeModByResearchTrap") == 0) {
				if (on)
					sk->inf2 |= INF2_RANGE_RESEARCHTRAP;
				else
					sk->inf2 &= ~INF2_RANGE_RESEARCHTRAP;
			} else if (strcmpi(skill_info, "AllowPlagiarism") == 0) {
				if (on)
					sk->inf2 |= INF2_ALLOW_PLAGIARIZE;
				else
					sk->inf2 &= ~INF2_ALLOW_PLAGIARIZE;
			} else if (strcmpi(skill_info, "None") != 0) {
				ShowWarning("%s: Invalid sub-type %s specified for skill ID %d in %s! Skipping sub-type...\n",
					    __func__, skill_info, sk->nameid, conf->file);
			}
		}
	}
}

/**
 * Validates a skill's attack type when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the attack type should be set it.
 *
 **/
static void skill_validate_attacktype(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->skill_type, BF_NONE);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "AttackType");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			const char *attack_type;

			if (libconfig->setting_lookup_string(t, lv, &attack_type) == CONFIG_TRUE) {
				if (strcmpi(attack_type, "Weapon") == 0)
					sk->skill_type[i] = BF_WEAPON;
				else if (strcmpi(attack_type, "Magic") == 0)
					sk->skill_type[i] = BF_MAGIC;
				else if (strcmpi(attack_type, "Misc") == 0)
					sk->skill_type[i] = BF_MISC;
				else if (strcmpi(attack_type, "None") != 0)
					ShowWarning("%s: Invalid attack type %s specified in level %d for skill ID %d in %s! Defaulting to None...\n",
						    __func__, attack_type, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	const char *attack_type;

	if (libconfig->setting_lookup_string(conf, "AttackType", &attack_type) == CONFIG_TRUE) {
		int attack = BF_NONE;

		if (strcmpi(attack_type, "Weapon") == 0) {
			attack = BF_WEAPON;
		} else if (strcmpi(attack_type, "Magic") == 0) {
			attack = BF_MAGIC;
		} else if (strcmpi(attack_type, "Misc") == 0) {
			attack = BF_MISC;
		} else if (strcmpi(attack_type, "None") != 0) {
			ShowWarning("%s: Invalid attack type %s specified for skill ID %d in %s! Defaulting to None...\n",
				    __func__, attack_type, sk->nameid, conf->file);
			return;
		}

		skill->level_set_value(sk->skill_type, attack);
	}
}

/**
 * Validates a skill's element when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the element should be set it.
 *
 **/
static void skill_validate_element(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->element, ELE_NEUTRAL);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Element");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			const char *element;

			if (libconfig->setting_lookup_string(t, lv, &element) == CONFIG_TRUE) {
				if (strcmpi(element, "Ele_Weapon") == 0)
					sk->element[i] = -1;
				else if (strcmpi(element, "Ele_Endowed") == 0)
					sk->element[i] = -2;
				else if (strcmpi(element, "Ele_Random") == 0)
					sk->element[i] = -3;
				else if (!script->get_constant(element, &sk->element[i]))
					ShowWarning("%s: Invalid element %s specified in level %d for skill ID %d in %s! Defaulting to Ele_Neutral...\n",
						    __func__, element, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	const char *element;

	if (libconfig->setting_lookup_string(conf, "Element", &element) == CONFIG_TRUE) {
		int ele = ELE_NEUTRAL;

		if (strcmpi(element, "Ele_Weapon") == 0) {
			ele = -1;
		} else if (strcmpi(element, "Ele_Endowed") == 0) {
			ele = -2;
		} else if (strcmpi(element, "Ele_Random") == 0) {
			ele = -3;
		} else if (!script->get_constant(element, &ele)) {
			ShowWarning("%s: Invalid element %s specified for skill ID %d in %s! Defaulting to Ele_Neutral...\n",
				    __func__, element, sk->nameid, conf->file);
			return;
		}

		skill->level_set_value(sk->element, ele);
	}
}

/**
 * Validates a skill's damage types when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the damage types should be set it.
 *
 **/
static void skill_validate_damagetype(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->nk = NK_NONE;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "DamageType");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			const char *damage_type = config_setting_name(tt);
			bool on = libconfig->setting_get_bool_real(tt);

			if (strcmpi(damage_type, "NoDamage") == 0) {
				if (on)
					sk->nk |= NK_NO_DAMAGE;
				else
					sk->nk &= ~NK_NO_DAMAGE;
			} else if (strcmpi(damage_type, "SplashArea") == 0) {
				if (on)
					sk->nk |= NK_SPLASH_ONLY;
				else
					sk->nk &= ~NK_SPLASH_ONLY;
			} else if (strcmpi(damage_type, "SplitDamage") == 0) {
				if (on)
					sk->nk |= NK_SPLASHSPLIT;
				else
					sk->nk &= ~NK_SPLASHSPLIT;
			} else if (strcmpi(damage_type, "IgnoreCards") == 0) {
				if (on)
					sk->nk |= NK_NO_CARDFIX_ATK;
				else
					sk->nk &= ~NK_NO_CARDFIX_ATK;
			} else if (strcmpi(damage_type, "IgnoreElement") == 0) {
				if (on)
					sk->nk |= NK_NO_ELEFIX;
				else
					sk->nk &= ~NK_NO_ELEFIX;
			} else if (strcmpi(damage_type, "IgnoreDefense") == 0) {
				if (on)
					sk->nk |= NK_IGNORE_DEF;
				else
					sk->nk &= ~NK_IGNORE_DEF;
			} else if (strcmpi(damage_type, "IgnoreFlee") == 0) {
				if (on)
					sk->nk |= NK_IGNORE_FLEE;
				else
					sk->nk &= ~NK_IGNORE_FLEE;
			} else if (strcmpi(damage_type, "IgnoreDefCards") == 0) {
				if (on)
					sk->nk |= NK_NO_CARDFIX_DEF;
				else
					sk->nk &= ~NK_NO_CARDFIX_DEF;
			} else {
				ShowWarning("%s: Invalid damage type %s specified for skill ID %d in %s! Skipping damage type...\n",
					    __func__, damage_type, sk->nameid, conf->file);
			}
		}
	}
}

/**
 * Validates a skill's splash range when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the splash range should be set it.
 *
 **/
static void skill_validate_splash_range(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->splash, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SplashRange");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int splash_range;

			if (libconfig->setting_lookup_int(t, lv, &splash_range) == CONFIG_TRUE) {
				if (splash_range >= SHRT_MIN && splash_range <= SHRT_MAX)
					sk->splash[i] = splash_range;
				else
					ShowWarning("%s: Invalid splash range %d specified in level %d for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 0...\n",
						    __func__, splash_range, i + 1, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
			}
		}

		return;
	}

	int splash_range;

	if (libconfig->setting_lookup_int(conf, "SplashRange", &splash_range) == CONFIG_TRUE) {
		if (splash_range >= SHRT_MIN && splash_range <= SHRT_MAX)
			skill->level_set_value(sk->splash, splash_range);
		else
			ShowWarning("%s: Invalid splash range %d specified for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 0...\n",
				    __func__, splash_range, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
	}
}

/**
 * Validates a skill's number of hits when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the number of hits should be set it.
 *
 **/
static void skill_validate_number_of_hits(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->num, 1);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "NumberOfHits");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int number_of_hits;

			if (libconfig->setting_lookup_int(t, lv, &number_of_hits) == CONFIG_TRUE) {
				if (number_of_hits >= SHRT_MIN && number_of_hits <= SHRT_MAX)
					sk->num[i] = number_of_hits;
				else
					ShowWarning("%s: Invalid number of hits %d specified in level %d for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 1...\n",
						    __func__, number_of_hits, i + 1, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
			}
		}

		return;
	}

	int number_of_hits;

	if (libconfig->setting_lookup_int(conf, "NumberOfHits", &number_of_hits) == CONFIG_TRUE) {
		if (number_of_hits >= SHRT_MIN && number_of_hits <= SHRT_MAX)
			skill->level_set_value(sk->num, number_of_hits);
		else
			ShowWarning("%s: Invalid number of hits %d specified for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 1...\n",
				    __func__, number_of_hits, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
	}
}

/**
 * Validates a skill's cast interruptibility when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the cast interruptibility should be set it.
 *
 **/
static void skill_validate_interrupt_cast(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->castcancel, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "InterruptCast");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int interrupt_cast;

			if (libconfig->setting_lookup_bool(t, lv, &interrupt_cast) == CONFIG_TRUE)
				sk->castcancel[i] = (interrupt_cast != 0) ? 1 : 0;
		}

		return;
	}

	int interrupt_cast;

	if (libconfig->setting_lookup_bool(conf, "InterruptCast", &interrupt_cast) == CONFIG_TRUE) {
		if (interrupt_cast != 0)
			skill->level_set_value(sk->castcancel, 1);
	}
}

/**
 * Validates a skill's cast defence rate when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the cast defence rate should be set it.
 *
 **/
static void skill_validate_cast_def_rate(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->cast_def_rate, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "CastDefRate");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int cast_def_rate;

			if (libconfig->setting_lookup_int(t, lv, &cast_def_rate) == CONFIG_TRUE) {
				if (cast_def_rate >= SHRT_MIN && cast_def_rate <= SHRT_MAX)
					sk->cast_def_rate[i] = cast_def_rate;
				else
					ShowWarning("%s: Invalid cast defence rate %d specified in level %d for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 0...\n",
						    __func__, cast_def_rate, i + 1, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
			}
		}

		return;
	}

	int cast_def_rate;

	if (libconfig->setting_lookup_int(conf, "CastDefRate", &cast_def_rate) == CONFIG_TRUE) {
		if (cast_def_rate >= SHRT_MIN && cast_def_rate <= SHRT_MAX)
			skill->level_set_value(sk->cast_def_rate, cast_def_rate);
		else
			ShowWarning("%s: Invalid cast defence rate %d specified for skill ID %d in %s! Minimum is %d, maximum is %d. Defaulting to 0...\n",
				    __func__, cast_def_rate, sk->nameid, conf->file, SHRT_MIN, SHRT_MAX);
	}
}

/**
 * Validates a skill's number of instances when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the number of instances should be set it.
 *
 **/
static void skill_validate_number_of_instances(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->maxcount, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SkillInstances");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int number_of_instances;

			if (libconfig->setting_lookup_int(t, lv, &number_of_instances) == CONFIG_TRUE) {
				if (number_of_instances >= 0 && number_of_instances <= MAX_SKILLUNITGROUP)
					sk->maxcount[i] = number_of_instances;
				else
					ShowWarning("%s: Invalid number of instances %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, number_of_instances, i + 1, sk->nameid, conf->file, MAX_SKILLUNITGROUP);
			}
		}

		return;
	}

	int number_of_instances;

	if (libconfig->setting_lookup_int(conf, "SkillInstances", &number_of_instances) == CONFIG_TRUE) {
		if (number_of_instances >= 0 && number_of_instances <= MAX_SKILLUNITGROUP)
			skill->level_set_value(sk->maxcount, number_of_instances);
		else
			ShowWarning("%s: Invalid number of instances %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
				    __func__, number_of_instances, sk->nameid, conf->file, MAX_SKILLUNITGROUP);
	}
}

/**
 * Validates a skill's number of knock back tiles when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the number of knock back tiles should be set it.
 *
 **/
static void skill_validate_knock_back_tiles(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->blewcount, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "KnockBackTiles");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int knock_back_tiles;

			if (libconfig->setting_lookup_int(t, lv, &knock_back_tiles) == CONFIG_TRUE) {
				if (knock_back_tiles >= 0)
					sk->blewcount[i] = knock_back_tiles;
				else
					ShowWarning("%s: Invalid number of knock back tiles %d specified in level %d for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
						    __func__, knock_back_tiles, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int knock_back_tiles;

	if (libconfig->setting_lookup_int(conf, "KnockBackTiles", &knock_back_tiles) == CONFIG_TRUE) {
		if (knock_back_tiles >= 0)
			skill->level_set_value(sk->blewcount, knock_back_tiles);
		else
			ShowWarning("%s: Invalid number of knock back tiles %d specified for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
				    __func__, knock_back_tiles, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's cast time when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the cast time should be set it.
 *
 **/
static void skill_validate_cast_time(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->cast, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "CastTime");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int cast_time;

			if (libconfig->setting_lookup_int(t, lv, &cast_time) == CONFIG_TRUE) {
				if (cast_time >= 0)
					sk->cast[i] = cast_time;
				else
					ShowWarning("%s: Invalid cast time %d specified in level %d for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
						    __func__, cast_time, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int cast_time;

	if (libconfig->setting_lookup_int(conf, "CastTime", &cast_time) == CONFIG_TRUE) {
		if (cast_time >= 0)
			skill->level_set_value(sk->cast, cast_time);
		else
			ShowWarning("%s: Invalid cast time %d specified for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
				    __func__, cast_time, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's after cast act delay when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the after cast act delay should be set it.
 *
 **/
static void skill_validate_act_delay(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->delay, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "AfterCastActDelay");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int act_delay;

			if (libconfig->setting_lookup_int(t, lv, &act_delay) == CONFIG_TRUE) {
				if (act_delay >= 0)
					sk->delay[i] = act_delay;
				else
					ShowWarning("%s: Invalid after cast act delay %d specified in level %d for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
						    __func__, act_delay, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int act_delay;

	if (libconfig->setting_lookup_int(conf, "AfterCastActDelay", &act_delay) == CONFIG_TRUE) {
		if (act_delay >= 0)
			skill->level_set_value(sk->delay, act_delay);
		else
			ShowWarning("%s: Invalid after cast act delay %d specified for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
				    __func__, act_delay, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's after cast walk delay when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the after cast walk delay should be set it.
 *
 **/
static void skill_validate_walk_delay(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->walkdelay, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "AfterCastWalkDelay");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int walk_delay;

			if (libconfig->setting_lookup_int(t, lv, &walk_delay) == CONFIG_TRUE) {
				if (walk_delay >= 0)
					sk->walkdelay[i] = walk_delay;
				else
					ShowWarning("%s: Invalid after cast walk delay %d specified in level %d for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
						    __func__, walk_delay, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int walk_delay;

	if (libconfig->setting_lookup_int(conf, "AfterCastWalkDelay", &walk_delay) == CONFIG_TRUE) {
		if (walk_delay >= 0)
			skill->level_set_value(sk->walkdelay, walk_delay);
		else
			ShowWarning("%s: Invalid after cast walk delay %d specified for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
				    __func__, walk_delay, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's stay duration when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the stay duration should be set it.
 *
 **/
static void skill_validate_skill_data1(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->upkeep_time, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SkillData1");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int skill_data1;

			if (libconfig->setting_lookup_int(t, lv, &skill_data1) == CONFIG_TRUE) {
				if (skill_data1 >= INFINITE_DURATION)
					sk->upkeep_time[i] = skill_data1;
				else
					ShowWarning("%s: Invalid stay duration %d specified in level %d for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
						    __func__, skill_data1, i + 1, sk->nameid, conf->file, INFINITE_DURATION);
			}
		}

		return;
	}

	int skill_data1;

	if (libconfig->setting_lookup_int(conf, "SkillData1", &skill_data1) == CONFIG_TRUE) {
		if (skill_data1 >= INFINITE_DURATION)
			skill->level_set_value(sk->upkeep_time, skill_data1);
		else
			ShowWarning("%s: Invalid stay duration %d specified for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
				    __func__, skill_data1, sk->nameid, conf->file, INFINITE_DURATION);
	}
}

/**
 * Validates a skill's effect duration when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the effect duration should be set it.
 *
 **/
static void skill_validate_skill_data2(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->upkeep_time2, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SkillData2");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int skill_data2;

			if (libconfig->setting_lookup_int(t, lv, &skill_data2) == CONFIG_TRUE) {
				if (skill_data2 >= INFINITE_DURATION)
					sk->upkeep_time2[i] = skill_data2;
				else
					ShowWarning("%s: Invalid effect duration %d specified in level %d for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
						    __func__, skill_data2, i + 1, sk->nameid, conf->file, INFINITE_DURATION);
			}
		}

		return;
	}

	int skill_data2;

	if (libconfig->setting_lookup_int(conf, "SkillData2", &skill_data2) == CONFIG_TRUE) {
		if (skill_data2 >= INFINITE_DURATION)
			skill->level_set_value(sk->upkeep_time2, skill_data2);
		else
			ShowWarning("%s: Invalid effect duration %d specified for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
				    __func__, skill_data2, sk->nameid, conf->file, INFINITE_DURATION);
	}
}

/**
 * Validates a skill's cooldown when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the cooldown should be set it.
 *
 **/
static void skill_validate_cooldown(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->cooldown, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "CoolDown");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int cooldown;

			if (libconfig->setting_lookup_int(t, lv, &cooldown) == CONFIG_TRUE) {
				if (cooldown >= 0)
					sk->cooldown[i] = cooldown;
				else
					ShowWarning("%s: Invalid cooldown %d specified in level %d for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
						    __func__, cooldown, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int cooldown;

	if (libconfig->setting_lookup_int(conf, "CoolDown", &cooldown) == CONFIG_TRUE) {
		if (cooldown >= 0)
			skill->level_set_value(sk->cooldown, cooldown);
		else
			ShowWarning("%s: Invalid cooldown %d specified for skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
				    __func__, cooldown, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's fixed cast time when reading the skill DB.
 * If RENEWAL_CAST is not defined, nothing is done.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the fixed cast time should be set it.
 *
 **/
static void skill_validate_fixed_cast_time(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

#ifdef RENEWAL_CAST
	skill->level_set_value(sk->fixed_cast, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "FixedCastTime");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int fixed_cast_time;

			if (libconfig->setting_lookup_int(t, lv, &fixed_cast_time) == CONFIG_TRUE) {
				if (fixed_cast_time >= INFINITE_DURATION)
					sk->fixed_cast[i] = fixed_cast_time;
				else
					ShowWarning("%s: Invalid fixed cast time %d specified in level %d for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
						    __func__, fixed_cast_time, i + 1, sk->nameid, conf->file, INFINITE_DURATION);
			}
		}

		return;
	}

	int fixed_cast_time;

	if (libconfig->setting_lookup_int(conf, "FixedCastTime", &fixed_cast_time) == CONFIG_TRUE) {
		if (fixed_cast_time >= INFINITE_DURATION)
			skill->level_set_value(sk->fixed_cast, fixed_cast_time);
		else
			ShowWarning("%s: Invalid fixed cast time %d specified for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
				    __func__, fixed_cast_time, sk->nameid, conf->file, INFINITE_DURATION);
	}
#else
#ifndef RENEWAL /** Check pre-RE skill DB for FixedCastTime. **/
	if (libconfig->setting_get_member(conf, "FixedCastTime") != NULL)
		ShowWarning("%s: Fixed cast time was specified for skill ID %d in %s without RENEWAL_CAST being defined! Skipping...\n", __func__, sk->nameid, conf->file);
#endif /** RENEWAL **/
#endif /** RENEWAL_CAST **/
}

/**
 * Validates a skill's cast time or delay options when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the cast time or delay options should be set it.
 * @param delay If true, the skill's delay options are validated, otherwise its cast time options.
 *
 **/
static void skill_validate_castnodex(struct config_setting_t *conf, struct s_skill_db *sk, bool delay)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(delay ? sk->delaynodex : sk->castnodex, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, delay ? "SkillDelayOptions" : "CastTimeOptions");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;
		int options = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			const char *value = config_setting_name(tt);
			bool on = libconfig->setting_get_bool_real(tt);

			if (strcmpi(value, "IgnoreDex") == 0) {
				if (on)
					options |= 1;
				else
					options &= ~1;
			} else if (strcmpi(value, "IgnoreStatusEffect") == 0) {
				if (on)
					options |= 2;
				else
					options &= ~2;
			} else if (strcmpi(value, "IgnoreItemBonus") == 0) {
				if (on)
					options |= 4;
				else
					options &= ~4;
			} else {
				const char *option_string = delay ? "skill delay" : "cast time";
				ShowWarning("%s: Invalid %s option %s specified for skill ID %d in %s! Skipping option...\n",
					    __func__, option_string, value, sk->nameid, conf->file);
			}
		}

		skill->level_set_value(delay ? sk->delaynodex : sk->castnodex, options);
	}
}

/**
 * Validates a skill's HP cost when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the HP cost should be set it.
 *
 **/
static void skill_validate_hp_cost(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->hp, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "HPCost");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int hp_cost;

			if (libconfig->setting_lookup_int(t, lv, &hp_cost) == CONFIG_TRUE) {
				if (hp_cost >= 0 && hp_cost <= battle_config.max_hp)
					sk->hp[i] = hp_cost;
				else
					ShowWarning("%s: Invalid HP cost %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, hp_cost, i + 1, sk->nameid, conf->file, battle_config.max_hp);
			}
		}

		return;
	}

	int hp_cost;

	if (libconfig->setting_lookup_int(conf, "HPCost", &hp_cost) == CONFIG_TRUE) {
		if (hp_cost >= 0 && hp_cost <= battle_config.max_hp)
			skill->level_set_value(sk->hp, hp_cost);
		else
			ShowWarning("%s: Invalid HP cost %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
				    __func__, hp_cost, sk->nameid, conf->file, battle_config.max_hp);
	}
}

/**
 * Validates a skill's SP cost when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the SP cost should be set it.
 *
 **/
static void skill_validate_sp_cost(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->sp, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SPCost");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int sp_cost;

			if (libconfig->setting_lookup_int(t, lv, &sp_cost) == CONFIG_TRUE) {
				if (sp_cost >= 0 && sp_cost <= battle_config.max_sp)
					sk->sp[i] = sp_cost;
				else
					ShowWarning("%s: Invalid SP cost %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, sp_cost, i + 1, sk->nameid, conf->file, battle_config.max_sp);
			}
		}

		return;
	}

	int sp_cost;

	if (libconfig->setting_lookup_int(conf, "SPCost", &sp_cost) == CONFIG_TRUE) {
		if (sp_cost >= 0 && sp_cost <= battle_config.max_sp)
			skill->level_set_value(sk->sp, sp_cost);
		else
			ShowWarning("%s: Invalid SP cost %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
				    __func__, sp_cost, sk->nameid, conf->file, battle_config.max_sp);
	}
}

/**
 * Validates a skill's HP rate cost when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the HP rate cost should be set it.
 *
 **/
static void skill_validate_hp_rate_cost(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->hp_rate, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "HPRateCost");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int hp_rate_cost;

			if (libconfig->setting_lookup_int(t, lv, &hp_rate_cost) == CONFIG_TRUE) {
				if (hp_rate_cost >= -100 && hp_rate_cost <= 100)
					sk->hp_rate[i] = hp_rate_cost;
				else
					ShowWarning("%s: Invalid HP rate cost %d specified in level %d for skill ID %d in %s! Minimum is -100, maximum is 100. Defaulting to 0...\n",
						    __func__, hp_rate_cost, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int hp_rate_cost;

	if (libconfig->setting_lookup_int(conf, "HPRateCost", &hp_rate_cost) == CONFIG_TRUE) {
		if (hp_rate_cost >= -100 && hp_rate_cost <= 100)
			skill->level_set_value(sk->hp_rate, hp_rate_cost);
		else
			ShowWarning("%s: Invalid HP rate cost %d specified for skill ID %d in %s! Minimum is -100, maximum is 100. Defaulting to 0...\n",
				    __func__, hp_rate_cost, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's SP rate cost when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the SP rate cost should be set it.
 *
 **/
static void skill_validate_sp_rate_cost(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->sp_rate, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SPRateCost");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int sp_rate_cost;

			if (libconfig->setting_lookup_int(t, lv, &sp_rate_cost) == CONFIG_TRUE) {
				if (sp_rate_cost >= -100 && sp_rate_cost <= 100)
					sk->sp_rate[i] = sp_rate_cost;
				else
					ShowWarning("%s: Invalid SP rate cost %d specified in level %d for skill ID %d in %s! Minimum is -100, maximum is 100. Defaulting to 0...\n",
						    __func__, sp_rate_cost, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int sp_rate_cost;

	if (libconfig->setting_lookup_int(conf, "SPRateCost", &sp_rate_cost) == CONFIG_TRUE) {
		if (sp_rate_cost >= -100 && sp_rate_cost <= 100)
			skill->level_set_value(sk->sp_rate, sp_rate_cost);
		else
			ShowWarning("%s: Invalid SP rate cost %d specified for skill ID %d in %s! Minimum is -100, maximum is 100. Defaulting to 0...\n",
				    __func__, sp_rate_cost, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's maximum HP trigger when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the maximum HP trigger should be set it.
 *
 **/
static void skill_validate_max_hp_trigger(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->mhp, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "MaxHPTrigger");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int max_hp_trigger;

			if (libconfig->setting_lookup_int(t, lv, &max_hp_trigger) == CONFIG_TRUE) {
				if (max_hp_trigger >= 0 && max_hp_trigger <= 100)
					sk->mhp[i] = max_hp_trigger;
				else
					ShowWarning("%s: Invalid maximum HP trigger %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is 100. Defaulting to 0...\n",
						    __func__, max_hp_trigger, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int max_hp_trigger;

	if (libconfig->setting_lookup_int(conf, "MaxHPTrigger", &max_hp_trigger) == CONFIG_TRUE) {
		if (max_hp_trigger >= 0 && max_hp_trigger <= 100)
			skill->level_set_value(sk->mhp, max_hp_trigger);
		else
			ShowWarning("%s: Invalid maximum HP trigger %d specified for skill ID %d in %s! Minimum is 0, maximum is 100. Defaulting to 0...\n",
				    __func__, max_hp_trigger, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's maximum SP trigger when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the maximum SP trigger should be set it.
 *
 **/
static void skill_validate_max_sp_trigger(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->msp, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "MaxSPTrigger");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int max_sp_trigger;

			if (libconfig->setting_lookup_int(t, lv, &max_sp_trigger) == CONFIG_TRUE) {
				if (max_sp_trigger >= 0 && max_sp_trigger <= 100)
					sk->msp[i] = max_sp_trigger;
				else
					ShowWarning("%s: Invalid maximum SP trigger %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is 100. Defaulting to 0...\n",
						    __func__, max_sp_trigger, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	int max_sp_trigger;

	if (libconfig->setting_lookup_int(conf, "MaxSPTrigger", &max_sp_trigger) == CONFIG_TRUE) {
		if (max_sp_trigger >= 0 && max_sp_trigger <= 100)
			skill->level_set_value(sk->msp, max_sp_trigger);
		else
			ShowWarning("%s: Invalid maximum SP trigger %d specified for skill ID %d in %s! Minimum is 0, maximum is 100. Defaulting to 0...\n",
				    __func__, max_sp_trigger, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's Zeny cost when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the Zeny cost should be set it.
 *
 **/
static void skill_validate_zeny_cost(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->zeny, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "ZenyCost");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int zeny_cost;

			if (libconfig->setting_lookup_int(t, lv, &zeny_cost) == CONFIG_TRUE) {
				if (zeny_cost >= 0 && zeny_cost <= MAX_ZENY)
					sk->zeny[i] = zeny_cost;
				else
					ShowWarning("%s: Invalid Zeny cost %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, zeny_cost, i + 1, sk->nameid, conf->file, MAX_ZENY);
			}
		}

		return;
	}

	int zeny_cost;

	if (libconfig->setting_lookup_int(conf, "ZenyCost", &zeny_cost) == CONFIG_TRUE) {
		if (zeny_cost >= 0 && zeny_cost <= MAX_ZENY)
			skill->level_set_value(sk->zeny, zeny_cost);
		else
			ShowWarning("%s: Invalid Zeny cost %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
				    __func__, zeny_cost, sk->nameid, conf->file, MAX_ZENY);
	}
}

/**
 * Validates a single weapon type when reading the skill DB.
 *
 * @param type The weapon type to validate.
 * @param on Whether the weapon type is required for the skill.
 * @param sk The s_skill_db struct where the weapon type should be set it.
 * @return 0 if the passed weapon type is valid, otherwise 1.
 *
 **/
static int skill_validate_weapontype_sub(const char *type, bool on, struct s_skill_db *sk)
{
	nullpo_retr(1, type);
	nullpo_retr(1, sk);

	if (strcmpi(type, "NoWeapon") == 0) {
		if (on)
			sk->weapon |= (1 << W_FIST);
		else
			sk->weapon &= ~(1 << W_FIST);
	} else if (strcmpi(type, "Daggers") == 0) {
		if (on)
			sk->weapon |= (1 << W_DAGGER);
		else
			sk->weapon &= ~(1 << W_DAGGER);
	} else if (strcmpi(type, "1HSwords") == 0) {

		if (on)
			sk->weapon |= (1 << W_1HSWORD);
		else
			sk->weapon &= ~(1 << W_1HSWORD);
	} else if (strcmpi(type, "2HSwords") == 0) {
		if (on)
			sk->weapon |= (1 << W_2HSWORD);
		else
			sk->weapon &= ~(1 << W_2HSWORD);
	} else if (strcmpi(type, "1HSpears") == 0) {
		if (on)
			sk->weapon |= (1 << W_1HSPEAR);
		else
			sk->weapon &= ~(1 << W_1HSPEAR);
	} else if (strcmpi(type, "2HSpears") == 0) {
		if (on)
			sk->weapon |= (1 << W_2HSPEAR);
		else
			sk->weapon &= ~(1 << W_2HSPEAR);
	} else if (strcmpi(type, "1HAxes") == 0) {
		if (on)
			sk->weapon |= (1 << W_1HAXE);
		else
			sk->weapon &= ~(1 << W_1HAXE);
	} else if (strcmpi(type, "2HAxes") == 0) {
		if (on)
			sk->weapon |= (1 << W_2HAXE);
		else
			sk->weapon &= ~(1 << W_2HAXE);
	} else if (strcmpi(type, "Maces") == 0) {
		if (on)
			sk->weapon |= (1 << W_MACE);
		else
			sk->weapon &= ~(1 << W_MACE);
	} else if (strcmpi(type, "2HMaces") == 0) {
		if (on)
			sk->weapon |= (1 << W_2HMACE);
		else
			sk->weapon &= ~(1 << W_2HMACE);
	} else if (strcmpi(type, "Staves") == 0) {
		if (on)
			sk->weapon |= (1 << W_STAFF);
		else
			sk->weapon &= ~(1 << W_STAFF);
	} else if (strcmpi(type, "Bows") == 0) {
		if (on)
			sk->weapon |= (1 << W_BOW);
		else
			sk->weapon &= ~(1 << W_BOW);
	} else if (strcmpi(type, "Knuckles") == 0) {
		if (on)
			sk->weapon |= (1 << W_KNUCKLE);
		else
			sk->weapon &= ~(1 << W_KNUCKLE);
	} else if (strcmpi(type, "Instruments") == 0) {
		if (on)
			sk->weapon |= (1 << W_MUSICAL);
		else
			sk->weapon &= ~(1 << W_MUSICAL);
	} else if (strcmpi(type, "Whips") == 0) {
		if (on)
			sk->weapon |= (1 << W_WHIP);
		else
			sk->weapon &= ~(1 << W_WHIP);
	} else if (strcmpi(type, "Books") == 0) {
		if (on)
			sk->weapon |= (1 << W_BOOK);
		else
			sk->weapon &= ~(1 << W_BOOK);
	} else if (strcmpi(type, "Katars") == 0) {
		if (on)
			sk->weapon |= (1 << W_KATAR);
		else
			sk->weapon &= ~(1 << W_KATAR);
	} else if (strcmpi(type, "Revolvers") == 0) {
		if (on)
			sk->weapon |= (1 << W_REVOLVER);
		else
			sk->weapon &= ~(1 << W_REVOLVER);
	} else if (strcmpi(type, "Rifles") == 0) {
		if (on)
			sk->weapon |= (1 << W_RIFLE);
		else
			sk->weapon &= ~(1 << W_RIFLE);
	} else if (strcmpi(type, "GatlingGuns") == 0) {
		if (on)
			sk->weapon |= (1 << W_GATLING);
		else
			sk->weapon &= ~(1 << W_GATLING);
	} else if (strcmpi(type, "Shotguns") == 0) {
		if (on)
			sk->weapon |= (1 << W_SHOTGUN);
		else
			sk->weapon &= ~(1 << W_SHOTGUN);
	} else if (strcmpi(type, "GrenadeLaunchers") == 0) {
		if (on)
			sk->weapon |= (1 << W_GRENADE);
		else
			sk->weapon &= ~(1 << W_GRENADE);
	} else if (strcmpi(type, "FuumaShurikens") == 0) {
		if (on)
			sk->weapon |= (1 << W_HUUMA);
		else
			sk->weapon &= ~(1 << W_HUUMA);
	} else if (strcmpi(type, "2HStaves") == 0) {
		if (on)
			sk->weapon |= (1 << W_2HSTAFF);
		else
			sk->weapon &= ~(1 << W_2HSTAFF);
	} else if (strcmpi(type, "DWDaggers") == 0) {
		if (on)
			sk->weapon |= (1 << W_DOUBLE_DD);
		else
			sk->weapon &= ~(1 << W_DOUBLE_DD);
	} else if (strcmpi(type, "DWSwords") == 0) {
		if (on)
			sk->weapon |= (1 << W_DOUBLE_SS);
		else
			sk->weapon &= ~(1 << W_DOUBLE_SS);
	} else if (strcmpi(type, "DWAxes") == 0) {
		if (on)
			sk->weapon |= (1 << W_DOUBLE_AA);
		else
			sk->weapon &= ~(1 << W_DOUBLE_AA);
	} else if (strcmpi(type, "DWDaggerSword") == 0) {
		if (on)
			sk->weapon |= (1 << W_DOUBLE_DS);
		else
			sk->weapon &= ~(1 << W_DOUBLE_DS);
	} else if (strcmpi(type, "DWDaggerAxe") == 0) {
		if (on)
			sk->weapon |= (1 << W_DOUBLE_DA);
		else
			sk->weapon &= ~(1 << W_DOUBLE_DA);
	} else if (strcmpi(type, "DWSwordAxe") == 0) {
		if (on)
			sk->weapon |= (1 << W_DOUBLE_SA);
		else
			sk->weapon &= ~(1 << W_DOUBLE_SA);
	} else if (strcmpi(type, "All") == 0) {
		sk->weapon = 0;
	} else {
		return 1;
	}

	return 0;
}

/**
 * Validates a skill's required weapon types when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required weapon types should be set it.
 *
 **/
static void skill_validate_weapontype(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->weapon = 0;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "WeaponTypes");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			bool on = libconfig->setting_get_bool_real(tt);

			if (skill->validate_weapontype_sub(config_setting_name(tt), on, sk) != 0)
				ShowWarning("%s: Invalid required weapon type %s specified for skill ID %d in %s! Skipping type...\n",
					    __func__, config_setting_name(tt), sk->nameid, conf->file);
		}

		return;
	}

	const char *weapon_type;

	if (libconfig->setting_lookup_string(conf, "WeaponTypes", &weapon_type) == CONFIG_TRUE) {
		if (skill->validate_weapontype_sub(weapon_type, true, sk) != 0)
			ShowWarning("%s: Invalid required weapon type %s specified for skill ID %d in %s! Defaulting to All...\n",
				    __func__, weapon_type, sk->nameid, conf->file);
	}
}

/**
 * Validates a single ammunition type when reading the skill DB.
 *
 * @param type The ammunition type to validate.
 * @param on Whether the ammunition type is required for the skill.
 * @param sk The s_skill_db struct where the ammunition type should be set it.
 * @return 0 if the passed ammunition type is valid, otherwise 1.
 *
 **/
static int skill_validate_ammotype_sub(const char *type, bool on, struct s_skill_db *sk)
{
	nullpo_retr(1, type);
	nullpo_retr(1, sk);

	if (strcmpi(type, "A_ARROW") == 0) {
		if (on)
			sk->ammo |= (1 << A_ARROW);
		else
			sk->ammo &= ~(1 << A_ARROW);
	} else if (strcmpi(type, "A_DAGGER") == 0) {
		if (on)
			sk->ammo |= (1 << A_DAGGER);
		else
			sk->ammo &= ~(1 << A_DAGGER);
	} else if (strcmpi(type, "A_BULLET") == 0) {
		if (on)
			sk->ammo |= (1 << A_BULLET);
		else
			sk->ammo &= ~(1 << A_BULLET);
	} else if (strcmpi(type, "A_SHELL") == 0) {
		if (on)
			sk->ammo |= (1 << A_SHELL);
		else
			sk->ammo &= ~(1 << A_SHELL);
	} else if (strcmpi(type, "A_GRENADE") == 0) {
		if (on)
			sk->ammo |= (1 << A_GRENADE);
		else
			sk->ammo &= ~(1 << A_GRENADE);
	} else if (strcmpi(type, "A_SHURIKEN") == 0) {
		if (on)
			sk->ammo |= (1 << A_SHURIKEN);
		else
			sk->ammo &= ~(1 << A_SHURIKEN);
	} else if (strcmpi(type, "A_KUNAI") == 0) {
		if (on)
			sk->ammo |= (1 << A_KUNAI);
		else
			sk->ammo &= ~(1 << A_KUNAI);
	} else if (strcmpi(type, "A_CANNONBALL") == 0) {
		if (on)
			sk->ammo |= (1 << A_CANNONBALL);
		else
			sk->ammo &= ~(1 << A_CANNONBALL);
	} else if (strcmpi(type, "A_THROWWEAPON") == 0) {
		if (on)
			sk->ammo |= (1 << A_THROWWEAPON);
		else
			sk->ammo &= ~(1 << A_THROWWEAPON);
	} else if (strcmpi(type, "All") == 0) {
		if (on)
			sk->ammo = 0xFFFFFFFF;
		else
			sk->ammo = 0;
	} else {
		return 1;
	}

	return 0;
}

/**
 * Validates a skill's required ammunition types when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required ammunition types should be set it.
 *
 **/
static void skill_validate_ammotype(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->ammo = 0;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "AmmoTypes");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			bool on = libconfig->setting_get_bool_real(tt);

			if (skill->validate_ammotype_sub(config_setting_name(tt), on, sk) != 0)
				ShowWarning("%s: Invalid required ammunition type %s specified for skill ID %d in %s! Skipping type...\n",
					    __func__, config_setting_name(tt), sk->nameid, conf->file);
		}
	}

	const char *ammo_type;

	if (libconfig->setting_lookup_string(conf, "AmmoTypes", &ammo_type) == CONFIG_TRUE) {
		if (skill->validate_ammotype_sub(ammo_type, true, sk) != 0)
			ShowWarning("%s: Invalid required ammunition type %s specified for skill ID %d in %s! Defaulting to None...\n",
				    __func__, ammo_type, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's required ammunition amount when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required ammunition amount should be set it.
 *
 **/
static void skill_validate_ammo_amount(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->ammo_qty, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "AmmoAmount");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int ammo_amount;

			if (libconfig->setting_lookup_int(t, lv, &ammo_amount) == CONFIG_TRUE) {
				if (ammo_amount >= 0 && ammo_amount <= MAX_AMOUNT)
					sk->ammo_qty[i] = ammo_amount;
				else
					ShowWarning("%s: Invalid required ammunition amount %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, ammo_amount, i + 1, sk->nameid, conf->file, MAX_AMOUNT);
			}
		}

		return;
	}

	int ammo_amount;

	if (libconfig->setting_lookup_int(conf, "AmmoAmount", &ammo_amount) == CONFIG_TRUE) {
		if (ammo_amount >= 0 && ammo_amount <= MAX_AMOUNT)
			skill->level_set_value(sk->ammo_qty, ammo_amount);
		else
			ShowWarning("%s: Invalid required ammunition amount %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
				    __func__, ammo_amount, sk->nameid, conf->file, MAX_AMOUNT);
	}
}

/**
 * Validates a single required state when reading the skill DB.
 *
 * @param state The required state to validate.
 * @return A number greater than or equal to 0 if the passed required state is valid, otherwise -1.
 *
 **/
static int skill_validate_state_sub(const char *state)
{
	nullpo_retr(-1, state);

	int ret_val = ST_NONE;

	if (strcmpi(state, "Hiding") == 0)
		ret_val = ST_HIDING;
	else if (strcmpi(state, "Cloaking") == 0)
		ret_val = ST_CLOAKING;
	else if (strcmpi(state, "Hidden") == 0)
		ret_val = ST_HIDDEN;
	else if (strcmpi(state, "Riding") == 0)
		ret_val = ST_RIDING;
	else if (strcmpi(state, "Falcon") == 0)
		ret_val = ST_FALCON;
	else if (strcmpi(state, "Cart") == 0)
		ret_val = ST_CART;
	else if (strcmpi(state, "Shield") == 0)
		ret_val = ST_SHIELD;
	else if (strcmpi(state, "Sight") == 0)
		ret_val = ST_SIGHT;
	else if (strcmpi(state, "ExplosionSpirits") == 0)
		ret_val = ST_EXPLOSIONSPIRITS;
	else if (strcmpi(state, "CartBoost") == 0)
		ret_val = ST_CARTBOOST;
	else if (strcmpi(state, "NotOverWeight") == 0)
		ret_val = ST_RECOV_WEIGHT_RATE;
	else if (strcmpi(state, "Moveable") == 0)
		ret_val = ST_MOVE_ENABLE;
	else if (strcmpi(state, "InWater") == 0)
		ret_val = ST_WATER;
	else if (strcmpi(state, "Dragon") == 0)
		ret_val = ST_RIDINGDRAGON;
	else if (strcmpi(state, "Warg") == 0)
		ret_val = ST_WUG;
	else if (strcmpi(state, "RidingWarg") == 0)
		ret_val = ST_RIDINGWUG;
	else if (strcmpi(state, "MadoGear") == 0)
		ret_val = ST_MADO;
	else if (strcmpi(state, "ElementalSpirit") == 0)
		ret_val = ST_ELEMENTALSPIRIT;
	else if (strcmpi(state, "PoisonWeapon") == 0)
		ret_val = ST_POISONINGWEAPON;
	else if (strcmpi(state, "RollingCutter") == 0)
		ret_val = ST_ROLLINGCUTTER;
	else if (strcmpi(state, "MH_Fighting") == 0)
		ret_val = ST_MH_FIGHTING;
	else if (strcmpi(state, "MH_Grappling") == 0)
		ret_val = ST_MH_GRAPPLING;
	else if (strcmpi(state, "Peco") == 0)
		ret_val = ST_PECO;
	else if (strcmpi(state, "QD_Shot_Ready") == 0)
		ret_val = ST_QD_SHOT_READY;
	else if (strcmpi(state, "SunStance") == 0)
		ret_val = ST_SUNSTANCE;
	else if (strcmpi(state, "MoonStance") == 0)
		ret_val = ST_MOONSTANCE;
	else if (strcmpi(state, "StarStance") == 0)
		ret_val = ST_STARSTANCE;
	else if (strcmpi(state, "UniverseStance") == 0)
		ret_val = ST_UNIVERSESTANCE;
	else if (strcmpi(state, "None") != 0)
		ret_val = -1;

	return ret_val;
}

/**
 * Validates a skill's required states when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required states should be set it.
 *
 **/
static void skill_validate_state(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->state, ST_NONE);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "State");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			const char *state;

			if (libconfig->setting_lookup_string(t, lv, &state) == CONFIG_TRUE) {
				int sta = skill->validate_state_sub(state);

				if (sta > ST_NONE)
					sk->state[i] = sta;
				else if (sta == -1)
					ShowWarning("%s: Invalid required state %s specified in level %d for skill ID %d in %s! Defaulting to None...\n",
						    __func__, state, i + 1, sk->nameid, conf->file);
			}
		}

		return;
	}

	const char *state;

	if (libconfig->setting_lookup_string(conf, "State", &state) == CONFIG_TRUE) {
		int sta = skill->validate_state_sub(state);

		if (sta > ST_NONE)
			skill->level_set_value(sk->state, sta);
		else if (sta == -1)
			ShowWarning("%s: Invalid required state %s specified for skill ID %d in %s! Defaulting to None...\n",
				    __func__, state, sk->nameid, conf->file);
	}
}

/**
 * Validates a skill's Spirit Sphere cost when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the Spirit Sphere cost should be set it.
 *
 **/
static void skill_validate_spirit_sphere_cost(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->spiritball, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "SpiritSphereCost");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int spirit_sphere_cost;

			if (libconfig->setting_lookup_int(t, lv, &spirit_sphere_cost) == CONFIG_TRUE) {
				if (spirit_sphere_cost >= 0 && spirit_sphere_cost <= MAX_SPIRITBALL)
					sk->spiritball[i] = spirit_sphere_cost;
				else
					ShowWarning("%s: Invalid Spirit Sphere cost %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, spirit_sphere_cost, i + 1, sk->nameid, conf->file, MAX_SPIRITBALL);
			}
		}

		return;
	}

	int spirit_sphere_cost;

	if (libconfig->setting_lookup_int(conf, "SpiritSphereCost", &spirit_sphere_cost) == CONFIG_TRUE) {
		if (spirit_sphere_cost >= 0 && spirit_sphere_cost <= MAX_SPIRITBALL)
			skill->level_set_value(sk->spiritball, spirit_sphere_cost);
		else
			ShowWarning("%s: Invalid Spirit Sphere cost %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
				    __func__, spirit_sphere_cost, sk->nameid, conf->file, MAX_SPIRITBALL);
	}
}

/**
 * Validates a skill's required items amounts when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required items amounts should be set it.
 *
 **/
static void skill_validate_item_requirements_sub_item_amount(struct config_setting_t *conf, struct s_skill_db *sk, int item_index)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_LEVEL; i++)
		sk->req_items.item[item_index].amount[i] = 0;

	if (config_setting_is_group(conf)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int amount;

			if (libconfig->setting_lookup_int(conf, lv, &amount) == CONFIG_TRUE) {
				if (amount >= 0 && amount <= MAX_AMOUNT)
					sk->req_items.item[item_index].amount[i] = amount;
				else
					ShowWarning("%s: Invalid required item amount %d specified in level %d for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
						    __func__, amount, i + 1, sk->nameid, conf->file, MAX_AMOUNT);
			} else {
				 // Items is not required for this skill level. (Not even in inventory!)
				sk->req_items.item[item_index].amount[i] = -1;
			}
		}

		return;
	}

	int amount = libconfig->setting_get_int(conf);

	if (amount >= 0 && amount <= MAX_AMOUNT) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++)
			sk->req_items.item[item_index].amount[i] = amount;
	} else {
		ShowWarning("%s: Invalid required item amount %d specified for skill ID %d in %s! Minimum is 0, maximum is %d. Defaulting to 0...\n",
			    __func__, amount, sk->nameid, conf->file, MAX_AMOUNT);
	}
}

/**
 * Validates a skill's required items when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required items should be set it.
 *
 **/
static void skill_validate_item_requirements_sub_items(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		sk->req_items.item[i].id = 0;

		for (int j = 0; j < MAX_SKILL_LEVEL; j++)
			sk->req_items.item[i].amount[j] = 0;
	}

	int item_index = 0;
	int count = libconfig->setting_length(conf);

	for (int i = 0; i < count; i++) {
		struct config_setting_t *t = libconfig->setting_get_elem(conf, i);

		if (t != NULL && strcasecmp(config_setting_name(t), "Any") != 0) {
			if (item_index >= MAX_SKILL_ITEM_REQUIRE) {
				ShowWarning("%s: Too many required items specified for skill ID %d in %s! Skipping item %s...\n",
					    __func__, sk->nameid, conf->file, config_setting_name(t));
				continue;
			}

			int item_id = skill->validate_requirements_item_name(config_setting_name(t));

			if (item_id == 0) {
				ShowWarning("%s: Invalid required item %s specified for skill ID %d in %s! Skipping item...\n",
					    __func__, config_setting_name(t), sk->nameid, conf->file);
				continue;
			}

			int j;

			ARR_FIND(0, MAX_SKILL_ITEM_REQUIRE, j, sk->req_items.item[j].id == item_id);

			if (j < MAX_SKILL_ITEM_REQUIRE) {
				ShowWarning("%s: Duplicate required item %s specified for skill ID %d in %s! Skipping item...\n",
					    __func__, config_setting_name(t), sk->nameid, conf->file);
				continue;
			}

			sk->req_items.item[item_index].id = item_id;
			skill->validate_item_requirements_sub_item_amount(t, sk, item_index);
			item_index++;
		}
	}
}

/**
 * Validates a skill's required items any-flag when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required items any-flag should be set it.
 *
 **/
static void skill_validate_item_requirements_sub_any_flag(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_LEVEL; i++)
		sk->req_items.any[i] = false;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Any");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int any_flag;

			if (libconfig->setting_lookup_bool(t, lv, &any_flag) == CONFIG_TRUE)
				sk->req_items.any[i] = (any_flag != 0);
		}

		return;
	}

	int any_flag;

	if (libconfig->setting_lookup_bool(conf, "Any", &any_flag) == CONFIG_TRUE && any_flag != 0) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++)
			sk->req_items.any[i] = true;
	}
}

/**
 * Validates a skill's required items when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required items should be set it.
 *
 **/
static void skill_validate_item_requirements(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Items");

	if (t != NULL && config_setting_is_group(t)) {
		skill->validate_item_requirements_sub_any_flag(t, sk);
		skill->validate_item_requirements_sub_items(t, sk);
	}
}

/**
 * Validates a skill's required equipment amounts when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required equipment amounts should be set it.
 *
 **/
static void skill_validate_equip_requirements_sub_item_amount(struct config_setting_t *conf, struct s_skill_db *sk, int item_index)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_LEVEL; i++)
		sk->req_equip.item[item_index].amount[i] = 0;

	if (config_setting_is_group(conf)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int amount;

			if (libconfig->setting_lookup_int(conf, lv, &amount) == CONFIG_TRUE) {
				if (amount > 0) {
					sk->req_equip.item[item_index].amount[i] = amount;
				} else {
					ShowWarning("%s: Invalid required equipment amount %d specified in level %d for skill ID %d in %s! Must be greater than 0. Defaulting to 1...\n",
						    __func__, amount, i + 1, sk->nameid, conf->file);
					sk->req_equip.item[item_index].amount[i] = 1;
				}
			}
		}

		return;
	}

	int amount = libconfig->setting_get_int(conf);

	if (amount <= 0) {
		ShowWarning("%s: Invalid required equipment amount %d specified for skill ID %d in %s! Must be greater than 0. Defaulting to 1...\n",
			    __func__, amount, sk->nameid, conf->file);
		amount = 1;
	}

	for (int i = 0; i < MAX_SKILL_LEVEL; i++)
		sk->req_equip.item[item_index].amount[i] = amount;
}

/**
 * Validates a skill's required equipment when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required equipment should be set it.
 *
 **/
static void skill_validate_equip_requirements_sub_items(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		sk->req_equip.item[i].id = 0;

		for (int j = 0; j < MAX_SKILL_LEVEL; j++)
			sk->req_equip.item[i].amount[j] = 0;
	}

	int item_index = 0;
	int count = libconfig->setting_length(conf);

	for (int i = 0; i < count; i++) {
		struct config_setting_t *t = libconfig->setting_get_elem(conf, i);

		if (t != NULL && strcasecmp(config_setting_name(t), "Any") != 0) {
			if (item_index >= MAX_SKILL_ITEM_REQUIRE) {
				ShowWarning("%s: Too many required equipment items specified for skill ID %d in %s! Skipping item %s...\n",
					    __func__, sk->nameid, conf->file, config_setting_name(t));
				continue;
			}

			int item_id = skill->validate_requirements_item_name(config_setting_name(t));
			struct item_data *it = itemdb->exists(item_id);

			if (item_id == 0 || it == NULL) {
				ShowWarning("%s: Invalid required equipment item %s specified for skill ID %d in %s! Skipping item...\n",
					    __func__, config_setting_name(t), sk->nameid, conf->file);
				continue;
			}

			if (it->type != IT_WEAPON && it->type != IT_AMMO && it->type != IT_ARMOR && it->type != IT_CARD) {
				ShowWarning("%s: Non-equipment item %s specified for skill ID %d in %s! Skipping item...\n",
					    __func__, config_setting_name(t), sk->nameid, conf->file);
				continue;
			}

			int j;

			ARR_FIND(0, MAX_SKILL_ITEM_REQUIRE, j, sk->req_equip.item[j].id == item_id);

			if (j < MAX_SKILL_ITEM_REQUIRE) {
				ShowWarning("%s: Duplicate required equipment item %s specified for skill ID %d in %s! Skipping item...\n",
					    __func__, config_setting_name(t), sk->nameid, conf->file);
				continue;
			}

			sk->req_equip.item[item_index].id = item_id;
			skill->validate_equip_requirements_sub_item_amount(t, sk, item_index);
			item_index++;
		}
	}
}

/**
 * Validates a skill's required equipment any-flag when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required equipment any-flag should be set it.
 *
 **/
static void skill_validate_equip_requirements_sub_any_flag(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_LEVEL; i++)
		sk->req_equip.any[i] = false;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Any");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int any_flag;

			if (libconfig->setting_lookup_bool(t, lv, &any_flag) == CONFIG_TRUE)
				sk->req_equip.any[i] = (any_flag != 0);
		}

		return;
	}

	int any_flag;

	if (libconfig->setting_lookup_bool(conf, "Any", &any_flag) == CONFIG_TRUE && any_flag != 0) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++)
			sk->req_equip.any[i] = true;
	}
}

/**
 * Validates a skill's required equipment when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the required equipment should be set it.
 *
 **/
static void skill_validate_equip_requirements(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Equip");

	if (t != NULL && config_setting_is_group(t)) {
		skill->validate_equip_requirements_sub_any_flag(t, sk);
		skill->validate_equip_requirements_sub_items(t, sk);
	}
}

/**
 * Validates a required item's config setting name when reading the skill DB.
 *
 * @param name The config setting name to validate.
 * @return The corresponding item ID if the passed config setting name is valid, otherwise 0.
 *
 **/
static int skill_validate_requirements_item_name(const char *name)
{
	nullpo_ret(name);

	int item_id = 0;

	if (strlen(name) > 2 && name[0] == 'I' && name[1] == 'D') {
		if ((item_id = atoi(name + 2)) == 0)
			return 0;

		struct item_data *it = itemdb->exists(item_id);

		if (it == NULL)
			return 0;

		return it->nameid;
	}

	if (!script->get_constant(name, &item_id))
		return 0;

	return item_id;
}

/**
 * Validates a skill's requirements when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the requirements should be set it.
 *
 **/
static void skill_validate_requirements(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Requirements");

	if (t != NULL && config_setting_is_group(t)) {
		skill->validate_hp_cost(t, sk);
		skill->validate_sp_cost(t, sk);
		skill->validate_hp_rate_cost(t, sk);
		skill->validate_sp_rate_cost(t, sk);
		skill->validate_max_hp_trigger(t, sk);
		skill->validate_max_sp_trigger(t, sk);
		skill->validate_zeny_cost(t, sk);
		skill->validate_weapontype(t, sk);
		skill->validate_ammotype(t, sk);
		skill->validate_ammo_amount(t, sk);
		skill->validate_state(t, sk);
		skill->validate_spirit_sphere_cost(t, sk);
		skill->validate_item_requirements(t, sk);
		skill->validate_equip_requirements(t, sk);
	}
}

/**
 * Validates a single unit ID when reading the skill DB.
 *
 * @param unit_id The unit ID to validate.
 * @return A number greater than or equal to 0 if the passed unit ID is valid, otherwise -1.
 *
 **/
static int skill_validate_unit_id_sub(int unit_id)
{
	if (unit_id == 0 || (unit_id >= UNT_SAFETYWALL && unit_id <= UNT_BOOKOFCREATINGSTAR))
		return unit_id;

	return -1;
}

/**
 * Validates a skill's unit IDs if specified as single value when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's unit ID data.
 * @param sk The s_skill_db struct where the unit IDs should be set it.
 * @param index The array index to use. (-1 for whole array.)
 * @param unit_id The unit ID to validate.
 *
 **/
static void skill_validate_unit_id_value(struct config_setting_t *conf, struct s_skill_db *sk, int index, int unit_id)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	if (skill->validate_unit_id_sub(unit_id) == -1) {
		char level_string[24];

		if (index == -1)
			*level_string = '\0';
		else
			snprintf(level_string, sizeof(level_string), "in level %d ", index + 1);

		ShowWarning("%s: Invalid unit ID %d specified %sfor skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
			    __func__, unit_id, level_string, sk->nameid, conf->file);

		return;
	}

	if (index == -1) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++)
			sk->unit_id[i][0] = unit_id;
	} else {
		sk->unit_id[index][0] = unit_id;
	}
}

/**
 * Validates a skill's unit IDs if specified as array when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's unit ID data.
 * @param sk The s_skill_db struct where the unit IDs should be set it.
 * @param index The array index to use. (-1 for whole array.)
 *
 **/
static void skill_validate_unit_id_array(struct config_setting_t *conf, struct s_skill_db *sk, int index)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	char level_string[24];

	if (index == -1)
		*level_string = '\0';
	else
		snprintf(level_string, sizeof(level_string), "in level %d ", index + 1);

	if (libconfig->setting_length(conf) == 0) {
		ShowWarning("%s: No unit ID(s) specified %sfor skill ID %d in %s! Defaulting to 0...\n",
			    __func__, level_string, sk->nameid, conf->file);
		return;
	}

	if (libconfig->setting_length(conf) > 2)
		ShowWarning("%s: Specified more than two unit IDs %sfor skill ID %d in %s! Reading only the first two...\n",
			    __func__, level_string, sk->nameid, conf->file);

	int unit_id1 = libconfig->setting_get_int_elem(conf, 0);

	if (skill->validate_unit_id_sub(unit_id1) == -1) {
		ShowWarning("%s: Invalid unit ID %d specified %sfor skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
			    __func__, unit_id1, level_string, sk->nameid, conf->file);
		unit_id1 = 0;
	}

	int unit_id2 = 0;

	if (libconfig->setting_length(conf) > 1) {
		unit_id2 = libconfig->setting_get_int_elem(conf, 1);

		if (skill->validate_unit_id_sub(unit_id2) == -1) {
			ShowWarning("%s: Invalid unit ID %d specified %sfor skill ID %d in %s! Must be greater than or equal to 0. Defaulting to 0...\n",
				    __func__, unit_id2, level_string, sk->nameid, conf->file);
			unit_id2 = 0;
		}
	}

	if (unit_id1 == 0 && unit_id2 == 0)
		return;

	if (index == -1) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			sk->unit_id[i][0] = unit_id1;
			sk->unit_id[i][1] = unit_id2;
		}
	} else {
		sk->unit_id[index][0] = unit_id1;
		sk->unit_id[index][1] = unit_id2;
	}
}

/**
 * Validates a skill's unit IDs if specified as group when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's unit ID data.
 * @param sk The s_skill_db struct where the unit IDs should be set it.
 *
 **/
static void skill_validate_unit_id_group(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
		struct config_setting_t *t;
		char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
		snprintf(lv, sizeof(lv), "Lv%d", i + 1);

		if ((t = libconfig->setting_get_member(conf, lv)) != NULL && config_setting_is_array(t)) {
			skill_validate_unit_id_array(t, sk, i);
			continue;
		}

		int unit_id;

		if (libconfig->setting_lookup_int(conf, lv, &unit_id) == CONFIG_TRUE)
			skill_validate_unit_id_value(conf, sk, i, unit_id);
	}
}

/**
 * Validates a skill's unit IDs when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit IDs should be set it.
 *
 **/
static void skill_validate_unit_id(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
		sk->unit_id[i][0] = 0;
		sk->unit_id[i][1] = 0;
	}

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Id");

	if (t != NULL && config_setting_is_group(t)) {
		skill_validate_unit_id_group(t, sk);
		return;
	}

	if (t != NULL && config_setting_is_array(t)) {
		skill_validate_unit_id_array(t, sk, -1);
		return;
	}

	int unit_id;

	if (libconfig->setting_lookup_int(conf, "Id", &unit_id) == CONFIG_TRUE)
		skill_validate_unit_id_value(conf, sk, -1, unit_id);
}

/**
 * Validates a skill's unit layout when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit layout should be set it.
 *
 **/
static void skill_validate_unit_layout(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->unit_layout_type, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Layout");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int unit_layout;

			if (libconfig->setting_lookup_int(t, lv, &unit_layout) == CONFIG_TRUE) {
				if (unit_layout >= -1 && unit_layout <= MAX_SKILL_UNIT_LAYOUT)
					sk->unit_layout_type[i] = unit_layout;
				else
					ShowWarning("%s: Invalid unit layout %d specified in level %d for skill ID %d in %s! Minimum is -1, maximum is %d. Defaulting to 0...\n",
						    __func__, unit_layout, i + 1, sk->nameid, conf->file, MAX_SKILL_UNIT_LAYOUT);
			}
		}

		return;
	}

	int unit_layout;

	if (libconfig->setting_lookup_int(conf, "Layout", &unit_layout) == CONFIG_TRUE) {
		if (unit_layout >= -1 && unit_layout <= MAX_SKILL_UNIT_LAYOUT)
			skill->level_set_value(sk->unit_layout_type, unit_layout);
		else
			ShowWarning("%s: Invalid unit layout %d specified for skill ID %d in %s! Minimum is -1, maximum is %d. Defaulting to 0...\n",
				    __func__, unit_layout, sk->nameid, conf->file, MAX_SKILL_UNIT_LAYOUT);
	}
}

/**
 * Validates a skill's unit range when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit range should be set it.
 *
 **/
static void skill_validate_unit_range(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->unit_range, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Range");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int unit_range;

			if (libconfig->setting_lookup_int(t, lv, &unit_range) == CONFIG_TRUE) {
				if (unit_range >= -1 && unit_range <= UCHAR_MAX)
					sk->unit_range[i] = unit_range;
				else
					ShowWarning("%s: Invalid unit range %d specified in level %d for skill ID %d in %s! Minimum is -1, maximum is %d. Defaulting to 0...\n",
						    __func__, unit_range, i + 1, sk->nameid, conf->file, UCHAR_MAX);
			}
		}

		return;
	}

	int unit_range;

	if (libconfig->setting_lookup_int(conf, "Range", &unit_range) == CONFIG_TRUE) {
		if (unit_range >= -1 && unit_range <= UCHAR_MAX)
			skill->level_set_value(sk->unit_range, unit_range);
		else
			ShowWarning("%s: Invalid unit range %d specified for skill ID %d in %s! Minimum is -1, maximum is %d. Defaulting to 0...\n",
				    __func__, unit_range, sk->nameid, conf->file, UCHAR_MAX);
	}
}

/**
 * Validates a skill's unit interval when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit interval should be set it.
 *
 **/
static void skill_validate_unit_interval(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->unit_interval, 0);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Interval");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			int unit_interval;

			if (libconfig->setting_lookup_int(t, lv, &unit_interval) == CONFIG_TRUE) {
				if (unit_interval >= INFINITE_DURATION)
					sk->unit_interval[i] = unit_interval;
				else
					ShowWarning("%s: Invalid unit interval %d specified in level %d for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
						    __func__, unit_interval, i + 1, sk->nameid, conf->file, INFINITE_DURATION);
			}
		}

		return;
	}

	int unit_interval;

	if (libconfig->setting_lookup_int(conf, "Interval", &unit_interval) == CONFIG_TRUE) {
		if (unit_interval >= INFINITE_DURATION)
			skill->level_set_value(sk->unit_interval, unit_interval);
		else
			ShowWarning("%s: Invalid unit interval %d specified for skill ID %d in %s! Must be greater than or equal to %d. Defaulting to 0...\n",
				    __func__, unit_interval, sk->nameid, conf->file, INFINITE_DURATION);
	}
}

/**
 * Validates a single unit flag when reading the skill DB.
 *
 * @param type The unit flag to validate.
 * @param on Whether the unit flag is set for the skill.
 * @param sk The s_skill_db struct where the unit flag should be set it.
 * @return 0 if the passed unit flag is valid, otherwise 1.
 *
 **/
static int skill_validate_unit_flag_sub(const char *type, bool on, struct s_skill_db *sk)
{
	nullpo_retr(1, type);
	nullpo_retr(1, sk);

	if (strcmpi(type, "UF_DEFNOTENEMY") == 0) {
		if (on)
			sk->unit_flag |= UF_DEFNOTENEMY;
		else
			sk->unit_flag &= ~UF_DEFNOTENEMY;
	} else if (strcmpi(type, "UF_NOREITERATION") == 0) {
		if (on)
			sk->unit_flag |= UF_NOREITERATION;
		else
			sk->unit_flag &= ~UF_NOREITERATION;
	} else if (strcmpi(type, "UF_NOFOOTSET") == 0) {
		if (on)
			sk->unit_flag |= UF_NOFOOTSET;
		else
			sk->unit_flag &= ~UF_NOFOOTSET;
	} else if (strcmpi(type, "UF_NOOVERLAP") == 0) {
		if (on)
			sk->unit_flag |= UF_NOOVERLAP;
		else
			sk->unit_flag &= ~UF_NOOVERLAP;
	} else if (strcmpi(type, "UF_PATHCHECK") == 0) {
		if (on)
			sk->unit_flag |= UF_PATHCHECK;
		else
			sk->unit_flag &= ~UF_PATHCHECK;
	} else if (strcmpi(type, "UF_NOPC") == 0) {
		if (on)
			sk->unit_flag |= UF_NOPC;
		else
			sk->unit_flag &= ~UF_NOPC;
	} else if (strcmpi(type, "UF_NOMOB") == 0) {
		if (on)
			sk->unit_flag |= UF_NOMOB;
		else
			sk->unit_flag &= ~UF_NOMOB;
	} else if (strcmpi(type, "UF_SKILL") == 0) {
		if (on)
			sk->unit_flag |= UF_SKILL;
		else
			sk->unit_flag &= ~UF_SKILL;
	} else if (strcmpi(type, "UF_DANCE") == 0) {
		if (on)
			sk->unit_flag |= UF_DANCE;
		else
			sk->unit_flag &= ~UF_DANCE;
	} else if (strcmpi(type, "UF_ENSEMBLE") == 0) {
		if (on)
			sk->unit_flag |= UF_ENSEMBLE;
		else
			sk->unit_flag &= ~UF_ENSEMBLE;
	} else if (strcmpi(type, "UF_SONG") == 0) {
		if (on)
			sk->unit_flag |= UF_SONG;
		else
			sk->unit_flag &= ~UF_SONG;
	} else if (strcmpi(type, "UF_DUALMODE") == 0) {
		if (on)
			sk->unit_flag |= UF_DUALMODE;
		else
			sk->unit_flag &= ~UF_DUALMODE;
	} else if (strcmpi(type, "UF_RANGEDSINGLEUNIT") == 0) {
		if (on)
			sk->unit_flag |= UF_RANGEDSINGLEUNIT;
		else
			sk->unit_flag &= ~UF_RANGEDSINGLEUNIT;
	}
	else if (strcmpi(type, "UF_REMOVEDBYFIRERAIN") == 0) {
		if (on)
			sk->unit_flag |= UF_REMOVEDBYFIRERAIN;
		else
			sk->unit_flag &= ~UF_REMOVEDBYFIRERAIN;
	} else {
		return 1;
	}

	return 0;
}

/**
 * Validates a skill's unit flags when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit flags should be set it.
 *
 **/
static void skill_validate_unit_flag(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	sk->unit_flag = 0;

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Flag");

	if (t != NULL && config_setting_is_group(t)) {
		struct config_setting_t *tt;
		int i = 0;

		while ((tt = libconfig->setting_get_elem(t, i++)) != NULL) {
			bool on = libconfig->setting_get_bool_real(tt);

			if (skill->validate_unit_flag_sub(config_setting_name(tt), on, sk))
				ShowWarning("%s: Invalid unit flag %s specified for skill ID %d in %s! Skipping flag...\n",
					    __func__, config_setting_name(tt), sk->nameid, conf->file);
		}
	}
}

/**
 * Validates a single unit target when reading the skill DB.
 *
 * @param target The unit target to validate.
 * @return A number greater than or equal to 0 if the passed unit target is valid, otherwise -1.
 *
 **/
static int skill_validate_unit_target_sub(const char *target)
{
	nullpo_retr(-1, target);

	int ret_val = BCT_NOONE;

	if (strcmpi(target, "NotEnemy") == 0)
		ret_val = BCT_NOENEMY;
	else if (strcmpi(target, "NotParty") == 0)
		ret_val = BCT_NOPARTY;
	else if (strcmpi(target, "NotGuild") == 0)
		ret_val = BCT_NOGUILD;
	else if (strcmpi(target, "Friend") == 0)
		ret_val = BCT_NOENEMY;
	else if (strcmpi(target, "Party") == 0)
		ret_val = BCT_PARTY;
	else if (strcmpi(target, "Ally") == 0)
		ret_val = BCT_PARTY|BCT_GUILD;
	else if (strcmpi(target, "Guild") == 0)
		ret_val = BCT_GUILD;
	else if (strcmpi(target, "All") == 0)
		ret_val = BCT_ALL;
	else if (strcmpi(target, "Enemy") == 0)
		ret_val = BCT_ENEMY;
	else if (strcmpi(target, "Self") == 0)
		ret_val = BCT_SELF;
	else if (strcmpi(target, "SameGuild") == 0)
		ret_val = BCT_SAMEGUILD;
	else if (strcmpi(target, "GuildAlly") == 0)
		ret_val = BCT_GUILDALLY;
	else if (strcmpi(target, "Neutral") == 0)
		ret_val = BCT_NEUTRAL;
	else if (strcmpi(target, "None") != 0)
		ret_val = -1;

	return ret_val;
}

/**
 * Validates a skill's unit targets when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit targets should be set it.
 *
 **/
static void skill_validate_unit_target(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	skill->level_set_value(sk->unit_target, BCT_NOONE);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Target");

	if (t != NULL && config_setting_is_group(t)) {
		for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
			char lv[6]; // Big enough to contain "Lv999" in case of custom MAX_SKILL_LEVEL.
			snprintf(lv, sizeof(lv), "Lv%d", i + 1);
			const char *unit_target;

			if (libconfig->setting_lookup_string(t, lv, &unit_target) == CONFIG_TRUE) {
				int target = skill->validate_unit_target_sub(unit_target);

				if (target > BCT_NOONE)
					sk->unit_target[i] = target;
				else if (target == -1)
					ShowWarning("%s: Invalid unit target %s specified in level %d for skill ID %d in %s! Defaulting to None...\n",
						    __func__, unit_target, i + 1, sk->nameid, conf->file);
			}
		}
	} else {
		const char *unit_target;

		if (libconfig->setting_lookup_string(conf, "Target", &unit_target) == CONFIG_TRUE) {
			int target = skill->validate_unit_target_sub(unit_target);

			if (target > BCT_NOONE)
				skill->level_set_value(sk->unit_target, target);
			else if (target == -1)
				ShowWarning("%s: Invalid unit target %s specified for skill ID %d in %s! Defaulting to None...\n",
					    __func__, unit_target, sk->nameid, conf->file);
		}
	}

	for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
		if ((sk->unit_flag & UF_DEFNOTENEMY) != 0 && battle_config.defnotenemy != 0)
			sk->unit_target[i] = BCT_NOENEMY;

		// By default target just characters.
		sk->unit_target[i] |= BL_CHAR;

		if ((sk->unit_flag & UF_NOPC) != 0)
			sk->unit_target[i] &= ~BL_PC;

		if ((sk->unit_flag & UF_NOMOB) != 0)
			sk->unit_target[i] &= ~BL_MOB;

		if ((sk->unit_flag & UF_SKILL) != 0)
			sk->unit_target[i] |= BL_SKILL;
	}
}

/**
 * Validates a skill's status change when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit data should be set it.
 *
 **/
static void skill_validate_status_change(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	int status_id = SC_NONE;
	const char *name = NULL;
	libconfig->setting_lookup_string(conf, "StatusChange", &name);

	if (name != NULL && (!script->get_constant(name, &status_id) || status_id <= SC_NONE || status_id >= SC_MAX)) {
		ShowWarning("%s: Invalid status change %s specified for skill ID %d in %s! Defaulting to SC_NONE...\n", __func__, name, sk->nameid, conf->file);
		status_id = SC_NONE;
	}
	sk->status_type = status_id;
}

/**
 * Validates a skill's unit data when reading the skill DB.
 *
 * @param conf The libconfig settings block which contains the skill's data.
 * @param sk The s_skill_db struct where the unit data should be set it.
 *
 **/
static void skill_validate_unit(struct config_setting_t *conf, struct s_skill_db *sk)
{
	nullpo_retv(conf);
	nullpo_retv(sk);

	struct config_setting_t *t = libconfig->setting_get_member(conf, "Unit");

	if (t != NULL && config_setting_is_group(t)) {
		skill->validate_unit_id(t, sk);
		skill->validate_unit_layout(t, sk);
		skill->validate_unit_range(t, sk);
		skill->validate_unit_interval(t, sk);
		skill->validate_unit_flag(t, sk);
		skill->validate_unit_target(t, sk);
	}
}

/**
 * Validate additional field settings via plugins
 * when parsing skill_db.conf
 * @param   conf    struct, pointer to the skill configuration
 * @param   sk      struct, struct, pointer to s_skill_db
 * @return  (void)
 */
static void skill_validate_additional_fields(struct config_setting_t *conf, struct s_skill_db *sk)
{
	// Does nothing like a boss. *cough* plugins *cough*
}

/**
 * Reads a skill DB file from relative path.
 *
 * @param filename The skill DB's file name including the DB path.
 * @return True on success, otherwise false.
 *
 **/
static bool skill_read_skilldb(const char *filename)
{
	nullpo_retr(false, filename);

	char filepath[256];

	libconfig->format_db_path(filename, filepath, sizeof(filepath));

	if (!exists(filepath)) {
		ShowError("%s: Can't find file %s! Abort reading skills...\n", __func__, filepath);
		return false;
	}

	struct config_t skilldb;

	if (libconfig->load_file(&skilldb, filepath) == 0)
		return false; // Libconfig error report.

	struct config_setting_t *sk = libconfig->setting_get_member(skilldb.root, "skill_db");

	if (sk == NULL) {
		ShowError("%s: Skill DB could not be loaded! Please check %s.\n", __func__, filepath);
		libconfig->destroy(&skilldb);
		return false;
	}

	struct config_setting_t *conf;
	int index = 0;
	int count = 0;

	while ((conf = libconfig->setting_get_elem(sk, index++)) != NULL) {
		struct s_skill_db tmp_db = {0};

		/** Validate mandatory fields. **/
		skill->validate_id(conf, &tmp_db, index);
		if (tmp_db.nameid == 0)
			continue;

		skill->validate_name(conf, &tmp_db);
		if (*tmp_db.name == '\0')
			continue;

		skill->validate_max_level(conf, &tmp_db);
		if (tmp_db.max == 0)
			continue;

		/** Validate optional fields. **/
		skill->validate_description(conf, &tmp_db);
		skill->validate_range(conf, &tmp_db);
		skill->validate_hittype(conf, &tmp_db);
		skill->validate_skilltype(conf, &tmp_db);
		skill->validate_skillinfo(conf, &tmp_db);
		skill->validate_attacktype(conf, &tmp_db);
		skill->validate_element(conf, &tmp_db);
		skill->validate_damagetype(conf, &tmp_db);
		skill->validate_splash_range(conf, &tmp_db);
		skill->validate_number_of_hits(conf, &tmp_db);
		skill->validate_interrupt_cast(conf, &tmp_db);
		skill->validate_cast_def_rate(conf, &tmp_db);
		skill->validate_number_of_instances(conf, &tmp_db);
		skill->validate_knock_back_tiles(conf, &tmp_db);
		skill->validate_cast_time(conf, &tmp_db);
		skill->validate_act_delay(conf, &tmp_db);
		skill->validate_walk_delay(conf, &tmp_db);
		skill->validate_skill_data1(conf, &tmp_db);
		skill->validate_skill_data2(conf, &tmp_db);
		skill->validate_cooldown(conf, &tmp_db);
		skill->validate_fixed_cast_time(conf, &tmp_db);
		skill->validate_castnodex(conf, &tmp_db, false);
		skill->validate_castnodex(conf, &tmp_db, true);
		skill->validate_requirements(conf, &tmp_db);
		skill->validate_unit(conf, &tmp_db);
		skill->validate_status_change(conf, &tmp_db);

		/** Validate additional fields for plugins. **/
		skill->validate_additional_fields(conf, &tmp_db);

		/** Add the skill. **/
		skill->dbs->db[skill->get_index(tmp_db.nameid)] = tmp_db;
		strdb_iput(skill->name2id_db, tmp_db.name, tmp_db.nameid);
		script->set_constant2(tmp_db.name, tmp_db.nameid, false, false);
		count++;
	}

	libconfig->destroy(&skilldb);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filepath);
	return true;
}

/*===============================
 * DB reading.
 * produce_db.txt
 * create_arrow_db.txt
 * abra_db.txt
 *------------------------------*/
static void skill_readdb(bool minimal)
{
	// init skill db structures
	db_clear(skill->name2id_db);

	/* when != it was called during init and this procedure was already performed by skill_defaults()  */
	if( core->runflag == MAPSERVER_ST_RUNNING ) {
		memset(ZEROED_BLOCK_POS(skill->dbs), 0, ZEROED_BLOCK_SIZE(skill->dbs));
	}

	// load skill databases
	safestrncpy(skill->dbs->db[0].name, "UNKNOWN_SKILL", sizeof(skill->dbs->db[0].name));
	safestrncpy(skill->dbs->db[0].desc, "Unknown Skill", sizeof(skill->dbs->db[0].desc));

	itemdb->name_constants(); // refresh ItemDB constants before loading of skills

#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = DBPATH"skill_db.conf";
#endif // ENABLE_CASE_CHECK
	skill->read_skilldb(DBPATH"skill_db.conf");
#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = NULL;
#endif // ENABLE_CASE_CHECK

	if (minimal)
		return;

	skill->init_unit_layout();
	sv->readdb(map->db_path, "produce_db.txt",               ',',   4, 4+2*MAX_PRODUCE_RESOURCE,       MAX_SKILL_PRODUCE_DB, skill->parse_row_producedb);
	sv->readdb(map->db_path, "create_arrow_db.txt",          ',', 1+2,   1+2*MAX_ARROW_RESOURCE,         MAX_SKILL_ARROW_DB, skill->parse_row_createarrowdb);
	sv->readdb(map->db_path, "abra_db.txt",                  ',',   4,                        4,          MAX_SKILL_ABRA_DB, skill->parse_row_abradb);
	//Warlock
	sv->readdb(map->db_path, "spellbook_db.txt",             ',',   3,                        3,     MAX_SKILL_SPELLBOOK_DB, skill->parse_row_spellbookdb);
	//Guillotine Cross
	sv->readdb(map->db_path, "magicmushroom_db.txt",         ',',   1,                        1, MAX_SKILL_MAGICMUSHROOM_DB, skill->parse_row_magicmushroomdb);
	sv->readdb(map->db_path, "skill_improvise_db.txt",       ',',   2,                        2,     MAX_SKILL_IMPROVISE_DB, skill->parse_row_improvisedb);
	sv->readdb(map->db_path, "skill_changematerial_db.txt",  ',',   4,                    4+2*5,       MAX_SKILL_PRODUCE_DB, skill->parse_row_changematerialdb);
}

static void skill_reload(void)
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
static int do_init_skill(bool minimal)
{
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

static int do_final_skill(void)
{
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
void skill_defaults(void)
{
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
	skill->firerain_unit_pos = 0;
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
	skill->get_msp = skill_get_msp;
	skill->get_sp = skill_get_sp;
	skill->get_hp_rate = skill_get_hp_rate;
	skill->get_sp_rate = skill_get_sp_rate;
	skill->get_state = skill_get_state;
	skill->get_spiritball = skill_get_spiritball;
	skill->get_item_index = skill_get_item_index;
	skill->get_itemid = skill_get_itemid;
	skill->get_itemqty = skill_get_itemqty;
	skill->get_item_any_flag = skill_get_item_any_flag;
	skill->get_equip_id = skill_get_equip_id;
	skill->get_equip_amount = skill_get_equip_amount;
	skill->get_equip_any_flag = skill_get_equip_any_flag;
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
	skill->get_casttype = skill_get_casttype;
	skill->get_casttype2 = skill_get_casttype2;
	skill->get_sc_type = skill_get_sc_type;
	skill->is_combo = skill_is_combo;
	skill->name2id = skill_name2id;
	skill->isammotype = skill_isammotype;
	skill->castend_type = skill_castend_type;
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
	skill->check_condition_required_equip = skill_check_condition_required_equip;
	skill->check_condition_castbegin = skill_check_condition_castbegin;
	skill->check_condition_required_items = skill_check_condition_required_items;
	skill->items_required = skill_items_required;
	skill->check_condition_castend = skill_check_condition_castend;
	skill->get_any_item_index = skill_get_any_item_index;
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
	skill->not_ok_unknown = skill_notok_unknown;
	skill->not_ok_hom = skillnotok_hom;
	skill->not_ok_hom_unknown = skillnotok_hom_unknown;
	skill->not_ok_mercenary = skillnotok_mercenary;
	skill->validate_autocast_data = skill_validate_autocast_data;
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
	skill->bind_trap = skill_bind_trap;
	skill->cell_overlap = skill_cell_overlap;
	skill->timerskill = skill_timerskill;
	skill->trap_do_splash = skill_trap_do_splash;
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
	skill->unit_onplace_timer_unknown = skill_unit_onplace_timer_unknown;
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
	skill->init_unit_layout_unknown = skill_init_unit_layout_unknown;
	/* Skill DB Libconfig */
	skill->validate_id = skill_validate_id;
	skill->name_contains_invalid_character = skill_name_contains_invalid_character;
	skill->validate_name = skill_validate_name;
	skill->validate_max_level = skill_validate_max_level;
	skill->validate_description = skill_validate_description;
	skill->validate_range = skill_validate_range;
	skill->validate_hittype = skill_validate_hittype;
	skill->validate_skilltype = skill_validate_skilltype;
	skill->validate_skillinfo = skill_validate_skillinfo;
	skill->validate_attacktype = skill_validate_attacktype;
	skill->validate_element = skill_validate_element;
	skill->validate_damagetype = skill_validate_damagetype;
	skill->validate_splash_range = skill_validate_splash_range;
	skill->validate_number_of_hits = skill_validate_number_of_hits;
	skill->validate_interrupt_cast = skill_validate_interrupt_cast;
	skill->validate_cast_def_rate = skill_validate_cast_def_rate;
	skill->validate_number_of_instances = skill_validate_number_of_instances;
	skill->validate_knock_back_tiles = skill_validate_knock_back_tiles;
	skill->validate_cast_time = skill_validate_cast_time;
	skill->validate_act_delay = skill_validate_act_delay;
	skill->validate_walk_delay = skill_validate_walk_delay;
	skill->validate_skill_data1 = skill_validate_skill_data1;
	skill->validate_skill_data2 = skill_validate_skill_data2;
	skill->validate_cooldown = skill_validate_cooldown;
	skill->validate_fixed_cast_time = skill_validate_fixed_cast_time;
	skill->validate_castnodex = skill_validate_castnodex;
	skill->validate_hp_cost = skill_validate_hp_cost;
	skill->validate_sp_cost = skill_validate_sp_cost;
	skill->validate_hp_rate_cost = skill_validate_hp_rate_cost;
	skill->validate_sp_rate_cost = skill_validate_sp_rate_cost;
	skill->validate_max_hp_trigger = skill_validate_max_hp_trigger;
	skill->validate_max_sp_trigger = skill_validate_max_sp_trigger;
	skill->validate_zeny_cost = skill_validate_zeny_cost;
	skill->validate_weapontype_sub = skill_validate_weapontype_sub;
	skill->validate_weapontype = skill_validate_weapontype;
	skill->validate_ammotype_sub = skill_validate_ammotype_sub;
	skill->validate_ammotype = skill_validate_ammotype;
	skill->validate_ammo_amount = skill_validate_ammo_amount;
	skill->validate_state_sub = skill_validate_state_sub;
	skill->validate_state = skill_validate_state;
	skill->validate_spirit_sphere_cost = skill_validate_spirit_sphere_cost;
	skill->validate_item_requirements_sub_item_amount = skill_validate_item_requirements_sub_item_amount;
	skill->validate_item_requirements_sub_items = skill_validate_item_requirements_sub_items;
	skill->validate_item_requirements_sub_any_flag = skill_validate_item_requirements_sub_any_flag;
	skill->validate_item_requirements = skill_validate_item_requirements;
	skill->validate_equip_requirements_sub_item_amount = skill_validate_equip_requirements_sub_item_amount;
	skill->validate_equip_requirements_sub_items = skill_validate_equip_requirements_sub_items;
	skill->validate_equip_requirements_sub_any_flag = skill_validate_equip_requirements_sub_any_flag;
	skill->validate_equip_requirements = skill_validate_equip_requirements;
	skill->validate_requirements_item_name = skill_validate_requirements_item_name;
	skill->validate_requirements = skill_validate_requirements;
	skill->validate_unit_id_sub = skill_validate_unit_id_sub;
	skill->validate_unit_id = skill_validate_unit_id;
	skill->validate_unit_layout = skill_validate_unit_layout;
	skill->validate_unit_range = skill_validate_unit_range;
	skill->validate_unit_interval = skill_validate_unit_interval;
	skill->validate_unit_flag_sub = skill_validate_unit_flag_sub;
	skill->validate_unit_flag = skill_validate_unit_flag;
	skill->validate_unit_target_sub = skill_validate_unit_target_sub;
	skill->validate_unit_target = skill_validate_unit_target;
	skill->validate_unit = skill_validate_unit;
	skill->validate_status_change = skill_validate_status_change;
	skill->validate_additional_fields = skill_validate_additional_fields;
	skill->read_skilldb = skill_read_skilldb;
	skill->config_set_level = skill_config_set_level;
	skill->level_set_value = skill_level_set_value;
	/* */
	skill->parse_row_producedb = skill_parse_row_producedb;
	skill->parse_row_createarrowdb = skill_parse_row_createarrowdb;
	skill->parse_row_abradb = skill_parse_row_abradb;
	skill->parse_row_spellbookdb = skill_parse_row_spellbookdb;
	skill->parse_row_magicmushroomdb = skill_parse_row_magicmushroomdb;
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
	skill->splash_target = skill_splash_target;
	skill->check_npc_chaospanic = skill_check_npc_chaospanic;
	skill->count_wos = skill_count_wos;
	skill->add_bard_dancer_soullink_songs = skill_add_bard_dancer_soullink_songs;
}
