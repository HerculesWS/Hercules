// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_AUCTION_H
#define CHAR_INT_AUCTION_H

#include "../common/mmo.h"

#ifdef HERCULES_CORE
void inter_auction_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_auction_interface interface
 **/
struct inter_auction_interface {
	DBMap* db; // int auction_id -> struct auction_data*
	int (*count) (int char_id, bool buy);
	void (*save) (struct auction_data *auction);
	unsigned int (*create) (struct auction_data *auction);
	int (*end_timer) (int tid, int64 tick, int id, intptr_t data);
	void (*delete_) (struct auction_data *auction);
	void (*fromsql) (void);
	int (*parse_frommap) (int fd);
	int (*sql_init) (void);
	void (*sql_final) (void);
};

struct inter_auction_interface *inter_auction;

#endif /* CHAR_INT_AUCTION_H */
