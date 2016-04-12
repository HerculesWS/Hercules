/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016  Hercules Dev Team
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
#ifndef LOGIN_LCLIF_H
#define LOGIN_LCLIF_H

#include "common/hercules.h"

/* Forward Declarations */
struct login_session_data;

/* Enums */
/// Parse function return code
enum parsefunc_rcode {
	PACKET_VALID         =  1,
	PACKET_INCOMPLETE    =  0,
	PACKET_UNKNOWN       = -1,
	PACKET_INVALIDLENGTH = -2,
	PACKET_STOPPARSE     = -3,
	PACKET_SKIP          = -4, //internal parser will skip this packet and go parser another, meant for plugins. [hemagx]
};

/* Function Typedefs */
typedef enum parsefunc_rcode (LoginParseFunc)(int fd, struct login_session_data *sd);

/* Structs */
/// Login packet DB entry
struct login_packet_db {
	int16 len;             ///< Packet length
	LoginParseFunc *pFunc; ///< Packet parsing function
};

struct lclif_interface {
	void (*init)(void);
	void (*final)(void);

	void (*connection_error)(int fd, uint8 error);
	bool (*server_list)(struct login_session_data *sd);
	void (*auth_failed)(int fd, time_t ban, uint32 error);
	void (*login_error)(int fd, uint8 error);
	void (*coding_key)(int fd, struct login_session_data *sd);
	const struct login_packet_db *(*packet)(int16 packet_id);
	enum parsefunc_rcode (*parse_packet)(const struct login_packet_db *lpd, int fd, struct login_session_data *sd);
	int (*parse)(int fd);
	enum parsefunc_rcode (*parse_sub)(int fd, struct login_session_data *sd);
};

#ifdef HERCULES_CORE
void lclif_defaults(void);
#endif

HPShared struct lclif_interface *lclif;

#endif // LOGIN_LCLIF_H
