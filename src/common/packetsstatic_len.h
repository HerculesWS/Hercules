/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2022 Hercules Dev Team
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
#ifndef COMMON_PACKETSSTATIC_LEN_H
#define COMMON_PACKETSSTATIC_LEN_H

#ifdef packetLen
#error packetLen already defined
#endif

#define DEFINE_PACKET_HEADER(name, id) \
	STATIC_ASSERT((int32)(PACKET_LEN_##id) == -1 || sizeof(struct PACKET_##name) == \
		(size_t)PACKET_LEN_##id, "Wrong size PACKET_"#name); \
	enum { HEADER_##name = id };

#define DEFINE_PACKET_ID(name, id) \
	enum { HEADER_##name = id };

#define CHECK_PACKET_HEADER(name, id) \
	STATIC_ASSERT((int32)(PACKET_LEN_##id) == -1 || sizeof(struct PACKET_##name) == \
		(size_t)PACKET_LEN_##id, "Wrong size PACKET_"#name); \

#define packetLen(id, len) PACKET_LEN_##id = (len),
enum packet_lengths {
#include "common/packets_len.h"
};
#undef packetLen

#endif /* COMMON_PACKETSSTATIC_LEN_H */
