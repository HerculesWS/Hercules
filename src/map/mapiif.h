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
#ifndef MAP_MAPIIF_H
#define MAP_MAPIIF_H

#include "common/hercules.h"

struct PACKET_API_PROXY;

/**
 * mapiif interface
 **/
struct mapiif_interface {
	void (*init) (bool minimal);
	void (*final) (void);
	int (*parse_fromchar_api_proxy) (int fd);
	void (*parse_adventurer_agency_info) (int fd);
};

#ifdef HERCULES_CORE
void mapiif_defaults(void);
#endif // HERCULES_CORE

HPShared struct mapiif_interface *mapiif;

#endif /* MAP_MAPIIF_H */
