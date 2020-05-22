/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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

#include "config/core.h" // ANTI_MAYAP_CHEAT, RENEWAL, SECURE_NPCTIMEOUT
#include "api/aclif.h"

#include "common/HPM.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/ers.h"
#include "common/grfio.h"
#include "common/memmgr.h"
#include "common/mmo.h" // NEW_CARTS, char_achievements
#include "common/nullpo.h"
#include "common/packets.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "common/utils.h"
#include "api/apisessiondata.h"
#include "api/httpparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static struct aclif_interface aclif_s;
struct aclif_interface *aclif;

static bool aclif_setip(const char *ip)
{
	char ip_str[16];
	nullpo_retr(false, ip);
	aclif->api_ip = sockt->host2ip(ip);
	if (!aclif->api_ip) {
		ShowWarning("Failed to resolve Api server address! (%s)\n", ip);
		return false;
	}

	safestrncpy(aclif->api_ip_str, ip, sizeof(aclif->api_ip_str));
	ShowInfo("Api server IP address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, sockt->ip2str(aclif->api_ip, ip_str));
	return true;
}

/*==========================================
 * Main client packet processing function
 *------------------------------------------*/
static int aclif_parse(int fd)
{
	ShowInfo("parse called: %d\n", fd);
	if (!httpparser->parse(fd))
	{
		ShowError("http parser error: %d\n", fd);
		sockt->eof(fd);
		sockt->close(fd);
		return 0;
	}
	if (sockt->session[fd]->flag.eof != 0) {
		ShowInfo("closed: %d\n", fd);
		sockt->close(fd);
	}

	return 0;
}

static int aclif_connected(int fd)
{
	ShowInfo("connected called: %d\n", fd);

	nullpo_ret(sockt->session[fd]);
	struct api_session_data *sd = NULL;
	CREATE(sd, struct api_session_data, 1);
	sd->fd = fd;
	sockt->session[fd]->session_data = sd;
	httpparser->init_parser(fd, sd);
	return 0;
}

static int aclif_session_delete(int fd)
{
	nullpo_ret(sockt->session[fd]);
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_ret(sd);
	aFree(sd->url);

	httpparser->delete_parser(fd);
	return 0;
}

static void aclif_load_handlers(void)
{
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		aclif->handlers_db[i] = strdb_alloc(DB_OPT_BASE | DB_OPT_RELEASE_DATA, MAX_URL_SIZE);
	}
#define handler(method, url, func) aclif->add_handler(method, url, func)
#include "api/handlers.h"
#undef handler
}

static void aclif_add_handler(enum http_method method, const char *url, HttpParseHandler func)
{
	nullpo_retv(url);
	nullpo_retv(func);
	Assert_retv(method >= 0 && method < HTTP_MAX_PROTOCOL);

	ShowWarning("Add url: %s\n", url);
	struct HttpHandler *handler = aCalloc(1, sizeof(struct HttpHandler));
	handler->method = method;
	handler->func = func;

	strdb_put(aclif->handlers_db[method], url, handler);
}

void aclif_set_url(int fd, enum http_method method, const char *url, size_t size)
{
	nullpo_retv(url);
	Assert_retv(method >= 0 && method < HTTP_MAX_PROTOCOL);

	if (size > MAX_URL_SIZE) {
		sockt->close(fd);
		return;
	}
	struct api_session_data *sd = sockt->session[fd]->session_data;
	nullpo_retv(sd);

	aFree(sd->url);
	sd->url = aMalloc(size + 1);
	safestrncpy(sd->url, url, size + 1);

	struct HttpHandler *handler = strdb_get(aclif->handlers_db[method], sd->url);
	if (handler == NULL) {
		ShowWarning("Unknown url %d: %s\n", fd, sd->url);
		sockt->eof(fd);
		return;
	}
	if (handler->func == NULL) {
		ShowError("found NULL handler for url %d: %s\n", fd, url);
		Assert_report(0);
		sockt->eof(fd);
		return;
	}

	sd->flag.url = 1;

	ShowWarning("url: %s\n", sd->url);
}


// parsers

static bool aclif_parse_userconfig_load(int fd, struct api_session_data *sd)
{
	nullpo_retr(false, sd);
	ShowInfo("aclif_parse_userconfig_load called: %d\n", sd->parser.method);
	return true;
}

static int do_init_aclif(bool minimal)
{
	if (minimal)
		return 0;

	sockt->set_defaultparse(aclif->parse);
	sockt->set_default_client_connected(aclif->connected);
	sockt->set_default_delete(aclif->session_delete);
	sockt->validate = false;
	if (sockt->make_listen_bind(aclif->bind_ip, aclif->api_port) == -1) {
		ShowFatalError("Failed to bind to port '"CL_WHITE"%d"CL_RESET"'\n", aclif->api_port);
		exit(EXIT_FAILURE);
	}

	aclif->load_handlers();

	return 0;
}

static void do_final_aclif(void)
{
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		db_destroy(aclif->handlers_db[i]);
		aclif->handlers_db[i] = NULL;
	}
}

void aclif_defaults(void)
{
	aclif = &aclif_s;
	/* vars */
	aclif->bind_ip = INADDR_ANY;
	aclif->api_port = 7000;
	for (int i = 0; i < HTTP_MAX_PROTOCOL; i ++) {
		aclif->handlers_db[i] = NULL;
	}
	/* core */
	aclif->init = do_init_aclif;
	aclif->final = do_final_aclif;
	aclif->setip = aclif_setip;
	aclif->parse = aclif_parse;
	aclif->connected = aclif_connected;
	aclif->session_delete = aclif_session_delete;
	aclif->load_handlers = aclif_load_handlers;
	aclif->add_handler = aclif_add_handler;
	aclif->set_url = aclif_set_url;

	aclif->parse_userconfig_load = aclif_parse_userconfig_load;
}
