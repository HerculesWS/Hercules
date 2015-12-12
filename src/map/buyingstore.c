/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define HERCULES_CORE

#include "buyingstore.h" // struct s_buyingstore

#include "map/atcommand.h" // msg_txt
#include "map/battle.h" // battle_config.*
#include "map/chrif.h"
#include "map/clif.h" // clif-"buyingstore_*
#include "map/log.h" // log_pick_pc, log_zeny
#include "map/pc.h" // struct map_session_data
#include "common/cbasetypes.h"
#include "common/db.h" // ARR_FIND
#include "common/nullpo.h" // nullpo_*
#include "common/showmsg.h" // ShowWarning
#include "common/socket.h" // RBUF*
#include "common/strlib.h" // safestrncpy

struct buyingstore_interface buyingstore_s;
struct buyingstore_interface *buyingstore;

/// Returns unique buying store id
unsigned int buyingstore_getuid(void) {
	return buyingstore->nextid++;
}

bool buyingstore_setup(struct map_session_data* sd, unsigned char slots)
{
	nullpo_retr(false, sd);
	if( !battle_config.feature_buying_store || sd->state.vending || sd->state.buyingstore || sd->state.trading || slots == 0 )
	{
		return false;
	}

	if(pc_ismuted(&sd->sc, MANNER_NOROOM))
	{// custom: mute limitation
		return false;
	}

	if( map->list[sd->bl.m].flag.novending ) {
		// custom: no vending maps
		clif->message(sd->fd, msg_sd(sd,276)); // "You can't open a shop on this map"
		return false;
	}

	if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNOVENDING)) {
		// custom: no vending cells
		clif->message(sd->fd, msg_sd(sd,204)); // "You can't open a shop on this cell."
		return false;
	}

	if( slots > MAX_BUYINGSTORE_SLOTS )
	{
		ShowWarning("buyingstore_setup: Requested %d slots, but server supports only %d slots.\n", (int)slots, MAX_BUYINGSTORE_SLOTS);
		slots = MAX_BUYINGSTORE_SLOTS;
	}

	sd->buyingstore.slots = slots;
	clif->buyingstore_open(sd);

	return true;
}

void buyingstore_create(struct map_session_data* sd, int zenylimit, unsigned char result, const char* storename, const uint8* itemlist, unsigned int count)
{
	unsigned int i, weight, listidx;

	nullpo_retv(sd);
	if (!result || count == 0) {
		// canceled, or no items
		return;
	}

	if( !battle_config.feature_buying_store || pc_istrading(sd) || sd->buyingstore.slots == 0 || count > sd->buyingstore.slots || zenylimit <= 0 || zenylimit > sd->status.zeny || !storename[0] )
	{// disabled or invalid input
		sd->buyingstore.slots = 0;
		clif->buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to buy (give zeny)
		sd->buyingstore.slots = 0;
		clif->message(sd->fd, msg_sd(sd,246));
		clif->buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return;
	}

	if(pc_ismuted(&sd->sc, MANNER_NOROOM))
	{// custom: mute limitation
		return;
	}

	if( map->list[sd->bl.m].flag.novending ) {
		// custom: no vending maps
		clif->message(sd->fd, msg_sd(sd,276)); // "You can't open a shop on this map"
		return;
	}

	if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNOVENDING)) {
		// custom: no vending cells
		clif->message(sd->fd, msg_sd(sd,204)); // "You can't open a shop on this cell."
		return;
	}

	weight = sd->weight;

	// check item list
	for (i = 0; i < count; i++) {
		// itemlist: <name id>.W <amount>.W <price>.L
		unsigned short nameid, amount;
		int price, idx;
		struct item_data* id;

		nameid = RBUFW(itemlist,i*8+0);
		amount = RBUFW(itemlist,i*8+2);
		price  = RBUFL(itemlist,i*8+4);

		if( ( id = itemdb->exists(nameid) ) == NULL || amount == 0 )
		{// invalid input
			break;
		}

		if( price <= 0 || price > BUYINGSTORE_MAX_PRICE )
		{// invalid price: unlike vending, items cannot be bought at 0 Zeny
			break;
		}

		if (!id->flag.buyingstore || !itemdb->cantrade_sub(id, pc_get_group_level(sd), pc_get_group_level(sd))
		 || (idx = pc->search_inventory(sd, nameid)) == INDEX_NOT_FOUND
		 ) { // restrictions: allowed, no character-bound items and at least one must be owned
			break;
		}

		if( sd->status.inventory[idx].amount+amount > BUYINGSTORE_MAX_AMOUNT )
		{// too many items of same kind
			break;
		}

		if( i )
		{// duplicate check. as the client does this too, only malicious intent should be caught here
			ARR_FIND( 0, i, listidx, sd->buyingstore.items[listidx].nameid == nameid );
			if( listidx != i )
			{// duplicate
				ShowWarning("buyingstore_create: Found duplicate item on buying list (nameid=%hu, amount=%hu, account_id=%d, char_id=%d).\n", nameid, amount, sd->status.account_id, sd->status.char_id);
				break;
			}
		}

		weight+= id->weight*amount;
		sd->buyingstore.items[i].nameid = nameid;
		sd->buyingstore.items[i].amount = amount;
		sd->buyingstore.items[i].price  = price;
	}

	if( i != count )
	{// invalid item/amount/price
		sd->buyingstore.slots = 0;
		clif->buyingstore_open_failed(sd, BUYINGSTORE_CREATE, 0);
		return;
	}

	if( (sd->max_weight*90)/100 < weight )
	{// not able to carry all wanted items without getting overweight (90%)
		sd->buyingstore.slots = 0;
		clif->buyingstore_open_failed(sd, BUYINGSTORE_CREATE_OVERWEIGHT, weight);
		return;
	}

	// success
	sd->state.buyingstore = true;
	sd->buyer_id = buyingstore->getuid();
	sd->buyingstore.zenylimit = zenylimit;
	sd->buyingstore.slots = i;  // store actual amount of items
	safestrncpy(sd->message, storename, sizeof(sd->message));
	clif->buyingstore_myitemlist(sd);
	clif->buyingstore_entry(sd);
}

void buyingstore_close(struct map_session_data* sd)
{
	nullpo_retv(sd);
	if (sd->state.buyingstore)
	{
		// invalidate data
		sd->state.buyingstore = false;
		memset(&sd->buyingstore, 0, sizeof(sd->buyingstore));

		// notify other players
		clif->buyingstore_disappear_entry(sd);
	}
}

void buyingstore_open(struct map_session_data* sd, int account_id)
{
	struct map_session_data* pl_sd;

	nullpo_retv(sd);
	if( !battle_config.feature_buying_store || pc_istrading(sd) )
	{// not allowed to sell
		return;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to sell
		clif->message(sd->fd, msg_sd(sd,246));
		return;
	}

	if( ( pl_sd = map->id2sd(account_id) ) == NULL || !pl_sd->state.buyingstore ) {
		// not online or not buying
		return;
	}

	if( !searchstore->queryremote(sd, account_id) && ( sd->bl.m != pl_sd->bl.m || !check_distance_bl(&sd->bl, &pl_sd->bl, AREA_SIZE) ) )
	{// out of view range
		return;
	}

	// success
	clif->buyingstore_itemlist(sd, pl_sd);
}


void buyingstore_trade(struct map_session_data* sd, int account_id, unsigned int buyer_id, const uint8* itemlist, unsigned int count)
{
	int zeny = 0;
	unsigned int i, weight, listidx, k;
	struct map_session_data* pl_sd;

	nullpo_retv(sd);
	if( count == 0 )
	{// nothing to do
		return;
	}

	if( !battle_config.feature_buying_store || pc_istrading(sd) )
	{// not allowed to sell
		clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( !pc_can_give_items(sd) )
	{// custom: GM is not allowed to sell
		clif->message(sd->fd, msg_sd(sd,246));
		clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( ( pl_sd = map->id2sd(account_id) ) == NULL || !pl_sd->state.buyingstore || pl_sd->buyer_id != buyer_id ) {
		// not online, not buying or not same store
		clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	if( !searchstore->queryremote(sd, account_id) && ( sd->bl.m != pl_sd->bl.m || !check_distance_bl(&sd->bl, &pl_sd->bl, AREA_SIZE) ) )
	{// out of view range
		clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, 0);
		return;
	}

	searchstore->clearremote(sd);

	if( pl_sd->status.zeny < pl_sd->buyingstore.zenylimit )
	{// buyer lost zeny in the mean time? fix the limit
		pl_sd->buyingstore.zenylimit = pl_sd->status.zeny;
	}
	weight = pl_sd->weight;

	// check item list
	for( i = 0; i < count; i++ )
	{// itemlist: <index>.W <name id>.W <amount>.W
		unsigned short nameid, amount;
		int index;

		index  = RBUFW(itemlist,i*6+0)-2;
		nameid = RBUFW(itemlist,i*6+2);
		amount = RBUFW(itemlist,i*6+4);

		if( i )
		{// duplicate check. as the client does this too, only malicious intent should be caught here
			ARR_FIND( 0, i, k, RBUFW(itemlist,k*6+0)-2 == index );
			if( k != i )
			{// duplicate
				ShowWarning("buyingstore_trade: Found duplicate item on selling list (prevnameid=%hu, prevamount=%hu, nameid=%hu, amount=%hu, account_id=%d, char_id=%d).\n",
					RBUFW(itemlist,k*6+2), RBUFW(itemlist,k*6+4), nameid, amount, sd->status.account_id, sd->status.char_id);
				clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
				return;
			}
		}

		if( index < 0 || index >= ARRAYLENGTH(sd->status.inventory) || sd->inventory_data[index] == NULL || sd->status.inventory[index].nameid != nameid || sd->status.inventory[index].amount < amount )
		{// invalid input
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		if (sd->status.inventory[index].expire_time || (sd->status.inventory[index].bound && !pc_can_give_bound_items(sd))
		 || !itemdb_cantrade(&sd->status.inventory[index], pc_get_group_level(sd), pc_get_group_level(pl_sd))
		 || memcmp(sd->status.inventory[index].card, buyingstore->blankslots, sizeof(buyingstore->blankslots))
		) {
			// non-tradable item
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		ARR_FIND( 0, pl_sd->buyingstore.slots, listidx, pl_sd->buyingstore.items[listidx].nameid == nameid );
		if( listidx == pl_sd->buyingstore.slots || pl_sd->buyingstore.items[listidx].amount == 0 )
		{// there is no such item or the buyer has already bought all of them
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		if( pl_sd->buyingstore.items[listidx].amount < amount )
		{// buyer does not need that much of the item
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_COUNT, nameid);
			return;
		}

		if( pc->checkadditem(pl_sd, nameid, amount) == ADDITEM_OVERAMOUNT )
		{// buyer does not have enough space for this item
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}

		if( amount*(unsigned int)sd->inventory_data[index]->weight > pl_sd->max_weight-weight )
		{// normally this is not supposed to happen, as the total weight is
		 // checked upon creation, but the buyer could have gained items
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_FAILED, nameid);
			return;
		}
		weight+= amount*sd->inventory_data[index]->weight;

		if( amount*pl_sd->buyingstore.items[listidx].price > pl_sd->buyingstore.zenylimit-zeny )
		{// buyer does not have enough zeny
			clif->buyingstore_trade_failed_seller(sd, BUYINGSTORE_TRADE_SELLER_ZENY, nameid);
			return;
		}
		zeny+= amount*pl_sd->buyingstore.items[listidx].price;
	}

	// process item list
	for( i = 0; i < count; i++ )
	{// itemlist: <index>.W <name id>.W <amount>.W
		unsigned short nameid, amount;
		int index;

		index  = RBUFW(itemlist,i*6+0)-2;
		nameid = RBUFW(itemlist,i*6+2);
		amount = RBUFW(itemlist,i*6+4);

		ARR_FIND( 0, pl_sd->buyingstore.slots, listidx, pl_sd->buyingstore.items[listidx].nameid == nameid );
		zeny = amount*pl_sd->buyingstore.items[listidx].price;

		// move item
		pc->additem(pl_sd, &sd->status.inventory[index], amount, LOG_TYPE_BUYING_STORE);
		pc->delitem(sd, index, amount, 1, DELITEM_NORMAL, LOG_TYPE_BUYING_STORE);
		pl_sd->buyingstore.items[listidx].amount-= amount;

		// pay up
		pc->payzeny(pl_sd, zeny, LOG_TYPE_BUYING_STORE, sd);
		pc->getzeny(sd, zeny, LOG_TYPE_BUYING_STORE, pl_sd);
		pl_sd->buyingstore.zenylimit-= zeny;

		// notify clients
		clif->buyingstore_delete_item(sd, index, amount, pl_sd->buyingstore.items[listidx].price);
		clif->buyingstore_update_item(pl_sd, nameid, amount);
	}

	if( map->save_settings&128 ) {
		chrif->save(sd, 0);
		chrif->save(pl_sd, 0);
	}

	// check whether or not there is still something to buy
	ARR_FIND( 0, pl_sd->buyingstore.slots, i, pl_sd->buyingstore.items[i].amount != 0 );
	if( i == pl_sd->buyingstore.slots )
	{// everything was bought
		clif->buyingstore_trade_failed_buyer(pl_sd, BUYINGSTORE_TRADE_BUYER_NO_ITEMS);
	}
	else if( pl_sd->buyingstore.zenylimit == 0 )
	{// zeny limit reached
		clif->buyingstore_trade_failed_buyer(pl_sd, BUYINGSTORE_TRADE_BUYER_ZENY);
	}
	else
	{// continue buying
		return;
	}

	// cannot continue buying
	buyingstore->close(pl_sd);

	// remove auto-trader
	if( pl_sd->state.autotrade ) {
		map->quit(pl_sd);
	}
}


/// Checks if an item is being bought in given player's buying store.
bool buyingstore_search(struct map_session_data* sd, unsigned short nameid)
{
	unsigned int i;

	nullpo_retr(false, sd);
	if (!sd->state.buyingstore)
	{// not buying
		return false;
	}

	ARR_FIND( 0, sd->buyingstore.slots, i, sd->buyingstore.items[i].nameid == nameid && sd->buyingstore.items[i].amount );
	if( i == sd->buyingstore.slots )
	{// not found
		return false;
	}

	return true;
}


/// Searches for all items in a buyingstore, that match given ids, price and possible cards.
/// @return Whether or not the search should be continued.
bool buyingstore_searchall(struct map_session_data* sd, const struct s_search_store_search* s)
{
	unsigned int i, idx;
	struct s_buyingstore_item* it;

	nullpo_retr(true, sd);

	if( !sd->state.buyingstore )
	{// not buying
		return true;
	}

	for( idx = 0; idx < s->item_count; idx++ )
	{
		ARR_FIND( 0, sd->buyingstore.slots, i, sd->buyingstore.items[i].nameid == s->itemlist[idx] && sd->buyingstore.items[i].amount );
		if( i == sd->buyingstore.slots )
		{// not found
			continue;
		}
		it = &sd->buyingstore.items[i];

		if( s->min_price && s->min_price > (unsigned int)it->price )
		{// too low price
			continue;
		}

		if( s->max_price && s->max_price < (unsigned int)it->price )
		{// too high price
			continue;
		}

		if( s->card_count )
		{// ignore cards, as there cannot be any
			;
		}

		if( !searchstore->result(s->search_sd, sd->buyer_id, sd->status.account_id, sd->message, it->nameid, it->amount, it->price, buyingstore->blankslots, 0) )
		{// result set full
			return false;
		}
	}

	return true;
}
void buyingstore_defaults(void) {
	buyingstore = &buyingstore_s;

	buyingstore->nextid = 0;
	memset(buyingstore->blankslots,0,sizeof(buyingstore->blankslots));
	/* */
	buyingstore->setup = buyingstore_setup;
	buyingstore->create = buyingstore_create;
	buyingstore->close = buyingstore_close;
	buyingstore->open = buyingstore_open;
	buyingstore->trade = buyingstore_trade;
	buyingstore->search = buyingstore_search;
	buyingstore->searchall = buyingstore_searchall;
	buyingstore->getuid = buyingstore_getuid;

}
