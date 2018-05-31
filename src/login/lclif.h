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

/** @file
 * Login Client Interface.
 **/

/* Forward Declarations */
struct login_session_data;
struct lclif_interface_private;

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
	int16 len;              ///< Packet length
	LoginParseFunc **pFunc; ///< Packet parsing function
};

/// The login clif (client interface) interface.
struct lclif_interface {
	struct lclif_interface_private *p; ///< Private interface

	/// Interface initialization.
	void (*init)(void);

	/// Interface finalization.
	void (*final)(void);

	/**
	 * Reports a connection error to the client.
	 *
	 * @param fd    Client connection file descriptor.
	 * @param error Error code.
	 * @see #PACKET_SC_NOTIFY_BAN.
	 */
	void (*connection_error)(int fd, uint8 error);

	/**
	 * Sends the character server list to the client.
	 *
	 * @param sd The client to send to.
	 * @return Success status.
	 * @retval false in case of failure (no server available).
	 * @see #PACKET_AC_ACCEPT_LOGIN.
	 */
	bool (*server_list)(struct login_session_data *sd);

	/**
	 * Reports an authentication failure to the client.
	 *
	 * @param fd    The client connection file descriptor.
	 * @param ban   The ban duration (if error == 6).
	 * @param error The authentication error code.
	 * @see #PACKET_ID_AC_REFUSE_LOGIN.
	 * @see #PACKET_ID_AC_REFUSE_LOGIN_R2.
	 */
	void (*auth_failed)(int fd, time_t ban, uint32 error);

	/**
	 * Reports a login error to the client.
	 *
	 * @param fd    Client connection file descriptor.
	 * @param error Error code.
	 * @see #PACKET_AC_REFUSE_LOGIN.
	 */
	void (*login_error)(int fd, uint8 error);

	/**
	 * Sends an authentication challenge to the client.
	 *
	 * @param fd Client connection file descriptor.
	 * @param sd The client to send to.
	 * @see #PACKET_AC_ACK_HASH.
	 */
	void (*coding_key)(int fd, struct login_session_data *sd);

	/**
	 * Retrieves a packet's data from the packet db.
	 *
	 * @param packet_id The packet id.
	 * @return The packet data.
	 * @retval NULL if the packet doesn't exist.
	 */
	const struct login_packet_db *(*packet)(int16 packet_id);

	/**
	 * Parses a packet.
	 *
	 * @param lpd Packet database entry.
	 * @param fd  Client connection file descriptor.
	 * @param sd  Session data.
	 * @return Parse result error code.
	 */
	enum parsefunc_rcode (*parse_packet)(const struct login_packet_db *lpd, int fd, struct login_session_data *sd);

	/**
	 * Packet parser loop function.
	 *
	 * Parses packets received from a client.
	 *
	 * @param fd Client connection file descriptor.
	 * @return error code.
	 * @retval 0 in case of success.
	 */
	int (*parse)(int fd);
};

#ifdef HERCULES_CORE
void lclif_defaults(void);
#endif

HPShared struct lclif_interface *lclif;

#endif // LOGIN_LCLIF_H
