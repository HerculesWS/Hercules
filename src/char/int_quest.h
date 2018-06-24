/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2018  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#ifndef CHAR_INT_QUEST_H
#define CHAR_INT_QUEST_H

#include "common/hercules.h"

/**
 * inter_quest interface
 **/
struct inter_quest_interface {
	int (*parse_frommap) (int fd);

	struct quest *(*fromsql) (int char_id, int *count);
	bool (*delete) (int char_id, int quest_id);
	bool (*add) (int char_id, struct quest qd);
	bool (*update) (int char_id, struct quest qd);
	bool (*save) (int char_id, const struct quest *new_qd, int new_n);
};

#ifdef HERCULES_CORE
void inter_quest_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_quest_interface *inter_quest;

#endif /* CHAR_INT_QUEST_H */
