// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/socket.h"
#include "../common/showmsg.h"
#include "../common/mapindex.h"
#include "../common/sql.h"
#include "char.h"
#include "inter.h"
#include "int_party.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct party_data {
	struct party party;
	unsigned int min_lv, max_lv;
	int family; //Is this party a family? if so, this holds the child id.
	unsigned char size; //Total size of party.
};

static struct party_data *party_pt;
static DBMap* party_db_; // int party_id -> struct party_data*

int mapif_party_broken(int party_id,int flag);
int party_check_empty(struct party_data *p);
int mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id);
int party_check_exp_share(struct party_data *p);
int mapif_party_optionchanged(int fd,struct party *p, int account_id, int flag);

//Updates party's level range and unsets even share if broken.
static int int_party_check_lv(struct party_data *p) {
	int i;
	unsigned int lv;
	p->min_lv = UINT_MAX;
	p->max_lv = 0;
	for(i=0;i<MAX_PARTY;i++){
		/**
		 * - If not online OR if it's a family party and this is the child (doesn't affect exp range)
		 **/
		if(!p->party.member[i].online || p->party.member[i].char_id == p->family )
			continue;

		lv=p->party.member[i].lv;
		if (lv < p->min_lv) p->min_lv = lv;
		if (lv > p->max_lv) p->max_lv = lv;
	}

	if (p->party.exp && !party_check_exp_share(p)) {
		p->party.exp = 0;
		mapif_party_optionchanged(0, &p->party, 0, 0);
		return 0;
	}
	return 1;
}
//Calculates the state of a party.
static void int_party_calc_state(struct party_data *p)
{
	int i;
	unsigned int lv;
	p->min_lv = UINT_MAX;
	p->max_lv = 0;
	p->party.count =
	p->size =
	p->family = 0;

	//Check party size
	for(i=0;i<MAX_PARTY;i++){
		if (!p->party.member[i].lv) continue;
		p->size++;
		if(p->party.member[i].online)
			p->party.count++;
	}
	if( p->size == 2 && ( char_child(p->party.member[0].char_id,p->party.member[1].char_id) || char_child(p->party.member[1].char_id,p->party.member[0].char_id) ) ) {
		//Child should be able to share with either of their parents  [RoM]
		if(p->party.member[0].class_&0x2000) //first slot is the child?
			p->family = p->party.member[0].char_id;
		else
			p->family = p->party.member[1].char_id;
	} else if( p->size == 3 ) {
		//Check Family State.
		p->family = char_family(
			p->party.member[0].char_id,
			p->party.member[1].char_id,
			p->party.member[2].char_id
		);
	}
	//max/min levels.
	for(i=0;i<MAX_PARTY;i++){
		lv=p->party.member[i].lv;
		if (!lv) continue;
		if(p->party.member[i].online &&
			//On families, the kid is not counted towards exp share rules.
			p->party.member[i].char_id != p->family)
		{
			if( lv < p->min_lv ) p->min_lv=lv;
			if( p->max_lv < lv ) p->max_lv=lv;
		}
	}

	if (p->party.exp && !party_check_exp_share(p)) {
		p->party.exp = 0; //Set off even share.
		mapif_party_optionchanged(0, &p->party, 0, 0);
	}
	return;
}

// Save party to mysql
int inter_party_tosql(struct party *p, int flag, int index)
{
	// 'party' ('party_id','name','exp','item','leader_id','leader_char')
	char esc_name[NAME_LENGTH*2+1];// escaped party name
	int party_id;

	if( p == NULL || p->party_id == 0 )
		return 0;
	party_id = p->party_id;

#ifdef NOISY
	ShowInfo("Save party request ("CL_BOLD"%d"CL_RESET" - %s).\n", party_id, p->name);
#endif
	SQL->EscapeStringLen(sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));

	if( flag & PS_BREAK )
	{// Break the party
		// we'll skip name-checking and just reset everyone with the same party id [celest]
		if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%d'", char_db, party_id) )
			Sql_ShowDebug(sql_handle);
		if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` WHERE `party_id`='%d'", party_db, party_id) )
			Sql_ShowDebug(sql_handle);
		//Remove from memory
		idb_remove(party_db_, party_id);
		return 1;
	}

	if( flag & PS_CREATE )
	{// Create party
		if( SQL_ERROR == SQL->Query(sql_handle, "INSERT INTO `%s` "
			"(`name`, `exp`, `item`, `leader_id`, `leader_char`) "
			"VALUES ('%s', '%d', '%d', '%d', '%d')",
			party_db, esc_name, p->exp, p->item, p->member[index].account_id, p->member[index].char_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
		party_id = p->party_id = (int)SQL->LastInsertId(sql_handle);
	}

	if( flag & PS_BASIC )
	{// Update party info.
		if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `name`='%s', `exp`='%d', `item`='%d' WHERE `party_id`='%d'",
			party_db, esc_name, p->exp, p->item, party_id) )
			Sql_ShowDebug(sql_handle);
	}

	if( flag & PS_LEADER )
	{// Update leader
		if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s`  SET `leader_id`='%d', `leader_char`='%d' WHERE `party_id`='%d'",
			party_db, p->member[index].account_id, p->member[index].char_id, party_id) )
			Sql_ShowDebug(sql_handle);
	}
	
	if( flag & PS_ADDMEMBER )
	{// Add one party member.
		if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `party_id`='%d' WHERE `account_id`='%d' AND `char_id`='%d'",
			char_db, party_id, p->member[index].account_id, p->member[index].char_id) )
			Sql_ShowDebug(sql_handle);
	}

	if( flag & PS_DELMEMBER )
	{// Remove one party member.
		if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%d' AND `account_id`='%d' AND `char_id`='%d'",
			char_db, party_id, p->member[index].account_id, p->member[index].char_id) )
			Sql_ShowDebug(sql_handle);
	}

	if( save_log )
		ShowInfo("Party Saved (%d - %s)\n", party_id, p->name);
	return 1;
}

// Read party from mysql
struct party_data *inter_party_fromsql(int party_id)
{
	int leader_id = 0;
	int leader_char = 0;
	struct party_data* p;
	struct party_member* m;
	char* data;
	size_t len;
	int i;

#ifdef NOISY
	ShowInfo("Load party request ("CL_BOLD"%d"CL_RESET")\n", party_id);
#endif
	if( party_id <= 0 )
		return NULL;
	
	//Load from memory
	p = (struct party_data*)idb_get(party_db_, party_id);
	if( p != NULL )
		return p;

	p = party_pt;
	memset(p, 0, sizeof(struct party_data));

	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `party_id`, `name`,`exp`,`item`, `leader_id`, `leader_char` FROM `%s` WHERE `party_id`='%d'", party_db, party_id) )
	{
		Sql_ShowDebug(sql_handle);
		return NULL;
	}

	if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
		return NULL;

	p->party.party_id = party_id;
	SQL->GetData(sql_handle, 1, &data, &len); memcpy(p->party.name, data, min(len, NAME_LENGTH));
	SQL->GetData(sql_handle, 2, &data, NULL); p->party.exp = (atoi(data) ? 1 : 0);
	SQL->GetData(sql_handle, 3, &data, NULL); p->party.item = atoi(data);
	SQL->GetData(sql_handle, 4, &data, NULL); leader_id = atoi(data);
	SQL->GetData(sql_handle, 5, &data, NULL); leader_char = atoi(data);
	SQL->FreeResult(sql_handle);

	// Load members
	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `account_id`,`char_id`,`name`,`base_level`,`last_map`,`online`,`class` FROM `%s` WHERE `party_id`='%d'", char_db, party_id) )
	{
		Sql_ShowDebug(sql_handle);
		return NULL;
	}
	for( i = 0; i < MAX_PARTY && SQL_SUCCESS == SQL->NextRow(sql_handle); ++i )
	{
		m = &p->party.member[i];
		SQL->GetData(sql_handle, 0, &data, NULL); m->account_id = atoi(data);
		SQL->GetData(sql_handle, 1, &data, NULL); m->char_id = atoi(data);
		SQL->GetData(sql_handle, 2, &data, &len); memcpy(m->name, data, min(len, NAME_LENGTH));
		SQL->GetData(sql_handle, 3, &data, NULL); m->lv = atoi(data);
		SQL->GetData(sql_handle, 4, &data, NULL); m->map = mapindex->name2id(data);
		SQL->GetData(sql_handle, 5, &data, NULL); m->online = (atoi(data) ? 1 : 0);
		SQL->GetData(sql_handle, 6, &data, NULL); m->class_ = atoi(data);
		m->leader = (m->account_id == leader_id && m->char_id == leader_char ? 1 : 0);
	}
	SQL->FreeResult(sql_handle);

	if( save_log )
		ShowInfo("Party loaded (%d - %s).\n", party_id, p->party.name);
	//Add party to memory.
	CREATE(p, struct party_data, 1);
	memcpy(p, party_pt, sizeof(struct party_data));
	//init state
	int_party_calc_state(p);
	idb_put(party_db_, party_id, p);
	return p;
}

int inter_party_sql_init(void)
{
	//memory alloc
	party_db_ = idb_alloc(DB_OPT_RELEASE_DATA);
	party_pt = (struct party_data*)aCalloc(sizeof(struct party_data), 1);
	if (!party_pt) {
		ShowFatalError("inter_party_sql_init: Out of Memory!\n");
		exit(EXIT_FAILURE);
	}

	/* Uncomment the following if you want to do a party_db cleanup (remove parties with no members) on startup.[Skotlex]
	ShowStatus("cleaning party table...\n");
	if( SQL_ERROR == SQL->Query(sql_handle, "DELETE FROM `%s` USING `%s` LEFT JOIN `%s` ON `%s`.leader_id =`%s`.account_id AND `%s`.leader_char = `%s`.char_id WHERE `%s`.account_id IS NULL",
		party_db, party_db, char_db, party_db, char_db, party_db, char_db, char_db) )
		Sql_ShowDebug(sql_handle);
	*/
	return 0;
}

void inter_party_sql_final(void)
{
	party_db_->destroy(party_db_, NULL);
	aFree(party_pt);
	return;
}

// Search for the party according to its name
struct party_data* search_partyname(char* str)
{
	char esc_name[NAME_LENGTH*2+1];
	char* data;
	struct party_data* p = NULL;

	SQL->EscapeStringLen(sql_handle, esc_name, str, safestrnlen(str, NAME_LENGTH));
	if( SQL_ERROR == SQL->Query(sql_handle, "SELECT `party_id` FROM `%s` WHERE `name`='%s'", party_db, esc_name) )
		Sql_ShowDebug(sql_handle);
	else if( SQL_SUCCESS == SQL->NextRow(sql_handle) ) {
		SQL->GetData(sql_handle, 0, &data, NULL);
		p = inter_party_fromsql(atoi(data));
	}
	SQL->FreeResult(sql_handle);

	return p;
}

// Returns whether this party can keep having exp share or not.
int party_check_exp_share(struct party_data *p)
{
	return (p->party.count < 2 || p->max_lv - p->min_lv <= party_share_level);
}

// Is there any member in the party?
int party_check_empty(struct party_data *p)
{
	int i;
	if (p==NULL||p->party.party_id==0) return 1;
	for(i=0;i<MAX_PARTY && !p->party.member[i].account_id;i++);
	if (i < MAX_PARTY) return 0;
	// If there is no member, then break the party
	mapif_party_broken(p->party.party_id,0);
	inter_party_tosql(&p->party, PS_BREAK, 0);
	return 1;
}

//-------------------------------------------------------------------
// Communication to the map server


// Create a party whether or not
int mapif_party_created(int fd,int account_id,int char_id,struct party *p)
{
	WFIFOHEAD(fd, 39);
	WFIFOW(fd,0)=0x3820;
	WFIFOL(fd,2)=account_id;
	WFIFOL(fd,6)=char_id;
	if(p!=NULL){
		WFIFOB(fd,10)=0;
		WFIFOL(fd,11)=p->party_id;
		memcpy(WFIFOP(fd,15),p->name,NAME_LENGTH);
		ShowInfo("int_party: Party created (%d - %s)\n",p->party_id,p->name);
	}else{
		WFIFOB(fd,10)=1;
		WFIFOL(fd,11)=0;
		memset(WFIFOP(fd,15),0,NAME_LENGTH);
	}
	WFIFOSET(fd,39);

	return 0;
}

//Party information not found
static void mapif_party_noinfo(int fd, int party_id, int char_id)
{
	WFIFOHEAD(fd, 12);
	WFIFOW(fd,0) = 0x3821;
	WFIFOW(fd,2) = 12;
	WFIFOL(fd,4) = char_id;
	WFIFOL(fd,8) = party_id;
	WFIFOSET(fd,12);
	ShowWarning("int_party: info not found (party_id=%d char_id=%d)\n", party_id, char_id);
}

//Digest party information
static void mapif_party_info(int fd, struct party* p, int char_id)
{
	unsigned char buf[8 + sizeof(struct party)];
	WBUFW(buf,0) = 0x3821;
	WBUFW(buf,2) = 8 + sizeof(struct party);
	WBUFL(buf,4) = char_id;
	memcpy(WBUFP(buf,8), p, sizeof(struct party));

	if(fd<0)
		mapif_sendall(buf,WBUFW(buf,2));
	else
		mapif_send(fd,buf,WBUFW(buf,2));
}

//Whether or not additional party members
int mapif_party_memberadded(int fd, int party_id, int account_id, int char_id, int flag) {
	WFIFOHEAD(fd, 15);
	WFIFOW(fd,0) = 0x3822;
	WFIFOL(fd,2) = party_id;
	WFIFOL(fd,6) = account_id;
	WFIFOL(fd,10) = char_id;
	WFIFOB(fd,14) = flag;
	WFIFOSET(fd,15);

	return 0;
}

// Party setting change notification
int mapif_party_optionchanged(int fd,struct party *p,int account_id,int flag)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x3823;
	WBUFL(buf,2)=p->party_id;
	WBUFL(buf,6)=account_id;
	WBUFW(buf,10)=p->exp;
	WBUFW(buf,12)=p->item;
	WBUFB(buf,14)=flag;
	if(flag==0)
		mapif_sendall(buf,15);
	else
		mapif_send(fd,buf,15);
	return 0;
}

//Withdrawal notification party
int mapif_party_withdraw(int party_id,int account_id, int char_id) {
	unsigned char buf[16];

	WBUFW(buf,0) = 0x3824;
	WBUFL(buf,2) = party_id;
	WBUFL(buf,6) = account_id;
	WBUFL(buf,10) = char_id;
	mapif_sendall(buf, 14);
	return 0;
}

//Party map update notification
int mapif_party_membermoved(struct party *p,int idx)
{
	unsigned char buf[20];

	WBUFW(buf,0) = 0x3825;
	WBUFL(buf,2) = p->party_id;
	WBUFL(buf,6) = p->member[idx].account_id;
	WBUFL(buf,10) = p->member[idx].char_id;
	WBUFW(buf,14) = p->member[idx].map;
	WBUFB(buf,16) = p->member[idx].online;
	WBUFW(buf,17) = p->member[idx].lv;
	mapif_sendall(buf, 19);
	return 0;
}

//Dissolution party notification
int mapif_party_broken(int party_id,int flag)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x3826;
	WBUFL(buf,2)=party_id;
	WBUFB(buf,6)=flag;
	mapif_sendall(buf,7);
	//printf("int_party: broken %d\n",party_id);
	return 0;
}

//Remarks in the party
int mapif_party_message(int party_id,int account_id,char *mes,int len, int sfd)
{
	unsigned char buf[512];
	WBUFW(buf,0)=0x3827;
	WBUFW(buf,2)=len+12;
	WBUFL(buf,4)=party_id;
	WBUFL(buf,8)=account_id;
	memcpy(WBUFP(buf,12),mes,len);
	mapif_sendallwos(sfd, buf,len+12);
	return 0;
}

//-------------------------------------------------------------------
// Communication from the map server


// Create Party
int mapif_parse_CreateParty(int fd, char *name, int item, int item2, struct party_member *leader)
{
	struct party_data *p;
	int i;
	if( (p=search_partyname(name))!=NULL){
		mapif_party_created(fd,leader->account_id,leader->char_id,NULL);
		return 0;
	}
	// Check Authorised letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) == NULL) {
				if( name[i] == '"' ) { /* client-special-char */
					normalize_name(name,"\"");
					mapif_parse_CreateParty(fd,name,item,item2,leader);
					return 0;
				}
				mapif_party_created(fd,leader->account_id,leader->char_id,NULL);
				return 0;
			}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) != NULL) {
				mapif_party_created(fd,leader->account_id,leader->char_id,NULL);
				return 0;
			}
	}

	p = (struct party_data*)aCalloc(1, sizeof(struct party_data));
	
	memcpy(p->party.name,name,NAME_LENGTH);
	p->party.exp=0;
	p->party.item=(item?1:0)|(item2?2:0);

	memcpy(&p->party.member[0], leader, sizeof(struct party_member));
	p->party.member[0].leader=1;
	p->party.member[0].online=1;

	p->party.party_id=-1;//New party.
	if (inter_party_tosql(&p->party,PS_CREATE|PS_ADDMEMBER,0)) {
		//Add party to db
		int_party_calc_state(p);
		idb_put(party_db_, p->party.party_id, p);
		mapif_party_info(fd, &p->party, 0);
		mapif_party_created(fd,leader->account_id,leader->char_id,&p->party);
	} else { //Failed to create party.
		aFree(p);
		mapif_party_created(fd,leader->account_id,leader->char_id,NULL);
	}

	return 0;
}

// Party information request
static void mapif_parse_PartyInfo(int fd, int party_id, int char_id)
{
	struct party_data *p;
	p = inter_party_fromsql(party_id);

	if (p)
		mapif_party_info(fd, &p->party, char_id);
	else
		mapif_party_noinfo(fd, party_id, char_id);
}

// Add a player to party request
int mapif_parse_PartyAddMember(int fd, int party_id, struct party_member *member)
{
	struct party_data *p;
	int i;

	p = inter_party_fromsql(party_id);
	if( p == NULL || p->size == MAX_PARTY ) {
		mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 1);
		return 0;
	}

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == 0 );
	if( i == MAX_PARTY )
	{// Party full
		mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 1);
		return 0;
	}

	memcpy(&p->party.member[i], member, sizeof(struct party_member));
	p->party.member[i].leader = 0;
	if (p->party.member[i].online) p->party.count++;
	p->size++;
	if (p->size == 2 || p->size == 3) // Check family state. And also accept either of their Parents. [RoM]
		int_party_calc_state(p);
	else //Check even share range.
	if (member->lv < p->min_lv || member->lv > p->max_lv || p->family) {
		if (p->family) p->family = 0; //Family state broken.
		int_party_check_lv(p);
	}

	mapif_party_info(-1, &p->party, 0);
	mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 0);
	inter_party_tosql(&p->party, PS_ADDMEMBER, i);

	return 0;
}

//Party setting change request
int mapif_parse_PartyChangeOption(int fd,int party_id,int account_id,int exp,int item)
{
	struct party_data *p;
	int flag = 0;
	p = inter_party_fromsql(party_id);

	if(!p)
		return 0;

	p->party.exp=exp;
	if( exp && !party_check_exp_share(p) ){
		flag|=0x01;
		p->party.exp=0;
	}
	p->party.item = item&0x3; //Filter out invalid values.
	mapif_party_optionchanged(fd,&p->party,account_id,flag);
	inter_party_tosql(&p->party, PS_BASIC, 0);
	return 0;
}

//Request leave party
int mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id)
{
	struct party_data *p;
	int i,j=-1;

	p = inter_party_fromsql(party_id);
	if( p == NULL )
	{// Party does not exists?
		if( SQL_ERROR == SQL->Query(sql_handle, "UPDATE `%s` SET `party_id`='0' WHERE `party_id`='%d'", char_db, party_id) )
			Sql_ShowDebug(sql_handle);
		return 0;
	}

	for (i = 0; i < MAX_PARTY; i++) {
		if(p->party.member[i].account_id == account_id &&
			p->party.member[i].char_id == char_id) {
			break;
		}
	}
	if (i >= MAX_PARTY)
		return 0; //Member not found?

	mapif_party_withdraw(party_id, account_id, char_id);

	if (p->party.member[i].leader){
		p->party.member[i].account_id = 0;
		for (j = 0; j < MAX_PARTY; j++) {
			if (!p->party.member[j].account_id)
				continue;
			mapif_party_withdraw(party_id, p->party.member[j].account_id, p->party.member[j].char_id);
			p->party.member[j].account_id = 0;
		}
		//Party gets deleted on the check_empty call below.
	} else {
		inter_party_tosql(&p->party,PS_DELMEMBER,i);
		j = p->party.member[i].lv;
		if(p->party.member[i].online) p->party.count--;
		memset(&p->party.member[i], 0, sizeof(struct party_member));
		p->size--;
		if (j == p->min_lv || j == p->max_lv || p->family)
		{
			if(p->family) p->family = 0; //Family state broken.
			int_party_check_lv(p);
		}
	}
		
	if (party_check_empty(p) == 0)
		mapif_party_info(-1, &p->party, 0);
	return 0;
}
// When member goes to other map or levels up.
int mapif_parse_PartyChangeMap(int fd, int party_id, int account_id, int char_id, unsigned short map, int online, unsigned int lv)
{
	struct party_data *p;
	int i;

	p = inter_party_fromsql(party_id);
	if (p == NULL)
		return 0;

	for(i = 0; i < MAX_PARTY && 
		(p->party.member[i].account_id != account_id ||
		p->party.member[i].char_id != char_id); i++);

	if (i == MAX_PARTY) return 0;

	if (p->party.member[i].online != online)
	{
		p->party.member[i].online = online;
		if (online)
			p->party.count++;
		else
			p->party.count--;
		// Even share check situations: Family state (always breaks)
		// character logging on/off is max/min level (update level range) 
		// or character logging on/off has a different level (update level range using new level)
		if (p->family ||
			(p->party.member[i].lv <= p->min_lv || p->party.member[i].lv >= p->max_lv) ||
			(p->party.member[i].lv != lv && (lv <= p->min_lv || lv >= p->max_lv))
			)
		{
			p->party.member[i].lv = lv;
			int_party_check_lv(p);
		}
		//Send online/offline update.
		mapif_party_membermoved(&p->party, i);
	}

	if (p->party.member[i].lv != lv) {
		if(p->party.member[i].lv == p->min_lv ||
			p->party.member[i].lv == p->max_lv)
		{
			p->party.member[i].lv = lv;
			int_party_check_lv(p);
		} else
			p->party.member[i].lv = lv;
		//There is no need to send level update to map servers
		//since they do nothing with it.
	}

	if (p->party.member[i].map != map) {
		p->party.member[i].map = map;
		mapif_party_membermoved(&p->party, i);
	}
	return 0;
}

//Request party dissolution
int mapif_parse_BreakParty(int fd,int party_id)
{
	struct party_data *p;

	p = inter_party_fromsql(party_id);

	if(!p)
		return 0;
	inter_party_tosql(&p->party,PS_BREAK,0);
	mapif_party_broken(fd,party_id);
	return 0;
}

//Party sending the message
int mapif_parse_PartyMessage(int fd,int party_id,int account_id,char *mes,int len)
{
	return mapif_party_message(party_id,account_id,mes,len, fd);
}

int mapif_parse_PartyLeaderChange(int fd,int party_id,int account_id,int char_id)
{
	struct party_data *p;
	int i;

	p = inter_party_fromsql(party_id);

	if(!p)
		return 0;

	for (i = 0; i < MAX_PARTY; i++)
	{
		if(p->party.member[i].leader) 
			p->party.member[i].leader = 0;
		if(p->party.member[i].account_id == account_id &&
			p->party.member[i].char_id == char_id)
	  	{
			p->party.member[i].leader = 1;
			inter_party_tosql(&p->party,PS_LEADER, i);
		}
	}
	return 1;
}


// Communication from the map server
//-Analysis that only one packet
// Data packet length is set to inter.c that you
// Do NOT go and check the packet length, RFIFOSKIP is done by the caller
// Return :
// 	0 : error
//	1 : ok
int inter_party_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)) {
	case 0x3020: mapif_parse_CreateParty(fd, (char*)RFIFOP(fd,4), RFIFOB(fd,28), RFIFOB(fd,29), (struct party_member*)RFIFOP(fd,30)); break;
	case 0x3021: mapif_parse_PartyInfo(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
	case 0x3022: mapif_parse_PartyAddMember(fd, RFIFOL(fd,4), (struct party_member*)RFIFOP(fd,8)); break;
	case 0x3023: mapif_parse_PartyChangeOption(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOW(fd,10), RFIFOW(fd,12)); break;
	case 0x3024: mapif_parse_PartyLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x3025: mapif_parse_PartyChangeMap(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOW(fd,14), RFIFOB(fd,16), RFIFOW(fd,17)); break;
	case 0x3026: mapif_parse_BreakParty(fd, RFIFOL(fd,2)); break;
	case 0x3027: mapif_parse_PartyMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3029: mapif_parse_PartyLeaderChange(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	default:
		return 0;
	}
	return 1;
}

//Leave request from the server (for delete character)
int inter_party_leave(int party_id,int account_id, int char_id)
{
	return mapif_parse_PartyLeave(-1,party_id,account_id, char_id);
}

int inter_party_CharOnline(int char_id, int party_id)
{
	struct party_data* p;
	int i;

	if( party_id == -1 )
	{// Get party_id from the database
		char* data;

		if( SQL_ERROR == SQL->Query(sql_handle, "SELECT party_id FROM `%s` WHERE char_id='%d'", char_db, char_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}

		if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
			return 0; //Eh? No party?

		SQL->GetData(sql_handle, 0, &data, NULL);
		party_id = atoi(data);
		SQL->FreeResult(sql_handle);
	}
	if (party_id == 0)
		return 0; //No party...
	
	p = inter_party_fromsql(party_id);
	if(!p) {
		ShowError("Character %d's party %d not found!\n", char_id, party_id);
		return 0;
	}

	//Set member online
	for(i=0; i<MAX_PARTY; i++) {
		if (p->party.member[i].char_id == char_id) {
			if (!p->party.member[i].online) {
				p->party.member[i].online = 1;
				p->party.count++;
				if (p->party.member[i].lv < p->min_lv ||
					p->party.member[i].lv > p->max_lv)
					int_party_check_lv(p);
			}
			break;
		}
	}
	return 1;
}

int inter_party_CharOffline(int char_id, int party_id) {
	struct party_data *p=NULL;
	int i;

	if( party_id == -1 )
	{// Get guild_id from the database
		char* data;

		if( SQL_ERROR == SQL->Query(sql_handle, "SELECT party_id FROM `%s` WHERE char_id='%d'", char_db, char_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}

		if( SQL_SUCCESS != SQL->NextRow(sql_handle) )
			return 0; //Eh? No party?

		SQL->GetData(sql_handle, 0, &data, NULL);
		party_id = atoi(data);
		SQL->FreeResult(sql_handle);
	}
	if (party_id == 0)
		return 0; //No party...
	
	//Character has a party, set character offline and check if they were the only member online
	if ((p = inter_party_fromsql(party_id)) == NULL)
		return 0;

	//Set member offline
	for(i=0; i< MAX_PARTY; i++) {
		if(p->party.member[i].char_id == char_id)
		{
			p->party.member[i].online = 0;
			p->party.count--;
			if(p->party.member[i].lv == p->min_lv ||
				p->party.member[i].lv == p->max_lv)
				int_party_check_lv(p);
			break;
		}
	}

	if(!p->party.count)
		//Parties don't have any data that needs be saved at this point... so just remove it from memory.
		idb_remove(party_db_, party_id);
	return 1;
}
