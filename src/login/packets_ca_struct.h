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
#ifndef LOGIN_PACKETS_CA_STRUCT_H
#define LOGIN_PACKETS_CA_STRUCT_H

#include "common/hercules.h"
#include "common/mmo.h"
#include "common/packetsstatic_len.h"

/* Enums */

/// Packet IDs
enum login_packet_ca_id {
	HEADER_CA_LOGIN                = 0x0064,
	HEADER_CA_LOGIN2               = 0x01dd,
	HEADER_CA_LOGIN3               = 0x01fa,
	HEADER_CA_CONNECT_INFO_CHANGED = 0x0200,
	HEADER_CA_EXE_HASHCHECK        = 0x0204,
	HEADER_CA_LOGIN_PCBANG         = 0x0277,
	HEADER_CA_LOGIN4               = 0x027c,
	HEADER_CA_LOGIN_HAN            = 0x02b0,
	HEADER_CA_SSO_LOGIN_REQ        = 0x0825,
	HEADER_CA_LOGIN_OTP            = 0x0acf,
	HEADER_CA_REQ_HASH             = 0x01db,
	HEADER_CA_CHARSERVERCONNECT    = 0x2710, // Custom Hercules Packet
	HEADER_CA_APISERVERCONNECT     = 0x2720, // Custom Hercules Packet
	//HEADER_CA_SSO_LOGIN_REQa       = 0x825a, /* unused */
};

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

/**
 * Packet structure for CA_LOGIN.
 */
struct PACKET_CA_LOGIN {
	int16 packet_id;   ///< Packet ID (#HEADER_CA_LOGIN)
	uint32 version;    ///< Client Version
	char id[24];       ///< Username
	char password[24]; ///< Password
	uint8 clienttype;  ///< Client Type
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN2.
 */
struct PACKET_CA_LOGIN2 {
	int16 packet_id;        ///< Packet ID (#HEADER_CA_LOGIN2)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN3.
 */
struct PACKET_CA_LOGIN3 {
	int16 packet_id;        ///< Packet ID (#HEADER_CA_LOGIN3)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
	uint8 clientinfo;       ///< Index of the connection in the clientinfo file (+10 if the command-line contains "pc")
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN4.
 */
struct PACKET_CA_LOGIN4 {
	int16 packet_id;        ///< Packet ID (#HEADER_CA_LOGIN4)
	uint32 version;         ///< Client Version
	char id[24];            ///< Username
	uint8 password_md5[16]; ///< Password hash
	uint8 clienttype;       ///< Client Type
	char mac_address[13];   ///< MAC Address
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN_PCBANG.
 */
struct PACKET_CA_LOGIN_PCBANG {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_LOGIN_PCBANG)
	uint32 version;       ///< Client Version
	char id[24];          ///< Username
	char password[24];    ///< Password
	uint8 clienttype;     ///< Client Type
	char ip[16];          ///< IP Address
	char mac_address[13]; ///< MAC Address
} __attribute__((packed));

/**
 * Packet structure for CA_LOGIN_HAN.
 */
struct PACKET_CA_LOGIN_HAN {
	int16 packet_id;        ///< Packet ID (#HEADER_CA_LOGIN_HAN)
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
struct PACKET_CA_SSO_LOGIN_REQ {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_SSO_LOGIN_REQ)
	int16 packet_len;     ///< Length (variable length)
	uint32 version;       ///< Clientver
	uint8 clienttype;     ///< Clienttype
	char id[24];          ///< Username
	char password[27];    ///< Password
	int8 mac_address[17]; ///< MAC Address
	char ip[15];          ///< IP Address
	char t1[];            ///< SSO Login Token (variable length)
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20181114 || PACKETVER_RE_NUM >= 20181114
/**
 * Packet structure for CA_SSO_LOGIN_REQ.
 *
 * Variable-length packet.
 */
struct PACKET_CA_ACK_MOBILE_OTP {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_ACK_MOBILE_OTP)
	int16 packet_len;     ///< Length (variable length)
	uint32 aid;           ///< Account ID
	char code[6];         ///< Code
} __attribute__((packed));
DEFINE_PACKET_HEADER(CA_ACK_MOBILE_OTP, 0x09a3);
#endif

#if PACKETVER_MAIN_NUM >= 20181114 || PACKETVER_RE_NUM >= 20181114 || defined(PACKETVER_ZERO)
struct PACKET_CA_OTP_CODE {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_OTP_CODE)
	char code[9];         ///< Code
} __attribute__((packed));
DEFINE_PACKET_HEADER(CA_OTP_CODE, 0x0ad0);
#endif

/**
 * Packet structure for CA_LOGIN_OTP.
 */
struct PACKET_CA_LOGIN_OTP {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_LOGIN_OTP)
#if PACKETVER >= 20171113
	uint32 devFlags;      ///< flags including dev flag
#endif
	char login[25];       ///< Username
	char password[32];    ///< Password encrypted by rijndael
	char flagsStr[5];     ///< Unknown flags. Normally string: G000
} __attribute__((packed));

#if 0 // Unused
struct PACKET_CA_SSO_LOGIN_REQa {
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
struct PACKET_CA_CONNECT_INFO_CHANGED {
	int16 packet_id; ///< Packet ID (#HEADER_CA_CONNECT_INFO_CHANGED)
	char id[24];    ///< account.userid
} __attribute__((packed));

/**
 * Packet structure for CA_EXE_HASHCHECK.
 *
 * (kRO 2004-05-31aSakexe langtype 0 and 6)
 */
struct PACKET_CA_EXE_HASHCHECK {
	int16 packet_id;      ///< Packet ID (#HEADER_CA_EXE_HASHCHECK)
	uint8 hash_value[16]; ///< Client MD5 hash
} __attribute__((packed));

/**
 * Packet structure for CA_REQ_HASH.
 */
struct PACKET_CA_REQ_HASH {
	int16 packet_id; ///< Packet ID (#HEADER_CA_REQ_HASH)
} __attribute__((packed));

/**
 * Packet structure for CA_CHARSERVERCONNECT.
 *
 * This packet is used internally, to signal a char-server connection.
 */
struct PACKET_CA_CHARSERVERCONNECT {
	int16 packet_id;   ///< Packet ID (#HEADER_CA_CHARSERVERCONNECT)
	char userid[24];   ///< Username
	char password[24]; ///< Password
	int32 unknown;
	int32 ip;          ///< Charserver IP
	int16 port;        ///< Charserver port
	char name[MAX_CHARSERVER_NAME_SIZE];  ///< Charserver name
	int16 unknown2;
	int16 type;        ///< Charserver type
	int16 new;         ///< Whether charserver is to be marked as new
} __attribute__((packed));

/**
 * Packet structure for CA_APISERVERCONNECT.
 *
 * This packet is used internally, to signal a api-server connection.
 */
struct PACKET_CA_APISERVERCONNECT {
	int16 packet_id;   ///< Packet ID (#HEADER_CA_APISERVERCONNECT)
	char userid[24];   ///< Username
	char password[24]; ///< Password
} __attribute__((packed));

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif // LOGIN_PACKETS_CA_STRUCT_H
