/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
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

#include "log.h"

#include "map/battle.h"
#include "map/itemdb.h"
#include "map/map.h"
#include "map/mob.h"
#include "map/pc.h"
#include "common/cbasetypes.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/sql.h" // SQL_INNODB
#include "common/strlib.h"
#include "common/HPM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct log_interface log_s;
struct log_interface *logs;

/// obtain log type character for item/zeny logs
char log_picktype2char(e_log_pick_type type) {
	switch( type ) {
		case LOG_TYPE_TRADE:            return 'T';  // (T)rade
		case LOG_TYPE_VENDING:          return 'V';  // (V)ending
		case LOG_TYPE_PICKDROP_PLAYER:  return 'P';  // (P)player
		case LOG_TYPE_PICKDROP_MONSTER: return 'M';  // (M)onster
		case LOG_TYPE_NPC:              return 'S';  // NPC (S)hop
		case LOG_TYPE_SCRIPT:           return 'N';  // (N)PC Script
		case LOG_TYPE_STEAL:            return 'D';  // Steal/Snatcher
		case LOG_TYPE_CONSUME:          return 'C';  // (C)onsumed
		case LOG_TYPE_PRODUCE:          return 'O';  // Pr(O)duced/Ingredients
		case LOG_TYPE_MVP:              return 'U';  // MVP Rewards
		case LOG_TYPE_COMMAND:          return 'A';  // (A)dmin command
		case LOG_TYPE_STORAGE:          return 'R';  // Sto(R)age
		case LOG_TYPE_GSTORAGE:         return 'G';  // (G)uild storage
		case LOG_TYPE_MAIL:             return 'E';  // (E)mail attachment
		case LOG_TYPE_AUCTION:          return 'I';  // Auct(I)on
		case LOG_TYPE_BUYING_STORE:     return 'B';  // (B)uying Store
		case LOG_TYPE_LOOT:             return 'L';  // (L)oot (consumed monster pick/drop)
		case LOG_TYPE_BANK:             return 'K';  // Ban(K) Transactions
		case LOG_TYPE_DIVORCE:          return 'Y';  // Divorce
		case LOG_TYPE_ROULETTE:         return 'Z';  // Roulette
		case LOG_TYPE_RENTAL:           return 'W';  // Rental
		case LOG_TYPE_CARD:             return 'Q';  // Card
		case LOG_TYPE_INV_INVALID:      return 'J';  // Invalid in inventory
		case LOG_TYPE_CART_INVALID:     return 'H';  // Invalid in cart
		case LOG_TYPE_EGG:              return '@';  // Egg
		case LOG_TYPE_QUEST:            return '0';  // Quest
		case LOG_TYPE_SKILL:            return '1';  // Skill
		case LOG_TYPE_REFINE:           return '2';  // Refine
		case LOG_TYPE_OTHER:            return 'X';  // Other
	}

	// should not get here, fallback
	ShowDebug("log_picktype2char: Unknown pick type %u.\n", type);
	return 'X';
}

/// obtain log type character for chat logs
char log_chattype2char(e_log_chat_type type) {
	switch( type ) {
		case LOG_CHAT_GLOBAL:   return 'O';  // Gl(O)bal
		case LOG_CHAT_WHISPER:  return 'W';  // (W)hisper
		case LOG_CHAT_PARTY:    return 'P';  // (P)arty
		case LOG_CHAT_GUILD:    return 'G';  // (G)uild
		case LOG_CHAT_MAINCHAT: return 'M';  // (M)ain chat
	}

	// should not get here, fallback
	ShowDebug("log_chattype2char: Unknown chat type %u.\n", type);
	return 'O';
}

/// check if this item should be logged according the settings
bool should_log_item(int nameid, int amount, int refine, struct item_data *id) {
	int filter = logs->config.filter;

	if( id == NULL )
		return false;

	if( ( filter&LOG_FILTER_ALL ) ||
		( filter&LOG_FILTER_HEALING && id->type == IT_HEALING ) ||
		( filter&LOG_FILTER_ETC_AMMO && ( id->type == IT_ETC || id->type == IT_AMMO ) ) ||
		( filter&LOG_FILTER_USABLE && ( id->type == IT_USABLE || id->type == IT_CASH ) ) ||
		( filter&LOG_FILTER_WEAPON && id->type == IT_WEAPON ) ||
		( filter&LOG_FILTER_ARMOR && id->type == IT_ARMOR ) ||
		( filter&LOG_FILTER_CARD && id->type == IT_CARD ) ||
		( filter&LOG_FILTER_PETITEM && ( id->type == IT_PETEGG || id->type == IT_PETARMOR ) ) ||
		( filter&LOG_FILTER_PRICE && id->value_buy >= logs->config.price_items_log ) ||
		( filter&LOG_FILTER_AMOUNT && abs(amount) >= logs->config.amount_items_log ) ||
		( filter&LOG_FILTER_REFINE && refine >= logs->config.refine_items_log ) ||
		( filter&LOG_FILTER_CHANCE && ( ( id->maxchance != -1 && id->maxchance <= logs->config.rare_items_log ) || id->nameid == ITEMID_EMPERIUM ) )
	)
		return true;

	return false;
}
void log_branch_sub_sql(struct map_session_data* sd)
{
	struct SqlStmt *stmt;

	nullpo_retv(sd);
	stmt = SQL->StmtMalloc(logs->mysql_handle);
	if( SQL_SUCCESS != SQL->StmtPrepare(stmt, LOG_QUERY " INTO `%s` (`branch_date`, `account_id`, `char_id`, `char_name`, `map`) VALUES (NOW(), '%d', '%d', ?, '%s')", logs->config.log_branch, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
	   ||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
	   ||  SQL_SUCCESS != SQL->StmtExecute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}
	SQL->StmtFree(stmt);
}
void log_branch_sub_txt(struct map_session_data* sd) {
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(sd);
	if( ( logfp = fopen(logs->config.log_branch, "a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp,"%s - %s[%d:%d]\t%s\n", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex));
	fclose(logfp);
}

/// logs items, that summon monsters
void log_branch(struct map_session_data* sd) {
	nullpo_retv(sd);

	if( !logs->config.branch )
		return;

	logs->branch_sub(sd);
}
void log_pick_sub_sql(int id, int16 m, e_log_pick_type type, int amount, struct item* itm, struct item_data *data) {
	nullpo_retv(itm);
	if( SQL_ERROR == SQL->Query(logs->mysql_handle,
	    LOG_QUERY " INTO `%s` (`time`, `char_id`, `type`, `nameid`, `amount`, `refine`, `card0`, `card1`, `card2`, `card3`, `map`, `unique_id`) "
	    "VALUES (NOW(), '%d', '%c', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s', '%"PRIu64"')",
	    logs->config.log_pick, id, logs->picktype2char(type), itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3],
	    map->list[m].name, itm->unique_id)
	) {
		Sql_ShowDebug(logs->mysql_handle);
		return;
	}
}
void log_pick_sub_txt(int id, int16 m, e_log_pick_type type, int amount, struct item* itm, struct item_data *data) {
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(itm);
	if( ( logfp = fopen(logs->config.log_pick, "a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp,"%s - %d\t%c\t%d,%d,%d,%d,%d,%d,%d,%s,'%"PRIu64"'\n",
	        timestring, id, logs->picktype2char(type), itm->nameid, amount, itm->refine, itm->card[0], itm->card[1], itm->card[2], itm->card[3],
		map->list[m].name, itm->unique_id);
	fclose(logfp);
}
/// logs item transactions (generic)
void log_pick(int id, int16 m, e_log_pick_type type, int amount, struct item* itm, struct item_data *data) {
	nullpo_retv(itm);
	if( ( logs->config.enable_logs&type ) == 0 ) {// disabled
		return;
	}

	if( !logs->should_log_item(itm->nameid, amount, itm->refine, data) )
		return; //we skip logging this item set - it doesn't meet our logging conditions [Lupus]

	logs->pick_sub(id,m,type,amount,itm,data);
}

/// logs item transactions (players)
void log_pick_pc(struct map_session_data* sd, e_log_pick_type type, int amount, struct item* itm, struct item_data *data) {
	nullpo_retv(sd);
	nullpo_retv(itm);
	log_pick(sd->status.char_id, sd->bl.m, type, amount, itm, data ? data : itemdb->exists(itm->nameid));
}

/// logs item transactions (monsters)
void log_pick_mob(struct mob_data* md, e_log_pick_type type, int amount, struct item* itm, struct item_data *data) {
	nullpo_retv(md);
	nullpo_retv(itm);
	log_pick(md->class_, md->bl.m, type, amount, itm, data ? data : itemdb->exists(itm->nameid));
}
void log_zeny_sub_sql(struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount) {
	nullpo_retv(sd);
	nullpo_retv(src_sd);
	if( SQL_ERROR == SQL->Query(logs->mysql_handle, LOG_QUERY " INTO `%s` (`time`, `char_id`, `src_id`, `type`, `amount`, `map`) VALUES (NOW(), '%d', '%d', '%c', '%d', '%s')",
							   logs->config.log_zeny, sd->status.char_id, src_sd->status.char_id, logs->picktype2char(type), amount, mapindex_id2name(sd->mapindex)) )
	{
		Sql_ShowDebug(logs->mysql_handle);
		return;
	}
}
void log_zeny_sub_txt(struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount) {
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(sd);
	nullpo_retv(src_sd);
	if( ( logfp = fopen(logs->config.log_zeny, "a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp, "%s - %s[%d]\t%s[%d]\t%d\t\n", timestring, src_sd->status.name, src_sd->status.account_id, sd->status.name, sd->status.account_id, amount);
	fclose(logfp);
}
/// logs zeny transactions
void log_zeny(struct map_session_data* sd, e_log_pick_type type, struct map_session_data* src_sd, int amount)
{
	nullpo_retv(sd);

	if( !logs->config.zeny || ( logs->config.zeny != 1 && abs(amount) < logs->config.zeny ) )
		return;

	logs->zeny_sub(sd,type,src_sd,amount);
}
void log_mvpdrop_sub_sql(struct map_session_data* sd, int monster_id, int* log_mvp) {
	nullpo_retv(sd);
	nullpo_retv(log_mvp);
	if( SQL_ERROR == SQL->Query(logs->mysql_handle, LOG_QUERY " INTO `%s` (`mvp_date`, `kill_char_id`, `monster_id`, `prize`, `mvpexp`, `map`) VALUES (NOW(), '%d', '%d', '%d', '%d', '%s') ",
							   logs->config.log_mvpdrop, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1], mapindex_id2name(sd->mapindex)) )
	{
		Sql_ShowDebug(logs->mysql_handle);
		return;
	}
}
void log_mvpdrop_sub_txt(struct map_session_data* sd, int monster_id, int* log_mvp) {
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(sd);
	nullpo_retv(log_mvp);
	if( ( logfp = fopen(logs->config.log_mvpdrop,"a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp,"%s - %s[%d:%d]\t%d\t%d,%d\n", timestring, sd->status.name, sd->status.account_id, sd->status.char_id, monster_id, log_mvp[0], log_mvp[1]);
	fclose(logfp);
}
/// logs MVP monster rewards
void log_mvpdrop(struct map_session_data* sd, int monster_id, int* log_mvp)
{
	nullpo_retv(sd);

	if( !logs->config.mvpdrop )
		return;

	logs->mvpdrop_sub(sd,monster_id,log_mvp);
}

void log_atcommand_sub_sql(struct map_session_data* sd, const char* message)
{
	struct SqlStmt *stmt;

	nullpo_retv(sd);
	nullpo_retv(message);
	stmt = SQL->StmtMalloc(logs->mysql_handle);
	if( SQL_SUCCESS != SQL->StmtPrepare(stmt, LOG_QUERY " INTO `%s` (`atcommand_date`, `account_id`, `char_id`, `char_name`, `map`, `command`) VALUES (NOW(), '%d', '%d', ?, '%s', ?)", logs->config.log_gm, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
	   ||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
	   ||  SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, message, safestrnlen(message, 255))
	   ||  SQL_SUCCESS != SQL->StmtExecute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}
	SQL->StmtFree(stmt);
}
void log_atcommand_sub_txt(struct map_session_data* sd, const char* message) {
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(sd);
	nullpo_retv(message);
	if( ( logfp = fopen(logs->config.log_gm, "a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp, "%s - %s[%d]: %s\n", timestring, sd->status.name, sd->status.account_id, message);
	fclose(logfp);
}
/// logs used atcommands
void log_atcommand(struct map_session_data* sd, const char* message)
{
	nullpo_retv(sd);

	if( !logs->config.commands ||
	    !pc->should_log_commands(sd) )
		return;

	logs->atcommand_sub(sd,message);
}

void log_npc_sub_sql(struct map_session_data *sd, const char *message)
{
	struct SqlStmt *stmt;

	nullpo_retv(sd);
	nullpo_retv(message);
	stmt = SQL->StmtMalloc(logs->mysql_handle);
	if (SQL_SUCCESS != SQL->StmtPrepare(stmt, LOG_QUERY " INTO `%s` (`npc_date`, `account_id`, `char_id`, `char_name`, `map`, `mes`) VALUES (NOW(), '%d', '%d', ?, '%s', ?)", logs->config.log_npc, sd->status.account_id, sd->status.char_id, mapindex_id2name(sd->mapindex) )
	 || SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH))
	 || SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, message, safestrnlen(message, 255))
	 || SQL_SUCCESS != SQL->StmtExecute(stmt)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}
	SQL->StmtFree(stmt);
}
void log_npc_sub_txt(struct map_session_data *sd, const char *message) {
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(sd);
	nullpo_retv(message);
	if( ( logfp = fopen(logs->config.log_npc, "a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp, "%s - %s[%d]: %s\n", timestring, sd->status.name, sd->status.account_id, message);
	fclose(logfp);
}
/// logs messages passed to script command 'logmes'
void log_npc(struct map_session_data* sd, const char* message)
{
	nullpo_retv(sd);

	if( !logs->config.npc )
		return;

	logs->npc_sub(sd,message);
}

/**
 * Logs a chat message to the SQL backend.
 *
 * @param type         Chat type.
 * @param type_id      Additional ID, dependent on chat type (Guild ID, Party ID, etc). Zero when unused.
 * @param src_charid   Source character ID.
 * @param src_accid    Source account ID.
 * @param mapname      Source location map name
 * @param x            Source location x coordinate
 * @param y            Source location y coordinate
 * @param dst_charname Destination character name. Must not be NULL.
 * @param message      Message to log.
 */
void log_chat_sub_sql(e_log_chat_type type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char *dst_charname, const char *message)
{
	struct SqlStmt* stmt;

	nullpo_retv(dst_charname);
	nullpo_retv(message);
	stmt = SQL->StmtMalloc(logs->mysql_handle);
	if( SQL_SUCCESS != SQL->StmtPrepare(stmt, LOG_QUERY " INTO `%s` (`time`, `type`, `type_id`, `src_charid`, `src_accountid`, `src_map`, `src_map_x`, `src_map_y`, `dst_charname`, `message`) VALUES (NOW(), '%c', '%d', '%d', '%d', '%s', '%d', '%d', ?, ?)", logs->config.log_chat, logs->chattype2char(type), type_id, src_charid, src_accid, mapname, x, y)
	 || SQL_SUCCESS != SQL->StmtBindParam(stmt, 0, SQLDT_STRING, dst_charname, safestrnlen(dst_charname, NAME_LENGTH))
	 || SQL_SUCCESS != SQL->StmtBindParam(stmt, 1, SQLDT_STRING, message, safestrnlen(message, CHAT_SIZE_MAX))
	 || SQL_SUCCESS != SQL->StmtExecute(stmt)
	) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		return;
	}
	SQL->StmtFree(stmt);
}

/**
 * Logs a chat message to the TXT backend.
 *
 * @param type         Chat type.
 * @param type_id      Additional ID, dependent on chat type (Guild ID, Party ID, etc). Zero when unused.
 * @param src_charid   Source character ID.
 * @param src_accid    Source account ID.
 * @param mapname      Source location map name
 * @param x            Source location x coordinate
 * @param y            Source location y coordinate
 * @param dst_charname Destination character name. Must not be NULL.
 * @param message      Message to log.
 */
void log_chat_sub_txt(e_log_chat_type type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char *dst_charname, const char *message)
{
	char timestring[255];
	time_t curtime;
	FILE* logfp;

	nullpo_retv(mapname);
	nullpo_retv(dst_charname);
	nullpo_retv(message);
	if( ( logfp = fopen(logs->config.log_chat, "a") ) == NULL )
		return;
	time(&curtime);
	strftime(timestring, sizeof(timestring), "%m/%d/%Y %H:%M:%S", localtime(&curtime));
	fprintf(logfp, "%s - %c,%d,%d,%d,%s,%d,%d,%s,%s\n", timestring, logs->chattype2char(type), type_id, src_charid, src_accid, mapname, x, y, dst_charname, message);
	fclose(logfp);
}

/**
 * Logs a chat message.
 *
 * @param type         Chat type.
 * @param type_id      Additional ID, dependent on chat type (Guild ID, Party ID, etc). Zero when unused.
 * @param src_charid   Source character ID.
 * @param src_accid    Source account ID.
 * @param mapname      Source location map name
 * @param x            Source location x coordinate
 * @param y            Source location y coordinate
 * @param dst_charname Destination character name. May be NULL when unused.
 * @param message      Message to log.
 */
void log_chat(e_log_chat_type type, int type_id, int src_charid, int src_accid, const char *mapname, int x, int y, const char *dst_charname, const char *message)
{
	if ((logs->config.chat&type) == 0) {
		// disabled
		return;
	}

	if (logs->config.log_chat_woe_disable && (map->agit_flag || map->agit2_flag)) {
		// no chat logging during woe
		return;
	}

	if (dst_charname == NULL)
		dst_charname = "";

	logs->chat_sub(type,type_id,src_charid,src_accid,mapname,x,y,dst_charname,message);
}

void log_sql_init(void) {
	// log db connection
	logs->mysql_handle = SQL->Malloc();

	ShowInfo(""CL_WHITE"[SQL]"CL_RESET": Connecting to the Log Database "CL_WHITE"%s"CL_RESET" At "CL_WHITE"%s"CL_RESET"...\n",logs->db_name,logs->db_ip);
	if ( SQL_ERROR == SQL->Connect(logs->mysql_handle, logs->db_id, logs->db_pw, logs->db_ip, logs->db_port, logs->db_name) )
		exit(EXIT_FAILURE);
	ShowStatus(""CL_WHITE"[SQL]"CL_RESET": Successfully '"CL_GREEN"connected"CL_RESET"' to Database '"CL_WHITE"%s"CL_RESET"'.\n", logs->db_name);

	if (map->default_codepage[0] != '\0')
		if ( SQL_ERROR == SQL->SetEncoding(logs->mysql_handle, map->default_codepage) )
			Sql_ShowDebug(logs->mysql_handle);
}
void log_sql_final(void) {
	ShowStatus("Close Log DB Connection....\n");
	SQL->Free(logs->mysql_handle);
	logs->mysql_handle = NULL;
}

void log_set_defaults(void) {
	memset(&logs->config, 0, sizeof(logs->config));

	//LOG FILTER Default values
	logs->config.refine_items_log = 5;    // log refined items, with refine >= +5
	logs->config.rare_items_log   = 100;  // log rare items. drop chance <= 1%
	logs->config.price_items_log  = 1000; // 1000z
	logs->config.amount_items_log = 100;
}

int log_config_read(const char* cfgName) {
	static int count = 0;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	nullpo_retr(1, cfgName);
	if( count++ == 0 )
		log_set_defaults();

	if( ( fp = fopen(cfgName, "r") ) == NULL ) {
		ShowError("Log configuration file not found at: %s\n", cfgName);
		return 1;
	}

	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) == 2) {
			if( strcmpi(w1, "enable_logs") == 0 )
				logs->config.enable_logs = (e_log_pick_type)config_switch(w2);
			else if( strcmpi(w1, "sql_logs") == 0 )
				logs->config.sql_logs = (bool)config_switch(w2);
//start of common filter settings
			else if( strcmpi(w1, "rare_items_log") == 0 )
				logs->config.rare_items_log = atoi(w2);
			else if( strcmpi(w1, "refine_items_log") == 0 )
				logs->config.refine_items_log = atoi(w2);
			else if( strcmpi(w1, "price_items_log") == 0 )
				logs->config.price_items_log = atoi(w2);
			else if( strcmpi(w1, "amount_items_log") == 0 )
				logs->config.amount_items_log = atoi(w2);
//end of common filter settings
			else if( strcmpi(w1, "log_branch") == 0 )
				logs->config.branch = config_switch(w2);
			else if( strcmpi(w1, "log_filter") == 0 )
				logs->config.filter = config_switch(w2);
			else if( strcmpi(w1, "log_zeny") == 0 )
				logs->config.zeny = config_switch(w2);
			else if( strcmpi(w1, "log_commands") == 0 )
				logs->config.commands = config_switch(w2);
			else if( strcmpi(w1, "log_npc") == 0 )
				logs->config.npc = config_switch(w2);
			else if( strcmpi(w1, "log_chat") == 0 )
				logs->config.chat = config_switch(w2);
			else if( strcmpi(w1, "log_mvpdrop") == 0 )
				logs->config.mvpdrop = config_switch(w2);
			else if( strcmpi(w1, "log_chat_woe_disable") == 0 )
				logs->config.log_chat_woe_disable = (bool)config_switch(w2);
			else if( strcmpi(w1, "log_branch_db") == 0 )
				safestrncpy(logs->config.log_branch, w2, sizeof(logs->config.log_branch));
			else if( strcmpi(w1, "log_pick_db") == 0 )
				safestrncpy(logs->config.log_pick, w2, sizeof(logs->config.log_pick));
			else if( strcmpi(w1, "log_zeny_db") == 0 )
				safestrncpy(logs->config.log_zeny, w2, sizeof(logs->config.log_zeny));
			else if( strcmpi(w1, "log_mvpdrop_db") == 0 )
				safestrncpy(logs->config.log_mvpdrop, w2, sizeof(logs->config.log_mvpdrop));
			else if( strcmpi(w1, "log_gm_db") == 0 )
				safestrncpy(logs->config.log_gm, w2, sizeof(logs->config.log_gm));
			else if( strcmpi(w1, "log_npc_db") == 0 )
				safestrncpy(logs->config.log_npc, w2, sizeof(logs->config.log_npc));
			else if( strcmpi(w1, "log_chat_db") == 0 )
				safestrncpy(logs->config.log_chat, w2, sizeof(logs->config.log_chat));
			//support the import command, just like any other config
			else if( strcmpi(w1,"import") == 0 )
				logs->config_read(w2);
			else if (HPM->parseConf(w1, w2, HPCT_LOG))
				; // handled by plugins
			else
				ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}
	}

	fclose(fp);

	if( --count == 0 ) {// report final logging state
		const char* target = logs->config.sql_logs ? "table" : "file";

		if( logs->config.enable_logs && logs->config.filter ) {
			ShowInfo("Logging item transactions to %s '%s'.\n", target, logs->config.log_pick);
		}
		if( logs->config.branch ) {
			ShowInfo("Logging monster summon item usage to %s '%s'.\n", target, logs->config.log_pick);
		}
		if( logs->config.chat ) {
			ShowInfo("Logging chat to %s '%s'.\n", target, logs->config.log_chat);
		}
		if( logs->config.commands ) {
			ShowInfo("Logging commands to %s '%s'.\n", target, logs->config.log_gm);
		}
		if( logs->config.mvpdrop ) {
			ShowInfo("Logging MVP monster rewards to %s '%s'.\n", target, logs->config.log_mvpdrop);
		}
		if( logs->config.npc ) {
			ShowInfo("Logging 'logmes' messages to %s '%s'.\n", target, logs->config.log_npc);
		}
		if( logs->config.zeny ) {
			ShowInfo("Logging Zeny transactions to %s '%s'.\n", target, logs->config.log_zeny);
		}
		logs->config_done();
	}

	return 0;
}

void log_config_complete(void) {
	if( logs->config.sql_logs ) {
		logs->pick_sub = log_pick_sub_sql;
		logs->zeny_sub = log_zeny_sub_sql;
		logs->npc_sub = log_npc_sub_sql;
		logs->chat_sub = log_chat_sub_sql;
		logs->atcommand_sub = log_atcommand_sub_sql;
		logs->branch_sub = log_branch_sub_sql;
		logs->mvpdrop_sub = log_mvpdrop_sub_sql;
	}
}

void log_defaults(void) {
	logs = &log_s;

	sprintf(logs->db_ip,"127.0.0.1");
	sprintf(logs->db_id,"ragnarok");
	sprintf(logs->db_pw,"ragnarok");
	sprintf(logs->db_name,"log");

	logs->db_port = 3306;
	logs->mysql_handle = NULL;
	/* */

	logs->pick_pc = log_pick_pc;
	logs->pick_mob = log_pick_mob;
	logs->zeny = log_zeny;
	logs->npc = log_npc;
	logs->chat = log_chat;
	logs->atcommand = log_atcommand;
	logs->branch = log_branch;
	logs->mvpdrop = log_mvpdrop;

	/* will be modified in a few seconds once loading is complete. */
	logs->pick_sub = log_pick_sub_txt;
	logs->zeny_sub = log_zeny_sub_txt;
	logs->npc_sub = log_npc_sub_txt;
	logs->chat_sub = log_chat_sub_txt;
	logs->atcommand_sub = log_atcommand_sub_txt;
	logs->branch_sub = log_branch_sub_txt;
	logs->mvpdrop_sub = log_mvpdrop_sub_txt;

	logs->config_read = log_config_read;
	logs->config_done = log_config_complete;
	logs->sql_init = log_sql_init;
	logs->sql_final = log_sql_final;

	logs->picktype2char = log_picktype2char;
	logs->chattype2char = log_chattype2char;
	logs->should_log_item = should_log_item;
}
