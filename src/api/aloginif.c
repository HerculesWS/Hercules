/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
 * Copyright (C) Athena Dev Teams
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

#include "config/core.h" // AUTOTRADE_PERSISTENCY
#include "aloginif.h"

#include "api/aclif.h"
#include "common/HPM.h"
#include "common/api.h"
#include "common/cbasetypes.h"
#include "common/ers.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"

#include "api/api.h"
#include "api/apisessiondata.h"
#include "api/handlers.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

static struct aloginif_interface aloginif_s;
struct aloginif_interface *aloginif;


// sets login-server's user id
static void aloginif_setuserid(char *id)
{
	nullpo_retv(id);
	memcpy(aloginif->userid, id, NAME_LENGTH);
}

// sets login-server's password
static void aloginif_setpasswd(char *pwd)
{
	nullpo_retv(pwd);
	memcpy(aloginif->passwd, pwd, NAME_LENGTH);
}

// security check, prints warning if using default password
static void aloginif_checkdefaultlogin(void)
{
#ifndef BUILDBOT
	if (strcmp(aloginif->userid, "s1")==0 && strcmp(aloginif->passwd, "p1")==0) {
		ShowWarning("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your 'login' table to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("and then edit your user/password in conf/api-server.conf (or conf/import/api_conf.txt)\n");
	}
#endif
}

// sets login-server's ip address
static bool aloginif_setip(const char *ip)
{
	char ip_str[16];

	nullpo_retr(false, ip);
	if (!(aloginif->ip = sockt->host2ip(ip))) {
		ShowWarning("Failed to Resolve Login Server Address! (%s)\n", ip);
		return false;
	}

	safestrncpy(aloginif->ip_str, ip, sizeof(aloginif->ip_str));

	ShowInfo("Login Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, sockt->ip2str(aloginif->ip, ip_str));

	return true;
}

// sets login-server's port number
static void aloginif_setport(uint16 port)
{
	aloginif->port = port;
}

static void aloginif_connect_to_server(void)
{
	Assert_retv(aloginif->fd != -1);

	WFIFOHEAD(aloginif->fd, 50);
	WFIFOW(aloginif->fd, 0) = 0x2720;
	memcpy(WFIFOP(aloginif->fd, 2), aloginif->userid, NAME_LENGTH);
	memcpy(WFIFOP(aloginif->fd, 26), aloginif->passwd, NAME_LENGTH);
	WFIFOSET(aloginif->fd, 50);
}

/*==========================================
 *
 *------------------------------------------*/
static int aloginif_parse(int fd)
{
	// only process data from the char-server
	if (fd != aloginif->fd) {
		ShowDebug("aloginif_parse: Disconnecting invalid session #%d (is not the login-server)\n", fd);
		sockt->close(fd);
		return 0;
	}

	if (sockt->session[fd]->flag.eof) {
		sockt->close(fd);
		aloginif->fd = -1;
		aloginif->on_disconnect();
		return 0;
	} else if (sockt->session[fd]->flag.ping) { /* we've reached stall time */
		if (DIFF_TICK(sockt->last_tick, sockt->session[fd]->rdata_tick) > (sockt->stall_time * 2)) { /* we can't wait any longer */
			sockt->eof(fd);
			return 0;
		} else if (sockt->session[fd]->flag.ping != 2) { /* we haven't sent ping out yet */
			aloginif->keepalive(fd);
			sockt->session[fd]->flag.ping = 2;
		}
	}

	while (RFIFOREST(fd) >= 2) {
		int cmd = RFIFOW(fd, 0);

/*
		if (VECTOR_LENGTH(HPM->packets[hpChrif_Parse]) > 0) {
			int result = HPM->parse_packets(fd,cmd,hpChrif_Parse);
			if (result == 1)
				continue;
			if (result == 2)
				return 0;
		}
*/

		if (cmd < ALOGINIF_PACKET_LEN_TABLE_START || cmd >= ALOGINIF_PACKET_LEN_TABLE_START + ARRAYLENGTH(aloginif->packet_len_table) || aloginif->packet_len_table[cmd - ALOGINIF_PACKET_LEN_TABLE_START] == 0) {
			ShowWarning("aloginif_parse: session #%d, failed (unrecognized command 0x%.4x).\n", fd, (unsigned int)cmd);
			sockt->eof(fd);
			return 0;
		}

		int packet_len = aloginif->packet_len_table[cmd - ALOGINIF_PACKET_LEN_TABLE_START];

		if (packet_len == -1) { // dynamic-length packet, second WORD holds the length
			if (RFIFOREST(fd) < 4)
				return 0;
			packet_len = RFIFOW(fd, 2);
		}

		if ((int)RFIFOREST(fd) < packet_len)
			return 0;

		ShowDebug("Received packet 0x%4x (%d bytes) from login-server (connection %d)\n", (uint32)cmd, packet_len, fd);

		switch (cmd) {
			case 0x2811: aloginif->parse_connection_state(fd); break;
			case 0x2812: aloginif->parse_pong(fd); break;
			case 0x2813: aloginif->parse_disconnect_user(fd); break;
			case 0x2814: aloginif->parse_connect_user(fd); break;
			case 0x2815: aloginif->parse_char_servers_list(fd); break;
			case 0x2816: aloginif->parse_remove_char_server(fd); break;
			case 0x2817: aloginif->parse_add_char_server(fd); break;
			case 0x2818: aloginif->parse_proxy_from_char_server(fd); break;
			default:
				ShowError("aloginif_parse : unknown packet (session #%d): 0x%x. Disconnecting.\n", fd, (unsigned int)cmd);
				sockt->eof(fd);
				return 0;
		}
		if (fd == aloginif->fd) //There's the slight chance we lost the connection during parse, in which case this would segfault if not checked [Skotlex]
			RFIFOSKIP(fd, packet_len);
	}

	return 0;
}

static void aloginif_on_ready(void)
{
}

static int aloginif_parse_pong(int fd)
{
	if (sockt->session[fd])
		sockt->session[fd]->flag.ping = 0;
	return 0;
}

static int aloginif_parse_disconnect_user(int fd)
{
	aclif->delete_online_player(RFIFOL(fd, 2));
	return 0;
}

static int aloginif_parse_connect_user(int fd)
{
	aclif->add_online_player(RFIFOL(fd, 2), RFIFOP(fd, 6));
	return 0;
}

static int aloginif_parse_connection_state(int fd)
{
	switch (RFIFOB(fd, 2)) {
	case 0:
		ShowStatus("Connected to login-server (connection #%d).\n", fd);
		aloginif->on_ready();
		break;
	case 1: // Invalid username/password
		ShowError("Can not connect to login-server.\n");
		ShowError("The server communication passwords (default s1/p1) are probably invalid.\n");
		ShowError("Also, please make sure your login db has the correct communication username/passwords and the gender of the account is S.\n");
		ShowError("The communication passwords are set in /conf/map/map-server.conf and /conf/char/char-server.conf\n");
		sockt->eof(fd);
		return 1;
	case 2: // IP not allowed
		ShowError("Can not connect to login-server.\n");
		ShowError("Please make sure your IP is allowed in conf/network.conf\n");
		sockt->eof(fd);
		return 1;
	default:
		ShowError("Invalid response from the login-server. Error code: %d\n", (int)RFIFOB(fd, 2));
		sockt->eof(fd);
		return 1;
	}
	return 0;
}

static int aloginif_parse_char_servers_list(int fd)
{
	db_clear(aclif->char_servers_db);
	db_clear(aclif->char_servers_id_db);

	const int part_size = 2 + MAX_CHARSERVER_NAME_SIZE;

	int offset = 4;
	const int count = (RFIFOW(fd, 2) - offset) / part_size;
	ShowInfo("Got %d char servers.\n", count);
	for (int f = 0; f < count; f ++) {
		aclif->add_char_server(RFIFOW(fd, offset), aStrdup(RFIFOP(fd, offset + 2)));
		offset += part_size;
	}
	return 0;
}

static int aloginif_parse_remove_char_server(int fd)
{
	const int char_server_id = RFIFOW(fd, 2);
	const char *name = idb_get(aclif->char_servers_id_db, char_server_id);
	nullpo_retr(1, name);

	aclif->remove_char_server(char_server_id, name);
	return 0;
}

static int aloginif_parse_add_char_server(int fd)
{
	const int char_server_id = RFIFOW(fd, 2);
	struct char_server_data *data = aCalloc(1, sizeof(struct char_server_data));
	data->id = char_server_id;
	char *name = aStrdup(RFIFOP(fd, 4));
	strdb_put(aclif->char_servers_db, name, data);
	idb_put(aclif->char_servers_id_db, data->id, name);
	return 0;
}

/// Called when the connection to Login Server is disconnected.
static void aloginif_on_disconnect(void)
{
	ShowWarning("Connection to Login Server lost.\n\n");
}

static void aloginif_keepalive(int fd)
{
	WFIFOHEAD(fd, 2);
	WFIFOW(fd, 0) = 0x2841;
	WFIFOSET(fd, 2);
}

static void aloginif_send_to_char(int fd, struct api_session_data *sd, int msg_id)
{
	Assert_retv(aloginif->fd != -1);

	const int len = WFIFO_APICHAR_SIZE;
	WFIFO_APICHAR_PACKET(msg_id, sd, len);

	WFIFOSET(aloginif->fd, len);
}

static void aloginif_parse_from_char(int fd, Handler_func func)
{
	const int user_fd = RFIFOL(fd, 10);
	if (!sockt->session_is_active(user_fd))
		return;
	struct api_session_data *sd = sockt->session[user_fd]->session_data;
	nullpo_retv(sd);
	if (sd->account_id != RFIFOL(fd, 14))
		return;
	if (sd->id != RFIFOL(fd, 18))
		return;

	if (func != NULL) {
		func(user_fd, sd, RFIFOP(fd, WFIFO_APICHAR_SIZE), RFIFOW(fd, 2) - WFIFO_APICHAR_SIZE);
	} else {
		aclif->terminate_connection(user_fd);
	}
}

static int aloginif_parse_proxy_from_char_server(int fd)
{
	const uint command = RFIFOW(fd, 4);
	Assert_ret(command > 0 && command < API_MSG_MAX);
	aloginif->parse_from_char(fd, aloginif->msg_map[command]);
	return 0;
}

/*==========================================
 * Destructor
 *------------------------------------------*/
static void do_final_aloginif(void)
{
	if (aloginif->fd != -1) {
		sockt->close(aloginif->fd);
		aloginif->fd = -1;
	}
}

/*==========================================
 *
 *------------------------------------------*/
static void do_init_aloginif(bool minimal)
{
	if (minimal)
		return;

	// establish api-login connection if not present
	timer->add_func_list(api->check_connect_login_server, "api->check_connect_login_server");
	timer->add_interval(timer->gettick() + 1000, api->check_connect_login_server, 0, 0, 10 * 1000);
}

/*=====================================
* Default Functions : aloginif.h
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void aloginif_defaults(void)
{
	aloginif = &aloginif_s;

	const int packet_len_table[ALOGINIF_PACKET_LEN_TABLE_SIZE] = {
		 0,  3,  2,  6, 22, -1,  4, 24, // 0x2810 - 0x2817
		-1,  0,  0,  0,  0,  0,  0,  0, // 0x2818 - 0x2825
	};

	/* vars */
	aloginif->connected = 0;

	memcpy(aloginif->packet_len_table, &packet_len_table, sizeof(aloginif->packet_len_table));
	aloginif->fd = -1;
	aloginif->srvinfo = 0;
	memset(aloginif->ip_str, 0, sizeof(aloginif->ip_str));
	aloginif->ip = 0;
	aloginif->port = 6900;
	memset(aloginif->userid, 0, sizeof(aloginif->userid));
	memset(aloginif->passwd, 0, sizeof(aloginif->passwd));
	aloginif->state = 0;

	/* */
	aloginif->init = do_init_aloginif;
	aloginif->final = do_final_aloginif;

	/* funcs */
	aloginif->setuserid = aloginif_setuserid;
	aloginif->setpasswd = aloginif_setpasswd;
	aloginif->checkdefaultlogin = aloginif_checkdefaultlogin;
	aloginif->setip = aloginif_setip;
	aloginif->setport = aloginif_setport;
	aloginif->connect_to_server = aloginif_connect_to_server;
	aloginif->on_disconnect = aloginif_on_disconnect;
	aloginif->keepalive = aloginif_keepalive;
	aloginif->on_ready = aloginif_on_ready;
	aloginif->send_to_char = aloginif_send_to_char;

	aloginif->parse = aloginif_parse;
	aloginif->parse_connection_state = aloginif_parse_connection_state;
	aloginif->parse_pong = aloginif_parse_pong;
	aloginif->parse_disconnect_user = aloginif_parse_disconnect_user;
	aloginif->parse_connect_user = aloginif_parse_connect_user;
	aloginif->parse_char_servers_list = aloginif_parse_char_servers_list;
	aloginif->parse_add_char_server = aloginif_parse_add_char_server;
	aloginif->parse_remove_char_server = aloginif_parse_remove_char_server;
	aloginif->parse_proxy_from_char_server = aloginif_parse_proxy_from_char_server;
	aloginif->parse_from_char = aloginif_parse_from_char;
}
