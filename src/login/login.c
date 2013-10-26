// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/core.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/md5calc.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/HPM.h"
#include "account.h"
#include "ipban.h"
#include "login.h"
#include "loginlog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Login_Config login_config;

int login_fd; // login server socket
struct mmo_char_server server[MAX_SERVERS]; // char server data

static struct account_engine {
	AccountDB* (*constructor)(void);
	AccountDB* db;
} account_engine[] = {
	{account_db_sql, NULL}
};

// account database
AccountDB* accounts = NULL;

//Account registration flood protection [Kevin]
int allowed_regs = 1;
int time_allowed = 10; //in seconds

// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];

int subnet_count = 0;

int mmo_auth_new(const char* userid, const char* pass, const char sex, const char* last_ip);

//-----------------------------------------------------
// Auth database
//-----------------------------------------------------
#define AUTH_TIMEOUT 30000

struct auth_node {

	int account_id;
	uint32 login_id1;
	uint32 login_id2;
	uint32 ip;
	char sex;
	uint32 version;
	uint8 clienttype;
};

static DBMap* auth_db; // int account_id -> struct auth_node*


//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------
struct online_login_data {

	int account_id;
	int waiting_disconnect;
	int char_server;
};

static DBMap* online_db; // int account_id -> struct online_login_data*
static int waiting_disconnect_timer(int tid, int64 tick, int id, intptr_t data);

/**
 * @see DBCreateData
 */
static DBData create_online_user(DBKey key, va_list args)
{
	struct online_login_data* p;
	CREATE(p, struct online_login_data, 1);
	p->account_id = key.i;
	p->char_server = -1;
	p->waiting_disconnect = INVALID_TIMER;
	return DB->ptr2data(p);
}

struct online_login_data* add_online_user(int char_server, int account_id)
{
	struct online_login_data* p;
	p = idb_ensure(online_db, account_id, create_online_user);
	p->char_server = char_server;
	if( p->waiting_disconnect != INVALID_TIMER )
	{
		timer->delete(p->waiting_disconnect, waiting_disconnect_timer);
		p->waiting_disconnect = INVALID_TIMER;
	}
	return p;
}

void remove_online_user(int account_id)
{
	struct online_login_data* p;
	p = (struct online_login_data*)idb_get(online_db, account_id);
	if( p == NULL )
		return;
	if( p->waiting_disconnect != INVALID_TIMER )
		timer->delete(p->waiting_disconnect, waiting_disconnect_timer);

	idb_remove(online_db, account_id);
}

static int waiting_disconnect_timer(int tid, int64 tick, int id, intptr_t data) {
	struct online_login_data* p = (struct online_login_data*)idb_get(online_db, id);
	if( p != NULL && p->waiting_disconnect == tid && p->account_id == id )
	{
		p->waiting_disconnect = INVALID_TIMER;
		remove_online_user(id);
		idb_remove(auth_db, id);
	}
	return 0;
}

/**
 * @see DBApply
 */
static int online_db_setoffline(DBKey key, DBData *data, va_list ap)
{
	struct online_login_data* p = DB->data2ptr(data);
	int server = va_arg(ap, int);
	if( server == -1 )
	{
		p->char_server = -1;
		if( p->waiting_disconnect != INVALID_TIMER )
		{
			timer->delete(p->waiting_disconnect, waiting_disconnect_timer);
			p->waiting_disconnect = INVALID_TIMER;
		}
	}
	else if( p->char_server == server )
		p->char_server = -2; //Char server disconnected.
	return 0;
}

/**
 * @see DBApply
 */
static int online_data_cleanup_sub(DBKey key, DBData *data, va_list ap)
{
	struct online_login_data *character= DB->data2ptr(data);
	if (character->char_server == -2) //Unknown server.. set them offline
		remove_online_user(character->account_id);
	return 0;
}

static int online_data_cleanup(int tid, int64 tick, int id, intptr_t data) {
	online_db->foreach(online_db, online_data_cleanup_sub);
	return 0;
} 


//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, uint8* buf, size_t len)
{
	int i, c;

	for( i = 0, c = 0; i < ARRAYLENGTH(server); ++i )
	{
		int fd = server[i].fd;
		if( session_isValid(fd) && fd != sfd )
		{
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			++c;
		}
	}

	return c;
}


/// Initializes a server structure.
void chrif_server_init(int id)
{
	memset(&server[id], 0, sizeof(server[id]));
	server[id].fd = -1;
}


/// Destroys a server structure.
void chrif_server_destroy(int id)
{
	if( server[id].fd != -1 )
	{
		do_close(server[id].fd);
		server[id].fd = -1;
	}
}


/// Resets all the data related to a server.
void chrif_server_reset(int id)
{
	online_db->foreach(online_db, online_db_setoffline, id); //Set all chars from this char server to offline.
	chrif_server_destroy(id);
	chrif_server_init(id);
}


/// Called when the connection to Char Server is disconnected.
void chrif_on_disconnect(int id)
{
	ShowStatus("Char-server '%s' has disconnected.\n", server[id].name);
	chrif_server_reset(id);
}


//-----------------------------------------------------
// periodic ip address synchronization
//-----------------------------------------------------
static int sync_ip_addresses(int tid, int64 tick, int id, intptr_t data) {
	uint8 buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	charif_sendallwos(-1, buf, 2);
	return 0;
}


//-----------------------------------------------------
// encrypted/unencrypted password check (from eApp)
//-----------------------------------------------------
bool check_encrypted(const char* str1, const char* str2, const char* passwd)
{
	char tmpstr[64+1], md5str[32+1];

	safesnprintf(tmpstr, sizeof(tmpstr), "%s%s", str1, str2);
	MD5_String(tmpstr, md5str);

	return (0==strcmp(passwd, md5str));
}

bool check_password(const char* md5key, int passwdenc, const char* passwd, const char* refpass)
{
	if(passwdenc == 0)
	{
		return (0==strcmp(passwd, refpass));
	}
	else
	{
		// password mode set to 1 -> md5(md5key, refpass) enable with <passwordencrypt></passwordencrypt>
		// password mode set to 2 -> md5(refpass, md5key) enable with <passwordencrypt2></passwordencrypt2>
		
		return ((passwdenc&0x01) && check_encrypted(md5key, refpass, passwd)) ||
		       ((passwdenc&0x02) && check_encrypted(refpass, md5key, passwd));
	}
}


//-----------------------------------------------------
// custom timestamp formatting (from eApp)
//-----------------------------------------------------
const char* timestamp2string(char* str, size_t size, time_t timestamp, const char* format)
{
	size_t len = strftime(str, size, format, localtime(&timestamp));
	memset(str + len, '\0', size - len);
	return str;
}


//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
//--------------------------------------------
int lan_subnetcheck(uint32 ip)
{
	int i;
	ARR_FIND( 0, subnet_count, i, (subnet[i].char_ip & subnet[i].mask) == (ip & subnet[i].mask) );
	return ( i < subnet_count ) ? subnet[i].char_ip : 0;
}

//----------------------------------
// Reading Lan Support configuration
//----------------------------------
int login_lan_config_read(const char *lancfgName)
{
	FILE *fp;
	int line_num = 0;
	char line[1024], w1[64], w2[64], w3[64], w4[64];

	if((fp = fopen(lancfgName, "r")) == NULL) {
		ShowWarning("LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		line_num++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[1] == '\n')
			continue;

		if(sscanf(line,"%[^:]: %[^:]:%[^:]:%[^\r\n]", w1, w2, w3, w4) != 4)
		{
			ShowWarning("Error syntax of configuration file %s in line %d.\n", lancfgName, line_num);
			continue;
		}

		if( strcmpi(w1, "subnet") == 0 )
		{
			subnet[subnet_count].mask = str2ip(w2);
			subnet[subnet_count].char_ip = str2ip(w3);
			subnet[subnet_count].map_ip = str2ip(w4);

			if( (subnet[subnet_count].char_ip & subnet[subnet_count].mask) != (subnet[subnet_count].map_ip & subnet[subnet_count].mask) )
			{
				ShowError("%s: Configuration Error: The char server (%s) and map server (%s) belong to different subnetworks!\n", lancfgName, w3, w4);
				continue;
			}

			subnet_count++;
		}
	}

	if( subnet_count > 1 ) /* only useful if there is more than 1 available */
		ShowStatus("Read information about %d subnetworks.\n", subnet_count);

	fclose(fp);
	return 0;
}

//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int parse_fromchar(int fd)
{
	int j, id;
	uint32 ipl;
	char ip[16];

	ARR_FIND( 0, ARRAYLENGTH(server), id, server[id].fd == fd );
	if( id == ARRAYLENGTH(server) )
	{// not a char server
		ShowDebug("parse_fromchar: Disconnecting invalid session #%d (is not a char-server)\n", fd);
		set_eof(fd);
		do_close(fd);
		return 0;
	}

	if( session[fd]->flag.eof )
	{
		do_close(fd);
		server[id].fd = -1;
		chrif_on_disconnect(id);
		return 0;
	}

	ipl = server[id].ip;
	ip2str(ipl, ip);

	while( RFIFOREST(fd) >= 2 ) {
		uint16 command = RFIFOW(fd,0);

		if( HPM->packetsc[hpParse_FromChar] ) {
			if( (j = HPM->parse_packets(fd,hpParse_FromChar)) ) {
				if( j == 1 ) continue;
				if( j == 2 ) return 0;
			}
		}
		
		switch( command ) {

		case 0x2712: // request from char-server to authenticate an account
			if( RFIFOREST(fd) < 23 )
				return 0;
		{
			struct auth_node* node;

			int account_id = RFIFOL(fd,2);
			uint32 login_id1 = RFIFOL(fd,6);
			uint32 login_id2 = RFIFOL(fd,10);
			uint8 sex = RFIFOB(fd,14);
			//uint32 ip_ = ntohl(RFIFOL(fd,15));
			int request_id = RFIFOL(fd,19);
			RFIFOSKIP(fd,23);

			node = (struct auth_node*)idb_get(auth_db, account_id);
			if( runflag == LOGINSERVER_ST_RUNNING &&
				node != NULL &&
				node->account_id == account_id &&
				node->login_id1  == login_id1 &&
				node->login_id2  == login_id2 &&
				node->sex        == sex_num2str(sex) /*&&
				node->ip         == ip_*/ )
			{// found
				//ShowStatus("Char-server '%s': authentication of the account %d accepted (ip: %s).\n", server[id].name, account_id, ip);

				// send ack
				WFIFOHEAD(fd,25);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOL(fd,6) = login_id1;
				WFIFOL(fd,10) = login_id2;
				WFIFOB(fd,14) = sex;
				WFIFOB(fd,15) = 0;// ok
				WFIFOL(fd,16) = request_id;
				WFIFOL(fd,20) = node->version;
				WFIFOB(fd,24) = node->clienttype;
				WFIFOSET(fd,25);

				// each auth entry can only be used once
				idb_remove(auth_db, account_id);
			}
			else
			{// authentication not found
				ShowStatus("Char-server '%s': authentication of the account %d REFUSED (ip: %s).\n", server[id].name, account_id, ip);
				WFIFOHEAD(fd,25);
				WFIFOW(fd,0) = 0x2713;
				WFIFOL(fd,2) = account_id;
				WFIFOL(fd,6) = login_id1;
				WFIFOL(fd,10) = login_id2;
				WFIFOB(fd,14) = sex;
				WFIFOB(fd,15) = 1;// auth failed
				WFIFOL(fd,16) = request_id;
				WFIFOL(fd,20) = 0;
				WFIFOB(fd,24) = 0;
				WFIFOSET(fd,25);
			}
		}
		break;

		case 0x2714:
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			int users = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			// how many users on world? (update)
			if( server[id].users != users )
			{
				ShowStatus("set users %s : %d\n", server[id].name, users);

				server[id].users = (uint16)users;
			}
		}
		break;

		case 0x2715: // request from char server to change e-email from default "a@a.com"
			if (RFIFOREST(fd) < 46)
				return 0;
		{
			struct mmo_account acc;
			char email[40];

			int account_id = RFIFOL(fd,2);
			safestrncpy(email, (char*)RFIFOP(fd,6), 40); remove_control_chars(email);
			RFIFOSKIP(fd,46);

			if( e_mail_check(email) == 0 )
				ShowNotice("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - e-mail is invalid (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else
			if( !accounts->load_num(accounts, &acc, account_id) || strcmp(acc.email, "a@a.com") == 0 || acc.email[0] == '\0' )
				ShowNotice("Char-server '%s': Attempt to create an e-mail on an account with a default e-mail REFUSED - account doesn't exist or e-mail of account isn't default e-mail (account: %d, ip: %s).\n", server[id].name, account_id, ip);
			else {
				memcpy(acc.email, email, 40);
				ShowNotice("Char-server '%s': Create an e-mail on an account with a default e-mail (account: %d, new e-mail: %s, ip: %s).\n", server[id].name, account_id, email, ip);
				// Save
				accounts->save(accounts, &acc);
			}
		}
		break;

		case 0x2716: // request account data
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			struct mmo_account acc;
			time_t expiration_time = 0;
			char email[40] = "";
			int group_id = 0;
			uint8 char_slots = 0;
			char birthdate[10+1] = "";
			char pincode[4+1] = "\0\0\0\0";

			int account_id = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowNotice("Char-server '%s': account %d NOT found (ip: %s).\n", server[id].name, account_id, ip);
			else {
				safestrncpy(email, acc.email, sizeof(email));
				expiration_time = acc.expiration_time;
				group_id = acc.group_id;
				char_slots = acc.char_slots;
				safestrncpy(pincode, acc.pincode, sizeof(pincode));
				safestrncpy(birthdate, acc.birthdate, sizeof(birthdate));
				if( strlen(pincode) == 0 )
					memset(pincode,'\0',sizeof(pincode));
			}

			WFIFOHEAD(fd,72);
			WFIFOW(fd,0) = 0x2717;
			WFIFOL(fd,2) = account_id;
			safestrncpy((char*)WFIFOP(fd,6), email, 40);
			WFIFOL(fd,46) = (uint32)expiration_time;
			WFIFOB(fd,50) = (unsigned char)group_id;
			WFIFOB(fd,51) = char_slots;
			safestrncpy((char*)WFIFOP(fd,52), birthdate, 10+1);
			safestrncpy((char*)WFIFOP(fd,63), pincode, 4+1 );
			WFIFOL(fd,68) = acc.pincode_change;
			WFIFOSET(fd,72);
		}
		break;

		case 0x2719: // ping request from charserver
			RFIFOSKIP(fd,2);

			WFIFOHEAD(fd,2);
			WFIFOW(fd,0) = 0x2718;
			WFIFOSET(fd,2);
		break;

		// Map server send information to change an email of an account via char-server
		case 0x2722:	// 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			struct mmo_account acc;
			char actual_email[40];
			char new_email[40];

			int account_id = RFIFOL(fd,2);
			safestrncpy(actual_email, (char*)RFIFOP(fd,6), 40);
			safestrncpy(new_email, (char*)RFIFOP(fd,46), 40);
			RFIFOSKIP(fd, 86);

			if( e_mail_check(actual_email) == 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual email is invalid (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else
			if( e_mail_check(new_email) == 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a invalid new e-mail (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else
			if( strcmpi(new_email, "a@a.com") == 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command) with a default e-mail (account: %d, ip: %s)\n", server[id].name, account_id, ip);
			else
			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but account doesn't exist (account: %d, ip: %s).\n", server[id].name, account_id, ip);
			else
			if( strcmpi(acc.email, actual_email) != 0 )
				ShowNotice("Char-server '%s': Attempt to modify an e-mail on an account (@email GM command), but actual e-mail is incorrect (account: %d (%s), actual e-mail: %s, proposed e-mail: %s, ip: %s).\n", server[id].name, account_id, acc.userid, acc.email, actual_email, ip);
			else {
				safestrncpy(acc.email, new_email, 40);
				ShowNotice("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s).\n", server[id].name, account_id, acc.userid, new_email, ip);
				// Save
				accounts->save(accounts, &acc);
			}
		}
		break;

		case 0x2724: // Receiving an account state update request from a map-server (relayed via char-server)
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			struct mmo_account acc;

			int account_id = RFIFOL(fd,2);
			unsigned int state = RFIFOL(fd,6);
			RFIFOSKIP(fd,10);

			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowNotice("Char-server '%s': Error of Status change (account: %d not found, suggested status %d, ip: %s).\n", server[id].name, account_id, state, ip);
			else
			if( acc.state == state )
				ShowNotice("Char-server '%s':  Error of Status change - actual status is already the good status (account: %d, status %d, ip: %s).\n", server[id].name, account_id, state, ip);
			else {
				ShowNotice("Char-server '%s': Status change (account: %d, new status %d, ip: %s).\n", server[id].name, account_id, state, ip);

				acc.state = state;
				// Save
				accounts->save(accounts, &acc);

				// notify other servers
				if (state != 0) {
					uint8 buf[11];
					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = account_id;
					WBUFB(buf,6) = 0; // 0: change of state, 1: ban
					WBUFL(buf,7) = state; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
				}
			}
		}
		break;

		case 0x2725: // Receiving of map-server via char-server a ban request
			if (RFIFOREST(fd) < 18)
				return 0;
		{
			struct mmo_account acc;

			int account_id = RFIFOL(fd,2);
			int year = (short)RFIFOW(fd,6);
			int month = (short)RFIFOW(fd,8);
			int mday = (short)RFIFOW(fd,10);
			int hour = (short)RFIFOW(fd,12);
			int min = (short)RFIFOW(fd,14);
			int sec = (short)RFIFOW(fd,16);
			RFIFOSKIP(fd,18);

			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowNotice("Char-server '%s': Error of ban request (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				time_t timestamp;
				struct tm *tmtime;
				if (acc.unban_time == 0 || acc.unban_time < time(NULL))
					timestamp = time(NULL); // new ban
				else
					timestamp = acc.unban_time; // add to existing ban
				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + year;
				tmtime->tm_mon  = tmtime->tm_mon + month;
				tmtime->tm_mday = tmtime->tm_mday + mday;
				tmtime->tm_hour = tmtime->tm_hour + hour;
				tmtime->tm_min  = tmtime->tm_min + min;
				tmtime->tm_sec  = tmtime->tm_sec + sec;
				timestamp = mktime(tmtime);
				if (timestamp == -1)
					ShowNotice("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s).\n", server[id].name, account_id, ip);
				else
				if( timestamp <= time(NULL) || timestamp == 0 )
					ShowNotice("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s).\n", server[id].name, account_id, ip);
				else
				{
					uint8 buf[11];
					char tmpstr[24];
					timestamp2string(tmpstr, sizeof(tmpstr), timestamp, login_config.date_format);
					ShowNotice("Char-server '%s': Ban request (account: %d, new final date of banishment: %d (%s), ip: %s).\n", server[id].name, account_id, timestamp, tmpstr, ip);

					acc.unban_time = timestamp;

					// Save
					accounts->save(accounts, &acc);

					WBUFW(buf,0) = 0x2731;
					WBUFL(buf,2) = account_id;
					WBUFB(buf,6) = 1; // 0: change of status, 1: ban
					WBUFL(buf,7) = (uint32)timestamp; // status or final date of a banishment
					charif_sendallwos(-1, buf, 11);
				}
			}
		}
		break;

		case 0x2727: // Change of sex (sex is reversed)
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			struct mmo_account acc;

			int account_id = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowNotice("Char-server '%s': Error of sex change (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			if( acc.sex == 'S' )
				ShowNotice("Char-server '%s': Error of sex change - account to change is a Server account (account: %d, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				unsigned char buf[7];
				char sex = ( acc.sex == 'M' ) ? 'F' : 'M'; //Change gender

				ShowNotice("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s).\n", server[id].name, account_id, sex, ip);

				acc.sex = sex;
				// Save
				accounts->save(accounts, &acc);

				// announce to other servers
				WBUFW(buf,0) = 0x2723;
				WBUFL(buf,2) = account_id;
				WBUFB(buf,6) = sex_str2num(sex);
				charif_sendallwos(-1, buf, 7);
			}
		}
		break;

		case 0x2728:	// We receive account_reg2 from a char-server, and we send them to other map-servers.
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
				return 0;
		{
			struct mmo_account acc;

			int account_id = RFIFOL(fd,4);

			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowStatus("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				int len;
				int p;
				ShowNotice("char-server '%s': receiving (from the char-server) of account_reg2 (account: %d, ip: %s).\n", server[id].name, account_id, ip);
				for( j = 0, p = 13; j < ACCOUNT_REG2_NUM && p < RFIFOW(fd,2); ++j )
				{
					sscanf((char*)RFIFOP(fd,p), "%31c%n", acc.account_reg2[j].str, &len);
					acc.account_reg2[j].str[len]='\0';
					p +=len+1; //+1 to skip the '\0' between strings.
					sscanf((char*)RFIFOP(fd,p), "%255c%n", acc.account_reg2[j].value, &len);
					acc.account_reg2[j].value[len]='\0';
					p +=len+1;
					remove_control_chars(acc.account_reg2[j].str);
					remove_control_chars(acc.account_reg2[j].value);
				}
				acc.account_reg2_num = j;

				// Save
				accounts->save(accounts, &acc);

				// Sending information towards the other char-servers.
				RFIFOW(fd,0) = 0x2729;// reusing read buffer
				charif_sendallwos(fd, RFIFOP(fd,0), RFIFOW(fd,2));
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		}
		break;

		case 0x272a:	// Receiving of map-server via char-server an unban request
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			struct mmo_account acc;

			int account_id = RFIFOL(fd,2);
			RFIFOSKIP(fd,6);

			if( !accounts->load_num(accounts, &acc, account_id) )
				ShowNotice("Char-server '%s': Error of UnBan request (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
			else
			if( acc.unban_time == 0 )
				ShowNotice("Char-server '%s': Error of UnBan request (account: %d, no change for unban date, ip: %s).\n", server[id].name, account_id, ip);
			else
			{
				ShowNotice("Char-server '%s': UnBan request (account: %d, ip: %s).\n", server[id].name, account_id, ip);
				acc.unban_time = 0;
				accounts->save(accounts, &acc);
			}
		}
		break;

		case 0x272b:    // Set account_id to online [Wizputer]
			if( RFIFOREST(fd) < 6 )
				return 0;
			add_online_user(id, RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
		break;

		case 0x272c:   // Set account_id to offline [Wizputer]
			if( RFIFOREST(fd) < 6 )
				return 0;
			remove_online_user(RFIFOL(fd,2));
			RFIFOSKIP(fd,6);
		break;

		case 0x272d:	// Receive list of all online accounts. [Skotlex]
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				struct online_login_data *p;
				int aid;
				uint32 i, users;
				online_db->foreach(online_db, online_db_setoffline, id); //Set all chars from this char-server offline first
				users = RFIFOW(fd,4);
				for (i = 0; i < users; i++) {
					aid = RFIFOL(fd,6+i*4);
					p = idb_ensure(online_db, aid, create_online_user);
					p->char_server = id;
					if (p->waiting_disconnect != INVALID_TIMER)
					{
						timer->delete(p->waiting_disconnect, waiting_disconnect_timer);
						p->waiting_disconnect = INVALID_TIMER;
					}
				}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x272e: //Request account_reg2 for a character.
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			struct mmo_account acc;
			size_t off;

			int account_id = RFIFOL(fd,2);
			int char_id = RFIFOL(fd,6);
			RFIFOSKIP(fd,10);

			WFIFOHEAD(fd,ACCOUNT_REG2_NUM*sizeof(struct global_reg));
			WFIFOW(fd,0) = 0x2729;
			WFIFOL(fd,4) = account_id;
			WFIFOL(fd,8) = char_id;
			WFIFOB(fd,12) = 1; //Type 1 for Account2 registry

			off = 13;
			if( accounts->load_num(accounts, &acc, account_id) )
			{
				for( j = 0; j < acc.account_reg2_num; j++ )
				{
					if( acc.account_reg2[j].str[0] != '\0' )
					{
						off += sprintf((char*)WFIFOP(fd,off), "%s", acc.account_reg2[j].str)+1; //We add 1 to consider the '\0' in place.
						off += sprintf((char*)WFIFOP(fd,off), "%s", acc.account_reg2[j].value)+1;
					}
				}
			}

			WFIFOW(fd,2) = (uint16)off;
			WFIFOSET(fd,WFIFOW(fd,2));
		}
		break;

		case 0x2736: // WAN IP update from char-server
			if( RFIFOREST(fd) < 6 )
				return 0;
			server[id].ip = ntohl(RFIFOL(fd,2));
			ShowInfo("Updated IP of Server #%d to %d.%d.%d.%d.\n",id, CONVIP(server[id].ip));
			RFIFOSKIP(fd,6);
		break;

		case 0x2737: //Request to set all offline.
			ShowInfo("Setting accounts from char-server %d offline.\n", id);
			online_db->foreach(online_db, online_db_setoffline, id);
			RFIFOSKIP(fd,2);
		break;

		case 0x2738: //Change PIN Code for a account
			if( RFIFOREST(fd) < 11 )
				return 0;
			else {
				struct mmo_account acc;
				
				if( accounts->load_num(accounts, &acc, RFIFOL(fd,2) ) ) {
					strncpy( acc.pincode, (char*)RFIFOP(fd,6), 5 );
					acc.pincode_change = ((unsigned int)time( NULL ));
					accounts->save(accounts, &acc);
				}
				RFIFOSKIP(fd,11);
			}
			break;
			
		case 0x2739: // PIN Code was entered wrong too often
			if( RFIFOREST(fd) < 6 )
				return 0;
			else {
				struct mmo_account acc;
				
				if( accounts->load_num(accounts, &acc, RFIFOL(fd,2) ) ) {
					struct online_login_data* ld;

					if( ( ld = (struct online_login_data*)idb_get(online_db,acc.account_id) ) == NULL )
						return 0;
					
					login_log( host2ip(acc.last_ip), acc.userid, 100, "PIN Code check failed" );
				}
				
				remove_online_user(acc.account_id);
				RFIFOSKIP(fd,6);
			}
		break;
				
		default:
			ShowError("parse_fromchar: Unknown packet 0x%x from a char-server! Disconnecting!\n", command);
			set_eof(fd);
			return 0;
		} // switch
	} // while

	return 0;
}


//-------------------------------------
// Make new account
//-------------------------------------
int mmo_auth_new(const char* userid, const char* pass, const char sex, const char* last_ip) {
	static int num_regs = 0; // registration counter
	static int64 new_reg_tick = 0;
	int64 tick = timer->gettick();
	struct mmo_account acc;

	//Account Registration Flood Protection by [Kevin]
	if( new_reg_tick == 0 )
		new_reg_tick = timer->gettick();
	if( DIFF_TICK(tick, new_reg_tick) < 0 && num_regs >= allowed_regs ) {
		ShowNotice("Account registration denied (registration limit exceeded)\n");
		return 3;
	}

	if( login_config.new_acc_length_limit && ( strlen(userid) < 4 || strlen(pass) < 4 ) )
		return 1;

	// check for invalid inputs
	if( sex != 'M' && sex != 'F' )
		return 0; // 0 = Unregistered ID

	// check if the account doesn't exist already
	if( accounts->load_str(accounts, &acc, userid) ) {
		ShowNotice("Attempt of creation of an already existant account (account: %s_%c, pass: %s, received pass: %s)\n", userid, sex, acc.pass, pass);
		return 1; // 1 = Incorrect Password
	}

	memset(&acc, '\0', sizeof(acc));
	acc.account_id = -1; // assigned by account db
	safestrncpy(acc.userid, userid, sizeof(acc.userid));
	safestrncpy(acc.pass, pass, sizeof(acc.pass));
	acc.sex = sex;
	safestrncpy(acc.email, "a@a.com", sizeof(acc.email));
	acc.expiration_time = ( login_config.start_limited_time != -1 ) ? time(NULL) + login_config.start_limited_time : 0;
	safestrncpy(acc.lastlogin, "0000-00-00 00:00:00", sizeof(acc.lastlogin));
	safestrncpy(acc.last_ip, last_ip, sizeof(acc.last_ip));
	safestrncpy(acc.birthdate, "0000-00-00", sizeof(acc.birthdate));
	safestrncpy(acc.pincode, "\0", sizeof(acc.pincode));
	acc.pincode_change = 0;
	acc.char_slots = 0;
	
	if( !accounts->create(accounts, &acc) )
		return 0;

	ShowNotice("Account creation (account %s, id: %d, pass: %s, sex: %c)\n", acc.userid, acc.account_id, acc.pass, acc.sex);

	if( DIFF_TICK(tick, new_reg_tick) > 0 ) {// Update the registration check.
		num_regs = 0;
		new_reg_tick = tick + time_allowed*1000;
	}
	++num_regs;

	return -1;
}

//-----------------------------------------------------
// Check/authentication of a connection
//-----------------------------------------------------
int mmo_auth(struct login_session_data* sd, bool isServer) {
	struct mmo_account acc;
	int len;

	char ip[16];
	ip2str(session[sd->fd]->client_addr, ip);

	// DNS Blacklist check
	if( login_config.use_dnsbl ) {
		char r_ip[16];
		char ip_dnsbl[256];
		char* dnsbl_serv;
		uint8* sin_addr = (uint8*)&session[sd->fd]->client_addr;

		sprintf(r_ip, "%u.%u.%u.%u", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);

		for( dnsbl_serv = strtok(login_config.dnsbl_servs,","); dnsbl_serv != NULL; dnsbl_serv = strtok(NULL,",") ) {
			sprintf(ip_dnsbl, "%s.%s", r_ip, trim(dnsbl_serv));
			if( host2ip(ip_dnsbl) ) {
				ShowInfo("DNSBL: (%s) Blacklisted. User Kicked.\n", r_ip);
				return 3;
			}
		}

	}

	//Client Version check
	if( login_config.check_client_version && sd->version != login_config.client_version_to_connect )
		return 5;

	len = strnlen(sd->userid, NAME_LENGTH);

	// Account creation with _M/_F
	if( login_config.new_account_flag ) {
		if( len > 2 && strnlen(sd->passwd, NAME_LENGTH) > 0 && // valid user and password lengths
			sd->passwdenc == 0 && // unencoded password
			sd->userid[len-2] == '_' && memchr("FfMm", sd->userid[len-1], 4) ) // _M/_F suffix
		{
			int result;

			// remove the _M/_F suffix
			len -= 2;
			sd->userid[len] = '\0';

			result = mmo_auth_new(sd->userid, sd->passwd, TOUPPER(sd->userid[len+1]), ip);
			if( result != -1 )
				return result;// Failed to make account. [Skotlex].
		}
	}
	
	if( !accounts->load_str(accounts, &acc, sd->userid) ) {
		ShowNotice("Unknown account (account: %s, received pass: %s, ip: %s)\n", sd->userid, sd->passwd, ip);
		return 0; // 0 = Unregistered ID
	}

	if( !check_password(sd->md5key, sd->passwdenc, sd->passwd, acc.pass) ) {
		ShowNotice("Invalid password (account: '%s', pass: '%s', received pass: '%s', ip: %s)\n", sd->userid, acc.pass, sd->passwd, ip);
		return 1; // 1 = Incorrect Password
	}

	if( acc.expiration_time != 0 && acc.expiration_time < time(NULL) ) {
		ShowNotice("Connection refused (account: %s, pass: %s, expired ID, ip: %s)\n", sd->userid, sd->passwd, ip);
		return 2; // 2 = This ID is expired
	}

	if( acc.unban_time != 0 && acc.unban_time > time(NULL) ) {
		char tmpstr[24];
		timestamp2string(tmpstr, sizeof(tmpstr), acc.unban_time, login_config.date_format);
		ShowNotice("Connection refused (account: %s, pass: %s, banned until %s, ip: %s)\n", sd->userid, sd->passwd, tmpstr, ip);
		return 6; // 6 = Your are Prohibited to log in until %s
	}

	if( acc.state != 0 ) {
		ShowNotice("Connection refused (account: %s, pass: %s, state: %d, ip: %s)\n", sd->userid, sd->passwd, acc.state, ip);
		return acc.state - 1;
	}
	
	if( login_config.client_hash_check && !isServer ) {
		struct client_hash_node *node = login_config.client_hash_nodes;
		bool match = false;

		if( !sd->has_client_hash ) {
			ShowNotice("Client doesn't sent client hash (account: %s, pass: %s, ip: %s)\n", sd->userid, sd->passwd, acc.state, ip);
			return 5;
		}

		while( node ) {
			if( node->group_id <= acc.group_id && memcmp(node->hash, sd->client_hash, 16) == 0 ) {
				match = true;
				break;
			}

			node = node->next;
		}

		if( !match ) {
			char smd5[33];
			int i;

			for( i = 0; i < 16; i++ )
				sprintf(&smd5[i * 2], "%02x", sd->client_hash[i]);

			ShowNotice("Invalid client hash (account: %s, pass: %s, sent md5: %d, ip: %s)\n", sd->userid, sd->passwd, smd5, ip);
			return 5;
		}
	}

	ShowNotice("Authentication accepted (account: %s, id: %d, ip: %s)\n", sd->userid, acc.account_id, ip);

	// update session data
	sd->account_id = acc.account_id;
	sd->login_id1 = rnd() + 1;
	sd->login_id2 = rnd() + 1;
	safestrncpy(sd->lastlogin, acc.lastlogin, sizeof(sd->lastlogin));
	sd->sex = acc.sex;
	sd->group_id = (uint8)acc.group_id;

	// update account data
	timestamp2string(acc.lastlogin, sizeof(acc.lastlogin), time(NULL), "%Y-%m-%d %H:%M:%S");
	safestrncpy(acc.last_ip, ip, sizeof(acc.last_ip));
	acc.unban_time = 0;
	acc.logincount++;

	accounts->save(accounts, &acc);

	if( sd->sex != 'S' && sd->account_id < START_ACCOUNT_NUM )
		ShowWarning("Account %s has account id %d! Account IDs must be over %d to work properly!\n", sd->userid, sd->account_id, START_ACCOUNT_NUM);

	return -1; // account OK
}

void login_auth_ok(struct login_session_data* sd)
{
	int fd = sd->fd;
	uint32 ip = session[fd]->client_addr;

	uint8 server_num, n;
	uint32 subnet_char_ip;
	struct auth_node* node;
	int i;

	if( runflag != LOGINSERVER_ST_RUNNING )
	{
		// players can only login while running
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x81;
		WFIFOB(fd,2) = 1;// server closed
		WFIFOSET(fd,3);
		return;
	}

	if( login_config.group_id_to_connect >= 0 && sd->group_id != login_config.group_id_to_connect ) {
		ShowStatus("Connection refused: the required group id for connection is %d (account: %s, group: %d).\n", login_config.group_id_to_connect, sd->userid, sd->group_id);
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x81;
		WFIFOB(fd,2) = 1; // 01 = Server closed
		WFIFOSET(fd,3);
		return;
	} else if( login_config.min_group_id_to_connect >= 0 && login_config.group_id_to_connect == -1 && sd->group_id < login_config.min_group_id_to_connect ) {
		ShowStatus("Connection refused: the minium group id required for connection is %d (account: %s, group: %d).\n", login_config.min_group_id_to_connect, sd->userid, sd->group_id);
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x81;
		WFIFOB(fd,2) = 1; // 01 = Server closed
		WFIFOSET(fd,3);
		return;		
	}

	server_num = 0;
	for( i = 0; i < ARRAYLENGTH(server); ++i )
		if( session_isActive(server[i].fd) )
			server_num++;

	if( server_num == 0 )
	{// if no char-server, don't send void list of servers, just disconnect the player with proper message
		ShowStatus("Connection refused: there is no char-server online (account: %s).\n", sd->userid);
		WFIFOHEAD(fd,3);
		WFIFOW(fd,0) = 0x81;
		WFIFOB(fd,2) = 1; // 01 = Server closed
		WFIFOSET(fd,3);
		return;
	}

	{
		struct online_login_data* data = (struct online_login_data*)idb_get(online_db, sd->account_id);
		if( data )
		{// account is already marked as online!
			if( data->char_server > -1 )
			{// Request char servers to kick this account out. [Skotlex]
				uint8 buf[6];
				ShowNotice("User '%s' is already online - Rejected.\n", sd->userid);
				WBUFW(buf,0) = 0x2734;
				WBUFL(buf,2) = sd->account_id;
				charif_sendallwos(-1, buf, 6);
				if( data->waiting_disconnect == INVALID_TIMER )
					data->waiting_disconnect = timer->add(timer->gettick()+AUTH_TIMEOUT, waiting_disconnect_timer, sd->account_id, 0);

				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x81;
				WFIFOB(fd,2) = 8; // 08 = Server still recognizes your last login
				WFIFOSET(fd,3);
				return;
			}
			else
			if( data->char_server == -1 )
			{// client has authed but did not access char-server yet
				// wipe previous session
				idb_remove(auth_db, sd->account_id);
				remove_online_user(sd->account_id);
				data = NULL;
			}
		}
	}

	login_log(ip, sd->userid, 100, "login ok");
	ShowStatus("Connection of the account '%s' accepted.\n", sd->userid);

	WFIFOHEAD(fd,47+32*server_num);
	WFIFOW(fd,0) = 0x69;
	WFIFOW(fd,2) = 47+32*server_num;
	WFIFOL(fd,4) = sd->login_id1;
	WFIFOL(fd,8) = sd->account_id;
	WFIFOL(fd,12) = sd->login_id2;
	WFIFOL(fd,16) = 0; // in old version, that was for ip (not more used)
	//memcpy(WFIFOP(fd,20), sd->lastlogin, 24); // in old version, that was for name (not more used)
	memset(WFIFOP(fd,20), 0, 24);
	WFIFOW(fd,44) = 0; // unknown
	WFIFOB(fd,46) = sex_str2num(sd->sex);
	for( i = 0, n = 0; i < ARRAYLENGTH(server); ++i )
	{
		if( !session_isValid(server[i].fd) )
			continue;

		subnet_char_ip = lan_subnetcheck(ip); // Advanced subnet check [LuzZza]
		WFIFOL(fd,47+n*32) = htonl((subnet_char_ip) ? subnet_char_ip : server[i].ip);
		WFIFOW(fd,47+n*32+4) = ntows(htons(server[i].port)); // [!] LE byte order here [!]
		memcpy(WFIFOP(fd,47+n*32+6), server[i].name, 20);
		WFIFOW(fd,47+n*32+26) = server[i].users;
		WFIFOW(fd,47+n*32+28) = server[i].type;
		WFIFOW(fd,47+n*32+30) = server[i].new_;
		n++;
	}
	WFIFOSET(fd,47+32*server_num);

	// create temporary auth entry
	CREATE(node, struct auth_node, 1);
	node->account_id = sd->account_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->sex;
	node->ip = ip;
	node->version = sd->version;
	node->clienttype = sd->clienttype;
	idb_put(auth_db, sd->account_id, node);

	{
		struct online_login_data* data;

		// mark client as 'online'
		data = add_online_user(-1, sd->account_id);

		// schedule deletion of this node
		data->waiting_disconnect = timer->add(timer->gettick()+AUTH_TIMEOUT, waiting_disconnect_timer, sd->account_id, 0);
	}
}

void login_auth_failed(struct login_session_data* sd, int result)
{
	int fd = sd->fd;
	uint32 ip = session[fd]->client_addr;

	if (login_config.log_login)
	{
		const char* error;
		switch( result ) {
		case   0: error = "Unregistered ID."; break; // 0 = Unregistered ID
		case   1: error = "Incorrect Password."; break; // 1 = Incorrect Password
		case   2: error = "Account Expired."; break; // 2 = This ID is expired
		case   3: error = "Rejected from server."; break; // 3 = Rejected from Server
		case   4: error = "Blocked by GM."; break; // 4 = You have been blocked by the GM Team
		case   5: error = "Not latest game EXE."; break; // 5 = Your Game's EXE file is not the latest version
		case   6: error = "Banned."; break; // 6 = Your are Prohibited to log in until %s
		case   7: error = "Server Over-population."; break; // 7 = Server is jammed due to over populated
		case   8: error = "Account limit from company"; break; // 8 = No more accounts may be connected from this company
		case   9: error = "Ban by DBA"; break; // 9 = MSI_REFUSE_BAN_BY_DBA
		case  10: error = "Email not confirmed"; break; // 10 = MSI_REFUSE_EMAIL_NOT_CONFIRMED
		case  11: error = "Ban by GM"; break; // 11 = MSI_REFUSE_BAN_BY_GM
		case  12: error = "Working in DB"; break; // 12 = MSI_REFUSE_TEMP_BAN_FOR_DBWORK
		case  13: error = "Self Lock"; break; // 13 = MSI_REFUSE_SELF_LOCK
		case  14: error = "Not Permitted Group"; break; // 14 = MSI_REFUSE_NOT_PERMITTED_GROUP
		case  15: error = "Not Permitted Group"; break; // 15 = MSI_REFUSE_NOT_PERMITTED_GROUP
		case  99: error = "Account gone."; break; // 99 = This ID has been totally erased
		case 100: error = "Login info remains."; break; // 100 = Login information remains at %s
		case 101: error = "Hacking investigation."; break; // 101 = Account has been locked for a hacking investigation. Please contact the GM Team for more information
		case 102: error = "Bug investigation."; break; // 102 = This account has been temporarily prohibited from login due to a bug-related investigation
		case 103: error = "Deleting char."; break; // 103 = This character is being deleted. Login is temporarily unavailable for the time being
		case 104: error = "Deleting spouse char."; break; // 104 = This character is being deleted. Login is temporarily unavailable for the time being
		default : error = "Unknown Error."; break;
		}

		login_log(ip, sd->userid, result, error);
	}

	if( result == 1 && login_config.dynamic_pass_failure_ban )
		ipban_log(ip); // log failed password attempt

#if PACKETVER >= 20120000 /* not sure when this started */
	WFIFOHEAD(fd,26);
	WFIFOW(fd,0) = 0x83e;
	WFIFOL(fd,2) = result;
	if( result != 6 )
		memset(WFIFOP(fd,6), '\0', 20);
	else { // 6 = Your are Prohibited to log in until %s
		struct mmo_account acc;
		time_t unban_time = ( accounts->load_str(accounts, &acc, sd->userid) ) ? acc.unban_time : 0;
		timestamp2string((char*)WFIFOP(fd,6), 20, unban_time, login_config.date_format);
	}
	WFIFOSET(fd,26);
#else
	WFIFOHEAD(fd,23);
	WFIFOW(fd,0) = 0x6a;
	WFIFOB(fd,2) = (uint8)result;
	if( result != 6 )
		memset(WFIFOP(fd,3), '\0', 20);
	else { // 6 = Your are Prohibited to log in until %s
		struct mmo_account acc;
		time_t unban_time = ( accounts->load_str(accounts, &acc, sd->userid) ) ? acc.unban_time : 0;
		timestamp2string((char*)WFIFOP(fd,3), 20, unban_time, login_config.date_format);
	}
	WFIFOSET(fd,23);
#endif
}


//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or char-server connection requests)
//----------------------------------------------------------------------------------------
int parse_login(int fd)
{
	struct login_session_data* sd = (struct login_session_data*)session[fd]->session_data;
	int result;

	char ip[16];
	uint32 ipl = session[fd]->client_addr;
	ip2str(ipl, ip);

	if( session[fd]->flag.eof )
	{
		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", ip);
		do_close(fd);
		return 0;
	}

	if( sd == NULL )
	{
		// Perform ip-ban check
		if( login_config.ipban && ipban_check(ipl) )
		{
			ShowStatus("Connection refused: IP isn't authorised (deny/allow, ip: %s).\n", ip);
			login_log(ipl, "unknown", -3, "ip banned");
			WFIFOHEAD(fd,23);
			WFIFOW(fd,0) = 0x6a;
			WFIFOB(fd,2) = 3; // 3 = Rejected from Server
			WFIFOSET(fd,23);
			set_eof(fd);
			return 0;
		}

		// create a session for this new connection
		CREATE(session[fd]->session_data, struct login_session_data, 1);
		sd = (struct login_session_data*)session[fd]->session_data;
		sd->fd = fd;
	}

	while( RFIFOREST(fd) >= 2 ) {
		uint16 command = RFIFOW(fd,0);

		if( HPM->packetsc[hpParse_Login] ) {
			if( (result = HPM->parse_packets(fd,hpParse_Login)) ) {
				if( result == 1 ) continue;
				if( result == 2 ) return 0;
			}
		}
		
		switch( command ) {

		case 0x0200:		// New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26)
				return 0;
			RFIFOSKIP(fd,26);
		break;

		// client md5 hash (binary)
		case 0x0204: // S 0204 <md5 hash>.16B (kRO 2004-05-31aSakexe langtype 0 and 6)
			if (RFIFOREST(fd) < 18)
				return 0;

			sd->has_client_hash = 1;
			memcpy(sd->client_hash, RFIFOP(fd, 2), 16);

			RFIFOSKIP(fd,18);
		break;

		// request client login (raw password)
		case 0x0064: // S 0064 <version>.L <username>.24B <password>.24B <clienttype>.B
		case 0x0277: // S 0277 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B
		case 0x02b0: // S 02b0 <version>.L <username>.24B <password>.24B <clienttype>.B <ip address>.16B <adapter address>.13B <g_isGravityID>.B
		// request client login (md5-hashed password)
		case 0x01dd: // S 01dd <version>.L <username>.24B <password hash>.16B <clienttype>.B
		case 0x01fa: // S 01fa <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.B(index of the connection in the clientinfo file (+10 if the command-line contains "pc"))
		case 0x027c: // S 027c <version>.L <username>.24B <password hash>.16B <clienttype>.B <?>.13B(junk)
		case 0x0825: // S 0825 <packetsize>.W <version>.L <clienttype>.B <userid>.24B <password>.27B <mac>.17B <ip>.15B <token>.(packetsize - 0x5C)B
		{
			size_t packet_len = RFIFOREST(fd);

			if( (command == 0x0064 && packet_len < 55)
			||  (command == 0x0277 && packet_len < 84)
			||  (command == 0x02b0 && packet_len < 85)
			||  (command == 0x01dd && packet_len < 47)
			||  (command == 0x01fa && packet_len < 48)
			||  (command == 0x027c && packet_len < 60)
			||  (command == 0x0825 && (packet_len < 4 || packet_len < RFIFOW(fd, 2))) )
				return 0;
		}
		{
			uint32 version;
			char username[NAME_LENGTH];
			char password[PASSWD_LEN];
			unsigned char passhash[16];
			uint8 clienttype;
			bool israwpass = (command==0x0064 || command==0x0277 || command==0x02b0 || command == 0x0825);
			
			// Shinryo: For the time being, just use token as password.
			if(command == 0x0825)
			{
				char *accname = (char *)RFIFOP(fd, 9);
				char *token = (char *)RFIFOP(fd, 0x5C);
				size_t uAccLen = strlen(accname);
				size_t uTokenLen = RFIFOREST(fd) - 0x5C;

				version = RFIFOL(fd,4);
				
				if(uAccLen <= 0 || uTokenLen <= 0) {
					login_auth_failed(sd, 3);
					return 0;
				}

				safestrncpy(username, accname, NAME_LENGTH);
				safestrncpy(password, token, min(uTokenLen+1, PASSWD_LEN)); // Variable-length field, don't copy more than necessary
				clienttype = RFIFOB(fd, 8);
			}
			else
			{
				version = RFIFOL(fd,2);
				safestrncpy(username, (const char*)RFIFOP(fd,6), NAME_LENGTH);
				if( israwpass )
				{
					safestrncpy(password, (const char*)RFIFOP(fd,30), NAME_LENGTH);
					clienttype = RFIFOB(fd,54);
				}
				else
				{
					memcpy(passhash, RFIFOP(fd,30), 16);
					clienttype = RFIFOB(fd,46);
				}
			}
			RFIFOSKIP(fd,RFIFOREST(fd)); // assume no other packet was sent

			sd->clienttype = clienttype;
			sd->version = version;
			safestrncpy(sd->userid, username, NAME_LENGTH);
			if( israwpass )
			{
				ShowStatus("Request for connection of %s (ip: %s).\n", sd->userid, ip);
				safestrncpy(sd->passwd, password, PASSWD_LEN);
				if( login_config.use_md5_passwds )
					MD5_String(sd->passwd, sd->passwd);
				sd->passwdenc = 0;
			}
			else
			{
				ShowStatus("Request for connection (passwdenc mode) of %s (ip: %s).\n", sd->userid, ip);
				bin2hex(sd->passwd, passhash, 16); // raw binary data here!
				sd->passwdenc = PASSWORDENC;
			}

			if( sd->passwdenc != 0 && login_config.use_md5_passwds )
			{
				login_auth_failed(sd, 3); // send "rejected from server"
				return 0;
			}
			
			result = mmo_auth(sd, false);

			if( result == -1 )
				login_auth_ok(sd);
			else
				login_auth_failed(sd, result);
		}
		break;

		case 0x01db:	// Sending request of the coding key
			RFIFOSKIP(fd,2);
		{
			memset(sd->md5key, '\0', sizeof(sd->md5key));
			sd->md5keylen = (uint16)(12 + rnd() % 4);
			MD5_Salt(sd->md5keylen, sd->md5key);

			WFIFOHEAD(fd,4 + sd->md5keylen);
			WFIFOW(fd,0) = 0x01dc;
			WFIFOW(fd,2) = 4 + sd->md5keylen;
			memcpy(WFIFOP(fd,4), sd->md5key, sd->md5keylen);
			WFIFOSET(fd,WFIFOW(fd,2));
		}
		break;

		case 0x2710:	// Connection request of a char-server
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			char server_name[20];
			char message[256];
			uint32 server_ip;
			uint16 server_port;
			uint16 type;
			uint16 new_;

			safestrncpy(sd->userid, (char*)RFIFOP(fd,2), NAME_LENGTH);
			safestrncpy(sd->passwd, (char*)RFIFOP(fd,26), NAME_LENGTH);
			if( login_config.use_md5_passwds )
				MD5_String(sd->passwd, sd->passwd);
			sd->passwdenc = 0;
			sd->version = login_config.client_version_to_connect; // hack to skip version check
			server_ip = ntohl(RFIFOL(fd,54));
			server_port = ntohs(RFIFOW(fd,58));
			safestrncpy(server_name, (char*)RFIFOP(fd,60), 20);
			type = RFIFOW(fd,82);
			new_ = RFIFOW(fd,84);
			RFIFOSKIP(fd,86);

			ShowInfo("Connection request of the char-server '%s' @ %u.%u.%u.%u:%u (account: '%s', pass: '%s', ip: '%s')\n", server_name, CONVIP(server_ip), server_port, sd->userid, sd->passwd, ip);
			sprintf(message, "charserver - %s@%u.%u.%u.%u:%u", server_name, CONVIP(server_ip), server_port);
			login_log(session[fd]->client_addr, sd->userid, 100, message);

			result = mmo_auth(sd, true);
			if( runflag == LOGINSERVER_ST_RUNNING &&
				result == -1 &&
				sd->sex == 'S' &&
				sd->account_id >= 0 && sd->account_id < ARRAYLENGTH(server) &&
				!session_isValid(server[sd->account_id].fd) )
			{
				ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
				safestrncpy(server[sd->account_id].name, server_name, sizeof(server[sd->account_id].name));
				server[sd->account_id].fd = fd;
				server[sd->account_id].ip = server_ip;
				server[sd->account_id].port = server_port;
				server[sd->account_id].users = 0;
				server[sd->account_id].type = type;
				server[sd->account_id].new_ = new_;

				session[fd]->func_parse = parse_fromchar;
				session[fd]->flag.server = 1;
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

				// send connection success
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2711;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd,3);
			}
			else
			{
				ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);
				WFIFOHEAD(fd,3);
				WFIFOW(fd,0) = 0x2711;
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd,3);
			}
		}
		return 0; // processing will continue elsewhere

		default:
			ShowNotice("Abnormal end of connection (ip: %s): Unknown packet 0x%x\n", ip, command);
			set_eof(fd);
			return 0;
		}
	}

	return 0;
}


void login_set_defaults()
{
	login_config.login_ip = INADDR_ANY;
	login_config.login_port = 6900;
	login_config.ipban_cleanup_interval = 60;
	login_config.ip_sync_interval = 0;
	login_config.log_login = true;
	safestrncpy(login_config.date_format, "%Y-%m-%d %H:%M:%S", sizeof(login_config.date_format));
	login_config.new_account_flag = true;
	login_config.new_acc_length_limit = true;
	login_config.use_md5_passwds = false;
	login_config.group_id_to_connect = -1;
	login_config.min_group_id_to_connect = -1;
	login_config.check_client_version = false;
	login_config.client_version_to_connect = 20;

	login_config.ipban = true;
	login_config.dynamic_pass_failure_ban = true;
	login_config.dynamic_pass_failure_ban_interval = 5;
	login_config.dynamic_pass_failure_ban_limit = 7;
	login_config.dynamic_pass_failure_ban_duration = 5;
	login_config.use_dnsbl = false;
	safestrncpy(login_config.dnsbl_servs, "", sizeof(login_config.dnsbl_servs));

	login_config.client_hash_check = 0;
	login_config.client_hash_nodes = NULL;
}

//-----------------------------------
// Reading main configuration file
//-----------------------------------
int login_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) < 2)
			continue;

		if(!strcmpi(w1,"timestamp_format"))
			safestrncpy(timestamp_format, w2, 20);
		else if(!strcmpi(w1,"stdout_with_ansisequence"))
			stdout_with_ansisequence = config_switch(w2);
		else if(!strcmpi(w1,"console_silent")) {
			msg_silent = atoi(w2);
			if( msg_silent ) /* only bother if we actually have this enabled */
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
		}
		else if( !strcmpi(w1, "bind_ip") ) {
			login_config.login_ip = host2ip(w2);
			if( login_config.login_ip ) {
				char ip_str[16];
				ShowStatus("Login server binding IP address : %s -> %s\n", w2, ip2str(login_config.login_ip, ip_str));
			}
		}
		else if( !strcmpi(w1, "login_port") ) {
			login_config.login_port = (uint16)atoi(w2);
		}
		else if(!strcmpi(w1, "log_login"))
			login_config.log_login = (bool)config_switch(w2);

		else if(!strcmpi(w1, "new_account"))
			login_config.new_account_flag = (bool)config_switch(w2);
		else if(!strcmpi(w1, "new_acc_length_limit"))
			login_config.new_acc_length_limit = (bool)config_switch(w2);
		else if(!strcmpi(w1, "start_limited_time"))
			login_config.start_limited_time = atoi(w2);
		else if(!strcmpi(w1, "check_client_version"))
			login_config.check_client_version = (bool)config_switch(w2);
		else if(!strcmpi(w1, "client_version_to_connect"))
			login_config.client_version_to_connect = strtoul(w2, NULL, 10);
		else if(!strcmpi(w1, "use_MD5_passwords"))
			login_config.use_md5_passwds = (bool)config_switch(w2);
		else if(!strcmpi(w1, "group_id_to_connect"))
			login_config.group_id_to_connect = atoi(w2);
		else if(!strcmpi(w1, "min_group_id_to_connect"))
			login_config.min_group_id_to_connect = atoi(w2);
		else if(!strcmpi(w1, "date_format"))
			safestrncpy(login_config.date_format, w2, sizeof(login_config.date_format));
		else if(!strcmpi(w1, "allowed_regs")) //account flood protection system
			allowed_regs = atoi(w2);
		else if(!strcmpi(w1, "time_allowed"))
			time_allowed = atoi(w2);
		else if(!strcmpi(w1, "use_dnsbl"))
			login_config.use_dnsbl = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dnsbl_servers"))
			safestrncpy(login_config.dnsbl_servs, w2, sizeof(login_config.dnsbl_servs));
		else if(!strcmpi(w1, "ipban_cleanup_interval"))
			login_config.ipban_cleanup_interval = (unsigned int)atoi(w2);
		else if(!strcmpi(w1, "ip_sync_interval"))
			login_config.ip_sync_interval = (unsigned int)1000*60*atoi(w2); //w2 comes in minutes.
		else if(!strcmpi(w1, "client_hash_check"))
			login_config.client_hash_check = config_switch(w2);
		else if(!strcmpi(w1, "client_hash")) {
			int group = 0;
			char md5[33];

			if (sscanf(w2, "%d, %32s", &group, md5) == 2) {
				struct client_hash_node *nnode;
				int i;
				CREATE(nnode, struct client_hash_node, 1);

				for (i = 0; i < 32; i += 2) {
					char buf[3];
					unsigned int byte;

					memcpy(buf, &md5[i], 2);
					buf[2] = 0;

					sscanf(buf, "%x", &byte);
					nnode->hash[i / 2] = (uint8)(byte & 0xFF);
				}

				nnode->group_id = group;
				nnode->next = login_config.client_hash_nodes;

				login_config.client_hash_nodes = nnode;
			}
		}
		else if(!strcmpi(w1, "import"))
			login_config_read(w2);
		else
		{
			AccountDB* db = account_engine[0].db;
			if( db )
				db->set_property(db, w1, w2);
			ipban_config_read(w1, w2);
			loginlog_config_read(w1, w2);
		}
	}
	fclose(fp);
	ShowInfo("Finished reading %s.\n", cfgName);
	return 0;
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void)
{
	int i;
	struct client_hash_node *hn = login_config.client_hash_nodes;

	ShowStatus("Terminating...\n");
	
	HPM->event(HPET_FINAL);
	
	while (hn) {
		struct client_hash_node *tmp = hn;
		hn = hn->next;
		aFree(tmp);
	}

	login_log(0, "login server", 100, "login server shutdown");

	if( login_config.log_login )
		loginlog_final();

	ipban_final();

	if( account_engine[0].db )
	{// destroy account engine
		account_engine[0].db->destroy(account_engine[0].db);
		account_engine[0].db = NULL;
	}
	accounts = NULL; // destroyed in account_engine
	online_db->destroy(online_db, NULL);
	auth_db->destroy(auth_db, NULL);
	
	for( i = 0; i < ARRAYLENGTH(server); ++i )
		chrif_server_destroy(i);

	if( login_fd != -1 )
	{
		do_close(login_fd);
		login_fd = -1;
	}

	ShowStatus("Finished.\n");
}

//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
}

void set_server_type(void) {
	SERVER_TYPE = SERVER_TYPE_LOGIN;
}


/// Called when a terminate signal is received.
void do_shutdown(void)
{
	if( runflag != LOGINSERVER_ST_SHUTDOWN )
	{
		int id;
		runflag = LOGINSERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		// TODO proper shutdown procedure; kick all characters, wait for acks, ...  [FlavioJS]
		for( id = 0; id < ARRAYLENGTH(server); ++id )
			chrif_server_reset(id);
		flush_fifos();
		runflag = CORE_ST_STOP;
	}
}


//------------------------------
// Login server initialization
//------------------------------
int do_init(int argc, char** argv)
{
	int i;

	// intialize engine (to accept config settings)
	account_engine[0].db = account_engine[0].constructor();

	// read login-server configuration
	login_set_defaults();
	login_config_read((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
	login_lan_config_read((argc > 2) ? argv[2] : LAN_CONF_NAME);

	rnd_init();
		
	for( i = 0; i < ARRAYLENGTH(server); ++i )
		chrif_server_init(i);

	// initialize logging
	if( login_config.log_login )
		loginlog_init();

	// initialize static and dynamic ipban system
	ipban_init();
	
	// Online user database init
	online_db = idb_alloc(DB_OPT_RELEASE_DATA);
	timer->add_func_list(waiting_disconnect_timer, "waiting_disconnect_timer");

	// Interserver auth init
	auth_db = idb_alloc(DB_OPT_RELEASE_DATA);

	// set default parser as parse_login function
	set_defaultparse(parse_login);

	// every 10 minutes cleanup online account db.
	timer->add_func_list(online_data_cleanup, "online_data_cleanup");
	timer->add_interval(timer->gettick() + 600*1000, online_data_cleanup, 0, 0, 600*1000);

	// add timer to detect ip address change and perform update
	if (login_config.ip_sync_interval) {
		timer->add_func_list(sync_ip_addresses, "sync_ip_addresses");
		timer->add_interval(timer->gettick() + login_config.ip_sync_interval, sync_ip_addresses, 0, 0, login_config.ip_sync_interval);
	}

	// Account database init
	accounts = account_engine[0].db;
	if( accounts == NULL ) {
		ShowFatalError("do_init: account engine 'sql' not found.\n");
		exit(EXIT_FAILURE);
	} else {

		if(!accounts->init(accounts)) {
			ShowFatalError("do_init: Failed to initialize account engine 'sql'.\n");
			exit(EXIT_FAILURE);
		}
	}

	HPM->share(account_db_sql_up(accounts),"sql_handle");
	HPM->config_read();
	HPM->event(HPET_INIT);
	
	// server port open & binding	
	if( (login_fd = make_listen_bind(login_config.login_ip,login_config.login_port)) == -1 ) {
		ShowFatalError("Failed to bind to port '"CL_WHITE"%d"CL_RESET"'\n",login_config.login_port);
		exit(EXIT_FAILURE);
	}
	
	if( runflag != CORE_ST_STOP ) {
		shutdown_callback = do_shutdown;
		runflag = LOGINSERVER_ST_RUNNING;
	}
	
	ShowStatus("The login-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %u).\n\n", login_config.login_port);
	login_log(0, "login server", 100, "login server started");
	
	HPM->event(HPET_READY);
	
	return 0;
}
