/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
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

static struct inter_userconfig_interface inter_userconfig_s;
struct inter_userconfig_interface *inter_userconfig;

static int inter_userconfig_load_emotes(int account_id, struct userconfig_emotes *emotes)
{
	nullpo_ret(emotes);
	if (!inter_userconfig->emotes_from_sql(account_id, emotes))
		inter_userconfig->use_default_emotes(account_id, emotes);

	return 0;
}

static bool inter_userconfig_emotes_from_sql(int account_id, struct userconfig_emotes *emotes)
{
	nullpo_ret(emotes);

	StringBuf buf;
	StrBuf->Init(&buf);
	StrBuf->AppendStr(&buf, "SELECT `emote0`");
	for (int i = 1; i < MAX_EMOTES; i++)
		StrBuf->Printf(&buf, ", `emote%d`", i);
	StrBuf->Printf(&buf, " FROM `%s` WHERE `account_id` = '%d'", emotes_db, account_id);

	if (SQL_ERROR == SQL->QueryStr(inter->sql_handle, StrBuf->Value(&buf))) {
		Sql_ShowDebug(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	}

	if (SQL_SUCCESS != SQL->NextRow(inter->sql_handle)) {
		SQL->FreeResult(inter->sql_handle);
		StrBuf->Destroy(&buf);
		return false;
	}

	char *data = NULL;
	for (int index = 0; index < MAX_EMOTES; index ++) {
		SQL->GetData(inter->sql_handle, index, &data, NULL);
		strncpy(emotes->emote[index], data, EMOTE_SIZE);
                ShowError("load emote %d: %s\n", index, emotes->emote[index]);
	}

	SQL->FreeResult(inter->sql_handle);
	StrBuf->Destroy(&buf);
	return true;
}

static void inter_userconfig_use_default_emotes(int account_id, struct userconfig_emotes *emotes)
{
	nullpo_retv(emotes);

	// should be used strncpy [4144]
	strncpy(emotes->emote[0], "/!", EMOTE_SIZE);
	strncpy(emotes->emote[1], "/?", EMOTE_SIZE);
	strncpy(emotes->emote[2], "/기쁨", EMOTE_SIZE);
	strncpy(emotes->emote[3], "/하트", EMOTE_SIZE);
	strncpy(emotes->emote[4], "/땀", EMOTE_SIZE);
	strncpy(emotes->emote[5], "/아하", EMOTE_SIZE);
	strncpy(emotes->emote[6], "/짜증", EMOTE_SIZE);
	strncpy(emotes->emote[7], "/화", EMOTE_SIZE);
	strncpy(emotes->emote[8], "/돈", EMOTE_SIZE);
	strncpy(emotes->emote[9], "/...", EMOTE_SIZE);

	// english emotes
//	"/!","/?","/ho","/lv","/swt","/ic","/an","/ag","/$","/..."
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
		if (SQL_SUCCESS != SQL->StmtBindParam(stmt, i, SQLDT_STRING, emotes->emote[i], strnlen(emotes->emote[i], EMOTE_SIZE))) {
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

void inter_userconfig_defaults(void)
{
	inter_userconfig = &inter_userconfig_s;

	inter_userconfig->load_emotes = inter_userconfig_load_emotes;
	inter_userconfig->save_emotes = inter_userconfig_save_emotes;
	inter_userconfig->use_default_emotes = inter_userconfig_use_default_emotes;
	inter_userconfig->emotes_from_sql = inter_userconfig_emotes_from_sql;
	inter_userconfig->emotes_to_sql = inter_userconfig_emotes_to_sql;
}
