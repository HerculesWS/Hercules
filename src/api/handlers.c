/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020-2021 Andrei Karas (4144)
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
// SEND_ASYNC_DATA_EMPTY is workaround for visual studio bugs
#define SEND_ASYNC_DATA_EMPTY(name, data) aloginif->send_to_char(fd, sd, API_MSG_ ## name, data, 0);
#define SEND_ASYNC_DATA_SPLIT(name, data, size) aloginif->send_split_to_char(fd, sd, API_MSG_ ## name, data, size);

#define SEND_ASYNC_USERHOKEY_V2_TAB(name) \
	JsonP *name = jsonparser->get(userHotkeyV2, #name); \
	if (!jsonparser->is_null_or_missing(name)) { \
		ShowInfo("send tab " # name "\n"); \
		CREATE_DATA(data, userconfig_save_userhotkey_v2); \
		data.hotkeys.tab = UserHotKey_v2_ ## name; \
		handlers->sendHotkeyV2Tab(name, &data.hotkeys); \
		SEND_ASYNC_DATA(userconfig_save_userhotkey_v2, &data); \
	}

#define GET_JSON_HEADER(name, json) \
	if (!aclif->get_valid_header_data_json(sd, CONST_POST_ ## name, POST_ ## name, json)) { \
		ShowError("Valid post header %s not found. Can be memory leaks.", POST_ ## name); \
		Assert_report(0); \
		aclif->terminate_connection(fd); \
		return false; \
	}

#define GET_STR_HEADER(name, var, varSize) \
	if (!aclif->get_valid_header_data_str(sd, CONST_POST_ ## name, POST_ ## name, var, varSize)) { \
		ShowError("Valid post header %s not found. Can be memory leaks.", POST_ ## name); \
		Assert_report(0); \
		aclif->terminate_connection(fd); \
		return false; \
	}
#define GET_STR_HEADER_EMPTY(name, var, varSize) aclif->get_valid_header_data_str(sd, CONST_POST_ ## name, POST_ ## name, var, varSize);

#define RET_INT_HEADER(name, def) aclif->ret_valid_header_data_int(sd, CONST_POST_ ## name, POST_ ## name, def);
#define GET_INT_HEADER(name, var) aclif->get_valid_header_data_int(sd, CONST_POST_ ## name, POST_ ## name, var);

static struct handlers_interface handlers_s;
struct handlers_interface *handlers;

#define DEBUG_LOG

const char *handlers_hotkeyTabIdToName(int tab_id)
{
	Assert_retr(NULL, tab_id >= 0 && tab_id < UserHotKey_v2_max);

	static char *name[4] = {
		"SkillBar_1Tab",
		"SkillBar_2Tab",
		"InterfaceTab",
		"EmotionTab",
	};
	return name[tab_id];
}

DATA(userconfig_load)
{
	JsonW *json = sd->json;
	nullpo_retv(json);

	// finally send json
#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif
	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	sd->json = NULL;

	aclif->terminate_connection(fd);
}

DATA(userconfig_load_emotes)
{
	// create initial json node and add emotionHotkey
	JsonW *json = jsonwriter->create("{\"Type\":1}");
	sd->json = json;
	JsonW *dataNode = jsonwriter->add_new_object(json, "data");
	JsonW *emotionHotkey = jsonwriter->add_new_array(dataNode, "EmotionHotkey");

	// WhisperBlockList not implimented yet
	jsonwriter->add_new_null(dataNode, "WhisperBlockList");

	// add empty UserHotkey_V2 for future usage
	jsonwriter->add_new_object(dataNode, "UserHotkey_V2");

	GET_DATA(p, userconfig_load_emotes);

	jsonwriter->add_new_strings_to_array(emotionHotkey,
		p->emotes.emote[0], p->emotes.emote[1], p->emotes.emote[2], p->emotes.emote[3], p->emotes.emote[4],
		p->emotes.emote[5], p->emotes.emote[6], p->emotes.emote[7], p->emotes.emote[8], p->emotes.emote[9],
		NULL);
}

DATA(userconfig_load_hotkeys)
{
	JsonW *json = sd->json;
	nullpo_retv(json);

	GET_DATA(p, userconfig_load_hotkeys_tab);

	JsonW *dataNode = jsonwriter->get(json, "data");
	JsonW *userHotkeyV2Node = jsonwriter->get(dataNode, "UserHotkey_V2");
	const char *tab_name = handlers->hotkeyTabIdToName(p->hotkeys.tab);
	if (p->hotkeys.count != 0) {
		JsonW *tabNode = jsonwriter->add_new_array(userHotkeyV2Node, tab_name);
		for (int i = 0; i < p->hotkeys.count; i ++) {
			JsonW *hotkey = jsonwriter->add_new_object_to_array(tabNode);
			jsonwriter->add_new_string(hotkey, "desc", p->hotkeys.keys[i].desc);
			jsonwriter->add_new_number(hotkey, "index", p->hotkeys.keys[i].index);
			jsonwriter->add_new_number(hotkey, "key1", p->hotkeys.keys[i].key1);
			jsonwriter->add_new_number(hotkey, "key2", p->hotkeys.keys[i].key2);
		}
	} else {
		jsonwriter->add_new_null(userHotkeyV2Node, tab_name);
	}
}

HTTPURL(userconfig_load)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_load called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	SEND_ASYNC_DATA_EMPTY(userconfig_load_emotes, NULL);
	SEND_ASYNC_DATA_EMPTY(userconfig_load_hotkeys, NULL);
//	SEND_ASYNC_DATA_EMPTY(userconfig_load, NULL);

	return true;
}

void handlers_sendHotkeyV2Tab(JsonP *json, struct userconfig_userhotkeys_v2 *hotkeys)
{
	int i = 0;
	JSONPARSER_FOR_EACH(value, json) {
		if (i >= MAX_USERHOTKEYS)
			break;
		const char *desc = jsonparser->get_child_string_value(value, "desc");
		if (desc == NULL)
			continue;
		hotkeys->keys[i].index = jsonparser->get_child_int_value(value, "index");
		hotkeys->keys[i].key1 = jsonparser->get_child_int_value(value, "key1");
		hotkeys->keys[i].key2 = jsonparser->get_child_int_value(value, "key2");
		safestrncpy(hotkeys->keys[i].desc, desc, HOTKEY_DESCRIPTION_SIZE);
		i ++;
	}
	hotkeys->count = i;
}

IGNORE_DATA(userconfig_save)

HTTPURL(userconfig_save)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_save called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif

	aclif->show_request(fd, sd, false);

	JsonP *json = NULL;
	GET_JSON_HEADER(DATA, &json);
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


	JsonP *userHotkeyV2 = jsonparser->get(dataNode, "UserHotkey_V2");
	if (userHotkeyV2 != NULL) {
		SEND_ASYNC_USERHOKEY_V2_TAB(SkillBar_1Tab)
		SEND_ASYNC_USERHOKEY_V2_TAB(SkillBar_2Tab)
		SEND_ASYNC_USERHOKEY_V2_TAB(InterfaceTab)
		SEND_ASYNC_USERHOKEY_V2_TAB(EmotionTab)
	}
	JsonP *emotionHotkey = jsonparser->get(dataNode, "EmotionHotkey");

	// save emotes
	if (!jsonparser->is_null_or_missing(emotionHotkey)) {
		const int sz = jsonparser->get_array_size(emotionHotkey);
		Assert_retr(false, sz == MAX_EMOTES);
		CREATE_DATA(data, userconfig_save_emotes);
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

	SEND_ASYNC_DATA_EMPTY(charconfig_load, NULL);

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
	GET_STR_HEADER(IMG_TYPE, &imgType, NULL);
	char *img = NULL;
	uint32 img_size = 0;
	GET_STR_HEADER(IMG, &img, &img_size);
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
	data.guild_id = RET_INT_HEADER(GUILD_ID, 0);
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

	data.guild_id = RET_INT_HEADER(GUILD_ID, 0);
	data.version = RET_INT_HEADER(VERSION, 0);

	SEND_ASYNC_DATA(emblem_download, &data);

	return true;
}

IGNORE_DATA(party_list)

HTTPURL(party_list)
{
#ifdef DEBUG_LOG
	ShowInfo("party_list called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	JsonW *json = jsonwriter->create("{\"totalPage\":0,\"data\":[],\"Type\":1}");
	httpsender->send_json(fd, json);
	jsonwriter->delete(json);

	return true;
}

IGNORE_DATA(party_get)

HTTPURL(party_get)
{
#ifdef DEBUG_LOG
	ShowInfo("party_get called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	JsonW *json = jsonwriter->create("{\"Type\":1}");
	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	aclif->terminate_connection(fd);

	return true;
}

DATA(party_add)
{
	GET_DATA(p, party_add);
	ShowError("result: %d\n", p->result);

	JsonW *json = jsonwriter->create_empty();
	jsonwriter->add_new_number(json, "Type", p->result);

#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif

	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	aclif->terminate_connection(fd);
}

HTTPURL(party_add)
{
#ifdef DEBUG_LOG
	ShowInfo("party_add called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
	aclif->show_request(fd, sd, false);

	char *text = NULL;
	CREATE_DATA(data, party_add);

	GET_STR_HEADER(CHAR_NAME, &text, NULL);
	safestrncpy(data.entry.char_name, text, NAME_LENGTH);
	GET_STR_HEADER_EMPTY(MEMO, &text, NULL);
	if (text != NULL)
		safestrncpy(data.entry.message, text, NAME_LENGTH);
	else
		memset(data.entry.message, 0, NAME_LENGTH);
	data.entry.type = RET_INT_HEADER(TYPE, 0);
	data.entry.min_level = RET_INT_HEADER(MINLV, 0);
	data.entry.max_level = RET_INT_HEADER(MAXLV, 0);
	data.entry.healer = RET_INT_HEADER(HEALER, 0);
	data.entry.assist = RET_INT_HEADER(ASSIST, 0);
	data.entry.tanker = RET_INT_HEADER(TANKER, 0);
	data.entry.dealer = RET_INT_HEADER(DEALER, 0);

	SEND_ASYNC_DATA(party_add, &data);

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
	handlers->sendHotkeyV2Tab = handlers_sendHotkeyV2Tab;
	handlers->hotkeyTabIdToName = handlers_hotkeyTabIdToName;

#define handler(method, url, func, flags) handlers->parse_ ## func = handlers_parse_ ## func
#define handler2(method, url, func, flags) handlers->parse_ ## func = handlers_parse_ ## func; \
	handlers->func = handlers_ ## func
#define packet_handler(func) handlers->func = handlers_ ## func
#include "api/urlhandlers.h"
#undef handler
#undef handler2
#undef packet_handler
}
