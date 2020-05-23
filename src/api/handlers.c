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
#include "api/handlers.h"

#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "api/apisessiondata.h"
#include "api/httpsender.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define HTTPURL(x) \
	static bool handlers_parse_ ## x(int fd, struct api_session_data *sd) __attribute__((nonnull (2))); \
	static bool handlers_parse_ ## x(int fd, struct api_session_data *sd)

static struct handlers_interface handlers_s;
struct handlers_interface *handlers;

HTTPURL(userconfig_load)
{
	ShowInfo("handlers_parse_userconfig_load called %d: %d\n", fd, sd->parser.method);

	ShowInfo("user agent: %s\n", (const char*)strdb_get(sd->headers_db, "User-Agent"));

	httpsender->send_html(fd, "<html>test line</html>\n");

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

	handlers->parse_userconfig_load = handlers_parse_userconfig_load;
}
