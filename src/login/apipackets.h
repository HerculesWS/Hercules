/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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
#ifndef LOGIN_APIPACKETS_H
#define LOGIN_APIPACKETS_H

#include "common/apipackets.h"
#include "map/chrif.h"

#define WFIFO_APILOGIN_PACKET_REPLY_EMPTY(wfd) \
	WFIFOHEAD((wfd), WFIFO_APICHAR_SIZE); \
	memcpy(WFIFOP((wfd), 0), RFIFOP(fd, 0), WFIFO_APICHAR_SIZE); \
	struct PACKET_API_PROXY *packet = WFIFOP((wfd), 0); \
	packet->packet_id = HEADER_API_PROXY_REPLY; \
	packet->packet_len = WFIFO_APICHAR_SIZE; \

#define WFIFO_APILOGIN_PACKET_REPLY(wfd, type) \
	WFIFOHEAD((wfd), WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_REPLY_ ## type)); \
	memcpy(WFIFOP((wfd), 0), RFIFOP(fd, 0), WFIFO_APICHAR_SIZE); \
	struct PACKET_API_PROXY *packet = WFIFOP((wfd), 0); \
	packet->packet_id = HEADER_API_PROXY_REPLY; \
	packet->packet_len = WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_REPLY_ ## type); \
	struct PACKET_API_REPLY_ ## type *data = WFIFOP((wfd), sizeof(struct PACKET_API_PROXY))

#endif /* LOGIN_APIPACKETS_H */
