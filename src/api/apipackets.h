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
#ifndef API_APIPACKETS_H
#define API_APIPACKETS_H

#include "common/apipackets.h"
#include "api/aloginif.h"

#define GET_DATA(var, type) const struct PACKET_API_REPLY_ ## type *var = (const struct PACKET_API_REPLY_ ## type*)data;
#define CREATE_DATA(var, type) struct PACKET_API_ ## type ## _data var = { 0 };
#define SEND_ASYNC_DATA(name, data) aloginif->send_to_char(fd, sd, API_MSG_ ## name, data, sizeof(struct PACKET_API_ ## name));
// SEND_ASYNC_DATA_EMPTY is workaround for visual studio bugs
#define SEND_ASYNC_DATA_EMPTY(name, data) aloginif->send_to_char(fd, sd, API_MSG_ ## name, data, 0);
#define SEND_ASYNC_DATA_SPLIT(name, data, size) aloginif->send_split_to_char(fd, sd, API_MSG_ ## name, data, size);

#endif /* API_APIPACKETS_H */
