// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../common/cbasetypes.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/HPMi.h"
#include "../common/mmo.h"
#include "../config/core.h"
#include "../map/clif.h"
#include "../map/pc.h"
#include "../map/map.h"
#include "../map/itemdb.h"
#include <stdio.h>
#include <stdlib.h>

HPExport struct hplugin_info pinfo = {
	"DB2SQL",		// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.4",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};

SqlStmt* stmt;

int (*parse_dbrow)(char** str, const char* source, int line, int scriptopt);

char* trimbraces(char* str) {
	size_t start;
	size_t end;
	
	if( str == NULL )
		return str;
	
	for( start = 0; str[start] && str[start] == '{'; ++start )
		;
	for( end = strlen(str); start < end && str[end-1] && (str[end-1] == '}' || str[end-1] == '\n'); --end )
		;
	if( start == end )
		*str = '\0';
	else {
		str[end] = '\0';
		memmove(str,str+start,end-start+1);
		trim(str);
	}
	return str;
}
int db2sql(char** str, const char* source, int line, int scriptopt) {
	struct item_data *it = NULL;
	unsigned char offset = 0;
#ifdef RENEWAL
	if( iMap->db_use_sql_item_db ) offset = 1;
#endif
	if( (it = itemdb->exists(parse_dbrow(str,source,line,scriptopt))) ) {
	/* renewal has the 'matk' and 'equip_level' is now 'equip_level_min', and there is a new 'equip_level_max' field */
#ifdef RENEWAL
		if( SQL_SUCCESS != SQL->StmtPrepare(stmt, "REPLACE INTO `%s` (`id`,`name_english`,`name_japanese`,`type`,`price_buy`,`price_sell`,`weight`,`atk`,`matk`,`defence`,`range`,`slots`,`equip_jobs`,`equip_upper`,`equip_genders`,`equip_locations`,`weapon_level`,`equip_level_min`,`equip_level_max`,`refineable`,`view`,`script`,`equip_script`,`unequip_script`) VALUES ('%u',?,?,'%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u',?,?,?)",iMap->item_db_re_db,
											it->nameid,it->flag.delay_consume?IT_DELAYCONSUME:it->type,it->value_buy,it->value_sell,it->weight,it->atk,it->matk,it->def,it->range,it->slot,(unsigned int)strtoul(str[11+offset],NULL,0),atoi(str[12+offset]),atoi(str[13+offset]),atoi(str[14+offset]),it->wlv,it->elv,it->elvmax,atoi(str[17+offset]),it->look) )
#else
		if( SQL_SUCCESS != SQL->StmtPrepare(stmt, "REPLACE INTO `%s` (`id`,`name_english`,`name_japanese`,`type`,`price_buy`,`price_sell`,`weight`,`atk`,`defence`,`range`,`slots`,`equip_jobs`,`equip_upper`,`equip_genders`,`equip_locations`,`weapon_level`,`equip_level`,`refineable`,`view`,`script`,`equip_script`,`unequip_script`) VALUES ('%u',?,?,'%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u','%u',?,?,?)",iMap->item_db_db,
											it->nameid,it->flag.delay_consume?IT_DELAYCONSUME:it->type,it->value_buy,it->value_sell,it->weight,it->atk,it->def,it->range,it->slot,(unsigned int)strtoul(str[11],NULL,0),atoi(str[12]),atoi(str[13]),atoi(str[14]),it->wlv,it->elv,atoi(str[17]),it->look) )
#endif
			SqlStmt_ShowDebug(stmt);
		else {
			if ( SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, it->name, strlen(it->name)) )
				SqlStmt_ShowDebug(stmt);
			else {
				if ( SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, it->jname, strlen(it->jname)) )
					SqlStmt_ShowDebug(stmt);
				else {
				#ifdef RENEWAL
					if( iMap->db_use_sql_item_db ) offset += 1;
				#endif
					if( it->script ) trimbraces(str[19+offset]);
					if ( SQL_SUCCESS != SQL->StmtBindParam(stmt, 2, SQLDT_STRING, it->script?str[19+offset]:"", it->script?strlen(str[19+offset]):0) )
						SqlStmt_ShowDebug(stmt);
					else {
						if( it->equip_script ) trimbraces(str[20+offset]);
						if ( SQL_SUCCESS != SQL->StmtBindParam(stmt, 3, SQLDT_STRING, it->equip_script?str[20+offset]:"", it->equip_script?strlen(str[20+offset]):0) )
							SqlStmt_ShowDebug(stmt);
						else {
							if( it->unequip_script ) trimbraces(str[21+offset]);
							if ( SQL_SUCCESS != SQL->StmtBindParam(stmt, 4, SQLDT_STRING, it->unequip_script?str[21+offset]:"", it->unequip_script?strlen(str[21+offset]):0) )
								SqlStmt_ShowDebug(stmt);
							else {
								if( SQL_SUCCESS != SQL->StmtExecute(stmt) )
									SqlStmt_ShowDebug(stmt);
							}
						}
					}
				}
			}
		}
		return it->nameid;
	}
	return 0;
}

CPCMD(db2sql) {
	
	if( iMap->db_use_sql_item_db ) {
		ShowInfo("db2sql: this should not be used with 'db_use_sql_item_db' enabled, skipping...\n");
		return;
	}
	
	stmt = SQL->StmtMalloc(mysql_handle);
	if( stmt == NULL ) {
		SqlStmt_ShowDebug(stmt);
		return;
	}

	/* link */
	parse_dbrow = itemdb->parse_dbrow;
	itemdb->parse_dbrow = db2sql;
	/* empty table */
#ifdef RENEWAL
	if ( SQL_ERROR == SQL->Query(mysql_handle, "DELETE FROM `%s`", iMap->item_db_re_db ) )
#else
	if ( SQL_ERROR == SQL->Query(mysql_handle, "DELETE FROM `%s`", iMap->item_db_db) )
#endif
		Sql_ShowDebug(mysql_handle);
	else {
		itemdb->reload();
	}
	/* unlink */
	itemdb->parse_dbrow = parse_dbrow;
	
	SQL->StmtFree(stmt);
}

HPExport void plugin_init (void) {
	SQL = GET_SYMBOL("SQL");
	itemdb = GET_SYMBOL("itemdb");
	iMap = GET_SYMBOL("iMap");
	strlib = GET_SYMBOL("strlib");
	
	HPMi->addCPCommand("server:tools:db2sql",CPCMD_A(db2sql));
}