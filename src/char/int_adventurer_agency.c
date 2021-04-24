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
#include "char/mapif.h"
#include "common/cbasetypes.h"
#include "common/mapcharpackets.h"
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

static int inter_adventurer_agency_parse_frommap(int fd)
{
	RFIFOHEAD(fd);

	switch (RFIFOW(fd, 0)) {
		case 0x3084:
			inter_adventurer_agency->pJoinParty(fd);
			break;
		default:
			return 0;
	}

	return 1;
}

static void inter_adventurer_agency_parse_joinParty(int fd)
{
	const struct PACKET_MAPCHAR_AGENCY_JOIN_PARTY_REQ *p = RFIFOP(fd, 0);
	const int char_id = p->char_id;
	const int party_id = p->party_id;
	const int map_index = p->map_index;

	struct mmo_charstatus *cp = (struct mmo_charstatus*)idb_get(chr->char_db_, char_id);
	nullpo_retv(cp);
	if (cp->party_id != 0) {
		mapif->agency_joinPartyResult(fd, char_id, AGENCY_PLAYER_ALREADY_IN_PARTY);
		return;
	}

	struct party_data* party = (struct party_data*)idb_get(inter_party->db, party_id);
	if (party == NULL) {
		mapif->agency_joinPartyResult(fd, char_id, AGENCY_PARTY_NOT_FOUND);
		return;
	}

	struct party_member member = { 0 };
	member.account_id = cp->account_id;
	member.char_id    = cp->char_id;
	safestrncpy(member.name, cp->name, NAME_LENGTH);
	member.class      = cp->class;
	member.map        = map_index;
	member.lv         = cp->base_level;
	member.online     = 1;
	member.leader     = 0;

	if (!inter_party->add_member(party_id, &member)) {
		// for avoid another request to db, considerer only error
		// from inter_party->add_member is too much members already
		mapif->agency_joinPartyResult(fd, char_id, AGENCY_PARTY_NUMBER_EXCEEDED);
		return;
	}
	mapif->agency_joinPartyResult(fd, char_id, AGENCY_JOIN_ACCEPTED);
}

static bool inter_adventurer_agency_entry_check_existing(int char_id, int party_id)
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

static void inter_adventurer_agency_entry_delete_existing(int char_id, int party_id)
{
	if (SQL_ERROR == SQL->Query(inter->sql_handle,
	    "DELETE FROM `%s` WHERE `char_id`='%d' OR `party_id`='%d'",
	    adventurer_agency_db, char_id, party_id)) {
		Sql_ShowDebug(inter->sql_handle);
	}

	return;
}

static int inter_adventurer_agency_entry_delete(int char_id, int master_aid)
{
	struct mmo_charstatus *cp = (struct mmo_charstatus*)idb_get(chr->char_db_, char_id);
	nullpo_retr(1, cp);
	if (cp->party_id == 0)
		return 1;

	struct party_data* p = (struct party_data*)idb_get(inter_party->db, cp->party_id);
	if (p == NULL)
		return 1;

	if (inter_party->is_leader(p, char_id) != 1)
		return 1;

	inter_adventurer_agency->entry_delete_existing(char_id, cp->party_id);
	return 1;
}

bool inter_adventurer_agency_entry_tosql(int char_id, const char *char_name, int party_id, const struct party_add_data *entry)
{
	nullpo_retr(false, entry);

	char message_esc[NAME_LENGTH];
	SQL->EscapeStringLen(inter->sql_handle, message_esc, entry->message, strlen(entry->message));
	char char_name_esc[NAME_LENGTH];
	SQL->EscapeStringLen(inter->sql_handle, char_name_esc, char_name, strlen(char_name));

	const int flags = inter_adventurer_agency->entry_to_flags(char_id, entry);
	if (SQL_ERROR == SQL->Query(inter->sql_handle,
	    "INSERT INTO `%s` (`char_id`,`char_name`,`party_id`,`min_level`,`max_level`,`type`,`flags`,`message`) VALUES ('%d','%s','%d','%u','%u','%d','%d','%s')",
	    adventurer_agency_db, char_id, char_name_esc, party_id, entry->min_level, entry->max_level, entry->type, flags, message_esc)) {
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

	if (inter_adventurer_agency->entry_check_existing(char_id, cp->party_id)) {
		inter_adventurer_agency->entry_delete_existing(char_id, cp->party_id);
	}

	return inter_adventurer_agency->entry_tosql(char_id, cp->name, cp->party_id, entry);
}

void inter_adventurer_agency_get_page(int char_id, int page, struct adventuter_agency_page *packet)
{
	nullpo_retv(packet);

	packet->entry[0].char_id = 0;

	if (page > 0)
		page --;
	if (SQL_SUCCESS != SQL->Query(inter->sql_handle,
	    "SELECT `char_id`, `char_name`, `min_level`, `max_level`, `type`, `flags`, `message` FROM `%s` LIMIT %d, %d",
	    adventurer_agency_db, page * ADVENTURER_AGENCY_PAGE_SIZE, ADVENTURER_AGENCY_PAGE_SIZE)) {
		Sql_ShowDebug(inter->sql_handle);
		return;
	}

	char *data = NULL;
	int index = 0;
	while (index < ADVENTURER_AGENCY_PAGE_SIZE) {
		if (SQL_SUCCESS != SQL->NextRow(inter->sql_handle)) {
			break;
		}
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		packet->entry[index].char_id = atoi(data);

		// do not access any methods for avoid new db connections
		struct mmo_charstatus *cp = (struct mmo_charstatus*)idb_get(chr->char_db_, packet->entry[index].char_id);
		if (cp == NULL) {
			// add offline char
			packet->entry[index].account_id = 0;
		} else {
			packet->entry[index].account_id = cp->account_id;
		}

		SQL->GetData(inter->sql_handle, 1, &data, NULL);
		safestrncpy(packet->entry[index].char_name, data, NAME_LENGTH);
		SQL->GetData(inter->sql_handle, 2, &data, NULL);
		packet->entry[index].min_level = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL);
		packet->entry[index].max_level = atoi(data);
		SQL->GetData(inter->sql_handle, 4, &data, NULL);
		packet->entry[index].type = atoi(data);
		SQL->GetData(inter->sql_handle, 5, &data, NULL);
		packet->entry[index].flags = atoi(data);
		SQL->GetData(inter->sql_handle, 6, &data, NULL);
		safestrncpy(packet->entry[index].message, data, NAME_LENGTH);

		index ++;
	}
	packet->entry[index].char_id = 0;
	SQL->FreeResult(inter->sql_handle);

	return;
}

int inter_adventurer_agency_get_pages_count(void)
{
	if (SQL_SUCCESS != SQL->Query(inter->sql_handle,
	    "SELECT count(`char_id`) FROM `%s`", adventurer_agency_db)) {
		Sql_ShowDebug(inter->sql_handle);
		return 0;
	}

	char *data = NULL;
	int count = 0;
	if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		count = atoi(data);
	}

	SQL->FreeResult(inter->sql_handle);

	return count / ADVENTURER_AGENCY_PAGE_SIZE;
}

int inter_adventurer_agency_get_player_request(int char_id, struct adventuter_agency_entry *entry)
{
	nullpo_retr(1, entry);

	entry->char_id = 0;

	if (SQL_SUCCESS != SQL->Query(inter->sql_handle,
	    "SELECT `char_id`, `min_level`, `max_level`, `type`, `flags`, `message` FROM `%s` WHERE char_id='%d'",
	    adventurer_agency_db, char_id)) {
		Sql_ShowDebug(inter->sql_handle);
		return 1;
	}

	char *data = NULL;
	if (SQL_SUCCESS == SQL->NextRow(inter->sql_handle)) {
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		entry->char_id = atoi(data);
		// do not access any methods for avoid new db connections
		struct mmo_charstatus *cp = (struct mmo_charstatus*)idb_get(chr->char_db_, entry->char_id);
		if (cp == NULL) {
			safestrncpy(entry->char_name, "offline", NAME_LENGTH);
			entry->account_id = 0;
		} else {
			safestrncpy(entry->char_name, cp->name, NAME_LENGTH);
			entry->account_id = cp->account_id;
		}

		SQL->GetData(inter->sql_handle, 1, &data, NULL);
		entry->min_level = atoi(data);
		SQL->GetData(inter->sql_handle, 2, &data, NULL);
		entry->max_level = atoi(data);
		SQL->GetData(inter->sql_handle, 3, &data, NULL);
		entry->type = atoi(data);
		SQL->GetData(inter->sql_handle, 4, &data, NULL);
		entry->flags = atoi(data);
		SQL->GetData(inter->sql_handle, 5, &data, NULL);
		safestrncpy(entry->message, data, NAME_LENGTH);
	}

	SQL->FreeResult(inter->sql_handle);
	return 1;
}

void inter_adventurer_agency_defaults(void)
{
	inter_adventurer_agency = &inter_adventurer_agency_s;

	inter_adventurer_agency->pJoinParty = inter_adventurer_agency_parse_joinParty;
	inter_adventurer_agency->parse_frommap = inter_adventurer_agency_parse_frommap;
	inter_adventurer_agency->entry_add = inter_adventurer_agency_entry_add;
	inter_adventurer_agency->entry_check_existing = inter_adventurer_agency_entry_check_existing;
	inter_adventurer_agency->entry_delete_existing = inter_adventurer_agency_entry_delete_existing;
	inter_adventurer_agency->entry_to_flags = inter_adventurer_agency_entry_to_flags;
	inter_adventurer_agency->entry_tosql = inter_adventurer_agency_entry_tosql;
	inter_adventurer_agency->entry_delete = inter_adventurer_agency_entry_delete;
	inter_adventurer_agency->get_page = inter_adventurer_agency_get_page;
	inter_adventurer_agency->get_pages_count = inter_adventurer_agency_get_pages_count;
	inter_adventurer_agency->get_player_request = inter_adventurer_agency_get_player_request;
}
