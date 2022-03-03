/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2022 Hercules Dev Team
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
#ifndef CHAR_INTER_H
#define CHAR_INTER_H

#include "common/hercules.h"
#include "common/db.h"
#include "common/packets_struct.h"

#include <stdarg.h>

/* Forward Declarations */
struct Sql; // common/sql.h
struct config_t; // common/conf.h

/**
 * inter interface
 **/
struct inter_interface {
	bool enable_logs; ///< Whether to log inter-server operations.
	struct Sql *sql_handle;
	const char* (*msg_txt) (int msg_number);
	bool (*msg_config_read) (const char *cfg_name, bool allow_override);
	void (*do_final_msg) (void);
	const char* (*job_name) (int class);
	void (*vmsg_to_fd) (int fd, int u_fd, int aid, char* msg, va_list ap) __attribute__((format(printf, 4, 0)));
	void (*msg_to_fd) (int fd, int u_fd, int aid, char *msg, ...) __attribute__((format(printf, 4, 5)));
	void (*savereg) (int account_id, int char_id, const char *key, unsigned int index, intptr_t val, bool is_string);
	int (*accreg_fromsql) (int account_id,int char_id, int fd, int type);
	int (*vlog) (char* fmt, va_list ap) __attribute__((format(printf, 1, 0)));
	int (*log) (char* fmt, ...) __attribute__((format(printf, 1, 2)));
	int (*init_sql) (const char *file);
	int (*mapif_init) (int fd);
	int (*check_length) (int fd, int length);
	int (*parse_frommap) (int fd);
	void (*final) (void);
	bool (*config_read) (const char *filename, bool imported);
	bool (*config_read_log) (const char *filename, const struct config_t *config, bool imported);
	bool (*config_read_connection) (const char *filename, const struct config_t *config, bool imported);
	void (*accinfo) (int u_fd, int aid, int castergroup, const char *query, int map_fd);
	void (*accinfo2) (bool success, int map_fd, int u_fd, int u_aid, int account_id, const char *userid, const char *user_pass,
			const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate,
			int group_id, int logincount, int state);
};

#ifdef HERCULES_CORE
extern int party_share_level; ///< Share range for parties.

void inter_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_interface *inter;

#endif /* CHAR_INTER_H */
