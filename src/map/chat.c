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

#include "chat.h"

#include "map/atcommand.h" // msg_sd(sd,)
#include "map/battle.h" // struct battle_config
#include "map/clif.h"
#include "map/map.h"
#include "map/npc.h" // npc_event_do()
#include "map/pc.h"
#include "map/skill.h" // ext_skill_unit_onplace()
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/strlib.h"

#include <stdio.h>
#include <string.h>

struct chat_interface chat_s;
struct chat_interface *chat;

/// Initializes a chatroom object (common functionality for both pc and npc chatrooms).
/// Returns a chatroom object on success, or NULL on failure.
struct chat_data* chat_createchat(struct block_list* bl, const char* title, const char* pass, int limit, bool pub, int trigger, const char* ev, int zeny, int min_level, int max_level)
{
	struct chat_data* cd;
	nullpo_retr(NULL, bl);
	nullpo_retr(NULL, title);
	nullpo_retr(NULL, pass);
	nullpo_retr(NULL, ev);

	/* Given the overhead and the numerous instances (npc allocated or otherwise) wouldn't it be beneficial to have it use ERS? [Ind] */
	CREATE(cd, struct chat_data, 1);

	safestrncpy(cd->title, title, sizeof(cd->title));
	safestrncpy(cd->pass, pass, sizeof(cd->pass));
	cd->pub = pub;
	cd->users = 0;
	cd->limit = min(limit, ARRAYLENGTH(cd->usersd));
	cd->trigger = trigger;
	cd->zeny = zeny;
	cd->min_level = min_level;
	cd->max_level = max_level;
	memset(cd->usersd, 0, sizeof(cd->usersd));
	cd->owner = bl;
	safestrncpy(cd->npc_event, ev, sizeof(cd->npc_event));

	cd->bl.id   = map->get_new_object_id();
	cd->bl.m    = bl->m;
	cd->bl.x    = bl->x;
	cd->bl.y    = bl->y;
	cd->bl.type = BL_CHAT;
	cd->bl.next = cd->bl.prev = NULL;

	if( cd->bl.id == 0 ) {
		aFree(cd);
		return NULL;
	}

	map->addiddb(&cd->bl);

	if( bl->type != BL_NPC )
		cd->kick_list = idb_alloc(DB_OPT_BASE);

	return cd;
}

/*==========================================
 * player chatroom creation
 *------------------------------------------*/
bool chat_createpcchat(struct map_session_data* sd, const char* title, const char* pass, int limit, bool pub) {
	struct chat_data* cd;
	nullpo_ret(sd);
	nullpo_ret(title);
	nullpo_ret(pass);

	if (sd->chat_id != 0)
		return false; //Prevent people abusing the chat system by creating multiple chats, as pointed out by End of Exam. [Skotlex]

	if( sd->state.vending || sd->state.buyingstore )
	{// not chat, when you already have a store open
		return false;
	}

	if( map->list[sd->bl.m].flag.nochat ) {
		clif->message(sd->fd, msg_sd(sd,281)); // You can't create chat rooms in this map
		return false;
	}

	if (map->getcell(sd->bl.m, &sd->bl, sd->bl.x, sd->bl.y, CELL_CHKNOCHAT) ) {
		clif->message (sd->fd, msg_sd(sd,865)); // "Can't create chat rooms in this area."
		return false;
	}

	pc_stop_walking(sd, STOPWALKING_FLAG_FIXPOS);

	cd = chat->create(&sd->bl, title, pass, limit, pub, 0, "", 0, 1, MAX_LEVEL);
	if( cd ) {
		cd->users = 1;
		cd->usersd[0] = sd;
		pc_setchatid(sd,cd->bl.id);
		pc_stop_attack(sd);
		clif->createchat(sd,0); // 0 = success
		clif->dispchat(cd,0);
		return true;
	}
	clif->createchat(sd,1); // 1 = Room limit exceeded

	return false;
}

/*==========================================
 * join an existing chatroom
 *------------------------------------------*/
bool chat_joinchat(struct map_session_data* sd, int chatid, const char* pass) {
	struct chat_data* cd;

	nullpo_ret(sd);
	nullpo_ret(pass);
	cd = map->id2cd(chatid);

	if (cd == NULL || cd->bl.type != BL_CHAT || cd->bl.m != sd->bl.m
	 || sd->state.vending || sd->state.buyingstore || sd->chat_id != 0
	 || ((cd->owner->type == BL_NPC) ? cd->users+1 : cd->users) >= cd->limit
	) {
		clif->joinchatfail(sd,0); // room full
		return false;
	}

	if( !cd->pub && strncmp(pass, cd->pass, sizeof(cd->pass)) != 0 && !pc_has_permission(sd, PC_PERM_JOIN_ALL_CHAT) )
	{
		clif->joinchatfail(sd,1); // wrong password
		return false;
	}

	if (sd->status.base_level < cd->min_level || sd->status.base_level > cd->max_level) {
		if(sd->status.base_level < cd->min_level)
			clif->joinchatfail(sd,5); // too low level
		else
			clif->joinchatfail(sd,6); // too high level

		return false;
	}

	if( sd->status.zeny < cd->zeny ) {
		clif->joinchatfail(sd,4); // not enough zeny
		return false;
	}

	if( cd->owner->type != BL_NPC && idb_exists(cd->kick_list,sd->status.char_id) ) {
		clif->joinchatfail(sd,2); // You have been kicked out of the room.
		return false;
	}

	pc_stop_walking(sd, STOPWALKING_FLAG_FIXPOS);
	cd->usersd[cd->users] = sd;
	cd->users++;

	pc_setchatid(sd,cd->bl.id);

	clif->joinchatok(sd, cd); //To the person who newly joined the list of all
	clif->addchat(cd, sd); //Reports To the person who already in the chat
	clif->dispchat(cd, 0); //Reported number of changes to the people around

	chat->trigger_event(cd); //Event

	return true;
}


/*==========================================
 * Leave a chatroom
 * Return
 *  0: User not found in chatroom/Missing data
 *  1: Success
 *  2: Chat room deleted (chat room empty)
 *  3: Owner changed (Owner left and a new one as assigned)
 *------------------------------------------*/
int chat_leavechat(struct map_session_data* sd, bool kicked) {
	struct chat_data* cd;
	int i;
	int leavechar;

	nullpo_retr(0, sd);

	cd = map->id2cd(sd->chat_id);
	if (cd == NULL) {
		pc_setchatid(sd, 0);
		return 0;
	}

	ARR_FIND( 0, cd->users, i, cd->usersd[i] == sd );
	if (i == cd->users) {
		// Not found in the chatroom?
		pc_setchatid(sd, 0);
		return 0;
	}

	clif->leavechat(cd, sd, kicked);
	pc_setchatid(sd, 0);
	cd->users--;

	leavechar = i;

	for( i = leavechar; i < cd->users; i++ )
		cd->usersd[i] = cd->usersd[i+1];


	if( cd->users == 0 && cd->owner->type == BL_PC ) { // Delete empty chatroom
		struct skill_unit* su;
		struct skill_unit_group* group;

		clif->clearchat(cd, 0);
		db_destroy(cd->kick_list);
		map->deliddb(&cd->bl);
		map->delblock(&cd->bl);
		map->freeblock(&cd->bl);

		su = map->find_skill_unit_oncell(&sd->bl, sd->bl.x, sd->bl.y, AL_WARP, NULL, 0);
		group = (su != NULL) ? su->group : NULL;
		if (group != NULL)
			skill->unit_onplace(su, &sd->bl, group->tick);

		return 2;
	}

	if( leavechar == 0 && cd->owner->type == BL_PC ) {
		// Set and announce new owner
		cd->owner = &cd->usersd[0]->bl;
		clif->changechatowner(cd, cd->usersd[0]);
		clif->clearchat(cd, 0);

		//Adjust Chat location after owner has been changed.
		map->delblock( &cd->bl );
		cd->bl.x = cd->owner->x;
		cd->bl.y = cd->owner->y;
		map->addblock( &cd->bl );

		clif->dispchat(cd,0);
		return 3; // Owner changed
	}
	clif->dispchat(cd,0); // refresh chatroom

	return 1;
}

/*==========================================
 * Change a chatroom's owner
 * Return
 *  0: User not found/Missing data
 *  1: Success
 *------------------------------------------*/
bool chat_changechatowner(struct map_session_data* sd, const char* nextownername) {
	struct chat_data* cd;
	struct map_session_data* tmpsd;
	int i;

	nullpo_ret(sd);
	nullpo_ret(nextownername);

	cd = map->id2cd(sd->chat_id);
	if (cd == NULL || &sd->bl != cd->owner)
		return false;

	ARR_FIND( 1, cd->users, i, strncmp(cd->usersd[i]->status.name, nextownername, NAME_LENGTH) == 0 );
	if( i == cd->users )
		return false;  // name not found

	// erase temporarily
	clif->clearchat(cd,0);

	// set new owner
	cd->owner = &cd->usersd[i]->bl;
	clif->changechatowner(cd,cd->usersd[i]);

	// swap the old and new owners' positions
	tmpsd = cd->usersd[i];
	cd->usersd[i] = cd->usersd[0];
	cd->usersd[0] = tmpsd;

	// set the new chatroom position
	map->delblock( &cd->bl );
	cd->bl.x = cd->owner->x;
	cd->bl.y = cd->owner->y;
	map->addblock( &cd->bl );

	// and display again
	clif->dispchat(cd,0);

	return true;
}

/*==========================================
 * Change a chatroom's status (title, etc)
 * Return
 *  0: Missing data
 *  1: Success
 *------------------------------------------*/
bool chat_changechatstatus(struct map_session_data* sd, const char* title, const char* pass, int limit, bool pub) {
	struct chat_data* cd;

	nullpo_ret(sd);
	nullpo_ret(title);
	nullpo_ret(pass);

	cd = map->id2cd(sd->chat_id);
	if (cd == NULL || &sd->bl != cd->owner)
		return false;

	safestrncpy(cd->title, title, CHATROOM_TITLE_SIZE);
	safestrncpy(cd->pass, pass, CHATROOM_PASS_SIZE);
	cd->limit = min(limit, ARRAYLENGTH(cd->usersd));
	cd->pub = pub;

	clif->changechatstatus(cd);
	clif->dispchat(cd,0);

	return true;
}

/*==========================================
 * Kick an user from a chatroom
 * Return:
 *  0: User cannot be kicked (is gm)/Missing data
 *  1: Success
 *------------------------------------------*/
bool chat_kickchat(struct map_session_data* sd, const char* kickusername) {
	struct chat_data* cd;
	int i;

	nullpo_ret(sd);
	nullpo_ret(kickusername);

	cd = map->id2cd(sd->chat_id);

	if (cd == NULL || &sd->bl != cd->owner)
		return false;

	ARR_FIND( 0, cd->users, i, strncmp(cd->usersd[i]->status.name, kickusername, NAME_LENGTH) == 0 );
	if( i == cd->users ) // User not found
		return false;

	if (pc_has_permission(cd->usersd[i], PC_PERM_NO_CHAT_KICK))
		return false; //gm kick protection [Valaris]

	idb_iput(cd->kick_list,cd->usersd[i]->status.char_id,1);

	chat->leave(cd->usersd[i], true);
	return true;
}

/*==========================================
 * Creates a chat room for the npc
 *------------------------------------------*/
bool chat_createnpcchat(struct npc_data* nd, const char* title, int limit, bool pub, int trigger, const char* ev, int zeny, int min_level, int max_level)
{
	struct chat_data* cd;
	nullpo_ret(nd);

	if( nd->chat_id ) {
		ShowError("chat_createnpcchat: npc '%s' already has a chatroom, cannot create new one!\n", nd->exname);
		return false;
	}

	if (zeny > MAX_ZENY || max_level > MAX_LEVEL) {
		ShowError("chat_createnpcchat: npc '%s' has a required lvl or amount of zeny over the max limit!\n", nd->exname);
		return false;
	}

	cd = chat->create(&nd->bl, title, "", limit, pub, trigger, ev, zeny, min_level, max_level);

	if( cd ) {
		nd->chat_id = cd->bl.id;
		clif->dispchat(cd,0);
		return true;
	}

	return false;
}

/*==========================================
 * Removes the chatroom from the npc.
 * Return:
 *  0: Missing data
 *  1: Success
 *------------------------------------------*/
bool chat_deletenpcchat(struct npc_data* nd) {
	struct chat_data *cd;
	nullpo_ret(nd);

	cd = map->id2cd(nd->chat_id);
	if (cd == NULL)
		return false;

	chat->npc_kick_all(cd);
	clif->clearchat(cd, 0);
	map->deliddb(&cd->bl);
	map->delblock(&cd->bl);
	map->freeblock(&cd->bl);
	nd->chat_id = 0;

	return true;
}

/*==========================================
 * Trigger npc event when we enter the chatroom
 * Return
 *  0: Couldn't trigger / Missing data
 *  1: Success
 *------------------------------------------*/
bool chat_triggerevent(struct chat_data *cd)
{
	nullpo_ret(cd);

	if( cd->users >= cd->trigger && cd->npc_event[0] )
	{
		npc->event_do(cd->npc_event);
		return true;
	}
	return false;
}

/// Enables the event of the chat room.
/// At most, 127 users are needed to trigger the event.
bool chat_enableevent(struct chat_data* cd)
{
	nullpo_ret(cd);

	cd->trigger &= 0x7f;
	chat->trigger_event(cd);
	return true;
}

/// Disables the event of the chat room
bool chat_disableevent(struct chat_data* cd)
{
	nullpo_ret(cd);

	cd->trigger |= 0x80;
	return true;
}

/// Kicks all the users from the chat room.
bool chat_npckickall(struct chat_data* cd)
{
	nullpo_ret(cd);

	while( cd->users > 0 )
		chat->leave(cd->usersd[cd->users-1], false);

	return true;
}

/*=====================================
* Default Functions : chat.h
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void chat_defaults(void) {
	chat = &chat_s;

	/* funcs */
	chat->create_pc_chat = chat_createpcchat;
	chat->join = chat_joinchat;
	chat->leave = chat_leavechat;
	chat->change_owner = chat_changechatowner;
	chat->change_status = chat_changechatstatus;
	chat->kick = chat_kickchat;
	chat->create_npc_chat = chat_createnpcchat;
	chat->delete_npc_chat = chat_deletenpcchat;
	chat->enable_event = chat_enableevent;
	chat->disable_event = chat_disableevent;
	chat->npc_kick_all = chat_npckickall;
	chat->trigger_event = chat_triggerevent;
	chat->create = chat_createchat;
}
