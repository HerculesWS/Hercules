// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "config/core.h" // DBPATH, RENEWAL
#include "itemdb.h"

#include "map/battle.h" // struct battle_config
#include "map/map.h"
#include "map/mob.h"    // MAX_MOB_DB
#include "map/pc.h"     // W_MUSICAL, W_WHIP
#include "map/script.h" // item script processing
#include "common/HPM.h"
#include "common/conf.h"
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct itemdb_interface itemdb_s;
struct itemdb_interface *itemdb;

/**
 * Search for item name
 * name = item alias, so we should find items aliases first. if not found then look for "jname" (full name)
 * @see DBApply
 */
int itemdb_searchname_sub(DBKey key, DBData *data, va_list ap)
{
	struct item_data *item = DB->data2ptr(data), **dst, **dst2;
	char *str;
	str=va_arg(ap,char *);
	dst=va_arg(ap,struct item_data **);
	dst2=va_arg(ap,struct item_data **);
	if (item == &itemdb->dummy) return 0;

	//Absolute priority to Aegis code name.
	if (*dst != NULL) return 0;
	if ( battle_config.case_sensitive_aegisnames && strcmp(item->name,str) == 0 )
		*dst=item;
	else if ( !battle_config.case_sensitive_aegisnames && strcasecmp(item->name,str) == 0 )
		*dst=item;

	//Second priority to Client displayed name.
	if (*dst2 != NULL) return 0;
	if( strcmpi(item->jname,str)==0 )
		*dst2=item;
	return 0;
}

/*==========================================
 * Return item data from item name. (lookup)
 *------------------------------------------*/
struct item_data* itemdb_searchname(const char *str) {
	struct item_data* item;
	struct item_data* item2=NULL;
	int i;

	for( i = 0; i < ARRAYLENGTH(itemdb->array); ++i ) {
		item = itemdb->array[i];
		if( item == NULL )
			continue;

		// Absolute priority to Aegis code name.
		if ( battle_config.case_sensitive_aegisnames && strcmp(item->name,str) == 0 )
			return item;
		else if ( !battle_config.case_sensitive_aegisnames && strcasecmp(item->name,str) == 0 )
			return item;

		//Second priority to Client displayed name.
		if (!item2 && strcasecmp(item->jname,str) == 0)
			item2 = item;
	}

	item = NULL;
	itemdb->other->foreach(itemdb->other,itemdb->searchname_sub,str,&item,&item2);
	return item?item:item2;
}
/* name to item data */
struct item_data* itemdb_name2id(const char *str) {
	return strdb_get(itemdb->names,str);
}

/**
 * @see DBMatcher
 */
int itemdb_searchname_array_sub(DBKey key, DBData data, va_list ap)
{
	struct item_data *item = DB->data2ptr(&data);
	char *str;
	str=va_arg(ap,char *);
	if (item == &itemdb->dummy)
		return 1; //Invalid item.
	if(stristr(item->jname,str))
		return 0;
	if(battle_config.case_sensitive_aegisnames && strstr(item->name,str))
		return 0;
	if(!battle_config.case_sensitive_aegisnames && stristr(item->name,str))
		return 0;
	return strcmpi(item->jname,str);
}

/*==========================================
 * Founds up to N matches. Returns number of matches [Skotlex]
 * search flag :
 * 0 - approximate match
 * 1 - exact match
 *------------------------------------------*/
int itemdb_searchname_array(struct item_data** data, int size, const char *str, int flag) {
	struct item_data* item;
	int i;
	int count=0;

	// Search in the array
	for( i = 0; i < ARRAYLENGTH(itemdb->array); ++i )
	{
		item = itemdb->array[i];
		if( item == NULL )
			continue;

		if(
		    (!flag
		    && (stristr(item->jname,str)
		       || (battle_config.case_sensitive_aegisnames && strstr(item->name,str))
		       || (!battle_config.case_sensitive_aegisnames && stristr(item->name,str))
		    ))
		 || (flag
		    && (strcmp(item->jname,str) == 0
		       || (battle_config.case_sensitive_aegisnames && strcmp(item->name,str) == 0)
		       || (!battle_config.case_sensitive_aegisnames && strcasecmp(item->name,str) == 0)
		    ))
		) {
			if( count < size )
				data[count] = item;
			++count;
		}
	}

	// search in the db
	if( count < size )
	{
		DBData *db_data[MAX_SEARCH];
		int db_count = 0;
		size -= count;
		db_count = itemdb->other->getall(itemdb->other, (DBData**)&db_data, size, itemdb->searchname_array_sub, str);
		for (i = 0; i < db_count; i++)
			data[count++] = DB->data2ptr(db_data[i]);
		count += db_count;
	}
	return count;
}
/* [Ind/Hercules] */
int itemdb_chain_item(unsigned short chain_id, int *rate) {
	struct item_chain_entry *entry;

	if( chain_id >= itemdb->chain_count ) {
		ShowError("itemdb_chain_item: unknown chain id %d\n", chain_id);
		return UNKNOWN_ITEM_ID;
	}

	entry = &itemdb->chains[chain_id].items[ rnd()%itemdb->chains[chain_id].qty ];

	if( rnd()%10000 >= entry->rate )
		return 0;

	if( rate )
		rate[0] = entry->rate;
	return entry->id;
}
/* [Ind/Hercules] */
void itemdb_package_item(struct map_session_data *sd, struct item_package *package) {
	int i = 0, get_count, j, flag;

	for( i = 0; i < package->must_qty; i++ ) {
		struct item it;
		memset(&it, 0, sizeof(it));

		it.nameid = package->must_items[i].id;
		it.identify = 1;

		if( package->must_items[i].hours ) {
			it.expire_time = (unsigned int)(time(NULL) + ((package->must_items[i].hours*60)*60));
		}

		if( package->must_items[i].named ) {
			it.card[0] = CARD0_FORGE;
			it.card[1] = 0;
			it.card[2] = GetWord(sd->status.char_id, 0);
			it.card[3] = GetWord(sd->status.char_id, 1);
		}

		if( package->must_items[i].announce )
			clif->package_announce(sd,package->must_items[i].id,package->id);

		if ( package->must_items[i].force_serial )
			it.unique_id = itemdb->unique_id(sd);

		get_count = itemdb->isstackable(package->must_items[i].id) ? package->must_items[i].qty : 1;

		it.amount = get_count == 1 ? 1 : get_count;

		for( j = 0; j < package->must_items[i].qty; j += get_count ) {
			if ( ( flag = pc->additem(sd, &it, get_count, LOG_TYPE_SCRIPT) ) )
				clif->additem(sd, 0, 0, flag);
		}
	}

	if( package->random_qty ) {
		for( i = 0; i < package->random_qty; i++ ) {
			struct item_package_rand_entry *entry;

			entry = &package->random_groups[i].random_list[rnd()%package->random_groups[i].random_qty];

			while( 1 ) {
				if( rnd()%10000 >= entry->rate ) {
					entry = entry->next;
					continue;
				} else {
					struct item it;
					memset(&it, 0, sizeof(it));

					it.nameid = entry->id;
					it.identify = 1;

					if( entry->hours ) {
						it.expire_time = (unsigned int)(time(NULL) + ((entry->hours*60)*60));
					}

					if( entry->named ) {
						it.card[0] = CARD0_FORGE;
						it.card[1] = 0;
						it.card[2] = GetWord(sd->status.char_id, 0);
						it.card[3] = GetWord(sd->status.char_id, 1);
					}

					if( entry->announce )
						clif->package_announce(sd,entry->id,package->id);

					get_count = itemdb->isstackable(entry->id) ? entry->qty : 1;

					it.amount = get_count == 1 ? 1 : get_count;

					for( j = 0; j < entry->qty; j += get_count ) {
						if ( ( flag = pc->additem(sd, &it, get_count, LOG_TYPE_SCRIPT) ) )
							clif->additem(sd, 0, 0, flag);
					}
					break;
				}
			}
		}
	}
}

/*==========================================
 * Return a random item id from group. (takes into account % chance giving/tot group)
 *------------------------------------------*/
int itemdb_searchrandomid(struct item_group *group) {

	if (group->qty)
		return group->nameid[rnd()%group->qty];

	ShowError("itemdb_searchrandomid: No item entries for group id %d\n", group->id);
	return UNKNOWN_ITEM_ID;
}
bool itemdb_in_group(struct item_group *group, int nameid) {
	int i;

	for( i = 0; i < group->qty; i++ )
		if( group->nameid[i] == nameid )
			return true;

	return false;
}

/// Searches for the item_data.
/// Returns the item_data or NULL if it does not exist.
struct item_data* itemdb_exists(int nameid)
{
	struct item_data* item;

	if( nameid >= 0 && nameid < ARRAYLENGTH(itemdb->array) )
		return itemdb->array[nameid];
	item = (struct item_data*)idb_get(itemdb->other,nameid);
	if( item == &itemdb->dummy )
		return NULL;// dummy data, doesn't exist
	return item;
}

/// Returns human readable name for given item type.
/// @param type Type id to retrieve name for ( IT_* ).
const char* itemdb_typename(int type)
{
	switch(type)
	{
		case IT_HEALING:        return "Potion/Food";
		case IT_USABLE:         return "Usable";
		case IT_ETC:            return "Etc.";
		case IT_WEAPON:         return "Weapon";
		case IT_ARMOR:          return "Armor";
		case IT_CARD:           return "Card";
		case IT_PETEGG:         return "Pet Egg";
		case IT_PETARMOR:       return "Pet Accessory";
		case IT_AMMO:           return "Arrow/Ammunition";
		case IT_DELAYCONSUME:   return "Delay-Consume Usable";
		case IT_CASH:           return "Cash Usable";
	}
	return "Unknown Type";
}

/*==========================================
 * Converts the jobid from the format in itemdb
 * to the format used by the map server. [Skotlex]
 *------------------------------------------*/
void itemdb_jobid2mapid(unsigned int *bclass, unsigned int jobmask)
{
	int i;
	bclass[0]= bclass[1]= bclass[2]= 0;
	//Base classes
	if (jobmask & 1<<JOB_NOVICE) {
		//Both Novice/Super-Novice are counted with the same ID
		bclass[0] |= 1<<MAPID_NOVICE;
		bclass[1] |= 1<<MAPID_NOVICE;
	}
	for (i = JOB_NOVICE+1; i <= JOB_THIEF; i++)
	{
		if (jobmask & 1<<i)
			bclass[0] |= 1<<(MAPID_NOVICE+i);
	}
	//2-1 classes
	if (jobmask & 1<<JOB_KNIGHT)
		bclass[1] |= 1<<MAPID_SWORDMAN;
	if (jobmask & 1<<JOB_PRIEST)
		bclass[1] |= 1<<MAPID_ACOLYTE;
	if (jobmask & 1<<JOB_WIZARD)
		bclass[1] |= 1<<MAPID_MAGE;
	if (jobmask & 1<<JOB_BLACKSMITH)
		bclass[1] |= 1<<MAPID_MERCHANT;
	if (jobmask & 1<<JOB_HUNTER)
		bclass[1] |= 1<<MAPID_ARCHER;
	if (jobmask & 1<<JOB_ASSASSIN)
		bclass[1] |= 1<<MAPID_THIEF;
	//2-2 classes
	if (jobmask & 1<<JOB_CRUSADER)
		bclass[2] |= 1<<MAPID_SWORDMAN;
	if (jobmask & 1<<JOB_MONK)
		bclass[2] |= 1<<MAPID_ACOLYTE;
	if (jobmask & 1<<JOB_SAGE)
		bclass[2] |= 1<<MAPID_MAGE;
	if (jobmask & 1<<JOB_ALCHEMIST)
		bclass[2] |= 1<<MAPID_MERCHANT;
	if (jobmask & 1<<JOB_BARD)
		bclass[2] |= 1<<MAPID_ARCHER;
#if 0 // Bard/Dancer share the same slot now.
	if (jobmask & 1<<JOB_DANCER)
		bclass[2] |= 1<<MAPID_ARCHER;
#endif // 0
	if (jobmask & 1<<JOB_ROGUE)
		bclass[2] |= 1<<MAPID_THIEF;
	//Special classes that don't fit above.
	if (jobmask & 1<<21) //Taekwon boy
		bclass[0] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<22) //Star Gladiator
		bclass[1] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<23) //Soul Linker
		bclass[2] |= 1<<MAPID_TAEKWON;
	if (jobmask & 1<<JOB_GUNSLINGER)
	{//Rebellion job can equip Gunslinger equips. [Rytech]
		bclass[0] |= 1<<MAPID_GUNSLINGER;
		bclass[1] |= 1<<MAPID_GUNSLINGER;
	}
	if (jobmask & 1<<JOB_NINJA)
		{bclass[0] |= 1<<MAPID_NINJA;
		bclass[1] |= 1<<MAPID_NINJA;}//Kagerou/Oboro jobs can equip Ninja equips. [Rytech]
	if (jobmask & 1<<26) //Bongun/Munak
		bclass[0] |= 1<<MAPID_GANGSI;
	if (jobmask & 1<<27) //Death Knight
		bclass[1] |= 1<<MAPID_GANGSI;
	if (jobmask & 1<<28) //Dark Collector
		bclass[2] |= 1<<MAPID_GANGSI;
	if (jobmask & 1<<29) //Kagerou / Oboro
		bclass[1] |= 1<<MAPID_NINJA;
	if (jobmask & 1<<30) //Rebellion
		bclass[1] |= 1<<MAPID_GUNSLINGER;
}

void create_dummy_data(void)
{
	memset(&itemdb->dummy, 0, sizeof(struct item_data));
	itemdb->dummy.nameid=500;
	itemdb->dummy.weight=1;
	itemdb->dummy.value_sell=1;
	itemdb->dummy.type=IT_ETC; //Etc item
	safestrncpy(itemdb->dummy.name,"UNKNOWN_ITEM",sizeof(itemdb->dummy.name));
	safestrncpy(itemdb->dummy.jname,"UNKNOWN_ITEM",sizeof(itemdb->dummy.jname));
	itemdb->dummy.view_id=UNKNOWN_ITEM_ID;
}

struct item_data* create_item_data(int nameid)
{
	struct item_data *id;
	CREATE(id, struct item_data, 1);
	id->nameid = nameid;
	id->weight = 1;
	id->type = IT_ETC;
	return id;
}

/*==========================================
 * Loads (and creates if not found) an item from the db.
 *------------------------------------------*/
struct item_data* itemdb_load(int nameid) {
	struct item_data *id;

	if( nameid >= 0 && nameid < ARRAYLENGTH(itemdb->array) )
	{
		id = itemdb->array[nameid];
		if( id == NULL || id == &itemdb->dummy )
			id = itemdb->array[nameid] = itemdb->create_item_data(nameid);
		return id;
	}

	id = (struct item_data*)idb_get(itemdb->other, nameid);
	if( id == NULL || id == &itemdb->dummy )
	{
		id = itemdb->create_item_data(nameid);
		idb_put(itemdb->other, nameid, id);
	}
	return id;
}

/*==========================================
 * Loads an item from the db. If not found, it will return the dummy item.
 *------------------------------------------*/
struct item_data* itemdb_search(int nameid)
{
	struct item_data* id;
	if( nameid >= 0 && nameid < ARRAYLENGTH(itemdb->array) )
		id = itemdb->array[nameid];
	else
		id = (struct item_data*)idb_get(itemdb->other, nameid);

	if( id == NULL )
	{
		ShowWarning("itemdb_search: Item ID %d does not exists in the item_db. Using dummy data.\n", nameid);
		id = &itemdb->dummy;
		itemdb->dummy.nameid = nameid;
	}
	return id;
}

/*==========================================
 * Returns if given item is a player-equippable piece.
 *------------------------------------------*/
int itemdb_isequip(int nameid)
{
	int type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
			return 1;
		default:
			return 0;
	}
}

/*==========================================
 * Alternate version of itemdb_isequip
 *------------------------------------------*/
int itemdb_isequip2(struct item_data *data) {
	nullpo_ret(data);
	switch(data->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_AMMO:
			return 1;
		default:
			return 0;
	}
}

/*==========================================
 * Returns if given item's type is stackable.
 *------------------------------------------*/
int itemdb_isstackable(int nameid)
{
	int type=itemdb_type(nameid);
	switch(type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETEGG:
		case IT_PETARMOR:
			return 0;
		default:
			return 1;
	}
}

/*==========================================
 * Alternate version of itemdb_isstackable
 *------------------------------------------*/
int itemdb_isstackable2(struct item_data *data)
{
	nullpo_ret(data);
	switch(data->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETEGG:
		case IT_PETARMOR:
			return 0;
		default:
			return 1;
	}
}

/*==========================================
 * Trade Restriction functions [Skotlex]
 *------------------------------------------*/
int itemdb_isdropable_sub(struct item_data *item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NODROP) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cantrade_sub(struct item_data* item, int gmlv, int gmlv2) {
	return (item && (!(item->flag.trade_restriction&ITR_NOTRADE) || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_canpartnertrade_sub(struct item_data* item, int gmlv, int gmlv2) {
	return (item && (item->flag.trade_restriction&ITR_PARTNEROVERRIDE || gmlv >= item->gm_lv_trade_override || gmlv2 >= item->gm_lv_trade_override));
}

int itemdb_cansell_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NOSELLTONPC) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_cancartstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NOCART) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NOSTORAGE) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canguildstore_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NOGSTORAGE) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canmail_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NOMAIL) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_canauction_sub(struct item_data* item, int gmlv, int unused) {
	return (item && (!(item->flag.trade_restriction&ITR_NOAUCTION) || gmlv >= item->gm_lv_trade_override));
}

int itemdb_isrestricted(struct item* item, int gmlv, int gmlv2, int (*func)(struct item_data*, int, int))
{
	struct item_data* item_data = itemdb->search(item->nameid);
	int i;

	if (!func(item_data, gmlv, gmlv2))
		return 0;

	if(item_data->slot == 0 || itemdb_isspecial(item->card[0]))
		return 1;

	for(i = 0; i < item_data->slot; i++) {
		if (!item->card[i]) continue;
		if (!func(itemdb->search(item->card[i]), gmlv, gmlv2))
			return 0;
	}
	return 1;
}

/*==========================================
 * Specifies if item-type should drop unidentified.
 *------------------------------------------*/
int itemdb_isidentified(int nameid) {
	int type=itemdb_type(nameid);
	switch (type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETARMOR:
			return 0;
		default:
			return 1;
	}
}
/* same as itemdb_isidentified but without a lookup */
int itemdb_isidentified2(struct item_data *data) {
	switch (data->type) {
		case IT_WEAPON:
		case IT_ARMOR:
		case IT_PETARMOR:
			return 0;
		default:
			return 1;
	}
}

void itemdb_read_groups(void) {
	config_t item_group_conf;
	config_setting_t *itg = NULL, *it = NULL;
#ifdef RENEWAL
	const char *config_filename = "db/re/item_group.conf"; // FIXME hardcoded name
#else
	const char *config_filename = "db/pre-re/item_group.conf"; // FIXME hardcoded name
#endif
	const char *itname;
	int i = 0, count = 0, c;
	unsigned int *gsize = NULL;

	if (libconfig->read_file(&item_group_conf, config_filename)) {
		ShowError("can't read %s\n", config_filename);
		return;
	}

	gsize = aMalloc( libconfig->setting_length(item_group_conf.root) * sizeof(unsigned int) );

	for(i = 0; i < libconfig->setting_length(item_group_conf.root); i++)
		gsize[i] = 0;

	i = 0;
	while( (itg = libconfig->setting_get_elem(item_group_conf.root,i++)) ) {
		const char *name = config_setting_name(itg);

		if( !itemdb->name2id(name) ) {
			ShowWarning("itemdb_read_groups: unknown group item '%s', skipping..\n",name);
			config_setting_remove(item_group_conf.root, name);
			--i;
			continue;
		}

		c = 0;
		while( (it = libconfig->setting_get_elem(itg,c++)) ) {
			if( config_setting_is_list(it) )
				gsize[ i - 1 ] += libconfig->setting_get_int_elem(it,1);
			else
				gsize[ i - 1 ] += 1;
		}
	}

	i = 0;
	CREATE(itemdb->groups, struct item_group, libconfig->setting_length(item_group_conf.root));
	itemdb->group_count = (unsigned short)libconfig->setting_length(item_group_conf.root);

	while( (itg = libconfig->setting_get_elem(item_group_conf.root,i++)) ) {
		struct item_data *data = itemdb->name2id(config_setting_name(itg));
		int ecount = 0;

		data->group = &itemdb->groups[count];

		itemdb->groups[count].id = data->nameid;
		itemdb->groups[count].qty = gsize[ count ];

		CREATE(itemdb->groups[count].nameid, unsigned short, gsize[ count ] + 1);
		c = 0;
		while( (it = libconfig->setting_get_elem(itg,c++)) ) {
			int repeat = 1;
			if( config_setting_is_list(it) ) {
				itname = libconfig->setting_get_string_elem(it,0);
				repeat = libconfig->setting_get_int_elem(it,1);
			} else
				itname = libconfig->setting_get_string_elem(itg,c - 1);

			if( itname[0] == 'I' && itname[1] == 'D' && strlen(itname) < 8 ) {
				if( !( data = itemdb->exists(atoi(itname+2)) ) )
					ShowWarning("itemdb_read_groups: unknown item ID '%d' in group '%s'!\n",atoi(itname+2),config_setting_name(itg));
			} else if( !( data = itemdb->name2id(itname) ) )
				ShowWarning("itemdb_read_groups: unknown item '%s' in group '%s'!\n",itname,config_setting_name(itg));

			itemdb->groups[count].nameid[ecount] = data ? data->nameid : 0;
			if( repeat > 1 ) {
				//memset would be better? I failed to get the following to work though hu
				//memset(&itemdb->groups[count].nameid[ecount+1],itemdb->groups[count].nameid[ecount],sizeof(itemdb->groups[count].nameid[0])*repeat);
				int z;
				for( z = ecount+1; z < ecount+repeat; z++ )
					itemdb->groups[count].nameid[z] = itemdb->groups[count].nameid[ecount];
			}
			ecount += repeat;
		}
		count++;
	}

	libconfig->destroy(&item_group_conf);
	aFree(gsize);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, config_filename);
}

/* [Ind/Hercules] - HCache for Packages */
void itemdb_write_cached_packages(const char *config_filename) {
	FILE *file;
	unsigned short pcount = itemdb->package_count;
	unsigned short i;

	if( !(file = HCache->open(config_filename,"wb")) ) {
		return;
	}

	// first 2 bytes = package count
	hwrite(&pcount,sizeof(pcount),1,file);

	for(i = 0; i < pcount; i++) {
		unsigned short id = itemdb->packages[i].id, random_qty = itemdb->packages[i].random_qty, must_qty = itemdb->packages[i].must_qty;
		unsigned short c;
		//into a package, first 2 bytes = id.
		hwrite(&id,sizeof(id),1,file);
		//next 2 bytes = must count
		hwrite(&must_qty,sizeof(must_qty),1,file);
		//next 2 bytes = random count
		hwrite(&random_qty,sizeof(random_qty),1,file);
		//now we loop into must
		for(c = 0; c < must_qty; c++) {
			struct item_package_must_entry *entry = &itemdb->packages[i].must_items[c];
			unsigned char announce = entry->announce == 1 ? 1 : 0, named = entry->named == 1 ? 1 : 0, force_serial = entry->force_serial == 1 ? 1 : 0;
			//first 2 byte = item id
			hwrite(&entry->id,sizeof(entry->id),1,file);
			//next 2 byte = qty
			hwrite(&entry->qty,sizeof(entry->qty),1,file);
			//next 2 byte = hours
			hwrite(&entry->hours,sizeof(entry->hours),1,file);
			//next 1 byte = announce (1:0)
			hwrite(&announce,sizeof(announce),1,file);
			//next 1 byte = named (1:0)
			hwrite(&named,sizeof(named),1,file);
			//next 1 byte = ForceSerial (1:0)
			hwrite(&force_serial,sizeof(force_serial),1,file);
		}
		//now we loop into random groups
		for(c = 0; c < random_qty; c++) {
			struct item_package_rand_group *group = &itemdb->packages[i].random_groups[c];
			unsigned short group_qty = group->random_qty, h;

			//next 2 bytes = how many entries in this group
			hwrite(&group_qty,sizeof(group_qty),1,file);
			//now we loop into the group's list
			for(h = 0; h < group_qty; h++) {
				struct item_package_rand_entry *entry = &itemdb->packages[i].random_groups[c].random_list[h];
				unsigned char announce = entry->announce == 1 ? 1 : 0, named = entry->named == 1 ? 1 : 0, force_serial = entry->force_serial == 1 ? 1 : 0;
				//first 2 byte = item id
				hwrite(&entry->id,sizeof(entry->id),1,file);
				//next 2 byte = qty
				hwrite(&entry->qty,sizeof(entry->qty),1,file);
				//next 2 byte = rate
				hwrite(&entry->rate,sizeof(entry->rate),1,file);
				//next 2 byte = hours
				hwrite(&entry->hours,sizeof(entry->hours),1,file);
				//next 1 byte = announce (1:0)
				hwrite(&announce,sizeof(announce),1,file);
				//next 1 byte = named (1:0)
				hwrite(&named,sizeof(named),1,file);
				//next 1 byte = ForceSerial (1:0)
				hwrite(&force_serial,sizeof(force_serial),1,file);
			}
		}
	}
	fclose(file);

	return;
}
bool itemdb_read_cached_packages(const char *config_filename) {
	FILE *file;
	unsigned short pcount = 0;
	unsigned short i;

	if( !(file = HCache->open(config_filename,"rb")) ) {
		return false;
	}

	// first 2 bytes = package count
	hread(&pcount,sizeof(pcount),1,file);

	CREATE(itemdb->packages, struct item_package, pcount);
	itemdb->package_count = pcount;

	for( i = 0; i < pcount; i++ ) {
		unsigned short id = 0, random_qty = 0, must_qty = 0;
		struct item_data *pdata;
		struct item_package *package = &itemdb->packages[i];
		unsigned short c;

		//into a package, first 2 bytes = id.
		hread(&id,sizeof(id),1,file);
		//next 2 bytes = must count
		hread(&must_qty,sizeof(must_qty),1,file);
		//next 2 bytes = random count
		hread(&random_qty,sizeof(random_qty),1,file);

		if( !(pdata = itemdb->exists(id)) )
			ShowWarning("itemdb_read_cached_packages: unknown package item '%d', skipping..\n",id);
		else
			pdata->package = &itemdb->packages[i];

		package->id = id;
		package->random_qty = random_qty;
		package->must_qty = must_qty;
		package->must_items = NULL;
		package->random_groups = NULL;

		if( package->must_qty ) {
			CREATE(package->must_items, struct item_package_must_entry, package->must_qty);
			//now we loop into must
			for(c = 0; c < package->must_qty; c++) {
				struct item_package_must_entry *entry = &itemdb->packages[i].must_items[c];
				unsigned short mid = 0, qty = 0, hours = 0;
				unsigned char announce = 0, named = 0, force_serial = 0;
				struct item_data *data;
				//first 2 byte = item id
				hread(&mid,sizeof(mid),1,file);
				//next 2 byte = qty
				hread(&qty,sizeof(qty),1,file);
				//next 2 byte = hours
				hread(&hours,sizeof(hours),1,file);
				//next 1 byte = announce (1:0)
				hread(&announce,sizeof(announce),1,file);
				//next 1 byte = named (1:0)
				hread(&named,sizeof(named),1,file);
				//next 1 byte = ForceSerial (1:0)
				hread(&force_serial,sizeof(force_serial),1,file);

				if( !(data = itemdb->exists(mid)) )
					ShowWarning("itemdb_read_cached_packages: unknown item '%d' in package '%s'!\n",mid,itemdb_name(package->id));

				entry->id = data ? data->nameid : 0;
				entry->hours = hours;
				entry->qty = qty;
				entry->announce = announce ? 1 : 0;
				entry->named = named ? 1 : 0;
				entry->force_serial = force_serial ? 1 : 0;
			}
		}
		if( package->random_qty ) {
			//now we loop into random groups
			CREATE(package->random_groups, struct item_package_rand_group, package->random_qty);
			for(c = 0; c < package->random_qty; c++) {
				unsigned short group_qty = 0, h;
				struct item_package_rand_entry *prev = NULL;

				//next 2 bytes = how many entries in this group
				hread(&group_qty,sizeof(group_qty),1,file);

				package->random_groups[c].random_qty = group_qty;
				CREATE(package->random_groups[c].random_list, struct item_package_rand_entry, package->random_groups[c].random_qty);

				//now we loop into the group's list
				for(h = 0; h < group_qty; h++) {
					struct item_package_rand_entry *entry = &itemdb->packages[i].random_groups[c].random_list[h];
					unsigned short mid = 0, qty = 0, hours = 0, rate = 0;
					unsigned char announce = 0, named = 0, force_serial = 0;
					struct item_data *data;

					if( prev ) prev->next = entry;

					//first 2 byte = item id
					hread(&mid,sizeof(mid),1,file);
					//next 2 byte = qty
					hread(&qty,sizeof(qty),1,file);
					//next 2 byte = rate
					hread(&rate,sizeof(rate),1,file);
					//next 2 byte = hours
					hread(&hours,sizeof(hours),1,file);
					//next 1 byte = announce (1:0)
					hread(&announce,sizeof(announce),1,file);
					//next 1 byte = named (1:0)
					hread(&named,sizeof(named),1,file);
					//next 1 byte = ForceSerial (1:0)
					hread(&force_serial,sizeof(force_serial),1,file);

					if( !(data = itemdb->exists(mid)) )
						ShowWarning("itemdb_read_cached_packages: unknown item '%d' in package '%s'!\n",mid,itemdb_name(package->id));

					entry->id = data ? data->nameid : 0;
					entry->rate = rate;
					entry->hours = hours;
					entry->qty = qty;
					entry->announce = announce ? 1 : 0;
					entry->named = named ? 1 : 0;
					entry->force_serial = force_serial ? 1 : 0;
					prev = entry;
				}
				if( prev )
					prev->next = &itemdb->packages[i].random_groups[c].random_list[0];
			}
		}
	}
	fclose(file);
	ShowStatus("Done reading '"CL_WHITE"%hu"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"' ("CL_GREEN"C"CL_RESET").\n", pcount, config_filename);

	return true;
}
void itemdb_read_packages(void) {
	config_t item_packages_conf;
	config_setting_t *itg = NULL, *it = NULL, *t = NULL;
#ifdef RENEWAL
	const char *config_filename = "db/re/item_packages.conf"; // FIXME hardcoded name
#else
	const char *config_filename = "db/pre-re/item_packages.conf"; // FIXME hardcoded name
#endif
	const char *itname;
	int i = 0, count = 0, c = 0, highest_gcount = 0;
	unsigned int *must = NULL, *random = NULL, *rgroup = NULL, **rgroups = NULL;
	struct item_package_rand_entry **prev = NULL;

	if( HCache->check(config_filename) ) {
		if( itemdb->read_cached_packages(config_filename) )
			return;
	}

	if (libconfig->read_file(&item_packages_conf, config_filename)) {
		ShowError("can't read %s\n", config_filename);
		return;
	}

	must = aMalloc( libconfig->setting_length(item_packages_conf.root) * sizeof(unsigned int) );
	random = aMalloc( libconfig->setting_length(item_packages_conf.root) * sizeof(unsigned int) );
	rgroup = aMalloc( libconfig->setting_length(item_packages_conf.root) * sizeof(unsigned int) );
	rgroups = aMalloc( libconfig->setting_length(item_packages_conf.root) * sizeof(unsigned int *) );

	for(i = 0; i < libconfig->setting_length(item_packages_conf.root); i++) {
		must[i] = 0;
		random[i] = 0;
		rgroup[i] = 0;
		rgroups[i] = NULL;
	}

	/* validate tree, drop poisonous fruits! */
	i = 0;
	while( (itg = libconfig->setting_get_elem(item_packages_conf.root,i++)) ) {
		const char *name = config_setting_name(itg);

		if( !itemdb->name2id(name) ) {
			ShowWarning("itemdb_read_packages: unknown package item '%s', skipping..\n",name);
			libconfig->setting_remove(item_packages_conf.root, name);
			--i;
			continue;
		}

		c = 0;
		while( (it = libconfig->setting_get_elem(itg,c++)) ) {
			int rval = 0;
			if( !( t = libconfig->setting_get_member(it, "Random") ) || (rval = libconfig->setting_get_int(t)) < 0 ) {
				ShowWarning("itemdb_read_packages: invalid 'Random' value (%d) for item '%s' in package '%s', defaulting to must!\n",rval,config_setting_name(it),name);
				libconfig->setting_remove(it, config_setting_name(it));
				--c;
				continue;
			}

			if( rval == 0 )
				must[ i - 1 ] += 1;
			else {
				random[ i - 1 ] += 1;
				if( rval > rgroup[i - 1] )
					rgroup[i - 1] = rval;
				if( rval > highest_gcount )
					highest_gcount = rval;
			}
		}
	}

	CREATE(prev, struct item_package_rand_entry *, highest_gcount);
	for(i = 0; i < highest_gcount; i++) {
		prev[i] = NULL;
	}

	for(i = 0; i < libconfig->setting_length(item_packages_conf.root); i++ ) {
		rgroups[i] = aMalloc( rgroup[i] * sizeof(unsigned int) );
		for( c = 0; c < rgroup[i]; c++ ) {
			rgroups[i][c] = 0;
		}
	}

	/* grab the known sizes */
	i = 0;
	while( (itg = libconfig->setting_get_elem(item_packages_conf.root,i++)) ) {
	   c = 0;
	   while( (it = libconfig->setting_get_elem(itg,c++)) ) {
		   int rval = 0;
		   if ((t = libconfig->setting_get_member(it, "Random")) != NULL && (rval = libconfig->setting_get_int(t)) > 0) {
			   rgroups[i - 1][rval - 1] += 1;
		   }
		}
	}

	CREATE(itemdb->packages, struct item_package, libconfig->setting_length(item_packages_conf.root));
	itemdb->package_count = (unsigned short)libconfig->setting_length(item_packages_conf.root);

	/* write */
	i = 0;
	while( (itg = libconfig->setting_get_elem(item_packages_conf.root,i++)) ) {
		struct item_data *data = itemdb->name2id(config_setting_name(itg));
		int r = 0, m = 0;

		for(r = 0; r < highest_gcount; r++) {
			prev[r] = NULL;
		}

		data->package = &itemdb->packages[count];

		itemdb->packages[count].id  = data->nameid;
		itemdb->packages[count].random_groups = NULL;
		itemdb->packages[count].must_items = NULL;
		itemdb->packages[count].random_qty = rgroup[ i - 1 ];
		itemdb->packages[count].must_qty = must[ i - 1 ];

		if( itemdb->packages[count].random_qty ) {
			CREATE(itemdb->packages[count].random_groups, struct item_package_rand_group, itemdb->packages[count].random_qty);
			for( c = 0; c < itemdb->packages[count].random_qty; c++ ) {
				if( !rgroups[ i - 1 ][c] )
					ShowError("itemdb_read_packages: package '%s' missing 'Random' field %d! there must not be gaps!\n",config_setting_name(itg),c+1);
				else
					CREATE(itemdb->packages[count].random_groups[c].random_list, struct item_package_rand_entry, rgroups[ i - 1 ][c]);
				itemdb->packages[count].random_groups[c].random_qty = 0;
			}
		}
		if( itemdb->packages[count].must_qty )
			CREATE(itemdb->packages[count].must_items, struct item_package_must_entry, itemdb->packages[count].must_qty);

		c = 0;
		while( (it = libconfig->setting_get_elem(itg,c++)) ) {
			int icount = 1, expire = 0, rate = 10000, gid = 0;
			bool announce = false, named = false, force_serial = false;

			itname = config_setting_name(it);

			if( itname[0] == 'I' && itname[1] == 'D' && strlen(itname) < 8 ) {
				if( !( data = itemdb->exists(atoi(itname+2)) ) )
					ShowWarning("itemdb_read_packages: unknown item ID '%d' in package '%s'!\n",atoi(itname+2),config_setting_name(itg));
			} else if( !( data = itemdb->name2id(itname) ) )
				ShowWarning("itemdb_read_packages: unknown item '%s' in package '%s'!\n",itname,config_setting_name(itg));

			if( ( t = libconfig->setting_get_member(it, "Count")) )
				icount = libconfig->setting_get_int(t);

			if( ( t = libconfig->setting_get_member(it, "Expire")) )
				expire = libconfig->setting_get_int(t);

			if( ( t = libconfig->setting_get_member(it, "Rate")) ) {
				if( (rate = (unsigned short)libconfig->setting_get_int(t)) > 10000 ) {
					ShowWarning("itemdb_read_packages: invalid rate (%d) for item '%s' in package '%s'!\n",rate,itname,config_setting_name(itg));
					rate = 10000;
				}
			}

			if( ( t = libconfig->setting_get_member(it, "Announce")) && libconfig->setting_get_bool(t) )
				announce = true;

			if( ( t = libconfig->setting_get_member(it, "Named")) && libconfig->setting_get_bool(t) )
				named = true;

			if( ( t = libconfig->setting_get_member(it, "ForceSerial")) && libconfig->setting_get_bool(t) )
				force_serial = true;

			if( !( t = libconfig->setting_get_member(it, "Random") ) ) {
				ShowWarning("itemdb_read_packages: missing 'Random' field for item '%s' in package '%s', defaulting to must!\n",itname,config_setting_name(itg));
				gid = 0;
			} else
				gid = libconfig->setting_get_int(t);

			if( gid == 0 ) {
				itemdb->packages[count].must_items[m].id = data ? data->nameid : 0;
				itemdb->packages[count].must_items[m].qty = icount;
				itemdb->packages[count].must_items[m].hours = expire;
				itemdb->packages[count].must_items[m].announce = announce == true ? 1 : 0;
				itemdb->packages[count].must_items[m].named = named == true ? 1 : 0;
				itemdb->packages[count].must_items[m].force_serial = force_serial == true ? 1 : 0;
				m++;
			} else {
				int gidx = gid - 1;

				r = itemdb->packages[count].random_groups[gidx].random_qty;

				if( prev[gidx] )
					prev[gidx]->next = &itemdb->packages[count].random_groups[gidx].random_list[r];

				itemdb->packages[count].random_groups[gidx].random_list[r].id = data ? data->nameid : 0;
				itemdb->packages[count].random_groups[gidx].random_list[r].qty = icount;
				if( (itemdb->packages[count].random_groups[gidx].random_list[r].rate = rate) == 10000 ) {
					ShowWarning("itemdb_read_packages: item '%s' in '%s' has 100%% drop rate!! set this item as 'Random: 0' or other items won't drop!!!\n",itname,config_setting_name(itg));
				}
				itemdb->packages[count].random_groups[gidx].random_list[r].hours = expire;
				itemdb->packages[count].random_groups[gidx].random_list[r].announce = announce == true ? 1 : 0;
				itemdb->packages[count].random_groups[gidx].random_list[r].named = named == true ? 1 : 0;
				itemdb->packages[count].random_groups[gidx].random_list[r].force_serial = force_serial == true ? 1 : 0;
				itemdb->packages[count].random_groups[gidx].random_qty += 1;

				prev[gidx] = &itemdb->packages[count].random_groups[gidx].random_list[r];
			}
		}

		for(r = 0; r < highest_gcount; r++) {
			if( prev[r] )
				prev[r]->next = &itemdb->packages[count].random_groups[r].random_list[0];
		}

		for( r = 0; r < itemdb->packages[count].random_qty; r++  ) {
			if( itemdb->packages[count].random_groups[r].random_qty == 1 ) {
				//item packages don't stop looping until something comes out of them, so if you have only one item in it the drop is guaranteed.
				ShowWarning("itemdb_read_packages: in '%s' 'Random: %d' group has only 1 random option, drop rate will be 100%%!\n",
				            itemdb_name(itemdb->packages[count].id),r+1);
				itemdb->packages[count].random_groups[r].random_list[0].rate = 10000;
			}
		}
		count++;
	}

	aFree(must);
	aFree(random);
	for(i = 0; i < libconfig->setting_length(item_packages_conf.root); i++ ) {
		aFree(rgroups[i]);
	}
	aFree(rgroups);
	aFree(rgroup);
	aFree(prev);

	libconfig->destroy(&item_packages_conf);

	if( HCache->enabled )
		itemdb->write_cached_packages(config_filename);

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, config_filename);
}

void itemdb_read_chains(void) {
	config_t item_chain_conf;
	config_setting_t *itc = NULL;
#ifdef RENEWAL
	const char *config_filename = "db/re/item_chain.conf"; // FIXME hardcoded name
#else
	const char *config_filename = "db/pre-re/item_chain.conf"; // FIXME hardcoded name
#endif
	int i = 0, count = 0;

	if (libconfig->read_file(&item_chain_conf, config_filename)) {
		ShowError("can't read %s\n", config_filename);
		return;
	}

	CREATE(itemdb->chains, struct item_chain, libconfig->setting_length(item_chain_conf.root));
	itemdb->chain_count = (unsigned short)libconfig->setting_length(item_chain_conf.root);

#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = config_filename;
#endif // ENABLE_CASE_CHECK
	while( (itc = libconfig->setting_get_elem(item_chain_conf.root,i++)) ) {
		struct item_data *data = NULL;
		struct item_chain_entry *prev = NULL;
		const char *name = config_setting_name(itc);
		int c = 0;
		config_setting_t *entry = NULL;

		script->set_constant2(name,i-1,0);
		itemdb->chains[count].qty = (unsigned short)libconfig->setting_length(itc);

		CREATE(itemdb->chains[count].items, struct item_chain_entry, libconfig->setting_length(itc));

		while( (entry = libconfig->setting_get_elem(itc,c++)) ) {
			const char *itname = config_setting_name(entry);
			if( itname[0] == 'I' && itname[1] == 'D' && strlen(itname) < 8 ) {
				if( !( data = itemdb->exists(atoi(itname+2)) ) )
					ShowWarning("itemdb_read_chains: unknown item ID '%d' in chain '%s'!\n",atoi(itname+2),name);
			} else if( !( data = itemdb->name2id(itname) ) )
				ShowWarning("itemdb_read_chains: unknown item '%s' in chain '%s'!\n",itname,name);

			if( prev )
				prev->next = &itemdb->chains[count].items[c - 1];

			itemdb->chains[count].items[c - 1].id = data ? data->nameid : 0;
			itemdb->chains[count].items[c - 1].rate = data ? libconfig->setting_get_int(entry) : 0;

			prev = &itemdb->chains[count].items[c - 1];
		}

		if( prev )
			prev->next = &itemdb->chains[count].items[0];

		count++;
	}
#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = NULL;
#endif // ENABLE_CASE_CHECK

	libconfig->destroy(&item_chain_conf);

	if( !script->get_constant("ITMCHAIN_ORE",&i) )
		ShowWarning("itemdb_read_chains: failed to find 'ITMCHAIN_ORE' chain to link to cache!\n");
	else
		itemdb->chain_cache[ECC_ORE] = i;

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, config_filename);
}

/**
 * @return: amount of retrieved entries.
 **/
int itemdb_combo_split_atoi (char *str, int *val) {
	int i;

	for (i=0; i<MAX_ITEMS_PER_COMBO; i++) {
		if (!str) break;

		val[i] = atoi(str);
		str = strchr(str,':');
		if (str)
			*str++=0;
	}

	if( i == 0 ) //No data found.
		return 0;

	return i;
}
/**
 * <combo{:combo{:combo:{..}}}>,<{ script }>
 **/
void itemdb_read_combos() {
	uint32 lines = 0, count = 0;
	char line[1024];
	char filepath[256];
	FILE* fp;

	sprintf(filepath, "%s/%s", map->db_path, DBPATH"item_combo_db.txt");

	if ((fp = fopen(filepath, "r")) == NULL) {
		ShowError("itemdb_read_combos: File not found \"%s\".\n", filepath);
		return;
	}

	// process rows one by one
	while(fgets(line, sizeof(line), fp)) {
		char *str[2], *p;

		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));

		p = line;
		p = trim(p);
		if (*p == '\0')
			continue;// empty line

		if (!strchr(p,',')) {
			/* is there even a single column? */
			ShowError("itemdb_read_combos: Insufficient columns in line %d of \"%s\", skipping.\n", lines, filepath);
			continue;
		}

		str[0] = p;
		p = strchr(p,',');
		*p = '\0';
		p++;

		str[1] = p;
		p = strchr(p,',');
		p++;

		if (str[1][0] != '{') {
			ShowError("itemdb_read_combos(#1): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, filepath);
			continue;
		}

		/* no ending key anywhere (missing \}\) */
		if ( str[1][strlen(str[1])-1] != '}' ) {
			ShowError("itemdb_read_combos(#2): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, filepath);
			continue;
		} else {
			int items[MAX_ITEMS_PER_COMBO];
			int v = 0, retcount = 0;
			struct item_combo *combo = NULL;

			if((retcount = itemdb->combo_split_atoi(str[0], items)) < 2) {
				ShowError("itemdb_read_combos: line %d of \"%s\" doesn't have enough items to make for a combo (min:2), skipping.\n", lines, filepath);
				continue;
			}

			/* validate */
			for(v = 0; v < retcount; v++) {
				if( !itemdb->exists(items[v]) ) {
					ShowError("itemdb_read_combos: line %d of \"%s\" contains unknown item ID %d, skipping.\n", lines, filepath,items[v]);
					break;
				}
			}
			/* failed at some item */
			if( v < retcount )
				continue;

			RECREATE(itemdb->combos, struct item_combo*, ++itemdb->combo_count);

			CREATE(combo, struct item_combo, 1);

			combo->count = retcount;
			combo->script = script->parse(str[1], filepath, lines, 0, NULL);
			combo->id = itemdb->combo_count - 1;
			/* populate ->nameid field */
			for( v = 0; v < retcount; v++ ) {
				combo->nameid[v] = items[v];
			}

			itemdb->combos[itemdb->combo_count - 1] = combo;

			/* populate the items to refer to this combo */
			for( v = 0; v < retcount; v++ ) {
				struct item_data * it;
				int index;

				it = itemdb->exists(items[v]);
				index = it->combos_count;
				RECREATE(it->combos, struct item_combo*, ++it->combos_count);
				it->combos[index] = combo;
			}
		}
		count++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%"PRIu32""CL_RESET"' entries in '"CL_WHITE"item_combo_db"CL_RESET"'.\n", count);

	return;
}

/*======================================
 * Applies gender restrictions according to settings. [Skotlex]
 *======================================*/
int itemdb_gendercheck(struct item_data *id)
{
	if (id->nameid == WEDDING_RING_M) //Grom Ring
		return 1;
	if (id->nameid == WEDDING_RING_F) //Bride Ring
		return 0;
	if (id->look == W_MUSICAL && id->type == IT_WEAPON) //Musical instruments are always male-only
		return 1;
	if (id->look == W_WHIP && id->type == IT_WEAPON) //Whips are always female-only
		return 0;

	return (battle_config.ignore_items_gender) ? 2 : id->sex;
}

/**
 * Validates an item DB entry and inserts it into the database.
 * This function is called after preparing the item entry data, and it takes
 * care of inserting it and cleaning up any remainders of the previous one.
 *
 * @param *entry  Pointer to the new item_data entry. Ownership is NOT taken,
 *                but the content is modified to reflect the validation.
 * @param n       Ordinal number of the entry, to be displayed in case of
 *                validation errors.
 * @param *source Source of the entry (table or file name), to be displayed in
 *                case of validation errors.
 * @return Nameid of the validated entry, or 0 in case of failure.
 *
 * Note: This is safe to call if the new entry is a copy of the old one (i.e.
 * item_db2 inheritance), as it will make sure not to free any scripts still in
 * use in the new entry.
 */
int itemdb_validate_entry(struct item_data *entry, int n, const char *source) {
	struct item_data *item;

	if( entry->nameid <= 0 || entry->nameid >= MAX_ITEMDB ) {
		ShowWarning("itemdb_validate_entry: Invalid item ID %d in entry %d of '%s', allowed values 0 < ID < %d (MAX_ITEMDB), skipping.\n",
				entry->nameid, n, source, MAX_ITEMDB);
		if (entry->script) {
			script->free_code(entry->script);
			entry->script = NULL;
		}
		if (entry->equip_script) {
			script->free_code(entry->equip_script);
			entry->equip_script = NULL;
		}
		if (entry->unequip_script) {
			script->free_code(entry->unequip_script);
			entry->unequip_script = NULL;
		}
		return 0;
	}

	if( entry->type < 0 || entry->type == IT_UNKNOWN || entry->type == IT_UNKNOWN2
	 || (entry->type > IT_DELAYCONSUME && entry->type < IT_CASH ) || entry->type >= IT_MAX
	) {
		// catch invalid item types
		ShowWarning("itemdb_validate_entry: Invalid item type %d for item %d in '%s'. IT_ETC will be used.\n",
		            entry->type, entry->nameid, source);
		entry->type = IT_ETC;
	} else if( entry->type == IT_DELAYCONSUME ) {
		//Items that are consumed only after target confirmation
		entry->type = IT_USABLE;
		entry->flag.delay_consume = 1;
	}

	//When a particular price is not given, we should base it off the other one
	//(it is important to make a distinction between 'no price' and 0z)
	if( entry->value_buy < 0 && entry->value_sell < 0 ) {
		entry->value_buy = entry->value_sell = 0;
	} else if( entry->value_buy < 0 ) {
		entry->value_buy = entry->value_sell * 2;
	} else if( entry->value_sell < 0 ) {
		entry->value_sell = entry->value_buy / 2;
	}
	if( entry->value_buy/124. < entry->value_sell/75. ) {
		ShowWarning("itemdb_validate_entry: Buying/Selling [%d/%d] price of item %d (%s) in '%s' "
		            "allows Zeny making exploit through buying/selling at discounted/overcharged prices!\n",
		            entry->value_buy, entry->value_sell, entry->nameid, entry->jname, source);
	}

	if( entry->slot > MAX_SLOTS ) {
		ShowWarning("itemdb_validate_entry: Item %d (%s) in '%s' specifies %d slots, but the server only supports up to %d. Using %d slots.\n",
		            entry->nameid, entry->jname, source, entry->slot, MAX_SLOTS, MAX_SLOTS);
		entry->slot = MAX_SLOTS;
	}

	if (!entry->equip && itemdb->isequip2(entry)) {
		ShowWarning("itemdb_validate_entry: Item %d (%s) in '%s' is an equipment with no equip-field! Making it an etc item.\n",
		            entry->nameid, entry->jname, source);
		entry->type = IT_ETC;
	}

	if (entry->flag.trade_restriction > ITR_ALL) {
		ShowWarning("itemdb_validate_entry: Invalid trade restriction flag 0x%x for item %d (%s) in '%s', defaulting to none.\n",
		            entry->flag.trade_restriction, entry->nameid, entry->jname, source);
		entry->flag.trade_restriction = ITR_NONE;
	}

	if (entry->gm_lv_trade_override < 0 || entry->gm_lv_trade_override > 100) {
		ShowWarning("itemdb_validate_entry: Invalid trade-override GM level %d for item %d (%s) in '%s', defaulting to none.\n",
		            entry->gm_lv_trade_override, entry->nameid, entry->jname, source);
		entry->gm_lv_trade_override = 0;
	}
	if (entry->gm_lv_trade_override == 0) {
		// Default value if none or an ivalid one was specified
		entry->gm_lv_trade_override = 100;
	}

	if (entry->item_usage.flag > INR_ALL) {
		ShowWarning("itemdb_validate_entry: Invalid nouse flag 0x%x for item %d (%s) in '%s', defaulting to none.\n",
		            entry->item_usage.flag, entry->nameid, entry->jname, source);
		entry->item_usage.flag = INR_NONE;
	}

	if (entry->item_usage.override > 100) {
		ShowWarning("itemdb_validate_entry: Invalid nouse-override GM level %d for item %d (%s) in '%s', defaulting to none.\n",
		            entry->item_usage.override, entry->nameid, entry->jname, source);
		entry->item_usage.override = 0;
	}
	if (entry->item_usage.override == 0) {
		// Default value if none or an ivalid one was specified
		entry->item_usage.override = 100;
	}

	if (entry->stack.amount > 0 && !itemdb->isstackable2(entry)) {
		ShowWarning("itemdb_validate_entry: Item %d (%s) of type %d is not stackable, ignoring stack settings in '%s'.\n",
		            entry->nameid, entry->jname, entry->type, source);
		memset(&entry->stack, '\0', sizeof(entry->stack));
	}

	entry->wlv = cap_value(entry->wlv, REFINE_TYPE_ARMOR, REFINE_TYPE_MAX);

	if( !entry->elvmax )
		entry->elvmax = MAX_LEVEL;
	else if( entry->elvmax < entry->elv )
		entry->elvmax = entry->elv;

	if( entry->type != IT_ARMOR && entry->type != IT_WEAPON && !entry->flag.no_refine )
		entry->flag.no_refine = 1;

	if (entry->flag.available != 1) {
		entry->flag.available = 1;
		entry->view_id = 0;
	}

	entry->sex = itemdb->gendercheck(entry); //Apply gender filtering.

	// Validated. Finally insert it
	item = itemdb->load(entry->nameid);

	if (item->script && item->script != entry->script) { // Don't free if it's inheriting the same script
		script->free_code(item->script);
		item->script = NULL;
	}
	if (item->equip_script && item->equip_script != entry->equip_script) { // Don't free if it's inheriting the same script
		script->free_code(item->equip_script);
		item->equip_script = NULL;
	}
	if (item->unequip_script && item->unequip_script != entry->unequip_script) { // Don't free if it's inheriting the same script
		script->free_code(item->unequip_script);
		item->unequip_script = NULL;
	}

	*item = *entry;
	return item->nameid;
}

void itemdb_readdb_additional_fields(int itemid, config_setting_t *it, int n, const char *source)
{
    // do nothing. plugins can do own work
}

/**
 * Processes one itemdb entry from the libconfig backend, loading and inserting
 * it into the item database.
 *
 * @param *it     Libconfig setting entry. It is expected to be valid and it
 *                won't be freed (it is care of the caller to do so if
 *                necessary)
 * @param n       Ordinal number of the entry, to be displayed in case of
 *                validation errors.
 * @param *source Source of the entry (file name), to be displayed in case of
 *                validation errors.
 * @return Nameid of the validated entry, or 0 in case of failure.
 */
int itemdb_readdb_libconfig_sub(config_setting_t *it, int n, const char *source) {
	struct item_data id = { 0 };
	config_setting_t *t = NULL;
	const char *str = NULL;
	int i32 = 0;
	bool inherit = false;

	/*
	 * // Mandatory fields
	 * Id: ID
	 * AegisName: "Aegis_Name"
	 * Name: "Item Name"
	 * // Optional fields
	 * Type: Item Type
	 * Buy: Buy Price
	 * Sell: Sell Price
	 * Weight: Item Weight
	 * Atk: Attack
	 * Matk: Attack
	 * Def: Defense
	 * Range: Attack Range
	 * Slots: Slots
	 * Job: Job mask
	 * Upper: Upper mask
	 * Gender: Gender
	 * Loc: Equip location
	 * WeaponLv: Weapon Level
	 * EquipLv: Equip required level or [min, max]
	 * Refine: Refineable
	 * View: View ID
	 * BindOnEquip: (true or false)
	 * BuyingStore: (true or false)
	 * Delay: Delay to use item
	 * ForceSerial: (true or false)
	 * Trade: {
	 *   override: Group to override
	 *   nodrop: (true or false)
	 *   notrade: (true or false)
	 *   partneroverride: (true or false)
	 *   noselltonpc: (true or false)
	 *   nocart: (true or false)
	 *   nostorage: (true or false)
	 *   nogstorage: (true or false)
	 *   nomail: (true or false)
	 *   noauction: (true or false)
	 * }
	 * Nouse: {
	 *   override: Group to override
	 *   sitting: (true or false)
	 * }
	 * Stack: [Stackable Amount, Stack Type]
	 * Sprite: SpriteID
	 * Script: <"
	 *     Script
	 *     (it can be multi-line)
	 * ">
	 * OnEquipScript: <" OnEquip Script ">
	 * OnUnequipScript: <" OnUnequip Script ">
	 * Inherit: inherit or override
	 */
	if( !itemdb->lookup_const(it, "Id", &i32) ) {
		ShowWarning("itemdb_readdb_libconfig_sub: Invalid or missing id in \"%s\", entry #%d, skipping.\n", source, n);
		return 0;
	}
	id.nameid = (uint16)i32;

	if( (t = libconfig->setting_get_member(it, "Inherit")) && (inherit = libconfig->setting_get_bool(t)) ) {
		if( !itemdb->exists(id.nameid) ) {
			ShowWarning("itemdb_readdb_libconfig_sub: Trying to inherit nonexistent item %d, default values will be used instead.\n", id.nameid);
			inherit = false;
		} else {
			// Use old entry as default
			struct item_data *old_entry = itemdb->load(id.nameid);
			memcpy(&id, old_entry, sizeof(struct item_data));
		}
	}

	if( !libconfig->setting_lookup_string(it, "AegisName", &str) || !*str ) {
		if( !inherit ) {
			ShowWarning("itemdb_readdb_libconfig_sub: Missing AegisName in item %d of \"%s\", skipping.\n", id.nameid, source);
			return 0;
		}
	} else {
		safestrncpy(id.name, str, sizeof(id.name));
	}

	if( !libconfig->setting_lookup_string(it, "Name", &str) || !*str ) {
		if( !inherit ) {
			ShowWarning("itemdb_readdb_libconfig_sub: Missing Name in item %d of \"%s\", skipping.\n", id.nameid, source);
			return 0;
		}
	} else {
		safestrncpy(id.jname, str, sizeof(id.jname));
	}

	if( itemdb->lookup_const(it, "Type", &i32) )
		id.type = i32;
	else if( !inherit )
		id.type = IT_ETC;

	if( itemdb->lookup_const(it, "Buy", &i32) )
		id.value_buy = i32;
	else if( !inherit )
		id.value_buy = -1;
	if( itemdb->lookup_const(it, "Sell", &i32) )
		id.value_sell = i32;
	else if( !inherit )
		id.value_sell = -1;

	if( itemdb->lookup_const(it, "Weight", &i32) && i32 >= 0 )
		id.weight = i32;

	if( itemdb->lookup_const(it, "Atk", &i32) && i32 >= 0 )
		id.atk = i32;

	if( itemdb->lookup_const(it, "Matk", &i32) && i32 >= 0 )
		id.matk = i32;

	if( itemdb->lookup_const(it, "Def", &i32) && i32 >= 0 )
		id.def = i32;

	if( itemdb->lookup_const(it, "Range", &i32) && i32 >= 0 )
		id.range = i32;

	if( itemdb->lookup_const(it, "Slots", &i32) && i32 >= 0 )
		id.slot = i32;

	if( itemdb->lookup_const(it, "Job", &i32) ) // This is an unsigned value, do not check for >= 0
		itemdb->jobid2mapid(id.class_base, (unsigned int)i32);
	else if( !inherit )
		itemdb->jobid2mapid(id.class_base, UINT_MAX);

	if( itemdb->lookup_const(it, "Upper", &i32) && i32 >= 0 )
		id.class_upper = (unsigned int)i32;
	else if( !inherit )
		id.class_upper = ITEMUPPER_ALL;

	if( itemdb->lookup_const(it, "Gender", &i32) && i32 >= 0 )
		id.sex = i32;
	else if( !inherit )
		id.sex = 2;

	if( itemdb->lookup_const(it, "Loc", &i32) && i32 >= 0 )
		id.equip = i32;

	if( itemdb->lookup_const(it, "WeaponLv", &i32) && i32 >= 0 )
		id.wlv = i32;

	if( (t = libconfig->setting_get_member(it, "EquipLv")) ) {
		if( config_setting_is_aggregate(t) ) {
			if( libconfig->setting_length(t) >= 2 )
				id.elvmax = libconfig->setting_get_int_elem(t, 1);
			if( libconfig->setting_length(t) >= 1 )
				id.elv = libconfig->setting_get_int_elem(t, 0);
		} else {
			id.elv = libconfig->setting_get_int(t);
		}
	}

	if( (t = libconfig->setting_get_member(it, "Refine")) )
		id.flag.no_refine = libconfig->setting_get_bool(t) ? 0 : 1;

	if( itemdb->lookup_const(it, "View", &i32) && i32 >= 0 )
		id.look = i32;

	if( (t = libconfig->setting_get_member(it, "BindOnEquip")) )
		id.flag.bindonequip = libconfig->setting_get_bool(t) ? 1 : 0;

	if( (t = libconfig->setting_get_member(it, "ForceSerial")) )
		id.flag.force_serial = libconfig->setting_get_bool(t) ? 1 : 0;

	if ( (t = libconfig->setting_get_member(it, "BuyingStore")) )
		id.flag.buyingstore = libconfig->setting_get_bool(t) ? 1 : 0;

	if ((t = libconfig->setting_get_member(it, "KeepAfterUse")))
		id.flag.keepafteruse = libconfig->setting_get_bool(t) ? 1 : 0;

	if (itemdb->lookup_const(it, "Delay", &i32) && i32 >= 0)
		id.delay = i32;

	if ( (t = libconfig->setting_get_member(it, "Trade")) ) {
		if (config_setting_is_group(t)) {
			config_setting_t *tt = NULL;

			if ((tt = libconfig->setting_get_member(t, "override"))) {
				id.gm_lv_trade_override = libconfig->setting_get_int(tt);
			}

			if ((tt = libconfig->setting_get_member(t, "nodrop"))) {
				id.flag.trade_restriction &= ~ITR_NODROP;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NODROP;
			}

			if ((tt = libconfig->setting_get_member(t, "notrade"))) {
				id.flag.trade_restriction &= ~ITR_NOTRADE;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOTRADE;
			}

			if ((tt = libconfig->setting_get_member(t, "partneroverride"))) {
				id.flag.trade_restriction &= ~ITR_PARTNEROVERRIDE;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_PARTNEROVERRIDE;
			}

			if ((tt = libconfig->setting_get_member(t, "noselltonpc"))) {
				id.flag.trade_restriction &= ~ITR_NOSELLTONPC;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOSELLTONPC;
			}

			if ((tt = libconfig->setting_get_member(t, "nocart"))) {
				id.flag.trade_restriction &= ~ITR_NOCART;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOCART;
			}

			if ((tt = libconfig->setting_get_member(t, "nostorage"))) {
				id.flag.trade_restriction &= ~ITR_NOSTORAGE;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOSTORAGE;
			}

			if ((tt = libconfig->setting_get_member(t, "nogstorage"))) {
				id.flag.trade_restriction &= ~ITR_NOGSTORAGE;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOGSTORAGE;
			}

			if ((tt = libconfig->setting_get_member(t, "nomail"))) {
				id.flag.trade_restriction &= ~ITR_NOMAIL;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOMAIL;
			}

			if ((tt = libconfig->setting_get_member(t, "noauction"))) {
				id.flag.trade_restriction &= ~ITR_NOAUCTION;
				if (libconfig->setting_get_bool(tt))
					id.flag.trade_restriction |= ITR_NOAUCTION;
			}
		} else { // Fallback to int if it's not a group
			id.flag.trade_restriction = libconfig->setting_get_int(t);
		}
	}

	if ((t = libconfig->setting_get_member(it, "Nouse"))) {
		if (config_setting_is_group(t)) {
			config_setting_t *nt = NULL;

			if ((nt = libconfig->setting_get_member(t, "override"))) {
				id.item_usage.override = libconfig->setting_get_int(nt);
			}

			if ((nt = libconfig->setting_get_member(t, "sitting"))) {
				id.item_usage.flag &= ~INR_SITTING;
				if (libconfig->setting_get_bool(nt))
					id.item_usage.flag |= INR_SITTING;
			}

		} else { // Fallback to int if it's not a group
			id.item_usage.flag = libconfig->setting_get_int(t);
		}
	}

	if ((t = libconfig->setting_get_member(it, "Stack"))) {
		if (config_setting_is_aggregate(t) && libconfig->setting_length(t) >= 1) {
			int stack_flag = libconfig->setting_get_int_elem(t, 1);
			int stack_amount = libconfig->setting_get_int_elem(t, 0);
			if (stack_amount >= 0) {
				id.stack.amount = cap_value(stack_amount, 0, USHRT_MAX);
				id.stack.inventory = (stack_flag&1)!=0;
				id.stack.cart = (stack_flag&2)!=0;
				id.stack.storage = (stack_flag&4)!=0;
				id.stack.guildstorage = (stack_flag&8)!=0;
			}
		}
	}

	if (itemdb->lookup_const(it, "Sprite", &i32) && i32 >= 0) {
		id.flag.available = 1;
		id.view_id = i32;
	}

	if( libconfig->setting_lookup_string(it, "Script", &str) )
		id.script = *str ? script->parse(str, source, -id.nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;

	if( libconfig->setting_lookup_string(it, "OnEquipScript", &str) )
		id.equip_script = *str ? script->parse(str, source, -id.nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;

	if( libconfig->setting_lookup_string(it, "OnUnequipScript", &str) )
		id.unequip_script = *str ? script->parse(str, source, -id.nameid, SCRIPT_IGNORE_EXTERNAL_BRACKETS, NULL) : NULL;

	return itemdb->validate_entry(&id, n, source);
}

bool itemdb_lookup_const(const config_setting_t *it, const char *name, int *value)
{
	if (libconfig->setting_lookup_int(it, name, value))
	{
		return true;
	}
	else
	{
		const char *str = NULL;
		if (libconfig->setting_lookup_string(it, name, &str))
		{
			if (*str && script->get_constant(str, value))
				return true;
		}
	}
	return false;
}

/**
 * Reads from a libconfig-formatted itemdb file and inserts the found entries into the
 * item database, overwriting duplicate ones (i.e. item_db2 overriding item_db.)
 *
 * @param *filename File name, relative to the database path.
 * @return The number of found entries.
 */
int itemdb_readdb_libconfig(const char *filename) {
	bool duplicate[MAX_ITEMDB];
	config_t item_db_conf;
	config_setting_t *itdb, *it;
	char filepath[256];
	int i = 0, count = 0;

	sprintf(filepath, "%s/%s", map->db_path, filename);
	memset(&duplicate,0,sizeof(duplicate));
	if( libconfig->read_file(&item_db_conf, filepath) || !(itdb = libconfig->setting_get_member(item_db_conf.root, "item_db")) ) {
		ShowError("can't read %s\n", filepath);
		return 0;
	}

	while( (it = libconfig->setting_get_elem(itdb,i++)) ) {
		int nameid = itemdb->readdb_libconfig_sub(it, i-1, filename);

		if( !nameid )
			continue;

		itemdb->readdb_additional_fields(nameid, it, i - 1, filename);
		count++;

		if( duplicate[nameid] ) {
			ShowWarning("itemdb_readdb:%s: duplicate entry of ID #%d (%s/%s)\n",
					filename, nameid, itemdb_name(nameid), itemdb_jname(nameid));
		} else
			duplicate[nameid] = true;
	}
	libconfig->destroy(&item_db_conf);
	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' entries in '"CL_WHITE"%s"CL_RESET"'.\n", count, filename);

	return count;
}

/*==========================================
* Unique item ID function
* Only one operation by once
*------------------------------------------*/
uint64 itemdb_unique_id(struct map_session_data *sd) {

	return ((uint64)sd->status.char_id << 32) | sd->status.uniqueitem_counter++;
}

/**
 * Reads all item-related databases.
 */
void itemdb_read(bool minimal) {
	int i;
	DBData prev;

	const char *filename[] = {
		DBPATH"item_db.conf",
		"item_db2.conf",
	};
	for (i = 0; i < ARRAYLENGTH(filename); i++)
		itemdb->readdb_libconfig(filename[i]);

	for( i = 0; i < ARRAYLENGTH(itemdb->array); ++i ) {
		if( itemdb->array[i] ) {
			if( itemdb->names->put(itemdb->names,DB->str2key(itemdb->array[i]->name),DB->ptr2data(itemdb->array[i]),&prev) ) {
				struct item_data *data = DB->data2ptr(&prev);
				ShowError("itemdb_read: duplicate AegisName '%s' in item ID %d and %d\n",itemdb->array[i]->name,itemdb->array[i]->nameid,data->nameid);
			}
		}
	}

	if (minimal)
		return;

	itemdb->read_combos();
	itemdb->read_groups();
	itemdb->read_chains();
	itemdb->read_packages();

}

/**
 * retrieves item_combo data by combo id
 **/
struct item_combo * itemdb_id2combo( unsigned short id ) {
	if( id > itemdb->combo_count )
		return NULL;
	return itemdb->combos[id];
}

/**
 * check is item have usable type
 **/
bool itemdb_is_item_usable(struct item_data *item)
{
	return item->type == IT_HEALING || item->type == IT_USABLE || item->type == IT_CASH;
}

/*==========================================
 * Initialize / Finalize
 *------------------------------------------*/

/// Destroys the item_data.
void destroy_item_data(struct item_data* self, int free_self)
{
	if( self == NULL )
		return;
	// free scripts
	if( self->script )
		script->free_code(self->script);
	if( self->equip_script )
		script->free_code(self->equip_script);
	if( self->unequip_script )
		script->free_code(self->unequip_script);
	if( self->combos )
		aFree(self->combos);
	HPM->data_store_destroy(&self->hdata);
#if defined(DEBUG)
	// trash item
	memset(self, 0xDD, sizeof(struct item_data));
#endif
	// free self
	if( free_self )
		aFree(self);
}

/**
 * @see DBApply
 */
int itemdb_final_sub(DBKey key, DBData *data, va_list ap)
{
	struct item_data *id = DB->data2ptr(data);

	if( id != &itemdb->dummy )
		itemdb->destroy_item_data(id, 1);

	return 0;
}
void itemdb_clear(bool total) {
	int i;
	// clear the previous itemdb data
	for( i = 0; i < ARRAYLENGTH(itemdb->array); ++i ) {
		if( itemdb->array[i] )
			itemdb->destroy_item_data(itemdb->array[i], 1);
	}

	if( itemdb->groups )
	{
		for( i = 0; i < itemdb->group_count; i++ ) {
			if( itemdb->groups[i].nameid )
				aFree(itemdb->groups[i].nameid);
		}
		aFree(itemdb->groups);
	}

	itemdb->groups = NULL;
	itemdb->group_count = 0;

	if (itemdb->chains) {
		for (i = 0; i < itemdb->chain_count; i++) {
			if (itemdb->chains[i].items)
				aFree(itemdb->chains[i].items);
		}
		aFree(itemdb->chains);
	}

	itemdb->chains = NULL;
	itemdb->chain_count = 0;

	if (itemdb->packages) {
		for (i = 0; i < itemdb->package_count; i++) {
			if (itemdb->packages[i].random_groups) {
				int j;
				for (j = 0; j < itemdb->packages[i].random_qty; j++)
					aFree(itemdb->packages[i].random_groups[j].random_list);
				aFree(itemdb->packages[i].random_groups);
			}
			if( itemdb->packages[i].must_items )
				aFree(itemdb->packages[i].must_items);
		}
		aFree(itemdb->packages);
		itemdb->packages = NULL;
	}
	itemdb->package_count = 0;

	if (itemdb->combos) {
		for (i = 0; i < itemdb->combo_count; i++) {
			if (itemdb->combos[i]->script) // Check if script was loaded
				script->free_code(itemdb->combos[i]->script);
			aFree(itemdb->combos[i]);
		}
		aFree(itemdb->combos);
	}

	itemdb->combos = NULL;
	itemdb->combo_count = 0;

	if (total)
		return;

	itemdb->other->clear(itemdb->other, itemdb->final_sub);

	memset(itemdb->array, 0, sizeof(itemdb->array));

	db_clear(itemdb->names);
}

void itemdb_reload(void) {
	struct s_mapiterator* iter;
	struct map_session_data* sd;

	int i,d,k;

	itemdb->clear(false);

	// read new data
	itemdb->read(false);

	//Epoque's awesome @reloaditemdb fix - thanks! [Ind]
	//- Fixes the need of a @reloadmobdb after a @reloaditemdb to re-link monster drop data
	for( i = 0; i < MAX_MOB_DB; i++ ) {
		struct mob_db *entry;
		if( !((i < 1324 || i > 1363) && (i < 1938 || i > 1946)) )
			continue;
		entry = mob->db(i);
		for(d = 0; d < MAX_MOB_DROP; d++) {
			struct item_data *id;
			if( !entry->dropitem[d].nameid )
				continue;
			id = itemdb->search(entry->dropitem[d].nameid);

			for (k = 0; k < MAX_SEARCH; k++) {
				if (id->mob[k].chance <= entry->dropitem[d].p)
					break;
			}

			if (k == MAX_SEARCH)
				continue;

			if (id->mob[k].id != i && k != MAX_SEARCH - 1)
				memmove(&id->mob[k+1], &id->mob[k], (MAX_SEARCH-k-1)*sizeof(id->mob[0]));
			id->mob[k].chance = entry->dropitem[d].p;
			id->mob[k].id = i;
		}
	}

	// readjust itemdb pointer cache for each player
	iter = mapit_geteachpc();
	for( sd = (struct map_session_data*)mapit->first(iter); mapit->exists(iter); sd = (struct map_session_data*)mapit->next(iter) ) {
		memset(sd->item_delay, 0, sizeof(sd->item_delay));  // reset item delays
		pc->setinventorydata(sd);
		if( battle_config.item_check )
			sd->state.itemcheck = 1;
		/* clear combo bonuses */
		if( sd->combo_count ) {
			aFree(sd->combos);
			sd->combos = NULL;
			sd->combo_count = 0;
			if( pc->load_combo(sd) > 0 )
				status_calc_pc(sd,SCO_FORCE);
		}
		pc->checkitem(sd);
	}
	mapit->free(iter);
}
void itemdb_name_constants(void) {
	DBIterator *iter = db_iterator(itemdb->names);
	struct item_data *data;

#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = "Item Database (Likely an invalid or conflicting AegisName)";
#endif // ENABLE_CASE_CHECK
	for( data = dbi_first(iter); dbi_exists(iter); data = dbi_next(iter) )
		script->set_constant2(data->name,data->nameid,0);
#ifdef ENABLE_CASE_CHECK
	script->parser_current_file = NULL;
#endif // ENABLE_CASE_CHECK

	dbi_destroy(iter);
}
void do_final_itemdb(void) {
	itemdb->clear(true);

	itemdb->other->destroy(itemdb->other, itemdb->final_sub);
	itemdb->destroy_item_data(&itemdb->dummy, 0);
	db_destroy(itemdb->names);
}

void do_init_itemdb(bool minimal) {
	memset(itemdb->array, 0, sizeof(itemdb->array));
	itemdb->other = idb_alloc(DB_OPT_BASE);
	itemdb->names = strdb_alloc(DB_OPT_BASE,ITEM_NAME_LENGTH);
	itemdb->create_dummy_data(); //Dummy data item.
	itemdb->read(minimal);

	if (minimal)
		return;

	clif->cashshop_load();

	/** it failed? we disable it **/
	if( !clif->parse_roulette_db() )
		battle_config.feature_roulette = 0;
}
void itemdb_defaults(void) {
	itemdb = &itemdb_s;

	itemdb->init = do_init_itemdb;
	itemdb->final = do_final_itemdb;
	itemdb->reload = itemdb_reload;
	itemdb->name_constants = itemdb_name_constants;
	/* */
	itemdb->groups = NULL;
	itemdb->group_count = 0;
	/* */
	itemdb->chains = NULL;
	itemdb->chain_count = 0;
	/* */
	itemdb->packages = NULL;
	itemdb->package_count = 0;
	/* */
	itemdb->combos = NULL;
	itemdb->combo_count = 0;
	/* */
	itemdb->names = NULL;
	/* */
	/* itemdb->array is cleared on itemdb->init() */
	itemdb->other = NULL;
	memset(&itemdb->dummy, 0, sizeof(struct item_data));
	/* */
	itemdb->read_groups = itemdb_read_groups;
	itemdb->read_chains = itemdb_read_chains;
	itemdb->read_packages = itemdb_read_packages;
	/* */
	itemdb->write_cached_packages = itemdb_write_cached_packages;
	itemdb->read_cached_packages = itemdb_read_cached_packages;
	/* */
	itemdb->name2id = itemdb_name2id;
	itemdb->search_name = itemdb_searchname;
	itemdb->search_name_array = itemdb_searchname_array;
	itemdb->load = itemdb_load;
	itemdb->search = itemdb_search;
	itemdb->exists = itemdb_exists;
	itemdb->in_group = itemdb_in_group;
	itemdb->group_item = itemdb_searchrandomid;
	itemdb->chain_item = itemdb_chain_item;
	itemdb->package_item = itemdb_package_item;
	itemdb->searchname_sub = itemdb_searchname_sub;
	itemdb->searchname_array_sub = itemdb_searchname_array_sub;
	itemdb->searchrandomid = itemdb_searchrandomid;
	itemdb->typename = itemdb_typename;
	itemdb->jobid2mapid = itemdb_jobid2mapid;
	itemdb->create_dummy_data = create_dummy_data;
	itemdb->create_item_data = create_item_data;
	itemdb->isequip = itemdb_isequip;
	itemdb->isequip2 = itemdb_isequip2;
	itemdb->isstackable = itemdb_isstackable;
	itemdb->isstackable2 = itemdb_isstackable2;
	itemdb->isdropable_sub = itemdb_isdropable_sub;
	itemdb->cantrade_sub = itemdb_cantrade_sub;
	itemdb->canpartnertrade_sub = itemdb_canpartnertrade_sub;
	itemdb->cansell_sub = itemdb_cansell_sub;
	itemdb->cancartstore_sub = itemdb_cancartstore_sub;
	itemdb->canstore_sub = itemdb_canstore_sub;
	itemdb->canguildstore_sub = itemdb_canguildstore_sub;
	itemdb->canmail_sub = itemdb_canmail_sub;
	itemdb->canauction_sub = itemdb_canauction_sub;
	itemdb->isrestricted = itemdb_isrestricted;
	itemdb->isidentified = itemdb_isidentified;
	itemdb->isidentified2 = itemdb_isidentified2;
	itemdb->combo_split_atoi = itemdb_combo_split_atoi;
	itemdb->read_combos = itemdb_read_combos;
	itemdb->gendercheck = itemdb_gendercheck;
	itemdb->validate_entry = itemdb_validate_entry;
	itemdb->readdb_additional_fields = itemdb_readdb_additional_fields;
	itemdb->readdb_libconfig_sub = itemdb_readdb_libconfig_sub;
	itemdb->readdb_libconfig = itemdb_readdb_libconfig;
	itemdb->unique_id = itemdb_unique_id;
	itemdb->read = itemdb_read;
	itemdb->destroy_item_data = destroy_item_data;
	itemdb->final_sub = itemdb_final_sub;
	itemdb->clear = itemdb_clear;
	itemdb->id2combo = itemdb_id2combo;
	itemdb->is_item_usable = itemdb_is_item_usable;
	itemdb->lookup_const = itemdb_lookup_const;
}
