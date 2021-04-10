/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020-2021 Andrei Karas (4144)
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

#include "char/int_adventurer_agency.h"

#include "char/char.h"
#include "char/int_party.h"
#include "char/inter.h"
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

static struct inter_adventurer_agency_interface inter_adventurer_agency_s;
struct inter_adventurer_agency_interface *inter_adventurer_agency;

static bool inter_adventurer_agency_check_existing(int char_id, int party_id)
{
	if (SQL_ERROR == SQL->Query(inter->sql_handle,
	    "SELECT `char_id` FROM `%s` WHERE `char_id`='%d' OR `party_id`='%d'",
	    adventurer_agency_db, char_id, party_id)) {
		Sql_ShowDebug(inter->sql_handle);
	} else if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
		SQL->FreeResult(inter->sql_handle);
		return true;
	}
	SQL->FreeResult(inter->sql_handle);

	return false;
}

bool inter_adventurer_agency_entry_tosql(int char_id, int party_id, const struct party_add_data *entry)
{
	nullpo_retr(false, entry);

	char message_esc[NAME_LENGTH];
	SQL->EscapeStringLen(inter->sql_handle, message_esc, entry->message, strlen(entry->message));

	const int flags = inter_adventurer_agency->entry_to_flags(char_id, entry);
	if (SQL_ERROR == SQL->Query(inter->sql_handle,
	    "INSERT INTO `%s` (`char_id`,`party_id`,`min_level`,`max_level`,`type`,`flags`,`message`) VALUES ('%d','%d','%u','%u','%d','%d','%s')",
	    adventurer_agency_db, char_id, party_id, entry->min_level, entry->max_level, entry->type, flags, message_esc)) {
		Sql_ShowDebug(inter->sql_handle);
		return false;
	}
	return true;
}

int inter_adventurer_agency_entry_to_flags(int char_id, const struct party_add_data *entry)
{
	nullpo_retr(0, entry);
	int flags = 0;
	if (entry->healer)
		flags |= AGENCY_HEALER;
	if (entry->assist)
		flags |= AGENCY_ASSIST;
	if (entry->tanker)
		flags |= AGENCY_TANKER;
	if (entry->dealer)
		flags |= AGENCY_DEALER;
	return flags;
}

bool inter_adventurer_agency_entry_add(int char_id, const struct party_add_data *entry)
{
	nullpo_retr(false, entry);

	struct mmo_charstatus *cp = (struct mmo_charstatus*)idb_get(chr->char_db_, char_id);
	nullpo_retr(false, cp);
	if (cp->party_id == 0)
		return false;

	struct party_data* p = (struct party_data*)idb_get(inter_party->db, cp->party_id);
	if (p == NULL)
		return false;

	if (inter_party->is_leader(p, char_id) != 1)
		return false;

	if (inter_adventurer_agency->check_existing(char_id, cp->party_id))
		return false;

	return inter_adventurer_agency->entry_tosql(char_id, cp->party_id, entry);
}

void inter_adventurer_agency_defaults(void)
{
	inter_adventurer_agency = &inter_adventurer_agency_s;

	inter_adventurer_agency->entry_add = inter_adventurer_agency_entry_add;
	inter_adventurer_agency->check_existing = inter_adventurer_agency_check_existing;
	inter_adventurer_agency->entry_to_flags = inter_adventurer_agency_entry_to_flags;
	inter_adventurer_agency->entry_tosql = inter_adventurer_agency_entry_tosql;
}
