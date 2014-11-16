// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "loginif.h"

#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "mapif.h"

#include "../common/cbasetypes.h"
#include "../common/core.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"

struct loginif_interface loginif_s;

/// Resets all the data.
void loginif_reset(void)
{
	int id;
	// TODO kick everyone out and reset everything or wait for connect and try to reacquire locks [FlavioJS]
	for( id = 0; id < ARRAYLENGTH(chr->server); ++id )
		mapif->server_reset(id);
	flush_fifos();
	exit(EXIT_FAILURE);
}


/// Checks the conditions for the server to stop.
/// Releases the cookie when all characters are saved.
/// If all the conditions are met, it stops the core loop.
void loginif_check_shutdown(void)
{
	if( runflag != CHARSERVER_ST_SHUTDOWN )
		return;
	runflag = CORE_ST_STOP;
}


/// Called when the connection to Login Server is disconnected.
void loginif_on_disconnect(void)
{
	ShowWarning("Connection to Login Server lost.\n\n");
}


/// Called when all the connection steps are completed.
void loginif_on_ready(void)
{
	int i;

	loginif->check_shutdown();

	//Send online accounts to login server.
	chr->send_accounts_tologin(INVALID_TIMER, timer->gettick(), 0, 0);

	// if no map-server already connected, display a message...
	ARR_FIND( 0, ARRAYLENGTH(chr->server), i, chr->server[i].fd > 0 && chr->server[i].map );
	if( i == ARRAYLENGTH(chr->server) )
		ShowStatus("Awaiting maps from map-server.\n");
}

void do_init_loginif(void)
{
	// establish char-login connection if not present
	timer->add_func_list(chr->check_connect_login_server, "chr->check_connect_login_server");
	timer->add_interval(timer->gettick() + 1000, chr->check_connect_login_server, 0, 0, 10 * 1000);

	// send a list of all online account IDs to login server
	timer->add_func_list(chr->send_accounts_tologin, "chr->send_accounts_tologin");
	timer->add_interval(timer->gettick() + 1000, chr->send_accounts_tologin, 0, 0, 3600 * 1000); //Sync online accounts every hour
}

void do_final_loginif(void)
{
	if( chr->login_fd != -1 )
	{
		do_close(chr->login_fd);
		chr->login_fd = -1;
	}
}

void loginif_block_account(int account_id, int flag)
{
	WFIFOHEAD(chr->login_fd,10);
	WFIFOW(chr->login_fd,0) = 0x2724;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOL(chr->login_fd,6) = flag; // new account status
	WFIFOSET(chr->login_fd,10);
}

void loginif_ban_account(int account_id, short year, short month, short day, short hour, short minute, short second)
{
	WFIFOHEAD(chr->login_fd,18);
	WFIFOW(chr->login_fd, 0) = 0x2725;
	WFIFOL(chr->login_fd, 2) = account_id;
	WFIFOW(chr->login_fd, 6) = year;
	WFIFOW(chr->login_fd, 8) = month;
	WFIFOW(chr->login_fd,10) = day;
	WFIFOW(chr->login_fd,12) = hour;
	WFIFOW(chr->login_fd,14) = minute;
	WFIFOW(chr->login_fd,16) = second;
	WFIFOSET(chr->login_fd,18);
}

void loginif_unban_account(int account_id)
{
	WFIFOHEAD(chr->login_fd,6);
	WFIFOW(chr->login_fd,0) = 0x272a;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOSET(chr->login_fd,6);
}

void loginif_changesex(int account_id)
{
	WFIFOHEAD(chr->login_fd,6);
	WFIFOW(chr->login_fd,0) = 0x2727;
	WFIFOL(chr->login_fd,2) = account_id;
	WFIFOSET(chr->login_fd,6);
}

void loginif_auth(int fd, struct char_session_data* sd, uint32 ipl)
{
	WFIFOHEAD(chr->login_fd,23);
	WFIFOW(chr->login_fd,0) = 0x2712; // ask login-server to authenticate an account
	WFIFOL(chr->login_fd,2) = sd->account_id;
	WFIFOL(chr->login_fd,6) = sd->login_id1;
	WFIFOL(chr->login_fd,10) = sd->login_id2;
	WFIFOB(chr->login_fd,14) = sd->sex;
	WFIFOL(chr->login_fd,15) = htonl(ipl);
	WFIFOL(chr->login_fd,19) = fd;
	WFIFOSET(chr->login_fd,23);
}

void loginif_send_users_count(int users)
{
	WFIFOHEAD(chr->login_fd,6);
	WFIFOW(chr->login_fd,0) = 0x2714;
	WFIFOL(chr->login_fd,2) = users;
	WFIFOSET(chr->login_fd,6);
}

void loginif_connect_to_server(void)
{
	WFIFOHEAD(chr->login_fd,86);
	WFIFOW(chr->login_fd,0) = 0x2710;
	memcpy(WFIFOP(chr->login_fd,2), chr->userid, NAME_LENGTH);
	memcpy(WFIFOP(chr->login_fd,26), chr->passwd, NAME_LENGTH);
	WFIFOL(chr->login_fd,50) = 0;
	WFIFOL(chr->login_fd,54) = htonl(chr->ip);
	WFIFOW(chr->login_fd,58) = htons(chr->port);
	memcpy(WFIFOP(chr->login_fd,60), chr->server_name, 20);
	WFIFOW(chr->login_fd,80) = 0;
	WFIFOW(chr->login_fd,82) = chr->server_type;
	WFIFOW(chr->login_fd,84) = chr->new_display; //only display (New) if they want to [Kevin]
	WFIFOSET(chr->login_fd,86);
}

void loginif_defaults(void) {
	loginif = &loginif_s;

	loginif->init = do_init_loginif;
	loginif->final = do_final_loginif;
	loginif->reset = loginif_reset;
	loginif->check_shutdown = loginif_check_shutdown;
	loginif->on_disconnect = loginif_on_disconnect;
	loginif->on_ready = loginif_on_ready;
	loginif->block_account = loginif_block_account;
	loginif->ban_account = loginif_ban_account;
	loginif->unban_account = loginif_unban_account;
	loginif->changesex = loginif_changesex;
	loginif->auth = loginif_auth;
	loginif->send_users_count = loginif_send_users_count;
	loginif->connect_to_server = loginif_connect_to_server;
}
