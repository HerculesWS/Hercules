// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LOGIN_IPBAN_H
#define LOGIN_IPBAN_H

#include "../common/cbasetypes.h"

#ifdef HERCULES_CORE
// TODO: Interface
// initialize
void ipban_init(void);

// finalize
void ipban_final(void);

// check ip against ban list
bool ipban_check(uint32 ip);

// increases failure count for the specified IP
void ipban_log(uint32 ip);

// parses configuration option
bool ipban_config_read(const char *key, const char* value);
#endif // HERCULES_CORE

#endif /* LOGIN_IPBAN_H */
