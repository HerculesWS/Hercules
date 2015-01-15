// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_MERCENARY_H
#define CHAR_INT_MERCENARY_H

#include "../common/cbasetypes.h"

struct mmo_charstatus;

#ifdef HERCULES_CORE
void inter_mercenary_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_mercenary interface
 **/
struct inter_mercenary_interface {
	bool (*owner_fromsql) (int char_id, struct mmo_charstatus *status);
	bool (*owner_tosql) (int char_id, struct mmo_charstatus *status);
	bool (*owner_delete) (int char_id);
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*parse_frommap) (int fd);
};

struct inter_mercenary_interface *inter_mercenary;

#endif /* CHAR_INT_MERCENARY_H */
