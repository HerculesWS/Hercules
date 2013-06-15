// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAIL_H_
#define _MAIL_H_

#include "../common/mmo.h"

struct mail_interface {
	void (*clear) (struct map_session_data *sd);
	int (*removeitem) (struct map_session_data *sd, short flag);
	int (*removezeny) (struct map_session_data *sd, short flag);
	unsigned char (*setitem) (struct map_session_data *sd, int idx, int amount);
	bool (*setattachment) (struct map_session_data *sd, struct mail_message *msg);
	void (*getattachment) (struct map_session_data* sd, int zeny, struct item* item);
	int (*openmail) (struct map_session_data *sd);
	void (*deliveryfail) (struct map_session_data *sd, struct mail_message *msg);
	bool (*invalid_operation) (struct map_session_data *sd);
} mail_s;

struct mail_interface *mail;

#endif /* _MAIL_H_ */
