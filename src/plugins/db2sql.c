/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2020 Hercules Dev Team
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
#include "config/core.h"

#include "common/hercules.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "map/battle.h"
#include "map/itemdb.h"
#include "map/mob.h"
#include "map/map.h"
#include "map/pc.h"

#include "common/HPMDataCheck.h"

#include <stdio.h>
#include <stdlib.h>

HPExport struct hplugin_info pinfo = {
	"DB2SQL",        // Plugin name
	SERVER_TYPE_MAP, // Which server types this plugin works with?
	"0.5",           // Plugin version
	HPM_VERSION,     // HPM Version (don't change, macro is automatically updated)
};

#ifdef RENEWAL
#define DBSUFFIX "_re"
#else // not RENEWAL
#define DBSUFFIX ""
#endif

/// Conversion state tracking.
struct {
	FILE *fp; ///< Currently open file pointer
	struct {
		char *p;    ///< Buffer pointer
		size_t len; ///< Buffer length
	} buf[4]; ///< Output buffer
	const char *db_name; ///< Database table name
} tosql;

/// Whether the item_db converter will automatically run.
bool itemdb2sql_torun = false;
/// Whether the mob_db converter will automatically run.
bool mobdb2sql_torun = false;
/// mysql handle for escape strings
static struct Sql *sql_handle = NULL;

/// Backup of the original item_db parser function pointer.
int (*itemdb_readdb_libconfig_sub) (struct config_setting_t *it, int n, const char *source);
/// Backup of the original mob_db parser function pointer.
int (*mob_read_db_sub) (struct config_setting_t *it, int n, const char *source);
bool (*mob_skill_db_libconfig_sub_skill) (struct config_setting_t *it, int n, int mob_id);

//
void do_mobskilldb2sql(void);

/**
 * Normalizes and appends a string to the output buffer.
 *
 * @param str The string to append.
 */
void hstr(const char *str)
{
	nullpo_retv(str);
	if (strlen(str) > tosql.buf[3].len) {
		tosql.buf[3].len = tosql.buf[3].len + strlen(str) + 1000;
		RECREATE(tosql.buf[3].p,char,tosql.buf[3].len);
	}
	safestrncpy(tosql.buf[3].p,str,strlen(str));
	normalize_name(tosql.buf[3].p,"\t\n ");
}

/**
 * Prints a SQL file header for the current item_db file.
 */
void db2sql_fileheader(void)
{
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	int year = lt->tm_year+1900;

	fprintf(tosql.fp,
			"-- This file is part of Hercules.\n"
			"-- http://herc.ws - http://github.com/HerculesWS/Hercules\n"
			"--\n"
			"-- Copyright (C) 2013-%d Hercules Dev Team\n"
			"--\n"
			"-- Hercules is free software: you can redistribute it and/or modify\n"
			"-- it under the terms of the GNU General Public License as published by\n"
			"-- the Free Software Foundation, either version 3 of the License, or\n"
			"-- (at your option) any later version.\n"
			"--\n"
			"-- This program is distributed in the hope that it will be useful,\n"
			"-- but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
			"-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
			"-- GNU General Public License for more details.\n"
			"--\n"
			"-- You should have received a copy of the GNU General Public License\n"
			"-- along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"

			"-- NOTE: This file was auto-generated and should never be manually edited,\n"
			"--       as it will get overwritten. If you need to modify this file,\n"
			"--       please consider modifying the corresponding .conf file inside\n"
			"--       the db folder, and then re-run the db2sql plugin.\n\n"

			"-- GENERATED FILE DO NOT EDIT --\n"
			"\n", year);
}

/**
 * Converts the Job field of an Item DB entry to the numeric format used in the SQL table.
 */
uint64 itemdb2sql_readdb_job_sub(struct config_setting_t *t)
{
	uint64 jobmask = 0;
	int idx = 0;
	struct config_setting_t *it = NULL;
	bool enable_all = false;

	if (libconfig->setting_lookup_bool_real(t, "All", &enable_all) && enable_all) {
		jobmask |= UINT64_MAX;
	}
	while ((it = libconfig->setting_get_elem(t, idx++)) != NULL) {
		const char *job_name = config_setting_name(it);
		int job_id;

		if (strcmp(job_name, "All") == 0)
			continue;

		if ((job_id = pc->check_job_name(job_name)) != -1) {
			uint64 newmask = 0;
			switch (job_id) {
				// Base Classes
				case JOB_NOVICE:
				case JOB_SUPER_NOVICE:
					newmask = 1ULL << JOB_NOVICE;
					break;
				case JOB_SWORDMAN:
				case JOB_MAGE:
				case JOB_ARCHER:
				case JOB_ACOLYTE:
				case JOB_MERCHANT:
				case JOB_THIEF:
				// 2-1 Classes
				case JOB_KNIGHT:
				case JOB_PRIEST:
				case JOB_WIZARD:
				case JOB_BLACKSMITH:
				case JOB_HUNTER:
				case JOB_ASSASSIN:
				// 2-2 Classes
				case JOB_CRUSADER:
				case JOB_MONK:
				case JOB_SAGE:
				case JOB_ALCHEMIST:
				case JOB_BARD:
				case JOB_DANCER:
				case JOB_ROGUE:
				// Extended Classes
				case JOB_GUNSLINGER:
				case JOB_NINJA:
					newmask = 1ULL << job_id;
					break;
				// Extended Classes (special handling)
				case JOB_TAEKWON:
					newmask = 1ULL << 21;
					break;
				case JOB_STAR_GLADIATOR:
					newmask = 1ULL << 22;
					break;
				case JOB_SOUL_LINKER:
					newmask = 1ULL << 23;
					break;
				// Other Classes
				case JOB_GANGSI: //Bongun/Munak
					newmask = 1ULL << 26;
					break;
				case JOB_DEATH_KNIGHT:
					newmask = 1ULL << 27;
					break;
				case JOB_DARK_COLLECTOR:
					newmask = 1ULL << 28;
					break;
				case JOB_KAGEROU:
				case JOB_OBORO:
					newmask = 1ULL << 29;
					break;
				case JOB_REBELLION:
					newmask = 1ULL << 30;
					break;
			}

			if (libconfig->setting_get_bool(it)) {
				jobmask |= newmask;
			} else {
				jobmask &= ~newmask;
			}
		}
	}

	return jobmask;
}

/**
 * Converts an Item DB entry to SQL.
 *
 * @see itemdb_readdb_libconfig_sub.
 */
int itemdb2sql_sub(struct config_setting_t *entry, int n, const char *source)
{
	struct item_data *it = NULL;

	if ((it = itemdb->exists(itemdb_readdb_libconfig_sub(entry,n,source)))) {
		char e_name[ITEM_NAME_LENGTH*2+1];
		const char *bonus = NULL;
		char *str;
		int i32;
		uint32 ui32;
		uint64 ui64;
		struct config_setting_t *t = NULL;
		StringBuf buf;

		nullpo_ret(entry);

		StrBuf->Init(&buf);

		// id
		StrBuf->Printf(&buf, "'%u',", (uint32)it->nameid);

		// name_english
		SQL->EscapeString(sql_handle, e_name, it->name);
		StrBuf->Printf(&buf, "'%s',", e_name);

		// name_japanese
		SQL->EscapeString(sql_handle, e_name, it->jname);
		StrBuf->Printf(&buf, "'%s',", e_name);

		// type
		StrBuf->Printf(&buf, "'%d',", it->flag.delay_consume ? IT_DELAYCONSUME : it->type);

		// subtype
		StrBuf->Printf(&buf, "'%d',", it->subtype);

		// price_buy
		StrBuf->Printf(&buf, "'%d',", it->value_buy);

		// price_sell
		StrBuf->Printf(&buf, "'%d',", it->value_sell);

		// weight
		StrBuf->Printf(&buf, "'%d',", it->weight);

		// atk
		StrBuf->Printf(&buf, "'%d',", it->atk);

		// matk
		StrBuf->Printf(&buf, "'%d',", it->matk);

		// defence
		StrBuf->Printf(&buf, "'%d',", it->def);

		// range
		StrBuf->Printf(&buf, "'%d',", it->range);

		// slots
		StrBuf->Printf(&buf, "'%d',", it->slot);

		// equip_jobs
		if ((t = libconfig->setting_get_member(entry, "Job")) != NULL) {
			if (config_setting_is_group(t)) {
				ui64 = itemdb2sql_readdb_job_sub(t);
			} else if (itemdb->lookup_const(entry, "Job", &i32)) { // This is an unsigned value, do not check for >= 0
				ui64 = (uint64)i32;
			} else {
				ui64 = UINT64_MAX;
			}
		} else {
			ui64 = UINT64_MAX;
		}
		StrBuf->Printf(&buf, "'%"PRIu64"',", ui64);

		// equip_upper
		if (itemdb->lookup_const_mask(entry, "Upper", &i32) && i32 >= 0)
			ui32 = (uint32)i32;
		else
			ui32 = ITEMUPPER_ALL;

		StrBuf->Printf(&buf, "'%u',", ui32);

		// equip_genders
		StrBuf->Printf(&buf, "'%d',", it->sex);

		// equip_locations
		StrBuf->Printf(&buf, "'%d',", it->equip);

		// weapon_level
		StrBuf->Printf(&buf, "'%d',", it->wlv);

		// equip_level_min
		StrBuf->Printf(&buf, "'%d',", it->elv);

		// equip_level_max
		if ((t = libconfig->setting_get_member(entry, "EquipLv")) && config_setting_is_aggregate(t) && libconfig->setting_length(t) >= 2)
			StrBuf->Printf(&buf, "'%d',", it->elvmax);
		else
			StrBuf->AppendStr(&buf, "NULL,");

		// refineable
		StrBuf->Printf(&buf, "'%d',", it->flag.no_refine?0:1);

		// disable_options
		StrBuf->Printf(&buf, "'%d',", it->flag.no_options?1:0);

		// view_sprite
		StrBuf->Printf(&buf, "'%d',", it->view_sprite);

		// bindonequip
		StrBuf->Printf(&buf, "'%d',", it->flag.bindonequip?1:0);

		// forceserial
		StrBuf->Printf(&buf, "'%d',", it->flag.force_serial?1:0);

		// buyingstore
		StrBuf->Printf(&buf, "'%d',", it->flag.buyingstore?1:0);

		// delay
		StrBuf->Printf(&buf, "'%d',", it->delay);

		// trade_flag
		StrBuf->Printf(&buf, "'%d',", it->flag.trade_restriction);

		// trade_group
		if (it->flag.trade_restriction != ITR_NONE && it->gm_lv_trade_override > 0 && it->gm_lv_trade_override < 100) {
			StrBuf->Printf(&buf, "'%d',", it->gm_lv_trade_override);
		} else {
			StrBuf->AppendStr(&buf, "NULL,");
		}

		// nouse_flag
		StrBuf->Printf(&buf, "'%u',", it->item_usage.flag);

		// nouse_group
		if (it->item_usage.flag != INR_NONE && it->item_usage.override > 0 && it->item_usage.override < 100) {
			StrBuf->Printf(&buf, "'%u',", it->item_usage.override);
		} else {
			StrBuf->AppendStr(&buf, "NULL,");
		}

		// stack_amount
		StrBuf->Printf(&buf, "'%u',", it->stack.amount);

		// stack_flag
		if (it->stack.amount) {
			uint32 value = 0; // FIXME: Use an enum
			value |= it->stack.inventory ? 1 : 0;
			value |= it->stack.cart ? 2 : 0;
			value |= it->stack.storage ? 4 : 0;
			value |= it->stack.guildstorage ? 8 : 0;
			StrBuf->Printf(&buf, "'%u',", value);
		} else {
			StrBuf->AppendStr(&buf, "NULL,");
		}

		// sprite
		if (it->flag.available) {
			StrBuf->Printf(&buf, "'%d',", it->view_id);
		} else {
			StrBuf->AppendStr(&buf, "NULL,");
		}

		// script
		if (it->script && libconfig->setting_lookup_string(entry, "Script", &bonus)) {
			hstr(bonus);
			str = tosql.buf[3].p;
			if (strlen(str) > tosql.buf[0].len) {
				tosql.buf[0].len = tosql.buf[0].len + strlen(str) + 1000;
				RECREATE(tosql.buf[0].p,char,tosql.buf[0].len);
			}
			SQL->EscapeString(sql_handle, tosql.buf[0].p, str);
			StrBuf->Printf(&buf, "'%s',", tosql.buf[0].p);
		} else {
			StrBuf->AppendStr(&buf, "'',");
		}

		// equip_script
		if (it->equip_script && libconfig->setting_lookup_string(entry, "OnEquipScript", &bonus)) {
			hstr(bonus);
			str = tosql.buf[3].p;
			if (strlen(str) > tosql.buf[1].len) {
				tosql.buf[1].len = tosql.buf[1].len + strlen(str) + 1000;
				RECREATE(tosql.buf[1].p,char,tosql.buf[1].len);
			}
			SQL->EscapeString(sql_handle, tosql.buf[1].p, str);
			StrBuf->Printf(&buf, "'%s',", tosql.buf[1].p);
		} else {
			StrBuf->AppendStr(&buf, "'',");
		}

		// unequip_script
		if (it->unequip_script && libconfig->setting_lookup_string(entry, "OnUnequipScript", &bonus)) {
			hstr(bonus);
			str = tosql.buf[3].p;
			if (strlen(str) > tosql.buf[2].len) {
				tosql.buf[2].len = tosql.buf[2].len + strlen(str) + 1000;
				RECREATE(tosql.buf[2].p,char,tosql.buf[2].len);
			}
			SQL->EscapeString(sql_handle, tosql.buf[2].p, str);
			StrBuf->Printf(&buf, "'%s'", tosql.buf[2].p);
		} else {
			StrBuf->AppendStr(&buf, "''");
		}

		fprintf(tosql.fp, "REPLACE INTO `%s` VALUES (%s);\n", tosql.db_name, StrBuf->Value(&buf));

		StrBuf->Destroy(&buf);
	}

	return it?it->nameid:0;
}

/**
 * Prints a SQL table header for the current item_db table.
 */
void itemdb2sql_tableheader(void)
{
	db2sql_fileheader();

	fprintf(tosql.fp,
			"--\n"
			"-- Table structure for table `%s`\n"
			"--\n"
			"\n"
			"DROP TABLE IF EXISTS `%s`;\n"
			"CREATE TABLE `%s` (\n"
			"  `id` int UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `name_english` varchar(50) NOT NULL DEFAULT '',\n"
			"  `name_japanese` varchar(50) NOT NULL DEFAULT '',\n"
			"  `type` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `subtype` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `price_buy` mediumint DEFAULT NULL,\n"
			"  `price_sell` mediumint DEFAULT NULL,\n"
			"  `weight` smallint UNSIGNED DEFAULT NULL,\n"
			"  `atk` smallint UNSIGNED DEFAULT NULL,\n"
			"  `matk` smallint UNSIGNED DEFAULT NULL,\n"
			"  `defence` smallint UNSIGNED DEFAULT NULL,\n"
			"  `range` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `slots` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `equip_jobs` bigint UNSIGNED DEFAULT NULL,\n"
			"  `equip_upper` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `equip_genders` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `equip_locations` mediumint UNSIGNED DEFAULT NULL,\n"
			"  `weapon_level` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `equip_level_min` smallint UNSIGNED DEFAULT NULL,\n"
			"  `equip_level_max` smallint UNSIGNED DEFAULT NULL,\n"
			"  `refineable` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `disable_options` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `view_sprite` smallint UNSIGNED DEFAULT NULL,\n"
			"  `bindonequip` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `forceserial` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `buyingstore` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `delay` mediumint UNSIGNED DEFAULT NULL,\n"
			"  `trade_flag` smallint UNSIGNED DEFAULT NULL,\n"
			"  `trade_group` smallint UNSIGNED DEFAULT NULL,\n"
			"  `nouse_flag` smallint UNSIGNED DEFAULT NULL,\n"
			"  `nouse_group` smallint UNSIGNED DEFAULT NULL,\n"
			"  `stack_amount` mediumint UNSIGNED DEFAULT NULL,\n"
			"  `stack_flag` tinyint UNSIGNED DEFAULT NULL,\n"
			"  `sprite` mediumint UNSIGNED DEFAULT NULL,\n"
			"  `script` text,\n"
			"  `equip_script` text,\n"
			"  `unequip_script` text,\n"
			" PRIMARY KEY (`id`)\n"
			") ENGINE=MyISAM;\n"
			"\n", tosql.db_name,tosql.db_name,tosql.db_name);
}

/**
 * Item DB Conversion.
 *
 * Converts Item DB and Item DB2 to SQL scripts.
 */
void do_itemdb2sql(void)
{
	int i;
	struct convert_db_files {
		const char *name;
		const char *source;
		const char *destination;
	} files[] = {
		{"item_db", DBPATH"item_db.conf", "sql-files/item_db" DBSUFFIX ".sql"},
		{"item_db2", "item_db2.conf", "sql-files/item_db2.sql"},
	};

	/* link */
	itemdb_readdb_libconfig_sub = itemdb->readdb_libconfig_sub;
	itemdb->readdb_libconfig_sub = itemdb2sql_sub;

	memset(&tosql.buf, 0, sizeof(tosql.buf));
	itemdb->clear(false);

	for (i = 0; i < ARRAYLENGTH(files); i++) {
		if ((tosql.fp = fopen(files[i].destination, "wt+")) == NULL) {
			ShowError("itemdb_tosql: File not found \"%s\".\n", files[i].destination);
			return;
		}

		tosql.db_name = files[i].name;
		itemdb2sql_tableheader();

		itemdb->readdb_libconfig(files[i].source);

		fclose(tosql.fp);
	}

	/* unlink */
	itemdb->readdb_libconfig_sub = itemdb_readdb_libconfig_sub;

	for (i = 0; i < ARRAYLENGTH(tosql.buf); i++) {
		if (tosql.buf[i].p)
			aFree(tosql.buf[i].p);
	}
}

/**
 * Converts a Mob DB entry to SQL.
 *
 * @see mobdb_readdb_libconfig_sub.
 */
int mobdb2sql_sub(struct config_setting_t *mobt, int n, const char *source)
{
	struct mob_db *md = NULL;
	nullpo_ret(mobt);

	if ((md = mob->db(mob_read_db_sub(mobt, n, source))) != mob->dummy) {
		char e_name[NAME_LENGTH*2+1];
		StringBuf buf;
		int card_idx = 9, i;

		StrBuf->Init(&buf);

		// id
		StrBuf->Printf(&buf, "%d,", md->mob_id);

		// Sprite
		SQL->EscapeString(sql_handle, e_name, md->sprite);
		StrBuf->Printf(&buf, "'%s',", e_name);

		// kName
		SQL->EscapeString(sql_handle, e_name, md->name);
		StrBuf->Printf(&buf, "'%s',", e_name);

		// iName
		SQL->EscapeString(sql_handle, e_name, md->jname);
		StrBuf->Printf(&buf, "'%s',", e_name);

		// LV
		StrBuf->Printf(&buf, "%u,", md->lv);

		// HP
		StrBuf->Printf(&buf, "%u,", md->status.max_hp);

		// SP
		StrBuf->Printf(&buf, "%u,", md->status.max_sp);

		// EXP
		StrBuf->Printf(&buf, "%u,", md->base_exp);

		// JEXP
		StrBuf->Printf(&buf, "%u,", md->job_exp);

		// Range1
		StrBuf->Printf(&buf, "%u,", md->status.rhw.range);

		// ATK1
		StrBuf->Printf(&buf, "%u,", md->status.rhw.atk);

		// ATK2
		StrBuf->Printf(&buf, "%u,", md->status.rhw.atk2);

		// DEF
		StrBuf->Printf(&buf, "%d,", md->status.def);

		// MDEF
		StrBuf->Printf(&buf, "%d,", md->status.mdef);

		// STR
		StrBuf->Printf(&buf, "%u,", md->status.str);

		// AGI
		StrBuf->Printf(&buf, "%u,", md->status.agi);

		// VIT
		StrBuf->Printf(&buf, "%u,", md->status.vit);

		// INT
		StrBuf->Printf(&buf, "%u,", md->status.int_);

		// DEX
		StrBuf->Printf(&buf, "%u,", md->status.dex);

		// LUK
		StrBuf->Printf(&buf, "%u,", md->status.luk);

		// Range2
		StrBuf->Printf(&buf, "%d,", md->range2);

		// Range3
		StrBuf->Printf(&buf, "%d,", md->range3);

		// Scale
		StrBuf->Printf(&buf, "%u,", md->status.size);

		// Race
		StrBuf->Printf(&buf, "%u,", md->status.race);

		// Element
		StrBuf->Printf(&buf, "%d,", md->status.def_ele + 20 * md->status.ele_lv);

		// Mode
		StrBuf->Printf(&buf, "%u,", md->status.mode);

		// Speed
		StrBuf->Printf(&buf, "%u,", md->status.speed);

		// aDelay
		StrBuf->Printf(&buf, "%u,", md->status.adelay);

		// aMotion
		StrBuf->Printf(&buf, "%u,", md->status.amotion);

		// dMotion
		StrBuf->Printf(&buf, "%u,", md->status.dmotion);

		// MEXP
		StrBuf->Printf(&buf, "%u,", md->mexp);

		for (i = 0; i < 3; i++) {
			// MVP{i}id
			StrBuf->Printf(&buf, "%d,", md->mvpitem[i].nameid);
			// MVP{i}per
			StrBuf->Printf(&buf, "%d,", md->mvpitem[i].p);
		}

		// Scan for cards
		for (i = 0; i < 10; i++) {
			struct item_data *it = NULL;
			if (md->dropitem[i].nameid != 0 && (it = itemdb->exists(md->dropitem[i].nameid)) != NULL && it->type == IT_CARD)
				card_idx = i;
		}

		for (i = 0; i < 10; i++) {
			if (card_idx == i)
				continue;
			// Drop{i}id
			StrBuf->Printf(&buf, "%d,", md->dropitem[i].nameid);
			// Drop{i}per
			StrBuf->Printf(&buf, "%d,", md->dropitem[i].p);
		}

		// DropCardid
		StrBuf->Printf(&buf, "%d,", md->dropitem[card_idx].nameid);
		// DropCardper
		StrBuf->Printf(&buf, "%d", md->dropitem[card_idx].p);

		fprintf(tosql.fp, "REPLACE INTO `%s` VALUES (%s);\n", tosql.db_name, StrBuf->Value(&buf));

		StrBuf->Destroy(&buf);
	}

	return md ? md->mob_id : 0;
}

/**
 * Prints a SQL table header for the current mob_db table.
 */
void mobdb2sql_tableheader(void)
{
	db2sql_fileheader();

	fprintf(tosql.fp,
			"--\n"
			"-- Table structure for table `%s`\n"
			"--\n"
			"\n"
			"DROP TABLE IF EXISTS `%s`;\n"
			"CREATE TABLE `%s` (\n"
			"  `ID` mediumint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Sprite` text NOT NULL,\n"
			"  `kName` text NOT NULL,\n"
			"  `iName` text NOT NULL,\n"
			"  `LV` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `HP` int UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `SP` mediumint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `EXP` mediumint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `JEXP` mediumint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Range1` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `ATK1` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `ATK2` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `DEF` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MDEF` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `STR` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `AGI` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `VIT` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `INT` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `DEX` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `LUK` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Range2` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Range3` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Scale` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Race` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Element` tinyint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Mode` int UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Speed` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `aDelay` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `aMotion` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `dMotion` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MEXP` mediumint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MVP1id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MVP1per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MVP2id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MVP2per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MVP3id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `MVP3per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop1id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop1per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop2id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop2per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop3id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop3per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop4id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop4per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop5id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop5per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop6id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop6per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop7id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop7per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop8id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop8per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop9id` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `Drop9per` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `DropCardid` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `DropCardper` smallint UNSIGNED NOT NULL DEFAULT '0',\n"
			"  PRIMARY KEY (`ID`)\n"
			") ENGINE=MyISAM;\n"
			"\n", tosql.db_name, tosql.db_name, tosql.db_name);
}

/**
 * Mob DB Conversion.
 *
 * Converts Mob DB and Mob DB2 to SQL scripts.
 */
void do_mobdb2sql(void)
{
	int i;
	struct convert_db_files {
		const char *name;
		const char *source;
		const char *destination;
	} files[] = {
		{"mob_db", DBPATH"mob_db.conf", "sql-files/mob_db" DBSUFFIX ".sql"},
		{"mob_db2", "mob_db2.conf", "sql-files/mob_db2.sql"},
	};

	/* link */
	mob_read_db_sub = mob->read_db_sub;
	mob->read_db_sub = mobdb2sql_sub;

	if (map->minimal) {
		// Set up modifiers
		battle->config_set_defaults();
	}

	memset(&tosql.buf, 0, sizeof(tosql.buf));
	for (i = 0; i < ARRAYLENGTH(files); i++) {
		if ((tosql.fp = fopen(files[i].destination, "wt+")) == NULL) {
			ShowError("mobdb_tosql: File not found \"%s\".\n", files[i].destination);
			return;
		}

		tosql.db_name = files[i].name;
		mobdb2sql_tableheader();

		mob->read_libconfig(files[i].source, false);

		fclose(tosql.fp);
	}

	/* unlink */
	mob->read_db_sub = mob_read_db_sub;

	for (i = 0; i < ARRAYLENGTH(tosql.buf); i++) {
		if (tosql.buf[i].p)
			aFree(tosql.buf[i].p);
	}

	// Run mob_skill_db converter
	do_mobskilldb2sql();
}

/**
 * Converts Mob Skill State constant to string
 */
const char* mob_skill_state_tostring(enum MobSkillState mss)
{
	switch(mss) {
	case MSS_ANY:
		return "any";
	case MSS_IDLE:
		return "idle";
	case MSS_WALK:
		return "walk";
	case MSS_LOOT:
		return "loot";
	case MSS_DEAD:
		return "dead";
	case MSS_BERSERK:
		return "attack";
	case MSS_ANGRY:
		return "angry";
	case MSS_RUSH:
		return "chase";
	case MSS_FOLLOW:
		return "follow";
	case MSS_ANYTARGET:
		return "anytarget";
	}

	return "unknown";
}

/**
 * Converts Mob Skill Target constant to string
 */
const char* mob_skill_target_tostring(int target)
{
	switch(target) {
	case MST_TARGET:
		return "target";
	case MST_RANDOM:
		return "randomtarget";
	case MST_SELF:
		return "self";
	case MST_FRIEND:
		return "friend";
	case MST_MASTER:
		return "master";
	case MST_AROUND1:
		return "around1";
	case MST_AROUND2:
		return "around2";
	case MST_AROUND3:
		return "around3";
	//case MST_AROUND: // same value as MST_AROUND4
	case MST_AROUND4:
		return "around4";
	case MST_AROUND5:
		return "around5";
	case MST_AROUND6:
		return "around6";
	case MST_AROUND7:
		return "around7";
	case MST_AROUND8:
		return "around8";
	}
	return "unknown";
}

/**
 * Converts Mob Skill Condition constant to string
 */
const char* mob_skill_condition_tostring(int condition)
{
	switch(condition) {
	case MSC_ALWAYS:
		return "always";
	case MSC_MYHPLTMAXRATE:
		return "myhpltmaxrate";
	case MSC_MYHPINRATE:
		return "myhpinrate";
	case MSC_FRIENDHPLTMAXRATE:
		return "friendhpltmaxrate";
	case MSC_FRIENDHPINRATE:
		return "friendhpinrate";
	case MSC_MYSTATUSON:
		return "mystatuson";
	case MSC_MYSTATUSOFF:
		return "mystatusoff";
	case MSC_FRIENDSTATUSON:
		return "friendstatuson";
	case MSC_FRIENDSTATUSOFF:
		return "friendstatusoff";
	case MSC_ATTACKPCGT:
		return "attackpcgt";
	case MSC_ATTACKPCGE:
		return "attackpcge";
	case MSC_SLAVELT:
		return "slavelt";
	case MSC_SLAVELE:
		return "slavele";
	case MSC_CLOSEDATTACKED:
		return "closedattacked";
	case MSC_LONGRANGEATTACKED:
		return "longrangeattacked";
	case MSC_AFTERSKILL:
		return "afterskill";
	case MSC_SKILLUSED:
		return "skillused";
	case MSC_CASTTARGETED:
		return "casttargeted";
	case MSC_RUDEATTACKED:
		return "rudeattacked";
	case MSC_MASTERHPLTMAXRATE:
		return "masterhpltmaxrate";
	case MSC_MASTERATTACKED:
		return "masterattacked";
	case MSC_ALCHEMIST:
		return "alchemist";
	case MSC_SPAWN:
		return "onspawn";
	}
	return "unknown";
}

/**
 * Converts a Mob Skill DB entry to SQL.
 *
 * @see mob_skill_db_libconfig_sub_skill.
 */
bool mobskilldb2sql_sub(struct config_setting_t *it, int n, int mob_id)
{
	int i32 = 0, i;
	StringBuf buf;
	struct mob_db *md = mob->db(mob_id);
	char valname[15];
	const char *name = config_setting_name(it);
	char e_name[NAME_LENGTH*2+1];

	nullpo_retr(false, it);
	Assert_retr(false, mob_id <= 0 || md != mob->dummy);

	StrBuf->Init(&buf);

	// MonsterID
	StrBuf->Printf(&buf, "%d,", mob_id);

	// Info
	SQL->EscapeString(sql_handle, e_name, md->name);
	StrBuf->Printf(&buf, "'%s@%s',", e_name, name);

	if (mob->lookup_const(it, "SkillState", &i32) && (i32 < MSS_ANY || i32 > MSS_ANYTARGET)) {
		ShowWarning("mob_skill_db_libconfig_sub_skill: Invalid skill state %d for skill '%s' in monster %d, defaulting to MSS_ANY.\n", i32, name, mob_id);
		i32 = MSS_ANY;
	}
	// State
	StrBuf->Printf(&buf, "'%s',", mob_skill_state_tostring(i32));

	// SkillID
	if (!(i32 = skill->name2id(name))) {
		ShowWarning("mob_skill_db_libconfig_sub_skill: Non existant skill id %d in monster %d, skipping.\n", i32, mob_id);
		return false;
	}
	StrBuf->Printf(&buf, "%d,", i32);

	// SkillLv
	if (!libconfig->setting_lookup_int(it, "SkillLevel", &i32) || i32 <= 0)
		i32 = 1;
	StrBuf->Printf(&buf, "%d,", i32);

	// Rate
	i32 = 0;
	libconfig->setting_lookup_int(it, "Rate", &i32);
	StrBuf->Printf(&buf, "%d,", i32);

	// CastTime
	i32 = 0;
	libconfig->setting_lookup_int(it, "CastTime", &i32);
	StrBuf->Printf(&buf, "%d,", i32);

	// Delay
	i32 = 0;
	libconfig->setting_lookup_int(it, "Delay", &i32);
	StrBuf->Printf(&buf, "%d,", i32);

	// Cancelable
	if (libconfig->setting_lookup_bool(it, "Cancelable", &i32)) {
		StrBuf->Printf(&buf, "'%s',", ((i32 == 0) ? "no" : "yes"));
	} else {
		StrBuf->Printf(&buf, "'no',");
	}

	// Target
	if (mob->lookup_const(it, "SkillTarget", &i32) && (i32 < MST_TARGET || i32 > MST_AROUND)) {
		i32 = MST_TARGET;
	}
	StrBuf->Printf(&buf, "'%s',", mob_skill_target_tostring(i32));

	// Condition
	if (mob->lookup_const(it, "CastCondition", &i32) && (i32 < MSC_ALWAYS || i32 > MSC_SPAWN)) {
		i32 = MSC_ALWAYS;
	}
	StrBuf->Printf(&buf, "'%s',", mob_skill_condition_tostring(i32));

	// ConditionValue
	i32 = 0;
	if (mob->lookup_const(it, "ConditionData", &i32)) {
		StrBuf->Printf(&buf, "'%d',", i32);
	} else {
		StrBuf->Printf(&buf, "NULL,");
	}

	// Val1-Val5
	for (i = 0; i < 5; i++) {
		sprintf(valname, "val%1d", i);
		if (libconfig->setting_lookup_int(it, valname, &i32)) {
			StrBuf->Printf(&buf, "%d,", i32);
		} else {
			StrBuf->Printf(&buf, "NULL,");
		}
	}

	// Emotion
	if (libconfig->setting_lookup_int(it, "Emotion", &i32)) {
		StrBuf->Printf(&buf, "'%d',", i32);
	} else {
		StrBuf->Printf(&buf, "NULL,");
	}

	if (libconfig->setting_lookup_int(it, "ChatMsgID", &i32) && i32 > 0 && i32 <= MAX_MOB_CHAT) {
		StrBuf->Printf(&buf, "'%d'", i32);
	} else {
		StrBuf->Printf(&buf, "NULL");
	}

	fprintf(tosql.fp, "REPLACE INTO `%s` VALUES (%s);\n", tosql.db_name, StrBuf->Value(&buf));

	StrBuf->Destroy(&buf);

	return true;

}


/**
 * Prints a SQL table header for the current mob_skill_db table.
 */
void mobskilldb2sql_tableheader(void)
{
	db2sql_fileheader();

	fprintf(tosql.fp,
			"--\n"
			"-- Table structure for table `%s`\n"
			"--\n"
			"\n"
			"DROP TABLE IF EXISTS `%s`;\n"
			"CREATE TABLE `%s` (\n"
			"  `MOB_ID` smallint NOT NULL,\n"
			"  `INFO` text NOT NULL,\n"
			"  `STATE` text NOT NULL,\n"
			"  `SKILL_ID` smallint NOT NULL,\n"
			"  `SKILL_LV` tinyint NOT NULL,\n"
			"  `RATE` smallint NOT NULL,\n"
			"  `CASTTIME` mediumint NOT NULL,\n"
			"  `DELAY` int NOT NULL,\n"
			"  `CANCELABLE` text NOT NULL,\n"
			"  `TARGET` text NOT NULL,\n"
			"  `CONDITION` text NOT NULL,\n"
			"  `CONDITION_VALUE` text,\n"
			"  `VAL1` int DEFAULT NULL,\n"
			"  `VAL2` int DEFAULT NULL,\n"
			"  `VAL3` int DEFAULT NULL,\n"
			"  `VAL4` int DEFAULT NULL,\n"
			"  `VAL5` int DEFAULT NULL,\n"
			"  `EMOTION` TEXT,\n"
			"  `CHAT` TEXT\n"
			") ENGINE=MyISAM;\n"
			"\n", tosql.db_name, tosql.db_name, tosql.db_name);
}

/**
 * Mob Skill DB Conversion
 */
void do_mobskilldb2sql(void)
{
	int i;
	struct convert_db_files {
		const char *name;
		const char *source;
		const char *destination;
	} files[] = {
		{"mob_skill_db", DBPATH"mob_skill_db.conf", "sql-files/mob_skill_db" DBSUFFIX ".sql"},
		{"mob_skill_db2", "mob_skill_db2.conf", "sql-files/mob_skill_db2.sql"},
	};

	/* link */
	mob_skill_db_libconfig_sub_skill = mob->skill_db_libconfig_sub_skill;
	mob->skill_db_libconfig_sub_skill = mobskilldb2sql_sub;

	memset(&tosql.buf, 0, sizeof(tosql.buf));
	for (i = 0; i < ARRAYLENGTH(files); i++) {
		if ((tosql.fp = fopen(files[i].destination, "wt+")) == NULL) {
			ShowError("do_mobskilldb2sql: File not found \"%s\".\n", files[i].destination);
			return;
		}

		tosql.db_name = files[i].name;
		mobskilldb2sql_tableheader();

		mob->skill_db_libconfig(files[i].source, false);

		fclose(tosql.fp);
	}

	/* unlink */
	mob->skill_db_libconfig_sub_skill = mob_skill_db_libconfig_sub_skill;

	for (i = 0; i < ARRAYLENGTH(tosql.buf); i++) {
		if (tosql.buf[i].p)
			aFree(tosql.buf[i].p);
	}
}

/**
 * Console command db2sql.
 */
CPCMD(db2sql)
{
	do_itemdb2sql();
	do_mobdb2sql();
}

/**
 * Console command itemdb2sql.
 */
CPCMD(itemdb2sql)
{
	do_itemdb2sql();
}

/**
 * Console command mobdb2sql.
 */
CPCMD(mobdb2sql)
{
	do_mobdb2sql();
}

/**
 * Command line argument handler for --db2sql
 */
CMDLINEARG(db2sql)
{
	map->minimal = true;
	itemdb2sql_torun = true;
	mobdb2sql_torun = true;
	return true;
}

/**
 * Command line argument handler for --itemdb2sql
 */
CMDLINEARG(itemdb2sql)
{
	map->minimal = true;
	itemdb2sql_torun = true;
	return true;
}

/**
 * Command line argument handler for --mobdb2sql
 */
CMDLINEARG(mobdb2sql)
{
	map->minimal = true;
	mobdb2sql_torun = true;
	return true;
}

HPExport void server_preinit(void)
{
	addArg("--db2sql", false, db2sql, NULL);
	addArg("--itemdb2sql", false, itemdb2sql, NULL);
	addArg("--mobdb2sql", false, mobdb2sql, NULL);
}

HPExport void plugin_init(void)
{
	addCPCommand("server:tools:db2sql", db2sql);
	addCPCommand("server:tools:itemdb2sql", itemdb2sql);
	addCPCommand("server:tools:mobdb2sql", mobdb2sql);
}

HPExport void server_online(void)
{
	sql_handle = SQL->Malloc();
	if (itemdb2sql_torun)
		do_itemdb2sql();
	if (mobdb2sql_torun)
		do_mobdb2sql();
}

HPExport void plugin_final (void)
{
	SQL->Free(sql_handle);
	sql_handle = NULL;
}
