// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#ifndef _BUYINGSTORE_H_
#define _BUYINGSTORE_H_

struct s_search_store_search;

#define MAX_BUYINGSTORE_SLOTS 5

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

struct buyingstore_interface {
	bool (*setup) (struct map_session_data* sd, unsigned char slots);
	void (*create) (struct map_session_data* sd, int zenylimit, unsigned char result, const char* storename, const uint8* itemlist, unsigned int count);
	void (*close) (struct map_session_data* sd);
	void (*open) (struct map_session_data* sd, int account_id);
	void (*trade) (struct map_session_data* sd, int account_id, unsigned int buyer_id, const uint8* itemlist, unsigned int count);
	bool (*search) (struct map_session_data* sd, unsigned short nameid);
	bool (*searchall) (struct map_session_data* sd, const struct s_search_store_search* s);
};

struct buyingstore_interface *buyingstore;

void buyingstore_defaults (void);

#endif  // _BUYINGSTORE_H_
