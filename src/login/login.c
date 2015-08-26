// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "login.h"

#include "login/HPMlogin.h"
#include "login/account.h"
#include "login/ipban.h"
#include "login/loginlog.h"
#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/core.h"
#include "common/db.h"
#include "common/malloc.h"
#include "common/md5calc.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>

struct login_interface login_s;
struct login_interface *login;
struct Login_Config login_config;
struct mmo_char_server server[MAX_SERVERS]; // char server data

struct Account_engine account_engine[] = {
	{account_db_sql, NULL}
};

// account database
AccountDB* accounts = NULL;

//-----------------------------------------------------
// Auth database
//-----------------------------------------------------
#define AUTH_TIMEOUT 30000

/**
 * @see DBCreateData
 */
static DBData login_create_online_user(DBKey key, va_list args)
{
	struct online_login_data* p;
	CREATE(p, struct online_login_data, 1);
	p->account_id = key.i;
	p->char_server = -1;
	p->waiting_disconnect = INVALID_TIMER;
	return DB->ptr2data(p);
}

struct online_login_data* login_add_online_user(int char_server, int account_id)
{
	struct online_login_data* p;
	p = idb_ensure(login->online_db, account_id, login->create_online_user);
	p->char_server = char_server;
	if( p->waiting_disconnect != INVALID_TIMER )
	{
		timer->delete(p->waiting_disconnect, login->waiting_disconnect_timer);
		p->waiting_disconnect = INVALID_TIMER;
	}
	return p;
}

void login_remove_online_user(int account_id)
{
	struct online_login_data* p;
	p = (struct online_login_data*)idb_get(login->online_db, account_id);
	if( p == NULL )
		return;
	if( p->waiting_disconnect != INVALID_TIMER )
		timer->delete(p->waiting_disconnect, login->waiting_disconnect_timer);

	idb_remove(login->online_db, account_id);
}

static int login_waiting_disconnect_timer(int tid, int64 tick, int id, intptr_t data) {
	struct online_login_data* p = (struct online_login_data*)idb_get(login->online_db, id);
	if( p != NULL && p->waiting_disconnect == tid && p->account_id == id )
	{
		p->waiting_disconnect = INVALID_TIMER;
		login->remove_online_user(id);
		idb_remove(login->auth_db, id);
	}
	return 0;
}

/**
 * @see DBApply
 */
static int login_online_db_setoffline(DBKey key, DBData *data, va_list ap)
{
	struct online_login_data* p = DB->data2ptr(data);
	int server_id = va_arg(ap, int);
	nullpo_ret(p);
	if( server_id == -1 )
	{
		p->char_server = -1;
		if( p->waiting_disconnect != INVALID_TIMER )
		{
			timer->delete(p->waiting_disconnect, login->waiting_disconnect_timer);
			p->waiting_disconnect = INVALID_TIMER;
		}
	}
	else if( p->char_server == server_id )
		p->char_server = -2; //Char server disconnected.
	return 0;
}

/**
 * @see DBApply
 */
static int login_online_data_cleanup_sub(DBKey key, DBData *data, va_list ap)
{
	struct online_login_data *character= DB->data2ptr(data);
	nullpo_ret(character);
	if (character->char_server == -2) //Unknown server.. set them offline
		login->remove_online_user(character->account_id);
	return 0;
}

static int login_online_data_cleanup(int tid, int64 tick, int id, intptr_t data) {
	login->online_db->foreach(login->online_db, login->online_data_cleanup_sub);
	return 0;
}


//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, uint8* buf, size_t len)
{
	int i, c;

	nullpo_ret(buf);
	for( i = 0, c = 0; i < ARRAYLENGTH(server); ++i )
	{
		int fd = server[i].fd;
		if (sockt->session_is_valid(fd) && fd != sfd) {
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
	Assert_retv(id >= 0 && id < MAX_SERVERS);
	memset(&server[id], 0, sizeof(server[id]));
	server[id].fd = -1;
}


/// Destroys a server structure.
void chrif_server_destroy(int id)
{
	Assert_retv(id >= 0 && id < MAX_SERVERS);
	if (server[id].fd != -1)
	{
		sockt->close(server[id].fd);
		server[id].fd = -1;
	}
}


/// Resets all the data related to a server.
void chrif_server_reset(int id)
{
	login->online_db->foreach(login->online_db, login->online_db_setoffline, id); //Set all chars from this char server to offline.
	chrif_server_destroy(id);
	chrif_server_init(id);
}


/// Called when the connection to Char Server is disconnected.
void chrif_on_disconnect(int id)
{
	Assert_retv(id >= 0 && id < MAX_SERVERS);
	ShowStatus("Char-server '%s' has disconnected.\n", server[id].name);
	chrif_server_reset(id);
}


//-----------------------------------------------------
// periodic ip address synchronization
//-----------------------------------------------------
static int login_sync_ip_addresses(int tid, int64 tick, int id, intptr_t data) {
	uint8 buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	charif_sendallwos(-1, buf, 2);
	return 0;
}


//-----------------------------------------------------
// encrypted/unencrypted password check (from eApp)
//-----------------------------------------------------
bool login_check_encrypted(const char* str1, const char* str2, const char* passwd)
{
	char tmpstr[64+1], md5str[32+1];

	nullpo_ret(str1);
	nullpo_ret(str2);
	nullpo_ret(passwd);
	safesnprintf(tmpstr, sizeof(tmpstr), "%s%s", str1, str2);
	MD5_String(tmpstr, md5str);

	return (0==strcmp(passwd, md5str));
}

bool login_check_password(const char* md5key, int passwdenc, const char* passwd, const char* refpass)
{
	nullpo_ret(passwd);
	nullpo_ret(refpass);
	if(passwdenc == PWENC_NONE) {
		return (0==strcmp(passwd, refpass));
	} else {
		// password mode set to PWENC_ENCRYPT  -> md5(md5key, refpass) enable with <passwordencrypt></passwordencrypt>
		// password mode set to PWENC_ENCRYPT2 -> md5(refpass, md5key) enable with <passwordencrypt2></passwordencrypt2>

		return ((passwdenc&PWENC_ENCRYPT) && login->check_encrypted(md5key, refpass, passwd)) ||
		       ((passwdenc&PWENC_ENCRYPT2) && login->check_encrypted(refpass, md5key, passwd));
	}
}


/**
 * Checks whether the given IP comes from LAN or WAN.
 *
 * @param ip IP address to check.
 * @retval 0 if it is a WAN IP.
 * @return the appropriate LAN server address to send, if it is a LAN IP.
 */
uint32 login_lan_subnet_check(uint32 ip)
{
	return sockt->lan_subnet_check(ip, NULL);
}

void login_fromchar_auth_ack(int fd, int account_id, uint32 login_id1, uint32 login_id2, uint8 sex, int request_id, struct login_auth_node* node)
{
	WFIFOHEAD(fd,33);
	WFIFOW(fd,0) = 0x2713;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = login_id1;
	WFIFOL(fd,10) = login_id2;
	WFIFOB(fd,14) = sex;
	if (node)
	{
		WFIFOB(fd,15) = 0;// ok
		WFIFOL(fd,16) = request_id;
		WFIFOL(fd,20) = node->version;
		WFIFOB(fd,24) = node->clienttype;
		WFIFOL(fd,25) = node->group_id;
		WFIFOL(fd,29) = (unsigned int)node->expiration_time;
	}
	else
	{
		WFIFOB(fd,15) = 1;// auth failed
		WFIFOL(fd,16) = request_id;
		WFIFOL(fd,20) = 0;
		WFIFOB(fd,24) = 0;
		WFIFOL(fd,25) = 0;
		WFIFOL(fd,29) = 0;
	}
	WFIFOSET(fd,33);
}

void login_fromchar_parse_auth(int fd, int id, const char *const ip)
{
	struct login_auth_node* node;

	int account_id = RFIFOL(fd,2);
	uint32 login_id1 = RFIFOL(fd,6);
	uint32 login_id2 = RFIFOL(fd,10);
	uint8 sex = RFIFOB(fd,14);
	//uint32 ip_ = ntohl(RFIFOL(fd,15));
	int request_id = RFIFOL(fd,19);
	RFIFOSKIP(fd,23);

	node = (struct login_auth_node*)idb_get(login->auth_db, account_id);
	if( core->runflag == LOGINSERVER_ST_RUNNING &&
		node != NULL &&
		node->account_id == account_id &&
		node->login_id1  == login_id1 &&
		node->login_id2  == login_id2 &&
		node->sex        == sex_num2str(sex) /*&&
		node->ip         == ip_*/ )
	{// found
		//ShowStatus("Char-server '%s': authentication of the account %d accepted (ip: %s).\n", server[id].name, account_id, ip);

		// send ack
		login->fromchar_auth_ack(fd, account_id, login_id1, login_id2, sex, request_id, node);
		// each auth entry can only be used once
		idb_remove(login->auth_db, account_id);
	}
	else
	{// authentication not found
		nullpo_retv(ip);
		ShowStatus("Char-server '%s': authentication of the account %d REFUSED (ip: %s).\n", server[id].name, account_id, ip);
		login->fromchar_auth_ack(fd, account_id, login_id1, login_id2, sex, request_id, NULL);
	}
}

void login_fromchar_parse_update_users(int fd, int id)
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

void login_fromchar_parse_request_change_email(int fd, int id, const char *const ip)
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
		memcpy(acc.email, email, sizeof(acc.email));
		ShowNotice("Char-server '%s': Create an e-mail on an account with a default e-mail (account: %d, new e-mail: %s, ip: %s).\n", server[id].name, account_id, email, ip);
		// Save
		accounts->save(accounts, &acc);
	}
}

void login_fromchar_account(int fd, int account_id, struct mmo_account *acc)
{
	WFIFOHEAD(fd,72);
	WFIFOW(fd,0) = 0x2717;
	WFIFOL(fd,2) = account_id;
	if (acc)
	{
		time_t expiration_time = 0;
		char email[40] = "";
		int group_id = 0;
		uint8 char_slots = 0;
		char birthdate[10+1] = "";
		char pincode[4+1] = "\0\0\0\0";

		safestrncpy(email, acc->email, sizeof(email));
		expiration_time = acc->expiration_time;
		group_id = acc->group_id;
		char_slots = acc->char_slots;
		safestrncpy(pincode, acc->pincode, sizeof(pincode));
		safestrncpy(birthdate, acc->birthdate, sizeof(birthdate));
		if (pincode[0] == '\0')
			memset(pincode,'\0',sizeof(pincode));

		safestrncpy((char*)WFIFOP(fd,6), email, 40);
		WFIFOL(fd,46) = (uint32)expiration_time;
		WFIFOB(fd,50) = (unsigned char)group_id;
		WFIFOB(fd,51) = char_slots;
		safestrncpy((char*)WFIFOP(fd,52), birthdate, 10+1);
		safestrncpy((char*)WFIFOP(fd,63), pincode, 4+1 );
		WFIFOL(fd,68) = acc->pincode_change;
	}
	else
	{
		safestrncpy((char*)WFIFOP(fd,6), "", 40);
		WFIFOL(fd,46) = 0;
		WFIFOB(fd,50) = 0;
		WFIFOB(fd,51) = 0;
		safestrncpy((char*)WFIFOP(fd,52), "", 10+1);
		safestrncpy((char*)WFIFOP(fd,63), "\0\0\0\0", 4+1 );
		WFIFOL(fd,68) = 0;
	}
	WFIFOSET(fd,72);
}

void login_fromchar_parse_account_data(int fd, int id, const char *const ip)
{
	struct mmo_account acc;

	int account_id = RFIFOL(fd,2);
	RFIFOSKIP(fd,6);

	if( !accounts->load_num(accounts, &acc, account_id) )
	{
		ShowNotice("Char-server '%s': account %d NOT found (ip: %s).\n", server[id].name, account_id, ip);
		login->fromchar_account(fd, account_id, NULL);
	}
	else {
		login->fromchar_account(fd, account_id, &acc);
	}
}

void login_fromchar_pong(int fd)
{
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2718;
	WFIFOSET(fd,2);
}

void login_fromchar_parse_ping(int fd)
{
	RFIFOSKIP(fd,2);
	login->fromchar_pong(fd);
}

void login_fromchar_parse_change_email(int fd, int id, const char *const ip)
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
		safestrncpy(acc.email, new_email, sizeof(acc.email));
		ShowNotice("Char-server '%s': Modify an e-mail on an account (@email GM command) (account: %d (%s), new e-mail: %s, ip: %s).\n", server[id].name, account_id, acc.userid, new_email, ip);
		// Save
		accounts->save(accounts, &acc);
	}
}

void login_fromchar_account_update_other(int account_id, unsigned int state)
{
	uint8 buf[11];
	WBUFW(buf,0) = 0x2731;
	WBUFL(buf,2) = account_id;
	WBUFB(buf,6) = 0; // 0: change of state, 1: ban
	WBUFL(buf,7) = state; // status or final date of a banishment
	charif_sendallwos(-1, buf, 11);
}

void login_fromchar_parse_account_update(int fd, int id, const char *const ip)
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
			login->fromchar_account_update_other(account_id, state);
		}
	}
}

void login_fromchar_ban(int account_id, time_t timestamp)
{
	uint8 buf[11];
	WBUFW(buf,0) = 0x2731;
	WBUFL(buf,2) = account_id;
	WBUFB(buf,6) = 1; // 0: change of status, 1: ban
	WBUFL(buf,7) = (uint32)timestamp; // status or final date of a banishment
	charif_sendallwos(-1, buf, 11);
}

void login_fromchar_parse_ban(int fd, int id, const char *const ip)
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

	if (!accounts->load_num(accounts, &acc, account_id)) {
		ShowNotice("Char-server '%s': Error of ban request (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
	} else {
		time_t timestamp;
		struct tm *tmtime;
		if (acc.unban_time == 0 || acc.unban_time < time(NULL))
			timestamp = time(NULL); // new ban
		else
			timestamp = acc.unban_time; // add to existing ban
		tmtime = localtime(&timestamp);
		tmtime->tm_year += year;
		tmtime->tm_mon  += month;
		tmtime->tm_mday += mday;
		tmtime->tm_hour += hour;
		tmtime->tm_min  += min;
		tmtime->tm_sec  += sec;
		timestamp = mktime(tmtime);
		if (timestamp == -1) {
			ShowNotice("Char-server '%s': Error of ban request (account: %d, invalid date, ip: %s).\n", server[id].name, account_id, ip);
		} else if( timestamp <= time(NULL) || timestamp == 0 ) {
			ShowNotice("Char-server '%s': Error of ban request (account: %d, new date unbans the account, ip: %s).\n", server[id].name, account_id, ip);
		} else {
			char tmpstr[24];
			timestamp2string(tmpstr, sizeof(tmpstr), timestamp, login_config.date_format);
			ShowNotice("Char-server '%s': Ban request (account: %d, new final date of banishment: %ld (%s), ip: %s).\n",
			           server[id].name, account_id, (long)timestamp, tmpstr, ip);

			acc.unban_time = timestamp;

			// Save
			accounts->save(accounts, &acc);

			login->fromchar_ban(account_id, timestamp);
		}
	}
}

void login_fromchar_change_sex_other(int account_id, char sex)
{
	unsigned char buf[7];
	WBUFW(buf,0) = 0x2723;
	WBUFL(buf,2) = account_id;
	WBUFB(buf,6) = sex_str2num(sex);
	charif_sendallwos(-1, buf, 7);
}

void login_fromchar_parse_change_sex(int fd, int id, const char *const ip)
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
		char sex = ( acc.sex == 'M' ) ? 'F' : 'M'; //Change gender

		ShowNotice("Char-server '%s': Sex change (account: %d, new sex %c, ip: %s).\n", server[id].name, account_id, sex, ip);

		acc.sex = sex;
		// Save
		accounts->save(accounts, &acc);

		// announce to other servers
		login->fromchar_change_sex_other(account_id, sex);
	}
}

void login_fromchar_parse_account_reg2(int fd, int id, const char *const ip)
{
	struct mmo_account acc;

	int account_id = RFIFOL(fd,4);

	if( !accounts->load_num(accounts, &acc, account_id) )
		ShowStatus("Char-server '%s': receiving (from the char-server) of account_reg2 (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
	else {
		mmo_save_accreg2(accounts,fd,account_id,RFIFOL(fd, 8));
	}
	RFIFOSKIP(fd,RFIFOW(fd,2));
}

void login_fromchar_parse_unban(int fd, int id, const char *const ip)
{
	struct mmo_account acc;

	int account_id = RFIFOL(fd,2);
	RFIFOSKIP(fd,6);

	if( !accounts->load_num(accounts, &acc, account_id) )
		ShowNotice("Char-server '%s': Error of Unban request (account: %d not found, ip: %s).\n", server[id].name, account_id, ip);
	else
	if( acc.unban_time == 0 )
		ShowNotice("Char-server '%s': Error of Unban request (account: %d, no change for unban date, ip: %s).\n", server[id].name, account_id, ip);
	else
	{
		ShowNotice("Char-server '%s': Unban request (account: %d, ip: %s).\n", server[id].name, account_id, ip);
		acc.unban_time = 0;
		accounts->save(accounts, &acc);
	}
}

void login_fromchar_parse_account_online(int fd, int id)
{
	login->add_online_user(id, RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
}

void login_fromchar_parse_account_offline(int fd)
{
	login->remove_online_user(RFIFOL(fd,2));
	RFIFOSKIP(fd,6);
}

void login_fromchar_parse_online_accounts(int fd, int id)
{
	uint32 i, users;
	login->online_db->foreach(login->online_db, login->online_db_setoffline, id); //Set all chars from this char-server offline first
	users = RFIFOW(fd,4);
	for (i = 0; i < users; i++) {
		int aid = RFIFOL(fd,6+i*4);
		struct online_login_data *p = idb_ensure(login->online_db, aid, login->create_online_user);
		p->char_server = id;
		if (p->waiting_disconnect != INVALID_TIMER)
		{
			timer->delete(p->waiting_disconnect, login->waiting_disconnect_timer);
			p->waiting_disconnect = INVALID_TIMER;
		}
	}
}

void login_fromchar_parse_request_account_reg2(int fd)
{
	int account_id = RFIFOL(fd,2);
	int char_id = RFIFOL(fd,6);
	RFIFOSKIP(fd,10);

	mmo_send_accreg2(accounts,fd,account_id,char_id);
}

void login_fromchar_parse_update_wan_ip(int fd, int id)
{
	server[id].ip = ntohl(RFIFOL(fd,2));
	ShowInfo("Updated IP of Server #%d to %d.%d.%d.%d.\n",id, CONVIP(server[id].ip));
	RFIFOSKIP(fd,6);
}

void login_fromchar_parse_all_offline(int fd, int id)
{
	ShowInfo("Setting accounts from char-server %d offline.\n", id);
	login->online_db->foreach(login->online_db, login->online_db_setoffline, id);
	RFIFOSKIP(fd,2);
}

void login_fromchar_parse_change_pincode(int fd)
{
	struct mmo_account acc;

	if (accounts->load_num(accounts, &acc, RFIFOL(fd,2))) {
		safestrncpy(acc.pincode, (char*)RFIFOP(fd,6), sizeof(acc.pincode));
		acc.pincode_change = ((unsigned int)time(NULL));
		accounts->save(accounts, &acc);
	}
	RFIFOSKIP(fd,11);
}

bool login_fromchar_parse_wrong_pincode(int fd)
{
	struct mmo_account acc;

	if( accounts->load_num(accounts, &acc, RFIFOL(fd,2) ) ) {
		struct online_login_data* ld = (struct online_login_data*)idb_get(login->online_db,acc.account_id);

		if (ld == NULL) {
			RFIFOSKIP(fd,6);
			return true;
		}

		login_log(sockt->host2ip(acc.last_ip), acc.userid, 100, "PIN Code check failed"); // FIXME: Do we really want to log this with the same code as successful logins?
	}

	login->remove_online_user(acc.account_id);
	RFIFOSKIP(fd,6);
	return false;
}

void login_fromchar_accinfo(int fd, int account_id, int u_fd, int u_aid, int u_group, int map_fd, struct mmo_account *acc)
{
	if (acc)
	{
		WFIFOHEAD(fd,183);
		WFIFOW(fd,0) = 0x2737;
		safestrncpy((char*)WFIFOP(fd,2), acc->userid, NAME_LENGTH);
		if (u_group >= acc->group_id)
			safestrncpy((char*)WFIFOP(fd,26), acc->pass, 33);
		else
			memset(WFIFOP(fd,26), '\0', 33);
		safestrncpy((char*)WFIFOP(fd,59), acc->email, 40);
		safestrncpy((char*)WFIFOP(fd,99), acc->last_ip, 16);
		WFIFOL(fd,115) = acc->group_id;
		safestrncpy((char*)WFIFOP(fd,119), acc->lastlogin, 24);
		WFIFOL(fd,143) = acc->logincount;
		WFIFOL(fd,147) = acc->state;
		if (u_group >= acc->group_id)
			safestrncpy((char*)WFIFOP(fd,151), acc->pincode, 5);
		else
			memset(WFIFOP(fd,151), '\0', 5);
		safestrncpy((char*)WFIFOP(fd,156), acc->birthdate, 11);
		WFIFOL(fd,167) = map_fd;
		WFIFOL(fd,171) = u_fd;
		WFIFOL(fd,175) = u_aid;
		WFIFOL(fd,179) = account_id;
		WFIFOSET(fd,183);
	}
	else
	{
		WFIFOHEAD(fd,18);
		WFIFOW(fd,0) = 0x2736;
		WFIFOL(fd,2) = map_fd;
		WFIFOL(fd,6) = u_fd;
		WFIFOL(fd,10) = u_aid;
		WFIFOL(fd,14) = account_id;
		WFIFOSET(fd,18);
	}
}

void login_fromchar_parse_accinfo(int fd)
{
	struct mmo_account acc;
	int account_id = RFIFOL(fd, 2), u_fd = RFIFOL(fd, 6), u_aid = RFIFOL(fd, 10), u_group = RFIFOL(fd, 14), map_fd = RFIFOL(fd, 18);
	if (accounts->load_num(accounts, &acc, account_id)) {
		login->fromchar_accinfo(fd, account_id, u_fd, u_aid, u_group, map_fd, &acc);
	} else {
		login->fromchar_accinfo(fd, account_id, u_fd, u_aid, u_group, map_fd, NULL);
	}
	RFIFOSKIP(fd,22);
}

//--------------------------------
// Packet parsing for char-servers
//--------------------------------
int login_parse_fromchar(int fd)
{
	int j, id;
	uint32 ipl;
	char ip[16];

	ARR_FIND( 0, ARRAYLENGTH(server), id, server[id].fd == fd );
	if( id == ARRAYLENGTH(server) )
	{// not a char server
		ShowDebug("login_parse_fromchar: Disconnecting invalid session #%d (is not a char-server)\n", fd);
		sockt->eof(fd);
		sockt->close(fd);
		return 0;
	}

	if( sockt->session[fd]->flag.eof )
	{
		sockt->close(fd);
		server[id].fd = -1;
		chrif_on_disconnect(id);
		return 0;
	}

	ipl = server[id].ip;
	sockt->ip2str(ipl, ip);

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
			login->fromchar_parse_auth(fd, id, ip);
		}
		break;

		case 0x2714:
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			login->fromchar_parse_update_users(fd, id);
		}
		break;

		case 0x2715: // request from char server to change e-email from default "a@a.com"
			if (RFIFOREST(fd) < 46)
				return 0;
		{
			login->fromchar_parse_request_change_email(fd, id, ip);
		}
		break;

		case 0x2716: // request account data
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			login->fromchar_parse_account_data(fd, id, ip);
		}
		break;

		case 0x2719: // ping request from charserver
			login->fromchar_parse_ping(fd);
		break;

		// Map server send information to change an email of an account via char-server
		case 0x2722: // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			login->fromchar_parse_change_email(fd, id, ip);
		}
		break;

		case 0x2724: // Receiving an account state update request from a map-server (relayed via char-server)
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			login->fromchar_parse_account_update(fd, id, ip);
		}
		break;

		case 0x2725: // Receiving of map-server via char-server a ban request
			if (RFIFOREST(fd) < 18)
				return 0;
		{
			login->fromchar_parse_ban(fd, id, ip);
		}
		break;

		case 0x2727: // Change of sex (sex is reversed)
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			login->fromchar_parse_change_sex(fd, id, ip);
		}
		break;

		case 0x2728: // We receive account_reg2 from a char-server, and we send them to other map-servers.
			if( RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2) )
				return 0;
		{
			login->fromchar_parse_account_reg2(fd, id, ip);
		}
		break;

		case 0x272a: // Receiving of map-server via char-server an unban request
			if( RFIFOREST(fd) < 6 )
				return 0;
		{
			login->fromchar_parse_unban(fd, id, ip);
		}
		break;

		case 0x272b:    // Set account_id to online [Wizputer]
			if( RFIFOREST(fd) < 6 )
				return 0;
			login->fromchar_parse_account_online(fd, id);
		break;

		case 0x272c:   // Set account_id to offline [Wizputer]
			if( RFIFOREST(fd) < 6 )
				return 0;
			login->fromchar_parse_account_offline(fd);
		break;

		case 0x272d: // Receive list of all online accounts. [Skotlex]
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			{
				login->fromchar_parse_online_accounts(fd, id);
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
		break;

		case 0x272e: //Request account_reg2 for a character.
			if (RFIFOREST(fd) < 10)
				return 0;
		{
			login->fromchar_parse_request_account_reg2(fd);
		}
		break;

		case 0x2736: // WAN IP update from char-server
			if( RFIFOREST(fd) < 6 )
				return 0;
			login->fromchar_parse_update_wan_ip(fd, id);
		break;

		case 0x2737: //Request to set all offline.
			login->fromchar_parse_all_offline(fd, id);
		break;

		case 0x2738: //Change PIN Code for a account
			if( RFIFOREST(fd) < 11 )
				return 0;
			else {
				login->fromchar_parse_change_pincode(fd);
			}
			break;

		case 0x2739: // PIN Code was entered wrong too often
			if( RFIFOREST(fd) < 6 )
				return 0;
			else {
				if (login->fromchar_parse_wrong_pincode(fd))
					return 0;
			}
		break;

		case 0x2740: // Accinfo request forwarded by charserver on mapserver's account
			if( RFIFOREST(fd) < 22 )
				return 0;
			else {
				login->fromchar_parse_accinfo(fd);
			}
		break;
		default:
			ShowError("login_parse_fromchar: Unknown packet 0x%x from a char-server! Disconnecting!\n", command);
			sockt->eof(fd);
			return 0;
		} // switch
	} // while

	return 0;
}


//-------------------------------------
// Make new account
//-------------------------------------
int login_mmo_auth_new(const char* userid, const char* pass, const char sex, const char* last_ip) {
	static int num_regs = 0; // registration counter
	static int64 new_reg_tick = 0;
	int64 tick = timer->gettick();
	struct mmo_account acc;

	nullpo_retr(3, userid);
	nullpo_retr(3, pass);
	nullpo_retr(3, last_ip);
	//Account Registration Flood Protection by [Kevin]
	if( new_reg_tick == 0 )
		new_reg_tick = timer->gettick();
	if( DIFF_TICK(tick, new_reg_tick) < 0 && num_regs >= login_config.allowed_regs ) {
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
		ShowNotice("Attempt of creation of an already existing account (account: %s_%c, pass: %s, received pass: %s)\n", userid, sex, acc.pass, pass);
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
		new_reg_tick = tick + login_config.time_allowed*1000;
	}
	++num_regs;

	return -1;
}

//-----------------------------------------------------
// Check/authentication of a connection
//-----------------------------------------------------
// TODO: Map result values to an enum (or at least document them)
int login_mmo_auth(struct login_session_data* sd, bool isServer) {
	struct mmo_account acc;
	size_t len;

	char ip[16];
	nullpo_ret(sd);
	sockt->ip2str(sockt->session[sd->fd]->client_addr, ip);

	// DNS Blacklist check
	if( login_config.use_dnsbl ) {
		char r_ip[16];
		char ip_dnsbl[256];
		char* dnsbl_serv;
		uint8* sin_addr = (uint8*)&sockt->session[sd->fd]->client_addr;

		sprintf(r_ip, "%u.%u.%u.%u", sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3]);

		for( dnsbl_serv = strtok(login_config.dnsbl_servs,","); dnsbl_serv != NULL; dnsbl_serv = strtok(NULL,",") ) {
			sprintf(ip_dnsbl, "%s.%s", r_ip, trim(dnsbl_serv));
			if (sockt->host2ip(ip_dnsbl)) {
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
		if (len > 2 && sd->passwd[0] != '\0' && // valid user and password lengths
			sd->passwdenc == PWENC_NONE && // unencoded password
			sd->userid[len-2] == '_' && memchr("FfMm", sd->userid[len-1], 4)) // _M/_F suffix
		{
			int result;

			// remove the _M/_F suffix
			len -= 2;
			sd->userid[len] = '\0';

			result = login->mmo_auth_new(sd->userid, sd->passwd, TOUPPER(sd->userid[len+1]), ip);
			if( result != -1 )
				return result;// Failed to make account. [Skotlex].
		}
	}

	if( len <= 0 ) { /** a empty password is fine, a userid is not. **/
		ShowNotice("Empty userid (received pass: '%s', ip: %s)\n", sd->passwd, ip);
		return 0; // 0 = Unregistered ID
	}

	if( !accounts->load_str(accounts, &acc, sd->userid) ) {
		ShowNotice("Unknown account (account: %s, received pass: %s, ip: %s)\n", sd->userid, sd->passwd, ip);
		return 0; // 0 = Unregistered ID
	}

	if( !login->check_password(sd->md5key, sd->passwdenc, sd->passwd, acc.pass) ) {
		ShowNotice("Invalid password (account: '%s', pass: '%s', received pass: '%s', ip: %s)\n", sd->userid, acc.pass, sd->passwd, ip);
		return 1; // 1 = Incorrect Password
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
		struct client_hash_node *node = NULL;
		bool match = false;

		for( node = login_config.client_hash_nodes; node; node = node->next ) {
			if( acc.group_id < node->group_id )
				continue;
			if( *node->hash == '\0' // Allowed to login without hash
			 || (sd->has_client_hash && memcmp(node->hash, sd->client_hash, 16) == 0 ) // Correct hash
			) {
				match = true;
				break;
			}
		}

		if( !match ) {
			char smd5[33];
			int i;

			if( !sd->has_client_hash ) {
				ShowNotice("Client didn't send client hash (account: %s, pass: %s, ip: %s)\n", sd->userid, sd->passwd, ip);
				return 5;
			}

			for( i = 0; i < 16; i++ )
				sprintf(&smd5[i * 2], "%02x", sd->client_hash[i]);
			smd5[32] = '\0';

			ShowNotice("Invalid client hash (account: %s, pass: %s, sent md5: %s, ip: %s)\n", sd->userid, sd->passwd, smd5, ip);
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
	sd->expiration_time = acc.expiration_time;

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

void login_connection_problem(int fd, uint8 status)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x81;
	WFIFOB(fd,2) = status;
	WFIFOSET(fd,3);
}

void login_kick(struct login_session_data* sd)
{
	uint8 buf[6];
	nullpo_retv(sd);
	WBUFW(buf,0) = 0x2734;
	WBUFL(buf,2) = sd->account_id;
	charif_sendallwos(-1, buf, 6);
}

void login_auth_ok(struct login_session_data* sd)
{
	int fd = 0;
	uint32 ip;
	uint8 server_num, n;
	struct login_auth_node* node;
	int i;

	nullpo_retv(sd);
	fd = sd->fd;
	ip = sockt->session[fd]->client_addr;
	if( core->runflag != LOGINSERVER_ST_RUNNING )
	{
		// players can only login while running
		login->connection_problem(fd, 1); // 01 = server closed
		return;
	}

	if( login_config.group_id_to_connect >= 0 && sd->group_id != login_config.group_id_to_connect ) {
		ShowStatus("Connection refused: the required group id for connection is %d (account: %s, group: %d).\n", login_config.group_id_to_connect, sd->userid, sd->group_id);
		login->connection_problem(fd, 1); // 01 = server closed
		return;
	} else if( login_config.min_group_id_to_connect >= 0 && login_config.group_id_to_connect == -1 && sd->group_id < login_config.min_group_id_to_connect ) {
		ShowStatus("Connection refused: the minimum group id required for connection is %d (account: %s, group: %d).\n", login_config.min_group_id_to_connect, sd->userid, sd->group_id);
		login->connection_problem(fd, 1); // 01 = server closed
		return;
	}

	server_num = 0;
	for( i = 0; i < ARRAYLENGTH(server); ++i )
		if (sockt->session_is_active(server[i].fd))
			server_num++;

	if( server_num == 0 )
	{// if no char-server, don't send void list of servers, just disconnect the player with proper message
		ShowStatus("Connection refused: there is no char-server online (account: %s).\n", sd->userid);
		login->connection_problem(fd, 1); // 01 = server closed
		return;
	}

	{
		struct online_login_data* data = (struct online_login_data*)idb_get(login->online_db, sd->account_id);
		if( data )
		{// account is already marked as online!
			if( data->char_server > -1 )
			{// Request char servers to kick this account out. [Skotlex]
				ShowNotice("User '%s' is already online - Rejected.\n", sd->userid);
				login->kick(sd);
				if( data->waiting_disconnect == INVALID_TIMER )
					data->waiting_disconnect = timer->add(timer->gettick()+AUTH_TIMEOUT, login->waiting_disconnect_timer, sd->account_id, 0);

				login->connection_problem(fd, 8); // 08 = Server still recognizes your last login
				return;
			}
			else
			if( data->char_server == -1 )
			{// client has authed but did not access char-server yet
				// wipe previous session
				idb_remove(login->auth_db, sd->account_id);
				login->remove_online_user(sd->account_id);
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
	for (i = 0, n = 0; i < ARRAYLENGTH(server); ++i) {
		uint32 subnet_char_ip;

		if (!sockt->session_is_valid(server[i].fd))
			continue;

		subnet_char_ip = login->lan_subnet_check(ip);
		WFIFOL(fd,47+n*32) = htonl((subnet_char_ip) ? subnet_char_ip : server[i].ip);
		WFIFOW(fd,47+n*32+4) = sockt->ntows(htons(server[i].port)); // [!] LE byte order here [!]
		memcpy(WFIFOP(fd,47+n*32+6), server[i].name, 20);
		WFIFOW(fd,47+n*32+26) = server[i].users;

		if( server[i].type == CST_PAYING && sd->expiration_time > time(NULL) )
			WFIFOW(fd,47+n*32+28) = CST_NORMAL;
		else
			WFIFOW(fd,47+n*32+28) = server[i].type;

		WFIFOW(fd,47+n*32+30) = server[i].new_;
		n++;
	}
	WFIFOSET(fd,47+32*server_num);

	// create temporary auth entry
	CREATE(node, struct login_auth_node, 1);
	node->account_id = sd->account_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->sex;
	node->ip = ip;
	node->version = sd->version;
	node->clienttype = sd->clienttype;
	node->group_id = sd->group_id;
	node->expiration_time = sd->expiration_time;
	idb_put(login->auth_db, sd->account_id, node);

	{
		struct online_login_data* data;

		// mark client as 'online'
		data = login->add_online_user(-1, sd->account_id);

		// schedule deletion of this node
		data->waiting_disconnect = timer->add(timer->gettick()+AUTH_TIMEOUT, login->waiting_disconnect_timer, sd->account_id, 0);
	}
}

void login_auth_failed(struct login_session_data* sd, int result)
{
	int fd;
	uint32 ip;
	nullpo_retv(sd);

	fd = sd->fd;
	ip = sockt->session[fd]->client_addr;
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

		login_log(ip, sd->userid, result, error); // FIXME: result can be 100, conflicting with the value 100 we use for successful login...
	}

	if (result == 1 && login_config.dynamic_pass_failure_ban && !sockt->trusted_ip_check(ip))
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

void login_login_error(int fd, uint8 status)
{
	WFIFOHEAD(fd,23);
	WFIFOW(fd,0) = 0x6a;
	WFIFOB(fd,2) = status;
	WFIFOSET(fd,23);
}

void login_parse_ping(int fd, struct login_session_data* sd) __attribute__((nonnull (2)));
void login_parse_ping(int fd, struct login_session_data* sd)
{
	RFIFOSKIP(fd,26);
}

void login_parse_client_md5(int fd, struct login_session_data* sd) __attribute__((nonnull (2)));
void login_parse_client_md5(int fd, struct login_session_data* sd)
{
	sd->has_client_hash = 1;
	memcpy(sd->client_hash, RFIFOP(fd, 2), 16);

	RFIFOSKIP(fd,18);
}

bool login_parse_client_login(int fd, struct login_session_data* sd, const char *const ip) __attribute__((nonnull (2)));
bool login_parse_client_login(int fd, struct login_session_data* sd, const char *const ip)
{
	uint32 version;
	char username[NAME_LENGTH];
	char password[PASSWD_LEN];
	unsigned char passhash[16];
	uint8 clienttype;
	int result;
	uint16 command = RFIFOW(fd,0);
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
			login->auth_failed(sd, 3);
			return true;
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
		sd->passwdenc = PWENC_NONE;
	}
	else
	{
		ShowStatus("Request for connection (passwdenc mode) of %s (ip: %s).\n", sd->userid, ip);
		bin2hex(sd->passwd, passhash, 16); // raw binary data here!
		sd->passwdenc = PASSWORDENC;
	}

	if (sd->passwdenc != PWENC_NONE && login_config.use_md5_passwds) {
		login->auth_failed(sd, 3); // send "rejected from server"
		return true;
	}

	result = login->mmo_auth(sd, false);

	if( result == -1 )
		login->auth_ok(sd);
	else
		login->auth_failed(sd, result);
	return false;
}

void login_send_coding_key(int fd, struct login_session_data* sd) __attribute__((nonnull (2)));
void login_send_coding_key(int fd, struct login_session_data* sd)
{
	WFIFOHEAD(fd,4 + sd->md5keylen);
	WFIFOW(fd,0) = 0x01dc;
	WFIFOW(fd,2) = 4 + sd->md5keylen;
	memcpy(WFIFOP(fd,4), sd->md5key, sd->md5keylen);
	WFIFOSET(fd,WFIFOW(fd,2));
}

void login_parse_request_coding_key(int fd, struct login_session_data* sd) __attribute__((nonnull (2)));
void login_parse_request_coding_key(int fd, struct login_session_data* sd)
{
	memset(sd->md5key, '\0', sizeof(sd->md5key));
	sd->md5keylen = (uint16)(12 + rnd() % 4);
	MD5_Salt(sd->md5keylen, sd->md5key);

	login->send_coding_key(fd, sd);
}

void login_char_server_connection_status(int fd, struct login_session_data* sd, uint8 status) __attribute__((nonnull (2)));
void login_char_server_connection_status(int fd, struct login_session_data* sd, uint8 status)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x2711;
	WFIFOB(fd,2) = status;
	WFIFOSET(fd,3);
}

void login_parse_request_connection(int fd, struct login_session_data* sd, const char *const ip, uint32 ipl) __attribute__((nonnull (2, 3)));
void login_parse_request_connection(int fd, struct login_session_data* sd, const char *const ip, uint32 ipl)
{
	char server_name[20];
	char message[256];
	uint32 server_ip;
	uint16 server_port;
	uint16 type;
	uint16 new_;
	int result;

	safestrncpy(sd->userid, (char*)RFIFOP(fd,2), NAME_LENGTH);
	safestrncpy(sd->passwd, (char*)RFIFOP(fd,26), NAME_LENGTH);
	if( login_config.use_md5_passwds )
		MD5_String(sd->passwd, sd->passwd);
	sd->passwdenc = PWENC_NONE;
	sd->version = login_config.client_version_to_connect; // hack to skip version check
	server_ip = ntohl(RFIFOL(fd,54));
	server_port = ntohs(RFIFOW(fd,58));
	safestrncpy(server_name, (char*)RFIFOP(fd,60), 20);
	type = RFIFOW(fd,82);
	new_ = RFIFOW(fd,84);
	RFIFOSKIP(fd,86);

	ShowInfo("Connection request of the char-server '%s' @ %u.%u.%u.%u:%u (account: '%s', pass: '%s', ip: '%s')\n", server_name, CONVIP(server_ip), server_port, sd->userid, sd->passwd, ip);
	sprintf(message, "charserver - %s@%u.%u.%u.%u:%u", server_name, CONVIP(server_ip), server_port);
	login_log(sockt->session[fd]->client_addr, sd->userid, 100, message);

	result = login->mmo_auth(sd, true);
	if (core->runflag == LOGINSERVER_ST_RUNNING &&
		result == -1 &&
		sd->sex == 'S' &&
		sd->account_id >= 0 &&
		sd->account_id < ARRAYLENGTH(server) &&
		!sockt->session_is_valid(server[sd->account_id].fd) &&
		sockt->allowed_ip_check(ipl))
	{
		ShowStatus("Connection of the char-server '%s' accepted.\n", server_name);
		safestrncpy(server[sd->account_id].name, server_name, sizeof(server[sd->account_id].name));
		server[sd->account_id].fd = fd;
		server[sd->account_id].ip = server_ip;
		server[sd->account_id].port = server_port;
		server[sd->account_id].users = 0;
		server[sd->account_id].type = type;
		server[sd->account_id].new_ = new_;

		sockt->session[fd]->func_parse = login->parse_fromchar;
		sockt->session[fd]->flag.server = 1;
		sockt->realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

		// send connection success
		login->char_server_connection_status(fd, sd, 0);
	}
	else
	{
		ShowNotice("Connection of the char-server '%s' REFUSED.\n", server_name);
		login->char_server_connection_status(fd, sd, 3);
	}
}

//----------------------------------------------------------------------------------------
// Default packet parsing (normal players or char-server connection requests)
//----------------------------------------------------------------------------------------
int login_parse_login(int fd)
{
	struct login_session_data* sd = (struct login_session_data*)sockt->session[fd]->session_data;
	int result;

	char ip[16];
	uint32 ipl = sockt->session[fd]->client_addr;
	sockt->ip2str(ipl, ip);

	if( sockt->session[fd]->flag.eof )
	{
		ShowInfo("Closed connection from '"CL_WHITE"%s"CL_RESET"'.\n", ip);
		sockt->close(fd);
		return 0;
	}

	if( sd == NULL )
	{
		// Perform ip-ban check
		if (login_config.ipban && !sockt->trusted_ip_check(ipl) && ipban_check(ipl))
		{
			ShowStatus("Connection refused: IP isn't authorized (deny/allow, ip: %s).\n", ip);
			login_log(ipl, "unknown", -3, "ip banned");
			login->login_error(fd, 3); // 3 = Rejected from Server
			sockt->eof(fd);
			return 0;
		}

		// create a session for this new connection
		CREATE(sockt->session[fd]->session_data, struct login_session_data, 1);
		sd = (struct login_session_data*)sockt->session[fd]->session_data;
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

		case 0x0200: // New alive packet: structure: 0x200 <account.userid>.24B. used to verify if client is always alive.
			if (RFIFOREST(fd) < 26)
				return 0;
			login->parse_ping(fd, sd);
		break;

		// client md5 hash (binary)
		case 0x0204: // S 0204 <md5 hash>.16B (kRO 2004-05-31aSakexe langtype 0 and 6)
			if (RFIFOREST(fd) < 18)
				return 0;

			login->parse_client_md5(fd, sd);
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
			if (login->parse_client_login(fd, sd, ip))
				return 0;
		}
		break;

		case 0x01db: // Sending request of the coding key
			RFIFOSKIP(fd,2);
		{
			login->parse_request_coding_key(fd, sd);
		}
		break;

		case 0x2710: // Connection request of a char-server
			if (RFIFOREST(fd) < 86)
				return 0;
		{
			login->parse_request_connection(fd, sd, ip, ipl);
		}
		return 0; // processing will continue elsewhere

		default:
			ShowNotice("Abnormal end of connection (ip: %s): Unknown packet 0x%x\n", ip, command);
			sockt->eof(fd);
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
	login_config.allowed_regs = 1;
	login_config.time_allowed = 10;

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
	FILE* fp;
	nullpo_retr(1, cfgName);
	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) < 2)
			continue;

		if(!strcmpi(w1,"timestamp_format"))
			safestrncpy(showmsg->timestamp_format, w2, 20);
		else if(!strcmpi(w1,"stdout_with_ansisequence"))
			showmsg->stdout_with_ansisequence = config_switch(w2) ? true : false;
		else if(!strcmpi(w1,"console_silent")) {
			showmsg->silent = atoi(w2);
			if (showmsg->silent) /* only bother if we actually have this enabled */
				ShowInfo("Console Silent Setting: %d\n", atoi(w2));
		}
		else if( !strcmpi(w1, "bind_ip") ) {
			login_config.login_ip = sockt->host2ip(w2);
			if( login_config.login_ip ) {
				char ip_str[16];
				ShowStatus("Login server binding IP address : %s -> %s\n", w2, sockt->ip2str(login_config.login_ip, ip_str));
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
			login_config.client_version_to_connect = (unsigned int)strtoul(w2, NULL, 10);
		else if(!strcmpi(w1, "use_MD5_passwords"))
			login_config.use_md5_passwds = (bool)config_switch(w2);
		else if(!strcmpi(w1, "group_id_to_connect"))
			login_config.group_id_to_connect = atoi(w2);
		else if(!strcmpi(w1, "min_group_id_to_connect"))
			login_config.min_group_id_to_connect = atoi(w2);
		else if(!strcmpi(w1, "date_format"))
			safestrncpy(login_config.date_format, w2, sizeof(login_config.date_format));
		else if(!strcmpi(w1, "allowed_regs")) //account flood protection system
			login_config.allowed_regs = atoi(w2);
		else if(!strcmpi(w1, "time_allowed"))
			login_config.time_allowed = atoi(w2);
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
			memset(md5, '\0', 33);

			if (sscanf(w2, "%d, %32s", &group, md5) == 2) {
				struct client_hash_node *nnode;
				CREATE(nnode, struct client_hash_node, 1);

				if (strcmpi(md5, "disabled") == 0) {
					nnode->hash[0] = '\0';
				} else {
					int i;
					for (i = 0; i < 32; i += 2) {
						char buf[3];
						unsigned int byte;

						memcpy(buf, &md5[i], 2);
						buf[2] = 0;

						sscanf(buf, "%x", &byte);
						nnode->hash[i / 2] = (uint8)(byte & 0xFF);
					}
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
			if (db)
				db->set_property(db, w1, w2);
			ipban_config_read(w1, w2);
			loginlog_config_read(w1, w2);
			HPM->parseConf(w1, w2, HPCT_LOGIN);
		}
	}
	fclose(fp);
	ShowInfo("Finished reading %s.\n", cfgName);
	return 0;
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
int do_final(void) {
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
	login->online_db->destroy(login->online_db, NULL);
	login->auth_db->destroy(login->auth_db, NULL);

	for( i = 0; i < ARRAYLENGTH(server); ++i )
		chrif_server_destroy(i);

	if( login->fd != -1 )
	{
		sockt->close(login->fd);
		login->fd = -1;
	}

	HPM_login_do_final();

	aFree(login->LOGIN_CONF_NAME);
	aFree(login->NET_CONF_NAME);

	HPM->event(HPET_POST_FINAL);

	ShowStatus("Finished.\n");
	return EXIT_SUCCESS;
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
void do_shutdown_login(void)
{
	if( core->runflag != LOGINSERVER_ST_SHUTDOWN )
	{
		int id;
		core->runflag = LOGINSERVER_ST_SHUTDOWN;
		ShowStatus("Shutting down...\n");
		// TODO proper shutdown procedure; kick all characters, wait for acks, ...  [FlavioJS]
		for( id = 0; id < ARRAYLENGTH(server); ++id )
			chrif_server_reset(id);
		sockt->flush_fifos();
		core->runflag = CORE_ST_STOP;
	}
}

/**
 * --login-config handler
 *
 * Overrides the default login configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(loginconfig)
{
	aFree(login->LOGIN_CONF_NAME);
	login->LOGIN_CONF_NAME = aStrdup(params);
	return true;
}
/**
 * --net-config handler
 *
 * Overrides the default subnet configuration file.
 * @see cmdline->exec
 */
static CMDLINEARG(netconfig)
{
	aFree(login->NET_CONF_NAME);
	login->NET_CONF_NAME = aStrdup(params);
	return true;
}
/**
 * Defines the local command line arguments
 */
void cmdline_args_init_local(void)
{
	CMDLINEARG_DEF2(login-config, loginconfig, "Alternative login-server configuration.", CMDLINE_OPT_PARAM);
	CMDLINEARG_DEF2(net-config, netconfig, "Alternative subnet configuration.", CMDLINE_OPT_PARAM);
}

//------------------------------
// Login server initialization
//------------------------------
int do_init(int argc, char** argv)
{
	int i;

	// initialize engine (to accept config settings)
	account_engine[0].db = account_engine[0].constructor();
	accounts = account_engine[0].db;
	if( accounts == NULL ) {
		ShowFatalError("do_init: account engine 'sql' not found.\n");
		exit(EXIT_FAILURE);
	}

	login_defaults();

	// read login-server configuration
	login_set_defaults();

	login->LOGIN_CONF_NAME = aStrdup("conf/login-server.conf");
	login->NET_CONF_NAME   = aStrdup("conf/network.conf");

	HPM_login_do_init();
	cmdline->exec(argc, argv, CMDLINE_OPT_PREINIT);
	HPM->config_read();
	HPM->event(HPET_PRE_INIT);

	cmdline->exec(argc, argv, CMDLINE_OPT_NORMAL);
	login_config_read(login->LOGIN_CONF_NAME);
	sockt->net_config_read(login->NET_CONF_NAME);

	for( i = 0; i < ARRAYLENGTH(server); ++i )
		chrif_server_init(i);

	// initialize logging
	if( login_config.log_login )
		loginlog_init();

	// initialize static and dynamic ipban system
	ipban_init();

	// Online user database init
	login->online_db = idb_alloc(DB_OPT_RELEASE_DATA);
	timer->add_func_list(login->waiting_disconnect_timer, "login->waiting_disconnect_timer");

	// Interserver auth init
	login->auth_db = idb_alloc(DB_OPT_RELEASE_DATA);

	// set default parser as login_parse_login function
	sockt->set_defaultparse(login->parse_login);

	// every 10 minutes cleanup online account db.
	timer->add_func_list(login->online_data_cleanup, "login->online_data_cleanup");
	timer->add_interval(timer->gettick() + 600*1000, login->online_data_cleanup, 0, 0, 600*1000);

	// add timer to detect ip address change and perform update
	if (login_config.ip_sync_interval) {
		timer->add_func_list(login->sync_ip_addresses, "login->sync_ip_addresses");
		timer->add_interval(timer->gettick() + login_config.ip_sync_interval, login->sync_ip_addresses, 0, 0, login_config.ip_sync_interval);
	}

	// Account database init
	if(!accounts->init(accounts)) {
		ShowFatalError("do_init: Failed to initialize account engine 'sql'.\n");
		exit(EXIT_FAILURE);
	}

	HPM->event(HPET_INIT);

	// server port open & binding
	if ((login->fd = sockt->make_listen_bind(login_config.login_ip,login_config.login_port)) == -1) {
		ShowFatalError("Failed to bind to port '"CL_WHITE"%d"CL_RESET"'\n",login_config.login_port);
		exit(EXIT_FAILURE);
	}

	if( core->runflag != CORE_ST_STOP ) {
		core->shutdown_callback = do_shutdown_login;
		core->runflag = LOGINSERVER_ST_RUNNING;
	}

	ShowStatus("The login-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %u).\n\n", login_config.login_port);
	login_log(0, "login server", 100, "login server started");

	HPM->event(HPET_READY);

	return 0;
}

void login_defaults(void) {
	login = &login_s;

	login->lc = &login_config;
	login->accounts = accounts;

	login->mmo_auth = login_mmo_auth;
	login->mmo_auth_new = login_mmo_auth_new;
	login->waiting_disconnect_timer = login_waiting_disconnect_timer;
	login->create_online_user = login_create_online_user;
	login->add_online_user = login_add_online_user;
	login->remove_online_user = login_remove_online_user;
	login->online_db_setoffline = login_online_db_setoffline;
	login->online_data_cleanup_sub = login_online_data_cleanup_sub;
	login->online_data_cleanup = login_online_data_cleanup;
	login->sync_ip_addresses = login_sync_ip_addresses;
	login->check_encrypted = login_check_encrypted;
	login->check_password = login_check_password;
	login->lan_subnet_check = login_lan_subnet_check;

	login->fromchar_auth_ack = login_fromchar_auth_ack;
	login->fromchar_accinfo = login_fromchar_accinfo;
	login->fromchar_account = login_fromchar_account;
	login->fromchar_account_update_other = login_fromchar_account_update_other;
	login->fromchar_ban = login_fromchar_ban;
	login->fromchar_change_sex_other = login_fromchar_change_sex_other;
	login->fromchar_pong = login_fromchar_pong;
	login->fromchar_parse_auth = login_fromchar_parse_auth;
	login->fromchar_parse_update_users = login_fromchar_parse_update_users;
	login->fromchar_parse_request_change_email = login_fromchar_parse_request_change_email;
	login->fromchar_parse_account_data = login_fromchar_parse_account_data;
	login->fromchar_parse_ping = login_fromchar_parse_ping;
	login->fromchar_parse_change_email = login_fromchar_parse_change_email;
	login->fromchar_parse_account_update = login_fromchar_parse_account_update;
	login->fromchar_parse_ban = login_fromchar_parse_ban;
	login->fromchar_parse_change_sex = login_fromchar_parse_change_sex;
	login->fromchar_parse_account_reg2 = login_fromchar_parse_account_reg2;
	login->fromchar_parse_unban = login_fromchar_parse_unban;
	login->fromchar_parse_account_online = login_fromchar_parse_account_online;
	login->fromchar_parse_account_offline = login_fromchar_parse_account_offline;
	login->fromchar_parse_online_accounts = login_fromchar_parse_online_accounts;
	login->fromchar_parse_request_account_reg2 = login_fromchar_parse_request_account_reg2;
	login->fromchar_parse_update_wan_ip = login_fromchar_parse_update_wan_ip;
	login->fromchar_parse_all_offline = login_fromchar_parse_all_offline;
	login->fromchar_parse_change_pincode = login_fromchar_parse_change_pincode;
	login->fromchar_parse_wrong_pincode = login_fromchar_parse_wrong_pincode;
	login->fromchar_parse_accinfo = login_fromchar_parse_accinfo;

	login->parse_fromchar = login_parse_fromchar;
	login->parse_login = login_parse_login;
	login->parse_ping = login_parse_ping;
	login->parse_client_md5 = login_parse_client_md5;
	login->parse_client_login = login_parse_client_login;
	login->parse_request_coding_key = login_parse_request_coding_key;
	login->parse_request_connection = login_parse_request_connection;
	login->auth_ok = login_auth_ok;
	login->auth_failed = login_auth_failed;
	login->char_server_connection_status = login_char_server_connection_status;
	login->connection_problem = login_connection_problem;
	login->kick = login_kick;
	login->login_error = login_login_error;
	login->send_coding_key = login_send_coding_key;

	login->LOGIN_CONF_NAME = NULL;
	login->NET_CONF_NAME = NULL;
}
