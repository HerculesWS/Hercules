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

#define RFIFO_DATA_PTR() RFIFOP(fd, WFIFO_APICHAR_SIZE)
#define RFIFO_API_DATA(var, type) const struct PACKET_API_ ## type *var = (const struct PACKET_API_ ## type*)RFIFO_DATA_PTR()
#define RFIFO_API_PROXY_PACKET(var) const struct PACKET_API_PROXY *var = RFIFOP(fd, 0)
#define RFIFO_API_PROXY_PACKET_CHUNKED(var) const struct PACKET_API_PROXY_CHUNKED *var = RFIFOP(fd, 0)
#define GET_RFIFO_API_PROXY_PACKET_SIZE(fd) (RFIFOW(fd, 2) - sizeof(struct PACKET_API_PROXY))
#define GET_RFIFO_API_PROXY_PACKET_CHUNKED_SIZE(fd) (RFIFOW(fd, 2) - sizeof(struct PACKET_API_PROXY_CHUNKED))


static int capiif_parse_fromlogin_api_proxy(int fd)
{
	const uint command = RFIFOW(fd, 4);

	switch (command) {
		case API_MSG_userconfig_load:
			capiif->parse_userconfig_load(fd);
			break;
		case API_MSG_userconfig_save_emotes:
			capiif->parse_userconfig_save_emotes(fd);
			break;
		case API_MSG_charconfig_load:
			capiif->parse_charconfig_load(fd);
			break;
		case API_MSG_umblem_upload_guild_id:
			capiif->parse_umblem_upload_guild_id(fd);
			break;
		case API_MSG_umblem_upload:
			capiif->parse_umblem_upload(fd);
			break;
		default:
			ShowError("Unknown proxy packet 0x%04x received from login-server, disconnecting.\n", command);
			sockt->eof(fd);
			return 0;
	}

	RFIFOSKIP(fd, RFIFOW(fd, 2));
	return 0;
}


void capiif_parse_userconfig_load(int fd)
{
	WFIFO_APICHAR_PACKET_REPLY(userconfig_load);
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

// debug
#include "common/utils.h"
// debug

void capiif_parse_umblem_upload_guild_id(int fd)
{
	RFIFO_API_PROXY_PACKET_CHUNKED(p);
	RFIFO_API_DATA(data, umblem_upload_guild_id_data);
	ShowError("Upload emblem for guild id: %d\n", data->guild_id);

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

void capiif_parse_umblem_upload(int fd)
{
	RFIFO_API_PROXY_PACKET_CHUNKED(p);

	struct online_char_data* character = capiif->get_online_character(&p->base);
	if (character == NULL)
		return;

//	ShowDump(p->data, GET_RFIFO_API_PROXY_PACKET_CHUNKED_SIZE(fd));

	if (character->data->emblem_guild_id == 0) {
		chr->clean_online_char_emblem_data(character);
		ShowError("Upload emblem guild data while emblem guild id is not set.");
		return;
	}

	RFIFO_CHUNKED_INIT(p, character->data->emblem_data, character->data->emblem_data_size) {
		ShowError("Wrong guild emblem packets order\n");
		chr->clean_online_char_emblem_data(character);
		return;
	}

	RFIFO_CHUNKED_COMPLETE(p) {
		if (character->data->emblem_data_size > 65000) {
			ShowError("Big emblems not supported yer");
			chr->clean_online_char_emblem_data(character);
			return;
		}

		if (inter_guild->is_guild_master(p->base.char_id, character->data->emblem_guild_id)) {
			inter_guild->update_emblem(character->data->emblem_data_size,
				character->data->emblem_guild_id,
				character->data->emblem_data);
		}

		chr->clean_online_char_emblem_data(character);
	}
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
	capiif->parse_fromlogin_api_proxy = capiif_parse_fromlogin_api_proxy;
	capiif->parse_userconfig_load = capiif_parse_userconfig_load;
	capiif->parse_userconfig_save_emotes = capiif_parse_userconfig_save_emotes;
	capiif->parse_charconfig_load = capiif_parse_charconfig_load;
	capiif->parse_umblem_upload = capiif_parse_umblem_upload;
	capiif->parse_umblem_upload_guild_id = capiif_parse_umblem_upload_guild_id;
}
