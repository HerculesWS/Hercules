// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _LOGIN_IPBAN_H_
#define _LOGIN_IPBAN_H_

#include "../common/cbasetypes.h"

// initialize
void ipban_init(void);

// finalize
void ipban_final(void);

// check ip against ban list
bool ipban_check(uint32 ip);

// increases failure count for the specified IP
void ipban_log(uint32 ip);

// parses configuration option
bool ipban_config_read(const char* key, const char* value);


#endif /* _LOGIN_IPBAN_H_ */
