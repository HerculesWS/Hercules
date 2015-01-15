// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_ELEMENTAL_H
#define CHAR_INT_ELEMENTAL_H

#include "../common/cbasetypes.h"

#ifdef HERCULES_CORE
void inter_elemental_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_elemental_interface interface
 **/
struct inter_elemental_interface {
	void (*sql_init) (void);
	void (*sql_final) (void);
	int (*parse_frommap) (int fd);
};

struct inter_elemental_interface *inter_elemental;

#endif /* CHAR_INT_ELEMENTAL_H */
