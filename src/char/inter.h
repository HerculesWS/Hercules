// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef CHAR_INTER_H
#define CHAR_INTER_H

#include "char.h"
#include "../common/cbasetypes.h"
#include "../common/sql.h"

struct accreg;

#ifdef HERCULES_CORE
extern unsigned int party_share_level;

void inter_defaults(void);
#endif // HERCULES_CORE

/**
 * inter interface
 **/
struct inter_interface {
	Sql* sql_handle;
	const char* (*msg_txt) (int msg_number);
	bool (*msg_config_read) (const char *cfg_name, bool allow_override);
	void (*do_final_msg) (void);
	const char* (*job_name) (int class_);
	void (*vmsg_to_fd) (int fd, int u_fd, int aid, char* msg, va_list ap);
	void (*msg_to_fd) (int fd, int u_fd, int aid, char *msg, ...);
	void (*savereg) (int account_id, int char_id, const char *key, unsigned int index, intptr_t val, bool is_string);
	int (*accreg_fromsql) (int account_id,int char_id, int fd, int type);
	int (*config_read) (const char* cfgName);
	int (*vlog) (char* fmt, va_list ap);
	int (*log) (char* fmt, ...);
	int (*init_sql) (const char *file);
	int (*mapif_init) (int fd);
	int (*check_ttl_wisdata_sub) (DBKey key, DBData *data, va_list ap);
	int (*check_ttl_wisdata) (void);
	int (*check_length) (int fd, int length);
	int (*parse_frommap) (int fd);
	void (*final) (void);
};

struct inter_interface *inter;

#endif /* CHAR_INTER_H */
