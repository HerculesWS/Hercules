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
#ifndef LOGIN_PACKETS_AC_STRUCT_H
#define LOGIN_PACKETS_AC_STRUCT_H

#include "common/hercules.h"
#include "common/mmo.h"
#include "common/packetsstatic_len.h"

/* Enums */

/// Packet IDs
enum login_ac_packet_id {
	HEADER_AC_ACCEPT_LOGIN         = 0x0069,
	HEADER_AC_ACCEPT_LOGIN2        = 0x0ac4,
	HEADER_AC_REFUSE_LOGIN         = 0x006a,
	HEADER_SC_NOTIFY_BAN           = 0x0081,
	HEADER_AC_ACK_HASH             = 0x01dc,
	HEADER_AC_REFUSE_LOGIN_R2      = 0x083e,
	HEADER_AC_REFUSE_LOGIN_R3      = 0x0b02,
};

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

/**
 * Packet structure for SC_NOTIFY_BAN.
 */
struct PACKET_SC_NOTIFY_BAN {
	int16 packet_id;  ///< Packet ID (#HEADER_SC_NOTIFY_BAN)
	uint8 error_code; ///< Error code
} __attribute__((packed));

/**
 * Packet structure for AC_REFUSE_LOGIN.
 */
struct PACKET_AC_REFUSE_LOGIN {
	int16 packet_id;     ///< Packet ID (#HEADER_AC_REFUSE_LOGIN)
	uint8 error_code;    ///< Error code
	char block_date[20]; ///< Ban expiration date
} __attribute__((packed));

/**
 * Packet structure for AC_REFUSE_LOGIN_R2.
 */
struct PACKET_AC_REFUSE_LOGIN_R2 {
	int16 packet_id;     ///< Packet ID (#HEADER_AC_REFUSE_LOGIN_R2)
	uint32 error_code;   ///< Error code
	char block_date[20]; ///< Ban expiration date
} __attribute__((packed));

#define AUTH_TOKEN_SIZE 16

/**
 * Packet structure for AC_ACCEPT_LOGIN.
 *
 * Variable-length packet.
 */
struct PACKET_AC_ACCEPT_LOGIN {
	int16 packet_id;          ///< Packet ID (#HEADER_AC_ACCEPT_LOGIN)
	int16 packet_len;         ///< Packet length (variable length)
	int32 auth_code;          ///< Authentication code
	uint32 aid;               ///< Account ID
	uint32 user_level;        ///< User level
	uint32 last_login_ip;     ///< Last login IP
	char last_login_time[26]; ///< Last login timestamp
	uint8 sex;                ///< Account sex
#if PACKETVER >= 20170315
	unsigned char auth_token[AUTH_TOKEN_SIZE];
	uint8 twitter_flag;
#endif
	struct {
		uint32 ip;        ///< Server IP address
		int16 port;       ///< Server port
		char name[MAX_CHARSERVER_NAME_SIZE];  ///< Server name
		uint16 usercount; ///< Online users
		uint16 state;     ///< Server state
		uint16 property;  ///< Server property
#if PACKETVER >= 20170315
		char unknown2[128];
#endif
	} server_list[];          ///< List of charservers
} __attribute__((packed));

/**
 * Packet structure for AC_ACK_HASH.
 *
 * Variable-length packet
 */
struct PACKET_AC_ACK_HASH {
	int16 packet_id;  ///< Packet ID (#HEADER_AC_ACK_HASH)
	int16 packet_len; ///< Packet length (variable length)
	uint8 secret[];   ///< Challenge string
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20181114 || PACKETVER_RE_NUM >= 20181114 || defined(PACKETVER_ZERO)
struct PACKET_AC_REQ_MOBILE_OTP {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_SSO_LOGIN_REQ)
	uint32 aid;           ///< Account ID
} __attribute__((packed));
DEFINE_PACKET_HEADER(AC_REQ_MOBILE_OTP, 0x09a2);
#endif

#if PACKETVER_MAIN_NUM >= 20171213 || PACKETVER_RE_NUM >= 20171213 || PACKETVER_ZERO_NUM >= 20171808
// AC_LOGIN_OTP2
struct PACKET_AC_LOGIN_OTP {
	int16 packet_id;
	int16 packet_len;
	int32 loginFlag;
	char loginFlag2[20];
	char token[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(AC_LOGIN_OTP, 0x0ae3);
#elif PACKETVER_ZERO_NUM >= 20171123
// AC_LOGIN_OTP2
struct PACKET_AC_LOGIN_OTP {
	int16 packet_id;
	int16 packet_len;
	int32 loginFlag;
	char loginFlag2[6];
	char token[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(AC_LOGIN_OTP, 0x0ae3);
#elif PACKETVER_MAIN_NUM >= 20170621 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
// AC_LOGIN_OTP1
struct PACKET_AC_LOGIN_OTP {
	int16 packet_id;
	int16 packet_len;
	int32 loginFlag;
	char token[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(AC_LOGIN_OTP, 0x0ad1);
#endif

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif // LOGIN_PACKETS_AC_STRUCT_H
