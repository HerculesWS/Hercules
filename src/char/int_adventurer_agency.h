/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) 2020-2021 Andrei Karas (4144)
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
#ifndef CHAR_INT_ADVENTURER_AGENCY_H
#define CHAR_INT_ADVENTURER_AGENCY_H

#include "common/hercules.h"

struct party_add_data;

enum adventurer_agency_flags {
	AGENCY_HEALER = 1,
	AGENCY_ASSIST = 2,
	AGENCY_TANKER = 4,
	AGENCY_DEALER = 8
};

/**
 * inter_adventurer_agency_interface interface
 **/
struct inter_adventurer_agency_interface {
	bool (*entry_add) (int char_id, const struct party_add_data *entry);
	bool (*check_existing) (int char_id, int party_id);
	bool (*entry_tosql) (int char_id, int party_id, const struct party_add_data *entry);
	int (*entry_to_flags) (int char_id, const struct party_add_data *entry);
};

#ifdef HERCULES_CORE
void inter_adventurer_agency_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_adventurer_agency_interface *inter_adventurer_agency;

#endif /* CHAR_INT_ADVENTURER_AGENCY_H */
