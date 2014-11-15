// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "loginif.h"

#include <stdlib.h>

#include "char.h"
#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

void loginif_reset(void);
void loginif_check_shutdown(void);
void loginif_on_disconnect(void);
void loginif_on_ready(void);
void loginif_block_account(int account_id, int flag);
void loginif_ban_account(int account_id, short year, short month, short day, short hour, short minute, short second);
void loginif_unban_account(int account_id);
void loginif_changesex(int account_id);
void loginif_auth(int fd, struct char_session_data* sd, uint32 ipl);
void loginif_send_users_count(int users);
void loginif_connect_to_server(void);

void loginif_defaults(void) {
	loginif = &loginif_s;

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
