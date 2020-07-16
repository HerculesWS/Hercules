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

#include "capiif.h"

#include "char/char.h"
#include "char/mapif.h"
#include "common/api.h"
#include "common/cbasetypes.h"
#include "common/core.h"
#include "common/db.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/timer.h"

#include <stdlib.h>
#include <string.h>

static struct capiif_interface capiif_s;
struct capiif_interface *capiif;

#define WFIFO_CHARAPI_SIZE 22

#define WFIFO_APICHAR_PACKET_REPLY(len) \
	WFIFOHEAD(chr->login_fd, len); \
	memcpy(WFIFOP(chr->login_fd, 0), RFIFOP(fd, 0), WFIFO_CHARAPI_SIZE); \
	WFIFOW(chr->login_fd, 0) = 0x2818; \
	WFIFOW(chr->login_fd, 2) = len;

static int capiif_parse_fromlogin_api_proxy(int fd)
{
	const uint command = RFIFOW(fd, 4);

	switch (command) {
		case API_MSG_userconfig_load:
			capiif->parse_userconfig_load(fd);
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
	const int len = WFIFO_CHARAPI_SIZE + 4;
	WFIFO_APICHAR_PACKET_REPLY(len)

	int offset = WFIFO_CHARAPI_SIZE;
	WFIFOW(chr->login_fd, offset) = 0x1122;
	WFIFOSET(chr->login_fd, len);
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
	capiif->parse_fromlogin_api_proxy = capiif_parse_fromlogin_api_proxy;
	capiif->parse_userconfig_load = capiif_parse_userconfig_load;
}
