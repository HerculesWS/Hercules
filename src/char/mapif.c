// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "mapif.h"

#include <stdlib.h>

#include "char.h"
#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

void mapif_ban(int id, unsigned int flag, int status);
void mapif_server_init(int id);
void mapif_server_destroy(int id);
void mapif_server_reset(int id);
void mapif_on_disconnect(int id);
void mapif_on_parse_accinfo(int account_id, int u_fd, int u_aid, int u_group, int map_fd);
void mapif_char_ban(int char_id, time_t timestamp);
int mapif_sendall(unsigned char *buf, unsigned int len);
int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len);
int mapif_send(int fd, unsigned char *buf, unsigned int len);
void mapif_send_users_count(int users);

void mapif_defaults(void) {
	mapif = &mapif_s;

	mapif->ban = mapif_ban;
	mapif->server_init = mapif_server_init;
	mapif->server_destroy = mapif_server_destroy;
	mapif->server_reset = mapif_server_reset;
	mapif->on_disconnect = mapif_on_disconnect;
	mapif->on_parse_accinfo = mapif_on_parse_accinfo;
	mapif->char_ban = mapif_char_ban;
	mapif->sendall = mapif_sendall;
	mapif->sendallwos = mapif_sendallwos;
	mapif->send = mapif_send;
	mapif->send_users_count = mapif_send_users_count;
}
