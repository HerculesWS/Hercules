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
#define MAX_PROCESSED_PACKETS (3)

// Packet DB
#define MIN_PACKET_DB 0x0064
#define MAX_PACKET_DB 0x08ff

/* Enums */

/// Packet IDs
enum login_packet_id {
	// CA (Client to Login)
	PACKET_ID_CA_LOGIN                = 0x0064,
	PACKET_ID_CA_LOGIN2               = 0x01dd,
	PACKET_ID_CA_LOGIN3               = 0x01fa,
	PACKET_ID_CA_CONNECT_INFO_CHANGED = 0x0200,
	PACKET_ID_CA_EXE_HASHCHECK        = 0x0204,
	PACKET_ID_CA_LOGIN_PCBANG         = 0x0277,
	PACKET_ID_CA_LOGIN4               = 0x027c,
	PACKET_ID_CA_LOGIN_HAN            = 0x02b0,
	PACKET_ID_CA_SSO_LOGIN_REQ        = 0x0825,
	PACKET_ID_CA_REQ_HASH             = 0x01db,
	PACKET_ID_CA_CHARSERVERCONNECT    = 0x2710, // Custom Hercules Packet
	//PACKET_ID_CA_SSO_LOGIN_REQa       = 0x825a, /* unused */

	// AC (Login to Client)

	PACKET_ID_AC_ACCEPT_LOGIN         = 0x0069,
	PACKET_ID_AC_ACCEPT_LOGIN2        = 0x0ac4,
	PACKET_ID_AC_REFUSE_LOGIN         = 0x006a,
	PACKET_ID_SC_NOTIFY_BAN           = 0x0081,
	PACKET_ID_AC_ACK_HASH             = 0x01dc,
	PACKET_ID_AC_REFUSE_LOGIN_R2      = 0x083e,
};

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

/**
 * Packet structure for CA_LOGIN.
 */
struct packet_CA_LOGIN {
	int16 packet_id;   ///< Packet ID (#PACKET_ID_CA_LOGIN)
	uint32 version;    ///< Client Version
	char id[24];       ///< Username
	char password[24]; ///< Password
	uint8 clienttype;  ///< Client Type
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN2.
 */
struct packet_CA_LOGIN2 {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN2)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN3.
 */
struct packet_CA_LOGIN3 {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN3)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
	uint8 clientinfo;       ///< Index of the connection in the clientinfo file (+10 if the command-line contains "pc")
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN4.
 */
struct packet_CA_LOGIN4 {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN4)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
	char mac_address[13];   ///< MAC Address
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN_PCBANG.
 */
struct packet_CA_LOGIN_PCBANG {
	int16 packet_id;      ///< Packet ID (#PACKET_ID_CA_LOGIN_PCBANG)
	uint32 version;	      ///< Client Version
	char id[24];          ///< Username
	char password[24];    ///< Password
	uint8 clienttype;     ///< Client Type
	char ip[16];          ///< IP Address
	char mac_address[13]; ///< MAC Address
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN_HAN.
 */
struct packet_CA_LOGIN_HAN {
	int16 packet_id;        ///< Packet ID (#PACKET_ID_CA_LOGIN_HAN)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	char password[24];      ///< Password
	uint8 clienttype;       ///< Client Type
	char ip[16];            ///< IP Address
	char mac_address[13];   ///< MAC Address
	uint8 is_han_game_user; ///< 'isGravityID'
} __attribute__((packed));

/**
 * Packet structure for CA_SSO_LOGIN_REQ.
 *
 * Variable-length packet.
 */
struct packet_CA_SSO_LOGIN_REQ {
	int16 packet_id;      ///< Packet ID (#PACKET_ID_CA_SSO_LOGIN_REQ)
	int16 packet_len;     ///< Length (variable length)
	uint32 version;       ///< Clientver
	uint8 clienttype;     ///< Clienttype
	char id[24];          ///< Username
	char password[27];    ///< Password
	int8 mac_address[17]; ///< MAC Address
	char ip[15];          ///< IP Address
	char t1[];            ///< SSO Login Token (variable length)
} __attribute__((packed));

#if 0 // Unused
struct packet_CA_SSO_LOGIN_REQa {
	int16 packet_id;
	int16 packet_len;
	uint32 version;
	uint8 clienttype;
	char id[24];
	int8 mac_address[17];
	char ip[15];
	char t1[];
} __attribute__((packed));
#endif // unused

/**
 * Packet structure for CA_CONNECT_INFO_CHANGED.
 *
 * New alive packet. Used to verify if client is always alive.
 */
struct packet_CA_CONNECT_INFO_CHANGED {
	int16 packet_id; ///< Packet ID (#PACKET_ID_CA_CONNECT_INFO_CHANGED)
	char id[24];    ///< account.userid
} __attribute__((packed));

/**
 * Packet structure for CA_EXE_HASHCHECK.
 *
 * (kRO 2004-05-31aSakexe langtype 0 and 6)
 */
struct packet_CA_EXE_HASHCHECK {
	int16 packet_id;      ///< Packet ID (#PACKET_ID_CA_EXE_HASHCHECK)
	uint8 hash_value[16]; ///< Client MD5 hash
} __attribute__((packed));

/**
 * Packet structure for CA_REQ_HASH.
 */
struct packet_CA_REQ_HASH {
	int16 packet_id; ///< Packet ID (#PACKET_ID_CA_REQ_HASH)
} __attribute__((packed));

/**
 * Packet structure for CA_CHARSERVERCONNECT.
 *
 * This packet is used internally, to signal a char-server connection.
 */
struct packet_CA_CHARSERVERCONNECT {
	int16 packet_id;   ///< Packet ID (#PACKET_ID_CA_CHARSERVERCONNECT)
	char userid[24];   ///< Username
	char password[24]; ///< Password
	int32 unknown;
	int32 ip;          ///< Charserver IP
	int16 port;        ///< Charserver port
	char name[20];     ///< Charserver name
	int16 unknown2;
	int16 type;        ///< Charserver type
	int16 new;         ///< Whether charserver is to be marked as new
} __attribute__((packed));

/**
 * Packet structure for SC_NOTIFY_BAN.
 */
struct packet_SC_NOTIFY_BAN {
	int16 packet_id;  ///< Packet ID (#PACKET_ID_SC_NOTIFY_BAN)
	uint8 error_code; ///< Error code
} __attribute__((packed));

/**
 * Packet structure for AC_REFUSE_LOGIN.
 */
struct packet_AC_REFUSE_LOGIN {
	int16 packet_id;     ///< Packet ID (#PACKET_ID_AC_REFUSE_LOGIN)
	uint8 error_code;    ///< Error code
	char block_date[20]; ///< Ban expiration date
} __attribute__((packed));

/**
 * Packet structure for AC_REFUSE_LOGIN_R2.
 */
struct packet_AC_REFUSE_LOGIN_R2 {
	int16 packet_id;     ///< Packet ID (#PACKET_ID_AC_REFUSE_LOGIN_R2)
	uint32 error_code;   ///< Error code
	char block_date[20]; ///< Ban expiration date
} __attribute__((packed));

/**
 * Packet structure for AC_ACCEPT_LOGIN.
 *
 * Variable-length packet.
 */
struct packet_AC_ACCEPT_LOGIN {
	int16 packet_id;          ///< Packet ID (#PACKET_ID_AC_ACCEPT_LOGIN)
	int16 packet_len;         ///< Packet length (variable length)
	int32 auth_code;          ///< Authentication code
	uint32 aid;               ///< Account ID
	uint32 user_level;        ///< User level
	uint32 last_login_ip;     ///< Last login IP
	char last_login_time[26]; ///< Last login timestamp
	uint8 sex;                ///< Account sex
#if PACKETVER >= 20170315
	char unknown1[17];
#endif
	struct {
		uint32 ip;        ///< Server IP address
		int16 port;       ///< Server port
		char name[20];    ///< Server name
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
struct packet_AC_ACK_HASH {
	int16 packet_id;  ///< Packet ID (#PACKET_ID_AC_ACK_HASH)
	int16 packet_len; ///< Packet length (variable length)
	uint8 secret[];   ///< Challenge string
} __attribute__((packed));

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

/**
 * Login Client Interface additional data
 */
struct lclif_interface_dbs {
	struct login_packet_db packet_db[MAX_PACKET_DB + 1]; ///< Packet database.
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

	LoginParseFunc *parse_CA_CONNECT_INFO_CHANGED; ///< Packet handler for #packet_CA_CONNECT_INFO_CHANGED.
	LoginParseFunc *parse_CA_EXE_HASHCHECK;        ///< Packet handler for #packet_CA_EXE_HASHCHECK.
	LoginParseFunc *parse_CA_LOGIN;                ///< Packet handler for #packet_CA_LOGIN.
	LoginParseFunc *parse_CA_LOGIN2;               ///< Packet handler for #packet_CA_LOGIN2.
	LoginParseFunc *parse_CA_LOGIN3;               ///< Packet handler for #packet_CA_LOGIN3.
	LoginParseFunc *parse_CA_LOGIN4;               ///< Packet handler for #packet_CA_LOGIN4.
	LoginParseFunc *parse_CA_LOGIN_PCBANG;         ///< Packet handler for #packet_CA_LOGIN_PCBANG.
	LoginParseFunc *parse_CA_LOGIN_HAN;            ///< Packet handler for #packet_CA_LOGIN_HAN.
	LoginParseFunc *parse_CA_SSO_LOGIN_REQ;        ///< Packet handler for #packet_CA_SSO_LOGIN_REQ.
	LoginParseFunc *parse_CA_REQ_HASH;             ///< Packet handler for #packet_CA_REQ_HASH.
	LoginParseFunc *parse_CA_CHARSERVERCONNECT;    ///< Packet handler for #packet_CA_CHARSERVERCONNECT.
};

#endif // LOGIN_LCLIF_P_H
