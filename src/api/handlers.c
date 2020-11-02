/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020 Andrei Karas (4144)
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

#include "config/core.h" // ANTI_MAYAP_CHEAT, RENEWAL, SECURE_NPCTIMEOUT
#include "api/handlers.h"

#include "common/cbasetypes.h"
#include "common/api.h"
#include "common/apipackets.h"
#include "common/chunked.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/utils.h"
#include "api/aclif.h"
#include "api/aloginif.h"
#include "api/apisessiondata.h"
#include "api/httpparser.h"
#include "api/httpsender.h"
#include "api/jsonwriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define HTTPURL(x) \
	static bool handlers_parse_ ## x(int fd, struct api_session_data *sd) __attribute__((nonnull (2))); \
	static bool handlers_parse_ ## x(int fd, struct api_session_data *sd)

#define DATA(x) \
	static void handlers_ ## x(int fd, struct api_session_data *sd, const void *data, size_t data_size) __attribute__((nonnull (2))); \
	static void handlers_ ## x(int fd, struct api_session_data *sd, const void *data, size_t data_size)

#define IGNORE_DATA(x) \
	DATA(x) \
	{ \
	}

#define GET_DATA(var, type) const struct PACKET_API_REPLY_ ## type *var = (const struct PACKET_API_REPLY_ ## type*)data;
#define CREATE_DATA(var, type) struct PACKET_API_ ## type ## _data var = { 0 };
#define SEND_ASYNC_DATA(name, data) aloginif->send_to_char(fd, sd, API_MSG_ ## name, data, sizeof(struct PACKET_API_ ## name));
#define SEND_ASYNC_DATA_SPLIT(name, data, size) aloginif->send_split_to_char(fd, sd, API_MSG_ ## name, data, size);

static struct handlers_interface handlers_s;
struct handlers_interface *handlers;

#define DEBUG_LOG

DATA(userconfig_load)
{
	JsonW *json = jsonwriter->create("{\"Type\":1}");
	JsonW *dataNode = jsonwriter->add_new_object(json, "data");
	JsonW *emotionHotkey = jsonwriter->add_new_array(dataNode, "EmotionHotkey");
	jsonwriter->add_new_null(dataNode, "WhisperBlockList");

	GET_DATA(p, userconfig_load);

	jsonwriter->add_strings_to_array(emotionHotkey,
		p->emotes.emote[0], p->emotes.emote[1], p->emotes.emote[2], p->emotes.emote[3], p->emotes.emote[4],
		p->emotes.emote[5], p->emotes.emote[6], p->emotes.emote[7], p->emotes.emote[8], p->emotes.emote[9],
		NULL);
#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif
	httpsender->send_json(fd, json);
	jsonwriter->delete(json);

	aclif->terminate_connection(fd);
}

HTTPURL(userconfig_load)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_load called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	SEND_ASYNC_DATA(userconfig_load, NULL);

	return true;
}

IGNORE_DATA(userconfig_save)

HTTPURL(userconfig_save)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_save called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif

	aclif->show_request(fd, sd, false);

	JsonP *json = NULL;
	aclif->get_post_header_data_json(sd, "data", &json);
	if (json == NULL) {
		ShowError("Error parsing json %d: %s\n", fd, sd->url);
		return false;
	}
	JsonP *dataNode = jsonparser->get(json, "data");
	if (dataNode == NULL) {
		ShowError("data node is missing in userconfig_save. %d: %s\n", fd, sd->url);
		jsonparser->delete(json);
		return false;
	}

	JsonP *userHotkey = jsonparser->get(dataNode, "UserHotkey");

	if (!jsonparser->is_null_or_missing(userHotkey)) {
		ShowError("UserHotkey node in userconfig_save not supporter. %d: %s\n", fd, sd->url);
		jsonparser->delete(json);
		return false;
	}

	CREATE_DATA(data, userconfig_save_emotes);

	JsonP *userHotkeyV2 = jsonparser->get(json, "UserHotkey_V2");
	if (userHotkeyV2 != NULL) {
		// send hotkey data to char server
	}
	JsonP *emotionHotkey = jsonparser->get(dataNode, "EmotionHotkey");

	// save emotes
	if (!jsonparser->is_null_or_missing(emotionHotkey)) {
		const int sz = jsonparser->get_array_size(emotionHotkey);
		Assert_retr(false, sz == MAX_EMOTES);
		int i = 0;
		JSONPARSER_FOR_EACH(emotion, emotionHotkey) {
			// should be used strncpy [4144]
			const char *str = jsonparser->get_string_value(emotion);
			if (str != NULL)
				strncpy(data.emotes.emote[i], str, EMOTE_SIZE);
			i++;
		}
		SEND_ASYNC_DATA(userconfig_save_emotes, &data);
	}

	jsonparser->delete(json);

	return true;
}

DATA(charconfig_load)
{
	// send hardcoded settings
	httpsender->send_plain(fd, "{\"Type\":1,\"data\":{\"HomunSkillInfo\":null,\"UseSkillInfo\":null}}");

	aclif->terminate_connection(fd);
}

HTTPURL(charconfig_load)
{
#ifdef DEBUG_LOG
	ShowInfo("charconfig_load called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	SEND_ASYNC_DATA(charconfig_load, NULL);

	return true;
}

DATA(emblem_upload)
{
	aclif->terminate_connection(fd);
}

HTTPURL(emblem_upload)
{
#ifdef DEBUG_LOG
	ShowInfo("emblem_upload called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	char *imgType = NULL;
	aclif->get_post_header_data_str(sd, "ImgType", &imgType, NULL);
	char *img = NULL;
	uint32 img_size = 0;
	aclif->get_post_header_data_str(sd, "Img", &img, &img_size);
	if (strcmp(imgType, "BMP") == 0) {
		if (img_size < 10 || strncmp(img, "BM", 2) != 0) {
			ShowError("wrong bmp image %d: %s\n", fd, sd->url);
			return false;
		}
	} else if (strcmp(imgType, "GIF") == 0) {
		if (img_size < 10 || strncmp(img, "GIF", 3) != 0 ||
		    memcmp(img + 3, "87a", 3) != 0 || memcmp(img + 3, "89a", 3) != 0) {
			ShowError("wrong gif image %d: %s\n", fd, sd->url);
			return false;
		}
	} else {
		ShowError("unknown image type '%s'. %d: %s\n", imgType, fd, sd->url);
		return false;
	}

	CREATE_DATA(data, emblem_upload_guild_id);
	int guild_id = 0;
	aclif->get_post_header_data_int(sd, "GDID", &guild_id);
	data.guild_id = guild_id;
	SEND_ASYNC_DATA(emblem_upload_guild_id, &data);
	SEND_ASYNC_DATA_SPLIT(emblem_upload, img, img_size);

	return true;
}

DATA(emblem_download)
{
#ifdef DEBUG_LOG
	ShowError("emblem_download data called\n");
#endif

	GET_DATA(p, emblem_download);
	const size_t src_emblem_size = data_size - CHUNKED_FLAG_SIZE;

	RFIFO_CHUNKED_INIT(p, src_emblem_size, sd->data, sd->data_size);

	RFIFO_CHUNKED_ERROR(p) {
		ShowError("Wrong guild emblem packets order\n");
		aclif->terminate_connection(fd);
		return;
	}

	RFIFO_CHUNKED_COMPLETE(p) {
		if (sd->data_size > 65000) {
			ShowError("Big emblems not supported yet\n");
			aclif->terminate_connection(fd);
			return;
		}
		httpsender->send_binary(fd, sd->data, sd->data_size);
		aclif->terminate_connection(fd);
	}
}

HTTPURL(emblem_download)
{
#ifdef DEBUG_LOG
	ShowInfo("emblem_download called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	CREATE_DATA(data, emblem_download);

	int value = 0;
	aclif->get_post_header_data_int(sd, "GDID", &value);
	data.guild_id = value;
	aclif->get_post_header_data_int(sd, "Version", &value);
	data.version = value;

	SEND_ASYNC_DATA(emblem_download, &data);

	return true;
}

HTTPURL(test_url)
{
#ifdef DEBUG_LOG
	ShowInfo("test_url called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif

	char buf[1000];
	const char *user_agent = (const char*)strdb_get(sd->headers_db, "User-Agent");
	const char *format = "<html>Hercules test.<br/>Your user agent is: %s<br/></html>\n";
	safesnprintf(buf, sizeof(buf), format, user_agent);

	httpsender->send_html(fd, buf);

	aclif->terminate_connection(fd);

	return true;
}

static int do_init_handlers(bool minimal)
{
	return 0;
}

static void do_final_handlers(void)
{
}

void handlers_defaults(void)
{
	handlers = &handlers_s;
	handlers->init = do_init_handlers;
	handlers->final = do_final_handlers;

#define handler(method, url, func, flags) handlers->parse_ ## func = handlers_parse_ ## func
#define handler2(method, url, func, flags) handlers->parse_ ## func = handlers_parse_ ## func; \
	handlers->func = handlers_ ## func
#define packet_handler(func) handlers->func = handlers_ ## func
#include "api/urlhandlers.h"
#undef handler
#undef handler2
#undef packet_handler
}
