// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "mapif.h"

#include <stdlib.h>

#include "char.h"
#include "int_auction.h"
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
void mapif_auction_message(int char_id, unsigned char result);
void mapif_auction_sendlist(int fd, int char_id, short count, short pages, unsigned char *buf);
void mapif_parse_auction_requestlist(int fd);
void mapif_auction_register(int fd, struct auction_data *auction);
void mapif_parse_auction_register(int fd);
void mapif_auction_cancel(int fd, int char_id, unsigned char result);
void mapif_parse_auction_cancel(int fd);
void mapif_auction_close(int fd, int char_id, unsigned char result);
void mapif_parse_auction_close(int fd);
void mapif_auction_bid(int fd, int char_id, int bid, unsigned char result);
void mapif_parse_auction_bid(int fd);

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
	mapif->auction_message = mapif_auction_message;
	mapif->auction_sendlist = mapif_auction_sendlist;
	mapif->parse_auction_requestlist = mapif_parse_auction_requestlist;
	mapif->auction_register = mapif_auction_register;
	mapif->parse_auction_register = mapif_parse_auction_register;
	mapif->auction_cancel = mapif_auction_cancel;
	mapif->parse_auction_cancel = mapif_parse_auction_cancel;
	mapif->auction_close = mapif_auction_close;
	mapif->parse_auction_close = mapif_parse_auction_close;
	mapif->auction_bid = mapif_auction_bid;
	mapif->parse_auction_bid = mapif_parse_auction_bid;
}
