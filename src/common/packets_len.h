/**
 * This file is part of Hercules.
 * https://herc.ws - https://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef COMMON_PACKETS_LEN_H
#define COMMON_PACKETS_LEN_H

#if defined(PACKETVER_ZERO)
#include "common/packets/packets_len_zero.h"
#elif defined(PACKETVER_RE)
#include "common/packets/packets_len_re.h"
#elif defined(PACKETVER_SAK)
#include "common/packets/packets_len_sak.h"
#elif defined(PACKETVER_AD)
#include "common/packets/packets_len_ad.h"
#else
#include "common/packets/packets_len_main.h"
#endif

#endif /* COMMON_PACKETS_LEN_H */
