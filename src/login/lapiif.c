/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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

#include "common/cbasetypes.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"

#include <string.h>

static struct lapiif_interface lapiif_s;
struct lapiif_interface *lapiif;

static void lapiif_disconnect_user(int account_id)
{
}

static void lapiif_connect_user(struct login_session_data *sd, const unsigned char* auth_token)
{
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
	if (sockt->session[fd]->flag.eof) {
		sockt->close(fd);
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
			case 0x2821: lapiif->parse_ping(fd); break;
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

static void lapiif_pong(int fd)
{
	WFIFOHEAD(fd, 2);
	WFIFOW(fd, 0) = 0x2812;
	WFIFOSET(fd, 2);
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
		0, 2, 0, 0, 0, 0, 0, 0 // 2820,
	};

	memcpy(lapiif->packet_len_table, &packet_len_table, sizeof(lapiif->packet_len_table));

	lapiif->init = lapiif_init;
	lapiif->final = lapiif_final;
	lapiif->connect_user = lapiif_connect_user;
	lapiif->disconnect_user = lapiif_disconnect_user;
	lapiif->server_init = lapiif_server_init;
	lapiif->server_destroy = lapiif_server_destroy;
	lapiif->server_reset = lapiif_server_reset;
	lapiif->on_disconnect = lapiif_on_disconnect;
	lapiif->pong = lapiif_pong;
	lapiif->parse = lapiif_parse;
	lapiif->parse_ping = lapiif_parse_ping;
}
