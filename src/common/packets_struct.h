/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2022 Hercules Dev Team
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


#ifndef COMMON_PACKETS_STRUCT_H
#define COMMON_PACKETS_STRUCT_H

#include "common/cbasetypes.h"
#include "common/mmo.h"
#include "common/packetsstatic_len.h"

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

struct PACKET_INTER_CREATE_PET {
	int16 packet_id;
	int account_id;
	int char_id;
	int pet_class;
	int pet_lv;
	int pet_egg_id;
	int pet_equip;
	int16 intimate;
	int16 hungry;
	char rename_flag;
	char incubate;
	char pet_name[NAME_LENGTH];
};
DEFINE_PACKET_ID(INTER_CREATE_PET, 0x3080);

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* COMMON_PACKETS_STRUCT_H */
