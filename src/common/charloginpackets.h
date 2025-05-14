/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2025 Hercules Dev Team
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
#ifndef COMMON_CHARLOGINPACKETS_H
#define COMMON_CHARLOGINPACKETS_H

// Packets sent by Char-Server to Login-Server

#include "common/hercules.h"
#include "common/packetsmacro.h"

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
	#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

struct PACKET_CHARLOGIN_SET_ACCOUNT_ONLINE {
	int16 packetType;
	int account_id;
	uint8 standalone; // 0 - real player (false) / 1 - standalone/server generated (true)
} __attribute__((packed));
DEFINE_PACKET_ID(CHARLOGIN_SET_ACCOUNT_ONLINE, 0x272b)

struct PACKET_CHARLOGIN_ONLINE_ACCOUNTS {
	int16 packetType;
	uint16 packetLength;
	uint32 list_length;
	int accounts[];
} __attribute__((packed));
DEFINE_PACKET_ID(CHARLOGIN_ONLINE_ACCOUNTS, 0x272d)

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
	#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* COMMON_CHARLOGINPACKETS_H */
