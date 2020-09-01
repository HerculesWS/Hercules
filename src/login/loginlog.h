/**
 * This file is part of Hercules.
 * https://herc.ws - https://github.com/HerculesWS/Hercules
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef LOGIN_LOGINLOG_H
#define LOGIN_LOGINLOG_H

#include "common/hercules.h"
#include "common/cbasetypes.h"

struct config_t;

struct s_loginlog_dbs {
	char log_db_hostname[32];
	uint16 log_db_port;
	char log_db_username[32];
	char log_db_password[100];
	char log_db_database[32];
	char log_codepage[32];
	char log_login_db[256];
};

/**
 * Loginlog.c Interface
 **/
struct loginlog_interface {
	struct Sql *sql_handle;
	bool enabled;
	struct s_loginlog_dbs *dbs;
	unsigned long (*failedattempts) (uint32 ip, unsigned int minutes);
	void (*log) (uint32 ip, const char* username, int rcode, const char* message);
	bool (*init) (void);
	bool (*final) (void);
	bool (*config_read_names) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read_log) (const char *filename, struct config_t *config, bool imported);
	bool (*config_read) (const char *filename, bool imported);
};

#ifdef HERCULES_CORE
void loginlog_defaults(void);
#endif // HERCULES_CORE

HPShared struct loginlog_interface *loginlog;

#endif /* LOGIN_LOGINLOG_H */
