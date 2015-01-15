// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_LOGINIF_H
#define CHAR_LOGINIF_H

#include "char.h"

/**
 * loginif interface
 **/
struct loginif_interface {
	void (*init) (void);
	void (*final) (void);
	void (*reset) (void);
	void (*check_shutdown) (void);
	void (*on_disconnect) (void);
	void (*on_ready) (void);
	void (*block_account) (int account_id, int flag);
	void (*ban_account) (int account_id, short year, short month, short day, short hour, short minute, short second);
	void (*unban_account) (int account_id);
	void (*changesex) (int account_id);
	void (*auth) (int fd, struct char_session_data* sd, uint32 ipl);
	void (*send_users_count) (int users);
	void (*connect_to_server) (void);
};

struct loginif_interface *loginif;

#ifdef HERCULES_CORE
void loginif_defaults(void);
#endif // HERCULES_CORE

#endif /* CHAR_LOGINIF_H */
