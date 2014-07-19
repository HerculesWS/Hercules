// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atcommand.h"
#include "battle.h"
#include "chrif.h"
#include "clif.h"
#include "guild.h"
#include "intif.h"
#include "itemdb.h"
#include "log.h"
#include "map.h" // struct map_session_data
#include "pc.h"
#include "../common/cbasetypes.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"

struct storage_interface storage_s;
struct guild_storage_interface gstorage_s;

/*==========================================
 * Sort items in the warehouse
 *------------------------------------------*/
int storage_comp_item(const void *i1_, const void *i2_)
{
	struct item *i1 = (struct item *)i1_;
	struct item *i2 = (struct item *)i2_;

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
int storage_reconnect_sub(DBKey key, DBData *data, va_list ap)
{
	struct guild_storage *stor = DB->data2ptr(data);
	if( stor->dirty && !stor->storage_status ) //Save closed storages.
		gstorage->save(0, stor->guild_id,0);

	return 0;
}

//Function to be invoked upon server reconnection to char. To save all 'dirty' storages [Skotlex]
void do_reconnect_storage(void) {
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

	if(sd->state.storage_flag)
		return 1; //Already open?
	
	if( !pc_can_give_items(sd) )
  	{ //check is this GM level is allowed to put items to storage
		clif->message(sd->fd, msg_txt(246));
		return 1;
	}
	
	sd->state.storage_flag = 1;
	storage->sortitem(sd->status.storage.items, ARRAYLENGTH(sd->status.storage.items));
	clif->storagelist(sd, sd->status.storage.items, ARRAYLENGTH(sd->status.storage.items));
	clif->updatestorageamount(sd, sd->status.storage.storage_amount, MAX_STORAGE);
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
		a->bound == b->bound )
	{
		int i;
		for (i = 0; i < MAX_SLOTS && (a->card[i] == b->card[i]); i++);
		return (i == MAX_SLOTS);
	}
	return 0;
}

/*==========================================
 * Internal add-item function.
 *------------------------------------------*/
int storage_additem(struct map_session_data* sd, struct item* item_data, int amount) {
	struct storage_data* stor = &sd->status.storage;
	struct item_data *data;
	int i;

	if( item_data->nameid <= 0 || amount <= 0 )
		return 1;
	
	data = itemdb->search(item_data->nameid);

	if( data->stack.storage && amount > data->stack.amount )
	{// item stack limitation
		return 1;
	}

	if( !itemdb_canstore(item_data, pc_get_group_level(sd)) )
	{	//Check if item is storable. [Skotlex]
		clif->message (sd->fd, msg_txt(264));
		return 1;
	}
	
	if( item_data->bound > IBT_ACCOUNT && !pc_can_give_bound_items(sd) ) {
		clif->message(sd->fd, msg_txt(294));
		return 1;
	}

	if( itemdb->isstackable2(data) )
	{//Stackable
		for( i = 0; i < MAX_STORAGE; i++ )
		{
			if( compare_item(&stor->items[i], item_data) )
			{// existing items found, stack them
				if( amount > MAX_AMOUNT - stor->items[i].amount || ( data->stack.storage && amount > data->stack.amount - stor->items[i].amount ) )
					return 1;
				stor->items[i].amount += amount;
				clif->storageitemadded(sd,&stor->items[i],i,amount);
				return 0;
			}
		}
	}

	// find free slot
	ARR_FIND( 0, MAX_STORAGE, i, stor->items[i].nameid == 0 );
	if( i >= MAX_STORAGE )
		return 1;

	// add item to slot
	memcpy(&stor->items[i],item_data,sizeof(stor->items[0]));
	stor->storage_amount++;
	stor->items[i].amount = amount;
	clif->storageitemadded(sd,&stor->items[i],i,amount);
	clif->updatestorageamount(sd, stor->storage_amount, MAX_STORAGE);

	return 0;
}

/*==========================================
 * Internal del-item function
 *------------------------------------------*/
int storage_delitem(struct map_session_data* sd, int n, int amount)
{
	if( sd->status.storage.items[n].nameid == 0 || sd->status.storage.items[n].amount < amount )
		return 1;

	sd->status.storage.items[n].amount -= amount;
	if( sd->status.storage.items[n].amount == 0 )
	{
		memset(&sd->status.storage.items[n],0,sizeof(sd->status.storage.items[0]));
		sd->status.storage.storage_amount--;
		if( sd->state.storage_flag == 1 ) clif->updatestorageamount(sd, sd->status.storage.storage_amount, MAX_STORAGE);
	}
	if( sd->state.storage_flag == 1 ) clif->storageitemremoved(sd,n,amount);
	return 0;
}

/*==========================================
 * Add an item to the storage from the inventory.
 * @index : inventory idx
 * return
 *	0 : fail
 *	1 : success
 *------------------------------------------*/
int storage_storageadd(struct map_session_data* sd, int index, int amount) {
	nullpo_ret(sd);

	if( sd->status.storage.storage_amount > MAX_STORAGE )
		return 0; // storage full

	if( index < 0 || index >= MAX_INVENTORY )
		return 0;

	if( sd->status.inventory[index].nameid <= 0 )
		return 0; // No item on that spot

	if( amount < 1 || amount > sd->status.inventory[index].amount )
  		return 0;

	if( storage->additem(sd,&sd->status.inventory[index],amount) == 0 )
		pc->delitem(sd,index,amount,0,4,LOG_TYPE_STORAGE);
	else
		clif->dropitem(sd, index,0);

	return 1;
}

/*==========================================
 * Retrieve an item from the storage into inventory
 * @index : storage idx
 * return
 *	0 : fail
 *	1 : success
 *------------------------------------------*/
int storage_storageget(struct map_session_data* sd, int index, int amount)
{
	int flag;

	if( index < 0 || index >= MAX_STORAGE )
		return 0;

	if( sd->status.storage.items[index].nameid <= 0 )
		return 0; //Nothing there
	
	if( amount < 1 || amount > sd->status.storage.items[index].amount )
		return 0;

	if( (flag = pc->additem(sd,&sd->status.storage.items[index],amount,LOG_TYPE_STORAGE)) == 0 )
		storage->delitem(sd,index,amount);
	else
		clif->additem(sd,0,0,flag);

	return 1;
}

/*==========================================
 * Move an item from cart to storage.
 * @index : cart inventory index
 * return
 *	0 : fail
 *	1 : success
 *------------------------------------------*/
int storage_storageaddfromcart(struct map_session_data* sd, int index, int amount)
{
	nullpo_ret(sd);

	if( sd->status.storage.storage_amount > MAX_STORAGE )
  		return 0; // storage full / storage closed

	if( index < 0 || index >= MAX_CART )
  		return 0;

	if( sd->status.cart[index].nameid <= 0 )
		return 0; //No item there.
	
	if( amount < 1 || amount > sd->status.cart[index].amount )
		return 0;

	if( storage->additem(sd,&sd->status.cart[index],amount) == 0 )
		pc->cart_delitem(sd,index,amount,0,LOG_TYPE_STORAGE);

	return 1;
}

/*==========================================
 * Get from Storage to the Cart inventory
 * @index : storage index
 * return
 *	0 : fail
 *	1 : success
 *------------------------------------------*/
int storage_storagegettocart(struct map_session_data* sd, int index, int amount) {
	int flag = 0;
	nullpo_ret(sd);

	if( index < 0 || index >= MAX_STORAGE )
		return 0;
	
	if( sd->status.storage.items[index].nameid <= 0 )
		return 0; //Nothing there.
	
	if( amount < 1 || amount > sd->status.storage.items[index].amount )
		return 0;
	
	if( (flag = pc->cart_additem(sd,&sd->status.storage.items[index],amount,LOG_TYPE_STORAGE)) == 0 )
		storage->delitem(sd,index,amount);
	else {
		clif->dropitem(sd, index,0);
		clif->cart_additem_ack(sd,flag == 1?0x0:0x1);
	}

	return 1;
}


/*==========================================
 * Modified By Valaris to save upon closing [massdriller]
 *------------------------------------------*/
void storage_storageclose(struct map_session_data* sd) {
	nullpo_retv(sd);

	clif->storageclose(sd);

	if( map->save_settings&4 )
		chrif->save(sd,0); //Invokes the storage saving as well.

	sd->state.storage_flag = 0;
}

/*==========================================
 * When quitting the game.
 *------------------------------------------*/
void storage_storage_quit(struct map_session_data* sd, int flag) {
	nullpo_retv(sd);
	
	if (map->save_settings&4)
		chrif->save(sd, flag); //Invokes the storage saving as well.

	sd->state.storage_flag = 0;
}

/**
 * @see DBCreateData
 */
DBData create_guildstorage(DBKey key, va_list args)
{
	struct guild_storage *gs = NULL;
	gs = (struct guild_storage *) aCalloc(sizeof(struct guild_storage), 1);
	gs->guild_id=key.i;
	return DB->ptr2data(gs);
}

/**
 * Verifies if guild_storage was already initialized, if not creates it
 *
 * @retval success pointer to guild_storage
 * @retval NULL Nonexistent guild
 **/
struct guild_storage *guild2storage(int guild_id)
{
	struct guild_storage *gs = NULL;
	if(guild->search(guild_id) != NULL)
		gs = idb_ensure(gstorage->db,guild_id,gstorage->create);
	return gs;
}

//For just locating a storage without creating one. [Skotlex]
struct guild_storage *guild2storage2(int guild_id) {
	return (struct guild_storage*)idb_get(gstorage->db,guild_id);
}

/**
 * Deletes a guild storage
 **/
void guild_storage_delete(int guild_id) {
	struct guild_storage *gs;

	gs = guild2storage2(guild_id);
	if( gs == NULL )
		return;

	if( gs->items )
		aFree(gs->items);

	idb_remove(gstorage->db, guild_id);
	return;
}

/**
 * Allocates memory for gs->items using aCalloc
 *  Memory is freed at gstorage->final and at gstorage->delete
 *  If aCalloc is not used to allocate memory unkown items may be saved in `guild_storage` table
 * Called from intif_parse_LoadGuildStorage
 * @retval true success
 **/
bool storage_guild_storage_allocate_items( struct guild_storage *gs ) {
	struct guild *g;
	nullpo_retr(false,gs);

	if( gs->items )
		return false;

	if( (g = guild->search(gs->guild_id)) == 0 ) {
		ShowWarning("storage_guild_storage_create_items: Invalid guild_id (%d)!\n", gs->guild_id);
		return false;
	}

	// It is expected that g->max_storage is already available, but it doesn't hurt to
	// re-set it to the proper value
#if PACKETVER >= 20140205
	g->max_storage = guild->checkskill(g, GD_GUILD_STORAGE)*100;
#else
	g->max_storage = MAX_GUILD_STORAGE;
#endif

	gs->items = aCalloc(((gs->storage_amount > g->max_storage)? gs->storage_amount : g->max_storage), sizeof(gs->items[0]));
	return true;
}

/**
 * Attempt to open guild storage for sd
 * @retval 0 success (open or req to create a new one)
 * @retval 1 storage is already open (storage_flag)
 * @retval 2 sd has no guild / guild information hasn't arrived yet
 * @retval 3 sd's guild doesn't have GD_GUILD_STORAGE (msg_txt(335)) (PACKETVER >= 20140205)
 * @retval 4 sd doesn't have permission to use storage (msg_txt(336)) (PACKETVER >= 20140205)
 **/
int storage_guild_storageopen( struct map_session_data* sd ) {
	struct guild_storage *gstor;

	nullpo_retr(1,sd);

	if( sd->status.guild_id <= 0 || !sd->guild )
		return 2;

	if(sd->state.storage_flag)
		return 1; // Can't open both storages at a time.

	if( !guild->checkskill(sd->guild, GD_GUILD_STORAGE) )
		return 3; // Can't open storage if guild has none

#if PACKETVER >= 20140205
	{
		int pos;
		if( (pos = guild->getposition(sd->guild, sd)) < 0 || !(sd->guild->position[pos].mode&0x100) )
			return 4; // Doesn't have permission
	}
#endif

	if( !pc_can_give_items(sd) ) { //check is this GM level can open guild storage and store items [Lupus]
		clif->message(sd->fd, msg_txt(246));
		return 1;
	}

	if( (gstor = gstorage->id2storage2(sd->status.guild_id)) == NULL || gstor->items == NULL ) {
		intif->request_guild_storage(sd->status.account_id,sd->status.guild_id);
		return 0;
	}

	if( gstor->storage_status )
		return 1;
		
	if( gstor->lock )
		return 1;
	
	gstor->storage_status = true;
	sd->state.storage_flag = 2;
	storage->sortitem(gstor->items, gstor->storage_amount);
	clif->storagelist(sd, gstor->items, gstor->storage_amount);
	clif->updatestorageamount(sd, gstor->storage_amount, sd->guild->max_storage);
	return 0;
}

/*==========================================
* Attempt to add an item in guild storage, then refresh it
* return
* 	0 : success
* 	1 : fail
 *------------------------------------------*/
int guild_storage_additem(struct map_session_data* sd, struct guild_storage* stor, struct item* item_data, int amount)
{
	struct item_data *data;
	int i;

	nullpo_retr(1, sd);
	nullpo_retr(1, sd->guild);
	nullpo_retr(1, stor);
	nullpo_retr(1, item_data);

	if(item_data->nameid <= 0 || amount <= 0)
		return 1;

	data = itemdb->search(item_data->nameid);

	if( data->stack.guildstorage && amount > data->stack.amount )
	{// item stack limitation
		return 1;
	}

	if( !itemdb_canguildstore(item_data, pc_get_group_level(sd)) || item_data->expire_time )
 	{	//Check if item is storable. [Skotlex]
		clif->message (sd->fd, msg_txt(264));
		return 1;
	}

	if( item_data->bound && item_data->bound != IBT_GUILD && !pc_can_give_bound_items(sd) ) {
		clif->message(sd->fd, msg_txt(294));
		return 1;
	}
	
	if( itemdb->isstackable2(data) ) { // Stackable
		for( i = 0; i < sd->guild->max_storage; i++ ) {
			if(compare_item(&stor->items[i], item_data)) {
				if( amount > MAX_AMOUNT - stor->items[i].amount || ( data->stack.guildstorage && amount > data->stack.amount - stor->items[i].amount ) )
					return 1;
				stor->items[i].amount+=amount;
				clif->storageitemadded(sd,&stor->items[i],i,amount);
				stor->dirty = true;
				return 0;
			}
		}
	}

	// Add item
	for( i = 0; i < sd->guild->max_storage && stor->items[i].nameid; i++ );
	
	if( i >= sd->guild->max_storage )
		return 1;
	
	memcpy(&stor->items[i],item_data,sizeof(stor->items[0]));
	stor->items[i].amount=amount;
	stor->storage_amount++;
	clif->storageitemadded(sd,&stor->items[i],i,amount);
	clif->updatestorageamount(sd, stor->storage_amount, sd->guild->max_storage);
	stor->dirty = true;
	return 0;
}

/*==========================================
* Attempt to delete an item in guild storage, then refresh it
* return
* 	0 : success
* 	1 : fail
 *------------------------------------------*/
int guild_storage_delitem(struct map_session_data* sd, struct guild_storage* stor, int n, int amount)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, stor);

	if(stor->items[n].nameid==0 || stor->items[n].amount<amount)
		return 1;

	stor->items[n].amount-=amount;
	if(stor->items[n].amount==0){
		memset(&stor->items[n],0,sizeof(stor->items[0]));
		stor->storage_amount--;
		clif->updatestorageamount(sd, stor->storage_amount, sd->guild->max_storage);
	}
	clif->storageitemremoved(sd,n,amount);
	stor->dirty = true;
	return 0;
}

/*==========================================
* Attempt to add an item in guild storage from inventory, then refresh it
* @index : inventory idx
* return
* 	0 : fail
* 	1 : succes
 *------------------------------------------*/
int storage_guild_storageadd(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(sd->guild);
	nullpo_ret(stor=gstorage->id2storage2(sd->status.guild_id));
		
	if( !stor->storage_status || stor->storage_amount > sd->guild->max_storage )
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

	if(gstorage->additem(sd,stor,&sd->status.inventory[index],amount)==0)
		pc->delitem(sd,index,amount,0,4,LOG_TYPE_GSTORAGE);
	else
		clif->dropitem(sd, index,0);

	return 1;
}

/*==========================================
* Attempt to retrieve an item from guild storage to inventory, then refresh it
* @index : storage idx
* return
* 	0 : fail
* 	1 : success
 *------------------------------------------*/
int storage_guild_storageget(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;
	int flag;

	nullpo_ret(sd);
	nullpo_ret(sd->guild);
	nullpo_ret(stor=guild2storage2(sd->status.guild_id));

	if(!stor->storage_status)
  		return 0;
	
	if( index < 0 || index >= sd->guild->max_storage )
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
//	log_fromstorage(sd, index, 1);

	return 0;
}

/*==========================================
* Attempt to add an item in guild storage from cart, then refresh it
* @index : cart inventory idx
* return
* 	0 : fail
* 	1 : success
 *------------------------------------------*/
int storage_guild_storageaddfromcart(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(sd->guild);
	nullpo_ret(stor=guild2storage2(sd->status.guild_id));

	if( !stor->storage_status || stor->storage_amount > sd->guild->max_storage )
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
* 	0 : fail
* 	1 : success
 *------------------------------------------*/
int storage_guild_storagegettocart(struct map_session_data* sd, int index, int amount)
{
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(sd->guild);
	nullpo_ret(stor=guild2storage2(sd->status.guild_id));

	if(!stor->storage_status)
	  	return 0;

	if( index < 0 || index >= sd->guild->max_storage )
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
* 	0 : fail (no storage)
* 	1 : success
 *------------------------------------------*/
int storage_guild_storagesave(int account_id, int guild_id, int flag) {
	struct guild_storage *stor = guild2storage2(guild_id);

	if( stor ) {
		if( flag ) //Char quitting, close it.
			stor->storage_status = false;
	 	if( stor->dirty )
			intif->send_guild_storage(account_id,stor);
		return 1;
	}
	return 0;
}

/*==========================================
* ACK save of guild storage
* return
* 	0 : fail (no storage)
* 	1 : success
 *------------------------------------------*/
int storage_guild_storagesaved(int guild_id)
{
	struct guild_storage *stor;

	if((stor=gstorage->id2storage2(guild_id)) != NULL) {
		if( stor->dirty && !stor->storage_status ) { //Storage has been correctly saved.
			stor->dirty = false;
		}
		return 1;
	}
	return 0;
}

//Close storage for sd and save it
int storage_guild_storageclose(struct map_session_data* sd) {
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=gstorage->id2storage2(sd->status.guild_id));

	clif->storageclose(sd);
	if (stor->storage_status) {
		if (map->save_settings&4)
			chrif->save(sd, 0); //This one also saves the storage. [Skotlex]
		else
			gstorage->save(sd->status.account_id, sd->status.guild_id,0);
		stor->storage_status = false;
	}
	sd->state.storage_flag = false;

	return 0;
}

#if PACKETVER >= 20140205
/**
 * Grows g->items
 * Called from guild_skillupack
 * @retval true success
 **/
bool storage_guild_storage_grow( struct guild *g ) {
	struct guild_storage *gs;
	struct item *temp;
	int new_size;

	nullpo_retr(false, g);

	gs = gstorage->id2storage2(g->guild_id);

	if( gs == NULL || gs->items == NULL ) {
		// intif_parse_LoadGuildStorage will handle growing later
		intif->request_guild_storage(0, g->guild_id);
		return false;
	}

	new_size = guild->checkskill(g, GD_GUILD_STORAGE)*100;
	if( new_size > gs->storage_amount ) {
		temp = aRealloc(gs->items, new_size);
		if( !temp ) {
			ShowError("storage_guild_storage_grow: Couldn't grow storage!\n");
			return false;
		}
		gs->items = temp;
	}
	g->max_storage = new_size;

	return true;
}
#endif

int storage_guild_storage_quit(struct map_session_data* sd, int flag) {
	struct guild_storage *stor;

	nullpo_ret(sd);
	nullpo_ret(stor=gstorage->id2storage2(sd->status.guild_id));
	
	if(flag) {
		//Only during a guild break flag is 1 (don't save storage)
		sd->state.storage_flag = 0;
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
	sd->state.storage_flag = 0;
	stor->storage_status = false;

	return 0;
}

void do_init_gstorage(bool minimal) {
	if (minimal)
		return;
	gstorage->db = idb_alloc(DB_OPT_RELEASE_DATA);
}

/**
 * Finalizes guild storage processes
 *	Frees gs->items
 **/
void do_final_gstorage( void ) {
	struct guild_storage *gs;
	DBIterator *iter = db_iterator(gstorage->db);

	for( gs = dbi_first(iter); dbi_exists(iter); gs = dbi_next(iter) ) {
		if( gs->items )
			aFree(gs->items);
	}

	db_destroy(gstorage->db);
}

void storage_defaults(void) {
	storage = &storage_s;

	/* */
	storage->reconnect = do_reconnect_storage;
	/* */
	storage->delitem = storage_delitem;
	storage->open = storage_storageopen;
	storage->add = storage_storageadd;
	storage->get = storage_storageget;
	storage->additem = storage_additem;
	storage->addfromcart = storage_storageaddfromcart;
	storage->gettocart = storage_storagegettocart;
	storage->close = storage_storageclose;
	storage->pc_quit = storage_storage_quit;
	storage->comp_item = storage_comp_item;
	storage->sortitem = storage_sortitem;
	storage->reconnect_sub = storage_reconnect_sub;
}
void gstorage_defaults(void) {
	gstorage = &gstorage_s;
	
	/* */
	gstorage->init = do_init_gstorage;
	gstorage->final = do_final_gstorage;
	/* */
	gstorage->allocate_items = storage_guild_storage_allocate_items;
	gstorage->grow = storage_guild_storage_grow;
	gstorage->id2storage = guild2storage;
	gstorage->id2storage2 = guild2storage2;
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
