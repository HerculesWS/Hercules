/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017 Hercules Dev Team
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
#ifndef CHAR_INT_RODEX_H
#define CHAR_INT_RODEX_H

#include "common/mmo.h"
#include "common/db.h"

struct item;

/**
 * inter_rodex interface
 **/
struct inter_rodex_interface {
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*parse_frommap) (int fd);
	int (*fromsql) (int char_id, int account_id, int8 opentype, int64 mail_id, struct rodex_maillist *mails);
	bool (*hasnew) (int char_id, int account_id);
	bool (*checkname) (const char *name, int *target_char_id, short *target_class, int *target_level);
	int64 (*savemessage) (struct rodex_message* msg);
};

#ifdef HERCULES_CORE
void inter_rodex_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_rodex_interface *inter_rodex;

#endif /* CHAR_INT_RODEX_H */
