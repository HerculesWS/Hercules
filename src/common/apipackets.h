/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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
#ifndef COMMON_API_PACKETS_H
#define COMMON_API_PACKETS_H

#include "common/hercules.h"
#include "common/mmo.h"

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

#ifndef HOTKEY_DESCRIPTION_SIZE
#define HOTKEY_DESCRIPTION_SIZE 116
#endif

// [4144] for now using number of hotkeys bit bigger than actual amount
#ifndef MAX_USERHOTKEYS
#define MAX_USERHOTKEYS 50
#endif

#ifndef ADVENTURER_AGENCY_PAGE_SIZE
#define ADVENTURER_AGENCY_PAGE_SIZE 10
#endif

#define HEADER_API_PROXY_REQUEST 0x2842
#define HEADER_API_PROXY_REPLY 0x2818

enum UserHotKey_v2
{
	UserHotKey_v2_SkillBar_1Tab = 0,
	UserHotKey_v2_SkillBar_2Tab = 1,
	UserHotKey_v2_InterfaceTab = 2,
	UserHotKey_v2_EmotionTab = 3,
	UserHotKey_v2_max
};

enum adventurer_agency_flags {
	AGENCY_HEALER = 1,
	AGENCY_ASSIST = 2,
	AGENCY_TANKER = 4,
	AGENCY_DEALER = 8
};

// base
struct PACKET_API_PROXY {
	int16 packet_id;
	int16 packet_len;
	int16 msg_id;
	int32 char_server_id;
	int32 client_fd;
	int32 account_id;
	int32 char_id;
	int32 client_random_id;
	uint16 flags;
} __attribute__((packed));

// [4144] duplicate of PACKET_API_PROXY with data field.
// for avoid visual studio bug with emply arrays
struct PACKET_API_PROXY0 {
	struct PACKET_API_PROXY base;
	char data[];
} __attribute__((packed));

enum proxy_flag {
	proxy_flag_default = 0,
	proxy_flag_map = 1
};

STATIC_ASSERT(sizeof(struct PACKET_API_PROXY) == sizeof(struct PACKET_API_PROXY0),
		"Structs PACKET_API_PROXY and PACKET_API_PROXY0 must be same");

struct PACKET_API_PROXY_CHUNKED {
	struct PACKET_API_PROXY base;
	uint8 flag;
	char data[];
} __attribute__((packed));

struct userconfig_emotes {
	char emote[MAX_EMOTES][EMOTE_SIZE];
} __attribute__((packed));

// api to char

struct PACKET_API_userconfig_save_emotes_data {
	struct userconfig_emotes emotes;
} __attribute__((packed));

struct PACKET_API_userconfig_save_emotes {
	struct PACKET_API_userconfig_save_emotes_data data;
} __attribute__((packed));

struct userconfig_save_userhotkey_key {
	char desc[HOTKEY_DESCRIPTION_SIZE];
	int index;
	int key1;
	int key2;
} __attribute__((packed));

struct userconfig_userhotkeys_v2 {
	int tab;
	int count;
	struct userconfig_save_userhotkey_key keys[MAX_USERHOTKEYS];
} __attribute__((packed));

struct PACKET_API_userconfig_save_userhotkey_v2_data {
	struct userconfig_userhotkeys_v2 hotkeys;
} __attribute__((packed));

struct PACKET_API_userconfig_save_userhotkey_v2 {
	struct PACKET_API_userconfig_save_userhotkey_v2_data data;
}  __attribute__((packed));

/*
empty structs not supported by visual studio. left for future usage

struct PACKET_API_userconfig_load {
} __attribute__((packed));

struct PACKET_API_charconfig_load {
} __attribute__((packed));

struct PACKET_API_emblem_upload {
} __attribute__((packed));
*/

struct PACKET_API_emblem_upload_guild_id_data {
	int guild_id;
} __attribute__((packed));

struct PACKET_API_emblem_upload_guild_id {
	struct PACKET_API_emblem_upload_guild_id_data data;
} __attribute__((packed));

struct PACKET_API_emblem_download_data {
	int guild_id;
	int version;
} __attribute__((packed));

struct PACKET_API_emblem_download {
	struct PACKET_API_emblem_download_data data;
} __attribute__((packed));

struct party_add_data {
	char char_name[NAME_LENGTH];
	char message[NAME_LENGTH];
	int type;
	uint32 min_level;
	uint32 max_level;
	char healer;
	char assist;
	char tanker;
	char dealer;
} __attribute__((packed));

struct PACKET_API_party_add_data {
	struct party_add_data entry;
} __attribute__((packed));

struct PACKET_API_party_add {
	struct PACKET_API_party_add_data data;
} __attribute__((packed));


struct PACKET_API_party_list_data {
	int page;
} __attribute__((packed));

struct PACKET_API_party_list {
	struct PACKET_API_party_list_data data;
} __attribute__((packed));

struct PACKET_API_party_del_data {
	int master_aid;
} __attribute__((packed));

struct PACKET_API_party_del {
	struct PACKET_API_party_del_data data;
} __attribute__((packed));

// char to api
struct PACKET_API_REPLY_userconfig_load_emotes {
	struct userconfig_emotes emotes;
} __attribute__((packed));

struct PACKET_API_REPLY_userconfig_load_hotkeys_tab {
	struct userconfig_userhotkeys_v2 hotkeys;
} __attribute__((packed));

/*
empty structs not supported by visual studio. left for future usage

struct PACKET_API_REPLY_userconfig_load {
} __attribute__((packed));

struct PACKET_API_REPLY_userconfig_save {
} __attribute__((packed));

struct PACKET_API_REPLY_charconfig_load {
} __attribute__((packed));

struct PACKET_API_REPLY_emblem_upload {
} __attribute__((packed));
*/

struct PACKET_API_REPLY_emblem_download {
	uint8 flag;
	char data[];
} __attribute__((packed));

struct PACKET_API_REPLY_party_add {
	int result;
} __attribute__((packed));

struct adventuter_agency_entry {
	int account_id;
	int char_id;
	char char_name[NAME_LENGTH];
	char message[NAME_LENGTH];
	int flags;
	int min_level;
	int max_level;
	int type;
} __attribute__((packed));

struct adventuter_agency_page {
	struct adventuter_agency_entry entry[ADVENTURER_AGENCY_PAGE_SIZE];
} __attribute__((packed));

struct PACKET_API_REPLY_party_list {
	int page;
	int totalPage;
	struct adventuter_agency_page data;
} __attribute__((packed));

struct PACKET_API_REPLY_party_get {
	int type;
	struct adventuter_agency_entry data;
} __attribute__((packed));

struct PACKET_API_REPLY_party_del {
	int type;
} __attribute__((packed));

#define WFIFO_APICHAR_SIZE sizeof(struct PACKET_API_PROXY)
#define CHUNKED_FLAG_SIZE 1

#define RFIFO_DATA_PTR() RFIFOP(fd, WFIFO_APICHAR_SIZE)
#define RFIFO_API_DATA(var, type) const struct PACKET_API_ ## type *var = (const struct PACKET_API_ ## type*)RFIFO_DATA_PTR()
#define RFIFO_API_PROXY_PACKET(var) const struct PACKET_API_PROXY *var = RFIFOP(fd, 0)
#define RFIFO_API_PROXY_PACKET_CHUNKED(var) const struct PACKET_API_PROXY_CHUNKED *var = RFIFOP(fd, 0)
#define GET_RFIFO_API_PROXY_PACKET_SIZE(fd) (RFIFOW(fd, 2) - sizeof(struct PACKET_API_PROXY))
#define PROXY_PACKET_FLAG(packet, flag) ((packet)->flags & flag) != 0

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* COMMON_API_PACKETS_H */
