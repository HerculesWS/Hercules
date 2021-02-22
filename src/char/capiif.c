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
#define HERCULES_CORE

#include "capiif.h"

#include "char/char.h"
#include "char/mapif.h"
#include "char/int_guild.h"
#include "char/int_userconfig.h"
#include "common/api.h"
#include "common/apipackets.h"
#include "common/cbasetypes.h"
#include "common/chunked.h"
#include "common/core.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdlib.h>
#include <string.h>

static struct capiif_interface capiif_s;
struct capiif_interface *capiif;

#define WFIFO_APICHAR_PACKET_REPLY_EMPTY() \
	WFIFOHEAD(chr->login_fd, WFIFO_APICHAR_SIZE); \
	memcpy(WFIFOP(chr->login_fd, 0), RFIFOP(fd, 0), WFIFO_APICHAR_SIZE); \
	struct PACKET_API_PROXY *packet = WFIFOP(chr->login_fd, 0); \
	packet->packet_id = HEADER_API_PROXY_REPLY; \
	packet->packet_len = WFIFO_APICHAR_SIZE; \

#define WFIFO_APICHAR_PACKET_REPLY(type) \
	WFIFOHEAD(chr->login_fd, WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_REPLY_ ## type)); \
	memcpy(WFIFOP(chr->login_fd, 0), RFIFOP(fd, 0), WFIFO_APICHAR_SIZE); \
	struct PACKET_API_PROXY *packet = WFIFOP(chr->login_fd, 0); \
	packet->packet_id = HEADER_API_PROXY_REPLY; \
	packet->packet_len = WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_REPLY_ ## type); \
	struct PACKET_API_REPLY_ ## type *data = WFIFOP(chr->login_fd, sizeof(struct PACKET_API_PROXY))

#define INIT_PACKET_REPLY_PROXY_FIELDS(p, p2) \
	(p)->msg_id = (p2)->msg_id; \
	(p)->server_id = (p2)->server_id; \
	(p)->client_fd = (p2)->client_fd; \
	(p)->account_id = (p2)->account_id; \
	(p)->char_id = (p2)->char_id; \
	(p)->client_random_id = (p2)->client_random_id

#define RFIFO_DATA_PTR() RFIFOP(fd, WFIFO_APICHAR_SIZE)
#define RFIFO_API_DATA(var, type) const struct PACKET_API_ ## type *var = (const struct PACKET_API_ ## type*)RFIFO_DATA_PTR()
#define RFIFO_API_PROXY_PACKET(var) const struct PACKET_API_PROXY *var = RFIFOP(fd, 0)
#define RFIFO_API_PROXY_PACKET_CHUNKED(var) const struct PACKET_API_PROXY_CHUNKED *var = RFIFOP(fd, 0)
#define GET_RFIFO_API_PROXY_PACKET_SIZE(fd) (RFIFOW(fd, 2) - sizeof(struct PACKET_API_PROXY))

#define DEBUG_LOG

static int capiif_parse_fromlogin_api_proxy(int fd)
{
	const uint32 command = RFIFOW(fd, 4);

	switch (command) {
		case API_MSG_userconfig_load_emotes:
			capiif->parse_userconfig_load_emotes(fd);
			break;
		case API_MSG_userconfig_save_emotes:
			capiif->parse_userconfig_save_emotes(fd);
			break;
		case API_MSG_charconfig_load:
			capiif->parse_charconfig_load(fd);
			break;
		case API_MSG_emblem_upload_guild_id:
			capiif->parse_emblem_upload_guild_id(fd);
			break;
		case API_MSG_emblem_upload:
			capiif->parse_emblem_upload(fd);
			break;
		case API_MSG_emblem_download:
			capiif->parse_emblem_download(fd);
			break;
		case API_MSG_userconfig_save_userhotkey_v2:
			capiif->parse_userconfig_save_userhotkey_v2(fd);
			break;
		case API_MSG_userconfig_load_hotkeys:
			capiif->parse_userconfig_load_hotkeys(fd);
			break;
		default:
			ShowError("Unknown proxy packet 0x%04x received from login-server, disconnecting.\n", command);
			sockt->eof(fd);
			return 0;
	}

	RFIFOSKIP(fd, RFIFOW(fd, 2));
	return 0;
}


void capiif_parse_userconfig_load_emotes(int fd)
{
	WFIFO_APICHAR_PACKET_REPLY(userconfig_load_emotes);
	RFIFO_API_PROXY_PACKET(p);

	inter_userconfig->load_emotes(p->account_id, &data->emotes);

	WFIFOSET(chr->login_fd, packet->packet_len);
}

void capiif_parse_userconfig_save_emotes(int fd)
{
	RFIFO_API_DATA(data, userconfig_save_emotes_data);
	RFIFO_API_PROXY_PACKET(p);

	inter_userconfig->save_emotes(p->account_id, &data->emotes);

//	dont need send reply
}

void capiif_parse_charconfig_load(int fd)
{
	WFIFO_APICHAR_PACKET_REPLY_EMPTY();
	WFIFOSET(chr->login_fd, WFIFO_APICHAR_SIZE);
}

void capiif_parse_emblem_upload_guild_id(int fd)
{
	RFIFO_API_PROXY_PACKET_CHUNKED(p);
	RFIFO_API_DATA(data, emblem_upload_guild_id_data);

	struct online_char_data* character = capiif->get_online_character(&p->base);
	if (character == NULL)
		return;
	chr->ensure_online_char_data(character);
	if (character->data->emblem_data != NULL) {
		character->data->emblem_guild_id = 0;
		character->data->emblem_data_size = 0;
		ShowError("Upload emblem guild id while emblem data transfer in progress.");
		return;
	}
	character->data->emblem_guild_id = data->guild_id;
}

void capiif_parse_emblem_upload(int fd)
{
	RFIFO_API_PROXY_PACKET_CHUNKED(p);

	struct online_char_data* character = capiif->get_online_character(&p->base);
	if (character == NULL)
		return;

	if (character->data->emblem_guild_id == 0) {
		chr->clean_online_char_emblem_data(character);
		ShowError("Upload emblem guild data while emblem guild id is not set.\n");
		return;
	}

	RFIFO_CHUNKED_INIT(p, GET_RFIFO_API_PROXY_PACKET_CHUNKED_SIZE(fd), character->data->emblem_data, character->data->emblem_data_size);

	RFIFO_CHUNKED_ERROR(p) {
		ShowError("Wrong guild emblem packets order\n");
		chr->clean_online_char_emblem_data(character);
		return;
	}

	RFIFO_CHUNKED_COMPLETE(p) {
		if (character->data->emblem_data_size > 65000) {
			ShowError("Big emblems not supported yet\n");
			chr->clean_online_char_emblem_data(character);
			return;
		}

		if (inter_guild->is_guild_master(p->base.char_id, character->data->emblem_guild_id)) {
			if (!inter_guild->validate_emblem(character->data->emblem_data,
			    character->data->emblem_data_size)) {
				ShowError("Invalid image uploaded\n");
			}
			inter_guild->update_emblem(character->data->emblem_data_size,
				character->data->emblem_guild_id,
				character->data->emblem_data);
		}

		chr->clean_online_char_emblem_data(character);
	}
}

void capiif_parse_emblem_download(int fd)
{
	RFIFO_API_DATA(data, emblem_download_data);

#ifdef DEBUG_LOG
	ShowInfo("download emblem for %d, %d\n", data->guild_id, data->version);
#endif

	capiif->emblem_download(fd, data->guild_id, data->version);
}

void capiif_parse_userconfig_save_userhotkey_v2(int fd)
{
	RFIFO_API_DATA(data, userconfig_save_userhotkey_v2_data);
	RFIFO_API_PROXY_PACKET(p);

	inter_userconfig->hotkey_tab_tosql(p->account_id, &data->hotkeys);
}

void capiif_emblem_download(int fd, int guild_id, int emblem_id)
{
	struct guild *g = inter_guild->fromsql(guild_id);
	if (g == NULL) {
		WFIFO_APICHAR_PACKET_REPLY_EMPTY();
		WFIFOSET(chr->login_fd, WFIFO_APICHAR_SIZE);
		return;
	}

	RFIFO_API_PROXY_PACKET(p2);
	WFIFO_CHUNKED_INIT(p, chr->login_fd, HEADER_API_PROXY_REPLY, PACKET_API_PROXY_CHUNKED, g->emblem_data, g->emblem_len) {
		WFIFO_CHUNKED_BLOCK_START(p);
		INIT_PACKET_REPLY_PROXY_FIELDS(&p->base, p2);
		WFIFO_CHUNKED_BLOCK_END();
	}
	WFIFO_CHUNKED_FINAL_START(p);
	INIT_PACKET_REPLY_PROXY_FIELDS(&p->base, p2);
	WFIFO_CHUNKED_FINAL_END();
}

static struct online_char_data* capiif_get_online_character(const struct PACKET_API_PROXY *p)
{
	struct online_char_data* character = (struct online_char_data*)idb_get(chr->online_char_db, p->account_id);
	if (character == NULL) {
		ShowError("Cant get online character. Account %d is not online.", p->account_id);
		return NULL;
	}
	if (character->char_id != p->char_id) {
		ShowError("Cant get online character. Character %d is not online.", p->char_id);
		return NULL;
	}
	return character;
}

void capiif_parse_userconfig_load_hotkeys(int fd)
{
	RFIFO_API_PROXY_PACKET(p);

	for (int tab = 0; tab < UserHotKey_v2_max; tab ++) {
		WFIFO_APICHAR_PACKET_REPLY(userconfig_load_hotkeys_tab);
		inter_userconfig->hotkey_tab_fromsql(p->account_id, &data->hotkeys, tab);
		WFIFOSET(chr->login_fd, packet->packet_len);
	}
	WFIFO_APICHAR_PACKET_REPLY_EMPTY()
	packet->msg_id = API_MSG_userconfig_load;
	WFIFOSET(chr->login_fd, packet->packet_len);
}

static void do_init_capiif(void)
{
}

static void do_final_capiif(void)
{
}

void capiif_defaults(void) {
	capiif = &capiif_s;

	capiif->init = do_init_capiif;
	capiif->final = do_final_capiif;
	capiif->get_online_character = capiif_get_online_character;
	capiif->emblem_download = capiif_emblem_download;
	capiif->parse_fromlogin_api_proxy = capiif_parse_fromlogin_api_proxy;
	capiif->parse_userconfig_load_emotes = capiif_parse_userconfig_load_emotes;
	capiif->parse_userconfig_save_emotes = capiif_parse_userconfig_save_emotes;
	capiif->parse_charconfig_load = capiif_parse_charconfig_load;
	capiif->parse_emblem_upload = capiif_parse_emblem_upload;
	capiif->parse_emblem_upload_guild_id = capiif_parse_emblem_upload_guild_id;
	capiif->parse_emblem_download = capiif_parse_emblem_download;
	capiif->parse_userconfig_save_userhotkey_v2 = capiif_parse_userconfig_save_userhotkey_v2;
	capiif->parse_userconfig_load_hotkeys = capiif_parse_userconfig_load_hotkeys;
}
