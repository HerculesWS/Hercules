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

#include "mail.h"

#include "map/atcommand.h"
#include "map/clif.h"
#include "map/itemdb.h"
#include "map/log.h"
#include "map/pc.h"
#include "map/storage.h"
#include "common/nullpo.h"
#include "common/showmsg.h"

#include <time.h>
#include <string.h>

struct mail_interface mail_s;
struct mail_interface *mail;

void mail_clear(struct map_session_data *sd)
{
	nullpo_retv(sd);
	sd->mail.nameid = 0;
	sd->mail.index = 0;
	sd->mail.amount = 0;
	sd->mail.zeny = 0;

	return;
}

int mail_removeitem(struct map_session_data *sd, short flag)
{
	nullpo_ret(sd);

	if( sd->mail.amount )
	{
		if (flag) // Item send
			pc->delitem(sd, sd->mail.index, sd->mail.amount, 1, DELITEM_NORMAL, LOG_TYPE_MAIL);
		else
			clif->additem(sd, sd->mail.index, sd->mail.amount, 0);
	}

	sd->mail.nameid = 0;
	sd->mail.index = 0;
	sd->mail.amount = 0;
	return 1;
}

int mail_removezeny(struct map_session_data *sd, short flag)
{
	nullpo_ret(sd);

	if (flag && sd->mail.zeny > 0)
	{  //Zeny send
		pc->payzeny(sd,sd->mail.zeny,LOG_TYPE_MAIL, NULL);
	}
	sd->mail.zeny = 0;

	return 1;
}

unsigned char mail_setitem(struct map_session_data *sd, int idx, int amount) {

	nullpo_retr(1, sd);
	if( pc_istrading(sd) )
		return 1;

	if( idx == 0 ) { // Zeny Transfer
		if( amount < 0 || !pc_can_give_items(sd) )
			return 1;

		if( amount > sd->status.zeny )
			amount = sd->status.zeny;

		sd->mail.zeny = amount;
		// clif->updatestatus(sd, SP_ZENY);
		return 0;
	} else { // Item Transfer
		idx -= 2;
		mail->removeitem(sd, 0);

		if( idx < 0 || idx >= MAX_INVENTORY )
			return 1;
		if( amount <= 0 || amount > sd->status.inventory[idx].amount )
			return 1;
		if( !pc_can_give_items(sd) || sd->status.inventory[idx].expire_time ||
			!itemdb_canmail(&sd->status.inventory[idx],pc_get_group_level(sd)) ||
			(sd->status.inventory[idx].bound && !pc_can_give_bound_items(sd)) )
			return 1;

		sd->mail.index = idx;
		sd->mail.nameid = sd->status.inventory[idx].nameid;
		sd->mail.amount = amount;

		return 0;
	}
}

bool mail_setattachment(struct map_session_data *sd, struct mail_message *msg)
{
	int n;

	nullpo_retr(false,sd);
	nullpo_retr(false,msg);

	if( sd->mail.zeny < 0 || sd->mail.zeny > sd->status.zeny )
		return false;

	n = sd->mail.index;
	Assert_retr(false, n >= 0 && n < MAX_INVENTORY);
	if( sd->mail.amount )
	{
		if( sd->status.inventory[n].nameid != sd->mail.nameid )
			return false;

		if( sd->status.inventory[n].amount < sd->mail.amount )
			return false;

		if( sd->weight > sd->max_weight )
			return false;

		memcpy(&msg->item, &sd->status.inventory[n], sizeof(struct item));
		msg->item.amount = sd->mail.amount;
		if (msg->item.amount != sd->mail.amount)  // check for amount overflow
			return false;
	}
	else
		memset(&msg->item, 0x00, sizeof(struct item));

	msg->zeny = sd->mail.zeny;

	// Removes the attachment from sender
	mail->removeitem(sd,1);
	mail->removezeny(sd,1);

	return true;
}

void mail_getattachment(struct map_session_data* sd, int zeny, struct item* item)
{
	nullpo_retv(sd);
	nullpo_retv(item);
	if( item->nameid > 0 && item->amount > 0 )
	{
		pc->additem(sd, item, item->amount, LOG_TYPE_MAIL);
		clif->mail_getattachment(sd->fd, 0);
	}

	if( zeny > 0 )
	{  //Zeny receive
		pc->getzeny(sd, zeny,LOG_TYPE_MAIL, NULL);
	}
}

int mail_openmail(struct map_session_data *sd)
{
	nullpo_ret(sd);

	if (sd->state.storage_flag != STORAGE_FLAG_CLOSED || sd->state.vending || sd->state.buyingstore || sd->state.trading)
		return 0;

	clif->mail_window(sd->fd, 0);

	return 1;
}

void mail_deliveryfail(struct map_session_data *sd, struct mail_message *msg)
{
	nullpo_retv(sd);
	nullpo_retv(msg);

	if( msg->item.amount > 0 )
	{
		// Item receive (due to failure)
		pc->additem(sd, &msg->item, msg->item.amount, LOG_TYPE_MAIL);
	}

	if( msg->zeny > 0 )
	{
		pc->getzeny(sd,msg->zeny,LOG_TYPE_MAIL, NULL); //Zeny receive (due to failure)
	}

	clif->mail_send(sd->fd, true);
}

// This function only check if the mail operations are valid
bool mail_invalid_operation(struct map_session_data *sd) {
	nullpo_retr(false, sd);
	if( !map->list[sd->bl.m].flag.town && !pc->can_use_command(sd, "@mail") ) {
		ShowWarning("clif->parse_Mail: char '%s' trying to do invalid mail operations.\n", sd->status.name);
		return true;
	}

	return false;
}

void mail_defaults(void)
{
	mail = &mail_s;

	mail->clear = mail_clear;
	mail->removeitem = mail_removeitem;
	mail->removezeny = mail_removezeny;
	mail->setitem = mail_setitem;
	mail->setattachment = mail_setattachment;
	mail->getattachment = mail_getattachment;
	mail->openmail = mail_openmail;
	mail->deliveryfail = mail_deliveryfail;
	mail->invalid_operation = mail_invalid_operation;
}
