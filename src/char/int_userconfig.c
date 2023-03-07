/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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

#include "int_userconfig.h"

#include "char/char.h"
#include "char/int_mail.h"
#include "char/inter.h"
#include "char/mapif.h"
#include "common/cbasetypes.h"
#include "common/apipackets.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>

//#define DEBUG_EMOTES

static struct inter_userconfig_interface inter_userconfig_s;
struct inter_userconfig_interface *inter_userconfig;
static struct inter_userconfig_dbs inter_userconfigdbs;

static bool inter_userconfig_load_emotes(int account_id, struct userconfig_emotes *emotes)
{
	nullpo_retr(false, emotes);

	enum userconfig_from_sql_result sql_result = inter_userconfig->emotes_from_sql(account_id, emotes);
	if (sql_result == USERCONFIG_FROM_SQL_ERROR)
		return false;

	if (sql_result == USERCONFIG_FROM_SQL_NOT_EXISTS)
		inter_userconfig->use_default_emotes(account_id, emotes);

	return true;
}

static enum userconfig_from_sql_result inter_userconfig_emotes_from_sql(int account_id, struct userconfig_emotes *emotes)
{
	nullpo_ret(emotes);

	StringBuf buf;
	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `emote0`");
	for (int i = 1; i < MAX_EMOTES; i++)
		StrBuf->Printf(&buf, ", `emote%d`", i);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `account_id` = '%d'", emotes_db, account_id);

	int result = SQL->QueryStrFetch(inter->sql_handle, StrBuf->Value(&buf));
	if (result != SQL_SUCCESS) {
		if (result == SQL_ERROR)
			Sql_ShowDebug(inter->sql_handle);
		
		StrBuf->Destroy(&buf);
		return (result == SQL_NO_DATA ? USERCONFIG_FROM_SQL_NOT_EXISTS : USERCONFIG_FROM_SQL_ERROR);
	}

	char *data = NULL;
	for (int index = 0; index < MAX_EMOTES; index ++) {
		SQL->GetData(inter->sql_handle, index, &data, NULL);
		safestrncpy(emotes->emote[index], data, EMOTE_SIZE);
	}

	SQL->FreeResult(inter->sql_handle);
	StrBuf->Destroy(&buf);
	return USERCONFIG_FROM_SQL_SUCCESS;
}

static void inter_userconfig_use_default_emotes(int account_id, struct userconfig_emotes *emotes)
{
	nullpo_retv(emotes);

	for (int i = 0; i < MAX_EMOTES; i ++) {
		safestrncpy(emotes->emote[i], inter_userconfig->dbs->default_emotes[i], EMOTE_SIZE);
	}
}

static int inter_userconfig_save_emotes(int account_id, const struct userconfig_emotes *emotes)
{
	inter_userconfig->emotes_to_sql(account_id, emotes);
	return 0;
}

static bool inter_userconfig_emotes_to_sql(int account_id, const struct userconfig_emotes *emotes)
{
	nullpo_retr(false, emotes);

	struct SqlStmt *stmt = SQL->StmtMalloc(inter->sql_handle);
	StringBuf buf;
	StrBuf->Init(&buf);
	StrBuf->Printf(&buf, "REPLACE INTO `%s` (`account_id`", emotes_db);
	for (int i = 0; i < MAX_EMOTES; i++)
		StrBuf->Printf(&buf, ", `emote%d`", i);
	StrBuf->Printf(&buf, ") VALUES(%d", account_id);
	for (int i = 0; i < MAX_EMOTES; i++)
		StrBuf->AppendStr(&buf, ", ?");
	StrBuf->AppendStr(&buf, ")");

	if (SQL_ERROR == SQL->StmtPrepareStr(stmt, StrBuf->Value(&buf))) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		StrBuf->Destroy(&buf);
		return false;
	}
	for (int i = 0; i < MAX_EMOTES; i++) {
		if (SQL_SUCCESS != SQL->StmtBindParam(stmt, i, SQLDT_STRING, emotes->emote[i], strnlen(emotes->emote[i], EMOTE_SIZE - 1))) {
			SqlStmt_ShowDebug(stmt);
			SQL->StmtFree(stmt);
			StrBuf->Destroy(&buf);
			return false;
		}
	}

	if (SQL_SUCCESS != SQL->StmtExecute(stmt)) {
		SqlStmt_ShowDebug(stmt);
		SQL->StmtFree(stmt);
		StrBuf->Destroy(&buf);
		return false;
	}
	SQL->StmtFree(stmt);
	StrBuf->Destroy(&buf);
	return true;
}

void inter_userconfig_hotkey_tab_tosql(int account_id, const struct userconfig_userhotkeys_v2 *hotkeys)
{
	nullpo_retv(hotkeys);

	inter_userconfig->hotkey_tab_clear(account_id, hotkeys->tab);

	char desc_esc[HOTKEY_DESCRIPTION_SIZE * 2 + 1];
#ifdef DEBUG_EMOTES
	ShowError("tab: %d\n", hotkeys->tab);
#endif
	for (int i = 0; i < hotkeys->count; i ++) {
#ifdef DEBUG_EMOTES
		ShowError("desc: %s\n", hotkeys->keys[i].desc);
		ShowError("index: %d\n", hotkeys->keys[i].index);
		ShowError("key1: %d\n", hotkeys->keys[i].key1);
		ShowError("key2: %d\n", hotkeys->keys[i].key2);
#endif
		SQL->EscapeString(inter->sql_handle, desc_esc, hotkeys->keys[i].desc);
		if (SQL_ERROR == SQL->Query(inter->sql_handle,
		    "INSERT INTO `%s` (`account_id`,`tab`,`desc`,`index`,`key1`,`key2`) VALUES ('%d','%d','%s','%d','%d','%d')",
		    hotkeys_db, account_id, hotkeys->tab, desc_esc, hotkeys->keys[i].index, hotkeys->keys[i].key1, hotkeys->keys[i].key2)) {
			Sql_ShowDebug(inter->sql_handle);
			return;
		}
	}
}

void inter_userconfig_hotkey_tab_clear(int account_id, int tab_id)
{
	if (SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' and `tab` = '%d'",
	    hotkeys_db, account_id, tab_id)) {
		Sql_ShowDebug(inter->sql_handle);
	}
}

bool inter_userconfig_hotkey_tab_fromsql(int account_id, struct userconfig_userhotkeys_v2 *hotkeys, int tab_id)
{
	hotkeys->tab = tab_id;

	if (SQL_SUCCESS != SQL->Query(inter->sql_handle,
	    "SELECT `desc`, `index`, `key1`, `key2` FROM `%s` WHERE `account_id` = '%d' AND `tab` = '%d'",
	    hotkeys_db, account_id, tab_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}

	char *data = NULL;
	int index = 0;
	for (index = 0; index < MAX_USERHOTKEYS && SQL_SUCCESS == SQL->NextRow(inter->sql_handle); index ++) {
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		safestrncpy(hotkeys->keys[index].desc, data, HOTKEY_DESCRIPTION_SIZE);
		SQL->GetData(inter->sql_handle, 1, &data, NULL);
		hotkeys->keys[index].index = atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, NULL);
		hotkeys->keys[index].key1 = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL);
		hotkeys->keys[index].key2 = atoi(data);
	}

	SQL->FreeResult(inter->sql_handle);

	hotkeys->count = index;
	return true;
}

static bool inter_userconfig_config_read(const char *filename, const struct config_t *config, bool imported)
{
	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	const struct config_setting_t *setting = libconfig->lookup(config, "char_configuration/emotes");
	if (setting == NULL) {
		if (imported)
			return true;
		ShowError("inter_userconfig_config_read: char_configuration/emotes was not found in %s!\n", filename);
		return false;
	}

	const struct config_setting_t *t = libconfig->setting_get_member(setting, "default_emotes");
	const int len = libconfig->setting_length(t);
	if (len != MAX_EMOTES) {
		ShowError("inter_userconfig_config_read: wrong number of default emotes found: %d vs %d\n", len, MAX_EMOTES);
		return false;
	}

	if (t == NULL) {
		if (imported)
			return true;
		ShowError("inter_userconfig_config_read: default_emotes not found\n");
		return false;
	}
	for (int i = 0; i < len; ++i) {
		const char *emote_str = libconfig->setting_get_string_elem(t, i);
		if (emote_str == NULL || strlen(emote_str) >= EMOTE_SIZE) {
			ShowError("inter_userconfig_config_read: default_emotes[%d] size too big: '%s'\n", i, emote_str);
			return false;
		}
		safestrncpy(inter_userconfig->dbs->default_emotes[i], emote_str, EMOTE_SIZE);
	}

	return true;
}

static void inter_userconfig_init(void)
{
	safestrncpy(inter_userconfig->dbs->default_emotes[0], "/!", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[1], "/?", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[2], "/기쁨", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[3], "/하트", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[4], "/땀", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[5], "/아하", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[6], "/짜증", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[7], "/화", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[8], "/돈", EMOTE_SIZE);
	safestrncpy(inter_userconfig->dbs->default_emotes[9], "/...", EMOTE_SIZE);
}

void inter_userconfig_defaults(void)
{
	inter_userconfig = &inter_userconfig_s;
	inter_userconfig->dbs = &inter_userconfigdbs;

	inter_userconfig->init = inter_userconfig_init;
	inter_userconfig->config_read = inter_userconfig_config_read;
	inter_userconfig->load_emotes = inter_userconfig_load_emotes;
	inter_userconfig->save_emotes = inter_userconfig_save_emotes;
	inter_userconfig->use_default_emotes = inter_userconfig_use_default_emotes;
	inter_userconfig->emotes_from_sql = inter_userconfig_emotes_from_sql;
	inter_userconfig->emotes_to_sql = inter_userconfig_emotes_to_sql;
	inter_userconfig->hotkey_tab_tosql = inter_userconfig_hotkey_tab_tosql;
	inter_userconfig->hotkey_tab_clear = inter_userconfig_hotkey_tab_clear;
	inter_userconfig->hotkey_tab_fromsql = inter_userconfig_hotkey_tab_fromsql;
}
