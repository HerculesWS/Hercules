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
#ifndef CHAR_INT_MAIL_H
#define CHAR_INT_MAIL_H

#include "common/hercules.h"

struct item;
struct mail_data;
struct mail_message;

/**
 * inter_mail interface
 **/
struct inter_mail_interface {
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*parse_frommap) (int fd);
	int (*fromsql) (int char_id, struct mail_data* md);
	int (*savemessage) (struct mail_message* msg);
	bool (*loadmessage) (int mail_id, struct mail_message* msg);
	bool (*DeleteAttach) (int mail_id);
	void (*sendmail) (int send_id, const char* send_name, int dest_id, const char* dest_name, const char* title, const char* body, int zeny, struct item *item);
	bool (*mark_read) (int mail_id);
	bool (*get_attachment) (int char_id, int mail_id, struct mail_message *msg);
	bool (*delete) (int char_id, int mail_id);
	bool (*return_message) (int char_id, int mail_id, int *new_mail);
	bool (*send) (int account_id, struct mail_message *msg);
};

#ifdef HERCULES_CORE
void inter_mail_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_mail_interface *inter_mail;

#endif /* CHAR_INT_MAIL_H */
