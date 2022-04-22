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

#include "mapiif.h"

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

static struct mapiif_interface mapiif_s;
struct mapiif_interface *mapiif;

#define DEBUG_LOG

static int mapiif_parse_fromchar_api_proxy(int fd)
{
	RFIFO_API_PROXY_PACKET(packet);
	const uint32 msg = packet->msg_id;

	switch (msg) {
		default:
			ShowError("Unknown proxy packet 0x%04x received from char-server, disconnecting.\n", msg);
			sockt->eof(fd);
			return 0;
	}

	RFIFOSKIP(fd, packet->packet_len);
	return 0;
}

static void do_init_mapiif(bool minimal)
{
}

static void do_final_mapiif(void)
{
}

void mapiif_defaults(void) {
	mapiif = &mapiif_s;

	mapiif->init = do_init_mapiif;
	mapiif->final = do_final_mapiif;
	mapiif->parse_fromchar_api_proxy = mapiif_parse_fromchar_api_proxy;
}
