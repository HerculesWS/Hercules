// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_PINCODE_H
#define CHAR_PINCODE_H

#include "char.h"

#define PINCODE_OK 0
#define PINCODE_ASK 1
#define PINCODE_NOTSET 2
#define PINCODE_EXPIRED 3
#define PINCODE_UNUSED 7
#define	PINCODE_WRONG 8

/* Pincode Interface */
struct pincode_interface {
	/* vars */
	int *enabled;
	int *changetime;
	int *maxtry;
	int *charselect;
	unsigned int *multiplier;
	unsigned int *baseSeed;
	/* handler */
	void (*handle) (int fd, struct char_session_data* sd);
	void (*decrypt) (unsigned int userSeed, char* pin);
	void (*error) (int account_id);
	void (*update) (int account_id, char* pin);
	void (*sendstate) (int fd, struct char_session_data* sd, uint16 state);
	void (*setnew) (int fd, struct char_session_data* sd);
	void (*change) (int fd, struct char_session_data* sd);
	int  (*compare) (int fd, struct char_session_data* sd, char* pin);
	void (*check) (int fd, struct char_session_data* sd);
	bool (*config_read) (char *w1, char *w2);
} pincode_s;

struct pincode_interface *pincode;

void pincode_defaults(void);

#endif /* CHAR_PINCODE_H */
