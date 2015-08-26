// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_HOMUN_H
#define CHAR_INT_HOMUN_H

#include "common/hercules.h"

/**
 * inter_homunculus interface
 **/
struct inter_homunculus_interface {
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*parse_frommap) (int fd);
};

#ifdef HERCULES_CORE
void inter_homunculus_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_homunculus_interface *inter_homunculus;

#endif /* CHAR_INT_HOMUN_H */
