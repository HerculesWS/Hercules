/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
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
#ifndef CHAR_PINCODE_H
#define CHAR_PINCODE_H

#include "common/hercules.h"
#include "common/db.h"

/* Forward Declarations */
struct char_session_data;
struct config_t; // common/conf.h

enum pincode_make_response {
	PINCODE_MAKE_SUCCESS        = 0,
	PINCODE_MAKE_DUPLICATED     = 1,
	PINCODE_MAKE_RESTRICT_PW    = 2,
	PINCODE_MAKE_PERSONALNUM_PW = 3,
	PINCODE_MAKE_FAILED         = 4,
};

enum pincode_edit_response {
	PINCODE_EDIT_SUCCESS        = 0x0,
	PINCODE_EDIT_FAILED         = 0x1,
	PINCODE_EDIT_RESTRICT_PW    = 0x2,
	PINCODE_EDIT_PERSONALNUM_PW = 0x3,
};

enum pincode_login_response {
	PINCODE_LOGIN_OK          = 0,
	PINCODE_LOGIN_ASK         = 1,
	PINCODE_LOGIN_NOTSET      = 2,
	PINCODE_LOGIN_EXPIRED     = 3,
	PINCODE_LOGIN_RESTRICT_PW = 5,
	PINCODE_LOGIN_UNUSED      = 7,
	PINCODE_LOGIN_WRONG       = 8,
};

enum pincode_login_response2 {
	PINCODE_LOGIN_FLAG_LOCKED = 0,
	PINCODE_LOGIN_FLAG_WRONG  = 2,
};

/**
 * pincode interface
 **/
struct pincode_interface {
	/* vars */
	int enabled;
	int changetime;
	int maxtry;
	int charselect;
	bool check_blacklist;
	VECTOR_DECL(char *) blacklist;
	unsigned int multiplier;
	unsigned int baseSeed;
	/* handler */
	void (*handle) (int fd, struct char_session_data* sd);
	void (*decrypt) (unsigned int userSeed, char* pin);
	void (*error) (int account_id);
	void (*update) (int account_id, char* pin);
	void (*makestate) (int fd, struct char_session_data *sd, enum pincode_make_response state);
	void (*editstate) (int fd, struct char_session_data *sd, enum pincode_edit_response state);
	void (*loginstate) (int fd, struct char_session_data *sd, enum pincode_login_response state);
	void (*loginstate2) (int fd, struct char_session_data *sd, enum pincode_login_response state, enum pincode_login_response2 flag);
	void (*setnew) (int fd, struct char_session_data* sd);
	void (*change) (int fd, struct char_session_data* sd);
	bool (*isBlacklisted) (const char *pin);
	int  (*compare) (int fd, struct char_session_data* sd, char* pin);
	void (*check) (int fd, struct char_session_data* sd);
	bool (*config_read) (const char *filename, const struct config_t *config, bool imported);
	void (*init) (void);
	void (*final) (void);
};

#ifdef HERCULES_CORE
void pincode_defaults(void);
#endif // HERCULES_CORE

HPShared struct pincode_interface *pincode;

#endif /* CHAR_PINCODE_H */
