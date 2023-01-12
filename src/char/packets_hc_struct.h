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
#ifndef CHAR_PACKETS_HC_STRUCT_H
#define CHAR_PACKETS_HC_STRUCT_H

#include "common/hercules.h"
#include "common/mmo.h"
#include "common/packetsstatic_len.h"

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_HC_ACK_CHARINFO_PER_PAGE {
	int16 packetId;
	int16 packetLen;
	// chars list[]
} __attribute__((packed));
DEFINE_PACKET_HEADER(HC_ACK_CHARINFO_PER_PAGE, 0x0b72);
#elif PACKETVER_MAIN_NUM >= 20130522 || PACKETVER_RE_NUM >= 20130327 || defined(PACKETVER_ZERO)
struct PACKET_HC_ACK_CHARINFO_PER_PAGE {
	int16 packetId;
	int16 packetLen;
	// chars list[]
} __attribute__((packed));
DEFINE_PACKET_HEADER(HC_ACK_CHARINFO_PER_PAGE, 0x099d);
#endif

#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
#define MAX_CHAR_BUF (PACKET_LEN_0x0b6f - 2)
#else  // PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
#define MAX_CHAR_BUF (PACKET_LEN_0x006d - 2)
#endif  // // PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

// because no structs set packet ids in define
#if PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
DEFINE_PACKET_ID(HC_ACCEPT_MAKECHAR, 0x0b6f)
DEFINE_PACKET_ID(HC_ACK_CHANGE_CHARACTER_SLOT, 0x0b70)
DEFINE_PACKET_ID(HC_UPDATE_CHARINFO, 0x0b71)
#else  // PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
DEFINE_PACKET_ID(HC_ACCEPT_MAKECHAR, 0x006d)
DEFINE_PACKET_ID(HC_ACK_CHANGE_CHARACTER_SLOT, 0x08d5)
DEFINE_PACKET_ID(HC_UPDATE_CHARINFO, 0x08e3)
#endif  // // PACKETVER_MAIN_NUM >= 20201007 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif // CHAR_PACKETS_HC_STRUCT_H
