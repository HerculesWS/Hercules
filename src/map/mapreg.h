// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAPREG_H_
#define _MAPREG_H_

struct mapreg_save {
	int uid;
	union {
		int i;
		char *str;
	} u;
	bool save;
};

void mapreg_reload(void);
void mapreg_final(void);
void mapreg_init(void);
bool mapreg_config_read(const char* w1, const char* w2);

int mapreg_readreg(int uid);
char* mapreg_readregstr(int uid);
bool mapreg_setreg(int uid, int val);
bool mapreg_setregstr(int uid, const char* str);

#endif /* _MAPREG_H_ */
