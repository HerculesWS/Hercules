/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#ifndef LOGIN_IPBAN_P_H
#define LOGIN_IPBAN_P_H

#include "ipban.h"

struct ipban_config {
	// Sql settings
	uint16 db_port;
	char   db_hostname[32];
	char   db_username[32];
	char   db_password[100];
	char   db_database[32];
	char   db_codepage[32];
	char   db_table[32];
	uint32 cleanup_interval;                  ///< interval (in seconds) to clean up expired IP bans
	bool enabled;                                   ///< perform IP blocking (via contents of `ipbanlist`) ?
	bool dynamic_pass_failure_ban;                  ///< automatic IP blocking due to failed login attemps ?
	uint32 dynamic_pass_failure_ban_interval;       ///< how far to scan the loginlog for password failures
	uint32 dynamic_pass_failure_ban_limit;          ///< number of failures needed to trigger the ipban
	uint32 dynamic_pass_failure_ban_duration;       ///< duration of the ipban
};

// The login ipban automatic ban private interface
struct ipban_interface_private {
	struct ipban_config *config;

	struct Sql *sql_handle;
	int cleanup_timer_id;
	bool inited;

	int (*cleanup) (int tid, int64 tick, int id, intptr_t data);
	bool (*config_read_inter) (const char *filename, bool imported);
	bool (*config_read_connection) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read_dynamic) (const char *filename, struct config_t *config, bool imported);
};
#endif
