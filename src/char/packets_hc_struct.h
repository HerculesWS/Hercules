/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016-2020 Hercules Dev Team
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

#if PACKETVER_MAIN_NUM >= 20130522 || PACKETVER_RE_NUM >= 20130327 || defined(PACKETVER_ZERO)
struct PACKET_HC_ACK_CHARINFO_PER_PAGE {
	int16 packetId;
	int16 packetLen;
	// chars list[]
} __attribute__((packed));
DEFINE_PACKET_HEADER(HC_ACK_CHARINFO_PER_PAGE, 0x099d);
#endif

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif // CHAR_PACKETS_HC_STRUCT_H
