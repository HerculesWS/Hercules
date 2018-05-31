/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
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

#include "storage.h"

#include "map/atcommand.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clif.h"
#include "map/guild.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/map.h" // struct map_session_data
#include "map/pc.h"
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/nullpo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct storage_interface storage_s;
struct guild_storage_interface gstorage_s;

struct storage_interface *storage;
struct guild_storage_interface *gstorage;

/*==========================================
 * Sort items in the warehouse
 *------------------------------------------*/
int storage_comp_item(const void *i1_, const void *i2_)
{
	const struct item *i1 = i1_;
	const struct item *i2 = i2_;

	if (i1->nameid == i2->nameid)
		return 0;
	else if (!(i1->nameid) || !(i1->amount))
		return 1;
	else if (!(i2->nameid) || !(i2->amount))
		return -1;
	return i1->nameid - i2->nameid;
}

//Sort item by storage_comp_item (nameid)
void storage_sortitem(struct item* items, unsigned int size)
{
	nullpo_retv(items);

	if( battle_config.client_sort_storage )
	{
		qsort(items, size, sizeof(struct item), storage->comp_item);
	}
}

/**
 * Parses storage and saves 'dirty' ones upon reconnect. [Skotlex]
 * @see DBApply
 */
int storage_reconnect_sub(union DBKey key, struct DBData *data, va_list ap)
{
	struct guild_storage *stor = DB->data2ptr(data);
	nullpo_ret(stor);
	if (stor->dirty && stor->storage_status == 0) //Save closed storages.
		gstorage->save(0, stor->guild_id,0);

	return 0;
}

//Function to be invoked upon server reconnection to char. To save all 'dirty' storages [Skotlex]
void do_reconnect_storage(void)
{
	gstorage->db->foreach(gstorage->db, storage->reconnect_sub);
}

/*==========================================
 * Opens a storage. Returns:
 * 0 - success
 * 1 - fail
 *------------------------------------------*/
int storage_storageopen(struct map_session_data *sd)
{
	nullpo_ret(sd);

	if (sd->state.storage_flag != STORAGE_FLAG_CLOSED)
		return 1; //Already open?

	if (sd->storage.received == false) {
		clif->message(sd->fd, msg_sd(sd, 27)); // Storage has not been loaded yet.
		return 1;
	}

	if( !pc_can_give_items(sd) ) {
		//check is this GM level is allowed to put items to storage
		clif->message(sd->fd, msg_sd(sd,246)); // Your GM level doesn't authorize you to perform this action.
		return 1;
	}

	sd->state.storage_flag = STORAGE_FLAG_NORMAL;

	if (sd->storage.aggregate > 0) {
		storage->sortitem(VECTOR_DATA(sd->storage.item), VECTOR_LENGTH(sd->storage.item));
		clif->storagelist(sd, VECTOR_DATA(sd->storage.item), VECTOR_LENGTH(sd->storage.item));
	}

	clif->updatestorageamount(sd, sd->storage.aggregate, MAX_STORAGE);
	return 0;
}

/* helper function
 * checking if 2 item structure are identique
 */
int compare_item(struct item *a, struct item *b)
{
	if( a->nameid == b->nameid &&
		a->identify == b->identify &&
		a->refine == b->refine &&
		a->attribute == b->attribute &&
		a->expire_time == b->expire_time &&
		a->bound == b->bound &&
		a->unique_id == b->unique_id)
	{
		int i = 0, k = 0;
		ARR_FIND(0, MAX_SLOTS, i, a->card[i] != b->card[i]);
		ARR_FIND(0, MAX_ITEM_OPTIONS, k, a->option[k].index != b->option[k].index || a->option[k].value != b->option[k].value);

		if (i == MAX_SLOTS && k == MAX_ITEM_OPTIONS)
			return 1;
	}
	return 0;
}

/*==========================================
 * Internal add-item function.
 *------------------------------------------*/
int storage_additem(struct map_session_data* sd, struct item* item_data, int amount)
{
	struct item_data *data = NULL;
	struct item *it = NULL;
	int i;

	nullpo_retr(1, sd);
	Assert_retr(1, sd->storage.received == true);

	nullpo_retr(1, item_data);
	Assert_retr(1, item_data->nameid > 0);

	Assert_retr(1, amount > 0);

	data = itemdb->search(item_data->nameid);

	if (data->stack.storage && amount > data->stack.amount) // item stack limitation
		return 1;

	if (!itemdb_canstore(item_data, pc_get_group_level(sd))) {
		//Check if item is storable. [Skotlex]
		clif->message (sd->fd, msg_sd(sd, 264)); // This item cannot be stored.
		return 1;
	}

	if (item_data->bound > IBT_ACCOUNT && !pc_can_give_bound_items(sd)) {
		clif->message(sd->fd, msg_sd(sd, 294)); // This bound item cannot be stored there.
		return 1;
	}

	if (itemdb->isstackable2(data)) {//Stackable
		for (i = 0; i < VECTOR_LENGTH(sd->storage.item); i++) {
			it = &VECTOR_INDEX(sd->storage.item, i);

			if (it->nameid == 0)
				continue;

			if (compare_item(it, item_data)) { // existing items found, stack them
				if (amount > MAX_AMOUNT - it->amount || (data->stack.storage && amount > data->stack.amount - it->amount))
					return 1;

				it->amount += amount;

				clif->storageitemadded(sd, it, i, amount);

				sd->storage.save = true; // set a save flag.

				return 0;
			}
		}
	}

	// Check if storage exceeds limit.
	if (sd->storage.aggregate >= MAX_STORAGE)
		return 1;

	ARR_FIND(0, VECTOR_LENGTH(sd->storage.item), i, VECTOR_INDEX(sd->storage.item, i).nameid == 0);

	if (i == VECTOR_LENGTH(sd->storage.item)) {
		VECTOR_ENSURE(sd->storage.item, 1, 1);
		VECTOR_PUSH(sd->storage.item, *item_data);
		it = &VECTOR_LAST(sd->storage.item);
	} else {
		it = &VECTOR_INDEX(sd->storage.item, i);
		*it = *item_data;
	}

	it->amount = amount;

	sd->storage.aggregate++;

	clif->storageitemadded(sd, it, i, amount);

	clif->updatestorageamount(sd, sd->storage.aggregate, MAX_STORAGE);

	sd->storage.save = true; // set a save flag.

	return 0;
}

/*==========================================
 * Internal del-item function
 *------------------------------------------*/
int storage_delitem(struct map_session_data* sd, int n, int amount)
{
	struct item *it = NULL;

	nullpo_retr(1, sd);

	Assert_retr(1, sd->storage.received == true);

	Assert_retr(1, n >= 0 && n < VECTOR_LENGTH(sd->storage.item));

	it = &VECTOR_INDEX(sd->storage.item, n);

	Assert_retr(1, amount <= it->amount);

	Assert_retr(1, it->nameid > 0);

	it->amount -= amount;

	if (it->amount == 0) {
		memset(it, 0, sizeof(struct item));
		clif->updatestorageamount(sd, --sd->storage.aggregate, MAX_STORAGE);
	}

	sd->storage.save = true;

	if (sd->state.storage_flag == STORAGE_FLAG_NORMAL)
		clif->storageitemremoved(sd, n, amount);

	return 0;
}

/*==========================================
 * Add an item to the storage from the inventory.
 * @index : inventory idx
 * return
 *   0 : fail
 *   1 : success
 *------------------------------------------*/
int storage_add_from_inventory(struct map_session_data* sd, int index, int amount)
{
	nullpo_ret(sd);

	Assert_ret(sd->storage.received == true);

	if (sd->storage.aggregate > MAX_STORAGE)
		return 0; // storage full

	if (index < 0 || index >= MAX_INVENTORY)
		return 0;

	if (sd->status.inventory[index].nameid <= 0)
		return 0; // No item on that spot

	if (amount < 1 || amount > sd->status.inventory[index].amount)
		return 0;

	if (storage->additem(sd, &sd->status.inventory[index], amount) == 0)
		pc->delitem(sd, index, amount, 0, DELITEM_TOSTORAGE, LOG_TYPE_STORAGE);
	else
		clif->dropitem(sd, index, 0);

	return 1;
}

/*==========================================
 * Retrieve an item from the storage into inventory
 * @index : storage idx
 * return
 *   0 : fail
 *   1 : success
 *------------------------------------------*/
int storage_add_to_inventory(struct map_session_data* sd, int index, int amount)
{
	int flag;
	struct item *it = NULL;

	nullpo_ret(sd);

	Assert_ret(sd->storage.received == true);

	if (index < 0 || index >= VECTOR_LENGTH(sd->storage.item))
		return 0;

	it = &VECTOR_INDEX(sd->storage.item, index);

	if (it->nameid <= 0)
		return 0; //Nothing there

	if (amount < 1 || amount > it->amount)
		return 0;

	if ((flag = pc->additem(sd, it, amount, LOG_TYPE_STORAGE)) == 0)
		storage->delitem(sd, index, amount);
	else
		clif->additem(sd, 0, 0, flag);

	return 1;
}

/*==========================================
 * Move an item from cart to storage.
 * @index : cart inventory index
 * return
 *   0 : fail
 *   1 : success
 *------------------------------------------*/
int storage_storageaddfromcart(struct map_session_data* sd, int index, int amount)
{
	nullpo_ret(sd);

	Assert_ret(sd->storage.received == true);

	if (sd->storage.aggregate > MAX_STORAGE)
		return 0; // storage full / storage closed

	if (index < 0 || index >= MAX_CART)
		return 0;

	if( sd->status.cart[index].nameid <= 0 )
		return 0; //No item there.

	if (amount < 1 || amount > sd->status.cart[index].amount)
		return 0;

	if (storage->additem(sd,&sd->status.cart[index],amount) == 0)
		pc->cart_delitem(sd,index,amount,0,LOG_TYPE_STORAGE);

	return 1;
}

/*==========================================
 * Get from Storage to the Cart inventory
 * @index : storage index
 * return
 *   0 : fail
 *   1 : success
 *------------------------------------------*/
int storage_storagegettocart(struct map_session_data* sd, int index, int amount)
{
	int flag = 0;
	struct item *it = NULL;

	nullpo_ret(sd);

	Assert_ret(sd->storage.received == true);

	if (index < 0 || index >= VECTOR_LENGTH(sd->storage.item))
		return 0;

	it = &VECTOR_INDEX(sd->storage.item, index);

	if (it->nameid <= 0)
		return 0; //Nothing there.

	if (amount < 1 || amount > it->amount)
		return 0;

	if ((flag = pc->cart_additem(sd, it, amount, LOG_TYPE_STORAGE)) == 0)
		storage->delitem(sd, index, amount);
	else {
		clif->dropitem(sd, index,0);
		clif->cart_additem_ack(sd, flag == 1?0x0:0x1);
	}

	return 1;
}


/*==========================================
 * Modified By Valaris to save upon closing [massdriller]
 *------------------------------------------*/
void storage_storageclose(struct map_session_data* sd)
{
	int i = 0;

	nullpo_retv(sd);

	Assert_retv(sd->storage.received == true);

	clif->storageclose(sd);

	if (map->save_settings & 4)
		chrif->save(sd, 0); //Invokes the storage saving as well.

	/* Erase deleted account storage items from memory
	 * and resize the vector. */
	while (i < VECTOR_LENGTH(sd->storage.item)) {
		if (VECTOR_INDEX(sd->storage.item, i).nameid == 0) {
			VECTOR_ERASE(sd->storage.item, i);
		} else {
			i++;
		}
	}

	sd->state.storage_flag = STORAGE_FLAG_CLOSED;
}

/*==========================================
 * When quitting the game.
 *------------------------------------------*/
void storage_storage_quit(struct map_session_data* sd, int flag)
{
	nullpo_retv(sd);

	if (map->save_settings&4)
		chrif->save(sd, flag); //Invokes the storage saving as well.

	sd->state.storage_flag = STORAGE_FLAG_CLOSED;
}

/**
 * @see DBCreateData
 */
struct DBData create_guildstorage(union DBKey key, va_list args)
{
	struct guild_storage *gs = NULL;
	gs = (struct guild_storage *) aCalloc(sizeof(struct guild_storage), 1);
	gs->guild_id=key.i;
	return DB->ptr2data(gs);
}

struct guild_storage *guild2storage_ensure(int guild_id)
{
	struct guild_storage *gs = NULL;
	if(guild->search(guild_id) != NULL)
		gs = idb_ensure(gstorage->db,guild_id,gstorage->create);
	return gs;
}

int guild_storage_delete(int guild_id)
{
	idb_remove(gstorage->db,guild_id);
	return 0;
}

/*==========================================
* Attempt to open guild storage for sd
* return
*   0 : success (open or req to create a new one)
*   1 : fail
*   2 : no guild for sd
 *------------------------------------------*/
int storage_guild_storageopen(struct map_session_data* sd)
{
	struct guild_storage *gstor;

	nullpo_ret(sd);

	if(sd->status.guild_id <= 0)
		return 2;

	if (sd->state.storage_flag != STORAGE_FLAG_CLOSED)
		return 1; //Can't open both storages at a time.

	if( !pc_can_give_items(sd) ) { //check is this GM level can open guild storage and store items [Lupus]
		clif->message(sd->fd, msg_sd(sd,246)); // Your GM level doesn't authorize you to perform this action.
		return 1;
	}

	if((gstor = idb_get(gstorage->db,sd->status.guild_id)) == NULL) {
		intif->request_guild_storage(sd->status.account_id,sd->status.guild_id);
		return 0;
	}
	if(gstor->storage_status)
		return 1;

	if( gstor->lock )
		return 1;

	gstor->storage_status = 1;
	sd->state.storage_flag = STORAGE_FLAG_GUILD;
	storage->sortitem(gstor->items, ARRAYLENGTH(gstor->items));
	clif->storagelist(sd, gstor->items, ARRAYLENGTH(gstor->items));
	clif->updatestorageamount(sd, gstor->storage_amount, MAX_GUILD_STORAGE);
	return 0;
}

/*==========================================
* Attempt to add an item in guild storage, then refresh it
* return
*   0 : success
*   1 : fail
 *------------------------------------------*/
int guild_storage_additem(struct map_session_data* sd, struct guild_storage* stor, struct item* item_data, int amount)
{
	struct item_data *data;
	int i;

	nullpo_retr(1, sd);
	nullpo_retr(1, stor);
	nullpo_retr(1, item_data);

	if(item_data->nameid <= 0 || amount <= 0)
		return 1;

	data = itemdb->search(item_data->nameid);

	if( data->stack.guildstorage && amount > data->stack.amount )
	{// item stack limitation
		return 1;
	}

	if (!itemdb_canguildstore(item_data, pc_get_group_level(sd)) || item_data->expire_time) {
		//Check if item is storable. [Skotlex]
		clif->message (sd->fd, msg_sd(sd,264)); // This item cannot be stored.
		return 1;
	}

	if( item_data->bound && item_data->bound != IBT_GUILD && !pc_can_give_bound_items(sd) ) {
		clif->message(sd->fd, msg_sd(sd,294)); // This bound item cannot be stored there.
		return 1;
	}

	if(itemdb->isstackable2(data)){ //Stackable
		for(i=0;i<MAX_GUILD_STORAGE;i++){
			if(compare_item(&stor->items[i], item_data)) {
				if( amount > MAX_AMOUNT - stor->items[i].amount || ( data->stack.guildstorage && amount > data->stack.amount - stor->items[i].amount ) )
					return 1;
				stor->items[i].amount+=amount;
				clif->storageitemadded(sd,&stor->items[i],i,amount);
				stor->dirty = 1;
				return 0;
			}
		}
	}
	//Add item
	for(i=0;i<MAX_GUILD_STORAGE && stor->items[i].nameid;i++);

	if(i>=MAX_GUILD_STORAGE)
		return 1;

	memcpy(&stor->items[i],item_data,sizeof(stor->items[0]));
	stor->items[i].amount=amount;
	stor->storage_amount++;
	clif->storageitemadded(sd,&stor->items[i],i,amount);
	clif->updatestorageamount(sd, stor->storage_amount, MAX_GUILD_STORAGE);
	stor->dirty = 1;
	return 0;
}

/*==========================================
* Attempt to delete an item in guild storage, then refresh it
* return
*   0 : success
*   1 : fail
 *------------------------------------------*/
int guild_storage_delitem(struct map_session_data* sd, struct guild_storage* stor, int n, int amount)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, stor);

	Assert_retr(1, n >= 0 && n < MAX_GUILD_STORAGE);
	if(stor->items[n].nameid==0 || stor->items[n].amount<amount)
		return 1;

	stor->items[n].amount-=amount;
	if(stor->items[n].amount==0){
		memset(&stor->items[n],0,sizeof(stor->items[0]));
		stor->storage_amount--;
		clif->updatestorageamount(sd, stor->storage_amount, MAX_GUILD_STORAGE);
	}
	clif->storageitemremoved(sd,n,amount);
	stor->dirty = 1;
	return 0;
}

/*==========================================
* Attempt to add an item in guild storage from inventory, then refresh it
* @index : inventory idx
* return
*   0 : fail
*   1 : succes
 *------------------------------------------*/
int storage_guild_storageadd(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=idb_get(gstorage->db,sd->status.guild_id));

	if( !stor->storage_status || stor->storage_amount > MAX_GUILD_STORAGE )
		return 0;

	if( index<0 || index>=MAX_INVENTORY )
		return 0;

	if( sd->status.inventory[index].nameid <= 0 )
		return 0;

	if( amount < 1 || amount > sd->status.inventory[index].amount )
		return 0;

	if( stor->lock ) {
		gstorage->close(sd);
		return 0;
	}

	if( gstorage->additem(sd,stor,&sd->status.inventory[index],amount) == 0 )
		pc->delitem(sd, index, amount, 0, DELITEM_TOSTORAGE, LOG_TYPE_GSTORAGE);
	else
		clif->dropitem(sd, index, 0);

	return 1;
}

/*==========================================
* Attempt to retrieve an item from guild storage to inventory, then refresh it
* @index : storage idx
* return
*   0 : fail
*   1 : success
 *------------------------------------------*/
int storage_guild_storageget(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;
	int flag;

	nullpo_ret(sd);
	nullpo_ret(stor=idb_get(gstorage->db,sd->status.guild_id));

	if(!stor->storage_status)
		return 0;

	if(index<0 || index>=MAX_GUILD_STORAGE)
		return 0;

	if(stor->items[index].nameid <= 0)
		return 0;

	if(amount < 1 || amount > stor->items[index].amount)
		return 0;

	if( stor->lock ) {
		gstorage->close(sd);
		return 0;
	}

	if((flag = pc->additem(sd,&stor->items[index],amount,LOG_TYPE_GSTORAGE)) == 0)
		gstorage->delitem(sd,stor,index,amount);
	else //inform fail
		clif->additem(sd,0,0,flag);
	//log_fromstorage(sd, index, 1);

	return 0;
}

/*==========================================
* Attempt to add an item in guild storage from cart, then refresh it
* @index : cart inventory idx
* return
*   0 : fail
*   1 : success
 *------------------------------------------*/
int storage_guild_storageaddfromcart(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=idb_get(gstorage->db,sd->status.guild_id));

	if( !stor->storage_status || stor->storage_amount > MAX_GUILD_STORAGE )
		return 0;

	if( index < 0 || index >= MAX_CART )
		return 0;

	if( sd->status.cart[index].nameid <= 0 )
		return 0;

	if( amount < 1 || amount > sd->status.cart[index].amount )
		return 0;

	if(gstorage->additem(sd,stor,&sd->status.cart[index],amount)==0)
		pc->cart_delitem(sd,index,amount,0,LOG_TYPE_GSTORAGE);

	return 1;
}

/*==========================================
* Attempt to retrieve an item from guild storage to cart, then refresh it
* @index : storage idx
* return
*   0 : fail
*   1 : success
 *------------------------------------------*/
int storage_guild_storagegettocart(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=idb_get(gstorage->db,sd->status.guild_id));

	if(!stor->storage_status)
		return 0;

	if(index<0 || index>=MAX_GUILD_STORAGE)
		return 0;

	if(stor->items[index].nameid<=0)
		return 0;

	if(amount < 1 || amount > stor->items[index].amount)
		return 0;

	if(pc->cart_additem(sd,&stor->items[index],amount,LOG_TYPE_GSTORAGE)==0)
		gstorage->delitem(sd,stor,index,amount);

	return 1;
}

/*==========================================
* Request to save guild storage
* return
*   0 : fail (no storage)
*   1 : success
 *------------------------------------------*/
int storage_guild_storagesave(int account_id, int guild_id, int flag)
{
	struct guild_storage *stor = idb_get(gstorage->db,guild_id);

	if(stor)
	{
		if (flag) //Char quitting, close it.
			stor->storage_status = 0;
		if (stor->dirty)
			intif->send_guild_storage(account_id,stor);
		return 1;
	}
	return 0;
}

/*==========================================
* ACK save of guild storage
* return
*   0 : fail (no storage)
*   1 : success
 *------------------------------------------*/
int storage_guild_storagesaved(int guild_id)
{
	struct guild_storage *stor;

	if((stor=idb_get(gstorage->db,guild_id)) != NULL) {
		if (stor->dirty && stor->storage_status == 0) {
			//Storage has been correctly saved.
			stor->dirty = 0;
		}
		return 1;
	}
	return 0;
}

//Close storage for sd and save it
int storage_guild_storageclose(struct map_session_data* sd)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=idb_get(gstorage->db,sd->status.guild_id));

	clif->storageclose(sd);
	if (stor->storage_status) {
		if (map->save_settings&4)
			chrif->save(sd, 0); //This one also saves the storage. [Skotlex]
		else
			gstorage->save(sd->status.account_id, sd->status.guild_id,0);
		stor->storage_status=0;
	}
	sd->state.storage_flag = STORAGE_FLAG_CLOSED;

	return 0;
}

int storage_guild_storage_quit(struct map_session_data* sd, int flag)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=idb_get(gstorage->db,sd->status.guild_id));

	if(flag) {
		//Only during a guild break flag is 1 (don't save storage)
		sd->state.storage_flag = STORAGE_FLAG_CLOSED;
		stor->storage_status = 0;
		clif->storageclose(sd);
		if (map->save_settings&4)
			chrif->save(sd,0);
		return 0;
	}

	if(stor->storage_status) {
		if (map->save_settings&4)
			chrif->save(sd,0);
		else
			gstorage->save(sd->status.account_id,sd->status.guild_id,1);
	}
	sd->state.storage_flag = STORAGE_FLAG_CLOSED;
	stor->storage_status = 0;

	return 0;
}

void do_init_gstorage(bool minimal)
{
	if (minimal)
		return;
	gstorage->db = idb_alloc(DB_OPT_RELEASE_DATA);
}

void do_final_gstorage(void)
{
	db_destroy(gstorage->db);
}

void storage_defaults(void)
{
	storage = &storage_s;

	/* */
	storage->reconnect = do_reconnect_storage;
	/* */
	storage->delitem = storage_delitem;
	storage->open = storage_storageopen;
	storage->add = storage_add_from_inventory;
	storage->get = storage_add_to_inventory;
	storage->additem = storage_additem;
	storage->addfromcart = storage_storageaddfromcart;
	storage->gettocart = storage_storagegettocart;
	storage->close = storage_storageclose;
	storage->pc_quit = storage_storage_quit;
	storage->comp_item = storage_comp_item;
	storage->sortitem = storage_sortitem;
	storage->reconnect_sub = storage_reconnect_sub;
}

void gstorage_defaults(void)
{
	gstorage = &gstorage_s;

	/* */
	gstorage->init = do_init_gstorage;
	gstorage->final = do_final_gstorage;
	/* */
	gstorage->ensure = guild2storage_ensure;
	gstorage->delete = guild_storage_delete;
	gstorage->open = storage_guild_storageopen;
	gstorage->additem = guild_storage_additem;
	gstorage->delitem = guild_storage_delitem;
	gstorage->add = storage_guild_storageadd;
	gstorage->get = storage_guild_storageget;
	gstorage->addfromcart = storage_guild_storageaddfromcart;
	gstorage->gettocart = storage_guild_storagegettocart;
	gstorage->close = storage_guild_storageclose;
	gstorage->pc_quit = storage_guild_storage_quit;
	gstorage->save = storage_guild_storagesave;
	gstorage->saved = storage_guild_storagesaved;
	gstorage->create = create_guildstorage;
}
