// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_PARTY_H
#define CHAR_INT_PARTY_H

#include "../common/mmo.h"

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

#ifdef HERCULES_CORE
void inter_party_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_party interface
 **/
struct inter_party_interface {
	struct party_data *pt;
	DBMap* db;  // int party_id -> struct party_data*
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

struct inter_party_interface *inter_party;

#endif /* CHAR_INT_PARTY_H */
