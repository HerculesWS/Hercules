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
#ifndef CHAR_INT_PARTY_H
#define CHAR_INT_PARTY_H

#include "common/hercules.h"
#include "common/mmo.h"

/* Forward Declarations */
struct DBMap; // common/db.h

//Party Flags on what to save/delete.
enum {
	PS_CREATE = 0x01, //Create a new party entry (index holds leader's info)
	PS_BASIC = 0x02, //Update basic party info.
	PS_LEADER = 0x04, //Update party's leader
	PS_ADDMEMBER = 0x08, //Specify new party member (index specifies which party member)
	PS_DELMEMBER = 0x10, //Specify member that left (index specifies which party member)
	PS_BREAK = 0x20, //Specify that this party must be deleted.
};

struct party_data {
	struct party party;
	unsigned int min_lv, max_lv;
	int family; //Is this party a family? if so, this holds the child id.
	unsigned char size; //Total size of party.
};

/**
 * inter_party interface
 **/
struct inter_party_interface {
	struct party_data *pt;
	struct DBMap *db;  // int party_id -> struct party_data*
	int (*check_lv) (struct party_data *p);
	void (*calc_state) (struct party_data *p);
	int (*tosql) (struct party *p, int flag, int index);
	struct party_data* (*fromsql) (int party_id);
	int (*sql_init) (void);
	void (*sql_final) (void);
	struct party_data* (*search_partyname) (const char *str);
	int (*check_exp_share) (struct party_data *p);
	int (*check_empty) (struct party_data *p);
	int (*parse_frommap) (int fd);
	int (*leave) (int party_id,int account_id, int char_id);
	int (*CharOnline) (int char_id, int party_id);
	int (*CharOffline) (int char_id, int party_id);
};

#ifdef HERCULES_CORE
void inter_party_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_party_interface *inter_party;

#endif /* CHAR_INT_PARTY_H */
