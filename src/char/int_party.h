// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_INT_PARTY_H_
#define _CHAR_INT_PARTY_H_

//Party Flags on what to save/delete.
enum {
	PS_CREATE = 0x01, //Create a new party entry (index holds leader's info) 
	PS_BASIC = 0x02, //Update basic party info.
	PS_LEADER = 0x04, //Update party's leader
	PS_ADDMEMBER = 0x08, //Specify new party member (index specifies which party member)
	PS_DELMEMBER = 0x10, //Specify member that left (index specifies which party member)
	PS_BREAK = 0x20, //Specify that this party must be deleted.
};

struct party;

int inter_party_parse_frommap(int fd);
int inter_party_sql_init(void);
void inter_party_sql_final(void);
int inter_party_leave(int party_id,int account_id, int char_id);
int inter_party_CharOnline(int char_id, int party_id);
int inter_party_CharOffline(int char_id, int party_id);

#endif /* _CHAR_INT_PARTY_H_ */
