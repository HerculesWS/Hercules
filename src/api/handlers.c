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
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "api/aclif.h"
#include "api/aloginif.h"
#include "api/apisessiondata.h"
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

#define GET_DATA(var, type) const struct PACKET_API_REPLY_ ## type *var = (const struct PACKET_API_REPLY_ ## type*)data;
#define LOAD_ASYNC_DATA(name, data) aloginif->send_to_char(fd, sd, API_MSG_ ## name, data, sizeof(struct PACKET_API_ ## name));

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
		p->emote[0], p->emote[1], p->emote[2], p->emote[3], p->emote[4],
		p->emote[5], p->emote[6], p->emote[7], p->emote[8], p->emote[9],
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
	ShowInfo("userconfig_load called %d: %d\n", fd, sd->parser.method);
#endif
	aclif->show_request(fd, sd, false);

	LOAD_ASYNC_DATA(userconfig_load, NULL);

	return true;
}

DATA(userconfig_save)
{
	aclif->terminate_connection(fd);
}

HTTPURL(userconfig_save)
{
#ifdef DEBUG_LOG
	ShowInfo("userconfig_save called %d: %d\n", fd, sd->parser.method);
#endif

	aclif->show_request(fd, sd, false);

	LOAD_ASYNC_DATA(userconfig_save, NULL);

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
	ShowInfo("charconfig_load called %d: %d\n", fd, sd->parser.method);
#endif
	aclif->show_request(fd, sd, false);

	LOAD_ASYNC_DATA(charconfig_load, NULL);

	return true;
}

HTTPURL(test_url)
{
#ifdef DEBUG_LOG
	ShowInfo("test_url called %d: %d\n", fd, sd->parser.method);
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
#include "api/urlhandlers.h"
#undef handler
#undef handler2
}