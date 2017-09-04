/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017 Hercules Dev Team
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

#include "rodex.h"

#include "map/battle.h"
#include "map/date.h"
#include "map/intif.h"
#include "map/itemdb.h"
#include "map/pc.h"

#include "common/nullpo.h"
#include "common/sql.h"
#include "common/memmgr.h"


// NOTE : These values are hardcoded into the client
// Cost of each Attached Item
#define ATTACHITEM_COST 2500
// Percent of Attached Zeny that will be paid as Tax
#define ATTACHZENY_TAX 2
// Maximun number of messages that can be sent in one day
#define DAILY_MAX_MAILS 100

struct rodex_interface rodex_s;
struct rodex_interface *rodex;

/// Checks if RoDEX System is enabled in the server
/// Returns true if it's enabled, false otherwise
bool rodex_isenabled(void)
{
	if (battle_config.feature_rodex == 1)
		return true;

	return false;
}

/// Checks and refreshes the user daily number of Stamps
/// @param sd : The player who's being checked
void rodex_refresh_stamps(struct map_session_data *sd)
{
	int today = date_get_date();
	nullpo_retv(sd);

	// Note : Weirdly, iRO starts this with maximum messages of the day and decrements
	//        but our clients starts this at 0 and increments
	if (sd->sc.data[SC_DAILYSENDMAILCNT] == NULL) {
		sc_start2(NULL, &sd->bl, SC_DAILYSENDMAILCNT, 100, today, 0, INFINITE_DURATION);
	} else {
		int sc_date = sd->sc.data[SC_DAILYSENDMAILCNT]->val1;
		if (sc_date != today) {
			sc_start2(NULL, &sd->bl, SC_DAILYSENDMAILCNT, 100, today, 0, INFINITE_DURATION);
		}
	}
}

/// Attaches an item to a message being written
/// @param sd : The player who's writting
/// @param idx : the inventory idx of the item
/// @param amount : Amount of the item to be attached
void rodex_add_item(struct map_session_data *sd, int16 idx, int16 amount)
{
	int i;
	bool is_stack = false;

	nullpo_retv(sd);

	if (idx < 0 || idx >= MAX_INVENTORY) {
		clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_FATAL_ERROR);
		return;
	}

	if (amount < 0 || amount > sd->status.inventory[idx].amount) {
		clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_FATAL_ERROR);
		return;
	}

	if (!pc_can_give_items(sd) || sd->status.inventory[idx].expire_time ||
		!itemdb_canmail(&sd->status.inventory[idx], pc_get_group_level(sd)) ||
		(sd->status.inventory[idx].bound && !pc_can_give_bound_items(sd))) {
		clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_NOT_TRADEABLE);
		return;
	}

	if (itemdb->isstackable(sd->status.inventory[idx].nameid) == 1) {
		for (i = 0; i < RODEX_MAX_ITEM; ++i) {
			if (sd->rodex.tmp.items[i].idx == idx) {
				if (sd->status.inventory[idx].nameid == sd->rodex.tmp.items[i].item.nameid &&
					sd->status.inventory[idx].unique_id == sd->rodex.tmp.items[i].item.unique_id) {
					is_stack = true;
					break;
				}
			}
		}

		if (i == RODEX_MAX_ITEM && sd->rodex.tmp.items_count < RODEX_MAX_ITEM) {
			ARR_FIND(0, RODEX_MAX_ITEM, i, sd->rodex.tmp.items[i].idx == 0);
		}
	} else if (sd->rodex.tmp.items_count < RODEX_MAX_ITEM) {
		ARR_FIND(0, RODEX_MAX_ITEM, i, sd->rodex.tmp.items[i].idx == 0);
	} else {
		i = RODEX_MAX_ITEM;
	}

	if (i == RODEX_MAX_ITEM) {
		clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_NO_SPACE);
		return;
	}

	if (sd->rodex.tmp.items[i].item.amount + amount > sd->status.inventory[idx].amount) {
		clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_FATAL_ERROR);
		return;
	}

	if (sd->rodex.tmp.weight + sd->inventory_data[idx]->weight * amount > RODEX_WEIGHT_LIMIT) {
		clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_FATAL_ERROR);
		return;
	}

	sd->rodex.tmp.items[i].idx = idx;
	sd->rodex.tmp.weight += sd->inventory_data[idx]->weight * amount;
	if (is_stack == false) {
		sd->rodex.tmp.items[i].item = sd->status.inventory[idx];
		sd->rodex.tmp.items[i].item.amount = amount;
		sd->rodex.tmp.items_count++;
	} else {
		sd->rodex.tmp.items[i].item.amount += amount;
	}
	sd->rodex.tmp.type |= MAIL_TYPE_ITEM;

	clif->rodex_add_item_result(sd, idx, amount, RODEX_ADD_ITEM_SUCCESS);
}

/// Removes an item attached to a message being writen
/// @param sd : The player who's writting the message
/// @param idx : The index of the item
/// @param amount : How much to remove
void rodex_remove_item(struct map_session_data *sd, int16 idx, int16 amount)
{
	int i;
	struct item *it;
	struct item_data *itd;

	nullpo_retv(sd);
	Assert_retv(idx >= 0 && idx < MAX_INVENTORY);

	for (i = 0; i < RODEX_MAX_ITEM; ++i) {
		if (sd->rodex.tmp.items[i].idx == idx)
			break;
	}

	if (i == RODEX_MAX_ITEM) {
		clif->rodex_remove_item_result(sd, idx, -1);
		return;
	}

	it = &sd->rodex.tmp.items[i].item;

	if (amount <= 0 || amount > it->amount) {
		clif->rodex_remove_item_result(sd, idx, -1);
		return;
	}

	itd = itemdb->search(it->nameid);

	if (amount == it->amount) {
		sd->rodex.tmp.weight -= itd->weight * amount;
		sd->rodex.tmp.items_count--;
		if (sd->rodex.tmp.items_count < 1) {
			sd->rodex.tmp.type &= ~MAIL_TYPE_ITEM;
		}
		memset(&sd->rodex.tmp.items[i], 0x0, sizeof(sd->rodex.tmp.items[0]));
		clif->rodex_remove_item_result(sd, idx, 0);
		return;
	}

	it->amount -= amount;
	sd->rodex.tmp.weight -= itd->weight * amount;

	clif->rodex_remove_item_result(sd, idx, it->amount);
}

/// Request if character with given name exists and returns information about him
/// @param sd : The player who's requesting
/// @param name : The name of the character to check
/// @param base_level : Reference to return the character base level, if he exists
/// @param char_id : Reference to return the character id, if he exists
/// @param class : Reference to return the character class id, if he exists
void rodex_check_player(struct map_session_data *sd, const char *name, int *base_level, int *char_id, short *class)
{
	intif->rodex_checkname(sd, name);
}

/// Sends a Mail to an character
/// @param sd : The player who's sending
/// @param receiver_name : The name of the character who's receiving the message
/// @param body : Mail message
/// @param title : Mail Title
/// @param zeny : Amount of zeny attached
/// Returns result code:
/// 	RODEX_SEND_MAIL_SUCCESS = 0,
///		RODEX_SEND_MAIL_FATAL_ERROR = 1,
///		RODEX_SEND_MAIL_COUNT_ERROR = 2,
///		RODEX_SEND_MAIL_ITEM_ERROR = 3,
///		RODEX_SEND_MAIL_RECEIVER_ERROR = 4
int rodex_send_mail(struct map_session_data *sd, const char *receiver_name, const char *body, const char *title, int64 zeny)
{
	int i;
	int64 total_zeny;

	nullpo_retr(RODEX_SEND_MAIL_FATAL_ERROR, sd);
	nullpo_retr(RODEX_SEND_MAIL_FATAL_ERROR, receiver_name);
	nullpo_retr(RODEX_SEND_MAIL_FATAL_ERROR, body);
	nullpo_retr(RODEX_SEND_MAIL_FATAL_ERROR, title);

	if (zeny < 0) {
		rodex->clean(sd, 1);
		return RODEX_SEND_MAIL_FATAL_ERROR;
	}

	total_zeny = zeny + sd->rodex.tmp.items_count * ATTACHITEM_COST + (2 * zeny)/100;

	if (strcmp(receiver_name, sd->rodex.tmp.receiver_name) != 0) {
		rodex->clean(sd, 1);
		return RODEX_SEND_MAIL_RECEIVER_ERROR;
	}

	if (total_zeny > sd->status.zeny || total_zeny < 0) {
		rodex->clean(sd, 1);
		return RODEX_SEND_MAIL_FATAL_ERROR;
	}

	rodex_refresh_stamps(sd);

	if (sd->sc.data[SC_DAILYSENDMAILCNT] != NULL) {
		if (sd->sc.data[SC_DAILYSENDMAILCNT]->val2 >= DAILY_MAX_MAILS) {
			rodex->clean(sd, 1);
			return RODEX_SEND_MAIL_COUNT_ERROR;
		}

		sc_start2(NULL, &sd->bl, SC_DAILYSENDMAILCNT, 100, sd->sc.data[SC_DAILYSENDMAILCNT]->val1, sd->sc.data[SC_DAILYSENDMAILCNT]->val2 + 1, INFINITE_DURATION);
	} else {
		sc_start2(NULL, &sd->bl, SC_DAILYSENDMAILCNT, 100, date_get_date(), 1, INFINITE_DURATION);
	}

	for (i = 0; i < RODEX_MAX_ITEM; i++) {
		int16 idx = sd->rodex.tmp.items[i].idx;
		int j;
		struct item *tmpItem = &sd->rodex.tmp.items[i].item;
		struct item *invItem = &sd->status.inventory[idx];

		if (tmpItem->nameid == 0)
			continue;

		if (tmpItem->nameid != invItem->nameid ||
		    tmpItem->unique_id != invItem->unique_id ||
		    tmpItem->refine != invItem->refine ||
		    tmpItem->attribute != invItem->attribute ||
		    tmpItem->expire_time != invItem->expire_time ||
		    tmpItem->bound != invItem->bound ||
		    tmpItem->amount > invItem->amount ||
		    tmpItem->amount < 1) {
			rodex->clean(sd, 1);
			return RODEX_SEND_MAIL_ITEM_ERROR;
		}
		for (j = 0; j < MAX_SLOTS; j++) {
			if (tmpItem->card[j] != invItem->card[j]) {
				rodex->clean(sd, 1);
				return RODEX_SEND_MAIL_ITEM_ERROR;
			}
		}
		for (j = 0; j < MAX_ITEM_OPTIONS; j++) {
			if (tmpItem->option[j].index != invItem->option[j].index ||
			    tmpItem->option[j].value != invItem->option[j].value ||
			    tmpItem->option[j].param != invItem->option[j].param) {
				rodex->clean(sd, 1);
				return RODEX_SEND_MAIL_ITEM_ERROR;
			}
		}
	}

	if (total_zeny > 0 && pc->payzeny(sd, (int)total_zeny, LOG_TYPE_MAIL, NULL) != 0) {
		rodex->clean(sd, 1);
		return RODEX_SEND_MAIL_FATAL_ERROR;
	}

	for (i = 0; i < RODEX_MAX_ITEM; i++) {
		int16 idx = sd->rodex.tmp.items[i].idx;

		if (sd->rodex.tmp.items[i].item.nameid == 0) {
			continue;
		}

		if (pc->delitem(sd, idx, sd->rodex.tmp.items[i].item.amount, 0, DELITEM_NORMAL, LOG_TYPE_MAIL) != 0) {
			rodex->clean(sd, 1);
			return RODEX_SEND_MAIL_ITEM_ERROR;
		}
	}

	sd->rodex.tmp.zeny = zeny;
	sd->rodex.tmp.is_read = false;
	sd->rodex.tmp.is_deleted = false;
	sd->rodex.tmp.send_date = (int)time(NULL);
	sd->rodex.tmp.expire_date = (int)time(NULL) + RODEX_EXPIRE;
	if (strlen(sd->rodex.tmp.body) > 0)
		sd->rodex.tmp.type |= MAIL_TYPE_TEXT;
	if (sd->rodex.tmp.zeny > 0)
		sd->rodex.tmp.type |= MAIL_TYPE_ZENY;
	sd->rodex.tmp.sender_id = sd->status.char_id;
	safestrncpy(sd->rodex.tmp.sender_name, sd->status.name, NAME_LENGTH);
	safestrncpy(sd->rodex.tmp.title, title, RODEX_TITLE_LENGTH);
	safestrncpy(sd->rodex.tmp.body, body, RODEX_BODY_LENGTH);

	intif->rodex_sendmail(&sd->rodex.tmp);
	return RODEX_SEND_MAIL_SUCCESS; // this will not inform client of the success yet. (see rodex_send_mail_result)
}

/// The result of a message send, called by char-server
/// @param ssd : Sender's sd
/// @param rsd : Receiver's sd
/// @param result : Message sent (true) or failed (false)
void rodex_send_mail_result(struct map_session_data *ssd, struct map_session_data *rsd, bool result)
{
	if (ssd != NULL) {
		rodex->clean(ssd, 1);
		if (result == false) {
			clif->rodex_send_mail_result(ssd->fd, ssd, RODEX_SEND_MAIL_FATAL_ERROR);
			return;
		}

		clif->rodex_send_mail_result(ssd->fd, ssd, RODEX_SEND_MAIL_SUCCESS);
	}

	if (rsd != NULL) {
		clif->rodex_icon(rsd->fd, true);
		clif_disp_onlyself(rsd, "You've got a new mail!");
	}
	return;
}

/// Retrieves one message from character
/// @param sd : Character
/// @param mail_id : Mail ID that's being retrieved
/// Returns the message
struct rodex_message *rodex_get_mail(struct map_session_data *sd, int64 mail_id)
{
	int i;
	struct rodex_message *msg;

	nullpo_retr(NULL, sd);

	ARR_FIND(0, VECTOR_LENGTH(sd->rodex.messages), i, VECTOR_INDEX(sd->rodex.messages, i).id == mail_id && VECTOR_INDEX(sd->rodex.messages, i).is_deleted != true);
	if (i == VECTOR_LENGTH(sd->rodex.messages))
		return NULL;

	msg = &VECTOR_INDEX(sd->rodex.messages, i);

	return msg;
}

/// Request to read a mail by its ID
/// @param sd : Who's reading
/// @param mail_id : Mail ID to be read
void rodex_read_mail(struct map_session_data *sd, int64 mail_id)
{
	struct rodex_message *msg;

	nullpo_retv(sd);

	msg = rodex->get_mail(sd, mail_id);
	nullpo_retv(msg);

	if (msg->is_read == false) {
		intif->rodex_updatemail(msg->id, 0);
		msg->is_read = true;
	}

	clif->rodex_read_mail(sd, msg->opentype, msg);
}

/// Deletes a mail
/// @param sd : Who's deleting
/// @param mail_id : Mail ID to be deleted
void rodex_delete_mail(struct map_session_data *sd, int64 mail_id)
{
	struct rodex_message *msg;

	nullpo_retv(sd);

	msg = rodex->get_mail(sd, mail_id);
	nullpo_retv(msg);

	msg->is_deleted = true;
	intif->rodex_updatemail(msg->id, 3);

	clif->rodex_delete_mail(sd, msg->opentype, msg->id);
}

/// Gets attached zeny
/// @param sd : Who's getting
/// @param mail_id : Mail ID that we're getting zeny from
void rodex_get_zeny(struct map_session_data *sd, int8 opentype, int64 mail_id)
{
	struct rodex_message *msg;

	nullpo_retv(sd);

	msg = rodex->get_mail(sd, mail_id);

	if (msg == NULL) {
		clif->rodex_request_zeny(sd, opentype, mail_id, RODEX_GET_ZENY_FATAL_ERROR);
		return;
	}

	if ((int64)sd->status.zeny + msg->zeny > MAX_ZENY) {
		clif->rodex_request_zeny(sd, opentype, mail_id, RODEX_GET_ZENY_LIMIT_ERROR);
		return;
	}

	if (pc->getzeny(sd, (int)msg->zeny, LOG_TYPE_MAIL, NULL) != 0) {
		clif->rodex_request_zeny(sd, opentype, mail_id, RODEX_GET_ZENY_FATAL_ERROR);
		return;
	}

	msg->zeny = 0;
	intif->rodex_updatemail(mail_id, 1);

	clif->rodex_request_zeny(sd, opentype, mail_id, RODEX_GET_ZENY_SUCCESS);
}

/// Gets attached item
/// @param sd : Who's getting
/// @param mail_id : Mail ID that we're getting items from
void rodex_get_items(struct map_session_data *sd, int8 opentype, int64 mail_id)
{
	struct rodex_message *msg;
	int weight = 0;
	int empty_slots = 0, required_slots;
	int i;

	nullpo_retv(sd);

	msg = rodex->get_mail(sd, mail_id);

	if (msg == NULL) {
		clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEM_FATAL_ERROR);
		return;
	}

	if (msg->items_count < 1) {
		clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEM_FATAL_ERROR);
		return;
	}

	for (i = 0; i < RODEX_MAX_ITEM; ++i) {
		if (msg->items[i].item.nameid != 0) {
			weight += itemdb->search(msg->items[i].item.nameid)->weight * msg->items[i].item.amount;
		}
	}

	if ((sd->weight + weight > sd->max_weight)) {
		clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEM_FULL_ERROR);
		return;
	}

	required_slots = msg->items_count;
	for (i = 0; i < MAX_INVENTORY; ++i) {
		if (sd->status.inventory[i].nameid == 0) {
			empty_slots++;
		} else if (itemdb->isstackable(sd->status.inventory[i].nameid) == 1) {
			int j;
			ARR_FIND(0, msg->items_count, j, sd->status.inventory[i].nameid == msg->items[j].item.nameid);
			if (j < msg->items_count) {
				struct item_data *idata = itemdb->search(sd->status.inventory[i].nameid);

				if ((idata->stack.inventory && sd->status.inventory[i].amount + msg->items[j].item.amount > idata->stack.amount) ||
					sd->status.inventory[i].amount + msg->items[j].item.amount > MAX_AMOUNT) {
					clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEM_FULL_ERROR);
					return;
				}

				required_slots--;
			}
		}
	}

	if (empty_slots < required_slots) {
		clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEM_FULL_ERROR);
		return;
	}

	for (i = 0; i < RODEX_MAX_ITEM; ++i) {
		struct item *it = &msg->items[i].item;

		if (it->nameid == 0) {
			continue;
		}

		if (pc->additem(sd, it, it->amount, LOG_TYPE_MAIL) != 0) {
			clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEM_FULL_ERROR);
			intif->rodex_updatemail(mail_id, 2);
			return;
		} else {
			memset(it, 0x0, sizeof(*it));
		}
	}

	intif->rodex_updatemail(mail_id, 2);

	clif->rodex_request_items(sd, opentype, mail_id, RODEX_GET_ITEMS_SUCCESS);
}

/// Cleans user's RoDEX related data
/// - should be called everytime we're going to stop using rodex in this character
/// @param sd : Target to clean
/// @param flag :
///		0 - clear everything
///		1 - clear tmp only
void rodex_clean(struct map_session_data *sd, int8 flag)
{
	nullpo_retv(sd);

	if (flag == 0)
		VECTOR_CLEAR(sd->rodex.messages);

	memset(&sd->rodex.tmp, 0x0, sizeof(sd->rodex.tmp));
}

/// User request to open rodex, load mails from char-server
/// @param sd : Who's requesting
/// @param open_type : Box Type (see RODEX_OPENTYPE)
void rodex_open(struct map_session_data *sd, int8 open_type)
{
	nullpo_retv(sd);
	if (open_type == RODEX_OPENTYPE_ACCOUNT && battle_config.feature_rodex_use_accountmail == false)
		open_type = RODEX_OPENTYPE_MAIL;

	intif->rodex_requestinbox(sd->status.char_id, sd->status.account_id, 0, open_type, 0);
}

/// User request to read next page of mails
/// @param sd : Who's requesting
/// @param open_type : Box Type (see RODEX_OPENTYPE)
/// @param last_mail_id : The last mail from the current page
void rodex_next_page(struct map_session_data *sd, int8 open_type, int64 last_mail_id)
{
	int64 msg_count, page_start = 0;
	nullpo_retv(sd);

	if (open_type == RODEX_OPENTYPE_ACCOUNT && battle_config.feature_rodex_use_accountmail == false) {
		// Should not happen
		open_type = RODEX_OPENTYPE_MAIL;
		rodex->open(sd, open_type);
		return;
	}

	msg_count = VECTOR_LENGTH(sd->rodex.messages);

	if (last_mail_id > 0) {
		// Find where the page starts
		ARR_FIND(0, msg_count, page_start, VECTOR_INDEX(sd->rodex.messages, page_start).id == last_mail_id);
		if (page_start > 0 && page_start < msg_count) {
			--page_start; // Valid page, get first item of next page
		} else {
			page_start = msg_count - 1; // Should not happen, invalid lower_id given
		}
		clif->rodex_send_maillist(sd->fd, sd, open_type, page_start);
	}
}

/// User's request to refresh his mail box
/// @param sd : Who's requesting
/// @param open_type : Box Type (See RODEX_OPENTYPE)
/// @param first_mail_id : The first mail id known by client, currently unused
void rodex_refresh(struct map_session_data *sd, int8 open_type, int64 first_mail_id)
{
	nullpo_retv(sd);
	if (open_type == RODEX_OPENTYPE_ACCOUNT && battle_config.feature_rodex_use_accountmail == false)
		open_type = RODEX_OPENTYPE_MAIL;

	// Some clients sends the first mail id it currently has and expects to receive
	// a list of newer mails, other clients sends first mail id as 0 and expects
	// to receive the first page (as if opening the box)
	if (first_mail_id > 0) {
		intif->rodex_requestinbox(sd->status.char_id, sd->status.account_id, 1, open_type, first_mail_id);
	} else {
		intif->rodex_requestinbox(sd->status.char_id, sd->status.account_id, 0, open_type, first_mail_id);
	}
}

void do_init_rodex(bool minimal)
{
	if (minimal)
		return;
}

void do_final_rodex(void)
{

}

void rodex_defaults(void)
{
	rodex = &rodex_s;

	rodex->init = do_init_rodex;
	rodex->final = do_final_rodex;

	rodex->open = rodex_open;
	rodex->next_page = rodex_next_page;
	rodex->refresh = rodex_refresh;
	rodex->isenabled = rodex_isenabled;
	rodex->add_item = rodex_add_item;
	rodex->remove_item = rodex_remove_item;
	rodex->check_player = rodex_check_player;
	rodex->send_mail = rodex_send_mail;
	rodex->send_mail_result = rodex_send_mail_result;
	rodex->get_mail = rodex_get_mail;
	rodex->read_mail = rodex_read_mail;
	rodex->delete_mail = rodex_delete_mail;
	rodex->get_zeny = rodex_get_zeny;
	rodex->get_items = rodex_get_items;
	rodex->clean = rodex_clean;
}
