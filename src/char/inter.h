// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _CHAR_INTER_H_
#define _CHAR_INTER_H_

#include "char.h"
#include "../common/sql.h"

struct accreg;

int inter_init_sql(const char *file);
void inter_final(void);
int inter_parse_frommap(int fd);
int inter_mapif_init(int fd);
int mapif_send_gmaccounts(void);
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason);
void mapif_parse_accinfo2(bool success, int map_fd, int u_fd, int u_aid, int account_id, const char *userid, const char *user_pass, const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate, int group_id, int logincount, int state);

int inter_log(char *fmt,...);
int inter_vlog(char *fmt, va_list ap);

#define inter_cfgName "conf/inter-server.conf"

extern unsigned int party_share_level;

extern Sql* sql_handle;
extern Sql* lsql_handle;

int inter_accreg_tosql(int account_id, int char_id, struct accreg *reg, int type);

#endif /* _CHAR_INTER_H_ */
