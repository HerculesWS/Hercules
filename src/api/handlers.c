/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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
//#include "common/chunked.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/utils.h"
#include "api/aclif.h"
#include "api/apipackets.h"
#include "api/apisessiondata.h"
#include "api/httpparser.h"
#include "api/httpsender.h"
#include "api/imageparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define SEND_ASYNC_USERHOKEY_V2_TAB(name) \
	JsonP *name = jsonparser->get(userHotkeyV2, #name); \
	if (!jsonparser->is_null_or_missing(name)) { \
		ShowInfo("send tab " # name "\n"); \
		CREATE_HTTP_DATA(data, userconfig_save_userhotkey_v2); \
		data.hotkeys.tab = UserHotKey_v2_ ## name; \
		handlers->sendHotkeyV2Tab(name, &data.hotkeys); \
		SEND_CHAR_ASYNC_DATA(userconfig_save_userhotkey_v2, &data); \
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

//#define DEBUG_LOG
#define REQUEST_LOG

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

HTTP_DATA(userconfig_load)
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

HTTP_DATA(userconfig_load_emotes)
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

	GET_HTTP_DATA(p, userconfig_load_emotes);

	jsonwriter->add_new_strings_to_array(emotionHotkey,
		p->emotes.emote[0], p->emotes.emote[1], p->emotes.emote[2], p->emotes.emote[3], p->emotes.emote[4],
		p->emotes.emote[5], p->emotes.emote[6], p->emotes.emote[7], p->emotes.emote[8], p->emotes.emote[9],
		NULL);
}

HTTP_DATA(userconfig_load_hotkeys)
{
	JsonW *json = sd->json;
	nullpo_retv(json);

	GET_HTTP_DATA(p, userconfig_load_hotkeys_tab);

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

HTTP_URL(userconfig_load)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_load called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	SEND_CHAR_ASYNC_DATA_EMPTY(userconfig_load_emotes, NULL);
	SEND_CHAR_ASYNC_DATA_EMPTY(userconfig_load_hotkeys, NULL);
//	SEND_CHAR_ASYNC_DATA_EMPTY(userconfig_load, NULL);

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

HTTP_IGNORE_DATA(userconfig_save)

HTTP_URL(userconfig_save)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_save called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
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
		CREATE_HTTP_DATA(data, userconfig_save_emotes);
		int i = 0;
		JSONPARSER_FOR_EACH(emotion, emotionHotkey) {
			const char *str = jsonparser->get_string_value(emotion);
			if (str != NULL)
				safestrncpy(data.emotes.emote[i], str, EMOTE_SIZE);
			i++;
		}
		SEND_CHAR_ASYNC_DATA(userconfig_save_emotes, &data);
	}

	jsonparser->delete(json);

	return true;
}

HTTP_DATA(charconfig_load)
{
	// send hardcoded settings
	httpsender->send_plain(fd, "{\"Type\":1,\"data\":{\"HomunSkillInfo\":null,\"UseSkillInfo\":null}}");

	aclif->terminate_connection(fd);
}

HTTP_URL(charconfig_load)
{
#ifdef DEBUG_LOG
	ShowInfo("charconfig_load called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	SEND_CHAR_ASYNC_DATA_EMPTY(charconfig_load, NULL);

	return true;
}

HTTP_DATA(emblem_upload)
{
#ifdef DEBUG_LOG
	ShowInfo("emblem_upload data received\n");
#endif

	GET_HTTP_DATA(p, emblem_upload);

	if (p->result == 1)
		httpsender->send_json_text(fd, "{\"Type\":1}", HTTP_STATUS_OK);
	else // Not sure if intentional, but kRO sends status 500
		httpsender->send_json_text(fd, "{\"Type\":4}", HTTP_STATUS_INTERNAL_SERVER_ERROR);

	aclif->terminate_connection(fd);
}

HTTP_URL(emblem_upload)
{
#ifdef DEBUG_LOG
	ShowInfo("emblem_upload called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	char *imgType = NULL;
	GET_STR_HEADER(IMG_TYPE, &imgType, NULL);
	char *img = NULL;
	uint32 img_size = 0;
	bool is_gif = false;
	GET_STR_HEADER(IMG, &img, &img_size);

	bool has_error = false;
	if (strcmp(imgType, "BMP") == 0) {
		if (!imageparser->validate_bmp_emblem(img, img_size)) {
			ShowError("wrong bmp image %d: %s\n", fd, sd->url);
			has_error = true;
		}
	} else if (strcmp(imgType, "GIF") == 0) {
		if (!imageparser->validate_gif_emblem(img, img_size)) {
			ShowError("wrong gif image %d: %s\n", fd, sd->url);
			has_error = true;
		}
		is_gif = true;
	} else {
		ShowError("unknown image type '%s'. %d: %s\n", imgType, fd, sd->url);
		has_error = true;
	}

	if (has_error) {
		// Not sure if intentional, but kRO sends status 500
		httpsender->send_json_text(fd, "{\"Type\":4}", HTTP_STATUS_INTERNAL_SERVER_ERROR);
		return false;
	}

	CREATE_HTTP_DATA(data, emblem_upload_guild_id);
	data.guild_id = RET_INT_HEADER(GUILD_ID, 0);
	data.is_gif = is_gif;
	SEND_CHAR_ASYNC_DATA(emblem_upload_guild_id, &data);
	SEND_CHAR_ASYNC_DATA_SPLIT(emblem_upload, img, img_size);

	return true;
}

HTTP_DATA(emblem_download)
{
#ifdef DEBUG_LOG
	ShowError("emblem_download data called\n");
#endif

	GET_HTTP_DATA(p, emblem_download);
	const size_t src_emblem_size = data_size - CHUNKED_FLAG_SIZE;

	RFIFO_CHUNKED_INIT(p, src_emblem_size, sd->data);

	RFIFO_CHUNKED_ERROR(p) {
		ShowError("Wrong guild emblem packets order\n");
		aclif->terminate_connection(fd);
		return;
	}

	RFIFO_CHUNKED_COMPLETE(p) {
		httpsender->send_binary(fd, sd->data.data, sd->data.data_size);
		aclif->terminate_connection(fd);
	}
}

HTTP_URL(emblem_download)
{
#ifdef DEBUG_LOG
	ShowInfo("emblem_download called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	CREATE_HTTP_DATA(data, emblem_download);

	data.guild_id = RET_INT_HEADER(GUILD_ID, 0);
	data.version = RET_INT_HEADER(VERSION, 0);

	SEND_CHAR_ASYNC_DATA(emblem_download, &data);

	return true;
}

HTTP_DATA(party_list)
{
	GET_HTTP_DATA(p, party_list);
	int index = 0;
	JsonW *json = jsonwriter->create_empty();
	jsonwriter->add_new_number(json, "totalPage", p->totalPage);
	JsonW *dataNode = jsonwriter->add_new_array(json, "data");

	while (index < ADVENTURER_AGENCY_PAGE_SIZE) {
		const struct adventuter_agency_entry *entry = &p->data.entry[index];
		if (entry->char_id == 0)
			break;
		JsonW *objNode = jsonwriter->add_new_object_to_array(dataNode);
		jsonwriter->add_new_number(objNode, "AID", entry->account_id);
		jsonwriter->add_new_number(objNode, "GID", entry->char_id);
		jsonwriter->add_new_string(objNode, "CharName", entry->char_name);
		jsonwriter->add_new_string(objNode, "WorldName", sd->world_name);
		jsonwriter->add_new_number(objNode, "Tanker", (entry->flags & AGENCY_TANKER) ? 1 : 0);
		jsonwriter->add_new_number(objNode, "Dealer", (entry->flags & AGENCY_DEALER) ? 1 : 0);
		jsonwriter->add_new_number(objNode, "Healer", (entry->flags & AGENCY_HEALER) ? 1 : 0);
		jsonwriter->add_new_number(objNode, "Assist", (entry->flags & AGENCY_ASSIST) ? 1 : 0);
		jsonwriter->add_new_number(objNode, "MinLV", entry->min_level);
		jsonwriter->add_new_number(objNode, "MaxLV", entry->max_level);
		jsonwriter->add_new_number(objNode, "Type", entry->type);
		jsonwriter->add_new_string(objNode, "Memo", entry->message);
		index ++;
	}

#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif

	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	aclif->terminate_connection(fd);
}

HTTP_URL(party_list)
{
#ifdef DEBUG_LOG
	ShowInfo("party_list called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	CREATE_HTTP_DATA(data, party_list);
	data.page = RET_INT_HEADER(PAGE, 0);
	SEND_CHAR_ASYNC_DATA(party_list, &data);

	return true;
}

HTTP_DATA(party_get)
{
	GET_HTTP_DATA(p, party_get);

	JsonW *json = jsonwriter->create("{\"Type\":1}");
	if (p->data.char_id != 0) {
		JsonW *dataNode = jsonwriter->add_new_object(json, "data");
		const struct adventuter_agency_entry *entry = &p->data;
		jsonwriter->add_new_number(dataNode, "AID", entry->account_id);
		jsonwriter->add_new_number(dataNode, "GID", entry->char_id);
		jsonwriter->add_new_string(dataNode, "CharName", entry->char_name);
		jsonwriter->add_new_string(dataNode, "WorldName", sd->world_name);
		jsonwriter->add_new_number(dataNode, "Tanker", (entry->flags & AGENCY_TANKER) ? 1 : 0);
		jsonwriter->add_new_number(dataNode, "Dealer", (entry->flags & AGENCY_DEALER) ? 1 : 0);
		jsonwriter->add_new_number(dataNode, "Healer", (entry->flags & AGENCY_HEALER) ? 1 : 0);
		jsonwriter->add_new_number(dataNode, "Assist", (entry->flags & AGENCY_ASSIST) ? 1 : 0);
		jsonwriter->add_new_number(dataNode, "MinLV", entry->min_level);
		jsonwriter->add_new_number(dataNode, "MaxLV", entry->max_level);
		jsonwriter->add_new_number(dataNode, "Type", entry->type);
		jsonwriter->add_new_string(dataNode, "Memo", entry->message);
	}

#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif

	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	aclif->terminate_connection(fd);
}

HTTP_URL(party_get)
{
#ifdef DEBUG_LOG
	ShowInfo("party_get called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	SEND_CHAR_ASYNC_DATA_EMPTY(party_get, NULL);

	return true;
}

HTTP_DATA(party_add)
{
	GET_HTTP_DATA(p, party_add);
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

HTTP_URL(party_add)
{
#ifdef DEBUG_LOG
	ShowInfo("party_add called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	char *text = NULL;
	CREATE_HTTP_DATA(data, party_add);

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

	SEND_CHAR_ASYNC_DATA(party_add, &data);

	return true;
}

HTTP_DATA(party_del)
{
	GET_HTTP_DATA(p, party_del);

	JsonW *json = jsonwriter->create_empty();
	jsonwriter->add_new_number(json, "Type", p->type);

#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif

	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	aclif->terminate_connection(fd);
}

HTTP_URL(party_del)
{
#ifdef DEBUG_LOG
	ShowInfo("party_del called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	CREATE_HTTP_DATA(data, party_del);
	data.master_aid = RET_INT_HEADER(MASTER_AID, 0);
	SEND_CHAR_ASYNC_DATA(party_del, &data);

	return true;
}

HTTP_DATA(party_info)
{
	GET_HTTP_DATA(p, party_info);

	JsonW *json = jsonwriter->create_empty();
	jsonwriter->add_new_number(json, "Type", p->type);

#ifdef DEBUG_LOG
	jsonwriter->print(json);
#endif

	httpsender->send_json(fd, json);
	jsonwriter->delete(json);
	aclif->terminate_connection(fd);
}

HTTP_URL(party_info)
{
#ifdef DEBUG_LOG
	ShowInfo("party_info called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif
#ifdef REQUEST_LOG
	aclif->show_request(fd, sd, false);
#endif
	CREATE_HTTP_DATA(data, party_info);
	data.account_id = sd->account_id;
	data.master_aid = RET_INT_HEADER(QUERY_AID, 0);
	SEND_MAP_ASYNC_DATA(party_info, &data);

	return true;
}

HTTP_URL(test_url)
{
#ifdef DEBUG_LOG
	ShowInfo("test_url called %d: %s\n", fd, httpparser->get_method_str(sd));
#endif

	char buf[1000];
	const char *user_agent = (const char*)strdb_get(sd->headers_db, "User-Agent");
	const char *format = "<html>Hercules test.<br/>Your user agent is: %s<br/></html>\n";
	snprintf(buf, sizeof(buf), format, user_agent);

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
