// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_MAIL_H
#define CHAR_INT_MAIL_H

#include "../common/cbasetypes.h"

struct item;
struct mail_data;
struct mail_message;

#ifdef HERCULES_CORE
void inter_mail_defaults(void);
#endif // HERCULES_CORE

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
};

struct inter_mail_interface *inter_mail;

#endif /* CHAR_INT_MAIL_H */
