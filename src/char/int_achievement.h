/**
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2017-2021 Hercules Dev Team
* Copyright (C) Smokexyz
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
#ifndef CHAR_INT_ACHIEVEMENT_H
#define CHAR_INT_ACHIEVEMENT_H

#include "common/hercules.h"
#include "common/db.h"

struct achievement;
struct char_achievements;

/**
 * inter_achievement Interface
 */
struct inter_achievement_interface {
	struct DBMap *char_achievements;
	/* */
	int (*sql_init) (void);
	void (*sql_final) (void);
	/* */
	int (*tosql) (int char_id, struct char_achievements *cp, const struct char_achievements *p);
	bool (*fromsql) (int char_id, struct char_achievements *a);
	/* */
	struct DBData(*ensure_char_achievements) (union DBKey key, va_list args);
	int (*char_achievements_clear) (union DBKey key, struct DBData *data, va_list args);
	/* */
	int (*parse_frommap) (int fd);
};

#ifdef HERCULES_CORE
void inter_achievement_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_achievement_interface *inter_achievement;
#endif /* CHAR_INT_ACHIEVEMENT_H */
