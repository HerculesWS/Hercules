/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2023 Hercules Dev Team
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

#include "config/core.h" // CONSOLE_INPUT, MAX_CONSOLE_INPUT
#include "common/packets.h"

#include "common/cbasetypes.h"
#include "common/mmo.h"
#include "common/nullpo.h"

#include <string.h>

static struct packets_interface packets_s;
struct packets_interface *packets;

static void packets_init(void)
{
	packets->addLens();
}

static void packets_addLens(void)
{
#define packetLen(id, len) packets->addLen(id, len);
#include "common/packets_len.h"
#undef packetLen
#define packetLen(id, len) packets->addLenIntif(id, len);
#include "common/packets_intif_len.h"
#undef packetLen
#define packetLen(id, len) packets->addLenInter(id, len);
#include "common/packets_inter_len.h"
#undef packetLen
#define packetLen(id, len) packets->addLenChrif(id, len);
#include "common/packets_chrif_len.h"
#undef packetLen
}

static void packets_addLen(int id, int len)
{
	Assert_retv(id <= MAX_PACKET_DB && id >= MIN_PACKET_DB);
	packets->db[id] = len;
}

static void packets_addLenIntif(int id, int len)
{
	Assert_retv(id <= MAX_INTIF_PACKET_DB && id >= MIN_INTIF_PACKET_DB);
	packets->intif_db[id - MIN_INTIF_PACKET_DB] = len;
}

static void packets_addLenInter(int id, int len)
{
	Assert_retv(id <= MAX_INTER_PACKET_DB && id >= MIN_INTER_PACKET_DB);
	packets->inter_db[id - MIN_INTER_PACKET_DB] = len;
}

static void packets_addLenChrif(int id, int len)
{
	Assert_retv(id <= MAX_CHRIF_PACKET_DB && id >= MIN_CHRIF_PACKET_DB);
	packets->chrif_db[id - MIN_CHRIF_PACKET_DB] = len;
}

static void packets_final(void)
{
}

void packets_defaults(void)
{
	packets = &packets_s;
	packets->init = packets_init;
	packets->final = packets_final;
	packets->addLens = packets_addLens;
	packets->addLen = packets_addLen;
	packets->addLenIntif = packets_addLenIntif;
	packets->addLenInter = packets_addLenInter;
	packets->addLenChrif = packets_addLenChrif;

	memset(&packets->db, 0, sizeof(packets->db));
	memset(&packets->intif_db, 0, sizeof(packets->intif_db));
	memset(&packets->inter_db, 0, sizeof(packets->inter_db));
	memset(&packets->chrif_db, 0, sizeof(packets->chrif_db));
}
