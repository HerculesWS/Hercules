// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/nullpo.h"
#include "../common/socket.h"

#include "trade.h"
#include "clif.h"
#include "itemdb.h"
#include "map.h"
#include "path.h"
#include "pc.h"
#include "npc.h"
#include "battle.h"
#include "chrif.h"
#include "storage.h"
#include "intif.h"
#include "atcommand.h"
#include "log.h"

#include <stdio.h>
#include <string.h>

struct trade_interface trade_s;

/*==========================================
 * Initiates a trade request.
 *------------------------------------------*/
void trade_traderequest(struct map_session_data *sd, struct map_session_data *target_sd)
{
	nullpo_retv(sd);

	if (map->list[sd->bl.m].flag.notrade) {
		clif->message (sd->fd, msg_txt(272));
		return; //Can't trade in notrade mapflag maps.
	}

	if (target_sd == NULL || sd == target_sd) {
		clif->tradestart(sd, 1); // character does not exist
		return;
	}

	if (target_sd->npc_id)
	{	//Trade fails if you are using an NPC.
		clif->tradestart(sd, 2);
		return;
	}

	if (!battle_config.invite_request_check) {
		if (target_sd->guild_invite > 0 || target_sd->party_invite > 0 || target_sd->adopt_invite) {
			clif->tradestart(sd, 2);
			return;
		}
	}

	if ( sd->trade_partner != 0 ) { // If a character tries to trade to another one then cancel the previous one
		struct map_session_data *previous_sd = map->id2sd(sd->trade_partner);
		if( previous_sd ){
			previous_sd->trade_partner = 0;
			clif->tradecancelled(previous_sd);
		} // Once cancelled then continue to the new one.
		sd->trade_partner = 0;
		clif->tradecancelled(sd);
	}

	if (target_sd->trade_partner != 0) {
		clif->tradestart(sd, 2); // person is in another trade
		return;
	}

	if (!pc_can_give_items(sd) || !pc_can_give_items(target_sd)) //check if both GMs are allowed to trade
	{
		clif->message(sd->fd, msg_txt(246));
		clif->tradestart(sd, 2); // GM is not allowed to trade
		return;
	}

	// Players can not request trade from far away, unless they are allowed to use @trade.
	if (!pc->can_use_command(sd, "@trade") &&
	    (sd->bl.m != target_sd->bl.m || !check_distance_bl(&sd->bl, &target_sd->bl, TRADE_DISTANCE))) {
		clif->tradestart(sd, 0); // too far
		return ;
	}

	target_sd->trade_partner = sd->status.account_id;
	sd->trade_partner = target_sd->status.account_id;
	clif->traderequest(target_sd, sd->status.name);
}

/*==========================================
 * Reply to a trade-request.
 * Type values:
 * 0: Char is too far
 * 1: Character does not exist
 * 2: Trade failed
 * 3: Accept
 * 4: Cancel
 * Weird enough, the client should only send 3/4
 * and the server is the one that can reply 0~2
 *------------------------------------------*/
void trade_tradeack(struct map_session_data *sd, int type) {
	struct map_session_data *tsd;
	nullpo_retv(sd);

	if (sd->state.trading || !sd->trade_partner)
		return; //Already trading or no partner set.

	if ((tsd = map->id2sd(sd->trade_partner)) == NULL) {
		clif->tradestart(sd, 1); // character does not exist
		sd->trade_partner=0;
		return;
	}

	if (tsd->state.trading || tsd->trade_partner != sd->bl.id)
	{
		clif->tradestart(sd, 2);
		sd->trade_partner=0;
		return; //Already trading or wrong partner.
	}

	if (type == 4) { // Cancel
		clif->tradestart(tsd, type);
		clif->tradestart(sd, type);
		sd->state.deal_locked = 0;
		sd->trade_partner = 0;
		tsd->state.deal_locked = 0;
		tsd->trade_partner = 0;
		return;
	}

	if (type != 3)
		return; //If client didn't send accept, it's a broken packet?

	// Players can not request trade from far away, unless they are allowed to use @trade.
	// Check here as well since the original character could had warped.
	if (!pc->can_use_command(sd, "@trade") &&
	    (sd->bl.m != tsd->bl.m || !check_distance_bl(&sd->bl, &tsd->bl, TRADE_DISTANCE))) {
		clif->tradestart(sd, 0); // too far
		sd->trade_partner=0;
		tsd->trade_partner = 0;
		return;
	}

	//Check if you can start trade.
	if (sd->npc_id || sd->state.vending || sd->state.buyingstore || sd->state.storage_flag ||
		tsd->npc_id || tsd->state.vending || tsd->state.buyingstore || tsd->state.storage_flag)
	{	//Fail
		clif->tradestart(sd, 2);
		clif->tradestart(tsd, 2);
		sd->state.deal_locked = 0;
		sd->trade_partner = 0;
		tsd->state.deal_locked = 0;
		tsd->trade_partner = 0;
		return;
	}

	//Initiate trade
	sd->state.trading = 1;
	tsd->state.trading = 1;
	memset(&sd->deal, 0, sizeof(sd->deal));
	memset(&tsd->deal, 0, sizeof(tsd->deal));
	clif->tradestart(tsd, type);
	clif->tradestart(sd, type);
}

/*==========================================
 * Check here hacker for duplicate item in trade
 * normal client refuse to have 2 same types of item (except equipment) in same trade window
 * normal client authorise only no equiped item and only from inventory
 *------------------------------------------*/
int impossible_trade_check(struct map_session_data *sd)
{
	struct item inventory[MAX_INVENTORY];
	char message_to_gm[200];
	int i, index;

	nullpo_retr(1, sd);

	if(sd->deal.zeny > sd->status.zeny) {
		pc_setglobalreg(sd,script->add_str("ZENY_HACKER"),1);
		return -1;
	}

	// get inventory of player
	memcpy(&inventory, &sd->status.inventory, sizeof(struct item) * MAX_INVENTORY);

	// remove this part: arrows can be trade and equiped
	// re-added! [celest]
	// remove equiped items (they can not be trade)
	for (i = 0; i < MAX_INVENTORY; i++)
		if (inventory[i].nameid > 0 && inventory[i].equip && !(inventory[i].equip & EQP_AMMO))
			memset(&inventory[i], 0, sizeof(struct item));

	// check items in player inventory
	for(i = 0; i < 10; i++) {
		if (!sd->deal.item[i].amount)
			continue;
		index = sd->deal.item[i].index;
		if (inventory[index].amount < sd->deal.item[i].amount) {
			// if more than the player have -> hack
			sprintf(message_to_gm, msg_txt(538), sd->status.name, sd->status.account_id); // Hack on trade: character '%s' (account: %d) try to trade more items that he has.
			intif->wis_message_to_gm(map->wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, message_to_gm);
			sprintf(message_to_gm, msg_txt(539), inventory[index].amount, inventory[index].nameid, sd->deal.item[i].amount); // This player has %d of a kind of item (id: %d), and try to trade %d of them.
			intif->wis_message_to_gm(map->wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, message_to_gm);
			// if we block people
			if (battle_config.ban_hack_trade < 0) {
				chrif->char_ask_name(-1, sd->status.name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
				set_eof(sd->fd); // forced to disconnect because of the hack
				// message about the ban
				strcpy(message_to_gm, msg_txt(540)); //  This player has been definitively blocked.
			// if we ban people
			} else if (battle_config.ban_hack_trade > 0) {
				chrif->char_ask_name(-1, sd->status.name, 2, 0, 0, 0, 0, battle_config.ban_hack_trade, 0); // type: 2 - ban (year, month, day, hour, minute, second)
				set_eof(sd->fd); // forced to disconnect because of the hack
				// message about the ban
				sprintf(message_to_gm, msg_txt(507), battle_config.ban_hack_trade); //  This player has been banned for %d minute(s).
			} else
				// message about the ban
				strcpy(message_to_gm, msg_txt(508)); //  This player hasn't been banned (Ban option is disabled).

			intif->wis_message_to_gm(map->wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, message_to_gm);
			return 1;
		}
		inventory[index].amount -= sd->deal.item[i].amount; // remove item from inventory
	}
	return 0;
}

/*==========================================
 * Checks if trade is possible (against zeny limits, inventory limits, etc)
 *------------------------------------------*/
int trade_check(struct map_session_data *sd, struct map_session_data *tsd)
{
	struct item inventory[MAX_INVENTORY];
	struct item inventory2[MAX_INVENTORY];
	struct item_data *data;
	int trade_i, i, n;
	short amount;

	// check zenys value against hackers (Zeny was already checked on time of adding, but you never know when you lost some zeny since then.
	if(sd->deal.zeny > sd->status.zeny || (tsd->status.zeny > MAX_ZENY - sd->deal.zeny))
		return 0;
	if(tsd->deal.zeny > tsd->status.zeny || (sd->status.zeny > MAX_ZENY - tsd->deal.zeny))
		return 0;

	// get inventory of player
	memcpy(&inventory, &sd->status.inventory, sizeof(struct item) * MAX_INVENTORY);
	memcpy(&inventory2, &tsd->status.inventory, sizeof(struct item) * MAX_INVENTORY);

	// check free slot in both inventory
	for(trade_i = 0; trade_i < 10; trade_i++) {
		amount = sd->deal.item[trade_i].amount;
		if (amount) {
			n = sd->deal.item[trade_i].index;
			if (amount > inventory[n].amount)
				return 0; //qty Exploit?

			data = itemdb->search(inventory[n].nameid);
			i = MAX_INVENTORY;
			if (itemdb->isstackable2(data)) { //Stackable item.
				for(i = 0; i < MAX_INVENTORY; i++)
					if (inventory2[i].nameid == inventory[n].nameid &&
						inventory2[i].card[0] == inventory[n].card[0] && inventory2[i].card[1] == inventory[n].card[1] &&
						inventory2[i].card[2] == inventory[n].card[2] && inventory2[i].card[3] == inventory[n].card[3]) {
						if (inventory2[i].amount + amount > MAX_AMOUNT)
							return 0;
						inventory2[i].amount += amount;
						inventory[n].amount -= amount;
						break;
					}
			}

			if (i == MAX_INVENTORY) {// look for an empty slot.
				for(i = 0; i < MAX_INVENTORY && inventory2[i].nameid; i++);
				if (i == MAX_INVENTORY)
					return 0;
				memcpy(&inventory2[i], &inventory[n], sizeof(struct item));
				inventory2[i].amount = amount;
				inventory[n].amount -= amount;
			}
		}
		amount = tsd->deal.item[trade_i].amount;
		if (!amount)
			continue;
		n = tsd->deal.item[trade_i].index;
		if (amount > inventory2[n].amount)
			return 0;
		// search if it's possible to add item (for full inventory)
		data = itemdb->search(inventory2[n].nameid);
		i = MAX_INVENTORY;
		if (itemdb->isstackable2(data)) {
			for(i = 0; i < MAX_INVENTORY; i++)
				if (inventory[i].nameid == inventory2[n].nameid &&
					inventory[i].card[0] == inventory2[n].card[0] && inventory[i].card[1] == inventory2[n].card[1] &&
					inventory[i].card[2] == inventory2[n].card[2] && inventory[i].card[3] == inventory2[n].card[3]) {
					if (inventory[i].amount + amount > MAX_AMOUNT)
						return 0;
					inventory[i].amount += amount;
					inventory2[n].amount -= amount;
					break;
				}
		}
		if (i == MAX_INVENTORY) {
			for(i = 0; i < MAX_INVENTORY && inventory[i].nameid; i++);
			if (i == MAX_INVENTORY)
				return 0;
			memcpy(&inventory[i], &inventory2[n], sizeof(struct item));
			inventory[i].amount = amount;
			inventory2[n].amount -= amount;
		}
	}

	return 1;
}

/*==========================================
 * Adds an item/qty to the trade window
 *------------------------------------------*/
void trade_tradeadditem(struct map_session_data *sd, short index, short amount) {
	struct map_session_data *target_sd;
	struct item *item;
	int trade_i, trade_weight;
	int src_lv, dst_lv;

	nullpo_retv(sd);
	if( !sd->state.trading || sd->state.deal_locked > 0 )
		return; //Can't add stuff.

	if( (target_sd = map->id2sd(sd->trade_partner)) == NULL )
	{
		trade->cancel(sd);
		return;
	}

	if( amount == 0 )
	{	//Why do this.. ~.~ just send an ack, the item won't display on the trade window.
		clif->tradeitemok(sd, index, TIO_SUCCESS);
		return;
	}

	index -= 2; // 0 is for zeny, 1 is unknown. Gravity, go figure...

	//Item checks...
	if( index < 0 || index >= MAX_INVENTORY )
		return;
	if( amount < 0 || amount > sd->status.inventory[index].amount )
		return;

	item = &sd->status.inventory[index];
	src_lv = pc_get_group_level(sd);
	dst_lv = pc_get_group_level(target_sd);
	if( !itemdb_cantrade(item, src_lv, dst_lv) && //Can't trade
		(pc->get_partner(sd) != target_sd || !itemdb_canpartnertrade(item, src_lv, dst_lv)) ) //Can't partner-trade
	{
		clif->message (sd->fd, msg_txt(260));
		clif->tradeitemok(sd, index+2, TIO_INDROCKS);
		return;
	}

	if( item->expire_time )
	{ // Rental System
		clif->message (sd->fd, msg_txt(260));
		clif->tradeitemok(sd, index+2, TIO_INDROCKS);
		return;
	}

	if( item->bound &&
			!( item->bound == IBT_GUILD && sd->status.guild_id == target_sd->status.guild_id ) &&
			!( item->bound == IBT_PARTY && sd->status.party_id == target_sd->status.party_id )
					&& !pc_can_give_bound_items(sd) ) {
		clif->message(sd->fd, msg_txt(293));
		clif->tradeitemok(sd, index+2, TIO_INDROCKS);
		return;
	}
	
	//Locate a trade position
	ARR_FIND( 0, 10, trade_i, sd->deal.item[trade_i].index == index || sd->deal.item[trade_i].amount == 0 );
	if( trade_i == 10 ) //No space left
	{
		clif->tradeitemok(sd, index+2, TIO_OVERWEIGHT);
		return;
	}

	trade_weight = sd->inventory_data[index]->weight * amount;
	if( target_sd->weight + sd->deal.weight + trade_weight > target_sd->max_weight )
	{	//fail to add item -- the player was over weighted.
		clif->tradeitemok(sd, index+2, TIO_OVERWEIGHT);
		return;
	}

	if( sd->deal.item[trade_i].index == index )
	{	//The same item as before is being readjusted.
		if( sd->deal.item[trade_i].amount + amount > sd->status.inventory[index].amount )
		{	//packet deal exploit check
			amount = sd->status.inventory[index].amount - sd->deal.item[trade_i].amount;
			trade_weight = sd->inventory_data[index]->weight * amount;
		}
		sd->deal.item[trade_i].amount += amount;
	}
	else
	{	//New deal item
		sd->deal.item[trade_i].index = index;
		sd->deal.item[trade_i].amount = amount;
	}
	sd->deal.weight += trade_weight;

	clif->tradeitemok(sd, index+2, TIO_SUCCESS); // Return the index as it was received
	clif->tradeadditem(sd, target_sd, index+2, amount);
}

/*==========================================
 * Adds the specified amount of zeny to the trade window
 *------------------------------------------*/
void trade_tradeaddzeny(struct map_session_data* sd, int amount)
{
	struct map_session_data* target_sd;
	nullpo_retv(sd);

	if( !sd->state.trading || sd->state.deal_locked > 0 )
		return; //Can't add stuff.

	if( (target_sd = map->id2sd(sd->trade_partner)) == NULL ) {
		trade->cancel(sd);
		return;
	}

	if( amount < 0 || amount > sd->status.zeny || amount > MAX_ZENY - target_sd->status.zeny )
	{	// invalid values, no appropriate packet for it => abort
		trade->cancel(sd);
		return;
	}

	sd->deal.zeny = amount;
	clif->tradeadditem(sd, target_sd, 0, amount);
}

/*==========================================
 * 'Ok' button on the trade window is pressed.
 *------------------------------------------*/
void trade_tradeok(struct map_session_data *sd) {
	struct map_session_data *target_sd;

	if(sd->state.deal_locked || !sd->state.trading)
		return;

	if ((target_sd = map->id2sd(sd->trade_partner)) == NULL) {
		trade->cancel(sd);
		return;
	}
	sd->state.deal_locked = 1;
	clif->tradeitemok(sd, 0, TIO_SUCCESS);
	clif->tradedeal_lock(sd, 0);
	clif->tradedeal_lock(target_sd, 1);
}

/*==========================================
 * 'Cancel' is pressed. (or trade was force-cancelled by the code)
 *------------------------------------------*/
void trade_tradecancel(struct map_session_data *sd) {
	struct map_session_data *target_sd;
	int trade_i;

	target_sd = map->id2sd(sd->trade_partner);

	if(!sd->state.trading)
	{ // Not trade acepted
		if( target_sd ) {
			target_sd->trade_partner = 0;
			clif->tradecancelled(target_sd);
		}
		sd->trade_partner = 0;
		clif->tradecancelled(sd);
		return;
	}

	for(trade_i = 0; trade_i < 10; trade_i++) { // give items back (only virtual)
		if (!sd->deal.item[trade_i].amount)
			continue;
		clif->additem(sd, sd->deal.item[trade_i].index, sd->deal.item[trade_i].amount, 0);
		sd->deal.item[trade_i].index = 0;
		sd->deal.item[trade_i].amount = 0;
	}
	if (sd->deal.zeny) {
		clif->updatestatus(sd, SP_ZENY);
		sd->deal.zeny = 0;
	}

	sd->state.deal_locked = 0;
	sd->state.trading = 0;
	sd->trade_partner = 0;
	clif->tradecancelled(sd);

	if (!target_sd)
		return;

	for(trade_i = 0; trade_i < 10; trade_i++) { // give items back (only virtual)
		if (!target_sd->deal.item[trade_i].amount)
			continue;
		clif->additem(target_sd, target_sd->deal.item[trade_i].index, target_sd->deal.item[trade_i].amount, 0);
		target_sd->deal.item[trade_i].index = 0;
		target_sd->deal.item[trade_i].amount = 0;
	}

	if (target_sd->deal.zeny) {
		clif->updatestatus(target_sd, SP_ZENY);
		target_sd->deal.zeny = 0;
	}
	target_sd->state.deal_locked = 0;
	target_sd->trade_partner = 0;
	target_sd->state.trading = 0;
	clif->tradecancelled(target_sd);
}

/*==========================================
 * lock sd and tsd trade data, execute the trade, clear, then save players
 *------------------------------------------*/
void trade_tradecommit(struct map_session_data *sd) {
	struct map_session_data *tsd;
	int trade_i;
	int flag;

	if (!sd->state.trading || !sd->state.deal_locked) //Locked should be 1 (pressed ok) before you can press trade.
		return;

	if ((tsd = map->id2sd(sd->trade_partner)) == NULL) {
		trade->cancel(sd);
		return;
	}

	sd->state.deal_locked = 2;

	if (tsd->state.deal_locked < 2)
		return; //Not yet time for trading.

	//Now is a good time (to save on resources) to check that the trade can indeed be made and it's not exploitable.
	// check exploit (trade more items that you have)
	if (trade->check_impossible(sd)) {
		trade->cancel(sd);
		return;
	}
	// check exploit (trade more items that you have)
	if (trade->check_impossible(tsd)) {
		trade->cancel(tsd);
		return;
	}
	// check for full inventory (can not add traded items)
	if (!trade->check(sd,tsd)) { // check the both players
		trade->cancel(sd);
		return;
	}

	// trade is accepted and correct.
	for( trade_i = 0; trade_i < 10; trade_i++ )
	{
		int n;
		if (sd->deal.item[trade_i].amount)
		{
			n = sd->deal.item[trade_i].index;

			flag = pc->additem(tsd, &sd->status.inventory[n], sd->deal.item[trade_i].amount,LOG_TYPE_TRADE);
			if (flag == 0)
				pc->delitem(sd, n, sd->deal.item[trade_i].amount, 1, 6, LOG_TYPE_TRADE);
			else
				clif->additem(sd, n, sd->deal.item[trade_i].amount, 0);
			sd->deal.item[trade_i].index = 0;
			sd->deal.item[trade_i].amount = 0;
		}
		if (tsd->deal.item[trade_i].amount)
		{
			n = tsd->deal.item[trade_i].index;

			flag = pc->additem(sd, &tsd->status.inventory[n], tsd->deal.item[trade_i].amount,LOG_TYPE_TRADE);
			if (flag == 0)
				pc->delitem(tsd, n, tsd->deal.item[trade_i].amount, 1, 6, LOG_TYPE_TRADE);
			else
				clif->additem(tsd, n, tsd->deal.item[trade_i].amount, 0);
			tsd->deal.item[trade_i].index = 0;
			tsd->deal.item[trade_i].amount = 0;
		}
	}

	if( sd->deal.zeny ) {
		pc->payzeny(sd ,sd->deal.zeny, LOG_TYPE_TRADE, tsd);
		pc->getzeny(tsd,sd->deal.zeny,LOG_TYPE_TRADE, sd);
		sd->deal.zeny = 0;

	}
	if ( tsd->deal.zeny) {
		pc->payzeny(tsd,tsd->deal.zeny,LOG_TYPE_TRADE, sd);
		pc->getzeny(sd ,tsd->deal.zeny,LOG_TYPE_TRADE, tsd);
		tsd->deal.zeny = 0;
	}

	sd->state.deal_locked = 0;
	sd->trade_partner = 0;
	sd->state.trading = 0;

	tsd->state.deal_locked = 0;
	tsd->trade_partner = 0;
	tsd->state.trading = 0;

	clif->tradecompleted(sd, 0);
	clif->tradecompleted(tsd, 0);

	// save both player to avoid crash: they always have no advantage/disadvantage between the 2 players
	if (map->save_settings&1)
  	{
		chrif->save(sd,0);
		chrif->save(tsd,0);
	}
}

void trade_defaults(void)
{
	trade = &trade_s;
	
	trade->request = trade_traderequest;
	trade->ack = trade_tradeack;
	trade->check_impossible = impossible_trade_check;
	trade->check = trade_check;
	trade->additem = trade_tradeadditem;
	trade->addzeny = trade_tradeaddzeny;
	trade->ok = trade_tradeok;
	trade->cancel = trade_tradecancel;
	trade->commit = trade_tradecommit;
}
