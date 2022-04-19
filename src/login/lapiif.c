/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020-2021 Andrei Karas (4144)
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

#include "lapiif.h"

#include "login/login.h"
#include "login/packets_ac_struct.h"

#include "common/cbasetypes.h"
#include "common/apipackets.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"

#include <string.h>

static struct lapiif_interface lapiif_s;
struct lapiif_interface *lapiif;

static void lapiif_disconnect_user(int account_id)
{
	for (int i = 0; i < ARRAYLENGTH(login->dbs->api_server); ++i) {
		const int fd = login->dbs->api_server[i].fd;
		if (!sockt->session_is_active(fd))
			continue;
		WFIFOHEAD(fd, 6);
		WFIFOW(fd, 0) = 0x2813;
		WFIFOL(fd, 2) = account_id;
		WFIFOSET(fd, 6);
	}
}

static void lapiif_connect_user(struct login_session_data *sd, const unsigned char* auth_token)
{
	nullpo_retv(sd);
	nullpo_retv(auth_token);

	for (int i = 0; i < ARRAYLENGTH(login->dbs->api_server); ++i) {
		const int fd = login->dbs->api_server[i].fd;
		if (!sockt->session_is_active(fd))
			continue;
		WFIFOHEAD(fd, 6 + AUTH_TOKEN_SIZE);
		WFIFOW(fd, 0) = 0x2814;
		WFIFOL(fd, 2) = sd->account_id;
		memcpy(WFIFOP(fd, 6), auth_token, AUTH_TOKEN_SIZE);
		WFIFOSET(fd, 6 + AUTH_TOKEN_SIZE);
	}
}

static void lapiif_connect_user_char(int char_server, int account_id)
{
	lapiif->set_char_online(account_id, 0);
}

/// Initializes a server structure.
static void lapiif_server_init(int id)
{
	Assert_retv(id >= 0 && id < MAX_SERVERS);
	memset(&login->dbs->api_server[id], 0, sizeof(login->dbs->api_server[id]));
	login->dbs->api_server[id].fd = -1;
}


/// Destroys a server structure.
static void lapiif_server_destroy(int id)
{
	Assert_retv(id >= 0 && id < MAX_SERVERS);
	if (login->dbs->api_server[id].fd != -1)
	{
		sockt->close(login->dbs->api_server[id].fd);
		login->dbs->api_server[id].fd = -1;
	}
}


/// Resets all the data related to a server.
static void lapiif_server_reset(int id)
{
	lapiif->server_destroy(id);
	lapiif->server_init(id);
}


/// Called when the connection to Char Server is disconnected.
static void lapiif_on_disconnect(int id)
{
	Assert_retv(id >= 0 && id < MAX_SERVERS);
	ShowStatus("Api-server has disconnected.\n");
	lapiif->server_reset(id);
}

static int lapiif_parse(int fd)
{
	int id;
	ARR_FIND(0, ARRAYLENGTH(login->dbs->api_server), id, login->dbs->api_server[id].fd == fd);
	if (id == ARRAYLENGTH(login->dbs->api_server))
	{  // not an api server
		ShowDebug("lapiif_parse: Disconnecting invalid session #%d (is not a api-server)\n", fd);
		sockt->eof(fd);
		sockt->close(fd);
		return 0;
	}

	if (sockt->session[fd]->flag.eof) {
		sockt->close(fd);
		login->dbs->api_server[id].fd = -1;
		lapiif->on_disconnect(id);
		return 0;
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

		if (cmd < LAPIIF_PACKET_LEN_TABLE_START || cmd >= LAPIIF_PACKET_LEN_TABLE_START + ARRAYLENGTH(lapiif->packet_len_table) || lapiif->packet_len_table[cmd - LAPIIF_PACKET_LEN_TABLE_START] == 0) {
			ShowWarning("lapiif_parse: session #%d, failed (unrecognized command 0x%.4x).\n", fd, (unsigned int)cmd);
			sockt->eof(fd);
			return 0;
		}

		int packet_len = lapiif->packet_len_table[cmd - LAPIIF_PACKET_LEN_TABLE_START];

		if (packet_len == -1) { // dynamic-length packet, second WORD holds the length
			if (RFIFOREST(fd) < 4)
				return 0;
			packet_len = RFIFOW(fd, 2);
		}

		if ((int)RFIFOREST(fd) < packet_len)
			return 0;

		ShowDebug("Received packet 0x%4x (%d bytes) from api-server (connection %d)\n", (uint32)cmd, packet_len, fd);

		switch (cmd) {
			case 0x2841: lapiif->parse_ping(fd); break;
			case HEADER_API_PROXY_REQUEST: lapiif->parse_proxy_api_to_char(fd); break;
			default:
				ShowError("lapiif_parse : unknown packet (session #%d): 0x%x. Disconnecting.\n", fd, (unsigned int)cmd);
				sockt->eof(fd);
				return 0;
		}
		if (sockt->session_is_valid(fd)) //There's the slight chance we lost the connection during parse, in which case this would segfault if not checked [Skotlex]
			RFIFOSKIP(fd, packet_len);
	}

	return 0;
}

static void lapiif_parse_ping(int fd)
{
	lapiif->pong(fd);
}

static void lapiif_parse_proxy_api_to_char(int fd)
{
	const int char_server_id = RFIFOL(fd, 6);

	Assert_retv(char_server_id >= 0 && char_server_id < MAX_SERVERS);
	const int char_fd = login->dbs->server[char_server_id].fd;
	if (!sockt->session_is_valid(fd)) {
		return;
	}
	const int len = RFIFOW(fd, 2);
	WFIFOHEAD(char_fd, len);
	memcpy(WFIFOP(char_fd, 0), RFIFOP(fd, 0), len);
	struct PACKET_API_PROXY *p = WFIFOP(char_fd, 0);
	p->packet_id = HEADER_API_PROXY_REQUEST;
	p->char_server_id = fd;
	WFIFOSET(char_fd, len);
}

static void lapiif_parse_proxy_api_from_char(int fd)
{
	const int api_fd = RFIFOL(fd, 6);
	const int len = RFIFOW(fd, 2);
	WFIFOHEAD(api_fd, len);
	memcpy(WFIFOP(api_fd, 0), RFIFOP(fd, 0), len);
	WFIFOW(api_fd, 0) = HEADER_API_PROXY_REPLY;
	WFIFOSET(api_fd, len);
}

static void lapiif_pong(int fd)
{
	WFIFOHEAD(fd, 2);
	WFIFOW(fd, 0) = 0x2812;
	WFIFOSET(fd, 2);
}

static void lapiif_add_char_server_to(int char_server_id, int api_server_id)
{
	Assert_retv(char_server_id >= 0 && char_server_id < MAX_SERVERS);
	Assert_retv(api_server_id >= 0 && api_server_id < MAX_SERVERS);
	Assert_retv(sockt->session_is_active(login->dbs->api_server[api_server_id].fd));

	const int fd = login->dbs->api_server[api_server_id].fd;

	WFIFOHEAD(fd, 4 + MAX_CHARSERVER_NAME_SIZE);
	WFIFOW(fd, 0) = 0x2817;
	WFIFOW(fd, 2) = char_server_id;
	safestrncpy(WFIFOP(fd, 4), login->dbs->server[char_server_id].name, MAX_CHARSERVER_NAME_SIZE);
	WFIFOSET(fd, 4 + MAX_CHARSERVER_NAME_SIZE);
}

static void lapiif_add_char_server(int char_server_id)
{
	Assert_retv(char_server_id >= 0 && char_server_id < MAX_SERVERS);

	for (int i = 0; i < ARRAYLENGTH(login->dbs->api_server); ++i) {
		if (!sockt->session_is_valid(login->dbs->api_server[i].fd))
			continue;
		lapiif->add_char_server_to(char_server_id, i);
	}
}

static void lapiif_remove_char_server_from(int char_server_id, int api_server_id)
{
	Assert_retv(char_server_id >= 0 && char_server_id < MAX_SERVERS);
	Assert_retv(api_server_id >= 0 && api_server_id < MAX_SERVERS);
	Assert_retv(sockt->session_is_active(login->dbs->api_server[api_server_id].fd));

	const int fd = login->dbs->api_server[api_server_id].fd;
	WFIFOHEAD(fd, 4);
	WFIFOW(fd, 0) = 0x2816;
	WFIFOW(fd, 2) = char_server_id;
	WFIFOSET(fd, 4);
}

static void lapiif_remove_char_server(int char_server_id)
{
	Assert_retv(char_server_id >= 0 && char_server_id < MAX_SERVERS);

	for (int i = 0; i < ARRAYLENGTH(login->dbs->api_server); ++i) {
		if (!sockt->session_is_valid(login->dbs->api_server[i].fd))
			continue;
		lapiif->remove_char_server_from(char_server_id, i);
	}
}

static void lapiif_send_char_servers(int api_server_id)
{
	Assert_retv(api_server_id >= 0 && api_server_id < MAX_SERVERS);
	Assert_retv(sockt->session_is_active(login->dbs->api_server[api_server_id].fd));

	int server_num = 0;

	for (int i = 0; i < ARRAYLENGTH(login->dbs->server); ++i) {
		if (sockt->session_is_active(login->dbs->server[i].fd))
			server_num++;
	}

	const int part_size = 2 + MAX_CHARSERVER_NAME_SIZE;
	int length = 4 + part_size * server_num;
	const int fd = login->dbs->api_server[api_server_id].fd;

	WFIFOHEAD(fd, length);
	WFIFOW(fd, 0) = 0x2815;
	WFIFOW(fd, 2) = length;
	int offset = 4;

	for (int i = 0; i < ARRAYLENGTH(login->dbs->server); ++i) {
		if (!sockt->session_is_valid(login->dbs->server[i].fd))
			continue;

		WFIFOW(fd, offset) = i;
		offset += 2;
		safestrncpy(WFIFOP(fd, offset), login->dbs->server[i].name, MAX_CHARSERVER_NAME_SIZE);
		offset += MAX_CHARSERVER_NAME_SIZE;
	}
	WFIFOSET(fd, length);
}

static void lapiif_set_char_online(int account_id, int char_id)
{
	for (int i = 0; i < ARRAYLENGTH(login->dbs->api_server); ++i) {
		if (!sockt->session_is_valid(login->dbs->api_server[i].fd))
			continue;
		lapiif->set_char_online_to(account_id, char_id, i);
	}
}

static void lapiif_set_char_online_to(int account_id, int char_id, int api_server_id)
{
	Assert_retv(api_server_id >= 0 && api_server_id < MAX_SERVERS);
	Assert_retv(sockt->session_is_active(login->dbs->api_server[api_server_id].fd));

	const int fd = login->dbs->api_server[api_server_id].fd;

	WFIFOHEAD(fd, 10);
	WFIFOW(fd, 0) = 0x2819;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOSET(fd, 10);
}

static void lapiif_init(void)
{
}

static void lapiif_final(void)
{
}

void lapiif_defaults(void)
{
	lapiif = &lapiif_s;

	const int packet_len_table[LAPIIF_PACKET_LEN_TABLE_SIZE] = {
		0,  2, -1,  0,  0,  0,  0,  0 // 0x2840 - 0x2847
	};

	memcpy(lapiif->packet_len_table, &packet_len_table, sizeof(lapiif->packet_len_table));

	lapiif->init = lapiif_init;
	lapiif->final = lapiif_final;
	lapiif->connect_user = lapiif_connect_user;
	lapiif->connect_user_char = lapiif_connect_user_char;
	lapiif->disconnect_user = lapiif_disconnect_user;
	lapiif->server_init = lapiif_server_init;
	lapiif->server_destroy = lapiif_server_destroy;
	lapiif->server_reset = lapiif_server_reset;
	lapiif->on_disconnect = lapiif_on_disconnect;
	lapiif->pong = lapiif_pong;
	lapiif->parse = lapiif_parse;
	lapiif->parse_ping = lapiif_parse_ping;
	lapiif->parse_proxy_api_to_char = lapiif_parse_proxy_api_to_char;
	lapiif->parse_proxy_api_from_char = lapiif_parse_proxy_api_from_char;
	lapiif->add_char_server = lapiif_add_char_server;
	lapiif->add_char_server_to = lapiif_add_char_server_to;
	lapiif->remove_char_server = lapiif_remove_char_server;
	lapiif->remove_char_server_from = lapiif_remove_char_server_from;
	lapiif->send_char_servers = lapiif_send_char_servers;
	lapiif->set_char_online = lapiif_set_char_online;
	lapiif->set_char_online_to = lapiif_set_char_online_to;
}
