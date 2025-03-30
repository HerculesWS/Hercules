/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2025 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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
#ifndef COMMON_CHARMAPPACKETS_H
#define COMMON_CHARMAPPACKETS_H

#include "common/hercules.h"
#include "common/packetsmacro.h"

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
	#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

struct PACKET_CHARMAP_AGENCY_JOIN_PARTY {
	int16 packetType;
	int char_id;
	int result;
} __attribute__((packed));

struct PACKET_CHARMAP_GUILD_EMBLEM {
	int16 packetType;
	uint16 packetLength;
	int guild_id;
	int emblem_id;
	uint8 flag;
	char data[];
} __attribute__((packed));
DEFINE_PACKET_ID(CHARMAP_GUILD_EMBLEM, 0x383f)

struct PACKET_CHARMAP_GUILD_INFO_EMPTY {
	int16 packetType;
	uint16 packetLength;
	int guild_id;
} __attribute__((packed));
struct PACKET_CHARMAP_GUILD_INFO {
	int16 packetType;
	uint16 packetLength;
	struct guild g;
};
DEFINE_PACKET_ID(CHARMAP_GUILD_INFO, 0x3831)

struct PACKET_CHARMAP_GUILD_INFO_EMBLEM {
	int16 packetType;
	uint16 packetLength;
	int guild_id;
	int emblem_id;
	uint8 flag;
	char data[];
} __attribute__((packed));
DEFINE_PACKET_ID(CHARMAP_GUILD_INFO_EMBLEM, 0x389c)

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
	#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* COMMON_CHARMAPPACKETS_H */
