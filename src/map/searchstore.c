// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/malloc.h"  // aMalloc, aRealloc, aFree
#include "../common/showmsg.h"  // ShowError, ShowWarning
#include "../common/strlib.h"  // safestrncpy
#include "battle.h"  // battle_config.*
#include "clif.h"  // clif->open_search_store_info, clif->search_store_info_*
#include "pc.h"  // struct map_session_data
#include "searchstore.h"  // struct s_search_store_info

struct searchstore_interface searchstore_s;

/// retrieves search function by type
static inline searchstore_search_t searchstore_getsearchfunc(unsigned char type) {
	switch( type ) {
		case SEARCHTYPE_VENDING:      return vending->search;
		case SEARCHTYPE_BUYING_STORE: return buyingstore->search;
	}
	return NULL;
}


/// retrieves search-all function by type
static inline searchstore_searchall_t searchstore_getsearchallfunc(unsigned char type) {
	switch( type ) {
		case SEARCHTYPE_VENDING:      return vending->searchall;
		case SEARCHTYPE_BUYING_STORE: return buyingstore->searchall;
	}
	return NULL;
}


/// checks if the player has a store by type
static inline bool searchstore_hasstore(struct map_session_data* sd, unsigned char type) {
	switch( type ) {
		case SEARCHTYPE_VENDING:      return sd->state.vending;
		case SEARCHTYPE_BUYING_STORE: return sd->state.buyingstore;
	}
	return false;
}


/// returns player's store id by type
static inline unsigned int searchstore_getstoreid(struct map_session_data* sd, unsigned char type) {
	switch( type ) {
		case SEARCHTYPE_VENDING:      return sd->vender_id;
		case SEARCHTYPE_BUYING_STORE: return sd->buyer_id;
	}
	return 0;
}


bool searchstore_open(struct map_session_data* sd, unsigned int uses, unsigned short effect) {
	if( !battle_config.feature_search_stores || sd->searchstore.open ) {
		return false;
	}

	if( !uses || effect >= EFFECTTYPE_MAX ) {// invalid input
		return false;
	}

	sd->searchstore.open   = true;
	sd->searchstore.uses   = uses;
	sd->searchstore.effect = effect;

	clif->open_search_store_info(sd);

	return true;
}


void searchstore_query(struct map_session_data* sd, unsigned char type, unsigned int min_price, unsigned int max_price, const unsigned short* itemlist, unsigned int item_count, const unsigned short* cardlist, unsigned int card_count)
{
	unsigned int i;
	struct map_session_data* pl_sd;
	struct DBIterator *iter;
	struct s_search_store_search s;
	searchstore_searchall_t store_searchall;
	time_t querytime;

	if( !battle_config.feature_search_stores ) {
		return;
	}

	if( !sd->searchstore.open ) {
		return;
	}

	if( ( store_searchall = searchstore_getsearchallfunc(type) ) == NULL ) {
		ShowError("searchstore_query: Unknown search type %u (account_id=%d).\n", (unsigned int)type, sd->bl.id);
		return;
	}

	time(&querytime);

	if( sd->searchstore.nextquerytime > querytime ) {
		clif->search_store_info_failed(sd, SSI_FAILED_LIMIT_SEARCH_TIME);
		return;
	}

	if( !sd->searchstore.uses ) {
		clif->search_store_info_failed(sd, SSI_FAILED_SEARCH_CNT);
		return;
	}

	// validate lists
	for( i = 0; i < item_count; i++ ) {
		if( !itemdb->exists(itemlist[i]) ) {
			ShowWarning("searchstore_query: Client resolved item %hu is not known.\n", itemlist[i]);
			clif->search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);
			return;
		}
	}
	for( i = 0; i < card_count; i++ ) {
		if( !itemdb->exists(cardlist[i]) ) {
			ShowWarning("searchstore_query: Client resolved card %hu is not known.\n", cardlist[i]);
			clif->search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);
			return;
		}
	}

	if( max_price < min_price ) {
		swap(min_price, max_price);
	}

	sd->searchstore.uses--;
	sd->searchstore.type = type;
	sd->searchstore.nextquerytime = querytime+battle_config.searchstore_querydelay;

	// drop previous results
	searchstore->clear(sd);

	// allocate max. amount of results
	sd->searchstore.items = (struct s_search_store_info_item*)aMalloc(sizeof(struct s_search_store_info_item)*battle_config.searchstore_maxresults);

	// search
	s.search_sd  = sd;
	s.itemlist   = itemlist;
	s.cardlist   = cardlist;
	s.item_count = item_count;
	s.card_count = card_count;
	s.min_price  = min_price;
	s.max_price  = max_price;
	iter         = db_iterator(vending->db);

	for( pl_sd = dbi_first(iter); dbi_exists(iter);  pl_sd = dbi_next(iter) ) {
		if( sd == pl_sd ) {// skip own shop, if any
			continue;
		}

		if( !store_searchall(pl_sd, &s) ) {// exceeded result size
			clif->search_store_info_failed(sd, SSI_FAILED_OVER_MAXCOUNT);
			break;
		}
	}

	dbi_destroy(iter);

	if( sd->searchstore.count ) {
		// reclaim unused memory
		sd->searchstore.items = (struct s_search_store_info_item*)aRealloc(sd->searchstore.items, sizeof(struct s_search_store_info_item)*sd->searchstore.count);

		// present results
		clif->search_store_info_ack(sd);

		// one page displayed
		sd->searchstore.pages++;
	} else {
		// cleanup
		searchstore->clear(sd);

		// update uses
		clif->search_store_info_ack(sd);

		// notify of failure
		clif->search_store_info_failed(sd, SSI_FAILED_NOTHING_SEARCH_ITEM);
	}
}


/// checks whether or not more results are available for the client
bool searchstore_querynext(struct map_session_data* sd) {
	if( sd->searchstore.count && ( sd->searchstore.count-1 )/SEARCHSTORE_RESULTS_PER_PAGE < sd->searchstore.pages ) {
		return true;
	}

	return false;
}


void searchstore_next(struct map_session_data* sd) {
	if( !battle_config.feature_search_stores || !sd->searchstore.open || sd->searchstore.count <= sd->searchstore.pages*SEARCHSTORE_RESULTS_PER_PAGE )
	{// nothing (more) to display
		return;
	}

	// present results
	clif->search_store_info_ack(sd);

	// one more page displayed
	sd->searchstore.pages++;
}


void searchstore_clear(struct map_session_data* sd) {
	searchstore->clearremote(sd);

	if( sd->searchstore.items ) {// release results
		aFree(sd->searchstore.items);
		sd->searchstore.items = NULL;
	}

	sd->searchstore.count = 0;
	sd->searchstore.pages = 0;
}


void searchstore_close(struct map_session_data* sd) {
	if( sd->searchstore.open ) {
		searchstore->clear(sd);

		sd->searchstore.uses = 0;
		sd->searchstore.open = false;
	}
}


void searchstore_click(struct map_session_data* sd, int account_id, int store_id, unsigned short nameid) {
	unsigned int i;
	struct map_session_data* pl_sd;
	searchstore_search_t store_search;

	if( !battle_config.feature_search_stores || !sd->searchstore.open || !sd->searchstore.count ) {
		return;
	}

	searchstore->clearremote(sd);

	ARR_FIND( 0, sd->searchstore.count, i,  sd->searchstore.items[i].store_id == store_id && sd->searchstore.items[i].account_id == account_id && sd->searchstore.items[i].nameid == nameid );
	if( i == sd->searchstore.count ) {// no such result, crafted
		ShowWarning("searchstore_click: Received request with item %hu of account %d, which is not part of current result set (account_id=%d, char_id=%d).\n", nameid, account_id, sd->bl.id, sd->status.char_id);
		clif->search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	if( ( pl_sd = map->id2sd(account_id) ) == NULL ) {// no longer online
		clif->search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	if( !searchstore_hasstore(pl_sd, sd->searchstore.type) || searchstore_getstoreid(pl_sd, sd->searchstore.type) != store_id ) {
		// no longer vending/buying or not same shop
		clif->search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	store_search = searchstore_getsearchfunc(sd->searchstore.type);

	if( !store_search(pl_sd, nameid) ) {// item no longer being sold/bought
		clif->search_store_info_failed(sd, SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE);
		return;
	}

	switch( sd->searchstore.effect ) {
		case EFFECTTYPE_NORMAL:
			// display coords

			if( sd->bl.m != pl_sd->bl.m ) {// not on same map, wipe previous marker
				clif->search_store_info_click_ack(sd, -1, -1);
			} else {
				clif->search_store_info_click_ack(sd, pl_sd->bl.x, pl_sd->bl.y);
			}

			break;
		case EFFECTTYPE_CASH:
			// open remotely

			// to bypass range checks
			sd->searchstore.remote_id = account_id;

			switch( sd->searchstore.type ) {
				case SEARCHTYPE_VENDING:      vending->list(sd, account_id); break;
				case SEARCHTYPE_BUYING_STORE: buyingstore->open(sd, account_id);       break;
			}

			break;
		default:
			// unknown
			ShowError("searchstore_click: Unknown search store effect %u (account_id=%d).\n", (unsigned int)sd->searchstore.effect, sd->bl.id);
	}
}


/// checks whether or not sd has opened account_id's shop remotely
bool searchstore_queryremote(struct map_session_data* sd, int account_id) {
	return (bool)( sd->searchstore.open && sd->searchstore.count && sd->searchstore.remote_id == account_id );
}


/// removes range-check bypassing for remotely opened stores
void searchstore_clearremote(struct map_session_data* sd) {
	sd->searchstore.remote_id = 0;
}


/// receives results from a store-specific callback
bool searchstore_result(struct map_session_data* sd, unsigned int store_id, int account_id, const char* store_name, unsigned short nameid, unsigned short amount, unsigned int price, const short* card, unsigned char refine)
{
	struct s_search_store_info_item* ssitem;

	if( sd->searchstore.count >= (unsigned int)battle_config.searchstore_maxresults ) {// no more
		return false;
	}

	ssitem = &sd->searchstore.items[sd->searchstore.count++];
	ssitem->store_id = store_id;
	ssitem->account_id = account_id;
	safestrncpy(ssitem->store_name, store_name, sizeof(ssitem->store_name));
	ssitem->nameid = nameid;
	ssitem->amount = amount;
	ssitem->price = price;
	memcpy(ssitem->card, card, sizeof(ssitem->card));
	ssitem->refine = refine;

	return true;
}

void searchstore_defaults (void) {
	searchstore = &searchstore_s;
	
	searchstore->open = searchstore_open;
	searchstore->query = searchstore_query;
	searchstore->querynext = searchstore_querynext;
	searchstore->next = searchstore_next;
	searchstore->clear = searchstore_clear;
	searchstore->close = searchstore_close;
	searchstore->click = searchstore_click;
	searchstore->queryremote = searchstore_queryremote;
	searchstore->clearremote = searchstore_clearremote;
	searchstore->result = searchstore_result;
	
}
