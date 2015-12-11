#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common/HPMi.h"
#include "map/intif.h"
#include "map/pc.h"
#include "map/storage.h"
#include "common/HPMDataCheck.h" // should always be the last file included! (if you don't make it last, it'll intentionally break compile time)

HPExport struct hplugin_info pinfo = {
	"storagedelitem", // Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};


//Declaration of MakeDWord to allow the 
uint32 MakeDWord(uint16 word0, uint16 word1) {
	return
		( (uint32)(word0        ) )|
		( (uint32)(word1 << 0x10) );
}

/// Counts / deletes the current item given by idx.
/// Used by buildin_delitem_search
/// Relies on all input data being already fully valid.
void buildin_storage_delitem_delete(struct map_session_data* sd, int idx, int* amount, bool delete_items)
{
	int delamount;
	struct item* inv = &sd->status.storage.items[idx];

	delamount = ( amount[0] < inv->amount ) ? amount[0] : inv->amount;

	if( delete_items )
	{
		if( itemdb_type(inv->nameid) == IT_PETEGG && inv->card[0] == CARD0_PET )
		{// delete associated pet
			intif->delete_petdata(MakeDWord(inv->card[1], inv->card[2]));
		}
		storage->delitem(sd, idx, delamount);
	}

	amount[0]-= delamount;
}

/// Searches for item(s) and checks, if there is enough of them.
/// Used by delitem and delitem2
/// Relies on all input data being already fully valid.
/// @param exact_match will also match item attributes and cards, not just name id
/// @return true when all items could be deleted, false when there were not enough items to delete
bool buildin_storage_delitem_search(struct map_session_data* sd, struct item* it, bool exact_match)
{
	bool delete_items = false;
	int i, amount, size;
	struct item* inv;

	// prefer always non-equipped items
	it->equip = 0;

	// when searching for nameid only, prefer additionally
	if (!exact_match) {
		// non-refined items
		it->refine = 0;
		// card-less items
		memset(it->card, 0, sizeof(it->card));
	}

	size = MAX_STORAGE;
	inv = sd->status.storage.items;

	for (;;) {
		int important = 0;
		amount = it->amount;

		// 1st pass -- less important items / exact match
		for (i = 0; amount && i < size; i++) {
			
			struct item *itm = NULL;

			if (!&inv[i] || !(itm = &inv[i])->nameid || itm->nameid != it->nameid) {
				// wrong/invalid item
				continue;
			}

			if (itm->equip != it->equip || itm->refine != it->refine) {
				// not matching attributes
				important++;
				continue;
			}

			if (exact_match) {
				if (itm->identify != it->identify || itm->attribute != it->attribute || memcmp(itm->card, it->card, sizeof(itm->card))) {
					// not matching exact attributes
					continue;
				}
			} else {
				if (itemdb_type(itm->nameid) == IT_PETEGG) {
					if (itm->card[0] == CARD0_PET && intif->CheckForCharServer()) {
						// pet which cannot be deleted
						continue;
					}
				} else if (memcmp(itm->card, it->card, sizeof(itm->card))) {
					// named/carded item
					important++;
					continue;
				}
			}

			// count / delete item
			buildin_storage_delitem_delete(sd, i, &amount, delete_items);
		}

		// 2nd pass -- any matching item
		if (amount == 0 || important == 0) {
			// either everything was already consumed or no items were skipped
			;
		} else {
			for (i = 0; amount && i < size; i++) {
				struct item *itm = NULL;

				if (!&inv[i] || !(itm = &inv[i])->nameid || itm->nameid != it->nameid) {
					// wrong/invalid item
					continue;
				}

				if (itemdb_type(itm->nameid) == IT_PETEGG && itm->card[0] == CARD0_PET && intif->CheckForCharServer()) {
					// pet which cannot be deleted
					continue;
				}

				if (exact_match) {
					if (itm->refine != it->refine || itm->identify != it->identify || itm->attribute != it->attribute
					 || memcmp(itm->card, it->card, sizeof(itm->card))) {
						// not matching attributes
						continue;
					}
				}

				// count / delete item
				buildin_storage_delitem_delete(sd, i, &amount, delete_items);
			}
		}

		if (amount) {
			// not enough items
			return false;
		} else if(delete_items) {
			// we are done with the work
			return true;
		} else {
			// get rid of the items now
			delete_items = true;
		}
	}
}

/*==========================================
 * storagedelitem <item id>,<amount>{,<account id>}
 * storagedelitem "<item name>",<amount>{,<account id>}
 * Deletes items from the target/attached player.
 *==========================================*/
BUILDIN(storagedelitem) {
	TBL_PC *sd;
	struct item it;

	if (script_hasdata(st,4)) {
		int account_id = script_getnum(st,4);
		sd = map->id2sd(account_id); // <account id>
		if (sd == NULL) {
			ShowError("script:storagedelitem: player not found (AID=%d).\n", account_id);
			st->state = END;
			return false;
		}
	} else {
		sd = script->rid2sd(st);// attached player
		if (sd == NULL)
			return true;
	}

	memset(&it, 0, sizeof(it));
	if (script_isstringtype(st, 2)) {
		const char* item_name = script_getstr(st, 2);
		struct item_data* id = itemdb->search_name(item_name);
		if (id == NULL) {
			ShowError("script:storagedelitem: unknown item \"%s\".\n", item_name);
			st->state = END;
			return false;
		}
		it.nameid = id->nameid;// "<item name>"
	} else {
		it.nameid = script_getnum(st, 2);// <item id>
		if (!itemdb->exists(it.nameid)) {
			ShowError("script:storagedelitem: unknown item \"%d\".\n", it.nameid);
			st->state = END;
			return false;
		}
	}

	it.amount=script_getnum(st,3);

	if( it.amount <= 0 )
		return true;// nothing to do

	if( buildin_storage_delitem_search(sd, &it, false) )
	{// success
		return true;
	}

	ShowError("script:storagedelitem: failed to delete %d items (AID=%d item_id=%d).\n", it.amount, sd->status.account_id, it.nameid);
	st->state = END;
	clif->scriptclose(sd, st->oid);
	return false;
}

/* Server Startup */
HPExport void plugin_init (void) {
	addScriptCommand( "storagedelitem", "vi?", storagedelitem);
}