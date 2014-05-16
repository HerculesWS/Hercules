// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _MAP_BUYINGSTORE_H_
#define _MAP_BUYINGSTORE_H_

#include "../common/cbasetypes.h"
#include "../common/mmo.h" // MAX_SLOTS

struct map_session_data;

/**
 * Declarations
 **/
struct s_search_store_search;

/**
 * Defines
 **/
#define MAX_BUYINGSTORE_SLOTS 5

/// constants (client-side restrictions)
#define BUYINGSTORE_MAX_PRICE 99990000
#define BUYINGSTORE_MAX_AMOUNT 9999

/**
 * Enumerations
 **/
/// failure constants for clif functions
enum e_buyingstore_failure {
	BUYINGSTORE_CREATE               = 1,  // "Failed to open buying store."
	BUYINGSTORE_CREATE_OVERWEIGHT    = 2,  // "Total amount of then possessed items exceeds the weight limit by %d. Please re-enter."
	BUYINGSTORE_TRADE_BUYER_ZENY     = 3,  // "All items within the buy limit were purchased."
	BUYINGSTORE_TRADE_BUYER_NO_ITEMS = 4,  // "All items were purchased."
	BUYINGSTORE_TRADE_SELLER_FAILED  = 5,  // "The deal has failed."
	BUYINGSTORE_TRADE_SELLER_COUNT   = 6,  // "The trade failed, because the entered amount of item %s is higher, than the buyer is willing to buy."
	BUYINGSTORE_TRADE_SELLER_ZENY    = 7,  // "The trade failed, because the buyer is lacking required balance."
	BUYINGSTORE_CREATE_NO_INFO       = 8,  // "No sale (purchase) information available."
};

/**
 * Structures
 **/
struct s_buyingstore_item {
	int price;
	unsigned short amount;
	unsigned short nameid;
};

struct s_buyingstore {
	struct s_buyingstore_item items[MAX_BUYINGSTORE_SLOTS];
	int zenylimit;
	unsigned char slots;
};

/**
 * Interface
 **/
struct buyingstore_interface {
	unsigned int nextid;
	short blankslots[MAX_SLOTS];  // used when checking whether or not an item's card slots are blank
	/* */
	bool (*setup) (struct map_session_data* sd, unsigned char slots);
	void (*create) (struct map_session_data* sd, int zenylimit, unsigned char result, const char* storename, const uint8* itemlist, unsigned int count);
	void (*close) (struct map_session_data* sd);
	void (*open) (struct map_session_data* sd, int account_id);
	void (*trade) (struct map_session_data* sd, int account_id, unsigned int buyer_id, const uint8* itemlist, unsigned int count);
	bool (*search) (struct map_session_data* sd, unsigned short nameid);
	bool (*searchall) (struct map_session_data* sd, const struct s_search_store_search* s);
	unsigned int (*getuid) (void);
};

struct buyingstore_interface *buyingstore;

void buyingstore_defaults (void);

#endif  // _MAP_BUYINGSTORE_H_
