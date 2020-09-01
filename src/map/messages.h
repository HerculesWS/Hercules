/**
 * This file is part of Hercules.
 * https://herc.ws - https://github.com/HerculesWS/Hercules
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MAP_MESSAGES_H
#define MAP_MESSAGES_H

#if defined(PACKETVER_ZERO)
#include "map/messages_zero.h"
#elif defined(PACKETVER_RE)
#include "map/messages_re.h"
#elif defined(PACKETVER_SAK)
#include "map/messages_sak.h"
#elif defined(PACKETVER_AD)
#include "map/messages_ad.h"
#else
#include "map/messages_main.h"
#endif

#endif /* MAP_MESSAGES_H */
