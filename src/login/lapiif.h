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
#ifndef LOGIN_LAPIIF_H
#define LOGIN_LAPIIF_H

#include "common/hercules.h"
#include "common/cbasetypes.h"

struct login_session_data;

#define LAPIIF_PACKET_LEN_TABLE_START 0x2840
#define LAPIIF_PACKET_LEN_TABLE_SIZE 0x8

/**
 * Lapi.c Interface
 **/
struct lapiif_interface {
	int packet_len_table[LAPIIF_PACKET_LEN_TABLE_SIZE];

	void (*init) (void);
	void (*final) (void);
	void (*connect_user) (struct login_session_data *sd, const unsigned char* auth_token);
	void (*disconnect_user) (int account_id);
	void (*server_init) (int id);
	void (*server_destroy) (int id);
	void (*server_reset) (int id);
	void (*on_disconnect) (int id);
	void (*pong) (int id);
	void (*add_char_server) (int char_server_id);
	void (*add_char_server_to) (int char_server_id, int api_server_id);
	void (*remove_char_server) (int char_server_id);
	void (*remove_char_server_from) (int char_server_id, int api_server_id);
	void (*send_char_servers) (int api_server_id);
	void (*set_char_online) (int account_id, int char_id);
	void (*set_char_online_to) (int account_id, int char_id, int api_server_id);
	int (*parse) (int fd);
	void (*parse_ping) (int fd);
	void (*parse_proxy_api_to_char) (int fd);
	void (*parse_proxy_api_from_char) (int fd);
};

#ifdef HERCULES_CORE
void lapiif_defaults(void);
#endif // HERCULES_CORE

HPShared struct lapiif_interface *lapiif;

#endif /* LOGIN_LAPIIF_H */
