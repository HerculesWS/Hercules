/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2020 Hercules Dev Team
 * Copyright (C) Athena Dev Teams
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

#include "config/core.h" // GP_BOUND_ITEMS
#include "intif.h"

#include "map/atcommand.h"
#include "map/battle.h"
#include "map/chrif.h"
#include "map/clan.h"
#include "map/clif.h"
#include "map/elemental.h"
#include "map/guild.h"
#include "map/homunculus.h"
#include "map/log.h"
#include "map/mail.h"
#include "map/map.h"
#include "map/mercenary.h"
#include "map/party.h"
#include "map/pc.h"
#include "map/pet.h"
#include "map/quest.h"
#include "map/rodex.h"
#include "map/storage.h"
#include "map/achievement.h"

#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static struct intif_interface intif_s;
struct intif_interface *intif;

#define inter_fd (chrif->fd) // alias

//-----------------------------------------------------------------
// Send to inter server

static int CheckForCharServer(void)
{
	return ((chrif->fd <= 0) || sockt->session[chrif->fd] == NULL || sockt->session[chrif->fd]->wdata == NULL);
}

// pet
static int intif_create_pet(int account_id, int char_id, int pet_class, int pet_lv, int pet_egg_id,
	int pet_equip, short intimate, short hungry, char rename_flag, char incubate, char *pet_name)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(pet_name);
	WFIFOHEAD(inter_fd, 32 + NAME_LENGTH);
	WFIFOW(inter_fd, 0) = 0x3080;
	WFIFOL(inter_fd, 2) = account_id;
	WFIFOL(inter_fd, 6) = char_id;
	WFIFOL(inter_fd, 10) = pet_class;
	WFIFOL(inter_fd, 14) = pet_lv;
	WFIFOL(inter_fd, 18) = pet_egg_id;
	WFIFOL(inter_fd, 22) = pet_equip;
	WFIFOW(inter_fd, 26) = intimate;
	WFIFOW(inter_fd, 28) = hungry;
	WFIFOB(inter_fd, 30) = rename_flag;
	WFIFOB(inter_fd, 31) = incubate;
	memcpy(WFIFOP(inter_fd, 32), pet_name, NAME_LENGTH);
	WFIFOSET(inter_fd, 32 + NAME_LENGTH);

	return 0;
}

static int intif_request_petdata(int account_id, int char_id, int pet_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 14);
	WFIFOW(inter_fd,0) = 0x3081;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOL(inter_fd,10) = pet_id;
	WFIFOSET(inter_fd,14);

	return 0;
}

static int intif_save_petdata(int account_id, struct s_pet *p)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(p);
	WFIFOHEAD(inter_fd, sizeof(struct s_pet) + 8);
	WFIFOW(inter_fd,0) = 0x3082;
	WFIFOW(inter_fd,2) = sizeof(struct s_pet) + 8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),p,sizeof(struct s_pet));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));

	return 0;
}

static int intif_delete_petdata(int pet_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3083;
	WFIFOL(inter_fd,2) = pet_id;
	WFIFOSET(inter_fd,6);

	return 1;
}

static int intif_rename(struct map_session_data *sd, int type, const char *name)
{
	if (intif->CheckForCharServer())
		return 1;

	nullpo_ret(sd);
	nullpo_ret(name);
	WFIFOHEAD(inter_fd,NAME_LENGTH+12);
	WFIFOW(inter_fd,0) = 0x3006;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = type;  //Type: 0 - PC, 1 - PET, 2 - HOM
	memcpy(WFIFOP(inter_fd,11),name, NAME_LENGTH);
	WFIFOSET(inter_fd,NAME_LENGTH+12);
	return 0;
}

//Request for saving registry values.
static int intif_saveregistry(struct map_session_data *sd)
{
	struct DBIterator *iter;
	union DBKey key;
	struct DBData *data;
	int plen = 0;
	size_t len;

	nullpo_ret(sd);
	if (intif->CheckForCharServer() || !sd->regs.vars)
		return -1;

	WFIFOHEAD(inter_fd, 60000 + 300);
	WFIFOW(inter_fd,0)  = 0x3004;
	/* 0x2 = length (set later) */
	WFIFOL(inter_fd,4)  = sd->status.account_id;
	WFIFOL(inter_fd,8)  = sd->status.char_id;
	WFIFOW(inter_fd,12) = 0;/* count */

	plen = 14;

	iter = db_iterator(sd->regs.vars);
	for( data = iter->first(iter,&key); iter->exists(iter); data = iter->next(iter,&key) ) {
		const char *varname = NULL;
		struct script_reg_state *src = NULL;

		if( data->type != DB_DATA_PTR ) /* its a @number */
			continue;

		varname = script->get_str(script_getvarid(key.i64));

		if( varname[0] == '@' ) /* @string$ can get here, so we skip */
			continue;

		if (strlen(varname) > SCRIPT_VARNAME_LENGTH) {
			ShowError("Variable name too big: %s\n", varname);
			continue;
		}
		src = DB->data2ptr(data);

		/* no need! */
		if( !src->update )
			continue;

		src->update = false;

		len = strlen(varname)+1;

		WFIFOB(inter_fd, plen) = (unsigned char)len;/* won't be higher; the column size is 32 */
		plen += 1;

		safestrncpy(WFIFOP(inter_fd,plen), varname, len);
		plen += len;

		WFIFOL(inter_fd, plen) = script_getvaridx(key.i64);
		plen += 4;

		if( src->type ) {
			struct script_reg_str *p = (struct script_reg_str *)src;

			WFIFOB(inter_fd, plen) = p->value ? 2 : 3;
			plen += 1;

			if( p->value ) {
				len = strlen(p->value);

				WFIFOB(inter_fd, plen) = (unsigned char)len; // Won't be higher; the column size is 255.
				plen += 1;

				safestrncpy(WFIFOP(inter_fd, plen), p->value, len + 1);
				plen += len + 1;
			} else {
				script->reg_destroy_single(sd,key.i64,&p->flag);
			}
		} else {
			struct script_reg_num *p = (struct script_reg_num *)src;

			WFIFOB(inter_fd, plen) =  p->value ? 0 : 1;
			plen += 1;

			if( p->value ) {
				WFIFOL(inter_fd, plen) = p->value;
				plen += 4;
			} else {
				script->reg_destroy_single(sd,key.i64,&p->flag);
			}
		}

		WFIFOW(inter_fd,12) += 1;

		if( plen > 60000 ) {
			WFIFOW(inter_fd, 2) = plen;
			WFIFOSET(inter_fd, plen);

			/* prepare follow up */
			WFIFOHEAD(inter_fd, 60000 + 300);
			WFIFOW(inter_fd,0)  = 0x3004;
			/* 0x2 = length (set later) */
			WFIFOL(inter_fd,4)  = sd->status.account_id;
			WFIFOL(inter_fd,8)  = sd->status.char_id;
			WFIFOW(inter_fd,12) = 0;/* count */

			plen = 14;
		}
	}
	dbi_destroy(iter);

	/* mark & go. */
	WFIFOW(inter_fd, 2) = plen;
	WFIFOSET(inter_fd, plen);

	sd->vars_dirty = false;

	return 0;
}

//Request the registries for this player.
static int intif_request_registry(struct map_session_data *sd, int flag)
{
	nullpo_ret(sd);

	/* if char server ain't online it doesn't load, shouldn't we kill the session then? */
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3005;
	WFIFOL(inter_fd,2) = sd->status.account_id;
	WFIFOL(inter_fd,6) = sd->status.char_id;
	WFIFOB(inter_fd,10) = (flag&1) ? 1 : 0; //Request Acc Reg 2
	WFIFOB(inter_fd,11) = (flag&2) ? 1 : 0; //Request Acc Reg
	WFIFOB(inter_fd,12) = (flag&4) ? 1 : 0; //Request Char Reg
	WFIFOSET(inter_fd,13);

	return 0;
}

//=================================================================
// Account Storage
//-----------------------------------------------------------------

/**
 * Request the inter-server for a character's storage data.
 * 
 * @packet 0x3010      [out] <account_id>.L <storage_id>.W <storage_size>.W
 * @param  sd          [in]  pointer to session data.
 * @param  storage_id  [in]  storage id to retrieve
 */
static void intif_request_account_storage(const struct map_session_data *sd, int storage_id)
{
	nullpo_retv(sd);

	const struct storage_settings *stst = storage->get_settings(storage_id);
	nullpo_retv(stst);

	/* Check for character server availability */
	if (intif->CheckForCharServer())
		return;

	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd, 0) = 0x3010;
	WFIFOL(inter_fd, 2) = sd->status.account_id;
	WFIFOW(inter_fd, 6) = storage_id;
	WFIFOW(inter_fd, 8) = stst->capacity;
	WFIFOSET(inter_fd, 10);
}

/**
 * Parse the reception of account storage from the inter-server.
 * @packet 0x3805 [in] <packet_len>.W <account_id>.L <storage_id>.W <struct item[]>.P
 * @param  fd     [in] file/socket descriptor.
 */
static void intif_parse_account_storage(int fd)
{
	int account_id = 0, payload_size = 0, storage_count = 0;
	int i = 0;
	struct map_session_data *sd = NULL;

	Assert_retv(fd > 0);

	payload_size = RFIFOW(fd, 2) - 10;

	if ((account_id = RFIFOL(fd, 4)) == 0 || (sd = map->id2sd(account_id)) == NULL) {
		ShowError("intif_parse_account_storage: Session pointer was null for account id %d!\n", account_id);
		return;
	}

	struct storage_data *stor = NULL;
	if ((stor = storage->ensure(sd, RFIFOW(fd, 8))) == NULL)
		return;

	if (stor->received) {
		ShowError("intif_parse_account_storage: Multiple calls from the inter-server received.\n");
		return;
	}

	storage_count = (payload_size/sizeof(struct item));

	VECTOR_ENSURE(stor->item, storage_count, 1);

	stor->aggregate = storage_count; // Total items in storage.

	for (i = 0; i < storage_count; i++) {
		const struct item *it = RFIFOP(fd, 10 + i * sizeof(struct item));
		VECTOR_PUSH(stor->item, *it);
	}

	stor->received = true; // Mark the storage state as received.
	stor->save = false; // Initialize the save flag as false.

	pc->checkitem(sd); // re-check remaining items.
}

/**
 * Send account storage information for saving.
 * 
 * @packet 0x3011      [out] <packet_len>.W <account_id>.L <storage_id>.W <struct item[]>.P
 * @param  sd          [in]  pointer to session data.
 * @param  storage_id  [in]  storage id to be saved.
 */
static void intif_send_account_storage(struct map_session_data *sd, int storage_id)
{
	int len = 0, i = 0, c = 0;

	nullpo_retv(sd);

	struct storage_data *stor = NULL;
	if ((stor = storage->ensure(sd, storage_id)) == NULL)
		return;

	// Assert that at this point in the code, both flags are true.
	Assert_retv(stor->save);
	Assert_retv(stor->received);

	if (intif->CheckForCharServer())
		return;

	len = 10 + stor->aggregate * sizeof(struct item);

	WFIFOHEAD(inter_fd, len);

	WFIFOW(inter_fd, 0) = 0x3011;
	WFIFOW(inter_fd, 2) = (uint16) len;
	WFIFOL(inter_fd, 4) = sd->status.account_id;
	WFIFOW(inter_fd, 8) = storage_id;

	for (i = 0, c = 0; i < VECTOR_LENGTH(stor->item); i++) {
		if (VECTOR_INDEX(stor->item, i).nameid == 0)
			continue;
		memcpy(WFIFOP(inter_fd, 10 + c * sizeof(struct item)), &VECTOR_INDEX(stor->item, i), sizeof(struct item));
		c++;
	}

	WFIFOSET(inter_fd, len);

	stor->save = false; // Save request has been sent
}

/**
 * Parse acknowledgement packet for the saving of an account's storage.
 * @packet 0x3808 [in] <account_id>.L <storage_id>.W <saved_flag>.B
 * @param fd      [in] file/socket descriptor.
 */
static void intif_parse_account_storage_save_ack(int fd)
{
	int account_id = RFIFOL(fd, 2);
	int storage_id = RFIFOW(fd, 6);
	char saved = RFIFOB(fd, 8);

	Assert_retv(account_id > 0);
	Assert_retv(fd > 0);
	Assert_retv(storage_id >= 0);
	
	struct map_session_data *sd = NULL;
	if ((sd = map->id2sd(account_id)) == NULL)
		return; // character is most probably offline.

	if (saved == 0) {
		ShowError("intif_parse_account_storage_save_ack: Storage id %d has not been saved! (AID: %d)\n", storage_id, account_id);
		return;
	}

	struct storage_data *stor = NULL;
	if ((stor = storage->ensure(sd, storage_id)) == NULL)
		return;

	stor->save = false; // Storage has been saved.
}

//=================================================================
// Guild Storage
//-----------------------------------------------------------------

static int intif_request_guild_storage(int account_id, int guild_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3018;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = guild_id;
	WFIFOSET(inter_fd,10);
	return 0;
}
static int intif_send_guild_storage(int account_id, struct guild_storage *gstor)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(gstor);
	WFIFOHEAD(inter_fd,sizeof(struct guild_storage)+12);
	WFIFOW(inter_fd,0) = 0x3019;
	WFIFOW(inter_fd,2) = (unsigned short)sizeof(struct guild_storage)+12;
	WFIFOL(inter_fd,4) = account_id;
	WFIFOL(inter_fd,8) = gstor->guild_id;
	memcpy( WFIFOP(inter_fd,12),gstor, sizeof(struct guild_storage) );
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Party creation request
static int intif_create_party(struct party_member *member, const char *name, int item, int item2)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(member);
	nullpo_ret(name);

	WFIFOHEAD(inter_fd,64);
	WFIFOW(inter_fd,0) = 0x3020;
	WFIFOW(inter_fd,2) = 30+sizeof(struct party_member);
	memcpy(WFIFOP(inter_fd,4),name, NAME_LENGTH);
	WFIFOB(inter_fd,28)= item;
	WFIFOB(inter_fd,29)= item2;
	memcpy(WFIFOP(inter_fd,30), member, sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 0;
}

// Party information request
static int intif_request_partyinfo(int party_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3021;
	WFIFOL(inter_fd,2) = party_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 0;
}

// Request to add a member to party
static int intif_party_addmember(int party_id, struct party_member *member)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(member);
	WFIFOHEAD(inter_fd,42);
	WFIFOW(inter_fd,0)=0x3022;
	WFIFOW(inter_fd,2)=8+sizeof(struct party_member);
	WFIFOL(inter_fd,4)=party_id;
	memcpy(WFIFOP(inter_fd,8),member,sizeof(struct party_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd, 2));
	return 1;
}

// Request to change party configuration (exp,item share)
static int intif_party_changeoption(int party_id, int account_id, int exp, int item)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3023;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOW(inter_fd,10)=exp;
	WFIFOW(inter_fd,12)=item;
	WFIFOSET(inter_fd,14);
	return 0;
}

// Request to leave party
static int intif_party_leave(int party_id, int account_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3024;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}

// Request keeping party for new map ??
static int intif_party_changemap(struct map_session_data *sd, int online)
{
	int16 m, map_index;

	if (intif->CheckForCharServer())
		return 0;
	if(!sd)
		return 0;

	if( (m=map->mapindex2mapid(sd->mapindex)) >= 0 && map->list[m].instance_id >= 0 )
		map_index = map_id2index(map->list[m].instance_src_map);
	else
		map_index = sd->mapindex;

	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd,0)=0x3025;
	WFIFOL(inter_fd,2)=sd->status.party_id;
	WFIFOL(inter_fd,6)=sd->status.account_id;
	WFIFOL(inter_fd,10)=sd->status.char_id;
	WFIFOW(inter_fd,14)=map_index;
	WFIFOB(inter_fd,16)=online;
	WFIFOW(inter_fd,17)=sd->status.base_level;
	WFIFOSET(inter_fd,19);
	return 1;
}

// Request breaking party
static int intif_break_party(int party_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0)=0x3026;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

// Request a new leader for party
static int intif_party_leaderchange(int party_id, int account_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,14);
	WFIFOW(inter_fd,0)=0x3029;
	WFIFOL(inter_fd,2)=party_id;
	WFIFOL(inter_fd,6)=account_id;
	WFIFOL(inter_fd,10)=char_id;
	WFIFOSET(inter_fd,14);
	return 0;
}

//=========================
// Clan System
//-------------------------

/**
 * Request clan member count
 *
 * @param clan_id Id of the clan to have members counted
 * @param kick_interval Interval of the inactivity kick
 */
static int intif_clan_membercount(int clan_id, int kick_interval)
{
	if (intif->CheckForCharServer() || clan_id == 0 || kick_interval <= 0)
		return 0;

	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd, 0) = 0x3044;
	WFIFOL(inter_fd, 2) = clan_id;
	WFIFOL(inter_fd, 6) = kick_interval;
	WFIFOSET(inter_fd, 10);
	return 1;
}

static int intif_clan_kickoffline(int clan_id, int kick_interval)
{
	if (intif->CheckForCharServer() || clan_id == 0 || kick_interval <= 0)
		return 0;

	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd, 0) = 0x3045;
	WFIFOL(inter_fd, 2) = clan_id;
	WFIFOL(inter_fd, 6) = kick_interval;
	WFIFOSET(inter_fd, 10);
	return 1;
}

static void intif_parse_RecvClanMemberAction(int fd)
{
	struct clan *c;
	int clan_id = RFIFOL(fd, 2);
	int count = RFIFOL(fd, 6);

	if ((c = clan->search(clan_id)) == NULL) {
		ShowError("intif_parse_RecvClanMemberAction: Received invalid clan_id '%d'\n", clan_id);
		return;
	}

	if (count < 0) {
		ShowError("intif_parse_RecvClanMemberAction: Received invalid member count value '%d'\n", count);
		return;
	}

	c->received = true;
	if (c->req_count_tid != INVALID_TIMER) {
		timer->delete(c->req_count_tid, clan->request_membercount);
		c->req_count_tid = INVALID_TIMER;
	}

	c->member_count = count;
	switch (c->req_state) {
	case CLAN_REQ_AFTER_KICK:
		if (c->req_kick_tid != INVALID_TIMER) {
			timer->delete(c->req_kick_tid, clan->request_kickoffline);
			c->req_kick_tid = INVALID_TIMER;
		}
		break;
	case CLAN_REQ_RELOAD:
		map->foreachpc(clan->rejoin);
		break;
	default:
		break;
	}
	c->req_state = CLAN_REQ_NONE;
}

// Request a Guild creation
static int intif_guild_create(const char *name, const struct guild_member *master)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(master);
	nullpo_ret(name);

	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+(8+NAME_LENGTH));
	WFIFOW(inter_fd,0)=0x3030;
	WFIFOW(inter_fd,2)=sizeof(struct guild_member)+(8+NAME_LENGTH);
	WFIFOL(inter_fd,4)=master->account_id;
	memcpy(WFIFOP(inter_fd,8),name,NAME_LENGTH);
	memcpy(WFIFOP(inter_fd,8+NAME_LENGTH),master,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Request Guild information
static int intif_guild_request_info(int guild_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3031;
	WFIFOL(inter_fd,2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

// Request to add member to the guild
static int intif_guild_addmember(int guild_id, struct guild_member *m)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(m);
	WFIFOHEAD(inter_fd,sizeof(struct guild_member)+8);
	WFIFOW(inter_fd,0) = 0x3032;
	WFIFOW(inter_fd,2) = sizeof(struct guild_member)+8;
	WFIFOL(inter_fd,4) = guild_id;
	memcpy(WFIFOP(inter_fd,8),m,sizeof(struct guild_member));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Request a new leader for guild
static int intif_guild_change_gm(int guild_id, const char *name, int len)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(name);
	Assert_ret(len > 0 && len < 32000);
	WFIFOHEAD(inter_fd, len + 8);
	WFIFOW(inter_fd, 0)=0x3033;
	WFIFOW(inter_fd, 2)=len+8;
	WFIFOL(inter_fd, 4)=guild_id;
	memcpy(WFIFOP(inter_fd,8),name,len);
	WFIFOSET(inter_fd,len+8);
	return 0;
}

// Request to leave guild
static int intif_guild_leave(int guild_id, int account_id, int char_id, int flag, const char *mes)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(mes);
	WFIFOHEAD(inter_fd, 55);
	WFIFOW(inter_fd, 0) = 0x3034;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = flag;
	safestrncpy(WFIFOP(inter_fd,15),mes,40);
	WFIFOSET(inter_fd,55);
	return 0;
}

//Update request / Lv online status of the guild members
static int intif_guild_memberinfoshort(int guild_id, int account_id, int char_id, int online, int lv, int class)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 23);
	WFIFOW(inter_fd, 0) = 0x3035;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd,10) = char_id;
	WFIFOB(inter_fd,14) = online;
	WFIFOL(inter_fd,15) = lv;
	WFIFOL(inter_fd,19) = class;
	WFIFOSET(inter_fd,23);
	return 0;
}

//Guild disbanded notification
static int intif_guild_break(int guild_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3036;
	WFIFOL(inter_fd, 2) = guild_id;
	WFIFOSET(inter_fd,6);
	return 0;
}

/**
 * Requests to change a basic guild information, it is parsed via mapif_parse_GuildBasicInfoChange
 * To see the information types that can be changed see mmo.h::guild_basic_info
 **/
static int intif_guild_change_basicinfo(int guild_id, int type, const void *data, int len)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(data);
	Assert_ret(len >= 0 && len < 32000);
	WFIFOHEAD(inter_fd, len + 10);
	WFIFOW(inter_fd,0)=0x3039;
	WFIFOW(inter_fd,2)=len+10;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOW(inter_fd,8)=type;
	memcpy(WFIFOP(inter_fd,10),data,len);
	WFIFOSET(inter_fd,len+10);
	return 0;
}

// Request a change of Guild member information
static int intif_guild_change_memberinfo(int guild_id, int account_id, int char_id, int type, const void *data, int len)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(data);
	Assert_ret(len >= 0 && len < 32000);
	WFIFOHEAD(inter_fd, len + 18);
	WFIFOW(inter_fd, 0)=0x303a;
	WFIFOW(inter_fd, 2)=len+18;
	WFIFOL(inter_fd, 4)=guild_id;
	WFIFOL(inter_fd, 8)=account_id;
	WFIFOL(inter_fd,12)=char_id;
	WFIFOW(inter_fd,16)=type;
	memcpy(WFIFOP(inter_fd,18),data,len);
	WFIFOSET(inter_fd,len+18);
	return 0;
}

// Request a change of Guild title
static int intif_guild_position(int guild_id, int idx, struct guild_position *p)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(p);
	WFIFOHEAD(inter_fd, sizeof(struct guild_position)+12);
	WFIFOW(inter_fd,0)=0x303b;
	WFIFOW(inter_fd,2)=sizeof(struct guild_position)+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=idx;
	memcpy(WFIFOP(inter_fd,12),p,sizeof(struct guild_position));
	WFIFOSET(inter_fd,WFIFOW(inter_fd,2));
	return 0;
}

// Request an update of Guildskill skill_id
static int intif_guild_skillup(int guild_id, uint16 skill_id, int account_id, int max)
{
	if( intif->CheckForCharServer() )
		return 0;
	WFIFOHEAD(inter_fd, 18);
	WFIFOW(inter_fd, 0)  = 0x303c;
	WFIFOL(inter_fd, 2)  = guild_id;
	WFIFOL(inter_fd, 6)  = skill_id;
	WFIFOL(inter_fd, 10) = account_id;
	WFIFOL(inter_fd, 14) = max;
	WFIFOSET(inter_fd, 18);
	return 0;
}

// Request a new guild relationship
static int intif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,19);
	WFIFOW(inter_fd, 0)=0x303d;
	WFIFOL(inter_fd, 2)=guild_id1;
	WFIFOL(inter_fd, 6)=guild_id2;
	WFIFOL(inter_fd,10)=account_id1;
	WFIFOL(inter_fd,14)=account_id2;
	WFIFOB(inter_fd,18)=flag;
	WFIFOSET(inter_fd,19);
	return 0;
}

// Request to change guild notice
static int intif_guild_notice(int guild_id, const char *mes1, const char *mes2)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(mes1);
	nullpo_ret(mes2);
	WFIFOHEAD(inter_fd,186);
	WFIFOW(inter_fd,0)=0x303e;
	WFIFOL(inter_fd,2)=guild_id;
	safestrncpy(WFIFOP(inter_fd, 6), mes1, MAX_GUILDMES1);
	safestrncpy(WFIFOP(inter_fd, 66), mes2, MAX_GUILDMES2);
	WFIFOSET(inter_fd,186);
	return 0;
}

// Request to change guild emblem
static int intif_guild_emblem(int guild_id, int len, const char *data)
{
	if (intif->CheckForCharServer())
		return 0;
	if(guild_id<=0 || len<0 || len>2000)
		return 0;
	nullpo_ret(data);
	Assert_ret(len >= 0 && len < 32000);
	WFIFOHEAD(inter_fd,len + 12);
	WFIFOW(inter_fd,0)=0x303f;
	WFIFOW(inter_fd,2)=len+12;
	WFIFOL(inter_fd,4)=guild_id;
	WFIFOL(inter_fd,8)=0;
	memcpy(WFIFOP(inter_fd,12),data,len);
	WFIFOSET(inter_fd,len+12);
	return 0;
}

/**
 * Requests guild castles data from char-server.
 * @param num Number of castles, size of castle_ids array.
 * @param castle_ids Pointer to array of castle IDs.
 */
static int intif_guild_castle_dataload(int num, int *castle_ids)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(castle_ids);
	WFIFOHEAD(inter_fd, 4 + num * sizeof(int));
	WFIFOW(inter_fd, 0) = 0x3040;
	WFIFOW(inter_fd, 2) = 4 + num * sizeof(int);
	memcpy(WFIFOP(inter_fd, 4), castle_ids, num * sizeof(int));
	WFIFOSET(inter_fd, WFIFOW(inter_fd, 2));
	return 1;
}

// Request change castle guild owner and save data
static int intif_guild_castle_datasave(int castle_id, int index, int value)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd,9);
	WFIFOW(inter_fd,0)=0x3041;
	WFIFOW(inter_fd,2)=castle_id;
	WFIFOB(inter_fd,4)=index;
	WFIFOL(inter_fd,5)=value;
	WFIFOSET(inter_fd,9);
	return 1;
}

//-----------------------------------------------------------------
// Homunculus Packets send to Inter server [albator]
//-----------------------------------------------------------------

static int intif_homunculus_create(int account_id, struct s_homunculus *sh)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(sh);
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3090;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;
}

static bool intif_homunculus_requestload(int account_id, int homun_id)
{
	if (intif->CheckForCharServer())
		return false;
	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd,0) = 0x3091;
	WFIFOL(inter_fd,2) = account_id;
	WFIFOL(inter_fd,6) = homun_id;
	WFIFOSET(inter_fd, 10);
	return true;
}

static int intif_homunculus_requestsave(int account_id, struct s_homunculus *sh)
{
	if (intif->CheckForCharServer())
		return 0;
	nullpo_ret(sh);
	WFIFOHEAD(inter_fd, sizeof(struct s_homunculus)+8);
	WFIFOW(inter_fd,0) = 0x3092;
	WFIFOW(inter_fd,2) = sizeof(struct s_homunculus)+8;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8),sh,sizeof(struct s_homunculus));
	WFIFOSET(inter_fd, WFIFOW(inter_fd,2));
	return 0;

}

static int intif_homunculus_requestdelete(int homun_id)
{
	if (intif->CheckForCharServer())
		return 0;
	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3093;
	WFIFOL(inter_fd,2) = homun_id;
	WFIFOSET(inter_fd,6);
	return 0;

}

//-----------------------------------------------------------------
// Packets receive from inter server

// Request player registre
static void intif_parse_Registers(int fd)
{
	int flag;
	struct map_session_data *sd;
	int account_id = RFIFOL(fd,4), char_id = RFIFOL(fd,8);
	struct auth_node *node = chrif->auth_check(account_id, char_id, ST_LOGIN);
	char type = RFIFOB(fd, 13);

	if (node)
		sd = node->sd;
	else { //Normally registries should arrive for in log-in chars.
		sd = map->id2sd(account_id);
	}

	if (!sd || sd->status.char_id != char_id) {
		return; //Character registry from another character.
	}

	flag = ( sd->vars_received&PRL_ACCG && sd->vars_received&PRL_ACCL && sd->vars_received&PRL_CHAR ) ? 0 : 1;

	switch (RFIFOB(fd,12)) {
		case 3: //Character Registry
			sd->vars_received |= PRL_CHAR;
			break;
		case 2: //Account Registry
			sd->vars_received |= PRL_ACCL;
			break;
		case 1: //Account2 Registry
			sd->vars_received |= PRL_ACCG;
			break;
		case 0:
			break;
		default:
			ShowError("intif_parse_Registers: Unrecognized type %d\n",RFIFOB(fd,12));
			return;
	}
	/* have it not complain about insertion of vars before loading, and not set those vars as new or modified */
	pc->reg_load = true;

	if (RFIFOW(fd, 14) != 0) {
		char key[SCRIPT_VARNAME_LENGTH+1];
		unsigned int index;
		int max = RFIFOW(fd, 14), cursor = 16, i;

		script->parser_current_file = "loading char/acc variables";//for script_add_str to refer to here in case errors occur

		/**
		 * Vessel!char_reg_num_db
		 *
		 * str type
		 * { keyLength(B), key(<keyLength>), index(L), valLength(B), val(<valLength>) }
		 **/
		if (type) {
			char sval[SCRIPT_STRING_VAR_LENGTH + 1];
			for (i = 0; i < max; i++) {
				int len = RFIFOB(fd, cursor);
				safestrncpy(key, RFIFOP(fd, cursor + 1), min((int)sizeof(key), len));
				cursor += len + 1;

				index = RFIFOL(fd, cursor);
				cursor += 4;

				len = RFIFOB(fd, cursor);
				safestrncpy(sval, RFIFOP(fd, cursor + 1), min((int)sizeof(sval), len + 1));
				cursor += len + 2;

				script->set_reg(NULL,sd,reference_uid(script->add_variable(key), index), key, sval, NULL);
			}
		/**
		 * Vessel!
		 *
		 * int type
		 * { keyLength(B), key(<keyLength>), index(L), value(L) }
		 **/
		} else {
			for (i = 0; i < max; i++) {
				int ival;

				int len = RFIFOB(fd, cursor);
				safestrncpy(key, RFIFOP(fd, cursor + 1), min((int)sizeof(key), len));
				cursor += len + 1;

				index = RFIFOL(fd, cursor);
				cursor += 4;

				ival = RFIFOL(fd, cursor);
				cursor += 4;

				script->set_reg(NULL,sd,reference_uid(script->add_variable(key), index), key, (const void *)h64BPTRSIZE(ival), NULL);
			}
		}
		script->parser_current_file = NULL;/* reset */
	}

	/* flag it back */
	pc->reg_load = false;

	if (flag && sd->vars_received&PRL_ACCG && sd->vars_received&PRL_ACCL && sd->vars_received&PRL_CHAR)
		pc->reg_received(sd); //Received all registry values, execute init scripts and what-not. [Skotlex]
}

static void intif_parse_LoadGuildStorage(int fd)
{
	struct guild_storage *gstor;
	struct map_session_data *sd;
	int guild_id, flag;

	guild_id = RFIFOL(fd,8);
	flag = RFIFOL(fd,12);
	if(guild_id <= 0)
		return;
	sd=map->id2sd( RFIFOL(fd,4) );
	if( flag ){ //If flag != 0, we attach a player and open the storage
		if(sd==NULL){
			ShowError("intif_parse_LoadGuildStorage: user not found %u\n", RFIFOL(fd,4));
			return;
		}
	}
	gstor=gstorage->ensure(guild_id);
	if(!gstor) {
		ShowWarning("intif_parse_LoadGuildStorage: error guild_id %d not exist\n",guild_id);
		return;
	}
	if (gstor->storage_status == 1) { // Already open.. lets ignore this update
		ShowWarning("intif_parse_LoadGuildStorage: storage received for a client already open (User %d:%d)\n", flag?sd->status.account_id:0, flag?sd->status.char_id:0);
		return;
	}
	if (gstor->dirty) { // Already have storage, and it has been modified and not saved yet! Exploit! [Skotlex]
		ShowWarning("intif_parse_LoadGuildStorage: received storage for an already modified non-saved storage! (User %d:%d)\n", flag?sd->status.account_id:0, flag?sd->status.char_id:0);
		return;
	}
	if (RFIFOW(fd,2)-13 != sizeof(struct guild_storage)) {
		ShowError("intif_parse_LoadGuildStorage: data size mismatch %d != %"PRIuS"\n", RFIFOW(fd,2)-13, sizeof(struct guild_storage));
		gstor->storage_status = 0;
		return;
	}

	memcpy(gstor,RFIFOP(fd,13),sizeof(struct guild_storage));
	if( flag )
		gstorage->open(sd);
}

// ACK guild_storage saved
static void intif_parse_SaveGuildStorage(int fd)
{
	gstorage->saved(/*RFIFOL(fd,2), */RFIFOL(fd,6));
}

// ACK party creation
static void intif_parse_PartyCreated(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party created by account %u\n\n", RFIFOL(fd,2));
	party->created(RFIFOL(fd,2), RFIFOL(fd,6),RFIFOB(fd,10),RFIFOL(fd,11), RFIFOP(fd,15));
}

// Receive party info
static void intif_parse_PartyInfo(int fd)
{
	if (RFIFOW(fd,2) == 12) {
		ShowWarning("intif: party noinfo (char_id=%u party_id=%u)\n", RFIFOL(fd,4), RFIFOL(fd,8));
		party->recv_noinfo(RFIFOL(fd,8), RFIFOL(fd,4));
		return;
	}

	if (RFIFOW(fd,2) != 8+sizeof(struct party))
		ShowError("intif: party info: data size mismatch (char_id=%u party_id=%u packet_len=%d expected_len=%"PRIuS")\n",
		          RFIFOL(fd,4), RFIFOL(fd,8), RFIFOW(fd,2), 8+sizeof(struct party));
	party->recv_info(RFIFOP(fd,8), RFIFOL(fd,4));
}

// ACK adding party member
static void intif_parse_PartyMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member added Party (%u), Account(%u), Char(%u)\n", RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
	party->member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10), RFIFOB(fd, 14));
}

// ACK changing party option
static void intif_parse_PartyOptionChanged(int fd)
{
	party->optionchanged(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOW(fd,10),RFIFOW(fd,12),RFIFOB(fd,14));
}

// ACK member leaving party
static void intif_parse_PartyMemberWithdraw(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: party member withdraw: Party(%u), Account(%u), Char(%u)\n", RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10));
	party->member_withdraw(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// ACK party break
static void intif_parse_PartyBroken(int fd)
{
	party->broken(RFIFOL(fd,2));
}

// ACK party on new map
static void intif_parse_PartyMove(int fd)
{
	party->recv_movemap(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOW(fd,14),RFIFOB(fd,16),RFIFOW(fd,17));
}

// ACK guild creation
static void intif_parse_GuildCreated(int fd)
{
	guild->created(RFIFOL(fd,2),RFIFOL(fd,6));
}

// ACK guild infos
static void intif_parse_GuildInfo(int fd)
{
	if (RFIFOW(fd,2) == 8) {
		ShowWarning("intif: guild noinfo %u\n", RFIFOL(fd,4));
		guild->recv_noinfo(RFIFOL(fd,4));
		return;
	}
	if (RFIFOW(fd,2)!=sizeof(struct guild)+4)
		ShowError("intif: guild info: data size mismatch - Gid: %u recv size: %d Expected size: %"PRIuS"\n",
		          RFIFOL(fd,4), RFIFOW(fd,2), sizeof(struct guild)+4);
	guild->recv_info(RFIFOP(fd,4));
}

// ACK adding guild member
static void intif_parse_GuildMemberAdded(int fd)
{
	if(battle_config.etc_log)
		ShowInfo("intif: guild member added %u %u %u %d\n", RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14));
	guild->member_added(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14));
}

// ACK member leaving guild
static void intif_parse_GuildMemberWithdraw(int fd)
{
	guild->member_withdraw(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOP(fd,55), RFIFOP(fd,15));
}

// ACK guild member basic info
static void intif_parse_GuildMemberInfoShort(int fd)
{
	guild->recv_memberinfoshort(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOL(fd,17),RFIFOL(fd,21));
}

// ACK guild break
static void intif_parse_GuildBroken(int fd)
{
	guild->broken(RFIFOL(fd,2),RFIFOB(fd,6));
}

// basic guild info change notice
// 0x3839 <packet len>.w <guild id>.l <type>.w <data>.?b
static void intif_parse_GuildBasicInfoChanged(int fd)
{
	//int len = RFIFOW(fd,2) - 10;
	int guild_id = RFIFOL(fd,4);
	int type = RFIFOW(fd,8);
	//void* data = RFIFOP(fd,10);

	struct guild* g = guild->search(guild_id);
	if( g == NULL )
		return;

	switch(type) {
		case GBI_EXP:        g->exp = RFIFOQ(fd,10); break;
		case GBI_GUILDLV:    g->guild_lv = RFIFOW(fd,10); break;
		case GBI_SKILLPOINT: g->skill_point = RFIFOL(fd,10); break;
		case GBI_SKILLLV: {
			int idx, max;
			const struct guild_skill *p_gs = RFIFOP(fd,10);
			struct guild_skill *gs = NULL;

			idx = p_gs->id - GD_SKILLBASE;
			Assert_retv(idx >= 0 && idx < MAX_GUILDSKILL);

			gs = &g->skill[idx];
			memcpy(gs, p_gs, sizeof(*gs));

			max = guild->skill_get_max(gs->id);
			if (gs->lv > max)
				gs->lv = max;

			break;
		}
	}
}

// guild member info change notice
// 0x383a <packet len>.w <guild id>.l <account id>.l <char id>.l <type>.w <data>.?b
static void intif_parse_GuildMemberInfoChanged(int fd)
{
	//int len = RFIFOW(fd,2) - 18;
	int guild_id = RFIFOL(fd,4);
	int account_id = RFIFOL(fd,8);
	int char_id = RFIFOL(fd,12);
	int type = RFIFOW(fd,16);
	//void* data = RFIFOP(fd,18);

	struct guild* g;
	int idx;

	g = guild->search(guild_id);
	if( g == NULL )
		return;

	idx = guild->getindex(g,account_id,char_id);
	if (idx == INDEX_NOT_FOUND)
		return;

	switch( type ) {
		case GMI_POSITION:   g->member[idx].position   = RFIFOW(fd,18); guild->memberposition_changed(g,idx,RFIFOW(fd,18)); break;
		case GMI_EXP:        g->member[idx].exp        = RFIFOQ(fd,18); break;
		case GMI_HAIR:       g->member[idx].hair       = RFIFOW(fd,18); break;
		case GMI_HAIR_COLOR: g->member[idx].hair_color = RFIFOW(fd,18); break;
		case GMI_GENDER:     g->member[idx].gender     = RFIFOW(fd,18); break;
		case GMI_CLASS:      g->member[idx].class      = RFIFOW(fd,18); break;
		case GMI_LEVEL:      g->member[idx].lv         = RFIFOW(fd,18); break;
	}
}

// ACK change of guild title
static void intif_parse_GuildPosition(int fd)
{
	if (RFIFOW(fd,2)!=sizeof(struct guild_position)+12)
		ShowError("intif: guild info: data size mismatch (%u) %d != %"PRIuS"\n",
		          RFIFOL(fd,4), RFIFOW(fd,2), sizeof(struct guild_position) + 12);
	guild->position_changed(RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12));
}

// ACK change of guild skill update
static void intif_parse_GuildSkillUp(int fd)
{
	guild->skillupack(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// ACK change of guild relationship
static void intif_parse_GuildAlliance(int fd)
{
	guild->allianceack(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOB(fd,18), RFIFOP(fd,19), RFIFOP(fd,43));
}

// ACK change of guild notice
static void intif_parse_GuildNotice(int fd)
{
	guild->notice_changed(RFIFOL(fd,2), RFIFOP(fd,6), RFIFOP(fd,66));
}

// ACK change of guild emblem
static void intif_parse_GuildEmblem(int fd)
{
	guild->emblem_changed(RFIFOW(fd,2)-12, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12));
}

// Reply guild castle data request
static void intif_parse_GuildCastleDataLoad(int fd)
{
	guild->castledataloadack(RFIFOW(fd,2), RFIFOP(fd,4));
}

// ACK change of guildmaster
static void intif_parse_GuildMasterChanged(int fd)
{
	guild->gm_changed(RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10));
}

// Request pet creation
static void intif_parse_CreatePet(int fd)
{
	pet->get_egg(RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
}

// ACK pet data
static void intif_parse_RecvPetData(int fd)
{
	struct s_pet p;
	int len;
	len=RFIFOW(fd,2);
	if (sizeof(struct s_pet) != len-9) {
		if (battle_config.etc_log)
			ShowError("intif: pet data: data size mismatch %d != %"PRIuS"\n", len-9, sizeof(struct s_pet));
	} else {
		memcpy(&p,RFIFOP(fd,9),sizeof(struct s_pet));
		pet->recv_petdata(RFIFOL(fd,4),&p,RFIFOB(fd,8));
	}
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
// ACK pet save data
static void intif_parse_SavePetOk(int fd)
{
	if(RFIFOB(fd,6) == 1)
		ShowError("pet data save failure\n");
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
// ACK deleting pet
static void intif_parse_DeletePetOk(int fd)
{
	if(RFIFOB(fd,2) == 1)
		ShowError("pet data delete failure\n");
}

// ACK changing name request, players,pets,homun
static void intif_parse_ChangeNameOk(int fd)
{
	struct map_session_data *sd = NULL;
	if((sd=map->id2sd(RFIFOL(fd,2)))==NULL ||
		sd->status.char_id != RFIFOL(fd,6))
		return;

	switch (RFIFOB(fd,10)) {
	case 0: //Players [NOT SUPPORTED YET]
		break;
	case 1: //Pets
		pet->change_name_ack(sd, RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	case 2: //Hom
		homun->change_name_ack(sd, RFIFOP(fd,12), RFIFOB(fd,11));
		break;
	}
	return;
}

//----------------------------------------------------------------
// Homunculus recv packets [albator]

static void intif_parse_CreateHomunculus(int fd)
{
	int len = RFIFOW(fd,2)-9;
	if (sizeof(struct s_homunculus) != len) {
		if (battle_config.etc_log)
			ShowError("intif: create homun data: data size mismatch %d != %"PRIuS"\n", len, sizeof(struct s_homunculus));
		return;
	}
	homun->recv_data(RFIFOL(fd,4), RFIFOP(fd,9), RFIFOB(fd,8)) ;
}

static void intif_parse_RecvHomunculusData(int fd)
{
	int len = RFIFOW(fd,2)-9;

	if (sizeof(struct s_homunculus) != len) {
		if (battle_config.etc_log)
			ShowError("intif: homun data: data size mismatch %d != %"PRIuS"\n", len, sizeof(struct s_homunculus));
		return;
	}
	homun->recv_data(RFIFOL(fd,4), RFIFOP(fd,9), RFIFOB(fd,8));
}

/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
static void intif_parse_SaveHomunculusOk(int fd)
{
	if(RFIFOB(fd,6) != 1)
		ShowError("homunculus data save failure for account %u\n", RFIFOL(fd,2));
}

/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
static void intif_parse_DeleteHomunculusOk(int fd)
{
	if(RFIFOB(fd,2) != 1)
		ShowError("Homunculus data delete failure\n");
}

/***************************************
 * ACHIEVEMENT SYSTEM FUNCTIONS
 ***************************************/
/**
 * Sends a request to the inter-server to load
 * and send a character's achievement data.
 * @packet [out] 0x3012 <char_id>.L
 * @param sd pointer to map_session_data.
 */
static void intif_achievements_request(struct map_session_data *sd)
{
	nullpo_retv(sd);

	if (intif->CheckForCharServer())
		return;

	WFIFOHEAD(inter_fd, 6);
	WFIFOW(inter_fd, 0) = 0x3012;
	WFIFOL(inter_fd, 2) = sd->status.char_id;
	WFIFOSET(inter_fd, 6);
}

/**
 * Handles reception of achievement data for a character from the inter-server.
 * @packet [in] 0x3810 <packet_len>.W <char_id>.L <char_achievements[]>.P
 * @param fd socket descriptor.
 */
static void intif_parse_achievements_load(int fd)
{
	int size = RFIFOW(fd, 2);
	int char_id = RFIFOL(fd, 4);
	int payload_count = (size - 8) / sizeof(struct achievement);
	struct map_session_data *sd = map->charid2sd(char_id);
	int i = 0;

	if (sd == NULL) {
		ShowError("intif_parse_achievements_load: Parse request for achievements received but character is offline!\n");
		return;
	}

	if (VECTOR_LENGTH(sd->achievement) > 0) {
		ShowError("intif_parse_achievements_load: Achievements already loaded! Possible multiple calls from the inter-server received.\n");
		return;
	}

	VECTOR_ENSURE(sd->achievement, payload_count, 1);

	for (i = 0; i < payload_count; i++) {
		struct achievement t_ach = { 0 };

		memcpy(&t_ach, RFIFOP(fd, 8 + i * sizeof(struct achievement)), sizeof(struct achievement));

		if (achievement->get(t_ach.id) == NULL) {
			ShowError("intif_parse_achievements_load: Invalid Achievement %d received from character %d. Ignoring...\n", t_ach.id, char_id);
			continue;
		}

		VECTOR_PUSH(sd->achievement, t_ach);
	}

	achievement->init_titles(sd);
	clif->achievement_send_list(fd, sd);
	sd->achievements_received = true;
}

/**
 * Send character's achievement data to the inter-server.
 * @packet 0x3013 <packet_len>.W <char_id>.L <achievements[]>.P
 * @param sd pointer to map session data.
 */
static void intif_achievements_save(struct map_session_data *sd)
{
	int packet_len = 0, payload_size = 0, i = 0;

	nullpo_retv(sd);
	/* check for character server. */
	if (intif->CheckForCharServer())
		return;
	/* Return if no data. */
	if (!(payload_size = VECTOR_LENGTH(sd->achievement) * sizeof(struct achievement)))
		return;

	packet_len = payload_size + 8;

	WFIFOHEAD(inter_fd, packet_len);
	WFIFOW(inter_fd, 0) = 0x3013;
	WFIFOW(inter_fd, 2) = packet_len;
	WFIFOL(inter_fd, 4) = sd->status.char_id;
	for (i = 0; i < VECTOR_LENGTH(sd->achievement); i++)
		memcpy(WFIFOP(inter_fd, 8 + i * sizeof(struct achievement)), &VECTOR_INDEX(sd->achievement, i), sizeof(struct achievement));
	WFIFOSET(inter_fd, packet_len);
}

/**************************************
 *     QUESTLOG SYSTEM FUNCTIONS      *
 **************************************/

/**
 * Requests a character's quest log entries to the inter server.
 *
 * @param sd Character's data
 */
static void intif_request_questlog(struct map_session_data *sd)
{
	nullpo_retv(sd);
	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3060;
	WFIFOL(inter_fd,2) = sd->status.char_id;
	WFIFOSET(inter_fd,6);
}

/**
 * Parses the received quest log entries for a character from the inter server.
 *
 * Received in reply to the requests made by intif_request_questlog.
 *
 * @see intif_parse
 */
static void intif_parse_QuestLog(int fd)
{
	int char_id = RFIFOL(fd, 4), num_received = (RFIFOW(fd, 2)-8)/sizeof(struct quest);
	struct map_session_data *sd = map->charid2sd(char_id);

	if (!sd) // User not online anymore
		return;

	sd->num_quests = sd->avail_quests = 0;

	if (num_received == 0) {
		if (sd->quest_log) {
			aFree(sd->quest_log);
			sd->quest_log = NULL;
		}
	} else {
		const struct quest *received = RFIFOP(fd, 8);
		int i, k = num_received;
		if (sd->quest_log) {
			RECREATE(sd->quest_log, struct quest, num_received);
		} else {
			CREATE(sd->quest_log, struct quest, num_received);
		}

		for (i = 0; i < num_received; i++) {
			if( quest->db(received[i].quest_id) == &quest->dummy ) {
				ShowError("intif_parse_QuestLog: quest %d not found in DB.\n", received[i].quest_id);
				continue;
			}
			if (received[i].state != Q_COMPLETE) {
				// Insert at the beginning
				memcpy(&sd->quest_log[sd->avail_quests++], &received[i], sizeof(struct quest));
			} else {
				// Insert at the end
				memcpy(&sd->quest_log[--k], &received[i], sizeof(struct quest));
			}
			sd->num_quests++;
		}
		if (sd->avail_quests < k) {
			// sd->avail_quests and k didn't meet in the middle: some entries were skipped
			if (k < num_received) // Move the entries at the end to fill the gap
				memmove(&sd->quest_log[k], &sd->quest_log[sd->avail_quests], sizeof(struct quest)*(num_received - k));
			sd->quest_log = aRealloc(sd->quest_log, sizeof(struct quest)*sd->num_quests);
		}
	}

	quest->pc_login(sd);
}

/**
 * Parses the quest log save ack for a character from the inter server.
 *
 * Received in reply to the requests made by intif_quest_save.
 *
 * @see intif_parse
 */
static void intif_parse_QuestSave(int fd)
{
	int cid = RFIFOL(fd, 2);
	struct map_session_data *sd = map->id2sd(cid);

	if( !RFIFOB(fd, 6) )
		ShowError("intif_parse_QuestSave: Failed to save quest(s) for character %d!\n", cid);
	else if( sd )
		sd->save_quest = false;
}

/**
 * Requests to the inter server to save a character's quest log entries.
 *
 * @param sd Character's data
 * @return 0 in case of success, nonzero otherwise
 */
static int intif_quest_save(struct map_session_data *sd)
{
	int len = sizeof(struct quest)*sd->num_quests + 8;

	if(intif->CheckForCharServer())
		return 1;

	WFIFOHEAD(inter_fd, len);
	WFIFOW(inter_fd,0) = 0x3061;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = sd->status.char_id;
	if( sd->num_quests )
		memcpy(WFIFOP(inter_fd,8), sd->quest_log, sizeof(struct quest)*sd->num_quests);
	WFIFOSET(inter_fd,  len);

	return 0;
}

/*==========================================
 * MAIL SYSTEM
 * By Zephyrus
 *==========================================*/

/*------------------------------------------
 * Inbox Request
 * flag: 0 Update Inbox | 1 OpenMail
 *------------------------------------------*/
static int intif_Mail_requestinbox(int char_id, unsigned char flag)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,7);
	WFIFOW(inter_fd,0) = 0x3048;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOB(inter_fd,6) = flag;
	WFIFOSET(inter_fd,7);

	return 0;
}

static void intif_parse_MailInboxReceived(int fd)
{
	struct map_session_data *sd;
	unsigned char flag = RFIFOB(fd,8);

	sd = map->charid2sd(RFIFOL(fd,4));

	if (sd == NULL) /** user is not online anymore and its ok (quest log also does this) **/
		return;

	if (RFIFOW(fd,2) - 9 != sizeof(struct mail_data)) {
		ShowError("intif_parse_MailInboxReceived: data size mismatch %d != %"PRIuS"\n", RFIFOW(fd,2) - 9, sizeof(struct mail_data));
		return;
	}

	//FIXME: this operation is not safe [ultramage]
	memcpy(&sd->mail.inbox, RFIFOP(fd,9), sizeof(struct mail_data));
	sd->mail.changed = false; // cache is now in sync

	if (flag)
		clif->mail_refreshinbox(sd);
	else if( battle_config.mail_show_status && ( battle_config.mail_show_status == 1 || sd->mail.inbox.unread ) ) {
		char output[128];
		sprintf(output, msg_sd(sd,510), sd->mail.inbox.unchecked, sd->mail.inbox.unread + sd->mail.inbox.unchecked);
		clif_disp_onlyself(sd, output);
	}
}
/*------------------------------------------
 * Mail Read
 *------------------------------------------*/
static int intif_Mail_read(int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3049;
	WFIFOL(inter_fd,2) = mail_id;
	WFIFOSET(inter_fd,6);

	return 0;
}
/*------------------------------------------
 * Get Attachment
 *------------------------------------------*/
static int intif_Mail_getattach(int char_id, int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304a;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd, 10);

	return 0;
}

static void intif_parse_MailGetAttach(int fd)
{
	struct map_session_data *sd;
	struct item item;
	int zeny = RFIFOL(fd,8);

	Assert_retv(zeny >= 0);
	sd = map->charid2sd( RFIFOL(fd,4) );

	if (sd == NULL) {
		ShowError("intif_parse_MailGetAttach: char not found %u\n", RFIFOL(fd,4));
		return;
	}

	if (RFIFOW(fd,2) - 12 != sizeof(struct item)) {
		ShowError("intif_parse_MailGetAttach: data size mismatch %d != %"PRIuS"\n", RFIFOW(fd,2) - 16, sizeof(struct item));
		return;
	}

	memcpy(&item, RFIFOP(fd,12), sizeof(struct item));

	mail->getattachment(sd, zeny, &item);
}
/*------------------------------------------
 * Delete Message
 *------------------------------------------*/
static int intif_Mail_delete(int char_id, int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304b;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

static void intif_parse_MailDelete(int fd)
{
	struct map_session_data *sd;
	int char_id = RFIFOL(fd,2);
	int mail_id = RFIFOL(fd,6);
	bool failed = RFIFOB(fd,10);

	if ( (sd = map->charid2sd(char_id)) == NULL) {
		ShowError("intif_parse_MailDelete: char not found %d\n", char_id);
		return;
	}

	if (!failed) {
		int i;
		ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
		if( i < MAIL_MAX_INBOX ) {
			memset(&sd->mail.inbox.msg[i], 0, sizeof(struct mail_message));
			sd->mail.inbox.amount--;
		}

		if( sd->mail.inbox.full )
			intif->Mail_requestinbox(sd->status.char_id, 1); // Free space is available for new mails
	}

	clif->mail_delete(sd->fd, mail_id, failed);
}
/*------------------------------------------
 * Return Message
 *------------------------------------------*/
static int intif_Mail_return(int char_id, int mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x304c;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = mail_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

static void intif_parse_MailReturn(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	short fail = RFIFOB(fd,10);

	if( sd == NULL ) {
		ShowError("intif_parse_MailReturn: char not found %u\n", RFIFOL(fd, 2));
		return;
	}

	if( !fail ) {
		int i;
		ARR_FIND(0, MAIL_MAX_INBOX, i, sd->mail.inbox.msg[i].id == mail_id);
		if( i < MAIL_MAX_INBOX ) {
			memset(&sd->mail.inbox.msg[i], 0, sizeof(struct mail_message));
			sd->mail.inbox.amount--;
		}

		if( sd->mail.inbox.full )
			intif->Mail_requestinbox(sd->status.char_id, 1); // Free space is available for new mails
	}

	clif->mail_return(sd->fd, mail_id, fail);
}
/*------------------------------------------
 * Send Mail
 *------------------------------------------*/
static int intif_Mail_send(int account_id, struct mail_message *msg)
{
	int len = sizeof(struct mail_message) + 8;

	if (intif->CheckForCharServer())
		return 0;

	nullpo_ret(msg);
	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x304d;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = account_id;
	memcpy(WFIFOP(inter_fd,8), msg, sizeof(struct mail_message));
	WFIFOSET(inter_fd,len);

	return 1;
}

static void intif_parse_MailSend(int fd)
{
	struct mail_message msg;
	struct map_session_data *sd;
	bool fail;

	if( RFIFOW(fd,2) - 4 != sizeof(struct mail_message) ) {
		ShowError("intif_parse_MailSend: data size mismatch %d != %"PRIuS"\n", RFIFOW(fd,2) - 4, sizeof(struct mail_message));
		return;
	}

	memcpy(&msg, RFIFOP(fd,4), sizeof(struct mail_message));
	fail = (msg.id == 0);

	// notify sender
	sd = map->charid2sd(msg.send_id);
	if( sd != NULL ) {
		if( fail )
			mail->deliveryfail(sd, &msg);
		else {
			clif->mail_send(sd->fd, false);
			if( map->save_settings&16 )
				chrif->save(sd, 0);
		}
	}
}

static void intif_parse_MailNew(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int mail_id = RFIFOL(fd,6);
	const char *sender_name = RFIFOP(fd,10);
	const char *title = RFIFOP(fd,34);

	if( sd == NULL )
		return;

	sd->mail.changed = true;
	clif->mail_new(sd->fd, mail_id, sender_name, title);
}

/*==========================================
 * AUCTION SYSTEM
 * By Zephyrus
 *==========================================*/
static int intif_Auction_requestlist(int char_id, short type, int price, const char *searchtext, short page)
{
	int len = NAME_LENGTH + 16;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(searchtext);
	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3050;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOW(inter_fd,8) = type;
	WFIFOL(inter_fd,10) = price;
	WFIFOW(inter_fd,14) = page;
	memcpy(WFIFOP(inter_fd,16), searchtext, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 0;
}

static void intif_parse_AuctionResults(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,4));
	short count = RFIFOW(fd,8);
	short pages = RFIFOW(fd,10);
	const uint8 *data = RFIFOP(fd,12);

	if( sd == NULL )
		return;

	clif->auction_results(sd, count, pages, data);
}

static int intif_Auction_register(struct auction_data *auction)
{
	int len = sizeof(struct auction_data) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(auction);
	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3051;
	WFIFOW(inter_fd,2) = len;
	memcpy(WFIFOP(inter_fd,4), auction, sizeof(struct auction_data));
	WFIFOSET(inter_fd,len);

	return 1;
}

static void intif_parse_AuctionRegister(int fd)
{
	struct map_session_data *sd;
	struct auction_data auction;

	if (RFIFOW(fd,2) - 4 != sizeof(struct auction_data)) {
		ShowError("intif_parse_AuctionRegister: data size mismatch %d != %"PRIuS"\n", RFIFOW(fd,2) - 4, sizeof(struct auction_data));
		return;
	}

	memcpy(&auction, RFIFOP(fd,4), sizeof(struct auction_data));
	if( (sd = map->charid2sd(auction.seller_id)) == NULL )
		return;

	if( auction.auction_id > 0 ) {
		clif->auction_message(sd->fd, 1); // Confirmation Packet ??
		if( map->save_settings&32 )
			chrif->save(sd,0);
	} else {
		int zeny = auction.hours*battle_config.auction_feeperhour;

		clif->auction_message(sd->fd, 4);
		pc->additem(sd, &auction.item, auction.item.amount, LOG_TYPE_AUCTION);

		pc->getzeny(sd, zeny, LOG_TYPE_AUCTION, NULL);
	}
}

static int intif_Auction_cancel(int char_id, unsigned int auction_id)
{
	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3052;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = auction_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

static void intif_parse_AuctionCancel(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	switch( result ) {
		case 0: clif->auction_message(sd->fd, 2); break;
		case 1: clif->auction_close(sd->fd, 2); break;
		case 2: clif->auction_close(sd->fd, 1); break;
		case 3: clif->auction_message(sd->fd, 3); break;
	}
}

static int intif_Auction_close(int char_id, unsigned int auction_id)
{
	if( intif->CheckForCharServer() )
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3053;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = auction_id;
	WFIFOSET(inter_fd,10);

	return 0;
}

static void intif_parse_AuctionClose(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	unsigned char result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	clif->auction_close(sd->fd, result);
	if( result == 0 ) {
		// FIXME: Leeching off a parse function
		clif->pAuction_cancelreg(fd, sd);
		intif->Auction_requestlist(sd->status.char_id, 6, 0, "", 1);
	}
}

static int intif_Auction_bid(int char_id, const char *name, unsigned int auction_id, int bid)
{
	int len = 16 + NAME_LENGTH;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(name);
	WFIFOHEAD(inter_fd,len);
	WFIFOW(inter_fd,0) = 0x3055;
	WFIFOW(inter_fd,2) = len;
	WFIFOL(inter_fd,4) = char_id;
	WFIFOL(inter_fd,8) = auction_id;
	WFIFOL(inter_fd,12) = bid;
	memcpy(WFIFOP(inter_fd,16), name, NAME_LENGTH);
	WFIFOSET(inter_fd,len);

	return 0;
}

static void intif_parse_AuctionBid(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	int bid = RFIFOL(fd,6);
	unsigned char result = RFIFOB(fd,10);

	if( sd == NULL )
		return;

	clif->auction_message(sd->fd, result);
	if( bid > 0 ) {
		pc->getzeny(sd, bid, LOG_TYPE_AUCTION,NULL);
	}
	if( result == 1 ) { // To update the list, display your buy list
		clif->pAuction_cancelreg(fd, sd);
		intif->Auction_requestlist(sd->status.char_id, 7, 0, "", 1);
	}
}

// Used to send 'You have won the auction' and 'You failed to won the auction' messages
static void intif_parse_AuctionMessage(int fd)
{
	struct map_session_data *sd = map->charid2sd(RFIFOL(fd,2));
	unsigned char result = RFIFOB(fd,6);

	if( sd == NULL )
		return;

	clif->auction_message(sd->fd, result);
}

/*==========================================
 * Mercenary's System
 *------------------------------------------*/
static int intif_mercenary_create(struct s_mercenary *merc)
{
	int size = sizeof(struct s_mercenary) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(merc);
	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x3070;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), merc, sizeof(struct s_mercenary));
	WFIFOSET(inter_fd,size);
	return 0;
}

static void intif_parse_MercenaryReceived(int fd)
{
	int len = RFIFOW(fd,2) - 5;

	if (sizeof(struct s_mercenary) != len) {
		if (battle_config.etc_log)
			ShowError("intif: create mercenary data size mismatch %d != %"PRIuS"\n", len, sizeof(struct s_mercenary));
		return;
	}

	mercenary->data_received(RFIFOP(fd,5), RFIFOB(fd,4));
}

static int intif_mercenary_request(int merc_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x3071;
	WFIFOL(inter_fd,2) = merc_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 0;
}

static int intif_mercenary_delete(int merc_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x3072;
	WFIFOL(inter_fd,2) = merc_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
static void intif_parse_MercenaryDeleted(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Mercenary data delete failure\n");
}

static int intif_mercenary_save(struct s_mercenary *merc)
{
	int size = sizeof(struct s_mercenary) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(merc);
	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x3073;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), merc, sizeof(struct s_mercenary));
	WFIFOSET(inter_fd,size);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
static void intif_parse_MercenarySaved(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Mercenary data save failure\n");
}

/*==========================================
 * Elemental's System
 *------------------------------------------*/
static int intif_elemental_create(struct s_elemental *ele)
{
	int size = sizeof(struct s_elemental) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(ele);
	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x307c;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), ele, sizeof(struct s_elemental));
	WFIFOSET(inter_fd,size);
	return 0;
}

static void intif_parse_ElementalReceived(int fd)
{
	int len = RFIFOW(fd,2) - 5;

	if (sizeof(struct s_elemental) != len) {
		if (battle_config.etc_log)
			ShowError("intif: create elemental data size mismatch %d != %"PRIuS"\n", len, sizeof(struct s_elemental));
		return;
	}

	elemental->data_received(RFIFOP(fd,5), RFIFOB(fd,4));
}

static int intif_elemental_request(int ele_id, int char_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,10);
	WFIFOW(inter_fd,0) = 0x307d;
	WFIFOL(inter_fd,2) = ele_id;
	WFIFOL(inter_fd,6) = char_id;
	WFIFOSET(inter_fd,10);
	return 0;
}

static int intif_elemental_delete(int ele_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd,6);
	WFIFOW(inter_fd,0) = 0x307e;
	WFIFOL(inter_fd,2) = ele_id;
	WFIFOSET(inter_fd,6);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
static void intif_parse_ElementalDeleted(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Elemental data delete failure\n");
}

static int intif_elemental_save(struct s_elemental *ele)
{
	int size = sizeof(struct s_elemental) + 4;

	if( intif->CheckForCharServer() )
		return 0;

	nullpo_ret(ele);
	WFIFOHEAD(inter_fd,size);
	WFIFOW(inter_fd,0) = 0x307f;
	WFIFOW(inter_fd,2) = size;
	memcpy(WFIFOP(inter_fd,4), ele, sizeof(struct s_elemental));
	WFIFOSET(inter_fd,size);
	return 0;
}
/* Really? Whats the point, shouldn't be sent when successful then [Ind] */
static void intif_parse_ElementalSaved(int fd)
{
	if( RFIFOB(fd,2) != 1 )
		ShowError("Elemental data save failure\n");
}

static void intif_request_accinfo(int u_fd, int aid, int group_lv, char *query)
{
	nullpo_retv(query);

	WFIFOHEAD(inter_fd,2 + 4 + 4 + 4 + NAME_LENGTH);
	WFIFOW(inter_fd,0) = 0x3007;
	WFIFOL(inter_fd,2) = u_fd;
	WFIFOL(inter_fd,6) = aid;
	WFIFOL(inter_fd,10) = group_lv;
	safestrncpy(WFIFOP(inter_fd,14), query, NAME_LENGTH);

	WFIFOSET(inter_fd,2 + 4 + 4 + 4 + NAME_LENGTH);

	return;
}

static void intif_parse_MessageToFD(int fd)
{
	int u_fd = RFIFOL(fd,4);

	Assert_retv(sockt->session_is_valid(u_fd));
	if( sockt->session[u_fd] && sockt->session[u_fd]->session_data ) {
		int aid = RFIFOL(fd,8);
		struct map_session_data * sd = sockt->session[u_fd]->session_data;
		/* matching e.g. previous fd owner didn't dc during request or is still the same */
		if( sd && sd->bl.id == aid ) {
			char msg[512];
			safestrncpy(msg, RFIFOP(fd,12), RFIFOW(fd,2) - 12);
			clif->messagecolor_self(u_fd, COLOR_DEFAULT ,msg);
		}

	}

	return;
}
/*==========================================
 * Item Bound System [Xantara][Mhalicot]
 *------------------------------------------*/
static void intif_itembound_req(int char_id, int aid, int guild_id)
{
#ifdef GP_BOUND_ITEMS
	struct guild_storage *gstor = idb_get(gstorage->db,guild_id);
	WFIFOHEAD(inter_fd,12);
	WFIFOW(inter_fd,0) = 0x3056;
	WFIFOL(inter_fd,2) = char_id;
	WFIFOL(inter_fd,6) = aid;
	WFIFOW(inter_fd,10) = guild_id;
	WFIFOSET(inter_fd,12);
	if(gstor)
		gstor->lock = 1; //Lock for retrieval process
#endif
}

//3856
static void intif_parse_Itembound_ack(int fd)
{
#ifdef GP_BOUND_ITEMS
	struct guild_storage *gstor;
	int guild_id = RFIFOW(fd,6);

	gstor = idb_get(gstorage->db,guild_id);
	if(gstor)
		gstor->lock = 0; //Unlock now that operation is completed
#endif
}

/*==========================================
 * RoDEX System
 *==========================================*/

/*------------------------------------------
 * Mail List
 *------------------------------------------*/

// Rodex Inbox Request
// char_id: char_id
// account_id: account_id (used by account mail)
// flag: 0 - Open/Refresh ; 1 = Next Page
static int intif_rodex_requestinbox(int char_id, int account_id, int8 flag, int8 opentype, int64 mail_id)
{
	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd, 20);
	WFIFOW(inter_fd, 0) = 0x3095;
	WFIFOL(inter_fd, 2) = char_id;
	WFIFOL(inter_fd, 6) = account_id;
	WFIFOL(inter_fd, 10) = flag;
	WFIFOB(inter_fd, 11) = opentype;
	WFIFOQ(inter_fd, 12) = mail_id;
	WFIFOSET(inter_fd, 20);

	return 0;
}

static void intif_parse_RequestRodexOpenInbox(int fd)
{
	struct map_session_data *sd;
#if PACKETVER < 20170419
	int8 opentype = RFIFOB(fd, 8);
#endif
	int8 flag = RFIFOB(fd, 9);
	int8 is_end = RFIFOB(fd, 10);
	int is_first = RFIFOB(fd, 11);
	int count = RFIFOL(fd, 12);
#if PACKETVER >= 20170419
	int64 mail_id = RFIFOQ(fd, 16);
#endif
	int i, j;

	sd = map->charid2sd(RFIFOL(fd, 4));

	if (sd == NULL) // user is not online anymore
		return;

	if (is_first == false && sd->rodex.total == 0) {
		ShowError("intif_parse_RodexInboxOpenReceived: mail list received in wrong order.\n");
		return;
	}

	if (is_first)
		sd->rodex.total = count;
	else
		sd->rodex.total += count;

	if (RFIFOW(fd, 2) - 24 != count * sizeof(struct rodex_message)) {
		ShowError("intif_parse_RodexInboxOpenReceived: data size mismatch %d != %"PRIuS"\n", RFIFOW(fd, 2) - 24, count * sizeof(struct rodex_message));
		return;
	}

	if (flag == 0 && is_first)
		VECTOR_CLEAR(sd->rodex.messages);

	for (i = 0, j = 24; i < count; ++i, j += sizeof(struct rodex_message)) {
		struct rodex_message msg = { 0 };
		VECTOR_ENSURE(sd->rodex.messages, 1, 1);
		memcpy(&msg, RFIFOP(fd, j), sizeof(struct rodex_message));
		VECTOR_PUSH(sd->rodex.messages, msg);
	}

	if (is_end == true) {
#if PACKETVER >= 20170419
		clif->rodex_send_mails_all(sd->fd, sd, mail_id);
#else
		if (flag == 0)
			clif->rodex_send_maillist(sd->fd, sd, opentype, VECTOR_LENGTH(sd->rodex.messages) - 1);
		else
			clif->rodex_send_refresh(sd->fd, sd, opentype, count);
#endif
	}
}

/*------------------------------------------
 * Notifications
 *------------------------------------------*/
static int intif_rodex_hasnew(struct map_session_data *sd)
{
	nullpo_retr(0, sd);

	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd, 10);
	WFIFOW(inter_fd, 0) = 0x3096;
	WFIFOL(inter_fd, 2) = sd->status.char_id;
	WFIFOL(inter_fd, 6) = sd->status.account_id;
	WFIFOSET(inter_fd, 10);

	return 0;
}

static void intif_parse_RodexNotifications(int fd)
{
	struct map_session_data *sd;
	bool has_messages;

	sd = map->charid2sd(RFIFOL(fd, 2));
	has_messages = RFIFOB(fd, 6);

	if (sd == NULL) // user is not online anymore
		return;

	clif->rodex_icon(sd->fd, has_messages);
}

/*------------------------------------------
 * Update Mail
 *------------------------------------------*/

/// Updates a mail
/// flag:
///     0 - receiver Read
///     1 - user got Zeny
///     2 - user got Items
///     3 - delete
///     4 - sender Read (returned mail)
static int intif_rodex_updatemail(struct map_session_data *sd, int64 mail_id, uint8 opentype, int8 flag)
{
	nullpo_ret(sd);

	if (intif->CheckForCharServer())
		return 0;

	WFIFOHEAD(inter_fd, 20);
	WFIFOW(inter_fd, 0) = 0x3097;
	WFIFOL(inter_fd, 2) = sd->status.account_id;
	WFIFOL(inter_fd, 6) = sd->status.char_id;
	WFIFOQ(inter_fd, 10) = mail_id;
	WFIFOB(inter_fd, 18) = opentype;
	WFIFOB(inter_fd, 19) = flag;
	WFIFOSET(inter_fd, 20);

	return 0;
}

/*------------------------------------------
 * Send Mail
 *------------------------------------------*/
static int intif_rodex_sendmail(struct rodex_message *msg)
{
	if (intif->CheckForCharServer())
		return 0;

	nullpo_retr(0, msg);

	WFIFOHEAD(inter_fd, 4 + sizeof(struct rodex_message));
	WFIFOW(inter_fd, 0) = 0x3098;
	WFIFOW(inter_fd, 2) = 4 + sizeof(struct rodex_message);
	memcpy(WFIFOP(inter_fd, 4), msg, sizeof(struct rodex_message));
	WFIFOSET(inter_fd, 4 + sizeof(struct rodex_message));

	return 0;
}

static void intif_parse_RodexSendMail(int fd)
{
	struct map_session_data *ssd = NULL, *rsd = NULL;
	int sender_id = RFIFOL(fd, 2);
	int receiver_id = RFIFOL(fd, 6);
	int receiver_accountid = RFIFOL(fd, 10);

	if (sender_id > 0)
		ssd = map->charid2sd(sender_id);

	if (receiver_id > 0)
		rsd = map->charid2sd(receiver_id);
	else if (receiver_accountid > 0)
		rsd = map->id2sd(receiver_accountid);

	rodex->send_mail_result(ssd, rsd, RFIFOL(fd, 14));
}

/*------------------------------------------
 * Check Player
 *------------------------------------------*/
static int intif_rodex_checkname(struct map_session_data *sd, const char *name)
{
	if (intif->CheckForCharServer())
		return 0;

	nullpo_retr(0, sd);
	nullpo_retr(0, name);

	WFIFOHEAD(inter_fd, 6 + NAME_LENGTH);
	WFIFOW(inter_fd, 0) = 0x3099;
	WFIFOL(inter_fd, 2) = sd->status.char_id;
	safestrncpy(WFIFOP(inter_fd, 6), name, NAME_LENGTH);
	WFIFOSET(inter_fd, 6 + NAME_LENGTH);

	return 0;
}

static void intif_parse_RodexCheckName(int fd)
{
	struct map_session_data *sd = NULL;
	int reqchar_id = RFIFOL(fd, 2);
	int target_char_id = RFIFOL(fd, 6);
	int target_class = RFIFOL(fd, 10);
	int target_level = RFIFOL(fd, 14);
	char name[NAME_LENGTH];

	safestrncpy(name, RFIFOP(inter_fd, 18), NAME_LENGTH);

	if (reqchar_id <= 0)
		return;

	sd = map->charid2sd(reqchar_id);

	if (sd == NULL) // User is not online anymore
		return;

	if (target_char_id == 0) {
		clif->rodex_checkname_result(sd, 0, 0, 0, name);
		return;
	}

	sd->rodex.tmp.receiver_id = target_char_id;
	safestrncpy(sd->rodex.tmp.receiver_name, name, NAME_LENGTH);

	clif->rodex_checkname_result(sd, target_char_id, target_class, target_level, name);
}

static void intif_parse_GetZenyAck(int fd)
{
	int char_id = RFIFOL(fd, 2);
	int64 zeny = RFIFOL(fd, 6);
	int64 mail_id = RFIFOQ(fd, 14);
	uint8 opentype = RFIFOB(fd, 22);
	struct map_session_data *sd = map->charid2sd(char_id);

	if (sd == NULL) // User is not online anymore
		return;
	rodex->getZenyAck(sd, mail_id, opentype, zeny);
}

static void intif_parse_GetItemsAck(int fd)
{
	int char_id = RFIFOL(fd, 2);

	struct map_session_data *sd = map->charid2sd(char_id);
	if (sd == NULL) // User is not online anymore
		return;

	int64 mail_id = RFIFOQ(fd, 6);
	uint8 opentype = RFIFOB(fd, 14);
	int count = RFIFOB(fd, 15);
	struct rodex_item items[RODEX_MAX_ITEM];
	memcpy(&items[0], RFIFOP(fd, 16), sizeof(struct rodex_item) * RODEX_MAX_ITEM);
	rodex->getItemsAck(sd, mail_id, opentype, count, &items[0]);
}

//-----------------------------------------------------------------
// Communication from the inter server
// Return a 0 (false) if there were any errors.
// 1, 2 if there are not enough to return the length of the packet if the packet processing
static int intif_parse(int fd)
{
	int packet_len, cmd;
	cmd = RFIFOW(fd,0);
	// Verify ID of the packet
	if (cmd < 0x3800 || cmd >= 0x3800+(sizeof(intif->packet_len_table)/sizeof(intif->packet_len_table[0]))
	 || intif->packet_len_table[cmd-0x3800] == 0
	) {
		return 0;
	}
	// Check the length of the packet
	packet_len = intif->packet_len_table[cmd-0x3800];
	if(packet_len==-1){
		if(RFIFOREST(fd)<4)
			return 2;
		packet_len = RFIFOW(fd,2);
	}
	if((int)RFIFOREST(fd)<packet_len){
		return 2;
	}
	// Processing branch
	switch(cmd){
		case 0x3804: intif->pRegisters(fd); break;
		case 0x3805: intif->pAccountStorage(fd); break;
		case 0x3806: intif->pChangeNameOk(fd); break;
		case 0x3807: intif->pMessageToFD(fd); break;
		case 0x3808: intif->pAccountStorageSaveAck(fd); break;
		case 0x3810: intif->pAchievementsLoad(fd); break;
		case 0x3818: intif->pLoadGuildStorage(fd); break;
		case 0x3819: intif->pSaveGuildStorage(fd); break;
		case 0x3820: intif->pPartyCreated(fd); break;
		case 0x3821: intif->pPartyInfo(fd); break;
		case 0x3822: intif->pPartyMemberAdded(fd); break;
		case 0x3823: intif->pPartyOptionChanged(fd); break;
		case 0x3824: intif->pPartyMemberWithdraw(fd); break;
		case 0x3825: intif->pPartyMove(fd); break;
		case 0x3826: intif->pPartyBroken(fd); break;
		case 0x3830: intif->pGuildCreated(fd); break;
		case 0x3831: intif->pGuildInfo(fd); break;
		case 0x3832: intif->pGuildMemberAdded(fd); break;
		case 0x3834: intif->pGuildMemberWithdraw(fd); break;
		case 0x3835: intif->pGuildMemberInfoShort(fd); break;
		case 0x3836: intif->pGuildBroken(fd); break;
		case 0x3839: intif->pGuildBasicInfoChanged(fd); break;
		case 0x383a: intif->pGuildMemberInfoChanged(fd); break;
		case 0x383b: intif->pGuildPosition(fd); break;
		case 0x383c: intif->pGuildSkillUp(fd); break;
		case 0x383d: intif->pGuildAlliance(fd); break;
		case 0x383e: intif->pGuildNotice(fd); break;
		case 0x383f: intif->pGuildEmblem(fd); break;
		case 0x3840: intif->pGuildCastleDataLoad(fd); break;
		case 0x3843: intif->pGuildMasterChanged(fd); break;

		//Quest system
		case 0x3860: intif->pQuestLog(fd); break;
		case 0x3861: intif->pQuestSave(fd); break;

		// Mail System
		case 0x3848: intif->pMailInboxReceived(fd); break;
		case 0x3849: intif->pMailNew(fd); break;
		case 0x384a: intif->pMailGetAttach(fd); break;
		case 0x384b: intif->pMailDelete(fd); break;
		case 0x384c: intif->pMailReturn(fd); break;
		case 0x384d: intif->pMailSend(fd); break;
		// Auction System
		case 0x3850: intif->pAuctionResults(fd); break;
		case 0x3851: intif->pAuctionRegister(fd); break;
		case 0x3852: intif->pAuctionCancel(fd); break;
		case 0x3853: intif->pAuctionClose(fd); break;
		case 0x3854: intif->pAuctionMessage(fd); break;
		case 0x3855: intif->pAuctionBid(fd); break;
		//Bound items
		case 0x3856:
#ifdef GP_BOUND_ITEMS
			intif->pItembound_ack(fd);
#else
			ShowWarning("intif_parse: Received 0x3856 with GP_BOUND_ITEMS disabled !!!\n");
#endif
			break;
		// Mercenary System
		case 0x3870: intif->pMercenaryReceived(fd); break;
		case 0x3871: intif->pMercenaryDeleted(fd); break;
		case 0x3872: intif->pMercenarySaved(fd); break;
		// Elemental System
		case 0x387c: intif->pElementalReceived(fd); break;
		case 0x387d: intif->pElementalDeleted(fd); break;
		case 0x387e: intif->pElementalSaved(fd); break;

		case 0x3880: intif->pCreatePet(fd); break;
		case 0x3881: intif->pRecvPetData(fd); break;
		case 0x3882: intif->pSavePetOk(fd); break;
		case 0x3883: intif->pDeletePetOk(fd); break;
		case 0x3890: intif->pCreateHomunculus(fd); break;
		case 0x3891: intif->pRecvHomunculusData(fd); break;
		case 0x3892: intif->pSaveHomunculusOk(fd); break;
		case 0x3893: intif->pDeleteHomunculusOk(fd); break;

		// RoDEX
		case 0x3895: intif->pRequestRodexOpenInbox(fd); break;
		case 0x3896: intif->pRodexHasNew(fd); break;
		case 0x3897: intif->pRodexSendMail(fd); break;
		case 0x3898: intif->pRodexCheckName(fd); break;
		case 0x3899: intif->pGetZenyAck(fd); break;
		case 0x389a: intif->pGetItemsAck(fd); break;

		// Clan System
		case 0x3858: intif->pRecvClanMemberAction(fd); break;
	default:
		ShowError("intif_parse : unknown packet %d %x\n",fd,RFIFOW(fd,0));
		return 0;
	}
	// Skip packet
	RFIFOSKIP(fd,packet_len);
	return 1;
}

/*=====================================
 * Default Functions : intif.h
 * Generated by HerculesInterfaceMaker
 * created by Susu
 *-------------------------------------*/
void intif_defaults(void)
{
	const int packet_len_table [INTIF_PACKET_LEN_TABLE_SIZE] = {
		 0, 0, 0, 0, -1,-1,37,-1,  9, 0, 0, 0,  0, 0,  0, 0, //0x3800-0x380f
		-1, 0, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0, //0x3810 Achievements [Smokexyz/Hercules]
		39,-1,15,15, 14,19, 7, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3820
		10,-1,15, 0, 79,25, 7, 0,  0,-1,-1,-1, 14,67,186,-1, //0x3830
		-1, 0, 0,14,  0, 0, 0, 0, -1,74,-1,11, 11,-1,  0, 0, //0x3840
		-1,-1, 7, 7,  7,11, 8, 0, 10, 0, 0, 0,  0, 0,  0, 0, //0x3850  Auctions [Zephyrus] itembound[Akinari] Clan System[Murilo BiO]
		-1, 7, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3860  Quests [Kevin] [Inkfish]
		-1, 3, 3, 0,  0, 0, 0, 0,  0, 0, 0, 0, -1, 3,  3, 0, //0x3870  Mercenaries [Zephyrus] / Elemental [pakpil]
		14,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0, //0x3880
		-1,-1, 7, 3,  0,-1, 7, 15,18 + NAME_LENGTH, 23, 16 + sizeof(struct rodex_item) * RODEX_MAX_ITEM, 0, 0, 0, 0, 0, //0x3890  Homunculus [albator] / RoDEX [KirieZ]
	};

	intif = &intif_s;

	/* */
	memcpy(intif->packet_len_table,&packet_len_table,sizeof(intif->packet_len_table));

	/* funcs */
	intif->parse = intif_parse;
	intif->create_pet = intif_create_pet;
	intif->saveregistry = intif_saveregistry;
	intif->request_registry = intif_request_registry;
	intif->request_account_storage = intif_request_account_storage;
	intif->send_account_storage = intif_send_account_storage;
	intif->request_guild_storage = intif_request_guild_storage;
	intif->send_guild_storage = intif_send_guild_storage;
	intif->create_party = intif_create_party;
	intif->request_partyinfo = intif_request_partyinfo;
	intif->party_addmember = intif_party_addmember;
	intif->party_changeoption = intif_party_changeoption;
	intif->party_leave = intif_party_leave;
	intif->party_changemap = intif_party_changemap;
	intif->break_party = intif_break_party;
	intif->party_leaderchange = intif_party_leaderchange;
	intif->guild_create = intif_guild_create;
	intif->guild_request_info = intif_guild_request_info;
	intif->guild_addmember = intif_guild_addmember;
	intif->guild_leave = intif_guild_leave;
	intif->guild_memberinfoshort = intif_guild_memberinfoshort;
	intif->guild_break = intif_guild_break;
	intif->guild_change_gm = intif_guild_change_gm;
	intif->guild_change_basicinfo = intif_guild_change_basicinfo;
	intif->guild_change_memberinfo = intif_guild_change_memberinfo;
	intif->guild_position = intif_guild_position;
	intif->guild_skillup = intif_guild_skillup;
	intif->guild_alliance = intif_guild_alliance;
	intif->guild_notice = intif_guild_notice;
	intif->guild_emblem = intif_guild_emblem;
	intif->guild_castle_dataload = intif_guild_castle_dataload;
	intif->guild_castle_datasave = intif_guild_castle_datasave;
	intif->request_petdata = intif_request_petdata;
	intif->save_petdata = intif_save_petdata;
	intif->delete_petdata = intif_delete_petdata;
	intif->rename = intif_rename;
	intif->homunculus_create = intif_homunculus_create;
	intif->homunculus_requestload = intif_homunculus_requestload;
	intif->homunculus_requestsave = intif_homunculus_requestsave;
	intif->homunculus_requestdelete = intif_homunculus_requestdelete;
	/******QUEST SYTEM*******/
	intif->request_questlog = intif_request_questlog;
	intif->quest_save = intif_quest_save;
	// MERCENARY SYSTEM
	intif->mercenary_create = intif_mercenary_create;
	intif->mercenary_request = intif_mercenary_request;
	intif->mercenary_delete = intif_mercenary_delete;
	intif->mercenary_save = intif_mercenary_save;
	// MAIL SYSTEM
	intif->Mail_requestinbox = intif_Mail_requestinbox;
	intif->Mail_read = intif_Mail_read;
	intif->Mail_getattach = intif_Mail_getattach;
	intif->Mail_delete = intif_Mail_delete;
	intif->Mail_return = intif_Mail_return;
	intif->Mail_send = intif_Mail_send;
	// AUCTION SYSTEM
	intif->Auction_requestlist = intif_Auction_requestlist;
	intif->Auction_register = intif_Auction_register;
	intif->Auction_cancel = intif_Auction_cancel;
	intif->Auction_close = intif_Auction_close;
	intif->Auction_bid = intif_Auction_bid;
	// ELEMENTAL SYSTEM
	intif->elemental_create = intif_elemental_create;
	intif->elemental_request = intif_elemental_request;
	intif->elemental_delete = intif_elemental_delete;
	intif->elemental_save = intif_elemental_save;
	// RoDEX
	intif->rodex_requestinbox = intif_rodex_requestinbox;
	intif->rodex_checkhasnew = intif_rodex_hasnew;
	intif->rodex_updatemail = intif_rodex_updatemail;
	intif->rodex_sendmail = intif_rodex_sendmail;
	intif->rodex_checkname = intif_rodex_checkname;
	/* Clan System */
	intif->clan_kickoffline = intif_clan_kickoffline;
	intif->clan_membercount = intif_clan_membercount;
	/* @accinfo */
	intif->request_accinfo = intif_request_accinfo;
	/* */
	intif->CheckForCharServer = CheckForCharServer;
	/* */
	intif->itembound_req = intif_itembound_req;
	/* Achievement System */
	intif->achievements_request = intif_achievements_request;
	intif->achievements_save = intif_achievements_save;
	/* parse functions */
	intif->pRegisters = intif_parse_Registers;
	intif->pChangeNameOk = intif_parse_ChangeNameOk;
	intif->pMessageToFD = intif_parse_MessageToFD;
	intif->pAccountStorage = intif_parse_account_storage;
	intif->pAccountStorageSaveAck = intif_parse_account_storage_save_ack;
	intif->pLoadGuildStorage = intif_parse_LoadGuildStorage;
	intif->pSaveGuildStorage = intif_parse_SaveGuildStorage;
	intif->pPartyCreated = intif_parse_PartyCreated;
	intif->pPartyInfo = intif_parse_PartyInfo;
	intif->pPartyMemberAdded = intif_parse_PartyMemberAdded;
	intif->pPartyOptionChanged = intif_parse_PartyOptionChanged;
	intif->pPartyMemberWithdraw = intif_parse_PartyMemberWithdraw;
	intif->pPartyMove = intif_parse_PartyMove;
	intif->pPartyBroken = intif_parse_PartyBroken;
	intif->pGuildCreated = intif_parse_GuildCreated;
	intif->pGuildInfo = intif_parse_GuildInfo;
	intif->pGuildMemberAdded = intif_parse_GuildMemberAdded;
	intif->pGuildMemberWithdraw = intif_parse_GuildMemberWithdraw;
	intif->pGuildMemberInfoShort = intif_parse_GuildMemberInfoShort;
	intif->pGuildBroken = intif_parse_GuildBroken;
	intif->pGuildBasicInfoChanged = intif_parse_GuildBasicInfoChanged;
	intif->pGuildMemberInfoChanged = intif_parse_GuildMemberInfoChanged;
	intif->pGuildPosition = intif_parse_GuildPosition;
	intif->pGuildSkillUp = intif_parse_GuildSkillUp;
	intif->pGuildAlliance = intif_parse_GuildAlliance;
	intif->pGuildNotice = intif_parse_GuildNotice;
	intif->pGuildEmblem = intif_parse_GuildEmblem;
	intif->pGuildCastleDataLoad = intif_parse_GuildCastleDataLoad;
	intif->pGuildMasterChanged = intif_parse_GuildMasterChanged;
	intif->pQuestLog = intif_parse_QuestLog;
	intif->pQuestSave = intif_parse_QuestSave;
	intif->pMailInboxReceived = intif_parse_MailInboxReceived;
	intif->pMailNew = intif_parse_MailNew;
	intif->pMailGetAttach = intif_parse_MailGetAttach;
	intif->pMailDelete = intif_parse_MailDelete;
	intif->pMailReturn = intif_parse_MailReturn;
	intif->pMailSend = intif_parse_MailSend;
	intif->pAuctionResults = intif_parse_AuctionResults;
	intif->pAuctionRegister = intif_parse_AuctionRegister;
	intif->pAuctionCancel = intif_parse_AuctionCancel;
	intif->pAuctionClose = intif_parse_AuctionClose;
	intif->pAuctionMessage = intif_parse_AuctionMessage;
	intif->pAuctionBid = intif_parse_AuctionBid;
	intif->pItembound_ack = intif_parse_Itembound_ack;
	intif->pMercenaryReceived = intif_parse_MercenaryReceived;
	intif->pMercenaryDeleted = intif_parse_MercenaryDeleted;
	intif->pMercenarySaved = intif_parse_MercenarySaved;
	intif->pElementalReceived = intif_parse_ElementalReceived;
	intif->pElementalDeleted = intif_parse_ElementalDeleted;
	intif->pElementalSaved = intif_parse_ElementalSaved;
	intif->pCreatePet = intif_parse_CreatePet;
	intif->pRecvPetData = intif_parse_RecvPetData;
	intif->pSavePetOk = intif_parse_SavePetOk;
	intif->pDeletePetOk = intif_parse_DeletePetOk;
	intif->pCreateHomunculus = intif_parse_CreateHomunculus;
	intif->pRecvHomunculusData = intif_parse_RecvHomunculusData;
	intif->pSaveHomunculusOk = intif_parse_SaveHomunculusOk;
	intif->pDeleteHomunculusOk = intif_parse_DeleteHomunculusOk;
	/* RoDEX */
	intif->pRequestRodexOpenInbox = intif_parse_RequestRodexOpenInbox;
	intif->pRodexHasNew = intif_parse_RodexNotifications;
	intif->pRodexSendMail = intif_parse_RodexSendMail;
	intif->pRodexCheckName = intif_parse_RodexCheckName;
	intif->pGetZenyAck = intif_parse_GetZenyAck;
	intif->pGetItemsAck = intif_parse_GetItemsAck;
	/* Clan System */
	intif->pRecvClanMemberAction = intif_parse_RecvClanMemberAction;
	/* Achievement System */
	intif->pAchievementsLoad = intif_parse_achievements_load;
}
