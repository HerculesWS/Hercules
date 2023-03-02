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
#define HERCULES_CORE

#include "capiif.h"

#include "char/mapif.h"
#include "char/int_adventurer_agency.h"
#include "char/int_guild.h"
#include "char/int_userconfig.h"
#include "char/apipackets.h"
#include "common/api.h"
#include "common/cbasetypes.h"
#include "common/chunked/wfifo.h"
#include "common/core.h"
#include "common/db.h"
#include "common/HPM.h"
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

#define DEBUG_LOG
//#define DEBUG_PACKETS

static int capiif_parse_fromlogin_api_proxy(int fd)
{
	RFIFO_API_PROXY_PACKET(packet);
	const uint32 msg = packet->msg_id;

#ifdef DEBUG_PACKETS
	ShowInfo("capiif_parse_fromlogin_api_proxy: msg: %u, flags: %u, len: %u\n", msg, packet->flags, packet->packet_len);
#endif  // DEBUG_PACKETS

	if (PROXY_PACKET_FLAG(packet, proxy_flag_map)) {
		mapif->send((const unsigned char *)packet, packet->packet_len);
		RFIFOSKIP(fd, packet->packet_len);
		return 0;
	}

	if (VECTOR_LENGTH(HPM->packets[hpProxy_ApiChar]) > 0) {
		int result = HPM->parse_packets(fd, msg, hpProxy_ApiChar);
		if (result == 1)
			return 0;
		if (result == 2) {
			RFIFOSKIP(fd, packet->packet_len);
			return 0;
		}
	}

	switch (msg) {
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
		case API_MSG_party_add:
			capiif->parse_party_add(fd);
			break;
		case API_MSG_party_list:
			capiif->parse_party_list(fd);
			break;
		case API_MSG_party_get:
			capiif->parse_party_get(fd);
			break;
		case API_MSG_party_del:
			capiif->parse_party_del(fd);
			break;
		default:
			ShowError("Unknown proxy packet 0x%04x received from login-server, disconnecting.\n", msg);
			sockt->eof(fd);
			return 0;
	}

	RFIFOSKIP(fd, packet->packet_len);
	return 0;
}

static void capiif_parse_proxy_api_from_map(int fd)
{
	RFIFO_API_PROXY_PACKET(inPacket);
	const int len = inPacket->packet_len;
	const int login_fd = chr->login_fd;
	if (!sockt->session_is_active(login_fd))
		return;
	WFIFOHEAD(login_fd, len);
	memcpy(WFIFOP(login_fd, 0), inPacket, len);
	WFIFOW(login_fd, 0) = HEADER_API_PROXY_REPLY;
	WFIFOSET(login_fd, len);
}

void capiif_parse_userconfig_load_emotes(int fd)
{
	WFIFO_APICHAR_PACKET_REPLY(userconfig_load_emotes);
	RFIFO_API_PROXY_PACKET(p);

	bool load_res = inter_userconfig->load_emotes(p->account_id, &data->emotes);
	data->result = (load_res ? 1 : 0);

	WFIFOSET(chr->login_fd, packet->packet_len);
}

void capiif_parse_userconfig_save_emotes(int fd)
{
	RFIFO_API_DATA(data, userconfig_save_emotes);
	RFIFO_API_PROXY_PACKET(p);

	inter_userconfig->save_emotes(p->account_id, &data->emotes);

//	dont need send reply
}

void capiif_parse_charconfig_load(int fd)
{
	WFIFO_APICHAR_PACKET_REPLY_EMPTY();
	WFIFOSET(chr->login_fd, WFIFO_APICHAR_SIZE);
}

static void capiif_send_emblem_upload_result(int fd, int result)
{
	WFIFO_APICHAR_PACKET_REPLY(emblem_upload);
	data->result = result;
	WFIFOSET(chr->login_fd, packet->packet_len);
}

void capiif_parse_emblem_upload_guild_id(int fd)
{
	RFIFO_API_PROXY_PACKET_CHUNKED(p);
	RFIFO_API_DATA(data, emblem_upload_guild_id);

	struct online_char_data* character = capiif->get_online_character(&p->base);
	if (character == NULL)
		return;
	chr->ensure_online_char_data(character);
	if (character->data->emblem_data.data != NULL) {
		character->data->emblem_guild_id = 0;
		fifo_chunk_buf_clear(character->data->emblem_data);
		ShowError("Upload emblem guild id while emblem data transfer in progress.");
		capiif->send_emblem_upload_result(fd, 0);
		return;
	}
	character->data->emblem_guild_id = data->guild_id;
	character->data->emblem_gif = data->is_gif;
}

void capiif_parse_emblem_upload(int fd)
{
	RFIFO_API_PROXY_PACKET_CHUNKED(p);

	struct online_char_data* character = capiif->get_online_character(&p->base);
	if (character == NULL)
		return;
	struct online_char_data2 *char_data = character->data;

	if (char_data->emblem_guild_id == 0) {
		chr->clean_online_char_emblem_data(character);
		ShowError("Upload emblem guild data while emblem guild id is not set.\n");
		capiif->send_emblem_upload_result(fd, 0);
		return;
	}

	RFIFO_CHUNKED_INIT(p, GET_RFIFO_API_PROXY_PACKET_CHUNKED_SIZE(fd), char_data->emblem_data);

	RFIFO_CHUNKED_ERROR(p) {
		ShowError("Wrong guild emblem packets order\n");
		chr->clean_online_char_emblem_data(character);
		capiif->send_emblem_upload_result(fd, 0);
		return;
	}

	RFIFO_CHUNKED_COMPLETE(p) {
		bool success = false;
		if (inter_guild->is_guild_master(p->base.char_id, char_data->emblem_guild_id)) {
			success = inter_guild->update_emblem(char_data->emblem_data.data_size,
				char_data->emblem_guild_id,
				char_data->emblem_data.data);
		}

		capiif->send_emblem_upload_result(fd, (success ? 1 : 0));

		chr->clean_online_char_emblem_data(character);
	}
}

void capiif_parse_emblem_download(int fd)
{
	RFIFO_API_DATA(data, emblem_download);

#ifdef DEBUG_LOG
	ShowInfo("download emblem for %d, %d\n", data->guild_id, data->version);
#endif

	capiif->emblem_download(fd, data->guild_id, data->version);
}

void capiif_parse_userconfig_save_userhotkey_v2(int fd)
{
	RFIFO_API_DATA(data, userconfig_save_userhotkey_v2);
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

void capiif_parse_party_add(int fd)
{
	RFIFO_API_DATA(rdata, party_add);
	RFIFO_API_PROXY_PACKET(p);
	WFIFO_APICHAR_PACKET_REPLY(party_add);

	if (inter_adventurer_agency->entry_add(p->char_id, &rdata->entry))
		data->result = 1;
	else
		data->result = 0;

	WFIFOSET(chr->login_fd, packet->packet_len);
}

void capiif_parse_party_list(int fd)
{
	RFIFO_API_DATA(rdata, party_list);
	RFIFO_API_PROXY_PACKET(p);
	WFIFO_APICHAR_PACKET_REPLY(party_list);

	inter_adventurer_agency->get_page(p->char_id, rdata->page, &data->data);
	data->totalPage = inter_adventurer_agency->get_pages_count();

	WFIFOSET(chr->login_fd, packet->packet_len);
}

void capiif_parse_party_get(int fd)
{
	RFIFO_API_PROXY_PACKET(p);
	WFIFO_APICHAR_PACKET_REPLY(party_get);

	data->type = inter_adventurer_agency->get_player_request(p->char_id, &data->data);

	WFIFOSET(chr->login_fd, packet->packet_len);
}

void capiif_parse_party_del(int fd)
{
	RFIFO_API_DATA(rdata, party_del);
	RFIFO_API_PROXY_PACKET(p);
	WFIFO_APICHAR_PACKET_REPLY(party_del);

	data->type = inter_adventurer_agency->entry_delete(p->char_id, rdata->master_aid);

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
	capiif->parse_proxy_api_from_map = capiif_parse_proxy_api_from_map;
	capiif->parse_userconfig_load_emotes = capiif_parse_userconfig_load_emotes;
	capiif->parse_userconfig_save_emotes = capiif_parse_userconfig_save_emotes;
	capiif->parse_charconfig_load = capiif_parse_charconfig_load;
	capiif->send_emblem_upload_result = capiif_send_emblem_upload_result;
	capiif->parse_emblem_upload = capiif_parse_emblem_upload;
	capiif->parse_emblem_upload_guild_id = capiif_parse_emblem_upload_guild_id;
	capiif->parse_emblem_download = capiif_parse_emblem_download;
	capiif->parse_userconfig_save_userhotkey_v2 = capiif_parse_userconfig_save_userhotkey_v2;
	capiif->parse_userconfig_load_hotkeys = capiif_parse_userconfig_load_hotkeys;
	capiif->parse_party_add = capiif_parse_party_add;
	capiif->parse_party_list = capiif_parse_party_list;
	capiif->parse_party_get = capiif_parse_party_get;
	capiif->parse_party_del = capiif_parse_party_del;
}
