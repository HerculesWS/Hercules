/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017-2020 Hercules Dev Team
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
#ifndef CHAR_INT_CLAN_H
#define CHAR_INT_CLAN_H

#include "common/mmo.h"

/**
 * inter clan Interface
 **/
struct inter_clan_interface {
	int (*kick_inactive_members) (int clan_id, int kick_interval);
	int (*count_members) (int clan_id, int kick_interval);
	int (*parse_frommap) (int fd);
};

#ifdef HERCULES_CORE
void inter_clan_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_clan_interface *inter_clan;
#endif /* CHAR_INT_CLAN_H */
