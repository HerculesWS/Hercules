// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/ers.h"
#include "../common/HPM.h"

#include "map.h"
#include "battle.h"
#include "clif.h"
#include "intif.h"
#include "npc.h"
#include "pc.h"
#include "pet.h"
#include "skill.h"
#include "status.h"
#include "homunculus.h"
#include "instance.h"
#include "mercenary.h"
#include "elemental.h"
#include "chrif.h"
#include "quest.h"
#include "storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

struct chrif_interface chrif_s;

//Used Packets:
//2af8: Outgoing, chrif_connect -> 'connect to charserver / auth @ charserver'
//2af9: Incoming, chrif_connectack -> 'answer of the 2af8 login(ok / fail)'
//2afa: Outgoing, chrif_sendmap -> 'sending our maps'
//2afb: Incoming, chrif_sendmapack -> 'Maps received successfully / or not ..'
//2afc: Outgoing, chrif_scdata_request -> request sc_data for pc->authok'ed char. <- new command reuses previous one.
//2afd: Incoming, chrif_authok -> 'client authentication ok'
//2afe: Outgoing, send_usercount_tochar -> 'sends player count of this map server to charserver'
//2aff: Outgoing, chrif_send_users_tochar -> 'sends all actual connected character ids to charserver'
//2b00: Incoming, map_setusers -> 'set the actual usercount? PACKET.2B COUNT.L.. ?' (not sure)
//2b01: Outgoing, chrif_save -> 'charsave of char XY account XY (complete struct)'
//2b02: Outgoing, chrif_charselectreq -> 'player returns from ingame to charserver to select another char.., this packets includes sessid etc' ? (not 100% sure)
//2b03: Incoming, clif_charselectok -> '' (i think its the packet after enterworld?) (not sure)
//2b04: Incoming, chrif_recvmap -> 'getting maps from charserver of other mapserver's'
//2b05: Outgoing, chrif_changemapserver -> 'Tell the charserver the mapchange / quest for ok...'
//2b06: Incoming, chrif_changemapserverack -> 'awnser of 2b05, ok/fail, data: dunno^^'
//2b07: Outgoing, chrif_removefriend -> 'Tell charserver to remove friend_id from char_id friend list'
//2b08: Outgoing, chrif_searchcharid -> '...'
//2b09: Incoming, map_addchariddb -> 'Adds a name to the nick db'
//2b0a: Incoming/Outgoing, socket_datasync()
//2b0b: Outgoing, update charserv skillid2idx
//2b0c: Outgoing, chrif_changeemail -> 'change mail address ...'
//2b0d: Incoming, chrif_changedsex -> 'Change sex of acc XY'
//2b0e: Outgoing, chrif_char_ask_name -> 'Do some operations (change sex, ban / unban etc)'
//2b0f: Incoming, chrif_char_ask_name_answer -> 'answer of the 2b0e'
//2b10: Outgoing, chrif_updatefamelist -> 'Update the fame ranking lists and send them'
//2b11: Outgoing, chrif_divorce -> 'tell the charserver to do divorce'
//2b12: Incoming, chrif_divorceack -> 'divorce chars
//2b13: FREE
//2b14: Incoming, chrif_accountban -> 'not sure: kick the player with message XY'
//2b15: FREE
//2b16: Outgoing, chrif_ragsrvinfo -> 'sends base / job / drop rates ....'
//2b17: Outgoing, chrif_char_offline -> 'tell the charserver that the char is now offline'
//2b18: Outgoing, chrif_char_reset_offline -> 'set all players OFF!'
//2b19: Outgoing, chrif_char_online -> 'tell the charserver that the char .. is online'
//2b1a: Outgoing, chrif_buildfamelist -> 'Build the fame ranking lists and send them'
//2b1b: Incoming, chrif_recvfamelist -> 'Receive fame ranking lists'
//2b1c: Outgoing, chrif_save_scdata -> 'Send sc_data of player for saving.'
//2b1d: Incoming, chrif_load_scdata -> 'received sc_data of player for loading.'
//2b1e: Incoming, chrif_update_ip -> 'Reqest forwarded from char-server for interserver IP sync.' [Lance]
//2b1f: Incoming, chrif_disconnectplayer -> 'disconnects a player (aid X) with the message XY ... 0x81 ..' [Sirius]
//2b20: Incoming, chrif_removemap -> 'remove maps of a server (sample: its going offline)' [Sirius]
//2b21: Incoming, chrif_save_ack. Returned after a character has been "final saved" on the char-server. [Skotlex]
//2b22: Incoming, chrif_updatefamelist_ack. Updated one position in the fame list.
//2b23: Outgoing, chrif_keepalive. charserver ping.
//2b24: Incoming, chrif_keepalive_ack. charserver ping reply.
//2b25: Incoming, chrif_deadopt -> 'Removes baby from Father ID and Mother ID'
//2b26: Outgoing, chrif_authreq -> 'client authentication request'
//2b27: Incoming, chrif_authfail -> 'client authentication failed'

//This define should spare writing the check in every function. [Skotlex]
#define chrif_check(a) do { if(!chrif->isconnected()) return a; } while(0)

/// Resets all the data.
void chrif_reset(void) {
	// TODO kick everyone out and reset everything [FlavioJS]
	exit(EXIT_FAILURE);
}

/// Checks the conditions for the server to stop.
/// Releases the cookie when all characters are saved.
/// If all the conditions are met, it stops the core loop.
void chrif_check_shutdown(void) {
	if( runflag != MAPSERVER_ST_SHUTDOWN )
		return;
	if( db_size(chrif->auth_db) > 0 )
		return;
	runflag = CORE_ST_STOP;
}

struct auth_node* chrif_search(int account_id) {
	return (struct auth_node*)idb_get(chrif->auth_db, account_id);
}

struct auth_node* chrif_auth_check(int account_id, int char_id, enum sd_state state) {
	struct auth_node *node = chrif->search(account_id);
	
	return ( node && node->char_id == char_id && node->state == state ) ? node : NULL;
}

bool chrif_auth_delete(int account_id, int char_id, enum sd_state state) {
	struct auth_node *node;

	if ( (node = chrif->auth_check(account_id, char_id, state) ) ) {
		int fd = node->sd ? node->sd->fd : node->fd;
		
		if ( session[fd] && session[fd]->session_data == node->sd )
			session[fd]->session_data = NULL;
		
		if ( node->sd )
			aFree(node->sd);
		
		ers_free(chrif->auth_db_ers, node);
		idb_remove(chrif->auth_db,account_id);
		
		return true;
	}
	return false;
}

//Moves the sd character to the auth_db structure.
bool chrif_sd_to_auth(TBL_PC* sd, enum sd_state state) {
	struct auth_node *node;
	
	if ( chrif->search(sd->status.account_id) )
		return false; //Already exists?

	node = ers_alloc(chrif->auth_db_ers, struct auth_node);
	
	memset(node, 0, sizeof(struct auth_node));
	
	node->account_id = sd->status.account_id;
	node->char_id = sd->status.char_id;
	node->login_id1 = sd->login_id1;
	node->login_id2 = sd->login_id2;
	node->sex = sd->status.sex;
	node->fd = sd->fd;
	node->sd = sd;	//Data from logged on char.
	node->node_created = timer->gettick(); //timestamp for node timeouts
	node->state = state;

	sd->state.active = 0;
	
	idb_put(chrif->auth_db, node->account_id, node);
	
	return true;
}

bool chrif_auth_logout(TBL_PC* sd, enum sd_state state) {
	
	if(sd->fd && state == ST_LOGOUT) { //Disassociate player, and free it after saving ack returns. [Skotlex]
		//fd info must not be lost for ST_MAPCHANGE as a final packet needs to be sent to the player.
		if ( session[sd->fd] )
			session[sd->fd]->session_data = NULL;
		sd->fd = 0;
	}
	
	return chrif->sd_to_auth(sd, state);
}

bool chrif_auth_finished(TBL_PC* sd) {
	struct auth_node *node= chrif->search(sd->status.account_id);
	
	if ( node && node->sd == sd && node->state == ST_LOGIN ) {
		node->sd = NULL;
		
		return chrif->auth_delete(node->account_id, node->char_id, ST_LOGIN);
	}
	
	return false;
}
// sets char-server's user id
void chrif_setuserid(char *id) {
	memcpy(chrif->userid, id, NAME_LENGTH);
}

// sets char-server's password
void chrif_setpasswd(char *pwd) {
	memcpy(chrif->passwd, pwd, NAME_LENGTH);
}

// security check, prints warning if using default password
void chrif_checkdefaultlogin(void) {
	if (strcmp(chrif->userid, "s1")==0 && strcmp(chrif->passwd, "p1")==0) {
		ShowWarning("Using the default user/password s1/p1 is NOT RECOMMENDED.\n");
		ShowNotice("Please edit your 'login' table to create a proper inter-server user/password (gender 'S')\n");
		ShowNotice("and then edit your user/password in conf/map-server.conf (or conf/import/map_conf.txt)\n");
	}
}

// sets char-server's ip address
bool chrif_setip(const char* ip) {
	char ip_str[16];
	
	if ( !( chrif->ip = host2ip(ip) ) ) {
		ShowWarning("Failed to Resolve Char Server Address! (%s)\n", ip);
		return false;
	}
	
	safestrncpy(chrif->ip_str, ip, sizeof(chrif->ip_str));
	
	ShowInfo("Char Server IP Address : '"CL_WHITE"%s"CL_RESET"' -> '"CL_WHITE"%s"CL_RESET"'.\n", ip, ip2str(chrif->ip, ip_str));
	
	return true;
}

// sets char-server's port number
void chrif_setport(uint16 port) {
	chrif->port = port;
}

// says whether the char-server is connected or not
int chrif_isconnected(void) {
	return (chrif->fd > 0 && session[chrif->fd] != NULL && chrif->state == 2);
}

/*==========================================
 * Saves character data.
 * Flag = 1: Character is quitting
 * Flag = 2: Character is changing map-servers
 *------------------------------------------*/
bool chrif_save(struct map_session_data *sd, int flag) {
	nullpo_ret(sd);

	pc->makesavestatus(sd);

	if (flag && sd->state.active) { //Store player data which is quitting
		//FIXME: SC are lost if there's no connection at save-time because of the way its related data is cleared immediately after this function. [Skotlex]
		if ( chrif->isconnected() )
			chrif->save_scdata(sd);
		if ( !chrif->auth_logout(sd,flag == 1 ? ST_LOGOUT : ST_MAPCHANGE) )
			ShowError("chrif_save: Failed to set up player %d:%d for proper quitting!\n", sd->status.account_id, sd->status.char_id);
	}

	chrif_check(false); //Character is saved on reconnect.

	//For data sync
	if (sd->state.storage_flag == 2)
		gstorage->save(sd->status.account_id, sd->status.guild_id, flag);

	if (flag)
		sd->state.storage_flag = 0; //Force close it.

	//Saving of registry values. 
	if (sd->vars_dirty)
		intif->saveregistry(sd);

	WFIFOHEAD(chrif->fd, sizeof(sd->status) + 13);
	WFIFOW(chrif->fd,0) = 0x2b01;
	WFIFOW(chrif->fd,2) = sizeof(sd->status) + 13;
	WFIFOL(chrif->fd,4) = sd->status.account_id;
	WFIFOL(chrif->fd,8) = sd->status.char_id;
	WFIFOB(chrif->fd,12) = (flag==1)?1:0; //Flag to tell char-server this character is quitting.
	memcpy(WFIFOP(chrif->fd,13), &sd->status, sizeof(sd->status));
	WFIFOSET(chrif->fd, WFIFOW(chrif->fd,2));

	if( sd->status.pet_id > 0 && sd->pd )
		intif->save_petdata(sd->status.account_id,&sd->pd->pet);
	if( sd->hd && homun_alive(sd->hd) )
		homun->save(sd->hd);
	if( sd->md && mercenary->get_lifetime(sd->md) > 0 )
		mercenary->save(sd->md);
	if( sd->ed && elemental->get_lifetime(sd->ed) > 0 )
		elemental->save(sd->ed);	
	if( sd->save_quest )
		intif->quest_save(sd);

	return true;
}

// connects to char-server (plaintext)
void chrif_connect(int fd) {
	ShowStatus("Logging in to char server...\n", chrif->fd);
	WFIFOHEAD(fd,60);
	WFIFOW(fd,0) = 0x2af8;
	memcpy(WFIFOP(fd,2), chrif->userid, NAME_LENGTH);
	memcpy(WFIFOP(fd,26), chrif->passwd, NAME_LENGTH);
	WFIFOL(fd,50) = 0;
	WFIFOL(fd,54) = htonl(clif->map_ip);
	WFIFOW(fd,58) = htons(clif->map_port);
	WFIFOSET(fd,60);
}

// sends maps to char-server
void chrif_sendmap(int fd) {
	int i;
	
	ShowStatus("Sending maps to char server...\n");
	
	// Sending normal maps, not instances
	WFIFOHEAD(fd, 4 + instance->start_id * 4);
	WFIFOW(fd,0) = 0x2afa;
	for(i = 0; i < instance->start_id; i++)
		WFIFOW(fd,4+i*4) = map_id2index(i);
	WFIFOW(fd,2) = 4 + i * 4;
	WFIFOSET(fd,WFIFOW(fd,2));
}

// receive maps from some other map-server (relayed via char-server)
void chrif_recvmap(int fd) {
	int i, j;
	uint32 ip = ntohl(RFIFOL(fd,4));
	uint16 port = ntohs(RFIFOW(fd,8));

	for(i = 10, j = 0; i < RFIFOW(fd,2); i += 4, j++) {
		map->setipport(RFIFOW(fd,i), ip, port);
	}
	
	if (battle_config.etc_log)
		ShowStatus("Received maps from %d.%d.%d.%d:%d (%d maps)\n", CONVIP(ip), port, j);

	chrif->other_mapserver_count++;
}

// remove specified maps (used when some other map-server disconnects)
void chrif_removemap(int fd) {
	int i, j;
	uint32 ip =  RFIFOL(fd,4);
	uint16 port = RFIFOW(fd,8);

	for(i = 10, j = 0; i < RFIFOW(fd, 2); i += 4, j++)
		map->eraseipport(RFIFOW(fd, i), ip, port);

	chrif->other_mapserver_count--;
	
	if(battle_config.etc_log)
		ShowStatus("remove map of server %d.%d.%d.%d:%d (%d maps)\n", CONVIP(ip), port, j);
}

// received after a character has been "final saved" on the char-server
void chrif_save_ack(int fd) {
	chrif->auth_delete(RFIFOL(fd,2), RFIFOL(fd,6), ST_LOGOUT);
	chrif->check_shutdown();
}

// request to move a character between mapservers
bool chrif_changemapserver(struct map_session_data* sd, uint32 ip, uint16 port) {
	nullpo_ret(sd);

	if (chrif->other_mapserver_count < 1) {//No other map servers are online!
		clif->authfail_fd(sd->fd, 0);
		return false;
	}

	chrif_check(false);

	WFIFOHEAD(chrif->fd,35);
	WFIFOW(chrif->fd, 0) = 0x2b05;
	WFIFOL(chrif->fd, 2) = sd->bl.id;
	WFIFOL(chrif->fd, 6) = sd->login_id1;
	WFIFOL(chrif->fd,10) = sd->login_id2;
	WFIFOL(chrif->fd,14) = sd->status.char_id;
	WFIFOW(chrif->fd,18) = sd->mapindex;
	WFIFOW(chrif->fd,20) = sd->bl.x;
	WFIFOW(chrif->fd,22) = sd->bl.y;
	WFIFOL(chrif->fd,24) = htonl(ip);
	WFIFOW(chrif->fd,28) = htons(port);
	WFIFOB(chrif->fd,30) = sd->status.sex;
	WFIFOL(chrif->fd,31) = htonl(session[sd->fd]->client_addr);
	WFIFOL(chrif->fd,35) = sd->group_id;
	WFIFOSET(chrif->fd,39);
	
	return true;
}

/// map-server change request acknowledgement (positive or negative)
/// R 2b06 <account_id>.L <login_id1>.L <login_id2>.L <char_id>.L <map_index>.W <x>.W <y>.W <ip>.L <port>.W
bool chrif_changemapserverack(int account_id, int login_id1, int login_id2, int char_id, short map_index, short x, short y, uint32 ip, uint16 port) {
	struct auth_node *node;
	
	if ( !( node = chrif->auth_check(account_id, char_id, ST_MAPCHANGE) ) )
		return false;

	if ( !login_id1 ) {
		ShowError("chrif_changemapserverack: map server change failed.\n");
		clif->authfail_fd(node->fd, 0);
	} else
		clif->changemapserver(node->sd, map_index, x, y, ntohl(ip), ntohs(port));

	//Player has been saved already, remove him from memory. [Skotlex]
	chrif->auth_delete(account_id, char_id, ST_MAPCHANGE);

	return (!login_id1)?false:true; // Is this the best approach here?
}

/*==========================================
 *
 *------------------------------------------*/
void chrif_connectack(int fd) {
	static bool char_init_done = false;

	if (RFIFOB(fd,2)) {
		ShowFatalError("Connection to char-server failed %d.\n", RFIFOB(fd,2));
		exit(EXIT_FAILURE);
	}

	ShowStatus("Successfully logged on to Char Server (Connection: '"CL_WHITE"%d"CL_RESET"').\n",fd);
	chrif->state = 1;
	chrif->connected = 1;

	chrif->sendmap(fd);

	ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc->event_doall("OnInterIfInit"));
	if( !char_init_done ) {
		char_init_done = true;
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc->event_doall("OnInterIfInitOnce"));
		guild->castle_map_init();
	}
	
	sockt->datasync(fd, true);
	chrif->skillid2idx(fd);
}

/**
 * @see DBApply
 */
int chrif_reconnect(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = DB->data2ptr(data);
	
	switch (node->state) {
		case ST_LOGIN:
			if ( node->sd ) {//Since there is no way to request the char auth, make it fail.
				pc->authfail(node->sd);
				chrif_char_offline(node->sd);
				chrif->auth_delete(node->account_id, node->char_id, ST_LOGIN);
			}
			break;
		case ST_LOGOUT:
			//Re-send final save
			chrif->save(node->sd, 1);
			break;
		case ST_MAPCHANGE: { //Re-send map-change request.
			struct map_session_data *sd = node->sd;
			uint32 ip;
			uint16 port;
			
			if( map->mapname2ipport(sd->mapindex,&ip,&port) == 0 )
				chrif->changemapserver(sd, ip, port);
			else //too much lag/timeout is the closest explanation for this error.
				clif->authfail_fd(sd->fd, 3);
			
			break;
			}
	}
	
	return 0;
}


/// Called when all the connection steps are completed.
void chrif_on_ready(void) {
	static bool once = false;
	ShowStatus("Map Server is now online.\n");
	
	chrif->state = 2;
	
	chrif->check_shutdown();

	//If there are players online, send them to the char-server. [Skotlex]
	chrif->send_users_tochar();

	//Auth db reconnect handling
	chrif->auth_db->foreach(chrif->auth_db,chrif->reconnect);

	//Re-save any storages that were modified in the disconnection time. [Skotlex]
	storage->reconnect();

	//Re-save any guild castles that were modified in the disconnection time.
	guild->castle_reconnect(-1, 0, 0);
	
	if( !once ) {
#ifdef AUTOTRADE_PERSISTENCY
		pc->autotrade_load();
#endif
		once = true;
	}
}


/*==========================================
 *
 *------------------------------------------*/
void chrif_sendmapack(int fd) {
	
	if (RFIFOB(fd,2)) {
		ShowFatalError("chrif : send map list to char server failed %d\n", RFIFOB(fd,2));
		exit(EXIT_FAILURE);
	}

	memcpy(map->wisp_server_name, RFIFOP(fd,3), NAME_LENGTH);
	
	chrif->on_ready();
}

/*==========================================
 * Request sc_data from charserver [Skotlex]
 *------------------------------------------*/
bool chrif_scdata_request(int account_id, int char_id) {
	
#ifdef ENABLE_SC_SAVING
	chrif_check(false);

	WFIFOHEAD(chrif->fd,10);
	WFIFOW(chrif->fd,0) = 0x2afc;
	WFIFOL(chrif->fd,2) = account_id;
	WFIFOL(chrif->fd,6) = char_id;
	WFIFOSET(chrif->fd,10);
#endif
	
	return true;
}

/*==========================================
 * Request auth confirmation
 *------------------------------------------*/
void chrif_authreq(struct map_session_data *sd, bool hstandalone) {
	struct auth_node *node= chrif->search(sd->bl.id);
	
	if( node != NULL || !chrif->isconnected() ) {
		set_eof(sd->fd);
		return;
	}

	WFIFOHEAD(chrif->fd,20);
	WFIFOW(chrif->fd,0) = 0x2b26;
	WFIFOL(chrif->fd,2) = sd->status.account_id;
	WFIFOL(chrif->fd,6) = sd->status.char_id;
	WFIFOL(chrif->fd,10) = sd->login_id1;
	WFIFOB(chrif->fd,14) = sd->status.sex;
	WFIFOL(chrif->fd,15) = htonl(session[sd->fd]->client_addr);
	WFIFOB(chrif->fd,19) = hstandalone ? 1 : 0;
	WFIFOSET(chrif->fd,20);
	chrif->sd_to_auth(sd, ST_LOGIN);
}

/*==========================================
 * Auth confirmation ack
 *------------------------------------------*/
void chrif_authok(int fd) {
	int account_id, group_id, char_id;
	uint32 login_id1,login_id2;
	time_t expiration_time;
	struct mmo_charstatus* charstatus;
	struct auth_node *node;
	bool changing_mapservers;
	TBL_PC* sd;

	//Check if both servers agree on the struct's size
	if( RFIFOW(fd,2) - 25 != sizeof(struct mmo_charstatus) ) {
		ShowError("chrif_authok: Data size mismatch! %d != %d\n", RFIFOW(fd,2) - 25, sizeof(struct mmo_charstatus));
		return;
	}

	account_id = RFIFOL(fd,4);
	login_id1 = RFIFOL(fd,8);
	login_id2 = RFIFOL(fd,12);
	expiration_time = (time_t)(int32)RFIFOL(fd,16);
	group_id = RFIFOL(fd,20);
	changing_mapservers = (RFIFOB(fd,24));
	charstatus = (struct mmo_charstatus*)RFIFOP(fd,25);
	char_id = charstatus->char_id;

	//Check if we don't already have player data in our server
	//Causes problems if the currently connected player tries to quit or this data belongs to an already connected player which is trying to re-auth.
	if ( ( sd = map->id2sd(account_id) ) != NULL )
		return;

	if ( ( node = chrif->search(account_id) ) == NULL )
		return; // should not happen

	if ( node->state != ST_LOGIN )
		return; //character in logout phase, do not touch that data.

	if ( node->sd == NULL ) {
		/*
		//When we receive double login info and the client has not connected yet,
		//discard the older one and keep the new one.
		chrif->auth_delete(node->account_id, node->char_id, ST_LOGIN);
		*/
		return; // should not happen
	}

	sd = node->sd;

	if( runflag == MAPSERVER_ST_RUNNING &&
		node->account_id == account_id &&
		node->char_id == char_id &&
		node->login_id1 == login_id1 )
	{ //Auth Ok
		if (pc->authok(sd, login_id2, expiration_time, group_id, charstatus, changing_mapservers))
			return;
	} else { //Auth Failed
		pc->authfail(sd);
	}
	
	chrif_char_offline(sd); //Set him offline, the char server likely has it set as online already.
	chrif->auth_delete(account_id, char_id, ST_LOGIN);
}

// client authentication failed
void chrif_authfail(int fd) {/* HELLO WORLD. ip in RFIFOL 15 is not being used (but is available) */
	int account_id, char_id;
	uint32 login_id1;
	char sex;
	struct auth_node* node;

	account_id = RFIFOL(fd,2);
	char_id    = RFIFOL(fd,6);
	login_id1  = RFIFOL(fd,10);
	sex        = RFIFOB(fd,14);

	node = chrif->search(account_id);
	
	if( node != NULL &&
		node->account_id == account_id &&
		node->char_id == char_id &&
		node->login_id1 == login_id1 &&
		node->sex == sex &&
		node->state == ST_LOGIN )
	{// found a match
		clif->authfail_fd(node->fd, 0);
		chrif->auth_delete(account_id, char_id, ST_LOGIN);
	}
}


/**
 * This can still happen (client times out while waiting for char to confirm auth data)
 * @see DBApply
 */
int auth_db_cleanup_sub(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = DB->data2ptr(data);
	const char* states[] = { "Login", "Logout", "Map change" };
	
	if(DIFF_TICK(timer->gettick(),node->node_created)>60000) {
		switch (node->state) {
			case ST_LOGOUT:
				//Re-save attempt (->sd should never be null here).
				node->node_created = timer->gettick(); //Refresh tick (avoid char-server load if connection is really bad)
				chrif->save(node->sd, 1);
				break;
			default:
				//Clear data. any connected players should have timed out by now.
				ShowInfo("auth_db: Node (state %s) timed out for %d:%d\n", states[node->state], node->account_id, node->char_id);
				chrif->char_offline_nsd(node->account_id, node->char_id);
				chrif->auth_delete(node->account_id, node->char_id, node->state);
				break;
		}
		return 1;
	}
	return 0;
}

int auth_db_cleanup(int tid, int64 tick, int id, intptr_t data) {
	chrif_check(0);
	chrif->auth_db->foreach(chrif->auth_db, chrif->auth_db_cleanup_sub);
	return 0;
}

/*==========================================
 * Request char selection
 *------------------------------------------*/
bool chrif_charselectreq(struct map_session_data* sd, uint32 s_ip) {
	nullpo_ret(sd);

	if( !sd->bl.id || !sd->login_id1 )
		return false;
	
	chrif_check(false);

	WFIFOHEAD(chrif->fd,18);
	WFIFOW(chrif->fd, 0) = 0x2b02;
	WFIFOL(chrif->fd, 2) = sd->bl.id;
	WFIFOL(chrif->fd, 6) = sd->login_id1;
	WFIFOL(chrif->fd,10) = sd->login_id2;
	WFIFOL(chrif->fd,14) = htonl(s_ip);
	WFIFOSET(chrif->fd,18);

	return true;
}

/*==========================================
 * Search Char trough id on char serv
 *------------------------------------------*/
bool chrif_searchcharid(int char_id) {
	
	if( !char_id )
		return false;
	
	chrif_check(false);

	WFIFOHEAD(chrif->fd,6);
	WFIFOW(chrif->fd,0) = 0x2b08;
	WFIFOL(chrif->fd,2) = char_id;
	WFIFOSET(chrif->fd,6);

	return true;
}

/*==========================================
 * Change Email
 *------------------------------------------*/
bool chrif_changeemail(int id, const char *actual_email, const char *new_email) {
	
	if (battle_config.etc_log)
		ShowInfo("chrif_changeemail: account: %d, actual_email: '%s', new_email: '%s'.\n", id, actual_email, new_email);

	chrif_check(false);

	WFIFOHEAD(chrif->fd,86);
	WFIFOW(chrif->fd,0) = 0x2b0c;
	WFIFOL(chrif->fd,2) = id;
	memcpy(WFIFOP(chrif->fd,6), actual_email, 40);
	memcpy(WFIFOP(chrif->fd,46), new_email, 40);
	WFIFOSET(chrif->fd,86);

	return true;
}

/*==========================================
 * S 2b0e <accid>.l <name>.24B <type>.w { <year>.w <month>.w <day>.w <hour>.w <minute>.w <second>.w }
 * Send an account modification request to the login server (via char server).
 * type of operation:
 *   1: block, 2: ban, 3: unblock, 4: unban, 5: changesex (use next function for 5), 6: charban
 *------------------------------------------*/
bool chrif_char_ask_name(int acc, const char* character_name, unsigned short operation_type, int year, int month, int day, int hour, int minute, int second) {
	
	chrif_check(false);

	WFIFOHEAD(chrif->fd,44);
	WFIFOW(chrif->fd,0) = 0x2b0e;
	WFIFOL(chrif->fd,2) = acc;
	safestrncpy((char*)WFIFOP(chrif->fd,6), character_name, NAME_LENGTH);
	WFIFOW(chrif->fd,30) = operation_type;
	
	if ( operation_type == 2 || operation_type == 6 ) {
		WFIFOW(chrif->fd,32) = year;
		WFIFOW(chrif->fd,34) = month;
		WFIFOW(chrif->fd,36) = day;
		WFIFOW(chrif->fd,38) = hour;
		WFIFOW(chrif->fd,40) = minute;
		WFIFOW(chrif->fd,42) = second;
	}
	
	WFIFOSET(chrif->fd,44);
	return true;
}

bool chrif_changesex(struct map_session_data *sd) {
	chrif_check(false);
	
	WFIFOHEAD(chrif->fd,44);
	WFIFOW(chrif->fd,0) = 0x2b0e;
	WFIFOL(chrif->fd,2) = sd->status.account_id;
	safestrncpy((char*)WFIFOP(chrif->fd,6), sd->status.name, NAME_LENGTH);
	WFIFOW(chrif->fd,30) = 5;
	WFIFOSET(chrif->fd,44);

	clif->message(sd->fd, msg_txt(408)); //"Need disconnection to perform change-sex request..."

	if (sd->fd)
		clif->authfail_fd(sd->fd, 15);
	else
		map->quit(sd);
	return true;
}

/*==========================================
 * R 2b0f <accid>.l <name>.24B <type>.w <answer>.w
 * Processing a reply to chrif->char_ask_name() (request to modify an account).
 * type of operation:
 *   1: block, 2: ban, 3: unblock, 4: unban, 5: changesex, 6: charban, 7: charunban
 * type of answer:
 *   0: login-server request done
 *   1: player not found
 *   2: gm level too low
 *   3: login-server offline
 *------------------------------------------*/
bool chrif_char_ask_name_answer(int acc, const char* player_name, uint16 type, uint16 answer) {
	struct map_session_data* sd;
	char action[25];
	char output[256];
	bool charsrv = ( type == 6 || type == 7 ) ? true : false;
	
	sd = map->id2sd(acc);
	
	if( acc < 0 || sd == NULL ) {
		ShowError("chrif_char_ask_name_answer failed - player not online.\n");
		return false;
	}

	/* re-use previous msg_txt */
	if( type == 6 ) type = 2;
	if( type == 7 ) type = 4;
	
	if( type > 0 && type <= 5 )
		snprintf(action,25,"%s",msg_txt(427+type)); //block|ban|unblock|unban|change the sex of
	else
		snprintf(action,25,"???");

	switch( answer ) {
		case 0 : sprintf(output, msg_txt(charsrv?434:424), action, NAME_LENGTH, player_name); break;
		case 1 : sprintf(output, msg_txt(425), NAME_LENGTH, player_name); break;
		case 2 : sprintf(output, msg_txt(426), action, NAME_LENGTH, player_name); break;
		case 3 : sprintf(output, msg_txt(427), action, NAME_LENGTH, player_name); break;
		default: output[0] = '\0'; break;
	}
	
	clif->message(sd->fd, output);
	return true;
}

/*==========================================
 * Request char server to change sex of char (modified by Yor)
 *------------------------------------------*/
void chrif_changedsex(int fd) {
	int acc, sex;
	struct map_session_data *sd;

	acc = RFIFOL(fd,2);
	sex = RFIFOL(fd,6);
	
	if ( battle_config.etc_log )
		ShowNotice("chrif_changedsex %d.\n", acc);
	
	sd = map->id2sd(acc);
	if ( sd ) { //Normally there should not be a char logged on right now!
		if ( sd->status.sex == sex ) 
			return; //Do nothing? Likely safe.
		sd->status.sex = !sd->status.sex;

		// reset skill of some job
		if ((sd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER) {
			int i, idx = 0;
			// remove specifical skills of Bard classes 
			for(i = 315; i <= 322; i++) {
				idx = skill->get_index(i);
				if (sd->status.skill[idx].id > 0 && sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT) {
					sd->status.skill_point += sd->status.skill[idx].lv;
					sd->status.skill[idx].id = 0;
					sd->status.skill[idx].lv = 0;
				}
			}
			// remove specifical skills of Dancer classes 
			for(i = 323; i <= 330; i++) {
				idx = skill->get_index(i);
				if (sd->status.skill[idx].id > 0 && sd->status.skill[idx].flag == SKILL_FLAG_PERMANENT) {
					sd->status.skill_point += sd->status.skill[idx].lv;
					sd->status.skill[idx].id = 0;
					sd->status.skill[idx].lv = 0;
				}
			}
			clif->updatestatus(sd, SP_SKILLPOINT);
			// change job if necessary
			if (sd->status.sex) //Changed from Dancer
				sd->status.class_ -= 1;
			else	//Changed from Bard
				sd->status.class_ += 1;
			//sd->class_ needs not be updated as both Dancer/Bard are the same.
		}
		// save character
		sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
							  // do same modify in login-server for the account, but no in char-server (it ask again login_id1 to login, and don't remember it)
		clif->message(sd->fd, msg_txt(409)); //"Your sex has been changed (need disconnection by the server)..."
		set_eof(sd->fd); // forced to disconnect for the change
		map->quit(sd); // Remove leftovers (e.g. autotrading) [Paradox924X]
	}
}
/*==========================================
 * Request Char Server to Divorce Players
 *------------------------------------------*/
bool chrif_divorce(int partner_id1, int partner_id2) {
	chrif_check(false);

	WFIFOHEAD(chrif->fd,10);
	WFIFOW(chrif->fd,0) = 0x2b11;
	WFIFOL(chrif->fd,2) = partner_id1;
	WFIFOL(chrif->fd,6) = partner_id2;
	WFIFOSET(chrif->fd,10);

	return true;
}

/*==========================================
 * Divorce players
 * only used if 'partner_id' is offline
 *------------------------------------------*/
bool chrif_divorceack(int char_id, int partner_id) {
	struct map_session_data* sd;
	int i;

	if( !char_id || !partner_id )
		return false;

	if( ( sd = map->charid2sd(char_id) ) != NULL && sd->status.partner_id == partner_id ) {
		sd->status.partner_id = 0;
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc->delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}

	if( ( sd = map->charid2sd(partner_id) ) != NULL && sd->status.partner_id == char_id ) {
		sd->status.partner_id = 0;
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->status.inventory[i].nameid == WEDDING_RING_M || sd->status.inventory[i].nameid == WEDDING_RING_F)
				pc->delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
	}
	
	return true;
}
/*==========================================
 * Removes Baby from parents
 *------------------------------------------*/
void chrif_deadopt(int father_id, int mother_id, int child_id) {
	struct map_session_data* sd;
	int idx = skill->get_index(WE_CALLBABY);

	if( father_id && ( sd = map->charid2sd(father_id) ) != NULL && sd->status.child == child_id ) {
		sd->status.child = 0;
		sd->status.skill[idx].id = 0;
		sd->status.skill[idx].lv = 0;
		sd->status.skill[idx].flag = 0;
		clif->deleteskill(sd,WE_CALLBABY);
	}

	if( mother_id && ( sd = map->charid2sd(mother_id) ) != NULL && sd->status.child == child_id ) {
		sd->status.child = 0;
		sd->status.skill[idx].id = 0;
		sd->status.skill[idx].lv = 0;
		sd->status.skill[idx].flag = 0;
		clif->deleteskill(sd,WE_CALLBABY);
	}

}

/*==========================================
 * Disconnection of a player (account or char has been banned of has a status, from login or char server) by [Yor]
 *------------------------------------------*/
void chrif_idbanned(int fd) {
	int id;
	struct map_session_data *sd;

	id = RFIFOL(fd,2);
	
	if ( battle_config.etc_log )
		ShowNotice("chrif_idbanned %d.\n", id);
	
	sd = ( RFIFOB(fd,6) == 2 ) ? map->charid2sd(id) : map->id2sd(id);

	if ( id < 0 || sd == NULL ) {
		/* player not online or unknown id, either way no error is necessary (since if you try to ban a offline char it still works) */
		return;
	}

	sd->login_id1++; // change identify, because if player come back in char within the 5 seconds, he can change its characters
	if (RFIFOB(fd,6) == 0) { // 0: change of statut
		int ret_status = RFIFOL(fd,7); // status or final date of a banishment
		if(0<ret_status && ret_status<=9)
			clif->message(sd->fd, msg_txt(411+ret_status));
		else if(ret_status==100)
			clif->message(sd->fd, msg_txt(421));
		else
			clif->message(sd->fd, msg_txt(420)); //"Your account has not more authorised."
	} else if (RFIFOB(fd,6) == 1) { // 1: ban
		time_t timestamp;
		char tmpstr[2048];
		timestamp = (time_t)RFIFOL(fd,7); // status or final date of a banishment
		strcpy(tmpstr, msg_txt(423)); //"Your account has been banished until "
		strftime(tmpstr + strlen(tmpstr), 24, "%d-%m-%Y %H:%M:%S", localtime(&timestamp));
		clif->message(sd->fd, tmpstr);
	} else if (RFIFOB(fd,6) == 2) { // 2: change of status for character
		time_t timestamp;
		char tmpstr[2048];
		timestamp = (time_t)RFIFOL(fd,7); // status or final date of a banishment
		strcpy(tmpstr, msg_txt(433)); //"This character has been banned until  "
		strftime(tmpstr + strlen(tmpstr), 24, "%d-%m-%Y %H:%M:%S", localtime(&timestamp));
		clif->message(sd->fd, tmpstr);
	}

	set_eof(sd->fd); // forced to disconnect for the change
	map->quit(sd); // Remove leftovers (e.g. autotrading) [Paradox924X]
}

//Disconnect the player out of the game, simple packet
//packet.w AID.L WHY.B 2+4+1 = 7byte
int chrif_disconnectplayer(int fd) {
	struct map_session_data* sd;
	int account_id = RFIFOL(fd, 2);

	sd = map->id2sd(account_id);
	if( sd == NULL ) {
		struct auth_node* auth = chrif->search(account_id);
		
		if( auth != NULL && chrif->auth_delete(account_id, auth->char_id, ST_LOGIN) )
			return 0;
		
		return -1;
	}

	if (!sd->fd) { //No connection
		if (sd->state.autotrade)
			map->quit(sd); //Remove it.
		//Else we don't remove it because the char should have a timer to remove the player because it force-quit before,
		//and we don't want them kicking their previous instance before the 10 secs penalty time passes. [Skotlex]
		return 0;
	}

	switch(RFIFOB(fd, 6)) {
		case 1: clif->authfail_fd(sd->fd, 1); break; //server closed
		case 2: clif->authfail_fd(sd->fd, 2); break; //someone else logged in
		case 3: clif->authfail_fd(sd->fd, 4); break; //server overpopulated
		case 4: clif->authfail_fd(sd->fd, 10); break; //out of available time paid for
		case 5: clif->authfail_fd(sd->fd, 15); break; //forced to dc by gm
	}
	return 0;
}

/*==========================================
 * Request/Receive top 10 Fame character list
 *------------------------------------------*/
int chrif_updatefamelist(struct map_session_data* sd) {
	char type;
	
	chrif_check(-1);

	switch(sd->class_ & MAPID_UPPERMASK) {
		case MAPID_BLACKSMITH: type = RANKTYPE_BLACKSMITH; break;
		case MAPID_ALCHEMIST:  type = RANKTYPE_ALCHEMIST;  break;
		case MAPID_TAEKWON:    type = RANKTYPE_TAEKWON;    break;
		default:
			return 0;
	}

	WFIFOHEAD(chrif->fd, 11);
	WFIFOW(chrif->fd,0) = 0x2b10;
	WFIFOL(chrif->fd,2) = sd->status.char_id;
	WFIFOL(chrif->fd,6) = sd->status.fame;
	WFIFOB(chrif->fd,10) = type;
	WFIFOSET(chrif->fd,11);

	return 0;
}

bool chrif_buildfamelist(void) {
	chrif_check(false);

	WFIFOHEAD(chrif->fd,2);
	WFIFOW(chrif->fd,0) = 0x2b1a;
	WFIFOSET(chrif->fd,2);

	return true;
}

void chrif_recvfamelist(int fd) {
	int num, size;
	int total = 0, len = 8;

	memset(pc->smith_fame_list, 0, sizeof(pc->smith_fame_list));
	memset(pc->chemist_fame_list, 0, sizeof(pc->chemist_fame_list));
	memset(pc->taekwon_fame_list, 0, sizeof(pc->taekwon_fame_list));

	size = RFIFOW(fd, 6); //Blacksmith block size
	
	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&pc->smith_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}
	
	total += num;

	size = RFIFOW(fd, 4); //Alchemist block size
	
	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&pc->chemist_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}
	
	total += num;

	size = RFIFOW(fd, 2); //Total packet length
	
	for (num = 0; len < size && num < MAX_FAME_LIST; num++) {
		memcpy(&pc->taekwon_fame_list[num], RFIFOP(fd,len), sizeof(struct fame_list));
 		len += sizeof(struct fame_list);
	}
	
	total += num;

	ShowInfo("Received Fame List of '"CL_WHITE"%d"CL_RESET"' characters.\n", total);
}

/// fame ranking update confirmation
/// R 2b22 <table>.B <index>.B <value>.L
int chrif_updatefamelist_ack(int fd) {
	struct fame_list* list;
	uint8 index;
	
	switch (RFIFOB(fd,2)) {
		case RANKTYPE_BLACKSMITH: list = pc->smith_fame_list;   break;
		case RANKTYPE_ALCHEMIST:  list = pc->chemist_fame_list; break;
		case RANKTYPE_TAEKWON:    list = pc->taekwon_fame_list; break;
		default: return 0;
	}
	
	index = RFIFOB(fd, 3);
	
	if (index >= MAX_FAME_LIST)
		return 0;
	
	list[index].fame = RFIFOL(fd,4);
	
	return 1;
}

bool chrif_save_scdata(struct map_session_data *sd) { //parses the sc_data of the player and sends it to the char-server for saving. [Skotlex]

#ifdef ENABLE_SC_SAVING
	int i, count=0;
	int64 tick;
	struct status_change_data data;
	struct status_change *sc = &sd->sc;
	const struct TimerData *td;

	chrif_check(false);
	tick = timer->gettick();
	
	WFIFOHEAD(chrif->fd, 14 + SC_MAX*sizeof(struct status_change_data));
	WFIFOW(chrif->fd,0) = 0x2b1c;
	WFIFOL(chrif->fd,4) = sd->status.account_id;
	WFIFOL(chrif->fd,8) = sd->status.char_id;
	
	for (i = 0; i < SC_MAX; i++) {
		if (!sc->data[i])
			continue;
		if (sc->data[i]->timer != INVALID_TIMER) {
			td = timer->get(sc->data[i]->timer);
			if (td == NULL || td->func != status->change_timer || DIFF_TICK(td->tick,tick) < 0)
				continue;
			data.tick = DIFF_TICK32(td->tick,tick); //Duration that is left before ending.
		} else
			data.tick = -1; //Infinite duration
		data.type = i;
		data.val1 = sc->data[i]->val1;
		data.val2 = sc->data[i]->val2;
		data.val3 = sc->data[i]->val3;
		data.val4 = sc->data[i]->val4;
		memcpy(WFIFOP(chrif->fd,14 +count*sizeof(struct status_change_data)),
			&data, sizeof(struct status_change_data));
		count++;
	}
	
	if (count == 0)
		return true; //Nothing to save. | Everything was as successful as if there was something to save.
	
	WFIFOW(chrif->fd,12) = count;
	WFIFOW(chrif->fd,2) = 14 +count*sizeof(struct status_change_data); //Total packet size
	WFIFOSET(chrif->fd,WFIFOW(chrif->fd,2));
#endif
	
	return true;
}

//Retrieve and load sc_data for a player. [Skotlex]
bool chrif_load_scdata(int fd) {

#ifdef ENABLE_SC_SAVING
	struct map_session_data *sd;
	struct status_change_data *data;
	int aid, cid, i, count;

	aid = RFIFOL(fd,4); //Player Account ID
	cid = RFIFOL(fd,8); //Player Char ID
	
	sd = map->id2sd(aid);
	
	if ( !sd ) {
		ShowError("chrif_load_scdata: Player of AID %d not found!\n", aid);
		return false;
	}
	
	if ( sd->status.char_id != cid ) {
		ShowError("chrif_load_scdata: Receiving data for account %d, char id does not matches (%d != %d)!\n", aid, sd->status.char_id, cid);
		return false;
	}
	
	count = RFIFOW(fd,12); //sc_count
	
	for (i = 0; i < count; i++) {
		data = (struct status_change_data*)RFIFOP(fd,14 + i*sizeof(struct status_change_data));
		status->change_start(&sd->bl, (sc_type)data->type, 10000, data->val1, data->val2, data->val3, data->val4, data->tick, 15);
	}
	
	pc->scdata_received(sd);
#endif
	
	return true;
}

/*==========================================
 * Send rates to char server [Wizputer]
 * S 2b16 <base rate>.L <job rate>.L <drop rate>.L
 *------------------------------------------*/
bool chrif_ragsrvinfo(int base_rate, int job_rate, int drop_rate) {
	chrif_check(false);

	WFIFOHEAD(chrif->fd,14);
	WFIFOW(chrif->fd,0) = 0x2b16;
	WFIFOL(chrif->fd,2) = base_rate;
	WFIFOL(chrif->fd,6) = job_rate;
	WFIFOL(chrif->fd,10) = drop_rate;
	WFIFOSET(chrif->fd,14);
	
	return true;
}


/*=========================================
 * Tell char-server charcter disconnected [Wizputer]
 *-----------------------------------------*/
bool chrif_char_offline_nsd(int account_id, int char_id) {
	chrif_check(false);

	WFIFOHEAD(chrif->fd,10);
	WFIFOW(chrif->fd,0) = 0x2b17;
	WFIFOL(chrif->fd,2) = char_id;
	WFIFOL(chrif->fd,6) = account_id;
	WFIFOSET(chrif->fd,10);

	return true;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------*/
bool chrif_flush(void) {
	chrif_check(false);

	set_nonblocking(chrif->fd, 0);
	flush_fifos();
	set_nonblocking(chrif->fd, 1);

	return true;
}

/*=========================================
 * Tell char-server to reset all chars offline [Wizputer]
 *-----------------------------------------*/
bool chrif_char_reset_offline(void) {
	chrif_check(false);

	WFIFOHEAD(chrif->fd,2);
	WFIFOW(chrif->fd,0) = 0x2b18;
	WFIFOSET(chrif->fd,2);

	return true;
}

/*=========================================
 * Tell char-server charcter is online [Wizputer]
 *-----------------------------------------*/
bool chrif_char_online(struct map_session_data *sd) {
	chrif_check(false);

	WFIFOHEAD(chrif->fd,10);
	WFIFOW(chrif->fd,0) = 0x2b19;
	WFIFOL(chrif->fd,2) = sd->status.char_id;
	WFIFOL(chrif->fd,6) = sd->status.account_id;
	WFIFOSET(chrif->fd,10);

	return true;
}

/// Called when the connection to Char Server is disconnected.
void chrif_on_disconnect(void) {
	if( chrif->connected != 1 )
		ShowWarning("Connection to Char Server lost.\n\n");
	chrif->connected = 0;
	
	chrif->other_mapserver_count = 0; //Reset counter. We receive ALL maps from all map-servers on reconnect.
	map->eraseallipport();

	//Attempt to reconnect in a second. [Skotlex]
	timer->add(timer->gettick() + 1000, chrif->check_connect_char_server, 0, 0);
}


void chrif_update_ip(int fd) {
	uint32 new_ip;
	
	WFIFOHEAD(fd,6);
	
	new_ip = host2ip(chrif->ip_str);
	
	if (new_ip && new_ip != chrif->ip)
		chrif->ip = new_ip; //Update chrif->ip

	new_ip = clif->refresh_ip();
	
	if (!new_ip)
		return; //No change
	
	WFIFOW(fd,0) = 0x2736;
	WFIFOL(fd,2) = htonl(new_ip);
	WFIFOSET(fd,6);
}

// pings the charserver ( since on-demand flag.ping was introduced, shouldn't this be dropped? only wasting traffic and processing [Ind])
void chrif_keepalive(int fd) {
	WFIFOHEAD(fd,2);
	WFIFOW(fd,0) = 0x2b23;
	WFIFOSET(fd,2);
}
void chrif_keepalive_ack(int fd) {
	session[fd]->flag.ping = 0;/* reset ping state, we received a packet */
}
void chrif_skillid2idx(int fd) {
	int i, count = 0;
	
	if( fd == 0 ) fd = chrif->fd;
	
	if( !session_isValid(fd) )
		return;
	
	WFIFOHEAD(fd,4 + (MAX_SKILL * 4));
	WFIFOW(fd,0) = 0x2b0b;
	for(i = 0; i < MAX_SKILL; i++) {
		if( skill->db[i].nameid ) {
			WFIFOW(fd, 4 + (count*4)) = skill->db[i].nameid;
			WFIFOW(fd, 6 + (count*4)) = i;
			count++;
		}
	}
	WFIFOW(fd,2) = 4 + (count * 4);
	WFIFOSET(fd,4 + (count * 4));

}
/*==========================================
 *
 *------------------------------------------*/
int chrif_parse(int fd) {
	int packet_len, cmd, r;

	// only process data from the char-server
	if ( fd != chrif->fd ) {
		ShowDebug("chrif_parse: Disconnecting invalid session #%d (is not the char-server)\n", fd);
		do_close(fd);
		return 0;
	}

	if ( session[fd]->flag.eof ) {
		do_close(fd);
		chrif->fd = -1;
		chrif->on_disconnect();
		return 0;
	} else if ( session[fd]->flag.ping ) {/* we've reached stall time */
		if( DIFF_TICK(sockt->last_tick, session[fd]->rdata_tick) > (sockt->stall_time * 2) ) {/* we can't wait any longer */
			set_eof(fd);
			return 0;
		} else if( session[fd]->flag.ping != 2 ) { /* we haven't sent ping out yet */
			chrif->keepalive(fd);
			session[fd]->flag.ping = 2;
		}
	}

	while ( RFIFOREST(fd) >= 2 ) {

		if( HPM->packetsc[hpChrif_Parse] ) {
			if( (r = HPM->parse_packets(fd,hpChrif_Parse)) ) {
				if( r == 1 ) continue;
				if( r == 2 ) return 0;
			}
		}

		cmd = RFIFOW(fd,0);
		
		if (cmd < 0x2af8 || cmd >= 0x2af8 + ARRAYLENGTH(chrif->packet_len_table) || chrif->packet_len_table[cmd-0x2af8] == 0) {
			r = intif->parse(fd); // Passed on to the intif

			if (r == 1) continue;	// Treated in intif 
			if (r == 2) return 0;	// Didn't have enough data (len==-1)

			ShowWarning("chrif_parse: session #%d, intif->parse failed (unrecognized command 0x%.4x).\n", fd, cmd);
			set_eof(fd);
			return 0;
		}

		if ( ( packet_len = chrif->packet_len_table[cmd-0x2af8] ) == -1) { // dynamic-length packet, second WORD holds the length
			if (RFIFOREST(fd) < 4)
				return 0;
			packet_len = RFIFOW(fd,2);
		}

		if ((int)RFIFOREST(fd) < packet_len)
			return 0;

		//ShowDebug("Received packet 0x%4x (%d bytes) from char-server (connection %d)\n", RFIFOW(fd,0), packet_len, fd);

		switch(cmd) {
			case 0x2af9: chrif->connectack(fd); break;
			case 0x2afb: chrif->sendmapack(fd); break;
			case 0x2afd: chrif->authok(fd); break;
			case 0x2b00: map->setusers(RFIFOL(fd,2)); chrif->keepalive(fd); break;
			case 0x2b03: clif->charselectok(RFIFOL(fd,2), RFIFOB(fd,6)); break;
			case 0x2b04: chrif->recvmap(fd); break;
			case 0x2b06: chrif->changemapserverack(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOW(fd,18), RFIFOW(fd,20), RFIFOW(fd,22), RFIFOL(fd,24), RFIFOW(fd,28)); break;
			case 0x2b09: map->addnickdb(RFIFOL(fd,2), (char*)RFIFOP(fd,6)); break;
			case 0x2b0a: sockt->datasync(fd, false); break;
			case 0x2b0d: chrif->changedsex(fd); break;
			case 0x2b0f: chrif->char_ask_name_answer(RFIFOL(fd,2), (char*)RFIFOP(fd,6), RFIFOW(fd,30), RFIFOW(fd,32)); break;
			case 0x2b12: chrif->divorceack(RFIFOL(fd,2), RFIFOL(fd,6)); break;
			case 0x2b14: chrif->idbanned(fd); break;
			case 0x2b1b: chrif->recvfamelist(fd); break;
			case 0x2b1d: chrif->load_scdata(fd); break;
			case 0x2b1e: chrif->update_ip(fd); break;
			case 0x2b1f: chrif->disconnectplayer(fd); break;
			case 0x2b20: chrif->removemap(fd); break;
			case 0x2b21: chrif->save_ack(fd); break;
			case 0x2b22: chrif->updatefamelist_ack(fd); break;
			case 0x2b24: chrif->keepalive_ack(fd); break;
			case 0x2b25: chrif->deadopt(RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
			case 0x2b27: chrif->authfail(fd); break;
			default:
				ShowError("chrif_parse : unknown packet (session #%d): 0x%x. Disconnecting.\n", fd, cmd);
				set_eof(fd);
				return 0;
		}
		if ( fd == chrif->fd ) //There's the slight chance we lost the connection during parse, in which case this would segfault if not checked [Skotlex]
			RFIFOSKIP(fd, packet_len);
	}

	return 0;
}

int send_usercount_tochar(int tid, int64 tick, int id, intptr_t data) {
	chrif_check(-1);

	WFIFOHEAD(chrif->fd,4);
	WFIFOW(chrif->fd,0) = 0x2afe;
	WFIFOW(chrif->fd,2) = map->usercount();
	WFIFOSET(chrif->fd,4);
	return 0;
}

/*==========================================
 * timerFunction
 * Send to char the number of client connected to map
 *------------------------------------------*/
bool send_users_tochar(void) {
	int users = 0, i = 0;
	struct map_session_data* sd;
	struct s_mapiterator* iter;

	chrif_check(false);

	users = map->usercount();
	
	WFIFOHEAD(chrif->fd, 6+8*users);
	WFIFOW(chrif->fd,0) = 0x2aff;
	
	iter = mapit_getallusers();
	
	for( sd = (TBL_PC*)mapit->first(iter); mapit->exists(iter); sd = (TBL_PC*)mapit->next(iter) ) {
		WFIFOL(chrif->fd,6+8*i) = sd->status.account_id;
		WFIFOL(chrif->fd,6+8*i+4) = sd->status.char_id;
		i++;
	}
	
	mapit->free(iter);
	
	WFIFOW(chrif->fd,2) = 6 + 8*users;
	WFIFOW(chrif->fd,4) = users;
	WFIFOSET(chrif->fd, 6+8*users);

	return true;
}

/*==========================================
 * timerFunction
  * Check the connection to char server, (if it down)
 *------------------------------------------*/
int check_connect_char_server(int tid, int64 tick, int id, intptr_t data) {
	static int displayed = 0;
	if ( chrif->fd <= 0 || session[chrif->fd] == NULL ) {
		if ( !displayed ) {
			ShowStatus("Attempting to connect to Char Server. Please wait.\n");
			displayed = 1;
		}

		chrif->state = 0;
		
		if ( ( chrif->fd = make_connection(chrif->ip, chrif->port,NULL) ) == -1) //Attempt to connect later. [Skotlex]
			return 0;

		session[chrif->fd]->func_parse = chrif->parse;
		session[chrif->fd]->flag.server = 1;
		realloc_fifo(chrif->fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);

		chrif->connect(chrif->fd);
		chrif->connected = (chrif->state == 2);
		chrif->srvinfo = 0;
	} else {
		if (chrif->srvinfo == 0) {
			chrif->ragsrvinfo(battle_config.base_exp_rate, battle_config.job_exp_rate, battle_config.item_rate_common);
			chrif->srvinfo = 1;
		}
	}
	if ( chrif->isconnected() )
		displayed = 0;
	return 0;
}

/*==========================================
 * Asks char server to remove friend_id from the friend list of char_id
 *------------------------------------------*/
bool chrif_removefriend(int char_id, int friend_id) {

	chrif_check(false);

	WFIFOHEAD(chrif->fd,10);
	WFIFOW(chrif->fd,0) = 0x2b07;
	WFIFOL(chrif->fd,2) = char_id;
	WFIFOL(chrif->fd,6) = friend_id;
	WFIFOSET(chrif->fd,10);
	
	return true;
}

void chrif_send_report(char* buf, int len) {
#ifndef STATS_OPT_OUT
	if( chrif->fd > 0 ) {
		WFIFOHEAD(chrif->fd,len + 2);
		
		WFIFOW(chrif->fd,0) = 0x3008;
		
		memcpy(WFIFOP(chrif->fd,2), buf, len);
		
		WFIFOSET(chrif->fd,len + 2);
		
		flush_fifo(chrif->fd); /* ensure it's sent now. */
	}
#endif
}

/**
 * Sends a single scdata for saving into char server, meant to ensure integrity of durationless conditions
 **/
void chrif_save_scdata_single(int account_id, int char_id, short type, struct status_change_entry *sce) {
	
	if( !chrif->fd )
		return;
	
	WFIFOHEAD(chrif->fd, 28);
	
	WFIFOW(chrif->fd, 0) = 0x2740;
	WFIFOL(chrif->fd, 2) = account_id;
	WFIFOL(chrif->fd, 6) = char_id;
	WFIFOW(chrif->fd, 10) = type;
	WFIFOL(chrif->fd, 12) = sce->val1;
	WFIFOL(chrif->fd, 16) = sce->val2;
	WFIFOL(chrif->fd, 20) = sce->val3;
	WFIFOL(chrif->fd, 24) = sce->val4;
	
	WFIFOSET(chrif->fd, 28);
	
}
/**
 * Sends a single scdata deletion request into char server, meant to ensure integrity of durationless conditions
 **/
void chrif_del_scdata_single(int account_id, int char_id, short type) {
	
	if( !chrif->fd ) {
		ShowError("MAYDAY! failed to delete status %d from CID:%d/AID:%d\n",type,char_id,account_id);
		return;
	}

	
	WFIFOHEAD(chrif->fd, 12);
	
	WFIFOW(chrif->fd, 0) = 0x2741;
	WFIFOL(chrif->fd, 2) = account_id;
	WFIFOL(chrif->fd, 6) = char_id;
	WFIFOW(chrif->fd, 10) = type;
	
	WFIFOSET(chrif->fd, 12);

}

/**	`
 * @see DBApply
 */
int auth_db_final(DBKey key, DBData *data, va_list ap) {
	struct auth_node *node = DB->data2ptr(data);
	
	if (node->sd)
		aFree(node->sd);
	
	ers_free(chrif->auth_db_ers, node);

	return 0;
}

/*==========================================
 * Destructor
 *------------------------------------------*/
void do_final_chrif(void) {
	
	if( chrif->fd != -1 ) {
		do_close(chrif->fd);
		chrif->fd = -1;
	}

	chrif->auth_db->destroy(chrif->auth_db, chrif->auth_db_final);
	
	ers_destroy(chrif->auth_db_ers);
}

/*==========================================
 *
 *------------------------------------------*/
void do_init_chrif(bool minimal) {
	if (minimal)
		return;

	chrif->auth_db = idb_alloc(DB_OPT_BASE);
	chrif->auth_db_ers = ers_new(sizeof(struct auth_node),"chrif.c::auth_db_ers",ERS_OPT_NONE);

	timer->add_func_list(chrif->check_connect_char_server, "check_connect_char_server");
	timer->add_func_list(chrif->auth_db_cleanup, "auth_db_cleanup");
	timer->add_func_list(chrif->send_usercount_tochar, "send_usercount_tochar");

	// establish map-char connection if not present
	timer->add_interval(timer->gettick() + 1000, chrif->check_connect_char_server, 0, 0, 10 * 1000);

	// wipe stale data for timed-out client connection requests
	timer->add_interval(timer->gettick() + 1000, chrif->auth_db_cleanup, 0, 0, 30 * 1000);

	// send the user count every 10 seconds, to hide the charserver's online counting problem
	timer->add_interval(timer->gettick() + 1000, chrif->send_usercount_tochar, 0, 0, UPDATE_INTERVAL);
}


/*=====================================
* Default Functions : chrif.h 
* Generated by HerculesInterfaceMaker
* created by Susu
*-------------------------------------*/
void chrif_defaults(void) {
	const int packet_len_table[CHRIF_PACKET_LEN_TABLE_SIZE] = { // U - used, F - free
		60, 3,-1,27,10,-1, 6,-1,	// 2af8-2aff: U->2af8, U->2af9, U->2afa, U->2afb, U->2afc, U->2afd, U->2afe, U->2aff
		6,-1,18, 7,-1,39,30, 10,	// 2b00-2b07: U->2b00, U->2b01, U->2b02, U->2b03, U->2b04, U->2b05, U->2b06, U->2b07
		6,30, -1, 0,86, 7,44,34,	// 2b08-2b0f: U->2b08, U->2b09, U->2b0a, F->2b0b, U->2b0c, U->2b0d, U->2b0e, U->2b0f
		11,10,10, 0,11, 0,266,10,	// 2b10-2b17: U->2b10, U->2b11, U->2b12, F->2b13, U->2b14, F->2b15, U->2b16, U->2b17
		2,10, 2,-1,-1,-1, 2, 7,		// 2b18-2b1f: U->2b18, U->2b19, U->2b1a, U->2b1b, U->2b1c, U->2b1d, U->2b1e, U->2b1f
		-1,10, 8, 2, 2,14,19,19,	// 2b20-2b27: U->2b20, U->2b21, U->2b22, U->2b23, U->2b24, U->2b25, U->2b26, U->2b27
	};

	chrif = &chrif_s;

	/* vars */
	chrif->connected = 0;
	chrif->other_mapserver_count = 0;
	
	memcpy(chrif->packet_len_table,&packet_len_table,sizeof(chrif->packet_len_table));
	chrif->fd = -1;
	chrif->srvinfo = 0;
	memset(chrif->ip_str,0,sizeof(chrif->ip_str));
	chrif->ip = 0;
	chrif->port = 6121;
	memset(chrif->userid,0,sizeof(chrif->userid));
	memset(chrif->passwd,0,sizeof(chrif->passwd));
	chrif->state = 0;
	
	/* */
	chrif->auth_db = NULL;
	chrif->auth_db_ers = NULL;
	/* */
	chrif->init = do_init_chrif;
	chrif->final = do_final_chrif;
	
	/* funcs */
	chrif->setuserid = chrif_setuserid;
	chrif->setpasswd = chrif_setpasswd;
	chrif->checkdefaultlogin = chrif_checkdefaultlogin;
	chrif->setip = chrif_setip;
	chrif->setport = chrif_setport;
	
	chrif->isconnected = chrif_isconnected;
	chrif->check_shutdown = chrif_check_shutdown;
	
	chrif->search = chrif_search;
	chrif->auth_check = chrif_auth_check;
	chrif->auth_delete = chrif_auth_delete;
	chrif->auth_finished = chrif_auth_finished;
	
	chrif->authreq = chrif_authreq;
	chrif->authok = chrif_authok;
	chrif->scdata_request = chrif_scdata_request;
	chrif->save = chrif_save;
	chrif->charselectreq = chrif_charselectreq;
	chrif->changemapserver = chrif_changemapserver;
	
	chrif->searchcharid = chrif_searchcharid;
	chrif->changeemail = chrif_changeemail;
	chrif->char_ask_name = chrif_char_ask_name;
	chrif->updatefamelist = chrif_updatefamelist;
	chrif->buildfamelist = chrif_buildfamelist;
	chrif->save_scdata = chrif_save_scdata;
	chrif->ragsrvinfo = chrif_ragsrvinfo;
	chrif->char_offline_nsd = chrif_char_offline_nsd;
	chrif->char_reset_offline = chrif_char_reset_offline;
	chrif->send_users_tochar = send_users_tochar;
	chrif->char_online = chrif_char_online;
	chrif->changesex = chrif_changesex;
	//chrif->chardisconnect = chrif_chardisconnect;
	chrif->divorce = chrif_divorce;
	
	chrif->removefriend = chrif_removefriend;
	chrif->send_report = chrif_send_report;
		
	chrif->flush = chrif_flush;
	chrif->skillid2idx = chrif_skillid2idx;
	
	chrif->sd_to_auth = chrif_sd_to_auth;
	chrif->check_connect_char_server = check_connect_char_server;
	chrif->auth_logout = chrif_auth_logout;
	chrif->save_ack = chrif_save_ack;
	chrif->reconnect = chrif_reconnect;
	chrif->auth_db_cleanup_sub = auth_db_cleanup_sub;
	chrif->char_ask_name_answer = chrif_char_ask_name_answer;
	chrif->auth_db_final = auth_db_final;
	chrif->send_usercount_tochar = send_usercount_tochar;
	chrif->auth_db_cleanup = auth_db_cleanup;
	
	chrif->connect = chrif_connect;
	chrif->connectack = chrif_connectack;
	chrif->sendmap = chrif_sendmap;
	chrif->sendmapack = chrif_sendmapack;
	chrif->recvmap = chrif_recvmap;
	chrif->changemapserverack = chrif_changemapserverack;
	chrif->changedsex = chrif_changedsex;
	chrif->divorceack = chrif_divorceack;
	chrif->idbanned = chrif_idbanned;
	chrif->recvfamelist = chrif_recvfamelist;
	chrif->load_scdata = chrif_load_scdata;
	chrif->update_ip = chrif_update_ip;
	chrif->disconnectplayer = chrif_disconnectplayer;
	chrif->removemap = chrif_removemap;
	chrif->updatefamelist_ack = chrif_updatefamelist_ack;
	chrif->keepalive = chrif_keepalive;
	chrif->keepalive_ack = chrif_keepalive_ack;
	chrif->deadopt = chrif_deadopt;
	chrif->authfail = chrif_authfail;
	chrif->on_ready = chrif_on_ready;
	chrif->on_disconnect = chrif_on_disconnect;
	chrif->parse = chrif_parse;
	chrif->save_scdata_single = chrif_save_scdata_single;
	chrif->del_scdata_single = chrif_del_scdata_single;
}
