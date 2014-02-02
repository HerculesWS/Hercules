// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_LOGINLOG_H_
#define _LOGIN_LOGINLOG_H_


unsigned long loginlog_failedattempts(uint32 ip, unsigned int minutes);
void login_log(uint32 ip, const char* username, int rcode, const char* message);
bool loginlog_init(void);
bool loginlog_final(void);
bool loginlog_config_read(const char* w1, const char* w2);


#endif /* _LOGIN_LOGINLOG_H_ */
