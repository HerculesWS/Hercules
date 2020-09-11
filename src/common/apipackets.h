/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
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
#ifndef COMMON_API_PACKETS_H
#define COMMON_API_PACKETS_H

#include "common/hercules.h"

/* Packets Structs */
#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

#ifndef EMOTE_SIZE
#define EMOTE_SIZE 50
#endif

#ifndef MAX_EMOTES
#define MAX_EMOTES 10
#endif

#define HEADER_API_PROXY_REQUEST 0x2842
#define HEADER_API_PROXY_REPLY 0x2818

// base
struct PACKET_API_PROXY {
	int16 packet_id;
	int16 packet_len;
	int16 msg_id;
	int32 server_id;
	int32 client_fd;
	int32 account_id;
	int32 char_id;
	int32 client_random_id;
	char data[];
} __attribute__((packed));

struct PACKET_API_EMPTY {
} __attribute__((packed));

// api to char
struct PACKET_API_userconfig_load {
} __attribute__((packed));

struct PACKET_API_userconfig_save {
} __attribute__((packed));

struct PACKET_API_charconfig_load {
} __attribute__((packed));

// char to api
//struct PACKET_API_REPLY_userconfig_load_emote {
//	char text[EMOTE_SIZE];
//} __attribute__((packed));

struct PACKET_API_REPLY_userconfig_load {
	char emote[MAX_EMOTES][EMOTE_SIZE];
} __attribute__((packed));

struct PACKET_API_REPLY_userconfig_save {
} __attribute__((packed));

struct PACKET_API_REPLY_charconfig_load {
} __attribute__((packed));

#define WFIFO_APICHAR_SIZE sizeof(struct PACKET_API_PROXY)

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* COMMON_API_PACKETS_H */
