/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2020 Hercules Dev Team
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
#ifndef COMMON_PACKETS_H
#define COMMON_PACKETS_H

#include "common/hercules.h"

#ifndef MIN_PACKET_DB
#define MIN_PACKET_DB 0x0064
#endif

#ifndef MAX_PACKET_DB
#define MAX_PACKET_DB 0x0F00
#endif

struct packets_interface {
	void (*init) (void);
	void (*final) (void);
	void (*addLens) (void);
	void (*addLen) (int id, int len);
	int db[MAX_PACKET_DB + 1];
};

#ifdef HERCULES_CORE
void packets_defaults(void);
#endif // HERCULES_CORE

HPShared struct packets_interface *packets;

#endif /* COMMON_PACKETS_H */
