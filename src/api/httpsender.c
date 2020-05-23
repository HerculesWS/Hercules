/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2020 Hercules Dev Team
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
#include "api/httpsender.h"

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
#include "api/aclif.h"
#include "api/apisessiondata.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define WFIFOADDSTR(fd, str) \
    memcpy(WFIFOP(fd, 0), str, strlen(str)); \
    WFIFOSET(fd, strlen(str));

static struct httpsender_interface httpsender_s;
struct httpsender_interface *httpsender;

static int do_init_httpsender(bool minimal)
{
	return 0;
}

static void do_final_httpsender(void)
{
}

static bool httpsender_send_html(int fd, const char *data)
{
	nullpo_retr(false, data);

	const int sz = strlen(data);
	WFIFOHEAD(fd, 200 + sz);
	WFIFOADDSTR(fd, "HTTP/1.1 200 OK\n");
	WFIFOADDSTR(fd, "Server: herc.ws/1.0\n");
	WFIFOADDSTR(fd, "Content-Type: text/html\n");

	char buf[50];
	safesnprintf(buf, 50, "Content-Length: %d\n", sz);

	WFIFOADDSTR(fd, buf);
	WFIFOADDSTR(fd, "\n");
	WFIFOADDSTR(fd, data);
	sockt->flush(fd);
	return true;
}

void httpsender_defaults(void)
{
	httpsender = &httpsender_s;

	httpsender->init = do_init_httpsender;
	httpsender->final = do_final_httpsender;

	httpsender->send_html = httpsender_send_html;
}
