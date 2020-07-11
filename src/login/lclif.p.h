/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016-2023 Hercules Dev Team
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
#ifndef LOGIN_LCLIF_P_H
#define LOGIN_LCLIF_P_H

/** @file
 * Private header for the login client interface.
 */

#include "login/lclif.h"

#include "common/hercules.h"
#include "common/mmo.h"

/* Definitions and macros */
/// Maximum amount of packets processed at once from the same client
#ifndef MAX_PROCESSED_PACKETS
#define MAX_PROCESSED_PACKETS (3)
#endif

// Packet DB
#ifndef MIN_PACKET_DB
#define MIN_PACKET_DB 0x0064
#endif
#ifndef MAX_PACKET_LOGIN_DB
#define MAX_PACKET_LOGIN_DB 0x0ad0
#endif

/**
 * Login Client Interface additional data
 */
struct lclif_interface_dbs {
	struct login_packet_db packet_db[MAX_PACKET_LOGIN_DB + 1]; ///< Packet database.
};

/**
 * Login Client Interface Private Interface
 */
struct lclif_interface_private {
	struct lclif_interface_dbs *dbs;

	/**
	 * Populates the packet database.
	 */
	void (*packetdb_loaddb)(void);

	/**
	 * Attempts to validate and parse a received packet.
	 *
	 * @param fd  Client connection file descriptor.
	 * @param sd  Session data.
	 * @return Parse result error code.
	 */
	enum parsefunc_rcode (*parse_sub)(int fd, struct login_session_data *sd);

	LoginParseFunc *parse_CA_CONNECT_INFO_CHANGED; ///< Packet handler for #PACKET_CA_CONNECT_INFO_CHANGED.
	LoginParseFunc *parse_CA_EXE_HASHCHECK;        ///< Packet handler for #PACKET_CA_EXE_HASHCHECK.
	LoginParseFunc *parse_CA_LOGIN;                ///< Packet handler for #PACKET_CA_LOGIN.
	LoginParseFunc *parse_CA_LOGIN2;               ///< Packet handler for #PACKET_CA_LOGIN2.
	LoginParseFunc *parse_CA_LOGIN3;               ///< Packet handler for #PACKET_CA_LOGIN3.
	LoginParseFunc *parse_CA_LOGIN4;               ///< Packet handler for #PACKET_CA_LOGIN4.
	LoginParseFunc *parse_CA_LOGIN_PCBANG;         ///< Packet handler for #PACKET_CA_LOGIN_PCBANG.
	LoginParseFunc *parse_CA_LOGIN_HAN;            ///< Packet handler for #PACKET_CA_LOGIN_HAN.
	LoginParseFunc *parse_CA_SSO_LOGIN_REQ;        ///< Packet handler for #PACKET_CA_SSO_LOGIN_REQ.
	LoginParseFunc *parse_CA_LOGIN_OTP;            ///< Packet handler for #PACKET_CA_LOGIN_OTP.
	LoginParseFunc *parse_CA_ACK_MOBILE_OTP;       ///< Packet handler for #PACKET_CA_ACK_MOBILE_OTP.
	LoginParseFunc *parse_CA_OTP_CODE;             ///< Packet handler for #PACKET_CA_OTP_CODE.
	LoginParseFunc *parse_CA_REQ_HASH;             ///< Packet handler for #PACKET_CA_REQ_HASH.
	LoginParseFunc *parse_CA_CHARSERVERCONNECT;    ///< Packet handler for #PACKET_CA_CHARSERVERCONNECT.
	LoginParseFunc *parse_CA_APISERVERCONNECT;     ///< Packet handler for #PACKET_CA_APISERVERCONNECT.
};

#endif // LOGIN_LCLIF_P_H
