// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../config/core.h"

#include <stdio.h>
#include <stdlib.h>

#include "../common/HPMi.h"
#include "../common/cbasetypes.h"
#include "../common/conf.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../map/clif.h"
#include "../map/itemdb.h"
#include "../map/map.h"
#include "../map/pc.h"

#include "../common/HPMDataCheck.h"

HPExport struct hplugin_info pinfo = {
	"DB2SQL",        // Plugin name
	SERVER_TYPE_MAP, // Which server types this plugin works with?
	"0.5",           // Plugin version
	HPM_VERSION,     // HPM Version (don't change, macro is automatically updated)
};

struct {
	FILE *fp;
	struct {
		char *p;
		size_t len;
	} buf[4];
	char *db_name;
} tosql;
bool torun = false;

int (*itemdb_readdb_libconfig_sub) (config_setting_t *it, int n, const char *source);

void hstr(const char *str) {
	if( strlen(str) > tosql.buf[3].len ) {
		tosql.buf[3].len = tosql.buf[3].len + strlen(str) + 1000;
		RECREATE(tosql.buf[3].p,char,tosql.buf[3].len);
	}
	safestrncpy(tosql.buf[3].p,str,strlen(str));
	normalize_name(tosql.buf[3].p,"\t\n ");
}
int db2sql(config_setting_t *entry, int n, const char *source) {
	struct item_data *it = NULL;
	
	if( (it = itemdb->exists(itemdb_readdb_libconfig_sub(entry,n,source))) ) {
		char e_name[ITEM_NAME_LENGTH*2+1];
		const char *bonus = NULL;
		char *str;
		int i32;
		unsigned int ui32;
		config_setting_t *t = NULL;
		StringBuf buf;

		StrBuf->Init(&buf);

		// id
		StrBuf->Printf(&buf, "'%u',", it->nameid);

		// name_english
		SQL->EscapeString(NULL, e_name, it->name);
		StrBuf->Printf(&buf, "'%s',", e_name);
		
		// name_japanese
		SQL->EscapeString(NULL, e_name, it->jname);
		StrBuf->Printf(&buf, "'%s',", e_name);

		// type
		StrBuf->Printf(&buf, "'%u',", it->flag.delay_consume?IT_DELAYCONSUME:it->type);

		// price_buy
		StrBuf->Printf(&buf, "'%u',", it->value_buy);

		// price_sell
		StrBuf->Printf(&buf, "'%u',", it->value_sell);

		// weight
		StrBuf->Printf(&buf, "'%u',", it->weight);

		// atk
		StrBuf->Printf(&buf, "'%u',", it->atk);

		// matk
		StrBuf->Printf(&buf, "'%u',", it->matk);

		// defence
		StrBuf->Printf(&buf, "'%u',", it->def);

		// range
		StrBuf->Printf(&buf, "'%u',", it->range);

		// slots
		StrBuf->Printf(&buf, "'%u',", it->slot);

		// equip_jobs
		if( libconfig->setting_lookup_int(entry, "Job", &i32) ) // This is an unsigned value, do not check for >= 0
			ui32 = (unsigned int)i32;
		else
			ui32 = UINT_MAX;
		StrBuf->Printf(&buf, "'%u',", ui32);

		// equip_upper
		if( libconfig->setting_lookup_int(entry, "Upper", &i32) && i32 >= 0 )
			ui32 = (unsigned int)i32;
		else
			ui32 = ITEMUPPER_ALL;
		StrBuf->Printf(&buf, "'%u',", ui32);

		// equip_genders
		StrBuf->Printf(&buf, "'%u',", it->sex);

		// equip_locations
		StrBuf->Printf(&buf, "'%u',", it->equip);

		// weapon_level
		StrBuf->Printf(&buf, "'%u',", it->wlv);

		// equip_level_min
		StrBuf->Printf(&buf, "'%u',", it->elv);

		// equip_level_max
		if( (t = libconfig->setting_get_member(entry, "EquipLv")) && config_setting_is_aggregate(t) && libconfig->setting_length(t) >= 2 )
			StrBuf->Printf(&buf, "'%u',", it->elvmax);
		else
			StrBuf->AppendStr(&buf, "NULL,");

		// refineable
		StrBuf->Printf(&buf, "'%u',", it->flag.no_refine?0:1);

		// view
		StrBuf->Printf(&buf, "'%u',", it->look);

		// bindonequip
		StrBuf->Printf(&buf, "'%u',", it->flag.bindonequip?1:0);

		// buyingstore
		StrBuf->Printf(&buf, "'%u',", it->flag.buyingstore?1:0);

		// delay
		StrBuf->Printf(&buf, "'%u',", it->delay);

		// trade_flag
		StrBuf->Printf(&buf, "'%u',", it->flag.trade_restriction);

		// trade_group
		if (it->flag.trade_restriction != ITR_NONE && it->gm_lv_trade_override > 0 && it->gm_lv_trade_override < 100) {
			StrBuf->Printf(&buf, "'%u',", it->gm_lv_trade_override);
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
			StrBuf->Printf(&buf, "'%u',", it->view_id);
		} else {
			StrBuf->AppendStr(&buf, "NULL,");
		}

		// script
		if (it->script) {
			libconfig->setting_lookup_string(entry, "Script", &bonus);
			hstr(bonus);
			str = tosql.buf[3].p;
			if (strlen(str) > tosql.buf[0].len) {
				tosql.buf[0].len = tosql.buf[0].len + strlen(str) + 1000;
				RECREATE(tosql.buf[0].p,char,tosql.buf[0].len);
			}
			SQL->EscapeString(NULL, tosql.buf[0].p, str);
		}
		StrBuf->Printf(&buf, "'%s',", it->script?tosql.buf[0].p:"");
		
		// equip_script
		if (it->equip_script) {
			libconfig->setting_lookup_string(entry, "OnEquipScript", &bonus);
			hstr(bonus);
			str = tosql.buf[3].p;
			if (strlen(str) > tosql.buf[1].len) {
				tosql.buf[1].len = tosql.buf[1].len + strlen(str) + 1000;
				RECREATE(tosql.buf[1].p,char,tosql.buf[1].len);
			}
			SQL->EscapeString(NULL, tosql.buf[1].p, str);
		}
		StrBuf->Printf(&buf, "'%s',", it->equip_script?tosql.buf[1].p:"");

		// unequip_script
		if (it->unequip_script) {
			libconfig->setting_lookup_string(entry, "OnUnequipScript", &bonus);
			hstr(bonus);
			str = tosql.buf[3].p;
			if (strlen(str) > tosql.buf[2].len) {
				tosql.buf[2].len = tosql.buf[2].len + strlen(str) + 1000;
				RECREATE(tosql.buf[2].p,char,tosql.buf[2].len);
			}
			SQL->EscapeString(NULL, tosql.buf[2].p, str);
		}
		StrBuf->Printf(&buf, "'%s'", it->unequip_script?tosql.buf[2].p:"");

		fprintf(tosql.fp, "REPLACE INTO `%s` VALUES (%s);\n", tosql.db_name, StrBuf->Value(&buf));

		StrBuf->Destroy(&buf);
	}

	return it?it->nameid:0;
}
void totable(void) {
	fprintf(tosql.fp,
			"-- NOTE: This file was auto-generated and should never be manually edited,\n"
			"--       as it will get overwritten. If you need to modify this file,\n"
			"--       please consider modifying the corresponding .conf file inside\n"
			"--       the db folder, and then re-run the db2sql plugin.\n"
			"\n"
			"--\n"
			"-- Table structure for table `%s`\n"
			"--\n"
			"\n"
			"DROP TABLE IF EXISTS `%s`;\n"
			"CREATE TABLE `%s` (\n"
			"  `id` smallint(5) UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `name_english` varchar(50) NOT NULL DEFAULT '',\n"
			"  `name_japanese` varchar(50) NOT NULL DEFAULT '',\n"
			"  `type` tinyint(2) UNSIGNED NOT NULL DEFAULT '0',\n"
			"  `price_buy` mediumint(10) DEFAULT NULL,\n"
			"  `price_sell` mediumint(10) DEFAULT NULL,\n"
			"  `weight` smallint(5) UNSIGNED DEFAULT NULL,\n"
			"  `atk` smallint(5) UNSIGNED DEFAULT NULL,\n"
			"  `matk` smallint(5) UNSIGNED DEFAULT NULL,\n"
			"  `defence` smallint(5) UNSIGNED DEFAULT NULL,\n"
			"  `range` tinyint(2) UNSIGNED DEFAULT NULL,\n"
			"  `slots` tinyint(2) UNSIGNED DEFAULT NULL,\n"
			"  `equip_jobs` int(12) UNSIGNED DEFAULT NULL,\n"
			"  `equip_upper` tinyint(8) UNSIGNED DEFAULT NULL,\n"
			"  `equip_genders` tinyint(2) UNSIGNED DEFAULT NULL,\n"
			"  `equip_locations` smallint(4) UNSIGNED DEFAULT NULL,\n"
			"  `weapon_level` tinyint(2) UNSIGNED DEFAULT NULL,\n"
			"  `equip_level_min` smallint(5) UNSIGNED DEFAULT NULL,\n"
			"  `equip_level_max` smallint(5) UNSIGNED DEFAULT NULL,\n"
			"  `refineable` tinyint(1) UNSIGNED DEFAULT NULL,\n"
			"  `view` smallint(3) UNSIGNED DEFAULT NULL,\n"
			"  `bindonequip` tinyint(1) UNSIGNED DEFAULT NULL,\n"
			"  `buyingstore` tinyint(1) UNSIGNED DEFAULT NULL,\n"
			"  `delay` mediumint(9) UNSIGNED DEFAULT NULL,\n"
			"  `trade_flag` smallint(4) UNSIGNED DEFAULT NULL,\n"
			"  `trade_group` smallint(3) UNSIGNED DEFAULT NULL,\n"
			"  `nouse_flag` smallint(4) UNSIGNED DEFAULT NULL,\n"
			"  `nouse_group` smallint(4) UNSIGNED DEFAULT NULL,\n"
			"  `stack_amount` mediumint(6) UNSIGNED DEFAULT NULL,\n"
			"  `stack_flag` tinyint(2) UNSIGNED DEFAULT NULL,\n"
			"  `sprite` mediumint(6) UNSIGNED DEFAULT NULL,\n"
			"  `script` text,\n"
			"  `equip_script` text,\n"
			"  `unequip_script` text,\n"
			" PRIMARY KEY (`id`)\n"
			") ENGINE=MyISAM;\n"
			"\n",tosql.db_name,tosql.db_name,tosql.db_name);
}
void do_db2sql(void) {
	if( map->db_use_sql_item_db ) {
		ShowInfo("db2sql: this should not be used with 'db_use_sql_item_db' enabled, skipping...\n");
		return;
	}
	
	/* link */
	itemdb_readdb_libconfig_sub = itemdb->readdb_libconfig_sub;
	itemdb->readdb_libconfig_sub = db2sql;
	/* */
	
	if ((tosql.fp = fopen("sql-files/item_db_re.sql", "wt+")) == NULL) {
		ShowError("itemdb_tosql: File not found \"%s\".\n", "sql-files/item_db_re.sql");
		return;
 	}
	
	tosql.db_name = map->item_db_re_db;
	totable();
	
	memset(&tosql.buf, 0, sizeof(tosql.buf) );
	
	itemdb->clear(false);
	itemdb->readdb_libconfig("re/item_db.conf");
	
	fclose(tosql.fp);
	
	if ((tosql.fp = fopen("sql-files/item_db.sql", "wt+")) == NULL) {
		ShowError("itemdb_tosql: File not found \"%s\".\n", "sql-files/item_db.sql");
		return;
 	}
	
	tosql.db_name = map->item_db_db;
	totable();
	
	itemdb->clear(false);
	itemdb->readdb_libconfig("pre-re/item_db.conf");
	
	fclose(tosql.fp);
	
	if ((tosql.fp = fopen("sql-files/item_db2.sql", "wt+")) == NULL) {
		ShowError("itemdb_tosql: File not found \"%s\".\n", "sql-files/item_db2.sql");
		return;
 	}
	
	tosql.db_name = map->item_db2_db;
	totable();
	
	itemdb->clear(false);
	itemdb->readdb_libconfig("item_db2.conf");
	
	fclose(tosql.fp);
	
	/* unlink */
	itemdb->readdb_libconfig_sub = itemdb_readdb_libconfig_sub;
	
	if( tosql.buf[0].p ) aFree(tosql.buf[0].p);
	if( tosql.buf[1].p ) aFree(tosql.buf[1].p);
	if( tosql.buf[2].p ) aFree(tosql.buf[2].p);
	if( tosql.buf[3].p ) aFree(tosql.buf[3].p);
}
CPCMD(db2sql) {
	do_db2sql();
}
void db2sql_arg(char *param) {
	map->minimal = torun = true;
}
HPExport void server_preinit (void) {
	SQL = GET_SYMBOL("SQL");
	itemdb = GET_SYMBOL("itemdb");
	map = GET_SYMBOL("map");
	strlib = GET_SYMBOL("strlib");
	iMalloc = GET_SYMBOL("iMalloc");
	libconfig = GET_SYMBOL("libconfig");
	StrBuf = GET_SYMBOL("StrBuf");
	
	addArg("--db2sql",false,db2sql_arg,NULL);
}
HPExport void plugin_init (void) {
	addCPCommand("server:tools:db2sql",db2sql);
}
HPExport void server_online (void) {
	if( torun )
		do_db2sql();
}
